/*===================== begin_copyright_notice ==================================

Copyright (c) 2017-2021, Intel Corporation

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
//! \file     media_sysinfo_g12.cpp
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

static bool InitTglMediaSysInfo(struct GfxDeviceInfo *devInfo, MEDIA_GT_SYSTEM_INFO *sysInfo)
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

static bool InitTglShadowSku(struct GfxDeviceInfo *devInfo,
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
    }

    skuTable->FtrLinearCCS = 1;
    skuTable->FtrTileY = 1;

    return true;
}

static bool InitTglShadowWa(struct GfxDeviceInfo *devInfo,
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

    //source and recon surfaces need to be aligned to the LCU size
    waTable->WaAlignYUVResourceToLCU = 1;

    /* For TGLLP and derivative platforms HW supports both 16K and 64K Aux granularity, POR mode is 64K. On all aux based
    platforms (where FtrFlaPhysCCS = 0) WaAuxTable64KGranular is expected to be set. */
    waTable->WaAuxTable64KGranular = 1;

    return true;
}

#ifdef IGFX_GEN12_DG1_SUPPORTED
static bool InitDG1ShadowSku(struct GfxDeviceInfo *devInfo,
                             SHADOW_MEDIA_FEATURE_TABLE *skuTable,
                             struct LinuxDriverInfo *drvInfo)
{
    if(!InitTglShadowSku(devInfo, skuTable, drvInfo))
    {
        return false;
    }
    skuTable->FtrLocalMemory = 1;

    return true;
}
#endif

static struct GfxDeviceInfo tgllpGt1Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_TIGERLAKE_LP,
    .displayFamily = IGFX_GEN12_CORE,
    .renderFamily  = IGFX_GEN12_CORE,
    .mediaFamily   = IGFX_UNKNOWN_CORE,
    .eGTType       = GTTYPE_GT1,
    .L3CacheSizeInKb = 0,
    .L3BankCount   = 4,
    .EUCount       = 0,
    .SliceCount    = 0,
    .SubSliceCount = 0,
    .MaxEuPerSubSlice = 0,
    .isLCIA        = 0,
    .hasLLC        = 0,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitTglMediaSysInfo,
    .InitShadowSku    = InitTglShadowSku,
    .InitShadowWa     = InitTglShadowWa,
};

static struct GfxDeviceInfo tgllpGt2Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_TIGERLAKE_LP,
    .displayFamily = IGFX_GEN12_CORE,
    .renderFamily  = IGFX_GEN12_CORE,
    .mediaFamily   = IGFX_UNKNOWN_CORE,
    .eGTType       = GTTYPE_GT2,
    .L3CacheSizeInKb = 0,
    .L3BankCount   = 8,
    .EUCount       = 0,
    .SliceCount    = 0,
    .SubSliceCount = 0,
    .MaxEuPerSubSlice = 0,
    .isLCIA        = 0,
    .hasLLC        = 0,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitTglMediaSysInfo,
    .InitShadowSku    = InitTglShadowSku,
    .InitShadowWa     = InitTglShadowWa,
};

#ifdef IGFX_GEN12_DG1_SUPPORTED
static struct GfxDeviceInfo dg1Gt2Info = {
    .platformType  = PLATFORM_MOBILE,
    .productFamily = IGFX_DG1,
    .displayFamily = IGFX_GEN12_CORE,
    .renderFamily  = IGFX_GEN12_CORE,
    .mediaFamily   = IGFX_UNKNOWN_CORE,
    .eGTType       = GTTYPE_GT2,
    .L3CacheSizeInKb = 0,
    .L3BankCount   = 8,
    .EUCount       = 0,
    .SliceCount    = 0,
    .SubSliceCount = 0,
    .MaxEuPerSubSlice = 0,
    .isLCIA        = 0,
    .hasLLC        = 0,
    .hasERAM       = 0,
    .InitMediaSysInfo = InitTglMediaSysInfo,
    .InitShadowSku    = InitDG1ShadowSku,
    .InitShadowWa     = InitTglShadowWa,
};
static bool dg1Gt2Device4905 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4905, &dg1Gt2Info);

static bool dg1Gt2Device4906 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4906, &dg1Gt2Info);


static bool dg1Gt2Device4907 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4907, &dg1Gt2Info);

static bool dg1Gt2Device4908 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4908, &dg1Gt2Info);

static bool dg1Gt2Device4909 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4909, &dg1Gt2Info);
#endif

#ifdef IGFX_GEN12_RKL_SUPPORTED
static struct GfxDeviceInfo rklGt1Info = {
    .platformType     = PLATFORM_MOBILE,
    .productFamily    = IGFX_ROCKETLAKE,
    .displayFamily    = IGFX_GEN12_CORE,
    .renderFamily     = IGFX_GEN12_CORE,
    .mediaFamily      = IGFX_UNKNOWN_CORE,
    .eGTType          = GTTYPE_GT1,
    .L3CacheSizeInKb  = 0,
    .L3BankCount      = 4,
    .EUCount          = 0,
    .SliceCount       = 0,
    .SubSliceCount    = 0,
    .MaxEuPerSubSlice = 0,
    .isLCIA           = 0,
    .hasLLC           = 0,
    .hasERAM          = 0,
    .InitMediaSysInfo = InitTglMediaSysInfo,
    .InitShadowSku    = InitTglShadowSku,
    .InitShadowWa     = InitTglShadowWa,
};

static struct GfxDeviceInfo rklGt1fInfo = {
    .platformType     = PLATFORM_MOBILE,
    .productFamily    = IGFX_ROCKETLAKE,
    .displayFamily    = IGFX_GEN12_CORE,
    .renderFamily     = IGFX_GEN12_CORE,
    .mediaFamily      = IGFX_UNKNOWN_CORE,
    .eGTType          = GTTYPE_GT0_5,
    .L3CacheSizeInKb  = 0,
    .L3BankCount      = 4,
    .EUCount          = 0,
    .SliceCount       = 0,
    .SubSliceCount    = 0,
    .MaxEuPerSubSlice = 0,
    .isLCIA           = 0,
    .hasLLC           = 0,
    .hasERAM          = 0,
    .InitMediaSysInfo = InitTglMediaSysInfo,
    .InitShadowSku    = InitTglShadowSku,
    .InitShadowWa     = InitTglShadowWa,
};

static bool rklGt1Device4C80 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4C80, &rklGt1Info);

static bool rklGt1Device4C8A = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4C8A, &rklGt1Info);

static bool rklGt1fDevice4C8B = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4C8B, &rklGt1Info);

static bool rklGt1fDevice4C8C = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4C8C, &rklGt1fInfo);

static bool rklGt1fDevice4C90 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4C90, &rklGt1Info);

static bool rklGt1fDevice4C9A = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4C9A, &rklGt1Info);
#endif

#ifdef IGFX_GEN12_ADLS_SUPPORTED
static struct GfxDeviceInfo adlsGt1Info = {
    .platformType     = PLATFORM_DESKTOP,
    .productFamily    = IGFX_ALDERLAKE_S,
    .displayFamily    = IGFX_GEN12_CORE,
    .renderFamily     = IGFX_GEN12_CORE,
    .mediaFamily      = IGFX_UNKNOWN_CORE,
    .eGTType          = GTTYPE_GT1,
    .L3CacheSizeInKb  = 0,
    .L3BankCount      = 4,
    .EUCount          = 0,
    .SliceCount       = 0,
    .SubSliceCount    = 0,
    .MaxEuPerSubSlice = 0,
    .isLCIA           = 0,
    .hasLLC           = 0,
    .hasERAM          = 0,
    .InitMediaSysInfo = InitTglMediaSysInfo,
    .InitShadowSku    = InitTglShadowSku,
    .InitShadowWa     = InitTglShadowWa,
};


static struct GfxDeviceInfo adlsGt1fInfo = {
    .platformType     = PLATFORM_DESKTOP,
    .productFamily    = IGFX_ALDERLAKE_S,
    .displayFamily    = IGFX_GEN12_CORE,
    .renderFamily     = IGFX_GEN12_CORE,
    .mediaFamily      = IGFX_UNKNOWN_CORE,
    .eGTType          = GTTYPE_GT0_5,
    .L3CacheSizeInKb  = 0,
    .L3BankCount      = 4,
    .EUCount          = 0,
    .SliceCount       = 0,
    .SubSliceCount    = 0,
    .MaxEuPerSubSlice = 0,
    .isLCIA           = 0,
    .hasLLC           = 0,
    .hasERAM          = 0,
    .InitMediaSysInfo = InitTglMediaSysInfo,
    .InitShadowSku    = InitTglShadowSku,
    .InitShadowWa     = InitTglShadowWa,
};


static bool adlsGt1Device4680 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4680, &adlsGt1Info);

static bool adlsGt1Device4681 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4681, &adlsGt1Info);

static bool adlsGt1Device4682 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4682, &adlsGt1Info);

static bool adlsGt1Device4683 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4683, &adlsGt1fInfo);

static bool adlsGt1Device4688 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4688, &adlsGt1Info);

static bool adlsGt1Device468A = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x468A, &adlsGt1Info);

static bool adlsGt1Device468B = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x468B, &adlsGt1Info);

static bool adlsGt1Device4690 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4690, &adlsGt1Info);

static bool adlsGt1Device4691 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4691, &adlsGt1Info);

static bool adlsGt1Device4692 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4692, &adlsGt1Info);

static bool adlsGt1Device4693 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4693, &adlsGt1Info);

static bool rplsGt1DeviceA780 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA780, &adlsGt1Info);

static bool rplsGt1DeviceA781 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA781, &adlsGt1Info);

static bool rplsGt1DeviceA782 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA782, &adlsGt1Info);

static bool rplsGt1DeviceA783 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA783, &adlsGt1Info);

static bool rplsGt1DeviceA788 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA788, &adlsGt1Info);

static bool rplsGt1DeviceA789 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA789, &adlsGt1Info);

static bool rplsGt1DeviceA78A = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA78A, &adlsGt1Info);

static bool rplsGt1DeviceA78B = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA78B, &adlsGt1Info);

#endif


#ifdef IGFX_GEN12_ADLP_SUPPORTED
static struct GfxDeviceInfo adlpGt2Info = {
    .platformType     = PLATFORM_MOBILE,
    .productFamily    = IGFX_ALDERLAKE_P,
    .displayFamily    = IGFX_GEN12_CORE,
    .renderFamily     = IGFX_GEN12_CORE,
    .mediaFamily      = IGFX_UNKNOWN_CORE,
    .eGTType          = GTTYPE_GT2,
    .L3CacheSizeInKb  = 0,
    .L3BankCount      = 8,
    .EUCount          = 0,
    .SliceCount       = 0,
    .SubSliceCount    = 0,
    .MaxEuPerSubSlice = 0,
    .isLCIA           = 0,
    .hasLLC           = 0,
    .hasERAM          = 0,
    .InitMediaSysInfo = InitTglMediaSysInfo,
    .InitShadowSku    = InitTglShadowSku,
    .InitShadowWa     = InitTglShadowWa,
};

static bool adlpGt2Device46A0 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46A0, &adlpGt2Info);

static bool adlpGt2Device46A1 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46A1, &adlpGt2Info);

static bool adlpGt2Device46A2 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46A2, &adlpGt2Info);

static bool adlpGt2Device46A3 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46A3, &adlpGt2Info);

static bool adlpGt2Device46A6 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46A6, &adlpGt2Info);

static bool adlpGt2Device4626 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4626, &adlpGt2Info);

static bool adlpGt2Device46B0 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46B0, &adlpGt2Info);

static bool adlpGt2Device46B1 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46B1, &adlpGt2Info);

static bool adlpGt2Device46B2 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46B2, &adlpGt2Info);

static bool adlpGt2Device46B3 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46B3, &adlpGt2Info);

static bool adlpGt2Device46A8 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46A8, &adlpGt2Info);

static bool adlpGt2Device4628 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x4628, &adlpGt2Info);

static bool adlpGt2Device46C0 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46C0, &adlpGt2Info);

static bool adlpGt2Device46C1 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46C1, &adlpGt2Info);

static bool adlpGt2Device46C2 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46C2, &adlpGt2Info);

static bool adlpGt2Device46C3 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46C3, &adlpGt2Info);

static bool adlpGt2Device46AA = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46AA, &adlpGt2Info);

static bool adlpGt2Device462A = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x462A, &adlpGt2Info);

static bool rplpGt2DeviceA7A0 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA7A0, &adlpGt2Info);

static bool rplpGt2DeviceA720 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA720, &adlpGt2Info);

static bool rplpGt2DeviceA7A8 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA7A8, &adlpGt2Info);

static bool rplpGt2DeviceA7A1 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA7A1, &adlpGt2Info);

static bool rplpGt2DeviceA721 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA721, &adlpGt2Info);

static bool rplpGt2DeviceA7A9 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA7A9, &adlpGt2Info);

static bool rplGt2DeviceA7AA = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA7AA, &adlpGt2Info);

static bool rplGt2DeviceA7AB = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA7AB, &adlpGt2Info);

static bool rplGt2DeviceA7AC = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA7AC, &adlpGt2Info);

static bool rplGt2DeviceA7AD = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0xA7AD, &adlpGt2Info);

#endif

#ifdef IGFX_GEN12_ADLN_SUPPORTED
static struct GfxDeviceInfo adlnGt1Info = {
    .platformType     = PLATFORM_MOBILE,
    .productFamily    = IGFX_ALDERLAKE_N,
    .displayFamily    = IGFX_GEN12_CORE,
    .renderFamily     = IGFX_GEN12_CORE,
    .mediaFamily      = IGFX_UNKNOWN_CORE,
    .eGTType          = GTTYPE_GT1,
    .L3CacheSizeInKb  = 0,
    .L3BankCount      = 8,
    .EUCount          = 0,
    .SliceCount       = 0,
    .SubSliceCount    = 0,
    .MaxEuPerSubSlice = 0,
    .isLCIA           = 0,
    .hasLLC           = 0,
    .hasERAM          = 0,
    .InitMediaSysInfo = InitTglMediaSysInfo,
    .InitShadowSku    = InitTglShadowSku,
    .InitShadowWa     = InitTglShadowWa,
};

static bool adlnGt1Device46D0 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46D0, &adlnGt1Info);

static bool adlnGt1Device46D1 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46D1, &adlnGt1Info);

static bool adlnGt1Device46D2 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46D2, &adlnGt1Info);

static bool twlGt1Device46D3 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46D3, &adlnGt1Info);

static bool twlGt1Device46D4 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x46D4, &adlnGt1Info);
#endif 


static bool tgllpGt2Device9a40 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9A40, &tgllpGt2Info);

static bool tgllpGt2Device9a49 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9A49, &tgllpGt2Info);

static bool tgllpGt2Device9a59 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9A59, &tgllpGt2Info);

static bool tgllpGt2Device9a60 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9A60, &tgllpGt2Info);

static bool tgllpGt2Device9a68 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9A68, &tgllpGt2Info);

static bool tgllpGt2Device9a70 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9A70, &tgllpGt2Info);

static bool tgllpGt1Device9a78 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9A78, &tgllpGt1Info);

static bool tgllpGt2Device9ac9 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9AC9, &tgllpGt2Info);

static bool tgllpGt2Device9af8 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9AF8, &tgllpGt2Info);

static bool tgllpGt2Device9ac0 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9AC0, &tgllpGt2Info);

static bool tgllpGt2Device9ad9 = DeviceInfoFactory<GfxDeviceInfo>::
    RegisterDevice(0x9AD9, &tgllpGt2Info);

