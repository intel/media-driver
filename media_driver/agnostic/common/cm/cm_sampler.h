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
//! \file      cm_sampler.h
//! \brief     Contains CmSampler declarations.
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMSAMPLER_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMSAMPLER_H_

#include "cm_def.h"

namespace CMRT_UMD
{
//!
//!
//! \brief      Abstraction of 3D sampler intrinsics.
//! \details    A CmSampler object is used to specify sampling operations to be
//!             performed by the 3D sampler kernel intrinsics.
//!
class CmSampler
{
public:
    //!
    //! \brief      Retrieves pointer to the SamplerIndex associated with this
    //!             CmSampler.
    //! \details    Each CmSampler is associated with a SamplerIndex containing a
    //!             unique value to which the CmSampler is mapped to.
    //!             CmDevice keeps the mapping between index value and CmSampler.
    //!             SamplerIndex is passed to CM kernel function as an argument to
    //!             indicate this CmSampler.
    //! \param      [out] index
    //!             Reference to SamplerIndex pointer. It'll point to the
    //!             SamplerIndex associated with this CmSampler.
    //! \retval     CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetIndex(SamplerIndex* &index) = 0;
};
};//namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMSAMPLER_H_
