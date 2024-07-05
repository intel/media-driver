/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     decode_av1_basic_feature_xe2_lpm_base.cpp
//! \brief    Defines the common interface for decode av1 parameter
//!

#include "decode_av1_basic_feature_xe2_lpm_base.h"

namespace decode
{
    Av1BasicFeatureXe2_Lpm_Base::Av1BasicFeatureXe2_Lpm_Base(DecodeAllocator* allocator,
        void* hwInterface, PMOS_INTERFACE osInterface) :
        Av1BasicFeature(allocator, hwInterface, osInterface)
    {
        DECODE_FUNC_CALL()
        if (hwInterface != nullptr)
        {
            m_avpItf      = static_cast<CodechalHwInterfaceXe2_Lpm_Base *>(hwInterface)->GetAvpInterfaceNext();
            m_osInterface = osInterface;
        }
    }
}  // namespace decode
