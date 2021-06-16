/*
* Copyright (c) 2019-2020, Intel Corporation
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
//! \file     decode_feature_manager.cpp
//! \brief    Defines the common interface for decode feature manager
//! \details  The decode feature manager is further sub-divided by codec type
//!           this file is for the base interface which is shared by all components.
//!

#include "decode_feature_manager.h"
#include "decode_basic_feature.h"
#include "decode_predication.h"
#include "decode_marker_packet.h"
#include "decode_utils.h"

namespace decode
{

MOS_STATUS DecodeFeatureManager::Init(void *settings)
{
    MEDIA_FUNC_CALL();

    MEDIA_CHK_STATUS_RETURN(CreateFeatures(settings));
    for (auto feature=m_features.begin(); feature!=m_features.end(); feature++)
    {
        MEDIA_CHK_STATUS_RETURN(feature->second->Init(settings));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeFeatureManager::CreateFeatures(void *codecSettings)
{
    DECODE_FUNC_CALL();

    DecodePredication* predication = MOS_New(DecodePredication, *m_allocator);
    DECODE_CHK_STATUS(RegisterFeatures(DecodeFeatureIDs::decodePredication, predication));

    DecodeMarker* marker = MOS_New(DecodeMarker, *m_allocator);
    DECODE_CHK_STATUS(RegisterFeatures(DecodeFeatureIDs::decodeMarker, marker));

    return MOS_STATUS_SUCCESS;
}

}
