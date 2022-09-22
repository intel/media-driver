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
//! \file     decode_av1_feature_manager.cpp
//! \brief    Defines the common interface for av1 decode feature manager
//! \details  The av1 decode feature manager is further sub-divided by codec type
//!           this file is for the base interface which is shared by all components.
//!

#include "decode_vp9_feature_manager.h"
#include "decode_vp9_basic_feature.h"
#include "decode_vp9_downsampling_feature.h"
#include "decode_utils.h"

namespace decode
{
    MOS_STATUS DecodeVp9FeatureManager::CreateFeatures(void *constSettings)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(DecodeFeatureManager::CreateFeatures(constSettings));

        Vp9BasicFeature *decBasic = MOS_New(Vp9BasicFeature, m_allocator, m_hwInterface, m_osInterface);
        DECODE_CHK_STATUS(RegisterFeatures(FeatureIDs::basicFeature, decBasic));

#ifdef _DECODE_PROCESSING_SUPPORTED
        auto decDownSampling = MOS_New(Vp9DownSamplingFeature, this, m_allocator, m_osInterface);
        DECODE_CHK_STATUS(RegisterFeatures(DecodeFeatureIDs::decodeDownSampling, decDownSampling));
#endif

        return MOS_STATUS_SUCCESS;
    }
}
