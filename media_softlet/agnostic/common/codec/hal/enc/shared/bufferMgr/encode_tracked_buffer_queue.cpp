/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     encode_buffer_allocator.cpp
//! \brief    Defines the interface for buffer allocator
//! \details  The allocator manages the buffers with the same type and alloc parameter
//!
#include "encode_tracked_buffer_queue.h"
#include <algorithm>
#include "encode_allocator.h"
#include "encode_utils.h"
#include "mos_os_hw.h"
#include "mos_os_specific.h"
#include "mos_utilities.h"

namespace encode {

BufferQueue::BufferQueue(EncodeAllocator *allocator, MOS_ALLOC_GFXRES_PARAMS &param, uint32_t maxCount)
    : m_maxCount(maxCount),
      m_allocator(allocator),
      m_allocParam(param)
{
    m_mutex = MosUtilities::MosCreateMutex();
}

BufferQueue::~BufferQueue()
{
    for (auto resource : m_resources)
    {
        DestoryResource(resource);
    }

    MosUtilities::MosDestroyMutex(m_mutex);
}

void *BufferQueue::AcquireResource()
{
    AutoLock lock(m_mutex);

    if (m_resourcePool.empty())
    {
        if (m_allocCount > m_maxCount)
        {
            ENCODE_VERBOSEMESSAGE("Reach max resource count, cannot allocate more");
            return nullptr;
        }
        
        void *resource = AllocateResource();
        if (resource != nullptr)
        {
            m_allocCount++;
            m_resources.push_back(resource);
        }
        return resource;
    }
    else
    {
        void *resource = m_resourcePool.back();
        m_resourcePool.pop_back();
        return resource;
    }
}

MOS_STATUS BufferQueue::ReleaseResource(void *resource)
{
    AutoLock lock(m_mutex);

    if (nullptr != resource)
    {
        if (std::find(m_resources.begin(), m_resources.end(), resource) == m_resources.end())
        {
            // resource not belong to this allocator
            ENCODE_VERBOSEMESSAGE("resource not belonged to current allocator");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        if (std::find(m_resourcePool.begin(), m_resourcePool.end(), resource) != m_resourcePool.end())
        {
            // resource already released
            ENCODE_VERBOSEMESSAGE("resource already returned");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        m_resourcePool.push_back(resource);
    }
    return MOS_STATUS_SUCCESS;
}

bool BufferQueue::SafeToDestory()
{ 
    AutoLock lock(m_mutex);

    return m_resourcePool.size() == m_resources.size();
}


void *BufferQueue::AllocateResource()
{
    if (m_allocator)
    {
        if (m_resourceType == ResourceType::surfaceResource)
        {
            MOS_SURFACE* surface = nullptr;
            surface = m_allocator->AllocateSurface(m_allocParam, false, MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE);
            m_allocator->GetSurfaceInfo(surface);
            return surface;
        }
        else if (m_resourceType == ResourceType::bufferResource)
        {
            return m_allocator->AllocateResource(m_allocParam, true);
        }
        else
        {
            return nullptr;
        }

    }
    else
    {
        return nullptr;
    }
}

MOS_STATUS BufferQueue::DestoryResource(void* resource)
{
    if (nullptr != resource && nullptr != m_allocator)
    {
        if (m_resourceType == ResourceType::surfaceResource)
        {
            m_allocator->DestroySurface((MOS_SURFACE *)resource);
        }
        else if (m_resourceType == ResourceType::bufferResource)
        {
            m_allocator->DestroyResource((MOS_RESOURCE *)resource);
        }
    }
    return MOS_STATUS_SUCCESS;
}

void BufferQueue::SetResourceType(ResourceType resType)
{ 
    m_resourceType = resType; 
}

}