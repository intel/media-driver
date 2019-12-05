/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     media_sysinfo_g11.cpp
//!

#include "igfxfmid.h"
#include "linux_system_info.h"
#include "skuwa_factory.h"
#include "linux_skuwa_debug.h"
#include "linux_media_skuwa.h"
#include "linux_shadow_skuwa.h"

//extern template class DeviceInfoFactory<GfxDeviceInfo>;
typedef DeviceInfoFactory<GfxDeviceInfo> base_fact;

#define GEN11_THREADS_PER_EU        7
#define GEN11_VDBOX4_SUBSLICE_COUNT 4
#define GEN11_VEBOX2_SUBSLICE_COUNT 4

static bool InitIclShadowSku(struct GfxDeviceInfo *devInfo,
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

    skuTable->FtrTileY = 1;

    return true;
}

static bool InitIclShadowWa(struct GfxDeviceInfo *devInfo,
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

    return true;
}

static bool InitIcllpMediaSysInfo(struct GfxDeviceInfo *devInfo, MEDIA_GT_SYSTEM_INFO *sysInfo)
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

    sysInfo->L3CacheSizeInKb = devInfo->L3CacheSizeInKb;
    sysInfo->L3BankCount     = devInfo->L3BankCount;
    sysInfo->VDBoxInfo.Instances.Bits.VDBox0Enabled = 1;
    sysInfo->VDBoxInfo.Instances.Bits.VDBox1Enabled = 0;
    sysInfo->VEBoxInfo.Instances.Bits.VEBox0Enabled = 1;
    sysInfo->MaxEuPerSubSlice = devInfo->MaxEuPerSubSlice;
    sysInfo->MaxSlicesSupported = sysInfo->SliceCount;
    sysInfo->MaxSubSlicesSupported = sysInfo->SubSliceCount;

    sysInfo->VEBoxInfo.NumberOfVEBoxEnabled = 1;
    sysInfo->VDBoxInfo.NumberOfVDBoxEnabled = 0;//Query the VDBox number from KMD

    sysInfo->ThreadCount = sysInfo->EUCount * GEN11_THREADS_PER_EU;

    sysInfo->VEBoxInfo.IsValid = true;
    sysInfo->VDBoxInfo.IsValid = true;

    /* the GMM doesn't care the real size of ERAM/LLC. Instead it is used to
     * indicate whether the LLC/ERAM exists
     */
    if (devInfo->hasERAM)
    {
        // 64M
        sysInfo->EdramSizeInKb = 64 * 1024;
    }
    if (devInfo->hasLLC)
    {
        // 2M
        sysInfo->LLCCacheSizeInKb = 2 * 1024;
    }

    return true;
}

static bool InitEhlShadowSku(struct GfxDeviceInfo *devInfo,
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
    skuTable->FtrEDram          = 0;
    skuTable->FtrLLCBypass      = 1;

    return true;
}
static bool InitEhlShadowWa(struct GfxDeviceInfo *devInfo,
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

    return true;
}

static bool InitEhlMediaSysInfo(struct GfxDeviceInfo *devInfo, MEDIA_GT_SYSTEM_INFO *sysInfo)
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

    sysInfo->L3CacheSizeInKb = devInfo->L3CacheSizeInKb;
    sysInfo->L3BankCount     = devInfo->L3BankCount;
    /* EHL has only one VDBox */
    sysInfo->VDBoxInfo.Instances.Bits.VDBox0Enabled = 1;
    sysInfo->VDBoxInfo.Instances.Bits.VDBox1Enabled = 0;
    sysInfo->VEBoxInfo.Instances.Bits.VEBox0Enabled = 1;
    sysInfo->MaxEuPerSubSlice = devInfo->MaxEuPerSubSlice;
    sysInfo->MaxSlicesSupported = sysInfo->SliceCount;
    sysInfo->MaxSubSlicesSupported = sysInfo->SubSliceCount;

    sysInfo->VEBoxInfo.NumberOfVEBoxEnabled = 1;
    sysInfo->VDBoxInfo.NumberOfVDBoxEnabled = 1;

    sysInfo->ThreadCount = sysInfo->EUCount * GEN11_THREADS_PER_EU;

    sysInfo->VEBoxInfo.IsValid = true;
    sysInfo->VDBoxInfo.IsValid = true;

    /* the GMM doesn't care the real size of ERAM/LLC. Instead it is used to
     * indicate whether the LLC/ERAM exists
     */
    if (devInfo->hasERAM)
    {
        // 64M
        sysInfo->EdramSizeInKb = 64 * 1024;
    }
    if (devInfo->hasLLC)
    {
        // 2M
        sysInfo->LLCCacheSizeInKb = 2 * 1024;
    }

    return true;
}

static struct GfxDeviceInfo icllpGt1Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_ICELAKE_LP,
    .displayFamily = IGFX_GEN11_CORE,
    .renderFamily  = IGFX_GEN11_CORE,
    .eGTType       = GTTYPE_GT1,
    .L3CacheSizeInKb = 2304,
    .L3BankCount   = 6,
    .EUCount       = 48,
    .SliceCount    = 1,
    .SubSliceCount = 6,
    .MaxEuPerSubSlice = 8,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitIcllpMediaSysInfo,
    .InitShadowSku    = InitIclShadowSku,
    .InitShadowWa     = InitIclShadowWa,
};

static struct GfxDeviceInfo icllpGt05Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_ICELAKE_LP,
    .displayFamily = IGFX_GEN11_CORE,
    .renderFamily  = IGFX_GEN11_CORE,
    .eGTType       = GTTYPE_GT1,
    .L3CacheSizeInKb = 2304,
    .L3BankCount   = 6,
    .EUCount       = 8,
    .SliceCount    = 1,
    .SubSliceCount = 1,
    .MaxEuPerSubSlice = 8,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitIcllpMediaSysInfo,
    .InitShadowSku    = InitIclShadowSku,
    .InitShadowWa     = InitIclShadowWa,
};
static struct GfxDeviceInfo icllpGt2Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_ICELAKE_LP,
    .displayFamily = IGFX_GEN11_CORE,
    .renderFamily  = IGFX_GEN11_CORE,
    .eGTType       = GTTYPE_GT2,
    .L3CacheSizeInKb = 3072,
    .L3BankCount   = 8,
    .EUCount       = 64,
    .SliceCount    = 1,
    .SubSliceCount = 8,
    .MaxEuPerSubSlice = 8,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitIcllpMediaSysInfo,
    .InitShadowSku    = InitIclShadowSku,
    .InitShadowWa     = InitIclShadowWa,
};
static struct GfxDeviceInfo ehlDevInfo = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_ELKHARTLAKE,
    .displayFamily = IGFX_GEN11_CORE,
    .renderFamily  = IGFX_GEN11_CORE,
    .eGTType       = GTTYPE_GT1,
    .L3CacheSizeInKb = 1280,
    .L3BankCount   = 4,
    .EUCount       = 32,
    .SliceCount    = 1,
    .SubSliceCount = 4,
    .MaxEuPerSubSlice = 8,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitEhlMediaSysInfo,
    .InitShadowSku    = InitEhlShadowSku,
    .InitShadowWa     = InitEhlShadowWa,
};

static bool icllpDeviceff05 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xff05, &icllpGt1Info);

static bool icllpDevice8a50 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x8a50, &icllpGt2Info);

static bool icllpDevice8a51 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x8a51, &icllpGt2Info);

static bool icllpDevice8a52 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x8a52, &icllpGt2Info);

static bool icllpDevice8a53 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x8a53, &icllpGt2Info);

static bool icllpDevice8a56 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x8a56, &icllpGt1Info);

static bool icllpDevice8a57 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x8a57, &icllpGt1Info);

static bool icllpDevice8a58 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x8a58, &icllpGt1Info);

static bool icllpDevice8a59 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x8a59, &icllpGt1Info);

static bool icllpDevice8a5d = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x8a5d, &icllpGt1Info);

static bool icllpDevice8a5c = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x8a5c, &icllpGt1Info);

static bool icllpDevice8a5b = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x8a5b, &icllpGt1Info);

static bool icllpDevice8a5a = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x8a5a, &icllpGt1Info);

static bool icllpDevice8a71 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x8a71, &icllpGt05Info);

static bool ehlDevice4500 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4500, &ehlDevInfo);

static bool ehlDevice4571 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4571, &ehlDevInfo);

static bool ehlDevice4551 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4551, &ehlDevInfo);

static bool ehlDevice4541 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4541, &ehlDevInfo);

static bool ehlDevice4E51 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4E51, &ehlDevInfo);

static bool ehlDevice4E61 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4E61, &ehlDevInfo);

static bool ehlDevice4E71 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4E71, &ehlDevInfo);
