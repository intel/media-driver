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
//! \file     media_feature_const_settings.h
//! \brief    Defines the common interface for meda feature default settings
//! \details  The meda feature default settings is further sub-divided by codec type
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __MEDIA_FEATURE_CONST_SETTINGS_H__
#define __MEDIA_FEATURE_CONST_SETTINGS_H__
#include "mos_os.h"
#include "mos_interface.h"

struct ConstTableSet
{
    void     *data = nullptr;
    uint32_t size = 0;
};

struct MediaFeatureSettings
{
    virtual ~MediaFeatureSettings() {};
};

class MediaFeatureConstSettings
{
public:

    //!
    //! \brief  MediaFeatureConstSettings constructor
    //!
    MediaFeatureConstSettings(){};
    MediaFeatureConstSettings(PMOS_INTERFACE osInterface)
    {
        if (osInterface && osInterface->pfnGetUserSettingInstance)
        {
            m_userSettingPtr = osInterface->pfnGetUserSettingInstance(osInterface);
        }
        if (!m_userSettingPtr)
        {
            MOS_OS_NORMALMESSAGE("Initialize m_userSettingPtr instance failed!");
        }
    }

    //!
    //! \brief  MediaFeatureConstSettings deconstructor
    //!
    virtual ~MediaFeatureConstSettings()
    {
        if (m_featureSetting != nullptr)
        {
            MOS_Delete(m_featureSetting);
        }
    }

    //!
    //! \brief  Prepare const settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PrepareConstSettings() { return MOS_STATUS_SUCCESS; };

    //!
    //! \brief  Get const settings
    //! \return feature default settings
    //!
    void* GetConstSettings() {return m_featureSetting;};

protected:

    //!
    //! \brief  Prepare TU specific settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetTUSettings() { return MOS_STATUS_SUCCESS; };

    //!
    //! \brief  Prepare common settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCommonSettings() { return MOS_STATUS_SUCCESS; };

    MediaFeatureSettings *m_featureSetting = nullptr;
    MediaUserSettingSharedPtr m_userSettingPtr = nullptr;  //!< usersettingInstance
MEDIA_CLASS_DEFINE_END(MediaFeatureConstSettings)
};

#endif  // !__MEDIA_FEATURE_CONST_SETTINGS_H__
