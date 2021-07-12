/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     null_hardware.cpp
//! \brief    Defines interfaces for null hardware
#include "mos_os_mock_adaptor.h"
#include "null_hardware.h"
#include "mhw_mi.h"

bool  NullHW::m_initilized = false;
bool  NullHW::m_enabled = false;
std::map<void*, bool> NullHW::m_forceBypassMap = {};

MOS_STATUS NullHW::Init(
    PMOS_CONTEXT osContext)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MOS_OS_CHK_NULL_RETURN(osContext);

    if (!m_initilized) {
        m_initilized = true;

        MOS_USER_FEATURE_VALUE_DATA UserFeatureData = {};
        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_NULLHW_ENABLE_ID,
            &UserFeatureData,
            osContext);

        m_enabled = (UserFeatureData.i32Data) ? true : false;

        if (m_enabled)
        {
            eStatus = MosMockAdaptor::Init(osContext);
        }
        else
        {
            eStatus = MOS_STATUS_INVALID_HANDLE;
        }

    }
    return eStatus;
}

MOS_STATUS NullHW::Destroy()
{
    return MosMockAdaptor::Destroy();
}

MOS_STATUS NullHW::StartPredicate(MhwMiInterface* miInterface, PMOS_COMMAND_BUFFER cmdBuffer)
{
    bool forceBypassHw = false;
    if (m_forceBypassMap.find((void*)miInterface) != m_forceBypassMap.end())
    {
        forceBypassHw = m_forceBypassMap[(void*)miInterface];
    }

    if (!m_enabled && !forceBypassHw)
    {
        return MOS_STATUS_SUCCESS;
    }
    MOS_OS_CHK_NULL_RETURN(miInterface);
    MOS_OS_CHK_NULL_RETURN(cmdBuffer);

    return miInterface->AddMiSetPredicateCmd(cmdBuffer, MHW_MI_SET_PREDICATE_ENABLE_ALWAYS);
}

MOS_STATUS NullHW::StopPredicate(MhwMiInterface* miInterface, PMOS_COMMAND_BUFFER cmdBuffer)
{
    bool forceBypassHw = false;
    if (m_forceBypassMap.find((void*)miInterface) != m_forceBypassMap.end())
    {
        forceBypassHw = m_forceBypassMap[(void*)miInterface];
    }

    if (!m_enabled && !forceBypassHw)
    {
        return MOS_STATUS_SUCCESS;
    }
    MOS_OS_CHK_NULL_RETURN(miInterface);
    MOS_OS_CHK_NULL_RETURN(cmdBuffer);

    return miInterface->AddMiSetPredicateCmd(cmdBuffer, MHW_MI_SET_PREDICATE_DISABLE);
}

void NullHW::StatusReport(void* handle, uint32_t &status, uint32_t &streamSize)
{
    bool forceBypassHw = false;
    if (m_forceBypassMap.find(handle) != m_forceBypassMap.end())
    {
        forceBypassHw = m_forceBypassMap[handle];
    }

    if (!m_enabled && !forceBypassHw)
    {
        return;
    }

    status = 0;
    streamSize = 1024;
}

MOS_STATUS NullHW::AddBypassMapItem(void* key, bool value)
{
    MOS_OS_CHK_NULL_RETURN(key);
    m_forceBypassMap[key] = value;
    return MOS_STATUS_SUCCESS;
}