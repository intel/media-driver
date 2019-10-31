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
//! \file     codechal_encode_hevc_mbenc_g12.cpp
//! \brief    HEVC dual-pipe encoder mbenc kernels for GEN12.
//!

#include "codechal_encode_hevc_mbenc_g12.h"
#include "codechal_encode_hevc_brc_g12.h"
#include "codechal_encode_hevc_g12.h"
#include "mhw_vdbox_hcp_g12_X.h"
#include "Gen12_HEVC_B_LCU32.h"
#include "Gen12_HEVC_B_LCU64.h"
#include "cm_wrapper.h"

#include "Gen12_HEVC_BRC_INIT.h"
#include "Gen12_HEVC_BRC_RESET.h"
#include "Gen12_HEVC_BRC_UPDATE.h"
#include "Gen12_HEVC_BRC_LCUQP.h"

#include "codechal_debug.h"

#if USE_PROPRIETARY_CODE
#include "cm_device_rt.h"
#endif

#if MOS_MEDIASOLO_SUPPORTED
#include "mos_os_solo.h"
#endif // (_DEBUG || _RELEASE_INTERNAL)

MOS_STATUS CodecHalHevcBrcG12::AllocateBrcResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodecHalHevcBrcG12::FreeBrcResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    // Destroy the container for BRC buffers
    if (m_histBufferBrc)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->DestroySurface(m_histBufferBrc));
        m_histBufferBrc = nullptr;
    }
    if (m_PAKStatsBufferBrc)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->DestroySurface(m_PAKStatsBufferBrc));
        m_PAKStatsBufferBrc = nullptr;
    }
    if (m_PICStateInBufferBrc)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->DestroySurface(m_PICStateInBufferBrc));
        m_PICStateInBufferBrc = nullptr;
    }
    if (m_PICStateOutBufferBrc)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->DestroySurface(m_PICStateOutBufferBrc));
        m_PICStateOutBufferBrc = nullptr;
    }
    if (m_CombinedEncBufferBrc)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->DestroySurface(m_CombinedEncBufferBrc));
        m_CombinedEncBufferBrc = nullptr;
    }
    if (m_PixelMBStatsBufferBrc)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->DestroySurface(m_PixelMBStatsBufferBrc));
        m_PixelMBStatsBufferBrc = nullptr;
    }
    if (m_ConstDataBufferBRC)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->DestroySurface(m_ConstDataBufferBRC));
        m_ConstDataBufferBRC = nullptr;
    }
    if (m_BrcMbQp)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->DestroySurface(m_BrcMbQp));
        m_BrcMbQp = nullptr;
    }
    if (m_BrcROISurf)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->DestroySurface(m_BrcROISurf));
        m_BrcROISurf = nullptr;
    }

    if (m_cmKrnBrc)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->DestroyKernel(m_cmKrnBrc));
        m_cmKrnBrc = nullptr;
    }
    if (m_cmProgramBrc)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->DestroyProgram(m_cmProgramBrc));
        m_cmProgramBrc = nullptr;
    }
    if (m_cmKrnBrcUpdate)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->DestroyKernel(m_cmKrnBrcUpdate));
        m_cmKrnBrcUpdate = nullptr;
    }
    if (m_cmProgramBrcUpdate)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->DestroyProgram(m_cmProgramBrcUpdate));
        m_cmProgramBrcUpdate = nullptr;
    }
    if (m_cmKrnBrcLCUQP)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->DestroyKernel(m_cmKrnBrcLCUQP));
        m_cmKrnBrcLCUQP = nullptr;
    }
    if (m_cmProgramBrcLCUQP)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->DestroyProgram(m_cmProgramBrcLCUQP));
        m_cmProgramBrcLCUQP = nullptr;
    }

    return eStatus;
}

MOS_STATUS CodecHalHevcBrcG12::InitBrcKernelState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (encoderBrc->m_brcInit) {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->LoadProgram((void *)HEVC_BRC_INIT_GENX,
            HEVC_BRC_INIT_GENX_SIZE,
            m_cmProgramBrc,
            "-nojitter"));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->CreateKernel(m_cmProgramBrc,
            "HEVC_brc_init",
            m_cmKrnBrc));
    }
    else if (encoderBrc->m_brcReset) {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->LoadProgram((void *)HEVC_BRC_RESET_GENX,
            HEVC_BRC_RESET_GENX_SIZE,
            m_cmProgramBrc,
            "-nojitter"));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->CreateKernel(m_cmProgramBrc,
            "HEVC_brc_reset",
            m_cmKrnBrc));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->LoadProgram((void *)HEVC_BRC_UPDATE_GENX,
        HEVC_BRC_UPDATE_GENX_SIZE,
        m_cmProgramBrcUpdate,
        "-nojitter"));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->CreateKernel(m_cmProgramBrcUpdate,
        "HEVC_brc_update",
        m_cmKrnBrcUpdate));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->LoadProgram((void *)HEVC_BRC_LCUQP_GENX,
        HEVC_BRC_LCUQP_GENX_SIZE,
        m_cmProgramBrcLCUQP,
        "-nojitter"));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->CreateKernel(m_cmProgramBrcLCUQP,
        "HEVC_brc_lcuqp",
        m_cmKrnBrcLCUQP));

    return eStatus;
}
MOS_STATUS CodecHalHevcBrcG12::SetupKernelArgsBrcInit()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    //Setup surfaces

    int idx = 0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrc->SetKernelArg(idx++, sizeof(encoderBrc->curbe), &encoderBrc->curbe));

    SurfaceIndex *pIndex0 = nullptr;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_histBufferBrc->GetIndex(pIndex0));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrc->SetKernelArg(idx++, sizeof(SurfaceIndex), pIndex0));

    //Setup Distortion 2D surface
    CmSurface2D *brcDistortion = (encoderBrc->m_pictureCodingType == I_TYPE) ? encoderBrc->m_brcBuffers.brcIntraDistortionSurface
                                                                             : encoderBrc->m_brcBuffers.meBrcDistortionSurface;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(brcDistortion->GetIndex(pIndex0));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrc->SetKernelArg(idx++, sizeof(SurfaceIndex), pIndex0));

    return eStatus;
}

MOS_STATUS CodecHalHevcBrcG12::SetupThreadSpace(CmKernel *cmKernel, CmThreadSpace * threadSpace)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKernel->SetThreadCount(1));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->CreateThreadSpace(1, 1, threadSpace));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKernel->AssociateThreadSpace(threadSpace));
    return eStatus;
}

MOS_STATUS CodecHalHevcBrcG12::SetupBrcLcuqpThreadSpace(CmKernel *cmKernel, CmThreadSpace * threadSpace)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    uint32_t xThread = (encoderBrc->m_downscaledWidthInMb4x * SCALE_FACTOR_4x + 15) >> 4;
    uint32_t yThread = (encoderBrc->m_downscaledHeightInMb4x * SCALE_FACTOR_4x + 7) >> 3;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKernel->SetThreadCount(xThread * yThread));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->CreateThreadSpace(xThread, yThread, threadSpace));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKernel->AssociateThreadSpace(threadSpace));

    return eStatus;
}

MOS_STATUS CodecHalHevcBrcG12::InitCurbeDataBrcInit()
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    return eStatus;
}

MOS_STATUS CodecHalHevcBrcG12::SetupSurfacesBrcInit()
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    if (!m_histBufferBrc)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->CreateBuffer(
            &encoderBrc->m_brcBuffers.resBrcHistoryBuffer, //m_brcHistoryBufferSize,
            m_histBufferBrc));
    }

    return eStatus;
}

MOS_STATUS CodecHalHevcBrcG12::EncodeBrcInitResetKernel()
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupThreadSpace(m_cmKrnBrc, m_threadSpaceBrcInit));

    // Setup curbe for BrcInitReset kernel
    CODECHAL_HEVC_BRC_KRNIDX brcKrnIdx = encoderBrc->m_brcInit ? CODECHAL_HEVC_BRC_INIT : CODECHAL_HEVC_BRC_RESET;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(BrcInitResetCurbe(
        brcKrnIdx));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupSurfacesBrcInit());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupKernelArgsBrcInit());

    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmTask->AddKernel(m_cmKrnBrc));

    if (!encoderBrc->m_singleTaskPhaseSupported || encoderBrc->m_lastTaskInPhase)
    {
        CmEvent * event = CM_NO_EVENT;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmQueue->EnqueueFast(encoderBrc->m_cmTask, event));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmTask->Reset());

        encoderBrc->m_lastTaskInPhase = false;
    }
    else
    {
        encoderBrc->m_cmTask->AddSync();
    }

    encoderBrc->m_brcInit = encoderBrc->m_brcReset = false;
    // End FW
    return eStatus;
}

MOS_STATUS CodecHalHevcBrcG12::BrcInitResetCurbe(CODECHAL_HEVC_BRC_KRNIDX  brcKrnIdx)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    // Initialize the CURBE data
    encoderBrc->curbe = encoderBrc->m_brcInitResetCurbeInit;

    uint32_t   profileLevelMaxFrame = encoderBrc->GetProfileLevelMaxFrameSize();

    if (encoderBrc->m_hevcSeqParams->RateControlMethod == RATECONTROL_CBR ||
        encoderBrc->m_hevcSeqParams->RateControlMethod == RATECONTROL_VBR ||
        encoderBrc->m_hevcSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        if (encoderBrc->m_hevcSeqParams->InitVBVBufferFullnessInBit == 0)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Initial VBV Buffer Fullness is zero\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (encoderBrc->m_hevcSeqParams->VBVBufferSizeInBit == 0)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("VBV buffer size in bits is zero\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    encoderBrc->curbe.DW0_ProfileLevelMaxFrame = profileLevelMaxFrame;
    encoderBrc->curbe.DW1_InitBufFull = encoderBrc->m_hevcSeqParams->InitVBVBufferFullnessInBit;
    encoderBrc->curbe.DW2_BufSize = encoderBrc->m_hevcSeqParams->VBVBufferSizeInBit;
    encoderBrc->curbe.DW3_TargetBitRate = encoderBrc->m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;    //DDI in Kbits
    encoderBrc->curbe.DW4_MaximumBitRate = encoderBrc->m_hevcSeqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS;
    encoderBrc->curbe.DW5_MinimumBitRate = 0;
    encoderBrc->curbe.DW6_FrameRateM = encoderBrc->m_hevcSeqParams->FrameRate.Numerator;
    encoderBrc->curbe.DW7_FrameRateD = encoderBrc->m_hevcSeqParams->FrameRate.Denominator;
    encoderBrc->curbe.DW8_BRCFlag = encoderBrc->BRCINIT_IGNORE_PICTURE_HEADER_SIZE;  // always ignore the picture header size set in BRC Update curbe;

    if (encoderBrc->m_hevcPicParams->NumROI)
    {
        encoderBrc->curbe.DW8_BRCFlag |= encoderBrc->BRCINIT_DISABLE_MBBRC; // BRC ROI need disable MBBRC logic in LcuBrc Kernel
    }
    else
    {
        encoderBrc->curbe.DW8_BRCFlag |= (encoderBrc->m_lcuBrcEnabled) ? 0 : encoderBrc->BRCINIT_DISABLE_MBBRC;
    }

    encoderBrc->curbe.DW8_BRCFlag |= (encoderBrc->m_brcEnabled && encoderBrc->m_numPipe > 1) ? encoderBrc->BRCINIT_USEHUCBRC : 0;
    encoderBrc->curbe.DW8_BRCFlag |= (encoderBrc->m_enableFramePanicMode) ? encoderBrc->BRCINIT_PANIC_MODE_ISENABLED : 0;

    // For non-ICQ, ACQP Buffer always set to 1
    encoderBrc->curbe.DW25_ACQPBuffer = 1;

    encoderBrc->curbe.DW25_SlidingWindowSize = encoderBrc->m_slidingWindowSize;

    if (encoderBrc->m_hevcSeqParams->RateControlMethod == RATECONTROL_CBR)
    {
        encoderBrc->curbe.DW4_MaximumBitRate = encoderBrc->curbe.DW3_TargetBitRate;
        encoderBrc->curbe.DW8_BRCFlag |= encoderBrc->BRCINIT_ISCBR;
    }
    else if (encoderBrc->m_hevcSeqParams->RateControlMethod == RATECONTROL_VBR)
    {
        if (encoderBrc->curbe.DW4_MaximumBitRate < encoderBrc->curbe.DW3_TargetBitRate)
        {
            encoderBrc->curbe.DW4_MaximumBitRate = 2 * encoderBrc->curbe.DW3_TargetBitRate;
        }
        encoderBrc->curbe.DW8_BRCFlag |= encoderBrc->BRCINIT_ISVBR;
    }
    else if (encoderBrc->m_hevcSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        encoderBrc->curbe.DW8_BRCFlag |= encoderBrc->BRCINIT_ISAVBR;
        // For AVBR, max bitrate = target bitrate,
        encoderBrc->curbe.DW3_TargetBitRate = encoderBrc->m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;    //DDI in Kbits
        encoderBrc->curbe.DW4_MaximumBitRate = encoderBrc->m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;
    }
    else if (encoderBrc->m_hevcSeqParams->RateControlMethod == RATECONTROL_ICQ)
    {
        encoderBrc->curbe.DW8_BRCFlag |= encoderBrc->BRCINIT_ISICQ;
        encoderBrc->curbe.DW25_ACQPBuffer = encoderBrc->m_hevcSeqParams->ICQQualityFactor;
    }
    else if (encoderBrc->m_hevcSeqParams->RateControlMethod == RATECONTROL_VCM)
    {
        encoderBrc->curbe.DW4_MaximumBitRate = encoderBrc->curbe.DW3_TargetBitRate;
        encoderBrc->curbe.DW8_BRCFlag |= encoderBrc->BRCINIT_ISVCM;
    }
    else if (encoderBrc->m_hevcSeqParams->RateControlMethod == RATECONTROL_CQP)
    {
        encoderBrc->curbe.DW8_BRCFlag = encoderBrc->BRCINIT_ISCQP;
    }
    else if (encoderBrc->m_hevcSeqParams->RateControlMethod == RATECONTROL_QVBR)
    {
        if (encoderBrc->curbe.DW4_MaximumBitRate < encoderBrc->curbe.DW3_TargetBitRate)
        {
            encoderBrc->curbe.DW4_MaximumBitRate = encoderBrc->curbe.DW3_TargetBitRate; // Use max bit rate for HRD compliance
        }
        encoderBrc->curbe.DW8_BRCFlag = encoderBrc->curbe.DW8_BRCFlag | encoderBrc->BRCINIT_ISQVBR | encoderBrc->BRCINIT_ISVBR; // We need to make sure that VBR is used for QP determination.
        // use ICQQualityFactor to determine the larger Qp for each MB
        encoderBrc->curbe.DW25_ACQPBuffer = encoderBrc->m_hevcSeqParams->ICQQualityFactor;
    }

    encoderBrc->curbe.DW9_FrameWidth = encoderBrc->m_oriFrameWidth;
    encoderBrc->curbe.DW10_FrameHeight = encoderBrc->m_oriFrameHeight;
    encoderBrc->curbe.DW10_AVBRAccuracy = encoderBrc->m_usAvbrAccuracy;
    encoderBrc->curbe.DW11_AVBRConvergence = encoderBrc->m_usAvbrConvergence;
    encoderBrc->curbe.DW12_NumberSlice = encoderBrc->m_numSlices;

    /**********************************************************************
    In case of non-HB/BPyramid Structure
    BRC_Param_A = GopP
    BRC_Param_B = GopB
    In case of HB/BPyramid GOP Structure
    BRC_Param_A, BRC_Param_B, BRC_Param_C, BRC_Param_D are
    BRC Parameters set as follows as per CModel equation
    ***********************************************************************/
    // BPyramid GOP
    const auto GopPicSize = encoderBrc->m_hevcSeqParams->GopPicSize;
    const auto GopRefDist = encoderBrc->m_hevcSeqParams->GopRefDist;

    encoderBrc->m_HierchGopBRCEnabled = false;
    if(encoderBrc->m_hevcSeqParams->HierarchicalFlag && GopRefDist > 1 && GopRefDist <= 8 )
    {      
        uint32_t numB[9]      = {0, 0, 1, 1, 1, 1, 1, 1, 1};
        uint32_t numB1[9]     = {0, 0, 0, 1, 2, 2, 2, 2, 2};
        uint32_t numB2[9]     = {0, 0, 0, 0, 0, 1, 2, 3, 4};

        uint32_t numOfPyramid = (GopPicSize - 1) / GopRefDist;
        uint32_t remOfPyramid = (GopPicSize - 1) % GopRefDist;

        encoderBrc->curbe.DW8_BRCGopP   = numOfPyramid * numB[GopRefDist] + numB[remOfPyramid + 1];
        encoderBrc->curbe.DW9_BRCGopB   = numOfPyramid * numB[GopRefDist] + numB[remOfPyramid];
        encoderBrc->curbe.DW13_BRCGopB1 = numOfPyramid * numB1[GopRefDist] + numB1[remOfPyramid];
        encoderBrc->curbe.DW14_BRCGopB2 = numOfPyramid * numB2[GopRefDist] + numB2[remOfPyramid];

        encoderBrc->m_HierchGopBRCEnabled = true;
     
        // B1 Level GOP
        if (GopRefDist <= 4 || encoderBrc->curbe.DW14_BRCGopB2 == 0)
        {
            encoderBrc->curbe.DW14_MaxBRCLevel = 3;
        }
        // B2 Level GOP
        else
        {
            encoderBrc->curbe.DW14_MaxBRCLevel = 4;
        }       
    }
    // For Regular GOP - No BPyramid
    else
    {
        encoderBrc->curbe.DW14_MaxBRCLevel = 1;
        encoderBrc->curbe.DW8_BRCGopP      = (GopRefDist) ? MOS_ROUNDUP_DIVIDE(GopPicSize - 1, GopRefDist) : 0;
        encoderBrc->curbe.DW9_BRCGopB      = GopPicSize - 1 - encoderBrc->curbe.DW8_BRCGopP;
    }

    // Set dynamic thresholds
    double inputBitsPerFrame = (double)((double)encoderBrc->curbe.DW4_MaximumBitRate * (double)encoderBrc->curbe.DW7_FrameRateD);
    inputBitsPerFrame = (double)(inputBitsPerFrame / encoderBrc->curbe.DW6_FrameRateM);

    if (encoderBrc->curbe.DW2_BufSize < (uint32_t)inputBitsPerFrame * 4)
    {
        encoderBrc->curbe.DW2_BufSize = (uint32_t)inputBitsPerFrame * 4;
    }

    if (encoderBrc->curbe.DW1_InitBufFull == 0)
    {
        encoderBrc->curbe.DW1_InitBufFull = 7 * encoderBrc->curbe.DW2_BufSize / 8;
    }
    if (encoderBrc->curbe.DW1_InitBufFull < (uint32_t)(inputBitsPerFrame * 2))
    {
        encoderBrc->curbe.DW1_InitBufFull = (uint32_t)(inputBitsPerFrame * 2);
    }
    if (encoderBrc->curbe.DW1_InitBufFull > encoderBrc->curbe.DW2_BufSize)
    {
        encoderBrc->curbe.DW1_InitBufFull = encoderBrc->curbe.DW2_BufSize;
    }

    if (encoderBrc->m_hevcSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        // For AVBR, Buffer size =  2*Bitrate, InitVBV = 0.75 * BufferSize
        encoderBrc->curbe.DW2_BufSize = 2 * encoderBrc->m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;
        encoderBrc->curbe.DW1_InitBufFull = (uint32_t)(0.75 * encoderBrc->curbe.DW2_BufSize);
    }

    if (encoderBrc->m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)
    {
        encoderBrc->curbe.DW15_LongTermInterval = 0; // no LTR for low delay brc
    }
    else
    {
        encoderBrc->curbe.DW15_LongTermInterval = (encoderBrc->m_enableBrcLTR && encoderBrc->m_ltrInterval) ?
                                                   encoderBrc->m_ltrInterval : encoderBrc->m_enableBrcLTR ? HEVC_BRC_LONG_TERM_REFRENCE_FLAG : 0;
    }

    double bpsRatio = ( (double) inputBitsPerFrame / (( (double) encoderBrc->curbe.DW2_BufSize) / 30));
    bpsRatio = (bpsRatio < 0.1) ? 0.1 : (bpsRatio > 3.5) ? 3.5 : bpsRatio;

    encoderBrc->curbe.DW19_DeviationThreshold0_PBframe = (uint32_t)(-50 * pow(0.90, bpsRatio));
    encoderBrc->curbe.DW19_DeviationThreshold1_PBframe = (uint32_t)(-50 * pow(0.66, bpsRatio));
    encoderBrc->curbe.DW19_DeviationThreshold2_PBframe = (uint32_t)(-50 * pow(0.46, bpsRatio));
    encoderBrc->curbe.DW19_DeviationThreshold3_PBframe = (uint32_t)(-50 * pow(0.3, bpsRatio));

    encoderBrc->curbe.DW20_DeviationThreshold4_PBframe = (uint32_t)(50 * pow(0.3, bpsRatio));
    encoderBrc->curbe.DW20_DeviationThreshold5_PBframe = (uint32_t)(50 * pow(0.46, bpsRatio));
    encoderBrc->curbe.DW20_DeviationThreshold6_PBframe = (uint32_t)(50 * pow(0.7, bpsRatio));
    encoderBrc->curbe.DW20_DeviationThreshold7_PBframe = (uint32_t)(50 * pow(0.9, bpsRatio));

    encoderBrc->curbe.DW21_DeviationThreshold0_VBRcontrol = (uint32_t)(-50 * pow(0.9, bpsRatio));
    encoderBrc->curbe.DW21_DeviationThreshold1_VBRcontrol = (uint32_t)(-50 * pow(0.7, bpsRatio));
    encoderBrc->curbe.DW21_DeviationThreshold2_VBRcontrol = (uint32_t)(-50 * pow(0.5, bpsRatio));
    encoderBrc->curbe.DW21_DeviationThreshold3_VBRcontrol = (uint32_t)(-50 * pow(0.3, bpsRatio));

    encoderBrc->curbe.DW22_DeviationThreshold4_VBRcontrol = (uint32_t)(100 * pow(0.4, bpsRatio));
    encoderBrc->curbe.DW22_DeviationThreshold5_VBRcontrol = (uint32_t)(100 * pow(0.5, bpsRatio));
    encoderBrc->curbe.DW22_DeviationThreshold6_VBRcontrol = (uint32_t)(100 * pow(0.75, bpsRatio));
    encoderBrc->curbe.DW22_DeviationThreshold7_VBRcontrol = (uint32_t)(100 * pow(0.9, bpsRatio));

    encoderBrc->curbe.DW23_DeviationThreshold0_Iframe = (uint32_t)(-50 * pow(0.8, bpsRatio));
    encoderBrc->curbe.DW23_DeviationThreshold1_Iframe = (uint32_t)(-50 * pow(0.6, bpsRatio));
    encoderBrc->curbe.DW23_DeviationThreshold2_Iframe = (uint32_t)(-50 * pow(0.34, bpsRatio));
    encoderBrc->curbe.DW23_DeviationThreshold3_Iframe = (uint32_t)(-50 * pow(0.2, bpsRatio));

    encoderBrc->curbe.DW24_DeviationThreshold4_Iframe = (uint32_t)(50 * pow(0.2, bpsRatio));
    encoderBrc->curbe.DW24_DeviationThreshold5_Iframe = (uint32_t)(50 * pow(0.4, bpsRatio));
    encoderBrc->curbe.DW24_DeviationThreshold6_Iframe = (uint32_t)(50 * pow(0.66, bpsRatio));
    encoderBrc->curbe.DW24_DeviationThreshold7_Iframe = (uint32_t)(50 * pow(0.9, bpsRatio));

    encoderBrc->curbe.DW26_RandomAccess = !encoderBrc->m_lowDelay;

    if (encoderBrc->m_brcInit)
    {
        encoderBrc->m_dBrcInitCurrentTargetBufFullInBits = encoderBrc->curbe.DW1_InitBufFull;
    }

    encoderBrc->m_brcInitResetBufSizeInBits = encoderBrc->curbe.DW2_BufSize;
    encoderBrc->m_dBrcInitResetInputBitsPerFrame = inputBitsPerFrame;

    return eStatus;
}

MOS_STATUS CodecHalHevcBrcG12::BrcUpdateCurbe(
    CODECHAL_HEVC_BRC_KRNIDX    brcKrnIdx)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    // Initialize the CURBE data
    encoderBrc->curbeBrcUpdate = encoderBrc->m_brcUpdateCurbeInit;

    encoderBrc->curbeBrcUpdate.DW5_TargetSize_Flag = 0;

    if (encoderBrc->m_dBrcInitCurrentTargetBufFullInBits > (double)encoderBrc->m_brcInitResetBufSizeInBits)
    {
        encoderBrc->m_dBrcInitCurrentTargetBufFullInBits -= (double)encoderBrc->m_brcInitResetBufSizeInBits;
        encoderBrc->curbeBrcUpdate.DW5_TargetSize_Flag = 1;
    }

    if (encoderBrc->m_numSkipFrames)
    {
        // pass num/size of skipped frames to update BRC
        encoderBrc->curbeBrcUpdate.DW6_NumSkippedFrames = encoderBrc->m_numSkipFrames;
        encoderBrc->curbeBrcUpdate.DW15_SizeOfSkippedFrames = encoderBrc->m_sizeSkipFrames;

        // account for skipped frame in calculating CurrentTargetBufFullInBits
        encoderBrc->m_dBrcInitCurrentTargetBufFullInBits += encoderBrc->m_dBrcInitResetInputBitsPerFrame * encoderBrc->m_numSkipFrames;
    }

    encoderBrc->curbeBrcUpdate.DW0_TargetSize = (uint32_t)(encoderBrc->m_dBrcInitCurrentTargetBufFullInBits);
    encoderBrc->curbeBrcUpdate.DW1_FrameNumber = encoderBrc->m_storeData - 1; // Check if we can remove this (set to 0)

    uint32_t picHdrSize = encoderBrc->GetPicHdrSize();
    encoderBrc->curbeBrcUpdate.DW2_PictureHeaderSize = picHdrSize;

    encoderBrc->curbeBrcUpdate.DW5_CurrFrameBrcLevel = encoderBrc->m_currFrameBrcLevel;

    encoderBrc->curbeBrcUpdate.DW5_MaxNumPAKs = m_brcNumPakPasses;

    if (encoderBrc->m_hevcSeqParams->RateControlMethod == RATECONTROL_CQP)
    {
        encoderBrc->curbeBrcUpdate.DW6_CqpValue = encoderBrc->m_hevcPicParams->QpY + encoderBrc->m_hevcSliceParams->slice_qp_delta;
    }

    encoderBrc->curbeBrcUpdate.DW6_SlidingWindowEnable = (encoderBrc->m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW);

    if (encoderBrc->m_hevcPicParams->NumROI)
    {
        encoderBrc->curbeBrcUpdate.DW6_ROIEnable = encoderBrc->m_brcEnabled ? false : true;
        encoderBrc->curbeBrcUpdate.DW6_BRCROIEnable = encoderBrc->m_brcEnabled ? true : false;
        encoderBrc->curbeBrcUpdate.DW6_RoiRatio = encoderBrc->CalculateROIRatio();
    }

    if (encoderBrc->m_minMaxQpControlEnabled)
    {
        if (encoderBrc->m_hevcPicParams->CodingType == I_TYPE)
        {
            encoderBrc->curbeBrcUpdate.DW7_FrameMaxQP = encoderBrc->m_maxQpForI;
            encoderBrc->curbeBrcUpdate.DW7_FrameMinQP = encoderBrc->m_minQpForI;
        }
        else if (encoderBrc->m_hevcPicParams->CodingType == P_TYPE)
        {
            encoderBrc->curbeBrcUpdate.DW7_FrameMaxQP = encoderBrc->m_maxQpForP;
            encoderBrc->curbeBrcUpdate.DW7_FrameMinQP = encoderBrc->m_minQpForP;
        }
        else if (encoderBrc->m_hevcPicParams->CodingType == B_TYPE)
        {
            encoderBrc->curbeBrcUpdate.DW7_FrameMaxQP = encoderBrc->m_maxQpForB;
            encoderBrc->curbeBrcUpdate.DW7_FrameMinQP = encoderBrc->m_minQpForB;
        }
    }

    //for low delay brc
    encoderBrc->curbeBrcUpdate.DW6_LowDelayEnable      = (encoderBrc->m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW);
    encoderBrc->curbeBrcUpdate.DW16_UserMaxFrameSize   = encoderBrc->GetProfileLevelMaxFrameSize();

    encoderBrc->curbeBrcUpdate.DW14_ParallelMode = encoderBrc->m_hevcSeqParams->ParallelBRC;

    if (encoderBrc->m_hevcSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        encoderBrc->curbeBrcUpdate.DW3_StartGAdjFrame0 = (uint32_t)((10 * encoderBrc->m_usAvbrConvergence) / (double)150);
        encoderBrc->curbeBrcUpdate.DW3_StartGAdjFrame1 = (uint32_t)((50 * encoderBrc->m_usAvbrConvergence) / (double)150);
        encoderBrc->curbeBrcUpdate.DW4_StartGAdjFrame2 = (uint32_t)((100 * encoderBrc->m_usAvbrConvergence) / (double)150);
        encoderBrc->curbeBrcUpdate.DW4_StartGAdjFrame3 = (uint32_t)((150 * encoderBrc->m_usAvbrConvergence) / (double)150);

        encoderBrc->curbeBrcUpdate.DW11_gRateRatioThreshold0 =
            (uint32_t)((100 - (encoderBrc->m_usAvbrAccuracy / (double)30)*(100 - 40)));
        encoderBrc->curbeBrcUpdate.DW11_gRateRatioThreshold1 =
            (uint32_t)((100 - (encoderBrc->m_usAvbrAccuracy / (double)30)*(100 - 75)));
        encoderBrc->curbeBrcUpdate.DW12_gRateRatioThreshold2 = (uint32_t)((100 - (encoderBrc->m_usAvbrAccuracy / (double)30)*(100 - 97)));
        encoderBrc->curbeBrcUpdate.DW12_gRateRatioThreshold3 = (uint32_t)((100 + (encoderBrc->m_usAvbrAccuracy / (double)30)*(103 - 100)));
        encoderBrc->curbeBrcUpdate.DW12_gRateRatioThreshold4 = (uint32_t)((100 + (encoderBrc->m_usAvbrAccuracy / (double)30)*(125 - 100)));
        encoderBrc->curbeBrcUpdate.DW12_gRateRatioThreshold5 = (uint32_t)((100 + (encoderBrc->m_usAvbrAccuracy / (double)30)*(160 - 100)));
    }

    if (encoderBrc->m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)
    {
        encoderBrc->curbeBrcUpdate.DW17_LongTerm_Current = 0; // no LTR for low delay brc
    }
    else
    {
        encoderBrc->m_isFrameLTR = (CodecHal_PictureIsLongTermRef(encoderBrc->m_currReconstructedPic));
        encoderBrc->curbeBrcUpdate.DW17_LongTerm_Current = (encoderBrc->m_enableBrcLTR && encoderBrc->m_isFrameLTR) ? 1 : 0;
    }

    return eStatus;
}
MOS_STATUS CodecHalHevcBrcG12::EncodeBrcFrameUpdateKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    perfTag.Value = 0;
    perfTag.Mode = (uint16_t)encoderBrc->m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE;
    perfTag.PictureCodingType = encoderBrc->m_pictureCodingType;
    encoderBrc->GetOsInterface()->pfnSetPerfTag(encoderBrc->GetOsInterface(), perfTag.Value);
    encoderBrc->GetOsInterface()->pfnIncPerfBufferID(encoderBrc->GetOsInterface());

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupThreadSpace(m_cmKrnBrcUpdate, m_threadSpaceBrcUpdate));

    CODECHAL_HEVC_BRC_KRNIDX brcKrnIdx = CODECHAL_HEVC_BRC_FRAME_UPDATE;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(BrcUpdateCurbe(brcKrnIdx));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupSurfacesBrcUpdate());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupKernelArgsBrcUpdate());
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpBrcInputBuffers()));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmTask->AddKernel(m_cmKrnBrcUpdate));

    if (!encoderBrc->m_singleTaskPhaseSupported || encoderBrc->m_lastTaskInPhase)
    {
        CmEvent * event = CM_NO_EVENT;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmQueue->EnqueueFast(encoderBrc->m_cmTask, event));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmTask->Reset());

        encoderBrc->m_lastTaskInPhase = false;
    }
    else
    {
        encoderBrc->m_cmTask->AddSync();
    }
    return eStatus;
}

MOS_STATUS CodecHalHevcBrcG12::SetupSurfacesBrcUpdate()
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    // Fill HCP_IMG_STATE so that BRC kernel can use it to generate the write buffer for PAK
    PMOS_RESOURCE brcHcpStateReadBuffer = &encoderBrc->m_brcBuffers.resBrcImageStatesReadBuffer[encoderBrc->m_currRecycledBufIdx];
    MHW_VDBOX_HEVC_PIC_STATE_G12 mhwHevcPicState;
    mhwHevcPicState.pHevcEncSeqParams = encoderBrc->m_hevcSeqParams;
    mhwHevcPicState.pHevcEncPicParams = encoderBrc->m_hevcPicParams;
    mhwHevcPicState.brcNumPakPasses = m_brcNumPakPasses;
    mhwHevcPicState.rhodomainRCEnable = encoderBrc->m_brcEnabled && (encoderBrc->m_numPipe > 1);
    mhwHevcPicState.bSAOEnable = encoderBrc->m_hevcSeqParams->SAO_enabled_flag ? (encoderBrc->m_hevcSliceParams->slice_sao_luma_flag || encoderBrc->m_hevcSliceParams->slice_sao_chroma_flag) : 0;
    mhwHevcPicState.bTransformSkipEnable = encoderBrc->m_hevcPicParams->transform_skip_enabled_flag;
    mhwHevcPicState.bHevcRdoqEnabled      = encoderBrc->m_hevcRdoqEnabled;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_hcpInterface->AddHcpHevcPicBrcBuffer(brcHcpStateReadBuffer, &mhwHevcPicState));

    PMOS_SURFACE brcConstantData = &encoderBrc->m_brcBuffers.sBrcConstantDataBuffer[encoderBrc->m_currRecycledBufIdx];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->SetupBrcConstantTable(brcConstantData));

    if (!m_histBufferBrc)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->CreateBuffer(
            &encoderBrc->m_brcBuffers.resBrcHistoryBuffer, //m_brcHistoryBufferSize,
            m_histBufferBrc));
    }

    // BRC Prev PAK statistics output buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->UpdateBuffer(
        &encoderBrc->m_brcBuffers.resBrcPakStatisticBuffer[encoderBrc->m_brcBuffers.uiCurrBrcPakStasIdxForRead],
        m_PAKStatsBufferBrc));

    // BRC HCP_PIC_STATE read
    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->UpdateBuffer(
        brcHcpStateReadBuffer,
        m_PICStateInBufferBrc));

    // BRC HCP_PIC_STATE write
    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->UpdateBuffer(
        &encoderBrc->m_brcBuffers.resBrcImageStatesWriteBuffer[encoderBrc->m_currRecycledBufIdx],
        m_PICStateOutBufferBrc));

    // BRC Data Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->UpdateSurface2D(
        &brcConstantData->OsResource,
        m_ConstDataBufferBRC));

    // Pixel MB Statistics surface
    if (!m_PixelMBStatsBufferBrc)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->CreateBuffer(
            &encoderBrc->m_resMbStatsBuffer,
            m_PixelMBStatsBufferBrc));
    }

    // Combined ENC-parameter buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->UpdateBuffer(
        &encoderBrc->m_brcInputForEncKernelBuffer->sResource,
        m_CombinedEncBufferBrc));

    return eStatus;
}

MOS_STATUS CodecHalHevcBrcG12::SetupKernelArgsBrcUpdate()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    int idx = 0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrcUpdate->SetKernelArg(idx++, sizeof(encoderBrc->m_brcUpdateCurbeInit), &encoderBrc->curbeBrcUpdate));

    SurfaceIndex *pIndex0 = nullptr;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_histBufferBrc->GetIndex(pIndex0));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrcUpdate->SetKernelArg(idx++, sizeof(SurfaceIndex), pIndex0));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_PAKStatsBufferBrc->GetIndex(pIndex0));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrcUpdate->SetKernelArg(idx++, sizeof(SurfaceIndex), pIndex0));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_PICStateInBufferBrc->GetIndex(pIndex0));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrcUpdate->SetKernelArg(idx++, sizeof(SurfaceIndex), pIndex0));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_PICStateOutBufferBrc->GetIndex(pIndex0));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrcUpdate->SetKernelArg(idx++, sizeof(SurfaceIndex), pIndex0));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_CombinedEncBufferBrc->GetIndex(pIndex0));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrcUpdate->SetKernelArg(idx++, sizeof(SurfaceIndex), pIndex0));

    //Setup Distortion 2D surface
    CmSurface2D *brcDistortion = (encoderBrc->m_pictureCodingType == I_TYPE) ? encoderBrc->m_brcBuffers.brcIntraDistortionSurface
                                                                             : encoderBrc->m_brcBuffers.meBrcDistortionSurface;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(brcDistortion->GetIndex(pIndex0));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrcUpdate->SetKernelArg(idx++, sizeof(SurfaceIndex), pIndex0));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_ConstDataBufferBRC->GetIndex(pIndex0));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrcUpdate->SetKernelArg(idx++, sizeof(SurfaceIndex), pIndex0));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_PixelMBStatsBufferBrc->GetIndex(pIndex0));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrcUpdate->SetKernelArg(idx++, sizeof(SurfaceIndex), pIndex0));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_brcBuffers.mvAndDistortionSumSurface->GetIndex(pIndex0));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrcUpdate->SetKernelArg(idx++, sizeof(SurfaceIndex), pIndex0));

    return eStatus;
}

MOS_STATUS CodecHalHevcBrcG12::EncodeBrcLcuUpdateKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    perfTag.Value = 0;
    perfTag.Mode = (uint16_t)encoderBrc->m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE_LCU;
    perfTag.PictureCodingType = encoderBrc->m_pictureCodingType;
    encoderBrc->GetOsInterface()->pfnSetPerfTag(encoderBrc->GetOsInterface(), perfTag.Value);
    encoderBrc->GetOsInterface()->pfnIncPerfBufferID(encoderBrc->GetOsInterface());

    CODECHAL_HEVC_BRC_KRNIDX brcKrnIdx = CODECHAL_HEVC_BRC_LCU_UPDATE;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(BrcUpdateCurbe(brcKrnIdx));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupSurfacesBrcLcuQp());

    if (encoderBrc->m_hevcPicParams->NumROI)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->SetupROISurface());
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupKernelArgsBrcLcuQp());

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupBrcLcuqpThreadSpace(m_cmKrnBrcLCUQP, m_threadSpaceBrcLCUQP));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmTask->AddKernel(m_cmKrnBrcLCUQP));

    if (!encoderBrc->m_singleTaskPhaseSupported || encoderBrc->m_lastTaskInPhase)
    {
        CmEvent * event = CM_NO_EVENT;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmQueue->EnqueueFast(encoderBrc->m_cmTask, event));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmTask->Reset());

        encoderBrc->m_lastTaskInPhase = false;
    }
    else
    {
        encoderBrc->m_cmTask->AddSync();
    }

    return eStatus;
}

MOS_STATUS CodecHalHevcBrcG12::SetupSurfacesBrcLcuQp()
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    if (!m_histBufferBrc)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->CreateBuffer(
             &encoderBrc->m_brcBuffers.resBrcHistoryBuffer,
             m_histBufferBrc));
    }

    // Pixel MB Statistics surface
    if (!m_PixelMBStatsBufferBrc)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->CreateBuffer(
             &encoderBrc->m_resMbStatsBuffer,
             m_PixelMBStatsBufferBrc));
    }

    if (!m_BrcMbQp) {

        // BRC Data Surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->CreateSurface2D(
            &encoderBrc->m_brcBuffers.sBrcMbQpBuffer.OsResource,
            m_BrcMbQp));
    }

    if (!m_BrcROISurf)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->m_cmDev->CreateSurface2D(
            &encoderBrc->m_brcBuffers.sBrcRoiSurface.OsResource,
            m_BrcROISurf));
    }

    return eStatus;
}

MOS_STATUS CodecHalHevcBrcG12::SetupKernelArgsBrcLcuQp()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    int idx = 0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrcLCUQP->SetKernelArg(idx++, sizeof(encoderBrc->m_brcUpdateCurbeInit), &encoderBrc->curbeBrcUpdate));

    SurfaceIndex *pIndex0 = nullptr;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_histBufferBrc->GetIndex(pIndex0));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrcLCUQP->SetKernelArg(idx++, sizeof(SurfaceIndex), pIndex0));

    //Setup Distortion 2D surface
    CmSurface2D *brcDistortion = (encoderBrc->m_pictureCodingType == I_TYPE) ? encoderBrc->m_brcBuffers.brcIntraDistortionSurface
                                                                             : encoderBrc->m_brcBuffers.meBrcDistortionSurface;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(brcDistortion->GetIndex(pIndex0));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrcLCUQP->SetKernelArg(idx++, sizeof(SurfaceIndex), pIndex0));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_PixelMBStatsBufferBrc->GetIndex(pIndex0));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrcLCUQP->SetKernelArg(idx++, sizeof(SurfaceIndex), pIndex0));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_BrcMbQp->GetIndex(pIndex0));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrcLCUQP->SetKernelArg(idx++, sizeof(SurfaceIndex), pIndex0));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_BrcROISurf->GetIndex(pIndex0));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrnBrcLCUQP->SetKernelArg(idx++, sizeof(SurfaceIndex), pIndex0));
    return eStatus;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodecHalHevcBrcG12::DumpBrcInputBuffers()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->GetDebugInterface()->DumpBuffer(
        &encoderBrc->m_mvAndDistortionSumSurface.sResource,
        CodechalDbgAttr::attrInput,
        "MvDistSum",
        encoderBrc->m_mvAndDistortionSumSurface.dwSize,
        0,
        CODECHAL_MEDIA_STATE_BRC_UPDATE));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->GetDebugInterface()->DumpBuffer(
        &encoderBrc->m_brcBuffers.resBrcImageStatesReadBuffer[encoderBrc->m_currRecycledBufIdx],
        CodechalDbgAttr::attrInput,
        "ImgStateRead",
        BRC_IMG_STATE_SIZE_PER_PASS * encoderBrc->GetHwInterface()->GetMfxInterface()->GetBrcNumPakPasses(),
        0,
        CODECHAL_MEDIA_STATE_BRC_UPDATE));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->GetDebugInterface()->DumpSurface(
        &encoderBrc->m_brcBuffers.sBrcConstantDataBuffer[encoderBrc->m_currRecycledBufIdx],
        CodechalDbgAttr::attrInput,
        "ConstData",
        CODECHAL_MEDIA_STATE_BRC_UPDATE));

    // PAK statistics buffer is only dumped for BrcUpdate kernel input
    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->GetDebugInterface()->DumpBuffer(
        &encoderBrc->m_brcBuffers.resBrcPakStatisticBuffer[encoderBrc->m_brcBuffers.uiCurrBrcPakStasIdxForRead],
        CodechalDbgAttr::attrInput,
        "PakStats",
        HEVC_BRC_PAK_STATISTCS_SIZE,
        0,
        CODECHAL_MEDIA_STATE_BRC_UPDATE));
    // HEVC maintains a ptr to its own distortion surface, as it may be a couple different surfaces
    if (encoderBrc->m_brcDistortion)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            encoderBrc->GetDebugInterface()->DumpBuffer(
                &encoderBrc->m_brcDistortion->OsResource,
                CodechalDbgAttr::attrInput,
                "BrcDist_BeforeFrameBrc",
                encoderBrc->m_brcBuffers.sMeBrcDistortionBuffer.dwPitch * encoderBrc->m_brcBuffers.sMeBrcDistortionBuffer.dwHeight,
                encoderBrc->m_brcBuffers.dwMeBrcDistortionBottomFieldOffset,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->GetDebugInterface()->DumpBuffer(&encoderBrc->m_brcBuffers.resBrcHistoryBuffer,
        CodechalDbgAttr::attrInput,
        "HistoryRead_beforeFramBRC",
        encoderBrc->m_brcHistoryBufferSize,
        0,
        CODECHAL_MEDIA_STATE_BRC_UPDATE));

    if (encoderBrc->m_brcBuffers.pMbEncKernelStateInUse)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->GetDebugInterface()->DumpCurbe(
            CODECHAL_MEDIA_STATE_BRC_UPDATE,
            encoderBrc->m_brcBuffers.pMbEncKernelStateInUse));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoderBrc->GetDebugInterface()->DumpBuffer(&encoderBrc->m_resMbStatsBuffer,
        CodechalDbgAttr::attrInput,
        "MBStatsSurf",
        encoderBrc->GetHwInterface()->m_avcMbStatBufferSize,
        0,
        CODECHAL_MEDIA_STATE_BRC_UPDATE));

    return eStatus;
}
#endif
