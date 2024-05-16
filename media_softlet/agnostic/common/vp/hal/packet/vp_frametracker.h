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
//! \file     vp_frametracker.h
//! \brief    vp frame tracker.
//!
#ifndef __VP_FRAMETRACKER_H__
#define __VP_FRAMETRACKER_H__
#include <chrono>
#include <stack>
#include "mos_defs.h"
#include "vp_utils.h"

namespace vp
{
using Clock = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;

#define FRAME_TRACING_BEGIN 2 //Skip first 2 frames. Tuning the value can change skipped frame
#define FRAME_TRACING_PERIOD 5 //Calbrate FPS with 5 frames, can enlarge/decrease calculation window
#define MAX_FRAME_TRACING (FRAME_TRACING_BEGIN + FRAME_TRACING_PERIOD + 1)
#define FPS60_THRESHOLD 33 // 60FPS threshold. Can tune this value according to different senario

class VpFrameTracker
{
public:
    VpFrameTracker();

    virtual ~VpFrameTracker()
    {
        m_startTimeQueue.clear();
    };

    virtual MOS_STATUS UpdateFPS();

    virtual bool Is60Fps();

    double GetFPS()
    {
        std::chrono::duration<double> elapsed = m_startTimeQueue.back() - m_startTimeQueue.front();

        if (m_startTimeQueue.size() != (FRAME_TRACING_PERIOD + 1))
        {
            VP_RENDER_ASSERTMESSAGE("Frame tracker queue size is not correct!");
            VP_RENDER_ASSERT(m_startTimeQueue.size() == (FRAME_TRACING_PERIOD + 1));
        }

        return ((1.0 / elapsed.count()) * FRAME_TRACING_PERIOD);
    }

protected:

    int                   m_numberOfFrames;
    std::deque<TimePoint> m_startTimeQueue;
    bool                  m_isEnabled = false;

MEDIA_CLASS_DEFINE_END(vp__VpFrameTracker)
};

}  // namespace vp
#endif  // !__VP_FRAMETRACKER_H__
