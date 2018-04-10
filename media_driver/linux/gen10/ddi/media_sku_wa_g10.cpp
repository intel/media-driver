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
//! \file     media_sku_wa_g10.cpp
//!

#include "igfxfmid.h"
#include "linux_system_info.h"
#include "skuwa_factory.h"
#include "linux_skuwa_debug.h"
#include "linux_media_skuwa.h"

#define GEN10_VEBOX2_SUBSLICES 6

//extern template class DeviceInfoFactory<GfxDeviceInfo>;
typedef DeviceInfoFactory<LinuxDeviceInit> DeviceInit;

static struct LinuxCodecInfo cnlCodecInfo =
{
    .avcDecoding        = 1,
    .mpeg2Decoding      = 1,
    .vp8Decoding        = 1,
    .vc1Decoding        = 1,
    .jpegDecoding       = 1,
    .avcEncoding        = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .mpeg2Encoding      = 1,
    .hevcDecoding       = 1,
    .hevcEncoding       = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .jpegEncoding       = 1,
    .avcVdenc           = 1,
    .vp9Decoding        = 1,
    .hevc10Decoding     = 1,
    .vp9b10Decoding     = 1,
    .hevc10Encoding     = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .vp8Encoding        = 1,
    .hevcVdenc          = 1,
    .vp9Vdenc           = 1,
};

static bool InitCnlMediaSku(struct GfxDeviceInfo *devInfo,
                             MediaFeatureTable *skuTable,
                             struct LinuxDriverInfo *drvInfo)
{
    if ((devInfo == nullptr) || (skuTable == nullptr) || (drvInfo == nullptr))
    {
        DEVINFO_ERROR("null ptr is passed\n");
        return false;
    }

    if (drvInfo->hasBsd)
    {
        LinuxCodecInfo *codecInfo = &cnlCodecInfo;

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

        /* VP8 enc */
        MEDIA_WR_SKU(skuTable, FtrEncodeVP8, codecInfo->vp8Encoding);

        /* HEVC VDENC 8/10 420 Enc */
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVCVdencMain, codecInfo->hevcVdenc);
        MEDIA_WR_SKU(skuTable, FtrEncodeHEVCVdencMain10, codecInfo->hevcVdenc);

        /* Vp9 VDENC 8/10 420 Enc*/
        MEDIA_WR_SKU(skuTable, FtrEncodeVP9Vdenc, codecInfo->vp9Vdenc);
        MEDIA_WR_SKU(skuTable, FtrEncodeVP9Vdenc10bit420, codecInfo->vp9Vdenc);
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
    else
    {
        /* GT1 is by default */
        MEDIA_WR_SKU(skuTable, FtrGT1, 1);
    }
    MEDIA_WR_SKU(skuTable, FtrVERing, drvInfo->hasVebox);
    MEDIA_WR_SKU(skuTable, FtrPPGTT, drvInfo->hasPpgtt);
    MEDIA_WR_SKU(skuTable, FtrEDram, devInfo->hasERAM);
    MEDIA_WR_SKU(skuTable, FtrVcs2,  drvInfo->hasBsd2);

    MEDIA_WR_SKU(skuTable, FtrSliceShutdown, 0);
    MEDIA_WR_SKU(skuTable, FtrSingleVeboxSlice, 1);
    if (devInfo->SubSliceCount >= GEN10_VEBOX2_SUBSLICES)
    {
        MEDIA_WR_SKU(skuTable, FtrSingleVeboxSlice, 0);
    }

    MEDIA_WR_SKU(skuTable, FtrSFCPipe, 1);
    MEDIA_WR_SKU(skuTable, FtrSSEUPowerGating, 1);

    /* MMC is disabled by default. It can be enabled based on HW validation */
    MEDIA_WR_SKU(skuTable, FtrMemoryCompression, 0);
    MEDIA_WR_SKU(skuTable, FtrHcpDecMemoryCompression, 0);
    MEDIA_WR_SKU(skuTable, Ftr10bitDecMemoryCompression, 0);

    MEDIA_WR_SKU(skuTable, FtrVpP010Output, 1);
    MEDIA_WR_SKU(skuTable, FtrPerCtxtPreemptionGranularityControl, 1);

    return true;
}

static bool InitCnlMediaWa(struct GfxDeviceInfo *devInfo,
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

    MEDIA_WR_WA(waTable, Wa8BitFrameIn10BitHevc, 1);

    MEDIA_WR_WA(waTable, WaDisableSFCSrcCrop, 1);

    MEDIA_WR_WA(waTable, WaSFC270DegreeRotation, 1);

    MEDIA_WR_WA(waTable, WaEnableYV12BugFixInHalfSliceChicken7, 1);

    MEDIA_WR_WA(waTable, WaDisablePreemptForMediaWalkerWithGroups, 1);
    return true;
}

static struct LinuxDeviceInit cnlDeviceInit =
{
    .productFamily    = IGFX_CANNONLAKE,
    .InitMediaFeature = InitCnlMediaSku,
    .InitMediaWa      = InitCnlMediaWa,
};

static bool cnlDeviceRegister = DeviceInfoFactory<LinuxDeviceInit>::
    RegisterDevice(IGFX_CANNONLAKE, &cnlDeviceInit);
