/*
 * arch/arm/mach-tegra/thermal.h
 *
 * Copyright (c) 2010-2013 NVIDIA CORPORATION. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __MACH_THERMAL_H
#define __MACH_THERMAL_H

#include <linux/therm_est.h>
#include <linux/thermal.h>

struct tegra_cooling_device {
	char *cdev_type;
	int *trip_temperatures;
	int trip_temperatures_num;
};

#define MAX_THROT_TABLE_SIZE	(64)
#define NO_CAP			(ULONG_MAX) /* no cap */
#define CPU_THROT_LOW		0 /* lowest throttle freq. only used for CPU */

#ifdef CONFIG_ARCH_TEGRA_12x_SOC
#define GBUS_CNT		1
#else
#define GBUS_CNT		0
#endif

#ifdef CONFIG_TEGRA_DUAL_CBUS
#define CBUS_CNT		2
#else
#define CBUS_CNT		1
#endif

/* cpu, gpu(0|1), cbus(1|2), sclk, emc */
#define NUM_OF_CAP_FREQS	(1 + GBUS_CNT + CBUS_CNT + 1 + 1)

struct throttle_table {
	unsigned long cap_freqs[NUM_OF_CAP_FREQS];
};

struct balanced_throttle {
	struct thermal_cooling_device *cdev;
	struct list_head node;
	unsigned long cur_state;
	int throttle_count;
	int throt_tab_size;
	struct throttle_table *throt_tab;
};

#ifdef CONFIG_TEGRA_THERMAL_THROTTLE
int tegra_throttle_init(struct mutex *cpu_lock);
struct thermal_cooling_device *balanced_throttle_register(
		struct balanced_throttle *bthrot,
		char *type);
void tegra_throttle_exit(void);
bool tegra_is_throttling(int *count);
unsigned long tegra_throttle_governor_speed(unsigned long requested_speed);
#else
static inline int tegra_throttle_init(struct mutex *cpu_lock)
{ return 0; }
static inline struct thermal_cooling_device *balanced_throttle_register(
		struct balanced_throttle *bthrot,
		char *type)
{ return ERR_PTR(-ENODEV); }
static inline void tegra_throttle_exit(void)
{}
static inline bool tegra_is_throttling(int *count)
{ return false; }
static inline unsigned long tegra_throttle_governor_speed(
	unsigned long requested_speed)
{ return requested_speed; }
#endif /* CONFIG_TEGRA_THERMAL_THROTTLE */

#endif	/* __MACH_THERMAL_H */
