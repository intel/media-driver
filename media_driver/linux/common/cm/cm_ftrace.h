/*
* Copyright (c) 2007-2017, Intel Corporation
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
//! \file      cm_ftrace.h
//! \brief     Contains Class Cm Ftrace definitions
//!

#ifndef MEDIADRIVER_LINUX_COMMON_CM_CMFTRACE_H_
#define MEDIADRIVER_LINUX_COMMON_CM_CMFTRACE_H_

#include "cm_common.h"

struct CM_PROFILING_INFO;

class CmFtrace
{
public:
    static CmFtrace* GetInstance();
    void   WriteTaskProfilingInfo(CM_PROFILING_INFO *taskInfo);

private:

    static CmFtrace* m_ftrace; // static instance
    int m_filehandle;          //ftrace marker file descriptor

    CmFtrace( const CmFtrace& ); // disable operator and constructor
    void operator=( const CmFtrace& );

    CmFtrace();
    ~CmFtrace();
};

#endif  // #ifndef MEDIADRIVER_LINUX_COMMON_CM_CMFTRACE_H_
