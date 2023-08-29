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
//! \file     decode_allocator.cpp
//! \brief    Defines the interface for decode resource allocate
//! \details  decode allocator will allocate and destory buffers, the caller
//!           can use directly
//!

#include "decode_allocator.h"
#include "decode_utils.h"
#include "External/Common/GmmResourceInfoExt.h"
#include "External/Common/GmmCachePolicyExt.h"
#include "decode_utils.h"
#include "media_allocator.h"
#include "mos_os_cp_interface_specific.h"
#include "mos_utilities.h"
#include "decode_resource_array.h"
#include "mos_interface.h"

namespace decode {

DecodeAllocator::DecodeAllocator(PMOS_INTERFACE osInterface, bool limitedLMemBar) :
    m_osInterface(osInterface), m_limitedLMemBar(limitedLMemBar)
{
    m_allocator = MOS_New(Allocator, m_osInterface);
#if (_DEBUG || _RELEASE_INTERNAL)
    m_forceLockable = ReadUserFeature(m_osInterface->pfnGetUserSettingInstance(m_osInterface), "ForceDecodeResourceLockable", MediaUserSetting::Group::Sequence).Get<uint32_t>();
#endif
}

DecodeAllocator::~DecodeAllocator()
{
    MOS_Delete(m_allocator);
}

MOS_BUFFER* DecodeAllocator::AllocateBuffer(
    const uint32_t sizeOfBuffer, const char* nameOfBuffer,
    ResourceUsage resUsageType, ResourceAccessReq accessReq,
    bool initOnAllocate, uint8_t initValue, bool bPersistent)
{
    if (!m_allocator)
        return nullptr;

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type            = MOS_GFXRES_BUFFER;
    allocParams.TileType        = MOS_TILE_LINEAR;
    allocParams.Format          = Format_Buffer;
    allocParams.dwBytes         = sizeOfBuffer;
    allocParams.pBufName        = nameOfBuffer;
    allocParams.bIsPersistent   = bPersistent;
    allocParams.ResUsageType    = static_cast<MOS_HW_RESOURCE_DEF>(resUsageType);
    SetAccessRequirement(accessReq, allocParams);

    MOS_BUFFER* buffer = m_allocator->AllocateBuffer(allocParams, false, COMPONENT_Decode);
    if (buffer == nullptr)
    {
        return nullptr;
    }

    if (initOnAllocate)
    {
        DECODE_ASSERT(accessReq != notLockableVideoMem);
        MOS_STATUS status = m_allocator->OsFillResource(&buffer->OsResource, sizeOfBuffer, initValue);
        if (status != MOS_STATUS_SUCCESS)
        {
            DECODE_ASSERTMESSAGE("Failed to initialize buffer %s", nameOfBuffer);
        }
    }

    buffer->size = sizeOfBuffer;
    buffer->name = nameOfBuffer;
    buffer->initOnAllocate = initOnAllocate;
    buffer->initValue = initValue;
    buffer->bPersistent = bPersistent;

    return buffer;
}

BufferArray * DecodeAllocator::AllocateBufferArray(
    const uint32_t sizeOfBuffer, const char* nameOfBuffer, const uint32_t numberOfBuffer,
    ResourceUsage resUsageType, ResourceAccessReq accessReq,
    bool initOnAllocate, uint8_t initValue, bool bPersistent)
{
    if (!m_allocator)
    {
        return nullptr;
    }

    BufferArray * bufferArray = MOS_New(BufferArray, this);
    if (bufferArray == nullptr)
    {
        return nullptr;
    }

    for (uint32_t i = 0; i < numberOfBuffer; i++)
    {
        MOS_BUFFER *buf = AllocateBuffer(sizeOfBuffer, nameOfBuffer, resUsageType, accessReq,
            initOnAllocate, initValue, bPersistent);
        bufferArray->Push(buf);
    }

    return bufferArray;
}

MOS_SURFACE* DecodeAllocator::AllocateSurface(
    const uint32_t width, const uint32_t height, const char* nameOfSurface,
    MOS_FORMAT format, bool isCompressible,
    ResourceUsage resUsageType, ResourceAccessReq accessReq,
    MOS_TILE_MODE_GMM gmmTileMode)
{
    if (!m_allocator)
    {
        return nullptr;
    }

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type        = MOS_GFXRES_2D;
    allocParams.TileType    = MOS_TILE_Y;
    allocParams.Format      = format;
    allocParams.dwWidth     = width;
    allocParams.dwHeight    = height;
    allocParams.dwArraySize = 1;
    allocParams.pBufName    = nameOfSurface;
    allocParams.bIsCompressible = isCompressible;
    allocParams.ResUsageType = static_cast<MOS_HW_RESOURCE_DEF>(resUsageType);
    allocParams.m_tileModeByForce = gmmTileMode;
    SetAccessRequirement(accessReq, allocParams);

    MOS_SURFACE* surface = m_allocator->AllocateSurface(allocParams, false, COMPONENT_Decode);
    if (surface == nullptr)
    {
        return nullptr;
    }
    if (GetSurfaceInfo(surface) != MOS_STATUS_SUCCESS)
    {
        DECODE_ASSERTMESSAGE("Failed to get surface informaton for %s", nameOfSurface);
    }

    return surface;
}

MOS_SURFACE *DecodeAllocator::AllocateLinearSurface(
    const uint32_t width, const uint32_t height, const char *nameOfSurface,
    MOS_FORMAT format, bool isCompressible,
    ResourceUsage resUsageType, ResourceAccessReq accessReq,
    MOS_TILE_MODE_GMM gmmTileMode)
{
    if (!m_allocator)
    {
        return nullptr;
    }

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type              = MOS_GFXRES_2D;
    allocParams.TileType          = MOS_TILE_LINEAR;
    allocParams.Format            = format;
    allocParams.dwWidth           = width;
    allocParams.dwHeight          = height;
    allocParams.dwArraySize       = 1;
    allocParams.pBufName          = nameOfSurface;
    allocParams.bIsCompressible   = isCompressible;
    allocParams.ResUsageType      = static_cast<MOS_HW_RESOURCE_DEF>(resUsageType);
    allocParams.m_tileModeByForce = gmmTileMode;
    SetAccessRequirement(accessReq, allocParams);

    MOS_SURFACE *surface = m_allocator->AllocateSurface(allocParams, false, COMPONENT_Decode);
    if (surface == nullptr)
    {
        return nullptr;
    }
    if (GetSurfaceInfo(surface) != MOS_STATUS_SUCCESS)
    {
        DECODE_ASSERTMESSAGE("Failed to get surface informaton for %s", nameOfSurface);
    }

    return surface;
}

SurfaceArray * DecodeAllocator::AllocateSurfaceArray(
    const uint32_t width, const uint32_t height, const char* nameOfSurface,
    const uint32_t numberOfSurface, MOS_FORMAT format, bool isCompressed,
    ResourceUsage resUsageType, ResourceAccessReq accessReq)
{
    if (!m_allocator)
        return nullptr;

    SurfaceArray * surfaceArray = MOS_New(SurfaceArray, this);
    if (surfaceArray == nullptr)
    {
        return nullptr;
    }

    for (uint32_t i = 0; i < numberOfSurface; i++)
    {
        MOS_SURFACE *surface = AllocateSurface(width, height, nameOfSurface, format, isCompressed,
            resUsageType, accessReq);
        surfaceArray->Push(surface);
    }

    return surfaceArray;
}

PMHW_BATCH_BUFFER DecodeAllocator::AllocateBatchBuffer(
    const uint32_t sizeOfBuffer, const uint32_t numOfBuffer, ResourceAccessReq accessReq)
{
    // The default setting is lockableVideoMem
    bool notLockable = false;
    bool inSystemMem = false;

    // Config setting if running with limited LMem bar config or HM enabled.
    if (m_limitedLMemBar || 
        (m_osInterface->osCpInterface != nullptr && m_osInterface->osCpInterface->IsHMEnabled())
    )
    {
        if (accessReq == notLockableVideoMem)
        {
            if (m_osInterface->osCpInterface != nullptr && m_osInterface->osCpInterface->IsHMEnabled())
            {
                notLockable = true;
                inSystemMem = false;
            }
        }
        else
        {
            // allocate batch buffer in systemMem with limited LMem bar config due to local memory limitation
            notLockable = false;
            inSystemMem = true;
        }
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_forceLockable)
    {
        notLockable = 0;
    }
#endif

    PMHW_BATCH_BUFFER batchBuffer = MOS_New(MHW_BATCH_BUFFER);
    if (Mhw_AllocateBb(m_osInterface, batchBuffer, nullptr, sizeOfBuffer, numOfBuffer,
                       notLockable, inSystemMem) != MOS_STATUS_SUCCESS)
    {
        MOS_Delete(batchBuffer);
        return nullptr;
    }

    return batchBuffer;
}

BatchBufferArray * DecodeAllocator::AllocateBatchBufferArray(
    const uint32_t sizeOfSubBuffer, const uint32_t numOfSubBuffer,
    const uint32_t numberOfBatchBuffer, bool secondLevel,
    ResourceAccessReq accessReq)
{
    if (!m_allocator)
        return nullptr;

    BatchBufferArray * batchBufferArray = MOS_New(BatchBufferArray, this);
    if (batchBufferArray == nullptr)
    {
        return nullptr;
    }

    for (uint32_t i = 0; i < numberOfBatchBuffer; i++)
    {
        PMHW_BATCH_BUFFER batchBuffer = AllocateBatchBuffer(sizeOfSubBuffer, numOfSubBuffer, accessReq);
        if (batchBuffer == nullptr)
        {
            continue;
        }
        batchBuffer->bSecondLevel = secondLevel;
        batchBufferArray->Push(batchBuffer);
    }

    return batchBufferArray;
}

void* DecodeAllocator::Lock(MOS_RESOURCE* resource, MOS_LOCK_PARAMS* lockFlag)
{
    if (!m_allocator)
        return nullptr;

    return m_allocator->Lock(resource, lockFlag);
}

void* DecodeAllocator::LockResourceForWrite(MOS_RESOURCE* resource)
{
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;

    if (!m_allocator)
        return nullptr;

    return m_allocator->Lock(resource, &lockFlags);
}

void* DecodeAllocator::LockResourceWithNoOverwrite(MOS_RESOURCE* resource)
{
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly   = 1;
    lockFlags.NoOverWrite = 1;

    if (!m_allocator)
        return nullptr;

    return m_allocator->Lock(resource, &lockFlags);
}

void* DecodeAllocator::LockResourceForRead(MOS_RESOURCE* resource)
{
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;

    if (!m_allocator)
        return nullptr;

    return m_allocator->Lock(resource, &lockFlags);
}

void* DecodeAllocator::LockResourceForRead(MOS_BUFFER* buffer)
{
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;

    if (m_allocator == nullptr || buffer == nullptr)
        return nullptr;

    return m_allocator->Lock(&buffer->OsResource, &lockFlags);
}

MOS_STATUS DecodeAllocator::Lock(PMHW_BATCH_BUFFER batchBuffer)
{
    DECODE_CHK_NULL(batchBuffer);
    return Mhw_LockBb(m_osInterface, batchBuffer);
}

MOS_STATUS DecodeAllocator::UnLock(MOS_RESOURCE* resource)
{
    DECODE_CHK_NULL(m_allocator);
    return m_allocator->UnLock(resource);
}

MOS_STATUS DecodeAllocator::UnLock(MOS_BUFFER* buffer)
{
    DECODE_CHK_NULL(m_allocator);
    DECODE_CHK_NULL(buffer);
    return m_allocator->UnLock(&buffer->OsResource);
}

MOS_STATUS DecodeAllocator::UnLock(PMHW_BATCH_BUFFER batchBuffer, bool resetBuffer)
{
    DECODE_CHK_NULL(batchBuffer);
    return Mhw_UnlockBb(m_osInterface, batchBuffer, resetBuffer);
}

MOS_STATUS DecodeAllocator::Resize(MOS_BUFFER* &buffer, const uint32_t sizeNew,
                                   ResourceAccessReq accessReq, bool force, bool clearData)
{
    DECODE_CHK_NULL(buffer);

    if (sizeNew == buffer->size)
    {
        if (clearData)
        {
            if(m_allocator->OsFillResource(&buffer->OsResource, buffer->size, 0) != MOS_STATUS_SUCCESS)
            {
                DECODE_ASSERTMESSAGE("Failed to clear buffer data");
            }
        }
        return MOS_STATUS_SUCCESS;
    }

    if (force || (sizeNew > buffer->size))
    {
        if(clearData)
        {
            buffer->initOnAllocate = true;
            buffer->initValue      = 0;
        }
        ResourceUsage resUsageType = ConvertGmmResourceUsage(buffer->OsResource.pGmmResInfo->GetCachePolicyUsage());
        MOS_BUFFER* bufferNew = AllocateBuffer(
            sizeNew, buffer->name, resUsageType, accessReq,
            buffer->initOnAllocate, buffer->initValue, buffer->bPersistent);
        DECODE_CHK_NULL(bufferNew);

        Destroy(buffer);
        buffer = bufferNew;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeAllocator::Resize(MOS_SURFACE* &surface, const uint32_t widthNew, const uint32_t heightNew,
                                   ResourceAccessReq accessReq, bool force, const char* nameOfSurface)
{
    DECODE_CHK_NULL(surface);

    if ((widthNew == surface->dwWidth) && (heightNew == surface->dwHeight))
    {
        return MOS_STATUS_SUCCESS;
    }

    if (force || (widthNew > surface->dwWidth) || (heightNew > surface->dwHeight))
    {
        ResourceUsage resUsageType = ConvertGmmResourceUsage(surface->OsResource.pGmmResInfo->GetCachePolicyUsage());
        MOS_SURFACE* surfaceNew = AllocateSurface(widthNew, heightNew, nameOfSurface,
            surface->Format, surface->bCompressible, resUsageType, accessReq, surface->TileModeGMM);
        DECODE_CHK_NULL(surfaceNew);

        Destroy(surface);
        surface = surfaceNew;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeAllocator::Resize(
    PMHW_BATCH_BUFFER &batchBuffer, const uint32_t sizeOfBufferNew, const uint32_t numOfBufferNew,
    ResourceAccessReq accessReq)
{
    DECODE_CHK_NULL(batchBuffer);

    if (int32_t(sizeOfBufferNew) <= batchBuffer->iSize && numOfBufferNew <= batchBuffer->count)
    {
        return MOS_STATUS_SUCCESS;
    }

    PMHW_BATCH_BUFFER batchBufferNew = AllocateBatchBuffer(sizeOfBufferNew, numOfBufferNew, accessReq);
    DECODE_CHK_NULL(batchBufferNew);

    DECODE_CHK_STATUS(Destroy(batchBuffer));
    batchBuffer = batchBufferNew;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeAllocator::Destroy(MOS_BUFFER* & buffer)
{
    DECODE_CHK_NULL(m_allocator);
    if (buffer == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    DECODE_CHK_STATUS(m_allocator->DestroyBuffer(buffer));
    buffer = nullptr;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeAllocator::Destroy(MOS_SURFACE* & surface)
{
    DECODE_CHK_NULL(m_allocator);
    if (surface == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    //if free the compressed surface, need set the sync dealloc flag as 1 for sync dealloc for aux table update
    MOS_GFXRES_FREE_FLAGS resFreeFlags = {0};
    resFreeFlags.SynchronousDestroy = m_allocator->isSyncFreeNeededForMMCSurface(surface) ? 1 : 0;
    DECODE_NORMALMESSAGE("SynchronousDestroy flag (%d) for surface\n", resFreeFlags.SynchronousDestroy);

    DECODE_CHK_STATUS(m_allocator->DestroySurface(surface, resFreeFlags));
    surface = nullptr;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeAllocator::Destroy(MOS_SURFACE& surface)
{
    DECODE_CHK_NULL(m_allocator);

    MOS_SURFACE* dup = MOS_New(MOS_SURFACE);
    DECODE_CHK_NULL(dup);
    MOS_STATUS status = MOS_SecureMemcpy(dup, sizeof(MOS_SURFACE), &surface, sizeof(MOS_SURFACE));
    if (status != MOS_STATUS_SUCCESS)
    {
        MOS_Delete(dup);
        return status;
    }

    //if free the compressed surface, need set the sync dealloc flag as 1 for sync dealloc for aux table update
    MOS_GFXRES_FREE_FLAGS resFreeFlags = {0};
    resFreeFlags.SynchronousDestroy = m_allocator->isSyncFreeNeededForMMCSurface(dup) ? 1 : 0;
    DECODE_NORMALMESSAGE("SynchronousDestroy flag (%d) for surface\n", resFreeFlags.SynchronousDestroy);

    DECODE_CHK_STATUS(m_allocator->DestroySurface(dup, resFreeFlags));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeAllocator::Destroy(BufferArray* & bufferArray)
{
    DECODE_CHK_NULL(m_allocator);
    if (bufferArray == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_Delete(bufferArray);
    bufferArray = nullptr;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeAllocator::Destroy(SurfaceArray* & surfaceArray)
{
    DECODE_CHK_NULL(m_allocator);
    if (surfaceArray == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_Delete(surfaceArray);
    surfaceArray = nullptr;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeAllocator::Destroy(PMHW_BATCH_BUFFER &batchBuffer)
{
    if (batchBuffer == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    DECODE_CHK_STATUS(Mhw_FreeBb(m_osInterface, batchBuffer, nullptr));
    MOS_Delete(batchBuffer);
    batchBuffer = nullptr;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeAllocator::Destroy(BatchBufferArray* & batchBufferArray)
{
    DECODE_CHK_NULL(m_allocator);
    if (batchBufferArray == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_Delete(batchBufferArray);
    batchBufferArray = nullptr;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeAllocator::DestroyAllResources()
{
    DECODE_CHK_NULL(m_allocator);

    return m_allocator->DestroyAllResources();
}

MOS_STATUS DecodeAllocator::SyncOnResource(MOS_RESOURCE* resource, bool IsWriteOperation)
{
    DECODE_CHK_NULL(resource);

    MOS_GPU_CONTEXT gpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);
    m_osInterface->pfnSyncOnResource(m_osInterface, resource, gpuContext, IsWriteOperation ? 1 : 0);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeAllocator::SyncOnResource(MOS_BUFFER* buffer, bool IsWriteOperation)
{
    DECODE_CHK_NULL(buffer);

    MOS_GPU_CONTEXT gpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);
    m_osInterface->pfnSyncOnResource(m_osInterface, &buffer->OsResource, gpuContext, IsWriteOperation ? 1 : 0);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeAllocator::SkipResourceSync(MOS_RESOURCE *resource)
{
    DECODE_CHK_NULL(m_allocator);

    return m_allocator->SkipResourceSync(resource);
}

MOS_STATUS DecodeAllocator::SkipResourceSync(MOS_BUFFER *buffer)
{
    DECODE_CHK_NULL(m_allocator);
    DECODE_CHK_NULL(buffer);

    return m_allocator->SkipResourceSync(&buffer->OsResource);
}

bool DecodeAllocator::ResourceIsNull(MOS_RESOURCE* resource)
{
    return Mos_ResourceIsNull(resource);
}

MOS_STATUS DecodeAllocator::GetSurfaceInfo(PMOS_SURFACE surface)
{
    DECODE_CHK_NULL(m_allocator);
    DECODE_CHK_NULL(surface);

    surface->Format       = Format_Invalid;
    surface->dwArraySlice = 0;
    surface->dwMipSlice   = 0;
    surface->S3dChannel   = MOS_S3D_NONE;
    DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&surface->OsResource, surface));

    return MOS_STATUS_SUCCESS;

}

MOS_STATUS DecodeAllocator::UpdateResoreceUsageType(
    PMOS_RESOURCE osResource,
    ResourceUsage resUsageType)
{
    DECODE_CHK_NULL(m_allocator);

    return (m_allocator->UpdateResourceUsageType(osResource, static_cast<MOS_HW_RESOURCE_DEF>(resUsageType)));
}

MOS_STATUS DecodeAllocator::RegisterResource(PMOS_RESOURCE osResource)
{
    DECODE_CHK_NULL(osResource);

    return(m_osInterface->pfnRegisterResource(
            m_osInterface,
            osResource,
            false,
            false));
}

ResourceUsage DecodeAllocator::ConvertGmmResourceUsage(const GMM_RESOURCE_USAGE_TYPE gmmResUsage)
{
    if (nullptr == m_osInterface)
    {
        DECODE_ASSERTMESSAGE("mos interface is nullptr")
        return resourceDefault;
    }
    MOS_HW_RESOURCE_DEF gmmUsage = m_osInterface->pfnGmmToMosResourceUsageType(gmmResUsage);
    return (ResourceUsage)gmmUsage;
}

void DecodeAllocator::SetAccessRequirement(
    ResourceAccessReq accessReq, MOS_ALLOC_GFXRES_PARAMS &allocParams)
{
    // The default setting is lockableVideoMem, just use default setting
    // if not running with limited LMem bar config and not enabled HM. otherwise goto required setting.
    if (!m_limitedLMemBar && 
        !(m_osInterface->osCpInterface != nullptr && m_osInterface->osCpInterface->IsHMEnabled())
    )
    {
        allocParams.Flags.bNotLockable = 0;
        allocParams.dwMemType = MOS_MEMPOOL_VIDEOMEMORY;
        return;
    }

    allocParams.Flags.bNotLockable = (accessReq == notLockableVideoMem) ? 1 : 0;

    if (accessReq == lockableSystemMem)
    {
        allocParams.dwMemType = MOS_MEMPOOL_SYSTEMMEMORY;
    }
    else if (accessReq == notLockableVideoMem)
    {
        allocParams.dwMemType = MOS_MEMPOOL_DEVICEMEMORY;
    }
    else
    {
        allocParams.dwMemType = MOS_MEMPOOL_VIDEOMEMORY;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_forceLockable)
    {
        allocParams.Flags.bNotLockable = 0;

        if (allocParams.dwMemType == MOS_MEMPOOL_DEVICEMEMORY)
        {
            allocParams.dwMemType = MOS_MEMPOOL_VIDEOMEMORY;
        }
    }
#endif
}

}
