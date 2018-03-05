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
/// @file   Mhal_mtlb.c
/// @brief  MTLB Driver
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/printk.h>
#include <linux/delay.h>
#include "MsTypes.h"
#include "mdrv_types.h"
#include "mdrv_miu.h"
#include "regMIU.h"
#include "mhal_miu.h"
#if defined(CONFIG_COMPAT)
#include <linux/compat.h>
#endif
#include "mdrv_system.h"

#if defined(CONFIG_MIPS)
#include <asm/mips-boards/prom.h>
#include "mhal_chiptop_reg.h"
#elif defined(CONFIG_ARM)
#include <prom.h>
#include <asm/mach/map.h>
#elif defined(CONFIG_ARM64)
#include <asm/arm-boards/prom.h>
#include <asm/mach/map.h>
#endif
#include "chip_setup.h"
#include <linux/bug.h>
#include <asm/barrier.h>

//-------------------------------------------------------------------------------------------------
//  Define
//-------------------------------------------------------------------------------------------------

#define MIU_CLIENT_GP0  \
    MIU_CLIENT_NONE, \
    MIU_CLIENT_VIVALDI9_DECODER_R, \
    MIU_CLIENT_SECAU_R2_RW, \
    MIU_CLIENT_USB_UHC4_RW,\
    MIU_CLIENT_SECURE_R2_RW,\
    MIU_CLIENT_AU_R2_RW, \
    MIU_CLIENT_VD_R2D_RW,\
    MIU_CLIENT_PM51_RW, \
    MIU_CLIENT_VD_R2I_R, \
    MIU_CLIENT_USB_UHC0_RW, \
    MIU_CLIENT_USB_UHC1_RW, \
    MIU_CLIENT_USB_UHC2_RW, \
    MIU_CLIENT_MVD_BBU_RW, \
    MIU_CLIENT_EMAC_RW, \
    MIU_CLIENT_BDMA_RW, \
    MIU_CLIENT_DUMMY

#define MIU_CLIENT_GP1  \
    MIU_CLIENT_VIVALDI9_MAD_RW, \
    MIU_CLIENT_DEMOD_RW, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_JPD720P_RW, \
    MIU_CLIENT_VE_RW, \
    MIU_CLIENT_SC_DIPW_RW,\
    MIU_CLIENT_SC_LOCALDIMING_RW, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_TSP_ORZ_W, \
    MIU_CLIENT_TSP_ORZ_R, \
    MIU_CLIENT_USB_UHC3_RW, \
    MIU_CLIENT_VD_VBI_RW, \
    MIU_CLIENT_VD_COMB_W, \
    MIU_CLIENT_VD_COMB_R,  \
    MIU_CLIENT_ZDEC_RW, \
    MIU_CLIENT_ZDEC_ACP_RW

#define MIU_CLIENT_GP2  \
    MIU_CLIENT_CMD_QUEUE_RW, \
    MIU_CLIENT_GE_RW, \
    MIU_CLIENT_MIIC0_RW, \
    MIU_CLIENT_UART_DMA_RW, \
    MIU_CLIENT_MVD_RW, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_GPD_RW, \
    MIU_CLIENT_MFE0_W, \
    MIU_CLIENT_MFE1_R, \
    MIU_CLIENT_NAND_RW, \
    MIU_CLIENT_SDIO_RW, \
    MIU_CLIENT_DSCRMB_RW, \
    MIU_CLIENT_TSP_FIQ_RW, \
    MIU_CLIENT_TSP_ORZ_W, \
    MIU_CLIENT_TSP_ORZ_R,\
    MIU_CLIENT_TSO_RW

#define MIU_CLIENT_GP3  \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY

#define MIU_CLIENT_GP4  \
    MIU_CLIENT_HVD_BBU_R, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_HVD_RW, \
    MIU_CLIENT_SECHVD_RW, \
    MIU_CLIENT_EVD_RW,\
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_MVD_RTO_RW, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY

#define MIU_CLIENT_GP5  \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY

#define MIU_CLIENT_GP6  \
    MIU_CLIENT_SC_IPMAIN_RW, \
    MIU_CLIENT_SC_OPMAIN_RW, \
    MIU_CLIENT_MVOP_128BIT_R, \
    MIU_CLIENT_MFDEC_R, \
    MIU_CLIENT_SECMFDEC_R, \
    MIU_CLIENT_GOP0_R, \
    MIU_CLIENT_GOP1_R, \
    MIU_CLIENT_GOP2_R, \
    MIU_CLIENT_GOP3_PDW0_RW, \
    MIU_CLIENT_SC_PDW_W, \
    MIU_CLIENT_SC_IPSUB_RW, \
    MIU_CLIENT_SC_DIPW_RW, \
    MIU_CLIENT_SC2_OPMAIN_RW, \
    MIU_CLIENT_SC_OD_RW, \
    MIU_CLIENT_SC2_IPMAIN_RW, \
    MIU_CLIENT_SC_IPMAIN2_RW

#define MIU_CLIENT_GP7  \
    MIU_CLIENT_MIPS_RW, \
    MIU_CLIENT_G3D_RW

#define MIU1_CLIENT_GP0  \
    MIU_CLIENT_NONE, \
    MIU_CLIENT_VIVALDI9_DECODER_R, \
    MIU_CLIENT_SECAU_R2_RW, \
    MIU_CLIENT_USB_UHC4_RW,\
    MIU_CLIENT_SECURE_R2_RW,\
    MIU_CLIENT_AU_R2_RW, \
    MIU_CLIENT_VD_R2D_RW,\
    MIU_CLIENT_PM51_RW, \
    MIU_CLIENT_VD_R2I_R, \
    MIU_CLIENT_USB_UHC0_RW, \
    MIU_CLIENT_USB_UHC1_RW, \
    MIU_CLIENT_USB_UHC2_RW, \
    MIU_CLIENT_MVD_BBU_RW, \
    MIU_CLIENT_EMAC_RW, \
    MIU_CLIENT_BDMA_RW, \
    MIU_CLIENT_DUMMY

#define MIU1_CLIENT_GP1  \
    MIU_CLIENT_VIVALDI9_MAD_RW, \
    MIU_CLIENT_DEMOD_RW, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_JPD720P_RW, \
    MIU_CLIENT_VE_RW, \
    MIU_CLIENT_SC_DIPW_RW,\
    MIU_CLIENT_SC_LOCALDIMING_RW, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_TSP_ORZ_W, \
    MIU_CLIENT_TSP_ORZ_R, \
    MIU_CLIENT_USB_UHC3_RW, \
    MIU_CLIENT_VD_VBI_RW, \
    MIU_CLIENT_VD_COMB_W, \
    MIU_CLIENT_VD_COMB_R,  \
    MIU_CLIENT_ZDEC_RW, \
    MIU_CLIENT_ZDEC_ACP_RW

#define MIU1_CLIENT_GP2  \
    MIU_CLIENT_CMD_QUEUE_RW, \
    MIU_CLIENT_GE_RW, \
    MIU_CLIENT_MIIC0_RW, \
    MIU_CLIENT_UART_DMA_RW, \
    MIU_CLIENT_MVD_RW, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_GPD_RW, \
    MIU_CLIENT_MFE0_W, \
    MIU_CLIENT_MFE1_R, \
    MIU_CLIENT_NAND_RW, \
    MIU_CLIENT_SDIO_RW, \
    MIU_CLIENT_DSCRMB_RW, \
    MIU_CLIENT_TSP_FIQ_RW, \
    MIU_CLIENT_TSP_ORZ_W, \
    MIU_CLIENT_TSP_ORZ_R,\
    MIU_CLIENT_TSO_RW

#define MIU1_CLIENT_GP3  \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY

#define MIU1_CLIENT_GP4  \
    MIU_CLIENT_HVD_BBU_R, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_HVD_RW, \
    MIU_CLIENT_SECHVD_RW, \
    MIU_CLIENT_EVD_RW,\
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_MVD_RTO_RW, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY

#define MIU1_CLIENT_GP5  \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY, \
    MIU_CLIENT_DUMMY

#define MIU1_CLIENT_GP6  \
    MIU_CLIENT_SC_IPMAIN_RW, \
    MIU_CLIENT_SC_OPMAIN_RW, \
    MIU_CLIENT_MVOP_128BIT_R, \
    MIU_CLIENT_MFDEC_R, \
    MIU_CLIENT_SECMFDEC_R, \
    MIU_CLIENT_GOP0_R, \
    MIU_CLIENT_GOP1_R, \
    MIU_CLIENT_GOP2_R, \
    MIU_CLIENT_GOP3_PDW0_RW, \
    MIU_CLIENT_SC_PDW_W, \
    MIU_CLIENT_SC_IPSUB_RW, \
    MIU_CLIENT_SC_DIPW_RW, \
    MIU_CLIENT_SC2_OPMAIN_RW, \
    MIU_CLIENT_SC_OD_RW, \
    MIU_CLIENT_SC2_IPMAIN_RW, \
    MIU_CLIENT_SC_IPMAIN2_RW

#define MIU1_CLIENT_GP7  \
    MIU_CLIENT_MIPS_RW, \
    MIU_CLIENT_G3D_RW


//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
#ifdef CONFIG_CMA
#define IDNUM_KERNELPROTECT (14)
#else
#define IDNUM_KERNELPROTECT (12)
#endif

const eMIUClientID clientTbl[MIU_MAX_DEVICE][MIU_MAX_TBL_CLIENT] =
{
    {
        MIU_CLIENT_GP0,
        MIU_CLIENT_GP1,
        MIU_CLIENT_GP2,
        MIU_CLIENT_GP3,
        MIU_CLIENT_GP4,
        MIU_CLIENT_GP5,
        MIU_CLIENT_GP6,
        MIU_CLIENT_GP7
    },
    {
        MIU1_CLIENT_GP0,
        MIU1_CLIENT_GP1,
        MIU1_CLIENT_GP2,
        MIU1_CLIENT_GP3,
        MIU1_CLIENT_GP4,
        MIU1_CLIENT_GP5,
        MIU1_CLIENT_GP6,
        MIU1_CLIENT_GP7
    }
};

MS_U8 clientId_KernelProtect[IDNUM_KERNELPROTECT] =
{
    MIU_CLIENT_MIPS_RW, MIU_CLIENT_NAND_RW, MIU_CLIENT_USB_UHC0_RW, MIU_CLIENT_USB_UHC1_RW,
    MIU_CLIENT_USB_UHC2_RW, MIU_CLIENT_G3D_RW, MIU_CLIENT_USB3_RW, MIU_CLIENT_SDIO_RW,
    MIU_CLIENT_SATA_RW, MIU_CLIENT_USB_UHC3_RW, MIU_CLIENT_USB30_1_RW, MIU_CLIENT_USB30_2_RW
#ifdef CONFIG_CMA
    ,MIU_CLIENT_GOP3_PDW0_RW
    ,MIU_CLIENT_GE_RW
#endif
};

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
typedef enum
{
  E_CHIP_MIU_0 = 0,
  E_CHIP_MIU_1,
  E_CHIP_MIU_NUM,
} CHIP_MIU_ID;

//-------------------------------------------------------------------------------------------------
//  Macros
//-------------------------------------------------------------------------------------------------
#define _phy_to_miu_offset(MiuSel, Offset, PhysAddr) if (PhysAddr < ARM_MIU1_BASE_ADDR) \
                                                        {MiuSel = E_CHIP_MIU_0; Offset = PhysAddr;} \
                                                     else \
                                                         {MiuSel = E_CHIP_MIU_1; Offset = PhysAddr - ARM_MIU1_BASE_ADDR;} \

#define _miu_offset_to_phy(MiuSel, Offset, PhysAddr) if (MiuSel == E_CHIP_MIU_0) \
                                                        {PhysAddr = Offset;} \
                                                     else \
                                                         {PhysAddr = Offset + ARM_MIU1_BASE_ADDR;} \

#define MIU_HAL_ERR(fmt, args...)   printk(KERN_ERR "miu hal error %s:%d" fmt,__FUNCTION__,__LINE__,## args)

//-------------------------------------------------------------------------------------------------
//  Local Variable
//-------------------------------------------------------------------------------------------------
//static MS_U32 _gMIU_MapBase = 0xBF200000;      //default set to MIPS platfrom
#if defined(CONFIG_ARM) || defined(CONFIG_MIPS)
static MS_U32 _gMIU_MapBase = 0xFD200000UL;   //default set to arm 32bit platfrom
#elif defined(CONFIG_ARM64)
extern ptrdiff_t mstar_pm_base;
static ptrdiff_t _gMIU_MapBase;
#endif

MS_BOOL IDEnables[MIU_MAX_DEVICE][MIU_MAX_PROTECT_BLOCK][MIU_MAX_PROTECT_ID] = {{{0},{0},{0},{0}}, {{0},{0},{0},{0}}}; //ID enable for protect block 0~3
MS_U8 IDs[MIU_MAX_DEVICE][MIU_MAX_PROTECT_ID] = {{0}, {0}}; //IDs for protection

static SRAM_Slit_FlagGroup *_gSlit0_FlagGroupBase;
static SRAM_Slit_FlagGroup *_gSlit1_FlagGroupBase;

//-------------------------------------------------------------------------------------------------
//  MTLB HAL internal function
//-------------------------------------------------------------------------------------------------
MS_U32 HAL_MIU_BA2PA(MS_U32 u32BusAddr)
{
    MS_PHYADDR u32PhyAddr = 0x0UL;

    // pa = ba - offset
	if( (u32BusAddr >= ARM_MIU0_BUS_BASE) && (u32BusAddr < ARM_MIU1_BUS_BASE) )	// MIU0
		u32PhyAddr = u32BusAddr - ARM_MIU0_BUS_BASE + ARM_MIU0_BASE_ADDR;
	else // MIU1
		u32PhyAddr = u32BusAddr - ARM_MIU1_BUS_BASE + ARM_MIU1_BASE_ADDR;

	return u32PhyAddr;
}

MS_S16 HAL_MIU_GetClientInfo(MS_U8 u8MiuDev, eMIUClientID eClientID)
{
    MS_U8 idx;

    if (MIU_MAX_DEVICE <= u8MiuDev)
    {
        MIU_HAL_ERR("Wrong MIU device:%u\n", u8MiuDev);
        return (-1);
    }

    for (idx = 0; idx < MIU_MAX_TBL_CLIENT; idx++)
        if (eClientID == clientTbl[u8MiuDev][idx])
            return idx;
    return (-1);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_ReadByte
/// @brief \b Function  \b Description: read 1 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <OUT>        \b None :
/// @param <RET>        \b MS_U8
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
MS_U8 HAL_MIU_ReadByte(MS_U32 u32RegAddr)
{
#if defined(CONFIG_ARM64)
	_gMIU_MapBase = (mstar_pm_base + 0x00200000UL);
#endif
    return ((volatile MS_U8*)(_gMIU_MapBase))[(u32RegAddr << 1) - (u32RegAddr & 1)];
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_Read4Byte
/// @brief \b Function  \b Description: read 2 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <OUT>        \b None :
/// @param <RET>        \b MS_U16
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
MS_U16 HAL_MIU_Read2Byte(MS_U32 u32RegAddr)
{
#if defined(CONFIG_ARM64)
	_gMIU_MapBase = (mstar_pm_base + 0x00200000UL);
#endif
    return ((volatile MS_U16*)(_gMIU_MapBase))[u32RegAddr];
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_WriteByte
/// @brief \b Function  \b Description: write 1 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u8Val : 1 byte data
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_WriteByte(MS_U32 u32RegAddr, MS_U8 u8Val)
{
    if (!u32RegAddr)
    {
        MIU_HAL_ERR("%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

#if defined(CONFIG_ARM64)
	_gMIU_MapBase = (mstar_pm_base + 0x00200000UL);
#endif
    ((volatile MS_U8*)(_gMIU_MapBase))[(u32RegAddr << 1) - (u32RegAddr & 1)] = u8Val;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_Write2Byte
/// @brief \b Function  \b Description: write 2 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u16Val : 2 byte data
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_Write2Byte(MS_U32 u32RegAddr, MS_U16 u16Val)
{
    if (!u32RegAddr)
    {
        MIU_HAL_ERR("%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

#if defined(CONFIG_ARM64)
	_gMIU_MapBase = (mstar_pm_base + 0x00200000UL);
#endif
    ((volatile MS_U16*)(_gMIU_MapBase))[u32RegAddr] = u16Val;
    return TRUE;
}

void HAL_MIU_SetProtectID(MS_U32 u32Reg, MS_U8 u8MiuDev, MS_U8 u8ClientID)
{
    MS_S16 sVal = HAL_MIU_GetClientInfo(u8MiuDev, (eMIUClientID)u8ClientID);
    MS_S16 sIDVal;

    if (0 > sVal)
        sVal = 0;

    sIDVal = HAL_MIU_ReadByte(u32Reg);
    sIDVal &= 0x80;
    sIDVal |= sVal;
    HAL_MIU_WriteByte(u32Reg, sIDVal);
}

MS_BOOL HAL_MIU_WriteRegBit(MS_U32 u32RegAddr, MS_U8 u8Mask, MS_BOOL bEnable)
{
    MS_U8 u8Val = HAL_MIU_ReadByte(u32RegAddr);
    if (!u32RegAddr)
    {
        MIU_HAL_ERR("%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

    u8Val = HAL_MIU_ReadByte(u32RegAddr);
    u8Val = (bEnable) ? (u8Val | u8Mask) : (u8Val & ~u8Mask);
    HAL_MIU_WriteByte(u32RegAddr, u8Val);
    return TRUE;
}

MS_BOOL HAL_MIU_SetGroupID(MS_U8 u8MiuSel, MS_U8 u8Blockx, MS_U8 *pu8ProtectId, MS_U32 u32RegAddrID, MS_U32 u32RegAddrIDenable)
{
    MS_U32 u32index0, u32index1;
    MS_U8 u8ID;
    MS_U8 u8isfound0, u8isfound1;
    MS_U16 u16idenable;

    //reset IDenables for protect u8Blockx
    for(u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        IDEnables[u8MiuSel][u8Blockx][u32index0] = 0;
    }

    for(u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        u8ID = pu8ProtectId[u32index0];

        //Unused ID
        if(u8ID == 0)
           continue;

        u8isfound0 = FALSE;

        for(u32index1 = 0; u32index1 < MIU_MAX_PROTECT_ID; u32index1++)
        {
            if(IDs[u8MiuSel][u32index1] == u8ID)
            {
                //ID reused former setting
                IDEnables[u8MiuSel][u8Blockx][u32index1] = 1;
                u8isfound0 = TRUE;
                break;
            }
        }

        //Need to create new ID in IDs
        if(u8isfound0 != TRUE)
        {
            u8isfound1 = FALSE;

            for(u32index1 = 0; u32index1 < MIU_MAX_PROTECT_ID; u32index1++)
            {
                if(IDs[u8MiuSel][u32index1] == 0)
                {
                    IDs[u8MiuSel][u32index1] = u8ID;
                    IDEnables[u8MiuSel][u8Blockx][u32index1] = 1;
                    u8isfound1 = TRUE;
                    break;
                }
            }

            //ID overflow
            if(u8isfound1 == FALSE)
                return FALSE;
        }
    }

    u16idenable = 0;

    for(u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        if(IDEnables[u8MiuSel][u8Blockx][u32index0] == 1)
            u16idenable |= (1<<u32index0);
    }

    HAL_MIU_Write2Byte(u32RegAddrIDenable, u16idenable);

    for(u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
         HAL_MIU_SetProtectID(u32RegAddrID + u32index0, u8MiuSel, IDs[u8MiuSel][u32index0]);
    }

    return TRUE;
}

MS_BOOL HAL_MIU_ResetGroupID(MS_U8 u8MiuSel, MS_U8 u8Blockx, MS_U8 *pu8ProtectId, MS_U32 u32RegAddrID, MS_U32 u32RegAddrIDenable)
{
    MS_U32 u32index0, u32index1;
    MS_U8 u8isIDNoUse;
    MS_U16 u16idenable;

    //reset IDenables for protect u8Blockx
    for(u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        IDEnables[u8MiuSel][u8Blockx][u32index0] = 0;
    }

    u16idenable = 0x0UL;

    HAL_MIU_Write2Byte(u32RegAddrIDenable, u16idenable);

    for(u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        u8isIDNoUse  = FALSE;

        for(u32index1 = 0; u32index1 < MIU_MAX_PROTECT_BLOCK; u32index1++)
        {
            if(IDEnables[u8MiuSel][u32index1][u32index0] == 1)
            {
                //protect ID is still be used
                u8isIDNoUse  = FALSE;
                break;
            }
            u8isIDNoUse  = TRUE;
        }

        if(u8isIDNoUse == TRUE)
            IDs[u8MiuSel][u32index0] = 0;
    }

    for(u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
         HAL_MIU_SetProtectID(u32RegAddrID + u32index0, u8MiuSel, IDs[u8MiuSel][u32index0]);
    }

    return TRUE;
}

void HAL_MIU_Write2BytesBit(MS_U32 u32RegOffset, MS_BOOL bEnable, MS_U16 u16Mask)
{
    MS_U16 val = HAL_MIU_Read2Byte(u32RegOffset);
    val = (bEnable) ? (val | u16Mask) : (val & ~u16Mask);
    HAL_MIU_Write2Byte(u32RegOffset, val);
}

//-------------------------------------------------------------------------------------------------
//  MTLB HAL function
//-------------------------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MIU_GetDefaultClientID_KernelProtect()
/// @brief \b Function \b Description:  Get default client id array pointer for protect kernel
/// @param <RET>           \b     : The pointer of Array of client IDs
////////////////////////////////////////////////////////////////////////////////
MS_U8* HAL_MIU_GetDefaultClientID_KernelProtect()
{
     if(IDNUM_KERNELPROTECT > 0)
         return  (MS_U8 *)&clientId_KernelProtect[0];

     return NULL;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MIU_Protect()
/// @brief \b Function \b Description:  Enable/Disable MIU Protection mode
/// @param u8Blockx        \b IN     : MIU Block to protect (0 ~ 4)
/// @param *pu8ProtectId   \b IN     : Allow specified client IDs to write
/// @param u32Start        \b IN     : Starting bus address
/// @param u32End          \b IN     : End bus address
/// @param bSetFlag        \b IN     : Disable or Enable MIU protection
///                                      - -Disable(0)
///                                      - -Enable(1)
/// @param <OUT>           \b None    :
/// @param <RET>           \b None    :
/// @param <GLOBAL>        \b None    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_Protect(
                          MS_U8    u8Blockx,
                          MS_U8    *pu8ProtectId,
                          MS_U32   u32BusStart,
                          MS_U32   u32BusEnd,
                          MS_BOOL  bSetFlag
                         )
{
    MS_U32 u32RegAddr;
    MS_U32 u32Reg;
    MS_U32 u32RegAddrStar;
    MS_U32 u32RegAddrMSB;
    MS_U32 u32RegAddrIDenable;
    MS_U32 u32MiuProtectEn;
    MS_U32 u32StartOffset;
    MS_U32 u32EndOffset;
    MS_U16 u16Data;
    MS_U16 u16Data1;
    MS_U16 u16Data2;
    MS_U8  u8Data;
    MS_U8  u8MiuSel;
    MS_U32 u32Start, u32End;

    u32Start = HAL_MIU_BA2PA(u32BusStart);
    u32End = HAL_MIU_BA2PA(u32BusEnd);

    // Get MIU selection and offset
	_phy_to_miu_offset(u8MiuSel, u32EndOffset, u32End);
    _phy_to_miu_offset(u8MiuSel, u32StartOffset, u32Start);

    u32Start = u32StartOffset;
    u32End = u32EndOffset;

    // Incorrect Block ID
    if(u8Blockx >= E_MIU_BLOCK_NUM)
    {
        MIU_HAL_ERR("Err: Out of the number of protect device\n");
        return FALSE;
    }
    else if(((u32Start & ((1 << MIU_PAGE_SHIFT) -1)) != 0) || ((u32End & ((1 << MIU_PAGE_SHIFT) -1)) != 0))
    {
        MIU_HAL_ERR("Err: Protected address should be aligned to 8KB\n");
        return FALSE;
    }
    else if(u32Start >= u32End)
    {
        MIU_HAL_ERR("Err: Start address is equal to or more than end address\n");
        return FALSE;
    }

    //write_enable
    u8Data = 1 << u8Blockx;

    if(u8MiuSel == E_CHIP_MIU_0)
    {
        u32RegAddrMSB = MIU_PROTECT0_MSB;
        u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);

        u32RegAddr = MIU_PROTECT0_ID0;
	    u32MiuProtectEn=MIU_PROTECT_EN_INTERNAL;
        u32Reg = MIU_REG_BASE;

        switch (u8Blockx)
        {
            case E_MIU_BLOCK_0:
                u32RegAddrStar = MIU_PROTECT0_START;
                u32RegAddrIDenable = MIU_PROTECT0_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFFF0UL);
                break;
            case E_MIU_BLOCK_1:
                u32RegAddrStar = MIU_PROTECT1_START;
                u32RegAddrIDenable = MIU_PROTECT1_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFF0FUL);
                break;
            case E_MIU_BLOCK_2:
                u32RegAddrStar = MIU_PROTECT2_START;
                u32RegAddrIDenable = MIU_PROTECT2_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xF0FFUL);
                break;
            case E_MIU_BLOCK_3:
                u32RegAddrStar = MIU_PROTECT3_START;
                u32RegAddrIDenable = MIU_PROTECT3_ID_ENABLE;
                u16Data2 = (u16Data1 & 0x0FFFUL);
                break;
            default:
		        return FALSE;
        }
    }
    else if(u8MiuSel == E_CHIP_MIU_1)
    {
        u32RegAddrMSB = MIU1_PROTECT0_MSB;
        u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);

        u32RegAddr = MIU1_PROTECT0_ID0;
	    u32MiuProtectEn=MIU1_PROTECT_EN;
        u32Reg = MIU1_REG_BASE;

        switch (u8Blockx)
        {
            case E_MIU_BLOCK_0:
                u32RegAddrStar = MIU1_PROTECT0_START;
                u32RegAddrIDenable = MIU1_PROTECT0_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFFF0UL);
                break;
            case E_MIU_BLOCK_1:
                u32RegAddrStar = MIU1_PROTECT1_START;
                u32RegAddrIDenable = MIU1_PROTECT1_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFF0FUL);
                break;
            case E_MIU_BLOCK_2:
                u32RegAddrStar = MIU1_PROTECT2_START;
                u32RegAddrIDenable = MIU1_PROTECT2_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xF0FFUL);
                break;
            case E_MIU_BLOCK_3:
                u32RegAddrStar = MIU1_PROTECT3_START;
                u32RegAddrIDenable = MIU1_PROTECT3_ID_ENABLE;
                u16Data2 = (u16Data1 & 0x0FFFUL);
                break;
            default:
		        return FALSE;
        }
    }
    else
    {
        MIU_HAL_ERR("%s not support MIU%u!\n", __FUNCTION__, u8MiuSel );
        return FALSE;
    }

    // Disable MIU protect
    HAL_MIU_WriteRegBit(u32MiuProtectEn,u8Data,DISABLE);

    if ( bSetFlag )
    {
        // Set Protect IDs
        if(HAL_MIU_SetGroupID(u8MiuSel, u8Blockx, pu8ProtectId, u32RegAddr, u32RegAddrIDenable) == FALSE)
        {
            return FALSE;
        }

        // Set BIT29,30 of start/end address
        u16Data2 = u16Data2 | (MS_U16)((u32Start >> 29) << (u8Blockx*4));						// u16Data2 for start_ext addr
        u16Data1 = u16Data2 | (MS_U16)(((u32End - 1) >> 29) << (u8Blockx*4+2));
        HAL_MIU_Write2Byte(u32RegAddrMSB, u16Data1);

        // Start Address
        u16Data = (MS_U16)(u32Start >> MIU_PAGE_SHIFT);   //8k/unit
        HAL_MIU_Write2Byte(u32RegAddrStar , u16Data);

        // End Address
        u16Data = (MS_U16)((u32End >> MIU_PAGE_SHIFT)-1);   //8k/unit;
        HAL_MIU_Write2Byte(u32RegAddrStar + 2, u16Data);

        // Enable MIU protect
        HAL_MIU_WriteRegBit(u32MiuProtectEn, u8Data, ENABLE);
    }
    else
    {
        // Reset Protect IDs
        HAL_MIU_ResetGroupID(u8MiuSel, u8Blockx, pu8ProtectId, u32RegAddr, u32RegAddrIDenable);
    }

    // clear log
    HAL_MIU_Write2BytesBit(u32Reg+REG_MIU_PROTECT_STATUS, TRUE, REG_MIU_PROTECT_LOG_CLR);
    HAL_MIU_Write2BytesBit(u32Reg+REG_MIU_PROTECT_STATUS, FALSE, REG_MIU_PROTECT_LOG_CLR);

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_ParseOccupiedResource
/// @brief \b Function  \b Description: Parse occupied resource to software structure
/// @return             \b 0: Fail 1: OK
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_ParseOccupiedResource(void)
{
    MS_U8  u8MiuSel;
    MS_U8  u8Blockx;
    MS_U8  u8ClientID;
    MS_U16 u16idenable;
    MS_U32 u32index;
    MS_U32 u32RegAddr;
    MS_U32 u32RegAddrIDenable;

    for(u8MiuSel = E_MIU_0; u8MiuSel < MIU_MAX_DEVICE; u8MiuSel++)
    {
        for(u8Blockx = E_MIU_BLOCK_0; u8Blockx < E_MIU_BLOCK_NUM; u8Blockx++)
        {
            if(u8MiuSel == E_MIU_0)
            {
                u32RegAddr = MIU_PROTECT0_ID0;

                switch (u8Blockx)
                {
                    case E_MIU_BLOCK_0:
                        u32RegAddrIDenable = MIU_PROTECT0_ID_ENABLE;
                        break;
                    case E_MIU_BLOCK_1:
                        u32RegAddrIDenable = MIU_PROTECT1_ID_ENABLE;
                        break;
                    case E_MIU_BLOCK_2:
                        u32RegAddrIDenable = MIU_PROTECT2_ID_ENABLE;
                        break;
                    case E_MIU_BLOCK_3:
                        u32RegAddrIDenable = MIU_PROTECT3_ID_ENABLE;
                        break;
                    default:
                        return false;
                }
            }
            else if(u8MiuSel == E_MIU_1)
            {
                u32RegAddr = MIU1_PROTECT0_ID0;

                switch (u8Blockx)
                {
                case E_MIU_BLOCK_0:
                     u32RegAddrIDenable = MIU1_PROTECT0_ID_ENABLE;
                     break;
                 case E_MIU_BLOCK_1:
                     u32RegAddrIDenable = MIU1_PROTECT1_ID_ENABLE;
                     break;
                 case E_MIU_BLOCK_2:
                     u32RegAddrIDenable = MIU1_PROTECT2_ID_ENABLE;
                     break;
                 case E_MIU_BLOCK_3:
                     u32RegAddrIDenable = MIU1_PROTECT3_ID_ENABLE;
                     break;
                 default:
                     return false;
                }
            }
            else
            {
                printk(KERN_ERR "%s not support MIU%u!\n", __FUNCTION__, u8MiuSel );
                return FALSE;
            }

            u16idenable = HAL_MIU_Read2Byte(u32RegAddrIDenable);
            for(u32index = 0; u32index < MIU_MAX_PROTECT_ID; u32index++)
            {
                IDEnables[u8MiuSel][u8Blockx][u32index] = ((u16idenable >> u32index) & 0x1UL)? 1: 0;
            }
        }//for(u8Blockx = E_MIU_BLOCK_0; u8Blockx < E_MIU_BLOCK_NUM; u8Blockx++)

        for(u32index = 0; u32index < MIU_MAX_PROTECT_ID; u32index++)
        {
            u8ClientID = HAL_MIU_ReadByte(u32RegAddr + u32index) & 0x7F;
            IDs[u8MiuSel][u32index] = clientTbl[u8MiuSel][u8ClientID];
        }
    }//for(u8MiuSel = E_MIU_0; u8MiuSel < E_MIU_NUM; u8MiuSel++)

    return TRUE;
}

//#define MIU_SLIT_DEBUG
MIU_Bank MIU_NormalBankInfo[MIU_MAX_DEVICE];
MIU_BWBank MIU_BWBankInfo[MIU_MAX_DEVICE][2];
//MIU_BWBankInfo[MIU_MAX_DEVICE][0] for ARB Base
//MIU_BWBankInfo[MIU_MAX_DEVICE][1] for ARBB Base
SRAM_Slit_FlagGroup Slit_FlagGroupInfo[MIU_MAX_DEVICE];
MS_BOOL gbSlitInited = FALSE;

MS_BOOL HAL_MIU_Save(void)
{
    int index;

//protect ID enable
    for( index = 0; index < 4; index++ )
    {
        MIU_NormalBankInfo[0].u16ProtectIDEn[index] = HAL_MIU_Read2Byte(MIU_PROTECT0_ID_ENABLE+index*2);
        MIU_NormalBankInfo[1].u16ProtectIDEn[index] = HAL_MIU_Read2Byte(MIU1_PROTECT0_ID_ENABLE+index*2);
    }
//protect ID
    for( index = 0; index < MIU_MAX_PROTECT_ID; index++ )
    {
        MIU_NormalBankInfo[0].u8GroupClient[index] = HAL_MIU_ReadByte(MIU_PROTECT0_ID0+index);
        MIU_NormalBankInfo[1].u8GroupClient[index] = HAL_MIU_ReadByte(MIU1_PROTECT0_ID0+index);
    }
//RQ
    for( index = 0; index < 0x40; index++ )
    {
        MIU_NormalBankInfo[0].u16RQ[index] = HAL_MIU_Read2Byte( MIU_RQ+index*2 );
        MIU_NormalBankInfo[1].u16RQ[index] = HAL_MIU_Read2Byte( MIU1_RQ+index*2 );
    }
//miu protect addr
    for( index = 0; index < 9; index++ )
    {
        MIU_NormalBankInfo[0].u16ProtectAddr[index] = HAL_MIU_Read2Byte(MIU_PROTECT0_START+index*2);
        MIU_NormalBankInfo[1].u16ProtectAddr[index] = HAL_MIU_Read2Byte(MIU1_PROTECT0_START+index*2);
    }
//After Protect ID & Addr, enable miu protect
    MIU_NormalBankInfo[0].u16ProtectEn = HAL_MIU_Read2Byte(MIU_PROTECT_EN_INTERNAL);
    MIU_NormalBankInfo[1].u16ProtectEn = HAL_MIU_Read2Byte(MIU1_PROTECT_EN);
//miu select
    for( index = 0; index < 6; index++ )
    {
        MIU_NormalBankInfo[0].u16SelMIU[index] = HAL_MIU_Read2Byte(REG_MIU_SEL0+index*2);
        MIU_NormalBankInfo[1].u16SelMIU[index] = HAL_MIU_Read2Byte(REG_MIU1_SEL0+index*2);
    }
//group priority
    MIU_NormalBankInfo[0].u16GroupPriority = HAL_MIU_Read2Byte(MIU_REG_BASE+REG_MIU_GROUP_PRIORITY);
    MIU_NormalBankInfo[1].u16GroupPriority = HAL_MIU_Read2Byte(MIU1_REG_BASE+REG_MIU_GROUP_PRIORITY);
    MIU_BWBankInfo[0][0].u16GroupPriority = HAL_MIU_Read2Byte( MIU_ARB_REG_BASE+REG_MIU_GROUP_PRIORITY );
    MIU_BWBankInfo[1][0].u16GroupPriority = HAL_MIU_Read2Byte( MIU1_ARB_REG_BASE+REG_MIU_GROUP_PRIORITY );

//BWRQ
    for( index = 0; index < 0x20; index++ )
    {
        MIU_BWBankInfo[0][0].u16BWRQ[index] = HAL_MIU_Read2Byte( MIU_ARB_REG_BASE+index*2 );
        MIU_BWBankInfo[0][1].u16BWRQ[index] = HAL_MIU_Read2Byte( MIU_ARBB_REG_BASE+index*2 );
        MIU_BWBankInfo[1][0].u16BWRQ[index] = HAL_MIU_Read2Byte( MIU1_ARB_REG_BASE+index*2 );
        MIU_BWBankInfo[1][1].u16BWRQ[index] = HAL_MIU_Read2Byte( MIU1_ARBB_REG_BASE+index*2 );
    }
//Slit MIU
//    for( index = 0; index < 0x10; index++ )
//    {
//        MIU_BWBankInfo[0][0].u16Slit[index] = HAL_MIU_Read2Byte( REG_MIU0_SLIT_RW_ID0+index*2 );
//        MIU_BWBankInfo[1][0].u16Slit[index] = HAL_MIU_Read2Byte( REG_MIU1_SLIT_RW_ID0+index*2 );
//    }
//    for(index = 0; index < SLIT_NUM; index++)
//    {
//        Slit_FlagGroupInfo[0].cluster[index].flag[0] = _gSlit0_FlagGroupBase->cluster[index].flag[0];
//        Slit_FlagGroupInfo[1].cluster[index].flag[0] = _gSlit1_FlagGroupBase->cluster[index].flag[0];
//    }
#ifdef  MIU_SLIT_DEBUG
    HAL_MIU_FlagSramDump(E_MIU_0);
    HAL_MIU_FlagSramDump(E_MIU_1);
#endif
    if((TRUE == gbSlitInited)&&(_gSlit0_FlagGroupBase)&&(_gSlit1_FlagGroupBase))
    {
        iounmap(_gSlit0_FlagGroupBase);
        iounmap(_gSlit1_FlagGroupBase);
        _gSlit0_FlagGroupBase = 0;
        _gSlit1_FlagGroupBase = 0;
    }
//BW ctrl
    for( index = 0; index <= 0x0D; index++ )
    {
        MIU_BWBankInfo[0][0].u16BWCtrl[index] = HAL_MIU_Read2Byte( MIU_BW_CTRL+index*2 );
        MIU_BWBankInfo[1][0].u16BWCtrl[index] = HAL_MIU_Read2Byte( MIU1_BW_CTRL+index*2 );
    }
//BW select MIU
    for( index = 0; index < 2; index++ )
    {
        MIU_BWBankInfo[0][0].u16BWSelMIU[index] = HAL_MIU_Read2Byte( REG_MIU_SEL6+index*2 );
        MIU_BWBankInfo[1][0].u16BWSelMIU[index] = HAL_MIU_Read2Byte( REG_MIU1_SEL6+index*2 );
    }
    return TRUE;
}

void HAL_MIU_FlagSramDump(MS_U8 u8MiuSel)
{
    SRAM_Slit_FlagGroup *_gSlit_FlagGroupBase;
    MS_U8 bPrevStatus;
    MS_U8 bCurrStatus;
    MS_U16 u16RegVal;
    MS_U16 u16LowBoundidx;
    MS_U32 u32RegAddr0;
    MS_U32 u32PrevIndx;
    MS_U32 u32CurrIndx;
    MS_U32 u32RegLowBound;

    printk("\n ===== MIU%d Slit =====\n", u8MiuSel);

    if(u8MiuSel == E_CHIP_MIU_0)
    {
        _gSlit_FlagGroupBase = _gSlit0_FlagGroupBase;
        u32RegAddr0 = REG_MIU0_SLIT_RW_ID0;
        u32RegLowBound = REG_MIU0_SLIT_LOW_BOUND;
    }
    else if(u8MiuSel == E_CHIP_MIU_1)
    {
        _gSlit_FlagGroupBase = _gSlit1_FlagGroupBase;
        u32RegAddr0 = REG_MIU1_SLIT_RW_ID0;
        u32RegLowBound = REG_MIU1_SLIT_LOW_BOUND;
    }
    else
    {
        return;
    }

    //Flush memory
    Chip_Flush_Memory();

    // Enable the accessibility of of slit sram for HEMCU
    u16RegVal = HAL_MIU_Read2Byte(u32RegAddr0);
    u16RegVal |= REG_MIU_SLIT_ID_ENABLE;
    HAL_MIU_Write2Byte(u32RegAddr0, u16RegVal);

    // Wait for complete of switch
    smp_mb();
    //Flush memory
    Chip_Flush_Memory();
    //udelay(100);

    if((_gSlit_FlagGroupBase->cluster[0].flag[0] & 0x01) == 0x01)
    {
       bPrevStatus = 1;
    }
    else
    {
       bPrevStatus = 0;
    }

    // Get low bound base idx, an index means 1MB entry
    u16LowBoundidx = HAL_MIU_Read2Byte(u32RegLowBound);

    // Dump between 0 to (n-1)th slot
    u32PrevIndx = 0;
    for (u32CurrIndx = 1; u32CurrIndx < 256; u32CurrIndx++)
    {
        if((_gSlit_FlagGroupBase->cluster[u32CurrIndx].flag[0] & 0x01) == 0x01)
        {
           bCurrStatus = 1;
        }
        else
        {
           bCurrStatus = 0;
        }

        if(bCurrStatus != bPrevStatus )
        {
            if(bPrevStatus == 1)
            {
                printk("<%04dMB> - <%04dMB-1>: enable\n", u32PrevIndx + u16LowBoundidx, u32CurrIndx + u16LowBoundidx);
            }
            else
            {
                printk("<%04dMB> - <%04dMB-1>: disable\n", u32PrevIndx + u16LowBoundidx, u32CurrIndx + u16LowBoundidx);
            }

            bPrevStatus = bCurrStatus;
            u32PrevIndx = u32CurrIndx;
        }
    }

    // Dump the n-th slot
    if(bPrevStatus == 1)
    {
        printk("<%04dMB> - <%04dMB-1>: enable\n", u32PrevIndx + u16LowBoundidx, u32CurrIndx + u16LowBoundidx);
    }
    else
    {
        printk("<%04dMB> - <%04dMB-1>: disable\n", u32PrevIndx + u16LowBoundidx, u32CurrIndx + u16LowBoundidx);
    }

    // Disable the accessibility of of slit sram for HEMCU
    u16RegVal = HAL_MIU_Read2Byte(u32RegAddr0);
    u16RegVal &= ~REG_MIU_SLIT_ID_ENABLE;
    HAL_MIU_Write2Byte(u32RegAddr0, u16RegVal);
}

MS_BOOL HAL_MIU_Restore(void)
{
    MS_U16 u16Data;
    int index;

// First Disable miu protect
    HAL_MIU_Write2BytesBit(MIU_PROTECT_EN, FALSE, BIT3 | BIT2 | BIT1 | BIT0);
    HAL_MIU_Write2BytesBit(MIU1_PROTECT_EN, FALSE, BIT3 | BIT2 | BIT1 | BIT0);
//protect ID enable
    for( index = 0; index < 4; index++ )
    {
        HAL_MIU_Write2Byte( MIU_PROTECT0_ID_ENABLE+index*2, MIU_NormalBankInfo[0].u16ProtectIDEn[index] );
        HAL_MIU_Write2Byte( MIU1_PROTECT0_ID_ENABLE+index*2, MIU_NormalBankInfo[1].u16ProtectIDEn[index] );
    }
//protect ID
    for( index = 0; index < MIU_MAX_PROTECT_ID; index++ )
    {
        HAL_MIU_WriteByte( MIU_PROTECT0_ID0+index, MIU_NormalBankInfo[0].u8GroupClient[index] );
        HAL_MIU_WriteByte( MIU1_PROTECT0_ID0+index, MIU_NormalBankInfo[1].u8GroupClient[index] );
    }
//RQ
    for( index = 0; index < 0x40; index++ )
    {
        HAL_MIU_Write2Byte( MIU_RQ+index*2, MIU_NormalBankInfo[0].u16RQ[index] );
        HAL_MIU_Write2Byte( MIU1_RQ+index*2, MIU_NormalBankInfo[1].u16RQ[index] );
    }
//miu protect addr
    for( index = 0; index < 9; index++ )
    {
        HAL_MIU_Write2Byte( MIU_PROTECT0_START+index*2, MIU_NormalBankInfo[0].u16ProtectAddr[index] );
        HAL_MIU_Write2Byte( MIU1_PROTECT0_START+index*2, MIU_NormalBankInfo[1].u16ProtectAddr[index] );
    }
//After Protect ID & Addr, enable miu protect
    HAL_MIU_Write2Byte( MIU_PROTECT_EN_INTERNAL, MIU_NormalBankInfo[0].u16ProtectEn );
    HAL_MIU_Write2Byte( MIU1_PROTECT_EN, MIU_NormalBankInfo[1].u16ProtectEn );
//miu select
    for( index = 0; index < 6; index++ )
    {
        HAL_MIU_Write2Byte( REG_MIU_SEL0+index*2, MIU_NormalBankInfo[0].u16SelMIU[index] );
        HAL_MIU_Write2Byte( REG_MIU1_SEL0+index*2, MIU_NormalBankInfo[1].u16SelMIU[index] );
    }
//group priority
    HAL_MIU_Write2Byte( MIU_REG_BASE+REG_MIU_GROUP_PRIORITY, MIU_NormalBankInfo[0].u16GroupPriority );
    HAL_MIU_Write2Byte( MIU1_REG_BASE+REG_MIU_GROUP_PRIORITY, MIU_NormalBankInfo[1].u16GroupPriority );
    HAL_MIU_Write2Byte( MIU_ARB_REG_BASE+REG_MIU_GROUP_PRIORITY, MIU_BWBankInfo[0][0].u16GroupPriority );
    HAL_MIU_Write2Byte( MIU1_ARB_REG_BASE+REG_MIU_GROUP_PRIORITY, MIU_BWBankInfo[1][0].u16GroupPriority );

//BWRQ
    for( index = 0; index < 0x20; index++ )
    {
        HAL_MIU_Write2Byte( MIU_ARB_REG_BASE+index*2, MIU_BWBankInfo[0][0].u16BWRQ[index] );
        HAL_MIU_Write2Byte( MIU_ARBB_REG_BASE+index*2, MIU_BWBankInfo[0][1].u16BWRQ[index] );
        HAL_MIU_Write2Byte( MIU1_ARB_REG_BASE+index*2, MIU_BWBankInfo[1][0].u16BWRQ[index] );
        HAL_MIU_Write2Byte( MIU1_ARBB_REG_BASE+index*2, MIU_BWBankInfo[1][1].u16BWRQ[index] );
    }
//Slit MIU
//    if (gbSlitInited == TRUE)
//    {
//        HAL_MIU_SlitInit();
//    }
//    for( index = 0; index < 0x10; index++ )
//    {
//        HAL_MIU_Write2Byte( REG_MIU0_SLIT_RW_ID0+index*2, MIU_BWBankInfo[0][0].u16Slit[index] );
//        HAL_MIU_Write2Byte( REG_MIU1_SLIT_RW_ID0+index*2, MIU_BWBankInfo[1][0].u16Slit[index] );
//    }
//    //MIU0
//    // Disable MIU protect
//    HAL_MIU_WriteRegBit(REG_MIU0_SLIT_PROTECT_EN, BIT0, DISABLE);
//    // Enable slit sram
//    u16Data = HAL_MIU_Read2Byte(REG_MIU0_SLIT_CTL);
//    u16Data |= REG_MIU_SLIT_TOP_EN;
//    HAL_MIU_Write2Byte(REG_MIU0_SLIT_CTL, u16Data);
//    //Flush memory
//    Chip_Flush_Memory();
//    // Enable the accessibility of of slit sram for HEMCU
//    u16Data = HAL_MIU_Read2Byte(REG_MIU0_SLIT_RW_ID0);
//    u16Data |= REG_MIU_SLIT_ID_ENABLE;
//    HAL_MIU_Write2Byte(REG_MIU0_SLIT_RW_ID0, u16Data);
//    // Wait for complete of switch
//    udelay(100);
//    for(index = 0; index < SLIT_NUM; index++)
//    {
//        _gSlit0_FlagGroupBase->cluster[index].flag[0] = Slit_FlagGroupInfo[0].cluster[index].flag[0];
//        //Flush memory
//        Chip_Flush_Memory();
//    }
//    // Disable the accessibility of of slit sram for HEMCU
//    u16Data = HAL_MIU_Read2Byte(REG_MIU0_SLIT_RW_ID0);
//    u16Data &= ~REG_MIU_SLIT_ID_ENABLE;
//    HAL_MIU_Write2Byte(REG_MIU0_SLIT_RW_ID0, u16Data);
//    // Disable slit sram
//    u16Data = HAL_MIU_Read2Byte(REG_MIU0_SLIT_RW_ID0);
//    u16Data &= ~REG_MIU_SLIT_TOP_EN;
//    HAL_MIU_Write2Byte(REG_MIU0_SLIT_RW_ID0, u16Data);
//    // Enable MIU protect
//    HAL_MIU_Write2Byte(MIU_ARB_REG_BASE+0x52UL, MIU_BWBankInfo[0][0].u16Slit[9]);
//    ////MIU1
//    // Disable MIU protect
//    HAL_MIU_WriteRegBit(REG_MIU1_SLIT_PROTECT_EN, BIT0, DISABLE);
//    // Enable slit sram
//    u16Data = HAL_MIU_Read2Byte(REG_MIU1_SLIT_CTL);
//    u16Data |= REG_MIU_SLIT_TOP_EN;
//    HAL_MIU_Write2Byte(REG_MIU1_SLIT_CTL, u16Data);
//    //Flush memory
//    Chip_Flush_Memory();
//    // Enable the accessibility of of slit sram for HEMCU
//    u16Data = HAL_MIU_Read2Byte(REG_MIU1_SLIT_RW_ID0);
//    u16Data |= REG_MIU_SLIT_ID_ENABLE;
//    HAL_MIU_Write2Byte(REG_MIU1_SLIT_RW_ID0, u16Data);
//    // Wait for complete of switch
//    udelay(100);
//    for(index = 0; index < SLIT_NUM; index++)
//    {
//        _gSlit1_FlagGroupBase->cluster[index].flag[0] = Slit_FlagGroupInfo[1].cluster[index].flag[0];
//        //Flush memory
//        Chip_Flush_Memory();
//    }
//    // Disable the accessibility of of slit sram for HEMCU
//    u16Data = HAL_MIU_Read2Byte(REG_MIU1_SLIT_RW_ID0);
//    u16Data &= ~REG_MIU_SLIT_ID_ENABLE;
//    HAL_MIU_Write2Byte(REG_MIU1_SLIT_RW_ID0, u16Data);
//    // Disable slit sram
//    u16Data = HAL_MIU_Read2Byte(REG_MIU1_SLIT_RW_ID0);
//    u16Data &= ~REG_MIU_SLIT_TOP_EN;
//    HAL_MIU_Write2Byte(REG_MIU1_SLIT_RW_ID0, u16Data);
//    // Enable MIU protect
//    HAL_MIU_Write2Byte(MIU1_ARB_REG_BASE+0x52UL, MIU_BWBankInfo[1][0].u16Slit[9]);
#ifdef  MIU_SLIT_DEBUG
    HAL_MIU_FlagSramDump(E_MIU_0);
    HAL_MIU_FlagSramDump(E_MIU_1);
#endif
//BW ctrl
    for( index = 0; index <= 0x0D; index++ )
    {
        HAL_MIU_Write2Byte( MIU_BW_CTRL+index*2, MIU_BWBankInfo[0][0].u16BWCtrl[index] );
        HAL_MIU_Write2Byte( MIU1_BW_CTRL+index*2, MIU_BWBankInfo[1][0].u16BWCtrl[index] );
    }
//BW select MIU
    for( index = 0; index < 2; index++ )
    {
        HAL_MIU_Write2Byte( REG_MIU_SEL6+index*2, MIU_BWBankInfo[0][0].u16BWSelMIU[index] );
        HAL_MIU_Write2Byte( REG_MIU1_SEL6+index*2, MIU_BWBankInfo[1][0].u16BWSelMIU[index] );
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_SlitInit
/// @brief \b Function  \b Description: Initialize slit sram
/// @return             \b 0: Fail 1: OK
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_SlitInit(void)
{
    MS_U32 u32RegAddr0;
    MS_U16 u16Val;
    MS_U64 u64FlagSramBase0;
    MS_U64 u64FlagSramBase1;
    MS_BOOL ret;

    gbSlitInited = TRUE;
    ret = TRUE;

    // Map virtual address to _gSlit0_FlagGroupBase
    u64FlagSramBase0 = MIU_FLAGSRAM_BUS_BASE;
    _gSlit0_FlagGroupBase = (SRAM_Slit_FlagGroup *)ioremap((phys_addr_t)u64FlagSramBase0, sizeof(SRAM_Slit_FlagGroup));
    if(!_gSlit0_FlagGroupBase)
    {
        MIU_HAL_ERR("ioremap _gSlit0_FlagGroupBase = 0x%zx fail\n", (size_t)_gSlit0_FlagGroupBase);
        ret = FALSE;
        BUG_ON(!_gSlit0_FlagGroupBase);
    }

    // Map virtual address to _gSlit1_FlagGroupBase
    u64FlagSramBase1 = MIU1_FLAGSRAM_BUS_BASE;
    _gSlit1_FlagGroupBase = (SRAM_Slit_FlagGroup *)ioremap((phys_addr_t)u64FlagSramBase1, sizeof(SRAM_Slit_FlagGroup));
    if(!_gSlit1_FlagGroupBase)
    {
        MIU_HAL_ERR("ioremap _gSlit1_FlagGroupBase = 0x%zx fail\n", (size_t)_gSlit1_FlagGroupBase);
        ret = FALSE;
        BUG_ON(!_gSlit1_FlagGroupBase);
    }

    printk("_gSlit0_FlagGroupBase = 0x%zx mapping on 0x%zx\n", (size_t)_gSlit0_FlagGroupBase, (size_t)u64FlagSramBase0);
    printk("_gSlit1_FlagGroupBase = 0x%zx mapping on 0x%zx\n", (size_t)_gSlit1_FlagGroupBase, (size_t)u64FlagSramBase1);

    // Setup HEMCH into programable ID list
    u32RegAddr0 = REG_MIU0_SLIT_RW_ID0;
    u16Val = HAL_MIU_GetClientInfo(E_MIU_0, (eMIUClientID)MIU_CLIENT_MIPS_RW);
    HAL_MIU_Write2Byte(u32RegAddr0, u16Val);

    u32RegAddr0 = REG_MIU1_SLIT_RW_ID0;
    u16Val = HAL_MIU_GetClientInfo(E_MIU_1, (eMIUClientID)MIU_CLIENT_MIPS_RW);
    HAL_MIU_Write2Byte(u32RegAddr0, u16Val);

    // Setup slit sram base address as MIU_FLAGSRAM_BASE with 16k unit
    u32RegAddr0 = REG_MIU0_SLIT_BASE_ADDR_LOW;
    HAL_MIU_Write2Byte(u32RegAddr0, (MIU_FLAGSRAM_BASE >> MIU_FLAGSRAM_BASE_SHIFT) & 0xffff);
    u32RegAddr0 = REG_MIU0_SLIT_BASE_ADDR_HIGH;
    HAL_MIU_Write2Byte(u32RegAddr0, ((MIU_FLAGSRAM_BASE >> MIU_FLAGSRAM_BASE_SHIFT) >> 16));

    u32RegAddr0 = REG_MIU1_SLIT_BASE_ADDR_LOW;
    HAL_MIU_Write2Byte(u32RegAddr0, (MIU1_FLAGSRAM_BASE >> MIU_FLAGSRAM_BASE_SHIFT) & 0xffff);
    u32RegAddr0 = REG_MIU1_SLIT_BASE_ADDR_HIGH;
    HAL_MIU_Write2Byte(u32RegAddr0, ((MIU1_FLAGSRAM_BASE >> MIU_FLAGSRAM_BASE_SHIFT) >> 16));

    return ret;
}
