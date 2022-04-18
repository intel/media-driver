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
//! \file     encode_buffer_tracker.cpp
//! \brief    Defines the interface for buffer tracker
//! \details  The tracker manages the buffers with different type
//!

#include "encode_tracked_buffer.h"
#include <iterator>
#include <utility>
#include "encode_tracked_buffer_slot.h"
#include "mos_utilities.h"

namespace encode {
constexpr MapBufferResourceType TrackedBuffer::m_mapBufferResourceType[];
TrackedBuffer::TrackedBuffer(EncodeAllocator *allocator, uint8_t maxRefCnt, uint8_t maxNonRefCnt)
    : m_maxRefSlotCnt(maxRefCnt),
      m_maxNonRefSlotCnt(maxNonRefCnt),
      m_allocator(allocator)
{
    m_maxSlotCnt = m_maxRefSlotCnt + m_maxNonRefSlotCnt;
    for (uint8_t i = 0; i < m_maxSlotCnt; i++)
    {
        m_bufferSlots.push_back(MOS_New(BufferSlot, this));
    }

    m_mutex = MosUtilities::MosCreateMutex();
}

TrackedBuffer::~TrackedBuffer()
{
    for (auto it = m_bufferSlots.begin(); it != m_bufferSlots.end(); it++)
    {
        (*it)->Reset();
        MOS_Delete(*it);
    }
    m_bufferQueue.clear();
    m_oldQueue.clear();

    MosUtilities::MosDestroyMutex(m_mutex);
}

MOS_STATUS TrackedBuffer::RegisterParam(BufferType type, MOS_ALLOC_GFXRES_PARAMS param)
{
    auto iter = m_allocParams.find(type);
    if (iter == m_allocParams.end())
    {
        m_allocParams.insert(std::make_pair(type, param));
    }
    else
    {
        // erase the older param and insert new one when resultion change happens
        m_allocParams.erase(iter);
        m_allocParams.insert(std::make_pair(type, param));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS TrackedBuffer::ReleaseUnusedSlots(
    CODEC_REF_LIST* refList,
    bool lazyRelease)
{
    ENCODE_CHK_NULL_RETURN(refList);

    if (!refList->bUsedAsRef)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (lazyRelease && m_bufferSlots.size() < m_maxSlotCnt)
    {
        return MOS_STATUS_SUCCESS;
    }

    // find all the slots which are not used by current reflist
    uint8_t refIndex = 0;

    for (uint8_t slotIndex = 0; slotIndex < m_maxSlotCnt; slotIndex++)
    {
        for (refIndex = 0; refIndex < refList->ucNumRef; refIndex++)
        {
            CODEC_PICTURE currRef = refList->RefList[refIndex];

            if (currRef.FrameIdx == m_bufferSlots[slotIndex]->GetFrameIdx())
            {
                break;
            }
        }

        if (refIndex == refList->ucNumRef)
        {
            ENCODE_CHK_STATUS_RETURN(m_bufferSlots[slotIndex]->Reset());

            if (lazyRelease)
            {
                break;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
    
}

MOS_STATUS TrackedBuffer::Acquire(
    CODEC_REF_LIST* refList,
    bool isIdrFrame,
    bool lazyRelease)
{
    ENCODE_CHK_NULL_RETURN(refList);
    AutoLock lock(m_mutex);

    //if encouter Idr frame, need to clear the reference slots
    if (isIdrFrame)
    {
        for (auto it = m_bufferSlots.begin(); it != m_bufferSlots.end(); it++)
        {
            (*it)->Reset();
        }
    }

    ENCODE_CHK_STATUS_RETURN(ReleaseUnusedSlots(refList, lazyRelease));

    // get current free slot index
    refList->ucScalingIdx = m_currSlotIndex = 0XFF;
    for (uint8_t i = 0; i < m_maxSlotCnt; i++)
    {
        BufferSlot *slot = m_bufferSlots[i];
        if (slot->IsFree())
        {
            m_currSlotIndex = i;
            slot->SetBusy();
            slot->SetFrameIdx(refList->RefPic.FrameIdx);
            break;
        }
    }
    
    //if not found, wait for previous slot returned
    if (m_currSlotIndex == 0XFF)
    {
        MOS_STATUS status = m_condition.Wait(m_mutex);
        if (status != MOS_STATUS_SUCCESS || m_currSlotIndex == 0XFF)
        {
            return MOS_STATUS_UNKNOWN;
        }

        ENCODE_ASSERT(m_currSlotIndex != 0XFF);

        BufferSlot *slot = m_bufferSlots[m_currSlotIndex];
        ENCODE_CHK_NULL_RETURN(slot);
        slot->SetBusy();
        slot->SetFrameIdx(refList->RefPic.FrameIdx);
    }

    refList->ucScalingIdx = m_currSlotIndex;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS TrackedBuffer::Release(CODEC_REF_LIST* refList)
{
    ENCODE_CHK_NULL_RETURN(refList);
    AutoLock lock(m_mutex);

    uint8_t slotIndex = refList->ucScalingIdx;
    if (slotIndex >= m_maxSlotCnt)
    {
        return MOS_STATUS_SUCCESS;
    }

    //if the condition variable is waiting, assign the returned slot to m_currSlotIndex
    //and signal the waiting condition
    if (m_currSlotIndex == 0XFF && !refList->bUsedAsRef)
    {
        ENCODE_CHK_STATUS_RETURN(m_bufferSlots[slotIndex]->Reset());

        m_currSlotIndex = slotIndex;
        m_condition.Signal();
    }

    if (!m_oldQueue.empty())
    {
        for (auto iter = m_oldQueue.begin(); iter != m_oldQueue.end();)
        {
            if (iter->second->SafeToDestory())
            {
                iter = m_oldQueue.erase(iter);
            }
            else
            {
                iter++;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS TrackedBuffer::OnSizeChange()
{
    for (auto iter = m_bufferQueue.begin(); iter != m_bufferQueue.end();)
    {
        if (iter->second->SafeToDestory())
        {
            iter = m_bufferQueue.erase(iter);
        }
        else
        {
            iter++;
        }
    }

    if (!m_bufferQueue.empty())
    {
        m_oldQueue.insert(std::make_move_iterator(m_bufferQueue.begin()),
            std::make_move_iterator(m_bufferQueue.end()));
        m_bufferQueue.clear();
    }

    return MOS_STATUS_SUCCESS;
}

MOS_SURFACE *TrackedBuffer::GetSurface(BufferType type, uint32_t index)
{
    ResourceType resType = GetResourceType(type);

    if (index > m_maxSlotCnt || resType != ResourceType::surfaceResource)
    {
        return nullptr;
    }

    return (MOS_SURFACE *)m_bufferSlots[index]->GetResource(type);
}

MOS_RESOURCE *TrackedBuffer::GetBuffer(BufferType type, uint32_t index)
{
    ResourceType resType = GetResourceType(type);
    if (index > m_maxSlotCnt || resType != ResourceType::bufferResource)
    {
        return nullptr;
    }

    return (MOS_RESOURCE *)m_bufferSlots[index]->GetResource(type);
}

std::shared_ptr<BufferQueue> TrackedBuffer::GetBufferQueue(BufferType type)
{
    auto iter = m_bufferQueue.find(type);
    if (iter == m_bufferQueue.end())
    {
        auto param = m_allocParams.find(type);
        if (param == m_allocParams.end())
        {
            return nullptr;
        }

        ResourceType resType = GetResourceType(type);

        auto alloc = std::make_shared<BufferQueue>(m_allocator, param->second, m_maxSlotCnt);
        alloc->SetResourceType(resType);
        m_bufferQueue.insert(std::make_pair(type, alloc));
        return alloc;
    }
    else
    {
        return iter->second;
    }
}

}