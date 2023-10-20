/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_avc_vdenc_feature_manager.h
//! \brief    Defines the common interface for avc vdenc feature manager
//! \details  The encode feature manager is further sub-divided by codec type
//!           this file is for the base interface which is shared by all components.
//!
#ifndef __ENCODE_AVC_VDENC_FEATURE_MANAGER_H__
#define __ENCODE_AVC_VDENC_FEATURE_MANAGER_H__

#include "media_feature_manager.h"
#include "encode_feature_manager.h"
#include "codec_hw_next.h"
#include "encode_recycle_resource.h"
#include "encode_tracked_buffer.h"
#include "codec_def_encode_avc.h"
#include "media_copy_wrapper.h"

namespace encode
{

class EncodeAvcVdencFeatureManager : public EncodeFeatureManager
{
public:
    //!
    //! \brief  EncodeAvcVdencFeatureManager constructor
    //! \param  [in] allocator
    //!         Pointer to EncodeAllocator
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] trackedBuf
    //!         Pointer to TrackedBuffer
    //! \param  [in] recycleBuf
    //!         Pointer to RecycleResource
    //!
     EncodeAvcVdencFeatureManager(EncodeAllocator *allocator,
                                  CodechalHwInterfaceNext *hwInterface,
                                  TrackedBuffer       *trackedBuf,
                                  RecycleResource     *recycleBuf,
                                  MediaCopyWrapper    *mediaCopyWrapper) :
                                  m_allocator(allocator),
                                  m_hwInterface(hwInterface),
                                  m_recycleResource(recycleBuf),
                                  m_trackedBuf(trackedBuf),
                                  m_mediaCopyWrapper(mediaCopyWrapper) {}

    //!
    //! \brief  EncodeAvcVdencFeatureManager deconstructor
    //!
    virtual ~EncodeAvcVdencFeatureManager() {}

    //!
    //! \brief  Check the conflict between features
    //! \param  [in] params
    //!         encode parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CheckFeatures(void *params) override;

protected:
    //!
    //! \brief  Map target usage
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS MapTargetUsage(uint8_t &targetUsage);

protected:

    MOS_STATUS ValidatePassNum(
        PCODEC_AVC_ENCODE_SEQUENCE_PARAMS avcSeqParams,
        PCODEC_AVC_ENCODE_PIC_PARAMS avcPicParams);

    EncodeAllocator     *m_allocator = nullptr;
    CodechalHwInterfaceNext *m_hwInterface = nullptr;
    RecycleResource     *m_recycleResource = nullptr;
    TrackedBuffer       *m_trackedBuf = nullptr;
    MediaCopyWrapper    *m_mediaCopyWrapper = nullptr;

MEDIA_CLASS_DEFINE_END(encode__EncodeAvcVdencFeatureManager)
};

}
#endif // !__ENCODE_AVC_VDENC_FEATURE_MANAGER_H__
