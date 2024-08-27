/*
* Copyright (c) 2018-2024, Intel Corporation
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
//! \file     encode_recycle_resource.h
//! \brief    Defines the interface for recycle resource
//! \details  The manager for the recycle resources
//!

#ifndef __ENCODE_RECYCLE_RESOURCE_H__
#define __ENCODE_RECYCLE_RESOURCE_H__

#include "media_class_trace.h"
#include "mos_defs.h"
#include "mos_os.h"
#include "mos_os_specific.h"
#include <stdint.h>
#include <map>
#include <utility>
#if _MEDIA_RESERVED
#include "encode_recycle_resource_ext.h"
#endif

namespace encode
{
    enum RecycleResId
    {
        PakInfo = 0,
        FrameStatStreamOutBuffer,
        VdencBRCHistoryBuffer,
        VdencBrcDebugBuffer,
        LcuBaseAddressBuffer,
        VdencBrcPakMmioBuffer,
        VdencStatsBuffer,
        StreamInBuffer,
        HucRoiOutputBuffer,
        HucRoiDeltaQpBuffer,
        HucRoiMapBuffer,
        BrcPakStatisticBuffer,
        BrcPakStatisticBufferFull,
        PakSliceSizeStreamOutBuffer,
        CuRecordStreamOutBuffer,
        CuCountBuffer,
        PreEncRawSurface,
        PreEncRef0,
        PreEncRef1,
        VdaqmBuffer0,
        VdaqmBuffer1,
        VdaqmBuffer2,
        VdaqmBuffer3,
        VdaqmBuffer4,
#if _MEDIA_RESERVED
#define RECYCLE_IDS_EXT
#include "encode_recycle_resource_ext.h"
#undef RECYCLE_IDS_EXT
#endif
    };

class EncodeAllocator;
class RecycleQueue;

class RecycleResource
{
public:
    //!
    //! \brief  Constructor
    //! \param  [in] allocator
    //!         Pointer to EncodeAllocator
    //!
    RecycleResource(EncodeAllocator *allocator);

    //!
    //! \brief  Destructor
    //!
    virtual ~RecycleResource();

    //!
    //! \brief  Register resource
    //! \param  [in] id
    //!         The ID of resource which defined in RecycleResId
    //! \param  [in] param
    //!         MOS_ALLOC_GFXRES_PARAMS
    //! \param  [in] maxLimit
    //!         The limitation of number of the resource
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RegisterResource(RecycleResId id, MOS_ALLOC_GFXRES_PARAMS param, uint32_t maxLimit = m_maxRecycleNum);

    //!
    //! \brief  Get Surface
    //! \param  [in] id
    //!         The ID of resource which defined in RecycleResId
    //! \param  [in] frameIndex
    //!         Frame index
    //! \return MOS_SURFACE *
    //!         MOS_SURFACE * if success, else nullptr if the pool is empty
    //!
    MOS_SURFACE *GetSurface(RecycleResId id, uint32_t frameIndex);

    //!
    //! \brief  Get Buffer
    //! \param  [in] id
    //!         The ID of resource which defined in RecycleResId
    //! \param  [in] frameIndex
    //!         Frame index
    //! \return MOS_RESOURCE *
    //!         MOS_RESOURCE * if success, else nullptr if the pool is empty
    //!
    virtual MOS_RESOURCE* GetBuffer(RecycleResId id, uint32_t frameIndex);

protected:
    //!
    //! \brief  Get resource queue
    //! \param  [in] id
    //!         The ID of resource which defined in RecycleResId
    //! \return RecycleQueue *
    //!         RecycleQueue * if success, else nullptr
    //!
    RecycleQueue *GetResQueue(RecycleResId id)
    {
        auto it = m_resourceQueues.find(id);

        if (it == m_resourceQueues.end())
        {
            return nullptr;
        }

        return it->second;
    }

    static const uint8_t m_maxRecycleNum = 6;
    EncodeAllocator *m_allocator     = nullptr;  //!< encoder allocator

    std::map<RecycleResId, RecycleQueue *> m_resourceQueues{};  //!< resource queues

MEDIA_CLASS_DEFINE_END(encode__RecycleResource)
};
}  // namespace encode
#endif  // !__ENCODE_RECYCLE_RESOURCE_H__
