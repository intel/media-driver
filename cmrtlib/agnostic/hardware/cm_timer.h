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
#ifndef CMRTLIB_AGNOSTIC_HARDWARE_CM_TIMER_H_
#define CMRTLIB_AGNOSTIC_HARDWARE_CM_TIMER_H_

#include "cm_def_hw.h"
#include "cm_include.h"

#if MDF_PROFILER_ENABLED
//!
//! CM Timer
//!
class CmTimer
{
public:
    CmTimer(const char *functionName);

    ~CmTimer();

private:
    void Start();

    void Stop();

    float GetTimeinMs();

    void InsertEventStartFlag();

    void InsertEventEndFlag();

    uint64_t m_cycles;

    LARGE_INTEGER m_start;

    LARGE_INTEGER m_end;

    LARGE_INTEGER m_freq;

    char *m_funcName;
};

#endif  // #if MDF_PROFILER_ENABLED

#endif  // #ifndef CMRTLIB_AGNOSTIC_HARDWARE_CM_TIMER_H_
