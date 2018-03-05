/*
 * arch/arm/mach-tegra/io.c
 *
 * Copyright (C) 2010 Google, Inc.
 *
 * Author:
 *	Colin Cross <ccross@google.com>
 *	Erik Gilling <konkers@google.com>
 *
 * Copyright (C) 2010-2013 NVIDIA Corporation
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/tegra-soc.h>

#include <asm/page.h>
#include <asm/mach/map.h>

#include "board.h"
#include "iomap.h"

#ifdef CONFIG_ARM64
#define MT_DEVICE MT_DEVICE_nGnRE
#endif

bool iotable_init_done = false;

static struct map_desc tegra_io_desc[] __initdata = {
	{
		.virtual = (unsigned long)IO_PPSB_VIRT,
		.pfn = __phys_to_pfn(IO_PPSB_PHYS),
		.length = IO_PPSB_SIZE,
		.type = MT_DEVICE,
	},
	{
		.virtual = (unsigned long)IO_APB_VIRT,
		.pfn = __phys_to_pfn(IO_APB_PHYS),
		.length = IO_APB_SIZE,
		.type = MT_DEVICE,
	},
	{
		.virtual = (unsigned long)IO_CPU_VIRT,
		.pfn = __phys_to_pfn(IO_CPU_PHYS),
		.length = IO_CPU_SIZE,
		.type = MT_DEVICE,
	},
	{
		.virtual = (unsigned long)IO_IRAM_VIRT,
		.pfn = __phys_to_pfn(IO_IRAM_PHYS),
		.length = IO_IRAM_SIZE,
		.type = MT_DEVICE,
	},
	{
		.virtual = (unsigned long)IO_HOST1X_VIRT,
		.pfn = __phys_to_pfn(IO_HOST1X_PHYS),
		.length = IO_HOST1X_SIZE,
		.type = MT_DEVICE,
	},
	{
		.virtual = (unsigned long)IO_USB_VIRT,
		.pfn = __phys_to_pfn(IO_USB_PHYS),
		.length = IO_USB_SIZE,
		.type = MT_DEVICE,
	},
	{
		.virtual = (unsigned long)IO_SDMMC_VIRT,
		.pfn = __phys_to_pfn(IO_SDMMC_PHYS),
		.length = IO_SDMMC_SIZE,
		.type = MT_DEVICE,
	},
	{
		.virtual = (unsigned long)IO_PPCS_VIRT,
		.pfn = __phys_to_pfn(IO_PPCS_PHYS),
		.length = IO_PPCS_SIZE,
		.type = MT_DEVICE,
	},
#if defined(CONFIG_TEGRA_PCI)
	{
		.virtual = (unsigned long)IO_PCIE_VIRT,
		.pfn = __phys_to_pfn(IO_PCIE_PHYS),
		.length = IO_PCIE_SIZE,
		.type = MT_DEVICE,
	},
#endif
#if defined(CONFIG_TEGRA_GMI)
	{
		.virtual = (unsigned long)IO_NOR_VIRT,
		.pfn = __phys_to_pfn(IO_NOR_PHYS),
		.length = IO_NOR_SIZE,
		.type = MT_DEVICE,
	},
#endif
#ifdef CONFIG_TEGRA_GK20A
	{
		.virtual = (unsigned long)IO_GK20A_B0_VIRT,
		.pfn = __phys_to_pfn(IO_GK20A_B0_PHYS),
		.length = IO_GK20A_B0_SIZE,
		.type = MT_DEVICE,
	},
	{
		.virtual = (unsigned long)IO_GK20A_B1_VIRT,
		.pfn = __phys_to_pfn(IO_GK20A_B1_PHYS),
		.length = IO_GK20A_B1_SIZE,
		.type = MT_DEVICE,
	},
#endif
};

#ifdef CONFIG_TEGRA_PRE_SILICON_SUPPORT
static struct map_desc tegra_io_desc_linsim[] __initdata = {
	{
		.virtual = (unsigned long)IO_SMC_VIRT,
		.pfn = __phys_to_pfn(IO_SMC_PHYS),
		.length = IO_SMC_SIZE,
		.type = MT_DEVICE,
	},
	{
		.virtual = (unsigned long)IO_SIM_ESCAPE_VIRT,
		.pfn = __phys_to_pfn(IO_SIM_ESCAPE_PHYS),
		.length = IO_SIM_ESCAPE_SIZE,
		.type = MT_DEVICE,
	},
};
#endif

void __init tegra_map_common_io(void)
{
	debug_ll_io_init();
	iotable_init(tegra_io_desc, ARRAY_SIZE(tegra_io_desc));

#ifdef CONFIG_TEGRA_PRE_SILICON_SUPPORT
	if (tegra_platform_is_linsim())
		iotable_init(tegra_io_desc_linsim,
			ARRAY_SIZE(tegra_io_desc_linsim));
#endif

	iotable_init_done = true;
}
