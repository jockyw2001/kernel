///////////////////////////////////////////////////////////////////////////////////////////////////
//
// * Copyright (c) 2010 - 2012 MStar Semiconductor, Inc.
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
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// @file   mdrv_xc_io.h
// @brief  XC KMD Driver Interface
// @author MStar Semiconductor Inc.
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_XC_IO_H
#define _MDRV_XC_IO_H

//=============================================================================
// Includs
//=============================================================================
#include "mdrv_xc_st.h"
//=============================================================================
// Defines
//=============================================================================
//IO Ctrl defines:
#define MDRV_XC_IOC_SET_HDR_TYPE_NR                  (0)
#define MDRV_XC_IOC_GET_HDR_TYPE_NR                  (1)
#define MDRV_XC_IOC_SET_INPUT_SOURCE_TYPE_NR         (2)
#define MDRV_XC_IOC_GET_INPUT_SOURCE_TYPE_NR         (3)
#define MDRV_XC_IOC_SET_OPEN_METADATA_INFO_NR        (4)
#define MDRV_XC_IOC_SET_3DLUT_INFO_NR                (5)
#define MDRV_XC_IOC_SET_SHMEM_INFO_NR                (6)
#define MDRV_XC_IOC_ENABLE_HDR_NR                    (7)
#define MDRV_XC_IOC_SET_HDR_WINDOW_NR                (8)
#define MDRV_XC_IOC_CONFIG_AUTO_DOWNLOAD_NR          (9)
#define MDRV_XC_IOC_WRITE_AUTO_DOWNLOAD_NR           (10)
#define MDRV_XC_IOC_FIRE_AUTO_DOWNLOAD_NR            (11)
#define MDRV_XC_IOC_GET_AUTO_DOWNLOAD_CAPS_NR        (12)
#define MDRV_XC_IOC_SET_COLOR_FORMAT_NR              (13)
#define MDRV_XC_IOC_MLOAD_INIT_NR                    (14)
#define MDRV_XC_IOC_MLOAD_ENABLE_NR                  (15)
#define MDRV_XC_IOC_MLOAD_GETSTATUS_NR               (16)
#define MDRV_XC_IOC_MLOAD_WRT_CMD_NR                 (17)
#define MDRV_XC_IOC_MLOAD_FIRE_NR                    (18)
#define MDRV_XC_IOC_MLOAD_WRT_CMD_NONXC_NR           (19)
#define MDRV_XC_IOC_MLOAD_SET_IP_TRIG_P_NR           (20)
#define MDRV_XC_IOC_MLOAD_GET_IP_TRIG_P_NR           (21)
#define MDRV_XC_IOC_MLOAD_SET_TRIG_SYNC_NR           (22)
#define MDRV_XC_IOC_DS_GET_DSFORCE_INDEX_SUPPORT_NR  (23)
#define MDRV_XC_IOC_DS_SET_DSINDEX_SOURCE_SELECT_NR  (24)
#define MDRV_XC_IOC_DS_SET_DSFORCE_INDEX_NR          (25)
#define MDRV_XC_IOC_DS_SET_DYNAMICSCALING_NR         (26)
#define MDRV_XC_IOC_DS_SET_DYNAMICSCALING_FLAG_NR    (27)
#define MDRV_XC_IOC_DS_GET_DS_STATUS_NR              (28)
#define MDRV_XC_IOC_DS_ENABLE_IPM_TUNE_AFTER_DS_NR   (29)
#define MDRV_XC_IOC_DS_WRITE_SWDS_CMD_NR             (30)
#define MDRV_XC_IOC_DS_ADD_NULL_CMD_NR               (31)
#define MDRV_XC_IOC_DS_SET_CLIENT_NR                 (32)
#define MDRV_XC_IOC_DS_GET_SUPPORT_INDEX_NUM_NR      (33)
#define MDRV_XC_IOC_CFDCONCTRL_NR                    (34)
#define MDRV_XC_IOC_DLCCHANGECURVE_NR                (35)
#define MDRV_XC_IOC_DLCONOFFINFO_NR                  (36)
#define MDRV_XC_IOC_DLCBLECHANGECURVE_NR             (37)
#define MDRV_XC_IOC_DLCBLEONOFFINFO_NR               (38)
#define MDRV_XC_IOC_DLCGETHISTOGRAMINFO_NR           (39)
#define MDRV_XC_IOC_DLCSETDLCINITINFO_NR             (40)
#define MDRV_XC_IOC_DLCSETTMOINFO_NR                 (41)
#define MDRV_XC_IOC_DLCSETHDRINFO_NR                 (42)
#define MDRV_XC_IOC_DS_WRITE_SWDS_NONXC_CMD_NR       (43)
#define MDRV_XC_IOC_DS_GET_INDEX_NR                  (44)
#define MDRV_XC_IOC_DS_FIRE_INDEX_NR                 (45)
#define MDRV_XC_IOC_SET_DSHDRINFO_NR                 (46)
#define MDRV_XC_IOC_GET_CFD_SUPPORT_STATUS_NR        (47)
#define MDRV_XC_IOC_DS_WRITE_SWDS_CMD_MASK_NR        (48)
#define MDRV_XC_IOC_MAX_NR                           (49)

// use 'x' as magic number
#define MDRV_XC_IOC_MAGIC      ('x')
#define MDRV_XC_IOC_SET_HDR_TYPE                     _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_SET_HDR_TYPE_NR, EN_KDRV_XC_HDR_TYPE)
#define MDRV_XC_IOC_GET_HDR_TYPE                     _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_GET_HDR_TYPE_NR, EN_KDRV_XC_HDR_TYPE)
#define MDRV_XC_IOC_SET_INPUT_SOURCE_TYPE            _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_SET_INPUT_SOURCE_TYPE_NR, EN_KDRV_XC_INPUT_SOURCE_TYPE)
#define MDRV_XC_IOC_GET_INPUT_SOURCE_TYPE            _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_GET_INPUT_SOURCE_TYPE_NR, EN_KDRV_XC_INPUT_SOURCE_TYPE)
#define MDRV_XC_IOC_SET_OPEN_METADATA_INFO           _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_SET_OPEN_METADATA_INFO_NR, ST_KDRV_XC_OPEN_METADATA_INFO)
#define MDRV_XC_IOC_SET_3DLUT_INFO                   _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_SET_3DLUT_INFO_NR, ST_KDRV_XC_3DLUT_INFO)
#define MDRV_XC_IOC_SET_SHMEM_INFO                   _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_SET_SHMEM_INFO_NR, ST_KDRV_XC_SHARE_MEMORY_INFO)
#define MDRV_XC_IOC_ENABLE_HDR                       _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_ENABLE_HDR_NR, MS_BOOL)
#define MDRV_XC_IOC_SET_HDR_WINDOW                   _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_SET_HDR_WINDOW_NR, ST_KDRV_XC_WINDOW_INFO)
#define MDRV_XC_IOC_CONFIG_AUTO_DOWNLOAD             _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_CONFIG_AUTO_DOWNLOAD_NR, ST_KDRV_XC_AUTODOWNLOAD_CONFIG_INFO)
#define MDRV_XC_IOC_WRITE_AUTO_DOWNLOAD              _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_WRITE_AUTO_DOWNLOAD_NR, ST_KDRV_XC_AUTODOWNLOAD_DATA_INFO)
#define MDRV_XC_IOC_FIRE_AUTO_DOWNLOAD               _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_FIRE_AUTO_DOWNLOAD_NR, EN_KDRV_XC_AUTODOWNLOAD_CLIENT)
#define MDRV_XC_IOC_GET_AUTO_DOWNLOAD_CAPS           _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_GET_AUTO_DOWNLOAD_CAPS_NR, ST_KDRV_XC_AUTODOWNLOAD_CLIENT_SUPPORTED_CAPS)
#define MDRV_XC_IOC_SET_COLOR_FORMAT                 _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_SET_COLOR_FORMAT_NR, EN_KDRV_XC_HDR_COLOR_FORMAT)
#define MDRV_XC_IOC_MLOAD_INIT                       _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_MLOAD_INIT_NR, ST_KDRV_MLOAD_INIT)
#define MDRV_XC_IOC_MLOAD_ENABLE                     _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_MLOAD_ENABLE_NR, ST_KDRV_MLOAD_ENABLE)
#define MDRV_XC_IOC_MLOAD_GETSTATUS                  _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_MLOAD_GETSTATUS_NR, ST_KDRV_MLOAD_GETSTATUS)
#define MDRV_XC_IOC_MLOAD_WRT_CMD                    _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_MLOAD_WRT_CMD_NR, ST_KDRV_MLOAD_WRT_CMD)
#define MDRV_XC_IOC_MLOAD_FIRE                       _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_MLOAD_FIRE_NR, ST_KDRV_MLOAD_FIRE)
#define MDRV_XC_IOC_MLOAD_WRT_CMD_NONXC              _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_MLOAD_WRT_CMD_NONXC_NR, ST_KDRV_MLOAD_WRT_CMD_NONXC)
#define MDRV_XC_IOC_MLOAD_SET_IP_TRIG_P              _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_MLOAD_SET_IP_TRIG_P_NR, ST_KDRV_MLOAD_SET_IP_TRIG_P)
#define MDRV_XC_IOC_MLOAD_GET_IP_TRIG_P              _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_MLOAD_GET_IP_TRIG_P_NR, ST_KDRV_MLOAD_GET_IP_TRIG_P)
#define MDRV_XC_IOC_MLOAD_SET_TRIG_SYNC              _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_MLOAD_SET_TRIG_SYNC_NR, EN_MLOAD_TRIG_SYNC)
#define MDRV_XC_IOC_CFDCONCTRL                       _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_CFDCONCTRL_NR, ST_KDRV_XC_CFD_CONTROL_INFO)
#define MDRV_XC_IOC_DLCCHANGECURVE                   _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DLCCHANGECURVE_NR, ST_KDRV_XC_DLC_INFO)
#define MDRV_XC_IOC_DLCONOFFINFO                     _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DLCONOFFINFO_NR, MS_BOOL)
#define MDRV_XC_IOC_DLCBLECHANGECURVE                _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DLCBLECHANGECURVE_NR, ST_KDRV_XC_DLC_BLE_INFO)
#define MDRV_XC_IOC_DLCBLEONOFFINFO                  _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DLCBLEONOFFINFO_NR, MS_BOOL)
#define MDRV_XC_IOC_DLCGETHISTOGRAMINFO              _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DLCGETHISTOGRAMINFO_NR, MS_U16)
#define MDRV_XC_IOC_DLCSETDLCINITINFO                _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DLCSETDLCINITINFO_NR, ST_KDRV_XC_DLC_INIT_INFO)
#define MDRV_XC_IOC_DLCSETTMOINFO                    _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DLCSETTMOINFO_NR, ST_KDRV_XC_TMO_INFO)
#define MDRV_XC_IOC_DS_GET_DSFORCE_INDEX_SUPPORT     _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DS_GET_DSFORCE_INDEX_SUPPORT_NR, ST_KDRV_DS_GET_DSForceIndexSupported)
#define MDRV_XC_IOC_DS_SET_DSINDEX_SOURCE_SELECT     _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DS_SET_DSINDEX_SOURCE_SELECT_NR, ST_KDRV_DS_SET_DSIndexSourceSelect)
#define MDRV_XC_IOC_DS_SET_DSFORCE_INDEX             _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DS_SET_DSFORCE_INDEX_NR, ST_KDRV_DS_SET_DSForceIndex)
#define MDRV_XC_IOC_DS_SET_DYNAMICSCALING            _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DS_SET_DYNAMICSCALING_NR, ST_KDRV_DS_SetDynamicScaling)
#define MDRV_XC_IOC_DS_SET_DYNAMICSCALING_FLAG       _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DS_SET_DYNAMICSCALING_FLAG_NR, ST_KDRV_DS_SET_DynamicScalingFlag)
#define MDRV_XC_IOC_DS_GET_DS_STATUS                 _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DS_GET_DS_STATUS_NR, ST_KDRV_DS_GetDynamicScalingStatus)
#define MDRV_XC_IOC_DS_ENABLE_IPM_TUNE_AFTER_DS      _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DS_ENABLE_IPM_TUNE_AFTER_DS_NR, ST_KDRV_DS_EnableIPMTuneAfterDS)
#define MDRV_XC_IOC_DS_WRITE_SWDS_CMD                _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DS_WRITE_SWDS_CMD_NR, ST_KDRV_DS_WriteSWDSCommand)
#define MDRV_XC_IOC_DS_ADD_NULL_CMD                  _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DS_ADD_NULL_CMD_NR, ST_KDRV_DS_Add_NullCommand)
#define MDRV_XC_IOC_DS_SET_CLIENT                    _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DS_SET_CLIENT_NR, ST_KDRV_DS_SET_CLIENT)
#define MDRV_XC_IOC_DS_GET_SUPPORT_INDEX_NUM         _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DS_GET_SUPPORT_INDEX_NUM_NR, ST_KDRV_DS_GET_SUPPORT_INDEX_NUM)
#define MDRV_XC_IOC_DS_WRITE_SWDS_NONXC_CMD          _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DS_WRITE_SWDS_NONXC_CMD_NR, ST_KDRV_DS_WriteSWDSCommandNonXC)
#define MDRV_XC_IOC_DS_GET_INDEX                     _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DS_GET_INDEX_NR, ST_KDRV_DS_GET_INDEX_INFO)
#define MDRV_XC_IOC_DS_FIRE_INDEX                    _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DS_FIRE_INDEX_NR, ST_KDRV_DS_FIRE_INDEX_INFO)
#define MDRV_XC_IOC_SET_DSHDRINFO                    _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_SET_DSHDRINFO_NR, ST_KDRV_XC_DS_HDRInfo)
#define MDRV_XC_IOC_GET_CFD_SUPPORT_STATUS           _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_GET_CFD_SUPPORT_STATUS_NR, MS_BOOL)
#define MDRV_XC_IOC_DS_WRITE_SWDS_CMD_MASK           _IOWR(MDRV_XC_IOC_MAGIC, MDRV_XC_IOC_DS_WRITE_SWDS_CMD_MASK_NR, ST_KDRV_DS_WriteSWDSCommand_Mask)
#endif //_MDRV_XC_IO_H
