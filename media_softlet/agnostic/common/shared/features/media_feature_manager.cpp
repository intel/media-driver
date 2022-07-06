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
#include "media_feature.h"
#include "mos_utilities.h"

MOS_STATUS MediaFeatureManager::RegisterFeatures(
    int                featureID,
    MediaFeature *     feature,
    std::vector<int> &&packetIds,
    LIST_TYPE          packetIdListType)
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
    m_packetIdList[featureID]      = std::move(packetIds);
    m_packetIdListTypes[featureID] = packetIdListType;

    return MOS_STATUS_SUCCESS;
}

std::shared_ptr<MediaFeatureManager::ManagerLite> MediaFeatureManager::GetPacketLevelFeatureManager(int packetId)
{
    MEDIA_FUNC_CALL();

    auto manager = std::make_shared<ManagerLite>();

    for (const auto &e : m_features)
    {
        const auto &packetIds = m_packetIdList.at(e.first);
        bool        blockList = m_packetIdListTypes.at(e.first) == LIST_TYPE::BLOCK_LIST;

        // Default value if packetId is not in feature's packet ID vector.
        // For block list, feature is by default allowed if it's not in the list.
        // For allow list, feature is by default blocked if it's not in the list.
        bool blocked = !blockList;

        for (auto id : packetIds)
        {
            if (blockList && id == packetId)
            {
                blocked = true;
                break;
            }
            else if (!blockList && id == packetId)
            {
                blocked = false;
                break;
            }
        }

        if (!blocked)
        {
            manager->m_features[e.first] = e.second;
        }
    }

    return manager;
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
