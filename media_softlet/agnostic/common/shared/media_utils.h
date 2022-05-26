/*
* Copyright (c) 2018-2021, Intel Corporation
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
#ifndef __MEDIA_UTILS_H__
#define __MEDIA_UTILS_H__
#include <mutex>
#include "mos_util_debug.h"
#include "media_class_trace.h"

//------------------------------------------------------------------------------
// Macros specific to MOS_CODEC_SUBCOMP_ENCODE sub-comp
//------------------------------------------------------------------------------
#define MEDIA_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _expr)

#define MEDIA_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _message, ##__VA_ARGS__)

#define MEDIA_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _message, ##__VA_ARGS__)

#define MEDIA_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _message, ##__VA_ARGS__)

#define MEDIA_CHK_NULL_RETURN(_ptr)                                                  \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _ptr)

#define MEDIA_CHK_STATUS_RETURN(_stmt)                                               \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _stmt)

#define MEDIA_CHK_STATUS_MESSAGE_RETURN(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _stmt, _message, ##__VA_ARGS__)

class Trace
{
public:
    Trace(const char* name) : m_name(name)
    {
        MEDIA_VERBOSEMESSAGE("Enter function:%s\r\n", name);
    }

    ~Trace()
    {
        MEDIA_VERBOSEMESSAGE("Exit function:%s\r\n", m_name);
    }

protected:
    const char* m_name;
MEDIA_CLASS_DEFINE_END(Trace)
};

class AutoLock
{
public:
    AutoLock(PMOS_MUTEX mutex) : m_mutex(mutex) { MosUtilities::MosLockMutex(mutex); }
    ~AutoLock() { MosUtilities::MosUnlockMutex(m_mutex); }
protected:
    PMOS_MUTEX m_mutex;
MEDIA_CLASS_DEFINE_END(AutoLock)
};

class Condition
{
public:
    Condition()
    {
        m_sem = MosUtilities::MosCreateSemaphore(0, 1);
    }

    ~Condition()
    {
        MosUtilities::MosDestroySemaphore(m_sem);
    }
    MOS_STATUS Wait(PMOS_MUTEX mutex)
    {
        MOS_STATUS status = MOS_STATUS_SUCCESS;
        MosUtilities::MosUnlockMutex(mutex);
        status = MosUtilities::MosWaitSemaphore(m_sem, 5000);
        MosUtilities::MosLockMutex(mutex);
        return status;
    }
    MOS_STATUS Signal()
    {
        MosUtilities::MosPostSemaphore(m_sem, 1);
        return MOS_STATUS_SUCCESS;
    }

protected:
    PMOS_SEMAPHORE m_sem;
MEDIA_CLASS_DEFINE_END(Condition)
};

#define MEDIA_FUNC_CALL() Trace trace(__FUNCTION__);

#endif // !__MEDIA_UTILS_H__
