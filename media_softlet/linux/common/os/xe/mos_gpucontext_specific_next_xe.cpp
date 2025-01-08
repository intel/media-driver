/*
* Copyright (c) 2019-2023, Intel Corporation
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
//! \file    mos_gpucontext_specific_next_xe.cpp
//! \brief   Container class for the Linux specific gpu context
//!

#include <unistd.h>
#include "mos_gpucontext_specific_next_xe.h"
#include "mos_context_specific_next.h"
#include "mos_graphicsresource_specific_next.h"
#include "mos_commandbuffer_specific_next.h"
#include "mos_util_devult_specific_next.h"
#include "mos_cmdbufmgr_next.h"
#include "mos_os_virtualengine_next.h"
#include "mos_interface.h"
#include "mos_os_cp_interface_specific.h"

#define MI_BATCHBUFFER_END 0x05000000
static pthread_mutex_t command_dump_mutex = PTHREAD_MUTEX_INITIALIZER;


GpuContextSpecificNextXe::~GpuContextSpecificNextXe()
{
    MOS_OS_FUNCTION_ENTER;
}

MOS_STATUS GpuContextSpecificNextXe::Init3DCtx(PMOS_CONTEXT osParameters,
                PMOS_GPUCTX_CREATOPTIONS createOption,
                unsigned int *nengine,
                void *engine_map)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    __u16 engine_class = DRM_XE_ENGINE_CLASS_RENDER;
    __u64 caps = 0;
    uint8_t ctxWidth = 1;
    uint8_t numPlacement = 1;

    if (mos_query_engines(osParameters->bufmgr, engine_class, caps, nengine, engine_map))
    {
        MOS_OS_ASSERTMESSAGE("Failed to query engines.");
        return MOS_STATUS_UNKNOWN;
    }
    numPlacement = *nengine;

    m_i915Context[0] = mos_context_create_shared(osParameters->bufmgr,
                                             nullptr, //no need anymore, get vm_id from global
                                             0,
                                             m_bProtectedContext, // not support currently
                                             engine_map,
                                             ctxWidth,
                                             numPlacement,
                                             0);
    if (m_i915Context[0] == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to create context.");
        return MOS_STATUS_UNKNOWN;
    }
    m_i915Context[0]->pOsContext = osParameters;

    return eStatus;
}

MOS_STATUS GpuContextSpecificNextXe::InitComputeCtx(PMOS_CONTEXT osParameters,
                unsigned int *nengine,
                void *engine_map,
                MOS_GPU_NODE gpuNode,
                bool *isEngineSelectEnable)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    __u16 engine_class = DRM_XE_ENGINE_CLASS_COMPUTE;
    __u64 caps = 0;
    uint8_t ctxWidth = 1;
    uint8_t numPlacement = 1;

    if (mos_query_engines(osParameters->bufmgr, engine_class, caps, nengine, engine_map))
    {
        MOS_OS_ASSERTMESSAGE("Failed to query engines.");
        return MOS_STATUS_UNKNOWN;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    //Note: this debug function need to refine or override since different engine_map struct
    *isEngineSelectEnable = SelectEngineInstanceByUser(engine_map, nengine, m_engineInstanceSelect, gpuNode);
#endif

    numPlacement = *nengine;
    m_i915Context[0] = mos_context_create_shared(osParameters->bufmgr,
                                             nullptr,
                                             0,
                                             m_bProtectedContext,
                                             engine_map,
                                             ctxWidth,
                                             numPlacement,
                                             0);
    if (m_i915Context[0] == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to create context.");
        return MOS_STATUS_UNKNOWN;
    }
    m_i915Context[0]->pOsContext = osParameters;

    return eStatus;
}

MOS_STATUS GpuContextSpecificNextXe::InitVdVeCtx(PMOS_CONTEXT osParameters,
                MOS_STREAM_HANDLE streamState,
                PMOS_GPUCTX_CREATOPTIONS createOption,
                unsigned int *nengine,
                void *engine_map,
                MOS_GPU_NODE gpuNode,
                bool *isEngineSelectEnable)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;


    __u16 engine_class = (gpuNode == MOS_GPU_NODE_VE)? DRM_XE_ENGINE_CLASS_VIDEO_ENHANCE : DRM_XE_ENGINE_CLASS_VIDEO_DECODE;
    __u64 caps = 0;
    uint8_t ctxWidth = 1;
    uint8_t numPlacement = 1;

    SetEngineQueryFlags(createOption, caps);

    if (mos_query_engines(osParameters->bufmgr, engine_class, caps, nengine, engine_map))
    {
        MOS_OS_ASSERTMESSAGE("Failed to query engines.");
        return MOS_STATUS_UNKNOWN;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    *isEngineSelectEnable = SelectEngineInstanceByUser(engine_map, nengine, m_engineInstanceSelect, gpuNode);
#endif

    numPlacement = *nengine;

    m_i915Context[0] = mos_context_create_shared(osParameters->bufmgr,
                                             nullptr,
                                             0,
                                             m_bProtectedContext,
                                             engine_map,
                                             ctxWidth,
                                             numPlacement,
                                             0);
    if (m_i915Context[0] == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to create context.");
        return MOS_STATUS_UNKNOWN;
    }

    m_i915Context[0]->pOsContext = osParameters;

    if (*nengine >= 2 && *nengine <= MAX_ENGINE_INSTANCE_NUM)
    {
        //if ctxWidth > 1, numPlacement should always be 1
        numPlacement = 1;
        streamState->bParallelSubmission = true;
        //create context with different width
        for(int i = 1; i < *nengine; i++)
        {
            ctxWidth = i + 1;
            m_i915Context[i] = mos_context_create_shared(osParameters->bufmgr,
                                                         nullptr,
                                                         0,
                                                         m_bProtectedContext,
                                                         engine_map,
                                                         ctxWidth,
                                                         numPlacement,
                                                         0);
            if (m_i915Context[i] == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("Failed to create context.");
                return MOS_STATUS_UNKNOWN;
            }
            m_i915Context[i]->pOsContext = osParameters;
        }
    }

    return eStatus;
}

MOS_STATUS GpuContextSpecificNextXe::InitBltCtx(PMOS_CONTEXT osParameters,
                unsigned int *nengine,
                void *engine_map)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    __u16 engine_class = DRM_XE_ENGINE_CLASS_COPY;
    __u64 caps = 0;
    uint8_t ctxWidth = 1;
    uint8_t numPlacement = 1;

    if (mos_query_engines(osParameters->bufmgr, engine_class, caps, nengine, engine_map))
    {
        MOS_OS_ASSERTMESSAGE("Failed to query engines.");
        return MOS_STATUS_UNKNOWN;
    }
    numPlacement = *nengine;
    if (numPlacement >= 2)
    {
        // only use BCS0. BCS8 is paging copy
        numPlacement = 1;
    }

    m_i915Context[0] = mos_context_create_shared(osParameters->bufmgr,
                                             nullptr,
                                             0,
                                             m_bProtectedContext,
                                             engine_map,
                                             ctxWidth,
                                             numPlacement,
                                             0);
    if (m_i915Context[0] == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to create context.");
        return MOS_STATUS_UNKNOWN;
    }
    m_i915Context[0]->pOsContext = osParameters;

    return eStatus;
}

MOS_STATUS GpuContextSpecificNextXe::PatchCommandBuffer(
    MOS_STREAM_HANDLE   streamState,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_OS_FUNCTION_ENTER;

    auto perStreamParameters = (PMOS_CONTEXT)streamState->perStreamParameters;
    auto cmd_bo     = cmdBuffer->OsResource.bo;
    std::vector<PMOS_RESOURCE> mappedResList;

    // Now, the patching will be done, based on the patch list.
    for (uint32_t patchIndex = 0; patchIndex < m_currentNumPatchLocations; patchIndex++)
    {
        auto currentPatch = &m_patchLocationList[patchIndex];
        MOS_OS_CHK_NULL_RETURN(currentPatch);

        /**
         * Indicate whether temCmdBo is cmdbuffer in vector of m_secondaryCmdBufs;
         * If yes, tempCmdBo is also a primary cmd bo for scalability; And in this case, we need to
         * update its execlist.
         * Otherwise if no and temCmdBo != cmdBuffer->OsResource.bo, it should be second level
         * batch buffer for primary cmdbuffer. And in this case, we should not update its execlist.
         */
        bool isSecondaryCmdBuf = false;
        auto tempCmdBo = currentPatch->cmdBo == nullptr ? cmd_bo : currentPatch->cmdBo;

        // Following are for Nested BB buffer, if it's nested BB, we need to ensure it's locked.
        if (tempCmdBo != cmd_bo)
        {
            auto it = m_secondaryCmdBufs.begin();
            while(it != m_secondaryCmdBufs.end())
            {
                if (it->second->OsResource.bo == tempCmdBo)
                {
                    isSecondaryCmdBuf = true;
                    break;
                }
                it++;
            }

            for(auto allocIdx = 0; allocIdx < m_numAllocations && (!isSecondaryCmdBuf); allocIdx++)
            {
                auto tempRes = (PMOS_RESOURCE)m_allocationList[allocIdx].hAllocation;
                if (tempCmdBo == tempRes->bo)
                {
                    GraphicsResourceNext::LockParams param;
                    param.m_writeRequest = true;
                    tempRes->pGfxResourceNext->Lock(m_osContext, param);
                    mappedResList.push_back(tempRes);
                    break;
                }
            }
        }

        // This is the resource for which patching will be done
        auto resource = (PMOS_RESOURCE)m_allocationList[currentPatch->AllocationIndex].hAllocation;
        MOS_OS_CHK_NULL_RETURN(resource);

        // For now, we'll assume the system memory's DRM bo pointer
        // is NULL.  If nullptr is detected, then the resource has been
        // placed inside the command buffer's indirect state area.
        // We'll simply set alloc_bo to the command buffer's bo pointer.
        MOS_OS_ASSERT(resource->bo);

        auto alloc_bo = (resource->bo) ? resource->bo : tempCmdBo;

        MOS_OS_CHK_STATUS_RETURN(streamState->osCpInterface->PermeatePatchForHM(
            tempCmdBo->virt,
            currentPatch,
            resource));

        uint64_t boOffset = alloc_bo->offset64;

        MOS_OS_CHK_NULL_RETURN(tempCmdBo->virt);

        if (perStreamParameters->bUse64BitRelocs)
        {
            *((uint64_t *)((uint8_t *)tempCmdBo->virt + currentPatch->PatchOffset)) =
                    boOffset + currentPatch->AllocationOffset;
        }
        else
        {
            *((uint32_t *)((uint8_t *)tempCmdBo->virt + currentPatch->PatchOffset)) =
                    boOffset + currentPatch->AllocationOffset;
        }

#if (_DEBUG || _RELEASE_INTERNAL)
        {
            uint32_t evtData[] = {alloc_bo->handle, currentPatch->uiWriteOperation, currentPatch->AllocationOffset};
            MOS_TraceEventExt(EVENT_MOS_BATCH_SUBMIT, EVENT_TYPE_INFO,
                              evtData, sizeof(evtData),
                              &boOffset, sizeof(boOffset));
        }
#endif

        if(tempCmdBo != alloc_bo)
        {
            // reuse this api to update exec list in cmd bo
            mos_bo_add_softpin_target(isSecondaryCmdBuf ? tempCmdBo : cmd_bo, alloc_bo, currentPatch->uiWriteOperation);
        }
    }

    for(auto res: mappedResList)
    {
        res->pGfxResourceNext->Unlock(m_osContext);
    }
    mappedResList.clear();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GpuContextSpecificNextXe::EndSubmitCommandBuffer(
    MOS_STREAM_HANDLE   streamState,
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                cmdBufMapIsReused)
{
    MOS_OS_FUNCTION_ENTER;

    auto it = m_secondaryCmdBufs.begin();
#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    pthread_mutex_lock(&command_dump_mutex);
    if (streamState->dumpCommandBuffer)
    {
        while(it != m_secondaryCmdBufs.end())
        {
            MosInterface::DumpCommandBuffer(streamState, it->second);
            it++;
        }
    }
    pthread_mutex_unlock(&command_dump_mutex);
#endif  // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

    for (uint32_t patchIndex = 0; patchIndex < m_currentNumPatchLocations; patchIndex++)
    {
        auto currentPatch = &m_patchLocationList[patchIndex];
        MOS_OS_CHK_NULL_RETURN(currentPatch);

        // reuse this api to clear exec list in cmd bo
        if(currentPatch->cmdBo)
            mos_bo_clear_relocs(currentPatch->cmdBo, 0);
    }
    // Now, we can unmap the video command buffer, since we don't need CPU access anymore.
    if (!cmdBufMapIsReused && cmdBuffer->OsResource.pGfxResourceNext)
    {
        cmdBuffer->OsResource.pGfxResourceNext->Unlock(m_osContext);
    }
    ClearSecondaryCmdBuffer(cmdBufMapIsReused);

    // Reset resource allocation
    m_numAllocations = 0;
    MosUtilities::MosZeroMemory(m_allocationList, sizeof(ALLOCATION_LIST) * m_maxNumAllocations);
    m_currentNumPatchLocations = 0;
    MosUtilities::MosZeroMemory(m_patchLocationList, sizeof(PATCHLOCATIONLIST) * m_maxNumAllocations);
    m_resCount = 0;

    MosUtilities::MosZeroMemory(m_writeModeList, sizeof(bool) * m_maxNumAllocations);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GpuContextSpecificNextXe::SubmitCommandBuffer(
    MOS_STREAM_HANDLE   streamState,
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                nullRendering)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_TraceEventExt(EVENT_MOS_BATCH_SUBMIT, EVENT_TYPE_START, nullptr, 0, nullptr, 0);

    MOS_OS_CHK_NULL_RETURN(streamState);
    auto perStreamParameters = (PMOS_CONTEXT)streamState->perStreamParameters;
    MOS_OS_CHK_NULL_RETURN(perStreamParameters);
    MOS_OS_CHK_NULL_RETURN(cmdBuffer);
    MOS_OS_CHK_NULL_RETURN(m_patchLocationList);

    MOS_GPU_NODE gpuNode  = OSKMGetGpuNode(m_gpuContext);
    uint32_t     execFlag = gpuNode;
    MOS_STATUS   eStatus  = MOS_STATUS_SUCCESS;
    int32_t      ret      = 0;
    bool         scalaEnabled      = false;
    bool         cmdBufMapIsReused = false;
    auto         it                = m_secondaryCmdBufs.begin();

    m_cmdBufFlushed = true;
    auto cmd_bo     = cmdBuffer->OsResource.bo;

    if (m_secondaryCmdBufs.size() >= 2)
    {
        scalaEnabled = true;
        cmdBuffer->iSubmissionType = SUBMISSION_TYPE_MULTI_PIPE_MASTER;
    }

    // Patch cmdbuffer addr
    MOS_OS_CHK_STATUS_RETURN(PatchCommandBuffer(streamState, cmdBuffer));

    int32_t perfData = perStreamParameters->pPerfData != nullptr ? *(int32_t *)(perStreamParameters->pPerfData) : 0;

    int32_t DR4 = perStreamParameters->uEnablePerfTag ? perfData : 0;

    if (gpuNode != I915_EXEC_RENDER &&
        streamState->osCpInterface->IsTearDownHappen())
    {
        // skip PAK command when CP tear down happen to avoid of GPU hang
        // conditonal batch buffer start PoC is in progress
    }
    else if (nullRendering == false)
    {
        UnlockPendingOcaBuffers(cmdBuffer, perStreamParameters);
        if (streamState->ctxBasedScheduling && m_i915Context[0] != nullptr)
        {
            //For multipipe, FE, BEs bb are all in m_secondaryCmdBufs;
            //For single pipe, reuse m_secondaryCmdBufs and add cmdBuffer in it.
            if(!scalaEnabled)
            {
                // Need to resue m_secondaryCmdBufs
                ClearSecondaryCmdBuffer(cmdBufMapIsReused);
                m_secondaryCmdBufs[0] = cmdBuffer;
                cmdBufMapIsReused = true;
            }

            ret = ParallelSubmitCommands(m_secondaryCmdBufs,
                                 perStreamParameters,
                                 execFlag,
                                 DR4);
        }

        if (ret != 0)
        {
            eStatus = MOS_STATUS_UNKNOWN;
        }
    }

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Command buffer submission failed!");
    }

    MosUtilDevUltSpecific::MOS_DEVULT_FuncCall(pfnUltGetCmdBuf, cmdBuffer);

    // Reset global variable
    MOS_OS_CHK_STATUS_RETURN(EndSubmitCommandBuffer(streamState, cmdBuffer, cmdBufMapIsReused));

    MOS_TraceEventExt(EVENT_MOS_BATCH_SUBMIT, EVENT_TYPE_END, &eStatus, sizeof(eStatus), nullptr, 0);
    return eStatus;
}

/**
 * Both of single pipe and multi-pipe are submitted through this interface
 * FE and BE sync through bo.deps since they use different ctx
 */
int32_t GpuContextSpecificNextXe::ParallelSubmitCommands(
    std::map<uint32_t, PMOS_COMMAND_BUFFER> secondaryCmdBufs,
    PMOS_CONTEXT osContext,
    uint32_t execFlag,
    int32_t dr4)
{
    MOS_OS_FUNCTION_ENTER;

    int32_t      ret        = 0;
    int          fence      = -1;
    auto         it         = m_secondaryCmdBufs.begin();
    MOS_LINUX_BO *cmdBos[MAX_PARALLEN_CMD_BO_NUM];
    int          numBatch     = 0;
    uint32_t batchBufferEndCmd = MI_BATCHBUFFER_END;

    MOS_LINUX_CONTEXT *queue = m_i915Context[0];

    while(it != m_secondaryCmdBufs.end())
    {
        MapResourcesToAuxTable(it->second->OsResource.bo);
        // Add Batch buffer End Command
        if (MOS_FAILED(Mos_AddCommand(
                        it->second,
                        &batchBufferEndCmd,
                        sizeof(uint32_t))))
        {
            MOS_OS_ASSERTMESSAGE("Inserting BB_END failed!");
            return MOS_STATUS_UNKNOWN;
        }

        if(it->second->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_ALONE
                    || it->second->iSubmissionType & SUBMISSION_TYPE_SINGLE_PIPE
                    || m_secondaryCmdBufs.size() == 1)
        {
            queue = m_i915Context[0];
            MOS_OS_CHK_NULL_RETURN(queue);
            numBatch = 1;
            cmdBos[0] = it->second->OsResource.bo;

            ret = mos_bo_context_exec3(cmdBos,
                                  numBatch,
                                  queue,
                                  nullptr,
                                  0,
                                  dr4,
                                  execFlag, // not used
                                  &fence); // not used
            cmdBos[0] = nullptr;
            numBatch = 0;
        }

        if((it->second->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_MASTER)
            || (it->second->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_SLAVE))
        {
            cmdBos[numBatch++] = it->second->OsResource.bo;

            if(it->second->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_FLAGS_LAST_PIPE)
            {
                queue = m_i915Context[numBatch - 1];
                MOS_OS_CHK_NULL_RETURN(queue);

                ret = mos_bo_context_exec3(cmdBos,
                                              numBatch,
                                              queue,
                                              nullptr,
                                              0,
                                              dr4,
                                              execFlag,
                                              &fence);

                for(int i = 0; i < numBatch; i++)
                {
                    cmdBos[i] = nullptr;
                }
                numBatch = 0;
            }
        }
        it++;
    }

    return ret;
}

void GpuContextSpecificNextXe::UpdatePriority(int32_t priority)
{
    MOS_OS_FUNCTION_ENTER;
    //Note: need to implement this bufmgr api to set exec queue property
}

MOS_STATUS GpuContextSpecificNextXe::ReportEngineInfo(
        void *engine_map,
        int engineNum, bool engineSelectEnable)
{
    MOS_UNUSED(engine_map);
    MOS_UNUSED(engineNum);
    MOS_UNUSED(engineSelectEnable);
    return MOS_STATUS_SUCCESS;
}

void GpuContextSpecificNextXe::ClearSecondaryCmdBuffer(
    bool cmdBufMapIsReused)
{
    auto it = m_secondaryCmdBufs.begin();
    while (it != m_secondaryCmdBufs.end())
    {
        if (it->second->OsResource.pGfxResourceNext)
        {
            it->second->OsResource.pGfxResourceNext->Unlock(m_osContext);
        }
        if (!cmdBufMapIsReused)
        {
            MOS_FreeMemory(it->second);
        }
        it++;
    }
    m_secondaryCmdBufs.clear();
}
