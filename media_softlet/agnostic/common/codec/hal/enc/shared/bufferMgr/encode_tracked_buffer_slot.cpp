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
//! \file     encode_buffer_slot.cpp
//! \brief    Defines the interface for buffer slot
//! \details  The slot manages the buffers with the same type
//!
#include "encode_tracked_buffer_slot.h"
#include <utility>
#include "encode_tracked_buffer_queue.h"

namespace encode {

BufferSlot::BufferSlot(TrackedBuffer* tracker) :
    m_tracker(tracker)
{
}

BufferSlot::~BufferSlot()
{
    for (auto iter = m_buffers.begin(); iter != m_buffers.end(); iter++)
    {
        std::shared_ptr<BufferQueue> queue = m_bufferQueues[iter->first];
        queue->ReleaseResource(iter->second);
    }
    m_buffers.clear();
    m_bufferQueues.clear();
}

MOS_STATUS BufferSlot::Reset()
{
    m_isBusy = false;
    for (auto iter = m_buffers.begin(); iter != m_buffers.end(); iter++)
    {
        std::shared_ptr<BufferQueue> queue = m_bufferQueues[iter->first];
        queue->ReleaseResource(iter->second);
    }
    m_buffers.clear();
    m_bufferQueues.clear();

    return MOS_STATUS_SUCCESS;
}

void *BufferSlot::GetResource(BufferType type)
{
    // when the slot is in free status, should not return any surfaces
    if (IsFree())
    {
        return nullptr;
    }

    // if surface already in the pool, return it directly
    auto iter = m_buffers.find(type);
    if (iter != m_buffers.end())
    {
        return iter->second;
    }

    std::shared_ptr<BufferQueue> queue = m_tracker->GetBufferQueue(type);
    if (queue == nullptr)
    {
        return nullptr;
    }

    void* resource = queue->AcquireResource();
    // record the surface acquired, only one surface for each type should be kept in the slot
    m_buffers.insert(std::make_pair(type, resource));
    m_bufferQueues.insert(std::make_pair(type, queue));
    return resource;
}

}