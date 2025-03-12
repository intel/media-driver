/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     media_sku_wa_ptl.cpp
//!

#include "igfxfmid.h"
#include "linux_system_info.h"
#include "skuwa_factory.h"
#include "linux_skuwa_debug.h"
#include "linux_media_skuwa.h"
#include "media_user_setting_specific.h"

static constexpr uint32_t singleVeboxSubSliceNumMax = 24;

//extern template class DeviceInfoFactory<GfxDeviceInfo>;
typedef DeviceInfoFactory<LinuxDeviceInit> DeviceInit;

static struct LinuxCodecInfo ptlCodecInfo =
{
    .avcDecoding        = 1,
    .mpeg2Decoding      = 1,
    .vp8Decoding        = 1,
    .vc1Decoding        = 0,
    .jpegDecoding       = 1,
    .avcEncoding        = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .mpeg2Encoding      = 0,
    .hevcDecoding       = 1,
    .hevcEncoding       = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .jpegEncoding       = 1,
    .avcVdenc           = 1,
    .vp9Decoding        = 1,
    .hevc10Decoding     = 1,
    .vp9b10Decoding     = 1,
    .hevc10Encoding     = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .hevc12Encoding     = 0,
    .vp8Encoding        = 0,
    .hevcVdenc          = 1,
    .vp9Vdenc           = 1,
    .adv0Decoding       = 1,
    .adv1Decoding       = 1,
};

static bool InitPtlMediaSkuExt(struct GfxDeviceInfo *devInfo,
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
        LinuxCodecInfo *codecInfo = &ptlCodecInfo;

        MEDIA_WR_SKU(skuTable, FtrAVCVLDLongDecoding, codecInfo->avcDecoding);
        MEDIA_WR_SKU(skuTable, FtrIntelAVCVLDHigh422Decoding, codecInfo->avcDecoding);
        MEDIA_WR_SKU(skuTable, FtrMPEG2VLDDecoding, codecInfo->mpeg2Decoding);
        MEDIA_WR_SKU(skuTable, FtrIntelVP8VLDDecoding, codecInfo->vp8Decoding);
        MEDIA_WR_SKU(skuTable, FtrVC1VLDDecoding, codecInfo->vc1Decoding);
        MEDIA_WR_SKU(skuTable, FtrIntelJPEGDecoding, codecInfo->jpegDecoding);
        MEDIA_WR_SKU(skuTable, FtrEncodeAVC, codecInfo->avcEncoding);
        MEDIA_WR_SKU(skuTable, FtrEncodeMPEG2, codecInfo->mpeg2Encoding);
        MEDIA_WR_SKU(skuTable, FtrIntelHEVCVLDMainDecoding, codecInfo->hevcDecoding);
        MEDIA_WR_SKU(skuTable, FtrIntelHEVCVLDMain10Decoding, codecInfo->hevc10Decoding);
        MEDIA_WR_SKU(skuTable, FtrIntelVVCVLDDecodingMain10, 1);
        MEDIA_WR_SKU(skuTable, FtrIntelVVCVLDDecodingMultilayerMain10, 0);

        MEDIA_WR_SKU(skuTable, FtrEncodeHEVC, codecInfo->hevcEncoding);
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVC10bit, codecInfo->hevc10Encoding);
        MEDIA_WR_SKU(skuTable, FtrEncodeJPEG, codecInfo->jpegEncoding);
        MEDIA_WR_SKU(skuTable, FtrEncodeAVCVdenc, codecInfo->avcVdenc);
        MEDIA_WR_SKU(skuTable, FtrVP9VLDDecoding, codecInfo->vp9Decoding);
        MEDIA_WR_SKU(skuTable, FtrIntelVP9VLDProfile0Decoding8bit420, codecInfo->vp9Decoding);
        MEDIA_WR_SKU(skuTable, FtrVP9VLD10bProfile2Decoding, codecInfo->vp9b10Decoding);
        MEDIA_WR_SKU(skuTable, FtrIntelVP9VLDProfile2Decoding, codecInfo->vp9b10Decoding);
        MEDIA_WR_SKU(skuTable, FtrIntelAV1VLDDecoding8bit420, codecInfo->adv0Decoding);
        MEDIA_WR_SKU(skuTable, FtrAV1VLDProfile1Decoding8bit444, codecInfo->adv0Decoding);
        MEDIA_WR_SKU(skuTable, FtrIntelAV1VLDDecoding10bit420, codecInfo->adv1Decoding);
        MEDIA_WR_SKU(skuTable, FtrAV1VLDProfile1Decoding10bit444, codecInfo->adv1Decoding);

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

        /* AV1 VDENC 8/10Bit 420 */
        MEDIA_WR_SKU(skuTable, FtrEncodeAV1Vdenc, 1);
        MEDIA_WR_SKU(skuTable, FtrEncodeAV1Vdenc10bit420, 1);
    }

    MEDIA_WR_SKU(skuTable, FtrEnableProtectedHuc, drvInfo->hasProtectedHuc);
    MEDIA_WR_SKU(skuTable, FtrEnableMediaKernels, drvInfo->hasHuc);
    MEDIA_WR_SKU(skuTable, FtrVERing, drvInfo->hasVebox);
    MEDIA_WR_SKU(skuTable, FtrPPGTT, drvInfo->hasPpgtt);
    MEDIA_WR_SKU(skuTable, FtrEDram, devInfo->hasERAM);

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
    MEDIA_WR_SKU(skuTable, FtrSfcScalability, 1);

    MEDIA_WR_SKU(skuTable, FtrSWMediaReset, 1);
    MEDIA_WR_SKU(skuTable, FtrGucSubmission, 1);

    MEDIA_WR_SKU(skuTable, FtrTileY, 0);
    MEDIA_WR_SKU(skuTable, FtrLinearCCS, 1);
    MEDIA_WR_SKU(skuTable, FtrFlatPhysCCS, 1);

    MEDIA_WR_SKU(skuTable, FtrWithSlimVdbox, 0);

    MEDIA_WR_SKU(skuTable, FtrE2ECompression, 1);
    MEDIA_WR_SKU(skuTable, FtrXe2Compression , 1);
    MEDIA_WR_SKU(skuTable, FtrHDR, 1);
    MEDIA_WR_SKU(skuTable, FtrDisableRenderTargetWidthAdjust, 1);

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
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
        MEDIA_WR_SKU(skuTable, FtrXe2Compression , 0);
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

    //Disable LocalMemory for all iGraphics
    MEDIA_WR_SKU(skuTable, FtrLocalMemory, 0);

    MEDIA_WR_SKU(skuTable, FtrConditionalBatchBuffEnd, 1);
    MEDIA_WR_SKU(skuTable, FtrUseSwSwizzling, 0);
    MEDIA_WR_SKU(skuTable, FtrMemoryRemapSupport, 1);

    MEDIA_WR_SKU(skuTable, FtrAV1VLDLSTDecoding, 1);
    MEDIA_WR_SKU(skuTable, FtrMediaIPSeparation , 1);

    return true;
}

static bool InitPtlMediaWaExt(struct GfxDeviceInfo *devInfo,
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

    MEDIA_WR_WA(waTable, WaSFC270DegreeRotation, 0);

    MEDIA_WR_WA(waTable, WaEnableYV12BugFixInHalfSliceChicken7, 1);

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_AUX_TABLE_16K_GRANULAR_ID,
        &userFeatureData,
        (MOS_CONTEXT_HANDLE)nullptr);

    MEDIA_WR_WA(waTable, Wa16KInputHeightNV12Planar420, 1);

    /*software wa to disable calculate the UV offset by gmmlib
      CPU blt call will add/remove padding on the platform*/
    MEDIA_WR_WA(waTable, WaDisableGmmLibOffsetInDeriveImage, 1);

    /* Turn off MMC for codec, need to remove once turn it on */
    MEDIA_WR_WA(waTable, WaDisableCodecMmc, 0);

    /* Turn off MMC for VPP, need to remove once turn it on */
    MEDIA_WR_WA(waTable, WaDisableVPMmc, 0);

    MEDIA_WR_WA(waTable, WaDisableSetObjectCapture, 1);

    MEDIA_WR_WA(waTable, Wa_Vp9UnalignedHeight, 0);

    MEDIA_WR_WA(waTable, Wa_15013355402, 1);

    MEDIA_WR_WA(waTable, Wa_15014143531, 1);

    MEDIA_WR_WA(waTable, Wa_16021867713, 1);

    return true;
}


static struct LinuxDeviceInit ptlDeviceInit =
{
    .productFamily    = IGFX_PTL,
    .InitMediaFeature = InitPtlMediaSkuExt,
    .InitMediaWa      = InitPtlMediaWaExt,
};

static bool ptlDeviceRegister = DeviceInfoFactory<LinuxDeviceInit>::
    RegisterDevice(IGFX_PTL, &ptlDeviceInit);
