///////////////////////////////////////////////////////////////////////////////////////////////////
//
// * Copyright (c) 2008 - 2009 MStar Semiconductor, Inc.
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
/// file    color_format_input.c
/// @brief  MStar XC Driver DDI HAL Level
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _HAL_COLOR_FORMAT_INPUT_C
#define _HAL_COLOR_FORMAT_INPUT_C
#endif

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include "mdrv_mstypes.h"
#include "color_format_input.h"
#include "color_format_driver_hardware.h"
#include "color_format_ip.h"
#include "mhal_dlc.h"
#include "mhal_xc.h"

#define  HDR_DBG_HAL(x)  //x
#define  HDR_DBG_HAL_CFD(x)  //x
#define  HDR_DBG_TOP(x)  //x

#define  TEST_CODING_MMIN 0
#define  TEST_CODING_HDMIIN 0

//unit : 1 nits
#define  MAX_LUMINANCE_UPBOUND  10000
#define  MAX_LUMINANCE_LOWBOUND     1

//unit : 0.0001 nits
#define  MIN_LUMINANCE_UPBOUND 10000
#define  MIN_LUMINANCE_LOWBOUND    1

#define FunctionMode(Mode) ((Mode) & 0x3F)
#define function1(var1,var2) var1 = (var1&MaskForMode_HB) | (var2&MaskForMode_LB)

//=============================================================================
// Global Variables
//=============================================================================

#if 0 //RealChip
extern MS_BOOL KApi_XC_MLoad_Fire(EN_MLOAD_CLIENT_TYPE _client_type, MS_BOOL bImmeidate);
extern BOOL MHal_XC_FireAutoDownload(EN_KDRV_XC_AUTODOWNLOAD_CLIENT enClient);
#endif

#if (1 == CFD_debug)
#else
#define printf
#endif


extern MS_U16 LUT_CFD_CV1[256];
extern MS_U32 LUT_CFD_CV2[256];

extern volatile MS_U8 u8IP2_CSC_Ratio1;
extern volatile MS_U8 u8VOP_3x3_Ratio1;
extern volatile MS_U8 u8Degamma_Ratio1;
extern volatile MS_U16 u16Degamma_Ratio2;
extern volatile MS_U16 u163x3_Ratio2;
static MS_U8 u8TempVector;
static MS_U8 u8Input_HDRMode;
HDR_CONTENT_LIGHT_MATADATA STU_HDR_METADATA;

#if RealChip
#else
MS_U16 LUT_CFD_NITS2PQCODE[145] =
{
    0x0002,
    0x0002,0x0003,0x0003,0x0003,0x0004,0x0004,0x0004,0x0004,0x0005,0x0005,0x0005,0x0005,0x0005,0x0006,0x0006,0x0006,0x0006,0x0006,
    0x0008,0x0009,0x000b,0x000c,0x000d,0x000e,0x000f,0x000f,0x0010,0x0011,0x0012,0x0012,0x0013,0x0014,0x0014,0x0015,0x0015,0x0016,
    0x001b,0x001f,0x0022,0x0025,0x0028,0x002b,0x002d,0x002f,0x0031,0x0033,0x0035,0x0037,0x0038,0x003a,0x003c,0x003d,0x003e,0x0040,
    0x004b,0x0055,0x005d,0x0063,0x0069,0x006f,0x0074,0x0078,0x007c,0x0080,0x0084,0x0088,0x008b,0x008e,0x0091,0x0094,0x0097,0x0099,
    0x00b0,0x00c1,0x00ce,0x00da,0x00e5,0x00ee,0x00f6,0x00fe,0x0104,0x010b,0x0111,0x0117,0x011c,0x0121,0x0126,0x012a,0x012e,0x0133,
    0x0154,0x016d,0x0181,0x0192,0x01a0,0x01ad,0x01b8,0x01c2,0x01cc,0x01d4,0x01dc,0x01e4,0x01eb,0x01f1,0x01f7,0x01fd,0x0203,0x0208,
    0x0232,0x0250,0x0268,0x027c,0x028d,0x029c,0x02a9,0x02b4,0x02bf,0x02c8,0x02d1,0x02d9,0x02e1,0x02e8,0x02ef,0x02f5,0x02fb,0x0301,
    0x032e,0x034e,0x0367,0x037c,0x038d,0x039b,0x03a8,0x03b4,0x03be,0x03c8,0x03d1,0x03d9,0x03e0,0x03e7,0x03ee,0x03f4,0x03fa,0x03ff
};
#endif

//for HDMI EDID gamut order
//[i][j]
//SMPTE170 M = BT601_525
//BT709 = sRGB
//AdobeRGB
//BT2020

//[j]
//={supported in Sink, supportRGB, gamut index}
//[x][0] -> 0:not supported by EDID, 1:supported by EDID
//[x][1] -> 0:only RGB, 1:only YUV 2:both
//[x][2] -> colormetry supported by this HW

MS_U16 Mapi_Cfd_TMO_Param_Check(STU_CFD_MS_ALG_INTERFACE_TMO *pstu_TMO_Param)
{
    MS_U16 u16_check_status = TRUE;

    return u16_check_status;
}

//share with SW
//volatile STU_CFD_MM_PARSER g_stu_Cfd_MM_Paramters;

//STU_CFDAPI_MM_PARSER              g_stu_Cfd_MM_Paramters;
//STU_CFDAPI_HDMI_INFOFRAME_PARSER  g_stu_Cfd_HDMI_Infoframe_Parameters;
//STU_CFDAPI_HDMI_EDID_PARSER       g_stu_Cfd_HDMI_EDID_Paramters;

//STU_CFDAPI_MAIN_CONTROL           g_stu_Cfd_Main_Control_Paramters;
//STU_CFDAPI_PANEL_FORMAT           g_stu_Cfd_Panel_Paramters;

MS_U8 Mapi_Cfd_Inter_CheckLuminanceBound(MS_U16 *u16Luminance, MS_U16 u16UPBound, MS_U16 u16LowBound)
{
    MS_U8 u8_check_status = TRUE;

    if (*u16Luminance > u16UPBound)
    {
        u8_check_status = FALSE;
        *u16Luminance = u16UPBound;
    }
    else if (*u16Luminance < u16LowBound)
    {

        u8_check_status = FALSE;
        *u16Luminance = u16LowBound;
    }

    return u8_check_status;
}

void Mapi_Cfd_Main_Control_PrintResult(STU_CFDAPI_TOP_CONTROL *pstu_Cfd_top)
{
#if 0
    HDR_DBG_HAL_CFD(printk("\n  IP  Kano decision tree:    \n"));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_TMO_curve_Mode=%d    \n",  pstu_Cfd_top->pstu_HW_IP_Param->pstu_TMO_Input->stu_Kano_TMOIP_Param.u8HDR_TMO_curve_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_TMO_curve_enable_Mode=%d    \n",  pstu_Cfd_top->pstu_HW_IP_Param->pstu_TMO_Input->stu_Kano_TMOIP_Param.u8HDR_TMO_curve_enable_Mode ));

    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_IP_PATH_Mode=%d    \n",    pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_IP_PATH_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_Path_enable_Mode=%d    \n",    pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_Path_enable_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_RGB3D_Mode=%d    \n",  pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_RGB3D_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_YCGAINOFFSET_In_Mode=%d    \n",    pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_YCGAINOFFSET_In_Mode ));

    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_YCGAINOFFSET_Out_Mode=%d    \n",  pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_YCGAINOFFSET_Out_Mode ));
    //
    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_Hist_enable_Mode=%d    \n",    pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_Hist_enable_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_UVC_Mode=%d    \n",    pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_UVC_Mode ));

    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_InputCSC_Mode=%d    \n",  pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_InputCSC_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_R2RProcess_enable_Mode=%d    \n",  pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_R2RProcess_enable_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_RGBOffset_Mode=%d    \n",  pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_RGBOffset_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_RGBclipping_Mode=%d    \n",    pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_RGBclipping_Mode ));

    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_Degamma_enable_Mode=%d    \n",  pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_Degamma_enable_Mode ));
    //HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_Degamma_SRAM_Mode=%d    \n",     pstu_Cfd_top->pstu_Kano_HDRIP_Param->u8HDR_Degamma_SRAM_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_3x3_Mode=%d    \n",    pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_3x3_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_Compress_settings_Mode=%d    \n",  pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_Compress_settings_Mode ));

    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_Compress_dither_Mode=%d    \n",    pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_Compress_dither_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_3x3Clip_Mode=%d    \n",    pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_3x3Clip_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_Gamma_enable_Mode=%d    \n",  pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_Gamma_enable_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_Gamma_maxdata_Mode=%d    \n",  pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_Gamma_maxdata_Mode ));

    //HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_Gamma_SRAM_Mode=%d    \n",   pstu_Cfd_top->pstu_Kano_HDRIP_Param->u8HDR_Gamma_SRAM_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8HDR_OutCSC_Mode=%d    \n",  pstu_Cfd_top->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Kano_HDRIP_Param.u8HDR_OutCSC_Mode ));

    HDR_DBG_HAL_CFD(printk("\n  IP  u8SDR_IP2_CSC_Mode=%d    \n",    pstu_Cfd_top->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Kano_SDRIP_Param.u8SDR_IP2_CSC_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8SDR_VIP_CM_Mode=%d    \n",  pstu_Cfd_top->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Kano_SDRIP_Param.u8SDR_VIP_CM_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8SDR_Conv420_CSC_Mode=%d    \n",    pstu_Cfd_top->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Kano_SDRIP_Param.u8SDR_Conv420_CSC_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  IP  u8SDR_HDMITX_CSC_Mode=%d    \n",  pstu_Cfd_top->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Kano_SDRIP_Param.u8SDR_HDMITX_CSC_Mode ));
#endif
}

//output global
//input local


void Mapi_Cfd_inter_HDR_Metadata_Param_Init(STU_CFDAPI_HDR_METADATA_FORMAT *pstu_HDR_Param)
{
    //initialize with Kim's function
    //for Dolby
}

MS_U8 Mapi_Cfd_inter_HDR_Metadata_Param_Check(STU_CFDAPI_HDR_METADATA_FORMAT *pstu_HDR_Param)
{
    MS_U8 u8_check_status = E_CFD_MC_ERR_NOERR;
    return u8_check_status;
}

void Mapi_Cfd_inter_PANEL_Param_Init(STU_CFDAPI_PANEL_FORMAT *pstPanelParamInita)
{
    //order R->G->B
    //BT709
    //data *0.00002 0xC350 = 1

    pstPanelParamInita->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[0] = 0x7D00; //0.64
    pstPanelParamInita->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[1] = 0x3A98; //0.3
    pstPanelParamInita->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[2] = 0x1D4C; //0.15

    pstPanelParamInita->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[0] = 0x4047; //0.33
    pstPanelParamInita->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[1] = 0x7530; //0.6
    pstPanelParamInita->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[2] = 0x0BB8; //0.06

    pstPanelParamInita->stu_Cfd_Panel_ColorMetry.u16White_point_x = 0x3D13; //0.3127
    pstPanelParamInita->stu_Cfd_Panel_ColorMetry.u16White_point_y = 0x4042; //0.3290

    pstPanelParamInita->u16Panel_Med_Luminance = 50;

    pstPanelParamInita->u16Panel_Max_Luminance = 100;
    //data * 1 nits

    pstPanelParamInita->u16Panel_Min_Luminance = 500;
    //data * 0.0001 nits
}

MS_U16 Mapi_Cfd_inter_PANEL_Param_Check(STU_CFDAPI_PANEL_FORMAT *pstPanelParamInita)
{
    MS_U16 u16_check_status = E_CFD_MC_ERR_NOERR;
    MS_U8 u8_check_status2 = E_CFD_MC_ERR_NOERR;
    MS_U8 u8_check_status_Chromaticity_Coordinates;
    MS_U16 u16Temp;

    HDR_DBG_HAL_CFD(printk("\n  PANEL  u16Master_Panel_Max_Luminance=%d    \n",  pstPanelParamInita->u16Panel_Max_Luminance ));
    HDR_DBG_HAL_CFD(printk("\n  PANEL  u16Master_Panel_Min_Luminance=%d    \n",  pstPanelParamInita->u16Panel_Min_Luminance ));
    HDR_DBG_HAL_CFD(printk("\n  PANEL  u16Display_Primaries_x[0]=%d    \n",      pstPanelParamInita->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[0] ));
    HDR_DBG_HAL_CFD(printk("\n  PANEL  u16Display_Primaries_y[0]=%d    \n",      pstPanelParamInita->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[0] ));
    HDR_DBG_HAL_CFD(printk("\n  PANEL  u16Display_Primaries_x[1]=%d    \n",      pstPanelParamInita->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[1] ));
    HDR_DBG_HAL_CFD(printk("\n  PANEL  u16Display_Primaries_y[1]=%d    \n",      pstPanelParamInita->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[1] ));
    HDR_DBG_HAL_CFD(printk("\n  PANEL  u16Display_Primaries_x[2]=%d    \n",      pstPanelParamInita->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[2] ));
    HDR_DBG_HAL_CFD(printk("\n  PANEL  u16Display_Primaries_y[2]=%d    \n",      pstPanelParamInita->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[2] ));
    HDR_DBG_HAL_CFD(printk("\n  PANEL  u16White_point_x=%d    \n",                        pstPanelParamInita->stu_Cfd_Panel_ColorMetry.u16White_point_x ));
    HDR_DBG_HAL_CFD(printk("\n  PANEL  u16White_point_y=%d    \n",                        pstPanelParamInita->stu_Cfd_Panel_ColorMetry.u16White_point_y ));

#if 0
    if (pstPanelParamInita->u16Panel_Max_Luminance > 10000)
    {
        u8_check_status = FALSE;
        HDR_DBG_HAL_CFD(printk("\n  PANEL  u16Panel_Max_Luminance %d is out of range \n",    pstPanelParamInita->u16Panel_Max_Luminance ));
        pstPanelParamInita->u16Panel_Max_Luminance = 10000;
        HDR_DBG_HAL_CFD(printk("\n  force PANEL u16Panel_Max_Luminance to 1e4\n"));
    }

    if (pstPanelParamInita->u16Panel_Min_Luminance > 10000)
    {
        u8_check_status = FALSE;
        HDR_DBG_HAL_CFD(printk("\n  PANEL  u16Panel_Min_Luminance %d is out of range \n",    pstPanelParamInita->u16Panel_Min_Luminance ));
        pstPanelParamInita->u16Panel_Min_Luminance = 10000;
        HDR_DBG_HAL_CFD(printk("\n  force PANEL u16Panel_Min_Luminance to 1e4\n"));
    }
#endif

    u16Temp = (pstPanelParamInita->u16Panel_Max_Luminance);
    u8_check_status2 = Mapi_Cfd_Inter_CheckLuminanceBound(&u16Temp, MAX_LUMINANCE_UPBOUND, MAX_LUMINANCE_LOWBOUND);
    pstPanelParamInita->u16Panel_Max_Luminance = u16Temp;

    if (u8_check_status2 == FALSE)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u16Target_Max_Luminance %d is out of range \n", pstPanelParamInita->u16Panel_Max_Luminance ));
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u16Target_Max_Luminance to %d\n",pstPanelParamInita->u16Panel_Max_Luminance));
        u16_check_status = E_CFD_MC_ERR_Panel_infor;
    }

    u16Temp = (pstPanelParamInita->u16Panel_Med_Luminance);
    u8_check_status2 = Mapi_Cfd_Inter_CheckLuminanceBound(&u16Temp, MAX_LUMINANCE_UPBOUND, MAX_LUMINANCE_LOWBOUND);
    pstPanelParamInita->u16Panel_Med_Luminance = u16Temp;

    if (u8_check_status2 == FALSE)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u16Panel_Med_Luminance %d is out of range \n",   pstPanelParamInita->u16Panel_Med_Luminance ));
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u16Panel_Med_Luminance to %d\n",pstPanelParamInita->u16Panel_Med_Luminance));
        u16_check_status = E_CFD_MC_ERR_Panel_infor;
    }

    u16Temp = (pstPanelParamInita->u16Panel_Min_Luminance);
    u8_check_status2 = Mapi_Cfd_Inter_CheckLuminanceBound(&u16Temp, MAX_LUMINANCE_UPBOUND, MAX_LUMINANCE_LOWBOUND);
    pstPanelParamInita->u16Panel_Min_Luminance = u16Temp;

    if (u8_check_status2 == FALSE)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u16Target_Max_Luminance %d is out of range \n", pstPanelParamInita->u16Panel_Min_Luminance ));
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u16Target_Max_Luminance to %d\n",pstPanelParamInita->u16Panel_Min_Luminance));
        u16_check_status = E_CFD_MC_ERR_Panel_infor;
    }

    MS_U16 u16_xy_highbound = 50000;

    u8_check_status_Chromaticity_Coordinates = MS_Cfd_inter_Check_Chromaticity_Coordinates(&(pstPanelParamInita->stu_Cfd_Panel_ColorMetry),u16_xy_highbound);
    //  u8_check_status_Chromaticity_Coordinates = mscfd_HDMI_Check_Chromaticity_Coordinates(&(g_stu_Cfd_HDMI_Infoframe_Parameters.stu_Cfd_Master_Display));

    if ((u8_check_status_Chromaticity_Coordinates & 0x01) == 0x01)
    {
        u16_check_status = E_CFD_MC_ERR_Panel_infor_Mastering_Display_Coordinates;
        HDR_DBG_HAL_CFD(printk("\n  PANEL - the rule of Chromaticity_Coordinates fails, please check\n"));
    }

    if ((u8_check_status_Chromaticity_Coordinates & 0x02) == 0x02)
    {
        u16_check_status = E_CFD_MC_ERR_Panel_infor_Mastering_Display_Coordinates;
        HDR_DBG_HAL_CFD(printk("\n  PANEL - Chromaticity_Coordinates is out of range, force the value to %d\n",u16_xy_highbound));
    }

    if (E_CFD_MC_ERR_NOERR != u16_check_status)
    {
        HDR_DBG_HAL_CFD(printk("\nError:please check: some parameters in Panel structure is wrong, but has been force to some value\n"));
        HDR_DBG_HAL_CFD(printk("Error code = %04x !!!, error is in [ %s  , %d]\n",u16_check_status, __FUNCTION__,__LINE__));
    }
    u16_check_status = E_CFD_MC_ERR_NOERR;//Easter_test
    return u16_check_status;
}

void Mapi_Cfd_inter_OSD_Param_Init(STU_CFDAPI_OSD_CONTROL *pstu_OSD_Param)
{
    pstu_OSD_Param->u16Hue = 50;
    pstu_OSD_Param->u16Saturation = 128;
    pstu_OSD_Param->u16Contrast = 128;

    pstu_OSD_Param->u8OSD_UI_En = 1;
    pstu_OSD_Param->u8OSD_UI_Mode = 0;

    pstu_OSD_Param->u8HDR_UI_H2SMode = 0;
}

MS_U16 Mapi_Cfd_inter_OSD_Param_Check(STU_CFDAPI_OSD_CONTROL *pstu_OSD_Param)
{
    MS_U16 u16_check_status = E_CFD_MC_ERR_NOERR;

    if (pstu_OSD_Param->u8OSD_UI_En >= E_CFD_RESERVED_AT0x02) {
        u16_check_status = E_CFD_MC_ERR_OSD_infor;
        HDR_DBG_HAL_CFD(printk("\n  OSD  u8OSD_UI_En %d is out of range \n",        pstu_OSD_Param->u8OSD_UI_En ));
        pstu_OSD_Param->u8OSD_UI_En = E_CFD_MODE_AT0x01;
        HDR_DBG_HAL_CFD(printk("\n  OSD u8OSD_UI_En to 1\n"));
    }

    if (pstu_OSD_Param->u8OSD_UI_Mode >= E_CFD_RESERVED_AT0x02) {
        u16_check_status = E_CFD_MC_ERR_OSD_infor;
        HDR_DBG_HAL_CFD(printk("\n  OSD  u8OSD_UI_Mode %d is out of range \n",      pstu_OSD_Param->u8OSD_UI_Mode ));
        pstu_OSD_Param->u8OSD_UI_Mode = E_CFD_MODE_AT0x00;
        HDR_DBG_HAL_CFD(printk("\n  OSD u8OSD_UI_Mode to 0\n"));
    }

    if (pstu_OSD_Param->u8HDR_UI_H2SMode >= E_CFD_RESERVED_AT0x03) {
        u16_check_status = E_CFD_MC_ERR_OSD_infor;
        HDR_DBG_HAL_CFD(printk("\n  OSD  u8HDR_UI_H2SMode %d is out of range \n",      pstu_OSD_Param->u8HDR_UI_H2SMode ));
        pstu_OSD_Param->u8HDR_UI_H2SMode = E_CFD_MODE_AT0x00;
        HDR_DBG_HAL_CFD(printk("\n  OSD u8HDR_UI_H2SMode to 0\n"));
    }

    return u16_check_status;
}

void Mapi_Cfd_inter_OSD_Process_configures_init(STU_CFDAPI_OSD_PROCESS_CONFIGS *pt)
{
    //default off
    pt->u8OSD_SDR2HDR_en = 0;

    pt->u8OSD_IsFullRange = E_CFD_CFIO_RANGE_FULL;

    //default = RGB444
    pt->u8OSD_Dataformat = E_CFD_MC_FORMAT_RGB;

    //set primary of OSD/Video
    pt->u8OSD_colorprimary = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
    pt->u8Video_colorprimary = E_CFD_CFIO_CP_BT2020;

    pt->u8OSD_transferf = E_CFD_CFIO_TR_BT709;
    pt->u16OSD_MaxLumInNits = 600;

    pt->u16AntiTMO_SourceInNits = 650;

    pt->u8OSD_HDRMode = E_CFIO_MODE_SDR;
    pt->u8Video_HDRMode = E_CFIO_MODE_HDR2;

    pt->u16Video_MaxLumInNits = 4000;

    pt->u8Video_IsFullRange = E_CFD_CFIO_RANGE_LIMIT;

    //the data format of the place for OSD blending
    pt->u8Video_Dataformat = E_CFD_MC_FORMAT_YUV444;

    pt->u8Video_MatrixCoefficients = E_CFD_CFIO_MC_BT2020NCL;
}

//check user configures is ok
//return value:
//1: have wrong configures
//0: no wrong configures

MS_U8 Mapi_Cfd_inter_OSD_SDR2HDR_configures_check(STU_CFDAPI_OSD_PROCESS_CONFIGS *pt)
{
    MS_U8 check_status;

    check_status = 0;

    if (pt->u8OSD_SDR2HDR_en>=2)
    {
        pt->u8OSD_SDR2HDR_en = 0;
        check_status = 1;
    }

    if (pt->u8OSD_IsFullRange >= E_CFD_CFIO_RANGE_RESERVED_START)
    {
        pt->u8OSD_IsFullRange = E_CFD_CFIO_RANGE_FULL;
        check_status = 1;
    }

    if (pt->u8OSD_Dataformat >= E_CFD_MC_FORMAT_RESERVED_START)
    {
        pt->u8OSD_Dataformat = E_CFD_MC_FORMAT_RGB;
        check_status = 1;
    }

    if (E_CFD_CFIO_CP_BT709_SRGB_SYCC != pt->u8OSD_colorprimary)
    {
        pt->u8OSD_colorprimary = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
        check_status = 1;
    }

    //only support 709 to 2020 or 709 to 709 now
    if ((E_CFD_CFIO_CP_BT709_SRGB_SYCC != pt->u8Video_colorprimary) && (E_CFD_CFIO_CP_BT2020 != pt->u8Video_colorprimary))
    {
        pt->u8Video_colorprimary = E_CFD_CFIO_CP_BT2020;
        check_status = 1;
    }

#if 1
    if (E_CFIO_MODE_SDR != pt->u8OSD_HDRMode)
    {
        pt->u8OSD_HDRMode = E_CFIO_MODE_SDR;
        check_status = 1;
    }

    if ((E_CFIO_MODE_SDR != pt->u8Video_HDRMode) && (E_CFIO_MODE_HDR2 != pt->u8Video_HDRMode))
    {
        pt->u8Video_HDRMode = E_CFIO_MODE_SDR;
        check_status = 1;
    }
#endif

    if ((50 > pt->u16OSD_MaxLumInNits) || (1000 < pt->u16OSD_MaxLumInNits))
    {
        pt->u16OSD_MaxLumInNits = 600;
        check_status = 1;
    }

    if ((50 > pt->u16AntiTMO_SourceInNits) || (1000 < pt->u16AntiTMO_SourceInNits))
    {
        pt->u16AntiTMO_SourceInNits = 650;
        check_status = 1;
    }

    if ((50 > pt->u16Video_MaxLumInNits) || (10000 < pt->u16Video_MaxLumInNits))
    {
        pt->u16Video_MaxLumInNits = 4000;
        check_status = 1;
    }

    if (pt->u8Video_IsFullRange >= E_CFD_CFIO_RANGE_RESERVED_START)
    {
        pt->u8Video_IsFullRange = E_CFD_CFIO_RANGE_LIMIT;
        check_status = 1;
    }

    if (pt->u8Video_Dataformat >= E_CFD_MC_FORMAT_RESERVED_START)
    {
        pt->u8Video_Dataformat = E_CFD_MC_FORMAT_YUV444;
        check_status = 1;
    }

    return check_status;
}


#if 0
void Mapi_Cfd_TurnOff(STU_CFDAPI_MAIN_CONTROL *pstControlParamInit)
{
    //0: color format driver off
    //1: color format dirver on - normal mode
    //2: color format driver on - test mode
    pstControlParamInit->u8Process_Mode = 0;
}

void Mapi_Cfd_TurnOn(STU_CFDAPI_MAIN_CONTROL *pstControlParamInit)
{
    //0: color format driver off
    //1: color format dirver on - normal mode
    //2: color format driver on - test mode
    pstControlParamInit->u8Process_Mode = 1;
}
#endif




//MS_BOOL msCfd_MM_Param_Init(MS_U32 *u32MMParamInit )
MS_U16 Mapi_Cfd_HDR_Test_Ratio(MS_U8 u8Switch)
{
    MS_U16 Ratio;
    switch(u8Switch)
    {
        case 0:
            Ratio = 0x40;
            break;
        case 1:
            Ratio = 0x40;
            break;
        case 2:
            Ratio = 0x80;
            break;
        case 3:
            Ratio = 0xC0;
            break;
        case 4:
            Ratio = 0xFF;
            break;
        case 5:
            Ratio = 0x140;
            break;
        case 6:
            Ratio = 0x180;
            break;
        case 7:
            Ratio = 0x1C0;
            break;
        case 8:
            Ratio = 0x200;
            break;
        default:
            Ratio = 0x40;
            break;
    }
    return Ratio;
}

void Mapi_Cfd_Main_Control_Param_Init_test000(STU_CFDAPI_MAIN_CONTROL *pstControlParamInit,STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    MS_U16 u16Ratio = 0,u8Switch = 0;
    switch(u8TempVector)
    {
        case 0:
            //Mapi_Cfd_Main_Control_Param_Init_test000(&st_CFD_Main_Control,&stu_CfdAPI_Top_Param);
            break;
        case 1:
            Mapi_Cfd_testvector001(pstu_CfdAPI_Top_Param);
            break;
        case 2:
            Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);
            break;
        case 3:
            Mapi_Cfd_testvector003(pstu_CfdAPI_Top_Param);
            break;
        case 4:
            Mapi_Cfd_testvector004(pstu_CfdAPI_Top_Param);
            break;
        case 5:
            Mapi_Cfd_testvector005(pstu_CfdAPI_Top_Param);
            break;
        case 6:
            Mapi_Cfd_testvector006(pstu_CfdAPI_Top_Param);
            break;
        case 7:
            Mapi_Cfd_testvector007(pstu_CfdAPI_Top_Param);
            break;
        case 8:
            Mapi_Cfd_testvector008(pstu_CfdAPI_Top_Param);
            break;
        case 9:
            Mapi_Cfd_testvector009(pstu_CfdAPI_Top_Param);
            break;
        case 10:
            Mapi_Cfd_testvector010(pstu_CfdAPI_Top_Param);
            break;
        case 11:
            Mapi_Cfd_testvector011(pstu_CfdAPI_Top_Param);
            break;
        case 12:
            Mapi_Cfd_testvector012(pstu_CfdAPI_Top_Param);
            break;
        case 13:
            Mapi_Cfd_testvector013(pstu_CfdAPI_Top_Param);
            break;
        case 14:
            Mapi_Cfd_testvector014(pstu_CfdAPI_Top_Param);
            break;
        case 15:
            Mapi_Cfd_testvector015(pstu_CfdAPI_Top_Param);
            break;
        case 20:
            Mapi_Cfd_testvector020(pstu_CfdAPI_Top_Param);
            break;
        case 21:
            Mapi_Cfd_testvector021(pstu_CfdAPI_Top_Param);
            break;
        case 22:
            Mapi_Cfd_testvector022(pstu_CfdAPI_Top_Param);
            break;
        case 23:
            Mapi_Cfd_testvector023(pstu_CfdAPI_Top_Param);
            break;
        case 24:
            Mapi_Cfd_testvector024(pstu_CfdAPI_Top_Param);
            break;
        case 25:
            Mapi_Cfd_testvector025(pstu_CfdAPI_Top_Param);
            break;
        case 31: //301
            Mapi_Cfd_testvector301(pstu_CfdAPI_Top_Param);
            break;
        case 32://302
            Mapi_Cfd_testvector302(pstu_CfdAPI_Top_Param);
            break;
        case 33://302
            Mapi_Cfd_testvector303(pstu_CfdAPI_Top_Param);
            break;
        case 34://302
            Mapi_Cfd_testvector034(pstu_CfdAPI_Top_Param);
            break;
        case 35://302
            Mapi_Cfd_testvector035(pstu_CfdAPI_Top_Param);
            break;
        case 36://302
            Mapi_Cfd_testvector036(pstu_CfdAPI_Top_Param);
            break;

    }
#if RealChip//Get from deummy register
    msDlc_FunctionEnter();
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode                                       = msReadByte(REG_SC_Ali_BK30_01_L);//0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstControlParamInit->u8HW_Structure                             = ;//E_CFD_HWS_STB_TYPE1 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstControlParamInit->u8HW_PatchEn                               = ;//0x1     ;     //0: patch off, 1:patch on
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source                                = msReadByte(REG_SC_Ali_BK30_01_H);// ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx                             = msReadByte(REG_SC_Ali_BK30_02_L);   //assign by E_CFD_INPUT_ANALOG_FORMAT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                = msReadByte(REG_SC_Ali_BK30_02_H);  //assign by E_CFD_CFIO
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                            = msReadByte(REG_SC_Ali_BK30_03_L);//assign by E_CFD_MC_FORMAT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                           = msReadByte(REG_SC_Ali_BK30_03_H);//assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_OSD_Param->u16Hue = MApi_GFLIP_XC_R2BYTE(REG_SC_BK30_01_L);
    pstu_CfdAPI_Top_Param->pstu_OSD_Param->u16Saturation = MApi_GFLIP_XC_R2BYTE(REG_SC_BK30_02_L);
    pstu_CfdAPI_Top_Param->pstu_OSD_Param->u16Contrast = MApi_GFLIP_XC_R2BYTE(REG_SC_BK30_03_L);
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->u8HW_MainSub_Mode = msReadByte(REG_SC_Ali_BK30_04_L);
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                               = msReadByte(REG_SC_Ali_BK30_04_L);//assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                         = msReadByte(REG_SC_Ali_BK30_04_H);//0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source                             = msReadByte(REG_SC_Ali_BK30_05_L);//assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format                             = msReadByte(REG_SC_Ali_BK30_05_H); //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                         = msReadByte(REG_SC_Ali_BK30_06_L); //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange                        = msReadByte(REG_SC_Ali_BK30_06_H);//assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode                            = msReadByte(REG_SC_Ali_BK30_07_H);//assign by E_CFIO_HDR_STATUS
//pstControlParamInit->u8Input_Format_HDMI_CE_Flag                = ; //1: CE 0 : IT
//pstControlParamInit->u8Output_Format_HDMI_CE_Flag               = ; //1: CE 0 : IT
//pstControlParamInit->u8HDMIOutput_GammutMapping_Mode            = ;  //1: ON 0: OFF
//pstControlParamInit->u8HDMIOutput_GammutMapping_MethodMode              = ;  //0: extension 1:compression
//pstControlParamInit->u8MMInput_ColorimetryHandle_Mode       = msReadByte(REG_SC_Ali_BK30_08_L);      //1: ON 0: OFF
//pstControlParamInit->u8PanelOutput_GammutMapping_Mode           = msReadByte(REG_SC_Ali_BK30_08_H);  //1:depend on output_source infor ON 0: use default values
//pstControlParamInit->u8TMO_TargetRefer_Mode                         = msReadByte(REG_SC_Ali_BK30_09_H);   //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->u16Panel_Max_Luminance =  (msReadByte(REG_SC_Ali_BK30_0A_H)<<8)+msReadByte(REG_SC_Ali_BK30_0A_L);
//pstControlParamInit->u16Target_Min_Luminance                        = (msReadByte(REG_SC_Ali_BK30_0B_H)<<8)+msReadByte(REG_SC_Ali_BK30_0B_L);
    //msReadByte(REG_SC_Ali_BK30_0D_L)
    //msReadByte(REG_SC_Ali_BK30_0D_H)
    //msReadByte(REG_SC_Ali_BK30_0E_H)
    //msReadByte(REG_SC_Ali_BK30_0F_H)
//pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_enable_Mode = (msReadByte(REG_SC_Ali_BK30_08_L)&0x01);
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = ((msReadByte(REG_SC_Ali_BK30_08_L)>>4)&0x03);
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = ((msReadByte(REG_SC_Ali_BK30_08_L)>>6)&0x03);
//pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Manual_Vars_en = msReadByte(REG_SC_Ali_BK30_08_H);
//pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_MC = msReadByte(REG_SC_Ali_BK30_09_H);
//pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Manual_Vars_en = pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Manual_Vars_en;
//pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_MC = pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_MC;

#if NowHW == Maserati

    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_DeGamma_Manual_Vars_en = msReadByte(REG_SC_Ali_BK30_08_H);
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_TR = msReadByte(REG_SC_Ali_BK30_09_H);
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Manual_Vars_en = msReadByte(REG_SC_Ali_BK30_0B_H);
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_TR = msReadByte(REG_SC_Ali_BK30_0B_L);

#endif

    //printf("REG_SC_Ali_BK30_0F_H:%d\n",(msReadByte(REG_SC_Ali_BK30_0F_H)>>7));
    // if(1 == (msReadByte(REG_SC_Ali_BK30_0F_H)>>7)&0x1)
    // {
    //     u8Switch = msReadByte(REG_SC_Ali_BK30_0F_L);
    //     u16Ratio = Mapi_Cfd_HDR_Test_Ratio(u8Switch&0xF);
    //     pstu_Manhattan_SDRIP_Param->u8IP2_CSC_Ratio1 = u16Ratio;
    //      u16Ratio = Mapi_Cfd_HDR_Test_Ratio((u8Switch>>4)&0xF);
    //      pstu_Manhattan_SDRIP_Param->u8VOP_3x3_Ratio1 = u16Ratio;
    //      pstu_Manhattan_SDRIP_Param->u8Degamma_Ratio1 = u16Ratio;

    //      u8Switch = msReadByte(REG_SC_Ali_BK30_0F_H);
    //      u16Ratio = Mapi_Cfd_HDR_Test_Ratio(u8Switch&0xF);
    //      pstu_Manhattan_SDRIP_Param->u16Degamma_Ratio2 = u16Ratio;
    //     pstu_Manhattan_SDRIP_Param->u163x3_Ratio2 = u16Ratio;
    //  }
    //  else
//   {
//        pstu_Manhattan_SDRIP_Param->u8IP2_CSC_Ratio1 = u8IP2_CSC_Ratio1;
//       pstu_Manhattan_SDRIP_Param->u8VOP_3x3_Ratio1 = u8VOP_3x3_Ratio1;
//       pstu_Manhattan_SDRIP_Param->u8Degamma_Ratio1 = u8Degamma_Ratio1;
    //      pstu_Manhattan_SDRIP_Param->u16Degamma_Ratio2 = u16Degamma_Ratio2;
    //      pstu_Manhattan_SDRIP_Param->u163x3_Ratio2 = u163x3_Ratio2;
    // }

    //printf("u8Process_Mode                    :%d REG_SC_Ali_BK30_01_L\n",pstControlParamInit->u8Process_Mode                       );
    //printf("u8HW_Structure                    :%d REG_SC_Ali_BK30_01_H\n",pstControlParamInit->u8HW_Structure                       );
    //printf("u8HW_PatchEn                      :%d REG_SC_Ali_BK30_02_L\n",pstControlParamInit->u8HW_PatchEn                         );
    printf("u8Input_Source                      :%d REG_SC_Ali_BK30_02_H\n",pstControlParamInit->u8Input_Source                         );
    //printf("u8Input_AnalogIdx                 :%d REG_SC_Ali_BK30_03_L\n",pstControlParamInit->u8Input_AnalogIdx                    );
    printf("u8Input_Format                      :%d REG_SC_Ali_BK30_03_H\n",pstControlParamInit->u8Input_Format                         );
    printf("u8Input_DataFormat                  :%d REG_SC_Ali_BK30_04_L\n",pstControlParamInit->u8Input_DataFormat                     );
    printf("u8Input_IsFullRange                 :%d REG_SC_Ali_BK30_04_H\n",pstControlParamInit->u8Input_IsFullRange                    );
    printf("u8Input_HDRMode                     :%d REG_SC_Ali_BK30_05_L\n",pstControlParamInit->u8Input_HDRMode                        );
    printf("u8Input_IsRGBBypass                 :%d REG_SC_Ali_BK30_05_H\n",pstControlParamInit->u8Input_IsRGBBypass                    );
    printf("u8Output_Source                     :%d REG_SC_Ali_BK30_06_L\n",pstControlParamInit->u8Output_Source                        );
    printf("u8Output_Format                     :%d REG_SC_Ali_BK30_06_H\n",pstControlParamInit->u8Output_Format                        );
    printf("u8Output_DataFormat                 :%d REG_SC_Ali_BK30_07_H\n",pstControlParamInit->u8Output_DataFormat                    );
    printf("u8Output_IsFullRange                :%d REG_SC_Ali_BK30_08_L\n",pstControlParamInit->u8Output_IsFullRange                   );
    printf("u8Output_HDRMode                    :%d REG_SC_Ali_BK30_08_H\n",pstControlParamInit->u8Output_HDRMode                       );
#if NowHW == Maserati
    printf("pstu_Maserati_HDRIP_Param->u8HDR_enable_Mode            :%d REG_SC_Ali_BK30_08_L Bit[0]  \n",pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_enable_Mode             );
#endif
    printf("pstu_Main_Control->u8Input_SDRIPMode                    :%d REG_SC_Ali_BK30_08_L Bit[5:4]\n",pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode                     );
    printf("pstu_Main_Control->u8Input_HDRIPMode                    :%d REG_SC_Ali_BK30_08_L Bit[7:6]\n",pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode                     );
#if NowHW == Maserati
    //printf("pstu_Maserati_HDRIP_Param->u8HDR_InputCSC_Manual_Vars_en:%d REG_SC_Ali_BK30_08_H\n",pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Manual_Vars_en );
    //printf("pstu_Maserati_HDRIP_Param->u8HDR_InputCSC_MC            :%d REG_SC_Ali_BK30_09_H\n",pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_MC             );
    printf("pstu_Maserati_HDRIP_Param->u8HDR_DeGamma_Manual_Vars_en :%d REG_SC_Ali_BK30_08_H\n",pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_DeGamma_Manual_Vars_en  );
    printf("pstu_Maserati_HDRIP_Param->u8HDR_Degamma_TR             :%d REG_SC_Ali_BK30_09_H\n",pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_TR              );
    printf("pstu_Maserati_HDRIP_Param->u8HDR_Gamma_Manual_Vars_en   :%d REG_SC_Ali_BK30_0B_H\n",pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Manual_Vars_en    );
    printf("pstu_Maserati_HDRIP_Param->u8HDR_Gamma_TR               :%d REG_SC_Ali_BK30_0B_L\n",pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_TR                );
#endif
    printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance :%d REG_SC_Ali_BK30_0A_H\n ",pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance);
    msDlc_FunctionExit();

#else//Get input from Dummy register for Manhattan
    //pstControlParamInit->u8Process_Mode                   = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
    //pstControlParamInit->u8HW_Structure                   = E_CFD_HWS_STB_TYPE1 ;     //assign by E_CFD_MC_HW_STRUCTURE
    //pstControlParamInit->u8HW_PatchEn                     = 0x1     ;     //0: patch off, 1:patch on
    pstControlParamInit->u8Input_Source                 = E_CFD_INPUT_SOURCE_STORAGE ;     //assign by E_CFD_MC_SOURCE
    //pstControlParamInit->u8Input_AnalogIdx                = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstControlParamInit->u8Input_Format                 = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstControlParamInit->u8Input_DataFormat             = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstControlParamInit->u8Input_IsFullRange            = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstControlParamInit->u8Input_HDRMode                = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstControlParamInit->u8Input_IsRGBBypass            = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstControlParamInit->u8Output_Source                = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
    pstControlParamInit->u8Output_Format                = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstControlParamInit->u8Output_DataFormat            = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstControlParamInit->u8Output_IsFullRange           = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstControlParamInit->u8Output_HDRMode               = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    //pstControlParamInit->u8Input_Format_HDMI_CE_Flag      = 0x0     ;    //1: CE 0 : IT
    //pstControlParamInit->u8Output_Format_HDMI_CE_Flag     = 0x1     ;    //1: CE 0 : IT
    pstControlParamInit->u8HDMIOutput_GammutMapping_Mode    = 0x1     ;     //1: ON 0: OFF
    pstControlParamInit->u8HDMIOutput_GammutMapping_MethodMode  = 0x0     ;     //0: extension 1:compression
    pstControlParamInit->u8MMInput_ColorimetryHandle_Mode   = 0x1     ;      //1: ON 0: OFF
    pstControlParamInit->u8TMO_TargetRefer_Mode         = 0x1     ;      //1:depend on output_source infor ON 0: use default values
    pstControlParamInit->u16Target_Max_Luminance        = 100     ;      //for TMO, unit : 1 nits
    pstControlParamInit->u16Target_Min_Luminance        = 500     ;      //for TMO, unit : 0.0001 nits
#endif
}

void Mapi_Cfd_HWIPS_Default(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{

#if NowHW == Maserati
//[STU_CFDAPI_Maserati_TMOIP]
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_enable_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_setting_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_UVC_setting_Mode  = 0xC0    ;      //default = 0xc0
//[STU_CFDAPI_Maserati_HDRIP]
    //pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Composer_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Module1_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Mode = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Ratio1   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Manual_Vars_en   = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_MC   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_SRAM_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_DeGamma_Manual_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_TR    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_Lut_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.pu32HDR_Degamma_Lut_Address = NULL    ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Lut_Length   = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8DHDR_Degamma_Max_Lum_En   = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Max_Lum  = 0xFF00  ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_3x3_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_Manual_Vars_en    = 0x0     ;      //0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_InputCP   = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_OutputCP  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_SRAM_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Manual_Vars_en  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_TR  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Lut_En  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.pu32HDR_Gamma_Lut_Address   = NULL    ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Gamma_Lut_Length = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Mode    = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Ratio1  = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Manual_Vars_en  = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_MC  = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Yoffset_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_NLM_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_NLM_setting_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACGain_enable_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACGain_setting_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACE_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACE_setting_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither1_setting_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_enable_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_SRAM_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_setting_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_444to422_enable_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither2_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither2_setting_Mode  = 0xC0    ;      //default = 0xc0

//[STU_CFDAPI_Maserati_SDRIP]
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_Mode  = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_Manual_Vars_en    = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_MC    = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_CSC_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PreYoffset_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PreYgain_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PreYgain_dither_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PostYoffset_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PostYgain_Mode    = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PostYoffset2_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_Mode  = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_Manual_Vars_en    = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_MC    = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3RGBClip_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8LinearRGBBypass_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_enable_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Dither_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_SRAM_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Degamma_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8DeGamma_Manual_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_TR    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Lut_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.pu32Degamma_Lut_Address = NULL    ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Degamma_Lut_Length   = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Max_Lum_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Degamma_Max_Lum  = 0xFF00  ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_enable_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u163x3_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_Manual_Vars_en    = 0x0     ;      //0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_InputCP   = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_OutputCP  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Compress_settings_Mode    = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Compress_dither_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3Clip_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_enable_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_Dither_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_maxdata_Mode    = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_SRAM_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_Mode_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_TR  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_Lut_En  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.pu32Gamma_Lut_Address   = NULL    ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Gamma_Lut_Length = 0x0     ;      //

//[STU_CFDAPI_Maserati_DLCIP]
    //pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_DLC_Input->stu_Maserati_DLC_Param.u8DLC_curve_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_DLC_Input->stu_Maserati_DLC_Param.u8DLC_curve_enable_Mode = 0xC0    ;      //default = 0xc0
//pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_DLC_Input->stu_Maserati_DLC_Param.u8UVC_enable_Mode = 0xC0    ;      //default = 0xc0
//pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_DLC_Input->stu_Maserati_DLC_Param.u8UVC_setting_Mode    = 0xC0    ;      //default = 0xc0
#endif

//[STU_CFDAPI_Curry_TMOIP]
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Curry_TMOIP_Param.u8HDR_TMO_curve_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Curry_TMOIP_Param.u8HDR_TMO_curve_enable_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Curry_TMOIP_Param.u8HDR_TMO_curve_setting_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Curry_TMOIP_Param.u8HDR_UVC_setting_Mode  = 0xC0    ;      //default = 0xc0
//[STU_CFDAPI_Curry_HDRIP]
    //pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_Composer_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_Module1_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_InputCSC_Mode = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_InputCSC_Ratio1   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_InputCSC_Manual_Vars_en   = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_InputCSC_MC   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_Degamma_SRAM_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_Degamma_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u16HDR_Degamma_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_DeGamma_Manual_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_Degamma_TR    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_Degamma_Lut_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.pu32HDR_Degamma_Lut_Address = NULL    ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u16HDR_Degamma_Lut_Length   = 0x200     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8DHDR_Degamma_Max_Lum_En   = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u16HDR_Degamma_Max_Lum  = 0xFF00  ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_3x3_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u16HDR_3x3_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_3x3_Manual_Vars_en    = 0x0     ;      //0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_3x3_InputCP   = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_3x3_OutputCP  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_Gamma_SRAM_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_Gamma_Manual_Vars_en  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_Gamma_TR  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_Gamma_Lut_En  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.pu32HDR_Gamma_Lut_Address   = NULL    ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u16HDR_Gamma_Lut_Length = 0x200     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_OutputCSC_Mode    = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_OutputCSC_Ratio1  = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_OutputCSC_Manual_Vars_en  = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_OutputCSC_MC  = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_Yoffset_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_NLM_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_NLM_setting_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_ACGain_enable_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_ACGain_setting_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_ACE_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_ACE_setting_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_Dither1_setting_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_3DLUT_enable_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_3DLUT_SRAM_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_3DLUT_setting_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_444to422_enable_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_Dither2_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Curry_HDRIP_Param.u8HDR_Dither2_setting_Mode  = 0xC0    ;      //default = 0xc0

    //pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_Mode
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Curry_SDRIP_Param.u8SDR_IP2_CSC_Mode = 0x00;
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Curry_SDRIP_Param.u8SDR_IP2_CSC_Ratio1 = 0x40;
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Curry_SDRIP_Param.u8SDR_IP2_CSC_Manual_Vars_en = 0;
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Curry_SDRIP_Param.u8SDR_IP2_CSC_MC = E_CFD_CFIO_MC_BT709_XVYCC709;

    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Curry_SDRIP_Param.u8SDR_VIP_CM_Mode = 0x00;
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Curry_SDRIP_Param.u8SDR_VIP_CM_Ratio1 = 0x40;
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Curry_SDRIP_Param.u8SDR_VIP_CM_Manual_Vars_en = 0;
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Curry_SDRIP_Param.u8SDR_VIP_CM_MC = E_CFD_CFIO_MC_BT709_XVYCC709;

    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Curry_SDRIP_Param.u8SDR_Conv420_CM_Mode = 0x00;
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Curry_SDRIP_Param.u8SDR_Conv420_CM_Ratio1 = 0x40;
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Curry_SDRIP_Param.u8SDR_Conv420_CM_Manual_Vars_en = 0;
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Curry_SDRIP_Param.u8SDR_Conv420_CM_MC = E_CFD_CFIO_MC_BT709_XVYCC709;

#if (Curry_Control_HDMITX_CSC == 1)
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Curry_SDRIP_Param.u8SDR_HDMITX_CSC_Mode = 0xC0;
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Curry_SDRIP_Param.u8SDR_HDMITX_CSC_Ratio1 = 0x40;
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Curry_SDRIP_Param.u8SDR_HDMITX_CSC_Manual_Vars_en = 0;
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Curry_SDRIP_Param.u8SDR_HDMITX_CSC_MC = E_CFD_CFIO_MC_BT709_XVYCC709;
#endif
}
void Mapi_Cfd_testvector_base(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format_Mode = 0;
}

void Mapi_Cfd_testvector001(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//[Global]
    Mapi_Cfd_testvector_base(pstu_CfdAPI_Top_Param);
//[Color_format_driver_structure]

//[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

//[STU_CFDAPI_MM_PARSER]
//pstu_CfdAPI_Top_Param->pstu_MM_Param->u8MM_Codec      = E_CFD_MM_CODEC_HEVC ;      //E_CFD_MM_CODEC_OTHERS
//pstu_CfdAPI_Top_Param->pstu_MM_Param->u8ColorDescription_Valid    = E_CFD_VALID ;      //E_CFD_VALIDORNOT
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Max_Luminance = 40000000 ;      //data = code * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Min_Luminance = 500     ;      //data = code * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[0]   = 0x7D00  ;      //data = code*0.00002 0xC350 = 1
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[1]   = 0x3A98  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[2]   = 0x1D4C  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[0]   = 0x4047  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[1]   = 0x7530  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[2]   = 0x0BB8  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_x    = 0x3D13  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_y    = 0x4042  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Mastering_Display_Infor_Valid   = E_CFD_NOT_VALID ;      //E_CFD_VALIDORNOT

//[STU_CFDAPI_HDMI_INFOFRAME_PARSER]
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_HDR_InfoFrame_Valid  = E_CFD_NOT_VALID ;      //E_CFD_VALIDORNOT
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_EOTF = E_CFD_HDMI_EOTF_SMPTE2084 ;      //E_CFD_HDMI_HDR_INFOFRAME_EOTF
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_SMD_ID   = E_CFD_HDMI_META_TYPE1 ;      //E_CFD_HDMI_HDR_INFOFRAME_METADATA
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Max_Luminance = 4000    ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Min_Luminance = 500     ;      //data * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Content_Light_Level    = 600     ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Frame_Avg_Light_Level  = 128     ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[0]   = 0x7D00  ;      //data *0.00002 0xC350 = 1
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[1]   = 0x3A98  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[2]   = 0x1D4C  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[0]   = 0x4047  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[1]   = 0x7530  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[2]   = 0x0BB8  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_x    = 0x3D13  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_y    = 0x4042  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8Mastering_Display_Infor_Valid   = E_CFD_NOT_VALID ;      //E_CFD_VALIDORNOT
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_Support_Format   = 0x0     ;      //{Q1 Q0 YQ1 YQ0 Y2 Y1 Y0}
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_Colorspace   = E_CFD_CFIO_YUV_BT709 ;      //E_CFD_CFIO
//pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_AVIInfoFrame_Valid = E_CFD_VALID ;      //E_CFD_VALIDORNOT

//[STU_CFDAPI_PANEL_FORMAT]
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[0] = 0x7D00  ;      //data *0.00002 0xC350 = 1
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[1] = 0x3A98  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[2] = 0x1D4C  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[0] = 0x4047  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[1] = 0x7530  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[2] = 0x0BB8  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16White_point_x  = 0x3D13  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16White_point_y  = 0x4042  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->u16Panel_Max_Luminance = 150     ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->u16Panel_Min_Luminance = 500     ;      //data * 0.0001 nits

//[STU_CFDAPI_Maserati_TMOIP]
#if NowHW == Maserati
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_enable_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_setting_Mode    = 0xC0    ;      //default = 0xc0

//[STU_CFDAPI_Maserati_SDRIP]
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_Mode  = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_Manual_Vars_en    = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_MC    = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_CSC_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PreYoffset_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PreYgain_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PreYgain_dither_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PostYoffset_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PostYgain_Mode    = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PostYoffset2_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_Mode  = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_Manual_Vars_en    = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_MC    = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3RGBClip_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8LinearRGBBypass_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_enable_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Dither_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_SRAM_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Degamma_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8DeGamma_Manual_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_TR    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Lut_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.pu32Degamma_Lut_Address = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Degamma_Lut_Length   = 0x100     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Max_Lum_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Degamma_Max_Lum  = 0xFF00  ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_enable_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u163x3_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_Manual_Vars_en    = 0x0     ;      //0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_InputCP   = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_OutputCP  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Compress_settings_Mode    = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Compress_dither_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3Clip_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_enable_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_Dither_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_maxdata_Mode    = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_SRAM_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_Mode_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_TR  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_Lut_En  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.pu32Gamma_Lut_Address   = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Gamma_Lut_Length = 0x100     ;      //

//[STU_CFDAPI_Maserati_DLCIP]
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_DLC_Input->stu_Maserati_DLC_Param.u8DLC_curve_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_DLC_Input->stu_Maserati_DLC_Param.u8DLC_curve_enable_Mode = 0xC0    ;      //default = 0xc0
//pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_DLC_Input->stu_Maserati_DLC_Param.u8UVC_enable_Mode = 0xC0    ;      //default = 0xc0
//pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_DLC_Input->stu_Maserati_DLC_Param.u8UVC_setting_Mode    = 0xC0    ;      //default = 0xc0

//[STU_CFDAPI_Maserati_TMOIP]
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_enable_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_setting_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_UVC_setting_Mode  = 0xC0    ;      //default = 0xc0

//[STU_CFDAPI_Maserati_HDRIP]
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Composer_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Module1_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Mode = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Ratio1   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Manual_Vars_en   = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_MC   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_SRAM_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_DeGamma_Manual_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_TR    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_Lut_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.pu32HDR_Degamma_Lut_Address = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Lut_Length   = 0x200     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8DHDR_Degamma_Max_Lum_En   = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Max_Lum  = 0xFF00  ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_3x3_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_Manual_Vars_en    = 0x0     ;      //0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_InputCP   = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_OutputCP  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_SRAM_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Manual_Vars_en  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_TR  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Lut_En  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.pu32HDR_Gamma_Lut_Address   = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Gamma_Lut_Length = 0x200     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Mode    = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Ratio1  = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Manual_Vars_en  = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_MC  = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Yoffset_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_NLM_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_NLM_setting_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACGain_enable_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACGain_setting_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACE_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACE_setting_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither1_setting_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_enable_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_SRAM_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_setting_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_444to422_enable_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither2_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither2_setting_Mode  = 0xC0    ;      //default = 0xc0
#endif
}

void Mapi_Cfd_testvector002(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{


//[Global]

//[Color_format_driver_structure]

//[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

//[STU_CFDAPI_MM_PARSER]
//pstu_CfdAPI_Top_Param->pstu_MM_Param->u8MM_Codec      = E_CFD_MM_CODEC_HEVC ;      //E_CFD_MM_CODEC_OTHERS
//pstu_CfdAPI_Top_Param->pstu_MM_Param->u8ColorDescription_Valid    = E_CFD_VALID ;      //E_CFD_VALIDORNOT
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Max_Luminance = 40000000 ;      //data = code * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Min_Luminance = 500     ;      //data = code * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[0]   = 0x7D00  ;      //data = code*0.00002 0xC350 = 1
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[1]   = 0x3A98  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[2]   = 0x1D4C  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[0]   = 0x4047  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[1]   = 0x7530  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[2]   = 0x0BB8  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_x    = 0x3D13  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_y    = 0x4042  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Mastering_Display_Infor_Valid   = E_CFD_NOT_VALID ;      //E_CFD_VALIDORNOT

//[STU_CFDAPI_HDMI_INFOFRAME_PARSER]
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_HDR_InfoFrame_Valid  = E_CFD_NOT_VALID ;      //E_CFD_VALIDORNOT
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_EOTF = E_CFD_HDMI_EOTF_SMPTE2084 ;      //E_CFD_HDMI_HDR_INFOFRAME_EOTF
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_SMD_ID   = E_CFD_HDMI_META_TYPE1 ;      //E_CFD_HDMI_HDR_INFOFRAME_METADATA
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Max_Luminance = 4000    ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Min_Luminance = 500     ;      //data * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Content_Light_Level    = 600     ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Frame_Avg_Light_Level  = 128     ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[0]   = 0x7D00  ;      //data *0.00002 0xC350 = 1
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[1]   = 0x3A98  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[2]   = 0x1D4C  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[0]   = 0x4047  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[1]   = 0x7530  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[2]   = 0x0BB8  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_x    = 0x3D13  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_y    = 0x4042  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8Mastering_Display_Infor_Valid   = E_CFD_NOT_VALID ;      //E_CFD_VALIDORNOT
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_Support_Format   = 0x0     ;      //{Q1 Q0 YQ1 YQ0 Y2 Y1 Y0}
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_Colorspace   = E_CFD_CFIO_YUV_BT709 ;      //E_CFD_CFIO
//pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_AVIInfoFrame_Valid = E_CFD_VALID ;      //E_CFD_VALIDORNOT

//[STU_CFDAPI_PANEL_FORMAT]
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[0] = 0x7D00  ;      //data *0.00002 0xC350 = 1
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[1] = 0x3A98  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[2] = 0x1D4C  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[0] = 0x4047  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[1] = 0x7530  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[2] = 0x0BB8  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16White_point_x  = 0x3D13  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16White_point_y  = 0x4042  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->u16Panel_Max_Luminance = 150     ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->u16Panel_Min_Luminance = 500     ;      //data * 0.0001 nits

//[STU_CFDAPI_Maserati_TMOIP]
#if NowHW == Maserati
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_enable_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_setting_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_UVC_setting_Mode  = 0xC0    ;      //default = 0xc0
//[STU_CFDAPI_Maserati_HDRIP]
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Composer_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Module1_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Mode = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Ratio1   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Manual_Vars_en   = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_MC   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_SRAM_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_DeGamma_Manual_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_TR    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_Lut_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.pu32HDR_Degamma_Lut_Address = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Lut_Length   = 0x200     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8DHDR_Degamma_Max_Lum_En   = 0x0     ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Max_Lum  = 0xff00  ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_3x3_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_Manual_Vars_en    = 0x0     ;      //0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_InputCP   = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_OutputCP  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_SRAM_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Manual_Vars_en  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_TR  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Lut_En  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.pu32HDR_Gamma_Lut_Address   = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Gamma_Lut_Length = 0x200     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Mode    = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Ratio1  = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Manual_Vars_en  = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_MC  = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Yoffset_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_NLM_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_NLM_setting_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACGain_enable_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACGain_setting_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACE_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACE_setting_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither1_setting_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_enable_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_SRAM_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_setting_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_444to422_enable_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither2_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither2_setting_Mode  = 0xC0    ;      //default = 0xc0

//[STU_CFDAPI_Maserati_SDRIP]
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_Mode  = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_Manual_Vars_en    = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_MC    = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_CSC_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PreYoffset_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PreYgain_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PreYgain_dither_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PostYoffset_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PostYgain_Mode    = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PostYoffset2_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_Mode  = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_Manual_Vars_en    = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_MC    = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3RGBClip_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8LinearRGBBypass_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_enable_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Dither_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_SRAM_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Degamma_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8DeGamma_Manual_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_TR    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Lut_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.pu32Degamma_Lut_Address = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Degamma_Lut_Length   = 0x100     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Max_Lum_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Degamma_Max_Lum  = 0xFF00  ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_enable_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u163x3_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_Manual_Vars_en    = 0x0     ;      //0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_InputCP   = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_OutputCP  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Compress_settings_Mode    = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Compress_dither_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3Clip_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_enable_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_Dither_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_maxdata_Mode    = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_SRAM_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_Mode_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_TR  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_Lut_En  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.pu32Gamma_Lut_Address   = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Gamma_Lut_Length = 0x100     ;      //

//[STU_CFDAPI_Maserati_DLCIP]
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_DLC_Input->stu_Maserati_DLC_Param.u8DLC_curve_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_DLC_Input->stu_Maserati_DLC_Param.u8DLC_curve_enable_Mode = 0xC0    ;      //default = 0xc0
//pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_DLC_Input->stu_Maserati_DLC_Param.u8UVC_enable_Mode = 0xC0    ;      //default = 0xc0
//pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_DLC_Input->stu_Maserati_DLC_Param.u8UVC_setting_Mode    = 0xC0    ;      //default = 0xc0
#endif

}

void Mapi_Cfd_testvector003(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);
    //[STU_CFDAPI_MAIN_CONTROL]

//[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_ADOBE_RGB ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector004(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_ADOBE_RGB ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}

void Mapi_Cfd_testvector005(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_RGB_BT2020 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}

void Mapi_Cfd_testvector006(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_RGB_BT2020 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}

void Mapi_Cfd_testvector007(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}

void Mapi_Cfd_testvector008(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x2     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}

void Mapi_Cfd_testvector009(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}

void Mapi_Cfd_testvector010(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

//[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}

//
void Mapi_Cfd_testvector011(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}

void Mapi_Cfd_testvector012(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_ADOBE_YCC601 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}

void Mapi_Cfd_testvector013(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_XVYCC_601 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}

void Mapi_Cfd_testvector014(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_XVYCC_709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}

void Mapi_Cfd_testvector015(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_SYCC601 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}

void Mapi_Cfd_testvector016(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_CL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector020(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}

void Mapi_Cfd_testvector021(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}

void Mapi_Cfd_testvector022(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}

void Mapi_Cfd_testvector023(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}

void Mapi_Cfd_testvector024(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}


void Mapi_Cfd_testvector025(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector022(pstu_CfdAPI_Top_Param);

//[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

//[STU_CFDAPI_Maserati_SDRIP]
#if NowHW == Maserati
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_Mode  = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_Manual_Vars_en    = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_MC    = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_CSC_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PreYoffset_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PreYgain_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PreYgain_dither_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PostYoffset_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PostYgain_Mode    = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PostYoffset2_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_Mode  = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_Ratio1    = 0x80    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_Manual_Vars_en    = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_MC    = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3RGBClip_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8LinearRGBBypass_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_enable_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Dither_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_SRAM_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Ratio1    = 0x80    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Degamma_Ratio2   = 0x200   ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8DeGamma_Manual_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_TR    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Lut_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.pu32Degamma_Lut_Address = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Degamma_Lut_Length   = 0x100     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Max_Lum_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Degamma_Max_Lum  = 0xFF00  ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_enable_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u163x3_Ratio2   = 0x80    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_Manual_Vars_en    = 0x0     ;      //0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_InputCP   = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_OutputCP  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Compress_settings_Mode    = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Compress_dither_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3Clip_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_enable_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_Dither_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_maxdata_Mode    = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_SRAM_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_Mode_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_TR  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_Lut_En  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.pu32Gamma_Lut_Address   = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Gamma_Lut_Length = 0x100    ;      //
#endif
}

void Mapi_Cfd_testvector026(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

//[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x0     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector028(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

//[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x2     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector029(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

//[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x2     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector030(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

//[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x2     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector031(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x2     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x2     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
}

void Mapi_Cfd_testvector032(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x2     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x3     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector033(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector034(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector035(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x1     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status


//pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Manual_Vars_en    = 0x1     ;      // force gamma 2.2
//pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_TR    = 0x4 ;      //E_CFD_CFIO_TR
}

void Mapi_Cfd_testvector036(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status


//pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Manual_Vars_en    = 0x1     ;      // force gamma 2.2
//pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_TR    = 0x4 ;      //E_CFD_CFIO_TR

}


void Mapi_Cfd_testvector037(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x2     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector038(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x2     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector039(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x2     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector040(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x3     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector041(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector042(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector043(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector044(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector045(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x2     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector046(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x2     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector047(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector048(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_ADOBE_RGB ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector049(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_ADOBE_RGB ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}


void Mapi_Cfd_testvector050(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x0     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector051(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x0     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector052(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_ADOBE_RGB ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x0     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector053(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_ADOBE_RGB ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x0     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}


void Mapi_Cfd_testvector054(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_ADOBE_RGB ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x2     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector055(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_DOLBY_HDR_TEMP ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR1 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x0     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x3     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector056(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_DOLBY_HDR_TEMP ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR1 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x0     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x3     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector061(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    //Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);
    //[STU_CFDAPI_MAIN_CONTROL]
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Colour_primaries  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Transfer_Characteristics  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC

//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Max_Luminance = 40000000 ;      //data = code * 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Min_Luminance = 500     ;      //data = code * 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[0]   = 0x7D00  ;      //data = code*0.00002 0xC350 = 1
pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[1]   = 0x3A98  ;      //
pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[2]   = 0x1D4C  ;      //
pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[0]   = 0x4047  ;      //
pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[1]   = 0x7530  ;      //
pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[2]   = 0x0BB8  ;      //
pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_x    = 0x3D13  ;      //
pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_y    = 0x4042  ;      //
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Mastering_Display_Infor_Valid   = E_CFD_NOT_VALID ;      //E_CFD_VALIDORNOT

//[STU_CFDAPI_HDMI_INFOFRAME_PARSER]
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_HDR_InfoFrame_Valid  = E_CFD_NOT_VALID ;      //E_CFD_VALIDORNOT
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_EOTF = E_CFD_HDMI_EOTF_SMPTE2084 ;      //E_CFD_HDMI_HDR_INFOFRAME_EOTF
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_SMD_ID   = E_CFD_HDMI_META_TYPE1 ;      //E_CFD_HDMI_HDR_INFOFRAME_METADATA
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Max_Luminance = 4000    ;      //data * 1 nits
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Min_Luminance = 500     ;      //data * 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Content_Light_Level    = 600     ;      //data * 1 nits
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Frame_Avg_Light_Level  = 128     ;      //data * 1 nits
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[0]   = 0x7D00  ;      //data *0.00002 0xC350 = 1
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[1]   = 0x3A98  ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[2]   = 0x1D4C  ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[0]   = 0x4047  ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[1]   = 0x7530  ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[2]   = 0x0BB8  ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_x    = 0x3D13  ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_y    = 0x4042  ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8Mastering_Display_Infor_Valid   = E_CFD_NOT_VALID ;      //E_CFD_VALIDORNOT
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_Support_Format   = 0x0     ;      //{Q1 Q0 YQ1 YQ0 Y2 Y1 Y0}
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_Colorspace   = E_CFD_CFIO_YUV_BT709 ;      //E_CFD_CFIO

//[STU_CFDAPI_HDMI_EDID_PARSER]
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid = 0x1     ;       //0 :Not valid, 1 :valid
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF    = 0x1     ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_SM  = 0x1     ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance   = 116     ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance   = 44      ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = 23      ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length    = 6       ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[0]    = 0x28f   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[1]    = 0x133   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[2]    = 0x9A    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[0]    = 0x152   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[1]    = 0x266   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[2]    = 0x3D    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16White_point_x = 0x140   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16White_point_y = 0x151   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_base_block_version = 0x01    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_base_block_reversion   = 0x03    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_CEA_block_reversion    = 0x03    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_VCDB_Valid  = 0x01    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Support_YUVFormat   = 0x1f    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xff    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid  = E_CFD_VALID ;      //

//[STU_CFDAPI_PANEL_FORMAT]
pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[0] = 0x7D00  ;      //data *0.00002 0xC350 = 1
pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[1] = 0x3A98  ;      //
pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[2] = 0x1D4C  ;      //
pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[0] = 0x4047  ;      //
pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[1] = 0x7530  ;      //
pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[2] = 0x0BB8  ;      //
pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16White_point_x  = 0x3D13  ;      //
pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16White_point_y  = 0x4042  ;      //
pstu_CfdAPI_Top_Param->pstu_Panel_Param->u16Panel_Max_Luminance = 150     ;      //data * 1 nits
pstu_CfdAPI_Top_Param->pstu_Panel_Param->u16Panel_Min_Luminance = 500     ;      //data * 0.0001 nits


}


void Mapi_Cfd_testvector062_CurryOnly(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    //Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
       //must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS

       //need to give a default value
       //remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                 = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                                = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;
}

void Mapi_Cfd_testvector062(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Colour_primaries  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Transfer_Characteristics  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
}

void Mapi_Cfd_testvector063(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Colour_primaries  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Transfer_Characteristics  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
}

void Mapi_Cfd_testvector064(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Colour_primaries  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Transfer_Characteristics  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
}

void Mapi_Cfd_testvector065(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_CL ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Colour_primaries  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Transfer_Characteristics  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
}

void Mapi_Cfd_testvector066(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_CL ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Colour_primaries  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Transfer_Characteristics  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
}

void Mapi_Cfd_testvector067(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_ADOBE_RGB ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Colour_primaries  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Transfer_Characteristics  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
}

void Mapi_Cfd_testvector068(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_RGB_BT601_625 ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Colour_primaries  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Transfer_Characteristics  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
}

void Mapi_Cfd_testvector069(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Colour_primaries  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Transfer_Characteristics  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
}

void Mapi_Cfd_testvector070(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Colour_primaries  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Transfer_Characteristics  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC

//[STU_CFDAPI_HDMI_EDID_PARSER]
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid = 0x1     ;       //0 :Not valid, 1 :valid
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF    = 0x1     ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_SM  = 0x1     ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance   = 116     ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance   = 44      ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = 23      ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length    = 6       ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[0]    = 0x28f   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[1]    = 0x133   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[2]    = 0x9A    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[0]    = 0x152   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[1]    = 0x266   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[2]    = 0x3D    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16White_point_x = 0x140   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16White_point_y = 0x151   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_base_block_version = 0x01    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_base_block_reversion   = 0x03    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_CEA_block_reversion    = 0x03    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_VCDB_Valid  = 0x01    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Support_YUVFormat   = 0x1f    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xff    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid  = E_CFD_VALID ;      //

}


void Mapi_Cfd_testvector071(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    //Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x1     ;      //1:depend on output_source infor ON 0: use default values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Colour_primaries  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Transfer_Characteristics  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid = 0x1     ;       //0 :Not valid, 1 :valid
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF    = 0x1     ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_SM  = 0x1     ;      //
//pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance = 116     ;      //
//pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance = 23      ;      //
//pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance   = 44      ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length    = 6       ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[0]    = 0x28f   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[1]    = 0x133   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[2]    = 0x9A    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[0]    = 0x152   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[1]    = 0x266   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[2]    = 0x3D    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16White_point_x = 0x140   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16White_point_y = 0x151   ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_base_block_version = 0x01    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_base_block_reversion   = 0x03    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_CEA_block_reversion    = 0x03    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_VCDB_Valid  = 0x01    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Support_YUVFormat   = 0x1f    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xff    ;      //
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid  = E_CFD_VALID ;      //
}


void Mapi_Cfd_testvector072(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Colour_primaries  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Transfer_Characteristics  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
}

void Mhal_CFD_HDMI_addStaticMetadata(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_HDR_InfoFrame_Valid  = E_CFD_VALID ;
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8Mastering_Display_Infor_Valid   = E_CFD_VALID ;      //E_CFD_VALIDORNOT

    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Max_Luminance = 4000    ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Min_Luminance = 500     ;      //data * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Content_Light_Level    = 1600    ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Frame_Avg_Light_Level  = 128     ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[0]   = 0x7D00  ;      //data *0.00002 0xC350 = 1
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[1]   = 0x3A98  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[2]   = 0x1D4C  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[0]   = 0x4047  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[1]   = 0x7530  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[2]   = 0x0BB8  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_x    = 0x3D13  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_y    = 0x4042  ;      //

}

//with mastering display order R->G->B
void Mhal_CFD_HDMI_addStaticMetadata01(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_HDR_InfoFrame_Valid  = E_CFD_VALID ;
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8Mastering_Display_Infor_Valid   = E_CFD_VALID ;      //E_CFD_VALIDORNOT

    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Max_Luminance = 4000    ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Min_Luminance = 500     ;      //data * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Content_Light_Level    = 1600    ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Frame_Avg_Light_Level  = 128     ;      //data * 1 nits

    //R
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[0]   = 32000  ;      //data *0.00002 0xC350 = 1
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[0]   = 16500  ;      //
    //G
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[1]   = 15000  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[1]   = 30000  ;      //
    //B
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[2]   = 7500  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[2]   = 3000  ;      //

    //D65
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_x    = 15635  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_y    = 16450  ;      //
}

//with mastering display order G->B->R
void Mhal_CFD_HDMI_addStaticMetadata02(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_HDR_InfoFrame_Valid  = E_CFD_VALID ;
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8Mastering_Display_Infor_Valid   = E_CFD_VALID ;      //E_CFD_VALIDORNOT

    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Max_Luminance = 4000    ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Min_Luminance = 500     ;      //data * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Content_Light_Level    = 1600    ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Frame_Avg_Light_Level  = 128     ;      //data * 1 nits

    //G
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[0]   = 15000  ;     //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[0]   = 30000  ;     //
    //B
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[1]   = 7500  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[1]   = 3000  ;      //
    //R
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[2]   = 32000  ;     //data *0.00002 0xC350 = 1
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[2]   = 16500  ;     //

    //D65
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_x    = 15635  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_y    = 16450  ;      //
}

//with mastering display order B->R->G
void Mhal_CFD_HDMI_addStaticMetadata03(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_HDR_InfoFrame_Valid  = E_CFD_VALID ;
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8Mastering_Display_Infor_Valid   = E_CFD_VALID ;      //E_CFD_VALIDORNOT

    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Max_Luminance = 4000    ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Min_Luminance = 500     ;      //data * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Content_Light_Level    = 1600    ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Frame_Avg_Light_Level  = 128     ;      //data * 1 nits

    //B
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[0]   = 7500  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[0]   = 3000  ;      //
    //R
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[1]   = 32000  ;     //data *0.00002 0xC350 = 1
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[1]   = 16500  ;     //
    //G
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[2]   = 15000  ;     //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[2]   = 30000  ;     //

    //D65
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_x    = 15635  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_y    = 16450  ;      //
}

//with mastering display order B->R->G
//negative test
void Mhal_CFD_HDMI_addStaticMetadata03WithWrongValues(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_HDR_InfoFrame_Valid  = E_CFD_VALID ;
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8Mastering_Display_Infor_Valid   = E_CFD_VALID ;      //E_CFD_VALIDORNOT

    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Max_Luminance = 4000    ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Min_Luminance = 500     ;      //data * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Content_Light_Level    = 1600    ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Frame_Avg_Light_Level  = 128     ;      //data * 1 nits

    //B
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[0]   = 7500  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[0]   = 3000  ;      //
    //R
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[1]   = 32000  ;     //data *0.00002 0xC350 = 1
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[1]   = 46500  ;     //
    //G
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[2]   = 15000  ;     //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[2]   = 30000  ;     //

    //D65
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_x    = 15635  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_y    = 16450  ;      //
}


//test HDMI in with HDR infoFrame, CFD bypass
//infoFrame contains mastering information
void Mapi_Cfd_testvector073(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    //Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x1     ;      //1:depend on output_source infor ON 0: use default values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Med_Luminance   = 100     ;      //for TMO, unit : 1 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Source_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Colour_primaries  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Transfer_Characteristics  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_ext_Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC

//input inforFrame
Mhal_CFD_HDMI_addStaticMetadata(pstu_CfdAPI_Top_Param);

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 7;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xFF;

}

void Mapi_Cfd_testvector074(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector073(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                       = 0;
}

//based on Mapi_Cfd_testvector073
//test mastering display parsing
//test HDMI in with HDR infoFrame, CFD bypass
//infoFrame contains mastering information
void Mapi_Cfd_testvector077(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector073(pstu_CfdAPI_Top_Param);

    //input inforFrame
    Mhal_CFD_HDMI_addStaticMetadata01(pstu_CfdAPI_Top_Param);

    //EDID
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 7;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xFF;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

//based on Mapi_Cfd_testvector073
//test mastering display parsing
//test HDMI in with HDR infoFrame, CFD bypass
//infoFrame contains mastering information
void Mapi_Cfd_testvector078(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector073(pstu_CfdAPI_Top_Param);

    //input inforFrame
    Mhal_CFD_HDMI_addStaticMetadata02(pstu_CfdAPI_Top_Param);

    //EDID
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 7;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xFF;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

//based on Mapi_Cfd_testvector073
//test mastering display parsing
//test HDMI in with HDR infoFrame, CFD bypass
//infoFrame contains mastering information
void Mapi_Cfd_testvector079(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector073(pstu_CfdAPI_Top_Param);

    //input inforFrame
    //Mhal_CFD_HDMI_addStaticMetadata03(pstu_CfdAPI_Top_Param);
    Mhal_CFD_HDMI_addStaticMetadata03WithWrongValues(pstu_CfdAPI_Top_Param);

    //EDID
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 7;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xFF;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}


void Mapi_Cfd_testvector080(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_XVYCC_601 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_XVYCC_601 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector081(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_CL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector082(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_CL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector100(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

//[STU_CFDAPI_MM_PARSER]
//pstu_CfdAPI_Top_Param->pstu_MM_Param->u8MM_Codec      = E_CFD_MM_CODEC_HEVC ;      //E_CFD_MM_CODEC_OTHERS
//pstu_CfdAPI_Top_Param->pstu_MM_Param->u8ColorDescription_Valid    = E_CFD_VALID ;      //E_CFD_VALIDORNOT
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Max_Luminance = 40000000 ;      //data = code * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Min_Luminance = 500     ;      //data = code * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[0]   = 0x7D00  ;      //data = code*0.00002 0xC350 = 1
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[1]   = 0x3A98  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[2]   = 0x1D4C  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[0]   = 0x4047  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[1]   = 0x7530  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[2]   = 0x0BB8  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_x    = 0x3D13  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_y    = 0x4042  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Mastering_Display_Infor_Valid   = E_CFD_NOT_VALID ;      //E_CFD_VALIDORNOT

}

void Mapi_Cfd_testvector101(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x2     ;      //2:Force709  1: ON 0: OFF

    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat  = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat  = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    //pstu_CfdAPI_Top_Param->pstu_MM_Param->u8ColorDescription_Valid    = E_CFD_NOT_VALID ;      //E_CFD_VALIDORNOT

}

void Mapi_Cfd_testvector102(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    =  E_CFD_CFIO_CP_BT709_SRGB_SYCC;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    =  E_CFD_CFIO_TR_BT709;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   =  E_CFD_CFIO_MC_BT709_XVYCC709;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR;

}

void Mapi_Cfd_testvector103(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    =  E_CFD_CFIO_CP_BT709_SRGB_SYCC;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    =  E_CFD_CFIO_TR_BT709;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   =  E_CFD_CFIO_MC_BT709_XVYCC709;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR;

}

void Mapi_Cfd_testvector104(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    =  E_CFD_CFIO_CP_BT709_SRGB_SYCC;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    =  E_CFD_CFIO_TR_BT709;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   =  E_CFD_CFIO_MC_BT709_XVYCC709;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2;

}

void Mapi_Cfd_testvector105(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_RGB_NOTSPECIFIED ;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    =  E_CFD_CFIO_CP_BT709_SRGB_SYCC;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    =  E_CFD_CFIO_TR_BT709;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   =  E_CFD_CFIO_MC_IDENTITY;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR;

}

void Mapi_Cfd_testvector106(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    =  E_CFD_CFIO_CP_BT709_SRGB_SYCC;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    =  E_CFD_CFIO_TR_BT709;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   =  E_CFD_CFIO_MC_IDENTITY;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR;

}

void Mapi_Cfd_testvector107(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    =  E_CFD_CFIO_CP_BT709_SRGB_SYCC;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    =  E_CFD_CFIO_TR_BT709;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   =  E_CFD_CFIO_MC_IDENTITY;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2;

}

void Mapi_Cfd_testvector108(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    =  E_CFD_CFIO_CP_BT709_SRGB_SYCC;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    =  E_CFD_CFIO_TR_XVYCC;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   =  E_CFD_CFIO_MC_BT601625_XVYCC601_SYCC;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR;

}

void Mapi_Cfd_testvector109(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    =  E_CFD_CFIO_CP_BT709_SRGB_SYCC;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    =  E_CFD_CFIO_TR_XVYCC;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   =  E_CFD_CFIO_MC_BT601625_XVYCC601_SYCC;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR;

}

void Mapi_Cfd_testvector110(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_RGB_NOTSPECIFIED ;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    =  E_CFD_CFIO_CP_BT709_SRGB_SYCC;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    =  E_CFD_CFIO_TR_XVYCC;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   =  E_CFD_CFIO_MC_BT601625_XVYCC601_SYCC;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR;

}

void Mapi_Cfd_testvector111(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    =  E_CFD_CFIO_CP_BT2020;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    =  E_CFD_CFIO_TR_SMPTE2084;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   =  E_CFD_CFIO_MC_BT2020NCL;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR;

}

void Mapi_Cfd_testvector112(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    =  E_CFD_CFIO_CP_BT2020;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    =  E_CFD_CFIO_TR_SMPTE2084;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   =  E_CFD_CFIO_MC_BT2020NCL;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2;

}

void Mapi_Cfd_testvector113(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    =  E_CFD_CFIO_CP_BT2020;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    =  E_CFD_CFIO_TR_SMPTE2084;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   =  E_CFD_CFIO_MC_IDENTITY;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2;

}

void Mapi_Cfd_testvector114(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    =  E_CFD_CFIO_CP_BT2020;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    =  E_CFD_CFIO_TR_SMPTE2084;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   =  E_CFD_CFIO_MC_IDENTITY;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR1;

}

void Mapi_Cfd_testvector115(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;     //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF

    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
}

void Mapi_Cfd_testvector116(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;     //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF

    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_SMPTE2084 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = 9 ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = 16 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = 9 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
}

void Mapi_Cfd_testvector117(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;     //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF

    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_IDENTITY ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
}

void Mapi_Cfd_testvector118(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;     //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF

    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_SMPTE2084 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_IDENTITY ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
}

void Mapi_Cfd_testvector119(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;     //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF

    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_UNSPECIFIED ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
}

void Mapi_Cfd_testvector120(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;     //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF

    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_UNSPECIFIED ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
}

void Mapi_Cfd_testvector121(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;     //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF

    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_UNSPECIFIED ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
}

void Mapi_Cfd_testvector122(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;     //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF

    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_UNSPECIFIED ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_SMPTE2084 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
}

void Mapi_Cfd_testvector123(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;     //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF

    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_SMPTE2084 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_UNSPECIFIED ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
}

void Mapi_Cfd_testvector124(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;     //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF

    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_UNSPECIFIED ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_IDENTITY ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
}

void Mapi_Cfd_testvector125(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;     //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF

    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_UNSPECIFIED ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_IDENTITY ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
}

void Mapi_Cfd_testvector126(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;     //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF

    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_UNSPECIFIED ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_SMPTE2084 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_IDENTITY ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
}

void Mapi_Cfd_testvector127(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;     //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF

    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_UNSPECIFIED ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_UNSPECIFIED ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_UNSPECIFIED ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
}

void Mapi_Cfd_testvector167(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector100(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;     //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR

    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //2:Force709  1: ON 0: OFF

    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_SMPTE2084 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
}


void Mapi_Cfd_testvector150(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector002(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_CVBS ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_NTSC_44 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x0     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

}

void Mapi_Cfd_testvector151(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector150(pstu_CfdAPI_Top_Param);

    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_BGHI;

}

void Mapi_Cfd_testvector152(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector150(pstu_CfdAPI_Top_Param);

    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_SCART_RGB_NTSC;

}

void Mapi_Cfd_testvector153(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector150(pstu_CfdAPI_Top_Param);

    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_SCART_RGB_PAL;

}

void Mapi_Cfd_testvector154(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector150(pstu_CfdAPI_Top_Param);

    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_YPBPR_1080P_60HZ;

}

void Mapi_Cfd_testvector180(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    //Mapi_Cfd_testvector150(pstu_CfdAPI_Top_Param);

    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_YPBPR_1080P_60HZ;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source                               = E_CFD_INPUT_SOURCE_STORAGE;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                   = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                               = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                              = E_CFD_CFIO_RANGE_LIMIT;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                  = E_CFIO_MODE_HDR2;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode                                                           = 1 ;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode                                                           = 1  ;
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format                = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat            = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange           = E_CFD_CFIO_RANGE_LIMIT;
pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode               = E_CFIO_MODE_SDR;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source                                  = E_CFD_OUTPUT_SOURCE_PANEL;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format                                  = E_CFD_CFIO_RGB_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                              = E_CFD_MC_FORMAT_RGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange                             = E_CFD_CFIO_RANGE_LIMIT;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode                                 = E_CFIO_MODE_SDR;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode                                                      = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance                                                     = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance                                                     = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status                                                            = 64;
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                           = E_CFD_CFIO_CP_BT2020;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                   = E_CFD_CFIO_TR_SMPTE2084;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                              = E_CFD_CFIO_MC_BT2020NCL;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                      = E_CFD_CFIO_RANGE_LIMIT;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Max_Luminance                                                   = 1;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Min_Luminance                                                   = 0;
pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[0]                     = 0;
pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[1]                     = 0;
pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[2]                     = 0;
pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[0]                     = 0;
pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[1]                     = 0;
pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[2]                     = 0;
pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_x                              = 0;
pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_y                              = 0;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Mastering_Display_Infor_Valid                                                 = 0;
//[STU_CFDAPI_HDMI_INFOFRAME_PARSER]
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_HDR_InfoFrame_Valid                                    = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_EOTF                                                   = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_SMD_ID                                                 = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Max_Luminance                                       = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Min_Luminance                                       = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Content_Light_Level                                          = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Frame_Avg_Light_Level                                        = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[0] = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[1] = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[2] = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[0] = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[1] = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[2] = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_x          = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_y          = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8Mastering_Display_Infor_Valid                                     = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_Support_Format                                         = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_Colorspace                                             = 0;
//[STU_CFDAPI_PANEL_FORMAT]
pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[0]                           = 32000;
pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[1]                           = 15000;
pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[2]                           = 7500 ;
pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[0]                           = 16455;
pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[1]                           = 30000;
pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[2]                           = 3000 ;
pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16White_point_x                                    = 15635;
pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16White_point_y                                    = 16450;
pstu_CfdAPI_Top_Param->pstu_Panel_Param->u16Panel_Max_Luminance                                                       = 100  ;
pstu_CfdAPI_Top_Param->pstu_Panel_Param->u16Panel_Min_Luminance                                                       = 500  ;

}

void Mapi_Cfd_testvector201(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{

//[Global]

//[Color_format_driver_structure]

//[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

//[STU_CFDAPI_MM_PARSER]
//pstu_CfdAPI_Top_Param->pstu_MM_Param->u8MM_Codec      = E_CFD_MM_CODEC_HEVC ;      //E_CFD_MM_CODEC_OTHERS
//pstu_CfdAPI_Top_Param->pstu_MM_Param->u8ColorDescription_Valid    = E_CFD_VALID ;      //E_CFD_VALIDORNOT
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries    = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag   = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Max_Luminance = 40000000 ;      //data = code * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Min_Luminance = 500     ;      //data = code * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[0]   = 0x7D00  ;      //data = code*0.00002 0xC350 = 1
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[1]   = 0x3A98  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[2]   = 0x1D4C  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[0]   = 0x4047  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[1]   = 0x7530  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[2]   = 0x0BB8  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_x    = 0x3D13  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_y    = 0x4042  ;      //
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Mastering_Display_Infor_Valid   = E_CFD_NOT_VALID ;      //E_CFD_VALIDORNOT

//[STU_CFDAPI_HDMI_INFOFRAME_PARSER]
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_HDR_InfoFrame_Valid  = E_CFD_NOT_VALID ;      //E_CFD_VALIDORNOT
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_EOTF = E_CFD_HDMI_EOTF_SMPTE2084 ;      //E_CFD_HDMI_HDR_INFOFRAME_EOTF
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_SMD_ID   = E_CFD_HDMI_META_TYPE1 ;      //E_CFD_HDMI_HDR_INFOFRAME_METADATA
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Max_Luminance = 4000    ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Min_Luminance = 500     ;      //data * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Content_Light_Level    = 600     ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Frame_Avg_Light_Level  = 128     ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[0]   = 0x7D00  ;      //data *0.00002 0xC350 = 1
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[1]   = 0x3A98  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[2]   = 0x1D4C  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[0]   = 0x4047  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[1]   = 0x7530  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[2]   = 0x0BB8  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_x    = 0x3D13  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_y    = 0x4042  ;      //
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8Mastering_Display_Infor_Valid   = E_CFD_NOT_VALID ;      //E_CFD_VALIDORNOT
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_Support_Format   = 0x0     ;      //{Q1 Q0 YQ1 YQ0 Y2 Y1 Y0}
    pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_Colorspace   = E_CFD_CFIO_YUV_BT709 ;      //E_CFD_CFIO
//pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_AVIInfoFrame_Valid = E_CFD_VALID ;      //E_CFD_VALIDORNOT

//[STU_CFDAPI_PANEL_FORMAT]
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[0] = 0x7D00  ;      //data *0.00002 0xC350 = 1
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[1] = 0x3A98  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[2] = 0x1D4C  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[0] = 0x4047  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[1] = 0x7530  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[2] = 0x0BB8  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16White_point_x  = 0x3D13  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16White_point_y  = 0x4042  ;      //
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->u16Panel_Max_Luminance = 150     ;      //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_Panel_Param->u16Panel_Min_Luminance = 500     ;      //data * 0.0001 nits

//[STU_CFDAPI_Maserati_TMOIP]
#if NowHW == Maserati
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_enable_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_setting_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_UVC_setting_Mode  = 0xC0    ;      //default = 0xc0

//[STU_CFDAPI_Maserati_HDRIP]
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Composer_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Module1_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Mode = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Ratio1   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Manual_Vars_en   = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_MC   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_SRAM_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_DeGamma_Manual_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_TR    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_Lut_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.pu32HDR_Degamma_Lut_Address = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Lut_Length   = 0x200     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8DHDR_Degamma_Max_Lum_En   = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Max_Lum  = 0xFF00  ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_3x3_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_Manual_Vars_en    = 0x0     ;      //0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_InputCP   = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_OutputCP  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_SRAM_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Manual_Vars_en  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_TR  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Lut_En  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.pu32HDR_Gamma_Lut_Address   = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Gamma_Lut_Length = 0x200     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Mode    = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Ratio1  = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Manual_Vars_en  = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_MC  = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Yoffset_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_NLM_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_NLM_setting_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACGain_enable_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACGain_setting_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACE_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACE_setting_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither1_setting_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_enable_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_SRAM_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_setting_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_444to422_enable_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither2_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither2_setting_Mode  = 0xC0    ;      //default = 0xc0

//[STU_CFDAPI_Maserati_SDRIP]
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_Mode  = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_Manual_Vars_en    = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8IP2_CSC_MC    = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_CSC_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PreYoffset_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PreYgain_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PreYgain_dither_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PostYoffset_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PostYgain_Mode    = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VIP_PostYoffset2_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_Mode  = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_Manual_Vars_en    = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3_MC    = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8VOP_3x3RGBClip_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8LinearRGBBypass_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_enable_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Dither_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_SRAM_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Degamma_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8DeGamma_Manual_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_TR    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Lut_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.pu32Degamma_Lut_Address = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Degamma_Lut_Length   = 0x100     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Degamma_Max_Lum_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Degamma_Max_Lum  = 0xFF00  ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_enable_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u163x3_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_Manual_Vars_en    = 0x0     ;      //0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_InputCP   = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3_OutputCP  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Compress_settings_Mode    = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Compress_dither_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u83x3Clip_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_enable_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_Dither_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_maxdata_Mode    = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_SRAM_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_Mode_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_TR  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u8Gamma_Lut_En  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.pu32Gamma_Lut_Address   = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_SDRIP_Input->stu_Maserati_SDRIP_Param.u16Gamma_Lut_Length = 0x100     ;      //

//[STU_CFDAPI_Maserati_DLCIP]
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_DLC_Input->stu_Maserati_DLC_Param.u8DLC_curve_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_DLC_Input->stu_Maserati_DLC_Param.u8DLC_curve_enable_Mode = 0xC0    ;      //default = 0xc0
//pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_DLC_Input->stu_Maserati_DLC_Param.u8UVC_enable_Mode = 0xC0    ;      //default = 0xc0
//pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_DLC_Input->stu_Maserati_DLC_Param.u8UVC_setting_Mode    = 0xC0    ;      //default = 0xc0
#endif

}

void Mapi_Cfd_testvector202(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector201(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format = E_CFD_CFIO_YUV_BT2020_CL;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT2020_NCL;

}

void Mapi_Cfd_testvector203(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector201(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format = E_CFD_CFIO_XVYCC_709;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709;

}

void Mapi_Cfd_testvector204(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector201(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode = E_CFIO_MODE_SDR;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode = E_CFIO_MODE_HDR2;

}

void Mapi_Cfd_testvector205(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector201(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode = E_CFIO_MODE_HDR2;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode = E_CFIO_MODE_HDR1;

}

void Mapi_Cfd_testvector206(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector201(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode = E_CFIO_MODE_SDR;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode = E_CFIO_MODE_SDR;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode = E_CFIO_MODE_HDR2;
}

void Mapi_Cfd_testvector207(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector201(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode = E_CFIO_MODE_HDR2;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode = E_CFIO_MODE_HDR2;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode = E_CFIO_MODE_HDR1;
}

void Mapi_Cfd_testvector208(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector201(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode = E_CFIO_MODE_SDR;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode = E_CFIO_MODE_HDR1;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode = E_CFIO_MODE_HDR1;
}

void Mapi_Cfd_testvector301(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector001(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x0     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

    //HDR user control
#if NowHW == Maserati
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_enable_Mode   = 0x41    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Composer_Mode = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Module1_enable_Mode   = 0x41    ;      //default = 0xc0
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Mode = 0x40    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Ratio1   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Manual_Vars_en   = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_MC   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_SRAM_Mode = 0x40    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_DeGamma_Manual_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_TR    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_Lut_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.pu32HDR_Degamma_Lut_Address = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Lut_Length   = 0x200     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8DHDR_Degamma_Max_Lum_En   = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Max_Lum  = 0xFF00  ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_Mode  = 0x40    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_3x3_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_Manual_Vars_en    = 0x0     ;      //0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_InputCP   = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_OutputCP  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_SRAM_Mode   = 0x40    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Manual_Vars_en  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_TR  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Lut_En  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.pu32HDR_Gamma_Lut_Address   = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Gamma_Lut_Length = 0x200     ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Mode    = 0x40    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Ratio1  = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Manual_Vars_en  = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_MC  = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Yoffset_Mode  = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_NLM_enable_Mode   = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_NLM_setting_Mode  = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACGain_enable_Mode    = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACGain_setting_Mode   = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACE_enable_Mode   = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACE_setting_Mode  = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither1_setting_Mode  = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_enable_Mode = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_SRAM_Mode   = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_setting_Mode    = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_444to422_enable_Mode  = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither2_enable_Mode   = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither2_setting_Mode  = 0x40    ;      //default = 0xc0

    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_Mode    = 0x41    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_enable_Mode = 0x41    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Maserati_TMO_Param.u8HDR_TMO_curve_setting_Mode    = 0x41    ;      //default = 0xc0
#endif
}

void Mapi_Cfd_testvector302(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector001(pstu_CfdAPI_Top_Param);

    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange    = E_CFD_CFIO_RANGE_LIMIT ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode    = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x1     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

    //[STU_CFDAPI_Maserati_HDRIP]
#if NowHW == Maserati
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Composer_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Module1_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Mode = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Ratio1   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Manual_Vars_en   = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_MC   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_SRAM_Mode = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_DeGamma_Manual_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_TR    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_Lut_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.pu32HDR_Degamma_Lut_Address = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Lut_Length   = 0x200     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8DHDR_Degamma_Max_Lum_En   = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Max_Lum  = 0xFF00  ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_Mode  = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_3x3_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_Manual_Vars_en    = 0x0     ;      //0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_InputCP   = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_OutputCP  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_SRAM_Mode   = 0xC0    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Manual_Vars_en  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_TR  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Lut_En  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.pu32HDR_Gamma_Lut_Address   = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Gamma_Lut_Length = 0x200     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Mode    = 0xC0    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Ratio1  = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Manual_Vars_en  = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_MC  = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Yoffset_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_NLM_enable_Mode   = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_NLM_setting_Mode  = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACGain_enable_Mode    = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACGain_setting_Mode   = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACE_enable_Mode   = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACE_setting_Mode  = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither1_setting_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_enable_Mode = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_SRAM_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_setting_Mode    = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_444to422_enable_Mode  = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither2_enable_Mode   = 0xC0    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither2_setting_Mode  = 0xC0    ;      //default = 0xc0
#endif
}

void Mapi_Cfd_testvector303(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector001(pstu_CfdAPI_Top_Param);
    //[STU_CFDAPI_MAIN_CONTROL]
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Mode  = 0x1     ;     //0:off 1:on - normal mode 2:on - test mode
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_Structure  = E_CFD_HWS_TV_TYPE0 ;     //assign by E_CFD_MC_HW_STRUCTURE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HW_PatchEn    = 0x0     ;     //0: patch off, 1:patch on
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_HDMI ;     //assign by E_CFD_MC_SOURCE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_AnalogIdx   = E_CFD_INPUT_ANALOG_RF_PAL_60 ;     //assign by E_CFD_INPUT_ANALOG_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format    = E_CFD_CFIO_YUV_BT2020_NCL ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat    = E_CFD_MC_FORMAT_YUV444 ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange   = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass   = 0x1     ;     //0:no R2Y for RGB in;1:R2Y for RGB in
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode = 0x1     ;     //0: bypass all SDR IP; 1: auto for all SDR IP ; 2: bypass besides outputCSC
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 0x1     ;     //0: bypass all HDR IP; 1: bypass all HDR IP ; 2: openHDR ; 3:Dolby HDR
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Temp_Format   = E_CFD_CFIO_YUV_BT709 ;     //assign by E_CFD_CFIO
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Temp_DataFormat   = E_CFD_MC_FORMAT_YUV422 ;     //assign by E_CFD_MC_FORMAT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Temp_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Temp_HDRMode  = E_CFIO_MODE_HDR1 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_PANEL ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFD_CFIO
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat   = E_CFD_MC_FORMAT_RGB ;     //assign by E_CFD_MC_FORMAT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange  = E_CFD_CFIO_RANGE_FULL ;     //assign by E_CFD_CFIO_RANGE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format_HDMI_CE_Flag = 0x0     ;    //1: CE 0 : IT
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format_HDMI_CE_Flag    = 0x0     ;    //1: CE 0 : IT
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode   = 0x0     ;     //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode = 0x0     ;     //0: extension 1:compression
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode  = 0x0     ;      //1: ON 0: OFF
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode    = 0x0     ;      //1:depend on output_source infor ON 0: use default values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance   = 100     ;      //for TMO, unit : 1 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance   = 500     ;      //for TMO, unit : 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status  = 0x0     ;      //report for final status

    //HDR user control
#if NowHW == Maserati
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_enable_Mode   = 0x41    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Composer_Mode = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Module1_enable_Mode   = 0x41    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Mode = 0x47    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Ratio1   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_Manual_Vars_en   = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_InputCSC_MC   = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_SRAM_Mode = 0x41    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_Ratio1    = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_DeGamma_Manual_Vars_en    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_TR    = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Degamma_Lut_En    = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.pu32HDR_Degamma_Lut_Address = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Lut_Length   = 0x200     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8DHDR_Degamma_Max_Lum_En   = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Degamma_Max_Lum  = 0xFF00  ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_Mode  = 0x40    ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_3x3_Ratio2   = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_Manual_Vars_en    = 0x0     ;      //0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_InputCP   = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3x3_OutputCP  = E_CFD_CFIO_CP_BT709_SRGB_SYCC ;      //E_CFD_CFIO_CP
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_SRAM_Mode   = 0x41   ;      //0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Manual_Vars_en  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_TR  = E_CFD_CFIO_TR_BT709 ;      //E_CFD_CFIO_TR
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Gamma_Lut_En  = 0x0     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.pu32HDR_Gamma_Lut_Address   = NULL    ;      //
pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u16HDR_Gamma_Lut_Length = 0x200     ;      //
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Mode    = 0x42    ;      //E_CFD_IP_CSC_PROCESS,0xC0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Ratio1  = 0x40    ;      //0x40 = 1x
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_Manual_Vars_en  = 0x0     ;      //0x0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_OutputCSC_MC  = E_CFD_CFIO_MC_BT709_XVYCC709 ;      //E_CFD_CFIO_MC
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Yoffset_Mode  = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_NLM_enable_Mode   = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_NLM_setting_Mode  = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACGain_enable_Mode    = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACGain_setting_Mode   = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACE_enable_Mode   = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_ACE_setting_Mode  = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither1_setting_Mode  = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_enable_Mode = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_SRAM_Mode   = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_3DLUT_setting_Mode    = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_444to422_enable_Mode  = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither2_enable_Mode   = 0x40    ;      //default = 0xc0
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_HDRIP_Input->stu_Maserati_HDRIP_Param.u8HDR_Dither2_setting_Mode  = 0x40    ;      //default = 0xc0
#endif
}

//refer to EDID
//MM in = YUV, limited, SDR, 709
//Process = SDR in, SDR out

void Mapi_Cfd_testvector400(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = 2;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = 0;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                 = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                                = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_BT709;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT709_XVYCC709;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

}

//refer to EDID
//MM in = YUV, limited, SDR, 2020 ncl
//Process = SDR in, SDR out

void Mapi_Cfd_testvector401(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = 2;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = 0;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                 = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT2020;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_BT2020NCL;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT2020NCL;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

}

//refer to EDID
//MM in = YUV, full, SDR, 709
//Process = SDR in, SDR out

void Mapi_Cfd_testvector402(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_FULL;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_BT709;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT709_XVYCC709;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_FULL;

}


//refer to EDID
//MM in = YUV, limited, SDR, BT2020CL, HDMI TX not support BT2020CL
//Process = SDR in, SDR out

void Mapi_Cfd_testvector403(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT2020;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_BT2020CL;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT2020CL;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;


//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0x00;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;

}

//refer to EDID
//MM in = YUV, limited, SDR, BT2020CL , HDMI TX support BT2020CL
//Process = SDR in, SDR out

void Mapi_Cfd_testvector407(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT2020;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_BT2020CL;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT2020CL;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;


//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xFF;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

//refer to EDID
//MM in = RGB, full, SDR, 709
//Process = SDR in, SDR out

void Mapi_Cfd_testvector404(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_RGB_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_RGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_FULL;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_BT709;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT709_XVYCC709;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_FULL;

}

//refer to EDID
//MM in = YUV, full, SDR, 709
//Process = SDR in, SDR out

void Mapi_Cfd_testvector405(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_RGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_FULL;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_BT709;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_IDENTITY;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_FULL;

}

//refer to EDID
//MM in = YUV, full, SDR, 709
//Process = SDR in, SDR out

void Mapi_Cfd_testvector406(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV422;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_FULL;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_BT709;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_IDENTITY;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_FULL;

}

//MM in = YUV, limited, SDR, xvYCC601
//Process = SDR in, SDR out

void Mapi_Cfd_testvector408_0(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_FULL;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_XVYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT601625_XVYCC601_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0x00;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

//MM in = YUV, limited, SDR, xvYCC601
//Process = SDR in, SDR out

void Mapi_Cfd_testvector408_1(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_FULL;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_XVYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT601625_XVYCC601_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xff;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

//MM in = YUV, full, SDR, xvYCC601
//Process = SDR in, SDR out
void Mapi_Cfd_testvector409(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_FULL;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_XVYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT601625_XVYCC601_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_FULL;

}

//MM in = RGB, full, SDR, xvYCC601
//Process = SDR in, SDR out
void Mapi_Cfd_testvector410(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_RGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_FULL;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_XVYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT601625_XVYCC601_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_FULL;
}

//MM in = YUV, limited, HDR PQ, 2020 ncl , HDMI TX = SDR TV
void Mapi_Cfd_testvector411(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
#if 1
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 1;
#else
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
#endif
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT2020;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_SMPTE2084;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT2020NCL;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;

#if 0
//test for bypass case
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                           = 7;
#else
//test for not bypass case
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
#endif
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0x00;

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

//input is JPEG, BT601 SDR
void Mapi_Cfd_testvector4111(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
#if 1
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 1;
#else
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
#endif
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = 5;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = 6;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = 5;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = 0;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 0;

#if 0
//test for bypass case
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                           = 7;
#else
//test for not bypass case
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
#endif
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0x00;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;

}

//MM in = YUV, limited, HDR PQ, 2020 ncl , HDMI TX = HDR TV
void Mapi_Cfd_testvector412(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT2020;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_SMPTE2084;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT2020NCL;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;

#if 1
//test for bypass case
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                           = 7;
#else
//test for not bypass case
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
#endif
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xff;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;

}

//MM in = YUV, limited, HDR PQ, 2020 ncl , HDMI TX = HDR TV ??
void Mapi_Cfd_testvector413(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT2020;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_SMPTE2084;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_IDENTITY;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

}

//like 402, u8MMInput_ColorimetryHandle_Mode = 0
void Mapi_Cfd_testvector415(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_FULL;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_BT709;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT709_XVYCC709;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_FULL;

}

//like 411, u8MMInput_ColorimetryHandle_Mode = 0
void Mapi_Cfd_testvector416(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_FULL;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_SMPTE2084;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT709_XVYCC709;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_FULL;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;

#if 0
//test for bypass case
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 7;
#else
//test for not bypass case
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
#endif
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0x00;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

//like 404, u8MMInput_ColorimetryHandle_Mode = 0
void Mapi_Cfd_testvector417(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_RGB_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_RGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_FULL;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_BT709;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_IDENTITY;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_FULL;

}


//MM in = RGB, full, HDR PQ, 2020 ncl , HDMI TX = HDR TV
void Mapi_Cfd_testvector418(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_RGB_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_RGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_FULL;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_SMPTE2084;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_IDENTITY;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_FULL;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 7;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}


//MM in = RGB, full, HDR PQ, 2020 ncl , HDMI TX = SDR TV
void Mapi_Cfd_testvector419(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_RGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_FULL;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_UNSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_SMPTE2084;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_IDENTITY;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_FULL;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

//refer to EDID
//MM in = YUV, limited, SDR, xvYCC601 , HDMI TX = HDR TV
void Mapi_Cfd_testvector429(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_RGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_FULL;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_XVYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT601625_XVYCC601_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_FULL;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 7;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

//refer to EDID
//like 412, u8HDMIOutput_GammutMapping_Mode = 0
//MM in = YUV, limited, HDR10, BT2020 ncl
//HDMI out, EDID support SDR,HDR,SMPTE2084
//Process = bypass
//no gamut mapping

void Mapi_Cfd_testvector430(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT2020;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_SMPTE2084;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT2020NCL;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 7;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

//refer to EDID
//like 430, EDID adds u8HDMISink_Extended_Colorspace
//MM in = YUV, limited, HDR10, BT2020 ncl
//HDMI out, EDID support SDR,HDR,SMPTE2084, colorimetry
//Process = bypass

void Mapi_Cfd_testvector431(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT2020;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_SMPTE2084;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT2020NCL;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 7;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xff;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}


//MM in = RGB, full, HDR PQ, AdobeRGB , HDMI TX = HDR TV
//HDMI out = HDR is valid
//Process = bypass
void Mapi_Cfd_testvector432(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_RGB_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_RGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_FULL;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_ADOBERGB;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_SMPTE2084;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT601625_XVYCC601_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_FULL;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 7;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xff;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

//MM in = RGB, full, HDR PQ, AdobeRGB , HDMI TX = SDR TV
void Mapi_Cfd_testvector433(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_RGB_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_RGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_FULL;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_ADOBERGB;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_SMPTE2084;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT601625_XVYCC601_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_FULL;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 7;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0x00;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}


//test purpose = HDR EDID parsing of max, max frame-average, min
//based on testvector432
//MM in = HDR
//HDMI out = HDR is valid
//Process = bypass

void Mapi_Cfd_testvector435(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    //Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
    //must set these values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;

    //need to give a default value
    //remove to the function before calling CFD
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 1;

    //MM information
    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT2020;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_SMPTE2084;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT2020NCL;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;


//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xff;

#define testcase 1

#if (testcase == 1)
//case 1 ----------------------------------------------------------------------------------------------------
//max: 12525 cd/m^2 = code 255
//max ave : 3805 cd/m^2 = code 200
//min:  0.002 cd/m^2 = code 1

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance = 255;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = 200;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance = 1;
//------------------------------------------------------------------------------------------------------------
#elif (testcase == 2)

//case 2 ----------------------------------------------------------------------------------------------------
//max: 1037.472 cd/m^2 = code 140
//max ave : 95.76 cd/m^2 = code 30
//min:  2.614 cd/m^2 = code 128

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance = 140;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = 30;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance = 128;
//------------------------------------------------------------------------------------------------------------

#elif (testcase == 3)

//case 3 ----------------------------------------------------------------------------------------------------
//max: 51.095 cd/m^2 = code 1
//max ave : 817.518 cd/m^2 = code 129
//min:  0.314 cd/m^2 = code 200

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = 129;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance = 200;
//-----------------------------------------------------------------------------------------------------------

#endif

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 7;
}

//test purpose = HDR EDID parsing of max, max frame-average, min
//based on testvector432
//MM in = HDR
//HDMI out = HDR is valid
//Process = bypass

void Mapi_Cfd_testvector436(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    //Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
    //must set these values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;

    //need to give a default value
    //remove to the function before calling CFD
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 1;

    //MM information
    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT2020;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_SMPTE2084;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT2020NCL;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;


//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xff;

#define testcase436 2

#if (testcase436 == 1)
//case 1 ----------------------------------------------------------------------------------------------------
//max: 12525 cd/m^2 = code 255
//max ave : 3805 cd/m^2 = code 200
//min:  0.002 cd/m^2 = code 1

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance = 255;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = 200;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance = 1;
//------------------------------------------------------------------------------------------------------------
#elif (testcase436 == 2)

//case 2 ----------------------------------------------------------------------------------------------------
//max: 1037.472 cd/m^2 = code 140
//max ave : 95.76 cd/m^2 = code 30
//min:  2.614 cd/m^2 = code 128

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance = 140;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = 30;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance = 128;
//------------------------------------------------------------------------------------------------------------

#elif (testcase436 == 3)

//case 3 ----------------------------------------------------------------------------------------------------
//max: 51.095 cd/m^2 = code 1
//max ave : 817.518 cd/m^2 = code 129
//min:  0.314 cd/m^2 = code 200

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = 129;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance = 200;
//-----------------------------------------------------------------------------------------------------------

#endif
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 7;
}

//test purpose = HDR EDID parsing of max, max frame-average, min
//based on testvector432
//MM in = HDR
//HDMI out = HDR is valid
//Process = bypass

void Mapi_Cfd_testvector437(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    //Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
    //must set these values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;

    //need to give a default value
    //remove to the function before calling CFD
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 1;

    //MM information
    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT2020;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_SMPTE2084;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT2020NCL;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;


//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xff;

#define testcase437 3

#if (testcase437 == 1)
//case 1 ----------------------------------------------------------------------------------------------------
//max: 12525 cd/m^2 = code 255
//max ave : 3805 cd/m^2 = code 200
//min:  0.002 cd/m^2 = code 1

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance = 255;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = 200;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance = 1;
//------------------------------------------------------------------------------------------------------------
#elif (testcase437 == 2)

//case 2 ----------------------------------------------------------------------------------------------------
//max: 1037.472 cd/m^2 = code 140
//max ave : 95.76 cd/m^2 = code 30
//min:  2.614 cd/m^2 = code 128

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance = 140;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = 30;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance = 128;
//------------------------------------------------------------------------------------------------------------

#elif (testcase437 == 3)

//case 3 ----------------------------------------------------------------------------------------------------
//max: 51.095 cd/m^2 = code 1
//max ave : 817.518 cd/m^2 = code 129
//min:  0.314 cd/m^2 = code 200

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = 129;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance = 200;
//-----------------------------------------------------------------------------------------------------------

#endif
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 7;
}

//test purpose = HDR EDID parsing of max, max frame-average, min
//use default value in CFD
//u8TMO_TargetRefer_Mode = 0;
//based on testvector432
//MM in = HDR
//HDMI out = HDR is valid
//Process = bypass

void Mapi_Cfd_testvector438(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    //Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
    //must set these values
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;

    //need to give a default value
    //remove to the function before calling CFD
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

    //MM information
    //[STU_CFDAPI_MM_PARSER]
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT2020;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_SMPTE2084;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT2020NCL;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;


//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xff;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

//test purpose = MM HLG in
//based on testvector432
//MM in = HDR
//HDMI out = HDR is valid
//Process = bypass

void Mapi_Cfd_testvector440(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR3 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 1;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT2020;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_HLG;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT2020NCL;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 7;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xff;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

//test purpose = HDR EDID parsing of max, max frame-average, min
//based on testvector432
//MM in = HDR
//HDMI out = HDR is not valid
//Process = SDR

void Mapi_Cfd_testvector441(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR3 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT2020;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_HLG;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT2020NCL;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xff;

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

//gamut mapping case
void Mapi_Cfd_testvector450(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT2020;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_SMPTE2084;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT2020NCL;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xFF;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

void Mapi_Cfd_testvector451(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR2 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 2;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT2020;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_SMPTE2084;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT2020NCL;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xFF;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

void Mapi_Cfd_testvector452(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT2020;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_BT2020NCL;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT2020NCL;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0x00;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

void Mapi_Cfd_testvector453(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR3 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_ADOBERGB;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_HLG;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT601625_XVYCC601_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xFF;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

void Mapi_Cfd_testvector454(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_ADOBERGB;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_ADOBERGB;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT601625_XVYCC601_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xFF;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

void Mapi_Cfd_testvector455(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_HDR3 ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_HLG;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT709_XVYCC709;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xFF;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

void Mapi_Cfd_testvector456(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;

//need to give a default value
//remove to the function before calling CFD
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_BT709;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT709_XVYCC709;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_LIMIT;

//EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 0;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 3;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = 0xFF;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

//add mastering SEI information to test vector
void Mhal_Cfd_MMin_AddMasteringSEI(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{

    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Mastering_Display_Infor_Valid = E_CFD_VALID;

    //SEI from HEVC
    //data = code * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Max_Luminance = 40000000;
    //data = code * 0.0001 nits
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Min_Luminance = 500;
    //data * 1 nits
    //pstMMParamInita->u16Max_Content_Light_Level = 0;
    //data * 1 nits
    //pstMMParamInita->u16Max_Frame_Avg_Light_Level = 0;

    //order R->G->B
    //data = code*0.00002 0xC350 = 1
    //BT709
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[0] = 0x7D00;  //0.64
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[1] = 0x3A98;  //0.3
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[2] = 0x1D4C;  //0.15

    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[0] = 0x4074;  //0.33
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[1] = 0x7530;  //0.6
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[2] = 0x0BB8;  //0.06

    //D65
    //data *0.00002 0xC350 = 1
    //x 0.3127
    //y 0.3290
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_x = 0x3D13;  //0.3127
    pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_y = 0x4042;  //0.3290
}

//add content light level SEI information to test vector
void Mhal_Cfd_MMin_AddContentLightLevelSEI(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u8MM_HDR_ContentLightMetaData_Valid = E_CFD_VALID;
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u16Max_content_light_level = 1300;    //data * 1 nits
    pstu_CfdAPI_Top_Param->pstu_MM_Param->u16Max_pic_average_light_level = 100;    //data * 1 nits
}

//test purpose = test MM in with Mastering EDID and content light level SEI
//CFD bypass , link input SEI with outpu InfoFrame
//test group : from 460 to 464

//based on testvector430
//MM in = HDR
//HDMI out = HDR is valid
//Process = bypass

//test MM in no Mastering SEI and no content light level SEI
//CFD bypass , link input SEI with outpu InfoFrame
void Mapi_Cfd_testvector460(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector430(pstu_CfdAPI_Top_Param);
    //Mhal_Cfd_MMin_AddMasteringSEI(pstu_CfdAPI_Top_Param);
}

//test MM in with Mastering SEI and content light level SEI
//CFD bypass , link input SEI with outpu InfoFrame
void Mapi_Cfd_testvector461(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector430(pstu_CfdAPI_Top_Param);
    Mhal_Cfd_MMin_AddMasteringSEI(pstu_CfdAPI_Top_Param);
    Mhal_Cfd_MMin_AddContentLightLevelSEI(pstu_CfdAPI_Top_Param);
}

//test MM in with Mastering SEI and no content light level SEI
//CFD bypass , link input SEI with outpu InfoFrame
void Mapi_Cfd_testvector462(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector430(pstu_CfdAPI_Top_Param);
    Mhal_Cfd_MMin_AddMasteringSEI(pstu_CfdAPI_Top_Param);
    //Mhal_Cfd_MMin_AddContentLightLevelSEI(pstu_CfdAPI_Top_Param);
}

//test MM in no Mastering SEI and with content light level SEI
//CFD bypass , link input SEI with outpu InfoFrame
void Mapi_Cfd_testvector463(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector430(pstu_CfdAPI_Top_Param);
    //Mhal_Cfd_MMin_AddMasteringSEI(pstu_CfdAPI_Top_Param);
    Mhal_Cfd_MMin_AddContentLightLevelSEI(pstu_CfdAPI_Top_Param);
}

//test MM in with Mastering SEI and content light level SEI
//CFD not bypass , link input SEI with outpu InfoFrame
void Mapi_Cfd_testvector464(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector430(pstu_CfdAPI_Top_Param);
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid = 0;

}

//test HDR bypass
void Mapi_Cfd_testvector470(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    //EDID
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                 = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF                                                          = 7;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace                                = 0xff;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 4;
}

//predefind case 001
//for UI display
void Mapi_Cfd_testvector501(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
//Mapi_Cfd_testvector061(pstu_CfdAPI_Top_Param);
//must set these values
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = E_CFD_CFIO_YUV_NOTSPECIFIED;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = E_CFD_MC_FORMAT_YUV444;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = E_CFD_CFIO_RANGE_LIMIT;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = 0;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source    = E_CFD_INPUT_SOURCE_STORAGE ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source   = E_CFD_OUTPUT_SOURCE_HDMI ;    //assign by E_CFD_MC_SOURCE
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode   = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode  = E_CFIO_MODE_SDR ;     //assign by E_CFIO_HDR_STATUS
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format = E_CFD_CFIO_RGB_NOTSPECIFIED ;     //assign by E_CFIO_HDR_STATUS

pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format   = E_CFD_CFIO_SRGB;
pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = E_CFD_MC_FORMAT_RGB;

//need to give a default value
//remove to the function before calling CFD
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = 0;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = 0;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = 1;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = 0;
//pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 0;

//MM information
//[STU_CFDAPI_MM_PARSER]
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = E_CFD_CFIO_TR_BT709;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = E_CFD_CFIO_MC_BT709_XVYCC709;
pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = E_CFD_CFIO_RANGE_FULL;

}

//1001
void Mapi_Cfd_testvector1001(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_OSD_Param->u8HDR_UI_H2SMode = 0;
}

//1002
void Mapi_Cfd_testvector1002(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_OSD_Param->u8HDR_UI_H2SMode = 1;
}

//1003
void Mapi_Cfd_testvector1003(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_OSD_Param->u8HDR_UI_H2SMode = 2;
}


//1010
//source auto
#if 0
void Mapi_Cfd_testvector1010(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 2;

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_SourceUserModeEn = 0;
    //0: auto
    //priority : infoFrame > driver > CFD default
    //1: force, refer to u8SourceUserMode

    //write
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_SourceUserMode = 0;
    //0: refer to infoFrame
    //1: refer to driver
    //2: refer to CFD default
}

//1011
//source force mode = 0;
void Mapi_Cfd_testvector1011(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 2;

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_SourceUserModeEn = 1;
    //0: auto
    //priority : infoFrame > driver > CFD default
    //1: force, refer to u8SourceUserMode

    //write
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_SourceUserMode = 0;
    //0: refer to infoFrame
    //1: refer to driver
    //2: refer to CFD default
}

//1012
//source force mode = 1;
void Mapi_Cfd_testvector1012(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 2;

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_SourceUserModeEn = 1;
    //0: auto
    //priority : infoFrame > driver > CFD default
    //1: force, refer to u8SourceUserMode

    //write
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_SourceUserMode = 1;
    //0: refer to infoFrame
    //1: refer to driver
    //2: refer to CFD default
}

//1013
//source force mode = 2;
void Mapi_Cfd_testvector1013(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 2;

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_SourceUserModeEn = 1;
    //0: auto
    //priority : infoFrame > driver > CFD default
    //1: force, refer to u8SourceUserMode

    //write
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_SourceUserMode = 2;
    //0: refer to infoFrame
    //1: refer to driver
    //2: refer to CFD default
}


//1014
//source auto
void Mapi_Cfd_testvector1014(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 2;

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetUserModeEn = 0;
    //0: auto
    //priority : infoFrame > driver > CFD default
    //1: force, refer to u8SourceUserMode

    //write
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetUserMode = 0;
    //0: refer to infoFrame
    //1: refer to driver
    //2: refer to CFD default
}

//1015
//source force mode = 0;
void Mapi_Cfd_testvector1015(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 2;

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetUserModeEn = 1;
    //0: auto
    //priority : infoFrame > driver > CFD default
    //1: force, refer to u8SourceUserMode

    //write
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetUserMode = 0;
    //0: refer to infoFrame
    //1: refer to driver
    //2: refer to CFD default
}

//1016
//source force mode = 1;
void Mapi_Cfd_testvector1016(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 2;

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetUserModeEn = 1;
    //0: auto
    //priority : infoFrame > driver > CFD default
    //1: force, refer to u8SourceUserMode

    //write
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetUserMode = 1;
    //0: refer to infoFrame
    //1: refer to driver
    //2: refer to CFD default
}

//1017
//source force mode = 2;
void Mapi_Cfd_testvector1017(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode = 2;

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetUserModeEn = 1;
    //0: auto
    //priority : infoFrame > driver > CFD default
    //1: force, refer to u8SourceUserMode

    //write
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetUserMode = 2;
    //0: refer to infoFrame
    //1: refer to driver
    //2: refer to CFD default
}
#endif

#if (NowHW == Kano)
//Kano000
void Mapi_Cfd_testvectorKano000(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Kano_TMOIP_Param.u8HDR_TMO_param_SetbyDriver = 1;
}

//0010
void Mapi_Cfd_testvectorKano0010(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector411(pstu_CfdAPI_Top_Param);
    Mhal_Cfd_MMin_AddMasteringSEI(pstu_CfdAPI_Top_Param);
    Mhal_Cfd_MMin_AddContentLightLevelSEI(pstu_CfdAPI_Top_Param);
    Mapi_Cfd_testvectorKano000(pstu_CfdAPI_Top_Param);

    //for source
    Mapi_Cfd_testvector1010(pstu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector1011(pstu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector1012(pstu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector1013(pstu_CfdAPI_Top_Param);

    //for target
    //Mapi_Cfd_testvector1014(pstu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector1015(pstu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector1016(pstu_CfdAPI_Top_Param);
    Mapi_Cfd_testvector1017(pstu_CfdAPI_Top_Param);

#define Kano0010 4

#if (Kano0010 == 1)
//case 1 ----------------------------------------------------------------------------------------------------
//max: 12525 cd/m^2 = code 255
//max ave : 3805 cd/m^2 = code 200
//min:  0.002 cd/m^2 = code 1

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance = 255;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = 200;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance = 1;
//------------------------------------------------------------------------------------------------------------
#elif (Kano0010 == 2)

//case 2 ----------------------------------------------------------------------------------------------------
//max: 1037.472 cd/m^2 = code 140
//max ave : 95.76 cd/m^2 = code 30
//min:  2.614 cd/m^2 = code 128

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance = 140;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = 30;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance = 128;
//------------------------------------------------------------------------------------------------------------

#elif (Kano0010 == 3)

//case 3 ----------------------------------------------------------------------------------------------------
//max: 51.095 cd/m^2 = code 1
//max ave : 817.518 cd/m^2 = code 129
//min:  0.314 cd/m^2 = code 200

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance = 1;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = 129;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance = 200;
//-----------------------------------------------------------------------------------------------------------

#elif (Kano0010 == 4)

//case 4 ----------------------------------------------------------------------------------------------------
//max: 672.717 cd/m^2 = code 120
//max ave : 118.921 cd/m^2 = code 40
//min:  0.662 cd/m^2 = code 80

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance = 120;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = 40;
pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance = 80;
//-----------------------------------------------------------------------------------------------------------

pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 7;

#endif
}

//0011
void Mapi_Cfd_testvectorKano0011(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector411(pstu_CfdAPI_Top_Param);
    //Mhal_Cfd_MMin_AddMasteringSEI(pstu_CfdAPI_Top_Param);
    Mhal_Cfd_MMin_AddContentLightLevelSEI(pstu_CfdAPI_Top_Param);
    Mapi_Cfd_testvectorKano000(pstu_CfdAPI_Top_Param);
    Mapi_Cfd_testvector1010(pstu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector1011(pstu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector1012(pstu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector1013(pstu_CfdAPI_Top_Param);

    Mapi_Cfd_testvector1014(pstu_CfdAPI_Top_Param);

    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 0;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance = 140;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = 30;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance = 128;

    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 7;
}

//0012
void Mapi_Cfd_testvectorKano0012(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    Mapi_Cfd_testvector411(pstu_CfdAPI_Top_Param);
    //Mhal_Cfd_MMin_AddMasteringSEI(pstu_CfdAPI_Top_Param);
    Mhal_Cfd_MMin_AddContentLightLevelSEI(pstu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvectorKano000(pstu_CfdAPI_Top_Param);
    pstu_CfdAPI_Top_Param->pstu_HW_IP_Param->pstu_TMO_Input->stu_Kano_TMOIP_Param.u8HDR_TMO_param_SetbyDriver = 0;
    Mapi_Cfd_testvector1010(pstu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector1011(pstu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector1012(pstu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector1013(pstu_CfdAPI_Top_Param);

    Mapi_Cfd_testvector1014(pstu_CfdAPI_Top_Param);
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid                                           = 0;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid                                                    = 1;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance = 140;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = 30;
    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance = 128;

    pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = 7;
}
#endif

void Mapi_Cfd_Cmodel_Debug_testvector(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source                                                              = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source                                                                   );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                              = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format                                                                 );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                          = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_DataFormat                                                             );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                         = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsFullRange                                                            );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                             = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode                                                                );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                         = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_IsRGBBypass                                                            );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode                                                           = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_SDRIPMode                                                              );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode                                                           = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode                                                              );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format                                           = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_Format                                              );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat                                       = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_DataFormat                                          );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange                                      = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_IsFullRange                                         );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode                                          = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->stu_Middle_Format[0].u8Mid_HDRMode                                             );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source                                                             = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source                                                              );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format                                                             = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Format                                                                );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                         = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_DataFormat                                                            );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange                                                        = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_IsFullRange                                                           );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode                                                            = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_HDRMode                                                               );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                             = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode                                                );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                       = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8HDMIOutput_GammutMapping_MethodMode                                          );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                            = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode                                               );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                            = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PanelOutput_GammutMapping_Mode                                               );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode                                                      = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8TMO_TargetRefer_Mode                                                         );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance                                                     = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Max_Luminance                                                        );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance                                                     = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u16Target_Min_Luminance                                                        );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status                                                            = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Process_Status                                                               );
printf("pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PredefinedProcess                            = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Main_Control->u8PredefinedProcess                                                               );

    if ((E_CFD_INPUT_SOURCE_STORAGE == pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source)||
            (E_CFD_INPUT_SOURCE_DTV == pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source)||
            (E_CFD_INPUT_SOURCE_JPEG == pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source))
    {
printf("//[STU_CFDAPI_MM_PARSER]                                                                                                   \n"                                                                                                                          );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                              = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Colour_primaries                                                                 );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                      = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Transfer_Characteristics                                                         );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                 = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Matrix_Coeffs                                                                    );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                         = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Video_Full_Range_Flag                                                            );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Max_Luminance                                                   = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Max_Luminance                                                      );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Min_Luminance                                                   = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->u32Master_Panel_Min_Luminance                                                      );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[0]                     = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[0]                        );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[1]                     = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[1]                        );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[2]                     = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[2]                        );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[0]                     = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[0]                        );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[1]                     = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[1]                        );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[2]                     = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[2]                        );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_x                              = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_x                                 );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_y                              = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_y                                 );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Mastering_Display_Infor_Valid                                                 = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->u8Mastering_Display_Infor_Valid                                                    );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->u8MM_HDR_ContentLightMetaData_Valid                        = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->u8MM_HDR_ContentLightMetaData_Valid     );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->u16Max_content_light_level                                                         = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->u16Max_content_light_level                            );
printf("pstu_CfdAPI_Top_Param->pstu_MM_Param->u16Max_pic_average_light_level                                                 = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_MM_Param->u16Max_pic_average_light_level                     );

    }

    if  (E_CFD_INPUT_SOURCE_HDMI == pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source)
    {
printf("//[STU_CFDAPI_HDMI_INFOFRAME_PARSER]                                                                                       \n"                                                                                                                          );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_HDR_InfoFrame_Valid                                    = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_HDR_InfoFrame_Valid                                       );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_EOTF                                                   = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_EOTF                                                      );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_SMD_ID                                                 = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_SMD_ID                                                    );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Max_Luminance                                       = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Max_Luminance                                          );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Min_Luminance                                       = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Master_Panel_Min_Luminance                                          );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Content_Light_Level                                          = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Content_Light_Level                                             );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Frame_Avg_Light_Level                                        = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u16Max_Frame_Avg_Light_Level                                           );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[0] = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[0]    );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[1] = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[1]    );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[2] = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[2]    );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[0] = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[0]    );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[1] = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[1]    );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[2] = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[2]    );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_x          = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_x             );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_y          = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_y             );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8Mastering_Display_Infor_Valid                                     = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8Mastering_Display_Infor_Valid                                        );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_Support_Format                                         = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_Support_Format                                            );
printf("pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_Colorspace                                             = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_InfoFrame_Param->u8HDMISource_Colorspace                                                );
    }

    if (E_CFD_OUTPUT_SOURCE_PANEL == pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source)
    {
    printf("\n"                                                                                                                          );
printf("//[STU_CFDAPI_PANEL_FORMAT]                                                                                                \n"                                                                                                                          );
printf("pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[0]                           = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[0]                              );
printf("pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[1]                           = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[1]                              );
printf("pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[2]                           = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_x[2]                              );
printf("pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[0]                           = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[0]                              );
printf("pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[1]                           = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[1]                              );
printf("pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[2]                           = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16Display_Primaries_y[2]                              );
printf("pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16White_point_x                                    = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16White_point_x                                       );
printf("pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16White_point_y                                    = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Panel_Param->stu_Cfd_Panel_ColorMetry.u16White_point_y                                       );
printf("pstu_CfdAPI_Top_Param->pstu_Panel_Param->u16Panel_Max_Luminance                                                       = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Panel_Param->u16Panel_Max_Luminance                                                          );
printf("pstu_CfdAPI_Top_Param->pstu_Panel_Param->u16Panel_Min_Luminance                                                       = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_Panel_Param->u16Panel_Min_Luminance                                                          );
}

    if (E_CFD_OUTPUT_SOURCE_HDMI == pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Output_Source)
    {
    printf("\n"                                                                                                                          );
    printf("//[STU_CFDAPI_HDMI_EDID_PARSER]                                                                                                \n"                                                                                                                          );
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Valid);
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EOTF);
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_SM = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_SM);
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Luminance);
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance);
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Desired_Content_Min_Luminance);
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_HDRData_Block_Length);
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u16Display_Primaries_x[0] = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[0]  );
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u16Display_Primaries_x[1] = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[1]  );
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u16Display_Primaries_x[2] = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[2]  );
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u16Display_Primaries_y[0] = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[0]  );
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u16Display_Primaries_y[1] = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[1]  );
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u16Display_Primaries_y[2] = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[2]  );
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u16White_point_x = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16White_point_x  );
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u16White_point_y = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->stu_Cfd_HDMISink_Panel_ColorMetry.u16White_point_y  );
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_base_block_version = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_base_block_version      );
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_base_block_reversion = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_base_block_reversion    );
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_CEA_block_reversion = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_CEA_block_reversion     );

    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_VCDB_Valid = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_VCDB_Valid     );
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Support_YUVFormat = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Support_YUVFormat     );
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_Extended_Colorspace     );
    printf("pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_HDMI_EDID_Param->u8HDMISink_EDID_Valid     );
    }

    printf("\n");
    //printf("//[STU_CFDAPI_HDMI_EDID_PARSER] \n");
    printf("pstu_CfdAPI_Top_Param->pstu_OSD_Param->u8HDR_UI_H2SMode = %d; \n" ,pstu_CfdAPI_Top_Param->pstu_OSD_Param->u8HDR_UI_H2SMode );
    printf("\n");

    printf("\n");
    printf("//[STU_CFDAPI_TMOIP]\n");

}

//for Dolby input case
void Mapi_Cfd_Dolby_Input_Force(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    //pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format = E_CFD_CFIO_DOLBY_HDR_TEMP;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRMode = E_CFIO_MODE_HDR1;
    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_HDRIPMode = 3;
}

//for MM input case
void Mapi_Cfd_MM_Input_Force(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{

    pstu_CfdAPI_Top_Param->pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode = 1;
}

//for CFD input auto
void Mapi_Cfd_Input_Force(STU_CFDAPI_TOP_CONTROL *pstu_CfdAPI_Top_Param)
{
    if (E_CFD_CFIO_DOLBY_HDR_TEMP == pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Format)
    {
        Mapi_Cfd_Dolby_Input_Force(pstu_CfdAPI_Top_Param);
    }

    if ((E_CFD_INPUT_SOURCE_STORAGE == pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source)||
        (E_CFD_INPUT_SOURCE_DTV == pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source)||
        (E_CFD_INPUT_SOURCE_JPEG == pstu_CfdAPI_Top_Param->pstu_Main_Control->u8Input_Source))
    {
        Mapi_Cfd_MM_Input_Force(pstu_CfdAPI_Top_Param);
    }



}

void Mapi_Cfd_inter_Main_Control_Param_Init(STU_CFDAPI_MAIN_CONTROL *pstControlParamInit )
{

    //0: color format driver off
    //1: color format dirver on - normal mode
    //2: color format driver on - test mode
    //pstControlParamInit->u8Process_Mode = 0;
    //pstControlParamInit->u8Process_Mode = 1;

    //E_CFD_MC_HW_STRUCTURE
    //pstControlParamInit->u8HW_Structure = E_CFD_HWS_TV_TYPE0;

    //Patch for Kano
    //pstControlParamInit->u8HW_PatchEn = 1;

    //E_CFD_MC_SOURCE
    pstControlParamInit->u8Input_Source = E_CFD_INPUT_SOURCE_STORAGE;
    //pstControlParamInit->u8Input_Source = E_CFD_INPUT_SOURCE_HDMI;
    //pstControlParamInit->u8Input_Source = E_CFD_MC_SOURCE_ANALOG;

    //E_CFD_INPUT_ANALOG_FORMAT
    //pstControlParamInit->u8Input_AnalogIdx = E_CFD_INPUT_ANALOG_RF_PAL_60;

    //E_CFD_CFIO
    pstControlParamInit->u8Input_Format = E_CFD_CFIO_YUV_BT709;

    //E_CFD_MC_FORMAT
    pstControlParamInit->u8Input_DataFormat = E_CFD_MC_FORMAT_YUV444;
    //pstControlParamInit->u8Input_DataFormat = E_CFD_MC_FORMAT_RGB;

    //E_CFD_CFIO_RANGE
    //0:limit 1:full
    pstControlParamInit->u8Input_IsFullRange = E_CFD_CFIO_RANGE_LIMIT;

    //E_CFIO_HDR_STATUS
    //0:SDR 1:HDR1 2:HDR2
    pstControlParamInit->u8Input_HDRMode = E_CFIO_MODE_SDR;

    //0:no R2Y for RGB in
    //1:R2Y for RGB in
    pstControlParamInit->u8Input_IsRGBBypass = 0;

    //new controls in Maserati
    //pstControlParamInit->u8IP2CSC_Bypass = 0;
    pstControlParamInit->u8Input_SDRIPMode = 1;
    pstControlParamInit->u8Input_HDRIPMode = 1;

    //pstControlParamInit->u8IP2CSC_Mode = E_CFD_IP_CSC_OFF;

    //pstControlParamInit->u8Middle_Format_num = 1;
    pstControlParamInit->stu_Middle_Format[0].u8Mid_Format_Mode = 0;
    pstControlParamInit->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709;
    pstControlParamInit->stu_Middle_Format[0].u8Mid_IsFullRange = E_CFD_CFIO_RANGE_FULL;
    pstControlParamInit->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV444;
    pstControlParamInit->stu_Middle_Format[0].u8Mid_HDRMode = E_CFIO_MODE_SDR;

    //E_CFD_MC_SOURCE
    pstControlParamInit->u8Output_Source = E_CFD_OUTPUT_SOURCE_HDMI;
//  pstControlParamInit->u8Output_Source = E_CFD_OUTPUT_SOURCE_PANEL;

    //E_CFD_CFIO
    pstControlParamInit->u8Output_Format = E_CFD_CFIO_YUV_BT709;

    //E_CFD_MC_FORMAT
    pstControlParamInit->u8Output_DataFormat = E_CFD_MC_FORMAT_YUV444;
    //pstControlParamInit->u8Output_DataFormat = E_CFD_MC_FORMAT_RGB;

    //E_CFD_CFIO_RANGE
    //0:limit 1:full
    pstControlParamInit->u8Output_IsFullRange = E_CFD_CFIO_RANGE_LIMIT;

    //E_CFIO_HDR_STATUS
    //0:SDR 1:HDR1 2:HDR2
    pstControlParamInit->u8Output_HDRMode = E_CFIO_MODE_SDR;

    //pstControlParamInit->u8Input_Format_HDMI_CE_Flag = 0;
    //pstControlParamInit->u8Output_Format_HDMI_CE_Flag = 0;

    pstControlParamInit->u8HDMIOutput_GammutMapping_Mode = 1;
    pstControlParamInit->u8HDMIOutput_GammutMapping_MethodMode = 0;
    pstControlParamInit->u8MMInput_ColorimetryHandle_Mode = 1;
    pstControlParamInit->u8PanelOutput_GammutMapping_Mode = 1;

    //pstControlParamInit->u8TMO_Mode = E_CFD_MC_MODE_BYPASS;
    //pstControlParamInit->u8HDR2SDR_Mode = E_CFD_MC_MODE_NORMAL;
    //pstControlParamInit->u8SDR_Mode = E_CFD_MC_MODE_NORMAL;

    //pstControlParamInit->u8TMO_TestMode_Content = 0;
    //pstControlParamInit->u8HDR2SDR_TestMode_Content = 0;
    //pstControlParamInit->u8SDR_TestMode_Content = 0;

    //TMO controls
    //pstControlParamInit->u8TMO_UseMasteringPanelInfor_Flag = 1;

    //1:use MM/HDMI infoFrame Mastering infor for source Luminance range
    //if not exist , the same value as 0
    //pstControlParamInit->u8TMO_SourceInforMode = 1;

    //0:use default value
    //use u16Source_Max_Luminance,u16Source_Min_Luminance,u16Source_Mean_Luminance

    //1:use Panel/HDMI EDID infor for target Luminance range
    //if not exist , the same value as 0
    pstControlParamInit->u8TMO_TargetRefer_Mode = 1;

    //range 1 nits to 10000 nits
    pstControlParamInit->u16Source_Max_Luminance = 4000;    //data * 1 nits
    pstControlParamInit->u16Source_Med_Luminance = 120;   //data * 1 nits

    //range 1e-4 nits to 6.55535 nits
    pstControlParamInit->u16Source_Min_Luminance = 60;    //data * 0.0001 nits

    //range 1 nits to 10000 nits
    pstControlParamInit->u16Target_Max_Luminance = 600;    //data * 1 nits
    pstControlParamInit->u16Target_Med_Luminance = 125;   //data * 1 nits

    //range 1e-4 nits to 6.55535 nits
    pstControlParamInit->u16Target_Min_Luminance = 500;    //data * 0.0001 nits

    //
    pstControlParamInit->u8Process_Status = 0;
    pstControlParamInit->u8Process_Status2 = 0;
    pstControlParamInit->u8Process_Status3 = 0;

    pstControlParamInit->u8Input_ext_Colour_primaries = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
    pstControlParamInit->u8Input_ext_Transfer_Characteristics = E_CFD_CFIO_TR_BT709;
    pstControlParamInit->u8Input_ext_Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709;

    pstControlParamInit->u8PredefinedProcess = 0;

}

void Mapi_Cfd_inter_MM_Param_Init(STU_CFDAPI_MM_PARSER *pstMMParamInita)
{

    //E_CFD_MM_CODEC
    //pstMMParamInita->u8MM_Codec = E_CFD_MM_CODEC_OTHERS;
    //pstMMParamInita->u8MM_Codec = 3;

    //E_CFD_VALIDORNOT
    //pstMMParamInita->u8ColorDescription_Valid = E_CFD_VALID;

    //BT709
    pstMMParamInita->u8Colour_primaries = E_CFD_CFIO_CP_BT709_SRGB_SYCC;

    //BT709
    pstMMParamInita->u8Transfer_Characteristics = E_CFD_CFIO_TR_BT709;

    //BT709
    pstMMParamInita->u8Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709;

    //range
    pstMMParamInita->u8Video_Full_Range_Flag = 0;

    //SEI from HEVC
    //data = code * 0.0001 nits
    pstMMParamInita->u32Master_Panel_Max_Luminance = 40000000;
    //data = code * 0.0001 nits
    pstMMParamInita->u32Master_Panel_Min_Luminance = 500;
    //data * 1 nits
    //pstMMParamInita->u16Max_Content_Light_Level = 0;
    //data * 1 nits
    //pstMMParamInita->u16Max_Frame_Avg_Light_Level = 0;

    //order R->G->B
    //data = code*0.00002 0xC350 = 1
    //BT709
    pstMMParamInita->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[0] = 0x7D00;  //0.64
    pstMMParamInita->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[1] = 0x3A98;  //0.3
    pstMMParamInita->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[2] = 0x1D4C;  //0.15

    pstMMParamInita->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[0] = 0x4074;  //0.33
    pstMMParamInita->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[1] = 0x7530;  //0.6
    pstMMParamInita->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[2] = 0x0BB8;  //0.06

    //D65
    //data *0.00002 0xC350 = 1
    //x 0.3127
    //y 0.3290
    pstMMParamInita->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_x = 0x3D13;  //0.3127
    pstMMParamInita->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_y = 0x4042;  //0.3290

    pstMMParamInita->u8Mastering_Display_Infor_Valid = E_CFD_NOT_VALID;

    pstMMParamInita->u16Max_content_light_level = 300;    //data * 1 nits
    pstMMParamInita->u16Max_pic_average_light_level = 100;    //data * 1 nits

}

void Mapi_Cfd_inter_HDMI_InfoFrame_Param_Init(STU_CFDAPI_HDMI_INFOFRAME_PARSER *pstHDMI_InfoFrameParamInit)
{

    pstHDMI_InfoFrameParamInit->u8HDMISource_HDR_InfoFrame_Valid = E_CFD_NOT_VALID;
    //assign by E_CFD_VALIDORNOT

    pstHDMI_InfoFrameParamInit->u8HDMISource_EOTF = E_CFD_HDMI_EOTF_SMPTE2084;
    //assign by E_CFD_HDMI_HDR_INFOFRAME_EOTF

    pstHDMI_InfoFrameParamInit->u8HDMISource_SMD_ID = E_CFD_HDMI_META_TYPE1;
    //assign by E_CFD_HDMI_HDR_INFOFRAME_METADATA

    pstHDMI_InfoFrameParamInit->u16Master_Panel_Max_Luminance = 4000;
    //data * 1 nits

    pstHDMI_InfoFrameParamInit->u16Master_Panel_Min_Luminance = 500;
    //data * 0.0001 nits

    pstHDMI_InfoFrameParamInit->u16Max_Content_Light_Level = 600;
    //data * 1 nits

    pstHDMI_InfoFrameParamInit->u16Max_Frame_Avg_Light_Level = 128;
    //data * 1 nits

    //order R->G->B
    //BT709
    //data *0.00002 0xC350 = 1
    pstHDMI_InfoFrameParamInit->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[0] = 0x7D00; //0.64
    pstHDMI_InfoFrameParamInit->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[1] = 0x3A98; //0.3
    pstHDMI_InfoFrameParamInit->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[2] = 0x1D4C; //0.15

    pstHDMI_InfoFrameParamInit->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[0] = 0x4047; //0.33
    pstHDMI_InfoFrameParamInit->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[1] = 0x7530; //0.6
    pstHDMI_InfoFrameParamInit->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[2] = 0x0BB8; //0.06

    pstHDMI_InfoFrameParamInit->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_x = 0x3D13; //0.3127
    pstHDMI_InfoFrameParamInit->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_y = 0x4042; //0.3290

    pstHDMI_InfoFrameParamInit->u8Mastering_Display_Infor_Valid = E_CFD_NOT_VALID;
    //assign by E_CFD_VALIDORNOT

    pstHDMI_InfoFrameParamInit->u8HDMISource_Support_Format = 0;
    //MS_U8 u8HDMISource_Support_Format;
    //[2:0] = {Y2 Y1 Y0}
    //[4:3] = {YQ1 YQ0}
    //[6:5] = {Q1 Q0}
    //information in AVI infoFrame

    pstHDMI_InfoFrameParamInit->u8HDMISource_Colorspace = E_CFD_CFIO_YUV_BT709;
    //assign by E_CFD_CFIO

    //pstHDMI_InfoFrameParamInit->u8HDMISource_AVIInfoFrame_Valid = E_CFD_VALID;

}

void Mapi_Cfd_inter_HDMI_EDID_Param_Init(STU_CFDAPI_HDMI_EDID_PARSER *pstHDMI_EDIDParamInita)
{

    MS_U8 u8temp = 0;
    MS_U8 u8unit_flag = 0;
    MS_U8 *pu8;
    pu8 = &u8unit_flag;
    MS_U16 u16nits = 0;

    pstHDMI_EDIDParamInita->u8HDMISink_HDRData_Block_Valid = E_CFD_NOT_VALID;
    //assign by E_CFD_VALIDORNOT

    pstHDMI_EDIDParamInita->u8HDMISink_EOTF = 1;
    pstHDMI_EDIDParamInita->u8HDMISink_SM   = 1;

    //need LUT_CFD_CV1 to transfer
    //assume the range of max is [1~1e4] in nits
    //Desired_Content_Max_Luminance = 617 nits
    u16nits = 617;
    u8temp = MS_Cfd_nits2code_max_EDID(u16nits,LUT_CFD_CV1);
    //u8temp = 116
    pstHDMI_EDIDParamInita->u8HDMISink_Desired_Content_Max_Luminance = u8temp;

    //need LUT_CFD_CV1 to transfer
    //Desired_Content_Max_Luminance = 130 nits
    u8temp = MS_Cfd_nits2code_max_EDID(130,LUT_CFD_CV1);
    //u8temp = 44
    pstHDMI_EDIDParamInita->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance = u8temp;

    //MS_U8 MS_Cfd_nits2code_min_EDID(MS_U16 u16Nits, MS_U32 *nits_table, MS_U16 U16_max_luminance, MS_U8 *pu8DataRange_flag)
    //Desired_Content_Min_Luminance = 0.05 nits = 500*0.0001 nits
    //unit = 1e-4 nits
    u8unit_flag = 1;
    //for CFD , use u8unit_flag to indicate the basic unit of min luminance data.
    u8temp = MS_Cfd_nits2code_min_EDID(500, LUT_CFD_CV2, u16nits, pu8);
    //u8temp = 22
    pstHDMI_EDIDParamInita->u8HDMISink_Desired_Content_Min_Luminance = u8temp;

    pstHDMI_EDIDParamInita->u8HDMISink_HDRData_Block_Length = 7;

    //BT709
    //MS_U16 u16display_primaries_x[3]; //data *1/1024 0x03FF = 0.999
    pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[0] = 0x28f;     //0x28f = 655, 655/1024 = 0.64
    pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[1] = 0x133;     //0x133 = 307, 307/1024 = 0.3
    pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[2] = 0x9A;      //0x9a  = 154, 154/1024 = 0.15
    //pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[2] = 0x3ff;      //0x9a  = 154, 154/1024 = 0.15

    pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[0] = 0x152;    //0x152 = 338, 338/1024 = 0.33
    pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[1] = 0x266;    //0x266 = 614, 614/1024 = 0.6
    pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[2] = 0x3D;     //0x3D  = 61,  61/1024 = 0.06

    pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry.u16White_point_x = 0x140;    //0x140  = 320, 320/1024 = 0.3127
    pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry.u16White_point_y = 0x151;    //0x151  = 337,  337/1024 = 0.3290

    pstHDMI_EDIDParamInita->u8HDMISink_EDID_base_block_version = 0x01;
    pstHDMI_EDIDParamInita->u8HDMISink_EDID_base_block_reversion = 0x03;
    pstHDMI_EDIDParamInita->u8HDMISink_EDID_CEA_block_reversion = 0x03;

    pstHDMI_EDIDParamInita->u8HDMISink_VCDB_Valid = 0x01;
    //bit 0:VCDB is not avaliable
    //bit 1:VCDB is avaliable

    pstHDMI_EDIDParamInita->u8HDMISink_Support_YUVFormat = 0x1f;
    //bit 0:Support_YUV444
    //bit 1:Support_YUV422
    //bit 2:Support_YUV420
    //bit 3:Y_quantization_range   0:no data(due to CE or IT video) ; 1:selectable
    //bit 4:RGB_quantization_range 0:no data(due to CE or IT video) ; 1:selectable

    pstHDMI_EDIDParamInita->u8HDMISink_Extended_Colorspace = 0xff;
    //bit 0:Support_xvYCC601
    //bit 1:Support_xvYCC709
    //bit 2:Support_sYCC601
    //bit 3:Support_AdobeYCC601
    //bit 4:Support_AdobeRGB
    //bit 5:Support_BT2020cYCC
    //bit 6:Support_BT2020YCC
    //bit 7:Support_BT2020RGB

    pstHDMI_EDIDParamInita->u8HDMISink_EDID_Valid = E_CFD_VALID;


}

MS_U16 Mapi_Cfd_inter_TOP_Param_CrossCheck(STU_CFDAPI_TOP_CONTROL *pstu_Cfd_api_top)
{

    MS_U16 u16_check_status = E_CFD_MC_ERR_NOERR;

    STU_CFDAPI_MAIN_CONTROL                     *pstu_Control_Param;
    STU_CFDAPI_MM_PARSER                        *pstu_MM_Param;
    STU_CFDAPI_HDMI_EDID_PARSER                 *pstu_HDMI_EDID_Param;
    STU_CFDAPI_HDMI_INFOFRAME_PARSER            *pstu_HDMI_InfoFrame_Param;
    STU_CFDAPI_PANEL_FORMAT                     *pstu_Panel_Param;
    STU_CFDAPI_OSD_CONTROL                      *pstu_OSD_Param;

    pstu_Control_Param                          = pstu_Cfd_api_top->pstu_Main_Control;
    pstu_MM_Param                               = pstu_Cfd_api_top->pstu_MM_Param;
    pstu_HDMI_EDID_Param                        = pstu_Cfd_api_top->pstu_HDMI_EDID_Param;
    pstu_HDMI_InfoFrame_Param                   = pstu_Cfd_api_top->pstu_HDMI_InfoFrame_Param;
    pstu_Panel_Param                            = pstu_Cfd_api_top->pstu_Panel_Param;
    pstu_OSD_Param                              = pstu_Cfd_api_top->pstu_OSD_Param;

    //MM
    if ((E_CFD_INPUT_SOURCE_STORAGE == pstu_Cfd_api_top->pstu_Main_Control->u8Input_Source)||
        (E_CFD_INPUT_SOURCE_DTV == pstu_Cfd_api_top->pstu_Main_Control->u8Input_Source)||
        (E_CFD_INPUT_SOURCE_JPEG == pstu_Cfd_api_top->pstu_Main_Control->u8Input_Source))
    {
        if ((E_CFD_CFIO_TR_SMPTE2084 == pstu_MM_Param->u8Transfer_Characteristics) && (E_CFIO_MODE_SDR == pstu_Control_Param->u8Input_HDRMode))
        {
            u16_check_status = E_CFD_MC_ERR_INPUT_MAIN_CONTROLS_MM_INTERRELATION;
            HDR_DBG_HAL_CFD(printk("\n  ERROR:u8Transfer_Characteristics = 2084 ,but u8Input_HDRMode = SDR \n"));

            pstu_Control_Param->u8Input_HDRMode = E_CFIO_MODE_HDR2;
            HDR_DBG_HAL_CFD(printk("\n  force MM u8Input_HDRMode to E_CFIO_MODE_HDR2\n"));
            //pstu_MM_Param->u8Transfer_Characteristics = E_CFD_CFIO_TR_BT709;
            //HDR_DBG_HAL_CFD(printk("\n  force MM u8Transfer_Characteristics to E_CFD_CFIO_TR_BT709\n"));
        }

        if ((E_CFD_CFIO_TR_SMPTE2084 != pstu_MM_Param->u8Transfer_Characteristics) && (E_CFIO_MODE_HDR2 == pstu_Control_Param->u8Input_HDRMode))
        {
            u16_check_status = E_CFD_MC_ERR_INPUT_MAIN_CONTROLS_MM_INTERRELATION;
            HDR_DBG_HAL_CFD(printk("\n  ERROR:u8Transfer_Characteristics is not 2084 ,but u8Input_HDRMode = openHDR \n"));

            pstu_Control_Param->u8Input_HDRMode = E_CFIO_MODE_SDR;
            HDR_DBG_HAL_CFD(printk("\n  force MM u8Input_HDRMode to E_CFIO_MODE_SDR\n"));

            //pstu_MM_Param->u8Transfer_Characteristics = E_CFD_CFIO_TR_SMPTE2084;
            //HDR_DBG_HAL_CFD(printk("\n  force MM u8Transfer_Characteristics to E_CFD_CFIO_TR_SMPTE2084\n"));
        }

            if ((E_CFD_CFIO_TR_HLG == pstu_MM_Param->u8Transfer_Characteristics) && (E_CFIO_MODE_SDR == pstu_Control_Param->u8Input_HDRMode))
        {
            u16_check_status = E_CFD_MC_ERR_INPUT_MAIN_CONTROLS_MM_INTERRELATION;
            HDR_DBG_HAL_CFD(printk("\n  ERROR:u8Transfer_Characteristics = HLG ,but u8Input_HDRMode = SDR \n"));

            pstu_Control_Param->u8Input_HDRMode = E_CFIO_MODE_HDR3;
            HDR_DBG_HAL_CFD(printk("\n  force MM u8Input_HDRMode to E_CFIO_MODE_HDR3\n"));
        }

        if ((E_CFD_CFIO_TR_HLG != pstu_MM_Param->u8Transfer_Characteristics) && (E_CFIO_MODE_HDR3 == pstu_Control_Param->u8Input_HDRMode))
        {
            u16_check_status = E_CFD_MC_ERR_INPUT_MAIN_CONTROLS_MM_INTERRELATION;
            HDR_DBG_HAL_CFD(printk("\n  ERROR:u8Transfer_Characteristics is not HLG ,but u8Input_HDRMode = openHDR \n"));

            pstu_Control_Param->u8Input_HDRMode = E_CFIO_MODE_SDR;
            HDR_DBG_HAL_CFD(printk("\n  force MM u8Input_HDRMode to E_CFIO_MODE_SDR\n"));

            //pstu_MM_Param->u8Transfer_Characteristics = E_CFD_CFIO_TR_SMPTE2084;
            //HDR_DBG_HAL_CFD(printk("\n  force MM u8Transfer_Characteristics to E_CFD_CFIO_TR_SMPTE2084\n"));
        }

    }
    //HDMI
    else if (E_CFD_INPUT_SOURCE_HDMI == pstu_Cfd_api_top->pstu_Main_Control->u8Input_Source)
    {
        if (E_CFD_VALID == pstu_HDMI_InfoFrame_Param->u8HDMISource_HDR_InfoFrame_Valid)
        {
        if ((E_CFD_HDMI_EOTF_SMPTE2084 == pstu_HDMI_InfoFrame_Param->u8HDMISource_EOTF) && (E_CFIO_MODE_SDR == pstu_Control_Param->u8Input_HDRMode))
        {
            u16_check_status = E_CFD_MC_ERR_INPUT_MAIN_CONTROLS_HDMI_INTERRELATION;
            HDR_DBG_HAL_CFD(printk("\n  ERROR:u8HDMISource_EOTF = E_CFD_HDMI_EOTF_SMPTE2084 ,but u8Input_HDRMode = SDR \n"));

            pstu_Control_Param->u8Input_HDRMode = E_CFIO_MODE_HDR2;
            HDR_DBG_HAL_CFD(printk("\n  force MM u8Input_HDRMode to E_CFIO_MODE_HDR2\n"));
            //pstu_MM_Param->u8Transfer_Characteristics = E_CFD_CFIO_TR_BT709;
            //HDR_DBG_HAL_CFD(printk("\n  force MM u8Transfer_Characteristics to E_CFD_CFIO_TR_BT709\n"));
        }

        if ((E_CFD_HDMI_EOTF_SMPTE2084 != pstu_HDMI_InfoFrame_Param->u8HDMISource_EOTF) && (E_CFIO_MODE_HDR2 == pstu_Control_Param->u8Input_HDRMode))
        {
            u16_check_status = E_CFD_MC_ERR_INPUT_MAIN_CONTROLS_MM_INTERRELATION;
            HDR_DBG_HAL_CFD(printk("\n  ERROR:u8HDMISource_EOTF is not E_CFD_HDMI_EOTF_SMPTE2084 ,but u8Input_HDRMode = openHDR \n"));

            pstu_Control_Param->u8Input_HDRMode = E_CFIO_MODE_SDR;
            HDR_DBG_HAL_CFD(printk("\n  force MM u8Input_HDRMode to E_CFIO_MODE_SDR\n"));

            //pstu_MM_Param->u8Transfer_Characteristics = E_CFD_CFIO_TR_SMPTE2084;
            //HDR_DBG_HAL_CFD(printk("\n  force MM u8Transfer_Characteristics to E_CFD_CFIO_TR_SMPTE2084\n"));
        }
    }
    }
    else
    {
        //do nothing
    }

    if (E_CFD_OUTPUT_SOURCE_PANEL != pstu_Control_Param->u8Output_Source)
    {
        if ((0 == pstu_Control_Param->u8Input_SDRIPMode)||(2 == pstu_Control_Param->u8Input_SDRIPMode))
        {
            u16_check_status = E_CFD_MC_ERR_OUTPUT_PANEL_SDRIPMODE_INTERRELATION;
            HDR_DBG_HAL_CFD(printk("\n  ERROR:u8Input_SDRIPMode mode 0 and 2 are only for Panel out \n"));
            pstu_Control_Param->u8Input_SDRIPMode = 1;
            HDR_DBG_HAL_CFD(printk("\n  force u8Input_SDRIPMode to 1\n"));
        }
    }
    else
    {
        //do nothing
    }

    if (E_CFD_MC_ERR_NOERR != u16_check_status)
    {
        //HDR_DBG_HAL_CFD(printk("\nError:please check some parameters in Mapi_Cfd_inter_TOP_Param_CrossCheck is wrong, but has been force to some value\n"));
        HDR_DBG_HAL_CFD(printk("Error code = %04x !!!, error is in [ %s  , %d]\n",u16_check_status, __FUNCTION__,__LINE__));
    }

    return u16_check_status;

}

MS_U16 Mapi_Cfd_inter_Main_Control_Param_Check(STU_CFDAPI_MAIN_CONTROL *pstControlParamInit )
{

    MS_U16 u16_check_status = E_CFD_MC_ERR_NOERR;
    MS_U8 u8_check_status2 = E_CFD_MC_ERR_NOERR;
    MS_U16 u16Temp;

    //MS_U16 u2;

    HDR_DBG_HAL_CFD(printk("\n  MC  STU_CFD_MAIN_CONTROL:    \n"));
    HDR_DBG_HAL_CFD(printk("\n  MC  version=%d    \n",                      pstControlParamInit->u32Version ));
    HDR_DBG_HAL_CFD(printk("\n  MC  length=%d    \n",                       pstControlParamInit->u16Length ));

    HDR_DBG_HAL_CFD(printk("\n  MC  u8Input_Source=%d    \n",               pstControlParamInit->u8Input_Source ));
    //HDR_DBG_HAL_CFD(printk("\n  MC  u8Input_AnalogIdx=%d    \n",          pstControlParamInit->u8Input_AnalogIdx ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8Input_Format=%d    \n",               pstControlParamInit->u8Input_Format ));

    HDR_DBG_HAL_CFD(printk("\n  MC  u8Input_DataFormat=%d    \n",           pstControlParamInit->u8Input_DataFormat ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8Input_IsFullRange=%d    \n",          pstControlParamInit->u8Input_IsFullRange ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8Input_HDRMode=%d    \n",              pstControlParamInit->u8Input_HDRMode ));

    HDR_DBG_HAL_CFD(printk("\n  MC  u8Input_IsRGBBypass=%d    \n",          pstControlParamInit->u8Input_IsRGBBypass ));


    HDR_DBG_HAL_CFD(printk("\n  MC  u8Mid_Format_Mode[0]=%d    \n",         pstControlParamInit->stu_Middle_Format[0].u8Mid_Format_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8Mid_Format[0]=%d    \n",              pstControlParamInit->stu_Middle_Format[0].u8Mid_Format ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8Mid_DataFormat[0]=%d    \n",          pstControlParamInit->stu_Middle_Format[0].u8Mid_DataFormat ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8Mid_IsFullRange[0]=%d    \n",         pstControlParamInit->stu_Middle_Format[0].u8Mid_IsFullRange ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8Mid_HDRMode[0]=%d    \n",             pstControlParamInit->stu_Middle_Format[0].u8Mid_HDRMode ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8Mid_Colour_primaries[0]=%d    \n",    pstControlParamInit->stu_Middle_Format[0].u8Mid_Colour_primaries ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8Mid_Transfer_Characteristics[0]=%d    \n",        pstControlParamInit->stu_Middle_Format[0].u8Mid_Transfer_Characteristics ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8Mid_Matrix_Coeffs[0]=%d    \n",       pstControlParamInit->stu_Middle_Format[0].u8Mid_Matrix_Coeffs ));


    //lack two bypass controls
    HDR_DBG_HAL_CFD(printk("\n  MC  u8Input_SDRIPMode=%d    \n",        pstControlParamInit->u8Input_SDRIPMode ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8Input_HDRIPMode=%d    \n",        pstControlParamInit->u8Input_HDRIPMode ));

    HDR_DBG_HAL_CFD(printk("\n  MC  u8Output_Source=%d    \n",              pstControlParamInit->u8Output_Source ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8Output_Format=%d    \n",              pstControlParamInit->u8Output_Format ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8Output_DataFormat=%d    \n",          pstControlParamInit->u8Output_DataFormat ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8Output_IsFullRange=%d    \n",         pstControlParamInit->u8Output_IsFullRange ));

    HDR_DBG_HAL_CFD(printk("\n  MC  u8Output_HDRMode=%d    \n",             pstControlParamInit->u8Output_HDRMode ));

    HDR_DBG_HAL_CFD(printk("\n  MC  u8HDMIOutput_GammutMapping_Mode=%d    \n",            pstControlParamInit->u8HDMIOutput_GammutMapping_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8HDMIOutput_GammutMapping_MethodMode=%d    \n",        pstControlParamInit->u8HDMIOutput_GammutMapping_MethodMode ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8MMInput_ColorimetryHandle_Mode=%d    \n",     pstControlParamInit->u8MMInput_ColorimetryHandle_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8PanelOutput_GammutMapping_Mode=%d    \n",     pstControlParamInit->u8PanelOutput_GammutMapping_Mode ));


    HDR_DBG_HAL_CFD(printk("\n  MC  u8TMO_TargetRefer_Mode=%d    \n",           pstControlParamInit->u8TMO_TargetRefer_Mode ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u16Source_Max_Luminance=%d    \n",          pstControlParamInit->u16Source_Max_Luminance ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u16Source_Med_Luminance=%d    \n",          pstControlParamInit->u16Source_Med_Luminance ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u16Source_Min_Luminance=%d    \n",          pstControlParamInit->u16Source_Min_Luminance ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u16Target_Max_Luminance=%d    \n",          pstControlParamInit->u16Target_Max_Luminance ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u16Target_Med_Luminance=%d    \n",          pstControlParamInit->u16Target_Med_Luminance ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u16Target_Min_Luminance=%d    \n",          pstControlParamInit->u16Target_Min_Luminance ));

    HDR_DBG_HAL_CFD(printk("\n  MC  u8Input_ext_Colour_primaries=%d    \n",         pstControlParamInit->u8Input_ext_Colour_primaries ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8Input_ext_Transfer_Characteristics=%d    \n",         pstControlParamInit->u8Input_ext_Transfer_Characteristics ));
    HDR_DBG_HAL_CFD(printk("\n  MC  u8Input_ext_Matrix_Coeffs=%d    \n",            pstControlParamInit->u8Input_ext_Matrix_Coeffs ));

    HDR_DBG_HAL_CFD(printk("\n  MC  u8PredefinedProcess=%d    \n",            pstControlParamInit->u8PredefinedProcess ));

#if 0
    if ((pstControlParamInit->u8Process_Mode) > 2)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8HW_Structure is not correct \n"));
        u8_check_status = FALSE;
    }


    if ((pstControlParamInit->u8HW_Structure) > E_CFD_HWS_EMUEND)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8HW_Structure is not correct \n"));
        u8_check_status = FALSE;
    }

    if ((pstControlParamInit->u8Input_Source) == E_CFD_OUTPUT_SOURCE_PANEL)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl E_CFD_OUTPUT_SOURCE_PANEL is not correct \n"));
        u16_check_status = E_CFD_MC_ERR_INPUT_SOURCE;
        pstControlParamInit->u8Input_Source = E_CFD_INPUT_SOURCE_HDMI;
    }
#endif

    if ((pstControlParamInit->u8Input_Source) >= E_CFD_INPUT_SOURCE_RESERVED_START)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl current u8Input_Source is reserved \n"));
        u16_check_status = E_CFD_MC_ERR_INPUT_SOURCE;
        pstControlParamInit->u8Input_Source = E_CFD_INPUT_SOURCE_HDMI;
    }

#if 0
    if ((pstControlParamInit->u8Input_AnalogIdx) >= E_CFD_INPUT_ANALOG_RESERVED_START)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl current u8Input_AnalogIdx is reserved \n"));
        u16_check_status = E_CFD_MC_ERR_INPUT_ANALOGIDX;
        pstControlParamInit->u8Input_AnalogIdx = E_CFD_INPUT_ANALOG_RF_NTSC_44;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_AnalogIdx to E_CFD_INPUT_ANALOG_RF_NTSC_44\n"));
    }
#endif

    if ((pstControlParamInit->u8Input_Format) >= E_CFD_CFIO_RESERVED_START)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Input_Format is reserved \n"));
        u16_check_status = E_CFD_MC_ERR_INPUT_FORMAT;
        pstControlParamInit->u8Input_Format = E_CFD_CFIO_YUV_BT709;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_Format to E_CFD_CFIO_YUV_BT709\n"));
    }

    if ((pstControlParamInit->u8Input_DataFormat) >= E_CFD_MC_FORMAT_RESERVED_START)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Input_DataFormat is reserved \n"));
        u16_check_status = E_CFD_MC_ERR_INPUT_DATAFORMAT;
        pstControlParamInit->u8Input_DataFormat = E_CFD_MC_FORMAT_YUV422;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_DataFormat to E_CFD_MC_FORMAT_YUV422\n"));
    }

    if ((pstControlParamInit->u8Input_IsFullRange) >= E_CFD_CFIO_RANGE_RESERVED_START)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Input_IsFullRange is reserved \n"));
        u16_check_status = E_CFD_MC_ERR_INPUT_ISFULLRANGE;
        pstControlParamInit->u8Input_IsFullRange = E_CFD_CFIO_RANGE_LIMIT;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_IsFullRange to E_CFD_CFIO_RANGE_LIMIT\n"));
    }

    if ((pstControlParamInit->u8Input_HDRMode) >= E_CFIO_MODE_RESERVED_START)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Input_HDRMode is reserved \n"));
        u16_check_status = E_CFD_MC_ERR_INPUT_HDRMODE;
        pstControlParamInit->u8Input_HDRMode = E_CFIO_MODE_SDR;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_HDRMode to E_CFIO_MODE_SDR\n"));
    }

    if ((pstControlParamInit->u8Input_IsRGBBypass) >= E_CFD_RESERVED_AT0x02)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Input_IsRGBBypass is not correct \n"));
        u16_check_status = E_CFD_MC_ERR_INPUT_ISRGBBYPASS;
        pstControlParamInit->u8Input_IsRGBBypass = 0;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_IsRGBBypass to 0\n"));
    }

    if ((pstControlParamInit->u8Input_SDRIPMode) >= E_CFD_RESERVED_AT0x03)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Input_SDRIPMode is not correct \n"));
        u16_check_status = E_CFD_MC_ERR_INPUT_SDRIPMODE;
        pstControlParamInit->u8Input_SDRIPMode = 1;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_SDRIPMode to 1\n"));
    }

    if ((pstControlParamInit->u8Input_HDRIPMode) >= E_CFD_RESERVED_AT0x04)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Input_HDRIPMode is not correct \n"));
        u16_check_status = E_CFD_MC_ERR_INPUT_HDRIPMODE;
        pstControlParamInit->u8Input_HDRIPMode = 1;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_HDRIPMode to 1\n"));
    }


    if ((pstControlParamInit->stu_Middle_Format[0].u8Mid_Format_Mode) >= E_CFD_RESERVED_AT0x02)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Mid_Format_Mode is not correct \n"));
        u16_check_status = E_CFD_MC_ERR_INPUT_Mid_Format_Mode;
        pstControlParamInit->stu_Middle_Format[0].u8Mid_Format_Mode = 0;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_HDRIPMode to 1\n"));
    }

    if ((pstControlParamInit->stu_Middle_Format[0].u8Mid_Format) >= E_CFD_CFIO_RESERVED_START)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Mid_Format is reserved \n"));
        u16_check_status = E_CFD_MC_ERR_INPUT_Mid_Format;
        pstControlParamInit->stu_Middle_Format[0].u8Mid_Format = E_CFD_CFIO_YUV_BT709;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Mid_Format to E_CFD_CFIO_YUV_BT709\n"));
    }

    if ((pstControlParamInit->stu_Middle_Format[0].u8Mid_DataFormat) >= E_CFD_MC_FORMAT_RESERVED_START)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Mid_DataFormat is reserved \n"));
        u16_check_status = E_CFD_MC_ERR_INPUT_Mid_DataFormat;
        pstControlParamInit->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_YUV422;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Mid_DataFormat to E_CFD_MC_FORMAT_YUV422\n"));
    }

    if ((pstControlParamInit->stu_Middle_Format[0].u8Mid_IsFullRange) >= E_CFD_CFIO_RANGE_RESERVED_START)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Mid_IsFullRange is reserved \n"));
        u16_check_status = E_CFD_MC_ERR_INPUT_Mid_IsFullRange;
        pstControlParamInit->stu_Middle_Format[0].u8Mid_IsFullRange = E_CFD_CFIO_RANGE_LIMIT;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Mid_IsFullRange to E_CFD_CFIO_RANGE_LIMIT\n"));
    }

    if ((pstControlParamInit->stu_Middle_Format[0].u8Mid_HDRMode) >= E_CFIO_MODE_RESERVED_START)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Mid_HDRMode is reserved \n"));
        u16_check_status = E_CFD_MC_ERR_INPUT_Mid_HDRMode;
        pstControlParamInit->stu_Middle_Format[0].u8Mid_HDRMode = E_CFIO_MODE_SDR;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Mid_HDRMode to E_CFIO_MODE_SDR\n"));
    }

    if (pstControlParamInit->stu_Middle_Format[0].u8Mid_Colour_primaries >= E_CFD_CFIO_CP_RESERVED_START) {
        u16_check_status = E_CFD_MC_ERR_INPUT_Mid_Colour_primaries;
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl  u8Mid_Colour_primaries %d is out of range \n",     pstControlParamInit->stu_Middle_Format[0].u8Mid_Colour_primaries ));
        pstControlParamInit->stu_Middle_Format[0].u8Mid_Colour_primaries = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Mid_Colour_primaries to E_CFD_CFIO_MC_BT709_XVYCC709\n"));
    }

    if (pstControlParamInit->stu_Middle_Format[0].u8Mid_Transfer_Characteristics  >= E_CFD_CFIO_TR_RESERVED_START) {
        u16_check_status = E_CFD_MC_ERR_INPUT_Mid_Transfer_Characteristics;
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl  u8Mid_Transfer_Characteristics %d is out of range \n",     pstControlParamInit->stu_Middle_Format[0].u8Mid_Transfer_Characteristics ));
        pstControlParamInit->stu_Middle_Format[0].u8Mid_Transfer_Characteristics = E_CFD_CFIO_TR_BT709;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Mid_Transfer_Characteristics to E_CFD_CFIO_TR_BT709\n"));
    }

    if (pstControlParamInit->stu_Middle_Format[0].u8Mid_Matrix_Coeffs >= E_CFD_CFIO_MC_RESERVED_START) {
        u16_check_status = E_CFD_MC_ERR_INPUT_Mid_Matrix_Coeffs;
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl  u8Mid_Matrix_Coeffs %d is out of range \n",        pstControlParamInit->stu_Middle_Format[0].u8Mid_Matrix_Coeffs ));
        pstControlParamInit->stu_Middle_Format[0].u8Mid_Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Mid_Matrix_Coeffs to E_CFD_CFIO_MC_BT709_XVYCC709\n"));
    }

#if 0
    if ((pstControlParamInit->u8HDRIPOutput_ForceColoriMetryRemapping) > 1)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8HDRIPOutput_ForceColoriMetryRemapping is not correct \n"));
        pstControlParamInit->u8HDRIPOutput_ForceColoriMetryRemapping = 1;
        u8_check_status = FALSE;
    }
#endif

    if ((pstControlParamInit->u8Output_Source) == E_CFD_OUTPUT_SOURCE_MM)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Output_Source is not correct \n"));
        u16_check_status = E_CFD_MC_ERR_OUTPUT_SOURCE;
        pstControlParamInit->u8Output_Source = E_CFD_OUTPUT_SOURCE_PANEL;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Output_Source to E_CFD_OUTPUT_SOURCE_PANEL\n"));
    }

    if ((pstControlParamInit->u8Output_Source) >= E_CFD_OUTPUT_SOURCE_RESERVED_START)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Output_Source is not correct \n"));
        u16_check_status = E_CFD_MC_ERR_OUTPUT_SOURCE;
        pstControlParamInit->u8Output_Source = E_CFD_OUTPUT_SOURCE_PANEL;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Output_Source to E_CFD_OUTPUT_SOURCE_PANEL\n"));
    }

    if ((pstControlParamInit->u8Output_Format) >= E_CFD_CFIO_RESERVED_START)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Output_Format is not correct \n"));
        u16_check_status = E_CFD_MC_ERR_OUTPUT_FORMAT;
        pstControlParamInit->u8Output_Format = E_CFD_CFIO_RGB_BT709;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Output_Format to E_CFD_CFIO_RGB_BT709\n"));
    }

    if ((pstControlParamInit->u8Output_DataFormat) >= E_CFD_MC_FORMAT_RESERVED_START)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Output_DataFormat is not correct \n"));
        u16_check_status = E_CFD_MC_ERR_OUTPUT_DATAFORMAT;
        pstControlParamInit->u8Output_Format = E_CFD_MC_FORMAT_YUV422;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Output_DataFormat to E_CFD_MC_FORMAT_YUV422\n"));
    }

    if ((pstControlParamInit->u8Output_IsFullRange) >= E_CFD_CFIO_RANGE_RESERVED_START)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Output_IsFullRange is reserved \n"));
        u16_check_status = E_CFD_MC_ERR_OUTPUT_ISFULLRANGE;
        pstControlParamInit->u8Output_IsFullRange = E_CFD_CFIO_RANGE_LIMIT;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Output_IsFullRange to E_CFD_CFIO_RANGE_LIMIT\n"));
    }

    if ((pstControlParamInit->u8Output_HDRMode) >= E_CFIO_MODE_RESERVED_START)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Output_HDRMode is not correct \n"));
        u16_check_status = E_CFD_MC_ERR_OUTPUT_HDRMODE;
        pstControlParamInit->u8Output_HDRMode = E_CFIO_MODE_SDR;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Output_HDRMode to E_CFIO_MODE_SDR\n"));
    }

#if 0
    if (((pstControlParamInit->u8Input_Source) != E_CFD_INPUT_SOURCE_HDMI) && (pstControlParamInit->u8Input_Format_HDMI_CE_Flag == 1))
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Input_Format_HDMI_CE_Flag is not correct \n"));
        u8_check_status = FALSE;
    }

    if (((pstControlParamInit->u8Output_Source) != E_CFD_OUTPUT_SOURCE_HDMI) && (pstControlParamInit->u8Output_Format_HDMI_CE_Flag == 1))
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Output_Format_HDMI_CE_Flag is not correct \n"));
        u8_check_status = FALSE;
    }
#endif

    if ((pstControlParamInit->u8HDMIOutput_GammutMapping_Mode) >= E_CFD_RESERVED_AT0x03)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8HDMIOutput_GammutMapping_Mode is not correct \n"));
        u16_check_status = E_CFD_MC_ERR_HDMIOutput_GammutMapping_Mode;
        pstControlParamInit->u8HDMIOutput_GammutMapping_Mode = E_CFD_MODE_AT0x00;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8HDMIOutput_GammutMapping_Mode to 1\n"));
    }


    if ((pstControlParamInit->u8HDMIOutput_GammutMapping_MethodMode) >= E_CFD_RESERVED_AT0x02)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8HDMIOutput_GammutMapping_MethodMode is not correct \n"));
        u16_check_status = E_CFD_MC_ERR_HDMIOutput_GammutMapping_MethodMode;
        pstControlParamInit->u8HDMIOutput_GammutMapping_MethodMode = E_CFD_MODE_AT0x00;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8HDMIOutput_GammutMapping_MethodMode to 0\n"));
    }


    if ((pstControlParamInit->u8MMInput_ColorimetryHandle_Mode) >= E_CFD_RESERVED_AT0x04)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8MMInput_ColorimetryHandle_Mode is not correct \n"));
        u16_check_status = E_CFD_MC_ERR_MMInput_ColorimetryHandle_Mode;
        pstControlParamInit->u8MMInput_ColorimetryHandle_Mode = E_CFD_MODE_AT0x01;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8MMInput_ColorimetryHandle_Mode to 1\n"));
    }

    if ((pstControlParamInit->u8PanelOutput_GammutMapping_Mode) >= E_CFD_RESERVED_AT0x02)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8PanelOutput_GammutMapping_Mode is not correct \n"));
        u16_check_status = E_CFD_MC_ERR_PanelOutput_GammutMapping_Mode;
        pstControlParamInit->u8PanelOutput_GammutMapping_Mode = E_CFD_MODE_AT0x00;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8PanelOutput_GammutMapping_Mode to 0\n"));
    }

    if ((pstControlParamInit->u8TMO_TargetRefer_Mode) >= E_CFD_RESERVED_AT0x02)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8TMO_TargetRefer_Mode is not correct \n"));
        u16_check_status = E_CFD_MC_ERR_TMO_TargetRefer_Mode;
        pstControlParamInit->u8TMO_TargetRefer_Mode = E_CFD_MODE_AT0x00;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8TMO_TargetRefer_Mode to 0\n"));
    }

    u16Temp = (pstControlParamInit->u16Source_Max_Luminance);
    u8_check_status2 = Mapi_Cfd_Inter_CheckLuminanceBound(&u16Temp, MAX_LUMINANCE_UPBOUND, MAX_LUMINANCE_LOWBOUND);
    pstControlParamInit->u16Source_Max_Luminance = u16Temp;

    if (u8_check_status2 == FALSE)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u16Source_Max_Luminance %d is out of range \n", pstControlParamInit->u16Source_Max_Luminance ));
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u16Source_Max_Luminance to %d\n",pstControlParamInit->u16Source_Max_Luminance));
        u16_check_status = E_CFD_MC_ERR_Source_Max_Luminance;
    }

    u16Temp = (pstControlParamInit->u16Source_Med_Luminance);
    u8_check_status2 = Mapi_Cfd_Inter_CheckLuminanceBound(&u16Temp, MAX_LUMINANCE_UPBOUND, MAX_LUMINANCE_LOWBOUND);
    pstControlParamInit->u16Source_Med_Luminance = u16Temp;

    if (u8_check_status2 == FALSE)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u16Source_Med_Luminance %d is out of range \n", pstControlParamInit->u16Source_Med_Luminance ));
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u16Source_Med_Luminance to %d\n",pstControlParamInit->u16Source_Med_Luminance));
        u16_check_status = E_CFD_MC_ERR_Source_Med_Luminance;
    }

    u16Temp = (pstControlParamInit->u16Source_Min_Luminance);
    u8_check_status2 = Mapi_Cfd_Inter_CheckLuminanceBound(&u16Temp, MAX_LUMINANCE_UPBOUND, MAX_LUMINANCE_LOWBOUND);
    pstControlParamInit->u16Source_Min_Luminance = u16Temp;

    if (u8_check_status2 == FALSE)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u16Source_Min_Luminance %d is out of range \n", pstControlParamInit->u16Source_Min_Luminance ));
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u16Source_Min_Luminance to %d\n",pstControlParamInit->u16Source_Min_Luminance));
        u16_check_status = E_CFD_MC_ERR_Source_Min_Luminance;
    }

    u16Temp = (pstControlParamInit->u16Target_Max_Luminance);
    u8_check_status2 = Mapi_Cfd_Inter_CheckLuminanceBound(&u16Temp, MAX_LUMINANCE_UPBOUND, MAX_LUMINANCE_LOWBOUND);
    pstControlParamInit->u16Target_Max_Luminance = u16Temp;

    if (u8_check_status2 == FALSE)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u16Target_Max_Luminance %d is out of range \n", pstControlParamInit->u16Target_Max_Luminance ));
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u16Target_Max_Luminance to %d\n",pstControlParamInit->u16Target_Max_Luminance));
        u16_check_status = E_CFD_MC_ERR_Target_Max_Luminance;
    }

    u16Temp = (pstControlParamInit->u16Target_Med_Luminance);
    u8_check_status2 = Mapi_Cfd_Inter_CheckLuminanceBound(&u16Temp, MAX_LUMINANCE_UPBOUND, MAX_LUMINANCE_LOWBOUND);
    pstControlParamInit->u16Target_Med_Luminance = u16Temp;

    if (u8_check_status2 == FALSE)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u16Target_Med_Luminance %d is out of range \n", pstControlParamInit->u16Target_Med_Luminance ));
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u16Target_Med_Luminance to %d\n",pstControlParamInit->u16Target_Med_Luminance));
        u16_check_status = E_CFD_MC_ERR_Target_Med_Luminance;
    }

    u16Temp = (pstControlParamInit->u16Target_Min_Luminance);
    u8_check_status2 = Mapi_Cfd_Inter_CheckLuminanceBound(&u16Temp, MAX_LUMINANCE_UPBOUND, MAX_LUMINANCE_LOWBOUND);
    pstControlParamInit->u16Target_Min_Luminance = u16Temp;

    if (u8_check_status2 == FALSE)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD  u16Target_Min_Luminance %d is out of range \n",    pstControlParamInit->u16Target_Min_Luminance ));
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u16Target_Min_Luminance to %d\n",pstControlParamInit->u16Target_Min_Luminance));
        u16_check_status = E_CFD_MC_ERR_Target_Max_Luminance;
    }


    if (pstControlParamInit->u8Input_ext_Colour_primaries >= E_CFD_CFIO_CP_RESERVED_START) {
        u16_check_status = E_CFD_MC_ERR_INPUT_Mid_Colour_primaries;
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl  u8Input_ext_Colour_primaries %d is out of range \n",       pstControlParamInit->u8Input_ext_Colour_primaries ));
        pstControlParamInit->u8Input_ext_Colour_primaries = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_ext_Colour_primaries to E_CFD_CFIO_MC_BT709_XVYCC709\n"));
    }

    if (pstControlParamInit->u8Input_ext_Transfer_Characteristics  >= E_CFD_CFIO_TR_RESERVED_START) {
        u16_check_status = E_CFD_MC_ERR_INPUT_Mid_Transfer_Characteristics;
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl  u8Input_ext_Transfer_Characteristics %d is out of range \n",       pstControlParamInit->u8Input_ext_Transfer_Characteristics ));
        pstControlParamInit->u8Input_ext_Transfer_Characteristics = E_CFD_CFIO_TR_BT709;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_ext_Transfer_Characteristics to E_CFD_CFIO_TR_BT709\n"));
    }

    if (pstControlParamInit->u8Input_ext_Matrix_Coeffs >= E_CFD_CFIO_MC_RESERVED_START) {
        u16_check_status = E_CFD_MC_ERR_INPUT_Mid_Matrix_Coeffs;
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl  u8Input_ext_Matrix_Coeffs %d is out of range \n",      pstControlParamInit->u8Input_ext_Matrix_Coeffs ));
        pstControlParamInit->u8Input_ext_Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_ext_Matrix_Coeffs to E_CFD_CFIO_MC_BT709_XVYCC709\n"));
    }

    if (pstControlParamInit->u8PredefinedProcess >= E_CFD_RESERVED_AT0x02) {
        u16_check_status = E_CFD_MC_ERR_OVERRANGE;
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl  u8PredefinedProcess %d is out of range \n",      pstControlParamInit->u8PredefinedProcess ));
        pstControlParamInit->u8PredefinedProcess = 0;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8PredefinedProcess to 0\n"));
    }


#if 0
    //STB
    if (((pstControlParamInit->u8HW_Structure) == E_CFD_HWS_STB_TYPE1)||
        ((pstControlParamInit->u8HW_Structure) == E_CFD_HWS_STB_TYPE0))
    {

        if ((pstControlParamInit->u8Input_Source) == E_CFD_MC_SOURCE_ANALOG)
        {
            HDR_DBG_HAL_CFD(printk("\n  for STB, CFD MainControl E_CFD_MC_SOURCE_ANALOG is not correct \n"));
            u8_check_status = FALSE;
        }

        if ((pstControlParamInit->u8Output_Source) == E_CFD_OUTPUT_SOURCE_PANEL)
        {
            HDR_DBG_HAL_CFD(printk("\n  for STB, CFD MainControl E_CFD_OUTPUT_SOURCE_PANEL is not correct \n"));
            u8_check_status = FALSE;
        }

    }
    else//TV
    {

        if ((pstControlParamInit->u8Output_Source) != E_CFD_OUTPUT_SOURCE_PANEL)
        {
            HDR_DBG_HAL_CFD(printk("\n  for TV, CFD MainControl u8Output_Source is not correct \n"));
            u8_check_status = FALSE;
        }

    }
#endif

    //Interrelation rule ===================================================================================

#if 0
    if ((pstControlParamInit->u8Input_Format) >= E_CFD_CFIO_RESERVED_START)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Input_Format is reserved \n"));
        u16_check_status = E_CFD_MC_ERR_INPUT_FORMAT;
        pstControlParamInit->u8Input_Format = E_CFD_CFIO_YUV_BT709;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_Format to E_CFD_CFIO_YUV_BT709\n"));
    }

    if ((pstControlParamInit->u8Input_DataFormat) >= E_CFD_MC_FORMAT_RESERVED_START)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u8Input_DataFormat is reserved \n"));
        u16_check_status = E_CFD_MC_ERR_INPUT_DATAFORMAT;
        pstControlParamInit->u8Input_DataFormat = E_CFD_MC_FORMAT_YUV422;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_DataFormat to E_CFD_MC_FORMAT_YUV422\n"));
    }
#endif

    //RGB
    if ((pstControlParamInit->u8Input_Format >= E_CFD_CFIO_RGB_BT601_625) && (pstControlParamInit->u8Input_Format <= E_CFD_CFIO_ADOBE_RGB))
    {
        if (pstControlParamInit->u8Input_DataFormat != E_CFD_MC_FORMAT_RGB)
        {
            HDR_DBG_HAL_CFD(printk("\n  ERROR:u8Input_Format and u8Input_DataFormat don't match\n"));
            pstControlParamInit->u8Input_DataFormat = E_CFD_MC_FORMAT_RGB;
            HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_DataFormat to E_CFD_MC_FORMAT_RGB\n"));
            u16_check_status = E_CFD_MC_ERR_INPUT_FORMAT_DATAFORMAT_NOT_MATCH;
        }
    }

    //YUV
    if ((pstControlParamInit->u8Input_Format > E_CFD_CFIO_YUV_NOTSPECIFIED) && (pstControlParamInit->u8Input_Format <= E_CFD_CFIO_ADOBE_YCC601))
    {
        if (pstControlParamInit->u8Input_DataFormat == E_CFD_MC_FORMAT_RGB)
        {
            HDR_DBG_HAL_CFD(printk("\n  ERROR:u8Input_Format and u8Input_DataFormat don't match\n"));
            pstControlParamInit->u8Input_DataFormat = E_CFD_MC_FORMAT_YUV444;
            HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_DataFormat to E_CFD_MC_FORMAT_YUV444\n"));
            u16_check_status = E_CFD_MC_ERR_INPUT_FORMAT_DATAFORMAT_NOT_MATCH;
        }
    }

    //RGB
    if ((pstControlParamInit->stu_Middle_Format[0].u8Mid_Format >= E_CFD_CFIO_RGB_BT601_625) && (pstControlParamInit->stu_Middle_Format[0].u8Mid_Format <= E_CFD_CFIO_ADOBE_RGB))
    {
        if (pstControlParamInit->stu_Middle_Format[0].u8Mid_DataFormat != E_CFD_MC_FORMAT_RGB)
        {
            HDR_DBG_HAL_CFD(printk("\n  ERROR:u8Mid_Format and u8Mid_DataFormat don't match\n"));
            pstControlParamInit->stu_Middle_Format[0].u8Mid_DataFormat = E_CFD_MC_FORMAT_RGB;
            HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Mid_DataFormat to E_CFD_MC_FORMAT_RGB\n"));
            u16_check_status = E_CFD_MC_ERR_INPUT_FORMAT_DATAFORMAT_NOT_MATCH;
        }
    }

    //YUV
    if ((pstControlParamInit->stu_Middle_Format[0].u8Mid_Format > E_CFD_CFIO_YUV_NOTSPECIFIED) && (pstControlParamInit->stu_Middle_Format[0].u8Mid_Format <= E_CFD_CFIO_ADOBE_YCC601))
    {
        if (pstControlParamInit->stu_Middle_Format[0].u8Mid_DataFormat == E_CFD_MC_FORMAT_RGB)
        {
            HDR_DBG_HAL_CFD(printk("\n  ERROR:u8Mid_Format and u8Mid_DataFormat don't match\n"));
            pstControlParamInit->u8Input_DataFormat = E_CFD_MC_FORMAT_YUV444;
            HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Mid_DataFormat to E_CFD_MC_FORMAT_YUV444\n"));
            u16_check_status = E_CFD_MC_ERR_INPUT_FORMAT_DATAFORMAT_NOT_MATCH;
        }
    }

    //RGB
    if ((pstControlParamInit->u8Output_Format >= E_CFD_CFIO_RGB_BT601_625) && (pstControlParamInit->u8Output_Format <= E_CFD_CFIO_ADOBE_RGB))
    {
        if (pstControlParamInit->u8Output_DataFormat != E_CFD_MC_FORMAT_RGB)
        {
            HDR_DBG_HAL_CFD(printk("\n  ERROR:u8Output_Format and u8Output_DataFormat don't match\n"));
            pstControlParamInit->u8Output_DataFormat = E_CFD_MC_FORMAT_RGB;
            HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Output_DataFormat to E_CFD_MC_FORMAT_RGB\n"));
            u16_check_status = E_CFD_MC_ERR_INPUT_FORMAT_DATAFORMAT_NOT_MATCH;
        }
    }

    //YUV
    if ((pstControlParamInit->u8Output_Format > E_CFD_CFIO_YUV_NOTSPECIFIED) && (pstControlParamInit->u8Output_Format <= E_CFD_CFIO_ADOBE_YCC601))
    {
        if (pstControlParamInit->u8Output_DataFormat == E_CFD_MC_FORMAT_RGB)
        {
            HDR_DBG_HAL_CFD(printk("\n  ERROR:u8Output_Format and u8Output_DataFormat don't match\n"));
            pstControlParamInit->u8Input_DataFormat = E_CFD_MC_FORMAT_YUV444;
            HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Output_DataFormat to E_CFD_MC_FORMAT_YUV444\n"));
            u16_check_status = E_CFD_MC_ERR_INPUT_FORMAT_DATAFORMAT_NOT_MATCH;
        }
    }

    //if ((2 == pstControlParamInit->u8Input_HDRIPMode) && ( E_CFIO_MODE_HDR2 != pstControlParamInit->u8Input_HDRMode))
    if ((2 == pstControlParamInit->u8Input_HDRIPMode) && ( 0 == MS_Cfd_CheckOpenHDR(pstControlParamInit->u8Input_HDRMode)))
    {
        HDR_DBG_HAL_CFD(printk("\n  ERROR:u8Input_HDRIPMode = 2 is only for u8Input_HDRMode = openHDR \n"));
        pstControlParamInit->u8Input_HDRIPMode = 1;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_HDRIPMode to %d\n",1));
        u16_check_status = E_CFD_MC_ERR_INPUT_HDRIPMODE_HDRMODE_RULE_VIOLATION;
    }

    if ((3 == pstControlParamInit->u8Input_HDRIPMode) && ( E_CFIO_MODE_HDR1 != pstControlParamInit->u8Input_HDRMode))
    {
        HDR_DBG_HAL_CFD(printk("\n  ERROR:u8Input_HDRIPMode = 3 is only for u8Input_HDRMode = HDR1 \n"));
        pstControlParamInit->u8Input_HDRIPMode = 1;
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u8Input_HDRIPMode to %d\n",1));
        u16_check_status = E_CFD_MC_ERR_INPUT_HDRIPMODE_HDRMODE_RULE_VIOLATION;
    }
    /*
    if ((E_CFD_CFIO_DOLBY_HDR_TEMP == pstControlParamInit->u8Input_Format) || (E_CFIO_MODE_HDR1 == pstControlParamInit->u8Input_HDRMode) || (3 == pstControlParamInit->u8Input_HDRIPMode))
    {
        if ((E_CFD_CFIO_DOLBY_HDR_TEMP != pstControlParamInit->u8Input_Format) || (E_CFIO_MODE_HDR1 != pstControlParamInit->u8Input_HDRMode) || (3 != pstControlParamInit->u8Input_HDRIPMode))
        {
            u16_check_status = E_CFD_MC_ERR_INPUT_MAIN_CONTROLS;
            HDR_DBG_HAL_CFD(printk("\n ERROR:for Dolby HDR input case:"));
            HDR_DBG_HAL_CFD(printk("\n force th these values"));
            HDR_DBG_HAL_CFD(printk("\n u8Input_Format must be E_CFD_CFIO_DOLBY_HDR_TEMP"));
            HDR_DBG_HAL_CFD(printk("\n u8Input_HDRMode must be E_CFIO_MODE_HDR1"));
            HDR_DBG_HAL_CFD(printk("\n u8Input_HDRIPMode must be 3"));

            pstControlParamInit->u8Input_Format = E_CFD_CFIO_DOLBY_HDR_TEMP;
            pstControlParamInit->u8Input_HDRMode = E_CFIO_MODE_HDR1;
            pstControlParamInit->u8Input_HDRIPMode = 3;

        }
    }
    */
    //======================================================================================================

    if (E_CFD_MC_ERR_NOERR != u16_check_status)
    {
        HDR_DBG_HAL_CFD(printk("\nError:please check: some parameters in Main Control structure is wrong, but has been force to some value\n"));
        HDR_DBG_HAL_CFD(printk("Error code = %04x !!!, error is in [ %s  , %d]\n",u16_check_status, __FUNCTION__,__LINE__));
    }
    u16_check_status = E_CFD_MC_ERR_NOERR;//Easter_test
    return u16_check_status;
}



MS_U16 Mapi_Cfd_inter_MM_Param_Check(STU_CFDAPI_MM_PARSER *pstMMParamInita)
{
    MS_U16 u16_check_status = E_CFD_MC_ERR_NOERR;
    MS_U8 u8_check_status2 = E_CFD_MC_ERR_NOERR;
    MS_U8 u8_check_status_Chromaticity_Coordinates = TRUE;

    HDR_DBG_HAL_CFD(printk("\n  MM  STU_CFDAPI_MM_PARSER:    \n"));
    HDR_DBG_HAL_CFD(printk("\n  MM  version=%d    \n",                      pstMMParamInita->u32Version ));
    HDR_DBG_HAL_CFD(printk("\n  MM  length=%d    \n",                       pstMMParamInita->u16Length ));
    //HDR_DBG_HAL_CFD(printk("\n  MM  u8MM_Codec=%d    \n",                     pstMMParamInita->u8MM_Codec ));
    //HDR_DBG_HAL_CFD(printk("\n  MM  u8ColorDescription_Valid=%d    \n",       pstMMParamInita->u8ColorDescription_Valid ));
    HDR_DBG_HAL_CFD(printk("\n  MM  u8Colour_primaries=%d    \n",               pstMMParamInita->u8Colour_primaries));
    HDR_DBG_HAL_CFD(printk("\n  MM  u8Transfer_Characteristics=%d    \n",       pstMMParamInita->u8Transfer_Characteristics ));
    HDR_DBG_HAL_CFD(printk("\n  MM  u8Matrix_Coeffs=%d    \n",                  pstMMParamInita->u8Matrix_Coeffs ));
    HDR_DBG_HAL_CFD(printk("\n  MM  u8Video_Full_Range_Flag=%d    \n",          pstMMParamInita->u8Video_Full_Range_Flag ));
    HDR_DBG_HAL_CFD(printk("\n  MM  u32Master_Panel_Max_Luminance=%d    \n",    pstMMParamInita->u32Master_Panel_Max_Luminance ));
    HDR_DBG_HAL_CFD(printk("\n  MM  u32Master_Panel_Min_Luminance=%d    \n",    pstMMParamInita->u32Master_Panel_Min_Luminance ));
    //HDR_DBG_HAL_CFD(printk("\n  MM  u16Max_Content_Light_Level=%d    \n",     pstMMParamInita->u16Max_Content_Light_Level ));
    //HDR_DBG_HAL_CFD(printk("\n  MM  u16Max_Frame_Avg_Light_Level=%d    \n",   pstMMParamInita->u16Max_Frame_Avg_Light_Level ));
    HDR_DBG_HAL_CFD(printk("\n  MM  u8Mastering_Display_Infor_Valid=%d  \n",    pstMMParamInita->u8Mastering_Display_Infor_Valid ));
    HDR_DBG_HAL_CFD(printk("\n  MM  u16Display_Primaries_x[0]=%d    \n",        pstMMParamInita->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[0] ));
    HDR_DBG_HAL_CFD(printk("\n  MM  u16Display_Primaries_y[0]=%d    \n",        pstMMParamInita->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[0] ));
    HDR_DBG_HAL_CFD(printk("\n  MM  u16Display_Primaries_x[1]=%d    \n",        pstMMParamInita->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[1] ));
    HDR_DBG_HAL_CFD(printk("\n  MM  u16Display_Primaries_y[1]=%d    \n",        pstMMParamInita->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[1] ));
    HDR_DBG_HAL_CFD(printk("\n  MM  u16Display_Primaries_x[2]=%d    \n",        pstMMParamInita->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_x[2] ));
    HDR_DBG_HAL_CFD(printk("\n  MM  u16Display_Primaries_y[2]=%d    \n",        pstMMParamInita->stu_Cfd_MM_MasterPanel_ColorMetry.u16Display_Primaries_y[2] ));
    HDR_DBG_HAL_CFD(printk("\n  MM  u16White_point_x=%d    \n",                 pstMMParamInita->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_x ));
    HDR_DBG_HAL_CFD(printk("\n  MM  u16White_point_y=%d    \n",                 pstMMParamInita->stu_Cfd_MM_MasterPanel_ColorMetry.u16White_point_y ));
    HDR_DBG_HAL_CFD(printk("\n  MM  u16Max_content_light_level=%d    \n",       pstMMParamInita->u16Max_content_light_level ));
    HDR_DBG_HAL_CFD(printk("\n  MM  u16Max_pic_average_light_level=%d    \n",   pstMMParamInita->u16Max_pic_average_light_level ));

    //add constraints
    //detect values out of range and show messages.

#if 0
    if (pstMMParamInita->u8MM_Codec >= E_CFD_MM_CODEC_EMUEND) {
        u8_check_status = FALSE;
        HDR_DBG_HAL_CFD(printk("\n  MM  u8MM_Codec %d is out of range \n",      pstMMParamInita->u8MM_Codec ));
        pstMMParamInita->u8MM_Codec = E_CFD_MM_CODEC_OTHERS;
        HDR_DBG_HAL_CFD(printk("\n  force MM u8MM_Codec to E_CFD_MM_CODEC_OTHERS[%s]\n",__FUNCTION__));
    }


    if (pstMMParamInita->u8ColorDescription_Valid >= E_CFD_VALID_EMUEND) {
        u8_check_status = FALSE;
        HDR_DBG_HAL_CFD(printk("\n  MM  u8ColorDescription_Valid %d is out of range \n",        pstMMParamInita->u8ColorDescription_Valid ));
        pstMMParamInita->u8ColorDescription_Valid = E_CFD_NOT_VALID;
        HDR_DBG_HAL_CFD(printk("\n  force MM u8ColorDescription_Valid to E_CFD_NOT_VALID\n"));
    }
#endif

    if (pstMMParamInita->u8Colour_primaries >= E_CFD_CFIO_CP_RESERVED_START) {
        u16_check_status = E_CFD_MC_ERR_MM_Colour_primaries;
        HDR_DBG_HAL_CFD(printk("\n  MM  u8Colour_primaries %d is out of range \n",pstMMParamInita->u8Colour_primaries ));
        pstMMParamInita->u8Colour_primaries = E_CFD_CFIO_CP_BT709_SRGB_SYCC;
        HDR_DBG_HAL_CFD(printk("\n  force MM u8Colour_primaries to E_CFD_CFIO_CP_BT709_SRGB_SYCC\n"));
    }

    if (pstMMParamInita->u8Transfer_Characteristics >= E_CFD_CFIO_TR_RESERVED_START) {
        u16_check_status = E_CFD_MC_ERR_MM_Transfer_Characteristics;
        HDR_DBG_HAL_CFD(printk("\n  MM  u8Transfer_Characteristics %d is out of range \n",  pstMMParamInita->u8Transfer_Characteristics ));
        pstMMParamInita->u8Transfer_Characteristics = E_CFD_CFIO_TR_BT709;
        HDR_DBG_HAL_CFD(printk("\n  force MM u8Transfer_Characteristics to E_CFD_CFIO_TR_BT709\n"));
    }

    if (pstMMParamInita->u8Matrix_Coeffs >= E_CFD_CFIO_MC_RESERVED_START) {
        u16_check_status = E_CFD_MC_ERR_MM_Matrix_Coeffs;
        HDR_DBG_HAL_CFD(printk("\n  MM  u8Matrix_Coeffs %d is out of range \n",     pstMMParamInita->u8Matrix_Coeffs ));
        pstMMParamInita->u8Matrix_Coeffs = E_CFD_CFIO_MC_BT709_XVYCC709;
        HDR_DBG_HAL_CFD(printk("\n  force MM u8Matrix_Coeffs to E_CFD_CFIO_MC_BT709_XVYCC709\n"));
    }

    if (pstMMParamInita->u8Video_Full_Range_Flag >= E_CFD_RESERVED_AT0x02) {
        u16_check_status = E_CFD_MC_ERR_MM_IsFullRange;
        HDR_DBG_HAL_CFD(printk("\n  MM  u8Video_Full_Range_Flag %d is out of range \n",     pstMMParamInita->u8Video_Full_Range_Flag ));
        pstMMParamInita->u8Video_Full_Range_Flag = E_CFD_CFIO_RANGE_LIMIT;
        HDR_DBG_HAL_CFD(printk("\n  force MM u8Video_Full_Range_Flag to E_CFD_CFIO_RANGE_LIMIT\n"));
    }

    //in SEI
    //g_stu_Cfd_MM_Paramters.stu_Cfd_Master_Display.u32Master_Panel_Max_Luminance;
    //force reasonable maximum of u32Master_Panel_Max_Luminance = 1e8 = 1e8*0.0001 = 10000 nits
    //force reasonable minimum of u32Master_Panel_Max_Luminance = 1e4 = 1e4*0.0001 = 1 nits

    MS_U16 u16Temp = (pstMMParamInita->u32Master_Panel_Max_Luminance/10000);
    u8_check_status2 = Mapi_Cfd_Inter_CheckLuminanceBound(&u16Temp, MAX_LUMINANCE_UPBOUND, MAX_LUMINANCE_LOWBOUND);
    pstMMParamInita->u32Master_Panel_Max_Luminance = u16Temp*10000;

    if (u8_check_status2 == FALSE)
    {
        HDR_DBG_HAL_CFD(printk("\n  MM  u32Master_Panel_Max_Luminance %d is out of range \n",   pstMMParamInita->u32Master_Panel_Max_Luminance ));
        HDR_DBG_HAL_CFD(printk("\n  force MM u32Master_Panel_Max_Luminance to %d\n",pstMMParamInita->u32Master_Panel_Max_Luminance));
        u16_check_status = E_CFD_MC_ERR_MM_Mastering_Display;
    }

    //g_stu_Cfd_MM_Paramters.stu_Cfd_Master_Display.u32Master_Panel_Min_Luminance
    //force reasonable maximum of u32Master_Panel_Min_Luminance = 1e4 = 1e6*0.0001 = 1 nits
    //force reasonable minimum of u32Master_Panel_Min_Luminance = 1 = 0.0001 nits

    u16Temp = (pstMMParamInita->u32Master_Panel_Min_Luminance&0xffff);
    u8_check_status2 = Mapi_Cfd_Inter_CheckLuminanceBound(&u16Temp, MIN_LUMINANCE_UPBOUND, MIN_LUMINANCE_LOWBOUND);
    pstMMParamInita->u32Master_Panel_Min_Luminance = u16Temp;

    if (u8_check_status2 == FALSE)
    {
        HDR_DBG_HAL_CFD(printk("\n  MM  u32Master_Panel_Min_Luminance %d is out of range \n",   pstMMParamInita->u32Master_Panel_Min_Luminance ));
        HDR_DBG_HAL_CFD(printk("\n  force MM u32Master_Panel_Min_Luminance to %d\n",pstMMParamInita->u32Master_Panel_Min_Luminance));
        u16_check_status = E_CFD_MC_ERR_MM_Mastering_Display;
    }

    if (pstMMParamInita->u8Mastering_Display_Infor_Valid >= E_CFD_VALID_EMUEND) {
        u16_check_status = E_CFD_MC_ERR_MM_Mastering_Display;
        HDR_DBG_HAL_CFD(printk("\n  MM  u8Mastering_Display_Infor_Valid %d is out of range \n",     pstMMParamInita->u8Mastering_Display_Infor_Valid ));
        pstMMParamInita->u8Mastering_Display_Infor_Valid = E_CFD_NOT_VALID;
        HDR_DBG_HAL_CFD(printk("\n  force MM u8Mastering_Display_Infor_Valid to E_CFD_NOT_VALID\n"));
    }


    //all the primary should be in the range of 0 ~ 50000
    //the order of x[3] and y[3] should follow Rx >= Gx >= Bx, Gy >= Ry >= By
    /*
    g_stu_Cfd_MM_Paramters.stu_Cfd_Master_Display.u16Display_Primaries_x[0]     = (u32MMParamInit[9]&0xffff);
    g_stu_Cfd_MM_Paramters.stu_Cfd_Master_Display.u16Display_Primaries_x[1]     = (u32MMParamInit[10]&0xffff);
    g_stu_Cfd_MM_Paramters.stu_Cfd_Master_Display.u16Display_Primaries_x[2]     = (u32MMParamInit[11]&0xffff);

    g_stu_Cfd_MM_Paramters.stu_Cfd_Master_Display.u16Display_Primaries_y[0]     = (u32MMParamInit[12]&0xffff);
    g_stu_Cfd_MM_Paramters.stu_Cfd_Master_Display.u16Display_Primaries_y[1]     = (u32MMParamInit[13]&0xffff);
    g_stu_Cfd_MM_Paramters.stu_Cfd_Master_Display.u16Display_Primaries_y[2]     = (u32MMParamInit[14]&0xffff);

    g_stu_Cfd_MM_Paramters.stu_Cfd_Master_Display.u16White_point_x              = (u32MMParamInit[15]&0xffff);
    g_stu_Cfd_MM_Paramters.stu_Cfd_Master_Display.u16White_point_y              = (u32MMParamInit[16]&0xffff);
    */

    MS_U16 u16_xy_highbound = 50000;
    u8_check_status_Chromaticity_Coordinates = MS_Cfd_inter_Check_Chromaticity_Coordinates(&(pstMMParamInita->stu_Cfd_MM_MasterPanel_ColorMetry),u16_xy_highbound);

    if ((u8_check_status_Chromaticity_Coordinates & 0x01) == 0x01)
    {
        u16_check_status = E_CFD_MC_ERR_MM_Mastering_Display_Coordinates;
        HDR_DBG_HAL_CFD(printk("\n  MM - the order rule of Chromaticity_Coordinates fails, please check\n"));
    }

    if ((u8_check_status_Chromaticity_Coordinates & 0x02) == 0x02)
    {
        u16_check_status = E_CFD_MC_ERR_MM_Mastering_Display_Coordinates;
        HDR_DBG_HAL_CFD(printk("\n  MM - Chromaticity_Coordinates is out of range, force the value to %d\n",u16_xy_highbound));
    }

    if (E_CFD_MC_ERR_NOERR != u16_check_status)
    {
        HDR_DBG_HAL_CFD(printk("\nError:please check: some parameters in MM input structure is wrong, but has been force to some value\n"));
        HDR_DBG_HAL_CFD(printk("Error code = %04x !!!, error is in [ %s  , %d]\n",u16_check_status, __FUNCTION__,__LINE__));
    }
    u16_check_status = E_CFD_MC_ERR_NOERR;//Easter_test
    return u16_check_status;
}


MS_U16 Mapi_Cfd_inter_HDMI_InfoFrame_Param_Check(STU_CFDAPI_HDMI_INFOFRAME_PARSER *pstHDMI_InfoFrameParamInit)
{
    MS_U16 u16_check_status = E_CFD_MC_ERR_NOERR;
    MS_U8 u8_check_status2 = E_CFD_MC_ERR_NOERR;
    MS_U8 u8_check_status_Chromaticity_Coordinates = TRUE;
    MS_U16 u16Temp = 0;

    HDR_DBG_HAL_CFD(printk("\n  HDMISource  STU_CFD_HDMI_EDID_PARSER:    \n"));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u8HDMISource_HDR_InfoFrame_Valid=%d    \n", pstHDMI_InfoFrameParamInit->u8HDMISource_HDR_InfoFrame_Valid ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u8HDMISource_EOTF=%d    \n",                    pstHDMI_InfoFrameParamInit->u8HDMISource_EOTF ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u8HDMISource_SMD_ID=%d    \n",                  pstHDMI_InfoFrameParamInit->u8HDMISource_SMD_ID ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16Master_Panel_Max_Luminance=%d    \n",        pstHDMI_InfoFrameParamInit->u16Master_Panel_Max_Luminance ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16Master_Panel_Min_Luminance=%d    \n",        pstHDMI_InfoFrameParamInit->u16Master_Panel_Min_Luminance ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16Max_Content_Light_Level=%d    \n",           pstHDMI_InfoFrameParamInit->u16Max_Content_Light_Level ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16Max_Frame_Avg_Light_Level=%d    \n",     pstHDMI_InfoFrameParamInit->u16Max_Frame_Avg_Light_Level ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16Display_Primaries_x[0]=%d    \n",            pstHDMI_InfoFrameParamInit->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[0] ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16Display_Primaries_y[0]=%d    \n",            pstHDMI_InfoFrameParamInit->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[0] ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16Display_Primaries_x[1]=%d    \n",            pstHDMI_InfoFrameParamInit->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[1] ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16Display_Primaries_y[1]=%d    \n",            pstHDMI_InfoFrameParamInit->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[1] ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16Display_Primaries_x[2]=%d    \n",            pstHDMI_InfoFrameParamInit->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_x[2] ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16Display_Primaries_y[2]=%d    \n",            pstHDMI_InfoFrameParamInit->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16Display_Primaries_y[2] ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16White_point_x=%d    \n",                     pstHDMI_InfoFrameParamInit->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_x ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16White_point_y=%d    \n",                     pstHDMI_InfoFrameParamInit->stu_Cfd_HDMISource_MasterPanel_ColorMetry.u16White_point_y ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u8Mastering_Display_Infor_Valid=%d    \n",  pstHDMI_InfoFrameParamInit->u8Mastering_Display_Infor_Valid ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u8HDMISource_Support_Format=%d    \n",      pstHDMI_InfoFrameParamInit->u8HDMISource_Support_Format ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISource  u8HDMISource_Colorspace=%d    \n",              pstHDMI_InfoFrameParamInit->u8HDMISource_Colorspace ));
    //HDR_DBG_HAL_CFD(printk("\n  HDMISource  u8HDMISource_AVIInfoFrame_Valid=%d    \n",                pstHDMI_InfoFrameParamInit->u8HDMISource_AVIInfoFrame_Valid ));

    //add constraints
    //detect values out of range and show messages.

    if (pstHDMI_InfoFrameParamInit->u8HDMISource_HDR_InfoFrame_Valid >= E_CFD_VALID_EMUEND) {
        u16_check_status = E_CFD_MC_ERR_HDMI_INFOFRAME_HDR_Infor;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource  u8HDMISource_HDR_InfoFrame_Valid %d is out of range \n",        pstHDMI_InfoFrameParamInit->u8HDMISource_HDR_InfoFrame_Valid ));
        pstHDMI_InfoFrameParamInit->u8HDMISource_HDR_InfoFrame_Valid = E_CFD_VALID;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISource u8HDMISource_HDR_InfoFrame_Valid to E_CFD_VALID\n"));
    }

    if (pstHDMI_InfoFrameParamInit->u8HDMISource_EOTF >= E_CFD_HDMI_EOTF_RESERVED) {
        u16_check_status = E_CFD_MC_ERR_HDMI_INFOFRAME_HDR_Infor;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource  u8HDMISource_EOTF %d is out of range \n",       pstHDMI_InfoFrameParamInit->u8HDMISource_EOTF ));
        pstHDMI_InfoFrameParamInit->u8HDMISource_EOTF = E_CFD_HDMI_EOTF_SDR_GAMMA;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISource u8HDMISource_EOTF to E_CFD_HDMI_EOTF_SDR_GAMMA\n"));
    }

    if (pstHDMI_InfoFrameParamInit->u8HDMISource_SMD_ID != E_CFD_HDMI_META_TYPE1) {
        u16_check_status = E_CFD_MC_ERR_HDMI_INFOFRAME_HDR_Infor;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource  u8HDMISource_SMD_ID %d is wrong \n",        pstHDMI_InfoFrameParamInit->u8HDMISource_SMD_ID ));
        pstHDMI_InfoFrameParamInit->u8HDMISource_SMD_ID = E_CFD_HDMI_META_TYPE1;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISource u8HDMISource_SMD_ID to E_CFD_HDMI_META_TYPE1\n"));
    }

    //in HDMI
    //g_stu_Cfd_MM_Paramters.stu_Cfd_Master_Display.u16Master_Panel_Max_Luminance;
    //force reasonable maximum of u16Master_Panel_Max_Luminance = 1e4 = 10000 nits
    //force reasonable minimum of u16Master_Panel_Max_Luminance = 1e2 = 100 nits

#if 0
    if (pstHDMI_InfoFrameParamInit->u16Master_Panel_Max_Luminance > 10000) {
        u16_check_status = E_CFD_MC_ERR_HDMI_INFOFRAME_HDR_Infor;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16Master_Panel_Max_Luminance %d is out of range \n",   pstHDMI_InfoFrameParamInit->u16Master_Panel_Max_Luminance ));
        pstHDMI_InfoFrameParamInit->u16Master_Panel_Max_Luminance = 10000;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISource u16Master_Panel_Max_Luminance to 1e4\n"));
    }

    if (pstHDMI_InfoFrameParamInit->u16Master_Panel_Min_Luminance > 10000) {
        u16_check_status = E_CFD_MC_ERR_HDMI_INFOFRAME_HDR_Infor;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16Master_Panel_Min_Luminance %d is out of range \n",   pstHDMI_InfoFrameParamInit->u16Master_Panel_Min_Luminance ));
        pstHDMI_InfoFrameParamInit->u16Master_Panel_Min_Luminance = 10000;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISource u16Master_Panel_Min_Luminance to 1e4\n"));
    }
#endif

    u16Temp = (pstHDMI_InfoFrameParamInit->u16Master_Panel_Max_Luminance);
    u8_check_status2 = Mapi_Cfd_Inter_CheckLuminanceBound(&u16Temp, MAX_LUMINANCE_UPBOUND, MAX_LUMINANCE_LOWBOUND);
    pstHDMI_InfoFrameParamInit->u16Master_Panel_Max_Luminance = u16Temp;

    if (u8_check_status2 == FALSE)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u16Target_Max_Luminance %d is out of range \n", pstHDMI_InfoFrameParamInit->u16Master_Panel_Max_Luminance ));
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u16Target_Max_Luminance to %d\n",pstHDMI_InfoFrameParamInit->u16Master_Panel_Max_Luminance));
        u16_check_status = E_CFD_MC_ERR_HDMI_INFOFRAME_HDR_Infor;
    }

    u16Temp = (pstHDMI_InfoFrameParamInit->u16Master_Panel_Min_Luminance);
    u8_check_status2 = Mapi_Cfd_Inter_CheckLuminanceBound(&u16Temp, MAX_LUMINANCE_UPBOUND, MAX_LUMINANCE_LOWBOUND);
    pstHDMI_InfoFrameParamInit->u16Master_Panel_Min_Luminance = u16Temp;

    if (u8_check_status2 == FALSE)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u16Target_Max_Luminance %d is out of range \n", pstHDMI_InfoFrameParamInit->u16Master_Panel_Min_Luminance ));
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u16Target_Max_Luminance to %d\n",pstHDMI_InfoFrameParamInit->u16Master_Panel_Min_Luminance));
        u16_check_status = E_CFD_MC_ERR_HDMI_INFOFRAME_HDR_Infor;
    }

#if 0
    else if (pstHDMI_InfoFrameParamInit->u16Master_Panel_Max_Luminance < 100) {
        u8_check_status = FALSE;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16Master_Panel_Max_Luminance %d is out of range \n",   pstHDMI_InfoFrameParamInit->u16Master_Panel_Max_Luminance ));
        pstHDMI_InfoFrameParamInit->u16Master_Panel_Max_Luminance = 100;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISource u16Master_Panel_Max_Luminance to 1e2\n"));
    }
#endif
    //g_stu_Cfd_HDMI_Infoframe_Parameters.stu_Cfd_Master_Display.u16Master_Panel_Min_Luminance : no limits

#if 0
    if (pstHDMI_InfoFrameParamInit->u16Max_Content_Light_Level > 10000) {
        u8_check_status = FALSE;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16Max_Content_Light_Level %d is out of range \n",  pstHDMI_InfoFrameParamInit->u16Max_Content_Light_Level ));
        pstHDMI_InfoFrameParamInit->u16Max_Content_Light_Level = 10000;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISource u16Max_Content_Light_Level to 1e4\n"));
    }
    else if (pstHDMI_InfoFrameParamInit->u16Max_Content_Light_Level < 100) {
        u8_check_status = FALSE;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16Max_Content_Light_Level %d is out of range \n",  pstHDMI_InfoFrameParamInit->u16Max_Content_Light_Level ));
        pstHDMI_InfoFrameParamInit->u16Max_Content_Light_Level = 100;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISource u16Max_Content_Light_Level to 1e2\n"));
    }

    if (pstHDMI_InfoFrameParamInit->u16Max_Frame_Avg_Light_Level > 10000) {
        u8_check_status = FALSE;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16Max_Frame_Avg_Light_Level %d is out of range \n",    pstHDMI_InfoFrameParamInit->u16Max_Frame_Avg_Light_Level ));
        pstHDMI_InfoFrameParamInit->u16Max_Frame_Avg_Light_Level = 10000;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISource u16Max_Frame_Avg_Light_Level to 1e4\n"));
    }
    else if (pstHDMI_InfoFrameParamInit->u16Max_Frame_Avg_Light_Level < 100) {
        u8_check_status = FALSE;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource  u16Max_Frame_Avg_Light_Level %d is out of range \n",    pstHDMI_InfoFrameParamInit->u16Max_Frame_Avg_Light_Level ));
        pstHDMI_InfoFrameParamInit->u16Max_Frame_Avg_Light_Level = 100;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISource u16Max_Frame_Avg_Light_Level to 1e2\n"));
    }
#endif

    u16Temp = (pstHDMI_InfoFrameParamInit->u16Max_Content_Light_Level);
    u8_check_status2 = Mapi_Cfd_Inter_CheckLuminanceBound(&u16Temp, MAX_LUMINANCE_UPBOUND, MAX_LUMINANCE_LOWBOUND);
    pstHDMI_InfoFrameParamInit->u16Max_Content_Light_Level = u16Temp;

    if (u8_check_status2 == FALSE)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u16Max_Content_Light_Level %d is out of range \n",  pstHDMI_InfoFrameParamInit->u16Max_Content_Light_Level ));
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u16Max_Content_Light_Level to %d\n",pstHDMI_InfoFrameParamInit->u16Max_Content_Light_Level));
        u16_check_status = E_CFD_MC_ERR_HDMI_INFOFRAME_HDR_Infor;
    }

    u16Temp = (pstHDMI_InfoFrameParamInit->u16Max_Frame_Avg_Light_Level);
    u8_check_status2 = Mapi_Cfd_Inter_CheckLuminanceBound(&u16Temp, MAX_LUMINANCE_UPBOUND, MAX_LUMINANCE_LOWBOUND);
    pstHDMI_InfoFrameParamInit->u16Max_Frame_Avg_Light_Level = u16Temp;

    if (u8_check_status2 == FALSE)
    {
        HDR_DBG_HAL_CFD(printk("\n  CFD MainControl u16Max_Frame_Avg_Light_Level %d is out of range \n",    pstHDMI_InfoFrameParamInit->u16Max_Frame_Avg_Light_Level ));
        HDR_DBG_HAL_CFD(printk("\n  force MainControl u16Max_Frame_Avg_Light_Level to %d\n",pstHDMI_InfoFrameParamInit->u16Max_Frame_Avg_Light_Level));
        u16_check_status = E_CFD_MC_ERR_HDMI_INFOFRAME_HDR_Infor;
    }

    MS_U16 u16_xy_highbound = 50000;
    u8_check_status_Chromaticity_Coordinates = MS_Cfd_inter_Check_Chromaticity_Coordinates(&(pstHDMI_InfoFrameParamInit->stu_Cfd_HDMISource_MasterPanel_ColorMetry),u16_xy_highbound);
//  u8_check_status_Chromaticity_Coordinates = mscfd_HDMI_Check_Chromaticity_Coordinates(&(g_stu_Cfd_HDMI_Infoframe_Parameters.stu_Cfd_Master_Display));

    if ((u8_check_status_Chromaticity_Coordinates & 0x01) == 0x01)
    {
        u16_check_status = E_CFD_MC_ERR_HDMI_INFOFRAME_Mastering_Display_Coordinates;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource - the rule of Chromaticity_Coordinates fails, please check\n"));
    }

    if ((u8_check_status_Chromaticity_Coordinates & 0x02) == 0x02)
    {
        u16_check_status = E_CFD_MC_ERR_HDMI_INFOFRAME_Mastering_Display_Coordinates;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource - Chromaticity_Coordinates is out of range, force the value to %d\n",u16_xy_highbound));
    }

    if (pstHDMI_InfoFrameParamInit->u8Mastering_Display_Infor_Valid >= E_CFD_VALID_EMUEND) {
        u16_check_status = E_CFD_MC_ERR_HDMI_INFOFRAME_HDR_Infor;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource  u8Mastering_Display_Infor_Valid %d is out of range \n",     pstHDMI_InfoFrameParamInit->u8Mastering_Display_Infor_Valid ));
        pstHDMI_InfoFrameParamInit->u8Mastering_Display_Infor_Valid = E_CFD_NOT_VALID;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISource u8Mastering_Display_Infor_Valid to E_CFD_NOT_VALID\n"));
    }


    //g_stu_Cfd_HDMI_Infoframe_Parameters.u8HDMISource_Support_Format : 0~127 full range is ok
    //MS_U8 u8HDMISource_Support_Format;
    //[2:0] = {Y2 Y1 Y0}
    //[4:3] = {YQ1 YQ0}
    //[6:5] = {Q1 Q0}

    if (pstHDMI_InfoFrameParamInit->u8HDMISource_Support_Format >= E_CFD_RESERVED_AT0x80) {
        u16_check_status = E_CFD_MC_ERR_HDMI_INFOFRAME;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource  u8HDMISource_Support_Format %d is out of range \n",     pstHDMI_InfoFrameParamInit->u8HDMISource_Support_Format ));
        pstHDMI_InfoFrameParamInit->u8HDMISource_Support_Format = (pstHDMI_InfoFrameParamInit->u8HDMISource_Support_Format&0x7f);
        HDR_DBG_HAL_CFD(printk("\n  force HDMISource u8HDMISource_Support_Format to LSB 7 bits of original data\n"));
    }

    //g_stu_Cfd_HDMI_Infoframe_Parameters.u8HDMISource_Colorspace : 0~255 full range is ok
    //need to check
    if (pstHDMI_InfoFrameParamInit->u8HDMISource_Colorspace == E_CFD_CFIO_RGB_NOTSPECIFIED) {
        u16_check_status = E_CFD_MC_ERR_HDMI_INFOFRAME;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource  u8HDMISource_Colorspace is not specified \n",       pstHDMI_InfoFrameParamInit->u8HDMISource_Colorspace ));
        pstHDMI_InfoFrameParamInit->u8HDMISource_Colorspace = E_CFD_CFIO_SRGB;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISource u8HDMISource_Colorspace to E_CFD_CFIO_SRGB\n"));
    }

    if (pstHDMI_InfoFrameParamInit->u8HDMISource_Colorspace == E_CFD_CFIO_YUV_NOTSPECIFIED) {
        u16_check_status = E_CFD_MC_ERR_HDMI_INFOFRAME;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource  u8HDMISource_Colorspace is not specified \n",       pstHDMI_InfoFrameParamInit->u8HDMISource_Colorspace ));
        pstHDMI_InfoFrameParamInit->u8HDMISource_Colorspace = E_CFD_CFIO_YUV_BT709;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISource u8HDMISource_Colorspace to E_CFD_CFIO_YUV_BT709\n"));
    }

    if (pstHDMI_InfoFrameParamInit->u8HDMISource_Colorspace >= E_CFD_CFIO_RESERVED_START) {
        u16_check_status = E_CFD_MC_ERR_HDMI_INFOFRAME;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource  u8HDMISource_Colorspace is out of range \n",        pstHDMI_InfoFrameParamInit->u8HDMISource_Colorspace ));
        pstHDMI_InfoFrameParamInit->u8HDMISource_Colorspace = E_CFD_CFIO_YUV_BT709;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISource u8HDMISource_Colorspace to E_CFD_CFIO_YUV_BT709\n"));
    }

#if 0
    if (pstHDMI_InfoFrameParamInit->u8HDMISource_AVIInfoFrame_Valid >= E_CFD_VALID_EMUEND) {
        u8_check_status = FALSE;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource  u8HDMISource_AVIInfoFrame_Valid %d is out of range \n",     pstHDMI_InfoFrameParamInit->u8HDMISource_AVIInfoFrame_Valid ));
        pstHDMI_InfoFrameParamInit->u8HDMISource_AVIInfoFrame_Valid = E_CFD_NOT_VALID;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISource u8HDMISource_AVIInfoFrame_Valid to E_CFD_NOT_VALID\n"));
    }
#endif

    if (E_CFD_MC_ERR_NOERR != u16_check_status)
    {
        HDR_DBG_HAL_CFD(printk("\nError:please check: some parameters in HDMI infoFrame structure is wrong, but has been forced to some value\n"));
        HDR_DBG_HAL_CFD(printk("Error code = %04x !!!, error is in [ %s  , %d]\n",u16_check_status, __FUNCTION__,__LINE__));
    }
    u16_check_status = E_CFD_MC_ERR_NOERR;//Easter_test
    return u16_check_status;
}

MS_U16 Mapi_Cfd_inter_HDMI_EDID_Param_Check(STU_CFDAPI_HDMI_EDID_PARSER *pstHDMI_EDIDParamInita)
{
    MS_U16 u16_check_status = E_CFD_MC_ERR_NOERR;
    MS_U8 u8_check_status2 = E_CFD_MC_ERR_NOERR;
    MS_U8 u8_check_status_Chromaticity_Coordinates = TRUE;

    HDR_DBG_HAL_CFD(printk("\n  HDMISink  g_stu_Cfd_HDMI_EDID_Parameters:    \n"));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  version=%d    \n",                            pstHDMI_EDIDParamInita->u32Version ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  length=%d    \n",                             pstHDMI_EDIDParamInita->u16Length ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_HDRData_Block_Valid=%d    \n", pstHDMI_EDIDParamInita->u8HDMISink_HDRData_Block_Valid ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_EOTF=%d    \n",                    pstHDMI_EDIDParamInita->u8HDMISink_EOTF ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_SM=%d    \n",                      pstHDMI_EDIDParamInita->u8HDMISink_SM ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_Desired_Content_Max_Luminance=%d    \n",               pstHDMI_EDIDParamInita->u8HDMISink_Desired_Content_Max_Luminance ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance=%d    \n", pstHDMI_EDIDParamInita->u8HDMISink_Desired_Content_Max_Frame_Avg_Luminance ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_Desired_Content_Min_Luminance=%d    \n",               pstHDMI_EDIDParamInita->u8HDMISink_Desired_Content_Min_Luminance ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_HDRData_Block_Length=%d    \n",                            pstHDMI_EDIDParamInita->u8HDMISink_HDRData_Block_Length ));

    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u16display_primaries_x[0]=%d    \n",  pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[0] ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u16display_primaries_y[0]=%d    \n",  pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[0] ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u16display_primaries_x[1]=%d    \n",  pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[1] ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u16display_primaries_y[1]=%d    \n",  pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[1] ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u16display_primaries_x[2]=%d    \n",  pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_x[2] ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u16display_primaries_y[2]=%d    \n",  pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry.u16Display_Primaries_y[2] ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u16white_point_x=%d    \n",               pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry.u16White_point_x ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u16white_point_y=%d    \n",               pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry.u16White_point_y ));

    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_EDID_base_block_version=%d    \n",     pstHDMI_EDIDParamInita->u8HDMISink_EDID_base_block_version ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_EDID_base_block_reversion=%d    \n",   pstHDMI_EDIDParamInita->u8HDMISink_EDID_base_block_reversion ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_EDID_CEA_block_reversion=%d    \n",        pstHDMI_EDIDParamInita->u8HDMISink_EDID_CEA_block_reversion ));

    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_VCDB_Valid=%d    \n",                      pstHDMI_EDIDParamInita->u8HDMISink_VCDB_Valid ));

    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_Support_YUVFormat=%d    \n",               pstHDMI_EDIDParamInita->u8HDMISink_Support_YUVFormat ));
    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_Extended_Colorspace=%d    \n",         pstHDMI_EDIDParamInita->u8HDMISink_Extended_Colorspace ));

    HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_EDID_Valid=%d    \n",                      pstHDMI_EDIDParamInita->u8HDMISink_EDID_Valid ));

    //add constraints
    //detect values out of range and show messages.

    if (pstHDMI_EDIDParamInita->u8HDMISink_HDRData_Block_Valid >= E_CFD_VALID_EMUEND) {
        u16_check_status = E_CFD_MC_ERR_HDMI_EDID;
        HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_HDRData_Block_Valid %d is out of range \n",        pstHDMI_EDIDParamInita->u8HDMISink_HDRData_Block_Valid ));
        pstHDMI_EDIDParamInita->u8HDMISink_HDRData_Block_Valid = E_CFD_NOT_VALID;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISink u8HDMISink_HDRData_Block_Valid to E_CFD_NOT_VALID\n"));
    }

    MS_U16 u16_xy_highbound = 0x3ff;
    u8_check_status_Chromaticity_Coordinates = MS_Cfd_inter_Check_Chromaticity_Coordinates(&(pstHDMI_EDIDParamInita->stu_Cfd_HDMISink_Panel_ColorMetry),u16_xy_highbound);
//  u8_check_status_Chromaticity_Coordinates = mscfd_HDMI_Check_Chromaticity_Coordinates(&(g_stu_Cfd_HDMI_Infoframe_Parameters.stu_Cfd_Master_Display));

    if ((u8_check_status_Chromaticity_Coordinates & 0x01) == 0x01)
    {
        u16_check_status = E_CFD_MC_ERR_HDMI_EDID_Mastering_Display_Coordinates;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource - the rule of Chromaticity_Coordinates fails, please check\n"));
    }

    if ((u8_check_status_Chromaticity_Coordinates & 0x02) == 0x02)
    {
        u16_check_status = E_CFD_MC_ERR_HDMI_EDID_Mastering_Display_Coordinates;
        HDR_DBG_HAL_CFD(printk("\n  HDMISource - Chromaticity_Coordinates is out of range, force the value to %d\n",u16_xy_highbound));
    }

    if (pstHDMI_EDIDParamInita->u8HDMISink_EDID_base_block_version != 1) {
        u16_check_status = E_CFD_MC_ERR_HDMI_EDID;
        HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_EDID_base_block_version is not correct \n"));
    }

    if (pstHDMI_EDIDParamInita->u8HDMISink_EDID_base_block_reversion != 3) {
        u16_check_status = E_CFD_MC_ERR_HDMI_EDID;
        HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_EDID_base_block_reversion is not correct \n"));
    }

    if (pstHDMI_EDIDParamInita->u8HDMISink_EDID_CEA_block_reversion != 3) {
        u16_check_status = E_CFD_MC_ERR_HDMI_EDID;
        HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_EDID_CEA_block_reversion is not correct \n"));
    }

    if (pstHDMI_EDIDParamInita->u8HDMISink_VCDB_Valid >= E_CFD_VALID_EMUEND) {
        u16_check_status = E_CFD_MC_ERR_HDMI_EDID;
        HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_VCDB_Valid %d is out of range \n",     pstHDMI_EDIDParamInita->u8HDMISink_VCDB_Valid ));
        pstHDMI_EDIDParamInita->u8HDMISink_VCDB_Valid = E_CFD_NOT_VALID;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISink u8HDMISink_VCDB_Valid to E_CFD_NOT_VALID\n"));
    }

    if (pstHDMI_EDIDParamInita->u8HDMISink_EDID_Valid >= E_CFD_VALID_EMUEND) {
        u16_check_status = E_CFD_MC_ERR_HDMI_EDID;
        HDR_DBG_HAL_CFD(printk("\n  HDMISink  u8HDMISink_EDID_Valid %d is out of range \n",     pstHDMI_EDIDParamInita->u8HDMISink_EDID_Valid ));
        pstHDMI_EDIDParamInita->u8HDMISink_EDID_Valid = E_CFD_NOT_VALID;
        HDR_DBG_HAL_CFD(printk("\n  force HDMISink u8HDMISink_EDID_Valid to E_CFD_NOT_VALID\n"));
    }

    if (E_CFD_MC_ERR_NOERR != u16_check_status)
    {
        HDR_DBG_HAL_CFD(printk("\nError:please check:some parameters in HDMI EDID structure is wrong, but has been forced to some value\n"));
        HDR_DBG_HAL_CFD(printk("Error code = %04x !!!, error is in [ %s  , %d]\n",u16_check_status, __FUNCTION__,__LINE__));
    }
    u16_check_status = E_CFD_MC_ERR_NOERR;//Easter_test
    return u16_check_status;
}


//MS_U8 mscfd_MM_Check_Chromaticity_Coordinates(STU_CFD_MASTER_DISPLAY_MM_FORMAT *p_stu_master_display_mm_format)
MS_U8 MS_Cfd_inter_Check_Chromaticity_Coordinates(STU_CFD_COLORIMETRY *p_stu_colormetry, MS_U16 u16_xy_highbound)
{
    //bit 0 : indicate that Rx > Gx > Bx, Gy > Ry > By is followed
    //bit 1 : indicate that default range [0~1] of xy is followed

    MS_U8 u8_check_status_Chromaticity_Coordinates;
    MS_U8 u8_i;

    u8_check_status_Chromaticity_Coordinates = 0;

#if 0 //this rule has been given up
    if ((p_stu_colormetry->u16Display_Primaries_x[0] > p_stu_colormetry->u16Display_Primaries_x[1])&&
        (p_stu_colormetry->u16Display_Primaries_x[1] > p_stu_colormetry->u16Display_Primaries_x[2])&&
        (p_stu_colormetry->u16Display_Primaries_y[1] > p_stu_colormetry->u16Display_Primaries_y[0])&&
        (p_stu_colormetry->u16Display_Primaries_y[0] > p_stu_colormetry->u16Display_Primaries_y[2]))

    {
        u8_check_status_Chromaticity_Coordinates = 0;
    }
    else
    {
        u8_check_status_Chromaticity_Coordinates = 1;
    }
#endif

    for (u8_i = 0; u8_i<=2; u8_i++)
    {
        if (p_stu_colormetry->u16Display_Primaries_x[u8_i] > u16_xy_highbound)
        {
            p_stu_colormetry->u16Display_Primaries_x[u8_i] = u16_xy_highbound;
            u8_check_status_Chromaticity_Coordinates |= 0x02;
        }

        if (p_stu_colormetry->u16Display_Primaries_y[u8_i] > u16_xy_highbound)
        {
            p_stu_colormetry->u16Display_Primaries_y[u8_i] = u16_xy_highbound;
            u8_check_status_Chromaticity_Coordinates |= 0x02;
        }
    }


    if (p_stu_colormetry->u16White_point_x > u16_xy_highbound)
    {
        p_stu_colormetry->u16White_point_x = u16_xy_highbound;
        u8_check_status_Chromaticity_Coordinates |= 0x02;
    }

    if (p_stu_colormetry->u16White_point_y > u16_xy_highbound)
    {
        p_stu_colormetry->u16White_point_y = u16_xy_highbound;
        u8_check_status_Chromaticity_Coordinates |= 0x02;
    }

    return u8_check_status_Chromaticity_Coordinates;
}

void Mapi_Cfd_inter_HW_IPS_Main_Param_Init(STU_CFDAPI_HW_IPS *pstu_HW_IP_Param)
{
    pstu_HW_IP_Param->u8HW_MainSub_Mode = 0;
}

MS_U16 Mapi_Cfd_inter_HW_IPS_Main_Param_Check(STU_CFDAPI_HW_IPS *pstu_HW_IP_Param)
{
    MS_U16 u16_check_status = 0;

    if (pstu_HW_IP_Param->u8HW_MainSub_Mode >= E_CFD_RESERVED_AT0x02) {
        u16_check_status = E_CFD_MC_ERR_HW_Main_Param;
        HDR_DBG_HAL_CFD(printk("\n  HW IP  u8HW_MainSub_Mode %d is out of range \n",        pstu_HW_IP_Param->u8HW_MainSub_Mode ));
        pstu_HW_IP_Param->u8HW_MainSub_Mode = E_CFD_MODE_AT0x00;
        HDR_DBG_HAL_CFD(printk("\n  force HW IP u8HW_MainSub_Mode to 0\n"));
    }

    return u16_check_status;
}

//static STU_CFDAPI_MAIN_CONTROL _stu_CFD_Main_Control;

STU_CFDAPI_MAIN_CONTROL _stu_CFD_Main_Control;


static STU_CFDAPI_MM_PARSER    _stu_Cfd_MM_Paramters;
static STU_CFDAPI_HDMI_INFOFRAME_PARSER _stu_Cfd_HDMI_infoFrame_Paramters;
static STU_CFDAPI_HDMI_EDID_PARSER _stu_Cfd_HDMI_EDID_Panel_Paramters;
static STU_CFDAPI_HDR_METADATA_FORMAT   _stu_HDR_Metadata_Format_Param;
static STU_CFDAPI_PANEL_FORMAT _stu_Panel_Param;
static STU_CFDAPI_OSD_CONTROL  _stu_OSD_Param;
static STU_CFDAPI_HDMI_INFOFRAME_PARSER_OUT _stu_Cfd_HDMI_infoFrame_Paramters_out;

static STU_CFDAPI_OSD_PROCESS_CONFIGS _stu_Cfd_OSD_Process_Configs;

static STU_CFDAPI_HAL_SW_RETURN _stu_SW_Return;
static STU_CFDAPI_HAL_CONTROL _stu_hal_control;

static STU_CFDAPI_Curry_DLCIP   _stu_Curry_DLCIP_Param[2];
static STU_CFDAPI_Curry_TMOIP   _stu_Curry_TMOIP_Param[2];
static STU_CFDAPI_Curry_HDRIP   _stu_Curry_HDRIP_Param[2];
static STU_CFDAPI_Curry_SDRIP   _stu_Curry_SDRIP_Param[2];

static STU_CFDAPI_Kastor_OSDIP _stu_Kastor_OSDIP_Param;

static MS_U8 _u8MainSub_Mode;

void Mapi_Cfd_inter_Main_Control_Param_Set(STU_CFDAPI_MAIN_CONTROL *pstControlParamInit)
{
    //input source change
    if (_stu_CFD_Main_Control.u8Input_Source != pstControlParamInit->u8Input_Source)
    {
        _stu_OSD_Param.u8OSD_UI_Mode = 0;
    }
    u8Input_HDRMode = pstControlParamInit->u8Input_HDRMode;
    _stu_CFD_Main_Control = *pstControlParamInit;

    //printf("Mapi_Cfd_inter_Main_Control_Param_Set:_stu_CFD_Main_Control.u8PredefinedProcess = %d\n", _stu_CFD_Main_Control.u8PredefinedProcess);
}

void Mapi_Cfd_inter_MM_Param_Set(STU_CFDAPI_MM_PARSER *pstMMParamInita)
{
    _stu_Cfd_MM_Paramters = *pstMMParamInita;
}

void Mapi_Cfd_inter_HDMI_InfoFrame_Param_Set(STU_CFDAPI_HDMI_INFOFRAME_PARSER *pstHDMI_INFOFRAME_ParamInita)
{
    _stu_Cfd_HDMI_infoFrame_Paramters = *pstHDMI_INFOFRAME_ParamInita;
}


void Mapi_Cfd_inter_HDMI_InfoFrame_Param_Out_Get(STU_CFDAPI_HDMI_INFOFRAME_PARSER_OUT *pstHDMI_INFOFRAME_OUT_ParamInita)
{
    memcpy(pstHDMI_INFOFRAME_OUT_ParamInita,&_stu_Cfd_HDMI_infoFrame_Paramters_out,sizeof(STU_CFDAPI_HDMI_INFOFRAME_PARSER_OUT));
}


void Mapi_Cfd_inter_OSD_Process_Param_Out_Get(STU_CFDAPI_OSD_PROCESS_CONFIGS *pstu_Cfd_OSD_Process_Configs)
{
    memcpy(pstu_Cfd_OSD_Process_Configs,&_stu_Cfd_OSD_Process_Configs,sizeof(STU_CFDAPI_OSD_PROCESS_CONFIGS));
}

void Mapi_Cfd_inter_OSD_Process_Param_Set(STU_CFDAPI_OSD_PROCESS_CONFIGS *pstu_Cfd_OSD_Process_Configs)
{
    _stu_Cfd_OSD_Process_Configs = *pstu_Cfd_OSD_Process_Configs;
}

void Mapi_Cfd_inter_OSD_Process_Param_Set_OSDonly(STU_CFDAPI_OSD_PROCESS_CONFIGS *pstu_Cfd_OSD_Process_Configs)
{
    _stu_Cfd_OSD_Process_Configs.u8OSD_IsFullRange = pstu_Cfd_OSD_Process_Configs->u8OSD_IsFullRange;

    _stu_Cfd_OSD_Process_Configs.u8OSD_Dataformat = pstu_Cfd_OSD_Process_Configs->u8OSD_Dataformat;

    _stu_Cfd_OSD_Process_Configs.u8OSD_HDRMode = pstu_Cfd_OSD_Process_Configs->u8OSD_HDRMode;

    _stu_Cfd_OSD_Process_Configs.u8OSD_colorprimary = pstu_Cfd_OSD_Process_Configs->u8OSD_colorprimary;

    _stu_Cfd_OSD_Process_Configs.u8OSD_transferf = pstu_Cfd_OSD_Process_Configs->u8OSD_transferf;

    _stu_Cfd_OSD_Process_Configs.u16OSD_MaxLumInNits = pstu_Cfd_OSD_Process_Configs->u16OSD_MaxLumInNits;
}


//modify here

void Mapi_Cfd_inter_HDMI_EDID_Param_Set(STU_CFDAPI_HDMI_EDID_PARSER *pstHDMI_EDIDParamInita)
{
    _stu_Cfd_HDMI_EDID_Panel_Paramters = *pstHDMI_EDIDParamInita;
}

void Mapi_Cfd_inter_HDR_Metadata_Param_Set(STU_CFDAPI_HDR_METADATA_FORMAT *pstu_HDR_Metadata_Param)
{
    _stu_HDR_Metadata_Format_Param = *pstu_HDR_Metadata_Param;
}

void Mapi_Cfd_inter_PANEL_Param_Set(STU_CFDAPI_PANEL_FORMAT *pstPanelParamInita)
{
    _stu_Panel_Param = *pstPanelParamInita;
}

void Mapi_Cfd_inter_OSD_Param_Set(STU_CFDAPI_OSD_CONTROL *pstu_OSD_Param)
{
    _stu_OSD_Param = *pstu_OSD_Param;
}

void Mapi_Cfd_Curry_TMOIP_Param_Set(MS_U8 u8HWMainSubMode, STU_CFDAPI_Curry_TMOIP *pstu_Curry_TMOIP_Param)
{
    _stu_Curry_TMOIP_Param[u8HWMainSubMode] = *pstu_Curry_TMOIP_Param;
}

void Mapi_Cfd_Curry_DLCIP_Param_Set(MS_U8 u8HWMainSubMode, STU_CFDAPI_Curry_DLCIP *pstu_Curry_DLCIP_Param)
{
    _stu_Curry_DLCIP_Param[u8HWMainSubMode] = *pstu_Curry_DLCIP_Param;
}

void Mapi_Cfd_Curry_DLCIP_CurveMode_Set(MS_U8 u8HWMainSubMode, MS_BOOL bUserMode)
{
#if 0
    if (bUserMode)
    {
        _stu_Curry_DLCIP_Param[u8HWMainSubMode].u8DLC_curve_Mode = 0x43;
    }
    else
    {
        _stu_Curry_DLCIP_Param[u8HWMainSubMode].u8DLC_curve_Mode = 0xC0;
    }
#endif
}

void Mapi_Cfd_Curry_SDRIP_Param_Set(MS_U8 u8HWMainSubMode, STU_CFDAPI_Curry_SDRIP *pstu_Curry_SDRIP_Param)
{
    _stu_Curry_SDRIP_Param[u8HWMainSubMode] = *pstu_Curry_SDRIP_Param;
    _u8MainSub_Mode = u8HWMainSubMode;
}

void Mapi_Cfd_Curry_HDRIP_Param_Set(MS_U8 u8HWMainSubMode, STU_CFDAPI_Curry_HDRIP *pstu_Curry_HDRIP_Param)
{
    _stu_Curry_HDRIP_Param[u8HWMainSubMode] = *pstu_Curry_HDRIP_Param;
}

//0: default
//1: Force CFD input to SDR 709
void Mhal_Cfd_Main_Predefined_Control_Set(MS_U8 u8Value)
{
    _stu_CFD_Main_Control.u8PredefinedProcess = u8Value;
}


void Mhal_Cfd_OSD_H2SUI_Set(MS_U8 u8Value)
{
    _stu_OSD_Param.u8HDR_UI_H2SMode = u8Value;
}

MS_BOOL Mhal_Cfd_Hal_Control_Get(STU_CFDAPI_HAL_CONTROL* pst_cfd_hal_control)
{
    //check NULL
    if (NULL == pst_cfd_hal_control)
    {
        return FALSE;
    }

    memcpy(pst_cfd_hal_control,&(_stu_hal_control),sizeof(STU_CFDAPI_HAL_CONTROL));

    return TRUE;
}

void Mhal_Cfd_Hal_SetAutodownloadFirePermission(MS_U8 u8Value)
{
    _stu_hal_control.u8AutodownloadFirePermission = u8Value;
}

MS_BOOL Mhal_Cfd_HDRIP_copy(MS_U8 u8window, STU_CFDAPI_Curry_HDRIP* pstHDRIP)
{
    //check NULL
    if (NULL == pstHDRIP)
    {
        return FALSE;
    }

    memcpy(pstHDRIP,&(_stu_Curry_HDRIP_Param[u8window]),sizeof(STU_CFDAPI_Curry_HDRIP));

    return TRUE;
}

void Mhal_CFD_PredefinedInputCase(STU_CFDAPI_TOP_CONTROL *pstu_Cfd_api_top)
{
    MS_U8 u8temp;

    u8temp = pstu_Cfd_api_top->pstu_Main_Control->u8PredefinedProcess;

    //predefined case
    if (1 == u8temp)
    {
        Mapi_Cfd_testvector501(pstu_Cfd_api_top);
    }
}

void MS_Cfd_OSD_Process_configs_print(STU_CFDAPI_OSD_PROCESS_CONFIGS *pstu_OSD_Process_Configs)
{
    printf("\n  OSD:  \n");
    printf("\n  OSD:  u8OSD_SDR2HDR_en=%d;    \n", pstu_OSD_Process_Configs->u8OSD_SDR2HDR_en );
    printf("\n  OSD:  u8OSD_IsFullRange=%d;    \n", pstu_OSD_Process_Configs->u8OSD_IsFullRange );
    printf("\n  OSD:  u8OSD_Dataformat=%d;    \n", pstu_OSD_Process_Configs->u8OSD_Dataformat );
    printf("\n  OSD:  u8OSD_HDRMode=%d;    \n", pstu_OSD_Process_Configs->u8OSD_HDRMode );
    printf("\n  OSD:  u8OSD_colorprimary=%d;    \n", pstu_OSD_Process_Configs->u8OSD_colorprimary );
    printf("\n  OSD:  u8OSD_transferf=%d;    \n", pstu_OSD_Process_Configs->u8OSD_transferf );
    printf("\n  OSD:  u16OSD_MaxLumInNits=%d;    \n", pstu_OSD_Process_Configs->u16OSD_MaxLumInNits );
    printf("\n  OSD:  u16AntiTMO_SourceInNits=%d;    \n", pstu_OSD_Process_Configs->u16AntiTMO_SourceInNits );
    printf("\n");
    printf("\n  OSD:  u8Video_colorprimary=%d;    \n", pstu_OSD_Process_Configs->u8Video_colorprimary );
    printf("\n  OSD:  u8Video_MatrixCoefficients=%d;    \n", pstu_OSD_Process_Configs->u8Video_MatrixCoefficients );
    printf("\n  OSD:  u8Video_HDRMode=%d;    \n", pstu_OSD_Process_Configs->u8Video_HDRMode );
    printf("\n  OSD:  u16Video_MaxLumInNits=%d;    \n", pstu_OSD_Process_Configs->u16Video_MaxLumInNits );
    printf("\n  OSD:  u8Video_IsFullRange=%d;    \n", pstu_OSD_Process_Configs->u8Video_IsFullRange );
    printf("\n  OSD:  u8Video_Dataformat=%d;    \n", pstu_OSD_Process_Configs->u8Video_Dataformat );

#if (0 == RealChip)

//produce test vectors

    FILE *fp;

    fp = fopen("testvector_OSD_process.txt","wt");

    fprintf(fp,"\n\n");
    fprintf(fp,"\n pstu_OSD_Process_Configs->u8OSD_SDR2HDR_en=%d    \n", pstu_OSD_Process_Configs->u8OSD_SDR2HDR_en );
    fprintf(fp,"\n pstu_OSD_Process_Configs->u8OSD_IsFullRange=%d    \n", pstu_OSD_Process_Configs->u8OSD_IsFullRange );
    fprintf(fp,"\n pstu_OSD_Process_Configs->u8OSD_Dataformat=%d    \n", pstu_OSD_Process_Configs->u8OSD_Dataformat );
    fprintf(fp,"\n pstu_OSD_Process_Configs->u8OSD_HDRMode=%d    \n", pstu_OSD_Process_Configs->u8OSD_HDRMode );
    fprintf(fp,"\n pstu_OSD_Process_Configs->u8OSD_colorprimary=%d    \n", pstu_OSD_Process_Configs->u8OSD_colorprimary );
    fprintf(fp,"\n pstu_OSD_Process_Configs->u8OSD_transferf=%d    \n", pstu_OSD_Process_Configs->u8OSD_transferf );
    fprintf(fp,"\n pstu_OSD_Process_Configs->u16OSD_MaxLumInNits=%d    \n", pstu_OSD_Process_Configs->u16OSD_MaxLumInNits );
    fprintf(fp,"\n");
    fprintf(fp,"\n pstu_OSD_Process_Configs->u8Video_colorprimary=%d    \n", pstu_OSD_Process_Configs->u8Video_colorprimary );
    fprintf(fp,"\n pstu_OSD_Process_Configs->u8Video_MatrixCoefficients=%d    \n", pstu_OSD_Process_Configs->u8Video_MatrixCoefficients );
    fprintf(fp,"\n pstu_OSD_Process_Configs->u8Video_HDRMode=%d    \n", pstu_OSD_Process_Configs->u8Video_HDRMode );
    fprintf(fp,"\n pstu_OSD_Process_Configs->u16Video_MaxLumInNits=%d    \n", pstu_OSD_Process_Configs->u16Video_MaxLumInNits );
    fprintf(fp,"\n pstu_OSD_Process_Configs->u8Video_IsFullRange=%d    \n", pstu_OSD_Process_Configs->u8Video_IsFullRange );
    fprintf(fp,"\n pstu_OSD_Process_Configs->u8Video_Dataformat=%d    \n", pstu_OSD_Process_Configs->u8Video_Dataformat );

    fclose(fp);

#endif

}

//copy structure to hal level
//only for K6
void Mhal_CFD_OSD_Configs_copy(STU_CFDAPI_OSD_PROCESS_Input *pstu_Cfd_OSD_Process_Input, STU_CFDAPI_OSD_PROCESS_CONFIGS *pstu_Cfd_OSD_Process_Configs)
{
    pstu_Cfd_OSD_Process_Input->u8OSD_SDR2HDR_en = pstu_Cfd_OSD_Process_Configs->u8OSD_SDR2HDR_en;

    pstu_Cfd_OSD_Process_Input->u8OSD_IsFullRange = pstu_Cfd_OSD_Process_Configs->u8OSD_IsFullRange;

    pstu_Cfd_OSD_Process_Input->u8OSD_Dataformat = pstu_Cfd_OSD_Process_Configs->u8OSD_Dataformat;

    pstu_Cfd_OSD_Process_Input->u8OSD_HDRMode = pstu_Cfd_OSD_Process_Configs->u8OSD_HDRMode;

    pstu_Cfd_OSD_Process_Input->u8OSD_colorprimary = pstu_Cfd_OSD_Process_Configs->u8OSD_colorprimary;

    pstu_Cfd_OSD_Process_Input->u8OSD_transferf = pstu_Cfd_OSD_Process_Configs->u8OSD_transferf;

    pstu_Cfd_OSD_Process_Input->u16OSD_MaxLumInNits = pstu_Cfd_OSD_Process_Configs->u16OSD_MaxLumInNits;

    pstu_Cfd_OSD_Process_Input->u8Video_colorprimary = pstu_Cfd_OSD_Process_Configs->u8Video_colorprimary;

    pstu_Cfd_OSD_Process_Input->u8Video_MatrixCoefficients = pstu_Cfd_OSD_Process_Configs->u8Video_MatrixCoefficients;

    pstu_Cfd_OSD_Process_Input->u8Video_HDRMode = pstu_Cfd_OSD_Process_Configs->u8Video_HDRMode;

    pstu_Cfd_OSD_Process_Input->u16Video_MaxLumInNits = pstu_Cfd_OSD_Process_Configs->u16Video_MaxLumInNits;

    pstu_Cfd_OSD_Process_Input->u8Video_IsFullRange = pstu_Cfd_OSD_Process_Configs->u8Video_IsFullRange;

    pstu_Cfd_OSD_Process_Input->u8Video_Dataformat = pstu_Cfd_OSD_Process_Configs->u8Video_Dataformat;

    pstu_Cfd_OSD_Process_Input->u16AntiTMO_SourceInNits = pstu_Cfd_OSD_Process_Configs->u16AntiTMO_SourceInNits;

}

MS_U16  Mhal_CFD_OSD_process(STU_CFDAPI_HDMI_INFOFRAME_PARSER_OUT *pstu_HDMI_InfoFrame_Param_out, STU_CFDAPI_Kastor_OSDIP  *pstu_Kastor_OSDIP_Param,STU_CFDAPI_OSD_PROCESS_CONFIGS  *pstu_Cfd_OSD_Process_Configs)
{
    MS_U16 u16temp;
    MS_U8   OSD_control_check_status;

    STU_CFDAPI_OSD_PROCESS_Input stu_OSD_input;

    memset(&stu_OSD_input,0x00,sizeof(STU_CFDAPI_OSD_PROCESS_Input));

    OSD_control_check_status = 0;

#if (1 == CFD_OSD_DummyRegs_Debug)

    MS_U8 u8byte0 = 0;
    MS_U8 u8byte1 = 0;
    MS_U8 u8byte2 = 0;
    MS_U8 u8byte3 = 0;

#if RealChip
    u8byte1 = msReadByte( _PK_H_(0x79, 0x7c));
    u8byte0 = msReadByte( _PK_L_(0x79, 0x7c));
    u8byte3 = msReadByte( _PK_H_(0x79, 0x7d));
    u8byte2 = msReadByte( _PK_L_(0x79, 0x7d));
#endif

    pstu_Kastor_OSDIP_Param->u8UserMode = u8byte1;

 #endif

//update OSD spec here
//call xxx_function()

OSD_control_check_status = Mapi_Cfd_inter_OSD_SDR2HDR_configures_check(pstu_Cfd_OSD_Process_Configs);

if (OSD_control_check_status == 1)
{
    //printk("\033[1;35m###[%s][%d]### OSD S2H somthing wrong!!!!!!!!!!!!!!!!!\033[0m\n",__FUNCTION__,__LINE__);
}

    MS_Cfd_OSD_Process_configs_print(pstu_Cfd_OSD_Process_Configs);
    Mhal_CFD_OSD_Configs_copy(&stu_OSD_input,pstu_Cfd_OSD_Process_Configs);

    u16temp = 0;

    //Process on/off
    if (0 == pstu_Kastor_OSDIP_Param->u8UserMode)
    {
        pstu_Kastor_OSDIP_Param->u8Status = 0; //bypass
    }
    else if (1 == pstu_Kastor_OSDIP_Param->u8UserMode)
    {
        pstu_Kastor_OSDIP_Param->u8Status = 1; //SDR2HDR
    }
    else
    {
        //if (2 == pstu_HDMI_InfoFrame_Param_out->u8HDMISource_EOTF)
        if (1 == pstu_Cfd_OSD_Process_Configs->u8OSD_SDR2HDR_en)
        {
            pstu_Kastor_OSDIP_Param->u8Status = 1; //SDR2HDR
        }
        else
        {
            pstu_Kastor_OSDIP_Param->u8Status = 0; //bypass
        }
    }

    //P lut
    pstu_Kastor_OSDIP_Param->u8OSD_Plut_Mode = 1;

    //gamma table
    pstu_Kastor_OSDIP_Param->u8OSD_Gamma_SRAM_Mode = 1;

#if (1 == CFD_OSD_DummyRegs_Debug)

    pstu_Kastor_OSDIP_Param->u8Status = u8byte0;
    pstu_Kastor_OSDIP_Param->u8OSD_Plut_Mode = u8byte3;
    pstu_Kastor_OSDIP_Param->u8OSD_Gamma_SRAM_Mode = u8byte2;

#endif


#if 1

printf("\033[1;35m###[CFD OSD][%s][%d]### u8UserMode = %d!!!!!!!!!!!!!!!!!\033[0m\n",__FUNCTION__,__LINE__,pstu_Kastor_OSDIP_Param->u8UserMode);
printf("\033[1;35m###[CFD OSD][%s][%d]### u8HDMISource_EOTF = %d!!!!!!!!!!!!!!!!!\033[0m\n",__FUNCTION__,__LINE__,pstu_HDMI_InfoFrame_Param_out->u8HDMISource_EOTF);
printf("\033[1;35m###[CFD OSD][%s][%d]### u8Status = %d!!!!!!!!!!!!!!!!!\033[0m\n",__FUNCTION__,__LINE__,pstu_Kastor_OSDIP_Param->u8Status);
printf("\033[1;35m###[CFD OSD][%s][%d]### u8OSD_Plut_Mode = %d!!!!!!!!!!!!!!!!!\033[0m\n",__FUNCTION__,__LINE__,pstu_Kastor_OSDIP_Param->u8OSD_Plut_Mode);
printf("\033[1;35m###[CFD OSD][%s][%d]### u8OSD_Gamma_SRAM_Mode = %d !!!!!!!!!!!!!!!!!\033[0m\n",__FUNCTION__,__LINE__,pstu_Kastor_OSDIP_Param->u8OSD_Gamma_SRAM_Mode);



    u16temp = MS_Cfd_Kastor_OSD_process(pstu_Kastor_OSDIP_Param,&stu_OSD_input);

#endif

    return u16temp;
}


void Color_Format_Driver(void)
{
    //printf("\033[1;35m###[River][%s][%d]### ML test!!!!!!!!!!!!!!!!!\033[0m\n",__FUNCTION__,__LINE__);
    //if (1)//!MHal_XC_CheckMuteStatusByRegister(MAIN_WINDOW) && !MHal_XC_CheckMuteStatusByRegister(SUB_WINDOW))
    //{
    //    printf("\033[1;35m###[River][%s][%d]### By ML!!!!!!!!!!!!!!!!!\033[0m\n",__FUNCTION__,__LINE__);
        //KApi_XC_MLoad_WriteCmd(E_CLIENT_MAIN_HDR, DoVi_Addr_L(0x06, 0x40), 0xdead, 0xFFFF);
        //KApi_XC_MLoad_WriteCmd(E_CLIENT_MAIN_HDR, DoVi_Addr_L(0x06, 0x41), 0xbeef, 0xFFFF);
        //KApi_XC_MLoad_WriteCmd(E_CLIENT_MAIN_HDR, DoVi_Addr_L(0x06, 0x60), 0xbeef, 0xFFFF);
        //KApi_XC_MLoad_WriteCmd(E_CLIENT_MAIN_HDR, DoVi_Addr_L(0x06, 0x61), 0xdead, 0xFFFF);
    //}
    //else
    //{
        //printf("\033[1;35m###[River][%s][%d]### By RIU!!!!!!!!!!!!!!!!!\033[0m\n",__FUNCTION__,__LINE__);
        //MApi_GFLIP_XC_W2BYTE( ADDR, VAL );
    //}
    //printf("\033[1;35m###[River][%s][%d]### Fire test!!!!!!!!!!!!!!!!!\033[0m\n",__FUNCTION__,__LINE__);
    //KApi_XC_MLoad_Fire(E_CLIENT_MAIN_HDR, TRUE);
    //printf("\033[1;35m###[River][%s][%d]### leave ML test!!!!!!!!!!!!!!!!!\033[0m\n",__FUNCTION__,__LINE__);
    //return;

    printf("\033[1;35m###[Brian][%s][%d]### Into CFD 0905 pm0304!!!!!!!!!!!!!!!!!\033[0m\n",__FUNCTION__,__LINE__);


    MS_U32 u32MMParamInit[18] = {0};
    MS_U16 u16HDMIInfoFrameParamInit[19] = {0};
    MS_U16 u16HDMIEDIDParamInit[21] = {0};
    MS_U16 u16_call_status = 0;
    MS_U16 u16_call_status_OSD = 0;

    MS_U32 u32temp;
    MS_U16 u16In;
    //float dd;
    //=============================================================================
    //TMO
    //STU_CFDAPI_TMO_CONTROL  stu_TMO_Param;
    //memset(&stu_TMO_Param, 0, sizeof(STU_CFDAPI_TMO_CONTROL));
    //stu_TMO_Param.u16Length = sizeof(STU_CFDAPI_TMO_CONTROL);
    //stu_TMO_Param.u32Version = CFD_HDMI_TMO_ST_VERSION;

#if (0)
    //output infoFrame
    STU_CFDAPI_HDMI_INFOFRAME_PARSER_OUT stu_Cfd_HDMI_infoFrame_Paramters_out;
    memset(&stu_Cfd_HDMI_infoFrame_Paramters_out, 0, sizeof(STU_CFDAPI_HDMI_INFOFRAME_PARSER_OUT));
    stu_Cfd_HDMI_infoFrame_Paramters_out.u16Length = sizeof(STU_CFDAPI_HDMI_INFOFRAME_PARSER_OUT);
    stu_Cfd_HDMI_infoFrame_Paramters_out.u32Version = CFD_HDMI_INFOFRAME_OUT_ST_VERSION;

    //HW for Kano
    STU_CFDAPI_Curry_DLCIP  stu_Curry_DLCIP_Param;
    memset(&stu_Curry_DLCIP_Param, 0xC0, sizeof(STU_CFDAPI_Curry_DLCIP));
    stu_Curry_DLCIP_Param.u16Length = sizeof(STU_CFDAPI_Curry_DLCIP);
    stu_Curry_DLCIP_Param.u32Version = CFD_CURRY_DLCIP_ST_VERSION;

    STU_CFDAPI_Curry_TMOIP   stu_Curry_TMOIP_Param;
    memset(&stu_Curry_TMOIP_Param, 0xC0, sizeof(STU_CFDAPI_Curry_TMOIP));
    stu_Curry_TMOIP_Param.u16Length = sizeof(STU_CFDAPI_Curry_TMOIP);
    stu_Curry_TMOIP_Param.u32Version = CFD_CURRY_TMOIP_ST_VERSION;

    STU_CFDAPI_Curry_HDRIP   stu_Curry_HDRIP_Param;
    memset(&stu_Curry_HDRIP_Param, 0xC0, sizeof(STU_CFDAPI_Curry_HDRIP));
    stu_Curry_HDRIP_Param.u16Length = sizeof(STU_CFDAPI_Curry_HDRIP);
    stu_Curry_HDRIP_Param.u32Version = CFD_CURRY_HDRIP_ST_VERSION;

    STU_CFDAPI_Curry_SDRIP   stu_Curry_SDRIP_Param;
    memset(&stu_Curry_SDRIP_Param, 0xC0, sizeof(STU_CFDAPI_Curry_SDRIP));
    stu_Curry_SDRIP_Param.u16Length = sizeof(STU_CFDAPI_Curry_SDRIP);
    stu_Curry_SDRIP_Param.u32Version = CFD_CURRY_SDRIP_ST_VERSION;
#endif


#if (0)
    //HW for Kano
    STU_CFDAPI_Kano_DLCIP  stu_Kano_DLCIP_Param;
    memset(&stu_Kano_DLCIP_Param, 0xC0, sizeof(STU_CFDAPI_Kano_DLCIP));
    stu_Kano_DLCIP_Param.u16Length = sizeof(STU_CFDAPI_Kano_DLCIP);
    stu_Kano_DLCIP_Param.u32Version = CFD_KANO_DLCIP_ST_VERSION;

    STU_CFDAPI_Kano_TMOIP   stu_Kano_TMOIP_Param;
    memset(&stu_Kano_TMOIP_Param, 0xC0, sizeof(STU_CFDAPI_Kano_TMOIP));
    stu_Kano_TMOIP_Param.u16Length = sizeof(STU_CFDAPI_Kano_TMOIP);
    stu_Kano_TMOIP_Param.u32Version = CFD_KANO_TMOIP_ST_VERSION;

    STU_CFDAPI_Kano_HDRIP   stu_Kano_HDRIP_Param;
    memset(&stu_Kano_HDRIP_Param, 0xC0, sizeof(STU_CFDAPI_Kano_HDRIP));
    stu_Kano_HDRIP_Param.u16Length = sizeof(STU_CFDAPI_Kano_HDRIP);
    stu_Kano_HDRIP_Param.u32Version = CFD_KANO_HDRIP_ST_VERSION;

    STU_CFDAPI_Kano_SDRIP   stu_Kano_SDRIP_Param;
    memset(&stu_Kano_SDRIP_Param, 0xC0, sizeof(STU_CFDAPI_Kano_SDRIP));
    stu_Kano_SDRIP_Param.u16Length = sizeof(STU_CFDAPI_Kano_SDRIP);
    stu_Kano_SDRIP_Param.u32Version = CFD_KANO_SDRIP_ST_VERSION;
#endif

#if (0)
    //HW for Manhattan
    STU_CFDAPI_Manhattan_TMOIP   stu_Manhattan_TMOIP_Param;
    memset(&stu_Manhattan_TMOIP_Param, 0xFF, sizeof(STU_CFDAPI_Manhattan_TMOIP));
    stu_Manhattan_TMOIP_Param.u16Length = sizeof(STU_CFDAPI_Manhattan_TMOIP);
    stu_Manhattan_TMOIP_Param.u32Version = CFD_Manhattan_TMOIP_ST_VERSION;

    STU_CFDAPI_Manhattan_HDRIP   stu_Manhattan_HDRIP_Param;
    memset(&stu_Manhattan_HDRIP_Param, 0xFF, sizeof(STU_CFDAPI_Manhattan_HDRIP));
    stu_Manhattan_HDRIP_Param.u16Length = sizeof(STU_CFDAPI_Manhattan_HDRIP);
    stu_Manhattan_HDRIP_Param.u32Version = CFD_Manhattan_HDRIP_ST_VERSION;

    STU_CFDAPI_Manhattan_SDRIP   stu_Manhattan_SDRIP_Param;
    memset(&stu_Manhattan_SDRIP_Param, 0xFF, sizeof(STU_CFDAPI_Manhattan_SDRIP));
    stu_Manhattan_SDRIP_Param.u16Length = sizeof(STU_CFDAPI_Manhattan_SDRIP);
    stu_Manhattan_SDRIP_Param.u32Version = CFD_Manhattan_SDRIP_ST_VERSION;
#endif

#if (0)
//#if NowHW == Maserati
    //HW for Maserati
    STU_CFDAPI_Maserati_DLCIP   stu_Maserati_DLCIP_Param;
    memset(&stu_Maserati_DLCIP_Param, 0xC0, sizeof(STU_CFDAPI_Maserati_DLCIP));
    stu_Maserati_DLCIP_Param.u16Length = sizeof(STU_CFDAPI_Maserati_DLCIP);
    stu_Maserati_DLCIP_Param.u32Version = CFD_Maserati_DLCIP_ST_VERSION;

    STU_CFDAPI_Maserati_TMOIP   stu_Maserati_TMOIP_Param;
    memset(&stu_Maserati_TMOIP_Param, 0xC0, sizeof(STU_CFDAPI_Maserati_TMOIP));
    stu_Maserati_TMOIP_Param.u16Length = sizeof(STU_CFDAPI_Maserati_TMOIP);
    stu_Maserati_TMOIP_Param.u32Version = CFD_Maserati_TMOIP_ST_VERSION;

    STU_CFDAPI_Maserati_HDRIP   stu_Maserati_HDRIP_Param;
    memset(&stu_Maserati_HDRIP_Param, 0xC0, sizeof(STU_CFDAPI_Maserati_HDRIP));
    stu_Maserati_HDRIP_Param.u16Length = sizeof(STU_CFDAPI_Maserati_HDRIP);
    stu_Maserati_HDRIP_Param.u32Version = CFD_Maserati_HDRIP_ST_VERSION;


    //only main
#if 0
    STU_CFDAPI_Maserati_SDRIP   stu_Maserati_SDRIP_Param;
    memset(&stu_Maserati_SDRIP_Param, 0xC0, sizeof(STU_CFDAPI_Maserati_SDRIP));
    stu_Maserati_SDRIP_Param.u16Length = sizeof(STU_CFDAPI_Maserati_SDRIP);
    stu_Maserati_SDRIP_Param.u32Version = CFD_Maserati_SDRIP_ST_VERSION;
#endif

    //for Main/sub
    //stu_Maserati_SDRIP_Param[0] is for SC0 (main)
    //stu_Maserati_SDRIP_Param[1] is for SC1 (sub)
    STU_CFDAPI_Maserati_SDRIP   stu_Maserati_SDRIP_Param[2];
    memset(&stu_Maserati_SDRIP_Param[0], 0xC0, sizeof(STU_CFDAPI_Maserati_SDRIP));
    stu_Maserati_SDRIP_Param[0].u16Length = sizeof(STU_CFDAPI_Maserati_SDRIP);
    stu_Maserati_SDRIP_Param[0].u32Version = CFD_Maserati_SDRIP_ST_VERSION;
    memset(&stu_Maserati_SDRIP_Param[1], 0xC0, sizeof(STU_CFDAPI_Maserati_SDRIP));
    stu_Maserati_SDRIP_Param[1].u16Length = sizeof(STU_CFDAPI_Maserati_SDRIP);
    stu_Maserati_SDRIP_Param[1].u32Version = CFD_Maserati_SDRIP_ST_VERSION;

#endif


    //==========================================================================================
    Mapi_Cfd_inter_OSD_Process_configures_init(&_stu_Cfd_OSD_Process_Configs);

    //Top control
    STU_CFDAPI_TOP_CONTROL stu_CfdAPI_Top_Param;
    //memset(&stu_CfdAPI_Top_Param, 0x00, sizeof(stu_CfdAPI_Top_Param));
    stu_CfdAPI_Top_Param.pstu_Main_Control = &_stu_CFD_Main_Control;
    stu_CfdAPI_Top_Param.pstu_MM_Param     = &_stu_Cfd_MM_Paramters;
    stu_CfdAPI_Top_Param.pstu_HDMI_EDID_Param = &_stu_Cfd_HDMI_EDID_Panel_Paramters;
    stu_CfdAPI_Top_Param.pstu_HDMI_InfoFrame_Param = &_stu_Cfd_HDMI_infoFrame_Paramters;
    stu_CfdAPI_Top_Param.pstu_HDR_Metadata_Format_Param = &_stu_HDR_Metadata_Format_Param;
    stu_CfdAPI_Top_Param.pstu_Panel_Param = &_stu_Panel_Param;
    stu_CfdAPI_Top_Param.pstu_OSD_Param = &_stu_OSD_Param;
    //stu_CfdAPI_Top_Param.pstu_TMO_Param = &stu_TMO_Param;

    //should modify here
    stu_CfdAPI_Top_Param.pstu_HDMI_InfoFrame_Param_out = &_stu_Cfd_HDMI_infoFrame_Paramters_out;
    stu_CfdAPI_Top_Param.pstu_SW_Return = &_stu_SW_Return;

    stu_CfdAPI_Top_Param.pstu_OSD_Process_Configs = &_stu_Cfd_OSD_Process_Configs;

    //HW IP control

    STU_CFD_MS_ALG_INTERFACE_DLC stu_DLC_Input;
    STU_CFD_MS_ALG_INTERFACE_TMO stu_TMO_Input;
    STU_CFD_MS_ALG_INTERFACE_HDRIP stu_HDRIP_Input;
    STU_CFD_MS_ALG_INTERFACE_SDRIP stu_SDRIP_Input;

//#if (NowHW == Kastor)
//    STU_CFD_MS_ALG_INTERFACE_OSDIP stu_OSDIP_Input;
//#endif

    STU_CFDAPI_HW_IPS stu_HW_IP_Param;
    stu_HW_IP_Param.pstu_DLC_Input = &stu_DLC_Input;
    stu_HW_IP_Param.pstu_TMO_Input = &stu_TMO_Input;
    stu_HW_IP_Param.pstu_HDRIP_Input = &stu_HDRIP_Input;
    stu_HW_IP_Param.pstu_SDRIP_Input = &stu_SDRIP_Input;
    stu_CfdAPI_Top_Param.pstu_HW_IP_Param = &stu_HW_IP_Param;

//#if (NowHW == Kastor)
//    stu_HW_IP_Param.pstu_OSDIP_Input = &stu_OSDIP_Input;
//#endif

#if 1
    memcpy(&(stu_HDRIP_Input.stu_Curry_HDRIP_Param),&(_stu_Curry_HDRIP_Param[0]),sizeof(STU_CFDAPI_Curry_HDRIP));

    //for SC0 (main)
    memcpy(&(stu_SDRIP_Input.stu_Curry_SDRIP_Param),&(_stu_Curry_SDRIP_Param[0]),sizeof(STU_CFDAPI_Curry_SDRIP));
    //for SC1 (sub)
    //memcpy(&(stu_SDRIP_Input.stu_Curry_SDRIP_Param),&(stu_Curry_SDRIP_Param[1]),sizeof(STU_CFDAPI_Curry_SDRIP));

    memcpy(&(stu_TMO_Input.stu_Curry_TMOIP_Param),&_stu_Curry_TMOIP_Param[0],sizeof(STU_CFDAPI_Curry_TMOIP));
    memcpy(&(stu_DLC_Input.stu_Curry_DLC_Param),&_stu_Curry_DLCIP_Param[0],sizeof(STU_CFDAPI_Curry_DLCIP));

    //memcpy(&(stu_OSDIP_Input.stu_Kastor_OSDIP_Param),&stu_Kastor_OSDIP_Param,sizeof(STU_CFDAPI_Kastor_OSDIP));

#endif

    Mapi_Cfd_inter_HW_IPS_Main_Param_Init(&stu_HW_IP_Param);

#if RealChip
    stu_CfdAPI_Top_Param.pstu_HW_IP_Param->u8HW_MainSub_Mode = _u8MainSub_Mode;
#endif

    u16_call_status = Mapi_Cfd_inter_HW_IPS_Main_Param_Check(&stu_HW_IP_Param);

    //test vector =======================================================
    if(E_CFD_INPUT_SOURCE_HDMI == stu_CfdAPI_Top_Param.pstu_Main_Control->u8Input_Source)
    {
        if ( 1 == stu_CfdAPI_Top_Param.pstu_HDMI_InfoFrame_Param->u8HDMISource_HDR_InfoFrame_Valid &&
            0 == stu_CfdAPI_Top_Param.pstu_HDMI_InfoFrame_Param->u8HDMISource_SMD_ID)//HDMI need add mm case wait for SW
    {
        STU_HDR_METADATA.u8MetaData_Valid = 1;
        STU_HDR_METADATA.u16MaxContentLightLevel = stu_CfdAPI_Top_Param.pstu_HDMI_InfoFrame_Param->u16Max_Content_Light_Level;
        STU_HDR_METADATA.u16MaxFrameAverageLightLevel = stu_CfdAPI_Top_Param.pstu_HDMI_InfoFrame_Param->u16Max_Frame_Avg_Light_Level;
    }
    else
    {
        STU_HDR_METADATA.u8MetaData_Valid = 0;
            STU_HDR_METADATA.u16MaxContentLightLevel = 4000;
            STU_HDR_METADATA.u16MaxFrameAverageLightLevel = 0;
        }
    }
    else if(E_CFD_INPUT_SOURCE_STORAGE == stu_CfdAPI_Top_Param.pstu_Main_Control->u8Input_Source)
    {
        if(1 == stu_CfdAPI_Top_Param.pstu_MM_Param->u8MM_HDR_ContentLightMetaData_Valid)//mm case
        {
            STU_HDR_METADATA.u8MetaData_Valid = 1;
            STU_HDR_METADATA.u16MaxContentLightLevel = stu_CfdAPI_Top_Param.pstu_MM_Param->u16Max_content_light_level;
            STU_HDR_METADATA.u16MaxFrameAverageLightLevel = stu_CfdAPI_Top_Param.pstu_MM_Param->u16Max_pic_average_light_level;
        }
        else
        {
            STU_HDR_METADATA.u8MetaData_Valid = 0;
            STU_HDR_METADATA.u16MaxContentLightLevel = 4000;
        STU_HDR_METADATA.u16MaxFrameAverageLightLevel = 0;
    }
    }

    MS_U8 u8TestMode = 0;

    MS_U8 u8byte0 = 0;
    MS_U8 u8byte1 = 0;
    MS_U8 u8byte2 = 0;
    MS_U8 u8byte3 = 0;

#if RealChip
    //dummy in SOC
    //if(MApi_GFLIP_XC_R2BYTE(REG_SC_BK30_01_L) == 0x001C)
    //{
    //  u8TestMode = msReadByte(REG_SC_Ali_BK30_0D_H);
    //}

#if (1 == CFD_MAININOPUT_DummyRegs_Debug)

    u8byte1 = msReadByte( _PK_H_(0x79, 0x7c));
    u8byte0 = msReadByte( _PK_L_(0x79, 0x7c));
    u8byte3 = msReadByte( _PK_H_(0x79, 0x7d));
    u8byte2 = msReadByte( _PK_L_(0x79, 0x7d));

#endif

    //assign
    //link to UI
    //stu_CfdAPI_Top_Param.pstu_Main_Control->u8TMO_TargetRefer_Mode = u8byte1&0x03;

#endif

    //printf("\n test : PK_H_ : %x\n",_PK_H_(0x79, 0x7C));
    printf("\n test : u8byte0 = %x\n",u8byte0);
    printf("\n test : u8byte1 = %x\n",u8byte1);
    printf("\n test : u8byte2 = %x\n",u8byte2);
    printf("\n test : u8byte3 = %x\n",u8byte3);

    //Mhal_Cfd_Main_Predefined_Control_Set(u8byte3);

    printf("\n test2 : _stu_CFD_Main_Control.u8PredefinedProcess = %d\n",_stu_CFD_Main_Control.u8PredefinedProcess);

    //printk("\n test : u8byte0 = %x\n",u8byte0);
    //printk("\n test : u8byte1 = %x\n",u8byte1);
    //printk("\n test : u8byte2 = %x\n",u8byte2);
    //printk("\n test : u8byte3 = %x\n",u8byte3);

    if(0 != u8TestMode )
    {
        u8TempVector = u8TestMode;
    }

    Mapi_Cfd_testvector_base(&stu_CfdAPI_Top_Param);


    switch(u8byte0)
    {
        case 0:
            //Mapi_Cfd_Main_Control_Param_Init_test000(&_stu_CFD_Main_Control,&stu_CfdAPI_Top_Param);
            //RGB full in , SDR in , no GM , HDMI out
            //printf("\n test : case0\n");
            //Mapi_Cfd_testvector061(&stu_CfdAPI_Top_Param);
            break;
        case 1:
         //RGB limit in , SDR in , no GM , IsRGBvypass = 0, HDMI out
         printf("\n test : case1\n");
            //Mapi_Cfd_testvector062(&stu_CfdAPI_Top_Param);
            //Mapi_Cfd_testvector001(&stu_CfdAPI_Top_Param);
            Mapi_Cfd_testvector450(&stu_CfdAPI_Top_Param);
         break;
        case 2:
         printf("\n test : case2\n");
            //Mapi_Cfd_testvector063(&stu_CfdAPI_Top_Param);
         //Mapi_Cfd_testvector002(&stu_CfdAPI_Top_Param);
         Mapi_Cfd_testvector451(&stu_CfdAPI_Top_Param);
            break;
        case 3:
         printf("\n test : case3\n");
         //Mapi_Cfd_testvector064(&stu_CfdAPI_Top_Param);
            //Mapi_Cfd_testvector003(&stu_CfdAPI_Top_Param);
            Mapi_Cfd_testvector452(&stu_CfdAPI_Top_Param);
            break;
        case 4:
         printf("\n test : case4\n");
         //Mapi_Cfd_testvector065(&stu_CfdAPI_Top_Param);
            //Mapi_Cfd_testvector004(&stu_CfdAPI_Top_Param);
            Mapi_Cfd_testvector453(&stu_CfdAPI_Top_Param);
            break;
        case 5:
         printf("\n test : case5\n");
         //Mapi_Cfd_testvector066(&stu_CfdAPI_Top_Param);
            //Mapi_Cfd_testvector005(&stu_CfdAPI_Top_Param);
            Mapi_Cfd_testvector454(&stu_CfdAPI_Top_Param);
            break;
        case 6:
         printf("\n test : case6\n");
         //Mapi_Cfd_testvector067(&stu_CfdAPI_Top_Param);
            //Mapi_Cfd_testvector006(&stu_CfdAPI_Top_Param);
            Mapi_Cfd_testvector455(&stu_CfdAPI_Top_Param);
            break;
        case 7:
         printf("\n test : case7\n");
         //Mapi_Cfd_testvector068(&stu_CfdAPI_Top_Param);
            //Mapi_Cfd_testvector007(&stu_CfdAPI_Top_Param);
            Mapi_Cfd_testvector456(&stu_CfdAPI_Top_Param);
            break;
        case 8:
         printf("\n test : case8\n");
         Mapi_Cfd_testvector069(&stu_CfdAPI_Top_Param);
            //Mapi_Cfd_testvector008(&stu_CfdAPI_Top_Param);
            break;
        case 9:
         printf("\n test : case9\n");
         //Mapi_Cfd_testvector070(&stu_CfdAPI_Top_Param);
            //Mapi_Cfd_testvector009(&stu_CfdAPI_Top_Param);
            Mapi_Cfd_testvector412(&stu_CfdAPI_Top_Param);
            break;
        case 10:
         printf("\n test : case10\n");
         Mapi_Cfd_testvector071(&stu_CfdAPI_Top_Param);
            //Mapi_Cfd_testvector010(&stu_CfdAPI_Top_Param);
            break;
        case 11:
         printf("\n test : case11\n");
         Mapi_Cfd_testvector072(&stu_CfdAPI_Top_Param);
            //Mapi_Cfd_testvector011(&stu_CfdAPI_Top_Param);
            break;
        case 12:
         printf("\n test : case12\n");
            Mapi_Cfd_testvector435(&stu_CfdAPI_Top_Param);
            break;
        case 13:
         printf("\n test : case13\n");
            Mapi_Cfd_testvector436(&stu_CfdAPI_Top_Param);
            break;
        case 14:
         printf("\n test : case14\n");
            Mapi_Cfd_testvector437(&stu_CfdAPI_Top_Param);
            break;
        case 15:
            printf("\n test : case15\n");
            Mapi_Cfd_testvector438(&stu_CfdAPI_Top_Param);
            break;
        case 20:
            printf("\n test : case20\n");
            Mapi_Cfd_testvector411(&stu_CfdAPI_Top_Param);
            Mapi_Cfd_testvector1001(&stu_CfdAPI_Top_Param);
            break;
        case 21:
            printf("\n test : case21\n");
            Mapi_Cfd_testvector412(&stu_CfdAPI_Top_Param);
            Mapi_Cfd_testvector1001(&stu_CfdAPI_Top_Param);
            break;
        case 22:
            printf("\n test : case22 : testvector 461\n");
            Mapi_Cfd_testvector461(&stu_CfdAPI_Top_Param);
            Mapi_Cfd_testvector1001(&stu_CfdAPI_Top_Param);
            break;
        case 23:
            printf("\n test : case23 : testvector 461\n");
            Mapi_Cfd_testvector441(&stu_CfdAPI_Top_Param);
            Mapi_Cfd_testvector1002(&stu_CfdAPI_Top_Param);
            break;
        case 24:
            printf("\n test : case24 : testvector 462\n");
            Mapi_Cfd_testvector441(&stu_CfdAPI_Top_Param);
            Mapi_Cfd_testvector1003(&stu_CfdAPI_Top_Param);
            break;
        case 25:
            printf("\n test : case25 : testvector 463\n");
            Mapi_Cfd_testvector412(&stu_CfdAPI_Top_Param);
            Mapi_Cfd_testvector1003(&stu_CfdAPI_Top_Param);
            break;
        case 31: //301
            printf("\n test : case31 : testvector 464\n");
            Mapi_Cfd_testvector464(&stu_CfdAPI_Top_Param);
            break;
        case 32://302
            printf("\n test : case32 \n");
            //Mapi_Cfd_testvector470(&stu_CfdAPI_Top_Param);
            Mhal_Cfd_Main_Predefined_Control_Set(1);
            break;
        case 33://302
            printf("\n test : case33 \n");
            Mapi_Cfd_testvector441(&stu_CfdAPI_Top_Param);
            break;
        case 34://302
            printf("\n test : case34 \n");
            //Mapi_Cfd_testvector034(&stu_CfdAPI_Top_Param);
            stu_CfdAPI_Top_Param.pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode  = 0;
            break;
        case 35://302
            Mapi_Cfd_testvector035(&stu_CfdAPI_Top_Param);
            break;
        case 36://302
            Mapi_Cfd_testvector036(&stu_CfdAPI_Top_Param);
            break;
        case 255://Auto
            break;

        default:
         Mapi_Cfd_testvector072(&stu_CfdAPI_Top_Param);
         break;

    }

    //printf("\n test : case34 \n");
    //stu_CfdAPI_Top_Param.pstu_Main_Control->u8HDMIOutput_GammutMapping_Mode  = 0;

    //test on 2015/11/10
    //for HDMI input
    //Mapi_Cfd_testvector_base(&stu_CfdAPI_Top_Param);

#if RealChip
    Mapi_Cfd_HWIPS_Default(&stu_CfdAPI_Top_Param);
#endif

    //RGB full in , SDR in , no GM , Panel out
    //Mapi_Cfd_testvector001(&stu_CfdAPI_Top_Param);

    //RGB full in , SDR in , no GM , HDMI out
    //Mapi_Cfd_testvector061(&stu_CfdAPI_Top_Param);

    //RGB full in , SDR in , no GM , IsRGBvypass = 1
    //Mapi_Cfd_testvector002(&stu_CfdAPI_Top_Param);

    //RGB limit in , SDR in , no GM , IsRGBvypass = 0, HDMI out
    //Mapi_Cfd_testvector062(&stu_CfdAPI_Top_Param);


    //RGB limit in , SDR in , no GM
    //Mapi_Cfd_testvector003(&stu_CfdAPI_Top_Param);

    //RGB limit in , SDR in , no GM , IsRGBvypass = 1
    //Mapi_Cfd_testvector004(&stu_CfdAPI_Top_Param);

    //Mapi_Cfd_testvector005(&stu_CfdAPI_Top_Param);
    //RGB limit in , SDR in , GM in SDR IPs
    //Mapi_Cfd_testvector006(&stu_CfdAPI_Top_Param);
    //RGB limit in , SDR in , GM in SDR IPs
    //Mapi_Cfd_testvector007(&stu_CfdAPI_Top_Param);

    //YUV limit in , SDR in , no GM
    //Mapi_Cfd_testvector008(&stu_CfdAPI_Top_Param);

    //YUV full in , SDR in , no GM, HDMI out
    //Mapi_Cfd_testvector063(&stu_CfdAPI_Top_Param);

    //YUV full in , SDR in , no GM
    //Mapi_Cfd_testvector009(&stu_CfdAPI_Top_Param);

    ////YUV limit in , SDR in , no GM, HDMI out
    //Mapi_Cfd_testvector064(&stu_CfdAPI_Top_Param);

    ////BT2020CL in, YUV full in , SDR in , no GM, HDMI out
    //Mapi_Cfd_testvector065(&stu_CfdAPI_Top_Param);

    ////BT2020CL in, YUV limit in , SDR in , no GM, HDMI out
    //Mapi_Cfd_testvector066(&stu_CfdAPI_Top_Param);

    //RGB full in , HDR in , no GM , HDMI out, Force HDR bypass
    //Mapi_Cfd_testvector067(&stu_CfdAPI_Top_Param);

    //RGB limit in , HDR in , no GM , IsRGBvypass = 0, HDMI out, Force HDR bypass
    //Mapi_Cfd_testvector068(&stu_CfdAPI_Top_Param);

    ////BT2020NCL in, YUV full in , HDR in , no GM, HDMI out
    //Mapi_Cfd_testvector069(&stu_CfdAPI_Top_Param);

    ////BT2020NCL in, YUV limit in , HDR in , no GM, HDMI out
    //Mapi_Cfd_testvector070(&stu_CfdAPI_Top_Param);

    ////BT2020NCL in, YUV full in , HDR in , no GM, HDMI out , HDR out , HDR bypass
    //Mapi_Cfd_testvector071(&stu_CfdAPI_Top_Param);

    ////BT2020NCL in, YUV limit in , HDR in , no GM, HDMI out , HDR out , HDR bypass
    //Mapi_Cfd_testvector072(&stu_CfdAPI_Top_Param);

    //Mapi_Cfd_testvector010(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector011(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector012(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector013(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector014(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector015(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector016(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector020(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector021(&stu_CfdAPI_Top_Param);

    //SDR, do nothing in HDR IP, only Y2R in SDR IP
    //Mapi_Cfd_testvector022(&stu_CfdAPI_Top_Param);

    //SDR, do nothing in HDR IP, only Y2R in SDR IP

    //Mapi_Cfd_testvector023(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector024(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector025(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector026(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector028(&stu_CfdAPI_Top_Param);

    //SDR, do nothing in HDR IP, only Y2R in SDR IP
    //Mapi_Cfd_testvector029(&stu_CfdAPI_Top_Param);


    //Mapi_Cfd_testvector030(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector031(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector032(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector033(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector034(&stu_CfdAPI_Top_Param);

    //Mapi_Cfd_testvector035(&stu_CfdAPI_Top_Param);

    //Mapi_Cfd_testvector036(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector037(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector038(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector039(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector040(&stu_CfdAPI_Top_Param);

    //Mapi_Cfd_testvector041(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector042(&stu_CfdAPI_Top_Param);

    //YUV limit in , HDR in, TMO in HDR IP, GM in SDR IP (to Panel)
    //Mapi_Cfd_testvector043(&stu_CfdAPI_Top_Param);

    //HDR , GM and TMO in HDR IP, GM in SDR IP (to Panel)
    //Mapi_Cfd_testvector044(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector045(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector046(&stu_CfdAPI_Top_Param);

    //YUV full in , HDR in, GM and TMO in HDR IP, GM in SDR IP (to Panel)
    //Mapi_Cfd_testvector047(&stu_CfdAPI_Top_Param);

    //RGB limit in , HDR in, GM and TMO in HDR IP, GM in SDR IP (to Panel)
    //Mapi_Cfd_testvector048(&stu_CfdAPI_Top_Param);

    //RGB full in , HDR in, GM and TMO in HDR IP, GM in SDR IP (to Panel)
    //Mapi_Cfd_testvector049(&stu_CfdAPI_Top_Param);

    //Mapi_Cfd_testvector050(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector051(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector052(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector053(&stu_CfdAPI_Top_Param);

    //Mapi_Cfd_testvector054(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector055(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector056(&stu_CfdAPI_Top_Param);

    //Mapi_Cfd_testvector080(&stu_CfdAPI_Top_Param);
    ////Mapi_Cfd_testvector081(&stu_CfdAPI_Top_Param);
    ////Mapi_Cfd_testvector082(&stu_CfdAPI_Top_Param);

    //for MM input
    //YUV full SDR in, MM=BT709, GM in SDR IP, ForceMMtoHDMI mode = 1
    //Mapi_Cfd_testvector100(&stu_CfdAPI_Top_Param);

    //YUV full SDR in, MM=BT709, GM in SDR IP, ForceMMtoHDMI mode = 2
    //Mapi_Cfd_testvector101(&stu_CfdAPI_Top_Param);

    //Mapi_Cfd_testvector102(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector103(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector104(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector105(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector106(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector107(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector108(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector109(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector110(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector111(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector112(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector113(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector114(&stu_CfdAPI_Top_Param);

    //default u8MMInput_ColorimetryHandle_Mode = 0

    //Rec 709. SDR . YUV
    //Mapi_Cfd_testvector115(&stu_CfdAPI_Top_Param);

    //Rec 709. HDR . YUV
    //Mapi_Cfd_testvector116(&stu_CfdAPI_Top_Param);

    //Rec 709. SDR . RGB
    //Mapi_Cfd_testvector117(&stu_CfdAPI_Top_Param);

    //Rec 709. HDR . RGB
    //Mapi_Cfd_testvector118(&stu_CfdAPI_Top_Param);

    //reserved case from 115
    //cp,tr,mc = {2,1,1}
    //Mapi_Cfd_testvector119(&stu_CfdAPI_Top_Param);
    //cp,tr,mc = {1,2,1}
    //Mapi_Cfd_testvector120(&stu_CfdAPI_Top_Param);
    //cp,tr,mc = {1,1,2}
    //Mapi_Cfd_testvector121(&stu_CfdAPI_Top_Param);

    //reserved case from 116
    //cp,tr,mc = {2,16,1}
    //Mapi_Cfd_testvector122(&stu_CfdAPI_Top_Param);
    //cp,tr,mc = {1,16,2}
    //Mapi_Cfd_testvector123(&stu_CfdAPI_Top_Param);

    //reserved case from 117
    //cp,tr,mc = {2,1,0}
    //Mapi_Cfd_testvector124(&stu_CfdAPI_Top_Param);
    //cp,tr,mc = {1,2,0}
    //Mapi_Cfd_testvector125(&stu_CfdAPI_Top_Param);

    //reserved case from 118
    //cp,tr,mc = {2,16,0}
    //Mapi_Cfd_testvector126(&stu_CfdAPI_Top_Param);
    //cp,tr,mc = {2,2,2}
    //Mapi_Cfd_testvector127(&stu_CfdAPI_Top_Param);

    //add u8MMInput_ColorimetryHandle_Mode = 1
    //stu_CfdAPI_Top_Param.pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x1     ;      //2:Force709  1: ON 0: OFF
    //add u8MMInput_ColorimetryHandle_Mode = 2
    //stu_CfdAPI_Top_Param.pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x2     ;      //2:Force709  1: ON 0: OFF
    //add u8MMInput_ColorimetryHandle_Mode = 3
    //stu_CfdAPI_Top_Param.pstu_Main_Control->u8MMInput_ColorimetryHandle_Mode  = 0x3     ;      //2:Force709  1: ON 0: OFF

    //second case fir case 116, no precons, assgin middle format
    //Mapi_Cfd_testvector167(&stu_CfdAPI_Top_Param);

    //for analog input
    //Mapi_Cfd_testvector150(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector151(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector152(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector153(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector154(&stu_CfdAPI_Top_Param);

    //debug with Easter
    //Mapi_Cfd_testvector180(&stu_CfdAPI_Top_Param);

    //for error handling
    //Mapi_Cfd_testvector201(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector202(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector203(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector204(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector205(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector206(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector207(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector208(&stu_CfdAPI_Top_Param);

    //test for system
    //Mapi_Cfd_testvector301(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector302(&stu_CfdAPI_Top_Param);

    //box case MM, HDMI out
    //Mapi_Cfd_testvector400(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector401(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector402(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector405(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector406(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector409(&stu_CfdAPI_Top_Param);

    //error or force happens
    //Mapi_Cfd_testvector404(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector408(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector410(&stu_CfdAPI_Top_Param);

    //Curry Mikoshi case
    //u8MMInput_ColorimetryHandle_Mode = 1
    //SDR TV case
    //Mapi_Cfd_testvector411(&stu_CfdAPI_Top_Param);

    //HDR TV case
    //Mapi_Cfd_testvector412(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector413(&stu_CfdAPI_Top_Param);

    //gamut mapping case
    //Mapi_Cfd_testvector450(&stu_CfdAPI_Top_Param);

    //u8HDMIOutput_GammutMapping_Mode = 2
    //Mapi_Cfd_testvector451(&stu_CfdAPI_Top_Param);

    //Mapi_Cfd_testvector452(&stu_CfdAPI_Top_Param);

    //MM input : AdobeYCC601 HDR=HLG, HDMI output : BT2020 NCL SDR, do GM, TMO
    //Mapi_Cfd_testvector453(&stu_CfdAPI_Top_Param);

    //MM input : AdobeYCC601 SDR, HDMI output : BT2020 NCL SDR, do GM
    //Mapi_Cfd_testvector454(&stu_CfdAPI_Top_Param);

    //Mapi_Cfd_testvector455(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector456(&stu_CfdAPI_Top_Param);

    //HLG case
    //u8MMInput_ColorimetryHandle_Mode = 1
    //HDR TV
    //Mapi_Cfd_testvector440(&stu_CfdAPI_Top_Param);
    //SDR TV
    //Mapi_Cfd_testvector441(&stu_CfdAPI_Top_Param);

    //u8MMInput_ColorimetryHandle_Mode = 0
    //Mapi_Cfd_testvector415(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector416(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector417(&stu_CfdAPI_Top_Param);

    //RGB HDR IN , HDR TV
    //Mapi_Cfd_testvector418(&stu_CfdAPI_Top_Param);
    //RGB HDR IN , SDR TV
    //Mapi_Cfd_testvector419(&stu_CfdAPI_Top_Param);

    //bypass cases from EDID
    //Mapi_Cfd_testvector429(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector430(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector431(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector432(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector433(&stu_CfdAPI_Top_Param);

    //test HDR EDID parsing of luminance
    //Mapi_Cfd_testvector435(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector436(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector437(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector438(&stu_CfdAPI_Top_Param);

    //test HDMI in with HDR infoFrame, CFD bypass
    //infoFrame contains mastering information
    //Mapi_Cfd_testvector073(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector074(&stu_CfdAPI_Top_Param);

    ////based on Mapi_Cfd_testvector073
    //test mastering display parsing
    //test HDMI in with HDR infoFrame, CFD bypass
    //infoFrame contains mastering information
    //Mapi_Cfd_testvector077(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector078(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector079(&stu_CfdAPI_Top_Param);

    //test MM in with Mastering SEI and content light level SEI
    //CFD bypass , link input SEI with outpu InfoFrame
    //Mapi_Cfd_testvector460(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector461(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector462(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector463(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector464(&stu_CfdAPI_Top_Param);


    //test passs
    //Mapi_Cfd_Main_Control_Param_Init_test000(&st_CFD_Main_Control);

    //test passs
    //Mapi_Cfd_testvector001(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector002(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector003(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector004(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector005(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector006(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector007(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector008(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector009(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector010(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector011(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector012(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector013(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector014(&stu_CfdAPI_Top_Param);

    //need to recheck

    //Mapi_Cfd_testvector015(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector024(&stu_CfdAPI_Top_Param);

    //Mapi_Cfd_testvector020(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector021(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector022(&stu_CfdAPI_Top_Param);
    //Mapi_Cfd_testvector023(&stu_CfdAPI_Top_Param);

    //UI
    //auto
    //Mapi_Cfd_testvector1001(&stu_CfdAPI_Top_Param);

    //force HDR2SDR on
    //Mapi_Cfd_testvector1002(&stu_CfdAPI_Top_Param);

    //force HDR2SDR off
    //Mapi_Cfd_testvector1003(&stu_CfdAPI_Top_Param);

    //===================================================================
//#if RealChip
    Mapi_Cfd_Cmodel_Debug_testvector(&stu_CfdAPI_Top_Param);
//#endif


    //only for test
    //stu_CfdAPI_Top_Param.pstu_Main_Control->u8PredefinedProcess = 1;
    //stu_CfdAPI_Top_Param.pstu_OSD_Param->u8HDR_UI_H2SMode = 2;

#if (CFD_SW_VERSION >= 0x00000014)

    Mhal_CFD_PredefinedInputCase(&stu_CfdAPI_Top_Param);

#endif


    u16_call_status = Mapi_Cfd_ColorFormatDriver_Control(&stu_CfdAPI_Top_Param);

    memcpy(&(_stu_Curry_HDRIP_Param[0]),&(stu_HDRIP_Input.stu_Curry_HDRIP_Param),sizeof(STU_CFDAPI_Curry_HDRIP));
    //print modes after decision
    //Mapi_Cfd_Main_Control_PrintResult(&stu_CfdAPI_Top_Param);

    //OSD process

    //default auto mode;
    _stu_Kastor_OSDIP_Param.u8UserMode = 2;
    u16_call_status = Mhal_CFD_OSD_process(stu_CfdAPI_Top_Param.pstu_HDMI_InfoFrame_Param_out,&(_stu_Kastor_OSDIP_Param),&(_stu_Cfd_OSD_Process_Configs));


#if ((CFD_SW_VERSION >= 0x00000014) && (CFD_SW_RETURN_ST_VERSION >= 1))

    if ((1 == stu_CfdAPI_Top_Param.pstu_SW_Return->u8VideoADFlag)||(1 == stu_CfdAPI_Top_Param.pstu_SW_Return->u8OSDADFlag))
    {
        _stu_hal_control.u8AutodownloadFirePermission  = 1;
    }
    else
    {
        _stu_hal_control.u8AutodownloadFirePermission  = 0;
    }

#else
    _stu_hal_control.u8AutodownloadFirePermission  = 0;
#endif

printf("\nCFD : u8AutodownloadFirePermission = %d\n\n",_stu_hal_control.u8AutodownloadFirePermission);




    return 0;
}

void Color_Format_Driver_Dolby_Set(MS_U8 u8Dolby_Mode)
{
#if RealChip
    static MS_U8 u8LastDolbyMode = 0xFF;
    if(u8Dolby_Mode == u8LastDolbyMode)
    {
        return;
    }
    u8LastDolbyMode = u8Dolby_Mode;
    _stu_OSD_Param.u8OSD_UI_Mode = 0;
    if(0x01 == u8Dolby_Mode)
    {
        HDR_DBG_HAL_CFD(printk("===========[%s][%d][Dolby]===========\n",__FUNCTION__,__LINE__));
        _stu_CFD_Main_Control.u8Input_HDRMode = E_CFIO_MODE_HDR1;
    }
    else
    {
        HDR_DBG_HAL_CFD(printk("===========[%s][%d][NotDolby]===========\n",__FUNCTION__,__LINE__));
        _stu_CFD_Main_Control.u8Input_HDRMode = u8Input_HDRMode;
    }
    Color_Format_Driver();
    //KApi_XC_MLoad_Fire(E_CLIENT_MAIN_HDR, TRUE);
    MHal_XC_FireAutoDownload(E_KDRV_XC_AUTODOWNLOAD_CLIENT_HDR);
#endif
}
