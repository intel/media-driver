/*
* Copyright (c) 2018-2023, Intel Corporation
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
//! \file     media_sku_wa_g12.cpp
//!

#include "igfxfmid.h"
#include "linux_system_info.h"
#include "skuwa_factory.h"
#include "linux_skuwa_debug.h"
#include "linux_media_skuwa.h"
#include "mos_utilities.h"
#include "mos_os_specific.h"
#include "media_user_setting_specific.h"

static constexpr uint32_t singleVeboxSubSliceNumMax = 24;

//extern template class DeviceInfoFactory<GfxDeviceInfo>;
typedef DeviceInfoFactory<LinuxDeviceInit> DeviceInit;

static struct LinuxCodecInfo tglCodecInfo =
{
    .avcDecoding    = 1,
    .mpeg2Decoding  = 1,
    .vp8Decoding    = 1,
    .vc1Decoding    = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .jpegDecoding   = 1,
    .avcEncoding    = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .mpeg2Encoding  = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .hevcDecoding   = 1,
    .hevcEncoding   = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .jpegEncoding   = 1,
    .avcVdenc       = 1,
    .vp9Decoding    = 1,
    .hevc10Decoding = 1,
    .vp9b10Decoding = 1,
    .hevc10Encoding = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .hevc12Encoding = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .vp8Encoding    = 0,
    .hevcVdenc      = 1,
    .vp9Vdenc       = 1,
    .adv0Decoding   = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .adv1Decoding   = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
};

static bool InitTglMediaSku(struct GfxDeviceInfo *devInfo,
                             MediaFeatureTable *skuTable,
                             struct LinuxDriverInfo *drvInfo,
                             MediaUserSettingSharedPtr userSettingPtr)
{
    if ((devInfo == nullptr) || (skuTable == nullptr) || (drvInfo == nullptr))
    {
        DEVINFO_ERROR("null ptr is passed\n");
        return false;
    }

    if (drvInfo->hasBsd)
    {
        LinuxCodecInfo *codecInfo = &tglCodecInfo;

        if (devInfo->productFamily == IGFX_TIGERLAKE_LP &&
            drvInfo->devRev == 0)
        {
            codecInfo->adv0Decoding = 0;
            codecInfo->adv1Decoding = 0;
        }

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
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVC12bit, codecInfo->hevc12Encoding);
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVC10bit422, codecInfo->hevc10Encoding);
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVC12bit422, codecInfo->hevc12Encoding);
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
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVCVdencMain422, 0);
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVCVdencMain10bit422, 0);
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
    MEDIA_WR_SKU(skuTable, FtrVcs2, 0);

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

    MEDIA_WR_SKU(skuTable, FtrRAMode, 1);
    MEDIA_WR_SKU(skuTable, FtrVpP010Output, 1);
    MEDIA_WR_SKU(skuTable, FtrVp10BitSupport, 1);
    MEDIA_WR_SKU(skuTable, FtrVp16BitSupport, 1);

    MEDIA_WR_SKU(skuTable, FtrContextBasedScheduling, 1);
    MEDIA_WR_SKU(skuTable, FtrSfcScalability, 1);

    MEDIA_WR_SKU(skuTable, FtrTileY, 1);

    bool disableMMC = false;
    MEDIA_WR_SKU(skuTable, FtrE2ECompression, 1);

    if (drvInfo->devRev <= 2)
    {
        // turn off Compression as HW known issue
        MEDIA_WR_SKU(skuTable, FtrE2ECompression, 0);
    }

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
        MEDIA_WR_SKU(skuTable, FtrE2ECompression, 0);
    }

    // Force MMC Turn on for all components if set reg key
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

    MEDIA_WR_SKU(skuTable, FtrLinearCCS, 1);

    MEDIA_WR_SKU(skuTable, FtrUseSwSwizzling, 1);
    MEDIA_WR_SKU(skuTable, FtrScalingFirst, 1);

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

    MEDIA_WR_SKU(skuTable, FtrHDR, 1);

    return true;
}

static bool InitTglMediaWa(struct GfxDeviceInfo *devInfo,
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

    /*software wa to add a dummy workload in front of
      normal workload per batch buffer*/
    MEDIA_WR_WA(waTable, Wa_1508208842, 1);

    MEDIA_WR_WA(waTable, Wa16KInputHeightNV12Planar420, 1);

    /*software wa to disable calculate the UV offset by gmmlib
      CPU blt call will add/remove padding on the platform*/
    MEDIA_WR_WA(waTable, WaDisableGmmLibOffsetInDeriveImage, 1);

    /*software wa to fix some corner cases of HEVC/VP9 SFC and Scalability*/
    MEDIA_WR_WA(waTable, Wa_14010222001, 1);

    /*software wa to fix some corner hang cases for Scalability*/
    MEDIA_WR_WA(waTable, Wa_2209620131, 1);

    /*software wa to prevent error propagation for vertical intra refresh on H264 VDEnc*/
    MEDIA_WR_WA(waTable, Wa_18011246551, 1);

    MEDIA_WR_WA(waTable, WaDisableVeboxFor8K, 1);

    if (drvInfo->devRev == 0)
    {
        /* Turn off MMC for codec, need to remove once turn it on */
        MEDIA_WR_WA(waTable, WaDisableCodecMmc, 1);

        /* Turn off MMC for VPP, need to remove once turn it on */
        MEDIA_WR_WA(waTable, WaDisableVPMmc, 1);
    }

    /*software wa to use frame based decoding for AV1 decode*/
    MEDIA_WR_WA(waTable, Wa_1409820462, 1);

    /*software wa to use huc copy for init aux table for MFX*/
    MEDIA_WR_WA(waTable, Wa_22010493002, 1);

    MEDIA_WR_WA(waTable, Wa_Vp9UnalignedHeight, 1);
    MEDIA_WR_WA(waTable, Wa_AvcUnalignedHeight, 1);
    MEDIA_WR_WA(waTable, WaDisableSetObjectCapture, 0);
    return true;
}


#ifdef IGFX_GEN12_DG1_SUPPORTED
static bool InitDG1MediaSku(struct GfxDeviceInfo *devInfo,
                            MediaFeatureTable *skuTable,
                            struct LinuxDriverInfo *drvInfo,
                            MediaUserSettingSharedPtr userSettingPtr)
{
    if (!InitTglMediaSku(devInfo, skuTable, drvInfo, userSettingPtr))
    {
        return false;
    }
    MEDIA_WR_SKU(skuTable, FtrLocalMemory, 1);

    if (drvInfo->devRev > 0)
    {
        MEDIA_WR_SKU(skuTable, FtrAV1VLDLSTDecoding, 1);
    }
    MEDIA_WR_SKU(skuTable, FtrIntelVP8VLDDecoding, 0);

    bool enableCodecMMC = false;
    bool enableVPMMC    = false;

    // Disable MMC for all components if set reg key
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_ENABLE_MMC_ID,
        &userFeatureData,
        (MOS_CONTEXT_HANDLE)nullptr);
    if (userFeatureData.bData)
    {
        enableVPMMC = true;
    }

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_CODEC_MMC_ENABLE_ID,
        &userFeatureData,
        (MOS_CONTEXT_HANDLE)nullptr);
    if (userFeatureData.bData)
    {
        enableCodecMMC = true;
    }

    if (!enableCodecMMC && !enableVPMMC)
    {
        MEDIA_WR_SKU(skuTable, FtrE2ECompression, 0);
    }

    return true;
}

static bool InitDG1MediaWa(struct GfxDeviceInfo *devInfo,
                                   MediaWaTable *waTable,
                         struct LinuxDriverInfo *drvInfo)
{
    if (!InitTglMediaWa(devInfo, waTable, drvInfo))
    {
        DEVINFO_ERROR("InitMediaWA failed\n");
        return false;
    }

    /* Turn off MMC for codec, need to remove once turn it on */
    MEDIA_WR_WA(waTable, WaDisableCodecMmc, 1);

    /* Turn off MMC for VPP, need to remove once turn it on */
    MEDIA_WR_WA(waTable, WaDisableVPMmc, 1);

    /* Enable VPP copy */
    MEDIA_WR_WA(waTable, WaEnableVPPCopy, 1);
    return true;
}

static struct LinuxDeviceInit dg1DeviceInit =
{
     .productFamily    = IGFX_DG1,
     .InitMediaFeature = InitDG1MediaSku,
     .InitMediaWa      = InitDG1MediaWa,
};

static bool dg1DeviceRegister = DeviceInfoFactory<LinuxDeviceInit>::
    RegisterDevice(IGFX_DG1, &dg1DeviceInit);

#endif

#ifdef IGFX_GEN12_RKL_SUPPORTED
static bool InitRKLMediaSku(struct GfxDeviceInfo *devInfo,
    MediaFeatureTable *                           skuTable,
    struct LinuxDriverInfo                       *drvInfo,
    MediaUserSettingSharedPtr                     userSettingPtr)
{
    if (!InitTglMediaSku(devInfo, skuTable, drvInfo, userSettingPtr))
    {
        return false;
    }

    if (devInfo->eGTType == GTTYPE_GT0_5)
    {
        MEDIA_WR_SKU(skuTable, FtrGT0_5, 1);
    }

    MEDIA_WR_SKU(skuTable, FtrSfcScalability, 0);
    MEDIA_WR_SKU(skuTable, FtrIntelVP8VLDDecoding, 0);

    // turn off Compression on RKL as HW known issue
    MEDIA_WR_SKU(skuTable, FtrE2ECompression, 0);
    return true;
}

static bool InitRKLMediaWa(struct GfxDeviceInfo *devInfo,
    MediaWaTable *                               waTable,
    struct LinuxDriverInfo *                     drvInfo)
{
    if (!InitTglMediaWa(devInfo, waTable, drvInfo))
    {
        return false;
    }

    return true;
}

static struct LinuxDeviceInit rklDeviceInit =
{
    .productFamily    = IGFX_ROCKETLAKE,
    .InitMediaFeature = InitRKLMediaSku,
    .InitMediaWa      = InitRKLMediaWa,
};

static bool rklDeviceRegister = DeviceInfoFactory<LinuxDeviceInit>::
    RegisterDevice(IGFX_ROCKETLAKE, &rklDeviceInit);

#endif

#ifdef IGFX_GEN12_ADLS_SUPPORTED
static bool InitAdlsMediaSku(struct GfxDeviceInfo *devInfo,
                             MediaFeatureTable *skuTable,
                             struct LinuxDriverInfo *drvInfo,
                             MediaUserSettingSharedPtr userSettingPtr)
{
    if (!InitTglMediaSku(devInfo, skuTable, drvInfo, userSettingPtr))
    {
        return false;
    }

    if (devInfo->eGTType == GTTYPE_GT0_5)
    {
        MEDIA_WR_SKU(skuTable, FtrGT0_5, 1);
    }

    MEDIA_WR_SKU(skuTable, FtrAV1VLDLSTDecoding, 1);

    MEDIA_WR_SKU(skuTable, FtrHeight8AlignVE3DLUTDualPipe, 1);

    //Disable VP8 for ADLS
    MEDIA_WR_SKU(skuTable, FtrIntelVP8VLDDecoding, 0);

    // Disable HEVC 422 Virtual Tile Scalability
    MEDIA_WR_SKU(skuTable, FtrDecodeHEVC422VTScalaDisable, 1);

    MEDIA_WR_SKU(skuTable, FtrVirtualTileScalabilityDisable, 1);
    MEDIA_WR_SKU(skuTable, FtrE2ECompression, 1);

    //RPL-S
    if (drvInfo->devId == 0xA780 || 
        drvInfo->devId == 0xA781 || 
        drvInfo->devId == 0xA782 || 
        drvInfo->devId == 0xA783 || 
        drvInfo->devId == 0xA788 || 
        drvInfo->devId == 0xA789 || 
        drvInfo->devId == 0xA78A || 
        drvInfo->devId == 0xA78B)
    {
        MEDIA_WR_SKU(skuTable, FtrGucSubmission, 1);
    }

    return true;
}

static bool InitAdlsMediaWa(struct GfxDeviceInfo* devInfo,
                            MediaWaTable* waTable,
                            struct LinuxDriverInfo* drvInfo)
{
    if (!InitTglMediaWa(devInfo, waTable, drvInfo))
    {
        return false;
    }

    //ADL-S not need this
    MEDIA_WR_WA(waTable, Wa_1409820462, 0);

    //ADL-S need enable MMC for all stepping
    MEDIA_WR_WA(waTable, WaDisableCodecMmc, 0);
    MEDIA_WR_WA(waTable, WaDisableVPMmc, 0);
    MEDIA_WR_WA(waTable, WaDisableClearCCS, 1);

    return true;
}

static struct LinuxDeviceInit adlsDeviceInit =
{
    .productFamily    = IGFX_ALDERLAKE_S,
    .InitMediaFeature = InitAdlsMediaSku,
    .InitMediaWa = InitAdlsMediaWa,
};

static bool adlsDeviceRegister = DeviceInfoFactory<LinuxDeviceInit>::
    RegisterDevice(IGFX_ALDERLAKE_S, &adlsDeviceInit);
#endif


#ifdef IGFX_GEN12_ADLP_SUPPORTED
static bool InitAdlpMediaSku(struct GfxDeviceInfo *devInfo,
    MediaFeatureTable *                            skuTable,
    struct LinuxDriverInfo                        *drvInfo,
    MediaUserSettingSharedPtr                      userSettingPtr)
{
    if (!InitTglMediaSku(devInfo, skuTable, drvInfo, userSettingPtr))
    {
        return false;
    }

    if (devInfo->eGTType == GTTYPE_GT0_5)
    {
        MEDIA_WR_SKU(skuTable, FtrGT0_5, 1);
    }

    MEDIA_WR_SKU(skuTable, FtrAV1VLDLSTDecoding, 1);
    MEDIA_WR_SKU(skuTable, FtrGucSubmission, 1);
    MEDIA_WR_SKU(skuTable, FtrDecodeHEVC422VTScalaDisable, 1);
    MEDIA_WR_SKU(skuTable, FtrVirtualTileScalabilityDisable, 1);

    return true;
}

static bool InitAdlpMediaWa(struct GfxDeviceInfo *devInfo,
    MediaWaTable *                                waTable,
    struct LinuxDriverInfo *                      drvInfo)
{
    if (!InitTglMediaWa(devInfo, waTable, drvInfo))
    {
        return false;
    }

    //ADL-P not need this
    MEDIA_WR_WA(waTable, Wa_1409820462, 0);
    MEDIA_WR_WA(waTable, WaDisableClearCCS, 1);

    return true;
}

static struct LinuxDeviceInit adlpDeviceInit =
    {
        .productFamily    = IGFX_ALDERLAKE_P,
        .InitMediaFeature = InitAdlpMediaSku,
        .InitMediaWa      = InitAdlpMediaWa,
};

static bool adlpDeviceRegister = DeviceInfoFactory<LinuxDeviceInit>::
    RegisterDevice(IGFX_ALDERLAKE_P, &adlpDeviceInit);

#endif

#ifdef IGFX_GEN12_ADLN_SUPPORTED
static bool InitAdlnMediaSku(struct GfxDeviceInfo *devInfo,
    MediaFeatureTable *                            skuTable,
    struct LinuxDriverInfo                        *drvInfo,
    MediaUserSettingSharedPtr                      userSettingPtr)
{
    if (!InitTglMediaSku(devInfo, skuTable, drvInfo, userSettingPtr))
    {
        return false;
    }

    if (devInfo->eGTType == GTTYPE_GT0_5)
    {
        MEDIA_WR_SKU(skuTable, FtrGT0_5, 1);
    }
    
    MEDIA_WR_SKU(skuTable, FtrHeight8AlignVE3DLUTDualPipe, 1);
    MEDIA_WR_SKU(skuTable, FtrAV1VLDLSTDecoding, 1);
    MEDIA_WR_SKU(skuTable, FtrGucSubmission, 1);
    MEDIA_WR_SKU(skuTable, FtrDecodeHEVC422VTScalaDisable, 1);
    MEDIA_WR_SKU(skuTable, FtrE2ECompression, 1);

    return true;
}

static bool InitAdlnMediaWa(struct GfxDeviceInfo *devInfo,
    MediaWaTable *                                waTable,
    struct LinuxDriverInfo *                      drvInfo)
{
    if (!InitTglMediaWa(devInfo, waTable, drvInfo))
    {
        return false;
    }

    //ADL-N keeps below setting same as ADL-S/ADL-P
    MEDIA_WR_WA(waTable, Wa_1409820462, 0);

    //ADL-N need enable MMC for all stepping
    MEDIA_WR_WA(waTable, WaDisableCodecMmc, 0);
    MEDIA_WR_WA(waTable, WaDisableVPMmc, 0);
    MEDIA_WR_WA(waTable, WaDisableClearCCS, 1);

    return true;
}

static struct LinuxDeviceInit adlnDeviceInit =
    {
        .productFamily    = IGFX_ALDERLAKE_N,
        .InitMediaFeature = InitAdlnMediaSku,
        .InitMediaWa      = InitAdlnMediaWa,
};

static bool adlnDeviceRegister = DeviceInfoFactory<LinuxDeviceInit>::
    RegisterDevice(IGFX_ALDERLAKE_N, &adlnDeviceInit);

#endif

static struct LinuxDeviceInit tgllpDeviceInit =
{
    .productFamily    = IGFX_TIGERLAKE_LP,
    .InitMediaFeature = InitTglMediaSku,
    .InitMediaWa      = InitTglMediaWa,
};

static bool tgllpDeviceRegister = DeviceInfoFactory<LinuxDeviceInit>::
    RegisterDevice(IGFX_TIGERLAKE_LP, &tgllpDeviceInit);


