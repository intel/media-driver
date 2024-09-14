/*
* Copyright (c) 2021 - 2024, Intel Corporation
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
//! \file     encode_avc_basic_feature_xe2_lpm.h
//! \brief    Defines the common interface for encode avc Xe2_LPM basic feature
//!
#ifndef __ENCODE_AVC_BASIC_FEATURE_XE2_LPM_H__
#define __ENCODE_AVC_BASIC_FEATURE_XE2_LPM_H__

#include "encode_avc_basic_feature.h"

namespace encode
{

class AvcBasicFeatureXe2_Lpm : public AvcBasicFeature
{
public:
    AvcBasicFeatureXe2_Lpm(EncodeAllocator * allocator,
                                    CodechalHwInterfaceNext *hwInterface,
                                    TrackedBuffer *trackedBuf,
                                    RecycleResource *recycleBuf,
                                    MediaCopyWrapper *mediaCopyWrapper,
                                    void *constSettings = nullptr) :
                                    AvcBasicFeature(allocator, hwInterface, trackedBuf, recycleBuf, mediaCopyWrapper, constSettings) {}

    virtual ~AvcBasicFeatureXe2_Lpm() {}

    virtual MOS_STATUS Update(void *params) override;

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

MEDIA_CLASS_DEFINE_END(encode__AvcBasicFeatureXe2_Lpm)
};

}  // namespace encode

#endif  // !__ENCODE_AVC_BASIC_FEATURE_XE2_LPM_H__
