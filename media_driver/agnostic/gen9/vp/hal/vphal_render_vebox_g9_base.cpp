/*
* Copyright (c) 2011-2018, Intel Corporation
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
//! \file     vphal_render_vebox_g9_base.cpp
//! \brief    Interface and structure specific for SKL (GEN9) Vebox
//! \details  Interface and structure specific for SKL (GEN9) Vebox
//!
#include "vphal.h"
#include "vphal_render_vebox_base.h"
#include "vphal_render_vebox_g9_base.h"
#include "vphal_render_sfc_g9_base.h"
#include "vphal_render_vebox_util_base.h"
#include "vpkrnheader.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igvpkrn_isa_g9.h"
#endif

#define MAX_INPUT_PREC_BITS         16
#define DOWNSHIFT_WITH_ROUND(x, n)  (((x) + (((n) > 0) ? (1 << ((n) - 1)) : 0)) >> (n))
#define INTERP(x0, x1, x, y0, y1)   ((uint32_t) floor(y0+(x-x0)*(y1-y0)/(double)(x1-x0)))

const char g_KernelDNDI_Str_g9[KERNEL_VEBOX_BASE_MAX][MAX_PATH] =
{
    DBG_TEXT("Reserved"),
    DBG_TEXT("UpdateDNState"),
};

// Kernel Params ---------------------------------------------------------------
const RENDERHAL_KERNEL_PARAM g_Vebox_KernelParam_g9[KERNEL_VEBOX_BASE_MAX] =
{
///*  GRF_Count
//    |  BT_Count
//    |  |    Sampler_Count
//    |  |    |  Thread_Count
//    |  |    |  |                             GRF_Start_Register
//    |  |    |  |                             |   CURBE_Length
//    |  |    |  |                             |   |   block_width
//    |  |    |  |                             |   |   |    block_height
//    |  |    |  |                             |   |   |    |   blocks_x
//    |  |    |  |                             |   |   |    |   |   blocks_y
//    |  |    |  |                             |   |   |    |   |   |*/
    { 0, 0,   0, VPHAL_USE_MEDIA_THREADS_MAX,  0,  0,   0,  0,  0,  0 },    // Reserved
    { 4, 34,  0, VPHAL_USE_MEDIA_THREADS_MAX,  0,  1,  64,  8,  1,  1 },    // UPDATEDNSTATE
};

const uint32_t   dwDenoiseASDThreshold[NOISEFACTOR_MAX + 1] = {
    512, 514, 516, 518, 520, 522, 524, 526, 528, 530, 532, 534, 536, 538, 540, 542,
    544, 546, 548, 550, 552, 554, 556, 558, 560, 562, 564, 566, 568, 570, 572, 574,
    576, 578, 580, 582, 584, 586, 588, 590, 592, 594, 596, 598, 600, 602, 604, 606,
    608, 610, 612, 614, 616, 618, 620, 622, 624, 626, 628, 630, 632, 634, 636, 638,
    640 };

const uint32_t   dwDenoiseHistoryDelta[NOISEFACTOR_MAX + 1] = {
    4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
    5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
    6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
    7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
    8 };

const uint32_t   dwDenoiseMaximumHistory[NOISEFACTOR_MAX + 1] = {
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208 };

const uint32_t   dwDenoiseSTADThreshold[NOISEFACTOR_MAX + 1] = {
    2048, 2052, 2056, 2060, 2064, 2068, 2072, 2076, 2080, 2084, 2088, 2092, 2096, 2100, 2104, 2108,
    2112, 2116, 2120, 2124, 2128, 2132, 2136, 2140, 2144, 2148, 2152, 2156, 2160, 2164, 2168, 2172,
    2176, 2180, 2184, 2188, 2192, 2196, 2200, 2204, 2208, 2212, 2216, 2220, 2224, 2228, 2232, 2236,
    2240, 2244, 2248, 2252, 2256, 2260, 2264, 2268, 2272, 2276, 2280, 2284, 2288, 2292, 2296, 2300,
    2304 };

const uint32_t   dwDenoiseSCMThreshold[NOISEFACTOR_MAX + 1] = {
    512, 514, 516, 518, 520, 522, 524, 526, 528, 530, 532, 534, 536, 538, 540, 542,
    544, 546, 548, 550, 552, 554, 556, 558, 560, 562, 564, 566, 568, 570, 572, 574,
    576, 578, 580, 582, 584, 586, 588, 590, 592, 594, 596, 598, 600, 602, 604, 606,
    608, 610, 612, 614, 616, 618, 620, 622, 624, 626, 628, 630, 632, 634, 636, 638,
    640 };

const uint32_t   dwDenoiseMPThreshold[NOISEFACTOR_MAX + 1] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
    2 };

const uint32_t   dwLTDThreshold[NOISEFACTOR_MAX + 1] = {
    64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
    80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
    96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
    128 };

const uint32_t   dwTDThreshold[NOISEFACTOR_MAX + 1] = {
   128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
   144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
   160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
   176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
   192 };

const uint32_t   dwGoodNeighborThreshold[NOISEFACTOR_MAX + 1] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0 };

const uint32_t   dwPixRangeThreshold0[NOISEFACTOR_MAX + 1] = {
    32,  37,  42,  47,  52,  57,  62,  67,  72,  77,  82,  87,  92,  97, 102, 107,
    112, 117, 122, 127, 132, 137, 142, 147, 152, 157, 162, 167, 172, 177, 182, 187,
    192, 198, 204, 210, 216, 222, 228, 234, 240, 246, 252, 258, 264, 270, 276, 282,
    288, 294, 300, 306, 312, 318, 324, 330, 336, 342, 348, 354, 360, 366, 372, 378,
    384 };

const uint32_t   dwPixRangeThreshold1[NOISEFACTOR_MAX + 1] = {
    64,  70,  76,  82,  88,  94, 100, 106, 112, 118, 124, 130, 136, 142, 148, 154,
    160, 166, 172, 178, 184, 190, 196, 202, 208, 214, 220, 226, 232, 238, 244, 250,
    256, 266, 276, 286, 296, 306, 316, 326, 336, 346, 356, 366, 376, 386, 396, 406,
    416, 426, 436, 446, 456, 466, 476, 486, 496, 506, 516, 526, 536, 546, 556, 566,
    576 };

const uint32_t   dwPixRangeThreshold2[NOISEFACTOR_MAX + 1] = {
    128, 140, 152, 164, 176, 188, 200, 212, 224, 236, 248, 260, 272, 284, 296, 308,
    320, 332, 344, 356, 368, 380, 392, 404, 416, 428, 440, 452, 464, 476, 488, 500,
    512, 524, 536, 548, 560, 572, 584, 596, 608, 620, 632, 644, 656, 668, 680, 692,
    704, 716, 728, 740, 752, 764, 776, 788, 800, 812, 824, 836, 848, 860, 872, 884,
    896 };

const uint32_t   dwPixRangeThreshold3[NOISEFACTOR_MAX + 1] = {
    128, 144, 160, 176, 192, 208, 224, 240, 256, 272, 288, 304, 320, 336, 352, 368,
    384, 400, 416, 432, 448, 464, 480, 496, 512, 528, 544, 560, 576, 592, 608, 624,
    640, 660, 680, 700, 720, 740, 760, 780, 800, 820, 840, 860, 880, 900, 920, 940,
    960, 980, 1000, 1020, 1040, 1060, 1080, 1100, 1120, 1140, 1160, 1180, 1200, 1220, 1240, 1260,
    1280 };

const uint32_t   dwPixRangeThreshold4[NOISEFACTOR_MAX + 1] = {
    128, 152, 176, 200, 224, 248, 272, 296, 320, 344, 368, 392, 416, 440, 464, 488,
    512, 536, 560, 584, 608, 632, 656, 680, 704, 728, 752, 776, 800, 824, 848, 872,
    896, 928, 960, 992, 1024, 1056, 1088, 1120, 1152, 1184, 1216, 1248, 1280, 1312, 1344, 1376,
    1408, 1440, 1472, 1504, 1536, 1568, 1600, 1632, 1664, 1696, 1728, 1760, 1792, 1824, 1856, 1888,
    1920 };

const uint32_t   dwPixRangeThreshold5[NOISEFACTOR_MAX + 1] = {
    128, 164, 200, 236, 272, 308, 344, 380, 416, 452, 488, 524, 560, 596, 632, 668,
    704, 740, 776, 812, 848, 884, 920, 956, 992, 1028, 1064, 1100, 1136, 1172, 1208, 1244,
    1280, 1320, 1360, 1400, 1440, 1480, 1520, 1560, 1600, 1640, 1680, 1720, 1760, 1800, 1840, 1880,
    1920, 1960, 2000, 2040, 2080, 2120, 2160, 2200, 2240, 2280, 2320, 2360, 2400, 2440, 2480, 2520,
    2560 };

const uint32_t   dwPixRangeWeight0[NOISEFACTOR_MAX + 1] = {
    16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
    16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
    16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
    16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
    16 };

const uint32_t   dwPixRangeWeight1[NOISEFACTOR_MAX + 1] = {
    9,   9,   9,   9,   9,   9,   9,  10,  10,  10,  10,  10,  10,  11,  11,  11,
    11,  11,  11,  11,  12,  12,  12,  12,  12,  12,  13,  13,  13,  13,  13,  13,
    14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
    14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
    15 };

const uint32_t   dwPixRangeWeight2[NOISEFACTOR_MAX + 1] = {
    2,   2,   2,   2,   3,   3,   3,   3,   4,   4,   4,   4,   5,   5,   5,   5,
    6,   6,   6,   6,   7,   7,   7,   7,   8,   8,   8,   8,   9,   9,   9,   9,
    10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  11,  11,  11,  11,  11,
    11,  11,  11,  11,  11,  11,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
    13 };

const uint32_t   dwPixRangeWeight3[NOISEFACTOR_MAX + 1] = {
    0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   2,   2,   2,
    2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   4,
    5,   5,   5,   5,   5,   5,   5,   6,   6,   6,   6,   6,   6,   7,   7,   7,
    7,   7,   7,   7,   8,   8,   8,   8,   8,   8,   9,   9,   9,   9,   9,   9,
    10 };

const uint32_t   dwPixRangeWeight4[NOISEFACTOR_MAX + 1] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
    2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   4,   4,   4,
    4,   4,   4,   4,   5,   5,   5,   5,   5,   5,   6,   6,   6,   6,   6,   6,
    7 };

const uint32_t   dwPixRangeWeight5[NOISEFACTOR_MAX + 1] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,
    2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
    4 };

const uint32_t   dwLTDThresholdUV[NOISEFACTOR_MAX + 1] = {
    4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
    5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
    6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
    7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
    8 };

const uint32_t   dwTDThresholdUV[NOISEFACTOR_MAX + 1] = {
    10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
    11,  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
    12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
    13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
    14 };

const uint32_t   dwSTADThresholdUV[NOISEFACTOR_MAX + 1] = {
    128, 128, 128, 128, 129, 129, 129, 129, 130, 130, 130, 130, 131, 131, 131, 131,
    132, 132, 132, 132, 133, 133, 133, 133, 134, 134, 134, 134, 135, 135, 135, 135,
    136, 136, 136, 136, 137, 137, 137, 137, 138, 138, 138, 138, 139, 139, 139, 139,
    140, 140, 140, 140, 141, 141, 141, 141, 142, 142, 142, 142, 143, 143, 143, 143,
    144 };

//!
//! \brief    IsFormatMMCSupported
//! \details  Check if the format of vebox output surface is supported by MMC
//! \param    [in] Format
//! \return   bool  true if suported, otherwise not supported
//!
bool VPHAL_VEBOX_STATE_G9_BASE::IsFormatMMCSupported(
    MOS_FORMAT                  Format)
{
    bool    bRet;

    bRet = false;

    if ((Format != Format_NV12)     &&
        (Format != Format_YUY2)     &&
        (Format != Format_YUYV)     &&
        (Format != Format_UYVY)     &&
        (Format != Format_YVYU)     &&
        (Format != Format_VYUY)     &&
        (Format != Format_AYUV)     &&
        (Format != Format_Y416)     &&
        (Format != Format_A8B8G8R8) &&
        (Format != Format_A16B16G16R16))
    {
        VPHAL_RENDER_NORMALMESSAGE("Unsupported Format '0x%08x' for VEBOX MMC ouput.", Format);
        goto finish;
    }

    bRet = true;

finish:
    return bRet;
}

MOS_STATUS VPHAL_VEBOX_STATE_G9_BASE::GetFFDISurfParams(
    VPHAL_CSPACE        &ColorSpace,
    VPHAL_SAMPLE_TYPE   &SampleType)
{
    PVPHAL_VEBOX_RENDER_DATA pRenderData = GetLastExecRenderData();

    if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData))
    {
        ColorSpace = m_sfcPipeState->GetInputColorSpace();
    }
    else
    {
        ColorSpace = m_currentSurface->ColorSpace;
    }

    // When IECP is enabled and Bob or interlaced scaling is selected for interlaced input,
    // output surface's SampleType should be same to input's. Bob is being
    // done in Composition part
    if (pRenderData->bIECP &&
        (m_currentSurface->pDeinterlaceParams                         &&
         m_currentSurface->pDeinterlaceParams->DIMode == DI_MODE_BOB) ||
         m_currentSurface->bInterlacedScaling)
    {
        SampleType = m_currentSurface->SampleType;
    }
    else
    {
        SampleType = SAMPLE_PROGRESSIVE;
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Get Output surface params needed when allocate surfaces
//! \details  Get Output surface params needed when allocate surfaces
//! \param    [out] Format
//!           Format of output surface
//! \param    [out] TileType
//!           Tile type of output surface
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if success, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G9_BASE::GetOutputSurfParams(
    MOS_FORMAT          &Format,
    MOS_TILE_TYPE       &TileType)
{
    PVPHAL_VEBOX_RENDER_DATA      pRenderData = GetLastExecRenderData();

    if (pRenderData->bDeinterlace)
    {
        Format   = Format_YUY2;
        TileType = MOS_TILE_Y;
    }
    else
    {
        Format  = IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData) ?
                    m_sfcPipeState->GetInputFormat() :
                    m_currentSurface->Format;

        TileType = m_currentSurface->TileType;
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Check for DN only case
//! \details  Check for DN only case
//! \return   bool
//!           Return true if DN only case, otherwise not
//!
bool VPHAL_VEBOX_STATE_G9_BASE::IsDNOnly()
{
    PVPHAL_VEBOX_RENDER_DATA pRenderData = GetLastExecRenderData();

    return pRenderData->bDenoise &&
           (!pRenderData->bDeinterlace) &&
           (!IsQueryVarianceEnabled()) &&
           (!IsIECPEnabled());
}

bool VPHAL_VEBOX_STATE_G9_BASE::IsFFDISurfNeeded()
{
    PVPHAL_VEBOX_RENDER_DATA pRenderData = GetLastExecRenderData();

    if (pRenderData->bDeinterlace ||
        IsQueryVarianceEnabled()  ||
        pRenderData->bIECP        ||
        (pRenderData->bDenoise && IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData)))  // DN + SFC needs IECP implicitly and outputs to DI surface
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool VPHAL_VEBOX_STATE_G9_BASE::IsFFDNSurfNeeded()
{
    return GetLastExecRenderData()->bDenoise ? true : false;
}

bool VPHAL_VEBOX_STATE_G9_BASE::IsSTMMSurfNeeded()
{

    return (GetLastExecRenderData()->bDenoise || GetLastExecRenderData()->bDeinterlace);
}

//!
//! \brief    Vebox allocate resources
//! \details  Allocate resources that will be used in Vebox
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G9_BASE::AllocateResources()
{
    MOS_STATUS             eStatus;
    PMOS_INTERFACE         pOsInterface;
    PRENDERHAL_INTERFACE   pRenderHal;
    MOS_FORMAT             format;
    MOS_TILE_TYPE          TileType;
    uint32_t               dwWidth;
    uint32_t               dwHeight;
    uint32_t               dwSize;
    int32_t                i;
    bool                   bAllocated;
    bool                   bDIEnable;
    bool                   bSurfCompressed;
    bool                   bFFDNSurfCompressed;
    MOS_RESOURCE_MMC_MODE  SurfCompressionMode;
    MOS_RESOURCE_MMC_MODE  FFDNSurfCompressionMode;
    MHW_VEBOX_SURFACE_PARAMS      MhwVeboxSurfaceParam;
    PMHW_VEBOX_INTERFACE          pVeboxInterface;
    PVPHAL_VEBOX_STATE_G9_BASE    pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA      pRenderData = GetLastExecRenderData();

    bAllocated              = false;
    bSurfCompressed         = false;
    bFFDNSurfCompressed     = false;
    SurfCompressionMode     = MOS_MMC_DISABLED;
    FFDNSurfCompressionMode = MOS_MMC_DISABLED;
    pOsInterface            = pVeboxState->m_pOsInterface;
    pRenderHal              = pVeboxState->m_pRenderHal;
    pVeboxInterface         = pVeboxState->m_pVeboxInterface;

    GetOutputSurfParams(format, TileType);

    // In DN only case, input, output and previous De-noised
    // surfaces all should have precisely the same memory compression status.
    // Either all these surfaces should be compressed together
    // or none of them compressed at all.This is HW limitation.
    if (IsDNOnly())
    {
        bSurfCompressed     = pVeboxState->m_currentSurface->bCompressible;
        SurfCompressionMode = pVeboxState->m_currentSurface->bIsCompressed ? MOS_MMC_HORIZONTAL : MOS_MMC_DISABLED;
    }
    // Only Tiled Y surfaces support MMC
    else if (pVeboxState->bEnableMMC   &&
             (TileType == MOS_TILE_Y) &&
             pVeboxState->IsFormatMMCSupported(format))
    {
        bSurfCompressed     = true;
        SurfCompressionMode = MOS_MMC_HORIZONTAL;
    }

    // Allocate FFDI/IECP surfaces----------------------------------------------
    if (IsFFDISurfNeeded())
    {
        VPHAL_CSPACE        ColorSpace;
        VPHAL_SAMPLE_TYPE   SampleType;

        GetFFDISurfParams(ColorSpace, SampleType);

        for (i = 0; i < pVeboxState->iNumFFDISurfaces; i++)
        {
            VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
                    pOsInterface,
                    pVeboxState->FFDISurfaces[i],
                    "VeboxFFDISurface_g9",
                    format,
                    MOS_GFXRES_2D,
                    TileType,
                    pVeboxState->m_currentSurface->dwWidth,
                    pVeboxState->m_currentSurface->dwHeight,
                    bSurfCompressed,
                    SurfCompressionMode,
                    &bAllocated));

            pVeboxState->FFDISurfaces[i]->SampleType = SampleType;

            // Copy rect sizes so that if input surface state needs to adjust,
            // output surface can be adjustted also.
            pVeboxState->FFDISurfaces[i]->rcSrc    = pVeboxState->m_currentSurface->rcSrc;
            pVeboxState->FFDISurfaces[i]->rcDst    = pVeboxState->m_currentSurface->rcDst;
            // Copy max src rect
            pVeboxState->FFDISurfaces[i]->rcMaxSrc = pVeboxState->m_currentSurface->rcMaxSrc;

            // Copy Rotation, it's used in setting SFC state
            pVeboxState->FFDISurfaces[i]->Rotation = pVeboxState->m_currentSurface->Rotation;

            pVeboxState->FFDISurfaces[i]->ColorSpace = ColorSpace;

            if (bAllocated)
            {
                // Report Compress Status
                m_reporting->FFDICompressible = bSurfCompressed;
                m_reporting->FFDICompressMode = (uint8_t)(SurfCompressionMode);
            }
        }
    }

    // When DI switch to DNDI, the first FFDN surface pitch doesn't match with
    // the input surface pitch and cause the flicker issue
    // Or for 2 clip playback in WMP, the first one is HW decoding, the second one is SW decoding,
    // when the second clip playback starting without media pipeline recreation,
    // the internal FFDNSurfaces are compressed, but VP input surface is uncompressed.
    if ((pVeboxState->bDIEnabled && !pVeboxState->bDNEnabled && pRenderData->bDenoise) ||
        ((pVeboxState->m_currentSurface->bIsCompressed == false) && ((bSurfCompressed == true) || (pVeboxState->FFDNSurfaces[0]->bIsCompressed == true))))
    {
        bFFDNSurfCompressed     = pVeboxState->m_currentSurface->bCompressible;
        FFDNSurfCompressionMode = pVeboxState->m_currentSurface->bIsCompressed ? MOS_MMC_HORIZONTAL : MOS_MMC_DISABLED;
    }
    else
    {
        bFFDNSurfCompressed     = bSurfCompressed;
        FFDNSurfCompressionMode = SurfCompressionMode;
    }

    // Allocate FFDN surfaces---------------------------------------------------
    if (IsFFDNSurfNeeded())
    {
        for (i = 0; i < VPHAL_NUM_FFDN_SURFACES; i++)
        {
            VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
                    pOsInterface,
                    pVeboxState->FFDNSurfaces[i],
                    "VeboxFFDNSurface_g9",
                    pVeboxState->m_currentSurface->Format,
                    MOS_GFXRES_2D,
                    pVeboxState->m_currentSurface->TileType,
                    pVeboxState->m_currentSurface->dwWidth,
                    pVeboxState->m_currentSurface->dwHeight,
                    bFFDNSurfCompressed,
                    FFDNSurfCompressionMode,
                    &bAllocated));

            // if allocated, pVeboxState->PreviousSurface is not valid for DN reference.
            if (bAllocated)
            {
                // If DI is enabled, try to use app's reference if provided
                if (pRenderData->bRefValid                         &&
                    pRenderData->bDeinterlace                      &&
                    (pVeboxState->m_currentSurface->pBwdRef  != nullptr) &&
                    (pVeboxState->FFDNSurfaces[i]->dwPitch == pVeboxState->m_currentSurface->pBwdRef->dwPitch))
                {
                    CopySurfaceValue(pVeboxState->m_previousSurface, pVeboxState->m_currentSurface->pBwdRef);
                }
                else
                {
                    pRenderData->bRefValid = false;
                }
            }

            // DN's output format should be same to input
            pVeboxState->FFDNSurfaces[i]->SampleType =
                pVeboxState->m_currentSurface->SampleType;

            // Copy rect sizes so that if input surface state needs to adjust,
            // output surface can be adjustted also.
            pVeboxState->FFDNSurfaces[i]->rcSrc    = pVeboxState->m_currentSurface->rcSrc;
            pVeboxState->FFDNSurfaces[i]->rcDst    = pVeboxState->m_currentSurface->rcDst;
            // Copy max src rect
            pVeboxState->FFDNSurfaces[i]->rcMaxSrc = pVeboxState->m_currentSurface->rcMaxSrc;

            // Set Colorspace of FFDN
            pVeboxState->FFDNSurfaces[i]->ColorSpace = pVeboxState->m_currentSurface->ColorSpace;

            // Copy FrameID and parameters, as DN output will be used as next blt's current
            pVeboxState->FFDNSurfaces[i]->FrameID            = pVeboxState->m_currentSurface->FrameID;
            pVeboxState->FFDNSurfaces[i]->pDenoiseParams     = pVeboxState->m_currentSurface->pDenoiseParams;

            if (bAllocated)
            {
                // Report Compress Status
                m_reporting->FFDNCompressible = bFFDNSurfCompressed;
                m_reporting->FFDNCompressMode = (uint8_t)(FFDNSurfCompressionMode);
            }
        }
    }

    // Adjust the rcMaxSrc of pRenderTarget when Vebox output is enabled
    if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))
    {
        pRenderData->pRenderTarget->rcMaxSrc = pVeboxState->m_currentSurface->rcMaxSrc;
    }

    // Allocate STMM (Spatial-Temporal Motion Measure) Surfaces------------------
    if (IsSTMMSurfNeeded())
    {
        if (pVeboxState->bEnableMMC)
        {
            bSurfCompressed     = true;
            SurfCompressionMode = MOS_MMC_HORIZONTAL;
        }
        else
        {
            bSurfCompressed     = false;
            SurfCompressionMode = MOS_MMC_DISABLED;
        }

        for (i = 0; i < VPHAL_NUM_STMM_SURFACES; i++)
        {
            VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
                pOsInterface,
                &pVeboxState->STMMSurfaces[i],
                "VeboxSTMMSurface_g9",
                Format_STMM,
                MOS_GFXRES_2D,
                MOS_TILE_Y,
                pVeboxState->m_currentSurface->dwWidth,
                pVeboxState->m_currentSurface->dwHeight,
                bSurfCompressed,
                SurfCompressionMode,
                &bAllocated));

            if (bAllocated)
            {
                VPHAL_RENDER_CHK_STATUS(VeboxInitSTMMHistory(i));

                // Report Compress Status
                m_reporting->STMMCompressible = bSurfCompressed;
                m_reporting->STMMCompressMode = (uint8_t)(SurfCompressionMode);
            }
        }
    }

    // Allocate Statistics State Surface----------------------------------------
    // Width to be a aligned on 64 bytes and height is 1/4 the height
    // Per frame information written twice per frame for 2 slices
    // Surface to be a rectangle aligned with dwWidth to get proper dwSize
    bDIEnable  = pRenderData->bDeinterlace || IsQueryVarianceEnabled();

    VPHAL_RENDER_CHK_STATUS(VpHal_InitVeboxSurfaceParams(
                            pVeboxState->m_currentSurface, &MhwVeboxSurfaceParam));
    VPHAL_RENDER_CHK_STATUS(pVeboxInterface->VeboxAdjustBoundary(
        &MhwVeboxSurfaceParam,
        &dwWidth,
        &dwHeight,
        bDIEnable));

    dwWidth     = MOS_ALIGN_CEIL(dwWidth, 64);
    dwHeight    = MOS_ROUNDUP_DIVIDE(dwHeight, 4) +
                  MOS_ROUNDUP_DIVIDE(VPHAL_VEBOX_STATISTICS_SIZE_G9 * sizeof(uint32_t), dwWidth);
    dwSize      = dwWidth * dwHeight;

    VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
                pOsInterface,
                &pVeboxState->VeboxStatisticsSurface,
                "VeboxStatisticsSurface_g9",
                Format_Buffer,
                MOS_GFXRES_BUFFER,
                MOS_TILE_LINEAR,
                dwSize,
                1,
                false,
                MOS_MMC_DISABLED,
                &bAllocated));

    if (bAllocated)
    {
        // initialize Statistics Surface
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnFillResource(
                    pOsInterface,
                    &(pVeboxState->VeboxStatisticsSurface.OsResource),
                    dwSize,
                    0));

        pVeboxState->dwVeboxPerBlockStatisticsWidth  = dwWidth;
        pVeboxState->dwVeboxPerBlockStatisticsHeight = dwHeight -
            MOS_ROUNDUP_DIVIDE(VPHAL_VEBOX_STATISTICS_SIZE_G9 * sizeof(uint32_t), dwWidth);
    }

#if VEBOX_AUTO_DENOISE_SUPPORTED
    // Allocate Temp Surface for Vebox Update kernels----------------------------------------
    // the surface size is one Page
    dwSize      = MHW_PAGE_SIZE;

    VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
                pOsInterface,
                &pVeboxState->VeboxTempSurface,
                "VeboxTempSurface_g9",
                Format_Buffer,
                MOS_GFXRES_BUFFER,
                MOS_TILE_LINEAR,
                dwSize,
                1,
                false,
                MOS_MMC_DISABLED,
                &bAllocated));

    if (bAllocated)
    {
        // initialize Statistics Surface
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnFillResource(
                    pOsInterface,
                    &(pVeboxState->VeboxTempSurface.OsResource),
                    dwSize,
                    0));
    }

    // Allocate Spatial Attributes Configuration Surface for DN kernel Gen9+-----------
    dwSize      = MHW_PAGE_SIZE;

    VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
        pOsInterface,
        &pVeboxState->VeboxSpatialAttributesConfigurationSurface,
        "VeboxSpatialAttributesConfigurationSurface_g9",
        Format_RAW,
        MOS_GFXRES_BUFFER,
        MOS_TILE_LINEAR,
        dwSize,
        1,
        false,
        MOS_MMC_DISABLED,
        &bAllocated));

    if (bAllocated)
    {
        // initialize Spatial Attributes Configuration Surface
        VPHAL_RENDER_CHK_STATUS(VeboxInitSpatialAttributesConfiguration());
    }
#endif

finish:
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        pVeboxState->FreeResources();
    }

    return eStatus;
}

//!
//! \brief    Vebox free resources
//! \details  Free resources that are used in Vebox
//! \return   void
//!
void VPHAL_VEBOX_STATE_G9_BASE::FreeResources()
{
    PVPHAL_VEBOX_STATE_G9_BASE   pVeboxState = this;
    int32_t i;
    PMOS_INTERFACE       pOsInterface = pVeboxState->m_pOsInterface;

    // Free FFDI surfaces
    for (i = 0; i < pVeboxState->iNumFFDISurfaces; i++)
    {
        if (pVeboxState->FFDISurfaces[i])
        {
            pOsInterface->pfnFreeResource(
                pOsInterface,
                &pVeboxState->FFDISurfaces[i]->OsResource);
        }
    }

    // Free FFDN surfaces
    for (i = 0; i < VPHAL_NUM_FFDN_SURFACES; i++)
    {
        if (pVeboxState->FFDNSurfaces[i])
        {
            pOsInterface->pfnFreeResource(
                pOsInterface,
                &pVeboxState->FFDNSurfaces[i]->OsResource);
        }
    }

    // Free DI history buffers (STMM = Spatial-temporal motion measure)
    for (i = 0; i < VPHAL_NUM_STMM_SURFACES; i++)
    {
        pOsInterface->pfnFreeResource(
            pOsInterface,
            &pVeboxState->STMMSurfaces[i].OsResource);
    }

    // Free Statistics data surface for VEBOX
    pOsInterface->pfnFreeResource(
        pOsInterface,
        &pVeboxState->VeboxStatisticsSurface.OsResource);

#if VEBOX_AUTO_DENOISE_SUPPORTED
    // Free Spatial Attributes Configuration Surface for DN kernel
    pOsInterface->pfnFreeResource(
        pOsInterface,
        &pVeboxState->VeboxSpatialAttributesConfigurationSurface.OsResource);

    // Free Temp Surface for VEBOX
    pOsInterface->pfnFreeResource(
        pOsInterface,
        &pVeboxState->VeboxTempSurface.OsResource);
#endif

    // Free SFC resources
    if (MEDIA_IS_SKU(pVeboxState->m_pSkuTable, FtrSFCPipe) &&
        m_sfcPipeState)
    {
        m_sfcPipeState->FreeResources();
    }

}

//!
//! \brief    Setup Vebox_DI_IECP Command params for VEBOX final output surface on G9
//! \details  Setup Vebox_DI_IECP Command params for VEBOX final output surface on G9
//! \param    [in] bDiScdEnable
//!           Is DI/Variances report enabled
//! \param    [in,out] pVeboxDiIecpCmdParams
//!           Pointer to VEBOX_DI_IECP command parameters
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G9_BASE::SetupDiIecpStateForOutputSurf(
    bool                                    bDiScdEnable,
    PMHW_VEBOX_DI_IECP_CMD_PARAMS           pVeboxDiIecpCmdParams)
{
    PMOS_INTERFACE                pOsInterface;
    PRENDERHAL_INTERFACE          pRenderHal;
    PMHW_VEBOX_INTERFACE          pVeboxInterface;
    PVPHAL_VEBOX_STATE_G9_BASE    pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA      pRenderData = GetLastExecRenderData();
    MHW_VEBOX_SURFACE_CNTL_PARAMS VeboxSurfCntlParams;
    PVPHAL_SURFACE                pSurface;
    MOS_STATUS                    eStatus     = MOS_STATUS_SUCCESS;

    pOsInterface    = pVeboxState->m_pOsInterface;
    pRenderHal      = pVeboxState->m_pRenderHal;
    pVeboxInterface = pVeboxState->m_pVeboxInterface;

    // VEBOX final output surface
    if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))
    {
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
                pOsInterface,
                &pRenderData->pRenderTarget->OsResource,
                true,
                true));

        pVeboxDiIecpCmdParams->pOsResCurrOutput   =
            &pRenderData->pRenderTarget->OsResource;
        pVeboxDiIecpCmdParams->dwCurrOutputSurfOffset =
            pRenderData->pRenderTarget->dwOffset;
        pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value =
            pVeboxState->DnDiSurfMemObjCtl.CurrentOutputSurfMemObjCtl;

        if (IsFormatMMCSupported(pRenderData->pRenderTarget->Format) &&
            (pRenderData->Component                      == COMPONENT_VPreP)     &&
            (pRenderData->pRenderTarget->CompressionMode == MOS_MMC_HORIZONTAL))
        {
            // Update control bits for Current Output Surf
            pSurface = pRenderData->pRenderTarget;
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed       = pSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode     = pSurface->CompressionMode;
            VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value)));
        }
    }
    else if (bDiScdEnable)
    {
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
            pOsInterface,
            &pVeboxState->FFDISurfaces[pRenderData->iFrame1]->OsResource,
            true,
            true));

        pVeboxDiIecpCmdParams->pOsResCurrOutput   =
            &pVeboxState->FFDISurfaces[pRenderData->iFrame1]->OsResource;
        pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value =
            pVeboxState->DnDiSurfMemObjCtl.CurrentOutputSurfMemObjCtl;

        // Update control bits for Current Output Surf
        pSurface = pVeboxState->FFDISurfaces[pRenderData->iFrame1];
        MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
        VeboxSurfCntlParams.bIsCompressed       = pSurface->bIsCompressed;
        VeboxSurfCntlParams.CompressionMode     = pSurface->CompressionMode;
        VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxSurfaceControlBits(
            &VeboxSurfCntlParams,
            (uint32_t *)&(pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value)));

        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
            pOsInterface,
            &pVeboxState->FFDISurfaces[pRenderData->iFrame0]->OsResource,
            true,
            true));

        pVeboxDiIecpCmdParams->pOsResPrevOutput   =
            &pVeboxState->FFDISurfaces[pRenderData->iFrame0]->OsResource;
        pVeboxDiIecpCmdParams->PrevOutputSurfCtrl.Value =
            pVeboxState->DnDiSurfMemObjCtl.CurrentOutputSurfMemObjCtl;

        // Update control bits for PrevOutput surface
        pSurface = pVeboxState->FFDISurfaces[pRenderData->iFrame0];
        MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
        VeboxSurfCntlParams.bIsCompressed       = pSurface->bIsCompressed;
        VeboxSurfCntlParams.CompressionMode     = pSurface->CompressionMode;
        VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxSurfaceControlBits(
            &VeboxSurfCntlParams,
            (uint32_t *)&(pVeboxDiIecpCmdParams->PrevOutputSurfCtrl.Value)));
    }
    else if (IsIECPEnabled()) // IECP output surface without DI
    {
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
                pOsInterface,
                &pVeboxState->FFDISurfaces[pRenderData->iCurDNOut]->OsResource,
                true,
                true));

        pVeboxDiIecpCmdParams->pOsResCurrOutput   =
            &pVeboxState->FFDISurfaces[pRenderData->iCurDNOut]->OsResource;
        pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value =
            pVeboxState->DnDiSurfMemObjCtl.CurrentOutputSurfMemObjCtl;

        // Update control bits for CurrOutputSurf surface
        pSurface = pVeboxState->FFDISurfaces[pRenderData->iCurDNOut];
        MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
        VeboxSurfCntlParams.bIsCompressed       = pSurface->bIsCompressed;
        VeboxSurfCntlParams.CompressionMode     = pSurface->CompressionMode;
        VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxSurfaceControlBits(
            &VeboxSurfCntlParams,
            (uint32_t *)&(pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value)));
    }

finish:
    return eStatus;
}

//!
//! \brief    Setup Vebox_DI_IECP Command params for Gen9
//! \details  Setup Vebox_DI_IECP Command params for Gen9
//! \param    [in] bDiScdEnable
//!           Is DI/Variances report enabled
//! \param    [in,out] pVeboxDiIecpCmdParams
//!           Pointer to VEBOX_DI_IECP command parameters
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G9_BASE::SetupDiIecpState(
    bool                                bDiScdEnable,
    PMHW_VEBOX_DI_IECP_CMD_PARAMS       pVeboxDiIecpCmdParams)
{
    PMOS_INTERFACE                      pOsInterface;
    PRENDERHAL_INTERFACE                pRenderHal;
    uint32_t                            dwWidth;
    uint32_t                            dwHeight;
    bool                                bDIEnable;
    MOS_STATUS                          eStatus;
    MHW_VEBOX_SURFACE_PARAMS            MhwVeboxSurfaceParam;
    PMHW_VEBOX_INTERFACE                pVeboxInterface;
    MHW_VEBOX_SURFACE_CNTL_PARAMS       VeboxSurfCntlParams;
    PVPHAL_SURFACE                      pSurface;
    PVPHAL_VEBOX_STATE_G9_BASE          pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA            pRenderData = GetLastExecRenderData();

    pOsInterface    = pVeboxState->m_pOsInterface;
    pRenderHal      = pVeboxState->m_pRenderHal;
    pVeboxInterface = pVeboxState->m_pVeboxInterface;
    MOS_ZeroMemory(pVeboxDiIecpCmdParams, sizeof(*pVeboxDiIecpCmdParams));

    // Align dwEndingX with surface state
    bDIEnable  = pRenderData->bDeinterlace || IsQueryVarianceEnabled();
    VPHAL_RENDER_CHK_STATUS(VpHal_InitVeboxSurfaceParams(
                            pVeboxState->m_currentSurface, &MhwVeboxSurfaceParam));
    VPHAL_RENDER_CHK_STATUS(pVeboxInterface->VeboxAdjustBoundary(
        &MhwVeboxSurfaceParam,
        &dwWidth,
        &dwHeight,
        bDIEnable));

    pVeboxDiIecpCmdParams->dwStartingX = 0;
    pVeboxDiIecpCmdParams->dwEndingX   = dwWidth - 1;

    // Input Surface
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
        pOsInterface,
        &pVeboxState->m_currentSurface->OsResource,
        false,
        true));

    pVeboxDiIecpCmdParams->pOsResCurrInput          =
        &pVeboxState->m_currentSurface->OsResource;
    pVeboxDiIecpCmdParams->dwCurrInputSurfOffset    =
        pVeboxState->m_currentSurface->dwOffset;
    pVeboxDiIecpCmdParams->CurrInputSurfCtrl.Value  =
        pVeboxState->DnDiSurfMemObjCtl.CurrentInputSurfMemObjCtl;

    // Update control bits for current surface
    pSurface = pVeboxState->m_currentSurface;
    MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
    VeboxSurfCntlParams.bIsCompressed       = pSurface->bIsCompressed;
    VeboxSurfCntlParams.CompressionMode     = pSurface->CompressionMode;
    VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxSurfaceControlBits(
        &VeboxSurfCntlParams,
        (uint32_t *)&(pVeboxDiIecpCmdParams->CurrInputSurfCtrl.Value)));

    // Reference surface
    if (pRenderData->bRefValid)
    {
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
            pOsInterface,
            &pVeboxState->m_previousSurface->OsResource,
            false,
            true));

        pVeboxDiIecpCmdParams->pOsResPrevInput          =
            &pVeboxState->m_previousSurface->OsResource;
        pVeboxDiIecpCmdParams->dwPrevInputSurfOffset    =
            pVeboxState->m_previousSurface->dwOffset;
        pVeboxDiIecpCmdParams->PrevInputSurfCtrl.Value  =
            pVeboxState->DnDiSurfMemObjCtl.PreviousInputSurfMemObjCtl;

        // Update control bits for PreviousSurface surface
        pSurface = pVeboxState->m_previousSurface;
        MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
        VeboxSurfCntlParams.bIsCompressed       = pSurface->bIsCompressed;
        VeboxSurfCntlParams.CompressionMode     = pSurface->CompressionMode;
        VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxSurfaceControlBits(
            &VeboxSurfCntlParams,
            (uint32_t *)&(pVeboxDiIecpCmdParams->PrevInputSurfCtrl.Value)));
    }

    // VEBOX final output surface
    VPHAL_RENDER_CHK_STATUS(SetupDiIecpStateForOutputSurf(bDiScdEnable, pVeboxDiIecpCmdParams));

    // DN intermediate output surface
    if (IsFFDNSurfNeeded())
    {
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
            pOsInterface,
            &pVeboxState->FFDNSurfaces[pRenderData->iCurDNOut]->OsResource,
            true,
            true));

        pVeboxDiIecpCmdParams->pOsResDenoisedCurrOutput   =
            &pVeboxState->FFDNSurfaces[pRenderData->iCurDNOut]->OsResource;
        pVeboxDiIecpCmdParams->DenoisedCurrOutputSurfCtrl.Value =
            pVeboxState->DnDiSurfMemObjCtl.DnOutSurfMemObjCtl;

        // Update control bits for DenoisedCurrOutputSurf surface
        pSurface = pVeboxState->FFDNSurfaces[pRenderData->iCurDNOut];
        MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
        VeboxSurfCntlParams.bIsCompressed       = pSurface->bIsCompressed;
        VeboxSurfCntlParams.CompressionMode     = pSurface->CompressionMode;
        VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxSurfaceControlBits(
            &VeboxSurfCntlParams,
            (uint32_t *)&(pVeboxDiIecpCmdParams->DenoisedCurrOutputSurfCtrl.Value)));

        // For DN + SFC scenario, allocate FFDISurfaces also
        // since this usage needs IECP implicitly
        // For DN + DI + SFC, DI have registered FFDISurfaces, So don't register again
        if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData) && !bDiScdEnable)
        {
            VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
                        pOsInterface,
                        &pVeboxState->FFDISurfaces[pRenderData->iCurDNOut]->OsResource,
                        true,
                        true));

            pVeboxDiIecpCmdParams->pOsResCurrOutput   =
                &pVeboxState->FFDISurfaces[pRenderData->iCurDNOut]->OsResource;
            pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value =
                pVeboxState->DnDiSurfMemObjCtl.CurrentOutputSurfMemObjCtl;

            // Update control bits for CurrOutputSurf surface
            pSurface = pVeboxState->FFDISurfaces[pRenderData->iCurDNOut];
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed       = pSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode     = pSurface->CompressionMode;
            VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value)));
        }
    }

    // STMM surface
    if (bDiScdEnable || IsSTMMSurfNeeded())
    {
        // STMM in
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
            pOsInterface,
            &pVeboxState->STMMSurfaces[pRenderData->iCurHistIn].OsResource,
            false,
            true));

        pVeboxDiIecpCmdParams->pOsResStmmInput   =
            &pVeboxState->STMMSurfaces[pRenderData->iCurHistIn].OsResource;
        pVeboxDiIecpCmdParams->StmmInputSurfCtrl.Value =
            pVeboxState->DnDiSurfMemObjCtl.STMMInputSurfMemObjCtl;

        // Update control bits for stmm input surface
        pSurface = &(pVeboxState->STMMSurfaces[pRenderData->iCurHistIn]);
        MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
        VeboxSurfCntlParams.bIsCompressed       = pSurface->bIsCompressed;
        VeboxSurfCntlParams.CompressionMode     = pSurface->CompressionMode;
        VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxSurfaceControlBits(
            &VeboxSurfCntlParams,
            (uint32_t *)&(pVeboxDiIecpCmdParams->StmmInputSurfCtrl.Value)));

        // STMM out
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
            pOsInterface,
            &pVeboxState->STMMSurfaces[pRenderData->iCurHistOut].OsResource,
            true,
            true));

        pVeboxDiIecpCmdParams->pOsResStmmOutput   =
            &pVeboxState->STMMSurfaces[pRenderData->iCurHistOut].OsResource;
        pVeboxDiIecpCmdParams->StmmOutputSurfCtrl.Value =
            pVeboxState->DnDiSurfMemObjCtl.STMMOutputSurfMemObjCtl;

        // Update control bits for stmm output surface
        pSurface = &(pVeboxState->STMMSurfaces[pRenderData->iCurHistOut]);
        MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
        VeboxSurfCntlParams.bIsCompressed       = pSurface->bIsCompressed;
        VeboxSurfCntlParams.CompressionMode     = pSurface->CompressionMode;
        VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxSurfaceControlBits(
            &VeboxSurfCntlParams,
            (uint32_t *)&(pVeboxDiIecpCmdParams->StmmOutputSurfCtrl.Value)));
    }

    // Statistics data: GNE, FMD
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
        pOsInterface,
        &pVeboxState->VeboxStatisticsSurface.OsResource,
        true,
        true));

    pVeboxDiIecpCmdParams->pOsResStatisticsOutput   =
        &pVeboxState->VeboxStatisticsSurface.OsResource;
    pVeboxDiIecpCmdParams->StatisticsOutputSurfCtrl.Value =
        pVeboxState->DnDiSurfMemObjCtl.StatisticsOutputSurfMemObjCtl;

finish:
    return eStatus;
}

//!
//! \brief    Vebox query statistics surface layout
//! \details  Get Specific Layout Info like GNE Offset, size of per frame info inside 
//!           Vebox Statistics Surface for SKL+
//!           SKL+ changes: 
//!                 1) ACE histogram is outside of Vebox Statistics Surface;
//!                 2) Add White Balence Statistics;
//!
//!           | Layout of Statistics surface when DI enabled and DN either On or Off on SKL+\n
//!           |     --------------------------------------------------------------\n
//!           |     | 16 bytes for x=0, Y=0       | 16 bytes for x=16, Y=0       | ...\n
//!           |     |-------------------------------------------------------------\n
//!           |     | 16 bytes for x=0, Y=4       | ...\n
//!           |     |------------------------------\n
//!           |     | ...\n
//!           |     |------------------------------\n
//!           |     | 16 bytes for x=0, Y=height-4| ...\n
//!           |     |-----------------------------------------------Pitch--------------\n
//!           |     | 17 DW Reserved         | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
//!           |     |------------------------------------------------------------------\n
//!           |     | 11 DW FMD0 | 6 DW GNE0 | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
//!           |     |------------------------------------------------------------------\n
//!           |     | 17 DW Reserved         | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
//!           |     |------------------------------------------------------------------\n
//!           |     | 11 DW FMD1 | 6 DW GNE1 | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
//!           |     -------------------------------------------------------------------\n
//!           |\n
//!           | Layout of Statistics surface when DN enabled and DI disabled\n
//!           |     --------------------------------------------------------------\n
//!           |     | 16 bytes for x=0, Y=0       | 16 bytes for x=16, Y=0       | ...\n
//!           |     |-------------------------------------------------------------\n
//!           |     | 16 bytes for x=0, Y=4       | ...\n
//!           |     |------------------------------\n
//!           |     | ...\n
//!           |     |------------------------------\n
//!           |     | 16 bytes for x=0, Y=height-4| ...\n
//!           |     |-----------------------------------------------Pitch--------------\n
//!           |     | 11 DW FMD0 | 6 DW GNE0 | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
//!           |     |------------------------------------------------------------------\n
//!           |     | 11 DW FMD1 | 6 DW GNE1 | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
//!           |     -------------------------------------------------------------------\n
//!           |\n
//!           | Layout of Statistics surface when both DN and DI are disabled\n
//!           |     ------------------------------------------------Pitch--------------\n
//!           |     | 17 DW White Balence0   | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
//!           |     |------------------------------------------------------------------\n
//!           |     | 17 DW White Balence1   | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
//!           |     -------------------------------------------------------------------\n
//! \param    [in] QueryType
//!           Query type
//! \param    [out] pQuery
//!           return layout type
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G9_BASE::VeboxQueryStatLayout(
    VEBOX_STAT_QUERY_TYPE       QueryType,
    uint32_t*                   pQuery)
{
    MOS_STATUS             eStatus = MOS_STATUS_SUCCESS;

    VPHAL_RENDER_ASSERT(pQuery);

    switch (QueryType)
    {
        case VEBOX_STAT_QUERY_GNE_OFFEST:
            *pQuery = VPHAL_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_G9;
            break;

        case VEBOX_STAT_QUERY_PER_FRAME_SIZE:
            *pQuery = VPHAL_VEBOX_STATISTICS_PER_FRAME_SIZE_G9;
            break;

        case VEBOX_STAT_QUERY_FMD_OFFEST:
            *pQuery = VPHAL_VEBOX_STATISTICS_SURFACE_FMD_OFFSET_G9;
            break;

        case VEBOX_STAT_QUERY_STD_OFFEST:
            *pQuery = VPHAL_VEBOX_STATISTICS_SURFACE_STD_OFFSET_G9;
            break;

        default:
            VPHAL_RENDER_ASSERTMESSAGE("Vebox Statistics Layout Query, type ('%d') is not implemented.", QueryType);
            eStatus = MOS_STATUS_UNKNOWN;
            break;
    }

    return eStatus;
}

//!
//! \brief    Vebox get Luma default value
//! \details  Initialize luma denoise paramters w/ default values.
//! \param    [out] pLumaParams
//!           Pointer to Luma DN parameter
//! \return   void
//!
void VPHAL_VEBOX_STATE_G9_BASE::GetLumaDefaultValue(
    PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams)
{
    VPHAL_RENDER_ASSERT(pLumaParams);

    pLumaParams->dwDenoiseASDThreshold      = NOISE_ABSSUMTEMPORALDIFF_THRESHOLD_DEFAULT_G9;
    pLumaParams->dwDenoiseHistoryDelta      = NOISE_HISTORY_DELTA_DEFAULT;
    pLumaParams->dwDenoiseMaximumHistory    = NOISE_HISTORY_MAX_DEFAULT_G9;
    pLumaParams->dwDenoiseSTADThreshold     = NOISE_SUMABSTEMPORALDIFF_THRESHOLD_DEFAULT_G9;
    pLumaParams->dwDenoiseSCMThreshold      = NOISE_SPATIALCOMPLEXITYMATRIX_THRESHOLD_DEFAULT_G9;
    pLumaParams->dwDenoiseMPThreshold       = NOISE_NUMMOTIONPIXELS_THRESHOLD_DEFAULT_G9;
    pLumaParams->dwLTDThreshold             = NOISE_LOWTEMPORALPIXELDIFF_THRESHOLD_DEFAULT_G9;
    pLumaParams->dwTDThreshold              = NOISE_TEMPORALPIXELDIFF_THRESHOLD_DEFAULT_G9;
}

//!
//! \brief    Vebox set DN parameter
//! \details  Set denoise paramters for luma and chroma.
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \param    [in] pLumaParams
//!           Pointer to Luma DN parameter
//! \param    [in] pChromaParams
//!           Pointer to Chroma DN parameter
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G9_BASE::SetDNDIParams(
    PVPHAL_SURFACE                   pSrcSurface,
    PVPHAL_SAMPLER_STATE_DNDI_PARAM  pLumaParams,
    PVPHAL_DNUV_PARAMS               pChromaParams)
{
    MOS_STATUS                       eStatus;
    PVPHAL_DENOISE_PARAMS            pDNParams;
    uint32_t                         dwDenoiseFactor;
    PVPHAL_VEBOX_RENDER_DATA         pRenderData = GetLastExecRenderData();

    VPHAL_RENDER_ASSERT(pSrcSurface);
    VPHAL_RENDER_ASSERT(pLumaParams);
    VPHAL_RENDER_ASSERT(pChromaParams);
    VPHAL_RENDER_ASSERT(pRenderData);

    eStatus             = MOS_STATUS_SUCCESS;
    pDNParams           = pSrcSurface->pDenoiseParams;

    VPHAL_RENDER_ASSERT(pDNParams);

    // Set Luma DN params
    if (pRenderData->bDenoise)
    {
        // Setup Denoise Params
        GetLumaDefaultValue(pLumaParams);

        // Initialize pixel range threshold array
        pRenderData->VeboxDNDIParams.dwPixRangeThreshold[0] = NOISE_BLF_RANGE_THRESHOLD_S0_DEFAULT;
        pRenderData->VeboxDNDIParams.dwPixRangeThreshold[1] = NOISE_BLF_RANGE_THRESHOLD_S1_DEFAULT;
        pRenderData->VeboxDNDIParams.dwPixRangeThreshold[2] = NOISE_BLF_RANGE_THRESHOLD_S2_DEFAULT;
        pRenderData->VeboxDNDIParams.dwPixRangeThreshold[3] = NOISE_BLF_RANGE_THRESHOLD_S3_DEFAULT;
        pRenderData->VeboxDNDIParams.dwPixRangeThreshold[4] = NOISE_BLF_RANGE_THRESHOLD_S4_DEFAULT;
        pRenderData->VeboxDNDIParams.dwPixRangeThreshold[5] = NOISE_BLF_RANGE_THRESHOLD_S5_DEFAULT;

        // Initialize pixel range weight array
        pRenderData->VeboxDNDIParams.dwPixRangeWeight[0]    = NOISE_BLF_RANGE_WGTS0_DEFAULT;
        pRenderData->VeboxDNDIParams.dwPixRangeWeight[1]    = NOISE_BLF_RANGE_WGTS1_DEFAULT;
        pRenderData->VeboxDNDIParams.dwPixRangeWeight[2]    = NOISE_BLF_RANGE_WGTS2_DEFAULT;
        pRenderData->VeboxDNDIParams.dwPixRangeWeight[3]    = NOISE_BLF_RANGE_WGTS3_DEFAULT;
        pRenderData->VeboxDNDIParams.dwPixRangeWeight[4]    = NOISE_BLF_RANGE_WGTS4_DEFAULT;
        pRenderData->VeboxDNDIParams.dwPixRangeWeight[5]    = NOISE_BLF_RANGE_WGTS5_DEFAULT;

        // Denoise Slider case (no auto DN detect)
        if (!pDNParams->bAutoDetect)
        {
            dwDenoiseFactor = (uint32_t)pDNParams->fDenoiseFactor;

            if (dwDenoiseFactor > NOISEFACTOR_MAX)
            {
                dwDenoiseFactor = NOISEFACTOR_MAX;
            }

            pLumaParams->dwDenoiseHistoryDelta   = dwDenoiseHistoryDelta[dwDenoiseFactor];
            pLumaParams->dwDenoiseMaximumHistory = dwDenoiseMaximumHistory[dwDenoiseFactor];
            pLumaParams->dwDenoiseASDThreshold   = dwDenoiseASDThreshold[dwDenoiseFactor];
            pLumaParams->dwDenoiseSCMThreshold   = dwDenoiseSCMThreshold[dwDenoiseFactor];
            pLumaParams->dwDenoiseMPThreshold    = dwDenoiseMPThreshold[dwDenoiseFactor];
            pLumaParams->dwLTDThreshold          = dwLTDThreshold[dwDenoiseFactor];
            pLumaParams->dwTDThreshold           = dwTDThreshold[dwDenoiseFactor];
            pLumaParams->dwDenoiseSTADThreshold  = dwDenoiseSTADThreshold[dwDenoiseFactor];

            pRenderData->VeboxDNDIParams.dwPixRangeThreshold[0] = dwPixRangeThreshold0[dwDenoiseFactor];
            pRenderData->VeboxDNDIParams.dwPixRangeThreshold[1] = dwPixRangeThreshold1[dwDenoiseFactor];
            pRenderData->VeboxDNDIParams.dwPixRangeThreshold[2] = dwPixRangeThreshold2[dwDenoiseFactor];
            pRenderData->VeboxDNDIParams.dwPixRangeThreshold[3] = dwPixRangeThreshold3[dwDenoiseFactor];
            pRenderData->VeboxDNDIParams.dwPixRangeThreshold[4] = dwPixRangeThreshold4[dwDenoiseFactor];
            pRenderData->VeboxDNDIParams.dwPixRangeThreshold[5] = dwPixRangeThreshold5[dwDenoiseFactor];

            pRenderData->VeboxDNDIParams.dwPixRangeWeight[0] = dwPixRangeWeight0[dwDenoiseFactor];
            pRenderData->VeboxDNDIParams.dwPixRangeWeight[1] = dwPixRangeWeight1[dwDenoiseFactor];
            pRenderData->VeboxDNDIParams.dwPixRangeWeight[2] = dwPixRangeWeight2[dwDenoiseFactor];
            pRenderData->VeboxDNDIParams.dwPixRangeWeight[3] = dwPixRangeWeight3[dwDenoiseFactor];
            pRenderData->VeboxDNDIParams.dwPixRangeWeight[4] = dwPixRangeWeight4[dwDenoiseFactor];
            pRenderData->VeboxDNDIParams.dwPixRangeWeight[5] = dwPixRangeWeight5[dwDenoiseFactor];
        }
    }

    // Set Chroma DN params
    if (pRenderData->bChromaDenoise)
    {
        // Setup Denoise Params
        pChromaParams->dwHistoryDeltaUV = NOISE_HISTORY_DELTA_DEFAULT;
        pChromaParams->dwHistoryMaxUV   = NOISE_HISTORY_MAX_DEFAULT;

        // Denoise Slider case (no auto DN detect)
        if (!pDNParams->bAutoDetect)
        {
            dwDenoiseFactor = (uint32_t)pDNParams->fDenoiseFactor;

            if (dwDenoiseFactor > NOISEFACTOR_MAX)
            {
                dwDenoiseFactor = NOISEFACTOR_MAX;
            }

            pChromaParams->dwLTDThresholdU  =
            pChromaParams->dwLTDThresholdV  = dwLTDThresholdUV[dwDenoiseFactor];

            pChromaParams->dwTDThresholdU   =
            pChromaParams->dwTDThresholdV   = dwTDThresholdUV[dwDenoiseFactor];

            pChromaParams->dwSTADThresholdU =
            pChromaParams->dwSTADThresholdV = dwSTADThresholdUV[dwDenoiseFactor];
        }
    }

    if (pDNParams && pDNParams->bEnableHVSDenoise)
    {
        VPHAL_VEBOX_STATE::VeboxSetHVSDNParams(pSrcSurface);
    }

    return eStatus;
}

//!
//! \brief    Get output surface of Vebox
//! \details  Get output surface of Vebox in current operation
//! \param    [in] bDiVarianceEnable
//!           Is DI/Variances report enabled
//! \return   PVPHAL_SURFACE
//!           Corresponding output surface pointer
//!
PVPHAL_SURFACE VPHAL_VEBOX_STATE_G9_BASE::GetSurfOutput(
    bool                                    bDiVarianceEnable)
{
    PVPHAL_SURFACE                          pSurface    = nullptr;
    PVPHAL_VEBOX_STATE_G9_BASE              pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA                pRenderData = GetLastExecRenderData();

    if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))                    // Vebox output pipe
    {
        pSurface = pRenderData->pRenderTarget;
    }
    else if (bDiVarianceEnable)                                     // DNDI, DI, DI + IECP
    {
        pSurface = pVeboxState->FFDISurfaces[pRenderData->iFrame0];
    }
    else if (IsIECPEnabled())                                       // DN + IECP or IECP only
    {
        pSurface = pVeboxState->FFDISurfaces[pRenderData->iCurDNOut];
    }
    else if (pRenderData->bDenoise)                                 // DN only
    {
        pSurface = pVeboxState->FFDNSurfaces[pRenderData->iCurDNOut];
    }
    else if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData))                 // Write to SFC
    {
        // Vebox o/p should not be written to memory
        pSurface = nullptr;
    }
    else
    {
        VPHAL_RENDER_ASSERTMESSAGE("Unable to determine Vebox Output Surface.");
    }

    return pSurface;
}

//!
//! \brief    Setup surface states for Vebox
//! \details  Setup surface states for use in the current Vebox Operation
//! \param    [in] bDiVarianceEnable
//!           Is DI/Variances report enabled
//! \param    [in,out] pVeboxSurfaceStateCmdParams
//!           Pointer to VEBOX_SURFACE_STATE command parameters
//! \return   void
//!
void VPHAL_VEBOX_STATE_G9_BASE::SetupSurfaceStates(
    bool                                    bDiVarianceEnable,
    PVPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS   pVeboxSurfaceStateCmdParams)
{
    PVPHAL_VEBOX_STATE_G9_BASE              pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA                pRenderData = GetLastExecRenderData();

    MOS_ZeroMemory(pVeboxSurfaceStateCmdParams,
        sizeof(VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS));

    pVeboxSurfaceStateCmdParams->pSurfInput = pVeboxState->m_currentSurface;

    pVeboxSurfaceStateCmdParams->pSurfOutput = pVeboxState->GetSurfOutput(bDiVarianceEnable);

    pVeboxSurfaceStateCmdParams->pSurfSTMM      = &pVeboxState->STMMSurfaces[pRenderData->iCurHistIn];
    pVeboxSurfaceStateCmdParams->pSurfDNOutput  = pVeboxState->FFDNSurfaces[pRenderData->iCurDNOut];

    pVeboxSurfaceStateCmdParams->bDIEnable      = bDiVarianceEnable;

}

bool VPHAL_VEBOX_STATE_G9_BASE::UseKernelResource()
{
    return false; // can always use driver resource in clear memory
}

//!
//! \brief    Setup Vebox_State Command parameter
//! \param    [in] bDiVarianceEnable
//!           Is DI/Variances report enabled
//! \param    [in,out] pVeboxStateCmdParams
//!           Pointer to VEBOX_STATE command parameters
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G9_BASE::SetupVeboxState(
    bool                                    bDiVarianceEnable,
    PMHW_VEBOX_STATE_CMD_PARAMS             pVeboxStateCmdParams)
{
    PMHW_VEBOX_MODE         pVeboxMode;
    PMOS_INTERFACE          pOsInterface;
    MOS_STATUS              eStatus;

    PVPHAL_VEBOX_STATE_G9_BASE              pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA                pRenderData = GetLastExecRenderData();

    pVeboxMode    = &pVeboxStateCmdParams->VeboxMode;
    pOsInterface  = pVeboxState->m_pOsInterface;
    eStatus       = MOS_STATUS_SUCCESS;

    MOS_ZeroMemory(pVeboxStateCmdParams, sizeof(*pVeboxStateCmdParams));

    if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData) ||
        IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))
    {
        // On SKL, GlobalIECP must be enabled when the output pipe is Vebox or SFC
        pVeboxMode->GlobalIECPEnable = true;
    }
    else
    {
        pVeboxMode->GlobalIECPEnable = IsIECPEnabled();
    }

    pVeboxMode->DIEnable                     = bDiVarianceEnable;

    pVeboxMode->SFCParallelWriteEnable       = IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData) &&
                                               (pRenderData->bDenoise || bDiVarianceEnable);
    pVeboxMode->DNEnable                     = pRenderData->bDenoise;
    pVeboxMode->DNDIFirstFrame               = !pRenderData->bRefValid;

    pVeboxMode->DIOutputFrames               = SetDIOutputFrame(pRenderData, pVeboxState, pVeboxMode);

    pVeboxMode->DisableEncoderStatistics     = true;

    if((pVeboxMode->DIEnable == false)                                                 &&
       (pVeboxMode->DNEnable != false || pVeboxMode->HotPixelFilteringEnable != false) &&
       ((pVeboxState->bDisableTemporalDenoiseFilter)            ||
        (IS_RGB_CSPACE(pVeboxState->m_currentSurface->ColorSpace)) ||
        (pVeboxMode->HotPixelFilteringEnable && (pVeboxMode->DNEnable == false) && (pVeboxMode->DIEnable == false))))
    {
        pVeboxMode->DisableTemporalDenoiseFilter = true;
        // GlobalIECP or Demosaic must be enabled even if IECP not used
        pVeboxMode->GlobalIECPEnable             = true;
    }
    else
    {
        pVeboxMode->DisableTemporalDenoiseFilter = false;
    }

    pVeboxStateCmdParams->bUseVeboxHeapKernelResource
                                             = UseKernelResource();

    // This field must be set if 00b for products that don't have 2 slices.
    if (MEDIA_IS_SKU(pVeboxState->m_pRenderHal->pSkuTable, FtrSingleVeboxSlice))
    {
        pVeboxMode->SingleSliceVeboxEnable = 0;
    }
    else
    {
        // Permanent program limitation that should go in all the configurations of SKLGT which have 2 VEBOXes (i.e. GT3 & GT4)
        // VEBOX1 should be disabled whenever there is an VE-SFC workload.
        // This is because we have only one SFC all the GT configurations and that SFC is tied to VEBOX0.Hence the programming restriction.
        if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData))
        {
            pVeboxMode->SingleSliceVeboxEnable = 1;
        }
        else
        {
            pVeboxMode->SingleSliceVeboxEnable = 0;
        }
    }

    return eStatus;
}

//!
//! \brief    Get the output pipe on SKL
//! \details  There are 3 output pipes on SKL. Check which output pipe can be applied
//! \param    [in] pcRenderParams
//!           Pointer to VpHal render parameters
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \param    [out] pbCompNeeded
//!           return whether composition is needed after Vebox/SFC
//! \return   VPHAL_OUTPUT_PIPE_MODE
//!           return the output pipe mode
//!
VPHAL_OUTPUT_PIPE_MODE VPHAL_VEBOX_STATE_G9_BASE::GetOutputPipe(
    PCVPHAL_RENDER_PARAMS       pcRenderParams,
    PVPHAL_SURFACE              pSrcSurface,
    bool*                       pbCompNeeded)
{
    VPHAL_OUTPUT_PIPE_MODE      OutputPipe;
    bool                        bCompBypassFeasible;
    bool                        bOutputPipeVeboxFeasible;
    PVPHAL_SURFACE              pTarget;
    PVPHAL_VEBOX_STATE_G9_BASE  pVeboxState = this;

    OutputPipe  = VPHAL_OUTPUT_PIPE_MODE_COMP;

    bCompBypassFeasible = IS_COMP_BYPASS_FEASIBLE(*pbCompNeeded, pcRenderParams, pSrcSurface);

    if (!bCompBypassFeasible)
    {
        OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
        goto finish;
    }

    bOutputPipeVeboxFeasible = IS_OUTPUT_PIPE_VEBOX_FEASIBLE(pVeboxState, pcRenderParams, pSrcSurface);
    if (bOutputPipeVeboxFeasible)
    {
        OutputPipe = VPHAL_OUTPUT_PIPE_MODE_VEBOX;
        goto finish;
    }

    pTarget    = pcRenderParams->pTarget[0];
    // Check if SFC can be the output pipe
    if (m_sfcPipeState)
    {
        OutputPipe = m_sfcPipeState->GetOutputPipe(
                        pSrcSurface,
                        pTarget,
                        pcRenderParams);
    }
    else
    {
        OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
    }

    // Explore the potential to still output by VEBOX and perform quick color fill in composition
    if (bCompBypassFeasible &&
        OutputPipe == VPHAL_OUTPUT_PIPE_MODE_COMP &&
        pcRenderParams->pColorFillParams &&
        pSrcSurface->rcDst.left  == pTarget->rcDst.left &&
        pSrcSurface->rcDst.top   == pTarget->rcDst.top &&
        pSrcSurface->rcDst.right == pTarget->rcDst.right &&
        pSrcSurface->rcDst.bottom < pTarget->rcDst.bottom)
    {
        int32_t lTargetBottom;
        lTargetBottom         = pTarget->rcDst.bottom;
        pTarget->rcDst.bottom = pSrcSurface->rcDst.bottom;

        // Check if Vebox can be the output pipe again
        bOutputPipeVeboxFeasible = IS_OUTPUT_PIPE_VEBOX_FEASIBLE(pVeboxState, pcRenderParams, pSrcSurface);
        if (bOutputPipeVeboxFeasible)
        {
            OutputPipe              = VPHAL_OUTPUT_PIPE_MODE_VEBOX;
            pTarget->bFastColorFill = true;
        }
        pTarget->rcDst.bottom = lTargetBottom;
    }

finish:
    *pbCompNeeded = (OutputPipe == VPHAL_OUTPUT_PIPE_MODE_COMP) ? true : false;
    return OutputPipe;
}

//!
//! \brief    Vebox is needed on SKL
//! \details  Check if Vebox Render operation can be applied
//! \param    [in] pcRenderParams
//!           Pointer to VpHal render parameters
//! \param    [in,out] pRenderPassData
//!           Pointer to Render data
//! \return   bool
//!           return true if Vebox is needed, otherwise false
//!
bool VPHAL_VEBOX_STATE_G9_BASE::IsNeeded(
    PCVPHAL_RENDER_PARAMS       pcRenderParams,
    RenderpassData              *pRenderPassData)
{
    PVPHAL_VEBOX_RENDER_DATA    pRenderData;
    PRENDERHAL_INTERFACE        pRenderHal;
    PVPHAL_SURFACE              pRenderTarget;
    bool                        bVeboxNeeded;
    PMOS_INTERFACE              pOsInterface;
    MOS_STATUS                  eStatus;
    PVPHAL_VEBOX_STATE_G9_BASE  pVeboxState = this;
    PVPHAL_SURFACE              pSrcSurface;

    bVeboxNeeded  = false;
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pRenderHal);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pOsInterface);

    pRenderHal    = pVeboxState->m_pRenderHal;
    pOsInterface  = pVeboxState->m_pOsInterface;

    pRenderTarget = pcRenderParams->pTarget[0];
    pRenderData   = GetLastExecRenderData();
    pSrcSurface   = pRenderPassData->pSrcSurface;

    VPHAL_RENDER_CHK_NULL(pSrcSurface);

    // Check whether VEBOX is available
    // VTd doesn't support VEBOX
    if (!MEDIA_IS_SKU(pVeboxState->m_pSkuTable, FtrVERing))
    {
        pRenderPassData->bCompNeeded = true;
        goto finish;
    }

    // check if UserPtr enabling.
    if (pcRenderParams->bUserPrt_16Align[0])
    {
        pRenderPassData->bCompNeeded = true;
        goto finish;
    }

    // Check if the Surface size is greater than 64x16 which is the minimum Width and Height VEBOX can handle
    if (pSrcSurface->dwWidth < MHW_VEBOX_MIN_WIDTH || pSrcSurface->dwHeight < MHW_VEBOX_MIN_HEIGHT)
    {
        pRenderPassData->bCompNeeded = true;
        goto finish;
    }

    pRenderData->Init();
    if (MEDIA_IS_SKU(m_pSkuTable, FtrSFCPipe) && m_sfcPipeState)
    {
        m_sfcPipeState->InitRenderData();
    }

    // Determine the output pipe before setting the rendering flags for Vebox and SFC
    SET_VPHAL_OUTPUT_PIPE(
        pRenderData,
        GetOutputPipe(
            pcRenderParams,
            pSrcSurface,
            &pRenderPassData->bCompNeeded));

    // Set MMC State
    SET_VPHAL_MMC_STATE(pRenderData, pVeboxState->bEnableMMC);

    // Update execution state based on current and past events such as the
    // # of future and past frames available.
    pVeboxState->UpdateVeboxExecutionState(
        pSrcSurface,
        pRenderData->OutputPipe);

    // Set Component
    SET_VPHAL_COMPONENT(pRenderData, pcRenderParams->Component);

    // Check if Vebox can be used to process the surface
    if (pVeboxState->IsFormatSupported(pSrcSurface))
    {
        // Save Alpha passed by App to be used in Vebox
        if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))
        {
            pRenderData->pAlphaParams = pcRenderParams->pCompAlpha;
        }

        // Setup Rendering Flags for Vebox
        VeboxSetRenderingFlags(
            pSrcSurface,
            pRenderTarget);

        // Vebox is needed if Vebox isn't bypassed
        bVeboxNeeded = !pRenderData->bVeboxBypass;
    }

    // if ScalingPreference == VPHAL_SCALING_PREFER_SFC_FOR_VEBOX, use SFC only when VEBOX is required
    if ((pSrcSurface->ScalingPreference == VPHAL_SCALING_PREFER_SFC_FOR_VEBOX)    &&
        (bVeboxNeeded == false))
    {
        VPHAL_RENDER_NORMALMESSAGE("DDI choose to use SFC only for VEBOX, and since VEBOX is not required, change to Composition.");
        pRenderData->OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
        pRenderPassData->bCompNeeded = true;
    }

    // Check if we want to enable SFC processing
    if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData))
    {
        // Setup Rendering Flags for SFC pipe
        m_sfcPipeState->SetRenderingFlags(
            pcRenderParams->pColorFillParams,
            pcRenderParams->pCompAlpha,
            pSrcSurface,
            pRenderTarget,
            pRenderData);

        // Update Vebox Rendering Flags when the output pipe is SFC.
        // If input surface format is AYUV, and just have one layer as primary,Procamp can also enable.
        // Those flags cannot be updated inside Vebox's SetRenderingFlags due to ScalingPreference option will
        // turn back to composition when Vebox is not needed in above code.
        pRenderData->bProcamp = (IS_YUV_FORMAT(pSrcSurface->Format) ||
                                (pSrcSurface->Format == Format_AYUV &&
                                pcRenderParams->uSrcCount == 1))    &&
                                pSrcSurface->pProcampParams         &&
                                pSrcSurface->pProcampParams->bEnabled;
        pRenderData->bBeCsc   = IS_RGB_CSPACE(pSrcSurface->ColorSpace);
        pRenderData->bIECP    = pRenderData->bIECP    ||
                                pRenderData->bProcamp ||
                                pRenderData->bBeCsc;

        bVeboxNeeded = true;
    }

finish:
    return bVeboxNeeded;
}

//!
//! \brief    Vebox get the back-end colorspace conversion matrix
//! \details  When the i/o is A8R8G8B8 or X8R8G8B8, the transfer matrix
//!           needs to be updated accordingly
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \param    [in] pOutSurface
//!           Pointer to output surface of Vebox
//! \return   void
//!
void VPHAL_VEBOX_STATE_G9_BASE::VeboxGetBeCSCMatrix(
    PVPHAL_SURFACE                   pSrcSurface,
    PVPHAL_SURFACE                   pOutSurface)
{
    PVPHAL_VEBOX_STATE_G9_BASE           pVeboxState = this;
    float       fTemp[3];

    // Get the matrix to use for conversion
    VpHal_GetCscMatrix(
        pSrcSurface->ColorSpace,
        pOutSurface->ColorSpace,
        pVeboxState->fCscCoeff,
        pVeboxState->fCscInOffset,
        pVeboxState->fCscOutOffset);

    // Vebox CSC converts RGB input to YUV for SFC
    // Vebox only supports A8B8G8R8 input, swap the 1st and 3rd
    // columns of the transfer matrix for A8R8G8B8 and X8R8G8B8
    // This only happens when SFC output is used
    if ((pSrcSurface->Format == Format_A8R8G8B8) ||
        (pSrcSurface->Format == Format_X8R8G8B8))
    {
        fTemp[0] = pVeboxState->fCscCoeff[0];
        fTemp[1] = pVeboxState->fCscCoeff[3];
        fTemp[2] = pVeboxState->fCscCoeff[6];

        pVeboxState->fCscCoeff[0] = pVeboxState->fCscCoeff[2];
        pVeboxState->fCscCoeff[3] = pVeboxState->fCscCoeff[5];
        pVeboxState->fCscCoeff[6] = pVeboxState->fCscCoeff[8];

        pVeboxState->fCscCoeff[2] = fTemp[0];
        pVeboxState->fCscCoeff[5] = fTemp[1];
        pVeboxState->fCscCoeff[8] = fTemp[2];
    }
}

#if VEBOX_AUTO_DENOISE_SUPPORTED
//!
//! \brief    Load update kernel curbe data
//! \details  Loads the static data of update kernel to curbe
//! \param    [out] iCurbeOffsetOutDN
//!           Pointer to DN kernel curbe offset
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G9_BASE::LoadUpdateDenoiseKernelStaticData(
    int32_t*                            iCurbeOffsetOutDN)
{
    PRENDERHAL_INTERFACE                pRenderHal;
    VEBOX_STATE_UPDATE_STATIC_DATA_G9   DNStaticData;        // DN Update kernelStatic parameters
    PMHW_VEBOX_INTERFACE                pVeboxInterface;
    PVPHAL_DENOISE_PARAMS               pDenoiseParams;      // Denoise
    int32_t                             iOffset0, iOffset1;
    MOS_STATUS                          eStatus;
    PVPHAL_VEBOX_STATE_G9_BASE          pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA            pRenderData = GetLastExecRenderData();

    pRenderHal   = pVeboxState->m_pRenderHal;
    pVeboxInterface = pVeboxState->m_pVeboxInterface;
    eStatus = MOS_STATUS_SUCCESS;

    // init the static data
    MOS_ZeroMemory(&DNStaticData, sizeof(VEBOX_STATE_UPDATE_STATIC_DATA_G9));

    pDenoiseParams = m_currentSurface->pDenoiseParams;
    VPHAL_RENDER_ASSERT(pDenoiseParams);

    // Get offset for slice0 and slice1
    VPHAL_RENDER_CHK_STATUS(VeboxGetStatisticsSurfaceOffsets(
        &iOffset0,
        &iOffset1));

    // Load DN update kernel CURBE data
    if (pRenderData->bAutoDenoise)
    {
        // set the curbe data for DN update kernel
        DNStaticData.DW00.OffsetToSlice0 = iOffset0;
        DNStaticData.DW01.OffsetToSlice1 = iOffset1;
        DNStaticData.DW02.FirstFrameFlag = pVeboxState->bFirstFrame;
        DNStaticData.DW02.NoiseLevel     = pDenoiseParams->NoiseLevel;
        DNStaticData.DW03.RangeThrAdp2NLvl       = 1;
        DNStaticData.DW04.VeboxStatisticsSurface = BI_DN_STATISTICS_SURFACE;
        DNStaticData.DW05.VeboxDndiStateSurface  = BI_DN_VEBOX_STATE_SURFACE;
        DNStaticData.DW06.VeboxTempSurface       = BI_DN_TEMP_SURFACE;
        DNStaticData.DW07.VeboxSpatialAttributesConfigurationSurface = BI_DN_SPATIAL_ATTRIBUTES_CONFIGURATION_SURFACE;

        *iCurbeOffsetOutDN = pRenderHal->pfnLoadCurbeData(
            pRenderHal,
            pRenderData->pMediaState,
            &DNStaticData,
            sizeof(DNStaticData));

        if (*iCurbeOffsetOutDN < 0)
        {
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }

        pRenderData->iCurbeLength += sizeof(DNStaticData);
    }

finish:
    return eStatus;
}

//!
//! \brief    Setup surface states for Denoise
//! \details  Setup Surface State for Vebox States Auto DN kernel
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G9_BASE::SetupSurfaceStatesForDenoise()
{
    PRENDERHAL_INTERFACE            pRenderHal;
    PMOS_INTERFACE                  pOsInterface;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParams;
    MOS_STATUS                      eStatus;
    MOS_FORMAT                      tmpFormat;
    bool                            bUseKernelResource;
    const MHW_VEBOX_HEAP            *pVeboxHeap = nullptr;
    PVPHAL_VEBOX_STATE_G9_BASE      pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA        pRenderData = GetLastExecRenderData();

    eStatus            = MOS_STATUS_SUCCESS;
    pRenderHal         = pVeboxState->m_pRenderHal;
    pOsInterface       = pVeboxState->m_pOsInterface;
    VPHAL_RENDER_CHK_STATUS(pVeboxState->m_pVeboxInterface->GetVeboxHeapInfo(
                                &pVeboxHeap));
    VPHAL_RENDER_CHK_NULL(pVeboxHeap);
    VPHAL_RENDER_CHK_NULL(pOsInterface);
    VPHAL_RENDER_CHK_NULL(pOsInterface->osCpInterface);

    bUseKernelResource = UseKernelResource();

    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));
    MOS_ZeroMemory(&pVeboxState->VeboxHeapResource, sizeof(VPHAL_SURFACE));
    MOS_ZeroMemory(&pVeboxState->tmpResource, sizeof(VPHAL_SURFACE));

    // Treat the 1D buffer as 2D surface
    // VEBox State Surface
    pVeboxState->VeboxHeapResource.Format   = Format_L8;
    pVeboxState->VeboxHeapResource.dwWidth  = SECURE_BLOCK_COPY_KERNEL_SURF_WIDTH; // Hard code for secure Block Copy kernel
    pVeboxState->VeboxHeapResource.dwPitch  = SECURE_BLOCK_COPY_KERNEL_SURF_WIDTH; // Hard code for secure Block Copy kernel
    pVeboxState->VeboxHeapResource.dwHeight =
        MOS_ROUNDUP_DIVIDE(pVeboxHeap->uiInstanceSize, SECURE_BLOCK_COPY_KERNEL_SURF_WIDTH);
    pVeboxState->VeboxHeapResource.dwOffset =
        pVeboxHeap->uiInstanceSize *
        pVeboxHeap->uiCurState;
    pVeboxState->VeboxHeapResource.TileType = MOS_TILE_LINEAR;
    pVeboxState->VeboxHeapResource.OsResource = bUseKernelResource ?
                                    pVeboxHeap->KernelResource :
                                    pVeboxHeap->DriverResource;

    // Temp Surface: for Noise Level History
    pVeboxState->tmpResource.Format = Format_L8;
    pVeboxState->tmpResource.dwWidth = SECURE_BLOCK_COPY_KERNEL_SURF_WIDTH; // Hard code for secure Block Copy kernel
    pVeboxState->tmpResource.dwPitch = SECURE_BLOCK_COPY_KERNEL_SURF_WIDTH; // Hard code for secure Block Copy kernel
    pVeboxState->tmpResource.dwHeight =
        MOS_ROUNDUP_DIVIDE(MHW_PAGE_SIZE, SECURE_BLOCK_COPY_KERNEL_SURF_WIDTH);
    pVeboxState->tmpResource.dwOffset = 0;
    pVeboxState->tmpResource.TileType = MOS_TILE_LINEAR;
    pVeboxState->tmpResource.OsResource = pVeboxState->VeboxTempSurface.OsResource;

    // Statistics Surface-----------------------------------------------------------
    tmpFormat                                  = pVeboxState->VeboxStatisticsSurface.Format;
    pVeboxState->VeboxStatisticsSurface.Format = Format_RAW;

    VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetBufferSurfaceForHwAccess(
                pRenderHal,
                &pVeboxState->VeboxStatisticsSurface,
                &pVeboxState->RenderHalVeboxStatisticsSurface,
                nullptr,
                pRenderData->iBindingTable,
                BI_DN_STATISTICS_SURFACE,
                false));

    pVeboxState->VeboxStatisticsSurface.Format = tmpFormat;

    // VEBox State Surface-----------------------------------------------------------
    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));

    SurfaceParams.Type              = pRenderHal->SurfaceTypeDefault;
    SurfaceParams.bRenderTarget     = true;
    SurfaceParams.bWidthInDword_Y   = true;
    SurfaceParams.bWidthInDword_UV  = true;
    SurfaceParams.Boundary          = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    SurfaceParams.bWidth16Align     = false;

    VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetSurfaceForHwAccess(
                pRenderHal,
                &pVeboxState->VeboxHeapResource,
                &pVeboxState->RenderHalVeboxHeapResource,
                &SurfaceParams,
                pRenderData->iBindingTable,
                BI_DN_VEBOX_STATE_SURFACE,
                true));

    // VEBox Temp Surface-----------------------------------------------------------
    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));

    SurfaceParams.Type              = pRenderHal->SurfaceTypeDefault;
    SurfaceParams.bRenderTarget     = true;
    SurfaceParams.bWidthInDword_Y   = true;
    SurfaceParams.bWidthInDword_UV  = true;
    SurfaceParams.Boundary          = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    SurfaceParams.bWidth16Align     = false;

    // set bRenderTarget=false to skip first frame for PermeatePatchForHM().
    if (pVeboxState->bFirstFrame && pOsInterface->osCpInterface->IsHMEnabled())
    {
        SurfaceParams.bRenderTarget = false;
    }

    VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetSurfaceForHwAccess(
                pRenderHal,
                &pVeboxState->tmpResource,
                &pVeboxState->RenderHalTmpResource,
                &SurfaceParams,
                pRenderData->iBindingTable,
                BI_DN_TEMP_SURFACE,
                true));

    // Spatial Attributes Configuration Surface------------------------------------
    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));

    VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetBufferSurfaceForHwAccess(
        pRenderHal,
        &pVeboxState->VeboxSpatialAttributesConfigurationSurface,
        &pVeboxState->RenderHalVeboxSpatialAttributesConfigurationSurface,
        &SurfaceParams,
        pRenderData->iBindingTable,
        BI_DN_SPATIAL_ATTRIBUTES_CONFIGURATION_SURFACE,
        false));

finish:
    return eStatus;
}
#endif

//!
//! \brief    Setup kernels for Vebox auto mode features
//! \details  Setup kernels that co-operate with Vebox auto mode features
//! \param    [in] iKDTIndex
//!           Index to Kernel Parameter Array (defined platform specific)
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G9_BASE::SetupVeboxKernel(
    int32_t                      iKDTIndex)
{
    Kdll_CacheEntry             *pCacheEntryTable;                              // Kernel Cache Entry table
    Kdll_FilterEntry            *pFilter;                                       // Kernel Filter (points to base of filter array)
    int32_t                     iKUID;                                          // Kernel Unique ID (DNDI uses combined kernels)
    int32_t                     iInlineLength;                                  // Inline data length
    MOS_STATUS                  eStatus;                                        // Return code
    PVPHAL_VEBOX_STATE_G9_BASE  pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA    pRenderData = GetLastExecRenderData();

    // Initialize Variables
    eStatus             = MOS_STATUS_SUCCESS;
    pFilter             = &pVeboxState->SearchFilter[0];
    pCacheEntryTable    = pVeboxState->m_pKernelDllState->ComponentKernelCache.pCacheEntries;

    // Initialize States
    MOS_ZeroMemory(pFilter, sizeof(pVeboxState->SearchFilter));
    MOS_ZeroMemory(&pRenderData->KernelEntry[iKDTIndex], sizeof(Kdll_CacheEntry));

#if VEBOX_AUTO_DENOISE_SUPPORTED
    if (iKDTIndex == KERNEL_UPDATEDNSTATE)
    {
        iKUID                = IDR_VP_UpdateDNState;
        iInlineLength        = 0; // No inline data
        pRenderData->PerfTag = VPHAL_VEBOX_UPDATE_DN_STATE;
    }
    else // Incorrect index to kernel parameters array
#endif
    {
        VPHAL_RENDER_ASSERTMESSAGE(
            "Incorrect index to kernel parameters array.");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    // Store pointer to Kernel Parameter
    pRenderData->pKernelParam[iKDTIndex] =
        &pVeboxState->pKernelParamTable[iKDTIndex];

    // Set Parameters for Kernel Entry
    pRenderData->KernelEntry[iKDTIndex].iKUID          = iKUID;
    pRenderData->KernelEntry[iKDTIndex].iKCID          = -1;
    pRenderData->KernelEntry[iKDTIndex].iFilterSize    = 2;
    pRenderData->KernelEntry[iKDTIndex].pFilter        = pFilter;
    pRenderData->KernelEntry[iKDTIndex].iSize          = pCacheEntryTable[iKUID].iSize;
    pRenderData->KernelEntry[iKDTIndex].pBinary        = pCacheEntryTable[iKUID].pBinary;

    // set the Inline Data length
    pRenderData->iInlineLength              = iInlineLength;

    VPHAL_RENDER_NORMALMESSAGE(
        "Vebox Kernels: %s", g_KernelDNDI_Str_g9[iKDTIndex]);

finish:
    return eStatus;
}

//!
//! \brief    Vebox format support check
//! \details  Checks to see if Vebox operation is supported with source surface format
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \return   bool
//!           return true if input surface format is supported, otherwise false
//!
bool VPHAL_VEBOX_STATE_G9_BASE::IsFormatSupported(
    PVPHAL_SURFACE              pSrcSurface)
{
    bool    bRet;

    bRet = false;

    // Check if Sample Format is supported
    // Vebox only support P016 format, P010 format can be supported by faking it as P016
    if (pSrcSurface->Format != Format_NV12 &&
        pSrcSurface->Format != Format_AYUV &&
        pSrcSurface->Format != Format_Y416 &&
        pSrcSurface->Format != Format_P010 &&
        pSrcSurface->Format != Format_P016 &&
        !IS_PA_FORMAT(pSrcSurface->Format))
    {
        VPHAL_RENDER_NORMALMESSAGE("Unsupported Source Format '0x%08x' for VEBOX.", pSrcSurface->Format);
        goto finish;
    }

    bRet = true;

finish:
    return bRet;
}

//!
//! \brief    Vebox format support check
//! \details  Checks to see if RT format is supported when Vebox output pipe is selected
//! \param    [in] pSrcSurface
//!           Pointer to Render source surface of VPP BLT
//! \param    [in] pRTSurface
//!           Pointer to Render target surface of VPP BLT
//! \return   bool
//!           return true if render target surface format is supported, otherwise false
//!
bool VPHAL_VEBOX_STATE_G9_BASE::IsRTFormatSupported(
    PVPHAL_SURFACE              pSrcSurface,
    PVPHAL_SURFACE              pRTSurface)
{
    bool                        bRet;

    bRet = false;

    // Check if RT Format is supported by Vebox
    if (IS_PA_FORMAT(pRTSurface->Format) ||
        pRTSurface->Format == Format_NV12)
    {
        // Supported Vebox Render Target format. Vebox Pipe Output can be selected.
        bRet = true;
    }

    if ((pSrcSurface->ColorSpace == CSpace_BT2020) &&
        ((pSrcSurface->Format == Format_P010)      ||
        (pSrcSurface->Format == Format_P016))      &&
        IS_RGB32_FORMAT(pRTSurface->Format))
    {
        bRet = true;
    }

    return bRet;
}

//!
//! \brief    Vebox format support check for DN
//! \details  Check if the input surface format is supported for DN
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \return   bool
//!           return true if input surface format is supported, otherwise false
//!
bool VPHAL_VEBOX_STATE_G9_BASE::IsDnFormatSupported(
    PVPHAL_SURFACE              pSrcSurface)
{
    bool    bRet;

    bRet = false;
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pSrcSurface);

    if ((pSrcSurface->Format != Format_YUYV)         &&
        (pSrcSurface->Format != Format_VYUY)         &&
        (pSrcSurface->Format != Format_YVYU)         &&
        (pSrcSurface->Format != Format_UYVY)         &&
        (pSrcSurface->Format != Format_YUY2)         &&
        (pSrcSurface->Format != Format_Y8)           &&
        (pSrcSurface->Format != Format_NV12)         &&
        (pSrcSurface->Format != Format_A8B8G8R8)     &&
        (pSrcSurface->Format != Format_A16B16G16R16))
    {
        VPHAL_RENDER_NORMALMESSAGE("Unsupported Format '0x%08x' for VEBOX DN.", pSrcSurface->Format);
        goto finish;
    }

    bRet = true;

finish:
    return bRet;
}

//!
//! \brief    Check if surface format is supported by DI
//! \details  Check if surface format is supported by DI
//! \param    [in] pSrc
//!           Pointer to input surface of Vebox
//! \return   bool
//!           Return true if surface format is supported, otherwise return false
//!
bool VPHAL_VEBOX_STATE_G9_BASE::IsDiFormatSupported(
    PVPHAL_SURFACE              pSrc)
{
    bool bRet = false;

    VPHAL_RENDER_CHK_NULL_NO_STATUS(pSrc);

    if (pSrc->Format != Format_AYUV &&
        pSrc->Format != Format_Y410 &&
        pSrc->Format != Format_Y416 &&
        pSrc->Format != Format_P010 &&
        pSrc->Format != Format_P016 &&
        pSrc->Format != Format_A8B8G8R8 &&
        pSrc->Format != Format_A8R8G8B8 &&
        pSrc->Format != Format_B10G10R10A2 &&
        pSrc->Format != Format_R10G10B10A2 &&
        pSrc->Format != Format_A16B16G16R16 &&
        pSrc->Format != Format_A16R16G16B16)
    {
        bRet = true;
    }
    else
    {
        bRet = false;
    }

finish:
    return bRet;
}

VphalSfcState* VPHAL_VEBOX_STATE_G9_BASE::CreateSfcState()
{
#if __VPHAL_SFC_SUPPORTED
    VphalSfcState *sfcState = MOS_New(VphalSfcStateG9, m_pOsInterface, m_pRenderHal, m_pSfcInterface);
#else
    VphalSfcState *sfcState = nullptr;
#endif

    return sfcState;
}

VPHAL_VEBOX_STATE_G9_BASE::VPHAL_VEBOX_STATE_G9_BASE(
    PMOS_INTERFACE                  pOsInterface,
    PMHW_VEBOX_INTERFACE            pVeboxInterface,
    PMHW_SFC_INTERFACE              pSfcInterface,
    PRENDERHAL_INTERFACE            pRenderHal,
    PVPHAL_VEBOX_EXEC_STATE         pVeboxExecState,
    PVPHAL_RNDR_PERF_DATA           pPerfData,
    const VPHAL_DNDI_CACHE_CNTL     &dndiCacheCntl,
    MOS_STATUS                      *peStatus) :
    VPHAL_VEBOX_STATE(pOsInterface, pVeboxInterface, pSfcInterface, pRenderHal, pVeboxExecState, pPerfData, dndiCacheCntl, peStatus)
{
    // States
    pKernelParamTable                   = (PRENDERHAL_KERNEL_PARAM)g_Vebox_KernelParam_g9;
    iNumFFDISurfaces                    = 2;  // PE on: 4 used. PE off: 2 used

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    m_hvsKernelBinary                   = (uint8_t *)IGVP_HVS_DENOISE_G900;
    m_hvsKernelBinarySize               = IGVP_HVS_DENOISE_G900_SIZE;
#endif
}

