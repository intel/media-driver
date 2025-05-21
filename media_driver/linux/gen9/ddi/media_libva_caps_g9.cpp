/*
* Copyright (c) 2017-2021, Intel Corporation
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
//! \file     media_libva_caps_g9.cpp
//! \brief    This file implements the C++ class/interface for gen9 media capbilities. 
//!

#include "codec_def_encode_hevc.h"
#include "media_libva_util.h"
#include "media_libva.h"
#include "media_libva_caps_cp_interface.h"
#include "media_libva_caps_g9.h"
#include "media_libva_caps_factory.h"

const VAImageFormat m_supportedImageformatsG9[] =
{
    // "VA_LSB_FIRST" is to identify how following bit masks mapped to address instead of char order in VA_FOURCC_RGBA naming.
    {VA_FOURCC_BGRA,           VA_LSB_FIRST,   32, 32, 0x00ff0000, 0x0000ff00, 0x000000ff,  0xff000000}, /* [31:0] A:R:G:B 8:8:8:8 little endian */
    {VA_FOURCC_RGBA,           VA_LSB_FIRST,   32, 32, 0x000000ff, 0x0000ff00, 0x00ff0000,  0xff000000}, /* [31:0] A:B:G:R 8:8:8:8 little endian */
    {VA_FOURCC_BGRX,           VA_LSB_FIRST,   32, 24, 0x00ff0000, 0x0000ff00, 0x000000ff,  0},          /* [31:0] X:R:G:B 8:8:8:8 little endian */
    {VA_FOURCC_RGBX,           VA_LSB_FIRST,   32, 24, 0x000000ff, 0x0000ff00, 0x00ff0000,  0},          /* [31:0] X:B:G:R 8:8:8:8 little endian */
    {VA_FOURCC_A2R10G10B10,    VA_LSB_FIRST,   32, 30, 0x3ff00000, 0x000ffc00, 0x000003ff,  0x30000000}, /* [31:0] A:R:G:B 2:10:10:10 little endian */
    {VA_FOURCC_A2B10G10R10,    VA_LSB_FIRST,   32, 30, 0x000003ff, 0x000ffc00, 0x3ff00000,  0x30000000}, /* [31:0] A:B:G:R 2:10:10:10 little endian */
    {VA_FOURCC_X2R10G10B10,    VA_LSB_FIRST,   32, 30, 0x3ff00000, 0x000ffc00, 0x000003ff,  0},          /* [31:0] X:R:G:B 2:10:10:10 little endian */
    {VA_FOURCC_X2B10G10R10,    VA_LSB_FIRST,   32, 30, 0x000003ff, 0x000ffc00, 0x3ff00000,  0},          /* [31:0] X:B:G:R 2:10:10:10 little endian */
    {VA_FOURCC_RGB565,         VA_LSB_FIRST,   16, 16, 0xf800,     0x07e0,     0x001f,      0},          /* [15:0] R:G:B 5:6:5 little endian */
    {VA_FOURCC_RGBP,           VA_LSB_FIRST,   24, 24,0,0,0,0},
    {VA_FOURCC_BGRP,           VA_LSB_FIRST,   24, 24,0,0,0,0},
    {VA_FOURCC_AYUV,           VA_LSB_FIRST,   32, 0,0,0,0,0},
    {VA_FOURCC_Y800,           VA_LSB_FIRST,   8,  0,0,0,0,0},
    {VA_FOURCC_NV12,           VA_LSB_FIRST,   12, 0,0,0,0,0},
    {VA_FOURCC_NV21,           VA_LSB_FIRST,   12, 0,0,0,0,0},
    {VA_FOURCC_YUY2,           VA_LSB_FIRST,   16, 0,0,0,0,0},
    {VA_FOURCC_UYVY,           VA_LSB_FIRST,   16, 0,0,0,0,0},
    {VA_FOURCC_YV12,           VA_LSB_FIRST,   12, 0,0,0,0,0},
    {VA_FOURCC_I420,           VA_LSB_FIRST,   12, 0,0,0,0,0},
    {VA_FOURCC_411P,           VA_LSB_FIRST,   12, 0,0,0,0,0},
    {VA_FOURCC_422H,           VA_LSB_FIRST,   16, 0,0,0,0,0},
    {VA_FOURCC_422V,           VA_LSB_FIRST,   16, 0,0,0,0,0},
    {VA_FOURCC_444P,           VA_LSB_FIRST,   24, 0,0,0,0,0},
    {VA_FOURCC_IMC3,           VA_LSB_FIRST,   16, 0,0,0,0,0},
    {VA_FOURCC_P010,           VA_LSB_FIRST,   24, 0,0,0,0,0}
};

VAStatus MediaLibvaCapsG9::QueryImageFormats(VAImageFormat *formatList, int32_t *numFormats)
{
    DDI_CHK_NULL(formatList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(numFormats, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    int32_t num = 0;
    uint32_t maxNum = GetImageFormatsMaxNum();

    memset(formatList, 0,  sizeof(m_supportedImageformatsG9));
    for (uint32_t idx = 0; idx < maxNum; idx++)
    {
        formatList[num].fourcc           = m_supportedImageformatsG9[idx].fourcc;
        formatList[num].byte_order       = m_supportedImageformatsG9[idx].byte_order;
        formatList[num].bits_per_pixel   = m_supportedImageformatsG9[idx].bits_per_pixel;
        formatList[num].depth            = m_supportedImageformatsG9[idx].depth;
        formatList[num].red_mask         = m_supportedImageformatsG9[idx].red_mask;
        formatList[num].green_mask       = m_supportedImageformatsG9[idx].green_mask;
        formatList[num].blue_mask        = m_supportedImageformatsG9[idx].blue_mask;
        formatList[num].alpha_mask       = m_supportedImageformatsG9[idx].alpha_mask;
        num++;
    }
    *numFormats = num;

    return VA_STATUS_SUCCESS;
}

uint32_t MediaLibvaCapsG9::GetImageFormatsMaxNum()
{
    return sizeof(m_supportedImageformatsG9)/sizeof(m_supportedImageformatsG9[0]);
}

bool MediaLibvaCapsG9::IsImageSupported(uint32_t fourcc)
{
    uint32_t maxNum = GetImageFormatsMaxNum();
    for (int32_t idx = 0; idx < maxNum; idx++)
    {
        if (m_supportedImageformatsG9[idx].fourcc == fourcc)
        {
            return true;
        }
    }

    return false;
}

VAStatus MediaLibvaCapsG9::PopulateColorMaskInfo(VAImageFormat *vaImgFmt)
{
    uint32_t maxNum = GetImageFormatsMaxNum();

    DDI_CHK_NULL(vaImgFmt, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    for (int32_t idx = 0; idx < maxNum; idx++)
    {
        if (m_supportedImageformatsG9[idx].fourcc == vaImgFmt->fourcc)
        {
            vaImgFmt->red_mask   = m_supportedImageformatsG9[idx].red_mask;
            vaImgFmt->green_mask = m_supportedImageformatsG9[idx].green_mask;
            vaImgFmt->blue_mask  = m_supportedImageformatsG9[idx].blue_mask;
            vaImgFmt->alpha_mask = m_supportedImageformatsG9[idx].alpha_mask;

            return VA_STATUS_SUCCESS;
        }
    }

    return VA_STATUS_ERROR_INVALID_IMAGE_FORMAT;
}

VAStatus MediaLibvaCapsG9::GetPlatformSpecificAttrib(VAProfile profile,
        VAEntrypoint entrypoint,
        VAConfigAttribType type,
        uint32_t *value)
{
    DDI_CHK_NULL(value, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    VAStatus status = VA_STATUS_SUCCESS;
    *value = VA_ATTRIB_NOT_SUPPORTED;
    switch ((int)type)
    {
        case VAConfigAttribEncMaxRefFrames:
        {
            if (entrypoint == VAEntrypointEncSliceLP || !IsHevcProfile(profile))
            {
                status = VA_STATUS_ERROR_INVALID_PARAMETER;
            }
            else
            {
                *value = ENCODE_DP_HEVC_NUM_MAX_VME_L0_REF_G9 | (ENCODE_DP_HEVC_NUM_MAX_VME_L1_REF_G9 << 16);;
            }
            break;
        }
        case VAConfigAttribDecProcessing:
        {
            if (IsAvcProfile(profile) || IsHevcProfile(profile))
            {
                *value = VA_DEC_PROCESSING;
            }
            else
            {
                *value = VA_DEC_PROCESSING_NONE;
            }
            break;
        }
        case VAConfigAttribEncIntraRefresh:
        {
            if(IsAvcProfile(profile))
            {
                *value = VA_ENC_INTRA_REFRESH_ROLLING_COLUMN |
                    VA_ENC_INTRA_REFRESH_ROLLING_ROW;
            }
            else if(entrypoint == VAEntrypointEncSlice && IsHevcProfile(profile))
            {
                *value = VA_ENC_INTRA_REFRESH_ROLLING_COLUMN;
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
                // the capacity is differnt for CQP and BRC mode, set it as larger one here
                roi_attr.bits.num_roi_regions = ENCODE_DP_AVC_MAX_ROI_NUM_BRC;
                roi_attr.bits.roi_rc_priority_support = 0;
                roi_attr.bits.roi_rc_qp_delta_support = 1;
            }
            else if (IsHevcProfile(profile))
            {
                roi_attr.bits.num_roi_regions = ENCODE_DP_HEVC_MAX_NUM_ROI;
                roi_attr.bits.roi_rc_priority_support = 0;
                roi_attr.bits.roi_rc_qp_delta_support = 1;
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
            if (entrypoint == VAEntrypointEncSlice && IsHevcProfile(profile))
            {
                *value = CODECHAL_HEVC_MAX_NUM_SLICES_LVL_5;
            }
            else
            {
                *value =0;
                status = VA_STATUS_ERROR_INVALID_PARAMETER;
            }
            break;
        }
        case VAConfigAttribMaxPictureWidth:
        {
            if(profile == VAProfileJPEGBaseline)
            {
                *value = ENCODE_JPEG_MAX_PIC_WIDTH;
            }
            else if(IsHevcProfile(profile) || IsAvcProfile(profile) || IsVp8Profile(profile))
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
            else if(IsHevcProfile(profile) || IsAvcProfile(profile) || IsVp8Profile(profile))
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

VAStatus MediaLibvaCapsG9::LoadProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;
    status = LoadAvcDecProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
    status = LoadAvcEncProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
    status = LoadAvcEncLpProfileEntrypoints();
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
    status = LoadHevcDecProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
    status = LoadHevcEncProfileEntrypoints();
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
    status = m_CapsCp->LoadCpProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize CP Caps!");
    return status;
}

VAStatus MediaLibvaCapsG9::QueryAVCROIMaxNum(uint32_t rcMode, bool isVdenc, uint32_t *maxNum, bool *isRoiInDeltaQP)
{
    DDI_CHK_NULL(maxNum, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(isRoiInDeltaQP, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    if(isVdenc)
    {
        *maxNum = ENCODE_VDENC_AVC_MAX_ROI_NUMBER_G9;
    }
    else
    {
        switch (rcMode)
        {
            case VA_RC_CQP:
                *maxNum = ENCODE_DP_AVC_MAX_ROI_NUMBER;
                break;
            default:
                *maxNum = ENCODE_DP_AVC_MAX_ROI_NUM_BRC;
                break;
        }
    }

    *isRoiInDeltaQP = true;
    return VA_STATUS_SUCCESS;
}
