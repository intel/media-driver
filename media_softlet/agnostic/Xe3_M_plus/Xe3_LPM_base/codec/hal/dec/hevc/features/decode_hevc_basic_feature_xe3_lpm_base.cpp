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
//! \file     decode_hevc_basic_feature_xe3_lpm_base.cpp
//! \brief    Defines the common interface for decode hevc parameter
//!

#include "decode_hevc_basic_feature_xe3_lpm_base.h"

namespace decode
{
HevcBasicFeatureXe3_Lpm_Base::HevcBasicFeatureXe3_Lpm_Base(DecodeAllocator *allocator, void *hwInterface, PMOS_INTERFACE osInterface) : 
    HevcBasicFeature(allocator, hwInterface, osInterface)
{
    DECODE_FUNC_CALL();
    if (hwInterface != nullptr)
    {
        m_osInterface = osInterface;
    }
}
MOS_STATUS HevcBasicFeatureXe3_Lpm_Base::CreateReferenceBeforeLoopFilter()
{
    if (m_destSurface.dwWidth == 0 ||
        m_destSurface.dwHeight == 0)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_referenceBeforeLoopFilter == nullptr)
    {
        m_referenceBeforeLoopFilter = m_allocator->AllocateSurface(
            m_destSurface.dwWidth, m_destSurface.dwHeight, "Reference before loop filter", m_destSurface.Format, false, resourceOutputPicture, notLockableVideoMem);
        DECODE_CHK_NULL(m_referenceBeforeLoopFilter);
    }
    else
    {
        DECODE_CHK_STATUS(m_allocator->Resize(m_referenceBeforeLoopFilter, m_destSurface.dwWidth, m_destSurface.dwHeight, notLockableVideoMem, false, "Reference before loop filter"));
    }
    return MOS_STATUS_SUCCESS;
}

}  // namespace decode