/*
* Copyright (c) 2018-2022, Intel Corporation
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
//! \file     encode_scalability_multipipe.cpp
//! \brief    Defines the common interface for encode scalability multipipe mode.
//!

#include "encode_scalability_multipipe.h"

#include "media_context.h"
#include "media_status_report.h"
#include "mhw_utilities.h"
#include "encode_status_report_defs.h"
#include "hal_oca_interface_next.h"
#include "mos_os_virtualengine_next.h"
#include "mos_interface.h"

namespace encode
{
EncodeScalabilityMultiPipe::EncodeScalabilityMultiPipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType) :
    MediaScalabilityMultiPipe(mediaContext)
{
    m_hwInterface   = (CodechalHwInterfaceNext *)hwInterface;
    m_componentType = componentType;
}

EncodeScalabilityMultiPipe::~EncodeScalabilityMultiPipe()
{
}
MOS_STATUS EncodeScalabilityMultiPipe::AllocateSemaphore()
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint32_t *      data = nullptr;
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

    for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_resSemaphoreAllPipes); i++)
    {
        SCALABILITY_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                  m_osInterface,
                                                  &allocParamsForBufferLinear,
                                                  &m_resSemaphoreAllPipes[i]),
            "Cannot create HW semaphore for scalability all pipes sync.");
        data = (uint32_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_resSemaphoreAllPipes[i],
            &lockFlagsWriteOnly);
        SCALABILITY_CHK_NULL_RETURN(data);
        *data = 0;
        SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_resSemaphoreAllPipes[i]));
    }

    allocParamsForBufferLinear.pBufName = "Sync One Pipe Wait SemaphoreMemory";

    for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_resSemaphoreOnePipeWait); i++)
    {
        SCALABILITY_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                  m_osInterface,
                                                  &allocParamsForBufferLinear,
                                                  &m_resSemaphoreOnePipeWait[i]),
            "Cannot create HW semaphore for scalability one pipe sync.");
        data = (uint32_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_resSemaphoreOnePipeWait[i],
            &lockFlagsWriteOnly);
        SCALABILITY_CHK_NULL_RETURN(data);
        *data = 0;
        SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_resSemaphoreOnePipeWait[i]));
    }

    allocParamsForBufferLinear.pBufName = "HW semaphore delay buffer";

    SCALABILITY_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resDelayMinus),
        "Cannot create HW semaphore delay buffer.");
    data = (uint32_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_resDelayMinus,
        &lockFlagsWriteOnly);
    SCALABILITY_CHK_NULL_RETURN(data);
    *data = 0;
    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_resDelayMinus));

    allocParamsForBufferLinear.pBufName = "Sync One Pipe For Another SemaphoreMemory";

    SCALABILITY_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                              m_osInterface,
                                              &allocParamsForBufferLinear,
                                              &m_resSemaphoreOnePipeForAnother),
        "Cannot create HW semaphore for scalability one pipe wait for another.");
    data = (uint32_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_resSemaphoreOnePipeForAnother,
        &lockFlagsWriteOnly);
    SCALABILITY_CHK_NULL_RETURN(data);
    *data = 0;
    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_resSemaphoreOnePipeForAnother));

    allocParamsForBufferLinear.pBufName = "Sync Other Pipes For One SemaphoreMemory";

    SCALABILITY_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                              m_osInterface,
                                              &allocParamsForBufferLinear,
                                              &m_resSemaphoreOtherPipesForOne),
        "Cannot create HW semaphore for scalability other pipes wait for one.");
    data = (uint32_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_resSemaphoreOtherPipesForOne,
        &lockFlagsWriteOnly);
    SCALABILITY_CHK_NULL_RETURN(data);
    *data = 0;
    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_resSemaphoreOtherPipesForOne));

    return eStatus;
}
MOS_STATUS EncodeScalabilityMultiPipe::Initialize(const MediaScalabilityOption &option)
{
    SCALABILITY_FUNCTION_ENTER;

    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);
    m_osInterface = m_hwInterface->GetOsInterface();
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);
    m_miItf = m_hwInterface->GetMiInterfaceNext();
    SCALABILITY_CHK_NULL_RETURN(m_miItf);
    m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
    if (!m_userSettingPtr)
    {
        ENCODE_NORMALMESSAGE("Initialize m_userSettingPtr instance failed!");
    }

    m_scalabilityOption = MOS_New(EncodeScalabilityOption, (const EncodeScalabilityOption &)option);
    SCALABILITY_CHK_NULL_RETURN(m_scalabilityOption);

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MediaUserSetting::Value outValue;
    auto statusKey = ReadUserSetting(
                        m_userSettingPtr,
                        outValue,
                        "Enable Frame Tracking",
                        MediaUserSetting::Group::Sequence);

    if (statusKey == MOS_STATUS_SUCCESS)
    {
        m_frameTrackingEnabled = outValue.Get<bool>();
    }
    else
    {
        m_frameTrackingEnabled = m_osInterface->bEnableKmdMediaFrameTracking ? true : false;
    }

    //virtual engine init with scalability
    MOS_VIRTUALENGINE_INIT_PARAMS veInitParms;
    MOS_ZeroMemory(&veInitParms, sizeof(veInitParms));
    veInitParms.bScalabilitySupported = true;

    // Disabling the Secondary command buffer creation in MOS_VE
    // To be programmed once Encode moves to using secondary command buffers in MOS VE interface
    veInitParms.ucMaxNumPipesInUse             = MOS_MAX_ENGINE_INSTANCE_PER_CLASS;
    veInitParms.ucNumOfSdryCmdBufSets          = 16;
    veInitParms.ucMaxNumOfSdryCmdBufInOneFrame = veInitParms.ucMaxNumPipesInUse * m_maxNumBRCPasses;

    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnVirtualEngineInit(m_osInterface, &m_veHitParams, veInitParms));
    SCALABILITY_CHK_NULL_RETURN(m_osInterface->osStreamState);
    m_veState = m_osInterface->osStreamState->virtualEngineInterface;
    SCALABILITY_CHK_NULL_RETURN(m_veState);
    SCALABILITY_CHK_NULL_RETURN(m_veHitParams);

    m_pipeNum = m_scalabilityOption->GetNumPipe();
    if (m_pipeNum > m_maxPipeNum)
    {
        SCALABILITY_ASSERTMESSAGE("numPipe exceed max supported pipe number!");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    m_pipeIndexForSubmit = m_pipeNum;

    PMOS_GPUCTX_CREATOPTIONS_ENHANCED gpuCtxCreateOption = MOS_New(MOS_GPUCTX_CREATOPTIONS_ENHANCED);
    SCALABILITY_CHK_NULL_RETURN(gpuCtxCreateOption);
    gpuCtxCreateOption->LRCACount = m_scalabilityOption->GetNumPipe();
    gpuCtxCreateOption->UsingSFC  = false;

    EncodeScalabilityOption *scalabilityOption  = 
                    dynamic_cast<EncodeScalabilityOption *>(m_scalabilityOption);
    if (scalabilityOption != nullptr && scalabilityOption->IsVdencEnabled())
    {
        gpuCtxCreateOption->Flags |=  (1 << 2);
    }
#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_osInterface->bEnableDbgOvrdInVE)
    {
        gpuCtxCreateOption->DebugOverride = true;
        for (uint32_t i = 0; i < m_osInterface->pfnGetVeEngineCount(m_osInterface->osStreamState); i++)
        {
            gpuCtxCreateOption->EngineInstance[i] =
                m_osInterface->pfnGetEngineLogicIdByIdx(m_osInterface->osStreamState, i);
        }
    }
#endif
    m_gpuCtxCreateOption = (PMOS_GPUCTX_CREATOPTIONS)(gpuCtxCreateOption);

    //Allocate and init for semaphores
    SCALABILITY_CHK_STATUS_RETURN(AllocateSemaphore());

    //Update encoder scalability status
    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnSetMultiEngineEnabled(m_osInterface, COMPONENT_Encode, true));
    return eStatus;
}

MOS_STATUS EncodeScalabilityMultiPipe::GetGpuCtxCreationOption(MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption)
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

MOS_STATUS EncodeScalabilityMultiPipe::Destroy()
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

    for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_resSemaphoreAllPipes); i++)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_resSemaphoreAllPipes[i]);
    }
    for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_resSemaphoreOnePipeWait); i++)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_resSemaphoreOnePipeWait[i]);
    }

    m_osInterface->pfnFreeResource(m_osInterface, &m_resSemaphoreOnePipeForAnother);

    m_osInterface->pfnFreeResource(m_osInterface, &m_resSemaphoreOtherPipesForOne);

    m_osInterface->pfnFreeResource(m_osInterface, &m_resDelayMinus);

    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnSetMultiEngineEnabled(m_osInterface, COMPONENT_Encode, false));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeScalabilityMultiPipe::ResizeCommandBufferAndPatchList(
    uint32_t                    requestedCommandBufferSize,
    uint32_t                    requestedPatchListSize)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);

    return m_hwInterface->ResizeCommandBufferAndPatchList(requestedCommandBufferSize, requestedPatchListSize);
}

//Move it to MediaScalabiliyt if decode&VP can share it. Curryt only EncodeMultiPipe/SiglePipe can share it.
MOS_STATUS EncodeScalabilityMultiPipe::VerifySpaceAvailable(uint32_t requestedSize, uint32_t requestedPatchListSize, bool &singleTaskPhaseSupportedInPak)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    MOS_STATUS eStatus           = MOS_STATUS_SUCCESS;
    MOS_STATUS statusPatchList   = MOS_STATUS_SUCCESS;
    MOS_STATUS statusCmdBuf      = MOS_STATUS_SUCCESS;

    bool bothPatchListAndCmdBufChkSuccess = false;
    uint8_t looptimes = m_singleTaskPhaseSupported ? 2 : 1;

    for(auto i = 0 ; i < looptimes ; i++)
    {
        SCALABILITY_CHK_STATUS_RETURN(MediaScalability::VerifySpaceAvailable(
            requestedSize, requestedPatchListSize, bothPatchListAndCmdBufChkSuccess));
        if (bothPatchListAndCmdBufChkSuccess == true)
        {
            singleTaskPhaseSupportedInPak = m_singleTaskPhaseSupported;
            return eStatus;
        }
        
        if (requestedPatchListSize)
        {
            statusPatchList = (MOS_STATUS)m_osInterface->pfnVerifyPatchListSize(
                m_osInterface,
                requestedPatchListSize);
        }
        statusCmdBuf = (MOS_STATUS)m_osInterface->pfnVerifyCommandBufferSize(
            m_osInterface,
            requestedSize,
            0);
        if ((statusCmdBuf == MOS_STATUS_SUCCESS) && (statusPatchList == MOS_STATUS_SUCCESS))
        {
            singleTaskPhaseSupportedInPak = m_singleTaskPhaseSupported;
            return eStatus;
        }
    }

    eStatus = MOS_STATUS_NO_SPACE;
    SCALABILITY_ASSERTMESSAGE("Resize Command buffer failed with no space!");
    return eStatus;
}

MOS_STATUS EncodeScalabilityMultiPipe::VerifyCmdBuffer(uint32_t requestedSize, uint32_t requestedPatchListSize, bool &singleTaskPhaseSupportedInPak)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // Verify Primary Cmd buffer.
    SCALABILITY_CHK_STATUS_RETURN(VerifySpaceAvailable(
        requestedSize, requestedPatchListSize, singleTaskPhaseSupportedInPak));

    // Verify all pipes' secondary cmd buffers
    for (uint8_t currentPipeVerify = 0; currentPipeVerify < m_pipeNum; currentPipeVerify++)
    {
        MOS_STATUS statusPatchList = MOS_STATUS_SUCCESS;
        MOS_STATUS statusCmdBuf    = MOS_STATUS_SUCCESS;
        
        
        //Verify 2nd level BB;
        uint32_t bufIdxPlus1 = currentPipeVerify + 1;  //Make CMD buffer one next to one.
        
        eStatus = MOS_STATUS_NO_SPACE;
        for (auto i = 0; (i < 3) && (eStatus != MOS_STATUS_SUCCESS); i++)
        {
            // Verify secondary cmd buffer
            eStatus = (MOS_STATUS)m_osInterface->pfnVerifyCommandBufferSize(
                m_osInterface,
                requestedSize,
                bufIdxPlus1);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnResizeCommandBufferAndPatchList(
                    m_osInterface,
                    requestedSize,
                    0,
                    bufIdxPlus1));
                // Set status to NO_SPACE to enter the commaned buffer size verification on next loop.
                eStatus = MOS_STATUS_NO_SPACE;
                SCALABILITY_ASSERTMESSAGE("Verify Command buffer failed with no space!");
            }
        }
    }
    return eStatus;
}

MOS_STATUS EncodeScalabilityMultiPipe::GetCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer, bool frameTrackingRequested)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;
    PMOS_COMMAND_BUFFER scdryCmdBuffer;

    if (m_currentPipe >= m_pipeNum)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        SCALABILITY_ASSERTMESSAGE("Verify Command buffer failed with invalid parameter:currentPipe!");
        return eStatus;
    }
    if (m_currentPass >= m_maxNumBRCPasses)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        SCALABILITY_ASSERTMESSAGE("Verify Command buffer failed with invalid parameter:currentPass!");
        return eStatus;
    }

    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &m_primaryCmdBuffer, 0));
    uint32_t bufIdxPlus1 = m_currentPipe + 1;  //Make CMD buffer one next to one.
    m_osInterface->pfnGetCommandBuffer(m_osInterface, &m_secondaryCmdBuffer[bufIdxPlus1 - 1], bufIdxPlus1);

    if (m_osInterface->apoMosEnabled)
    {
        int32_t submissionType = IsFirstPipe() ? SUBMISSION_TYPE_MULTI_PIPE_MASTER : SUBMISSION_TYPE_MULTI_PIPE_SLAVE;
        if (IsLastPipe())
        {
            submissionType |= SUBMISSION_TYPE_MULTI_PIPE_FLAGS_LAST_PIPE;
        }
        SCALABILITY_CHK_NULL_RETURN(m_osInterface->osStreamState);
        SCALABILITY_CHK_NULL_RETURN(m_osInterface->osStreamState->virtualEngineInterface);
        SCALABILITY_CHK_STATUS_RETURN(m_osInterface->osStreamState->virtualEngineInterface->SetSubmissionType(&(m_secondaryCmdBuffer[bufIdxPlus1 - 1]), submissionType));
    }
    else
    {
        m_secondaryCmdBuffer[bufIdxPlus1 - 1].iSubmissionType =
            IsFirstPipe() ? SUBMISSION_TYPE_MULTI_PIPE_MASTER : SUBMISSION_TYPE_MULTI_PIPE_SLAVE;

        if (IsLastPipe())
        {
            m_secondaryCmdBuffer[bufIdxPlus1 - 1].iSubmissionType |= SUBMISSION_TYPE_MULTI_PIPE_FLAGS_LAST_PIPE;
        }
    }
    *cmdBuffer = m_secondaryCmdBuffer[bufIdxPlus1 - 1];

    SCALABILITY_CHK_NULL_RETURN(m_osInterface->osCpInterface);
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);

    if (!m_attrReady)
    {
        SCALABILITY_CHK_STATUS_RETURN(SendAttrWithFrameTracking(m_primaryCmdBuffer, frameTrackingRequested));
        m_attrReady = true;
    }
    return eStatus;
}
MOS_STATUS EncodeScalabilityMultiPipe::ReturnCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (m_currentPipe >= m_pipeNum)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        SCALABILITY_ASSERTMESSAGE("Verify Command buffer failed with invalid parameter:currentPipe!");
        return eStatus;
    }
    if (m_currentPass >= m_maxNumBRCPasses)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        SCALABILITY_ASSERTMESSAGE("Verify Command buffer failed with invalid parameter:currentPass!");
        return eStatus;
    }
    uint32_t bufIdxPlus1 = m_currentPipe + 1;  //Make CMD buffer one next to one.
    m_secondaryCmdBuffer[bufIdxPlus1 - 1] = *cmdBuffer;  //Need to record the iOffset, ptr and other data of CMD buffer, it's not maintain in the mos.
    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &m_secondaryCmdBuffer[bufIdxPlus1 - 1], bufIdxPlus1);
    m_primaryCmdBuffer.Attributes.bFrequencyBoost |= cmdBuffer->Attributes.bFrequencyBoost;
    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &m_primaryCmdBuffer, 0);
    return eStatus;
}
MOS_STATUS EncodeScalabilityMultiPipe::SetHintParams()
{
    SCALABILITY_FUNCTION_ENTER;

    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    MOS_VIRTUALENGINE_SET_PARAMS veParams;
    MOS_ZeroMemory(&veParams, sizeof(veParams));

    veParams.ucScalablePipeNum = m_pipeNum;
    veParams.bScalableMode     = true;

    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnSetHintParams(m_osInterface, &veParams));
    return eStatus;
}
MOS_STATUS EncodeScalabilityMultiPipe::PopulateHintParams(PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);
    SCALABILITY_CHK_NULL_RETURN(m_veHitParams);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    MOS_STATUS            eStatus  = MOS_STATUS_SUCCESS;
    PMOS_CMD_BUF_ATTRI_VE attriVe  = m_osInterface->pfnGetAttributeVeBuffer(cmdBuffer);
    if (attriVe)
    {
        attriVe->VEngineHintParams     = *(m_veHitParams);
        attriVe->bUseVirtualEngineHint = true;
    }
    return eStatus;
}

MOS_STATUS EncodeScalabilityMultiPipe::SubmitCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    bool cmdBufferReadyForSubmit = IsPipeReadyToSubmit();

    // Hold the actual command buffer submission till last pipe
    if (!cmdBufferReadyForSubmit)
    {
        return eStatus;
    }

    // Add BB end for every secondary cmd buf when ready for submit
    for (uint32_t pipe = 0; pipe < m_pipeNum; pipe++)
    {
        uint32_t bufIdxPlus1 = pipe + 1;

        MOS_COMMAND_BUFFER& cmdBufferToAddBbEnd = m_secondaryCmdBuffer[bufIdxPlus1 - 1];
        SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBufferToAddBbEnd, bufIdxPlus1));
        SCALABILITY_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(&cmdBufferToAddBbEnd, nullptr));
        SCALABILITY_CHK_STATUS_RETURN(Oca1stLevelBBEnd(cmdBufferToAddBbEnd));
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBufferToAddBbEnd, bufIdxPlus1);
    }

    m_attrReady = false;

    SCALABILITY_CHK_STATUS_RETURN(SetHintParams());
    SCALABILITY_CHK_STATUS_RETURN(PopulateHintParams(&m_primaryCmdBuffer));

    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &m_primaryCmdBuffer, false));

    return eStatus;
}
MOS_STATUS EncodeScalabilityMultiPipe::SyncAllPipes(uint32_t semaphoreId, PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (semaphoreId >= m_maxSemaphoreNum)
    {
        SCALABILITY_ASSERTMESSAGE("SyncAllPipes failed with invalid parameter:semaphoreId!");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (Mos_ResourceIsNull(&m_resSemaphoreAllPipes[semaphoreId]))
    {
        return MOS_STATUS_UNKNOWN;
    }
    //Not stop watch dog here, expect to stop it in the packet when needed.
    //HW Semaphore cmd to make sure all pipes start encode at the same time
    SCALABILITY_CHK_STATUS_RETURN(m_hwInterface->SendMiAtomicDwordCmd(&m_resSemaphoreAllPipes[semaphoreId], 1, MHW_MI_ATOMIC_INC, cmdBuffer));
    SCALABILITY_CHK_STATUS_RETURN(m_hwInterface->SendHwSemaphoreWaitCmd(
        &m_resSemaphoreAllPipes[semaphoreId],
        m_pipeNum,
        MHW_MI_SAD_EQUAL_SDD,
        cmdBuffer));
    
    // Program some placeholder cmds to resolve the hazard between pipe sync
    auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
    storeDataParams                  = {};
    storeDataParams.pOsResource      = &m_resDelayMinus;
    storeDataParams.dwResourceOffset = 0;
    storeDataParams.dwValue          = 0xDE1A;
    
    for (uint32_t i = 0; i < m_numDelay; i++)
    {
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));
    }

    //clean HW semaphore memory
    SCALABILITY_CHK_STATUS_RETURN(m_hwInterface->SendMiAtomicDwordCmd(&m_resSemaphoreAllPipes[semaphoreId], 1, MHW_MI_ATOMIC_DEC, cmdBuffer));
    return eStatus;
}
MOS_STATUS EncodeScalabilityMultiPipe::SyncOnePipeWaitOthers(PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // Send MI_FLUSH command
    auto &miFlushDwParams            = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    miFlushDwParams                  = {};
    miFlushDwParams.bVideoPipelineCacheInvalidate = true;

    if (!Mos_ResourceIsNull(&m_resSemaphoreOnePipeWait[m_currentPipe]))
    {
        miFlushDwParams.pOsResource = &m_resSemaphoreOnePipeWait[m_currentPipe];
        miFlushDwParams.dwDataDW1   = m_currentPass + 1;
    }
    SCALABILITY_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

    if (IsFirstPipe())
    {
        // first pipe needs to ensure all other pipes are ready
        for (uint32_t i = 1; i < m_pipeNum; i++)
        {
            if (!Mos_ResourceIsNull(&m_resSemaphoreOnePipeWait[i]))
            {
                SCALABILITY_CHK_STATUS_RETURN(
                    m_hwInterface->SendHwSemaphoreWaitCmd(
                        &m_resSemaphoreOnePipeWait[i],
                        m_currentPass + 1,
                        MHW_MI_SAD_EQUAL_SDD,
                        cmdBuffer));
            }
        }
    }
    return eStatus;
}
MOS_STATUS EncodeScalabilityMultiPipe::SyncPipe(uint32_t syncType, uint32_t semaphoreId, PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    switch (syncType)
    {
    case syncAllPipes:
        eStatus = SyncAllPipes(semaphoreId, cmdBuffer);
        break;
    case syncOnePipeWaitOthers:
        eStatus = SyncOnePipeWaitOthers(cmdBuffer);
        break;
    case syncOnePipeForAnother:
        eStatus = SyncOnePipeForAnother(cmdBuffer);
        break;
    case syncOtherPipesForOne:
        eStatus = SyncOtherPipesForOne(cmdBuffer);
        break;
    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        break;
    }
    return eStatus;
}
MOS_STATUS EncodeScalabilityMultiPipe::ResetSemaphore(uint32_t syncType, uint32_t semaphoreId, PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    switch (syncType)
    {
    case syncAllPipes:
        if (semaphoreId >= m_maxSemaphoreNum)
        {
            SCALABILITY_ASSERTMESSAGE("SyncAllPipes failed with invalid parameter:semaphoreId!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        if (!Mos_ResourceIsNull(&m_resSemaphoreAllPipes[semaphoreId]))
        {
            SCALABILITY_CHK_STATUS_RETURN(
                m_hwInterface->SendMiStoreDataImm(
                    &m_resSemaphoreAllPipes[semaphoreId],
                    0,
                    cmdBuffer));
        }
        break;
    case syncOnePipeWaitOthers:
        if (!Mos_ResourceIsNull(&m_resSemaphoreOnePipeWait[semaphoreId]))
        {
            SCALABILITY_CHK_STATUS_RETURN(
                m_hwInterface->SendMiStoreDataImm(
                    &m_resSemaphoreOnePipeWait[semaphoreId],
                    0,
                    cmdBuffer));
        }
        break;
    case syncOnePipeForAnother:
        if (!Mos_ResourceIsNull(&m_resSemaphoreOnePipeForAnother))
        {
            SCALABILITY_CHK_STATUS_RETURN(
                m_hwInterface->SendMiStoreDataImm(
                    &m_resSemaphoreOnePipeForAnother,
                    0,
                    cmdBuffer));
        }
        break;
    case syncOtherPipesForOne:
        if (!Mos_ResourceIsNull(&m_resSemaphoreOtherPipesForOne))
        {
            SCALABILITY_CHK_STATUS_RETURN(
                m_hwInterface->SendMiStoreDataImm(
                    &m_resSemaphoreOtherPipesForOne,
                    0,
                    cmdBuffer));
        }
        break;
    default:
        SCALABILITY_ASSERTMESSAGE("Reset semaphore failed with invalid parameter: syncType!");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        break;
    }
    return eStatus;
}
MOS_STATUS EncodeScalabilityMultiPipe::UpdateState(void *statePars)
{
    SCALABILITY_FUNCTION_ENTER;
    MOS_STATUS   eStatus         = MOS_STATUS_SUCCESS;
    StateParams *encodeStatePars = (StateParams *)statePars;
    if (encodeStatePars->currentPipe >= m_pipeNum)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        SCALABILITY_ASSERTMESSAGE("UpdateState failed with invalid parameter:currentPipe!");
        return eStatus;
    }
    if (m_currentPass >= m_maxNumBRCPasses)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        SCALABILITY_ASSERTMESSAGE("UpdateState failed with invalid parameter:currentPass!");
        return eStatus;
    }
    m_currentPipe              = encodeStatePars->currentPipe;
    m_currentPass              = encodeStatePars->currentPass;
    m_pipeIndexForSubmit       = encodeStatePars->pipeIndexForSubmit;
    m_singleTaskPhaseSupported = encodeStatePars->singleTaskPhaseSupported;
    m_statusReport             = encodeStatePars->statusReport;
    m_currentRow               = encodeStatePars->currentRow;
    m_currentSubPass           = encodeStatePars->currentSubPass;
    return eStatus;
}

MOS_STATUS EncodeScalabilityMultiPipe::SendAttrWithFrameTracking(
    MOS_COMMAND_BUFFER &cmdBuffer,
    bool                frameTrackingRequested)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    SCALABILITY_FUNCTION_ENTER;

    bool renderEngineUsed = m_mediaContext->IsRenderEngineUsed();

    // initialize command buffer attributes
    cmdBuffer.Attributes.bTurboMode               = m_hwInterface->m_turboMode;
    cmdBuffer.Attributes.bMediaPreemptionEnabled  = renderEngineUsed ? m_hwInterface->GetRenderInterfaceNext()->IsPreemptionEnabled() : 0;
    cmdBuffer.Attributes.dwNumRequestedEUSlices   = m_hwInterface->m_numRequestedEuSlices;
    cmdBuffer.Attributes.dwNumRequestedSubSlices  = m_hwInterface->m_numRequestedSubSlices;
    cmdBuffer.Attributes.dwNumRequestedEUs        = m_hwInterface->m_numRequestedEus;
    cmdBuffer.Attributes.bValidPowerGatingRequest = true;

    PMOS_RESOURCE resource = nullptr;
    uint32_t      offset   = 0;

    if (frameTrackingRequested && m_frameTrackingEnabled)
    {
        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(encode::statusReportGlobalCount, resource, offset));
        cmdBuffer.Attributes.bEnableMediaFrameTracking    = true;
        cmdBuffer.Attributes.resMediaFrameTrackingSurface = resource;
        cmdBuffer.Attributes.dwMediaFrameTrackingTag      = m_statusReport->GetSubmittedCount() + 1;
        // Set media frame tracking address offset(the offset from the encoder status buffer page)
        cmdBuffer.Attributes.dwMediaFrameTrackingAddrOffset = 0;
    }

    return eStatus;
}

MOS_STATUS EncodeScalabilityMultiPipe::SyncOnePipeForAnother(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    SCALABILITY_FUNCTION_ENTER;

    if (m_currentPipe == 0)
    {
        SCALABILITY_CHK_STATUS_RETURN(m_hwInterface->SendMiAtomicDwordCmd(&m_resSemaphoreOnePipeForAnother, 1, MHW_MI_ATOMIC_INC, cmdBuffer));
    }
    else
    {
        SCALABILITY_CHK_STATUS_RETURN(m_hwInterface->SendHwSemaphoreWaitCmd(
            &m_resSemaphoreOnePipeForAnother,
            1,
            MHW_MI_SAD_EQUAL_SDD,
            cmdBuffer));
        //clean HW semaphore memory
        SCALABILITY_CHK_STATUS_RETURN(m_hwInterface->SendMiAtomicDwordCmd(&m_resSemaphoreOnePipeForAnother, 1, MHW_MI_ATOMIC_DEC, cmdBuffer));
    }

    return eStatus;
}

MOS_STATUS EncodeScalabilityMultiPipe::SyncOtherPipesForOne(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    SCALABILITY_FUNCTION_ENTER;

    if (m_currentPipe == 0)
    {
        SCALABILITY_CHK_STATUS_RETURN(m_hwInterface->SendMiAtomicDwordCmd(&m_resSemaphoreOtherPipesForOne, m_pipeNum - 1, MHW_MI_ATOMIC_INC, cmdBuffer));
    }
    else
    {
        SCALABILITY_CHK_STATUS_RETURN(m_hwInterface->SendHwSemaphoreWaitCmd(
            &m_resSemaphoreOtherPipesForOne,
            0,
            MHW_MI_SAD_NOT_EQUAL_SDD,
            cmdBuffer));
        //clean HW semaphore memory
        SCALABILITY_CHK_STATUS_RETURN(m_hwInterface->SendMiAtomicDwordCmd(&m_resSemaphoreOtherPipesForOne, 1, MHW_MI_ATOMIC_DEC, cmdBuffer));
    }

    return eStatus;
}

MOS_STATUS EncodeScalabilityMultiPipe::Oca1stLevelBBStart(MOS_COMMAND_BUFFER &cmdBuffer)
{
    MHW_MI_MMIOREGISTERS mmioRegister;
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);

    auto vdencItf = m_hwInterface->GetVdencInterfaceNext();
    SCALABILITY_CHK_NULL_RETURN(vdencItf);
    bool validMmio = vdencItf->ConvertToMiRegister(MHW_VDBOX_NODE_1, mmioRegister);

    if (validMmio)
    {
        SCALABILITY_CHK_NULL_RETURN(m_osInterface);
        SCALABILITY_CHK_NULL_RETURN(m_osInterface->pOsContext);

        HalOcaInterfaceNext::On1stLevelBBStart(
            cmdBuffer,
            (MOS_CONTEXT_HANDLE)m_osInterface->pOsContext,
            m_osInterface->CurrentGpuContextHandle,
            m_miItf,
            mmioRegister);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeScalabilityMultiPipe::Oca1stLevelBBEnd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);
    HalOcaInterfaceNext::On1stLevelBBEnd(cmdBuffer, *m_osInterface);

    return MOS_STATUS_SUCCESS;
}

}
