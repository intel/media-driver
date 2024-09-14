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
//! \file     encode_hevc_vdenc_feature_manager_xe2_lpm_base.h
//! \brief    Defines the Xe2_LPM+ interface for encode feature manager
//! \details  The encode feature manager is further sub-divided by codec type
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __ENCODE_HEVC_VDENC_FEATURE_MANAGER_XE2_LPM_BASE_H__
#define __ENCODE_HEVC_VDENC_FEATURE_MANAGER_XE2_LPM_BASE_H__
#include <vector>
#include "encode_hevc_vdenc_feature_manager.h"
#include "encode_hevc_vdenc_const_settings_xe2_lpm_base.h"
#include "encode_hevc_vdenc_weighted_prediction.h"
#include "encode_hevc_vdenc_scc_xe2_lpm_base.h"
#include "encode_hevc_vdenc_preenc.h"
#include "encode_hevc_vdenc_fullenc.h"

namespace encode
{

class EncodeHevcVdencFeatureManagerXe2_Lpm_Base : public EncodeHevcVdencFeatureManager
{
public:
    //!
    //! \brief  EncodeHevcVdencFeatureManagerXe2_Lpm_Base constructor
    //! \param  [in] allocator
    //!         Pointer to EncodeAllocator
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] trackedBuf
    //!         Pointer to TrackedBuffer
    //! \param  [in] recycleBuf
    //!         Pointer to RecycleResource
    //!
    EncodeHevcVdencFeatureManagerXe2_Lpm_Base(EncodeAllocator *allocator,
                             CodechalHwInterfaceNext *hwInterface,
                             TrackedBuffer       *trackedBuf,
                             RecycleResource     *recycleBuf):
                             EncodeHevcVdencFeatureManager(allocator, hwInterface, trackedBuf, recycleBuf){};

    //!
    //! \brief  EncodeHevcVdencFeatureManagerXe2_Lpm_Base deconstructor
    //!
    virtual ~EncodeHevcVdencFeatureManagerXe2_Lpm_Base(){};

protected:
    //!
    //! \brief  Create feature const settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CreateConstSettings() override;

    //!
    //! \brief  Create features
    //! \param  [in] constsettings
    //!         feature const settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CreateFeatures(void *constSettings) override;

    //!
    //! \brief  Map target usage
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS MapTargetUsage(uint8_t &targetUsage) override;

    //!
    //! \brief  Check Platform Capabilities
    //! \param  [in] hevcSeqParams
    //!         sequese parameters from DDI
    //! \param  [in] hevcPictureParams
    //!         picture parameters from DDI
    //! \param  [in] hevcSliceParams
    //!         slice parameters from DDI
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CheckPlatformCapability(
        PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams,
        PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSliceParams) override;

MEDIA_CLASS_DEFINE_END(encode__EncodeHevcVdencFeatureManagerXe2_Lpm_Base)
};


}
#endif // !__ENCODE_HEVC_VDENC_FEATURE_MANAGER_XE2_LPM_BASE_H__

