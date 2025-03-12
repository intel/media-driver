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
//! \file     decode_avc_basic_feature_xe3_lpm_base.h
//! \brief    Defines the common interface for decode avc basic feature
//!
#ifndef __DECODE_AVC_BASIC_FEATURE_XE3_LPM_BASE_H__
#define __DECODE_AVC_BASIC_FEATURE_XE3_LPM_BASE_H__

#include "decode_avc_basic_feature.h"

namespace decode
{
class AvcBasicFeatureXe3_Lpm_Base: public AvcBasicFeature
{
public:
    //!
    //! \brief  AvcBasicFeatureXe3_Lpm_Base constructor
    //!
    AvcBasicFeatureXe3_Lpm_Base(DecodeAllocator *allocator, void *hwInterface, PMOS_INTERFACE osInterface) :
        AvcBasicFeature(allocator, hwInterface, osInterface)
    {
    }

    //!
    //! \brief  AvcBasicFeatureXe3_Lpm_Base deconstructor
    //!
    virtual ~AvcBasicFeatureXe3_Lpm_Base() {}

protected:
    virtual MOS_STATUS CheckBitDepthAndChromaSampling() override;

MEDIA_CLASS_DEFINE_END(decode__AvcBasicFeatureXe3_Lpm_Base)
};

}  // namespace decode

#endif  // !__DECODE_AVC_BASIC_FEATURE_XE3_LPM_BASE_H__
