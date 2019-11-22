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
//! \file     media_ddi_encode_hevc.cpp
//! \brief    HEVC class definition for DDI media encoder.
//!

#include "media_ddi_base.h"
#include "media_ddi_encode_base.h"
#include "media_ddi_encode_hevc.h"
#include "media_libva_util.h"
#include "media_ddi_encode_const.h"
#include "media_ddi_factory.h"
#include "codechal_encoder_base.h"

extern template class MediaDdiFactoryNoArg<DdiEncodeBase>;

static bool isEncodeHevcRegistered =
    MediaDdiFactoryNoArg<DdiEncodeBase>::RegisterCodec<DdiEncodeHevc>(ENCODE_ID_HEVC);

DdiEncodeHevc::~DdiEncodeHevc()
{
    if (m_encodeCtx == nullptr)
    {
        return;
    }

    MOS_FreeMemory(m_encodeCtx->pSeqParams);
    m_encodeCtx->pSeqParams = nullptr;

    MOS_FreeMemory(((PCODEC_HEVC_ENCODE_PICTURE_PARAMS)m_encodeCtx->pPicParams)->pDirtyRect);
    ((PCODEC_HEVC_ENCODE_PICTURE_PARAMS)m_encodeCtx->pPicParams)->pDirtyRect = nullptr;
    MOS_FreeMemory(m_encodeCtx->pPicParams);
    m_encodeCtx->pPicParams = nullptr;

    if (m_encodeCtx->ppNALUnitParams)
    {
        // Allocate one contiguous memory for the NALUnitParams buffers
        // only need to free one time
        MOS_FreeMemory(m_encodeCtx->ppNALUnitParams[0]);
        m_encodeCtx->ppNALUnitParams[0] = nullptr;

        MOS_FreeMemory(m_encodeCtx->ppNALUnitParams);
        m_encodeCtx->ppNALUnitParams = nullptr;
    }

    MOS_FreeMemory(m_encodeCtx->pSliceParams);
    m_encodeCtx->pSliceParams = nullptr;

    MOS_FreeMemory(m_encodeCtx->pEncodeStatusReport);
    m_encodeCtx->pEncodeStatusReport = nullptr;

    MOS_FreeMemory(m_encodeCtx->pSEIFromApp->pSEIBuffer);
    m_encodeCtx->pSEIFromApp->pSEIBuffer = nullptr;
    MOS_FreeMemory(m_encodeCtx->pSEIFromApp);
    m_encodeCtx->pSEIFromApp = nullptr;

    MOS_FreeMemory(m_encodeCtx->pSliceHeaderData);
    m_encodeCtx->pSliceHeaderData = nullptr;

    if (m_encodeCtx->pbsBuffer)
    {
        MOS_FreeMemory(m_encodeCtx->pbsBuffer->pBase);
        m_encodeCtx->pbsBuffer->pBase = nullptr;

        MOS_FreeMemory(m_encodeCtx->pbsBuffer);
        m_encodeCtx->pbsBuffer = nullptr;
    }
}

VAStatus DdiEncodeHevc::ContextInitialize(
    CodechalSetting *codecHalSettings)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_encodeCtx->pCpDdiInterface, "nullptr m_encodeCtx->pCpDdiInterface.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(codecHalSettings, "nullptr codecHalSettings.", VA_STATUS_ERROR_INVALID_CONTEXT);

    if (true == m_encodeCtx->bVdencActive)
    {
        codecHalSettings->codecFunction = CODECHAL_FUNCTION_ENC_VDENC_PAK;
        codecHalSettings->disableUltraHME = false;
        codecHalSettings->disableSuperHME = false;
    }
    else
    {
        codecHalSettings->codecFunction = CODECHAL_FUNCTION_ENC_PAK;
    }
    codecHalSettings->height          = m_encodeCtx->dworiFrameHeight;
    codecHalSettings->width           = m_encodeCtx->dworiFrameWidth;
    codecHalSettings->mode            = m_encodeCtx->wModeType;
    codecHalSettings->standard        = CODECHAL_HEVC;

    if(m_encodeCtx->vaProfile==VAProfileHEVCMain)
    {
       codecHalSettings->chromaFormat    = HCP_CHROMA_FORMAT_YUV420;
       codecHalSettings->lumaChromaDepth = CODECHAL_LUMA_CHROMA_DEPTH_8_BITS;
    }
    else if(m_encodeCtx->vaProfile==VAProfileHEVCMain10)
    {
       codecHalSettings->chromaFormat    = HCP_CHROMA_FORMAT_YUV420;
       codecHalSettings->lumaChromaDepth = CODECHAL_LUMA_CHROMA_DEPTH_10_BITS;
    }
    else if(m_encodeCtx->vaProfile == VAProfileHEVCMain12)
    {
       codecHalSettings->chromaFormat    = HCP_CHROMA_FORMAT_YUV420;
       codecHalSettings->lumaChromaDepth = CODECHAL_LUMA_CHROMA_DEPTH_12_BITS;
    }
    else if(m_encodeCtx->vaProfile == VAProfileHEVCMain422_10)
    {
        codecHalSettings->chromaFormat    = HCP_CHROMA_FORMAT_YUV422;
        codecHalSettings->lumaChromaDepth = CODECHAL_LUMA_CHROMA_DEPTH_10_BITS;
    }    
    else if(m_encodeCtx->vaProfile == VAProfileHEVCMain422_12)
    {
        codecHalSettings->chromaFormat    = HCP_CHROMA_FORMAT_YUV422;
        codecHalSettings->lumaChromaDepth = CODECHAL_LUMA_CHROMA_DEPTH_12_BITS;
    }
    else if (m_encodeCtx->vaProfile == VAProfileHEVCMain444)
    {
        codecHalSettings->chromaFormat = HCP_CHROMA_FORMAT_YUV444;
        codecHalSettings->lumaChromaDepth = CODECHAL_LUMA_CHROMA_DEPTH_8_BITS;
    }
    else if (m_encodeCtx->vaProfile == VAProfileHEVCMain444_10)
    {
        codecHalSettings->chromaFormat = HCP_CHROMA_FORMAT_YUV444;
        codecHalSettings->lumaChromaDepth = CODECHAL_LUMA_CHROMA_DEPTH_10_BITS;
    }
    else if (m_encodeCtx->vaProfile == VAProfileHEVCMain444_12)
    {
        codecHalSettings->chromaFormat = HCP_CHROMA_FORMAT_YUV444;
        codecHalSettings->lumaChromaDepth = CODECHAL_LUMA_CHROMA_DEPTH_12_BITS;
    }

    VAStatus eStatus = VA_STATUS_SUCCESS;

    // Allocate sequence params
    m_encodeCtx->pSeqParams = (void *)MOS_AllocAndZeroMemory(sizeof(CODEC_HEVC_ENCODE_SEQUENCE_PARAMS));
    DDI_CHK_NULL(m_encodeCtx->pSeqParams, "nullptr m_encodeCtx->pSeqParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Allocate picture params
    m_encodeCtx->pPicParams = (void *)MOS_AllocAndZeroMemory(sizeof(CODEC_HEVC_ENCODE_PICTURE_PARAMS));
    DDI_CHK_NULL(m_encodeCtx->pPicParams, "nullptr m_encodeCtx->pPicParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Allocate NAL unit params
    m_encodeCtx->ppNALUnitParams = (PCODECHAL_NAL_UNIT_PARAMS *)MOS_AllocAndZeroMemory(sizeof(PCODECHAL_NAL_UNIT_PARAMS) * HEVC_MAX_NAL_UNIT_TYPE);
    DDI_CHK_NULL(m_encodeCtx->ppNALUnitParams, "nullptr m_encodeCtx->ppNALUnitParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    PCODECHAL_NAL_UNIT_PARAMS nalUnitParams = (CODECHAL_NAL_UNIT_PARAMS *)MOS_AllocAndZeroMemory(sizeof(CODECHAL_NAL_UNIT_PARAMS) * HEVC_MAX_NAL_UNIT_TYPE);
    DDI_CHK_NULL(nalUnitParams, "nullptr nalUnitParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    for (uint32_t i = 0; i < HEVC_MAX_NAL_UNIT_TYPE; i++)
    {
        m_encodeCtx->ppNALUnitParams[i] = &(nalUnitParams[i]);
    }

    // Allocate SliceParams
    // Supports one LCU per slice, with minimum LCU size of 16x16
    m_encodeCtx->pSliceParams = (void *)MOS_AllocAndZeroMemory(m_encodeCtx->wPicHeightInMB * m_encodeCtx->wPicWidthInMB * sizeof(CODEC_HEVC_ENCODE_SLICE_PARAMS));
    DDI_CHK_NULL(m_encodeCtx->pSliceParams, "nullptr m_encodeCtx->pSliceParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Allocate Encode Status Report
    m_encodeCtx->pEncodeStatusReport = (void *)MOS_AllocAndZeroMemory(CODECHAL_ENCODE_STATUS_NUM * sizeof(EncodeStatusReport));
    DDI_CHK_NULL(m_encodeCtx->pEncodeStatusReport, "nullptr m_encodeCtx->pEncodeStatusReport.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Allocate SEI structure
    m_encodeCtx->pSEIFromApp = (CodechalEncodeSeiData *)MOS_AllocAndZeroMemory(sizeof(CodechalEncodeSeiData));
    DDI_CHK_NULL(m_encodeCtx->pSEIFromApp, "nullptr m_encodeCtx->pSEIFromApp.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // for slice header from application
    m_encodeCtx->pSliceHeaderData = (CODEC_ENCODER_SLCDATA *)MOS_AllocAndZeroMemory(m_encodeCtx->wPicHeightInMB * m_encodeCtx->wPicWidthInMB * sizeof(CODEC_ENCODER_SLCDATA));
    DDI_CHK_NULL(m_encodeCtx->pSliceHeaderData, "nullptr m_encodeCtx->pSliceHeaderData.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Create the bit stream buffer to hold the packed headers from application
    m_encodeCtx->pbsBuffer = (BSBuffer *)MOS_AllocAndZeroMemory(sizeof(BSBuffer));
    DDI_CHK_NULL(m_encodeCtx->pbsBuffer, "nullptr m_encodeCtx->pbsBuffer.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pbsBuffer->BufferSize = m_encodeCtx->wPicHeightInMB * m_encodeCtx->wPicWidthInMB * PACKED_HEADER_SIZE_PER_ROW;
    m_encodeCtx->pbsBuffer->pBase      = (uint8_t *)MOS_AllocAndZeroMemory(m_encodeCtx->pbsBuffer->BufferSize);
    DDI_CHK_NULL(m_encodeCtx->pbsBuffer->pBase, "nullptr m_encodeCtx->pbsBuffer->pBase.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    return eStatus;
}

VAStatus DdiEncodeHevc::RenderPicture(VADriverContextP ctx, VAContextID context, VABufferID *buffers, int32_t numBuffers)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr context", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_MEDIA_CONTEXT *mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    for (int32_t i = 0; i < numBuffers; i++)
    {
        DDI_MEDIA_BUFFER *buf = DdiMedia_GetBufferFromVABufferID(mediaCtx, buffers[i]);
        DDI_CHK_NULL(buf, "Invalid buffer.", VA_STATUS_ERROR_INVALID_BUFFER);
        if (buf->uiType == VAEncMacroblockDisableSkipMapBufferType)
        {
            DdiMedia_MediaBufferToMosResource(buf, &(m_encodeCtx->resPerMBSkipMapBuffer));
            m_encodeCtx->bMbDisableSkipMapEnabled = true;
            continue;
        }
        uint32_t dataSize = buf->iSize;
        // can use internal function instead of DdiMedia_MapBuffer here?
        void *data = nullptr;
        DdiMedia_MapBuffer(ctx, buffers[i], &data);

        DDI_CHK_NULL(data, "nullptr data.", VA_STATUS_ERROR_INVALID_BUFFER);

        switch (buf->uiType)
        {
        case VAEncSequenceParameterBufferType:
            DDI_CHK_STATUS(ParseSeqParams(data), VA_STATUS_ERROR_INVALID_BUFFER);
            m_encodeCtx->bNewSeq = true;
            break;

        case VAEncPictureParameterBufferType:
            DDI_CHK_STATUS(ParsePicParams(mediaCtx, data), VA_STATUS_ERROR_INVALID_BUFFER);
            DDI_CHK_STATUS(
                    AddToStatusReportQueue((void *)m_encodeCtx->resBitstreamBuffer.bo),
                    VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncSliceParameterBufferType:
        {
            uint32_t numSlices = buf->uiNumElements;
            DDI_CHK_STATUS(ParseSlcParams(mediaCtx, data, numSlices), VA_STATUS_ERROR_INVALID_BUFFER);
            break;
        }

        case VAEncPackedHeaderParameterBufferType:
            vaStatus = ParsePackedHeaderParams(data);
            break;

        case VAEncPackedHeaderDataBufferType:
            vaStatus = ParsePackedHeaderData(data);
            break;

        case VAEncMiscParameterBufferType:
            DDI_CHK_STATUS(ParseMiscParams(data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncQPBufferType:
            DdiMedia_MediaBufferToMosResource(buf, &m_encodeCtx->resMBQpBuffer);
            m_encodeCtx->bMBQpEnable = true;
            break;

        default:
            DDI_ASSERTMESSAGE("not supported buffer type.");
            break;
        }
        DdiMedia_UnmapBuffer(ctx, buffers[i]);
    }

    DDI_FUNCTION_EXIT(vaStatus);
    return vaStatus;
}

VAStatus DdiEncodeHevc::EncodeInCodecHal(uint32_t numSlices)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx->pCodecHal, "nullptr m_encodeCtx->pCodecHal", VA_STATUS_ERROR_INVALID_PARAMETER);

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_encodeCtx->RTtbl);

    EncoderParams encodeParams;
    MOS_ZeroMemory(&encodeParams, sizeof(encodeParams));

    if (m_encodeCtx->bVdencActive)
    {
        encodeParams.ExecCodecFunction = CODECHAL_FUNCTION_ENC_VDENC_PAK;
    }
    else
    {
        encodeParams.ExecCodecFunction = CODECHAL_FUNCTION_ENC_PAK;
    }

    // Raw Surface
    MOS_SURFACE rawSurface;
    MOS_ZeroMemory(&rawSurface, sizeof(rawSurface));
    rawSurface.dwOffset = 0;

    DdiMedia_MediaSurfaceToMosResource(rtTbl->pCurrentRT, &(rawSurface.OsResource));

    // Recon Surface
    MOS_SURFACE reconSurface;
    MOS_ZeroMemory(&reconSurface, sizeof(reconSurface));
    reconSurface.dwOffset = 0;

    DdiMedia_MediaSurfaceToMosResource(rtTbl->pCurrentReconTarget, &(reconSurface.OsResource));

    //clear registered recon/ref surface flags
    DDI_CHK_RET(ClearRefList(&m_encodeCtx->RTtbl, true), "ClearRefList failed!");

    // Bitstream surface
    MOS_RESOURCE bitstreamSurface;
    MOS_ZeroMemory(&bitstreamSurface, sizeof(bitstreamSurface));
    bitstreamSurface        = m_encodeCtx->resBitstreamBuffer;  // in render picture
    bitstreamSurface.Format = Format_Buffer;

    encodeParams.psRawSurface        = &rawSurface;
    encodeParams.psReconSurface      = &reconSurface;
    encodeParams.presBitstreamBuffer = &bitstreamSurface;

    MOS_SURFACE mbQpSurface;
    if (m_encodeCtx->bMBQpEnable)
    {
        // MBQp surface
        MOS_ZeroMemory(&mbQpSurface, sizeof(mbQpSurface));
        mbQpSurface.Format     = Format_Buffer_2D;
        mbQpSurface.dwOffset   = 0;
        mbQpSurface.OsResource = m_encodeCtx->resMBQpBuffer;

        encodeParams.psMbQpDataSurface = &mbQpSurface;
        encodeParams.bMbQpDataEnabled  = true;
    }

    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams = (PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS)((uint8_t *)m_encodeCtx->pSeqParams);
    hevcSeqParams->TargetUsage = m_encodeCtx->targetUsage;
    encodeParams.pSeqParams   = m_encodeCtx->pSeqParams;
    encodeParams.pVuiParams   = m_encodeCtx->pVuiParams;
    encodeParams.pPicParams   = m_encodeCtx->pPicParams;
    encodeParams.pSliceParams = m_encodeCtx->pSliceParams;

    // Sequence data
    encodeParams.bNewSeq = m_encodeCtx->bNewSeq;

    // VUI
    encodeParams.bNewVuiData = m_encodeCtx->bNewVuiData;

    // Slice level data
    encodeParams.dwNumSlices = numSlices;

    // IQmatrix params
    encodeParams.bNewQmatrixData = m_encodeCtx->bNewQmatrixData;
    encodeParams.bPicQuant       = m_encodeCtx->bPicQuant;
    encodeParams.ppNALUnitParams = m_encodeCtx->ppNALUnitParams;
    encodeParams.pSeiData        = m_encodeCtx->pSEIFromApp;
    encodeParams.pSeiParamBuffer = m_encodeCtx->pSEIFromApp->pSEIBuffer;
    encodeParams.dwSEIDataOffset = 0;

    CODECHAL_HEVC_IQ_MATRIX_PARAMS hevcIqMatrixParams;
    encodeParams.pIQMatrixBuffer = &hevcIqMatrixParams;

    // whether driver need to pack slice header
    if (m_encodeCtx->bHavePackedSliceHdr)
    {
        encodeParams.bAcceleratorHeaderPackingCaps = false;
    }
    else
    {
        encodeParams.bAcceleratorHeaderPackingCaps = true;
    }

    encodeParams.pBSBuffer      = m_encodeCtx->pbsBuffer;
    encodeParams.pSlcHeaderData = (void *)m_encodeCtx->pSliceHeaderData;

    CodechalEncoderState *encoder = dynamic_cast<CodechalEncoderState *>(m_encodeCtx->pCodecHal);
    if(encoder != nullptr)
    {
        encoder->m_mfeEncodeParams.submitIndex  = 0;
        encoder->m_mfeEncodeParams.submitNumber = 1; //By default we only use one stream
        encoder->m_mfeEncodeParams.streamId  = 0;
    }

    MOS_STATUS status = m_encodeCtx->pCodecHal->Execute(&encodeParams);
    if (MOS_STATUS_SUCCESS != status)
    {
        DDI_ASSERTMESSAGE("DDI:Failed in Codechal!");
        return VA_STATUS_ERROR_ENCODING_ERROR;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeHevc::ResetAtFrameLevel()
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);

    // Assume there is only one SPS parameter
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams = (PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);
    hevcSeqParams->bResetBRC                         = 0x0;

    m_encodeCtx->dwNumSlices      = 0x0;
    m_encodeCtx->indexNALUnit     = 0x0;
    m_encodeCtx->uiSliceHeaderCnt = 0x0;

    // reset bsbuffer every frame
    m_encodeCtx->pbsBuffer->pCurrent    = m_encodeCtx->pbsBuffer->pBase;
    m_encodeCtx->pbsBuffer->SliceOffset = 0x0;
    m_encodeCtx->pbsBuffer->BitOffset   = 0x0;
    m_encodeCtx->pbsBuffer->BitSize     = 0x0;

    // clear the packed header information
    if (nullptr != m_encodeCtx->ppNALUnitParams)
    {
        MOS_ZeroMemory(m_encodeCtx->ppNALUnitParams[0], sizeof(CODECHAL_NAL_UNIT_PARAMS) * HEVC_MAX_NAL_UNIT_TYPE);
    }

    m_encodeCtx->bHavePackedSliceHdr   = false;
    m_encodeCtx->bLastPackedHdrIsSlice = false;
    m_encodeCtx->bMBQpEnable           = false;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeHevc::ParseSeqParams(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncSequenceParameterBufferHEVC *seqParams = (VAEncSequenceParameterBufferHEVC *)ptr;

    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams = (PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS)((uint8_t *)m_encodeCtx->pSeqParams);
    DDI_CHK_NULL(hevcSeqParams, "nullptr hevcSeqParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    //MOS_ZeroMemory(hevcSeqParams, sizeof(CODEC_HEVC_ENCODE_SEQUENCE_PARAMS));

    uint8_t log2MinCUSize = seqParams->log2_min_luma_coding_block_size_minus3 + 3;

    hevcSeqParams->wFrameWidthInMinCbMinus1  = (seqParams->pic_width_in_luma_samples >> log2MinCUSize) - 1;
    hevcSeqParams->wFrameHeightInMinCbMinus1 = (seqParams->pic_height_in_luma_samples >> log2MinCUSize) - 1;
    hevcSeqParams->general_profile_idc       = seqParams->general_profile_idc;
    hevcSeqParams->Level                     = seqParams->general_level_idc / 3;  // App sends (30*level), we track as (10*level)
    hevcSeqParams->general_tier_flag         = seqParams->general_tier_flag;
    hevcSeqParams->GopPicSize                = seqParams->intra_period;
    hevcSeqParams->GopRefDist                = seqParams->ip_period;
    hevcSeqParams->chroma_format_idc         = seqParams->seq_fields.bits.chroma_format_idc;

    hevcSeqParams->RateControlMethod = VARC2HalRC(m_encodeCtx->uiRCMethod);

    hevcSeqParams->TargetBitRate = MOS_ROUNDUP_DIVIDE(seqParams->bits_per_second, CODECHAL_ENCODE_BRC_KBPS);
    hevcSeqParams->MaxBitRate    = MOS_ROUNDUP_DIVIDE(seqParams->bits_per_second, CODECHAL_ENCODE_BRC_KBPS);
    hevcSeqParams->MinBitRate    = MOS_ROUNDUP_DIVIDE(seqParams->bits_per_second, CODECHAL_ENCODE_BRC_KBPS);
    // if didn't setting FrameRate, set to 30 by default, can be overwritten by misc paramter
    if (!hevcSeqParams->FrameRate.Numerator) {
        hevcSeqParams->FrameRate.Numerator   = 3000;
        hevcSeqParams->FrameRate.Denominator = 100;
    }

    // set default same as application, can be overwritten by HRD params
    hevcSeqParams->InitVBVBufferFullnessInBit = (uint32_t)seqParams->bits_per_second;
    hevcSeqParams->VBVBufferSizeInBit         = (uint32_t)seqParams->bits_per_second << 1;

    hevcSeqParams->scaling_list_enable_flag           = seqParams->seq_fields.bits.scaling_list_enabled_flag;
    hevcSeqParams->sps_temporal_mvp_enable_flag       = seqParams->seq_fields.bits.sps_temporal_mvp_enabled_flag;
    hevcSeqParams->strong_intra_smoothing_enable_flag = seqParams->seq_fields.bits.strong_intra_smoothing_enabled_flag;
    hevcSeqParams->amp_enabled_flag                   = seqParams->seq_fields.bits.amp_enabled_flag;
    hevcSeqParams->SAO_enabled_flag                   = seqParams->seq_fields.bits.sample_adaptive_offset_enabled_flag;
    hevcSeqParams->pcm_enabled_flag                   = seqParams->seq_fields.bits.pcm_enabled_flag;
    hevcSeqParams->pcm_loop_filter_disable_flag       = seqParams->seq_fields.bits.pcm_loop_filter_disabled_flag;
    hevcSeqParams->LowDelayMode                       = seqParams->seq_fields.bits.low_delay_seq;
    hevcSeqParams->HierarchicalFlag                   = seqParams->seq_fields.bits.hierachical_flag;

    hevcSeqParams->log2_max_coding_block_size_minus3 = seqParams->log2_diff_max_min_luma_coding_block_size +
                                                       seqParams->log2_min_luma_coding_block_size_minus3;
    hevcSeqParams->log2_min_coding_block_size_minus3    = seqParams->log2_min_luma_coding_block_size_minus3;
    hevcSeqParams->log2_max_transform_block_size_minus2 = seqParams->log2_diff_max_min_transform_block_size +
                                                          seqParams->log2_min_transform_block_size_minus2;
    hevcSeqParams->log2_min_transform_block_size_minus2 = seqParams->log2_min_transform_block_size_minus2;
    hevcSeqParams->max_transform_hierarchy_depth_intra  = seqParams->max_transform_hierarchy_depth_intra;
    hevcSeqParams->max_transform_hierarchy_depth_inter  = seqParams->max_transform_hierarchy_depth_inter;
    hevcSeqParams->log2_min_PCM_cb_size_minus3          = seqParams->log2_min_pcm_luma_coding_block_size_minus3;
    hevcSeqParams->log2_max_PCM_cb_size_minus3          = seqParams->log2_max_pcm_luma_coding_block_size_minus3;
    hevcSeqParams->bit_depth_luma_minus8                = seqParams->seq_fields.bits.bit_depth_luma_minus8;
    hevcSeqParams->bit_depth_chroma_minus8              = seqParams->seq_fields.bits.bit_depth_chroma_minus8;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeHevc::ParsePicParams(
    DDI_MEDIA_CONTEXT *mediaCtx,
    void              *ptr)
{
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncPictureParameterBufferHEVC *picParams = (VAEncPictureParameterBufferHEVC *)ptr;

    PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams = (PCODEC_HEVC_ENCODE_PICTURE_PARAMS)((uint8_t *)m_encodeCtx->pPicParams);
    DDI_CHK_NULL(hevcPicParams, "nullptr hevcPicParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    MOS_ZeroMemory(hevcPicParams, sizeof(CODEC_HEVC_ENCODE_PICTURE_PARAMS));

    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams = (PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS)((uint8_t *)m_encodeCtx->pSeqParams);

    if(picParams->decoded_curr_pic.picture_id != VA_INVALID_SURFACE)
    {
        DDI_CHK_RET(RegisterRTSurfaces(&(m_encodeCtx->RTtbl), DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParams->decoded_curr_pic.picture_id)), "RegisterRTSurfaces failed!");
    }

    // Curr Recon Pic
    SetupCodecPicture(
        mediaCtx,
        &(m_encodeCtx->RTtbl),
        &hevcPicParams->CurrReconstructedPic,
        picParams->decoded_curr_pic,
        false,
        false);

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_encodeCtx->RTtbl);
    rtTbl->pCurrentReconTarget           = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParams->decoded_curr_pic.picture_id);
    // The surface for reconstructed frame is not registered, return error to app
    if (nullptr == rtTbl->pCurrentReconTarget)
    {
        DDI_ASSERTMESSAGE("invalid surface for reconstructed frame");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    // curr orig pic
    hevcPicParams->CurrOriginalPic.FrameIdx = (uint8_t)GetRenderTargetID(rtTbl, rtTbl->pCurrentReconTarget);
    hevcPicParams->CurrOriginalPic.PicFlags = hevcPicParams->CurrReconstructedPic.PicFlags;
    hevcPicParams->CurrOriginalPic.PicEntry = hevcPicParams->CurrReconstructedPic.PicEntry;
    hevcPicParams->CollocatedRefPicIndex    = picParams->collocated_ref_pic_index;

    // RefFrame List
    for (uint32_t i = 0; i < numMaxRefFrame; i++)
    {
        if(picParams->reference_frames[i].picture_id != VA_INVALID_SURFACE)
        {
            DDI_CHK_RET(UpdateRegisteredRTSurfaceFlag(&(m_encodeCtx->RTtbl), DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParams->reference_frames[i].picture_id)), "RegisterRTSurfaces failed!");
        }
        SetupCodecPicture(
            mediaCtx,
            &(m_encodeCtx->RTtbl),
            &(hevcPicParams->RefFrameList[i]),
            picParams->reference_frames[i],
            true,
            false);

        hevcPicParams->RefFramePOCList[i] = picParams->reference_frames[i].pic_order_cnt;
    }

    hevcPicParams->CurrPicOrderCnt = picParams->decoded_curr_pic.pic_order_cnt;

    /* picParams->coding_type; App is always setting this to 0 */
    hevcPicParams->CodingType = picParams->pic_fields.bits.coding_type;

    hevcPicParams->HierarchLevelPlus1 = picParams->hierarchical_level_plus1;

    /* Reset it to zero now */
    hevcPicParams->NumSlices = 0;

    hevcPicParams->sign_data_hiding_flag          = picParams->pic_fields.bits.sign_data_hiding_enabled_flag;
    hevcPicParams->constrained_intra_pred_flag    = picParams->pic_fields.bits.constrained_intra_pred_flag;
    hevcPicParams->transform_skip_enabled_flag    = picParams->pic_fields.bits.transform_skip_enabled_flag;
    hevcPicParams->transquant_bypass_enabled_flag = picParams->pic_fields.bits.transquant_bypass_enabled_flag;
    hevcPicParams->tiles_enabled_flag             = picParams->pic_fields.bits.tiles_enabled_flag;
    hevcPicParams->cu_qp_delta_enabled_flag       = picParams->pic_fields.bits.cu_qp_delta_enabled_flag;
    hevcPicParams->weighted_pred_flag             = picParams->pic_fields.bits.weighted_pred_flag;
    hevcPicParams->weighted_bipred_flag           = picParams->pic_fields.bits.weighted_bipred_flag;
    hevcPicParams->loop_filter_across_slices_flag = picParams->pic_fields.bits.pps_loop_filter_across_slices_enabled_flag;
    hevcPicParams->loop_filter_across_tiles_flag  = picParams->pic_fields.bits.loop_filter_across_tiles_enabled_flag;
    hevcPicParams->bLastPicInSeq                  = (picParams->last_picture & HEVC_LAST_PICTURE_EOSEQ) ? 1 : 0;
    hevcPicParams->bLastPicInStream               = (picParams->last_picture & HEVC_LAST_PICTURE_EOSTREAM) ? 1 : 0;
    hevcPicParams->bUseRawPicForRef               = false;
    hevcPicParams->bScreenContent                 = picParams->pic_fields.bits.screen_content_flag;
    hevcPicParams->bEmulationByteInsertion        = true;

    hevcPicParams->QpY                              = picParams->pic_init_qp;
    hevcPicParams->diff_cu_qp_delta_depth           = picParams->diff_cu_qp_delta_depth;
    hevcPicParams->pps_cb_qp_offset                 = picParams->pps_cb_qp_offset;
    hevcPicParams->pps_cr_qp_offset                 = picParams->pps_cr_qp_offset;
    hevcPicParams->num_tile_columns_minus1          = picParams->num_tile_columns_minus1;
    hevcPicParams->num_tile_rows_minus1             = picParams->num_tile_rows_minus1;
    hevcPicParams->log2_parallel_merge_level_minus2 = picParams->log2_parallel_merge_level_minus2;
    hevcPicParams->LcuMaxBitsizeAllowed             = picParams->ctu_max_bitsize_allowed;
    hevcPicParams->bUsedAsRef                       = picParams->pic_fields.bits.reference_pic_flag;
    hevcPicParams->slice_pic_parameter_set_id       = picParams->slice_pic_parameter_set_id;
    hevcPicParams->nal_unit_type                    = picParams->nal_unit_type;
    hevcPicParams->no_output_of_prior_pics_flag     = picParams->pic_fields.bits.no_output_of_prior_pics_flag;
    hevcPicParams->bDisplayFormatSwizzle            = NeedDisapayFormatSwizzle(rtTbl->pCurrentRT, rtTbl->pCurrentReconTarget);

    // Correct Input color space of Sequence parameter here
    hevcSeqParams->InputColorSpace                  = hevcPicParams->bDisplayFormatSwizzle ? ECOLORSPACE_P601 : ECOLORSPACE_P709;

    if (hevcPicParams->tiles_enabled_flag)
    {
        uint16_t shift = hevcSeqParams->log2_max_coding_block_size_minus3 - 
                                            hevcSeqParams->log2_min_coding_block_size_minus3;
        uint16_t frameWidthAligedInLCU = MOS_ROUNDUP_SHIFT((hevcSeqParams->wFrameWidthInMinCbMinus1 + 1), shift);
        uint16_t frameHeightAligedInLCU = MOS_ROUNDUP_SHIFT((hevcSeqParams->wFrameHeightInMinCbMinus1 + 1), shift);

        if(hevcPicParams->num_tile_columns_minus1 > CODECHAL_GET_ARRAY_LENGTH(picParams->column_width_minus1) ||
            hevcPicParams->num_tile_rows_minus1 > CODECHAL_GET_ARRAY_LENGTH(picParams->row_height_minus1))
        {
            // app passed wrong parameters
            DDI_ASSERTMESSAGE("invalid tile parameters!");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        for(uint32_t i = 0; i < (uint32_t)(hevcPicParams->num_tile_columns_minus1); i++)
        {
            hevcPicParams->tile_column_width[i] = picParams->column_width_minus1[i] + 1;
            frameWidthAligedInLCU -= hevcPicParams->tile_column_width[i];
        }

        if(frameWidthAligedInLCU > 0)
        {
            hevcPicParams->tile_column_width[hevcPicParams->num_tile_columns_minus1] = frameWidthAligedInLCU;
        }
        else
        {
            // app passed wrong parameters
            DDI_ASSERTMESSAGE("invalid tile parameters!");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        for(uint32_t i = 0; i < (uint32_t)(hevcPicParams->num_tile_rows_minus1); i++)
        {
            hevcPicParams->tile_row_height[i] = picParams->row_height_minus1[i] + 1;
            frameHeightAligedInLCU -= hevcPicParams->tile_row_height[i];
        }

        if(frameHeightAligedInLCU > 0)
        {
            hevcPicParams->tile_row_height[hevcPicParams->num_tile_rows_minus1] = frameHeightAligedInLCU;
        }
        else
        {
            // app passed wrong parameters
            DDI_ASSERTMESSAGE("invalid tile parameters!");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
    }

    DDI_MEDIA_BUFFER *buf = DdiMedia_GetBufferFromVABufferID(mediaCtx, picParams->coded_buf);
    if (nullptr == buf)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    //Application may re-use the buffer so need to remove before adding to status report again
    RemoveFromStatusReportQueue(buf);
    DdiMedia_MediaBufferToMosResource(buf, &(m_encodeCtx->resBitstreamBuffer));

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeHevc::ParseSlcParams(
    DDI_MEDIA_CONTEXT *mediaCtx,
    void              *ptr,
    uint32_t          numSlices)
{
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncSliceParameterBufferHEVC *vaEncSlcParamsHEVC = (VAEncSliceParameterBufferHEVC *)ptr;

    PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSlcParams = (PCODEC_HEVC_ENCODE_SLICE_PARAMS)m_encodeCtx->pSliceParams;
    DDI_CHK_NULL(hevcSlcParams, "nullptr hevcSlcParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams = (PCODEC_HEVC_ENCODE_PICTURE_PARAMS)(m_encodeCtx->pPicParams);
    DDI_CHK_NULL(hevcPicParams, "nullptr hevcPicParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    hevcPicParams->CodingType = CodechalPicTypeFromVaSlcType(vaEncSlcParamsHEVC->slice_type);

    if (vaEncSlcParamsHEVC->slice_segment_address == 0)
    {
        hevcPicParams->NumSlices = 0;
    }
    else
    {
        hevcSlcParams += hevcPicParams->NumSlices;
    }

    MOS_ZeroMemory(
        hevcSlcParams,
        numSlices * sizeof(CODEC_HEVC_ENCODE_SLICE_PARAMS));

    for (uint32_t slcCount = 0; slcCount < numSlices; slcCount++, hevcSlcParams++)
    {
        hevcSlcParams->slice_segment_address = vaEncSlcParamsHEVC->slice_segment_address;
        hevcSlcParams->NumLCUsInSlice        = vaEncSlcParamsHEVC->num_ctu_in_slice;

        hevcSlcParams->num_ref_idx_l0_active_minus1         = vaEncSlcParamsHEVC->num_ref_idx_l0_active_minus1;
        hevcSlcParams->num_ref_idx_l1_active_minus1         = vaEncSlcParamsHEVC->num_ref_idx_l1_active_minus1;
        hevcSlcParams->bLastSliceOfPic                      = vaEncSlcParamsHEVC->slice_fields.bits.last_slice_of_pic_flag;
        hevcSlcParams->dependent_slice_segment_flag         = vaEncSlcParamsHEVC->slice_fields.bits.dependent_slice_segment_flag;
        hevcSlcParams->slice_temporal_mvp_enable_flag       = vaEncSlcParamsHEVC->slice_fields.bits.slice_temporal_mvp_enabled_flag;
        hevcSlcParams->slice_type                           = vaEncSlcParamsHEVC->slice_type;
        hevcSlcParams->slice_sao_luma_flag                  = vaEncSlcParamsHEVC->slice_fields.bits.slice_sao_luma_flag;
        hevcSlcParams->slice_sao_chroma_flag                = vaEncSlcParamsHEVC->slice_fields.bits.slice_sao_chroma_flag;
        hevcSlcParams->mvd_l1_zero_flag                     = vaEncSlcParamsHEVC->slice_fields.bits.mvd_l1_zero_flag;
        hevcSlcParams->cabac_init_flag                      = vaEncSlcParamsHEVC->slice_fields.bits.cabac_init_flag;
        hevcSlcParams->slice_deblocking_filter_disable_flag = vaEncSlcParamsHEVC->slice_fields.bits.slice_deblocking_filter_disabled_flag;
        hevcSlcParams->collocated_from_l0_flag              = vaEncSlcParamsHEVC->slice_fields.bits.collocated_from_l0_flag;

        hevcSlcParams->slice_qp_delta                      = vaEncSlcParamsHEVC->slice_qp_delta;
        hevcSlcParams->slice_cb_qp_offset                  = vaEncSlcParamsHEVC->slice_cb_qp_offset;
        hevcSlcParams->slice_cr_qp_offset                  = vaEncSlcParamsHEVC->slice_cr_qp_offset;
        hevcSlcParams->beta_offset_div2                    = vaEncSlcParamsHEVC->slice_beta_offset_div2;
        hevcSlcParams->tc_offset_div2                      = vaEncSlcParamsHEVC->slice_tc_offset_div2;
        hevcSlcParams->MaxNumMergeCand                     = vaEncSlcParamsHEVC->max_num_merge_cand;
        hevcSlcParams->luma_log2_weight_denom              = vaEncSlcParamsHEVC->luma_log2_weight_denom;
        hevcSlcParams->delta_chroma_log2_weight_denom      = vaEncSlcParamsHEVC->delta_chroma_log2_weight_denom;
        hevcSlcParams->slice_id                            = hevcPicParams->NumSlices + slcCount;
        hevcSlcParams->BitLengthSliceHeaderStartingPortion = 40;

        hevcSlcParams->bLastSliceOfPic = (slcCount == numSlices - 1) ? 1 : 0;
        // If either stored slice count or current count in this buffer are non-zero, then verify
        // that previous slice is not marked as last slice (it will be marked as last if it was last
        // slice in previous buffer, which is not necessarily last slice in picture).
        if (slcCount || hevcPicParams->NumSlices)
        {
            hevcSlcParams--;
            hevcSlcParams->bLastSliceOfPic = false;
            hevcSlcParams++;
        }

        for (uint32_t i = 0; i < numMaxRefFrame; i++)
        {
            // list 0
            hevcSlcParams->luma_offset[0][i]       = vaEncSlcParamsHEVC->luma_offset_l0[i];
            hevcSlcParams->delta_luma_weight[0][i] = vaEncSlcParamsHEVC->delta_luma_weight_l0[i];

            hevcSlcParams->chroma_offset[0][i][0]       = vaEncSlcParamsHEVC->chroma_offset_l0[i][0];
            hevcSlcParams->delta_chroma_weight[0][i][0] = vaEncSlcParamsHEVC->delta_chroma_weight_l0[i][0];

            hevcSlcParams->chroma_offset[0][i][1]       = vaEncSlcParamsHEVC->chroma_offset_l0[i][1];
            hevcSlcParams->delta_chroma_weight[0][i][1] = vaEncSlcParamsHEVC->delta_chroma_weight_l0[i][1];

            // list 1
            hevcSlcParams->luma_offset[1][i]       = vaEncSlcParamsHEVC->luma_offset_l1[i];
            hevcSlcParams->delta_luma_weight[1][i] = vaEncSlcParamsHEVC->delta_luma_weight_l1[i];

            hevcSlcParams->chroma_offset[1][i][0]       = vaEncSlcParamsHEVC->chroma_offset_l1[i][0];
            hevcSlcParams->delta_chroma_weight[1][i][0] = vaEncSlcParamsHEVC->delta_chroma_weight_l1[i][0];

            hevcSlcParams->chroma_offset[1][i][1]       = vaEncSlcParamsHEVC->chroma_offset_l1[i][1];
            hevcSlcParams->delta_chroma_weight[1][i][1] = vaEncSlcParamsHEVC->delta_chroma_weight_l1[i][1];
        }

        for (uint32_t i = 0; i < numMaxRefFrame; i++)
        {
            if(i >  hevcSlcParams->num_ref_idx_l0_active_minus1)
            {
                hevcSlcParams->RefPicList[0][i].FrameIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC;
                hevcSlcParams->RefPicList[0][i].PicFlags = PICTURE_INVALID;
                hevcSlcParams->RefPicList[0][i].PicEntry = 0xFF;
            }
            else
            {
                SetupCodecPicture(
                    mediaCtx,
                    &(m_encodeCtx->RTtbl),
                    &(hevcSlcParams->RefPicList[0][i]),
                    vaEncSlcParamsHEVC->ref_pic_list0[i],
                    false,
                    true);
                GetSlcRefIdx(&(hevcPicParams->RefFrameList[0]), &(hevcSlcParams->RefPicList[0][i]));
            }
        }

        for (uint32_t i = 0; i < numMaxRefFrame; i++)
        {
            if(i >  hevcSlcParams->num_ref_idx_l1_active_minus1)
            {
                hevcSlcParams->RefPicList[1][i].FrameIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC;
                hevcSlcParams->RefPicList[1][i].PicFlags = PICTURE_INVALID;
                hevcSlcParams->RefPicList[1][i].PicEntry = 0xFF;
            }
            else
            {
                SetupCodecPicture(
                    mediaCtx,
                    &(m_encodeCtx->RTtbl),
                    &(hevcSlcParams->RefPicList[1][i]),
                    vaEncSlcParamsHEVC->ref_pic_list1[i],
                    false,
                    true);
                GetSlcRefIdx(&(hevcPicParams->RefFrameList[0]), &(hevcSlcParams->RefPicList[1][i]));
            }
        }

        vaEncSlcParamsHEVC++;
    }

    hevcPicParams->NumSlices += numSlices;
    m_encodeCtx->dwNumSlices = hevcPicParams->NumSlices;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeHevc::FindNalUnitStartCodes(
    uint8_t * buf,
    uint32_t size,
    uint32_t * startCodesOffset,
    uint32_t * startCodesLength)
{
    uint8_t i = 0;

    while (((i + 3) < size) &&
           (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0x01) &&
           (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0 || buf[i+3] != 0x01))
    {
        i++;
    }

    if ((i + 3) == size)
    {
        if (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0x01)
        {
            return VA_STATUS_ERROR_INVALID_BUFFER; //NALU start codes doesn't exit
        }
        else
        {
            *startCodesOffset = size - 3;
            *startCodesLength = 3;
            return VA_STATUS_SUCCESS;
        }
    }

    if (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0x01)
    {
        *startCodesOffset = i;
        *startCodesLength = 4;
    }
    else
    {
        *startCodesOffset = i;
        *startCodesLength = 3;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeHevc::ParsePackedHeaderParams(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx.", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr.", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncPackedHeaderParameterBuffer *encPackedHeaderParamBuf = (VAEncPackedHeaderParameterBuffer *)ptr;
    m_encodeCtx->bLastPackedHdrIsSlice                        = false;
    if (encPackedHeaderParamBuf->type == VAEncPackedHeaderHEVC_Slice)
    {
        m_encodeCtx->bLastPackedHdrIsSlice = true;
        m_encodeCtx->bHavePackedSliceHdr   = true;

        // Current minimum LCU size for HEVC is 16x16. Minimum 1 LCU per slice.
        if (m_encodeCtx->uiSliceHeaderCnt >= m_encodeCtx->wPicHeightInMB * m_encodeCtx->wPicWidthInMB)
        {
            return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        }

        // get the packed header size
        m_encodeCtx->pSliceHeaderData[m_encodeCtx->uiSliceHeaderCnt].BitSize                = encPackedHeaderParamBuf->bit_length;
        // don't know NALU start codes now, assign to 4 here when has_emulation_bytes is 0 and later will correct it
        m_encodeCtx->pSliceHeaderData[m_encodeCtx->uiSliceHeaderCnt].SkipEmulationByteCount = (encPackedHeaderParamBuf->has_emulation_bytes) ? (encPackedHeaderParamBuf->bit_length + 7) / 8 : 4;
    }
    else
    {
        /* App does not indicate the type, using PPS in general for headers */
        m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiNalUnitType             = HEVC_NAL_UT_PPS;
        m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->bInsertEmulationBytes     = (encPackedHeaderParamBuf->has_emulation_bytes) ? false : true;
        // don't know NALU start codes now, assign to 4 here when has_emulation_bytes is 0 and later will correct it
        m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiSkipEmulationCheckCount = (encPackedHeaderParamBuf->has_emulation_bytes) ? (encPackedHeaderParamBuf->bit_length + 7) / 8 : 4;
        m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiSize                    = (encPackedHeaderParamBuf->bit_length + 7) / 8;
        m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiOffset                  = 0;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeHevc::ParsePackedHeaderData(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx.", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr.", VA_STATUS_ERROR_INVALID_PARAMETER);

    BSBuffer *bsBuffer = m_encodeCtx->pbsBuffer;
    DDI_CHK_NULL(bsBuffer, "nullptr bsBuffer.", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (m_encodeCtx->indexNALUnit == 0 && m_encodeCtx->uiSliceHeaderCnt == 0)
    {
        bsBuffer->pCurrent    = bsBuffer->pBase;
        bsBuffer->SliceOffset = 0;
        bsBuffer->BitOffset   = 0;
        bsBuffer->BitSize     = 0;
    }

    uint32_t   hdrDataSize = 0;
    MOS_STATUS eStatus     = MOS_STATUS_SUCCESS;
    if (true == m_encodeCtx->bLastPackedHdrIsSlice)
    {
        hdrDataSize = (m_encodeCtx->pSliceHeaderData[m_encodeCtx->uiSliceHeaderCnt].BitSize + 7) / 8;

        eStatus = MOS_SecureMemcpy(bsBuffer->pCurrent, bsBuffer->BufferSize - bsBuffer->SliceOffset, (uint8_t *)ptr, hdrDataSize);
        if (MOS_STATUS_SUCCESS != eStatus)
        {
            DDI_ASSERTMESSAGE("DDI:packed slice header size is too large to be supported!");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        m_encodeCtx->pSliceHeaderData[m_encodeCtx->uiSliceHeaderCnt].SliceOffset = bsBuffer->pCurrent - bsBuffer->pBase;

        // correct SkipEmulationByteCount
        // according to LibVA principle, one packed header buffer should only contain one NALU,
        // so when has_emulation_bytes is 0, SkipEmulationByteCount only needs to skip the NALU start codes
        if (m_encodeCtx->pSliceHeaderData[m_encodeCtx->uiSliceHeaderCnt].SkipEmulationByteCount != hdrDataSize)
        {
            uint32_t startCodesOffset = 0;
            uint32_t startCodesLength = 0;
            VAStatus vaSts = VA_STATUS_SUCCESS;
            vaSts = FindNalUnitStartCodes((uint8_t *)ptr, hdrDataSize, &startCodesOffset, &startCodesLength);
            if (VA_STATUS_SUCCESS != vaSts)
            {
                DDI_ASSERTMESSAGE("DDI: packed slice header doesn't include NAL unit start codes!");
                return vaSts;
            }
            m_encodeCtx->pSliceHeaderData[m_encodeCtx->uiSliceHeaderCnt].SkipEmulationByteCount = MOS_MIN(15, (startCodesOffset + startCodesLength));
        }

        m_encodeCtx->uiSliceHeaderCnt++;
        m_encodeCtx->bLastPackedHdrIsSlice = false;
    }
    else
    {
        // copy sps and pps header data
        hdrDataSize = m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiSize;

        eStatus = MOS_SecureMemcpy(bsBuffer->pCurrent, bsBuffer->BufferSize - bsBuffer->SliceOffset, (uint8_t *)ptr, hdrDataSize);
        if (MOS_STATUS_SUCCESS != eStatus)
        {
            DDI_ASSERTMESSAGE("DDI:packed header size is too large to be supported!");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        // correct uiSkipEmulationCheckCount
        // according to LibVA principle, one packed header buffer should only contain one NALU,
        // so when has_emulation_bytes is 0, uiSkipEmulationCheckCount only needs to skip the NALU start codes
        if (m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiSkipEmulationCheckCount != hdrDataSize)
        {
            uint32_t startCodesOffset = 0;
            uint32_t startCodesLength = 0;
            VAStatus vaSts = VA_STATUS_SUCCESS;
            vaSts = FindNalUnitStartCodes((uint8_t *)ptr, hdrDataSize, &startCodesOffset, &startCodesLength);
            if (VA_STATUS_SUCCESS != vaSts)
            {
                DDI_ASSERTMESSAGE("DDI: packed header doesn't include NAL unit start codes!");
                return vaSts;
            }
            m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiSkipEmulationCheckCount = MOS_MIN(15, (startCodesOffset + startCodesLength));
        }

        m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiOffset = bsBuffer->pCurrent - bsBuffer->pBase;
        m_encodeCtx->indexNALUnit++;
    }

    bsBuffer->pCurrent += hdrDataSize;
    bsBuffer->SliceOffset += hdrDataSize;
    bsBuffer->BitSize += hdrDataSize * 8;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeHevc::ParseMiscParams(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx->pSeqParams, "nullptr m_encodeCtx->pSeqParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx->pPicParams, "nullptr m_encodeCtx->pPicParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS seqParams = (PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS  picParams = (PCODEC_HEVC_ENCODE_PICTURE_PARAMS)(m_encodeCtx->pPicParams);

    VAEncMiscParameterBuffer *miscParamBuf = (VAEncMiscParameterBuffer *)ptr;
    DDI_CHK_NULL(miscParamBuf->data, "nullptr miscParamBuf->data", VA_STATUS_ERROR_INVALID_PARAMETER);

    switch ((int32_t)(miscParamBuf->type))
    {
    case VAEncMiscParameterTypeQualityLevel:
    {
        VAEncMiscParameterBufferQualityLevel *vaEncMiscParamQualityLevel = (VAEncMiscParameterBufferQualityLevel *)miscParamBuf->data;
        m_encodeCtx->targetUsage                                         = (uint8_t)vaEncMiscParamQualityLevel->quality_level;

#ifdef _FULL_OPEN_SOURCE
        if(seqParams->TargetUsage >= 1 && seqParams->TargetUsage <= 2)
        {
            seqParams->TargetUsage = 4;
        }
        else if(seqParams->TargetUsage >= 3 && seqParams->TargetUsage <= 5)
        {
            seqParams->TargetUsage = 7;
        }
        else
        {
            DDI_ASSERTMESSAGE("unsupported target usage in HEVC encoder.");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
#endif

        // HEVC only supports TU=1, 4, and 7
        if (1 != m_encodeCtx->targetUsage && 4 != m_encodeCtx->targetUsage && 7 != m_encodeCtx->targetUsage)
        {
            DDI_ASSERTMESSAGE("unsupported target usage in HEVC encoder.");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        break;
    }
    case VAEncMiscParameterTypeHRD:
    {
        VAEncMiscParameterHRD *vaEncMiscParamHRD = (VAEncMiscParameterHRD *)miscParamBuf->data;
        seqParams->VBVBufferSizeInBit            = vaEncMiscParamHRD->buffer_size;
        seqParams->InitVBVBufferFullnessInBit    = vaEncMiscParamHRD->initial_buffer_fullness;
        seqParams->RateControlMethod             = RATECONTROL_CBR;
        break;
    }
    case VAEncMiscParameterTypeFrameRate:
    {
        VAEncMiscParameterFrameRate *vaEncMiscParamFR = (VAEncMiscParameterFrameRate *)miscParamBuf->data;
        uint32_t vaFRnumerator = vaEncMiscParamFR->framerate & 0xffff;

#if (VA_MAJOR_VERSION < 1)
        uint32_t vaFRDenominator = 100;
#else
        uint32_t vaFRDenominator = (vaEncMiscParamFR->framerate >> 16) & 0xffff;
#endif

        if(vaFRDenominator == 0)
        {
            vaFRDenominator = 1;
        }
        // Pass frame rate value to seqParams and set BRC reset flag and bNewSeq to true while dynamic BRC occures
        seqParams->FrameRate.Numerator = vaFRnumerator;
        seqParams->FrameRate.Denominator = vaFRDenominator;
        if(m_previousFRvalue != 0)
        {
            uint16_t seqFRvalue = (uint16_t)(seqParams->FrameRate.Numerator / seqParams->FrameRate.Denominator);
            if(m_previousFRvalue != seqFRvalue)
            {
                seqParams->bResetBRC = 0x1;
                m_encodeCtx->bNewSeq = true;
            }
        }
        m_previousFRvalue = (uint16_t)(vaFRnumerator / vaFRDenominator);
        break;
    }
    case VAEncMiscParameterTypeRateControl:
    {
        // Assume only one SPS here, modify when enable multiple SPS support
        VAEncMiscParameterRateControl *vaEncMiscParamRC = (VAEncMiscParameterRateControl *)miscParamBuf->data;

        seqParams->TargetBitRate = MOS_ROUNDUP_DIVIDE(vaEncMiscParamRC->bits_per_second, CODECHAL_ENCODE_BRC_KBPS);
        seqParams->MBBRC         = (vaEncMiscParamRC->rc_flags.bits.mb_rate_control <= mbBrcDisabled) ? vaEncMiscParamRC->rc_flags.bits.mb_rate_control : 0;
        //enable parallelBRC for Android and Linux
        seqParams->ParallelBRC = vaEncMiscParamRC->rc_flags.bits.enable_parallel_brc;

        if (true == m_encodeCtx->bVdencActive)
        {
            picParams->BRCMaxQp  = (vaEncMiscParamRC->max_qp == 0) ? 51 : (uint8_t)CodecHal_Clip3(1, 51, (int8_t)vaEncMiscParamRC->max_qp);
            picParams->BRCMinQp = (vaEncMiscParamRC->min_qp == 0) ? 1 : (uint8_t)CodecHal_Clip3(1, picParams->BRCMaxQp, (int8_t)vaEncMiscParamRC->min_qp);
        }
        else
        {
            picParams->BRCMaxQp  = (uint8_t)CodecHal_Clip3(0, 51, (int8_t)vaEncMiscParamRC->max_qp); //use 0 to ignore
            picParams->BRCMinQp = (uint8_t)CodecHal_Clip3(0, picParams->BRCMaxQp, (int8_t)vaEncMiscParamRC->min_qp); //use 0 to ignore
        }

        if (VA_RC_NONE == m_encodeCtx->uiRCMethod || VA_RC_CQP == m_encodeCtx->uiRCMethod)
        {
            seqParams->RateControlMethod = RATECONTROL_CQP;
            seqParams->MBBRC             = 0;
        }
        else if (VA_RC_CBR & m_encodeCtx->uiRCMethod )
        {
            seqParams->MaxBitRate        = seqParams->TargetBitRate;
            seqParams->MinBitRate        = seqParams->TargetBitRate;
            seqParams->RateControlMethod = RATECONTROL_CBR;
        }
        else if (VA_RC_ICQ & m_encodeCtx->uiRCMethod)
        {
            seqParams->ICQQualityFactor  = vaEncMiscParamRC->ICQ_quality_factor;
            seqParams->RateControlMethod = RATECONTROL_ICQ;
            seqParams->MBBRC             = 1;
        }
        else if(VA_RC_VCM & m_encodeCtx->uiRCMethod)
        {
            seqParams->RateControlMethod = RATECONTROL_VCM;
            seqParams->MBBRC             = 0;
        }
        else if(VA_RC_VBR & m_encodeCtx->uiRCMethod)
        {
            seqParams->RateControlMethod = RATECONTROL_VBR;
        }
        else if (VA_RC_QVBR & m_encodeCtx->uiRCMethod)
        {
            seqParams->RateControlMethod = RATECONTROL_QVBR;
            seqParams->ICQQualityFactor = vaEncMiscParamRC->quality_factor;
        }
        else
        {
            DDI_ASSERTMESSAGE("invalid RC method.");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        if((RATECONTROL_VCM == seqParams->RateControlMethod)
            || (RATECONTROL_VBR == seqParams->RateControlMethod)
            || (RATECONTROL_CBR == seqParams->RateControlMethod)
            || (RATECONTROL_QVBR == seqParams->RateControlMethod))
        {
            seqParams->MaxBitRate    = seqParams->TargetBitRate;
            seqParams->MinBitRate    = seqParams->TargetBitRate * (2 * vaEncMiscParamRC->target_percentage - 100) / 100;
            seqParams->TargetBitRate = seqParams->TargetBitRate * vaEncMiscParamRC->target_percentage / 100;

            if ((m_encodeCtx->uiTargetBitRate != seqParams->TargetBitRate) ||
                (m_encodeCtx->uiMaxBitRate != seqParams->MaxBitRate))
            {
                if ((m_encodeCtx->uiTargetBitRate != 0) && (m_encodeCtx->uiMaxBitRate != 0))
                {
                    seqParams->bResetBRC = 0x1;
                    m_encodeCtx->bNewSeq = true;
                }
                m_encodeCtx->uiTargetBitRate = seqParams->TargetBitRate;
                m_encodeCtx->uiMaxBitRate    = seqParams->MaxBitRate;
            }
        }
#ifndef ANDROID
        seqParams->FrameSizeTolerance = static_cast<ENCODE_FRAMESIZE_TOLERANCE>(vaEncMiscParamRC->rc_flags.bits.frame_tolerance_mode);
#endif
        break;
    }
    case VAEncMiscParameterTypeParallelBRC:
    {
        VAEncMiscParameterParallelRateControl *vaEncMiscParameterParallel = (VAEncMiscParameterParallelRateControl *)miscParamBuf->data;

        seqParams->NumOfBInGop[0] = vaEncMiscParameterParallel->num_b_in_gop[0];
        seqParams->NumOfBInGop[1] = vaEncMiscParameterParallel->num_b_in_gop[1];
        seqParams->NumOfBInGop[2] = vaEncMiscParameterParallel->num_b_in_gop[2];

        break;
    }
    case VAEncMiscParameterTypeRIR:
    {
        VAEncMiscParameterRIR *vaEncMiscParamRIR = (VAEncMiscParameterRIR *)miscParamBuf->data;
        picParams->bEnableRollingIntraRefresh    = vaEncMiscParamRIR->rir_flags.value & 0x3;  //only lower two bits are valid

        // Set for all frames since pic type is not known yet. Disable in slice params if its I or B type.
        if ((ROLLING_I_COLUMN == picParams->bEnableRollingIntraRefresh) || (ROLLING_I_ROW == picParams->bEnableRollingIntraRefresh))
        {
            picParams->IntraInsertionLocation  = (uint16_t)vaEncMiscParamRIR->intra_insertion_location;
            picParams->IntraInsertionSize      = (uint8_t)vaEncMiscParamRIR->intra_insert_size;
            picParams->QpDeltaForInsertedIntra = vaEncMiscParamRIR->qp_delta_for_inserted_intra;
        }
        else if ((ROLLING_I_ROW | ROLLING_I_COLUMN) == picParams->bEnableRollingIntraRefresh)  // cannot have row and column rolling
        {
            picParams->bEnableRollingIntraRefresh = ROLLING_I_DISABLED;
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        break;
    }
    case VAEncMiscParameterTypeROI:
    {
        VAEncMiscParameterBufferROI *vaEncMiscParamROI = (VAEncMiscParameterBufferROI *)miscParamBuf->data;
        uint32_t                     maxROIsupported   = CODECHAL_ENCODE_HEVC_MAX_NUM_ROI;
        uint8_t                      blockSize         = (m_encodeCtx->bVdencActive) ? vdencRoiBlockSize : CODECHAL_MACROBLOCK_WIDTH;

         uint32_t frameWidth = (seqParams->wFrameWidthInMinCbMinus1 + 1) << (seqParams->log2_min_coding_block_size_minus3 + 3);
         uint32_t frameHeight = (seqParams->wFrameHeightInMinCbMinus1 + 1) << (seqParams->log2_min_coding_block_size_minus3 + 3);
         uint16_t rightBorder = (uint16_t)CODECHAL_GET_WIDTH_IN_BLOCKS(frameWidth, blockSize) - 1;
         uint16_t bottomBorder = (uint16_t)CODECHAL_GET_HEIGHT_IN_BLOCKS(frameHeight, blockSize) - 1;

        if (vaEncMiscParamROI->num_roi)
        {
            for (uint32_t i = 0; i < vaEncMiscParamROI->num_roi; ++i)
            {
                auto &pic_param_roi = picParams->ROI[i];
                auto &va_roi = vaEncMiscParamROI->roi[i];

                pic_param_roi.Top    = va_roi.roi_rectangle.y;
                pic_param_roi.Left   = va_roi.roi_rectangle.x;
                pic_param_roi.Right  = va_roi.roi_rectangle.x + va_roi.roi_rectangle.width - 1;
                pic_param_roi.Bottom = va_roi.roi_rectangle.y+va_roi.roi_rectangle.height - 1;

                pic_param_roi.PriorityLevelOrDQp = va_roi.roi_value;

                if( pic_param_roi.Left > pic_param_roi.Right){
                    DDI_ASSERTMESSAGE("ROI left > ROI Right.");
                    return VA_STATUS_ERROR_INVALID_PARAMETER;
                }

                // check frame boundary
                if( pic_param_roi.Top > pic_param_roi.Bottom){
                    DDI_ASSERTMESSAGE("ROI Top > ROI Bottom.");
                    return VA_STATUS_ERROR_INVALID_PARAMETER;
                }


                if (m_encodeCtx->bVdencActive == false)
                {
                    // align to CTB edge (32 on Gen9) to avoid QP average issue
                    pic_param_roi.Left   = MOS_ALIGN_FLOOR(pic_param_roi.Left, 32);
                    pic_param_roi.Right  = MOS_ALIGN_CEIL(pic_param_roi.Right, 32);
                    pic_param_roi.Top    = MOS_ALIGN_FLOOR(pic_param_roi.Top, 32);
                    pic_param_roi.Bottom = MOS_ALIGN_CEIL(pic_param_roi.Bottom, 32);
                }


                // Convert from pixel units to block size units
                pic_param_roi.Left   /= blockSize;
                pic_param_roi.Right  /= blockSize;
                pic_param_roi.Top    /= blockSize;
                pic_param_roi.Bottom /= blockSize;

                // DDI defination for right and bottom parameters is inclusive whereas
                // the defination in BRC kernel input is exclusive. So adding 1 to right & bottom.
                 pic_param_roi.Right += 1;
                 pic_param_roi.Bottom += 1;

                 // check frame boundary
                 pic_param_roi.Left   = pic_param_roi.Left  > rightBorder  ? rightBorder : pic_param_roi.Left;
                 pic_param_roi.Right  = pic_param_roi.Right  > rightBorder  ? rightBorder  : pic_param_roi.Right;
                 pic_param_roi.Top    = pic_param_roi.Top    > bottomBorder ? bottomBorder : pic_param_roi.Top;
                 pic_param_roi.Bottom = pic_param_roi.Bottom > bottomBorder ? bottomBorder : pic_param_roi.Bottom;

            }
            picParams->NumROI = MOS_MIN(vaEncMiscParamROI->num_roi, maxROIsupported);
        }
#ifndef ANDROID
        // support DeltaQP based ROI by default
        seqParams->ROIValueInDeltaQP = vaEncMiscParamROI->roi_flags.bits.roi_value_is_qp_delta;
        if(picParams->NumROI != 0 && seqParams->ROIValueInDeltaQP == 0)
        {
            DDI_ASSERTMESSAGE("ROI does not support priority level now.");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
#endif
        break;
    }
    case VAEncMiscParameterTypeDirtyRect:
    {
        VAEncMiscParameterBufferDirtyRect *vaEncMiscDirtyRect = (VAEncMiscParameterBufferDirtyRect *)miscParamBuf->data;

        uint8_t blockSize  = (m_encodeCtx->bVdencActive) ? vdencRoiBlockSize : CODECHAL_MACROBLOCK_WIDTH;
        uint32_t frameWidth = (seqParams->wFrameWidthInMinCbMinus1 + 1) << (seqParams->log2_min_coding_block_size_minus3 + 3);
        uint32_t frameHeight = (seqParams->wFrameHeightInMinCbMinus1 + 1) << (seqParams->log2_min_coding_block_size_minus3 + 3);

         DDI_CHK_NULL(vaEncMiscDirtyRect->roi_rectangle, "nullptr dirty rect", VA_STATUS_ERROR_INVALID_PARAMETER);
        if(vaEncMiscDirtyRect->num_roi_rectangle > ENCODE_VDENC_HEVC_MAX_DIRTYRECT_G10)
        {
            DDI_ASSERTMESSAGE("The number of dirty rect is greater than %d", ENCODE_VDENC_HEVC_MAX_DIRTYRECT_G10);
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        picParams->pDirtyRect = m_pDirtyRect;

        if (vaEncMiscDirtyRect->num_roi_rectangle > m_numDirtyRects)
        {
            picParams->pDirtyRect = (PCODEC_ROI)MOS_ReallocMemory(picParams->pDirtyRect, vaEncMiscDirtyRect->num_roi_rectangle * sizeof(CODEC_ROI));
            if (!picParams->pDirtyRect)
            {
                picParams->NumDirtyRects = 0;
                DDI_ASSERTMESSAGE("Allocation of pDirtyRect failed!");
                return VA_STATUS_ERROR_ALLOCATION_FAILED;
            }
            m_pDirtyRect = picParams->pDirtyRect;
            m_numDirtyRects = vaEncMiscDirtyRect->num_roi_rectangle;
        }

        if(vaEncMiscDirtyRect->num_roi_rectangle > 0)
        {
            MOS_ZeroMemory(picParams->pDirtyRect, vaEncMiscDirtyRect->num_roi_rectangle * sizeof(CODEC_ROI));
            picParams->NumDirtyRects = 0;

            for (int32_t i = 0; i < vaEncMiscDirtyRect->num_roi_rectangle; i++)
            {
                auto &rect    = picParams->pDirtyRect[i];
                auto &va_rect = vaEncMiscDirtyRect->roi_rectangle[i];

                // Check and adjust if rect not within frame boundaries
                rect.Left   = MOS_MIN(va_rect.x, frameWidth - 1);
                rect.Top    = MOS_MIN(va_rect.y, frameHeight - 1);
                rect.Right  = MOS_MIN(va_rect.x + va_rect.width, frameWidth - 1);
                rect.Bottom = MOS_MIN(va_rect.y + va_rect.height, frameHeight - 1);

                if( rect.Left > rect.Right ||  rect.Top > rect.Bottom)
                {
                    DDI_ASSERTMESSAGE("Invalid rect region parameter.");
                    return VA_STATUS_ERROR_INVALID_PARAMETER;
                }

                rect.Right  = MOS_ALIGN_CEIL(rect.Right, blockSize);
                rect.Bottom = MOS_ALIGN_CEIL(rect.Bottom, blockSize);

                // Convert from pixel units to block units
                rect.Left /= blockSize;
                rect.Right /= blockSize;
                rect.Top /= blockSize;
                rect.Bottom /= blockSize;

                picParams->NumDirtyRects ++;
            }
        }
        break;
    }
    case VAEncMiscParameterTypeSkipFrame:
    {
        VAEncMiscParameterSkipFrame *vaEncMiscParamSkipFrame = (VAEncMiscParameterSkipFrame *)miscParamBuf->data;
        // populate skipped frame params from DDI
        if (FRAME_SKIP_NORMAL != vaEncMiscParamSkipFrame->skip_frame_flag)
        {
            DDI_ASSERTMESSAGE("unsupported misc parameter type.");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        picParams->SkipFrameFlag  = vaEncMiscParamSkipFrame->skip_frame_flag;
        picParams->NumSkipFrames  = vaEncMiscParamSkipFrame->num_skip_frames;
        picParams->SizeSkipFrames = vaEncMiscParamSkipFrame->size_skip_frames;
        break;
    }
    case VAEncMiscParameterTypeMaxSliceSize:
    {
        VAEncMiscParameterMaxSliceSize *vaEncMiscParamMaxSliceSize = (VAEncMiscParameterMaxSliceSize *)miscParamBuf->data;
        m_encodeCtx->EnableSliceLevelRateCtrl                      = true;
        seqParams->SliceSizeControl                                = true;
        picParams->MaxSliceSizeInBytes                             = vaEncMiscParamMaxSliceSize->max_slice_size;
        break;
    }
    case VAEncMiscParameterTypeEncQuality:
    {
        VAEncMiscParameterEncQuality *vaEncMiscParamPrivate           = (VAEncMiscParameterEncQuality *)miscParamBuf->data;
        picParams->bUseRawPicForRef                                = vaEncMiscParamPrivate->useRawPicForRef;
        break;
    }
    default:
        DDI_ASSERTMESSAGE("unsupported misc parameter type.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    return VA_STATUS_SUCCESS;
}

uint8_t DdiEncodeHevc::CodechalPicTypeFromVaSlcType(uint8_t vaSliceType)
{
    switch (vaSliceType)
    {
    case sliceTypeI:
        return I_TYPE;
    case sliceTypeP:
        return P_TYPE;
    case sliceTypeB:
        return B_TYPE;
    default:
        return 0;
    }
}

void DdiEncodeHevc::GetSlcRefIdx(CODEC_PICTURE *picReference, CODEC_PICTURE *slcReference)
{
    if (nullptr == picReference || nullptr == slcReference)
    {
        return;
    }

    int32_t i = 0;
    if (CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC != slcReference->FrameIdx)
    {
        for (i = 0; i < numMaxRefFrame; i++)
        {
            if (slcReference->FrameIdx == picReference[i].FrameIdx)
            {
                slcReference->FrameIdx = i;
                slcReference->PicEntry = i;
                break;
            }
        }
        if (numMaxRefFrame == i)
        {
            slcReference->FrameIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC;
            slcReference->PicEntry = 0xFF;
        }
    }
}

void DdiEncodeHevc::SetupCodecPicture(
    DDI_MEDIA_CONTEXT             *mediaCtx,
    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl,
    CODEC_PICTURE                 *codecHalPic,
    VAPictureHEVC                 vaPicHEVC,
    bool                          picReference,
    bool                          sliceReference)
{
    DDI_UNUSED(sliceReference);

    if (DDI_CODEC_INVALID_FRAME_INDEX != vaPicHEVC.picture_id)
    {
        codecHalPic->FrameIdx = GetRenderTargetID(rtTbl, DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, vaPicHEVC.picture_id));
        codecHalPic->PicEntry = codecHalPic->FrameIdx;
    }
    else
    {
        codecHalPic->FrameIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC;
        codecHalPic->PicFlags = PICTURE_INVALID;
        codecHalPic->PicEntry = 0xFF;
    }

    if (PICTURE_INVALID != codecHalPic->PicFlags)
    {
        codecHalPic->PicFlags = PICTURE_FRAME;
        if (picReference)
        {
            if ((vaPicHEVC.flags & VA_PICTURE_HEVC_LONG_TERM_REFERENCE) == VA_PICTURE_HEVC_LONG_TERM_REFERENCE)
            {
                codecHalPic->PicFlags = (CODEC_PICTURE_FLAG)(codecHalPic->PicFlags | PICTURE_LONG_TERM_REFERENCE);
                codecHalPic->PicEntry |= 0x80;
            }
            else
            {
                codecHalPic->PicFlags = (CODEC_PICTURE_FLAG)(codecHalPic->PicFlags | PICTURE_SHORT_TERM_REFERENCE);
            }
        }
    }
}

uint32_t DdiEncodeHevc::getSliceParameterBufferSize()
{
    return sizeof(VAEncSliceParameterBufferHEVC);
}

uint32_t DdiEncodeHevc::getSequenceParameterBufferSize()
{
    return sizeof(VAEncSequenceParameterBufferHEVC);
}

uint32_t DdiEncodeHevc::getPictureParameterBufferSize()
{
    return sizeof(VAEncPictureParameterBufferHEVC);
}

uint32_t DdiEncodeHevc::getQMatrixBufferSize()
{
    return sizeof(VAQMatrixBufferHEVC);
}
