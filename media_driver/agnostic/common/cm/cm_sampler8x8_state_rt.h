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
//! \file      cm_sampler8x8_state_rt.h 
//! \brief     Contains CmSampler8x8State_RT declarations. 
//!

#ifndef MEDIARIVER_AGNOSTIC_COMMON_CM_CMSAMPLER8X8STATERT_H_
#define MEDIARIVER_AGNOSTIC_COMMON_CM_CMSAMPLER8X8STATERT_H_

#include "cm_sampler8x8.h"

namespace CMRT_UMD
{
class SamplerIndex;

//*-----------------------------------------------------------------------------
//! CM Sampler8x8State
//*-----------------------------------------------------------------------------
class CmSampler8x8State_RT: public CmSampler8x8
{
public:
    CM_RT_API int32_t GetIndex(SamplerIndex* &index);

    static int32_t Create(const CM_SAMPLER_8X8_DESCR &sampleState,
                          unsigned int index,
                          CmSampler8x8State_RT* &sampler);

    static int32_t Destroy(CmSampler8x8State_RT* &sampler);

    //NOT depend on RTTI::dynamic_cast
    CM_ENUM_CLASS_TYPE Type() const
    { return CM_ENUM_CLASS_TYPE_CMSAMPLER8X8STATE_RT; }

    CM_SAMPLER_STATE_TYPE GetStateType() { return m_stateType; }

private:
    CmSampler8x8State_RT(const CM_SAMPLER_8X8_DESCR &sampleState);

    ~CmSampler8x8State_RT();

    CmSampler8x8State_RT& operator= (const CmSampler8x8State_RT &other);

    int32_t Initialize(unsigned int index);

    SamplerIndex *m_index;

    CM_AVS_STATE_MSG m_avsState;
    CM_CONVOLVE_STATE_MSG m_convolveState;
    CM_MISC_STATE_MSG m_miscState;

    CM_SAMPLER_STATE_TYPE m_stateType;
};
}; //namespace

#endif  // #ifndef #ifndef MEDIARIVER_AGNOSTIC_COMMON_CM_CMSAMPLER8X8STATERT_H_
