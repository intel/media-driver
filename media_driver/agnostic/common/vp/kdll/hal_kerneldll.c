/*
* Copyright (c) 2008-2017, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
//!
//! \file      hal_kerneldll.c 
//! \brief         Kernel Dynamic Linking / Loading routines 
//!
#ifndef VPHAL_LIB

#if IMOLA
#include <stdlib.h>
#endif // IMOLA
#include <math.h> //for sin & cos
#endif  // VPHAL_LIB

#if EMUL || VPHAL_LIB
#include <math.h>
#include "support.h"
#elif LINUX
#else  // !(EMUL | VPHAL_LIB) && !LINUX

#endif // EMUL | VPHAL_LIB

#include "hal_kerneldll.h"
#include "vphal.h"

// Define _DEBUG symbol for KDLL Release build before loading the "vpkrnheader.h" file
// This is necessary for full kernels names in both Release/Debug versions of KDLL app
#if EMUL || VPHAL_LIB
#ifndef _DEBUG
#define _DEBUG 2
#endif // _DEBUG
#endif // EMUL || VPHAL_LIB

// Kernel IDs and Kernel Names
#include "vpkrnheader.h" // IDR_VP_TOTAL_NUM_KERNELS

// Undefine _DEBUG symbol for the remaining of the KDLL Release build
#if _DEBUG == 2
#undef _DEBUG
#endif // _DEBUG

#ifndef PI
#define PI 3.1415926535897932f
#endif // PI

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

const bool g_cIsFormatYUV[Format_Count] =
{
    false,  // Format_Any
    false,  // Format_A8R8G8B8
    false,  // Format_X8R8G8B8
    false,  // Format_A8B8G8R8
    false,  // Format_X8B8G8R8
    false,  // Format_A16B16G16R16
    false,  // Format_A16R16G16B16
    false,  // Format_R5G6B5
    false,  // Format_R32U
    false,  // Format_R32F
    false,  // Format_R8G8B8
    false,  // Format_RGBP
    false,  // Format_BGRP
    true,   // Format_YUY2
    true,   // Format_YUYV
    true,   // Format_YVYU
    true,   // Format_UYVY
    true,   // Format_VYUY
    true,   // Format_Y216
    true,   // Format_Y210
    true,   // Format_Y416
    true,   // Format_AYUV
    true,   // Format_AUYV
    true,   // Format_Y410
    true,   // Format_400P
    true,   // Format_NV12
    true,   // Format_NV12_UnAligned
    true,   // Format_NV21
    true,   // Format_NV11
    true,   // Format_NV11_UnAligned
    true,   // Format_P208
    true,   // Format_P208_UnAligned
    true,   // Format_IMC1
    true,   // Format_IMC2
    true,   // Format_IMC3
    true,   // Format_IMC4
    true,   // Format_422H
    true,   // Format_422V
    true,   // Format_444P
    true,   // Format_411P
    true,   // Format_411R
    true,   // Format_I420
    true,   // Format_IYUV
    true,   // Format_YV12
    true,   // Format_YVU9
    true,   // Format_AI44    (YUV originally, palette may be converted to RGB)
    true,   // Format_IA44    (same as above)
    false,  // Format_P8      (using RGB since P8 is uncommon in FC)
    false,  // Format_A8P8    (same as above)
    false,  // Format_A8
    false,  // Format_L8
    false,  // Format_A4L4
    false,  // Format_A8L8
    true,   // Format_IRW0
    true,   // Format_IRW1
    true,   // Format_IRW2
    true,   // Format_IRW3
    true,   // Format_IRW4
    true,   // Format_IRW5
    true,   // Format_IRW6
    true,   // Format_IRW7
    false,  // Format_STMM
    false,  // Format_Buffer
    false,  // Format_Buffer_2D
    false,  // Format_V8U8
    false,  // Format_R32S
    false,  // Format_R8U
    false,  // Format_R8G8UN
    false,  // Format_R8G8SN
    false,  // Format_G8R8_G8B8
    false,  // Format_R16U
    false,  // Format_R16S
    false,  // Format_R16UN
    false,  // Format_RAW
    false,  // Format_Y8
    false,  // Format_Y1
    false,  // Format_Y16U
    false,  // Format_Y16S
    false,  // Format_L16
    false,  // Format_D16
    false,  // Format_R10G10B10A2
    false,  // Format_B10G10R10A2
    true,   // Format_P016
    true,   // Format_P010
    true    // Format_YV12_Planar
};

const float g_cCSC_sRGB_stRGB[12] =
{
    0.858824f,  0.000000f,  0.000000f,  16.000000f,   // stR = C0 * sR + C1 * sG + C2  * sB + C3
    0.000000f,  0.858824f,  0.000000f,  16.000000f,   // stG = C4 * sR + C5 * sG + C6  * sB + C7
    0.000000f,  0.000000f,  0.858824f,  16.000000f    // stB = C8 * sR + C9 * sG + C10 * sB + C11
};

const float g_cCSC_stRGB_sRGB[12] =
{
    1.164384f,  0.000000f,  0.000000f, -18.630137f,   // sR   = C0 * stR + C1 * stG + C2  * stB + C3
    0.000000f,  1.164384f,  0.000000f, -18.630137f,   // sG   = C4 * stR + C5 * stG + C6  * stB + C7
    0.000000f,  0.000000f,  1.164384f, -18.630137f    // sB   = C8 * stR + C9 * stG + C10 * stB + C11
};

const float g_cCSC_Identity[12] =
{
    1.0f,  0.0f, 0.0f, 0.0f,
    0.0f,  1.0f, 0.0f, 0.0f,
    0.0f,  0.0f, 1.0f, 0.0f
};

const float g_cCSC_GrayIdentity[12] =
{
    1.0f,  0.0f, 0.0f, 0.000f,
    0.0f,  0.0f, 0.0f, 128.0f,
    0.0f,  0.0f, 0.0f, 128.0f
};

// Generic RGB to YUV conversion matrix from BT.601 standard
const float g_cCSC_BT601_RGB_YUV[9] =
{
    0.299000f,  0.587000f,  0.114000f,
   -0.168736f, -0.331264f,  0.500000f,
    0.500000f, -0.418688f, -0.081312f
};

// Generic RGB to YUV conversion matrix from BT.709 standard
const float g_cCSC_BT709_RGB_YUV[9] =
{
    0.212600f,  0.715200f,  0.072200f,
   -0.114572f, -0.385428f,  0.500000f,
    0.500000f, -0.454153f, -0.045847f
};

// Generic YUV to RGB conversion matrix from BT.601 standard
const float g_cCSC_BT601_YUV_RGB[9] =
{
    1.000000f,  0.000000f,  1.402000f,
    1.000000f, -0.344136f, -0.714136f,
    1.000000f,  1.772000f,  0.000000f
};

// Generic YUV to RGB conversion matrix from BT.709 standard
const float g_cCSC_BT709_YUV_RGB[9] =
{
    1.000000f,  0.000000f,  1.574800f,
    1.000000f, -0.187324f, -0.468124f,
    1.000000f,  1.855600f,  0.000000f
};

// BT2020 RGB to Non-constant YUV conversion matrix from R-REC-BT.2020-1-201406-I!!PDF-E.pdf
const float g_cCSC_BT2020_RGB_YUV[9] =
{
    0.262700f,  0.678000f,  0.059300f,     // Y
    -0.139630f, -0.360370f, 0.500000f,     // U
    0.500000f,  -0.459786f, -0.040214f     // V
};

// BT2020 Non-constant YUV to RGB conversion matrix from R-REC-BT.2020-1-201406-I!!PDF-E.pdf
const float g_cCSC_BT2020_YUV_RGB[9] =
{
    1.000000f, 0.000000f,  1.474600f,     //R
    1.000000f, -0.164553f, -0.571353f,    //G
    1.000000f, 1.881400f,  0.000000f      //B
};

// BT2020 YUV Limited Range to BT2020 RGB full range conversion matrix
const float g_cCSC_BT2020_LimitedYUV_RGB[9] =
{
    1.164383f,  0.000000f,  1.678680f,    // R
    1.164383f, -0.187332f, -0.650421f,    // G
    1.164383f,  2.141769f,  0.000000f     // B
};

// BT2020 RGB full range to BT2020 YUV Limited Range tconversion matrix
const float g_cCSC_BT2020_RGB_LimitedYUV[9] =
{
    0.225617f,   0.582275f,  0.050934f,  // Y
    -0.122650f, -0.316559f,  0.439209f,  // U
    0.439209f,  -0.403885f, -0.035324f   // V
};

const char  *g_cInit_ComponentNames[] =
{
    IDR_VP_KERNEL_NAMES
};

#define FOLD_HASH(folded_hash, hash)                                   \
    {                                                                  \
        folded_hash = (((hash) >> 8) ^ (hash)) & 0x00ff00ff;           \
        folded_hash = ((folded_hash >> 16) ^ folded_hash) & 0xff;      \
    }                                                                  \

#if _DEBUG || EMUL || VPHAL_LIB

#ifndef VPHAL_LIB
#pragma warning( push )
#pragma warning( disable : 4996 )
#endif // VPHAL_LIB

const char    *KernelDll_GetLayerString(Kdll_Layer layer)
{
    switch (layer)
    {
        case Layer_Invalid     : return _T("Invalid");
        case Layer_None        : return _T("None");
        case Layer_Background  : return _T("Background");
        case Layer_MainVideo   : return _T("Main Video");
        case Layer_SubVideo    : return _T("Sub-Video");
        case Layer_SubPicture1 : return _T("Sub-Picture 1");
        case Layer_SubPicture2 : return _T("Sub-Picture 2");
        case Layer_SubPicture3 : return _T("Sub-Picture 3");
        case Layer_SubPicture4 : return _T("Sub-Picture 4");
        case Layer_Graphics    : return _T("Graphics");
        case Layer_RenderTarget: return _T("Render Target");
    }

    return nullptr;
}

const char    *KernelDll_GetFormatString(MOS_FORMAT   format)
{
    switch (format)
    {
        case Format_Invalid     : return _T("Invalid");
        case Format_Source      : return _T("Current layer");
        case Format_RGB         : return _T("RGB");
        case Format_RGB32       : return _T("RGB32");
        case Format_PA          : return _T("PA");
        case Format_PL2         : return _T("PL2");
        case Format_PL3         : return _T("PL3");
        case Format_PL3_RGB     : return _T("PL3_RGB");
        case Format_PAL         : return _T("PAL");
        case Format_None        : return _T("None");
        case Format_Any         : return _T("Any");
        case Format_A8R8G8B8    : return _T("ARGB");
        case Format_X8R8G8B8    : return _T("RGB");
        case Format_A8B8G8R8    : return _T("ABGR");
        case Format_X8B8G8R8    : return _T("BGR");
        case Format_A16B16G16R16: return _T("A16B16G16R16");
        case Format_A16R16G16B16: return _T("A16R16G16B16");
        case Format_R5G6B5      : return _T("RGB16");
        case Format_R8G8B8      : return _T("RGB24");
        case Format_R32U        : return _T("R32U");
        case Format_R32F        : return _T("R32F");
        case Format_RGBP        : return _T("RGBP");
        case Format_BGRP        : return _T("BGRP");
        case Format_YUY2        : return _T("YUY2");
        case Format_YUYV        : return _T("YUYV");
        case Format_YVYU        : return _T("YVYU");
        case Format_UYVY        : return _T("UYVY");
        case Format_VYUY        : return _T("VYUY");
        case Format_Y416        : return _T("Y416");
        case Format_AYUV        : return _T("AYUV");
        case Format_AUYV        : return _T("AUYV");
        case Format_400P        : return _T("400P");
        case Format_NV12        : return _T("NV12");
        case Format_NV12_UnAligned: return _T("NV12_UnAligned");
        case Format_NV21        : return _T("NV21");
        case Format_NV11        : return _T("NV11");
        case Format_NV11_UnAligned: return _T("NV11_UnAligned");
        case Format_P208        : return _T("P208");
        case Format_P208_UnAligned: return _T("P208_UnAligned");
        case Format_IMC1        : return _T("IMC1");
        case Format_IMC2        : return _T("IMC2");
        case Format_IMC3        : return _T("IMC3");
        case Format_IMC4        : return _T("IMC4");
        case Format_422H        : return _T("422H");
        case Format_422V        : return _T("422V");
        case Format_444P        : return _T("444P");
        case Format_411P        : return _T("411P");
        case Format_411R        : return _T("411R");
        case Format_I420        : return _T("I420");
        case Format_IYUV        : return _T("IYUV");
        case Format_YV12        : return _T("YV12");
        case Format_YVU9        : return _T("YVU9");
        case Format_AI44        : return _T("AI44");
        case Format_IA44        : return _T("IA44");
        case Format_P8          : return _T("P8");
        case Format_A8P8        : return _T("A8P8");
        case Format_A8          : return _T("A8");
        case Format_L8          : return _T("L8");
        case Format_A4L4        : return _T("A4L4");
        case Format_A8L8        : return _T("A8L8");
        case Format_IRW0        : return _T("IRW0");
        case Format_IRW1        : return _T("IRW1");
        case Format_IRW2        : return _T("IRW2");
        case Format_IRW3        : return _T("IRW3");
        case Format_IRW4        : return _T("IRW4");
        case Format_IRW5        : return _T("IRW5");
        case Format_IRW6        : return _T("IRW6");
        case Format_IRW7        : return _T("IRW7");
        case Format_STMM        : return _T("STMM");
        case Format_Buffer      : return _T("Buffer");
        case Format_Buffer_2D   : return _T("Buffer_2D");
        case Format_V8U8        : return _T("V8U8");
        case Format_R32S        : return _T("R32S");
        case Format_R8U         : return _T("RU8");
        case Format_R8G8UN      : return _T("R8G8UN");
        case Format_R8G8SN      : return _T("R8S8UN");
        case Format_G8R8_G8B8   : return _T("G8R8_G8B8");
        case Format_R16U        : return _T("R16U");
        case Format_R16S        : return _T("R16S");
        case Format_R16UN       : return _T("R16UN");
        case Format_RAW         : return _T("RAW");
        case Format_Y8          : return _T("Y8");
        case Format_Y1          : return _T("Y1");
        case Format_Y16U        : return _T("Y16U");
        case Format_Y16S        : return _T("Y16S");
        case Format_L16         : return _T("L16");
        case Format_D16         : return _T("D16");
        case Format_R10G10B10A2 : return _T("R10G10B10A2");
        case Format_B10G10R10A2 : return _T("B10G10R10A2");
        case Format_P016        : return _T("P016");
        case Format_P010        : return _T("P010");
        case Format_YV12_Planar : return _T("YV12_Planar");
        default                 : return _T("Invalid format");
    }

    return nullptr;
}

const char    *KernelDll_GetCSpaceString(VPHAL_CSPACE cspace)
{
    switch (cspace)
    {
        case CSpace_None                : return _T("None");
        case CSpace_Source              : return _T("Current layer");
        case CSpace_RGB                 : return _T("RGB");
        case CSpace_YUV                 : return _T("YUV");
        case CSpace_Any                 : return _T("Any");
        case CSpace_sRGB                : return _T("sRGB");
        case CSpace_BT601               : return _T("BT.601");
        case CSpace_BT601_FullRange     : return _T("BT.601_FullRange");
        case CSpace_BT709               : return _T("BT.709");
        case CSpace_BT709_FullRange     : return _T("BT.709_FullRange");
        case CSpace_xvYCC601            : return _T("xvYCC.601");
        case CSpace_xvYCC709            : return _T("xvYCC.709");
        case CSpace_BT601Gray           : return _T("BT.601Gray");
        case CSpace_BT601Gray_FullRange : return _T("BT.601Gray_FullRange");
        default                         : return _T("Invalid cspace");
    }

    return nullptr;
}

const char    *KernelDll_GetSamplingString(Kdll_Sampling sampling)
{
    switch (sampling)
    {
        case Sample_None             : return _T("No Sampling");
        case Sample_Source           : return _T("Current layer");
        case Sample_Any              : return _T("Any Sampling");
        case Sample_Scaling_Any      : return _T("Any Scale");
        case Sample_Scaling          : return _T("Scale");
        case Sample_Scaling_034x     : return _T("0.34x");
        case Sample_Scaling_AVS      : return _T("AVS");
        case Sample_iScaling         : return _T("iScale");
        case Sample_iScaling_034x    : return _T("0.34x iScaling");
        case Sample_iScaling_AVS     : return _T("iAVS");
    }

    return nullptr;
}

const char    *KernelDll_GetRotationString(VPHAL_ROTATION rotation)
{
    switch (rotation)
    {
        case VPHAL_ROTATION_IDENTITY            : return _T("0");
        case VPHAL_ROTATION_90                  : return _T("90");
        case VPHAL_ROTATION_180                 : return _T("180");
        case VPHAL_ROTATION_270                 : return _T("270");
        case VPHAL_MIRROR_HORIZONTAL            : return _T("Horizontal");
        case VPHAL_MIRROR_VERTICAL              : return _T("Vertical");
        case VPHAL_ROTATE_90_MIRROR_VERTICAL    : return _T("90 Mirror Vertical");
        case VPHAL_ROTATE_90_MIRROR_HORIZONTAL  : return _T("90 Mirror Horizontal");
    }

    return nullptr;
}

const char    *KernelDll_GetProcessString(Kdll_Processing process)
{
    switch (process)
    {
        case Process_None        : return _T("No processing");
        case Process_Source      : return _T("Current layer");
        case Process_Any         : return _T("Any processing");
        case Process_Composite   : return _T("Composite");
        case Process_CBlend      : return _T("Const Blend");
        case Process_SBlend      : return _T("Source Blend");
        case Process_SBlend_4bits: return _T("Source Blend 4-bits");
        case Process_PBlend      : return _T("Part Blend");
        case Process_CSBlend     : return _T("ConstSource Blend");
        case Process_CPBlend     : return _T("ConstPart Blend");
        case Process_DI          : return _T("DI");
        case Process_DN          : return _T("DN");
        case Process_DNDI        : return _T("DNDI");
    }

    return nullptr;
}


const char    *KernelDll_GetParserStateString(Kdll_ParserState state)
{
    switch (state)
    {
        case Parser_Invalid                 : return _T("Invalid");
        case Parser_Begin                   : return _T("Begin");
        case Parser_SetRenderMethod         : return _T("SetRenderMethod");
        case Parser_SetupLayer0             : return _T("SetupLayer0");
        case Parser_SetupLayer1             : return _T("SetupLayer1");
        case Parser_SetParamsLayer0         : return _T("SetParamsLayer0");
        case Parser_SetParamsLayer1         : return _T("SetParamsLayer1");
        case Parser_SetParamsTarget         : return _T("SetParamsTarget");
        case Parser_SampleLayer0            : return _T("SampleLayer0");
        case Parser_SampleLayer0Mix         : return _T("SampleLayer0Mix");
        case Parser_SampleLayer0ColorFill   : return _T("SampleLayer0ColorFill");
        case Parser_RotateLayer0Check       : return _T("SampleRotateLayer0Check");
        case Parser_RotateLayer0            : return _T("SampleRotateLayer0");
        case Parser_SampleLayer0Done        : return _T("SampleLayer0Done");
        case Parser_ShuffleLayer0           : return _T("ShuffleLayer0");
        case Parser_SampleLayer1            : return _T("SampleLayer1");
        case Parser_SampleLayer1Done        : return _T("SampleLayer1Done");
        case Parser_ShuffleLayer1           : return _T("ShuffleLayer1");
        case Parser_SampleLayer0SelectCSC   : return _T("SampleLayer0SelectCSC");
        case Parser_SetupCSC0               : return _T("SetupCSC0");
        case Parser_ExecuteCSC0             : return _T("ExecuteCSC0");
        case Parser_ExecuteCSC0Done         : return _T("ExecuteCSC0Done");
        case Parser_SetupCSC1               : return _T("SetupCSC1");
        case Parser_ExecuteCSC1             : return _T("ExecuteCSC1");
        case Parser_ExecuteCSC1Done         : return _T("ExecuteCSC1Done");
        case Parser_Lumakey                 : return _T("LumaKey");
        case Parser_ProcessLayer            : return _T("ProcessLayer");
        case Parser_ProcessLayerDone        : return _T("ProcessLayerDone");
        case Parser_DualOutput              : return _T("DualOutput");
        case Parser_Rotation                : return _T("Rotation");
        case Parser_DestSurfIndex           : return _T("DestSurfIndex");
        case Parser_Colorfill               : return _T("Colorfill");
        case Parser_WriteOutput             : return _T("WriteOutput");
        case Parser_End                     : return _T("End");
        default                             : return _T("Invalid parser state");
    }

    return nullptr;
}

const char    *KernelDll_GetRuleIDString(Kdll_RuleID RID)
{
    switch (RID)
    {
        case RID_Op_EOF             : return _T("EOF");
        case RID_Op_NewEntry        : return _T("NewEntry");
        case RID_IsTargetCspace     : return _T("IsTargetCspace");
        case RID_IsLayerID          : return _T("IsLayerID");
        case RID_IsLayerFormat      : return _T("IsLayerFormat");
        case RID_IsParserState      : return _T("IsParserState");
        case RID_IsRenderMethod     : return _T("IsRenderMethod");
        case RID_IsShuffling        : return _T("IsShuffling");
        case RID_IsLayerRotation    : return _T("IsLayerRotation");
        case RID_IsSrc0Format       : return _T("IsSrc0Format");
        case RID_IsSrc0Sampling     : return _T("IsSrc0Sampling");
        case RID_IsSrc0ColorFill    : return _T("IsSrc0ColorFill");
        case RID_IsSrc0LumaKey      : return _T("IsSrc0LumaKey");
        case RID_IsSrc0Procamp      : return _T("IsSrc0Procamp");
        case RID_IsSrc0Rotation     : return _T("IsSrc0Rotation");
        case RID_IsSrc0Coeff        : return _T("IsSrc0Coeff");
        case RID_IsSrc0Processing   : return _T("IsSrc0Processing");
        case RID_IsSrc1Format       : return _T("IsSrc1Format");
        case RID_IsSrc1Sampling     : return _T("IsSrc1Sampling");
        case RID_IsSrc1LumaKey      : return _T("IsSrc1LumaKey");
        case RID_IsSrc1SamplerLumaKey: return _T("IsSrc1SamplerLumaKey");
        case RID_IsSrc1Coeff        : return _T("IsSrc1Coeff");
        case RID_IsSrc1Processing   : return _T("IsSrc1Processing");
        case RID_IsLayerNumber      : return _T("IsLayerNumber");
        case RID_IsQuadrant         : return _T("IsQuadrant");
        case RID_IsCSCBeforeMix     : return _T("IsCSCBeforeMix");
        case RID_IsDualOutput       : return _T("IsDualOutput");
        case RID_IsRTRotate         : return _T("IsRTRotate");
        case RID_IsTargetFormat     : return _T("IsTargetFormat");
        case RID_Is64BSaveEnabled   : return _T("Is64BSaveEnabled");
        case RID_IsTargetTileType   : return _T("IsTargetTileType");
        case RID_IsProcampEnabled   : return _T("IsProcampEnabled");
        case RID_SetTargetCspace    : return _T("SetTargetCspace");
        case RID_SetParserState     : return _T("SetParserState");
        case RID_SetSrc0Format      : return _T("SetSrc0Format");
        case RID_SetSrc0Sampling    : return _T("SetSrc0Sampling");
        case RID_SetSrc0ColorFill   : return _T("SetSrc0ColorFill");
        case RID_SetSrc0LumaKey     : return _T("SetSrc0LumaKey");
        case RID_SetSrc0Rotation    : return _T("SetSrc0Rotation");
        case RID_SetSrc0Coeff       : return _T("SetSrc0Coeff");
        case RID_SetSrc0Processing  : return _T("SetSrc0Processing");
        case RID_SetSrc1Format      : return _T("SetSrc1Format");
        case RID_SetSrc1Sampling    : return _T("SetSrc1Sampling");
        case RID_SetSrc1Rotation    : return _T("SetSrc1Rotation");
        case RID_SetSrc1LumaKey     : return _T("SetSrc1LumaKey");
        case RID_SetSrc1SamplerLumaKey: return _T("SetSrc1SamplerLumaKey");
        case RID_SetSrc1Procamp     : return _T("SetSrc1Procamp");
        case RID_SetSrc1Coeff       : return _T("SetSrc1Coeff");
        case RID_SetSrc1Processing  : return _T("SetSrc1Processing");
        case RID_SetKernel          : return _T("SetKernel");
        case RID_SetNextLayer       : return _T("SetNextLayer");
        case RID_SetPatchData       : return _T("SetPatchData");
        case RID_SetQuadrant        : return _T("SetQuadrant");
        case RID_SetCSCBeforeMix    : return _T("SetCSCBeforeMix");
        case RID_SetPatch           : return _T("SetPatch");
        case RID_IsSrc0Chromasiting : return _T("IsSrc0Chromasiting");
        case RID_IsSrc1Procamp      : return _T("IsSrc1Procamp");
        case RID_IsSrc1Chromasiting : return _T("IsSrc1Chromasiting");
        case RID_IsSetCoeffMode     : return _T("IsSetCoeffMode");
        case RID_IsConstOutAlpha    : return _T("IsConstOutAlpha");
        case RID_IsDitherNeeded     : return _T("IsDitherNeeded");
        case RID_SetSrc0Procamp     : return _T("SetSrc0Procamp");
    }

    return nullptr;
}

const char    *KernelDll_GetCoeffIDString(Kdll_CoeffID CID)
{
    switch (CID)
    {
        case CoeffID_Src0   : return _T("Src0 coeff");
        case CoeffID_Src1   : return _T("Src1 coeff");
        case CoeffID_Source : return _T("Current layer");
        case CoeffID_Any    : return _T("Any coeff");
        case CoeffID_None   : return _T("No coeff");
        case CoeffID_0      : return _T("Coeff 0");
        case CoeffID_1      : return _T("Coeff 1");
        case CoeffID_2      : return _T("Coeff 2");
        case CoeffID_3      : return _T("Coeff 3");
        case CoeffID_4      : return _T("Coeff 4");
        case CoeffID_5      : return _T("Coeff 5");
    }

    return nullptr;
}

const char    *KernelDll_GetShuffleString(Kdll_Shuffling shuffling)
{
    switch (shuffling)
    {
        case Shuffle_None          : return _T("None");
        case Shuffle_Any           : return _T("Any");
        case Shuffle_All_8x8_Layer : return _T("All 8x8 Layer");
        case Shuffle_RenderTarget  : return _T("Render Target");
    }

    return nullptr;
}

int32_t KernelDll_PrintRule(
    char                    *szOut,
    int32_t                 iSize,
    const Kdll_RuleEntry    *pEntry,
    Kdll_KernelCache        *pCache)
{
    char    data[32];
    int32_t increment   = 1;
    const char *szRID   = KernelDll_GetRuleIDString(pEntry->id);
    const char *szValue = nullptr;

    switch (pEntry->id)
    {
        case RID_IsParserState     :
        case RID_SetParserState    :
            szValue = KernelDll_GetParserStateString((Kdll_ParserState) pEntry->value);
            break;

        case RID_IsLayerID         :
            szValue = KernelDll_GetLayerString((Kdll_Layer) pEntry->value);
            break;

        case RID_Op_NewEntry       :
        case RID_Op_EOF            :
        case RID_IsLayerNumber     :
        case RID_IsQuadrant        :
        case RID_SetQuadrant       :
        case RID_SetPatchData      :
            _stprintf(data, _T("%d"), pEntry->value);
            szValue = data;
            break;

        case RID_IsSrc0ColorFill   :
        case RID_SetSrc0ColorFill  :
            if (pEntry->value == ColorFill_Source)
            {
                szValue = _T("Current Layer");
            }
            else if (pEntry->value)
            {
                szValue = _T("TRUE");
            }
            else
            {
                szValue = _T("FALSE");
            }
            break;

        case RID_IsSrc0LumaKey          :
        case RID_IsSrc1LumaKey          :
        case RID_SetSrc0LumaKey         :
        case RID_SetSrc1LumaKey         :
        case RID_IsSrc1SamplerLumaKey   :
        case RID_SetSrc1SamplerLumaKey  :
            if (pEntry->value == LumaKey_Source)
            {
                szValue = _T("Current Layer");
            }
            else if (pEntry->value)
            {
                szValue = _T("TRUE");
            }
            else
            {
                szValue = _T("FALSE");
            }
            break;

        case RID_IsSrc0Procamp    :
        case RID_IsSrc1Procamp    :
        case RID_SetSrc0Procamp   :
        case RID_SetSrc1Procamp   :
            if (pEntry->value == Procamp_Source)
            {
                szValue = _T("Current Layer");
            }
            else if (pEntry->value == DL_PROCAMP_DISABLED)
            {
                szValue = _T("FALSE");
            }
            else
            {
                szValue = _T("TRUE");
            }
            break;

        case RID_IsSrc0Chromasiting :
        case RID_IsSrc1Chromasiting :
            if (pEntry->value == DL_CHROMASITING_DISABLE)
            {
                szValue = _T("FALSE");
            }
            else
            {
                szValue = _T("TRUE");
            }
            break;

        case RID_SetKernel:
            if (pCache && (pEntry->value < pCache->iCacheEntries))
            {
                szValue = pCache->pCacheEntries[pEntry->value].szName;
            }
            else
            {
                _stprintf(data, _T("%d"), pEntry->value);
                szValue = data;
            }
            break;

        case RID_SetNextLayer:
            if (pEntry->value == 0)
            {
                szValue = _T("Next Layer");
            }
            else if (pEntry->value == -1)
            {
                szValue = _T("Previous Layer");
            }
            else if (pEntry->value == 2)
            {
                szValue = _T("Target Layer");
            }
            else if (pEntry->value == -2)
            {
                szValue = _T("Main Layer");
            }
            break;

        case RID_SetPatch          :
            _stprintf(data, _T("%d patch entries"), pEntry->value);
            increment += pEntry->value;
            szValue = data;
            break;

        case RID_IsLayerFormat     :
        case RID_IsSrc0Format      :
        case RID_IsSrc1Format      :
        case RID_SetSrc0Format     :
        case RID_SetSrc1Format     :
        case RID_IsTargetFormat    :
            szValue = KernelDll_GetFormatString((MOS_FORMAT  ) pEntry->value);
            break;

        case RID_IsTargetCspace    :
        case RID_SetTargetCspace   :
            szValue = KernelDll_GetCSpaceString((VPHAL_CSPACE) pEntry->value);
            break;

        case RID_IsSrc0Sampling    :
        case RID_IsSrc1Sampling    :
        case RID_SetSrc0Sampling   :
        case RID_SetSrc1Sampling   :
            szValue = KernelDll_GetSamplingString((Kdll_Sampling) pEntry->value);
            break;

        case RID_IsSrc0Rotation   :
            szValue = KernelDll_GetRotationString((VPHAL_ROTATION) pEntry->value);
            break;

        case RID_IsShuffling       :
            szValue = KernelDll_GetShuffleString((Kdll_Shuffling) pEntry->value);
            break;

        case RID_IsSrc0Coeff       :
        case RID_IsSrc1Coeff       :
        case RID_SetSrc0Coeff      :
        case RID_SetSrc1Coeff      :
            szValue = KernelDll_GetCoeffIDString((Kdll_CoeffID) pEntry->value);
            break;

        case RID_IsSrc1Processing  :
        case RID_SetSrc1Processing :
            szValue = KernelDll_GetProcessString((Kdll_Processing) pEntry->value);
            break;

        case RID_IsCSCBeforeMix    :
        case RID_SetCSCBeforeMix   :
            if (pEntry->value)
            {
                szValue = _T("TRUE");
            }
            else
            {
                szValue = _T("FALSE");
            }
            break;

        case RID_Is64BSaveEnabled  :
            if (pEntry->value)
            {
                szValue = _T("TRUE");
            }
            else
            {
                szValue = _T("FALSE");
            }
            break;

        default:
            break;
    }

    if (szRID && szValue)
    {
        _sntprintf(szOut, iSize, _T("%-22s %s"), szRID, szValue);
    }
    else if (szRID)
    {
        _sntprintf(szOut, iSize, _T("%-22s"), szRID);
    }

    return increment;
}

#ifndef VPHAL_LIB
#pragma warning( pop )
#endif // !VPHAL_LIB

#endif // _DEBUG || EMUL || VPHAL_LIB

/*----------------------------------------------------------------------------
| Name      : KernelDll_IsSameFormatType
| Purpose   : Check if 2 formats are similar
|
| Input     : Internal Format
|
| Return    : FourCC format
\---------------------------------------------------------------------------*/
bool KernelDll_IsSameFormatType(MOS_FORMAT   format1, MOS_FORMAT   format2)
{
    int32_t group1, group2;

    switch (format1)
    {
        CASE_PA_FORMAT:
            group1 = 1;
            break;

        CASE_PL2_FORMAT:
            group1 = 2;
            break;

        CASE_PL3_FORMAT:
            group1 = 3;
            break;

        CASE_RGB_FORMAT:
            group1 = 4;
            break;

        default:
            group1 = 0;
            break;
    }

    switch (format2)
    {
        CASE_PA_FORMAT:
            group2 = 1;
            break;

        CASE_PL2_FORMAT:
            group2 = 2;
            break;

        CASE_PL3_FORMAT:
            group2 = 3;
            break;

        CASE_RGB_FORMAT:
            group2 = 4;
            break;

        default:
            group2 = 0;
            break;
    }

    if (group1 == group2)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool KernelDll_IsYUVFormat(MOS_FORMAT   format)
{
    if (format >= Format_Any && format < Format_Count)
    {
        return g_cIsFormatYUV[format];
    }
    else
    {
        return false;
    }
}

bool KernelDll_IsCspace(VPHAL_CSPACE cspace, VPHAL_CSPACE match)
{
    switch (match)
    {
        case CSpace_RGB:
            return (cspace == CSpace_sRGB            ||
                    cspace == CSpace_stRGB);

        case CSpace_YUV:
            return (cspace == CSpace_BT709           ||
                    cspace == CSpace_BT601           ||
                    cspace == CSpace_BT601_FullRange ||
                    cspace == CSpace_BT709_FullRange ||
                    cspace == CSpace_xvYCC709        ||
                    cspace == CSpace_xvYCC601);

        case CSpace_Gray:
            return (cspace == CSpace_BT601Gray       ||
                    cspace == CSpace_BT601Gray_FullRange);

        case CSpace_Any:
            return (cspace != CSpace_None);

        case CSpace_BT2020:
            return (cspace == CSpace_BT2020    ||
                    cspace == CSpace_BT2020_FullRange);

        case CSpace_BT2020_RGB:
            return (cspace == CSpace_BT2020_RGB ||
                    cspace == CSpace_BT2020_stRGB);

        default:
            return (cspace == match);
    }

    return false;
}

bool KernelDll_IsFormat(
    MOS_FORMAT      format,
    VPHAL_CSPACE    cspace,
    MOS_FORMAT      match)
{
    switch (match)
    {
        case Format_Any:
            return (format != Format_None);
            break;

        case Format_RGB_Swap:
            return (IS_RGB_SWAP(format));

        case Format_RGB_No_Swap:
            return (IS_RGB_NO_SWAP(format));

        case Format_RGB:
            if (IS_PAL_FORMAT(format))
            {
                return (KernelDll_IsCspace(cspace, CSpace_RGB));
            }
            else
            {
                return (IS_RGB_FORMAT(format) && !IS_PL3_RGB_FORMAT(format));
            }

        case Format_RGB32:
            return (IS_RGB32_FORMAT(format));

        case Format_PA:
            if (IS_PAL_FORMAT(format))
            {
                return (KernelDll_IsCspace(cspace, CSpace_YUV));
            }
            else
            {
                return (IS_PA_FORMAT(format) ||
                        format == Format_AUYV);
            }

        case Format_PL2:
            return (IS_PL2_FORMAT(format));

        case Format_PL2_UnAligned:
            return (IS_PL2_FORMAT_UnAligned(format));

        case Format_PL3:
            return (IS_PL3_FORMAT(format));

        case Format_PL3_RGB:
            return (IS_PL3_RGB_FORMAT(format));

        case Format_AYUV:
            return (format == Format_AYUV);

        case Format_PAL:
            return (IS_PAL_FORMAT(format));

        default:
            return (format == match);
    }

    return false;
}

/*----------------------------------------------------------------------------
| Purpose   : Group common color spaces into one
| Returns   : Return the representative color space of the group
\---------------------------------------------------------------------------*/
VPHAL_CSPACE KernelDll_TranslateCspace(VPHAL_CSPACE cspace)
{
    switch (cspace)
    {
        case CSpace_BT709:
        case CSpace_xvYCC709:
            return CSpace_BT709;

        case CSpace_BT601:
        case CSpace_xvYCC601:
            return CSpace_BT601;

        case CSpace_BT601_FullRange:
            return CSpace_BT601_FullRange;

        case CSpace_BT709_FullRange:
            return CSpace_BT709_FullRange;

        case CSpace_RGB:
        case CSpace_sRGB:
            return CSpace_sRGB;

        case CSpace_stRGB:
            return CSpace_stRGB;

        case CSpace_Gray:
        case CSpace_BT601Gray:
            return CSpace_BT601Gray;

        case CSpace_BT601Gray_FullRange:
            return CSpace_BT601Gray_FullRange;

        case CSpace_BT2020:
            return CSpace_BT2020;

        case CSpace_BT2020_FullRange:
            return CSpace_BT2020_FullRange;

        case CSpace_BT2020_RGB:
            return CSpace_BT2020_RGB;

        case CSpace_BT2020_stRGB:
            return CSpace_BT2020_stRGB;

        default:
            return CSpace_None;
    }
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_GetYuvRangeAndOffset
| Purpose   : Get the YUV offset and excursion for the input color space
| Return    : true if success else false
\---------------------------------------------------------------------------*/
bool KernelDll_GetYuvRangeAndOffset(
    Kdll_CSpace cspace,
    float       *pLumaOffset,
    float       *pLumaExcursion,
    float       *pChromaZero,
    float       *pChromaExcursion)
{
    bool res = true;

    switch (cspace)
    {
        case CSpace_BT601_FullRange:
        case CSpace_BT709_FullRange:
        case CSpace_BT601Gray_FullRange:
        case CSpace_BT2020_FullRange:
            *pLumaOffset      =   0.0f;
            *pLumaExcursion   = 255.0f;
            *pChromaZero      = 128.0f;
            *pChromaExcursion = 255.0f;
            break;

        case CSpace_BT601:
        case CSpace_BT709:
        case CSpace_xvYCC601: // since matrix is the same as 601, use the same range
        case CSpace_xvYCC709: // since matrix is the same as 709, use the same range
        case CSpace_BT601Gray:
        case CSpace_BT2020:
            *pLumaOffset      =  16.0f;
            *pLumaExcursion   = 219.0f;
            *pChromaZero      = 128.0f;
            *pChromaExcursion = 224.0f;
            break;

        default:
            res = false;
            break;
    }

    return res;
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_GetRgbRangeAndOffset
| Purpose   : Get the RGB offset and excursion for the input color space
| Return    : true if success else false
\---------------------------------------------------------------------------*/
bool KernelDll_GetRgbRangeAndOffset(
    Kdll_CSpace cspace,
    float       *pRgbOffset,
    float       *pRgbExcursion)
{
    bool res = true;

    switch (cspace)
    {
        case CSpace_sRGB:
        case CSpace_BT2020_RGB:
            *pRgbOffset    =   0.0f;
            *pRgbExcursion = 255.0f;
            break;

        case CSpace_stRGB:
        case CSpace_BT2020_stRGB:
            *pRgbOffset    =  16.0f;
            *pRgbExcursion = 219.0f;
            break;

        default:
            res = false;
            break;
    }

    return res;
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_CalcYuvToRgbMatrix
| Purpose   : Given the YUV->RGB transfer matrix, get the final matrix after
|             applying offsets and excursions.
|
| [R']     [R_o]                                 [R_e/Y_e    0       0   ]  [Y'  - Y_o]
| [G']  =  [R_o] + [YUVtoRGBCoeff (3x3 matrix)]. [   0    R_e/C_e    0   ]. [Cb' - C_z]
| [B']     [R_o]                                 [   0       0    R_e/C_e]. [Cr' - C_z]
|
| [R']  = [C0  C1   C2] [Y' ]   [C3]      {Out pMatrix}
| [G']  = [C4  C5   C6].[Cb'] + [C7]
| [B']  = [C8  C9  C10] [Cr'] + [C11]
|
| Return    : true if success else false
\---------------------------------------------------------------------------*/
bool KernelDll_CalcYuvToRgbMatrix(
    Kdll_CSpace     src,                        // [in] YUV Color space
    Kdll_CSpace     dst,                        // [in] RGB Color space
    float           *pTransferMatrix,             // [in] Transfer matrix (3x3)
    float           *pOutMatrix)                  // [out] Conversion matrix (3x4)
{
    bool    res;
    float   Y_o, Y_e, C_z, C_e;
    float   R_o, R_e;

    res = true;

    res = KernelDll_GetRgbRangeAndOffset(dst, &R_o, &R_e);
    if (res == false)
    {
        goto finish;
    }

    res = KernelDll_GetYuvRangeAndOffset(src, &Y_o, &Y_e, &C_z, &C_e);
    if (res == false)
    {
        goto finish;
    }

    // after + (3x3)(3x3)
    pOutMatrix[0]  = pTransferMatrix[0] * R_e / Y_e;
    pOutMatrix[4]  = pTransferMatrix[3] * R_e / Y_e;
    pOutMatrix[8]  = pTransferMatrix[6] * R_e / Y_e;
    pOutMatrix[1]  = pTransferMatrix[1] * R_e / C_e;
    pOutMatrix[5]  = pTransferMatrix[4] * R_e / C_e;
    pOutMatrix[9]  = pTransferMatrix[7] * R_e / C_e;
    pOutMatrix[2]  = pTransferMatrix[2] * R_e / C_e;
    pOutMatrix[6]  = pTransferMatrix[5] * R_e / C_e;
    pOutMatrix[10] = pTransferMatrix[8] * R_e / C_e;

    // (3x1) - (3x3)(3x3)(3x1)
    pOutMatrix[3]  = R_o - (pOutMatrix[0] * Y_o + pOutMatrix[1] * C_z + pOutMatrix[2]  * C_z);
    pOutMatrix[7]  = R_o - (pOutMatrix[4] * Y_o + pOutMatrix[5] * C_z + pOutMatrix[6]  * C_z);
    pOutMatrix[11] = R_o - (pOutMatrix[8] * Y_o + pOutMatrix[9] * C_z + pOutMatrix[10] * C_z);

finish:
    return res;
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_CalcRgbToYuvMatrix
| Purpose   : Given the RGB->YUV transfer matrix, get the final matrix after
|             applying offsets and excursions.
|
| [Y' ]     [Y_o - Y_e.R_o/R_e]   [Y_e/R_e    0       0   ]  [   RGB to YUV  ]  [R']
| [Cb']  =  [C_z]               + [   0    C_e/R_e    0   ]. [Transfer matrix]. [G']
| [Cr']     [C_z]                 [   0       0    C_e/R_e]  [   3x3 matrix  ]  [B']
|
| [Y' ]  = [C0  C1   C2] [R']   [C3]      {Out pMatrix}
| [Cb']  = [C4  C5   C6].[G'] + [C7]
| [Cr']  = [C8  C9  C10] [B'] + [C11]
|
| Return    : true if success else false
\---------------------------------------------------------------------------*/
bool KernelDll_CalcRgbToYuvMatrix(
    Kdll_CSpace     src,                        // [in] RGB Color space
    Kdll_CSpace     dst,                        // [in] YUV Color space
    float           *pTransferMatrix,             // [in] Transfer matrix (3x3)
    float           *pOutMatrix)                  // [out] Conversion matrix (3x4)
{
    bool    res;
    float   Y_o, Y_e, C_z, C_e;
    float   R_o, R_e;

    res = true;

    res = KernelDll_GetRgbRangeAndOffset(src, &R_o, &R_e);
    if (res == false)
    {
        goto finish;
    }

    res = KernelDll_GetYuvRangeAndOffset(dst, &Y_o, &Y_e, &C_z, &C_e);
    if (res == false)
    {
        goto finish;
    }

    // multiplication of + onwards
    pOutMatrix[0]  = pTransferMatrix[0] * Y_e / R_e;
    pOutMatrix[1]  = pTransferMatrix[1] * Y_e / R_e;
    pOutMatrix[2]  = pTransferMatrix[2] * Y_e / R_e;
    pOutMatrix[4]  = pTransferMatrix[3] * C_e / R_e;
    pOutMatrix[5]  = pTransferMatrix[4] * C_e / R_e;
    pOutMatrix[6]  = pTransferMatrix[5] * C_e / R_e;
    pOutMatrix[8]  = pTransferMatrix[6] * C_e / R_e;
    pOutMatrix[9]  = pTransferMatrix[7] * C_e / R_e;
    pOutMatrix[10] = pTransferMatrix[8] * C_e / R_e;

    // before +
    pOutMatrix[3]  = Y_o - Y_e * R_o / R_e;
    pOutMatrix[7]  = C_z;
    pOutMatrix[11] = C_z;

finish:
    return res;
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_CalcGrayCoeffs
| Purpose   : Given CSC matrix, calculate the new matrix making Chroma zero.
|             Chroma will be read from the surface, but we need to factor in C_z
|             by adjusting this in the constant.
|
| [R']  = [C0  C1   C2] [Y' ]   [C3]      {Out pMatrix}
| [G']  = [C4  C5   C6].[C_z] + [C7]
| [B']  = [C8  C9  C10] [C_z]   [C11]
|
| New C3 = C1 * C_z + C2 * C_z + C3
|
| Return    : true if success else false
\---------------------------------------------------------------------------*/
bool KernelDll_CalcGrayCoeffs(
    Kdll_CSpace     src,                       // [in] YUV source Color space
    float           *pMatrix)                   // [in/out] Conversion matrix (3x4)
{
    float   Y_o, Y_e, C_z, C_e;
    bool    res;

    res = true;

    res = KernelDll_GetYuvRangeAndOffset(src, &Y_o, &Y_e, &C_z, &C_e);
    if (res == false)
    {
        goto finish;
    }

    // Calculate the constant offset by factoring in C_z
    pMatrix[3]  = pMatrix[1] * C_z + pMatrix[2]  * C_z + pMatrix[3];
    pMatrix[7]  = pMatrix[5] * C_z + pMatrix[6]  * C_z + pMatrix[7];
    pMatrix[11] = pMatrix[9] * C_z + pMatrix[10] * C_z + pMatrix[11];

    // Nullify the effect of chroma read
    pMatrix[1] = pMatrix[2]  = 0;
    pMatrix[5] = pMatrix[6]  = 0;
    pMatrix[9] = pMatrix[10] = 0;

finish:
    return res;
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_3x3MatrixProduct
| Purpose   : Given two [3x4] input matrices, calculate [3x3]x[3x3] ignoring
|             the last column in both inputs
| Return    : none
\---------------------------------------------------------------------------*/
void KernelDll_3x3MatrixProduct(
    float       *dest,
    const float *m1,
    const float *m2)
{
    dest[ 0] = m1[0] * m2[0] + m1[1] * m2[4] + m1[ 2] * m2[ 8];
    dest[ 1] = m1[0] * m2[1] + m1[1] * m2[5] + m1[ 2] * m2[ 9];
    dest[ 2] = m1[0] * m2[2] + m1[1] * m2[6] + m1[ 2] * m2[10];

    dest[ 4] = m1[4] * m2[0] + m1[5] * m2[4] + m1[ 6] * m2[ 8];
    dest[ 5] = m1[4] * m2[1] + m1[5] * m2[5] + m1[ 6] * m2[ 9];
    dest[ 6] = m1[4] * m2[2] + m1[5] * m2[6] + m1[ 6] * m2[10];

    dest[ 8] = m1[8] * m2[0] + m1[9] * m2[4] + m1[10] * m2[ 8];
    dest[ 9] = m1[8] * m2[1] + m1[9] * m2[5] + m1[10] * m2[ 9];
    dest[10] = m1[8] * m2[2] + m1[9] * m2[6] + m1[10] * m2[10];
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_CalcYuvToYuvMatrix
| Purpose   : Calculate the matrix equation for converting b/w YUV color spaces.
|             1. Get conversion matrix from Source YUV to sRGB
|             2. Get conversion matrix from sRGB to Destination YUV
|             3. Apply the transformation below to get the final matrix
|
| [Y'dst]  = [C0  C1   C2] [C0  C1   C2][Y'src] [C0  C1   C2] [C3]    [C3]
| [U']     = [C4  C5   C6].[C4  C5   C6][C_z] + [C4  C5   C6].[C7]  + [C7]
| [V']     = [C8  C9  C10] [C8  C9  C10][C_z]   [C8  C9  C10] [C11]   [C11]
|             dst matrix    src matrix           dst matrix    src     dst
|
| [Y'dst]  = [C0  C1   C2] [Y'src]   [C3]      {Out pMatrix}
| [U']     = [C4  C5   C6].[C_z] +   [C7]
| [V']     = [C8  C9  C10] [C_z]     [C11]
|
| Return    : true if success else false
\---------------------------------------------------------------------------*/
bool KernelDll_CalcYuvToYuvMatrix(
    Kdll_CSpace     src,                        // [in] RGB Color space
    Kdll_CSpace     dst,                        // [in] YUV Color space
    float           *pOutMatrix)                 // [out] Conversion matrix (3x4)
{
    float   fYuvToRgb[12]  = {0};
    float   fRgbToYuv[12]  = {0};
    bool    res;

    res = true;

    // 1. Get conversion matrix from Source YUV to sRGB
    if (IS_BT601_CSPACE(src))
    {
        res = KernelDll_CalcYuvToRgbMatrix(src, CSpace_sRGB, (float *) g_cCSC_BT601_YUV_RGB, fYuvToRgb);
    }
    else
    {
        res = KernelDll_CalcYuvToRgbMatrix(src, CSpace_sRGB, (float *) g_cCSC_BT709_YUV_RGB, fYuvToRgb);
    }
    if (res == false)
    {
        goto finish;
    }

    // 2. Get conversion matrix from sRGB to Destination YUV
    if (IS_BT601_CSPACE(dst))
    {
        res = KernelDll_CalcRgbToYuvMatrix(CSpace_sRGB, dst, (float *) g_cCSC_BT601_RGB_YUV, fRgbToYuv);
    }
    else
    {
        res = KernelDll_CalcRgbToYuvMatrix(CSpace_sRGB, dst, (float *) g_cCSC_BT709_RGB_YUV, fRgbToYuv);
    }
    if (res == false)
    {
        goto finish;
    }

    // 3. Multiply the 2 matrices above
    KernelDll_3x3MatrixProduct(pOutMatrix, fRgbToYuv, fYuvToRgb);

    // Perform [3x3][3x1] matrix multiply + [3x1] matrix
    pOutMatrix[3]  = fRgbToYuv[0] * fYuvToRgb[3]  + fRgbToYuv[1] * fYuvToRgb[7] +
                     fRgbToYuv[2] * fYuvToRgb[11] +                fRgbToYuv[3];
    pOutMatrix[7]  = fRgbToYuv[4] * fYuvToRgb[3]  + fRgbToYuv[5] * fYuvToRgb[7] +
                     fRgbToYuv[6] * fYuvToRgb[11] +                fRgbToYuv[7];
    pOutMatrix[11] = fRgbToYuv[8] * fYuvToRgb[3]  + fRgbToYuv[9] * fYuvToRgb[7] +
                     fRgbToYuv[10]* fYuvToRgb[11] +                fRgbToYuv[11];

finish:
    return res;
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_GetCSCMatrix
| Purpose   : Get the required matrix for the given CSC conversion
| Return    :
\---------------------------------------------------------------------------*/
void KernelDll_GetCSCMatrix(
    Kdll_CSpace     src,                        // [in] Source Color space
    Kdll_CSpace     dst,                        // [in] Destination Color space
    float           *pCSC_Matrix)                // [out] CSC matrix to use
{
    bool        bMatrix;
    bool        bSrcGray;
    Kdll_CSpace temp;
    int32_t     i;

    bMatrix  = false;
    bSrcGray = KernelDll_IsCspace(src, CSpace_Gray);

    // convert gray color spaces to its equivalent non-gray cpsace
    switch(src)
    {
        case CSpace_BT601Gray:
            temp = CSpace_BT601;
            break;
        case CSpace_BT601Gray_FullRange:
            temp = CSpace_BT601_FullRange;
            break;
        default:
            temp = src;
            break;
    }

    // BT601/709 YUV to sRGB/stRGB conversion
    if (KernelDll_IsCspace(temp, CSpace_YUV) || KernelDll_IsCspace(temp, CSpace_Gray))
    {
        if(KernelDll_IsCspace(dst, CSpace_RGB))
        {
            if (IS_BT601_CSPACE(temp))
            {
                KernelDll_CalcYuvToRgbMatrix(temp, dst, (float *) g_cCSC_BT601_YUV_RGB, pCSC_Matrix);
                bMatrix = true;
            }
            else // if (IS_BT709_CSPACE(temp))
            {
                KernelDll_CalcYuvToRgbMatrix(temp, dst, (float *) g_cCSC_BT709_YUV_RGB, pCSC_Matrix);
                bMatrix = true;
            }
        }
    }
    // sRGB/stRGB to BT601/709 YUV conversion
    else if (KernelDll_IsCspace(temp, CSpace_RGB))
    {
        if (KernelDll_IsCspace(dst, CSpace_YUV))
        {
            if (IS_BT601_CSPACE(dst))
            {
                KernelDll_CalcRgbToYuvMatrix(temp, dst, (float *) g_cCSC_BT601_RGB_YUV, pCSC_Matrix);
                bMatrix = true;
            }
            else // if (IS_BT709_CSPACE(temp))
            {
                KernelDll_CalcRgbToYuvMatrix(temp, dst, (float *) g_cCSC_BT709_RGB_YUV, pCSC_Matrix);
                bMatrix = true;
            }
        }
    }
    // BT2020 YUV to RGB conversion
    else if (KernelDll_IsCspace(temp, CSpace_BT2020))
    {
        if (KernelDll_IsCspace(dst, CSpace_BT2020_RGB))
        {
            KernelDll_CalcYuvToRgbMatrix(temp, dst, (float *)g_cCSC_BT2020_YUV_RGB, pCSC_Matrix);
            bMatrix = true;
        }
    }
    // BT2020 RGB to YUV conversion
    else if (KernelDll_IsCspace(temp, CSpace_BT2020_RGB))
    {
        if (KernelDll_IsCspace(dst, CSpace_BT2020))
        {
            KernelDll_CalcRgbToYuvMatrix(temp, dst, (float *)g_cCSC_BT2020_RGB_YUV, pCSC_Matrix);
            bMatrix = true;
        }
    }

    // If matrix has not been derived yet, its one of the below special cases
    if (!bMatrix)
    {
        if (temp == dst) // Check if its identity matrix
        {
            MOS_SecureMemcpy(pCSC_Matrix, sizeof(g_cCSC_Identity), (void *)g_cCSC_Identity, sizeof(g_cCSC_Identity));
        }
        else if (KernelDll_IsCspace(temp, CSpace_RGB)) // sRGB to stRGB inter-conversions
        {
            if (temp == CSpace_sRGB)
            {
                MOS_SecureMemcpy(pCSC_Matrix, sizeof(g_cCSC_sRGB_stRGB), (void *)g_cCSC_sRGB_stRGB, sizeof(g_cCSC_sRGB_stRGB));
            }
            else //temp == CSpace_stRGB
            {
                MOS_SecureMemcpy(pCSC_Matrix, sizeof(g_cCSC_stRGB_sRGB), (void *)g_cCSC_stRGB_sRGB, sizeof(g_cCSC_stRGB_sRGB));
            }
        }
        else if (KernelDll_IsCspace(temp, CSpace_YUV)) // 601 to 709 inter-conversions
        {
            KernelDll_CalcYuvToYuvMatrix(temp, dst, pCSC_Matrix);
        }
        else
        {
            VPHAL_RENDER_ASSERTMESSAGE("Not supported color space conversion(from %d to %d)", src, dst);
        }
    }

    // Calculate the Gray transformation matrix now
    if (bSrcGray)
    {
        KernelDll_CalcGrayCoeffs(src, pCSC_Matrix);
    }

    VPHAL_RENDER_NORMALMESSAGE("");
    for(i = 0; i < 3; i++)
    {
        VPHAL_RENDER_NORMALMESSAGE("%f\t%f\t%f\t%f",
                            pCSC_Matrix[4 * i],
                            pCSC_Matrix[4 * i + 1],
                            pCSC_Matrix[4 * i + 2],
                            pCSC_Matrix[4 * i + 3]);
    }
}

void KernelDll_MatrixProduct(
    float       *dest,
    const float *m1,
    const float *m2)
{
    bool  save;
    float temp[12];

    // setup temp matrix to allow the following operations:
    //   dest = dest * m2
    //   dest = m1 * dest
    //   dest = dest * dest
    save = (m1 == dest) || (m2 == dest);
    m1   = (m1 == dest) ? temp : m1;
    m2   = (m2 == dest) ? temp : m2;
    if (save) MOS_SecureMemcpy(temp, sizeof(temp), (void *)dest, sizeof(temp));

    // Multiply the matrices
    dest[ 0] = m1[0] * m2[0] + m1[1] * m2[4] + m1[ 2] * m2[ 8];
    dest[ 1] = m1[0] * m2[1] + m1[1] * m2[5] + m1[ 2] * m2[ 9];
    dest[ 2] = m1[0] * m2[2] + m1[1] * m2[6] + m1[ 2] * m2[10];
    dest[ 3] = m1[0] * m2[3] + m1[1] * m2[7] + m1[ 2] * m2[11] + m1[3];
    dest[ 4] = m1[4] * m2[0] + m1[5] * m2[4] + m1[ 6] * m2[ 8];
    dest[ 5] = m1[4] * m2[1] + m1[5] * m2[5] + m1[ 6] * m2[ 9];
    dest[ 6] = m1[4] * m2[2] + m1[5] * m2[6] + m1[ 6] * m2[10];
    dest[ 7] = m1[4] * m2[3] + m1[5] * m2[7] + m1[ 6] * m2[11] + m1[7];
    dest[ 8] = m1[8] * m2[0] + m1[9] * m2[4] + m1[10] * m2[ 8];
    dest[ 9] = m1[8] * m2[1] + m1[9] * m2[5] + m1[10] * m2[ 9];
    dest[10] = m1[8] * m2[2] + m1[9] * m2[6] + m1[10] * m2[10];
    dest[11] = m1[8] * m2[3] + m1[9] * m2[7] + m1[10] * m2[11] + m1[11];
}

bool KernelDll_MapCSCMatrix(
                            Kdll_CSCType    csctype,
                            const float    *matrix,
                            short          *coeff)
{
    // Unified kernel architecture requires that the color space
    // conversion coefficients programmed in specific orders, depends on the
    // type of the color space conversion.
    //
    // M (matrix)  ---> C (coeff)

    switch (csctype)
    {
        case CSC_YUV_RGB:
            // direct mapping from matrix to coeff
            coeff[ 0] = FLOAT_TO_SHORT(matrix[0] );      // M0  --> C0
            coeff[ 1] = FLOAT_TO_SHORT(matrix[1] );      // M1  --> C1
            coeff[ 2] = FLOAT_TO_SHORT(matrix[2] );      // M2  --> C2
            coeff[ 3] = FLOAT_TO_SHORT(matrix[3] );      // M3  --> C3
            coeff[ 4] = FLOAT_TO_SHORT(matrix[4] );      // M4  --> C4
            coeff[ 5] = FLOAT_TO_SHORT(matrix[5] );      // M5  --> C5
            coeff[ 6] = FLOAT_TO_SHORT(matrix[6] );      // M6  --> C6
            coeff[ 7] = FLOAT_TO_SHORT(matrix[7] );      // M7  --> C7
            coeff[ 8] = FLOAT_TO_SHORT(matrix[8] );      // M8  --> C8
            coeff[ 9] = FLOAT_TO_SHORT(matrix[9] );      // M9  --> C9
            coeff[10] = FLOAT_TO_SHORT(matrix[10]);      // M10 --> C10
            coeff[11] = FLOAT_TO_SHORT(matrix[11]);      // M11 --> C11
            break;

        case CSC_RGB_YUV:
            coeff[ 6] = FLOAT_TO_SHORT(matrix[0] );      // M0   --> C6
            coeff[ 4] = FLOAT_TO_SHORT(matrix[1] );      // M1   --> C4
            coeff[ 5] = FLOAT_TO_SHORT(matrix[2] );      // M2   --> C5
            coeff[ 7] = FLOAT_TO_SHORT(matrix[3] );      // M3   --> C7
            coeff[10] = FLOAT_TO_SHORT(matrix[4] );      // M4   --> C10
            coeff[ 8] = FLOAT_TO_SHORT(matrix[5] );      // M5   --> C8
            coeff[ 9] = FLOAT_TO_SHORT(matrix[6] );      // M6   --> C9
            coeff[11] = FLOAT_TO_SHORT(matrix[7] );      // M7   --> C11
            coeff[ 2] = FLOAT_TO_SHORT(matrix[8] );      // M8   --> C2
            coeff[ 0] = FLOAT_TO_SHORT(matrix[9] );      // M9   --> C0
            coeff[ 1] = FLOAT_TO_SHORT(matrix[10]);      // M10  --> C1
            coeff[ 3] = FLOAT_TO_SHORT(matrix[11]);      // M11  --> C3
            break;

        case CSC_YUV_YUV:
            coeff[ 4] = FLOAT_TO_SHORT(matrix[0] );      // M0   --> C4
            coeff[ 5] = FLOAT_TO_SHORT(matrix[1] );      // M1   --> C5
            coeff[ 6] = FLOAT_TO_SHORT(matrix[2] );      // M2   --> C6
            coeff[ 7] = FLOAT_TO_SHORT(matrix[3] );      // M3   --> C7
            coeff[ 8] = FLOAT_TO_SHORT(matrix[4] );      // M4   --> C8
            coeff[ 9] = FLOAT_TO_SHORT(matrix[5] );      // M5   --> C9
            coeff[10] = FLOAT_TO_SHORT(matrix[6] );      // M6   --> C10
            coeff[11] = FLOAT_TO_SHORT(matrix[7] );      // M7   --> C11
            coeff[ 0] = FLOAT_TO_SHORT(matrix[8] );      // M8   --> C0
            coeff[ 1] = FLOAT_TO_SHORT(matrix[9] );      // M9   --> C1
            coeff[ 2] = FLOAT_TO_SHORT(matrix[10]);      // M10  --> C2
            coeff[ 3] = FLOAT_TO_SHORT(matrix[11]);      // M11  --> C3
            break;

        default:
            //CSC_RGB_RGB
            coeff[ 2] = FLOAT_TO_SHORT(matrix[0] );      // M0   --> C2
            coeff[ 0] = FLOAT_TO_SHORT(matrix[1] );      // M1   --> C0
            coeff[ 1] = FLOAT_TO_SHORT(matrix[2] );      // M2   --> C1
            coeff[ 3] = FLOAT_TO_SHORT(matrix[3] );      // M3   --> C3
            coeff[ 6] = FLOAT_TO_SHORT(matrix[4] );      // M4   --> C6
            coeff[ 4] = FLOAT_TO_SHORT(matrix[5] );      // M5   --> C4
            coeff[ 5] = FLOAT_TO_SHORT(matrix[6] );      // M6   --> C5
            coeff[ 7] = FLOAT_TO_SHORT(matrix[7] );      // M7   --> C7
            coeff[10] = FLOAT_TO_SHORT(matrix[8] );      // M8   --> C10
            coeff[ 8] = FLOAT_TO_SHORT(matrix[9] );      // M9   --> C8
            coeff[ 9] = FLOAT_TO_SHORT(matrix[10]);      // M10  --> C9
            coeff[11] = FLOAT_TO_SHORT(matrix[11]);      // M11  --> C11
            break;
    }

    return true;
}

void KernelDll_UpdateCscCoefficients(Kdll_State      *pState,
                                     Kdll_CSC_Matrix *pMatrix)
{
    float csc[12];    // CSC  matrix (YUV->RGB)
    float icsc[12];   // ICSC matrix (RGB->YUV), (YUV->YUV)
    float m[12];      // auxiliary matrix
    float matrix[12];    // final matrix
    Kdll_CSCType csctype;
    Kdll_Procamp *pProcamp = nullptr;
    VPHAL_CSPACE  src = pMatrix->SrcSpace;
    VPHAL_CSPACE  dst = pMatrix->DstSpace;
    bool bCSC, bICSC;

    bCSC = bICSC = false;
    MOS_ZeroMemory(m, sizeof(m));
    MOS_ZeroMemory(csc, sizeof(csc));
    MOS_ZeroMemory(icsc, sizeof(icsc));

    // Select procamp parameters
    if (pMatrix->iProcampID > DL_PROCAMP_DISABLED &&
        pMatrix->iProcampID < pState->iProcampSize &&
        pState->pProcamp != nullptr)
    {
        pProcamp = pState->pProcamp + pMatrix->iProcampID;
    }

    // Setup CSC matrix
    if (src != dst)
    {
        if ((dst == CSpace_sRGB) && (src != CSpace_stRGB))
        {
            KernelDll_GetCSCMatrix(src, dst, csc);
            MOS_SecureMemcpy(m, sizeof(csc), (void *)csc, sizeof(csc));
            bCSC = true;
            csctype = CSC_YUV_RGB;
        }
        else if ((dst == CSpace_stRGB) && (src != CSpace_sRGB))
        {
            KernelDll_GetCSCMatrix(src, dst, csc);
            MOS_SecureMemcpy(m, sizeof(csc), (void *)csc, sizeof(csc));
            bCSC = true;
            csctype = CSC_YUV_RGB;
        }
        else
        {
            KernelDll_GetCSCMatrix(src, dst, icsc);
            MOS_SecureMemcpy(m, sizeof(icsc), (void *)icsc, sizeof(icsc));
            bICSC = true;
            if (KernelDll_IsCspace(src, CSpace_RGB) && !KernelDll_IsCspace(dst, CSpace_RGB))
            {
                csctype = CSC_RGB_YUV;
            }
            else if (KernelDll_IsCspace(src, CSpace_BT2020_RGB) && KernelDll_IsCspace(dst, CSpace_BT2020))
            {
                csctype = CSC_RGB_YUV;
            }
            else if (KernelDll_IsCspace(src, CSpace_BT2020) && KernelDll_IsCspace(dst, CSpace_BT2020_RGB))
            {
                csctype = CSC_YUV_RGB;
            }
            else
            {
                csctype = CSC_YUV_YUV;
            }
        }
    }
    // Setup CSC matrix for procamp in sRGB space
    else if ((dst == CSpace_sRGB) && (pProcamp))
    {
        KernelDll_GetCSCMatrix(CSpace_sRGB, CSpace_BT709, icsc);
        KernelDll_GetCSCMatrix(CSpace_BT709, CSpace_sRGB, csc);
        bICSC = bCSC = true;
        csctype = CSC_RGB_RGB;
    }
    // Setup CSC matrix for procamp in stRGB space
    else if ((dst == CSpace_stRGB) && (pProcamp))
    {
        KernelDll_GetCSCMatrix(CSpace_stRGB, CSpace_BT709, icsc);
        KernelDll_GetCSCMatrix(CSpace_BT709, CSpace_stRGB, csc);
        bICSC = bCSC = true;
        csctype = CSC_RGB_RGB;
    }
    else
    {
        MOS_SecureMemcpy(m, sizeof(g_cCSC_Identity), (void *)g_cCSC_Identity, sizeof(g_cCSC_Identity));
        csctype = CSC_YUV_YUV;
    }

    // Product only happens if Procamp is present
    // Otherwise use the original matrix
    if (pProcamp)
    {
        float b,c,h,s;

        // Calculate procamp parameters
        b = pProcamp->fBrightness;
        c = pProcamp->fContrast;
        h = pProcamp->fHue * (PI / 180.0f);
        s = pProcamp->fSaturation;

        // procamp matrix
        //
        // [Y']   [ c            0          0  ] [Y]   [ 16  - 16 * c + b              ]
        // [U'] = [ 0   c*s*cos(h)  c*s*sin(h) ] [U] + [ 128 - 128*c*s*(cos(h)+sin(h)) ]
        // [V']   [ 0  -c*s*sin(h)  c*s*cos(h) ] [V]   [ 128 - 128*c*s*(cos(h)-sin(h)) ]

        matrix[0]  = c;
        matrix[1]  = 0.0f;
        matrix[2]  = 0.0f;
        matrix[3]  = 16.0f - 16.0f * c + b;
        matrix[4]  = 0.0f;
        matrix[5]  = (float)cos(h) * c * s;
        matrix[6]  = (float)sin(h) * c * s;
        matrix[7]  = 128.0f * (1.0f - matrix[5] - matrix[6]);
        matrix[8]  = 0.0f;
        matrix[9]  = -matrix[6];
        matrix[10] =  matrix[5];
        matrix[11] = 128.0f * (1.0f - matrix[5] + matrix[6]);

        // Calculate final CSC matrix (csc * pa * icsc)
        if (bICSC)
        {   // Calculate [pa] * [icsc]
            KernelDll_MatrixProduct(matrix, matrix, icsc);
        }

        if (bCSC)
        {   // Calculate [csc] * [pa]     (if no icsc)
            //        or [csc] * [pa] * [icsc]
            KernelDll_MatrixProduct(matrix, csc, matrix);
        }

        // Update procamp version
        pMatrix->iProcampVersion = pProcamp->iProcampVersion;

        // Use the output matrix to generate kernel CSC parameters
        MOS_SecureMemcpy(m, sizeof(m), (void *)matrix, sizeof(m));
    }

    // normalize for kernel use
    matrix[0]  = ROUND_FLOAT(m[ 0], 128.0f);  // 9.7
    matrix[1]  = ROUND_FLOAT(m[ 1], 128.0f);  // 9.7
    matrix[2]  = ROUND_FLOAT(m[ 2], 128.0f);  // 9.7
    matrix[3]  = ROUND_FLOAT(m[ 3],   0.5f);  // 16.0 (value/2)
    matrix[4]  = ROUND_FLOAT(m[ 4], 128.0f);  // 9.7
    matrix[5]  = ROUND_FLOAT(m[ 5], 128.0f);  // 9.7
    matrix[6]  = ROUND_FLOAT(m[ 6], 128.0f);  // 9.7
    matrix[7]  = ROUND_FLOAT(m[ 7],   0.5f);  // 16.0 (value/2)
    matrix[8]  = ROUND_FLOAT(m[ 8], 128.0f);  // 9.7
    matrix[9]  = ROUND_FLOAT(m[ 9], 128.0f);  // 9.7
    matrix[10] = ROUND_FLOAT(m[10], 128.0f);  // 9.7
    matrix[11] = ROUND_FLOAT(m[11],   0.5f);  // 16.0 (value/2)

    // Save matrix as kernel CSC coefficients
    pState->pfnMapCSCMatrix(csctype, matrix, pMatrix->Coeff);
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_FindRule
| Purpose   : Find a rule that matches the current search/input state
|
| Input     : pState       - Kernel Dll state
|             pSearchState - current DL search state
|
| Return    :
\---------------------------------------------------------------------------*/
bool KernelDll_FindRule(
    Kdll_State       *pState,
    Kdll_SearchState *pSearchState)
{
    uint32_t parser_state = (uint32_t)pSearchState->state;
    Kdll_RuleEntrySet    *pRuleSet;
    const Kdll_RuleEntry *pRuleEntry;
    int32_t              iRuleCount;
    int32_t              iMatchCount;
    bool                 bLayerFormatMatched;
    bool                 bSrc0FormatMatched;
    bool                 bSrc1FormatMatched;
    bool                 bTargetFormatMatched;
    bool                 bSrc0SampingMatched;

    VPHAL_RENDER_FUNCTION_ENTER;

    // All Custom states are handled as a single group
    if (parser_state >= Parser_Custom)
    {
        parser_state = Parser_Custom;
    }

    pRuleSet   = pState->pDllRuleTable[parser_state];
    iRuleCount = pState->iDllRuleCount[parser_state];

    if (pRuleSet == nullptr || iRuleCount == 0)
    {
        VPHAL_RENDER_NORMALMESSAGE("Search rules undefined.");
        pSearchState->pMatchingRuleSet = nullptr;
        return false;
    }

    // Search matching entry
    for ( ; iRuleCount > 0; iRuleCount--, pRuleSet++)
    {
        // Points to the first rule, get number of matches
        pRuleEntry  = pRuleSet->pRuleEntry;
        iMatchCount = pRuleSet->iMatchCount;

        // Initialize for each Ruleset
        bLayerFormatMatched  = false;
        bSrc0FormatMatched   = false;
        bSrc1FormatMatched   = false;
        bTargetFormatMatched = false;
        bSrc0SampingMatched  = false;

        // Match all rules within the same RuleSet
        for (; iMatchCount > 0; iMatchCount--, pRuleEntry++)
        {
            switch (pRuleEntry->id)
            {
                // Match current Parser State
                case RID_IsParserState:
                    if (pSearchState->state == (Kdll_ParserState) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match render method
                case RID_IsRenderMethod:
                    if (pSearchState->pFilter->RenderMethod == (Kdll_RenderMethod)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match target color space
                case RID_IsTargetCspace:
                    if (KernelDll_IsCspace(pSearchState->cspace, (VPHAL_CSPACE) pRuleEntry->value))
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match current layer ID
                case RID_IsLayerID:
                    if (pSearchState->pFilter->layer == (Kdll_Layer) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match current layer format
                case RID_IsLayerFormat:
                    if (pRuleEntry->logic == Kdll_Or && bLayerFormatMatched)
                    {
                        // Already found matching format in the ruleset
                        continue;
                    }
                    else
                    {
                        // Check if the layer format matches the rule
                        if (KernelDll_IsFormat(pSearchState->pFilter->format,
                                                pSearchState->pFilter->cspace,
                                                (MOS_FORMAT  ) pRuleEntry->value))
                        {
                            bLayerFormatMatched = true;
                        }

                        if (pRuleEntry->logic == Kdll_None && !bLayerFormatMatched)
                        {
                            // Last entry and No matching format was found
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }

                // Match shuffling requirement
                case RID_IsShuffling:
                    if (pSearchState->ShuffleSamplerData == (Kdll_Shuffling) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Check if RT rotates
                case RID_IsRTRotate:
                    if (pSearchState->bRTRotate == (pRuleEntry->value ? true : false) )
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match current layer rotation
                case RID_IsLayerRotation:
                    if (pSearchState->pFilter->rotation == (VPHAL_ROTATION) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src0 source format (surface)
                case RID_IsSrc0Format:
                    if (pRuleEntry->logic == Kdll_Or && bSrc0FormatMatched)
                    {
                        // Already found matching format in the ruleset
                        continue;
                    }
                    else
                    {
                        // Check if the source 0 format matches the rule
                        // The intermediate colorspace is used to determine
                        // if palettized input is given in RGB or YUV format.
                        if (KernelDll_IsFormat(pSearchState->src0_format,
                                                pSearchState->cspace,
                                                (MOS_FORMAT  ) pRuleEntry->value))
                        {
                            bSrc0FormatMatched = true;
                        }

                        if (pRuleEntry->logic == Kdll_None && !bSrc0FormatMatched)
                        {
                            // Last entry and No matching format was found
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }

                // Match Src0 sampling mode
                case RID_IsSrc0Sampling:
                    // Check if the layer format matches the rule
                    if (pSearchState->src0_sampling == (Kdll_Sampling) pRuleEntry->value)
                    {
                        bSrc0SampingMatched = true;
                        continue;
                    }
                    else if (bSrc0SampingMatched || pRuleEntry->logic == Kdll_Or)
                    {
                        continue;
                    }
                    else if ((Kdll_Sampling) pRuleEntry->value == Sample_Any &&
                            pSearchState->src0_sampling != Sample_None)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src0 rotation
                case RID_IsSrc0Rotation:
                    if (pSearchState->src0_rotation == (VPHAL_ROTATION) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src0 Colorfill
                case RID_IsSrc0ColorFill:
                    if (pSearchState->src0_colorfill == (int32_t)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src0 Luma Key
                case RID_IsSrc0LumaKey:
                    if (pSearchState->src0_lumakey == (int32_t)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src0 Procamp
                case RID_IsSrc0Procamp:
                    if (pSearchState->pFilter->procamp == (int32_t)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src0 CSC coefficients
                case RID_IsSrc0Coeff:
                    if (pSearchState->src0_coeff == (Kdll_CoeffID) pRuleEntry->value)
                    {
                        continue;
                    }
                    else if ((Kdll_CoeffID) pRuleEntry->value == CoeffID_Any &&
                            pSearchState->src0_coeff != CoeffID_None)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src0 CSC coefficients setting mode
                case RID_IsSetCoeffMode:
                    if (pSearchState->pFilter->SetCSCCoeffMode == (Kdll_SetCSCCoeffMethod) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src0 processing mode
                case RID_IsSrc0Processing:
                    if (pSearchState->src0_process == (Kdll_Processing) pRuleEntry->value)
                    {
                        continue;
                    }
                    if ((Kdll_Processing) pRuleEntry->value == Process_Any &&
                        pSearchState->src0_process != Process_None)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src0 chromasiting mode
                case RID_IsSrc0Chromasiting:
                    if (pSearchState->Filter->chromasiting == (int32_t)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src1 source format (surface)
                case RID_IsSrc1Format:
                    if (pRuleEntry->logic == Kdll_Or && bSrc1FormatMatched)
                    {
                        // Already found matching format in the ruleset
                        continue;
                    }
                    else
                    {
                        // Check if the source 1 format matches the rule
                        // The intermediate colorspace is used to determine
                        // if palettized input is given in RGB or YUV format.
                        if (KernelDll_IsFormat(pSearchState->src1_format,
                                                pSearchState->cspace,
                                                (MOS_FORMAT) pRuleEntry->value))
                        {
                            bSrc1FormatMatched = true;
                        }

                        if (pRuleEntry->logic == Kdll_None && !bSrc1FormatMatched)
                        {
                            // Last entry and No matching format was found
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }
                // Match Src1 sampling mode
                case RID_IsSrc1Sampling:
                    if (pSearchState->src1_sampling == (Kdll_Sampling) pRuleEntry->value)
                    {
                        continue;
                    }
                    else if ((Kdll_Sampling) pRuleEntry->value == Sample_Any &&
                            pSearchState->src1_sampling != Sample_None)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src1 Luma Key
                case RID_IsSrc1LumaKey:
                    if (pSearchState->src1_lumakey == (int32_t)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }
                    
                // Match Src1 Sampler LumaKey
                case RID_IsSrc1SamplerLumaKey:
                    if (pSearchState->src1_samplerlumakey == (int32_t)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src1 Procamp
                case RID_IsSrc1Procamp:
                    if (pSearchState->pFilter->procamp == (int32_t)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src1 CSC coefficients
                case RID_IsSrc1Coeff:
                    if (pSearchState->src1_coeff == (Kdll_CoeffID) pRuleEntry->value)
                    {
                        continue;
                    }
                    else if ((Kdll_CoeffID) pRuleEntry->value == CoeffID_Any &&
                            pSearchState->src1_coeff != CoeffID_None)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src1 processing mode
                case RID_IsSrc1Processing:
                    if (pSearchState->src1_process == (Kdll_Processing) pRuleEntry->value)
                    {
                        continue;
                    }
                    if ((Kdll_Processing) pRuleEntry->value == Process_Any &&
                        pSearchState->src1_process != Process_None)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src1 chromasiting mode
                case RID_IsSrc1Chromasiting:
                    //pSearchState->pFilter is pointed to the real sub layer
                    if (pSearchState->pFilter->chromasiting == (int32_t)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Layer number
                case RID_IsLayerNumber:
                    if (pSearchState->layer_number == (int32_t) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match quadrant
                case RID_IsQuadrant:
                    if (pSearchState->quadrant == (int32_t) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Set CSC flag before Mix
                case RID_IsCSCBeforeMix:
                    if (pSearchState->bCscBeforeMix == (pRuleEntry->value ? true : false))
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                case RID_IsDualOutput:
                    if (pSearchState->pFilter->dualout == (pRuleEntry->value ? true : false))
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                case RID_IsTargetFormat:
                    if (pRuleEntry->logic == Kdll_Or && bTargetFormatMatched)
                    {
                        // Already found matching format in the ruleset
                        continue;
                    }
                    else
                    {
                        if (pSearchState->target_format == (MOS_FORMAT) pRuleEntry->value)
                        {
                            bTargetFormatMatched = true;
                        }

                        if (pRuleEntry->logic == Kdll_None && !bTargetFormatMatched)
                        {
                            // Last entry and No matching format was found
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }

                case RID_Is64BSaveEnabled:
                    if (pSearchState->b64BSaveEnabled == (pRuleEntry->value ? true : false))
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                case RID_IsTargetTileType:
                    if (pRuleEntry->logic == Kdll_None &&
                        pSearchState->target_tiletype == (MOS_TILE_TYPE) pRuleEntry->value)
                    {
                        continue;
                    }
                    else if (pRuleEntry->logic == Kdll_Not &&
                             pSearchState->target_tiletype != (MOS_TILE_TYPE) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                case RID_IsProcampEnabled:
                    if (pSearchState->bProcamp == (pRuleEntry->value ? true : false))
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                case RID_IsConstOutAlpha:
                    if (pSearchState->pFilter->bFillOutputAlphaWithConstant == (pRuleEntry->value ? true : false))
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                case RID_IsDitherNeeded:
                    if (pSearchState->pFilter->bIsDitherNeeded == (pRuleEntry->value ? true : false))
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }
                // Undefined search rule will fail
                default:
                    VPHAL_RENDER_ASSERTMESSAGE("Invalid rule %d @ layer %d, state %d.", pRuleEntry->id, pSearchState->layer_number, pSearchState->state);
                    break;
            }  // End of switch to deal with all matching rule IDs

            // Rule didn't match - try another RuleSet
            break;
        } // End of file loop to test all rules for the current RuleSet

        // Match
        if (iMatchCount == 0)
        {
            pSearchState->pMatchingRuleSet = pRuleSet;
            return true;
        }
    }   // End of for loop to test all RuleSets for the current parser state

    // Failed to find a matching rule -> kernel search will fail
    VPHAL_RENDER_NORMALMESSAGE("Fail to find a matching rule @ layer %d, state %d.", pSearchState->layer_number, pSearchState->state);

    // No match -> return
    pSearchState->pMatchingRuleSet = nullptr;
    return false;
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_GetPatchData
| Purpose   : Get binary data block to be used for kernel patching
|
| Input     : pState       - [in]  Current DL state
|             pSearchState - [in]  Current DL search state
|             iPatchKind   - [in]  Patch kind
|             pSize        - [out] Data block Size
|
| Return    : nullptr - Unsupported patch data kind
|             <>nullptr - Pointer to data block
\---------------------------------------------------------------------------*/
static uint8_t *KernelDll_GetPatchData(
    Kdll_State          *pState,
    Kdll_SearchState    *pSearchState,
    int32_t             iPatchKind,
    int32_t             *pSize)
{
    MOS_UNUSED(pState);

    VPHAL_RENDER_FUNCTION_ENTER;

    if (iPatchKind == PatchKind_CSC_Coeff_Src0 ||
        iPatchKind == PatchKind_CSC_Coeff_Src1)
    {
        Kdll_CoeffID  coeffID  = CoeffID_None;
        uint8_t       matrixID = DL_CSC_DISABLED;

        // Get matrix id
        if (iPatchKind == PatchKind_CSC_Coeff_Src0)
        {
            coeffID = pSearchState->src0_coeff;
        }
        else
        {
            coeffID = pSearchState->src1_coeff;
        }

        // Get matrix associated with the coefficient ID
        if (coeffID > CoeffID_None)
        {
            matrixID = pSearchState->CscParams.MatrixID[coeffID];
        }

        // Found matrix
        if (matrixID < DL_CSC_MAX)
        {
            Kdll_CSC_Matrix *pMatrix = &(pSearchState->CscParams.Matrix[matrixID]);

            *pSize = 12 * sizeof(uint16_t);

            if (pState->bEnableCMFC)
            {
                if (pSearchState->CscParams.PatchMatrixNum < DL_CSC_MAX)
                {
                    pSearchState->CscParams.PatchMatrixID[pSearchState->CscParams.PatchMatrixNum] = matrixID;
                    pSearchState->CscParams.PatchMatrixNum ++;
                }
                else
                {
                    VPHAL_RENDER_NORMALMESSAGE("Patch CSC coefficient exceed limitation");
                }
            }

            return ((uint8_t *)pMatrix->Coeff);
        }
    }
    else
    {
        VPHAL_RENDER_NORMALMESSAGE("Invalid patch kind %d.", iPatchKind);
    }

    return nullptr;
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_UpdateState
| Purpose   : Update search state using current matching rule
|
| Input     : pState       - Kernel Dll state
|             pSearchState - current DL search state
|
| Return    :
\---------------------------------------------------------------------------*/
bool KernelDll_UpdateState(
    Kdll_State       *pState,
    Kdll_SearchState *pSearchState)
{
    Kdll_RuleEntrySet    *pRuleSet   = pSearchState->pMatchingRuleSet;
    const Kdll_RuleEntry *pRuleEntry;
    int32_t               iSetCount;

    VPHAL_RENDER_FUNCTION_ENTER;

    // Ensures that we have a matching rule
    if (pRuleSet == nullptr)
    {
        return false;
    }

    // Get rule entry and number of state update ("Set") rules; validate
    pRuleEntry = pRuleSet->pRuleEntry;
    iSetCount  = pRuleSet->iSetCount;
    if (pRuleEntry == nullptr || iSetCount < 1)
    {
        VPHAL_RENDER_NORMALMESSAGE("Invalid rule set.");
        return false;
    }

    // Jump to set rules (skip match rules)
    pRuleEntry += pRuleSet->iMatchCount;

    // Apply state update rules
    for (; iSetCount > 0; iSetCount--, pRuleEntry++)
    {
        switch (pRuleEntry->id)
        {
            // Add kernel to the Dynamic Linking array
            case RID_SetKernel:
                if (pSearchState->KernelCount < DL_MAX_KERNELS)
                {
                    int32_t i = pSearchState->KernelCount++;
                    pSearchState->KernelID [i] = pRuleEntry->value;
                    pSearchState->KernelGrp[i] = pRuleSet->iGroup;  // Group associated with the kernel ID
                }
                else
                {
                    VPHAL_RENDER_ASSERTMESSAGE("reached maximum number of component kernels.");
                    return false;
                }
                break;

            // Set Parser State
            case RID_SetParserState:
                pSearchState->state = (Kdll_ParserState) pRuleEntry->value;
                break;

            // Move to Next/Prev Layer
            case RID_SetNextLayer:
                if (pRuleEntry->value == -1)
                {
                    pSearchState->layer_number--;
                    pSearchState->pFilter--;
                }
                else if (pRuleEntry->value == -2) // jump to layer main video
                {
                   do
                   {
                        pSearchState->layer_number--;
                        pSearchState->pFilter--;
                        if (pSearchState->pFilter == nullptr || pSearchState->layer_number < 0 )
                        {
                             return false;
                        }
                    }
                    while (pSearchState->pFilter->layer != Layer_MainVideo);
                }
                else if (pRuleEntry->value == 2) // jump to target layer
                {
                    while (pSearchState->pFilter->layer < Layer_RenderTarget )
                    {
                        pSearchState->layer_number++;
                        pSearchState->pFilter++;
                    }
                }
                else
                {
                    pSearchState->layer_number++;
                    pSearchState->pFilter++;
                }
                break;

            // Set patch data
            case RID_SetPatchData:
                {
                    uint8_t *pData = nullptr;
                    int32_t iSize = 0;
                    int32_t iKernelIndex = pSearchState->KernelCount - 1;
                    int32_t iPatchIndex;
                    Kdll_PatchData *pPatch;

                    // Get block of data for patching
                    pData = KernelDll_GetPatchData(pState, pSearchState, (Kdll_PatchKind)pRuleEntry->value, &iSize);
                    if (pData == nullptr || iSize == 0)
                    {
                        VPHAL_RENDER_ASSERTMESSAGE("invalid patch.");
                        return false;
                    }

                    // Append to the existing patch data block
                    iPatchIndex = pSearchState->PatchID[iKernelIndex];

                    // Allocate new patch structure
                    if (iPatchIndex < 0)
                    {
                        // Fail to allocate
                        if (pSearchState->PatchCount >= DL_MAX_PATCHES)
                        {
                            VPHAL_RENDER_ASSERTMESSAGE("reached maximum number of patches.");
                            return false;
                        }

                        // Get new patch block
                        iPatchIndex = pSearchState->PatchCount++;
                        pSearchState->PatchID[iKernelIndex] = iPatchIndex;

                        // Reset new patch entry
                        pPatch = &(pSearchState->Patches[iPatchIndex]);
                        MOS_ZeroMemory(pPatch, sizeof(Kdll_PatchData));
                    }
                    else
                    {
                        // Get Patch entry already in use
                        pPatch = &(pSearchState->Patches[iPatchIndex]);
                    }

                    // Check if data can be appended
                    if (pPatch->iPatchDataSize + iSize > DL_MAX_PATCH_DATA_SIZE)
                    {
                        VPHAL_RENDER_ASSERTMESSAGE("exceeded maximum patch size.");
                        return false;
                    }

                    // Append patch data
                    MOS_SecureMemcpy(pPatch->Data + pPatch->iPatchDataSize, iSize, (void *)pData, iSize);
                    pPatch->iPatchDataSize += iSize;
                }
                break;

            // Set patch operation
            case RID_SetPatch:
                {
                    int32_t iKernelIndex = pSearchState->KernelCount - 1;
                    int32_t iPatchIndex  = pSearchState->PatchID[iKernelIndex];
                    Kdll_PatchData      *pPatch;
                    uint8_t             *pPatchRule;
                    Kdll_PatchBlock     *pPatchBlock;
                    int32_t              nPatches;

                    // No patch associated with the current kernel
                    if (iPatchIndex < 0)
                    {
                        return false;
                    }

                    // Get Patch entry
                    pPatch = &(pSearchState->Patches[iPatchIndex]);

                    // Get number of patches and pointer to first rule extension (patch rule)
                    nPatches   = pRuleEntry->value;
                    pPatchRule = (uint8_t*) (pRuleEntry + 1);

                    // Check if rules can be applied
                    if (nPatches + pPatch->nPatches > DL_MAX_PATCH_BLOCKS)
                    {
                        VPHAL_RENDER_ASSERTMESSAGE("exceeded number of patch blocks.");
                        return false;
                    }

                    // Set Patches
                    pPatchBlock = &(pPatch->Patch[pPatch->nPatches]);
                    for (; nPatches > 0; nPatches--, pPatchBlock++, pPatch->nPatches++)
                    {
                        pPatchBlock->BlockSize = ((Kdll_PatchRuleEntry *)pPatchRule)->Size;
                        pPatchBlock->SrcOffset = ((Kdll_PatchRuleEntry *)pPatchRule)->Source;
                        pPatchBlock->DstOffset = ((Kdll_PatchRuleEntry *)pPatchRule)->Dest;
                        pPatchRule += sizeof(Kdll_RuleEntry);
                    }

                    // Skip rule extensions
                    iSetCount  -= pRuleEntry->value;
                    pRuleEntry += pRuleEntry->value;
                }
                break;

            // Set destination colorspace
            case RID_SetTargetCspace:
                if ((VPHAL_CSPACE)pRuleEntry->value == CSpace_Source)
                {
                    pSearchState->cspace = pSearchState->pFilter->cspace;
                }
                else
                {
                    pSearchState->cspace = (VPHAL_CSPACE)pRuleEntry->value;
                }
                break;

            // Set Src0 source format
            case RID_SetSrc0Format:
                if ((MOS_FORMAT  )pRuleEntry->value == Format_Source)
                {
                    pSearchState->src0_format = pSearchState->pFilter->format;
                }
                else
                {
                    pSearchState->src0_format = (MOS_FORMAT  ) pRuleEntry->value;
                }
                break;

            // Set Src0 sampling mode
            case RID_SetSrc0Sampling:
                if ((Kdll_Sampling)pRuleEntry->value == Sample_Source)
                {
                    pSearchState->src0_sampling = pSearchState->pFilter->sampler;
                }
                else
                {
                    pSearchState->src0_sampling = (Kdll_Sampling) pRuleEntry->value;
                }
                break;

            // Set Src0 Rotation
            case RID_SetSrc0Rotation:
                pSearchState->src0_rotation = pSearchState->pFilter->rotation;
                break;

            // Set Src0 Colorfill
            case RID_SetSrc0ColorFill:
                if ((int32_t)pRuleEntry->value == ColorFill_Source)
                {
                    pSearchState->src0_colorfill = pSearchState->pFilter->colorfill;
                }
                else
                {
                    pSearchState->src0_colorfill = (int32_t)pRuleEntry->value;
                }
                break;

            // Set Src0 luma key
            case RID_SetSrc0LumaKey:
                if (pRuleEntry->value == LumaKey_Source)
                {
                    pSearchState->src0_lumakey = pSearchState->pFilter->lumakey;
                }
                else
                {
                    pSearchState->src0_lumakey = (int32_t)pRuleEntry->value;
                }
                break;

            // Set Src0 Procamp
            case RID_SetSrc0Procamp:
                if (pRuleEntry->value == Procamp_Source)
                {
                    pSearchState->src0_procamp = pSearchState->pFilter->procamp;
                }
                else
                {
                    pSearchState->src0_procamp = (int32_t)pRuleEntry->value;
                }
                break;

            // Set Src0 CSC coefficients
            case RID_SetSrc0Coeff:
                if ((Kdll_CoeffID)pRuleEntry->value == CoeffID_Source)
                {
                    if (pSearchState->pFilter->matrix == DL_CSC_DISABLED)
                    {
                        pSearchState->src0_coeff = CoeffID_None;
                    }
                    else
                    {
                        Kdll_CSC_Matrix *matrix = pSearchState->CscParams.Matrix;
                        matrix += pSearchState->pFilter->matrix;

                        pSearchState->src0_coeff = matrix->iCoeffID;
                    }
                }
                else
                {
                    pSearchState->src0_coeff = (Kdll_CoeffID) pRuleEntry->value;
                }
                break;

            case RID_SetSrc0Processing:
                if ((Kdll_Processing)pRuleEntry->value == Process_Source)
                {
                    pSearchState->src0_process = pSearchState->pFilter->process;
                }
                else
                {
                    pSearchState->src0_process = (Kdll_Processing) pRuleEntry->value;
                }
                break;

            // Set Src1 source format
            case RID_SetSrc1Format:
                if ((MOS_FORMAT  )pRuleEntry->value == Format_Source)
                {
                    pSearchState->src1_format = pSearchState->pFilter->format;
                }
                else
                {
                    pSearchState->src1_format = (MOS_FORMAT  ) pRuleEntry->value;
                }
                break;

            // Set Src1 sampling mode
            case RID_SetSrc1Sampling:
                if ((Kdll_Sampling)pRuleEntry->value == Sample_Source)
                {
                    pSearchState->src1_sampling = pSearchState->pFilter->sampler;
                }
                else
                {
                    pSearchState->src1_sampling = (Kdll_Sampling) pRuleEntry->value;
                }
                break;

            // Set Src1 Rotation
            case RID_SetSrc1Rotation:
                pSearchState->src1_rotation = pSearchState->pFilter->rotation;
                break;

            // Set Src1 luma key
            case RID_SetSrc1LumaKey:
                if (pRuleEntry->value == LumaKey_Source)
                {
                    pSearchState->src1_lumakey = pSearchState->pFilter->lumakey;
                }
                else
                {
                    pSearchState->src1_lumakey = (int32_t)pRuleEntry->value;
                }
                break;

            // Set Src1 Sampler LumaKey
            case RID_SetSrc1SamplerLumaKey:
                if (pRuleEntry->value == LumaKey_Source)
                {
                    pSearchState->src1_samplerlumakey = pSearchState->pFilter->samplerlumakey;
                }
                else
                {
                    pSearchState->src1_samplerlumakey = (int32_t)pRuleEntry->value;
                }
                break;

            // Set Src1 Procamp
            case RID_SetSrc1Procamp:
                if (pRuleEntry->value == Procamp_Source)
                {
                    pSearchState->src1_procamp = pSearchState->pFilter->procamp;
                }
                else
                {
                    pSearchState->src1_procamp = (int32_t)pRuleEntry->value;
                }
                break;

            // Set Src1 CSC coefficients
            case RID_SetSrc1Coeff:
                if ((Kdll_CoeffID)pRuleEntry->value == CoeffID_Source)
                {
                    if (pSearchState->pFilter->matrix == DL_CSC_DISABLED)
                    {
                        pSearchState->src1_coeff = CoeffID_None;
                    }
                    else
                    {
                        Kdll_CSC_Matrix *matrix = pSearchState->CscParams.Matrix;
                        matrix += pSearchState->pFilter->matrix;

                        pSearchState->src1_coeff = matrix->iCoeffID;
                    }
                }
                else
                {
                    pSearchState->src1_coeff = (Kdll_CoeffID) pRuleEntry->value;
                }
                break;

            // Set Src1 processing mode
            case RID_SetSrc1Processing:
                if ((Kdll_Processing)pRuleEntry->value == Process_Source)
                {
                    pSearchState->src1_process = pSearchState->pFilter->process;
                }
                else
                {
                    pSearchState->src1_process = (Kdll_Processing) pRuleEntry->value;
                }
                break;

            // Set current quadrant
            case RID_SetQuadrant:
                pSearchState->quadrant = (int32_t) pRuleEntry->value;
                break;

            // Set CSC flag before Mix
            case RID_SetCSCBeforeMix:
                pSearchState->bCscBeforeMix = pRuleEntry->value ? true : false;
                break;

            // Unsupported "Set" rule
            default:
                // Failed to find a matching rule -> kernel search will fail
                VPHAL_RENDER_ASSERTMESSAGE("Invalid rule %d @ layer %d, state %d.", pRuleEntry->id, pSearchState->layer_number, pSearchState->state);
                return false;
        }
    }

    // Reset matching rule
    pSearchState->pMatchingRuleSet = nullptr;
    return true;
}

//-----------------------------------------------------------------------------------------
// KernelDll_SortRuleTable - Sort master dynamic linking rule table
//
// Parameters:
//    char  *pState    - [in] Kernel Dll state
//
// Output: true  - Master rule table (and acceleration table) successfully created
//         false - Failed to setup master rule table
//-----------------------------------------------------------------------------------------
bool KernelDll_SortRuleTable(Kdll_State *pState)
{
    uint8_t group;
    int32_t state;
    const Kdll_RuleEntry *pRule = nullptr;
    Kdll_RuleEntrySet    *pRuleSet;
    int32_t i, j;

    int32_t iTotal = 0;
    int32_t iNoOverr[Parser_Count];    // Non-overridable (enforced) rules
    int32_t iDefault[Parser_Count];    // Default rules
    int32_t iCustom [Parser_Count];    // Custom rules

    VPHAL_RENDER_FUNCTION_ENTER;

    // Release previous table (rule table update)
    if (pState->pSortedRules)
    {
        MOS_FreeMemory(pState->pSortedRules);
        pState->pSortedRules = nullptr;

        MOS_ZeroMemory(pState->pDllRuleTable, sizeof(pState->pDllRuleTable));
        MOS_ZeroMemory(pState->iDllRuleCount, sizeof(pState->iDllRuleCount));
    }

    // Zero counters
    MOS_ZeroMemory(iNoOverr, sizeof(iNoOverr));
    MOS_ZeroMemory(iDefault, sizeof(iDefault));
    MOS_ZeroMemory(iCustom , sizeof(iCustom));

    // Count number of entries for each state
    for (i = 0; i < 2; i++)
    {
        if (i == 0)
        {
            pRule = pState->pRuleTableDefault;
        }
        else if (i == 1)
        {
            pRule = pState->pRuleTableCustom;
        }

        // Table not set - continue
        if (!pRule) continue;

        for (; pRule->id != RID_Op_EOF; pRule++)
        {
            // Skip extended rules (variable lenght)
            if (RID_IS_EXTENDED(pRule->id))
            {   // value contains number of entries
                pRule += pRule->value;
            }
            else if (pRule->id == RID_Op_NewEntry)
            {
                // Save Rule Group
                if (i == 0)
                {
                    group = pRule->value;
                }
                else
                {
                    group = RULE_CUSTOM;
                }

                // Second rule must always be RID_IsParserState
                pRule++;
                if (pRule->id != RID_IsParserState)
                {
                    VPHAL_RENDER_ASSERTMESSAGE("Rule does not start with State.");
                    return false;
                }

                // Get Parser State -> validate value
                state = pRule->value;
                if (state <  Parser_Begin)
                {
                    VPHAL_RENDER_ASSERTMESSAGE("Invalid State %d.", state);
                    return false;
                }
                else if (state >= Parser_Custom)
                {   // Custom states are set together in the same entry
                    state = Parser_Custom;
                }

                if (group == RULE_NO_OVERRIDE)
                {
                    iNoOverr[state]++;
                }
                else if (group == RULE_DEFAULT)
                {
                    iDefault[state]++;
                }
                else
                {
                    iCustom[state]++;
                }

                iTotal++;
            }
        }
    }

    // Allocate rules
    pState->pSortedRules = (Kdll_RuleEntrySet *)MOS_AllocAndZeroMemory(iTotal * sizeof(Kdll_RuleEntrySet));
    if (!pState->pSortedRules)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Failed to allocate rule table.");
        return false;
    }

    // Setup pointers to sorted rules
    pState->pDllRuleTable[0] = pState->pSortedRules;
    for (j = 0, i = 0; i < Parser_Count; i++)
    {
        // Setup start pointer and number of entries to search for each state
        pState->pDllRuleTable[i] = pState->pDllRuleTable[j] + pState->iDllRuleCount[j];
        pState->iDllRuleCount[i] = iNoOverr[i] + iCustom[i] + iDefault[i];

        // Setup offsets to rules for sorting
        iDefault[i] = iNoOverr[i] + iCustom[i];   // Last set of rules
        iCustom [i] = iNoOverr[i];                // 2nd set of rules
        iNoOverr[i] = 0;                          // 1st set of rules

        j = i;
    }

    // Sort rules for fast access
    // Integrate enforced, custom, default rules into one single access table
    for (i = 0; i < 2; i++)
    {
        if (i == 0)
        {
            pRule = pState->pRuleTableDefault;
        }
        else if (i == 1)
        {
            pRule = pState->pRuleTableCustom;
        }

        // Table not set - continue
        if (!pRule) continue;

        while (pRule->id != RID_Op_EOF)
        {
            if (pRule->id != RID_Op_NewEntry)
            {
                VPHAL_RENDER_ASSERTMESSAGE("New rule entry expected.");
                return false;
            }

            // Save Rule Group
            if (i == 0)
            {
                group = pRule->value;
            }
            else
            {
                group = RULE_CUSTOM;
            }

            // Get Parser State -> validate value
            pRule++;
            state = pRule->value;
            if (state >= Parser_Custom)
            {   // Custom states are set together in the same entry
                state = Parser_Custom;
            }
            else
            {   // Skip state check - already handled by acceleration table
                pRule++;
            }

            // Point to sorted rule set entry
            if (group == RULE_NO_OVERRIDE)
            {
                j = iNoOverr[state]++;
            }
            else if (group == RULE_DEFAULT)
            {
                j = iDefault[state]++;
            }
            else
            {
                j = iCustom[state]++;
            }

            // Point to sorted ruleset for the current parser state
            pRuleSet = pState->pDllRuleTable[state] + j;

            // Fill RuleSet
            pRuleSet->pRuleEntry = pRule;
            pRuleSet->iGroup     = group;

            // Count number of match rules, including extended rules
            while (RID_IS_MATCH(pRule->id))
            {
                if (RID_IS_EXTENDED(pRule->id))
                {
                    pRuleSet->iMatchCount += pRule->value;
                    pRule += pRule->value;
                }

                pRuleSet->iMatchCount++;
                pRule++;
            }

            // Count number of set rules, including extended rules
            while (RID_IS_SET(pRule->id))
            {
                if (RID_IS_EXTENDED(pRule->id))
                {
                    pRuleSet->iSetCount += pRule->value;
                    pRule += pRule->value;
                }

                pRuleSet->iSetCount++;
                pRule++;
            }

            // Rule must have at least one "Set" rule
            if (pRuleSet->iSetCount < 1)
            {
                VPHAL_RENDER_ASSERTMESSAGE("Ruleset must have at least one set rule.");
                return false;
            }
        }
    }

    // Rule table is now sorted and integrated with custom rules
    return true;
}

//---------------------------------------------------------------------------------------
// KernelDll_AllocateStates - Allocate Kernel Dynamic Linking/Loading (Dll) States
//
//    - Allocate DL states
//    - Setup export/import list for linking
//    - Prepare pool of search nodes
//    - Load component kernels from binary file
//    - Setup kernel cache
//    - Setup kernel dynamic linking rules
//
// Parameters: [in] pKernelBin        - Pointer to Kernel binary file loaded in sys memory
//             [in] uKernelSize       - Kernel file size
//             [in] pFcPatchBin       - Pointer to FC patch binary file loaded in sys memory
//             [in] uFcPatchCacheSize - FC patch binary file size
//             [in] platform          - Gfx platform
//             [in] pDefaultRules     - Dynamic Linking Rules Table
//
// Output: Pointer to allocated Kernel dll state
//         nullptr - Failed to allocate Kernel dll state
//-----------------------------------------------------------------------------------------
Kdll_State *KernelDll_AllocateStates(
    void                    *pKernelBin,
    uint32_t                uKernelSize,
    void                    *pFcPatchCache,
    uint32_t                uFcPatchCacheSize,
    const Kdll_RuleEntry    *pDefaultRules,
    void(*ModifyFunctionPointers)(PKdll_State))
{
    Kdll_State            *pState;
    Kdll_CacheEntry       *pCacheEntry;
    Kdll_KernelCache      *pKernelCache;
    Kdll_KernelHashTable  *pHashTable;
    Kdll_KernelHashEntry  *pHashEntries;

    int32_t                iSize;
    int32_t                nExports    = 0;
    int32_t                nImports    = 0;
    uint32_t              *pLinkOffset = nullptr;
    Kdll_LinkData         *pLinkSort   = nullptr;
    Kdll_LinkData         *pLinkData;
    Kdll_LinkData         *pExports;
    Kdll_LinkFileHeader   *pLinkHeader;

    int32_t i, j;
    uint32_t *pOffsets;
    uint8_t *pBase;

    VPHAL_RENDER_FUNCTION_ENTER;

    // Allocate dynamic linking states
    i     = sizeof(Kdll_State);                                    // Dynamic linking states
    i    += sizeof(Kdll_CacheEntry) * IDR_VP_TOTAL_NUM_KERNELS;    // Component kernel cache entries
    i    += sizeof(Kdll_CacheEntry) * IDR_VP_TOTAL_NUM_KERNELS;    // CMFC kernel patch cache entries
    i    += sizeof(Kdll_CacheEntry) * DL_DEFAULT_COMBINED_KERNELS; // Combined kernel cache entries
    i    += DL_COMBINED_KERNEL_CACHE_SIZE;                         // Combined kernel buffer
    i    += sizeof(Kdll_LinkData)   * DL_MAX_EXPORT_COUNT;         // Kernel Export table

    pState = (Kdll_State *)MOS_AllocAndZeroMemory(i);
    if (!pState)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Failed to allocate kernel dll states.");
        goto cleanup;
    }
    pState->iSize        = i;
    pState->dwRefresh    = 0;
    pState->pProcamp     = nullptr;
    pState->iProcampSize = 0;
    pState->pSortedRules = nullptr;

    if ((pFcPatchCache != nullptr) && (uFcPatchCacheSize != 0))
    {
        pState->bEnableCMFC = true;
    }

    // Initialize platform specific function pointers
    if (!KernelDll_SetupFunctionPointers(pState, ModifyFunctionPointers))
    {
        VPHAL_RENDER_ASSERTMESSAGE("Failed to setup function pointers.");
        goto cleanup;
    }

    pKernelCache = &pState->ComponentKernelCache;

    // No custom kernels/rules
    pState->pRuleTableCustom   = nullptr;
    pState->pCustomKernelCache = nullptr;

    // Set Kernel DLL Rules
    pState->pRuleTableDefault  = pDefaultRules;

    // Integrate and sort rule tables
    KernelDll_SortRuleTable(pState);

    // Setup component kernel cache
    pKernelCache->pCache           = (uint8_t*)pKernelBin;
    pKernelCache->iCacheSize       = (int32_t)  uKernelSize;
    pKernelCache->iCacheFree       = 0;
    pKernelCache->iCacheMaxEntries = IDR_VP_TOTAL_NUM_KERNELS;
    pKernelCache->iCacheEntries    = IDR_VP_TOTAL_NUM_KERNELS;
    pKernelCache->pCacheEntries    = (Kdll_CacheEntry *)(pState + 1);

    pOffsets    = (uint32_t *) pKernelCache->pCache;
    pBase       = (uint8_t *)(pOffsets + IDR_VP_TOTAL_NUM_KERNELS + 1);
    pCacheEntry = pKernelCache->pCacheEntries;
    for (i = 0; i < IDR_VP_TOTAL_NUM_KERNELS; i++, pCacheEntry++)
    {
        pCacheEntry->iKUID      = i;
        pCacheEntry->iKCID      = -1;
        pCacheEntry->dwLoaded   = 0;
        pCacheEntry->dwRefresh  = 0;
        pCacheEntry->wHashEntry = 0;
        pCacheEntry->szName     = g_cInit_ComponentNames[i];
        pCacheEntry->iSize      = pOffsets[i + 1] - pOffsets[i];
        pCacheEntry->pBinary    = (pCacheEntry->iSize > 0) ? (pBase + pOffsets[i]) : nullptr;
    }

    // Setup CMFC kernel patch cache
    pKernelCache = &pState->CmFcPatchCache;

    if (pState->bEnableCMFC && pFcPatchCache)
    {
        pKernelCache->pCache           = (uint8_t*)pFcPatchCache;
        pKernelCache->iCacheSize       = (int32_t)uFcPatchCacheSize;
        pKernelCache->iCacheFree       = 0;
        pKernelCache->iCacheMaxEntries = IDR_VP_TOTAL_NUM_KERNELS;
        pKernelCache->iCacheEntries    = IDR_VP_TOTAL_NUM_KERNELS;
        pKernelCache->pCacheEntries    = pCacheEntry;

        pOffsets = (uint32_t *)pKernelCache->pCache;
        pBase = (uint8_t *)(pOffsets + IDR_VP_TOTAL_NUM_KERNELS + 1);
        for (i = 0; i < IDR_VP_TOTAL_NUM_KERNELS; i++, pCacheEntry++)
        {
            pCacheEntry->iKUID      = i;
            pCacheEntry->iKCID      = -1;
            pCacheEntry->dwLoaded   = 0;
            pCacheEntry->dwRefresh  = 0;
            pCacheEntry->wHashEntry = 0;
            pCacheEntry->szName     = g_cInit_ComponentNames[i];
            pCacheEntry->iSize      = pOffsets[i + 1] - pOffsets[i];
            pCacheEntry->pBinary    = (pCacheEntry->iSize > 0) ? (pBase + pOffsets[i]) : nullptr;
        }
    }
    else
    {
        pCacheEntry += IDR_VP_TOTAL_NUM_KERNELS;
    }

    // Setup combined kernel cache
    pKernelCache = &pState->KernelCache;
    pKernelCache->iCacheMaxEntries = DL_DEFAULT_COMBINED_KERNELS;
    pKernelCache->iCacheEntries    = 0;
    pKernelCache->iCacheSize       = DL_COMBINED_KERNEL_CACHE_SIZE;     // Size of kernel cache
    pKernelCache->iCacheFree       = DL_COMBINED_KERNEL_CACHE_SIZE;     // Free cache size
    pKernelCache->iCacheID         = 0x00010000;                        // Cache ID
    pKernelCache->pCacheEntries    = pCacheEntry;                       // Cached kernel entries
    pKernelCache->pCache           = (uint8_t *)(pCacheEntry + DL_DEFAULT_COMBINED_KERNELS); // kernels

    // reset cache entries
    for (i = 0; i < DL_DEFAULT_COMBINED_KERNELS; i++, pCacheEntry++)
    {
        pCacheEntry->iKUID      = -1;
        pCacheEntry->iKCID      = -1;
        pCacheEntry->pBinary    = pKernelCache->pCache + i * DL_CACHE_BLOCK_SIZE;
        if(i != DL_DEFAULT_COMBINED_KERNELS - 1)
        {
            pCacheEntry->pNextEntry = pCacheEntry + 1;
        }
        else
        {
            pCacheEntry->pNextEntry = nullptr;
        }
    }

    //------------------------------------
    // Setup hash table
    //------------------------------------
    pHashTable   = &pState->KernelHashTable;
    pHashEntries = pState->KernelHashTable.HashEntry - 1;

    pHashTable->pool    = 1;                          // first in pool (1 based index)
    pHashTable->last    = DL_MAX_COMBINED_KERNELS;    // last in pool (for releasing)

    for (i = 1; i <= DL_MAX_COMBINED_KERNELS; i++)
    {
        pHashEntries[i].next = i + 1;
    }
    pHashEntries[i - 1].next = 0;  // last entry

    //------------------------------------
    // Setup dynamic linking import/export array
    //------------------------------------
    pCacheEntry = pState->ComponentKernelCache.pCacheEntries;
    iSize       = pCacheEntry[IDR_VP_LinkFile].iSize;
    if (iSize == 0)
    {
        VPHAL_RENDER_NORMALMESSAGE("Link file is missing.");
        goto cleanup;
    }

    // Get link file binary data
    pLinkHeader = (Kdll_LinkFileHeader *) pCacheEntry[IDR_VP_LinkFile].pBinary;
    if (pLinkHeader->dwVersion != IDR_VP_LINKFILE_VERSION ||
        sizeof(Kdll_LinkFileHeader) != IDR_VP_LINKFILE_HEADER)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Invalid link file version.");
        goto cleanup;
    }
    iSize = (iSize - IDR_VP_LINKFILE_HEADER) / sizeof(Kdll_LinkData);

    // Create temporary list of sorted link data and offsets
    pLinkSort   = (Kdll_LinkData *)MOS_AllocAndZeroMemory(iSize * sizeof(Kdll_LinkData));
    pLinkOffset = (uint32_t *)MOS_AllocAndZeroMemory((IDR_VP_TOTAL_NUM_KERNELS + 1) * sizeof(uint32_t));
    if (!pLinkSort || !pLinkOffset)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Failed to allocate temporary buffers.");
        goto cleanup;
    }

    // Count number of imports for each component kernel
    pCacheEntry[0].pLink = pLinkData = (Kdll_LinkData *)(pLinkHeader + 1);
    for (i = iSize; i > 0; i--, pLinkData++)
    {
        if (pLinkData->iKUID < IDR_VP_TOTAL_NUM_KERNELS)
        {
            pCacheEntry[pLinkData->iKUID].nLink++;
        }

        nExports += pLinkData->bExport;
        nImports += !pLinkData->bExport;
    }

    // Sanity check
    if (nExports != (int32_t)pLinkHeader->dwExports ||
        nImports != (int32_t)pLinkHeader->dwImports)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Inconsistent header data.");
        goto cleanup;
    }

    if (nExports > DL_MAX_EXPORT_COUNT)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Unsupported number of exports %d > %d.", nExports, DL_MAX_EXPORT_COUNT);
        goto cleanup;
    }

    pState->ComponentKernelCache.pExports = pExports = (Kdll_LinkData *)(pKernelCache->pCache + pKernelCache->iCacheSize);
    pState->ComponentKernelCache.nExports = nExports;

    // Calculate offsets for sorting
    pLinkOffset[0] = 0;
    pLinkData = pCacheEntry[0].pLink;
    for (i = 1; i < IDR_VP_TOTAL_NUM_KERNELS; i++)
    {
        pLinkOffset[i] = pLinkOffset[i-1] + pCacheEntry[i-1].nLink;
        pCacheEntry[i].pLink = (pCacheEntry[i].nLink) ? (pLinkData + pLinkOffset[i]) : nullptr;
    }
    pLinkOffset[i] = pLinkOffset[i-1] + pCacheEntry[i-1].nLink;

    // Sort link data
    for (i = iSize; i > 0; i--, pLinkData++)
    {
        j = pLinkOffset[MOS_MIN(pLinkData->iKUID, IDR_VP_TOTAL_NUM_KERNELS)]++;
        pLinkSort[j] = *pLinkData;

        // Add to export table
        if (pLinkData->bExport &&
            pLinkData->iLabelID < DL_MAX_EXPORT_COUNT)
        {
            pExports[pLinkData->iLabelID] = *pLinkData;
        }
    }

    // Copy sort data
    pLinkData = pCacheEntry[0].pLink;
    MOS_SecureMemcpy(pLinkData, iSize * sizeof(Kdll_LinkData), (void *)pLinkSort, iSize * sizeof(Kdll_LinkData));

    // Release sort buffers
    MOS_FreeMemory(pLinkOffset);
    MOS_FreeMemory(pLinkSort);

    // Return
    return pState;

cleanup:
    if (pState)
    {
        MOS_FreeMemory(pState->pSortedRules);
        pState->pSortedRules = nullptr;
    }

    // Free DL States and temporary sort buffers
    MOS_FreeMemory(pState);
    MOS_FreeMemory(pLinkSort);
    MOS_FreeMemory(pLinkOffset);

    return nullptr;
}

//---------------------------------------------------------------------------------------
// KernelDll_ReleaseStates - Release Kernel Dynamic Linking/Loading (Dll) States
//
// Parameters:
//    Kdll_State *pState - [in] Kernel dll State to release
//
// Output: Pointer to allocated Kernel dll state
//         nullptr - Failed to allocate Kernel dll state
//-----------------------------------------------------------------------------------------
void KernelDll_ReleaseStates(Kdll_State *pState)
{
    VPHAL_RENDER_FUNCTION_ENTER;

    if (!pState) return;
    KernelDll_ReleaseAdditionalCacheEntries(&pState->KernelCache);
    MOS_FreeMemory(pState->ComponentKernelCache.pCache);
    MOS_FreeMemory(pState->CmFcPatchCache.pCache);
    MOS_FreeMemory(pState->pSortedRules);
    MOS_FreeMemory(pState);
}

//---------------------------------------------------------------------------------------
// KernelDll_SetupProcampParameters - Setup Kernel Procamp Parameters
//
// Parameters:
//    Kdll_State   *pState       - [in] Kernel dll State to release
//    Kdll_Procamp *pProcamp     - [in] Pointer to array of Procamp Parameters
//    int32_t       iProcampSize - [in] Size of the array
//
// Output: Pointer to allocated Kernel dll state
//         nullptr - Failed to allocate Kernel dll state
//-----------------------------------------------------------------------------------------
void KernelDll_SetupProcampParameters(Kdll_State    *pState,
                                      Kdll_Procamp  *pProcamp,
                                      int32_t        iProcampSize)
{
    VPHAL_RENDER_FUNCTION_ENTER;

    // Setup pointer to procamp parameters
    pState->pProcamp     = pProcamp;
    pState->iProcampSize = iProcampSize;
}

//---------------------------------------------------------------------------------------
// Kdll_SearchKernel - Performs full kernel search, including selection of best match
//                     Search state must be initialized by KernelDll_StartKernelSearch
//
// Parameters:
//    Kdll_State       *pState       - [in]     Dynamic Linking state
//    Kdll_SearchState *pSearchState - [in/out] Kernel search state
//
// Output: true if suceeded, false otherwise
//---------------------------------------------------------------------------------------
bool KernelDll_SearchKernel(Kdll_State       *pState,
                            Kdll_SearchState *pSearchState)
{
    VPHAL_RENDER_FUNCTION_ENTER;

    // Check parameters
    if ((!pSearchState) || pSearchState->iFilterSize < 1)
    {
        VPHAL_RENDER_NORMALMESSAGE("Search is empty, must contain 2 or more layers.");
        return false;
    }

    // Setup CSC; allocate and calculate CSC matrices
    if (!pState->pfnSetupCSC(pState, pSearchState))
    {
        VPHAL_RENDER_NORMALMESSAGE("CSC setup failed.");
        return false;
    }

    // Initial search states
    pSearchState->bCscBeforeMix = false;
    pSearchState->state        = Parser_Begin;
    pSearchState->cspace       = pSearchState->CscParams.ColorSpace;
    pSearchState->quadrant     = 0;
    pSearchState->layer_number = 0;

    pSearchState->pMatchingRuleSet = nullptr;

    // Reset Src0 state
    pSearchState->src0_format   = Format_None;
    pSearchState->src0_sampling = Sample_None;
    pSearchState->src0_colorfill = false;
    pSearchState->src0_lumakey   = LumaKey_False;
    pSearchState->src0_coeff    = CoeffID_None;

    // Reset Src1 state
    pSearchState->src1_format   = Format_None;
    pSearchState->src1_sampling = Sample_None;
    pSearchState->src1_lumakey   = LumaKey_False;
    pSearchState->src1_samplerlumakey = LumaKey_False;
    pSearchState->src1_coeff    = CoeffID_None;
    pSearchState->src1_process  = Process_None;

    // Search loop
    while (pSearchState->state != Parser_End)
    {
#if EMUL || VPHAL_LIB
        if (pState->pfnCbSearchSate)
        {
            pState->pfnCbSearchSate(pState->pToken, CB_REASON_BEGIN_SEARCH, pSearchState);
        }
#endif
        // Find rule that matches
        if (!pState->pfnFindRule(pState, pSearchState))
        {
#if EMUL || VPHAL_LIB
            if (pState->pfnCbSearchSate)
            {
                pState->pfnCbSearchSate(pState->pToken, CB_REASON_SEARCH_FAILED, pSearchState);
            }
#endif
            return false;
        }

#if EMUL || VPHAL_LIB
        if (pState->pfnCbSearchSate)
        {
            pState->pfnCbSearchSate(pState->pToken, CB_REASON_BEGIN_UPDATE, pSearchState);
        }
#endif
        // Update state
        if (!pState->pfnUpdateState(pState, pSearchState))
        {
#if EMUL || VPHAL_LIB
            if (pState->pfnCbSearchSate)
            {
                pState->pfnCbSearchSate(pState->pToken, CB_REASON_UPDATE_FAILED, pSearchState);
            }
#endif
            return false;
        }
    }

#if EMUL || VPHAL_LIB
    if (pState->pfnCbSearchSate)
    {
        pState->pfnCbSearchSate(pState->pToken, CB_REASON_END_SEARCH, pSearchState);
    }
#endif

    VPHAL_RENDER_VERBOSEMESSAGE("Search completed successfully.");
    return true;
}

//--------------------------------------------------------------
// Append kernel, include symbols to resolve
//--------------------------------------------------------------
bool Kdll_AppendKernel(Kdll_KernelCache *pKernelCache,
                       Kdll_SearchState *pSearchState,
                       int32_t           iKUID,
                       Kdll_PatchData   *pKernelPatch)
{
    Kdll_State      *pState;
    Kdll_Symbol     *pSymbols;
    Kdll_CacheEntry *kernels;
    Kdll_LinkData   *link;
    Kdll_LinkData   *liSearch_reloc;
    uint8_t         *kernel;
    int             *size;
    int             *left;
    int dwSize;
    int i;
    int             base;
    bool            bInline;
    bool            res;

    VPHAL_RENDER_FUNCTION_ENTER;

    res = false;

    // Check if Kernel ID is valid
    if (iKUID >= pKernelCache->iCacheEntries)
    {
        VPHAL_RENDER_NORMALMESSAGE("invalid Kernel ID %d.", iKUID);
        goto cleanup;
    }

    // Get KDLL state
    pState   = pSearchState->pKdllState;

    // Get current combined kernel
    kernel   = pSearchState->Kernel;
    size     = &pSearchState->KernelSize;
    left     = &pSearchState->KernelLeft;
    pSymbols = &pSearchState->KernelLink;
    base     = (*size) >> 2;

    // Find selected kernel and kernel size; check if there is enough space
    kernels  = &pKernelCache->pCacheEntries[iKUID];
    dwSize = kernels->iSize;
    if (*left < dwSize)
    {
        VPHAL_RENDER_NORMALMESSAGE("exceeded maximum kernel size.");
        goto cleanup;
    }

    // Check if there is enough space for symbols
    if (pSymbols->dwCount + kernels->nLink >= pSymbols->dwSize)
    {
        VPHAL_RENDER_NORMALMESSAGE("exceeded maximum numbers of symbols to resolve.");
        goto cleanup;
    }

#if EMUL || VPHAL_LIB
    VPHAL_RENDER_NORMALMESSAGE("%s.", kernels->szName);

    if (pState->pfnCbListKernel)
    {
        pState->pfnCbListKernel(pState->pToken, kernels->szName);
    }
#elif _DEBUG // EMUL || VPHAL_LIB
    VPHAL_RENDER_NORMALMESSAGE("%s.", kernels->szName);
#endif // _DEBUG

    // Append symbols to resolve, relocate symbols
    link = kernels->pLink;
    liSearch_reloc = pSymbols->pLink + pSymbols->dwCount;

    bInline = false;
    if (link)
    {
        for (i = kernels->nLink; i > 0; i--, link++)
        {
            if (link->bInline)
            {
                // Inline code included
                if (!link->bExport)
                {
                    bInline = true;
                }
            }
            else
            {
                *liSearch_reloc = *link;
                liSearch_reloc->dwOffset += base;
                liSearch_reloc++;

                pSymbols->dwCount++;
            }
        }
    }

    // Append kernel
    MOS_SecureMemcpy(&kernel[*size], dwSize, (void *)kernels->pBinary, dwSize);

    // Patch kernel
    if (pKernelPatch)
    {
        uint8_t *pSource      = pKernelPatch->Data;
        uint8_t *pDestination = kernel + (*size);
        int32_t i;

        Kdll_PatchBlock *pBlock = pKernelPatch->Patch;
        for (i = pKernelPatch->nPatches; i > 0; i--, pBlock++)
        {
            MOS_SecureMemcpy(pDestination + pBlock->DstOffset, pBlock->BlockSize, (void *)(pSource + pBlock->SrcOffset), pBlock->BlockSize);
        }
    }

    res = true;
    *size += dwSize;
    *left -= dwSize;

    // Insert inline code
    if (bInline)
    {
        for (link = kernels->pLink, i = kernels->nLink; (i > 0) && (res); i--, link++)
        {
            if (link->bInline && (!link->bExport))
            {
                iKUID = pKernelCache->pExports[link->iLabelID].iKUID;
                res &= Kdll_AppendKernel(pKernelCache, pSearchState, iKUID, pKernelPatch);
            }
        }
    }

cleanup:
    return res;
}

//--------------------------------------------------------------
// Resolve kernel dependencies and perform patching
//--------------------------------------------------------------
bool Kdll_ResolveKernelDependencies(
    Kdll_State         *pState,
    Kdll_SearchState   *pSearchState)
{
    Kdll_KernelCache *cache    = &pState->ComponentKernelCache;
    uint8_t          *kernel   =  pSearchState->Kernel;
    Kdll_Symbol      *pSymbols = &pSearchState->KernelLink;
    uint32_t          nExports = cache->nExports;
    Kdll_LinkData    *pExports = cache->pExports;
    Kdll_LinkData    *pLink;
    int32_t           iKUID;
    int32_t           iOffset;
    uint32_t          dwResolveOffset[DL_MAX_EXPORT_COUNT];
    bool              bResolveDone;
    int32_t           i;
    uint32_t         *d;

    VPHAL_RENDER_FUNCTION_ENTER;

    MOS_ZeroMemory(dwResolveOffset, sizeof(dwResolveOffset));

    do
    {
        // Update exports
        for (pLink = pSymbols->pLink, i = pSymbols->dwCount; i > 0; i--, pLink++)
        {
            if (pLink->bExport)
            {
                dwResolveOffset[pLink->iLabelID] = pLink->dwOffset;
            }
        }

        bResolveDone = true;
        for (pLink = pSymbols->pLink, i = pSymbols->dwCount; i > 0; i--, pLink++)
        {
            // validate label
            if (pLink->iLabelID > nExports ||              // invalid label
                pExports[pLink->iLabelID].bExport == 0)    // label not in the export table
            {
                VPHAL_RENDER_ASSERTMESSAGE("Invalid/unresolved label %d.", pLink->iLabelID);
                return false;
            }

            // load dependencies
            if (!pLink->bExport && !dwResolveOffset[pLink->iLabelID])
            {
                // set flag for another pass as newly loaded
                // kernels may contain dependencies of their own
                bResolveDone = false;

                // Load dependencies
                iKUID = pExports[pLink->iLabelID].iKUID;
                Kdll_AppendKernel(cache, pSearchState, iKUID, nullptr);

                // Restart
                break;
            }
        } // for
    } while (!bResolveDone);

    // All modules must be loaded by now, start patching
    for (pLink = pSymbols->pLink, i = pSymbols->dwCount; i > 0; i--, pLink++)
    {
        iOffset  = (int32_t)dwResolveOffset[pLink->iLabelID] - 4;
        iOffset -= pLink->dwOffset;

        d = ((uint32_t *)kernel) + pLink->dwOffset;

        // Patch offset
        if (!pLink->bExport && !pLink->bInline)
        {
            d[3] = iOffset << 2; // jmpi - index * 8 bits
        }
    }

    return true;
}

//--------------------------------------------------------------
// Fowler/Noll/Vo FNV-1a hash algorithm - public domain
//--------------------------------------------------------------
uint32_t KernelDll_SimpleHash(void *pData, int32_t iSize)
{
   static const uint32_t k = 0x1000193;
   uint32_t hash = 0x811c9dc5;
   char *p = (char *)pData;

   for(; iSize > 0; iSize--)
   {
      hash ^= (*p++);
      hash *= k;
   }

   return hash;
}

//--------------------------------------------------------------
// KernelDll_GetCombinedKernel - Search combined kernel
//--------------------------------------------------------------
Kdll_CacheEntry *KernelDll_GetCombinedKernel(
    Kdll_State          *pState,
    Kdll_FilterEntry    *pFilter,
    int32_t             iFilterSize,
    uint32_t            dwHash)
{
    Kdll_KernelHashTable *pHashTable;
    Kdll_KernelHashEntry *entries, *curr, *next;
    uint32_t folded_hash;
    uint16_t entry;

    VPHAL_RENDER_FUNCTION_ENTER;

    // Get hash table
    pHashTable = &pState->KernelHashTable;

    // fold hash from 32 to 8 bit :-)
    FOLD_HASH(folded_hash, dwHash)

    // No entries
    entry = pHashTable->wHashTable[folded_hash];
    if (entry == 0 || entry > DL_MAX_COMBINED_KERNELS ) return nullptr;

    entries = (&pHashTable->HashEntry[0]) - 1;  // all indices are 1 based (0 means null)
    curr    = &entries[entry];
    for (; (curr != nullptr); curr = next)
    {
        // match 32-bit hash, then compare filter
        if (curr->dwHash  == dwHash &&
            curr->iFilter == iFilterSize)
        {
            if (memcmp(curr->pFilter, pFilter, iFilterSize * sizeof(Kdll_FilterEntry)) == 0)
            {
                break;
            }
        }

        // Next entry with the same 8-bit folded hash
        next = (curr->next) ? (&entries[curr->next]) : nullptr;
    }

    if (curr)
    {   // Kernel already cached
        curr->pCacheEntry->dwRefresh = pState->dwRefresh++;
        return (curr->pCacheEntry);
    }
    else
    {   // Kernel must be built
        return nullptr;
    }
}

//--------------------------------------------------------------
// KernelDll_AllocateHashEntry - Allocate hash entry
//--------------------------------------------------------------
uint16_t KernelDll_AllocateHashEntry(Kdll_KernelHashTable *pHashTable,
                                 uint32_t              hash)
{
    Kdll_KernelHashEntry *pHashEntry = &pHashTable->HashEntry[0] - 1;
    Kdll_KernelHashEntry *pNewEntry;
    uint32_t folded_hash;
    uint16_t entry;

    VPHAL_RENDER_FUNCTION_ENTER;

    entry = pHashTable->pool;
    if (!entry)
    {
        return 0;
    }

    // Get entry from pool
    pNewEntry = &pHashEntry[entry];
    pHashTable->pool = pNewEntry->next;
    if (pHashTable->last == entry)
    {
        pHashTable->last = 0;
    }

    // Initialize entry, attach to the hash table
    FOLD_HASH(folded_hash, hash);
    pNewEntry->dwHash      = hash;
    pNewEntry->next        = pHashTable->wHashTable[folded_hash];
    pNewEntry->iFilter     = 0;
    pNewEntry->pFilter     = nullptr;
    pNewEntry->pCacheEntry = nullptr;
    pHashTable->wHashTable[folded_hash] = entry;
    return entry;
}

//--------------------------------------------------------------
// KernelDll_ReleaseHashEntry - Release hash table entry
//--------------------------------------------------------------
void KernelDll_ReleaseHashEntry(Kdll_KernelHashTable *pHashTable, uint16_t entry)
{
    Kdll_KernelHashEntry *pHashEntry = &pHashTable->HashEntry[0] - 1;
    uint32_t folded_hash;
    uint16_t next;

    VPHAL_RENDER_FUNCTION_ENTER;

    if (entry == 0)
    {
        return;
    }

    // unlink entry
    next = pHashEntry[entry].next;
    pHashEntry[entry].next = 0;

    // remove references to entry from hash table
    FOLD_HASH(folded_hash, pHashEntry[entry].dwHash);
    if (pHashTable->wHashTable[folded_hash] == entry)
    {
        pHashTable->wHashTable[folded_hash] = next;
    }
    else
    {
        uint16_t prev = pHashTable->wHashTable[folded_hash];

        while (prev != 0 &&
               pHashEntry[prev].next != entry)
        {
            prev = pHashEntry[prev].next;
        }

        if (prev)
        {
            pHashEntry[prev].next = next;
        }
    }

    // return entry to pool
    if (pHashTable->pool == 0)
    {
        pHashTable->pool = entry;
    }
    else
    {
        pHashEntry[pHashTable->last].next = entry;
    }
    pHashTable->last = entry;
}

//--------------------------------------------------------------
// KernelDll_ReleaseCacheEntry - Release cache entry
//--------------------------------------------------------------
void KernelDll_ReleaseCacheEntry(Kdll_KernelCache *pCache,
                                 Kdll_CacheEntry  *pEntry)
{
    pEntry->iKUID = -1;
    pEntry->iKCID = -1;
    pCache->iCacheEntries--;
}

//--------------------------------------------------------------
// KernelDll_CacheGarbageCollection - performs garbage collection
//--------------------------------------------------------------
bool KernelDll_GarbageCollection(Kdll_State *pState, int32_t size)
{
    Kdll_KernelCache     *pCache     = &pState->KernelCache;
    Kdll_CacheEntry      *pEntry     = pCache->pCacheEntries;
    Kdll_CacheEntry      *pOldest    = nullptr;
    Kdll_KernelHashTable *pHashTable = &pState->KernelHashTable;
    Kdll_KernelHashEntry *pHashEntry = &pHashTable->HashEntry[0] - 1;
    uint32_t              dwOldest   = (uint32_t)-1;
    uint16_t              wEntry     = 0;
    int32_t i;

    MOS_UNUSED(size);

    VPHAL_RENDER_FUNCTION_ENTER;

    // Adjust refresh values to avoid overflow
    if (pState->dwRefresh > 0xffff0000)
    {
        pState->dwRefresh -= 0x80000000;
        for (i = pCache->iCacheMaxEntries; i > 0; i--)
        {
            if (pEntry->dwRefresh < 0x80000000)
                pEntry->dwRefresh = 0;
            else
                pEntry->dwRefresh -= 0x80000000;
            pEntry = pEntry->pNextEntry;
        }
    }

    // No need to deallocate old entries
    if (pCache->iCacheEntries < DL_MAX_COMBINED_KERNELS)
    {
        return true;
    }

    for (i = pCache->iCacheMaxEntries; i > 0; i--)
    {
        // deallocate old unreferenced entries
        if (pEntry->iKCID != -1 && pEntry->dwLoaded == 0)
        {
            if (pEntry->dwRefresh < dwOldest)
            {
                pOldest  = pEntry;
                dwOldest = pEntry->dwRefresh;
                wEntry   = pEntry->wHashEntry;
            }
        }
        pEntry = pEntry->pNextEntry;
    }

    // No entry to release, sanity checks
    pHashEntry += wEntry;
    if (!pOldest ||
        wEntry == 0 ||
        pHashEntry->pCacheEntry != pOldest)
    {
        VPHAL_RENDER_ASSERT(false);
        return false;
    }

    // Release hash and cache entries
    KernelDll_ReleaseHashEntry(pHashTable, wEntry);
    KernelDll_ReleaseCacheEntry(pCache, pOldest);

    return true;
}

//--------------------------------------------------------------
// KernelDll_AllocateAdditionalCacheEntries - Allocate more kernel cache entries
//--------------------------------------------------------------
Kdll_CacheEntry *
KernelDll_AllocateAdditionalCacheEntries(Kdll_KernelCache *pCache)
{
    Kdll_CacheEntry *pNewEntry = nullptr;
    Kdll_CacheEntry *pChcheEntry;
    int i, j;

    VPHAL_RENDER_FUNCTION_ENTER;

    // Check num
    if (pCache->iCacheEntries + DL_NEW_COMBINED_KERNELS > DL_MAX_COMBINED_KERNELS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("KernelDll_AllocateAdditionalCacheEntries: Can't allocate more kernel cache entries\n");
        return nullptr;
    }

    // Allocate the new entires
    i = (sizeof(Kdll_CacheEntry) + DL_CACHE_BLOCK_SIZE) * DL_NEW_COMBINED_KERNELS;
    pNewEntry = (Kdll_CacheEntry *)MOS_AllocAndZeroMemory(i);
    if (!pNewEntry)
    {
        VPHAL_RENDER_ASSERTMESSAGE("KernelDll_AllocateAdditionalCacheEntries: Failed to allocate kernel cache entries\n");
        return nullptr;
    }

    // Update the cache entires
    pChcheEntry = pCache->pCacheEntries;
    for(j = 0; j < pCache->iCacheMaxEntries - 1; j++)
    {
        pChcheEntry = pChcheEntry->pNextEntry;
    }
    pChcheEntry->pNextEntry = pNewEntry;
    for(j = 0; j < DL_NEW_COMBINED_KERNELS; j++, pNewEntry++)
    {
        pNewEntry->iKUID   = -1;
        pNewEntry->iKCID   = -1;
        pNewEntry->pBinary = (uint8_t *)(pNewEntry + DL_NEW_COMBINED_KERNELS - j) + j * DL_CACHE_BLOCK_SIZE;
        if(j != DL_NEW_COMBINED_KERNELS - 1)
        {
            pNewEntry->pNextEntry = pNewEntry + 1;
        }
        else
        {
            pNewEntry->pNextEntry = nullptr;
        }
    }

    pCache->iCacheMaxEntries += DL_NEW_COMBINED_KERNELS;
    pCache->iCacheSize       += DL_NEW_COMBINED_KERNELS * DL_CACHE_BLOCK_SIZE;
    pCache->iCacheFree       += DL_NEW_COMBINED_KERNELS * DL_CACHE_BLOCK_SIZE;
    return (Kdll_CacheEntry *)(pNewEntry - DL_NEW_COMBINED_KERNELS);
}

//--------------------------------------------------------------
// KernelDll_ReleaseAdditionalCacheEntries - Release the additional kernel cache entries
//--------------------------------------------------------------
void KernelDll_ReleaseAdditionalCacheEntries(Kdll_KernelCache *pCache)
{
    VPHAL_RENDER_FUNCTION_ENTER;
    if(pCache->iCacheMaxEntries > DL_DEFAULT_COMBINED_KERNELS)
    {
        Kdll_CacheEntry *pNewEntries, *pEntries;
        pNewEntries = (pCache->pCacheEntries + DL_DEFAULT_COMBINED_KERNELS - 1)->pNextEntry;
        for(int i = 0; i < (pCache->iCacheMaxEntries - DL_DEFAULT_COMBINED_KERNELS) / DL_NEW_COMBINED_KERNELS; i++)
        {
            pEntries = (pNewEntries + DL_NEW_COMBINED_KERNELS - 1)->pNextEntry;
            MOS_FreeMemory(pNewEntries);
            pNewEntries = pEntries;
        }
    }
}

//--------------------------------------------------------------
// KernelDll_AllocateCacheEntry - Allocate cache entry for a given size
//--------------------------------------------------------------
Kdll_CacheEntry *
KernelDll_AllocateCacheEntry(Kdll_KernelCache *pCache, int32_t iSize)
{
    Kdll_CacheEntry *pEntry          = pCache->pCacheEntries;
    uint8_t *pCacheBinary               = nullptr;
    Kdll_CacheEntry *pCacheNextEntry = nullptr;
    int32_t i, j;

    VPHAL_RENDER_FUNCTION_ENTER;

    // Check size
    if (iSize > DL_CACHE_BLOCK_SIZE)
    {
        return nullptr;
    }

    // Search empty entry
    j = pCache->iCacheMaxEntries;
    for (i = 0; i < j; i++)
    {
        if (pEntry->iKCID == -1)
        {
            break;
        }
        pEntry = pEntry->pNextEntry;
    }
    if (i == j)
    {
        // Try to allocate more cache entries
        pEntry = KernelDll_AllocateAdditionalCacheEntries(pCache);
        if(! pEntry)
        {
            return nullptr;
        }
    }

    // Reset entry
    pCacheBinary    = pEntry->pBinary;
    pCacheNextEntry = pEntry->pNextEntry;
    MOS_ZeroMemory(pEntry, sizeof(Kdll_CacheEntry));
    pEntry->iSize   = iSize;
    pEntry->pBinary    = pCacheBinary;
    pEntry->pNextEntry = pCacheNextEntry;

    // Increment entries
    pCache->iCacheEntries++;
    return pEntry;
}

//--------------------------------------------------------------
// Kerneldll_GetComponentKernel - Get component/static kernel
//                                entry from cache
//--------------------------------------------------------------
Kdll_CacheEntry *
KernelDll_GetComponentKernel(Kdll_State *pState,
                             int32_t     iKUID)
{
    Kdll_CacheEntry *pEntry = nullptr;

    if (iKUID < pState->ComponentKernelCache.iCacheMaxEntries)
    {
        pEntry = &(pState->ComponentKernelCache.pCacheEntries[iKUID]);
        if (pEntry->iKUID != iKUID ||
            pEntry->pBinary == nullptr ||
            pEntry->iSize == 0)
        {
            pEntry = nullptr;
        }
    }

    return pEntry;
}

//--------------------------------------------------------------
// KernelDll_AddKernel - Add kernel into hash table and kernel cache
//--------------------------------------------------------------
Kdll_CacheEntry *
KernelDll_AddKernel(Kdll_State       *pState,           // Kernel Dll state
                    Kdll_SearchState *pSearchState,     // Search state
                    Kdll_FilterEntry *pFilter,          // Original filter
                    int32_t           iFilterSize,      // Original filter size
                    uint32_t          dwHash)
{
    Kdll_CacheEntry      *pCacheEntry;
    Kdll_KernelHashTable *pHashTable;
    Kdll_KernelHashEntry *pHashEntry;
    uint16_t entry;
    int32_t size;
    uint8_t *ptr;

    VPHAL_RENDER_FUNCTION_ENTER;

    // Check kernel
    if (pSearchState->KernelSize <= 0)
    {
        return nullptr;
    }

    // Get hash table
    pHashTable = &pState->KernelHashTable;
    pHashEntry = &pHashTable->HashEntry[0] - 1;  // all indices are 1 based (0 = null)

    // allocate space in kernel cache to store the kernel, filter, CSC parameters
    size  = pSearchState->KernelSize +                                  // Kernel
            pSearchState->iFilterSize * sizeof(Kdll_FilterEntry) * 2 +  // Original + Modified Filter
            sizeof(Kdll_CSC_Params) +                                   // CSC parameters
            sizeof(VPHAL_CSPACE);                                       // Intermediate Color Space for colorfill

    // Run garbage collection, create space for new kernel and metadata
    KernelDll_GarbageCollection(pState, size);

    // Get new kernel cache entry
    pCacheEntry = KernelDll_AllocateCacheEntry(&pState->KernelCache, size);
    if (!pCacheEntry)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Failed to allocate cache space for new kernel.");
        return nullptr;
    }

    // Get hash entry
    entry = KernelDll_AllocateHashEntry(pHashTable, dwHash);
    if (!entry)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Failed to allocate hash entry for new kernel.");
        KernelDll_ReleaseCacheEntry(&pState->KernelCache, pCacheEntry);
        return nullptr;
    }

    // Setup cache entry, copy kernel
    pCacheEntry->iKUID       = -1;
    pCacheEntry->iKCID       = pState->KernelCache.iCacheID;  // Create new kernel cache id (KCID)
    pCacheEntry->dwRefresh   = pState->dwRefresh++;
    pCacheEntry->wHashEntry  = entry;

    // Save kernel
    pCacheEntry->iSize = pSearchState->KernelSize;
    MOS_SecureMemcpy(pCacheEntry->pBinary, pSearchState->KernelSize, (void *)pSearchState->Kernel, pSearchState->KernelSize);
    ptr = pCacheEntry->pBinary + pSearchState->KernelSize;

    // Save modified filter
    pCacheEntry->iFilterSize = pSearchState->iFilterSize;
    pCacheEntry->pFilter     = (Kdll_FilterEntry *) (ptr);
    MOS_SecureMemcpy(ptr, pSearchState->iFilterSize * sizeof(Kdll_FilterEntry), (void *)pSearchState->Filter, pSearchState->iFilterSize * sizeof(Kdll_FilterEntry));
    ptr += pSearchState->iFilterSize * sizeof(Kdll_FilterEntry);

    // Save CSC parameters associated with the kernel
    pCacheEntry->pCscParams = (Kdll_CSC_Params *) (ptr);
    MOS_SecureMemcpy(ptr, sizeof(Kdll_CSC_Params), (void *)&pSearchState->CscParams, sizeof(Kdll_CSC_Params));
    ptr += sizeof(Kdll_CSC_Params);
    // Save intermediate color space for colorfill
    pCacheEntry->colorfill_cspace = pState->colorfill_cspace;
    ptr += sizeof(VPHAL_CSPACE);

    // increment KCID (Range = 0x00010000 - 0x7fffffff)
    pState->KernelCache.iCacheID = 0x00010000 + (pState->KernelCache.iCacheID - 0x0000ffff) % 0x7fff0000;

    // Setup hash entry, copy filter
    pHashEntry += entry;
    pHashEntry->pCacheEntry = pCacheEntry;

    // Save original filter for search purposes - modified filter is used for rendering
    pHashEntry->iFilter     = iFilterSize;
    pHashEntry->pFilter     = (Kdll_FilterEntry *) (ptr);
    MOS_SecureMemcpy(ptr, iFilterSize * sizeof(Kdll_FilterEntry), (void *)pFilter, iFilterSize * sizeof(Kdll_FilterEntry));

    return pCacheEntry;
}

//--------------------------------------------------------------
// KernelDll_BuildKernel - build kernel
//--------------------------------------------------------------
bool KernelDll_BuildKernel(Kdll_State *pState, Kdll_SearchState *pSearchState)
{
    Kdll_KernelCache *pKernelCache = &pState->ComponentKernelCache;
    Kdll_KernelCache *pCustomCache =  pState->pCustomKernelCache;
    Kdll_PatchData   *pKernelPatch;
    bool              res;
    int32_t           offset = 0;
    int32_t           *pKernelID, *pGroupID, *pPatchID;

    VPHAL_RENDER_FUNCTION_ENTER;

    pSearchState->KernelLink.dwSize  = DL_MAX_SYMBOLS;
    pSearchState->KernelLink.dwCount = 0;
    pSearchState->KernelLink.pLink   = pSearchState->LinkArray;
    pSearchState->KernelSize         = 0;
    pSearchState->KernelLeft         = sizeof(pSearchState->Kernel);
    pSearchState->KernelLink.dwCount = 0;

#if EMUL || VPHAL_LIB || _DEBUG || _RELEASE_INTERNAL
    VPHAL_RENDER_NORMALMESSAGE("Component Kernels:");
#endif // EMUL || VPHAL_LIB || _DEBUG

    pKernelID = pSearchState->KernelID;
    pGroupID  = pSearchState->KernelGrp;
    pPatchID  = pSearchState->PatchID;

    for (offset = 0; offset < pSearchState->KernelCount; offset++, pKernelID++, pGroupID++, pPatchID++)
    {
        // Get patch information associated with the kernel
        pKernelPatch = (*pPatchID >= 0) ? &(pSearchState->Patches[*pPatchID]) : nullptr;

        // Append/Patch kernel from custom cache
        if (*pGroupID == GROUP_CUSTOM)
        {
            res = Kdll_AppendKernel(pCustomCache, pSearchState, *pKernelID, pKernelPatch);
        }
        // Append/Patch kernel from internal cache
        else
        {
            res = Kdll_AppendKernel(pKernelCache, pSearchState, *pKernelID, pKernelPatch);
        }

        if (!res)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to build kernel ID %d.", pSearchState->KernelID[offset]);
            return false;
        }
        else
        {
            Kdll_CacheEntry *kernels = (*pGroupID == GROUP_CUSTOM) ? &pCustomCache->pCacheEntries[*pKernelID] : &pKernelCache->pCacheEntries[*pKernelID];
            VPHAL_RENDER_NORMALMESSAGE("Component kernels [%d]: %s", *pKernelID, kernels->szName);
        }
    }

    // Resolve kernel dependencies
    res = Kdll_ResolveKernelDependencies(pState, pSearchState);
    if (!res)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Failed to resolve symbols.");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------
// KernelDll_StartKernelSearch - Starts kernel search
//
// Parameters:
//    Kdll_State       *pState       - [in]     Dynamic Linking State
//    Kdll_FilterEntry *pFilter      - [in]     Search filter (array of search entries)
//    int               iFilterSize  - [in]     Search filter size
//    Kdll_SearchState *pSearchState - [in/out] Kernel search state
//
// Output: none
//---------------------------------------------------------------------------------------
void KernelDll_StartKernelSearch(
    Kdll_State       *pState,
    Kdll_SearchState *pSearchState,
    Kdll_FilterEntry *pFilter,
    int32_t          iFilterSize,
    uint32_t         uiIs64BInstrEnabled)
{
    int32_t nLayer;

    VPHAL_RENDER_FUNCTION_ENTER;

    // Reset all states
    MOS_ZeroMemory(pSearchState, sizeof(Kdll_SearchState));

    // Setup KDLL state
    pSearchState->pKdllState    = pState;     // KDLL state

    // Cleanup kernel table
    pSearchState->KernelCount   = 0;          // # of kernels

    // Cleanup patch data
    memset(pSearchState->Patches ,  0, sizeof(pSearchState->Patches));
    memset(pSearchState->PatchID , -1, sizeof(pSearchState->PatchID));
    pSearchState->PatchCount = 0;

    // Copy original filter; filter will be modified as part of the search
    if (pFilter && iFilterSize > 0)
    {
        MOS_SecureMemcpy(pSearchState->Filter, iFilterSize * sizeof(Kdll_FilterEntry), pFilter, iFilterSize * sizeof(Kdll_FilterEntry));
        pSearchState->pFilter      = pSearchState->Filter;
        pSearchState->iFilterSize  = iFilterSize;

        for (nLayer = 0; nLayer < iFilterSize; nLayer++)
        {
            // DScale Kernels are enabled for all gen9 stepping.
            //For Gen9+, kernel don't support sublayer DScale+rotation
            //Sampler_unorm does not support Y410/RGB10, we need to use sampler_16 to support Y410/RGB10
            if (!pFilter[nLayer].bEnableDscale &&
                (!pFilter[nLayer].bWaEnableDscale ||
                (pFilter[nLayer].layer == Layer_SubVideo &&
                 pFilter[nLayer].rotation != VPHAL_ROTATION_IDENTITY)))
            {
                if (pFilter[nLayer].sampler == Sample_Scaling_034x)
                {
                    pSearchState->pFilter[nLayer].sampler = Sample_Scaling;
                }
                else if (pFilter[nLayer].sampler == Sample_iScaling_034x)
                {
                    pSearchState->pFilter[nLayer].sampler = Sample_iScaling;
                }
                else if (pFilter[nLayer].sampler == Sample_iScaling_AVS)
                {
                    pSearchState->pFilter[nLayer].sampler = Sample_iScaling_AVS;
                }
            }
        }

        // Copy the render target format
        pSearchState->target_format = pSearchState->pFilter[iFilterSize - 1].format;

        // Copy the render target tile type
        pSearchState->target_tiletype = pSearchState->pFilter[iFilterSize - 1].tiletype;

        // Indicate whether to use 64B save kernel for render target surface
        if (uiIs64BInstrEnabled                               &&
            ((pSearchState->target_tiletype == MOS_TILE_X)    ||
            (pSearchState->target_tiletype  == MOS_TILE_LINEAR)))
        {
            pSearchState->b64BSaveEnabled = true;
        }
    }
}

//---------------------------------------------------------------------------------------
// KernelDll_SetupFunctionPointers - Setup Function pointers based on platform
//
// Parameters:
//    char  *pState    - [in/out] Kernel Dll state
//           platform  - [in] platform
//
// Output: true  - Function pointers are set
//         false - Failed to setup function pointers (invalid platform)
//-----------------------------------------------------------------------------------------
static bool KernelDll_SetupFunctionPointers(
    Kdll_State  *pState,
    void        (*ModifyFunctionPointers)(PKdll_State))
{
    VPHAL_RENDER_FUNCTION_ENTER;

    pState->pfnSetupCSC          = KernelDll_SetupCSC;
    pState->pfnMapCSCMatrix      = KernelDll_MapCSCMatrix;
    pState->pfnFindRule          = KernelDll_FindRule;
    pState->pfnUpdateState       = KernelDll_UpdateState;
    pState->pfnSearchKernel      = KernelDll_SearchKernel;
    pState->pfnBuildKernel       = KernelDll_BuildKernel;
    pState->pfnStartKernelSearch = KernelDll_StartKernelSearch;

    if (ModifyFunctionPointers != nullptr)
    {
        (*ModifyFunctionPointers)(pState);
    }

#if EMUL || VPHAL_LIB
    // Disable callbacks
    pState->pToken = nullptr;
    pState->pfnCbListKernel = nullptr;
    pState->pfnCbSearchSate = nullptr;
#endif // EMUL || VPHAL_LIB

    return true;
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_SetupCSC
| Purpose   : Defines CSC conversions necessary for a given filter
|
| Input     : pState       - Kernel Dll state
|             pSearchState - current DL search state
|
| Return    :
\---------------------------------------------------------------------------*/
bool KernelDll_SetupCSC(
    Kdll_State       *pState,
    Kdll_SearchState *pSearchState)
{
    int i, m;                                       // Integer iterators

    bool bCoeffID_0_Used = false;

    VPHAL_CSPACE     cspace      = CSpace_None;      // Current ColorSpace
    VPHAL_CSPACE     out_cspace  = CSpace_None;      // Render Target CS
    VPHAL_CSPACE     main_cspace = CSpace_None;      // Main video CS
    VPHAL_CSPACE     sel_cspace  = CSpace_Any;       // Selected CS

    Kdll_FilterEntry *pFilter;                      // Current Filter information
    int               iFilterSize = pSearchState->iFilterSize;
    Kdll_CSC_Params  *pCSC        = &pSearchState->CscParams;

    int             csc_count;                      // Number of CSC operations
    int             matrix_count;                   // Number of Matrices in use
    int             procamp_count = 0;              // Number of PA operations
    int             sel_csc_count = -1;             // Minimum number of CSC operations
    int             iCoeffID;                       // coeffID for layers other than main video
    uint8_t         cspace_in_use[CSpace_Count];    // Color Spaces in use

    Kdll_CSC_Matrix   curr_matrix;
    Kdll_CSC_Matrix  *matrix   = pCSC->Matrix;      // Color Space conversion matrix
    uint8_t          *matrixID = pCSC->MatrixID;    // CSC coefficient allocation table

    // Clear all CSC matrices
    MOS_ZeroMemory(matrix, sizeof(pCSC->Matrix));
    memset(matrixID, DL_CSC_DISABLED, sizeof(pCSC->MatrixID));
    memset(pCSC->PatchMatrixID, DL_CSC_DISABLED, sizeof(pCSC->PatchMatrixID));
    pCSC->PatchMatrixNum = 0;

    // Clear array of color spaces in use
    MOS_ZeroMemory(cspace_in_use, sizeof(cspace_in_use));

    //---------------------------------------------------------------//
    // Collect information about Color Spaces in use
    // Get Primary Video and Render Target Color Spaces
    // Force xvYCC passthrough if enabled
    //---------------------------------------------------------------//
    for (i = iFilterSize, pFilter = pSearchState->Filter; i > 0; i--, pFilter++)
    {
        // Disable Procamp for all layers except Main Video
        // Disable Procamp if source is RGB
        if (pFilter->layer  != Layer_MainVideo ||
            pFilter->cspace == CSpace_sRGB     ||
            pFilter->cspace == CSpace_stRGB)
        {
            pFilter->procamp = DL_PROCAMP_DISABLED;
        }

        // Count number of procamp operations (limited by number of independent coefficients)
        // Ignore layers with palletized/constant colors
        if (pFilter->procamp != DL_PROCAMP_DISABLED &&
            pFilter->cspace  != CSpace_Any)
        {
            procamp_count++;
        }

        // Set xvYCC passthrough mode
        if (pFilter->cspace == CSpace_xvYCC709 ||
            pFilter->cspace == CSpace_xvYCC601)
        {
            sel_cspace = pFilter->cspace;
        }

        // Get Main Video color space
        if (pFilter->layer == Layer_MainVideo)
        {
            main_cspace = pFilter->cspace;
        }

        // Get Render Target color space
        if (pFilter->layer == Layer_RenderTarget)
        {
            // Target is sRGB/stRGB
            if (!KernelDll_IsYUVFormat(pFilter->format))
            {
                // Disable xvYCC passthrough (sRGB cannot have extended gamut)
                sel_cspace      = CSpace_Any;
            }
            out_cspace = pFilter->cspace;
        }

        // Mark color spaces in use for search that follows
        if (pFilter->cspace > CSpace_Any && pFilter->cspace < CSpace_Count)
        {
            cspace_in_use[pFilter->cspace] = 1;
        }
    }

    // Check max number of procamp operations
    if (procamp_count > DL_PROCAMP_MAX)
    {
        return false;
    }

    //---------------------------------------------------------------//
    // Search Color Space that provides minimum number of CSC conversions
    // If there are multiple solutions, select main video cspace (quality)
    //---------------------------------------------------------------//
    if (sel_cspace == CSpace_Any)
    {
        int cs;
        for (cs = (CSpace_Any + 1); cs < CSpace_Count; cs++)
        {
            // Skip color spaces not in use
            cspace = (VPHAL_CSPACE) cs;
            if (!cspace_in_use[cspace])
            {
                continue;
            }

            // xvYCC and BT are treated as same for CSC considerations (BT.x to xvYCC.x matrix is I)
            cspace = KernelDll_TranslateCspace(cspace);

            // Count # of CS conversions and matrices
            csc_count = 0;
            for (i = iFilterSize, pFilter = pSearchState->Filter; i > 0; i--, pFilter++)
            {
                // Ignore layers where the Color Space may be set in software (colorfill, palletized)
                if (pFilter->cspace == CSpace_Any)
                {
                    continue;
                }

                // Check if CSC/PA is required
                if (KernelDll_TranslateCspace(pFilter->cspace) != cspace ||
                    pFilter->procamp != DL_PROCAMP_DISABLED)
                {
                    csc_count++;
                }
            }

            // Save best choice as requiring minimum number of CSC operations
            if ((sel_csc_count < 0) ||                                     // Initial value
                (csc_count <  sel_csc_count) ||                            // Minimum number of CSC operations
                (csc_count == sel_csc_count && cs == main_cspace) )        // Use main cspace as default if same CSC count
            {
                sel_cspace    = cspace;
                sel_csc_count = csc_count;
            }
        }
    }

    // Due to put the colorfill behind CSC, so Src0 cspace needs to change
    // to selspace in order to fill colorfill values correctly.
    pState->colorfill_cspace = sel_cspace;

    // color space is selected by now... setup CSC matrices
    matrix_count  = 0;
    iCoeffID = 1;
    for (i = iFilterSize, pFilter = pSearchState->Filter; i > 0; i--, pFilter++)
    {
        // Setup CSC for palettized/colorfill layers
        if (pFilter->cspace == CSpace_Any)
        {
            // Set Color Space and format (for software)
            if (pFilter->format == Format_Any)
            {
                pFilter->format = KernelDll_IsCspace(sel_cspace, CSpace_YUV) ? Format_AYUV : Format_A8R8G8B8;
            }

            pFilter->cspace = sel_cspace;
            pFilter->matrix = DL_CSC_DISABLED;
        }
        else
        {
            // Setup CSC parameters: SrcSpace is the layer color space,
            //                       DstSpace is the internal color space selected
            curr_matrix.SrcSpace   = KernelDll_TranslateCspace(pFilter->cspace);
            curr_matrix.DstSpace   = KernelDll_TranslateCspace(sel_cspace);
            curr_matrix.iProcampID = pFilter->procamp;

            // Check if CSC is necessary
            if (curr_matrix.SrcSpace   == curr_matrix.DstSpace &&
                curr_matrix.iProcampID == DL_PROCAMP_DISABLED)
            {
                pFilter->matrix = DL_CSC_DISABLED;
                curr_matrix.iCoeffID = CoeffID_None;
                continue;
            }

            // Reserve CoeffID_0 for main video - CoeffID_0 gets CSC coeff from static parameters
            // If main video doesn't use CoeffID_0, assign to RT
            if ( (pFilter->layer == Layer_MainVideo) ||
                 (pFilter->layer == Layer_RenderTarget))
            {
                if (bCoeffID_0_Used)
                {
                    curr_matrix.iCoeffID = (Kdll_CoeffID)iCoeffID++;
                }
                else
                {
                    curr_matrix.iCoeffID = CoeffID_0;
                    bCoeffID_0_Used = true;
                }
            }
            else
            {
                curr_matrix.iCoeffID = (Kdll_CoeffID)iCoeffID++;
            }

            // CSC at the target layer is from internal cspace (SrcSpace)
            //                              to external cspace (DstCspace)
            if (pFilter->layer == Layer_RenderTarget)
            {
                VPHAL_CSPACE      aux = curr_matrix.SrcSpace;
                curr_matrix.SrcSpace = curr_matrix.DstSpace;
                curr_matrix.DstSpace = aux;
            }

            // Search CSC matrix - avoid duplicated CSC matrices
            for (m = 0; m < matrix_count; m++)
            {
                if (curr_matrix.SrcSpace   == matrix[m].SrcSpace &&
                    curr_matrix.DstSpace   == matrix[m].DstSpace &&
                    curr_matrix.iProcampID == matrix[m].iProcampID)
                {
                    break;
                }
            }

            // Check limit
            if (m == matrix_count)
            {
                // Exceeded number of CSC matrices allowed
                if (matrix_count == DL_CSC_MAX)
                {
                    return false;
                }

                matrix[m].bInUse     = true;
                matrix[m].SrcSpace   = curr_matrix.SrcSpace;
                matrix[m].DstSpace   = curr_matrix.DstSpace;
                matrix[m].iProcampID = curr_matrix.iProcampID;
                matrix[m].iCoeffID   = curr_matrix.iCoeffID;

                // Calculate coefficients for the first time
                KernelDll_UpdateCscCoefficients(pState, &matrix[m]);

                // Next matrix
                matrix_count++;
            }

            // point to the matrix
            pFilter->matrix = m;
        }
    }

    // Link matrices to kernel coefficients (and vice-versa)
    matrix        = pCSC->Matrix;
    for (m = 0; m < matrix_count; m++, matrix++)
    {
        // Coefficient table points to matrix index
        matrixID[matrix->iCoeffID] = (uint8_t)m;
    }

    // Save selected color space
    pCSC->ColorSpace = sel_cspace;

    return true;
}

//---------------------------------------------------------------------------------------
// Kdll_AddKernelList - Add kernel to CM FC kernel list
//
// Parameters:
//    Kdll_KernelCache *pKernelCache     - [in]     Component kernel cache
//    Kdll_KernelCache *pCmFcPatchCache  - [in]     Component kernel patch data cache
//    Kdll_SearchState *pSearchState     - [in/out] Kernel search state
//    Kdll_PatchData   *pKernelPatch     - [in]     Kernel Patch data
//    void             *pPatchDst         - [in]     Patch data Dst address
//    int32_t          iKUID             - [in]     Kernel Unique ID
//    cm_fc_kernel_t   *Cm_Fc_Kernels    - [in/out] CM FC Kernels
//
// Output: true if suceeded, false otherwise
//---------------------------------------------------------------------------------------
bool Kdll_AddKernelList(Kdll_KernelCache *pKernelCache,
                        Kdll_KernelCache *pCmFcPatchCache,
                        Kdll_SearchState *pSearchState,
                        int32_t           iKUID,
                        Kdll_PatchData   *pKernelPatch,
                        void             *pPatchDst,
                        cm_fc_kernel_t   *Cm_Fc_Kernels)
{
    Kdll_State      *pState;
    Kdll_Symbol     *pSymbols;
    Kdll_CacheEntry *kernels;
    Kdll_CacheEntry *pPatch;
    Kdll_LinkData   *link;
    Kdll_LinkData   *liSearch_reloc;
    int             *size;
    int             *left;
    int             dwSize;
    int             i;
    int             base;
    bool            bInline;
    bool            res;

    VPHAL_RENDER_FUNCTION_ENTER;

    res = false;

    // Check if Kernel ID is valid
    if (iKUID >= pKernelCache->iCacheEntries)
    {
        VPHAL_RENDER_NORMALMESSAGE("invalid Kernel ID %d.", iKUID);
        goto finish;
    }

    // Get KDLL state
    pState = pSearchState->pKdllState;

    // Get current combined kernel
    size = &pSearchState->KernelSize;
    left = &pSearchState->KernelLeft;
    pSymbols = &pSearchState->KernelLink;
    base = (*size) >> 2;

    // Find selected kernel/patch and kernel size; check if there is enough space 
    kernels = &pKernelCache->pCacheEntries[iKUID];
    pPatch = &pCmFcPatchCache->pCacheEntries[iKUID];
    dwSize = kernels->iSize;
    if (*left < dwSize)
    {
        VPHAL_RENDER_NORMALMESSAGE("exceeded maximum kernel size.");
        goto finish;
    }

    // Check if there is enough space for symbols
    if (pSymbols->dwCount + kernels->nLink >= pSymbols->dwSize)
    {
        VPHAL_RENDER_NORMALMESSAGE("exceeded maximum numbers of symbols to resolve.");
        goto finish;
    }

#if EMUL || VPHAL_LIB
    VPHAL_RENDER_NORMALMESSAGE("%s.", kernels->szName);

    if (pState->pfnCbListKernel)
    {
        pState->pfnCbListKernel(pState->pToken, kernels->szName);
    }
#elif _DEBUG || _RELEASE_INTERNAL // EMUL || VPHAL_LIB
    VPHAL_RENDER_NORMALMESSAGE("%s.", kernels->szName);
#endif // _DEBUG

    // Append symbols to resolve, relocate symbols
    link = kernels->pLink;
    liSearch_reloc = pSymbols->pLink + pSymbols->dwCount;

    bInline = false;
    if (link)
    {
        for (i = kernels->nLink; i > 0; i--, link++)
        {
            if (link->bInline)
            {
                // Inline code included
                if (!link->bExport)
                {
                    bInline = true;
                }
            }
            else
            {
                *liSearch_reloc = *link;
                liSearch_reloc->dwOffset += base;
                liSearch_reloc++;

                pSymbols->dwCount++;
            }
        }
    }

    *size += dwSize;
    *left -= dwSize;
    Cm_Fc_Kernels->binary_buf = (const char *)kernels->pBinary;
    Cm_Fc_Kernels->binary_size = kernels->iSize;
    Cm_Fc_Kernels->patch_buf = (const char *)pPatch->pBinary;
    Cm_Fc_Kernels->patch_size = pPatch->iSize;
    res = true;

finish:
    return res;
}

//---------------------------------------------------------------------------------------
// KernelDll_BuildKernel_CmFc - Build CM based FC combine Kernel
//
// Parameters: [in/out] pState        - Pointer to Kernel binary file loaded in sys memory
//             [in/out] pSearchState       - Kernel file size
//
// Output: bool
//         TRUE - Successful FALSE - Failed
//-----------------------------------------------------------------------------------------
bool KernelDll_BuildKernel_CmFc(Kdll_State *pState, Kdll_SearchState *pSearchState)
{
    Kdll_KernelCache *pKernelCache = &pState->ComponentKernelCache;
    Kdll_KernelCache *pPatchCache = &pState->CmFcPatchCache;
    Kdll_KernelCache *pCustomCache = pState->pCustomKernelCache;
    bool              res;
    int32_t           offset = 0;
    int32_t           *pKernelID, *pPatchID;
    uint8_t           *pPatchData;
    Kdll_PatchData   *pKernelPatch;
    uint8_t          *kernel = pSearchState->Kernel;
    Kdll_Symbol      *pSymbols = &pSearchState->KernelLink;
    uint32_t          nExports = pKernelCache->nExports;
    Kdll_LinkData    *pExports = pKernelCache->pExports;
    Kdll_LinkData    *pLink;
    int32_t           iOffset;
    uint32_t          dwResolveOffset[DL_MAX_EXPORT_COUNT];
    uint32_t          dwTotalKernelCount;
    uint32_t          dwEstimatedKernelSize;
    int32_t           iKUID;
    bool              bResolveDone;
    int32_t           i;
    cm_fc_kernel_t    Cm_Fc_kernels[DL_MAX_KERNELS];

    VPHAL_RENDER_FUNCTION_ENTER;

    // Disable pop-up box window for STL assertion to avoid VM hang in auto test.
#if (!LINUX)
    ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#if defined(_MSC_VER)
    ::_set_error_mode(_OUT_TO_STDERR);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
#endif

    pSearchState->KernelLink.dwSize = DL_MAX_SYMBOLS;
    pSearchState->KernelLink.dwCount = 0;
    pSearchState->KernelLink.pLink = pSearchState->LinkArray;
    pSearchState->KernelSize = 0;
    pSearchState->KernelLeft = sizeof(pSearchState->Kernel);
    pSearchState->KernelLink.dwCount = 0;

    MOS_ZeroMemory(Cm_Fc_kernels, sizeof(Cm_Fc_kernels));
    dwTotalKernelCount = 0;
    dwEstimatedKernelSize = 0;

#if EMUL || VPHAL_LIB || _DEBUG
    VPHAL_RENDER_NORMALMESSAGE("Component Kernels:");
#endif // EMUL || VPHAL_LIB || _DEBUG

    pKernelID = pSearchState->KernelID;
    pPatchID = pSearchState->PatchID;
    pPatchData = nullptr;

    for (offset = 0; offset < pSearchState->KernelCount; offset++, pKernelID++, pPatchID++, dwTotalKernelCount++)
    {
        // Get patch information associated with the kernel
        pKernelPatch = (*pPatchID >= 0) ? &(pSearchState->Patches[*pPatchID]) : nullptr;

        // Append/Patch kernel from internal cache
        res = Kdll_AddKernelList(pKernelCache, pPatchCache, pSearchState, *pKernelID, pKernelPatch, pPatchData, &Cm_Fc_kernels[dwTotalKernelCount]);

        dwEstimatedKernelSize += Cm_Fc_kernels[dwTotalKernelCount].binary_size;

        if (*pKernelID == IDR_VP_EOT)
        {
            dwTotalKernelCount--;
        }

        if (!res)
        {
            VPHAL_RENDER_NORMALMESSAGE("Failed to build kernel ID %d.", pSearchState->KernelID[offset]);
            res = false;
            goto finish;
        }
    }

    // Resolve kernel dependencies
    MOS_ZeroMemory(dwResolveOffset, sizeof(dwResolveOffset));

    do
    {
        // Update exports
        for (pLink = pSymbols->pLink, i = pSymbols->dwCount; i > 0; i--, pLink++)
        {
            if (pLink->bExport)
            {
                dwResolveOffset[pLink->iLabelID] = pLink->dwOffset;
            }
        }

        bResolveDone = true;
        for (pLink = pSymbols->pLink, i = pSymbols->dwCount; i > 0; i--, pLink++)
        {
            // validate label
            if (pLink->iLabelID > nExports ||              // invalid label
                pExports[pLink->iLabelID].bExport == 0)    // label not in the export table
            {
                VPHAL_RENDER_NORMALMESSAGE("Invalid/unresolved label %d.", pLink->iLabelID);
                res = false;
                goto finish;
            }

            // load dependencies
            if (!pLink->bExport && !dwResolveOffset[pLink->iLabelID])
            {
                // set flag for another pass as newly loaded 
                // kernels may contain dependencies of their own
                bResolveDone = false;

                // Add dependencies to kernel list
                iKUID = pExports[pLink->iLabelID].iKUID;
                res = Kdll_AddKernelList(pKernelCache, pPatchCache, pSearchState, iKUID, nullptr, nullptr, &Cm_Fc_kernels[dwTotalKernelCount]);

                if (!res)
                {
                    VPHAL_RENDER_NORMALMESSAGE("Failed to build kernel ID %d.", pSearchState->KernelID[offset]);
                    res = false;
                    goto finish;
                }

                dwTotalKernelCount++;

                // Restart
                break;
            }
        } // for
    } while (!bResolveDone);

    if (dwEstimatedKernelSize > DL_MAX_KERNEL_SIZE)
    {
        res = false;
        VPHAL_RENDER_NORMALMESSAGE("Kernel size exceeded kdll limitatin.");
        goto finish;
    }

    dwEstimatedKernelSize = DL_MAX_KERNEL_SIZE;

    // Get combine kernel binary from CMFC lib
    if (CM_FC_OK != cm_fc_combine_kernels(dwTotalKernelCount, Cm_Fc_kernels, (char *)pSearchState->Kernel, (size_t *)&dwEstimatedKernelSize, nullptr))
    {
        res = false;
        VPHAL_RENDER_NORMALMESSAGE("cm_fc_combine_kernels() function call failed.");
        goto finish;
    }

    // Get combine kernel binary size from CMFC lib
    pSearchState->KernelSize = dwEstimatedKernelSize;

    res = true;

finish:
    return res;
}

//---------------------------------------------------------------------------------------
// KernelDll_SetupFunctionPointers - Setup Function pointers based on platform
//
// Parameters:
//    KdllState  *pState    - [in/out] Kernel Dll state
//
// Output: true  - Function pointers are set
//         false - Failed to setup function pointers (invalid platform)
//-----------------------------------------------------------------------------------------
bool KernelDll_SetupFunctionPointers_Ext(
    Kdll_State  *pState)
{
    VPHAL_RENDER_FUNCTION_ENTER;

    if (pState->bEnableCMFC)
    {
        pState->pfnBuildKernel      = KernelDll_BuildKernel_CmFc;
    }

    return true;
}

#ifdef __cplusplus
}
#endif // __cplusplus
