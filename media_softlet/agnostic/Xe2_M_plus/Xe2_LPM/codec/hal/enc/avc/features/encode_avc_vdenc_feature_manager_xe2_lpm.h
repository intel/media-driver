/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     encode_avc_vdenc_feature_manager_xe2_lpm.h
//! \brief    Defines the common interface for avc Xe2_LPM_base vdenc feature manager
//!
#ifndef __ENCODE_AVC_VDENC_FEATURE_MANAGER_XE2_LPM_H__
#define __ENCODE_AVC_VDENC_FEATURE_MANAGER_XE2_LPM_H__

#include "encode_avc_vdenc_feature_manager.h"

namespace encode
{

class EncodeAvcVdencFeatureManagerXe2_Lpm : public EncodeAvcVdencFeatureManager
{
public:

    EncodeAvcVdencFeatureManagerXe2_Lpm(
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        TrackedBuffer       *trackedBuf,
        RecycleResource     *recycleBuf,
        MediaCopyWrapper    *mediaCopyWrapper) :
        EncodeAvcVdencFeatureManager(allocator, hwInterface, trackedBuf, recycleBuf, mediaCopyWrapper) {}

    virtual ~EncodeAvcVdencFeatureManagerXe2_Lpm() {}

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
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS MapTargetUsage(uint8_t &targetUsage) override;

MEDIA_CLASS_DEFINE_END(encode__EncodeAvcVdencFeatureManagerXe2_Lpm)
};

}
#endif // !__ENCODE_AVC_VDENC_FEATURE_MANAGER_XE2_LPM_H__
