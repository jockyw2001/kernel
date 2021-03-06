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

#ifndef IR_FORMAT_H
#define IR_FORMAT_H

//-------------------------------------------------------------------------------------------
// Customer IR Specification parameter define (Please modify them by IR SPEC)
//-------------------------------------------------------------------------------------------
#if defined(CONFIG_ENABLE_CUSTOMER_IR_PATH)
#define IR_TYPE_SEL                 IR_TYPE_NEW
#endif

#define IR_MODE_SEL             IR_TYPE_FULLDECODE_MODE
// IR Header code define
#define IR_HEADER_CODE0         0x41UL    // Custom 0
#define IR_HEADER_CODE1         0x01UL    // Custom 1

// IR Timing define
#define IR_HEADER_CODE_TIME     9000    // us
#define IR_OFF_CODE_TIME        4500    // us
#define IR_OFF_CODE_RP_TIME     2500    // us
#define IR_LOGI_01H_TIME        560     // us
#define IR_LOGI_0_TIME          1120    // us
#define IR_LOGI_1_TIME          2240    // us
#define IR_TIMEOUT_CYC          140000  // us

// IR Format define
#define IRKEY_DUMY              0xFFUL
#define IRDA_KEY_MAPPING_POWER  IRKEY_POWER

typedef enum _IrCommandType
{
    IRKEY_TV_RADIO          = 0x42,
    IRKEY_CHANNEL_LIST      = 0x54,
    IRKEY_CHANNEL_FAV_LIST  = 0x47,
    IRKEY_CHANNEL_RETURN    = 0x11,
    IRKEY_CHANNEL_PLUS      = 0x0C,
    IRKEY_CHANNEL_MINUS     = 0x10,

    IRKEY_AUDIO             = 0x48,
    IRKEY_VOLUME_PLUS       = 0x17,
    IRKEY_VOLUME_MINUS      = 0x16,

    IRKEY_UP                = 0x15,
    IRKEY_POWER             = 0x01,
    IRKEY_EXIT              = 0x1E,
    IRKEY_MENU              = 0x08,
    IRKEY_DOWN              = 0x1C,
    IRKEY_LEFT              = 0x18,
    IRKEY_SELECT            = 0x1A,
    IRKEY_RIGHT             = 0x14,

    IRKEY_NUM_0             = 0x12,
    IRKEY_NUM_1             = 0x05,
    IRKEY_NUM_2             = 0x06,
    IRKEY_NUM_3             = 0x07,
    IRKEY_NUM_4             = 0x09,
    IRKEY_NUM_5             = 0x0A,
    IRKEY_NUM_6             = 0x0B,
    IRKEY_NUM_7             = 0x00,
    IRKEY_NUM_8             = 0x0E,
    IRKEY_NUM_9             = 0x0F,

    IRKEY_MUTE              = 0x04,
    IRKEY_PAGE_UP           = IRKEY_DUMY,
    IRKEY_PAGE_DOWN         = IRKEY_DUMY-1,
    IRKEY_CLOCK             = IRKEY_DUMY-2,

    IRKEY_INFO              = 0x49,
    IRKEY_RED               = 0x4A,
    IRKEY_GREEN             = 0x4B,
    IRKEY_YELLOW            = 0x4C,
    IRKEY_BLUE              = 0x4D,
    IRKEY_MTS               = 0x53,
    IRKEY_NINE_LATTICE      = IRKEY_DUMY-3,
    IRKEY_TTX               = 0x1F,
    IRKEY_CC                = 0x1F,
    IRKEY_INPUT_SOURCE      = 0x51,
    IRKEY_CRADRD            = IRKEY_DUMY-4,
//    IRKEY_PICTURE           = IRKEY_DUMY-5,
    IRKEY_ZOOM              = 0x45,
    IRKEY_DASH              = 0x41,
    IRKEY_SLEEP             = 0x13,
    IRKEY_EPG               = 0x1B,
    IRKEY_PIP               = IRKEY_DUMY-5,

  	IRKEY_MIX               = IRKEY_DUMY-6,
    IRKEY_INDEX             = IRKEY_DUMY-7,
    IRKEY_HOLD              = IRKEY_DUMY-8,
    IRKEY_PREVIOUS          = IRKEY_DUMY-9,
    IRKEY_NEXT              = IRKEY_DUMY-10,
    IRKEY_BACKWARD          = IRKEY_DUMY-11,
    IRKEY_FORWARD           = IRKEY_DUMY-12,
    IRKEY_PLAY              = IRKEY_DUMY-13,
    IRKEY_RECORD            = IRKEY_DUMY-14,
    IRKEY_STOP              = IRKEY_DUMY-15,
    IRKEY_PAUSE             = IRKEY_DUMY-16,

    IRKEY_SIZE              = IRKEY_DUMY-17,
    IRKEY_REVEAL            = IRKEY_DUMY-18,
    IRKEY_SUBCODE           = IRKEY_DUMY-19,
}IrCommandType;
//-------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
// IR system parameter define for H/W setting (Please don't modify them)
//-------------------------------------------------------------------------------------------
#define IR_CKDIV_NUM                        ((XTAL_CLOCK_FREQ+500000)/1000000)
#define IR_CKDIV_NUM_BOOT                   XTAL_CLOCK_FREQ
#define IR_CLK_BOOT                         (XTAL_CLOCK_FREQ/1000000)
#define IR_CLK                              IR_CLK_BOOT

#define irGetMinCnt_BOOT(time, tolerance)   ((u32)(((double)time*((double)IR_CLK_BOOT)/(IR_CKDIV_NUM_BOOT+1))*((double)1-tolerance)))
#define irGetMaxCnt_BOOT(time, tolerance)   ((u32)(((double)time*((double)IR_CLK_BOOT)/(IR_CKDIV_NUM_BOOT+1))*((double)1+tolerance)))
#define irGetMinCnt(time, tolerance)        ((u32)(((double)time*((double)IR_CLK)/(IR_CKDIV_NUM+1))*((double)1-tolerance)))
#define irGetMaxCnt(time, tolerance)        ((u32)(((double)time*((double)IR_CLK)/(IR_CKDIV_NUM+1))*((double)1+tolerance)))

#define irGetCnt_BOOT(time)                 ((u32)((double)time*((double)IR_CLK_BOOT)/(IR_CKDIV_NUM_BOOT+1)))
#define irGetCnt(time)                      ((u32)((double)time*((double)IR_CLK)/(IR_CKDIV_NUM+1)))

// 12Mhz
#define IR_RP_TIMEOUT_BOOT      irGetCnt_BOOT(IR_TIMEOUT_CYC)
#define IR_HDC_UPB_BOOT         irGetMaxCnt_BOOT(IR_HEADER_CODE_TIME, 0.2)
#define IR_HDC_LOB_BOOT         irGetMinCnt_BOOT(IR_HEADER_CODE_TIME, 0.2)
#define IR_OFC_UPB_BOOT         irGetMaxCnt_BOOT(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_LOB_BOOT         irGetMinCnt_BOOT(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_RP_UPB_BOOT      irGetMaxCnt_BOOT(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_OFC_RP_LOB_BOOT      irGetMinCnt_BOOT(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_LG01H_UPB_BOOT       irGetMaxCnt_BOOT(IR_LOGI_01H_TIME, 0.35)
#define IR_LG01H_LOB_BOOT       irGetMinCnt_BOOT(IR_LOGI_01H_TIME, 0.3)
#define IR_LG0_UPB_BOOT         irGetMaxCnt_BOOT(IR_LOGI_0_TIME, 0.2)
#define IR_LG0_LOB_BOOT         irGetMinCnt_BOOT(IR_LOGI_0_TIME, 0.2)
#define IR_LG1_UPB_BOOT         irGetMaxCnt_BOOT(IR_LOGI_1_TIME, 0.2)
#define IR_LG1_LOB_BOOT         irGetMinCnt_BOOT(IR_LOGI_1_TIME, 0.2)

// 90Mhz
#define IR_RP_TIMEOUT           irGetCnt(IR_TIMEOUT_CYC)
#define IR_HDC_UPB              irGetMaxCnt(IR_HEADER_CODE_TIME, 0.2)
#define IR_HDC_LOB              irGetMinCnt(IR_HEADER_CODE_TIME, 0.2)
#define IR_OFC_UPB              irGetMaxCnt(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_LOB              irGetMinCnt(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_RP_UPB           irGetMaxCnt(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_OFC_RP_LOB           irGetMinCnt(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_LG01H_UPB            irGetMaxCnt(IR_LOGI_01H_TIME, 0.35)
#define IR_LG01H_LOB            irGetMinCnt(IR_LOGI_01H_TIME, 0.3)
#define IR_LG0_UPB              irGetMaxCnt(IR_LOGI_0_TIME, 0.2)
#define IR_LG0_LOB              irGetMinCnt(IR_LOGI_0_TIME, 0.2)
#define IR_LG1_UPB              irGetMaxCnt(IR_LOGI_1_TIME, 0.2)
#define IR_LG1_LOB              irGetMinCnt(IR_LOGI_1_TIME, 0.2)

//-------------------------------------------------------------------------------------------

#endif

