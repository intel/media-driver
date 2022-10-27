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
//! \file     encode_jpeg_feature_manager.cpp
//! \brief    Defines the common interface for jpeg feature manager
//! \details  The encode feature manager is further sub-divided by codec type
//!           this file is for the base interface which is shared by all components.
//!

#include "encode_jpeg_feature_manager.h"
#include "encode_jpeg_basic_feature.h"
#include "encode_jpeg_packer_feature.h"
#include "media_jpeg_feature_defs.h"

namespace encode
{

MOS_STATUS EncodeJpegFeatureManager::CreateConstSettings()
{
    ENCODE_FUNC_CALL();
    m_featureConstSettings = MOS_New(MediaFeatureConstSettings);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeJpegFeatureManager::CreateFeatures(void *constSettings)
{
    ENCODE_FUNC_CALL();
    EncodeBasicFeature *encBasic = MOS_New(JpegBasicFeature, m_allocator, m_hwInterface, m_trackedBuf, m_recycleResource);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(FeatureIDs::basicFeature, encBasic));

    JpegPackerFeature *jpgPacker = MOS_New(JpegPackerFeature, this, nullptr, m_hwInterface, nullptr);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(JpegFeatureIDs::jpegPackerFeature, jpgPacker));

    return MOS_STATUS_SUCCESS;
}

}
