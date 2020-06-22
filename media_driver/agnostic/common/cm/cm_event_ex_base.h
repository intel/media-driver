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
//! \file      cm_event_ex_base.h
//! \brief     Contains Class CmEventExBase  definitions
//!
#pragma once

#include "cm_event.h"

class CmKernelEx;
typedef struct _CM_HAL_STATE CM_HAL_STATE;
class CmTracker;

namespace CMRT_UMD
{
class CmNotifierGroup;
};
class CmEventExBase : public CMRT_UMD::CmEvent
{
public:
    virtual int32_t GetExecutionTime(uint64_t &time);

    virtual int32_t GetSurfaceDetails(uint32_t kernIndex, uint32_t surfBTI, CM_SURFACE_DETAILS& outDetails)
    {
        return CM_NOT_IMPLEMENTED;
    }

    virtual int32_t GetProfilingInfo(CM_EVENT_PROFILING_INFO infoType, size_t paramSize, void *inputValue, void *value)
    {
        return CM_NOT_IMPLEMENTED;
    }

    virtual int32_t GetExecutionTickTime(uint64_t &ticks);

    void SetNotifier(CMRT_UMD::CmNotifierGroup *notifier) {m_notifier = notifier; }

protected:
    CmEventExBase(CM_HAL_STATE *state, uint32_t id, CmTracker *tracker):
        m_taskId(id),
        m_cmTracker(tracker),
        m_cmhal(state),
        m_start(0),
        m_end(0),
        m_state(CM_STATUS_RESET),
        m_osData(nullptr),
        m_notifier(nullptr)
    {
    }

    virtual ~CmEventExBase();

    CM_STATUS Query();

    virtual void RleaseOsData() {}
    
    uint32_t m_taskId;
    CmTracker *m_cmTracker;

    CM_HAL_STATE *m_cmhal;

    uint64_t m_start;
    uint64_t m_end;
    CM_STATUS m_state;

    void *m_osData;

    CMRT_UMD::CmNotifierGroup *m_notifier;

private:
    bool LogTimestamps(const char *callerFunctionName, int callerLineNumber);
};
