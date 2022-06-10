/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     decode_unique_id.h
//! \brief    Defines the utils interface for decode unique id.
//! \details  Defines the utils interface to declare the unique decode id
//!           among decoders.
//!

#ifndef __DECODE_UNIQUE_ID_H__
#define __DECODE_UNIQUE_ID_H__

#include "decode_utils.h"
#include "media_class_trace.h"
#include <stdint.h>

namespace decode {

class DecodeUniqueId
{
protected:
    static uint32_t GetGlobalUniqueId() { return m_decodeUniqueId; }
    static void     IncGlobalUniqueId()
    {
        AutoLock autoLock(m_mutex);
        m_decodeUniqueId++;
    }

private:
    static uint32_t m_decodeUniqueId;
    static Mutex    m_mutex;

MEDIA_CLASS_DEFINE_END(decode__DecodeUniqueId)
};

#define DeclareDecodeUniqueId(type, name) \
    class name##type##UniqueId : public DecodeUniqueId \
    { \
    public: \
        name##type##UniqueId() \
        { \
            m_##name##type = GetGlobalUniqueId(); \
            IncGlobalUniqueId(); \
        } \
        inline uint32_t GetUniqueId() \
        { \
            return m_##name##type; \
        } \
    private: \
        uint32_t m_##name##type; \
    }; \
    class name##type##UniqueId name##type##UniqueId_instance_;

#define GetDecodeUniqueId(scope, type, name) \
    ((scope)->name##type##UniqueId_instance_.GetUniqueId())

}

#endif
