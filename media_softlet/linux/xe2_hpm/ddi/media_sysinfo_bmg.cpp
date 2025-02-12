/*===================== begin_copyright_notice ==================================

Copyright (c) 2024, Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     media_sysinfo_bmg.cpp
//!

#include "igfxfmid.h"
#include "linux_system_info.h"
#include "skuwa_factory.h"
#include "linux_skuwa_debug.h"
#include "linux_media_skuwa.h"
#include "linux_shadow_skuwa.h"
#include "media_user_setting_specific.h"

//extern template class DeviceInfoFactory<GfxDeviceInfo>;
typedef DeviceInfoFactory<GfxDeviceInfo> base_fact;

#define THREADS_NUMBER_PER_EU 7

static bool InitBmgMediaSysInfo(struct GfxDeviceInfo *devInfo, MEDIA_GT_SYSTEM_INFO *sysInfo)
{
    if ((devInfo == nullptr) || (sysInfo == nullptr))
    {
        DEVINFO_ERROR("null ptr is passed\n");
        return false;
    }

    if (!sysInfo->SliceCount)
    {
        sysInfo->SliceCount    = devInfo->SliceCount;
    }

    if (!sysInfo->SubSliceCount)
    {
        sysInfo->SubSliceCount = devInfo->SubSliceCount;
    }

    if (!sysInfo->EUCount)
    {
        sysInfo->EUCount       = devInfo->EUCount;
    }

    sysInfo->L3BankCount                            = devInfo->L3BankCount;
    sysInfo->VEBoxInfo.Instances.Bits.VEBox0Enabled = 1;
    sysInfo->MaxEuPerSubSlice = devInfo->MaxEuPerSubSlice;
    sysInfo->MaxSlicesSupported = sysInfo->SliceCount;
    sysInfo->MaxSubSlicesSupported = sysInfo->SubSliceCount;

    sysInfo->VEBoxInfo.NumberOfVEBoxEnabled = 0; /*Query the VEBox engine info from KMD*/
    sysInfo->VDBoxInfo.NumberOfVDBoxEnabled = 0; /*Query the VDBox engine info from KMD*/

    sysInfo->ThreadCount = sysInfo->EUCount * THREADS_NUMBER_PER_EU;

    sysInfo->VEBoxInfo.IsValid = true;
    sysInfo->VDBoxInfo.IsValid = true;

    //Media driver does not set the other gtsysinfo fileds such as L3CacheSizeInKb, EdramSizeInKb and LLCCacheSizeInKb now.
    //If needed in the future, query them from KMD.

    return true;
}

static bool InitBmgShadowSku(struct GfxDeviceInfo *devInfo,
                             SHADOW_MEDIA_FEATURE_TABLE *skuTable,
                             struct LinuxDriverInfo *drvInfo)
{
    if ((devInfo == nullptr) || (skuTable == nullptr) || (drvInfo == nullptr))
    {
        DEVINFO_ERROR("null ptr is passed\n");
        return false;
    }

    skuTable->FtrVERing = 0;
    if (drvInfo->hasVebox)
    {
       skuTable->FtrVERing = 1;
    }

    skuTable->FtrVcs2 = 0;

    skuTable->FtrULT = 0;

    skuTable->FtrPPGTT = 1;
    skuTable->FtrIA32eGfxPTEs = 1;

    skuTable->FtrDisplayYTiling = 1;
    skuTable->FtrEDram = devInfo->hasERAM;

    bool disableMMC     = false;
    skuTable->FtrE2ECompression = 1;
    skuTable->FtrXe2Compression = 1;
    // Disable MMC for all components if set reg key
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_DISABLE_MMC_ID,
        &userFeatureData,
        (MOS_CONTEXT_HANDLE)nullptr);
    if (userFeatureData.bData)
    {
        disableMMC = true;
    }

    if (disableMMC)
    {
        skuTable->FtrE2ECompression = 0;
        skuTable->FtrXe2Compression = 0;
    }

    skuTable->FtrLinearCCS = 1;
    skuTable->FtrTileY = 0;

    skuTable->FtrLocalMemory = 1;
    skuTable->FtrFlatPhysCCS = 1;

    return true;
}

static bool InitBmgShadowWa(struct GfxDeviceInfo *devInfo,
                             SHADOW_MEDIA_WA_TABLE *waTable,
                             struct LinuxDriverInfo *drvInfo)
{
    if ((devInfo == nullptr) || (waTable == nullptr) || (drvInfo == nullptr))
    {
        DEVINFO_ERROR("null ptr is passed\n");
        return false;
    }

    /* by default PPGTT is enabled */
    waTable->WaForceGlobalGTT = 0;
    if (drvInfo->hasPpgtt == 0)
    {
        waTable->WaForceGlobalGTT = 1;
    }

    waTable->WaDisregardPlatformChecks          = 1;
    waTable->Wa4kAlignUVOffsetNV12LinearSurface = 1;

    //source and recon surfaces need to be aligned to the LCU size
    waTable->WaAlignYUVResourceToLCU = 1;

    return true;
}


static struct GfxDeviceInfo bmgInfo = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_BMG,
    .displayFamily = IGFX_UNKNOWN_CORE,
    .renderFamily  = IGFX_XE2_HPG_CORE,
    .mediaFamily   = IGFX_UNKNOWN_CORE,
    .eGTType       = GTTYPE_GT2,
    .L3CacheSizeInKb = 0,
    .L3BankCount   = 0,
    .EUCount       = 0,
    .SliceCount    = 0,
    .SubSliceCount = 0,
    .MaxEuPerSubSlice = 0,
    .isLCIA        = 0,
    .hasLLC        = 0,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitBmgMediaSysInfo,
    .InitShadowSku    = InitBmgShadowSku,
    .InitShadowWa     = InitBmgShadowWa,
};

static bool bmgDevicee202 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xe202, &bmgInfo);

static bool bmgDevicee20B = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xe20B, &bmgInfo);

static bool bmgDevicee20C = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xe20C, &bmgInfo);

static bool bmgDevicee20D = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xe20D, &bmgInfo);

static bool bmgDevicee212 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xe212, &bmgInfo);

static bool bmgDevicee215 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xe215, &bmgInfo);

static bool bmgDevicee216 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xe216, &bmgInfo);