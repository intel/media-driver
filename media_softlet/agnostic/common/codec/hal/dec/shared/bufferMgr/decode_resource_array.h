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
//! \file     decode_resource_array.h
//! \brief    Defines the interface for surface array
//! \details  The surface array record the index for picking up surface, 
//!           can be used as recycle surfaces.
//!

#ifndef __DECODE_RESOURCE_ARRAY_H__
#define __DECODE_RESOURCE_ARRAY_H__
#include <map>
#include <vector>
#include "mos_os.h"
#include "decode_allocator.h"
#include "decode_utils.h"

namespace decode
{

template <class T>
class ResourceArray
{
public:
    //!
    //! \brief  Constructor
    //!
    ResourceArray(DecodeAllocator *allocator)
        : m_allocator(allocator)
    {}

    //!
    //! \brief  Constructor
    //!
    ResourceArray()
    {}

    //!
    //! \brief  Destructor
    //!
    virtual ~ResourceArray() { Destroy(); }

    //!
    //! \brief  Add one resource to resource queue
    //! \param  [in] resource
    //!         point of resource
    //! \param  [in] size
    //!         size of resource
    //! \return No return
    //!
    void Push(T *resource)
    {
        if (resource != nullptr)
        {
            m_resourceQueue.push_back(resource);
        }
    }

    //!
    //! \brief  Get number of surface array
    //! \return uint32_t
    //!         Return the number of surface array
    //!
    uint32_t ArraySize()
    {
        return m_resourceQueue.size();
    }

    //!
    //! \brief  Get next resource from resource queue, loop back to first element if reach end
    //! \return T* &
    //!         point of resource if success, else nullptr if the array is empty
    //!
    T* & Fetch()
    {
        if (m_resourceQueue.empty())
        {
            return m_empty;
        }

        m_nextIndex++;
        if (m_nextIndex >= m_resourceQueue.size())
        {
            m_nextIndex = 0;
        }

        return m_resourceQueue[m_nextIndex];
    }

    //!
    //! \brief  Get current resource from resource queue
    //! \return T*
    //!         point of resource if success, else nullptr if the array is empty
    //!
    T* Peek()
    {
        if (m_resourceQueue.empty())
        {
            return m_empty;
        }

        return m_resourceQueue[m_nextIndex];
    }

    //!
    //! \brief  Get surface from surface queue by index
    //! \param  [in] index
    //!         resource index
    //! \return T*
    //!         Point to resource
    //!
    T* & operator[] (uint32_t index)
    {
        if (m_resourceQueue.empty())
        {
            return m_empty;
        }

        if (index >= m_resourceQueue.size())
        {
            return m_empty;
        }

        return m_resourceQueue[index];
    }

private:
    //!
    //! \brief  Destroy resource array
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Destroy()
    {
        for (auto &resource : m_resourceQueue)
        {
            if (resource == nullptr)
            {
                continue;
            }
            if (m_allocator != nullptr)
            {
                DECODE_CHK_STATUS(m_allocator->Destroy(resource));
            }
        }

        m_resourceQueue.clear();

        return MOS_STATUS_SUCCESS;
    }

private:
    DecodeAllocator *m_allocator = nullptr;

    std::vector<T *> m_resourceQueue; //!< resource queue
    uint32_t m_nextIndex  = 0;

    T* m_empty = nullptr;

MEDIA_CLASS_DEFINE_END(decode__ResourceArray)
};

}  // namespace decode
#endif  // !__DECODE_SURFACE_ARRAY_H__
