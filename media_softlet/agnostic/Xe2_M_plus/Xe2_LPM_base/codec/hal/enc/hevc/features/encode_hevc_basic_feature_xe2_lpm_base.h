/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     encode_hevc_basic_feature_xe2_lpm_base.h
//! \brief    Defines the common interface for encode hevc Xe2_LPM basic feature
//!
#ifndef __ENCODE_HEVC_BASIC_FEATURE_XE2_LPM_BASE_H__
#define __ENCODE_HEVC_BASIC_FEATURE_XE2_LPM_BASE_H__

#include "encode_hevc_basic_feature.h"
#include "encode_allocator.h"

namespace encode
{

class HevcBasicFeatureXe2_Lpm_Base : public HevcBasicFeature
{
public:
    HevcBasicFeatureXe2_Lpm_Base(EncodeAllocator *allocator,
                                    CodechalHwInterfaceNext *hwInterface,
                                    TrackedBuffer *trackedBuf,
                                    RecycleResource *recycleBuf,
                                    void *constSettings = nullptr) :
                                    HevcBasicFeature(allocator, hwInterface, trackedBuf, recycleBuf, constSettings) {}

    virtual ~HevcBasicFeatureXe2_Lpm_Base() {}

    virtual MOS_STATUS Update(void *params) override;

MEDIA_CLASS_DEFINE_END(encode__HevcBasicFeatureXe2_Lpm_Base)
};

}  // namespace encode

#endif  // !__ENCODE_HEVC_BASIC_FEATURE_XE2_LPM_BASE_H__
