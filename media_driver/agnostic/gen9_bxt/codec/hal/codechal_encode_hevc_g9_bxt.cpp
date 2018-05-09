/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_encode_hevc_g9_bxt.cpp
//! \brief    HEVC dual-pipe encoder for GEN9 BXT.
//!

#include "codechal_encode_hevc_g9_bxt.h"
//#include "igcodeckrn_g9_bxt.h"
#include "igcodeckrn_g9.h"

//! HEVC encoder kernel header structure for G9 BXT
struct CODECHAL_ENC_HEVC_KERNEL_HEADER_G9_BXT
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
    CODECHAL_KERNEL_HEADER HEVC_LCUEnc_P_MB;                                //!< P frame MbEnc kernel
    CODECHAL_KERNEL_HEADER Hevc_LCUEnc_P_Adv;                               //!< P frame Adv kernel
};

//! \brief  typedef of struct CODECHAL_ENC_HEVC_KERNEL_HEADER_G9_BXT
using PCODECHAL_ENC_HEVC_KERNEL_HEADER_G9_BXT = struct CODECHAL_ENC_HEVC_KERNEL_HEADER_G9_BXT*;

MOS_STATUS CodechalEncHevcStateG9Bxt::GetKernelHeaderAndSize(
    void                           *binary,
    EncOperation                   operation,
    uint32_t                       krnStateIdx,
    void                           *krnHeader,
    uint32_t                       *krnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    PCODECHAL_ENC_HEVC_KERNEL_HEADER_G9_BXT kernelHeaderTable = (PCODECHAL_ENC_HEVC_KERNEL_HEADER_G9_BXT)binary;
    uint32_t nextKrnOffset = *krnSize;

    PCODECHAL_KERNEL_HEADER currKrnHeader = nullptr;
    if (operation == ENC_SCALING2X)
    {
        currKrnHeader = &kernelHeaderTable->Hevc_LCUEnc_I_2xDownSampling_Kernel;
    }
    else if (operation == ENC_SCALING4X)
    {
        currKrnHeader = &kernelHeaderTable->Hevc_LCUEnc_I_DS4HME;
    }
    else if (operation == ENC_ME)
    {
        // Only BXT supports P frame. P HME index CODECHAL_ENCODE_ME_IDX_P is 0 and B HME index CODECHAL_ENCODE_ME_IDX_B is 1
        // Use kernel total number to check if P frame is supported
        // MBEnc kernels + BRC kernels + DownSampling kernel + DS combined kernel
        if (kernelHeaderTable->nKernelCount == (CODECHAL_HEVC_MBENC_NUM_BXT_SKL + CODECHAL_HEVC_BRC_NUM + 2))
        {
            if (krnStateIdx == 0)
            {
                currKrnHeader = &kernelHeaderTable->Hevc_LCUEnc_P_HME;
            }
            else
            {
                currKrnHeader = &kernelHeaderTable->Hevc_LCUEnc_B_HME;
            }
        }
        else
        {
            currKrnHeader = &kernelHeaderTable->Hevc_LCUEnc_B_HME;
        }
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

        case CODECHAL_HEVC_MBENC_PENC:
            currKrnHeader = &kernelHeaderTable->HEVC_LCUEnc_P_MB;
            break;

        case CODECHAL_HEVC_MBENC_ADV_P:
            currKrnHeader = &kernelHeaderTable->Hevc_LCUEnc_P_Adv;
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
    if (nextKrnHeader < invalidEntry)
    {
        nextKrnOffset = nextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *krnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG9Bxt::Initialize(CodechalSetting * settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // common initilization
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcStateG9::Initialize(settings));

    m_cscDsState->EnableMmc();

    return eStatus;
}

CodechalEncHevcStateG9Bxt::CodechalEncHevcStateG9Bxt(
    CodechalHwInterface* hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo)
    :CodechalEncHevcStateG9(hwInterface, debugInterface, standardInfo)
{
    //m_kernelBase = (uint8_t *)IGCODECKRN_G9_BXT;
    m_kernelBase = (uint8_t *)IGCODECKRN_G9; //IGCODECKRN_G9_BXT causes GPU HANG, use IGCODECKRN_G9.
    pfnGetKernelHeaderAndSize = GetKernelHeaderAndSize;
    m_noMeKernelForPFrame = false;

    m_numRegionsInSlice = 2;

    MOS_STATUS eStatus = InitMhw();
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("HEVC encoder MHW initialization failed.");
    }
}

