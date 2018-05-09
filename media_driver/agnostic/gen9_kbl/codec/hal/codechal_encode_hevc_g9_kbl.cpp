/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     codechal_encode_hevc_g9_kbl.cpp
//! \brief    HEVC dual-pipe encoder for GEN9 KBL.
//!

#include "codechal_encode_hevc_g9_kbl.h"
#include "igcodeckrn_g9.h"

//! HEVC encoder kernel header structure for G9 KBL
struct CODECHAL_ENC_HEVC_KERNEL_HEADER_G9_KBL
{
    int nKernelCount;                                                       //!< Total number of kernels

    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_I_2xDownSampling_Kernel;             //!< 2x down sampling kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_I_32x32_PU_ModeDecision_Kernel;      //!< Intra 32x32 PU mode decision kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_I_16x16_PU_SADComputation_Kernel;    //!< Intra 16x16 PU SAD computation kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_I_16x16_PU_ModeDecision_Kernel;      //!< Intra 16x16 PU mode decision kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_I_8x8_PU_Kernel;                     //!< Intra 8x8 PU mode decision kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_I_8x8_PU_FMode_Kernel;               //!< Intra 8x8 PU final mode decision kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_PB_32x32_PU_IntraCheck;              //!< P/B 32x32 PU intra mode check kernel
    CODECHAL_KERNEL_HEADER HEVC_LCUEnc_PB_MB;                               //!< P/B MbEnc Kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_I_DS4HME;                            //!< 4x Scaling kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_P_HME;                               //!< P frame HME kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_B_HME;                               //!< B frame HME kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_I_COARSE;                            //!< Intra coarse kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_BRC_Init;                            //!< BRC init kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_BRC_Reset;                           //!< BRC reset kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_BRC_Update;                          //!< BRC update kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_BRC_LCU_Update;                      //!< BRC LCU update kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_PB_Pak;                              //!< P/B frame PAK kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_PB_Adv;                              //!< P/B frame Adv kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_BRC_Blockcopy;                       //!< BRC blockcopy kerenel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_DS_Combined;                         //!< Down scale and format conversion kernel
};

//! \brief  typedef of struct CODECHAL_ENC_HEVC_KERNEL_HEADER_G9_KBL
using PCODECHAL_ENC_HEVC_KERNEL_HEADER_G9_KBL = struct CODECHAL_ENC_HEVC_KERNEL_HEADER_G9_KBL*;

bool CodechalEncHevcStateG9Kbl::CheckSupportedColorFormat(
    PMOS_SURFACE surface)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!surface)
    {
        return false;
    }

    bool isColorFormatSupported = false;
    if (IS_Y_MAJOR_TILE_FORMAT(surface->TileType))
    {
        switch (surface->Format)
        {
        case Format_NV12:
        case Format_P010:       // Planar 4:2:0, 10b
            isColorFormatSupported = true;
            break;

        default:
            break;
        }
    }

    return isColorFormatSupported;
}

MOS_STATUS CodechalEncHevcStateG9Kbl::GetKernelHeaderAndSize(
    void                           *binary,
    EncOperation                   operation,
    uint32_t                       krnStateIdx,
    void                           *krnHeader,
    uint32_t                       *krnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    PCODECHAL_ENC_HEVC_KERNEL_HEADER_G9_KBL kernelHeaderTable = (PCODECHAL_ENC_HEVC_KERNEL_HEADER_G9_KBL)binary;
    PCODECHAL_KERNEL_HEADER currKrnHeader = nullptr;

    if (operation == ENC_SCALING4X)
    {
        currKrnHeader = &kernelHeaderTable->Hevc_LCUEnc_I_DS4HME;
    }
    else if (operation == ENC_ME)
    {
        currKrnHeader = &kernelHeaderTable->Hevc_LCUEnc_B_HME;
    }
    else if (operation == ENC_BRC)
    {
        switch (krnStateIdx)
        {
        case CODECHAL_HEVC_BRC_COARSE_INTRA:
            currKrnHeader = &kernelHeaderTable->Hevc_LCUEnc_I_COARSE;
            break;

        case CODECHAL_HEVC_BRC_INIT:
            currKrnHeader = &kernelHeaderTable->Hevc_LCUEnc_BRC_Init;
            break;

        case CODECHAL_HEVC_BRC_RESET:
            currKrnHeader = &kernelHeaderTable->Hevc_LCUEnc_BRC_Reset;
            break;

        case CODECHAL_HEVC_BRC_FRAME_UPDATE:
            currKrnHeader = &kernelHeaderTable->Hevc_LCUEnc_BRC_Update;
            break;

        case CODECHAL_HEVC_BRC_LCU_UPDATE:
            currKrnHeader = &kernelHeaderTable->Hevc_LCUEnc_BRC_LCU_Update;
            break;

        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported BRC mode requested");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
    }
    else if (operation == ENC_MBENC)
    {
        switch (krnStateIdx)
        {
        case CODECHAL_HEVC_MBENC_2xSCALING:
        case CODECHAL_HEVC_MBENC_32x32MD:
        case CODECHAL_HEVC_MBENC_16x16SAD:
        case CODECHAL_HEVC_MBENC_16x16MD:
        case CODECHAL_HEVC_MBENC_8x8PU:
        case CODECHAL_HEVC_MBENC_8x8FMODE:
        case CODECHAL_HEVC_MBENC_32x32INTRACHECK:
        case CODECHAL_HEVC_MBENC_BENC:
            currKrnHeader = &kernelHeaderTable->Hevc_LCUEnc_I_2xDownSampling_Kernel;
            currKrnHeader += krnStateIdx;
            break;

        case CODECHAL_HEVC_MBENC_BPAK:
            currKrnHeader = &kernelHeaderTable->Hevc_LCUEnc_PB_Pak;
            break;

        case CODECHAL_HEVC_MBENC_ADV:
            currKrnHeader = &kernelHeaderTable->Hevc_LCUEnc_PB_Adv;
            break;

        case CODECHAL_HEVC_MBENC_DS_COMBINED:
            currKrnHeader = &kernelHeaderTable->Hevc_LCUEnc_DS_Combined;
            break;

        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    *((PCODECHAL_KERNEL_HEADER)krnHeader) = *currKrnHeader;

    PCODECHAL_KERNEL_HEADER nextKrnHeader = (currKrnHeader + 1);
    PCODECHAL_KERNEL_HEADER invalidEntry = (PCODECHAL_KERNEL_HEADER)(((uint8_t*)binary) + sizeof(*kernelHeaderTable));
    uint32_t nextKrnOffset = *krnSize;

    if (nextKrnHeader < invalidEntry)
    {
        nextKrnOffset = nextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *krnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9Kbl::SetSequenceStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcStateG9::SetSequenceStructs());

    // KBL HEVC only: turn off Recon's MMC to avoid perf drop at 4K resolution TU4/7
    m_disableReconMMCD = (m_hevcSeqParams->TargetUsage != (uint32_t)TARGETUSAGE_BEST_QUALITY) && m_encode4KSequence;

    return eStatus;
}

CodechalEncHevcStateG9Kbl::CodechalEncHevcStateG9Kbl(
    CodechalHwInterface* hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo)
    :CodechalEncHevcStateG9(hwInterface, debugInterface, standardInfo)
{
    m_kernelBase = (uint8_t *)IGCODECKRN_G9;
    pfnGetKernelHeaderAndSize = GetKernelHeaderAndSize;

    MOS_STATUS eStatus = InitMhw();
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("HEVC encoder MHW initialization failed.");
    }
}

