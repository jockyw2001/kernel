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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   drvIR.h
/// @brief  IR Driver Interface
/// @author MStar Semiconductor Inc.
///
/// Driver to initialize and access IR.
///     - Provide functions to initialize IR timing, and enable IR interrupt.
///     - Provide IR ISR.
///     - Provide IR callback function registration for AP.
///     - Provide function to get IR key.
///
/// \b [Example]
/// @code
///
/// // Initalize the IR in the boot time.
/// MDrv_IR_Init();
///
/// // *****************************************************************************
///
/// // Set the delay time of IR. First repeat key code is sent after one second.
/// // The following repeat key code is sent after 0.5 seconds.
/// MDrv_IR_SetDelayTime(1000, 500);
///
/// // Please refer to the following diagram. Assume that when users press and hold
/// // IR button, the repeat key is sent every 200ms.
/// // The 1st press is sent, and the return repeat code is 0.
/// // The 5th repeat key is sent because of the 1st delay time is 1000ms.
/// // The 8th repeat key is sent because of the 2nd delay time is 500ms, and
/// // the time between the 5th and the 8th repeat key is 600ms which is greater
/// // than 500ms.
/// // Note: Do not support RELEASE event.
///
/// @endcode
///
/// @image html IR_delay.JPG "IR delay time"
///
/// @code
/// // *****************************************************************************
///
/// // Set the callback function. The function MApi_IR_SetCallback is called if
/// // the IR interrupt is generated and the delay time setting is matched.
/// void MApi_IR_SetCallback(U8 *pu8Key, U8 *pu8Flg);
///
/// MDrv_IR_Set_Callback(MApi_IR_SetCallback);
///
/// // *****************************************************************************
///
/// // Polling & get the IR key directly. Users can call the MDrv_IR_GetKey to get
/// // the IR key if it returns TRUE.
/// U8 u8Key, u8Flg;
///
/// MDrv_IR_GetKey(&u8Key, &u8Flg);
///
/// @endcode
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MHAL_IR_REG_H_
#define _MHAL_IR_REG_H_


//Define IR  IRQ Vector
#define INT_NUM_IR_ALL (E_FIQEXPL_IR_IN) 

// Define IRQ registers
#define REG_IRQ_BASE            0xFD003200
#define REG_IRQ_MASK_IR         (REG_IRQ_BASE + 0x14)
    #define IRQ_UNMASK_IR       0xF7FF

// Define IrDa registers
#define REG_IR_BASE             0xFD007B00
///////////////////////////////////////////////////////////////////////////////
#define REG_IR_CTRL             (0x0000*4 + (REG_IR_BASE))
    #define IR_SEPR_EN              0x0200
    #define IR_TIMEOUT_CHK_EN       0x0100
    #define IR_INV                  0x80
    #define IR_INT_MASK             0x40
    #define IR_RPCODE_EN            0x20
    #define IR_LG01H_CHK_EN         0x10
    #define IR_DCODE_PCHK_EN        0x08
    #define IR_CCODE_CHK_EN         0x04
    #define IR_LDCCHK_EN            0x02
    #define IR_EN                   0x01
#define REG_IR_HDC_UPB          (0x0001*4 + (REG_IR_BASE))
#define REG_IR_HDC_LOB          (0x0002*4 + (REG_IR_BASE))
#define REG_IR_OFC_UPB          (0x0003*4 + (REG_IR_BASE))
#define REG_IR_OFC_LOB          (0x0004*4 + (REG_IR_BASE))
#define REG_IR_OFC_RP_UPB       (0x0005*4 + (REG_IR_BASE))
#define REG_IR_OFC_RP_LOB       (0x0006*4 + (REG_IR_BASE))
#define REG_IR_LG01H_UPB        (0x0007*4 + (REG_IR_BASE))
#define REG_IR_LG01H_LOB        (0x0008*4 + (REG_IR_BASE))
#define REG_IR_LG0_UPB          (0x0009*4 + (REG_IR_BASE))
#define REG_IR_LG0_LOB          (0x000A*4 + (REG_IR_BASE))
#define REG_IR_LG1_UPB          (0x000B*4 + (REG_IR_BASE))
#define REG_IR_LG1_LOB          (0x000C*4 + (REG_IR_BASE))
#define REG_IR_SEPR_UPB         (0x000D*4 + (REG_IR_BASE))
#define REG_IR_SEPR_LOB         (0x000E*4 + (REG_IR_BASE))
#define REG_IR_TIMEOUT_CYC_L    (0x000F*4 + (REG_IR_BASE))
#define REG_IR_TIMEOUT_CYC_H_CODE_BYTE  (0x0010*4 + (REG_IR_BASE))
    #define IR_CCB_CB               0x9F00//ir_ccode_byte+ir_code_bit_num
#define REG_IR_SEPR_BIT_FIFO_CTRL       (0x0011*4 + (REG_IR_BASE))
#define REG_IR_CCODE            (0x0012*4 + (REG_IR_BASE))
#define REG_IR_GLHRM_NUM        (0x0013*4 + (REG_IR_BASE))
#define REG_IR_CKDIV_NUM_KEY_DATA       (0x0014*4 + (REG_IR_BASE))
#define REG_IR_SHOT_CNT_L       (0x0015*4 + (REG_IR_BASE))
#define REG_IR_SHOT_CNT_H_FIFO_STATUS   (0x0016*4 + (REG_IR_BASE))
    #define IR_RPT_FLAG             0x0100
    #define IR_FIFO_EMPTY           0x0200
#define REG_IR_FIFO_RD_PULSE    (0x0018*4 + (REG_IR_BASE))
#define REG_IR_CCODE1           (0x0020UL*4 + (REG_IR_BASE))
#define REG_IR_CCODE1_CHK_EN    (0x0021UL*4 + (REG_IR_BASE))
#define IR_CCODE1_CHK_EN    0x01UL
//for RC5 HW mode
#define REG_IR_RC_BASE             0xFD007A00
#define REG_IR_RC_CTRL             (0x0000*4 + (REG_IR_RC_BASE))
#define IR_RC_AUTOCONFIG              (1<<5)
#define IR_RC_FIFO_CLEAR              (1<<6)
#define IR_RC_FIFO_WFIRST             (1<<7)
#define IR_RC_EN                      (1<<8)
#define IR_RC6_EN                     (1<<9)
#define IR_RC5EXT_EN                  (1<<10)
#define IR_RC_WKUP_EN                 (1<<11)
#define IR_RCIN_INV                   (1<<15)
#define REG_IR_RC_LONGPULSE_THR     (0x0001*4 + (REG_IR_RC_BASE))
#define REG_IR_RC_LONGPULSE_MAR     (0x0002*4 + (REG_IR_RC_BASE))
#define REG_IR_RC_CLK_INT_THR       (0x0003*4 + (REG_IR_RC_BASE))
#define IR_RC6_ECO_EN                  (1<<0)
#define REG_IR_RC_WD_TIMEOUT_CNT    (0x0004*4 + (REG_IR_RC_BASE))
#define REG_IR_RC_COMP_KEY1_KEY2    (0x0005*4 + (REG_IR_RC_BASE))
#define REG_IR_RC_KEY_COMMAND_ADD   (0x0006*4 + (REG_IR_RC_BASE))
#define REG_IR_RC_KEY_MISC          (0x0007*4 + (REG_IR_RC_BASE))
#define REG_IR_RC_KEY_FIFO_STATUS   (0x0008*4 + (REG_IR_RC_BASE))
#define IR_RC_FIFO_EMPTY			(1<<0)
#define IR_RC_TIMEOUT_FLAG 			(1<<1)
#define IR_RC_FIFO_FULL             (1<<2)
#define REG_IR_RC_FIFO_RD_PULSE     (0x0009*4 + (REG_IR_RC_BASE))
#define REG_IR_RC_CMP_RCKEY         (0x000A*4 + (REG_IR_RC_BASE))
#define IR_RC_POWER_WAKEUP_EN      (1<<8)

#endif
