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
//! \file     media_user_setting.cpp
//! \brief    The interface of media user setting
//!

#include <algorithm>
#include "media_user_setting.h"
#include "media_user_setting_configure.h"

namespace MediaUserSetting {

struct MakeSharedEnabler : public MediaUserSetting
{
};

std::shared_ptr<MediaUserSetting> MediaUserSetting::m_instance = nullptr;

MediaUserSetting::MediaUserSetting()
{
}
MediaUserSetting::MediaUserSetting(MOS_USER_FEATURE_KEY_PATH_INFO *keyPathInfo) : m_configure(keyPathInfo)
{

}

void MediaUserSetting::Destroy()
{
    if (m_instance != nullptr)
    {
        m_instance.reset();
        m_instance = nullptr;
    }
 }


std::shared_ptr<MediaUserSetting> MediaUserSetting::Instance()
{
    if (m_instance == nullptr)
    {
        m_instance = std::make_shared<MakeSharedEnabler>();
    }

    return m_instance;
}

MOS_STATUS MediaUserSetting::Register(
    const std::string &valueName,
    const Group &group,
    const Value &defaultValue,
    bool isReportKey,
    bool debugOnly,
    bool useCustomPath,
    const std::string &customPath,
    bool statePath)
{
    return m_configure.Register(
                    valueName,
                    group,
                    defaultValue,
                    isReportKey,
                    debugOnly,
                    useCustomPath,
                    customPath,
                    statePath);
}

MOS_STATUS MediaUserSetting::Read(Value &value,
    const std::string &valueName,
    const Group &group,
    const Value &customValue,
    bool useCustomValue)
{
    return m_configure.Read(value, valueName, group, customValue, useCustomValue);
}

MOS_STATUS MediaUserSetting::Write(
    const std::string &valueName,
    const Value &value,
    const Group &group,
    bool isForReport)
{
    return m_configure.Write(valueName, value, group, isForReport);
}

bool MediaUserSetting::IsDeclaredUserSetting(const std::string &valueName)
{
    return m_configure.IsDefinitionExist(valueName);
}

}

