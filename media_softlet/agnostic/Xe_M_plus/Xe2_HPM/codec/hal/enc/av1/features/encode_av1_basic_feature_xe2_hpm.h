/*
* Copyright (c) 2021 - 2023, Intel Corporation
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
//! \file     encode_av1_basic_feature_xe2_hpm.h
//! \brief    Defines the Xe2_HPM class for encode av1 basic feature
//!
#ifndef __ENCODE_AV1_BASIC_FEATURE_XE2_HPM_H__
#define __ENCODE_AV1_BASIC_FEATURE_XE2_HPM_H__

#include "encode_av1_basic_feature_xe_lpm_plus_base.h"

namespace encode
{
class Av1BasicFeatureXe2_Hpm : public Av1BasicFeatureXe_Lpm_Plus_Base
{
public:
    Av1BasicFeatureXe2_Hpm(MediaFeatureManager *featureManager,
                     EncodeAllocator           *allocator,
                     CodechalHwInterfaceNext   *hwInterface,
                     TrackedBuffer             *trackedBuf,
                     RecycleResource           *recycleBuf,
                     void                      *constSettings) :
                     Av1BasicFeatureXe_Lpm_Plus_Base(featureManager, allocator, hwInterface, trackedBuf, recycleBuf, constSettings){};

    virtual ~Av1BasicFeatureXe2_Hpm(){};

    virtual MOS_STATUS Update(void *params) override;

MEDIA_CLASS_DEFINE_END(encode__Av1BasicFeatureXe2_Hpm)
};

}  // namespace encode

#endif  // !__ENCODE_AV1_BASIC_FEATURE_XE2_HPM_H__
