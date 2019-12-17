/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     media_feature_manager.cpp
//! \brief    Defines the common interface for media feature manager
//! \details  The encode feature manager is further sub-divided by codec type
//!           this file is for the base interface which is shared by all components.
//!

#include "media_feature_manager.h"

MOS_STATUS MediaFeatureManager::Init(void *settings)
{
    MEDIA_FUNC_CALL();

    MEDIA_CHK_STATUS_RETURN(CreateConstSettigs());
    MEDIA_CHK_NULL_RETURN(m_featureConstSettings);
    MEDIA_CHK_STATUS_RETURN(m_featureConstSettings->PrepareConstSettings());

    MEDIA_CHK_STATUS_RETURN(CreateFeatures(m_featureConstSettings->GetConstSettings()));
    for (auto feature=m_features.begin(); feature!=m_features.end(); feature++)
    {
        MEDIA_CHK_STATUS_RETURN(feature->second->Init(settings));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaFeatureManager::RegisterFeatures(FeatureIDs featureID, MediaFeature *feature)
{
    MEDIA_FUNC_CALL();
    MEDIA_CHK_NULL_RETURN(feature);

    auto iter = m_features.find(featureID);
    if (iter == m_features.end())
    {
        m_features.insert(std::make_pair(featureID, feature));
    }
    else
    {
        MEDIA_NORMALMESSAGE("This feature already exist, will replay it with the new one!");
        if (iter->second != nullptr)
        {
            MOS_Delete(iter->second);
        };
        iter->second = feature;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaFeatureManager::Update(void *params)
{
    MEDIA_FUNC_CALL();
    for (auto feature=m_features.begin(); feature!=m_features.end(); feature++)
    {
        MEDIA_CHK_STATUS_RETURN(feature->second->Update(params));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaFeatureManager::Destroy()
{
    MEDIA_FUNC_CALL();
    for (auto feature=m_features.begin(); feature!=m_features.end(); feature++)
    {
        if (feature->second != nullptr)
        {
            MOS_Delete(feature->second);
        };
    }
    m_features.clear();

    if (m_featureConstSettings != nullptr)
    {
        MOS_Delete(m_featureConstSettings);
    }
    return MOS_STATUS_SUCCESS;
}
