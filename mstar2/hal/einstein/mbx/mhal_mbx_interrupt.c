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
/// file    mhal_mbx_interrupt.c
/// @brief  MStar MailBox interrupt DDI
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#define _MHAL_MBX_INTERRUPT_C

//=============================================================================
// Include Files
//=============================================================================
#include <linux/interrupt.h>
#include <linux/delay.h>

#include "mdrv_mstypes.h"
#include "../../../drv/mbx/mdrv_mbx.h"
#include "mhal_mbx_interrupt.h"

//=============================================================================
// Compile options
//=============================================================================


//=============================================================================
// Local Defines
//=============================================================================

//=============================================================================
// Debug Macros
//=============================================================================
#define MBXINT_DEBUG
#ifdef MBXINT_DEBUG
    #define MBXINT_PRINT(fmt, args...)      printk("[MailBox (Driver)][%05d] " fmt, __LINE__, ## args)
    #define MBXINT_ASSERT(_cnd, _fmt, _args...)                   \
                                    if (!(_cnd)) {              \
                                        MBXINT_PRINT(_fmt, ##_args);  \
                                    }
#else
    #define MBXINT_PRINT(_fmt, _args...)
    #define MBXINT_ASSERT(_cnd, _fmt, _args...)
#endif

//=============================================================================
// Macros
//=============================================================================

//=============================================================================
// Local Variables
//=============================================================================
static MBX_MSGRECV_CB_FUNC _pMBXMsgRecvCbFunc = NULL;
static U32 _MbxIntMask=0;

//=============================================================================
// Global Variables
//=============================================================================

//=============================================================================
// Local Function Prototypes
//=============================================================================
static void _MHAL_MBXINT_AEON2PMHandler(unsigned long unused);
static void _MHAL_MBXINT_AEON2MIPSHandler(unsigned long unused);
static void _MHAL_MBXINT_MIPS2PMHandler(unsigned long unused);
static void _MHAL_MBXINT_MIPS2AEONHandler(unsigned long unused);
static void _MHAL_MBXINT_PM2MIPSHandler(unsigned long unused);
static void _MHAL_MBXINT_PM2AEONHandler(unsigned long unused);

//callback from driver:
static irqreturn_t _MHAL_MBXINT_INTHandler(int irq, void *dev_id);

#if 0
static MS_U16 _MapFiq2FiqMask(MS_S32 s32Fiq);
#endif

static MBX_Result _MHAL_MBXINT_SetHostCPU(MBX_CPU_ID eHostCPU);
static U32 _MHAL_MBXINT_GetIntFlgMask(U32 mbxInt);
static int _MHAL_MBXINT_IsIntFlgSet(U32 mbxInt);
static void _MHAL_MBXINT_SetIntFlg(U32 mbxInt);
static void _MHAL_MBXINT_ClearIntFlg(U32 mbxInt);
//=============================================================================
// Local Function
//=============================================================================

//-------------------------------------------------------------------------------------------------
/// Get Int flag mask
/// @param  eHostCPUID                  \b IN: The Host CPU ID
/// @return Flag bit mask
/// @attention
/// <b>[MXLIB] <em></em></b>
//-------------------------------------------------------------------------------------------------
U32 _MHAL_MBXINT_GetIntFlgMask(U32 mbxInt)
{
    U32 flg=0;
    switch(mbxInt)
    {
    case MBX_INT_MIPS2PM:
        flg = 0x01;
        break;
    case MBX_INT_PM2MIPS:
        flg = 0x02;
        break;
    case MBX_INT_R2MIPS:
        flg = 0x04;
        break;
    default:
        break;
    }
    return flg;
}
//-------------------------------------------------------------------------------------------------
/// Test if Int flag is set
/// @param  eHostCPUID                  \b IN: The Host CPU ID
/// @return !=0: set; ==0: unset
/// @attention
/// <b>[MXLIB] <em></em></b>
//-------------------------------------------------------------------------------------------------
int _MHAL_MBXINT_IsIntFlgSet(U32 mbxInt)
{
    U32 flg=_MHAL_MBXINT_GetIntFlgMask(mbxInt);
    if(flg==0)return 0;
    return (_MbxIntMask&flg);
}
//-------------------------------------------------------------------------------------------------
/// set Int flag
/// @param  eHostCPUID                  \b IN: The Host CPU ID
/// @return void
/// @attention
/// <b>[MXLIB] <em></em></b>
//-------------------------------------------------------------------------------------------------
void _MHAL_MBXINT_SetIntFlg(U32 mbxInt)
{
    U32 flg=_MHAL_MBXINT_GetIntFlgMask(mbxInt);
    if(flg==0)return;
    _MbxIntMask |= flg;
}
//-------------------------------------------------------------------------------------------------
/// clear Int flag
/// @param  eHostCPUID                  \b IN: The Host CPU ID
/// @return void
/// @attention
/// <b>[MXLIB] <em></em></b>
//-------------------------------------------------------------------------------------------------
void _MHAL_MBXINT_ClearIntFlg(U32 mbxInt)
{
    U32 flg=_MHAL_MBXINT_GetIntFlgMask(mbxInt);
    if(flg==0)return;
    _MbxIntMask &= ~flg;
}


//-------------------------------------------------------------------------------------------------
/// Notify Interrupt Aeon2PM
/// @param  unused                  \b IN: unused
/// @return void
/// @attention
/// <b>[OBAMA] <em></em></b>
//-------------------------------------------------------------------------------------------------
void _MHAL_MBXINT_AEON2PMHandler(unsigned long unused)
{
    if(NULL == _pMBXMsgRecvCbFunc)
    {
        return;
    }

    _pMBXMsgRecvCbFunc(MBX_INT_AEON2PM);
}

DECLARE_TASKLET(_mbxAeon2PMTaskletMBXINT, _MHAL_MBXINT_AEON2PMHandler, 0);

//-------------------------------------------------------------------------------------------------
/// Notify Interrupt Aeon2Mips
/// @param  unused                  \b IN: unused
/// @return void
/// @attention
/// <b>[OBAMA] <em></em></b>
//-------------------------------------------------------------------------------------------------
void _MHAL_MBXINT_AEON2MIPSHandler(unsigned long unused)
{
    if(NULL == _pMBXMsgRecvCbFunc)
    {
        return;
    }

    _pMBXMsgRecvCbFunc(MBX_INT_R2MIPS);
}

DECLARE_TASKLET(_mbxAeon2MipsTaskletMBXINT, _MHAL_MBXINT_AEON2MIPSHandler, 0);

//-------------------------------------------------------------------------------------------------
/// Notify Interrupt Mips2PM
/// @param  unused                  \b IN: unused
/// @return void
/// @attention
/// <b>[OBAMA] <em></em></b>
//-------------------------------------------------------------------------------------------------
void _MHAL_MBXINT_MIPS2PMHandler(unsigned long unused)
{
    if(NULL == _pMBXMsgRecvCbFunc)
    {
        return;
    }

    _pMBXMsgRecvCbFunc(MBX_INT_MIPS2PM);
}

DECLARE_TASKLET(_mbxMips2PMTaskletMBXINT, _MHAL_MBXINT_MIPS2PMHandler, 0);

//-------------------------------------------------------------------------------------------------
/// Notify Interrupt Mips2Aeon
/// @param  unused                  \b IN: unused
/// @return void
/// @attention
/// <b>[OBAMA] <em></em></b>
//-------------------------------------------------------------------------------------------------
void _MHAL_MBXINT_MIPS2AEONHandler(unsigned long unused)
{
    if(NULL == _pMBXMsgRecvCbFunc)
    {
        return;
    }

    _pMBXMsgRecvCbFunc(MBX_INT_MIPS2AEON);
}

DECLARE_TASKLET(_mbxMips2AeonTaskletMBXINT, _MHAL_MBXINT_MIPS2AEONHandler, 0);

//-------------------------------------------------------------------------------------------------
/// Notify Interrupt PM2Mips
/// @param  unused                  \b IN: unused
/// @return void
/// @attention
/// <b>[OBAMA] <em></em></b>
//-------------------------------------------------------------------------------------------------
void _MHAL_MBXINT_PM2MIPSHandler(unsigned long unused)
{
    if(NULL == _pMBXMsgRecvCbFunc)
    {
        return;
    }

    _pMBXMsgRecvCbFunc(MBX_INT_PM2MIPS);
}

DECLARE_TASKLET(_mbxPM2MipsTaskletMBXINT, _MHAL_MBXINT_PM2MIPSHandler, 0);

//-------------------------------------------------------------------------------------------------
/// Notify Interrupt PM2Aeon
/// @param  unused                  \b IN: unused
/// @return void
/// @attention
/// <b>[OBAMA] <em></em></b>
//-------------------------------------------------------------------------------------------------
void _MHAL_MBXINT_PM2AEONHandler(unsigned long unused)
{
    if(NULL == _pMBXMsgRecvCbFunc)
    {
        return;
    }

    _pMBXMsgRecvCbFunc(MBX_INT_PM2AEON);
}

DECLARE_TASKLET(_mbxPM2AeonTaskletMBXINT, _MHAL_MBXINT_PM2AEONHandler, 0);

//-------------------------------------------------------------------------------------------------
/// Handle Interrupt, schedule tasklet
/// @param  irq                  \b IN: interrupt number
/// @param  dev_id                  \b IN: dev id
/// @return irqreturn_t: IRQ_HANDLED
/// @attention
/// <b>[OBAMA] <em></em></b>
//-------------------------------------------------------------------------------------------------
irqreturn_t _MHAL_MBXINT_INTHandler(int irq, void *dev_id)
{
    switch(irq)
    {
        case E_FIQ_INT_MIPS_TO_8051:
            tasklet_schedule(&_mbxMips2PMTaskletMBXINT);
            MHAL_MBXINT_Clear(MBX_INT_MIPS2PM);
            break;
        case E_FIQ_INT_8051_TO_MIPS:
            tasklet_schedule(&_mbxPM2MipsTaskletMBXINT);
            MHAL_MBXINT_Clear(MBX_INT_PM2MIPS);
            break;
		case E_FIQEXPL_SECURER2_TO_ARM:
			tasklet_schedule(&_mbxAeon2MipsTaskletMBXINT);
			MHAL_MBXINT_Clear(MBX_INT_R2MIPS);
			break;
        default:
            break;
    }

    return IRQ_HANDLED;
}

//-------------------------------------------------------------------------------------------------
/// Set Interrupt to Host CPU ID: Enable related interrupt and attached related callback.
/// @param  eHostCPUID                  \b IN: The Host CPU ID
/// @return E_MBX_SUCCESS
/// @attention
/// <b>[MXLIB] <em></em></b>
//-------------------------------------------------------------------------------------------------
MBX_Result _MHAL_MBXINT_SetHostCPU(MBX_CPU_ID eHostCPU)
{
    MS_U32     s32Err;

#if (MP_PLATFORM_INT_1_to_1_SPI == 1)
	unsigned long irqflags = SA_INTERRUPT | IRQF_ONESHOT;
#else
	unsigned long irqflags = SA_INTERRUPT;
#endif

    switch(eHostCPU)
    {
        case E_MBX_CPU_PM:
            s32Err = request_irq(E_FIQ_INT_MIPS_TO_8051, _MHAL_MBXINT_INTHandler, irqflags, "MBX_FIQ_MIPS2PM", NULL);
            if(s32Err<0)
            {
                MBXINT_PRINT("request FIQ: %x Failed!\n", MBX_INT_MIPS2PM);
                return E_MBX_UNKNOW_ERROR;
            }

            MHAL_MBXINT_Enable(MBX_INT_MIPS2PM, TRUE);
            _MHAL_MBXINT_SetIntFlg(MBX_INT_MIPS2PM);
            break;
        case E_MBX_CPU_MIPS:
            s32Err = request_irq(E_FIQ_INT_8051_TO_MIPS, _MHAL_MBXINT_INTHandler, irqflags, "MBX_FIQ_PM2MIPS", NULL);
            if(s32Err<0)
            {
                MBXINT_PRINT("request FIQ: %x Failed!\n", MBX_INT_PM2MIPS);
                return E_MBX_UNKNOW_ERROR;
            }
            MHAL_MBXINT_Enable(MBX_INT_PM2MIPS, TRUE);
            _MHAL_MBXINT_SetIntFlg(MBX_INT_PM2MIPS);
            break;
        default:
            return E_MBX_ERR_INVALID_CPU_ID;
    }

	//E_FIQEXPL_SECURER2_TO_ARM
	s32Err = request_irq(E_FIQEXPL_SECURER2_TO_ARM, _MHAL_MBXINT_INTHandler, irqflags, "MBX_FIQ_R2toMIPS", NULL);
	if(s32Err<0)
    {
        printk(KERN_EMERG "request FIQ: %x Failed!\n", E_FIQEXPL_SECURE_R2_TO_ARM);
        return E_MBX_UNKNOW_ERROR;
    }

    MHAL_MBXINT_Enable(MBX_INT_R2MIPS, TRUE);
    _MHAL_MBXINT_SetIntFlg(MBX_INT_R2MIPS);

    return E_MBX_SUCCESS;
}

//=============================================================================
// Mailbox HAL Interrupt Driver Function
//=============================================================================

//-------------------------------------------------------------------------------------------------
/// Handle Interrupt INIT
/// @param  eHostCPU                  \b IN: interrupt owner
/// @param  pMBXRecvMsgCBFunc                  \b IN: callback func by driver
/// @return E_MBX_ERR_INVALID_CPU_ID: the cpu id is wrong
/// @return E_MBX_UNKNOW_ERROR: request_irq failed;
/// @return E_MBX_SUCCESS: success;
/// @attention
/// <b>[OBAMA] <em></em></b>
//-------------------------------------------------------------------------------------------------
MBX_Result MHAL_MBXINT_Init (MBX_CPU_ID eHostCPU, MBX_MSGRECV_CB_FUNC pMBXRecvMsgCBFunc)
{
    _pMBXMsgRecvCbFunc = pMBXRecvMsgCBFunc;

    return _MHAL_MBXINT_SetHostCPU(eHostCPU);
}

//-------------------------------------------------------------------------------------------------
/// Handle Interrupt DeINIT
/// @param  eHostCPU                  \b IN: interrupt owner
/// @return void;
/// @attention
/// <b>[OBAMA] <em></em></b>
//-------------------------------------------------------------------------------------------------
void MHAL_MBXINT_DeInit (MBX_CPU_ID eHostCPU)
{
    switch(eHostCPU)
    {
        case E_MBX_CPU_PM:
            MHAL_MBXINT_Enable(MBX_INT_MIPS2PM, FALSE);
            free_irq(E_FIQ_INT_MIPS_TO_8051, NULL);
            _MHAL_MBXINT_ClearIntFlg(MBX_INT_MIPS2PM);
            break;
        case E_MBX_CPU_MIPS:
            MHAL_MBXINT_Enable(MBX_INT_PM2MIPS, FALSE);
            free_irq(E_FIQ_INT_8051_TO_MIPS, NULL);
            _MHAL_MBXINT_ClearIntFlg(MBX_INT_PM2MIPS);
            break;
        default:
            return;
    }
    //added according to _MHAL_MBXINT_SetHostCPU
    MHAL_MBXINT_Enable(E_FIQEXPL_SECURER2_TO_ARM, FALSE);
    free_irq(E_FIQEXPL_SECURER2_TO_ARM, NULL);
    _MHAL_MBXINT_ClearIntFlg(E_FIQEXPL_SECURER2_TO_ARM);
}

//-------------------------------------------------------------------------------------------------
/// Suspend MBX interrupt
/// @param  eHostCPU                  \b IN: interrupt owner
/// @return E_MBX_ERR_INVALID_CPU_ID: the cpu id is wrong
/// @return E_MBX_SUCCESS: success;
/// @attention
/// <b>[OBAMA] <em></em></b>
//-------------------------------------------------------------------------------------------------
MBX_Result MHAL_MBXINT_Suspend(MBX_CPU_ID eHostCPU)
{
    if(_MHAL_MBXINT_IsIntFlgSet(MBX_INT_MIPS2PM)){
        MHAL_MBXINT_Enable(MBX_INT_MIPS2PM,FALSE);
        disable_irq(E_FIQ_INT_MIPS_TO_8051);
    }

    if(_MHAL_MBXINT_IsIntFlgSet(MBX_INT_PM2MIPS)){
        MHAL_MBXINT_Enable(MBX_INT_PM2MIPS,FALSE);
        disable_irq(E_FIQ_INT_8051_TO_MIPS);
    }

    if(_MHAL_MBXINT_IsIntFlgSet(E_FIQEXPL_SECURER2_TO_ARM)){
        MHAL_MBXINT_Enable(E_FIQEXPL_SECURER2_TO_ARM,FALSE);
        disable_irq(E_FIQEXPL_SECURER2_TO_ARM);
    }
    return E_MBX_SUCCESS;
}
//-------------------------------------------------------------------------------------------------
/// Resume MBX interrupt
/// @param  eHostCPU                  \b IN: interrupt owner
/// @return E_MBX_ERR_INVALID_CPU_ID: the cpu id is wrong
/// @return E_MBX_SUCCESS: success;
/// @attention
/// <b>[OBAMA] <em></em></b>
//-------------------------------------------------------------------------------------------------
MBX_Result MHAL_MBXINT_Resume(MBX_CPU_ID eHostCPU)
{
    if(_MHAL_MBXINT_IsIntFlgSet(MBX_INT_MIPS2PM)){
        MHAL_MBXINT_Enable(MBX_INT_MIPS2PM,TRUE);
        enable_irq(E_FIQ_INT_MIPS_TO_8051);
    }

    if(_MHAL_MBXINT_IsIntFlgSet(MBX_INT_PM2MIPS)){
        MHAL_MBXINT_Enable(MBX_INT_PM2MIPS,TRUE);
        enable_irq(E_FIQ_INT_8051_TO_MIPS);
    }

    if(_MHAL_MBXINT_IsIntFlgSet(E_FIQEXPL_SECURER2_TO_ARM)){
        MHAL_MBXINT_Enable(E_FIQEXPL_SECURER2_TO_ARM,TRUE);
        enable_irq(E_FIQEXPL_SECURER2_TO_ARM);
    }
    return E_MBX_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// Reset Host CPU for MBX Interrupt
/// @param  ePrevCPU                  \b IN: previous host cpu id
/// @param  eConfigCpu                  \b IN: new configed cpu id
/// @return E_MBX_SUCCESS: success;
/// @return E_MBX_INVALID_CPU_ID
/// @attention
/// <b>[MXLIB] <em></em></b>
//-------------------------------------------------------------------------------------------------
MBX_Result MHAL_MBXINT_ResetHostCPU (MBX_CPU_ID ePrevCPU, MBX_CPU_ID eConfigCpu)
{
    MHAL_MBXINT_DeInit(ePrevCPU);

    return _MHAL_MBXINT_SetHostCPU(eConfigCpu);
}

//-------------------------------------------------------------------------------------------------
/// enable/disable Interrupt
/// @param  s32Fiq                  \b IN: interrupt for enable/disable
/// @param  bEnable                  \b IN: Enable or Disable
/// @return E_MBX_SUCCESS: success;
/// @attention
/// <b>[OBAMA] <em></em></b>
//-------------------------------------------------------------------------------------------------
MBX_Result MHAL_MBXINT_Enable (MS_S32 s32Fiq, MS_BOOL bEnable)
{
    MS_U16 u16RegValHost;
    MS_U16 u16FIQMask;
    MS_U16 u16IdxHost;

    switch(s32Fiq)
    {
        case MBX_INT_MIPS2PM:
            u16IdxHost = REG_INT_FIQMASK_PM;
            u16FIQMask = _BIT_ARM_PM;
            break;
        case MBX_INT_PM2MIPS:
            u16IdxHost = REG_INT_FIQMASK_ARM;
            u16FIQMask = _BIT_PM_ARM;
            break;
		case MBX_INT_R2MIPS:
			u16IdxHost = REG_INT_FIQMASK_ARM_H;
            u16FIQMask = _BIT_R2_ARM;
			break;
        default:
            return 0;
    }

    mb();
    u16RegValHost = INT_REG(u16IdxHost);
    if(bEnable)
    {
        INT_REG(u16IdxHost) = u16RegValHost & ~(u16FIQMask);
    }
    else
    {
        INT_REG(u16IdxHost) = u16RegValHost | (u16FIQMask);
    }
    mb();

    return E_MBX_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// Fire Interrupt
/// @param  dstCPUID                  \b IN: dst cpu of interrupt
/// @param  srcCPUID                  \b IN: src cpu of interrupt
/// @return E_MBX_SUCCESS: success;
/// @attention
/// <b>[OBAMA] <em></em></b>
//-------------------------------------------------------------------------------------------------
MBX_Result MHAL_MBXINT_Fire (MBX_CPU_ID dstCPUID, MBX_CPU_ID srcCPUID)
{
    MBXINT_ASSERT((dstCPUID!=srcCPUID),"dst cpu is the same as src cpu!\n");

    switch(srcCPUID)
    {
        case E_MBX_CPU_PM: //PM to HK
            CPU_INT_REG(REG_INT_PMFIRE) = INT_PM_ARM;
            mb();
            CPU_INT_REG(REG_INT_PMFIRE) = 0;
            mb();
            break;
        case E_MBX_CPU_MIPS: //HK to PM

           if (dstCPUID == E_MBX_CPU_MIPS_VPE1)
           	 CPU_INT_REG(REG_INT_ARMFIRE) = INT_ARM_R2; //to R2
		   else
			 CPU_INT_REG(REG_INT_ARMFIRE) = INT_ARM_PM; //to PM

            mb();
            CPU_INT_REG(REG_INT_ARMFIRE) = 0;
            mb();
            break;
        default:
            MBXINT_ASSERT(FALSE,"wrong src cpu!\n");
            break;
    }

    return E_MBX_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/// clear Interrupt
/// @param  s32Fiq                  \b IN: interrupt number
/// @return E_MBX_SUCCESS: success;
/// @attention
/// <b>[OBAMA] <em></em></b>
//-------------------------------------------------------------------------------------------------
MBX_Result MHAL_MBXINT_Clear (MS_S32 s32Fiq)
{
    MS_U16 u16RegValHost;
    MS_U16 u16FIQMask;
    MS_U16 u16IdxHost;

    switch(s32Fiq)
    {
        case MBX_INT_MIPS2PM:
            u16IdxHost = REG_INT_FIQMASK_PM;
            u16FIQMask = _BIT_ARM_PM;
            break;
        case MBX_INT_PM2MIPS:
            u16IdxHost = REG_INT_FIQMASK_ARM;
            u16FIQMask = _BIT_PM_ARM;
            break;
		case MBX_INT_R2MIPS: 
			u16IdxHost = 0x2f;
			u16FIQMask = 1<<1;
			break;
        default:
            return 0;
    }

    mb();

    u16RegValHost = INT_REG(u16IdxHost);
    INT_REG(u16IdxHost) = u16RegValHost | (u16FIQMask);

    mb();


    return E_MBX_SUCCESS;
}
