/*
* Copyright (c) 2017, Intel Corporation
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
//! \file      cm_sampler_rt.h
//! \brief     Contains CmSamplerRT declaration.
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMSAMPLERRT_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMSAMPLERRT_H_

#include "cm_sampler.h"

namespace CMRT_UMD
{
class CmSamplerRT: public CmSampler
{
public:
    static int32_t Create(unsigned int index, CmSamplerRT* &sampler);

    static int32_t Destroy(CmSamplerRT* &sampler);

    CM_RT_API int32_t GetIndex(SamplerIndex* &index);

    CM_ENUM_CLASS_TYPE Type() const { return CM_ENUM_CLASS_TYPE_CMSAMPLER_RT; }

protected:
    CmSamplerRT();

    ~CmSamplerRT();

    int32_t Initialize(unsigned int index);

    SamplerIndex *m_index;

private:
    CmSamplerRT(const CmSamplerRT &other);

    CmSamplerRT& operator=(const CmSamplerRT &other);
};
};//namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMSAMPLERRT_H_
