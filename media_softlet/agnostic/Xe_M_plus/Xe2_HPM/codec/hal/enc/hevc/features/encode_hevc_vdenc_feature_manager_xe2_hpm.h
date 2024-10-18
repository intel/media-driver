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
//! \file     encode_hevc_vdenc_feature_manager_xe2_hpm.h
//! \brief    Defines the xe2 hpm interface for encode feature manager
//! \details  The encode feature manager is further sub-divided by codec type
//!

#ifndef __ENCODE_HEVC_VDENC_FEATURE_MANAGER_XE2_HPM_H__
#define __ENCODE_HEVC_VDENC_FEATURE_MANAGER_XE2_HPM_H__

#include "encode_hevc_vdenc_feature_manager_xe_lpm_plus_base.h"
#include "encode_hevc_vdenc_const_settings_xe2_hpm.h"

namespace encode
{

class EncodeHevcVdencFeatureManagerXe2_Hpm : public EncodeHevcVdencFeatureManagerXe_Lpm_Plus_Base
{
public:
    //!
    //! \brief  EncodeHevcVdencFeatureManagerXe2_Hpm constructor
    //! \param  [in] allocator
    //!         Pointer to EncodeAllocator
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] trackedBuf
    //!         Pointer to TrackedBuffer
    //! \param  [in] recycleBuf
    //!         Pointer to RecycleResource
    //!
    EncodeHevcVdencFeatureManagerXe2_Hpm(EncodeAllocator *allocator,
                             CodechalHwInterfaceNext *hwInterface,
                             TrackedBuffer       *trackedBuf,
                             RecycleResource     *recycleBuf):
                             EncodeHevcVdencFeatureManagerXe_Lpm_Plus_Base(allocator, hwInterface, trackedBuf, recycleBuf){};

    //!
    //! \brief  EncodeHevcVdencFeatureManagerXe2_Hpm deconstructor
    //!
    virtual ~EncodeHevcVdencFeatureManagerXe2_Hpm(){};

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

MEDIA_CLASS_DEFINE_END(encode__EncodeHevcVdencFeatureManagerXe2_Hpm)
};

}
#endif // !__ENCODE_HEVC_VDENC_FEATURE_MANAGER_XE2_HPM_H__
