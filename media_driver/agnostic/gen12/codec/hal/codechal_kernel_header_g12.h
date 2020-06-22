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
//! \file     codechal_kernel_header_g12.h
//! \brief    Gen12 kernel header definitions.
//!

#ifndef __CODECHAL_KERNEL_HEADER_G12_H__
#define __CODECHAL_KERNEL_HEADER_G12_H__

#include "codechal_encoder_base.h"

struct HmeDsScoreboardKernelHeader {
    int nKernelCount;
    union
    {
        struct
        {
            CODECHAL_KERNEL_HEADER hmeDownscaleGenX0;
            CODECHAL_KERNEL_HEADER hmeDownscaleGenX1;
            CODECHAL_KERNEL_HEADER hmeDownscaleGenX2;
            CODECHAL_KERNEL_HEADER hmeDownscaleGenX3;
            CODECHAL_KERNEL_HEADER hmeGenX0;
            CODECHAL_KERNEL_HEADER hmeGenX1;
            CODECHAL_KERNEL_HEADER hmeGenX2;
            CODECHAL_KERNEL_HEADER hmeHevcVdenc;
            CODECHAL_KERNEL_HEADER hmeHevcVdencRAB;
            CODECHAL_KERNEL_HEADER hmeDetectionGenX0;
            CODECHAL_KERNEL_HEADER dsConvertGenX0;
            CODECHAL_KERNEL_HEADER initSwScoreboard;
            CODECHAL_KERNEL_HEADER intraDistortion;
            CODECHAL_KERNEL_HEADER dynamicScaling;
        };
    };
};

enum HmeDsScoreboardKernelIdx
{
    // HME + Scoreboard Kernel Surface
    CODECHAL_HME_DOWNSCALE_GEN12_0_KRNIDX = 0,
    CODECHAL_HME_DOWNSCALE_GEN12_1_KRNIDX,
    CODECHAL_HME_DOWNSCALE_GEN12_2_KRNIDX,
    CODECHAL_HME_DOWNSCALE_GEN12_3_KRNIDX,
    CODECHAL_HME_GEN12_0_KRNIDX,
    CODECHAL_HME_GEN12_1_KRNIDX,
    CODECHAL_HME_GEN12_2_KRNIDX,
    CODECHAL_HME_HEVC_VDENC__GEN12_KRNIDX,
    CODECHAL_HME_HEVC_VDENC_RAB_GEN12_KRNIDX,
    CODECHAL_HMEDetection_GEN12_0_KRNIDX,
    CODECHAL_DS_CONVERT_GEN12_0_KRNIDX,
    CODECHAL_SW_SCOREBOARD_G12_KRNIDX,
    CODECHAL_INTRA_DISTORTION_G12_KRNIDX,
    CODECHAL_DYNAMIC_SCALING_G12_KRNIDX,
    CODECHAL_HmeDsSwScoreboardInit_NUM_KRN_G12
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
static MOS_STATUS GetCommonKernelHeaderAndSizeG12(
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

    HmeDsScoreboardKernelHeader *kernelHeaderTable;
    kernelHeaderTable = (HmeDsScoreboardKernelHeader*)binary;
    PCODECHAL_KERNEL_HEADER invalidEntry;
    invalidEntry = &(kernelHeaderTable->dynamicScaling) + 1;

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
            currKrnHeader = &kernelHeaderTable->hmeGenX0;
            break;
        case VDENC_ME_P:
            currKrnHeader = &kernelHeaderTable->hmeGenX0;
            break;
        case VDENC_ME_B:
            currKrnHeader = &kernelHeaderTable->hmeGenX1;
            break;
        case VDENC_ME:
            currKrnHeader = &kernelHeaderTable->hmeGenX2;
            break;
        case VDENC_STREAMIN_HEVC:
            currKrnHeader = &kernelHeaderTable->hmeHevcVdenc;
            break;
        case VDENC_STREAMIN_HEVC_RAB:
            currKrnHeader = &kernelHeaderTable->hmeHevcVdencRAB;
            break;
        case ENC_SFD:
            currKrnHeader = &kernelHeaderTable->hmeDetectionGenX0;
            break;
        case ENC_SCALING_CONVERSION:
            currKrnHeader = &kernelHeaderTable->dsConvertGenX0;
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
        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
    }

    currKrnHeader += krnStateIdx;
    *((PCODECHAL_KERNEL_HEADER)krnHeader) = *currKrnHeader;

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

#endif  // __CODECHAL_KERNEL_HEADER_G12_H__
