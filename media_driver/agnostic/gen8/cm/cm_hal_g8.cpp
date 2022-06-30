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
//! \file      cm_hal_g8.cpp 
//! \brief     Common HAL CM Gen8 functions 
//!

#include "cm_hal_g8.h"
#include "cm_common.h"
#include "renderhal_platform_interface.h"
#include "mhw_state_heap_hwcmd_g8_X.h"
#if defined(ENABLE_KERNELS) && (!defined(_FULL_OPEN_SOURCE))
#include "cm_gpucopy_kernel_g8.h"
#include "cm_gpuinit_kernel_g8.h"
#else
unsigned int iGPUCopy_kernel_isa_size_gen8 = 0;
unsigned int iGPUInit_kernel_isa_size_Gen8 = 0;
unsigned char *pGPUCopy_kernel_isa_gen8 = nullptr;
unsigned char *pGPUInit_kernel_isa_Gen8 = nullptr;

#endif

#define CM_NS_PER_TICK_RENDER_G8        (80)

union CM_HAL_MEMORY_OBJECT_CONTROL_G8
{
    struct
    {
        uint32_t age          : 2;
        uint32_t              : 1;
        uint32_t targetCache  : 2;
        uint32_t cacheControl : 2;
        uint32_t              : 25;
    } Gen8;

    uint32_t value;
};

#if (_RELEASE_INTERNAL || _DEBUG)
#if defined(CM_DIRECT_GUC_SUPPORT)
MOS_STATUS CM_HAL_G8_X::SubmitDummyCommands(
    PMHW_BATCH_BUFFER       batchBuffer,
    int32_t                 taskId,
    PCM_HAL_KERNEL_PARAM    *kernelParam,
    void                    **cmdBuffer)
{
    return MOS_STATUS_UNIMPLEMENTED;

}
#endif
#endif

MOS_STATUS CM_HAL_G8_X::SubmitCommands(
    PMHW_BATCH_BUFFER       batchBuffer,
    int32_t                 taskId,
    PCM_HAL_KERNEL_PARAM    *kernelParam,
    void                    **cmdBuffer)
{
    MOS_STATUS                      eStatus        = MOS_STATUS_SUCCESS;
    PCM_HAL_STATE                   state          = m_cmState;
    PMOS_INTERFACE                  osInterface    = m_cmState->osInterface;
    PRENDERHAL_INTERFACE_LEGACY     renderHal      = m_cmState->renderHal;
    MhwRenderInterface              *mhwRender     = renderHal->pMhwRenderInterface;
    PMHW_MI_INTERFACE               mhwMiInterface = renderHal->pMhwMiInterface;
    PRENDERHAL_STATE_HEAP           stateHeap      = renderHal->pStateHeap;
    MHW_PIPE_CONTROL_PARAMS         pipeCtrlParams   = g_cRenderHal_InitPipeControlParams;
    MHW_MEDIA_STATE_FLUSH_PARAM     flushParam      = g_cRenderHal_InitMediaStateFlushParams;
    MHW_ID_LOAD_PARAMS              idLoadParams;
    int32_t                         remaining      = 0;
    bool                            enableWalker    = state->walkerParams.CmWalkerEnable;
    bool                            enableGpGpu     = state->taskParam->blGpGpuWalkerEnabled;
    MOS_COMMAND_BUFFER              mosCmdBuffer;
    uint32_t                        syncTag;
    int64_t                         *taskSyncLocation;
    int32_t                         syncOffset;
    int32_t                         tmp;
    PCM_HAL_TASK_PARAM              taskParam = state->taskParam;
    PCM_HAL_BB_ARGS                 bbCmArgs;
    RENDERHAL_GENERIC_PROLOG_PARAMS genericPrologParams = {};
    MOS_RESOURCE                    *osResource;
    uint32_t                        tag;
    uint32_t                        tagOffset = 0;
    bool                            slmUsed = false;

    MOS_ZeroMemory(&mosCmdBuffer, sizeof(MOS_COMMAND_BUFFER));

    // get the tag
    tag = renderHal->trackerProducer.GetNextTracker(renderHal->currentTrackerIndex);

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

    // Update power option of this command;
    CM_CHK_MOSSTATUS_GOTOFINISH( state->pfnUpdatePowerOption( state, &state->powerOption ) );

    // Register batch buffer for rendering
    if (!enableWalker && !enableGpGpu)
    {
        CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnRegisterResource(
            osInterface,
            &batchBuffer->OsResource,
            true,
            true));
    }

    // Register Timestamp Buffer
    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnRegisterResource(
        osInterface,
        &state->renderTimeStampResource.osResource,
        true,
        true));

    // Allocate all available space, unused buffer will be returned later
    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnGetCommandBuffer(osInterface, &mosCmdBuffer, 0));
    remaining = mosCmdBuffer.iRemaining;

    // Enable preemption flag in the command buffer header
    // The flag is required for both Middle Batch Buffer(Thread Group) and Middle Thread preemptions.
    if (enableGpGpu)
    {
        if (taskParam->slmSize == 0 && taskParam->hasBarrier == false)
        {
            state->renderHal->pfnEnableGpgpuMiddleBatchBufferPreemption(state->renderHal);
        }
    }

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
    pipeCtrlParams = g_cRenderHal_InitPipeControlParams;
    pipeCtrlParams.presDest          = &state->renderTimeStampResource.osResource;
    pipeCtrlParams.dwResourceOffset  = syncOffset;
    pipeCtrlParams.dwPostSyncOp      = MHW_FLUSH_WRITE_TIMESTAMP_REG;
    pipeCtrlParams.dwFlushMode       = MHW_FLUSH_WRITE_CACHE;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(&mosCmdBuffer, nullptr, &pipeCtrlParams));

    // Initialize command buffer and insert prolog
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnInitCommandBuffer(renderHal, &mosCmdBuffer, &genericPrologParams));

    // update tracker tag used with CM tracker resource
    renderHal->trackerProducer.StepForward(renderHal->currentTrackerIndex);

    // Increment sync tag
    syncTag = renderHal->pStateHeap->dwNextTag++;

    // Check if any task to use SLM
    for (uint32_t i = 0; i < state->taskParam->numKernels; i ++)
    {
        if (kernelParam[i]->slmSize > 0)
        {
            slmUsed = true;
            break;
        }
    }

    //Check GPGPU task param
    if (taskParam->slmSize > 0) {
        slmUsed = true;
    }

    //enable BDW L3 config
    state->l3Settings.enableSlm = slmUsed;
    HalCm_GetLegacyRenderHalL3Setting( &state->l3Settings, &renderHal->L3CacheSettings );
    renderHal->pfnEnableL3Caching(renderHal, &renderHal->L3CacheSettings);
    mhwRender->SetL3Cache(&mosCmdBuffer);

    if (renderHal->bSIPKernel)
    {
        CM_CHK_MOSSTATUS_GOTOFINISH(SetupHwDebugControl(renderHal, &mosCmdBuffer));
    }

    // Send Pipeline Select command
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwRender->AddPipelineSelectCmd(&mosCmdBuffer, enableGpGpu));

    // Send State Base Address command
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSendStateBaseAddress(renderHal, &mosCmdBuffer));

    // Send Surface States
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSendSurfaces(renderHal, &mosCmdBuffer));

    if ( renderHal->bSIPKernel)
    {
        // Send SIP State
        CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSendSipStateCmd(renderHal, &mosCmdBuffer));
    }

    // Setup VFE State params. Each Renderer MUST call pfnSetVfeStateParams().
    // See comment in RenderHal_SetVfeStateParams() for details.
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
        PRENDERHAL_DYNAMIC_STATE dynamicState = ((PRENDERHAL_MEDIA_STATE_LEGACY)stateHeap->pCurMediaState)->pDynamicState;
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
            if ( taskParam->conditionalEndBitmap & ((uint64_t)1 << (i)))
            {
                // this could be batch buffer end so need to update sync tag, media state flush, write end timestamp

                CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnSendSyncTag(renderHal, &mosCmdBuffer));

                // WA for BDW/CHV
                if (MEDIA_IS_WA(renderHal->pWaTable, WaMSFWithNoWatermarkTSGHang))
                {
                    flushParam.bFlushToGo = 1;
                    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMediaStateFlush(&mosCmdBuffer, nullptr, &flushParam));
                }
                else if (MEDIA_IS_WA(renderHal->pWaTable, WaAddMediaStateFlushCmd))
                {
                    flushParam.bFlushToGo = 0;
                    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMediaStateFlush(&mosCmdBuffer, nullptr, &flushParam));
                }

                // Insert a pipe control for synchronization since this Conditional Batch Buffer End command
                // will use value written by previous kernel. Also needed since this may be the Batch Buffer End
                pipeCtrlParams = g_cRenderHal_InitPipeControlParams;
                pipeCtrlParams.presDest = &state->renderTimeStampResource.osResource;
                pipeCtrlParams.dwPostSyncOp = MHW_FLUSH_NOWRITE;
                pipeCtrlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
                                CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(&mosCmdBuffer, nullptr, &pipeCtrlParams));

                // issue a PIPE_CONTROL to write timestamp
                pipeCtrlParams = g_cRenderHal_InitPipeControlParams;
                pipeCtrlParams.presDest = &state->renderTimeStampResource.osResource;
                pipeCtrlParams.dwResourceOffset = syncOffset + sizeof(uint64_t);
                pipeCtrlParams.dwPostSyncOp = MHW_FLUSH_WRITE_TIMESTAMP_REG;
                pipeCtrlParams.dwFlushMode = MHW_FLUSH_READ_CACHE;
                                CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(&mosCmdBuffer, nullptr, &pipeCtrlParams));

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
                pipeCtrlParams = g_cRenderHal_InitPipeControlParams;
                pipeCtrlParams.presDest = &state->renderTimeStampResource.osResource;
                pipeCtrlParams.dwPostSyncOp = MHW_FLUSH_NOWRITE;
                pipeCtrlParams.dwFlushMode = MHW_FLUSH_CUSTOM;
                pipeCtrlParams.bInvalidateTextureCache = true;
                pipeCtrlParams.bFlushRenderTargetCache = true;
                CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(&mosCmdBuffer, nullptr, &pipeCtrlParams));
            }

            CM_CHK_MOSSTATUS_GOTOFINISH(state->pfnSendMediaWalkerState(state, kernelParam[i], &mosCmdBuffer));
        }

        // WA for BDW/CHV
        if (MEDIA_IS_WA(renderHal->pWaTable, WaMSFWithNoWatermarkTSGHang))
        {
            flushParam.bFlushToGo = 1;
            CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMediaStateFlush(&mosCmdBuffer, nullptr, &flushParam));
        }
        else if (MEDIA_IS_WA(renderHal->pWaTable, WaAddMediaStateFlushCmd))
        {
            flushParam.bFlushToGo = 0;
            CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMediaStateFlush(&mosCmdBuffer, nullptr, &flushParam));
        }
    }
    else if (enableGpGpu)
    {
        // send GPGPU walker command, if required
        for (uint32_t i = 0; i < state->taskParam->numKernels; i ++)
        {
            //Insert PIPE_CONTROL as synchronization if synchronization is set
            if((i > 0) && (taskParam->syncBitmap & ((uint64_t)1 << (i-1))))
            {
                //Insert a pipe control as synchronization
                pipeCtrlParams = g_cRenderHal_InitPipeControlParams;
                pipeCtrlParams.presDest = &state->renderTimeStampResource.osResource;
                pipeCtrlParams.dwPostSyncOp = MHW_FLUSH_NOWRITE;
                pipeCtrlParams.dwFlushMode = MHW_FLUSH_CUSTOM;
                pipeCtrlParams.bInvalidateTextureCache = true;
                pipeCtrlParams.bFlushRenderTargetCache = true;
                CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(&mosCmdBuffer, nullptr, &pipeCtrlParams));
            }

            CM_CHK_MOSSTATUS_GOTOFINISH(state->pfnSendGpGpuWalkerState(state, kernelParam[i], &mosCmdBuffer));
        }

        // WA for BDW/CHV
        if (MEDIA_IS_WA(renderHal->pWaTable, WaMSFWithNoWatermarkTSGHang))
        {
            flushParam.bFlushToGo = 1;
            CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMediaStateFlush(&mosCmdBuffer, nullptr, &flushParam));
        }
        else if (MEDIA_IS_WA(renderHal->pWaTable, WaAddMediaStateFlushCmd))
        {
            flushParam.bFlushToGo = 0;
            CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMediaStateFlush(&mosCmdBuffer, nullptr, &flushParam));
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
            // Add BB end command
            mhwMiInterface->AddMiBatchBufferEnd(nullptr, batchBuffer);
        }
        else //reuse BB
        {
            // Skip BB end command
            mhwMiInterface->SkipMiBatchBufferEndBb(batchBuffer);
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
    pipeCtrlParams = g_cRenderHal_InitPipeControlParams;
    pipeCtrlParams.presDest      = &state->renderTimeStampResource.osResource;
    pipeCtrlParams.dwPostSyncOp  = MHW_FLUSH_NOWRITE;
    pipeCtrlParams.dwFlushMode   = MHW_FLUSH_WRITE_CACHE;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(&mosCmdBuffer, nullptr, &pipeCtrlParams));

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

    if ( slmUsed & state->pfnIsWASLMinL3Cache())
    {
        //Disable SLM in L3 when command submitted
        state->l3Settings.enableSlm = false;
        HalCm_GetLegacyRenderHalL3Setting( &state->l3Settings, &renderHal->L3CacheSettings );
        renderHal->pfnEnableL3Caching(renderHal, &renderHal->L3CacheSettings);
        mhwRender->SetL3Cache(&mosCmdBuffer);
    }

    // Send Sync Tag
    if (!state->dshEnabled || !(enableWalker || enableGpGpu))
    {
        CM_CHK_MOSSTATUS_GOTOFINISH( renderHal->pfnSendSyncTag( renderHal, &mosCmdBuffer ) );
    }

    // Update tracker resource
    CM_CHK_MOSSTATUS_GOTOFINISH(state->pfnUpdateTrackerResource(state, &mosCmdBuffer, tag));

    // issue a PIPE_CONTROL to write timestamp
    syncOffset += sizeof(uint64_t);
    pipeCtrlParams = g_cRenderHal_InitPipeControlParams;
    pipeCtrlParams.presDest          = &state->renderTimeStampResource.osResource;
    pipeCtrlParams.dwResourceOffset  = syncOffset;
    pipeCtrlParams.dwPostSyncOp      = MHW_FLUSH_WRITE_TIMESTAMP_REG;
    pipeCtrlParams.dwFlushMode       = MHW_FLUSH_READ_CACHE;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(&mosCmdBuffer, nullptr, &pipeCtrlParams));

    // Record registers by unified media profiler in the end
    if (state->perfProfiler != nullptr)
    {
        CM_CHK_MOSSTATUS_GOTOFINISH(state->perfProfiler->AddPerfCollectEndCmd((void *)state, state->osInterface, mhwMiInterface, &mosCmdBuffer));
    }
    
    //Couple to the BB_START , otherwise GPU Hang without it in KMD.
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiBatchBufferEnd(&mosCmdBuffer, nullptr));

    // Return unused command buffer space to OS
    osInterface->pfnReturnCommandBuffer(osInterface, &mosCmdBuffer, 0);

#if MDF_COMMAND_BUFFER_DUMP
    if (state->dumpCommandBuffer)
    {
        state->pfnDumpCommadBuffer(state, &mosCmdBuffer, 0, mhw_state_heap_g8_X::RENDER_SURFACE_STATE_CMD::byteSize);
    }
#endif


#if MDF_SURFACE_STATE_DUMP
    if (state->dumpSurfaceState)
    {
        state->pfnDumpSurfaceState(state, 0, mhw_state_heap_g8_X::RENDER_SURFACE_STATE_CMD::byteSize);
       
    }
#endif

    CM_CHK_MOSSTATUS_GOTOFINISH( state->pfnGetGpuTime( state, &state->taskTimeStamp->submitTimeInGpu[ taskId ] ) );
    CM_CHK_MOSSTATUS_GOTOFINISH( state->pfnGetGlobalTime( &state->taskTimeStamp->submitTimeInCpu[ taskId ] ) );

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

    // reset EU saturation
    state->euSaturationEnabled = false;

    renderHal->bEUSaturationNoSSD   = false;

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

MOS_STATUS CM_HAL_G8_X::SetMediaWalkerParams(
    CM_WALKING_PARAMETERS          engineeringParams,
    PCM_HAL_WALKER_PARAMS          walkerParams)
{

    MEDIA_OBJECT_WALKER_CMD_G6 mediaWalkerCmd;
    mediaWalkerCmd.DW5.value = engineeringParams.Value[0];
    walkerParams->scoreboardMask = mediaWalkerCmd.DW5.scoreboardMask;

    mediaWalkerCmd.DW6.value = engineeringParams.Value[1];
    walkerParams->colorCountMinusOne = mediaWalkerCmd.DW6.colorCountMinusOne;
    walkerParams->midLoopUnitX = mediaWalkerCmd.DW6.midLoopUnitX;
    walkerParams->midLoopUnitY = mediaWalkerCmd.DW6.midLoopUnitY;
    walkerParams->middleLoopExtraSteps = mediaWalkerCmd.DW6.midLoopExtraSteps;

    mediaWalkerCmd.DW7.value = engineeringParams.Value[2];
    walkerParams->localLoopExecCount = mediaWalkerCmd.DW7.localLoopExecCount;
    walkerParams->globalLoopExecCount = mediaWalkerCmd.DW7.globalLoopExecCount;

    mediaWalkerCmd.DW8.value = engineeringParams.Value[3];
    walkerParams->blockResolution.x = mediaWalkerCmd.DW8.blockResolutionX;
    walkerParams->blockResolution.y = mediaWalkerCmd.DW8.blockResolutionY;

    mediaWalkerCmd.DW9.value = engineeringParams.Value[4];
    walkerParams->localStart.x = mediaWalkerCmd.DW9.localStartX;
    walkerParams->localStart.y = mediaWalkerCmd.DW9.localStartY;

    mediaWalkerCmd.DW11.value = engineeringParams.Value[6];
    walkerParams->localOutLoopStride.x = mediaWalkerCmd.DW11.localOuterLoopStrideX;
    walkerParams->localOutLoopStride.y = mediaWalkerCmd.DW11.localOuterLoopStrideY;

    mediaWalkerCmd.DW12.value = engineeringParams.Value[7];
    walkerParams->localInnerLoopUnit.x = mediaWalkerCmd.DW12.localInnerLoopUnitX;
    walkerParams->localInnerLoopUnit.y = mediaWalkerCmd.DW12.localInnerLoopUnitY;

    mediaWalkerCmd.DW13.value = engineeringParams.Value[8];
    walkerParams->globalResolution.x = mediaWalkerCmd.DW13.globalResolutionX;
    walkerParams->globalResolution.y = mediaWalkerCmd.DW13.globalResolutionY;

    mediaWalkerCmd.DW14.value = engineeringParams.Value[9];
    walkerParams->globalStart.x = mediaWalkerCmd.DW14.globalStartX;
    walkerParams->globalStart.y = mediaWalkerCmd.DW14.globalStartY;

    mediaWalkerCmd.DW15.value = engineeringParams.Value[10];
    walkerParams->globalOutlerLoopStride.x = mediaWalkerCmd.DW15.globalOuterLoopStrideX;
    walkerParams->globalOutlerLoopStride.y = mediaWalkerCmd.DW15.globalOuterLoopStrideY;

    mediaWalkerCmd.DW16.value = engineeringParams.Value[11];
    walkerParams->globalInnerLoopUnit.x = mediaWalkerCmd.DW16.globalInnerLoopUnitX;
    walkerParams->globalInnerLoopUnit.y = mediaWalkerCmd.DW16.globalInnerLoopUnitY;

    walkerParams->localEnd.x = 0;
    walkerParams->localEnd.y = 0;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CM_HAL_G8_X::HwSetSurfaceMemoryObjectControl(
    uint16_t                        memObjCtl,
    PRENDERHAL_SURFACE_STATE_PARAMS surfStateParams )
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE            renderHal = m_cmState->renderHal;
    CM_HAL_MEMORY_OBJECT_CONTROL_G8 cacheType;

    MOS_ZeroMemory( &cacheType, sizeof( CM_HAL_MEMORY_OBJECT_CONTROL_G8 ) );

    if ( ( memObjCtl & CM_MEMOBJCTL_CACHE_MASK ) >> 8 == CM_INVALID_MEMOBJCTL )
    {
        CM_CHK_NULL_GOTOFINISH_MOSERROR(renderHal->pOsInterface->pfnGetGmmClientContext(renderHal->pOsInterface));
        cacheType.value = renderHal->pOsInterface->pfnGetGmmClientContext(renderHal->pOsInterface)->CachePolicyGetMemoryObject(nullptr, CM_RESOURCE_USAGE_SurfaceState).DwordValue;

        // for default value and SVM surface, override the cache control from WB to WT
        if ( ( ( memObjCtl & 0xF0 ) >> 4 ) == 2 )
        {
            cacheType.Gen8.cacheControl = 2;
        }
    }
    else
    {
        // Get the cache type of the memory object.
        // Since memObjCtl is composed with cache type(8:15), memory type(4:7), ages(0:3), rearranging is needed
        cacheType.Gen8.age = ( memObjCtl & 0xF );
        cacheType.Gen8.cacheControl = ( memObjCtl & 0xF0 ) >> 4;
        cacheType.Gen8.targetCache = ( memObjCtl & CM_MEMOBJCTL_CACHE_MASK ) >> 8;
    }

    surfStateParams->MemObjCtl = cacheType.value;
    
finish:
    return eStatus;
}

MOS_STATUS CM_HAL_G8_X::RegisterSampler8x8(
    PCM_HAL_SAMPLER_8X8_PARAM    param)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    PMHW_SAMPLER_STATE_PARAM    samplerEntry = nullptr;
    PCM_HAL_SAMPLER_8X8_ENTRY   sampler8x8Entry = nullptr;
    PCM_HAL_STATE               state = m_cmState;

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

        int16_t samplerIndex = 0;
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
            CM_ASSERTMESSAGE("Sampler or AVS table is full");
            return MOS_STATUS_NULL_POINTER;
        }

        //State data from application
        samplerEntry->SamplerType                  = MHW_SAMPLER_TYPE_AVS;
        samplerEntry->ElementType                  = MHW_Sampler64Elements;
        samplerEntry->Avs                          = param->sampler8x8State.avsParam.avsState;
        samplerEntry->Avs.stateID                  = samplerIndex;
        samplerEntry->Avs.iTable8x8_Index          = samplerIndex;  // Used for calculating the Media offset of 8x8 table
        samplerEntry->Avs.pMhwSamplerAvsTableParam = &sampler8x8Entry->sampler8x8State.mhwSamplerAvsTableParam;

        if (samplerEntry->Avs.EightTapAFEnable)
            param->sampler8x8State.avsParam.avsTable.adaptiveFilterAllChannels = true;
        else
            param->sampler8x8State.avsParam.avsTable.adaptiveFilterAllChannels = false;

        RegisterSampler8x8AVSTable(&sampler8x8Entry->sampler8x8State,
                                   &param->sampler8x8State.avsParam.avsTable);

        sampler8x8Entry->sampler8x8State.stateType  = CM_SAMPLER8X8_AVS;
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

        if ( samplerEntry == nullptr )
        {
            return MOS_STATUS_INVALID_HANDLE;
        }
        samplerEntry->SamplerType  = MHW_SAMPLER_TYPE_MISC;

        samplerEntry->Misc.byteHeight = param->sampler8x8State.miscState.DW0.Height;
        samplerEntry->Misc.byteWidth  = param->sampler8x8State.miscState.DW0.Width;
        samplerEntry->Misc.wRow[0]    = param->sampler8x8State.miscState.DW0.Row0;
        samplerEntry->Misc.wRow[1]    = param->sampler8x8State.miscState.DW1.Row1;
        samplerEntry->Misc.wRow[2]    = param->sampler8x8State.miscState.DW1.Row2;
        samplerEntry->Misc.wRow[3]    = param->sampler8x8State.miscState.DW2.Row3;
        samplerEntry->Misc.wRow[4]    = param->sampler8x8State.miscState.DW2.Row4;
        samplerEntry->Misc.wRow[5]    = param->sampler8x8State.miscState.DW3.Row5;
        samplerEntry->Misc.wRow[6]    = param->sampler8x8State.miscState.DW3.Row6;
        samplerEntry->Misc.wRow[7]    = param->sampler8x8State.miscState.DW4.Row7;
        samplerEntry->Misc.wRow[8]    = param->sampler8x8State.miscState.DW4.Row8;
        samplerEntry->Misc.wRow[9]    = param->sampler8x8State.miscState.DW5.Row9;
        samplerEntry->Misc.wRow[10]   = param->sampler8x8State.miscState.DW5.Row10;
        samplerEntry->Misc.wRow[11]   = param->sampler8x8State.miscState.DW6.Row11;
        samplerEntry->Misc.wRow[12]   = param->sampler8x8State.miscState.DW6.Row12;
        samplerEntry->Misc.wRow[13]   = param->sampler8x8State.miscState.DW7.Row13;
        samplerEntry->Misc.wRow[14]   = param->sampler8x8State.miscState.DW7.Row14;
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

        if ( samplerEntry == nullptr )
        {
            return MOS_STATUS_INVALID_HANDLE;
        }

        MOS_ZeroMemory(&samplerEntry->Convolve, sizeof(samplerEntry->Convolve));

        samplerEntry->SamplerType  = MHW_SAMPLER_TYPE_CONV;

        samplerEntry->Convolve.ui8Height               = param->sampler8x8State.convolveState.height;
        samplerEntry->Convolve.ui8Width                = param->sampler8x8State.convolveState.width;
        samplerEntry->Convolve.ui8ScaledDownValue      = param->sampler8x8State.convolveState.scaleDownValue;
        samplerEntry->Convolve.ui8SizeOfTheCoefficient = param->sampler8x8State.convolveState.coeffSize;

        samplerEntry->ElementType = MHW_Sampler64Elements;

        for ( int i = 0; i < CM_NUM_CONVOLVE_ROWS_BDW; i++ )
        {
            MHW_SAMPLER_CONVOLVE_COEFF_TABLE *coeffTable  = &(samplerEntry->Convolve.CoeffTable[i]);
            CM_HAL_CONVOLVE_COEFF_TABLE      *sourceTable = &(param->sampler8x8State.convolveState.table[i]);
            if ( samplerEntry->Convolve.ui8SizeOfTheCoefficient == 1 )
            {
                coeffTable->wFilterCoeff[0]  = FloatToS3_12( sourceTable->FilterCoeff_0_0 );
                coeffTable->wFilterCoeff[1]  = FloatToS3_12( sourceTable->FilterCoeff_0_1 );
                coeffTable->wFilterCoeff[2]  = FloatToS3_12( sourceTable->FilterCoeff_0_2 );
                coeffTable->wFilterCoeff[3]  = FloatToS3_12( sourceTable->FilterCoeff_0_3 );
                coeffTable->wFilterCoeff[4]  = FloatToS3_12( sourceTable->FilterCoeff_0_4 );
                coeffTable->wFilterCoeff[5]  = FloatToS3_12( sourceTable->FilterCoeff_0_5 );
                coeffTable->wFilterCoeff[6]  = FloatToS3_12( sourceTable->FilterCoeff_0_6 );
                coeffTable->wFilterCoeff[7]  = FloatToS3_12( sourceTable->FilterCoeff_0_7 );
                coeffTable->wFilterCoeff[8]  = FloatToS3_12( sourceTable->FilterCoeff_0_8 );
                coeffTable->wFilterCoeff[9]  = FloatToS3_12( sourceTable->FilterCoeff_0_9 );
                coeffTable->wFilterCoeff[10] = FloatToS3_12( sourceTable->FilterCoeff_0_10 );
                coeffTable->wFilterCoeff[11] = FloatToS3_12( sourceTable->FilterCoeff_0_11 );
                coeffTable->wFilterCoeff[12] = FloatToS3_12( sourceTable->FilterCoeff_0_12 );
                coeffTable->wFilterCoeff[13] = FloatToS3_12( sourceTable->FilterCoeff_0_13 );
                coeffTable->wFilterCoeff[14] = FloatToS3_12( sourceTable->FilterCoeff_0_14 );
                coeffTable->wFilterCoeff[15] = FloatToS3_12( sourceTable->FilterCoeff_0_15 );
            }
            else
            {
                coeffTable->wFilterCoeff[0]  = FloatToS3_4( sourceTable->FilterCoeff_0_0 );
                coeffTable->wFilterCoeff[1]  = FloatToS3_4( sourceTable->FilterCoeff_0_1 );
                coeffTable->wFilterCoeff[2]  = FloatToS3_4( sourceTable->FilterCoeff_0_2 );
                coeffTable->wFilterCoeff[3]  = FloatToS3_4( sourceTable->FilterCoeff_0_3 );
                coeffTable->wFilterCoeff[4]  = FloatToS3_4( sourceTable->FilterCoeff_0_4 );
                coeffTable->wFilterCoeff[5]  = FloatToS3_4( sourceTable->FilterCoeff_0_5 );
                coeffTable->wFilterCoeff[6]  = FloatToS3_4( sourceTable->FilterCoeff_0_6 );
                coeffTable->wFilterCoeff[7]  = FloatToS3_4( sourceTable->FilterCoeff_0_7 );
                coeffTable->wFilterCoeff[8]  = FloatToS3_4( sourceTable->FilterCoeff_0_8 );
                coeffTable->wFilterCoeff[9]  = FloatToS3_4( sourceTable->FilterCoeff_0_9 );
                coeffTable->wFilterCoeff[10] = FloatToS3_4( sourceTable->FilterCoeff_0_10 );
                coeffTable->wFilterCoeff[11] = FloatToS3_4( sourceTable->FilterCoeff_0_11 );
                coeffTable->wFilterCoeff[12] = FloatToS3_4( sourceTable->FilterCoeff_0_12 );
                coeffTable->wFilterCoeff[13] = FloatToS3_4( sourceTable->FilterCoeff_0_13 );
                coeffTable->wFilterCoeff[14] = FloatToS3_4( sourceTable->FilterCoeff_0_14 );
                coeffTable->wFilterCoeff[15] = FloatToS3_4( sourceTable->FilterCoeff_0_15 );
            }
        }

    }

    return eStatus;
}

MOS_STATUS CM_HAL_G8_X::SetupHwDebugControl(
    PRENDERHAL_INTERFACE   renderHal,
    PMOS_COMMAND_BUFFER    cmdBuffer)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    if (!renderHal || !cmdBuffer)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    MHW_MI_LOAD_REGISTER_IMM_PARAMS loadRegImm;
    MOS_ZeroMemory(&loadRegImm, sizeof(MHW_MI_LOAD_REGISTER_IMM_PARAMS));

    // INSTPM, global debug enable
    loadRegImm.dwRegister = INSTPM;
    loadRegImm.dwData = (INSTPM_GLOBAL_DEBUG_ENABLE << 16) | INSTPM_GLOBAL_DEBUG_ENABLE;
    eStatus = renderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &loadRegImm);
    if(eStatus != MOS_STATUS_SUCCESS)
    {
        return eStatus;
    }

    // TD_CTL, force thread breakpoint enable
    // Also enable external exception, because the source-level debugger has to
    // be able to interrupt runing EU threads.
    loadRegImm.dwRegister = TD_CTL;
    loadRegImm.dwData = TD_CTL_FORCE_THREAD_BKPT_ENABLE | TD_CTL_FORCE_EXT_EXCEPTION_ENABLE;
    eStatus = renderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &loadRegImm);

    return eStatus;
}

MOS_STATUS CM_HAL_G8_X::RegisterSampler8x8AVSTable(
    PCM_HAL_SAMPLER_8X8_TABLE  sampler8x8AvsTable,
    PCM_AVS_TABLE_STATE_PARAMS avsTable)
{
    MOS_ZeroMemory(&sampler8x8AvsTable->mhwSamplerAvsTableParam, sizeof(sampler8x8AvsTable->mhwSamplerAvsTableParam));

    sampler8x8AvsTable->mhwSamplerAvsTableParam.byteTransitionArea8Pixels = MEDIASTATE_AVS_TRANSITION_AREA_8_PIXELS;
    sampler8x8AvsTable->mhwSamplerAvsTableParam.byteTransitionArea4Pixels = MEDIASTATE_AVS_TRANSITION_AREA_4_PIXELS;
    sampler8x8AvsTable->mhwSamplerAvsTableParam.byteMaxDerivative8Pixels  = MEDIASTATE_AVS_MAX_DERIVATIVE_8_PIXELS;
    sampler8x8AvsTable->mhwSamplerAvsTableParam.byteMaxDerivative4Pixels  = MEDIASTATE_AVS_MAX_DERIVATIVE_4_PIXELS;
    sampler8x8AvsTable->mhwSamplerAvsTableParam.byteDefaultSharpnessLevel = MEDIASTATE_AVS_SHARPNESS_LEVEL_SHARP;

    sampler8x8AvsTable->mhwSamplerAvsTableParam.bEnableRGBAdaptive         = false;
    sampler8x8AvsTable->mhwSamplerAvsTableParam.bAdaptiveFilterAllChannels = avsTable->adaptiveFilterAllChannels;
    sampler8x8AvsTable->mhwSamplerAvsTableParam.bBypassXAdaptiveFiltering  = true;
    sampler8x8AvsTable->mhwSamplerAvsTableParam.bBypassYAdaptiveFiltering  = true;

    // Assign the coefficient table;
    for (uint32_t i = 0; i < CM_NUM_HW_POLYPHASE_TABLES_G8; i++)
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

    if (!avsTable->bypassXAF  && !avsTable->bypassYAF) {
        sampler8x8AvsTable->mhwSamplerAvsTableParam.byteMaxDerivative8Pixels  = avsTable->maxDerivative8Pixels;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.byteMaxDerivative4Pixels  = avsTable->maxDerivative4Pixels;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.byteTransitionArea8Pixels = avsTable->transitionArea8Pixels;
        sampler8x8AvsTable->mhwSamplerAvsTableParam.byteTransitionArea4Pixels = avsTable->transitionArea4Pixels;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CM_HAL_G8_X::UpdatePlatformInfoFromPower(
    PCM_PLATFORM_INFO platformInfo,
    bool              euSaturated)
{
    PCM_HAL_STATE              state     = m_cmState;
    PRENDERHAL_INTERFACE       renderHal = state->renderHal;
    CM_POWER_OPTION            cmPower;

    if ( state->requestSingleSlice ||
         renderHal->bRequestSingleSlice ||
        (state->powerOption.nSlice != 0 && state->powerOption.nSlice < platformInfo->numSlices))
    {
        platformInfo->numSubSlices = platformInfo->numSubSlices / platformInfo->numSlices;
        if (state->powerOption.nSlice > 1)
        {
            platformInfo->numSubSlices *= state->powerOption.nSlice;
            platformInfo->numSlices     = state->powerOption.nSlice;
        }
        else
        {
            platformInfo->numSlices     = 1;
        }
    }
    else if (euSaturated)
    {
        // No SSD and EU Saturation, request maximum number of slices/subslices/EUs
        cmPower.nSlice    = (uint16_t)platformInfo->numSlices;
        cmPower.nSubSlice = (uint16_t)platformInfo->numSubSlices;
        cmPower.nEU       = (uint16_t)(platformInfo->numEUsPerSubSlice * platformInfo->numSubSlices);

        state->pfnSetPowerOption(state, &cmPower);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CM_HAL_G8_X::GetExpectedGtSystemConfig(
    PCM_EXPECTED_GT_SYSTEM_INFO expectedConfig)
{
    if (m_genGT == PLATFORM_INTEL_GT1)
    {
        expectedConfig->numSlices    = BDW_GT1_MAX_NUM_SLICES;
        expectedConfig->numSubSlices = BDW_GT1_MAX_NUM_SUBSLICES;
    }
    else if( m_genGT == PLATFORM_INTEL_GT1_5 )
    {
        expectedConfig->numSlices    = BDW_GT1_5_MAX_NUM_SLICES;
        expectedConfig->numSubSlices = BDW_GT1_5_MAX_NUM_SUBSLICES;
    }
    else if (m_genGT == PLATFORM_INTEL_GT2)
    {
        expectedConfig->numSlices    = BDW_GT2_MAX_NUM_SLICES;
        expectedConfig->numSubSlices = BDW_GT2_MAX_NUM_SUBSLICES;
    }
    else if (m_genGT == PLATFORM_INTEL_GT3)
    {
        expectedConfig->numSlices    = BDW_GT3_MAX_NUM_SLICES;
        expectedConfig->numSubSlices = BDW_GT3_MAX_NUM_SUBSLICES;
    }
    else
    {
        expectedConfig->numSlices    = 0;
        expectedConfig->numSubSlices = 0;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CM_HAL_G8_X::AllocateSIPCSRResource()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    if (Mos_ResourceIsNull(&m_cmState->sipResource.osResource))
    {
        eStatus = HalCm_AllocateSipResource(m_cmState); // create  sip resource if it does not exist
    }

    return eStatus;
}

MOS_STATUS CM_HAL_G8_X::GetCopyKernelIsa(void  *&isa, uint32_t &isaSize)
{

    isa = (void *)pGPUCopy_kernel_isa_gen8;
    isaSize = iGPUCopy_kernel_isa_size_gen8;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CM_HAL_G8_X::GetInitKernelIsa(void  *&isa, uint32_t &isaSize)
{
    isa = (void *)pGPUInit_kernel_isa_Gen8;
    isaSize = iGPUInit_kernel_isa_size_Gen8;

    return MOS_STATUS_SUCCESS;
}

uint32_t CM_HAL_G8_X::GetMediaWalkerMaxThreadWidth()
{
    return CM_MAX_THREADSPACE_WIDTH_FOR_MW;
}

uint32_t CM_HAL_G8_X::GetMediaWalkerMaxThreadHeight()
{
    return CM_MAX_THREADSPACE_HEIGHT_FOR_MW;
}

MOS_STATUS CM_HAL_G8_X::GetHwSurfaceBTIInfo(
          PCM_SURFACE_BTI_INFO btiInfo)
{
    if (btiInfo == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    btiInfo->normalSurfaceStart      =  CM_NULL_SURFACE_BINDING_INDEX + 1;
    btiInfo->normalSurfaceEnd        =  CM_GLOBAL_SURFACE_INDEX_START - 1;
    btiInfo->reservedSurfaceStart    =  CM_GLOBAL_SURFACE_INDEX_START;
    btiInfo->reservedSurfaceEnd      =  CM_GLOBAL_SURFACE_INDEX_START +  CM_GLOBAL_SURFACE_NUMBER + CM_GTPIN_SURFACE_NUMBER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CM_HAL_G8_X::SetSuggestedL3Conf(
            L3_SUGGEST_CONFIG l3Config)
{
    if (l3Config >= sizeof(BDW_L3_PLANE)/sizeof(L3ConfigRegisterValues))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    return HalCm_SetL3Cache((L3ConfigRegisterValues *)&BDW_L3_PLANE[l3Config],
                                     &m_cmState->l3Settings);
}

MOS_STATUS CM_HAL_G8_X::GetGenStepInfo(char*& stepInfoStr)
{
    const char *genSteppingInfoTable[] = { "A0", "XX", "XX", "B0", "D0", "E0", "F0",
                                           "G0", "G1", "H0", "J0" };

    uint32_t genStepId = m_cmState->platform.usRevId;

    uint32_t tablesize = sizeof(genSteppingInfoTable) / sizeof(char *);

    if (genStepId < tablesize)
    {
        stepInfoStr = (char *)genSteppingInfoTable[genStepId];
    }
    else
    {
        stepInfoStr = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

int32_t CM_HAL_G8_X::ColorCountSanityCheck(uint32_t colorCount)
{
    if (colorCount == CM_INVALID_COLOR_COUNT || colorCount > CM_THREADSPACE_MAX_COLOR_COUNT)
    {
        CM_ASSERTMESSAGE("Error: Invalid color count.");
        return CM_INVALID_ARG_VALUE;
    }
    return CM_SUCCESS;
}

bool CM_HAL_G8_X::MemoryObjectCtrlPolicyCheck(uint32_t memCtrl)
{
    if ( memCtrl > MEMORY_OBJECT_CONTROL_BDW_L3_LLC_ELLC_ALLOWED )
    {
        return false;
    }

    return true;
}

int32_t CM_HAL_G8_X::GetConvSamplerIndex(
    PMHW_SAMPLER_STATE_PARAM  samplerParam,
    char                     *samplerIndexTable,
    int32_t                   nSamp8X8Num,
    int32_t                   nSampConvNum)
{

    //  2D convolve BDW
    int32_t samplerIndex = 1 + (nSamp8X8Num + nSampConvNum) * 2;
    while (samplerIndexTable[samplerIndex] != CM_INVALID_INDEX)
    {
        samplerIndex += 2;
    }

    return samplerIndex;
}

MOS_STATUS CM_HAL_G8_X::SetL3CacheConfig(
            const L3ConfigRegisterValues *values,
            PCmHalL3Settings cmHalL3Setting)
{
    return HalCm_SetL3Cache( values, cmHalL3Setting );
}

MOS_STATUS CM_HAL_G8_X::GetSamplerParamInfoForSamplerType(
            PMHW_SAMPLER_STATE_PARAM mhwSamplerParam,
            SamplerParam  &samplerParam)
{
    const unsigned int samplerElementSize[MAX_ELEMENT_TYPE_COUNT] = {16, 32, 64, 128, 1024, 2048};

    // gets element_type
    switch (mhwSamplerParam->SamplerType)
    {
        case MHW_SAMPLER_TYPE_CONV:
        case MHW_SAMPLER_TYPE_AVS:
            samplerParam.elementType = MHW_Sampler64Elements;
            break;
        case MHW_SAMPLER_TYPE_MISC:
            samplerParam.elementType = MHW_Sampler2Elements;
            break;
        case MHW_SAMPLER_TYPE_3D:
            samplerParam.elementType = MHW_Sampler1Element;
            break;
        default:
            samplerParam.elementType = MHW_Sampler1Element;
            break;
    }

    // bti_stepping for convolve or AVS is 2, other cases are 1.
    if ((mhwSamplerParam->SamplerType == MHW_SAMPLER_TYPE_CONV) ||
        (mhwSamplerParam->SamplerType == MHW_SAMPLER_TYPE_AVS))
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

    return MOS_STATUS_SUCCESS;
}

uint64_t CM_HAL_G8_X::ConverTicksToNanoSecondsDefault(uint64_t ticks)
{
    return (uint64_t)(ticks * CM_NS_PER_TICK_RENDER_G8);
}

