/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     encode_tracked_buffer.h
//! \brief    Defines the interface for buffer tracker
//! \details  The tracker manages the buffers with different type
//!

#ifndef __ENCODE_BUFFER_TRACKER_H__
#define __ENCODE_BUFFER_TRACKER_H__

#include "codec_def_common.h"
#include "encode_tracked_buffer_queue.h"
#include "encode_utils.h"
#include "media_class_trace.h"
#include "mos_defs.h"
#include "mos_defs_specific.h"
#include "mos_os.h"
#include "mos_os_specific.h"
#include <stdint.h>
#include <map>
#include <memory>
#include <vector>

#if _MEDIA_RESERVED
#include "encode_tracked_buffer_ext.h"
#endif

namespace encode
{
// define the buffer types in the tracked list
enum class BufferType
{
    mbCodedBuffer,
    mvDataBuffer,
    mvTemporalBuffer,
    ds4xSurface,
    ds8xSurface,
    segmentIdStreamOutBuffer,
    vdencSegIdStreamOutBuffer,
    bwdAdaptCdfBuffer,
    postCdefReconSurface,
    preMbCodedBuffer,
    preRefSurface,
    preRawSurface,
    preDs4xSurface,
    preDs8xSurface,
    superResRefScaled,
    superResRef4xDsScaled,
    superResRef8xDsScaled,
    preencRef0,
    preencRef1,
    AlignedRawSurface,
};

struct MapBufferResourceType
{
    BufferType   buffer;
    ResourceType type;
};

class EncodeAllocator;
class BufferSlot;
class TrackedBuffer
{
public:
    //!
    //! \brief  Constructor
    //! \param  [in] osInterface
    //!         Pointer to MOS_INTERFACE
    //! \param  [in] maxRefCnt
    //!         max reference frame count
    //! \param  [in] maxNonRefCnt
    //!         max non-reference frame count
    //!
    TrackedBuffer(EncodeAllocator *allocator, uint8_t maxRefCnt, uint8_t maxNonRefCnt);

    //!
    //! \brief  Destructor
    //!
    virtual ~TrackedBuffer();

    //!
    //! \brief  Increase the reference count, default value is 1
    //! \return uint8_t
    //!         return current free slot index
    //!
    virtual uint8_t GetCurrIndex() { return m_currSlotIndex; }

    //!
    //! \brief  Register allocate parameters, it's required when allocate surface
    //! \param  [in] type
    //!         BufferType
    //! \param  [in] param
    //!         MOS_ALLOC_GFXRES_PARAMS
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RegisterParam(BufferType type, MOS_ALLOC_GFXRES_PARAMS param);

    //!
    //! \brief  Acquire buffer before encoding start for each frame
    //! \param  [in] refList
    //!         CODEC_REF_LIST*
    //! \param  [in] isIdrFrame
    //!         indicate whether current frame is IDR frame,
    //!         if true, will reset the buffer tracker status
    //! \param  [in] lazyRelease
    //!         whether use lazy method to realse the resources, the default value is false
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Acquire(CODEC_REF_LIST *refList,
        bool isIdrFrame,
        bool lazyRelease = false);

    //!
    //! \brief  Release buffers when current frame encoding completed
    //!         It should be invoked in GetStatusReport
    //! \param  [in] refList
    //!         CODEC_REF_LIST*
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Release(CODEC_REF_LIST *refList);

    //!
    //! \brief  It must be invoked when resolution changes, it will release
    //!         internal buffers on demand
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS OnSizeChange();

    //!
    //! \brief  Get Surface from given slot
    //! \param  [in]type
    //!         BufferType
    //! \param  [in]index
    //!         BufferSlot index
    //! \return MOS_SURFACE *
    //!         MOS_SURFACE * if success, else nullptr if the pool is empty
    //!
    MOS_SURFACE *GetSurface(BufferType type, uint32_t index);

    //!
    //! \brief  Get Buffer from given slot
    //! \param  [in]type
    //!         BufferType
    //! \param  [in]index
    //!         BufferSlot index
    //! \return MOS_RESOURCE *
    //!         MOS_RESOURCE * if success, else nullptr if the pool is empty
    //!
    virtual MOS_RESOURCE *GetBuffer(BufferType type, uint32_t index);

protected:
    //!
    //! \brief  Get Resource type according to the buffer type
    //! \param  [in]type
    //!         BufferType
    //! \return ResourceType
    //!         return the ResourceType
    //!
    ResourceType GetResourceType(BufferType buffer)
    {
        for (auto pair : m_mapBufferResourceType)
        {
            if (pair.buffer == buffer)
            {
                return pair.type;
            }
        }

        return ResourceType::invalidResource;
    }

    //!
    //! \brief  Reset the unused slots then can use them for other frames
    //! \param  [in]refList
    //!         the pointer of reference list
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS ReleaseUnusedSlots(CODEC_REF_LIST* refList, bool lazyRelease);

    friend class BufferSlot;
    //!
    //! \brief  Get the tracked buffer queue according to the buffer type
    //! \param  [in]type
    //!         BufferType
    //! \return shared_ptr<BufferQueue>
    //!         shared_ptr<BufferQueue> if success, else nullptr
    std::shared_ptr<BufferQueue> GetBufferQueue(BufferType type);

    static constexpr MapBufferResourceType m_mapBufferResourceType[] =
    {
        {BufferType::mbCodedBuffer,             ResourceType::bufferResource},
        {BufferType::mvDataBuffer,              ResourceType::bufferResource},
        {BufferType::mvTemporalBuffer,          ResourceType::bufferResource},
        {BufferType::ds4xSurface,               ResourceType::surfaceResource},
        {BufferType::ds8xSurface,               ResourceType::surfaceResource},
        {BufferType::segmentIdStreamOutBuffer,  ResourceType::bufferResource},
        {BufferType::vdencSegIdStreamOutBuffer, ResourceType::bufferResource},
        {BufferType::bwdAdaptCdfBuffer,         ResourceType::bufferResource},
        {BufferType::postCdefReconSurface,      ResourceType::surfaceResource},
        {BufferType::preMbCodedBuffer,          ResourceType::bufferResource},
        {BufferType::preRefSurface,             ResourceType::surfaceResource},
        {BufferType::preRawSurface,             ResourceType::surfaceResource},
        {BufferType::preDs4xSurface,            ResourceType::surfaceResource},
        {BufferType::preDs8xSurface,            ResourceType::surfaceResource},
        {BufferType::preencRef0,                ResourceType::bufferResource},
        {BufferType::preencRef1,                ResourceType::bufferResource},
        {BufferType::superResRefScaled,         ResourceType::surfaceResource},
        {BufferType::superResRef4xDsScaled,     ResourceType::surfaceResource},
        {BufferType::superResRef8xDsScaled,     ResourceType::surfaceResource},
        {BufferType::AlignedRawSurface,         ResourceType::surfaceResource},
    };

    uint8_t m_maxSlotCnt        = 0;     //!< max slot count in the tracked buffer
    uint8_t m_maxRefSlotCnt     = 0;     //!< max reference slot count in the tracked buffer
    uint8_t m_maxNonRefSlotCnt  = 0;     //!< max non-reference slot count int he tracked buffer
    uint8_t m_currSlotIndex     = 0;     //!< current free slot index

    PMOS_MUTEX                m_mutex;                //!< mutex
    Condition                 m_condition;            //!< condition
    EncodeAllocator *         m_allocator = nullptr;  //!< encoder allocator
    std::vector<BufferSlot *> m_bufferSlots = {};          //!< buffer slots

    std::map<BufferType, MOS_ALLOC_GFXRES_PARAMS>       m_allocParams = {};  //!< allocate parameters
    std::map<BufferType, std::shared_ptr<BufferQueue> > m_bufferQueue = {};  //!< buffer queues
    std::map<BufferType, std::shared_ptr<BufferQueue> > m_oldQueue = {};     //!< old queues for resolution change

MEDIA_CLASS_DEFINE_END(encode__TrackedBuffer)
};
}  // namespace encode
#endif  // !__ENCODE_BUFFER_TRACKER_H__
