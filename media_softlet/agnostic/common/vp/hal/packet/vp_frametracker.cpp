/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     vp_frametracker.cpp
//! \brief    vp frame tracker.
//! \details  vp frame tracker.
//!
#include "vp_frametracker.h"
using namespace vp;

VpFrameTracker::VpFrameTracker() : m_numberOfFrames(0), m_startTimeQueue({}), m_isEnabled(false)
{
}

MOS_STATUS VpFrameTracker::UpdateFPS()
{
    if (m_isEnabled)
    {
        m_numberOfFrames++;
        if (m_numberOfFrames > FRAME_TRACING_BEGIN)
        {
            if (m_startTimeQueue.size() <= FRAME_TRACING_PERIOD)
            {
                m_startTimeQueue.push_back(Clock::now());
            }
            else
            {
                m_startTimeQueue.pop_front();
                m_startTimeQueue.push_back(Clock::now());
                m_numberOfFrames = MAX_FRAME_TRACING;
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

bool VpFrameTracker::Is60Fps()
{
    // Only be calculated with isEnabled flag has been set
    // Skip first FRAME_TRACING_BEGIN frames to avoid noise
    // Calculate the FPS during FRAME_TRACING_PERIOD
    // If the FPS is greater than FPS60_THRESHOLD, return true
    m_isEnabled = true;
    bool res         = false;

    if (m_numberOfFrames >= MAX_FRAME_TRACING)
    {
        double currFPS = GetFPS();
        if (currFPS > FPS60_THRESHOLD)
        {
            res = true;
        }
        else
        {
            res = false;
        }
    }
    return res;
}