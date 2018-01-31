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
//! \file      cm_csync.h 
//! \brief     Contains CSync and CLock definitions 
//!

#ifndef MEDIADRIVER_LINUX_COMMON_CM_CMCSYNC_H_
#define MEDIADRIVER_LINUX_COMMON_CM_CMCSYNC_H_

#include "cm_debug.h"

namespace CMRT_UMD
{
class CSync
{
public:
    CSync()
    {
        int32_t ret = 0;
        ret = pthread_mutex_init(&m_criticalSection, nullptr);
        if (ret != 0)
        {
            CM_ASSERTMESSAGE("Error: Failed in pthread_mutex_init.");
        }
    }

    ~CSync()
    {
        int32_t ret = 0 ;
        ret = pthread_mutex_destroy(&m_criticalSection);
        if (ret != 0)
        {
            CM_ASSERTMESSAGE("Error: Failed in pthread_mutex_destroy.");
        }
    }

    void Acquire()
    {
        int32_t ret = 0;
        ret = pthread_mutex_lock(&m_criticalSection);
        if (ret != 0)
        {
            CM_ASSERTMESSAGE("Error: Failed in pthread_mutex_lock.");
        }
    }

    void Release()
    {
        int32_t ret = 0;
        ret = pthread_mutex_unlock(&m_criticalSection);
        if (ret != 0)
        {
            CM_ASSERTMESSAGE("Error: Failed in pthread_mutex_unlock.");
        }
    }

private:
    pthread_mutex_t m_criticalSection;
};

class CLock
{
public:
    CLock(CSync &refSync) : m_refSync(refSync) { Lock(); }
    ~CLock() { Unlock(); }

private:
    CSync &m_refSync;                     // Synchronization object

    CLock(const CLock &refcSource);
    CLock &operator=(const CLock &refcSource);
    void Lock() { m_refSync.Acquire(); }
    void Unlock() { m_refSync.Release(); }
};
}; //namespace CMRT_UMD

#endif // #ifndef MEDIADRIVER_LINUX_COMMON_CM_CMCSYNC_H_
