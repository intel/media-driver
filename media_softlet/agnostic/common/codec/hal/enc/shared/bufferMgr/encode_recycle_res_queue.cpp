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
//! \file     encode_recycle_res_queue.cpp
//! \brief    Defines the interface for resource queue
//!

#include "encode_recycle_res_queue.h"
#include "encode_allocator.h"
#include "encode_utils.h"
#include "mos_os_specific.h"
#include "mos_utilities.h"

namespace encode {

RecycleQueue::RecycleQueue(const MOS_ALLOC_GFXRES_PARAMS &param,
    EncodeAllocator *allocator,
    uint32_t maxLimit) :
    m_maxLimit(maxLimit),
    m_allocator(allocator)

{
    MOS_SecureMemcpy(
        &m_param,
        sizeof(MOS_ALLOC_GFXRES_PARAMS),
        &param,
        sizeof(MOS_ALLOC_GFXRES_PARAMS));
}

RecycleQueue::~RecycleQueue()
{
    m_resources.clear();
}

void *RecycleQueue::GetResource(uint32_t frameIndex, ResourceType type)
{
    if (m_allocator == nullptr)
    {
        return nullptr;
    }

    uint32_t currIndex = frameIndex % m_maxLimit;

    while (currIndex >= m_resources.size())
    {
        m_type = type;

        void *resource = nullptr;

        if (m_type == ResourceType::SURFACE)
        {
            resource = m_allocator->AllocateSurface(m_param, true);
        }
        else if (m_type == ResourceType::BUFFER)
        {
            resource = m_allocator->AllocateResource(m_param, true);
        }
        else
        {
            return nullptr;
        }
        
        m_resources.push_back(resource);
    }

    void *res = m_resources[currIndex];

    return res;
}

MOS_STATUS RecycleQueue::DestroyAllResources(EncodeAllocator *allocator)
{
    ENCODE_CHK_NULL_RETURN(allocator);

    for (auto res : m_resources)
    {
        if (res == nullptr)
        {
            continue;
        }

        if (m_type == SURFACE)
        {
            ENCODE_CHK_STATUS_RETURN(allocator->DestroySurface((MOS_SURFACE *)res));
        }
        else if (m_type == BUFFER)
        {
             ENCODE_CHK_STATUS_RETURN(allocator->DestroyResource((MOS_RESOURCE *)res));
        }
    }

    m_resources.clear();

    return MOS_STATUS_SUCCESS;
}

}