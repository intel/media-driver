/*
* Copyright (c) 2017-2019, Intel Corporation
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

static bool MediaGetParam(int fd, int32_t param, uint32_t *retValue)
{
    struct drm_i915_getparam gp;

    gp.param = param;
    gp.value = (int32_t *)retValue;
    return drmIoctl(fd, DRM_IOCTL_I915_GETPARAM, &gp) == 0;
}

// Currently all Gen HW has 7 threads per EU. Redefine when this changes.
#define GEN_HW_THREADS_PER_EU       7

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
    LinuxDriverInfo drvInfo = {18, 3, 0, 23172, 3, 1, 0, 1, 0, 0, 1, 0};
    if (HWInfoGetLinuxDrvInfo(fd, &drvInfo) != MOS_STATUS_SUCCESS)
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
\*****************************************************************************/
MOS_STATUS HWInfo_GetGfxInfo(int32_t           fd,
                          PLATFORM             *gfxPlatform,
                          MEDIA_FEATURE_TABLE  *skuTable,
                          MEDIA_WA_TABLE       *waTable,
                          MEDIA_SYSTEM_INFO    *gtSystemInfo)
{
    if ((fd < 0) ||
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

    LinuxDriverInfo drvInfo = {18, 3, 0, 23172, 3, 1, 0, 1, 0, 0, 1, 0};
    if (!Mos_Solo_IsEnabled() && HWInfoGetLinuxDrvInfo(fd, &drvInfo) != MOS_STATUS_SUCCESS)
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

    if (gtSystemInfo->VDBoxInfo.NumberOfVDBoxEnabled == 0)
    {
        unsigned int nengine = MAX_ENGINE_INSTANCE_NUM;
        struct i915_engine_class_instance uengines[MAX_ENGINE_INSTANCE_NUM];
        if (mos_query_engines(fd,I915_ENGINE_CLASS_VIDEO,0,&nengine,uengines) == 0)
        {
            gtSystemInfo->VDBoxInfo.NumberOfVDBoxEnabled = nengine;
        }
        for (int i=0; i<nengine; i++)
        {
            gtSystemInfo->VDBoxInfo.Instances.VDBoxEnableMask |= 1<<uengines[i].engine_instance;
        }
    }

    if (gtSystemInfo->VEBoxInfo.NumberOfVEBoxEnabled == 0)
    {
        unsigned int nengine = MAX_ENGINE_INSTANCE_NUM;
        struct i915_engine_class_instance uengines[MAX_ENGINE_INSTANCE_NUM];
        if (mos_query_engines(fd,I915_ENGINE_CLASS_VIDEO_ENHANCE,0,&nengine,uengines) == 0)
        {
            MOS_OS_ASSERT(nengine <= MAX_ENGINE_INSTANCE_NUM);
            gtSystemInfo->VEBoxInfo.NumberOfVEBoxEnabled = nengine;
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Failed to query vebox engine\n");
            return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
        }
    }

    uint32_t platformKey = devInfo->productFamily;
    LinuxDeviceInit *devInit = getDeviceInit(platformKey);

    if (devInit && devInit->InitMediaFeature &&
        devInit->InitMediaWa &&
        devInit->InitMediaFeature(devInfo, skuTable, &drvInfo) &&
        devInit->InitMediaWa(devInfo, waTable, &drvInfo))
    {
        MOS_OS_NORMALMESSAGE("Init Media SKU/WA info successfully\n");
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
        devExtInit->InitMediaFeature(devInfo, skuTable, &drvInfo) &&
        devExtInit->InitMediaWa(devInfo, waTable, &drvInfo))
    {
        MOS_OS_NORMALMESSAGE("Init Media SystemInfo successfully\n");
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
        &UserFeatureData);
    //Disable plarform checks to support pre-si features (e.g., MMCD) on MediaSolo
    MEDIA_WR_WA(waTable, WaDisregardPlatformChecks, (UserFeatureData.i32Data) ? true : false);
#endif

    return MOS_STATUS_SUCCESS;
}

#ifndef I915_PARAM_HAS_BSD2
#define LOCAL_I915_PARAM_HAS_BSD2 I915_PARAM_HAS_BSD2
#else
#define LOCAL_I915_PARAM_HAS_BSD2 31
#endif

#ifdef I915_PARAM_HUC_STATUS
#define LOCAL_I915_PARAM_HAS_HUC I915_PARAM_HUC_STATUS
#else
#define LOCAL_I915_PARAM_HAS_HUC 42
#endif

#ifdef I915_PARAM_EU_TOTAL
#define LOCAL_I915_PARAM_EU_TOTAL I915_PARAM_EU_TOTAL
#else
#define LOCAL_I915_PARAM_EU_TOTAL 34
#endif

#ifdef I915_PARAM_SUBSLICE_TOTAL
#define LOCAL_I915_PARAM_SUBSLICE I915_PARAM_SUBSLICE_TOTAL
#else
#define LOCAL_I915_PARAM_SUBSLICE 33
#endif

#ifdef I915_PARAM_REVISION
#define LOCAL_I915_PARAM_REVISION I915_PARAM_REVISION
#else
#define LOCAL_I915_PARAM_REVISION 32
#endif

MOS_STATUS HWInfoGetLinuxDrvInfo(int fd, struct LinuxDriverInfo *drvInfo)
{
    if ((fd < 0) || (drvInfo == nullptr))
    {
        return MOS_STATUS_INVALID_HANDLE;
    }

    uint32_t retValue = 0;

    drvInfo->hasBsd = 0;
    if (MediaGetParam(fd, I915_PARAM_HAS_BSD, &retValue))
    {
        drvInfo->hasBsd = !!retValue;
    }

    drvInfo->hasBsd2 = 0;
    retValue = 0;
    if (MediaGetParam(fd, LOCAL_I915_PARAM_HAS_BSD2, &retValue))
    {
        drvInfo->hasBsd2 = !!retValue;
    }

    drvInfo->hasVebox = 0;
    retValue = 0;
    if (MediaGetParam(fd, I915_PARAM_HAS_VEBOX, &retValue))
    {
        drvInfo->hasVebox = !!retValue;
    }

    drvInfo->hasPpgtt = 1;
    retValue = 0;
    if (MediaGetParam(fd, I915_PARAM_HAS_ALIASING_PPGTT, &retValue))
    {
        drvInfo->hasPpgtt = !!retValue;
    }

    drvInfo->hasHuc = 0;
    retValue = 0;
    if (MediaGetParam(fd, LOCAL_I915_PARAM_HAS_HUC, &retValue))
    {
        drvInfo->hasHuc = !!retValue;
    }

    drvInfo->devId = 0;
    retValue = 0;
    if (MediaGetParam(fd, I915_PARAM_CHIPSET_ID, &retValue))
    {
        drvInfo->devId = retValue;
    }
    drvInfo->devRev = 0;
    retValue = 0;
    if (MediaGetParam(fd, LOCAL_I915_PARAM_REVISION, &retValue))
    {
        drvInfo->devRev = retValue;
    }

    drvInfo->euCount = 0;
    retValue = 0;
    if (MediaGetParam(fd, LOCAL_I915_PARAM_EU_TOTAL, &retValue))
    {
        drvInfo->euCount = retValue;
    }

    drvInfo->subSliceCount = 0;
    retValue = 0;
    if (MediaGetParam(fd, LOCAL_I915_PARAM_SUBSLICE, &retValue))
    {
        drvInfo->subSliceCount = retValue;
    }

    // There is no interface to read total slice count from drm/i915, so we
    // will set the slice count in InitMediaSysInfo accordint to Device ID.
    drvInfo->sliceCount = 0;

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
MOS_STATUS HWInfo_GetGmmInfo(int32_t                 fd,
                          SHADOW_MEDIA_FEATURE_TABLE *shadowSkuTable,
                          SHADOW_MEDIA_WA_TABLE      *shadowWaTable,
                          MEDIA_SYSTEM_INFO          *systemInfo)
{
    if ((fd < 0) ||
        (shadowSkuTable == nullptr) ||
        (shadowWaTable == nullptr) ||
        (systemInfo == nullptr))
    {
        MOS_OS_ASSERTMESSAGE("Invalid parameter \n");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    LinuxDriverInfo drvInfo = {18, 3, 0, 23172, 3, 1, 0, 1, 0, 0, 1, 0};
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_VALUE_DATA         UserFeatureData;
#endif

    if (!Mos_Solo_IsEnabled() && HWInfoGetLinuxDrvInfo(fd, &drvInfo) != MOS_STATUS_SUCCESS)
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

