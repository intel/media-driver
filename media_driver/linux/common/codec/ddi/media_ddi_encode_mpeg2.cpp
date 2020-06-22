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
//! \file     media_ddi_encode_mpeg2.cpp
//! \brief    Defines class for DDI media mpeg2 encode.
//!

#include "media_libva_encoder.h"
#include "media_libva_util.h"
#include "media_ddi_encode_mpeg2.h"
#include "media_ddi_encode_const.h"
#include "media_ddi_factory.h"

extern template class MediaDdiFactoryNoArg<DdiEncodeBase>;
static bool isEncodeMpeg2Registered =
    MediaDdiFactoryNoArg<DdiEncodeBase>::RegisterCodec<DdiEncodeMpeg2>(ENCODE_ID_MPEG2);

DdiEncodeMpeg2::~DdiEncodeMpeg2()
{
    RemoveUserData();

    MOS_FreeMemory(m_encodeCtx->pSeqParams);
    m_encodeCtx->pSeqParams = nullptr;

    MOS_FreeMemory(m_encodeCtx->pPicParams);
    m_encodeCtx->pPicParams = nullptr;

    if (nullptr != m_encodeCtx->ppNALUnitParams)
    {
        // Allocate one contiguous memory for the NALUnitParams buffers
        // only need to free one time
        MOS_FreeMemory(m_encodeCtx->ppNALUnitParams[0]);
        m_encodeCtx->ppNALUnitParams[0] = nullptr;

        MOS_FreeMemory(m_encodeCtx->ppNALUnitParams);
        m_encodeCtx->ppNALUnitParams = nullptr;
    }

    MOS_FreeMemory(m_encodeCtx->pVuiParams);
    m_encodeCtx->pVuiParams = nullptr;

    MOS_FreeMemory(m_encodeCtx->pSliceParams);
    m_encodeCtx->pSliceParams = nullptr;

    MOS_FreeMemory(m_encodeCtx->pEncodeStatusReport);
    m_encodeCtx->pEncodeStatusReport = nullptr;

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
    MOS_FreeMemory(m_encodeCtx->pQmatrixParams);
    m_encodeCtx->pQmatrixParams = nullptr;
}

VAStatus DdiEncodeMpeg2::ContextInitialize(
    CodechalSetting *codecHalSettings)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_encodeCtx->pCpDdiInterface, "nullptr m_encodeCtx->pCpDdiInterface.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(codecHalSettings, "nullptr codecHalSettings.", VA_STATUS_ERROR_INVALID_CONTEXT);

    codecHalSettings->codecFunction = CODECHAL_FUNCTION_ENC_PAK;
    codecHalSettings->width       = m_encodeCtx->dwFrameWidth;
    codecHalSettings->height      = m_encodeCtx->dwFrameHeight;
    codecHalSettings->mode          = m_encodeCtx->wModeType;
    codecHalSettings->standard      = CODECHAL_MPEG2;

    m_encodeCtx->pSeqParams = (void *)MOS_AllocAndZeroMemory(sizeof(CodecEncodeMpeg2SequenceParams));
    DDI_CHK_NULL(m_encodeCtx->pSeqParams, "nullptr m_encodeCtx->pSeqParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pPicParams = (void *)MOS_AllocAndZeroMemory(sizeof(CodecEncodeMpeg2PictureParams));
    DDI_CHK_NULL(m_encodeCtx->pPicParams, "nullptr m_encodeCtx->pPicParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pQmatrixParams = (void *)MOS_AllocAndZeroMemory(sizeof(CodecEncodeMpeg2QmatixParams));
    DDI_CHK_NULL(m_encodeCtx->pQmatrixParams, "nullptr m_encodeCtx->pQmatrixParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pVuiParams = (void *)MOS_AllocAndZeroMemory(sizeof(CodecEncodeMpeg2VuiParams));
    DDI_CHK_NULL(m_encodeCtx->pVuiParams, "nullptr m_encodeCtx->pVuiParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Allocate SliceParams
    // slice params num should equal to the mbHeight  //attention !!! zxf
    m_encodeCtx->pSliceParams = (void *)MOS_AllocAndZeroMemory((m_encodeCtx->dwFrameHeight >> 4) * sizeof(CodecEncodeMpeg2SliceParmas));
    DDI_CHK_NULL(m_encodeCtx->pSliceParams, "nullptr m_encodeCtx->pSliceParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Allocate Encode Status Report
    m_encodeCtx->pEncodeStatusReport = (void *)MOS_AllocAndZeroMemory(CODECHAL_ENCODE_STATUS_NUM * sizeof(EncodeStatusReport));
    DDI_CHK_NULL(m_encodeCtx->pEncodeStatusReport, "nullptr m_encodeCtx->pEncodeStatusReport.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Allocate SEI structure
    m_encodeCtx->pSEIFromApp = (CodechalEncodeSeiData *)MOS_AllocAndZeroMemory(sizeof(CodechalEncodeSeiData));
    DDI_CHK_NULL(m_encodeCtx->pSEIFromApp, "nullptr m_encodeCtx->pSEIFromApp.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pSliceHeaderData = (CODEC_ENCODER_SLCDATA *)MOS_AllocAndZeroMemory(m_encodeCtx->wPicHeightInMB *
                                                                                    sizeof(CODEC_ENCODER_SLCDATA));
    DDI_CHK_NULL(m_encodeCtx->pSliceHeaderData, "nullptr m_encodeCtx->pSliceHeaderData.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Create the bit stream buffer to hold the packed headers from application
    m_encodeCtx->pbsBuffer = (BSBuffer *)MOS_AllocAndZeroMemory(sizeof(BSBuffer));
    DDI_CHK_NULL(m_encodeCtx->pbsBuffer, "nullptr m_encodeCtx->pbsBuffer.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pbsBuffer->pBase = (uint8_t *)MOS_AllocAndZeroMemory(m_encodeCtx->wPicHeightInMB * PACKED_HEADER_SIZE_PER_ROW);
    DDI_CHK_NULL(m_encodeCtx->pbsBuffer->pBase, "nullptr m_encodeCtx->pbsBuffer->pBase.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pbsBuffer->BufferSize = m_encodeCtx->wPicHeightInMB * PACKED_HEADER_SIZE_PER_ROW;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeMpeg2::RenderPicture(
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

        case VAEncSliceParameterBufferType:
            {
                uint32_t numSlices = buf->uiNumElements;
                DDI_CHK_STATUS(ParseSlcParams(mediaCtx, data, numSlices), VA_STATUS_ERROR_INVALID_BUFFER);
            }
            break;

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

VAStatus DdiEncodeMpeg2::EncodeInCodecHal(
    uint32_t numSlices)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx->pMediaCtx, "nullptr m_encodeCtx->pMediaCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx->pCodecHal, "nullptr m_encodeCtx->pCodecHal", VA_STATUS_ERROR_INVALID_PARAMETER);

    EncoderParams encodeParams;
    MOS_ZeroMemory(&encodeParams, sizeof(EncoderParams));

    if (m_encodeCtx->bVdencActive == true)
    {
        encodeParams.ExecCodecFunction = CODECHAL_FUNCTION_ENC_VDENC_PAK;
    }
    else
    {
        encodeParams.ExecCodecFunction = CODECHAL_FUNCTION_ENC_PAK;
    }

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_encodeCtx->RTtbl);

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

    //clear registered recon/ref surface flags
    DDI_CHK_RET(ClearRefList(&m_encodeCtx->RTtbl, false), "ClearRefList failed!");

    // Bitstream surface
    MOS_RESOURCE bitstreamSurface;
    MOS_ZeroMemory(&bitstreamSurface, sizeof(MOS_RESOURCE));
    bitstreamSurface        = m_encodeCtx->resBitstreamBuffer;  // in render picture
    bitstreamSurface.Format = Format_Buffer;

    encodeParams.psRawSurface        = &rawSurface;
    encodeParams.psReconSurface      = &reconSurface;
    encodeParams.presBitstreamBuffer = &bitstreamSurface;

    MOS_SURFACE mbQpSurface;
    if (m_encodeCtx->bMBQpEnable)
    {
        MOS_ZeroMemory(&mbQpSurface, sizeof(MOS_SURFACE));
        mbQpSurface.Format             = Format_Buffer_2D;
        mbQpSurface.dwOffset           = 0;
        mbQpSurface.OsResource         = m_encodeCtx->resMBQpBuffer;
        encodeParams.psMbQpDataSurface = &mbQpSurface;
        encodeParams.bMbQpDataEnabled  = true;
    }

    encodeParams.bMbDisableSkipMapEnabled = m_encodeCtx->bMbDisableSkipMapEnabled;

    MOS_SURFACE disableSkipMapSurface;
    if (encodeParams.bMbDisableSkipMapEnabled)
    {
        MOS_ZeroMemory(&disableSkipMapSurface, sizeof(MOS_SURFACE));
        disableSkipMapSurface.Format           = Format_Buffer;
        disableSkipMapSurface.dwOffset         = 0;
        disableSkipMapSurface.OsResource       = m_encodeCtx->resPerMBSkipMapBuffer;
        encodeParams.psMbDisableSkipMapSurface = &disableSkipMapSurface;
    }

    encodeParams.pMpeg2UserDataListHead = m_userDataListHead;

    CodecEncodeMpeg2SequenceParams *mpeg2Sps = (CodecEncodeMpeg2SequenceParams *)m_encodeCtx->pSeqParams;
    mpeg2Sps->m_targetUsage               = m_encodeCtx->targetUsage;

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

    CODEC_AVC_IQ_MATRIX_PARAMS iqMatrixParams;
    MOS_STATUS                    status = MOS_SecureMemcpy(&iqMatrixParams.ScalingList4x4,
        6 * 16 * sizeof(uint8_t),
        &m_scalingLists4x4,
        6 * 16 * sizeof(uint8_t));
    if (MOS_STATUS_SUCCESS != status)
    {
        DDI_ASSERTMESSAGE("DDI:Failed to copy scaling list 4x4!");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    status = MOS_SecureMemcpy(&iqMatrixParams.ScalingList8x8,
        2 * 64 * sizeof(uint8_t),
        &m_scalingLists8x8,
        2 * 64 * sizeof(uint8_t));
    if (MOS_STATUS_SUCCESS != status)
    {
        DDI_ASSERTMESSAGE("DDI:Failed to copy scaling list 8x8!");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    encodeParams.pIQMatrixBuffer = &iqMatrixParams;

    // whether driver need to pack slice header
    if (true == m_encodeCtx->bHavePackedSliceHdr)
    {
        encodeParams.bAcceleratorHeaderPackingCaps = false;
    }
    else
    {
        encodeParams.bAcceleratorHeaderPackingCaps = true;
    }

    encodeParams.pBSBuffer      = m_encodeCtx->pbsBuffer;
    encodeParams.pSlcHeaderData = (void *)m_encodeCtx->pSliceHeaderData;

    status = m_encodeCtx->pCodecHal->Execute(&encodeParams);
    if (MOS_STATUS_SUCCESS != status)
    {
        DDI_ASSERTMESSAGE("DDI:Failed in Codechal!");
        return VA_STATUS_ERROR_ENCODING_ERROR;
    }

    return VA_STATUS_SUCCESS;
}

// parse quant matrix extension from app
VAStatus DdiEncodeMpeg2::Qmatrix(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAIQMatrixBufferMPEG2 *qMatrix = (VAQMatrixBufferMPEG2 *)ptr;

    CodecEncodeMpeg2QmatixParams *mpeg2QParams = (CodecEncodeMpeg2QmatixParams *)m_encodeCtx->pQmatrixParams;
    DDI_CHK_NULL(mpeg2QParams, "nullptr mpeg2QParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    mpeg2QParams->m_newQmatrix[0] = qMatrix->load_intra_quantiser_matrix;
    mpeg2QParams->m_newQmatrix[1] = qMatrix->load_non_intra_quantiser_matrix;
    mpeg2QParams->m_newQmatrix[2] = qMatrix->load_chroma_intra_quantiser_matrix;
    mpeg2QParams->m_newQmatrix[3] = qMatrix->load_chroma_non_intra_quantiser_matrix;

    if (mpeg2QParams->m_newQmatrix[0])
    {
        for (uint8_t idx = 0; idx < 64; idx++)
        {
            mpeg2QParams->m_qmatrix[0][CODEC_AVC_Qmatrix_scan_8x8[idx]] = qMatrix->intra_quantiser_matrix[idx];
        }
    }
    if (mpeg2QParams->m_newQmatrix[1])
    {
        for (uint8_t idx = 0; idx < 64; idx++)
        {
            mpeg2QParams->m_qmatrix[1][CODEC_AVC_Qmatrix_scan_8x8[idx]] = qMatrix->non_intra_quantiser_matrix[idx];
        }
    }
    if (mpeg2QParams->m_newQmatrix[2])
    {
        for (uint8_t idx = 0; idx < 64; idx++)
        {
            mpeg2QParams->m_qmatrix[2][CODEC_AVC_Qmatrix_scan_8x8[idx]] = qMatrix->chroma_intra_quantiser_matrix[idx];
        }
    }
    if (mpeg2QParams->m_newQmatrix[3])
    {
        for (uint8_t idx = 0; idx < 64; idx++)
        {
            mpeg2QParams->m_qmatrix[3][CODEC_AVC_Qmatrix_scan_8x8[idx]] = qMatrix->chroma_non_intra_quantiser_matrix[idx];
        }
    }

    return VA_STATUS_SUCCESS;
}

// reset the parameters before each frame
VAStatus DdiEncodeMpeg2::ResetAtFrameLevel()
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);

    // Assume there is only one SPS parameter
    CodecEncodeMpeg2SequenceParams *mpeg2SeqParams = (CodecEncodeMpeg2SequenceParams *)(m_encodeCtx->pSeqParams);
    DDI_CHK_NULL(mpeg2SeqParams, "nullptr mpeg2SeqParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    mpeg2SeqParams->m_resetBRC = false;

    m_encodeCtx->dwNumSlices  = 0x0;
    m_encodeCtx->indexNALUnit = 0x0;

    // reset bsbuffer every frame
    m_encodeCtx->pbsBuffer->pCurrent    = m_encodeCtx->pbsBuffer->pBase;
    m_encodeCtx->pbsBuffer->SliceOffset = 0x0;
    m_encodeCtx->pbsBuffer->BitOffset   = 0x0;
    m_encodeCtx->pbsBuffer->BitSize     = 0x0;
    m_encodeCtx->bNewVuiData            = false;
    m_encodeCtx->bMBQpEnable            = false;

    RemoveUserData();

    // clear the packed header information
    if (nullptr != m_encodeCtx->ppNALUnitParams)
    {
        MOS_ZeroMemory(m_encodeCtx->ppNALUnitParams[0], sizeof(CODECHAL_NAL_UNIT_PARAMS) * CODEC_ENCODE_MPEG2_MAX_NAL_TYPE);
    }

    return VA_STATUS_SUCCESS;
}

// parse mepg2 sequence parameters from app
VAStatus DdiEncodeMpeg2::ParseSeqParams(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncSequenceParameterBufferMPEG2 *seqParams = (VAEncSequenceParameterBufferMPEG2 *)ptr;

    CodecEncodeMpeg2SequenceParams *mpeg2Sps = (CodecEncodeMpeg2SequenceParams *)m_encodeCtx->pSeqParams;

    CodecEncodeMpeg2PictureParams *halMpeg2PPS = (CodecEncodeMpeg2PictureParams *)m_encodeCtx->pPicParams;
    DDI_CHK_NULL(mpeg2Sps, "nullptr mpeg2Sps", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(halMpeg2PPS, "nullptr halMpeg2PPS", VA_STATUS_ERROR_INVALID_PARAMETER);

    MOS_ZeroMemory(mpeg2Sps, sizeof(CodecEncodeMpeg2SequenceParams));

    mpeg2Sps->m_frameWidth                = seqParams->picture_width;
    mpeg2Sps->m_frameHeight               = seqParams->picture_height;
    mpeg2Sps->m_profile                   = seqParams->sequence_extension.bits.profile_and_level_indication & 0x70;
    mpeg2Sps->m_level                     = seqParams->sequence_extension.bits.profile_and_level_indication & 0x0f;
    mpeg2Sps->m_chromaFormat              = seqParams->sequence_extension.bits.chroma_format;
    mpeg2Sps->m_lowDelay                  = seqParams->sequence_extension.bits.low_delay;
    mpeg2Sps->m_noAcceleratorSPSInsertion = false;
    mpeg2Sps->m_progressiveSequence       = seqParams->sequence_extension.bits.progressive_sequence;
    mpeg2Sps->m_rateControlMethod         = VARC2HalRC(m_encodeCtx->uiRCMethod);
    mpeg2Sps->m_resetBRC                  = false;  // if reset the brc each i frame
    mpeg2Sps->m_bitrate                   = seqParams->bits_per_second;
    mpeg2Sps->m_vbvBufferSize             = seqParams->vbv_buffer_size;
    //set Initial vbv buffer fullness to half of vbv buffer size in byte
    mpeg2Sps->m_initVBVBufferFullnessInBit = seqParams->vbv_buffer_size * (CODEC_ENCODE_MPEG2_VBV_BUFFER_SIZE_UNITS >> 1);
    mpeg2Sps->m_maxBitRate                 = mpeg2Sps->m_bitrate;
    mpeg2Sps->m_minBitRate                 = mpeg2Sps->m_bitrate;

    if ((m_encodeCtx->uiRCMethod == VA_RC_CBR) && (seqParams->vbv_buffer_size > 0))
    {
        //set maxframe size to half of vbv buffer size in byte, in codechal, this value will set the minimum value between this value and picture_with * picture_height
        mpeg2Sps->m_userMaxFrameSize = seqParams->vbv_buffer_size * (CODEC_ENCODE_MPEG2_VBV_BUFFER_SIZE_UNITS >> 4);
    }
    else
    {
        mpeg2Sps->m_userMaxFrameSize = seqParams->picture_width * seqParams->picture_height * 3 / 2;
    }

    mpeg2Sps->m_aspectRatio   = seqParams->aspect_ratio_information;
    mpeg2Sps->m_frameRateExtD = seqParams->sequence_extension.bits.frame_rate_extension_d;
    mpeg2Sps->m_frameRateExtN = seqParams->sequence_extension.bits.frame_rate_extension_n;
    if(seqParams->frame_rate <= 0.0)
    {
        DDI_NORMALMESSAGE("invalidate frame rate code, set it to default 30");
        seqParams->frame_rate = 30;
    }
    mpeg2Sps->m_frameRateCode = (uint32_t)CalculateFramerateCode(seqParams->frame_rate, mpeg2Sps->m_frameRateExtD, mpeg2Sps->m_frameRateExtN);

    halMpeg2PPS->m_gopPicSize = seqParams->intra_period;
    halMpeg2PPS->m_gopRefDist = seqParams->ip_period;
    m_timeCode                = seqParams->gop_header.bits.time_code;
    m_newTimeCode             = true;
    halMpeg2PPS->m_gopOptFlag = seqParams->gop_header.bits.closed_gop;

    return VA_STATUS_SUCCESS;
}

// parse mepg2 picture prarameters from app
VAStatus DdiEncodeMpeg2::ParsePicParams(
    DDI_MEDIA_CONTEXT *mediaCtx,
    void              *ptr)
{
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_encodeCtx->RTtbl);

    VAEncPictureParameterBufferMPEG2 *picParams = (VAEncPictureParameterBufferMPEG2 *)ptr;

    CodecEncodeMpeg2PictureParams *mpeg2PicParams = (CodecEncodeMpeg2PictureParams *)(m_encodeCtx->pPicParams);
    DDI_CHK_NULL(mpeg2PicParams, "nullptr mpeg2PicParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    CodecEncodeMpeg2SequenceParams *mpeg2SeqParams = (CodecEncodeMpeg2SequenceParams *)(m_encodeCtx->pSeqParams);
    DDI_CHK_NULL(mpeg2SeqParams, "nullptr mpeg2SeqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    float frameRate = 0;
    if ((mpeg2SeqParams->m_frameRateCode > 0) && (mpeg2SeqParams->m_frameRateCode < (sizeof(frameRateTable) / sizeof(frameRateTable[0]))))
    {
        //calculate real frame rate from frame_rate_code, frame_rate_extension_n and frame_rate_extension_d
        frameRate = frameRateTable[mpeg2SeqParams->m_frameRateCode - 1].value * (mpeg2SeqParams->m_frameRateExtN + 1) / (mpeg2SeqParams->m_frameRateExtD + 1);
    }
    else
    {
        //frame_rate_code is invalid, set frame_rate to 30 defaultly.
        frameRate = 30.0;
    }

    uint32_t frameRateRounded = (uint32_t)(frameRate * 100 + 50) / 100;

    mpeg2PicParams->m_lastPicInStream          = picParams->last_picture;
    mpeg2PicParams->m_pictureCodingType        = picParams->picture_type == VAEncPictureTypeIntra ? I_TYPE : (picParams->picture_type == VAEncPictureTypePredictive ? P_TYPE : B_TYPE);
    mpeg2PicParams->m_interleavedFieldBFF      = !picParams->picture_coding_extension.bits.top_field_first;
    mpeg2PicParams->m_fieldCodingFlag          = false;
    mpeg2PicParams->m_fieldFrameCodingFlag     = (!mpeg2SeqParams->m_progressiveSequence) && (!picParams->picture_coding_extension.bits.progressive_frame);
    mpeg2PicParams->m_pic4MVallowed            = 1;
    mpeg2PicParams->m_fcode00                  = picParams->f_code[0][0];
    mpeg2PicParams->m_fcode01                  = picParams->f_code[0][1];
    mpeg2PicParams->m_fcode10                  = picParams->f_code[1][0];
    mpeg2PicParams->m_fcode11                  = picParams->f_code[1][1];
    mpeg2PicParams->m_intraDCprecision         = picParams->picture_coding_extension.bits.intra_dc_precision;
    mpeg2PicParams->m_concealmentMotionVectors = picParams->picture_coding_extension.bits.concealment_motion_vectors;
    mpeg2PicParams->m_qscaleType               = picParams->picture_coding_extension.bits.q_scale_type;  //determin quantiser_scale with scale code

    mpeg2PicParams->m_intraVlcFormat    = picParams->picture_coding_extension.bits.intra_vlc_format;
    mpeg2PicParams->m_alternateScan     = picParams->picture_coding_extension.bits.alternate_scan;
    mpeg2PicParams->m_framePredFrameDCT = picParams->picture_coding_extension.bits.frame_pred_frame_dct;

    mpeg2PicParams->m_progressiveField     = picParams->picture_coding_extension.bits.progressive_frame;
    mpeg2PicParams->m_repeatFirstField     = picParams->picture_coding_extension.bits.repeat_first_field;
    mpeg2PicParams->m_compositeDisplayFlag = picParams->picture_coding_extension.bits.composite_display_flag;
    mpeg2PicParams->m_temporalReference    = picParams->temporal_reference;
    mpeg2PicParams->m_vbvDelay             = picParams->vbv_delay;

    if (mpeg2PicParams->m_compositeDisplayFlag == 1)
    {
        mpeg2PicParams->m_vaxis           = picParams->composite_display.bits.v_axis;
        mpeg2PicParams->m_fieldSequence   = picParams->composite_display.bits.field_sequence;
        mpeg2PicParams->m_subCarrier      = picParams->composite_display.bits.sub_carrier;
        mpeg2PicParams->m_burstAmplitude  = picParams->composite_display.bits.burst_amplitude;
        mpeg2PicParams->m_subCarrierPhase = picParams->composite_display.bits.sub_carrier_phase;
    }
    if ( picParams->reconstructed_picture == VA_INVALID_SURFACE)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    auto recon = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParams->reconstructed_picture);
    DDI_CHK_RET(RegisterRTSurfaces(&m_encodeCtx->RTtbl, recon),"RegisterRTSurfaces failed!");

    mpeg2PicParams->m_currReconstructedPic.FrameIdx = GetRenderTargetID(rtTbl, recon);
    mpeg2PicParams->m_currReconstructedPic.PicFlags = PICTURE_FRAME;

    // be attention , that codec hal use this value to manager the reference list
    mpeg2PicParams->m_currOriginalPic.FrameIdx = GetRenderTargetID(rtTbl, recon);
    mpeg2PicParams->m_currOriginalPic.PicFlags = mpeg2PicParams->m_currReconstructedPic.PicFlags;

    if (DDI_CODEC_INVALID_FRAME_INDEX != picParams->forward_reference_picture)
    {
        auto fwRef = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParams->forward_reference_picture);
        UpdateRegisteredRTSurfaceFlag(&m_encodeCtx->RTtbl,fwRef);
        mpeg2PicParams->m_refFrameList[0].FrameIdx = GetRenderTargetID(rtTbl, fwRef);
        mpeg2PicParams->m_refFrameList[0].PicFlags = PICTURE_FRAME;
    }
    else
    {
        mpeg2PicParams->m_refFrameList[0].FrameIdx = (uint8_t)DDI_CODEC_INVALID_FRAME_INDEX;
        mpeg2PicParams->m_refFrameList[0].PicFlags = PICTURE_INVALID;
    }
    if (DDI_CODEC_INVALID_FRAME_INDEX != picParams->backward_reference_picture)
    {
        auto bwRef = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParams->backward_reference_picture);
        UpdateRegisteredRTSurfaceFlag(&m_encodeCtx->RTtbl,bwRef);
        mpeg2PicParams->m_refFrameList[1].FrameIdx = GetRenderTargetID(rtTbl, bwRef);
        mpeg2PicParams->m_refFrameList[1].PicFlags = PICTURE_FRAME;
    }
    else
    {
        mpeg2PicParams->m_refFrameList[1].FrameIdx = (uint8_t)DDI_CODEC_INVALID_FRAME_INDEX;
        mpeg2PicParams->m_refFrameList[1].PicFlags = PICTURE_INVALID;
    }
    if (mpeg2PicParams->m_pictureCodingType == I_TYPE)
    {
        mpeg2PicParams->m_newGop = true;
    }
    else
    {
        mpeg2PicParams->m_newGop = false;
    }
    rtTbl->pCurrentReconTarget = recon;;

    DDI_MEDIA_BUFFER *buf = DdiMedia_GetBufferFromVABufferID(mediaCtx, picParams->coded_buf);

    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);
    RemoveFromStatusReportQueue(buf);
    DdiMedia_MediaBufferToMosResource(buf, &(m_encodeCtx->resBitstreamBuffer));
    mpeg2PicParams->m_numSlice = 0;

    //According to MPEG2 spec, GOP header time_code include 6 fields and the ranges of values are Hour (0~23), Minute(0~59), Marker bit(always 1), Second (0~59), Picuture (0~59), Drop_frame flag( 0 or 1).
    //split timecode to element
    uint32_t timeCode = m_timeCode;

    uint32_t timeCodeDropframe = (timeCode >> 24) & 0x1;

    uint32_t timeCodeHr = (timeCode >> 19) & 0x1f;

    uint32_t timeCodeMin = (timeCode >> 13) & 0x3f;

    uint32_t timeCodeMarkerBit = 0x1;

    uint32_t timeCodeSec = (timeCode >> 6) & 0x3f;

    uint32_t timeCodePic = (timeCode)&0x3f;
    //update each timecode element when parsing new picparams
    if (false == m_newTimeCode)
    {
        timeCodePic++;
        if (timeCodePic >= frameRateRounded)
        {
            timeCodePic = 0;
            timeCodeSec++;
        }
        if (timeCodeSec > maxTimeCodeSec)
        {
            timeCodeSec = 0;
            timeCodeMin++;
        }
        if (timeCodeMin > maxTimeCodeMin)
        {
            timeCodeMin = 0;
            timeCodeHr++;
        }
        if (timeCodeHr > maxTimeCodeHr)
        {
            timeCodeHr = 0;
        }
    }
    else
    {
        m_newTimeCode = false;
    }

    //fill all updated elements back to timecode in the format used by codechal
    mpeg2PicParams->m_timeCode = ((timeCodePic & 0x3f) |
                                  ((timeCodeSec & 0x3f) << 6) |
                                  ((timeCodeMarkerBit & 1) << 12) |
                                  ((timeCodeMin & 0x3f) << 13) |
                                  ((timeCodeHr & 0x1f) << 19) |
                                  (timeCodeDropframe & 1) << 24);

    m_timeCode                       = mpeg2PicParams->m_timeCode;
    mpeg2PicParams->m_skipFrameFlag  = 0;
    mpeg2PicParams->m_numSkipFrames  = 0;
    mpeg2PicParams->m_sizeSkipFrames = 0;

    return VA_STATUS_SUCCESS;
}

// parse mpeg2 slice parameters from app
VAStatus DdiEncodeMpeg2::ParseSlcParams(
    DDI_MEDIA_CONTEXT *mediaCtx,
    void              *ptr,
    uint32_t          numSlices)
{
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncSliceParameterBufferMPEG2 *vaEncSlcParamsMPEG2 = (VAEncSliceParameterBufferMPEG2 *)ptr;

    CodecEncodeMpeg2SliceParmas *mpeg2SlcParams = (CodecEncodeMpeg2SliceParmas *)m_encodeCtx->pSliceParams;

    CodecEncodeMpeg2PictureParams *mpeg2PicParams = (CodecEncodeMpeg2PictureParams *)(m_encodeCtx->pPicParams);
    DDI_CHK_NULL(mpeg2SlcParams, "nullptr mpeg2SlcParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(mpeg2PicParams, "nullptr mpeg2PicParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    // Move to a free MPEG2Slice parameter buffer
    uint8_t numOfSlices = m_encodeCtx->dwNumSlices;
    mpeg2SlcParams += numOfSlices;

    //attention memory area protection
    MOS_ZeroMemory(mpeg2SlcParams, sizeof(CodecEncodeMpeg2SliceParmas) * numSlices);

    CodecEncodeMpeg2SequenceParams *mpeg2Sps = (CodecEncodeMpeg2SequenceParams *)m_encodeCtx->pSeqParams;
    auto picWidthInMb = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(mpeg2Sps->m_frameWidth);

    for (uint32_t slcCount = 0; slcCount < numSlices; slcCount++)
    {
        mpeg2SlcParams->m_numMbsForSlice     = vaEncSlcParamsMPEG2->num_macroblocks;
        mpeg2SlcParams->m_firstMbX           = (vaEncSlcParamsMPEG2->macroblock_address % picWidthInMb);
        mpeg2SlcParams->m_firstMbY           = vaEncSlcParamsMPEG2->macroblock_address / picWidthInMb;
        mpeg2SlcParams->m_intraSlice         = vaEncSlcParamsMPEG2->is_intra_slice;
        mpeg2SlcParams->m_quantiserScaleCode = vaEncSlcParamsMPEG2->quantiser_scale_code;
        mpeg2SlcParams++;
        vaEncSlcParamsMPEG2++;
    }
    m_encodeCtx->dwNumSlices += numSlices;
    mpeg2PicParams->m_numSlice = m_encodeCtx->dwNumSlices;

    return VA_STATUS_SUCCESS;
}

// Since sequence header and picture header will be packed in codechal, we don't support packed header from application
VAStatus DdiEncodeMpeg2::ParsePackedHeaderParams(void *ptr)
{
    DDI_CHK_NULL(ptr, "invalid paked header pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx, "invalid encode context", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAEncPackedHeaderParameterBuffer *packedHeaderParamBuf = (VAEncPackedHeaderParameterBuffer *)ptr;
    if (packedHeaderParamBuf->type != VAEncPackedHeaderRawData)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    CodecEncodeMpeg2UserDataList *userDataNode = (CodecEncodeMpeg2UserDataList *)MOS_AllocAndZeroMemory(sizeof(CodecEncodeMpeg2UserDataList));
    DDI_CHK_NULL(userDataNode, "nullptr userDataNode.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    if (nullptr == m_userDataListHead)
    {
        m_userDataListHead = userDataNode;
        m_userDataListTail = nullptr;
    }
    if (m_userDataListTail)
    {
        CodecEncodeMpeg2UserDataList *userDataListTail = (CodecEncodeMpeg2UserDataList *)m_userDataListTail;
        userDataListTail->m_nextItem                   = userDataNode;
    }
    m_userDataListTail = userDataNode;

    uint32_t size            = (packedHeaderParamBuf->bit_length + 7) >> 3;
    userDataNode->m_userData = MOS_AllocAndZeroMemory(size);
    if (nullptr == userDataNode->m_userData)
    {
        MOS_FreeMemory(userDataNode);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    userDataNode->m_userDataSize = size;

    return VA_STATUS_SUCCESS;
}
// Since sequence header and picture header will be packed in codechal, we don't support packed header from application
VAStatus DdiEncodeMpeg2::ParsePackedHeaderData(void *ptr)
{
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    CodecEncodeMpeg2UserDataList *userDataListTail = (CodecEncodeMpeg2UserDataList *)m_userDataListTail;
    if (userDataListTail && userDataListTail->m_userDataSize)
    {
        MOS_SecureMemcpy(
            userDataListTail->m_userData,
            userDataListTail->m_userDataSize,
            ptr,
            userDataListTail->m_userDataSize);
        return VA_STATUS_SUCCESS;
    }
    return VA_STATUS_ERROR_INVALID_PARAMETER;
}

// it should be named DdiMpeg2Encode_ParseMiscParamVBV, because HRD is used by AVC, VBV is used by MPEG2
void DdiEncodeMpeg2::ParseMiscParamVBV(void *data)
{
    VAEncMiscParameterHRD *vaEncMiscParamHRD = (VAEncMiscParameterHRD *)data;

    CodecEncodeMpeg2SequenceParams *mpeg2SeqParams = (CodecEncodeMpeg2SequenceParams *)m_encodeCtx->pSeqParams;

    mpeg2SeqParams->m_vbvBufferSize              = vaEncMiscParamHRD->buffer_size / CODEC_ENCODE_MPEG2_VBV_BUFFER_SIZE_UNITS;
    mpeg2SeqParams->m_initVBVBufferFullnessInBit = vaEncMiscParamHRD->initial_buffer_fullness;
    //mpeg2SeqParams->m_rateControlMethod          = RATECONTROL_CBR;
}

// Parse the frame rate paramters from app
void DdiEncodeMpeg2::ParseMiscParamFR(void *data)
{
    CodecEncodeMpeg2SequenceParams *mpeg2SeqParams = (CodecEncodeMpeg2SequenceParams *)(m_encodeCtx->pSeqParams);

    VAEncMiscParameterFrameRate *vaencMiscParamFR = (VAEncMiscParameterFrameRate *)data;

    float frameRate                 = (float)(vaencMiscParamFR->framerate & 0xffff);
    uint32_t denominator            = (vaencMiscParamFR->framerate >> 16) & 0xffff;
    if(denominator == 0)
    {
        denominator = 1;
    }
    frameRate                       = frameRate / denominator;
    if(frameRate <= 0.0)
    {
        DDI_NORMALMESSAGE("invalidate frame rate code, set it to default 30");
        frameRate = 30;
    }
    mpeg2SeqParams->m_frameRateCode = (uint32_t)CalculateFramerateCode(frameRate,
        mpeg2SeqParams->m_frameRateExtD,
        mpeg2SeqParams->m_frameRateExtN);
}

// Parse rate control related information from app
void DdiEncodeMpeg2::ParseMiscParamRC(void *data)
{
    VAEncMiscParameterRateControl *vaEncMiscParamRC = (VAEncMiscParameterRateControl *)data;

    CodecEncodeMpeg2SequenceParams *mpeg2SeqParams = (CodecEncodeMpeg2SequenceParams *)(m_encodeCtx->pSeqParams);

    mpeg2SeqParams->m_bitrate = MOS_ROUNDUP_DIVIDE(vaEncMiscParamRC->bits_per_second, CODECHAL_ENCODE_BRC_KBPS);

    if (VA_RC_CQP == m_encodeCtx->uiRCMethod)
    {
        mpeg2SeqParams->m_rateControlMethod = (RATECONTROL_CBR | RATECONTROL_VBR);
    }

    if (VA_RC_CBR == m_encodeCtx->uiRCMethod)
    {
        mpeg2SeqParams->m_maxBitRate        = mpeg2SeqParams->m_bitrate;
        mpeg2SeqParams->m_minBitRate        = mpeg2SeqParams->m_bitrate;
        mpeg2SeqParams->m_rateControlMethod = RATECONTROL_CBR;
    }
    else
    {
        mpeg2SeqParams->m_maxBitRate = mpeg2SeqParams->m_bitrate;
        if (vaEncMiscParamRC->target_percentage > 50)
        {
            mpeg2SeqParams->m_minBitRate = mpeg2SeqParams->m_bitrate * (2 * vaEncMiscParamRC->target_percentage - 100) / 100;
        }
        else
        {
            mpeg2SeqParams->m_minBitRate = 0;
        }

        mpeg2SeqParams->m_bitrate           = mpeg2SeqParams->m_bitrate * vaEncMiscParamRC->target_percentage / 100;
        mpeg2SeqParams->m_rateControlMethod = RATECONTROL_VBR;

        if ((m_encodeCtx->uiTargetBitRate != mpeg2SeqParams->m_bitrate) ||
            (m_encodeCtx->uiMaxBitRate != mpeg2SeqParams->m_maxBitRate))
        {
            mpeg2SeqParams->m_resetBRC   = 0x1;
            m_encodeCtx->uiTargetBitRate = mpeg2SeqParams->m_bitrate;
            m_encodeCtx->uiMaxBitRate    = mpeg2SeqParams->m_maxBitRate;
        }
    }
}

// Parse some other parameters, now there are no Private data from APP
void DdiEncodeMpeg2::ParseMiscParamEncQuality(void *data)
{
    VAEncMiscParameterEncQuality *vaEncMiscParamEncQuality = (VAEncMiscParameterEncQuality *)data;

    CodecEncodeMpeg2SequenceParams *mpeg2SeqParams = (CodecEncodeMpeg2SequenceParams *)(m_encodeCtx->pSeqParams);

    mpeg2SeqParams->m_forcePanicModeControl = 1;
    mpeg2SeqParams->m_panicModeDisable = (uint8_t)vaEncMiscParamEncQuality->PanicModeDisable;

}

void DdiEncodeMpeg2::ParseMiscParamEncQualityLevel(void *data)
{
    VAEncMiscParameterBufferQualityLevel *vaMiscParamQualityLevel = (VAEncMiscParameterBufferQualityLevel*) data;
    m_encodeCtx->targetUsage= vaMiscParamQualityLevel->quality_level;
    // check TU, correct to default if wrong
    if ((m_encodeCtx->targetUsage > TARGETUSAGE_BEST_SPEED) ||
        (0 == m_encodeCtx->targetUsage))
    {
        m_encodeCtx->targetUsage = TARGETUSAGE_RT_SPEED;
        DDI_ASSERTMESSAGE("Target usage from application is not correct, should be in [0,7]!\n");
    }
}

void DdiEncodeMpeg2::ParseMiscParamMaxFrame(void *data)
{
    if (nullptr == m_encodeCtx || nullptr == data)
    {
        DDI_ASSERTMESSAGE("invalidate input parameters.");
        return;
    }

    CodecEncodeMpeg2SequenceParams *mpeg2SeqParams = (CodecEncodeMpeg2SequenceParams *)(m_encodeCtx->pSeqParams);

    VAEncMiscParameterBufferMaxFrameSize *maxFrameSize = (VAEncMiscParameterBufferMaxFrameSize *)data;

    mpeg2SeqParams->m_userMaxFrameSize = maxFrameSize->max_frame_size;
}

void DdiEncodeMpeg2::ParseMiscParamTypeExtension(void *data)
{
    if (nullptr == m_encodeCtx || nullptr == data)
    {
        DDI_ASSERTMESSAGE("invalidate input parameters.");
        return;
    }

    CodecEncodeMpeg2VuiParams *mpeg2VuiParams = (CodecEncodeMpeg2VuiParams *)(m_encodeCtx->pVuiParams);

    VAEncMiscParameterExtensionDataSeqDisplayMPEG2 *displayExt = (VAEncMiscParameterExtensionDataSeqDisplayMPEG2 *)data;

    if (displayExt->extension_start_code_identifier != Mpeg2sequenceDisplayExtension)
    {
        return;
    }

    m_encodeCtx->bNewVuiData                  = true;
    mpeg2VuiParams->m_videoFormat             = displayExt->video_format;
    mpeg2VuiParams->m_colourDescription       = displayExt->colour_description;
    mpeg2VuiParams->m_colourPrimaries         = displayExt->colour_primaries;
    mpeg2VuiParams->m_transferCharacteristics = displayExt->transfer_characteristics;
    mpeg2VuiParams->m_matrixCoefficients      = displayExt->matrix_coefficients;
    mpeg2VuiParams->m_displayHorizontalSize   = displayExt->display_horizontal_size;
    mpeg2VuiParams->m_displayVerticalSize     = displayExt->display_vertical_size;
}

// parse VAEncMiscParameterSkipFrame to PCODEC_MPEG2_ENCODE_PIC_PARAMS
void DdiEncodeMpeg2::ParseMiscParamSkipFrame(void *data)
{
    if (nullptr == m_encodeCtx || nullptr == data)
    {
        DDI_ASSERTMESSAGE("invalidate input parameters.");
        return;
    }

    // Assume only one PPS here, modify when enable multiple PPS support
    CodecEncodeMpeg2PictureParams *mpeg2PicParams = (CodecEncodeMpeg2PictureParams *)(m_encodeCtx->pPicParams);

    VAEncMiscParameterSkipFrame *vaEncMiscParamSkipFrame = (VAEncMiscParameterSkipFrame *)data;

    if (nullptr == mpeg2PicParams)
    {
        DDI_ASSERTMESSAGE("invalidate mpeg2 picture header.");
        return;
    }
    //MPEG2 only support normal skip frame
    if (FRAME_SKIP_NORMAL < mpeg2PicParams->m_skipFrameFlag)
    {
        DDI_ASSERTMESSAGE("invalidate input parameters, skip_frame_flag should equal to 0 or 1.");
        return;
    }

    // populate skipped frame params from DDI
    mpeg2PicParams->m_skipFrameFlag  = vaEncMiscParamSkipFrame->skip_frame_flag;
    mpeg2PicParams->m_numSkipFrames  = vaEncMiscParamSkipFrame->num_skip_frames;
    mpeg2PicParams->m_sizeSkipFrames = vaEncMiscParamSkipFrame->size_skip_frames;
}

// Parse Misc Params
VAStatus DdiEncodeMpeg2::ParseMiscParams(void *ptr)
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
    case VAEncMiscParameterTypeMaxFrameSize:
    {
        ParseMiscParamMaxFrame((void *)miscParamBuf->data);
        break;
    }
    case VAEncMiscParameterTypeExtensionData:
    {
        ParseMiscParamTypeExtension((void *)miscParamBuf->data);
        break;
    }
    case VAEncMiscParameterTypeSkipFrame:
    {
        ParseMiscParamSkipFrame((void *)miscParamBuf->data);
        break;
    }
    case VAEncMiscParameterTypeQualityLevel:
    {
        ParseMiscParamEncQualityLevel((void *)miscParamBuf->data);
        break;
    }

    default:
    {
        DDI_ASSERTMESSAGE("DDI: unsupported misc parameter type.");
        return VA_STATUS_ERROR_FLAG_NOT_SUPPORTED;
    }
    }

    return VA_STATUS_SUCCESS;
}

// get framerate code from frame rate value;
// frame_rate = frame_rate_value * (frame_rate_extension_n + 1)  (frame_rate_extension_d + 1)
uint32_t DdiEncodeMpeg2::CalculateFramerateCode(
    float   frameRate,
    uint8_t frameRateExtD,
    uint8_t frameRateExtN)
{
    if (frameRate <= 0.0)
    {
        return 0;
    }

    float frameRateValue = frameRate * (frameRateExtD + 1) / (frameRateExtN + 1);

    uint32_t delta         = 0xffffffff;
    uint32_t frameRateCode = 0;
    for (uint32_t i = 0; i < sizeof(frameRateTable) / sizeof(frameRateTable[0]); i++)
    {
        float tempResidual = frameRateTable[i].value - frameRateValue;
        tempResidual       = tempResidual < 0 ? -tempResidual : tempResidual;

        uint32_t residual = (uint32_t)(tempResidual * 1000);
        if (residual < delta)
        {
            delta         = residual;
            frameRateCode = frameRateTable[i].code;
        }
    }
    // if the gap from spec and custom requirement is too huge, error will be report
    if (delta > 1000)
    {
        return 0;
    }
    else
    {
        return frameRateCode;
    }
}

VAStatus DdiEncodeMpeg2::RemoveUserData()
{
    DDI_CHK_NULL(m_encodeCtx, "invalid encode context", VA_STATUS_ERROR_INVALID_CONTEXT);

    if (m_userDataListHead)
    {
        CodecEncodeMpeg2UserDataList *cur  = nullptr;
        CodecEncodeMpeg2UserDataList *next = nullptr;
        for (cur = (CodecEncodeMpeg2UserDataList *)m_userDataListHead; cur != nullptr; cur = next)
        {
            next = cur->m_nextItem;
            MOS_FreeMemory(cur->m_userData);
            MOS_FreeMemory(cur);
        }
        m_userDataListHead = nullptr;
        m_userDataListTail = nullptr;
    }
    return VA_STATUS_SUCCESS;
}

uint32_t DdiEncodeMpeg2::getSliceParameterBufferSize()
{
    return sizeof(VAEncSliceParameterBufferMPEG2);
}

uint32_t DdiEncodeMpeg2::getSequenceParameterBufferSize()
{
    return sizeof(VAEncSequenceParameterBufferMPEG2);
}

uint32_t DdiEncodeMpeg2::getPictureParameterBufferSize()
{
    return sizeof(VAEncPictureParameterBufferMPEG2);
}

uint32_t DdiEncodeMpeg2::getQMatrixBufferSize()
{
    return sizeof(VAIQMatrixBufferMPEG2);
}
