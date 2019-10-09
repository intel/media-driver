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
//! \file      cm_hal_g10.cpp 
//! \brief     Common HAL CM Gen10 functions 
//!

#include "cm_hal_g10.h"
#include "mhw_render_hwcmd_g10_X.h"
#include "mhw_state_heap_hwcmd_g10_X.h"
#include "cm_def.h"
#include "renderhal_platform_interface.h"
#include "mhw_render.h"
#if defined(ENABLE_KERNELS) && (!defined(_FULL_OPEN_SOURCE))
#include "cm_gpucopy_kernel_g10.h"
#include "cm_gpuinit_kernel_g10.h"
#else
unsigned int iGPUCopy_kernel_isa_size_gen10 = 0;
unsigned int iGPUInit_kernel_isa_size_Gen10 = 0;
unsigned char *pGPUCopy_kernel_isa_gen10 = nullptr;
unsigned char *pGPUInit_kernel_isa_Gen10 = nullptr;

#endif

// Gen10 Surface state tokenized commands - a SURFACE_STATE_G10 command and
// a surface state command, either SURFACE_STATE_G10 or SURFACE_STATE_ADV_G10
struct PACKET_SURFACE_STATE
{
    SURFACE_STATE_TOKEN_COMMON token;
    union
    {
        mhw_state_heap_g10_X::RENDER_SURFACE_STATE_CMD cmdSurfaceState;
        mhw_state_heap_g10_X::MEDIA_SURFACE_STATE_CMD cmdSurfaceStateAdv;
    };
};

#if (_RELEASE_INTERNAL || _DEBUG)
#if defined(CM_DIRECT_GUC_SUPPORT)
MOS_STATUS CM_HAL_G10_X::SubmitDummyCommands(
    PMHW_BATCH_BUFFER       batchBuffer,
    int32_t                 taskId,
    PCM_HAL_KERNEL_PARAM    *kernelParam,
    void                    **cmdBuffer) 
{
    return MOS_STATUS_UNIMPLEMENTED;

}
#endif
#endif

MOS_STATUS CM_HAL_G10_X::SubmitCommands(
    PMHW_BATCH_BUFFER       batchBuffer,
    int32_t                 taskId,
    PCM_HAL_KERNEL_PARAM    *kernelParam,
    void                    **cmdBuffer)
{
    MOS_STATUS                   eStatus        = MOS_STATUS_SUCCESS;
    PCM_HAL_STATE                state          = m_cmState;
    PRENDERHAL_INTERFACE         renderHal      = state->renderHal;
    PMOS_INTERFACE               osInterface    = renderHal->pOsInterface;
    MhwRenderInterface           *mhwRender     = renderHal->pMhwRenderInterface;
    PMHW_MI_INTERFACE            mhwMiInterface = renderHal->pMhwMiInterface;
    PRENDERHAL_STATE_HEAP        stateHeap      = renderHal->pStateHeap;
    MHW_PIPE_CONTROL_PARAMS      pipeCtlParams   = g_cRenderHal_InitPipeControlParams;
    MHW_MEDIA_STATE_FLUSH_PARAM  flushParam      = g_cRenderHal_InitMediaStateFlushParams;
    MHW_ID_LOAD_PARAMS           idLoadParams;
    int32_t                      remaining      = 0;
    bool                         enableWalker    = state->walkerParams.CmWalkerEnable;
    bool                         enableGpGpu     = state->taskParam->blGpGpuWalkerEnabled;
    PCM_HAL_TASK_PARAM           taskParam = state->taskParam;
    PCM_HAL_BB_ARGS              bbCmArgs;
    MOS_COMMAND_BUFFER           mosCmdBuffer;
    uint32_t                     syncTag;
    int64_t                      *taskSyncLocation;
    int32_t                      syncOffset;
    int32_t                      tmp;
    bool                         sipEnable = renderHal->bSIPKernel? true: false;
    bool                         csrEnable = renderHal->bCSRKernel ? true : false;

    RENDERHAL_GENERIC_PROLOG_PARAMS genericPrologParams = {};
    MOS_RESOURCE                 *osResource;
    uint32_t                     tag;
    uint32_t                     tagOffset = 0;

    MOS_ZeroMemory(&mosCmdBuffer, sizeof(MOS_COMMAND_BUFFER));
    // get the tag
    tag        = renderHal->trackerProducer.GetNextTracker(renderHal->currentTrackerIndex);

    // Get the task sync offset
    syncOffset     = state->pfnGetTaskSyncLocation(state, taskId);

    // Initialize the location
    taskSyncLocation                 = (int64_t*)(state->renderTimeStampResource.data + syncOffset);
    *taskSyncLocation                = CM_INVALID_INDEX;
    *(taskSyncLocation + 1)          = CM_INVALID_INDEX;
    if(state->cbbEnabled)
    {
        *(taskSyncLocation + 2)      = tag;
        *(taskSyncLocation + 3)      = state->renderHal->currentTrackerIndex;
    }

    // Register batch buffer for rendering
    if (!enableWalker && !enableGpGpu)
    {
        CM_CHK_MOSSTATUS_GOTOFINISH(osInterface->pfnRegisterResource(
            osInterface,
            &batchBuffer->OsResource,
            true,
            true));
    }

    // Register Timestamp Buffer
    CM_CHK_MOSSTATUS_GOTOFINISH(osInterface->pfnRegisterResource(
        osInterface,
        &state->renderTimeStampResource.osResource,
        true,
        true));

    // Allocate all available space, unused buffer will be returned later
    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnGetCommandBuffer(osInterface, &mosCmdBuffer, 0));
    remaining = mosCmdBuffer.iRemaining;

    // use frame tracking to write the tracker ID to CM tracker resource
    renderHal->trackerProducer.GetLatestTrackerResource(renderHal->currentTrackerIndex, &osResource, &tagOffset);
    renderHal->pfnSetupPrologParams(renderHal, &genericPrologParams, osResource, tagOffset, tag);
    FrameTrackerTokenFlat_SetProducer(&stateHeap->pCurMediaState->trackerToken, &renderHal->trackerProducer);
    FrameTrackerTokenFlat_Merge(&stateHeap->pCurMediaState->trackerToken, renderHal->currentTrackerIndex, tag);

    // Record registers by unified media profiler in the beginning
    if (state->perfProfiler != nullptr)
    {
        CM_CHK_MOSSTATUS_GOTOFINISH(state->perfProfiler->AddPerfCollectStartCmd((void *)state, state->osInterface, mhwMiInterface, &mosCmdBuffer));
    }

    //Send the First PipeControl Command to indicate the beginning of execution
    pipeCtlParams = g_cRenderHal_InitPipeControlParams;
    pipeCtlParams.presDest          = &state->renderTimeStampResource.osResource;
    pipeCtlParams.dwResourceOffset  = syncOffset;
    pipeCtlParams.dwPostSyncOp      = MHW_FLUSH_WRITE_TIMESTAMP_REG;
    pipeCtlParams.dwFlushMode       = MHW_FLUSH_WRITE_CACHE;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(&mosCmdBuffer, nullptr, &pipeCtlParams));

    // Initialize command buffer and insert prolog
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnInitCommandBuffer(renderHal, &mosCmdBuffer, &genericPrologParams));

    // update tracker tag used with CM tracker resource
    renderHal->trackerProducer.StepForward(renderHal->currentTrackerIndex);

    // Increment sync tag
    syncTag = stateHeap->dwNextTag++;

    //enable CNL L3 config
    HalCm_GetLegacyRenderHalL3Setting( &state->l3Settings, &renderHal->L3CacheSettings );
    renderHal->pfnEnableL3Caching(renderHal, &renderHal->L3CacheSettings);
    mhwRender->SetL3Cache(&mosCmdBuffer);

    if (sipEnable)
    {
        CM_CHK_MOSSTATUS_GOTOFINISH(SetupHwDebugControl(renderHal, &mosCmdBuffer));
    }

    // add granularity control for preemption for CNL
    // Supporting Preemption granularity control reg for 3D and GPGPU mode for per ctx and with non-privileged access
    if (MEDIA_IS_SKU(state->skuTable, FtrPerCtxtPreemptionGranularityControl))
    {
        MHW_MI_LOAD_REGISTER_IMM_PARAMS loadRegImm;
        MOS_ZeroMemory(&loadRegImm, sizeof(MHW_MI_LOAD_REGISTER_IMM_PARAMS));

        loadRegImm.dwRegister = MHW_RENDER_ENGINE_PREEMPTION_CONTROL_OFFSET;

        // Same reg offset and value for gpgpu pipe and media pipe
        if (enableGpGpu)
        {
            if (MEDIA_IS_SKU(state->skuTable, FtrGpGpuMidThreadLevelPreempt))
            {
                if (csrEnable)
                    loadRegImm.dwData = MHW_RENDER_ENGINE_MID_THREAD_PREEMPT_VALUE;
                else
                    loadRegImm.dwData = MHW_RENDER_ENGINE_THREAD_GROUP_PREEMPT_VALUE;
            }
            else if (MEDIA_IS_SKU(state->skuTable, FtrGpGpuThreadGroupLevelPreempt))
            {
                loadRegImm.dwData = MHW_RENDER_ENGINE_THREAD_GROUP_PREEMPT_VALUE;
                state->renderHal->pfnEnableGpgpuMiddleBatchBufferPreemption(state->renderHal);
            }
            else if (MEDIA_IS_SKU(state->skuTable, FtrGpGpuMidBatchPreempt))
            {
                loadRegImm.dwData = MHW_RENDER_ENGINE_MID_BATCH_PREEMPT_VALUE;
                state->renderHal->pfnEnableGpgpuMiddleBatchBufferPreemption(state->renderHal);
            }
            else
            {
                // if hit this branch then platform does not support any media preemption in render engine. Still program the register to avoid GPU hang
                loadRegImm.dwData = MHW_RENDER_ENGINE_MID_BATCH_PREEMPT_VALUE;
            }
        }
        else
        {
            if (MEDIA_IS_SKU(state->skuTable, FtrMediaMidThreadLevelPreempt))
            {
                loadRegImm.dwData = MHW_RENDER_ENGINE_MID_THREAD_PREEMPT_VALUE;
            }
            else if (MEDIA_IS_SKU(state->skuTable, FtrMediaThreadGroupLevelPreempt))
            {
                loadRegImm.dwData = MHW_RENDER_ENGINE_THREAD_GROUP_PREEMPT_VALUE;
            }
            else if (MEDIA_IS_SKU(state->skuTable, FtrMediaMidBatchPreempt))
            {
                loadRegImm.dwData = MHW_RENDER_ENGINE_MID_BATCH_PREEMPT_VALUE;
            }
            else
            {
                // if hit this branch then platform does not support any media preemption in render engine. Still program the register to avoid GPU hang
                loadRegImm.dwData = MHW_RENDER_ENGINE_MID_BATCH_PREEMPT_VALUE;
            }
        }
        CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiLoadRegisterImmCmd(&mosCmdBuffer, &loadRegImm));
    }

    // Send Pipeline Select command
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwRender->AddPipelineSelectCmd(&mosCmdBuffer, enableGpGpu));

    // Send State Base Address command
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSendStateBaseAddress(renderHal, &mosCmdBuffer));

    // Send Surface States
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSendSurfaces(renderHal, &mosCmdBuffer));

    if (enableGpGpu) {
        if (csrEnable) {

            // Send CS_STALL pipe control
            //Insert a pipe control as synchronization
            pipeCtlParams = g_cRenderHal_InitPipeControlParams;
            pipeCtlParams.presDest = &state->renderTimeStampResource.osResource;
            pipeCtlParams.dwPostSyncOp = MHW_FLUSH_NOWRITE;
            pipeCtlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
            pipeCtlParams.bDisableCSStall = 0;
            CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(&mosCmdBuffer, nullptr, &pipeCtlParams));
        }

        if (sipEnable || csrEnable)
        {
            // Send SIP State
            CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSendSipStateCmd(renderHal, &mosCmdBuffer));

            CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnRegisterResource(
                osInterface,
                &state->csrResource,
                true,
                true));

            // Send csr base addr command
            CM_CHK_MOSSTATUS_GOTOFINISH(mhwRender->AddGpgpuCsrBaseAddrCmd(&mosCmdBuffer, &state->csrResource));
        }
    }

    // Setup VFE State params. Each Renderer MUST call pfnSetVfeStateParams().
    // See comment in VpHal_HwSetVfeStateParams() for details.
    tmp = RENDERHAL_USE_MEDIA_THREADS_MAX;
    if (state->maxHWThreadValues.userFeatureValue != 0)
    {
        if( state->maxHWThreadValues.userFeatureValue < renderHal->pHwCaps->dwMaxThreads)
        {
            tmp = state->maxHWThreadValues.userFeatureValue;
        }
    }
    else if (state->maxHWThreadValues.apiValue != 0)
    {
        if( state->maxHWThreadValues.apiValue < renderHal->pHwCaps->dwMaxThreads)
        {
            tmp = state->maxHWThreadValues.apiValue;
        }
    }

    renderHal->pfnSetVfeStateParams(
        renderHal,
        MEDIASTATE_DEBUG_COUNTER_FREE_RUNNING,
        tmp,
        state->taskParam->vfeCurbeSize,
        state->taskParam->urbEntrySize,
        &state->scoreboardParams);

    // Send VFE State
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwRender->AddMediaVfeCmd(&mosCmdBuffer,
                     renderHal->pRenderHalPltInterface->GetVfeStateParameters()));

    // Send CURBE Load
    if (state->taskParam->vfeCurbeSize > 0)
    {
        CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSendCurbeLoad(renderHal, &mosCmdBuffer));
    }

    // Send Interface Descriptor Load
    if (state->dshEnabled)
    {
        PRENDERHAL_DYNAMIC_STATE dynamicState = stateHeap->pCurMediaState->pDynamicState;
        idLoadParams.dwInterfaceDescriptorStartOffset = dynamicState->memoryBlock.GetOffset() +
                                                        dynamicState->MediaID.dwOffset;
        idLoadParams.dwInterfaceDescriptorLength      = dynamicState->MediaID.iCount * stateHeap->dwSizeMediaID;
    }
    else
    {
        idLoadParams.dwInterfaceDescriptorStartOffset = stateHeap->pCurMediaState->dwOffset + stateHeap->dwOffsetMediaID;
        idLoadParams.dwInterfaceDescriptorLength      = renderHal->StateHeapSettings.iMediaIDs * stateHeap->dwSizeMediaID;
    }
    idLoadParams.pKernelState = nullptr;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwRender->AddMediaIDLoadCmd(&mosCmdBuffer, &idLoadParams));

    if (enableWalker)
    {
        // send media walker command, if required

        for (uint32_t i = 0; i < state->taskParam->numKernels; i ++)
        {
            // Insert CONDITIONAL_BATCH_BUFFER_END
            if ( taskParam->conditionalEndBitmap & ((uint64_t)1 << (i)) )
            {
                // this could be batch buffer end so need to update sync tag, media state flush, write end timestamp

                CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSendSyncTag(renderHal, &mosCmdBuffer));

                // Insert a pipe control for synchronization since this Conditional Batch Buffer End command
                // will use value written by previous kernel. Also needed since this may be the Batch Buffer End
                pipeCtlParams = g_cRenderHal_InitPipeControlParams;
                pipeCtlParams.presDest = &state->renderTimeStampResource.osResource;
                pipeCtlParams.dwPostSyncOp = MHW_FLUSH_NOWRITE;
                pipeCtlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
                CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(&mosCmdBuffer, nullptr, &pipeCtlParams));

                // issue a PIPE_CONTROL to write timestamp
                pipeCtlParams = g_cRenderHal_InitPipeControlParams;
                pipeCtlParams.presDest = &state->renderTimeStampResource.osResource;
                pipeCtlParams.dwResourceOffset = syncOffset + sizeof(uint64_t);
                pipeCtlParams.dwPostSyncOp = MHW_FLUSH_WRITE_TIMESTAMP_REG;
                pipeCtlParams.dwFlushMode = MHW_FLUSH_READ_CACHE;
                CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(&mosCmdBuffer, nullptr, &pipeCtlParams));

                // Insert conditional batch buffer end
                mhwMiInterface->AddMiConditionalBatchBufferEndCmd(&mosCmdBuffer, &taskParam->conditionalBBEndParams[i]);
            }

            //Insert PIPE_CONTROL at two cases:
            // 1. synchronization is set
            // 2. the next kernel has dependency pattern
            if((i > 0) && ((taskParam->syncBitmap & ((uint64_t)1 << (i-1))) ||
                (kernelParam[i]->kernelThreadSpaceParam.patternType != CM_NONE_DEPENDENCY)))
            {
                //Insert a pipe control as synchronization
                pipeCtlParams = g_cRenderHal_InitPipeControlParams;
                pipeCtlParams.presDest = &state->renderTimeStampResource.osResource;
                pipeCtlParams.dwPostSyncOp = MHW_FLUSH_NOWRITE;
                pipeCtlParams.dwFlushMode = MHW_FLUSH_CUSTOM;
                pipeCtlParams.bInvalidateTextureCache = true;
                pipeCtlParams.bFlushRenderTargetCache = true;
                CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(&mosCmdBuffer, nullptr, &pipeCtlParams));
            }

            // send media walker command, if required
            CM_CHK_MOSSTATUS_GOTOFINISH(state->pfnSendMediaWalkerState(state, kernelParam[i], &mosCmdBuffer));
        }

    }
    else if (enableGpGpu)
    {
        // send GPGPU walker command, if required

        for (uint32_t i = 0; i < state->taskParam->numKernels; i ++)
        {
            MHW_GPGPU_WALKER_PARAMS gpGpuWalkerParams;

            gpGpuWalkerParams.InterfaceDescriptorOffset = kernelParam[i]->gpgpuWalkerParams.interfaceDescriptorOffset;
            gpGpuWalkerParams.GpGpuEnable               = kernelParam[i]->gpgpuWalkerParams.gpgpuEnabled;
            gpGpuWalkerParams.GroupWidth                = kernelParam[i]->gpgpuWalkerParams.groupWidth;
            gpGpuWalkerParams.GroupHeight               = kernelParam[i]->gpgpuWalkerParams.groupHeight;
            gpGpuWalkerParams.GroupDepth                = kernelParam[i]->gpgpuWalkerParams.groupDepth;
            gpGpuWalkerParams.ThreadWidth               = kernelParam[i]->gpgpuWalkerParams.threadWidth;
            gpGpuWalkerParams.ThreadHeight              = kernelParam[i]->gpgpuWalkerParams.threadHeight;
            gpGpuWalkerParams.ThreadDepth               = kernelParam[i]->gpgpuWalkerParams.threadDepth;
            gpGpuWalkerParams.SLMSize                   = kernelParam[i]->slmSize;
            //Insert PIPE_CONTROL at two cases:
            // 1. synchronization is set
            // 2. the next kernel has dependency pattern
            if((i > 0) && ((taskParam->syncBitmap & ((uint64_t)1 << (i-1))) ||
                (kernelParam[i]->kernelThreadSpaceParam.patternType != CM_NONE_DEPENDENCY)))
            {
                //Insert a pipe control as synchronization
                pipeCtlParams = g_cRenderHal_InitPipeControlParams;
                pipeCtlParams.presDest = &state->renderTimeStampResource.osResource;
                pipeCtlParams.dwPostSyncOp = MHW_FLUSH_NOWRITE;
                pipeCtlParams.dwFlushMode = MHW_FLUSH_CUSTOM;
                pipeCtlParams.bInvalidateTextureCache = true;
                pipeCtlParams.bFlushRenderTargetCache = true;
                CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(&mosCmdBuffer, nullptr, &pipeCtlParams));
            }

        // send media walker command, if required
         MOS_SecureMemcpy(&state->walkerParams, sizeof(MHW_WALKER_PARAMS), &kernelParam[i]->walkerParams, sizeof(CM_HAL_WALKER_PARAMS));
         CM_CHK_MOSSTATUS_GOTOFINISH(mhwRender->AddGpGpuWalkerStateCmd(&mosCmdBuffer, &gpGpuWalkerParams));
        }

    }
    else
    {
        // Send Start batch buffer command
        CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiBatchBufferStartCmd(
            &mosCmdBuffer,
            batchBuffer));

        CM_CHK_NULL_GOTOFINISH_MOSERROR(batchBuffer->pPrivateData);
        bbCmArgs = (PCM_HAL_BB_ARGS) batchBuffer->pPrivateData;

        if ( (bbCmArgs->refCount == 1) ||
                 (state->taskParam->reuseBBUpdateMask == 1) )
        {
            CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiBatchBufferEnd(nullptr, batchBuffer));
        }
        else
        {
            // Skip BB end command
            CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->SkipMiBatchBufferEndBb(batchBuffer));
        }

        // UnLock the batch buffer
        if ( (bbCmArgs->refCount == 1) ||
             (state->taskParam->reuseBBUpdateMask == 1) )
        {
            CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnUnlockBB(renderHal, batchBuffer));
        }
    }

    // issue a PIPE_CONTROL to flush all caches and the stall the CS before
    // issuing a PIPE_CONTROL to write the timestamp
    pipeCtlParams = g_cRenderHal_InitPipeControlParams;
    pipeCtlParams.presDest      = &state->renderTimeStampResource.osResource;
    pipeCtlParams.dwPostSyncOp  = MHW_FLUSH_NOWRITE;
    pipeCtlParams.dwFlushMode   = MHW_FLUSH_WRITE_CACHE;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(&mosCmdBuffer, nullptr, &pipeCtlParams));

    if (state->svmBufferUsed)
    {
        // Find the SVM slot, patch it into this dummy pipe_control
        for (uint32_t i = 0; i < state->cmDeviceParam.maxBufferTableSize; i++)
        {
            //Only register SVM resource here
            if (state->bufferTable[i].address)
            {
                CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnRegisterResource(
                        osInterface,
                        &state->bufferTable[i].osResource,
                        true,
                        false));
            }
        }
    }

    // Send Sync Tag
    CM_CHK_MOSSTATUS_GOTOFINISH( renderHal->pfnSendSyncTag( renderHal, &mosCmdBuffer ) );

    // Update tracker resource
    CM_CHK_MOSSTATUS_GOTOFINISH(state->pfnUpdateTrackerResource(state, &mosCmdBuffer, tag));

    // issue a PIPE_CONTROL to write timestamp
    syncOffset += sizeof(uint64_t);
    pipeCtlParams = g_cRenderHal_InitPipeControlParams;
    pipeCtlParams.presDest          = &state->renderTimeStampResource.osResource;
    pipeCtlParams.dwResourceOffset  = syncOffset;
    pipeCtlParams.dwPostSyncOp      = MHW_FLUSH_WRITE_TIMESTAMP_REG;
    pipeCtlParams.dwFlushMode       = MHW_FLUSH_READ_CACHE;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(&mosCmdBuffer, nullptr, &pipeCtlParams));

    // Record registers by unified media profiler in the end
    if (state->perfProfiler != nullptr)
    {
        CM_CHK_MOSSTATUS_GOTOFINISH(state->perfProfiler->AddPerfCollectEndCmd((void *)state, state->osInterface, mhwMiInterface, &mosCmdBuffer));
    }

    // Add PipeControl to invalidate ISP and MediaState to avoid PageFault issue
    MHW_PIPE_CONTROL_PARAMS pipeControlParams;

    MOS_ZeroMemory(&pipeControlParams, sizeof(pipeControlParams));
    pipeControlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
    pipeControlParams.bGenericMediaStateClear = true;
    pipeControlParams.bIndirectStatePointersDisable = true;
    pipeControlParams.bDisableCSStall = false;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(&mosCmdBuffer, nullptr, &pipeControlParams));

    //Couple to the BB_START , otherwise GPU Hang without it in KMD.
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiBatchBufferEnd(&mosCmdBuffer, nullptr));

    // Return unused command buffer space to OS
    osInterface->pfnReturnCommandBuffer(osInterface, &mosCmdBuffer, 0);

#if MDF_COMMAND_BUFFER_DUMP
    if (state->dumpCommandBuffer)
    {
        state->pfnDumpCommadBuffer(
            state,
            &mosCmdBuffer,
            offsetof(PACKET_SURFACE_STATE, cmdSurfaceState),
            mhw_state_heap_g10_X::RENDER_SURFACE_STATE_CMD::byteSize);
    }
#endif

#if MDF_SURFACE_STATE_DUMP
    if (state->dumpSurfaceState)
    {
        state->pfnDumpSurfaceState(
            state, 
            offsetof(PACKET_SURFACE_STATE, cmdSurfaceState), 
            mhw_state_heap_g10_X::RENDER_SURFACE_STATE_CMD::byteSize);
    }
#endif

    CM_CHK_MOSSTATUS_GOTOFINISH(state->pfnGetGlobalTime(&state->taskTimeStamp->submitTimeInCpu[taskId]));
    CM_CHK_MOSSTATUS_GOTOFINISH(state->pfnGetGpuTime(state, &state->taskTimeStamp->submitTimeInGpu[taskId]));

    // Submit command buffer
    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnSubmitCommandBuffer(osInterface,
        &mosCmdBuffer,
        state->nullHwRenderCm));

    if (state->nullHwRenderCm == false)
    {
        stateHeap->pCurMediaState->bBusy = true;
        if ( !enableWalker && !enableGpGpu )
        {
            batchBuffer->bBusy     = true;
            batchBuffer->dwSyncTag = syncTag;
        }
    }

    // reset API call number of HW threads
    state->maxHWThreadValues.apiValue = 0;

    state->pfnReferenceCommandBuffer(&mosCmdBuffer.OsResource, cmdBuffer);

    eStatus = MOS_STATUS_SUCCESS;

finish:
    // Failed -> discard all changes in Command Buffer
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        // Buffer overflow - display overflow size
        if (mosCmdBuffer.iRemaining < 0)
        {
            CM_ASSERTMESSAGE("Command Buffer overflow by %d bytes.", -mosCmdBuffer.iRemaining);
        }

        // Move command buffer back to beginning
        tmp = remaining - mosCmdBuffer.iRemaining;
        mosCmdBuffer.iRemaining  = remaining;
        mosCmdBuffer.iOffset    -= tmp;
        mosCmdBuffer.pCmdPtr     = mosCmdBuffer.pCmdBase + mosCmdBuffer.iOffset/sizeof(uint32_t);

        // Return unused command buffer space to OS
        osInterface->pfnReturnCommandBuffer(osInterface, &mosCmdBuffer, 0);
    }

    return eStatus;
}

MOS_STATUS CM_HAL_G10_X::HwSetSurfaceMemoryObjectControl(
    uint16_t                        memObjCtl,
    PRENDERHAL_SURFACE_STATE_PARAMS surfStateParams)
{
    PRENDERHAL_INTERFACE renderHal = m_cmState->renderHal;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MOS_HW_RESOURCE_DEF mosUsage;
    // The memory object control uint16_t is composed with cache type(8:15), memory type(4:7), ages(0:3)
    mosUsage = (MOS_HW_RESOURCE_DEF)((memObjCtl & CM_MEMOBJCTL_CACHE_MASK) >> 8);
    if (mosUsage >= MOS_HW_RESOURCE_DEF_MAX)
        mosUsage = GetDefaultMOCS();

    surfStateParams->MemObjCtl = renderHal->pOsInterface->pfnCachePolicyGetMemoryObject(mosUsage,
        renderHal->pOsInterface->pfnGetGmmClientContext(renderHal->pOsInterface)).DwordValue;

    return eStatus;
}


MOS_STATUS CM_HAL_G10_X::UpdatePlatformInfoFromPower(
    PCM_PLATFORM_INFO      platformInfo,
    bool                   euSaturated)
{
    UNUSED(platformInfo);
    UNUSED(euSaturated);
    // needs to be implemented
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CM_HAL_G10_X::AllocateSIPCSRResource()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    if (Mos_ResourceIsNull(&m_cmState->sipResource.osResource))
    {
        // create  sip resource if it does not exist
        CM_CHK_MOSSTATUS_RETURN(HalCm_AllocateSipResource(m_cmState));
        CM_CHK_MOSSTATUS_RETURN(HalCm_AllocateCSRResource(m_cmState));
    }
    return eStatus;
}

MOS_STATUS CM_HAL_G10_X::RegisterSampler8x8AVSTable(
   PCM_HAL_SAMPLER_8X8_TABLE  sampler8x8AvsTable,
   PCM_AVS_TABLE_STATE_PARAMS avsTable)
{
    MOS_ZeroMemory(&sampler8x8AvsTable->mhwSamplerAvsTableParam, sizeof(sampler8x8AvsTable->mhwSamplerAvsTableParam));

    sampler8x8AvsTable->mhwSamplerAvsTableParam.byteTransitionArea8Pixels = MEDIASTATE_AVS_TRANSITION_AREA_8_PIXELS;
    sampler8x8AvsTable->mhwSamplerAvsTableParam.byteTransitionArea4Pixels = MEDIASTATE_AVS_TRANSITION_AREA_4_PIXELS;
    sampler8x8AvsTable->mhwSamplerAvsTableParam.byteMaxDerivative8Pixels = MEDIASTATE_AVS_MAX_DERIVATIVE_8_PIXELS;
    sampler8x8AvsTable->mhwSamplerAvsTableParam.byteMaxDerivative4Pixels = MEDIASTATE_AVS_MAX_DERIVATIVE_4_PIXELS;

    sampler8x8AvsTable->mhwSamplerAvsTableParam.bEnableRGBAdaptive = avsTable->enableRgbAdaptive;
    sampler8x8AvsTable->mhwSamplerAvsTableParam.bAdaptiveFilterAllChannels = avsTable->adaptiveFilterAllChannels;

    // Assign the coefficient table;
    for (uint32_t i = 0; i < CM_NUM_HW_POLYPHASE_TABLES_G10; i++)
    {
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].ZeroXFilterCoefficient[0] = (uint8_t)avsTable->tbl0X[i].FilterCoeff_0_0;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].ZeroXFilterCoefficient[1] = (uint8_t)avsTable->tbl0X[i].FilterCoeff_0_1;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].ZeroXFilterCoefficient[2] = (uint8_t)avsTable->tbl0X[i].FilterCoeff_0_2;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].ZeroXFilterCoefficient[3] = (uint8_t)avsTable->tbl0X[i].FilterCoeff_0_3;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].ZeroXFilterCoefficient[4] = (uint8_t)avsTable->tbl0X[i].FilterCoeff_0_4;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].ZeroXFilterCoefficient[5] = (uint8_t)avsTable->tbl0X[i].FilterCoeff_0_5;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].ZeroXFilterCoefficient[6] = (uint8_t)avsTable->tbl0X[i].FilterCoeff_0_6;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].ZeroXFilterCoefficient[7] = (uint8_t)avsTable->tbl0X[i].FilterCoeff_0_7;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].ZeroYFilterCoefficient[0] = (uint8_t)avsTable->tbl0Y[i].FilterCoeff_0_0;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].ZeroYFilterCoefficient[1] = (uint8_t)avsTable->tbl0Y[i].FilterCoeff_0_1;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].ZeroYFilterCoefficient[2] = (uint8_t)avsTable->tbl0Y[i].FilterCoeff_0_2;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].ZeroYFilterCoefficient[3] = (uint8_t)avsTable->tbl0Y[i].FilterCoeff_0_3;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].ZeroYFilterCoefficient[4] = (uint8_t)avsTable->tbl0Y[i].FilterCoeff_0_4;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].ZeroYFilterCoefficient[5] = (uint8_t)avsTable->tbl0Y[i].FilterCoeff_0_5;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].ZeroYFilterCoefficient[6] = (uint8_t)avsTable->tbl0Y[i].FilterCoeff_0_6;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].ZeroYFilterCoefficient[7] = (uint8_t)avsTable->tbl0Y[i].FilterCoeff_0_7;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].OneXFilterCoefficient[0] = (uint8_t)avsTable->tbl1X[i].FilterCoeff_0_2;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].OneXFilterCoefficient[1] = (uint8_t)avsTable->tbl1X[i].FilterCoeff_0_3;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].OneXFilterCoefficient[2] = (uint8_t)avsTable->tbl1X[i].FilterCoeff_0_4;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].OneXFilterCoefficient[3] = (uint8_t)avsTable->tbl1X[i].FilterCoeff_0_5;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].OneYFilterCoefficient[0] = (uint8_t)avsTable->tbl1Y[i].FilterCoeff_0_2;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].OneYFilterCoefficient[1] = (uint8_t)avsTable->tbl1Y[i].FilterCoeff_0_3;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].OneYFilterCoefficient[2] = (uint8_t)avsTable->tbl1Y[i].FilterCoeff_0_4;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParam[i].OneYFilterCoefficient[3] = (uint8_t)avsTable->tbl1Y[i].FilterCoeff_0_5;
    }

    sampler8x8AvsTable->mhwSamplerAvsTableParam.byteDefaultSharpnessLevel = avsTable->defaultSharpLevel;
    sampler8x8AvsTable->mhwSamplerAvsTableParam.bBypassXAdaptiveFiltering = avsTable->bypassXAF;
    sampler8x8AvsTable->mhwSamplerAvsTableParam.bBypassYAdaptiveFiltering = avsTable->bypassYAF;

    if (!avsTable->bypassXAF && !avsTable->bypassYAF)
    {
        sampler8x8AvsTable->mhwSamplerAvsTableParam.byteMaxDerivative8Pixels = avsTable->maxDerivative8Pixels;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.byteMaxDerivative4Pixels = avsTable->maxDerivative4Pixels;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.byteTransitionArea8Pixels = avsTable->transitionArea8Pixels;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.byteTransitionArea4Pixels = avsTable->transitionArea4Pixels;
    }

    for (int i = 0; i < CM_NUM_HW_POLYPHASE_EXTRA_TABLES_G10; i++)
    {
        int src = i + CM_NUM_HW_POLYPHASE_TABLES_G10;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].ZeroXFilterCoefficient[0] = (uint8_t)avsTable->tbl0X[src].FilterCoeff_0_0;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].ZeroXFilterCoefficient[1] = (uint8_t)avsTable->tbl0X[src].FilterCoeff_0_1;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].ZeroXFilterCoefficient[2] = (uint8_t)avsTable->tbl0X[src].FilterCoeff_0_2;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].ZeroXFilterCoefficient[3] = (uint8_t)avsTable->tbl0X[src].FilterCoeff_0_3;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].ZeroXFilterCoefficient[4] = (uint8_t)avsTable->tbl0X[src].FilterCoeff_0_4;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].ZeroXFilterCoefficient[5] = (uint8_t)avsTable->tbl0X[src].FilterCoeff_0_5;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].ZeroXFilterCoefficient[6] = (uint8_t)avsTable->tbl0X[src].FilterCoeff_0_6;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].ZeroXFilterCoefficient[7] = (uint8_t)avsTable->tbl0X[src].FilterCoeff_0_7;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].ZeroYFilterCoefficient[0] = (uint8_t)avsTable->tbl0Y[src].FilterCoeff_0_0;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].ZeroYFilterCoefficient[1] = (uint8_t)avsTable->tbl0Y[src].FilterCoeff_0_1;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].ZeroYFilterCoefficient[2] = (uint8_t)avsTable->tbl0Y[src].FilterCoeff_0_2;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].ZeroYFilterCoefficient[3] = (uint8_t)avsTable->tbl0Y[src].FilterCoeff_0_3;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].ZeroYFilterCoefficient[4] = (uint8_t)avsTable->tbl0Y[src].FilterCoeff_0_4;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].ZeroYFilterCoefficient[5] = (uint8_t)avsTable->tbl0Y[src].FilterCoeff_0_5;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].ZeroYFilterCoefficient[6] = (uint8_t)avsTable->tbl0Y[src].FilterCoeff_0_6;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].ZeroYFilterCoefficient[7] = (uint8_t)avsTable->tbl0Y[src].FilterCoeff_0_7;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].OneXFilterCoefficient[0] = (uint8_t)avsTable->tbl1X[src].FilterCoeff_0_2;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].OneXFilterCoefficient[1] = (uint8_t)avsTable->tbl1X[src].FilterCoeff_0_3;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].OneXFilterCoefficient[2] = (uint8_t)avsTable->tbl1X[src].FilterCoeff_0_4;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].OneXFilterCoefficient[3] = (uint8_t)avsTable->tbl1X[src].FilterCoeff_0_5;

        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].OneYFilterCoefficient[0] = (uint8_t)avsTable->tbl1Y[src].FilterCoeff_0_2;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].OneYFilterCoefficient[1] = (uint8_t)avsTable->tbl1Y[src].FilterCoeff_0_3;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].OneYFilterCoefficient[2] = (uint8_t)avsTable->tbl1Y[src].FilterCoeff_0_4;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.paMhwAvsCoeffParamExtra[i].OneYFilterCoefficient[3] = (uint8_t)avsTable->tbl1Y[src].FilterCoeff_0_5;

    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CM_HAL_G10_X::RegisterSampler8x8(
    PCM_HAL_SAMPLER_8X8_PARAM    param)
{
    PCM_HAL_STATE               state = m_cmState;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    int16_t                     samplerIndex = 0;
    PMHW_SAMPLER_STATE_PARAM    samplerEntry = nullptr;
    PCM_HAL_SAMPLER_8X8_ENTRY   sampler8x8Entry = nullptr;

    if (param->sampler8x8State.stateType == CM_SAMPLER8X8_AVS)
    {
        for (uint32_t i = 0; i < state->cmDeviceParam.maxSamplerTableSize; i++) {
            if (!state->samplerTable[i].bInUse) {
                samplerEntry = &state->samplerTable[i];
                param->handle = (uint32_t)i << 16;
                samplerEntry->bInUse = true;
                break;
            }
        }

        for (uint32_t i = 0; i < state->cmDeviceParam.maxSampler8x8TableSize; i++) {
            if (!state->sampler8x8Table[i].inUse) {
                sampler8x8Entry = &state->sampler8x8Table[i];
                samplerIndex = (int16_t)i;
                param->handle |= (uint32_t)(i & 0xffff);
                sampler8x8Entry->inUse = true;
                break;
            }
        }

        if (!samplerEntry || !sampler8x8Entry) {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            CM_ASSERTMESSAGE("Sampler or AVS table is full");
            goto finish;
        }

        //State data from application
        samplerEntry->SamplerType = MHW_SAMPLER_TYPE_AVS;
        samplerEntry->ElementType = MHW_Sampler128Elements;
        samplerEntry->Avs = param->sampler8x8State.avsParam.avsState;
        samplerEntry->Avs.stateID = samplerIndex;
        samplerEntry->Avs.iTable8x8_Index = samplerIndex;  // Used for calculating the Media offset of 8x8 table
        samplerEntry->Avs.pMhwSamplerAvsTableParam = &sampler8x8Entry->sampler8x8State.mhwSamplerAvsTableParam;

        if (samplerEntry->Avs.EightTapAFEnable)
            param->sampler8x8State.avsParam.avsTable.adaptiveFilterAllChannels = true;
        else
            param->sampler8x8State.avsParam.avsTable.adaptiveFilterAllChannels = false;

        CM_CHK_MOSSTATUS_GOTOFINISH(RegisterSampler8x8AVSTable(&sampler8x8Entry->sampler8x8State,
            &param->sampler8x8State.avsParam.avsTable));

        sampler8x8Entry->sampler8x8State.stateType = CM_SAMPLER8X8_AVS;
    }
    else if (param->sampler8x8State.stateType == CM_SAMPLER8X8_MISC)
    {
        for (uint32_t i = 0; i < state->cmDeviceParam.maxSamplerTableSize; i++)
        {
            if (!state->samplerTable[i].bInUse)
            {
                samplerEntry = &state->samplerTable[i];
                param->handle = (uint32_t)i << 16;
                samplerEntry->bInUse = true;
                break;
            }
        }

        if (samplerEntry == nullptr)
        {
            return MOS_STATUS_INVALID_HANDLE;
        }
        samplerEntry->SamplerType = MHW_SAMPLER_TYPE_MISC;
        samplerEntry->ElementType = MHW_Sampler2Elements;

        samplerEntry->Misc.byteHeight = param->sampler8x8State.miscState.DW0.Height;
        samplerEntry->Misc.byteWidth = param->sampler8x8State.miscState.DW0.Width;
        samplerEntry->Misc.wRow[0] = param->sampler8x8State.miscState.DW0.Row0;
        samplerEntry->Misc.wRow[1] = param->sampler8x8State.miscState.DW1.Row1;
        samplerEntry->Misc.wRow[2] = param->sampler8x8State.miscState.DW1.Row2;
        samplerEntry->Misc.wRow[3] = param->sampler8x8State.miscState.DW2.Row3;
        samplerEntry->Misc.wRow[4] = param->sampler8x8State.miscState.DW2.Row4;
        samplerEntry->Misc.wRow[5] = param->sampler8x8State.miscState.DW3.Row5;
        samplerEntry->Misc.wRow[6] = param->sampler8x8State.miscState.DW3.Row6;
        samplerEntry->Misc.wRow[7] = param->sampler8x8State.miscState.DW4.Row7;
        samplerEntry->Misc.wRow[8] = param->sampler8x8State.miscState.DW4.Row8;
        samplerEntry->Misc.wRow[9] = param->sampler8x8State.miscState.DW5.Row9;
        samplerEntry->Misc.wRow[10] = param->sampler8x8State.miscState.DW5.Row10;
        samplerEntry->Misc.wRow[11] = param->sampler8x8State.miscState.DW6.Row11;
        samplerEntry->Misc.wRow[12] = param->sampler8x8State.miscState.DW6.Row12;
        samplerEntry->Misc.wRow[13] = param->sampler8x8State.miscState.DW7.Row13;
        samplerEntry->Misc.wRow[14] = param->sampler8x8State.miscState.DW7.Row14;
    }
    else if (param->sampler8x8State.stateType == CM_SAMPLER8X8_CONV)
    {
        for (uint32_t i = 0; i < state->cmDeviceParam.maxSamplerTableSize; i++)
        {
            if (!state->samplerTable[i].bInUse) {
                samplerEntry = &state->samplerTable[i];
                param->handle = (uint32_t)i << 16;
                samplerEntry->bInUse = true;
                break;
            }
        }


        if (samplerEntry == nullptr)
        {
            return MOS_STATUS_INVALID_HANDLE;
        }

        MOS_ZeroMemory(&samplerEntry->Convolve, sizeof(samplerEntry->Convolve));

        samplerEntry->SamplerType = MHW_SAMPLER_TYPE_CONV;

        samplerEntry->Convolve.ui8Height = param->sampler8x8State.convolveState.height;
        samplerEntry->Convolve.ui8Width = param->sampler8x8State.convolveState.width;
        samplerEntry->Convolve.ui8ScaledDownValue = param->sampler8x8State.convolveState.scaleDownValue;
        samplerEntry->Convolve.ui8SizeOfTheCoefficient = param->sampler8x8State.convolveState.coeffSize;

        samplerEntry->Convolve.ui8MSBWidth = param->sampler8x8State.convolveState.isHorizontal32Mode;
        samplerEntry->Convolve.ui8MSBHeight = param->sampler8x8State.convolveState.isVertical32Mode;
        samplerEntry->Convolve.skl_mode = param->sampler8x8State.convolveState.sklMode;

        // Currently use DW0.Reserved0 to save the detailed Convolve Type, the DW0.Reserved0 will be cleared when copy to sampelr heap
        samplerEntry->Convolve.ui8ConvolveType = param->sampler8x8State.convolveState.nConvolveType;
        if (samplerEntry->Convolve.skl_mode &&
            samplerEntry->Convolve.ui8ConvolveType == CM_CONVOLVE_SKL_TYPE_2D)
        {
            samplerEntry->ElementType = MHW_Sampler128Elements;
        }
        else if ((!samplerEntry->Convolve.skl_mode &&
            samplerEntry->Convolve.ui8ConvolveType == CM_CONVOLVE_SKL_TYPE_2D)
            || samplerEntry->Convolve.ui8ConvolveType == CM_CONVOLVE_SKL_TYPE_1P)
        {
            samplerEntry->ElementType = MHW_Sampler64Elements;
        }
        else
        {
            samplerEntry->ElementType = MHW_Sampler8Elements;
        }

        for (int i = 0; i < CM_NUM_CONVOLVE_ROWS_SKL; i++)
        {
            MHW_SAMPLER_CONVOLVE_COEFF_TABLE *coeffTable = &(samplerEntry->Convolve.CoeffTable[i]);
            CM_HAL_CONVOLVE_COEFF_TABLE      *sourceTable = &(param->sampler8x8State.convolveState.table[i]);
            if (samplerEntry->Convolve.ui8SizeOfTheCoefficient == 1)
            {
                coeffTable->wFilterCoeff[0] = FloatToS3_12(sourceTable->FilterCoeff_0_0);
                coeffTable->wFilterCoeff[1] = FloatToS3_12(sourceTable->FilterCoeff_0_1);
                coeffTable->wFilterCoeff[2] = FloatToS3_12(sourceTable->FilterCoeff_0_2);
                coeffTable->wFilterCoeff[3] = FloatToS3_12(sourceTable->FilterCoeff_0_3);
                coeffTable->wFilterCoeff[4] = FloatToS3_12(sourceTable->FilterCoeff_0_4);
                coeffTable->wFilterCoeff[5] = FloatToS3_12(sourceTable->FilterCoeff_0_5);
                coeffTable->wFilterCoeff[6] = FloatToS3_12(sourceTable->FilterCoeff_0_6);
                coeffTable->wFilterCoeff[7] = FloatToS3_12(sourceTable->FilterCoeff_0_7);
                coeffTable->wFilterCoeff[8] = FloatToS3_12(sourceTable->FilterCoeff_0_8);
                coeffTable->wFilterCoeff[9] = FloatToS3_12(sourceTable->FilterCoeff_0_9);
                coeffTable->wFilterCoeff[10] = FloatToS3_12(sourceTable->FilterCoeff_0_10);
                coeffTable->wFilterCoeff[11] = FloatToS3_12(sourceTable->FilterCoeff_0_11);
                coeffTable->wFilterCoeff[12] = FloatToS3_12(sourceTable->FilterCoeff_0_12);
                coeffTable->wFilterCoeff[13] = FloatToS3_12(sourceTable->FilterCoeff_0_13);
                coeffTable->wFilterCoeff[14] = FloatToS3_12(sourceTable->FilterCoeff_0_14);
                coeffTable->wFilterCoeff[15] = FloatToS3_12(sourceTable->FilterCoeff_0_15);
            }
            else
            {
                coeffTable->wFilterCoeff[0] = FloatToS3_4(sourceTable->FilterCoeff_0_0);
                coeffTable->wFilterCoeff[1] = FloatToS3_4(sourceTable->FilterCoeff_0_1);
                coeffTable->wFilterCoeff[2] = FloatToS3_4(sourceTable->FilterCoeff_0_2);
                coeffTable->wFilterCoeff[3] = FloatToS3_4(sourceTable->FilterCoeff_0_3);
                coeffTable->wFilterCoeff[4] = FloatToS3_4(sourceTable->FilterCoeff_0_4);
                coeffTable->wFilterCoeff[5] = FloatToS3_4(sourceTable->FilterCoeff_0_5);
                coeffTable->wFilterCoeff[6] = FloatToS3_4(sourceTable->FilterCoeff_0_6);
                coeffTable->wFilterCoeff[7] = FloatToS3_4(sourceTable->FilterCoeff_0_7);
                coeffTable->wFilterCoeff[8] = FloatToS3_4(sourceTable->FilterCoeff_0_8);
                coeffTable->wFilterCoeff[9] = FloatToS3_4(sourceTable->FilterCoeff_0_9);
                coeffTable->wFilterCoeff[10] = FloatToS3_4(sourceTable->FilterCoeff_0_10);
                coeffTable->wFilterCoeff[11] = FloatToS3_4(sourceTable->FilterCoeff_0_11);
                coeffTable->wFilterCoeff[12] = FloatToS3_4(sourceTable->FilterCoeff_0_12);
                coeffTable->wFilterCoeff[13] = FloatToS3_4(sourceTable->FilterCoeff_0_13);
                coeffTable->wFilterCoeff[14] = FloatToS3_4(sourceTable->FilterCoeff_0_14);
                coeffTable->wFilterCoeff[15] = FloatToS3_4(sourceTable->FilterCoeff_0_15);
            }
        }

        for (int i = CM_NUM_CONVOLVE_ROWS_SKL; i < CM_NUM_CONVOLVE_ROWS_SKL * 2; i++)
        {
            MHW_SAMPLER_CONVOLVE_COEFF_TABLE *coeffTable = &(samplerEntry->Convolve.CoeffTable[i]);
            CM_HAL_CONVOLVE_COEFF_TABLE      *sourceTable = &(param->sampler8x8State.convolveState.table[i - CM_NUM_CONVOLVE_ROWS_SKL]);

            if (samplerEntry->Convolve.ui8SizeOfTheCoefficient == 1)
            {
                coeffTable->wFilterCoeff[0] = FloatToS3_12(sourceTable->FilterCoeff_0_16);
                coeffTable->wFilterCoeff[1] = FloatToS3_12(sourceTable->FilterCoeff_0_17);
                coeffTable->wFilterCoeff[2] = FloatToS3_12(sourceTable->FilterCoeff_0_18);
                coeffTable->wFilterCoeff[3] = FloatToS3_12(sourceTable->FilterCoeff_0_19);
                coeffTable->wFilterCoeff[4] = FloatToS3_12(sourceTable->FilterCoeff_0_20);
                coeffTable->wFilterCoeff[5] = FloatToS3_12(sourceTable->FilterCoeff_0_21);
                coeffTable->wFilterCoeff[6] = FloatToS3_12(sourceTable->FilterCoeff_0_22);
                coeffTable->wFilterCoeff[7] = FloatToS3_12(sourceTable->FilterCoeff_0_23);
                coeffTable->wFilterCoeff[8] = FloatToS3_12(sourceTable->FilterCoeff_0_24);
                coeffTable->wFilterCoeff[9] = FloatToS3_12(sourceTable->FilterCoeff_0_25);
                coeffTable->wFilterCoeff[10] = FloatToS3_12(sourceTable->FilterCoeff_0_26);
                coeffTable->wFilterCoeff[11] = FloatToS3_12(sourceTable->FilterCoeff_0_27);
                coeffTable->wFilterCoeff[12] = FloatToS3_12(sourceTable->FilterCoeff_0_28);
                coeffTable->wFilterCoeff[13] = FloatToS3_12(sourceTable->FilterCoeff_0_29);
                coeffTable->wFilterCoeff[14] = FloatToS3_12(sourceTable->FilterCoeff_0_30);
                coeffTable->wFilterCoeff[15] = FloatToS3_12(sourceTable->FilterCoeff_0_31);
            }
            else
            {
                coeffTable->wFilterCoeff[0] = FloatToS3_4(sourceTable->FilterCoeff_0_16);
                coeffTable->wFilterCoeff[1] = FloatToS3_4(sourceTable->FilterCoeff_0_17);
                coeffTable->wFilterCoeff[2] = FloatToS3_4(sourceTable->FilterCoeff_0_18);
                coeffTable->wFilterCoeff[3] = FloatToS3_4(sourceTable->FilterCoeff_0_19);
                coeffTable->wFilterCoeff[4] = FloatToS3_4(sourceTable->FilterCoeff_0_20);
                coeffTable->wFilterCoeff[5] = FloatToS3_4(sourceTable->FilterCoeff_0_21);
                coeffTable->wFilterCoeff[6] = FloatToS3_4(sourceTable->FilterCoeff_0_22);
                coeffTable->wFilterCoeff[7] = FloatToS3_4(sourceTable->FilterCoeff_0_23);
                coeffTable->wFilterCoeff[8] = FloatToS3_4(sourceTable->FilterCoeff_0_24);
                coeffTable->wFilterCoeff[9] = FloatToS3_4(sourceTable->FilterCoeff_0_25);
                coeffTable->wFilterCoeff[10] = FloatToS3_4(sourceTable->FilterCoeff_0_26);
                coeffTable->wFilterCoeff[11] = FloatToS3_4(sourceTable->FilterCoeff_0_27);
                coeffTable->wFilterCoeff[12] = FloatToS3_4(sourceTable->FilterCoeff_0_28);
                coeffTable->wFilterCoeff[13] = FloatToS3_4(sourceTable->FilterCoeff_0_29);
                coeffTable->wFilterCoeff[14] = FloatToS3_4(sourceTable->FilterCoeff_0_30);
                coeffTable->wFilterCoeff[15] = FloatToS3_4(sourceTable->FilterCoeff_0_31);
            }
        }
    }

finish:
    return eStatus;
}

MOS_STATUS CM_HAL_G10_X::SetupHwDebugControl(
    PRENDERHAL_INTERFACE   renderHal,
    PMOS_COMMAND_BUFFER    cmdBuffer)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_MI_LOAD_REGISTER_IMM_PARAMS loadRegImm;

    //---------------------------------------
    CM_CHK_NULL_RETURN_MOSERROR( renderHal );
    CM_CHK_NULL_RETURN_MOSERROR( renderHal->pMhwMiInterface );
    CM_CHK_NULL_RETURN_MOSERROR( cmdBuffer );
    //---------------------------------------

    MOS_ZeroMemory( &loadRegImm, sizeof( MHW_MI_LOAD_REGISTER_IMM_PARAMS ) );

    // CS_DEBUG_MODE2, global debug enable
    loadRegImm.dwRegister = CS_DEBUG_MODE2;
    loadRegImm.dwData = ( CS_DEBUG_MODE2_GLOBAL_DEBUG << 16 ) | CS_DEBUG_MODE2_GLOBAL_DEBUG;
    CM_CHK_MOSSTATUS_RETURN( renderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd( cmdBuffer, &loadRegImm ) );

    // TD_CTL, force thread breakpoint enable
    // Also enable external exception, because the source-level debugger has to
    // be able to interrupt runing EU threads.
    loadRegImm.dwRegister = TD_CTL;
    loadRegImm.dwData = TD_CTL_FORCE_THREAD_BKPT_ENABLE | TD_CTL_FORCE_EXT_EXCEPTION_ENABLE;
    CM_CHK_MOSSTATUS_RETURN( renderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd( cmdBuffer, &loadRegImm ) );

    return eStatus;
}

MOS_STATUS CM_HAL_G10_X::SetMediaWalkerParams(
    CM_WALKING_PARAMETERS          engineeringParams,
    PCM_HAL_WALKER_PARAMS          walkerParams)
{
    mhw_render_g10_X::MEDIA_OBJECT_WALKER_CMD mediaWalkerCmd;
    mediaWalkerCmd.DW5.Value = engineeringParams.Value[0];
    walkerParams->scoreboardMask = mediaWalkerCmd.DW5.ScoreboardMask;

    mediaWalkerCmd.DW6.Value = engineeringParams.Value[1];
    walkerParams->colorCountMinusOne = mediaWalkerCmd.DW6.ColorCountMinusOne;
    walkerParams->midLoopUnitX = mediaWalkerCmd.DW6.MidLoopUnitX;
    walkerParams->midLoopUnitY = mediaWalkerCmd.DW6.LocalMidLoopUnitY;
    walkerParams->middleLoopExtraSteps = mediaWalkerCmd.DW6.MiddleLoopExtraSteps;

    mediaWalkerCmd.DW7.Value = engineeringParams.Value[2];
    walkerParams->localLoopExecCount = mediaWalkerCmd.DW7.LocalLoopExecCount;
    walkerParams->globalLoopExecCount = mediaWalkerCmd.DW7.GlobalLoopExecCount;

    mediaWalkerCmd.DW8.Value = engineeringParams.Value[3];
    walkerParams->blockResolution.x = mediaWalkerCmd.DW8.BlockResolutionX;
    walkerParams->blockResolution.y = mediaWalkerCmd.DW8.BlockResolutionY;

    mediaWalkerCmd.DW9.Value = engineeringParams.Value[4];
    walkerParams->localStart.x = mediaWalkerCmd.DW9.LocalStartX;
    walkerParams->localStart.y = mediaWalkerCmd.DW9.LocalStartY;

    mediaWalkerCmd.DW11.Value = engineeringParams.Value[6];
    walkerParams->localOutLoopStride.x = mediaWalkerCmd.DW11.LocalOuterLoopStrideX;
    walkerParams->localOutLoopStride.y = mediaWalkerCmd.DW11.LocalOuterLoopStrideY;

    mediaWalkerCmd.DW12.Value = engineeringParams.Value[7];
    walkerParams->localInnerLoopUnit.x = mediaWalkerCmd.DW12.LocalInnerLoopUnitX;
    walkerParams->localInnerLoopUnit.y = mediaWalkerCmd.DW12.LocalInnerLoopUnitY;

    mediaWalkerCmd.DW13.Value = engineeringParams.Value[8];
    walkerParams->globalResolution.x = mediaWalkerCmd.DW13.GlobalResolutionX;
    walkerParams->globalResolution.y = mediaWalkerCmd.DW13.GlobalResolutionY;

    mediaWalkerCmd.DW14.Value = engineeringParams.Value[9];
    walkerParams->globalStart.x = mediaWalkerCmd.DW14.GlobalStartX;
    walkerParams->globalStart.y = mediaWalkerCmd.DW14.GlobalStartY;

    mediaWalkerCmd.DW15.Value = engineeringParams.Value[10];
    walkerParams->globalOutlerLoopStride.x = mediaWalkerCmd.DW15.GlobalOuterLoopStrideX;
    walkerParams->globalOutlerLoopStride.y = mediaWalkerCmd.DW15.GlobalOuterLoopStrideY;

    mediaWalkerCmd.DW16.Value = engineeringParams.Value[11];
    walkerParams->globalInnerLoopUnit.x = mediaWalkerCmd.DW16.GlobalInnerLoopUnitX;
    walkerParams->globalInnerLoopUnit.y = mediaWalkerCmd.DW16.GlobalInnerLoopUnitY;

    walkerParams->localEnd.x = 0;
    walkerParams->localEnd.y = 0;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CM_HAL_G10_X::GetCopyKernelIsa(void  *&isa, uint32_t &isaSize)
{
    isa = (void *)pGPUCopy_kernel_isa_gen10;
    isaSize = iGPUCopy_kernel_isa_size_gen10;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CM_HAL_G10_X::GetInitKernelIsa(void  *&isa, uint32_t &isaSize)
{
    isa = (void *)pGPUInit_kernel_isa_Gen10;
    isaSize = iGPUInit_kernel_isa_size_Gen10;

    return MOS_STATUS_SUCCESS;
}

uint32_t CM_HAL_G10_X::GetMediaWalkerMaxThreadWidth()
{
    return CM_MAX_THREADSPACE_WIDTH_SKLUP_FOR_MW;
}

uint32_t CM_HAL_G10_X::GetMediaWalkerMaxThreadHeight()
{
    return CM_MAX_THREADSPACE_HEIGHT_SKLUP_FOR_MW;
}

MOS_STATUS CM_HAL_G10_X::GetHwSurfaceBTIInfo(
                      PCM_SURFACE_BTI_INFO btiInfo)
{
    if (btiInfo == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    btiInfo->normalSurfaceStart      =  CM_GLOBAL_SURFACE_INDEX_START_GEN9_PLUS + \
                        CM_GLOBAL_SURFACE_NUMBER + CM_GTPIN_SURFACE_NUMBER ;
    btiInfo->normalSurfaceEnd        =  GT_RESERVED_INDEX_START_GEN9_PLUS - 1;
    btiInfo->reservedSurfaceStart    =  CM_GLOBAL_SURFACE_INDEX_START_GEN9_PLUS;
    btiInfo->reservedSurfaceEnd      =  CM_GLOBAL_SURFACE_NUMBER + CM_GTPIN_SURFACE_NUMBER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CM_HAL_G10_X::SetSuggestedL3Conf(
            L3_SUGGEST_CONFIG l3Config)
{
    if (l3Config >= sizeof(CNL_L3_PLANE)/sizeof(L3ConfigRegisterValues))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    return HalCm_SetL3Cache((L3ConfigRegisterValues *)&CNL_L3_PLANE[l3Config],
                                     &m_cmState->l3Settings);
}

MOS_STATUS CM_HAL_G10_X::GetGenStepInfo(char*& stepInfoStr)
{
    const char *cmSteppingInfoCNL[] = {"A0", "B0", "C0"};

    uint32_t genStepId = m_cmState->platform.usRevId;

    uint32_t tablesize = sizeof(cmSteppingInfoCNL) / sizeof(char *);

    if (genStepId < tablesize)
    {
        stepInfoStr = (char *)cmSteppingInfoCNL[genStepId];
    }
    else
    {
        stepInfoStr = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

int32_t CM_HAL_G10_X::ColorCountSanityCheck(uint32_t colorCount)
{
    if (colorCount == CM_INVALID_COLOR_COUNT || colorCount > CM_THREADSPACE_MAX_COLOR_COUNT)
    {
        CM_ASSERTMESSAGE("Error: Invalid color count.");
        return CM_INVALID_ARG_VALUE;
    }
    return CM_SUCCESS;
}

bool CM_HAL_G10_X::MemoryObjectCtrlPolicyCheck(uint32_t memCtrl)
{
    if (memCtrl > MEMORY_OBJECT_CONTROL_SKL_NO_CACHE)
    {
        return false;
    }

    return true;
}

int32_t CM_HAL_G10_X::GetConvSamplerIndex(
    PMHW_SAMPLER_STATE_PARAM  samplerParam,
    char                     *samplerIndexTable,
    int32_t                   nSamp8X8Num,
    int32_t                   nSampConvNum)
{
    int32_t samplerIndex = 0;

    if ((samplerParam->Convolve.ui8ConvolveType == CM_CONVOLVE_SKL_TYPE_2D) &&
        (samplerParam->Convolve.skl_mode))
    {
        // 2D convolve & SKL+
        samplerIndex = 1 + nSampConvNum + nSamp8X8Num;
    }
    else if (samplerParam->Convolve.ui8ConvolveType == CM_CONVOLVE_SKL_TYPE_1D)
    {
        // 1D convolve & SKL+
        samplerIndex = nSampConvNum;
    }
    else
    {
        // 1P convolve SKL+
        samplerIndex = 1 + (nSamp8X8Num + nSampConvNum) * 2;
        while (samplerIndexTable[samplerIndex] != CM_INVALID_INDEX)
        {
            samplerIndex += 2;
        }

    }
    return samplerIndex;
}

MOS_STATUS CM_HAL_G10_X::SetL3CacheConfig(
            const L3ConfigRegisterValues *values,
            PCmHalL3Settings cmHalL3Setting)
{
    return HalCm_SetL3Cache( values, cmHalL3Setting );
}

MOS_STATUS CM_HAL_G10_X::GetSamplerParamInfoForSamplerType(
            PMHW_SAMPLER_STATE_PARAM mhwSamplerParam,
            SamplerParam  &samplerParam)
{
    const unsigned int samplerElementSize[MAX_ELEMENT_TYPE_COUNT] = { 16, 32, 64, 128, 1024, 2048 };

    // gets element_type
    switch (mhwSamplerParam->SamplerType)
    {
    case MHW_SAMPLER_TYPE_3D:
        samplerParam.elementType = MHW_Sampler1Element;
        break;
    case MHW_SAMPLER_TYPE_MISC:
        samplerParam.elementType = MHW_Sampler2Elements;
        break;
    case MHW_SAMPLER_TYPE_CONV:
        if ((!mhwSamplerParam->Convolve.skl_mode &&
            mhwSamplerParam->Convolve.ui8ConvolveType == CM_CONVOLVE_SKL_TYPE_2D)
            || mhwSamplerParam->Convolve.ui8ConvolveType == CM_CONVOLVE_SKL_TYPE_1P)
        {
            samplerParam.elementType = MHW_Sampler64Elements;
        }
        else if (mhwSamplerParam->Convolve.ui8ConvolveType == CM_CONVOLVE_SKL_TYPE_1D)
        {
            samplerParam.elementType = MHW_Sampler8Elements;
        }
        else
        {
            samplerParam.elementType = MHW_Sampler128Elements;
        }
        break;
    case MHW_SAMPLER_TYPE_AVS:
        samplerParam.elementType = MHW_Sampler128Elements;
        break;
    default:
        break;
    }

    // bti_stepping for BDW mode convolve or 1P convolve is 2, other cases are 1.
    if ((mhwSamplerParam->SamplerType == MHW_SAMPLER_TYPE_CONV) && ((!mhwSamplerParam->Convolve.skl_mode &&
        mhwSamplerParam->Convolve.ui8ConvolveType == CM_CONVOLVE_SKL_TYPE_2D)
        || mhwSamplerParam->Convolve.ui8ConvolveType == CM_CONVOLVE_SKL_TYPE_1P))
    {
        samplerParam.btiStepping = 2;
    }
    else
    {
        samplerParam.btiStepping = 1;
    }

    // gets multiplier
    samplerParam.btiMultiplier = samplerElementSize[samplerParam.elementType] / samplerParam.btiStepping;

    // gets size
    samplerParam.size = samplerElementSize[samplerParam.elementType];

    // Temporary solution for conv because MHW use 2048 bytes for all of the convolve samplers.
    // size should always be equal to bti_stepping * bti_multiplier except for this one.
    if (mhwSamplerParam->SamplerType == MHW_SAMPLER_TYPE_CONV)
    {
        samplerParam.size = 2048;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CM_HAL_G10_X::GetExpectedGtSystemConfig(
    PCM_EXPECTED_GT_SYSTEM_INFO expectedConfig)
{
    if (m_genGT == PLATFORM_INTEL_GT1)
    {
        expectedConfig->numSlices    = CNL_GT1_4X8_MAX_NUM_SLICES;
        expectedConfig->numSubSlices = CNL_GT1_4X8_MAX_NUM_SUBSLICES;
    }
    else if (m_genGT == PLATFORM_INTEL_GT2)
    {
        expectedConfig->numSlices    = CNL_GT2_7X8_MAX_NUM_SLICES;
        expectedConfig->numSubSlices = CNL_GT2_7X8_MAX_NUM_SUBSLICES;
    }
    else if (m_genGT == PLATFORM_INTEL_GT3)
    {
        expectedConfig->numSlices    = CNL_GT3_9X8_MAX_NUM_SLICES;
        expectedConfig->numSubSlices = CNL_GT3_9_8MAX_NUM_SUBSLICES;
    }
    else
    {
        expectedConfig->numSlices    = 0;
        expectedConfig->numSubSlices = 0;
    }

    return MOS_STATUS_SUCCESS;
}

uint64_t CM_HAL_G10_X::ConverTicksToNanoSecondsDefault(uint64_t ticks)
{
    return (uint64_t)(ticks * CM_NS_PER_TICK_RENDER_G10_DEFAULT);
}

