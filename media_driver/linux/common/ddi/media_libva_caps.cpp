/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     media_libva_caps.cpp
//! \brief    This file implements the base C++ class/interface for media capbilities.
//!

#include "hwinfo_linux.h"
#include "linux_system_info.h"
#include "media_libva_util.h"
#include "media_libva_vp.h"
#include "media_libva_common.h"
#include "media_libva_caps.h"
#include "media_libva_caps_cp_interface.h"
#include "media_ddi_decode_const.h"
#include "media_ddi_encode_const.h"
#include "media_libva_caps_factory.h"

typedef MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT> CapsFactory;

#ifdef ANDROID
#include <va/va_android.h>
#endif

#include "set"

#ifndef VA_ENCRYPTION_TYPE_NONE
#define VA_ENCRYPTION_TYPE_NONE 0x00000000
#endif


const uint32_t MediaLibvaCaps::m_decSliceMode[2] =
{
    VA_DEC_SLICE_MODE_NORMAL,
    VA_DEC_SLICE_MODE_BASE
};

const uint32_t MediaLibvaCaps::m_decProcessMode[2] =
{
    VA_DEC_PROCESSING_NONE,
    VA_DEC_PROCESSING
};

const uint32_t MediaLibvaCaps::m_encRcMode[9] =
{
    VA_RC_CQP, VA_RC_CBR, VA_RC_VBR,
    VA_RC_CBR | VA_RC_MB, VA_RC_VBR | VA_RC_MB,
    VA_RC_ICQ, VA_RC_VCM, VA_RC_QVBR, VA_RC_AVBR
};

const uint32_t MediaLibvaCaps::m_vpSurfaceAttr[m_numVpSurfaceAttr] =
{
    VA_FOURCC('I', '4', '2', '0'),
    VA_FOURCC('Y', 'V', '1', '2'),
    VA_FOURCC('Y', 'U', 'Y', '2'),
    VA_FOURCC('4', '2', '2', 'H'),
    VA_FOURCC('4', '2', '2', 'V'),
    VA_FOURCC('R', 'G', 'B', 'A'),
    VA_FOURCC('B', 'G', 'R', 'A'),
    VA_FOURCC('R', 'G', 'B', 'P'),
    VA_FOURCC('R', 'G', 'B', 'X'),
    VA_FOURCC('P', '0', '1', '0'),
    VA_FOURCC('R','G','2', '4'),
    VA_FOURCC_ARGB,
    VA_FOURCC_ABGR,
    VA_FOURCC_A2R10G10B10,
    VA_FOURCC_A2B10G10R10
};

const uint32_t MediaLibvaCaps::m_jpegSurfaceAttr[m_numJpegSurfaceAttr] =
{
    VA_FOURCC_NV12,
    VA_FOURCC_IMC3,
    VA_FOURCC_Y800,
    VA_FOURCC_411P,
    VA_FOURCC_422H,
    VA_FOURCC_422V,
    VA_FOURCC_444P
};

const uint32_t MediaLibvaCaps::m_jpegEncSurfaceAttr[m_numJpegEncSurfaceAttr] =
{
    VA_FOURCC_NV12,
    VA_FOURCC_YUY2,
    VA_FOURCC_UYVY,
    VA_FOURCC_Y800
};

MediaLibvaCaps::MediaLibvaCaps(DDI_MEDIA_CONTEXT *mediaCtx)
{
    m_mediaCtx = mediaCtx;
    m_CapsCp = Create_MediaLibvaCapsCpInterface();
    m_isEntryptSupported = m_CapsCp->IsDecEncryptionSupported(m_mediaCtx);
}

MediaLibvaCaps::~MediaLibvaCaps()
{
    FreeAttributeList();
    Delete_MediaLibvaCapsCpInterface(m_CapsCp);
    m_CapsCp = nullptr;
}

bool MediaLibvaCaps::CheckEntrypointCodecType(VAEntrypoint entrypoint, CodecType codecType)
{
    switch (codecType)
    {
        case videoEncode:
            if((entrypoint == VAEntrypointEncSlice)
                    || (entrypoint == VAEntrypointEncSliceLP)
                    || (entrypoint == VAEntrypointEncPicture)
                    || (entrypoint == VAEntrypointFEI)
                    || (entrypoint == VAEntrypointStats))
            {
                return true;
            }
            else
            {
                return false;
            }
            break;
        case videoDecode:
            return (entrypoint == VAEntrypointVLD);
        case videoProcess:
            if(entrypoint == VAEntrypointVideoProc)
            {
                return true;
            }
            else
            {
                return false;
            }
            break;
        default:
            DDI_ASSERTMESSAGE("DDI: Unsupported codecType");
            return false;
    }
}

VAStatus MediaLibvaCaps::AddDecConfig(uint32_t slicemode, uint32_t encryptType, uint32_t processType)
{
    m_decConfigs.emplace_back(slicemode, encryptType, processType);
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCaps::AddEncConfig(uint32_t rcMode, uint32_t feiFunction)
{
    m_encConfigs.emplace_back(rcMode, feiFunction);
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCaps::AddVpConfig(uint32_t attrib)
{
    m_vpConfigs.emplace_back(attrib);
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCaps::GetProfileEntrypointFromConfigId(
        VAConfigID configId,
        VAProfile *profile,
        VAEntrypoint *entrypoint,
        int32_t *profileTableIdx)
{
    DDI_CHK_NULL(profile, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(entrypoint, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(profileTableIdx, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    CodecType codecType;

    int32_t configOffset = 0;
    if((configId < (DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_BASE + m_decConfigs.size())) )
    {
        configOffset = configId - DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_BASE;
        codecType = videoDecode;
    }
    else if( (configId >= DDI_CODEC_GEN_CONFIG_ATTRIBUTES_ENC_BASE) && (configId < (DDI_CODEC_GEN_CONFIG_ATTRIBUTES_ENC_BASE + m_encConfigs.size())) )
    {
        configOffset = configId - DDI_CODEC_GEN_CONFIG_ATTRIBUTES_ENC_BASE;
        codecType = videoEncode;
    }
    else if( (configId >= DDI_VP_GEN_CONFIG_ATTRIBUTES_BASE) && (configId < (DDI_VP_GEN_CONFIG_ATTRIBUTES_BASE + m_vpConfigs.size())))
    {
        configOffset = configId - DDI_VP_GEN_CONFIG_ATTRIBUTES_BASE;
        codecType = videoProcess;
    }
    else
    {
        return VA_STATUS_ERROR_INVALID_CONFIG;
    }

    int32_t i;
    for (i = 0; i < m_profileEntryCount; i++)
    {
        if (CheckEntrypointCodecType(m_profileEntryTbl[i].m_entrypoint, codecType))
        {
            int32_t configStart = m_profileEntryTbl[i].m_configStartIdx;
            int32_t configEnd = m_profileEntryTbl[i].m_configStartIdx + m_profileEntryTbl[i].m_configNum;
            if (configOffset >= configStart && configOffset < configEnd)
            {
                break;
            }
        }
    }

    if (i == m_profileEntryCount)
    {
        return VA_STATUS_ERROR_INVALID_CONFIG;
    }
    else
    {
        *entrypoint  = m_profileEntryTbl[i].m_entrypoint;
        *profile = m_profileEntryTbl[i].m_profile;
        *profileTableIdx = i;
    }
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCaps::AddProfileEntry(
        VAProfile profile,
        VAEntrypoint entrypoint,
        AttribMap *attributeList,
        int32_t configStartIdx,
        int32_t configNum)
{
    if (m_profileEntryCount >= m_maxProfileEntries)
    {
        DDI_ASSERTMESSAGE("Invalid profile entrypoint number");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    m_profileEntryTbl[m_profileEntryCount].m_profile = profile;
    m_profileEntryTbl[m_profileEntryCount].m_entrypoint = entrypoint;
    m_profileEntryTbl[m_profileEntryCount].m_attributes = attributeList;
    m_profileEntryTbl[m_profileEntryCount].m_configStartIdx = configStartIdx;
    m_profileEntryTbl[m_profileEntryCount].m_configNum = configNum;
    m_profileEntryCount++;

    return VA_STATUS_SUCCESS;
}

int32_t MediaLibvaCaps::GetProfileTableIdx(VAProfile profile, VAEntrypoint entrypoint)
{
    // initialize ret value to "invalid profile"
    int32_t ret = -1;
    for (int32_t i = 0; i < m_profileEntryCount; i++)
    {
        if (m_profileEntryTbl[i].m_profile == profile)
        {
            //there are such profile , but no such entrypoint
            ret = -2;
            if(m_profileEntryTbl[i].m_entrypoint == entrypoint)
            {
                return i;
            }
        }
    }

    return ret;
}

VAStatus MediaLibvaCaps::CreateAttributeList(AttribMap **attributeList)
{
    DDI_CHK_NULL(attributeList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    *attributeList = MOS_New(AttribMap);
    DDI_CHK_NULL(*attributeList, "Null pointer", VA_STATUS_ERROR_ALLOCATION_FAILED);
    m_attributeLists.push_back(*attributeList);

    return VA_STATUS_SUCCESS;
}

int32_t MediaLibvaCaps::GetAttributeIndex(std::vector<VAConfigAttrib> *attribList, VAConfigAttribType type)
{
    DDI_CHK_NULL(attribList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    uint32_t attribSize = attribList->size();
    for (uint32_t i = 0; i < attribSize; i++)
    {
        if ((*attribList)[i].type == type)
        {
            return i;
        }
    }
    return -1;

}

VAStatus MediaLibvaCaps::SetAttribute(
        std::vector<VAConfigAttrib> *attributeList,
        VAConfigAttribType type,
        uint32_t value)
{
    DDI_CHK_NULL(attributeList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    int32_t index = GetAttributeIndex(attributeList, type);
    if (index >= 0)
    {
        (*attributeList)[index].value = value;
        return VA_STATUS_SUCCESS;
    }
    else
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
}

VAStatus MediaLibvaCaps::SetAttribute(
        VAProfile profile,
        VAEntrypoint entrypoint,
        VAConfigAttribType type,
        uint32_t value)
{
    int32_t idx = GetProfileTableIdx(profile, entrypoint);
    DDI_CHK_LARGER(idx, -1, "Didn't find the profile table", VA_STATUS_ERROR_INVALID_PARAMETER);
 
    auto attribList = m_profileEntryTbl[idx].m_attributes;
    DDI_CHK_NULL(attribList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    (*attribList)[type] = value;
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCaps::FreeAttributeList()
{
    uint32_t attribListCount = m_attributeLists.size();
    for (uint32_t i = 0; i < attribListCount; i++)
    {
        m_attributeLists[i]->clear();
        MOS_Delete(m_attributeLists[i]);
        m_attributeLists[i] = nullptr;
    }
    m_attributeLists.clear();
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCaps::CheckEncRTFormat(
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
    else if(profile == VAProfileHEVCMain10)
    {
        attrib->value = VA_RT_FORMAT_YUV420_10;
    }
    else if(profile == VAProfileHEVCMain12)
    {
        attrib->value = VA_RT_FORMAT_YUV420_12;
    }
    else if(profile == VAProfileHEVCMain422_10)
    {
        attrib->value = VA_RT_FORMAT_YUV422_10;
    }
    else if(profile == VAProfileHEVCMain422_12)
    {
        attrib->value = VA_RT_FORMAT_YUV422_12;
    }
    else if(profile == VAProfileHEVCMain444)
    {
        attrib->value = VA_RT_FORMAT_YUV444;
    }
    else if(profile == VAProfileHEVCMain444_10)
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

VAStatus MediaLibvaCaps::CreateEncAttributes(
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
    status = CheckEncRTFormat(profile, entrypoint, &attrib);
    DDI_CHK_RET(status, "Failed to Check Encode RT Format!");
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribMaxPictureWidth;
    attrib.value = CODEC_MAX_PIC_WIDTH;
    if(profile == VAProfileJPEGBaseline)
    {
        attrib.value = ENCODE_JPEG_MAX_PIC_WIDTH;
    }
    if(IsHevcProfile(profile))
    {
        attrib.value = CODEC_8K_MAX_PIC_WIDTH;
    }
    if(IsAvcProfile(profile) || IsVp8Profile(profile))
    {
        attrib.value = CODEC_4K_MAX_PIC_WIDTH;
    }
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribMaxPictureHeight;
    attrib.value = CODEC_MAX_PIC_HEIGHT;
    if(profile == VAProfileJPEGBaseline)
    {
        attrib.value = ENCODE_JPEG_MAX_PIC_HEIGHT;
    }
    if(IsHevcProfile(profile))
    {
        attrib.value = CODEC_8K_MAX_PIC_HEIGHT;
    }
    if(IsAvcProfile(profile) || IsVp8Profile(profile))
    {
        attrib.value = CODEC_4K_MAX_PIC_HEIGHT;
    }
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
    if ((IsAvcProfile(profile))||(IsHevcProfile(profile)))
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
            attrib.value |= VA_RC_ICQ | VA_RC_VCM | VA_RC_QVBR;
        }
    }
    if (IsAvcProfile(profile) && (entrypoint != VAEntrypointEncSliceLP))
    {
        attrib.value |= VA_RC_ICQ | VA_RC_VCM | VA_RC_QVBR | VA_RC_AVBR;
    }
    if (IsAvcProfile(profile) &&
            ((entrypoint == VAEntrypointEncSliceLP) && MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels)))
    {
        attrib.value |= VA_RC_QVBR;
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
    if(IsAvcProfile(profile))
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
        attrib.value = VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE;
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
        uint32_t encryptTypes[3] = {0};
        int32_t  numTypes =  m_CapsCp->GetEncryptionTypes(profile,
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

    attrib.type = VAConfigAttribEncROI;
    if (entrypoint == VAEntrypointEncSliceLP)
    {
        VAConfigAttribValEncROI roi_attrib = {0};
        if (IsAvcProfile(profile))
        {
            roi_attrib.bits.num_roi_regions = ENCODE_VDENC_AVC_MAX_ROI_NUMBER_G9;
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
    attrib.value = 4;
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribEncParallelRateControl;
    attrib.value = 1;
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

    if (IsAvcProfile(profile))
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

    return status;
}

VAStatus MediaLibvaCaps::CreateDecAttributes(
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
    else if(profile == VAProfileHEVCMain10)
    {
        attrib.value = VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10;
    }
    else if(profile == VAProfileHEVCMain422_10)
    {
        attrib.value = VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV400 | VA_RT_FORMAT_YUV420_10 | VA_RT_FORMAT_YUV422_10;
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
                || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelHEVCVLDMain12bit444Decoding))
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
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelVP9VLDProfile2Decoding10bit420))
        {
            (*attribList) [VAConfigAttribRTFormat] |= VA_RT_FORMAT_YUV420_10;
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
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelVP9VLDProfile1Decoding8bit444))
            (*attribList) [VAConfigAttribRTFormat] |= VA_RT_FORMAT_YUV444;
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrVP9VLD10bProfile2Decoding))
            (*attribList) [VAConfigAttribRTFormat] |= VA_RT_FORMAT_YUV420_10;
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelVP9VLDProfile3Decoding10bit444))
            (*attribList) [VAConfigAttribRTFormat] |= VA_RT_FORMAT_YUV444_10;
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelVP9VLDProfile2Decoding12bit420))
            (*attribList) [VAConfigAttribRTFormat] |= VA_RT_FORMAT_YUV420_12;
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelVP9VLDProfile3Decoding12bit444))
            (*attribList) [VAConfigAttribRTFormat] |= VA_RT_FORMAT_YUV444_12;
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
        attrib.value = CODEC_4K_MAX_PIC_WIDTH;
    }
    if(IsHevcProfile(profile) || IsVp9Profile(profile))
    {
        attrib.value = CODEC_8K_MAX_PIC_WIDTH;
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
        attrib.value = CODEC_4K_MAX_PIC_HEIGHT;
    }
    if(IsHevcProfile(profile) || IsVp9Profile(profile))
    {
        attrib.value = CODEC_8K_MAX_PIC_HEIGHT;
    }
    (*attribList)[attrib.type] = attrib.value;

    attrib.type = VAConfigAttribEncryption;
    attrib.value = VA_ATTRIB_NOT_SUPPORTED;
    if (m_isEntryptSupported)
    {
        attrib.value = 0;
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

VAStatus MediaLibvaCaps::LoadAvcDecProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _AVC_DECODE_SUPPORTED
    AttribMap *attributeList = nullptr;
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrAVCVLDLongDecoding)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrAVCVLDShortDecoding))
    {
        status = CreateDecAttributes(VAProfileH264Main, VAEntrypointVLD, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");

        VAProfile profile[3] = {
            VAProfileH264Main,
            VAProfileH264High,
            VAProfileH264ConstrainedBaseline};

        uint32_t configStartIdx, configNum;
        for (int32_t i = 0; i < 3; i++)
        {
            configStartIdx = m_decConfigs.size();
            for (int32_t j = 0; j < 2; j++)
            {
                for (int32_t k = 0; k < 2; k++)
                {
                    AddDecConfig(m_decSliceMode[j], VA_ENCRYPTION_TYPE_NONE, m_decProcessMode[k]);
                    if (m_isEntryptSupported)
                    {
                        uint32_t encrytTypes[3];

                        int32_t numTypes = m_CapsCp->GetEncryptionTypes(profile[i],
                                encrytTypes, 3);

                        if (numTypes > 0)
                        {
                            for (int32_t l = 0; l < numTypes; l++)
                            {
                                AddDecConfig(m_decSliceMode[j], encrytTypes[l],
                                        m_decProcessMode[k]);
                            }
                        }
                    }
                }
            }

            configNum = m_decConfigs.size() - configStartIdx;
            AddProfileEntry(profile[i], VAEntrypointVLD, attributeList, configStartIdx, configNum);
        }
    }
#endif
    return status;
}

VAStatus MediaLibvaCaps::LoadAvcEncProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#if defined (_AVC_ENCODE_VME_SUPPORTED) || defined (_AVC_ENCODE_VDENC_SUPPORTED)
    AttribMap *attributeList = nullptr;
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeAVC))
    {
        status = CreateEncAttributes(VAProfileH264Main, VAEntrypointEncSlice, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");

        VAProfile profile[3] = {
            VAProfileH264Main,
            VAProfileH264High,
            VAProfileH264ConstrainedBaseline};

        VAEntrypoint entrypoint[2] = {VAEntrypointEncSlice, VAEntrypointFEI};

        uint32_t feiFunctions[3] = {
                VA_FEI_FUNCTION_ENC,
                VA_FEI_FUNCTION_PAK,
                VA_FEI_FUNCTION_ENC_PAK};

        uint32_t configStartIdx;

        for (int32_t e = 0; e < 2; e++)
        {
            status = CreateEncAttributes(VAProfileH264ConstrainedBaseline, entrypoint[e], &attributeList);
            DDI_CHK_RET(status, "Failed to initialize Caps!");

            for (int32_t i = 0; i < 3; i++)
            {
                configStartIdx = m_encConfigs.size();
                bool isFei = !!(entrypoint[e] == VAEntrypointFEI);
                int32_t maxRcMode = (entrypoint[e] == VAEntrypointEncSlice ? 9 : 1);
                for (int32_t j = 0; j < maxRcMode; j++)
                {
                    if (isFei)
                    {
                        for (int32_t k = 0; k < 3; k++)
                        {
                            AddEncConfig(m_encRcMode[j], feiFunctions[k]);
                        }
                    }
                    else
                        AddEncConfig(m_encRcMode[j]);
                }
                AddProfileEntry(profile[i], entrypoint[e], attributeList,
                        configStartIdx, m_encConfigs.size() - configStartIdx);
            }
        }
    }
#endif
    return status;
}

VAStatus MediaLibvaCaps::LoadAvcEncLpProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#if defined (_AVC_ENCODE_VME_SUPPORTED) || defined (_AVC_ENCODE_VDENC_SUPPORTED)
    AttribMap *attributeList = nullptr;
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeAVCVdenc))
    {
        status = CreateEncAttributes(VAProfileH264Main, VAEntrypointEncSliceLP, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");

        VAProfile profile[3] = {
            VAProfileH264Main,
            VAProfileH264High,
            VAProfileH264ConstrainedBaseline};

        for (int32_t i = 0; i < 3; i++)
        {
            uint32_t configStartIdx = m_encConfigs.size();
            AddEncConfig(VA_RC_CQP);

            if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
            {
                /* m_encRcMode[0] is VA_RC_CQP and it is already added */
                for (int32_t j = 1; j < 5; j++)
                {
                    AddEncConfig(m_encRcMode[j]);
                }
                AddEncConfig(VA_RC_QVBR);
            }
            AddProfileEntry(profile[i], VAEntrypointEncSliceLP, attributeList,
                    configStartIdx, m_encConfigs.size() - configStartIdx);
        }
    }
#endif

    return status;
}

VAStatus MediaLibvaCaps::LoadMpeg2DecProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _MPEG2_DECODE_SUPPORTED
    AttribMap *attributeList = nullptr;
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrMPEG2VLDDecoding))
    {
        status = CreateDecAttributes(VAProfileMPEG2Simple, VAEntrypointVLD, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");

        VAProfile profile[2] = {VAProfileMPEG2Simple, VAProfileMPEG2Main};

        for (int32_t i = 0; i < 2; i++)
        {
            uint32_t configStartIdx = m_decConfigs.size();
            AddDecConfig(VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE);
            AddProfileEntry(profile[i], VAEntrypointVLD, attributeList, configStartIdx, 1);
        }
    }
#endif

    return status;
}

VAStatus MediaLibvaCaps::LoadMpeg2EncProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _MPEG2_ENCODE_VME_SUPPORTED
    AttribMap *attributeList = nullptr;
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeMPEG2))
    {
        status = CreateEncAttributes(VAProfileMPEG2Simple, VAEntrypointEncSlice, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");

        VAProfile profile[2] = {VAProfileMPEG2Simple, VAProfileMPEG2Main};
        for (int32_t i = 0; i < 2; i++)
        {
            uint32_t configStartIdx = m_encConfigs.size();
            for (int32_t j = 0; j < 3; j++)
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

VAStatus MediaLibvaCaps::LoadJpegDecProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _JPEG_DECODE_SUPPORTED
    AttribMap *attributeList = nullptr;
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelJPEGDecoding))
    {
        status = CreateDecAttributes(VAProfileJPEGBaseline, VAEntrypointVLD, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");

        uint32_t configStartIdx = m_decConfigs.size();
        AddDecConfig(VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE);
        AddProfileEntry(VAProfileJPEGBaseline, VAEntrypointVLD, attributeList, configStartIdx, 1);
    }
#endif

    return status;
}

VAStatus MediaLibvaCaps::LoadJpegEncProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _JPEG_ENCODE_SUPPORTED
    AttribMap *attributeList = nullptr;
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeJPEG))
    {
        status = CreateEncAttributes(VAProfileJPEGBaseline, VAEntrypointEncPicture, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_NONE);
        AddProfileEntry(VAProfileJPEGBaseline, VAEntrypointEncPicture, attributeList,
                configStartIdx, 1);
    }
#endif
    return status;
}

VAStatus MediaLibvaCaps::LoadVc1DecProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _VC1_DECODE_SUPPORTED
    AttribMap *attributeList = nullptr;
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrVC1VLDDecoding))
    {
        status = CreateDecAttributes(VAProfileVC1Main, VAEntrypointVLD, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");
        VAProfile profile[3] = {VAProfileVC1Advanced, VAProfileVC1Main, VAProfileVC1Simple};

        for (int32_t i = 0; i < 3; i++)
        {
            uint32_t configStartIdx = m_decConfigs.size();
            AddDecConfig(VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE);
            AddProfileEntry(profile[i], VAEntrypointVLD, attributeList, configStartIdx, 1);
        }
    }
#endif

    return status;
}

VAStatus MediaLibvaCaps::LoadVp8DecProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _VP8_DECODE_SUPPORTED
    AttribMap *attributeList;
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelVP8VLDDecoding))
    {
        status = CreateDecAttributes(VAProfileVP8Version0_3, VAEntrypointVLD, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");

        uint32_t configStartIdx = m_decConfigs.size();
        AddDecConfig(VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE);
        AddProfileEntry(VAProfileVP8Version0_3, VAEntrypointVLD, attributeList, configStartIdx, 1);
    }
#endif

    return status;
}

VAStatus MediaLibvaCaps::LoadVp8EncProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _VP8_ENCODE_SUPPORTED
    AttribMap *attributeList;
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeVP8))
    {
        status = CreateEncAttributes(VAProfileVP8Version0_3, VAEntrypointEncSlice, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");

        uint32_t configStartIdx = m_encConfigs.size();
        for (int32_t j = 0; j < 3; j++)
        {
            AddEncConfig(m_encRcMode[j]);
        }
        AddProfileEntry(VAProfileVP8Version0_3, VAEntrypointEncSlice, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }
#endif
    return status;
}

VAStatus MediaLibvaCaps::LoadAdvancedDecProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

    return status;
}

VAStatus MediaLibvaCaps::LoadVp9DecProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#ifdef _VP9_DECODE_SUPPORTED
    AttribMap *attributeList;
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelVP9VLDProfile0Decoding8bit420))
    {
        status = CreateDecAttributes(VAProfileVP9Profile0, VAEntrypointVLD, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");

        uint32_t configStartIdx = m_decConfigs.size();
        for (int32_t i = 0; i < 2; i++)
        {
            for (int32_t k = 0; k < 2; k++)
            {
                AddDecConfig(m_decSliceMode[i], VA_ENCRYPTION_TYPE_NONE, m_decProcessMode[k]);
                if (m_isEntryptSupported)
                {
                    uint32_t encrytTypes[3];

                    int32_t numTypes = m_CapsCp->GetEncryptionTypes(VAProfileVP9Profile0,
                            encrytTypes, 3);

                    if (numTypes > 0)
                    {
                        for (int32_t l = 0; l < numTypes; l++)
                        {
                            AddDecConfig(VA_DEC_SLICE_MODE_NORMAL, encrytTypes[l],
                                    m_decProcessMode[k]);
                        }
                    }
                }
            }
        }
        
        AddProfileEntry(VAProfileVP9Profile0, VAEntrypointVLD, attributeList,
                configStartIdx, m_decConfigs.size() - configStartIdx);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrVP9VLD10bProfile2Decoding)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelVP9VLDProfile2Decoding12bit420))
        {
            status = CreateDecAttributes(VAProfileVP9Profile2, VAEntrypointVLD, &attributeList);
            DDI_CHK_RET(status, "Failed to initialize Caps!");

            uint32_t configStartIdx = m_decConfigs.size();
            for (int32_t i = 0; i < 2; i++)
            {
                for (int32_t k = 0; k < 2; k++)
                {
                    AddDecConfig(m_decSliceMode[i], VA_ENCRYPTION_TYPE_NONE, m_decProcessMode[k]);
                    if (m_isEntryptSupported)
                    {

                        uint32_t encrytTypes[3];

                        int32_t numTypes = m_CapsCp->GetEncryptionTypes(VAProfileVP9Profile2,
                                encrytTypes, 3);

                        if (numTypes > 0)
                        {
                            for (int32_t l = 0; l < numTypes; l++)
                            {
                                AddDecConfig(VA_DEC_SLICE_MODE_NORMAL, encrytTypes[l],
                                        m_decProcessMode[k]);
                            }
                        }
                    }
                }
            }
            AddProfileEntry(VAProfileVP9Profile2, VAEntrypointVLD, attributeList,
                    configStartIdx, m_decConfigs.size() - configStartIdx);
        }

        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelVP9VLDProfile1Decoding8bit444))
        {
            status = CreateDecAttributes(VAProfileVP9Profile1, VAEntrypointVLD, &attributeList);
            DDI_CHK_RET(status, "Failed to initialize Caps!");
    
            uint32_t configStartIdx = m_decConfigs.size();
            for (int32_t i = 0; i < 2; i++)
            {   
                for (int32_t k = 0; k < 2; k++)
                {
                    AddDecConfig(m_decSliceMode[i], VA_ENCRYPTION_TYPE_NONE, m_decProcessMode[k]); 
                    if (m_isEntryptSupported)
                    {
                        uint32_t encrytTypes[3];
            
                        int32_t numTypes = m_CapsCp->GetEncryptionTypes(VAProfileVP9Profile1,
                                encrytTypes, 3);
            
                        if (numTypes > 0)
                        {
                            for (int32_t l = 0; l < numTypes; l++)
                            {
                                AddDecConfig(VA_DEC_SLICE_MODE_NORMAL, encrytTypes[l],
                                        m_decProcessMode[k]); 
                            }
                        }
                    }
                }
            }
            AddProfileEntry(VAProfileVP9Profile1, VAEntrypointVLD, attributeList,
                    configStartIdx, m_decConfigs.size() - configStartIdx);
        }

        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelVP9VLDProfile3Decoding10bit444)
                || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrIntelVP9VLDProfile3Decoding12bit444))
        {
            status = CreateDecAttributes(VAProfileVP9Profile3, VAEntrypointVLD, &attributeList);
            DDI_CHK_RET(status, "Failed to initialize Caps!");
    
            uint32_t configStartIdx = m_decConfigs.size();
            for (int32_t i = 0; i < 2; i++)
            {
                for (int32_t k = 0; k < 2; k++)
                {
                    AddDecConfig(m_decSliceMode[i], VA_ENCRYPTION_TYPE_NONE, m_decProcessMode[k]); 
                    if (m_isEntryptSupported)
                    {
                        uint32_t encrytTypes[3];
            
                        int32_t numTypes = m_CapsCp->GetEncryptionTypes(VAProfileVP9Profile3,
                                encrytTypes, 3);
            
                        if (numTypes > 0)
                        {
                            for (int32_t l = 0; l < numTypes; l++)
                            {
                                AddDecConfig(VA_DEC_SLICE_MODE_NORMAL, encrytTypes[l],
                                        m_decProcessMode[k]); 
                            }
                        }
                    }
                }
            }
            AddProfileEntry(VAProfileVP9Profile3, VAEntrypointVLD, attributeList,
                    configStartIdx, m_decConfigs.size() - configStartIdx);
        }
#endif
    return status;
}

VAStatus MediaLibvaCaps::LoadVp9EncProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

    return status;
}

VAStatus MediaLibvaCaps::LoadHevcDecProfileEntrypoints()
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
    
#endif
    return status;
}

VAStatus MediaLibvaCaps::LoadDecProfileEntrypoints(VAProfile profile)
{
    AttribMap *attributeList = nullptr;
    VAStatus status = CreateDecAttributes(profile, VAEntrypointVLD, &attributeList);
    DDI_CHK_RET(status, "Failed to initialize Caps!");

    uint32_t configStartIdx = m_decConfigs.size();
    for (int32_t j = 0; j < 2; j++)
    {
        for (int32_t k = 0; k < 2; k++)
        {
            AddDecConfig(m_decSliceMode[j], VA_ENCRYPTION_TYPE_NONE, m_decProcessMode[k]); 
            if (m_isEntryptSupported)
            {
                uint32_t encrytTypes[3];

                int32_t numTypes = m_CapsCp->GetEncryptionTypes(profile,
                        encrytTypes, 3);

                if (numTypes > 0)
                {
                    for (int32_t l = 0; l < numTypes; l++)
                    {
                        AddDecConfig(m_decSliceMode[j], encrytTypes[l],
                                m_decProcessMode[k]); 
                    }
                }
            }
        }
    }
    AddProfileEntry(profile, VAEntrypointVLD, attributeList,
                configStartIdx, m_decConfigs.size() - configStartIdx);
    return status;
}

VAStatus MediaLibvaCaps::LoadHevcEncProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;
    const uint8_t rcModeSize = (sizeof(m_encRcMode))/(sizeof(m_encRcMode[0]));

#if defined (_HEVC_ENCODE_VME_SUPPORTED) || defined (_HEVC_ENCODE_VDENC_SUPPORTED)
    AttribMap *attributeList = nullptr;

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVC))
    {
        status = CreateEncAttributes(VAProfileHEVCMain, VAEntrypointEncSlice, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");
        DDI_CHK_NULL(attributeList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    
        uint32_t configStartIdx = m_encConfigs.size();

        for (int32_t j = 0; j < rcModeSize; j++)
        {
            AddEncConfig(m_encRcMode[j]);
            AddEncConfig(m_encRcMode[j] | VA_RC_PARALLEL);
        }

        AddProfileEntry(VAProfileHEVCMain, VAEntrypointEncSlice, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);

        status = CreateEncAttributes(VAProfileHEVCMain, VAEntrypointFEI, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");
        DDI_CHK_NULL(attributeList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

        configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP, VA_FEI_FUNCTION_ENC_PAK);

        AddProfileEntry(VAProfileHEVCMain, VAEntrypointFEI, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVC10bit))
    {
        status = CreateEncAttributes(VAProfileHEVCMain10, VAEntrypointEncSlice, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");
        DDI_CHK_NULL(attributeList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    
        uint32_t configStartIdx = m_encConfigs.size();

        for (int32_t j = 0; j < rcModeSize; j++)
        {
            AddEncConfig(m_encRcMode[j]);
            AddEncConfig(m_encRcMode[j] | VA_RC_PARALLEL);
        }

        AddProfileEntry(VAProfileHEVCMain10, VAEntrypointEncSlice, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }
    
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVC12bit))
    {
        status = CreateEncAttributes(VAProfileHEVCMain12, VAEntrypointEncSlice, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");
        DDI_CHK_NULL(attributeList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    
        uint32_t configStartIdx = m_encConfigs.size();

        for (int32_t j = 0; j < rcModeSize; j++)
        {
            AddEncConfig(m_encRcMode[j]);
            AddEncConfig(m_encRcMode[j] | VA_RC_PARALLEL);
        }

        AddProfileEntry(VAProfileHEVCMain12, VAEntrypointEncSlice, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }
    
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVC10bit422))
    {
        status = CreateEncAttributes(VAProfileHEVCMain422_10, VAEntrypointEncSlice, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");
        DDI_CHK_NULL(attributeList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    
        uint32_t configStartIdx = m_encConfigs.size();

        for (int32_t j = 0; j < rcModeSize; j++)
        {
            AddEncConfig(m_encRcMode[j]);
            AddEncConfig(m_encRcMode[j] | VA_RC_PARALLEL);
        }

        AddProfileEntry(VAProfileHEVCMain422_10, VAEntrypointEncSlice, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }
    
    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVC12bit422))
    {
        status = CreateEncAttributes(VAProfileHEVCMain422_12, VAEntrypointEncSlice, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");
        DDI_CHK_NULL(attributeList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    
        uint32_t configStartIdx = m_encConfigs.size();

        for (int32_t j = 0; j < rcModeSize; j++)
        {
            AddEncConfig(m_encRcMode[j]);
            AddEncConfig(m_encRcMode[j] | VA_RC_PARALLEL);
        }

        AddProfileEntry(VAProfileHEVCMain422_12, VAEntrypointEncSlice, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }

#endif
    return status;
}

VAStatus MediaLibvaCaps::LoadNoneProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

    AttribMap *attributeList = nullptr;

    status = CreateDecAttributes(VAProfileNone, VAEntrypointVideoProc, &attributeList);
    DDI_CHK_RET(status, "Failed to initialize Caps!");

    uint32_t configStartIdx = m_vpConfigs.size();
    AddVpConfig(0);
    AddProfileEntry(VAProfileNone, VAEntrypointVideoProc, attributeList, configStartIdx, 1);

    configStartIdx = m_encConfigs.size();
    AddEncConfig(VA_RC_NONE);
    AddProfileEntry(VAProfileNone, VAEntrypointStats, attributeList,
            configStartIdx, 1);
    return status;
}

VAStatus MediaLibvaCaps::GetConfigAttributes(VAProfile profile,
        VAEntrypoint entrypoint,
        VAConfigAttrib *attribList,
        int32_t numAttribs)
{
    DDI_CHK_NULL(attribList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    int32_t i = GetProfileTableIdx(profile, entrypoint);

    switch(i)
    {
        case -2:
            return VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
        case -1:
            return VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
        default:
            break;
    }

    DDI_CHK_NULL(m_profileEntryTbl[i].m_attributes, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    for (int32_t j = 0; j < numAttribs; j++)
    {
        if (m_profileEntryTbl[i].m_attributes->find(attribList[j].type) !=
                m_profileEntryTbl[i].m_attributes->end())
        {
            attribList[j].value = (*m_profileEntryTbl[i].m_attributes)[attribList[j].type];
        }
        else
        {
            //For unknown attribute, set to VA_ATTRIB_NOT_SUPPORTED
            attribList[j].value = VA_ATTRIB_NOT_SUPPORTED;
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCaps::CreateDecConfig(
        int32_t profileTableIdx,
        VAConfigAttrib *attribList,
        int32_t numAttribs,
        VAConfigID *configId)
{
    DDI_CHK_NULL(configId, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (numAttribs)
    {
        DDI_CHK_NULL(attribList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    }

    VAConfigAttrib decAttributes[3];

    decAttributes[0].type = VAConfigAttribDecSliceMode;
    decAttributes[0].value = VA_DEC_SLICE_MODE_NORMAL;
    decAttributes[1].type = VAConfigAttribEncryption;
    decAttributes[1].value = VA_ENCRYPTION_TYPE_NONE;
    decAttributes[2].type = VAConfigAttribDecProcessing;
    decAttributes[2].value = VA_DEC_PROCESSING_NONE;

    int32_t i,j;
    for (j = 0; j < numAttribs; j++)
    {
        for (i = 0; i < 3; i++)
        {
            if (attribList[j].type == decAttributes[i].type)
            {
                decAttributes[i].value = attribList[j].value;
                break;
            }
        }
    }

    int32_t startIdx = m_profileEntryTbl[profileTableIdx].m_configStartIdx;
    int32_t configNum = m_profileEntryTbl[profileTableIdx].m_configNum;
    for (i = startIdx; i < (startIdx + configNum); i++)
    {
        if (decAttributes[0].value == m_decConfigs[i].m_sliceMode
                && decAttributes[1].value == m_decConfigs[i].m_encryptType
                && decAttributes[2].value == m_decConfigs[i].m_processType)
        {
            break;
        }
    }

    if (i < (startIdx + configNum))
    {
        *configId = DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_BASE + i;
        return VA_STATUS_SUCCESS;

    }
    else
    {
        *configId = 0xFFFFFFFF;
        return VA_STATUS_ERROR_ATTR_NOT_SUPPORTED;
    }
}

VAStatus MediaLibvaCaps::CreateEncConfig(
        int32_t profileTableIdx,
        VAEntrypoint entrypoint,
        VAConfigAttrib *attribList,
        int32_t numAttribs,
        VAConfigID *configId)
{
    DDI_CHK_NULL(configId, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (numAttribs)
    {
        DDI_CHK_NULL(attribList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    }

    uint32_t rcMode = VA_RC_CQP;
    if((entrypoint == VAEntrypointStats) || (entrypoint == VAEntrypointEncPicture))
    {
        rcMode = VA_RC_NONE;
    }

    bool rc_mb_flag = false;
    if (entrypoint == VAEntrypointEncSliceLP)
    {
        switch(m_profileEntryTbl[profileTableIdx].m_profile)
        {
            case VAProfileHEVCMain:
            case VAProfileHEVCMain10:
            case VAProfileHEVCMain444:
            case VAProfileHEVCMain444_10:
                rc_mb_flag = true;
                break;
            default:
                rc_mb_flag = false;
                break;
        }
    }

    uint32_t feiFunction = 0;

    int32_t j;
    for (j = 0; j < numAttribs; j++)
    {
        if (VAConfigAttribRateControl == attribList[j].type)
        {
            //do not set VA_RC_MB without other BRC mode
            //if it happend, just set it to default RC mode
            if(attribList[j].value != VA_RC_MB)
            {
                if ((attribList[j].value == VA_RC_CBR ||
                    attribList[j].value == VA_RC_VBR) && rc_mb_flag)
                    rcMode = attribList[j].value | VA_RC_MB;
                else
                    rcMode = attribList[j].value;
            }
        }
        if(VAConfigAttribFEIFunctionType == attribList[j].type)
        {
            feiFunction = attribList[j].value;
        }
        if(VAConfigAttribRTFormat == attribList[j].type)
        {
            VAConfigAttrib attribRT;
            CheckEncRTFormat(m_profileEntryTbl[profileTableIdx].m_profile, entrypoint, &attribRT);
            if((attribList[j].value | attribRT.value) == 0)
            {
                return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
            }
        }
    }

    // If VAEntrypointFEI but FEI type (ENC/PAK/ENCPAK) wasn't provided via VAConfigAttribFEIFunctionType
    // then use ENC_PAK as default
    if (VAEntrypointFEI == entrypoint && 0 == feiFunction)
        feiFunction = VA_FEI_FUNCTION_ENC_PAK;

    int32_t startIdx = m_profileEntryTbl[profileTableIdx].m_configStartIdx;
    int32_t configNum = m_profileEntryTbl[profileTableIdx].m_configNum;
    for (j = startIdx; j < (startIdx + configNum); j++)
    {
        if (m_encConfigs[j].m_rcMode == rcMode &&
            m_encConfigs[j].m_FeiFunction == feiFunction)
        {
            break;
        }
    }

    if (j < (configNum + startIdx))
    {
        *configId = j + DDI_CODEC_GEN_CONFIG_ATTRIBUTES_ENC_BASE;
        return VA_STATUS_SUCCESS;
    }
    else
    {
        *configId = 0xFFFFFFFF;
        return VA_STATUS_ERROR_ATTR_NOT_SUPPORTED;
    }
}

VAStatus MediaLibvaCaps::CreateVpConfig(
        int32_t profileTableIdx,
        VAConfigAttrib *attribList,
        int32_t numAttribs,
        VAConfigID *configId)
{
    // attribList and numAttribs are for future usage.
    DDI_UNUSED(attribList);
    DDI_UNUSED(numAttribs);

    DDI_CHK_NULL(configId, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    *configId = m_profileEntryTbl[profileTableIdx].m_configStartIdx
        + DDI_VP_GEN_CONFIG_ATTRIBUTES_BASE;

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCaps::CheckDecodeResolution(
        int32_t codecMode,
        VAProfile profile,
        uint32_t width,
        uint32_t height)
{

    uint32_t maxWidth = 0;
    uint32_t maxHeight = 0;
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

    uint32_t alignedHeight = 0;
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

VAStatus MediaLibvaCaps::CheckEncodeResolution(
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
                    || height <  m_encJpegMinHeight)
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

VAStatus MediaLibvaCaps::CheckProfile(VAProfile profile)
{
    VAStatus status = VA_STATUS_SUCCESS;
    if (IsVc1Profile(profile))
    {
        MOS_USER_FEATURE       userFeature;
        MOS_USER_FEATURE_VALUE userFeatureValue;
        MOS_ZeroMemory(&userFeatureValue, sizeof(userFeatureValue));
        userFeatureValue.u32Data    = true;
        userFeature.Type            = MOS_USER_FEATURE_TYPE_USER;
        userFeature.pValues         = &userFeatureValue;
        userFeature.uiNumValues     = 1;
        MOS_UserFeature_ReadValue(
            nullptr,
            &userFeature,
            "VC1Enabled",
            MOS_USER_FEATURE_VALUE_TYPE_INT32);

        if (!userFeatureValue.u32Data)
        {
            status = VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
        }

    }
    return status;
}

VAStatus MediaLibvaCaps::CreateConfig(
        VAProfile profile,
        VAEntrypoint entrypoint,
        VAConfigAttrib *attribList,
        int32_t numAttribs,
        VAConfigID *configId)
{

    DDI_CHK_NULL(configId, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    DDI_CHK_RET(CheckProfile(profile),"Failed to check config!");

    int32_t i = GetProfileTableIdx(profile, entrypoint);

    if (i < 0)
    {
        if(i == -2)
        {
            return VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
        }
        else
        {
            return VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
        }
    }

    if (CheckEntrypointCodecType(entrypoint, videoDecode))
    {
        return CreateDecConfig(i, attribList, numAttribs, configId);
    }
    else if(CheckEntrypointCodecType(entrypoint, videoProcess))
    {
        return CreateVpConfig(i, attribList, numAttribs, configId);
    }
    else if(CheckEntrypointCodecType(entrypoint, videoEncode))
    {
        return CreateEncConfig(i,entrypoint,attribList, numAttribs, configId);
    }
    else
    {
        DDI_ASSERTMESSAGE("DDI: Unsupported EntryPoint");
        return VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
    }
}

VAStatus MediaLibvaCaps::QueryConfigProfiles(
        VAProfile *profileList,
        int32_t *numProfiles)
{
    DDI_CHK_NULL(profileList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(numProfiles, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    std::set<int32_t> profiles;
    int32_t i;
    for (i = 0; i < m_profileEntryCount; i++)
    {
        profiles.insert((int32_t)m_profileEntryTbl[i].m_profile);
    }

    std::set<int32_t>::iterator it;
    for (it = profiles.begin(), i = 0; it != profiles.end(); ++it, i++)
    {
        profileList[i] = (VAProfile)*it;
    }

    *numProfiles = i;

    DDI_CHK_CONDITION((i > DDI_CODEC_GEN_MAX_PROFILES),
            "Execeed maximum number of profiles!", VA_STATUS_ERROR_MAX_NUM_EXCEEDED);
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCaps::QueryConfigEntrypoints(
        VAProfile profile,
        VAEntrypoint *entrypointList,
        int32_t *numEntrypoints)
{
    DDI_CHK_NULL(entrypointList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(numEntrypoints, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    int32_t j = 0;
    for (int32_t i = 0; i < m_profileEntryCount; i++)
    {
        if (m_profileEntryTbl[i].m_profile == profile)
        {
            entrypointList[j] = m_profileEntryTbl[i].m_entrypoint;
            j++;
        }
    }
    *numEntrypoints = j;
    DDI_CHK_CONDITION((j == 0), "cant find the profile!", VA_STATUS_ERROR_UNSUPPORTED_PROFILE);    
    /* If the assert fails then GEN_MAX_ENTRYPOINTS needs to be bigger */
    DDI_CHK_CONDITION((j > DDI_CODEC_GEN_MAX_ENTRYPOINTS),
            "Execeed maximum number of profiles!", VA_STATUS_ERROR_MAX_NUM_EXCEEDED);

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCaps::QueryConfigAttributes(
        VAConfigID configId,
        VAProfile *profile,
        VAEntrypoint *entrypoint,
        VAConfigAttrib *attribList,
        int32_t *numAttribs)
{
    DDI_CHK_NULL(profile, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(entrypoint, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(attribList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(numAttribs, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    int32_t profileTableIdx = -1;
    VAStatus status = GetProfileEntrypointFromConfigId(configId, profile, entrypoint, &profileTableIdx);
    DDI_CHK_RET(status, "Invalide config_id!");
    if (profileTableIdx < 0 || profileTableIdx >= m_profileEntryCount)
    {
        return VA_STATUS_ERROR_INVALID_CONFIG;
    }
    auto allAttribsList = m_profileEntryTbl[profileTableIdx].m_attributes;

    DDI_CHK_NULL(allAttribsList, "Null pointer", VA_STATUS_ERROR_INVALID_CONFIG);

    uint32_t j = 0;
    for (auto it = allAttribsList->begin(); it != allAttribsList->end(); ++it)
    {
        if (it->second != VA_ATTRIB_NOT_SUPPORTED)
        {
            attribList[j].type = it->first;
            attribList[j].value = it->second;
            j++;
        }
    }

    *numAttribs = j;

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCaps::GetEncConfigAttr(
        VAConfigID configId,
        VAProfile *profile,
        VAEntrypoint *entrypoint,
        uint32_t *rcMode,
        uint32_t *feiFunction)
{
    DDI_CHK_NULL(profile, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(entrypoint, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(rcMode, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    int32_t profileTableIdx = -1;
    int32_t configOffset = configId - DDI_CODEC_GEN_CONFIG_ATTRIBUTES_ENC_BASE;
    VAStatus status = GetProfileEntrypointFromConfigId(configId, profile, entrypoint, &profileTableIdx);
    DDI_CHK_RET(status, "Invalide config_id!");
    if (profileTableIdx < 0 || profileTableIdx >= m_profileEntryCount)
    {
        return VA_STATUS_ERROR_INVALID_CONFIG;
    }

    int32_t configStart = m_profileEntryTbl[profileTableIdx].m_configStartIdx;
    int32_t configEnd = m_profileEntryTbl[ profileTableIdx].m_configStartIdx
        + m_profileEntryTbl[profileTableIdx].m_configNum;

    if (configOffset < configStart || configOffset > configEnd)
    {
        return VA_STATUS_ERROR_INVALID_CONFIG;
    }
    *rcMode = m_encConfigs[configOffset].m_rcMode;
    *feiFunction = m_encConfigs[configOffset].m_FeiFunction;
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCaps::GetDecConfigAttr(
        VAConfigID configId,
        VAProfile *profile,
        VAEntrypoint *entrypoint,
        uint32_t *sliceMode,
        uint32_t *encryptType,
        uint32_t *processMode)
{
    DDI_CHK_NULL(profile, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(entrypoint, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(sliceMode, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(encryptType, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(processMode, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    int32_t profileTableIdx = -1;
    int32_t configOffset = configId - DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_BASE;
    VAStatus status = GetProfileEntrypointFromConfigId(configId, profile, entrypoint, &profileTableIdx);
    DDI_CHK_RET(status, "Invalide config_id!");
    if (profileTableIdx < 0 || profileTableIdx >= m_profileEntryCount)
    {
        return VA_STATUS_ERROR_INVALID_CONFIG;
    }

    int32_t configStart = m_profileEntryTbl[profileTableIdx].m_configStartIdx;
    int32_t configEnd = m_profileEntryTbl[ profileTableIdx].m_configStartIdx
        + m_profileEntryTbl[profileTableIdx].m_configNum;

    if (configOffset < configStart || configOffset > configEnd)
    {
        return VA_STATUS_ERROR_INVALID_CONFIG;
    }

    if (sliceMode)
    {
        *sliceMode =  m_decConfigs[configOffset].m_sliceMode;
    }

    if (encryptType)
    {
        *encryptType =  m_decConfigs[configOffset].m_encryptType;
    }

    if (processMode)
    {
        *processMode =  m_decConfigs[configOffset].m_processType;
    }
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCaps::GetVpConfigAttr(
        VAConfigID configId,
        VAProfile *profile,
        VAEntrypoint *entrypoint)
{
    DDI_CHK_NULL(profile, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(entrypoint, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    int32_t profileTableIdx = -1;
    int32_t configOffset = configId - DDI_VP_GEN_CONFIG_ATTRIBUTES_BASE;
    VAStatus status = GetProfileEntrypointFromConfigId(configId, profile, entrypoint, &profileTableIdx);
    DDI_CHK_RET(status, "Invalide config_id!");
    if (profileTableIdx < 0 || profileTableIdx >= m_profileEntryCount)
    {
        return VA_STATUS_ERROR_INVALID_CONFIG;
    }

    int32_t configStart = m_profileEntryTbl[profileTableIdx].m_configStartIdx;
    int32_t configEnd = m_profileEntryTbl[ profileTableIdx].m_configStartIdx
        + m_profileEntryTbl[profileTableIdx].m_configNum;

    if (configOffset < configStart || configOffset > configEnd)
    {
        return VA_STATUS_ERROR_INVALID_CONFIG;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCaps::QueryProcessingRate(
        VAConfigID configId,
        VAProcessingRateParameter *procBuf,
        uint32_t *processingRate)
{
    DDI_CHK_NULL(procBuf, "Null procBuf",        VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(processingRate, "Null processingRate", VA_STATUS_ERROR_INVALID_PARAMETER);

    int32_t profileTableIdx = -1;
    VAEntrypoint entrypoint;
    VAProfile profile;
    VAStatus status = GetProfileEntrypointFromConfigId(configId, &profile, &entrypoint, &profileTableIdx);
    DDI_CHK_RET(status, "Invalide config_id!");
    if (profileTableIdx < 0 || profileTableIdx >= m_profileEntryCount)
    {
        return VA_STATUS_ERROR_INVALID_CONFIG;
    }

    PLATFORM platform;
    MEDIA_FEATURE_TABLE skuTable;
    MEDIA_WA_TABLE waTable;
    memset(&platform, 0, sizeof(platform));

    if (MOS_STATUS_SUCCESS != HWInfo_GetGfxInfo(m_mediaCtx->fd, &platform, &skuTable, &waTable, m_mediaCtx->pGtSystemInfo))
    {
        DDI_ASSERTMESSAGE("Fatal error - Cannot get Sku/Wa Tables/GtSystemInfo and Platform information");
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    const int32_t tuIdxTable[] = {7, 6, 5, 4, 3, 2, 1, 0};
    VAProcessingRateParameterEnc *processingRateBuffEnc = nullptr;
    VAProcessingRateParameterDec *processingRateBuffDec = nullptr;
    uint32_t tuIdx = tuIdxTable[TARGETUSAGE_BEST_SPEED];
    VAStatus res = VA_STATUS_SUCCESS;
    CODECHAL_MODE encodeMode = CODECHAL_UNSUPPORTED_MODE;

    if ((entrypoint == VAEntrypointEncSlice) ||
         (entrypoint == VAEntrypointEncSliceLP))
    {
        // Get VAProcessingBufferEnc
        processingRateBuffEnc = &procBuf->proc_buf_enc;

        if (processingRateBuffEnc &&
        processingRateBuffEnc->quality_level < sizeof(tuIdxTable) / sizeof(tuIdxTable[0]))
        {
            // If app passes the rate input data from DDI and also its TU is valid, then use it
            tuIdx = tuIdxTable[processingRateBuffEnc->quality_level];
        }

        if (IsAvcProfile(profile))
        {
            encodeMode = CODECHAL_ENCODE_MODE_AVC;
        }
        else if(IsMpeg2Profile(profile))
        {
            encodeMode = CODECHAL_ENCODE_MODE_MPEG2;
        }
        else if (IsVp8Profile(profile))
        {
            encodeMode = CODECHAL_ENCODE_MODE_VP8;
        }
        else if (IsJpegProfile(profile))
        {
            encodeMode = CODECHAL_ENCODE_MODE_JPEG;
        }
        else if(IsHevcProfile(profile))
        {
            encodeMode = CODECHAL_ENCODE_MODE_HEVC;
        }
        else if (IsVp9Profile(profile))
        {
            encodeMode = CODECHAL_ENCODE_MODE_VP9;
        }

        res = GetMbProcessingRateEnc(
                &skuTable,
                tuIdx,
                encodeMode,
                (entrypoint == VAEntrypointEncSliceLP),
                processingRate);
    }
    else if (entrypoint == VAEntrypointVLD)
    {
        // Get VAProcessingBufferEnc
        processingRateBuffDec = & procBuf->proc_buf_dec;

        res = GetMbProcessingRateDec(
                &skuTable,
                processingRate);
    }
    else // VA_PROCESSING_RATE_NONE or else
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    return res;
}

VAStatus MediaLibvaCaps::QuerySurfaceAttributes(
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

    uint32_t i = 0;

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

        for (uint32_t j = 0; j < m_numVpSurfaceAttr; j++)
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
        attribs[i].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_VA |
            VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR |
            VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM |
            VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;
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
            for (uint32_t j = 0; j < m_numJpegSurfaceAttr; j++)
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
        else if(IsVp9Profile(profile))
        {
            maxWidth = m_decVp9MaxWidth;
            maxHeight = m_decVp9MaxHeight;
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

        attribs[i].type = VASurfaceAttribMemoryType;
        attribs[i].value.type = VAGenericValueTypeInteger;
        attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
        attribs[i].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_VA |
            VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR |
            VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM |
            VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;
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
            for (uint32_t j = 0; j < m_numJpegEncSurfaceAttr; j++)
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
        if(IsAvcProfile(profile)||IsHevcProfile(profile)||IsVp8Profile(profile))
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
        if(IsAvcProfile(profile)||IsHevcProfile(profile)||IsVp8Profile(profile))
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

        attribs[i].type = VASurfaceAttribMemoryType;
        attribs[i].value.type = VAGenericValueTypeInteger;
        attribs[i].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
        attribs[i].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_VA |
            VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR |
            VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM |
            VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;
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

bool MediaLibvaCaps::IsVc1Profile(VAProfile profile)
{
    return (
            (profile == VAProfileVC1Advanced)        ||
            (profile == VAProfileVC1Main)            ||
            (profile == VAProfileVC1Simple)
           );
}

bool MediaLibvaCaps::IsAvcProfile(VAProfile profile)
{
    return (
            (profile == VAProfileH264ConstrainedBaseline) ||
            (profile == VAProfileH264Main) ||
            (profile == VAProfileH264High)
           );
}

bool MediaLibvaCaps::IsMpeg2Profile(VAProfile profile)
{
    return (
            (profile == VAProfileMPEG2Simple) ||
            (profile == VAProfileMPEG2Main)
           );
}

bool MediaLibvaCaps::IsVp8Profile(VAProfile profile)
{
    return (profile == VAProfileVP8Version0_3);
}

bool MediaLibvaCaps::IsVp9Profile(VAProfile profile)
{
    return (
            (profile == VAProfileVP9Profile0) ||
            (profile == VAProfileVP9Profile2) ||
            (profile == VAProfileVP9Profile1) ||
            (profile == VAProfileVP9Profile3)
            );
}

bool MediaLibvaCaps::IsHevcProfile(VAProfile profile)
{
    return (
            (profile == VAProfileHEVCMain)       ||
            (profile == VAProfileHEVCMain10)     ||
            (profile == VAProfileHEVCMain12)     ||
            (profile == VAProfileHEVCMain422_10) ||
            (profile == VAProfileHEVCMain422_12) ||
            (profile == VAProfileHEVCMain444)    ||
            (profile == VAProfileHEVCMain444_10) ||
            (profile == VAProfileHEVCMain444_12)
           );
}

bool MediaLibvaCaps::IsJpegProfile(VAProfile profile)
{
    return (profile == VAProfileJPEGBaseline);
}

bool MediaLibvaCaps::IsEncFei(VAEntrypoint entrypoint, uint32_t feiFunction)
{
    if ((feiFunction & VA_FEI_FUNCTION_ENC_PAK)  ||
            (feiFunction == VA_FEI_FUNCTION_ENC) ||
            (feiFunction == VA_FEI_FUNCTION_PAK) ||
            (feiFunction == (VA_FEI_FUNCTION_ENC | VA_FEI_FUNCTION_PAK)) ||
            (entrypoint == VAEntrypointStats))
    {
        return true;
    }
    return false;
}

CODECHAL_FUNCTION MediaLibvaCaps::GetEncodeCodecFunction(VAProfile profile, VAEntrypoint entrypoint, uint32_t feiFunction)
{
    CODECHAL_FUNCTION codecFunction;
    if (profile == VAProfileJPEGBaseline)
    {
        codecFunction = CODECHAL_FUNCTION_PAK;
    }
    else if (entrypoint == VAEntrypointEncSliceLP)
    {
        codecFunction = CODECHAL_FUNCTION_ENC_VDENC_PAK;
    }
    else
    {
        codecFunction = CODECHAL_FUNCTION_ENC_PAK;
        /*
        //  FeiFunction bit 0: FEI_ENC_INTEL
        //                  1: FEI_PAK_INTEL
        //                  2: FEI_ENC_PAK_INTEL
        //
        //  +---+---+---+
        //  | 2 | 1 | 0 |  FeiFunction [2:0]
        //  +---+---+---+
        //
        //  b000 means ENC_PAK
        */
        if (feiFunction & VA_FEI_FUNCTION_ENC_PAK)
        {
            codecFunction = CODECHAL_FUNCTION_FEI_ENC_PAK;
        }
        else if (feiFunction == VA_FEI_FUNCTION_ENC)
        {
            codecFunction = CODECHAL_FUNCTION_FEI_ENC;
        }
        else if (feiFunction == VA_FEI_FUNCTION_PAK)
        {
            codecFunction = CODECHAL_FUNCTION_FEI_PAK;
        }
        else if (feiFunction == (VA_FEI_FUNCTION_ENC | VA_FEI_FUNCTION_PAK))
        {
            // codecFunction in context keeps FEI_ENC_PAK if input is ENC|PAK
            codecFunction = CODECHAL_FUNCTION_FEI_ENC_PAK;
        }
        else if (entrypoint == VAEntrypointStats)
        {
            codecFunction = CODECHAL_FUNCTION_FEI_PRE_ENC;
        }
    }
    return codecFunction;
}

CODECHAL_MODE MediaLibvaCaps::GetEncodeCodecMode(VAProfile profile, VAEntrypoint entrypoint)
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
            return CODECHAL_ENCODE_MODE_VP9;
        case VAProfileHEVCMain:
        case VAProfileHEVCMain10:
        case VAProfileHEVCMain12:    
        case VAProfileHEVCMain422_10:
        case VAProfileHEVCMain422_12:
            return CODECHAL_ENCODE_MODE_HEVC;
        default:
            DDI_ASSERTMESSAGE("Invalid Encode Mode");
            return CODECHAL_UNSUPPORTED_MODE;
    }
}

CODECHAL_MODE MediaLibvaCaps::GetDecodeCodecMode(VAProfile profile)
{
    switch (profile)
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
            return CODECHAL_DECODE_MODE_HEVCVLD;
        case VAProfileVC1Simple:
        case VAProfileVC1Main:
        case VAProfileVC1Advanced:
                return CODECHAL_DECODE_MODE_VC1VLD;
        default:
            DDI_ASSERTMESSAGE("Invalid Encode Mode");
            return CODECHAL_UNSUPPORTED_MODE;
    }
}

std::string MediaLibvaCaps::GetDecodeCodecKey(VAProfile profile)
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
            return DECODE_ID_HEVC;
        case VAProfileVC1Simple:
        case VAProfileVC1Main:
        case VAProfileVC1Advanced:
            return DECODE_ID_VC1;
        default:
            DDI_ASSERTMESSAGE("Invalid Encode Mode");
            return DECODE_ID_NONE;
    }
}

std::string MediaLibvaCaps::GetEncodeCodecKey(VAProfile profile, VAEntrypoint entrypoint, uint32_t feiFunction)
{
    switch (profile)
    {
        case VAProfileH264High:
        case VAProfileH264Main:
        case VAProfileH264ConstrainedBaseline:
            if (IsEncFei(entrypoint, feiFunction))
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
            return ENCODE_ID_VP9;
        case VAProfileHEVCMain:
        case VAProfileHEVCMain10:
        case VAProfileHEVCMain12:
        case VAProfileHEVCMain422_10:
        case VAProfileHEVCMain422_12:
            if (IsEncFei(entrypoint, feiFunction))
            {
                return ENCODE_ID_HEVCFEI;
            }
            else
            {
                return ENCODE_ID_HEVC;
            }
        case VAProfileNone:
            if (IsEncFei(entrypoint, feiFunction))
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

bool MediaLibvaCaps::IsDecConfigId(VAConfigID configId)
{
    return ((configId >= DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_BASE) &&
            (configId < (DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_BASE + m_decConfigs.size())));
}

bool MediaLibvaCaps::IsEncConfigId(VAConfigID configId)
{
    return ((configId >= DDI_CODEC_GEN_CONFIG_ATTRIBUTES_ENC_BASE) &&
            (configId < (DDI_CODEC_GEN_CONFIG_ATTRIBUTES_ENC_BASE + m_encConfigs.size())));
}

bool MediaLibvaCaps::IsVpConfigId(VAConfigID configId)
{
    return ((configId >= DDI_VP_GEN_CONFIG_ATTRIBUTES_BASE) &&
            (configId < (DDI_VP_GEN_CONFIG_ATTRIBUTES_BASE + m_vpConfigs.size())));
}

bool MediaLibvaCaps::IsMfeSupportedEntrypoint(VAEntrypoint entrypoint)
{
    if (entrypoint != VAEntrypointEncSlice &&           //MFE only support Encode slice
        entrypoint != VAEntrypointFEI )                 //and FEI yet
    {
        return false;
    }

    return true;
}

bool MediaLibvaCaps::IsMfeSupportedProfile(VAProfile profile)
{
    if (profile != VAProfileH264Main &&                  // MFE only support AVC now
        profile != VAProfileH264High &&
        profile != VAProfileH264ConstrainedBaseline)
    {
        return false;
    }

    return true;
}

VAStatus MediaLibvaCaps::DestroyConfig(VAConfigID configId)
{
    if( IsDecConfigId(configId) || IsEncConfigId(configId) || IsVpConfigId(configId))
    {
        return VA_STATUS_SUCCESS;
    }

    return VA_STATUS_ERROR_INVALID_CONFIG;
}

GMM_RESOURCE_FORMAT MediaLibvaCaps::ConvertMediaFmtToGmmFmt(
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
        default                      : return GMM_FORMAT_INVALID;
    }
}

GMM_RESOURCE_FORMAT MediaLibvaCaps::ConvertFourccToGmmFmt(uint32_t fourcc)
{
    switch (fourcc)
    {
        case VA_FOURCC_BGRA   : return GMM_FORMAT_B8G8R8A8_UNORM_TYPE;
        case VA_FOURCC_ARGB   : return GMM_FORMAT_B8G8R8A8_UNORM_TYPE;
        case VA_FOURCC_RGBA   : return GMM_FORMAT_R8G8B8A8_UNORM_TYPE;
        case VA_FOURCC_ABGR   : return GMM_FORMAT_R8G8B8A8_UNORM_TYPE;
        case VA_FOURCC_BGRX   : return GMM_FORMAT_B8G8R8X8_UNORM_TYPE;
        case VA_FOURCC_XRGB   : return GMM_FORMAT_B8G8R8X8_UNORM_TYPE;
        case VA_FOURCC_RGBX   : return GMM_FORMAT_R8G8B8X8_UNORM_TYPE;
        case VA_FOURCC_XBGR   : return GMM_FORMAT_R8G8B8X8_UNORM_TYPE;
        case VA_FOURCC_R8G8B8 : return GMM_FORMAT_R8G8B8_UNORM;
        case VA_FOURCC_RGBP   : return GMM_FORMAT_RGBP;
        case VA_FOURCC_BGRP   : return GMM_FORMAT_BGRP;
        case VA_FOURCC_RGB565 : return GMM_FORMAT_B5G6R5_UNORM_TYPE;
        case VA_FOURCC_AYUV   : return GMM_FORMAT_AYUV_TYPE;
        case VA_FOURCC_NV12   : return GMM_FORMAT_NV12_TYPE;
        case VA_FOURCC_NV21   : return GMM_FORMAT_NV21_TYPE;
        case VA_FOURCC_YUY2   : return GMM_FORMAT_YUY2;
        case VA_FOURCC_UYVY   : return GMM_FORMAT_UYVY;
        case VA_FOURCC_YV12   : return GMM_FORMAT_YV12_TYPE;
        case VA_FOURCC_I420   : return GMM_FORMAT_I420_TYPE;
        case VA_FOURCC_IYUV   : return GMM_FORMAT_IYUV_TYPE;
        case VA_FOURCC_411P   : return GMM_FORMAT_MFX_JPEG_YUV411_TYPE;
        case VA_FOURCC_422H   : return GMM_FORMAT_MFX_JPEG_YUV422H_TYPE;
        case VA_FOURCC_422V   : return GMM_FORMAT_MFX_JPEG_YUV422V_TYPE;
        case VA_FOURCC_444P   : return GMM_FORMAT_MFX_JPEG_YUV444_TYPE;
        case VA_FOURCC_IMC3   : return GMM_FORMAT_IMC3_TYPE;
        case VA_FOURCC_P208   : return GMM_FORMAT_P208_TYPE;
        case VA_FOURCC_P010   : return GMM_FORMAT_P010_TYPE;
        case VA_FOURCC_P016   : return GMM_FORMAT_P016_TYPE;
        case VA_FOURCC_Y210   : return GMM_FORMAT_Y210_TYPE;
        case VA_FOURCC_Y410   : return GMM_FORMAT_Y410_TYPE;
        case VA_FOURCC_Y800   : return GMM_FORMAT_GENERIC_8BIT;
        case VA_FOURCC_A2R10G10B10   : return GMM_FORMAT_R10G10B10A2_UNORM_TYPE;
        case VA_FOURCC_A2B10G10R10   : return GMM_FORMAT_B10G10R10A2_UNORM_TYPE;
        default               : return GMM_FORMAT_INVALID;
    }
}

bool MediaLibvaCaps::IsMfeSupportedOnPlatform(const PLATFORM &platform)
{
    return (GFX_IS_PRODUCT(platform, IGFX_SKYLAKE));
}

VAStatus MediaLibvaCaps::GetMbProcessingRateDec(
        MEDIA_FEATURE_TABLE *skuTable,
        uint32_t *mbProcessingRatePerSec)
{
    uint32_t idx = 0;

    DDI_CHK_NULL(skuTable, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(mbProcessingRatePerSec, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    const uint32_t mb_rate[2] =
    {
        // non-ULX, ULX/Atom
        4800000, 3600000
    };

    if (MEDIA_IS_SKU(skuTable, FtrLCIA) || //Atom
            MEDIA_IS_SKU(skuTable, FtrULX)) // ULX
    {
        idx = 1;
    }
    else
    {
        // Default is non-ULX
        idx = 0;
    }

    *mbProcessingRatePerSec = mb_rate[idx];
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCaps::GetMbProcessingRateEnc(
        MEDIA_FEATURE_TABLE *skuTable,
        uint32_t tuIdx,
        uint32_t codecMode,
        bool vdencActive,
        uint32_t *mbProcessingRatePerSec)
{
    DDI_CHK_NULL(skuTable, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(mbProcessingRatePerSec, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t gtIdx = 0;

    // Calculate the GT index based on GT type
    if (MEDIA_IS_SKU(skuTable, FtrGT1))
    {
        gtIdx = 4;
    }
    else if (MEDIA_IS_SKU(skuTable, FtrGT1_5))
    {
        gtIdx = 3;
    }
    else if (MEDIA_IS_SKU(skuTable, FtrGT2))
    {
        gtIdx = 2;
    }
    else if (MEDIA_IS_SKU(skuTable, FtrGT3))
    {
        gtIdx = 1;
    }
    else if (MEDIA_IS_SKU(skuTable, FtrGT4))
    {
        gtIdx = 0;
    }
    else
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (MEDIA_IS_SKU(skuTable, FtrULX))
    {
        static const uint32_t mbRate[7][5] =
        {
            // GT4 | GT3 |  GT2   | GT1.5  |  GT1
            { 0, 0, 1029393, 1029393, 676280 },
            { 0, 0, 975027, 975027, 661800 },
            { 0, 0, 776921, 776921, 640000 },
            { 0, 0, 776921, 776921, 640000 },
            { 0, 0, 776921, 776921, 640000 },
            { 0, 0, 416051, 416051, 317980 },
            { 0, 0, 214438, 214438, 180655 }
        };

        if (gtIdx == 0 || gtIdx == 1)
        {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        *mbProcessingRatePerSec = mbRate[tuIdx][gtIdx];
    }
    else if (MEDIA_IS_SKU(skuTable, FtrULT))
    {
        static const uint32_t defaultult_mb_rate[7][5] =
        {
            // GT4    | GT3   |  GT2   | GT1.5   |  GT1
            { 1544090, 1544090, 1544090, 1029393, 676280 },
            { 1462540, 1462540, 1462540, 975027, 661800 },
            { 1165381, 1165381, 1165381, 776921, 640000 },
            { 1165381, 1165381, 1165381, 776921, 640000 },
            { 1165381, 1165381, 1165381, 776921, 640000 },
            { 624076, 624076, 624076, 416051, 317980 },
            { 321657, 321657, 321657, 214438, 180655 }
        };

        *mbProcessingRatePerSec = defaultult_mb_rate[tuIdx][gtIdx];
    }
    else
    {
        // regular
        const uint32_t default_mb_rate[7][5] =
        {
            // GT4    | GT3   |   GT2  | GT1.5  |  GT1
            { 1544090, 1544090, 1544090, 1029393, 676280 },
            { 1462540, 1462540, 1462540, 975027, 661800 },
            { 1165381, 1165381, 1165381, 776921, 640000 },
            { 1165381, 1165381, 1165381, 776921, 640000 },
            { 1165381, 1165381, 1165381, 776921, 640000 },
            { 624076, 624076, 624076, 416051, 317980 },
            { 321657, 321657, 321657, 214438, 180655 }
        };

        *mbProcessingRatePerSec = default_mb_rate[tuIdx][gtIdx];
    }
    return VA_STATUS_SUCCESS;
}

MediaLibvaCaps * MediaLibvaCaps::CreateMediaLibvaCaps(DDI_MEDIA_CONTEXT *mediaCtx)
{
    if (mediaCtx != nullptr)
    {

        MediaLibvaCaps * Caps = CapsFactory::CreateCaps(
            (uint32_t)mediaCtx->platform.eProductFamily + MEDIA_EXT_FLAG, mediaCtx);

        if(Caps == nullptr)
        {
            Caps = CapsFactory::CreateCaps((uint32_t)mediaCtx->platform.eProductFamily, mediaCtx);
        }

        return Caps;

    }
    else
    {
        return nullptr;
    }
}
