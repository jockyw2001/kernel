///////////////////////////////////////////////////////////////////////////////////////////////////
//
// * Copyright (c) 2006 - 2007 MStar Semiconductor, Inc.
// This program is free software.
// You can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation;
// either version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program;
// if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _REG_GPIO_H_
#define _REG_GPIO_H_

//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------
#define GPIO_UNIT_NUM               198

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define REG_MIPS_BASE               0xFD000000//Use 8 bit addressing

#define REG_ALL_PAD_IN              (0x101ea1)   //set all pads (except SPI) as input
#define REG_LVDS_BASE               (0x103200)
#define REG_LVDS_BANK               REG_LVDS_BASE

#define PAD_NC 9999
#define PAD_GND 0

#define BALL_F6 PAD_I2S_OUT_WS
#define PAD_I2S_OUT_WS 0
#define GPIO_PAD_1 GPIO0
#ifndef BALL_F6_IS_GPIO
#define BALL_F6_IS_GPIO 0
#endif
#ifndef PAD_I2S_OUT_WS_IS_GPIO
#define PAD_I2S_OUT_WS_IS_GPIO BALL_F6_IS_GPIO
#endif

#define BALL_F5 PAD_I2S_OUT_MCK
#define PAD_I2S_OUT_MCK 1
#define GPIO_PAD_2 GPIO1
#ifndef BALL_F5_IS_GPIO
#define BALL_F5_IS_GPIO 0
#endif
#ifndef PAD_I2S_OUT_MCK_IS_GPIO
#define PAD_I2S_OUT_MCK_IS_GPIO BALL_F5_IS_GPIO
#endif

#define BALL_F4 PAD_I2S_OUT_BCK
#define PAD_I2S_OUT_BCK 2
#define GPIO_PAD_3 GPIO2
#ifndef BALL_F4_IS_GPIO
#define BALL_F4_IS_GPIO 0
#endif
#ifndef PAD_I2S_OUT_BCK_IS_GPIO
#define PAD_I2S_OUT_BCK_IS_GPIO BALL_F4_IS_GPIO
#endif

#define BALL_E4 PAD_I2S_OUT_SD0
#define PAD_I2S_OUT_SD0 3
#define GPIO_PAD_4 GPIO3
#ifndef BALL_E4_IS_GPIO
#define BALL_E4_IS_GPIO 0
#endif
#ifndef PAD_I2S_OUT_SD0_IS_GPIO
#define PAD_I2S_OUT_SD0_IS_GPIO BALL_E4_IS_GPIO
#endif

#define BALL_E5 PAD_I2S_OUT_SD1
#define PAD_I2S_OUT_SD1 4
#define GPIO_PAD_5 GPIO4
#ifndef BALL_E5_IS_GPIO
#define BALL_E5_IS_GPIO 0
#endif
#ifndef PAD_I2S_OUT_SD1_IS_GPIO
#define PAD_I2S_OUT_SD1_IS_GPIO BALL_E5_IS_GPIO
#endif

#define BALL_D4 PAD_I2S_OUT_SD2
#define PAD_I2S_OUT_SD2 5
#define GPIO_PAD_6 GPIO5
#ifndef BALL_D4_IS_GPIO
#define BALL_D4_IS_GPIO 0
#endif
#ifndef PAD_I2S_OUT_SD2_IS_GPIO
#define PAD_I2S_OUT_SD2_IS_GPIO BALL_D4_IS_GPIO
#endif

#define BALL_D5 PAD_I2S_OUT_SD3
#define PAD_I2S_OUT_SD3 6
#define GPIO_PAD_7 GPIO6
#ifndef BALL_D5_IS_GPIO
#define BALL_D5_IS_GPIO 0
#endif
#ifndef PAD_I2S_OUT_SD3_IS_GPIO
#define PAD_I2S_OUT_SD3_IS_GPIO BALL_D5_IS_GPIO
#endif

#define BALL_B1 PAD_SDIO_CLK
#define PAD_SDIO_CLK 7
#define GPIO_PAD_8 GPIO7
#ifndef BALL_B1_IS_GPIO
#define BALL_B1_IS_GPIO 0
#endif
#ifndef PAD_SDIO_CLK_IS_GPIO
#define PAD_SDIO_CLK_IS_GPIO BALL_B1_IS_GPIO
#endif

#define BALL_C2 PAD_SDIO_CMD
#define PAD_SDIO_CMD 8
#define GPIO_PAD_9 GPIO8
#ifndef BALL_C2_IS_GPIO
#define BALL_C2_IS_GPIO 0
#endif
#ifndef PAD_SDIO_CMD_IS_GPIO
#define PAD_SDIO_CMD_IS_GPIO BALL_C2_IS_GPIO
#endif

#define BALL_B2 PAD_SDIO_D0
#define PAD_SDIO_D0 9
#define GPIO_PAD_10 GPIO9
#ifndef BALL_B2_IS_GPIO
#define BALL_B2_IS_GPIO 0
#endif
#ifndef PAD_SDIO_D0_IS_GPIO
#define PAD_SDIO_D0_IS_GPIO BALL_B2_IS_GPIO
#endif

#define BALL_A1 PAD_SDIO_D1
#define PAD_SDIO_D1 10
#define GPIO_PAD_11 GPIO10
#ifndef BALL_A1_IS_GPIO
#define BALL_A1_IS_GPIO 0
#endif
#ifndef PAD_SDIO_D1_IS_GPIO
#define PAD_SDIO_D1_IS_GPIO BALL_A1_IS_GPIO
#endif

#define BALL_D3 PAD_SDIO_D2
#define PAD_SDIO_D2 11
#define GPIO_PAD_12 GPIO11
#ifndef BALL_D3_IS_GPIO
#define BALL_D3_IS_GPIO 0
#endif
#ifndef PAD_SDIO_D2_IS_GPIO
#define PAD_SDIO_D2_IS_GPIO BALL_D3_IS_GPIO
#endif

#define BALL_C1 PAD_SDIO_D3
#define PAD_SDIO_D3 12
#define GPIO_PAD_13 GPIO12
#ifndef BALL_C1_IS_GPIO
#define BALL_C1_IS_GPIO 0
#endif
#ifndef PAD_SDIO_D3_IS_GPIO
#define PAD_SDIO_D3_IS_GPIO BALL_C1_IS_GPIO
#endif

#define BALL_AG21 PAD_SM0_CD
#define PAD_SM0_CD 13
#define GPIO_PAD_14 GPIO13
#ifndef BALL_AG21_IS_GPIO
#define BALL_AG21_IS_GPIO 0
#endif
#ifndef PAD_SM0_CD_IS_GPIO
#define PAD_SM0_CD_IS_GPIO BALL_AG21_IS_GPIO
#endif

#define BALL_AF20 PAD_SM0_RST
#define PAD_SM0_RST 14
#define GPIO_PAD_15 GPIO14
#ifndef BALL_AF20_IS_GPIO
#define BALL_AF20_IS_GPIO 0
#endif
#ifndef PAD_SM0_RST_IS_GPIO
#define PAD_SM0_RST_IS_GPIO BALL_AF20_IS_GPIO
#endif

#define BALL_AG20 PAD_SM0_VCC
#define PAD_SM0_VCC 15
#define GPIO_PAD_16 GPIO15
#ifndef BALL_AG20_IS_GPIO
#define BALL_AG20_IS_GPIO 0
#endif
#ifndef PAD_SM0_VCC_IS_GPIO
#define PAD_SM0_VCC_IS_GPIO BALL_AG20_IS_GPIO
#endif

#define BALL_AH20 PAD_SM0_IO
#define PAD_SM0_IO 16
#define GPIO_PAD_17 GPIO16
#ifndef BALL_AH20_IS_GPIO
#define BALL_AH20_IS_GPIO 0
#endif
#ifndef PAD_SM0_IO_IS_GPIO
#define PAD_SM0_IO_IS_GPIO BALL_AH20_IS_GPIO
#endif

#define BALL_AF19 PAD_SM0_CLK
#define PAD_SM0_CLK 17
#define GPIO_PAD_18 GPIO17
#ifndef BALL_AF19_IS_GPIO
#define BALL_AF19_IS_GPIO 0
#endif
#ifndef PAD_SM0_CLK_IS_GPIO
#define PAD_SM0_CLK_IS_GPIO BALL_AF19_IS_GPIO
#endif

#define BALL_AG22 PAD_SM0_GPIO0
#define PAD_SM0_GPIO0 18
#define GPIO_PAD_19 GPIO18
#ifndef BALL_AG22_IS_GPIO
#define BALL_AG22_IS_GPIO 0
#endif
#ifndef PAD_SM0_GPIO0_IS_GPIO
#define PAD_SM0_GPIO0_IS_GPIO BALL_AG22_IS_GPIO
#endif

#define BALL_AF21 PAD_SM0_GPIO1
#define PAD_SM0_GPIO1 19
#define GPIO_PAD_20 GPIO19
#ifndef BALL_AF21_IS_GPIO
#define BALL_AF21_IS_GPIO 0
#endif
#ifndef PAD_SM0_GPIO1_IS_GPIO
#define PAD_SM0_GPIO1_IS_GPIO BALL_AF21_IS_GPIO
#endif

#define BALL_AF23 PAD_SM1_CD
#define PAD_SM1_CD 20
#define GPIO_PAD_21 GPIO20
#ifndef BALL_AF23_IS_GPIO
#define BALL_AF23_IS_GPIO 0
#endif
#ifndef PAD_SM1_CD_IS_GPIO
#define PAD_SM1_CD_IS_GPIO BALL_AF23_IS_GPIO
#endif

#define BALL_AG23 PAD_SM1_RST
#define PAD_SM1_RST 21
#define GPIO_PAD_22 GPIO21
#ifndef BALL_AG23_IS_GPIO
#define BALL_AG23_IS_GPIO 0
#endif
#ifndef PAD_SM1_RST_IS_GPIO
#define PAD_SM1_RST_IS_GPIO BALL_AG23_IS_GPIO
#endif

#define BALL_AH23 PAD_SM1_VCC
#define PAD_SM1_VCC 22
#define GPIO_PAD_23 GPIO22
#ifndef BALL_AH23_IS_GPIO
#define BALL_AH23_IS_GPIO 0
#endif
#ifndef PAD_SM1_VCC_IS_GPIO
#define PAD_SM1_VCC_IS_GPIO BALL_AH23_IS_GPIO
#endif

#define BALL_AF22 PAD_SM1_IO
#define PAD_SM1_IO 23
#define GPIO_PAD_24 GPIO23
#ifndef BALL_AF22_IS_GPIO
#define BALL_AF22_IS_GPIO 0
#endif
#ifndef PAD_SM1_IO_IS_GPIO
#define PAD_SM1_IO_IS_GPIO BALL_AF22_IS_GPIO
#endif

#define BALL_AH22 PAD_SM1_CLK
#define PAD_SM1_CLK 24
#define GPIO_PAD_25 GPIO24
#ifndef BALL_AH22_IS_GPIO
#define BALL_AH22_IS_GPIO 0
#endif
#ifndef PAD_SM1_CLK_IS_GPIO
#define PAD_SM1_CLK_IS_GPIO BALL_AH22_IS_GPIO
#endif

#define BALL_AF24 PAD_SM1_GPIO0
#define PAD_SM1_GPIO0 25
#define GPIO_PAD_26 GPIO25
#ifndef BALL_AF24_IS_GPIO
#define BALL_AF24_IS_GPIO 0
#endif
#ifndef PAD_SM1_GPIO0_IS_GPIO
#define PAD_SM1_GPIO0_IS_GPIO BALL_AF24_IS_GPIO
#endif

#define BALL_AG24 PAD_SM1_GPIO1
#define PAD_SM1_GPIO1 26
#define GPIO_PAD_27 GPIO26
#ifndef BALL_AG24_IS_GPIO
#define BALL_AG24_IS_GPIO 0
#endif
#ifndef PAD_SM1_GPIO1_IS_GPIO
#define PAD_SM1_GPIO1_IS_GPIO BALL_AG24_IS_GPIO
#endif

#define BALL_AC24 PAD_EJ_DINT
#define PAD_EJ_DINT 27
#define GPIO_PAD_28 GPIO27
#ifndef BALL_AC24_IS_GPIO
#define BALL_AC24_IS_GPIO 0
#endif
#ifndef PAD_EJ_DINT_IS_GPIO
#define PAD_EJ_DINT_IS_GPIO BALL_AC24_IS_GPIO
#endif

#define BALL_AC25 PAD_EJ_RSTZ
#define PAD_EJ_RSTZ 28
#define GPIO_PAD_29 GPIO28
#ifndef BALL_AC25_IS_GPIO
#define BALL_AC25_IS_GPIO 0
#endif
#ifndef PAD_EJ_RSTZ_IS_GPIO
#define PAD_EJ_RSTZ_IS_GPIO BALL_AC25_IS_GPIO
#endif

#define BALL_AD28 PAD_CI_A0
#define PAD_CI_A0 29
#define GPIO_PAD_30 GPIO29
#ifndef BALL_AD28_IS_GPIO
#define BALL_AD28_IS_GPIO 0
#endif
#ifndef PAD_CI_A0_IS_GPIO
#define PAD_CI_A0_IS_GPIO BALL_AD28_IS_GPIO
#endif

#define BALL_AD27 PAD_CI_A1
#define PAD_CI_A1 30
#define GPIO_PAD_31 GPIO30
#ifndef BALL_AD27_IS_GPIO
#define BALL_AD27_IS_GPIO 0
#endif
#ifndef PAD_CI_A1_IS_GPIO
#define PAD_CI_A1_IS_GPIO BALL_AD27_IS_GPIO
#endif

#define BALL_AD26 PAD_CI_A2
#define PAD_CI_A2 31
#define GPIO_PAD_32 GPIO31
#ifndef BALL_AD26_IS_GPIO
#define BALL_AD26_IS_GPIO 0
#endif
#ifndef PAD_CI_A2_IS_GPIO
#define PAD_CI_A2_IS_GPIO BALL_AD26_IS_GPIO
#endif

#define BALL_AF27 PAD_CI_A3
#define PAD_CI_A3 32
#define GPIO_PAD_33 GPIO32
#ifndef BALL_AF27_IS_GPIO
#define BALL_AF27_IS_GPIO 0
#endif
#ifndef PAD_CI_A3_IS_GPIO
#define PAD_CI_A3_IS_GPIO BALL_AF27_IS_GPIO
#endif

#define BALL_AF28 PAD_CI_A4
#define PAD_CI_A4 33
#define GPIO_PAD_34 GPIO33
#ifndef BALL_AF28_IS_GPIO
#define BALL_AF28_IS_GPIO 0
#endif
#ifndef PAD_CI_A4_IS_GPIO
#define PAD_CI_A4_IS_GPIO BALL_AF28_IS_GPIO
#endif

#define BALL_AF26 PAD_CI_A5
#define PAD_CI_A5 34
#define GPIO_PAD_35 GPIO34
#ifndef BALL_AF26_IS_GPIO
#define BALL_AF26_IS_GPIO 0
#endif
#ifndef PAD_CI_A5_IS_GPIO
#define PAD_CI_A5_IS_GPIO BALL_AF26_IS_GPIO
#endif

#define BALL_AG28 PAD_CI_A6
#define PAD_CI_A6 35
#define GPIO_PAD_36 GPIO35
#ifndef BALL_AG28_IS_GPIO
#define BALL_AG28_IS_GPIO 0
#endif
#ifndef PAD_CI_A6_IS_GPIO
#define PAD_CI_A6_IS_GPIO BALL_AG28_IS_GPIO
#endif

#define BALL_AH27 PAD_CI_A7
#define PAD_CI_A7 36
#define GPIO_PAD_37 GPIO36
#ifndef BALL_AH27_IS_GPIO
#define BALL_AH27_IS_GPIO 0
#endif
#ifndef PAD_CI_A7_IS_GPIO
#define PAD_CI_A7_IS_GPIO BALL_AH27_IS_GPIO
#endif

#define BALL_AG25 PAD_CI_A8
#define PAD_CI_A8 37
#define GPIO_PAD_38 GPIO37
#ifndef BALL_AG25_IS_GPIO
#define BALL_AG25_IS_GPIO 0
#endif
#ifndef PAD_CI_A8_IS_GPIO
#define PAD_CI_A8_IS_GPIO BALL_AG25_IS_GPIO
#endif

#define BALL_AD25 PAD_CI_A9
#define PAD_CI_A9 38
#define GPIO_PAD_39 GPIO38
#ifndef BALL_AD25_IS_GPIO
#define BALL_AD25_IS_GPIO 0
#endif
#ifndef PAD_CI_A9_IS_GPIO
#define PAD_CI_A9_IS_GPIO BALL_AD25_IS_GPIO
#endif

#define BALL_AE23 PAD_CI_A10
#define PAD_CI_A10 39
#define GPIO_PAD_40 GPIO39
#ifndef BALL_AE23_IS_GPIO
#define BALL_AE23_IS_GPIO 0
#endif
#ifndef PAD_CI_A10_IS_GPIO
#define PAD_CI_A10_IS_GPIO BALL_AE23_IS_GPIO
#endif

#define BALL_AC23 PAD_CI_A11
#define PAD_CI_A11 40
#define GPIO_PAD_41 GPIO40
#ifndef BALL_AC23_IS_GPIO
#define BALL_AC23_IS_GPIO 0
#endif
#ifndef PAD_CI_A11_IS_GPIO
#define PAD_CI_A11_IS_GPIO BALL_AC23_IS_GPIO
#endif

#define BALL_AG27 PAD_CI_A12
#define PAD_CI_A12 41
#define GPIO_PAD_42 GPIO41
#ifndef BALL_AG27_IS_GPIO
#define BALL_AG27_IS_GPIO 0
#endif
#ifndef PAD_CI_A12_IS_GPIO
#define PAD_CI_A12_IS_GPIO BALL_AG27_IS_GPIO
#endif

#define BALL_AH25 PAD_CI_A13
#define PAD_CI_A13 42
#define GPIO_PAD_43 GPIO42
#ifndef BALL_AH25_IS_GPIO
#define BALL_AH25_IS_GPIO 0
#endif
#ifndef PAD_CI_A13_IS_GPIO
#define PAD_CI_A13_IS_GPIO BALL_AH25_IS_GPIO
#endif

#define BALL_AF25 PAD_CI_A14
#define PAD_CI_A14 43
#define GPIO_PAD_44 GPIO43
#ifndef BALL_AF25_IS_GPIO
#define BALL_AF25_IS_GPIO 0
#endif
#ifndef PAD_CI_A14_IS_GPIO
#define PAD_CI_A14_IS_GPIO BALL_AF25_IS_GPIO
#endif

#define BALL_AC26 PAD_CI_D0
#define PAD_CI_D0 44
#define GPIO_PAD_45 GPIO44
#ifndef BALL_AC26_IS_GPIO
#define BALL_AC26_IS_GPIO 0
#endif
#ifndef PAD_CI_D0_IS_GPIO
#define PAD_CI_D0_IS_GPIO BALL_AC26_IS_GPIO
#endif

#define BALL_AC28 PAD_CI_D1
#define PAD_CI_D1 45
#define GPIO_PAD_46 GPIO45
#ifndef BALL_AC28_IS_GPIO
#define BALL_AC28_IS_GPIO 0
#endif
#ifndef PAD_CI_D1_IS_GPIO
#define PAD_CI_D1_IS_GPIO BALL_AC28_IS_GPIO
#endif

#define BALL_AC27 PAD_CI_D2
#define PAD_CI_D2 46
#define GPIO_PAD_47 GPIO46
#ifndef BALL_AC27_IS_GPIO
#define BALL_AC27_IS_GPIO 0
#endif
#ifndef PAD_CI_D2_IS_GPIO
#define PAD_CI_D2_IS_GPIO BALL_AC27_IS_GPIO
#endif

#define BALL_AD21 PAD_CI_D3
#define PAD_CI_D3 47
#define GPIO_PAD_48 GPIO47
#ifndef BALL_AD21_IS_GPIO
#define BALL_AD21_IS_GPIO 0
#endif
#ifndef PAD_CI_D3_IS_GPIO
#define PAD_CI_D3_IS_GPIO BALL_AD21_IS_GPIO
#endif

#define BALL_AC21 PAD_CI_D4
#define PAD_CI_D4 48
#define GPIO_PAD_49 GPIO48
#ifndef BALL_AC21_IS_GPIO
#define BALL_AC21_IS_GPIO 0
#endif
#ifndef PAD_CI_D4_IS_GPIO
#define PAD_CI_D4_IS_GPIO BALL_AC21_IS_GPIO
#endif

#define BALL_AC22 PAD_CI_D5
#define PAD_CI_D5 49
#define GPIO_PAD_50 GPIO49
#ifndef BALL_AC22_IS_GPIO
#define BALL_AC22_IS_GPIO 0
#endif
#ifndef PAD_CI_D5_IS_GPIO
#define PAD_CI_D5_IS_GPIO BALL_AC22_IS_GPIO
#endif

#define BALL_AD22 PAD_CI_D6
#define PAD_CI_D6 50
#define GPIO_PAD_51 GPIO50
#ifndef BALL_AD22_IS_GPIO
#define BALL_AD22_IS_GPIO 0
#endif
#ifndef PAD_CI_D6_IS_GPIO
#define PAD_CI_D6_IS_GPIO BALL_AD22_IS_GPIO
#endif

#define BALL_AE22 PAD_CI_D7
#define PAD_CI_D7 51
#define GPIO_PAD_52 GPIO51
#ifndef BALL_AE22_IS_GPIO
#define BALL_AE22_IS_GPIO 0
#endif
#ifndef PAD_CI_D7_IS_GPIO
#define PAD_CI_D7_IS_GPIO BALL_AE22_IS_GPIO
#endif

#define BALL_AG26 PAD_CI_IRQAZ
#define PAD_CI_IRQAZ 52
#define GPIO_PAD_53 GPIO52
#ifndef BALL_AG26_IS_GPIO
#define BALL_AG26_IS_GPIO 0
#endif
#ifndef PAD_CI_IRQAZ_IS_GPIO
#define PAD_CI_IRQAZ_IS_GPIO BALL_AG26_IS_GPIO
#endif

#define BALL_AH28 PAD_CI_RST
#define PAD_CI_RST 53
#define GPIO_PAD_54 GPIO53
#ifndef BALL_AH28_IS_GPIO
#define BALL_AH28_IS_GPIO 0
#endif
#ifndef PAD_CI_RST_IS_GPIO
#define PAD_CI_RST_IS_GPIO BALL_AH28_IS_GPIO
#endif

#define BALL_AE25 PAD_CI_IORDZ
#define PAD_CI_IORDZ 54
#define GPIO_PAD_55 GPIO54
#ifndef BALL_AE25_IS_GPIO
#define BALL_AE25_IS_GPIO 0
#endif
#ifndef PAD_CI_IORDZ_IS_GPIO
#define PAD_CI_IORDZ_IS_GPIO BALL_AE25_IS_GPIO
#endif

#define BALL_AD24 PAD_CI_IOWRZ
#define PAD_CI_IOWRZ 55
#define GPIO_PAD_56 GPIO55
#ifndef BALL_AD24_IS_GPIO
#define BALL_AD24_IS_GPIO 0
#endif
#ifndef PAD_CI_IOWRZ_IS_GPIO
#define PAD_CI_IOWRZ_IS_GPIO BALL_AD24_IS_GPIO
#endif

#define BALL_AE24 PAD_CI_OEZ
#define PAD_CI_OEZ 56
#define GPIO_PAD_57 GPIO56
#ifndef BALL_AE24_IS_GPIO
#define BALL_AE24_IS_GPIO 0
#endif
#ifndef PAD_CI_OEZ_IS_GPIO
#define PAD_CI_OEZ_IS_GPIO BALL_AE24_IS_GPIO
#endif

#define BALL_AH26 PAD_CI_WEZ
#define PAD_CI_WEZ 57
#define GPIO_PAD_58 GPIO57
#ifndef BALL_AH26_IS_GPIO
#define BALL_AH26_IS_GPIO 0
#endif
#ifndef PAD_CI_WEZ_IS_GPIO
#define PAD_CI_WEZ_IS_GPIO BALL_AH26_IS_GPIO
#endif

#define BALL_AE27 PAD_CI_REGZ
#define PAD_CI_REGZ 58
#define GPIO_PAD_59 GPIO58
#ifndef BALL_AE27_IS_GPIO
#define BALL_AE27_IS_GPIO 0
#endif
#ifndef PAD_CI_REGZ_IS_GPIO
#define PAD_CI_REGZ_IS_GPIO BALL_AE27_IS_GPIO
#endif

#define BALL_AD23 PAD_CI_CEZ
#define PAD_CI_CEZ 59
#define GPIO_PAD_60 GPIO59
#ifndef BALL_AD23_IS_GPIO
#define BALL_AD23_IS_GPIO 0
#endif
#ifndef PAD_CI_CEZ_IS_GPIO
#define PAD_CI_CEZ_IS_GPIO BALL_AD23_IS_GPIO
#endif

#define BALL_AE26 PAD_CI_WAITZ
#define PAD_CI_WAITZ 60
#define GPIO_PAD_61 GPIO60
#ifndef BALL_AE26_IS_GPIO
#define BALL_AE26_IS_GPIO 0
#endif
#ifndef PAD_CI_WAITZ_IS_GPIO
#define PAD_CI_WAITZ_IS_GPIO BALL_AE26_IS_GPIO
#endif

#define BALL_AE21 PAD_CI_CDZ
#define PAD_CI_CDZ 61
#define GPIO_PAD_62 GPIO61
#ifndef BALL_AE21_IS_GPIO
#define BALL_AE21_IS_GPIO 0
#endif
#ifndef PAD_CI_CDZ_IS_GPIO
#define PAD_CI_CDZ_IS_GPIO BALL_AE21_IS_GPIO
#endif

#define BALL_AF18 PAD_I2CM1_SDA
#define PAD_I2CM1_SDA 62
#define GPIO_PAD_63 GPIO62
#ifndef BALL_AF18_IS_GPIO
#define BALL_AF18_IS_GPIO 0
#endif
#ifndef PAD_I2CM1_SDA_IS_GPIO
#define PAD_I2CM1_SDA_IS_GPIO BALL_AF18_IS_GPIO
#endif

#define BALL_AG19 PAD_I2CM1_SCL
#define PAD_I2CM1_SCL 63
#define GPIO_PAD_64 GPIO63
#ifndef BALL_AG19_IS_GPIO
#define BALL_AG19_IS_GPIO 0
#endif
#ifndef PAD_I2CM1_SCL_IS_GPIO
#define PAD_I2CM1_SCL_IS_GPIO BALL_AG19_IS_GPIO
#endif

#define BALL_Y23 PAD_TS3_CLK
#define PAD_TS3_CLK 64
#define GPIO_PAD_65 GPIO64
#ifndef BALL_Y23_IS_GPIO
#define BALL_Y23_IS_GPIO 0
#endif
#ifndef PAD_TS3_CLK_IS_GPIO
#define PAD_TS3_CLK_IS_GPIO BALL_Y23_IS_GPIO
#endif

#define BALL_AA27 PAD_TS3_SYNC
#define PAD_TS3_SYNC 65
#define GPIO_PAD_66 GPIO65
#ifndef BALL_AA27_IS_GPIO
#define BALL_AA27_IS_GPIO 0
#endif
#ifndef PAD_TS3_SYNC_IS_GPIO
#define PAD_TS3_SYNC_IS_GPIO BALL_AA27_IS_GPIO
#endif

#define BALL_AA28 PAD_TS3_VLD
#define PAD_TS3_VLD 66
#define GPIO_PAD_67 GPIO66
#ifndef BALL_AA28_IS_GPIO
#define BALL_AA28_IS_GPIO 0
#endif
#ifndef PAD_TS3_VLD_IS_GPIO
#define PAD_TS3_VLD_IS_GPIO BALL_AA28_IS_GPIO
#endif

#define BALL_AA26 PAD_TS3_D0
#define PAD_TS3_D0 67
#define GPIO_PAD_68 GPIO67
#ifndef BALL_AA26_IS_GPIO
#define BALL_AA26_IS_GPIO 0
#endif
#ifndef PAD_TS3_D0_IS_GPIO
#define PAD_TS3_D0_IS_GPIO BALL_AA26_IS_GPIO
#endif

#define BALL_AB27 PAD_TS3_D1
#define PAD_TS3_D1 68
#define GPIO_PAD_69 GPIO68
#ifndef BALL_AB27_IS_GPIO
#define BALL_AB27_IS_GPIO 0
#endif
#ifndef PAD_TS3_D1_IS_GPIO
#define PAD_TS3_D1_IS_GPIO BALL_AB27_IS_GPIO
#endif

#define BALL_AB26 PAD_TS3_D2
#define PAD_TS3_D2 69
#define GPIO_PAD_70 GPIO69
#ifndef BALL_AB26_IS_GPIO
#define BALL_AB26_IS_GPIO 0
#endif
#ifndef PAD_TS3_D2_IS_GPIO
#define PAD_TS3_D2_IS_GPIO BALL_AB26_IS_GPIO
#endif

#define BALL_W24 PAD_TS3_D3
#define PAD_TS3_D3 70
#define GPIO_PAD_71 GPIO70
#ifndef BALL_W24_IS_GPIO
#define BALL_W24_IS_GPIO 0
#endif
#ifndef PAD_TS3_D3_IS_GPIO
#define PAD_TS3_D3_IS_GPIO BALL_W24_IS_GPIO
#endif

#define BALL_Y25 PAD_TS3_D4
#define PAD_TS3_D4 71
#define GPIO_PAD_72 GPIO71
#ifndef BALL_Y25_IS_GPIO
#define BALL_Y25_IS_GPIO 0
#endif
#ifndef PAD_TS3_D4_IS_GPIO
#define PAD_TS3_D4_IS_GPIO BALL_Y25_IS_GPIO
#endif

#define BALL_Y24 PAD_TS3_D5
#define PAD_TS3_D5 72
#define GPIO_PAD_73 GPIO72
#ifndef BALL_Y24_IS_GPIO
#define BALL_Y24_IS_GPIO 0
#endif
#ifndef PAD_TS3_D5_IS_GPIO
#define PAD_TS3_D5_IS_GPIO BALL_Y24_IS_GPIO
#endif

#define BALL_AA25 PAD_TS3_D6
#define PAD_TS3_D6 73
#define GPIO_PAD_74 GPIO73
#ifndef BALL_AA25_IS_GPIO
#define BALL_AA25_IS_GPIO 0
#endif
#ifndef PAD_TS3_D6_IS_GPIO
#define PAD_TS3_D6_IS_GPIO BALL_AA25_IS_GPIO
#endif

#define BALL_AA24 PAD_TS3_D7
#define PAD_TS3_D7 74
#define GPIO_PAD_75 GPIO74
#ifndef BALL_AA24_IS_GPIO
#define BALL_AA24_IS_GPIO 0
#endif
#ifndef PAD_TS3_D7_IS_GPIO
#define PAD_TS3_D7_IS_GPIO BALL_AA24_IS_GPIO
#endif

#define BALL_J25 PAD_I2CM0_SDA
#define PAD_I2CM0_SDA 75
#define GPIO_PAD_76 GPIO75
#ifndef BALL_J25_IS_GPIO
#define BALL_J25_IS_GPIO 0
#endif
#ifndef PAD_I2CM0_SDA_IS_GPIO
#define PAD_I2CM0_SDA_IS_GPIO BALL_J25_IS_GPIO
#endif

#define BALL_J24 PAD_I2CM0_SCL
#define PAD_I2CM0_SCL 76
#define GPIO_PAD_77 GPIO76
#ifndef BALL_J24_IS_GPIO
#define BALL_J24_IS_GPIO 0
#endif
#ifndef PAD_I2CM0_SCL_IS_GPIO
#define PAD_I2CM0_SCL_IS_GPIO BALL_J24_IS_GPIO
#endif

#define BALL_C23 PAD_SPDIF_OUT
#define PAD_SPDIF_OUT 77
#define GPIO_PAD_78 GPIO77
#ifndef BALL_C23_IS_GPIO
#define BALL_C23_IS_GPIO 0
#endif
#ifndef PAD_SPDIF_OUT_IS_GPIO
#define PAD_SPDIF_OUT_IS_GPIO BALL_C23_IS_GPIO
#endif

#define BALL_AB23 PAD_EJ_TDO
#define PAD_EJ_TDO 78
#define GPIO_PAD_79 GPIO78
#ifndef BALL_AB23_IS_GPIO
#define BALL_AB23_IS_GPIO 0
#endif
#ifndef PAD_EJ_TDO_IS_GPIO
#define PAD_EJ_TDO_IS_GPIO BALL_AB23_IS_GPIO
#endif

#define BALL_AB22 PAD_EJ_TDI
#define PAD_EJ_TDI 79
#define GPIO_PAD_80 GPIO79
#ifndef BALL_AB22_IS_GPIO
#define BALL_AB22_IS_GPIO 0
#endif
#ifndef PAD_EJ_TDI_IS_GPIO
#define PAD_EJ_TDI_IS_GPIO BALL_AB22_IS_GPIO
#endif

#define BALL_AB25 PAD_EJ_TMS
#define PAD_EJ_TMS 80
#define GPIO_PAD_81 GPIO80
#ifndef BALL_AB25_IS_GPIO
#define BALL_AB25_IS_GPIO 0
#endif
#ifndef PAD_EJ_TMS_IS_GPIO
#define PAD_EJ_TMS_IS_GPIO BALL_AB25_IS_GPIO
#endif

#define BALL_AB24 PAD_EJ_TCK
#define PAD_EJ_TCK 81
#define GPIO_PAD_82 GPIO81
#ifndef BALL_AB24_IS_GPIO
#define BALL_AB24_IS_GPIO 0
#endif
#ifndef PAD_EJ_TCK_IS_GPIO
#define PAD_EJ_TCK_IS_GPIO BALL_AB24_IS_GPIO
#endif

#define BALL_AA23 PAD_EJ_TRST_N
#define PAD_EJ_TRST_N 82
#define GPIO_PAD_83 GPIO82
#ifndef BALL_AA23_IS_GPIO
#define BALL_AA23_IS_GPIO 0
#endif
#ifndef PAD_EJ_TRST_N_IS_GPIO
#define PAD_EJ_TRST_N_IS_GPIO BALL_AA23_IS_GPIO
#endif

#define BALL_W23 PAD_TS2_CLK
#define PAD_TS2_CLK 83
#define GPIO_PAD_84 GPIO83
#ifndef BALL_W23_IS_GPIO
#define BALL_W23_IS_GPIO 0
#endif
#ifndef PAD_TS2_CLK_IS_GPIO
#define PAD_TS2_CLK_IS_GPIO BALL_W23_IS_GPIO
#endif

#define BALL_U23 PAD_TS2_SYNC
#define PAD_TS2_SYNC 84
#define GPIO_PAD_85 GPIO84
#ifndef BALL_U23_IS_GPIO
#define BALL_U23_IS_GPIO 0
#endif
#ifndef PAD_TS2_SYNC_IS_GPIO
#define PAD_TS2_SYNC_IS_GPIO BALL_U23_IS_GPIO
#endif

#define BALL_V23 PAD_TS2_VLD
#define PAD_TS2_VLD 85
#define GPIO_PAD_86 GPIO85
#ifndef BALL_V23_IS_GPIO
#define BALL_V23_IS_GPIO 0
#endif
#ifndef PAD_TS2_VLD_IS_GPIO
#define PAD_TS2_VLD_IS_GPIO BALL_V23_IS_GPIO
#endif

#define BALL_V24 PAD_TS2_D0
#define PAD_TS2_D0 86
#define GPIO_PAD_87 GPIO86
#ifndef BALL_V24_IS_GPIO
#define BALL_V24_IS_GPIO 0
#endif
#ifndef PAD_TS2_D0_IS_GPIO
#define PAD_TS2_D0_IS_GPIO BALL_V24_IS_GPIO
#endif

#define BALL_V25 PAD_TS2_D1
#define PAD_TS2_D1 87
#define GPIO_PAD_88 GPIO87
#ifndef BALL_V25_IS_GPIO
#define BALL_V25_IS_GPIO 0
#endif
#ifndef PAD_TS2_D1_IS_GPIO
#define PAD_TS2_D1_IS_GPIO BALL_V25_IS_GPIO
#endif

#define BALL_U24 PAD_TS2_D2
#define PAD_TS2_D2 88
#define GPIO_PAD_89 GPIO88
#ifndef BALL_U24_IS_GPIO
#define BALL_U24_IS_GPIO 0
#endif
#ifndef PAD_TS2_D2_IS_GPIO
#define PAD_TS2_D2_IS_GPIO BALL_U24_IS_GPIO
#endif

#define BALL_U25 PAD_TS2_D3
#define PAD_TS2_D3 89
#define GPIO_PAD_90 GPIO89
#ifndef BALL_U25_IS_GPIO
#define BALL_U25_IS_GPIO 0
#endif
#ifndef PAD_TS2_D3_IS_GPIO
#define PAD_TS2_D3_IS_GPIO BALL_U25_IS_GPIO
#endif

#define BALL_T24 PAD_TS2_D4
#define PAD_TS2_D4 90
#define GPIO_PAD_91 GPIO90
#ifndef BALL_T24_IS_GPIO
#define BALL_T24_IS_GPIO 0
#endif
#ifndef PAD_TS2_D4_IS_GPIO
#define PAD_TS2_D4_IS_GPIO BALL_T24_IS_GPIO
#endif

#define BALL_T23 PAD_TS2_D5
#define PAD_TS2_D5 91
#define GPIO_PAD_92 GPIO91
#ifndef BALL_T23_IS_GPIO
#define BALL_T23_IS_GPIO 0
#endif
#ifndef PAD_TS2_D5_IS_GPIO
#define PAD_TS2_D5_IS_GPIO BALL_T23_IS_GPIO
#endif

#define BALL_R23 PAD_TS2_D6
#define PAD_TS2_D6 92
#define GPIO_PAD_93 GPIO92
#ifndef BALL_R23_IS_GPIO
#define BALL_R23_IS_GPIO 0
#endif
#ifndef PAD_TS2_D6_IS_GPIO
#define PAD_TS2_D6_IS_GPIO BALL_R23_IS_GPIO
#endif

#define BALL_P23 PAD_TS2_D7
#define PAD_TS2_D7 93
#define GPIO_PAD_94 GPIO93
#ifndef BALL_P23_IS_GPIO
#define BALL_P23_IS_GPIO 0
#endif
#ifndef PAD_TS2_D7_IS_GPIO
#define PAD_TS2_D7_IS_GPIO BALL_P23_IS_GPIO
#endif

#define BALL_V26 PAD_TS1_CLK
#define PAD_TS1_CLK 94
#define GPIO_PAD_95 GPIO94
#ifndef BALL_V26_IS_GPIO
#define BALL_V26_IS_GPIO 0
#endif
#ifndef PAD_TS1_CLK_IS_GPIO
#define PAD_TS1_CLK_IS_GPIO BALL_V26_IS_GPIO
#endif

#define BALL_W26 PAD_TS1_SYNC
#define PAD_TS1_SYNC 95
#define GPIO_PAD_96 GPIO95
#ifndef BALL_W26_IS_GPIO
#define BALL_W26_IS_GPIO 0
#endif
#ifndef PAD_TS1_SYNC_IS_GPIO
#define PAD_TS1_SYNC_IS_GPIO BALL_W26_IS_GPIO
#endif

#define BALL_W27 PAD_TS1_VLD
#define PAD_TS1_VLD 96
#define GPIO_PAD_97 GPIO96
#ifndef BALL_W27_IS_GPIO
#define BALL_W27_IS_GPIO 0
#endif
#ifndef PAD_TS1_VLD_IS_GPIO
#define PAD_TS1_VLD_IS_GPIO BALL_W27_IS_GPIO
#endif

#define BALL_Y27 PAD_TS1_D0
#define PAD_TS1_D0 97
#define GPIO_PAD_98 GPIO97
#ifndef BALL_Y27_IS_GPIO
#define BALL_Y27_IS_GPIO 0
#endif
#ifndef PAD_TS1_D0_IS_GPIO
#define PAD_TS1_D0_IS_GPIO BALL_Y27_IS_GPIO
#endif

#define BALL_Y28 PAD_TS1_D1
#define PAD_TS1_D1 98
#define GPIO_PAD_99 GPIO98
#ifndef BALL_Y28_IS_GPIO
#define BALL_Y28_IS_GPIO 0
#endif
#ifndef PAD_TS1_D1_IS_GPIO
#define PAD_TS1_D1_IS_GPIO BALL_Y28_IS_GPIO
#endif

#define BALL_Y26 PAD_TS1_D2
#define PAD_TS1_D2 99
#define GPIO_PAD_100 GPIO99
#ifndef BALL_Y26_IS_GPIO
#define BALL_Y26_IS_GPIO 0
#endif
#ifndef PAD_TS1_D2_IS_GPIO
#define PAD_TS1_D2_IS_GPIO BALL_Y26_IS_GPIO
#endif

#define BALL_U27 PAD_TS1_D3
#define PAD_TS1_D3 100
#define GPIO_PAD_101 GPIO100
#ifndef BALL_U27_IS_GPIO
#define BALL_U27_IS_GPIO 0
#endif
#ifndef PAD_TS1_D3_IS_GPIO
#define PAD_TS1_D3_IS_GPIO BALL_U27_IS_GPIO
#endif

#define BALL_U28 PAD_TS1_D4
#define PAD_TS1_D4 101
#define GPIO_PAD_102 GPIO101
#ifndef BALL_U28_IS_GPIO
#define BALL_U28_IS_GPIO 0
#endif
#ifndef PAD_TS1_D4_IS_GPIO
#define PAD_TS1_D4_IS_GPIO BALL_U28_IS_GPIO
#endif

#define BALL_U26 PAD_TS1_D5
#define PAD_TS1_D5 102
#define GPIO_PAD_103 GPIO102
#ifndef BALL_U26_IS_GPIO
#define BALL_U26_IS_GPIO 0
#endif
#ifndef PAD_TS1_D5_IS_GPIO
#define PAD_TS1_D5_IS_GPIO BALL_U26_IS_GPIO
#endif

#define BALL_V28 PAD_TS1_D6
#define PAD_TS1_D6 103
#define GPIO_PAD_104 GPIO103
#ifndef BALL_V28_IS_GPIO
#define BALL_V28_IS_GPIO 0
#endif
#ifndef PAD_TS1_D6_IS_GPIO
#define PAD_TS1_D6_IS_GPIO BALL_V28_IS_GPIO
#endif

#define BALL_V27 PAD_TS1_D7
#define PAD_TS1_D7 104
#define GPIO_PAD_105 GPIO104
#ifndef BALL_V27_IS_GPIO
#define BALL_V27_IS_GPIO 0
#endif
#ifndef PAD_TS1_D7_IS_GPIO
#define PAD_TS1_D7_IS_GPIO BALL_V27_IS_GPIO
#endif

#define BALL_R24 PAD_TS0_CLK
#define PAD_TS0_CLK 105
#define GPIO_PAD_106 GPIO105
#ifndef BALL_R24_IS_GPIO
#define BALL_R24_IS_GPIO 0
#endif
#ifndef PAD_TS0_CLK_IS_GPIO
#define PAD_TS0_CLK_IS_GPIO BALL_R24_IS_GPIO
#endif

#define BALL_P24 PAD_TS0_SYNC
#define PAD_TS0_SYNC 106
#define GPIO_PAD_107 GPIO106
#ifndef BALL_P24_IS_GPIO
#define BALL_P24_IS_GPIO 0
#endif
#ifndef PAD_TS0_SYNC_IS_GPIO
#define PAD_TS0_SYNC_IS_GPIO BALL_P24_IS_GPIO
#endif

#define BALL_R25 PAD_TS0_VLD
#define PAD_TS0_VLD 107
#define GPIO_PAD_108 GPIO107
#ifndef BALL_R25_IS_GPIO
#define BALL_R25_IS_GPIO 0
#endif
#ifndef PAD_TS0_VLD_IS_GPIO
#define PAD_TS0_VLD_IS_GPIO BALL_R25_IS_GPIO
#endif

#define BALL_P25 PAD_TS0_D0
#define PAD_TS0_D0 108
#define GPIO_PAD_109 GPIO108
#ifndef BALL_P25_IS_GPIO
#define BALL_P25_IS_GPIO 0
#endif
#ifndef PAD_TS0_D0_IS_GPIO
#define PAD_TS0_D0_IS_GPIO BALL_P25_IS_GPIO
#endif

#define BALL_N24 PAD_TS0_D1
#define PAD_TS0_D1 109
#define GPIO_PAD_110 GPIO109
#ifndef BALL_N24_IS_GPIO
#define BALL_N24_IS_GPIO 0
#endif
#ifndef PAD_TS0_D1_IS_GPIO
#define PAD_TS0_D1_IS_GPIO BALL_N24_IS_GPIO
#endif

#define BALL_N23 PAD_TS0_D2
#define PAD_TS0_D2 110
#define GPIO_PAD_111 GPIO110
#ifndef BALL_N23_IS_GPIO
#define BALL_N23_IS_GPIO 0
#endif
#ifndef PAD_TS0_D2_IS_GPIO
#define PAD_TS0_D2_IS_GPIO BALL_N23_IS_GPIO
#endif

#define BALL_M23 PAD_TS0_D3
#define PAD_TS0_D3 111
#define GPIO_PAD_112 GPIO111
#ifndef BALL_M23_IS_GPIO
#define BALL_M23_IS_GPIO 0
#endif
#ifndef PAD_TS0_D3_IS_GPIO
#define PAD_TS0_D3_IS_GPIO BALL_M23_IS_GPIO
#endif

#define BALL_L23 PAD_TS0_D4
#define PAD_TS0_D4 112
#define GPIO_PAD_113 GPIO112
#ifndef BALL_L23_IS_GPIO
#define BALL_L23_IS_GPIO 0
#endif
#ifndef PAD_TS0_D4_IS_GPIO
#define PAD_TS0_D4_IS_GPIO BALL_L23_IS_GPIO
#endif

#define BALL_M24 PAD_TS0_D5
#define PAD_TS0_D5 113
#define GPIO_PAD_114 GPIO113
#ifndef BALL_M24_IS_GPIO
#define BALL_M24_IS_GPIO 0
#endif
#ifndef PAD_TS0_D5_IS_GPIO
#define PAD_TS0_D5_IS_GPIO BALL_M24_IS_GPIO
#endif

#define BALL_M25 PAD_TS0_D6
#define PAD_TS0_D6 114
#define GPIO_PAD_115 GPIO114
#ifndef BALL_M25_IS_GPIO
#define BALL_M25_IS_GPIO 0
#endif
#ifndef PAD_TS0_D6_IS_GPIO
#define PAD_TS0_D6_IS_GPIO BALL_M25_IS_GPIO
#endif

#define BALL_L24 PAD_TS0_D7
#define PAD_TS0_D7 115
#define GPIO_PAD_116 GPIO115
#ifndef BALL_L24_IS_GPIO
#define BALL_L24_IS_GPIO 0
#endif
#ifndef PAD_TS0_D7_IS_GPIO
#define PAD_TS0_D7_IS_GPIO BALL_L24_IS_GPIO
#endif

#define BALL_M28 PAD_NF_CLE
#define PAD_NF_CLE 116
#define GPIO_PAD_117 GPIO116
#ifndef BALL_M28_IS_GPIO
#define BALL_M28_IS_GPIO 0
#endif
#ifndef PAD_NF_CLE_IS_GPIO
#define PAD_NF_CLE_IS_GPIO BALL_M28_IS_GPIO
#endif

#define BALL_L27 PAD_NF_ALE
#define PAD_NF_ALE 117
#define GPIO_PAD_118 GPIO117
#ifndef BALL_L27_IS_GPIO
#define BALL_L27_IS_GPIO 0
#endif
#ifndef PAD_NF_ALE_IS_GPIO
#define PAD_NF_ALE_IS_GPIO BALL_L27_IS_GPIO
#endif

#define BALL_M26 PAD_NF_WEZ
#define PAD_NF_WEZ 118
#define GPIO_PAD_119 GPIO118
#ifndef BALL_M26_IS_GPIO
#define BALL_M26_IS_GPIO 0
#endif
#ifndef PAD_NF_WEZ_IS_GPIO
#define PAD_NF_WEZ_IS_GPIO BALL_M26_IS_GPIO
#endif

#define BALL_L28 PAD_NF_WPZ
#define PAD_NF_WPZ 119
#define GPIO_PAD_120 GPIO119
#ifndef BALL_L28_IS_GPIO
#define BALL_L28_IS_GPIO 0
#endif
#ifndef PAD_NF_WPZ_IS_GPIO
#define PAD_NF_WPZ_IS_GPIO BALL_L28_IS_GPIO
#endif

#define BALL_P27 PAD_NF_D0
#define PAD_NF_D0 120
#define GPIO_PAD_121 GPIO120
#ifndef BALL_P27_IS_GPIO
#define BALL_P27_IS_GPIO 0
#endif
#ifndef PAD_NF_D0_IS_GPIO
#define PAD_NF_D0_IS_GPIO BALL_P27_IS_GPIO
#endif

#define BALL_N27 PAD_NF_D1
#define PAD_NF_D1 121
#define GPIO_PAD_122 GPIO121
#ifndef BALL_N27_IS_GPIO
#define BALL_N27_IS_GPIO 0
#endif
#ifndef PAD_NF_D1_IS_GPIO
#define PAD_NF_D1_IS_GPIO BALL_N27_IS_GPIO
#endif

#define BALL_N26 PAD_NF_D2
#define PAD_NF_D2 122
#define GPIO_PAD_123 GPIO122
#ifndef BALL_N26_IS_GPIO
#define BALL_N26_IS_GPIO 0
#endif
#ifndef PAD_NF_D2_IS_GPIO
#define PAD_NF_D2_IS_GPIO BALL_N26_IS_GPIO
#endif

#define BALL_P28 PAD_NF_D3
#define PAD_NF_D3 123
#define GPIO_PAD_124 GPIO123
#ifndef BALL_P28_IS_GPIO
#define BALL_P28_IS_GPIO 0
#endif
#ifndef PAD_NF_D3_IS_GPIO
#define PAD_NF_D3_IS_GPIO BALL_P28_IS_GPIO
#endif

#define BALL_R28 PAD_NF_D4
#define PAD_NF_D4 124
#define GPIO_PAD_125 GPIO124
#ifndef BALL_R28_IS_GPIO
#define BALL_R28_IS_GPIO 0
#endif
#ifndef PAD_NF_D4_IS_GPIO
#define PAD_NF_D4_IS_GPIO BALL_R28_IS_GPIO
#endif

#define BALL_R27 PAD_NF_D5
#define PAD_NF_D5 125
#define GPIO_PAD_126 GPIO125
#ifndef BALL_R27_IS_GPIO
#define BALL_R27_IS_GPIO 0
#endif
#ifndef PAD_NF_D5_IS_GPIO
#define PAD_NF_D5_IS_GPIO BALL_R27_IS_GPIO
#endif

#define BALL_R26 PAD_NF_D6
#define PAD_NF_D6 126
#define GPIO_PAD_127 GPIO126
#ifndef BALL_R26_IS_GPIO
#define BALL_R26_IS_GPIO 0
#endif
#ifndef PAD_NF_D6_IS_GPIO
#define PAD_NF_D6_IS_GPIO BALL_R26_IS_GPIO
#endif

#define BALL_T27 PAD_NF_D7
#define PAD_NF_D7 127
#define GPIO_PAD_128 GPIO127
#ifndef BALL_T27_IS_GPIO
#define BALL_T27_IS_GPIO 0
#endif
#ifndef PAD_NF_D7_IS_GPIO
#define PAD_NF_D7_IS_GPIO BALL_T27_IS_GPIO
#endif

#define BALL_P26 PAD_NF_RBZ
#define PAD_NF_RBZ 128
#define GPIO_PAD_129 GPIO128
#ifndef BALL_P26_IS_GPIO
#define BALL_P26_IS_GPIO 0
#endif
#ifndef PAD_NF_RBZ_IS_GPIO
#define PAD_NF_RBZ_IS_GPIO BALL_P26_IS_GPIO
#endif

#define BALL_L26 PAD_NF_REZ
#define PAD_NF_REZ 129
#define GPIO_PAD_130 GPIO129
#ifndef BALL_L26_IS_GPIO
#define BALL_L26_IS_GPIO 0
#endif
#ifndef PAD_NF_REZ_IS_GPIO
#define PAD_NF_REZ_IS_GPIO BALL_L26_IS_GPIO
#endif

#define BALL_M27 PAD_NF_CEZ_BGA
#define PAD_NF_CEZ_BGA 130
#define GPIO_PAD_131 GPIO130
#ifndef BALL_M27_IS_GPIO
#define BALL_M27_IS_GPIO 0
#endif
#ifndef PAD_NF_CEZ_BGA_IS_GPIO
#define PAD_NF_CEZ_BGA_IS_GPIO BALL_M27_IS_GPIO
#endif

#define BALL_T26 PAD_NF_CE1Z_BGA
#define PAD_NF_CE1Z_BGA 131
#define GPIO_PAD_132 GPIO131
#ifndef BALL_T26_IS_GPIO
#define BALL_T26_IS_GPIO 0
#endif
#ifndef PAD_NF_CE1Z_BGA_IS_GPIO
#define PAD_NF_CE1Z_BGA_IS_GPIO BALL_T26_IS_GPIO
#endif

#define BALL_K23 PAD_DM_GPIO1
#define PAD_DM_GPIO1 132
#define GPIO_PAD_133 GPIO132
#ifndef BALL_K23_IS_GPIO
#define BALL_K23_IS_GPIO 0
#endif
#ifndef PAD_DM_GPIO1_IS_GPIO
#define PAD_DM_GPIO1_IS_GPIO BALL_K23_IS_GPIO
#endif

#define BALL_H23 PAD_DM_GPIO0
#define PAD_DM_GPIO0 133
#define GPIO_PAD_134 GPIO133
#ifndef BALL_H23_IS_GPIO
#define BALL_H23_IS_GPIO 0
#endif
#ifndef PAD_DM_GPIO0_IS_GPIO
#define PAD_DM_GPIO0_IS_GPIO BALL_H23_IS_GPIO
#endif

#define BALL_K24 PAD_S_GPIO0
#define PAD_S_GPIO0 134
#define GPIO_PAD_135 GPIO134
#ifndef BALL_K24_IS_GPIO
#define BALL_K24_IS_GPIO 0
#endif
#ifndef PAD_S_GPIO0_IS_GPIO
#define PAD_S_GPIO0_IS_GPIO BALL_K24_IS_GPIO
#endif

#define BALL_AH19 PAD_S_GPIO1
#define PAD_S_GPIO1 135
#define GPIO_PAD_136 GPIO135
#ifndef BALL_AH19_IS_GPIO
#define BALL_AH19_IS_GPIO 0
#endif
#ifndef PAD_S_GPIO1_IS_GPIO
#define PAD_S_GPIO1_IS_GPIO BALL_AH19_IS_GPIO
#endif

#define BALL_J23 PAD_S_GPIO2
#define PAD_S_GPIO2 136
#define GPIO_PAD_137 GPIO136
#ifndef BALL_J23_IS_GPIO
#define BALL_J23_IS_GPIO 0
#endif
#ifndef PAD_S_GPIO2_IS_GPIO
#define PAD_S_GPIO2_IS_GPIO BALL_J23_IS_GPIO
#endif

#define BALL_F25 PAD_VSYNC_OUT
#define PAD_VSYNC_OUT 137
#define GPIO_PAD_138 GPIO137
#ifndef BALL_F25_IS_GPIO
#define BALL_F25_IS_GPIO 0
#endif
#ifndef PAD_VSYNC_OUT_IS_GPIO
#define PAD_VSYNC_OUT_IS_GPIO BALL_F25_IS_GPIO
#endif

#define BALL_E25 PAD_HSYNC_OUT
#define PAD_HSYNC_OUT 138
#define GPIO_PAD_139 GPIO138
#ifndef BALL_E25_IS_GPIO
#define BALL_E25_IS_GPIO 0
#endif
#ifndef PAD_HSYNC_OUT_IS_GPIO
#define PAD_HSYNC_OUT_IS_GPIO BALL_E25_IS_GPIO
#endif

#define BALL_F26 PAD_GPIO_PM0
#define PAD_GPIO_PM0 139
#define GPIO_PAD_140 GPIO139
#ifndef BALL_F26_IS_GPIO
#define BALL_F26_IS_GPIO 0
#endif
#ifndef PAD_GPIO_PM0_IS_GPIO
#define PAD_GPIO_PM0_IS_GPIO BALL_F26_IS_GPIO
#endif

#define BALL_D22 PAD_GPIO_PM1
#define PAD_GPIO_PM1 140
#define GPIO_PAD_141 GPIO140
#ifndef BALL_D22_IS_GPIO
#define BALL_D22_IS_GPIO 0
#endif
#ifndef PAD_GPIO_PM1_IS_GPIO
#define PAD_GPIO_PM1_IS_GPIO BALL_D22_IS_GPIO
#endif

#define BALL_A24 PAD_GPIO_PM2
#define PAD_GPIO_PM2 141
#define GPIO_PAD_142 GPIO141
#ifndef BALL_A24_IS_GPIO
#define BALL_A24_IS_GPIO 0
#endif
#ifndef PAD_GPIO_PM2_IS_GPIO
#define PAD_GPIO_PM2_IS_GPIO BALL_A24_IS_GPIO
#endif

#define BALL_D23 PAD_GPIO_PM3
#define PAD_GPIO_PM3 142
#define GPIO_PAD_143 GPIO142
#ifndef BALL_D23_IS_GPIO
#define BALL_D23_IS_GPIO 0
#endif
#ifndef PAD_GPIO_PM3_IS_GPIO
#define PAD_GPIO_PM3_IS_GPIO BALL_D23_IS_GPIO
#endif

#define BALL_E26 PAD_GPIO_PM4
#define PAD_GPIO_PM4 143
#define GPIO_PAD_144 GPIO143
#ifndef BALL_E26_IS_GPIO
#define BALL_E26_IS_GPIO 0
#endif
#ifndef PAD_GPIO_PM4_IS_GPIO
#define PAD_GPIO_PM4_IS_GPIO BALL_E26_IS_GPIO
#endif

#define BALL_E23 PAD_GPIO_PM5
#define PAD_GPIO_PM5 144
#define GPIO_PAD_145 GPIO144
#ifndef BALL_E23_IS_GPIO
#define BALL_E23_IS_GPIO 0
#endif
#ifndef PAD_GPIO_PM5_IS_GPIO
#define PAD_GPIO_PM5_IS_GPIO BALL_E23_IS_GPIO
#endif

#define BALL_L25 PAD_GPIO_PM6
#define PAD_GPIO_PM6 145
#define GPIO_PAD_146 GPIO145
#ifndef BALL_L25_IS_GPIO
#define BALL_L25_IS_GPIO 0
#endif
#ifndef PAD_GPIO_PM6_IS_GPIO
#define PAD_GPIO_PM6_IS_GPIO BALL_L25_IS_GPIO
#endif

#define BALL_D24 PAD_GPIO_PM7
#define PAD_GPIO_PM7 146
#define GPIO_PAD_147 GPIO146
#ifndef BALL_D24_IS_GPIO
#define BALL_D24_IS_GPIO 0
#endif
#ifndef PAD_GPIO_PM7_IS_GPIO
#define PAD_GPIO_PM7_IS_GPIO BALL_D24_IS_GPIO
#endif

#define BALL_F22 PAD_IRIN2
#define PAD_IRIN2 154
#define GPIO_PAD_155 GPIO154
#ifndef BALL_F22_IS_GPIO
#define BALL_F22_IS_GPIO 0
#endif
#ifndef PAD_IRIN2_IS_GPIO
#define PAD_IRIN2_IS_GPIO BALL_F22_IS_GPIO
#endif

#define BALL_E11 PAD_GPIO_PM8
#define PAD_GPIO_PM8 147
#define GPIO_PAD_148 GPIO147
#ifndef BALL_E11_IS_GPIO
#define BALL_E11_IS_GPIO 0
#endif
#ifndef PAD_GPIO_PM8_IS_GPIO
#define PAD_GPIO_PM8_IS_GPIO BALL_E11_IS_GPIO
#endif

#define BALL_AG18 PAD_IRIN
#define PAD_IRIN 152
#define GPIO_PAD_153 GPIO152
#ifndef BALL_AG18_IS_GPIO
#define BALL_AG18_IS_GPIO 0
#endif
#ifndef PAD_IRIN_IS_GPIO
#define PAD_IRIN_IS_GPIO BALL_AG18_IS_GPIO
#endif

#define BALL_A2 PAD_GPIO_PM9
#define PAD_GPIO_PM9 148
#define GPIO_PAD_149 GPIO148
#ifndef BALL_A2_IS_GPIO
#define BALL_A2_IS_GPIO 0
#endif
#ifndef PAD_GPIO_PM9_IS_GPIO
#define PAD_GPIO_PM9_IS_GPIO BALL_A2_IS_GPIO
#endif

#define BALL_E6 PAD_GPIO_PM10
#define PAD_GPIO_PM10 149
#define GPIO_PAD_150 GPIO149
#ifndef BALL_E6_IS_GPIO
#define BALL_E6_IS_GPIO 0
#endif
#ifndef PAD_GPIO_PM10_IS_GPIO
#define PAD_GPIO_PM10_IS_GPIO BALL_E6_IS_GPIO
#endif

#define BALL_B25 PAD_PM_SPI_CK
#define PAD_PM_SPI_CK 156
#define GPIO_PAD_157 GPIO156
#ifndef BALL_B25_IS_GPIO
#define BALL_B25_IS_GPIO 0
#endif
#ifndef PAD_PM_SPI_CK_IS_GPIO
#define PAD_PM_SPI_CK_IS_GPIO BALL_B25_IS_GPIO
#endif

#define BALL_E24 PAD_GPIO_PM11
#define PAD_GPIO_PM11 150
#define GPIO_PAD_151 GPIO150
#ifndef BALL_E24_IS_GPIO
#define BALL_E24_IS_GPIO 0
#endif
#ifndef PAD_GPIO_PM11_IS_GPIO
#define PAD_GPIO_PM11_IS_GPIO BALL_E24_IS_GPIO
#endif

#define BALL_A25 PAD_PM_SPI_DI
#define PAD_PM_SPI_DI 158
#define GPIO_PAD_158 GPIO157
#ifndef BALL_A25_IS_GPIO
#define BALL_A25_IS_GPIO 0
#endif
#ifndef PAD_PM_SPI_DI_IS_GPIO
#define PAD_PM_SPI_DI_IS_GPIO BALL_A25_IS_GPIO
#endif

#define BALL_E22 PAD_GPIO_PM12
#define PAD_GPIO_PM12 152
#define GPIO_PAD_152 GPIO151
#ifndef BALL_E22_IS_GPIO
#define BALL_E22_IS_GPIO 0
#endif
#ifndef PAD_GPIO_PM12_IS_GPIO
#define PAD_GPIO_PM12_IS_GPIO BALL_E22_IS_GPIO
#endif

#define BALL_C24 PAD_PM_SPI_DO
#define PAD_PM_SPI_DO 159
#define GPIO_PAD_159 GPIO158
#ifndef BALL_C24_IS_GPIO
#define BALL_C24_IS_GPIO 0
#endif
#ifndef PAD_PM_SPI_DO_IS_GPIO
#define PAD_PM_SPI_DO_IS_GPIO BALL_C24_IS_GPIO
#endif

#define BALL_B24 PAD_PM_SPI_CZ
#define PAD_PM_SPI_CZ 156
#define GPIO_PAD_156 GPIO155
#ifndef BALL_B24_IS_GPIO
#define BALL_B24_IS_GPIO 0
#endif
#ifndef PAD_PM_SPI_CZ_IS_GPIO
#define PAD_PM_SPI_CZ_IS_GPIO BALL_B24_IS_GPIO
#endif


#define GPIO_OEN    0   //set o to nake output
#define GPIO_ODN    1

#define IN_HIGH    1   //input high
#define IN_LOW     0   //input low

#define OUT_HIGH    1   //output high
#define OUT_LOW     0   //output low

#define MHal_GPIO_REG(addr)             (*(volatile U8*)(REG_MIPS_BASE + (((addr) & ~1)<<1) + (addr & 1)))

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

#endif // _REG_GPIO_H_

