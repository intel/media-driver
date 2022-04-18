/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     Encode_feature_manager.cpp
//! \brief    Defines the common interface for encode feature manager
//! \details  The encode feature manager is further sub-divided by codec type
//!           this file is for the base interface which is shared by all components.
//!

#include "encode_feature_manager.h"
#include <map>
#include <utility>
#include "encode_utils.h"
#include "media_feature.h"
#include "media_feature_const_settings.h" 

namespace encode
{

MOS_STATUS EncodeFeatureManager::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(CreateConstSettings());
    ENCODE_CHK_NULL_RETURN(m_featureConstSettings);
    ENCODE_CHK_STATUS_RETURN(m_featureConstSettings->PrepareConstSettings());

    ENCODE_CHK_STATUS_RETURN(CreateFeatures(m_featureConstSettings->GetConstSettings()));
    for (auto feature=m_features.begin(); feature!=m_features.end(); feature++)
    {
        ENCODE_CHK_STATUS_RETURN(feature->second->Init(settings));
    }
    return MOS_STATUS_SUCCESS;
}

}
