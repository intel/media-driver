
/*
* Copyright (c) 2021-2022, Intel Corporation
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
    //! \param    [in] useCustomPath
    //!           Specifiy a read path
    //! \param    [in] customPath
    //!           The specified read path
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if no error, otherwise will return failed reason
    //!
    MOS_STATUS Register(
        const std::string &valueName,
        const Group &group,
        const Value &defaultValue,
        bool isReportKey = false,
        bool debugOnly = false,
        bool useCustomPath = false,
        const std::string &customPath = "",
        bool statePath = true);

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

    //!
    //! \brief    Check whether the key has been registered 
    //! \param    [in] valueName
    //!           Name of the item
    //! \return   bool
    //!           true if user setting key has registered, otherwise will return false
    //!
    bool IsDeclaredUserSetting(const std::string &valueName);

    //!
    //! \brief    Get media user setting definitions of specific group
    //! \param    [in] group
    //!           Group of the item
    //! \return   Media user setting definitions
    //!           Definitions of specific group, return definitions of device group if failed
    //!
    inline Internal::Definitions &GetDefinitions(const Group &group)
    {
        return m_configure.GetDefinitions(group);
    }
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
    bool useCustomPath = false,
    const std::string &customPath = "",
    bool statePath = true)
{
    auto instance = MediaUserSetting::MediaUserSetting::Instance();
    return instance->Register(valueName, group, defaultValue, isReportKey, false, useCustomPath, customPath, statePath);
}

inline MOS_STATUS ReadUserSetting(
    MediaUserSetting::Value         &value,
    const std::string               &valueName,
    const MediaUserSetting::Group   &group,
    PMOS_CONTEXT                    mosContext,
    const MediaUserSetting::Value   &customValue = MediaUserSetting::Value(),
    bool                            useCustomValue = false)
{
    auto instance = MediaUserSetting::MediaUserSetting::Instance();
    auto status   =  instance->Read(value, valueName, group, mosContext, customValue, useCustomValue);
    if(status != MOS_STATUS_USER_FEATURE_KEY_OPEN_FAILED && status != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("User setting %s read error", valueName.c_str());
    }
    return status;
}

template <typename T>
inline MOS_STATUS ReadUserSetting(
    T                               &value,
    const std::string               &valueName,
    const MediaUserSetting::Group   &group,
    PMOS_CONTEXT                    mosContext,
    const MediaUserSetting::Value   &customValue = MediaUserSetting::Value(),
    bool                            useCustomValue = false)
{
    MediaUserSetting::Value outValue;
    MOS_STATUS  status = ReadUserSetting(outValue, valueName, group, mosContext, customValue, useCustomValue);
    //If user setting is not set, outValue is the default value or customValue value if useCustomValue == true.
    //If the user setting is not registered, it is not allowed to read a value for it. Set it with the inital outValue.
    value = outValue.Get<T>();
    return status;
}

inline MOS_STATUS ReadUserSetting(
    bool                            &value,
    const std::string               &valueName,
    const MediaUserSetting::Group   &group,
    PMOS_CONTEXT                    mosContext,
    const MediaUserSetting::Value   &customValue = MediaUserSetting::Value(),
    bool                            useCustomValue = false)
{
    uint32_t outValue = 0;
    MOS_STATUS  status = ReadUserSetting(outValue, valueName, group, mosContext, customValue, useCustomValue);
    value = outValue ? true : false;
    return status;
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

inline bool IsDeclaredUserSetting(const std::string &valueName)
{
    auto instance = MediaUserSetting::MediaUserSetting::Instance();
    return instance->IsDeclaredUserSetting(valueName);
}

#if (_DEBUG || _RELEASE_INTERNAL)
inline MOS_STATUS DeclareUserSettingKeyForDebug(
    const std::string &valueName,
    const MediaUserSetting::Group &group,
    const MediaUserSetting::Value &defaultValue,
    bool isReportKey,
    bool useCustomPath = false,
    const std::string &customPath = "",
    bool statePath = true)
{
    auto instance = MediaUserSetting::MediaUserSetting::Instance();
    return instance->Register(valueName, group, defaultValue, isReportKey, true, useCustomPath, customPath, statePath);
}

inline MOS_STATUS ReadUserSettingForDebug(
    MediaUserSetting::Value         &value,
    const std::string               &valueName,
    const MediaUserSetting::Group   &group,
    PMOS_CONTEXT                    mosContext,
    const MediaUserSetting::Value   &customValue = MediaUserSetting::Value(),
    bool                            useCustomValue = false)
{
    auto instance = MediaUserSetting::MediaUserSetting::Instance();
    auto status   = instance->Read(value, valueName, group, mosContext, customValue, useCustomValue);
    if(status != MOS_STATUS_USER_FEATURE_KEY_OPEN_FAILED && status != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("User setting %s read error", valueName.c_str());
    }
    return status;
}

template <typename T>
inline MOS_STATUS ReadUserSettingForDebug(
    T                               &value,
    const std::string               &valueName,
    const MediaUserSetting::Group   &group,
    PMOS_CONTEXT                    mosContext,
    const MediaUserSetting::Value   &customValue = MediaUserSetting::Value(),
    bool                            useCustomValue = false)
{
    MediaUserSetting::Value outValue;
    MOS_STATUS  status = ReadUserSettingForDebug(outValue, valueName, group, mosContext, customValue, useCustomValue);

    //If user setting is not set, outValue is the default value or customValue value if useCustomValue == true.
    //If the user setting is not registered, it is not allowed to read a value for it. Set it with the inital outValue.
    value = outValue.Get<T>();
    return status;
}

inline MOS_STATUS ReadUserSettingForDebug(
    bool                            &value,
    const std::string               &valueName,
    const MediaUserSetting::Group   &group,
    PMOS_CONTEXT                    mosContext,
    const MediaUserSetting::Value   &customValue = MediaUserSetting::Value(),
    bool                            useCustomValue = false)
{
    uint32_t outValue = 0;
    MOS_STATUS  status = ReadUserSettingForDebug(outValue, valueName, group, mosContext, customValue, useCustomValue);
    value = outValue ? true : false;
    return status;
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
#define DeclareUserSettingKeyForDebug(valueName, group, defaultValue, isReportKey, ...) MOS_STATUS_SUCCESS
#define ReadUserSettingForDebug(value, valueName, group, mosContext, customValue, useCustomValue) MOS_STATUS_SUCCESS
#define WriteUserSettingForDebug(valueName, value, group, mosContext) MOS_STATUS_SUCCESS
#define ReportUserSettingForDebug(valueName, value, group, mosContext) MOS_STATUS_SUCCESS
#endif

#endif