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
//! \file      cm_ftrace.cpp
//! \brief     Class Cm Ftrace definitions
//!

#include "cm_ftrace.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "cm_task_internal.h"

#define TRACE_MARKER_PATH "/sys/kernel/debug/tracing/trace_marker"

#define TRACE_FLAG_PATH "/sys/kernel/debug/tracing/tracing_on"

#define MSG_LENGTH 1024

#define PRINT_TO_STRING(_fmt, _value)    \
    byteInput = snprintf(msgBuf + byteOffset, MSG_LENGTH - byteOffset, _fmt, _value); \
    byteOffset += byteInput;

//Global static pointer to ensure a single instance
CmFtrace* CmFtrace::m_ftrace = nullptr;

CmFtrace::CmFtrace()
{
    m_filehandle = open( TRACE_MARKER_PATH, O_WRONLY );
}

CmFtrace::~CmFtrace()
{
    if(m_filehandle >= 0)
    {
        close(m_filehandle);
    }
}

CmFtrace* CmFtrace::GetInstance()
{
    if(m_ftrace == nullptr)
    {
        m_ftrace = new (std::nothrow) CmFtrace();
    }

    return m_ftrace;
}

void CmFtrace::WriteTaskProfilingInfo(CM_PROFILING_INFO *taskInfo)
{
    if(taskInfo == nullptr)
    {
        return ;
    }

    char msgBuf[MSG_LENGTH];
    uint byteOffset = 0;
    uint byteInput = 0;

    PRINT_TO_STRING("%s: ", "mdf_v1")
    PRINT_TO_STRING("kernelcount=%d|", taskInfo->kernelCount);
    PRINT_TO_STRING("taskid=%d|",      taskInfo->taskID);
    PRINT_TO_STRING("threadid=%u|",    taskInfo->threadID);

    uint kernelNameOffset = 0;
    for(uint i=0 ; i< taskInfo->kernelCount; i++)
    {
        //Kernel name.
        char *kernelname = taskInfo->kernelNames + kernelNameOffset;
        PRINT_TO_STRING("kernelname=%s|", kernelname);
        kernelNameOffset += strlen(kernelname) + 1;

        //Local work width&height
        PRINT_TO_STRING("localwidth=%d|", taskInfo->localWorkWidth[i]);
        PRINT_TO_STRING("localheight=%d|", taskInfo->localWorkHeight[i]);

        //Global work width&height
        PRINT_TO_STRING("globalwidth=%d|",  taskInfo->globalWorkWidth[i]);
        PRINT_TO_STRING("globalheight=%d|", taskInfo->globalWorkHeight[i]);
    }

    //Note: enqueuetime/flushtime/completetime are measured in performance counter
    //Don't need to convert to nanosec on Linux since MOS_QueryPerformanceFrequency returns 1
    //On linux, we can't get the gpu ticks when the command submitted, so that
    //we can't do the adjustment the ticks b/w cpu and gpu, as what we did on windoes by GetGpuTime
    //hw_start_time = flush_time; hw_end_time = hw_start_time + kernel_execution_time

    LARGE_INTEGER kernelExeTime;
    kernelExeTime.QuadPart = taskInfo->hwEndTime.QuadPart - taskInfo->hwStartTime.QuadPart;

    //write time stampes
    PRINT_TO_STRING("enqueuetime=%lld|",   (long long)taskInfo->enqueueTime.QuadPart);
    PRINT_TO_STRING("flushtime=%lld|",     (long long)taskInfo->flushTime.QuadPart);
    PRINT_TO_STRING("hwstarttime=%lld|",   (long long)taskInfo->flushTime.QuadPart);
    PRINT_TO_STRING("hwendtime=%lld|",     (long long)(taskInfo->flushTime.QuadPart + kernelExeTime.QuadPart) );
    PRINT_TO_STRING("completetime=%lld\n", (long long)taskInfo->completeTime.QuadPart);

    // write message to trace_marker
    size_t writeSize = write(m_filehandle, msgBuf, byteOffset);

    return;
}
