/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     decode_scalability_multipipe_next.cpp
//! \brief    Defines the common interface for decode scalability multipipe mode.
//!

#include "decode_scalability_multipipe_next.h"

#include "media_context.h"
#include "media_status_report.h"
#include "mhw_utilities.h"
#include "decode_status_report_defs.h"
#include "mos_interface.h"
#include "mos_os_virtualengine_next.h"

namespace decode
{
DecodeScalabilityMultiPipeNext::DecodeScalabilityMultiPipeNext(void *hwInterface, MediaContext *mediaContext, uint8_t componentType)
    : MediaScalabilityMultiPipe(mediaContext)
{
    m_hwInterface   = (CodechalHwInterfaceNext *)hwInterface;
    m_componentType = componentType;
    m_secondaryCmdBuffers.clear();
    m_resSemaphoreAllPipes.clear();
    m_resSemaphoreOnePipeWait.clear();
}

DecodeScalabilityMultiPipeNext::~DecodeScalabilityMultiPipeNext()
{
}

MOS_STATUS DecodeScalabilityMultiPipeNext::AllocateSemaphore()
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    m_secondaryCmdBuffers.resize(m_initSecondaryCmdBufNum);

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.dwBytes  = sizeof(uint32_t);
    allocParamsForBufferLinear.pBufName = "Sync All Pipes SemaphoreMemory";

    m_resSemaphoreAllPipes.resize(m_maxCmdBufferSetsNum);
    for (auto &semaphoreBufferVec : m_resSemaphoreAllPipes)
    {
        semaphoreBufferVec.resize(m_scalabilityOption->GetNumPipe());
        for (auto &semaphoreBuffer : semaphoreBufferVec)
        {
            memset(&semaphoreBuffer, 0, sizeof(MOS_RESOURCE));
            SCALABILITY_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                      m_osInterface,
                                                      &allocParamsForBufferLinear,
                                                      &semaphoreBuffer),
                "Cannot create HW semaphore for scalability all pipes sync.");
            uint32_t *data = (uint32_t *)m_osInterface->pfnLockResource(
                m_osInterface,
                &semaphoreBuffer,
                &lockFlagsWriteOnly);
            SCALABILITY_CHK_NULL_RETURN(data);
            *data = 0;
            SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
                m_osInterface,
                &semaphoreBuffer));
        }
    }

    allocParamsForBufferLinear.pBufName = "Sync One Pipe Wait SemaphoreMemory";
    m_resSemaphoreOnePipeWait.resize(m_maxCmdBufferSetsNum);
    for (auto &semaphoreBufferVec : m_resSemaphoreOnePipeWait)
    {
        semaphoreBufferVec.resize(m_scalabilityOption->GetNumPipe());
        for (auto &semaphoreBuffer : semaphoreBufferVec)
        {
            memset(&semaphoreBuffer, 0, sizeof(MOS_RESOURCE));
            SCALABILITY_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                      m_osInterface,
                                                      &allocParamsForBufferLinear,
                                                      &semaphoreBuffer),
                "Cannot create HW semaphore for scalability one pipe sync.");
            uint32_t *data = (uint32_t *)m_osInterface->pfnLockResource(
                m_osInterface,
                &semaphoreBuffer,
                &lockFlagsWriteOnly);
            SCALABILITY_CHK_NULL_RETURN(data);
            *data = 0;
            SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
                m_osInterface,
                &semaphoreBuffer));
        }
    }

    m_semaphoreIndex = 0;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipeNext::Initialize(const MediaScalabilityOption &option)
{
    SCALABILITY_FUNCTION_ENTER;

    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);
    m_osInterface = m_hwInterface->GetOsInterface();
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);
    m_miItf = m_hwInterface->GetMiInterfaceNext();
    SCALABILITY_CHK_NULL_RETURN(m_miItf);

    DecodeScalabilityOption *decodeScalabilityOption = MOS_New(DecodeScalabilityOption, (const DecodeScalabilityOption &)option);
    SCALABILITY_CHK_NULL_RETURN(decodeScalabilityOption);
    m_scalabilityOption = decodeScalabilityOption;

    m_frameTrackingEnabled = m_osInterface->bEnableKmdMediaFrameTracking ? true : false;
    //virtual engine init with scalability
    MOS_VIRTUALENGINE_INIT_PARAMS veInitParms;
    MOS_ZeroMemory(&veInitParms, sizeof(veInitParms));
    veInitParms.bScalabilitySupported          = true;
    veInitParms.bFESeparateSubmit              = decodeScalabilityOption->IsFESeparateSubmission();
    veInitParms.ucMaxNumPipesInUse             = decodeScalabilityOption->GetMaxMultiPipeNum();
    veInitParms.ucNumOfSdryCmdBufSets          = m_maxCmdBufferSetsNum;
    veInitParms.ucMaxNumOfSdryCmdBufInOneFrame = decodeScalabilityOption->IsFESeparateSubmission() ?
                                                 veInitParms.ucMaxNumPipesInUse : (veInitParms.ucMaxNumPipesInUse + 1);
    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        MOS_STATUS status = m_osInterface->pfnVirtualEngineInit(m_osInterface, &m_veHitParams, veInitParms);
        SCALABILITY_CHK_STATUS_MESSAGE_RETURN(status, "Virtual Engine Init failed");
        m_veInterface = m_osInterface->pVEInterf;
        if (m_osInterface->osStreamState && m_osInterface->osStreamState->virtualEngineInterface)
        {
            // we set m_veState here when pOsInterface->apoMosEnabled is true
            m_veState = m_osInterface->osStreamState->virtualEngineInterface;
        }
    }

    m_pipeNum = m_scalabilityOption->GetNumPipe();
    m_pipeIndexForSubmit = m_pipeNum;

    PMOS_GPUCTX_CREATOPTIONS_ENHANCED gpuCtxCreateOption = MOS_New(MOS_GPUCTX_CREATOPTIONS_ENHANCED);
    SCALABILITY_CHK_NULL_RETURN(gpuCtxCreateOption);
    gpuCtxCreateOption->LRCACount = decodeScalabilityOption->GetLRCACount();
    gpuCtxCreateOption->UsingSFC  = decodeScalabilityOption->IsUsingSFC();
    if (decodeScalabilityOption->IsUsingSlimVdbox())
    {
        gpuCtxCreateOption->Flags |=  (1 << 2);
    }
#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_osInterface->bEnableDbgOvrdInVE)
    {
        gpuCtxCreateOption->DebugOverride = true;
        uint8_t engineLogicId             = 0;
        if (m_osInterface->pfnGetEngineLogicId(m_osInterface, engineLogicId) == MOS_STATUS_SUCCESS)
        {
            gpuCtxCreateOption->EngineInstance[0] = engineLogicId;
        }
    }
#endif
    m_gpuCtxCreateOption = (PMOS_GPUCTX_CREATOPTIONS)(gpuCtxCreateOption);

    //Allocate and init for semaphores
    SCALABILITY_CHK_STATUS_RETURN(AllocateSemaphore());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipeNext::GetGpuCtxCreationOption(MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(gpuCtxCreateOption);
    SCALABILITY_CHK_NULL_RETURN(m_gpuCtxCreateOption);

    MOS_GPUCTX_CREATOPTIONS_ENHANCED *dest = dynamic_cast<MOS_GPUCTX_CREATOPTIONS_ENHANCED *>(gpuCtxCreateOption);
    MOS_GPUCTX_CREATOPTIONS_ENHANCED *source = dynamic_cast<MOS_GPUCTX_CREATOPTIONS_ENHANCED *>(m_gpuCtxCreateOption);

    SCALABILITY_CHK_NULL_RETURN(dest);
    SCALABILITY_CHK_NULL_RETURN(source);

    *dest = *source;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipeNext::Destroy()
{
    SCALABILITY_FUNCTION_ENTER;

    SCALABILITY_CHK_STATUS_RETURN(MediaScalability::Destroy());

    if (m_gpuCtxCreateOption)
    {
        MOS_Delete(m_gpuCtxCreateOption);
    }
    if (m_scalabilityOption)
    {
        MOS_Delete(m_scalabilityOption);
    }

    m_osInterface->pfnDestroyVeInterface(&m_veInterface);

    for (auto &semaphoreBufferVec : m_resSemaphoreAllPipes)
    {
        for (auto &semaphoreBuffer : semaphoreBufferVec)
        {
            m_osInterface->pfnFreeResource(m_osInterface, &semaphoreBuffer);
        }
    }
    for (auto &semaphoreBufferVec : m_resSemaphoreOnePipeWait)
    {
        for (auto &semaphoreBuffer : semaphoreBufferVec)
        {
            m_osInterface->pfnFreeResource(m_osInterface, &semaphoreBuffer);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipeNext::ResizeCommandBufferAndPatchList(
    uint32_t                    requestedCommandBufferSize,
    uint32_t                    requestedPatchListSize)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);

    return m_hwInterface->ResizeCommandBufferAndPatchList(requestedCommandBufferSize, requestedPatchListSize);
}

MOS_STATUS DecodeScalabilityMultiPipeNext::VerifySpaceAvailable(uint32_t requestedSize, uint32_t requestedPatchListSize, bool &singleTaskPhaseSupportedInPak)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    uint8_t looptimes = 3;
    for(auto i = 0 ; i < looptimes ; i++)
    {
        bool bothPatchListAndCmdBufChkSuccess = false;
        SCALABILITY_CHK_STATUS_RETURN(MediaScalability::VerifySpaceAvailable(
            requestedSize, requestedPatchListSize, bothPatchListAndCmdBufChkSuccess));
        if (bothPatchListAndCmdBufChkSuccess)
        {
            return MOS_STATUS_SUCCESS;
        }

        MOS_STATUS statusPatchList = MOS_STATUS_SUCCESS;
        if (requestedPatchListSize)
        {
            statusPatchList = (MOS_STATUS)m_osInterface->pfnVerifyPatchListSize(
                m_osInterface,
                requestedPatchListSize);
        }

        MOS_STATUS statusCmdBuf = (MOS_STATUS)m_osInterface->pfnVerifyCommandBufferSize(
            m_osInterface,
            requestedSize,
            0);

        if (statusCmdBuf == MOS_STATUS_SUCCESS && statusPatchList == MOS_STATUS_SUCCESS)
        {
            return MOS_STATUS_SUCCESS;
        }
    }

    SCALABILITY_ASSERTMESSAGE("Resize Command buffer failed with no space!");
    return MOS_STATUS_NO_SPACE;
}

MOS_STATUS DecodeScalabilityMultiPipeNext::VerifyCmdBuffer(uint32_t requestedSize, uint32_t requestedPatchListSize, bool &singleTaskPhaseSupportedInPak)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    // Verify Primary Cmd buffer.
    SCALABILITY_CHK_STATUS_RETURN(VerifySpaceAvailable(
        requestedSize, requestedPatchListSize, singleTaskPhaseSupportedInPak));

    uint8_t looptimes = 3;
    for (auto i = 0; i < looptimes; i++)
    {
        // Verify secondary cmd buffer
        if (m_osInterface->pfnVerifyCommandBufferSize(
                m_osInterface,
                requestedSize,
                MOS_VE_HAVE_SECONDARY_CMDBUFFER) == MOS_STATUS_SUCCESS)
        {
            return MOS_STATUS_SUCCESS;
        }

        SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnResizeCommandBufferAndPatchList(
            m_osInterface,
            requestedSize,
            0,
            MOS_VE_HAVE_SECONDARY_CMDBUFFER));
    }

    SCALABILITY_ASSERTMESSAGE("Verify secondary command buffer failed with no space!");
    return MOS_STATUS_NO_SPACE;
}

MOS_STATUS DecodeScalabilityMultiPipeNext::GetCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer, bool frameTrackingRequested)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);
    SCALABILITY_CHK_NULL_RETURN(m_phase);

    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &m_primaryCmdBuffer, 0));

    uint32_t bufIdx = m_phase->GetCmdBufIndex();
    SCALABILITY_ASSERT(bufIdx >= DecodePhase::m_secondaryCmdBufIdxBase);
    uint32_t secondaryIdx = bufIdx - DecodePhase::m_secondaryCmdBufIdxBase;
    if (secondaryIdx >= m_secondaryCmdBuffers.size())
    {
        m_secondaryCmdBuffers.resize(secondaryIdx + 1);
    }
    auto &scdryCmdBuffer = m_secondaryCmdBuffers[secondaryIdx];
    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &scdryCmdBuffer, bufIdx));

    if (m_osInterface->apoMosEnabled)
    {
        SCALABILITY_CHK_NULL_RETURN(m_osInterface->osStreamState);
        SCALABILITY_CHK_NULL_RETURN(m_osInterface->osStreamState->virtualEngineInterface);
        SCALABILITY_CHK_STATUS_RETURN(m_osInterface->osStreamState->virtualEngineInterface->SetSubmissionType(&scdryCmdBuffer, m_phase->GetSubmissionType()));
    }
    else
    {
        scdryCmdBuffer.iSubmissionType = m_phase->GetSubmissionType();
    }
    *cmdBuffer = scdryCmdBuffer;

    if (!m_attrReady)
    {
        SCALABILITY_CHK_STATUS_RETURN(SendAttrWithFrameTracking(m_primaryCmdBuffer, frameTrackingRequested));
        // Insert noop to primary command buffer, avoid zero length command buffer
        SCALABILITY_CHK_STATUS_RETURN(m_miItf->ADDCMD_MI_NOOP(&m_primaryCmdBuffer, nullptr));
        m_attrReady = true;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipeNext::ReturnCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);
    SCALABILITY_CHK_NULL_RETURN(m_phase);

    uint32_t bufIdx = m_phase->GetCmdBufIndex();
    SCALABILITY_ASSERT(bufIdx >= DecodePhase::m_secondaryCmdBufIdxBase);
    uint32_t secondaryIdx = bufIdx - DecodePhase::m_secondaryCmdBufIdxBase;
    SCALABILITY_ASSERT(secondaryIdx < m_secondaryCmdBuffers.size());

    m_secondaryCmdBuffers[secondaryIdx] = *cmdBuffer;  //Need to record the iOffset, ptr and other data of CMD buffer, it's not maintain in the mos.
    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &m_secondaryCmdBuffers[secondaryIdx], bufIdx);
    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &m_primaryCmdBuffer, 0);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipeNext::SetHintParams()
{
    SCALABILITY_FUNCTION_ENTER;
    MOS_STATUS               eStatus                 = MOS_STATUS_SUCCESS;
    DecodeScalabilityOption *decodeScalabilityOption = dynamic_cast<DecodeScalabilityOption *>(m_scalabilityOption);
    SCALABILITY_CHK_NULL_RETURN(decodeScalabilityOption);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    MOS_VIRTUALENGINE_SET_PARAMS veParams;
    MOS_ZeroMemory(&veParams, sizeof(veParams));

    veParams.ucScalablePipeNum = m_pipeNum;
    veParams.bHaveFrontEndCmds = (decodeScalabilityOption->GetMode() == scalabilityVirtualTileMode) && 
                                 (!decodeScalabilityOption->IsFESeparateSubmission());
    veParams.bScalableMode     = true;

    m_osInterface->pVEInterf = m_veInterface;
    eStatus = m_osInterface->pfnSetHintParams(m_osInterface, &veParams);

    return eStatus;
}

MOS_STATUS DecodeScalabilityMultiPipeNext::PopulateHintParams(PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);
    SCALABILITY_CHK_NULL_RETURN(m_veHitParams);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    PMOS_CMD_BUF_ATTRI_VE attriVe  = m_osInterface->pfnGetAttributeVeBuffer(cmdBuffer);
    if (attriVe)
    {
        attriVe->VEngineHintParams     = *(m_veHitParams);
        attriVe->bUseVirtualEngineHint = true;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipeNext::SubmitCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    // Hold the actual command buffer submission till pipes are ready
    if (!IsPipeReadyToSubmit())
    {
        return MOS_STATUS_SUCCESS;
    }

    // Add BB end for every secondary cmd buf when ready for submit
    for (uint32_t secondaryIdx = 0; secondaryIdx < m_pipeNum; secondaryIdx++)
    {
        MOS_COMMAND_BUFFER& scdryCmdBuffer = m_secondaryCmdBuffers[secondaryIdx];
        uint32_t bufIdx = secondaryIdx + DecodePhase::m_secondaryCmdBufIdxBase;
        SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &scdryCmdBuffer, bufIdx));
        SCALABILITY_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(&scdryCmdBuffer, nullptr));
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &scdryCmdBuffer, bufIdx);
    }

    m_attrReady = false;

    SCALABILITY_CHK_STATUS_RETURN(SetHintParams());
    SCALABILITY_CHK_STATUS_RETURN(PopulateHintParams(&m_primaryCmdBuffer));

    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &m_primaryCmdBuffer, false));

    m_semaphoreIndex++;
    if (m_semaphoreIndex >= m_maxCmdBufferSetsNum)
    {
        m_semaphoreIndex = 0;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipeNext::SyncAllPipes(PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);

    SCALABILITY_ASSERT(m_semaphoreIndex < m_resSemaphoreAllPipes.size());
    auto &semaphoreBufs = m_resSemaphoreAllPipes[m_semaphoreIndex];
    SCALABILITY_ASSERT(semaphoreBufs.size() >= m_scalabilityOption->GetNumPipe());

    //Not stop watch dog here, expect to stop it in the packet when needed.
    //HW Semaphore cmd to make sure all pipes start encode at the same time

    // Increment all pipe flags
    for (uint32_t i = 0; i < m_pipeNum; i++)
    {
        if (!Mos_ResourceIsNull(&semaphoreBufs[i]))
        {
            SCALABILITY_CHK_STATUS_RETURN(m_hwInterface->SendMiAtomicDwordCmd(
                &semaphoreBufs[i], 1, MHW_MI_ATOMIC_INC, cmdBuffer));
        }
    }

    if (!Mos_ResourceIsNull(&semaphoreBufs[m_currentPipe]))
    {
        // Waiting current pipe flag euqal to pipe number which means other pipes are executing
        SCALABILITY_CHK_STATUS_RETURN(m_hwInterface->SendHwSemaphoreWaitCmd(
            &semaphoreBufs[m_currentPipe], m_pipeNum, MHW_MI_SAD_EQUAL_SDD, cmdBuffer));

        // Reset current pipe flag for next frame
        auto &params            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        params                  = {};
        params.pOsResource      = &semaphoreBufs[m_currentPipe];
        params.dwResourceOffset = 0;
        params.dwValue          = 0;
        SCALABILITY_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipeNext::SyncOnePipeWaitOthers(PMOS_COMMAND_BUFFER cmdBuffer, uint32_t pipeIdx)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);

    SCALABILITY_ASSERT(m_semaphoreIndex < m_resSemaphoreOnePipeWait.size());
    auto &semaphoreBufs = m_resSemaphoreOnePipeWait[m_semaphoreIndex];
    SCALABILITY_ASSERT(semaphoreBufs.size() >= m_scalabilityOption->GetNumPipe());

    // Send MI_FLUSH command
    auto &parFlush                         = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    parFlush                               = {};
    parFlush.bVideoPipelineCacheInvalidate = true;
    if (!Mos_ResourceIsNull(&semaphoreBufs[m_currentPipe]))
    {
        parFlush.pOsResource = &semaphoreBufs[m_currentPipe];
        parFlush.dwDataDW1   = m_currentPass + 1;
    }
    SCALABILITY_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

    if (m_currentPipe == pipeIdx)
    {
        // this pipe needs to ensure all other pipes are ready
        for (uint32_t i = 0; i < m_pipeNum; i++)
        {
            if (!Mos_ResourceIsNull(&semaphoreBufs[i]))
            {
                SCALABILITY_CHK_STATUS_RETURN(m_hwInterface->SendHwSemaphoreWaitCmd(
                        &semaphoreBufs[i], m_currentPass + 1, MHW_MI_SAD_EQUAL_SDD, cmdBuffer));
            }
        }

        // Reset all pipe flags for next frame
        for (uint32_t i = 0; i < m_pipeNum; i++)
        {
            if (!Mos_ResourceIsNull(&semaphoreBufs[i]))
            {
                auto &params            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
                params                  = {};
                params.pOsResource      = &semaphoreBufs[i];
                params.dwResourceOffset = 0;
                params.dwValue          = 0;
                SCALABILITY_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipeNext::SyncPipe(uint32_t syncType, uint32_t semaphoreId, PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    if (syncType == syncAllPipes)
    {
        return SyncAllPipes(cmdBuffer);
    }
    else if (syncType == syncOnePipeWaitOthers)
    {
        return SyncOnePipeWaitOthers(cmdBuffer, semaphoreId);
    }
    else
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
}

MOS_STATUS DecodeScalabilityMultiPipeNext::ResetSemaphore(uint32_t syncType, uint32_t semaphoreId, PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    // Don't need to reset semaphore
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipeNext::UpdateState(void *statePars)
{
    SCALABILITY_FUNCTION_ENTER;

    StateParams *decodeStatePars = (StateParams *)statePars;
    if (decodeStatePars->currentPipe >= m_pipeNum)
    {
        SCALABILITY_ASSERTMESSAGE("UpdateState failed with invalid parameter: currentPipe %d!",
                                  decodeStatePars->currentPipe);
        return MOS_STATUS_INVALID_PARAMETER;
    }
    m_currentPipe              = decodeStatePars->currentPipe;
    m_currentPass              = decodeStatePars->currentPass;
    m_pipeIndexForSubmit       = decodeStatePars->pipeIndexForSubmit;
    m_singleTaskPhaseSupported = decodeStatePars->singleTaskPhaseSupported;
    m_statusReport             = decodeStatePars->statusReport;
    m_currentRow               = decodeStatePars->currentRow;
    m_currentSubPass           = decodeStatePars->currentSubPass;
    m_componentState           = decodeStatePars->componentState;

    m_phase                    = static_cast<DecodePhase *>(m_componentState);
    SCALABILITY_CHK_NULL_RETURN(m_phase);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipeNext::SendAttrWithFrameTracking(
    MOS_COMMAND_BUFFER &cmdBuffer,
    bool                frameTrackingRequested)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    SCALABILITY_FUNCTION_ENTER;

    bool renderEngineUsed = m_mediaContext->IsRenderEngineUsed();

    // initialize command buffer attributes
    cmdBuffer.Attributes.bTurboMode               = m_hwInterface->m_turboMode;
    cmdBuffer.Attributes.bMediaPreemptionEnabled  = renderEngineUsed ? m_hwInterface->GetRenderInterfaceNext()->IsPreemptionEnabled() : 0;

    if (frameTrackingRequested && m_frameTrackingEnabled)
    {
        PMOS_RESOURCE resource = nullptr;
        uint32_t      offset   = 0;
        SCALABILITY_CHK_STATUS_RETURN(m_statusReport->GetAddress(decode::statusReportGlobalCount, resource, offset));

        cmdBuffer.Attributes.bEnableMediaFrameTracking    = true;
        cmdBuffer.Attributes.resMediaFrameTrackingSurface = resource;
        cmdBuffer.Attributes.dwMediaFrameTrackingTag      = m_statusReport->GetSubmittedCount() + 1;
        // Set media frame tracking address offset(the offset from the decode status buffer page)
        cmdBuffer.Attributes.dwMediaFrameTrackingAddrOffset = offset;
    }

    return eStatus;
}

MOS_STATUS DecodeScalabilityMultiPipeNext::CreateDecodeMultiPipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(hwInterface);
    SCALABILITY_CHK_NULL_RETURN(mediaContext);

    ((CodechalHwInterfaceNext *)hwInterface)->m_multiPipeScalability = MOS_New(DecodeScalabilityMultiPipeNext, hwInterface, mediaContext, scalabilityDecoder);
    SCALABILITY_CHK_NULL_RETURN(((CodechalHwInterfaceNext *)hwInterface)->m_multiPipeScalability);
    return eStatus;
}

}
