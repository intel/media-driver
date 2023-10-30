/*===================== begin_copyright_notice ==================================

Copyright (c) 2021, Intel Corporation

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
//! \file     media_sysinfo_xe.cpp
//!

#include "igfxfmid.h"
#include "linux_system_info.h"
#include "skuwa_factory.h"
#include "linux_skuwa_debug.h"
#include "linux_media_skuwa.h"
#include "linux_shadow_skuwa.h"
#include "media_user_setting_specific.h"

#define THREADS_NUMBER_PER_EU_HP 8

//extern template class DeviceInfoFactory<GfxDeviceInfo>;
typedef DeviceInfoFactory<GfxDeviceInfo> base_fact;

bool InitXeHPMediaSysInfo(struct GfxDeviceInfo *devInfo,
                                MEDIA_GT_SYSTEM_INFO *sysInfo)
{
    if ((devInfo == nullptr) || (sysInfo == nullptr))
    {
        DEVINFO_ERROR("null ptr is passed\n");
        return false;
    }

    if (!sysInfo->SliceCount)
    {
        sysInfo->SliceCount = devInfo->SliceCount;
    }

    if (!sysInfo->SubSliceCount)
    {
        sysInfo->SubSliceCount = devInfo->SubSliceCount;
    }

    if (!sysInfo->EUCount)
    {
        sysInfo->EUCount = devInfo->EUCount;
    }

    sysInfo->L3BankCount                            = devInfo->L3BankCount;
    sysInfo->VEBoxInfo.Instances.Bits.VEBox0Enabled = 1;
    sysInfo->MaxEuPerSubSlice                       = devInfo->MaxEuPerSubSlice;
    sysInfo->MaxSlicesSupported                     = sysInfo->SliceCount;
    sysInfo->MaxSubSlicesSupported                  = sysInfo->SubSliceCount;

    sysInfo->VEBoxInfo.NumberOfVEBoxEnabled = 0; /*Query the VEBox engine info from KMD*/
    sysInfo->VDBoxInfo.NumberOfVDBoxEnabled = 0; /*Query the VDBox engine info from KMD*/

    sysInfo->ThreadCount = sysInfo->EUCount * THREADS_NUMBER_PER_EU_HP;

    sysInfo->VEBoxInfo.IsValid = true;
    sysInfo->VDBoxInfo.IsValid = true;

    //Media driver does not set the other gtsysinfo fileds such as L3CacheSizeInKb, EdramSizeInKb and LLCCacheSizeInKb now.
    //If needed in the future, query them from KMD.

    return true;
}


bool InitXeHPShadowSku(struct GfxDeviceInfo *devInfo,
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

    skuTable->FtrDisplayYTiling = 0;
    skuTable->FtrEDram = devInfo->hasERAM;

    skuTable->FtrE2ECompression = 1;
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
        skuTable->FtrE2ECompression = 0;
    }

    skuTable->FtrLinearCCS = 1;
    skuTable->FtrFlatPhysCCS = 1;
    skuTable->FtrTileY = 0;

    skuTable->FtrLocalMemory = 1;

    return true;
}

bool InitPvcShadowSku(struct GfxDeviceInfo *devInfo,
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

    skuTable->FtrDisplayYTiling = 0;
    skuTable->FtrEDram = devInfo->hasERAM;

    skuTable->FtrE2ECompression = 1;
    skuTable->FtrUnified3DMediaCompressionFormats = 1;
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
        skuTable->FtrE2ECompression = 0;
    }

    skuTable->FtrLinearCCS = 1;
    skuTable->FtrFlatPhysCCS = 1;
    skuTable->FtrTileY = 0;

    skuTable->FtrLocalMemory = 1;

    return true;
}


bool InitXeHPShadowWa(struct GfxDeviceInfo *devInfo,
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

    waTable->Wa_15010089951 = 1;

    return true;
}

bool InitPvcShadowWa(struct GfxDeviceInfo *devInfo,
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

    // Set it to 0 if need to support 256B compress mode
    waTable->WaLimit128BMediaCompr = 0;

    return true;
}

static struct GfxDeviceInfo xehpGt4Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_XE_HP_SDV,
    .displayFamily = IGFX_XE_HP_CORE,
    .renderFamily  = IGFX_XE_HP_CORE,
    .mediaFamily   = IGFX_UNKNOWN_CORE,
    .eGTType       = GTTYPE_GT4,
    .L3CacheSizeInKb = 0,
    .L3BankCount   = 0,
    .EUCount       = 0,
    .SliceCount    = 0,
    .SubSliceCount = 0,
    .MaxEuPerSubSlice = 0,
    .isLCIA        = 0,
    .hasLLC        = 0,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitXeHPMediaSysInfo,
    .InitShadowSku    = InitXeHPShadowSku,
    .InitShadowWa     = InitXeHPShadowWa,
};

static struct GfxDeviceInfo pvcGt4Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_PVC,
    .displayFamily = IGFX_GEN12_CORE,
    .renderFamily  = IGFX_XE_HPC_CORE,
    .mediaFamily   = IGFX_UNKNOWN_CORE,
    .eGTType       = GTTYPE_GT4,
    .L3CacheSizeInKb = 0,
    .L3BankCount   = 0,
    .EUCount       = 0,
    .SliceCount    = 0,
    .SubSliceCount = 0,
    .MaxEuPerSubSlice = 0,
    .isLCIA        = 0,
    .hasLLC        = 0,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitXeHPMediaSysInfo,
    .InitShadowSku    = InitPvcShadowSku,
    .InitShadowWa     = InitPvcShadowWa,
};

static struct GfxDeviceInfo dg2Gt4Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_DG2,
    .displayFamily = IGFX_GEN12_CORE,
    .renderFamily  = IGFX_XE_HPG_CORE,
    .mediaFamily   = IGFX_UNKNOWN_CORE,
    .eGTType       = GTTYPE_UNDEFINED,
    .L3CacheSizeInKb = 0,
    .L3BankCount   = 0,
    .EUCount       = 0,
    .SliceCount    = 0,
    .SubSliceCount = 0,
    .MaxEuPerSubSlice = 0,
    .isLCIA        = 0,
    .hasLLC        = 0,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitXeHPMediaSysInfo,
    .InitShadowSku    = InitXeHPShadowSku,
    .InitShadowWa     = InitXeHPShadowWa,
};

static bool xehpGt4Devicef2d2 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xF2D2, &xehpGt4Info);

static bool xehpGt4Device0201 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0201, &xehpGt4Info);

static bool xehpGt4Device0202 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0202, &xehpGt4Info);

static bool xehpGt4Device0203 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0203, &xehpGt4Info);

static bool xehpGt4Device0204 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0204, &xehpGt4Info);

static bool xehpGt4Device0205 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0205, &xehpGt4Info);

static bool xehpGt4Device0206 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0206, &xehpGt4Info);

static bool xehpGt4Device0207 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0207, &xehpGt4Info);

static bool xehpGt4Device0208 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0208, &xehpGt4Info);

static bool xehpGt4Device0209 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0209, &xehpGt4Info);

static bool xehpGt4Device020A = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x020A, &xehpGt4Info);

static bool xehpGt4Device020B = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x020B, &xehpGt4Info);

static bool xehpGt4Device020C = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x020C, &xehpGt4Info);

static bool xehpGt4Device020D = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x020D, &xehpGt4Info);

static bool xehpGt4Device020E = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x020E, &xehpGt4Info);

static bool xehpGt4Device020F = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x020F, &xehpGt4Info);

static bool xehpGt4Device0210 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0210, &xehpGt4Info);

static bool pvcGt4Device0bd0 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0bd0, &pvcGt4Info);

static bool pvcGt4Device0bd5 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0bd5, &pvcGt4Info);

static bool pvcGt4Device0bd6 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0bd6, &pvcGt4Info);

static bool pvcGt4Device0bd7 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0bd7, &pvcGt4Info);

static bool pvcGt4Device0bd8 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0bd8, &pvcGt4Info);

static bool pvcGt4Device0bd9 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0bd9, &pvcGt4Info); 

static bool pvcGt4Device0bda = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0bda, &pvcGt4Info);

static bool pvcGt4Device0bdb = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0bdb, &pvcGt4Info);

static bool pvcGt4Device0be0 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0be0, &pvcGt4Info);

static bool pvcGt4Device0be1 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0be1, &pvcGt4Info);

static bool pvcGt4Device0be5 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x0be5, &pvcGt4Info);

static bool dg2Gt4Device4F80 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4F80, &dg2Gt4Info);

static bool dg2Gt4Device4F81 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4F81, &dg2Gt4Info);

static bool dg2Gt4Device4F82 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4F82, &dg2Gt4Info);

static bool dg2Gt4Device4F83 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4F83, &dg2Gt4Info);

static bool dg2Gt4Device4F84 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4F84, &dg2Gt4Info);

static bool dg2Gt4Device4F85 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4F85, &dg2Gt4Info);

static bool dg2Gt4Device4F86 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4F86, &dg2Gt4Info);

static bool dg2Gt4Device4F87 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4F87, &dg2Gt4Info);

static bool dg2Gt4Device4F88 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4F88, &dg2Gt4Info);

static bool dg2Gt4Device5690 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5690, &dg2Gt4Info);

static bool dg2Gt4Device5691 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5691, &dg2Gt4Info);

static bool dg2Gt4Device5692 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5692, &dg2Gt4Info);

static bool dg2Gt4Device5693 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5693, &dg2Gt4Info);

static bool dg2Gt4Device5694 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5694, &dg2Gt4Info);

static bool dg2Gt4Device5695 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5695, &dg2Gt4Info);

static bool dg2Gt4Device5696 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5696, &dg2Gt4Info);

static bool dg2Gt4Device5697 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x5697, &dg2Gt4Info);

static bool dg2Gt4Device56A0 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x56A0, &dg2Gt4Info);

static bool dg2Gt4Device56A1 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x56A1, &dg2Gt4Info);

static bool dg2Gt4Device56A2 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x56A2, &dg2Gt4Info);

static bool dg2Gt4Device56A3 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x56A3, &dg2Gt4Info);

static bool dg2Gt4Device56A4 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x56A4, &dg2Gt4Info);

static bool dg2Gt4Device56A5 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x56A5, &dg2Gt4Info);

static bool dg2Gt4Device56A6 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x56A6, &dg2Gt4Info);

static bool dg2Gt4Device56B0 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x56B0, &dg2Gt4Info);

static bool dg2Gt4Device56B1 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x56B1, &dg2Gt4Info);

static bool dg2Gt4Device56BA = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x56BA, &dg2Gt4Info);

static bool dg2Gt4Device56BB = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x56BB, &dg2Gt4Info);

static bool dg2Gt4Device56BC = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x56BC, &dg2Gt4Info);

static bool dg2Gt4Device56BD = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x56BD, &dg2Gt4Info);

static bool dg2Gt4Device56B2 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x56B2, &dg2Gt4Info);

static bool dg2Gt4Device56B3 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x56B3, &dg2Gt4Info);

static bool dg2Gt4Device56C0 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x56C0, &dg2Gt4Info);

static bool dg2Gt4Device56C1 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x56C1, &dg2Gt4Info);
