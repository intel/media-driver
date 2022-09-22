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
//! \file     decode_vp9_basic_feature_m12.h
//!

#ifndef __DECODE_VP9_BASIC_FEATURE_M12_H__
#define __DECODE_VP9_BASIC_FEATURE_M12_H__

#include "decode_vp9_basic_feature.h"

namespace decode
{
class Vp9BasicFeatureM12 : public Vp9BasicFeature
{
public:    

    Vp9BasicFeatureM12(DecodeAllocator *allocator, void *hwInterface, PMOS_INTERFACE osInterface);

    virtual ~Vp9BasicFeatureM12() {}

protected:
    
    MhwVdboxHcpInterface *m_hcpInterface = nullptr;



MEDIA_CLASS_DEFINE_END(decode__Vp9BasicFeatureM12)
};

}  // namespace decode
#endif