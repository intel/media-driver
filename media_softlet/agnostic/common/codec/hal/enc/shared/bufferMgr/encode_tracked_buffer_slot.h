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
//! \file     encode_buffer_slot.h
//! \brief    Defines the interface for buffer slot
//! \details  The slot manages the buffers with the same type
//!
#ifndef __ENCODE_TRACKED_BUFFER_SLOT_H__
#define __ENCODE_TRACKED_BUFFER_SLOT_H__

#include "encode_tracked_buffer.h"
#include "media_class_trace.h"
#include "mos_defs.h"
#include <stdint.h>
#include <map>
#include <memory>

namespace encode {

class BufferQueue;
class BufferSlot
{
public:
    //!
    //! \brief  Constructor
    //! \param  [in] tracker
    //!         pointer to BufferTracker
    //!
    BufferSlot(TrackedBuffer* tracker);

    //!
    //! \brief  Destructor
    //!
    ~BufferSlot();

    //!
    //! \brief  Set the busy flag to show that the slot is been using
    //! \return None
    //!
    void SetBusy()
    {
        m_isBusy = true;
    }

    //!
    //! \brief  Check whether the slot is free to reuse
    //! \return bool
    //!         true if the slot is free to reuse, otherwise false
    //!
    bool IsFree() const
    {
        return !m_isBusy;
    }

    //!
    //! \brief  Reset the slot status and return all buffers to allocator
    //! \return MOS_SURFACE *
    //!         MOS_SURFACE * if success, else nullptr if the pool is empty
    //!
    MOS_STATUS Reset();

    //!
    //! \brief  Set the frame index value for current slot
    //! \param  [in] frameIdx
    //!         Frame index from application
    //! \return void
    //!
    inline void SetFrameIdx(uint8_t frameIdx) { m_frameIndex = frameIdx; }

    //!
    //! \brief  Get associated frame index
    //! \return uint8_t
    //!         Frame index
    //!
    inline uint8_t GetFrameIdx() const { return m_frameIndex; }

    //!
    //! \brief  Get resource from current slot
    //! \param  [in]type
    //!         BufferType
    //! \return Pointer of resource
    //!         Pointer of resource if success, else nullptr if the pool is empty
    //!
    void *GetResource(BufferType type);

protected:
    uint8_t        m_frameIndex = 0;       //!< frame index associated with current slot
    TrackedBuffer *m_tracker  = nullptr;   //!< pointer to TrackedBuffer
    bool           m_isBusy   = false;     //!< whether the slot is been using

    std::map<BufferType, void *>                        m_buffers      = {};  //!< buffers attached with current slot
    std::map<BufferType, std::shared_ptr<BufferQueue> > m_bufferQueues = {};  //!< buffer queue for all types

MEDIA_CLASS_DEFINE_END(encode__BufferSlot)
};
}
#endif // !__ENCODE_TRACKED_BUFFER_SLOT_H__
