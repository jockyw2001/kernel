/*
 * arch/arm/mach-tegra/tegra11_soctherm.h
 *
 * Copyright (c) 2011-2014, NVIDIA CORPORATION. All rights reserved.
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

#ifndef __MACH_TEGRA_11x_SOCTHERM_H
#define __MACH_TEGRA_11x_SOCTHERM_H

/* This order must match the soc_therm HW register spec */
enum soctherm_sense {
	TSENSE_CPU0 = 0,
	TSENSE_CPU1,
	TSENSE_CPU2,
	TSENSE_CPU3,
	TSENSE_MEM0,
	TSENSE_MEM1,
	TSENSE_GPU,
	TSENSE_PLLX,
	TSENSE_SIZE,
};

/* This order must match the soc_therm HW register spec */
enum soctherm_therm_id {
	THERM_CPU = 0,
	THERM_GPU,
	THERM_MEM,
	THERM_PLL,
	THERM_SIZE,
};

enum soctherm_throttle_id {
	THROTTLE_LIGHT = 0,
	THROTTLE_HEAVY,
	THROTTLE_OC1,
	THROTTLE_OC2,
	THROTTLE_OC3,
	THROTTLE_OC4,
	THROTTLE_OC5, /* OC5 is reserved for WAR to Bug 1415030 */
	THROTTLE_SIZE,
};

enum soctherm_throttle_dev_id {
	THROTTLE_DEV_CPU = 0,
	THROTTLE_DEV_GPU,
	THROTTLE_DEV_SIZE,
	THROTTLE_DEV_NONE,
};

enum soctherem_oc_irq_id {
	TEGRA_SOC_OC_IRQ_1,
	TEGRA_SOC_OC_IRQ_2,
	TEGRA_SOC_OC_IRQ_3,
	TEGRA_SOC_OC_IRQ_4,
	TEGRA_SOC_OC_IRQ_5,
	TEGRA_SOC_OC_IRQ_MAX,
};

enum soctherm_polarity_id {
	SOCTHERM_ACTIVE_HIGH = 0,
	SOCTHERM_ACTIVE_LOW = 1,
};

struct soctherm_sensor {
	bool sensor_enable;
	int tall;
	int tiddq;
	int ten_count;
	int tsample;
	int tsamp_ate;
	u8 pdiv;
	u8 pdiv_ate;
};

struct soctherm_therm {
	bool zone_enable;
	int passive_delay;
	int num_trips;
	int hotspot_offset;
	struct thermal_trip_info trips[THERMAL_MAX_TRIPS];
	struct thermal_zone_params *tzp;
};

struct soctherm_throttle_dev {
	bool enable;
	u8 depth; /* if this is non-zero, the values below are ignored */
	u8 dividend;
	u8 divisor;
	u16 duration;
	u8 step;
	char *throttling_depth;
};

enum throt_mode {
	DISABLED = 0,
	STICKY,
	BRIEF,
	RESERVED,
};

struct soctherm_throttle {
	u8 pgmask;
	u8 throt_mode;
	u8 polarity;
	u8 priority;
	u8 period;
	u32 alarm_cnt_threshold;
	u32 alarm_filter;
	bool intr;
	struct soctherm_throttle_dev devs[THROTTLE_DEV_SIZE];
};

struct soctherm_tsensor_pmu_data {
	u8 poweroff_reg_data;
	u8 poweroff_reg_addr;
	u8 reset_tegra;
	u8 controller_type;
	u8 i2c_controller_id;
	u8 pinmux;
	u8 pmu_16bit_ops;
	u8 pmu_i2c_addr;
};

struct soctherm_fuse_correction_war {
	/* both scaled *1000000 */
	int a;
	int b;
};

/**
 * struct soctherm_platform_data - Board specific SOC_THERM info.
 * @oc_irq_base:		Base over-current IRQ number
 * @num_oc_irqs:		Number of over-current IRQs
 * @soctherm_clk_rate:		Clock rate for the SOC_THERM IP block.
 * @tsensor_clk_rate:		Clock rate for the thermal sensors.
 * @sensor_data:		An array containing the data of each sensor
 *				See struct soctherm_sensor.
 * @therm:			An array contanining the board specific
 *				thermal zones and their respective settings,
 *				such as trip points.
 *				See struct soctherm_therm.
 * @throttle:			static specification for hardware throttle
 *				responses.
 *				See struct soctherm_throttle.
 * @tshut_pmu_trip_data:	PMU-specific thermal shutdown settings.
 *				See struct tegra_thermtrip_pmic_data.
 *
 * therm is used for trip point configuration and should be moved out of
 * soctherm_platform_data.
 */
struct soctherm_platform_data {
	int oc_irq_base;
	int num_oc_irqs;
	unsigned long soctherm_clk_rate;
	unsigned long tsensor_clk_rate;

	struct soctherm_sensor sensor_data[TSENSE_SIZE];
	struct soctherm_therm therm[THERM_SIZE];
	struct soctherm_throttle throttle[THROTTLE_SIZE];
	struct tegra_thermtrip_pmic_data *tshut_pmu_trip_data;
};

#ifdef CONFIG_TEGRA_SOCTHERM
int __init tegra11_soctherm_init(struct soctherm_platform_data *data);
void tegra_soctherm_adjust_cpu_zone(bool high_voltage_range);
void tegra_soctherm_adjust_core_zone(bool high_voltage_range);
#else
static inline int tegra11_soctherm_init(struct soctherm_platform_data *data)
{
	return 0;
}
static inline void tegra_soctherm_adjust_cpu_zone(bool high_voltage_range)
{ }
static inline void tegra_soctherm_adjust_core_zone(bool high_voltage_range)
{ }
#endif

#endif /* __MACH_TEGRA_11x_SOCTHERM_H */
