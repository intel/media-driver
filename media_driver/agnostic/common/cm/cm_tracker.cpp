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
//! \file      cm_tracker.cpp
//! \brief     Contains Class CmTracker  definitions
//!

#include "cm_tracker.h"
#include "cm_debug.h"
#include "cm_common.h"
#include "cm_event_ex.h"

CmTracker::CmTracker(MOS_INTERFACE *osInterface):
    m_osInterface(osInterface),
    m_data(nullptr),
    m_maxTaskNum(0),
    m_nextTaskID(0)
{
    MOS_ZeroMemory(&m_resource, sizeof(m_resource));
}

MOS_STATUS CmTracker::Initialize(uint32_t taskNum)
{
    m_maxTaskNum = taskNum;
    uint32_t size = GetFrameTrackerOffset(0) + (m_maxTaskNum+1) * sizeof(_CmFrameTracker);
    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(allocParams));
    allocParams.Type = MOS_GFXRES_BUFFER;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.Format = Format_Buffer;
    allocParams.dwBytes = size;
    allocParams.pBufName = "CmTracker";

    CM_CHK_MOSSTATUS_RETURN(m_osInterface->pfnAllocateResource(m_osInterface, &allocParams,
                                                               &m_resource));
    CM_CHK_MOSSTATUS_RETURN(m_osInterface->pfnRegisterResource(m_osInterface, &m_resource,
                                                               true, true));
    CM_CHK_MOSSTATUS_RETURN(m_osInterface->pfnSkipResourceSync(&m_resource));

    MOS_LOCK_PARAMS lockParams;
    MOS_ZeroMemory(&lockParams, sizeof(lockParams));
    lockParams.ReadOnly = 1;
    lockParams.ForceCached = true;
    m_data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &m_resource, &lockParams);

    MOS_ZeroMemory(m_data, size);

    // init the tracker producer
    m_trackerProducer.Initialize(m_osInterface);

    return MOS_STATUS_SUCCESS;
}

CmTracker::~CmTracker()
{
    m_osInterface->pfnFreeResourceWithFlag(m_osInterface, &m_resource, 1);
}

MOS_STATUS CmTracker::AssignFrameTracker(uint32_t trackerIndex, uint32_t *taskId, uint32_t *tracker, bool hasEvent)
{
    int id = -1;
    for (uint32_t i = 0; i < m_maxTaskNum; i++)
    {
        id = (int)m_nextTaskID + i;
        id = id%m_maxTaskNum;
        _CmFrameTracker *frameTracker = (_CmFrameTracker *)(m_data + GetFrameTrackerOffset(id));
        if (frameTracker->valid == 0)
        {
            frameTracker->valid = hasEvent?1:0;
            frameTracker->tracker = m_trackerProducer.GetNextTracker(trackerIndex);
            frameTracker->trackerIndex = trackerIndex;
            frameTracker->start = CM_INVALID_INDEX;
            frameTracker->end = CM_INVALID_INDEX;
            *taskId = id;
            *tracker = frameTracker->tracker;
            m_trackerProducer.StepForward(trackerIndex);
            m_nextTaskID = *taskId + 1;
            return MOS_STATUS_SUCCESS;
        }
    }

    // can't find a slot for a new task
    // just assign the tracker and dummy task
    *taskId = m_maxTaskNum;
    *tracker = m_trackerProducer.GetNextTracker(trackerIndex);
    m_trackerProducer.StepForward(trackerIndex);
    return MOS_STATUS_UNKNOWN;
}

CM_HAL_TASK_STATUS CmTracker::Query(uint32_t taskId)
{
    _CmFrameTracker *frameTracker = (_CmFrameTracker *)(m_data + GetFrameTrackerOffset(taskId));
    if ((int)(frameTracker->tracker - *GetLatestTrackerAddr(frameTracker->trackerIndex)) <= 0 
        && frameTracker->end != CM_INVALID_INDEX)
    {
        return CM_TASK_FINISHED;
    }
    else if (frameTracker->start != CM_INVALID_INDEX)
    {
        return CM_TASK_IN_PROGRESS;
    }
    else
    {
        return CM_TASK_QUEUED;
    }
}

MOS_STATUS CmTracker::Refresh()
{
    CM_STATUS status;
    if (m_associatedEvents.size() == 0)
    {
        return MOS_STATUS_SUCCESS;
    }
    CMRT_UMD::CLock Locker(m_eventListSection);
    CmEventEx* event = *(m_associatedEvents.begin());
    event->GetStatus(status);
    if (status == CM_STATUS_FINISHED)
    {
        m_associatedEvents.pop_front();
    }
    return MOS_STATUS_SUCCESS;
}


void CmTracker::InvalidFrameTracker(uint32_t taskId)
{
    _CmFrameTracker *frameTracker = (_CmFrameTracker *)(m_data + GetFrameTrackerOffset(taskId));
    frameTracker->valid = 0;
}

CM_RETURN_CODE CmTracker::WaitForAllTasksFinished()
{
    uint32_t targetTaskId[MAX_TRACKER_NUMBER];
    uint32_t targetTracker[MAX_TRACKER_NUMBER];

    MOS_ZeroMemory(targetTaskId, sizeof(targetTaskId));
    MOS_ZeroMemory(targetTracker, sizeof(targetTracker));

    uint32_t inExecution = 0;
    for (uint32_t id = 0; id < m_maxTaskNum; id++)
    {
        _CmFrameTracker *frameTracker = (_CmFrameTracker *)(m_data + GetFrameTrackerOffset(id));
        if (Query(id) != CM_TASK_FINISHED)
        {
            ++ inExecution;
            if ((int)(frameTracker->tracker - targetTracker[frameTracker->trackerIndex]) > 0)
            {
                //update
                targetTracker[frameTracker->trackerIndex] = frameTracker->tracker;
                targetTaskId[frameTracker->trackerIndex] = id;
            }
        }
    }
    
    if (inExecution == 0) // no task pending at all
    {
        return CM_SUCCESS;
    }

    //Used for timeout detection
    uint64_t freq, start, timeout;
    MOS_QueryPerformanceFrequency(&freq);
    MOS_QueryPerformanceCounter(&start);
    timeout = start + (CM_MAX_TIMEOUT * freq * inExecution);

    // wait for the last task to be finished (largest tracker)
    for (int i = 0; i < MAX_TRACKER_NUMBER; i++)
    {
        if (targetTracker[i] == 0)
        {
            continue;
        }
        while (Query(targetTaskId[i]) != CM_TASK_FINISHED)
        {
            uint64_t current;
            MOS_QueryPerformanceCounter((uint64_t*)&current);
            if( current > timeout )
            {
                return CM_EXCEED_MAX_TIMEOUT;
            }
            
        }
    }
    return CM_SUCCESS;    
}

