/*
* Copyright (c) 2009-2019, Intel Corporation
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
//! \file     mos_interface.cpp
//! \brief    MOS interface implementation
//!

#include "mos_interface.h"

MOS_STATUS MosInterface::CreateOsDeviceContext(DDI_DEVICE_CONTEXT ddiDeviceContext, MOS_DEVICE_HANDLE *deviceContext)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DestroyOsDeviceContext(MOS_DEVICE_HANDLE deviceContext)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::CreateOsStreamState(
    MOS_STREAM_HANDLE *streamState,
    MOS_DEVICE_HANDLE  deviceContext)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DestroyOsStreamState(
    MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

uint32_t MosInterface::GetInterfaceVersion(MOS_DEVICE_HANDLE deviceContext)
{
    MOS_OS_FUNCTION_ENTER;

    return 0;
}

PLATFORM *MosInterface::GetPlatform(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    return nullptr;
}

MEDIA_FEATURE_TABLE *MosInterface::GetSkuTable(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    return nullptr;
}

MEDIA_WA_TABLE *MosInterface::GetWaTable(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    return nullptr;
}

MEDIA_SYSTEM_INFO *MosInterface::GetGtSystemInfo(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    return nullptr;
}

ADAPTER_INFO *MosInterface::GetAdapterInfo(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    return nullptr;
}

MOS_STATUS MosInterface::CreateGpuContext(
    MOS_STREAM_HANDLE             streamState,
    GpuContextCreateOption      &createOption,
    GPU_CONTEXT_HANDLE           &gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DestroyGpuContext(
    MOS_STREAM_HANDLE  streamState,
    GPU_CONTEXT_HANDLE gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SetGpuContext(
    MOS_STREAM_HANDLE  streamState,
    GPU_CONTEXT_HANDLE gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::AddCommand(
    COMMAND_BUFFER_HANDLE cmdBuffer,
    const void *          cmd,
    uint32_t              cmdSize)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
MOS_STATUS MosInterface::DumpCommandBuffer(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}
#endif  // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

MOS_STATUS MosInterface::GetCommandBuffer(
    MOS_STREAM_HANDLE      streamState,
    COMMAND_BUFFER_HANDLE &cmdBuffer,
    uint32_t               pipeIdx)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::ReturnCommandBuffer(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer,
    uint32_t              pipeIdx)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SubmitCommandBuffer(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer,
    bool                  nullRendering)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

MOS_STATUS MosInterface::ResetCommandBuffer(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::VerifyCommandBufferSize(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer,
    uint32_t              requestedSize,
    uint32_t              pipeIdx)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::ResizeCommandBufferAndPatchList(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer,
    uint32_t              requestedSize,
    uint32_t              requestedPatchListSize,
    uint32_t              pipeIdx)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SetPatchEntry(
    MOS_STREAM_HANDLE       streamState,
    PMOS_PATCH_ENTRY_PARAMS params)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::GetIndirectState(
    MOS_STREAM_HANDLE     streamState,
    uint8_t **            indirectState,
    uint32_t &            offset,
    uint32_t &            size)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SetupIndirectState(
    MOS_STREAM_HANDLE     streamState,
    uint32_t              size)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MEMORY_OBJECT_CONTROL_STATE MosInterface::GetCachePolicyMemoryObject(
    MOS_STREAM_HANDLE   streamState,
    MOS_HW_RESOURCE_DEF mosUsage)
{
    MOS_OS_FUNCTION_ENTER;

    return {0};
}

//Mos_Specific_SetOsResourceFromDdi
//DdiMedia_MediaSurfaceToMosResource
MOS_STATUS MosInterface::ConvertResourceFromDdi(
    OsSpecificRes osResource,
    MOS_RESOURCE_HANDLE &resource,
    uint32_t firstArraySlice,
    uint32_t mipSlice)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::CreateOsSpecificResourceInfo(OsSpecificRes resource, bool isInternal)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DestroySpecificResourceInfo(OsSpecificRes resource)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::AllocateResource(
    MOS_STREAM_HANDLE        streamState,
    PMOS_ALLOC_GFXRES_PARAMS params,
    MOS_RESOURCE_HANDLE     &resource)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::FreeResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    uint32_t            flag)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::GetResourceInfo(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    MosResourceInfo     &details)  //MOS_SURFACE
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

void *MosInterface::LockMosResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    PMOS_LOCK_PARAMS    flags)
{
    MOS_OS_FUNCTION_ENTER;

    return nullptr;
}

MOS_STATUS MosInterface::UnlockMosResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::RegisterResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    bool                write,
    uint32_t            streamIndex,
    uint32_t            gpuContextOrdinal)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

uint64_t MosInterface::GetResourceGfxAddress(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource)
{
    MOS_OS_FUNCTION_ENTER;

    return 0;
}

uint32_t MosInterface::GetResourceAllocationIndex(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource)
{
    MOS_OS_FUNCTION_ENTER;

    return 0;
}

MOS_STATUS MosInterface::SkipResourceSync(
    MOS_RESOURCE_HANDLE resource)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::ResourceSync(
    MOS_RESOURCE_HANDLE resource,
    MOS_DEVICE_HANDLE   deviceContext,
    uint32_t            index,
    SYNC_HAZARD         hazardType,
    GPU_CONTEXT_HANDLE  busyCtx,
    GPU_CONTEXT_HANDLE  requestorCtx,
    OS_HANDLE           osHandle)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::LockSync(
    MOS_RESOURCE_HANDLE resource,
    MOS_DEVICE_HANDLE   deviceContext,
    uint32_t            index,
    SYNC_HAZARD         hazardType,
    GPU_CONTEXT_HANDLE  busyCtx,
    bool                doNotWait)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::TrimResidency(
    bool      periodicTrim,
    bool      restartPeriodicTrim,
    uint64_t &numBytesToTrim,
    bool      trimToMinimum,
    bool      trimOnlyMediaResources)
{
    MOS_OS_FUNCTION_ENTER;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DecompResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SetMemoryCompressionMode(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    MOS_MEMCOMP_STATE   resMmcMode)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::GetMemoryCompressionMode(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    MOS_MEMCOMP_STATE  &resMmcMode)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SetMemoryCompressionHint(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    bool                hintOn)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::GetMemoryCompressionFormat(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    uint32_t *          resMmcFormat)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

uint32_t MosInterface::GetGpuStatusTag(
        MOS_STREAM_HANDLE  streamState,
        GPU_CONTEXT_HANDLE gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    return 0;
}

MOS_STATUS MosInterface::IncrementGpuStatusTag(
    MOS_STREAM_HANDLE  streamState,
    GPU_CONTEXT_HANDLE gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

uint64_t MosInterface::GetGpuStatusSyncTag(
    MOS_STREAM_HANDLE  streamState,
    GPU_CONTEXT_HANDLE gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    return 0;
}

MOS_STATUS MosInterface::GetGpuStatusBufferResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

//MosPerfInterface *GetPerfInterface(MOS_STREAM_HANDLE streamState)
GMM_CLIENT_CONTEXT *MosInterface::GetGmmClientContext(
    MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    return nullptr;
}

MosCpInterface *MosInterface::GetCpInterface(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    return streamState ? streamState->osCpInterface : nullptr;
}

MosOcaInterface *MosInterface::GetOcaInterface(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    return nullptr;
}

GpuContextSpecificNext *MosInterface::GetGpuContext(MOS_STREAM_HANDLE streamState, GPU_CONTEXT_HANDLE handle)
{
    MOS_OS_FUNCTION_ENTER;

    return nullptr;
}
