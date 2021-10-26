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
//! \file     media_ddi_decode_mpeg2.cpp
//! \brief    The class implementation of DdiDecodeMpeg2  for Mpeg2 decode
//!

#include "media_libva_decoder.h"
#include "media_libva_util.h"
#include "media_libva_caps.h"

#include "media_ddi_decode_mpeg2.h"
#include "mos_solo_generic.h"
#include "codechal_memdecomp.h"
#include "media_ddi_decode_const.h"
#include "media_ddi_factory.h"

void DdiDecodeMPEG2::ParseNumMbsForSlice(
    int32_t numSlices)
{
    CodecDecodeMpeg2PicParams *picParam = (CodecDecodeMpeg2PicParams *)(m_ddiDecodeCtx->DecodeParams.m_picParams);

    CodecDecodeMpeg2SliceParams *sliceParam = (CodecDecodeMpeg2SliceParams *)(m_ddiDecodeCtx->DecodeParams.m_sliceParams);

    uint16_t widthInMB, heightInMB, numMBInSlc;
    widthInMB  = m_picWidthInMB;
    heightInMB = m_picHeightInMB;

    int32_t                      slcIndex;
    CodecDecodeMpeg2SliceParams *nextSlc;
    for (slcIndex = 0; slcIndex < numSlices; slcIndex++)
    {
        if (slcIndex == numSlices - 1)  // last slice
        {
            numMBInSlc = widthInMB - sliceParam->m_sliceHorizontalPosition;
        }
        else
        {
            nextSlc    = sliceParam + 1;
            numMBInSlc = (nextSlc->m_sliceVerticalPosition * widthInMB + nextSlc->m_sliceHorizontalPosition) -
                         (sliceParam->m_sliceVerticalPosition * widthInMB + sliceParam->m_sliceHorizontalPosition);
        }

        sliceParam->m_numMbsForSlice = numMBInSlc & 0x007f;

        if (sliceParam->m_numMbsForSlice < numMBInSlc)
        {
            sliceParam->m_numMbsForSlice = widthInMB - sliceParam->m_sliceHorizontalPosition;
        }

        sliceParam++;
    }
}

VAStatus DdiDecodeMPEG2::ParseSliceParams(
    DDI_MEDIA_CONTEXT            *mediaCtx,
    VASliceParameterBufferMPEG2  *slcParam,
    uint32_t                      numSlices)
{
    CodecDecodeMpeg2PicParams *picParams = (CodecDecodeMpeg2PicParams *)(m_ddiDecodeCtx->DecodeParams.m_picParams);
    CodecDecodeMpeg2SliceParams *codecSlcParams = (CodecDecodeMpeg2SliceParams *)(m_ddiDecodeCtx->DecodeParams.m_sliceParams);
    codecSlcParams += m_ddiDecodeCtx->DecodeParams.m_numSlices;

    if ((slcParam == nullptr) ||
        (codecSlcParams == nullptr) ||
        (picParams == nullptr))
    {
        DDI_ASSERTMESSAGE("Invalid Parameter for Parsing Mpeg2 Slice parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    uint32_t isField;
    if ((picParams->m_currPic.PicFlags == PICTURE_TOP_FIELD) || (picParams->m_currPic.PicFlags == PICTURE_BOTTOM_FIELD))
    {
        isField = 1;
    }
    else
    {
        isField = 0;
    }

    uint32_t sliceBaseOffset = GetBsBufOffset(m_groupIndex);
    uint32_t slcCount;
    for (slcCount = 0; slcCount < numSlices; slcCount++)
    {
        codecSlcParams->m_sliceHorizontalPosition = slcParam->slice_horizontal_position;
        codecSlcParams->m_sliceVerticalPosition   = slcParam->slice_vertical_position;

        codecSlcParams->m_sliceDataSize   = (slcParam->slice_data_size) * 8;
        codecSlcParams->m_sliceDataOffset = slcParam->slice_data_offset + sliceBaseOffset;
        if (slcParam->slice_data_flag)
        {
            DDI_NORMALMESSAGE("The whole slice is not in the bitstream buffer for this Execute call");
        }
        codecSlcParams->m_macroblockOffset   = slcParam->macroblock_offset;
        codecSlcParams->m_quantiserScaleCode = slcParam->quantiser_scale_code;
        codecSlcParams->m_reservedBits       = 0;
        slcParam++;
        codecSlcParams++;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeMPEG2::ParseIQMatrix(
    DDI_MEDIA_CONTEXT     *mediaCtx,
    VAIQMatrixBufferMPEG2 *matrix)
{
    CodecMpeg2IqMatrix *iqMatrix = (CodecMpeg2IqMatrix *)(m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer);

    if ((iqMatrix == nullptr) || (matrix == nullptr))
    {
        DDI_ASSERTMESSAGE("Invalid Parameter for Parsing Mpeg2 IQ Matrix parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    iqMatrix->m_loadIntraQuantiserMatrix          = matrix->load_intra_quantiser_matrix;
    iqMatrix->m_loadNonIntraQuantiserMatrix       = matrix->load_non_intra_quantiser_matrix;
    iqMatrix->m_loadChromaIntraQuantiserMatrix    = matrix->load_chroma_intra_quantiser_matrix;
    iqMatrix->m_loadChromaNonIntraQuantiserMatrix = matrix->load_chroma_non_intra_quantiser_matrix;

    MOS_SecureMemcpy(iqMatrix->m_intraQuantiserMatrix,
        64,
        matrix->intra_quantiser_matrix,
        64);
    MOS_SecureMemcpy(iqMatrix->m_nonIntraQuantiserMatrix,
        64,
        matrix->non_intra_quantiser_matrix,
        64);
    MOS_SecureMemcpy(iqMatrix->m_chromaIntraQuantiserMatrix,
        64,
        matrix->chroma_intra_quantiser_matrix,
        64);
    MOS_SecureMemcpy(iqMatrix->m_chromaNonIntraQuantiserMatrix,
        64,
        matrix->chroma_non_intra_quantiser_matrix,
        64);
    return VA_STATUS_SUCCESS;
}

#define INTRA_CODED 0x1
#define PREDICTIVE_CODED 0x2
#define BIDIRECTIONAL_CODED 0x3

// S-(GMC) VOP coding used for MPEG4 part 2
//
#define S_CODED 0x4

VAStatus DdiDecodeMPEG2::ParsePicParams(
    DDI_MEDIA_CONTEXT             *mediaCtx,
    VAPictureParameterBufferMPEG2 *picParam)
{
    CodecDecodeMpeg2PicParams *codecPicParam = (CodecDecodeMpeg2PicParams *)(m_ddiDecodeCtx->DecodeParams.m_picParams);
    if ((codecPicParam == nullptr) || (picParam == nullptr))
    {
        DDI_ASSERTMESSAGE("Invalid Parameter for Parsing Mpeg2 Picture Parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    codecPicParam->m_currPic.FrameIdx = GetRenderTargetID(&(m_ddiDecodeCtx->RTtbl), m_ddiDecodeCtx->RTtbl.pCurrentRT);
    switch (picParam->picture_coding_extension.bits.picture_structure)
    {
    case TOP_FIELD:
        codecPicParam->m_currPic.PicFlags = PICTURE_TOP_FIELD;
        break;
    case BOTTOM_FIELD:
        codecPicParam->m_currPic.PicFlags = PICTURE_BOTTOM_FIELD;
        break;
    case FRAME_PICTURE:
        codecPicParam->m_currPic.PicFlags = PICTURE_FRAME;
        break;
    default:
        break;
    }
    codecPicParam->m_topFieldFirst     = picParam->picture_coding_extension.bits.top_field_first;
    codecPicParam->m_secondField       = !(picParam->picture_coding_extension.bits.is_first_field);
    codecPicParam->m_pictureCodingType = picParam->picture_coding_type;

    if (picParam->picture_coding_extension.bits.is_first_field)
    {
        if (codecPicParam->m_currPic.PicFlags == PICTURE_TOP_FIELD)
            codecPicParam->m_topFieldFirst = 1;
        if (codecPicParam->m_currPic.PicFlags == PICTURE_BOTTOM_FIELD)
            codecPicParam->m_topFieldFirst = 0;
    }
    else
    {
        if (codecPicParam->m_currPic.PicFlags == PICTURE_TOP_FIELD)
            codecPicParam->m_topFieldFirst = 0;

        if (codecPicParam->m_currPic.PicFlags == PICTURE_BOTTOM_FIELD)
            codecPicParam->m_topFieldFirst = 1;
    }
    if (picParam->picture_coding_type == INTRA_CODED)
    {
        codecPicParam->m_forwardRefIdx  = codecPicParam->m_currPic.FrameIdx;
        codecPicParam->m_backwardRefIdx = codecPicParam->m_currPic.FrameIdx;
    }
    else if (picParam->picture_coding_type == PREDICTIVE_CODED)
    {
        if (picParam->forward_reference_picture != VA_INVALID_SURFACE)
        {
            if (UpdateRegisteredRTSurfaceFlag(&(m_ddiDecodeCtx->RTtbl), DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->forward_reference_picture)) != VA_STATUS_SUCCESS)
            {
                DDI_CHK_RET(RegisterRTSurfaces(&(m_ddiDecodeCtx->RTtbl), DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->forward_reference_picture)), "RegisterRTSurfaces failed!");
            }

            codecPicParam->m_forwardRefIdx = GetRenderTargetID(&(m_ddiDecodeCtx->RTtbl), DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->forward_reference_picture));  //(unsigned char)GetMPEG2SurfaceIdx(&(pRTtbl->pRT[0]), picParam->forward_reference_picture);
        }
        else
        {
            codecPicParam->m_forwardRefIdx = codecPicParam->m_currPic.FrameIdx;
        }

        if (codecPicParam->m_secondField)
        {
            codecPicParam->m_backwardRefIdx = codecPicParam->m_currPic.FrameIdx;
        }
        else
        {
            codecPicParam->m_backwardRefIdx = codecPicParam->m_forwardRefIdx;
        }
    }
    else
    {
        if (picParam->forward_reference_picture != VA_INVALID_SURFACE)
        {
            if (UpdateRegisteredRTSurfaceFlag(&(m_ddiDecodeCtx->RTtbl), DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->forward_reference_picture)) != VA_STATUS_SUCCESS)
            {
                DDI_CHK_RET(RegisterRTSurfaces(&(m_ddiDecodeCtx->RTtbl), DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->forward_reference_picture)), "RegisterRTSurfaces failed!");
            }
            codecPicParam->m_forwardRefIdx = GetRenderTargetID(&(m_ddiDecodeCtx->RTtbl), DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->forward_reference_picture));  //(unsigned char)GetMPEG2SurfaceIdx(&(pRTtbl->pRT[0]), picParam->forward_reference_picture);
        }
        else
        {
            codecPicParam->m_forwardRefIdx = codecPicParam->m_currPic.FrameIdx;
        }
        if (picParam->backward_reference_picture != VA_INVALID_SURFACE)
        {
            if (UpdateRegisteredRTSurfaceFlag(&(m_ddiDecodeCtx->RTtbl), DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->backward_reference_picture)) != VA_STATUS_SUCCESS)
            {
                DDI_CHK_RET(RegisterRTSurfaces(&(m_ddiDecodeCtx->RTtbl), DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->backward_reference_picture)), "RegisterRTSurfaces failed!");
            }
            codecPicParam->m_backwardRefIdx = GetRenderTargetID(&(m_ddiDecodeCtx->RTtbl), DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->backward_reference_picture));  //(unsigned char)GetMPEG2SurfaceIdx(&(pRTtbl->pRT[0]), picParam->backward_reference_picture);
        }
        else
        {
            codecPicParam->m_backwardRefIdx = codecPicParam->m_currPic.FrameIdx;
        }
    }

    //add protection checking to prevent ref pic index larger than DPB size
    if (codecPicParam->m_forwardRefIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
    {
        codecPicParam->m_forwardRefIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2 - 1;
    }

    if (codecPicParam->m_backwardRefIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
    {
        codecPicParam->m_backwardRefIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2 - 1;
    }

    codecPicParam->W0.m_scanOrder          = picParam->picture_coding_extension.bits.alternate_scan;
    codecPicParam->W0.m_intraVlcFormat     = picParam->picture_coding_extension.bits.intra_vlc_format;
    codecPicParam->W0.m_quantizerScaleType = picParam->picture_coding_extension.bits.q_scale_type;
    codecPicParam->W0.m_concealmentMVFlag  = picParam->picture_coding_extension.bits.concealment_motion_vectors;
    codecPicParam->W0.m_frameDctPrediction = picParam->picture_coding_extension.bits.frame_pred_frame_dct;
    codecPicParam->W0.m_topFieldFirst      = picParam->picture_coding_extension.bits.top_field_first;
    codecPicParam->W0.m_intraDCPrecision   = picParam->picture_coding_extension.bits.intra_dc_precision;
    codecPicParam->W1.m_value              = 0;
    codecPicParam->W1.m_fcode11            = (picParam->f_code) & 0xf;
    codecPicParam->W1.m_fcode10            = (picParam->f_code >> 4) & 0xf;
    codecPicParam->W1.m_fcode01            = (picParam->f_code >> 8) & 0xf;
    codecPicParam->W1.m_fcode00            = (picParam->f_code >> 12) & 0xf;
    codecPicParam->m_horizontalSize        = picParam->horizontal_size;
    codecPicParam->m_verticalSize          = picParam->vertical_size;

    if (picParam->picture_coding_type == INTRA_CODED)
    {
        m_picWidthInMB  = (int16_t)(DDI_CODEC_NUM_MACROBLOCKS_WIDTH(picParam->horizontal_size));
        m_picHeightInMB = (int16_t)(DDI_CODEC_NUM_MACROBLOCKS_WIDTH(picParam->vertical_size));
    }

    if ((codecPicParam->m_currPic.PicFlags == PICTURE_TOP_FIELD) || (codecPicParam->m_currPic.PicFlags == PICTURE_BOTTOM_FIELD))
    {
        codecPicParam->m_verticalSize = picParam->vertical_size >> 1;
    }

    codecPicParam->m_statusReportFeedbackNumber = 0;

    DDI_CHK_NULL(mediaCtx, "Null mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_caps, "Null m_cpas", VA_STATUS_ERROR_INVALID_CONTEXT);
    VAStatus vaStatus = mediaCtx->m_caps->CheckDecodeResolution(
            m_ddiDecodeCtx->wMode,
            VAProfileMPEG2Simple,
            codecPicParam->m_horizontalSize,
            codecPicParam->m_verticalSize);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeMPEG2::RenderPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VABufferID       *buffers,
    int32_t          numBuffers)
{
    VAStatus           va = VA_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT mediaCtx;

    DDI_FUNCTION_ENTER();

    mediaCtx = DdiMedia_GetMediaContext(ctx);

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
            if (buf->uiNumElements == 0)
            {
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }

            VASliceParameterBufferMPEG2 *slcInfoMpeg2 =
                (VASliceParameterBufferMPEG2 *)data;
            uint32_t numSlices = buf->uiNumElements;
            DDI_CHK_RET(AllocSliceParamContext(numSlices),"AllocSliceParamContext failed!");
            DDI_CHK_RET(ParseSliceParams(mediaCtx, slcInfoMpeg2, numSlices),"ParseSliceParams failed!");
            m_ddiDecodeCtx->DecodeParams.m_numSlices += numSlices;
            m_groupIndex++;
            break;
        }
        case VAPictureParameterBufferType:
        {
            VAPictureParameterBufferMPEG2 *picParam = (VAPictureParameterBufferMPEG2 *)data;
            DDI_CHK_RET(ParsePicParams(mediaCtx, picParam),"ParsePicParams failed!");
            break;
        }
        case VAIQMatrixBufferType:
        {
            VAIQMatrixBufferMPEG2 *imxBuf =
                (VAIQMatrixBufferMPEG2 *)data;
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

VAStatus DdiDecodeMPEG2::SetDecodeParams()
{
    DDI_CHK_RET(DdiMediaDecode::SetDecodeParams(),"SetDecodeParams failed!");
    ParseNumMbsForSlice((&m_ddiDecodeCtx->DecodeParams)->m_numSlices);
    return  VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeMPEG2::AllocSliceParamContext(
    uint32_t numSlices)
{
    uint32_t baseSize = sizeof(CodecDecodeMpeg2SliceParams);

    if (m_sliceParamBufNum < (m_ddiDecodeCtx->DecodeParams.m_numSlices + numSlices))
    {
        // in order to avoid that the buffer is reallocated multi-times,
        // extra 10 slices are added.
        uint32_t extraSlices = numSlices + 10;

        m_ddiDecodeCtx->DecodeParams.m_sliceParams = realloc(m_ddiDecodeCtx->DecodeParams.m_sliceParams,
            baseSize * (m_sliceParamBufNum + extraSlices));

        if (m_ddiDecodeCtx->DecodeParams.m_sliceParams == nullptr)
        {
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }

        memset((void *)((uint8_t *)m_ddiDecodeCtx->DecodeParams.m_sliceParams + baseSize * m_sliceParamBufNum), 0, baseSize * extraSlices);
        m_sliceParamBufNum += extraSlices;
    }

    return VA_STATUS_SUCCESS;
}

void DdiDecodeMPEG2::DestroyContext(
    VADriverContextP ctx)
{
    FreeResourceBuffer();
    // explicitly call the base function to do the further clean-up
    DdiMediaDecode::DestroyContext(ctx);
}

uint8_t* DdiDecodeMPEG2::GetPicParamBuf(
    DDI_CODEC_COM_BUFFER_MGR    *bufMgr)
{
    return (uint8_t*)(&(bufMgr->Codec_Param.Codec_Param_MPEG2.PicParamMPEG2));
}

VAStatus DdiDecodeMPEG2::AllocSliceControlBuffer(
    DDI_MEDIA_BUFFER       *buf)
{
    DDI_CODEC_COM_BUFFER_MGR   *bufMgr;
    uint32_t                    availSize;
    uint32_t                    newSize;

    bufMgr     = &(m_ddiDecodeCtx->BufMgr);
    availSize  = m_sliceCtrlBufNum - bufMgr->dwNumSliceControl;
    if(availSize < buf->uiNumElements)
    {
        newSize   = sizeof(VASliceParameterBufferMPEG2) * (m_sliceCtrlBufNum - availSize + buf->uiNumElements);
        bufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2 = (VASliceParameterBufferMPEG2 *)realloc(bufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2, newSize);
        if(bufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2 == nullptr)
        {
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        MOS_ZeroMemory(bufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2 + m_sliceCtrlBufNum, sizeof(VASliceParameterBufferMPEG2) * (buf->uiNumElements - availSize));
        m_sliceCtrlBufNum = m_sliceCtrlBufNum - availSize + buf->uiNumElements;
    }
    buf->pData      = (uint8_t*)bufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2;
    buf->uiOffset   = sizeof(VASliceParameterBufferMPEG2) * bufMgr->dwNumSliceControl;

    bufMgr->dwNumSliceControl += buf->uiNumElements;

    return VA_STATUS_SUCCESS;
}

void DdiDecodeMPEG2::ContextInit(
    int32_t picWidth,
    int32_t picHeight)
{
    // call the function in base class to initialize it.
    DdiMediaDecode::ContextInit(picWidth, picHeight);

    m_ddiDecodeCtx->wMode             = CODECHAL_DECODE_MODE_MPEG2VLD;
}

VAStatus DdiDecodeMPEG2::InitResourceBuffer()
{
    VAStatus                  vaStatus = VA_STATUS_SUCCESS;
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_ddiDecodeCtx->BufMgr);

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

    // Most Mpeg2 clips are based on row.
    // As the pSliceData can be allocated on demand, the default size is based on wPicHeightInMB.
    bufMgr->m_maxNumSliceData = m_picHeightInMB;
    bufMgr->pSliceData        = (DDI_CODEC_BITSTREAM_BUFFER_INFO *)MOS_AllocAndZeroMemory(sizeof(bufMgr->pSliceData[0]) *
                                                                                   bufMgr->m_maxNumSliceData);

    if (bufMgr->pSliceData == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    bufMgr->dwNumSliceData    = 0;
    bufMgr->dwNumSliceControl = 0;

    m_sliceCtrlBufNum                          = m_picHeightInMB;
    bufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2 = (VASliceParameterBufferMPEG2 *)MOS_AllocAndZeroMemory(sizeof(VASliceParameterBufferMPEG2) * m_sliceCtrlBufNum);
    if (bufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2 == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    return VA_STATUS_SUCCESS;

finish:
    FreeResourceBuffer();
    return vaStatus;
}

void DdiDecodeMPEG2::FreeResourceBuffer()
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

    if (bufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2)
    {
        MOS_FreeMemory(bufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2);
        bufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2 = nullptr;
    }

    // free decode bitstream buffer object
    MOS_FreeMemory(bufMgr->pSliceData);
    bufMgr->pSliceData = nullptr;

    return;
}

VAStatus DdiDecodeMPEG2::CodecHalInit(
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

    m_codechalSettings->mode     = CODECHAL_DECODE_MODE_MPEG2VLD;
    m_codechalSettings->standard = CODECHAL_MPEG2;

    m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer = MOS_AllocAndZeroMemory(sizeof(CodecMpeg2IqMatrix));
    if (m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }
    m_ddiDecodeCtx->DecodeParams.m_picParams = MOS_AllocAndZeroMemory(sizeof(CodecDecodeMpeg2PicParams));
    if (m_ddiDecodeCtx->DecodeParams.m_picParams == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }

    m_sliceParamBufNum         = m_picHeightInMB;
    m_ddiDecodeCtx->DecodeParams.m_sliceParams = MOS_AllocAndZeroMemory(m_sliceParamBufNum * sizeof(CodecDecodeMpeg2SliceParams));
    if (m_ddiDecodeCtx->DecodeParams.m_sliceParams == nullptr)
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
    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_huffmanTable);
    m_ddiDecodeCtx->DecodeParams.m_huffmanTable = nullptr;
    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_sliceParams);
    m_ddiDecodeCtx->DecodeParams.m_sliceParams = nullptr;

    return vaStatus;
}

extern template class MediaDdiFactory<DdiMediaDecode, DDI_DECODE_CONFIG_ATTR>;

static bool mpeg2Registered =
    MediaDdiFactory<DdiMediaDecode, DDI_DECODE_CONFIG_ATTR>::RegisterCodec<DdiDecodeMPEG2>(DECODE_ID_MPEG2);
