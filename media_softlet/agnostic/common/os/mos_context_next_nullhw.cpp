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

    // A SKU that reports FtrWithSlimVdbox has slim real VDBoxes that cannot serve pipelines
    // needing a full-capability engine. Every other SKU keeps a uniform pool.
    bool isSlimVdboxSku = MEDIA_IS_SKU(GetSkuTable(), FtrWithSlimVdbox);

    m_dummyVdboxCount = fakeCount;
    for (uint32_t i = 0; i < fakeCount; i++)
    {
        m_dummyVdboxArray[i].m_node       = (i >= realCount) ? MOS_GPU_NODE_VE :
                                            (i % 2 == 0)     ? MOS_GPU_NODE_VIDEO :
                                                                MOS_GPU_NODE_VIDEO2;
        m_dummyVdboxArray[i].m_sfcEnabled = true;
        m_dummyVdboxArray[i].m_isSlimVd   = isSlimVdboxSku && (i < realCount);
    }
    MOS_ZeroMemory(m_slotRefCount, sizeof(m_slotRefCount));
    m_startSlotCounterDecode = DUMMY_VDBOX_NUM_MAX - 1;
    m_startSlotCounterEncode = 0;
    m_hasSlimVdboxTopology   = isSlimVdboxSku;
    m_dummyVdboxInitialized  = true;

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief  Does a slot belong to the tier the caller asked for?
//! \details On a slim topology the pool splits in two: the slim real VDBoxes serve
//!          pipelines that do not need a full-capability engine, and everything else
//!          (the VE slots) serves the pipelines that do.
//!
static inline bool IsSlotEligible(const DummyVdboxInfo &slot, bool needFullVdbox)
{
    return needFullVdbox ? !slot.m_isSlimVd : slot.m_isSlimVd;
}

//!
//! \brief  Can a slot take part in a scalable group claim for this request?
//! \details The scalable path counts VD engines, picks the least-loaded one, then bumps the
//!          whole group, and all three passes must agree on the candidate set. Keeping the
//!          predicate in one place is what makes them agree.
//!
static inline bool IsScalableVdCandidate(const DummyVdboxInfo &slot, bool needFullVdbox)
{
    return (slot.m_node == MOS_GPU_NODE_VIDEO || slot.m_node == MOS_GPU_NODE_VIDEO2) &&
           IsSlotEligible(slot, needFullVdbox);
}

MOS_STATUS OsContextNext::SelectAndClaimDummyVdSlot(
    bool          isEncode,
    bool          needFullVdbox,
    bool         &isScalable,
    MOS_GPU_NODE &gpuNode,
    int32_t      &claimedSlotIndex)
{
    std::lock_guard<std::mutex> lock(GetDummyVdboxMutex());

    if (!m_dummyVdboxInitialized || m_dummyVdboxCount == 0)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return m_hasSlimVdboxTopology
               ? SelectAndClaimDummyVdSlotTiered(isEncode, needFullVdbox, isScalable, gpuNode, claimedSlotIndex)
               : SelectAndClaimDummyVdSlotUniform(isEncode, isScalable, gpuNode, claimedSlotIndex);
}

MOS_STATUS OsContextNext::SelectAndClaimDummyVdSlotUniform(
    bool          isEncode,
    bool         &isScalable,
    MOS_GPU_NODE &gpuNode,
    int32_t      &claimedSlotIndex)
{
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

MOS_STATUS OsContextNext::SelectAndClaimDummyVdSlotTiered(
    bool          isEncode,
    bool          needFullVdbox,
    bool         &isScalable,
    MOS_GPU_NODE &gpuNode,
    int32_t      &claimedSlotIndex)
{
    // The origin counter is keyed on isEncode only, so both needFullVdbox directions share it.
    // That is deliberate and safe only because the origin is a pure tiebreak -- the strict '<'
    // least-loaded scan below dominates it. A needFullVdbox == true request usually sees a
    // single-slot eligible tier, so it counts as "level" and advances the origin on nearly
    // every call; that perturbs the other direction's tiebreak order but never its choice.
    // Making the origin authoritative would require splitting these into four counters keyed
    // on (isEncode, needFullVdbox).
    uint32_t &startSlotCounter = isEncode ? m_startSlotCounterEncode : m_startSlotCounterDecode;
    uint32_t  effectiveStart   = startSlotCounter % m_dummyVdboxCount;

    // Fairness is judged over the eligible tier alone. Including the other tier would let an
    // idle slot this pipeline can never claim pin the origin while its own tier is level.
    bool     allEqual  = true;
    bool     firstSeen = false;
    uint32_t firstRef  = 0;
    for (uint32_t i = 0; i < m_dummyVdboxCount; i++)
    {
        if (!IsSlotEligible(m_dummyVdboxArray[i], needFullVdbox))
        {
            continue;
        }
        if (!firstSeen)
        {
            firstRef  = m_slotRefCount[i];
            firstSeen = true;
        }
        else if (m_slotRefCount[i] != firstRef)
        {
            allEqual = false;
            break;
        }
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
            if (IsScalableVdCandidate(m_dummyVdboxArray[i], needFullVdbox))
                vdCount++;
        }

        if (vdCount >= 2)
        {
            int32_t  bestSlot = -1;
            uint32_t minRef   = UINT32_MAX;
            for (uint32_t offset = 0; offset < m_dummyVdboxCount; offset++)
            {
                uint32_t idx = (effectiveStart + offset) % m_dummyVdboxCount;
                if (IsScalableVdCandidate(m_dummyVdboxArray[idx], needFullVdbox) &&
                    m_slotRefCount[idx] < minRef)
                {
                    minRef   = m_slotRefCount[idx];
                    bestSlot = static_cast<int32_t>(idx);
                }
            }

            if (bestSlot < 0)
                return MOS_STATUS_INVALID_PARAMETER;

            // This bumps only the ELIGIBLE VD slots, while ReleaseDummyVdSlot decrements every
            // VD slot without consulting the tier. The two agree because InitDummyVdboxSlots
            // derives node type and slim-ness from the same i < realCount boundary, so on a
            // slim SKU the VD-node slots are exactly the slim slots, and a group claim only
            // ever runs with needFullVdbox == false (section 3.6). Decoupling those two lines
            // would desynchronise claim and release silently.
            for (uint32_t i = 0; i < m_dummyVdboxCount; i++)
            {
                if (IsScalableVdCandidate(m_dummyVdboxArray[i], needFullVdbox))
                    m_slotRefCount[i]++;
            }

            claimedSlotIndex = bestSlot;
            gpuNode          = m_dummyVdboxArray[bestSlot].m_node;
            return MOS_STATUS_SUCCESS;
        }

        // vdCount < 2: the eligible tier has too few VD engines for a scalable claim
        isScalable = false;
    }

    // Standard path: scan both tiers in one pass, keeping the least-loaded slot of each.
    int32_t  eligibleBest   = -1;
    int32_t  fallbackBest   = -1;
    uint32_t eligibleMinRef = UINT32_MAX;
    uint32_t fallbackMinRef = UINT32_MAX;
    for (uint32_t offset = 0; offset < m_dummyVdboxCount; offset++)
    {
        uint32_t idx = (effectiveStart + offset) % m_dummyVdboxCount;
        if (IsSlotEligible(m_dummyVdboxArray[idx], needFullVdbox))
        {
            if (m_slotRefCount[idx] < eligibleMinRef)
            {
                eligibleMinRef = m_slotRefCount[idx];
                eligibleBest   = static_cast<int32_t>(idx);
            }
        }
        else if (m_slotRefCount[idx] < fallbackMinRef)
        {
            fallbackMinRef = m_slotRefCount[idx];
            fallbackBest   = static_cast<int32_t>(idx);
        }
    }

    int32_t bestSlot = -1;
    if (!needFullVdbox && eligibleBest >= 0 && fallbackBest >= 0 && eligibleMinRef > fallbackMinRef)
    {
        // A slim-tier pipeline spills onto the full tier only when that tier is strictly
        // less loaded. Strict '>' keeps a tie on the slim tier, so there is no oscillation.
        bestSlot = fallbackBest;
    }
    else if (needFullVdbox && eligibleBest < 0)
    {
        // No spill in this direction: a pipeline that needs a full-capability engine cannot
        // run on a slim VDBox, so a topology without one is a configuration error.
        MOS_CHK_COND_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, true,
            "needFullVdbox requested but no eligible (non-slim) VDBox/VE slot "
            "is configured in the dummy pool; check MOCKADAPTOR_PIPE topology");
    }
    else
    {
        bestSlot = (eligibleBest >= 0) ? eligibleBest : fallbackBest;
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

    // If this release drains the whole pool back to idle, restore both
    // codecs' search origins immediately. Without this, a transient claim
    // (e.g. a capability-query pipeline that claims then immediately
    // releases a slot) leaves startSlotCounter permanently advanced --
    // ReleaseDummyVdSlot used to rewind only the per-slot refcounts, never
    // the counter -- drifting later real claims off the intended engine.
    bool allReleased = true;
    for (uint32_t i = 0; i < m_dummyVdboxCount; i++)
    {
        if (m_slotRefCount[i] != 0)
        {
            allReleased = false;
            break;
        }
    }
    if (allReleased)
    {
        m_startSlotCounterEncode = 0;
        m_startSlotCounterDecode = DUMMY_VDBOX_NUM_MAX - 1;
    }
}

#endif // (_DEBUG || _RELEASE_INTERNAL)
