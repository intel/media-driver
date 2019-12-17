/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     vp_vebox_cmd_packet_g12.cpp
//! \brief    vebox packet which used in by mediapipline.
//! \details  vebox packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

#include "vp_vebox_cmd_packet_g12.h"
#include "vp_utils.h"

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
    96,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
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
    32,  37,  42,  47,  52,  57,  62,  67,  72,  77,  82,  87,  92,  97,  102, 107,
    112, 117, 122, 127, 132, 137, 142, 147, 152, 157, 162, 167, 172, 177, 182, 187,
    192, 198, 204, 210, 216, 222, 228, 234, 240, 246, 252, 258, 264, 270, 276, 282,
    288, 294, 300, 306, 312, 318, 324, 330, 336, 342, 348, 354, 360, 366, 372, 378,
    384 };

const uint32_t   dwPixRangeThreshold1[NOISEFACTOR_MAX + 1] = {
    64,  70,  76,  82,  88,  94,  100, 106, 112, 118, 124, 130, 136, 142, 148, 154,
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
    128,  144,  160,  176,  192,  208,  224,  240,  256,  272,  288,  304,  320,  336,  352,  368,
    384,  400,  416,  432,  448,  464,  480,  496,  512,  528,  544,  560,  576,  592,  608,  624,
    640,  660,  680,  700,  720,  740,  760,  780,  800,  820,  840,  860,  880,  900,  920,  940,
    960,  980,  1000, 1020, 1040, 1060, 1080, 1100, 1120, 1140, 1160, 1180, 1200, 1220, 1240, 1260,
    1280 };

const uint32_t   dwPixRangeThreshold4[NOISEFACTOR_MAX + 1] = {
    128,  152,  176,  200,  224,  248,  272,  296,  320,  344,  368,  392,  416,  440,  464,  488,
    512,  536,  560,  584,  608,  632,  656,  680,  704,  728,  752,  776,  800,  824,  848,  872,
    896,  928,  960,  992,  1024, 1056, 1088, 1120, 1152, 1184, 1216, 1248, 1280, 1312, 1344, 1376,
    1408, 1440, 1472, 1504, 1536, 1568, 1600, 1632, 1664, 1696, 1728, 1760, 1792, 1824, 1856, 1888,
    1920 };

const uint32_t   dwPixRangeThreshold5[NOISEFACTOR_MAX + 1] = {
    128,  164,  200,  236,  272,  308,  344,  380,  416,  452,  488,  524,  560,  596,  632,  668,
    704,  740,  776,  812,  848,  884,  920,  956,  992,  1028, 1064, 1100, 1136, 1172, 1208, 1244,
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
    9,   9,   9,   9,   9,   9,   9,   10,  10,  10,  10,  10,  10,  11,  11,  11,
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

namespace vp {
VpVeboxCmdPacketG12::VpVeboxCmdPacketG12(MediaTask * task, PVP_MHWINTERFACE hwInterface, PVpAllocator &allocator, VPMediaMemComp *mmc)
    :VpVeboxCmdPacket(task, hwInterface, allocator, mmc)
{

}

VpVeboxCmdPacketG12::~VpVeboxCmdPacketG12()
{

}

void VpVeboxCmdPacketG12::GetLumaDefaultValue(
    PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams)
{
    VP_RENDER_ASSERT(pLumaParams);

    pLumaParams->dwDenoiseASDThreshold   = NOISE_ABSSUMTEMPORALDIFF_THRESHOLD_DEFAULT_G12;
    pLumaParams->dwDenoiseHistoryDelta   = NOISE_HISTORY_DELTA_DEFAULT;
    pLumaParams->dwDenoiseMaximumHistory = NOISE_HISTORY_MAX_DEFAULT_G12;
    pLumaParams->dwDenoiseSTADThreshold  = NOISE_SUMABSTEMPORALDIFF_THRESHOLD_DEFAULT_G12;
    pLumaParams->dwDenoiseSCMThreshold   = NOISE_SPATIALCOMPLEXITYMATRIX_THRESHOLD_DEFAULT_G12;
    pLumaParams->dwDenoiseMPThreshold    = NOISE_NUMMOTIONPIXELS_THRESHOLD_DEFAULT_G12;
    pLumaParams->dwLTDThreshold          = NOISE_LOWTEMPORALPIXELDIFF_THRESHOLD_DEFAULT_G12;
    pLumaParams->dwTDThreshold           = NOISE_TEMPORALPIXELDIFF_THRESHOLD_DEFAULT_G12;
}

MOS_STATUS VpVeboxCmdPacketG12::SetDNParams(
    PVPHAL_SURFACE                  pSrcSurface,
    PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams,
    PVPHAL_DNUV_PARAMS              pChromaParams)
{
    MOS_STATUS               eStatus;
    PVPHAL_DENOISE_PARAMS    pDNParams;
    uint32_t                 dwDenoiseFactor;
    PVPHAL_VEBOX_RENDER_DATA pRenderData = GetLastExecRenderData();

    VP_RENDER_ASSERT(pSrcSurface);
    VP_RENDER_ASSERT(pLumaParams);
    VP_RENDER_ASSERT(pChromaParams);
    VP_RENDER_ASSERT(pRenderData);

    eStatus   = MOS_STATUS_SUCCESS;
    pDNParams = pSrcSurface->pDenoiseParams;

    if (!m_PacketCaps.bDN)
    {
        VP_RENDER_NORMALMESSAGE("DN for Output is enabled in Vebox, pls recheck the features enabling in Vebox");
        return MOS_STATUS_INVALID_PARAMETER;
    }

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
        pRenderData->VeboxDNDIParams.dwPixRangeWeight[0] = NOISE_BLF_RANGE_WGTS0_DEFAULT;
        pRenderData->VeboxDNDIParams.dwPixRangeWeight[1] = NOISE_BLF_RANGE_WGTS1_DEFAULT;
        pRenderData->VeboxDNDIParams.dwPixRangeWeight[2] = NOISE_BLF_RANGE_WGTS2_DEFAULT;
        pRenderData->VeboxDNDIParams.dwPixRangeWeight[3] = NOISE_BLF_RANGE_WGTS3_DEFAULT;
        pRenderData->VeboxDNDIParams.dwPixRangeWeight[4] = NOISE_BLF_RANGE_WGTS4_DEFAULT;
        pRenderData->VeboxDNDIParams.dwPixRangeWeight[5] = NOISE_BLF_RANGE_WGTS5_DEFAULT;

        // User specified Denoise strength case (no auto DN detect)
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

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacketG12::SetDIParams(
    PVPHAL_SURFACE pSrcSurface)
{
    PVPHAL_VEBOX_RENDER_DATA pRenderData = GetLastExecRenderData();
    VP_RENDER_CHK_NULL_RETURN(pRenderData);

    if (!m_PacketCaps.bDI)
    {
        VP_RENDER_NORMALMESSAGE("DI for Output is enabled in Vebox, pls recheck the features enabling in Vebox");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Set DI params
    if (pRenderData->bDeinterlace)
    {
        pRenderData->VeboxDNDIParams.dwLumaTDMWeight            = VPHAL_VEBOX_DI_LUMA_TDM_WEIGHT_NATUAL;
        pRenderData->VeboxDNDIParams.dwChromaTDMWeight          = VPHAL_VEBOX_DI_CHROMA_TDM_WEIGHT_NATUAL;
        pRenderData->VeboxDNDIParams.dwSHCMDelta                = VPHAL_VEBOX_DI_SHCM_DELTA_NATUAL;
        pRenderData->VeboxDNDIParams.dwSHCMThreshold            = VPHAL_VEBOX_DI_SHCM_THRESHOLD_NATUAL;
        pRenderData->VeboxDNDIParams.dwSVCMDelta                = VPHAL_VEBOX_DI_SVCM_DELTA_NATUAL;
        pRenderData->VeboxDNDIParams.dwSVCMThreshold            = VPHAL_VEBOX_DI_SVCM_THRESHOLD_NATUAL;
        pRenderData->VeboxDNDIParams.bFasterConvergence         = false;
        pRenderData->VeboxDNDIParams.bTDMLumaSmallerWindow      = false;
        pRenderData->VeboxDNDIParams.bTDMChromaSmallerWindow    = false;
        pRenderData->VeboxDNDIParams.dwLumaTDMCoringThreshold   = VPHAL_VEBOX_DI_LUMA_TDM_CORING_THRESHOLD_NATUAL;
        pRenderData->VeboxDNDIParams.dwChromaTDMCoringThreshold = VPHAL_VEBOX_DI_CHROMA_TDM_CORING_THRESHOLD_NATUAL;
        pRenderData->VeboxDNDIParams.bBypassDeflickerFilter     = true;
        pRenderData->VeboxDNDIParams.bUseSyntheticContentMedian = false;
        pRenderData->VeboxDNDIParams.bLocalCheck                = true;
        pRenderData->VeboxDNDIParams.bSyntheticContentCheck     = false;
        pRenderData->VeboxDNDIParams.dwDirectionCheckThreshold  = VPHAL_VEBOX_DI_DIRECTION_CHECK_THRESHOLD_NATUAL;
        pRenderData->VeboxDNDIParams.dwTearingLowThreshold      = VPHAL_VEBOX_DI_TEARING_LOW_THRESHOLD_NATUAL;
        pRenderData->VeboxDNDIParams.dwTearingHighThreshold     = VPHAL_VEBOX_DI_TEARING_HIGH_THRESHOLD_NATUAL;
        pRenderData->VeboxDNDIParams.dwDiffCheckSlackThreshold  = VPHAL_VEBOX_DI_DIFF_CHECK_SLACK_THRESHOLD_NATUAL;
        pRenderData->VeboxDNDIParams.dwSADWT0                   = VPHAL_VEBOX_DI_SAD_WT0_NATUAL;
        pRenderData->VeboxDNDIParams.dwSADWT1                   = VPHAL_VEBOX_DI_SAD_WT1_NATUAL;
        pRenderData->VeboxDNDIParams.dwSADWT2                   = VPHAL_VEBOX_DI_SAD_WT2_NATUAL;
        pRenderData->VeboxDNDIParams.dwSADWT3                   = VPHAL_VEBOX_DI_SAD_WT3_NATUAL;
        pRenderData->VeboxDNDIParams.dwSADWT4                   = VPHAL_VEBOX_DI_SAD_WT4_NATUAL;
        pRenderData->VeboxDNDIParams.dwSADWT6                   = VPHAL_VEBOX_DI_SAD_WT6_NATUAL;
        if (pSrcSurface && pSrcSurface->pDeinterlaceParams)
        {
            pRenderData->VeboxDNDIParams.bSCDEnable             = pSrcSurface->pDeinterlaceParams->bSCDEnable;
        }
        else
        {
            pRenderData->VeboxDNDIParams.bSCDEnable             = false;
        }

        VP_RENDER_CHK_NULL_RETURN(pSrcSurface);
        if (MEDIA_IS_HDCONTENT(pSrcSurface->dwWidth, pSrcSurface->dwHeight))
        {
            pRenderData->VeboxDNDIParams.dwLPFWtLUT0 = VPHAL_VEBOX_DI_LPFWTLUT0_HD_NATUAL;
            pRenderData->VeboxDNDIParams.dwLPFWtLUT1 = VPHAL_VEBOX_DI_LPFWTLUT1_HD_NATUAL;
            pRenderData->VeboxDNDIParams.dwLPFWtLUT2 = VPHAL_VEBOX_DI_LPFWTLUT2_HD_NATUAL;
            pRenderData->VeboxDNDIParams.dwLPFWtLUT3 = VPHAL_VEBOX_DI_LPFWTLUT3_HD_NATUAL;
            pRenderData->VeboxDNDIParams.dwLPFWtLUT4 = VPHAL_VEBOX_DI_LPFWTLUT4_HD_NATUAL;
            pRenderData->VeboxDNDIParams.dwLPFWtLUT5 = VPHAL_VEBOX_DI_LPFWTLUT5_HD_NATUAL;
            pRenderData->VeboxDNDIParams.dwLPFWtLUT6 = VPHAL_VEBOX_DI_LPFWTLUT6_HD_NATUAL;
            pRenderData->VeboxDNDIParams.dwLPFWtLUT7 = VPHAL_VEBOX_DI_LPFWTLUT7_HD_NATUAL;
        }
        else
        {
            pRenderData->VeboxDNDIParams.dwLPFWtLUT0 = VPHAL_VEBOX_DI_LPFWTLUT0_SD_NATUAL;
            pRenderData->VeboxDNDIParams.dwLPFWtLUT1 = VPHAL_VEBOX_DI_LPFWTLUT1_SD_NATUAL;
            pRenderData->VeboxDNDIParams.dwLPFWtLUT2 = VPHAL_VEBOX_DI_LPFWTLUT2_SD_NATUAL;
            pRenderData->VeboxDNDIParams.dwLPFWtLUT3 = VPHAL_VEBOX_DI_LPFWTLUT3_SD_NATUAL;
            pRenderData->VeboxDNDIParams.dwLPFWtLUT4 = VPHAL_VEBOX_DI_LPFWTLUT4_SD_NATUAL;
            pRenderData->VeboxDNDIParams.dwLPFWtLUT5 = VPHAL_VEBOX_DI_LPFWTLUT5_SD_NATUAL;
            pRenderData->VeboxDNDIParams.dwLPFWtLUT6 = VPHAL_VEBOX_DI_LPFWTLUT6_SD_NATUAL;
            pRenderData->VeboxDNDIParams.dwLPFWtLUT7 = VPHAL_VEBOX_DI_LPFWTLUT7_SD_NATUAL;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacketG12::SetDNDIParams(
    PVPHAL_SURFACE                  pSrcSurface,
    PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams,
    PVPHAL_DNUV_PARAMS              pChromaParams)
{
    MOS_STATUS status;

    VP_RENDER_ASSERT(pSrcSurface);
    VP_RENDER_ASSERT(pLumaParams);
    VP_RENDER_ASSERT(pChromaParams);

    status = MOS_STATUS_SUCCESS;

    if (!m_PacketCaps.bVebox)
    {
        VP_RENDER_NORMALMESSAGE("DN/DI for Output is enabled in Vebox, pls recheck the features enabling in Vebox");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    status = SetDNParams(pSrcSurface, pLumaParams, pChromaParams);

    MOS_STATUS status2 = SetDIParams(pSrcSurface);

    if (MOS_SUCCEEDED(status))
    {
        status = status2;
    }

    return status;
}

//!
//! \brief    Setup Vebox_DI_IECP Command params
//! \details  Setup Vebox_DI_IECP Command params
//! \param    [in] bDiScdEnable
//!           Is DI/Variances report enabled
//! \param    [in,out] pVeboxDiIecpCmdParams
//!           Pointer to VEBOX_DI_IECP command parameters
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpVeboxCmdPacketG12::SetupDiIecpState(
    bool                          bDiScdEnable,
    PMHW_VEBOX_DI_IECP_CMD_PARAMS pVeboxDiIecpCmdParams)
{
    return VpVeboxCmdPacket::SetupDiIecpState(bDiScdEnable, pVeboxDiIecpCmdParams);
}

//!
//! \brief    Vebox query statistics surface layout
//! \details  Get Specific Layout Info like GNE Offset, size of per frame info inside
//!           Vebox Statistics Surface for CNL
//!
//!           | Layout of Statistics surface when DI enabled and DN either On or Off on CNL\n
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
MOS_STATUS VpVeboxCmdPacketG12::QueryStatLayout(
    VEBOX_STAT_QUERY_TYPE QueryType,
    uint32_t*             pQuery)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_ASSERT(pQuery);

    switch (QueryType)
    {
        case VEBOX_STAT_QUERY_GNE_OFFEST:
            *pQuery = VPHAL_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_G12;
            break;

        case VEBOX_STAT_QUERY_PER_FRAME_SIZE:
            *pQuery = VPHAL_VEBOX_STATISTICS_PER_FRAME_SIZE_G12;
            break;

        case VEBOX_STAT_QUERY_FMD_OFFEST:
            *pQuery = VPHAL_VEBOX_STATISTICS_SURFACE_FMD_OFFSET_G12;
            break;

        case VEBOX_STAT_QUERY_STD_OFFEST:
            *pQuery = VPHAL_VEBOX_STATISTICS_SURFACE_STD_OFFSET_G12;
            break;

        default:
            VP_RENDER_ASSERTMESSAGE("Vebox Statistics Layout Query, type ('%d') is not implemented.", QueryType);
            eStatus = MOS_STATUS_UNKNOWN;
            break;
    }

    return eStatus;
}

}
