/*
* Copyright (c) 2015-2020, Intel Corporation
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
//! \file     media_ddi_decode_vp9.cpp
//! \brief    The class implementation of DdiDecodeVP9  for VP9 decode
//!

#include "media_libva_decoder.h"
#include "media_libva_util.h"

#include "media_ddi_decode_vp9.h"
#include "mos_solo_generic.h"
#include "codechal_memdecomp.h"
#include "media_ddi_decode_const.h"
#include "media_ddi_factory.h"

VAStatus DdiDecodeVP9::ParseSliceParams(
    DDI_MEDIA_CONTEXT         *mediaCtx,
    VASliceParameterBufferVP9 *slcParam)
{
    PCODEC_VP9_PIC_PARAMS     picParam  = (PCODEC_VP9_PIC_PARAMS)(m_ddiDecodeCtx->DecodeParams.m_picParams);
    PCODEC_VP9_SEGMENT_PARAMS segParams = (PCODEC_VP9_SEGMENT_PARAMS)(m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer);

    if ((slcParam == nullptr) ||
        (picParam == nullptr) ||
        (segParams == nullptr))
    {
        DDI_ASSERTMESSAGE("Invalid Parameter for Parsing VP9 Slice parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    picParam->BSBytesInBuffer = slcParam->slice_data_size;

    int32_t i, j, k;
    for (i = 0; i < 8; i++)
    {
        segParams->SegData[i].SegmentFlags.fields.SegmentReferenceEnabled = slcParam->seg_param[i].segment_flags.fields.segment_reference_enabled;
        segParams->SegData[i].SegmentFlags.fields.SegmentReference        = slcParam->seg_param[i].segment_flags.fields.segment_reference;
        segParams->SegData[i].SegmentFlags.fields.SegmentReferenceSkipped = slcParam->seg_param[i].segment_flags.fields.segment_reference_skipped;

        for (j = 0; j < 4; j++)
        {
            for (k = 0; k < 2; k++)
            {
                segParams->SegData[i].FilterLevel[j][k] = slcParam->seg_param[i].filter_level[j][k];
            }
        }

        segParams->SegData[i].LumaACQuantScale   = slcParam->seg_param[i].luma_ac_quant_scale;
        segParams->SegData[i].LumaDCQuantScale   = slcParam->seg_param[i].luma_dc_quant_scale;
        segParams->SegData[i].ChromaACQuantScale = slcParam->seg_param[i].chroma_ac_quant_scale;
        segParams->SegData[i].ChromaDCQuantScale = slcParam->seg_param[i].chroma_dc_quant_scale;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeVP9::ParsePicParams(
    DDI_MEDIA_CONTEXT              *mediaCtx,
    VADecPictureParameterBufferVP9 *picParam)
{
    PCODEC_VP9_PIC_PARAMS picVp9Params = (PCODEC_VP9_PIC_PARAMS)(m_ddiDecodeCtx->DecodeParams.m_picParams);

    if ((picParam == nullptr) || (picVp9Params == nullptr))
    {
        DDI_ASSERTMESSAGE("Invalid Parameter for Parsing VP9 Picture parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    picVp9Params->FrameHeightMinus1 = picParam->frame_height - 1;
    picVp9Params->FrameWidthMinus1  = picParam->frame_width - 1;

    picVp9Params->PicFlags.fields.frame_type                   = picParam->pic_fields.bits.frame_type;
    picVp9Params->PicFlags.fields.show_frame                   = picParam->pic_fields.bits.show_frame;
    picVp9Params->PicFlags.fields.error_resilient_mode         = picParam->pic_fields.bits.error_resilient_mode;
    picVp9Params->PicFlags.fields.intra_only                   = picParam->pic_fields.bits.intra_only;
    picVp9Params->PicFlags.fields.LastRefIdx                   = picParam->pic_fields.bits.last_ref_frame;
    picVp9Params->PicFlags.fields.LastRefSignBias              = picParam->pic_fields.bits.last_ref_frame_sign_bias;
    picVp9Params->PicFlags.fields.GoldenRefIdx                 = picParam->pic_fields.bits.golden_ref_frame;
    picVp9Params->PicFlags.fields.GoldenRefSignBias            = picParam->pic_fields.bits.golden_ref_frame_sign_bias;
    picVp9Params->PicFlags.fields.AltRefIdx                    = picParam->pic_fields.bits.alt_ref_frame;
    picVp9Params->PicFlags.fields.AltRefSignBias               = picParam->pic_fields.bits.alt_ref_frame_sign_bias;
    picVp9Params->PicFlags.fields.allow_high_precision_mv      = picParam->pic_fields.bits.allow_high_precision_mv;
    picVp9Params->PicFlags.fields.mcomp_filter_type            = picParam->pic_fields.bits.mcomp_filter_type;
    picVp9Params->PicFlags.fields.frame_parallel_decoding_mode = picParam->pic_fields.bits.frame_parallel_decoding_mode;
    picVp9Params->PicFlags.fields.segmentation_enabled         = picParam->pic_fields.bits.segmentation_enabled;
    picVp9Params->PicFlags.fields.segmentation_temporal_update = picParam->pic_fields.bits.segmentation_temporal_update;
    picVp9Params->PicFlags.fields.segmentation_update_map      = picParam->pic_fields.bits.segmentation_update_map;
    picVp9Params->PicFlags.fields.reset_frame_context          = picParam->pic_fields.bits.reset_frame_context;
    picVp9Params->PicFlags.fields.refresh_frame_context        = picParam->pic_fields.bits.refresh_frame_context;
    picVp9Params->PicFlags.fields.frame_context_idx            = picParam->pic_fields.bits.frame_context_idx;
    picVp9Params->PicFlags.fields.LosslessFlag                 = picParam->pic_fields.bits.lossless_flag;

    int32_t frameIdx;
    frameIdx = GetRenderTargetID(&m_ddiDecodeCtx->RTtbl, m_ddiDecodeCtx->RTtbl.pCurrentRT);
    if (frameIdx == (int32_t)DDI_CODEC_INVALID_FRAME_INDEX)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    picVp9Params->CurrPic.FrameIdx = frameIdx;

    int32_t i;
    for (i = 0; i < 8; i++)
    {
        if (picParam->reference_frames[i] < mediaCtx->uiNumSurfaces)
        {
            PDDI_MEDIA_SURFACE refSurface          = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->reference_frames[i]);
            frameIdx                               = GetRenderTargetID(&m_ddiDecodeCtx->RTtbl, refSurface);
            picVp9Params->RefFrameList[i].FrameIdx = ((uint32_t)frameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9) ? (CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9 - 1) : frameIdx;
        }
        else
        {
            PDDI_MEDIA_SURFACE refSurface          = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->reference_frames[i]);
            if (refSurface != nullptr)
            {
                frameIdx = GetRenderTargetID(&m_ddiDecodeCtx->RTtbl, refSurface);
                if (frameIdx != DDI_CODEC_INVALID_FRAME_INDEX)
                {
                    picVp9Params->RefFrameList[i].FrameIdx = ((uint32_t)frameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9) ? (CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9 - 1) : frameIdx;
                }
                else
                {
                    picVp9Params->RefFrameList[i].FrameIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9 - 1;
                }
            }
            else
            {
                picVp9Params->RefFrameList[i].FrameIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9 - 1;
            }
        }
    }

    picVp9Params->filter_level                    = picParam->filter_level;
    picVp9Params->sharpness_level                 = picParam->sharpness_level;
    picVp9Params->log2_tile_rows                  = picParam->log2_tile_rows;
    picVp9Params->log2_tile_columns               = picParam->log2_tile_columns;
    picVp9Params->UncompressedHeaderLengthInBytes = picParam->frame_header_length_in_bytes;
    picVp9Params->FirstPartitionSize              = picParam->first_partition_size;
    picVp9Params->profile                         = picParam->profile;

    /* Only 8bit depth is supported on picParam->profile=0.
     * If picParam->profile=2,3, it is possible to support the 10/12 bit-depth.
     * otherwise the bit_depth is 8.
     */
    if (((picParam->profile == CODEC_PROFILE_VP9_PROFILE2) ||
            (picParam->profile == CODEC_PROFILE_VP9_PROFILE3)) &&
        (picParam->bit_depth >= 8))
    {
        picVp9Params->BitDepthMinus8 = picParam->bit_depth - 8;
    }
    else
    {
        picVp9Params->BitDepthMinus8 = 0;
    }

    picVp9Params->subsampling_x = picParam->pic_fields.bits.subsampling_x;
    picVp9Params->subsampling_y = picParam->pic_fields.bits.subsampling_y;

    MOS_SecureMemcpy(picVp9Params->SegTreeProbs, 7, picParam->mb_segment_tree_probs, 7);
    MOS_SecureMemcpy(picVp9Params->SegPredProbs, 3, picParam->segment_pred_probs, 3);

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    // Picture Info
    uint32_t subSamplingSum = picVp9Params->subsampling_x + picVp9Params->subsampling_y;
    DECODE_EVENTDATA_INFO_PICTUREVA eventData = {0};
    eventData.CodecFormat                   = m_ddiDecodeCtx->wMode;
    eventData.FrameType                     = picVp9Params->PicFlags.fields.frame_type == 0 ? I_TYPE : MIXED_TYPE;
    eventData.PicStruct                     = FRAME_PICTURE;
    eventData.Width                         = picVp9Params->FrameWidthMinus1 + 1;
    eventData.Height                        = picVp9Params->FrameHeightMinus1 + 1;
    eventData.Bitdepth                      = picVp9Params->BitDepthMinus8 + 8;
    eventData.ChromaFormat                  = (subSamplingSum == 2) ? 1 : (subSamplingSum == 1 ? 2 : 3);  // 1-4:2:0; 2-4:2:2; 3-4:4:4
    eventData.EnabledSegment                = picVp9Params->PicFlags.fields.segmentation_enabled;
    MOS_TraceEvent(EVENT_DECODE_INFO_PICTUREVA, EVENT_TYPE_INFO, &eventData, sizeof(eventData), NULL, 0);
#endif

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeVP9::SetDecodeParams()
{
     DDI_CHK_RET(DdiMediaDecode::SetDecodeParams(),"SetDecodeParams failed!");
#ifdef _DECODE_PROCESSING_SUPPORTED
    // Bridge the SFC input with vdbox output
    if (m_decProcessingType == VA_DEC_PROCESSING)
    {
        auto procParams =
            (DecodeProcessingParams *)m_ddiDecodeCtx->DecodeParams.m_procParams;
        procParams->m_inputSurface = (&m_ddiDecodeCtx->DecodeParams)->m_destSurface;
        // codechal_decode_sfc.c expects Input Width/Height information.
        procParams->m_inputSurface->dwWidth  = procParams->m_inputSurface->OsResource.iWidth;
        procParams->m_inputSurface->dwHeight = procParams->m_inputSurface->OsResource.iHeight;
        procParams->m_inputSurface->dwPitch  = procParams->m_inputSurface->OsResource.iPitch;
        procParams->m_inputSurface->Format   = procParams->m_inputSurface->OsResource.Format;

        if(m_requireInputRegion)
        {
            procParams->m_inputSurfaceRegion.m_x = 0;
            procParams->m_inputSurfaceRegion.m_y = 0;
            procParams->m_inputSurfaceRegion.m_width = procParams->m_inputSurface->dwWidth;
            procParams->m_inputSurfaceRegion.m_height = procParams->m_inputSurface->dwHeight;
        }
    }
#endif
     return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeVP9::RenderPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VABufferID       *buffers,
    int32_t          numBuffers)
{
    VAStatus           va = VA_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);

    DDI_FUNCTION_ENTER();

    void             *data = nullptr;
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
            if (slcFlag)
            {
                // VP9 only supports only one slice_data. If it is passed, another slice_data
                // buffer will be ignored.
                DDI_NORMALMESSAGE("Slice data is already rendered\n");
                break;
            }
            int32_t index = GetBitstreamBufIndexFromBuffer(&m_ddiDecodeCtx->BufMgr, buf);
            if (index == DDI_CODEC_INVALID_BUFFER_INDEX)
            {
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }

            DdiMedia_MediaBufferToMosResource(m_ddiDecodeCtx->BufMgr.pBitStreamBuffObject[index], &m_ddiDecodeCtx->BufMgr.resBitstreamBuffer);
            m_ddiDecodeCtx->DecodeParams.m_dataSize += dataSize;
            slcFlag = true;

            break;
        }
        case VASliceParameterBufferType:
        {
            if (m_ddiDecodeCtx->DecodeParams.m_numSlices)
            {
                // VP9 only supports only one slice. If it is passed, another slice_param
                // buffer will be ignored.
                DDI_NORMALMESSAGE("SliceParamBufferVP9 is already rendered\n");
                break;
            }
            if (buf->uiNumElements == 0)
            {
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }

            VASliceParameterBufferVP9 *slcInfoVP9 = (VASliceParameterBufferVP9 *)data;
            DDI_CHK_RET(ParseSliceParams(mediaCtx, slcInfoVP9),"ParseSliceParams failed!");
            m_ddiDecodeCtx->DecodeParams.m_numSlices++;
            m_groupIndex++;
            break;
        }
        case VAPictureParameterBufferType:
        {
            VADecPictureParameterBufferVP9 *picParam;

            picParam = (VADecPictureParameterBufferVP9 *)data;
            DDI_CHK_RET(ParsePicParams(mediaCtx, picParam),"ParsePicParams failed!");
            break;
        }

        case VAProcPipelineParameterBufferType:
        {
            DDI_CHK_RET(ParseProcessingBuffer(mediaCtx, data),"ParseProcessingBuffer failed!");
            break;
        }
        case VADecodeStreamoutBufferType:
        {
            DdiMedia_MediaBufferToMosResource(buf, &m_ddiDecodeCtx->BufMgr.resExternalStreamOutBuffer);
            m_streamOutEnabled = true;
            break;
        }

        default:
            va = m_ddiDecodeCtx->pCpDdiInterface->RenderCencPicture(ctx, context, buf, data);
            break;
        }
        DdiMedia_UnmapBuffer(ctx, buffers[i]);
    }

    DDI_FUNCTION_EXIT(va);
    return va;
}

VAStatus DdiDecodeVP9::InitResourceBuffer()
{
    VAStatus                  vaStatus = VA_STATUS_SUCCESS;
    DDI_CODEC_COM_BUFFER_MGR *bufMgr   = &(m_ddiDecodeCtx->BufMgr);

    bufMgr->pSliceData = nullptr;

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

    // VP9 can support only one SliceDataBuffer. In such case only one is enough.
    // 2 is allocated for the safety.
    bufMgr->m_maxNumSliceData = 2;
    bufMgr->pSliceData        = (DDI_CODEC_BITSTREAM_BUFFER_INFO *)MOS_AllocAndZeroMemory(sizeof(bufMgr->pSliceData[0]) * 2);

    if (bufMgr->pSliceData == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    bufMgr->dwNumSliceData    = 0;
    bufMgr->dwNumSliceControl = 0;

    bufMgr->Codec_Param.Codec_Param_VP9.pVASliceParaBufVP9 = (VASliceParameterBufferVP9 *)MOS_AllocAndZeroMemory(sizeof(VASliceParameterBufferVP9));
    if (bufMgr->Codec_Param.Codec_Param_VP9.pVASliceParaBufVP9 == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    return VA_STATUS_SUCCESS;

finish:
    FreeResourceBuffer();
    return vaStatus;
}

void DdiDecodeVP9::FreeResourceBuffer()
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

    if (bufMgr->Codec_Param.Codec_Param_VP9.pVASliceParaBufVP9)
    {
        MOS_FreeMemory(bufMgr->Codec_Param.Codec_Param_VP9.pVASliceParaBufVP9);
        bufMgr->Codec_Param.Codec_Param_VP9.pVASliceParaBufVP9 = nullptr;
    }

    // free decode bitstream buffer object
    MOS_FreeMemory(bufMgr->pSliceData);
    bufMgr->pSliceData = nullptr;
}

uint8_t* DdiDecodeVP9::GetPicParamBuf(
    DDI_CODEC_COM_BUFFER_MGR    *bufMgr)
{
    return (uint8_t*)(&(bufMgr->Codec_Param.Codec_Param_VP9.PicParamVP9));
}

VAStatus DdiDecodeVP9::AllocSliceControlBuffer(
    DDI_MEDIA_BUFFER       *buf)
{
    DDI_CODEC_COM_BUFFER_MGR   *bufMgr;

    bufMgr     = &(m_ddiDecodeCtx->BufMgr);
    if(bufMgr->Codec_Param.Codec_Param_VP9.pVASliceParaBufVP9 == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    buf->pData    = (uint8_t*)bufMgr->Codec_Param.Codec_Param_VP9.pVASliceParaBufVP9;
    buf->uiOffset = bufMgr->dwNumSliceControl * sizeof(VASliceParameterBufferVP9);

    bufMgr->dwNumSliceControl += buf->uiNumElements;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeVP9::CodecHalInit(
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

    m_codechalSettings->codecFunction = codecFunction;
    m_codechalSettings->width       = m_width;
    m_codechalSettings->height      = m_height;
    m_codechalSettings->intelEntrypointInUse = false;

    m_codechalSettings->lumaChromaDepth = CODECHAL_LUMA_CHROMA_DEPTH_8_BITS;
    if ((m_ddiDecodeAttr->profile == VAProfileVP9Profile2) ||
        (m_ddiDecodeAttr->profile == VAProfileVP9Profile3))
    {
        m_codechalSettings->lumaChromaDepth |= CODECHAL_LUMA_CHROMA_DEPTH_10_BITS;
    }

    m_codechalSettings->shortFormatInUse = m_ddiDecodeCtx->bShortFormatInUse;

    m_codechalSettings->mode           = CODECHAL_DECODE_MODE_VP9VLD;
    m_codechalSettings->standard       = CODECHAL_VP9;
    m_codechalSettings->chromaFormat = HCP_CHROMA_FORMAT_YUV420;

    if(m_ddiDecodeAttr->profile == VAProfileVP9Profile1 ||
       m_ddiDecodeAttr->profile == VAProfileVP9Profile3)
    {
        m_codechalSettings->chromaFormat = HCP_CHROMA_FORMAT_YUV444;
    }
    
    m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer = MOS_AllocAndZeroMemory(sizeof(CODEC_VP9_SEGMENT_PARAMS));
    if (m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }
    m_ddiDecodeCtx->DecodeParams.m_picParams = MOS_AllocAndZeroMemory(sizeof(CODEC_VP9_PIC_PARAMS));
    if (m_ddiDecodeCtx->DecodeParams.m_picParams == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }
#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_decProcessingType == VA_DEC_PROCESSING)
    {
        DecodeProcessingParams *procParams = nullptr;
        
        m_codechalSettings->downsamplingHinted = true;
        
        procParams = (DecodeProcessingParams *)MOS_AllocAndZeroMemory(sizeof(DecodeProcessingParams));
        if (procParams == nullptr)
        {
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            goto CleanUpandReturn;
        }
        
        m_ddiDecodeCtx->DecodeParams.m_procParams = procParams;
        procParams->m_outputSurface = (PMOS_SURFACE)MOS_AllocAndZeroMemory(sizeof(MOS_SURFACE));
        if (procParams->m_outputSurface == nullptr)
        {
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            goto CleanUpandReturn;
        }
    }
#endif
    vaStatus = CreateCodecHal(mediaCtx,
        ptr,
        &standardInfo);

    if (vaStatus != VA_STATUS_SUCCESS)
    {
        goto CleanUpandReturn;
    }

    if (InitResourceBuffer() != VA_STATUS_SUCCESS)
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
#ifdef _DECODE_PROCESSING_SUPPORTED
        if (m_ddiDecodeCtx->DecodeParams.m_procParams)
        {
            auto procParams =
                (DecodeProcessingParams *)m_ddiDecodeCtx->DecodeParams.m_procParams;
            MOS_FreeMemory(procParams->m_outputSurface);
            
            MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_procParams);
            m_ddiDecodeCtx->DecodeParams.m_procParams = nullptr;
        }
#endif

    return vaStatus;
}

VAStatus DdiDecodeVP9::InitDecodeParams(
    VADriverContextP ctx,
    VAContextID      context)
{
    slcFlag = false;
    /* skip the mediaCtx check as it is checked in caller */
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_RET(DecodeCombineBitstream(mediaCtx),"DecodeCombineBitstream failed!");
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_ddiDecodeCtx->BufMgr);
    bufMgr->dwNumSliceData    = 0;
    bufMgr->dwNumSliceControl = 0;

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_ddiDecodeCtx->RTtbl);

    if ((rtTbl == nullptr) || (rtTbl->pCurrentRT == nullptr))
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    return VA_STATUS_SUCCESS;
}

MOS_FORMAT DdiDecodeVP9::GetFormat()
{
    slcFlag = false;
    MOS_FORMAT Format = Format_NV12;
    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_ddiDecodeCtx->RTtbl);
    CodechalDecodeParams *decodeParams = &m_ddiDecodeCtx->DecodeParams;

    CODEC_VP9_PIC_PARAMS *picParams = (CODEC_VP9_PIC_PARAMS *)decodeParams->m_picParams;
    if((picParams->profile == CODEC_PROFILE_VP9_PROFILE1) &&
        (picParams->BitDepthMinus8 == 0))
    {
        Format = Format_AYUV;
    }
    if (((picParams->profile == CODEC_PROFILE_VP9_PROFILE2) ||
        (picParams->profile == CODEC_PROFILE_VP9_PROFILE3)) &&
        (picParams->BitDepthMinus8 > 0))
    {
        Format = Format_P010;
        if((picParams->BitDepthMinus8 > 2) || (rtTbl->pCurrentRT->format == Media_Format_P016 || rtTbl->pCurrentRT->format == Media_Format_P012))
        {
            Format = Format_P016;
        }
        if ((picParams->subsampling_x == 1) && (picParams->subsampling_y == 0))
        {
            Format = Format_Y210;
        }
        else if ((picParams->subsampling_x == 0) && (picParams->subsampling_y == 0))
        {
            if(picParams->BitDepthMinus8 == 2)
            {
                Format = Format_Y410;

                //10bit decode in 12bit
#if VA_CHECK_VERSION(1, 9, 0)
                if(rtTbl->pCurrentRT->format == Media_Format_Y416 || rtTbl->pCurrentRT->format == Media_Format_Y412)
#else
                if(rtTbl->pCurrentRT->format == Media_Format_Y416)
#endif
                {
                    Format = Format_Y416;
                }
            }
            else if(picParams->BitDepthMinus8 > 2)
            {
                Format = Format_Y416;
            }
        }
    }
    return Format;
}

void DdiDecodeVP9::DestroyContext(
    VADriverContextP ctx)
{
    FreeResourceBuffer();
    // explicitly call the base function to do the further clean-up
    DdiMediaDecode::DestroyContext(ctx);
}

void DdiDecodeVP9::ContextInit(
    int32_t picWidth,
    int32_t picHeight)
{
    // call the function in base class to initialize it.
    DdiMediaDecode::ContextInit(picWidth, picHeight);

    m_ddiDecodeCtx->wMode    = CODECHAL_DECODE_MODE_VP9VLD;
}

extern template class MediaDdiFactory<DdiMediaDecode, DDI_DECODE_CONFIG_ATTR>;

static bool vp9Registered =
    MediaDdiFactory<DdiMediaDecode, DDI_DECODE_CONFIG_ATTR>::RegisterCodec<DdiDecodeVP9>(DECODE_ID_VP9);
