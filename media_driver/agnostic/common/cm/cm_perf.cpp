/*
* Copyright (c) 2017 - 2021, Intel Corporation
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
//! \file      cm_perf.cpp 
//! \brief     Contains functions which recode time and calculate performance 
//!

#include <iostream>
#include <cm_debug.h>
#include "cm_log.h"

#if CM_LOG_ON

CmTimer::CmTimer(const std::string FunctionName)
{
    uint32_t success = MosUtilities::MosQueryPerformanceFrequency((uint64_t *)&m_freq.QuadPart);
    CM_ASSERT(success);

    m_funcName          = FunctionName;
    m_cycles            = 0;
    m_start.QuadPart    = 0;
    m_end.QuadPart      = 0;
    m_bstopped          = false;

    Start();
}

CmTimer::~CmTimer()
{
    if(!m_bstopped)
    {
        Stop();
    }
}

void CmTimer::Start()
{
    uint32_t success = MosUtilities::MosQueryPerformanceCounter((uint64_t *)&m_start.QuadPart);
    CM_ASSERT(success);

    return;
}

void CmTimer::Stop()
{
    uint32_t success = MosUtilities::MosQueryPerformanceCounter((uint64_t *)&m_end.QuadPart);
    CM_ASSERT(success);

    m_bstopped = true;

    m_cycles = m_end.QuadPart - m_start.QuadPart;

    return;
}

float CmTimer::TotalMilliSecond()
{
    return (float)m_cycles*(float)1000.0 / (float)m_freq.QuadPart;
}

std::string CmTimer::ToString()
{
    std::ostringstream  oss;
    oss << m_funcName << " Duration " << TotalMilliSecond() << std::endl;
    return oss.str();
}

#endif
