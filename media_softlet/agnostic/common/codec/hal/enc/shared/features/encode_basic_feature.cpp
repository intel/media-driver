/*
* Copyright (c) 2018-2022, Intel Corporation
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
//! \file     encode_basic_feature.cpp
//! \brief    Defines the common interface for encode basic feature
//!

#include "encode_basic_feature.h"
#include "encode_utils.h"
#include "encode_allocator.h"
#include "mos_interface.h"
#include "codec_def_common_encode.h"
#include "codec_def_encode_vp9.h"
#include "codec_def_encode_hevc.h"

namespace encode
{
EncodeBasicFeature::EncodeBasicFeature(
    EncodeAllocator *allocator,
    CodechalHwInterfaceNext *hwInterface,
    TrackedBuffer *trackedBuf,
    RecycleResource *recycleBuf):
    MediaFeature(hwInterface ? hwInterface->GetOsInterface() : nullptr),
    m_trackedBuf(trackedBuf),
    m_recycleBuf(recycleBuf),
    m_allocator(allocator)
{
    if(hwInterface)
    {
        m_osInterface = hwInterface->GetOsInterface();
    }

}

MOS_STATUS EncodeBasicFeature::Init(void *setting)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(setting);

    CodechalSetting *codecSettings = (CodechalSetting*)setting;
    m_standard = codecSettings->standard;
    m_mode = codecSettings->mode;
    m_codecFunction = codecSettings->codecFunction;

    m_is10Bit       = codecSettings->lumaChromaDepth == CODECHAL_LUMA_CHROMA_DEPTH_10_BITS ? true : false;
    m_chromaFormat  = codecSettings->chromaFormat;
    m_bitDepth      = codecSettings->lumaChromaDepth == CODECHAL_LUMA_CHROMA_DEPTH_8_BITS ?
                        8 : (codecSettings->lumaChromaDepth == CODECHAL_LUMA_CHROMA_DEPTH_10_BITS ? 10 : 12);
    m_standard      = codecSettings->standard;

    m_oriFrameWidth   = codecSettings->width;
    m_oriFrameHeight  = codecSettings->height;
    m_picWidthInMb    = (uint16_t)CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_oriFrameWidth);
    m_picHeightInMb   = (uint16_t)CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_oriFrameHeight);
    m_frameWidth      = m_picWidthInMb * CODECHAL_MACROBLOCK_WIDTH;
    m_frameHeight     = m_picHeightInMb * CODECHAL_MACROBLOCK_HEIGHT;

    m_currOriginalPic.PicFlags = PICTURE_INVALID;
    m_currOriginalPic.FrameIdx = 0;
    m_currOriginalPic.PicEntry = 0;

    //RCPanic settings
    MediaUserSetting::Value outValue;
    ReadUserSetting(
        m_userSettingPtr,
        outValue,
        "RC Panic Mode",
        MediaUserSetting::Group::Sequence);
    m_panicEnable = outValue.Get<bool>();

    ReadUserSetting(
        m_userSettingPtr,
        outValue,
        "HEVC Encode Enable HW Stitch",
        MediaUserSetting::Group::Sequence);
    m_enableTileStitchByHW = outValue.Get<bool>();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeBasicFeature::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_NULL_RETURN(m_allocator);

    EncoderParams* encodeParams = (EncoderParams*)params;

    m_newSeq                = encodeParams->bNewSeq;           // used by all except JPEG
    m_mbDataBufferSize      = encodeParams->dwMbDataBufferSize;// used by all except JPEG
    m_newVuiData            = encodeParams->bNewVuiData;       // used by AVC and MPEG2
    m_picQuant              = encodeParams->bPicQuant ;        // used by AVC and MPEG2
    m_newQmatrixData        = encodeParams->bNewQmatrixData;   // used by AVC and MPEG2
    m_numSlices             = encodeParams->dwNumSlices;       // used by all except VP9
    m_slcData               = (PCODEC_ENCODER_SLCDATA)(encodeParams->pSlcHeaderData);// used by AVC, MPEG2, and HEVC

    ENCODE_CHK_NULL_RETURN(encodeParams->psRawSurface);
    m_rawSurface           = *(encodeParams->psRawSurface);           // used by all
    m_allocator->GetSurfaceInfo(&m_rawSurface);
    ENCODE_CHK_STATUS_RETURN(m_allocator->UpdateResourceUsageType(&m_rawSurface.OsResource, MOS_HW_RESOURCE_USAGE_ENCODE_INPUT_RAW));

    ENCODE_CHK_NULL_RETURN(encodeParams->presBitstreamBuffer);
    m_resBitstreamBuffer    = *(encodeParams->presBitstreamBuffer);   // used by all

    m_resMetadataBuffer    = (encodeParams->presMetadataBuffer);
    m_metaDataOffset       = encodeParams->metaDataOffset;

    // Get resource details of the bitstream resource
    MOS_SURFACE bsSurface;
    MOS_ZeroMemory(&bsSurface, sizeof(bsSurface));
    bsSurface.OsResource = m_resBitstreamBuffer;
    m_allocator->GetSurfaceInfo(&bsSurface);
    ENCODE_CHK_STATUS_RETURN(m_allocator->UpdateResourceUsageType(&m_resBitstreamBuffer, MOS_HW_RESOURCE_USAGE_ENCODE_OUTPUT_BITSTREAM));

    m_bitstreamSize = encodeParams->dwBitstreamSize = bsSurface.dwHeight * bsSurface.dwWidth;

    if (Mos_ResourceIsNull(&m_rawSurface.OsResource))
    {
        ENCODE_ASSERTMESSAGE("Raw surface is nullptr!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    ReportUserSettingForDebug(
        m_userSettingPtr,
        "Encode Raw Surface Tile",
        m_rawSurface.TileType,
        MediaUserSetting::Group::Sequence);
#endif

    m_rawSurfaceToEnc     =
    m_rawSurfaceToPak     = &m_rawSurface;

    if (encodeParams->psReconSurface != nullptr)
    {
        m_reconSurface     = *(encodeParams->psReconSurface);         // used by all except JPEG
        ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(&m_reconSurface));
        ENCODE_CHK_STATUS_RETURN(m_allocator->UpdateResourceUsageType(&m_reconSurface.OsResource, MOS_HW_RESOURCE_USAGE_ENCODE_OUTPUT_PICTURE));
    }

    if (encodeParams->pBSBuffer != nullptr)
    {
        m_bsBuffer          = *(encodeParams->pBSBuffer);              // used by all except VP9
    }

    m_mbDisableSkipMapEnabled          = encodeParams->bMbDisableSkipMapEnabled;
    m_mbQpDataEnabled                  = encodeParams->bMbQpDataEnabled;

    if (encodeParams->bMbQpDataEnabled && encodeParams->psMbQpDataSurface != nullptr)
    {
        m_mbQpDataSurface = *(encodeParams->psMbQpDataSurface);
        ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(&m_mbQpDataSurface));
    }

    if (encodeParams->psMbDisableSkipMapSurface != nullptr)
    {
        m_mbDisableSkipMapSurface = *(encodeParams->psMbDisableSkipMapSurface);
        ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(&m_mbDisableSkipMapSurface));
    }

    m_slcData               =
        (PCODEC_ENCODER_SLCDATA)(encodeParams->pSlcHeaderData);             // used by AVC, MPEG2, and HEVC

    m_newSeqHeader = encodeParams->newSeqHeader;
    m_newPpsHeader = encodeParams->newPpsHeader;

    if (CodecHalUsesVideoEngine(m_codecFunction))
    {
        //UpdateBitstreamSize()
        // Get resource details of the bitstream resource
        MOS_SURFACE details;
        MOS_ZeroMemory(&details, sizeof(details));
        details.Format = Format_Invalid;
        ENCODE_CHK_STATUS_RETURN(
            m_osInterface->pfnGetResourceInfo(m_osInterface, &m_resBitstreamBuffer, &details));
        m_bitstreamSize = details.dwHeight * details.dwWidth;
    }

    // check output Chroma format
    UpdateFormat(params);

    m_predicationNotEqualZero              = encodeParams->m_predicationNotEqualZero;
    m_predicationEnabled                   = encodeParams->m_predicationEnabled;
    m_setMarkerEnabled                     = encodeParams->m_setMarkerEnabled;
    m_predicationResOffset                 = encodeParams->m_predicationResOffset;
    m_presPredication                      = encodeParams->m_presPredication;
    m_tempPredicationBuffer                = encodeParams->m_tempPredicationBuffer;

    if (m_predicationBuffer == nullptr && m_predicationEnabled)
    {
        // initiate allocation parameters and lock flags
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type         = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType     = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format       = Format_Buffer;
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        allocParamsForBufferLinear.dwBytes      = sizeof(uint32_t);
        allocParamsForBufferLinear.pBufName     = "PredicationBuffer";
        m_predicationBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeBasicFeature::UpdateFormat(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);
    EncoderParams* encodeParams = (EncoderParams*)params;

    PCODEC_VP9_ENCODE_SEQUENCE_PARAMS vp9SeqParams = nullptr;
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams = nullptr;

        // check output Chroma format
    switch (m_standard)
    {
    case CODECHAL_HEVC:
        hevcSeqParams =  static_cast<PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
        ENCODE_CHK_NULL_RETURN(hevcSeqParams);
        m_outputChromaFormat = hevcSeqParams->chroma_format_idc;
        break;

    case CODECHAL_VP9:
        // check output Chroma format
        vp9SeqParams = static_cast<PCODEC_VP9_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
        if (VP9_ENCODED_CHROMA_FORMAT_YUV420 == vp9SeqParams->SeqFlags.fields.EncodedFormat)
        {
            m_outputChromaFormat = HCP_CHROMA_FORMAT_YUV420;
        }
        else if (VP9_ENCODED_CHROMA_FORMAT_YUV422 == vp9SeqParams->SeqFlags.fields.EncodedFormat)
        {
            m_outputChromaFormat = HCP_CHROMA_FORMAT_YUV422;
        }
        else if (VP9_ENCODED_CHROMA_FORMAT_YUV444 == vp9SeqParams->SeqFlags.fields.EncodedFormat)
        {
            m_outputChromaFormat = HCP_CHROMA_FORMAT_YUV444;
        }
        else
        {
            ENCODE_ASSERTMESSAGE("Invalid output chromat format in VP9 Seq param!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        break;

    default:
        break;
    }

    if (m_outputChromaFormat == HCP_CHROMA_FORMAT_YUV422)
    {
        if (Format_YUY2 != m_reconSurface.Format && Format_Y216 != m_reconSurface.Format)
        {
            ENCODE_ASSERTMESSAGE("Recon surface format is not correct!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        else if (m_reconSurface.dwHeight < m_oriFrameHeight * 2 ||
                 m_reconSurface.dwWidth < m_oriFrameWidth / 2)
        {
            ENCODE_ASSERTMESSAGE("Recon surface allocation size is not correct!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeBasicFeature::UpdateTrackedBufferParameters()
{
    ENCODE_CHK_NULL_RETURN(m_trackedBuf);

    MOS_ALLOC_GFXRES_PARAMS allocParamsForLinear;
    MOS_ZeroMemory(&allocParamsForLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForLinear.Format   = Format_Buffer;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_Y;
    allocParamsForBuffer2D.Format   = Format_NV12;
    allocParamsForBuffer2D.Flags.bNotLockable = allocParamsForLinear.Flags.bNotLockable 
        = ((m_standard == CODECHAL_AV1) && !m_lockableResource);
    
    if (m_mbCodeSize > 0 && !m_isMbCodeRegistered)
    {
        allocParamsForLinear.pBufName = "mbCodeBuffer";
        // Must reserve at least 8 cachelines after MI_BATCH_BUFFER_END_CMD since HW prefetch max 8 cachelines from BB everytime
        // + 8 * CODECHAL_CACHELINE_SIZE is inherient from legacy code
        allocParamsForLinear.dwBytes = m_mbCodeSize + 8 * CODECHAL_CACHELINE_SIZE;
        allocParamsForLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::mbCodedBuffer, allocParamsForLinear));
    }

    if (m_mvDataSize > 0)
    {
        allocParamsForLinear.pBufName = "mvDataBuffer";
        allocParamsForLinear.dwBytes  = m_mvDataSize;
        ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::mvDataBuffer, allocParamsForLinear));
    }

    if (m_downscaledWidth4x > 0 && m_downscaledHeight4x > 0)
    {
        allocParamsForBuffer2D.dwWidth  = m_downscaledWidth4x;
        allocParamsForBuffer2D.dwHeight = m_downscaledHeight4x;
        allocParamsForBuffer2D.pBufName = "4xDSSurface";
        allocParamsForBuffer2D.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::ds4xSurface, allocParamsForBuffer2D));

        allocParamsForBuffer2D.dwWidth  = m_downscaledWidth4x >> 1;
        allocParamsForBuffer2D.dwHeight = MOS_ALIGN_CEIL(m_downscaledHeight4x >> 1, MOS_YTILE_H_ALIGNMENT) << 1;
        allocParamsForBuffer2D.pBufName = "8xDSSurface";
        allocParamsForBuffer2D.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::ds8xSurface, allocParamsForBuffer2D));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeBasicFeature::Reset(CODEC_REF_LIST *refList)
{
    ENCODE_CHK_NULL_RETURN(refList);
    ENCODE_CHK_NULL_RETURN(m_trackedBuf);

    m_trackedBuf->Release(refList);
    return MOS_STATUS_SUCCESS;
}
}  // namespace encode
