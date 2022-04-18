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
//! \file     encode_allocator.cpp
//! \brief    Defines the interface for encode resource allocate
//! \details  encode allocator will allocate and destory buffers, the caller
//!           can use directly
//!

#include "encode_allocator.h"
#include "encode_utils.h"
#include "media_allocator.h"
#include "mos_resource_defs.h"
#include "mos_utilities.h"

namespace encode {

EncodeAllocator::EncodeAllocator(PMOS_INTERFACE osInterface) :
    m_osInterface(osInterface)
{
    m_allocator = MOS_New(Allocator, m_osInterface);
}

EncodeAllocator::~EncodeAllocator()
{
    MOS_Delete(m_allocator);
}

MOS_RESOURCE* EncodeAllocator::AllocateResource(
    MOS_ALLOC_GFXRES_PARAMS &param,
    bool zeroOnAllocate,
    MOS_HW_RESOURCE_DEF resUsageType)
{
    if (!m_allocator)
        return nullptr;

    if (param.ResUsageType == MOS_HW_RESOURCE_DEF_MAX)
    {
        param.ResUsageType = resUsageType;
    }

    return m_allocator->AllocateResource(param, zeroOnAllocate, COMPONENT_Encode);
}

MOS_SURFACE* EncodeAllocator::AllocateSurface(
    MOS_ALLOC_GFXRES_PARAMS &param,
    bool zeroOnAllocate,
    MOS_HW_RESOURCE_DEF ResUsageType)
{
    if (!m_allocator)
        return nullptr;

    param.ResUsageType = ResUsageType;
    return m_allocator->AllocateSurface(param, zeroOnAllocate, COMPONENT_Encode);
}

MOS_STATUS EncodeAllocator::DestroyResource(MOS_RESOURCE* resource)
{
    ENCODE_CHK_NULL_RETURN(m_allocator);

    return m_allocator->DestroyResource(resource);
}

MOS_STATUS EncodeAllocator::DestroyAllResources()
{
    ENCODE_CHK_NULL_RETURN(m_allocator);

    return m_allocator->DestroyAllResources();
}

MOS_STATUS EncodeAllocator::DestroySurface(MOS_SURFACE* surface)
{
    ENCODE_CHK_NULL_RETURN(m_allocator);

    return m_allocator->DestroySurface(surface);
}

void* EncodeAllocator::Lock(MOS_RESOURCE* resource, MOS_LOCK_PARAMS* lockFlag)
{
    if (!m_allocator)
        return nullptr;

    return m_allocator->Lock(resource, lockFlag);
}

void* EncodeAllocator::LockResourceForWrite(MOS_RESOURCE* resource)
{
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;

    if (!m_allocator)
        return nullptr;

    return m_allocator->Lock(resource, &lockFlags);
}

void* EncodeAllocator::LockResourceWithNoOverwrite(MOS_RESOURCE* resource)
{
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly   = 1;
    lockFlags.NoOverWrite = 1;

    if (!m_allocator)
        return nullptr;

    return m_allocator->Lock(resource, &lockFlags);
}

void* EncodeAllocator::LockResourceForRead(MOS_RESOURCE* resource)
{
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;

    if (!m_allocator)
        return nullptr;

    return m_allocator->Lock(resource, &lockFlags);
}

MOS_STATUS EncodeAllocator::UnLock(MOS_RESOURCE* resource)
{
    ENCODE_CHK_NULL_RETURN(m_allocator);

    return m_allocator->UnLock(resource);
}

MOS_STATUS EncodeAllocator::SkipResourceSync(MOS_RESOURCE *resource)
{
    ENCODE_CHK_NULL_RETURN(m_allocator);

    return m_allocator->SkipResourceSync(resource);
}

MOS_STATUS EncodeAllocator::GetSurfaceInfo(PMOS_SURFACE surface)
{
    ENCODE_CHK_NULL_RETURN(m_allocator);
    ENCODE_CHK_NULL_RETURN(surface);

    surface->Format = Format_Invalid;
    surface->dwArraySlice = 0;
    surface->dwMipSlice   = 0;
    surface->S3dChannel   = MOS_S3D_NONE;
    ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(&surface->OsResource, surface));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAllocator::UpdateResourceUsageType(PMOS_RESOURCE osResource, MOS_HW_RESOURCE_DEF resUsageType)
{
    ENCODE_CHK_NULL_RETURN(m_allocator);

    return (m_allocator->UpdateResourceUsageType(osResource, resUsageType));
}

MOS_STATUS EncodeAllocator::SyncOnResource(
    PMOS_RESOURCE osResource,
    bool bWriteOperation)
{
    ENCODE_CHK_NULL_RETURN(m_allocator);

    return (m_allocator->SyncOnResource(osResource, bWriteOperation));
}
}
