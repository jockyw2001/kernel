/*
 * TI Palma series PMIC's GPIO driver.
 *
 * Copyright 2011 Texas Instruments
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * Author: Graeme Gregory <gg@slimlogic.co.uk>
 * Author: Laxman Dewangan <ldewangan@nvidia.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mfd/palmas.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/syscore_ops.h>

struct palmas_gpio {
	struct gpio_chip gpio_chip;
	struct palmas *palmas;
	bool enable_boost_bypass;
	int v_boost_bypass_gpio;
};

static inline struct palmas_gpio *to_palmas_gpio(struct gpio_chip *chip)
{
	return container_of(chip, struct palmas_gpio, gpio_chip);
}

static int palmas_gpio_get(struct gpio_chip *gc, unsigned offset)
{
	struct palmas_gpio *pg = to_palmas_gpio(gc);
	struct palmas *palmas = pg->palmas;
	unsigned int val;
	int ret;
	unsigned int reg;
	int gpio8 = (offset/8);

	offset %= 8;
	reg = (gpio8) ? PALMAS_GPIO_DATA_DIR2 : PALMAS_GPIO_DATA_DIR;

	ret = palmas_read(palmas, PALMAS_GPIO_BASE, reg, &val);
	if (ret < 0) {
		dev_err(gc->dev, "Reg 0x%02x read failed, %d\n", reg, ret);
		return ret;
	}

	if (val & BIT(offset))
		reg = (gpio8) ? PALMAS_GPIO_DATA_OUT2 : PALMAS_GPIO_DATA_OUT;
	else
		reg = (gpio8) ? PALMAS_GPIO_DATA_IN2 : PALMAS_GPIO_DATA_IN;

	ret = palmas_read(palmas, PALMAS_GPIO_BASE, reg, &val);
	if (ret < 0) {
		dev_err(gc->dev, "Reg 0x%02x read failed, %d\n", reg, ret);
		return ret;
	}
	return !!(val & BIT(offset));
}

static void palmas_gpio_set(struct gpio_chip *gc, unsigned offset, int val)
{
	struct palmas_gpio *pg = to_palmas_gpio(gc);
	struct palmas *palmas = pg->palmas;
	int ret;
	unsigned int reg;
	int gpio8 = (offset/8);

	offset %= 8;
	if (gpio8)
		reg = (val) ?
			PALMAS_GPIO_SET_DATA_OUT2 : PALMAS_GPIO_CLEAR_DATA_OUT2;
	else
		reg = (val) ?
			PALMAS_GPIO_SET_DATA_OUT : PALMAS_GPIO_CLEAR_DATA_OUT;

	ret = palmas_write(palmas, PALMAS_GPIO_BASE, reg, BIT(offset));
	if (ret < 0)
		dev_err(gc->dev, "Reg 0x%02x write failed, %d\n", reg, ret);
}

static int palmas_gpio_output(struct gpio_chip *gc, unsigned offset,
				int value)
{
	struct palmas_gpio *pg = to_palmas_gpio(gc);
	struct palmas *palmas = pg->palmas;
	int ret;
	unsigned int reg;
	int gpio8 = (offset/8);

	offset %= 8;
	reg = (gpio8) ? PALMAS_GPIO_DATA_DIR2 : PALMAS_GPIO_DATA_DIR;

	/* Set the initial value */
	palmas_gpio_set(gc, offset, value);

	ret = palmas_update_bits(palmas, PALMAS_GPIO_BASE, reg,
				BIT(offset), BIT(offset));
	if (ret < 0)
		dev_err(gc->dev, "Reg 0x%02x update failed, %d\n", reg, ret);
	return ret;
}

static int palmas_gpio_input(struct gpio_chip *gc, unsigned offset)
{
	struct palmas_gpio *pg = to_palmas_gpio(gc);
	struct palmas *palmas = pg->palmas;
	int ret;
	unsigned int reg;
	int gpio8 = (offset/8);

	offset %= 8;
	reg = (gpio8) ? PALMAS_GPIO_DATA_DIR2 : PALMAS_GPIO_DATA_DIR;

	ret = palmas_update_bits(palmas, PALMAS_GPIO_BASE, reg, BIT(offset), 0);
	if (ret < 0)
		dev_err(gc->dev, "Reg 0x%02x update failed, %d\n", reg, ret);
	return ret;
}

static int palmas_gpio_to_irq(struct gpio_chip *gc, unsigned offset)
{
	struct palmas_gpio *pg = to_palmas_gpio(gc);
	struct palmas *palmas = pg->palmas;

	return palmas_irq_get_virq(palmas, PALMAS_GPIO_0_IRQ + offset);
}

static int palmas_gpio_set_debounce(struct gpio_chip *gc, unsigned offset,
				    unsigned debounce)
{
	struct palmas_gpio *pg = to_palmas_gpio(gc);
	struct palmas *palmas = pg->palmas;
	int ret;
	unsigned int reg;
	int gpio8 = (offset/8);
	int val = 0;

	offset %= 8;
	reg = (gpio8) ? PALMAS_GPIO_DEBOUNCE_EN2 : PALMAS_GPIO_DEBOUNCE_EN;

	if (debounce)
		val = BIT(offset);

	ret = palmas_update_bits(palmas, PALMAS_GPIO_BASE, reg,
				BIT(offset), val);
	if (ret < 0)
		dev_err(gc->dev, "Reg 0x%02x update failed, %d\n", reg, ret);
	return ret;
}

static int palmas_gpio_probe(struct platform_device *pdev)
{
	struct palmas *palmas = dev_get_drvdata(pdev->dev.parent);
	struct palmas_platform_data *palmas_pdata;
	struct palmas_gpio *palmas_gpio;

	int ret;

	palmas_gpio = devm_kzalloc(&pdev->dev,
				sizeof(*palmas_gpio), GFP_KERNEL);
	if (!palmas_gpio) {
		dev_err(&pdev->dev, "Could not allocate palmas_gpio\n");
		return -ENOMEM;
	}

	palmas_gpio->palmas = palmas;
	palmas_gpio->gpio_chip.owner = THIS_MODULE;
	palmas_gpio->gpio_chip.label = dev_name(&pdev->dev);
	palmas_gpio->gpio_chip.ngpio = 8;
	if (palmas->ngpio > 0)
		palmas_gpio->gpio_chip.ngpio = palmas->ngpio;
	palmas_gpio->gpio_chip.can_sleep = 1;
	palmas_gpio->gpio_chip.direction_input = palmas_gpio_input;
	palmas_gpio->gpio_chip.direction_output = palmas_gpio_output;
	palmas_gpio->gpio_chip.to_irq = palmas_gpio_to_irq;
	palmas_gpio->gpio_chip.set	= palmas_gpio_set;
	palmas_gpio->gpio_chip.get	= palmas_gpio_get;
	palmas_gpio->gpio_chip.set_debounce = palmas_gpio_set_debounce;
	palmas_gpio->gpio_chip.dev = &pdev->dev;
#ifdef CONFIG_OF_GPIO
	palmas_gpio->gpio_chip.of_node = pdev->dev.of_node;
#endif
	palmas_pdata = dev_get_platdata(palmas->dev);
	if (palmas_pdata && palmas_pdata->gpio_base)
		palmas_gpio->gpio_chip.base = palmas_pdata->gpio_base;
	else
		palmas_gpio->gpio_chip.base = -1;

	ret = of_property_read_u32(pdev->dev.of_node, "v_boost_bypass_gpio",
		&palmas_gpio->v_boost_bypass_gpio);
	if (ret < 0) {
		palmas_gpio->v_boost_bypass_gpio = -1;
		dev_err(&pdev->dev, "%s:Could not find boost_bypass gpio\n",
			__func__);
	}

	ret = gpiochip_add(&palmas_gpio->gpio_chip);
	if (ret < 0) {
		dev_err(&pdev->dev, "Could not register gpiochip, %d\n", ret);
		return ret;
	}

	if (pdev->dev.of_node) {
		palmas_gpio->enable_boost_bypass = of_property_read_bool(
			pdev->dev.of_node, "ti,enable-boost-bypass");
	}

	/* Set Boost Bypass */
	if (palmas_gpio->enable_boost_bypass &&
		palmas_gpio->v_boost_bypass_gpio != -1) {
		dev_dbg(&pdev->dev,
			"%s:Enabling boost bypass feature, set PMIC GPIO_%d as output high\n",
			__func__, palmas_gpio->v_boost_bypass_gpio);
		ret = palmas_gpio_output(&(palmas_gpio->gpio_chip),
			palmas_gpio->v_boost_bypass_gpio, 1);
		if (ret < 0) {
			dev_err(&pdev->dev,
			"Could not enable boost bypass feature, ret:%d\n", ret);
		}
	}

	platform_set_drvdata(pdev, palmas_gpio);
	return ret;
}

#ifdef CONFIG_PM_SLEEP
static int palmas_gpio_resume(struct platform_device *pdev)
{
	struct palmas_gpio *palmas_gpio = platform_get_drvdata(pdev);
	int ret = 0;

	if (palmas_gpio->enable_boost_bypass &&
		palmas_gpio->v_boost_bypass_gpio != -1) {
		ret = palmas_gpio_output(&(palmas_gpio->gpio_chip),
			palmas_gpio->v_boost_bypass_gpio, 1);
		dev_dbg(&pdev->dev,
			"%s:Enable boost bypass, set PMIC GPIO_%d as high: %d\n",
			__func__, palmas_gpio->v_boost_bypass_gpio, ret);
	}

	return ret;
}

static int palmas_gpio_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct palmas_gpio *palmas_gpio = platform_get_drvdata(pdev);
	int ret = 0;

	if (palmas_gpio->enable_boost_bypass &&
		palmas_gpio->v_boost_bypass_gpio != -1) {
		ret = palmas_gpio_output(&(palmas_gpio->gpio_chip),
			palmas_gpio->v_boost_bypass_gpio, 0);
		dev_dbg(&pdev->dev,
			"%s:Disable boost bypass, set PMIC GPIO_%d as low: %d\n",
			__func__, palmas_gpio->v_boost_bypass_gpio, ret);
	}

	return ret;
}
#endif

static int palmas_gpio_remove(struct platform_device *pdev)
{
	struct palmas_gpio *palmas_gpio = platform_get_drvdata(pdev);

	return gpiochip_remove(&palmas_gpio->gpio_chip);
}

static struct of_device_id of_palmas_gpio_match[] = {
	{ .compatible = "ti,palmas-gpio"},
	{ .compatible = "ti,tps65913-gpio"},
	{ .compatible = "ti,tps65914-gpio"},
	{ .compatible = "ti,tps80036-gpio"},
	{ },
};
MODULE_DEVICE_TABLE(of, of_palmas_gpio_match);

static struct platform_driver palmas_gpio_driver = {
	.driver.name	= "palmas-gpio",
	.driver.owner	= THIS_MODULE,
	.driver.of_match_table = of_palmas_gpio_match,
	.probe		= palmas_gpio_probe,
	.remove		= palmas_gpio_remove,
#ifdef CONFIG_PM_SLEEP
	.suspend	= palmas_gpio_suspend,
	.resume		= palmas_gpio_resume,
#endif
};

static int __init palmas_gpio_init(void)
{
	return platform_driver_register(&palmas_gpio_driver);
}
subsys_initcall(palmas_gpio_init);

static void __exit palmas_gpio_exit(void)
{
	platform_driver_unregister(&palmas_gpio_driver);
}
module_exit(palmas_gpio_exit);

MODULE_ALIAS("platform:palmas-gpio");
MODULE_AUTHOR("Graeme Gregory <gg@slimlogic.co.uk>");
MODULE_AUTHOR("Laxman Dewangan <ldewangan@nvidia.com>");
MODULE_DESCRIPTION("GPIO driver for TI Palmas series PMICs");
MODULE_LICENSE("GPL v2");
