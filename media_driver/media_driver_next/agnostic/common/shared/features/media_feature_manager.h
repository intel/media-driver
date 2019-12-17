/*
* Copyright (c) 2018 - 2019, Intel Corporation
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
#include "media_feature.h"
#include "media_feature_const_settings.h"

enum FeatureIDs
{
    // HEVC features
    basicFeature = 0,
    encodeTile,
    hevcCqpFeature,
    hevcBrcFeature,
    hevcVdencRoiFeature,
    hevcVdencWpFeature,
    hevcVdencDssFeature,

    reserve6,
    reserve7,

    // Decode specific feature
    decodePredication,
    decodeMarker,
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

class MediaFeatureManager
{
public:
    //!
    //! \brief  MediaFeatureManager constructor
    //!
    MediaFeatureManager() {};

    //!
    //! \brief  MediaFeatureManager deconstructor
    //!
    virtual ~MediaFeatureManager() { Destroy(); }

    //!
    //! \brief  Initialize all features
    //! \param  [in] settings
    //!         Pointer to the initialize settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(void *settings);

    //!
    //! \brief  Register features
    //! \param  [in] featureID
    //!         ID of the feature to be reigstered
    //! \param  [in] feature
    //!         Pointer to the feature to be registered
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RegisterFeatures(FeatureIDs featureID, MediaFeature *feature);

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
    virtual MediaFeature *GetFeature(FeatureIDs featureID)
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

protected:

    //!
    //! \brief  Create feature const settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateConstSettigs() { return MOS_STATUS_SUCCESS; };

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

    std::map<FeatureIDs, MediaFeature *> m_features;
    MediaFeatureConstSettings *m_featureConstSettings = nullptr;
    uint8_t m_targetUsage = 0;
    uint8_t m_passNum = 1;
};

#endif  // !__MEDIA_FEATURE_MANAGER_H__
