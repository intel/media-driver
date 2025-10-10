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
//! \file     encode_vp9_basic_feature_xe3_lpm.h
//! \brief    Defines the Xe3_LPM+ common class for encode vp9 basic feature
//!
#ifndef __ENCODE_VP9_BASIC_FEATURE_XE3_LPM_H__
#define __ENCODE_VP9_BASIC_FEATURE_XE3_LPM_H__

#include "encode_vp9_basic_feature.h"

namespace encode
{
class Vp9BasicFeatureXe3_Lpm : public Vp9BasicFeature
{
public:
    Vp9BasicFeatureXe3_Lpm(EncodeAllocator          *allocator,
                     CodechalHwInterfaceNext        *hwInterface,
                     TrackedBuffer                  *trackedBuf,
                     RecycleResource                *recycleBuf,
                     void                           *constSettings) :
                     Vp9BasicFeature(allocator, hwInterface, trackedBuf, recycleBuf, constSettings){};

    virtual ~Vp9BasicFeatureXe3_Lpm(){};

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(VDENC_CMD2);

    MEDIA_CLASS_DEFINE_END(encode__Vp9BasicFeatureXe3_Lpm);
};

}  // namespace encode

#endif  // !__ENCODE_VP9_BASIC_FEATURE_XE3_LPM_H__
