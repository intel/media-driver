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
//! \file     frame_tracker.h
//! \brief    Manages the multiple trackers in one place.
//!

#ifndef __FRAME_TRACKER_H__
#define __FRAME_TRACKER_H__

#include "mos_os.h"
#include <map>

#define MAX_TRACKER_NUMBER 64
#define CHK_INDEX(index) if ((index) >= MAX_TRACKER_NUMBER) {return MOS_STATUS_NOT_ENOUGH_BUFFER; }

class FrameTrackerToken;
class FrameTrackerProducer;

// C-style struct for FrameTrackerToken
// for tokens that embeded in a struct other than a class
// in legacy DSH code, the token is always embeded in another struct
struct FrameTrackerTokenFlat
{
    FrameTrackerProducer *producer;
    uint32_t trackers[MAX_TRACKER_NUMBER];
    bool valid;
    bool stick;
};

bool FrameTrackerTokenFlat_IsExpired(const FrameTrackerTokenFlat *self);

static inline void FrameTrackerTokenFlat_Merge(FrameTrackerTokenFlat *self, uint32_t index, uint32_t tracker)
{
    if (index < MAX_TRACKER_NUMBER && tracker != 0)
    {
        self->trackers[index] = tracker;
    }
}

static inline void FrameTrackerTokenFlat_Merge(FrameTrackerTokenFlat *self, const FrameTrackerTokenFlat *token)
{
    self->producer = token->producer;
    for (int i = 0; i < MAX_TRACKER_NUMBER; i++)
    {
        if (token->trackers[i] != 0)
        {
            FrameTrackerTokenFlat_Merge(self, i, token->trackers[i]);
        }
    }
}

static inline void FrameTrackerTokenFlat_SetProducer(FrameTrackerTokenFlat *self, FrameTrackerProducer *producer)
{
    self->producer = producer;
}

static inline void FrameTrackerTokenFlat_Clear(FrameTrackerTokenFlat *self)
{
    self->stick = false;
    MOS_ZeroMemory(self->trackers, sizeof(self->trackers));
}

static inline void FrameTrackerTokenFlat_Validate(FrameTrackerTokenFlat *self)
{
    self->valid = true;
}

static inline void FrameTrackerTokenFlat_Invalidate(FrameTrackerTokenFlat *self)
{
    self->valid = false;
}

static inline bool FrameTrackerTokenFlat_IsValid(const FrameTrackerTokenFlat *self)
{
    return self->valid;
}

static inline void FrameTrackerTokenFlat_Stick(FrameTrackerTokenFlat *self)
{
    self->stick = true;
}

class FrameTrackerProducer
{
public:
    FrameTrackerProducer();
    ~FrameTrackerProducer();

    MOS_STATUS Initialize(MOS_INTERFACE *osInterface);

    int AssignNewTracker();

    inline void GetLatestTrackerResource(uint32_t index, MOS_RESOURCE **resource, uint32_t *offset)
    {
        *resource = &m_resource;
        *offset = index * m_trackerSize;
    }
    
    inline volatile uint32_t *GetLatestTrackerAddress(uint32_t index)
    {
        return (uint32_t *)((uint8_t *)m_resourceData + index * m_trackerSize);
    }

    inline uint32_t GetNextTracker(uint32_t index) { return m_counters[index];}

    inline MOS_STATUS StepForward(uint32_t index)
    {
        CHK_INDEX(index);
        ++ m_counters[index];
        if (m_counters[index] == 0)
        {
            m_counters[index] = 1;
        }
        return MOS_STATUS_SUCCESS;
    }

protected:
    // indexes to assign a new tracer
    uint32_t m_nextTrackerIndex;
    bool m_trackerInUse[MAX_TRACKER_NUMBER];

    // resource
    static const uint32_t m_trackerSize = 8; // reserved two dwords for each tracker
    MOS_RESOURCE m_resource;
    uint32_t *m_resourceData;

    // counters
    uint32_t m_counters[MAX_TRACKER_NUMBER];

    // interfaces
    MOS_INTERFACE *m_osInterface;
};

class FrameTrackerToken
{
public:
    FrameTrackerToken():
        m_producer(nullptr)
    {
    }

    ~FrameTrackerToken()
    {
    }

    bool IsExpired()
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

    void Merge(const FrameTrackerToken *token);

    inline void Merge(uint32_t index, uint32_t tracker) {m_holdTrackers[index] = tracker; }

    inline void SetProducer(FrameTrackerProducer *producer)
    {
        m_producer = producer;
    }

    inline void Clear() {m_holdTrackers.clear(); }

protected:
    FrameTrackerProducer *m_producer;
    std::map<uint32_t, uint32_t> m_holdTrackers;
};

#endif // __FRAME_TRACKER_H__
