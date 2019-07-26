/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     frame_tracker.cpp
//! \brief    Manages the multiple trackers in one place.
//!

#include "frame_tracker.h"
#include "mhw_utilities.h"

bool FrameTrackerTokenFlat_IsExpired(const FrameTrackerTokenFlat *self)
{
    if (self->stick)
    {
        return false;
    }
    if (self->producer == nullptr)
    {
        return true;
    }
    for (int i = 0; i < MAX_TRACKER_NUMBER; i ++)
    {
        if (self->trackers[i] != 0)
        {
            volatile uint32_t latestTracker = *(self->producer->GetLatestTrackerAddress(i));
            if ((int)(self->trackers[i] - latestTracker) > 0)
            {
                return false;
            }
        }
    }
    return true;
}

bool FrameTrackerToken::IsExpired()
{
    if (m_producer == nullptr)
    {
        return true;
    }

    for (auto ite = m_holdTrackers.begin(); ite != m_holdTrackers.end(); ite ++)
    {
        uint32_t index = ite->first;
        volatile uint32_t latestTracker = *(m_producer->GetLatestTrackerAddress(index));
        uint32_t holdTracker = ite->second;
        if ((int)(holdTracker - latestTracker) > 0)
        {
            return false;
        }
    }
    return true;
}

void FrameTrackerToken::Merge(const FrameTrackerToken *token)
{
    m_producer = token->m_producer;
    for (auto ite = token->m_holdTrackers.begin(); ite != token->m_holdTrackers.end(); ite ++)
    {
        uint32_t index = ite->first;
        uint32_t holdTracker = ite->second;
        Merge(index, holdTracker);
    }
}

FrameTrackerProducer::FrameTrackerProducer():
    m_nextTrackerIndex(0),
    m_resourceData(nullptr),
    m_osInterface(nullptr)
{
    Mos_ResetResource(&m_resource);
    MOS_ZeroMemory(m_trackerInUse, sizeof(m_trackerInUse));
    MOS_ZeroMemory(m_counters, sizeof(m_counters));
}

FrameTrackerProducer::~FrameTrackerProducer()
{
    if (!Mos_ResourceIsNull(&m_resource))
    {
        m_osInterface->pfnUnlockResource(
                m_osInterface,
                &m_resource);

        m_osInterface->pfnFreeResourceWithFlag(
            m_osInterface,
            &m_resource,
            1);
    }
}

MOS_STATUS FrameTrackerProducer::Initialize(MOS_INTERFACE *osInterface)
{
    m_osInterface = osInterface;
    MHW_CHK_NULL_RETURN(m_osInterface);
    
    // allocate the resource
    MOS_ALLOC_GFXRES_PARAMS allocParamsLinearBuffer;
    uint32_t size = MOS_ALIGN_CEIL(MAX_TRACKER_NUMBER * m_trackerSize, MHW_CACHELINE_SIZE);
    MOS_ZeroMemory(&allocParamsLinearBuffer, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsLinearBuffer.Type     = MOS_GFXRES_BUFFER;
    allocParamsLinearBuffer.TileType = MOS_TILE_LINEAR;
    allocParamsLinearBuffer.Format   = Format_Buffer;
    allocParamsLinearBuffer.dwBytes = size;
    allocParamsLinearBuffer.pBufName = "FrameTrackerResource";

    MHW_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsLinearBuffer,
        &m_resource));

    // Lock the Resource
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;
    lockFlags.ForceCached = true;

    m_resourceData = (uint32_t*)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_resource,
        &lockFlags);
    MOS_ZeroMemory(m_resourceData, size);

    m_osInterface->pfnSkipResourceSync(&m_resource);

    MHW_CHK_NULL_RETURN(m_resourceData);

    return MOS_STATUS_SUCCESS;
}

int FrameTrackerProducer::AssignNewTracker()
{
    uint32_t trackerIndex = m_nextTrackerIndex;
    bool found = false;
    do
    {
        if (!m_trackerInUse[trackerIndex])
        {
            found = true;
            break;
        }
        else
        {
            ++ trackerIndex;
            if (trackerIndex == MAX_TRACKER_NUMBER)
            {
                trackerIndex = 0;
            }
        }
    }
    while (trackerIndex != m_nextTrackerIndex);

    if (found)
    {
        m_trackerInUse[trackerIndex] = true;
        m_counters[trackerIndex] = 1;
        m_nextTrackerIndex = trackerIndex + 1;
        if (m_nextTrackerIndex == MAX_TRACKER_NUMBER)
        {
            m_nextTrackerIndex = 0;
        }

        return trackerIndex;
    }
    else
    {
        return -1;
    }
}
