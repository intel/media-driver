/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     codechal_encode_scalability.cpp
//! \brief    Implements the encode interface extension for encode scalability.
//! \details  Implements all functions required by CodecHal for scalability encoding.
//!

#include "codechal_encoder_base.h"
#include "codechal_encode_scalability.h"
#include "mos_util_user_interface.h"
#include "mos_os_virtualengine_next.h"

MOS_STATUS CodecHalEncodeScalability_InitializeState (
    PCODECHAL_ENCODE_SCALABILITY_STATE  pScalabilityState,
    CodechalHwInterface                 *hwInterface)
{
    PMOS_VIRTUALENGINE_INTERFACE   pVEInterface;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE                 osInterface;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hwInterface);
    osInterface = hwInterface->GetOsInterface();
    CODECHAL_ENCODE_CHK_NULL_RETURN(osInterface);

    pScalabilityState->pHwInterface           = hwInterface;
    pScalabilityState->ucScalablePipeNum      = 1;
    pScalabilityState->VideoContextSinglePipe = MOS_GPU_CONTEXT_VIDEO3;
    pScalabilityState->VideoContextScalable   = MOS_GPU_CONTEXT_INVALID_HANDLE;

    //virtual engine init with scalability
    MOS_VIRTUALENGINE_INIT_PARAMS   VEInitParms;
    MOS_ZeroMemory(&VEInitParms, sizeof(VEInitParms));
    VEInitParms.bScalabilitySupported           = true;

    // Disabling the Secondary command buffer creation in MOS_VE
    // To be programmed once Encode moves to using secondary command buffers in MOS VE interface
    VEInitParms.ucMaxNumOfSdryCmdBufInOneFrame  = VEInitParms.ucNumOfSdryCmdBufSets = 0;
    VEInitParms.ucMaxNumPipesInUse              = MOS_MAX_ENGINE_INSTANCE_PER_CLASS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(Mos_VirtualEngineInterface_Initialize(osInterface, &VEInitParms));
    pScalabilityState->pVEInterface = pVEInterface = osInterface->pVEInterf;

    if (pVEInterface->pfnVEGetHintParams)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pVEInterface->pfnVEGetHintParams(pVEInterface, true, &pScalabilityState->pScalHintParms));
    }

    if (pVEInterface->pfnVEGetHintParams)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pVEInterface->pfnVEGetHintParams(pVEInterface, false, &pScalabilityState->pSingleHintParms));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeScalability_ConstructParmsForGpuCtxCreation(
    PCODECHAL_ENCODE_SCALABILITY_STATE         pScalState,
    PMOS_GPUCTX_CREATOPTIONS_ENHANCED          gpuCtxCreatOpts)
{
    MOS_STATUS                               eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pScalState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pScalState->pHwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(gpuCtxCreatOpts);

    gpuCtxCreatOpts->UsingSFC = false;
    gpuCtxCreatOpts->LRCACount = pScalState->ucScalablePipeNum;

#if (_DEBUG || _RELEASE_INTERNAL)
    PMOS_INTERFACE pOsInterface    = pScalState->pHwInterface->GetOsInterface();
    CODECHAL_ENCODE_CHK_NULL_RETURN(pOsInterface);

    if (pOsInterface->bEnableDbgOvrdInVE)
    {
        PMOS_VIRTUALENGINE_INTERFACE pVEInterface = pScalState->pVEInterface;

        CODECHAL_ENCODE_CHK_NULL_RETURN(pVEInterface);
        gpuCtxCreatOpts->DebugOverride      = true;
        if (g_apoMosEnabled)
        {
            CODECHAL_ENCODE_CHK_NULL_RETURN(pVEInterface->veInterface);
            for (uint32_t i = 0; i < pVEInterface->veInterface->GetEngineCount(); i++)
            {
                gpuCtxCreatOpts->EngineInstance[i] = pVEInterface->veInterface->GetEngineLogicId(i);
            }
        }
        else
        {
            for (uint32_t i = 0; i < pVEInterface->ucEngineCount; i++)
            {
                gpuCtxCreatOpts->EngineInstance[i] = pVEInterface->EngineLogicId[i];
            }
        }

    }
#endif
    return eStatus;
}

MOS_STATUS CodecHalEncodeScalability_PopulateHintParams(
    PCODECHAL_ENCODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 cmdBuffer)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    PMOS_CMD_BUF_ATTRI_VE pAttriVe = (PMOS_CMD_BUF_ATTRI_VE)(cmdBuffer->Attributes.pAttriVe);

    if (pAttriVe)
    {
        if (pScalabilityState->ucScalablePipeNum >= 2)
        {
            CODECHAL_ENCODE_CHK_NULL_RETURN(pScalabilityState->pScalHintParms);
            pAttriVe->VEngineHintParams = *(pScalabilityState->pScalHintParms);
        }
        else
        {
            CODECHAL_ENCODE_CHK_NULL_RETURN(pScalabilityState->pSingleHintParms);
            pAttriVe->VEngineHintParams = *(pScalabilityState->pSingleHintParms);
        }
        pAttriVe->bUseVirtualEngineHint = true;
    }

    return eStatus;
}

MOS_STATUS CodecHalEncodeScalability_SetHintParams(
    CodechalEncoderState                       *pEncoder,
    PCODECHAL_ENCODE_SCALABILITY_STATE         pScalabilityState,
    PCODECHAL_ENCODE_SCALABILITY_SETHINT_PARMS pSetHintParms)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE                  pOsInterface;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pSetHintParms);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface->GetOsInterface());

    pOsInterface = pScalabilityState->pHwInterface->GetOsInterface();
    PMOS_VIRTUALENGINE_INTERFACE pVEInterface = pScalabilityState->pVEInterface;

    MOS_VIRTUALENGINE_SET_PARAMS    VEParams;
    MOS_ZeroMemory(&VEParams, sizeof(VEParams));

    VEParams.ucScalablePipeNum  = pScalabilityState->ucScalablePipeNum;
    VEParams.bScalableMode      = (pScalabilityState->ucScalablePipeNum >= 2);

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(pOsInterface))
    {
        //not used by VE2.0
        VEParams.bNeedSyncWithPrevious       = pSetHintParms->bNeedSyncWithPrevious;
        VEParams.bSameEngineAsLastSubmission = pSetHintParms->bSameEngineAsLastSubmission;
    }

    if (pScalabilityState->ucScalablePipeNum >= 2)
    {
        for (auto i = 0; i < pScalabilityState->ucScalablePipeNum; i++)
        {
            VEParams.veBatchBuffer[i] = pSetHintParms->veBatchBuffer[i];
        }
    }
    if (pVEInterface->pfnVESetHintParams)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pVEInterface->pfnVESetHintParams(pVEInterface, &VEParams));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeScalability_ChkGpuCtxReCreation(
    CodechalEncoderState                       *pEncoder,
    PCODECHAL_ENCODE_SCALABILITY_STATE         pScalabilityState,
    PMOS_GPUCTX_CREATOPTIONS_ENHANCED          CurgpuCtxCreatOpts)
{
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE pOsInterface;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(CurgpuCtxCreatOpts);

    pOsInterface = pScalabilityState->pHwInterface->GetOsInterface();
    bool                changed = false;
    CODECHAL_ENCODE_CHK_NULL_RETURN(pOsInterface);

#if (_DEBUG || _RELEASE_INTERNAL)
    if (pOsInterface->bEnableDbgOvrdInVE)
    {
        changed = false;
    }
    else
#endif
    {
        if (CurgpuCtxCreatOpts->LRCACount != pScalabilityState->ucScalablePipeNum)
        {
            changed = true;
            CurgpuCtxCreatOpts->LRCACount = pScalabilityState->ucScalablePipeNum;
        }
        else
        {
            changed = false;
        }
    }

    if (changed)
    {
        // Create a scalable GPU context once based on MOS_GPU_CONTEXT_VDBOX2_VIDEO3 if needed
        if (pScalabilityState->VideoContextScalable == MOS_GPU_CONTEXT_INVALID_HANDLE)
        {
            pScalabilityState->VideoContextScalable = MOS_VE_MULTINODESCALING_SUPPORTED(pOsInterface) ? MOS_GPU_CONTEXT_VIDEO6 : MOS_GPU_CONTEXT_VDBOX2_VIDEO3;

            eStatus = (MOS_STATUS)pOsInterface->pfnCreateGpuContext(
                pOsInterface,
                pScalabilityState->VideoContextScalable,
                MOS_GPU_NODE_VIDEO,
                CurgpuCtxCreatOpts);

            CODECHAL_ENCODE_CHK_STATUS_RETURN(pOsInterface->pfnRegisterBBCompleteNotifyEvent(
                pOsInterface,
                pScalabilityState->VideoContextScalable));
        }

        // Switch across single pipe/ scalable mode gpu contexts
        MOS_GPU_CONTEXT GpuContext = (pScalabilityState->ucScalablePipeNum == 1) ? pScalabilityState->VideoContextSinglePipe : pScalabilityState->VideoContextScalable;
        pEncoder->SetVideoContext(GpuContext);
        pOsInterface->pfnSetEncodePakContext(pOsInterface, GpuContext);

    }

    return eStatus;
}

void CodecHalEncodeScalability_EncodePhaseToSubmissionType(
    bool isFirstPipe,
    PMOS_COMMAND_BUFFER pCmdBuffer)
{
    if (isFirstPipe)
    {
        pCmdBuffer->iSubmissionType = SUBMISSION_TYPE_MULTI_PIPE_MASTER;
    }
    else
    {
        pCmdBuffer->iSubmissionType = SUBMISSION_TYPE_MULTI_PIPE_SLAVE;
    }
}
