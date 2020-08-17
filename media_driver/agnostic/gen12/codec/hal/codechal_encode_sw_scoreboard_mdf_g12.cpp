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
//! \file     codechal_encode_sw_scoreboard_g12.cpp
//! \brief    This file implements the SW scoreboard init feature for all codecs on Gen12 platform
//!

#include "codechal_encoder_base.h"
#include "codechal_encode_sw_scoreboard_mdf_g12.h"
#include "Gen12LP_Init_Scoreboard_genx.h"

//uint8_t CodechalEncodeSwScoreboardMdfG12::GetBTCount()
//{
//    return (uint8_t)swScoreboardNumSurfaces;
//}

MOS_STATUS CodechalEncodeSwScoreboardMdfG12::InitKernelState()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    if (!m_cmProgram)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->LoadProgram((void *)GEN12LP_INIT_SCOREBOARD_GENX,
            GEN12LP_INIT_SCOREBOARD_GENX_SIZE,
            m_cmProgram,
            "-nojitter"));
    }
    if (!m_cmKrn)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateKernel(m_cmProgram,
            "Init_Scoreboard",
            m_cmKrn));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeSwScoreboardMdfG12::SetCurbe(CurbeData& curbe)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    MOS_ZeroMemory(&curbe, sizeof(CurbeData));

    // ScoreboardWidth and Height in units of Thread Space
    curbe.DW0.scoreboardWidth     = m_curbeParams.scoreboardWidth;
    curbe.DW0.scoreboardHeight    = m_curbeParams.scoreboardHeight;
    curbe.DW1.isHevc                   = m_curbeParams.isHevc;
    curbe.DW2.numberOfWaveFrontsSplits = m_curbeParams.numberOfWaveFrontSplit;
    curbe.DW2.numberofChildThreads     = m_curbeParams.numberOfChildThread;
    curbe.DW4.dependencyPattern        = m_dependencyPatternIdx;

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    SW scoreboard init kernel function
//!
//! \param    [in] params
//!           Pointer to KernelParams
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodechalEncodeSwScoreboardMdfG12::Execute(KernelParams *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Allocate output surface
    m_surfaceParams.isHevc = params->isHevc;
    m_surfaceParams.swScoreboardSurfaceWidth = params->swScoreboardSurfaceWidth;
    m_surfaceParams.swScoreboardSurfaceHeight = params->swScoreboardSurfaceHeight;
    if (m_surfaceParams.isHevc)
    {
        m_surfaceParams.lcuInfoSurface = params->lcuInfoSurface;
    }
    // Setup Curbe
    m_curbeParams.scoreboardWidth = params->scoreboardWidth;
    m_curbeParams.scoreboardHeight = params->scoreboardHeight;
    m_curbeParams.isHevc = params->isHevc;
    m_curbeParams.numberOfWaveFrontSplit = params->numberOfWaveFrontSplit;
    m_curbeParams.numberOfChildThread = params->numberOfChildThread;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateResources());

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupSurfaces());

    //walkerCodecParams.WalkerMode = m_walkerMode;
    uint32_t dwResolutionX = (uint32_t)(ceil)((m_curbeParams.scoreboardWidth) / 4.0);
    uint32_t dwResolutionY = (uint32_t)(ceil)((m_curbeParams.scoreboardHeight) / 4.0);

    uint32_t threadCount = dwResolutionX * dwResolutionY;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrn->SetThreadCount(threadCount));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateThreadSpace(
        dwResolutionX,
        dwResolutionY,
        m_threadSpace));

    if (m_groupIdSelectSupported)
    {
        m_threadSpace->SetMediaWalkerGroupSelect((CM_MW_GROUP_SELECT)m_groupId);
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrn->AssociateThreadSpace(m_threadSpace));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupKernelArgs());

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmTask->AddKernel(m_cmKrn));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CmEvent * event = CM_NO_EVENT;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmQueue->EnqueueFast(m_encoder->m_cmTask, event));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmTask->Reset());
        m_lastTaskInPhase = false;
    }
    else
    {
        m_encoder->m_cmTask->AddSync();
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeSwScoreboardMdfG12::SetupKernelArgs()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    int idx = 0;
    CurbeData curbe;
    SurfaceIndex * pSurfIndex = nullptr;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbe(curbe));
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_scoreboardSurface);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpMDFCurbe(
            CODECHAL_MEDIA_STATE_SW_SCOREBOARD_INIT,
            (uint8_t*)&curbe,
            sizeof(CurbeData)));
    )

    m_cmKrn->SetKernelArg(idx++, sizeof(curbe), &curbe);
    m_scoreboardSurface->GetIndex(pSurfIndex);
    m_cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);
    if (m_lcuInfoSurface)
    {
        m_lcuInfoSurface->GetIndex(pSurfIndex);
    }
    m_cmKrn->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeSwScoreboardMdfG12::SetupSurfaces()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->UpdateSurface2D(
        &m_surfaceParams.swScoreboardSurface[m_surfaceParams.surfaceIndex].OsResource,
        m_scoreboardSurface));

    if (m_surfaceParams.isHevc)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->UpdateSurface2D(
            &m_surfaceParams.lcuInfoSurface->OsResource,
            m_lcuInfoSurface));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeSwScoreboardMdfG12::ReleaseResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    if (m_scoreboardSurface)
    {
        m_scoreboardSurface->NotifyUmdResourceChanged(nullptr);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->DestroySurface(m_scoreboardSurface));
        m_scoreboardSurface = nullptr;
    }

    if (m_lcuInfoSurface)
    {
        m_lcuInfoSurface->NotifyUmdResourceChanged(nullptr);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->DestroySurface(m_lcuInfoSurface));
        m_lcuInfoSurface = nullptr;
    }

    if (m_threadSpace)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->DestroyThreadSpace(m_threadSpace));
        m_threadSpace = nullptr;
    }

    if (m_cmKrn)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->DestroyKernel(m_cmKrn));
        m_cmKrn = nullptr;
    }

    if (m_cmProgram)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->DestroyProgram(m_cmProgram));
        m_cmProgram = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}
