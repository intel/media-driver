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
//! \file     media_user_setting_definition.cpp
//! \brief    User setting definition
//!

#include "media_user_setting_definition.h"

namespace MediaUserSetting {
namespace Internal {

Definition::Definition(const std::string &itemName,
    const Value &defaultValue,
    bool isReportKey,
    bool debugOnly,
    bool useCustomPath,
    const std::string &subPath,
    UFKEY_NEXT rootKey,
    bool statePath):
m_itemName(itemName),
m_itemEnvName(itemName),
m_defaultValue(defaultValue),
m_isReportKey(isReportKey),
m_debugOnly(debugOnly),
m_useCustomePath(useCustomPath),
m_subPath(subPath),
m_rootKey(rootKey),
m_statePath(statePath)
{
    std::replace(m_itemEnvName.begin(), m_itemEnvName.end(), ' ', '_');
}

Definition::Definition(const Definition& def)
{
    SetData(def);
}

Definition::~Definition()
{
}

inline Definition& Definition::operator=(const Definition& def)
{
    if (this != &def)
    {
        SetData(def);
    }

    return *this;
}

inline void Definition::SetData(const Definition& def)
{
    m_itemName = def.m_itemName;
    m_debugOnly = def.m_debugOnly;
    m_defaultValue = def.m_defaultValue;
    m_isReportKey = def.m_isReportKey;
    m_subPath = def.m_subPath;
    m_useCustomePath = def.m_useCustomePath;
    m_rootKey = def.m_rootKey;
    m_statePath = def.m_statePath;
}

}}