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
//! \file     encode_hevc_vdenc_feature_manager_xe_lpm_plus.h
//! \brief    Defines the Xe_Lpm_Plus interface for encode feature manager
//! \details  The encode feature manager is further sub-divided by codec type
//!

#ifndef __ENCODE_HEVC_VDENC_FEATURE_MANAGER_XE_LPM_PLUS_H__
#define __ENCODE_HEVC_VDENC_FEATURE_MANAGER_XE_LPM_PLUS_H__

#include "encode_hevc_vdenc_feature_manager_xe_lpm_plus_base.h"

namespace encode
{

class EncodeHevcVdencFeatureManagerXe_Lpm_Plus : public EncodeHevcVdencFeatureManagerXe_Lpm_Plus_Base
{
public:
    //!
    //! \brief  EncodeHevcVdencFeatureManagerXe_Lpm_Plus constructor
    //! \param  [in] allocator
    //!         Pointer to EncodeAllocator
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] trackedBuf
    //!         Pointer to TrackedBuffer
    //! \param  [in] recycleBuf
    //!         Pointer to RecycleResource
    //!
    EncodeHevcVdencFeatureManagerXe_Lpm_Plus(EncodeAllocator *allocator,
                             CodechalHwInterfaceNext *hwInterface,
                             TrackedBuffer       *trackedBuf,
                             RecycleResource     *recycleBuf):
                             EncodeHevcVdencFeatureManagerXe_Lpm_Plus_Base(allocator, hwInterface, trackedBuf, recycleBuf){};

    //!
    //! \brief  EncodeHevcVdencFeatureManagerXe_Lpm_Plus deconstructor
    //!
    virtual ~EncodeHevcVdencFeatureManagerXe_Lpm_Plus(){};

protected:
    //! \brief  Check the features
    //! \param  [in] params
    //!         encode parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CheckFeatures(void *params) override;

MEDIA_CLASS_DEFINE_END(encode__EncodeHevcVdencFeatureManagerXe_Lpm_Plus)
};

}
#endif // !__ENCODE_HEVC_VDENC_FEATURE_MANAGER_XE_LPM_PLUS_H__
