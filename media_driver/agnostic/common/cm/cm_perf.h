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
//!
//! \file      cm_perf.h
//! \brief     Contains functions which recode time and calculate performance
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMPERF_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMPERF_H_

#include <string>
#include "mos_os.h"

#if CM_LOG_ON

class CmTimer
{
public:
    CmTimer(std::string FunctionName);
    ~CmTimer();
    void Stop();
    std::string ToString();

private:
    float TotalMilliSecond();
    void Start();

    uint64_t            m_cycles;
    LARGE_INTEGER       m_start;
    LARGE_INTEGER       m_end;
    LARGE_INTEGER       m_freq;
    std::string         m_funcName;
    bool                m_bstopped;
};

#endif  // #if CM_LOG_ON

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMPERF_H_
