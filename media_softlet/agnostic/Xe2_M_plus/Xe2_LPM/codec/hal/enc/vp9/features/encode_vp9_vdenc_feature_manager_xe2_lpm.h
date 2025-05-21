/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     encode_vp9_vdenc_feature_manager_xe2_lpm.h
//! \brief    Defines the common interface for vp9 vdenc feature manager
//!

#ifndef __ENCODE_VP9_VDENC_FEATURE_MANAGER_XE2_LPM_H__
#define __ENCODE_VP9_VDENC_FEATURE_MANAGER_XE2_LPM_H__
#include <vector>
#include "encode_vp9_vdenc_feature_manager.h"
#include "encode_vp9_vdenc_const_settings_xe2_lpm.h"

namespace encode
{
class EncodeVp9VdencFeatureManagerXe2_Lpm : public EncodeVp9VdencFeatureManager
{
public:
    //!
    //! \brief  EncodeVp9VdencFeatureManagerXe2_Lpm constructor
    //! \param  [in] allocator
    //!         Pointer to EncodeAllocator
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] trackedBuf
    //!         Pointer to TrackedBuffer
    //! \param  [in] recycleBuf
    //!         Pointer to RecycleResource
    //!
    EncodeVp9VdencFeatureManagerXe2_Lpm(EncodeAllocator *allocator,
                                CodechalHwInterfaceNext *hwInterface,
                                TrackedBuffer *trackedBuf,
                                RecycleResource *recycleBuf):
                                EncodeVp9VdencFeatureManager(allocator, hwInterface, trackedBuf, recycleBuf){};

    //!
    //! \brief  EncodeVp9VdencFeatureManagerXe2_Lpm deconstructor
    //!
    virtual ~EncodeVp9VdencFeatureManagerXe2_Lpm(){};

protected:
    //!
    //! \brief  Create feature const settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CreateConstSettings();

MEDIA_CLASS_DEFINE_END(encode__EncodeVp9VdencFeatureManagerXe2_Lpm)
};

}  // namespace encode
#endif  // !__ENCODE_VP9_VDENC_FEATURE_MANAGER_XE2_LPM_H__