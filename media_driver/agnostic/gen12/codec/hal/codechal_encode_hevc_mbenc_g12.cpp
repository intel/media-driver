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
#include "mhw_vdbox_hcp_g12_X.h"
#include "codechal_kernel_hme_mdf_g12.h"
#include "codechal_kernel_header_g12.h"
#include "Gen12_HEVC_B_LCU32.h"
#include "Gen12_HEVC_B_LCU64.h"
#include "cm_wrapper.h"

#include "Gen12_HEVC_BRC_INIT.h"
#include "Gen12_HEVC_BRC_RESET.h"
#include "Gen12_HEVC_BRC_UPDATE.h"
#include "Gen12_HEVC_BRC_LCUQP.h"
#include "Gen12LP_CoarseIntra_genx.h"
#include "Gen12LP_WeightedPrediction_genx.h"

#if USE_PROPRIETARY_CODE
#include "cm_device_rt.h"
#endif

#if MOS_MEDIASOLO_SUPPORTED
#include "mos_os_solo.h"
#endif // (_DEBUG || _RELEASE_INTERNAL)

CodecHalHevcMbencG12::CodecHalHevcMbencG12(CodechalHwInterface* hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo)
    : CodechalEncHevcStateG12(hwInterface, debugInterface, standardInfo)
{
    m_useMdf = true;
    for (int32_t idx = 0; idx < MAX_VME_FWD_REF + MAX_VME_BWD_REF; idx++)
    {
        m_surfRefArray[idx] = nullptr;
        m_surf2XArray[idx]  = nullptr;
    }

}

CodecHalHevcMbencG12::~CodecHalHevcMbencG12() {
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_wpState)
    {
        MOS_Delete(m_wpState);
        m_wpState = nullptr;
    }

    if (m_intraDistKernel)
    {
        MOS_Delete(m_intraDistKernel);
        m_intraDistKernel = nullptr;
    }

    DestroyMDFResources();
}

MOS_STATUS CodecHalHevcMbencG12::AllocateEncResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcStateG12::AllocateEncResources());

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateMeResources());

    if (m_hmeSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->AllocateResources());
    }
    // Intermediate CU Record Surface
    if (!m_intermediateCuRecordLcu32)
    {
        //MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D(
            m_widthAlignedLcu32,
            m_heightAlignedLcu32 >> 1,
            Format_A8,
            m_intermediateCuRecordLcu32));
    }

    // Scratch Surface
    if (!m_scratchSurf)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D(
            m_widthAlignedLcu32 >> 3,
            m_heightAlignedLcu32 >> 5,
            Format_A8,
            m_scratchSurf));
    }

    // Enc constant table for B
    if (!m_constTableB)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateBuffer(
            m_encConstantDataLutSize,
            m_constTableB));
    }

    // Load Balance surface size
    if (!m_loadBalance)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateBuffer(
            m_threadMapSize,
            m_loadBalance));
    }

    //Debug surface
    if (!m_dbgSurface)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateBuffer(
            m_debugSurfaceSize,
            m_dbgSurface));
    }

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS CodecHalHevcMbencG12::AllocateMeResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    if (m_hmeSupported)
    {
        // BRC Distortion Surface
        if (!m_brcBuffers.meBrcDistortionSurface)
        {
            uint32_t width = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x << 3), 64);
            uint32_t height = MOS_ALIGN_CEIL((m_downscaledHeightInMb4x << 2), 8) << 1;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D(
                width,
                height,
                Format_A8,
                m_brcBuffers.meBrcDistortionSurface));
        }

        // MV and Distortion Summation Surface
        if (!m_brcBuffers.mvAndDistortionSumSurface)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateBuffer(
                m_mvdistSummationSurfSize,
                m_brcBuffers.mvAndDistortionSumSurface));
            CmEvent *event = nullptr;
            m_brcBuffers.mvAndDistortionSumSurface->InitSurface(0, event);
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodecHalHevcMbencG12::AllocateBrcResources()
{
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcState::AllocateBrcResources());

    // BRC Intra Distortion Surface
    uint32_t width = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x << 3), 64);
    uint32_t height = MOS_ALIGN_CEIL((m_downscaledHeightInMb4x << 2), 8) << 1;

    if (!m_brcBuffers.brcIntraDistortionSurface)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D(
            width,
            height,
            Format_A8,
            m_brcBuffers.brcIntraDistortionSurface));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodecHalHevcMbencG12::FreeBrcResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalEncHevcState::FreeBrcResources();

    if (m_brcBuffers.brcIntraDistortionSurface)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_brcBuffers.brcIntraDistortionSurface))
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodecHalHevcMbencG12::FreeMeResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_brcBuffers.meBrcDistortionSurface)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_brcBuffers.meBrcDistortionSurface))
    }

    if (m_brcBuffers.mvAndDistortionSumSurface)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_brcBuffers.mvAndDistortionSumSurface));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodecHalHevcMbencG12::FreeEncResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(FreeMeResources());

    if (m_intermediateCuRecordLcu32)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_intermediateCuRecordLcu32));
        m_intermediateCuRecordLcu32 = nullptr;
    }
    if (m_scratchSurf)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_scratchSurf));
        m_scratchSurf = nullptr;
    }
    if (m_cu16X16QpIn)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_cu16X16QpIn));
        m_cu16X16QpIn = nullptr;
    }
    if (m_constTableB)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_constTableB));
        m_constTableB = nullptr;
    }
    if (m_cuSplitSurf)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_cuSplitSurf));
        m_cuSplitSurf = nullptr;
    }
    if (m_loadBalance)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_loadBalance));
        m_loadBalance = nullptr;
    }
    if (m_dbgSurface)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_dbgSurface));
        m_dbgSurface = nullptr;
    }

    if (m_lcuLevelData)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_lcuLevelData));
        m_lcuLevelData = nullptr;
    }
    if (m_reconWithBoundaryPix)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_reconWithBoundaryPix));
        m_reconWithBoundaryPix = nullptr;
    }

    //container surfaces
    if (m_curSurf)
    {
        m_curSurf->NotifyUmdResourceChanged(nullptr);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_curSurf));
        m_curSurf = nullptr;
    }
    if (m_mbCodeBuffer)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_mbCodeBuffer));
        m_mbCodeBuffer = nullptr;
    }
    if (m_swScoreboardSurf)
    {
        m_swScoreboardSurf->NotifyUmdResourceChanged(nullptr);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_swScoreboardSurf));
        m_swScoreboardSurf = nullptr;
    }
    if (m_curSurf2X)
    {
        m_curSurf2X->NotifyUmdResourceChanged(nullptr);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_curSurf2X));
        m_curSurf2X = nullptr;
    }
    if (m_histInBuffer)
    {
        m_histInBuffer->NotifyUmdResourceChanged(nullptr);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_histInBuffer));
        m_histInBuffer = nullptr;
    }
    if (m_histOutBuffer)
    {
        m_histOutBuffer->NotifyUmdResourceChanged(nullptr);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_histOutBuffer));
        m_histOutBuffer = nullptr;
    }
    for (int32_t idx = 0; idx < MAX_VME_FWD_REF + MAX_VME_BWD_REF; idx++)
    {
        if (m_surfRefArray[idx])
        {
            m_surfRefArray[idx]->NotifyUmdResourceChanged(nullptr);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_surfRefArray[idx]));
            m_surfRefArray[idx] = nullptr;
        }
        if (m_surf2XArray[idx])
        {
            m_surf2XArray[idx]->NotifyUmdResourceChanged(nullptr);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_surf2XArray[idx]));
            m_surf2XArray[idx] = nullptr;
        }
    }

    //Free MDF objects
    if (m_cmKrnB)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroyKernel(m_cmKrnB));
        m_cmKrnB = nullptr;
    }
    if (m_cmKrnB64)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroyKernel(m_cmKrnB64));
        m_cmKrnB64 = nullptr;
    }
    if (m_cmProgramB)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroyProgram(m_cmProgramB));
        m_cmProgramB = nullptr;
    }
    if (m_cmProgramB64)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroyProgram(m_cmProgramB64));
        m_cmProgramB64 = nullptr;
    }
    if (m_hevcBrcG12)
    {
        MOS_Delete(m_hevcBrcG12);
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcStateG12::FreeEncResources());

    return eStatus;
}

MOS_STATUS CodecHalHevcMbencG12::AllocateMDFResources()
{
    uint32_t devOp = CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_DISABLE | CM_DEVICE_CONFIG_FAST_PATH_ENABLE;

    if (!m_mfeEnabled)
    {
        //create CM device
        if (!m_cmDev)
        {
            m_osInterface->pfnNotifyStreamIndexSharing(m_osInterface);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CreateCmDevice(
                m_osInterface->pOsContext,
                m_cmDev,
                devOp));
        }

        if (!m_surfIndexArray)
        {
            m_surfIndexArray = (MBencSurfaceIndex *) new (std::nothrow) (SurfaceIndex [m_maxMfeSurfaces][m_maxMultiFrames]);
            CODECHAL_ENCODE_CHK_NULL_RETURN(m_surfIndexArray);
        }
    }
    else
    {
        //create CM device
        if (!m_cmDev)
        {
            if (!m_mfeEncodeSharedState->pCmDev)
            {
                m_osInterface->pfnNotifyStreamIndexSharing(m_osInterface);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CreateCmDevice(
                    m_osInterface->pOsContext,
                    m_cmDev,
                    devOp));

                m_mfeEncodeSharedState->pCmDev = m_cmDev;
            }
            else
            {
                m_cmDev = m_mfeEncodeSharedState->pCmDev;
            }
        }

        if (!m_mfeEncodeSharedState->commonSurface)
        {
            m_surfIndexArray = (MBencSurfaceIndex *) new (std::nothrow) ( SurfaceIndex [m_maxMfeSurfaces][m_maxMultiFrames]);
            CODECHAL_ENCODE_CHK_NULL_RETURN(m_surfIndexArray);
            m_mfeEncodeSharedState->commonSurface = reinterpret_cast<SurfaceIndex *>(m_surfIndexArray);
        }
        else
        {
            m_surfIndexArray = reinterpret_cast<MBencSurfaceIndex *>(m_mfeEncodeSharedState->commonSurface);
        }

        if (!m_mfeEncodeSharedState->maxThreadWidthFrames)
        {
            m_mfeEncodeSharedState->maxThreadWidthFrames  = MOS_NewArray(uint32_t, m_maxMultiFrames);
            CODECHAL_ENCODE_CHK_NULL_RETURN(m_mfeEncodeSharedState->maxThreadWidthFrames);
        }
    }

    //create CM Queue
    if (!m_cmQueue)
    {
        CM_QUEUE_CREATE_OPTION queueCreateOption = CM_DEFAULT_QUEUE_CREATE_OPTION;
        if (m_computeContextEnabled)
        {
            queueCreateOption.QueueType = CM_QUEUE_TYPE_COMPUTE;
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateQueueEx(m_cmQueue, queueCreateOption));
    }

    //create CM task
    if (!m_cmTask)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateTask(m_cmTask));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodecHalHevcMbencG12::DestroyMDFResources()
{
    if (m_cmDev && m_cmTask)
    {
        m_cmDev->DestroyTask(m_cmTask);
        m_cmTask = nullptr;
    }

    if (!m_mfeEnabled)
    {
        delete[] m_surfIndexArray;
        m_surfIndexArray = nullptr;

        DestroyCmDevice(m_cmDev);
        m_cmDev = nullptr;
    }
    else 
    {
        if (m_mfeLastStream)
        {
            MOS_DeleteArray(m_mfeEncodeSharedState->maxThreadWidthFrames);
            m_mfeEncodeSharedState->maxThreadWidthFrames = nullptr;

            delete[] m_surfIndexArray;
            m_surfIndexArray = nullptr;
            m_mfeEncodeSharedState->commonSurface = nullptr;

            DestroyCmDevice(m_cmDev);
            m_mfeEncodeSharedState->pCmDev = m_cmDev = nullptr;
        }
        else
        {
            m_surfIndexArray = nullptr;
            m_cmDev = nullptr;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodecHalHevcMbencG12::InitKernelState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_colorBitMfeEnabled = m_mfeEnabled ? true : false;

    // Create weighted prediction kernel state
    m_wpState = MOS_New(CodechalEncodeWPMdfG12, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_wpState);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(((CodechalEncodeWPMdfG12 *)m_wpState)->InitKernelStateIsa((void *)GEN12LP_WEIGHTEDPREDICTION_GENX, GEN12LP_WEIGHTEDPREDICTION_GENX_SIZE));

    // create intra distortion kernel
    m_intraDistKernel = MOS_New(CodechalKernelIntraDistMdfG12, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_intraDistKernel);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_intraDistKernel->InitializeKernelIsa(
        (void*)GEN12LP_COARSEINTRA_GENX,
        GEN12LP_COARSEINTRA_GENX_SIZE));

    // Create SW scoreboard init kernel state
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_swScoreboardState = MOS_New(CodechalEncodeSwScoreboardMdfG12, this));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_swScoreboardState->InitKernelState());
    // Create Hme kernel
    m_hmeKernel = MOS_New(CodechalKernelHmeMdfG12, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hmeKernel)

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->LoadProgram((void *)GEN12_HEVC_B_LCU32,
        GEN12_HEVC_B_LCU32_SIZE,
        m_cmProgramB,
        "-nojitter"));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateKernel(m_cmProgramB,
        "Gen12_HEVC_Enc_B",
        m_cmKrnB));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->LoadProgram((void *)GEN12_HEVC_B_LCU64,
        GEN12_HEVC_B_LCU64_SIZE,
        m_cmProgramB64,
        "-nojitter"));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateKernel(m_cmProgramB64,
        "Gen12_HEVC_Enc_LCU64_B",
        m_cmKrnB64));

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hevcBrcG12 = MOS_New(CodecHalHevcBrcG12, this));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hevcBrcG12->InitBrcKernelState());
    return eStatus;
}

uint32_t CodecHalHevcMbencG12::GetMaxBtCount()
{
    uint16_t btIdxAlignment = m_hwInterface->GetRenderInterface()->m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment();
    uint32_t btCountPhase2 = btIdxAlignment;
    return btCountPhase2;
}

MOS_STATUS CodecHalHevcMbencG12::SetupKernelArgsB()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    //Setup surfaces
    //Setup first combined 1D surface
    int idx = 0;
    SurfaceIndex *surfIndex = nullptr;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_combinedBuffer1->GetIndex(surfIndex));
    (*m_surfIndexArray)[idx++][m_mfeEncodeParams.streamId] = *surfIndex;

    //Setup second combined 1D surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_combinedBuffer2->GetIndex(surfIndex));
    (*m_surfIndexArray)[idx++][m_mfeEncodeParams.streamId] = *surfIndex;

    //VME Surface
    (*m_surfIndexArray)[idx++][m_mfeEncodeParams.streamId] = *m_curVme;

    //Curr Pic
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_curSurf->GetIndex(surfIndex));
    (*m_surfIndexArray)[idx++][m_mfeEncodeParams.streamId] = *surfIndex;

    //Recon surface with populated boundary pixels.
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_reconWithBoundaryPix->GetIndex(surfIndex));
    (*m_surfIndexArray)[idx++][m_mfeEncodeParams.streamId] = *surfIndex;

    //Intermediate CU Record Surface for I and B kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_intermediateCuRecordLcu32->GetIndex(surfIndex));
    (*m_surfIndexArray)[idx++][m_mfeEncodeParams.streamId] = *surfIndex;

    // PAK object command surface
    (*m_surfIndexArray)[idx++][m_mfeEncodeParams.streamId] = *m_mbCodeSurfIdx;

    // CU packet for PAK surface
    (*m_surfIndexArray)[idx++][m_mfeEncodeParams.streamId] = *m_mvDataSurfIdx;

    //Software Scoreboard surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_swScoreboardSurf->GetIndex(surfIndex));
    (*m_surfIndexArray)[idx++][m_mfeEncodeParams.streamId] = *surfIndex;

    // CU 16x16 QP data input surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cu16X16QpIn->GetIndex(surfIndex));
    (*m_surfIndexArray)[idx++][m_mfeEncodeParams.streamId] = *surfIndex;

    // Lcu level data input
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_lcuLevelData->GetIndex(surfIndex));
    (*m_surfIndexArray)[idx++][m_mfeEncodeParams.streamId] = *surfIndex;

    //ColocatedCUMVDataSurface
    if (m_colocCumvData)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_colocCumvData->GetIndex(surfIndex));
    }
    (*m_surfIndexArray)[idx++][m_mfeEncodeParams.streamId] = *surfIndex;

    //HMEMotionPredDataSurface
    if (m_hmeMotionPredData)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeMotionPredData->GetIndex(surfIndex));
    }
    (*m_surfIndexArray)[idx++][m_mfeEncodeParams.streamId] = *surfIndex;

    if (m_isMaxLcu64)
    {
        if (m_curSurf2X)
        {
            (*m_surfIndexArray)[idx++][m_mfeEncodeParams.streamId] = *m_cur2XVme;

        }
        else
        {
            (*m_surfIndexArray)[idx++][m_mfeEncodeParams.streamId] = *surfIndex;
        }

    }


    // Kernel debug surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_dbgSurface->GetIndex(surfIndex));
    (*m_surfIndexArray)[idx++][m_mfeEncodeParams.streamId] = *surfIndex;

    //Init all the surfaces with dummy value
    if ((!m_mfeEnabled) || (m_mfeFirstStream))
    {
        for (int i = 0; i < m_maxMfeSurfaces; i++)
        {
            for (int j = 1; j < m_maxMultiFrames; j++)
            {
                (*m_surfIndexArray)[i][j] = (*m_surfIndexArray)[i][0];
            }
        }
    }

    if ((m_mfeLastStream) || (!m_mfeEnabled))
    {
        CmKernel *cmKrn = nullptr;
        if (m_isMaxLcu64)
        {
            cmKrn = m_cmKrnB64;
        }
        else
        {
            cmKrn = m_cmKrnB;
        }

        //Setup surfaces
        //Setup first combined 1D surface
        int idx = 0;
        int commonIdx = 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex) * m_maxMultiFrames, &((*m_surfIndexArray)[commonIdx++][0])));

        //Setup second combined 1D surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex) * m_maxMultiFrames, &((*m_surfIndexArray)[commonIdx++][0])));

        //VME Surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex) * m_maxMultiFrames, &((*m_surfIndexArray)[commonIdx++][0])));

        //Curr Pic
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex) * m_maxMultiFrames, &((*m_surfIndexArray)[commonIdx++][0])));

        //Recon surface with populated boundary pixels.
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex) * m_maxMultiFrames, &((*m_surfIndexArray)[commonIdx++][0])));

        //Intermediate CU Record Surface for I and B kernel
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex) * m_maxMultiFrames, &((*m_surfIndexArray)[commonIdx++][0])));

        // PAK object command surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex) * m_maxMultiFrames, &((*m_surfIndexArray)[commonIdx++][0])));

        // CU packet for PAK surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex) * m_maxMultiFrames, &((*m_surfIndexArray)[commonIdx++][0])));

        //Software Scoreboard surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex) * m_maxMultiFrames, &((*m_surfIndexArray)[commonIdx++][0])));

        // CU 16x16 QP data input surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex) * m_maxMultiFrames, &((*m_surfIndexArray)[commonIdx++][0])));

        // Lcu level data input
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex) * m_maxMultiFrames, &((*m_surfIndexArray)[commonIdx++][0])));

        //ColocatedCUMVDataSurface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex) * m_maxMultiFrames, &((*m_surfIndexArray)[commonIdx++][0])));

        //HMEMotionPredDataSurface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex) * m_maxMultiFrames, &((*m_surfIndexArray)[commonIdx++][0])));

        if (m_isMaxLcu64)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex) * m_maxMultiFrames, &((*m_surfIndexArray)[commonIdx++][0])));
        }

        //Enc const table
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_constTableB->GetIndex(surfIndex));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), surfIndex));

        //load Balance surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_loadBalance->WriteSurface(m_FrameBalance, nullptr));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_loadBalance->GetIndex(surfIndex));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), surfIndex));

        //reserved entries
        if (!m_isMaxLcu64)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), surfIndex));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), surfIndex));
        }

        // Kernel debug surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex) * m_maxMultiFrames, &((*m_surfIndexArray)[commonIdx++][0])));
    }

    return eStatus;
}

void CodecHalHevcMbencG12::SetColorBitRemap(uint8_t * remapTable, int32_t multiFrameNumber, int32_t curColor, int32_t * totalColor, int32_t * totalFrameAdj)
{
    if (multiFrameNumber == 1)
    {
        *totalColor = curColor;
        uint8_t * curColorLOC;
        for (int32_t i = 0; i < *totalColor; i++)
        {
            curColorLOC = (uint8_t *)(remapTable + i * m_frameColorMapEntrySize);

            curColorLOC[m_frameColorMapLocCurFrame] = static_cast<uint8_t>(multiFrameNumber - 1);
            curColorLOC[m_frameColorMapLocCurColor] = static_cast<uint8_t>(i);
            curColorLOC[m_frameColorMapLocTotalFrame] = 0;
            curColorLOC[m_frameColorMapLocTotalColor] = static_cast<uint8_t>(*totalColor);
        }
    }
    else if (multiFrameNumber == 2)
    {
        *totalColor = ((curColor + 1) >> 1) << 2;
        uint8_t * curColorLOC;
        int32_t n1 = 0;
        int32_t n2 = 0;
        for (int32_t i = 0; i < *totalColor; i++)
        {
            curColorLOC = (uint8_t *)(remapTable + i * m_frameColorMapEntrySize);
            curColorLOC[m_frameColorMapLocCurFrame] = i & 0x1;

            if (curColorLOC[m_frameColorMapLocCurFrame] == 0)
            {
                if (n1 >= curColor)
                    curColorLOC[m_frameColorMapLocCurFrame] = m_frameColorMapFrameInvalid;
                curColorLOC[m_frameColorMapLocCurColor] = static_cast<uint8_t>(n1);
                n1++;
            }
            else if (curColorLOC[m_frameColorMapLocCurFrame] == 1)
            {
                if (n2 >= curColor)
                    curColorLOC[m_frameColorMapLocCurFrame] = m_frameColorMapFrameInvalid;
                curColorLOC[m_frameColorMapLocCurColor] = static_cast<uint8_t>(n2);
                n2++;
            }
            curColorLOC[m_frameColorMapLocTotalColor] = static_cast<uint8_t>(*totalColor);
            if ((n1 >= curColor) && (n2 >= curColor))
            {
                *totalColor = i + 1;
                break;
            }
        }
        for (int32_t i = 0; i < *totalColor; i++)
        {
            curColorLOC = (uint8_t *)(remapTable + i * m_frameColorMapEntrySize);
            curColorLOC[m_frameColorMapLocTotalColor] = static_cast<uint8_t>(*totalColor);
            if (curColorLOC[m_frameColorMapLocCurFrame] != m_frameColorMapFrameInvalid)
                curColorLOC[m_frameColorMapLocTotalFrame] = static_cast<uint8_t>(totalFrameAdj[curColorLOC[m_frameColorMapLocCurFrame]]);
            else
                curColorLOC[m_frameColorMapLocTotalFrame] = 0;
        }
    }
    else if (multiFrameNumber == 3)
    {
        *totalColor = curColor << 2;
        uint8_t * curColorLOC;
        int32_t n1 = 0;
        int32_t n2 = 0;
        for (int32_t i = 0; i < *totalColor; i++)
        {
            curColorLOC = (uint8_t *)(remapTable + i * m_frameColorMapEntrySize);
            if ((i & 3) < 2)
            {
                curColorLOC[m_frameColorMapLocCurFrame] = i & 0x3;
                if (n1 >= curColor)
                    curColorLOC[m_frameColorMapLocCurFrame] = m_frameColorMapFrameInvalid;
                curColorLOC[m_frameColorMapLocCurColor] = i >> 2;
                curColorLOC[m_frameColorMapLocTotalFrame] = static_cast<uint8_t>(multiFrameNumber);
                if ((i & 3) == 1)
                    n1++;
            }
            else
            {
                curColorLOC[m_frameColorMapLocCurFrame] = 2;
                if (n2 >= curColor)
                    curColorLOC[m_frameColorMapLocCurFrame] = m_frameColorMapFrameInvalid;
                curColorLOC[m_frameColorMapLocCurColor] = static_cast<uint8_t>(n2);
                n2++;
            }

            if ((n1 >= curColor) && (n2 >= curColor))
            {
                *totalColor = i + 1;
                break;
            }
        }
        for (int32_t i = 0; i < *totalColor; i++)
        {
            curColorLOC = (uint8_t *)(remapTable + i * m_frameColorMapEntrySize);
            curColorLOC[m_frameColorMapLocTotalColor] = static_cast<uint8_t>(*totalColor);
            if (curColorLOC[m_frameColorMapLocCurFrame] != m_frameColorMapFrameInvalid)
                curColorLOC[m_frameColorMapLocTotalFrame] = static_cast<uint8_t>(totalFrameAdj[curColorLOC[m_frameColorMapLocCurFrame]]);
            else
                curColorLOC[m_frameColorMapLocTotalFrame] = 0;
        }
    }
    else if (multiFrameNumber == 4)
    {
        *totalColor = curColor << 2;
        uint8_t * curColorLOC;
        for (int32_t i = 0; i < *totalColor; i++)
        {
            curColorLOC = (uint8_t *)(remapTable + i * m_frameColorMapEntrySize);

            curColorLOC[m_frameColorMapLocCurFrame] = i & 0x3;
            curColorLOC[m_frameColorMapLocCurColor] = i >> 2;
            curColorLOC[m_frameColorMapLocTotalFrame] = static_cast<uint8_t>(totalFrameAdj[curColorLOC[m_frameColorMapLocCurFrame]]);
            curColorLOC[m_frameColorMapLocTotalColor] = static_cast<uint8_t>(*totalColor);
        }
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE(" Error: MultiFrameNumber , not supported!");
    }

    return;
}

MOS_STATUS CodecHalHevcMbencG12::EncodeMbEncKernel(
    CODECHAL_MEDIA_STATE_TYPE   encFunctionType)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_WALKER_CODEC_PARAMS    walkerCodecParams;
    CODECHAL_WALKER_DEGREE          walkerDegree;
    MHW_WALKER_PARAMS               walkerParams;
    uint32_t                        walkerResolutionX, walkerResolutionY, maxthreadWidth, maxthreadHeight;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_MBENC_KERNEL);

    CmKernel *cmKrn = nullptr;
    if (m_isMaxLcu64)
    {
        cmKrn             = m_cmKrnB64;
        if (m_hevcSeqParams->TargetUsage == 1)
        {
            walkerResolutionX = MOS_ALIGN_CEIL(m_frameWidth, MAX_LCU_SIZE) >> 6;
            walkerResolutionY = MOS_ALIGN_CEIL(m_frameHeight, MAX_LCU_SIZE) >> 6;
        }
        else
        {
            walkerResolutionX = 2 * (MOS_ALIGN_CEIL(m_frameWidth, MAX_LCU_SIZE) >> 6);
            walkerResolutionY = 2 * (MOS_ALIGN_CEIL(m_frameHeight, MAX_LCU_SIZE) >> 6);
        }
    }
    else
    {
        cmKrn             = m_cmKrnB;
        walkerResolutionX = MOS_ALIGN_CEIL(m_frameWidth, 32) >> 5;
        walkerResolutionY = MOS_ALIGN_CEIL(m_frameHeight, 32) >> 5;
    }

    if (m_numberConcurrentGroup > 1)
    {
        if (m_degree45Needed)
        {
            maxthreadWidth  = walkerResolutionX;
            maxthreadHeight = walkerResolutionX + (walkerResolutionX + walkerResolutionY + m_numberConcurrentGroup - 2) / m_numberConcurrentGroup;
        }
        else //for tu4 we ensure threadspace width and height is even or a multiple of 4
        {
            maxthreadWidth  = (walkerResolutionX + 1) & 0xfffe; //ensuring width is even
            maxthreadHeight = ((walkerResolutionX + 1) >> 1) + (walkerResolutionX + 2 * (((walkerResolutionY + 3) & 0xfffc) - 1) + (2 * m_numberConcurrentGroup - 1)) / (2 * m_numberConcurrentGroup);
        }

        maxthreadHeight *= m_numberEncKernelSubThread;
        maxthreadHeight += 1;
    }
    else
    {
        maxthreadWidth = walkerResolutionX;
        maxthreadHeight = walkerResolutionY;
        maxthreadHeight *= m_numberEncKernelSubThread;
    }

    // Generate Lcu Level Data
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GenerateLcuLevelData(m_lcuLevelInputDataSurface[m_currRecycledBufIdx]));

    // Generate Concurrent Thread Group Data
    uint32_t    curIdx = m_currRecycledBufIdx;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GenerateConcurrentThreadGroupData(m_encBCombinedBuffer1[curIdx].sResource));

    if (m_mfeEnabled)
    {
        if (m_mfeEncodeSharedState->maxTheadWidth < maxthreadWidth)
        {
            m_mfeEncodeSharedState->maxTheadWidth = maxthreadWidth;
        }
        if (m_mfeEncodeSharedState->maxTheadHeight < maxthreadHeight)
        {
            m_mfeEncodeSharedState->maxTheadHeight = maxthreadHeight;
        }

        m_mfeEncodeSharedState->maxThreadWidthFrames[m_mfeEncodeParams.submitIndex] = maxthreadWidth;
        m_mfeLastStream = (m_mfeEncodeParams.submitIndex == m_mfeEncodeParams.submitNumber - 1);
        m_mfeFirstStream = (m_mfeEncodeParams.submitIndex == 0);

        if (m_mfeLastStream)
        {
            for (uint32_t i = 0; i < m_mfeEncodeParams.submitNumber; i++)
            {
                m_totalFrameAdj[i] = m_mfeEncodeSharedState->maxTheadWidth - m_mfeEncodeSharedState->maxThreadWidthFrames[i];
            }
        }
    }

    int32_t totalColor = m_numberConcurrentGroup;
    if ((!m_mfeEnabled) || (m_mfeLastStream))
    {
        SetColorBitRemap(m_FrameBalance, m_mfeEncodeParams.submitNumber, m_numberConcurrentGroup, &totalColor, m_totalFrameAdj);
    }

    m_mbCodeIdxForTempMVP = 0xFF;
    if (m_pictureCodingType == I_TYPE || m_hevcSeqParams->sps_temporal_mvp_enable_flag == false)
    {
        // No temoporal MVP in the I frame
        m_hevcSliceParams->slice_temporal_mvp_enable_flag = false;
    }
    else
    {
        if (m_hevcPicParams->CollocatedRefPicIndex != 0xFF &&
            m_hevcPicParams->CollocatedRefPicIndex < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            uint8_t frameIdx = m_hevcPicParams->RefFrameList[m_hevcPicParams->CollocatedRefPicIndex].FrameIdx;
            // ref must be valid, ref list has max 127 entries
            if (frameIdx < 0x7F && m_hevcPicParams->RefFrameList[m_hevcPicParams->CollocatedRefPicIndex].PicEntry != 0xFF)
            {
                m_mbCodeIdxForTempMVP = m_refList[frameIdx]->ucScalingIdx;
            }
        }
        if (m_mbCodeIdxForTempMVP == 0xFF && m_hevcSliceParams->slice_temporal_mvp_enable_flag)
        {
            // Temporal reference MV index is invalid and so disable the temporal MVP
            CODECHAL_ENCODE_ASSERT(false);
            m_hevcSliceParams->slice_temporal_mvp_enable_flag = false;
        }
    }

    if (m_mfeEnabled && m_mfeLastStream)
    {
        //update the TS variables before submitting the kernels
        maxthreadWidth = m_mfeEncodeSharedState->maxTheadWidth;
        maxthreadHeight = m_mfeEncodeSharedState->maxTheadHeight;
    }

    if ((!m_mfeEnabled) || (m_mfeLastStream))
    {
        uint32_t threadCount = maxthreadWidth * maxthreadHeight * m_numberConcurrentGroup;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetThreadCount(threadCount));
    }

    // setup curbe, setup surfaces and send all kernel args
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitCurbeDataB());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupSurfacesB());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupKernelArgsB());

    if (m_mfeEnabled && (!m_mfeLastStream))
    {
        //Only last stream need to submit the kernels.
        return eStatus;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateThreadSpace(
        maxthreadWidth,
        maxthreadHeight,
        m_threadSpace));

    m_threadSpace->SetThreadSpaceColorCount(totalColor);

    switch (m_swScoreboardState->GetDependencyPattern())
    {
    case dependencyWavefront26Degree:
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_threadSpace->SelectMediaWalkingPattern(CM_WALK_WAVEFRONT26));
        break;
    case dependencyWavefront26ZDegree:
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_threadSpace->SelectMediaWalkingPattern(CM_WALK_WAVEFRONT26ZIG));
        break;
    case dependencyWavefront26DDegree:
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_threadSpace->SelectMediaWalkingPattern(CM_WALK_WAVEFRONT26D));
        break;
    case dependencyWavefront26XDDegree:
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_threadSpace->SelectMediaWalkingPattern(CM_WALK_WAVEFRONT26XD));
        break;
    case dependencyWavefront45XDDegree:
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_threadSpace->SelectMediaWalkingPattern(CM_WALK_WAVEFRONT45XD_2));
        break;
    case dependencyWavefront45DDegree:
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_threadSpace->SelectMediaWalkingPattern(CM_WALK_WAVEFRONT45D));
        break;
    case dependencyWavefront45Degree:
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_threadSpace->SelectMediaWalkingPattern(CM_WALK_WAVEFRONT));
        break;
    case dependencyWavefront26XDegree:
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_threadSpace->SelectMediaWalkingPattern(CM_WALK_WAVEFRONT26X));
        break;
    case dependencyWavefront26XDegreeAlt:
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_threadSpace->SelectMediaWalkingPattern(CM_WALK_WAVEFRONT26XALT));
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Walking pattern is not supported right now");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->AssociateThreadSpace(m_threadSpace));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmTask->AddKernel(cmKrn));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CmEvent * event = CM_NO_EVENT;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmQueue->EnqueueFast(m_cmTask, event));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmTask->Reset());
        m_lastTaskInPhase = false;
    }
    else
    {
        m_cmTask->AddSync();
    }

    CODECHAL_DEBUG_TOOL(
        CODEC_REF_LIST currRefList;
        currRefList        = *(m_refList[m_currReconstructedPic.FrameIdx]);
        currRefList.RefPic = m_currOriginalPic;

        m_debugInterface->m_currPic            = m_currOriginalPic;
        m_debugInterface->m_bufferDumpFrameNum = m_storeData;
        m_debugInterface->m_frameType          = m_pictureCodingType;

        DumpMbEncPakOutput(&currRefList, m_debugInterface);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_encBCombinedBuffer2[m_currRecycledBufIdx].sResource,
            CodechalDbgAttr::attrOutput,
            "HistoryOut",
            m_historyOutBufferSize,
            m_historyOutBufferOffset,
            CODECHAL_MEDIA_STATE_HEVC_B_MBENC));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_encBCombinedBuffer2[m_currRecycledBufIdx].sResource,
            CodechalDbgAttr::attrOutput,
            "CombinedBuffer2",
            m_encBCombinedBuffer2[m_currRecycledBufIdx].dwSize,
            0,
            CODECHAL_MEDIA_STATE_HEVC_B_MBENC));
    )

#if 0 // the dump should be done in the GetStatusReport. However, if ENC causes PAK hangs-up, there is no way to get them.
    {
        //CODECHAL_DEBUG_TOOL(
        currRefList = *(m_refList[m_currReconstructedPic.FrameIdx]);
        currRefList.RefPic = m_currOriginalPic;

        m_debugInterface->m_currPic = m_currOriginalPic;
        m_debugInterface->m_bufferDumpFrameNum = m_storeData;
        m_debugInterface->m_frameType = m_pictureCodingType;

        DumpMbEncPakOutput(&currRefList, m_debugInterface);

        //CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgDumpEncodeMbEncMbPakOutput(
        //    m_debugInterface,
        //    this,
        //    &currRefList,
        //    (m_codecFunction != CODECHAL_FUNCTION_HYBRIDPAK) ?
        //    CODECHAL_MEDIA_STATE_ENC_NORMAL : CODECHAL_MEDIA_STATE_HYBRID_PAK_P2));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &currRefList.resRefMbCodeBuffer,
            CodechalDbgAttr::attrOutput,
            "MbCode",
            m_picWidthInMb * m_frameFieldHeightInMb * 64,
            CodecHal_PictureIsBottomField(currRefList.RefPic) ? m_frameFieldHeightInMb * m_picWidthInMb * 64 : 0,
            (m_codecFunction != CODECHAL_FUNCTION_HYBRIDPAK) ?
            CODECHAL_MEDIA_STATE_ENC_NORMAL : CODECHAL_MEDIA_STATE_HYBRID_PAK_P2));

        if (m_mvDataSize)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &currRefList.resRefMvDataBuffer,
                CodechalDbgAttr::attrOutput,
                "MbData",
                m_picWidthInMb * m_frameFieldHeightInMb * (32 * 4),
                CodecHal_PictureIsBottomField(currRefList.RefPic) ? MOS_ALIGN_CEIL(m_frameFieldHeightInMb * m_picWidthInMb * (32 * 4), 0x1000) : 0,
                (m_codecFunction != CODECHAL_FUNCTION_HYBRIDPAK) ?
                CODECHAL_MEDIA_STATE_ENC_NORMAL : CODECHAL_MEDIA_STATE_HYBRID_PAK_P2));
        }
        if (CodecHalIsFeiEncode(m_codecFunction))
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_resDistortionBuffer,
                CodechalDbgAttr::attrOutput,
                "DistortionSurf",
                m_picWidthInMb * m_frameFieldHeightInMb * 48,
                CodecHal_PictureIsBottomField(currRefList.RefPic) ? MOS_ALIGN_CEIL(m_frameFieldHeightInMb * m_picWidthInMb * 48, 0x1000) : 0,
                (m_codecFunction != CODECHAL_FUNCTION_HYBRIDPAK) ?
                CODECHAL_MEDIA_STATE_ENC_NORMAL : CODECHAL_MEDIA_STATE_HYBRID_PAK_P2));
        }
    }
#endif
    return eStatus;
}

// ------------------------------------------------------------------------------
//| Purpose:    Setup Curbe for HEVC MbEnc I Kernels
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodecHalHevcMbencG12::InitCurbeDataB()
{
    uint32_t            curIdx = m_currRecycledBufIdx;
    MOS_LOCK_PARAMS lockFlags;
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    uint8_t tuMapping = ((m_hevcSeqParams->TargetUsage) / 3) % 3;  // Map TU 1,4,6 to 0,1,2

    // Initialize the CURBE data
    MBENC_CURBE curbe;

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_CQP)
    {
        curbe.QPType = QP_TYPE_CONSTANT;
        curbe.ROIEnable = (m_hevcPicParams->NumROI || m_mbQpDataEnabled) ? true : false;
    }
    else
    {
        curbe.QPType = m_lcuBrcEnabled ? QP_TYPE_CU_LEVEL : QP_TYPE_FRAME;
    }

    // TU based settings
    curbe.EnableCu64Check = m_tuSettings[EnableCu64CheckTuParam][tuMapping];
    curbe.MaxNumIMESearchCenter = m_tuSettings[MaxNumIMESearchCenterTuParam][tuMapping];
    curbe.MaxTransformDepthInter = m_tuSettings[Log2TUMaxDepthInterTuParam][tuMapping];
    curbe.MaxTransformDepthIntra = m_tuSettings[Log2TUMaxDepthIntraTuParam][tuMapping];
    curbe.Dynamic64Order = m_tuSettings[Dynamic64OrderTuParam][tuMapping];
    curbe.DynamicOrderTh = m_tuSettings[DynamicOrderThTuParam][tuMapping];
    curbe.Dynamic64Enable = m_tuSettings[Dynamic64EnableTuParam][tuMapping];
    curbe.Dynamic64Th = m_tuSettings[Dynamic64ThTuParam][tuMapping];
    curbe.IncreaseExitThresh = m_tuSettings[IncreaseExitThreshTuParam][tuMapping];
    curbe.IntraSpotCheck = m_tuSettings[IntraSpotCheckFlagTuParam][tuMapping];
    curbe.Fake32Enable = m_tuSettings[Fake32EnableTuParam][tuMapping];
    curbe.Dynamic64Min32         = m_tuSettings[Dynamic64Min32][tuMapping];

    curbe.FrameWidthInSamples = m_frameWidth;
    curbe.FrameHeightInSamples = m_frameHeight;

    curbe.Log2MaxCUSize = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
    curbe.Log2MinCUSize = m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3;
    curbe.Log2MaxTUSize = m_hevcSeqParams->log2_max_transform_block_size_minus2 + 2;
    curbe.Log2MinTUSize = m_hevcSeqParams->log2_min_transform_block_size_minus2 + 2;

    curbe.ChromaFormatType = m_hevcSeqParams->chroma_format_idc;

    curbe.TUDepthControl = curbe.MaxTransformDepthInter;

    int32_t sliceQp   = m_hevcSliceParams->slice_qp_delta + m_hevcPicParams->QpY;
    curbe.FrameQP = abs(sliceQp);
    curbe.FrameQPSign = (sliceQp > 0) ? 0 : 1;

#if 0 // no need in the optimized kernel because kernel does the table look-up
    LoadCosts(CODECHAL_HEVC_B_SLICE, (uint8_t)sliceQp);
    curbe.DW4_ModeIntra32x32Cost = m_modeCostCre[LUTCREMODE_INTRA_32X32];
    curbe.DW4_ModeIntraNonDC32x32Cost = m_modeCostCre[LUTCREMODE_INTRA_NONDC_32X32];

    curbe.DW5_ModeIntra16x16Cost = m_modeCostCre[LUTCREMODE_INTRA_16X16];
    curbe.DW5_ModeIntraNonDC16x16Cost = m_modeCostCre[LUTCREMODE_INTRA_NONDC_16X16];
    curbe.DW5_ModeIntra8x8Cost = m_modeCostCre[LUTCREMODE_INTRA_8X8];
    curbe.DW5_ModeIntraNonDC8x8Cost = m_modeCostCre[LUTCREMODE_INTRA_NONDC_8X8];

    curbe.DW6_ModeIntraNonPred = m_modeCostCre[LUTCREMODE_INTRA_NONPRED];

    curbe.DW7_ChromaIntraModeCost = m_modeCostCre[LUTCREMODE_INTRA_CHROMA];

    curbe.DW12_IntraModeCostMPM = m_modeCostRde[LUTRDEMODE_INTRA_MPM];

    curbe.DW13_IntraTUDept0Cost = m_modeCostRde[LUTRDEMODE_TU_DEPTH_0];
    curbe.DW13_IntraTUDept1Cost = m_modeCostRde[LUTRDEMODE_TU_DEPTH_1];

    curbe.DW14_IntraTU4x4CBFCost = m_modeCostRde[LUTRDEMODE_INTRA_CBF_4X4];
    curbe.DW14_IntraTU8x8CBFCost = m_modeCostRde[LUTRDEMODE_INTRA_CBF_8X8];
    curbe.DW14_IntraTU16x16CBFCost = m_modeCostRde[LUTRDEMODE_INTRA_CBF_16X16];
    curbe.DW14_IntraTU32x32CBFCost = m_modeCostRde[LUTRDEMODE_INTRA_CBF_32X32];
    curbe.DW15_LambdaRD = (uint16_t)m_lambdaRD;
    curbe.DW17_IntraNonDC8x8Penalty = m_modeCostRde[LUTRDEMODE_INTRA_NONDC_8X8];
    curbe.DW17_IntraNonDC32x32Penalty = m_modeCostRde[LUTRDEMODE_INTRA_NONDC_32X32];
#endif

    curbe.NumofColumnTile = m_hevcPicParams->num_tile_columns_minus1 + 1;
    curbe.NumofRowTile    = m_hevcPicParams->num_tile_rows_minus1 + 1;

    curbe.HMEFlag = m_hmeSupported ? 3 : 0;

    curbe.MaxRefIdxL0 = CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L0_REF_G10 - 1;
    curbe.MaxRefIdxL1 = CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L1_REF_G10 - 1;
    curbe.MaxBRefIdxL0 = CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L0_REF_G10 - 1;

    // Check whether Last Frame is I frame or not
    if (m_frameNum && m_lastPictureCodingType == I_TYPE)
    {
        // This is the flag to notify kernel not to use the history buffer
        curbe.LastFrameIsIntra = true;
    }
    else
    {
        curbe.LastFrameIsIntra = false;
    }

    curbe.SliceType             = (m_hevcPicParams->CodingType == P_TYPE) ? PicCodingTypeToSliceType(B_TYPE) : PicCodingTypeToSliceType(m_hevcPicParams->CodingType);
    curbe.TemporalMvpEnableFlag = m_hevcSliceParams->slice_temporal_mvp_enable_flag;
    curbe.CollocatedFromL0Flag  = m_hevcSliceParams->collocated_from_l0_flag;
    curbe.theSameRefList        = m_sameRefList;
    curbe.IsLowDelay            = m_lowDelay;
    curbe.NumRefIdxL0           = m_hevcSliceParams->num_ref_idx_l0_active_minus1 + 1;
    curbe.NumRefIdxL1           = m_hevcSliceParams->num_ref_idx_l1_active_minus1 + 1;
    if (m_hevcSeqParams->TargetUsage == 1)
    {
        // MaxNumMergeCand C Model uses 4 for TU1, 
        // for quality consideration, make sure not larger than the value from App as it will be used in PAK
        curbe.MaxNumMergeCand   = MOS_MIN(m_hevcSliceParams->MaxNumMergeCand, 4);        
    }
    else
    {
        // MaxNumMergeCand C Model uses 2 for TU4,7 
        // for quality consideration, make sure not larger than the value from App as it will be used in PAK
       curbe.MaxNumMergeCand   = MOS_MIN(m_hevcSliceParams->MaxNumMergeCand, 2);        
    }

    int32_t tbRefListL0[CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L0_REF_G10] = { 0 }, tbRefListL1[CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L1_REF_G10] = { 0 };
    curbe.FwdPocNumber_L0_mTb_0 = tbRefListL0[0] = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[0][0]);
    curbe.BwdPocNumber_L1_mTb_0 = tbRefListL1[0] = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[1][0]);
    curbe.FwdPocNumber_L0_mTb_1 = tbRefListL0[1] = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[0][1]);
    curbe.BwdPocNumber_L1_mTb_1 = tbRefListL1[1] = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[1][1]);

    curbe.FwdPocNumber_L0_mTb_2 = tbRefListL0[2] = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[0][2]);
    curbe.BwdPocNumber_L1_mTb_2 = tbRefListL1[2] = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[1][2]);
    curbe.FwdPocNumber_L0_mTb_3 = tbRefListL0[3] = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[0][3]);
    curbe.BwdPocNumber_L1_mTb_3 = tbRefListL1[3] = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[1][3]);

    curbe.RefFrameWinHeight = m_frameHeight;
    curbe.RefFrameWinWidth = m_frameWidth;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetRoundingIntraInterToUse());
    
    curbe.RoundingInter      = (m_roundingInterInUse + 1) << 4;  // Should be an input from par in the cmodel (slice state)
    curbe.RoundingIntra      = (m_roundingIntraInUse + 1) << 4;  // Should be an input from par in the cmodel (slice state)
    curbe.RDEQuantRoundValue = (m_roundingInterInUse + 1) << 4;

    uint32_t gopB = m_hevcSeqParams->GopRefDist;

    curbe.CostScalingForRA = (m_hevcSeqParams->LowDelayMode) ? 0 : 1;

    // get the min distance between current pic and ref pics
    uint32_t minPocDist     = 255;
    uint32_t costTableIndex = 0;

    if (curbe.SliceType == CODECHAL_ENCODE_HEVC_B_SLICE)
    {
        if (curbe.CostScalingForRA == 1)
        {
            for (uint8_t ref = 0; ref < curbe.NumRefIdxL0; ref++)
            {
                if ((uint32_t)abs(tbRefListL0[ref]) < minPocDist)
                    minPocDist = abs(tbRefListL0[ref]);
            }
            for (uint8_t ref = 0; ref < curbe.NumRefIdxL1; ref++)
            {
                if ((uint32_t)abs(tbRefListL1[ref]) < minPocDist)
                    minPocDist = abs(tbRefListL1[ref]);
            }

            if (gopB == 4)
            {
                costTableIndex = minPocDist;
                if (minPocDist == 4)
                    costTableIndex -= 1;
            }
            if (gopB == 8)
            {
                costTableIndex = minPocDist + 3;
                if (minPocDist == 4)
                    costTableIndex -= 1;
                if (minPocDist == 8)
                    costTableIndex -= 4;
            }
        }
    }
    else if (curbe.SliceType == CODECHAL_ENCODE_HEVC_P_SLICE)
    {
        costTableIndex = 8;
    }
    else
    {
        costTableIndex = 9;
    }

    curbe.CostTableIndex = costTableIndex;

    // the following fields are needed by the new optimized kernel in v052417
    curbe.Log2ParallelMergeLevel  = m_hevcPicParams->log2_parallel_merge_level_minus2 + 2;
    curbe.MaxIntraRdeIter = 1;
    curbe.CornerNeighborPixel = 0;
    curbe.IntraNeighborAvailFlags = 0;
    curbe.SubPelMode = 3; // qual-pel search
    curbe.InterSADMeasure = 2; // Haar transform
    curbe.IntraSADMeasure = 2; // Haar transform
    curbe.IntraPrediction = 0; // enable 32x32, 16x16, and 8x8 luma intra prediction
    curbe.RefIDCostMode = 1; // 0: AVC and 1: linear method
    curbe.TUBasedCostSetting = 0;
    curbe.ConcurrentGroupNum = m_numberConcurrentGroup;
    curbe.NumofUnitInWaveFront = m_numWavefrontInOneRegion;
    curbe.LoadBalenceEnable = 0; // when this flag is false, kernel does not use LoadBalance (or MBENC_B_FRAME_CONCURRENT_TG_DATA) buffe
    curbe.ThreadNumber = MOS_MIN(2, m_numberEncKernelSubThread);
    curbe.Pic_init_qp_B           = m_hevcSliceParams->slice_qp_delta + m_hevcPicParams->QpY;
    curbe.Pic_init_qp_P           = m_hevcSliceParams->slice_qp_delta + m_hevcPicParams->QpY;
    curbe.Pic_init_qp_I           = m_hevcSliceParams->slice_qp_delta + m_hevcPicParams->QpY;
    curbe.SuperHME = m_16xMeSupported;
    curbe.UltraHME = m_32xMeSupported;
    curbe.EnableCu64Check         = (m_hevcSeqParams->TargetUsage == 1);
    curbe.PerBFrameQPOffset = 0;

    switch (m_hevcSeqParams->TargetUsage)
    {
    case 1:
        curbe.Degree45 = 0;
        curbe.Break12Dependency = 0;
        curbe.DisableTemporal16and8 = 0;
        break;
    case 4:
        curbe.Degree45              = 1;
        curbe.Break12Dependency     = 1;
        curbe.DisableTemporal16and8 = 0;
        break;
    default:
        curbe.Degree45              = 1;
        curbe.Break12Dependency     = 1;
        curbe.DisableTemporal16and8 = 1;
        break;
    }

    curbe.WaveFrontSplitsEnable     = curbe.Degree45;  // when 45 degree, enable wave front split
    curbe.LongTermReferenceFlags_L0 = 0;
    for (uint32_t i = 0; i < curbe.NumRefIdxL0; i++)
    {
        curbe.LongTermReferenceFlags_L0 |= (m_hevcSliceParams->RefPicList[0][i].PicFlags & PICTURE_LONG_TERM_REFERENCE) << i;
    }
    curbe.LongTermReferenceFlags_L1 = 0;
    for (uint32_t i = 0; i < curbe.NumRefIdxL1; i++)
    {
        curbe.LongTermReferenceFlags_L1 |= (m_hevcSliceParams->RefPicList[1][i].PicFlags & PICTURE_LONG_TERM_REFERENCE) << i;
    }

    curbe.Stepping = 0;
    curbe.Cu64SkipCheckOnly = 0;
    curbe.Cu642Nx2NCheckOnly = 0;
    curbe.EnableCu64AmpCheck = 1;
    curbe.IntraSpeedMode = 0; // 35 mode
    curbe.DisableIntraNxN = 0;

#if 0 //needed only when using A stepping on simu/emu
    if (m_hwInterface->GetPlatform().usRevId == 0)
    {
        curbe.Stepping = 1;
        curbe.TUDepthControl = 1;
        curbe.MaxTransformDepthInter = 1;
        curbe.MaxTransformDepthIntra = 0;
        //buf->curbe.EnableCu64Check       = 1;
        curbe.Cu64SkipCheckOnly = 0;
        curbe.Cu642Nx2NCheckOnly = 1;
        curbe.EnableCu64AmpCheck = 0;
        curbe.DisableIntraNxN = 1;
        curbe.MaxNumMergeCand = 1;
    }
#endif

    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    auto buf = (PMBENC_COMBINED_BUFFER1)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_encBCombinedBuffer1[curIdx].sResource,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(buf);

    if (curbe.Degree45)
    {
        MOS_ZeroMemory(&buf->concurrent, sizeof(buf->concurrent));
    }
    buf->Curbe = curbe;

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_encBCombinedBuffer1[curIdx].sResource);

    // clean-up the thread dependency buffer in the second combined buffer
    if (m_numberEncKernelSubThread > 1)
    {
        MOS_LOCK_PARAMS lockFlags;

        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        auto data = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_encBCombinedBuffer2[curIdx].sResource,
            &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(&data[m_threadTaskBufferOffset], m_threadTaskBufferSize);

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_encBCombinedBuffer2[curIdx].sResource);
    }

    if (m_initEncConstTable)
    {
        // Initialize the Enc Constant Table surface
        if (m_isMaxLcu64)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_constTableB->WriteSurface(
                (unsigned char *)m_encLcu64ConstantDataLut,
                nullptr,
                sizeof(m_encLcu64ConstantDataLut)));
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_constTableB->WriteSurface(
                (unsigned char *)m_encLcu32ConstantDataLut,
                nullptr,
                sizeof(m_encLcu32ConstantDataLut)));
        }
        m_initEncConstTable = false;
    }

    return eStatus;
}

MOS_STATUS CodecHalHevcMbencG12::SetupSurfacesB()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    //Concurrent Thread Group Data
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->UpdateBuffer(
        &m_encBCombinedBuffer1[m_currRecycledBufIdx].sResource,
        m_combinedBuffer1));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->UpdateBuffer(
        &m_encBCombinedBuffer2[m_currRecycledBufIdx].sResource,
        m_combinedBuffer2));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_encBCombinedBuffer1[m_currRecycledBufIdx].sResource,
            CodechalDbgAttr::attrOutput,
            "CombinedBuffer1",
            m_encBCombinedBuffer1[m_currRecycledBufIdx].dwSize,
            0,
            CODECHAL_MEDIA_STATE_HEVC_B_MBENC));
    )

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_encBCombinedBuffer2[m_currRecycledBufIdx].sResource,
            CodechalDbgAttr::attrOutput,
            "HistoryIn",
            sizeof(MBENC_COMBINED_BUFFER2::ucHistoryInBuffer),
            sizeof(MBENC_COMBINED_BUFFER2::ucBrcCombinedEncBuffer),
            CODECHAL_MEDIA_STATE_HEVC_B_MBENC));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_encBCombinedBuffer2[m_currRecycledBufIdx].sResource,
            CodechalDbgAttr::attrOutput,
            "ThreadTask",
            m_threadTaskBufferSize,
            m_threadTaskBufferOffset,
            CODECHAL_MEDIA_STATE_HEVC_B_MBENC));)

    PMOS_SURFACE inputSurface = m_rawSurfaceToEnc;

    // Cur and VME surfaces
    //Source Y and UV
    //first create the 2D cur input surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->UpdateSurface2D(
        &inputSurface->OsResource,
        m_curSurf));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            inputSurface,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "MbEnc_Input_SrcSurf")));

    if (m_curVme)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroyVmeSurfaceG7_5(m_curVme));
        m_curVme = nullptr;
    }

    for (int32_t surface_idx = 0; surface_idx < 4; surface_idx++)
    {
        int32_t ll = 0;
        CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[ll][surface_idx];
        if (!CodecHal_PictureIsInvalid(refPic) &&
            !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
        {
            int32_t idx = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;

            PMOS_SURFACE refSurfacePtr = nullptr;
            if (surface_idx == 0 && m_useWeightedSurfaceForL0)
            {
                refSurfacePtr = m_wpState->GetWPOutputPicList(CODEC_WP_OUTPUT_L0_START + surface_idx);
            }
            else
            {
                refSurfacePtr = &m_refList[idx]->sRefBuffer;
            }

            // Picture Y VME
            //m_hwInterface->CacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->UpdateSurface2D(
                &refSurfacePtr->OsResource,
                m_surfRefArray[surface_idx]));

            CODECHAL_DEBUG_TOOL(
                m_debugInterface->m_refIndex = (uint16_t)refPic.FrameIdx;
                std::string refSurfName      = "RefSurf" + std::to_string(static_cast<uint32_t>(m_debugInterface->m_refIndex)) +
                                               "_L0" + std::to_string(surface_idx);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                    refSurfacePtr,
                    CodechalDbgAttr::attrReferenceSurfaces,
                    refSurfName.data())));
        }
        else
        {
            // Providing Dummy surface as per VME requirement.
            //m_hwInterface->CacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->UpdateSurface2D(
                &inputSurface->OsResource,
                m_surfRefArray[surface_idx]));
        }

        ll = 1;
        refPic = m_hevcSliceParams->RefPicList[ll][surface_idx];
        if (!CodecHal_PictureIsInvalid(refPic) &&
            !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
        {
            int32_t idx = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;

            PMOS_SURFACE refSurfacePtr = nullptr;
            if (surface_idx == 0 && m_useWeightedSurfaceForL1)
            {
                refSurfacePtr = m_wpState->GetWPOutputPicList(CODEC_WP_OUTPUT_L1_START + surface_idx);
            }
            else
            {
                refSurfacePtr = &m_refList[idx]->sRefBuffer;
            }

            // Picture Y VME
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->UpdateSurface2D(
                &refSurfacePtr->OsResource,
                m_surfRefArray[MAX_VME_BWD_REF + surface_idx]));

            CODECHAL_DEBUG_TOOL(
                m_debugInterface->m_refIndex = (uint16_t)refPic.FrameIdx;
                std::string refSurfName      = "RefSurf" + std::to_string(static_cast<uint32_t>(m_debugInterface->m_refIndex)) +
                                               "_L1" + std::to_string(surface_idx);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                    refSurfacePtr,
                    CodechalDbgAttr::attrEncodeRawInputSurface,
                    refSurfName.data())));
        }
        else
        {
            // Providing Dummy surface as per VME requirement.
            //m_hwInterface->CacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->UpdateSurface2D(
                &inputSurface->OsResource,
                m_surfRefArray[MAX_VME_BWD_REF + surface_idx]));
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateVmeSurfaceG7_5(
        m_curSurf,
        &m_surfRefArray[0],
        &m_surfRefArray[MAX_VME_BWD_REF],
        MAX_VME_FWD_REF,
        MAX_VME_BWD_REF,
        m_curVme));

    /* WA for 16k resolution tests with P010 format. Recon surface is NV12 format with width=2*original_width 
       32k width is not supported by MEDIA_SURFACE_STATE_CMD. 
       We can therefore change the recon dimensions to 16k width and 32k pitch,
       this will cover the portion of the surface that VME uses */
    if (MEDIA_IS_WA(m_waTable, Wa16kWidth32kPitchNV12ReconForP010Input) && m_curVme && m_encode16KSequence && (uint8_t)HCP_CHROMA_FORMAT_YUV420 == m_chromaFormat && inputSurface->Format == Format_P010)
    {
       CM_VME_SURFACE_STATE_PARAM  vmeDimensionParam;
       vmeDimensionParam.width   = ENCODE_HEVC_16K_PIC_WIDTH;
       vmeDimensionParam.height  = ENCODE_HEVC_16K_PIC_HEIGHT;
       m_cmDev->SetVmeSurfaceStateParam(m_curVme, &vmeDimensionParam);
    }

    // Current Y with reconstructed boundary pixels
    if (!m_reconWithBoundaryPix)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D(
            &m_currPicWithReconBoundaryPix.OsResource,
            m_reconWithBoundaryPix));
    }

    // PAK object command surface
    if (m_mbCodeBuffer)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_mbCodeBuffer));
        m_mbCodeBuffer = nullptr;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateBuffer(
        &m_resMbCodeSurface,
        m_mbCodeBuffer));

    // PAK object command surface
    CM_BUFFER_STATE_PARAM bufParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateBufferAlias(
        m_mbCodeBuffer,
        m_mbCodeSurfIdx));
    bufParams.uiBaseAddressOffset = 0;
    bufParams.uiSize = m_mvOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mbCodeBuffer->SetSurfaceStateParam(
        m_mbCodeSurfIdx,
        &bufParams));

    // CU packet for PAK surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateBufferAlias(
        m_mbCodeBuffer,
        m_mvDataSurfIdx));
    bufParams.uiBaseAddressOffset = m_mvOffset;
    bufParams.uiSize = m_mbCodeSize - m_mvOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mbCodeBuffer->SetSurfaceStateParam(
        m_mvDataSurfIdx,
        &bufParams));

    //Software Scoreboard surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->UpdateSurface2D(
        &m_swScoreboardState->GetCurSwScoreboardSurface()->OsResource,
        m_swScoreboardSurf));

    if ((!m_mbQpDataEnabled) || (m_brcEnabled))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->UpdateSurface2D(
            &m_brcBuffers.sBrcMbQpBuffer.OsResource,
            m_cu16X16QpIn));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->UpdateSurface2D(
            &m_mbQpDataSurface.OsResource,
            m_cu16X16QpIn));
    }

    // Lcu level data input
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->UpdateSurface2D(
        &m_lcuLevelInputDataSurface[m_currRecycledBufIdx].OsResource,
        m_lcuLevelData));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
            &m_lcuLevelInputDataSurface[m_currRecycledBufIdx],
            CodechalDbgAttr::attrOutput,
            "LcuInfoSurface",
            CODECHAL_MEDIA_STATE_HEVC_B_MBENC));)

    // Colocated CU Motion Vector Data Surface
    if (m_colocCumvData)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(m_colocCumvData));
        m_colocCumvData = nullptr;
    }

    if (m_mbCodeIdxForTempMVP != 0xFF)
    {
        //m_hwInterface->CacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateBuffer(
            m_trackedBuf->GetMvTemporalBuffer(m_mbCodeIdxForTempMVP),
            m_colocCumvData));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                m_trackedBuf->GetMvTemporalBuffer(m_mbCodeIdxForTempMVP),
                CodechalDbgAttr::attrOutput,
                "CollocatedMV",
                m_sizeOfMvTemporalBuffer,
                0,
                CODECHAL_MEDIA_STATE_HEVC_B_MBENC)););
    }

    // HME motion predictor data
    if (m_hmeEnabled)
    {
        m_hmeMotionPredData = m_hmeKernel->GetCmSurface(CodechalKernelHme::SurfaceId::me4xMvDataBuffer);
    }

    if (m_isMaxLcu64)
    {
        PMOS_SURFACE currScaledSurface2x = m_trackedBuf->Get2xDsSurface(CODEC_CURR_TRACKED_BUFFER);

        //VME 2X Inter prediction Surface for current frame
        //m_hwInterface->CacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->UpdateSurface2D(
            &currScaledSurface2x->OsResource,
            m_curSurf2X));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                currScaledSurface2x,
                CodechalDbgAttr::attrReferenceSurfaces,
                "2xScaledSurf"))
        );

        if (m_cur2XVme)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroyVmeSurfaceG7_5(m_cur2XVme));
            m_cur2XVme = nullptr;
        }

        // RefFrame's 2x DS surface
        for (int32_t surface_idx = 0; surface_idx < 4; surface_idx++)
        {
            int32_t ll = 0;
            CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[ll][surface_idx];
            if (!CodecHal_PictureIsInvalid(refPic) &&
                !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
            {
                int32_t idx = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;

                // Picture Y VME
                //m_hwInterface->CacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->UpdateSurface2D(
                    &m_trackedBuf->Get2xDsSurface(m_refList[idx]->ucScalingIdx)->OsResource,
                    m_surf2XArray[surface_idx]));

                CODECHAL_DEBUG_TOOL(
                    m_debugInterface->m_refIndex = (uint16_t)refPic.FrameIdx;
                    std::string refSurfName = "Ref2xScaledSurf" + std::to_string(static_cast<uint32_t>(m_debugInterface->m_refIndex)) +
                                               "_L0" + std::to_string(surface_idx);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                        m_trackedBuf->Get2xDsSurface(m_refList[idx]->ucScalingIdx),
                        CodechalDbgAttr::attrReferenceSurfaces,
                        refSurfName.data())));
            }
            else
            {
                // Providing Dummy surface as per VME requirement.
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->UpdateSurface2D(
                    &currScaledSurface2x->OsResource,
                    m_surf2XArray[surface_idx]));
            }

            ll = 1;
            refPic = m_hevcSliceParams->RefPicList[ll][surface_idx];
            if (!CodecHal_PictureIsInvalid(refPic) &&
                !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
            {
                int32_t idx = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;

                // Picture Y VME
                //m_hwInterface->CacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->UpdateSurface2D(
                    &m_trackedBuf->Get2xDsSurface(m_refList[idx]->ucScalingIdx)->OsResource,
                    m_surf2XArray[MAX_VME_BWD_REF + surface_idx]));

                CODECHAL_DEBUG_TOOL(
                    m_debugInterface->m_refIndex = (uint16_t)refPic.FrameIdx;
                    std::string refSurfName = "Ref2xScaledSurf" + std::to_string(static_cast<uint32_t>(m_debugInterface->m_refIndex)) +
                                               "_L1" + std::to_string(surface_idx);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                        m_trackedBuf->Get2xDsSurface(m_refList[idx]->ucScalingIdx),
                        CodechalDbgAttr::attrReferenceSurfaces,
                        refSurfName.data())));
            }
            else
            {
                // Providing Dummy surface as per VME requirement.
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->UpdateSurface2D(
                    &currScaledSurface2x->OsResource,
                    m_surf2XArray[MAX_VME_BWD_REF + surface_idx]));
            }
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateVmeSurfaceG7_5(
            m_curSurf2X,
            &m_surf2XArray[0],
            &m_surf2XArray[MAX_VME_BWD_REF],
            MAX_VME_FWD_REF,
            MAX_VME_BWD_REF,
            m_cur2XVme));
    }

    if (m_isMaxLcu64)
    {
        // Encoder History Input Buffer
        if (!m_histInBuffer)
        {
            //m_hwInterface->CacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D(
                &m_encoderHistoryInputBuffer.OsResource,
                m_histInBuffer));
        }

        // Encoder History Input Buffer
        if (!m_histOutBuffer)
        {
            //m_hwInterface->CacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateSurface2D(
                &m_encoderHistoryOutputBuffer.OsResource,
                m_histOutBuffer));
        }
    }

    return eStatus;
}

MOS_STATUS CodecHalHevcMbencG12::EncodeIntraDistKernel()
{
    CodechalKernelIntraDistMdfG12::CurbeParam curbeParam;
    curbeParam.downScaledWidthInMb4x = m_downscaledWidthInMb4x;
    curbeParam.downScaledHeightInMb4x = m_downscaledHeightInMb4x;

    CodechalKernelIntraDistMdfG12::SurfaceParams surfaceParam;
    surfaceParam.input4xDsSurface = m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
    surfaceParam.intraDistSurface = m_brcBuffers.brcIntraDistortionSurface;
    surfaceParam.intraDistBottomFieldOffset = m_brcBuffers.dwMeBrcDistortionBottomFieldOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_intraDistKernel->Execute(curbeParam, surfaceParam));

    return MOS_STATUS_SUCCESS;
}

//to remove this function after the fix in CodechalEncHevcState::GetRoundingIntraInterToUse() checked in.
MOS_STATUS CodecHalHevcMbencG12::GetRoundingIntraInterToUse()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_hevcPicParams->CustomRoundingOffsetsParams.fields.EnableCustomRoudingIntra)
    {
        m_roundingIntraInUse = m_hevcPicParams->CustomRoundingOffsetsParams.fields.RoundingOffsetIntra;
    }
    else
    {
        if (m_hevcPicParams->CodingType == I_TYPE)
        {
            m_roundingIntraInUse = 10;
        }
        else if (m_HierchGopBRCEnabled)
        {
            //Hierachical B GOP
            if (m_hevcPicParams->CodingType == P_TYPE)
            {
                m_roundingIntraInUse = 4;
            }
            else if (m_hevcPicParams->CodingType == B_TYPE)
            {
                m_roundingIntraInUse = 3;
                if (m_lowDelay && !m_hevcSeqParams->LowDelayMode)
                {
                    // RAB test, anchor frame
                    m_roundingIntraInUse = 4;
                }
            }
            else
            {
                m_roundingIntraInUse = 2;
            }
        }
        else
        {
            m_roundingIntraInUse = 10;
        }
    }

    if (m_hevcPicParams->CustomRoundingOffsetsParams.fields.EnableCustomRoudingInter)
    {
        m_roundingInterInUse = m_hevcPicParams->CustomRoundingOffsetsParams.fields.RoundingOffsetInter;
    }
    else
    {
        if (m_HierchGopBRCEnabled)
        {
            //Hierachical B GOP
            if (m_hevcPicParams->CodingType == I_TYPE ||
                m_hevcPicParams->CodingType == P_TYPE)
            {
                m_roundingInterInUse = 4;
            }
            else if (m_hevcPicParams->CodingType == B_TYPE)
            {
                m_roundingInterInUse = 3;
                if (m_lowDelay && !m_hevcSeqParams->LowDelayMode)
                {
                    // RAB test, anchor frame
                    m_roundingInterInUse = 4;
                }
            }
            else
            {
                m_roundingInterInUse = 2;
            }
        }
        else
        {
            m_roundingInterInUse = 4;
        }
    }

    CODECHAL_ENCODE_VERBOSEMESSAGE("Rounding intra in use:%d, rounding inter in use:%d.\n", m_roundingIntraInUse, m_roundingInterInUse);

    return eStatus;
}
