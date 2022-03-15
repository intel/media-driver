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
//! \file     media_sysinfo_g8.cpp
//!

#include "igfxfmid.h"
#include "linux_system_info.h"
#include "skuwa_factory.h"
#include "linux_skuwa_debug.h"
#include "linux_media_skuwa.h"
#include "linux_shadow_skuwa.h"

#define GEN8_THREADS_PER_EU 7

static bool InitBdwMediaSysInfo(struct GfxDeviceInfo *devInfo, MEDIA_GT_SYSTEM_INFO *sysInfo)
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
    sysInfo->VEBoxInfo.Instances.Bits.VEBox0Enabled = 1;
    sysInfo->MaxEuPerSubSlice = devInfo->MaxEuPerSubSlice;
    sysInfo->MaxSlicesSupported = sysInfo->SliceCount;
    sysInfo->MaxSubSlicesSupported = sysInfo->SubSliceCount;

    sysInfo->VEBoxInfo.NumberOfVEBoxEnabled = 1;
    sysInfo->VDBoxInfo.NumberOfVDBoxEnabled = 1;
    if (devInfo->eGTType == GTTYPE_GT3)
    {
        sysInfo->VDBoxInfo.Instances.Bits.VDBox1Enabled = 1;
        sysInfo->VEBoxInfo.Instances.Bits.VEBox1Enabled = 1;
        sysInfo->VEBoxInfo.NumberOfVEBoxEnabled++;
        sysInfo->VDBoxInfo.NumberOfVDBoxEnabled++;
    }

    sysInfo->ThreadCount = sysInfo->EUCount * GEN8_THREADS_PER_EU;

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

static bool InitBdwShadowSku(struct GfxDeviceInfo *devInfo,
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
    if ((devInfo->eGTType == GTTYPE_GT3) &&
        drvInfo->hasBsd2)
    {
        skuTable->FtrVcs2 = 1;
    }

    skuTable->FtrULT = 0;

    skuTable->FtrPPGTT = 1;
    skuTable->FtrIA32eGfxPTEs = 1;

    skuTable->FtrEDram = devInfo->hasERAM;

    skuTable->FtrTileY = 1;

    return true;
}

static bool InitBdwShadowWa(struct GfxDeviceInfo *devInfo,
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

    waTable->WaUseVAlign16OnTileXYBpp816 = 1;
    waTable->WaDisregardPlatformChecks = 1;

    return true;
}

static struct GfxDeviceInfo bdwGt1Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_BROADWELL,
    .displayFamily = IGFX_GEN8_CORE,
    .renderFamily  = IGFX_GEN8_CORE,
    .mediaFamily   = IGFX_UNKNOWN_CORE,
    .eGTType       = GTTYPE_GT1,
    .L3CacheSizeInKb = 384,
    .L3BankCount   = 2,
    .EUCount       = 12,
    .SliceCount    = 1,
    .SubSliceCount = 2,
    .MaxEuPerSubSlice = 6,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitBdwMediaSysInfo,
    .InitShadowSku    = InitBdwShadowSku,
    .InitShadowWa     = InitBdwShadowWa,
};

static struct GfxDeviceInfo bdwGt2Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_BROADWELL,
    .displayFamily = IGFX_GEN8_CORE,
    .renderFamily  = IGFX_GEN8_CORE,
    .mediaFamily   = IGFX_UNKNOWN_CORE,
    .eGTType       = GTTYPE_GT2,
    .L3CacheSizeInKb = 768,
    .L3BankCount   = 4,
    .EUCount       = 23,
    .SliceCount    = 1,
    .SubSliceCount = 3,
    .MaxEuPerSubSlice = 8,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitBdwMediaSysInfo,
    .InitShadowSku    = InitBdwShadowSku,
    .InitShadowWa     = InitBdwShadowWa,
};

static struct GfxDeviceInfo bdwGt3Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_BROADWELL,
    .displayFamily = IGFX_GEN8_CORE,
    .renderFamily  = IGFX_GEN8_CORE,
    .mediaFamily   = IGFX_UNKNOWN_CORE,
    .eGTType       = GTTYPE_GT3,
    .L3CacheSizeInKb = 1536,
    .L3BankCount   = 8,
    .EUCount       = 48,
    .SliceCount    = 2,
    .SubSliceCount = 6,
    .MaxEuPerSubSlice = 8,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitBdwMediaSysInfo,
    .InitShadowSku    = InitBdwShadowSku,
    .InitShadowWa     = InitBdwShadowWa,
};

static struct GfxDeviceInfo bdwGt3eInfo = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_BROADWELL,
    .displayFamily = IGFX_GEN8_CORE,
    .renderFamily  = IGFX_GEN8_CORE,
    .mediaFamily   = IGFX_UNKNOWN_CORE,
    .eGTType       = GTTYPE_GT3,
    .L3CacheSizeInKb = 1536,
    .L3BankCount   = 8,
    .EUCount       = 48,
    .SliceCount    = 2,
    .SubSliceCount = 6,
    .MaxEuPerSubSlice = 8,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 1,
    .InitMediaSysInfo = InitBdwMediaSysInfo,
    .InitShadowSku    = InitBdwShadowSku,
    .InitShadowWa     = InitBdwShadowWa,
};

//extern template class DeviceInfoFactory<GfxDeviceInfo>;
typedef DeviceInfoFactory<GfxDeviceInfo> base_fact;

static bool bdwDevice1602 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1602, &bdwGt1Info);

static bool bdwDevice1606 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1606, &bdwGt1Info);

static bool bdwDevice160a = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x160a, &bdwGt1Info);

static bool bdwDevice160d = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x160d, &bdwGt1Info);

static bool bdwDevice160e = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x160e, &bdwGt1Info);

static bool bdwDevice160b = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x160b, &bdwGt1Info);

static bool bdwDevice1612 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1612, &bdwGt2Info);

static bool bdwDevice1616 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1616, &bdwGt2Info);

static bool bdwDevice161a = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x161a, &bdwGt2Info);

static bool bdwDevice161d = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x161d, &bdwGt2Info);

static bool bdwDevice161e = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x161e, &bdwGt2Info);

static bool bdwDevice161b = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x161b, &bdwGt2Info);

static bool bdwDevice1622 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1622, &bdwGt3eInfo);

static bool bdwDevice1626 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1626, &bdwGt3Info);

static bool bdwDevice162b = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x162b, &bdwGt3Info);

static bool bdwDevice162a = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x162a, &bdwGt3eInfo);

static bool bdwDevice162d = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x162d, &bdwGt3Info);

static bool bdwDevice162e = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x162e, &bdwGt3Info);

/* Whether GT4 is needed? */
static bool bdwDevice163b = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x163b, &bdwGt3eInfo);
