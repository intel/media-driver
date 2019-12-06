/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     mos_util_user_interface.cpp
//! \brief    Common MOS util user feature key service across different platform
//!

#include "mos_os.h"
#include "mos_util_user_interface.h"

std::map<uint32_t, PMOS_USER_FEATURE_VALUE> MosUtilUserInterface::m_userFeatureKeyMap;
MosMutex                                    MosUtilUserInterface::m_mosMutex;
bool                                        MosUtilUserInterface::m_defaultValueChanged = false;

MOS_STATUS MosUtilUserInterface::AddEntry(const uint32_t keyId, PMOS_USER_FEATURE_VALUE userFeatureKey)
{
    m_mosMutex.Lock();
    auto result = m_userFeatureKeyMap.find(keyId);

    if (result == m_userFeatureKeyMap.end())
    {
        m_userFeatureKeyMap.insert(std::make_pair(keyId, userFeatureKey));
    }
    else
    {
        MOS_OS_NORMALMESSAGE("User feature key already exist, replacing the old one.");
        m_userFeatureKeyMap.erase(keyId);
        m_userFeatureKeyMap.insert(std::make_pair(keyId, userFeatureKey));
        m_mosMutex.Unlock();
        return MOS_STATUS_SUCCESS;
    }
    m_mosMutex.Unlock();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilUserInterface::DelEntry(const uint32_t keyId)
{
    m_mosMutex.Lock();

    auto result = m_userFeatureKeyMap.find(keyId);

    if (result != m_userFeatureKeyMap.end())
    {
        m_userFeatureKeyMap.erase(keyId);
    }
    else
    {
        m_mosMutex.Unlock();
        return MOS_STATUS_SUCCESS;
    }
    m_mosMutex.Unlock();

    return MOS_STATUS_SUCCESS;
}

PMOS_USER_FEATURE_VALUE MosUtilUserInterface::GetValue(uint32_t keyId)
{
    m_mosMutex.Lock();

    auto result = m_userFeatureKeyMap.find(keyId);

    if (result != m_userFeatureKeyMap.end())
    {
        auto userFeatureValue = result->second;
        m_mosMutex.Unlock();

        return userFeatureValue;
    }
    else
    {
        m_mosMutex.Unlock();
        return nullptr;
    }
    m_mosMutex.Unlock();

    return nullptr;
}
