/*
* Copyright (c) 2017-2020, Intel Corporation
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
//! \file     media_libva_caps_g10.cpp
//! \brief    This file implements the C++ class/interface for gen10 media capbilities. 
//!

#include "codec_def_encode_hevc_g10.h"
#include "media_libva_util.h"
#include "media_libva.h"
#include "media_libva_caps_cp_interface.h"
#include "media_libva_caps_g10.h"
#include "media_libva_caps_factory.h"

const VAImageFormat m_supportedImageformatsG10[] =
{   {VA_FOURCC_BGRA,           VA_LSB_FIRST,   32, 32, 0x0000ff00, 0x00ff0000, 0xff000000,  0x000000ff}, /* [31:0] B:G:R:A 8:8:8:8 little endian */
    {VA_FOURCC_ARGB,           VA_LSB_FIRST,   32, 32, 0x00ff0000, 0x0000ff00, 0x000000ff,  0xff000000}, /* [31:0] A:R:G:B 8:8:8:8 little endian */
    {VA_FOURCC_RGBA,           VA_LSB_FIRST,   32, 32, 0xff000000, 0x00ff0000, 0x0000ff00,  0x000000ff}, /* [31:0] R:G:B:A 8:8:8:8 little endian */
    {VA_FOURCC_ABGR,           VA_LSB_FIRST,   32, 32, 0x000000ff, 0x0000ff00, 0x00ff0000,  0xff000000}, /* [31:0] A:B:G:R 8:8:8:8 little endian */
    {VA_FOURCC_BGRX,           VA_LSB_FIRST,   32, 24, 0x0000ff00, 0x00ff0000, 0xff000000,  0},          /* [31:0] B:G:R:x 8:8:8:8 little endian */
    {VA_FOURCC_XRGB,           VA_LSB_FIRST,   32, 24, 0x00ff0000, 0x0000ff00, 0x000000ff,  0},          /* [31:0] x:R:G:B 8:8:8:8 little endian */
    {VA_FOURCC_RGBX,           VA_LSB_FIRST,   32, 24, 0xff000000, 0x00ff0000, 0x0000ff00,  0},          /* [31:0] R:G:B:x 8:8:8:8 little endian */
    {VA_FOURCC_XBGR,           VA_LSB_FIRST,   32, 24, 0x000000ff, 0x0000ff00, 0x00ff0000,  0},          /* [31:0] x:B:G:R 8:8:8:8 little endian */
    {VA_FOURCC_A2R10G10B10,    VA_LSB_FIRST,   32, 30, 0x3ff00000, 0x000ffc00, 0x000003ff,  0x30000000}, /* [31:0] A:R:G:B 2:10:10:10 little endian */
    {VA_FOURCC_A2B10G10R10,    VA_LSB_FIRST,   32, 30, 0x000003ff, 0x000ffc00, 0x3ff00000,  0x30000000}, /* [31:0] A:B:G:R 2:10:10:10 little endian */
    {VA_FOURCC_X2R10G10B10,    VA_LSB_FIRST,   32, 30, 0x3ff00000, 0x000ffc00, 0x000003ff,  0},          /* [31:0] X:R:G:B 2:10:10:10 little endian */
    {VA_FOURCC_X2B10G10R10,    VA_LSB_FIRST,   32, 30, 0x000003ff, 0x000ffc00, 0x3ff00000,  0},          /* [31:0] X:B:G:R 2:10:10:10 little endian */
    {VA_FOURCC_RGB565,         VA_LSB_FIRST,   16, 16, 0xf800,     0x07e0,     0x001f,      0},          /* [15:0] R:G:B 5:6:5 little endian */
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

VAStatus MediaLibvaCapsG10::QueryImageFormats(VAImageFormat *formatList, int32_t *numFormats)
{
    DDI_CHK_NULL(formatList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(numFormats, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    int32_t num = 0;
    uint32_t maxNum = GetImageFormatsMaxNum();

    memset(formatList, 0,  sizeof(m_supportedImageformatsG10));
    for (uint32_t idx = 0; idx < maxNum; idx++)
    {
        formatList[num].fourcc           = m_supportedImageformatsG10[idx].fourcc;
        formatList[num].byte_order       = m_supportedImageformatsG10[idx].byte_order;
        formatList[num].bits_per_pixel   = m_supportedImageformatsG10[idx].bits_per_pixel;
        formatList[num].depth            = m_supportedImageformatsG10[idx].depth;
        formatList[num].red_mask         = m_supportedImageformatsG10[idx].red_mask;
        formatList[num].green_mask       = m_supportedImageformatsG10[idx].green_mask;
        formatList[num].blue_mask        = m_supportedImageformatsG10[idx].blue_mask;
        formatList[num].alpha_mask       = m_supportedImageformatsG10[idx].alpha_mask;
        num++;
    }
    *numFormats = num;

    return VA_STATUS_SUCCESS;
}

uint32_t MediaLibvaCapsG10::GetImageFormatsMaxNum()
{
    return sizeof(m_supportedImageformatsG10)/sizeof(m_supportedImageformatsG10[0]);
}

bool MediaLibvaCapsG10::IsImageSupported(uint32_t fourcc)
{
    uint32_t maxNum = GetImageFormatsMaxNum();
    for (int32_t idx = 0; idx < maxNum; idx++)
    {
        if (m_supportedImageformatsG10[idx].fourcc == fourcc)
        {
            return true;
        }
    }
    
    return false;
}

VAStatus MediaLibvaCapsG10::PopulateColorMaskInfo(VAImageFormat *vaImgFmt)
{
    uint32_t maxNum = GetImageFormatsMaxNum();

    DDI_CHK_NULL(vaImgFmt, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    for (int32_t idx = 0; idx < maxNum; idx++)
    {
        if (m_supportedImageformatsG10[idx].fourcc == vaImgFmt->fourcc)
        {
            vaImgFmt->red_mask   = m_supportedImageformatsG10[idx].red_mask;
            vaImgFmt->green_mask = m_supportedImageformatsG10[idx].green_mask;
            vaImgFmt->blue_mask  = m_supportedImageformatsG10[idx].blue_mask;
            vaImgFmt->alpha_mask = m_supportedImageformatsG10[idx].alpha_mask;

            return VA_STATUS_SUCCESS;
        }
    }

    return VA_STATUS_ERROR_INVALID_IMAGE_FORMAT;
}

VAStatus MediaLibvaCapsG10::GetPlatformSpecificAttrib(VAProfile profile,
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
                *value = ENCODE_DP_HEVC_NUM_MAX_VME_L0_REF_G10 | (ENCODE_DP_HEVC_NUM_MAX_VME_L1_REF_G10 << 16);;
            }
            break;
        }
        case VAConfigAttribDecProcessing:
        {
#ifdef _DECODE_PROCESSING_SUPPORTED
            if (IsAvcProfile(profile) || IsHevcProfile(profile))
            {
                *value = VA_DEC_PROCESSING;
            }
            else
#endif
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

            *value = roi_attr.value;
            break;
        }
        case VAConfigAttribCustomRoundingControl:
        {
            *value = 0;
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

VAStatus MediaLibvaCapsG10::LoadHevcEncLpProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
    AttribMap *attributeList = nullptr;

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain10))
    {
        status = CreateEncAttributes(VAProfileHEVCMain, VAEntrypointEncSliceLP, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
        {
            for (int32_t j = 3; j < 7; j++)
            {
                AddEncConfig(m_encRcMode[j]);
                AddEncConfig(m_encRcMode[j] | VA_RC_PARALLEL);
            }
        }
        AddProfileEntry(VAProfileHEVCMain, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain10))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
        {
            for (int32_t j = 3; j < 7; j++)
            {
                AddEncConfig(m_encRcMode[j]);
                AddEncConfig(m_encRcMode[j] | VA_RC_PARALLEL);
            }
        }
        AddProfileEntry(VAProfileHEVCMain10, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }
#endif
    return status;
}

VAStatus MediaLibvaCapsG10::LoadVp9EncProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _VP9_ENCODE_VDENC_SUPPORTED
    AttribMap *attributeList;
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeVP9Vdenc) &&
        MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
    {
        status = CreateEncAttributes(VAProfileVP9Profile0, VAEntrypointEncSliceLP, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");

        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        AddEncConfig(VA_RC_CBR);
        AddEncConfig(VA_RC_VBR);
        AddProfileEntry(VAProfileVP9Profile0, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }
#endif
    return status;
}

VAStatus MediaLibvaCapsG10::LoadProfileEntrypoints()
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
    status = LoadHevcEncLpProfileEntrypoints();
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
VAStatus MediaLibvaCapsG10::CheckEncodeResolution(
        VAProfile profile,
        uint32_t width,
        uint32_t height)
{
    switch (profile)
    {
        case VAProfileJPEGBaseline:
            if (width > m_encJpegMaxWidth
                    || width < m_encJpegMinWidth
                    || height > m_encJpegMaxHeight
                    || height < m_encJpegMinHeight)
            {
                return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
            }
            break;
        case VAProfileMPEG2Simple:
        case VAProfileMPEG2Main:
            if( width > CODEC_MAX_PIC_WIDTH
                    || width < m_encMinWidth
                    || height > CODEC_MAX_PIC_HEIGHT
                    || height < m_encMinHeight)
            {
                return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
            }
            break;
        case VAProfileHEVCMain:
        case VAProfileHEVCMain10:
            if (width > m_maxHevcEncWidth
                    || width < m_encMinWidth
                    || height > m_maxHevcEncHeight
                    || height < m_encMinHeight)
            {
                return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
            }
            break;
        case VAProfileVP9Profile0:
            if ((width > m_encMax4kWidth) ||
                (width < m_encMinWidth) ||
                (height > m_encMax4kHeight) ||
                (height < m_encMinHeight) ||
                (width % 8) ||
                (height % 8))
            {
                return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
            }
            break;
        default:
            if (width > m_encMax4kWidth
                    || width < m_encMinWidth
                    || height > m_encMax4kHeight
                    || height < m_encMinHeight)
            {
                return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
            }
            break;
    }
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCapsG10::CheckDecodeResolution(
        int32_t codecMode,
        VAProfile profile,
        uint32_t width,
        uint32_t height)
{

    uint32_t maxWidth, maxHeight;
    switch (codecMode)
    {
        case CODECHAL_DECODE_MODE_MPEG2VLD:
            maxWidth = m_decMpeg2MaxWidth;
            maxHeight = m_decMpeg2MaxHeight;
            break;
        case CODECHAL_DECODE_MODE_VC1VLD:
            maxWidth = m_decVc1MaxWidth;
            maxHeight = m_decVc1MaxHeight;
            break;
        case CODECHAL_DECODE_MODE_JPEG:
            maxWidth = m_decJpegMaxWidth;
            maxHeight = m_decJpegMaxHeight;
            break;
        case CODECHAL_DECODE_MODE_HEVCVLD:
            maxWidth = m_decHevcMaxWidth;
            maxHeight = m_decHevcMaxHeight;
            break;
        case CODECHAL_DECODE_MODE_VP9VLD:
            maxWidth = m_decVp9MaxWidth;
            maxHeight = m_decVp9MaxHeight;
            break;
        default:
            maxWidth = m_decDefaultMaxWidth;
            maxHeight = m_decDefaultMaxHeight;
            break;
    }

    uint32_t alignedHeight;
    if (profile == VAProfileVC1Advanced)
    {
        alignedHeight = MOS_ALIGN_CEIL(height,32);
    }
    else
    {
        alignedHeight = height;
    }

    if (width > maxWidth || alignedHeight > maxHeight)
    {
        return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
    }
    else
    {
        return VA_STATUS_SUCCESS;
    }
}

VAStatus MediaLibvaCapsG10::QueryAVCROIMaxNum(uint32_t rcMode, bool isVdenc, uint32_t *maxNum, bool *isRoiInDeltaQP)
{
    DDI_CHK_NULL(maxNum, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(isRoiInDeltaQP, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    if(isVdenc)
    {
        *maxNum = ENCODE_VDENC_AVC_MAX_ROI_NUMBER;
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

VAStatus MediaLibvaCapsG10::LoadAvcEncProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#if defined (_AVC_ENCODE_VME_SUPPORTED) || defined (_AVC_ENCODE_VDENC_SUPPORTED)
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeAVC))
    {
        AttribMap *attributeList;
        VAProfile profile[3] = {
            VAProfileH264Main,
            VAProfileH264High,
            VAProfileH264ConstrainedBaseline};

        uint32_t configStartIdx;

        for (int32_t i = 0; i < 3; i++)
        {
            status = CreateEncAttributes(profile[i],
                    VAEntrypointEncSlice,
                    &attributeList);

            DDI_CHK_RET(status, "Failed to initialize Caps!");
            configStartIdx = m_encConfigs.size();
            int32_t maxRcMode = 7;
            for (int32_t j = 0; j < maxRcMode; j++)
            {
                AddEncConfig(m_encRcMode[j]);
            }
            AddProfileEntry(profile[i], VAEntrypointEncSlice, attributeList,
                        configStartIdx, m_encConfigs.size() - configStartIdx);
        }
    }
#endif
    return status;
}

VAStatus MediaLibvaCapsG10::LoadHevcEncProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _HEVC_ENCODE_VME_SUPPORTED
    AttribMap *attributeList = nullptr;

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVC)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVC10bit))
    {
        status = CreateEncAttributes(VAProfileHEVCMain, VAEntrypointEncSlice, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");
        DDI_CHK_NULL(attributeList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        for (int32_t j = 3; j < 7; j++)
        {
            AddEncConfig(m_encRcMode[j]);
            AddEncConfig(m_encRcMode[j] | VA_RC_PARALLEL);
        }

        AddProfileEntry(VAProfileHEVCMain, VAEntrypointEncSlice, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);

        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVC10bit))
        {
            configStartIdx = m_encConfigs.size();
            AddEncConfig(VA_RC_CQP);
            for (int32_t j = 3; j < 7; j++)
            {
                AddEncConfig(m_encRcMode[j]);
                AddEncConfig(m_encRcMode[j] | VA_RC_PARALLEL);
            }
            AddProfileEntry(VAProfileHEVCMain10, VAEntrypointEncSlice, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
        }
    }

#endif
    return status;
}

