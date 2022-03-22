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
//! \file     media_sysinfo_g9.cpp
//!

#include "igfxfmid.h"
#include "linux_system_info.h"
#include "skuwa_factory.h"
#include "linux_skuwa_debug.h"
#include "linux_media_skuwa.h"
#include "linux_shadow_skuwa.h"

//extern template class DeviceInfoFactory<GfxDeviceInfo>;
typedef DeviceInfoFactory<GfxDeviceInfo> base_fact;

#define GEN9_THREADS_PER_EU      7
#define GEN9_LCIA_THREADS_PER_EU 6
static bool InitGen9MediaSysInfo(struct GfxDeviceInfo *devInfo, MEDIA_GT_SYSTEM_INFO *sysInfo)
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
    if ((devInfo->eGTType == GTTYPE_GT3) ||
        (devInfo->eGTType == GTTYPE_GT4))
    {
        sysInfo->VDBoxInfo.Instances.Bits.VDBox1Enabled = 1;
        sysInfo->VEBoxInfo.Instances.Bits.VEBox1Enabled = 1;
        sysInfo->VEBoxInfo.NumberOfVEBoxEnabled++;
        sysInfo->VDBoxInfo.NumberOfVDBoxEnabled++;
    }

    sysInfo->ThreadCount = sysInfo->EUCount * GEN9_THREADS_PER_EU;

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

    if ((devInfo->eGTType == GTTYPE_GT4) && devInfo->hasERAM)
    {
        // 128M on GT4e
        sysInfo->EdramSizeInKb = 128 * 1024;
    }

    return true;
}

static bool InitGen9ShadowSku(struct GfxDeviceInfo *devInfo,
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

    return true;
}

static bool InitGen9ShadowWa(struct GfxDeviceInfo *devInfo,
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
    waTable->WaLosslessCompressionSurfaceStride = 1;
    waTable->WaFbcLinearSurfaceStride           = 1;
    waTable->Wa4kAlignUVOffsetNV12LinearSurface = 1;

    if (devInfo->productFamily == IGFX_SKYLAKE)
    {
        waTable->WaEncryptedEdramOnlyPartials = 1;
        waTable->WaDisableEdramForDisplayRT   = 1;
    }
    else
    if (devInfo->productFamily == IGFX_COFFEELAKE)
    {
        waTable->WaEncryptedEdramOnlyPartials = 1;
    }

    return true;
}

static bool InitLCIAMediaSysInfo(struct GfxDeviceInfo *devInfo, MEDIA_GT_SYSTEM_INFO *sysInfo)
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

    sysInfo->ThreadCount = sysInfo->EUCount * GEN9_LCIA_THREADS_PER_EU;

    sysInfo->VEBoxInfo.IsValid = true;
    sysInfo->VDBoxInfo.IsValid = true;

    sysInfo->EdramSizeInKb    = 0;
    sysInfo->LLCCacheSizeInKb = 0;

    return true;
}

static bool InitLCIAShadowSku(struct GfxDeviceInfo *devInfo,
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
    skuTable->FtrULT = 1;
    skuTable->FtrPPGTT = 1;
    skuTable->FtrIA32eGfxPTEs = 1;

    skuTable->FtrDisplayYTiling = 1;
    skuTable->FtrEDram = 0;
    skuTable->FtrLCIA  = 1;

    skuTable->FtrTileY = 1;

    return true;
}

static bool InitLCIAShadowWa(struct GfxDeviceInfo *devInfo,
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
    waTable->WaFbcLinearSurfaceStride           = 1;
    waTable->Wa4kAlignUVOffsetNV12LinearSurface = 1;
    waTable->WaLLCCachingUnsupported            = 1;

    return true;
}

static struct GfxDeviceInfo sklGt1Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_SKYLAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
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
    .InitMediaSysInfo = InitGen9MediaSysInfo,
    .InitShadowSku    = InitGen9ShadowSku,
    .InitShadowWa     = InitGen9ShadowWa,
};

static struct GfxDeviceInfo sklGt1f5Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_SKYLAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
    .eGTType       = GTTYPE_GT1_5,
    .L3CacheSizeInKb = 768,
    .L3BankCount   = 4,
    .EUCount       = 18,
    .SliceCount    = 1,
    .SubSliceCount = 3,
    .MaxEuPerSubSlice = 6,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitGen9MediaSysInfo,
    .InitShadowSku    = InitGen9ShadowSku,
    .InitShadowWa     = InitGen9ShadowWa,
};

static struct GfxDeviceInfo sklGt2Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_SKYLAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
    .eGTType       = GTTYPE_GT2,
    .L3CacheSizeInKb = 768,
    .L3BankCount   = 4,
    .EUCount       = 24,
    .SliceCount    = 1,
    .SubSliceCount = 3,
    .MaxEuPerSubSlice = 8,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitGen9MediaSysInfo,
    .InitShadowSku    = InitGen9ShadowSku,
    .InitShadowWa     = InitGen9ShadowWa,
};

static struct GfxDeviceInfo sklGt3Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_SKYLAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
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
    .InitMediaSysInfo = InitGen9MediaSysInfo,
    .InitShadowSku    = InitGen9ShadowSku,
    .InitShadowWa     = InitGen9ShadowWa,
};

static struct GfxDeviceInfo sklGt3eInfo = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_SKYLAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
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
    .InitMediaSysInfo = InitGen9MediaSysInfo,
    .InitShadowSku    = InitGen9ShadowSku,
    .InitShadowWa     = InitGen9ShadowWa,
};

static struct GfxDeviceInfo sklGt4Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_SKYLAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
    .eGTType       = GTTYPE_GT4,
    .L3CacheSizeInKb = 2304,
    .L3BankCount   = 12,
    .EUCount       = 72,
    .SliceCount    = 3,
    .SubSliceCount = 9,
    .MaxEuPerSubSlice = 8,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitGen9MediaSysInfo,
    .InitShadowSku    = InitGen9ShadowSku,
    .InitShadowWa     = InitGen9ShadowWa,
};

static struct GfxDeviceInfo sklGt4eInfo = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_SKYLAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
    .eGTType       = GTTYPE_GT4,
    .L3CacheSizeInKb = 2304,
    .L3BankCount   = 12,
    .EUCount       = 72,
    .SliceCount    = 3,
    .SubSliceCount = 9,
    .MaxEuPerSubSlice = 8,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 1,
    .InitMediaSysInfo = InitGen9MediaSysInfo,
    .InitShadowSku    = InitGen9ShadowSku,
    .InitShadowWa     = InitGen9ShadowWa,
};

static struct GfxDeviceInfo bxtGt1Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_BROXTON,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
    .eGTType       = GTTYPE_GT1,
    .L3CacheSizeInKb = 384,
    .L3BankCount   = 2,
    .EUCount       = 12,
    .SliceCount    = 1,
    .SubSliceCount = 2,
    .MaxEuPerSubSlice = 6,
    .isLCIA        = 1,
    .hasLLC        = 0,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitLCIAMediaSysInfo,
    .InitShadowSku    = InitLCIAShadowSku,
    .InitShadowWa     = InitLCIAShadowWa,
};

static struct GfxDeviceInfo bxtGt1f5Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_BROXTON,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
    .eGTType       = GTTYPE_GT1_5,
    .L3CacheSizeInKb = 384,
    .L3BankCount   = 2,
    .EUCount       = 18,
    .SliceCount    = 1,
    .SubSliceCount = 3,
    .MaxEuPerSubSlice = 6,
    .isLCIA        = 1,
    .hasLLC        = 0,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitLCIAMediaSysInfo,
    .InitShadowSku    = InitLCIAShadowSku,
    .InitShadowWa     = InitLCIAShadowWa,
};

static struct GfxDeviceInfo glkGt1Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_GEMINILAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
    .eGTType       = GTTYPE_GT1,
    .L3CacheSizeInKb = 384,
    .L3BankCount   = 2,
    .EUCount       = 12,
    .SliceCount    = 1,
    .SubSliceCount = 2,
    .MaxEuPerSubSlice = 6,
    .isLCIA        = 1,
    .hasLLC        = 0,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitLCIAMediaSysInfo,
    .InitShadowSku    = InitLCIAShadowSku,
    .InitShadowWa     = InitLCIAShadowWa,
};

static struct GfxDeviceInfo glkGt1f5Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_GEMINILAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
    .eGTType       = GTTYPE_GT1,
    .L3CacheSizeInKb = 384,
    .L3BankCount   = 2,
    .EUCount       = 18,
    .SliceCount    = 1,
    .SubSliceCount = 3,
    .MaxEuPerSubSlice = 6,
    .isLCIA        = 1,
    .hasLLC        = 0,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitLCIAMediaSysInfo,
    .InitShadowSku    = InitLCIAShadowSku,
    .InitShadowWa     = InitLCIAShadowWa,
};

static struct GfxDeviceInfo kblGt1Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_KABYLAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
    .eGTType       = GTTYPE_GT1_5,
    .L3CacheSizeInKb = 384,
    .L3BankCount   = 2,
    .EUCount       = 12,
    .SliceCount    = 1,
    .SubSliceCount = 2,
    .MaxEuPerSubSlice = 6,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitGen9MediaSysInfo,
    .InitShadowSku    = InitGen9ShadowSku,
    .InitShadowWa     = InitGen9ShadowWa,
};

static struct GfxDeviceInfo kblGt1f5Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_KABYLAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
    .eGTType       = GTTYPE_GT1_5,
    .L3CacheSizeInKb = 768,
    .L3BankCount   = 4,
    .EUCount       = 18,
    .SliceCount    = 1,
    .SubSliceCount = 3,
    .MaxEuPerSubSlice = 6,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitGen9MediaSysInfo,
    .InitShadowSku    = InitGen9ShadowSku,
    .InitShadowWa     = InitGen9ShadowWa,
};

static struct GfxDeviceInfo kblGt2Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_KABYLAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
    .eGTType       = GTTYPE_GT2,
    .L3CacheSizeInKb = 768,
    .L3BankCount   = 4,
    .EUCount       = 24,
    .SliceCount    = 1,
    .SubSliceCount = 3,
    .MaxEuPerSubSlice = 8,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitGen9MediaSysInfo,
    .InitShadowSku    = InitGen9ShadowSku,
    .InitShadowWa     = InitGen9ShadowWa,
};

static struct GfxDeviceInfo kblGt3Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_KABYLAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
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
    .InitMediaSysInfo = InitGen9MediaSysInfo,
    .InitShadowSku    = InitGen9ShadowSku,
    .InitShadowWa     = InitGen9ShadowWa,
};

static struct GfxDeviceInfo kblGt3eInfo = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_KABYLAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
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
    .InitMediaSysInfo = InitGen9MediaSysInfo,
    .InitShadowSku    = InitGen9ShadowSku,
    .InitShadowWa     = InitGen9ShadowWa,
};

static struct GfxDeviceInfo cflGt1Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_COFFEELAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
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
    .InitMediaSysInfo = InitGen9MediaSysInfo,
    .InitShadowSku    = InitGen9ShadowSku,
    .InitShadowWa     = InitGen9ShadowWa,
};

static struct GfxDeviceInfo cflGt2Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_COFFEELAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
    .eGTType       = GTTYPE_GT2,
    .L3CacheSizeInKb = 768,
    .L3BankCount   = 4,
    .EUCount       = 24,
    .SliceCount    = 1,
    .SubSliceCount = 3,
    .MaxEuPerSubSlice = 8,
    .isLCIA        = 0,
    .hasLLC        = 1,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitGen9MediaSysInfo,
    .InitShadowSku    = InitGen9ShadowSku,
    .InitShadowWa     = InitGen9ShadowWa,
};

static struct GfxDeviceInfo cflGt3Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_COFFEELAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
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
    .InitMediaSysInfo = InitGen9MediaSysInfo,
    .InitShadowSku    = InitGen9ShadowSku,
    .InitShadowWa     = InitGen9ShadowWa,
};

static struct GfxDeviceInfo cflGt3eInfo = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_COFFEELAKE,
    .displayFamily = IGFX_GEN9_CORE,
    .renderFamily  = IGFX_GEN9_CORE,
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
    .InitMediaSysInfo = InitGen9MediaSysInfo,
    .InitShadowSku    = InitGen9ShadowSku,
    .InitShadowWa     = InitGen9ShadowWa,
};

static bool sklDevice1902 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1902, &sklGt1Info);

static bool sklDevice1906 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1906, &sklGt1Info);

static bool sklDevice190a = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x190a, &sklGt1Info);

static bool sklDevice190b = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x190b, &sklGt1Info);

static bool sklDevice190e = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x190e, &sklGt1Info);

static bool sklDevice1913 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1913, &sklGt1f5Info);

static bool sklDevice1915 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1915, &sklGt1f5Info);

static bool sklDevice1917 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1917, &sklGt1f5Info);

static bool sklDevice1912 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1912, &sklGt2Info);

static bool sklDevice1916 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1916, &sklGt2Info);

static bool sklDevice191a = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x191a, &sklGt2Info);

static bool sklDevice191b = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x191b, &sklGt2Info);

static bool sklDevice191d = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x191d, &sklGt2Info);

static bool sklDevice191e = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x191e, &sklGt2Info);

static bool sklDevice1921 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1921, &sklGt2Info);

static bool sklDevice1923 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1923, &sklGt3Info);

static bool sklDevice192b = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x192b, &sklGt3eInfo);

static bool sklDevice1926 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1926, &sklGt3eInfo);

static bool sklDevice1927 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1927, &sklGt3eInfo);

static bool sklDevice192d = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x192d, &sklGt3eInfo);

/* GT4 */
static bool sklDevice192a = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x192a, &sklGt4Info);

static bool sklDevice1932 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1932, &sklGt4Info);

static bool sklDevice193a = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x193a, &sklGt4eInfo);

static bool sklDevice193b = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x193b, &sklGt4eInfo);

static bool sklDevice193d = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x193d, &sklGt4eInfo);

static bool bxtDevice0a84 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0a84, &bxtGt1f5Info);

static bool bxtDevice1a84 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1a84, &bxtGt1f5Info);

static bool bxtDevice1a85 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x1a85, &bxtGt1Info);

static bool bxtDevice5a84 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5a84, &bxtGt1f5Info);

static bool bxtDevice5a85 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5a85, &bxtGt1Info);

static bool glkDevice3a84 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3184, &glkGt1f5Info);

static bool glkDevice3a85 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3185, &glkGt1Info);

static bool kblDevice5906 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5906, &kblGt1Info);

static bool kblDevice5902 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5902, &kblGt1Info);

static bool kblDevice5908 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5908, &kblGt1Info);

static bool kblDevice590a = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x590a, &kblGt1Info);

static bool kblDevice590b = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x590b, &kblGt1Info);

static bool kblDevice590e = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x590e, &kblGt1Info);

static bool kblDevice5913 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5913, &kblGt1f5Info);

static bool kblDevice5915 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5915, &kblGt1f5Info);

static bool kblDevice5916 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5916, &kblGt2Info);

static bool kblDevice5917 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5917, &kblGt2Info);

static bool kblDevice5912 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5912, &kblGt2Info);

static bool kblDevice591e = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x591e, &kblGt2Info);

static bool kblDevice591a = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x591a, &kblGt2Info);

static bool kblDevice591b = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x591b, &kblGt2Info);

static bool kblDevice591d = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x591d, &kblGt2Info);

static bool kblDevice5921 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5921, &kblGt2Info);

static bool kblDevice5923 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5923, &kblGt3Info);

static bool kblDevice5926 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5926, &kblGt3eInfo);

static bool kblDevice5927 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5927, &kblGt3eInfo);

static bool kblDevice593b = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x593b, &kblGt3Info);

static bool kblDevice5932 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5932, &kblGt3Info);

static bool kblDevice592b = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x592b, &kblGt3Info);

static bool kblDevice592a = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x592a, &kblGt3Info);

static bool cflDevice3e93 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3e93, &cflGt1Info);

static bool cflDevice3e99 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3e99, &cflGt1Info);

static bool cflDevice3e90 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3e90, &cflGt1Info);

static bool cflDevice3e92 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3e92, &cflGt2Info);

static bool cflDevice3e94 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3e94, &cflGt2Info);

static bool cflDevice3e91 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3e91, &cflGt2Info);

static bool cflDevice3e96 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3e96, &cflGt2Info);

static bool cflDevice3e98 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3e98, &cflGt2Info);

static bool cflDevice3e9a = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3e9a, &cflGt2Info);

static bool cflDevice3e9b = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3e9b, &cflGt2Info);

static bool cflDevice3e9c = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3e9c, &cflGt1Info);

static bool cflDevice3ea5 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3ea5, &cflGt3eInfo);

static bool cflDevice3ea6 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3ea6, &cflGt3eInfo);

static bool cflDevice3ea7 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3ea7, &cflGt3eInfo);

static bool cflDevice3ea8 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3ea8, &cflGt3eInfo);

static bool cflDevice3ea9 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3ea9, &cflGt2Info);

static bool cflDevice3ea1 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3ea1, &cflGt1Info);

static bool cflDevice3ea4 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3ea4, &cflGt1Info);

static bool cflDevice3ea0 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3ea0, &cflGt2Info);

static bool cflDevice3ea3 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3ea3, &cflGt2Info);

static bool cflDevice3ea2 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x3ea2, &cflGt3eInfo);

/* CML GT1 */
static bool cmlDevice9b21 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9b21, &cflGt1Info);
static bool cmlDevice9baa = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9baa, &cflGt1Info);
static bool cmlDevice9bab = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9bab, &cflGt1Info);
static bool cmlDevice9bac = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9bac, &cflGt1Info);
static bool cmlDeviceba0 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9ba0, &cflGt1Info);
static bool cmlDevice9ba5 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9ba5, &cflGt1Info);
static bool cmlDevice9ba8 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9ba8, &cflGt1Info);
static bool cmlDevice9ba4 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9ba4, &cflGt1Info);
static bool cmlDevice9ba2 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9ba2, &cflGt1Info);

/* CML GT2 */
static bool cmlDevice9b41 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9b41, &cflGt2Info);
static bool cmlDevice9bca = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9bca, &cflGt2Info);
static bool cmlDevice9bcb = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9bcb, &cflGt2Info);
static bool cmlDevice9bcc = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9bcc, &cflGt2Info);
static bool cmlDevice9bc0 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9bc0, &cflGt2Info);
static bool cmlDevice9bc5 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9bc5, &cflGt2Info);
static bool cmlDevice9bc6 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9bc6, &cflGt2Info);
static bool cmlDevice9bc8 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9bc8, &cflGt2Info);
static bool cmlDevice9bc4 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9bc4, &cflGt2Info);
static bool cmlDevice9bc2 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9bc2, &cflGt2Info);
static bool cmlDevice9be6 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9be6, &cflGt2Info);
static bool cmlDevice9bf6 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9bf6, &cflGt2Info);

/* AML/KBL Y GT2 */
static bool cmlDevice591c = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x591C, &cflGt2Info);
static bool cmlDevice87C0 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x87C0, &cflGt2Info);

/* AML/CFL Y GT2 */
static bool cmlDevice87ca = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x87ca, &cflGt2Info);
