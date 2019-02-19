/*
* Copyright (c) 2015-2017, Intel Corporation
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
//! \file     media_ddi_decode_vp8.cpp
//! \brief    The class implementation of DdiDecodeVP8  for VP8 decode
//!

#include "media_libva_decoder.h"
#include "media_libva_util.h"

#include "media_ddi_decode_vp8.h"
#include "mos_solo_generic.h"
#include "codechal_memdecomp.h"
#include "media_ddi_decode_const.h"
#include "media_ddi_factory.h"

#define DDI_DECODE_VP8_QINDEX_RANGE 128

// Tables from the VP8 reference decoder
static const int32_t Vp8DcQlookup[DDI_DECODE_VP8_QINDEX_RANGE] =
{
4, 5, 6, 7, 8, 9, 10, 10, 11, 12, 13, 14, 15,
16, 17, 17, 18, 19, 20, 20, 21, 21, 22, 22, 23, 23,
24, 25, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
36, 37, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 46,
47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
73, 74, 75, 76, 76, 77, 78, 79, 80, 81, 82, 83, 84,
85, 86, 87, 88, 89, 91, 93, 95, 96, 98, 100, 101, 102,
104, 106, 108, 110, 112, 114, 116, 118, 122, 124, 126, 128, 130,
132, 134, 136, 138, 140, 143, 145, 148, 151, 154, 157,
};

static const int32_t Vp8AcQlookup[DDI_DECODE_VP8_QINDEX_RANGE] =
{
4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
56, 57, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 78,
80, 82, 84, 86, 88, 90, 92, 94, 96, 98, 100, 102, 104,
106, 108, 110, 112, 114, 116, 119, 122, 125, 128, 131, 134, 137,
140, 143, 146, 149, 152, 155, 158, 161, 164, 167, 170, 173, 177,
181, 185, 189, 193, 197, 201, 205, 209, 213, 217, 221, 225, 229,
234, 239, 245, 249, 254, 259, 264, 269, 274, 279, 284,
};

// Load VP8 Slice Parameters from the libva buffer into the Codec Hal Picture Parameters buffer.
// 2 libva buffers (VASliceParameterBufferVP8 & VAPictureParameterBufferVP8) get merged into one CodecHal buffer (CODEC_VP8_PIC_PARAMS)
// Only one sliceParameterBuffer is enough for VP8.
VAStatus DdiDecodeVP8::ParseSliceParams(
    DDI_MEDIA_CONTEXT         *mediaCtx,
    VASliceParameterBufferVP8 *slcParam)
{
    PCODEC_VP8_PIC_PARAMS picParams = (PCODEC_VP8_PIC_PARAMS)(m_ddiDecodeCtx->DecodeParams.m_picParams);

    if ((slcParam == nullptr) || (picParams == nullptr))
    {
        DDI_ASSERTMESSAGE("Invalid Parameter for Parsing VP8 Slice parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    // num_of_partitions = (1 << CodedCoeffTokenPartition)+1, count both control partition(frame header) and token partition.
    uint8_t num_token_partitions;
    num_token_partitions                = slcParam->num_of_partitions - 1;
    picParams->CodedCoeffTokenPartition = (num_token_partitions != 8) ? (num_token_partitions >> 1) : 3;
    //macroblock_offset is in unit of bit.it should be always the next byte, the byte is divided to two parts
    //used bits and remaining bits, if used bits == 8, uiFirstMbByteOffset should add 1, so use 8 to do the ceil operator
    picParams->uiFirstMbByteOffset = slcParam->slice_data_offset + ((slcParam->macroblock_offset + 8) >> 3);

    MOS_SecureMemcpy(picParams->uiPartitionSize, sizeof(picParams->uiPartitionSize), slcParam->partition_size, sizeof(picParams->uiPartitionSize));
    //partition 0 size in command buffer includes the one byte in bool decoder if remaining bits of bool decoder is zero.
    picParams->uiPartitionSize[0] -= (slcParam->macroblock_offset & 0x7) ? 0 : 1;

    return VA_STATUS_SUCCESS;
}

static uint16_t Vp8QuantIdx(int16_t index)
{
    if (index > 127)
        index = 127;
    if (index < 0)
        index = 0;

    return index;
}

static uint16_t Vp8AcQuant(uint16_t index)
{
    return Vp8AcQlookup[Vp8QuantIdx(index)];
}

static uint16_t Vp8DcQuant(uint16_t index)
{
    return Vp8DcQlookup[Vp8QuantIdx(index)];
}

// Copy VP8 IQ Matrix from the libva buffer into the Codec Hal buffer. A memcpy can be used, as the buffers have the same structure.
VAStatus DdiDecodeVP8::ParseIQMatrix(
    DDI_MEDIA_CONTEXT *  mediaCtx,
    VAIQMatrixBufferVP8 *matrix)
{
    CODEC_VP8_IQ_MATRIX_PARAMS *iqParams = (CODEC_VP8_IQ_MATRIX_PARAMS *)(m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer);

    if ((matrix == nullptr) || (iqParams == nullptr))
    {
        DDI_ASSERTMESSAGE("Invalid Parameter for Parsing VP8 IQMatrix parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    uint32_t i;
    for (i = 0; i < 4; i++)
    {
        iqParams->quantization_values[i][0] = Vp8DcQuant(matrix->quantization_index[i][1]);
        iqParams->quantization_values[i][1] = Vp8AcQuant(matrix->quantization_index[i][0]);
        iqParams->quantization_values[i][2] = Vp8DcQuant(matrix->quantization_index[i][4]);
        iqParams->quantization_values[i][3] = Vp8AcQuant(matrix->quantization_index[i][5]);
        iqParams->quantization_values[i][4] = 2 * Vp8DcQuant(matrix->quantization_index[i][2]);
        iqParams->quantization_values[i][5] = 155 * Vp8AcQuant(matrix->quantization_index[i][3]) / 100;

        if (iqParams->quantization_values[i][5] < 8)
            iqParams->quantization_values[i][5] = 8;
        if (iqParams->quantization_values[i][2] > 132)
            iqParams->quantization_values[i][2] = 132;
    }

    return VA_STATUS_SUCCESS;
}

// Load VP8 Picture Parameters from the libva buffer into the Codec Hal buffer.
VAStatus DdiDecodeVP8::ParsePicParams(
    DDI_MEDIA_CONTEXT           *mediaCtx,
    VAPictureParameterBufferVP8 *picParam)
{
    PDDI_MEDIA_SURFACE lastRefSurface   = nullptr;
    PDDI_MEDIA_SURFACE goldenRefSurface = nullptr;
    PDDI_MEDIA_SURFACE altRefSurface    = nullptr;

    PCODEC_VP8_PIC_PARAMS codecPicParams = (PCODEC_VP8_PIC_PARAMS)(m_ddiDecodeCtx->DecodeParams.m_picParams);

    PDDI_MEDIA_SURFACE *vp8Surfaces = m_ddiDecodeCtx->BufMgr.Codec_Param.Codec_Param_VP8.pReferenceFrames;

    PDDI_MEDIA_SURFACE currentSurface = m_ddiDecodeCtx->RTtbl.pCurrentRT;

    // only no-keyframe have last/gold/alt reference frame
    if (picParam->pic_fields.bits.key_frame)
    {
        lastRefSurface   = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->last_ref_frame);
        if(lastRefSurface)
        {
            DdiMedia_MediaSurfaceToMosResource(lastRefSurface, &m_resNoneRegLastRefFrame);
            m_ddiDecodeCtx->DecodeParams.m_presNoneRegLastRefFrame = &m_resNoneRegLastRefFrame;
            RegisterRTSurfaces(&m_ddiDecodeCtx->RTtbl, lastRefSurface);
        }
        goldenRefSurface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->golden_ref_frame);
        if(goldenRefSurface)
        {
            DdiMedia_MediaSurfaceToMosResource(lastRefSurface, &m_resNoneRegGoldenRefFrame);
            m_ddiDecodeCtx->DecodeParams.m_presNoneRegGoldenRefFrame = &m_resNoneRegGoldenRefFrame;
            RegisterRTSurfaces(&m_ddiDecodeCtx->RTtbl, lastRefSurface);
        }
        altRefSurface    = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->alt_ref_frame);
        if(altRefSurface)
        {
            DdiMedia_MediaSurfaceToMosResource(lastRefSurface, &m_resNoneRegAltRefFrame);
            m_ddiDecodeCtx->DecodeParams.m_presNoneRegAltRefFrame = &m_resNoneRegAltRefFrame;
            RegisterRTSurfaces(&m_ddiDecodeCtx->RTtbl, lastRefSurface);
        }
    }

    int32_t frameIdx;
    frameIdx = GetRenderTargetID(&m_ddiDecodeCtx->RTtbl, currentSurface);
    if (frameIdx == (int32_t)DDI_CODEC_INVALID_FRAME_INDEX)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    codecPicParams->ucCurrPicIndex = frameIdx;

    frameIdx                          = GetRenderTargetID(&m_ddiDecodeCtx->RTtbl, lastRefSurface);
    codecPicParams->ucLastRefPicIndex = ((uint32_t)frameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8) ? (CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8 - 1) : frameIdx;

    frameIdx                            = GetRenderTargetID(&m_ddiDecodeCtx->RTtbl, goldenRefSurface);
    codecPicParams->ucGoldenRefPicIndex = ((uint32_t)frameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8) ? (CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8 - 1) : frameIdx;

    frameIdx                         = GetRenderTargetID(&m_ddiDecodeCtx->RTtbl, altRefSurface);
    codecPicParams->ucAltRefPicIndex = ((uint32_t)frameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8) ? (CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8 - 1) : frameIdx;

    codecPicParams->CurrPic.FrameIdx            = codecPicParams->ucCurrPicIndex;
    codecPicParams->wFrameWidthInMbsMinus1      = ((picParam->frame_width + 15) / 16) - 1;
    codecPicParams->wFrameHeightInMbsMinus1     = ((picParam->frame_height + 15) / 16) - 1;
    codecPicParams->ucDeblockedPicIndex         = codecPicParams->ucCurrPicIndex;
    codecPicParams->ucReserved8Bits             = 0;
    codecPicParams->key_frame                   = (picParam->pic_fields.bits.key_frame == 0);  // Yes, really.
    codecPicParams->version                     = picParam->pic_fields.bits.version;
    codecPicParams->segmentation_enabled        = picParam->pic_fields.bits.segmentation_enabled;
    codecPicParams->update_mb_segmentation_map  = picParam->pic_fields.bits.update_mb_segmentation_map;
    codecPicParams->update_segment_feature_data = picParam->pic_fields.bits.update_segment_feature_data;
    codecPicParams->filter_type                 = picParam->pic_fields.bits.filter_type;
    codecPicParams->sign_bias_golden            = picParam->pic_fields.bits.sign_bias_golden;
    codecPicParams->sign_bias_alternate         = picParam->pic_fields.bits.sign_bias_alternate;
    codecPicParams->mb_no_coeff_skip            = picParam->pic_fields.bits.mb_no_coeff_skip;
    codecPicParams->mode_ref_lf_delta_update    = picParam->pic_fields.bits.mode_ref_lf_delta_update;

    // Loop filter settings
    codecPicParams->LoopFilterDisable                = picParam->pic_fields.bits.loop_filter_disable;
    codecPicParams->loop_filter_adj_enable           = picParam->pic_fields.bits.loop_filter_adj_enable;
    *((uint32_t *)codecPicParams->ucLoopFilterLevel) = *((uint32_t *)picParam->loop_filter_level);
    *((uint32_t *)codecPicParams->cRefLfDelta)       = *((uint32_t *)picParam->loop_filter_deltas_ref_frame);
    *((uint32_t *)codecPicParams->cModeLfDelta)      = *((uint32_t *)picParam->loop_filter_deltas_mode);
    codecPicParams->ucSharpnessLevel                 = picParam->pic_fields.bits.sharpness_level;

    // Probability settings
    codecPicParams->cMbSegmentTreeProbs[0]      = picParam->mb_segment_tree_probs[0];
    codecPicParams->cMbSegmentTreeProbs[1]      = picParam->mb_segment_tree_probs[1];
    codecPicParams->cMbSegmentTreeProbs[2]      = picParam->mb_segment_tree_probs[2];
    codecPicParams->ucProbSkipFalse             = picParam->prob_skip_false;
    codecPicParams->ucProbIntra                 = picParam->prob_intra;
    codecPicParams->ucProbLast                  = picParam->prob_last;
    codecPicParams->ucProbGolden                = picParam->prob_gf;
    *((uint32_t *)codecPicParams->ucYModeProbs) = *((uint32_t *)picParam->y_mode_probs);
    codecPicParams->ucUvModeProbs[0]            = picParam->uv_mode_probs[0];
    codecPicParams->ucUvModeProbs[1]            = picParam->uv_mode_probs[1];
    codecPicParams->ucUvModeProbs[2]            = picParam->uv_mode_probs[2];
    if (codecPicParams->ucMvUpdateProb[0] && picParam->mv_probs[0])
    {
        MOS_SecureMemcpy(codecPicParams->ucMvUpdateProb[0],
            sizeof(codecPicParams->ucMvUpdateProb[0]),
            picParam->mv_probs[0],
            sizeof(codecPicParams->ucMvUpdateProb[0]));
    }
    if (codecPicParams->ucMvUpdateProb[1] && picParam->mv_probs[1])
    {
        MOS_SecureMemcpy(codecPicParams->ucMvUpdateProb[1],
            sizeof(codecPicParams->ucMvUpdateProb[1]),
            picParam->mv_probs[1],
            sizeof(codecPicParams->ucMvUpdateProb[1]));
    }
    codecPicParams->ucP0EntropyCount = (8 - picParam->bool_coder_ctx.count) & 0x7;  //hardware needs used bits not remaining bits in bool decoder
    codecPicParams->ucP0EntropyValue = picParam->bool_coder_ctx.value;
    codecPicParams->uiP0EntropyRange = picParam->bool_coder_ctx.range;

    codecPicParams->uiStatusReportFeedbackNumber = 0;
    return VA_STATUS_SUCCESS;
}

// Copy VP8ProbabilityData from the libva buffer (pBuf) into the Codec Hal buffer. A memcpy can be used, as the buffers have the same structure.
VAStatus DdiDecodeVP8::ParseProbabilityData(
    struct _DDI_MEDIA_BUFFER *  vp8ProbDataBuff,
    VAProbabilityDataBufferVP8 *probInputBuf)
{
    if (vp8ProbDataBuff->pData && probInputBuf)
    {
        MOS_LINUX_BO *boDes = nullptr;
        boDes               = vp8ProbDataBuff->bo;

        mos_bo_wait_rendering(boDes);
        if (0 == mos_bo_subdata(boDes,
                     0,
                     sizeof(CODECHAL_VP8_COEFFPROB_DATA),
                     probInputBuf))
        {
            return VA_STATUS_SUCCESS;
        }
        else
        {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
    }
    else
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
}

VAStatus DdiDecodeVP8::RenderPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VABufferID       *buffers,
    int32_t          numBuffers)
{
    VAStatus           va = VA_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);

    DDI_FUNCTION_ENTER();
    void *            data = nullptr;
    for (int32_t i = 0; i < numBuffers; i++)
    {
        if (!buffers || (buffers[i] == VA_INVALID_ID))
        {
            return VA_STATUS_ERROR_INVALID_BUFFER;
        }

        DDI_MEDIA_BUFFER *buf = DdiMedia_GetBufferFromVABufferID(mediaCtx, buffers[i]);
        if (nullptr == buf)
        {
            return VA_STATUS_ERROR_INVALID_BUFFER;
        }

        uint32_t dataSize = buf->iSize;
        DdiMedia_MapBuffer(ctx, buffers[i], &data);

        if (data == nullptr)
        {
            return VA_STATUS_ERROR_INVALID_BUFFER;
        }

        switch ((int32_t)buf->uiType)
        {
        case VASliceDataBufferType:
        {
            int32_t index = GetBitstreamBufIndexFromBuffer(&m_ddiDecodeCtx->BufMgr, buf);
            if (index == DDI_CODEC_INVALID_BUFFER_INDEX)
            {
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }

            DdiMedia_MediaBufferToMosResource(m_ddiDecodeCtx->BufMgr.pBitStreamBuffObject[index], &m_ddiDecodeCtx->BufMgr.resBitstreamBuffer);
            m_ddiDecodeCtx->DecodeParams.m_dataSize += dataSize;
            break;
        }
        case VASliceParameterBufferType:
        {
            if (m_ddiDecodeCtx->DecodeParams.m_numSlices)
            {
                // VP8 only supports only one slice. If it is passed, another slice_param
                // buffer will be ignored.
                DDI_NORMALMESSAGE("SliceParamBufferVP8 is already rendered\n");
                break;
            }
            if (buf->uiNumElements == 0)
            {
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }
            uint32_t numSlices = buf->uiNumElements;

            VASliceParameterBufferVP8 *slcInfoVP8 = (VASliceParameterBufferVP8 *)data;
            for(uint32_t j = 0; j < numSlices; j++)
            {
                slcInfoVP8[j].slice_data_offset += GetBsBufOffset(m_groupIndex);
            }

            DDI_CHK_RET(ParseSliceParams(mediaCtx, slcInfoVP8),"ParseSliceParams failed!");
            m_ddiDecodeCtx->DecodeParams.m_numSlices += numSlices;
            m_groupIndex++;
            break;
        }
        case VAPictureParameterBufferType:
        {
            VAPictureParameterBufferVP8 *picParam = (VAPictureParameterBufferVP8 *)data;
            DDI_CHK_RET(ParsePicParams(mediaCtx, picParam),"ParsePicParams failed!");
            break;
        }
        case VAProbabilityBufferType:
        {
            VAProbabilityDataBufferVP8 *probInput = (VAProbabilityDataBufferVP8 *)data;
        DDI_CHK_RET(ParseProbabilityData(m_ddiDecodeCtx->BufMgr.Codec_Param.Codec_Param_VP8.pVP8ProbabilityDataBuffObject, probInput),"ParseProbabilityData failed!");
            DdiMedia_MediaBufferToMosResource(m_ddiDecodeCtx->BufMgr.Codec_Param.Codec_Param_VP8.pVP8ProbabilityDataBuffObject,
                &m_ddiDecodeCtx->BufMgr.Codec_Param.Codec_Param_VP8.resProbabilityDataBuffer);
            m_ddiDecodeCtx->DecodeParams.m_coefProbSize = dataSize;
            break;
        }
        case VAIQMatrixBufferType:
        {
            VAIQMatrixBufferVP8 *imxBuf = (VAIQMatrixBufferVP8 *)data;
            DDI_CHK_RET(ParseIQMatrix(mediaCtx, imxBuf),"ParseIQMatrix failed!");
            break;
        }

        case VADecodeStreamoutBufferType:
        {
            DdiMedia_MediaBufferToMosResource(buf, &m_ddiDecodeCtx->BufMgr.resExternalStreamOutBuffer);
            m_streamOutEnabled = true;
            break;
        }
        default:
            va = VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
            break;
        }
        DdiMedia_UnmapBuffer(ctx, buffers[i]);
    }

    DDI_FUNCTION_EXIT(va);
    return va;
}

VAStatus DdiDecodeVP8::InitResourceBuffer(DDI_MEDIA_CONTEXT *mediaCtx)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;

    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_ddiDecodeCtx->BufMgr);
    bufMgr->pSliceData               = nullptr;

    bufMgr->ui64BitstreamOrder = 0;
    bufMgr->dwMaxBsSize        = m_width *
                          m_height * 3 / 2;
    // minimal 10k bytes for some special case. Will refractor this later
    if (bufMgr->dwMaxBsSize < DDI_CODEC_MIN_VALUE_OF_MAX_BS_SIZE)
    {
        bufMgr->dwMaxBsSize = DDI_CODEC_MIN_VALUE_OF_MAX_BS_SIZE;
    }

    int32_t i;
    // init decode bitstream buffer object
    for (i = 0; i < DDI_CODEC_MAX_BITSTREAM_BUFFER; i++)
    {
        bufMgr->pBitStreamBuffObject[i] = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
        if (bufMgr->pBitStreamBuffObject[i] == nullptr)
        {
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            goto finish;
        }
        bufMgr->pBitStreamBuffObject[i]->iSize    = bufMgr->dwMaxBsSize;
        bufMgr->pBitStreamBuffObject[i]->uiType   = VASliceDataBufferType;
        bufMgr->pBitStreamBuffObject[i]->format   = Media_Format_Buffer;
        bufMgr->pBitStreamBuffObject[i]->uiOffset = 0;
        bufMgr->pBitStreamBuffObject[i]->bo       = nullptr;
        bufMgr->pBitStreamBase[i]                 = nullptr;
    }

    // VP8 can support up to eight token partitions. So the max number of sliceData is 8 + 1.
    // 10 is allocated for the safety.
    bufMgr->m_maxNumSliceData = 10;
    bufMgr->pSliceData        = (DDI_CODEC_BITSTREAM_BUFFER_INFO *)MOS_AllocAndZeroMemory(sizeof(bufMgr->pSliceData[0]) * 10);

    if (bufMgr->pSliceData == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    bufMgr->Codec_Param.Codec_Param_VP8.pVP8ProbabilityDataBuffObject = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
    if (bufMgr->Codec_Param.Codec_Param_VP8.pVP8ProbabilityDataBuffObject == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }
    bufMgr->Codec_Param.Codec_Param_VP8.pVP8ProbabilityDataBuffObject->iSize     = sizeof(CODECHAL_VP8_COEFFPROB_DATA);
    bufMgr->Codec_Param.Codec_Param_VP8.pVP8ProbabilityDataBuffObject->uiType    = VAProbabilityBufferType;
    bufMgr->Codec_Param.Codec_Param_VP8.pVP8ProbabilityDataBuffObject->format    = Media_Format_Buffer;
    bufMgr->Codec_Param.Codec_Param_VP8.pVP8ProbabilityDataBuffObject->uiOffset  = 0;
    bufMgr->Codec_Param.Codec_Param_VP8.pVP8ProbabilityDataBuffObject->pMediaCtx = mediaCtx;

    // Create a buffer of size iSize
    vaStatus = DdiMediaUtil_CreateBuffer(bufMgr->Codec_Param.Codec_Param_VP8.pVP8ProbabilityDataBuffObject, mediaCtx->pDrmBufMgr);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        goto finish;
    }

    bufMgr->Codec_Param.Codec_Param_VP8.pProbabilityDataBase = (uint8_t *)DdiMediaUtil_LockBuffer(bufMgr->Codec_Param.Codec_Param_VP8.pVP8ProbabilityDataBuffObject, MOS_LOCKFLAG_WRITEONLY);
    if (bufMgr->Codec_Param.Codec_Param_VP8.pProbabilityDataBase == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    bufMgr->dwNumSliceData    = 0;
    bufMgr->dwNumSliceControl = 0;

    // Max 4 slices/segments in VP8
    bufMgr->Codec_Param.Codec_Param_VP8.pVASliceParaBufVP8 = (VASliceParameterBufferVP8 *)MOS_AllocAndZeroMemory(sizeof(VASliceParameterBufferVP8) * 4);
    if (bufMgr->Codec_Param.Codec_Param_VP8.pVASliceParaBufVP8 == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    return VA_STATUS_SUCCESS;

finish:
    FreeResourceBuffer();
    return vaStatus;
}

void DdiDecodeVP8::FreeResourceBuffer()
{
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_ddiDecodeCtx->BufMgr);

    int32_t i;
    for (i = 0; i < DDI_CODEC_MAX_BITSTREAM_BUFFER; i++)
    {
        if (bufMgr->pBitStreamBase[i])
        {
            DdiMediaUtil_UnlockBuffer(bufMgr->pBitStreamBuffObject[i]);
            bufMgr->pBitStreamBase[i] = nullptr;
        }
        if (bufMgr->pBitStreamBuffObject[i])
        {
            DdiMediaUtil_FreeBuffer(bufMgr->pBitStreamBuffObject[i]);
            MOS_FreeMemory(bufMgr->pBitStreamBuffObject[i]);
            bufMgr->pBitStreamBuffObject[i] = nullptr;
        }
    }

    if (bufMgr->Codec_Param.Codec_Param_VP8.pVASliceParaBufVP8)
    {
        MOS_FreeMemory(bufMgr->Codec_Param.Codec_Param_VP8.pVASliceParaBufVP8);
        bufMgr->Codec_Param.Codec_Param_VP8.pVASliceParaBufVP8 = nullptr;
    }

    if (bufMgr->Codec_Param.Codec_Param_VP8.pVP8ProbabilityDataBuffObject)
    {
        DdiMediaUtil_UnlockBuffer(bufMgr->Codec_Param.Codec_Param_VP8.pVP8ProbabilityDataBuffObject);
        DdiMediaUtil_FreeBuffer(bufMgr->Codec_Param.Codec_Param_VP8.pVP8ProbabilityDataBuffObject);
        MOS_FreeMemory(bufMgr->Codec_Param.Codec_Param_VP8.pVP8ProbabilityDataBuffObject);
        bufMgr->Codec_Param.Codec_Param_VP8.pVP8ProbabilityDataBuffObject = nullptr;
    }

    // free decode bitstream buffer object
    MOS_FreeMemory(bufMgr->pSliceData);
    bufMgr->pSliceData = nullptr;
}

uint8_t* DdiDecodeVP8::GetPicParamBuf(
    DDI_CODEC_COM_BUFFER_MGR    *bufMgr)
{
    return (uint8_t*)(&(bufMgr->Codec_Param.Codec_Param_VP8.PicParamVP8));
}

VAStatus DdiDecodeVP8::AllocSliceControlBuffer(
    DDI_MEDIA_BUFFER       *buf)
{
    DDI_CODEC_COM_BUFFER_MGR   *bufMgr;

    bufMgr     = &(m_ddiDecodeCtx->BufMgr);

    if(bufMgr->Codec_Param.Codec_Param_VP8.pVASliceParaBufVP8 == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    buf->pData      = (uint8_t*)bufMgr->Codec_Param.Codec_Param_VP8.pVASliceParaBufVP8;
    buf->uiOffset   = bufMgr->dwNumSliceControl * sizeof(VASliceParameterBufferVP8);

    bufMgr->dwNumSliceControl += buf->uiNumElements;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeVP8::CodecHalInit(
    DDI_MEDIA_CONTEXT *mediaCtx,
    void              *ptr)
{
    VAStatus     vaStatus = VA_STATUS_SUCCESS;
    MOS_CONTEXT *mosCtx   = (MOS_CONTEXT *)ptr;

    CODECHAL_FUNCTION codecFunction = CODECHAL_FUNCTION_DECODE;
    m_ddiDecodeCtx->pCpDdiInterface->SetCpParams(m_ddiDecodeAttr->uiEncryptionType, m_codechalSettings);

    CODECHAL_STANDARD_INFO standardInfo;
    memset(&standardInfo, 0, sizeof(standardInfo));

    standardInfo.CodecFunction = codecFunction;
    standardInfo.Mode          = (CODECHAL_MODE)m_ddiDecodeCtx->wMode;

    m_codechalSettings->codecFunction                = codecFunction;
    m_codechalSettings->width                      = m_width;
    m_codechalSettings->height                     = m_height;
    m_codechalSettings->intelEntrypointInUse        = false;

    m_codechalSettings->lumaChromaDepth = CODECHAL_LUMA_CHROMA_DEPTH_8_BITS;

    m_codechalSettings->shortFormatInUse = m_ddiDecodeCtx->bShortFormatInUse;

    m_codechalSettings->mode     = CODECHAL_DECODE_MODE_VP8VLD;
    m_codechalSettings->standard = CODECHAL_VP8;

    m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer = MOS_AllocAndZeroMemory(sizeof(CODEC_VP8_IQ_MATRIX_PARAMS));
    if (m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }
    m_ddiDecodeCtx->DecodeParams.m_picParams = MOS_AllocAndZeroMemory(sizeof(CODEC_VP8_PIC_PARAMS));
    if (m_ddiDecodeCtx->DecodeParams.m_picParams == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }

    vaStatus = CreateCodecHal(mediaCtx,
        ptr,
        &standardInfo);

    if (vaStatus != VA_STATUS_SUCCESS)
    {
        goto CleanUpandReturn;
    }

    if (InitResourceBuffer(mediaCtx) != VA_STATUS_SUCCESS)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }

    return vaStatus;

CleanUpandReturn:
    FreeResourceBuffer();

    if (m_ddiDecodeCtx->pCodecHal)
    {
        m_ddiDecodeCtx->pCodecHal->Destroy();
        MOS_Delete(m_ddiDecodeCtx->pCodecHal);
        m_ddiDecodeCtx->pCodecHal = nullptr;
    }

    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer);
    m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer = nullptr;
    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_picParams);
    m_ddiDecodeCtx->DecodeParams.m_picParams = nullptr;
    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_sliceParams);
    m_ddiDecodeCtx->DecodeParams.m_sliceParams = nullptr;

    return vaStatus;
}

VAStatus DdiDecodeVP8::SetDecodeParams()
{
    DDI_CHK_RET(DdiMediaDecode::SetDecodeParams(),"SetDecodeParams failed!");
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_ddiDecodeCtx->BufMgr);
    (&m_ddiDecodeCtx->DecodeParams)->m_coefProbBuffer = &(bufMgr->Codec_Param.Codec_Param_VP8.resProbabilityDataBuffer);
    return  VA_STATUS_SUCCESS;
}

void DdiDecodeVP8::DestroyContext(
    VADriverContextP ctx)
{
    FreeResourceBuffer();
    // explicitly call the base function to do the further clean-up
    DdiMediaDecode::DestroyContext(ctx);
}

void DdiDecodeVP8::ContextInit(
    int32_t picWidth,
    int32_t picHeight)
{
    // call the function in base class to initialize it.
    DdiMediaDecode::ContextInit(picWidth, picHeight);

    m_ddiDecodeCtx->wMode    = CODECHAL_DECODE_MODE_VP8VLD;
}

extern template class MediaDdiFactory<DdiMediaDecode, DDI_DECODE_CONFIG_ATTR>;

static bool vp8Registered =
    MediaDdiFactory<DdiMediaDecode, DDI_DECODE_CONFIG_ATTR>::RegisterCodec<DdiDecodeVP8>(DECODE_ID_VP8);
