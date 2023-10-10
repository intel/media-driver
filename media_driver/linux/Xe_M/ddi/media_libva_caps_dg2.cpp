/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     media_libva_caps_dg2.cpp
//! \brief    This file implements the C++ class/interface for dg2 media capbilities.
//!

#include "media_libva.h"
#include "media_libva_util.h"
#include "media_ddi_encode_const.h"
#include "codec_def_encode_hevc_g12.h"
#include "codec_def_encode_av1.h"
#include "media_libva_caps_dg2.h"
#include "media_libva_caps_factory.h"
#include "media_libva_caps_cp_interface.h"

extern template class MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>;

VAStatus MediaLibvaCapsDG2::LoadAv1EncProfileEntrypoints()
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
        (*attributeList)[VAConfigAttribEncDirtyRect]      = 0;
        (*attributeList)[VAConfigAttribEncMaxRefFrames]   = CODEC_AV1_NUM_REFL0P_FRAMES | CODEC_AV1_NUM_REFL1B_FRAMES<<16;

        VAConfigAttrib attrib;
        attrib.type = (VAConfigAttribType) VAConfigAttribEncAV1;
        VAConfigAttribValEncAV1 attribValAV1Tools;
        memset(&attribValAV1Tools, 0, sizeof(attribValAV1Tools));

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
        AddEncConfig(VA_RC_ICQ);
        AddEncConfig(VA_RC_TCBRC);
        AddProfileEntry(VAProfileAV1Profile0, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }
#endif
    return status;
}

VAStatus MediaLibvaCapsDG2::LoadAvcEncLpProfileEntrypoints()
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
#endif

    return status;
}

VAStatus MediaLibvaCapsDG2::LoadProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;
    status = MediaLibvaCapsG12::LoadProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");

    status = LoadAv1EncProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");

    return status;
}

std::string MediaLibvaCapsDG2::GetEncodeCodecKey(VAProfile profile, VAEntrypoint entrypoint, uint32_t feiFunction)
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
        case VAProfileHEVCMain444:
        case VAProfileHEVCMain444_10:
        case VAProfileHEVCSccMain:
        case VAProfileHEVCSccMain10:
        case VAProfileHEVCSccMain444:
        case VAProfileHEVCSccMain444_10:
            return ENCODE_ID_HEVC;
        case VAProfileAV1Profile0:
        case VAProfileAV1Profile1:
            return ENCODE_ID_AV1;
        case VAProfileNone:
            if (IsEncFei(entrypoint, feiFunction))
                return ENCODE_ID_AVCFEI;
            else
                return ENCODE_ID_NONE;
        default:
            return ENCODE_ID_NONE;
    }
}

CODECHAL_MODE MediaLibvaCapsDG2::GetEncodeCodecMode(VAProfile profile, VAEntrypoint entrypoint)
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
        case VAProfileHEVCMain444:
        case VAProfileHEVCMain444_10:
        case VAProfileHEVCSccMain:
        case VAProfileHEVCSccMain10:
        case VAProfileHEVCSccMain444:
        case VAProfileHEVCSccMain444_10:
            return CODECHAL_ENCODE_MODE_HEVC;
        case VAProfileAV1Profile0:
        case VAProfileAV1Profile1:
            return CODECHAL_ENCODE_MODE_AV1;
        default:
            DDI_ASSERTMESSAGE("Invalid Encode Mode");
            return CODECHAL_UNSUPPORTED_MODE;
    }
}

VAStatus MediaLibvaCapsDG2::CheckEncodeResolution(
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
        case VAProfileHEVCMain12:
        case VAProfileHEVCMain422_10:
        case VAProfileHEVCMain444:
        case VAProfileHEVCMain444_10:
        case VAProfileHEVCSccMain:
        case VAProfileHEVCSccMain10:
        case VAProfileHEVCSccMain444:
        case VAProfileHEVCSccMain444_10:
            if (width > m_maxHevcEncWidth
                    || width < (m_vdencActive ? m_hevcVDEncMinWidth : m_encMinWidth)
                    || height > m_maxHevcEncHeight
                    || height < (m_vdencActive ? m_hevcVDEncMinHeight : m_encMinHeight))
            {
                return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
            }
            break;
        case VAProfileVP9Profile0:
        case VAProfileVP9Profile1:
        case VAProfileVP9Profile2:
        case VAProfileVP9Profile3:
            if ((width > m_maxVp9EncWidth) ||
                (width < m_minVp9EncWidth) ||
                (height > m_maxVp9EncHeight) ||
                (height < m_minVp9EncHeight) )
            {
                return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
            }
            break;
        case VAProfileAV1Profile0:
        case VAProfileAV1Profile1:
             if ((width > CODEC_8K_MAX_PIC_WIDTH) ||
                 (width < m_encMinWidth) ||
                 (height > CODEC_8K_MAX_PIC_HEIGHT) ||
                 (height < m_encMinHeight))
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

VAStatus MediaLibvaCapsDG2::CheckEncRTFormat(
        VAProfile profile,
        VAEntrypoint entrypoint,
        VAConfigAttrib* attrib)
{
    DDI_CHK_NULL(attrib, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    attrib->type = VAConfigAttribRTFormat;
    if (profile == VAProfileJPEGBaseline)
    {
        attrib->value = VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV400 | VA_RT_FORMAT_YUV411 | VA_RT_FORMAT_RGB16 | VA_RT_FORMAT_RGB32;
    }
    else if(profile == VAProfileHEVCMain10 || profile == VAProfileHEVCSccMain10)
    {
        attrib->value = VA_RT_FORMAT_YUV420_10;
    }
    else if(profile == VAProfileHEVCMain12)
    {
        attrib->value = VA_RT_FORMAT_YUV420_12;
    }
    else if(profile == VAProfileHEVCMain422_10)
    {
        attrib->value = VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV422_10;
    }
    else if(profile == VAProfileHEVCMain444 || profile == VAProfileHEVCSccMain444)
    {
        attrib->value = VA_RT_FORMAT_YUV444;
    }
    else if(profile == VAProfileHEVCMain444_10 || profile == VAProfileHEVCSccMain444_10)
    {
        attrib->value = VA_RT_FORMAT_YUV444_10;
    }
    else
    {
        attrib->value = VA_RT_FORMAT_YUV420;
    }

    EncodeFormat format = Others;
    EncodeType type = entrypoint == VAEntrypointEncSliceLP ? Vdenc : DualPipe;
    struct EncodeFormatTable* encodeFormatTable = m_encodeFormatTable;

    if(IsAvcProfile(profile))
    {
        format = AVC;
    }
    else if(IsHevcProfile(profile))
    {
        format = HEVC;
    }
    else if(IsVp9Profile(profile))
    {
        format = VP9;
    }
    else if(IsAV1Profile(profile))
    {
        format = AV1;
    }

    for(uint32_t i = 0; i < m_encodeFormatCount && encodeFormatTable != nullptr; encodeFormatTable++, i++)
    {
        if(encodeFormatTable->encodeFormat == format
        && encodeFormatTable->encodeType == type)
        {
            attrib->value = encodeFormatTable->colorFormat;
            break;
        }
    }
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCapsDG2::GetPlatformSpecificAttrib(VAProfile profile,
        VAEntrypoint entrypoint,
        VAConfigAttribType type,
        uint32_t *value)
{
    DDI_CHK_NULL(value, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    VAStatus status = VA_STATUS_SUCCESS;
    *value = VA_ATTRIB_NOT_SUPPORTED;
    switch ((int)type)
    {
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
            else if(IsAV1Profile(profile))
            {
                *value = CODEC_8K_MAX_PIC_WIDTH;
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
            else if(IsAV1Profile(profile))
            {
                *value = CODEC_8K_MAX_PIC_HEIGHT;
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
            if (!IsHevcSccProfile(profile))
            {
                *value = VA_PREDICTION_DIRECTION_PREVIOUS | VA_PREDICTION_DIRECTION_FUTURE | VA_PREDICTION_DIRECTION_BI_NOT_EMPTY;
            }
            else
            {
                // Here we set
                // VAConfigAttribPredictionDirection: VA_PREDICTION_DIRECTION_PREVIOUS | VA_PREDICTION_DIRECTION_BI_NOT_EMPTY together with
                // VAConfigAttribEncMaxRefFrames: L0 != 0, L1 !=0
                // to indicate SCC only supports I/low delay B
                *value = VA_PREDICTION_DIRECTION_PREVIOUS | VA_PREDICTION_DIRECTION_BI_NOT_EMPTY;
            }
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

VAStatus MediaLibvaCapsDG2::CreateEncAttributes(
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

    if (profile == VAProfileJPEGBaseline)
    {
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
    }

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
    if (entrypoint == VAEntrypointEncSliceLP &&
        MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels) &&
        !IsHevcSccProfile(profile)) // Currently, SCC doesn't support BRC
    {
        attrib.value |= VA_RC_CBR | VA_RC_VBR | VA_RC_MB;
        if (IsHevcProfile(profile))
        {
            attrib.value |= VA_RC_ICQ | VA_RC_VCM | VA_RC_QVBR;
#if VA_CHECK_VERSION(1, 10, 0)
            attrib.value |= VA_RC_TCBRC;
#endif
        }
        if (IsVp9Profile(profile))
        {
            attrib.value |= VA_RC_ICQ;
        }
    }
    if (IsAV1Profile(profile))
    {
        attrib.value = VA_RC_CQP | VA_RC_CBR | VA_RC_VBR | VA_RC_ICQ;
#if VA_CHECK_VERSION(1, 10, 0)
        attrib.value |= VA_RC_TCBRC;
#endif
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
            attrib.value = VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS |
                        VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE | VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS;
        }
        else
        {
            attrib.value = VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE |
                       VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS;
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

    if (IsAvcProfile(profile) || IsHevcProfile(profile) || IsAV1Profile(profile))
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
        GetPlatformSpecificAttrib(profile, entrypoint,
                (VAConfigAttribType)VAConfigAttribPredictionDirection, &attrib.value);
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

VAStatus MediaLibvaCapsDG2::AddEncSurfaceAttributes(
        VAProfile profile,
        VAEntrypoint entrypoint,
        VASurfaceAttrib  *attribList,
        uint32_t &numAttribs)
{
    DDI_CHK_NULL(attribList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (profile == VAProfileAV1Profile0)
    {
        attribList[numAttribs].type = VASurfaceAttribPixelFormat;
        attribList[numAttribs].value.type = VAGenericValueTypeInteger;
        attribList[numAttribs].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
        attribList[numAttribs].value.value.i = VA_FOURCC_NV12;
        numAttribs++;

        attribList[numAttribs].type = VASurfaceAttribPixelFormat;
        attribList[numAttribs].value.type = VAGenericValueTypeInteger;
        attribList[numAttribs].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
        attribList[numAttribs].value.value.i = VA_FOURCC_P010;
        numAttribs++;

        attribList[numAttribs].type = VASurfaceAttribMaxWidth;
        attribList[numAttribs].value.type = VAGenericValueTypeInteger;
        attribList[numAttribs].flags = VA_SURFACE_ATTRIB_GETTABLE;
        attribList[numAttribs].value.value.i = CODEC_8K_MAX_PIC_WIDTH;
        numAttribs++;

        attribList[numAttribs].type = VASurfaceAttribMaxHeight;
        attribList[numAttribs].value.type = VAGenericValueTypeInteger;
        attribList[numAttribs].flags = VA_SURFACE_ATTRIB_GETTABLE;
        attribList[numAttribs].value.value.i = CODEC_8K_MAX_PIC_HEIGHT;
        numAttribs++;

        attribList[numAttribs].type = VASurfaceAttribMinWidth;
        attribList[numAttribs].value.type = VAGenericValueTypeInteger;
        attribList[numAttribs].flags = VA_SURFACE_ATTRIB_GETTABLE;
        attribList[numAttribs].value.value.i = m_encMinWidth;
        numAttribs++;

        attribList[numAttribs].type = VASurfaceAttribMinHeight;
        attribList[numAttribs].value.type = VAGenericValueTypeInteger;
        attribList[numAttribs].flags = VA_SURFACE_ATTRIB_GETTABLE;
        attribList[numAttribs].value.value.i = m_encMinHeight;
        numAttribs++;

        attribList[numAttribs].type = VASurfaceAttribMemoryType;
        attribList[numAttribs].value.type = VAGenericValueTypeInteger;
        attribList[numAttribs].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
        attribList[numAttribs].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_VA |
            VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2;
        numAttribs++;
    }
    else
    {
        return MediaLibvaCapsG12::AddEncSurfaceAttributes(profile, entrypoint, attribList, numAttribs);
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCapsDG2::GetDisplayAttributes(
            VADisplayAttribute *attribList,
            int32_t numAttribs)
{
    DDI_CHK_NULL(attribList, "Null attribList", VA_STATUS_ERROR_INVALID_PARAMETER);
    for(auto i = 0; i < numAttribs; i ++)
    {
        switch(attribList->type)
        {
#if VA_CHECK_VERSION(1, 15, 0)
            case VADisplayPCIID:
                attribList->min_value = attribList->value = attribList->max_value = (m_mediaCtx->iDeviceId & 0xffff) | 0x80860000;
                attribList->flags = VA_DISPLAY_ATTRIB_GETTABLE;
                break;
#endif
            case VADisplayAttribCopy:
                attribList->min_value = attribList->value = attribList->max_value =
                    (1 << VA_EXEC_MODE_POWER_SAVING) | (1 << VA_EXEC_MODE_PERFORMANCE) | (1 << VA_EXEC_MODE_DEFAULT);
                // 100: perfromance model: Render copy
                // 10:  POWER_SAVING: BLT
                // 1:   default model: 1: vebox
                // 0:   don't support media copy.
                attribList->flags = VA_DISPLAY_ATTRIB_GETTABLE;
                break;
            default:
                attribList->min_value = VA_ATTRIB_NOT_SUPPORTED;
                attribList->max_value = VA_ATTRIB_NOT_SUPPORTED;
                attribList->value = VA_ATTRIB_NOT_SUPPORTED;
                attribList->flags = VA_DISPLAY_ATTRIB_NOT_SUPPORTED;
                break;
        }
        attribList ++;
    }
    return VA_STATUS_SUCCESS;
}

static bool dg2Registered = MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>::
    RegisterCaps<MediaLibvaCapsDG2>((uint32_t)IGFX_DG2);
