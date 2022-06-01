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
//! \file     encode_recycle_res_queue.h
//! \brief    Defines the interface for recycle resources queue
//!

#ifndef __ENCODE_RECYCLE_RES_QUEUE_H__
#define __ENCODE_RECYCLE_RES_QUEUE_H__

#include "media_class_trace.h"
#include "mos_defs.h"
#include "mos_os.h"
#include <stdint.h>
#include <vector>

namespace encode
{
class EncodeAllocator;

class RecycleQueue
{
public:
    enum ResourceType
    {
        INVALID,
        BUFFER,
        SURFACE
    };

    //!
    //! \brief  Constructor
    //! \param  [in] param
    //!         reference of MOS_ALLOC_GFXRES_PARAMS
    //! \param  [in] maxLimit
    //!         The max limitation
    //!
    RecycleQueue(const MOS_ALLOC_GFXRES_PARAMS &param, EncodeAllocator *allocator, uint32_t maxLimit);

    //!
    //! \brief  Destructor
    //!
    virtual ~RecycleQueue();

    //!
    //! \brief  Get resource from the free queue
    //! \return void *
    //!         void * if success, else nullptr
    //!
    void *GetResource(uint32_t frameInde, ResourceType type);

    //!
    //! \brief  destroy all resources in the resources queue
    //! \param  [in] allocator
    //!         Pointer to EncodeAllocator
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroyAllResources(EncodeAllocator *allocator);

    //!
    //! \brief  Whether the m_type match with the given type
    //! \return bool
    //!         true if matched, else false
    //!
    bool IsTypeMatched(ResourceType type) const
    {
        // Still didn't set the type, return true here, will be set later
        if (m_type == INVALID)
        {
            return true;
        }

        return m_type == type;
    }
private:
    uint32_t                m_maxLimit = 0;
    ResourceType            m_type = INVALID;
    MOS_ALLOC_GFXRES_PARAMS m_param = {};
    EncodeAllocator         *m_allocator = nullptr;  //!< encoder allocator
    std::vector<void *>     m_resources;      //<! All resources

MEDIA_CLASS_DEFINE_END(encode__RecycleQueue)
};
}  // namespace encode
#endif  // !__ENCODE_RECYCLE_RES_QUEUE_H__
