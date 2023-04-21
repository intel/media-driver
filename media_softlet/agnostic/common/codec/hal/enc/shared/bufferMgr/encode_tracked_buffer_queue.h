/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     encode_buffer_allocator.h
//! \brief    Defines the interface for buffer allocator
//! \details  The allocator manages the buffers with the same type and alloc parameter
//!
#ifndef __ENCODE_TRACKED_BUFFER_QUEUE_H__
#define __ENCODE_TRACKED_BUFFER_QUEUE_H__

#include "media_class_trace.h"
#include "mos_defs.h"
#include "mos_defs_specific.h"
#include "mos_os.h"
#include <stdint.h>
#include <vector>

namespace encode
{

enum ResourceType
{
    invalidResource,
    bufferResource,
    surfaceResource
};

class EncodeAllocator;
class BufferQueue
{
public:
    //!
    //! \brief  Constructor
    //! \param  [in] allocator
    //!         Pointer to EncodeAllocator
    //! \param  [in] param
    //!         reference to MOS_ALLOC_GFXRES_PARAMS
    //! \param  [in] maxCount
    //!         the max buffer count allocated
    //!
    BufferQueue(EncodeAllocator *allocator, MOS_ALLOC_GFXRES_PARAMS &param, uint32_t maxCount);

    //!
    //! \brief  Destructor
    //!
    ~BufferQueue();

    //!
    //! \brief  Acquire resource from the allocator pool
    //! \return Pointer of resource
    //!         Pointer of resource if success, else nullptr if the pool is empty
    //!
    void *AcquireResource();

    //!
    //! \brief  Release the resource and return to the pool
    //! \param  [in] resource
    //!         Pointer of resource
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReleaseResource(void *resource);

    //!
    //! \brief  Check whether it's safe to destory the allocator
    //! \return bool
    //!         true if all buffers are released, otherwise return false
    //!
    bool SafeToDestory();

    void SetResourceType(ResourceType resType);

protected:
    //!
    //! \brief  Allocate resource
    //! \return Pointer of resource
    //!         Pointer of resource if success, nullptr if the allocated surfaces reach the max count
    //!
    void *AllocateResource();

    //!
    //! \brief  Destroy resource
    //! \param  [in] resource
    //!         Pointer of resource
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestoryResource(void *resource);

protected:
    uint32_t   m_maxCount   = 0;        //!< max buffer count
    uint32_t   m_allocCount = 0;        //!< allocated buffer count
    PMOS_MUTEX m_mutex      = nullptr;  //!< mutex

    EncodeAllocator *          m_allocator    = nullptr;    //!< encoder allocator
    MOS_ALLOC_GFXRES_PARAMS    m_allocParam   = {};         //!< allocate parameter
    std::vector<void *>        m_resourcePool = {};         //!< resource pool
    std::vector<void *>        m_resources    = {};         //!< all allocated resources

    ResourceType m_resourceType = ResourceType::bufferResource;

MEDIA_CLASS_DEFINE_END(encode__BufferQueue)
};

}  // namespace encode

#endif  // !__ENCODE_TRACKED_BUFFER_QUEUE_H__
