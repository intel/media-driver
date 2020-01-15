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
//! \file     media_ddi_encode_vp9.cpp
//! \brief    Defines class for DDI media vp9 encode.
//!

#include "media_libva.h"
#include "media_libva_encoder.h"
#include "media_ddi_encode_vp9.h"
#include "media_libva_util.h"
#include "codec_def_encode_vp9.h"
#include "media_ddi_encode_const.h"
#include "media_ddi_factory.h"
#include "media_libvpx_vp9.h"

extern template class MediaDdiFactoryNoArg<DdiEncodeBase>;
static bool isEncodeVp9Registered =
    MediaDdiFactoryNoArg<DdiEncodeBase>::RegisterCodec<DdiEncodeVp9>(ENCODE_ID_VP9);

DdiEncodeVp9::~DdiEncodeVp9()
{
    if (m_encodeCtx == nullptr)
    {
        return;
    }

    MOS_FreeMemory(m_encodeCtx->pSeqParams);
    m_encodeCtx->pSeqParams = nullptr;

    MOS_FreeMemory(m_encodeCtx->pPicParams);
    m_encodeCtx->pPicParams = nullptr;

    MOS_FreeMemory(m_encodeCtx->pQmatrixParams);
    m_encodeCtx->pQmatrixParams = nullptr;

    MOS_FreeMemory(m_encodeCtx->pEncodeStatusReport);
    m_encodeCtx->pEncodeStatusReport = nullptr;

    if (m_encodeCtx->pbsBuffer)
    {
        MOS_FreeMemory(m_encodeCtx->pbsBuffer->pBase);
        m_encodeCtx->pbsBuffer->pBase = nullptr;
    }
    MOS_FreeMemory(m_encodeCtx->pbsBuffer);
    m_encodeCtx->pbsBuffer = nullptr;

    if (m_encodeCtx->ppNALUnitParams && m_encodeCtx->ppNALUnitParams[0])
    {
        /* ppNALUnitParams[0] indicates the start address of NALUnitParams */
        MOS_FreeMemory(m_encodeCtx->ppNALUnitParams[0]);
        m_encodeCtx->ppNALUnitParams[0] = nullptr;
    }

    MOS_FreeMemory(m_encodeCtx->ppNALUnitParams);
    m_encodeCtx->ppNALUnitParams = nullptr;

    MOS_FreeMemory(m_segParams);
    m_segParams = nullptr;

    MOS_FreeMemory(m_codedBufStatus);
    m_codedBufStatus = nullptr;
}

VAStatus DdiEncodeVp9::EncodeInCodecHal(uint32_t numSlices)
{
    DDI_UNUSED(numSlices);

    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_encodeCtx->pCodecHal, "nullptr m_encodeCtx->pCodecHal", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_encodeCtx->RTtbl);

    CODEC_VP9_ENCODE_SEQUENCE_PARAMS *seqParams = (PCODEC_VP9_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);
    CODEC_VP9_ENCODE_PIC_PARAMS *vp9PicParam = (PCODEC_VP9_ENCODE_PIC_PARAMS)(m_encodeCtx->pPicParams);

    EncoderParams encodeParams;
    MOS_ZeroMemory(&encodeParams, sizeof(EncoderParams));
    encodeParams.ExecCodecFunction = m_encodeCtx->codecFunction;

    /* check whether the target bit rate is initialized for BRC */
    if ((VA_RC_CBR == m_encodeCtx->uiRCMethod) ||
        (VA_RC_VBR == m_encodeCtx->uiRCMethod))
    {
        if (seqParams->TargetBitRate[0] == 0)
        {
            DDI_ASSERTMESSAGE("DDI: No RateControl param for BRC\n!");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
    }

    // Raw Surface
    MOS_SURFACE rawSurface;
    MOS_ZeroMemory(&rawSurface, sizeof(MOS_SURFACE));

    DdiMedia_MediaSurfaceToMosResource(rtTbl->pCurrentRT, &(rawSurface.OsResource));

    switch (rawSurface.OsResource.Format)
    {
    case Format_NV12:
        seqParams->SeqFlags.fields.SourceFormat   = VP9_ENCODED_CHROMA_FORMAT_YUV420;
        seqParams->SeqFlags.fields.SourceBitDepth = VP9_ENCODED_BIT_DEPTH_8;
        break;
    case Format_YUY2:
        seqParams->SeqFlags.fields.SourceFormat   = VP9_ENCODED_CHROMA_FORMAT_YUV422;
        seqParams->SeqFlags.fields.SourceBitDepth = VP9_ENCODED_BIT_DEPTH_8;
        break;
    case Format_UYVY:
        seqParams->SeqFlags.fields.SourceFormat   = VP9_ENCODED_CHROMA_FORMAT_YUV422;
        seqParams->SeqFlags.fields.SourceBitDepth = VP9_ENCODED_BIT_DEPTH_8;
        break;
    case Format_AYUV:
    case Format_A8R8G8B8:
    case Format_A8B8G8R8:
        seqParams->SeqFlags.fields.SourceFormat   = VP9_ENCODED_CHROMA_FORMAT_YUV444;
        seqParams->SeqFlags.fields.SourceBitDepth = VP9_ENCODED_BIT_DEPTH_8;
        break;
    case Format_P010:
        seqParams->SeqFlags.fields.SourceFormat   = VP9_ENCODED_CHROMA_FORMAT_YUV420;
        seqParams->SeqFlags.fields.SourceBitDepth = VP9_ENCODED_BIT_DEPTH_10;
        break;
    case Format_Y210:
        seqParams->SeqFlags.fields.SourceFormat   = VP9_ENCODED_CHROMA_FORMAT_YUV422;
        seqParams->SeqFlags.fields.SourceBitDepth = VP9_ENCODED_BIT_DEPTH_10;
        break;
    case Format_Y410:
    case Format_R10G10B10A2:
    case Format_B10G10R10A2:
        seqParams->SeqFlags.fields.SourceFormat   = VP9_ENCODED_CHROMA_FORMAT_YUV444;
        seqParams->SeqFlags.fields.SourceBitDepth = VP9_ENCODED_BIT_DEPTH_10;
        break;
    // P016 and Y416 aren't supported yet so return error for these formats
    case Format_P016:
    case Format_Y416:
    default:
        DDI_ASSERTMESSAGE("DDI:Incorrect Format for input surface\n!");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
        break;
    }

    rawSurface.Format = rawSurface.OsResource.Format;

    // Recon Surface
    MOS_SURFACE reconSurface;
    MOS_ZeroMemory(&reconSurface, sizeof(MOS_SURFACE));
    reconSurface.dwOffset = 0;

    DdiMedia_MediaSurfaceToMosResource(rtTbl->pCurrentReconTarget, &(reconSurface.OsResource));

    switch (m_encodeCtx->vaProfile)
    {
    case VAProfileVP9Profile1:
        reconSurface.Format                        = Format_AYUV;
        seqParams->SeqFlags.fields.EncodedFormat   = VP9_ENCODED_CHROMA_FORMAT_YUV444;
        seqParams->SeqFlags.fields.EncodedBitDepth = VP9_ENCODED_BIT_DEPTH_8;
        break;
    case VAProfileVP9Profile2:
        reconSurface.Format                        = Format_P010;
        seqParams->SeqFlags.fields.EncodedFormat   = VP9_ENCODED_CHROMA_FORMAT_YUV420;
        seqParams->SeqFlags.fields.EncodedBitDepth = VP9_ENCODED_BIT_DEPTH_10;
        break;
    case VAProfileVP9Profile3:
        reconSurface.Format                        = Format_Y410;
        seqParams->SeqFlags.fields.EncodedFormat   = VP9_ENCODED_CHROMA_FORMAT_YUV444;
        seqParams->SeqFlags.fields.EncodedBitDepth = VP9_ENCODED_BIT_DEPTH_10;
        break;
    case VAProfileVP9Profile0:
    default:
        reconSurface.Format                        = Format_NV12;
        seqParams->SeqFlags.fields.EncodedFormat   = VP9_ENCODED_CHROMA_FORMAT_YUV420;
        seqParams->SeqFlags.fields.EncodedBitDepth = VP9_ENCODED_BIT_DEPTH_8;
        break;
    }

    // Bitstream surface
    MOS_RESOURCE bitstreamSurface;
    MOS_ZeroMemory(&bitstreamSurface, sizeof(MOS_RESOURCE));
    bitstreamSurface        = m_encodeCtx->resBitstreamBuffer;  // in render picture
    bitstreamSurface.Format = Format_Buffer;

    //clear registered recon/ref surface flags
    DDI_CHK_RET(ClearRefList(&m_encodeCtx->RTtbl, true), "ClearRefList failed!");

    encodeParams.psRawSurface               = &rawSurface;
    encodeParams.psReconSurface             = &reconSurface;
    encodeParams.presBitstreamBuffer        = &bitstreamSurface;
    encodeParams.presMbCodeSurface          = &m_encodeCtx->resMbCodeBuffer;

    // Segmentation map buffer
    encodeParams.psMbSegmentMapSurface = &m_encodeCtx->segMapBuffer;
    encodeParams.bSegmentMapProvided   = !Mos_ResourceIsNull(&m_encodeCtx->segMapBuffer.OsResource);

    if (VA_RC_CQP == m_encodeCtx->uiRCMethod)
    {
        seqParams->RateControlMethod          = RATECONTROL_CQP;
        seqParams->TargetBitRate[0]           = 0;
        seqParams->MaxBitRate                 = 0;
        seqParams->MinBitRate                 = 0;
        seqParams->InitVBVBufferFullnessInBit = 0;
        seqParams->VBVBufferSizeInBit         = 0;
    }
    else if (VA_RC_CBR == m_encodeCtx->uiRCMethod)
    {
        seqParams->RateControlMethod = RATECONTROL_CBR;
        seqParams->MaxBitRate        = MOS_MAX(seqParams->MaxBitRate, seqParams->TargetBitRate[0]);
        seqParams->MinBitRate        = MOS_MIN(seqParams->MinBitRate, seqParams->TargetBitRate[0]);
    }
    else if (VA_RC_VBR == m_encodeCtx->uiRCMethod)
    {
        seqParams->RateControlMethod = RATECONTROL_VBR;
    }
    else if(VA_RC_ICQ == m_encodeCtx->uiRCMethod)
    {
        seqParams->RateControlMethod = RATECONTROL_CQL;
    }

    seqParams->TargetUsage = vp9TargetUsage;

    /* If the segmentation is not enabled, the SegData will be reset */
    if (vp9PicParam->PicFlags.fields.segmentation_enabled == 0)
    {
        DDI_CHK_NULL(m_segParams, "nullptr m_segParams", VA_STATUS_ERROR_INVALID_PARAMETER);
        for (int i = 0; i < 8; i++)
        {
            MOS_ZeroMemory(&(m_segParams->SegData[i]), sizeof(CODEC_VP9_ENCODE_SEG_PARAMS));
        }
    }
    else if (!isSegParamsChanged)
    {
        /* segmentation is enabled, but segment parameters are not changed */
        vp9PicParam->PicFlags.fields.seg_update_data = 0;
    }

    encodeParams.pSeqParams      = m_encodeCtx->pSeqParams;
    encodeParams.pPicParams      = m_encodeCtx->pPicParams;
    encodeParams.pSliceParams    = m_encodeCtx->pSliceParams;
    encodeParams.ppNALUnitParams = m_encodeCtx->ppNALUnitParams;
    encodeParams.pSegmentParams  = m_segParams;

    for (uint32_t i = 0; i < (seqParams->NumTemporalLayersMinus1+1); i++)
    {
        if (savedFrameRate[i] == 0)
        {
            /* use the default framerate if FrameRate is not passed */
            seqParams->FrameRate[i].uiNumerator   = 30;
            seqParams->FrameRate[i].uiDenominator = 1;
        }
    }

    if (!headerInsertFlag)
    {
        vp9_header_bitoffset picBitOffset;
        uint32_t headerLen = 0;
        uint32_t codecProfile = VP9_PROFILE_0;

        if ((m_encodeCtx->vaProfile >= VAProfileVP9Profile0) &&
            (m_encodeCtx->vaProfile <= VAProfileVP9Profile3))
        {
            codecProfile = m_encodeCtx->vaProfile - VAProfileVP9Profile0;
        }

        Vp9WriteUncompressHeader(m_encodeCtx,
                                   codecProfile,
                                   m_encodeCtx->pbsBuffer->pBase,
                                   &headerLen,
                                   &picBitOffset);

        vp9PicParam->BitOffsetForFirstPartitionSize = picBitOffset.bit_offset_first_partition_size;
        vp9PicParam->BitOffsetForQIndex             = picBitOffset.bit_offset_qindex;
        vp9PicParam->BitOffsetForLFLevel            = picBitOffset.bit_offset_lf_level;
        vp9PicParam->BitOffsetForLFRefDelta         = picBitOffset.bit_offset_ref_lf_delta;
        vp9PicParam->BitOffsetForLFModeDelta        = picBitOffset.bit_offset_mode_lf_delta;
        vp9PicParam->BitOffsetForSegmentation       = picBitOffset.bit_offset_segmentation;

        m_encodeCtx->ppNALUnitParams[0]->uiNalUnitType             = 0x22;
        m_encodeCtx->ppNALUnitParams[0]->bInsertEmulationBytes     = false;
        m_encodeCtx->ppNALUnitParams[0]->uiSkipEmulationCheckCount = 0;
        m_encodeCtx->ppNALUnitParams[0]->uiSize                    = headerLen;
        m_encodeCtx->ppNALUnitParams[0]->uiOffset                  = 0;
    }

    encodeParams.bNewSeq = m_encodeCtx->bNewSeq;
    if (seqParams->SeqFlags.fields.bResetBRC)
    {
        /* When the BRC needs to be reset, it indicates that the new Seq is issued. */
        encodeParams.bNewSeq = true;
    }

    encodeParams.bNewQmatrixData = m_encodeCtx->bNewQmatrixData;
    encodeParams.bPicQuant       = m_encodeCtx->bPicQuant;

    encodeParams.pBSBuffer = m_encodeCtx->pbsBuffer;

    MOS_STATUS status = m_encodeCtx->pCodecHal->Execute(&encodeParams);
    if (MOS_STATUS_SUCCESS != status)
    {
        DDI_ASSERTMESSAGE("DDI:Failed in Codechal!");
        return VA_STATUS_ERROR_ENCODING_ERROR;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp9::ContextInitialize(CodechalSetting *codecHalSettings)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_encodeCtx->pCpDdiInterface, "nullptr m_encodeCtx->pCpDdiInterface.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(codecHalSettings, "nullptr codecHalSettings.", VA_STATUS_ERROR_INVALID_CONTEXT);

    codecHalSettings->codecFunction = m_encodeCtx->codecFunction;
    codecHalSettings->width       = m_encodeCtx->dworiFrameWidth;
    codecHalSettings->height      = m_encodeCtx->dworiFrameHeight;
    codecHalSettings->mode          = m_encodeCtx->wModeType;
    codecHalSettings->standard      = CODECHAL_VP9;
    codecHalSettings->chromaFormat  = (m_chromaFormat == yuv444) ?
        VP9_ENCODED_CHROMA_FORMAT_YUV444 : VP9_ENCODED_CHROMA_FORMAT_YUV420;
    codecHalSettings->lumaChromaDepth = CODECHAL_LUMA_CHROMA_DEPTH_8_BITS;
    if (m_is10Bit)
    {
        codecHalSettings->lumaChromaDepth |= CODECHAL_LUMA_CHROMA_DEPTH_10_BITS;
    }

    VAStatus vaStatus = VA_STATUS_SUCCESS;

    m_encodeCtx->pSeqParams = (void *)MOS_AllocAndZeroMemory(sizeof(CODEC_VP9_ENCODE_SEQUENCE_PARAMS));
    DDI_CHK_NULL(m_encodeCtx->pSeqParams, "nullptr m_encodeCtx->pSeqParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pPicParams = (void *)MOS_AllocAndZeroMemory(sizeof(CODEC_VP9_ENCODE_PIC_PARAMS));
    DDI_CHK_NULL(m_encodeCtx->pPicParams, "nullptr m_encodeCtx->pPicParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Allocate Encode Status Report
    m_encodeCtx->pEncodeStatusReport = (void *)MOS_AllocAndZeroMemory(CODECHAL_ENCODE_STATUS_NUM * sizeof(EncodeStatusReport));
    DDI_CHK_NULL(m_encodeCtx->pEncodeStatusReport, "nullptr m_encodeCtx->pEncodeStatusReport.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Create the bit stream buffer to hold the packed headers from application
    m_encodeCtx->pbsBuffer = (PBSBuffer)MOS_AllocAndZeroMemory(sizeof(BSBuffer));
    DDI_CHK_NULL(m_encodeCtx->pbsBuffer, "nullptr m_encodeCtx->pbsBuffer.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    /* It is enough to allocate 4096 bytes for VP9 packed header */
    m_encodeCtx->pbsBuffer->BufferSize = 4096;
    m_encodeCtx->pbsBuffer->pBase      = (uint8_t *)MOS_AllocAndZeroMemory(m_encodeCtx->pbsBuffer->BufferSize);
    DDI_CHK_NULL(m_encodeCtx->pbsBuffer->pBase, "nullptr m_encodeCtx->pbsBuffer->pBase.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    const int32_t packedNum = 2;
    /* VP9 has only one Packed header.  */
    m_encodeCtx->ppNALUnitParams = (PCODECHAL_NAL_UNIT_PARAMS *)MOS_AllocAndZeroMemory(sizeof(PCODECHAL_NAL_UNIT_PARAMS) * packedNum);
    DDI_CHK_NULL(m_encodeCtx->ppNALUnitParams, "nullptr m_encodeCtx->ppNALUnitParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    CODECHAL_NAL_UNIT_PARAMS *nalUnitParams = (CODECHAL_NAL_UNIT_PARAMS *)MOS_AllocAndZeroMemory(sizeof(CODECHAL_NAL_UNIT_PARAMS) * packedNum);
    DDI_CHK_NULL(nalUnitParams, "nullptr nalUnitParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    for (int32_t i = 0; i < packedNum; i++)
    {
        m_encodeCtx->ppNALUnitParams[i] = &(nalUnitParams[i]);
    }

    // Allocate segment params
    m_segParams = (CODEC_VP9_ENCODE_SEGMENT_PARAMS *)MOS_AllocAndZeroMemory(sizeof(CODEC_VP9_ENCODE_SEGMENT_PARAMS) * 8);
    DDI_CHK_NULL(m_segParams, "nullptr m_segParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Allocate coded buffer status
    m_codedBufStatus = (VACodedBufferVP9Status *)MOS_AllocAndZeroMemory(DDI_ENCODE_MAX_STATUS_REPORT_BUFFER * sizeof(VACodedBufferVP9Status));
    DDI_CHK_NULL(m_codedBufStatus, "nullptr m_codedBufStatus.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    /* RT is used as the default target usage */
    vp9TargetUsage = TARGETUSAGE_RT_SPEED;

    return vaStatus;
}

VAStatus DdiEncodeVp9::RenderPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VABufferID       *buffers,
    int32_t          numBuffers)
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
        case VAIQMatrixBufferType:
        case VAQMatrixBufferType:
            DDI_CHK_STATUS(Qmatrix(data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;

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

        case VAEncPackedHeaderParameterBufferType:
            DDI_CHK_STATUS(ParsePackedHeaderParams(data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncPackedHeaderDataBufferType:
            DDI_CHK_STATUS(ParsePackedHeaderData(data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncMiscParameterBufferType:
            DDI_CHK_STATUS(ParseMiscParams(data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncMacroblockMapBufferType:
            DDI_CHK_STATUS(ParseSegMapParams(buf), VA_STATUS_ERROR_INVALID_BUFFER);
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

// Reset the paramters before each frame
VAStatus DdiEncodeVp9::ResetAtFrameLevel()
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);

    lastPackedHeaderType = 0;
    headerInsertFlag     = 0;

    CODEC_VP9_ENCODE_SEQUENCE_PARAMS *vp9SeqParam = (PCODEC_VP9_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);

    if (vp9SeqParam)
    {
        vp9SeqParam->SeqFlags.fields.bResetBRC = 0;
        vp9SeqParam->MaxBitRate = 0;
        vp9SeqParam->MinBitRate = 0xffffffff;
    }

    m_encodeCtx->bMBQpEnable = false;

    MOS_ZeroMemory(&(m_encodeCtx->segMapBuffer), sizeof(MOS_SURFACE));

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp9::ParseSeqParams(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncSequenceParameterBufferVP9 *seqParams = (VAEncSequenceParameterBufferVP9 *)ptr;

    CODEC_VP9_ENCODE_SEQUENCE_PARAMS *vp9SeqParams = (PCODEC_VP9_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);
    DDI_CHK_NULL(vp9SeqParams, "nullptr vp9SeqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    vp9SeqParams->wMaxFrameWidth   = seqParams->max_frame_width;
    vp9SeqParams->wMaxFrameHeight  = seqParams->max_frame_height;
    vp9SeqParams->GopPicSize       = seqParams->intra_period;

    /* the bits_per_second is only used when the target bit_rate is not initialized */
    if (vp9SeqParams->TargetBitRate[0] == 0)
    {
        vp9SeqParams->TargetBitRate[0] = MOS_ROUNDUP_DIVIDE(seqParams->bits_per_second, CODECHAL_ENCODE_BRC_KBPS);
    }

    if (vp9SeqParams->GopPicSize != savedGopSize)
    {
        savedGopSize = vp9SeqParams->GopPicSize;
        vp9SeqParams->SeqFlags.fields.bResetBRC = 1;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp9::ParsePicParams(DDI_MEDIA_CONTEXT *mediaCtx, void *ptr)
{
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncPictureParameterBufferVP9 *picParam = (VAEncPictureParameterBufferVP9 *)ptr;

    if ((picParam->frame_width_src == 0) && (picParam->frame_width_dst == 0))
    {
        DDI_ASSERTMESSAGE("DDI: frame width in VP9 PicParam is zero\n.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    if ((picParam->frame_height_src == 0) && (picParam->frame_height_dst == 0))
    {
        DDI_ASSERTMESSAGE("DDI: frame height in VP9 PicParam is zero\n.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    CODEC_VP9_ENCODE_PIC_PARAMS *vp9PicParam = (PCODEC_VP9_ENCODE_PIC_PARAMS)(m_encodeCtx->pPicParams);

    DDI_CHK_NULL(vp9PicParam, "nullptr vp9PicParam", VA_STATUS_ERROR_INVALID_PARAMETER);

    MOS_ZeroMemory(vp9PicParam, sizeof(CODEC_VP9_ENCODE_PIC_PARAMS));

    vp9PicParam->PicFlags.fields.frame_type                   = picParam->pic_flags.bits.frame_type;
    vp9PicParam->PicFlags.fields.show_frame                   = picParam->pic_flags.bits.show_frame;
    vp9PicParam->PicFlags.fields.error_resilient_mode         = picParam->pic_flags.bits.error_resilient_mode;
    vp9PicParam->PicFlags.fields.intra_only                   = picParam->pic_flags.bits.intra_only;
    vp9PicParam->PicFlags.fields.allow_high_precision_mv      = picParam->pic_flags.bits.allow_high_precision_mv;
    vp9PicParam->PicFlags.fields.mcomp_filter_type            = picParam->pic_flags.bits.mcomp_filter_type;
    vp9PicParam->PicFlags.fields.frame_parallel_decoding_mode = picParam->pic_flags.bits.frame_parallel_decoding_mode;
    vp9PicParam->PicFlags.fields.reset_frame_context          = picParam->pic_flags.bits.reset_frame_context;
    vp9PicParam->PicFlags.fields.refresh_frame_context        = picParam->pic_flags.bits.refresh_frame_context;
    vp9PicParam->PicFlags.fields.frame_context_idx            = picParam->pic_flags.bits.frame_context_idx;
    vp9PicParam->PicFlags.fields.segmentation_enabled         = picParam->pic_flags.bits.segmentation_enabled;
    vp9PicParam->PicFlags.fields.segmentation_temporal_update = picParam->pic_flags.bits.segmentation_temporal_update;
    vp9PicParam->PicFlags.fields.segmentation_update_map      = picParam->pic_flags.bits.segmentation_update_map;
    vp9PicParam->PicFlags.fields.LosslessFlag                 = picParam->pic_flags.bits.lossless_mode;
    vp9PicParam->PicFlags.fields.comp_prediction_mode         = picParam->pic_flags.bits.comp_prediction_mode;
    vp9PicParam->PicFlags.fields.super_frame                  = picParam->pic_flags.bits.super_frame_flag;
    vp9PicParam->PicFlags.fields.seg_update_data              = picParam->pic_flags.bits.segmentation_enabled;

    vp9PicParam->SrcFrameWidthMinus1          = picParam->frame_width_src - 1;
    vp9PicParam->SrcFrameHeightMinus1         = picParam->frame_height_src - 1;

    vp9PicParam->DstFrameWidthMinus1          = picParam->frame_width_dst - 1;
    vp9PicParam->DstFrameHeightMinus1         = picParam->frame_height_dst - 1;

    /* width_src and width_dst won't be zero at the same time
     * If only one of them is zero, assume that there is no dynamica scaling.
     * In such case it is dervied.
     */
    if ((picParam->frame_width_src == 0) || (picParam->frame_width_dst == 0))
    {
        if (picParam->frame_width_src == 0)
        {
            vp9PicParam->SrcFrameWidthMinus1 = picParam->frame_width_dst - 1;
        }
        else
        {
            vp9PicParam->DstFrameWidthMinus1 = picParam->frame_width_src - 1;
        }
    }

    /* Handle the zero height by using the mechanism similar to width */
    if ((picParam->frame_height_src == 0) || (picParam->frame_height_dst == 0))
    {
        if (picParam->frame_height_src == 0)
        {
            vp9PicParam->SrcFrameHeightMinus1 = picParam->frame_height_dst - 1;
        }
        else
        {
            vp9PicParam->DstFrameHeightMinus1 = picParam->frame_height_src - 1;
        }
    }

    vp9PicParam->filter_level                 = picParam->filter_level;
    vp9PicParam->sharpness_level              = picParam->sharpness_level;

    vp9PicParam->LumaACQIndex                 = picParam->luma_ac_qindex;
    vp9PicParam->LumaDCQIndexDelta            = picParam->luma_dc_qindex_delta;
    vp9PicParam->ChromaACQIndexDelta          = picParam->chroma_ac_qindex_delta;
    vp9PicParam->ChromaDCQIndexDelta          = picParam->chroma_dc_qindex_delta;

    vp9PicParam->RefFlags.fields.LastRefIdx        = picParam->ref_flags.bits.ref_last_idx;
    vp9PicParam->RefFlags.fields.GoldenRefIdx      = picParam->ref_flags.bits.ref_gf_idx;
    vp9PicParam->RefFlags.fields.AltRefIdx         = picParam->ref_flags.bits.ref_arf_idx;
    vp9PicParam->RefFlags.fields.LastRefSignBias   = picParam->ref_flags.bits.ref_last_sign_bias;
    vp9PicParam->RefFlags.fields.GoldenRefSignBias = picParam->ref_flags.bits.ref_gf_sign_bias;
    vp9PicParam->RefFlags.fields.AltRefSignBias    = picParam->ref_flags.bits.ref_arf_sign_bias;

    vp9PicParam->RefFlags.fields.ref_frame_ctrl_l0   = picParam->ref_flags.bits.ref_frame_ctrl_l0;
    vp9PicParam->RefFlags.fields.ref_frame_ctrl_l1   = picParam->ref_flags.bits.ref_frame_ctrl_l1;
    vp9PicParam->RefFlags.fields.refresh_frame_flags = picParam->refresh_frame_flags;
    vp9PicParam->temporal_id                         = picParam->ref_flags.bits.temporal_id;

    for (int32_t i = 0; i < 4; i++)
    {
        vp9PicParam->LFRefDelta[i] = picParam->ref_lf_delta[i];
    }

    vp9PicParam->LFModeDelta[0] = picParam->mode_lf_delta[0];
    vp9PicParam->LFModeDelta[1] = picParam->mode_lf_delta[1];

    vp9PicParam->sharpness_level = picParam->sharpness_level;

    vp9PicParam->BitOffsetForFirstPartitionSize = picParam->bit_offset_first_partition_size;
    vp9PicParam->BitOffsetForQIndex             = picParam->bit_offset_qindex;
    vp9PicParam->BitOffsetForLFLevel            = picParam->bit_offset_lf_level;
    vp9PicParam->BitOffsetForLFRefDelta         = picParam->bit_offset_ref_lf_delta;
    vp9PicParam->BitOffsetForLFModeDelta        = picParam->bit_offset_mode_lf_delta;
    vp9PicParam->BitOffsetForSegmentation       = picParam->bit_offset_segmentation;
    vp9PicParam->BitSizeForSegmentation         = picParam->bit_size_segmentation;

    vp9PicParam->log2_tile_rows = picParam->log2_tile_rows;
    vp9PicParam->log2_tile_columns = picParam->log2_tile_columns;

    vp9PicParam->SkipFrameFlag  = picParam->skip_frame_flag;
    vp9PicParam->NumSkipFrames  = picParam->number_skip_frames;
    vp9PicParam->SizeSkipFrames = picParam->skip_frames_size;

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_encodeCtx->RTtbl);

    auto recon = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->reconstructed_frame);
    DDI_CHK_RET(RegisterRTSurfaces(rtTbl, recon),"RegisterRTSurfaces failed!");

    SetupCodecPicture(mediaCtx, rtTbl, &vp9PicParam->CurrReconstructedPic,
                                             picParam->reconstructed_frame, false);
    rtTbl->pCurrentReconTarget = recon;
    DDI_CHK_NULL(rtTbl->pCurrentReconTarget, "NULL rtTbl->pCurrentReconTarget", VA_STATUS_ERROR_INVALID_PARAMETER);

    // curr orig pic
    vp9PicParam->CurrOriginalPic.FrameIdx = GetRenderTargetID(rtTbl, rtTbl->pCurrentReconTarget);
    vp9PicParam->CurrOriginalPic.PicFlags = vp9PicParam->CurrReconstructedPic.PicFlags;

    for (int32_t i = 0; i < 8; i++)
    {
        if (picParam->reference_frames[i] != VA_INVALID_SURFACE)
        {
            UpdateRegisteredRTSurfaceFlag(rtTbl, DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->reference_frames[i]));
        }
        SetupCodecPicture(
            mediaCtx,
            rtTbl,
            &vp9PicParam->RefFrameList[i],
            picParam->reference_frames[i],
            true);
    }

    DDI_MEDIA_BUFFER *buf = nullptr;

    buf = DdiMedia_GetBufferFromVABufferID(mediaCtx, picParam->coded_buf);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);
    RemoveFromStatusReportQueue(buf);
    DdiMedia_MediaBufferToMosResource(buf, &(m_encodeCtx->resBitstreamBuffer));

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp9::ParsePackedHeaderParams(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    m_encodeCtx->bLastPackedHdrIsSlice = false;

    VAEncPackedHeaderParameterBuffer *packedHeaderParamBuf = (VAEncPackedHeaderParameterBuffer *)ptr;

    if (packedHeaderParamBuf->type != VAEncPackedHeaderRawData)
    {
        DDI_ASSERTMESSAGE("DDI: incorrect packed header type %d\n.", packedHeaderParamBuf->type);
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    // VP9 will always only have 1 NAL type (PPS)
    m_encodeCtx->ppNALUnitParams[0]->uiNalUnitType             = 0x22;
    m_encodeCtx->ppNALUnitParams[0]->bInsertEmulationBytes     = false;
    m_encodeCtx->ppNALUnitParams[0]->uiSkipEmulationCheckCount = 0;
    m_encodeCtx->ppNALUnitParams[0]->uiSize                    = (packedHeaderParamBuf->bit_length + 7) / 8;
    m_encodeCtx->ppNALUnitParams[0]->uiOffset                  = 0;

    lastPackedHeaderType = VAEncPackedHeaderRawData;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp9::ParsePackedHeaderData(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    BSBuffer *bsBuffer = m_encodeCtx->pbsBuffer;
    DDI_CHK_NULL(bsBuffer, "nullptr bsBuffer", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (lastPackedHeaderType != VAEncPackedHeaderRawData)
    {
        DDI_ASSERTMESSAGE("DDI: the packed header param/data is not passed in pair \n.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    /* Only one header data is enough */
    if (headerInsertFlag)
    {
        return VA_STATUS_SUCCESS;
    }

    // Since VP9 only has 1 NAL type it's safe to reset each time unconditionally
    bsBuffer->pCurrent    = bsBuffer->pBase;
    bsBuffer->SliceOffset = 0;
    bsBuffer->BitOffset   = 0;
    bsBuffer->BitSize     = 0;

    // copy pps header data
    uint32_t hdrDataSize = m_encodeCtx->ppNALUnitParams[0]->uiSize;
    DDI_CHK_RET(
        MOS_SecureMemcpy(
            bsBuffer->pCurrent,
            bsBuffer->BufferSize,
            (uint8_t *)ptr,
            hdrDataSize),
        "DDI:packed header size is too large to be supported!");

    m_encodeCtx->ppNALUnitParams[0]->uiOffset = bsBuffer->pCurrent - bsBuffer->pBase;

    bsBuffer->pCurrent += hdrDataSize;
    bsBuffer->SliceOffset += hdrDataSize;
    bsBuffer->BitSize += hdrDataSize * 8;
    headerInsertFlag   = true;

    return VA_STATUS_SUCCESS;
}

/*
 * For VP9 this buffer is used to contain segment params
 */
VAStatus DdiEncodeVp9::Qmatrix(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_segParams, "nullptr m_segParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterTypeVP9PerSegmantParam *segParams = (VAEncMiscParameterTypeVP9PerSegmantParam *)ptr;
    isSegParamsChanged = false;

    for (int32_t i = 0; i < 8; ++i)
    {
        if (   m_segParams->SegData[i].SegmentFlags.fields.SegmentReferenceEnabled != segParams->seg_data[i].seg_flags.bits.segment_reference_enabled
            || m_segParams->SegData[i].SegmentFlags.fields.SegmentReference != segParams->seg_data[i].seg_flags.bits.segment_reference
            || m_segParams->SegData[i].SegmentFlags.fields.SegmentSkipped != segParams->seg_data[i].seg_flags.bits.segment_reference_skipped
            || m_segParams->SegData[i].SegmentQIndexDelta != MOS_CLAMP_MIN_MAX(segParams->seg_data[i].segment_qindex_delta, -255, 255)
            || m_segParams->SegData[i].SegmentLFLevelDelta != MOS_CLAMP_MIN_MAX(segParams->seg_data[i].segment_lf_level_delta, -63, 63))
            isSegParamsChanged = true;

        m_segParams->SegData[i].SegmentFlags.fields.SegmentReferenceEnabled =
            segParams->seg_data[i].seg_flags.bits.segment_reference_enabled;
        m_segParams->SegData[i].SegmentFlags.fields.SegmentReference =
            segParams->seg_data[i].seg_flags.bits.segment_reference;
        m_segParams->SegData[i].SegmentFlags.fields.SegmentSkipped =
            segParams->seg_data[i].seg_flags.bits.segment_reference_skipped;

        m_segParams->SegData[i].SegmentQIndexDelta  =
            MOS_CLAMP_MIN_MAX(segParams->seg_data[i].segment_qindex_delta, -255, 255);

        m_segParams->SegData[i].SegmentLFLevelDelta =
            MOS_CLAMP_MIN_MAX(segParams->seg_data[i].segment_lf_level_delta, -63, 63);
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp9::ParseMiscParamVBV(void *data)
{
    VAEncMiscParameterHRD *vaEncMiscParamHRD = (VAEncMiscParameterHRD *)data;

    CODEC_VP9_ENCODE_SEQUENCE_PARAMS *seqParams = (CODEC_VP9_ENCODE_SEQUENCE_PARAMS *)m_encodeCtx->pSeqParams;

    if ((seqParams == nullptr) || (vaEncMiscParamHRD == nullptr))
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    seqParams->VBVBufferSizeInBit         = vaEncMiscParamHRD->buffer_size;
    seqParams->InitVBVBufferFullnessInBit = vaEncMiscParamHRD->initial_buffer_fullness;

    seqParams->UpperVBVBufferLevelThresholdInBit = 800000;
    seqParams->LowerVBVBufferLevelThresholdInBit = 320000;

    if ((savedHrdSize != seqParams->VBVBufferSizeInBit) ||
        (savedHrdBufFullness != seqParams->InitVBVBufferFullnessInBit))
    {
        savedHrdSize        = seqParams->VBVBufferSizeInBit;
        savedHrdBufFullness = seqParams->InitVBVBufferFullnessInBit;
        seqParams->SeqFlags.fields.bResetBRC = 0x1;
    }

    return VA_STATUS_SUCCESS;
}

// Parse the frame rate paramters from app
VAStatus DdiEncodeVp9::ParseMiscParamFR(void *data)
{
    VAEncMiscParameterFrameRate *vaFrameRate = (VAEncMiscParameterFrameRate *)data;
    CODEC_VP9_ENCODE_SEQUENCE_PARAMS *seqParams = (PCODEC_VP9_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);

    /* This is the optional */
    if ((vaFrameRate == nullptr) || (seqParams == nullptr) ||
        (vaFrameRate->framerate_flags.bits.temporal_id > seqParams->NumTemporalLayersMinus1))
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    uint32_t temporalId = vaFrameRate->framerate_flags.bits.temporal_id;

    if (vaFrameRate->framerate != savedFrameRate[temporalId])
    {
          savedFrameRate[temporalId] = vaFrameRate->framerate;
          seqParams->SeqFlags.fields.bResetBRC |= 0x1;

          uint32_t frameRate = vaFrameRate->framerate;
          seqParams->FrameRate[temporalId].uiNumerator   = frameRate & (0xFFFF);
          seqParams->FrameRate[temporalId].uiDenominator = (frameRate >> 16) & (0xFFFF);
          if (seqParams->FrameRate[temporalId].uiDenominator == 0)
          {
              seqParams->FrameRate[temporalId].uiDenominator = 1;
          }
    }

    return VA_STATUS_SUCCESS;
}

// Parse rate control related information from app
VAStatus DdiEncodeVp9::ParseMiscParamRC(void *data)
{
    CODEC_VP9_ENCODE_SEQUENCE_PARAMS *seqParams = (PCODEC_VP9_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);
    DDI_CHK_NULL(seqParams, "nullptr vp9SeqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterRateControl *vaEncMiscParamRC = (VAEncMiscParameterRateControl *)data;
    DDI_CHK_NULL(data, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t temporalId = vaEncMiscParamRC->rc_flags.bits.temporal_id;
    DDI_CHK_LESS(temporalId, (seqParams->NumTemporalLayersMinus1+1),
        "invalid temporal id", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t bitRate                     = MOS_ROUNDUP_DIVIDE(vaEncMiscParamRC->bits_per_second, CODECHAL_ENCODE_BRC_KBPS);
    seqParams->MaxBitRate                = MOS_MAX(seqParams->MaxBitRate, bitRate);
    seqParams->SeqFlags.fields.bResetBRC = vaEncMiscParamRC->rc_flags.bits.reset;  // adding reset here. will apply both CBR and VBR

    if (VA_RC_CBR == m_encodeCtx->uiRCMethod)
    {
        seqParams->TargetBitRate[temporalId] = bitRate;
        seqParams->MinBitRate                = MOS_MIN(seqParams->MinBitRate, bitRate);
        seqParams->RateControlMethod         = RATECONTROL_CBR;
        if (savedTargetBit[temporalId] != bitRate)
        {
            savedTargetBit[temporalId] = bitRate;
            seqParams->SeqFlags.fields.bResetBRC |= 0x1;
        }
    }
    else if (VA_RC_VBR == m_encodeCtx->uiRCMethod || VA_RC_ICQ == m_encodeCtx->uiRCMethod)
    {
        seqParams->TargetBitRate[temporalId] = bitRate * vaEncMiscParamRC->target_percentage / 100;  // VBR target bits
        uint32_t minBitRate = bitRate * abs((int32_t)(2 * vaEncMiscParamRC->target_percentage) - 100) / 100;
        seqParams->MinBitRate = MOS_MIN(seqParams->TargetBitRate[temporalId], minBitRate);
        seqParams->RateControlMethod = RATECONTROL_VBR;

        if ((savedTargetBit[temporalId] != seqParams->TargetBitRate[temporalId]) ||
            (savedMaxBitRate[temporalId] != bitRate))
        {
            savedTargetBit[temporalId]           = seqParams->TargetBitRate[temporalId];
            seqParams->SeqFlags.fields.bResetBRC |= 0x1;
            savedMaxBitRate[temporalId]          = bitRate;
        }
    }

    if (VA_RC_ICQ == m_encodeCtx->uiRCMethod)
    {
        seqParams->ICQQualityFactor  = vaEncMiscParamRC->ICQ_quality_factor;
        seqParams->RateControlMethod = RATECONTROL_CQL;
    }

    /* the reset flag in RC will be considered. */
    seqParams->SeqFlags.fields.bResetBRC |= vaEncMiscParamRC->rc_flags.bits.reset;  // adding reset here. will apply both CBR and VBR

    /* Enabling Dynamic Scaling */
    seqParams->SeqFlags.fields.EnableDynamicScaling = vaEncMiscParamRC->rc_flags.bits.enable_dynamic_scaling;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp9::ParseMiscParamEncQuality(void *data)
{
    DDI_UNUSED(m_encodeCtx);
    DDI_UNUSED(data);

    /* Ignore it */
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp9::ParseMiscParamQualityLevel(void *data)
{
    DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterBufferQualityLevel *vaEncMiscParamQualityLevel = (VAEncMiscParameterBufferQualityLevel *)data;

    /* it will be mapped to 1, 4, 7.
     * 1-2 mapped to the 1
     * 6-7 mapped  the 7.
     * 0-3-4-5 mapped to 4.
     */

    if (vaEncMiscParamQualityLevel->quality_level == 0)
    {
        vp9TargetUsage = TARGETUSAGE_RT_SPEED;
    }
    else if (vaEncMiscParamQualityLevel->quality_level >= TARGETUSAGE_HI_SPEED)
    {
        vp9TargetUsage = TARGETUSAGE_BEST_SPEED;
    }
    else if (vaEncMiscParamQualityLevel->quality_level <= TARGETUSAGE_HI_QUALITY)
    {
#ifdef _FULL_OPEN_SOURCE
        vp9TargetUsage = TARGETUSAGE_RT_SPEED;
#else
        vp9TargetUsage = TARGETUSAGE_BEST_QUALITY;
#endif
    }
    else
    {
        vp9TargetUsage = TARGETUSAGE_RT_SPEED;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp9::ParseMiscParameterTemporalLayerParams(void *data)
{
    DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    CODEC_VP9_ENCODE_SEQUENCE_PARAMS *seqParams = (PCODEC_VP9_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);

    VAEncMiscParameterTemporalLayerStructure *vaEncTempLayerStruct = (VAEncMiscParameterTemporalLayerStructure *)data;
    DDI_CHK_LESS(vaEncTempLayerStruct->number_of_layers, (CODECHAL_ENCODE_VP9_MAX_NUM_TEMPORAL_LAYERS+1),
        "invalid number of temporal layers", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (vaEncTempLayerStruct->number_of_layers > 0)
    {
        seqParams->NumTemporalLayersMinus1 = vaEncTempLayerStruct->number_of_layers - 1;
    }
    else
    {
        seqParams->NumTemporalLayersMinus1 = 0;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp9::ParseSegMapParams(DDI_MEDIA_BUFFER *buf)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);

    m_encodeCtx->segMapBuffer.Format   = Format_Buffer_2D;
    m_encodeCtx->segMapBuffer.dwOffset = 0;
    DdiMedia_MediaBufferToMosResource(buf, &((m_encodeCtx->segMapBuffer).OsResource));
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp9::ParseMiscParams(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterBuffer *miscParamBuf = (VAEncMiscParameterBuffer *)ptr;
    DDI_CHK_NULL(miscParamBuf->data, "nullptr miscParamBuf->data", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = VA_STATUS_SUCCESS;
    switch ((int32_t)(miscParamBuf->type))
    {
    case VAEncMiscParameterTypeHRD:
    {
        vaStatus = ParseMiscParamVBV((void *)miscParamBuf->data);
        break;
    }
    case VAEncMiscParameterTypeFrameRate:
    {
        vaStatus = ParseMiscParamFR((void *)miscParamBuf->data);
        break;
    }
    case VAEncMiscParameterTypeRateControl:
    {
        vaStatus = ParseMiscParamRC((void *)miscParamBuf->data);
        break;
    }
    case VAEncMiscParameterTypeEncQuality:
    {
        vaStatus = ParseMiscParamEncQuality((void *)miscParamBuf->data);
        break;
    }
    case VAEncMiscParameterTypeTemporalLayerStructure:
    {
        vaStatus = ParseMiscParameterTemporalLayerParams((void *)miscParamBuf->data);
        break;
    }
    case VAEncMiscParameterTypeQualityLevel:
    {
        vaStatus = ParseMiscParamQualityLevel((void *)miscParamBuf->data);
        break;
    }
    default:
    {
        DDI_ASSERTMESSAGE("DDI: unsupported misc parameter type.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    }

    return vaStatus;
}

VAStatus DdiEncodeVp9::ReportExtraStatus(
    EncodeStatusReport   *encodeStatusReport,
    VACodedBufferSegment *codedBufferSegment)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(encodeStatusReport, "nullptr encodeStatusReport", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(codedBufferSegment, "nullptr codedBufferSegment", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = VA_STATUS_SUCCESS;

    // The coded buffer status are one-to-one correspondence with report buffers, even though the index is updated.
    VACodedBufferVP9Status *codedBufStatus = &(m_codedBufStatus[m_encodeCtx->statusReportBuf.ulUpdatePosition]);
    codedBufStatus->loop_filter_level = encodeStatusReport->loopFilterLevel;
    codedBufStatus->long_term_indication = encodeStatusReport->LongTermIndication;
    codedBufStatus->next_frame_width = encodeStatusReport->NextFrameWidthMinus1 + 1;
    codedBufStatus->next_frame_height = encodeStatusReport->NextFrameHeightMinus1 + 1;

    codedBufferSegment->next = codedBufStatus;

    return vaStatus;
}

void DdiEncodeVp9::SetupCodecPicture(
    DDI_MEDIA_CONTEXT                     *mediaCtx,
    DDI_CODEC_RENDER_TARGET_TABLE         *rtTbl,
    CODEC_PICTURE                         *codecHalPic,
    VASurfaceID                           surfaceID,
    bool                                  picReference)
{
    if(VA_INVALID_SURFACE != surfaceID)
    {
        DDI_MEDIA_SURFACE *surface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, surfaceID);
        codecHalPic->FrameIdx = GetRenderTargetID(rtTbl, surface);
    }
    else
    {
        codecHalPic->FrameIdx = (uint8_t)DDI_CODEC_INVALID_FRAME_INDEX;
    }

    if (picReference)
    {
        if (codecHalPic->FrameIdx == (uint8_t)DDI_CODEC_INVALID_FRAME_INDEX)
        {
            codecHalPic->PicFlags = PICTURE_INVALID;
        }
        else
        {
            codecHalPic->PicFlags = PICTURE_SHORT_TERM_REFERENCE;
        }
    }
    else
    {
        codecHalPic->PicFlags = PICTURE_FRAME;
    }
}

uint32_t DdiEncodeVp9::getSequenceParameterBufferSize()
{
        return sizeof(VAEncSequenceParameterBufferVP9);
}

uint32_t DdiEncodeVp9::getPictureParameterBufferSize()
{
        return sizeof(VAEncPictureParameterBufferVP9);
}

uint32_t DdiEncodeVp9::getQMatrixBufferSize()
{
        return sizeof(VAEncSegParamVP9);
}
