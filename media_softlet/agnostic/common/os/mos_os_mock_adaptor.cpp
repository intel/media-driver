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
//! \file     mos_os_mock_adaptor.cpp
//! \brief    Common interface and structure used in mock adaptor.
//!

#include "mos_os.h"
#include "mos_os_specific.h"
#include "mos_context_next.h"
#include "mos_interface.h"
#include "mos_os_mock_adaptor.h"
#include "mos_os_mock_adaptor_specific.h"

#define TGL_A0_REV_ID 0x00
#define TGL_B0_REV_ID 0x01
#define TGL_C0_REV_ID 0x02

bool  MosMockAdaptor::m_enabled = false;
PRODUCT_FAMILY MosMockAdaptor::m_productFamily = {};
std::string MosMockAdaptor::m_stepping = {};
uint16_t MosMockAdaptor::m_deviceId = 0;
MosMockAdaptor *MosMockAdaptor::m_mocAdaptor = nullptr;

MosMockAdaptor::MosMockAdaptor()
{
}

MosMockAdaptor::~MosMockAdaptor()
{
}

MOS_STATUS MosMockAdaptor::RegkeyRead(PMOS_CONTEXT osContext)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    int32_t     value   = 0;
    MediaUserSetting::Value  id;
    const char *pcDefaultValue  = nullptr;
    MediaUserSettingSharedPtr userSettingPtr = MosInterface::MosGetUserSettingInstance(osContext);

    ReadUserSettingForDebug(
        userSettingPtr,
        value,
        __MEDIA_USER_FEATURE_VALUE_MOCKADAPTOR_PLATFORM,
        MediaUserSetting::Group::Device);
    m_productFamily = (PRODUCT_FAMILY)value;

    ReadUserSettingForDebug(
        userSettingPtr,
        id,
        __MEDIA_USER_FEATURE_VALUE_MOCKADAPTOR_STEPPING,
        MediaUserSetting::Group::Device);

    if (id.ConstString().size() > 0)
    {
        m_stepping += id.ConstString();
    }
    else
    {
        pcDefaultValue = MOS_MOCKADAPTOR_DEFAULT_STEPPING;
        m_stepping.append(pcDefaultValue);
    }

    value = 0;
    ReadUserSettingForDebug(
        userSettingPtr,
        value,
        __MEDIA_USER_FEATURE_VALUE_MOCKADAPTOR_DEVICE,
        MediaUserSetting::Group::Device);
    m_deviceId = (uint16_t)value;

    return eStatus;
}

MOS_STATUS MosMockAdaptor::Init(
    MOS_CONTEXT_HANDLE osDriverContext,
    OsContextNext      *osDeviceContext)
{
    MOS_STATUS   eStatus       = MOS_STATUS_SUCCESS;
    uint32_t     value         = 0;
    bool         nullHwEnabled = false;
    PMOS_CONTEXT osContext     = (PMOS_CONTEXT)osDriverContext;
    MOS_OS_CHK_NULL_RETURN(osContext);
    MOS_OS_CHK_NULL_RETURN(osDeviceContext);

    MediaUserSettingSharedPtr userSettingPtr = MosInterface::MosGetUserSettingInstance(osContext);
    ReadUserSettingForDebug(
        userSettingPtr,
        value,
        __MEDIA_USER_FEATURE_VALUE_NULLHW_ENABLE,
        MediaUserSetting::Group::Device);
    nullHwEnabled = (value) ? true : false;
    osDeviceContext->SetNullHwIsEnabled(nullHwEnabled);

    if (nullHwEnabled && !m_enabled) {
        m_enabled = true;

        MOS_OS_CHK_STATUS_RETURN(RegkeyRead(osContext));

        m_mocAdaptor = MOS_New(MosMockAdaptorSpecific);
        MOS_OS_CHK_NULL_RETURN(m_mocAdaptor);

        eStatus = m_mocAdaptor->Initialize(osContext);
    }

    return eStatus;
}

MOS_STATUS MosMockAdaptor::Destroy()
{
    if (m_mocAdaptor != nullptr)
    {
        MOS_Delete(m_mocAdaptor);
        m_mocAdaptor = nullptr;
    }

    m_enabled = false;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosMockAdaptor::InitContext(
    PMOS_CONTEXT osContext)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL_RETURN(osContext);

    m_pPlatform     = &osContext->m_platform;
    m_pSkuTable     = &osContext->m_skuTable;
    m_pWaTable      = &osContext->m_waTable;
    m_pGtSystemInfo = &osContext->m_gtSystemInfo;

    MOS_OS_CHK_STATUS_RETURN(InitializePlatForm());

    if (m_pPlatform->eRenderCoreFamily >= IGFX_GEN12_CORE)
    {
        MOS_OS_CHK_STATUS_RETURN(InitializeSkuWaTable(osContext));
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Unsupported platform!");
        eStatus = MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    return eStatus;
}

MOS_STATUS MosMockAdaptor::Initialize(
    PMOS_CONTEXT osContext)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL_RETURN(osContext);
    MOS_OS_CHK_STATUS_RETURN(InitContext(osContext));

    MOS_OS_CHK_STATUS_RETURN(ReplacePlatformInfo(osContext));

    MOS_OS_CHK_STATUS_RETURN(UpdateUserFeatureKey(osContext));

    return eStatus;
}