/*
* Copyright (c) 2018-2019, Intel Corporation
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
//! \file    mos_gpucontext_specific.cpp
//! \brief   Container class for the Linux specific gpu context
//!

#include "mos_context_specific.h"
#include "mos_gpucontext_specific.h"
#include "mos_graphicsresource_specific.h"
#include "mos_commandbuffer_specific.h"
#include "mos_util_devult_specific.h"
#include "mos_cmdbufmgr.h"
#include "mos_os_virtualengine.h"
#include <unistd.h>

#define MI_BATCHBUFFER_END 0x05000000
static pthread_mutex_t command_dump_mutex = PTHREAD_MUTEX_INITIALIZER;

GpuContextSpecific::GpuContextSpecific(
    const MOS_GPU_NODE gpuNode,
    MOS_GPU_CONTEXT    mosGpuCtx,
    CmdBufMgr         *cmdBufMgr,
    GpuContext        *reusedContext)
{
    MOS_OS_FUNCTION_ENTER;

    m_nodeOrdinal          = gpuNode;
    m_cmdBufMgr            = cmdBufMgr;
    m_gpuContext           = mosGpuCtx;
    m_statusBufferResource = nullptr;
    m_maxPatchLocationsize = PATCHLOCATIONLIST_SIZE;

    if (reusedContext)
    {
        MOS_OS_NORMALMESSAGE("gpucontex reusing not enabled on Linux.");
    }
}

GpuContextSpecific::~GpuContextSpecific()
{
    MOS_OS_FUNCTION_ENTER;

    Clear();
}

MOS_STATUS GpuContextSpecific::Init(OsContext *osContext,
                    PMOS_INTERFACE osInterface,
                    MOS_GPU_NODE GpuNode,
                    PMOS_GPUCTX_CREATOPTIONS createOption)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osContext);

    if (m_cmdBufPoolMutex == nullptr)
    {
        m_cmdBufPoolMutex = MOS_CreateMutex();
    }

    MOS_OS_CHK_NULL_RETURN(m_cmdBufPoolMutex);

    MOS_LockMutex(m_cmdBufPoolMutex);

    m_cmdBufPool.clear();

    MOS_UnlockMutex(m_cmdBufPoolMutex);

    m_commandBufferSize = COMMAND_BUFFER_SIZE;

    m_nextFetchIndex = 0;

    m_cmdBufFlushed = true;

    m_osContext = osContext;

    MOS_OS_CHK_STATUS_RETURN(AllocateGPUStatusBuf());

    m_commandBuffer = (PMOS_COMMAND_BUFFER)MOS_AllocAndZeroMemory(sizeof(MOS_COMMAND_BUFFER));

    MOS_OS_CHK_NULL_RETURN(m_commandBuffer);

    m_IndirectHeapSize = 0;

    // each thread has its own GPU context, so do not need any lock as guarder here
    m_allocationList = (ALLOCATION_LIST *)MOS_AllocAndZeroMemory(sizeof(ALLOCATION_LIST) * ALLOCATIONLIST_SIZE);
    MOS_OS_CHK_NULL_RETURN(m_allocationList);
    m_maxNumAllocations = ALLOCATIONLIST_SIZE;

    m_patchLocationList = (PATCHLOCATIONLIST *)MOS_AllocAndZeroMemory(sizeof(PATCHLOCATIONLIST) * PATCHLOCATIONLIST_SIZE);
    MOS_OS_CHK_NULL_RETURN(m_patchLocationList);
    m_maxPatchLocationsize = PATCHLOCATIONLIST_SIZE;

    m_attachedResources = (PMOS_RESOURCE)MOS_AllocAndZeroMemory(sizeof(MOS_RESOURCE) * ALLOCATIONLIST_SIZE);
    MOS_OS_CHK_NULL_RETURN(m_attachedResources);

    m_writeModeList = (bool *)MOS_AllocAndZeroMemory(sizeof(bool) * ALLOCATIONLIST_SIZE);
    MOS_OS_CHK_NULL_RETURN(m_writeModeList);

    m_GPUStatusTag = 1;

    m_createOptionEnhanced = (MOS_GPUCTX_CREATOPTIONS_ENHANCED*)MOS_AllocAndZeroMemory(sizeof(MOS_GPUCTX_CREATOPTIONS_ENHANCED));
    MOS_OS_CHK_NULL_RETURN(m_createOptionEnhanced);
    m_createOptionEnhanced->SSEUValue = createOption->SSEUValue;

    if (typeid(*createOption) == typeid(MOS_GPUCTX_CREATOPTIONS_ENHANCED))
    {
        PMOS_GPUCTX_CREATOPTIONS_ENHANCED createOptionEnhanced = static_cast<PMOS_GPUCTX_CREATOPTIONS_ENHANCED>(createOption);
        m_createOptionEnhanced->UsingSFC = createOptionEnhanced->UsingSFC;
    }

    for (int i=0; i<MAX_ENGINE_INSTANCE_NUM+1; i++)
    {
        m_i915Context[i] = nullptr;
    }

    if (osInterface->ctxBasedScheduling)
    {
        m_i915Context[0] = mos_gem_context_create_shared(osInterface->pOsContext->bufmgr,
                                             osInterface->pOsContext->intel_context,
                                             I915_CONTEXT_CREATE_FLAGS_SINGLE_TIMELINE);
        if (m_i915Context[0] == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Failed to create context.\n");
            return MOS_STATUS_UNKNOWN;
        }
        m_i915Context[0]->pOsContext = osInterface->pOsContext;

        m_i915ExecFlag = I915_EXEC_DEFAULT;
        if (GpuNode == MOS_GPU_NODE_3D || GpuNode == MOS_GPU_NODE_COMPUTE)
        {
            struct i915_engine_class_instance engine_map;
            engine_map.engine_class = I915_ENGINE_CLASS_RENDER;
            engine_map.engine_instance = 0;

            if (mos_set_context_param_load_balance(m_i915Context[0],&engine_map, 1))
            {
                MOS_OS_ASSERTMESSAGE("Failed to set balancer extension.\n");
                return MOS_STATUS_UNKNOWN;
            }

            if (createOption->SSEUValue != 0)
            {
                struct drm_i915_gem_context_param_sseu sseu;
                MOS_ZeroMemory(&sseu, sizeof(sseu));
                sseu.flags = I915_CONTEXT_SSEU_FLAG_ENGINE_INDEX;
                sseu.engine.engine_instance = m_i915ExecFlag;

                if (mos_get_context_param_sseu(m_i915Context[0], &sseu))
                {
                    MOS_OS_ASSERTMESSAGE("Failed to get sseu configuration.");
                    return MOS_STATUS_UNKNOWN;
                }

                if (mos_hweight8(sseu.subslice_mask) > createOption->packed.SubSliceCount)
                {
                    sseu.subslice_mask = mos_switch_off_n_bits(sseu.subslice_mask,
                            mos_hweight8(sseu.subslice_mask)-createOption->packed.SubSliceCount);
                }

                if (mos_set_context_param_sseu(m_i915Context[0], sseu))
                {
                    MOS_OS_ASSERTMESSAGE("Failed to set sseu configuration.");
                    return MOS_STATUS_UNKNOWN;
                }
            }
        }
        else if (GpuNode == MOS_GPU_NODE_VIDEO || GpuNode == MOS_GPU_NODE_VIDEO2
                 || GpuNode == MOS_GPU_NODE_VE)
        {
            unsigned int nengine = MAX_ENGINE_INSTANCE_NUM;
            struct i915_engine_class_instance engine_map[MAX_ENGINE_INSTANCE_NUM];
            __u16 engine_class = (GpuNode == MOS_GPU_NODE_VE)? I915_ENGINE_CLASS_VIDEO_ENHANCE : I915_ENGINE_CLASS_VIDEO;
            __u64 caps = 0;

            if (m_createOptionEnhanced->UsingSFC)
            {
                caps |= I915_VIDEO_AND_ENHANCE_CLASS_CAPABILITY_SFC;
            }

            MOS_ZeroMemory(engine_map, sizeof(engine_map));
            if (mos_query_engines(osInterface->pOsContext->fd,engine_class,caps,&nengine,engine_map))
            {
                MOS_OS_ASSERTMESSAGE("Failed to query engines.\n");
                return MOS_STATUS_UNKNOWN;
            }

            if (mos_set_context_param_load_balance(m_i915Context[0], engine_map, nengine))
            {
                MOS_OS_ASSERTMESSAGE("Failed to set balancer extension.\n");
                return MOS_STATUS_UNKNOWN;
            }

            if (nengine >= 2)
            {
                //master queue
                m_i915Context[1] = mos_gem_context_create_shared(osInterface->pOsContext->bufmgr,
                                                                 osInterface->pOsContext->intel_context,
                                                                 I915_CONTEXT_CREATE_FLAGS_SINGLE_TIMELINE);
                if (m_i915Context[1] == nullptr)
                {
                    MOS_OS_ASSERTMESSAGE("Failed to create master context.\n");
                    return MOS_STATUS_UNKNOWN;
                }
                m_i915Context[1]->pOsContext = osInterface->pOsContext;

                if (mos_set_context_param_load_balance(m_i915Context[1], engine_map, 1))
                {
                    MOS_OS_ASSERTMESSAGE("Failed to set master context bond extension.\n");
                    return MOS_STATUS_UNKNOWN;
                }

                //slave queue
                for (int i=1; i<nengine; i++)
                {
                    m_i915Context[i+1] = mos_gem_context_create_shared(osInterface->pOsContext->bufmgr,
                                                                     osInterface->pOsContext->intel_context,
                                                                     I915_CONTEXT_CREATE_FLAGS_SINGLE_TIMELINE);
                    if (m_i915Context[i+1] == nullptr)
                    {
                        MOS_OS_ASSERTMESSAGE("Failed to create slave context.\n");
                        return MOS_STATUS_UNKNOWN;
                    }
                    m_i915Context[i+1]->pOsContext = osInterface->pOsContext;

                    if (mos_set_context_param_bond(m_i915Context[i+1], engine_map[0],&engine_map[i], 1))
                    {
                        MOS_OS_ASSERTMESSAGE("Failed to set slave context bond extension.\n");
                        return MOS_STATUS_UNKNOWN;
                    }
                }
            }
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Unknown engine class.\n");
            return MOS_STATUS_UNKNOWN;
        }
    }
    return MOS_STATUS_SUCCESS;
}

void GpuContextSpecific::Clear()
{
    MOS_OS_FUNCTION_ENTER;

    // hanlde the status buf bundled w/ the specified gpucontext
    if (m_statusBufferResource)
    {
        if (m_statusBufferResource->Unlock(m_osContext) != MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("failed to unlock the status buf bundled w/ the specified gpucontext");
        }
        m_statusBufferResource->Free(m_osContext, 0);
        MOS_Delete(m_statusBufferResource);
    }

    MOS_LockMutex(m_cmdBufPoolMutex);

    if (m_cmdBufMgr)
    {
        for (auto& curCommandBuffer : m_cmdBufPool)
        {
            auto curCommandBufferSpecific = static_cast<CommandBufferSpecific *>(curCommandBuffer);
            if (curCommandBufferSpecific == nullptr)
                continue;
            curCommandBufferSpecific->waitReady(); // wait ready and return to comamnd buffer manager.
            m_cmdBufMgr->ReleaseCmdBuf(curCommandBuffer);
        }
    }

    m_cmdBufPool.clear();

    MOS_UnlockMutex(m_cmdBufPoolMutex);
    MOS_DestroyMutex(m_cmdBufPoolMutex);
    m_cmdBufPoolMutex = nullptr;
    MOS_SafeFreeMemory(m_commandBuffer);
    MOS_SafeFreeMemory(m_allocationList);
    MOS_SafeFreeMemory(m_patchLocationList);
    MOS_SafeFreeMemory(m_attachedResources);
    MOS_SafeFreeMemory(m_writeModeList);
    MOS_SafeFreeMemory(m_createOptionEnhanced);

    for (int i=0; i<MAX_ENGINE_INSTANCE_NUM; i++)
    {
        if (m_i915Context[i])
        {
            mos_gem_context_destroy(m_i915Context[i]);
            m_i915Context[i] = nullptr;
        }
    }
}

MOS_STATUS GpuContextSpecific::RegisterResource(
    PMOS_RESOURCE osResource,
    bool          writeFlag)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osResource);

    MOS_OS_CHK_NULL_RETURN(m_attachedResources);

    PMOS_RESOURCE registeredResources = m_attachedResources;
    uint32_t      allocationIndex     = 0;

    for ( allocationIndex = 0; allocationIndex < m_resCount; allocationIndex++, registeredResources++)
    {
        if (osResource->bo == registeredResources->bo)
        {
            break;
        }
    }

    // Allocation list to be updated
    if (allocationIndex < m_maxNumAllocations)
    {
        // New buffer
        if (allocationIndex == m_resCount)
        {
            m_resCount++;
        }

        // Set allocation
        if (m_gpuContext >= MOS_GPU_CONTEXT_MAX)
        {
            MOS_OS_ASSERTMESSAGE("Gpu context exceeds max.");
            return MOS_STATUS_UNKNOWN; 
        }

        osResource->iAllocationIndex[m_gpuContext] = (allocationIndex);
        m_attachedResources[allocationIndex]           = *osResource;
        m_writeModeList[allocationIndex] |= writeFlag;
        m_allocationList[allocationIndex].hAllocation = &m_attachedResources[allocationIndex];
        m_allocationList[allocationIndex].WriteOperation |= writeFlag;
        m_numAllocations = m_resCount;
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Reached max # registrations.");
        return MOS_STATUS_UNKNOWN;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GpuContextSpecific::SetPatchEntry(
    PMOS_INTERFACE          osInterface,
    PMOS_PATCH_ENTRY_PARAMS params)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(m_patchLocationList);
    MOS_OS_CHK_NULL_RETURN(osInterface);
    MOS_OS_CHK_NULL_RETURN(params);

    m_patchLocationList[m_currentNumPatchLocations].AllocationIndex  = params->uiAllocationIndex;
    m_patchLocationList[m_currentNumPatchLocations].AllocationOffset = params->uiResourceOffset;
    m_patchLocationList[m_currentNumPatchLocations].PatchOffset      = params->uiPatchOffset;
    m_patchLocationList[m_currentNumPatchLocations].uiWriteOperation = params->bWrite ? true: false;
    m_patchLocationList[m_currentNumPatchLocations].cmdBo            =
                params->cmdBuffer != nullptr ? params->cmdBuffer->OsResource.bo : nullptr;

    if (osInterface->osCpInterface &&
        osInterface->osCpInterface->IsHMEnabled())
    {
        if (MOS_STATUS_SUCCESS != osInterface->osCpInterface->RegisterPatchForHM(
            (uint32_t *)(params->cmdBufBase + params->uiPatchOffset),
            params->bWrite,
            params->HwCommandType,
            params->forceDwordOffset,
            params->presResource,
            &m_patchLocationList[m_currentNumPatchLocations]))
        {
            MOS_OS_ASSERTMESSAGE("Failed to RegisterPatchForHM.");
        }
    }

    m_currentNumPatchLocations++;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GpuContextSpecific::GetCommandBuffer(
    PMOS_COMMAND_BUFFER comamndBuffer,
    uint32_t            flags)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(comamndBuffer);
    MOS_OS_CHK_NULL_RETURN(m_cmdBufMgr);
    MOS_OS_CHK_NULL_RETURN(m_commandBuffer);

    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;
    CommandBuffer* cmdBuf = nullptr;

    uint32_t secondaryIdx = flags;
    bool isPrimaryCmdBuffer = (secondaryIdx == 0);
    bool hasSecondaryCmdBuffer = (!isPrimaryCmdBuffer &&
                               (m_secondaryCmdBufs.count(secondaryIdx) != 0));

    bool needToAlloc = ((isPrimaryCmdBuffer && m_cmdBufFlushed) ||
                        (!isPrimaryCmdBuffer && !hasSecondaryCmdBuffer));

    if (needToAlloc)
    {
        MOS_LockMutex(m_cmdBufPoolMutex);
        if (m_cmdBufPool.size() < MAX_CMD_BUF_NUM)
        {
            cmdBuf = m_cmdBufMgr->PickupOneCmdBuf(m_commandBufferSize);
            if (cmdBuf == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("Invalid (nullptr) Pointer.");
                MOS_UnlockMutex(m_cmdBufPoolMutex);
                return MOS_STATUS_NULL_POINTER;
            }
            if ((eStatus = cmdBuf->BindToGpuContext(this)) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Invalid status of BindToGpuContext.");
                MOS_UnlockMutex(m_cmdBufPoolMutex);
                return eStatus;
            }
            m_cmdBufPool.push_back(cmdBuf);
        }
        else if (m_cmdBufPool.size() == MAX_CMD_BUF_NUM && m_nextFetchIndex < m_cmdBufPool.size())
        {
            auto cmdBufOld = m_cmdBufPool[m_nextFetchIndex];
            auto cmdBufSpecificOld = static_cast<CommandBufferSpecific *>(cmdBufOld);
            if (cmdBufSpecificOld == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("Invalid (nullptr) Pointer.");
                MOS_UnlockMutex(m_cmdBufPoolMutex);
                return MOS_STATUS_NULL_POINTER;
            }
            cmdBufSpecificOld->waitReady();
            cmdBufSpecificOld->UnBindToGpuContext();
            m_cmdBufMgr->ReleaseCmdBuf(cmdBufOld);  // here just return old command buffer to available pool

            //pick up new comamnd buffer
            cmdBuf = m_cmdBufMgr->PickupOneCmdBuf(m_commandBufferSize);
            if (cmdBuf == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("Invalid (nullptr) Pointer.");
                MOS_UnlockMutex(m_cmdBufPoolMutex);
                return MOS_STATUS_NULL_POINTER;
            }
            if ((eStatus = cmdBuf->BindToGpuContext(this)) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Invalid status of BindToGpuContext.");
                MOS_UnlockMutex(m_cmdBufPoolMutex);
                return eStatus;
            }
            m_cmdBufPool[m_nextFetchIndex] = cmdBuf;
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Command buffer bool size exceed max.");
            MOS_UnlockMutex(m_cmdBufPoolMutex);
            return MOS_STATUS_UNKNOWN;
        }
        MOS_UnlockMutex(m_cmdBufPoolMutex);

        // util now, we got new command buffer from CmdBufMgr, next step to fill in the input command buffer
        MOS_OS_CHK_STATUS_RETURN(cmdBuf->GetResource()->ConvertToMosResource(&comamndBuffer->OsResource));
        comamndBuffer->pCmdBase   = (uint32_t *)cmdBuf->GetLockAddr();
        comamndBuffer->pCmdPtr    = (uint32_t *)cmdBuf->GetLockAddr();
        comamndBuffer->iOffset    = 0;
        comamndBuffer->iRemaining = cmdBuf->GetCmdBufSize();
        comamndBuffer->iCmdIndex  = m_nextFetchIndex;
        comamndBuffer->iVdboxNodeIndex = MOS_VDBOX_NODE_INVALID;
        comamndBuffer->iVeboxNodeIndex = MOS_VEBOX_NODE_INVALID;
        comamndBuffer->Attributes.pAttriVe = nullptr;

        // zero comamnd buffer
        MOS_ZeroMemory(comamndBuffer->pCmdBase, comamndBuffer->iRemaining);
        comamndBuffer->iSubmissionType = SUBMISSION_TYPE_SINGLE_PIPE;
        MOS_ZeroMemory(&comamndBuffer->Attributes,sizeof(comamndBuffer->Attributes));

        if (isPrimaryCmdBuffer)
        {
            // update command buffer relared filed in GPU context
            m_cmdBufFlushed = false;

            // keep a copy in GPU context
            MOS_SecureMemcpy(m_commandBuffer, sizeof(MOS_COMMAND_BUFFER), comamndBuffer, sizeof(MOS_COMMAND_BUFFER));
        }
        else
        {
            PMOS_COMMAND_BUFFER tempCmdBuf = (PMOS_COMMAND_BUFFER)MOS_AllocAndZeroMemory(sizeof(MOS_COMMAND_BUFFER));
            MOS_OS_CHK_NULL_RETURN(tempCmdBuf);
            m_secondaryCmdBufs[secondaryIdx] = tempCmdBuf;
            MOS_SecureMemcpy(tempCmdBuf, sizeof(MOS_COMMAND_BUFFER), comamndBuffer, sizeof(MOS_COMMAND_BUFFER));
        }

        // Command buffers are treated as cyclical buffers, the CB after the just submitted one
        // has the minimal fence value that we should wait
        m_nextFetchIndex++;
        if (m_nextFetchIndex >= MAX_CMD_BUF_NUM)
        {
            m_nextFetchIndex = 0;
        }
    }
    else
    {
        // current command buffer still active, directly copy to comamndBuffer
        if (isPrimaryCmdBuffer)
        {
            MOS_SecureMemcpy(comamndBuffer, sizeof(MOS_COMMAND_BUFFER), m_commandBuffer, sizeof(MOS_COMMAND_BUFFER));
        }
        else
        {
            MOS_SecureMemcpy(comamndBuffer, sizeof(MOS_COMMAND_BUFFER), m_secondaryCmdBufs[secondaryIdx], sizeof(MOS_COMMAND_BUFFER));
        }
    }

    if (isPrimaryCmdBuffer)
    {
        MOS_OS_CHK_STATUS_RETURN(RegisterResource(&m_commandBuffer->OsResource, false));
    }
    else
    {
        MOS_OS_CHK_STATUS_RETURN(RegisterResource(&m_secondaryCmdBufs[secondaryIdx]->OsResource, false));
    }

    return MOS_STATUS_SUCCESS;
}

void GpuContextSpecific::ReturnCommandBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer,
    uint32_t            flags)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_ASSERT(cmdBuffer);
    MOS_OS_ASSERT(m_commandBuffer);

    bool isPrimaryCmdBuf = (flags == 0);

    if (isPrimaryCmdBuf)
    {
        m_commandBuffer->iOffset    = cmdBuffer->iOffset;
        m_commandBuffer->iRemaining = cmdBuffer->iRemaining;
        m_commandBuffer->pCmdPtr    = cmdBuffer->pCmdPtr;
        m_commandBuffer->iVdboxNodeIndex = cmdBuffer->iVdboxNodeIndex;
        m_commandBuffer->iVeboxNodeIndex = cmdBuffer->iVeboxNodeIndex;
    }
    else
    {
        uint32_t secondaryIdx = flags;
        MOS_OS_ASSERT(m_secondaryCmdBufs.count(secondaryIdx));

        MOS_SecureMemcpy(m_secondaryCmdBufs[secondaryIdx], sizeof(MOS_COMMAND_BUFFER), cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
    }
}

MOS_STATUS GpuContextSpecific::ResetCommandBuffer()
{
    m_cmdBufFlushed = true;
    auto it = m_secondaryCmdBufs.begin();
    while(it != m_secondaryCmdBufs.end())
    {
        MOS_FreeMemory(it->second);
        it++;
    }
    m_secondaryCmdBufs.clear();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GpuContextSpecific::SetIndirectStateSize(const uint32_t size)
{
    if(size < m_commandBufferSize)
    {
        m_IndirectHeapSize = size;
        return MOS_STATUS_SUCCESS;
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Indirect State Size if out of boundry!");
        return MOS_STATUS_UNKNOWN;
    }
}

MOS_STATUS GpuContextSpecific::GetIndirectState(
    uint32_t *offset,
    uint32_t *size)
{
    MOS_OS_FUNCTION_ENTER;

    if (offset)
    {
        *offset = m_commandBufferSize - m_IndirectHeapSize;
    }

    if (size)
    {
        *size = m_IndirectHeapSize;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GpuContextSpecific::GetIndirectStatePointer(
    uint8_t **indirectState)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(indirectState);

    *indirectState = (uint8_t *)m_commandBuffer->pCmdBase + m_commandBufferSize - m_IndirectHeapSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GpuContextSpecific::ResizeCommandBufferAndPatchList(
    uint32_t requestedCommandBufferSize,
    uint32_t requestedPatchListSize,
    uint32_t flags)
{
    MOS_OS_FUNCTION_ENTER;

    // m_commandBufferSize is used for allocate command buffer and submit command buffer, in this moment, command buffer has not allocated yet.
    // Linux KMD requires command buffer size align to 8 bytes, or it will not execute the commands.
    m_commandBufferSize = MOS_ALIGN_CEIL(requestedCommandBufferSize, 8);

    if (requestedPatchListSize > m_maxPatchLocationsize)
    {
        PPATCHLOCATIONLIST newPatchList = (PPATCHLOCATIONLIST)realloc(m_patchLocationList, sizeof(PATCHLOCATIONLIST) * requestedPatchListSize);
        MOS_OS_CHK_NULL_RETURN(newPatchList);

        m_patchLocationList = newPatchList;

        // now zero the extended portion
        MOS_ZeroMemory((m_patchLocationList + m_maxPatchLocationsize), sizeof(PATCHLOCATIONLIST) * (requestedPatchListSize - m_maxPatchLocationsize));
        m_maxPatchLocationsize = requestedPatchListSize;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GpuContextSpecific::ResizeCommandBuffer(uint32_t requestedSize)
{
    MOS_OS_FUNCTION_ENTER;

    m_commandBufferSize = requestedSize;

    return MOS_STATUS_SUCCESS;
}

uint32_t GetVcsExecFlag(PMOS_INTERFACE osInterface,
                            PMOS_COMMAND_BUFFER cmdBuffer,
                            MOS_GPU_NODE gpuNode)
{
    if (osInterface == 0 ||
        cmdBuffer == 0)
    {
        MOS_OS_ASSERTMESSAGE("Input invalid(null) parameter.");
        return I915_EXEC_DEFAULT;
    }

    uint32_t vcsExecFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING1;

    if (MOS_VDBOX_NODE_INVALID == cmdBuffer->iVdboxNodeIndex)
    {
       // That's those case when BB did not have any VDBOX# specific commands.
       // Thus, we need to select VDBOX# here. Alternatively we can rely on KMD
       // to make balancing for us, i.e. rely on Virtual Engine support.
       cmdBuffer->iVdboxNodeIndex = osInterface->pfnGetVdboxNodeId(osInterface, cmdBuffer);
       if (MOS_VDBOX_NODE_INVALID == cmdBuffer->iVdboxNodeIndex)
       {
           cmdBuffer->iVdboxNodeIndex = (gpuNode == MOS_GPU_NODE_VIDEO)?
               MOS_VDBOX_NODE_1: MOS_VDBOX_NODE_2;
       }
     }

     if (MOS_VDBOX_NODE_1 == cmdBuffer->iVdboxNodeIndex)
     {
         vcsExecFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING1;
     }
     else if (MOS_VDBOX_NODE_2 == cmdBuffer->iVdboxNodeIndex)
     {
         vcsExecFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING2;
     }

     return vcsExecFlag;
}

MOS_STATUS GpuContextSpecific::MapResourcesToAuxTable(mos_linux_bo *cmd_bo)
{
    MOS_OS_CHK_NULL_RETURN(cmd_bo);

    OsContextSpecific *osCtx = static_cast<OsContextSpecific*>(m_osContext);
    MOS_OS_CHK_NULL_RETURN(osCtx);

    AuxTableMgr *auxTableMgr = osCtx->GetAuxTableMgr();
    if (auxTableMgr)
    {
        // Map compress allocations to aux table if it is not mapped.
        for (uint32_t i = 0; i < m_numAllocations; i++)
        {
            auto res = (PMOS_RESOURCE)m_allocationList[i].hAllocation;
            MOS_OS_CHK_NULL_RETURN(res);
            MOS_OS_CHK_STATUS_RETURN(auxTableMgr->MapResource(res->pGmmResInfo, res->bo));
        }
        MOS_OS_CHK_STATUS_RETURN(auxTableMgr->EmitAuxTableBOList(cmd_bo));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GpuContextSpecific::SubmitCommandBuffer(
    PMOS_INTERFACE      osInterface,
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                nullRendering)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osInterface);
    PMOS_CONTEXT osContext = osInterface->pOsContext;
    MOS_OS_CHK_NULL_RETURN(osContext);
    MOS_OS_CHK_NULL_RETURN(cmdBuffer);
    MOS_OS_CHK_NULL_RETURN(m_patchLocationList);

    MOS_GPU_NODE gpuNode  = OSKMGetGpuNode(m_gpuContext);
    uint32_t     execFlag = gpuNode;
    MOS_STATUS   eStatus  = MOS_STATUS_SUCCESS;
    int32_t      ret      = 0;
    bool         scalaEnabled = false;
    auto         it           = m_secondaryCmdBufs.begin();

    // Command buffer object DRM pointer
    m_cmdBufFlushed = true;
    auto cmd_bo     = cmdBuffer->OsResource.bo;

    // Map Resource to Aux if needed
    MapResourcesToAuxTable(cmd_bo);
    for(auto it : m_secondaryCmdBufs)
    {
        MapResourcesToAuxTable(it.second->OsResource.bo);
    }

    if (m_secondaryCmdBufs.size() >= 2)
    {
        scalaEnabled = true;
        cmdBuffer->iSubmissionType = SUBMISSION_TYPE_MULTI_PIPE_MASTER;
    }

    std::vector<PMOS_RESOURCE> mappedResList;

    // Now, the patching will be done, based on the patch list.
    for (uint32_t patchIndex = 0; patchIndex < m_currentNumPatchLocations; patchIndex++)
    {
        auto currentPatch = &m_patchLocationList[patchIndex];
        MOS_OS_CHK_NULL_RETURN(currentPatch);

        auto tempCmdBo = currentPatch->cmdBo == nullptr ? cmd_bo : currentPatch->cmdBo;

        // Following are for Nested BB buffer, if it's nested BB, we need to ensure it's locked.
        if (tempCmdBo != cmd_bo)
        {
            bool isSecondaryCmdBuf = false;
            it = m_secondaryCmdBufs.begin();
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
                    GraphicsResource::LockParams param;
                    param.m_writeRequest = true;
                    tempRes->pGfxResource->Lock(m_osContext, param);
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

        MOS_OS_CHK_STATUS_RETURN(osInterface->osCpInterface->PermeatePatchForHM(
            tempCmdBo->virt,
            currentPatch,
            resource));

        uint64_t boOffset = alloc_bo->offset64;
        if (alloc_bo != tempCmdBo)
        {
            auto item_ctx = osContext->contextOffsetList.begin();
            for (; item_ctx != osContext->contextOffsetList.end(); item_ctx++)
            {
                if (item_ctx->intel_context == osContext->intel_context && item_ctx->target_bo == alloc_bo)
                {
                    boOffset = item_ctx->offset64;
                    break;
                }
            }
        }

        if (osContext->bUse64BitRelocs)
        {
            *((uint64_t *)((uint8_t *)tempCmdBo->virt + currentPatch->PatchOffset)) =
                    boOffset + currentPatch->AllocationOffset;
        }
        else
        {
            *((uint32_t *)((uint8_t *)tempCmdBo->virt + currentPatch->PatchOffset)) =
                    boOffset + currentPatch->AllocationOffset;
        }

         if (scalaEnabled)
        {
            it = m_secondaryCmdBufs.begin();
            while(it != m_secondaryCmdBufs.end())
            {
                if (it->second->OsResource.bo == tempCmdBo &&
                    it->second->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_SLAVE)
                {
                    mos_bo_set_exec_object_async(alloc_bo);
                    break;
                }
                it++;
            }
        }
        else if (cmdBuffer->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_SLAVE)
        {
            mos_bo_set_exec_object_async(alloc_bo);
        }

        // This call will patch the command buffer with the offsets of the indirect state region of the command buffer
        if (mos_gem_bo_is_softpin(alloc_bo))
        {
            ret = mos_bo_emit_reloc(tempCmdBo, 0, alloc_bo, 0, I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_CPU);
        }
        else
        {
            ret = mos_bo_emit_reloc2(
                tempCmdBo,                                                         // Command buffer
                currentPatch->PatchOffset,                                         // Offset in the command buffer
                alloc_bo,                                                          // Allocation object for which the patch will be made.
                currentPatch->AllocationOffset,                                    // Offset to the indirect state
                I915_GEM_DOMAIN_RENDER,                                            // Read domain
                (currentPatch->uiWriteOperation) ? I915_GEM_DOMAIN_RENDER : 0x0,   // Write domain
                boOffset);
        }

        if (ret != 0)
        {
            MOS_OS_ASSERTMESSAGE("Error patching alloc_bo = 0x%x, cmd_bo = 0x%x.",
                (uintptr_t)alloc_bo,
                (uintptr_t)tempCmdBo);
            return MOS_STATUS_UNKNOWN;
        }
    }

    for(auto res: mappedResList)
    {
        res->pGfxResource->Unlock(m_osContext);
    }

    //Add Batch buffer End Command
    uint32_t batchBufferEndCmd = MI_BATCHBUFFER_END;
    if (MOS_FAILED(Mos_AddCommand(
            cmdBuffer,
            &batchBufferEndCmd,
            sizeof(uint32_t))))
    {
        MOS_OS_ASSERTMESSAGE("Inserting BB_END failed!");
        return MOS_STATUS_UNKNOWN;
    }

    // Now, we can unmap the video command buffer, since we don't need CPU access anymore.
    MOS_OS_CHK_NULL_RETURN(cmdBuffer->OsResource.pGfxResource);
    cmdBuffer->OsResource.pGfxResource->Unlock(m_osContext);

    it = m_secondaryCmdBufs.begin();
    while(it != m_secondaryCmdBufs.end())
    {
        MOS_OS_CHK_NULL_RETURN(it->second->OsResource.pGfxResource);
        it->second->OsResource.pGfxResource->Unlock(m_osContext);

        it++;
    }

    int32_t perfData;
    if (osContext->pPerfData != nullptr)
    {
        perfData = *(int32_t *)(osContext->pPerfData);
    }
    else
    {
        perfData = 0;
    }

    drm_clip_rect_t *cliprects     = nullptr;
    int32_t          num_cliprects = 0;
    int32_t          DR4           = osContext->uEnablePerfTag ? perfData : 0;

    //Since CB2 command is not supported, remove it and set cliprects to nullprt as default.
    if ((gpuNode == MOS_GPU_NODE_VIDEO || gpuNode == MOS_GPU_NODE_VIDEO2) &&
        (cmdBuffer->iSubmissionType & SUBMISSION_TYPE_SINGLE_PIPE_MASK))
    {
        if (osContext->bKMDHasVCS2)
        {
            if (osContext->bPerCmdBufferBalancing && osInterface->pfnGetVdboxNodeId)
            {
                execFlag = GetVcsExecFlag(osInterface, cmdBuffer, gpuNode);
            }
            else if (gpuNode == MOS_GPU_NODE_VIDEO)
            {
                execFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING1;
            }
            else if (gpuNode == MOS_GPU_NODE_VIDEO2)
            {
                execFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING2;
            }
            else if ((gpuNode == MOS_GPU_NODE_BLT))
            {
                execFlag = I915_EXEC_BLT;
            }
            else
            {
                MOS_OS_ASSERTMESSAGE("Invalid gpuNode.");
            }
        }
    }

#if (_DEBUG || _RELEASE_INTERNAL)

    MOS_LINUX_BO *bad_cmd_bo = nullptr;
    MOS_LINUX_BO *nop_cmd_bo = nullptr;
    uint32_t      dwComponentTag = 0;
    uint32_t      dwCallType = 0;

    //dwComponentTag 3: decode,5: vpp,6: encode
    //dwCallType     8: PAK(CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE)
    //             34: PREENC
    //             5: VPP
    dwComponentTag = (perfData & 0xF000) >> 12;
    dwCallType     = (perfData & 0xFC) >> 2;

    if (osInterface->bTriggerCodecHang &&
        (dwComponentTag == 3 || (dwComponentTag == 6 && dwCallType == 8) ||
            (dwComponentTag == 6 && dwCallType == 34) ||
            (dwComponentTag == 5 && dwCallType == 5)))
    {
        bad_cmd_bo = Mos_GetBadCommandBuffer_Linux(osInterface);
        if (bad_cmd_bo)
        {
            ret = mos_bo_mrb_exec(bad_cmd_bo,
                4096,
                nullptr,
                0,
                0,
                execFlag);
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Mos_GetBadCommandBuffer_Linux failed!");
        }
    }
    else if (osInterface->bTriggerVPHang == true)
    {
        bad_cmd_bo = Mos_GetBadCommandBuffer_Linux(osInterface);

        if (bad_cmd_bo)
        {
            ret = mos_bo_mrb_exec(bad_cmd_bo,
                4096,
                nullptr,
                0,
                0,
                execFlag);
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Mos_GetBadCommandBuffer_Linux failed!");
        }

        osInterface->bTriggerVPHang = false;
    }

    nop_cmd_bo = nullptr;
    if (nullRendering == true)
    {
        nop_cmd_bo = Mos_GetNopCommandBuffer_Linux(osInterface);

        if (nop_cmd_bo)
        {
            ret = mos_bo_mrb_exec(nop_cmd_bo,
                4096,
                nullptr,
                0,
                0,
                execFlag);
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Mos_GetNopCommandBuffer_Linux failed!");
        }
    }

#endif  //(_DEBUG || _RELEASE_INTERNAL)

    if (gpuNode != I915_EXEC_RENDER &&
        osInterface->osCpInterface->IsTearDownHappen())
    {
        // skip PAK command when CP tear down happen to avoid of GPU hang
        // conditonal batch buffer start PoC is in progress
    }
    else if (nullRendering == false)
    {
        if (osInterface->ctxBasedScheduling && m_i915Context[0] != nullptr)
        {
            if (cmdBuffer->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_MASK)
            {
                if (scalaEnabled)
                {
                    uint32_t secondaryIndex = 0;
                    it = m_secondaryCmdBufs.begin();
                    while(it != m_secondaryCmdBufs.end())
                    {
                        if (it->second->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_SLAVE)
                        {
                            it->second->iSubmissionType |= (secondaryIndex << SUBMISSION_TYPE_MULTI_PIPE_SLAVE_INDEX_SHIFT);
                            secondaryIndex++;
                        }

                        ret = SubmitPipeCommands(it->second,
                                                 it->second->OsResource.bo,
                                                 osContext,
                                                 execFlag,
                                                 DR4);
                        it++;
                    }
                }
                else
                {
                    ret = SubmitPipeCommands(cmdBuffer,
                                             cmd_bo,
                                             osContext,
                                             execFlag,
                                             DR4);
                }
            }
            else
            {
                ret = mos_gem_bo_context_exec2(cmd_bo,
                    m_commandBufferSize,
                    m_i915Context[0],
                    cliprects,
                    num_cliprects,
                    DR4,
                    m_i915ExecFlag,
                    nullptr);
            }
        }
        else
        {
            ret = mos_gem_bo_context_exec2(cmd_bo,
                m_commandBufferSize,
                osContext->intel_context,
                cliprects,
                num_cliprects,
                DR4,
                execFlag,
                nullptr);
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

    MOS_DEVULT_FuncCall(pfnUltGetCmdBuf, cmdBuffer);

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
pthread_mutex_lock(&command_dump_mutex);
if (osInterface->bDumpCommandBuffer)
    {
        mos_bo_map(cmd_bo, 0);
        osInterface->pfnDumpCommandBuffer(osInterface, cmdBuffer);
        mos_bo_unmap(cmd_bo);
    }
    pthread_mutex_unlock(&command_dump_mutex);
#endif  // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

#if (_DEBUG || _RELEASE_INTERNAL)
    if (bad_cmd_bo)
    {
        mos_bo_wait_rendering(bad_cmd_bo);
        mos_bo_unreference(bad_cmd_bo);
    }
    if (nop_cmd_bo)
    {
        mos_bo_unreference(nop_cmd_bo);
    }
#endif  //(_DEBUG || _RELEASE_INTERNAL)

    //clear command buffer relocations to fix memory leak issue
    mos_gem_bo_clear_relocs(cmd_bo, 0);
    it = m_secondaryCmdBufs.begin();
    while(it != m_secondaryCmdBufs.end())
    {
        mos_gem_bo_clear_relocs(it->second->OsResource.bo, 0);
        MOS_FreeMemory(it->second);
        it++;
    }
    m_secondaryCmdBufs.clear();

    // Reset resource allocation
    m_numAllocations = 0;
    MOS_ZeroMemory(m_allocationList, sizeof(ALLOCATION_LIST) * m_maxNumAllocations);
    m_currentNumPatchLocations = 0;
    MOS_ZeroMemory(m_patchLocationList, sizeof(PATCHLOCATIONLIST) * m_maxNumAllocations);
    m_resCount = 0;

    MOS_ZeroMemory(m_writeModeList, sizeof(bool) * m_maxNumAllocations);
finish:
    return eStatus;
}

int32_t GpuContextSpecific::SubmitPipeCommands(
    MOS_COMMAND_BUFFER *cmdBuffer,
    MOS_LINUX_BO *cmdBo,
    PMOS_CONTEXT osContext,
    uint32_t execFlag,
    int32_t dr4)
{
    int32_t      ret        = 0;
    int          fence      = -1;
    unsigned int fenceFlag = 0;

    MOS_LINUX_CONTEXT *queue = m_i915Context[0];
    bool isVeboxSubmission   = false;

    if (execFlag == MOS_GPU_NODE_VIDEO || execFlag == MOS_GPU_NODE_VIDEO2)
    {
        execFlag = I915_EXEC_DEFAULT;
    }
    if (execFlag == MOS_GPU_NODE_VE)
    {
        execFlag = I915_EXEC_DEFAULT;
        isVeboxSubmission = true;
    }

    if(cmdBuffer->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_SLAVE)
    {
        fence = osContext->submit_fence;
        fenceFlag = I915_EXEC_FENCE_SUBMIT;
        int slaveIndex = (cmdBuffer->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_SLAVE_INDEX_MASK) >> SUBMISSION_TYPE_MULTI_PIPE_SLAVE_INDEX_SHIFT;
        if(slaveIndex < 7)
        {
            queue = m_i915Context[2 + slaveIndex]; //0 is for single pipe, 1 is for master, slave starts from 2
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("slaveIndex value: %s is invalid!", slaveIndex);
            return -1;
        }

        if (isVeboxSubmission)
        {
            queue = m_i915Context[cmdBuffer->iVeboxNodeIndex + 1];
        }
    }

    //Keep FE and BE0 running on same engine for VT decode
    if((cmdBuffer->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_ALONE)
        || (cmdBuffer->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_MASTER))
    {
        if(cmdBuffer->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_MASTER)
        {
            //Only master pipe needs fence out flag
            fenceFlag = I915_EXEC_FENCE_OUT;
        }
        queue = m_i915Context[1];
    }

    ret = mos_gem_bo_context_exec2(cmdBo,
                                  cmdBo->size,
                                  queue,
                                  nullptr,
                                  0,
                                  dr4,
                                  execFlag | fenceFlag,
                                  &fence);

    if(cmdBuffer->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_MASTER)
    {
        osContext->submit_fence = fence;
    }
    if(cmdBuffer->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_FLAGS_LAST_PIPE)
    {
        close(fence);
    }

    return ret;
}

void GpuContextSpecific::IncrementGpuStatusTag()
{
    m_GPUStatusTag = m_GPUStatusTag % UINT_MAX + 1;
    if (m_GPUStatusTag == 0)
    {
        m_GPUStatusTag = 1;
    }
}

void GpuContextSpecific::ResetGpuContextStatus()
{
    MOS_ZeroMemory(m_allocationList, sizeof(ALLOCATION_LIST) * ALLOCATIONLIST_SIZE);
    m_numAllocations = 0;
    MOS_ZeroMemory(m_patchLocationList, sizeof(PATCHLOCATIONLIST) * PATCHLOCATIONLIST_SIZE);
    m_currentNumPatchLocations = 0;

    MOS_ZeroMemory(m_attachedResources, sizeof(MOS_RESOURCE) * ALLOCATIONLIST_SIZE);
    m_resCount = 0;

    MOS_ZeroMemory(m_writeModeList, sizeof(bool) * ALLOCATIONLIST_SIZE);

    if ((m_cmdBufFlushed == true) && m_commandBuffer->OsResource.bo)
    {
        m_commandBuffer->OsResource.bo = nullptr;
    }
}

MOS_STATUS GpuContextSpecific::AllocateGPUStatusBuf()
{
    MOS_OS_FUNCTION_ENTER;

    GraphicsResource::CreateParams params;
    params.m_tileType  = MOS_TILE_LINEAR;
    params.m_type      = MOS_GFXRES_BUFFER;
    params.m_format    = Format_Buffer;
    params.m_width     = sizeof(MOS_GPU_STATUS_DATA);
    params.m_height    = 1;
    params.m_depth     = 1;
    params.m_arraySize = 1;
    params.m_name      = "GPU Status Buffer";

    GraphicsResource *graphicsResource = GraphicsResource::CreateGraphicResource(GraphicsResource::osSpecificResource);
    MOS_OS_CHK_NULL_RETURN(graphicsResource);

    MOS_OS_CHK_STATUS_RETURN(graphicsResource->Allocate(m_osContext, params));

    GraphicsResource::LockParams lockParams;
    lockParams.m_writeRequest = true;
    auto gpuStatusData       = (MOS_GPU_STATUS_DATA *)graphicsResource->Lock(m_osContext, lockParams);
    if (gpuStatusData == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Unable to lock gpu eStatus buffer for read.");
        graphicsResource->Free(m_osContext);
        MOS_Delete(graphicsResource);
        return MOS_STATUS_UNKNOWN;
    }

    m_statusBufferResource = graphicsResource;
    return MOS_STATUS_SUCCESS;
}
