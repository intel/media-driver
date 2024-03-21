/*
* Copyright (c) 2018 - 2020, Intel Corporation
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
//! \file     media_feature_manager.h
//! \brief    Defines the common interface for encode feature manager
//! \details  The media feature manager is further sub-divided by codec type
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __MEDIA_FEATURE_MANAGER_H__
#define __MEDIA_FEATURE_MANAGER_H__
#include <vector>
#include <stdint.h>
#include <map>
#include <memory>
#include <utility>
#include "media_user_setting.h"
#include "media_utils.h"
#include "mos_defs.h"
#include "media_feature_const_settings.h"

#define CONSTRUCTFEATUREID(_componentID, _subComponentID, _featureID) \
    (_componentID << 24 | _subComponentID << 16 | _featureID)

enum ComponentIDs
{
    FEATURE_COMPONENT_COMMON = 0,
    FEATURE_COMPONENT_ENCODE,
    FEATURE_COMPONENT_DECODE,
    FEATURE_COMPONENT_VP,
    FEATURE_COMPONENT_OTHER
};

enum SubComponentIDs
{
    FEATURE_SUBCOMPONENT_COMMON = 0,
    FEATURE_SUBCOMPONENT_HEVC,
    FEATURE_SUBCOMPONENT_VP9,
    FEATURE_SUBCOMPONENT_AVC,
    FEATURE_SUBCOMPONENT_AV1,
    FEATURE_SUBCOMPONENT_JPEG
};

struct FeatureIDs
{
    enum CommonFeatureIDs
    {
        basicFeature = CONSTRUCTFEATUREID(FEATURE_COMPONENT_COMMON, FEATURE_SUBCOMPONENT_COMMON, 0),
        encodeTile,
        preEncFeature,
    };
};
//!
//! \def RUN_FEATURE_INTERFACE_RETURN(_featureClassName, _featureID, _featureInterface, ...)
//!  Run _featureInterface if it exit
//!
#define RUN_FEATURE_INTERFACE_RETURN(_featureClassName, _featureID, _featureInterface, ...)                \
{                                                                                                   \
    if(m_featureManager){                                                                           \
        auto feature = static_cast<_featureClassName*>(m_featureManager->GetFeature(_featureID));   \
        if (feature)                                                                                \
        {                                                                                           \
            MEDIA_CHK_STATUS_RETURN(feature->_featureInterface(__VA_ARGS__));                       \
        }                                                                                           \
    }                                                                                               \
}

//!
//! \def RUN_FEATURE_INTERFACE_NO_RETURN(_featureClassName, _featureID, _featureInterface, ...)
//!  Run _featureInterface if it exit
//!
#define RUN_FEATURE_INTERFACE_NO_RETURN(_featureClassName, _featureID, _featureInterface, ...)                \
{                                                                                                   \
    if(m_featureManager){                                                                           \
        auto feature = static_cast<_featureClassName*>(m_featureManager->GetFeature(_featureID));   \
        if (feature)                                                                                \
        {                                                                                           \
            feature->_featureInterface(__VA_ARGS__);                              \
        }                                                                                           \
    }                                                                                               \
}

class MediaFeature;

enum class LIST_TYPE
{
    BLOCK_LIST,
    ALLOW_LIST,
};

class MediaFeatureManager  // for pipe line use
{
protected:
    using container_t = std::map<int, MediaFeature *>;

public:
    class ManagerLite final  // for packet use
    {
        friend class MediaFeatureManager;

    public:
        class iterator : public container_t::iterator
        {
        public:
            explicit iterator(container_t::iterator it) : container_t::iterator(it) {}

            container_t::mapped_type operator*() { return (*this)->second; }
        };

        ManagerLite() = default;

        iterator begin() { return iterator(m_features.begin()); }

        iterator end() { return iterator(m_features.end()); }

        MediaFeature *GetFeature(int featureID)
        {
            auto iter = m_features.find(featureID);
            if (iter == m_features.end())
            {
                return nullptr;
            }
            return iter->second;
        }

    private:
        container_t m_features;
    };

public:
    class iterator : public container_t::iterator
    {
    public:
        explicit iterator(container_t::iterator it) : container_t::iterator(it) {}

        container_t::mapped_type operator*() { return (*this)->second; }
    };

    //!
    //! \brief  MediaFeatureManager constructor
    //!
    MediaFeatureManager(){};

    //!
    //! \brief  MediaFeatureManager deconstructor
    //!
    virtual ~MediaFeatureManager() { Destroy(); }

    iterator begin() { return iterator(m_features.begin()); }

    iterator end() { return iterator(m_features.end()); }

    //!
    //! \brief  Initialize all features
    //! \param  [in] settings
    //!         Pointer to the initialize settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *settings) { return MOS_STATUS_UNIMPLEMENTED; }

    //!
    //! \brief  Register features, if last two parameters use
    //!         default values, feature to be registered will
    //!         not be blocked by any packet
    //! \param  [in] featureID
    //!         ID of the feature to be reigstered
    //! \param  [in] feature
    //!         Pointer to the feature to be registered
    //! \param  [in] packetIds
    //!         Packet ID list, if it is an allow list, feature will
    //!         be only added to packets in the list, otherwise feature
    //!         will be added to packets not in the list, by default it
    //!         is an empty list
    //! \param  [in] packetIdListType
    //!         Indicate whether packet ID list is a block list
    //!         or an allow list, by default it is a block list.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RegisterFeatures(
        int                featureID,
        MediaFeature *     feature,
        std::vector<int> &&packetIds        = {},
        LIST_TYPE          packetIdListType = LIST_TYPE::BLOCK_LIST);

    //!
    //! \brief  Get packet level feature manager
    //! \param  [in] packetId
    //!         ID of packet
    //! \return std::shared_ptr<ManagerLite>
    //!         A pointer to packet level feature manager
    //!
    std::shared_ptr<ManagerLite> GetPacketLevelFeatureManager(int packetId);

    //!
    //! \brief  Update all features
    //! \param  [in] params
    //!         Pointer to the params
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Update(void *params);

    //!
    //! \brief  Destroy all features
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Destroy();

    //!
    //! \brief  Get feature
    //! \param  [in] featureID
    //!         feature ID to get the feature
    //! \return MediaFeature*
    //!         Pointer of the feature
    //!
    virtual MediaFeature *GetFeature(int featureID)
    {
        auto iter = m_features.find(featureID);
        if (iter == m_features.end())
        {
            return nullptr;
        }
        return iter->second;
    }
    //!
    //! \brief  Get Pass Number
    //! \return uint8_t
    //!         actual pass number after feature check
    //!
    uint8_t GetNumPass() { return m_passNum; };
    MediaFeatureConstSettings *GetFeatureSettings() { return m_featureConstSettings; };
    //!
    //! \brief  Check the conflict between features
    //! \param  [in] params
    //!         encode parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CheckFeatures(void *params) { return MOS_STATUS_SUCCESS; };

    //!
    //! \brief  Get DDI Target Usage
    //! \return uint8_t
    //!         return the DDI target usage.
    //!
    uint8_t GetDDITargetUsage(){return m_ddiTargetUsage;}

protected:

    //!
    //! \brief  Create feature const settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateConstSettings() { return MOS_STATUS_SUCCESS; };

    //!
    //! \brief  Create features
    //! \param  [in] constsettings
    //!         feature const settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateFeatures(void *constSettings) { return MOS_STATUS_SUCCESS; };

    //!
    //! \brief  Get Target Usage
    //! \return uint8_t
    //!         return the target usage.
    //!
    uint8_t GetTargetUsage(){return m_targetUsage;}

    container_t m_features;
    std::map<int, std::vector<int>> m_packetIdList;  // map feature ID to a vector of packet ID
    std::map<int, LIST_TYPE> m_packetIdListTypes;  // map feature ID to a flag, indicates whether packet ID vector is a block list or an allow list
    MediaFeatureConstSettings *m_featureConstSettings = nullptr;
    uint8_t m_ddiTargetUsage = 0; // for user input setting report
    uint8_t m_targetUsage = 0;
    uint8_t m_passNum = 1;
    // Media user setting instance
    MediaUserSettingSharedPtr m_userSettingPtr = nullptr;
MEDIA_CLASS_DEFINE_END(MediaFeatureManager)
};

#endif  // !__MEDIA_FEATURE_MANAGER_H__
