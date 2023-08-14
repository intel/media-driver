/*
* Copyright (c) 2017-2023, Intel Corporation
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
//! \file        hwinfo_linux.c 
//! \brief     The purpose of the file is to get the sku/wa table according to platform information. 
//!

#include "hwinfo_linux.h"
#include "mos_utilities.h"
#include "mos_util_debug.h"
#include "skuwa_factory.h"
#include "linux_system_info.h"
#include "linux_shadow_skuwa.h"
#include "mos_solo_generic.h"
#include "media_user_setting_specific.h"

typedef DeviceInfoFactory<struct GfxDeviceInfo> DeviceInfoFact;
typedef DeviceInfoFactory<struct LinuxDeviceInit> DeviceInitFact;

static GfxDeviceInfo *getDeviceInfo(uint32_t devId)
{
    GfxDeviceInfo *devInfo = DeviceInfoFact::LookupDevice(devId);

    return devInfo;
}

static LinuxDeviceInit *getDeviceInit(uint32_t platKey)
{
    LinuxDeviceInit *devInfo = DeviceInitFact::LookupDevice(platKey);

    return devInfo;
}

/*****************************************************************************\
Description:
    Get ProductFamily according to input device FD

Input:
    fd              - file descriptor to the /dev/dri/cardX
Output:
    eProductFamily  - describing current platform product family
\*****************************************************************************/
MOS_STATUS HWInfo_GetGfxProductFamily(int32_t fd, PRODUCT_FAMILY &eProductFamily)
{
    if (fd < 0)
    {
        MOS_OS_ASSERTMESSAGE("Invalid parameter \n");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    uint32_t devId = 23172;
    if (mos_get_device_id(fd, &devId))
    {
        MOS_OS_ASSERTMESSAGE("Failed to get the chipset id\n");
        return MOS_STATUS_INVALID_HANDLE;
    }
    GfxDeviceInfo *devInfo = getDeviceInfo(devId);
    if (devInfo == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to get the device info for Device id: %x\n", devId);
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }
    eProductFamily = (PRODUCT_FAMILY)devInfo->productFamily;
    return MOS_STATUS_SUCCESS;
}


/*****************************************************************************\
Description:
    Get Sku/Wa tables and platform information according to input device FD

Input:
    fd         - file descriptor to the /dev/dri/cardX
Output:
    gfxPlatform  - describing current platform. is it mobile, desk,
                   server? Sku/Wa must know.
    skuTable     - describing SKU
    waTable      - the constraints list
    gtSystemInfo - describing current system information
    userSettingPtr - shared pointer to user setting instance
\*****************************************************************************/
MOS_STATUS HWInfo_GetGfxInfo(int32_t           fd,
                          MOS_BUFMGR           *pDrmBufMgr,
                          PLATFORM             *gfxPlatform,
                          MEDIA_FEATURE_TABLE  *skuTable,
                          MEDIA_WA_TABLE       *waTable,
                          MEDIA_SYSTEM_INFO    *gtSystemInfo,
                          MediaUserSettingSharedPtr userSettingPtr)
{
    if ((fd < 0) ||
        (pDrmBufMgr == nullptr) ||
        (gfxPlatform == nullptr) ||
        (skuTable == nullptr) ||
        (waTable == nullptr) ||
        (gtSystemInfo == nullptr))
    {
        MOS_OS_ASSERTMESSAGE("Invalid parameter \n");
        return MOS_STATUS_INVALID_PARAMETER;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_VALUE_DATA         UserFeatureData;
#endif

    LinuxDriverInfo drvInfo = {18, 3, 0, 23172, 3, 1, 0, 1, 0, 0, 1, 0, 0};
    if (!Mos_Solo_IsEnabled(nullptr) && mos_get_driver_info(pDrmBufMgr, &drvInfo))
    {
        MOS_OS_ASSERTMESSAGE("Failed to get the chipset id\n");
        return MOS_STATUS_INVALID_HANDLE;
    }

    GfxDeviceInfo *devInfo = getDeviceInfo(drvInfo.devId);
    if (devInfo == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to get the device info for Device id: %x\n", drvInfo.devId);
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }
    /* Initialize Platform Info */
    gfxPlatform->ePlatformType      = (PLATFORM_TYPE)devInfo->platformType;
    gfxPlatform->eProductFamily     = (PRODUCT_FAMILY)devInfo->productFamily;
    gfxPlatform->ePCHProductFamily  = PCH_UNKNOWN;
    gfxPlatform->eDisplayCoreFamily = (GFXCORE_FAMILY)devInfo->displayFamily;
    gfxPlatform->eRenderCoreFamily  = (GFXCORE_FAMILY)devInfo->renderFamily;
    gfxPlatform->eGTType            = (GTTYPE)devInfo->eGTType;
    gfxPlatform->usDeviceID         = drvInfo.devId;
    gfxPlatform->usRevId            = drvInfo.devRev;

    if (mos_query_device_blob(pDrmBufMgr, gtSystemInfo) == 0)
    {
        gtSystemInfo->EUCount           = gtSystemInfo->SubSliceCount * gtSystemInfo->MaxEuPerSubSlice;
        gtSystemInfo->ThreadCount       = gtSystemInfo->EUCount * gtSystemInfo->NumThreadsPerEu;
        gtSystemInfo->VEBoxInfo.IsValid = true;
        gtSystemInfo->VDBoxInfo.IsValid = true;
    }
    else
    {
        MOS_OS_NORMALMESSAGE("Device blob query is not supported yet.\n");
        gtSystemInfo->SliceCount        = drvInfo.sliceCount;
        gtSystemInfo->SubSliceCount     = drvInfo.subSliceCount;
        gtSystemInfo->EUCount           = drvInfo.euCount;

        if (devInfo->InitMediaSysInfo &&
            devInfo->InitMediaSysInfo(devInfo, gtSystemInfo))
        {
            MOS_OS_NORMALMESSAGE("Init Media SystemInfo\n");
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Failed to Init Gt System Info\n");
            return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
        }
    }

    if (mos_query_sys_engines(pDrmBufMgr, gtSystemInfo))
    {
        MOS_OS_ASSERTMESSAGE("Failed to Init Gt System Info\n");
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    uint32_t platformKey = devInfo->productFamily;
    LinuxDeviceInit *devInit = getDeviceInit(platformKey);

    if (devInit && devInit->InitMediaFeature &&
        devInit->InitMediaWa &&
        devInit->InitMediaFeature(devInfo, skuTable, &drvInfo, userSettingPtr) &&
        devInit->InitMediaWa(devInfo, waTable, &drvInfo))
    {
#ifdef _MEDIA_RESERVED
        MOS_OS_NORMALMESSAGE("Init Media SKU/WA info successfully\n");
        if (MEDIA_IS_SKU(skuTable, FtrMediaIPSeparation))
        {
            gfxPlatform->eMediaCoreFamily = (GFXCORE_FAMILY)devInfo->mediaFamily;

            if (0 == gfxPlatform->sMediaBlockID.Value)
            {
                if (mos_query_hw_ip_version(pDrmBufMgr, DRM_ENGINE_CLASS_VIDEO_DECODE, (void *)&(gfxPlatform->sMediaBlockID)))
                {
                    MOS_OS_ASSERTMESSAGE("Failed to query vdbox engine GmdID\n");
                    return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
                }
                else
                {
                    MOS_OS_NORMALMESSAGE("Media mediaRevID arch:%d, release:%d, RevisionID:%d\n", gfxPlatform->sMediaBlockID.GmdID.GMDArch,
                                        gfxPlatform->sMediaBlockID.GmdID.GMDRelease, gfxPlatform->sMediaBlockID.GmdID.RevisionID);
                }
            }

            if (0 == gfxPlatform->sRenderBlockID.Value)
            {
                if (mos_query_hw_ip_version(pDrmBufMgr, DRM_ENGINE_CLASS_RENDER, (void *)&(gfxPlatform->sRenderBlockID)))
                {
                    MOS_OS_ASSERTMESSAGE("Failed to query render GmdID\n");
                    return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
                }

                else
                {
                    MOS_OS_NORMALMESSAGE("Media sRenderBlockID arch:%d, release:%d, RevisionID:%d\n", gfxPlatform->sRenderBlockID.GmdID.GMDArch,
                                        gfxPlatform->sRenderBlockID.GmdID.GMDRelease, gfxPlatform->sRenderBlockID.GmdID.RevisionID);
                }
            }
        }
#endif
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Failed to Init SKU/WA Info\n");
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    uint32_t devExtKey = platformKey + MEDIA_EXT_FLAG;
    LinuxDeviceInit *devExtInit = getDeviceInit(devExtKey);

    /* The initializationof Ext SKU/WA is optional. So skip the check of return value */
    if (devExtInit && devExtInit->InitMediaFeature &&
        devExtInit->InitMediaWa &&
        devExtInit->InitMediaFeature(devInfo, skuTable, &drvInfo, userSettingPtr) &&
        devExtInit->InitMediaWa(devInfo, waTable, &drvInfo))
    {
        MOS_OS_NORMALMESSAGE("Init Media SystemInfo successfully\n");
    }
    if (drvInfo.isServer)
    {
        mos_set_platform_information(pDrmBufMgr, PLATFORM_INFORMATION_IS_SERVER);
    }

    /* disable it on Linux */
    MEDIA_WR_SKU(skuTable, FtrPerCtxtPreemptionGranularityControl, 0);
    MEDIA_WR_SKU(skuTable, FtrMediaThreadGroupLevelPreempt, 0);
    MEDIA_WR_SKU(skuTable, FtrMediaMidBatchPreempt, 0);
    MEDIA_WR_SKU(skuTable, FtrMediaMidThreadLevelPreempt, 0);
    MEDIA_WR_SKU(skuTable, FtrGpGpuThreadGroupLevelPreempt, 0);
    MEDIA_WR_SKU(skuTable, FtrGpGpuMidBatchPreempt, 0);
    MEDIA_WR_SKU(skuTable, FtrGpGpuMidThreadLevelPreempt, 0);

#if (_DEBUG || _RELEASE_INTERNAL)
    // User feature read to detect if simulation environment (MediaSolo) is enabled
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_ENABLE_ID,
        &UserFeatureData,
        (MOS_CONTEXT_HANDLE)nullptr);
    //Disable plarform checks to support pre-si features (e.g., MMCD) on MediaSolo
    MEDIA_WR_WA(waTable, WaDisregardPlatformChecks, (UserFeatureData.i32Data) ? true : false);
#endif

    return MOS_STATUS_SUCCESS;
}

/*****************************************************************************\
Description:
    Get the required Sku/Wa tables for GMM and platform information  based on device Fd

Input:
    fd         - file descriptor to the /dev/dri/cardX
Output:
     SHADOW_MEDIA_FEATURE_TABLE  *shadowSkuTable
     SHADOW_MEDIA_WA_TABLE      *shadowWaTable,
     MEDIA_SYSTEM_INFO       *systemInfo
\*****************************************************************************/
MOS_STATUS HWInfo_GetGmmInfo(MOS_BUFMGR              *pDrmBufMgr,
                          SHADOW_MEDIA_FEATURE_TABLE *shadowSkuTable,
                          SHADOW_MEDIA_WA_TABLE      *shadowWaTable,
                          MEDIA_SYSTEM_INFO          *systemInfo)
{
    if ((pDrmBufMgr == nullptr) ||
        (shadowSkuTable == nullptr) ||
        (shadowWaTable == nullptr) ||
        (systemInfo == nullptr))
    {
        MOS_OS_ASSERTMESSAGE("Invalid parameter \n");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    LinuxDriverInfo drvInfo = {18, 3, 0, 23172, 3, 1, 0, 1, 0, 0, 1, 0};

    if (!Mos_Solo_IsEnabled(nullptr) && mos_get_driver_info(pDrmBufMgr, &drvInfo))
    {
        MOS_OS_ASSERTMESSAGE("Failed to get the chipset id\n");
        return MOS_STATUS_INVALID_HANDLE;
    }

    GfxDeviceInfo *devInfo = getDeviceInfo(drvInfo.devId);
    if (devInfo == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to get the device info for Device id: %x\n", drvInfo.devId);
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    if (devInfo->InitMediaSysInfo &&
        devInfo->InitShadowSku &&
        devInfo->InitShadowWa &&
        devInfo->InitMediaSysInfo(devInfo, systemInfo) &&
        devInfo->InitShadowSku(devInfo, shadowSkuTable, &drvInfo) &&
        devInfo->InitShadowWa(devInfo, shadowWaTable, &drvInfo))
    {
        MOS_OS_NORMALMESSAGE("Init Media SystemInfo successfully\n");
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Failed to Init Media System Info\n");
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    return MOS_STATUS_SUCCESS;
}

