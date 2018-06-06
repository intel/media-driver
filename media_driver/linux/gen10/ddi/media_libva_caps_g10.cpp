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
//! \file     media_libva_caps_g10.cpp
//! \brief    This file implements the C++ class/interface for gen10 media capbilities. 
//!

#include "codec_def_encode_hevc_g10.h"
#include "media_libva_util.h"
#include "media_libva.h"
#include "media_libva_caps_g10.h"
#include "media_libva_caps_factory.h"

VAStatus MediaLibvaCapsG10::GetPlatformSpecificAttrib(VAProfile profile,
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
        default:
            status = VA_STATUS_ERROR_INVALID_PARAMETER;
            break;
    }
    return status;
}

VAStatus MediaLibvaCapsG10::LoadHevcEncLpProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _HEVC_ENCODE_SUPPORTED
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

#ifdef _VP9_ENCODE_SUPPORTED
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
    status = LoadNoneProfileEntrypoints();
    DDI_CHK_RET(status, "Failed to initialize Caps!");

    return status;
}
VAStatus MediaLibvaCapsG10::CheckEncodeResolution(
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

VAStatus MediaLibvaCapsG10::QueryAVCROIMaxNum(uint32_t rcMode, bool isVdenc, int32_t *maxNum, bool *isRoiInDeltaQP)
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

#ifdef _AVC_ENCODE_SUPPORTED
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

#ifdef _HEVC_ENCODE_SUPPORTED
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

