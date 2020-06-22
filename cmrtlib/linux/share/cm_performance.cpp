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
//
// cm_performance.cpp
// Include the standard header and generate the precompiled header.
//
#include <ctime>
#include "cm_include.h"

extern "C" int32_t QueryPerformanceFrequency(LARGE_INTEGER *frequency)
{
    struct timespec  res;
    int32_t          ret;

    if ( (ret = clock_getres(CLOCK_MONOTONIC, &res)) != 0 )
    {
        return -1;
    }

    // resolution (precision) can't be in seconds for current machine and OS
    if (res.tv_sec != 0)
    {
        return -1;
    }
    frequency->QuadPart = (1000000000LL) / res.tv_nsec;

    return 0;
}

extern "C" int32_t QueryPerformanceCounter(LARGE_INTEGER *performanceCount)
{
    struct timespec     res;
    struct timespec     t;
    int32_t             ret;

    if ( (ret = clock_getres (CLOCK_MONOTONIC, &res)) != 0 )
    {
        return -1;
    }
    if (res.tv_sec != 0)
    { // resolution (precision) can't be in seconds for current machine and OS
        return -1;
    }
    if( (ret = clock_gettime(CLOCK_MONOTONIC, &t)) != 0)
    {
        return -1;
    }
    performanceCount->QuadPart = (1000000000LL * t.tv_sec +
        t.tv_nsec) / res.tv_nsec;

    return 0;
}

