/*
 * Copyright (C) 2012-2014, NVIDIA CORPORATION. All rights reserved.
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
 *
 */

#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/export.h>
#include <linux/tegra-pmc.h>



#define PMC_CTRL			0x0
#define PMC_CTRL_INTR_LOW		(1 << 17)
#define PMC_PWRGATE_TOGGLE		0x30
#define PMC_PWRGATE_TOGGLE_START	(1 << 8)
#define PMC_REMOVE_CLAMPING		0x34
#define PMC_PWRGATE_STATUS		0x38

#define PMC_CPUPWRGOOD_TIMER	0xc8
#define PMC_CPUPWROFF_TIMER	0xcc

#define TEGRA_POWERGATE_PCIE	3
#define TEGRA_POWERGATE_VDEC	4
#define TEGRA_POWERGATE_CPU1	9
#define TEGRA_POWERGATE_CPU2	10
#define TEGRA_POWERGATE_CPU3	11

#define PMC_DPD_SAMPLE		0x20
#define PMC_IO_DPD_REQ		0x1B8
#define PMC_IO_DPD2_REQ		0x1C0

/* pmc register offsets needed for powering off PMU */
#define PMC_SCRATCH_WRITE_SHIFT			2
#define PMC_SCRATCH_WRITE_MASK			BIT(2)
#define PMC_ENABLE_RST_SHIFT			1
#define PMC_ENABLE_RST_MASK			BIT(1)
#define PMC_SENSOR_CTRL				0x1B0
#define PMC_SCRATCH54				0x258
#define PMC_SCRATCH55				0x25C

/* scratch54 register bit fields */
#define PMU_OFF_DATA_SHIFT			8
#define PMU_OFF_DATA_MASK			0xff
#define PMU_OFF_ADDR_SHIFT			0
#define PMU_OFF_ADDR_MASK			0xff

/* scratch55 register bit fields */
#define RESET_TEGRA_SHIFT			31
#define RESET_TEGRA_MASK			0x1
#define CONTROLLER_TYPE_SHIFT			30
#define CONTROLLER_TYPE_MASK			0x1
#define I2C_CONTROLLER_ID_SHIFT			27
#define I2C_CONTROLLER_ID_MASK			0x7
#define PINMUX_SHIFT				24
#define PINMUX_MASK				0x7
#define CHECKSUM_SHIFT				16
#define CHECKSUM_MASK				0xff
#define PMU_16BIT_SUPPORT_SHIFT			15
#define PMU_16BIT_SUPPORT_MASK			0x1
#define PMU_I2C_ADDRESS_SHIFT			0
#define PMU_I2C_ADDRESS_MASK			0x7f

static u8 tegra_cpu_domains[] = {
	0xFF,			/* not available for CPU0 */
	TEGRA_POWERGATE_CPU1,
	TEGRA_POWERGATE_CPU2,
	TEGRA_POWERGATE_CPU3,
};
static DEFINE_SPINLOCK(tegra_powergate_lock);

void __iomem *tegra_pmc_base;
static bool tegra_pmc_invert_interrupt;
#if defined(CONFIG_OF) && defined(CONFIG_COMMON_CLK)
static struct clk *tegra_pclk;
#endif

#ifdef CONFIG_OF
static struct pmc_pm_data pmc_pm_data;
#endif
struct pmc_pm_data *tegra_get_pm_data()
{
#ifdef CONFIG_OF
	/*
	 * Some boards have CONFIG_OF defined but no dts files
	 */
	if (!tegra_pmc_base)
		return NULL;
	return &pmc_pm_data;
#else
	return NULL;
#endif
}
EXPORT_SYMBOL(tegra_get_pm_data);

static inline u32 tegra_pmc_readl(u32 reg)
{
	return readl(tegra_pmc_base + reg);
}

static inline void tegra_pmc_writel(u32 val, u32 reg)
{
	writel(val, tegra_pmc_base + reg);
}

/**
 * _compute_pmic_checksum - compute checksum for some PMC_SCRATCH regs
 * @a: First half of the top PMC_SCRATCH register
 * @b: Second half of the top PMC_SCRATCH register
 * @v: Complete contents of the bottom PMC_SCRATCH register (except checksum)
 *
 * Compute a simple eight-bit checksum across two PMC_SCRATCH register
 * values.  There are two sets of two registers in the PMC scratch
 * space, and they are intended to configure how the boot ROM reacts
 * to certain exceptional cases (overtemperature and watchdog timer
 * expiration).  See the "Checksum Calculation" section in the "Boot
 * Process" section of the Tegra K1 TRM for more details here.
 * Originally intended for use with PMC_SCRATCH54/55.  Returns the
 * computed checksum byte.
 */
static u8 _compute_pmic_checksum(u32 a, u32 b, u32 v)
{
	u32 c;

	c = a + b;
	c += (v & 0xff) + ((v >> 8) & 0xff) + ((v >> 24) & 0xff);
	c &= 0xff;
	c = 0x100 - c;
	c &= 0xff;

	return c;
}

/**
 * tegra_pmc_enable_thermal_trip - enable hardware-controlled thermal reset
 *
 * Configure the PMC to initiate a hardware reset when the SOC_THERM
 * IP block detects a high temperature condition, and to allow us to
 * write to the PMC scratch registers to store the PMIC shutdown
 * command (in another function).  No return value.
 */
void tegra_pmc_enable_thermal_trip(void)
{
	u32 val;

	val = tegra_pmc_readl(PMC_SENSOR_CTRL);
	val &= ~PMC_SCRATCH_WRITE_MASK;
	val |= PMC_ENABLE_RST_MASK;
	tegra_pmc_writel(val, PMC_SENSOR_CTRL);
}
EXPORT_SYMBOL(tegra_pmc_enable_thermal_trip);

/**
 * tegra_pmc_config_thermal_trip - set PMC_SCRATCH54/55 from parameters
 * @poweroff_reg_data: What register value to write to the PMIC to power off
 * @poweroff_reg_addr: The PMIC register address to write @poweroff_reg_data
 * @controller_type: 0 for I2C, 1 for SPI, 2 for GPIO
 * @i2c_controller_id: I2C controller ID
 * @pinmux: pinmux configuration ID for the @controller_type pads
 * @pmu_16bit_ops: Must be set to 0.
 * @pmu_i2c_addr: I2C bus address of the PMIC to write to
 *
 * Configure the hardware thermal reset PMIC interaction functions of
 * the Tegra SoC.  More information on the argument values can be
 * found in include/linux/tegra-pmc.h.  No return value.
 *
 * XXX This function does no input validation, but it should.
 */
void tegra_pmc_config_thermal_trip(struct tegra_thermtrip_pmic_data *data)
{
	u32 v = 0;
	u32 c, w;

	/* Fill scratch registers to shutdown device on therm TRIP */
	v = data->poweroff_reg_data << PMU_OFF_DATA_SHIFT;
	v |= data->poweroff_reg_addr << PMU_OFF_ADDR_SHIFT;
	tegra_pmc_writel(v, PMC_SCRATCH54);

	w = tegra_pmc_readl(PMC_SCRATCH54);
	WARN(w != v, "PMC_SCRATCH%d value mismatch - chip may not shutdown PMIC correctly upon a thermal trip event", 54);

	v = 1 << RESET_TEGRA_SHIFT;
	v |= data->controller_type << CONTROLLER_TYPE_SHIFT;
	v |= data->i2c_controller_id << I2C_CONTROLLER_ID_SHIFT;
	v |= data->pinmux << PINMUX_SHIFT;
	v |= data->pmu_16bit_ops << PMU_16BIT_SUPPORT_SHIFT;
	v |= data->pmu_i2c_addr << PMU_I2C_ADDRESS_SHIFT;

	c = _compute_pmic_checksum(data->poweroff_reg_addr,
				   data->poweroff_reg_data, v);
	v |= c << CHECKSUM_SHIFT;

	tegra_pmc_writel(v, PMC_SCRATCH55);

	w = tegra_pmc_readl(PMC_SCRATCH55);
	WARN(w != v, "PMC_SCRATCH%d value mismatch - chip may not shutdown PMIC correctly upon a thermal trip event", 55);
}
EXPORT_SYMBOL(tegra_pmc_config_thermal_trip);

void tegra_pmc_set_dpd_sample()
{
	tegra_pmc_writel(0x1, PMC_DPD_SAMPLE);
}
EXPORT_SYMBOL(tegra_pmc_set_dpd_sample);

void tegra_pmc_clear_dpd_sample()
{
	tegra_pmc_writel(0x0, PMC_DPD_SAMPLE);
}
EXPORT_SYMBOL(tegra_pmc_clear_dpd_sample);

void tegra_pmc_remove_dpd_req()
{
	tegra_pmc_writel(0x400fffff, PMC_IO_DPD_REQ);
	tegra_pmc_readl(PMC_IO_DPD_REQ); /* unblock posted write */
	/* delay apb_clk * (SEL_DPD_TIM*5) */
	udelay(700);

	tegra_pmc_writel(0x40001fff, PMC_IO_DPD2_REQ);
	tegra_pmc_readl(PMC_IO_DPD2_REQ); /* unblock posted write */
	udelay(700);
}
EXPORT_SYMBOL(tegra_pmc_remove_dpd_req);

void tegra_pmc_pmu_interrupt_polarity(bool active_low)
{
	u32 pmc_ctrl;

	pmc_ctrl = tegra_pmc_readl(PMC_CTRL);
	if (active_low)
		pmc_ctrl |= PMC_CTRL_INTR_LOW;
	else
		pmc_ctrl &= ~PMC_CTRL_INTR_LOW;
	tegra_pmc_writel(pmc_ctrl, PMC_CTRL);
}
EXPORT_SYMBOL(tegra_pmc_pmu_interrupt_polarity);

static int tegra_pmc_get_cpu_powerdomain_id(int cpuid)
{
	if (cpuid <= 0 || cpuid >= num_possible_cpus())
		return -EINVAL;
	return tegra_cpu_domains[cpuid];
}

static bool tegra_pmc_powergate_is_powered(int id)
{
	return (tegra_pmc_readl(PMC_PWRGATE_STATUS) >> id) & 1;
}

static int tegra_pmc_powergate_set(int id, bool new_state)
{
	bool old_state;
	unsigned long flags;

	spin_lock_irqsave(&tegra_powergate_lock, flags);

	old_state = tegra_pmc_powergate_is_powered(id);
	WARN_ON(old_state == new_state);

	tegra_pmc_writel(PMC_PWRGATE_TOGGLE_START | id, PMC_PWRGATE_TOGGLE);

	spin_unlock_irqrestore(&tegra_powergate_lock, flags);

	return 0;
}

static int tegra_pmc_powergate_remove_clamping(int id)
{
	u32 mask;

	/*
	 * Tegra has a bug where PCIE and VDE clamping masks are
	 * swapped relatively to the partition ids.
	 */
	if (id ==  TEGRA_POWERGATE_VDEC)
		mask = (1 << TEGRA_POWERGATE_PCIE);
	else if	(id == TEGRA_POWERGATE_PCIE)
		mask = (1 << TEGRA_POWERGATE_VDEC);
	else
		mask = (1 << id);

	tegra_pmc_writel(mask, PMC_REMOVE_CLAMPING);

	return 0;
}

bool tegra_pmc_cpu_is_powered(int cpuid)
{
	int id;

	id = tegra_pmc_get_cpu_powerdomain_id(cpuid);
	if (id < 0)
		return false;
	return tegra_pmc_powergate_is_powered(id);
}

int tegra_pmc_cpu_power_on(int cpuid)
{
	int id;

	id = tegra_pmc_get_cpu_powerdomain_id(cpuid);
	if (id < 0)
		return id;
	return tegra_pmc_powergate_set(id, true);
}

int tegra_pmc_cpu_remove_clamping(int cpuid)
{
	int id;

	id = tegra_pmc_get_cpu_powerdomain_id(cpuid);
	if (id < 0)
		return id;
	return tegra_pmc_powergate_remove_clamping(id);
}

#if defined(CONFIG_OF) && defined(CONFIG_COMMON_CLK) && defined(CONFIG_PM_SLEEP)
void set_power_timers(unsigned long us_on, unsigned long us_off)
{
	unsigned long long ticks;
	unsigned long long pclk;
	unsigned long rate;
	static unsigned long tegra_last_pclk;

	rate = clk_get_rate(tegra_pclk);
	if (WARN_ON_ONCE(rate <= 0))
		pclk = 100000000;
	else
		pclk = rate;

	if ((rate != tegra_last_pclk)) {
		ticks = (us_on * pclk) + 999999ull;
		do_div(ticks, 1000000);
		tegra_pmc_writel((unsigned long)ticks, PMC_CPUPWRGOOD_TIMER);

		ticks = (us_off * pclk) + 999999ull;
		do_div(ticks, 1000000);
		tegra_pmc_writel((unsigned long)ticks, PMC_CPUPWROFF_TIMER);
		wmb();
	}
	tegra_last_pclk = pclk;
}
#endif

static const struct of_device_id matches[] __initconst = {
	{ .compatible = "nvidia,tegra124-pmc" },
	{ .compatible = "nvidia,tegra148-pmc" },
	{ .compatible = "nvidia,tegra114-pmc" },
	{ .compatible = "nvidia,tegra30-pmc" },
	{ .compatible = "nvidia,tegra20-pmc" },
	{ }
};

static void tegra_pmc_parse_dt(void)
{
	struct device_node *np;
	u32 prop;
	enum tegra_suspend_mode suspend_mode;
	u32 core_good_time[2] = {0, 0};
	u32 lp0_vec[2] = {0, 0};

	np = of_find_matching_node(NULL, matches);
	BUG_ON(!np);

	tegra_pmc_base = of_iomap(np, 0);

	tegra_pmc_invert_interrupt = of_property_read_bool(np,
				     "nvidia,invert-interrupt");
#if defined(CONFIG_OF) && defined(CONFIG_COMMON_CLK)
	tegra_pclk = of_clk_get_by_name(np, "pclk");
	WARN_ON(IS_ERR(tegra_pclk));
#endif

	/* Grabbing the power management configurations */
	if (of_property_read_u32(np, "nvidia,suspend-mode", &prop)) {
		suspend_mode = TEGRA_SUSPEND_NONE;
	} else {
		switch (prop) {
		case 0:
			suspend_mode = TEGRA_SUSPEND_LP0;
			break;
		case 1:
			suspend_mode = TEGRA_SUSPEND_LP1;
			break;
		case 2:
			suspend_mode = TEGRA_SUSPEND_LP2;
			break;
		default:
			suspend_mode = TEGRA_SUSPEND_NONE;
			break;
		}
	}

	if (of_property_read_u32(np, "nvidia,cpu-pwr-good-time", &prop))
		suspend_mode = TEGRA_SUSPEND_NONE;
	pmc_pm_data.cpu_good_time = prop;

	if (of_property_read_u32(np, "nvidia,cpu-pwr-off-time", &prop))
		suspend_mode = TEGRA_SUSPEND_NONE;
	pmc_pm_data.cpu_off_time = prop;

	if (of_property_read_u32_array(np, "nvidia,core-pwr-good-time",
			core_good_time, ARRAY_SIZE(core_good_time)))
		suspend_mode = TEGRA_SUSPEND_NONE;
	pmc_pm_data.core_osc_time = core_good_time[0];
	pmc_pm_data.core_pmu_time = core_good_time[1];

	if (of_property_read_u32(np, "nvidia,core-pwr-off-time",
				 &prop))
		suspend_mode = TEGRA_SUSPEND_NONE;
	pmc_pm_data.core_off_time = prop;

	pmc_pm_data.corereq_high = of_property_read_bool(np,
				"nvidia,core-power-req-active-high");

	pmc_pm_data.sysclkreq_high = of_property_read_bool(np,
				"nvidia,sys-clock-req-active-high");

	pmc_pm_data.combined_req = of_property_read_bool(np,
				"nvidia,combined-power-req");

	pmc_pm_data.cpu_pwr_good_en = of_property_read_bool(np,
				"nvidia,cpu-pwr-good-en");

	if (of_property_read_u32_array(np, "nvidia,lp0-vec", lp0_vec,
				       ARRAY_SIZE(lp0_vec)))
		if (suspend_mode == TEGRA_SUSPEND_LP0)
			suspend_mode = TEGRA_SUSPEND_LP1;

	pmc_pm_data.lp0_vec_phy_addr = lp0_vec[0];
	pmc_pm_data.lp0_vec_size = lp0_vec[1];

	pmc_pm_data.suspend_mode = suspend_mode;
}

static int __init tegra_pmc_init(void)
{
	u32 val;

	tegra_pmc_parse_dt();

	val = tegra_pmc_readl(PMC_CTRL);
	if (tegra_pmc_invert_interrupt)
		val |= PMC_CTRL_INTR_LOW;
	else
		val &= ~PMC_CTRL_INTR_LOW;
	tegra_pmc_writel(val, PMC_CTRL);

	return 0;
}
postcore_initcall(tegra_pmc_init);
