/*
* Copyright (c) 2018-2019, Intel Corporation
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
//! \file      cm_tracker.h
//! \brief     Contains Class CmTracker  definitions
//!
#pragma once

#include "cm_hal.h"
#include "cm_csync.h"
#include "frame_tracker.h"

class CmEventEx;

class CmTracker
{
public:
    CmTracker(MOS_INTERFACE *osInterface);
    ~CmTracker();
    MOS_STATUS Initialize(uint32_t taskNum = 64);

    MOS_STATUS AssignFrameTracker(uint32_t trackerIndex, uint32_t *taskId, uint32_t *tracker, bool hasEvent = true);

    void InvalidFrameTracker(uint32_t taskId);

    CM_HAL_TASK_STATUS Query(uint32_t taskId);

    MOS_STATUS Refresh();

    CM_RETURN_CODE WaitForAllTasksFinished();

    inline void AssociateEvent(CmEventEx *event) { CMRT_UMD::CLock Locker(m_eventListSection); m_associatedEvents.push_back(event); }
    inline void DeAssociateEvent(CmEventEx *event) { CMRT_UMD::CLock Locker(m_eventListSection); m_associatedEvents.remove(event); }

    inline volatile uint32_t* GetLatestTrackerAddr(uint32_t trackerIndex) {return m_trackerProducer.GetLatestTrackerAddress(trackerIndex); }
    inline void GetLatestTrackerResource(uint32_t trackerIndex, MOS_RESOURCE **resource, uint32_t *offset)
    {
        m_trackerProducer.GetLatestTrackerResource(trackerIndex, resource, offset);
    }
    inline FrameTrackerProducer *GetTrackerProducer() {return &m_trackerProducer;}

    inline MOS_RESOURCE* GetResource() {return &m_resource; }
    inline uint32_t GetFrameTrackerOffset(uint32_t taskId) {return taskId * sizeof(_CmFrameTracker); }
    inline uint32_t GetStartOffset(uint32_t taskId) {return GetFrameTrackerOffset(taskId) + offsetof(_CmFrameTracker, start); }
    inline uint32_t GetEndOffset(uint32_t taskId) {return GetFrameTrackerOffset(taskId) + offsetof(_CmFrameTracker, end); }
    inline uint64_t GetStart(uint32_t taskId) {return *((uint64_t *)(m_data + GetStartOffset(taskId))); }
    inline uint64_t GetEnd(uint32_t taskId) {return *((uint64_t *)(m_data + GetEndOffset(taskId))); }

protected:
    struct _CmFrameTracker
    {
        uint32_t valid;
        uint32_t tracker;
        uint64_t start;
        uint64_t end;
        uint32_t trackerIndex;
    };
    MOS_INTERFACE *m_osInterface;

    MOS_RESOURCE m_resource;
    uint8_t *m_data;
    uint32_t m_maxTaskNum;
    
    uint32_t m_nextTaskID;

    FrameTrackerProducer m_trackerProducer;

    std::list<CmEventEx *> m_associatedEvents;
    CMRT_UMD::CSync m_eventListSection;
    
};
