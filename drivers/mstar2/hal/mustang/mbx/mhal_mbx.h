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
/// @file   mhal_mbx.h
/// @brief  MStar Mailbox Driver DDI
/// @author MStar Semiconductor Inc.
/// @attention
/// <b>(OBSOLETED) <em>legacy interface is only used by MStar proprietary Mail Message communication\n
/// It's API level for backward compatible and will be remove in the next version.\n
/// Please refer @ref drvGE.h for future compatibility.</em></b>
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _HAL_MBX_H
#define _HAL_MBX_H

#ifdef _HAL_MBX_C
#define INTERFACE
#else
#define INTERFACE extern
#endif

//=============================================================================
// Includs
//=============================================================================
#include "mhal_mbx_reg.h"

//=============================================================================
// Defines & Macros
//=============================================================================
//busy bit Set/Clear/Get
#define   _BUSY_S(arg)  {\
                                        MS_U8 val; \
                                        mb();\
                                        val = REG8_MBX_GROUP(arg, REG8_MBX_STATE_1);\
                                        mb();\
                                        REG8_MBX_GROUP(arg, REG8_MBX_STATE_1) = val | MBX_STATE1_BUSY;\
                                        mb();\
                                    }

#define   _BUSY_C(arg)  {\
                                        MS_U8 val; \
                                        mb();\
                                        val = REG8_MBX_GROUP(arg, REG8_MBX_STATE_1);\
                                        mb();\
                                        REG8_MBX_GROUP(arg, REG8_MBX_STATE_1) = val & ~MBX_STATE1_BUSY;\
                                        mb();\
                                    }


#define   _BUSY(arg)    (REG8_MBX_GROUP(arg, REG8_MBX_STATE_1) & MBX_STATE1_BUSY);


//////////////////////////////////////////////////////////////
//error bit Set/Clear/Get
#define   _ERR_S(arg)   {\
                                        MS_U8 val;\
                                        mb();\
                                        val = REG8_MBX_GROUP(arg, REG8_MBX_STATE_1);\
                                        mb();\
                                        REG8_MBX_GROUP(arg, REG8_MBX_STATE_1) = val | MBX_STATE1_ERROR;\
                                        mb();\
                                    }

#define   _ERR_C(arg)    {\
                                        MS_U8 val;\
                                        mb();\
                                        val = REG8_MBX_GROUP(arg, REG8_MBX_STATE_1);\
                                        mb();\
                                        REG8_MBX_GROUP(arg, REG8_MBX_STATE_1) = val & ~MBX_STATE1_ERROR;\
                                        mb();\
                                    }


#define   _ERR(arg)    (REG8_MBX_GROUP(arg, REG8_MBX_STATE_1) & MBX_STATE1_ERROR)

//////////////////////////////////////////////////////////////
//disabled bit Set/Clear/Get
#define   _DISABLED_S(arg)   {\
                            MS_U8 val;\
                            mb();\
                            val = REG8_MBX_GROUP(arg, REG8_MBX_STATE_1);\
                            mb();\
                            REG8_MBX_GROUP(arg, REG8_MBX_STATE_1) = val | MBX_STATE1_DISABLED;\
                            mb();\
                         }

#define   _DISABLED_C(arg)   {\
                            MS_U8 val;\
                            mb();\
                            val = REG8_MBX_GROUP(arg, REG8_MBX_STATE_1);\
                            mb();\
                            REG8_MBX_GROUP(arg, REG8_MBX_STATE_1) = val & ~MBX_STATE1_DISABLED;\
                            mb();\
                         }

#define   _DISABLED(arg)    (REG8_MBX_GROUP(arg, REG8_MBX_STATE_1) & MBX_STATE1_DISABLED)

////////////////////////////////////////////////////////////////////////
//overflow bit Set/Clear/Get

#define   _OVERFLOW_S(arg)  {\
                                                MS_U8 val;\
                                                mb();\
                                                val = REG8_MBX_GROUP(arg, REG8_MBX_STATE_1);\
                                                mb();\
                                                REG8_MBX_GROUP(arg, REG8_MBX_STATE_1) = val | MBX_STATE1_OVERFLOW;\
                                                mb();\
                                             }


#define   _OVERFLOW_C(arg)   {\
                                                MS_U8 val;\
                                                mb();\
                                                val = REG8_MBX_GROUP(arg, REG8_MBX_STATE_1);\
                                                mb();\
                                                REG8_MBX_GROUP(arg, REG8_MBX_STATE_1) = val & ~MBX_STATE1_OVERFLOW;\
                                                mb();\
                                              }


#define   _OVERFLOW(arg)   (REG8_MBX_GROUP(arg, REG8_MBX_STATE_1) & MBX_STATE1_OVERFLOW)

////////////////////////////////////////////////////////////////////////
//status bit clear
#define   _S1_C(arg)   {\
                                    MS_U8 val;\
                                    mb();\
                                    val = REG8_MBX_GROUP(arg, REG8_MBX_STATE_1);\
                                    mb();\
                                    REG8_MBX_GROUP(arg, REG8_MBX_STATE_1) = val & ~(MBX_STATE1_DISABLED | MBX_STATE1_OVERFLOW | MBX_STATE1_ERROR | MBX_STATE1_BUSY);\
                                    mb();\
                                 }


////////////////////////////////////////////////////////////////////////
//fire bit Set/Clear/Get

#define   _FIRE_S(arg)   {\
                                        MS_U8 val;\
                                        mb();\
                                        val = REG8_MBX_GROUP(arg, REG8_MBX_CTRL);\
                                        mb();\
                                        REG8_MBX_GROUP(arg, REG8_MBX_CTRL) = val | MBX_CTRL_FIRE;\
                                        mb();\
                                     }


#define   _FIRE_C(arg)   {\
                                        MS_U8 val;\
                                        mb();\
                                        val = REG8_MBX_GROUP(arg, REG8_MBX_CTRL);\
                                        mb();\
                                        REG8_MBX_GROUP(arg, REG8_MBX_CTRL) = val & ~MBX_CTRL_FIRE;\
                                        mb();\
                                     }


#define   _FIRE(arg)   (REG8_MBX_GROUP(arg, REG8_MBX_CTRL) & MBX_CTRL_FIRE)

////////////////////////////////////////////////////////////////////////
//readback bit Set/Clear/Get
#define   _READBACK_S(arg)   {\
                                                MS_U8 val;\
                                                mb();\
                                                val = REG8_MBX_GROUP(arg, REG8_MBX_CTRL);\
                                                mb();\
                                                REG8_MBX_GROUP(arg, REG8_MBX_CTRL) = val | MBX_CTRL_READBACK;\
                                                mb();\
                                             }

#define   _READBACK_C(arg)   {\
                                                MS_U8 val;\
                                                mb();\
                                                val = REG8_MBX_GROUP(arg, REG8_MBX_CTRL);\
                                                mb();\
                                                REG8_MBX_GROUP(arg, REG8_MBX_CTRL) = val & ~MBX_CTRL_READBACK;\
                                                mb();\
                                             }


#define   _READBACK(arg)   (REG8_MBX_GROUP(arg, REG8_MBX_CTRL) & MBX_CTRL_READBACK)

////////////////////////////////////////////////////////////////////////
//instant bit Set/Clear/Get
#define   _INSTANT_S(arg)   {\
                                             MS_U8 val;\
                                             mb();\
                                             val = REG8_MBX_GROUP(arg, REG8_MBX_CTRL);\
                                             mb();\
                                             REG8_MBX_GROUP(arg, REG8_MBX_CTRL) = val | MBX_CTRL_INSTANT;\
                                             mb();\
                                           }


#define   _INSTANT_C(arg)   {\
                                             MS_U8 val;\
                                             mb();\
                                             val = REG8_MBX_GROUP(arg, REG8_MBX_CTRL);\
                                             mb();\
                                             REG8_MBX_GROUP(arg, REG8_MBX_CTRL) = val & ~MBX_CTRL_INSTANT;\
                                             mb();\
                                           }

#define   _INSTANT(arg)   (REG8_MBX_GROUP(arg, REG8_MBX_CTRL) & MBX_CTRL_INSTANT)

//=============================================================================
// Type and Structure Declaration
//=============================================================================

//=============================================================================
// Enums
/// MBX HAL Recv Status Define
typedef enum
{
    /// Recv Success
    E_MBXHAL_RECV_SUCCESS = 0,
    /// Recv Error: OverFlow
    E_MBXHAL_RECV_OVERFLOW = 1,
    /// Recv Error: Not Enabled
    E_MBXHAL_RECV_DISABLED = 2,
} MBXHAL_Recv_Status;

/// MBX HAL Fire Status Define
typedef enum
{
    /// Fire Success
    E_MBXHAL_FIRE_SUCCESS = 0,
    /// Still Firing
    E_MBXHAL_FIRE_ONGOING = 1,
    /// Fire Error: Overflow:
    E_MBXHAL_FIRE_OVERFLOW = 2,
    /// Fire Error: Not Enabled
    E_MBXHAL_FIRE_DISABLED = 3,
} MBXHAL_Fire_Status;

//=============================================================================
// Mailbox HAL Driver Function
//=============================================================================

INTERFACE MBX_Result MHAL_MBX_Init(MBX_ROLE_ID eHostRole);
INTERFACE MBX_Result MHAL_MBX_SetConfig(MBX_ROLE_ID eHostRole);

INTERFACE MBX_Result MHAL_MBX_SetInformation(MBX_ROLE_ID eTargetRole, MS_U8 *pU8Info, MS_U8 u8Size);
INTERFACE MBX_Result MHAL_MBX_GetInformation(MBX_ROLE_ID eTargetRole, MS_U8 *pU8Info, MS_U8 u8Size);

INTERFACE MBX_Result MHAL_MBX_Fire(MBX_Msg* pMbxMsg, MBX_ROLE_ID eSrcRole);
INTERFACE void       MHAL_MBX_ClearStatus (MBX_Msg* pMbxMsg, MBX_ROLE_ID eSrcRole);
INTERFACE MBX_Result MHAL_MBX_GetFireStatus(MBX_ROLE_ID eSrcRole, MBX_ROLE_ID eDstRole, MBXHAL_Fire_Status *pFireStatus);

INTERFACE MBX_Result MHAL_MBX_Recv(MBX_Msg* pMbxMsg, MBX_ROLE_ID eDstRole);
INTERFACE MBX_Result MHAL_MBX_RecvEnd(MBX_ROLE_ID eSrcRole, MBX_ROLE_ID eDstRole, MBXHAL_Recv_Status eRecvSatus);
INTERFACE MS_U16 MHAL_MBX_RegGroup(MBX_ROLE_ID eSrcRole, MBX_ROLE_ID eDstRole);

INTERFACE MS_U8 MHAL_PM_Get_BrickTerminator_Info(void);
INTERFACE void MHAL_PM_Set_BrickTerminator_Info(MS_U8 u8Value);

#undef INTERFACE
#endif //_HAL_MBX_H

