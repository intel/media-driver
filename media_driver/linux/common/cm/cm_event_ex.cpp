/*
* Copyright (c) 2018-2019, Intel Corporation
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
//! \file      cm_event_ex.cpp
//! \brief     Contains Class CmEventEx  definitions
//!

#include "cm_event_ex.h"
#include "cm_hal.h"
#include "cm_tracker.h"
#include "cm_notifier.h"

CmEventEx::~CmEventEx()
{
    if (m_cmTracker)
    {
        m_cmTracker->DeAssociateEvent(this);
    }
}

void CmEventEx::SetTaskOsData(MOS_RESOURCE *resource, HANDLE handle)
{
    UNUSED(handle);
    if (resource != nullptr)
    {
        mos_bo_reference((MOS_LINUX_BO *)(resource->bo));
        m_osData = resource->bo;
    }
}

int32_t CmEventEx::WaitForTaskFinished(uint32_t timeOutMs)
{
    int32_t result = mos_gem_bo_wait((MOS_LINUX_BO*)m_osData, 1000000LL*timeOutMs);
    mos_gem_bo_clear_relocs((MOS_LINUX_BO*)m_osData, 0);

    if (result) {
        return CM_EXCEED_MAX_TIMEOUT;   //translate the drm ecode (-ETIME or potentional variants) to CM ecode.
    }
    if (m_state != CM_STATUS_FINISHED)
    {
        Query();
    }

    if (m_state == CM_STATUS_FINISHED)
    {
        return CM_SUCCESS;
    }
    else
    {
        // if bo_wait() returns success but status is not finished in time stamp
        // it indicates something wrong in KMD, such as gpu reset happens.
        // need to return error code to application.
        return CM_EXCEED_MAX_TIMEOUT;
    }
}

int32_t CmEventEx::GetStatus(CM_STATUS &status)
{
    static const uint64_t TIME_OUT = 10000;  // 10 microseconds.
    if (m_state != CM_STATUS_FINISHED)
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
    status = m_state;
    return CM_SUCCESS;
}


void CmEventEx::RleaseOsData()
{
    mos_bo_unreference((MOS_LINUX_BO*)m_osData);
}
