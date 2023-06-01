/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     media_sku_wa_g8.cpp
//!

#include "igfxfmid.h"
#include "linux_system_info.h"
#include "skuwa_factory.h"
#include "linux_skuwa_debug.h"
#include "linux_media_skuwa.h"

//extern template class DeviceInfoFactory<GfxDeviceInfo>;
typedef DeviceInfoFactory<LinuxDeviceInit> DeviceInit;

static struct LinuxCodecInfo bdwCodecInfo =
{
    .avcDecoding        = 1,
    .mpeg2Decoding      = 1,
    .vp8Decoding        = 1,
    .vc1Decoding        = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .jpegDecoding       = 1,
    .avcEncoding        = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
    .mpeg2Encoding      = SET_STATUS_BY_FULL_OPEN_SOURCE(1, 0),
};

static bool InitBdwMediaSku(struct GfxDeviceInfo *devInfo,
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
        LinuxCodecInfo *codecInfo = &bdwCodecInfo;

        MEDIA_WR_SKU(skuTable, FtrAVCVLDLongDecoding, codecInfo->avcDecoding);
        MEDIA_WR_SKU(skuTable, FtrMPEG2VLDDecoding, codecInfo->mpeg2Decoding);
        MEDIA_WR_SKU(skuTable, FtrIntelVP8VLDDecoding, codecInfo->vp8Decoding);
        MEDIA_WR_SKU(skuTable, FtrVC1VLDDecoding, codecInfo->vc1Decoding);
        MEDIA_WR_SKU(skuTable, FtrIntelJPEGDecoding, codecInfo->jpegDecoding);
        MEDIA_WR_SKU(skuTable, FtrEncodeAVC, codecInfo->avcEncoding);
        MEDIA_WR_SKU(skuTable, FtrEncodeMPEG2, codecInfo->mpeg2Encoding);
    }

    if (devInfo->eGTType == GTTYPE_GT1)
    {
        MEDIA_WR_SKU(skuTable, FtrGT1, 1);
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
    MEDIA_WR_SKU(skuTable, FtrVcs2,   drvInfo->hasBsd2);
    MEDIA_WR_SKU(skuTable, FtrVERing, drvInfo->hasVebox);
    MEDIA_WR_SKU(skuTable, FtrPPGTT, drvInfo->hasPpgtt);
    MEDIA_WR_SKU(skuTable, FtrEDram, devInfo->hasERAM);
    if ((devInfo->eGTType == GTTYPE_GT3) ||
        (devInfo->eGTType == GTTYPE_GT4))
    {
        MEDIA_WR_SKU(skuTable, FtrSliceShutdown, 1);
    }
    else
    {
        MEDIA_WR_SKU(skuTable, FtrSingleVeboxSlice, 1);
    }

    MEDIA_WR_SKU(skuTable, FtrSliceShutdownOverride, 1);
    MEDIA_WR_SKU(skuTable, FtrTileY, 1);

    MEDIA_WR_SKU(skuTable, FtrUseSwSwizzling, 1);

    return true;
}

static bool InitBdwMediaWa(struct GfxDeviceInfo *devInfo,
                             MediaWaTable *waTable,
                             struct LinuxDriverInfo *drvInfo)
{
    if ((devInfo == nullptr) || (waTable == nullptr) || (drvInfo == nullptr))
    {
        DEVINFO_ERROR("null ptr is passed\n");
        return false;
    }

    MEDIA_WR_WA(waTable, WaForceGlobalGTT, !drvInfo->hasPpgtt);
    MEDIA_WR_WA(waTable, WaAddMediaStateFlushCmd, 1);
    MEDIA_WR_WA(waTable, WaDisableLockForTranscodePerf, 1);
    MEDIA_WR_WA(waTable, WaSendDummyVFEafterPipelineSelect, 1);
    MEDIA_WR_WA(waTable, WaVC1UnequalFieldHeights, 1);
    MEDIA_WR_WA(waTable, WaJPEGHeightAlignYUV422H2YToNV12, 1);
    MEDIA_WR_WA(waTable, WaEnableDscale, 1);
    MEDIA_WR_WA(waTable, Wa16KInputHeightNV12Planar420, 1);
    MEDIA_WR_WA(waTable, WaDisableCodecMmc, 1);
    MEDIA_WR_WA(waTable, WaDisableSetObjectCapture, 0);
    MEDIA_WR_WA(waTable, WaDisableGmmLibOffsetInDeriveImage, 1);
    return true;
}

static struct LinuxDeviceInit bdwDeviceInit =
{
    .productFamily = IGFX_BROADWELL,
    .InitMediaFeature = InitBdwMediaSku,
    .InitMediaWa      = InitBdwMediaWa,
};

static bool bdwDeviceRegister = DeviceInfoFactory<LinuxDeviceInit>::
    RegisterDevice(IGFX_BROADWELL, &bdwDeviceInit);
