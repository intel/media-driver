
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
//! \file     media_user_setting.h
//! \brief    The interface of media user setting
//! \details  This class is the implementation of media user setting interface. We need several steps to use the media
//!           user setting.
//!           1) Declare the definition of media user setting item in initialize function, just like:
//!              DeclareUserSettingKey("User Setting", MediaUserSetting::Device, MediaUserSetting::Value(true), true);
//!           If the setting item is only can be user in relase-internal/debug mode:
//!              DeclareUserSettingKeyForDebug("User Setting", MediaUserSetting::Device, MediaUserSetting::Value(true), true);
//!           If the return value is MOS_STATUS_FILE_EXISTS, means "User Setting" has been used.
//!           2) Then we can use ReadUserSetting to read the value of the specific media user setting item.
//!           If you want to provide the customized default value if failed call like:
//！             ReadUserSetting(value, "User Setting", MediaUserSetting::Device, m_osInterface->pOsContext, true, true);
//!           If you don't want to provide the customized default value:
//!              ReadUserSetting(value, "User Setting", MediaUserSetting::Device, m_osInterface->pOsContext);
//!           3) If you want to write specific media user setting to configuration path, call:
//!              WriteUserSetting("User Setting", MediaUserSetting::Value(false), m_osInterface->pOsContext)
//!           If you just want to report the value of specific setting item, need to call like:
//!              ReportUserSetting("User Setting", MediaUserSetting::Value(false), m_osInterface->pOsContext)
//!

#ifndef __MEDIA_USER_SETTING__H__
#define __MEDIA_USER_SETTING__H__

#include <string>
#include <map>
#include "mos_util_user_interface.h"
#include "media_user_setting_value.h"
#include "media_user_setting_configure.h"

namespace MediaUserSetting {

class MediaUserSetting
{
public:
    //!
    //! \brief    Static entrypoint, get the instance of media user setting
    //! \return   std::shared_ptr<MediaUserSetting>
    //!           Pointer of media user setting
    //!
    static std::shared_ptr<MediaUserSetting> Instance();

    //!
    //! \brief    Destroy media user setting
    //! \return   void
    //!
    static void Destroy();

    //!
    //! \brief    Register user setting item
    //! \param    [in] valueName
    //!           Name of the item
    //! \param    [in] group
    //!           Group of the item
    //! \param    [in] defaultValue
    //!           The default value of the item
    //! \param    [in] isReportKey
    //!           Whether this item can be reported
    //! \param    [in] debugOnly
    //!           Whether this item is only for debug/release-internal
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if no error, otherwise will return failed reason
    //!
    MOS_STATUS Register(
        const std::string &valueName,
        const Group &group,
        const Value &defaultValue,
        bool isReportKey = false,
        bool debugOnly = false,
        const std::string &customPath = "");

    //!
    //! \brief    Read value of specific item
    //! \param    [out] value
    //!           The return value of the item
    //! \param    [in] valueName
    //!           Name of the item
    //! \param    [in] group
    //!           Group of the item
    //! \param    [in] mosContext
    //!           The pointer of mos context
    //! \param    [in] customValue
    //!           The custom value when failed
    //! \param    [in] useCustomValue
    //!           Whether use costom value when failed
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if no error, otherwise will return failed reason
    //!
    MOS_STATUS Read(Value &value,
        const std::string &valueName,
        const Group &group,
        PMOS_CONTEXT mosContext,
        const Value &customValue = Value(),
        bool useCustomValue = false);

    //!
    //! \brief    Write value to specific item
    //! \param    [in] valueName
    //!           Name of the item
    //! \param    [in] value
    //!           The value write to specific item
    //! \param    [in] group
    //!           Group of the item
    //! \param    [in] mosContext
    //!           The pointer of mos context
    //! \param    [in] isForReport
    //!           This call is for reporting a item value or modify the value of the item
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if no error, otherwise will return failed reason
    //!
    MOS_STATUS Write(
        const std::string &valueName,
        const Value &value,
        const Group &group,
        PMOS_CONTEXT mosContext,
        bool isForReport = false);

protected:
    //!
    //! \brief    Constructor
    //!
    MediaUserSetting();

protected:
    static std::shared_ptr<MediaUserSetting> m_instance;
    Internal::Configure m_configure{};  //!< The pointer of Configure
};

}

inline MOS_STATUS DeclareUserSettingKey(
    const std::string &valueName,
    const MediaUserSetting::Group &group,
    const MediaUserSetting::Value &defaultValue,
    bool isReportKey,
    const std::string &customPath = "")
{
    auto instance = MediaUserSetting::MediaUserSetting::Instance();
    return instance->Register(valueName, group, defaultValue, isReportKey, false, customPath);
}

inline MOS_STATUS ReadUserSetting(
    MediaUserSetting::Value &value,
    const std::string &valueName,
    const MediaUserSetting::Group &group,
    PMOS_CONTEXT mosContext,
    const MediaUserSetting::Value &customValue = MediaUserSetting::Value(),
    bool useCustomValue = false)
{
    auto instance = MediaUserSetting::MediaUserSetting::Instance();
    return instance->Read(value, valueName, group, mosContext, customValue, useCustomValue);
}

inline MOS_STATUS WriteUserSetting(
    const std::string &valueName,
    const MediaUserSetting::Value &value,
    const MediaUserSetting::Group &group,
    PMOS_CONTEXT mosContext)
{
    auto instance = MediaUserSetting::MediaUserSetting::Instance();
    return instance->Write(valueName, value, group, mosContext);
}

inline MOS_STATUS ReportUserSetting(
    const std::string &valueName,
    const MediaUserSetting::Value &value,
    const MediaUserSetting::Group &group,
    PMOS_CONTEXT mosContext)
{
    auto instance = MediaUserSetting::MediaUserSetting::Instance();
    return instance->Write(valueName, value, group, mosContext, true);
}

#if (_DEBUG || _RELEASE_INTERNAL)
inline MOS_STATUS DeclareUserSettingKeyForDebug(
    const std::string &valueName,
    const MediaUserSetting::Group &group,
    const MediaUserSetting::Value &defaultValue,
    bool isReportKey,
    const std::string &customPath = "")
{
    auto instance = MediaUserSetting::MediaUserSetting::Instance();
    return instance->Register(valueName, group, defaultValue, isReportKey, true, customPath);
}

inline MOS_STATUS ReadUserSettingForDebug(
    MediaUserSetting::Value &value,
    const std::string &valueName,
    const MediaUserSetting::Group &group,
    PMOS_CONTEXT mosContext,
    const MediaUserSetting::Value &customValue = MediaUserSetting::Value(),
    bool useCustomValue = false)
{
    auto instance = MediaUserSetting::MediaUserSetting::Instance();
    return instance->Read(value, valueName, group, mosContext, customValue, useCustomValue);
}

inline MOS_STATUS WriteUserSettingForDebug(
    const std::string &valueName,
    const MediaUserSetting::Value &value,
    const MediaUserSetting::Group &group,
    PMOS_CONTEXT mosContext)
{
    auto instance = MediaUserSetting::MediaUserSetting::Instance();
    return instance->Write(valueName, value, group, mosContext);
}

inline MOS_STATUS ReportUserSettingForDebug(
    const std::string &valueName,
    const MediaUserSetting::Value &value,
    const MediaUserSetting::Group &group,
    PMOS_CONTEXT mosContext)
{
    auto instance = MediaUserSetting::MediaUserSetting::Instance();
    return instance->Write(valueName, value, group, mosContext, true);
}

#else
#define DeclareUserSettingKeyForDebug(valueName, group, defaultValue, isReportKey, customPath) MOS_STATUS_SUCCESS
#define ReadUserSettingForDebug(value, valueName, group, mosContext, customValue, useCustomValue) MOS_STATUS_SUCCESS
#define WriteUserSettingForDebug(valueName, value, group, mosContext) MOS_STATUS_SUCCESS
#define ReportUserSettingForDebug(valueName, value, group, mosContext) MOS_STATUS_SUCCESS
#endif

#endif