/*
* Copyright (c) 2018-2019, Intel Corporation
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
//! \file     media_libva_caps_g12.cpp
//! \brief    This file implements the C++ class/interface for gen12 media capbilities.
//!

#include "codec_def_encode_hevc_g12.h"
#include "media_libva_util.h"
#include "media_libva.h"
#include "media_libva_vp.h"
#include "media_libva_caps_cp_interface.h"
#include "media_libva_caps_g12.h"
#include "media_libva_caps_factory.h"
#include "media_ddi_decode_const.h"
#include "media_ddi_encode_const.h"
#include "media_ddi_decode_const_g12.h"

#ifndef VA_ENCRYPTION_TYPE_NONE
#define VA_ENCRYPTION_TYPE_NONE 0x00000000
#endif

const VAImageFormat m_supportedImageformatsG12[] =
{   {VA_FOURCC_BGRA,   VA_LSB_FIRST,   32, 24, 0x00ff0000, 0x0000ff00, 0x000000ff,  0xff000000},
    {VA_FOURCC_ARGB,   VA_LSB_FIRST,   32, 24, 0x00ff0000, 0x0000ff00, 0x000000ff,  0xff000000},
    {VA_FOURCC_RGBA,   VA_LSB_FIRST,   32, 24, 0x000000ff, 0x0000ff00, 0x00ff0000,  0xff000000},
    {VA_FOURCC_ABGR,   VA_LSB_FIRST,   32, 24, 0x000000ff, 0x0000ff00, 0x00ff0000,  0xff000000},
    {VA_FOURCC_BGRX,   VA_LSB_FIRST,   32, 24, 0x00ff0000, 0x0000ff00, 0x000000ff,  0},
    {VA_FOURCC_XRGB,   VA_LSB_FIRST,   32, 24, 0x00ff0000, 0x0000ff00, 0x000000ff,  0},
    {VA_FOURCC_RGBX,   VA_LSB_FIRST,   32, 24, 0x000000ff, 0x0000ff00, 0x00ff0000,  0},
    {VA_FOURCC_XBGR,   VA_LSB_FIRST,   32, 24, 0x000000ff, 0x0000ff00, 0x00ff0000,  0},
    {VA_FOURCC_RGB565, VA_LSB_FIRST,   16, 16, 0xf800,     0x07e0,     0x001f,      0},
    {VA_FOURCC_AYUV,   VA_LSB_FIRST,   32, 24, 0x00ff0000, 0x0000ff00, 0x000000ff,  0xff000000},
    {VA_FOURCC_NV12,   VA_LSB_FIRST,   12, 0,0,0,0,0},
    {VA_FOURCC_NV21,   VA_LSB_FIRST,   12, 0,0,0,0,0},
    {VA_FOURCC_YUY2,   VA_LSB_FIRST,   16, 0,0,0,0,0},
    {VA_FOURCC_UYVY,   VA_LSB_FIRST,   16, 0,0,0,0,0},
    {VA_FOURCC_YV12,   VA_LSB_FIRST,   12, 0,0,0,0,0},
    {VA_FOURCC_I420,   VA_LSB_FIRST,   12, 0,0,0,0,0},
    {VA_FOURCC_422H,   VA_LSB_FIRST,   16, 0,0,0,0,0},
    {VA_FOURCC_422V,   VA_LSB_FIRST,   16, 0,0,0,0,0},
    {VA_FOURCC_444P,   VA_LSB_FIRST,   24, 0,0,0,0,0},
    {VA_FOURCC_IMC3,   VA_LSB_FIRST,   16, 0,0,0,0,0},
    {VA_FOURCC_P010,   VA_LSB_FIRST,   24, 0,0,0,0,0},
    {VA_FOURCC_P016,   VA_LSB_FIRST,   24, 0,0,0,0,0},
    {VA_FOURCC_Y210,   VA_LSB_FIRST,   32, 0,0,0,0,0},
    {VA_FOURCC_Y216,   VA_LSB_FIRST,   32, 0,0,0,0,0},
    {VA_FOURCC_Y410,   VA_LSB_FIRST,   32, 0,0,0,0,0},
    {VA_FOURCC_Y416,   VA_LSB_FIRST,   64, 0,0,0,0,0},
    {VA_FOURCC_A2R10G10B10,    VA_LSB_FIRST,   32, 30, 0x3ff00000, 0x000ffc00, 0x000003ff, 0x30000000},  /* [31:0] A:R:G:B 2:10:10:10 little endian */
    {VA_FOURCC_A2B10G10R10,    VA_LSB_FIRST,   32, 30, 0x000003ff, 0x000ffc00, 0x3ff00000, 0x30000000}   /* [31:0] A:B:G:R 2:10:10:10 little endian */
};

const VAConfigAttribValEncRateControlExt MediaLibvaCapsG12::m_encVp9RateControlExt =
{
    {CODECHAL_ENCODE_VP9_MAX_NUM_TEMPORAL_LAYERS - 1, 1, 0}
};

VAStatus MediaLibvaCapsG12::QueryImageFormats(VAImageFormat *formatList, int32_t *numFormats)
{
    DDI_CHK_NULL(formatList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(numFormats, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    int32_t num = 0;
    uint32_t maxNum = GetImageFormatsMaxNum();

    memset(formatList, 0,  sizeof(m_supportedImageformatsG12));
    for (uint32_t idx = 0; idx < maxNum; idx++)
    {
        formatList[num].fourcc           = m_supportedImageformatsG12[idx].fourcc;
        formatList[num].byte_order       = m_supportedImageformatsG12[idx].byte_order;
        formatList[num].bits_per_pixel   = m_supportedImageformatsG12[idx].bits_per_pixel;
        formatList[num].depth            = m_supportedImageformatsG12[idx].depth;
        formatList[num].red_mask         = m_supportedImageformatsG12[idx].red_mask;
        formatList[num].green_mask       = m_supportedImageformatsG12[idx].green_mask;
        formatList[num].blue_mask        = m_supportedImageformatsG12[idx].blue_mask;
        formatList[num].alpha_mask       = m_supportedImageformatsG12[idx].alpha_mask;
        num++;
    }
    *numFormats = num;

    return VA_STATUS_SUCCESS;
}

uint32_t MediaLibvaCapsG12::GetImageFormatsMaxNum()
{
    return sizeof(m_supportedImageformatsG12)/sizeof(m_supportedImageformatsG12[0]);
}

bool MediaLibvaCapsG12::IsImageSupported(uint32_t fourcc)
{
    uint32_t maxNum = GetImageFormatsMaxNum();
    for (int32_t idx = 0; idx < maxNum; idx++)
    {
        if (m_supportedImageformatsG12[idx].fourcc == fourcc)
        {
            return true;
        }
    }

    return false;
}

VAStatus MediaLibvaCapsG12::PopulateColorMaskInfo(VAImageFormat *vaImgFmt)
{
    uint32_t maxNum = GetImageFormatsMaxNum();

    DDI_CHK_NULL(vaImgFmt, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    for (int32_t idx = 0; idx < maxNum; idx++)
    {
        if (m_supportedImageformatsG12[idx].fourcc == vaImgFmt->fourcc)
        {
            vaImgFmt->red_mask   = m_supportedImageformatsG12[idx].red_mask;
            vaImgFmt->green_mask = m_supportedImageformatsG12[idx].green_mask;
            vaImgFmt->blue_mask  = m_supportedImageformatsG12[idx].blue_mask;
            vaImgFmt->alpha_mask = m_supportedImageformatsG12[idx].alpha_mask;

            return VA_STATUS_SUCCESS;
        }
    }

    return VA_STATUS_ERROR_INVALID_IMAGE_FORMAT;
}

CODECHAL_MODE MediaLibvaCapsG12::GetEncodeCodecMode(VAProfile profile, VAEntrypoint entrypoint)
{
    if (entrypoint == VAEntrypointStats)
    {
        return  CODECHAL_ENCODE_MODE_AVC;
    }

    switch (profile)
    {
        case VAProfileH264High:
        case VAProfileH264Main:
        case VAProfileH264ConstrainedBaseline:
            return CODECHAL_ENCODE_MODE_AVC;
        case VAProfileMPEG2Main:
        case VAProfileMPEG2Simple:
            return CODECHAL_ENCODE_MODE_MPEG2;
        case VAProfileJPEGBaseline:
            return CODECHAL_ENCODE_MODE_JPEG;
        case VAProfileVP8Version0_3:
            return CODECHAL_ENCODE_MODE_VP8;
        case VAProfileVP9Profile0:
        case VAProfileVP9Profile1:
        case VAProfileVP9Profile2:
        case VAProfileVP9Profile3:
            return CODECHAL_ENCODE_MODE_VP9;
        case VAProfileHEVCMain:
        case VAProfileHEVCMain10:
        case VAProfileHEVCMain12:
        case VAProfileHEVCMain422_10:
        case VAProfileHEVCMain422_12:
        case VAProfileHEVCMain444:
        case VAProfileHEVCMain444_10:
            return CODECHAL_ENCODE_MODE_HEVC;
        default:
            DDI_ASSERTMESSAGE("Invalid Encode Mode");
            return CODECHAL_UNSUPPORTED_MODE;
    }
}

CODECHAL_MODE MediaLibvaCapsG12::GetDecodeCodecMode(VAProfile profile)
{
    int8_t vaProfile = (int8_t)profile;
    switch (vaProfile)
    {
        case VAProfileH264High:
        case VAProfileH264Main:
        case VAProfileH264ConstrainedBaseline:
            return CODECHAL_DECODE_MODE_AVCVLD;
        case VAProfileMPEG2Main:
        case VAProfileMPEG2Simple:
            return CODECHAL_DECODE_MODE_MPEG2VLD;
        case VAProfileJPEGBaseline:
            return CODECHAL_DECODE_MODE_JPEG;
        case VAProfileVP8Version0_3:
            return CODECHAL_DECODE_MODE_VP8VLD;
        case VAProfileVP9Profile0:
        case VAProfileVP9Profile1:
        case VAProfileVP9Profile2:
        case VAProfileVP9Profile3:
            return CODECHAL_DECODE_MODE_VP9VLD;
        case VAProfileHEVCMain:
        case VAProfileHEVCMain10:
        case VAProfileHEVCMain12:
        case VAProfileHEVCMain422_10:
        case VAProfileHEVCMain422_12:
        case VAProfileHEVCMain444:
        case VAProfileHEVCMain444_10:
        case VAProfileHEVCMain444_12:
        case VAProfileHEVCSccMain:
        case VAProfileHEVCSccMain10:
        case VAProfileHEVCSccMain444:
            return CODECHAL_DECODE_MODE_HEVCVLD;
        case VAProfileVC1Simple:
        case VAProfileVC1Main:
        case VAProfileVC1Advanced:
            return CODECHAL_DECODE_MODE_VC1VLD;
        default:
            DDI_ASSERTMESSAGE("Invalid Decode Mode");
            return CODECHAL_UNSUPPORTED_MODE;
    }
}

std::string MediaLibvaCapsG12::GetDecodeCodecKey(VAProfile profile)
{
    int8_t vaProfile = (int8_t)profile;
    switch (vaProfile)
    {
        case VAProfileH264High:
        case VAProfileH264Main:
        case VAProfileH264ConstrainedBaseline:
            return DECODE_ID_AVC;
        case VAProfileMPEG2Main:
        case VAProfileMPEG2Simple:
            return DECODE_ID_MPEG2;
        case VAProfileJPEGBaseline:
            return DECODE_ID_JPEG;
        case VAProfileVP8Version0_3:
            return DECODE_ID_VP8;
        case VAProfileVP9Profile0:
        case VAProfileVP9Profile1:
        case VAProfileVP9Profile2:
        case VAProfileVP9Profile3:
            return DECODE_ID_VP9;
        case VAProfileHEVCMain:
        case VAProfileHEVCMain10:
        case VAProfileHEVCMain12:
        case VAProfileHEVCMain422_10:
        case VAProfileHEVCMain422_12:
        case VAProfileHEVCMain444:
        case VAProfileHEVCMain444_10:
        case VAProfileHEVCMain444_12:
        case VAProfileHEVCSccMain:
        case VAProfileHEVCSccMain10:
        case VAProfileHEVCSccMain444:
            return DECODE_ID_HEVC_G12;
        case VAProfileVC1Simple:
        case VAProfileVC1Main:
        case VAProfileVC1Advanced:
            return DECODE_ID_VC1;
        default:
            DDI_ASSERTMESSAGE("Invalid Decode Mode");
            return DECODE_ID_NONE;
    }
}

std::string MediaLibvaCapsG12::GetEncodeCodecKey(VAProfile profile, VAEntrypoint entrypoint, uint32_t feiFunction)
{
    switch (profile)
    {
        case VAProfileH264High:
        case VAProfileH264Main:
        case VAProfileH264ConstrainedBaseline:
            if (IsEncFei(entrypoint, feiFunction))
                return ENCODE_ID_AVCFEI;
            else
                return ENCODE_ID_AVC;
        case VAProfileMPEG2Main:
        case VAProfileMPEG2Simple:
            return ENCODE_ID_MPEG2;
        case VAProfileJPEGBaseline:
            return ENCODE_ID_JPEG;
        case VAProfileVP8Version0_3:
            return ENCODE_ID_VP8;
        case VAProfileVP9Profile0:
        case VAProfileVP9Profile1:
        case VAProfileVP9Profile2:
        case VAProfileVP9Profile3:
            return ENCODE_ID_VP9;
        case VAProfileHEVCMain:
        case VAProfileHEVCMain10:
        case VAProfileHEVCMain12:
        case VAProfileHEVCMain422_10:
        case VAProfileHEVCMain422_12:
        case VAProfileHEVCMain444:
        case VAProfileHEVCMain444_10:
            if (IsEncFei(entrypoint, feiFunction))
                return ENCODE_ID_HEVCFEI;
            else
                return ENCODE_ID_HEVC;
        case VAProfileNone:
            if (IsEncFei(entrypoint, feiFunction))
                return ENCODE_ID_AVCFEI;
            else
                return ENCODE_ID_NONE;
        default:
            return ENCODE_ID_NONE;
    }
}

VAStatus MediaLibvaCapsG12::GetPlatformSpecificAttrib(VAProfile profile,
        VAEntrypoint entrypoint,
        VAConfigAttribType type,
        uint32_t *value)
{
    DDI_CHK_NULL(value, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    VAStatus status = VA_STATUS_SUCCESS;
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
                *value = ENCODE_DP_HEVC_NUM_MAX_VME_L0_REF_G12 | (ENCODE_DP_HEVC_NUM_MAX_VME_L1_REF_G12 << 16);;
            }
            break;
        }
        case VAConfigAttribDecProcessing:
        {
#ifdef _DECODE_PROCESSING_SUPPORTED
            if (IsAvcProfile(profile) || IsHevcProfile(profile) || IsJpegProfile(profile) || IsVp9Profile(profile))
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
            if(IsAvcProfile(profile) || IsHevcProfile(profile))
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
            if (entrypoint == VAEntrypointEncSliceLP)
            {
                status = VA_STATUS_ERROR_INVALID_PARAMETER;
            }
            else if (IsAvcProfile(profile))
            {
                VAConfigAttribValEncROI roi_attrib = {0};
                roi_attrib.bits.num_roi_regions = ENCODE_DP_AVC_MAX_ROI_NUM_BRC;
                roi_attrib.bits.roi_rc_priority_support = 1;
                roi_attrib.bits.roi_rc_qp_delta_support = 1;
                *value = roi_attrib.value;
            }
            else if (IsHevcProfile(profile))
            {
                VAConfigAttribValEncROI roi_attrib = {0};
                roi_attrib.bits.num_roi_regions = CODECHAL_ENCODE_HEVC_MAX_NUM_ROI;
                roi_attrib.bits.roi_rc_priority_support = 0;
                roi_attrib.bits.roi_rc_qp_delta_support = 1;
                *value = roi_attrib.value;
            }
            break;
        }
        case VAConfigAttribCustomRoundingControl:
        {
            *value = 0;
            break;
        }
        default:
            status = VA_STATUS_ERROR_INVALID_PARAMETER;
            break;
    }
    return status;
}

VAStatus MediaLibvaCapsG12::LoadHevcEncProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _HEVC_ENCODE_VME_SUPPORTED

    status = MediaLibvaCaps::LoadHevcEncProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVC))
    {
        SetAttribute(VAProfileHEVCMain, VAEntrypointEncSlice, VAConfigAttribEncTileSupport, 1);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVC10bit))
    {
        SetAttribute(VAProfileHEVCMain10, VAEntrypointEncSlice, VAConfigAttribEncTileSupport, 1);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVC12bit))
    {
        SetAttribute(VAProfileHEVCMain12, VAEntrypointEncSlice, VAConfigAttribEncTileSupport, 1);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVC10bit422))
    {
        SetAttribute(VAProfileHEVCMain422_10, VAEntrypointEncSlice, VAConfigAttribEncTileSupport, 1);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVC12bit422))
    {
        SetAttribute(VAProfileHEVCMain422_12, VAEntrypointEncSlice, VAConfigAttribEncTileSupport, 1);
    }
#endif
    return status;
}

VAStatus MediaLibvaCapsG12::LoadHevcEncLpProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;
    const uint8_t rcModeSize = (sizeof(m_encRcMode))/(sizeof(m_encRcMode[0]));

#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
    AttribMap *attributeList = nullptr;

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain10)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain444)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain10bit444))
    {
        status = CreateEncAttributes(VAProfileHEVCMain, VAEntrypointEncSliceLP, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");
        (*attributeList)[VAConfigAttribMaxPictureWidth] = CODEC_16K_MAX_PIC_WIDTH;
        (*attributeList)[VAConfigAttribMaxPictureHeight] = CODEC_16K_MAX_PIC_HEIGHT;
        (*attributeList)[VAConfigAttribEncTileSupport] = 1;
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
        {
            for (int32_t j = 3; j < rcModeSize; j++)
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
            for (int32_t j = 3; j < rcModeSize; j++)
            {
                AddEncConfig(m_encRcMode[j]);
                AddEncConfig(m_encRcMode[j] | VA_RC_PARALLEL);
            }
        }
        AddProfileEntry(VAProfileHEVCMain10, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain444))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
        {
            for (int32_t j = 3; j < rcModeSize; j++)
            {
                AddEncConfig(m_encRcMode[j]);
                AddEncConfig(m_encRcMode[j] | VA_RC_PARALLEL);
            }
        }
        AddProfileEntry(VAProfileHEVCMain444, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain10bit444))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
        {
            for (int32_t j = 3; j < rcModeSize; j++)
            {
                AddEncConfig(m_encRcMode[j]);
                AddEncConfig(m_encRcMode[j] | VA_RC_PARALLEL);
            }
        }
        AddProfileEntry(VAProfileHEVCMain444_10, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }
#endif
    return status;
}

VAStatus MediaLibvaCapsG12::LoadVp9EncProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _VP9_ENCODE_VDENC_SUPPORTED
    AttribMap *attributeList = nullptr;
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels) &&
        (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeVP9Vdenc) ||
         MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeVP9Vdenc8bit444) ||
         MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeVP9Vdenc10bit420) ||
         MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeVP9Vdenc10bit444)))
    {
        status = CreateEncAttributes(VAProfileVP9Profile0, VAEntrypointEncSliceLP, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");
        (*attributeList)[VAConfigAttribMaxPictureWidth] = m_maxVp9EncWidth;
        (*attributeList)[VAConfigAttribMaxPictureHeight] = m_maxVp9EncHeight;
        (*attributeList)[VAConfigAttribEncDynamicScaling] = 1;
        (*attributeList)[VAConfigAttribEncTileSupport] = 1;
        (*attributeList)[VAConfigAttribEncRateControlExt] = m_encVp9RateControlExt.value;
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeVP9Vdenc) &&
        MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        AddEncConfig(VA_RC_CBR);
        AddEncConfig(VA_RC_VBR);
        AddProfileEntry(VAProfileVP9Profile0, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeVP9Vdenc8bit444) &&
        MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        AddEncConfig(VA_RC_CBR);
        AddEncConfig(VA_RC_VBR);
        AddProfileEntry(VAProfileVP9Profile1, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeVP9Vdenc10bit420) &&
        MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        AddEncConfig(VA_RC_CBR);
        AddEncConfig(VA_RC_VBR);
        AddProfileEntry(VAProfileVP9Profile2, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeVP9Vdenc10bit444) &&
        MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        AddEncConfig(VA_RC_CBR);
        AddEncConfig(VA_RC_VBR);
        AddProfileEntry(VAProfileVP9Profile3, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }
#endif
    return status;
}

VAStatus MediaLibvaCapsG12::LoadProfileEntrypoints()
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
    status = LoadNoneProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");

    return status;
}

VAStatus MediaLibvaCapsG12::CheckEncodeResolution(
        VAProfile profile,
        uint32_t width,
        uint32_t height)
{
    uint32_t maxWidth, maxHeight;

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
        case VAProfileHEVCMain:
        case VAProfileHEVCMain10:
        case VAProfileHEVCMain12:
        case VAProfileHEVCMain444:
        case VAProfileHEVCMain444_10:
        case VAProfileHEVCMain422_10:
        case VAProfileHEVCMain422_12:
            if (width > m_maxHevcEncWidth
                    || width < m_encMinWidth
                    || height > m_maxHevcEncHeight
                    || height < m_encMinHeight)
            {
                return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
            }
            break;
        case VAProfileVP9Profile0:
        case VAProfileVP9Profile1:
        case VAProfileVP9Profile2:
        case VAProfileVP9Profile3:
            if ((width > m_maxVp9EncWidth) ||
                (width < m_encMinWidth) ||
                (height > m_maxVp9EncHeight) ||
                (height < m_encMinHeight) )
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
VAStatus MediaLibvaCapsG12::CheckDecodeResolution(
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
            maxWidth = m_decHevcMax16kWidth;
            maxHeight = m_decHevcMax16kHeight;
            break;
        case CODECHAL_DECODE_MODE_VP9VLD:
        case CODECHAL_DECODE_RESERVED_0:
            maxWidth = m_decVp9Max16kWidth;
            maxHeight = m_decVp9Max16kHeight;
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

VAStatus MediaLibvaCapsG12::QuerySurfaceAttributes(
        VAConfigID configId,
        VASurfaceAttrib *attribList,
        uint32_t *numAttribs)
{
    DDI_CHK_NULL(numAttribs, "Null num_attribs", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (attribList == nullptr)
    {
        *numAttribs = DDI_CODEC_GEN_MAX_SURFACE_ATTRIBUTES;
        return VA_STATUS_SUCCESS;
    }

    int32_t profileTableIdx = -1;
    VAEntrypoint entrypoint;
    VAProfile profile;
    VAStatus status = GetProfileEntrypointFromConfigId(configId, &profile, &entrypoint, &profileTableIdx);
    DDI_CHK_RET(status, "Invalid config_id!");
    if (profileTableIdx < 0 || profileTableIdx >= m_profileEntryCount)
    {
        return VA_STATUS_ERROR_INVALID_CONFIG;
    }

    VASurfaceAttrib *attribs = (VASurfaceAttrib *)MOS_AllocAndZeroMemory(DDI_CODEC_GEN_MAX_SURFACE_ATTRIBUTES * sizeof(*attribs));
    if (attribs == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    int32_t i = 0;

    if (entrypoint == VAEntrypointVideoProc)   /* vpp */
    {
        attribs[i].type = VASurfaceAttribPixelFormat;
        attribs[i].value.type = VAGenericValueTypeInteger;
        attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
        attribs[i].value.value.i = VA_FOURCC('N', 'V', '1', '2');
        i++;

        attribs[i].type = VASurfaceAttribMaxWidth;
        attribs[i].value.type = VAGenericValueTypeInteger;
        attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
        attribs[i].value.value.i = VP_MAX_PIC_WIDTH;
        i++;

        attribs[i].type = VASurfaceAttribMaxHeight;
        attribs[i].value.type = VAGenericValueTypeInteger;
        attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
        attribs[i].value.value.i = VP_MAX_PIC_HEIGHT;
        i++;

        attribs[i].type = VASurfaceAttribMinWidth;
        attribs[i].value.type = VAGenericValueTypeInteger;
        attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
        attribs[i].value.value.i = VP_MIN_PIC_WIDTH;
        i++;

        attribs[i].type = VASurfaceAttribMinHeight;
        attribs[i].value.type = VAGenericValueTypeInteger;
        attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
        attribs[i].value.value.i = VP_MIN_PIC_HEIGHT;
        i++;

        for (int32_t j = 0; j < m_numVpSurfaceAttr; j++)
        {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = m_vpSurfaceAttr[j];
            i++;
        }

        attribs[i].type = VASurfaceAttribMemoryType;
        attribs[i].value.type = VAGenericValueTypeInteger;
        attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
#ifdef ANDROID
        attribs[i].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_VA |
            VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR |
            VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM |
            VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME |
            VA_SURFACE_ATTRIB_MEM_TYPE_ANDROID_GRALLOC;
#else
        attribs[i].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_VA |
            VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR |
            VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM |
            VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;
#endif
        i++;

        attribs[i].type = VASurfaceAttribExternalBufferDescriptor;
        attribs[i].value.type = VAGenericValueTypePointer;
        attribs[i].flags = VA_SURFACE_ATTRIB_SETTABLE;
        attribs[i].value.value.p = nullptr; /* ignore */
        i++;
    }
    else if (entrypoint == VAEntrypointVLD)    /* vld */
    {
        if (profile == VAProfileHEVCMain10 || profile == VAProfileVP9Profile2)
        {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_P010;
            i++;

            if(profile == VAProfileVP9Profile2)
            {
                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_P016;
                i++;
            }
        }
        else if(profile == VAProfileHEVCMain12)
        {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_P016;
            i++;
        }
        else if(profile == VAProfileHEVCMain422_10)
        {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_YUY2;
            i++;

            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y210;
            i++;
        }
        else if(profile == VAProfileHEVCMain422_12)
        {
            //hevc  rext: Y216 12/16bit 422
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y216;
            i++;
        }
        else if(profile == VAProfileHEVCMain444 || profile == VAProfileVP9Profile1)
        {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_AYUV;
            i++;
        }
        else if(profile == VAProfileHEVCMain444_10 || profile == VAProfileVP9Profile3)
        {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y410;
            i++;

            if(profile == VAProfileVP9Profile3)
            {
                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_Y416;
                i++;
            }
        }
        else if(profile == VAProfileHEVCMain444_12)
        {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y416;
            i++;
        }
        else if(profile == VAProfileHEVCSccMain10)
        {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_P010;
            i++;
        }
        else if(profile == VAProfileHEVCSccMain444)
        {
            //422/444 8/10bit included
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_YUY2;
            i++;

            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y210;
            i++;

            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_AYUV;
            i++;

            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y410;
            i++;
        }
        else if (profile == VAProfileJPEGBaseline)
        {
            for (int32_t j = 0; j < m_numJpegSurfaceAttr; j++)
            {
                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = m_jpegSurfaceAttr[j];
                i++;
            }
        }
        else
        {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC('N', 'V', '1', '2');
            i++;
        }

        auto maxWidth = m_decDefaultMaxWidth;
        auto maxHeight = m_decDefaultMaxHeight;
        if(IsMpeg2Profile(profile))
        {
            maxWidth = m_decMpeg2MaxWidth;
            maxHeight = m_decMpeg2MaxHeight;
        }
        else if(IsHevcProfile(profile))
        {
            maxWidth = m_decHevcMaxWidth;
            maxHeight = m_decHevcMaxHeight;
        }
        else if(IsVc1Profile(profile))
        {
            maxWidth = m_decVc1MaxWidth;
            maxHeight = m_decVc1MaxHeight;
        }
        else if(IsJpegProfile(profile))
        {
            maxWidth = m_decJpegMaxWidth;
            maxHeight = m_decJpegMaxHeight;
        }

        attribs[i].type = VASurfaceAttribMaxWidth;
        attribs[i].value.type = VAGenericValueTypeInteger;
        attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE;
        attribs[i].value.value.i = maxWidth;
        i++;

        attribs[i].type = VASurfaceAttribMaxHeight;
        attribs[i].value.type = VAGenericValueTypeInteger;
        attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE;
        attribs[i].value.value.i = maxHeight;
        i++;
    }
    else if(entrypoint == VAEntrypointEncSlice || entrypoint == VAEntrypointEncSliceLP || entrypoint == VAEntrypointEncPicture || entrypoint == VAEntrypointFEI)
    {
        if (profile == VAProfileHEVCMain10 || profile == VAProfileVP9Profile2)
        {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC('P', '0', '1', '0');
            i++;
        }
        else if(profile == VAProfileHEVCMain12)
        {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_P016;
            i++;
        }
        else if(profile == VAProfileHEVCMain444)
        {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_AYUV;
            i++;
        }
        else if(profile == VAProfileHEVCMain444_10)
        {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y410;
            i++;
        }
        else if(profile == VAProfileHEVCMain422_10)
        {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_YUY2;
            i++;

            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y210;
            i++;
        }
        else if(profile == VAProfileHEVCMain422_12)
        {
            //hevc  rext: Y216 12/16bit 422
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y216;
            i++;
        }
        else if (profile == VAProfileJPEGBaseline)
        {
            for (int32_t j = 0; j < m_numJpegEncSurfaceAttr; j++)
            {
                attribs[i].type = VASurfaceAttribPixelFormat;
                attribs[i].value.type = VAGenericValueTypeInteger;
                attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = m_jpegEncSurfaceAttr[j];
                i++;
            }
        }
        else
        {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC('N', 'V', '1', '2');
            i++;
        }
        attribs[i].type = VASurfaceAttribMaxWidth;
        attribs[i].value.type = VAGenericValueTypeInteger;
        attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE;
        attribs[i].value.value.i = CODEC_MAX_PIC_WIDTH;

        if(profile == VAProfileJPEGBaseline)
        {
            attribs[i].value.value.i = ENCODE_JPEG_MAX_PIC_WIDTH;
        }
        else if(IsHevcProfile(profile))
        {
            attribs[i].value.value.i = CODEC_16K_MAX_PIC_WIDTH;
        }
        if(IsAvcProfile(profile))
        {
            attribs[i].value.value.i = CODEC_4K_MAX_PIC_WIDTH;
        }
        i++;

        attribs[i].type = VASurfaceAttribMaxHeight;
        attribs[i].value.type = VAGenericValueTypeInteger;
        attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE;
        attribs[i].value.value.i = CODEC_MAX_PIC_HEIGHT;
        if(profile == VAProfileJPEGBaseline)
        {
            attribs[i].value.value.i = ENCODE_JPEG_MAX_PIC_HEIGHT;
        }
        else if(IsHevcProfile(profile))
        {
            attribs[i].value.value.i = CODEC_16K_MAX_PIC_HEIGHT;
        }
        if(IsAvcProfile(profile))
        {
            attribs[i].value.value.i = CODEC_4K_MAX_PIC_HEIGHT;
        }
        i++;

        attribs[i].type = VASurfaceAttribMinWidth;
        attribs[i].value.type = VAGenericValueTypeInteger;
        attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE;
        attribs[i].value.value.i = m_encMinWidth;
        if(profile == VAProfileJPEGBaseline)
        {
            attribs[i].value.value.i = m_encJpegMinWidth;
        }
        i++;

        attribs[i].type = VASurfaceAttribMinHeight;
        attribs[i].value.type = VAGenericValueTypeInteger;
        attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE;
        attribs[i].value.value.i = m_encMinHeight;
        if(profile == VAProfileJPEGBaseline)
        {
            attribs[i].value.value.i = m_encJpegMinHeight;
        }
        i++;
    }
    else
    {
        MOS_FreeMemory(attribs);
        return VA_STATUS_ERROR_UNIMPLEMENTED;
    }

    if (i > *numAttribs)
    {
        *numAttribs = i;
        MOS_FreeMemory(attribs);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    *numAttribs = i;
    MOS_SecureMemcpy(attribList, i * sizeof(*attribs), attribs, i * sizeof(*attribs));

    MOS_FreeMemory(attribs);
    return status;
}

VAStatus MediaLibvaCapsG12::CreateDecAttributes(
        VAProfile profile,
        VAEntrypoint entrypoint,
        AttribMap **attributeList)
{
    DDI_CHK_NULL(attributeList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus status = CreateAttributeList(attributeList);
    DDI_CHK_RET(status, "Failed to initialize Caps!");

    auto attribList = *attributeList;
    DDI_CHK_NULL(attribList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAConfigAttrib attrib;
    attrib.type = VAConfigAttribRTFormat;
    if ( profile == VAProfileJPEGBaseline )
    {
        // at present, latest libva have not support RGB24.
        attrib.value = VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV400 | VA_RT_FORMAT_YUV411 | VA_RT_FORMAT_RGB16 | VA_RT_FORMAT_RGB32;
    }
    else
    {
        attrib.value = VA_RT_FORMAT_YUV420;
    }
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribDecSliceMode;
    if (IsAvcProfile(profile))
    {
        attrib.value = VA_DEC_SLICE_MODE_NORMAL | VA_DEC_SLICE_MODE_BASE;
    }
    else if (IsHevcProfile(profile))
    {
        bool  hevcmainProfileSupported = false;
        attrib.value = 0;
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMainDecoding)
                || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain10Decoding)
                || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain12bit420Decoding)
                || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLD42210bitDecoding)
                || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain12bit422Decoding)
                || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLD4448bitDecoding)
                || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLD44410bitDecoding)
                || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain12bit444Decoding)
                || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain8bit420SCC)
                || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain10bit420SCC)
                || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain8bit444SCC)
                || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain10bit444SCC))
        {
            attrib.value |= VA_DEC_SLICE_MODE_NORMAL;
            hevcmainProfileSupported = true;
        }
        if ((MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrHEVCVLDMainShortDecoding) ||
                    MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrHEVCVLDMain10ShortDecoding))
                && MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
        {
            attrib.value |= VA_DEC_SLICE_MODE_BASE;
            hevcmainProfileSupported = true;
        }
        if (!hevcmainProfileSupported)
        {
            attrib.value = VA_ATTRIB_NOT_SUPPORTED;
        }
    }
    else if (profile == VAProfileVP9Profile0
          || profile == VAProfileVP9Profile2
          || profile == VAProfileVP9Profile1
          || profile == VAProfileVP9Profile3)
    {
        bool    vp9ProfileSupported = false;
        attrib.value = 0;
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelVP9VLDProfile0Decoding8bit420)
             || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelVP9VLDProfile1Decoding8bit444))
        {
            attrib.value |= VA_DEC_SLICE_MODE_NORMAL | VA_DEC_SLICE_MODE_BASE;
            vp9ProfileSupported = true;
        }
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrVP9VLD10bProfile2Decoding)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelVP9VLDProfile3Decoding10bit444))
        {
            attrib.value |= VA_DEC_SLICE_MODE_NORMAL;
            vp9ProfileSupported = true;
        }
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelVP9VLDProfile2Decoding12bit420)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelVP9VLDProfile3Decoding12bit444))
        {
            attrib.value |= VA_DEC_SLICE_MODE_NORMAL;
            vp9ProfileSupported = true;
        }
        if (!vp9ProfileSupported)
        {
            attrib.value = VA_ATTRIB_NOT_SUPPORTED;
        }
    }
    else
    {
        attrib.value = VA_DEC_SLICE_MODE_NORMAL;
    }
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribDecProcessing;
    attrib.value = VA_DEC_PROCESSING_NONE;
    GetPlatformSpecificAttrib(profile, entrypoint,
            VAConfigAttribDecProcessing, &attrib.value);
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribMaxPictureWidth;
    attrib.value = CODEC_MAX_PIC_WIDTH;
    if(profile == VAProfileJPEGBaseline)
    {
        attrib.value = ENCODE_JPEG_MAX_PIC_WIDTH;
    }
    if(IsVc1Profile(profile))
    {
        attrib.value = CODEC_2K_MAX_PIC_WIDTH;
    }
    if(IsVp8Profile(profile))
    {
        attrib.value = CODEC_4K_MAX_PIC_WIDTH;
    }
    if(IsAvcProfile(profile))
    {
        attrib.value = CODEC_8K_MAX_PIC_WIDTH;
    }
    if(IsHevcProfile(profile) || IsVp9Profile(profile))
    {
        attrib.value = CODEC_16K_MAX_PIC_WIDTH;
    }
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribMaxPictureHeight;
    attrib.value = CODEC_MAX_PIC_HEIGHT;
    if(profile == VAProfileJPEGBaseline)
    {
        attrib.value = ENCODE_JPEG_MAX_PIC_HEIGHT;
    }
    if(IsVc1Profile(profile))
    {
        attrib.value = CODEC_2K_MAX_PIC_HEIGHT;
    }
    if(IsVp8Profile(profile))
    {
        attrib.value = CODEC_4K_MAX_PIC_HEIGHT;
    }
    if(IsAvcProfile(profile))
    {
        attrib.value = CODEC_8K_MAX_PIC_HEIGHT;
    }
    if(IsHevcProfile(profile) || IsVp9Profile(profile))
    {
        attrib.value = CODEC_16K_MAX_PIC_HEIGHT;
    }
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribEncryption;

    attrib.value = VA_ATTRIB_NOT_SUPPORTED;
    if (m_isEntryptSupported)
    {
        uint32_t encryptTypes[3] = {0};
        int32_t numTypes =  m_CapsCp->GetEncryptionTypes(profile,
                encryptTypes, 3);
        if (numTypes > 0)
        {
            for (int32_t j = 0; j < numTypes; j++)
            {
                attrib.value |= encryptTypes[j];
            }
        }
    }
    (*attribList)[attrib.type] = attrib.value;

    if(profile == VAProfileJPEGBaseline)
    {
        attrib.type = VAConfigAttribDecJPEG;
        attrib.value = ((1 << VA_ROTATION_NONE) | (1 << VA_ROTATION_90) | (1 << VA_ROTATION_180) | (1 << VA_ROTATION_270));
        (*attribList)[attrib.type] = attrib.value;
    }

    if(profile == VAProfileNone)
    {
        attrib.type = (VAConfigAttribType)VAConfigAttribStats;
        VAConfigAttribValStats attribValStats;
        memset(&attribValStats, 0, sizeof(attribValStats));
        attribValStats.bits.max_num_past_references   = DDI_CODEC_STATS_MAX_NUM_PAST_REFS;
        attribValStats.bits.max_num_future_references = DDI_CODEC_STATS_MAX_NUM_FUTURE_REFS;
        attribValStats.bits.num_outputs               = DDI_CODEC_STATS_MAX_NUM_OUTPUTS;
        attribValStats.bits.interlaced                = DDI_CODEC_STATS_INTERLACED_SUPPORT;
        attrib.value = attribValStats.value;
        (*attribList)[attrib.type] = attrib.value;
    }

    attrib.type = VAConfigAttribProcessingRate;
    attrib.value = VA_PROCESSING_RATE_DECODE;
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = (VAConfigAttribType)VAConfigAttribCustomRoundingControl;
    GetPlatformSpecificAttrib(profile, entrypoint,
            (VAConfigAttribType)VAConfigAttribCustomRoundingControl, &attrib.value);
    (*attribList)[attrib.type] = attrib.value;

    return status;
}

VAStatus MediaLibvaCapsG12::LoadJpegDecProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _JPEG_DECODE_SUPPORTED
    AttribMap *attributeList = nullptr;
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelJPEGDecoding))
    {
        status = CreateDecAttributes(VAProfileJPEGBaseline, VAEntrypointVLD, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");

        uint32_t configStartIdx = m_decConfigs.size();
        for(int32_t i = 0; i < 2; i++)
        {
            AddDecConfig(VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, m_decProcessMode[i]);
        }
        AddProfileEntry(VAProfileJPEGBaseline, VAEntrypointVLD, attributeList, configStartIdx, 2);
    }
#endif

    return status;
}

VAStatus MediaLibvaCapsG12::LoadHevcDecProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _HEVC_DECODE_SUPPORTED
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMainDecoding)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrHEVCVLDMainShortDecoding))
    {
        LoadDecProfileEntrypoints(VAProfileHEVCMain);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain10Decoding)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrHEVCVLDMain10ShortDecoding))
    {
        LoadDecProfileEntrypoints(VAProfileHEVCMain10);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain12bit420Decoding))
    {
        LoadDecProfileEntrypoints(VAProfileHEVCMain12);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLD42210bitDecoding))
    {
        LoadDecProfileEntrypoints(VAProfileHEVCMain422_10);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain12bit422Decoding))
    {
        LoadDecProfileEntrypoints(VAProfileHEVCMain422_12);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLD4448bitDecoding))
    {
        LoadDecProfileEntrypoints(VAProfileHEVCMain444);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLD44410bitDecoding))
    {
        LoadDecProfileEntrypoints(VAProfileHEVCMain444_10);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain12bit444Decoding))
    {
        LoadDecProfileEntrypoints(VAProfileHEVCMain444_12);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain8bit420SCC))
    {
        LoadDecProfileEntrypoints(VAProfileHEVCSccMain);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain10bit420SCC))
    {
        LoadDecProfileEntrypoints(VAProfileHEVCSccMain10);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain8bit444SCC)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain10bit444SCC))
    {
        LoadDecProfileEntrypoints(VAProfileHEVCSccMain444);
    }

#endif
    return status;
}

bool MediaLibvaCapsG12::IsHevcProfile(VAProfile profile)
{
    return (
            (profile == VAProfileHEVCMain)       ||
            (profile == VAProfileHEVCMain10)     ||
            (profile == VAProfileHEVCMain12)     ||
            (profile == VAProfileHEVCMain422_10) ||
            (profile == VAProfileHEVCMain422_12) ||
            (profile == VAProfileHEVCMain444)    ||
            (profile == VAProfileHEVCMain444_10) ||
            (profile == VAProfileHEVCMain444_12) ||
            (profile == VAProfileHEVCSccMain)    ||
            (profile == VAProfileHEVCSccMain10)  ||
            (profile == VAProfileHEVCSccMain444)
           );
}

VAStatus MediaLibvaCapsG12::QueryAVCROIMaxNum(uint32_t rcMode, bool isVdenc, uint32_t *maxNum, bool *isRoiInDeltaQP)
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

GMM_RESOURCE_FORMAT MediaLibvaCapsG12::ConvertMediaFmtToGmmFmt(
    DDI_MEDIA_FORMAT format)
{
    switch (format)
    {
        case Media_Format_X8R8G8B8   : return GMM_FORMAT_B8G8R8X8_UNORM_TYPE;
        case Media_Format_A8R8G8B8   : return GMM_FORMAT_B8G8R8A8_UNORM_TYPE;
        case Media_Format_X8B8G8R8   : return GMM_FORMAT_R8G8B8X8_UNORM_TYPE;
        case Media_Format_A8B8G8R8   : return GMM_FORMAT_R8G8B8A8_UNORM_TYPE;
        case Media_Format_R8G8B8A8   : return GMM_FORMAT_R8G8B8A8_UNORM_TYPE;
        case Media_Format_R5G6B5     : return GMM_FORMAT_B5G6R5_UNORM_TYPE;
        case Media_Format_R8G8B8     : return GMM_FORMAT_R8G8B8_UNORM;
        case Media_Format_RGBP       : return GMM_FORMAT_RGBP;
        case Media_Format_BGRP       : return GMM_FORMAT_BGRP;
        case Media_Format_NV12       : return GMM_FORMAT_NV12_TYPE;
        case Media_Format_NV21       : return GMM_FORMAT_NV21_TYPE;
        case Media_Format_YUY2       : return GMM_FORMAT_YUY2;
        case Media_Format_YVYU       : return GMM_FORMAT_YVYU;
        case Media_Format_UYVY       : return GMM_FORMAT_UYVY;
        case Media_Format_VYUY       : return GMM_FORMAT_VYUY;
        case Media_Format_YV12       : return GMM_FORMAT_YV12_TYPE;
        case Media_Format_IYUV       : return GMM_FORMAT_IYUV_TYPE;
        case Media_Format_I420       : return GMM_FORMAT_I420_TYPE;
        case Media_Format_444P       : return GMM_FORMAT_MFX_JPEG_YUV444_TYPE;
        case Media_Format_422H       : return GMM_FORMAT_MFX_JPEG_YUV422H_TYPE;
        case Media_Format_411P       : return GMM_FORMAT_MFX_JPEG_YUV411_TYPE;
        case Media_Format_422V       : return GMM_FORMAT_MFX_JPEG_YUV422V_TYPE;
        case Media_Format_IMC3       : return GMM_FORMAT_IMC3_TYPE;
        case Media_Format_400P       : return GMM_FORMAT_GENERIC_8BIT;
        case Media_Format_Buffer     : return GMM_FORMAT_RENDER_8BIT;
        case Media_Format_P010       : return GMM_FORMAT_P010_TYPE;
        case Media_Format_R10G10B10A2: return GMM_FORMAT_R10G10B10A2_UNORM_TYPE;
        case Media_Format_B10G10R10A2: return GMM_FORMAT_B10G10R10A2_UNORM_TYPE;
        case Media_Format_P016       : return GMM_FORMAT_P016_TYPE;
        case Media_Format_Y210       : return GMM_FORMAT_Y210_TYPE;
        case Media_Format_Y216       : return GMM_FORMAT_Y216_TYPE;
        case Media_Format_AYUV       : return GMM_FORMAT_AYUV_TYPE;
        case Media_Format_Y410       : return GMM_FORMAT_Y410_TYPE;
        case Media_Format_Y416       : return GMM_FORMAT_Y416_TYPE;
        case Media_Format_Y8         : return GMM_FORMAT_MEDIA_Y8_UNORM;
        case Media_Format_Y16S       : return GMM_FORMAT_MEDIA_Y16_SNORM;
        case Media_Format_Y16U       : return GMM_FORMAT_MEDIA_Y16_UNORM;
        case Media_Format_A16R16G16B16: return GMM_FORMAT_B16G16R16A16_UNORM;
        case Media_Format_A16B16G16R16: return GMM_FORMAT_R16G16B16A16_UNORM;
        default                      : return GMM_FORMAT_INVALID;
    }
}

extern template class MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>;

static bool tglLPRegistered = MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>::
    RegisterCaps<MediaLibvaCapsG12>((uint32_t)IGFX_TIGERLAKE_LP);
