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
//! \file     media_libva_caps_g11.cpp
//! \brief    This file implements the C++ class/interface for gen11 media capbilities. 
//!

#include "codec_def_encode_hevc_g11.h"
#include "media_libva_util.h"
#include "media_libva.h"
#include "media_libva_caps_g11.h"
#include "media_libva_caps_factory.h"
#include "media_ddi_decode_const.h"
#include "media_ddi_encode_const.h"
#include "media_ddi_decode_const_g11.h"
#include "media_libva_vp.h"

const VAImageFormat MediaLibvaCapsG11::m_G11ImageFormats[] =
{    {VA_FOURCC_AYUV, VA_LSB_FIRST, 24, 0,0,0,0,0},
     {VA_FOURCC_Y410, VA_LSB_FIRST, 24, 0,0,0,0,0},
     {VA_FOURCC_Y416, VA_LSB_FIRST, 64, 0,0,0,0,0}
};

const VAConfigAttribValEncRateControlExt MediaLibvaCapsG11::m_encVp9RateControlExt =
{
    {CODECHAL_ENCODE_VP9_MAX_NUM_TEMPORAL_LAYERS - 1, 1, 0}
};

VAStatus MediaLibvaCapsG11::QueryImageFormats(VAImageFormat *formatList, int32_t *numFormats)
{
    DDI_CHK_NULL(formatList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(numFormats, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    MediaLibvaCaps::QueryImageFormats(formatList, numFormats);

    int32_t num = *numFormats;
    uint32_t numG11ImageFormats = GetNumG11ImageFormats();

    for (int32_t idx = 0; idx < numG11ImageFormats; idx++)
    {
        formatList[num].fourcc           = m_G11ImageFormats[idx].fourcc;
        formatList[num].byte_order       = m_G11ImageFormats[idx].byte_order;
        formatList[num].bits_per_pixel   = m_G11ImageFormats[idx].bits_per_pixel;
        formatList[num].depth            = m_G11ImageFormats[idx].depth;
        formatList[num].red_mask         = m_G11ImageFormats[idx].red_mask;
        formatList[num].green_mask       = m_G11ImageFormats[idx].green_mask;
        formatList[num].blue_mask        = m_G11ImageFormats[idx].blue_mask;
        formatList[num].alpha_mask       = m_G11ImageFormats[idx].alpha_mask;
        num++;
    }
    *numFormats = num;

    return VA_STATUS_SUCCESS;
}

uint32_t MediaLibvaCapsG11::GetNumG11ImageFormats()
{
    return sizeof(m_G11ImageFormats)/sizeof(m_G11ImageFormats[0]);
}

uint32_t MediaLibvaCapsG11::GetImageFormatsMaxNum()
{
    return MediaLibvaCaps::GetImageFormatsMaxNum() +
        GetNumG11ImageFormats();
}

VAStatus MediaLibvaCapsG11::PopulateColorMaskInfo(VAImageFormat *vaImgFmt)
{
    if(MediaLibvaCaps::PopulateColorMaskInfo(vaImgFmt) == VA_STATUS_SUCCESS)
        return VA_STATUS_SUCCESS;

    uint32_t numG11ImageFormats = GetNumG11ImageFormats();

    for (int32_t idx = 0; idx < numG11ImageFormats; idx++)
    {
        if (m_G11ImageFormats[idx].fourcc == vaImgFmt->fourcc)
        {
            vaImgFmt->red_mask = m_G11ImageFormats[idx].red_mask;
            vaImgFmt->green_mask = m_G11ImageFormats[idx].green_mask;
            vaImgFmt->blue_mask = m_G11ImageFormats[idx].blue_mask;
            vaImgFmt->alpha_mask = m_G11ImageFormats[idx].alpha_mask;

            return VA_STATUS_SUCCESS;
        }
    }

    return VA_STATUS_ERROR_INVALID_IMAGE_FORMAT;
}

CODECHAL_MODE MediaLibvaCapsG11::GetEncodeCodecMode(VAProfile profile, VAEntrypoint entrypoint)
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
        case VAProfileHEVCMain444:
        case VAProfileHEVCMain444_10:
            return CODECHAL_ENCODE_MODE_HEVC;
        default:
            DDI_ASSERTMESSAGE("Invalid Encode Mode");
            return CODECHAL_UNSUPPORTED_MODE;
    }
}

std::string MediaLibvaCapsG11::GetDecodeCodecKey(VAProfile profile)
{
    switch (profile)
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
            return DECODE_ID_HEVC_G11;
        case VAProfileVC1Simple:
        case VAProfileVC1Main:
        case VAProfileVC1Advanced:
            return DECODE_ID_VC1;
        default:
            DDI_ASSERTMESSAGE("Invalid Encode Mode");
            return DECODE_ID_NONE;
    }
}

std::string MediaLibvaCapsG11::GetEncodeCodecKey(VAProfile profile, VAEntrypoint entrypoint)
{
    switch (profile)
    {
        case VAProfileH264High:
        case VAProfileH264Main:
        case VAProfileH264ConstrainedBaseline:
            if (IsEncFei(entrypoint))
            {
                return ENCODE_ID_AVCFEI;
            }
            else
            {
                return ENCODE_ID_AVC;
            }
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
        case VAProfileHEVCMain444:
        case VAProfileHEVCMain444_10:
            if (IsEncFei(entrypoint))
            {
                return ENCODE_ID_HEVCFEI;
            }
            else
            {
                return ENCODE_ID_HEVC;
            }
        case VAProfileNone:
            if (IsEncFei(entrypoint))
            {
                return ENCODE_ID_AVCFEI;
            }
            else
            {
                return ENCODE_ID_NONE;
            }
        default:
            return ENCODE_ID_NONE;
    }
}

VAStatus MediaLibvaCapsG11::GetPlatformSpecificAttrib(VAProfile profile,
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
                *value = ENCODE_DP_HEVC_NUM_MAX_VME_L0_REF_G11 | (ENCODE_DP_HEVC_NUM_MAX_VME_L1_REF_G11 << 16);;
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
                // the capacity is differnt for CQP and BRC mode, set it as larger one here
                uint32_t roiBRCPriorityLevelSupport = 1;
                uint32_t roiBRCQpDeltaSupport = 1;
                *value = (roiBRCPriorityLevelSupport << 9)
                    | (roiBRCQpDeltaSupport << 8)
                    | ENCODE_DP_AVC_MAX_ROI_NUM_BRC;
            }
            else
            {
                *value = 0;
            }
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
                *value = CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6;
            }
            else
            {
                *value =0;
                status = VA_STATUS_ERROR_INVALID_PARAMETER;
            }
            break;
        }
        default:
            status = VA_STATUS_ERROR_INVALID_PARAMETER;
            break;
    }
    return status;
}

VAStatus MediaLibvaCapsG11::LoadHevcEncLpProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
    AttribMap *attributeList = nullptr;

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain10)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain444)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain10bit444))
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
    
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain444))
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
        AddProfileEntry(VAProfileHEVCMain444, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }
    
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain10bit444))
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
        AddProfileEntry(VAProfileHEVCMain444_10, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }
#endif
    return status;
}

VAStatus MediaLibvaCapsG11::LoadVp9EncProfileEntrypoints()
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

VAStatus MediaLibvaCapsG11::LoadProfileEntrypoints()
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
#ifdef ENABLE_KERNELS
    status = LoadNoneProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");
#endif
    return status;
}

VAStatus MediaLibvaCapsG11::CheckEncodeResolution(
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
        case VAProfileHEVCMain444:
        case VAProfileHEVCMain444_10:
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
VAStatus MediaLibvaCapsG11::CheckDecodeResolution(
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

VAStatus MediaLibvaCapsG11::QueryAVCROIMaxNum(uint32_t rcMode, bool isVdenc, uint32_t *maxNum, bool *isRoiInDeltaQP)
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

GMM_RESOURCE_FORMAT MediaLibvaCapsG11::ConvertMediaFmtToGmmFmt(
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
        case Media_Format_NV12       : return GMM_FORMAT_NV12_TYPE;
        case Media_Format_NV21       : return GMM_FORMAT_NV21_TYPE;
        case Media_Format_YUY2       : return GMM_FORMAT_YUY2;
        case Media_Format_UYVY       : return GMM_FORMAT_UYVY;
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
        case Media_Format_Y210       : return GMM_FORMAT_Y210_TYPE;
        case Media_Format_AYUV       : return GMM_FORMAT_AYUV_TYPE;
        case Media_Format_Y410       : return GMM_FORMAT_Y410_TYPE;
        default                      : return GMM_FORMAT_INVALID;
    }
}

VAStatus MediaLibvaCapsG11::QuerySurfaceAttributes(
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
        else if (profile == VAProfileVP9Profile1)
        {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC('A', 'Y', 'U', 'V');
            i++;
        }
        else if (profile == VAProfileVP9Profile3)
        {
            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC('Y', '4', '1', '6');
            i++;

            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC('A', 'R', 'G', 'B');
            i++;

            attribs[i].type = VASurfaceAttribPixelFormat;
            attribs[i].value.type = VAGenericValueTypeInteger;
            attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribs[i].value.value.i = VA_FOURCC('A', 'B', 'G', 'R');
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
            attribs[i].value.value.i = CODEC_8K_MAX_PIC_WIDTH;
        }
        else if(IsAvcProfile(profile))
        {
            attribs[i].value.value.i = CODEC_4K_MAX_PIC_WIDTH;
        }
        else if (IsVp9Profile(profile))
        {
            attribs[i].value.value.i = m_maxVp9EncWidth;
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
            attribs[i].value.value.i = CODEC_8K_MAX_PIC_HEIGHT;
        }
        else if(IsAvcProfile(profile))
        {
            attribs[i].value.value.i = CODEC_4K_MAX_PIC_HEIGHT;
        }
        else if (IsVp9Profile(profile))
        {
            attribs[i].value.value.i = m_maxVp9EncHeight;
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

extern template class MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>;

static bool iclRegistered = MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>::
    RegisterCaps<MediaLibvaCapsG11>((uint32_t)IGFX_ICELAKE); 
static bool icllpRegistered = MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>::
    RegisterCaps<MediaLibvaCapsG11>((uint32_t)IGFX_ICELAKE_LP);
