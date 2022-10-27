/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     encode_vp9_vdenc_feature_manager.h
//! \brief    Defines the common interface for vp9 vdenc feature manager
//! \details  The encode feature manager is further sub-divided by platform type.
//!           This file is for the base interface which is shared by all platforms.
//!

#ifndef __ENCODE_VP9_VDENC_FEATURE_MANAGER_H__
#define __ENCODE_VP9_VDENC_FEATURE_MANAGER_H__
#include <vector>
#include "media_vp9_feature_defs.h"
#include "encode_feature_manager.h"
#include "encode_vp9_basic_feature.h"
#include "encode_vp9_tile.h"
#include "codec_hw_next.h"
#include "encode_recycle_resource.h"
#include "encode_tracked_buffer.h"

namespace encode
{

class EncodeVp9VdencFeatureManager : public EncodeFeatureManager
{
public:
    //!
    //! \brief  EncodeVp9VdencFeatureManager constructor
    //! \param  [in] allocator
    //!         Pointer to EncodeAllocator
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] trackedBuf
    //!         Pointer to TrackedBuffer
    //! \param  [in] recycleBuf
    //!         Pointer to RecycleResource
    //!
    EncodeVp9VdencFeatureManager(EncodeAllocator *allocator,
                            CodechalHwInterfaceNext *hwInterface,
                            TrackedBuffer *trackedBuf,
                            RecycleResource *recycleBuf):
                            m_allocator(allocator),
                            m_hwInterface(hwInterface),
                            m_recycleResource(recycleBuf),
                            m_trackedBuf(trackedBuf) {};

    //!
    //! \brief  EncodeVp9VdencFeatureManager destructor
    //!
    virtual ~EncodeVp9VdencFeatureManager() {}

    //!
    //! \brief  Check the conflict between features
    //! \param  [in] params
    //!         encode parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CheckFeatures(void *params) override;

    //!
    //! \brief  Set Pass Number
    //! \param  [in] passNum
    //!         Number of passes
    //! \return void
    //!
    void SetNumPass(uint8_t passNum) { m_passNum = passNum; };

protected:
    //!
    //! \brief  Create feature const settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateConstSettings() override;

    //!
    //! \brief  Create features
    //! \param  [in] constsettings
    //!         feature const settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateFeatures(void *constSettings) override;

    //!
    //! \brief  Map target usage
    //! \param  [in, out] targetUsage
    //!         Target usage
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS MapTargetUsage(uint8_t &targetUsage);

    //!
    //! \brief  Validate number of passes
    //! \param  [in] vp9SeqParams
    //!         sequence parameters from DDI
    //! \param  [in] vp9PictureParams
    //!         picture parameters from DDI
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ValidatePassNum(
        PCODEC_VP9_ENCODE_SEQUENCE_PARAMS vp9SeqParams,
        PCODEC_VP9_ENCODE_PIC_PARAMS  vp9PicParams);

    EncodeAllocator *m_allocator = nullptr;

    CodechalHwInterfaceNext *m_hwInterface     = nullptr;
    RecycleResource *    m_recycleResource = nullptr;
    TrackedBuffer *      m_trackedBuf      = nullptr;

MEDIA_CLASS_DEFINE_END(encode__EncodeVp9VdencFeatureManager)
};

}  // namespace encode
#endif  // !__ENCODE_HEVC_VDENC_FEATURE_MANAGER_H__
