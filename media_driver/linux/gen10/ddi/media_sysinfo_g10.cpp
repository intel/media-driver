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
//! \file     media_sysinfo_g10.cpp
//!

#include "igfxfmid.h"
#include "linux_system_info.h"
#include "skuwa_factory.h"
#include "linux_skuwa_debug.h"
#include "linux_media_skuwa.h"
#include "linux_shadow_skuwa.h"

//extern template class DeviceInfoFactory<GfxDeviceInfo>;
typedef DeviceInfoFactory<GfxDeviceInfo> base_fact;

#define GEN10_THREADS_PER_EU        7
#define GEN10_VDBOX2_SUBSLICE_COUNT 3
#define GEN10_VEBOX2_SUBSLICE_COUNT 6

static bool InitGen10MediaSysInfo(struct GfxDeviceInfo *devInfo, MEDIA_GT_SYSTEM_INFO *sysInfo)
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

    if (sysInfo->SubSliceCount >= GEN10_VDBOX2_SUBSLICE_COUNT)
    {
        sysInfo->VDBoxInfo.Instances.Bits.VDBox1Enabled = 1;
        sysInfo->VDBoxInfo.NumberOfVDBoxEnabled ++;
    }

    if (sysInfo->SubSliceCount >= GEN10_VEBOX2_SUBSLICE_COUNT)
    {
        sysInfo->VDBoxInfo.Instances.Bits.VDBox1Enabled = 1;
        sysInfo->VEBoxInfo.NumberOfVEBoxEnabled++;
    }

    sysInfo->ThreadCount = sysInfo->EUCount * GEN10_THREADS_PER_EU;

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

static bool InitGen10ShadowSku(struct GfxDeviceInfo *devInfo,
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
    if (((devInfo->eGTType == GTTYPE_GT3) || (devInfo->eGTType == GTTYPE_GT4)) &&
        drvInfo->hasBsd2)
    {
        skuTable->FtrVcs2 = 1;
    }

    skuTable->FtrULT = 0;

    skuTable->FtrPPGTT = 1;
    skuTable->FtrIA32eGfxPTEs = 1;

    skuTable->FtrDisplayYTiling = 1;
    skuTable->FtrEDram = devInfo->hasERAM;

    skuTable->FtrTileY = 1;

    return true;
}

static bool InitGen10ShadowWa(struct GfxDeviceInfo *devInfo,
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
    waTable->WaFbcLinearSurfaceStride           = 1;

    return true;
}

static struct GfxDeviceInfo cnlGt1Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_CANNONLAKE,
    .displayFamily = IGFX_GEN10_CORE,
    .renderFamily  = IGFX_GEN10_CORE,
    .eGTType       = GTTYPE_GT1,
    .L3CacheSizeInKb = 512,
    .L3BankCount   = 2,
    .EUCount       = 16,
    .SliceCount    = 1,
    .SubSliceCount = 2,
    .MaxEuPerSubSlice = 8,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitGen10MediaSysInfo,
    .InitShadowSku    = InitGen10ShadowSku,
    .InitShadowWa     = InitGen10ShadowWa,
};

static struct GfxDeviceInfo cnlGt1f5Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_CANNONLAKE,
    .displayFamily = IGFX_GEN10_CORE,
    .renderFamily  = IGFX_GEN10_CORE,
    .eGTType       = GTTYPE_GT1_5,
    .L3CacheSizeInKb = 768,
    .L3BankCount   = 3,
    .EUCount       = 24,
    .SliceCount    = 1,
    .SubSliceCount = 3,
    .MaxEuPerSubSlice = 8,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitGen10MediaSysInfo,
    .InitShadowSku    = InitGen10ShadowSku,
    .InitShadowWa     = InitGen10ShadowWa,
};

static struct GfxDeviceInfo cnlGt2Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_CANNONLAKE,
    .displayFamily = IGFX_GEN10_CORE,
    .renderFamily  = IGFX_GEN10_CORE,
    .eGTType       = GTTYPE_GT2,
    .L3CacheSizeInKb = 1536,
    .L3BankCount   = 6,
    .EUCount       = 32,
    .SliceCount    = 2,
    .SubSliceCount = 4,
    .MaxEuPerSubSlice = 8,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitGen10MediaSysInfo,
    .InitShadowSku    = InitGen10ShadowSku,
    .InitShadowWa     = InitGen10ShadowWa,
};

static struct GfxDeviceInfo cnlGt3Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_CANNONLAKE,
    .displayFamily = IGFX_GEN10_CORE,
    .renderFamily  = IGFX_GEN10_CORE,
    .eGTType       = GTTYPE_GT3,
    .L3CacheSizeInKb = 3072,
    .L3BankCount   = 12,
    .EUCount       = 72,
    .SliceCount    = 4,
    .SubSliceCount = 9,
    .MaxEuPerSubSlice = 8,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitGen10MediaSysInfo,
    .InitShadowSku    = InitGen10ShadowSku,
    .InitShadowWa     = InitGen10ShadowWa,
};

static struct GfxDeviceInfo cnlGt3eInfo = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_CANNONLAKE,
    .displayFamily = IGFX_GEN10_CORE,
    .renderFamily  = IGFX_GEN10_CORE,
    .eGTType       = GTTYPE_GT3,
    .L3CacheSizeInKb = 3072,
    .L3BankCount   = 12,
    .EUCount       = 72,
    .SliceCount    = 4,
    .SubSliceCount = 9,
    .MaxEuPerSubSlice = 8,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 1,
    .InitMediaSysInfo = InitGen10MediaSysInfo,
    .InitShadowSku    = InitGen10ShadowSku,
    .InitShadowWa     = InitGen10ShadowWa,
};

static bool cnlDevice5a4a = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5a4a, &cnlGt1Info);

static bool cnlDevice5a42 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5a42, &cnlGt1Info);

static bool cnlDevice5a41 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5a41, &cnlGt1Info);

static bool cnlDevice5a49 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5a49, &cnlGt1Info);

static bool cnlDevice5a44 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5a44, &cnlGt1Info);

static bool cnlDevice5a59 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5a59, &cnlGt1f5Info);

static bool cnlDevice5a5a = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5a5a, &cnlGt1f5Info);

static bool cnlDevice5a5c = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5a5c, &cnlGt1f5Info);

static bool cnlDevice5a50 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5a50, &cnlGt2Info);

static bool cnlDevice5a51 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5a51, &cnlGt2Info);

static bool cnlDevice5a52 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5a52, &cnlGt2Info);

static bool cnlDevice5a54 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5a54, &cnlGt2Info);
