/*
* Copyright (c) 2017, Intel Corporation
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
#include "cm_timer.h"
#include "cm_perf_statistics.h"

#define MSG_STRING_SIZE 256

#if MDF_PROFILER_ENABLED
extern CmPerfStatistics gCmPerfStatistics;

CmTimer::CmTimer(const char *functionName):
    m_cycles(0),
    m_funcName(const_cast<char*>(functionName))
{
    //Get frequency
    QueryPerformanceFrequency(&m_freq);

    // initialize private variables
    m_start.QuadPart = 0;
    m_end.QuadPart   = 0;

    //Start timer
    Start();

    return;
}

CmTimer::~CmTimer()
{
    Stop();
    gCmPerfStatistics.InsertApiCallRecord(m_funcName, GetTimeinMs(), m_start,
                                          m_end);
}

void CmTimer::Start()
{
    QueryPerformanceCounter(&m_start);  // recode API start time
    InsertEventStartFlag();
    return;
}

void CmTimer::Stop()
{
    QueryPerformanceCounter(&m_end);
    m_cycles = m_end.QuadPart - m_start.QuadPart;
    InsertEventEndFlag();
    return;
}

float CmTimer::GetTimeinMs()
{
    return (float)m_cycles * (float)1000.0 / (float)m_freq.QuadPart;
}

#endif  // #if MDF_PROFILER_ENABLED
