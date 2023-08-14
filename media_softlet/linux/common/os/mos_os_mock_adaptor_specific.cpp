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
//! \file     mos_os_mock_adaptor_specific.cpp
//! \brief    Common interface and structure used in mock adaptor.
//!
#include "mos_os.h"
#include "mos_os_specific.h"
#include "mos_interface.h"
#include "skuwa_factory.h"
#include "hwinfo_linux.h"
#include "mos_os_mock_adaptor_specific.h"

typedef DeviceInfoFactory<struct GfxDeviceInfo> DeviceInfoFact;
typedef DeviceInfoFactory<struct LinuxDeviceInit> DeviceInitFact;

MosMockAdaptorSpecific::MosMockAdaptorSpecific()
{

}

MosMockAdaptorSpecific::~MosMockAdaptorSpecific()
{

}

GfxDeviceInfo *MosMockAdaptorSpecific::GetDeviceInfo(uint32_t devId)
{
    GfxDeviceInfo *devInfo = DeviceInfoFact::LookupDevice(devId);

    return devInfo;
}

LinuxDeviceInit *MosMockAdaptorSpecific::GetDeviceInit(uint32_t platKey)
{
    LinuxDeviceInit *devInfo = DeviceInitFact::LookupDevice(platKey);

    return devInfo;
}

MOS_STATUS MosMockAdaptorSpecific::InitializeSkuWaTable(PMOS_CONTEXT context)
{
    MOS_OS_CHK_NULL_RETURN(context);
    MOS_OS_CHK_NULL_RETURN(m_pPlatform);
    auto            userSettingPtr = MosInterface::MosGetUserSettingInstance(context);

    LinuxDriverInfo drvInfo = {18, 3, 0, 23172, 3, 1, 0, 1, 0, 0, 1, 0};
    if (mos_get_driver_info(context->bufmgr, &drvInfo))
    {
        return MOS_STATUS_INVALID_HANDLE;
    }

    drvInfo.devId = m_pPlatform->usDeviceID;
    drvInfo.devRev = m_pPlatform->usRevId;

    GfxDeviceInfo *devInfo = GetDeviceInfo(drvInfo.devId);
    if (devInfo == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to get the device info for Device id: %x\n", drvInfo.devId);
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }
    /* Initialize Platform Info */
    m_pGtSystemInfo->SliceCount        = devInfo->SliceCount;
    m_pGtSystemInfo->SubSliceCount     = devInfo->SubSliceCount;
    m_pGtSystemInfo->EUCount           = devInfo->EUCount;

    if (devInfo->InitMediaSysInfo &&
        devInfo->InitMediaSysInfo(devInfo, m_pGtSystemInfo))
    {
        MOS_OS_NORMALMESSAGE("Init Media SystemInfo\n");
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Failed to Init Gt System Info\n");
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    if (mos_query_sys_engines(context->bufmgr, m_pGtSystemInfo))
    {
        MOS_OS_ASSERTMESSAGE("Failed to Init Gt System Info\n");
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    uint32_t platformKey = devInfo->productFamily;
    LinuxDeviceInit *devInit = GetDeviceInit(platformKey);

    if (devInit && devInit->InitMediaFeature &&
        devInit->InitMediaWa &&
        devInit->InitMediaFeature(devInfo, m_pSkuTable, &drvInfo, userSettingPtr) &&
        devInit->InitMediaWa(devInfo, m_pWaTable, &drvInfo))
    {
        MOS_OS_NORMALMESSAGE("Init Media SKU/WA info successfully\n");
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Failed to Init SKU/WA Info\n");
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    uint32_t devExtKey = platformKey + MEDIA_EXT_FLAG;
    LinuxDeviceInit *devExtInit = GetDeviceInit(devExtKey);

    /* The initializationof Ext SKU/WA is optional. So skip the check of return value */
    if (devExtInit && devExtInit->InitMediaFeature &&
        devExtInit->InitMediaWa &&
        devExtInit->InitMediaFeature(devInfo, m_pSkuTable, &drvInfo, userSettingPtr) &&
        devExtInit->InitMediaWa(devInfo, m_pWaTable, &drvInfo))
    {
        MOS_OS_NORMALMESSAGE("Init Media SystemInfo successfully\n");
    }

    /* disable it on Linux */
    MEDIA_WR_SKU(m_pSkuTable, FtrPerCtxtPreemptionGranularityControl, 0);
    MEDIA_WR_SKU(m_pSkuTable, FtrMediaThreadGroupLevelPreempt, 0);
    MEDIA_WR_SKU(m_pSkuTable, FtrMediaMidBatchPreempt, 0);
    MEDIA_WR_SKU(m_pSkuTable, FtrMediaMidThreadLevelPreempt, 0);
    MEDIA_WR_SKU(m_pSkuTable, FtrGpGpuThreadGroupLevelPreempt, 0);
    MEDIA_WR_SKU(m_pSkuTable, FtrGpGpuMidBatchPreempt, 0);
    MEDIA_WR_SKU(m_pSkuTable, FtrGpGpuMidThreadLevelPreempt, 0);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosMockAdaptorSpecific::UpdateUserFeatureKey(PMOS_CONTEXT osContext)
{
    MOS_OS_CHK_NULL_RETURN(osContext);
    MediaUserSettingSharedPtr   userSettingPtr  = MosInterface::MosGetUserSettingInstance(osContext);
    PLATFORM                    platForm        = osContext->m_platform;
    int32_t                     iDeviceId       = osContext->iDeviceId;
    int32_t                     eProductFaimily = (int32_t)platForm.eProductFamily;

    ReportUserSettingForDebug(userSettingPtr, __MEDIA_USER_FEATURE_VALUE_MOCKADAPTOR_PLATFORM,eProductFaimily,MediaUserSetting::Group::Device);
    ReportUserSettingForDebug(userSettingPtr, __MEDIA_USER_FEATURE_VALUE_MOCKADAPTOR_DEVICE,iDeviceId,MediaUserSetting::Group::Device);

    return MOS_STATUS_SUCCESS;
}
