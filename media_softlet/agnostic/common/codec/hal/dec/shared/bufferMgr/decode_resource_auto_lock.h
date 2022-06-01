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
//! \file     decode_resource_auto_lock.h
//! \brief    Defines the interface for resource auto lock
//! \details  The interface implements the auto lock and auto unlock function.
//!

#ifndef __DECODE_RESOURCE_AUTO_LOCK_H__
#define __DECODE_RESOURCE_AUTO_LOCK_H__
#include "decode_allocator.h"

namespace decode
{

class ResourceAutoLock
{
public:
    //!
    //! \brief  Constructor
    //!
    ResourceAutoLock(DecodeAllocator* allocator, PMOS_RESOURCE resource):
        m_allocator(allocator),
        m_resource(resource)
    {
    }

    //!
    //! \brief  Destructor
    //!
    virtual ~ResourceAutoLock()
    {
        if (m_allocator != nullptr && m_resource != nullptr)
        {
            m_allocator->UnLock(m_resource);
        }
    }

    void* LockResourceForWrite()
    {
        if (m_allocator == nullptr || m_resource == nullptr)
        {
            return nullptr;
        }
        return m_allocator->LockResourceForWrite(m_resource);
    }

    void* LockResourceWithNoOverwrite()
    {
        if (m_allocator == nullptr || m_resource == nullptr)
        {
            return nullptr;
        }

        return m_allocator->LockResourceWithNoOverwrite(m_resource);
    }

    void* LockResourceForRead()
    {
        if (m_allocator == nullptr || m_resource == nullptr)
        {
            return nullptr;
        }
        return m_allocator->LockResourceForRead(m_resource);
    }

private:
    DecodeAllocator* m_allocator;
    PMOS_RESOURCE    m_resource;

MEDIA_CLASS_DEFINE_END(decode__ResourceAutoLock)
};

}  // namespace decode
#endif  // !__DECODE_RESOURCE_AUTO_LOCK_H__