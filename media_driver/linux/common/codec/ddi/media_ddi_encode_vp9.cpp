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
#include "codechal_vdenc_vp9_base.h"
#include "media_ddi_encode_const.h"
#include "media_ddi_factory.h"

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

    MOS_FreeMemory(m_encodeCtx->ppNALUnitParams);
    m_encodeCtx->ppNALUnitParams = nullptr;

    MOS_FreeMemory(m_segParams);
    m_segParams = nullptr;
}

VAStatus DdiEncodeVp9::EncodeInCodecHal(uint32_t numSlices)
{
    DDI_UNUSED(numSlices);

    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_encodeCtx->pCodecHal, "nullptr m_encodeCtx->pCodecHal", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_encodeCtx->RTtbl);

    CODEC_VP9_ENCODE_SEQUENCE_PARAMS *seqParams = (PCODEC_VP9_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);

    EncoderParams encodeParams;
    MOS_ZeroMemory(&encodeParams, sizeof(EncoderParams));
    encodeParams.ExecCodecFunction = m_encodeCtx->codecFunction;

    // Raw Surface
    MOS_SURFACE rawSurface;
    MOS_ZeroMemory(&rawSurface, sizeof(MOS_SURFACE));
    rawSurface.Format   = Format_NV12;
    rawSurface.dwOffset = 0;

    DdiMedia_MediaSurfaceToMosResource(rtTbl->pCurrentRT, &(rawSurface.OsResource));
    // Recon Surface
    MOS_SURFACE reconSurface;
    MOS_ZeroMemory(&reconSurface, sizeof(MOS_SURFACE));
    reconSurface.Format   = Format_NV12;
    reconSurface.dwOffset = 0;

    DdiMedia_MediaSurfaceToMosResource(rtTbl->pCurrentReconTarget, &(reconSurface.OsResource));
    // Bitstream surface
    MOS_RESOURCE bitstreamSurface;
    MOS_ZeroMemory(&bitstreamSurface, sizeof(MOS_RESOURCE));
    bitstreamSurface        = m_encodeCtx->resBitstreamBuffer;  // in render picture
    bitstreamSurface.Format = Format_Buffer;

    encodeParams.psRawSurface               = &rawSurface;
    encodeParams.psReconSurface             = &reconSurface;
    encodeParams.presBitstreamBuffer        = &bitstreamSurface;
    encodeParams.presMbCodeSurface          = &m_encodeCtx->resMbCodeBuffer;

    // Segmentation map buffer
    encodeParams.psMbSegmentMapSurface = &m_encodeCtx->segMapBuffer;

    if (VA_RC_CQP == m_encodeCtx->uiRCMethod)
    {
        seqParams->RateControlMethod          = 0;
        seqParams->TargetBitRate[0]           = 0;
        seqParams->MaxBitRate                 = 0;
        seqParams->MinBitRate                 = 0;
        seqParams->InitVBVBufferFullnessInBit = 0;
        seqParams->VBVBufferSizeInBit         = 0;
    }
    else if (VA_RC_CBR == m_encodeCtx->uiRCMethod)
    {
        seqParams->RateControlMethod = 1;
        seqParams->MaxBitRate        = seqParams->TargetBitRate[0];
        seqParams->MinBitRate        = seqParams->TargetBitRate[0];
    }
    else if (VA_RC_VBR == m_encodeCtx->uiRCMethod)
    {
        seqParams->RateControlMethod = 2;
    }

    encodeParams.pSeqParams      = m_encodeCtx->pSeqParams;
    encodeParams.pPicParams      = m_encodeCtx->pPicParams;
    encodeParams.pSliceParams    = m_encodeCtx->pSliceParams;
    encodeParams.ppNALUnitParams = m_encodeCtx->ppNALUnitParams;
    encodeParams.pSegmentParams  = m_segParams;

    // Hack until misc FR buffer is correctly handled/parsed
    seqParams->FrameRate[0].uiNumerator   = 30;
    seqParams->FrameRate[0].uiDenominator = 1;

    encodeParams.bNewSeq = m_encodeCtx->bNewSeq;

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

VAStatus DdiEncodeVp9::ContextInitialize(CODECHAL_SETTINGS *codecHalSettings)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_encodeCtx->pCpDdiInterface, "nullptr m_encodeCtx->pCpDdiInterface.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(codecHalSettings, "nullptr codecHalSettings.", VA_STATUS_ERROR_INVALID_CONTEXT);

    codecHalSettings->CodecFunction = m_encodeCtx->codecFunction;
    codecHalSettings->dwWidth       = m_encodeCtx->dwFrameWidth;
    codecHalSettings->dwHeight      = m_encodeCtx->dwFrameHeight;
    codecHalSettings->Mode          = m_encodeCtx->wModeType;
    codecHalSettings->Standard      = CODECHAL_VP9;

    VAStatus vaStatus = VA_STATUS_SUCCESS;

    m_encodeCtx->pSeqParams = (void *)MOS_AllocAndZeroMemory(sizeof(CODEC_VP9_ENCODE_SEQUENCE_PARAMS));
    DDI_CHK_NULL(m_encodeCtx->pSeqParams, "nullptr m_encodeCtx->pSeqParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pPicParams = (void *)MOS_AllocAndZeroMemory(sizeof(CODEC_VP9_ENCODE_PIC_PARAMS));
    DDI_CHK_NULL(m_encodeCtx->pPicParams, "nullptr m_encodeCtx->pPicParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    codecHalSettings->pCpParams = m_encodeCtx->pCpDdiInterface->GetParams();

    // Allocate Encode Status Report
    m_encodeCtx->pEncodeStatusReport = (void *)MOS_AllocAndZeroMemory(CODECHAL_ENCODE_STATUS_NUM * sizeof(EncodeStatusReport));
    DDI_CHK_NULL(m_encodeCtx->pEncodeStatusReport, "nullptr m_encodeCtx->pEncodeStatusReport.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Create the bit stream buffer to hold the packed headers from application
    m_encodeCtx->pbsBuffer = (PBSBuffer)MOS_AllocAndZeroMemory(sizeof(BSBuffer));
    DDI_CHK_NULL(m_encodeCtx->pbsBuffer, "nullptr m_encodeCtx->pbsBuffer.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pbsBuffer->BufferSize = m_encodeCtx->wPicHeightInMB * m_encodeCtx->wPicWidthInMB * PACKED_HEADER_SIZE_PER_ROW;
    m_encodeCtx->pbsBuffer->pBase      = (uint8_t *)MOS_AllocAndZeroMemory(m_encodeCtx->pbsBuffer->BufferSize);
    DDI_CHK_NULL(m_encodeCtx->pbsBuffer->pBase, "nullptr m_encodeCtx->pbsBuffer->pBase.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->ppNALUnitParams = (PCODECHAL_NAL_UNIT_PARAMS *)MOS_AllocAndZeroMemory(sizeof(PCODECHAL_NAL_UNIT_PARAMS) * 0x3F);
    DDI_CHK_NULL(m_encodeCtx->ppNALUnitParams, "nullptr m_encodeCtx->ppNALUnitParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    CODECHAL_NAL_UNIT_PARAMS *nalUnitParams = (CODECHAL_NAL_UNIT_PARAMS *)MOS_AllocAndZeroMemory(sizeof(CODECHAL_NAL_UNIT_PARAMS) * 0x3F);
    DDI_CHK_NULL(nalUnitParams, "nullptr nalUnitParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    for (int32_t i = 0; i < 0x3F; i++)
    {
        m_encodeCtx->ppNALUnitParams[i] = &(nalUnitParams[i]);
    }

    // Allocate segment params
    m_segParams = (CODEC_VP9_ENCODE_SEGMENT_PARAMS *)MOS_AllocAndZeroMemory(sizeof(CODEC_VP9_ENCODE_SEGMENT_PARAMS) * 8);
    DDI_CHK_NULL(m_segParams, "nullptr m_segParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

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

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp9::ParseSeqParams(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncSequenceParameterBufferVP9 *seqParams = (VAEncSequenceParameterBufferVP9 *)ptr;

    CODEC_VP9_ENCODE_SEQUENCE_PARAMS *vp9SeqParams = (PCODEC_VP9_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);
    DDI_CHK_NULL(vp9SeqParams, "nullptr vp9SeqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    MOS_ZeroMemory(vp9SeqParams, sizeof(CODEC_VP9_ENCODE_SEQUENCE_PARAMS));

    vp9SeqParams->wMaxFrameWidth   = seqParams->max_frame_width;
    vp9SeqParams->wMaxFrameHeight  = seqParams->max_frame_height;
    vp9SeqParams->GopPicSize       = seqParams->intra_period;
    vp9SeqParams->TargetBitRate[0] = MOS_ROUNDUP_DIVIDE(seqParams->bits_per_second, CODECHAL_ENCODE_BRC_KBPS);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp9::ParsePicParams(DDI_MEDIA_CONTEXT *mediaCtx, void *ptr)
{
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncPictureParameterBufferVP9 *picParams = (VAEncPictureParameterBufferVP9 *)ptr;

    CODEC_VP9_ENCODE_PIC_PARAMS *vp9SeqParams = (PCODEC_VP9_ENCODE_PIC_PARAMS)(m_encodeCtx->pPicParams);

    DDI_CHK_NULL(vp9SeqParams, "nullptr vp9SeqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    MOS_ZeroMemory(vp9SeqParams, sizeof(CODEC_VP9_ENCODE_PIC_PARAMS));

    vp9SeqParams->PicFlags.fields.frame_type = picParams->pic_flags.bits.frame_type;
    vp9SeqParams->SrcFrameWidthMinus1        = picParams->frame_width_src - 1;
    vp9SeqParams->SrcFrameHeightMinus1       = picParams->frame_height_src - 1;
    vp9SeqParams->PicFlags.fields.show_frame = picParams->pic_flags.bits.show_frame;

    vp9SeqParams->DstFrameWidthMinus1  = picParams->frame_width_dst;
    vp9SeqParams->DstFrameHeightMinus1 = picParams->frame_height_dst;

    vp9SeqParams->filter_level = picParams->filter_level;

    vp9SeqParams->LumaACQIndex        = picParams->luma_ac_qindex;
    vp9SeqParams->LumaDCQIndexDelta   = picParams->luma_dc_qindex_delta;
    vp9SeqParams->ChromaACQIndexDelta = picParams->chroma_ac_qindex_delta;
    vp9SeqParams->ChromaDCQIndexDelta = picParams->chroma_dc_qindex_delta;

    vp9SeqParams->BitOffsetForFirstPartitionSize = picParams->bit_offset_first_partition_size;

    vp9SeqParams->RefFlags.fields.LastRefIdx   = picParams->ref_flags.bits.ref_last_idx;
    vp9SeqParams->RefFlags.fields.GoldenRefIdx = picParams->ref_flags.bits.ref_gf_idx;
    vp9SeqParams->RefFlags.fields.AltRefIdx    = picParams->ref_flags.bits.ref_arf_idx;

    vp9SeqParams->RefFlags.fields.ref_frame_ctrl_l0 = picParams->ref_flags.bits.ref_frame_ctrl_l0;
    vp9SeqParams->RefFlags.fields.ref_frame_ctrl_l1 = picParams->ref_flags.bits.ref_frame_ctrl_l1;

    for (int32_t i = 0; i < 4; i++)
    {
        vp9SeqParams->LFRefDelta[i] = picParams->ref_lf_delta[i];
    }

    vp9SeqParams->LFModeDelta[0] = picParams->mode_lf_delta[0];
    vp9SeqParams->LFModeDelta[1] = picParams->mode_lf_delta[1];

    vp9SeqParams->sharpness_level = picParams->sharpness_level;

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_encodeCtx->RTtbl);

    SetupCodecPicture(mediaCtx, rtTbl, &vp9SeqParams->CurrReconstructedPic,
                                             picParams->reconstructed_frame, false);

    rtTbl->pCurrentReconTarget = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParams->reconstructed_frame);
    DDI_CHK_NULL(rtTbl->pCurrentReconTarget, "nullptr rtTbl->pCurrentReconTarget", VA_STATUS_ERROR_INVALID_PARAMETER);

    // curr orig pic
    vp9SeqParams->CurrOriginalPic.FrameIdx = GetRenderTargetID(rtTbl, rtTbl->pCurrentReconTarget);
    vp9SeqParams->CurrOriginalPic.PicFlags = vp9SeqParams->CurrReconstructedPic.PicFlags;

    for (int32_t i = 0; i < 3; i++)
    {
        if (picParams->reference_frames[i] != VA_INVALID_SURFACE)
        {
            SetupCodecPicture(mediaCtx, rtTbl, &vp9SeqParams->RefFrameList[i],
                                         picParams->reference_frames[i], true);
        }
    }

    DDI_MEDIA_BUFFER *buf = nullptr;

    buf = DdiMedia_GetBufferFromVABufferID(mediaCtx, picParams->coded_buf);
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

    // VP9 will always only have 1 NAL type (PPS)
    m_encodeCtx->ppNALUnitParams[0]->uiNalUnitType             = 0x22;
    m_encodeCtx->ppNALUnitParams[0]->bInsertEmulationBytes     = (packedHeaderParamBuf->has_emulation_bytes) ? false : true;
    m_encodeCtx->ppNALUnitParams[0]->uiSkipEmulationCheckCount = 4;  // Does this need to change??
    m_encodeCtx->ppNALUnitParams[0]->uiSize                    = (packedHeaderParamBuf->bit_length + 7) / 8;
    m_encodeCtx->ppNALUnitParams[0]->uiOffset                  = 0;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp9::ParsePackedHeaderData(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    BSBuffer *bsBuffer = m_encodeCtx->pbsBuffer;
    DDI_CHK_NULL(bsBuffer, "nullptr bsBuffer", VA_STATUS_ERROR_INVALID_PARAMETER);

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

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp9::ParseFrameRate(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

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

    for (int32_t i = 0; i < 8; ++i)
    {
        m_segParams->SegData[i].SegmentQIndexDelta  = segParams->seg_data[i].segment_qindex_delta;
        m_segParams->SegData[i].SegmentLFLevelDelta = segParams->seg_data[i].segment_lf_level_delta;
    }

    return VA_STATUS_SUCCESS;
}

void DdiEncodeVp9::ParseMiscParamVBV(void *data)
{
    VAEncMiscParameterHRD *vaEncMiscParamHRD = (VAEncMiscParameterHRD *)data;

    CODEC_VP9_ENCODE_SEQUENCE_PARAMS *seqParams = (CODEC_VP9_ENCODE_SEQUENCE_PARAMS *)m_encodeCtx->pSeqParams;

    seqParams->VBVBufferSizeInBit         = vaEncMiscParamHRD->buffer_size;
    seqParams->InitVBVBufferFullnessInBit = vaEncMiscParamHRD->initial_buffer_fullness;

    seqParams->RateControlMethod = RATECONTROL_CBR;
}

// Parse the frame rate paramters from app
void DdiEncodeVp9::ParseMiscParamFR(void *data)
{
    VAEncMiscParameterFrameRate *vaFrameRate = (VAEncMiscParameterFrameRate *)data;
    vaFrameRate->framerate                   = vaFrameRate->framerate / 100;

    uint32_t *frameRate = &(m_encodeCtx->uFrameRate);

    MOS_SecureMemcpy(frameRate, sizeof(uint32_t), (void *)&vaFrameRate->framerate, sizeof(uint32_t));
}

// Parse rate control related information from app
void DdiEncodeVp9::ParseMiscParamRC(void *data)
{
    CODEC_VP9_ENCODE_SEQUENCE_PARAMS *seqParams = (PCODEC_VP9_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);

    VAEncMiscParameterRateControl *vaEncMiscParamRC = (VAEncMiscParameterRateControl *)data;

    seqParams->MaxBitRate                = MOS_ROUNDUP_DIVIDE(vaEncMiscParamRC->bits_per_second, CODECHAL_ENCODE_BRC_KBPS);
    seqParams->SeqFlags.fields.bResetBRC = vaEncMiscParamRC->rc_flags.bits.reset;  // adding reset here. will apply both CBR and VBR

    if (VA_RC_CBR == m_encodeCtx->uiRCMethod)
    {
        seqParams->MinBitRate        = seqParams->MaxBitRate;
        seqParams->RateControlMethod = RATECONTROL_CBR;
    }
    else if (VA_RC_VBR == m_encodeCtx->uiRCMethod)
    {
        seqParams->MinBitRate        = seqParams->MaxBitRate * (2 * vaEncMiscParamRC->target_percentage - 100) / 100;
        seqParams->TargetBitRate[0]  = seqParams->MaxBitRate * vaEncMiscParamRC->target_percentage / 100;  // VBR target bits
        seqParams->RateControlMethod = RATECONTROL_VBR;
        if ((m_encodeCtx->uiTargetBitRate != seqParams->TargetBitRate[0]) ||
            (m_encodeCtx->uiMaxBitRate != seqParams->MaxBitRate))
        {
            seqParams->SeqFlags.fields.bResetBRC = 0x1;
            m_encodeCtx->uiTargetBitRate         = seqParams->TargetBitRate[0];
            m_encodeCtx->uiMaxBitRate            = seqParams->MaxBitRate;
        }
    }
}

void DdiEncodeVp9::ParseMiscParamEncQuality(void *data)
{
    DDI_UNUSED(m_encodeCtx);
    DDI_UNUSED(data);
    return;
}

VAStatus DdiEncodeVp9::ParseSegMapParams(DDI_MEDIA_BUFFER *buf)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);

    MOS_ZeroMemory(&(m_encodeCtx->segMapBuffer), sizeof(MOS_SURFACE));
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

    switch ((int32_t)(miscParamBuf->type))
    {
    case VAEncMiscParameterTypeHRD:
    {
        ParseMiscParamVBV((void *)miscParamBuf->data);
        break;
    }
    case VAEncMiscParameterTypeFrameRate:
    {
        ParseMiscParamFR((void *)miscParamBuf->data);
        break;
    }
    case VAEncMiscParameterTypeRateControl:
    {
        ParseMiscParamRC((void *)miscParamBuf->data);
        break;
    }
    case VAEncMiscParameterTypeEncQuality:
    {
        ParseMiscParamEncQuality((void *)miscParamBuf->data);
        break;
    }
    default:
    {
        DDI_ASSERTMESSAGE("DDI: unsupported misc parameter type.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    }

    return VA_STATUS_SUCCESS;
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
