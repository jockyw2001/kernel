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
/// @file   Mhal_emac.c
/// @brief  EMAC Driver
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/mii.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/pci.h>
#include "mhal_emac.h"

extern unsigned char phyaddr;

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
struct _MHalBasicConfigEMAC
{
    u8 connected;           // 0:No, 1:Yes
    u8 speed;               // 10:10Mbps, 100:100Mbps
    // ETH_CTL Register:
    u8 wes;                 // 0:Disable, 1:Enable (WR_ENABLE_STATISTICS_REGS)
    // ETH_CFG Register:
    u8 duplex;              // 1:Half-duplex, 2:Full-duplex
    u8 cam;                 // 0:No CAM, 1:Yes
    u8 rcv_bcast;           // 0:No, 1:Yes
    u8 rlf;                 // 0:No, 1:Yes receive long frame(1522)
    // MAC Address:
    u8 sa1[6];              // Specific Addr 1 (MAC Address)
    u8 sa2[6];              // Specific Addr 2
    u8 sa3[6];              // Specific Addr 3
    u8 sa4[6];              // Specific Addr 4
};
typedef struct _MHalBasicConfigEMAC MHalBasicConfigEMAC;

struct _MHalUtilityVarsEMAC
{
    u32 cntChkCableConnect;
    u32 cntChkINTCounter;
    u32 readIdxRBQP;        // Reset = 0x00000000
    u32 rxOneFrameAddr;     // Reset = 0x00000000 (Store the Addr of "ReadONE_RX_Frame")
    u8 flagISR_INT_DONE;
};
typedef struct _MHalUtilityVarsEMAC MHalUtilityVarsEMAC;

MHalBasicConfigEMAC MHalThisBCE;
MHalUtilityVarsEMAC MHalThisUVE;

#define MHal_MAX_INT_COUNTER    100
//-------------------------------------------------------------------------------------------------
//  EMAC hardware for Titania
//-------------------------------------------------------------------------------------------------

/*8-bit RIU address*/
u8 MHal_EMAC_ReadReg8( u32 bank, u32 reg )
{
    u8 val;
    phys_addr_t address = EMAC_RIU_REG_BASE + bank*0x100UL*2;
    address = address + (reg << 1) - (reg & 1);

    val = *( ( volatile u8* ) address );
    return val;
}

void MHal_EMAC_WritReg8( u32 bank, u32 reg, u8 val )
{
    phys_addr_t address = EMAC_RIU_REG_BASE + bank*0x100UL*2;
    address = address + (reg << 1) - (reg & 1);

    *( ( volatile u8* ) address ) = val;
}
u32 MHal_EMAC_ReadReg32( u32 xoffset )
{
    phys_addr_t address = REG_ADDR_BASE + xoffset*2;

    u32 xoffsetValueL = *( ( volatile u32* ) address ) & 0x0000FFFFUL;
    u32 xoffsetValueH = *( ( volatile u32* ) ( address + 4) ) << 0x10UL;
    return( xoffsetValueH | xoffsetValueL );
}

void MHal_EMAC_WritReg32( u32 xoffset, u32 xval )
{
    phys_addr_t address = REG_ADDR_BASE + xoffset*2;

    *( ( volatile u32 * ) address ) = ( u32 ) ( xval & 0x0000FFFFUL );
    *( ( volatile u32 * ) ( address + 4 ) ) = ( u32 ) ( xval >> 0x10UL );
}

u32 MHal_EMAC_ReadRam32( phys_addr_t uRamAddr, u32 xoffset)
{
    return (*( u32 * ) ( ( char * ) uRamAddr + xoffset ) );
}

void MHal_EMAC_WritRam32( phys_addr_t uRamAddr, u32 xoffset, u32 xval )
{
    *( ( u32 * ) ( ( char * ) uRamAddr + xoffset ) ) = xval;
}

void MHal_EMAC_Write_SA1_MAC_Address( u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u32 w0 = ( u32 ) m3 << 24 | m2 << 16 | m1 << 8 | m0;
    u32 w1 = ( u32 ) m5 << 8 | m4;
    MHal_EMAC_WritReg32( REG_ETH_SA1L, w0 );
    MHal_EMAC_WritReg32( REG_ETH_SA1H, w1 );
}

void MHal_EMAC_Write_SA2_MAC_Address( u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u32 w0 = ( u32 ) m3 << 24 | m2 << 16 | m1 << 8 | m0;
    u32 w1 = ( u32 ) m5 << 8 | m4;
    MHal_EMAC_WritReg32( REG_ETH_SA2L, w0 );
    MHal_EMAC_WritReg32( REG_ETH_SA2H, w1 );
}

void MHal_EMAC_Write_SA3_MAC_Address( u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u32 w0 = ( u32 ) m3 << 24 | m2 << 16 | m1 << 8 | m0;
    u32 w1 = ( u32 ) m5 << 8 | m4;
    MHal_EMAC_WritReg32( REG_ETH_SA3L, w0 );
    MHal_EMAC_WritReg32( REG_ETH_SA3H, w1 );
}

void MHal_EMAC_Write_SA4_MAC_Address( u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u32 w0 = ( u32 ) m3 << 24 | m2 << 16 | m1 << 8 | m0;
    u32 w1 = ( u32 ) m5 << 8 | m4;
    MHal_EMAC_WritReg32( REG_ETH_SA4L, w0 );
    MHal_EMAC_WritReg32( REG_ETH_SA4H, w1 );
}

//-------------------------------------------------------------------------------------------------
//  R/W EMAC register for Titania
//-------------------------------------------------------------------------------------------------

void MHal_EMAC_update_HSH(u32 mc0, u32 mc1)
{
    MHal_EMAC_WritReg32( REG_ETH_HSL, mc0 );
    MHal_EMAC_WritReg32( REG_ETH_HSH, mc1 );
}

//-------------------------------------------------------------------------------------------------
//  Read control register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_CTL( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_CTL );
}

//-------------------------------------------------------------------------------------------------
//  Write Network control register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_CTL( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_CTL, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Network configuration register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_CFG( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_CFG );
}

//-------------------------------------------------------------------------------------------------
//  Write Network configuration register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_CFG( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_CFG, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read RBQP
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_RBQP( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_RBQP );
}

//-------------------------------------------------------------------------------------------------
//  Write RBQP
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_RBQP( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_RBQP, xval );
}

//-------------------------------------------------------------------------------------------------
//  Write Transmit Address register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_TAR( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_TAR, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read RBQP
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_TCR( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_TCR);
}

//-------------------------------------------------------------------------------------------------
//  Write Transmit Control register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_TCR( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_TCR, xval );
}

//-------------------------------------------------------------------------------------------------
//  Transmit Status Register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_TSR( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_TSR, xval );
}

u32 MHal_EMAC_Read_TSR( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_TSR );
}

void MHal_EMAC_Write_RSR( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_RSR, xval );
}

u32 MHal_EMAC_Read_RSR( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_RSR );
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt status register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_ISR( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_ISR, xval );
}

u32 MHal_EMAC_Read_ISR( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_ISR );
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt enable register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_IER( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_IER );
}

//-------------------------------------------------------------------------------------------------
//  Write Interrupt enable register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_IER( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_IER, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt disable register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_IDR( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_IDR );
}

//-------------------------------------------------------------------------------------------------
//  Write Interrupt disable register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_IDR( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_IDR, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt mask register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_IMR( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_IMR );
}

//-------------------------------------------------------------------------------------------------
//  Read PHY maintenance register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_MAN( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_MAN );
}

//-------------------------------------------------------------------------------------------------
//  Write PHY maintenance register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_MAN( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_MAN, xval );
}

//-------------------------------------------------------------------------------------------------
//  Write Receive Buffer Configuration
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_BUFF( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_BUFF, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Receive Buffer Configuration
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_BUFF( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_BUFF );
}

//-------------------------------------------------------------------------------------------------
//  Read Receive First Full Pointer
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_RDPTR( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_BUFFRDPTR );
}

//-------------------------------------------------------------------------------------------------
//  Write Receive First Full Pointer
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_RDPTR( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_BUFFRDPTR, xval );
}

//-------------------------------------------------------------------------------------------------
//  Write Receive First Full Pointer
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_WRPTR( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_BUFFWRPTR, xval );
}

//-------------------------------------------------------------------------------------------------
//  Frames transmitted OK
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_FRA( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_FRA );
}

//-------------------------------------------------------------------------------------------------
//  Single collision frames
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_SCOL( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_SCOL );
}

//-------------------------------------------------------------------------------------------------
//  Multiple collision frames
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_MCOL( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_MCOL );
}

//-------------------------------------------------------------------------------------------------
//  Frames received OK
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_OK( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_OK );
}

//-------------------------------------------------------------------------------------------------
//  Frame check sequence errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_SEQE( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_SEQE );
}

//-------------------------------------------------------------------------------------------------
//  Alignment errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_ALE( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_ALE );
}

//-------------------------------------------------------------------------------------------------
//  Late collisions
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_LCOL( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_LCOL );
}

//-------------------------------------------------------------------------------------------------
//  Excessive collisions
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_ECOL( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_ECOL );
}

//-------------------------------------------------------------------------------------------------
//  Transmit under-run errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_TUE( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_TUE );
}

//-------------------------------------------------------------------------------------------------
//  Carrier sense errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_CSE( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_CSE );
}

//-------------------------------------------------------------------------------------------------
//  Receive resource error
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_RE( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_RE );
}

//-------------------------------------------------------------------------------------------------
//  Received overrun
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_ROVR( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_ROVR );
}

//-------------------------------------------------------------------------------------------------
//  Received symbols error
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_SE( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_SE );
}

//-------------------------------------------------------------------------------------------------
//  Excessive length errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_ELR( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_ELR );
}

//-------------------------------------------------------------------------------------------------
//  Receive jabbers
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_RJB( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_RJB );
}

//-------------------------------------------------------------------------------------------------
//  Undersize frames
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_USF( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_USF );
}

//-------------------------------------------------------------------------------------------------
//  SQE test errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_SQEE( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_SQEE );
}

//-------------------------------------------------------------------------------------------------
//  Read Julian 100
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_JULIAN_0100( void )
{
    return MHal_EMAC_ReadReg32( REG_EMAC_JULIAN_0100 );
}

//-------------------------------------------------------------------------------------------------
//  Write Julian 100
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_JULIAN_0100( u32 xval )
{
    MHal_EMAC_WritReg32( REG_EMAC_JULIAN_0100, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Julian 104
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_JULIAN_0104( void )
{
    return MHal_EMAC_ReadReg32( REG_EMAC_JULIAN_0104 );
}

//-------------------------------------------------------------------------------------------------
//  Write Julian 104
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_JULIAN_0104( u32 xval )
{
    MHal_EMAC_WritReg32( REG_EMAC_JULIAN_0104, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Julian 108
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_JULIAN_0108( void )
{
    return MHal_EMAC_ReadReg32( REG_EMAC_JULIAN_0108 );
}

//-------------------------------------------------------------------------------------------------
//  Write Julian 108
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_JULIAN_0108( u32 xval )
{
    MHal_EMAC_WritReg32( REG_EMAC_JULIAN_0108, xval );
}

void MHal_EMAC_Set_Tx_JULIAN_T(u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(0x134UL);
    value &= 0xff0fffffUL;
    value |= xval << 20;

    MHal_EMAC_WritReg32(0x134UL, value);
}

void MHal_EMAC_Set_TEST(u32 xval)
{
    u32 value = 0xffffffffUL;
    int i=0;

    for(i = 0x100UL; i< 0x160UL;i+=4){
        MHal_EMAC_WritReg32(i, value);
        }

}

u32 MHal_EMAC_Get_Tx_FIFO_Threshold(void)
{
    return (MHal_EMAC_ReadReg32(0x134UL) & 0x00f00000UL) >> 20;
}

void MHal_EMAC_Set_Rx_FIFO_Enlarge(u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(0x134UL);
    value &= 0xfcffffffUL;
    value |= xval << 24;

    MHal_EMAC_WritReg32(0x134UL, value);
}

u32 MHal_EMAC_Get_Rx_FIFO_Enlarge(void)
{
    return (MHal_EMAC_ReadReg32(0x134UL) & 0x03000000UL) >> 24;
}

void MHal_EMAC_Set_Miu_Priority(u32 xval)
{
    u32 value;

    value = MHal_EMAC_ReadReg32(0x100UL);
    value &= 0xfff7ffffUL;
    value |= xval << 19;

    MHal_EMAC_WritReg32(0x100UL, value);
}

u32 MHal_EMAC_Get_Miu_Priority(void)
{
    return (MHal_EMAC_ReadReg32(0x100UL) & 0x00080000UL) >> 19;
}

void MHal_EMAC_Set_Tx_Hang_Fix_ECO(u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(0x134UL);
    value &= 0xfffbffffUL;
    value |= xval << 18;

    MHal_EMAC_WritReg32(0x134UL, value);
}

void MHal_EMAC_Set_MIU_Out_Of_Range_Fix(u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(0x134UL);
    value &= 0xefffffffUL;
    value |= xval << 28;

    MHal_EMAC_WritReg32(0x134UL, value);
}

void MHal_EMAC_Set_Rx_Tx_Burst16_Mode(u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(0x134UL);
    value &= 0xdfffffffUL;
    value |= xval << 29;

    MHal_EMAC_WritReg32(0x134UL, value);
}

void MHal_EMAC_Set_Tx_Rx_Req_Priority_Switch(u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(0x134UL);
    value &= 0xfff7ffffUL;
    value |= xval << 19;

    MHal_EMAC_WritReg32(0x134UL, value);
}

void MHal_EMAC_Set_Rx_Byte_Align_Offset(u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(0x134UL);
    value &= 0xf3ffffffUL;
    value |= xval << 26;

    MHal_EMAC_WritReg32(0x134UL, value);
}

void MHal_EMAC_Write_Protect(u32 start_addr, u32 length)
{
    u32 value;

    value = MHal_EMAC_ReadReg32(0x11cUL);
    value &= 0x0000ffffUL;
    value |= ((start_addr+length) >> 4) << 16;
    MHal_EMAC_WritReg32(0x11cUL, value);

    value = MHal_EMAC_ReadReg32(0x120);
    value &= 0x00000000UL;
    value |= ((start_addr+length) >> 4) >> 16;
    value |= (start_addr >> 4) << 16;
    MHal_EMAC_WritReg32(0x120UL, value);

    value = MHal_EMAC_ReadReg32(0x124UL);
    value &= 0xffff0000UL;
    value |= (start_addr >> 4) >> 16;
    MHal_EMAC_WritReg32(0x124UL, value);
}

void MHal_EMAC_Set_Miu_Highway(u32 xval)
{
    u32 value;
    value = MHal_EMAC_ReadReg32(0x134UL);
    value &= 0xbfffffffUL;
    value |= xval << 30;

    MHal_EMAC_WritReg32(0x134UL, value);
}

void MHal_EMAC_HW_init(void)
{
    MHal_EMAC_Set_Miu_Priority(1);
    MHal_EMAC_Set_Tx_JULIAN_T(4);
    MHal_EMAC_Set_Rx_Tx_Burst16_Mode(1);
    MHal_EMAC_Set_Rx_FIFO_Enlarge(2);
    MHal_EMAC_Set_Tx_Hang_Fix_ECO(1);
    MHal_EMAC_Set_MIU_Out_Of_Range_Fix(1);
    #ifdef RX_BYTE_ALIGN_OFFSET
    MHal_EMAC_Set_Rx_Byte_Align_Offset(2);
    #endif
    MHal_EMAC_WritReg32(REG_EMAC_JULIAN_0138, MHal_EMAC_ReadReg32(REG_EMAC_JULIAN_0138) | 0x00000001UL);
//    MHal_EMAC_Set_Miu_Highway(1);
}

//-------------------------------------------------------------------------------------------------
//  PHY INTERFACE
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Enable the MDIO bit in MAC control register
// When not called from an interrupt-handler, access to the PHY must be
// protected by a spinlock.
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_enable_mdi( void )
{
    u32 xval;
    xval = MHal_EMAC_Read_CTL();
    xval |= EMAC_MPE;
    MHal_EMAC_Write_CTL( xval );
}

//-------------------------------------------------------------------------------------------------
//  Disable the MDIO bit in the MAC control register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_disable_mdi( void )
{
    u32 xval;
    xval = MHal_EMAC_Read_CTL();
    xval &= ~EMAC_MPE;
    MHal_EMAC_Write_CTL( xval );
}

//-------------------------------------------------------------------------------------------------
// Write value to the a PHY register
// Note: MDI interface is assumed to already have been enabled.
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_write_phy( unsigned char phy_addr, unsigned char address, u32 value )
{
#if CONFIG_ETHERNET_ALBANY
    phys_addr_t uRegBase = INTERNEL_PHY_REG_BASE;

    phy_addr =0;

    *(volatile unsigned int *)(uRegBase + address*4) = value;
    udelay( 1 );
#else
    u32 uRegVal = 0, uCTL = 0;
    uRegVal =  ( EMAC_HIGH | EMAC_CODE_802_3 | EMAC_RW_W) | (( phy_addr & 0x1FUL ) << PHY_ADDR_OFFSET )
                | ( address << PHY_REGADDR_OFFSET ) | (value & 0xFFFFUL);

    uCTL = MHal_EMAC_Read_CTL();
    MHal_EMAC_enable_mdi();

    MHal_EMAC_Write_MAN( uRegVal );
    // Wait until IDLE bit in Network Status register is cleared //
    uRegVal = MHal_EMAC_ReadReg32( REG_ETH_SR );  //Must read Low 16 bit.
    while ( !( uRegVal & EMAC_IDLE ) )
    {
        uRegVal = MHal_EMAC_ReadReg32( REG_ETH_SR );
        barrier();
    }
    MHal_EMAC_Write_CTL(uCTL);
#endif
}
//-------------------------------------------------------------------------------------------------
// Read value stored in a PHY register.
// Note: MDI interface is assumed to already have been enabled.
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_read_phy( unsigned char phy_addr, unsigned char address, u32* value )
{
#if CONFIG_ETHERNET_ALBANY
    phys_addr_t uRegBase  = INTERNEL_PHY_REG_BASE;
    u32 tempvalue ;

    phy_addr =0;

    tempvalue = *(volatile unsigned int *)(INTERNEL_PHY_REG_BASE + 0x04UL);
    tempvalue |= 0x0004UL;
    *(volatile unsigned int *)(INTERNEL_PHY_REG_BASE + 0x04UL) = tempvalue;
    udelay( 1 );
    *value = *(volatile unsigned int *)(uRegBase + address*4);
#else
    u32 uRegVal = 0, uCTL = 0;

    uRegVal = (EMAC_HIGH | EMAC_CODE_802_3 | EMAC_RW_R)
            | ((phy_addr & 0x1fUL) << PHY_ADDR_OFFSET) | (address << PHY_REGADDR_OFFSET) | (0) ;

    uCTL = MHal_EMAC_Read_CTL();
    MHal_EMAC_enable_mdi();
    MHal_EMAC_Write_MAN(uRegVal);

    //Wait until IDLE bit in Network Status register is cleared //
    uRegVal = MHal_EMAC_ReadReg32( REG_ETH_SR );  //Must read Low 16 bit.
    while ( !( uRegVal & EMAC_IDLE ) )
    {
        uRegVal = MHal_EMAC_ReadReg32( REG_ETH_SR );
        barrier();
    }
    *value = ( MHal_EMAC_Read_MAN() & 0x0000ffffUL );
    MHal_EMAC_Write_CTL(uCTL);
#endif
}

#ifdef CONFIG_ETHERNET_ALBANY
void MHal_EMAC_TX_10MB_lookUp_table( void )
{
    // tx 10T link test pulse
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x0fUL*4) ) = 0x9800UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x10UL*4) ) = 0x8484UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x11UL*4) ) = 0x8888UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x12UL*4) ) = 0x8c8cUL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x13UL*4) ) = 0xC898UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x14UL*4) ) = 0x0000UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x15UL*4) ) = 0x1000UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x16UL*4) ) = ( (*( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x16UL*4) ) & 0xFF00UL) | 0x0000UL );

    // tx 10T look up table.
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x44UL*4) ) = 0x3C3CUL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x45UL*4) ) = 0x3C3CUL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x46UL*4) ) = 0x3C30UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x47UL*4) ) = 0x687CUL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x48UL*4) ) = 0x7834UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x49UL*4) ) = 0xD494UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x4AUL*4) ) = 0x84A0UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x4BUL*4) ) = 0xE4C8UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x4CUL*4) ) = 0xC8C8UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x4DUL*4) ) = 0xC8E8UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x4EUL*4) ) = 0x3C3CUL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x4FUL*4) ) = 0x3C3CUL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x50UL*4) ) = 0x2430UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x51UL*4) ) = 0x707CUL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x52UL*4) ) = 0x6420UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x53UL*4) ) = 0xD4A0UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x54UL*4) ) = 0x8498UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x55UL*4) ) = 0xD0C8UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x56UL*4) ) = 0xC8C8UL;
    *( ( u32 * ) ( ( char * ) INTERNEL_PHY_REG_BASE +  0x57UL*4) ) = 0xC8C8UL;
}
#endif

//-------------------------------------------------------------------------------------------------
// Update MAC speed and H/F duplex
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_update_speed_duplex( u32 uspeed, u32 uduplex )
{
    u32 xval;

    xval = MHal_EMAC_ReadReg32( REG_ETH_CFG ) & ~( EMAC_SPD | EMAC_FD );

    if ( uspeed == SPEED_100 )
    {
        if ( uduplex == DUPLEX_FULL )    // 100 Full Duplex //
        {
            xval = xval | EMAC_SPD | EMAC_FD;
        }
        else                           // 100 Half Duplex ///
        {
            xval = xval | EMAC_SPD;
        }
    }
    else
    {
        if ( uduplex == DUPLEX_FULL )    //10 Full Duplex //
        {
            xval = xval | EMAC_FD;
        }
        else                           // 10 Half Duplex //
        {
        }
    }
    MHal_EMAC_WritReg32( REG_ETH_CFG, xval );
}

u8 MHal_EMAC_CalcMACHash( u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u8 hashIdx0 = ( m0&0x01UL ) ^ ( ( m0&0x40UL ) >> 6 ) ^ ( ( m1&0x10UL ) >> 4 ) ^ ( ( m2&0x04UL ) >> 2 )
                 ^ ( m3&0x01UL ) ^ ( ( m3&0x40UL ) >> 6 ) ^ ( ( m4&0x10UL ) >> 4 ) ^ ( ( m5&0x04UL ) >> 2 );

     u8 hashIdx1 = ( m0&0x02UL ) ^ ( ( m0&0x80UL ) >> 6 ) ^ ( ( m1&0x20UL ) >> 4 ) ^ ( ( m2&0x08UL ) >> 2 )
                ^ ( m3&0x02UL ) ^ ( ( m3&0x80UL ) >> 6 ) ^ ( ( m4&0x20UL ) >> 4 ) ^ ( ( m5&0x08UL ) >> 2 );

    u8 hashIdx2 = ( m0&0x04UL ) ^ ( ( m1&0x01UL ) << 2 ) ^ ( ( m1&0x40UL ) >> 4 ) ^ ( ( m2&0x10UL ) >> 2 )
                ^ ( m3&0x04UL ) ^ ( ( m4&0x01UL ) << 2 ) ^ ( ( m4&0x40UL ) >> 4 ) ^ ( ( m5&0x10UL ) >> 2 );

    u8 hashIdx3 = ( m0&0x08UL ) ^ ( ( m1&0x02UL ) << 2 ) ^ ( ( m1&0x80UL ) >> 4 ) ^ ( ( m2&0x20UL ) >> 2 )
                ^ ( m3&0x08UL ) ^ ( ( m4&0x02UL ) << 2 ) ^ ( ( m4&0x80UL ) >> 4 ) ^ ( ( m5&0x20UL ) >> 2 );

    u8 hashIdx4 = ( m0&0x10UL ) ^ ( ( m1&0x04UL ) << 2 ) ^ ( ( m2&0x01UL ) << 4 ) ^ ( ( m2&0x40UL ) >> 2 )
                ^ ( m3&0x10UL ) ^ ( ( m4&0x04UL ) << 2 ) ^ ( ( m5&0x01UL ) << 4 ) ^ ( ( m5&0x40UL ) >> 2 );

    u8 hashIdx5 = ( m0&0x20UL ) ^ ( ( m1&0x08UL ) << 2 ) ^ ( ( m2&0x02UL ) << 4 ) ^ ( ( m2&0x80UL ) >> 2 )
                ^ ( m3&0x20UL ) ^ ( ( m4&0x08UL ) << 2 ) ^ ( ( m5&0x02UL ) << 4 ) ^ ( ( m5&0x80UL ) >> 2 );

    return( hashIdx0 | hashIdx1 | hashIdx2 | hashIdx3 | hashIdx4 | hashIdx5 );
}

//-------------------------------------------------------------------------------------------------
//Initialize and enable the PHY interrupt when link-state changes
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_enable_phyirq( void )
{
#if 0

#endif
}

//-------------------------------------------------------------------------------------------------
// Disable the PHY interrupt
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_disable_phyirq( void )
{
#if 0

#endif
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------

u32 MHal_EMAC_get_SA1H_addr( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_SA1H );
}

u32 MHal_EMAC_get_SA1L_addr( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_SA1L );
}

u32 MHal_EMAC_get_SA2H_addr( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_SA2H );
}

u32 MHal_EMAC_get_SA2L_addr( void )
{
    return MHal_EMAC_ReadReg32( REG_ETH_SA2L );
}

void MHal_EMAC_Write_SA1H( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_SA1H, xval );
}

void MHal_EMAC_Write_SA1L( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_SA1L, xval );
}

void MHal_EMAC_Write_SA2H( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_SA2H, xval );
}

void MHal_EMAC_Write_SA2L( u32 xval )
{
    MHal_EMAC_WritReg32( REG_ETH_SA2L, xval );
}

void* MDev_memset( void* s, u32 c, unsigned long count )
{
    char* xs = ( char* ) s;

    while ( count-- )
        *xs++ = c;

    return s;
}

//-------------------------------------------------------------------------------------------------
// Check INT Done
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_CheckINTDone( void )
{
    u32 retIntStatus;
    retIntStatus = MHal_EMAC_Read_ISR();
    MHalThisUVE.cntChkINTCounter = ( MHalThisUVE.cntChkINTCounter %
                                     MHal_MAX_INT_COUNTER );
    MHalThisUVE.cntChkINTCounter ++;
    if ( ( retIntStatus & EMAC_INT_DONE ) ||
         ( MHalThisUVE.cntChkINTCounter == ( MHal_MAX_INT_COUNTER - 1 ) ) )
    {
        MHalThisUVE.flagISR_INT_DONE = 0x01UL;
        return TRUE;
    }
    return FALSE;
}

extern unsigned char phyaddr;
//-------------------------------------------------------------------------------------------------
// MAC cable connection detection
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_CableConnection( void )
{
    u32 retValue = 0;
    u32 word_ETH_MAN = 0x00000000UL;
    u32 word_ETH_CTL = MHal_EMAC_Read_CTL();

    MHal_EMAC_Write_CTL( 0x00000010UL | word_ETH_CTL );
    MHalThisUVE.flagISR_INT_DONE = 0x00UL;
    MHalThisUVE.cntChkINTCounter = 0;
    MHal_EMAC_read_phy(phyaddr, MII_BMSR, &word_ETH_MAN);

    if ( word_ETH_MAN & BMSR_LSTATUS )
    {
        retValue = 1;
    }
    else
    {
        retValue = 0;
    }
    MHal_EMAC_Write_CTL( word_ETH_CTL );
    return(retValue);
}

//-------------------------------------------------------------------------------------------------
// EMAC Negotiation PHY
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_NegotiationPHY( void )
{
    // Set PHY --------------------------------------------------------------
    u32 retValue = 0;
    u32 bmsr;

    // IMPORTANT: Get real duplex by negotiation with peer.
    u32 word_ETH_CTL = MHal_EMAC_Read_CTL();
    MHal_EMAC_Write_CTL( 0x0000001CUL | word_ETH_CTL );

    MHalThisBCE.duplex = 1;   // Set default as Half-duplex if negotiation fails.
    retValue = 1;

    MHalThisUVE.flagISR_INT_DONE = 0x00UL;
    MHalThisUVE.cntChkINTCounter = 0;
    MHalThisUVE.cntChkCableConnect = 0;


    MHal_EMAC_read_phy(phyaddr, MII_BMSR, &bmsr);
    if ( (bmsr & BMSR_100FULL) || (bmsr & BMSR_10FULL) )
    {
       MHalThisBCE.duplex = 2;
       retValue = 2;
    }
    else
    {
        MHalThisBCE.duplex = 1;
        retValue = 1;
    }

    // NOTE: REG_ETH_CFG must be set according to new ThisBCE.duplex.

    MHal_EMAC_Write_CTL( word_ETH_CTL );
    // Set PHY --------------------------------------------------------------
    return(retValue);
}

//-------------------------------------------------------------------------------------------------
// EMAC Hardware register set
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// EMAC Timer set for Receive function
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_timer_callback( unsigned long value )
{
    u32 uRegVal;
    uRegVal = MHal_EMAC_Read_IER();
    uRegVal |= ( EMAC_INT_RCOM );
    MHal_EMAC_Write_IER( uRegVal );
}

void MHal_EMAC_trim_phy( void )
{
    u8 uRegVal;
    u8 uEfuVal0;
    u8 uEfuVal1;
    u8 uEfuVal2;

    //efuse read
    
    //reg28[8:2] = 0x4d
    uRegVal = MHal_EMAC_ReadReg8(0x0020UL, 0x50UL);
    uRegVal &= 0x03UL;
    uRegVal |= 0x34UL;
    MHal_EMAC_WritReg8(0x0020UL, 0x50UL, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(0x0020UL, 0x51UL);
    uRegVal &= 0xfeUL;
    uRegVal |= 0x01UL;
    MHal_EMAC_WritReg8(0x0020UL, 0x51UL, uRegVal);
    
    //reg28[13] = 0x01
    uRegVal = MHal_EMAC_ReadReg8(0x0020UL, 0x51UL);
    uRegVal |= 0x20UL;
    MHal_EMAC_WritReg8(0x0020UL, 0x51UL, uRegVal);
    
    do 
    {
        uRegVal = MHal_EMAC_ReadReg8(0x0020UL, 0x51UL);
    }while((uRegVal & 0x20UL) != 0);

    uEfuVal0 = MHal_EMAC_ReadReg8(0x0020UL, 0x58UL);
    uEfuVal1 = MHal_EMAC_ReadReg8(0x0020UL, 0x59UL);
    uEfuVal2 = MHal_EMAC_ReadReg8(0x0020UL, 0x5aUL);
    
    //write efuse into phy trim register
    uRegVal = MHal_EMAC_ReadReg8(0x0034UL, 0x60UL);
    uRegVal |= 0x04UL;
    MHal_EMAC_WritReg8(0x0034UL, 0x60UL, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(0x0034UL, 0x69UL);
    uRegVal |= 0x80UL;
    MHal_EMAC_WritReg8(0x0034UL, 0x69UL, uRegVal);
    
    MHal_EMAC_WritReg8(0x0034UL, 0x68UL, uEfuVal0);
    
    uRegVal = MHal_EMAC_ReadReg8(0x0034UL, 0x69UL);
    uRegVal &= 0xc0UL;
    uRegVal |= (uEfuVal1 & 0x3fUL);
    MHal_EMAC_WritReg8(0x0034UL, 0x69UL, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(0x0034UL, 0x60UL);
    uRegVal &= 0x7fUL;
    uRegVal |= (uEfuVal1 & 0x40UL) << 1;
    MHal_EMAC_WritReg8(0x0034UL, 0x60UL, uRegVal);

    uRegVal = MHal_EMAC_ReadReg8(0x0034UL, 0x61UL);
    uRegVal |= (uEfuVal1 >> 7);
    uRegVal |= (uEfuVal2 & 0x7fUL) << 1 ;
    MHal_EMAC_WritReg8(0x0034UL, 0x61UL, uRegVal);
}

//-------------------------------------------------------------------------------------------------
// EMAC clock on/off
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Power_On_Clk( void )
{
    u8 uRegVal;

    //Triming PHY setting via efuse value, do not need trimming now
    //MHal_EMAC_trim_phy();

    uRegVal = MHal_EMAC_ReadReg8(0x000e, 0x60);
    uRegVal &= 0xfe;
    MHal_EMAC_WritReg8(0x000e, 0x60, uRegVal);

    //swith RX discriptor format to mode 1
    MHal_EMAC_WritReg8(0x1021, 0x3a, 0x00);
    MHal_EMAC_WritReg8(0x1021, 0x3b, 0x01);

    //RX shift patch
    uRegVal = MHal_EMAC_ReadReg8(0x1021, 0x00);
    uRegVal |= 0x10;
    MHal_EMAC_WritReg8(0x1021, 0x00, uRegVal);

#ifdef CONFIG_ETHERNET_ALBANY
    //TX underrun patch
    uRegVal = MHal_EMAC_ReadReg8(0x1021, 0x39);
    uRegVal |= 0x01;
    MHal_EMAC_WritReg8(0x1021, 0x39, uRegVal);

    //det max
    MHal_EMAC_WritReg8(0x1219, 0x4f, 0x02);

    //det min
    MHal_EMAC_WritReg8(0x1219, 0x51, 0x01);

    //snr len (emc noise)
    MHal_EMAC_WritReg8(0x1219, 0x77, 0x18);

    //lpbk_enable set to 0
    MHal_EMAC_WritReg8(0x1218, 0x72, 0x80);

    MHal_EMAC_WritReg8(0x1219, 0xfc, 0x00);
    MHal_EMAC_WritReg8(0x1219, 0xfd, 0x00);
    MHal_EMAC_WritReg8(0x1219, 0xb7, 0x07);
    MHal_EMAC_WritReg8(0x1219, 0xcb, 0x11);
    MHal_EMAC_WritReg8(0x1219, 0xcc, 0x80);
    MHal_EMAC_WritReg8(0x1219, 0xcd, 0xd1);
    MHal_EMAC_WritReg8(0x1219, 0xd4, 0x00);
    MHal_EMAC_WritReg8(0x1219, 0xb9, 0x40);
    MHal_EMAC_WritReg8(0x1219, 0xbb, 0x05);
    MHal_EMAC_WritReg8(0x1219, 0xea, 0x46);
    MHal_EMAC_WritReg8(0x121a, 0xa1, 0x00);
    MHal_EMAC_WritReg8(0x121a, 0x3a, 0x03);
    MHal_EMAC_WritReg8(0x121a, 0x3b, 0x00);

    //100 gat
    MHal_EMAC_WritReg8(0x121a, 0xc5, 0x00);

    //200 gat
    MHal_EMAC_WritReg8(0x121a, 0x30, 0x43);

    //en_100t_phase
    MHal_EMAC_WritReg8(0x121a, 0x39, 0x41);

    // Prevent packet drop by inverted waveform
    MHal_EMAC_WritReg8(0x1218, 0x79, 0xd0);
    MHal_EMAC_WritReg8(0x1218, 0x77, 0x5a);

    //disable eee
    MHal_EMAC_WritReg8(0x1218, 0x2d, 0x7c);

    //10T waveform
    MHal_EMAC_WritReg8(0x121a, 0xe8, 0x06);
    MHal_EMAC_WritReg8(0x1218, 0x2b, 0x00);
    MHal_EMAC_WritReg8(0x121a, 0xe8, 0x00);
    MHal_EMAC_WritReg8(0x1218, 0x2b, 0x00);
    MHal_EMAC_WritReg8(0x121a, 0xe8, 0x06);
    MHal_EMAC_WritReg8(0x1218, 0xaa, 0x1c);
    MHal_EMAC_WritReg8(0x1218, 0xac, 0x1c);
    MHal_EMAC_WritReg8(0x1218, 0xad, 0x1c);
    MHal_EMAC_WritReg8(0x1218, 0xae, 0x1c);
    MHal_EMAC_WritReg8(0x1218, 0xaf, 0x1c);
    MHal_EMAC_WritReg8(0x121a, 0xe8, 0x00);
    MHal_EMAC_WritReg8(0x1218, 0xab, 0x28);
       
    // Set MII Mode
    MHal_EMAC_WritReg8(0x100b, 0xc0, 0x00);
    MHal_EMAC_WritReg8(0x100b, 0xc1, 0x00);
    MHal_EMAC_WritReg8(0x100b, 0xc2, 0x00);
    MHal_EMAC_WritReg8(0x100b, 0xc3, 0x00);
    MHal_EMAC_WritReg8(0x100b, 0xc4, 0x01);
    MHal_EMAC_WritReg8(0x100b, 0xc5, 0x00);
#else
    MHal_EMAC_WritReg8(0x100b, 0xc0, 0x00);
    MHal_EMAC_WritReg8(0x100b, 0xc1, 0x04);
    MHal_EMAC_WritReg8(0x100b, 0xc2, 0x04);
    MHal_EMAC_WritReg8(0x100b, 0xc3, 0x00);
    MHal_EMAC_WritReg8(0x100b, 0xc4, 0x00);
    MHal_EMAC_WritReg8(0x100b, 0xc5, 0x00);
#endif

    //chiptop [15] allpad_in
    uRegVal = MHal_EMAC_ReadReg8(0x101e, 0xa1);
    uRegVal &= 0x7f;
    MHal_EMAC_WritReg8(0x101e, 0xa1, uRegVal);

#ifdef CONFIG_ETHERNET_ALBANY
    uRegVal = MHal_EMAC_ReadReg8(0x101e, 0xdf);
    uRegVal &= 0xfe;
    MHal_EMAC_WritReg8(0x101e, 0xdf, uRegVal);
#else
    //et_mode = 1
    uRegVal = MHal_EMAC_ReadReg8(0x101e, 0xdf);
    uRegVal &= 0x01;
    MHal_EMAC_WritReg8(0x101e, 0xdf, uRegVal);
#endif
}

void MHal_EMAC_Power_Off_Clk( void )
{
}