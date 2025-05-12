/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     encode_avc_vdenc_stream_in_feature_xe3_lpm.h
//! \brief    Defines the feature interface for encode AVC VDEnc Stream-in on xe3_lpm
//!
#ifndef __ENCODE_AVC_VDENC_STREAM_IN_FEATURE_XE3_LPM_H__
#define __ENCODE_AVC_VDENC_STREAM_IN_FEATURE_XE3_LPM_H__

#include "encode_avc_vdenc_stream_in_feature.h"

namespace encode
{

class AvcVdencStreamInFeatureXe3_Lpm : public AvcVdencStreamInFeature
{
public:
    //!
    //! \brief  AvcVDEncStreamInXe3_Lpm constructor
    //!
    AvcVdencStreamInFeatureXe3_Lpm(
        MediaFeatureManager     *featureManager,
        EncodeAllocator         *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void                    *constSettings);
    //!
    //! \brief  AvcVDEncStreamInXe3_Lpm destructor
    //!
    virtual ~AvcVdencStreamInFeatureXe3_Lpm()
    {
        ENCODE_FUNC_CALL();
    }

    MHW_SETPAR_DECL_HDR(VDENC_AVC_IMG_STATE);

MEDIA_CLASS_DEFINE_END(encode__AvcVdencStreamInFeatureXe3_Lpm)
};

}  // namespace encode

#endif  // !__ENCODE_AVC_VDENC_STREAM_IN_FEATURE_XE3_LPM_H__
