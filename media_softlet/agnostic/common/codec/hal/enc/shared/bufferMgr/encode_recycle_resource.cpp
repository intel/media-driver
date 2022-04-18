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
//! \file     encode_recycle_resource.cpp
//! \brief    Defines the interface for recycle resource
//! \details  The manager for the recycle resources
//!

#include "encode_recycle_resource.h"
#include "encode_recycle_res_queue.h"
#include "mos_utilities.h"

namespace encode {

#define CHK_NULL_RETURN(ptr) \
    if ((ptr) == nullptr)    \
    {                        \
        return nullptr;      \
    }

#define CHK_NULL_RETURN_STATUS(ptr) \
    if ((ptr) == nullptr)    \
    {                        \
        return MOS_STATUS_INVALID_PARAMETER;      \
    }

#define CHK_STATUS_RETURN(sts)       \
    if ((sts) != MOS_STATUS_SUCCESS) \
    {                                \
        return nullptr;              \
    }

RecycleResource::RecycleResource(EncodeAllocator *allocator)
    : m_allocator(allocator)
{
}

RecycleResource::~RecycleResource()
{
    for (auto pair : m_resourceQueues)
    {
        auto que = pair.second;
        que->DestroyAllResources(m_allocator);
        MOS_Delete(que);
    }
    m_resourceQueues.clear();
}

MOS_STATUS RecycleResource::RegisterResource(
    RecycleResId id, 
    MOS_ALLOC_GFXRES_PARAMS param, 
    uint32_t maxLimit)
{
    auto it = m_resourceQueues.find(id);
    
    if (it != m_resourceQueues.end())
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    RecycleQueue *que = MOS_New(RecycleQueue, param, m_allocator, maxLimit);
    if (que == nullptr)
    {
        return MOS_STATUS_CLIENT_AR_NO_SPACE;
    }

    m_resourceQueues.insert(std::make_pair(id, que));

    return MOS_STATUS_SUCCESS;
}

MOS_SURFACE *RecycleResource::GetSurface(RecycleResId id, uint32_t frameIndex)
{
    CHK_NULL_RETURN(m_allocator);
    
    RecycleQueue *que = GetResQueue(id);
    CHK_NULL_RETURN(que);

    if (!que->IsTypeMatched(RecycleQueue::SURFACE))
    {
        return nullptr;
    }

    void *surface = que->GetResource(frameIndex, RecycleQueue::SURFACE);
    
    return (MOS_SURFACE *)surface;
}

MOS_RESOURCE *RecycleResource::GetBuffer(RecycleResId id, uint32_t frameIndex)
{
    CHK_NULL_RETURN(m_allocator);

    RecycleQueue *que = GetResQueue(id);
    CHK_NULL_RETURN(que);

    if (!que->IsTypeMatched(RecycleQueue::BUFFER))
    {
        return nullptr;
    }

    void *buffer = que->GetResource(frameIndex, RecycleQueue::BUFFER);

    return (MOS_RESOURCE *)buffer;
}

}