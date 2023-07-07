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
//! \file     encode_av1_basic_feature_xe_hpm.h
//! \brief    Defines the common interface for encode av1 basic feature
//!
#ifndef __ENCODE_AV1_BASIC_FEATURE_XE_HPM_H__
#define __ENCODE_AV1_BASIC_FEATURE_XE_HPM_H__

#include "encode_av1_basic_feature.h"

namespace encode
{
class Av1BasicFeatureXe_Hpm : public Av1BasicFeature
{
public:
    Av1BasicFeatureXe_Hpm(EncodeAllocator* allocator,
        CodechalHwInterfaceNext* hwInterface,
        TrackedBuffer* trackedBuf,
        RecycleResource* recycleBuf,
        void* constSettings) :
        Av1BasicFeature(allocator, hwInterface, trackedBuf, recycleBuf, constSettings) {};

    virtual MOS_STATUS Update(void *params) override;

    virtual ~Av1BasicFeatureXe_Hpm() {};

    MHW_SETPAR_DECL_HDR(AVP_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(VDENC_HEVC_VP9_TILE_SLICE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE); 

MEDIA_CLASS_DEFINE_END(encode__Av1BasicFeatureXe_Hpm)
};

}  // namespace encode

#endif  // !__ENCODE_AV1_BASIC_FEATURE_XE_HPM_H__