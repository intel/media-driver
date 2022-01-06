/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     vp_scalability_multipipe.cpp
//! \brief    Defines the common interface for vp scalability multipipe mode.
//!

#include "vp_scalability_multipipe.h"

#include "media_context.h"
#include "media_status_report.h"
#include "mhw_utilities.h"

namespace vp
{
VpScalabilityMultiPipe::VpScalabilityMultiPipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType)
    : MediaScalabilityMultiPipe(mediaContext)
{
    m_hwInterface   = (PVP_MHWINTERFACE)hwInterface;
    m_componentType = componentType;
}

VpScalabilityMultiPipe::~VpScalabilityMultiPipe()
{

}

MOS_STATUS VpScalabilityMultiPipe::AllocateSemaphore()
{
    VP_FUNC_CALL();

    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    m_secondaryCmdBuffers.resize(m_initSecondaryCmdBufNum);
    m_secondaryCmdBuffersReturned.resize(m_initSecondaryCmdBufNum);

    for (uint32_t idx = 0; idx < m_initSecondaryCmdBufNum; idx++)
    {
        m_secondaryCmdBuffersReturned[idx] = false;
    }

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

MOS_STATUS VpScalabilityMultiPipe::Initialize(const MediaScalabilityOption &option)
{
    VP_FUNC_CALL();

    SCALABILITY_FUNCTION_ENTER;

    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);
    m_osInterface = m_hwInterface->m_osInterface;
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);
    m_miInterface = m_hwInterface->m_mhwMiInterface;
    SCALABILITY_CHK_NULL_RETURN(m_miInterface);
    m_miItf = std::static_pointer_cast<mhw::mi::Itf>(m_miInterface->GetNewMiInterface());

    VpScalabilityOption *vpScalabilityOption = MOS_New(VpScalabilityOption, (const VpScalabilityOption &)option);
    SCALABILITY_CHK_NULL_RETURN(vpScalabilityOption);
    m_scalabilityOption = vpScalabilityOption;

    m_frameTrackingEnabled = m_osInterface->bEnableKmdMediaFrameTracking ? true : false;

    //virtual engine init with scalability
    MOS_VIRTUALENGINE_INIT_PARAMS veInitParms;
    MOS_ZeroMemory(&veInitParms, sizeof(veInitParms));
    veInitParms.bScalabilitySupported          = true;
    veInitParms.ucNumOfSdryCmdBufSets          = m_maxCmdBufferSetsNum;
    veInitParms.ucMaxNumPipesInUse             = vpScalabilityOption->GetMaxMultiPipeNum();
    veInitParms.ucMaxNumOfSdryCmdBufInOneFrame = veInitParms.ucMaxNumPipesInUse;

    if (m_osInterface->apoMosEnabled)
    {
        SCALABILITY_CHK_NULL_RETURN(m_osInterface->osStreamState);
        m_osInterface->osStreamState->component = COMPONENT_VPCommon;

        SCALABILITY_CHK_STATUS_RETURN(MosInterface::CreateVirtualEngineState(
            m_osInterface->osStreamState, &veInitParms, m_veState));
        SCALABILITY_CHK_NULL_RETURN(m_veState);
        
        SCALABILITY_CHK_STATUS_RETURN(MosInterface::GetVeHintParams(m_osInterface->osStreamState, true, &m_veHitParams));
        SCALABILITY_CHK_NULL_RETURN(m_veHitParams);
    }
    else
    {
        SCALABILITY_CHK_STATUS_RETURN(Mos_VirtualEngineInterface_Initialize(m_osInterface, &veInitParms));
        m_veInterface = m_osInterface->pVEInterf;
        SCALABILITY_CHK_NULL_RETURN(m_veInterface);

        if (m_veInterface->pfnVEGetHintParams != nullptr)
        {
            SCALABILITY_CHK_STATUS_RETURN(m_veInterface->pfnVEGetHintParams(m_veInterface, true, &m_veHitParams));
            SCALABILITY_CHK_NULL_RETURN(m_veHitParams);
        }
    }

    m_pipeNum = m_scalabilityOption->GetNumPipe();
    m_pipeIndexForSubmit = m_pipeNum;

    PMOS_GPUCTX_CREATOPTIONS_ENHANCED gpuCtxCreateOption = MOS_New(MOS_GPUCTX_CREATOPTIONS_ENHANCED);
    SCALABILITY_CHK_NULL_RETURN(gpuCtxCreateOption);
    gpuCtxCreateOption->LRCACount = vpScalabilityOption->GetLRCACount();
    gpuCtxCreateOption->UsingSFC  = vpScalabilityOption->IsUsingSFC();

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_osInterface->bEnableDbgOvrdInVE)
    {
        if (m_osInterface->apoMosEnabled)
        {
            for (uint32_t i = 0; i < MosInterface::GetVeEngineCount(m_osInterface->osStreamState); i++)
            {
                gpuCtxCreateOption->EngineInstance[i] =
                    MosInterface::GetEngineLogicId(m_osInterface->osStreamState, i);
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

MOS_STATUS VpScalabilityMultiPipe::GetGpuCtxCreationOption(MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption)
{
    VP_FUNC_CALL();

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

MOS_STATUS VpScalabilityMultiPipe::Destroy()
{
    VP_FUNC_CALL();

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

MOS_STATUS VpScalabilityMultiPipe::ResizeCommandBufferAndPatchList(
    uint32_t requestedCommandBufferSize,
    uint32_t requestedPatchListSize)
{
    VP_FUNC_CALL();

    MOS_UNUSED(requestedCommandBufferSize);
    MOS_UNUSED(requestedPatchListSize);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalabilityMultiPipe::VerifySpaceAvailable(uint32_t requestedSize, uint32_t requestedPatchListSize, bool &singleTaskPhaseSupportedInPak)
{
    VP_FUNC_CALL();

    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    uint8_t looptimes = 3;
    for (auto i = 0; i < looptimes; i++)
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

MOS_STATUS VpScalabilityMultiPipe::VerifyCmdBuffer(uint32_t requestedSize, uint32_t requestedPatchListSize, bool &singleTaskPhaseSupportedInPak)
{
    VP_FUNC_CALL();

    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    requestedSize = MOS_MAX(requestedSize, m_CmdBufferSize);

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

MOS_STATUS VpScalabilityMultiPipe::GetCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer, bool frameTrackingRequested)
{
    VP_FUNC_CALL();

    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;
    PMOS_COMMAND_BUFFER scdryCmdBuffer;

    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    if (m_currentPipe >= m_pipeNum)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        SCALABILITY_ASSERTMESSAGE("Verify Command buffer failed with invalid parameter:currentPipe!");
        return eStatus;
    }

    // Get primary cmd buffer
    if (Mos_ResourceIsNull(&m_primaryCmdBuffer.OsResource))
    {
        SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &m_primaryCmdBuffer, 0));
    }

    // Get Secondary cmd buffer
    uint32_t bufIdx = m_currentPipe;  //Make CMD buffer one next to one.
    uint32_t bufIdxPlus1 = bufIdx + 1;

    if (Mos_ResourceIsNull(&m_secondaryCmdBuffers[bufIdx].OsResource))
    {
        m_osInterface->pfnGetCommandBuffer(m_osInterface, &m_secondaryCmdBuffers[bufIdx], bufIdxPlus1);
    }

    if (m_osInterface->apoMosEnabled)
    {
        int32_t submissionType = IsFirstPipe() ? SUBMISSION_TYPE_MULTI_PIPE_MASTER : SUBMISSION_TYPE_MULTI_PIPE_SLAVE;
        if (IsLastPipe())
        {
            submissionType |= SUBMISSION_TYPE_MULTI_PIPE_FLAGS_LAST_PIPE;
        }
        SCALABILITY_CHK_NULL_RETURN(m_osInterface->osStreamState);
        SCALABILITY_CHK_STATUS_RETURN(MosInterface::SetVeSubmissionType(
            m_osInterface->osStreamState, &(m_secondaryCmdBuffers[bufIdx]), submissionType));
    }
    else
    {
        m_secondaryCmdBuffers[bufIdx].iSubmissionType =
            IsFirstPipe() ? SUBMISSION_TYPE_MULTI_PIPE_MASTER : SUBMISSION_TYPE_MULTI_PIPE_SLAVE;

        if (IsLastPipe())
        {
            m_secondaryCmdBuffers[bufIdx].iSubmissionType |= SUBMISSION_TYPE_MULTI_PIPE_FLAGS_LAST_PIPE;
        }
    }

    *cmdBuffer = m_secondaryCmdBuffers[bufIdx];
    m_secondaryCmdBuffersReturned[bufIdx] = false;

    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);

    if (!m_attrReady)
    {
        SCALABILITY_CHK_STATUS_RETURN(SendAttrWithFrameTracking(m_primaryCmdBuffer, frameTrackingRequested));
        m_attrReady = true;
    }

    return eStatus;
}

MOS_STATUS VpScalabilityMultiPipe::ReturnCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    if (m_currentPipe >= m_pipeNum)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        SCALABILITY_ASSERTMESSAGE("Verify Command buffer failed with invalid parameter:currentPipe!");
        return eStatus;
    }

    uint32_t bufIdx = m_currentPipe;  //Make CMD buffer one next to one.
    uint32_t bufIdxPlus1 = bufIdx + 1;
    if (m_secondaryCmdBuffersReturned[bufIdx] == false)
    {
        m_secondaryCmdBuffers[bufIdx] = *cmdBuffer;  //Need to record the iOffset, ptr and other data of CMD buffer, it's not maintain in the mos.
        m_secondaryCmdBuffersReturned[bufIdx] = true;
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &m_secondaryCmdBuffers[bufIdx], bufIdxPlus1);
    }
    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &m_primaryCmdBuffer, 0);

    return eStatus;
}

MOS_STATUS VpScalabilityMultiPipe::SetHintParams()
{
    VP_FUNC_CALL();

    SCALABILITY_FUNCTION_ENTER;

    if (m_osInterface->apoMosEnabled)
    {
        SCALABILITY_CHK_NULL_RETURN(m_osInterface->osStreamState);
    }
    else
    {
        SCALABILITY_CHK_NULL_RETURN(m_veInterface);
    }

    VpScalabilityOption *vpScalabilityOption = dynamic_cast<VpScalabilityOption *>(m_scalabilityOption);
    SCALABILITY_CHK_NULL_RETURN(vpScalabilityOption);

    MOS_VIRTUALENGINE_SET_PARAMS veParams;
    MOS_ZeroMemory(&veParams, sizeof(veParams));

    veParams.ucScalablePipeNum = m_pipeNum;
    veParams.bScalableMode     = true;

    if (m_osInterface->apoMosEnabled)
    {
        SCALABILITY_CHK_STATUS_RETURN(MosInterface::SetVeHintParams(m_osInterface->osStreamState, &veParams));
    }
    else
    {
        SCALABILITY_CHK_STATUS_RETURN(m_veInterface->pfnVESetHintParams(m_veInterface, &veParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalabilityMultiPipe::PopulateHintParams(PMOS_COMMAND_BUFFER cmdBuffer)
{
    VP_FUNC_CALL();

    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);
    SCALABILITY_CHK_NULL_RETURN(m_veHitParams);

    PMOS_CMD_BUF_ATTRI_VE attriVe  = MosInterface::GetAttributeVeBuffer(cmdBuffer);
    if (attriVe)
    {
        attriVe->VEngineHintParams     = *(m_veHitParams);
        attriVe->bUseVirtualEngineHint = true;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalabilityMultiPipe::SubmitCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    VP_FUNC_CALL();

    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    m_attrReady = false;

    if (m_osInterface->apoMosEnabled || (m_veInterface && m_veInterface->pfnVESetHintParams != nullptr))
    {
        SCALABILITY_CHK_STATUS_RETURN(SetHintParams());
        SCALABILITY_CHK_STATUS_RETURN(PopulateHintParams(&m_primaryCmdBuffer));
    }

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    if (m_osInterface->bDumpCommandBuffer)
    {
        for (uint32_t i = 0; i < m_pipeNum; i++)
        {
            SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnDumpCommandBuffer(m_osInterface, &m_secondaryCmdBuffers[i]));
        }
    }
#endif  // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &m_primaryCmdBuffer, false));

    MOS_ZeroMemory(&m_primaryCmdBuffer.OsResource, sizeof(m_primaryCmdBuffer.OsResource));
    for (uint32_t i = 0; i < m_pipeNum; i++)
    {
        MOS_ZeroMemory(&m_secondaryCmdBuffers[i].OsResource, sizeof(m_secondaryCmdBuffers[i].OsResource));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalabilityMultiPipe::SyncAllPipes(PMOS_COMMAND_BUFFER cmdBuffer)
{
    VP_FUNC_CALL();

    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface->m_mhwMiInterface);

    SCALABILITY_ASSERT(m_semaphoreIndex < m_resSemaphoreAllPipes.size());
    auto &semaphoreBufs = m_resSemaphoreAllPipes[m_semaphoreIndex];
    SCALABILITY_ASSERT(semaphoreBufs.size() >= m_scalabilityOption->GetNumPipe());

    // Increment all pipe flags
    for (uint32_t i = 0; i < m_pipeNum; i++)
    {
        if (!Mos_ResourceIsNull(&semaphoreBufs[i]))
        {
            SCALABILITY_CHK_STATUS_RETURN(SendMiAtomicDwordCmd(
                &semaphoreBufs[i], 1, MHW_MI_ATOMIC_INC, cmdBuffer));
        }
    }

    if (!Mos_ResourceIsNull(&semaphoreBufs[m_currentPipe]))
    {
        // Waiting current pipe flag euqal to pipe number which means other pipes are executing
        SCALABILITY_CHK_STATUS_RETURN(SendHwSemaphoreWaitCmd(
            &semaphoreBufs[m_currentPipe], m_pipeNum, MHW_MI_SAD_EQUAL_SDD, cmdBuffer));

        PMHW_MI_INTERFACE pMhwMiInterface = m_hwInterface->m_mhwMiInterface;

        MHW_MI_STORE_DATA_PARAMS dataParams = {};
        dataParams.pOsResource      = &semaphoreBufs[m_currentPipe];
        dataParams.dwResourceOffset = 0;
        dataParams.dwValue          = 0;

        // Reset current pipe semaphore
        SCALABILITY_CHK_STATUS_RETURN(pMhwMiInterface->AddMiStoreDataImmCmd(
            cmdBuffer, &dataParams));
    }

    m_semaphoreIndex += m_initSecondaryCmdBufNum;
    if (m_semaphoreIndex >= m_maxCmdBufferSetsNum)
    {
        m_semaphoreIndex = 0;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalabilityMultiPipe::SyncOnePipeWaitOthers(PMOS_COMMAND_BUFFER cmdBuffer, uint32_t pipeIdx)
{
    VP_FUNC_CALL();

    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);

    MhwMiInterface *miInterface = m_hwInterface->m_mhwMiInterface;
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
                SCALABILITY_CHK_STATUS_RETURN(SendHwSemaphoreWaitCmd(
                        &semaphoreBufs[i], m_currentPass + 1, MHW_MI_SAD_EQUAL_SDD, cmdBuffer));
            }
        }

        // Reset all pipe flags for next frame
        for (uint32_t i = 0; i < m_pipeNum; i++)
        {
            if (!Mos_ResourceIsNull(&semaphoreBufs[i]))
            {
                SCALABILITY_CHK_STATUS_RETURN(SendMiAtomicDwordCmd(
                    &semaphoreBufs[i], m_currentPass + 1, MHW_MI_ATOMIC_DEC, cmdBuffer));

            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalabilityMultiPipe::SyncPipe(uint32_t syncType, uint32_t semaphoreId, PMOS_COMMAND_BUFFER cmdBuffer)
{
    VP_FUNC_CALL();

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

MOS_STATUS VpScalabilityMultiPipe::ResetSemaphore(uint32_t syncType, uint32_t semaphoreId, PMOS_COMMAND_BUFFER cmdBuffer)
{
    VP_FUNC_CALL();

    SCALABILITY_FUNCTION_ENTER;
    // Don't need to reset semaphore
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalabilityMultiPipe::UpdateState(void *statePars)
{
    VP_FUNC_CALL();

    SCALABILITY_FUNCTION_ENTER;

    StateParams *vpStatePars = (StateParams *)statePars;
    if (vpStatePars->currentPipe < 0 || vpStatePars->currentPipe >= m_pipeNum)
    {
        SCALABILITY_ASSERTMESSAGE("UpdateState failed with invalid parameter: currentPipe %d!",
            vpStatePars->currentPipe);
        return MOS_STATUS_INVALID_PARAMETER;
    }
    m_currentPipe              = vpStatePars->currentPipe;
    m_currentPass              = vpStatePars->currentPass;
    m_singleTaskPhaseSupported = vpStatePars->singleTaskPhaseSupported;
    m_statusReport             = vpStatePars->statusReport;
    m_currentRow               = vpStatePars->currentRow;
    m_currentSubPass           = vpStatePars->currentSubPass;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalabilityMultiPipe::SendAttrWithFrameTracking(
    MOS_COMMAND_BUFFER &cmdBuffer,
    bool                frameTrackingRequested)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    SCALABILITY_FUNCTION_ENTER;

    bool renderEngineUsed = m_mediaContext->IsRenderEngineUsed();

    // Initialize command buffer attributes
    if (frameTrackingRequested && m_frameTrackingEnabled)
    {
        PMOS_RESOURCE gpuStatusBuffer = nullptr;

        SCALABILITY_CHK_NULL_RETURN(m_osInterface);
        SCALABILITY_CHK_NULL_RETURN(m_osInterface->pfnGetGpuStatusBufferResource);
        SCALABILITY_CHK_NULL_RETURN(m_osInterface->pfnRegisterResource);
        SCALABILITY_CHK_NULL_RETURN(m_osInterface->pfnGetGpuStatusTag);
        SCALABILITY_CHK_NULL_RETURN(m_osInterface->pfnGetGpuStatusTagOffset);

        // Get GPU Status buffer
        SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnGetGpuStatusBufferResource(m_osInterface, gpuStatusBuffer));
        SCALABILITY_CHK_NULL_RETURN(gpuStatusBuffer);
        // Register the buffer
        SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnRegisterResource(m_osInterface, gpuStatusBuffer, true, true));

        cmdBuffer.Attributes.bEnableMediaFrameTracking      = true;
        cmdBuffer.Attributes.resMediaFrameTrackingSurface   = gpuStatusBuffer;
        cmdBuffer.Attributes.dwMediaFrameTrackingTag        = m_osInterface->pfnGetGpuStatusTag(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        cmdBuffer.Attributes.dwMediaFrameTrackingAddrOffset = m_osInterface->pfnGetGpuStatusTagOffset(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);

        // Increment GPU Status Tag
        m_osInterface->pfnIncrementGpuStatusTag(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
    }

    return eStatus;
}

//!
//! \brief    Send hw semphore wait cmd
//! \details  Send hw semphore wait cmd for sync perpose
//!
//! \param    [in] semaMem
//!           Reource of Hw semphore
//! \param    [in] semaData
//!           Data of Hw semphore
//! \param    [in] opCode
//!           Operation code
//! \param    [in,out] cmdBuffer
//!           command buffer
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VpScalabilityMultiPipe::SendHwSemaphoreWaitCmd(
    PMOS_RESOURCE                             semaMem,
    uint32_t                                  semaData,
    MHW_COMMON_MI_SEMAPHORE_COMPARE_OPERATION opCode,
    PMOS_COMMAND_BUFFER                       cmdBuffer)
{
    VP_FUNC_CALL();

    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    PMHW_MI_INTERFACE            pMhwMiInterface;
    MHW_MI_SEMAPHORE_WAIT_PARAMS miSemaphoreWaitParams;

    MHW_CHK_NULL(m_hwInterface);
    MHW_CHK_NULL(m_hwInterface->m_mhwMiInterface);

    pMhwMiInterface = m_hwInterface->m_mhwMiInterface;

    MOS_ZeroMemory((&miSemaphoreWaitParams), sizeof(miSemaphoreWaitParams));
    miSemaphoreWaitParams.presSemaphoreMem = semaMem;
    miSemaphoreWaitParams.bPollingWaitMode = true;
    miSemaphoreWaitParams.dwSemaphoreData  = semaData;
    miSemaphoreWaitParams.CompareOperation = opCode;
    eStatus                                = pMhwMiInterface->AddMiSemaphoreWaitCmd(cmdBuffer, &miSemaphoreWaitParams);

finish:
    return eStatus;
}

//!
//! \brief    Send mi atomic dword cmd
//! \details  Send mi atomic dword cmd for sync perpose
//!
//! \param    [in] resource
//!           Reource used in mi atomic dword cmd
//! \param    [in] immData
//!           Immediate data
//! \param    [in] opCode
//!           Operation code
//! \param    [in,out] cmdBuffer
//!           command buffer
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VpScalabilityMultiPipe::SendMiAtomicDwordCmd(
    PMOS_RESOURCE               resource,
    uint32_t                    immData,
    MHW_COMMON_MI_ATOMIC_OPCODE opCode,
    PMOS_COMMAND_BUFFER         cmdBuffer)
{
    VP_FUNC_CALL();

    MOS_STATUS           eStatus = MOS_STATUS_SUCCESS;
    PMHW_MI_INTERFACE    pMhwMiInterface;
    MHW_MI_ATOMIC_PARAMS atomicParams;

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_mhwMiInterface);

    pMhwMiInterface = m_hwInterface->m_mhwMiInterface;

    MOS_ZeroMemory((&atomicParams), sizeof(atomicParams));
    atomicParams.pOsResource       = resource;
    atomicParams.dwDataSize        = sizeof(uint32_t);
    atomicParams.Operation         = opCode;
    atomicParams.bInlineData       = true;
    atomicParams.dwOperand1Data[0] = immData;
    eStatus                        = pMhwMiInterface->AddMiAtomicCmd(cmdBuffer, &atomicParams);

    return eStatus;
}
}
