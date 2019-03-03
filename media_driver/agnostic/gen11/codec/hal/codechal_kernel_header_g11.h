/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_kernel_header_g11.h
//! \brief    Gen11 kernel header definitions.
//!

#ifndef __CODECHAL_KERNEL_HEADER_G11_H__
#define __CODECHAL_KERNEL_HEADER_G11_H__

#include "codechal_encoder_base.h"

struct HmeDsScoreboardKernelHeaderG11 {
    int nKernelCount;
    union
    {
        struct
        {
            CODECHAL_KERNEL_HEADER hmeDownscaleGenX0;
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
            CODECHAL_KERNEL_HEADER hmeDownscaleGenX1;
#endif
            CODECHAL_KERNEL_HEADER hmeDownscaleGenX2;
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
            CODECHAL_KERNEL_HEADER hmeDownscaleGenX3;
#endif
            CODECHAL_KERNEL_HEADER hmeP;
            CODECHAL_KERNEL_HEADER hmeB;
            CODECHAL_KERNEL_HEADER hmeVdenc;
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
            CODECHAL_KERNEL_HEADER hmeHevcVdenc;
            CODECHAL_KERNEL_HEADER hmeDetectionGenX0;
#endif
            CODECHAL_KERNEL_HEADER dsConvertGenX0;
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
            CODECHAL_KERNEL_HEADER initSwScoreboard;
            CODECHAL_KERNEL_HEADER intraDistortion;
            CODECHAL_KERNEL_HEADER dynamicScaling;
            CODECHAL_KERNEL_HEADER weightedPrediction;
#endif
        };
    };
};

enum HmeDsScoreboardKernelIdxG11
{
    // HME + Scoreboard Kernel Surface
    CODECHAL_HME_DOWNSCALE_GENX_0_KRNIDX = 0,
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    CODECHAL_HME_DOWNSCALE_GENX_1_KRNIDX,
#endif
    CODECHAL_HME_DOWNSCALE_GENX_2_KRNIDX,
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    CODECHAL_HME_DOWNSCALE_GENX_3_KRNIDX,
#endif
    CODECHAL_HME_GENX_0_KRNIDX,
    CODECHAL_HME_GENX_1_KRNIDX,
    CODECHAL_HME_GENX_2_KRNIDX,
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    CODECHAL_HME_HEVC_VDENC_KRNIDX,
    CODECHAL_HMEDetection_GENX_0_KRNIDX,
#endif
    CODECHAL_DS_CONVERT_GENX_0_KRNIDX,
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    CODECHAL_SW_SCOREBOARD_G11_KRNIDX,
    CODECHAL_INTRA_DISTORTION_G11_KRNIDX,
    CODECHAL_DYNAMIC_SCALING_G11_KRNIDX,
#endif
    CODECHAL_HmeDsSwScoreboardInit_NUM_KRN_G11
};

//!
//! \brief    Get common kernel header and size
//!
//! \param    [in] binary
//!           Kernel binary
//!
//! \param    [in] operation
//!           Encode operation
//!
//! \param    [in] krnStateIdx
//!           Kernel state index
//!
//! \param    [in] krnHeader
//!           Kernel header
//!
//! \param    [in] krnSize
//!           Kernel size
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
static MOS_STATUS GetCommonKernelHeaderAndSizeG11(
    void                            *binary,
    EncOperation                    operation,
    uint32_t                        krnStateIdx,
    void                            *krnHeader,
    uint32_t                        *krnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    HmeDsScoreboardKernelHeaderG11 *kernelHeaderTable;
    kernelHeaderTable = (HmeDsScoreboardKernelHeaderG11*)binary;

    PCODECHAL_KERNEL_HEADER currKrnHeader;
    switch (operation)
    {
    case ENC_SCALING4X:
        currKrnHeader = &kernelHeaderTable->hmeDownscaleGenX0;
        break;
    case ENC_SCALING2X:
        currKrnHeader = &kernelHeaderTable->hmeDownscaleGenX2;
        break;
    case ENC_ME:
        currKrnHeader = &kernelHeaderTable->hmeP;
        break;
    case VDENC_ME_B:
        currKrnHeader = &kernelHeaderTable->hmeB;
        break;
    case VDENC_ME:
        currKrnHeader = &kernelHeaderTable->hmeVdenc;
        break;
    case ENC_SCALING_CONVERSION:
        currKrnHeader = &kernelHeaderTable->dsConvertGenX0;
        break;
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    case VDENC_STREAMIN_HEVC:
        currKrnHeader = &kernelHeaderTable->hmeHevcVdenc;
        break;
    case ENC_SFD:
        currKrnHeader = &kernelHeaderTable->hmeDetectionGenX0;
        break;
    case ENC_SCOREBOARD:
        currKrnHeader = &kernelHeaderTable->initSwScoreboard;
        break;
    case ENC_INTRA_DISTORTION:
        currKrnHeader = &kernelHeaderTable->intraDistortion;
        break;
    case ENC_DYS:
        currKrnHeader = &kernelHeaderTable->dynamicScaling;
        break;
    case ENC_WP:
        currKrnHeader = &kernelHeaderTable->weightedPrediction;
        break;
#endif
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    currKrnHeader += krnStateIdx;
    *((PCODECHAL_KERNEL_HEADER)krnHeader) = *currKrnHeader;

    PCODECHAL_KERNEL_HEADER invalidEntry;
    invalidEntry = &(kernelHeaderTable->hmeDownscaleGenX0) + CODECHAL_HmeDsSwScoreboardInit_NUM_KRN_G11;
    PCODECHAL_KERNEL_HEADER nextKrnHeader;
    nextKrnHeader = (currKrnHeader + 1);
    uint32_t nextKrnOffset;
    nextKrnOffset = *krnSize;

    if (nextKrnHeader < invalidEntry)
    {
        nextKrnOffset = nextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *krnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

#endif  // __CODECHAL_KERNEL_HEADER_G11_H__
