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
//! \file    mos_gpucontext_specific.cpp
//! \brief   Container class for the Linux specific gpu context
//!

#include "mos_context_specific.h"
#include "mos_gpucontext_specific.h"
#include "mos_graphicsresource_specific.h"
#include "mos_commandbuffer_specific.h"

#include "mos_cmdbufmgr.h"

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

    if (reusedContext)
    {
        MOS_OS_NORMALMESSAGE("gpucontex reusing not enabled on Linux");
    }
}

GpuContextSpecific::~GpuContextSpecific()
{
    MOS_OS_FUNCTION_ENTER;

    Clear();
}

MOS_STATUS GpuContextSpecific::Init(OsContext *osContext)
{
    MOS_OS_FUNCTION_ENTER;

    m_cmdBufPool.clear();

    m_commandBufferSize = COMMAND_BUFFER_SIZE;

    m_nextFetchIndex = 0;

    m_cmdBufFlushed = true;

    m_osContext = osContext;

    MOS_OS_CHK_STATUS_RETURN(AllocateGPUStatusBuf());

    m_commandBuffer = (PMOS_COMMAND_BUFFER)MOS_AllocAndZeroMemory(sizeof(MOS_COMMAND_BUFFER));

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
    }

    for (auto& curCommandBuffer : m_cmdBufPool)
    {
        auto curCommandBufferSpecific = static_cast<CommandBufferSpecific *>(curCommandBuffer);
        curCommandBufferSpecific->waitReady(); // wait ready and return to comamnd buffer manager.
        m_cmdBufMgr->ReleaseCmdBuf(curCommandBuffer);
    }

    m_cmdBufPool.clear();

    MOS_SafeFreeMemory(m_commandBuffer);
    MOS_SafeFreeMemory(m_allocationList);
    MOS_SafeFreeMemory(m_patchLocationList);
    MOS_SafeFreeMemory(m_attachedResources);
    MOS_SafeFreeMemory(m_writeModeList);
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
    m_patchLocationList[m_currentNumPatchLocations].uiWriteOperation = params->bWrite;

    if (osInterface->osCpInterface &&
        osInterface->osCpInterface->IsHMEnabled())
    {
        osInterface->osCpInterface->RegisterPatchForHM(
            (uint32_t *)(params->cmdBufBase + params->uiPatchOffset),
            params->bWrite,
            params->HwCommandType,
            params->forceDwordOffset,
            params->presResource);
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

    CommandBuffer* cmdBuf = nullptr;

    if (m_cmdBufFlushed)
    {
        if (m_cmdBufPool.size() < MAX_CMD_BUF_NUM)
        {
            cmdBuf = m_cmdBufMgr->PickupOneCmdBuf(m_commandBufferSize);
            MOS_OS_CHK_STATUS_RETURN(cmdBuf->BindToGpuContext(this));
            m_cmdBufPool.push_back(cmdBuf);
        }
        else if (m_cmdBufPool.size() == MAX_CMD_BUF_NUM)
        {
            auto cmdBufOld = m_cmdBufPool[m_nextFetchIndex];
            auto cmdBufSpecificOld = static_cast<CommandBufferSpecific *>(cmdBufOld);
            cmdBufSpecificOld->waitReady();
            cmdBufSpecificOld->UnBindToGpuContext();
            m_cmdBufMgr->ReleaseCmdBuf(cmdBufOld);  // here just return old command buffer to available pool

            //pick up new comamnd buffer
            cmdBuf         = m_cmdBufMgr->PickupOneCmdBuf(m_commandBufferSize);
            MOS_OS_CHK_STATUS_RETURN(cmdBuf->BindToGpuContext(this));
            m_cmdBufPool[m_nextFetchIndex] = cmdBuf;
        }

        // util now, we got new command buffer from CmdBufMgr, next step to fill in the input command buffer
        MOS_OS_CHK_STATUS_RETURN(cmdBuf->GetResource()->ConvertToMosResource(&comamndBuffer->OsResource));
        comamndBuffer->pCmdBase   = (uint32_t *)cmdBuf->GetLockAddr();
        comamndBuffer->pCmdPtr    = (uint32_t *)cmdBuf->GetLockAddr();
        comamndBuffer->iOffset    = 0;
        comamndBuffer->iRemaining = cmdBuf->GetCmdBufSize();
        comamndBuffer->iCmdIndex  = m_nextFetchIndex;

        // zero comamnd buffer
        MOS_ZeroMemory(comamndBuffer->pCmdBase, comamndBuffer->iRemaining);

        // update command buffer relared filed in GPU context 
        m_cmdBufFlushed = false;

        // keep a copy in GPU context
        MOS_SecureMemcpy(m_commandBuffer, sizeof(MOS_COMMAND_BUFFER), comamndBuffer, sizeof(MOS_COMMAND_BUFFER));

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
        MOS_SecureMemcpy(comamndBuffer, sizeof(MOS_COMMAND_BUFFER), m_commandBuffer, sizeof(MOS_COMMAND_BUFFER));
    }

    MOS_OS_CHK_STATUS_RETURN(RegisterResource(&m_commandBuffer->OsResource, false));

    return MOS_STATUS_SUCCESS;
}

void GpuContextSpecific::ReturnCommandBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer,
    uint32_t            flags)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_ASSERT(cmdBuffer);

    m_commandBuffer->iOffset    = cmdBuffer->iOffset;
    m_commandBuffer->iRemaining = cmdBuffer->iRemaining;
    m_commandBuffer->pCmdPtr    = cmdBuffer->pCmdPtr;
}

MOS_STATUS GpuContextSpecific::ResetCommandBuffer()
{
    m_cmdBufFlushed = true;
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
    uint32_t     addCb2   = 0xffffffff;
    MOS_STATUS   eStatus  = MOS_STATUS_SUCCESS;
    int32_t      ret      = 0;

    // Command buffer object DRM pointer
    m_cmdBufFlushed = true;
    auto cmd_bo     = cmdBuffer->OsResource.bo;

    // Now, the patching will be done, based on the patch list.
    for (uint32_t patchIndex = 0; patchIndex < m_currentNumPatchLocations; patchIndex++)
    {
        auto currentPatch = &m_patchLocationList[patchIndex];
        if (nullptr == currentPatch)
        {
            MOS_OS_ASSERTMESSAGE("Unexpected, found null entry in patch list!");
        }

        auto allocationIndex = currentPatch->AllocationIndex;
        auto resourceOffset  = currentPatch->AllocationOffset;
        auto patchOffset     = currentPatch->PatchOffset;

        // This is the resource for which patching will be done
        auto resource = (PMOS_RESOURCE)m_allocationList[allocationIndex].hAllocation;
        MOS_OS_CHK_NULL_RETURN(resource);

        // For now, we'll assume the system memory's DRM bo pointer
        // is NULL.  If nullptr is detected, then the resource has been
        // placed inside the command buffer's indirect state area.
        // We'll simply set alloc_bo to the command buffer's bo pointer.
        MOS_OS_ASSERT(resource->bo);

        auto alloc_bo = (resource->bo) ? resource->bo : cmd_bo;

        MOS_OS_CHK_STATUS_RETURN(osInterface->osCpInterface->PermeatePatchForHM(
            cmd_bo->virt,
            currentPatch))

#ifndef ANDROID
        uint64_t boOffset = alloc_bo->offset64;
        if (alloc_bo != cmd_bo)
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
            *((uint64_t *)((uint8_t *)cmd_bo->virt + patchOffset)) = boOffset + resourceOffset;
        }
        else
        {
            *((uint32_t *)((uint8_t *)cmd_bo->virt + patchOffset)) = boOffset + resourceOffset;
        }

        // This call will patch the command buffer with the offsets of the indirect state region of the command buffer
        ret = mos_bo_emit_reloc2(
            cmd_bo,                                                              // Command buffer
            patchOffset,                                                         // Offset in the command buffer
            alloc_bo,                                                            // Allocation object for which the patch will be made.
            resourceOffset,                                                      // Offset to the indirect state
            I915_GEM_DOMAIN_RENDER,                                              // Read domain
            (currentPatch->uiWriteOperation) ? I915_GEM_DOMAIN_RENDER : 0x0,   // Write domain
            boOffset);
#else
            if (osContext->bUse64BitRelocs)
            {
                *((uint64_t*)((uint8_t*)cmd_bo->virt + patchOffset)) = alloc_bo->offset64 + resourceOffset;
            }
            else
            {
                *((uint32_t*)((uint8_t*)cmd_bo->virt + patchOffset)) = alloc_bo->offset64 + resourceOffset;
            }

        // This call will patch the command buffer with the offsets of the indirect state region of the command buffer
        ret = mos_bo_emit_reloc(
            cmd_bo,  // Command buffer
            patchOffset,  // Offset in the command buffer
            alloc_bo,  // Allocation object for which the patch will be made.
            resourceOffset,  // Offset to the indirect state
            I915_GEM_DOMAIN_RENDER,  // Read domain
            (currentPatch->uiWriteOperation) ? I915_GEM_DOMAIN_RENDER : 0x0);  // Write domain
#endif
        if (ret != 0)
        {
            MOS_OS_ASSERTMESSAGE("Error patching alloc_bo = 0x%x, cmd_bo = 0x%x.",
                (uintptr_t)alloc_bo,
                (uintptr_t)cmd_bo);
            return MOS_STATUS_UNKNOWN;
        }
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

    if (gpuNode == I915_EXEC_RENDER)
    {
        if (true == osInterface->osCpInterface->IsHMEnabled())
        {
            cliprects     = (drm_clip_rect *)(&addCb2);
            num_cliprects = sizeof(addCb2);
        }
    }
    else
    {
        if (osContext->bKMDHasVCS2)
        {
            if (gpuNode == MOS_GPU_NODE_VIDEO)
            {
                execFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING1;
            }
            else if (gpuNode == MOS_GPU_NODE_VIDEO2)
            {
                execFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING2;
            }
        }
    }

#if (_DEBUG || _RELEASE_INTERNAL)

    MOS_LINUX_BO *bad_cmd_bo;
    MOS_LINUX_BO *nop_cmd_bo;
    uint32_t      dwComponentTag;
    uint32_t      dwCallType;

    // trigger GPU HANG if bTriggerCodecHang is set
    bad_cmd_bo = nullptr;

    //dwComponentTag 3: decode,5: vpp,6: encode
    //dwCallType     8: PAK(CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE)
    //		     34: PREENC
    //		     5: VPP
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
#ifdef ANDROID
        if (osContext->uEnablePerfTag == 2)
        {
            ret = mos_gem_bo_tag_exec(cmd_bo,
                m_commandBufferSize,
                osContext->intel_context,
                cliprects,
                num_cliprects,
                0,
                execFlag,
                perfData);
        }
        else
        {
            ret = mos_bo_mrb_exec(cmd_bo,
                m_commandBufferSize,
                cliprects,
                num_cliprects,
                DR4,
                execFlag);
        }
#else
        ret = mos_gem_bo_context_exec2(cmd_bo,
            m_commandBufferSize,
            osContext->intel_context,
            cliprects,
            num_cliprects,
            DR4,
            execFlag);
#endif

        if (ret != 0)
        {
            eStatus = MOS_STATUS_UNKNOWN;
        }
    }

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Command buffer submission failed!");
    }

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
