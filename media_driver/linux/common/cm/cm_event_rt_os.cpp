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
//! \file      cm_event_rt_os.cpp
//! \brief     Contains Linux-dependent CmEventRT member functions.
//!

#include "cm_event_rt.h"
#include "cm_queue_rt.h"

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//! Query the status of the task associated with the event
//! An event is generated when a task ( one kernel or multiples kernels running concurrently )
//! is enqueued.
//! This is a non-blocking call.
//! INPUT:
//!     The reference to status. For now only two status, CM_STATUS_QUEUED and CM_STATUS_FINISHED, are supported
//! OUTPUT:
//!     CM_SUCCESS if the status is successfully returned;
//!     CM_FAILURE if not.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmEventRT::GetStatus(CM_STATUS &status)
{
    static const uint64_t TIME_OUT = 10000; // 10 microseconds.
    if (m_status == CM_STATUS_FLUSHED || m_status == CM_STATUS_STARTED)
    {
        if (!m_osSignalTriggered)
        {
            CM_CHK_NULL_RETURN_CMERROR(m_osData);
            MOS_LINUX_BO *buffer_object = reinterpret_cast<MOS_LINUX_BO*>(m_osData);
            int result = mos_gem_bo_wait(buffer_object, TIME_OUT);
            mos_gem_bo_clear_relocs(buffer_object, 0);
            m_osSignalTriggered = (result == 0);
        }
        if (m_osSignalTriggered)
        {
            Query();
        }
    }

    m_queue->FlushTaskWithoutSync();

    status = m_status;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//! Wait for the task completed with event-driven mechanism
//! When the task finished, CM will be notified and waken up by KMD.
//! INPUT:
//!     Timeout in Milliseconds
//! OUTPUT:
//!     CM_SUCCESS:  if the task finished
//!     CM_EXCEED_MAX_TIMEOUT:  if timeout in synchoinization system call.
//!     CM_FEATURE_NOT_SUPPORTED_IN_DRIVER: if driver out-of-sync.
//!     CM_EVENT_DRIVEN_FAILURE : if synchronization system call returns WAIT_FAILED.
//*----------------------------------------------------------------------------------
CM_RT_API int32_t CmEventRT::WaitForTaskFinished(uint32_t timeOutMs)
{
    int32_t result    = CM_SUCCESS;

    if( m_status == CM_STATUS_FINISHED )
        goto finish;

    //Make sure task flushed
    while ( m_status == CM_STATUS_QUEUED )
    {
        m_queue->FlushTaskWithoutSync();  //Flush none if 1st task NOT finished yet
    }

    CM_ASSERT(m_osData != nullptr);

    //Wait bo finished
    result = mos_gem_bo_wait((MOS_LINUX_BO*)m_osData, 1000000LL*timeOutMs);
    mos_gem_bo_clear_relocs((MOS_LINUX_BO*)m_osData, 0);
    if (result) {
        result = CM_EXCEED_MAX_TIMEOUT;   //translate the drm ecode (-ETIME or potentional variants) to CM ecode.
        goto finish;
    }

    //Query status
    Query();
    if(m_status != CM_STATUS_FINISHED)
    {
        // if bo_wai() returns success but status is not finished in time stamp
        // it indicates something wrong in KMD, such as gpu reset happens.
        // need to return error code to application.
        result = CM_EXCEED_MAX_TIMEOUT;
        goto finish;
    }

    //Call flush to pop/destroy finished task and flush task in EnqueueedQueue.
    m_queue->FlushTaskWithoutSync();

finish:
    return result;
}

//*-----------------------------------------------------------------------------
//! Unreference the bo in linux.
//! INPUT:
//!     bo in a void * format
//! OUTPUT:
//!     No output is needed
//*-----------------------------------------------------------------------------
void CmEventRT::UnreferenceIfNeeded(void  *pdata)
{
    if( pdata )
    {
        mos_bo_unreference((MOS_LINUX_BO*)pdata);
    }
}
}  // namespace
