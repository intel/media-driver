/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     decode_downsampling_feature_xe3_lpm_base.cpp
//! \brief    Defines the common interface for decode downsampling features
//! \details  The decode downsampling feature interface is further sub-divided by codec standard,
//!           this file is for the base interface which is shared by all codecs.

#include "decode_downsampling_feature_xe3_lpm_base.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{
    DecodeDownSamplingFeatureXe3LpmBase::DecodeDownSamplingFeatureXe3LpmBase(
        MediaFeatureManager *featureManager, DecodeAllocator *allocator, PMOS_INTERFACE osInterface) :
        DecodeDownSamplingFeature(featureManager, allocator, osInterface)
    {
        m_featureManager = featureManager;
    }

    DecodeDownSamplingFeatureXe3LpmBase::~DecodeDownSamplingFeatureXe3LpmBase()
    {
    }


}  // namespace decode

#endif  //_DECODE_PROCESSING_SUPPORTED