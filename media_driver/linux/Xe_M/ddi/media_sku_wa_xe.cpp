/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     media_sku_wa_xe.cpp
//!

#include "igfxfmid.h"
#include "linux_system_info.h"
#include "skuwa_factory.h"
#include "linux_skuwa_debug.h"
#include "linux_media_skuwa.h"
#include "mos_utilities.h"
#include "mos_os_specific.h"
#include "media_user_setting_specific.h"

#ifndef SI_REV_LO
#define SI_REV_LO(SteppingID) (SteppingID & 0xFFFF)
#endif

#ifndef SI_WA_FROM
#define SI_WA_FROM(ulRevID, STEPPING) (ulRevID >= (int)SI_REV_LO(STEPPING))
#endif

#ifndef ACM_G10_MEDIA_REV_ID_B0
#define ACM_G10_MEDIA_REV_ID_B0 DG2_MEDIA_REV_ID_B0
#endif

static constexpr uint32_t singleVeboxSubSliceNumMax = 24;

//extern template class DeviceInfoFactory<GfxDeviceInfo>;
typedef DeviceInfoFactory<LinuxDeviceInit> DeviceInit;

static struct LinuxCodecInfo XehpSdvCodecInfo =
{
    .avcDecoding    = 1,
    .mpeg2Decoding  = 1,
    .vp8Decoding    = 0,
    .vc1Decoding    = 0,
    .jpegDecoding   = 1,
    .avcEncoding    = 0,
    .mpeg2Encoding  = 0,
    .hevcDecoding   = 1,
    .hevcEncoding   = 0,
    .jpegEncoding   = 0,
    .avcVdenc       = 0,
    .vp9Decoding    = 1,
    .hevc10Decoding = 1,
    .vp9b10Decoding = 1,
    .hevc10Encoding = 0,
    .hevc12Encoding = 0,
    .vp8Encoding    = 0,
    .hevcVdenc      = 0,
    .vp9Vdenc       = 0,
    .adv0Decoding   = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .adv1Decoding   = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
};

static struct LinuxCodecInfo Dg2CodecInfo =
{
    .avcDecoding    = 1,
    .mpeg2Decoding  = 1,
    .vp8Decoding    = 0,
    .vc1Decoding    = 0,
    .jpegDecoding   = 1,
    .avcEncoding    = 0,
    .mpeg2Encoding  = 0,
    .hevcDecoding   = 1,
    .hevcEncoding   = 0,
    .jpegEncoding   = 1,
    .avcVdenc       = 1,
    .vp9Decoding    = 1,
    .hevc10Decoding = 1,
    .vp9b10Decoding = 1,
    .hevc10Encoding = 0,
    .hevc12Encoding = 0,
    .vp8Encoding    = 0,
    .hevcVdenc      = 1,
    .vp9Vdenc       = 1,
    .adv0Decoding   = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .adv1Decoding   = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
};

static struct LinuxCodecInfo PvcCodecInfo =
{
    .avcDecoding    = 1,
    .mpeg2Decoding  = 1,
    .vp8Decoding    = 0,
    .vc1Decoding    = 0,
    .jpegDecoding   = 1,
    .avcEncoding    = 0,
    .mpeg2Encoding  = 0,
    .hevcDecoding   = 1,
    .hevcEncoding   = 0,
#ifdef IGFX_PVC_ENABLE_NON_UPSTREAM
    .jpegEncoding   = 1,
    .avcVdenc       = 1,
#else
    .jpegEncoding   = 0,
    .avcVdenc       = 0,
#endif
    .vp9Decoding    = 1,
    .hevc10Decoding = 1,
    .vp9b10Decoding = 1,
    .hevc10Encoding = 0,
    .hevc12Encoding = 0,
    .vp8Encoding    = 0,
#ifdef IGFX_PVC_ENABLE_NON_UPSTREAM
    .hevcVdenc      = 1,
    .vp9Vdenc       = 1,
#else
    .hevcVdenc      = 0,
    .vp9Vdenc       = 0,
#endif
    .adv0Decoding   = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .adv1Decoding   = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
};

static bool InitTglMediaSkuExt(struct GfxDeviceInfo *devInfo,
                             MediaFeatureTable *skuTable,
                             struct LinuxDriverInfo *drvInfo,
                             struct LinuxCodecInfo *codecInfo,
                             MediaUserSettingSharedPtr userSettingPtr)
{
    if ((devInfo == nullptr) || (skuTable == nullptr) || (drvInfo == nullptr))
    {
        DEVINFO_ERROR("null ptr is passed\n");
        return false;
    }

    if (drvInfo->hasBsd)
    {
        MEDIA_WR_SKU(skuTable, FtrAVCVLDLongDecoding, codecInfo->avcDecoding);
        MEDIA_WR_SKU(skuTable, FtrMPEG2VLDDecoding, codecInfo->mpeg2Decoding);
        MEDIA_WR_SKU(skuTable, FtrIntelVP8VLDDecoding, codecInfo->vp8Decoding);
        MEDIA_WR_SKU(skuTable, FtrVC1VLDDecoding, codecInfo->vc1Decoding);
        MEDIA_WR_SKU(skuTable, FtrIntelJPEGDecoding, codecInfo->jpegDecoding);
        MEDIA_WR_SKU(skuTable, FtrEncodeAVC, codecInfo->avcEncoding);
        MEDIA_WR_SKU(skuTable, FtrEncodeMPEG2, codecInfo->mpeg2Encoding);
        MEDIA_WR_SKU(skuTable, FtrIntelHEVCVLDMainDecoding, codecInfo->hevcDecoding);
        MEDIA_WR_SKU(skuTable, FtrIntelHEVCVLDMain10Decoding, codecInfo->hevc10Decoding);

        MEDIA_WR_SKU(skuTable, FtrEncodeHEVC, codecInfo->hevcEncoding);
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVC10bit, codecInfo->hevc10Encoding);
        MEDIA_WR_SKU(skuTable, FtrEncodeJPEG, codecInfo->jpegEncoding);
        MEDIA_WR_SKU(skuTable, FtrEncodeAVCVdenc, codecInfo->avcVdenc);
        MEDIA_WR_SKU(skuTable, FtrVP9VLDDecoding, codecInfo->vp9Decoding);
        MEDIA_WR_SKU(skuTable, FtrIntelVP9VLDProfile0Decoding8bit420, codecInfo->vp9Decoding);
        MEDIA_WR_SKU(skuTable, FtrVP9VLD10bProfile2Decoding, codecInfo->vp9b10Decoding);
        MEDIA_WR_SKU(skuTable, FtrIntelVP9VLDProfile2Decoding, codecInfo->vp9b10Decoding);
        MEDIA_WR_SKU(skuTable, FtrIntelAV1VLDDecoding8bit420, codecInfo->adv0Decoding);
        MEDIA_WR_SKU(skuTable, FtrIntelAV1VLDDecoding10bit420, codecInfo->adv1Decoding);

        /* VP8 enc */
        MEDIA_WR_SKU(skuTable, FtrEncodeVP8, codecInfo->vp8Encoding);

        /* HEVC VDENC */
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVCVdencMain, codecInfo->hevcVdenc);
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVCVdencMain10, codecInfo->hevcVdenc);

        /* Vp9 VDENC */
        MEDIA_WR_SKU(skuTable, FtrEncodeVP9Vdenc, codecInfo->vp9Vdenc);

        /* HEVC Main8/10bit-422/444 Decoding. Currently it is enabled. */
        MEDIA_WR_SKU(skuTable, FtrIntelHEVCVLD42210bitDecoding, 1);
        MEDIA_WR_SKU(skuTable, FtrIntelHEVCVLD4448bitDecoding, 1);
        MEDIA_WR_SKU(skuTable, FtrIntelHEVCVLD44410bitDecoding, 1);

        /* SFC Histogram Streamout. */
        MEDIA_WR_SKU(skuTable, FtrSFCHistogramStreamOut, 1);

        /* Subset buffer for realtile decoding. */
        MEDIA_WR_SKU(skuTable, FtrIntelHEVCVLDDecodingSubsetBuffer, 1);

        /* HEVC Main8/10bit-420/422/444 Scc Decoding. Currently it is enabled. */
        MEDIA_WR_SKU(skuTable, FtrIntelHEVCVLDMain8bit420SCC, 1);
        MEDIA_WR_SKU(skuTable, FtrIntelHEVCVLDMain10bit420SCC, 1);
        MEDIA_WR_SKU(skuTable, FtrIntelHEVCVLDMain8bit444SCC, 1);
        MEDIA_WR_SKU(skuTable, FtrIntelHEVCVLDMain10bit444SCC, 1);

        /* HEVC VDENC Main8/10 422/444 Encoding. */
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVCVdencMain444, codecInfo->hevcVdenc);
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVCVdencMain422, codecInfo->hevcVdenc);
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVCVdencMain10bit422, codecInfo->hevcVdenc);
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVCVdencMain10bit444, codecInfo->hevcVdenc);

        /* HEVC VDENC Main8/10bit-420/422/444 Scc Encoding. */
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVCVdencMainSCC, codecInfo->hevcVdenc);
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVCVdencMain10bitSCC, codecInfo->hevcVdenc);
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVCVdencMain444SCC, codecInfo->hevcVdenc);
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVCVdencMain10bit444SCC, codecInfo->hevcVdenc);

        /* HEVC 12bit Decoding. Currently it is enabled */
        MEDIA_WR_SKU(skuTable, FtrIntelHEVCVLDMain12bit420Decoding, 1);
        MEDIA_WR_SKU(skuTable, FtrIntelHEVCVLDMain12bit422Decoding, 1);
        MEDIA_WR_SKU(skuTable, FtrIntelHEVCVLDMain12bit444Decoding, 1);

        /* VP9 8 bit 444 */
        MEDIA_WR_SKU(skuTable, FtrIntelVP9VLDProfile1Decoding8bit444, 1);
        /* VP9 10 Bit 444*/
        MEDIA_WR_SKU(skuTable, FtrIntelVP9VLDProfile3Decoding10bit444, 1);
        /* VP9 12 bit 420/444 */
        MEDIA_WR_SKU(skuTable, FtrIntelVP9VLDProfile2Decoding12bit420, 1);
        MEDIA_WR_SKU(skuTable, FtrIntelVP9VLDProfile3Decoding12bit444, 1);

        /* VP9 VDENC 8Bit 444 */
        MEDIA_WR_SKU(skuTable, FtrEncodeVP9Vdenc8bit444, codecInfo->vp9Vdenc);
        /* VP9 VDENC 10Bit 420/444 */
        MEDIA_WR_SKU(skuTable, FtrEncodeVP9Vdenc10bit420, codecInfo->vp9Vdenc);
        MEDIA_WR_SKU(skuTable, FtrEncodeVP9Vdenc10bit444, codecInfo->vp9Vdenc);
    }

    MEDIA_WR_SKU(skuTable, FtrEnableMediaKernels, drvInfo->hasHuc);

    if (devInfo->eGTType == GTTYPE_GT1)
    {
        MEDIA_WR_SKU(skuTable, FtrGT1, 1);
    }
    else if (devInfo->eGTType == GTTYPE_GT1_5)
    {
        MEDIA_WR_SKU(skuTable, FtrGT1_5, 1);
    }
    else if (devInfo->eGTType == GTTYPE_GT2)
    {
        MEDIA_WR_SKU(skuTable, FtrGT2, 1);
    }
    else if (devInfo->eGTType == GTTYPE_GT3)
    {
        MEDIA_WR_SKU(skuTable, FtrGT3, 1);
    }
    else if (devInfo->eGTType == GTTYPE_GT4)
    {
        MEDIA_WR_SKU(skuTable, FtrGT4, 1);
    }
    else
    {
        /* GT1 is by default */
        MEDIA_WR_SKU(skuTable, FtrGT1, 1);
    }

    MEDIA_WR_SKU(skuTable, FtrVERing, drvInfo->hasVebox);
    MEDIA_WR_SKU(skuTable, FtrPPGTT, drvInfo->hasPpgtt);
    MEDIA_WR_SKU(skuTable, FtrEDram, devInfo->hasERAM);

    /* Virtual VDBOX ring is used on Gen12 */
    MEDIA_WR_SKU(skuTable, FtrVcs2,  0);

    MEDIA_WR_SKU(skuTable, FtrSingleVeboxSlice, 1);
    if (devInfo->SubSliceCount >= singleVeboxSubSliceNumMax)
    {
        MEDIA_WR_SKU(skuTable, FtrSingleVeboxSlice, 0);
    }

    MEDIA_WR_SKU(skuTable, FtrSFCPipe, 1);

    MEDIA_WR_SKU(skuTable, FtrHCP2SFCPipe, 1);
    MEDIA_WR_SKU(skuTable, FtrSSEUPowerGating, 1);
    MEDIA_WR_SKU(skuTable, FtrSSEUPowerGatingControlByUMD, 1);

    MEDIA_WR_SKU(skuTable, FtrPerCtxtPreemptionGranularityControl, 1);

    /* It is disabled by default. It can be enabled based on HW */
    MEDIA_WR_SKU(skuTable, FtrMemoryCompression, 0);
    MEDIA_WR_SKU(skuTable, FtrHcpDecMemoryCompression, 0);
    MEDIA_WR_SKU(skuTable, Ftr10bitDecMemoryCompression, 0);

    MEDIA_WR_SKU(skuTable, FtrCCSNode, 1);

    MEDIA_WR_SKU(skuTable, FtrVpP010Output, 1);
    MEDIA_WR_SKU(skuTable, FtrVp10BitSupport, 1);
    MEDIA_WR_SKU(skuTable, FtrVp16BitSupport, 1);

    MEDIA_WR_SKU(skuTable, FtrContextBasedScheduling, 1);

    MEDIA_WR_SKU(skuTable, FtrSWMediaReset, 1);

    MEDIA_WR_SKU(skuTable, FtrVeboxScalabilitywith4K, 1);

    MEDIA_WR_SKU(skuTable, FtrTileY, 1);
    MEDIA_WR_SKU(skuTable, FtrE2ECompression, 1);
    MEDIA_WR_SKU(skuTable, FtrLinearCCS, 1);
    MEDIA_WR_SKU(skuTable, FtrFlatPhysCCS, 0);

    MEDIA_WR_SKU(skuTable, FtrHeight8AlignVE3DLUTDualPipe, 1);

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
        MEDIA_WR_SKU(skuTable, FtrE2ECompression, 0);
    }

    // Create uncompressible surface by default
    MEDIA_WR_SKU(skuTable, FtrCompressibleSurfaceDefault, 0);

     bool compressibleSurfaceEnable = false;

    ReadUserSetting(userSettingPtr,
        compressibleSurfaceEnable,
        "Enable Compressible Surface Creation",
        MediaUserSetting::Group::Device);
        
#ifdef _MMC_SUPPORTED
    if (compressibleSurfaceEnable)
    {
        MEDIA_WR_SKU(skuTable, FtrCompressibleSurfaceDefault, 1);
    }
#endif

    if (drvInfo->devId == 0xFF20)
    {
        MEDIA_WR_SKU(skuTable, FtrConditionalBatchBuffEnd, 1);
    }

    MEDIA_WR_SKU(skuTable, FtrUseSwSwizzling, 1);

    MEDIA_WR_SKU(skuTable, FtrMemoryRemapSupport, 1);

    MEDIA_WR_SKU(skuTable, FtrAV1VLDLSTDecoding, 0);

    //Tile64
    if (GFX_IS_DG2_G11_CONFIG(drvInfo->devId))
    {
        MEDIA_WR_SKU(skuTable, FtrTileY, 0);
    }

    return true;
}

static bool InitTglMediaWaExt(struct GfxDeviceInfo *devInfo,
                             MediaWaTable *waTable,
                             struct LinuxDriverInfo *drvInfo)
{
    if ((devInfo == nullptr) || (waTable == nullptr) || (drvInfo == nullptr))
    {
        DEVINFO_ERROR("null ptr is passed\n");
        return false;
    }

    MEDIA_WR_WA(waTable, WaForceGlobalGTT, !drvInfo->hasPpgtt);
    MEDIA_WR_WA(waTable, WaMidBatchPreemption, 0);
    MEDIA_WR_WA(waTable, WaArbitraryNumMbsInSlice, 1);

    MEDIA_WR_WA(waTable, WaSuperSliceHeaderPacking, 1);

    MEDIA_WR_WA(waTable, WaSFC270DegreeRotation, 0);

    MEDIA_WR_WA(waTable, WaEnableYV12BugFixInHalfSliceChicken7, 1);

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_AUX_TABLE_16K_GRANULAR_ID,
        &userFeatureData,
        (MOS_CONTEXT_HANDLE)nullptr);

    MEDIA_WR_WA(waTable, WaDummyReference, 1);

    MEDIA_WR_WA(waTable, Wa16KInputHeightNV12Planar420, 1);

    /*software wa to disable calculate the UV offset by gmmlib
      CPU blt call will add/remove padding on the platform*/
    MEDIA_WR_WA(waTable, WaDisableGmmLibOffsetInDeriveImage, 1);

    /*software wa to fix some corner cases of HEVC/VP9 SFC and Scalability*/
    MEDIA_WR_WA(waTable, Wa_14010222001, 1);

    /*software wa to fix some corner hang cases for Scalability*/
    MEDIA_WR_WA(waTable, Wa_2209620131, 1);

    
    if (drvInfo->devId == 0x9A49 ||
        drvInfo->devId == 0x9AC9 ||
        drvInfo->devId == 0x9A60 ||
        drvInfo->devId == 0x9A68 ||
        drvInfo->devId == 0x9A70 ||
        drvInfo->devId == 0x4905 ||
        drvInfo->devId == 0x4906 ||
        drvInfo->devId == 0x4907)
    {
        /*software wa to use frame based decoding for AV1 decode*/
        MEDIA_WR_WA(waTable, Wa_1409820462, 1);
    }

    if (drvInfo->devId == 0x4905 ||
        drvInfo->devId == 0x4906 ||
        drvInfo->devId == 0x4907)
    {
        /* Turn off MMC for codec, need to remove once turn it on */
        MEDIA_WR_WA(waTable, WaDisableCodecMmc, 1);

        /* Turn off MMC for VPP, need to remove once turn it on */
        MEDIA_WR_WA(waTable, WaDisableVPMmc, 1);
    }

    if (MEDIA_IS_WA(waTable, Wa_14012254246))
    {
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_DISABLE_TLB_PREFETCH_ID,
            &userFeatureData,
            (MOS_CONTEXT_HANDLE)nullptr);
        // For PVC A0, if TLB pre-fetch disabled, resource could be allocated to system memory and local memory;
        // otherwise, resource could be allocated to local memory only.
        if (userFeatureData.u32Data == 0)
        {
            MEDIA_WR_WA(waTable, WaForceAllocateLML2, 1);
            MEDIA_WR_WA(waTable, WaForceAllocateLML3, 0);
            MEDIA_WR_WA(waTable, WaForceAllocateLML4, 0);
        }
    }

    // For DG2
    if (GFX_IS_DG2_G11_CONFIG(drvInfo->devId))
    {
        MEDIA_WR_WA(waTable, Wa_22011549751, 1);
    }

    MEDIA_WR_WA(waTable, WaEnableVPPCopy, 1);
    MEDIA_WR_WA(waTable, Wa_AvcUnalignedHeight, 1);
    return true;
}

static bool InitXehpSDVMediaSku(struct GfxDeviceInfo *devInfo,
                             MediaFeatureTable *skuTable,
                             struct LinuxDriverInfo *drvInfo,
                             MediaUserSettingSharedPtr userSettingPtr)
{
    if (!InitTglMediaSkuExt(devInfo, skuTable, drvInfo, &XehpSdvCodecInfo, userSettingPtr))
    {
        return false;
    }

    // Guc Submission
    MEDIA_WR_SKU(skuTable, FtrGucSubmission, 1);

    // doesn't support legacy TileY
    MEDIA_WR_SKU(skuTable, FtrTileY, 0);
    MEDIA_WR_SKU(skuTable, FtrE2ECompression, 1);
    MEDIA_WR_SKU(skuTable, FtrLinearCCS, 1);
    MEDIA_WR_SKU(skuTable, FtrFlatPhysCCS, 1);
    MEDIA_WR_SKU(skuTable, FtrCompsitionMemoryCompressedOut, 1);

    //Enable LocalMemory for all dGraphics
    MEDIA_WR_SKU(skuTable, FtrLocalMemory, 1);

    MEDIA_WR_SKU(skuTable, FtrConditionalBatchBuffEnd, 1);
    return true;
}

static bool InitXehpSDVMediaWa(struct GfxDeviceInfo *devInfo,
                             MediaWaTable *waTable,
                             struct LinuxDriverInfo *drvInfo)
{
    if (!InitTglMediaWaExt(devInfo, waTable, drvInfo))
    {
        DEVINFO_ERROR("InitMediaWA failed\n");
        return false;
    }
    /*software wa to use frame based decoding for AV1 decode*/
    MEDIA_WR_WA(waTable, Wa_1409820462, 1);

    /*software wa to add a dummy workload in front of
        normal workload per batch buffer*/
    MEDIA_WR_WA(waTable, Wa_1508208842, 1);

    /* Turn off MMC for codec, need to remove once turn it on */
    MEDIA_WR_WA(waTable, WaDisableCodecMmc, 1);

    /* Turn off MMC for VPP, need to remove once turn it on */
    MEDIA_WR_WA(waTable, WaDisableVPMmc, 1);

    /* default resource allocate policy for XeHP_SDV*/
    MEDIA_WR_WA(waTable, WaForceAllocateLML2, 0);

    /* only enable L3 for A step*/
    if (drvInfo->devRev == 0x0 || drvInfo->devRev == 0x1)
    {
       MEDIA_WR_WA(waTable, WaForceAllocateLML3, 1);
    }
    else
    {
       MEDIA_WR_WA(waTable, WaForceAllocateLML3, 0);
    }
    MEDIA_WR_WA(waTable, WaForceAllocateLML4, 0);

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_LOCAL_MEMORY_LEVEL_SWITCH_ID,
        &userFeatureData,
        (MOS_CONTEXT_HANDLE)nullptr);

    if(userFeatureData.u32Data == 2)
    {
        MEDIA_WR_WA(waTable, WaForceAllocateLML2, 1);
        MEDIA_WR_WA(waTable, WaForceAllocateLML3, 0);
        MEDIA_WR_WA(waTable, WaForceAllocateLML4, 0);
    }

    if(userFeatureData.u32Data == 3)
    {
        MEDIA_WR_WA(waTable, WaForceAllocateLML2, 0);
        MEDIA_WR_WA(waTable, WaForceAllocateLML3, 1);
        MEDIA_WR_WA(waTable, WaForceAllocateLML4, 0);
    }

    if(userFeatureData.u32Data == 4)
    {
        MEDIA_WR_WA(waTable, WaForceAllocateLML2, 0);
        MEDIA_WR_WA(waTable, WaForceAllocateLML3, 1);
        MEDIA_WR_WA(waTable, WaForceAllocateLML4, 1);
    }

    if (char* pLocalMemLevelSwitch = getenv("LOCALMEMLEVELSWITCH"))
    {
        int localMemLevelSwitch = atoi(pLocalMemLevelSwitch);

        if(localMemLevelSwitch == 2)
        {
            MEDIA_WR_WA(waTable, WaForceAllocateLML2, 1);
            MEDIA_WR_WA(waTable, WaForceAllocateLML3, 0);
            MEDIA_WR_WA(waTable, WaForceAllocateLML4, 0);
        }
        else if(localMemLevelSwitch == 3)
        {
            MEDIA_WR_WA(waTable, WaForceAllocateLML2, 0);
            MEDIA_WR_WA(waTable, WaForceAllocateLML3, 1);
            MEDIA_WR_WA(waTable, WaForceAllocateLML4, 0);
        }
        else if(localMemLevelSwitch == 4)
        {
            MEDIA_WR_WA(waTable, WaForceAllocateLML2, 0);
            MEDIA_WR_WA(waTable, WaForceAllocateLML3, 1);
            MEDIA_WR_WA(waTable, WaForceAllocateLML4, 1);
        }
    }
    // XeHP_SDV I as P WA
    MEDIA_WR_WA(waTable, Wa_22011549751, 1);
#ifdef IGFX_XEHP_SDV_ENABLE_NON_UPSTREAM
    MEDIA_WR_WA(waTable, WaHEVCVDEncForceDeltaQpRoiNotSupported, 1);
#else
    MEDIA_WR_WA(waTable, WaHEVCVDEncForceDeltaQpRoiNotSupported, 0);
#endif
    return true;
}

static bool InitPvcMediaSku(struct GfxDeviceInfo *devInfo,
                             MediaFeatureTable *skuTable,
                             struct LinuxDriverInfo *drvInfo,
                             MediaUserSettingSharedPtr userSettingPtr)
{
    if (!InitTglMediaSkuExt(devInfo, skuTable, drvInfo, &PvcCodecInfo, userSettingPtr))
    {
        return false;
    }

    // PVC doesn't have VEBOX and SFC
    MEDIA_WR_SKU(skuTable, FtrSingleVeboxSlice, 0);
    MEDIA_WR_SKU(skuTable, FtrSFCPipe, 0);
    MEDIA_WR_SKU(skuTable, FtrVERing, 0);
    // PVC doesn't support legacy TileY
    MEDIA_WR_SKU(skuTable, FtrTileY, 0);
    MEDIA_WR_SKU(skuTable, FtrVERing, 0);

    MEDIA_WR_SKU(skuTable, FtrVeboxScalabilitywith4K, 0);

    // Compression Flags
    if (!(drvInfo->devId == 0x0BD0 ||
       (drvInfo->devId == 0x0BD5 && drvInfo->devRev == 0x03)))
    {
        MEDIA_WR_SKU(skuTable, FtrLinearCCS, 1);
        MEDIA_WR_SKU(skuTable, FtrFlatPhysCCS, 1);
        MEDIA_WR_SKU(skuTable, FtrE2ECompression, 1);
        MEDIA_WR_SKU(skuTable, FtrRenderCompressionOnly, 1);
        MEDIA_WR_SKU(skuTable, FtrCompsitionMemoryCompressedOut, 1);
    }

    // Force MMC Turn on for all components if set reg key
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_FORCE_MMC_ON_ID,
        &userFeatureData,
        (MOS_CONTEXT_HANDLE)nullptr);

    if (userFeatureData.bData)
    {
        MEDIA_WR_SKU(skuTable, FtrE2ECompression, 1);
    }

    // Disable MMC for all components if set reg key
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_DISABLE_MMC_ID,
        &userFeatureData,
        (MOS_CONTEXT_HANDLE)nullptr);
    if (userFeatureData.bData)
    {
        MEDIA_WR_SKU(skuTable, FtrE2ECompression, 0);
    }

    MEDIA_WR_SKU(skuTable, FtrWithSlimVdbox, 1);

    // Enable LocalMemory for all dGraphics
    MEDIA_WR_SKU(skuTable, FtrLocalMemory, 1);

    //Enable AV1 Large Scale Tile decoding
    MEDIA_WR_SKU(skuTable, FtrAV1VLDLSTDecoding, 1);

    return true;
}

static bool InitPvcMediaWa(struct GfxDeviceInfo *devInfo,
                             MediaWaTable *waTable,
                             struct LinuxDriverInfo *drvInfo)
{
    if (!InitTglMediaWaExt(devInfo, waTable, drvInfo))
    {
        DEVINFO_ERROR("InitMediaWA failed\n");
        return false;
    }

    // turn on Compression on B Step+
    if (drvInfo->devId == 0x0BD0 ||
       (drvInfo->devId == 0x0BD5 && drvInfo->devRev == 0x03))
    {
        /* Turn off MMC for codec, need to remove once turn it on */
        MEDIA_WR_WA(waTable, WaDisableCodecMmc, 1);

        /* Turn off MMC for VPP, need to remove once turn it on */
        MEDIA_WR_WA(waTable, WaDisableVPMmc, 1);
    }

    // For PVC A0 dev, disable TLB prefetch WA
    if (drvInfo->devId == 0x0BD0 || drvInfo->devId == 0x0BD5)
    {
        MEDIA_WR_WA(waTable, Wa_14012254246, 1);
    }

    // for PVC XL/XT A0 dev, need add MEM policy WA, B0 doesn't need.
    if (drvInfo->devId == 0x0BD0 ||
       (drvInfo->devId == 0x0BD5 && drvInfo->devRev == 0x03))
    {
        /* default resource allocate policy for PVC */
        MEDIA_WR_WA(waTable, WaForceAllocateLML2, 0);
        MEDIA_WR_WA(waTable, WaForceAllocateLML3, 1);
        MEDIA_WR_WA(waTable, WaForceAllocateLML4, 0);

        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_LOCAL_MEMORY_LEVEL_SWITCH_ID,
            &userFeatureData,
            (MOS_CONTEXT_HANDLE)nullptr);

        if(userFeatureData.u32Data == 2)
        {
            MEDIA_WR_WA(waTable, WaForceAllocateLML2, 1);
            MEDIA_WR_WA(waTable, WaForceAllocateLML3, 0);
            MEDIA_WR_WA(waTable, WaForceAllocateLML4, 0);
        }
                
        if(userFeatureData.u32Data == 3)
        {
            MEDIA_WR_WA(waTable, WaForceAllocateLML2, 0);
            MEDIA_WR_WA(waTable, WaForceAllocateLML3, 1);
            MEDIA_WR_WA(waTable, WaForceAllocateLML4, 0);
        }

        if(userFeatureData.u32Data == 4)
        {
            MEDIA_WR_WA(waTable, WaForceAllocateLML2, 0);
            MEDIA_WR_WA(waTable, WaForceAllocateLML3, 1);
            MEDIA_WR_WA(waTable, WaForceAllocateLML4, 1);
        }

        if (char* pLocalMemLevelSwitch = getenv("LOCALMEMLEVELSWITCH"))
        {
            int localMemLevelSwitch = atoi(pLocalMemLevelSwitch);

            if(localMemLevelSwitch == 2)
            {
                MEDIA_WR_WA(waTable, WaForceAllocateLML2, 1);
                MEDIA_WR_WA(waTable, WaForceAllocateLML3, 0);
                MEDIA_WR_WA(waTable, WaForceAllocateLML4, 0);
            }
            else if(localMemLevelSwitch == 3)
            {
                MEDIA_WR_WA(waTable, WaForceAllocateLML2, 0);
                MEDIA_WR_WA(waTable, WaForceAllocateLML3, 1);
                MEDIA_WR_WA(waTable, WaForceAllocateLML4, 0);
            }
            else if(localMemLevelSwitch == 4)
            {
                MEDIA_WR_WA(waTable, WaForceAllocateLML2, 0);
                MEDIA_WR_WA(waTable, WaForceAllocateLML3, 1);
                MEDIA_WR_WA(waTable, WaForceAllocateLML4, 1);
            }
        }
    }

#ifdef IGFX_PVC_ENABLE_NON_UPSTREAM
    MEDIA_WR_WA(waTable, WaHEVCVDEncForceDeltaQpRoiNotSupported, 1);
#else
    MEDIA_WR_WA(waTable, WaHEVCVDEncForceDeltaQpRoiNotSupported, 0);
#endif

    // I as P WA
    MEDIA_WR_WA(waTable, Wa_22011549751, 1);

    // Remove the WA of DummyReference
    MEDIA_WR_WA(waTable, WaDummyReference, 0);

    return true;
}

static bool InitDg2MediaSku(struct GfxDeviceInfo *devInfo,
                             MediaFeatureTable *skuTable,
                             struct LinuxDriverInfo *drvInfo,
                             MediaUserSettingSharedPtr userSettingPtr)
{
    if (!InitTglMediaSkuExt(devInfo, skuTable, drvInfo, &Dg2CodecInfo, userSettingPtr))
    {
        return false;
    }

    // Disable CCS on DG2A
    if (drvInfo->devRev == 0)
    {
        MEDIA_WR_SKU(skuTable, FtrCCSNode, 0);
    }

    if (drvInfo->hasBsd)
    {
        /* AV1 VDENC 8/10Bit 420 */
        MEDIA_WR_SKU(skuTable, FtrEncodeAV1Vdenc, 1);
        MEDIA_WR_SKU(skuTable, FtrEncodeAV1Vdenc10bit420, 1);
    }

    MEDIA_WR_SKU(skuTable, FtrSfcScalability, 1);

    // Guc Submission
    MEDIA_WR_SKU(skuTable, FtrGucSubmission, 1);

    MEDIA_WR_SKU(skuTable, FtrTileY, 0);
    MEDIA_WR_SKU(skuTable, FtrE2ECompression, 1);
    MEDIA_WR_SKU(skuTable, FtrLinearCCS, 1);
    MEDIA_WR_SKU(skuTable, FtrFlatPhysCCS, 1);
    MEDIA_WR_SKU(skuTable, FtrCompsitionMemoryCompressedOut, 1);

    // Enable LocalMemory for all dGraphics
    MEDIA_WR_SKU(skuTable, FtrLocalMemory, 1);

    // Enable AV1 Large Scale Tile decoding
    MEDIA_WR_SKU(skuTable, FtrAV1VLDLSTDecoding, 1);

    // Enable HDR
    MEDIA_WR_SKU(skuTable, FtrHDR, 1);

    // Enable down scaling first if 3DLUT enabled
    MEDIA_WR_SKU(skuTable, FtrScalingFirst, 1);

    // Disable HEVC 422 Virtual Tile Scalability
    MEDIA_WR_SKU(skuTable, FtrDecodeHEVC422VTScalaDisable, 1);

    // Tile64
    if (drvInfo->devRev >= 0x04 &&
        (drvInfo->devId == 0x4F80 ||
         drvInfo->devId == 0x4F81 ||
         drvInfo->devId == 0x4F82 ||
         drvInfo->devId == 0x4F83 ||
         drvInfo->devId == 0x4F84 ||
         drvInfo->devId == 0x4F85 ||
         drvInfo->devId == 0x4F86 ||
         drvInfo->devId == 0x5690 ||
         drvInfo->devId == 0x5691 ||
         drvInfo->devId == 0x5692 ||
         drvInfo->devId == 0x5695 ||
         drvInfo->devId == 0x5696 ||
         drvInfo->devId == 0x5697 ||
         drvInfo->devId == 0x5698 ||
         drvInfo->devId == 0x56A0 ||
         drvInfo->devId == 0x56A1 ||
         drvInfo->devId == 0x56A2 ||
         drvInfo->devId == 0x56A3 ||
         drvInfo->devId == 0x56A4 ||
         drvInfo->devId == 0x56A5 ||
         drvInfo->devId == 0x56A6 ||
         drvInfo->devId == 0x56A7 ||
         drvInfo->devId == 0x56A8 ||
         drvInfo->devId == 0x56A9 ||
         drvInfo->devId == 0x56C0))
    {
        MEDIA_WR_SKU(skuTable, FtrTileY, 0);
    }

    MEDIA_WR_SKU(skuTable, FtrHDR, 1);

    // Enable HVS Denoise
    MEDIA_WR_SKU(skuTable, FtrHVSDenoise, 1);

    #define IS_SERVER_SKU(d) (((d) >= 0x56C0) && ((d) <= 0x56C1))
    if (IS_SERVER_SKU(drvInfo->devId))
    {
        drvInfo->isServer = 1;
    }
    else
    {
        drvInfo->isServer = 0;
    }

    return true;
}

static bool InitDg2MediaWa(struct GfxDeviceInfo *devInfo,
                             MediaWaTable *waTable,
                             struct LinuxDriverInfo *drvInfo)
{
    if (!InitTglMediaWaExt(devInfo, waTable, drvInfo))
    {
        DEVINFO_ERROR("InitMediaWA failed\n");
        return false;
    }

    /* Turn off MMC for codec, need to remove once turn it on */
    MEDIA_WR_WA(waTable, WaDisableCodecMmc, 1);

    /* Turn off MMC for VPP, need to remove once turn it on */
    MEDIA_WR_WA(waTable, WaDisableVPMmc, 1);

    // Compression Settings
    if  (SI_WA_FROM(drvInfo->devRev, ACM_G10_MEDIA_REV_ID_B0) ||
         GFX_IS_DG2_G11_CONFIG(drvInfo->devId))
    {
        /* Turn on MMC for codec and vpp*/
        MEDIA_WR_WA(waTable, WaDisableCodecMmc, 0);
        MEDIA_WR_WA(waTable, WaDisableVPMmc, 0);
    }

    if (GFX_IS_DG2_G11_CONFIG(drvInfo->devId) ||
       (GFX_IS_DG2_G10_CONFIG(drvInfo->devId) && SI_WA_FROM(drvInfo->devRev, ACM_G10_MEDIA_REV_ID_B0)))
    {
        MEDIA_WR_WA(waTable, Wa_22011700429, 1);
    }

    // For DG2 128/512 need to distinguish A stepping and B stepping
    // Disable B+ stepping features on DG2A
    if (GFX_IS_DG2_G10_CONFIG(drvInfo->devId) ||
        GFX_IS_DG2_G11_CONFIG(drvInfo->devId))
    {
        if (drvInfo->devRev < 4)
        {
            MEDIA_WR_WA(waTable, WaEnableOnlyASteppingFeatures, 1);
            MEDIA_WR_WA(waTable, Wa_14010476401, 1);
            MEDIA_WR_WA(waTable, Wa_22011531258, 1);
            MEDIA_WR_WA(waTable, Wa_2209975292, 1);
            MEDIA_WR_WA(waTable, WaHEVCVDEncForceDeltaQpRoiNotSupported, 1);
        }
        MEDIA_WR_WA(waTable, Wa_22011549751, 1);
    }

    // for DG2 256EU, support B+ features, don't need wa for A stepping
    if (GFX_IS_DG2_G12_CONFIG(drvInfo->devId))
    {
        MEDIA_WR_WA(waTable, Wa_22011549751, 1);
    }

    // CCS Chromakey
    if (GFX_IS_DG2_G11_CONFIG(drvInfo->devId))
    {
        MEDIA_WR_WA(waTable, Wa_16011481064, 1);
    }
    else
    {
        if (drvInfo->devRev >= 4)
        {
            MEDIA_WR_WA(waTable, Wa_16011481064, 1);
        }
    }

    MEDIA_WR_WA(waTable, Wa_15010089951, 1);

    // Remove the WA of DummyReference
    MEDIA_WR_WA(waTable, WaDummyReference, 0);

    MEDIA_WR_WA(waTable, WaDisableSetObjectCapture, 1);

    MEDIA_WR_WA(waTable, Wa_15013355402, 1);

    MEDIA_WR_WA(waTable, Wa_AvcUnalignedHeight, 0);

    return true;
}


static struct LinuxDeviceInit xehpSDVDeviceInit =
{
    .productFamily    = IGFX_XE_HP_SDV,
    .InitMediaFeature = InitXehpSDVMediaSku,
    .InitMediaWa      = InitXehpSDVMediaWa,
};

static bool xehpSDVDeviceRegister = DeviceInfoFactory<LinuxDeviceInit>::
    RegisterDevice(IGFX_XE_HP_SDV, &xehpSDVDeviceInit);

static struct LinuxDeviceInit dg2DeviceInit =
{
    .productFamily    = IGFX_DG2,
    .InitMediaFeature = InitDg2MediaSku,
    .InitMediaWa      = InitDg2MediaWa,
};

static bool dg2DeviceRegister = DeviceInfoFactory<LinuxDeviceInit>::
    RegisterDevice(IGFX_DG2, &dg2DeviceInit);

static struct LinuxDeviceInit pvcDeviceInit =
{
    .productFamily    = IGFX_PVC,
    .InitMediaFeature = InitPvcMediaSku,
    .InitMediaWa      = InitPvcMediaWa,
};

static bool pvcDeviceRegister = DeviceInfoFactory<LinuxDeviceInit>::
    RegisterDevice(IGFX_PVC, &pvcDeviceInit);
