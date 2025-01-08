/*
* Copyright (c) 2020-2023, Intel Corporation
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
#include "mos_os_virtualengine_scalability.h"
#include "media_context.h"
#include "media_status_report.h"
#include "mhw_utilities.h"
#include "mhw_mi_cmdpar.h"

namespace vp
{
VpScalabilityMultiPipe::VpScalabilityMultiPipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType)
    : VpScalabilityMultiPipeNext(hwInterface, mediaContext, componentType)
{
}

VpScalabilityMultiPipe::~VpScalabilityMultiPipe()
{

}

MOS_STATUS VpScalabilityMultiPipe::Destroy()
{
    VP_FUNC_CALL();

    SCALABILITY_FUNCTION_ENTER;

    SCALABILITY_CHK_STATUS_RETURN(VpScalabilityMultiPipeNext::Destroy());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpScalabilityMultiPipe::Initialize(const MediaScalabilityOption &option)
{
    VP_FUNC_CALL();

    SCALABILITY_FUNCTION_ENTER;

    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);

    m_osInterface = m_hwInterface->m_osInterface;
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface->m_vpPlatformInterface);
    m_miItf = m_hwInterface->m_vpPlatformInterface->GetMhwMiItf();

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
        SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnVirtualEngineInit(m_osInterface, &m_veHitParams, veInitParms));
        m_veState = m_osInterface->osStreamState->virtualEngineInterface;
        SCALABILITY_CHK_NULL_RETURN(m_veState);
        SCALABILITY_CHK_NULL_RETURN(m_veHitParams);
    }
    else
    {
        SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnVirtualEngineInit(m_osInterface, &m_veHitParams, veInitParms));
        m_veInterface = m_osInterface->pVEInterf;
        SCALABILITY_CHK_NULL_RETURN(m_veInterface);
        if (m_veInterface->pfnVEGetHintParams != nullptr)
        {
            SCALABILITY_CHK_NULL_RETURN(m_veHitParams);
        }
    }
    m_pipeNum            = m_scalabilityOption->GetNumPipe();
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

//!
//! \brief    Send hw semphore wait cmd
//! \details  Send hw semphore wait cmd for sync perpose
//!
//! \param    [in] semaMem
//!           Reource of Hw semphore
//! \param    [in] offset
//!           offset of semMem
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
    uint32_t                                  offset,
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

    if (m_miItf)
    {
        auto &params             = m_miItf->MHW_GETPAR_F(MI_SEMAPHORE_WAIT)();
        params                   = {};
        params.presSemaphoreMem = semaMem;
        params.dwResourceOffset = offset;
        params.bPollingWaitMode = true;
        params.dwSemaphoreData  = semaData;
        params.CompareOperation = (mhw::mi::MHW_COMMON_MI_SEMAPHORE_COMPARE_OPERATION) opCode;
        eStatus                 = m_miItf->MHW_ADDCMD_F(MI_SEMAPHORE_WAIT)(cmdBuffer);
    }
    else
    {
        MOS_ZeroMemory((&miSemaphoreWaitParams), sizeof(miSemaphoreWaitParams));
        miSemaphoreWaitParams.presSemaphoreMem = semaMem;
        miSemaphoreWaitParams.dwResourceOffset = offset;
        miSemaphoreWaitParams.bPollingWaitMode = true;
        miSemaphoreWaitParams.dwSemaphoreData  = semaData;
        miSemaphoreWaitParams.CompareOperation = opCode;
        eStatus                                = pMhwMiInterface->AddMiSemaphoreWaitCmd(cmdBuffer, &miSemaphoreWaitParams);
    }

finish:
    return eStatus;
}

//!
//! \brief    Send mi atomic dword cmd
//! \details  Send mi atomic dword cmd for sync perpose
//!
//! \param    [in] resource
//!           Reource used in mi atomic dword cmd
//! \param    [in] offset
//!           offset of resource
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
    uint32_t                    offset,
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

    if (m_miItf)
    {
        auto &params             = m_miItf->MHW_GETPAR_F(MI_ATOMIC)();
        params                   = {};
        params.pOsResource       = resource;
        params.dwResourceOffset  = offset;
        params.dwDataSize        = sizeof(uint32_t);
        params.Operation         = (mhw::mi::MHW_COMMON_MI_ATOMIC_OPCODE) opCode;
        params.bInlineData       = true;
        params.dwOperand1Data[0] = immData;
        eStatus                  = m_miItf->MHW_ADDCMD_F(MI_ATOMIC)(cmdBuffer);
    }
    else
    {
        MOS_ZeroMemory((&atomicParams), sizeof(atomicParams));
        atomicParams.pOsResource       = resource;
        atomicParams.dwResourceOffset  = offset;
        atomicParams.dwDataSize        = sizeof(uint32_t);
        atomicParams.Operation         = opCode;
        atomicParams.bInlineData       = true;
        atomicParams.dwOperand1Data[0] = immData;
        eStatus = pMhwMiInterface->AddMiAtomicCmd(cmdBuffer, &atomicParams);
    }

    return eStatus;
}

//!
//! \brief    Send mi flush dword cmd
//! \details  Send mi flush dword cmd for sync perpose
//!
//! \param    [in] semMem
//!           Reource used in mi flush dword cmd
//! \param    [in] semaData
//!           Immediate data
//! \param    [in,out] cmdBuffer
//!           command buffer
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VpScalabilityMultiPipe::AddMiFlushDwCmd(
    PMOS_RESOURCE                             semaMem,
    uint32_t                                  semaData,
    PMOS_COMMAND_BUFFER                       cmdBuffer)
{
    MOS_STATUS           eStatus = MOS_STATUS_SUCCESS;
    PMHW_MI_INTERFACE    pMhwMiInterface;
    MHW_MI_ATOMIC_PARAMS atomicParams;

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_mhwMiInterface);

    pMhwMiInterface = m_hwInterface->m_mhwMiInterface;

    // Send MI_FLUSH command
    if (m_miItf)
    {
        auto& parFlush = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        parFlush = {};
        parFlush.bVideoPipelineCacheInvalidate = true;
        if (!Mos_ResourceIsNull(semaMem))
        {
            parFlush.pOsResource = semaMem;
            parFlush.dwDataDW1   = semaData + 1;
        }
        SCALABILITY_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));
    }
    else
    {
        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.bVideoPipelineCacheInvalidate = true;
        if (!Mos_ResourceIsNull(semaMem))
        {
            flushDwParams.pOsResource = semaMem;
            flushDwParams.dwDataDW1   = semaData + 1;
        }
        SCALABILITY_CHK_STATUS_RETURN(pMhwMiInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));
    }

    return eStatus;
}

//!
//! \brief    Send mi store data dword cmd
//! \details  Send mi store dat dword cmd for sync perpose
//!
//! \param    [in] resource
//!           Reource used in mi store dat dword cmd
//! \param    [in] offset
//!           offset of resource
//! \param    [in,out] cmdBuffer
//!           command buffer
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VpScalabilityMultiPipe::AddMiStoreDataImmCmd(
    PMOS_RESOURCE               resource,
    uint32_t                    offset,
    PMOS_COMMAND_BUFFER         cmdBuffer)
{
    VP_FUNC_CALL();

    MOS_STATUS           eStatus = MOS_STATUS_SUCCESS;
    PMHW_MI_INTERFACE    pMhwMiInterface;
    MHW_MI_ATOMIC_PARAMS atomicParams;

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_mhwMiInterface);

    pMhwMiInterface = m_hwInterface->m_mhwMiInterface;

    if (m_miItf)
    {
        auto &params             = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        params                   = {};
        params.pOsResource       = resource;
        params.dwResourceOffset  = offset;
        params.dwValue           = 0;
        eStatus                  = m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer);
    }
    else
    {
        MHW_MI_STORE_DATA_PARAMS dataParams = {};
        dataParams.pOsResource      = resource;
        dataParams.dwResourceOffset = offset;
        dataParams.dwValue          = 0;

        // Reset current pipe semaphore
        SCALABILITY_CHK_STATUS_RETURN(pMhwMiInterface->AddMiStoreDataImmCmd(
            cmdBuffer, &dataParams));
    }

    return eStatus;
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
    uint32_t bufIdx      = m_currentPipe;  //Make CMD buffer one next to one.
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
        SCALABILITY_CHK_NULL_RETURN(m_osInterface->osStreamState->virtualEngineInterface);
        SCALABILITY_CHK_STATUS_RETURN(m_osInterface->osStreamState->virtualEngineInterface->SetSubmissionType(&(m_secondaryCmdBuffers[bufIdx]), submissionType));
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

    *cmdBuffer                            = m_secondaryCmdBuffers[bufIdx];
    m_secondaryCmdBuffersReturned[bufIdx] = false;

    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);

    if (!m_attrReady)
    {
        SCALABILITY_CHK_STATUS_RETURN(SendAttrWithFrameTracking(m_primaryCmdBuffer, frameTrackingRequested));
        m_attrReady = true;
    }

    return eStatus;
}

MOS_STATUS VpScalabilityMultiPipe::SetHintParams()
{
    VP_FUNC_CALL();

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

    VpScalabilityOption *vpScalabilityOption = dynamic_cast<VpScalabilityOption *>(m_scalabilityOption);
    SCALABILITY_CHK_NULL_RETURN(vpScalabilityOption);

    MOS_VIRTUALENGINE_SET_PARAMS veParams;
    MOS_ZeroMemory(&veParams, sizeof(veParams));

    veParams.ucScalablePipeNum = m_pipeNum;
    veParams.bScalableMode     = true;

    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnSetHintParams(m_osInterface, &veParams));

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

MOS_STATUS VpScalabilityMultiPipe::CreateMultiPipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType)
{
    SCALABILITY_CHK_NULL_RETURN(hwInterface);
    SCALABILITY_CHK_NULL_RETURN(mediaContext);

    ((PVP_MHWINTERFACE)hwInterface)->m_multiPipeScalability = MOS_New(VpScalabilityMultiPipe, hwInterface, mediaContext, scalabilityVp);
    SCALABILITY_CHK_NULL_RETURN(((PVP_MHWINTERFACE)hwInterface)->m_multiPipeScalability);
    return MOS_STATUS_SUCCESS;
}
}
