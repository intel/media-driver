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
//! \file     codechal_allocator.cpp
//! \brief    Class that provides a generic resource allocation service
//!

#include "codechal_allocator.h"

uint64_t CodechalAllocator::GetResourceTag(uint16_t resourceID, Match level)
{
    if (!m_resourceList.empty())
    {
        for (auto& res : m_resourceList)
        {
            if (resourceID == GetResourceID(res.first, level))
                return res.first;
        }
    }

    return 0;
}

void* CodechalAllocator::GetResourcePointer(uint16_t resourceID, Match level)
{
    if (!m_resourceList.empty())
    {
        for (auto& res : m_resourceList)
        {
            if (resourceID == GetResourceID(res.first, level))
                return res.second;
        }
    }

    return nullptr;
}

void* CodechalAllocator::Allocate1DBuffer(uint64_t resourceTag, uint32_t size,
    bool zeroOnAllocation, const char *bufName)
{
    MOS_RESOURCE* resource = MOS_New(MOS_RESOURCE);
    MOS_ZeroMemory(resource, sizeof(MOS_RESOURCE));

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(allocParams));
    allocParams.Type = MOS_GFXRES_BUFFER;
    allocParams.Format = Format_Buffer;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.dwBytes = size;
    allocParams.pBufName = bufName;

    if (MOS_STATUS_SUCCESS != m_osInterface->pfnAllocateResource(
        m_osInterface, &allocParams, resource))
    {
        MOS_Delete(resource);
        MOS_OS_ASSERTMESSAGE("allocate 1D buffer error!");
        return nullptr;
    }

    // place the newly allocated resource onto the list
    m_resourceList[resourceTag] = resource;
    MOS_OS_NORMALMESSAGE("allocate 1D buffer = 0x%x, tag = 0x%llx", resource, (long long)resourceTag);

    if (zeroOnAllocation)
    {
        ClearResource(resource, size);
    }

    return resource;
}

void* CodechalAllocator::Allocate2DBuffer(
    uint64_t resourceTag, uint32_t width, uint32_t height, MOS_FORMAT format,
    MOS_TILE_TYPE tile, bool zeroOnAllocation, const char *bufName)
{
    MOS_SURFACE* surface = MOS_New(MOS_SURFACE);
    MOS_ZeroMemory(surface, sizeof(MOS_SURFACE));

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(allocParams));
    allocParams.Type = MOS_GFXRES_2D;
    allocParams.Format = format;
    allocParams.TileType = tile;
    allocParams.dwWidth = width;
    allocParams.dwHeight = height;
    allocParams.pBufName = bufName;

    if (MOS_STATUS_SUCCESS != m_osInterface->pfnAllocateResource(
        m_osInterface, &allocParams, &surface->OsResource))
    {
        MOS_Delete(surface);
        MOS_OS_ASSERTMESSAGE("allocate 2D buffer error!");
        return nullptr;
    }

    // place the newly allocated resource onto the list
    m_resourceList[resourceTag] = surface;
    MOS_OS_NORMALMESSAGE("allocate 2D buffer = 0x%x, tag = 0x%llx", surface, (long long)resourceTag);

    if (zeroOnAllocation)
    {
        ClearResource(&surface->OsResource, width * height);
    }

    return surface;
}

void* CodechalAllocator::AllocateBatchBuffer(uint64_t resourceTag, uint32_t size, bool zeroOnAllocation)
{
    MHW_BATCH_BUFFER* batch = MOS_New(MHW_BATCH_BUFFER);
    MOS_ZeroMemory(batch, sizeof(MHW_BATCH_BUFFER));

    if (MOS_STATUS_SUCCESS != Mhw_AllocateBb(m_osInterface, batch, nullptr, size))
    {
        MOS_Delete(batch);
        MOS_OS_ASSERTMESSAGE("allocate batch buffer error!");
        return nullptr;
    }

    // place the newly allocated resource onto the list
    m_resourceList[resourceTag] = batch;
    MOS_OS_NORMALMESSAGE("allocate batch buffer = 0x%x, tag = 0x%llx", batch, (long long)resourceTag);

    if (zeroOnAllocation)
    {
        Mhw_LockBb(m_osInterface, batch);

        MOS_ZeroMemory(batch->pData, size);

        Mhw_UnlockBb(m_osInterface, batch, false);
    }

    return batch;
}

void CodechalAllocator::ReleaseResource(uint16_t resourceID, Match level)
{
    if (!m_resourceList.empty())
    {
        uint64_t tag = 0;
        void* pointer = nullptr;

        for (auto& res : m_resourceList)
        {
            if (resourceID == GetResourceID(res.first, level))
            {
                tag = res.first;
                pointer = res.second;
                break;
            }
        }

        if (tag)
        {
            // deallocate the matched resource
            Deallocate(tag, pointer);

            // remove from resource list
            m_resourceList.erase(tag);
        }
    }
}

bool CodechalAllocator::Is1DBuffer(uint64_t resourceTag)
{
    uint16_t typeID = (uint16_t)resourceTag;
    return (typeID >> 14 & 3) == 0;
}

bool CodechalAllocator::Is2DBuffer(uint64_t resourceTag)
{
    uint16_t typeID = (uint16_t)resourceTag;
    return (typeID >> 14 & 3) == 1;
}

bool CodechalAllocator::IsBatchBuffer(uint64_t resourceTag)
{
    uint16_t typeID = (uint16_t)resourceTag;
    return (typeID >> 14 & 3) == 2;
}

void CodechalAllocator::ClearResource(MOS_RESOURCE* resource, size_t size)
{
    MOS_LOCK_PARAMS LockFlags;
    MOS_ZeroMemory(&LockFlags, sizeof(LockFlags));
    LockFlags.WriteOnly = 1;

    uint8_t* ptr = (uint8_t*)m_osInterface->pfnLockResource(
        m_osInterface, resource, &LockFlags);

    MOS_ZeroMemory(ptr, size);

    m_osInterface->pfnUnlockResource(m_osInterface, resource);
}

void CodechalAllocator::Deallocate(uint64_t tag, void* pointer)
{
    if (Is1DBuffer(tag))
    {
        MOS_RESOURCE* ptr = (MOS_RESOURCE*)pointer;
        m_osInterface->pfnFreeResource(m_osInterface, ptr);
        MOS_OS_NORMALMESSAGE("free 1D buffer = 0x%x", ptr);

        // free resource container
        MOS_Delete(ptr);
    }
    else if (Is2DBuffer(tag))
    {
        MOS_SURFACE* ptr = (MOS_SURFACE*)pointer;
        m_osInterface->pfnFreeResource(m_osInterface, &ptr->OsResource);
        MOS_OS_NORMALMESSAGE("free 2D buffer = 0x%x", ptr);

        // free resource container
        MOS_Delete(ptr);
    }
    else if (IsBatchBuffer(tag))
    {
        MHW_BATCH_BUFFER* ptr = (MHW_BATCH_BUFFER*)pointer;
        Mhw_FreeBb(m_osInterface, ptr, nullptr);
        MOS_OS_NORMALMESSAGE("free batch buffer = 0x%x", ptr);

        // free resource container
        MOS_Delete(ptr);
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Unknown resource = 0x%x", pointer);
    }
}

CodechalAllocator::CodechalAllocator(MOS_INTERFACE* osInterface)
    : m_osInterface(osInterface),
    m_resourceList{}
{
}

CodechalAllocator::~CodechalAllocator()
{
    if (!m_resourceList.empty())
    {
        for (auto& res : m_resourceList)
        {
            Deallocate(res.first, res.second);
        }

        m_resourceList.clear();
    }
}
