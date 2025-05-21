/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file     media_libva_caps_mtl_base.cpp
//! \brief    This file implements the C++ class/interface for MTL+ media capbilities.
//!

#include "media_libva.h"
#include "media_libva_vp.h"
#include "media_libva_caps_mtl_base.h"
#include "media_libva_caps_factory.h"
#include "media_libva_caps_cp_interface.h"
#include "mos_bufmgr_priv.h"
#include "codec_def_encode_av1.h"
#include "codec_def_common.h"
#include "media_ddi_decode_const.h"
#include "media_ddi_decode_const_xe_m_plus.h"
#include "media_ddi_encode_const.h"
#include "drm_fourcc.h"

const VAImageFormat m_supportedImageformatsXe_Lpm_Plus_Base[] =
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
    {VA_FOURCC_AYUV,           VA_LSB_FIRST,   32, 0,0,0,0,0},
#if VA_CHECK_VERSION(1, 13, 0)
    {VA_FOURCC_XYUV,           VA_LSB_FIRST,   32, 0,0,0,0,0},
#endif
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
    {VA_FOURCC_P010,           VA_LSB_FIRST,   24, 0,0,0,0,0},
    {VA_FOURCC_P012,           VA_LSB_FIRST,   24, 0,0,0,0,0},
    {VA_FOURCC_P016,           VA_LSB_FIRST,   24, 0,0,0,0,0},
    {VA_FOURCC_Y210,           VA_LSB_FIRST,   32, 0,0,0,0,0},
#if VA_CHECK_VERSION(1, 9, 0)
    {VA_FOURCC_Y212,           VA_LSB_FIRST,   32, 0,0,0,0,0},
#endif
    {VA_FOURCC_Y216,           VA_LSB_FIRST,   32, 0,0,0,0,0},
    {VA_FOURCC_Y410,           VA_LSB_FIRST,   32, 0,0,0,0,0},
#if VA_CHECK_VERSION(1, 9, 0)
    {VA_FOURCC_Y412,           VA_LSB_FIRST,   64, 0,0,0,0,0},
#endif
    {VA_FOURCC_Y416,           VA_LSB_FIRST,   64, 0,0,0,0,0},
    {VA_FOURCC_RGBP,           VA_LSB_FIRST,   24, 24,0,0,0,0},
    {VA_FOURCC_BGRP,           VA_LSB_FIRST,   24, 24,0,0,0,0},
};

#define ENCODE_DP_HEVC_NUM_MAX_VME_L0_REF_G12 4
#define ENCODE_DP_HEVC_NUM_MAX_VME_L1_REF_G12 4

#ifndef VA_CENC_TYPE_NONE
#define VA_CENC_TYPE_NONE 0x00000000
#endif

// will remove when mtl open source
#define INTEL_PRELIM_ID_FLAG         (1ULL << 55)

#define intel_prelim_fourcc_mod_code(val) \
    (fourcc_mod_code(INTEL, (val)) | INTEL_PRELIM_ID_FLAG)

/* this definition is to avoid duplicate drm_fourcc.h this file is updated seldom */
#ifndef I915_FORMAT_MOD_4_TILED_MTL_MC_CCS
#define I915_FORMAT_MOD_4_TILED_MTL_MC_CCS    fourcc_mod_code(INTEL, 14)
#endif
#ifndef I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC
#define I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC    fourcc_mod_code(INTEL, 15)
#endif

extern template class MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>;

VAStatus MediaLibvaCapsMtlBase::LoadHevcEncProfileEntrypoints()
{
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCapsMtlBase::LoadAvcEncProfileEntrypoints()
{
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCapsMtlBase::LoadAvcEncLpProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#if defined(_AVC_ENCODE_VDENC_SUPPORTED)
    AttribMap *attributeList = nullptr;
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeAVCVdenc))
    {
        status = CreateEncAttributes(VAProfileH264Main, VAEntrypointEncSliceLP, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");

        VAProfile profile[3] = {
            VAProfileH264Main,
            VAProfileH264High,
            VAProfileH264ConstrainedBaseline};

        uint32_t encRcMode[] =
        {
            VA_RC_CQP, VA_RC_CBR, VA_RC_VBR,
            VA_RC_CBR | VA_RC_MB, VA_RC_VBR | VA_RC_MB,
            VA_RC_ICQ, VA_RC_QVBR
#if VA_CHECK_VERSION(1, 10, 0)
            , VA_RC_TCBRC
#endif
        };

        int32_t numModes = MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels) ? sizeof(encRcMode)/sizeof(encRcMode[0]) : 1;

        for (int32_t profileIdx = 0; profileIdx < sizeof(profile) / sizeof(profile[0]); profileIdx++)
        {
            uint32_t configStartIdx = m_encConfigs.size();
            for (int32_t modeIdx = 0; modeIdx < numModes; modeIdx++)
            {
                AddEncConfig(encRcMode[modeIdx]);
            }
            AddProfileEntry(profile[profileIdx], VAEntrypointEncSliceLP, attributeList,
                    configStartIdx, m_encConfigs.size() - configStartIdx);
        }
    }
#endif // defined(_AVC_ENCODE_VDENC_SUPPORTED)

    return status;
}

VAStatus MediaLibvaCapsMtlBase::LoadAv1EncProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _AV1_ENCODE_VDENC_SUPPORTED
    AttribMap *attributeList = nullptr;
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeAV1Vdenc)||
        MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeAV1Vdenc10bit420))
    {
        status = CreateEncAttributes(VAProfileAV1Profile0, VAEntrypointEncSliceLP, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");
        (*attributeList)[VAConfigAttribEncDynamicScaling] = 0;
        (*attributeList)[VAConfigAttribEncTileSupport]    = 1;
        (*attributeList)[VAConfigAttribEncDirtyRect]      = VA_ATTRIB_NOT_SUPPORTED;
        (*attributeList)[VAConfigAttribEncMaxRefFrames]   = CODEC_AV1_NUM_REFL0P_FRAMES | CODEC_AV1_NUM_REFL1B_FRAMES<<16;

        VAConfigAttrib attrib;
        attrib.type = (VAConfigAttribType) VAConfigAttribEncAV1;
        VAConfigAttribValEncAV1 attribValAV1Tools;
        memset(&attribValAV1Tools, 0, sizeof(attribValAV1Tools));
        attribValAV1Tools.bits.support_cdef_channel_strength = 1;

        attrib.value = attribValAV1Tools.value;
        (*attributeList)[attrib.type] = attrib.value;

        attrib.type = (VAConfigAttribType) VAConfigAttribEncAV1Ext1;
        VAConfigAttribValEncAV1Ext1 attribValAV1ToolsExt1;
        memset(&attribValAV1ToolsExt1, 0, sizeof(attribValAV1ToolsExt1));
        attribValAV1ToolsExt1.bits.interpolation_filter   = 31;
        attribValAV1ToolsExt1.bits.min_segid_block_size_accepted = 32;
        attribValAV1ToolsExt1.bits.segment_feature_support = 1;

        attrib.value = attribValAV1ToolsExt1.value;
        (*attributeList)[attrib.type] = attrib.value;

        attrib.type = (VAConfigAttribType) VAConfigAttribEncAV1Ext2;
        VAConfigAttribValEncAV1Ext2 attribValAV1ToolsExt2;
        memset(&attribValAV1ToolsExt2, 0, sizeof(attribValAV1ToolsExt2));
        attribValAV1ToolsExt2.bits.tile_size_bytes_minus1 = 3;
        attribValAV1ToolsExt2.bits.obu_size_bytes_minus1  = 3;
        attribValAV1ToolsExt2.bits.max_tile_num_minus1    = 511;
        attribValAV1ToolsExt2.bits.tx_mode_support        = 4;

        attrib.value = attribValAV1ToolsExt2.value;
        (*attributeList)[attrib.type] = attrib.value;
    }

    if(MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeAV1Vdenc)||
        MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeAV1Vdenc10bit420))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        AddEncConfig(VA_RC_CBR);
        AddEncConfig(VA_RC_VBR);
        AddProfileEntry(VAProfileAV1Profile0, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }
#endif
    return status;
}


VAStatus MediaLibvaCapsMtlBase::CheckDecodeResolution(
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
        case CODECHAL_DECODE_MODE_JPEG:
            maxWidth = m_decJpegMaxWidth;
            maxHeight = m_decJpegMaxHeight;
            break;
        case CODECHAL_DECODE_MODE_AVCVLD:
            maxWidth = m_decAvcMaxWidth;
            maxHeight = m_decAvcMaxHeight;
            break;
        case CODECHAL_DECODE_MODE_HEVCVLD:
            maxWidth = m_decHevcMax16kWidth;
            maxHeight = m_decHevcMax16kHeight;
            break;
        case CODECHAL_DECODE_MODE_VP9VLD:
        case CODECHAL_DECODE_MODE_AV1VLD:
            maxWidth = m_decVp9Max16kWidth;
            maxHeight = m_decVp9Max16kHeight;
            break;
        default:
            maxWidth = m_decDefaultMaxWidth;
            maxHeight = m_decDefaultMaxHeight;
            break;
    }

    if (width > maxWidth || height > maxHeight)
    {
        return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
    }
    else
    {
        return VA_STATUS_SUCCESS;
    }
}

CODECHAL_MODE MediaLibvaCapsMtlBase::GetDecodeCodecMode(VAProfile profile)
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
        case VAProfileHEVCSccMain444_10:
            return CODECHAL_DECODE_MODE_HEVCVLD;
        case VAProfileAV1Profile0:
        case VAProfileAV1Profile1:
            return CODECHAL_DECODE_MODE_AV1VLD;
        default:
            DDI_ASSERTMESSAGE("Invalid Decode Mode");
            return CODECHAL_UNSUPPORTED_MODE;
    }
}

std::string MediaLibvaCapsMtlBase::GetDecodeCodecKey(VAProfile profile)
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
        case VAProfileHEVCSccMain444_10:
            return DECODE_ID_HEVC_REXT;
        case VAProfileAV1Profile0:
        case VAProfileAV1Profile1:
            return DECODE_ID_AV1;
        default:
            DDI_ASSERTMESSAGE("Invalid Decode Mode");
            return DECODE_ID_NONE;
    }
}


std::string MediaLibvaCapsMtlBase::GetEncodeCodecKey(
    VAProfile profile,
    VAEntrypoint entrypoint,
    uint32_t feiFunction)
{
    switch (profile)
    {
        case VAProfileAV1Profile0:
            return ENCODE_ID_AV1;
        case VAProfileHEVCMain:
        case VAProfileHEVCMain10:
        case VAProfileHEVCMain444:
        case VAProfileHEVCMain444_10:
        case VAProfileHEVCSccMain:
        case VAProfileHEVCSccMain10:
        case VAProfileHEVCSccMain444:
        case VAProfileHEVCSccMain444_10:
            return ENCODE_ID_HEVC;
        case VAProfileVP9Profile0:
        case VAProfileVP9Profile1:
        case VAProfileVP9Profile2:
        case VAProfileVP9Profile3:
            return ENCODE_ID_VP9;
        default:
            return MediaLibvaCaps::GetEncodeCodecKey(profile, entrypoint, feiFunction);
    }
}

CODECHAL_MODE MediaLibvaCapsMtlBase::GetEncodeCodecMode(
    VAProfile profile,
    VAEntrypoint entrypoint)
{
    switch (profile)
    {
        case VAProfileAV1Profile0:
            return CODECHAL_ENCODE_MODE_AV1;
        case VAProfileHEVCMain:
        case VAProfileHEVCMain10:
        case VAProfileHEVCMain444:
        case VAProfileHEVCMain444_10:
        case VAProfileHEVCSccMain:
        case VAProfileHEVCSccMain10:
        case VAProfileHEVCSccMain444:
        case VAProfileHEVCSccMain444_10:
            return CODECHAL_ENCODE_MODE_HEVC;
        case VAProfileVP9Profile0:
        case VAProfileVP9Profile1:
        case VAProfileVP9Profile2:
        case VAProfileVP9Profile3:
            return CODECHAL_ENCODE_MODE_VP9;
        default:
            return MediaLibvaCaps::GetEncodeCodecMode(profile, entrypoint);
    }
}

VAStatus MediaLibvaCapsMtlBase::CheckEncodeResolution(
    VAProfile profile,
    uint32_t width,
    uint32_t height)
{
    switch (profile)
    {
        case VAProfileAV1Profile0:
            if ((width > CODEC_8K_MAX_PIC_WIDTH) ||
                (width < CODEC_128_MIN_PIC_WIDTH) ||
                (height > CODEC_8K_MAX_PIC_HEIGHT) ||
                (height < CODEC_96_MIN_PIC_HEIGHT))
            {
                 return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
            }
            break;
        case VAProfileHEVCMain:
        case VAProfileHEVCMain10:
        case VAProfileHEVCMain444:
        case VAProfileHEVCMain444_10:
        case VAProfileHEVCSccMain:
        case VAProfileHEVCSccMain10:
        case VAProfileHEVCSccMain444:
        case VAProfileHEVCSccMain444_10:
            if (width > CODEC_16K_MAX_PIC_WIDTH
                || width < CODEC_128_MIN_PIC_WIDTH
                || height > CODEC_12K_MAX_PIC_HEIGHT
                || height < CODEC_128_MIN_PIC_HEIGHT)
            {
                return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
            }
            break;
        case VAProfileVP9Profile0:
        case VAProfileVP9Profile1:
        case VAProfileVP9Profile2:
        case VAProfileVP9Profile3:
            if ((width > CODEC_8K_MAX_PIC_WIDTH) ||
                (width < CODEC_128_MIN_PIC_WIDTH) ||
                (height > CODEC_8K_MAX_PIC_HEIGHT) ||
                (height < CODEC_96_MIN_PIC_HEIGHT))
            {
                return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
            }
            break;
        default:
            return MediaLibvaCaps::CheckEncodeResolution(profile, width, height);
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCapsMtlBase::CheckEncRTFormat(
    VAProfile profile,
    VAEntrypoint entrypoint,
    VAConfigAttrib* attrib)
{
    DDI_CHK_NULL(attrib, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    EncodeFormat format = Others;
    attrib->type        = VAConfigAttribRTFormat;

    switch (profile)
    {
        case VAProfileAV1Profile0:
            format = AV1;
            attrib->value = VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP;
            break;
        default:
            return MediaLibvaCaps::CheckEncRTFormat(profile, entrypoint, attrib);;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCapsMtlBase::QueryImageFormats(VAImageFormat *formatList, int32_t *numFormats)
{
    DDI_CHK_NULL(formatList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(numFormats, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    int32_t num = 0;
    uint32_t maxNum = GetImageFormatsMaxNum();

    memset(formatList, 0,  sizeof(m_supportedImageformatsXe_Lpm_Plus_Base));
    for (uint32_t idx = 0; idx < maxNum; idx++)
    {
        formatList[num].fourcc           = m_supportedImageformatsXe_Lpm_Plus_Base[idx].fourcc;
        formatList[num].byte_order       = m_supportedImageformatsXe_Lpm_Plus_Base[idx].byte_order;
        formatList[num].bits_per_pixel   = m_supportedImageformatsXe_Lpm_Plus_Base[idx].bits_per_pixel;
        formatList[num].depth            = m_supportedImageformatsXe_Lpm_Plus_Base[idx].depth;
        formatList[num].red_mask         = m_supportedImageformatsXe_Lpm_Plus_Base[idx].red_mask;
        formatList[num].green_mask       = m_supportedImageformatsXe_Lpm_Plus_Base[idx].green_mask;
        formatList[num].blue_mask        = m_supportedImageformatsXe_Lpm_Plus_Base[idx].blue_mask;
        formatList[num].alpha_mask       = m_supportedImageformatsXe_Lpm_Plus_Base[idx].alpha_mask;
        num++;
    }
    *numFormats = num;

    return VA_STATUS_SUCCESS;
}

uint32_t MediaLibvaCapsMtlBase::GetImageFormatsMaxNum()
{
    return sizeof(m_supportedImageformatsXe_Lpm_Plus_Base)/sizeof(m_supportedImageformatsXe_Lpm_Plus_Base[0]);
}

VAStatus MediaLibvaCapsMtlBase::PopulateColorMaskInfo(VAImageFormat *vaImgFmt)
{
    uint32_t maxNum = GetImageFormatsMaxNum();

    DDI_CHK_NULL(vaImgFmt, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    for (int32_t idx = 0; idx < maxNum; idx++)
    {
        if (m_supportedImageformatsXe_Lpm_Plus_Base[idx].fourcc == vaImgFmt->fourcc)
        {
            vaImgFmt->red_mask   = m_supportedImageformatsXe_Lpm_Plus_Base[idx].red_mask;
            vaImgFmt->green_mask = m_supportedImageformatsXe_Lpm_Plus_Base[idx].green_mask;
            vaImgFmt->blue_mask  = m_supportedImageformatsXe_Lpm_Plus_Base[idx].blue_mask;
            vaImgFmt->alpha_mask = m_supportedImageformatsXe_Lpm_Plus_Base[idx].alpha_mask;

            return VA_STATUS_SUCCESS;
        }
    }

    return VA_STATUS_ERROR_INVALID_IMAGE_FORMAT;
}

bool MediaLibvaCapsMtlBase::IsImageSupported(uint32_t fourcc)
{
    uint32_t maxNum = GetImageFormatsMaxNum();
    for (int32_t idx = 0; idx < maxNum; idx++)
    {
        if (m_supportedImageformatsXe_Lpm_Plus_Base[idx].fourcc == fourcc)
        {
            return true;
        }
    }

    return false;
}

VAStatus MediaLibvaCapsMtlBase::QueryAVCROIMaxNum(uint32_t rcMode, bool isVdenc, uint32_t *maxNum, bool *isRoiInDeltaQP)
{
    DDI_CHK_NULL(maxNum, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(isRoiInDeltaQP, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    if(isVdenc)
    {
        *maxNum = ENCODE_VDENC_AVC_MAX_ROI_NUMBER_ADV;
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

VAStatus MediaLibvaCapsMtlBase::LoadProfileEntrypoints()
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
    status = LoadAv1DecProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
    status = LoadAv1EncProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
    status = LoadNoneProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
    status = m_CapsCp->LoadCpProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize CP Caps!");

    return status;
}

VAStatus MediaLibvaCapsMtlBase::LoadHevcDecProfileEntrypoints()
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

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain8bit444SCC))
    {
        LoadDecProfileEntrypoints(VAProfileHEVCSccMain444);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain10bit444SCC))
    {
        LoadDecProfileEntrypoints(VAProfileHEVCSccMain444_10);
    }

#endif
    return status;
}

VAStatus MediaLibvaCapsMtlBase::GetPlatformSpecificAttrib(VAProfile profile,
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
            // just VAConfigAttribEncMaxRefFrames of HEVC VME is different with platforms
            if (entrypoint == VAEntrypointEncSlice && IsHevcProfile(profile))
            {
                *value = ENCODE_DP_HEVC_NUM_MAX_VME_L0_REF_G12 | (ENCODE_DP_HEVC_NUM_MAX_VME_L1_REF_G12 << 16);
            }
            else
            {
                status = VA_STATUS_ERROR_INVALID_PARAMETER;
            }
            break;
        }
        case VAConfigAttribDecProcessing:
        {
#ifdef _DECODE_PROCESSING_SUPPORTED
            if ((IsAvcProfile(profile) || IsHevcProfile(profile) || IsJpegProfile(profile) || IsVp9Profile(profile))
                && !(MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrDisableVDBox2SFC)))
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
            if(IsAvcProfile(profile) || (entrypoint == VAEntrypointEncSliceLP && IsHevcProfile(profile)))
            {
                *value = VA_ENC_INTRA_REFRESH_ROLLING_COLUMN | VA_ENC_INTRA_REFRESH_ROLLING_ROW;
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
            *value = 1;
            break;
        }
        case VAConfigAttribEncMaxSlices:
        {
            if (entrypoint == VAEntrypointEncSlice && IsHevcProfile(profile))
            {
                *value = CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6;
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
            else if(IsHevcProfile(profile))
            {
                *value = CODEC_16K_MAX_PIC_WIDTH;
            }
            else if(IsVp9Profile(profile))
            {
                *value = CODEC_8K_MAX_PIC_WIDTH;
            }
            else if(IsAvcProfile(profile))
            {
                *value = CODEC_4K_MAX_PIC_WIDTH;
            }
            else if(profile == VAProfileAV1Profile0)
            {
                *value = CODEC_8K_MAX_PIC_WIDTH;
            }
            else if(profile == IsMpeg2Profile(profile))
            {
                *value = CODEC_2K_MAX_PIC_WIDTH;
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
            else if(IsHevcProfile(profile))
            {
                *value = CODEC_12K_MAX_PIC_HEIGHT;
            }
            else if(IsVp9Profile(profile))
            {
                *value = CODEC_8K_MAX_PIC_HEIGHT;
            }
            else if(IsAvcProfile(profile))
            {
                *value = CODEC_4K_MAX_PIC_HEIGHT;
            }
            else if(profile == VAProfileAV1Profile0)
            {
                *value = CODEC_8K_MAX_PIC_HEIGHT;
            }
            else if(profile == IsMpeg2Profile(profile))
            {
                *value = CODEC_2K_MAX_PIC_HEIGHT;
            }
            else
            {
                *value = CODEC_MAX_PIC_HEIGHT;
            }
            break;
        }
        case VAConfigAttribQPBlockSize:
        {
            if(IsAvcProfile(profile))
            {
                *value = CODECHAL_MACROBLOCK_WIDTH;
            }
            else
            {
                status = VA_STATUS_ERROR_INVALID_PARAMETER;
            }
            break;
        }
        case VAConfigAttribPredictionDirection:
        {
            *value = VA_PREDICTION_DIRECTION_PREVIOUS | VA_PREDICTION_DIRECTION_FUTURE | VA_PREDICTION_DIRECTION_BI_NOT_EMPTY;
            break;
        }
#if VA_CHECK_VERSION(1, 12, 0)
        case VAConfigAttribEncHEVCFeatures:
        {
            if (entrypoint == VAEntrypointEncSliceLP && IsHevcProfile(profile))
            {
                VAConfigAttribValEncHEVCFeatures hevcFeatures = {0};
                hevcFeatures.bits.separate_colour_planes = VA_FEATURE_NOT_SUPPORTED;
                hevcFeatures.bits.scaling_lists = VA_FEATURE_SUPPORTED;
                hevcFeatures.bits.amp = VA_FEATURE_REQUIRED;
                hevcFeatures.bits.sao = VA_FEATURE_SUPPORTED;
                hevcFeatures.bits.pcm = VA_FEATURE_NOT_SUPPORTED;
                hevcFeatures.bits.temporal_mvp = VA_FEATURE_SUPPORTED;
                hevcFeatures.bits.strong_intra_smoothing = VA_FEATURE_NOT_SUPPORTED;
                hevcFeatures.bits.dependent_slices = VA_FEATURE_NOT_SUPPORTED;
                hevcFeatures.bits.sign_data_hiding = VA_FEATURE_NOT_SUPPORTED;
                hevcFeatures.bits.constrained_intra_pred = VA_FEATURE_NOT_SUPPORTED;
                hevcFeatures.bits.transform_skip = VA_FEATURE_SUPPORTED;
                hevcFeatures.bits.cu_qp_delta = VA_FEATURE_REQUIRED;
                hevcFeatures.bits.weighted_prediction = VA_FEATURE_SUPPORTED;
                hevcFeatures.bits.transquant_bypass = VA_FEATURE_NOT_SUPPORTED;
                hevcFeatures.bits.deblocking_filter_disable = VA_FEATURE_NOT_SUPPORTED;
                *value = hevcFeatures.value;
            }
            break;
        }
        case VAConfigAttribEncHEVCBlockSizes:
        {
            if (entrypoint == VAEntrypointEncSliceLP && IsHevcProfile(profile))
            {
                VAConfigAttribValEncHEVCBlockSizes hevcBlockSize = {0};
                hevcBlockSize.bits.log2_max_coding_tree_block_size_minus3     = 3;
                hevcBlockSize.bits.log2_min_coding_tree_block_size_minus3     = 3;
                hevcBlockSize.bits.log2_min_luma_coding_block_size_minus3     = 0;
                hevcBlockSize.bits.log2_max_luma_transform_block_size_minus2  = 3;
                hevcBlockSize.bits.log2_min_luma_transform_block_size_minus2  = 0;
                hevcBlockSize.bits.max_max_transform_hierarchy_depth_inter    = 2;
                hevcBlockSize.bits.min_max_transform_hierarchy_depth_inter    = 0;
                hevcBlockSize.bits.max_max_transform_hierarchy_depth_intra    = 2;
                hevcBlockSize.bits.min_max_transform_hierarchy_depth_intra    = 0;
                hevcBlockSize.bits.log2_max_pcm_coding_block_size_minus3      = 0;
                hevcBlockSize.bits.log2_min_pcm_coding_block_size_minus3      = 0;
                *value = hevcBlockSize.value;
            }
            break;
        }
#endif
        default:
            status = VA_STATUS_ERROR_INVALID_PARAMETER;
            break;
    }
    return status;
}

VAStatus MediaLibvaCapsMtlBase::LoadAv1DecProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#if _AV1_DECODE_SUPPORTED
    AttribMap *attributeList = nullptr;
    if ((MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelAV1VLDDecoding8bit420)
        ||MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelAV1VLDDecoding10bit420)))
    {
        status = CreateDecAttributes((VAProfile) VAProfileAV1Profile0, VAEntrypointVLD, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");

        uint32_t configStartIdx = m_decConfigs.size();
        for (int32_t i = 0; i < 2; i++)
        {
            AddDecConfig(m_decSliceMode[i], VA_CENC_TYPE_NONE, VA_DEC_PROCESSING_NONE);
            if (m_isEntryptSupported)
            {
                uint32_t encrytTypes[DDI_CP_ENCRYPT_TYPES_NUM];

                int32_t numTypes = m_CapsCp->GetEncryptionTypes((VAProfile) VAProfileAV1Profile0,
                        encrytTypes, DDI_CP_ENCRYPT_TYPES_NUM);

                if (numTypes > 0)
                {
                    for (int32_t l = 0; l < numTypes; l++)
                    {
                        AddDecConfig(m_decSliceMode[i], encrytTypes[l],
                                VA_DEC_PROCESSING_NONE);
                    }
                }
            }
        }

        AddProfileEntry((VAProfile) VAProfileAV1Profile0, VAEntrypointVLD, attributeList,
                configStartIdx, m_decConfigs.size() - configStartIdx);
    }

#endif
    return status;
}

VAStatus MediaLibvaCapsMtlBase::CreateDecAttributes(
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
    else if(profile == VAProfileHEVCMain)
    {
        attrib.value = VA_RT_FORMAT_YUV420;
    }
    else if(profile == VAProfileHEVCMain10)
    {
        attrib.value = VA_RT_FORMAT_YUV420
            | VA_RT_FORMAT_YUV420_10BPP;
    }
    else if(profile == VAProfileHEVCMain12)
    {
        attrib.value = VA_RT_FORMAT_YUV420
            | VA_RT_FORMAT_YUV420_10BPP
            | VA_RT_FORMAT_YUV420_12
            | VA_RT_FORMAT_YUV400;
    }
    else if(profile == VAProfileHEVCMain422_10)
    {
        attrib.value = VA_RT_FORMAT_YUV420
            | VA_RT_FORMAT_YUV420_10BPP
            | VA_RT_FORMAT_YUV422
            | VA_RT_FORMAT_YUV422_10
            | VA_RT_FORMAT_YUV400;
    }
    else if(profile == VAProfileHEVCMain422_12)
    {
        attrib.value = VA_RT_FORMAT_YUV420
            | VA_RT_FORMAT_YUV420_10BPP
            | VA_RT_FORMAT_YUV420_12
            | VA_RT_FORMAT_YUV422
            | VA_RT_FORMAT_YUV422_10
            | VA_RT_FORMAT_YUV422_12
            | VA_RT_FORMAT_YUV400;
    }
    else if(profile == VAProfileHEVCMain444 || profile == VAProfileHEVCSccMain444)
    {
        attrib.value = VA_RT_FORMAT_YUV420
            | VA_RT_FORMAT_YUV422
            | VA_RT_FORMAT_YUV444
            | VA_RT_FORMAT_YUV400;
    }
    else if(profile == VAProfileHEVCMain444_10 || profile == VAProfileHEVCSccMain444_10)
    {
        attrib.value = VA_RT_FORMAT_YUV420
            | VA_RT_FORMAT_YUV420_10BPP
            | VA_RT_FORMAT_YUV422
            | VA_RT_FORMAT_YUV422_10
            | VA_RT_FORMAT_YUV444
            | VA_RT_FORMAT_YUV444_10
            | VA_RT_FORMAT_YUV400;
    }
    else if(profile == VAProfileHEVCMain444_12)
    {
        attrib.value = VA_RT_FORMAT_YUV420
            | VA_RT_FORMAT_YUV420_10BPP
            | VA_RT_FORMAT_YUV420_12
            | VA_RT_FORMAT_YUV422
            | VA_RT_FORMAT_YUV422_10
            | VA_RT_FORMAT_YUV422_12
            | VA_RT_FORMAT_YUV444
            | VA_RT_FORMAT_YUV444_10
            | VA_RT_FORMAT_YUV444_12
            | VA_RT_FORMAT_YUV400;
    }
    else if(profile == VAProfileHEVCSccMain)
    {
        attrib.value = VA_RT_FORMAT_YUV420
            | VA_RT_FORMAT_YUV400;
    }
    else if(profile == VAProfileHEVCSccMain10)
    {
        attrib.value = VA_RT_FORMAT_YUV420
            | VA_RT_FORMAT_YUV420_10BPP
            | VA_RT_FORMAT_YUV400;
    }
    else if (profile == VAProfileVP9Profile0)
    {
        attrib.value = VA_RT_FORMAT_YUV420;
    }
    else if (profile == VAProfileVP9Profile1)
    {
        attrib.value = VA_RT_FORMAT_YUV422
            | VA_RT_FORMAT_YUV444;
    }
    else if (profile == VAProfileVP9Profile2)
    {
        attrib.value = VA_RT_FORMAT_YUV420_10BPP
            | VA_RT_FORMAT_YUV420_12;
    }
    else if (profile == VAProfileVP9Profile3)
    {
        attrib.value = VA_RT_FORMAT_YUV422_10
            | VA_RT_FORMAT_YUV444_10
            | VA_RT_FORMAT_YUV422_12
            | VA_RT_FORMAT_YUV444_12;
    }
    else if (profile == VAProfileAV1Profile0)
    {
        attrib.value = VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP;
    }
    else
    {
        attrib.value = VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_RGB32;
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
    else if (profile == VAProfileAV1Profile0)
    {
        attrib.value = 0;
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelAV1VLDDecoding8bit420)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelAV1VLDDecoding10bit420))
        {
            attrib.value |= VA_DEC_SLICE_MODE_NORMAL | VA_DEC_SLICE_MODE_BASE;
        }
        else
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
    if(IsVc1Profile(profile) || IsMpeg2Profile(profile))
    {
        attrib.value = CODEC_2K_MAX_PIC_WIDTH;
    }
    if(IsVp8Profile(profile))
    {
        attrib.value = CODEC_4K_MAX_PIC_WIDTH;
    }
    if(IsAvcProfile(profile))
    {
        attrib.value = CODEC_4K_MAX_PIC_WIDTH;
    }
    if(IsHevcProfile(profile) || IsVp9Profile(profile) || IsAV1Profile(profile))
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
    if(IsVc1Profile(profile) || IsMpeg2Profile(profile))
    {
        attrib.value = CODEC_2K_MAX_PIC_HEIGHT;
    }
    if(IsVp8Profile(profile))
    {
        attrib.value = CODEC_4K_MAX_PIC_HEIGHT;
    }
    if(IsAvcProfile(profile))
    {
        attrib.value = CODEC_4K_MAX_PIC_HEIGHT;
    }
    if(IsHevcProfile(profile) || IsVp9Profile(profile) || IsAV1Profile(profile))
    {
        attrib.value = CODEC_16K_MAX_PIC_HEIGHT;
    }
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribEncryption;

    attrib.value = VA_ATTRIB_NOT_SUPPORTED;
    if (m_isEntryptSupported)
    {
        uint32_t encryptTypes[DDI_CP_ENCRYPT_TYPES_NUM] = {0};
        int32_t numTypes =  m_CapsCp->GetEncryptionTypes(profile,
                encryptTypes, DDI_CP_ENCRYPT_TYPES_NUM);
        if (numTypes > 0)
        {
            attrib.value = 0;
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

#if VA_CHECK_VERSION(1, 11, 0)
    if(IsAV1Profile(profile) && MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrAV1VLDLSTDecoding))
    {
        attrib.type                             = VAConfigAttribDecAV1Features;
        VAConfigAttribValDecAV1Features feature = {0};
        feature.bits.lst_support                = true;
        attrib.value                            = feature.value;
        (*attribList)[attrib.type]              = attrib.value;
    }
#endif
    return status;
}

VAStatus MediaLibvaCapsMtlBase::LoadHevcEncLpProfileEntrypoints()
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
        (*attributeList)[VAConfigAttribRateControl] |= VA_RC_QVBR;
        (*attributeList)[VAConfigAttribMaxPictureWidth] = CODEC_16K_MAX_PIC_WIDTH;
        (*attributeList)[VAConfigAttribMaxPictureHeight] = CODEC_12K_MAX_PIC_HEIGHT;
        (*attributeList)[VAConfigAttribEncTileSupport] = 1;
        (*attributeList)[VAConfigAttribEncSliceStructure] = VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS |
                                                        VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE | VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS;
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
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain8bit420SCC))
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
        AddProfileEntry(VAProfileHEVCSccMain, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain10bit420SCC))
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
        AddProfileEntry(VAProfileHEVCSccMain10, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }

      if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain8bit444SCC))
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
        AddProfileEntry(VAProfileHEVCSccMain444, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain10bit444SCC))
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
        AddProfileEntry(VAProfileHEVCSccMain444_10, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }
#endif
    return status;
}

const VAConfigAttribValEncRateControlExt MediaLibvaCapsMtlBase::m_encVp9RateControlExt =
    {
        {CODECHAL_ENCODE_VP9_MAX_NUM_TEMPORAL_LAYERS - 1, 1, 0}
};

bool MediaLibvaCapsMtlBase::IsAV1Profile(VAProfile profile)
{
    return (profile == (VAProfile)VAProfileAV1Profile0 ||
            profile == (VAProfile)VAProfileAV1Profile1);
}

VAStatus MediaLibvaCapsMtlBase::LoadVp9EncProfileEntrypoints()
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
        (*attributeList)[VAConfigAttribMaxPictureWidth]   = CODEC_8K_MAX_PIC_WIDTH;
        (*attributeList)[VAConfigAttribMaxPictureHeight]  = CODEC_8K_MAX_PIC_HEIGHT;
        (*attributeList)[VAConfigAttribEncDynamicScaling] = 1;
        (*attributeList)[VAConfigAttribEncTileSupport]    = 1;
        (*attributeList)[VAConfigAttribEncRateControlExt] = m_encVp9RateControlExt.value;
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeVP9Vdenc) &&
        MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        AddEncConfig(VA_RC_CBR);
        AddEncConfig(VA_RC_VBR);
        AddEncConfig(VA_RC_ICQ);  // for VP9 CQL only
        AddProfileEntry(VAProfileVP9Profile0, VAEntrypointEncSliceLP, attributeList, configStartIdx, m_encConfigs.size() - configStartIdx);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeVP9Vdenc8bit444) &&
        MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        AddEncConfig(VA_RC_CBR);
        AddEncConfig(VA_RC_VBR);
        AddEncConfig(VA_RC_ICQ);  // for VP9 CQL only
        AddProfileEntry(VAProfileVP9Profile1, VAEntrypointEncSliceLP, attributeList, configStartIdx, m_encConfigs.size() - configStartIdx);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeVP9Vdenc10bit420) &&
        MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        AddEncConfig(VA_RC_CBR);
        AddEncConfig(VA_RC_VBR);
        AddEncConfig(VA_RC_ICQ);  // for VP9 CQL only
        AddProfileEntry(VAProfileVP9Profile2, VAEntrypointEncSliceLP, attributeList, configStartIdx, m_encConfigs.size() - configStartIdx);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeVP9Vdenc10bit444) &&
        MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        AddEncConfig(VA_RC_CBR);
        AddEncConfig(VA_RC_VBR);
        AddEncConfig(VA_RC_ICQ);  // for VP9 CQL only
        AddProfileEntry(VAProfileVP9Profile3, VAEntrypointEncSliceLP, attributeList, configStartIdx, m_encConfigs.size() - configStartIdx);
    }
#endif
    return status;
}

GMM_RESOURCE_FORMAT MediaLibvaCapsMtlBase::ConvertMediaFmtToGmmFmt(
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
        case Media_Format_R10G10B10X2: return GMM_FORMAT_R10G10B10A2_UNORM_TYPE;
        case Media_Format_B10G10R10X2: return GMM_FORMAT_B10G10R10A2_UNORM_TYPE;
        case Media_Format_P012       : return GMM_FORMAT_P016_TYPE;
        case Media_Format_P016       : return GMM_FORMAT_P016_TYPE;
        case Media_Format_Y210       : return GMM_FORMAT_Y210_TYPE;
#if VA_CHECK_VERSION(1, 9, 0)
        case Media_Format_Y212       : return GMM_FORMAT_Y212_TYPE;
#endif
        case Media_Format_Y216       : return GMM_FORMAT_Y216_TYPE;
        case Media_Format_AYUV       : return GMM_FORMAT_AYUV_TYPE;
#if VA_CHECK_VERSION(1, 13, 0)
        case Media_Format_XYUV       : return GMM_FORMAT_AYUV_TYPE;
#endif
        case Media_Format_Y410       : return GMM_FORMAT_Y410_TYPE;
#if VA_CHECK_VERSION(1, 9, 0)
        case Media_Format_Y412       : return GMM_FORMAT_Y412_TYPE;
#endif
        case Media_Format_Y416       : return GMM_FORMAT_Y416_TYPE;
        case Media_Format_Y8         : return GMM_FORMAT_MEDIA_Y8_UNORM;
        case Media_Format_Y16S       : return GMM_FORMAT_MEDIA_Y16_SNORM;
        case Media_Format_Y16U       : return GMM_FORMAT_MEDIA_Y16_UNORM;
        case Media_Format_A16R16G16B16: return GMM_FORMAT_B16G16R16A16_UNORM;
        case Media_Format_A16B16G16R16: return GMM_FORMAT_R16G16B16A16_UNORM;
        default                      : return GMM_FORMAT_INVALID;
    }
}

VAStatus MediaLibvaCapsMtlBase::AddEncSurfaceAttributes(
    VAProfile        profile,
    VAEntrypoint     entrypoint,
    VASurfaceAttrib *attribList,
    uint32_t &       numAttribs)
{
    DDI_CHK_NULL(attribList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (entrypoint == VAEntrypointEncSlice || entrypoint == VAEntrypointEncSliceLP || entrypoint == VAEntrypointEncPicture || entrypoint == VAEntrypointFEI)
    {
        if (profile == VAProfileHEVCMain10 || profile == VAProfileHEVCSccMain10 || profile == VAProfileVP9Profile2)
        {
            attribList[numAttribs].type          = VASurfaceAttribPixelFormat;
            attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
            attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribList[numAttribs].value.value.i = VA_FOURCC('P', '0', '1', '0');
            numAttribs++;
        }
        else if (profile == VAProfileHEVCMain444 || profile == VAProfileHEVCSccMain444)
        {
            attribList[numAttribs].type          = VASurfaceAttribPixelFormat;
            attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
            attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribList[numAttribs].value.value.i = VA_FOURCC_AYUV;
            numAttribs++;

#if VA_CHECK_VERSION(1, 13, 0)
            attribList[numAttribs].type          = VASurfaceAttribPixelFormat;
            attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
            attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribList[numAttribs].value.value.i = VA_FOURCC_XYUV;
            numAttribs++;
#endif
        }
        else if (profile == VAProfileHEVCMain444_10 || profile == VAProfileHEVCSccMain444_10)
        {
            attribList[numAttribs].type          = VASurfaceAttribPixelFormat;
            attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
            attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribList[numAttribs].value.value.i = VA_FOURCC_Y410;
            numAttribs++;
        }
        else if (profile == VAProfileHEVCMain422_10)
        {
            attribList[numAttribs].type          = VASurfaceAttribPixelFormat;
            attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
            attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribList[numAttribs].value.value.i = VA_FOURCC_YUY2;
            numAttribs++;

            attribList[numAttribs].type          = VASurfaceAttribPixelFormat;
            attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
            attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribList[numAttribs].value.value.i = VA_FOURCC_Y210;
            numAttribs++;
        }
        else if (profile == VAProfileJPEGBaseline)
        {
            for (int32_t j = 0; j < m_numJpegEncSurfaceAttr; j++)
            {
                attribList[numAttribs].type          = VASurfaceAttribPixelFormat;
                attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
                attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribList[numAttribs].value.value.i = m_jpegEncSurfaceAttr[j];
                numAttribs++;
            }
        }
        else if (profile == VAProfileVP9Profile1)
        {
            attribList[numAttribs].type          = VASurfaceAttribPixelFormat;
            attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
            attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribList[numAttribs].value.value.i = VA_FOURCC_AYUV;
            numAttribs++;

#if VA_CHECK_VERSION(1, 13, 0)
            attribList[numAttribs].type          = VASurfaceAttribPixelFormat;
            attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
            attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribList[numAttribs].value.value.i = VA_FOURCC_XYUV;
            numAttribs++;
#endif
        }
        else if (profile == VAProfileVP9Profile3)
        {
            attribList[numAttribs].type          = VASurfaceAttribPixelFormat;
            attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
            attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribList[numAttribs].value.value.i = VA_FOURCC_Y410;
            numAttribs++;

            attribList[numAttribs].type          = VASurfaceAttribPixelFormat;
            attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
            attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribList[numAttribs].value.value.i = VA_FOURCC('A', 'R', 'G', 'B');
            numAttribs++;

            attribList[numAttribs].type          = VASurfaceAttribPixelFormat;
            attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
            attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribList[numAttribs].value.value.i = VA_FOURCC('A', 'B', 'G', 'R');
            numAttribs++;
        }
        else
        {
            attribList[numAttribs].type          = VASurfaceAttribPixelFormat;
            attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
            attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribList[numAttribs].value.value.i = VA_FOURCC('N', 'V', '1', '2');
            numAttribs++;
        }
        
        if (profile == VAProfileAV1Profile0)
        {
            attribList[numAttribs].type          = VASurfaceAttribPixelFormat;
            attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
            attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribList[numAttribs].value.value.i = VA_FOURCC('P', '0', '1', '0');
            numAttribs++;
        }

        attribList[numAttribs].type          = VASurfaceAttribMaxWidth;
        attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
        attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE;
        attribList[numAttribs].value.value.i = CODEC_MAX_PIC_WIDTH;

        if (profile == VAProfileJPEGBaseline)
        {
            attribList[numAttribs].value.value.i = ENCODE_JPEG_MAX_PIC_WIDTH;
        }
        else if (IsHevcProfile(profile))
        {
            attribList[numAttribs].value.value.i = CODEC_16K_MAX_PIC_WIDTH;
        }
        else if (IsVp9Profile(profile) || profile == VAProfileAV1Profile0)
        {
            attribList[numAttribs].value.value.i = CODEC_8K_MAX_PIC_WIDTH;
        }
        if (IsAvcProfile(profile))
        {
            attribList[numAttribs].value.value.i = CODEC_4K_MAX_PIC_WIDTH;
        }
        numAttribs++;

        attribList[numAttribs].type          = VASurfaceAttribMaxHeight;
        attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
        attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE;
        attribList[numAttribs].value.value.i = CODEC_MAX_PIC_HEIGHT;
        if (profile == VAProfileJPEGBaseline)
        {
            attribList[numAttribs].value.value.i = ENCODE_JPEG_MAX_PIC_HEIGHT;
        }
        else if (IsHevcProfile(profile))
        {
            uint32_t heightValue = CODEC_12K_MAX_PIC_HEIGHT;
            GetPlatformSpecificAttrib(profile, entrypoint, VAConfigAttribMaxPictureHeight, &heightValue);
            attribList[numAttribs].value.value.i = (int32_t)heightValue;
        }
        else if (IsVp9Profile(profile) || profile == VAProfileAV1Profile0)
        {
            attribList[numAttribs].value.value.i = CODEC_8K_MAX_PIC_HEIGHT;
        }
        if (IsAvcProfile(profile))
        {
            attribList[numAttribs].value.value.i = CODEC_4K_MAX_PIC_HEIGHT;
        }
        numAttribs++;

        attribList[numAttribs].type          = VASurfaceAttribMinWidth;
        attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
        attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE;
        attribList[numAttribs].value.value.i = m_encMinWidth;
        if (IsHevcProfile(profile))
        {
            attribList[numAttribs].value.value.i = m_hevcVDEncMinWidth;
        }
        else if (IsVp9Profile(profile))
        {
            attribList[numAttribs].value.value.i = m_minVp9EncWidth;
        }
        else if (profile == VAProfileJPEGBaseline)
        {
            attribList[numAttribs].value.value.i = m_encJpegMinWidth;
        }
        else if (profile == VAProfileAV1Profile0)
        {
            attribList[numAttribs].value.value.i = m_minAv1EncWidth;
        }
        numAttribs++;

        attribList[numAttribs].type          = VASurfaceAttribMinHeight;
        attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
        attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE;
        attribList[numAttribs].value.value.i = m_encMinHeight;
        if (IsHevcProfile(profile))
        {
            attribList[numAttribs].value.value.i = m_hevcVDEncMinHeight;
        }
        else if (IsVp9Profile(profile))
        {
            attribList[numAttribs].value.value.i = m_minVp9EncHeight;
        }
        else if (profile == VAProfileJPEGBaseline)
        {
            attribList[numAttribs].value.value.i = m_encJpegMinHeight;
        }
        else if (profile == VAProfileAV1Profile0)
        {
            attribList[numAttribs].value.value.i = m_minAv1EncHeight;
        }
        numAttribs++;

        attribList[numAttribs].type          = VASurfaceAttribMemoryType;
        attribList[numAttribs].value.type    = VAGenericValueTypeInteger;
        attribList[numAttribs].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
        attribList[numAttribs].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_VA |
#if VA_CHECK_VERSION(1, 21, 0)
                                               VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 |
#endif
                                               VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2;
        numAttribs++;
    }

    return VA_STATUS_SUCCESS;
}
VAStatus MediaLibvaCapsMtlBase::QuerySurfaceAttributes(
    VAConfigID       configId,
    VASurfaceAttrib *attribList,
    uint32_t *       numAttribs)
{
    DDI_CHK_NULL(numAttribs, "Null num_attribs", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (attribList == nullptr)
    {
        *numAttribs = DDI_CODEC_GEN_MAX_SURFACE_ATTRIBUTES;
        return VA_STATUS_SUCCESS;
    }

    int32_t      profileTableIdx = -1;
    VAEntrypoint entrypoint;
    VAProfile    profile;
    VAStatus     status = GetProfileEntrypointFromConfigId(configId, &profile, &entrypoint, &profileTableIdx);
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

    uint32_t i = 0;

    if (entrypoint == VAEntrypointVideoProc) /* vpp */
    {
        attribs[i].type          = VASurfaceAttribPixelFormat;
        attribs[i].value.type    = VAGenericValueTypeInteger;
        attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
        attribs[i].value.value.i = VA_FOURCC('N', 'V', '1', '2');
        i++;

        attribs[i].type          = VASurfaceAttribMaxWidth;
        attribs[i].value.type    = VAGenericValueTypeInteger;
        attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
        attribs[i].value.value.i = VP_MAX_PIC_WIDTH;
        i++;

        attribs[i].type          = VASurfaceAttribMaxHeight;
        attribs[i].value.type    = VAGenericValueTypeInteger;
        attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
        attribs[i].value.value.i = VP_MAX_PIC_HEIGHT;
        i++;

        attribs[i].type          = VASurfaceAttribMinWidth;
        attribs[i].value.type    = VAGenericValueTypeInteger;
        attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
        attribs[i].value.value.i = VP_MIN_PIC_WIDTH;
        i++;

        attribs[i].type          = VASurfaceAttribMinHeight;
        attribs[i].value.type    = VAGenericValueTypeInteger;
        attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
        attribs[i].value.value.i = VP_MIN_PIC_HEIGHT;
        i++;

        for (int32_t j = 0; j < m_numVpSurfaceAttr; j++)
        {
            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = m_vpSurfaceAttr[j];
            i++;
        }

        attribs[i].type       = VASurfaceAttribMemoryType;
        attribs[i].value.type = VAGenericValueTypeInteger;
        attribs[i].flags      = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
#ifdef ANDROID
        attribs[i].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_VA |
                                   VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR |
                                   VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM |
                                   VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME |
                                   VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2 |
#if VA_CHECK_VERSION(1, 21, 0)
                                   VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 |
#endif
                                   VA_SURFACE_ATTRIB_MEM_TYPE_ANDROID_GRALLOC;
#else
        attribs[i].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_VA |
                                   VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR |
                                   VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM |
                                   VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME |
#if VA_CHECK_VERSION(1, 21, 0)
                                   VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 |
#endif
                                   VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2;
#endif
        i++;

        attribs[i].type          = VASurfaceAttribExternalBufferDescriptor;
        attribs[i].value.type    = VAGenericValueTypePointer;
        attribs[i].flags         = VA_SURFACE_ATTRIB_SETTABLE;
        attribs[i].value.value.p = nullptr; /* ignore */
        i++;
    }
    else if (entrypoint == VAEntrypointVLD) /* vld */
    {
        if (profile == VAProfileHEVCMain10 || profile == VAProfileVP9Profile2)
        {
            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_P010;
            i++;

            if (profile == VAProfileVP9Profile2)
            {
                attribs[i].type          = VASurfaceAttribPixelFormat;
                attribs[i].value.type    = VAGenericValueTypeInteger;
                attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_P012;
                i++;

                attribs[i].type          = VASurfaceAttribPixelFormat;
                attribs[i].value.type    = VAGenericValueTypeInteger;
                attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_P016;
                i++;
            }
        }
        else if (profile == VAProfileAV1Profile0)
        {
            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC('N', 'V', '1', '2');
            i++;

            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_P010;
            i++;
        }
        else if (profile == VAProfileHEVCMain12)
        {
            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_P012;
            i++;

            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_P016;
            i++;
        }
        else if (profile == VAProfileHEVCMain422_10)
        {
            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_YUY2;
            i++;

            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y210;
            i++;
        }
        else if (profile == VAProfileHEVCMain422_12)
        {
            //hevc  rext: Y216 12/16bit 422
#if VA_CHECK_VERSION(1, 9, 0)
            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y212;
            i++;
#endif

            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y216;
            i++;

            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_P012;
            i++;
        }
        else if (profile == VAProfileHEVCMain444 || profile == VAProfileVP9Profile1)
        {
            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_AYUV;
            i++;

#if VA_CHECK_VERSION(1, 13, 0)
            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_XYUV;
            i++;
#endif
        }
        else if (profile == VAProfileHEVCMain444_10 || profile == VAProfileVP9Profile3)
        {
            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y410;
            i++;

            if (profile == VAProfileVP9Profile3)
            {
#if VA_CHECK_VERSION(1, 9, 0)
                attribs[i].type          = VASurfaceAttribPixelFormat;
                attribs[i].value.type    = VAGenericValueTypeInteger;
                attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_Y412;
                i++;
#endif

                attribs[i].type          = VASurfaceAttribPixelFormat;
                attribs[i].value.type    = VAGenericValueTypeInteger;
                attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = VA_FOURCC_Y416;
                i++;
            }
        }
        else if (profile == VAProfileHEVCMain444_12)
        {
#if VA_CHECK_VERSION(1, 9, 0)
            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y412;
            i++;

            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y212;
            i++;
#endif

            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y416;
            i++;

            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_P012;
            i++;
        }
        else if (profile == VAProfileHEVCSccMain10)
        {
            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_P010;
            i++;
        }
        else if (profile == VAProfileHEVCSccMain444)
        {
            //422/444 8/10bit included
            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_YUY2;
            i++;

            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y210;
            i++;

            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_AYUV;
            i++;

#if VA_CHECK_VERSION(1, 13, 0)
            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_XYUV;
            i++;
#endif

            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y410;
            i++;

            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC('N', 'V', '1', '2');
            i++;
        }
        else if (profile == VAProfileHEVCSccMain444_10)
        {
            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_AYUV;
            i++;

#if VA_CHECK_VERSION(1, 13, 0)
            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_XYUV;
            i++;
#endif

            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC_Y410;
            i++;
        }
        else if (profile == VAProfileJPEGBaseline)
        {
            for (int32_t j = 0; j < m_numJpegSurfaceAttr; j++)
            {
                attribs[i].type          = VASurfaceAttribPixelFormat;
                attribs[i].value.type    = VAGenericValueTypeInteger;
                attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
                attribs[i].value.value.i = m_jpegSurfaceAttr[j];
                i++;
            }
        }
        else
        {
            attribs[i].type          = VASurfaceAttribPixelFormat;
            attribs[i].value.type    = VAGenericValueTypeInteger;
            attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC('N', 'V', '1', '2');
            i++;
        }

        auto maxWidth  = m_decDefaultMaxWidth;
        auto maxHeight = m_decDefaultMaxHeight;
        if (IsMpeg2Profile(profile))
        {
            maxWidth  = m_decMpeg2MaxWidth;
            maxHeight = m_decMpeg2MaxHeight;
        }
        else if (IsAvcProfile(profile))
        {
            maxWidth  = m_decAvcMaxWidth;
            maxHeight = m_decAvcMaxWidth;
        }
        else if (IsHevcProfile(profile))
        {
            maxWidth  = m_decHevcMax16kWidth;
            maxHeight = m_decHevcMax16kWidth;
        }
        else if (IsVc1Profile(profile))
        {
            maxWidth  = m_decVc1MaxWidth;
            maxHeight = m_decVc1MaxHeight;
        }
        else if (IsJpegProfile(profile))
        {
            maxWidth  = m_decJpegMaxWidth;
            maxHeight = m_decJpegMaxHeight;
        }
        else if (IsVp9Profile(profile))
        {
            maxWidth  = m_decVp9Max16kWidth;
            maxHeight = m_decVp9Max16kHeight;
        }
        else if (IsAV1Profile(profile))
        {
            maxWidth  = m_decAv1Max16kWidth;
            maxHeight = m_decAv1Max16kHeight;
        }

        attribs[i].type          = VASurfaceAttribMaxWidth;
        attribs[i].value.type    = VAGenericValueTypeInteger;
        attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE;
        attribs[i].value.value.i = maxWidth;
        i++;

        attribs[i].type          = VASurfaceAttribMaxHeight;
        attribs[i].value.type    = VAGenericValueTypeInteger;
        attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE;
        attribs[i].value.value.i = maxHeight;
        i++;

        attribs[i].type          = VASurfaceAttribMemoryType;
        attribs[i].value.type    = VAGenericValueTypeInteger;
        attribs[i].flags         = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
        attribs[i].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_VA |
#if VA_CHECK_VERSION(1, 21, 0)
                                   VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 |
#endif
                                   VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2;
        i++;
    }
    else if (entrypoint == VAEntrypointEncSlice || entrypoint == VAEntrypointEncSliceLP || entrypoint == VAEntrypointEncPicture || entrypoint == VAEntrypointFEI)
    {
        AddEncSurfaceAttributes(profile, entrypoint, attribs, i);
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

bool MediaLibvaCapsMtlBase::IsHevcProfile(VAProfile profile)
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
            (profile == VAProfileHEVCSccMain444) ||
            (profile == VAProfileHEVCSccMain444_10)
           );
}

VAStatus MediaLibvaCapsMtlBase::CreateEncAttributes(
        VAProfile profile,
        VAEntrypoint entrypoint,
        AttribMap **attributeList)
{
    if(IsVp8Profile(profile))
    {
        return VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
    }

    DDI_CHK_NULL(attributeList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus status = CreateAttributeList(attributeList);
    DDI_CHK_RET(status, "Failed to initialize Caps!");

    auto attribList = *attributeList;
    DDI_CHK_NULL(attribList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAConfigAttrib attrib;
    attrib.type = VAConfigAttribRTFormat;
    status = CheckEncRTFormat(profile, entrypoint, &attrib);
    DDI_CHK_RET(status, "Failed to Check Encode RT Format!");
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribMaxPictureWidth;
    GetPlatformSpecificAttrib(profile, entrypoint,
        VAConfigAttribMaxPictureWidth, &attrib.value);
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribMaxPictureHeight;
    GetPlatformSpecificAttrib(profile, entrypoint,
        VAConfigAttribMaxPictureHeight, &attrib.value);
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribEncJPEG;
    attrib.value =
        ((JPEG_MAX_QUANT_TABLE << 14)       | // max_num_quantization_tables : 3
         (JPEG_MAX_NUM_HUFF_TABLE_INDEX << 11)   | // max_num_huffman_tables : 3
         (1 << 7)                    | // max_num_scans : 4
         (jpegNumComponent << 4));              // max_num_components : 3
    // arithmatic_coding_mode = 0
    // progressive_dct_mode = 0
    // non_interleaved_mode = 0
    // differential_mode = 0
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribEncQualityRange;
    if (profile == VAProfileJPEGBaseline)
    {
        // JPEG has no target usage.
        attrib.value = 1;
    }
    else
    {
        attrib.value = NUM_TARGET_USAGE_MODES - 1;// Indicates TUs from 1 upto the value reported are supported
    }
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribEncPackedHeaders;
    attrib.value = VA_ATTRIB_NOT_SUPPORTED;
    if ((IsAvcProfile(profile))||(IsHevcProfile(profile))||(IsAV1Profile(profile)))
    {
        attrib.value = VA_ENC_PACKED_HEADER_PICTURE    |
            VA_ENC_PACKED_HEADER_SEQUENCE   |
            VA_ENC_PACKED_HEADER_SLICE      |
            VA_ENC_PACKED_HEADER_RAW_DATA   |
            VA_ENC_PACKED_HEADER_MISC;
    }
    else if (IsMpeg2Profile(profile))
    {
        attrib.value = VA_ENC_PACKED_HEADER_RAW_DATA;
    }
    else if(IsJpegProfile(profile))
    {
        attrib.value = VA_ENC_PACKED_HEADER_RAW_DATA;
    }
    else if(IsVp9Profile(profile))
    {
        attrib.value = VA_ENC_PACKED_HEADER_RAW_DATA;
    }

    (*attribList)[attrib.type] = attrib.value;
    if(IsJpegProfile(profile))
    {
        return status;
    }

    attrib.type = VAConfigAttribRateControl;
    attrib.value = VA_RC_CQP;
    if (entrypoint != VAEntrypointEncSliceLP ||
            (entrypoint == VAEntrypointEncSliceLP && MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels)))
    {
        attrib.value |= VA_RC_CBR | VA_RC_VBR | VA_RC_MB;
        if (IsHevcProfile(profile))
        {
#if VA_CHECK_VERSION(1, 10, 0)
            attrib.value |= VA_RC_TCBRC;
#endif
            attrib.value |= VA_RC_VCM | VA_RC_QVBR | VA_RC_ICQ;
        }
        if (IsVp9Profile(profile))
        {
            attrib.value |= VA_RC_ICQ;
        }
    }
    if (IsAV1Profile(profile))
    {
        attrib.value = VA_RC_CQP | VA_RC_CBR | VA_RC_VBR;
    }
    if (IsAvcProfile(profile) &&
            ((entrypoint == VAEntrypointEncSliceLP) && MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels)))
    {
        attrib.value |= VA_RC_ICQ | VA_RC_QVBR;
#if VA_CHECK_VERSION(1, 10, 0)
        attrib.value |= VA_RC_TCBRC;
#endif
    }
    if(entrypoint == VAEntrypointFEI)
    {
        attrib.value = VA_RC_CQP;
    }
    else if(entrypoint == VAEntrypointStats)
    {
        attrib.value = VA_RC_NONE;
    }
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribEncInterlaced;
    attrib.value = VA_ENC_INTERLACED_NONE;
#ifndef ANDROID
    if(IsAvcProfile(profile) && (entrypoint != VAEntrypointEncSliceLP))
    {
        attrib.value = VA_ENC_INTERLACED_FIELD;
    }
    if(IsMpeg2Profile(profile))
    {
        attrib.value = VA_ENC_INTERLACED_FRAME;
    }
#endif
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribEncMaxRefFrames;
    if (entrypoint == VAEntrypointEncSliceLP)
    {
        attrib.value = DDI_CODEC_VDENC_MAX_L0_REF_FRAMES | (DDI_CODEC_VDENC_MAX_L1_REF_FRAMES << DDI_CODEC_LEFT_SHIFT_FOR_REFLIST1);
        if (IsAvcProfile(profile))
        {
            attrib.value = DDI_CODEC_VDENC_MAX_L0_REF_FRAMES | (DDI_CODEC_VDENC_MAX_L1_REF_FRAMES_RAB_AVC << DDI_CODEC_LEFT_SHIFT_FOR_REFLIST1);
        }
        if (IsHevcProfile(profile))
        {
            attrib.value = DDI_CODEC_VDENC_MAX_L0_REF_FRAMES_LDB | (DDI_CODEC_VDENC_MAX_L1_REF_FRAMES_LDB << DDI_CODEC_LEFT_SHIFT_FOR_REFLIST1);
        }
    }
    else
    {
        // default value: 1 frame for each reference list
        attrib.value = 1 | (1 << 16);
        if(IsAvcProfile(profile))
        {
            attrib.value = CODECHAL_ENCODE_NUM_MAX_VME_L0_REF | (CODECHAL_ENCODE_NUM_MAX_VME_L1_REF << 16);
        }
        if(IsVp8Profile(profile))
        {
            attrib.value = ENCODE_VP8_NUM_MAX_L0_REF ;
        }
        if(IsVp9Profile(profile))
        {
            attrib.value = ENCODE_VP9_NUM_MAX_L0_REF;
        }
        if (IsHevcProfile(profile))
        {
            GetPlatformSpecificAttrib(profile, entrypoint,
                    VAConfigAttribEncMaxRefFrames, &attrib.value);
        }
    }
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribEncMaxSlices;
    if (entrypoint == VAEntrypointEncSliceLP)
    {
        if (IsAvcProfile(profile))
        {
            attrib.value = ENCODE_AVC_MAX_SLICES_SUPPORTED;
        }
        else if (IsHevcProfile(profile))
        {
            attrib.value = ENCODE_HEVC_VDENC_NUM_MAX_SLICES;
        }
    }
    else
    {
        attrib.value = 0;
        if (IsAvcProfile(profile))
        {
            attrib.value = ENCODE_AVC_MAX_SLICES_SUPPORTED;
        }
        else if (IsHevcProfile(profile))
        {
            GetPlatformSpecificAttrib(profile, entrypoint,
                    VAConfigAttribEncMaxSlices, &attrib.value);
        }
    }
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribEncSliceStructure;
    if (entrypoint == VAEntrypointEncSliceLP)
    {
        if (IsHevcProfile(profile))
        {
            attrib.value = VA_ENC_SLICE_STRUCTURE_ARBITRARY_MACROBLOCKS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE;
        }
        else
        {
            attrib.value = VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE |
                       VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS | VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS;
        }
    }
    else
    {
        attrib.value = VA_ENC_SLICE_STRUCTURE_ARBITRARY_MACROBLOCKS;
    }
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribEncQuantization;
    if(IsAvcProfile(profile))
    {
        attrib.value = VA_ENC_QUANTIZATION_TRELLIS_SUPPORTED;
    }
    else
    {
        attrib.value = VA_ENC_QUANTIZATION_NONE;
    }
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribEncIntraRefresh;
    attrib.value = VA_ENC_INTRA_REFRESH_NONE;
    GetPlatformSpecificAttrib(profile, entrypoint,
        VAConfigAttribEncIntraRefresh, &attrib.value);
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribEncSkipFrame;
    if (entrypoint == VAEntrypointEncSliceLP)
    {
        if (IsAvcProfile(profile))
        {
            attrib.value = 1;
        }
        else
        {
            attrib.value = 0;
        }
    }
    else
    {
        attrib.value = 1;
    }
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribEncryption;
    attrib.value = VA_ATTRIB_NOT_SUPPORTED;
    if (m_isEntryptSupported)
    {
        attrib.value = 0;
        uint32_t encryptTypes[DDI_CP_ENCRYPT_TYPES_NUM] = {0};
        int32_t  numTypes =  m_CapsCp->GetEncryptionTypes(profile,
                 encryptTypes, DDI_CP_ENCRYPT_TYPES_NUM);
        if (numTypes > 0)
        {
            for (int32_t j = 0; j < numTypes; j++)
            {
                attrib.value |= encryptTypes[j];
            }
        }
    }
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribEncROI;
    if (entrypoint == VAEntrypointEncSliceLP)
    {
        VAConfigAttribValEncROI roi_attrib = {0};
        if (IsAvcProfile(profile))
        {
            roi_attrib.bits.num_roi_regions = ENCODE_VDENC_AVC_MAX_ROI_NUMBER_ADV;
        }
        else if (IsHevcProfile(profile))
        {
            roi_attrib.bits.num_roi_regions = CODECHAL_ENCODE_HEVC_MAX_NUM_ROI;
        }

        roi_attrib.bits.roi_rc_priority_support = 0;
        roi_attrib.bits.roi_rc_qp_delta_support = 1;

        attrib.value = roi_attrib.value;
    }
    else
    {
        GetPlatformSpecificAttrib(profile, entrypoint,
                VAConfigAttribEncROI, &attrib.value);
    }
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribProcessingRate;
    attrib.value = VA_PROCESSING_RATE_ENCODE;
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = (VAConfigAttribType)VAConfigAttribEncDirtyRect;
    if((entrypoint == VAEntrypointEncSliceLP) && IsHevcProfile(profile))
    {
        attrib.value = CODECHAL_ENCODE_HEVC_MAX_NUM_DIRTYRECT;
    }
    else
    {
        attrib.value = 4;
    }

    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribEncParallelRateControl;
    if(entrypoint == VAEntrypointEncSliceLP)
    {
        attrib.value = 0;
    }
    else
    {
        attrib.value = 1;
    }
    (*attribList)[attrib.type] = attrib.value;

    if ((entrypoint == VAEntrypointFEI) && (IsAvcProfile(profile) || IsHevcProfile(profile)))
    {
        attrib.type = (VAConfigAttribType)VAConfigAttribFEIFunctionType;
        attrib.value = IsAvcProfile(profile) ?
                       (VA_FEI_FUNCTION_ENC | VA_FEI_FUNCTION_PAK | VA_FEI_FUNCTION_ENC_PAK) :
                       VA_FEI_FUNCTION_ENC_PAK;
        (*attribList)[attrib.type] = attrib.value;
    }

    attrib.type = (VAConfigAttribType)VAConfigAttribFEIMVPredictors;
    attrib.value = 0;
    if(IsAvcProfile(profile) || IsHevcProfile(profile))
    {
        attrib.value = DDI_CODEC_FEI_MAX_NUM_MVPREDICTOR;
    }
    (*attribList)[attrib.type] = attrib.value;

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

    attrib.type = (VAConfigAttribType)VAConfigAttribCustomRoundingControl;
    GetPlatformSpecificAttrib(profile, entrypoint,
            (VAConfigAttribType)VAConfigAttribCustomRoundingControl, &attrib.value);
    (*attribList)[attrib.type] = attrib.value;

    if (IsAvcProfile(profile) || IsHevcProfile(profile) )
    {
        attrib.type = (VAConfigAttribType)VAConfigAttribMaxFrameSize;
        VAConfigAttribValMaxFrameSize attribValMaxFrameSize;
        memset(&attribValMaxFrameSize, 0, sizeof(attribValMaxFrameSize));
        attribValMaxFrameSize.bits.max_frame_size = 1;
        attribValMaxFrameSize.bits.multiple_pass  = 1;
        attribValMaxFrameSize.bits.reserved       = 0;
        attrib.value = attribValMaxFrameSize.value;
        (*attribList)[attrib.type] = attrib.value;
    }

    if (IsHevcProfile(profile))
    {
        attrib.type = (VAConfigAttribType) VAConfigAttribPredictionDirection;
        attrib.value = VA_PREDICTION_DIRECTION_PREVIOUS | VA_PREDICTION_DIRECTION_FUTURE | VA_PREDICTION_DIRECTION_BI_NOT_EMPTY;
        (*attribList)[attrib.type] = attrib.value;
#if VA_CHECK_VERSION(1, 12, 0)
        attrib.type = (VAConfigAttribType)VAConfigAttribEncHEVCFeatures;
        GetPlatformSpecificAttrib(profile, entrypoint,
                (VAConfigAttribType)VAConfigAttribEncHEVCFeatures, &attrib.value);
        (*attribList)[attrib.type] = attrib.value;
        attrib.type = (VAConfigAttribType)VAConfigAttribEncHEVCBlockSizes;
        GetPlatformSpecificAttrib(profile, entrypoint,
                (VAConfigAttribType)VAConfigAttribEncHEVCBlockSizes, &attrib.value);
        (*attribList)[attrib.type] = attrib.value;
#endif
    }

    return status;
}

VAStatus MediaLibvaCapsMtlBase::GetSurfaceModifier(DDI_MEDIA_SURFACE* mediaSurface, uint64_t &modifier)
{
    DDI_CHK_NULL(mediaSurface,                   "nullptr mediaSurface",                   VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(mediaSurface->bo,               "nullptr mediaSurface->bo",               VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(mediaSurface->pGmmResourceInfo, "nullptr mediaSurface->pGmmResourceInfo", VA_STATUS_ERROR_INVALID_SURFACE);
    GMM_TILE_TYPE gmmTileType = mediaSurface->pGmmResourceInfo->GetTileType();
    GMM_RESOURCE_FLAG       GmmFlags    = {0};
    GmmFlags = mediaSurface->pGmmResourceInfo->GetResFlags();

    bool                    bMmcEnabled = false;
    if ((GmmFlags.Gpu.MMC               ||
        GmmFlags.Gpu.CCS)               &&
        (GmmFlags.Info.MediaCompressed ||
         GmmFlags.Info.RenderCompressed))
    {
        bMmcEnabled = true;
    }
    else
    {
        bMmcEnabled = false;
    }

    if(GMM_TILED_4 == gmmTileType)
    {
        if(m_mediaCtx->m_auxTableMgr && bMmcEnabled)
        {
            modifier = GmmFlags.Info.MediaCompressed ? I915_FORMAT_MOD_4_TILED_MTL_MC_CCS :
                (GmmFlags.Info.RenderCompressed ? I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC : I915_FORMAT_MOD_4_TILED);
        }
        else
        {
            modifier = I915_FORMAT_MOD_4_TILED;
        }
        return VA_STATUS_SUCCESS;
    }
    else
    {
        return MediaLibvaCaps::GetSurfaceModifier(mediaSurface, modifier);
    }
}

VAStatus MediaLibvaCapsMtlBase::SetExternalSurfaceTileFormat(DDI_MEDIA_SURFACE* mediaSurface, uint32_t &tileformat, bool &bMemCompEnable, bool &bMemCompRC)
{
    DDI_CHK_NULL(mediaSurface,                     "nullptr mediaSurface",                     VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(mediaSurface->pSurfDesc,          "nullptr mediaSurface->pSurfDesc",          VA_STATUS_ERROR_INVALID_SURFACE);

    switch (mediaSurface->pSurfDesc->modifier)
    {
        case I915_FORMAT_MOD_4_TILED:
            tileformat = TILING_Y;
            bMemCompEnable = false;
            break;
        case I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC:
            tileformat = TILING_Y;
            bMemCompEnable = true;
            bMemCompRC = true;
            break;
        case I915_FORMAT_MOD_4_TILED_MTL_MC_CCS:
            tileformat = TILING_Y;
            bMemCompEnable = true;
            bMemCompRC = false;
            break;
        default:
            return MediaLibvaCaps::SetExternalSurfaceTileFormat(mediaSurface, tileformat, bMemCompEnable, bMemCompRC);
    }

    return VA_STATUS_SUCCESS;
}
