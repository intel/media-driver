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
//! \file     media_libva_caps_g8.cpp
//! \brief    This file implements the C++ class/interface for gen8 media capbilities. 
//!

#include "codec_def_encode_avc.h"
#include "media_libva_util.h"
#include "media_libva.h"
#include "media_libva_caps_g8.h"
#include "media_libva_caps_factory.h"

const VAImageFormat m_supportedImageformatsG8[] =
{
    // "VA_LSB_FIRST" is to identify how following bit masks mapped to address instead of char order in VA_FOURCC_RGBA naming.
    {VA_FOURCC_BGRA,           VA_LSB_FIRST,   32, 32, 0x00ff0000, 0x0000ff00, 0x000000ff,  0xff000000}, /* [31:0] A:R:G:B 8:8:8:8 little endian */
    {VA_FOURCC_RGBA,           VA_LSB_FIRST,   32, 32, 0x000000ff, 0x0000ff00, 0x00ff0000,  0xff000000}, /* [31:0] A:B:G:R 8:8:8:8 little endian */
    {VA_FOURCC_BGRX,           VA_LSB_FIRST,   32, 24, 0x00ff0000, 0x0000ff00, 0x000000ff,  0},          /* [31:0] X:R:G:B 8:8:8:8 little endian */
    {VA_FOURCC_RGBX,           VA_LSB_FIRST,   32, 24, 0x000000ff, 0x0000ff00, 0x00ff0000,  0},          /* [31:0] X:B:G:R 8:8:8:8 little endian */
    {VA_FOURCC_RGB565, VA_LSB_FIRST,   16, 16, 0xf800,     0x07e0,     0x001f,      0},
    {VA_FOURCC_NV12,   VA_LSB_FIRST,   12, 0,0,0,0,0},
    {VA_FOURCC_NV21,   VA_LSB_FIRST,   12, 0,0,0,0,0},
    {VA_FOURCC_YUY2,   VA_LSB_FIRST,   16, 0,0,0,0,0},
    {VA_FOURCC_UYVY,   VA_LSB_FIRST,   16, 0,0,0,0,0},
    {VA_FOURCC_YV12,   VA_LSB_FIRST,   12, 0,0,0,0,0},
    {VA_FOURCC_I420,   VA_LSB_FIRST,   12, 0,0,0,0,0},
    {VA_FOURCC_422H,   VA_LSB_FIRST,   16, 0,0,0,0,0},
    {VA_FOURCC_422V,   VA_LSB_FIRST,   16, 0,0,0,0,0},
    {VA_FOURCC_444P,   VA_LSB_FIRST,   24, 0,0,0,0,0},
    {VA_FOURCC_IMC3,   VA_LSB_FIRST,   16, 0,0,0,0,0}
};

VAStatus MediaLibvaCapsG8::QueryImageFormats(VAImageFormat *formatList, int32_t *numFormats)
{
    DDI_CHK_NULL(formatList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(numFormats, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    int32_t num = 0;
    uint32_t maxNum = GetImageFormatsMaxNum();

    memset(formatList, 0,  sizeof(m_supportedImageformatsG8));
    for (uint32_t idx = 0; idx < maxNum; idx++)
    {
        formatList[num].fourcc           = m_supportedImageformatsG8[idx].fourcc;
        formatList[num].byte_order       = m_supportedImageformatsG8[idx].byte_order;
        formatList[num].bits_per_pixel   = m_supportedImageformatsG8[idx].bits_per_pixel;
        formatList[num].depth            = m_supportedImageformatsG8[idx].depth;
        formatList[num].red_mask         = m_supportedImageformatsG8[idx].red_mask;
        formatList[num].green_mask       = m_supportedImageformatsG8[idx].green_mask;
        formatList[num].blue_mask        = m_supportedImageformatsG8[idx].blue_mask;
        formatList[num].alpha_mask       = m_supportedImageformatsG8[idx].alpha_mask;
        num++;
    }
    *numFormats = num;

    return VA_STATUS_SUCCESS;
}

uint32_t MediaLibvaCapsG8::GetImageFormatsMaxNum()
{
    return sizeof(m_supportedImageformatsG8)/sizeof(m_supportedImageformatsG8[0]);
}

bool MediaLibvaCapsG8::IsImageSupported(uint32_t fourcc)
{
    uint32_t maxNum = GetImageFormatsMaxNum();
    for (int32_t idx = 0; idx < maxNum; idx++)
    {
        if (m_supportedImageformatsG8[idx].fourcc == fourcc)
        {
            return true;
        }
    }

    return false;
}

VAStatus MediaLibvaCapsG8::PopulateColorMaskInfo(VAImageFormat *vaImgFmt)
{
    uint32_t maxNum = GetImageFormatsMaxNum();

    DDI_CHK_NULL(vaImgFmt, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    for (int32_t idx = 0; idx < maxNum; idx++)
    {
        if (m_supportedImageformatsG8[idx].fourcc == vaImgFmt->fourcc)
        {
            vaImgFmt->red_mask   = m_supportedImageformatsG8[idx].red_mask;
            vaImgFmt->green_mask = m_supportedImageformatsG8[idx].green_mask;
            vaImgFmt->blue_mask  = m_supportedImageformatsG8[idx].blue_mask;
            vaImgFmt->alpha_mask = m_supportedImageformatsG8[idx].alpha_mask;

            return VA_STATUS_SUCCESS;
        }
    }

    return VA_STATUS_ERROR_INVALID_IMAGE_FORMAT;
}

VAStatus MediaLibvaCapsG8::GetPlatformSpecificAttrib(
        VAProfile profile,
        VAEntrypoint entrypoint,
        VAConfigAttribType type,
        uint32_t *value)
{
    DDI_CHK_NULL(value, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    VAStatus status = VA_STATUS_SUCCESS;
    *value = VA_ATTRIB_NOT_SUPPORTED;
    switch ((int32_t)type)
    {
        case VAConfigAttribEncMaxRefFrames:
        {
            if (entrypoint == VAEntrypointEncSliceLP || !IsHevcProfile(profile))
            {
                status = VA_STATUS_ERROR_INVALID_PARAMETER;
            }
            else
            {
                *value = 1 | (1 << 16);
            }
            break;
        }
        case VAConfigAttribDecProcessing:
        {
            *value = VA_DEC_PROCESSING_NONE;
            break;
        }
        case VAConfigAttribEncIntraRefresh:
        {
            if(IsAvcProfile(profile))
            {
                *value = VA_ENC_INTRA_REFRESH_ROLLING_COLUMN |
                    VA_ENC_INTRA_REFRESH_ROLLING_ROW;
            }
            else
            {
                *value = VA_ENC_INTRA_REFRESH_NONE;
            }
            break;
        }
        case VAConfigAttribEncROI:
        {
            VAConfigAttribValEncROI roi_attr = { .value = 0 };

            if (entrypoint == VAEntrypointEncSliceLP)
            {
                status = VA_STATUS_ERROR_INVALID_PARAMETER;
            }
            else if (IsAvcProfile(profile))
            {
                roi_attr.bits.num_roi_regions = ENCODE_DP_AVC_MAX_ROI_NUMBER;
            }

            *value = roi_attr.value;
            break;
        }
        case VAConfigAttribCustomRoundingControl:
        {
            if (IsAvcProfile(profile))
            {
                *value = 1;
            }
            else
            {
                *value = 0;
            }
            break;
        }
        case VAConfigAttribEncMaxSlices:
        {
            *value = ENCODE_AVC_MAX_SLICES_SUPPORTED;
            break;
        }
        case VAConfigAttribMaxPictureWidth:
        {
            if(profile == VAProfileJPEGBaseline)
            {
                *value = ENCODE_JPEG_MAX_PIC_WIDTH;
            }
            else if(IsAvcProfile(profile))
            {
                *value = CODEC_4K_MAX_PIC_WIDTH;
            }
            else
            {
                *value = CODEC_MAX_PIC_WIDTH;
            }
            break;
        }
        case VAConfigAttribMaxPictureHeight:
        {
            if(profile == VAProfileJPEGBaseline)
            {
                *value = ENCODE_JPEG_MAX_PIC_HEIGHT;
            }
            else if(IsAvcProfile(profile))
            {
                *value = CODEC_4K_MAX_PIC_HEIGHT;
            }
            else
            {
                *value = CODEC_MAX_PIC_HEIGHT;
            }
            break;
        }
        default:
            status = VA_STATUS_ERROR_INVALID_PARAMETER;
            break;
    }
    return status;
}

VAStatus MediaLibvaCapsG8::LoadProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;
    status = LoadAvcDecProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
    status = LoadAvcEncProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
    status = LoadMpeg2DecProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
    status = LoadMpeg2EncProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
    status = LoadVc1DecProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
    status = LoadJpegDecProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
    status = LoadJpegEncProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
    status = LoadVp8DecProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
    status = LoadVp8EncProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
    status = LoadVp9DecProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
    status = LoadVp9EncProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
#if !defined(_FULL_OPEN_SOURCE) && defined(ENABLE_KERNELS)
    status = LoadNoneProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
#endif
    return status;
}

VAStatus MediaLibvaCapsG8::QueryAVCROIMaxNum(uint32_t rcMode, bool isVdenc, uint32_t *maxNum, bool *isRoiInDeltaQP)
{
    DDI_CHK_NULL(maxNum, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(isRoiInDeltaQP, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_CONDITION(isVdenc == true, "VDEnc is not supported in Gen8", VA_STATUS_ERROR_INVALID_PARAMETER);

    *maxNum = ENCODE_DP_AVC_MAX_ROI_NUMBER;
    *isRoiInDeltaQP =  false;
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCapsG8::GetMbProcessingRateEnc(
        MEDIA_FEATURE_TABLE *skuTable,
        uint32_t tuIdx,
        uint32_t codecMode,
        bool vdencActive,
        uint32_t *mbProcessingRatePerSec)
{
    DDI_CHK_NULL(skuTable, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(mbProcessingRatePerSec, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t gtIdx = 0;

    if (MEDIA_IS_SKU(skuTable, FtrGT1))
    {
        gtIdx = 3;
    }
    else if (MEDIA_IS_SKU(skuTable, FtrGT1_5))
    {
        gtIdx = 2;
    }
    else if (MEDIA_IS_SKU(skuTable, FtrGT2))
    {
        gtIdx = 1;
    }
    else if (MEDIA_IS_SKU(skuTable, FtrGT3))
    {
        gtIdx = 0;
    }
    else
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (MEDIA_IS_SKU(skuTable, FtrULX))
    {
        const uint32_t mbRate[7][4] =
        {
            // GT3 |  GT2   | GT1.5  |  GT1
            { 0, 750000, 750000, 676280 },
            { 0, 750000, 750000, 661800 },
            { 0, 750000, 750000, 640000 },
            { 0, 750000, 750000, 640000 },
            { 0, 750000, 750000, 640000 },
            { 0, 416051, 416051, 317980 },
            { 0, 214438, 214438, 180655 }
        };

        if (gtIdx == 0)
        {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        *mbProcessingRatePerSec = mbRate[tuIdx][gtIdx];
    }
    else if (MEDIA_IS_SKU(skuTable, FtrULT))
    {
        const uint32_t mbRate[7][4] =
        {
            // GT3   |  GT2   | GT1.5  |  GT1
            { 1544090, 1544090, 1029393, 676280 },
            { 1462540, 1462540, 975027, 661800 },
            { 1165381, 1165381, 776921, 640000 },
            { 1165381, 1165381, 776921, 640000 },
            { 1165381, 1165381, 776921, 640000 },
            { 624076, 624076, 416051, 317980 },
            { 321657, 321657, 214438, 180655 }
        };

        *mbProcessingRatePerSec = mbRate[tuIdx][gtIdx];
    }
    else
    {
        const uint32_t mbRate[7][4] =
        {
            // GT3   |   GT2  | GT1.5  |  GT1
            { 1544090, 1544090, 1029393, 676280 },
            { 1462540, 1462540, 975027, 661800 },
            { 1165381, 1165381, 776921, 640000 },
            { 1165381, 1165381, 776921, 640000 },
            { 1165381, 1165381, 776921, 640000 },
            { 624076, 624076, 416051, 317980 },
            { 321657, 321657, 214438, 180655 }
        };

        *mbProcessingRatePerSec = mbRate[tuIdx][gtIdx];
    }

    return VA_STATUS_SUCCESS;
}

extern template class MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>;

static bool bdwRegistered = MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>::
    RegisterCaps<MediaLibvaCapsG8>((uint32_t)IGFX_BROADWELL);
