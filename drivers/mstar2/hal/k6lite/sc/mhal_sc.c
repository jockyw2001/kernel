///////////////////////////////////////////////////////////////////////////////////////////////////
//
// * Copyright (c) 2006 - 2008 MStar Semiconductor, Inc.
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


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <asm/io.h>
#include <linux/sched.h>
#include <linux/version.h>
#include "mdrv_mstypes.h"
#include "reg_sc.h"
#include "mhal_sc.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static void _HAL_SC_ClearLsr(MS_U8 u8SCID)
{
    SC_WRITE(u8SCID, UART_LSR_CLR, UART_LSR_CLR_FLAG);
    SC_WRITE(u8SCID, UART_LSR_CLR, 0x00);
}


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
MS_BOOL HAL_SC_GetIntTxLevelAndGWT(MS_U8 u8SCID, HAL_SC_TX_LEVEL_GWT_INT *pstTxLevelGWT)
{
    MS_U8 u8RegData;

    u8RegData = SC_READ(u8SCID,UART_GWT_INT);

    //init
    pstTxLevelGWT->bTxLevelInt = FALSE;
    pstTxLevelGWT->bCWT_RxFail = FALSE;
    pstTxLevelGWT->bCWT_TxFail = FALSE;
    pstTxLevelGWT->bCGT_RxFail = FALSE;
    pstTxLevelGWT->bCGT_TxFail = FALSE;
    pstTxLevelGWT->bBGT_Fail = FALSE;
    pstTxLevelGWT->bBWT_Fail = FALSE;

    if (u8RegData & UART_GWT_TX_LEVEL_INT)
        pstTxLevelGWT->bTxLevelInt = TRUE;

    if (u8RegData & UART_GWT_CWT_RX_FAIL)
        pstTxLevelGWT->bCWT_RxFail = TRUE;

    if (u8RegData & UART_GWT_CWT_TX_FAIL)
        pstTxLevelGWT->bCWT_TxFail = TRUE;

    if (u8RegData & UART_GWT_CGT_RX_FAIL)
        pstTxLevelGWT->bCGT_RxFail = TRUE;

    if (u8RegData & UART_GWT_CGT_TX_FAIL)
        pstTxLevelGWT->bCGT_TxFail = TRUE;

    if (u8RegData & UART_GWT_BGT_FAIL)
        pstTxLevelGWT->bBGT_Fail = TRUE;

    if (u8RegData & UART_GWT_BWT_FAIL)
        pstTxLevelGWT->bBWT_Fail = TRUE;

    return TRUE;
}

void HAL_SC_ClearIntTxLevelAndGWT(MS_U8 u8SCID)
{
    SC_WRITE(u8SCID, UART_CTRL2, SC_READ(u8SCID, UART_CTRL2) | UART_CTRL2_FLAG_CLEAR);
    SC_WRITE(u8SCID, UART_CTRL2, SC_READ(u8SCID, UART_CTRL2) & (~UART_CTRL2_FLAG_CLEAR));
}

MS_BOOL HAL_SC_IsPendingINT(MS_U8 IIRReg)
{
    if (!(IIRReg & UART_IIR_NO_INT))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

MS_U8 HAL_SC_GetLsr(MS_U8 u8SCID)
{
    MS_U8 u8Data;

    u8Data = SC_READ(u8SCID,UART_LSR);

    _HAL_SC_ClearLsr(u8SCID);

    return u8Data;
}

MS_BOOL HAL_SC_CheckIntRstToIoEdgeFail(MS_U8 u8SCID)
{
    MS_U8 u8RegData;

    u8RegData = SC_READ(u8SCID,UART_SCFC);
    if (u8RegData & UART_SCFC_RST_TO_IO_EDGE_DET_FAIL)
        return TRUE;
    else
        return FALSE;
}

void HAL_SC_MaskIntRstToIoEdgeFail(MS_U8 u8SCID)
{
    SC_WRITE(u8SCID, UART_CTRL15, SC_READ(u8SCID, UART_CTRL15) | UART_CTRL15_RST_TO_IO_EDGE_DET_FAIL_MASK);
}

MS_BOOL HAL_SC_IsBwtInsteadExtCwt(MS_U8 u8SCID)
{
    return TRUE;
}

void HAL_SC_RstToIoEdgeDetCtrl(MS_U8 u8SCID, MS_BOOL bEnable)
{
    if (bEnable)
    {
        SC_OR(u8SCID, UART_SCFC, UART_SCFC_RST_TO_IO_EDGE_DET_EN);
    }
    else
    {
        SC_AND(u8SCID, UART_SCFC, ~(UART_SCFC_RST_TO_IO_EDGE_DET_EN));
    }
}
