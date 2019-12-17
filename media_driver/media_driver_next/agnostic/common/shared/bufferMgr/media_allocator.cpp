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
//! \file     media_allocator.cpp
//! \brief    Defines the common interface for media resouce manage
//! \details  Media allocator will allocate and destory buffers, the caller
//!           can use directly
//!
#include <algorithm>
#include "media_allocator.h"

Allocator::Allocator(PMOS_INTERFACE osInterface) : m_osInterface(osInterface)
{

}

Allocator::~Allocator()
{
    DestroyAllResources();
}

MOS_STATUS Allocator::DestroyAllResources()
{
#if (_DEBUG || _RELEASE_INTERNAL)
    for (auto it : m_resourcePool)
    {
        MOS_RESOURCE *resource = const_cast<MOS_RESOURCE *>(it.first);
        m_osInterface->pfnFreeResource(m_osInterface, resource);
        MOS_Delete(resource);
        MOS_Delete(it.second);
    }

    m_resourcePool.clear();

    for (auto it : m_surfacePool)
    {
        MOS_SURFACE *surface = const_cast<MOS_SURFACE *>(it.first);
        if (surface)
        {
            m_osInterface->pfnFreeResource(m_osInterface, &(surface->OsResource));
        }
        MOS_Delete(surface);
        MOS_Delete(it.second);
    }

    m_surfacePool.clear();
#else

    for (auto it : m_resourcePool)
    {
        m_osInterface->pfnFreeResource(m_osInterface, it);
        MOS_Delete(it);
    }
    m_resourcePool.clear();

    for (auto it : m_surfacePool)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &it->OsResource);
        MOS_Delete(it);
    }
    m_surfacePool.clear();

#endif
    return MOS_STATUS_SUCCESS;
}

MOS_RESOURCE *Allocator::AllocateResource(MOS_ALLOC_GFXRES_PARAMS &param, bool zeroOnAllocate, MOS_COMPONENT component)
{
    if (nullptr == m_osInterface)
    {
        return nullptr;
    }

    MOS_RESOURCE *resource = MOS_New(MOS_RESOURCE);
    memset(resource, 0, sizeof(MOS_RESOURCE));
    MOS_STATUS status = m_osInterface->pfnAllocateResource(m_osInterface, &param, resource);

    if (status != MOS_STATUS_SUCCESS)
    {
        MOS_Delete(resource);
        return nullptr;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    TraceInfo *info = MOS_New(TraceInfo);
    info->component = component;
    //Note, param.pBufName cannot be null.
    //This assignment statement will calcualte the string length and copy param.pBufName to info->name.
    //If param.pBufName is null, exception happens when to calculate the string length.
    MOS_OS_ASSERT(param.pBufName != nullptr);
    info->name     = param.pBufName;

    m_resourcePool.insert(std::make_pair(resource, info));
#else
    m_resourcePool.push_back(resource);
#endif

    if (zeroOnAllocate)
    {
        ClearResource(resource, param);
    }
    return resource;
}

PMOS_BUFFER Allocator::AllocateBuffer(MOS_ALLOC_GFXRES_PARAMS &param, bool zeroOnAllocate, MOS_COMPONENT component)
{
    if (nullptr == m_osInterface)
    {
        return nullptr;
    }

    MOS_BUFFER *buffer = MOS_New(MOS_BUFFER);
    memset(buffer, 0, sizeof(MOS_BUFFER));
    MOS_STATUS status = m_osInterface->pfnAllocateResource(m_osInterface, &param, &buffer->OsResource);

    if (status != MOS_STATUS_SUCCESS)
    {
        MOS_Delete(buffer);
        return nullptr;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    TraceInfo *info = MOS_New(TraceInfo);
    info->component = component;
    //Note, param.pBufName cannot be null.
    //This assignment statement will calcualte the string length and copy param.pBufName to info->name.
    //If param.pBufName is null, exception happens when to calculate the string length.
    MOS_OS_ASSERT(param.pBufName != nullptr);
    info->name     = param.pBufName;

    m_resourcePool.insert(std::make_pair(&buffer->OsResource, info));
#else
    m_resourcePool.push_back(&buffer->OsResource);
#endif

    if (zeroOnAllocate)
    {
        ClearResource(&buffer->OsResource, param);
    }
    return buffer;
}

MOS_SURFACE *Allocator::AllocateSurface(MOS_ALLOC_GFXRES_PARAMS &param, bool zeroOnAllocate, MOS_COMPONENT component)
{
    MOS_SURFACE *surface = MOS_New(MOS_SURFACE);
    if (nullptr == surface)
    {
        return nullptr;
    }
    MOS_STATUS status = m_osInterface->pfnAllocateResource(m_osInterface, &param, &surface->OsResource);

    m_osInterface->pfnGetResourceInfo(m_osInterface, &surface->OsResource, surface);
#if (_DEBUG || _RELEASE_INTERNAL)
    TraceInfo *info = MOS_New(TraceInfo);
    info->component = component;
    MOS_OS_ASSERT(param.pBufName != nullptr);
    info->name      = param.pBufName;

    m_surfacePool.insert(std::make_pair(surface, info));
#else
    m_surfacePool.push_back(surface);
#endif

    if (zeroOnAllocate)
    {
        ClearResource(&surface->OsResource, param);
    }
    return surface;
}

MOS_STATUS Allocator::DestroyResource(MOS_RESOURCE *resource)
{
    if (nullptr == resource)
    {
        return MOS_STATUS_NULL_POINTER;
    }
#if (_DEBUG || _RELEASE_INTERNAL)
    auto it = m_resourcePool.find(resource);
    if (it == m_resourcePool.end())
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_Delete(it->second);
#else
    auto it = std::find(m_resourcePool.begin(), m_resourcePool.end(), resource);
    if (it == m_resourcePool.end())
    {
        return MOS_STATUS_SUCCESS;
    }
#endif

    m_resourcePool.erase(it);
    m_osInterface->pfnFreeResource(m_osInterface, resource);
    MOS_Delete(resource);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Allocator::DestroyBuffer(MOS_BUFFER *buffer)
{
    if (nullptr == buffer)
    {
        return MOS_STATUS_NULL_POINTER;
    }
#if (_DEBUG || _RELEASE_INTERNAL)
    auto it = m_resourcePool.find(&buffer->OsResource);
    if (it == m_resourcePool.end())
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_Delete(it->second);
#else
    auto it = std::find(m_resourcePool.begin(), m_resourcePool.end(), &buffer->OsResource);
    if (it == m_resourcePool.end())
    {
        return MOS_STATUS_SUCCESS;
    }
#endif

    m_resourcePool.erase(it);
    m_osInterface->pfnFreeResource(m_osInterface, &buffer->OsResource);
    MOS_Delete(buffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Allocator::DestroySurface(MOS_SURFACE *surface)
{
    if (nullptr == surface)
    {
        return MOS_STATUS_NULL_POINTER;
    }
#if (_DEBUG || _RELEASE_INTERNAL)
    auto it = m_surfacePool.find(surface);
    if (it == m_surfacePool.end())
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_Delete(it->second);
#else
    auto it = std::find(m_surfacePool.begin(), m_surfacePool.end(), surface);
    if (it == m_surfacePool.end())
    {
        return MOS_STATUS_SUCCESS;
    }
#endif

    m_surfacePool.erase(it);
    m_osInterface->pfnFreeResource(m_osInterface, &surface->OsResource);
    MOS_Delete(surface);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Allocator::AllocateResource(MOS_RESOURCE *res, MOS_ALLOC_GFXRES_PARAMS &param)
{
    if (nullptr == m_osInterface || nullptr == res)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    memset(res, 0, sizeof(MOS_RESOURCE));
    MOS_STATUS status = m_osInterface->pfnAllocateResource(m_osInterface, &param, res);

    return status;
}

MOS_STATUS Allocator::FreeResource(MOS_RESOURCE *res)
{
    if (nullptr == m_osInterface)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    m_osInterface->pfnFreeResource(m_osInterface, res);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Allocator::ClearResource(MOS_RESOURCE *resource, MOS_ALLOC_GFXRES_PARAMS &param)
{
    MOS_LOCK_PARAMS lockFlag;
    memset(&lockFlag, 0, sizeof(lockFlag));
    lockFlag.WriteOnly = true;

    uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, resource, &lockFlag);
    if (data == 0)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    if (param.Format == Format_Buffer)
    {
        memset(data, 0, param.dwBytes);
    }
    else if (param.Format == Format_Buffer_2D)
    {
        memset(data, 0, param.dwHeight * param.dwWidth);
    }
    else if (param.Format == Format_NV12)
    {
        memset(data, 0, param.dwHeight * param.dwWidth);
    }
    else
    {
        m_osInterface->pfnUnlockResource(m_osInterface, resource);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, resource);

    return MOS_STATUS_SUCCESS;
}

void* Allocator::Lock(MOS_RESOURCE* resource, MOS_LOCK_PARAMS* lockFlag)
{
    if (nullptr == resource || nullptr == lockFlag)
    {
        return nullptr;
    }

    return (m_osInterface->pfnLockResource(m_osInterface, resource, lockFlag));
}

MOS_STATUS Allocator::UnLock(MOS_RESOURCE* resource)
{
    if (nullptr == resource)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return (m_osInterface->pfnUnlockResource(m_osInterface, resource));
}

MOS_STATUS Allocator::SkipResourceSync(MOS_RESOURCE* resource)
{
    if (nullptr == resource)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return (m_osInterface->pfnSkipResourceSync(resource));
}

MOS_STATUS Allocator::GetSurfaceInfo(PMOS_RESOURCE osResource, PMOS_SURFACE resDetails)
{
    return m_osInterface->pfnGetResourceInfo(m_osInterface, osResource, resDetails);
}

MOS_STATUS Allocator::OsFillResource(PMOS_RESOURCE osResource, uint32_t size, uint8_t value)
{
    return m_osInterface->pfnFillResource(m_osInterface, osResource, size, value);
}

MOS_STATUS Allocator::SyncOnResource(
    PMOS_RESOURCE         osResource,
    bool                  bWriteOperation)
{
    MOS_GPU_CONTEXT requestorGPUCtx = m_osInterface->pfnGetGpuContext(m_osInterface);
    m_osInterface->pfnSyncOnResource(m_osInterface, osResource, requestorGPUCtx, bWriteOperation);
    return MOS_STATUS_SUCCESS;
}
