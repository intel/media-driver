/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     mos_util_user_interface_next.cpp
//! \brief    Common MOS util user feature key service across different platform
//!

#include "mos_os_next.h"
#include "mos_util_user_interface_next.h"
#include "media_interfaces_mosutil.h"

std::map<uint32_t, PMOS_USER_FEATURE_VALUE>  MosUtilUserInterfaceNext::m_userFeatureKeyMap;
MosUtilUserInterfaceNext* MosUtilUserInterfaceNext::m_inst = nullptr;
uint32_t MosUtilUserInterfaceNext::m_refCount = 0;
bool MosUtilUserInterfaceNext::m_defaultValueChanged = true;

MosUtilUserInterfaceNext::MosUtilUserInterfaceNext()
{
}

MOS_STATUS MosUtilUserInterfaceNext::AddEntry(const uint32_t keyId, PMOS_USER_FEATURE_VALUE userFeatureKey)
{
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
        return MOS_STATUS_SUCCESS;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilUserInterfaceNext::DelEntry(const uint32_t keyId)
{
    auto result = m_userFeatureKeyMap.find(keyId);

    if (result != m_userFeatureKeyMap.end())
    {
        m_userFeatureKeyMap.erase(keyId);
    }
    else
    {
        return MOS_STATUS_SUCCESS;
    }

    return MOS_STATUS_SUCCESS;
}

PMOS_USER_FEATURE_VALUE MosUtilUserInterfaceNext::GetValue(uint32_t keyId)
{
    auto result = m_userFeatureKeyMap.find(keyId);

    if (result != m_userFeatureKeyMap.end())
    {
        return result->second;
    }
    else
    {
        return nullptr;
    }

    return nullptr;
}

MosUtilUserInterfaceNext* MosUtilUserInterfaceNext::GetInstance(PRODUCT_FAMILY productFamily)
{
    if (m_refCount == 0 && m_inst == nullptr)
    {
        m_inst = (MosUtilUserInterfaceNext*)MosUtilDevice::CreateFactory(productFamily);
    }
    ++m_refCount;

    return m_inst;
}

void MosUtilUserInterfaceNext::Destroy()
{
    if (m_refCount > 0)
    {
        --m_refCount;
        if (m_refCount == 0 && m_inst)
        {
            MOS_Delete(m_inst);
            m_inst = nullptr;
        }
    }
}
