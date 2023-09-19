/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file     media_ddi_encode_vp8.cpp
//! \brief    Defines class for DDI media vp8 encode.
//!

#include "media_libva.h"
#include "media_libva_encoder.h"
#include "media_libva_util.h"
#include "media_ddi_encode_vp8.h"
#include "media_ddi_encode_const.h"
#include "media_ddi_factory.h"

extern template class MediaDdiFactoryNoArg<DdiEncodeBase>;
static bool isEncodeVp8Registered =
    MediaDdiFactoryNoArg<DdiEncodeBase>::RegisterCodec<DdiEncodeVp8>(ENCODE_ID_VP8);

DdiEncodeVp8::~DdiEncodeVp8()
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

        MOS_FreeMemory(m_encodeCtx->pbsBuffer);
        m_encodeCtx->pbsBuffer = nullptr;
    }
}

VAStatus DdiEncodeVp8::ContextInitialize(CodechalSetting *codecHalSettings)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_encodeCtx->pCpDdiInterface, "nullptr m_encodeCtx->pCpDdiInterface.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(codecHalSettings, "nullptr codecHalSettings.", VA_STATUS_ERROR_INVALID_CONTEXT);

    codecHalSettings->codecFunction = m_encodeCtx->codecFunction;
    codecHalSettings->width       = m_encodeCtx->dwFrameWidth;
    codecHalSettings->height      = m_encodeCtx->dwFrameHeight;
    codecHalSettings->mode          = m_encodeCtx->wModeType;
    codecHalSettings->standard      = CODECHAL_VP8;
    VAStatus vaStatus               = VA_STATUS_SUCCESS;

    m_encodeCtx->pSeqParams = (void *)MOS_AllocAndZeroMemory(sizeof(CODEC_VP8_ENCODE_SEQUENCE_PARAMS));
    DDI_CHK_NULL(m_encodeCtx->pSeqParams, "nullptr m_encodeCtx->pSeqParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pPicParams = (void *)MOS_AllocAndZeroMemory(sizeof(CODEC_VP8_ENCODE_PIC_PARAMS));
    DDI_CHK_NULL(m_encodeCtx->pPicParams, "nullptr m_encodeCtx->pPicParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pQmatrixParams = (void *)MOS_AllocAndZeroMemory(sizeof(CODEC_VP8_ENCODE_QUANT_DATA));
    DDI_CHK_NULL(m_encodeCtx->pQmatrixParams, "nullptr m_encodeCtx->pQmatrixParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    if (m_encodeCtx->codecFunction == CODECHAL_FUNCTION_HYBRIDPAK)
    {
        m_encodeCtx->pSliceParams = (void *)MOS_AllocAndZeroMemory(sizeof(CODECHAL_VP8_HYBRIDPAK_FRAMEUPDATE));
        DDI_CHK_NULL(m_encodeCtx->pSliceParams, "nullptr m_encodeCtx->pSliceParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);
    }

    // Allocate Encode Status Report
    m_encodeCtx->pEncodeStatusReport = (void *)MOS_AllocAndZeroMemory(CODECHAL_ENCODE_STATUS_NUM * sizeof(EncodeStatusReport));
    DDI_CHK_NULL(m_encodeCtx->pEncodeStatusReport, "nullptr m_encodeCtx->pEncodeStatusReport.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Create the bit stream buffer to hold the packed headers from application
    m_encodeCtx->pbsBuffer = (PBSBuffer)MOS_AllocAndZeroMemory(sizeof(BSBuffer));
    DDI_CHK_NULL(m_encodeCtx->pbsBuffer, "nullptr m_encodeCtx->pbsBuffer.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    uint32_t bufferSize         = CODECHAL_VP8_FRAME_HEADER_SIZE;
    m_encodeCtx->pbsBuffer->pBase = (uint8_t *)MOS_AllocAndZeroMemory(bufferSize);
    DDI_CHK_NULL(m_encodeCtx->pbsBuffer->pBase, "nullptr m_encodeCtx->pbsBuffer->pBase.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pbsBuffer->BufferSize = bufferSize;

    // calculate mv offset
    uint32_t numMBs = DDI_CODEC_NUM_MACROBLOCKS_WIDTH(m_encodeCtx->dwFrameWidth) * DDI_CODEC_NUM_MACROBLOCKS_HEIGHT(m_encodeCtx->dwFrameHeight);

    if (VA_RC_CQP == m_encodeCtx->uiRCMethod)
    {
        m_mvOffset = MOS_ALIGN_CEIL(CODECHAL_VP8_MB_CODE_SIZE * sizeof(uint32_t),
                                      CODECHAL_VP8_MB_CODE_ALIGNMENT) *
                                  numMBs;
    }
    else
    {
        m_mvOffset = MOS_ALIGN_CEIL(CODECHAL_VP8_MB_CODE_SIZE * sizeof(uint32_t) + CODECHAL_VP8_MB_MV_CODE_SIZE, CODECHAL_VP8_MB_CODE_ALIGNMENT) + MOS_ALIGN_CEIL(CODECHAL_VP8_MB_CODE_SIZE * sizeof(uint32_t), CODECHAL_VP8_MB_CODE_ALIGNMENT) * numMBs;
    }

    return vaStatus;
}

VAStatus DdiEncodeVp8::RenderPicture(VADriverContextP ctx, VAContextID context, VABufferID *buffers, int32_t numBuffers)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr context", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_MEDIA_CONTEXT *mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_encodeCtx->pCodecHal, "nullptr m_encodeCtx->pCodecHal", VA_STATUS_ERROR_INVALID_CONTEXT);

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

VAStatus DdiEncodeVp8::StatusReport(
        DDI_MEDIA_BUFFER *mediaBuf,
        void             **buf)
{
    DDI_CHK_NULL(m_encodeCtx->pCpDdiInterface, "nullptr m_encodeCtx->pCpDdiInterface", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaBuf, "nullptr mediaBuf", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    m_encodeCtx->BufMgr.pCodedBufferSegment->status    = 0;

    //when this function is called, there must be a frame is ready, will wait until get the right information.
    uint32_t size         = 0;
    int32_t  index        = 0;
    uint32_t status       = 0;
    uint32_t timeOutCount = 0;
    VAStatus vaStatus     = VA_STATUS_SUCCESS;

    // Get encoded frame information from status buffer queue.
    while (VA_STATUS_SUCCESS == (vaStatus = GetSizeFromStatusReportBuffer(mediaBuf, &size, &status, &index)))
    {
        if ((index >= 0) && (size != 0))  //Get the matched encoded buffer information
        {
            uint32_t sizeOfExtStatusReportBuf          = 0;  //reset or not used

            // the first segment in the single-link list: pointer for the coded bitstream and the size
            m_encodeCtx->BufMgr.pCodedBufferSegment->buf    = DdiMediaUtil_LockBuffer(mediaBuf, MOS_LOCKFLAG_READONLY);
            m_encodeCtx->BufMgr.pCodedBufferSegment->size   = size;
            m_encodeCtx->BufMgr.pCodedBufferSegment->status = status;

            //making a segment for the StatusReport: pointer for the StatusReport and the size of it
            VACodedBufferSegment *pVACodedBufferSegmentForStatusReport;
            pVACodedBufferSegmentForStatusReport = m_encodeCtx->BufMgr.pCodedBufferSegmentForStatusReport;
            DDI_CHK_NULL(pVACodedBufferSegmentForStatusReport, "nullptr check in pVACodedBufferSegmentForStatusReport", VA_STATUS_ERROR_INVALID_CONTEXT);

            pVACodedBufferSegmentForStatusReport->size = sizeOfExtStatusReportBuf;
            pVACodedBufferSegmentForStatusReport->buf  = (void *)((char *)(m_encodeCtx->BufMgr.pCodedBufferSegment->buf) + size);
            pVACodedBufferSegmentForStatusReport->next = nullptr;

            VACodedBufferSegment *pFindTheLastEntry;
            pFindTheLastEntry = m_encodeCtx->BufMgr.pCodedBufferSegment;
            // if HDCP2 is enabled, the second segment for counter values is already there  So, move the connection as the third one
            if (m_encodeCtx->pCpDdiInterface->IsHdcp2Enabled())
                pFindTheLastEntry = (VACodedBufferSegment *)pFindTheLastEntry->next;
            // connect Status report here
            pFindTheLastEntry->next = pVACodedBufferSegmentForStatusReport;

            break;
        }

        mos_bo_wait_rendering(mediaBuf->bo);

        EncodeStatusReport* encodeStatusReport = (EncodeStatusReport*)m_encodeCtx->pEncodeStatusReport;
        encodeStatusReport->bSequential = true;  //Query the encoded frame status in sequential.

        uint16_t numStatus = 1;
        m_encodeCtx->pCodecHal->GetStatusReport(encodeStatusReport, numStatus);

        if (CODECHAL_STATUS_SUCCESSFUL == encodeStatusReport[0].CodecStatus)
        {
            // Only AverageQP is reported at this time. Populate other bits with relevant informaiton later;
            status = (encodeStatusReport[0].AverageQp & VA_CODED_BUF_STATUS_PICTURE_AVE_QP_MASK);
            // fill hdcp related buffer
            DDI_CHK_RET(m_encodeCtx->pCpDdiInterface->StatusReportForHdcp2Buffer(&m_encodeCtx->BufMgr, encodeStatusReport), "fail to get hdcp2 status report!");
            if (UpdateStatusReportBuffer(encodeStatusReport[0].bitstreamSize, status) != VA_STATUS_SUCCESS)
            {
                m_encodeCtx->BufMgr.pCodedBufferSegment->buf  = DdiMediaUtil_LockBuffer(mediaBuf, MOS_LOCKFLAG_READONLY);
                m_encodeCtx->BufMgr.pCodedBufferSegment->size = 0;
                m_encodeCtx->BufMgr.pCodedBufferSegment->status |= VA_CODED_BUF_STATUS_BAD_BITSTREAM;
                m_encodeCtx->statusReportBuf.ulUpdatePosition = (m_encodeCtx->statusReportBuf.ulUpdatePosition + 1) % DDI_ENCODE_MAX_STATUS_REPORT_BUFFER;
                break;
            }
            //Add encoded frame information into status buffer queue.
            continue;
        }
        else if (CODECHAL_STATUS_INCOMPLETE == encodeStatusReport[0].CodecStatus)
        {
            CodechalEncoderState *encoder = dynamic_cast<CodechalEncoderState *>(m_encodeCtx->pCodecHal);
            if (encoder != nullptr && encoder->m_inlineEncodeStatusUpdate)
            {
                m_encodeCtx->BufMgr.pCodedBufferSegment->buf  = DdiMediaUtil_LockBuffer(mediaBuf, MOS_LOCKFLAG_READONLY);
                m_encodeCtx->BufMgr.pCodedBufferSegment->size = 0;
                m_encodeCtx->BufMgr.pCodedBufferSegment->status |= VA_CODED_BUF_STATUS_BAD_BITSTREAM;
                m_encodeCtx->statusReportBuf.ulUpdatePosition = (m_encodeCtx->statusReportBuf.ulUpdatePosition + 1) % DDI_ENCODE_MAX_STATUS_REPORT_BUFFER;
                DDI_ASSERTMESSAGE("Something unexpected happened in HW, return error to application");
                break;
            }
            // Wait until encode PAK complete, sometimes we application detect encoded buffer object is Idle, may Enc done, but Pak not.
            uint32_t maxTimeOut                               = 100000;  //set max sleep times to 100000 = 1s, other wise return error.
            if (timeOutCount < maxTimeOut)
            {
                //sleep 10 us to wait encode complete, it won't impact the performance.
                uint32_t sleepTime                            = 10;      //sleep 10 us when encode is not complete.
                usleep(sleepTime);
                timeOutCount++;
                continue;
            }
            else
            {
                //if HW didn't response in 1s, assume there is an error in encoding process, return error to App.
                m_encodeCtx->BufMgr.pCodedBufferSegment->buf  = DdiMediaUtil_LockBuffer(mediaBuf, MOS_LOCKFLAG_READONLY);
                m_encodeCtx->BufMgr.pCodedBufferSegment->size = 0;
                m_encodeCtx->BufMgr.pCodedBufferSegment->status |= VA_CODED_BUF_STATUS_BAD_BITSTREAM;
                m_encodeCtx->statusReportBuf.ulUpdatePosition = (m_encodeCtx->statusReportBuf.ulUpdatePosition + 1) % DDI_ENCODE_MAX_STATUS_REPORT_BUFFER;
                DDI_ASSERTMESSAGE("Something unexpected happened in HW, return error to application");
                break;
            }
        }
        else if (CODECHAL_STATUS_ERROR == encodeStatusReport[0].CodecStatus)
        {
            DDI_NORMALMESSAGE("Encoding failure due to HW issue");
            m_encodeCtx->BufMgr.pCodedBufferSegment->buf  = DdiMediaUtil_LockBuffer(mediaBuf, MOS_LOCKFLAG_READONLY);
            m_encodeCtx->BufMgr.pCodedBufferSegment->size = 0;
            m_encodeCtx->BufMgr.pCodedBufferSegment->status |= VA_CODED_BUF_STATUS_BAD_BITSTREAM;
            m_encodeCtx->statusReportBuf.ulUpdatePosition = (m_encodeCtx->statusReportBuf.ulUpdatePosition + 1) % DDI_ENCODE_MAX_STATUS_REPORT_BUFFER;
            break;
        }
        else
        {
            break;
        }
    }

    if (vaStatus != VA_STATUS_SUCCESS)
    {
        return vaStatus;
    }

    *buf = m_encodeCtx->BufMgr.pCodedBufferSegment;
    return vaStatus;
}

VAStatus DdiEncodeVp8::InitCompBuffer()
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_encodeCtx->pCpDdiInterface, "nullptr m_encodeCtx->pCpDdiInterface.", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CODEC_COM_BUFFER_MGR *pBufMgr = &(m_encodeCtx->BufMgr);

    // memory allocation in the beging of the sequence for extended StatusReport and then save it from DDI_CODEC_COM_BUFFER_MGR, for example, VP8
    VACodedBufferSegment *pVACodedBufferSegmentForStatusReport = (VACodedBufferSegment *)MOS_AllocAndZeroMemory(sizeof(VACodedBufferSegment));
    if (pVACodedBufferSegmentForStatusReport == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    pBufMgr->pCodedBufferSegmentForStatusReport = pVACodedBufferSegmentForStatusReport;

    DDI_CHK_RET(DdiEncodeBase::InitCompBuffer(), "InitCompBuffer failed!");

    return VA_STATUS_SUCCESS;
}

void DdiEncodeVp8::FreeCompBuffer()
{
    DdiEncodeBase::FreeCompBuffer();

    DDI_CODEC_COM_BUFFER_MGR *pBufMgr = &(m_encodeCtx->BufMgr);
    if (pBufMgr->pCodedBufferSegmentForStatusReport != nullptr)
    {
        MOS_FreeMemory(pBufMgr->pCodedBufferSegmentForStatusReport);
        pBufMgr->pCodedBufferSegmentForStatusReport = nullptr;
    }
}

VAStatus DdiEncodeVp8::ResetAtFrameLevel()
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);

    CODEC_VP8_ENCODE_SEQUENCE_PARAMS *seqParams = (CODEC_VP8_ENCODE_SEQUENCE_PARAMS *)(m_encodeCtx->pSeqParams);
    DDI_CHK_NULL(seqParams, "nullptr seqParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    seqParams->ResetBRC = 0x0;

    // reset bsbuffer every frame
    m_encodeCtx->pbsBuffer->pCurrent    = m_encodeCtx->pbsBuffer->pBase;
    m_encodeCtx->pbsBuffer->SliceOffset = 0x0;
    m_encodeCtx->pbsBuffer->BitOffset   = 0x0;
    m_encodeCtx->pbsBuffer->BitSize     = 0x0;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp8::Qmatrix(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAQMatrixBufferVP8 *quantParams = (VAQMatrixBufferVP8 *)ptr;

    CODEC_VP8_ENCODE_QUANT_DATA *vp8QuantParams = (CODEC_VP8_ENCODE_QUANT_DATA *)(m_encodeCtx->pQmatrixParams);
    DDI_CHK_NULL(vp8QuantParams, "nullptr vp8QuantParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    MOS_ZeroMemory(vp8QuantParams, sizeof(CODEC_VP8_ENCODE_QUANT_DATA));

    for (int32_t i = 0; i < 4; i++)
    {
        vp8QuantParams->QIndex[i] = quantParams->quantization_index[i];
    }

    for (int32_t i = 0; i < 5; i++)
    {
        vp8QuantParams->QIndexDelta[i] = quantParams->quantization_index_delta[i];
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp8::ParseSeqParams(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncSequenceParameterBufferVP8 *seqParams = (VAEncSequenceParameterBufferVP8 *)ptr;

    CODEC_VP8_ENCODE_SEQUENCE_PARAMS *vp8SeqParams = (CODEC_VP8_ENCODE_SEQUENCE_PARAMS *)(m_encodeCtx->pSeqParams);
    DDI_CHK_NULL(vp8SeqParams, "nullptr vp8SeqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    MOS_ZeroMemory(vp8SeqParams, sizeof(CODEC_VP8_ENCODE_SEQUENCE_PARAMS));

    vp8SeqParams->FrameWidth       = seqParams->frame_width;
    vp8SeqParams->FrameWidthScale  = seqParams->frame_width_scale;
    vp8SeqParams->FrameHeight      = seqParams->frame_height;
    vp8SeqParams->FrameHeightScale = seqParams->frame_height_scale;

    vp8SeqParams->GopPicSize = seqParams->intra_period;

    vp8SeqParams->TargetBitRate[0] = MOS_ROUNDUP_DIVIDE(seqParams->bits_per_second, CODECHAL_ENCODE_BRC_KBPS);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp8::ParsePicParams(DDI_MEDIA_CONTEXT *mediaCtx, void *ptr)
{
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncPictureParameterBufferVP8 *picParams = (VAEncPictureParameterBufferVP8 *)ptr;

    CODEC_VP8_ENCODE_PIC_PARAMS *vp8PicParams = (CODEC_VP8_ENCODE_PIC_PARAMS *)(m_encodeCtx->pPicParams);
    DDI_CHK_NULL(vp8PicParams, "nullptr vp8PicParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    MOS_ZeroMemory(vp8PicParams, sizeof(CODEC_VP8_ENCODE_PIC_PARAMS));

    vp8PicParams->frame_type                  = picParams->pic_flags.bits.frame_type;
    vp8PicParams->version                     = picParams->pic_flags.bits.version;
    vp8PicParams->show_frame                  = picParams->pic_flags.bits.show_frame;
    vp8PicParams->color_space                 = picParams->pic_flags.bits.color_space;
    vp8PicParams->clamping_type               = picParams->pic_flags.bits.clamping_type;
    vp8PicParams->segmentation_enabled        = picParams->pic_flags.bits.segmentation_enabled;
    vp8PicParams->update_mb_segmentation_map  = picParams->pic_flags.bits.update_mb_segmentation_map;
    vp8PicParams->update_segment_feature_data = picParams->pic_flags.bits.update_segment_feature_data;
    vp8PicParams->filter_type                 = picParams->pic_flags.bits.loop_filter_type;
    vp8PicParams->loop_filter_adj_enable      = picParams->pic_flags.bits.loop_filter_adj_enable;
    vp8PicParams->CodedCoeffTokenPartition    = picParams->pic_flags.bits.num_token_partitions;
    vp8PicParams->refresh_golden_frame        = picParams->pic_flags.bits.refresh_golden_frame;
    vp8PicParams->refresh_alternate_frame     = picParams->pic_flags.bits.refresh_alternate_frame;
    vp8PicParams->copy_buffer_to_golden       = picParams->pic_flags.bits.copy_buffer_to_golden;
    vp8PicParams->copy_buffer_to_alternate    = picParams->pic_flags.bits.copy_buffer_to_alternate;
    vp8PicParams->sign_bias_golden            = picParams->pic_flags.bits.sign_bias_golden;
    vp8PicParams->sign_bias_alternate         = picParams->pic_flags.bits.sign_bias_alternate;
    vp8PicParams->refresh_entropy_probs       = picParams->pic_flags.bits.refresh_entropy_probs;
    vp8PicParams->refresh_last                = picParams->pic_flags.bits.refresh_last;
    vp8PicParams->mb_no_coeff_skip            = picParams->pic_flags.bits.mb_no_coeff_skip;
    vp8PicParams->forced_lf_adjustment        = picParams->pic_flags.bits.forced_lf_adjustment;

    if (vp8PicParams->frame_type == 0)
    {
        vp8PicParams->ref_frame_ctrl = 0;
    }
    else
    {
        vp8PicParams->ref_frame_ctrl = (!picParams->ref_flags.bits.no_ref_last) | ((!picParams->ref_flags.bits.no_ref_gf) << 1) | ((!picParams->ref_flags.bits.no_ref_arf) << 2);
    }

    // first_ref and second_ref parameters are currently passed through the reserved parameter by the application
    vp8PicParams->first_ref  = picParams->ref_flags.bits.first_ref;
    vp8PicParams->second_ref = picParams->ref_flags.bits.second_ref;
#ifdef ANDROID
    vp8PicParams->temporal_id = picParams->ref_flags.bits.temporal_id;
#endif
    // Copy list of 4 loop filter level values, delta values for ref frame and coding mode based MB-level
    for (int32_t i = 0; i < 4; i++)
    {
        vp8PicParams->loop_filter_level[i] = picParams->loop_filter_level[i];
        vp8PicParams->ref_lf_delta[i]      = picParams->ref_lf_delta[i];
        vp8PicParams->mode_lf_delta[i]     = picParams->mode_lf_delta[i];
    }

    vp8PicParams->sharpness_level = picParams->sharpness_level;
    vp8PicParams->ClampQindexHigh = picParams->clamp_qindex_high;
    vp8PicParams->ClampQindexLow  = picParams->clamp_qindex_low;

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_encodeCtx->RTtbl);

    rtTbl->pCurrentReconTarget = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParams->reconstructed_frame);
    DDI_CHK_NULL(rtTbl->pCurrentReconTarget, "nullptr rtTbl->pCurrentReconTarget", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_RET(RegisterRTSurfaces(rtTbl,rtTbl->pCurrentReconTarget), "register RT surfaces error");

    SetupCodecPicture(mediaCtx, rtTbl, &vp8PicParams->CurrReconstructedPic, picParams->reconstructed_frame, false);
    // curr orig pic
    vp8PicParams->CurrOriginalPic.FrameIdx = GetRenderTargetID(rtTbl, rtTbl->pCurrentReconTarget);
    vp8PicParams->CurrOriginalPic.PicFlags = vp8PicParams->CurrReconstructedPic.PicFlags;

    SetupCodecPicture(mediaCtx, rtTbl, &vp8PicParams->LastRefPic, picParams->ref_last_frame, true);

    SetupCodecPicture(mediaCtx, rtTbl, &vp8PicParams->GoldenRefPic, picParams->ref_gf_frame, true);

    SetupCodecPicture(mediaCtx, rtTbl, &vp8PicParams->AltRefPic, picParams->ref_arf_frame, true);

    DDI_MEDIA_BUFFER *buf = DdiMedia_GetBufferFromVABufferID(mediaCtx, picParams->coded_buf);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);
    RemoveFromStatusReportQueue(buf);
    DdiMedia_MediaBufferToMosResource(buf, &(m_encodeCtx->resBitstreamBuffer));

    return VA_STATUS_SUCCESS;
}

// Parse Misc Params
VAStatus DdiEncodeVp8::ParseMiscParams(void *ptr)
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
    case VAEncMiscParameterTypeQualityLevel:  // for target usage
    {
        ParseBufferQualityLevel((void *)miscParamBuf->data);
        break;
    }
    case VAEncMiscParameterTypeRateControl:
    {
        ParseMiscParamRC((void *)miscParamBuf->data);
        break;
    }
    case VAEncMiscParameterTypeTemporalLayerStructure:
    {
        ParseMiscParameterTemporalLayerParams((void *)miscParamBuf->data);
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

VAStatus DdiEncodeVp8::EncodeInCodecHal(uint32_t numSlices)
{
    DDI_UNUSED(numSlices);

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_encodeCtx->RTtbl);

    CODEC_VP8_ENCODE_SEQUENCE_PARAMS *seqParams = (PCODEC_VP8_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);

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

    MOS_RESOURCE mbCodeSurface;
    MOS_ZeroMemory(&mbCodeSurface, sizeof(MOS_RESOURCE));
    mbCodeSurface                    = m_encodeCtx->resMbCodeBuffer;
    encodeParams.psRawSurface        = &rawSurface;
    encodeParams.psReconSurface      = &reconSurface;
    encodeParams.presBitstreamBuffer = &bitstreamSurface;
    encodeParams.presMbCodeSurface   = &mbCodeSurface;

    // Segmentation map buffer
    encodeParams.psMbSegmentMapSurface = &m_encodeCtx->segMapBuffer;

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
        seqParams->MaxBitRate        = seqParams->TargetBitRate[0];
        seqParams->MinBitRate        = seqParams->TargetBitRate[0];
    }
    else if (VA_RC_VBR == m_encodeCtx->uiRCMethod)
    {
        seqParams->RateControlMethod = RATECONTROL_VBR;
    }
    if((m_encodeCtx->uiTargetBitRate != seqParams->TargetBitRate[0]) || (m_encodeCtx->uiMaxBitRate != seqParams-> MaxBitRate))
    {
        if(m_encodeCtx->uiTargetBitRate )
        {
            seqParams->ResetBRC = 0x1;
        }
        m_encodeCtx->uiTargetBitRate = seqParams->TargetBitRate[0];
        m_encodeCtx->uiMaxBitRate = seqParams->MaxBitRate;
    }

    encodeParams.pSeqParams   = m_encodeCtx->pSeqParams;
    encodeParams.pPicParams   = m_encodeCtx->pPicParams;
    encodeParams.pSliceParams = m_encodeCtx->pSliceParams;

    encodeParams.uiFrameRate = m_encodeCtx->uFrameRate;

    // Sequence data
    encodeParams.bNewSeq = m_encodeCtx->bNewSeq;

    // IQmatrix params
    encodeParams.bNewQmatrixData = m_encodeCtx->bNewQmatrixData;
    encodeParams.bPicQuant       = m_encodeCtx->bPicQuant;

    encodeParams.pQuantData = m_encodeCtx->pQmatrixParams;

    encodeParams.pBSBuffer = m_encodeCtx->pbsBuffer;

    MOS_STATUS status = m_encodeCtx->pCodecHal->Execute(&encodeParams);
    if (MOS_STATUS_SUCCESS != status)
    {
        DDI_ASSERTMESSAGE("DDI:Failed in Codechal!");
        return VA_STATUS_ERROR_ENCODING_ERROR;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeVp8::ParseSegMapParams(DDI_MEDIA_BUFFER *buf)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);

    MOS_ZeroMemory(&(m_encodeCtx->segMapBuffer), sizeof(MOS_SURFACE));
    m_encodeCtx->segMapBuffer.Format   = Format_Buffer_2D;
    m_encodeCtx->segMapBuffer.dwOffset = 0;
    DdiMedia_MediaBufferToMosResource(buf, &((m_encodeCtx->segMapBuffer).OsResource));
    return VA_STATUS_SUCCESS;
}

uint32_t DdiEncodeVp8::AddExtStatusReportParam(DDI_MEDIA_BUFFER *buf, uint32_t size)
{
    return 0;
}

void DdiEncodeVp8::ParseBufferQualityLevel(void *data)
{
    VAEncMiscParameterBufferQualityLevel *vaEncMiscParamQualityLevel = (VAEncMiscParameterBufferQualityLevel *)data;

    CODEC_VP8_ENCODE_SEQUENCE_PARAMS *seqParams = (CODEC_VP8_ENCODE_SEQUENCE_PARAMS *)m_encodeCtx->pSeqParams;
    seqParams->TargetUsage                      = vaEncMiscParamQualityLevel->quality_level;
}

void DdiEncodeVp8::ParseMiscParamVBV(void *data)
{
    VAEncMiscParameterHRD *vaEncMiscParamHRD = (VAEncMiscParameterHRD *)data;

    CODEC_VP8_ENCODE_SEQUENCE_PARAMS *seqParams = (CODEC_VP8_ENCODE_SEQUENCE_PARAMS *)m_encodeCtx->pSeqParams;

    seqParams->VBVBufferSizeInBit         = vaEncMiscParamHRD->buffer_size;
    seqParams->InitVBVBufferFullnessInBit = vaEncMiscParamHRD->initial_buffer_fullness;
    seqParams->RateControlMethod          = RATECONTROL_CBR;
}

void DdiEncodeVp8::ParseMiscParamFR(void *data)
{
    VAEncMiscParameterFrameRate *vaFrameRate = (VAEncMiscParameterFrameRate *)data;
    CODEC_VP8_ENCODE_SEQUENCE_PARAMS *seqParams = (PCODEC_VP8_ENCODE_SEQUENCE_PARAMS)m_encodeCtx->pSeqParams;

    uint32_t numerator = (vaFrameRate->framerate & 0xffff) * 100;
    auto denominator = (vaFrameRate->framerate >> 16)&0xfff;
    if(denominator == 0)
    {
        denominator = 1;
    }

    uint32_t tmpId = 0;
#ifdef ANDROID
    tmpId = vaFrameRate->framerate_flags.bits.temporal_id;
#endif
    seqParams->FramesPer100Sec[tmpId] = numerator/denominator;
    if(m_framesPer100Sec && m_framesPer100Sec != seqParams->FramesPer100Sec[tmpId])
    {
        seqParams->ResetBRC = 0x1;
    }
    m_framesPer100Sec = seqParams->FramesPer100Sec[tmpId];
}

void DdiEncodeVp8::ParseMiscParamRC(void *data)
{
    CODEC_VP8_ENCODE_SEQUENCE_PARAMS *seqParams = (CODEC_VP8_ENCODE_SEQUENCE_PARAMS *)(m_encodeCtx->pSeqParams);

    VAEncMiscParameterRateControl *vaEncMiscParamRC = (VAEncMiscParameterRateControl *)data;

    uint32_t tmpId = 0;
#ifdef ANDROID
    tmpId = vaEncMiscParamRC->rc_flags.bits.temporal_id;
#endif
    seqParams->MaxBitRate           = MOS_ROUNDUP_DIVIDE(vaEncMiscParamRC->bits_per_second, CODECHAL_ENCODE_BRC_KBPS);
    seqParams->TargetBitRate[tmpId] = seqParams->MaxBitRate;
    seqParams->ResetBRC             = vaEncMiscParamRC->rc_flags.bits.reset;  // adding reset here. will apply both CBR and VBR
    seqParams->MBBRC                = vaEncMiscParamRC->rc_flags.bits.mb_rate_control;

    if (VA_RC_CBR == m_encodeCtx->uiRCMethod)
    {
        seqParams->MinBitRate        = seqParams->MaxBitRate;
        seqParams->RateControlMethod = RATECONTROL_CBR;
    }
    else if (VA_RC_VBR == m_encodeCtx->uiRCMethod)
    {
        seqParams->MinBitRate           = seqParams->MaxBitRate * (2 * vaEncMiscParamRC->target_percentage - 100) / 100;
        seqParams->TargetBitRate[tmpId] = seqParams->MaxBitRate * vaEncMiscParamRC->target_percentage / 100;  // VBR target bits
        seqParams->RateControlMethod    = RATECONTROL_VBR;
    }
}

void DdiEncodeVp8::ParseMiscParamPrivate(void *data)
{
    DDI_UNUSED(data);
    // Nothing to do here
    return;
}

void DdiEncodeVp8::ParseMiscParameterTemporalLayerParams(void *data)
{
    CODEC_VP8_ENCODE_SEQUENCE_PARAMS *seqParams = (CODEC_VP8_ENCODE_SEQUENCE_PARAMS *)(m_encodeCtx->pSeqParams);

    VAEncMiscParameterTemporalLayerStructure *vaEncTempLayerStruct = (VAEncMiscParameterTemporalLayerStructure *)data;

    if (vaEncTempLayerStruct->number_of_layers > 0)
    {
        seqParams->NumTemporalLayersMinus1 = vaEncTempLayerStruct->number_of_layers - 1;
    }
    else
    {
        seqParams->NumTemporalLayersMinus1 = 0;
    }
}

void DdiEncodeVp8::SetupCodecPicture(
    DDI_MEDIA_CONTEXT                     *mediaCtx,
    DDI_CODEC_RENDER_TARGET_TABLE         *rtTbl,
    CODEC_PICTURE                         *codecHalPic,
    VASurfaceID                           surfaceID,
    bool                                  picReference)
{
    if(DDI_CODEC_INVALID_FRAME_INDEX != surfaceID)
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
            codecHalPic->PicFlags = PICTURE_SHORT_TERM_REFERENCE;  // No long term references in VP8
        }
    }
    else
    {
        codecHalPic->PicFlags = PICTURE_FRAME;
    }
}

uint32_t DdiEncodeVp8::getSequenceParameterBufferSize()
{
    return sizeof(VAEncSequenceParameterBufferVP8);
}

uint32_t DdiEncodeVp8::getPictureParameterBufferSize()
{
    return sizeof(VAEncPictureParameterBufferVP8);
}

uint32_t DdiEncodeVp8::getQMatrixBufferSize()
{
    return sizeof(VAQMatrixBufferVP8);
}
