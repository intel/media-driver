/*
* Copyright (c) 2019, Intel Corporation
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
//! \file    mos_gpucontext_specific_next_ext.cpp
//! \brief   Container class for the Linux specific gpu context
#include "mos_gpucontext_specific_next.h"

void GpuContextSpecificNext::SetEngineQueryFlags(
    PMOS_GPUCTX_CREATOPTIONS option,
    __u64 &caps)
{
    PMOS_GPUCTX_CREATOPTIONS_ENHANCED createOptionEnhanced = nullptr;

    if (typeid(*option) == typeid(MOS_GPUCTX_CREATOPTIONS_ENHANCED))
    {
        createOptionEnhanced = static_cast<PMOS_GPUCTX_CREATOPTIONS_ENHANCED>(option);
    }
    else
    {
        return;
    }

    if (createOptionEnhanced->UsingSFC)
    {
        caps |= I915_VIDEO_AND_ENHANCE_CLASS_CAPABILITY_SFC;
    }
}