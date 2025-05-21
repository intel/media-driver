/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     ddi_encode_av1_specific.cpp
//! \brief    HEVC class definition for DDI media encoder.
//!

#include "ddi_encode_base_specific.h"
#include "ddi_encode_av1_specific.h"
#include "media_libva_util_next.h"
#include "media_ddi_encode_const.h"
#include "media_ddi_factory.h"
#include "media_libva_interface_next.h"

#include "mos_solo_generic.h"

namespace encode
{

DdiEncodeAV1::~DdiEncodeAV1()
{
    if (m_encodeCtx == nullptr)
    {
        return;
    }

    MOS_FreeMemory(m_encodeCtx->pSeqParams);
    m_encodeCtx->pSeqParams = nullptr;

    MOS_FreeMemory(m_encodeCtx->pPicParams);
    m_encodeCtx->pPicParams = nullptr;

    MOS_FreeMemory(m_encodeCtx->pSliceParams);
    m_encodeCtx->pSliceParams = nullptr;

    MOS_FreeMemory(m_encodeCtx->pEncodeStatusReport);
    m_encodeCtx->pEncodeStatusReport = nullptr;

    MOS_FreeMemory(m_encodeCtx->pSliceHeaderData);
    m_encodeCtx->pSliceHeaderData = nullptr;

    if (m_encodeCtx->pbsBuffer)
    {
        MOS_FreeMemory(m_encodeCtx->pbsBuffer->pBase);
        m_encodeCtx->pbsBuffer->pBase = nullptr;
    }
    MOS_FreeMemory(m_encodeCtx->pbsBuffer);
    m_encodeCtx->pbsBuffer = nullptr;

    if (m_encodeCtx->ppNALUnitParams && m_encodeCtx->ppNALUnitParams[0])
    {
        // Allocate one contiguous memory for the NALUnitParams buffers
        // only need to free one time
        MOS_FreeMemory(m_encodeCtx->ppNALUnitParams[0]);
        m_encodeCtx->ppNALUnitParams[0] = nullptr;
    }

    MOS_FreeMemory(m_encodeCtx->ppNALUnitParams);
    m_encodeCtx->ppNALUnitParams = nullptr;
}

VAStatus DdiEncodeAV1::ContextInitialize(
    CodechalSetting *codecHalSettings)
{
    DDI_CODEC_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(m_encodeCtx->pCpDdiInterfaceNext, "nullptr m_encodeCtx->pCpDdiInterfaceNext.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(codecHalSettings, "nullptr codecHalSettings.", VA_STATUS_ERROR_INVALID_CONTEXT);

    codecHalSettings->codecFunction    = m_encodeCtx->codecFunction;
    codecHalSettings->width            = m_encodeCtx->dworiFrameWidth;
    codecHalSettings->height           = m_encodeCtx->dworiFrameHeight;
    codecHalSettings->mode             = m_encodeCtx->wModeType;
    codecHalSettings->standard         = CODECHAL_AV1;
    codecHalSettings->chromaFormat     = AVP_CHROMA_FORMAT_YUV420;
    codecHalSettings->lumaChromaDepth  = CODECHAL_LUMA_CHROMA_DEPTH_10_BITS;

    VAStatus eStatus = VA_STATUS_SUCCESS;

    // Allocate sequence params
    m_encodeCtx->pSeqParams = (void *)MOS_AllocAndZeroMemory(sizeof(CODEC_AV1_ENCODE_SEQUENCE_PARAMS));
    DDI_CODEC_CHK_NULL(m_encodeCtx->pSeqParams, "nullptr m_encodeCtx->pSeqParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Allocate picture params
    m_encodeCtx->pPicParams = (void *)MOS_AllocAndZeroMemory(sizeof(CODEC_AV1_ENCODE_PICTURE_PARAMS));
    DDI_CODEC_CHK_NULL(m_encodeCtx->pPicParams, "nullptr m_encodeCtx->pPicParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Allocate slice params
    m_encodeCtx->pSliceParams = (void *)MOS_AllocAndZeroMemory(TILE_GROUP_NUM_INCREMENT * sizeof(CODEC_AV1_ENCODE_TILE_GROUP_PARAMS));
    DDI_CODEC_CHK_NULL(m_encodeCtx->pSliceParams, "nullptr m_encodeCtx->pSliceParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);
    allocatedTileNum = TILE_GROUP_NUM_INCREMENT;

    // Allocate encode status report
    m_encodeCtx->pEncodeStatusReport = (void *)MOS_AllocAndZeroMemory(CODECHAL_ENCODE_STATUS_NUM*sizeof(EncodeStatusReportData));
    DDI_CODEC_CHK_NULL(m_encodeCtx->pEncodeStatusReport, "nullptr m_encodeCtx->pEncodeStatusReport.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    //Allocate Slice Header Data
    m_encodeCtx->pSliceHeaderData = (CODEC_ENCODER_SLCDATA *)MOS_AllocAndZeroMemory(ENCODE_VDENC_AV1_MAX_TILE_GROUP_NUM * sizeof(CODEC_ENCODER_SLCDATA));
    DDI_CODEC_CHK_NULL(m_encodeCtx->pSliceHeaderData, "nullptr m_encodeCtx->pSliceHeaderData.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Create the bit stream buffer to hold the packed headers from application
    m_encodeCtx->pbsBuffer = (PBSBuffer)MOS_AllocAndZeroMemory(sizeof(BSBuffer));
    DDI_CODEC_CHK_NULL(m_encodeCtx->pbsBuffer, "nullptr m_encodeCtx->pbsBuffer.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pbsBuffer->BufferSize = CODECHAL_AV1_FRAME_HEADER_SIZE;
    m_encodeCtx->pbsBuffer->pBase      = (uint8_t *)MOS_AllocAndZeroMemory(m_encodeCtx->pbsBuffer->BufferSize);
    DDI_CODEC_CHK_NULL(m_encodeCtx->pbsBuffer, "nullptr m_encodeCtx->pbsBuffer.", VA_STATUS_ERROR_ALLOCATION_FAILED);
    m_encodeCtx->pbsBuffer->pCurrent      = m_encodeCtx->pbsBuffer->pBase;

    // Allocate NAL unit params
    m_encodeCtx->ppNALUnitParams = (PCODECHAL_NAL_UNIT_PARAMS *)MOS_AllocAndZeroMemory(sizeof(PCODECHAL_NAL_UNIT_PARAMS)*MAX_NUM_OBU_TYPES);
    DDI_CODEC_CHK_NULL(m_encodeCtx->ppNALUnitParams, "nullptr m_encodeCtx->ppNALUnitParams", VA_STATUS_ERROR_ALLOCATION_FAILED);

    PCODECHAL_NAL_UNIT_PARAMS nalUnitParams = (CODECHAL_NAL_UNIT_PARAMS *)MOS_AllocAndZeroMemory(sizeof(CODECHAL_NAL_UNIT_PARAMS)*MAX_NUM_OBU_TYPES);
    DDI_CODEC_CHK_NULL(nalUnitParams, "nullptr nalUnitParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    for (uint32_t i = 0; i < MAX_NUM_OBU_TYPES; i++)
    {
        m_encodeCtx->ppNALUnitParams[i] = &(nalUnitParams[i]);
    }

    m_cpuFormat = true;

    if (Mos_Solo_Extension(m_encodeCtx->pCodecHal->GetOsInterface()->pOsContext))
    {
        DDI_CODEC_CHK_NULL(m_encodeCtx->pCodecHal, "nullptr m_encodeCtx->pCodecHal", VA_STATUS_ERROR_INVALID_CONTEXT);
        DDI_CODEC_CHK_NULL(m_encodeCtx->pCodecHal, "nullptr m_encodeCtx->pCodecHal->GetOsInterface()", VA_STATUS_ERROR_INVALID_CONTEXT);
        DDI_CODEC_CHK_NULL(m_encodeCtx->pCodecHal, "nullptr (m_encodeCtx->pCodecHal->GetOsInterface()->pOsContext", VA_STATUS_ERROR_INVALID_CONTEXT);
        DDI_CODEC_CHK_NULL(m_encodeCtx->pMediaCtx, "nullptr pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
        Mos_Solo_CreateExtension(ENCODE_MODE_AV1, m_encodeCtx->pMediaCtx->platform.eProductFamily, m_encodeCtx->pCodecHal->GetOsInterface()->pOsContext);
    }

    return eStatus;
}

VAStatus DdiEncodeAV1::RenderPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VABufferID       *buffers,
    int32_t          numBuffers)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;

    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_MEDIA_CONTEXT *mediaCtx = GetMediaContext(ctx);
    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CODEC_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(buffers, "nullptr buffers", VA_STATUS_ERROR_INVALID_BUFFER);
    for (int32_t i = 0; i < numBuffers; i++)
    {
        DDI_MEDIA_BUFFER *buf = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, buffers[i]);
        DDI_CODEC_CHK_NULL(buf, "Invalid buffer.", VA_STATUS_ERROR_INVALID_BUFFER);
        if (buf->uiType == VAEncMacroblockDisableSkipMapBufferType)
        {
            MediaLibvaCommonNext::MediaBufferToMosResource(buf, &(m_encodeCtx->resPerMBSkipMapBuffer));
            m_encodeCtx->bMbDisableSkipMapEnabled = true;
            continue;
        }
        uint32_t dataSize = buf->iSize;
        //can use internal function instead of MediaLibvaInterfaceNext::MapBuffer here?
        void *data = nullptr;
        MediaLibvaInterfaceNext::MapBuffer(ctx, buffers[i], &data);

        DDI_CODEC_CHK_NULL(data, "nullptr data.", VA_STATUS_ERROR_INVALID_BUFFER);

        switch (buf->uiType)
        {
        case VAEncSequenceParameterBufferType:
        {
            DDI_CHK_STATUS(ParseSeqParams(data), VA_STATUS_ERROR_INVALID_BUFFER);
            m_encodeCtx->bNewSeq = true;
            break;
        }
        case VAEncPictureParameterBufferType:
        {
            DDI_CHK_STATUS(ParsePicParams(mediaCtx, data), VA_STATUS_ERROR_INVALID_BUFFER);
            DDI_CHK_STATUS(
                AddToStatusReportQueue((void *)m_encodeCtx->resBitstreamBuffer.bo),
                VA_STATUS_ERROR_INVALID_BUFFER);
            break;
        }
        case VAEncSliceParameterBufferType:
        {
            uint32_t numTiles = buf->uiNumElements;
            DDI_CHK_STATUS(ParseTileGroupParams(data, numTiles), VA_STATUS_ERROR_INVALID_BUFFER);
            break;
        }
        case VAEncPackedHeaderParameterBufferType:
        {
            DDI_CHK_STATUS(ParsePackedHeaderParams(data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;
        }
        case VAEncPackedHeaderDataBufferType:
        {
            DDI_CHK_STATUS(ParsePackedHeaderData(data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;
        }
        case VAEncMiscParameterBufferType:
        {
            DDI_CHK_STATUS(ParseMiscParams(data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;
        }
        case VAEncMacroblockMapBufferType:
        {
            DDI_CHK_STATUS(ParseSegMapParams(data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;
        }
        default:
        {
            DDI_CODEC_ASSERTMESSAGE("not supported buffer type.");
            break;
        }
        }
        MediaLibvaInterfaceNext::UnmapBuffer(ctx, buffers[i]);
    }

    DDI_FUNCTION_EXIT(vaStatus);
    return vaStatus;
}

VAStatus DdiEncodeAV1::EncodeInCodecHal(uint32_t numSlices)
{
    DDI_CODEC_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(m_encodeCtx->pCodecHal, "nullptr m_encodeCtx->pCodecHal", VA_STATUS_ERROR_INVALID_CONTEXT);

    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS av1SeqParams = (PCODEC_AV1_ENCODE_SEQUENCE_PARAMS)((uint8_t *)m_encodeCtx->pSeqParams);
    DDI_CODEC_CHK_NULL(av1SeqParams, "nullptr av1SeqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_encodeCtx->RTtbl);

    EncoderParamsAV1 encodeParams;
    MOS_ZeroMemory(&encodeParams, sizeof(encodeParams));
    encodeParams.ExecCodecFunction = m_encodeCtx->codecFunction;

    /* check whether the target bit rate is initialized for BRC */
    if ((VA_RC_CBR == m_encodeCtx->uiRCMethod) ||
        (VA_RC_VBR == m_encodeCtx->uiRCMethod))
    {
        if (av1SeqParams->TargetBitRate[0] == 0)
        {
            DDI_CODEC_ASSERTMESSAGE("DDI: No RateControl param for BRC\n!");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
    }

    // Raw Surface
    MOS_SURFACE rawSurface;
    MOS_ZeroMemory(&rawSurface, sizeof(rawSurface));
    rawSurface.dwOffset = 0;

    MediaLibvaCommonNext::MediaSurfaceToMosResource(rtTbl->pCurrentRT, &(rawSurface.OsResource));

    // Recon Surface
    MOS_SURFACE reconSurface;
    MOS_ZeroMemory(&reconSurface, sizeof(reconSurface));
    reconSurface.dwOffset = 0;

    MediaLibvaCommonNext::MediaSurfaceToMosResource(rtTbl->pCurrentReconTarget, &(reconSurface.OsResource));

    // Clear registered recon/ref surface flags
    DDI_CODEC_CHK_RET(ClearRefList(&m_encodeCtx->RTtbl, true), "ClearRefList failed!");

    // Bitsream surface
    MOS_RESOURCE bitstreamSurface;
    MOS_ZeroMemory(&bitstreamSurface, sizeof(bitstreamSurface));
    bitstreamSurface        = m_encodeCtx->resBitstreamBuffer;
    bitstreamSurface.Format = Format_Buffer;

    encodeParams.psRawSurface        = &rawSurface;
    encodeParams.psReconSurface      = &reconSurface;
    encodeParams.presBitstreamBuffer = &bitstreamSurface;

    // Segmentation map won't be sent for each frame, so need to reset params before each frame.
    encodeParams.pSegmentMap         = nullptr;
    encodeParams.bSegmentMapProvided = false;

    if (m_isSegParamsChanged)
    {
        encodeParams.pSegmentMap         = m_encodeCtx->pSegmentMap;
        encodeParams.segmentMapDataSize  = m_encodeCtx->segmentMapDataSize;
        encodeParams.bSegmentMapProvided = true;
        m_isSegParamsChanged             = false;
    }

    if (m_encodeCtx->bNewSeq)
    {
        av1SeqParams->TargetUsage = m_encodeCtx->targetUsage;
    }

    encodeParams.pSeqParams      = m_encodeCtx->pSeqParams;
    encodeParams.pPicParams      = m_encodeCtx->pPicParams;
    encodeParams.pSliceParams    = m_encodeCtx->pSliceParams;
    encodeParams.ppNALUnitParams = m_encodeCtx->ppNALUnitParams;
    encodeParams.uiNumNalUnits   = m_encodeCtx->indexNALUnit;

    // Sequence data
    encodeParams.bNewSeq = m_encodeCtx->bNewSeq;
    if (av1SeqParams->SeqFlags.fields.ResetBRC)
    {
        /* When the BRC needs to be reset, it indicates that the new Seq is issued. */
        encodeParams.bNewSeq = true;
    }

    // Tile level data
    encodeParams.dwNumSlices = numSlices;

    for (uint32_t i = 0; i < (av1SeqParams->NumTemporalLayersMinus1 + 1); i++)
    {
        if (savedFrameRate[i] == 0)
        {
            /* use the default framerate if FrameRate is not passed */
            av1SeqParams->FrameRate[i].Numerator   = 30;
            av1SeqParams->FrameRate[i].Denominator = 1;
        }
    }

    encodeParams.pSlcHeaderData = (void *)m_encodeCtx->pSliceHeaderData;
    encodeParams.pBSBuffer = m_encodeCtx->pbsBuffer;

    MOS_STATUS status = m_encodeCtx->pCodecHal->Execute(&encodeParams);
    if (MOS_STATUS_SUCCESS != status)
    {
        DDI_CODEC_ASSERTMESSAGE("DDI:Failed in Codechal!");
        return VA_STATUS_ERROR_ENCODING_ERROR;
    }

    return VA_STATUS_SUCCESS;
}

// Reset the parameters before each frame
VAStatus DdiEncodeAV1::ResetAtFrameLevel()
{
    DDI_CODEC_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);

    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS av1SeqParams = (PCODEC_AV1_ENCODE_SEQUENCE_PARAMS)((uint8_t *)m_encodeCtx->pSeqParams);
    DDI_CODEC_CHK_NULL(av1SeqParams, "nullptr av1SeqParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    av1SeqParams->SeqFlags.fields.ResetBRC = 0;

    m_encodeCtx->dwNumSlices = 0;
    m_encodeCtx->bNewSeq     = false;

    // reset bsbuffer every frame
    PBSBuffer pBSBuffer    = m_encodeCtx->pbsBuffer;
    DDI_CODEC_CHK_NULL(pBSBuffer, "nullptr bsBuffer.", VA_STATUS_ERROR_INVALID_PARAMETER);

    *(pBSBuffer->pBase)    = 0; //init first byte to 0
    pBSBuffer->pCurrent    = pBSBuffer->pBase;
    pBSBuffer->SliceOffset = 0;
    pBSBuffer->BitOffset   = 0;
    pBSBuffer->BitSize     = 0;

    // clear the packed header information
    if (nullptr != m_encodeCtx->ppNALUnitParams)
    {
        MOS_ZeroMemory(m_encodeCtx->ppNALUnitParams[0], sizeof(CODECHAL_NAL_UNIT_PARAMS)*(m_encodeCtx->indexNALUnit));
    }
    m_encodeCtx->indexNALUnit = 0;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAV1::ParseSeqParams(void *ptr)
{
    DDI_CODEC_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CODEC_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncSequenceParameterBufferAV1 *seqParams = (VAEncSequenceParameterBufferAV1 *)ptr;

    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS av1SeqParams = (PCODEC_AV1_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);
    DDI_CODEC_CHK_NULL(av1SeqParams, "nullptr av1SeqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    av1SeqParams->seq_profile   = seqParams->seq_profile;
    av1SeqParams->seq_level_idx = seqParams->seq_level_idx;
    av1SeqParams->GopPicSize    = seqParams->intra_period;
    av1SeqParams->GopRefDist    = seqParams->ip_period;
 
    switch ((uint32_t)m_encodeCtx->uiRCMethod)
    {
    case VA_RC_TCBRC:
    case VA_RC_VBR:
        av1SeqParams->RateControlMethod = (uint8_t)RATECONTROL_VBR;
        break;
    case VA_RC_CQP:
        av1SeqParams->RateControlMethod = (uint8_t)RATECONTROL_CQP;
        break;
    case VA_RC_ICQ:
        av1SeqParams->RateControlMethod = (uint8_t)RATECONTROL_CQL;
        break;
    default:
        av1SeqParams->RateControlMethod = (uint8_t)RATECONTROL_CBR;
    }

    /* the bits_per_second is only used when the target bit_rate is not initialized */
    if (av1SeqParams->TargetBitRate[0] == 0)
    {
        av1SeqParams->TargetBitRate[0] = MOS_ROUNDUP_DIVIDE(seqParams->bits_per_second, CODECHAL_ENCODE_BRC_KBPS);
    }
    av1SeqParams->MaxBitRate = MOS_ROUNDUP_DIVIDE(seqParams->bits_per_second, CODECHAL_ENCODE_BRC_KBPS);
    av1SeqParams->MinBitRate = MOS_ROUNDUP_DIVIDE(seqParams->bits_per_second, CODECHAL_ENCODE_BRC_KBPS);

    //set default same as application, can be overwritten by misc params
    av1SeqParams->InitVBVBufferFullnessInBit = seqParams->bits_per_second;
    av1SeqParams->VBVBufferSizeInBit = seqParams->bits_per_second << 1;

    av1SeqParams->CodingToolFlags.fields.enable_order_hint    = seqParams->seq_fields.bits.enable_order_hint;
    av1SeqParams->CodingToolFlags.fields.enable_cdef          = seqParams->seq_fields.bits.enable_cdef;
    av1SeqParams->CodingToolFlags.fields.enable_warped_motion = seqParams->seq_fields.bits.enable_warped_motion;
    av1SeqParams->CodingToolFlags.fields.enable_restoration   = seqParams->seq_fields.bits.enable_restoration;

    av1SeqParams->order_hint_bits_minus_1 = seqParams->order_hint_bits_minus_1;
#if VA_CHECK_VERSION(1, 16, 0)
    av1SeqParams->SeqFlags.fields.HierarchicalFlag = seqParams->hierarchical_flag;
#else
    av1SeqParams->SeqFlags.fields.HierarchicalFlag = seqParams->reserved8b;
#endif
    av1SeqParams->SeqFlags.fields.DisplayFormatSwizzle = NeedDisplayFormatSwizzle(m_encodeCtx->RTtbl.pCurrentRT);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAV1::ParsePicParams(DDI_MEDIA_CONTEXT *mediaCtx, void *ptr)
{
    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CODEC_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CODEC_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncPictureParameterBufferAV1 *picParams = (VAEncPictureParameterBufferAV1 *)ptr;

    PCODEC_AV1_ENCODE_PICTURE_PARAMS av1PicParams = (PCODEC_AV1_ENCODE_PICTURE_PARAMS)(m_encodeCtx->pPicParams);
    DDI_CODEC_CHK_NULL(av1PicParams, "nullptr av1PicParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    MOS_ZeroMemory(av1PicParams, sizeof(CODEC_AV1_ENCODE_PICTURE_PARAMS));

    av1PicParams->frame_width_minus1  = picParams->frame_width_minus_1;
    av1PicParams->frame_height_minus1 = picParams->frame_height_minus_1;
    av1PicParams->NumTileGroupsMinus1 = picParams->num_tile_groups_minus1;
    av1PicParams->PicFlags.fields.EnableFrameOBU = picParams->picture_flags.bits.enable_frame_obu;

    av1PicParams->LoopRestorationFlags.fields.yframe_restoration_type  = picParams->loop_restoration_flags.bits.yframe_restoration_type;
    av1PicParams->LoopRestorationFlags.fields.cbframe_restoration_type = picParams->loop_restoration_flags.bits.cbframe_restoration_type;
    av1PicParams->LoopRestorationFlags.fields.crframe_restoration_type = picParams->loop_restoration_flags.bits.crframe_restoration_type;
    av1PicParams->LoopRestorationFlags.fields.lr_unit_shift            = picParams->loop_restoration_flags.bits.lr_unit_shift;
    av1PicParams->LoopRestorationFlags.fields.lr_uv_shift              = picParams->loop_restoration_flags.bits.lr_uv_shift;

    if (picParams->num_tile_groups_minus1 > 0 && picParams->picture_flags.bits.enable_frame_obu)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_encodeCtx->RTtbl);

    DDI_MEDIA_SURFACE *recon = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, picParams->reconstructed_frame);

    DDI_CODEC_CHK_RET(RegisterRTSurfaces(rtTbl, recon), "RegisterRTSurfaces failed");

    SetupCodecPicture(
        mediaCtx,
        rtTbl,
        &av1PicParams->CurrReconstructedPic,
        picParams->reconstructed_frame,
        false);

    rtTbl->pCurrentReconTarget = recon;
    DDI_CODEC_CHK_NULL(rtTbl->pCurrentReconTarget, "NULL rtTbl->pCurrentReconTarget", VA_STATUS_ERROR_INVALID_PARAMETER);

    // curr origin pic
    av1PicParams->CurrOriginalPic.FrameIdx = (uint8_t)GetRenderTargetID(rtTbl, rtTbl->pCurrentReconTarget);
    av1PicParams->CurrOriginalPic.PicFlags = av1PicParams->CurrReconstructedPic.PicFlags;
    av1PicParams->CurrOriginalPic.PicEntry = av1PicParams->CurrReconstructedPic.PicEntry;

    // RefFrame List
    for (uint32_t i = 0; i < 8; i++)
    {
        if (picParams->reference_frames[i] != VA_INVALID_SURFACE)
        {
            UpdateRegisteredRTSurfaceFlag(rtTbl, MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, picParams->reference_frames[i]));
        }
        SetupCodecPicture(
            mediaCtx,
            rtTbl,
            &av1PicParams->RefFrameList[i],
            picParams->reference_frames[i],
            true);
    }

    // bitstream buffer
    DDI_MEDIA_BUFFER *buf = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, picParams->coded_buf);
    DDI_CODEC_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);
    RemoveFromStatusReportQueue(buf);
    MediaLibvaCommonNext::MediaBufferToMosResource(buf, &(m_encodeCtx->resBitstreamBuffer));

    //reference frame index
    DDI_CODEC_CHK_RET(
        MOS_SecureMemcpy(av1PicParams->ref_frame_idx,
            sizeof(av1PicParams->ref_frame_idx),
            picParams->ref_frame_idx,
            sizeof(picParams->ref_frame_idx)),
        "DDI: PicParams parsing failed!");

    if ((picParams->picture_flags.bits.frame_type == intraOnlyFrame || picParams->picture_flags.bits.frame_type == keyFrame) &&
         picParams->primary_ref_frame != av1PrimaryRefNone)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    av1PicParams->primary_ref_frame = picParams->primary_ref_frame;
    av1PicParams->order_hint        = picParams->order_hint;

    av1PicParams->ref_frame_ctrl_l0.RefFrameCtrl.value = picParams->ref_frame_ctrl_l0.value;
    av1PicParams->ref_frame_ctrl_l1.RefFrameCtrl.value = picParams->ref_frame_ctrl_l1.value;

    if (picParams->picture_flags.bits.frame_type == sFrame && picParams->picture_flags.bits.error_resilient_mode != 1)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    // picture flags
    av1PicParams->PicFlags.fields.frame_type                   = picParams->picture_flags.bits.frame_type;
    av1PicParams->PicFlags.fields.error_resilient_mode         = picParams->picture_flags.bits.error_resilient_mode;
    av1PicParams->PicFlags.fields.disable_cdf_update           = picParams->picture_flags.bits.disable_cdf_update;
    av1PicParams->PicFlags.fields.allow_high_precision_mv      = picParams->picture_flags.bits.allow_high_precision_mv;
    av1PicParams->PicFlags.fields.use_ref_frame_mvs            = picParams->picture_flags.bits.use_ref_frame_mvs;
    av1PicParams->PicFlags.fields.disable_frame_end_update_cdf = picParams->picture_flags.bits.disable_frame_end_update_cdf;
    av1PicParams->PicFlags.fields.reduced_tx_set_used          = picParams->picture_flags.bits.reduced_tx_set;
    av1PicParams->PicFlags.fields.EnableFrameOBU               = picParams->picture_flags.bits.enable_frame_obu;
    av1PicParams->PicFlags.fields.LongTermReference            = picParams->picture_flags.bits.long_term_reference;
    av1PicParams->PicFlags.fields.DisableFrameRecon            = picParams->picture_flags.bits.disable_frame_recon;
    av1PicParams->PicFlags.fields.PaletteModeEnable            = picParams->picture_flags.bits.palette_mode_enable;
    av1PicParams->PicFlags.fields.allow_intrabc                = picParams->picture_flags.bits.allow_intrabc;
    av1PicParams->PicFlags.fields.SegIdBlockSize               = picParams->seg_id_block_size;

#if VA_CHECK_VERSION(1, 16, 0)
    av1PicParams->HierarchLevelPlus1                           = picParams->hierarchical_level_plus1;
#else
    av1PicParams->HierarchLevelPlus1                           = picParams->reserved8bits0;
#endif

    DDI_CODEC_CHK_RET(
        MOS_SecureMemcpy(av1PicParams->filter_level,
            sizeof(av1PicParams->filter_level),
            picParams->filter_level,
            sizeof(picParams->filter_level)),
        "DDI: PicParams parsing failed!");

    av1PicParams->filter_level_u = picParams->filter_level_u;
    av1PicParams->filter_level_v = picParams->filter_level_v;

    av1PicParams->cLoopFilterInfoFlags.value = picParams->loop_filter_flags.value;
    av1PicParams->interp_filter              = picParams->interpolation_filter;

    DDI_CODEC_CHK_RET(
        MOS_SecureMemcpy(av1PicParams->ref_deltas,
            sizeof(av1PicParams->ref_deltas),
            picParams->ref_deltas,
            sizeof(picParams->ref_deltas)),
        "DDI: PicParams parsing failed!");

    DDI_CODEC_CHK_RET(
        MOS_SecureMemcpy(av1PicParams->mode_deltas,
            sizeof(av1PicParams->mode_deltas),
            picParams->mode_deltas,
            sizeof(picParams->mode_deltas)),
        "DDI: PicParams parsing failed!");

    if (abs(picParams->y_dc_delta_q) > 15)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    av1PicParams->base_qindex   = picParams->base_qindex;
    av1PicParams->y_dc_delta_q  = picParams->y_dc_delta_q;
    av1PicParams->u_dc_delta_q  = picParams->u_dc_delta_q;
    av1PicParams->u_ac_delta_q  = picParams->u_ac_delta_q;
    av1PicParams->v_dc_delta_q  = picParams->v_dc_delta_q;
    av1PicParams->v_ac_delta_q  = picParams->v_ac_delta_q;
    av1PicParams->MinBaseQIndex = picParams->min_base_qindex;
    av1PicParams->MaxBaseQIndex = picParams->max_base_qindex;

    // Quatization Matrix is not supportted
    if (picParams->qmatrix_flags.bits.using_qmatrix != 0)
    {
        DDI_CODEC_ASSERTMESSAGE("Invalid Parameter for Parsing AV1 Picture parameter");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    av1PicParams->wQMatrixFlags.value = picParams->qmatrix_flags.value;

    av1PicParams->dwModeControlFlags.value = picParams->mode_control_flags.value;

    //segment params
    av1PicParams->stAV1Segments.SegmentFlags.fields.segmentation_enabled = picParams->segments.seg_flags.bits.segmentation_enabled;
    av1PicParams->stAV1Segments.SegmentFlags.fields.SegmentNumber        = picParams->segments.segment_number;
    av1PicParams->stAV1Segments.SegmentFlags.fields.update_map           = picParams->segments.seg_flags.bits.segmentation_update_map;
    av1PicParams->stAV1Segments.SegmentFlags.fields.temporal_update      = picParams->segments.seg_flags.bits.segmentation_temporal_update;

    DDI_CODEC_CHK_RET(
        MOS_SecureMemcpy(av1PicParams->stAV1Segments.feature_data,
            sizeof(av1PicParams->stAV1Segments.feature_data),
            picParams->segments.feature_data,
            sizeof(picParams->segments.feature_data)),
        "DDI: PicParams parsing failed!");

    DDI_CODEC_CHK_RET(
        MOS_SecureMemcpy(av1PicParams->stAV1Segments.feature_mask,
            sizeof(av1PicParams->stAV1Segments.feature_mask),
            picParams->segments.feature_mask,
            sizeof(picParams->segments.feature_mask)),
        "DDI: PicParams parsing failed!");

    av1PicParams->tile_cols = picParams->tile_cols;

    DDI_CODEC_CHK_RET(
        MOS_SecureMemcpy(av1PicParams->width_in_sbs_minus_1,
            sizeof(av1PicParams->width_in_sbs_minus_1),
            picParams->width_in_sbs_minus_1,
            sizeof(picParams->width_in_sbs_minus_1)),
        "DDI: PicParams parsing failed!");

    av1PicParams->tile_rows = picParams->tile_rows;

    DDI_CODEC_CHK_RET(
        MOS_SecureMemcpy(av1PicParams->height_in_sbs_minus_1,
            sizeof(av1PicParams->height_in_sbs_minus_1),
            picParams->height_in_sbs_minus_1,
            sizeof(picParams->height_in_sbs_minus_1)),
        "DDI: PicParams parsing failed!");

    DDI_CODEC_CHK_RET(CheckCDEF(picParams, mediaCtx->platform.eProductFamily), "invalid CDEF Paramter");

    DDI_CODEC_CHK_RET(CheckTile(picParams), "invalid Tile Paramter");
    
    av1PicParams->context_update_tile_id = picParams->context_update_tile_id;
    av1PicParams->temporal_id            = picParams->temporal_id;

    av1PicParams->cdef_damping_minus_3   = picParams->cdef_damping_minus_3;
    av1PicParams->cdef_bits              = picParams->cdef_bits;

    DDI_CODEC_CHK_RET(
        MOS_SecureMemcpy(av1PicParams->cdef_y_strengths,
            sizeof(av1PicParams->cdef_y_strengths),
            picParams->cdef_y_strengths,
            sizeof(picParams->cdef_y_strengths)),
        "DDI: PicParams parsing failed!");

    DDI_CODEC_CHK_RET(
        MOS_SecureMemcpy(av1PicParams->cdef_uv_strengths,
            sizeof(av1PicParams->cdef_uv_strengths),
            picParams->cdef_uv_strengths,
            sizeof(picParams->cdef_uv_strengths)),
        "DDI: PicParams parsing failed!");

    for(uint32_t i = 0; i < 7; i++)
    {
        av1PicParams->wm[i].wmtype  = picParams->wm[i].wmtype;
        av1PicParams->wm[i].invalid = picParams->wm[i].invalid;

        DDI_CODEC_CHK_RET(
            MOS_SecureMemcpy(av1PicParams->wm[i].wmmat,
                sizeof(av1PicParams->wm[i].wmmat),
                picParams->wm[i].wmmat,
                sizeof(picParams->wm[i].wmmat)),
            "DDI: PicParams parsing failed!");
    }

    av1PicParams->QIndexBitOffset           = picParams->bit_offset_qindex;
    av1PicParams->SegmentationBitOffset     = picParams->bit_offset_segmentation;
    av1PicParams->LoopFilterParamsBitOffset = picParams->bit_offset_loopfilter_params;
    av1PicParams->CDEFParamsBitOffset       = picParams->bit_offset_cdef_params;
    av1PicParams->CDEFParamsSizeInBits      = picParams->size_in_bits_cdef_params;
    av1PicParams->FrameHdrOBUSizeByteOffset = picParams->byte_offset_frame_hdr_obu_size;
    av1PicParams->FrameHdrOBUSizeInBits     = picParams->size_in_bits_frame_hdr_obu;

    av1PicParams->TileGroupOBUHdrInfo.value = picParams->tile_group_obu_hdr_info.value;

    av1PicParams->NumSkipFrames             = picParams->number_skip_frames;
    av1PicParams->FrameSizeReducedInBytes   = 0 - picParams->skip_frames_reduced_size;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAV1::ParseTileGroupParams(void *ptr, uint32_t numTileGroupParams)
{
    DDI_CODEC_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CODEC_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncTileGroupBufferAV1 *vaEncTileGroupParams = (VAEncTileGroupBufferAV1 *)ptr;

    PCODEC_AV1_ENCODE_TILE_GROUP_PARAMS av1TileGroupParams = (PCODEC_AV1_ENCODE_TILE_GROUP_PARAMS)m_encodeCtx->pSliceParams;
    DDI_CODEC_CHK_NULL(av1TileGroupParams, "nullptr av1TileGroup", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (m_encodeCtx->dwNumSlices + numTileGroupParams > allocatedTileNum)
    {
        av1TileGroupParams = (PCODEC_AV1_ENCODE_TILE_GROUP_PARAMS)MOS_ReallocMemory(av1TileGroupParams,
            (m_encodeCtx->dwNumSlices + numTileGroupParams + TILE_GROUP_NUM_INCREMENT)*sizeof(CODEC_AV1_ENCODE_TILE_GROUP_PARAMS));
        DDI_CODEC_CHK_NULL(av1TileGroupParams, "nullptr av1TileGroupParams", VA_STATUS_ERROR_INVALID_PARAMETER);

        allocatedTileNum = m_encodeCtx->dwNumSlices + numTileGroupParams + TILE_GROUP_NUM_INCREMENT;
        m_encodeCtx->pSliceParams = (void *)av1TileGroupParams;
    }

    av1TileGroupParams += m_encodeCtx->dwNumSlices;
    MOS_ZeroMemory(av1TileGroupParams, (numTileGroupParams*sizeof(CODEC_AV1_ENCODE_TILE_GROUP_PARAMS)));

    for (uint32_t i = 0; i < numTileGroupParams; i++)
    {
        av1TileGroupParams->TileGroupStart = vaEncTileGroupParams->tg_start;
        av1TileGroupParams->TileGroupEnd   = vaEncTileGroupParams->tg_end;
        av1TileGroupParams++;
        vaEncTileGroupParams++;
    }

    m_encodeCtx->dwNumSlices += numTileGroupParams;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAV1::ParsePackedHeaderParams(void *ptr)
{
    DDI_CODEC_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CODEC_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncPackedHeaderParameterBuffer *encPackedHeaderParamBuf = (VAEncPackedHeaderParameterBuffer *)ptr;

    m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->bInsertEmulationBytes     = (encPackedHeaderParamBuf->has_emulation_bytes) ? false : true;
    m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiSkipEmulationCheckCount = 3;
    m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiSize                    = (encPackedHeaderParamBuf->bit_length + 7) / 8;
    m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiOffset                  = 0; // will be overwritten later.

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAV1::ParsePackedHeaderData(void *ptr)
{
    DDI_CODEC_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CODEC_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    BSBuffer *bsBuffer = m_encodeCtx->pbsBuffer;
    DDI_CODEC_CHK_NULL(bsBuffer, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (m_encodeCtx->indexNALUnit == 0)
    {
        bsBuffer->pCurrent    = bsBuffer->pBase;
        bsBuffer->SliceOffset = 0;
        bsBuffer->BitOffset   = 0;
        bsBuffer->BitSize     = 0;
    }

    uint32_t hdrDataSize = m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiSize;

    DDI_CODEC_CHK_RET(
        MOS_SecureMemcpy(bsBuffer->pCurrent, bsBuffer->BufferSize - bsBuffer->SliceOffset, (uint8_t *)ptr, hdrDataSize),
        "DDI:packed header size is too large to be supported!");

    m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiOffset = bsBuffer->pCurrent - bsBuffer->pBase;
    m_encodeCtx->indexNALUnit++;

    bsBuffer->pCurrent    += hdrDataSize;
    bsBuffer->SliceOffset += hdrDataSize;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAV1::ParseMiscParams(void *ptr)
{
    DDI_CODEC_CHK_NULL(m_encodeCtx, "nullptr m_enocdeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CODEC_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterBuffer *miscParamBuf = (VAEncMiscParameterBuffer *)ptr;
    DDI_CODEC_CHK_NULL(miscParamBuf->data, "nullptr miscParamBuf->data", VA_STATUS_ERROR_INVALID_PARAMETER);

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
        vaStatus = ParseMiscParamTemporalLayerParams((void *)miscParamBuf->data);
        break;
    }
    case VAEncMiscParameterTypeQualityLevel:
    {
        vaStatus = ParseMiscParamQualityLevel((void *)miscParamBuf->data);
        break;
    }
    case VAEncMiscParameterTypeMaxFrameSize:
    {
        vaStatus = ParseMiscParamMaxFrameSize((void *)miscParamBuf->data);
        break;
    }
    default:
    {
        DDI_CODEC_ASSERTMESSAGE("DDI: unsupported misc parameter type.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    }

    return vaStatus;
}

VAStatus DdiEncodeAV1::ParseSegMapParams(void *ptr)
{
    DDI_CODEC_CHK_NULL(m_encodeCtx, "nullptr m_enocdeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CODEC_CHK_NULL(ptr, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncSegMapBufferAV1 *vaEncSegMapBuf = (VAEncSegMapBufferAV1 *)ptr;
    m_isSegParamsChanged = true;

    m_encodeCtx->segmentMapDataSize = vaEncSegMapBuf->segmentMapDataSize;
    m_encodeCtx->pSegmentMap        = vaEncSegMapBuf->pSegmentMap;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAV1::ParseMiscParamVBV(void *data)
{
    DDI_CODEC_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterHRD *vaEncMiscParamHRD = (VAEncMiscParameterHRD *)data;

    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS seqParams = (PCODEC_AV1_ENCODE_SEQUENCE_PARAMS)m_encodeCtx->pSeqParams;
    DDI_CODEC_CHK_NULL(seqParams, "nullptr seqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    seqParams->VBVBufferSizeInBit         = vaEncMiscParamHRD->buffer_size;
    seqParams->InitVBVBufferFullnessInBit = vaEncMiscParamHRD->initial_buffer_fullness;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAV1::ParseMiscParamFR(void *data)
{
    DDI_CODEC_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterFrameRate *vaFrameRate = (VAEncMiscParameterFrameRate *)data;

    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS seqParams = (PCODEC_AV1_ENCODE_SEQUENCE_PARAMS)m_encodeCtx->pSeqParams;
    DDI_CODEC_CHK_NULL(seqParams, "nullptr seqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (vaFrameRate->framerate_flags.bits.temporal_id > seqParams->NumTemporalLayersMinus1)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    uint32_t temporalId = vaFrameRate->framerate_flags.bits.temporal_id;

    if (vaFrameRate->framerate != savedFrameRate[temporalId])
    {
        savedFrameRate[temporalId] = vaFrameRate->framerate;
        //seqParams->SeqFlags.fields.ResetBRC |= 0x1;

        uint32_t frameRate = vaFrameRate->framerate;
        seqParams->FrameRate[temporalId].Numerator   = frameRate &(0xFFFF);
        seqParams->FrameRate[temporalId].Denominator = (frameRate >> 16) &(0xFFFF);

        if (seqParams->FrameRate[temporalId].Denominator == 0)
        {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
    }

    return VA_STATUS_SUCCESS;
}

// Parse rate control related information from app.
VAStatus DdiEncodeAV1::ParseMiscParamRC(void *data)
{
    DDI_CODEC_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterRateControl *vaEncMiscParamRC = (VAEncMiscParameterRateControl *)data;

    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS seqParams = (PCODEC_AV1_ENCODE_SEQUENCE_PARAMS)m_encodeCtx->pSeqParams;
    DDI_CODEC_CHK_NULL(seqParams, "nullptr seqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t temporalId = vaEncMiscParamRC->rc_flags.bits.temporal_id;
    DDI_CHK_LESS(temporalId, (seqParams->NumTemporalLayersMinus1 + 1),
        "invalid temporal id", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t bitRate                    = MOS_ROUNDUP_DIVIDE(vaEncMiscParamRC->bits_per_second, CODECHAL_ENCODE_BRC_KBPS);
    seqParams->MaxBitRate               = MOS_MAX(seqParams->MaxBitRate, bitRate);
    seqParams->SeqFlags.fields.ResetBRC = vaEncMiscParamRC->rc_flags.bits.reset;

    if (VA_RC_CBR == m_encodeCtx->uiRCMethod)
    {
        if(vaEncMiscParamRC->target_percentage != 0)
            seqParams->TargetBitRate[temporalId] = bitRate * vaEncMiscParamRC->target_percentage / 100;
        else
            seqParams->TargetBitRate[temporalId] = bitRate;
        seqParams->MaxBitRate                = seqParams->TargetBitRate[temporalId];
        seqParams->MinBitRate                = seqParams->TargetBitRate[temporalId];
        seqParams->RateControlMethod         = RATECONTROL_CBR;
        if (savedTargetBit[temporalId] != bitRate)
        {
            if (savedTargetBit[temporalId] != 0)
            {
                seqParams->SeqFlags.fields.ResetBRC |= 0x01;
            }
            savedTargetBit[temporalId] = bitRate;
        }
    }
    else if (VA_RC_VBR == m_encodeCtx->uiRCMethod)
    {
        if(vaEncMiscParamRC->target_percentage != 0)
            seqParams->TargetBitRate[temporalId] = bitRate * vaEncMiscParamRC->target_percentage / 100;
        else
            seqParams->TargetBitRate[temporalId] = bitRate;
        seqParams->MaxBitRate = bitRate;
        seqParams->MinBitRate = 0;
        seqParams->RateControlMethod = RATECONTROL_VBR;

        if ((savedTargetBit[temporalId] != seqParams->TargetBitRate[temporalId]) ||
            (savedMaxBitRate[temporalId] != bitRate))
        {
            if ((savedTargetBit[temporalId] != 0) && (savedMaxBitRate[temporalId] != 0))
            {
                seqParams->SeqFlags.fields.ResetBRC |= 0x01;
            }
            savedTargetBit[temporalId]  = seqParams->TargetBitRate[temporalId];
            savedMaxBitRate[temporalId] = bitRate;
        }
    }
    else if (VA_RC_ICQ == m_encodeCtx->uiRCMethod)
    {   
        seqParams->RateControlMethod = RATECONTROL_CQL;
        seqParams->ICQQualityFactor = vaEncMiscParamRC->quality_factor;
        if (savedQualityFactor != seqParams->ICQQualityFactor)
        {
            if (savedQualityFactor != 0)
            {
                seqParams->SeqFlags.fields.ResetBRC |= 0x01;
            }
            savedQualityFactor = seqParams->ICQQualityFactor;
        }
    }

    /* the reset flag in RC will be considered. */
    seqParams->SeqFlags.fields.ResetBRC |= vaEncMiscParamRC->rc_flags.bits.reset;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAV1::ParseMiscParamEncQuality(void *data)
{
    DDI_CODEC_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterEncQuality *vaEncMiscParamEncQuality = (VAEncMiscParameterEncQuality *)data;

    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS seqParams = (PCODEC_AV1_ENCODE_SEQUENCE_PARAMS)m_encodeCtx->pSeqParams;
    DDI_CODEC_CHK_NULL(seqParams, "nullptr seqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    seqParams->SeqFlags.fields.UseRawReconRef    = vaEncMiscParamEncQuality->useRawPicForRef;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAV1::ParseMiscParamTemporalLayerParams(void* data)
{
    DDI_CODEC_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterTemporalLayerStructure *vaEncTempLayerStruct = (VAEncMiscParameterTemporalLayerStructure *)data;
    DDI_CHK_LESS(vaEncTempLayerStruct->number_of_layers, (ENCODE_AV1_MAX_NUM_TEMPORAL_LAYERS+1),
        "invalid number of temporal layers", VA_STATUS_ERROR_INVALID_PARAMETER);

    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS seqParams = (PCODEC_AV1_ENCODE_SEQUENCE_PARAMS)m_encodeCtx->pSeqParams;
    DDI_CODEC_CHK_NULL(seqParams, "nullptr seqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

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

VAStatus DdiEncodeAV1::ParseMiscParamQualityLevel(void *data)
{
    DDI_CODEC_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CODEC_CHK_NULL(m_encodeCtx, "nullptr m_enocdeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterBufferQualityLevel *vaEncMiscParamQualityLevel = (VAEncMiscParameterBufferQualityLevel *)data;
    m_encodeCtx->targetUsage = (uint8_t)vaEncMiscParamQualityLevel->quality_level;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAV1::ParseMiscParamMaxFrameSize(void *data)
{
    DDI_CODEC_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterBufferMaxFrameSize *vaEncMiscParamMaxFrameSize = (VAEncMiscParameterBufferMaxFrameSize *)data;

    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS seqParams = (PCODEC_AV1_ENCODE_SEQUENCE_PARAMS)m_encodeCtx->pSeqParams;
    DDI_CODEC_CHK_NULL(seqParams, "nullptr seqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    seqParams->UserMaxIFrameSize = vaEncMiscParamMaxFrameSize->max_frame_size >> 3; // convert to bytes.
    seqParams->UserMaxPBFrameSize = vaEncMiscParamMaxFrameSize->max_frame_size >> 3;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAV1::CheckCDEF(const VAEncPictureParameterBufferAV1 *picParams, PRODUCT_FAMILY platform)
{
    DDI_CODEC_CHK_NULL(picParams, "nullptr picParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (picParams->cdef_damping_minus_3 > av1MaxCDEFDampingMinus3)
    {
        DDI_CODEC_ASSERTMESSAGE("The CDEF damping exceeds the max value.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (picParams->cdef_bits > av1MaxCDEFBits)
    {
        DDI_CODEC_ASSERTMESSAGE("The CDEF bits exceed the max value.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    for (uint32_t i = 0; i < av1MaxCDEFEntries; i++)
    {
        if (picParams->cdef_y_strengths[i] > av1MaxCDEFStrengths)
        {
            DDI_CODEC_ASSERTMESSAGE("The CDEF strengths exceed the max value.");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (picParams->cdef_uv_strengths[i] > av1MaxCDEFStrengths)
        {
            DDI_CODEC_ASSERTMESSAGE("The CDEF strengths exceed the max value.");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (picParams->cdef_uv_strengths[i] != picParams->cdef_y_strengths[i] && platform <= IGFX_DG2)
        {
            DDI_CODEC_ASSERTMESSAGE("Non-uniform CDEF strength of Y/UV are not supported for DG2.");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAV1::CheckTile(const VAEncPictureParameterBufferAV1 *picParams)
{
    int minTileHeightInSB = picParams->height_in_sbs_minus_1[0] + 1;
    int minTileWidthInSB = picParams->width_in_sbs_minus_1[0] + 1;

    for(int i = 1;i < picParams->tile_cols;i++)
    {
        minTileWidthInSB = MOS_MIN(minTileWidthInSB, picParams->width_in_sbs_minus_1[i] + 1);
    }
    for(int i = 1;i < picParams->tile_rows;i++)
    {
        minTileHeightInSB = MOS_MIN(minTileHeightInSB, picParams->height_in_sbs_minus_1[i] + 1);
    }

    if(minTileWidthInSB * minTileHeightInSB < 4 ||
        minTileWidthInSB < 2 ||
        minTileHeightInSB < 2)
    {
        DDI_CODEC_ASSERTMESSAGE("Unsupported Tile Size");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    return VA_STATUS_SUCCESS;
}

void DdiEncodeAV1::SetupCodecPicture(
    DDI_MEDIA_CONTEXT             *mediaCtx,
    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl,
    CODEC_PICTURE                 *codecHalPic,
    VASurfaceID                   surfaceID,
    bool                          picReference)
{
    if (VA_INVALID_SURFACE != surfaceID)
    {
        DDI_MEDIA_SURFACE *surface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, surfaceID);
        codecHalPic->FrameIdx      = GetRenderTargetID(rtTbl, surface);
        codecHalPic->PicEntry      = codecHalPic->FrameIdx;
    }
    else
    {
        codecHalPic->FrameIdx = (uint8_t)DDI_CODEC_INVALID_FRAME_INDEX;
        codecHalPic->PicEntry  = 0xFF;
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

uint32_t DdiEncodeAV1::getSequenceParameterBufferSize()
{
    return sizeof(VAEncSequenceParameterBufferAV1);
}

uint32_t DdiEncodeAV1::getPictureParameterBufferSize()
{
    return sizeof(VAEncPictureParameterBufferAV1);
}

uint32_t DdiEncodeAV1::getSliceParameterBufferSize()
{
    return sizeof(VAEncTileGroupBufferAV1);
}


CODECHAL_FUNCTION DdiEncodeAV1::GetEncodeCodecFunction(VAProfile profile, VAEntrypoint entrypoint, bool bVDEnc)
{
    CODECHAL_FUNCTION codecFunction = CODECHAL_FUNCTION_INVALID;
    if (bVDEnc)
    {
        codecFunction = CODECHAL_FUNCTION_ENC_VDENC_PAK;
    }
    else
    {
        DDI_CODEC_ASSERTMESSAGE("Unsuported CODECHAL_FUNCTION");
    }
    return codecFunction;
}

CODECHAL_MODE DdiEncodeAV1::GetEncodeCodecMode(
    VAProfile    profile,
    VAEntrypoint entrypoint)
{
    switch (profile)
    {
    case VAProfileAV1Profile0:
    case VAProfileAV1Profile1:
        return CODECHAL_ENCODE_MODE_AV1;
    default:
        DDI_CODEC_ASSERTMESSAGE("Unsuported CODECHAL_MODE");
        return CODECHAL_UNSUPPORTED_MODE;
    }
}

}  // namespace encode