/*
 * arch/arm/mach-tegra/include/mach/nct.h
 *
 * Copyright (c) 2013-2014, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __MACH_TEGRA_NCT_H
#define __MACH_TEGRA_NCT_H

#include <linux/platform_data/tegra_emc_pdata.h>

#define NCT_MAGIC_ID		0x7443566E /* "nVCt" */

#define NCT_FORMAT_VERSION	0x00010000 /* 0xMMMMNNNN (VMMMM.NNNN) */

#define NCT_ENTRY_OFFSET	0x4000
#define MAX_NCT_ENTRY		512
#define MAX_NCT_DATA_SIZE	1024

#define NVIDIA_OUI	0x00044B

enum nct_id_type {
	NCT_ID_START = 0,
	NCT_ID_SERIAL_NUMBER = NCT_ID_START,
	NCT_ID_WIFI_MAC_ADDR,
	NCT_ID_BT_ADDR,
	NCT_ID_CM_ID,
	NCT_ID_LBH_ID,
	NCT_ID_FACTORY_MODE,
	NCT_ID_RAMDUMP,
	NCT_ID_TEST,
	NCT_ID_BOARD_INFO,
	NCT_ID_GPS_ID,
	NCT_ID_LCD_ID,
	NCT_ID_ACCELEROMETER_ID,
	NCT_ID_COMPASS_ID,
	NCT_ID_GYROSCOPE_ID,
	NCT_ID_LIGHT_ID,
	NCT_ID_CHARGER_ID,
	NCT_ID_TOUCH_ID,
	NCT_ID_FUELGAUGE_ID,
	NCT_ID_MEMTABLE,
	NCT_ID_MEMTABLE_END = NCT_ID_MEMTABLE + TEGRA_EMC_MAX_FREQS - 1,
	NCT_ID_END = NCT_ID_MEMTABLE_END,
	NCT_ID_DISABLED = 0xEEEE,
	NCT_ID_MAX = 0xFFFF
};

struct nct_serial_number_type {
	char sn[30];
};

struct nct_mac_addr_type {
	u8 addr[6];
};

struct nct_cm_id_type {
	u16 id;
};

struct nct_lbh_id_type {
	u16 id;
};

struct nct_factory_mode_type {
	u32 flag;
};

struct nct_ramdump_type {
	u32 flag;
};

struct nct_board_info_type {
	u32 proc_board_id;
	u32 proc_sku;
	u32 proc_fab;
	u32 pmu_board_id;
	u32 pmu_sku;
	u32 pmu_fab;
	u32 display_board_id;
	u32 display_sku;
	u32 display_fab;
};

union nct_tegra_emc_table_type {
	struct tegra12_emc_table	tegra12_emc_table;
} __packed;


union nct_item_type {
	struct nct_serial_number_type	serial_number;
	struct nct_mac_addr_type	wifi_mac_addr;
	struct nct_mac_addr_type	bt_addr;
	struct nct_cm_id_type		cm_id;
	struct nct_lbh_id_type		lbh_id;
	struct nct_factory_mode_type	factory_mode;
	struct nct_ramdump_type		ramdump;
	struct nct_board_info_type	board_info;
	struct nct_lbh_id_type          gps_id;
	struct nct_lbh_id_type          lcd_id;
	struct nct_lbh_id_type          accelerometer_id;
	struct nct_lbh_id_type          compass_id;
	struct nct_lbh_id_type          gyroscope_id;
	struct nct_lbh_id_type          light_id;
	struct nct_lbh_id_type          charger_id;
	struct nct_lbh_id_type          touch_id;
	u8      fuelgauge_id[MAX_NCT_DATA_SIZE];
	union nct_tegra_emc_table_type	tegra_emc_table;
	u8	u8[MAX_NCT_DATA_SIZE];
	u16	u16[MAX_NCT_DATA_SIZE/2];
	u32	u32[MAX_NCT_DATA_SIZE/4];
} __packed;

struct nct_entry_type {
	u32 index;
	u32 reserved[2];
	union nct_item_type data;
	u32 checksum;
};

struct nct_part_head_type {
	u32 magicId;
	u32 vendorId;
	u32 productId;
	u32 version;
	u32 revision;
};

extern int tegra_nct_read_item(u32 index, union nct_item_type *buf);
extern int tegra_nct_is_init(void);

#endif /* __MACH_TEGRA_NCT_H */
