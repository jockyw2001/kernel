/*
 * Copyright (c) 2013-2014, NVIDIA CORPORATION. All rights reserved.
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

#include <linux/tegra-soc.h>
#include "fuse.h"

#ifndef __TEGRA12x_FUSE_OFFSETS_H
#define __TEGRA12x_FUSE_OFFSETS_H

/* private_key4 */
#define DEVKEY_START_OFFSET		0x2A
#define DEVKEY_START_BIT		12

/* arm_debug_dis */
#define JTAG_START_OFFSET		0x0
#define JTAG_START_BIT			12

/* security_mode */
#define ODM_PROD_START_OFFSET		0x0
#define ODM_PROD_START_BIT		11

/* boot_device_info */
#define SB_DEVCFG_START_OFFSET		0x2C
#define SB_DEVCFG_START_BIT		12

/* reserved_sw[2:0] */
#define SB_DEVSEL_START_OFFSET		0x2C
#define SB_DEVSEL_START_BIT		28

/* private_key0 -> private_key3 (SBK) */
#define SBK_START_OFFSET		0x22
#define SBK_START_BIT			12

/* reserved_sw[7:4] */
#define SW_RESERVED_START_OFFSET	0x2E
#define SW_RESERVED_START_BIT		0

/* reserved_sw[3] */
#define IGNORE_DEVSEL_START_OFFSET	0x2C
#define IGNORE_DEVSEL_START_BIT		31

/* public key */
#define PUBLIC_KEY_START_OFFSET		0x0A
#define PUBLIC_KEY_START_BIT		30

/* pkc_disable */
#define PKC_DISABLE_START_OFFSET        0x5A
#define PKC_DISABLE_START_BIT           9

/* video vp8 enable */
#define VP8_ENABLE_START_OFFSET		0x2E
#define VP8_ENABLE_START_BIT		4

/* odm lock */
#define ODM_LOCK_START_OFFSET		0x0
#define ODM_LOCK_START_BIT		6

/* reserved_odm0 -> reserved_odm7 */
#define ODM_RESERVED_DEVSEL_START_OFFSET	0x2E
#define ODM_RESERVED_START_BIT			5

/* AID */
#ifdef CONFIG_AID_FUSE
#define AID_START_OFFSET			0x72
#define AID_START_BIT				0
#endif

#define FUSE_VENDOR_CODE	0x200
#define FUSE_VENDOR_CODE_MASK	0xf
#define FUSE_FAB_CODE		0x204
#define FUSE_FAB_CODE_MASK	0x3f
#define FUSE_LOT_CODE_0		0x208
#define FUSE_LOT_CODE_1		0x20c
#define FUSE_LOT_CODE_1_MASK	0x0fffffff
#define FUSE_WAFER_ID		0x210
#define FUSE_WAFER_ID_MASK	0x3f
#define FUSE_X_COORDINATE	0x214
#define FUSE_X_COORDINATE_MASK	0x1ff
#define FUSE_Y_COORDINATE	0x218
#define FUSE_Y_COORDINATE_MASK	0x1ff
#define FUSE_OPS_RESERVED	0x220
#define FUSE_OPS_RESERVED_MASK	0x3f
#define FUSE_GPU_INFO		0x390
#define FUSE_GPU_INFO_MASK	(1<<2)
#define FUSE_SPARE_BIT		0x300
/* fuse registers used in public fuse data read API */
#define FUSE_FT_REV		0x128
#define FUSE_CP_REV		0x190
/* fuse spare bits are used to get Tj-ADT values */
#define NUM_TSENSOR_SPARE_BITS	28
/* tsensor calibration register */
#define FUSE_TSENSOR_CALIB_0	0x198
/* sparse realignment register */
#define FUSE_SPARE_REALIGNMENT_REG_0 0x2fc
/* tsensor8_calib */
#define FUSE_TSENSOR_CALIB_8 0x280

#define FUSE_BASE_CP_SHIFT	0
#define FUSE_BASE_CP_MASK	0x3ff
#define FUSE_BASE_FT_SHIFT	10
#define FUSE_BASE_FT_MASK	0x7ff
#define FUSE_SHIFT_CP_SHIFT	0
#define FUSE_SHIFT_CP_MASK	0x3f
#define FUSE_SHIFT_CP_BITS	6
#define FUSE_SHIFT_FT_SHIFT	21
#define FUSE_SHIFT_FT_MASK	0x1f
#define FUSE_SHIFT_FT_BITS	5

#define TEGRA_FUSE_SUPPLY	"vpp_fuse"

#define PGM_TIME_US 12

DEVICE_ATTR(public_key, 0440, tegra_fuse_show, tegra_fuse_store);
DEVICE_ATTR(pkc_disable, 0440, tegra_fuse_show, tegra_fuse_store);
DEVICE_ATTR(vp8_enable, 0440, tegra_fuse_show, tegra_fuse_store);
DEVICE_ATTR(odm_lock, 0440, tegra_fuse_show, tegra_fuse_store);

/*
 * Check CP fuse revision. Return value (depending on chip) is as below:
 *   Any: ERROR:      -ve:	Negative return value
 *  T12x: CP/FT:	1:	T124: Old style CP/FT fuse
 *  T12x: CP1/CP2:	0:	T124: New style CP1/CP2 fuse (default)
 *
 *  T13x: Old pattern:	2:	T132: Old ATE CP1/CP2 fuse (rev upto 0.8)
 *  T13x: Mid pattern:	1:	T132: Mid ATE CP1/CP2 fuse (rev 0.9 - 0.11)
 *  T13x: New pattern:	0:	T132: New ATE CP1/CP2 fuse (rev 0.12 onwards)
 */
inline int fuse_cp_rev_check(void)
{
	static enum tegra_chipid chip_id;
	u32 rev, rev_major, rev_minor;

	rev = tegra_fuse_readl(FUSE_CP_REV);
	rev_minor = rev & 0x1f;
	rev_major = (rev >> 5) & 0x3f;
	pr_debug("%s: CP rev %d.%d\n", __func__, rev_major, rev_minor);

	if (!chip_id)
		chip_id = tegra_get_chipid();

	/* T13x: all CP rev are valid */
	if (chip_id == TEGRA_CHIPID_TEGRA13) {
		/* CP rev <= 00.8 is old ATE pattern */
		if ((rev_major == 0) && (rev_minor <= 8))
			return 2;
		/* CP 00.8 > rev >= 00.11 is mid ATE pattern */
		if ((rev_major == 0) && (rev_minor <= 11))
			return 1;
		return 0; /* default new ATE pattern */
	}

	if (chip_id == TEGRA_CHIPID_TEGRA12) {
		/* CP rev < 00.4 is unsupported */
		if ((rev_major == 0) && (rev_minor < 4))
			return -EINVAL;
		/* CP rev < 00.8 is CP/FT (old style) */
		if ((rev_major == 0) && (rev_minor < 8))
			return 1;
		return 0; /* default new CP1/CP2 fuse */
	}

	return -EINVAL;
}

/*
 * Check FT fuse revision.
 * We check CP-rev and if it shows NEW style, we return ERROR.
 *  ERROR:    -ve:	Negative return value
 *  CP/FT:	0:	Old style CP/FT fuse (default for t12x)
 *  T13x:	1:	CP1/CP2 new ATE pattern (default for t13x)
 */
static inline int fuse_ft_rev_check(void)
{
	static enum tegra_chipid chip_id;
	u32 rev, rev_major, rev_minor;
	int check_cp = fuse_cp_rev_check();

	if (!chip_id)
		chip_id = tegra_get_chipid();

	/* T13x does not use FT */
	if (chip_id == TEGRA_CHIPID_TEGRA13)
		return 1;

	/* T12x */
	if (check_cp < 0)
		return check_cp;
	if (check_cp == 0)
		return -ENODEV; /* No FT rev in CP1/CP2 mode */

	rev = tegra_fuse_readl(FUSE_FT_REV);
	rev_minor = rev & 0x1f;
	rev_major = (rev >> 5) & 0x3f;
	pr_debug("%s: FT rev %d.%d\n", __func__, rev_major, rev_minor);

	/* FT rev < 00.5 is unsupported */
	if ((rev_major == 0) && (rev_minor < 5))
		return -EINVAL;

	return 0;
}

int tegra_fuse_get_tsensor_calibration_data(u32 *calib)
{
	/* tsensor calibration fuse */
	*calib = tegra_fuse_readl(FUSE_TSENSOR_CALIB_0);
	return 0;
}

int tegra_fuse_get_tsensor_spare_bits(u32 *spare_bits)
{
	u32 value;
	int i;

	BUG_ON(NUM_TSENSOR_SPARE_BITS > (sizeof(u32) * 8));
	if (!spare_bits)
		return -ENOMEM;
	*spare_bits = 0;
	/* spare bits 0-27 */
	for (i = 0; i < NUM_TSENSOR_SPARE_BITS; i++) {
		value = tegra_fuse_readl(FUSE_SPARE_BIT +
			(i << 2));
		if (value)
			*spare_bits |= BIT(i);
	}
	return 0;
}

unsigned long long tegra_chip_uid(void)
{

	u64 uid = 0ull;
	u32 reg;
	u32 cid;
	u32 vendor;
	u32 fab;
	u32 lot;
	u32 wafer;
	u32 x;
	u32 y;
	u32 i;

	/* This used to be so much easier in prior chips. Unfortunately, there
	   is no one-stop shopping for the unique id anymore. It must be
	   constructed from various bits of information burned into the fuses
	   during the manufacturing process. The 64-bit unique id is formed
	   by concatenating several bit fields. The notation used for the
	   various fields is <fieldname:size_in_bits> with the UID composed
	   thusly:

	   <CID:4><VENDOR:4><FAB:6><LOT:26><WAFER:6><X:9><Y:9>

	   Where:

		Field    Bits  Position Data
		-------  ----  -------- ----------------------------------------
		CID        4     60     Chip id
		VENDOR     4     56     Vendor code
		FAB        6     50     FAB code
		LOT       26     24     Lot code (5-digit base-36-coded-decimal,
					re-encoded to 26 bits binary)
		WAFER      6     18     Wafer id
		X          9      9     Wafer X-coordinate
		Y          9      0     Wafer Y-coordinate
		-------  ----
		Total     64
	*/

	/* chip id is 3 for tegra 12x */
	cid = 3;

	vendor = tegra_fuse_readl(FUSE_VENDOR_CODE) & FUSE_VENDOR_CODE_MASK;
	fab = tegra_fuse_readl(FUSE_FAB_CODE) & FUSE_FAB_CODE_MASK;

	/* Lot code must be re-encoded from a 5 digit base-36 'BCD' number
	   to a binary number. */
	lot = 0;
	reg = tegra_fuse_readl(FUSE_LOT_CODE_0) << 2;

	for (i = 0; i < 5; ++i) {
		u32 digit = (reg & 0xFC000000) >> 26;
		BUG_ON(digit >= 36);
		lot *= 36;
		lot += digit;
		reg <<= 6;
	}

	wafer = tegra_fuse_readl(FUSE_WAFER_ID) & FUSE_WAFER_ID_MASK;
	x = tegra_fuse_readl(FUSE_X_COORDINATE) & FUSE_X_COORDINATE_MASK;
	y = tegra_fuse_readl(FUSE_Y_COORDINATE) & FUSE_Y_COORDINATE_MASK;

	uid = ((unsigned long long)cid  << 60ull)
	    | ((unsigned long long)vendor << 56ull)
	    | ((unsigned long long)fab << 50ull)
	    | ((unsigned long long)lot << 24ull)
	    | ((unsigned long long)wafer << 18ull)
	    | ((unsigned long long)x << 9ull)
	    | ((unsigned long long)y << 0ull);
	return uid;
}

/* return uid in bootloader format */
static void tegra_chip_unique_id(u32 uid[4])
{
	u32 vendor;
	u32 fab;
	u32 wafer;
	u32 lot0;
	u32 lot1;
	u32 x, y;
	u32 rsvd;

	/** For t12x:
	 *
	 * Field        Bits     Data
	 * (LSB first)
	 * --------     ----     ----------------------------------------
	 * Reserved       6
	 * Y              9      Wafer Y-coordinate
	 * X              9      Wafer X-coordinate
	 * WAFER          6      Wafer id
	 * LOT_0         32      Lot code 0
	 * LOT_1         28      Lot code 1
	 * FAB            6      FAB code
	 * VENDOR         4      Vendor code
	 * --------     ----
	 * Total        100
	 *
	 * Gather up all the bits and pieces.
	 *
	 * <Vendor:4>
	 * <Fab:6><Lot0:26>
	 * <Lot0:6><Lot1:26>
	 * <Lot1:2><Wafer:6><X:9><Y:9><Reserved:6>
	 *
	 **/

	vendor = tegra_fuse_readl(FUSE_VENDOR_CODE) & FUSE_VENDOR_CODE_MASK;
	fab = tegra_fuse_readl(FUSE_FAB_CODE) & FUSE_FAB_CODE_MASK;
	wafer = tegra_fuse_readl(FUSE_WAFER_ID) & FUSE_WAFER_ID_MASK;
	x = tegra_fuse_readl(FUSE_X_COORDINATE) & FUSE_X_COORDINATE_MASK;
	y = tegra_fuse_readl(FUSE_Y_COORDINATE) & FUSE_Y_COORDINATE_MASK;

	lot0 = tegra_fuse_readl(FUSE_LOT_CODE_0);
	lot1 = tegra_fuse_readl(FUSE_LOT_CODE_1) & FUSE_LOT_CODE_1_MASK;
	rsvd = tegra_fuse_readl(FUSE_OPS_RESERVED) & FUSE_OPS_RESERVED_MASK;

	/* <Lot1:2><Wafer:6><X:9><Y:9><Reserved:6> */
	uid[3] = (lot1 << 30) | (wafer << 24) | (x << 15) | (y << 6) | rsvd;
	/* <Lot0:6><Lot1:26> */
	uid[2] = (lot0 << 26) | (lot1 >> 2);
	/* <Fab:6><Lot0:26> */
	uid[1] = (fab << 26) | (lot0 >> 6);
	/* <Vendor:4> */
	uid[0] = vendor;
}


static ssize_t tegra_cprev_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	u32 rev, major, minor;

	rev = tegra_fuse_readl(FUSE_CP_REV);
	minor = rev & 0x1f;
	major = (rev >> 5) & 0x3f;

	sprintf(buf, "%u.%u\n", major, minor);

	return strlen(buf);
}

static ssize_t tegra_uid_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	u32 uid[4];

	tegra_chip_unique_id(uid);
	sprintf(buf, "%X%08X%08X%08X\n", uid[0], uid[1], uid[2], uid[3]);
	return strlen(buf);
}

DEVICE_ATTR(cp_rev, 0444, tegra_cprev_show, NULL);
DEVICE_ATTR(uid, 0444, tegra_uid_show, NULL);

static int tsensor_calib_offset[] = {
	[0] = 0x198,
	[1] = 0x184,
	[2] = 0x188,
	[3] = 0x22c,
	[4] = 0x254,
	[5] = 0x258,
	[6] = 0x25c,
	[7] = 0x260,
};

int tegra_fuse_get_tsensor_calib(int index, u32 *calib)
{
	if (index < 0 || index >= ARRAY_SIZE(tsensor_calib_offset))
		return -EINVAL;
	*calib = tegra_fuse_readl(tsensor_calib_offset[index]);
	return 0;
}

/*
 * Returns CP or CP1 fuse dep on CP/FT or CP1/CP2 style fusing
 *   return value:
 *   -ve: ERROR
 *     0: New style CP1/CP2 fuse (default)
 *     1: Old style CP/FT fuse
 */
int tegra_fuse_calib_base_get_cp(u32 *base_cp, s32 *shifted_cp)
{
	s32 cp;
	u32 val;
	int check_cp = fuse_cp_rev_check();

	if (check_cp < 0)
		return check_cp;

	val = tegra_fuse_readl(FUSE_TSENSOR_CALIB_8);
	if (!val)
		return -EINVAL;

	if (base_cp)
		*base_cp = (((val) & (FUSE_BASE_CP_MASK
					<< FUSE_BASE_CP_SHIFT))
					>> FUSE_BASE_CP_SHIFT);

	val = tegra_fuse_readl(FUSE_SPARE_REALIGNMENT_REG_0);
	cp = (((val) & (FUSE_SHIFT_CP_MASK
				<< FUSE_SHIFT_CP_SHIFT))
				>> FUSE_SHIFT_CP_SHIFT);

	if (shifted_cp)
		*shifted_cp = ((s32)(cp)
				<< (32 - FUSE_SHIFT_CP_BITS)
				>> (32 - FUSE_SHIFT_CP_BITS));

	return check_cp; /* return tri-state: 0, 1, or -ve */
}

/*
 * Returns FT or CP2 fuse dep on CP/FT or CP1/CP2 style fusing
 *   return value:
 *   -ve: ERROR
 *     0: New style CP1/CP2 fuse (default)
 *     1: Old style CP/FT fuse
 */
int tegra_fuse_calib_base_get_ft(u32 *base_ft, s32 *shifted_ft)
{
	s32 ft_or_cp2;
	u32 val;
	int check_cp = fuse_cp_rev_check();
	int check_ft = fuse_ft_rev_check();

	if (check_cp < 0)
		return check_cp;
	/* when check_cp is 1, check_ft must be valid */
	if (check_cp != 0 && check_ft < 0)
		return -EINVAL;

	val = tegra_fuse_readl(FUSE_TSENSOR_CALIB_8);
	if (!val)
		return -EINVAL;

	if (base_ft)
		*base_ft = (((val) & (FUSE_BASE_FT_MASK
					<< FUSE_BASE_FT_SHIFT))
					>> FUSE_BASE_FT_SHIFT);

	ft_or_cp2 = (((val) & (FUSE_SHIFT_FT_MASK
				<< FUSE_SHIFT_FT_SHIFT))
				>> FUSE_SHIFT_FT_SHIFT);

	if (shifted_ft)
		*shifted_ft = ((s32)(ft_or_cp2)
				<< (32 - FUSE_SHIFT_FT_BITS)
				>> (32 - FUSE_SHIFT_FT_BITS));

	return check_cp; /* return tri-state: 0, 1, or -ve */
}

int tegra_fuse_add_sysfs_variables(struct platform_device *pdev,
					bool odm_security_mode)
{
	dev_attr_odm_lock.attr.mode = 0640;
	if (odm_security_mode) {
		dev_attr_public_key.attr.mode =  0440;
		dev_attr_pkc_disable.attr.mode = 0440;
		dev_attr_vp8_enable.attr.mode = 0440;
		dev_attr_cp_rev.attr.mode = 0444;
		dev_attr_uid.attr.mode = 0444;
	} else {
		dev_attr_public_key.attr.mode =  0640;
		dev_attr_pkc_disable.attr.mode = 0640;
		dev_attr_vp8_enable.attr.mode = 0640;
		dev_attr_cp_rev.attr.mode = 0444;
		dev_attr_uid.attr.mode = 0444;
	}

	CHK_ERR(&pdev->dev, sysfs_create_file(&pdev->dev.kobj,
				&dev_attr_public_key.attr));
	CHK_ERR(&pdev->dev, sysfs_create_file(&pdev->dev.kobj,
				&dev_attr_pkc_disable.attr));
	CHK_ERR(&pdev->dev, sysfs_create_file(&pdev->dev.kobj,
				&dev_attr_vp8_enable.attr));
	CHK_ERR(&pdev->dev, sysfs_create_file(&pdev->dev.kobj,
				&dev_attr_odm_lock.attr));
	CHK_ERR(&pdev->dev, sysfs_create_file(&pdev->dev.kobj,
				&dev_attr_cp_rev.attr));
	CHK_ERR(&pdev->dev, sysfs_create_file(&pdev->dev.kobj,
				&dev_attr_uid.attr));

	return 0;
}

int tegra_fuse_rm_sysfs_variables(struct platform_device *pdev)
{
	sysfs_remove_file(&pdev->dev.kobj, &dev_attr_public_key.attr);
	sysfs_remove_file(&pdev->dev.kobj, &dev_attr_pkc_disable.attr);
	sysfs_remove_file(&pdev->dev.kobj, &dev_attr_vp8_enable.attr);
	sysfs_remove_file(&pdev->dev.kobj, &dev_attr_odm_lock.attr);
	sysfs_remove_file(&pdev->dev.kobj, &dev_attr_cp_rev.attr);
	sysfs_remove_file(&pdev->dev.kobj, &dev_attr_uid.attr);

	return 0;
}

int tegra_fuse_ch_sysfs_perm(struct device *dev, struct kobject *kobj)
{
	CHK_ERR(dev, sysfs_chmod_file(kobj,
				&dev_attr_public_key.attr, 0440));
	CHK_ERR(dev, sysfs_chmod_file(kobj,
				&dev_attr_pkc_disable.attr, 0440));
	CHK_ERR(dev, sysfs_chmod_file(kobj,
				&dev_attr_vp8_enable.attr, 0440));
	CHK_ERR(dev, sysfs_chmod_file(kobj,
				&dev_attr_cp_rev.attr, 0444));
	CHK_ERR(dev, sysfs_chmod_file(kobj,
				&dev_attr_uid.attr, 0444));

	return 0;
}
#endif /* __TEGRA12x_FUSE_OFFSETS_H */
