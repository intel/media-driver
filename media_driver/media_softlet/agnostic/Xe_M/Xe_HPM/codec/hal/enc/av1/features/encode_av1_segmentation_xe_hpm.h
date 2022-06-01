/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     encode_av1_segmentation_xe_hpm.h
//! \brief    Defines the common interface for encode av1 segmentation feature
//!
#ifndef __ENCODE_AV1_SEGMENTATION_XE_HPM_H__
#define __ENCODE_AV1_SEGMENTATION_XE_HPM_H__

#include "encode_av1_segmentation.h"

namespace encode
{

class Av1SegmentationXe_Hpm : public Av1Segmentation
{
public:
    Av1SegmentationXe_Hpm(
        MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        void *constSettings);

    virtual ~Av1SegmentationXe_Hpm();

    MHW_SETPAR_DECL_HDR(VDENC_CMD2);
MEDIA_CLASS_DEFINE_END(encode__Av1SegmentationXe_Hpm)
};

}  // namespace encode

#endif  // !__ENCODE_AV1_SEGMENTATION_XE_HPM_H__
