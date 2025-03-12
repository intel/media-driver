/*
* Copyright (c) 2019-2021, Intel Corporation
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
//! \file     decode_scalability_multipipe.cpp
//! \brief    Defines the common interface for decode scalability multipipe mode.
//!

#include "decode_scalability_multipipe.h"

#include "media_context.h"
#include "media_status_report.h"
#include "mhw_utilities.h"
#include "decode_status_report_defs.h"
#include "codechal_hw.h"

namespace decode
{
DecodeScalabilityMultiPipe::DecodeScalabilityMultiPipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType)
    : DecodeScalabilityMultiPipeNext(mediaContext, mediaContext, componentType)
{
    m_hwInterface   = (CodechalHwInterface *)(((CodechalHwInterfaceNext *)hwInterface)->legacyHwInterface);
    m_componentType = componentType;
    m_secondaryCmdBuffers.clear();
    m_resSemaphoreAllPipes.clear();
    m_resSemaphoreOnePipeWait.clear();
}

DecodeScalabilityMultiPipe::~DecodeScalabilityMultiPipe()
{
}

MOS_STATUS DecodeScalabilityMultiPipe::AllocateSemaphore()
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

MOS_STATUS DecodeScalabilityMultiPipe::Initialize(const MediaScalabilityOption &option)
{
    SCALABILITY_FUNCTION_ENTER;

    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);
    m_osInterface = m_hwInterface->GetOsInterface();
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    m_miItf = m_hwInterface->GetMiInterfaceNext();

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

    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnVirtualEngineInit(m_osInterface, &m_veHitParams, veInitParms));
    if (m_osInterface->apoMosEnabled)
    {
        SCALABILITY_CHK_NULL_RETURN(m_osInterface->osStreamState);
        m_veState = m_osInterface->osStreamState->virtualEngineInterface;
        SCALABILITY_CHK_NULL_RETURN(m_veState);
        SCALABILITY_CHK_NULL_RETURN(m_veHitParams);
    }
    else
    {
        m_veInterface = m_osInterface->pVEInterf;
        SCALABILITY_CHK_NULL_RETURN(m_veInterface);
        if (m_veInterface->pfnVEGetHintParams != nullptr)
        {
            SCALABILITY_CHK_NULL_RETURN(m_veHitParams);
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
        if (m_osInterface->apoMosEnabled)
        {
            for (uint32_t i = 0; i < m_osInterface->pfnGetVeEngineCount(m_osInterface->osStreamState); i++)
            {
                gpuCtxCreateOption->EngineInstance[i] =
                    m_osInterface->pfnGetEngineLogicIdByIdx(m_osInterface->osStreamState, i);
            }
        }
        else
        {
            for (uint32_t i = 0; i < m_veInterface->ucEngineCount; i++)
            {
                gpuCtxCreateOption->EngineInstance[i] = m_veInterface->EngineLogicId[i];
            }
        }
    }
#endif
    m_gpuCtxCreateOption = (PMOS_GPUCTX_CREATOPTIONS)(gpuCtxCreateOption);

    //Allocate and init for semaphores
    SCALABILITY_CHK_STATUS_RETURN(AllocateSemaphore());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipe::GetGpuCtxCreationOption(MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption)
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

MOS_STATUS DecodeScalabilityMultiPipe::Destroy()
{
    SCALABILITY_FUNCTION_ENTER;

    SCALABILITY_CHK_STATUS_RETURN(MediaScalability::Destroy());

    if (!m_osInterface->apoMosEnabled)
    {
        if (m_veInterface)
        {
            if (m_veInterface->pfnVEDestroy)
            {
                m_veInterface->pfnVEDestroy(m_veInterface);
            }
            MOS_FreeMemAndSetNull(m_veInterface);
        }
    }
    if (m_gpuCtxCreateOption)
    {
        MOS_Delete(m_gpuCtxCreateOption);
    }
    if (m_scalabilityOption)
    {
        MOS_Delete(m_scalabilityOption);
    }

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

MOS_STATUS DecodeScalabilityMultiPipe::ResizeCommandBufferAndPatchList(
    uint32_t                    requestedCommandBufferSize,
    uint32_t                    requestedPatchListSize)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);

    return m_hwInterface->ResizeCommandBufferAndPatchList(requestedCommandBufferSize, requestedPatchListSize);
}

MOS_STATUS DecodeScalabilityMultiPipe::VerifySpaceAvailable(uint32_t requestedSize, uint32_t requestedPatchListSize, bool &singleTaskPhaseSupportedInPak)
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

MOS_STATUS DecodeScalabilityMultiPipe::VerifyCmdBuffer(uint32_t requestedSize, uint32_t requestedPatchListSize, bool &singleTaskPhaseSupportedInPak)
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

MOS_STATUS DecodeScalabilityMultiPipe::GetCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer, bool frameTrackingRequested)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);
    SCALABILITY_CHK_NULL_RETURN(m_phase);

    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &m_primaryCmdBuffer, 0));

    uint32_t bufIdx = m_phase->GetCmdBufIndex();
    SCALABILITY_COND_CHECK(bufIdx < DecodePhase::m_secondaryCmdBufIdxBase, " bufIdx(%d) is less than m_secondaryCmdBufIdxBase(%d), invalid !", bufIdx, DecodePhase::m_secondaryCmdBufIdxBase);

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
        SCALABILITY_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiNoop(&m_primaryCmdBuffer, nullptr));
        m_attrReady = true;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipe::ReturnCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
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

MOS_STATUS DecodeScalabilityMultiPipe::SetHintParams()
{
    SCALABILITY_FUNCTION_ENTER;

    SCALABILITY_CHK_NULL_RETURN(m_osInterface);
    if (m_osInterface->apoMosEnabled)
    {
        SCALABILITY_CHK_NULL_RETURN(m_osInterface->osStreamState);
    }
    else
    {
        SCALABILITY_CHK_NULL_RETURN(m_veInterface);
    }

    DecodeScalabilityOption *decodeScalabilityOption = dynamic_cast<DecodeScalabilityOption *>(m_scalabilityOption);
    SCALABILITY_CHK_NULL_RETURN(decodeScalabilityOption);

    MOS_VIRTUALENGINE_SET_PARAMS veParams;
    MOS_ZeroMemory(&veParams, sizeof(veParams));

    veParams.ucScalablePipeNum = m_pipeNum;
    veParams.bHaveFrontEndCmds = (decodeScalabilityOption->GetMode() == scalabilityVirtualTileMode) && 
                                 (!decodeScalabilityOption->IsFESeparateSubmission());
    veParams.bScalableMode     = true;

    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnSetHintParams(m_osInterface, &veParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipe::PopulateHintParams(PMOS_COMMAND_BUFFER cmdBuffer)
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

MOS_STATUS DecodeScalabilityMultiPipe::SubmitCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
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
        SCALABILITY_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(&scdryCmdBuffer, nullptr));
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &scdryCmdBuffer, bufIdx);
    }

    m_attrReady = false;

    if (m_osInterface->apoMosEnabled || (m_veInterface && m_veInterface->pfnVESetHintParams != nullptr))
    {
        SCALABILITY_CHK_STATUS_RETURN(SetHintParams());
        SCALABILITY_CHK_STATUS_RETURN(PopulateHintParams(&m_primaryCmdBuffer));
    }

    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &m_primaryCmdBuffer, false));

    m_semaphoreIndex++;
    if (m_semaphoreIndex >= m_maxCmdBufferSetsNum)
    {
        m_semaphoreIndex = 0;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipe::SyncAllPipes(PMOS_COMMAND_BUFFER cmdBuffer)
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
        MHW_MI_STORE_DATA_PARAMS    dataParams;
        dataParams.pOsResource      = &semaphoreBufs[m_currentPipe];
        dataParams.dwResourceOffset = 0;
        dataParams.dwValue          = 0;
        SCALABILITY_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiStoreDataImmCmd(
            cmdBuffer, &dataParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipe::SyncOnePipeWaitOthers(PMOS_COMMAND_BUFFER cmdBuffer, uint32_t pipeIdx)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);

    MhwMiInterface *miInterface = m_hwInterface->GetMiInterface();
    SCALABILITY_CHK_NULL_RETURN(miInterface);

    SCALABILITY_ASSERT(m_semaphoreIndex < m_resSemaphoreOnePipeWait.size());
    auto &semaphoreBufs = m_resSemaphoreOnePipeWait[m_semaphoreIndex];
    SCALABILITY_ASSERT(semaphoreBufs.size() >= m_scalabilityOption->GetNumPipe());

    // Send MI_FLUSH command
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    if (!Mos_ResourceIsNull(&semaphoreBufs[m_currentPipe]))
    {
        flushDwParams.pOsResource = &semaphoreBufs[m_currentPipe];
        flushDwParams.dwDataDW1   = m_currentPass + 1;
    }
    SCALABILITY_CHK_STATUS_RETURN(miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

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
                MHW_MI_STORE_DATA_PARAMS    dataParams;
                dataParams.pOsResource      = &semaphoreBufs[i];
                dataParams.dwResourceOffset = 0;
                dataParams.dwValue          = 0;
                SCALABILITY_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiStoreDataImmCmd(
                    cmdBuffer, &dataParams));
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipe::SyncPipe(uint32_t syncType, uint32_t semaphoreId, PMOS_COMMAND_BUFFER cmdBuffer)
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

MOS_STATUS DecodeScalabilityMultiPipe::ResetSemaphore(uint32_t syncType, uint32_t semaphoreId, PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    // Don't need to reset semaphore
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeScalabilityMultiPipe::UpdateState(void *statePars)
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

MOS_STATUS DecodeScalabilityMultiPipe::SendAttrWithFrameTracking(
    MOS_COMMAND_BUFFER &cmdBuffer,
    bool                frameTrackingRequested)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    SCALABILITY_FUNCTION_ENTER;

    bool renderEngineUsed = m_mediaContext->IsRenderEngineUsed();

    // initialize command buffer attributes
    cmdBuffer.Attributes.bTurboMode               = m_hwInterface->m_turboMode;
    cmdBuffer.Attributes.bMediaPreemptionEnabled  = renderEngineUsed ? m_hwInterface->GetRenderInterface()->IsPreemptionEnabled() : 0;

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

MOS_STATUS DecodeScalabilityMultiPipe::CreateDecodeMultiPipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(hwInterface);
    SCALABILITY_CHK_NULL_RETURN(mediaContext);

    ((CodechalHwInterfaceNext *)hwInterface)->m_multiPipeScalability = MOS_New(DecodeScalabilityMultiPipe, hwInterface, mediaContext, scalabilityDecoder);
    SCALABILITY_CHK_NULL_RETURN(((CodechalHwInterfaceNext *)hwInterface)->m_multiPipeScalability);
    return eStatus;
}

}
