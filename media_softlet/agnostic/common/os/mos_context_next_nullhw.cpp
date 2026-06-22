/*
* Copyright (c) 2026, Intel Corporation
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
//! \file     mos_context_next_nullhw.cpp
//! \brief    OsContextNext NullHW dummy VDBox slot management
//!

#include "mos_context_next.h"
#include "mos_os_mock_adaptor.h"

#if (_DEBUG || _RELEASE_INTERNAL)

MOS_STATUS OsContextNext::InitDummyVdboxSlots()
{
    if (!GetNullHwIsEnabled())
    {
        return MOS_STATUS_SUCCESS;
    }

    std::lock_guard<std::mutex> lock(GetDummyVdboxMutex());

    if (m_dummyVdboxInitialized)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_mockAdaptor == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    uint32_t fakeCount = m_mockAdaptor->GetFakeVdboxCount();
    uint32_t realCount = m_mockAdaptor->GetRealVdboxCount();

    if (fakeCount == 0)
    {
        fakeCount = 1;
    }
    if (fakeCount > DUMMY_VDBOX_NUM_MAX)
    {
        fakeCount = DUMMY_VDBOX_NUM_MAX;
    }
    if (realCount > fakeCount)
    {
        realCount = fakeCount;
    }

    m_dummyVdboxCount = fakeCount;
    for (uint32_t i = 0; i < fakeCount; i++)
    {
        m_dummyVdboxArray[i].m_node       = (i >= realCount) ? MOS_GPU_NODE_VE :
                                            (i % 2 == 0)     ? MOS_GPU_NODE_VIDEO :
                                                                MOS_GPU_NODE_VIDEO2;
        m_dummyVdboxArray[i].m_sfcEnabled = true;
        m_dummyVdboxArray[i].m_isSlimVd   = false;
    }
    MOS_ZeroMemory(m_slotRefCount, sizeof(m_slotRefCount));
    m_startSlotCounterDecode = DUMMY_VDBOX_NUM_MAX - 1;
    m_startSlotCounterEncode = 0;
    m_dummyVdboxInitialized  = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS OsContextNext::SelectAndClaimDummyVdSlot(
    bool          isEncode,
    bool         &isScalable,
    MOS_GPU_NODE &gpuNode,
    int32_t      &claimedSlotIndex)
{
    std::lock_guard<std::mutex> lock(GetDummyVdboxMutex());

    if (!m_dummyVdboxInitialized || m_dummyVdboxCount == 0)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    uint32_t &startSlotCounter = isEncode ? m_startSlotCounterEncode : m_startSlotCounterDecode;
    uint32_t  effectiveStart   = startSlotCounter % m_dummyVdboxCount;

    bool     allEqual = true;
    uint32_t firstRef = m_slotRefCount[0];
    for (uint32_t i = 1; i < m_dummyVdboxCount; i++)
    {
        if (m_slotRefCount[i] != firstRef) { allEqual = false; break; }
    }
    if (allEqual)
    {
        startSlotCounter++;
    }

    if (isScalable)
    {
        uint32_t vdCount = 0;
        for (uint32_t i = 0; i < m_dummyVdboxCount; i++)
        {
            if (m_dummyVdboxArray[i].m_node == MOS_GPU_NODE_VIDEO ||
                m_dummyVdboxArray[i].m_node == MOS_GPU_NODE_VIDEO2)
                vdCount++;
        }

        if (vdCount >= 2)
        {
            int32_t  bestSlot = -1;
            uint32_t minRef   = UINT32_MAX;
            for (uint32_t offset = 0; offset < m_dummyVdboxCount; offset++)
            {
                uint32_t idx = (effectiveStart + offset) % m_dummyVdboxCount;
                if ((m_dummyVdboxArray[idx].m_node == MOS_GPU_NODE_VIDEO ||
                     m_dummyVdboxArray[idx].m_node == MOS_GPU_NODE_VIDEO2) &&
                    m_slotRefCount[idx] < minRef)
                {
                    minRef   = m_slotRefCount[idx];
                    bestSlot = static_cast<int32_t>(idx);
                }
            }

            if (bestSlot < 0)
                return MOS_STATUS_INVALID_PARAMETER;

            for (uint32_t i = 0; i < m_dummyVdboxCount; i++)
            {
                if (m_dummyVdboxArray[i].m_node == MOS_GPU_NODE_VIDEO ||
                    m_dummyVdboxArray[i].m_node == MOS_GPU_NODE_VIDEO2)
                    m_slotRefCount[i]++;
            }

            claimedSlotIndex = bestSlot;
            gpuNode          = m_dummyVdboxArray[bestSlot].m_node;
            return MOS_STATUS_SUCCESS;
        }

        // vdCount < 2: config requested scalable but not enough VD engines — fall back
        isScalable = false;
    }

    // Standard path: all slots eligible, only best slot refCount++
    int32_t  bestSlot = -1;
    uint32_t minRef   = UINT32_MAX;
    for (uint32_t offset = 0; offset < m_dummyVdboxCount; offset++)
    {
        uint32_t idx = (effectiveStart + offset) % m_dummyVdboxCount;
        if (m_slotRefCount[idx] < minRef)
        {
            minRef   = m_slotRefCount[idx];
            bestSlot = static_cast<int32_t>(idx);
        }
    }

    if (bestSlot < 0)
        return MOS_STATUS_INVALID_PARAMETER;

    m_slotRefCount[bestSlot]++;
    claimedSlotIndex = bestSlot;
    gpuNode          = m_dummyVdboxArray[bestSlot].m_node;

    return MOS_STATUS_SUCCESS;
}

void OsContextNext::ReleaseDummyVdSlot(int32_t slotIndex, bool isScalable)
{
    std::lock_guard<std::mutex> lock(GetDummyVdboxMutex());

    if (slotIndex < 0 || slotIndex >= static_cast<int32_t>(m_dummyVdboxCount))
        return;

    if (isScalable)
    {
        for (uint32_t i = 0; i < m_dummyVdboxCount; i++)
        {
            if ((m_dummyVdboxArray[i].m_node == MOS_GPU_NODE_VIDEO ||
                 m_dummyVdboxArray[i].m_node == MOS_GPU_NODE_VIDEO2) &&
                m_slotRefCount[i] > 0)
            {
                m_slotRefCount[i]--;
            }
        }
    }
    else
    {
        if (m_slotRefCount[slotIndex] > 0)
            m_slotRefCount[slotIndex]--;
    }
}

#endif // (_DEBUG || _RELEASE_INTERNAL)
