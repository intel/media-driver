
/*
* Copyright (c) 2021-2023, Intel Corporation
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
#include "media_user_setting_configure.h"

#define MOS_UserFeature_ReadValue_ID(pOsUserFeatureInterface, valueID, pValueData, osCtx)                 \
    UserFeatureReadValue(pOsUserFeatureInterface, valueID, pValueData, osCtx)

#define MOS_UserFeature_WriteValues_ID(pOsUserFeatureInterface, pWriteValues, uiNumOfValues, osCtx)       \
    UserFeatureWriteValue(pOsUserFeatureInterface, pWriteValues, uiNumOfValues, osCtx)

#define WriteUserFeature(key, value, osCtx)                                            \
{                                                                                                    \
    MOS_USER_FEATURE_VALUE_WRITE_DATA UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__; \
    UserFeatureWriteData.Value.i32Data                     = (value);                                \
    UserFeatureWriteData.ValueID                           = (key);                                  \
    UserFeatureWriteValue(nullptr, &UserFeatureWriteData, 1, osCtx);                   \
}

namespace MediaUserSetting {

class MediaUserSetting
{
public:
    //!
    //! \brief    Constructor
    //!
    MediaUserSetting();

    //!
    //! \brief    Constructor
    //!
    MediaUserSetting(MOS_USER_FEATURE_KEY_PATH_INFO *keyPathInfo);

    //!
    //! \brief    Destructor
    //!
    virtual ~MediaUserSetting();

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
    virtual MOS_STATUS Register(
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
    virtual MOS_STATUS Read(Value &value,
        const std::string &valueName,
        const Group &group,
        const Value &customValue = Value(),
        bool useCustomValue = false,
        uint32_t option = MEDIA_USER_SETTING_INTERNAL);

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
    virtual MOS_STATUS Write(
        const std::string &valueName,
        const Value &value,
        const Group &group,
        bool isForReport = false,
        uint32_t option = MEDIA_USER_SETTING_INTERNAL);

    //!
    //! \brief    Check whether the key has been registered 
    //! \param    [in] valueName
    //!           Name of the item
    //! \return   bool
    //!           true if user setting key has registered, otherwise will return false
    //!
    bool IsDeclaredUserSetting(const std::string &valueName);

    //!
    //! \brief    Read single value from User Feature
    //! \param    [in] pOsUserFeatureInterface
    //!           Pointer to OS User Interface structure
    //! \param    [in] valueID
    //!           value of enum type in MOS_USER_FEATURE_VALUE_TYPE. declares the user feature key to be readed
    //! \param    [in,out] pValueData
    //!           Pointer to User Feature Data
    //! \param    [in] mosCtx
    //!           Pointer to DDI device context
    //! \return   MOS_STATUS
    //!           Returns one of the MOS_STATUS error codes if failed
    //!
    virtual MOS_STATUS UserFeatureReadValue(
        PMOS_USER_FEATURE_INTERFACE     pOsUserFeatureInterface,
        uint32_t                        valueID,
        PMOS_USER_FEATURE_VALUE_DATA    pValueData,
        MOS_CONTEXT_HANDLE              mosCtx);

    //!
    //! \brief    Write Values to User Feature with specified ID
    //! \param    [in] pOsUserFeatureInterface
    //!           Pointer to OS User Interface structure
    //! \param    [in] pWriteValues
    //!           Pointer to User Feature Data, and related User Feature Key ID (enum type in MOS_USER_FEATURE_VALUE_TYPE)
    //! \param    [in] uiNumOfValues
    //!           number of user feature keys to be written.
    //! \param    [in] mosCtx
    //!           Pointer to DDI device context
    //! \return   MOS_STATUS
    //!           Returns one of the MOS_STATUS error codes if failed,
    //!           else MOS_STATUS_SUCCESS
    //!
    virtual MOS_STATUS UserFeatureWriteValue(
        PMOS_USER_FEATURE_INTERFACE        pOsUserFeatureInterface,
        PMOS_USER_FEATURE_VALUE_WRITE_DATA pWriteValues,
        uint32_t                           uiNumOfValues,
        MOS_CONTEXT_HANDLE                 mosCtx);

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
    Internal::Configure m_configure{};  //!< The pointer of Configure
};

}

inline MOS_STATUS DeclareUserSettingKey(
    MediaUserSettingSharedPtr userSetting,
    const std::string &valueName,
    const MediaUserSetting::Group &group,
    const MediaUserSetting::Value &defaultValue,
    bool isReportKey,
    bool useCustomPath = false,
    const std::string &customPath = "",
    bool statePath = true)
{
    if (nullptr == userSetting)
    {
        return MOS_STATUS_NULL_POINTER;
    }
    return userSetting->Register(valueName, group, defaultValue, isReportKey, false, useCustomPath, customPath, statePath);
}

inline MOS_STATUS ReadUserSetting(
    MediaUserSettingSharedPtr       userSetting,
    MediaUserSetting::Value         &value,
    const std::string               &valueName,
    const MediaUserSetting::Group   &group,
    const MediaUserSetting::Value   &customValue = MediaUserSetting::Value(),
    bool                            useCustomValue = false,
    uint32_t                        option = MEDIA_USER_SETTING_INTERNAL)
{
    if (nullptr == userSetting)
    {
        return MOS_STATUS_NULL_POINTER;
    }
    return userSetting->Read(value, valueName, group, customValue, useCustomValue, option);
}

template <typename T>
inline MOS_STATUS ReadUserSetting(
    MediaUserSettingSharedPtr userSetting,
    T                               &value,
    const std::string               &valueName,
    const MediaUserSetting::Group   &group,
    const MediaUserSetting::Value   &customValue = MediaUserSetting::Value(),
    bool                            useCustomValue = false,
    uint32_t                        option = MEDIA_USER_SETTING_INTERNAL)
{
    MediaUserSetting::Value outValue;
    MOS_STATUS  status = ReadUserSetting(userSetting, outValue, valueName, group, customValue, useCustomValue, option);
    //If the user setting is not registered, it is not allowed to read a value for it.for internal user setting, Set it with the inital outValue; for external user setting, keep input value
    //If user setting is not set, internal user setting outValue is the default value or customValue value if useCustomValue == true; for external user setting, keep input value
    if (option != MEDIA_USER_SETTING_INTERNAL && status != MOS_STATUS_SUCCESS)
    {
        return status;
    }
    value = outValue.Get<T>();
    return status;
}

inline MOS_STATUS WriteUserSetting(
    MediaUserSettingSharedPtr userSetting,
    const std::string &valueName,
    const MediaUserSetting::Value &value,
    const MediaUserSetting::Group &group,
    uint32_t option = MEDIA_USER_SETTING_INTERNAL)
{
    if (nullptr == userSetting)
    {
        return MOS_STATUS_NULL_POINTER;
    }
    return userSetting->Write(valueName, value, group);
}

inline MOS_STATUS ReportUserSetting(
    MediaUserSettingSharedPtr userSetting,
    const std::string &valueName,
    const MediaUserSetting::Value &value,
    const MediaUserSetting::Group &group,
    uint32_t option = MEDIA_USER_SETTING_INTERNAL)
{
    if (nullptr == userSetting)
    {
        return MOS_STATUS_NULL_POINTER;
    }
    return userSetting->Write(valueName, value, group, true, option);
}

inline bool IsDeclaredUserSetting(
    MediaUserSettingSharedPtr userSetting,
    const std::string &valueName)
{
    if (nullptr == userSetting)
    {
        return false;
    }
    return userSetting->IsDeclaredUserSetting(valueName);
}

#if (_DEBUG || _RELEASE_INTERNAL)
inline MOS_STATUS DeclareUserSettingKeyForDebug(
    MediaUserSettingSharedPtr userSetting,
    const std::string &valueName,
    const MediaUserSetting::Group &group,
    const MediaUserSetting::Value &defaultValue,
    bool isReportKey,
    bool useCustomPath = false,
    const std::string &customPath = "",
    bool statePath = true)
{
    if (nullptr == userSetting)
    {
        return MOS_STATUS_NULL_POINTER;
    }
    return userSetting->Register(valueName, group, defaultValue, isReportKey, true, useCustomPath, customPath, statePath);
}

inline MOS_STATUS ReadUserSettingForDebug(
    MediaUserSettingSharedPtr       userSetting,
    MediaUserSetting::Value         &value,
    const std::string               &valueName,
    const MediaUserSetting::Group   &group,
    const MediaUserSetting::Value   &customValue = MediaUserSetting::Value(),
    bool                            useCustomValue = false,
    uint32_t                        option = MEDIA_USER_SETTING_INTERNAL)
{
    if (nullptr == userSetting)
    {
        return MOS_STATUS_NULL_POINTER;
    }
    return userSetting->Read(value, valueName, group, customValue, useCustomValue, option);
}

template <typename T>
inline MOS_STATUS ReadUserSettingForDebug(
    MediaUserSettingSharedPtr       userSetting,
    T                               &value,
    const std::string               &valueName,
    const MediaUserSetting::Group   &group,
    const MediaUserSetting::Value   &customValue = MediaUserSetting::Value(),
    bool                            useCustomValue = false,
    uint32_t                        option         = MEDIA_USER_SETTING_INTERNAL)
{
    MediaUserSetting::Value outValue;
    MOS_STATUS  status = ReadUserSettingForDebug(userSetting, outValue, valueName, group, customValue, useCustomValue, option);

    //If the user setting is not registered, it is not allowed to read a value for it.for internal user setting, Set it with the inital outValue; for external user setting, keep input value
    //If user setting is not set, internal user setting outValue is the default value or customValue value if useCustomValue == true; for external user setting, keep input value
    if (option != MEDIA_USER_SETTING_INTERNAL && status != MOS_STATUS_SUCCESS)
    {
        return status;
    }
    value = outValue.Get<T>();
    return status;
}

inline MOS_STATUS WriteUserSettingForDebug(
    MediaUserSettingSharedPtr userSetting,
    const std::string &valueName,
    const MediaUserSetting::Value &value,
    const MediaUserSetting::Group &group,
    uint32_t option = MEDIA_USER_SETTING_INTERNAL)
{
    if (nullptr == userSetting)
    {
        return MOS_STATUS_NULL_POINTER;
    }
    return userSetting->Write(valueName, value, group);
}

inline MOS_STATUS ReportUserSettingForDebug(
    MediaUserSettingSharedPtr userSetting,
    const std::string &valueName,
    const MediaUserSetting::Value &value,
    const MediaUserSetting::Group &group,
    uint32_t option = MEDIA_USER_SETTING_INTERNAL)
{
    if (nullptr == userSetting)
    {
        return MOS_STATUS_NULL_POINTER;
    }
    return userSetting->Write(valueName, value, group, true, option);
}

#else
#define DeclareUserSettingKeyForDebug(userSetting, valueName, group, defaultValue, isReportKey, ...) MOS_STATUS_SUCCESS
#define ReadUserSettingForDebug(userSetting, value, valueName, group, ...) MOS_STATUS_SUCCESS
#define WriteUserSettingForDebug(userSetting, valueName, value, group, ...) MOS_STATUS_SUCCESS
#define ReportUserSettingForDebug(userSetting, valueName, value, group, ...) MOS_STATUS_SUCCESS
#endif

#endif