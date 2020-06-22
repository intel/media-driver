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
//! \file      cm_event_ex_base.cpp
//! \brief     Contains Class CmEventExBase  definitions
//!

#include "cm_event_ex.h"
#include "cm_hal.h"
#include "cm_log.h"
#include "cm_tracker.h"
#include "cm_notifier.h"

CmEventExBase::~CmEventExBase()
{
    if (m_cmTracker)
    {
        m_cmTracker->InvalidFrameTracker(m_taskId);
    }
}

CM_STATUS CmEventExBase::Query()
{
    if (m_state == CM_STATUS_FINISHED)
    {
        return CM_STATUS_FINISHED;
    }
    switch(m_cmTracker->Query(m_taskId))
    {
        case CM_TASK_FINISHED:
            m_state = CM_STATUS_FINISHED;
            break;
        case CM_TASK_IN_PROGRESS:
            m_state = CM_STATUS_FLUSHED;
            break;
        case CM_TASK_QUEUED:
            m_state = CM_STATUS_QUEUED;
            break;
        default:
            m_state = CM_STATUS_RESET;
            break;
    }
    if (m_state == CM_STATUS_FINISHED)
    {
        m_start = m_cmTracker->GetStart(m_taskId);
        m_end = m_cmTracker->GetEnd(m_taskId);

        m_cmTracker->InvalidFrameTracker(m_taskId);

        RleaseOsData();

        if (m_notifier)
        {
            m_notifier->NotifyTaskCompleted(m_taskId);
        }
        LogTimestamps(__FUNCTION__, __LINE__);
    }

    return m_state;
}

int32_t CmEventExBase::GetExecutionTime(uint64_t &time)
{
    if (m_state != CM_STATUS_FINISHED)
    {
        Query();
    }
    if (m_state == CM_STATUS_FINISHED)
    {
        uint64_t ticks = m_end - m_start;
        time = HalCm_ConvertTicksToNanoSeconds(m_cmhal, ticks);
        return CM_SUCCESS;
    }
    return CM_FAILURE;
}

int32_t CmEventExBase::GetExecutionTickTime(uint64_t &ticks)
{
    if (m_state != CM_STATUS_FINISHED)
    {
        Query();
    }
    if (m_state == CM_STATUS_FINISHED)
    {
        ticks = m_end - m_start;
        return CM_SUCCESS;
    }
    return CM_FAILURE;
}

bool CmEventExBase::LogTimestamps(const char *callerFunctionName, int callerLineNumber)
{
#if CM_LOG_ON
    static const char *status_strings[] = {"CM_STATUS_QUEUED",
                                           "CM_STATUS_FLUSHED",
                                           "CM_STATUS_FINISHED",
                                           "CM_STATUS_STARTED",
                                           "CM_STATUS_RESET"};
    uint64_t ticks = m_end - m_start;
    uint64_t duration = HalCm_ConvertTicksToNanoSeconds(m_cmhal, ticks);

    std::ostringstream log_stream;
    log_stream << callerFunctionName << "():\n"
               << "<CmEvent>:" << static_cast<void*>(this) << ".\n"
               << " Status: " << status_strings[m_state] << ".\n"
               << " Duration: " << duration << "ns.\n"
               << " Duration in ticks: " << ticks << ".\n"
               << " Start time in ticks: " << m_start << "\n"
               << " End time in ticks: " << m_end << "\n" << std::endl;
    if (!log_stream.good())
    {
        return false;
    }
    CmLogger::GetInstance().Print(CM_LOG_LEVEL_DEBUG, __FILE__, callerLineNumber,
                                  log_stream.str());
#endif  // #if CM_LOG_ON
    return true;
}
