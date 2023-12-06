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
#ifndef __ENCODE_UTILS_H__
#define __ENCODE_UTILS_H__
#include <mutex>
#include "mos_util_debug.h"
#include "mos_utilities.h"

#define _SW_BRC _MEDIA_RESERVED && (_DEBUG || _RELEASE_INTERNAL)

enum HuCFunction
{
    NONE_BRC = 0,
    BRC_INIT,
    BRC_RESET,
    BRC_UPDATE,
    LA_INIT,
    LA_RESET,
    LA_UPDATE,
    PAK_INTEGRATE,
    HPU_VP9
};

//------------------------------------------------------------------------------
// Macros specific to MOS_CODEC_SUBCOMP_ENCODE sub-comp
//------------------------------------------------------------------------------
#define ENCODE_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _expr)

#define ENCODE_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _message, ##__VA_ARGS__)

#define ENCODE_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _message, ##__VA_ARGS__)

#define ENCODE_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _message, ##__VA_ARGS__)

#define ENCODE_CHK_NULL_RETURN(_ptr)                                                  \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _ptr)

#define ENCODE_CHK_NULL_NO_STATUS_RETURN(_ptr)                                                  \
    MOS_CHK_NULL_NO_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _ptr)

#define ENCODE_CHK_STATUS_RETURN(_stmt)                                               \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _stmt)

#define ENCODE_CHK_STATUS_NO_STATUS_RETURN(_stmt)                                               \
    MOS_CHK_STATUS_NO_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _stmt)

#define ENCODE_CHK_COND_RETURN(_stmt, _message, ...)                                            \
    MOS_CHK_COND_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _stmt, _message, ##__VA_ARGS__)

#define ENCODE_CHK_STATUS_MESSAGE_RETURN(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _stmt, _message, ##__VA_ARGS__)

#define ENCODE_CHK_NULL_WITH_DESTROY_RETURN_VALUE(_ptr, destroyFunction) \
    MOS_CHK_COND_WITH_DESTROY_RETURN_VALUE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, (nullptr == _ptr), destroyFunction, MOS_STATUS_NULL_POINTER, "error nullptr!")

#define ENCODE_CHK_STATUS_WITH_DESTROY_RETURN_VALUE(_stmt, destroyFunction)                                                                                     \
{                                                                                                                                                               \
    MOS_STATUS sts = (MOS_STATUS)(_stmt);                                                                                                                       \
    MOS_CHK_COND_WITH_DESTROY_RETURN_VALUE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, (MOS_STATUS_SUCCESS != sts), destroyFunction, sts, "error status!")   \
}

namespace encode {

class Trace
{
public:
    Trace(const char* name) : m_name(name)
    {
        ENCODE_VERBOSEMESSAGE("Enter function:%s\r\n", name);
    }

    ~Trace()
    {
        ENCODE_VERBOSEMESSAGE("Exit function:%s\r\n", m_name);
    }

protected:
    const char* m_name;
};

class AutoLock
{
public:
    AutoLock(PMOS_MUTEX mutex) : m_mutex(mutex) { MosUtilities::MosLockMutex(mutex); }
    ~AutoLock() { MosUtilities::MosUnlockMutex(m_mutex); }
protected:
    PMOS_MUTEX m_mutex;
MEDIA_CLASS_DEFINE_END(encode__AutoLock)
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
MEDIA_CLASS_DEFINE_END(encode__Condition)
};

//!
//! \brief    Allocate data list with specific type and length 
//!
//! \param    [in,out] dataList
//!           Pointer to a type * pointer. Specify the address of the memory handles 
//! \param    [in] length
//!           Specify the number of data members 
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <class type>
MOS_STATUS EncodeAllocateDataList(type **dataList, uint32_t length)
{
    type *ptr;
    ptr = (type *)MOS_AllocAndZeroMemory(sizeof(type) * length);
    if (ptr == nullptr)
    {
        ENCODE_ASSERTMESSAGE("No memory to allocate CodecHal data list.");
        return MOS_STATUS_NO_SPACE;
    }
    for (uint32_t i = 0; i < length; i++)
    {
        dataList[i] = &(ptr[i]);
    }
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Free data list
//!
//! \param    [in,out] dataList
//!           Pointer to a type * pointer. Specify the address of the memory handles 
//! \param    [in] length
//!           Specify the number of data members 
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <class type>
MOS_STATUS EncodeFreeDataList(type **dataList, uint32_t length)
{
    type* ptr;
    ptr = dataList[0];
    if (ptr)
    {
        MOS_FreeMemory(ptr);
    }
    for (uint32_t i = 0; i < length; i++)
    {
        dataList[i] = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

}

#define ENCODE_FUNC_CALL() encode::Trace trace(__FUNCTION__);

#endif // !__ENCODE_UTILS_H__
