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
//! \file      cm_task_internal_os.cpp 
//! \brief     Contains Class CmTaskInternal  definitions 
//!

#include "cm_task_internal.h"

#include "cm_ftrace.h"
#include "cm_device_rt.h"
#include "cm_event_rt.h"

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Write task info into trace_marker
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::VtuneWriteEventInfo()
{
    if(!m_cmDevice->IsVtuneLogOn())
    {   // return directly if ETW log is off
        return CM_SUCCESS;
    }

    if (m_taskProfilingInfo.kernelCount == 0 ||
        m_taskProfilingInfo.kernelNames == nullptr)
    {
        //Skip ETW Write since information is not filled
        //Vebox/EnqueueWithHints
        return CM_SUCCESS;
    }

    //Get Complete Time
    m_taskEvent->GetCompleteTime(&m_taskProfilingInfo.completeTime);

    //Get HW start/end Time
    m_taskEvent->GetHWStartTime(&m_taskProfilingInfo.hwStartTime);
    m_taskEvent->GetHWEndTime(&m_taskProfilingInfo.hwEndTime);
    CmFtrace *ftrace = CmFtrace::GetInstance();
    if (ftrace == nullptr)
    {
        return CM_NULL_POINTER;
    }
    ftrace->WriteTaskProfilingInfo(&m_taskProfilingInfo);

    return CM_SUCCESS;
}
}  // namespace
