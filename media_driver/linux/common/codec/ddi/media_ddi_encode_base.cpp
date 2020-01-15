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
//! \file     media_ddi_encode_base.cpp
//! \brief    Implements base class for DDI media encode and encode parameters parser
//!

#include "media_libva_util.h"
#include "media_libva_common.h"
#include "media_ddi_encode_base.h"

DdiEncodeBase::DdiEncodeBase()
    :DdiMediaBase()
{
    m_codechalSettings = CodechalSetting::CreateCodechalSetting();
}

VAStatus DdiEncodeBase::BeginPicture(
    VADriverContextP    ctx,
    VAContextID         context,
    VASurfaceID         renderTarget)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "Null mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_MEDIA_SURFACE *curRT = (DDI_MEDIA_SURFACE *)DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, renderTarget);
    DDI_CHK_NULL(curRT, "Null curRT", VA_STATUS_ERROR_INVALID_SURFACE);

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_encodeCtx->RTtbl);
    // raw input frame
    rtTbl->pCurrentRT = curRT;
    if (m_encodeCtx->codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC)
    {
        DDI_CHK_RET(RegisterRTSurfaces(rtTbl, curRT),"RegisterRTSurfaces failed!");
    }
    // reset some the parameters in picture level
    ResetAtFrameLevel();

    DDI_FUNCTION_EXIT(VA_STATUS_SUCCESS);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeBase::EndPicture(
    VADriverContextP    ctx,
    VAContextID         context)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "Null mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus status = EncodeInCodecHal(m_encodeCtx->dwNumSlices);
    ClearPicParams();
    if (VA_STATUS_SUCCESS != status)
    {
        DDI_ASSERTMESSAGE("DDI:DdiEncode_EncodeInCodecHal return failure.");
        return VA_STATUS_ERROR_ENCODING_ERROR;
    }

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_encodeCtx->RTtbl);
    rtTbl->pCurrentRT                    = nullptr;
    m_encodeCtx->bNewSeq                 = false;

    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_encodeCtx->BufMgr);
    bufMgr->dwNumSliceData           = 0;
    bufMgr->dwEncodeNumSliceControl  = 0;

    DDI_FUNCTION_EXIT(VA_STATUS_SUCCESS);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeBase::AddToStatusReportQueue(void *codedBuf)
{
    DDI_CHK_NULL(m_encodeCtx->pCpDdiInterface, "Null m_encodeCtx->pCpDdiInterface", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(codedBuf, "Null codedBuf", VA_STATUS_ERROR_INVALID_BUFFER);

    int32_t idx                                       = m_encodeCtx->statusReportBuf.ulHeadPosition;
    m_encodeCtx->statusReportBuf.infos[idx].pCodedBuf = codedBuf;
    m_encodeCtx->statusReportBuf.infos[idx].uiSize    = 0;
    m_encodeCtx->statusReportBuf.infos[idx].uiStatus  = 0;
    MOS_STATUS status = m_encodeCtx->pCpDdiInterface->StoreCounterToStatusReport(&m_encodeCtx->statusReportBuf.infos[idx]);
    if (status != MOS_STATUS_SUCCESS)
    {
        return VA_STATUS_ERROR_INVALID_BUFFER;
    }
    m_encodeCtx->statusReportBuf.ulHeadPosition = (m_encodeCtx->statusReportBuf.ulHeadPosition + 1) % DDI_ENCODE_MAX_STATUS_REPORT_BUFFER;

    return VA_STATUS_SUCCESS;

}

VAStatus DdiEncodeBase::InitCompBuffer()
{
    DDI_CHK_NULL(m_encodeCtx, "Null m_encodeCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_encodeCtx->pCpDdiInterface, "Null m_encodeCtx->pCpDdiInterface.", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_encodeCtx->BufMgr);
    PDDI_MEDIA_CONTEXT      mediaCtx = m_encodeCtx->pMediaCtx;

    bufMgr->dwEncodeNumSliceControl = 0;

    // create status reporting structure
    bufMgr->pCodedBufferSegment = (VACodedBufferSegment *)MOS_AllocAndZeroMemory(sizeof(VACodedBufferSegment));
    if (bufMgr->pCodedBufferSegment == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    bufMgr->pCodedBufferSegment->next = nullptr;

    DDI_CHK_RET(m_encodeCtx->pCpDdiInterface->InitHdcp2Buffer(bufMgr), "fail to init hdcp2 buffer!");

    return VA_STATUS_SUCCESS;
}

void DdiEncodeBase::FreeCompBuffer()
{
    DDI_CHK_NULL(m_encodeCtx, "Null m_encodeCtx.", );
    DDI_CHK_NULL(m_encodeCtx->pCpDdiInterface, "Null m_encodeCtx->pCpDdiInterface.", );
    DDI_CHK_NULL(m_encodeCtx->pMediaCtx, "Null m_encodeCtx->pMediaCtx.", );

    PDDI_MEDIA_CONTEXT mediaCtx = m_encodeCtx->pMediaCtx;
    DDI_CODEC_COM_BUFFER_MGR *bufMgr   = &(m_encodeCtx->BufMgr);
    // free  encode bitstream buffer object
    MOS_FreeMemory(bufMgr->pSliceData);
    bufMgr->pSliceData = nullptr;

    m_encodeCtx->pCpDdiInterface->FreeHdcp2Buffer(bufMgr);

    // free status report struct
    MOS_FreeMemory(bufMgr->pCodedBufferSegment);
    bufMgr->pCodedBufferSegment = nullptr;
}

VAStatus DdiEncodeBase::StatusReport(
    DDI_MEDIA_BUFFER    *mediaBuf,
    void                **buf)
{
    DDI_CHK_NULL(m_encodeCtx->pCpDdiInterface, "Null m_encodeCtx->pCpDdiInterface", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaBuf, "Null mediaBuf", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf, "Null buf", VA_STATUS_ERROR_INVALID_CONTEXT);

    m_encodeCtx->BufMgr.pCodedBufferSegment->status    = 0;

    //when this function is called, there must be a frame is ready, will wait until get the right information.
    uint32_t size         = 0;
    int32_t  index        = 0;
    uint32_t status       = 0;
    uint32_t timeOutCount = 0;
    VAStatus eStatus      = VA_STATUS_SUCCESS;

    // Get encoded frame information from status buffer queue.
    while (VA_STATUS_SUCCESS == (eStatus = GetSizeFromStatusReportBuffer(mediaBuf, &size, &status, &index)))
    {
        if ((index >= 0) && (size != 0))  //Get the matched encoded buffer information
        {
            // the first segment in the single-link list: pointer for the coded bitstream and the size
            m_encodeCtx->BufMgr.pCodedBufferSegment->buf    = DdiMediaUtil_LockBuffer(mediaBuf, MOS_LOCKFLAG_READONLY);
            m_encodeCtx->BufMgr.pCodedBufferSegment->size   = size;
            m_encodeCtx->BufMgr.pCodedBufferSegment->status = status;
            break;
        }
        else if ((index >= 0) && (size == 0) && (status & VA_CODED_BUF_STATUS_BAD_BITSTREAM))
        {
            m_encodeCtx->BufMgr.pCodedBufferSegment->buf    = DdiMediaUtil_LockBuffer(mediaBuf, MOS_LOCKFLAG_READONLY);
            m_encodeCtx->BufMgr.pCodedBufferSegment->size   = size;
            m_encodeCtx->BufMgr.pCodedBufferSegment->status = status;
            break;
        }

        mos_bo_wait_rendering(mediaBuf->bo);

        EncodeStatusReport *encodeStatusReport = (EncodeStatusReport*)m_encodeCtx->pEncodeStatusReport;
        encodeStatusReport->bSequential = true;  //Query the encoded frame status in sequential.

        uint16_t numStatus = 1;
        MOS_STATUS mosStatus = MOS_STATUS_SUCCESS;
        mosStatus = m_encodeCtx->pCodecHal->GetStatusReport(encodeStatusReport, numStatus);
        if (MOS_STATUS_NOT_ENOUGH_BUFFER == mosStatus)
        {
            return VA_STATUS_ERROR_NOT_ENOUGH_BUFFER;
        } else if (MOS_STATUS_SUCCESS != mosStatus)
        {
            return VA_STATUS_ERROR_ENCODING_ERROR;
        }

        if (CODECHAL_STATUS_SUCCESSFUL == encodeStatusReport[0].CodecStatus)
        {
            // Only AverageQP is reported at this time. Populate other bits with relevant informaiton later;
            status = (encodeStatusReport[0].AverageQp & VA_CODED_BUF_STATUS_PICTURE_AVE_QP_MASK);
            if(m_encodeCtx->wModeType == CODECHAL_ENCODE_MODE_AVC)
            {
                CodecEncodeAvcFeiPicParams *feiPicParams = (CodecEncodeAvcFeiPicParams*) m_encodeCtx->pFeiPicParams;
                if ((feiPicParams != NULL) && (feiPicParams->dwMaxFrameSize != 0))
                {
                    // The reported the pass number should be multi-pass PAK caused by the MaxFrameSize.
                    // if the suggestedQpYDelta is 0, it means that MaxFrameSize doesn't trigger multi-pass PAK.
                    // The MaxMbSize triggers multi-pass PAK, the cases should be ignored when reporting the PAK pass.
                    if ((encodeStatusReport[0].SuggestedQpYDelta == 0) && (encodeStatusReport[0].NumberPasses != 1))
                    {
                        encodeStatusReport[0].NumberPasses = 1;
                    }
                }
            }
            status = status | ((encodeStatusReport[0].NumberPasses) & 0xf)<<24;
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

            // Report extra status for completed coded buffer
            eStatus = ReportExtraStatus(encodeStatusReport, m_encodeCtx->BufMgr.pCodedBufferSegment);
            if (VA_STATUS_SUCCESS != eStatus)
            {
                break;
            }

            //Add encoded frame information into status buffer queue.
            continue;
        }
        else if (CODECHAL_STATUS_INCOMPLETE == encodeStatusReport[0].CodecStatus)
        {
            bool inlineEncodeStatusUpdate;
            CodechalEncoderState *encoder = dynamic_cast<CodechalEncoderState *>(m_encodeCtx->pCodecHal);
            DDI_CHK_NULL(encoder, "Null codechal encoder", VA_STATUS_ERROR_INVALID_CONTEXT);
            inlineEncodeStatusUpdate = encoder->m_inlineEncodeStatusUpdate;

            if (inlineEncodeStatusUpdate)
            {
                m_encodeCtx->BufMgr.pCodedBufferSegment->buf  = DdiMediaUtil_LockBuffer(mediaBuf, MOS_LOCKFLAG_READONLY);
                m_encodeCtx->BufMgr.pCodedBufferSegment->size = 0;
                m_encodeCtx->BufMgr.pCodedBufferSegment->status |= VA_CODED_BUF_STATUS_BAD_BITSTREAM;
                UpdateStatusReportBuffer(encodeStatusReport[0].bitstreamSize, m_encodeCtx->BufMgr.pCodedBufferSegment->status);
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
                UpdateStatusReportBuffer(encodeStatusReport[0].bitstreamSize, m_encodeCtx->BufMgr.pCodedBufferSegment->status);
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
            UpdateStatusReportBuffer(encodeStatusReport[0].bitstreamSize, m_encodeCtx->BufMgr.pCodedBufferSegment->status);
            break;
        }
        else
        {
            break;
        }
    }

    if (eStatus != VA_STATUS_SUCCESS)
    {
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    *buf = m_encodeCtx->BufMgr.pCodedBufferSegment;
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeBase::EncStatusReport(
    DDI_MEDIA_BUFFER    *mediaBuf,
    void                **buf)
{
    DDI_CHK_NULL(mediaBuf, "Null mediaBuf", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf, "Null buf", VA_STATUS_ERROR_INVALID_CONTEXT);

    EncodeStatusReport* encodeStatusReport = (EncodeStatusReport*)m_encodeCtx->pEncodeStatusReport;
    uint16_t numStatus    = 1;
    uint32_t maxTimeOut   = 500000;  //set max sleep times to 500000 = 5s, other wise return error.
    uint32_t sleepTime    = 10;  //sleep 10 us when encode is not complete.
    uint32_t timeOutCount = 0;

    //when this function is called, there must be a frame is ready, will wait until get the right information.
    while (1)
    {
        encodeStatusReport->bSequential = true;  //Query the encoded frame status in sequential.
        m_encodeCtx->pCodecHal->GetStatusReport(encodeStatusReport, numStatus);

        if (CODECHAL_STATUS_SUCCESSFUL == encodeStatusReport[0].CodecStatus)
        {
            // Only AverageQP is reported at this time. Populate other bits with relevant informaiton later;
            uint32_t status = (encodeStatusReport[0].AverageQp & VA_CODED_BUF_STATUS_PICTURE_AVE_QP_MASK);
            status = status | ((encodeStatusReport[0].NumberPasses & 0xf)<<24);
            if (UpdateEncStatusReportBuffer(status) != VA_STATUS_SUCCESS)
            {
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }
            break;
        }
        else if (CODECHAL_STATUS_INCOMPLETE == encodeStatusReport[0].CodecStatus)
        {
            // Wait until encode PAK complete, sometimes we application detect encoded buffer object is Idle, may Enc done, but Pak not.
            if (timeOutCount < maxTimeOut)
            {
                //sleep 10 us to wait encode complete, it won't impact the performance.
                usleep(sleepTime);
                timeOutCount++;
                continue;
            }
            else
            {
                //if HW didn't response in 5s, assume there is an error in encoding process, return error to App.
                return VA_STATUS_ERROR_ENCODING_ERROR;
            }
        }
        else
        {
            // App will call twice StatusReport() for 1 frame, for the second call, just return.
            break;
        }
    }

    if (mediaBuf->bo)
    {
        *buf = DdiMediaUtil_LockBuffer(mediaBuf, MOS_LOCKFLAG_READONLY);
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeBase::PreEncStatusReport(
    DDI_MEDIA_BUFFER    *mediaBuf,
    void                **buf)
{
    DDI_CHK_NULL(mediaBuf, "Null mediaBuf", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf, "Null buf", VA_STATUS_ERROR_INVALID_CONTEXT);

    EncodeStatusReport* encodeStatusReport = (EncodeStatusReport*)m_encodeCtx->pEncodeStatusReport;
    uint16_t numStatus    = 1;
    uint32_t maxTimeOut   = 500000;  //set max sleep times to 500000 = 5s, other wise return error.
    uint32_t sleepTime    = 10;  //sleep 10 us when encode is not complete.
    uint32_t timeOutCount = 0;

    //when this function is called, there must be a frame is ready, will wait until get the right information.
    while (1)
    {
        encodeStatusReport->bSequential = true;  //Query the encoded frame status in sequential.
        m_encodeCtx->pCodecHal->GetStatusReport(encodeStatusReport, numStatus);

        if (CODECHAL_STATUS_SUCCESSFUL == encodeStatusReport[0].CodecStatus)
        {
            // Only AverageQP is reported at this time. Populate other bits with relevant informaiton later;
            uint32_t status = (encodeStatusReport[0].AverageQp & VA_CODED_BUF_STATUS_PICTURE_AVE_QP_MASK);
            status = status | ((encodeStatusReport[0].NumberPasses & 0xf)<<24);
            if (UpdatePreEncStatusReportBuffer(status) != VA_STATUS_SUCCESS)
            {
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }
            break;
        }
        else if (CODECHAL_STATUS_INCOMPLETE == encodeStatusReport[0].CodecStatus)
        {
            // Wait until encode PAK complete, sometimes we application detect encoded buffer object is Idle, may Enc done, but Pak not.
            if (timeOutCount < maxTimeOut)
            {
                //sleep 10 us to wait encode complete, it won't impact the performance.
                usleep(sleepTime);
                timeOutCount++;
                continue;
            }
            else
            {
                //if HW didn't response in 5s, assume there is an error in encoding process, return error to App.
                return VA_STATUS_ERROR_ENCODING_ERROR;
            }
        }
        else
        {
            // App will call twice PreEncStatusReport() for 1 frame, for the second call, just return.
            break;
        }
    }

    if (mediaBuf->bo)
    {
        *buf = DdiMediaUtil_LockBuffer(mediaBuf, MOS_LOCKFLAG_READONLY);
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeBase::RemoveFromStatusReportQueue(DDI_MEDIA_BUFFER *buf)
{
    VAStatus eStatus = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(m_encodeCtx, "Null m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf, "Null buf", VA_STATUS_ERROR_INVALID_CONTEXT);

    int32_t    index  = 0;
    uint32_t   size   = 0;
    uint32_t   status = 0;

    eStatus = GetSizeFromStatusReportBuffer(buf, &size, &status, &index);
    if (VA_STATUS_SUCCESS != eStatus)
    {
        return eStatus;
    }

    if (index >= 0)
    {
        m_encodeCtx->statusReportBuf.infos[index].pCodedBuf = nullptr;
        m_encodeCtx->statusReportBuf.infos[index].uiSize    = 0;
    }
    return eStatus;
}

VAStatus DdiEncodeBase::RemoveFromEncStatusReportQueue(
    DDI_MEDIA_BUFFER                  *buf,
    DDI_ENCODE_FEI_ENC_BUFFER_TYPE    typeIdx)
{
    VAStatus eStatus = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(m_encodeCtx, "Null m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf, "Null buf", VA_STATUS_ERROR_INVALID_CONTEXT);

    if ((typeIdx < 0) || (typeIdx >= FEI_ENC_BUFFER_TYPE_MAX))
    {
        DDI_ASSERTMESSAGE("ENC RemoveFromEncStatusReportBuffer, gets invalid buffer type index! .");
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    int32_t  index  = 0;
    uint32_t status = 0;

    eStatus = GetIndexFromEncStatusReportBuffer(buf, typeIdx, &status, &index);
    if (VA_STATUS_SUCCESS != eStatus)
    {
        return eStatus;
    }

    if (index >= 0)
    {
        m_encodeCtx->statusReportBuf.encInfos[index].pEncBuf[typeIdx] = nullptr;
    }

    return eStatus;
}

VAStatus DdiEncodeBase::RemoveFromPreEncStatusReportQueue(
    DDI_MEDIA_BUFFER                  *buf,
    DDI_ENCODE_PRE_ENC_BUFFER_TYPE    typeIdx)
{
    VAStatus eStatus = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(m_encodeCtx, "Null m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf, "Null buf", VA_STATUS_ERROR_INVALID_CONTEXT);

    if ((typeIdx < 0) || (typeIdx >= PRE_ENC_BUFFER_TYPE_MAX))
    {
        DDI_ASSERTMESSAGE("PRE ENC RemoveFromEncStatusReportBuffer, gets invalid buffer type index! .");
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    int32_t  index  = 0;
    uint32_t status = 0;

    eStatus = GetIndexFromPreEncStatusReportBuffer(buf, typeIdx, &status, &index);
    if (VA_STATUS_SUCCESS != eStatus)
    {
        return eStatus;
    }

    bool bufferIsUpdated = m_encodeCtx->statusReportBuf.ulUpdatePosition < m_encodeCtx->statusReportBuf.ulHeadPosition ?
                            (index < m_encodeCtx->statusReportBuf.ulUpdatePosition)
                            : (m_encodeCtx->statusReportBuf.ulUpdatePosition == m_encodeCtx->statusReportBuf.ulHeadPosition ?
                                true
                                : ((index < m_encodeCtx->statusReportBuf.ulUpdatePosition)
                                  &&(index > m_encodeCtx->statusReportBuf.ulHeadPosition)));

    // Remove updated status report buffer
    if (index >= 0 && bufferIsUpdated)
    {
        m_encodeCtx->statusReportBuf.preencInfos[index].pPreEncBuf[typeIdx] = nullptr;
        m_encodeCtx->statusReportBuf.preencInfos[index].uiBuffers = 0;
    }

    return eStatus;
}

VAStatus DdiEncodeBase::GetSizeFromStatusReportBuffer(
    DDI_MEDIA_BUFFER    *buf,
    uint32_t            *size,
    uint32_t            *status,
    int32_t             *index)
{
    VAStatus eStatus = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(m_encodeCtx, "Null m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_encodeCtx->pCpDdiInterface, "Null m_encodeCtx->pCpDdiInterface", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf, "Null buf", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(size, "Null size", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(status, "Null status", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(index, "Null index", VA_STATUS_ERROR_INVALID_CONTEXT);

    int32_t i = 0;
    for (i = 0; i < DDI_ENCODE_MAX_STATUS_REPORT_BUFFER; i++)
    {
        // check if the buffer has already been added to status report queue
        if (m_encodeCtx->statusReportBuf.infos[i].pCodedBuf == (void *)buf->bo)
        {
            *size   = m_encodeCtx->statusReportBuf.infos[i].uiSize;
            *status = m_encodeCtx->statusReportBuf.infos[i].uiStatus;

            break;
        }
    }

    if (i >= DDI_ENCODE_MAX_STATUS_REPORT_BUFFER)
    {
        // no matching buffer has been found
        *size   = 0;
        i       = DDI_CODEC_INVALID_BUFFER_INDEX;
        eStatus = MOS_STATUS_INVALID_HANDLE;
    }

    *index = i;

    return eStatus;
}

VAStatus DdiEncodeBase::GetIndexFromEncStatusReportBuffer(
    DDI_MEDIA_BUFFER                  *buf,
    DDI_ENCODE_FEI_ENC_BUFFER_TYPE    typeIdx,
    uint32_t                          *status,
    int32_t                           *index)
{
    VAStatus eStatus = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(m_encodeCtx, "Null m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf, "Null buf", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(status, "Null status", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(index, "Null index", VA_STATUS_ERROR_INVALID_CONTEXT);

    if ((typeIdx < 0) || (typeIdx >= FEI_ENC_BUFFER_TYPE_MAX))
    {
        DDI_ASSERTMESSAGE("ENC GetIndexFromEncStatusReportBuffer, gets invalid buffer type index! .");
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    int32_t i = 0;
    for (i = 0; i < DDI_ENCODE_MAX_STATUS_REPORT_BUFFER; i++)
    {
        // check if the buffer has already been added to status report queue
        if (m_encodeCtx->statusReportBuf.encInfos[i].pEncBuf[typeIdx] == (void *)buf->bo)
        {
            *status = m_encodeCtx->statusReportBuf.encInfos[i].uiStatus;
            break;
        }
    }

    if (i >= DDI_ENCODE_MAX_STATUS_REPORT_BUFFER)
    {
        // no matching buffer has been found
        i       = DDI_CODEC_INVALID_BUFFER_INDEX;
        eStatus = VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    *index = i;

    return eStatus;
}

VAStatus DdiEncodeBase::GetIndexFromPreEncStatusReportBuffer(
    DDI_MEDIA_BUFFER                  *buf,
    DDI_ENCODE_PRE_ENC_BUFFER_TYPE    typeIdx,
    uint32_t                          *status,
    int32_t                           *index)
{
    VAStatus eStatus = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(m_encodeCtx, "Null m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf, "Null buf", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(status, "Null status", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(index, "Null index", VA_STATUS_ERROR_INVALID_CONTEXT);

    if ((typeIdx < 0) || (typeIdx >= PRE_ENC_BUFFER_TYPE_MAX))
    {
        DDI_ASSERTMESSAGE("PRE ENC GetIndexFromPreEncStatusReportBuffer, gets invalid buffer type index! .");
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    int32_t i = 0;
    for (i = 0; i < DDI_ENCODE_MAX_STATUS_REPORT_BUFFER; i++)
    {
        // check if the buffer has already been added to status report queue
        if (m_encodeCtx->statusReportBuf.preencInfos[i].pPreEncBuf[typeIdx] == (void *)buf->bo)
        {
            *status = m_encodeCtx->statusReportBuf.preencInfos[i].uiStatus;
            break;
        }
    }

    if (i >= DDI_ENCODE_MAX_STATUS_REPORT_BUFFER)
    {
        // no matching buffer has been found
        i       = DDI_CODEC_INVALID_BUFFER_INDEX;
        eStatus = VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    *index = i;

    return eStatus;
}

bool DdiEncodeBase::CodedBufferExistInStatusReport(DDI_MEDIA_BUFFER *buf)
{
    if (nullptr == m_encodeCtx || nullptr == buf)
    {
        return false;
    }

    for (int32_t i = 0; i < DDI_ENCODE_MAX_STATUS_REPORT_BUFFER; i++)
    {
        if (m_encodeCtx->statusReportBuf.infos[i].pCodedBuf == (void *)buf->bo)
        {
            return true;
        }
    }
    return false;
}

bool DdiEncodeBase::EncBufferExistInStatusReport(
    DDI_MEDIA_BUFFER                  *buf,
    DDI_ENCODE_FEI_ENC_BUFFER_TYPE    typeIdx)
{
    if (nullptr == m_encodeCtx || nullptr == buf)
    {
        return false;
    }

    if ((typeIdx < 0) || (typeIdx >= FEI_ENC_BUFFER_TYPE_MAX))
    {
        DDI_ASSERTMESSAGE("ENC EncBufferExistInStatusReport, gets invalid buffer type index! .");
        return false;
    }

    for (int32_t i = 0; i < DDI_ENCODE_MAX_STATUS_REPORT_BUFFER; i++)
    {
        if (m_encodeCtx->statusReportBuf.encInfos[i].pEncBuf[typeIdx] == (void *)buf->bo)
        {
            return true;
        }
    }
    return false;
}

bool DdiEncodeBase::PreEncBufferExistInStatusReport(
    DDI_MEDIA_BUFFER                  *buf,
    DDI_ENCODE_PRE_ENC_BUFFER_TYPE    typeIdx)
{
    if (nullptr == m_encodeCtx || nullptr == buf)
    {
        return false;
    }

    if ((typeIdx < 0) || (typeIdx >= PRE_ENC_BUFFER_TYPE_MAX))
    {
        DDI_ASSERTMESSAGE("ENC EncBufferExistInStatusReport, gets invalid buffer type index! .");
        return false;
    }

    for (int32_t i = 0; i < DDI_ENCODE_MAX_STATUS_REPORT_BUFFER; i++)
    {
        if (m_encodeCtx->statusReportBuf.preencInfos[i].pPreEncBuf[typeIdx] == (void *)buf->bo)
        {
            return true;
        }
    }
    return false;
}

uint8_t DdiEncodeBase::VARC2HalRC(uint32_t vaRC)
{
    if ((VA_RC_VBR == vaRC) || ((VA_RC_VBR | VA_RC_MB) == vaRC))
    {
        return (uint8_t)RATECONTROL_VBR;
    }
    else if (VA_RC_CQP == vaRC)
    {
        return (uint8_t)RATECONTROL_CQP;
    }
    else if (VA_RC_ICQ == vaRC)
    {
        return (uint8_t)RATECONTROL_ICQ;
    }
    else if (VA_RC_VCM == vaRC)
    {
        return (uint8_t)RATECONTROL_VCM;
    }
    else if (VA_RC_QVBR == vaRC)
    {
        return (uint8_t)RATECONTROL_QVBR;
    }
    else if (VA_RC_AVBR == vaRC)
    {
        return (uint8_t)RATECONTROL_AVBR;
    }
    else  // VA_RC_CBR or VA_RC_CBR|VA_RC_MB
    {
        return (uint8_t)RATECONTROL_CBR;
    }
}

VAStatus DdiEncodeBase::UpdateStatusReportBuffer(
    uint32_t    size,
    uint32_t    status)
{
    VAStatus eStatus = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(m_encodeCtx, "Null m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    int32_t i = m_encodeCtx->statusReportBuf.ulUpdatePosition;
    if (m_encodeCtx->statusReportBuf.infos[i].pCodedBuf != nullptr &&
        m_encodeCtx->statusReportBuf.infos[i].uiSize == 0)
    {
        m_encodeCtx->statusReportBuf.infos[i].uiSize   = size;
        m_encodeCtx->statusReportBuf.infos[i].uiStatus = status;
        m_encodeCtx->statusReportBuf.ulUpdatePosition  = (m_encodeCtx->statusReportBuf.ulUpdatePosition + 1) % DDI_ENCODE_MAX_STATUS_REPORT_BUFFER;
    }
    else
    {
        DDI_ASSERTMESSAGE("DDI: Buffer is not enough in UpdateStatusReportBuffer! .");
        eStatus = VA_STATUS_ERROR_OPERATION_FAILED;
    }

    return eStatus;
}

VAStatus DdiEncodeBase::UpdateEncStatusReportBuffer(uint32_t status)
{
    VAStatus  eStatus                         = VA_STATUS_SUCCESS;
    bool      distortionEnable               = false;
    bool      mbCodeMvOrCTBCmdCuRecordEnable = false;

    DDI_CHK_NULL(m_encodeCtx, "Null m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_encodeCtx->pFeiPicParams, "Null m_encodeCtx->pFeiPicParams", VA_STATUS_ERROR_INVALID_CONTEXT);

    if(m_encodeCtx->wModeType == CODECHAL_ENCODE_MODE_AVC)
    {
        distortionEnable               = ((CodecEncodeAvcFeiPicParams *)(m_encodeCtx->pFeiPicParams))->DistortionEnable;
        mbCodeMvOrCTBCmdCuRecordEnable = ((CodecEncodeAvcFeiPicParams *)(m_encodeCtx->pFeiPicParams))->MbCodeMvEnable;
    }
    else if(m_encodeCtx->wModeType == CODECHAL_ENCODE_MODE_HEVC)
    {
        distortionEnable               = ((CodecEncodeHevcFeiPicParams *)(m_encodeCtx->pFeiPicParams))->bDistortionEnable;
        mbCodeMvOrCTBCmdCuRecordEnable = ((CodecEncodeHevcFeiPicParams *)(m_encodeCtx->pFeiPicParams))->bCTBCmdCuRecordEnable;
    }

    int32_t i = m_encodeCtx->statusReportBuf.ulUpdatePosition;
    if (((m_encodeCtx->statusReportBuf.encInfos[i].pEncBuf[0] != nullptr) && mbCodeMvOrCTBCmdCuRecordEnable) ||
        ((m_encodeCtx->statusReportBuf.encInfos[i].pEncBuf[1] != nullptr) && mbCodeMvOrCTBCmdCuRecordEnable) ||
        ((m_encodeCtx->statusReportBuf.encInfos[i].pEncBuf[2] != nullptr) && distortionEnable))
    {
        m_encodeCtx->statusReportBuf.encInfos[i].uiStatus = status;
        m_encodeCtx->statusReportBuf.ulUpdatePosition     = (m_encodeCtx->statusReportBuf.ulUpdatePosition + 1) % DDI_ENCODE_MAX_STATUS_REPORT_BUFFER;
    }
    else
    {
        DDI_ASSERTMESSAGE("Buffer is not enough in UpdateEncStatusReportBuffer! .");
        eStatus = VA_STATUS_ERROR_OPERATION_FAILED;
    }

    if ((i + 1) == DDI_ENCODE_MAX_STATUS_REPORT_BUFFER)
    {
        for (int32_t cnt = 0; cnt < DDI_ENCODE_MAX_STATUS_REPORT_BUFFER; cnt++)
        {
            m_encodeCtx->statusReportBuf.encInfos[cnt].uiBuffers = 0;
        }
    }

    return eStatus;
}

VAStatus DdiEncodeBase::UpdatePreEncStatusReportBuffer(uint32_t status)
{
    bool                    toUpdateStatistics;
    VAStatus                eStatus = VA_STATUS_SUCCESS;
    FeiPreEncParams         *preEncParams;

    DDI_CHK_NULL(m_encodeCtx, "Null m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    preEncParams = (FeiPreEncParams*)(m_encodeCtx->pPreEncParams);
    DDI_CHK_NULL(preEncParams, "Null preEncParams", VA_STATUS_ERROR_INVALID_CONTEXT);

    int32_t i = m_encodeCtx->statusReportBuf.ulUpdatePosition;
    toUpdateStatistics = (!preEncParams->bDisableStatisticsOutput) &&
                          ((!preEncParams->bInterlaced) ? (m_encodeCtx->statusReportBuf.preencInfos[i].pPreEncBuf[1] != nullptr)
                                                         : ((m_encodeCtx->statusReportBuf.preencInfos[i].pPreEncBuf[1] != nullptr) &&
                                                               (m_encodeCtx->statusReportBuf.preencInfos[i].pPreEncBuf[2] != nullptr)));
    if (((m_encodeCtx->statusReportBuf.preencInfos[i].pPreEncBuf[0] != nullptr) && (!preEncParams->bDisableMVOutput)) || toUpdateStatistics)
    {
        m_encodeCtx->statusReportBuf.preencInfos[i].uiStatus = status;
        m_encodeCtx->statusReportBuf.ulUpdatePosition        = (m_encodeCtx->statusReportBuf.ulUpdatePosition + 1) % DDI_ENCODE_MAX_STATUS_REPORT_BUFFER;
    }
    else
    {
        DDI_ASSERTMESSAGE("Buffer is not enough in UpdatePreEncStatusReportBuffer! .");
        eStatus = VA_STATUS_ERROR_OPERATION_FAILED;
    }

    if ((i + 1) == DDI_ENCODE_MAX_STATUS_REPORT_BUFFER)
    {
        for (int32_t cnt = 0; cnt < DDI_ENCODE_MAX_STATUS_REPORT_BUFFER; cnt++)
        {
            m_encodeCtx->statusReportBuf.preencInfos[cnt].uiBuffers = 0;
        }
    }

    return eStatus;
}

VAStatus DdiEncodeBase::CreateBuffer(
    VADriverContextP    ctx,
    VABufferType        type,
    uint32_t            size,
    uint32_t            elementsNum,
    void                *data,
    VABufferID          *bufId)
{
    VAStatus va = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(m_encodeCtx, "Null m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    // for VAEncSliceParameterBufferType buffer, VAEncQPBufferType buffer and 
    // VAEncMacroblockMapBufferType buffer, the number of elements can be greater than 1
    if ((type != VAEncSliceParameterBufferType) &&
        (type != VAEncQPBufferType) &&
        (type != VAEncMacroblockMapBufferType) &&
        (elementsNum > 1))
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (0 == size)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    // for coded buffer, does not support to upload some data directly
    if ((VAEncCodedBufferType == type) && (nullptr != data))
    {
        DDI_ASSERTMESSAGE("DDI:can not initialize the coded buffer!");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    // for FEI ENC output buffers
    if ((m_encodeCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC) && (nullptr != data) &&
        ((VAEncFEIMVBufferType == type) || (VAEncFEIMBCodeBufferType == type) || (VAEncFEIDistortionBufferType == type) ||  (VAEncFEICURecordBufferType == type)))
    {
        DDI_ASSERTMESSAGE("DDI:can not initialize the MVs, CURecord, MBcode and Distortion buffer for FEI ENC only!");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if ((m_encodeCtx->codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC) && (nullptr != data) &&
        ((VAStatsMVBufferType == type) || (VAStatsStatisticsBufferType == type) || (VAStatsStatisticsBottomFieldBufferType == type)))
    {
        DDI_ASSERTMESSAGE("DDI:can not initialize the MV and Statistics buffer!");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    DDI_MEDIA_BUFFER *buf = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
    if (buf == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);

    buf->pMediaCtx     = mediaCtx;
    buf->uiNumElements = elementsNum;
    buf->uiType        = type;
    buf->uiOffset      = 0;

    uint32_t bufSize = 0;
    uint32_t expectedSize = 0xffffffff;

    switch ((int32_t)type)
    {
    case VAProbabilityBufferType:
    case VAEncCodedBufferType:
    {
        buf->iSize  = size;
        buf->format = Media_Format_Buffer;
        va           = DdiMediaUtil_CreateBuffer(buf, mediaCtx->pDrmBufMgr);
        if (va != VA_STATUS_SUCCESS)
        {
            MOS_FreeMemory(buf);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        break;
    }
    case VAEncMacroblockMapBufferType:
    {
        buf->uiWidth = MOS_ALIGN_CEIL(size, 64);
        if (size != buf->uiWidth)
        {
            va = VA_STATUS_ERROR_INVALID_PARAMETER;
            CleanUpBufferandReturn(buf);
            return va;
        }
        bufSize            = size * elementsNum;
        buf->uiHeight      = elementsNum;
        buf->uiPitch       = buf->uiWidth;
        buf->iSize         = bufSize;
        buf->format        = Media_Format_2DBuffer;
        buf->uiNumElements = 1;

        va = DdiMediaUtil_CreateBuffer(buf, mediaCtx->pDrmBufMgr);
        if (va != VA_STATUS_SUCCESS)
        {
            MOS_FreeMemory(buf);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        break;
    }
    case VAEncMacroblockDisableSkipMapBufferType:
    {
        buf->uiHeight = m_encodeCtx->wPicHeightInMB;
        buf->uiWidth  = m_encodeCtx->wPicWidthInMB;
        buf->iSize    = m_encodeCtx->wPicHeightInMB * m_encodeCtx->wPicWidthInMB;
        buf->format   = Media_Format_2DBuffer;

        va = DdiMediaUtil_CreateBuffer(buf, mediaCtx->pDrmBufMgr);
        if (va != VA_STATUS_SUCCESS)
        {
            MOS_FreeMemory(buf);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        break;
    }
    case VAEncSliceParameterBufferType:
    {
        // elementsNum could be larger than 1 for this case
        // modify to support MPEG2 later
        // Currently the slice boundary is at MB row level
        // Here size is assumed to be the size of the slice control parameter for one single slice
        // and elementsNum is the number of slices
        expectedSize = getSliceParameterBufferSize();

        if ((size < expectedSize) ||
            (0 == elementsNum) ||
            (elementsNum > (m_encodeCtx->dwFrameHeight / CODECHAL_MACROBLOCK_HEIGHT)))
        {
            va = VA_STATUS_ERROR_INVALID_PARAMETER;
            CleanUpBufferandReturn(buf);
            return va;
        }

        bufSize = size * elementsNum;
        break;
    }
    case VAEncSequenceParameterBufferType:  // does not exist for JPEG
    {
        // elementsNum should be 1, ignore here just for robustness
        bufSize = size;
        expectedSize = getSequenceParameterBufferSize();

        if (bufSize < expectedSize)
        {
            va = VA_STATUS_ERROR_INVALID_PARAMETER;
            CleanUpBufferandReturn(buf);
            return va;
        }
        break;
    }
    case VAEncPictureParameterBufferType:
    {
        // elementsNum should be 1, ignore here just for robustness
        bufSize = size;
        expectedSize = getPictureParameterBufferSize();

        if (bufSize < expectedSize)
        {
            va = VA_STATUS_ERROR_INVALID_PARAMETER;
            CleanUpBufferandReturn(buf);
            return va;
        }
        break;
    }
    case VAIQMatrixBufferType:
    case VAQMatrixBufferType:
    {
        // elementsNum should be 1, ignore here just for robustness
        bufSize = size;
        expectedSize = getQMatrixBufferSize();

        if (bufSize < expectedSize)
        {
            va = VA_STATUS_ERROR_INVALID_PARAMETER;
            CleanUpBufferandReturn(buf);
            return va;
        }
        break;
    }
    case VAEncPackedHeaderParameterBufferType:  // doesnt exist for JPEG
    {
        // elementsNum should be 1, ignore here just for robustness
        bufSize = size;
        if (bufSize < sizeof(VAEncPackedHeaderParameterBuffer))
        {
            va = VA_STATUS_ERROR_INVALID_PARAMETER;
            CleanUpBufferandReturn(buf);
            return va;
        }
        break;
    }
    case VAEncPackedHeaderDataBufferType:  // doesnt exist for JPEG
    {
        // elementsNum should be 1, ignore here just for robustness
        bufSize = size;
        break;
    }
    case VAEncMiscParameterBufferType:  // doesnt exist for JPEG
    {
        // elementsNum should be 1, ignore here just for robustness
        bufSize = size;

        if (bufSize < sizeof(VAEncMiscParameterBuffer))
        {
            va = VA_STATUS_ERROR_INVALID_PARAMETER;
            CleanUpBufferandReturn(buf);
            return va;
        }
        break;
    }
    case VAHuffmanTableBufferType:  // only for JPEG
    {
        bufSize = size;

        if (bufSize < sizeof(VAHuffmanTableBufferJPEGBaseline))
        {
            va = VA_STATUS_ERROR_INVALID_PARAMETER;
            CleanUpBufferandReturn(buf);
            return va;
        }
        break;
    }
    case VAEncFEIMBControlBufferType:
    {
        bufSize       = size;
        buf->iSize  = size;
        buf->format = Media_Format_Buffer;
        va           = DdiMediaUtil_CreateBuffer(buf, mediaCtx->pDrmBufMgr);
        if (va != VA_STATUS_SUCCESS)
        {
            MOS_FreeMemory(buf);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        break;
    }
    case VAEncFEIMVPredictorBufferType:
    {
        bufSize       = size;
        buf->iSize  = size;
        buf->format = Media_Format_Buffer;
        va           = DdiMediaUtil_CreateBuffer(buf, mediaCtx->pDrmBufMgr);
        if (va != VA_STATUS_SUCCESS)
        {
            MOS_FreeMemory(buf);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        break;
    }
    case VAEncQPBufferType:
    {
        //The permb qp buffer of legacy encoder is a 2D buffer, because dynamic resolution change, we cant determine the buffer size with the resolution information in encoder context
        //so the size information should be from application, the width should be the size, the height is the elementsNum to define this 2D buffer,width should always 64 byte alignment.
        //please pay attention: 1 byte present 1 MB QP values for AVC, 4 bytes present 1 MB QP values for MPEG2, lowest byte is the real QP value, other 3 byes is other mb level contrl
        //which havent been exposed. the permb QP buffer of FEI is 1D buffer.
        if (CODECHAL_FUNCTION_ENC_PAK == m_encodeCtx->codecFunction ||
            CODECHAL_FUNCTION_ENC_VDENC_PAK == m_encodeCtx->codecFunction ||
            (((CODECHAL_FUNCTION_FEI_ENC_PAK == m_encodeCtx->codecFunction) || (CODECHAL_FUNCTION_FEI_ENC == m_encodeCtx->codecFunction)) &&
              (m_encodeCtx->wModeType == CODECHAL_ENCODE_MODE_HEVC)))
        {
            buf->uiWidth = MOS_ALIGN_CEIL(size, 64);
            if (size != buf->uiWidth)
            {
                va = VA_STATUS_ERROR_INVALID_PARAMETER;
                CleanUpBufferandReturn(buf);
                return va;
            }
            bufSize            = size * elementsNum;
            buf->uiHeight      = elementsNum;
            buf->uiPitch       = buf->uiWidth;
            buf->iSize         = bufSize;
            buf->format        = Media_Format_2DBuffer;
            buf->uiNumElements = 1;
        }
        else
        {
            bufSize       = size;
            buf->iSize  = size;
            buf->format = Media_Format_Buffer;
        }
        va = DdiMediaUtil_CreateBuffer(buf, mediaCtx->pDrmBufMgr);
        if (va != VA_STATUS_SUCCESS)
        {
            MOS_FreeMemory(buf);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        break;
    }
    case VAEncFEICTBCmdBufferType:
    case VAEncFEIMVBufferType:
    {
        bufSize       = size;
        buf->iSize  = bufSize;
        buf->format = Media_Format_Buffer;
        va           = DdiMediaUtil_CreateBuffer(buf, mediaCtx->pDrmBufMgr);
        if (va != VA_STATUS_SUCCESS)
        {
            MOS_FreeMemory(buf);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        break;
    }
    case VAEncFEICURecordBufferType:
    case VAEncFEIMBCodeBufferType:
    {
        bufSize       = size;
        buf->iSize  = bufSize;
        buf->format = Media_Format_Buffer;
        va           = DdiMediaUtil_CreateBuffer(buf, mediaCtx->pDrmBufMgr);
        if (va != VA_STATUS_SUCCESS)
        {
            MOS_FreeMemory(buf);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        break;
    }
    case VAEncFEIDistortionBufferType:
    {
        bufSize       = size;
        buf->iSize  = bufSize;
        buf->format = Media_Format_Buffer;
        va           = DdiMediaUtil_CreateBuffer(buf, mediaCtx->pDrmBufMgr);
        if (va != VA_STATUS_SUCCESS)
        {
            MOS_FreeMemory(buf);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        break;
    }
    case VAStatsStatisticsParameterBufferType:
    {
        // elementsNum should be 1, ignore here just for robustness
        bufSize = size;
        if (bufSize < sizeof(VAStatsStatisticsParameterH264))
        {
            va = VA_STATUS_ERROR_INVALID_PARAMETER;
            CleanUpBufferandReturn(buf);
            return va;
        }

        break;
    }
    case VAStatsMVPredictorBufferType:
    {
        bufSize       = size;
        buf->iSize  = size * elementsNum;
        buf->format = Media_Format_Buffer;
        va           = DdiMediaUtil_CreateBuffer(buf, mediaCtx->pDrmBufMgr);
        if (va != VA_STATUS_SUCCESS)
        {
            MOS_FreeMemory(buf);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }

        break;
    }
    case VAStatsMVBufferType:
    {
        bufSize       = size;
        buf->iSize  = size * elementsNum;
        buf->format = Media_Format_Buffer;
        va           = DdiMediaUtil_CreateBuffer(buf, mediaCtx->pDrmBufMgr);
        if (va != VA_STATUS_SUCCESS)
        {
            MOS_FreeMemory(buf);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        break;
    }
    case VAStatsStatisticsBufferType:
    case VAStatsStatisticsBottomFieldBufferType:
    {
        bufSize       = size;
        buf->iSize  = size * elementsNum;
        buf->format = Media_Format_Buffer;
        va           = DdiMediaUtil_CreateBuffer(buf, mediaCtx->pDrmBufMgr);
        if (va != VA_STATUS_SUCCESS)
        {
            MOS_FreeMemory(buf);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        break;
    }
    default:
    {
        bufSize = size * elementsNum;

        if (0 == bufSize)
        {
            va = VA_STATUS_ERROR_INVALID_PARAMETER;
            CleanUpBufferandReturn(buf);
            return va;
        }

        va = m_encodeCtx->pCpDdiInterface->CreateBuffer(type, buf, size, elementsNum);
        if (va  == VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE)
        {
            MOS_FreeMemory(buf);
            DDI_ASSERTMESSAGE("DDI: non supported buffer type = %d, size = %d, num = %d", type, size, elementsNum);
            return va;
        }

        break;
    }
    }

    if ((VAEncCodedBufferType != type) &&
        (VAEncMacroblockMapBufferType != type) &&
        (VAEncFEIMVBufferType != type) &&
        (VAEncFEIMBCodeBufferType != type) &&
        (VAEncFEICTBCmdBufferType != type)      &&
        (VAEncFEICURecordBufferType != type)    &&
        (VAEncFEIDistortionBufferType != type) &&
        (VAEncFEIMBControlBufferType != type) &&
        (VAEncFEIMVPredictorBufferType != type) &&
        (VAStatsMVBufferType != type) &&
        (VAStatsStatisticsBufferType != type) &&
        (VAStatsStatisticsBottomFieldBufferType != type) &&
        (VAStatsMVPredictorBufferType != type) &&
        (VAEncQPBufferType != type) &&
        (VAEncMacroblockDisableSkipMapBufferType != (int32_t)type) &&
        (VAProbabilityBufferType != (int32_t)type))
    {
        buf->pData = (uint8_t*)MOS_AllocAndZeroMemory(bufSize);
        if (nullptr == buf->pData)
        {
            va = VA_STATUS_ERROR_ALLOCATION_FAILED;
            CleanUpBufferandReturn(buf);
            return va;
        }
        buf->iSize  = bufSize;
        buf->format = Media_Format_CPU;
    }

    PDDI_MEDIA_BUFFER_HEAP_ELEMENT bufferHeapElement = DdiMediaUtil_AllocPMediaBufferFromHeap(mediaCtx->pBufferHeap);
    if (nullptr == bufferHeapElement)
    {
        va = VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        CleanUpBufferandReturn(buf);
        return va;
    }

    bufferHeapElement->pBuffer   = buf;
    bufferHeapElement->pCtx      = (void*)m_encodeCtx;
    bufferHeapElement->uiCtxType = DDI_MEDIA_CONTEXT_TYPE_ENCODER;
    *bufId                        = bufferHeapElement->uiVaBufferID;
    mediaCtx->uiNumBufs++;

    // return success if data is nullptr, no need to copy data
    if (data == nullptr || VAEncMacroblockMapBufferType == type)
    {
        return va;
    }

    DdiMediaUtil_LockBuffer(buf, MOS_LOCKFLAG_WRITEONLY | MOS_LOCKFLAG_READONLY);

    MOS_STATUS eStatus = MOS_SecureMemcpy((void*)(buf->pData + buf->uiOffset), bufSize, (void*)data, bufSize);

    DdiMediaUtil_UnlockBuffer(buf);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        DdiMedia_DestroyBufFromVABufferID(mediaCtx, bufferHeapElement->uiVaBufferID);
        va = VA_STATUS_ERROR_OPERATION_FAILED;
        CleanUpBufferandReturn(buf);
        return va;
    }

    // return success
    return va;
}

void DdiEncodeBase::CleanUpBufferandReturn(DDI_MEDIA_BUFFER *buf)
{
    if (buf)
    {
        MOS_FreeMemory(buf->pData);
        MOS_FreeMemory(buf);
    }
}

uint32_t DdiEncodeBase::getSliceParameterBufferSize()
{
    return 0xffffffff;
}

uint32_t DdiEncodeBase::getSequenceParameterBufferSize()
{
    return 0xffffffff;
}

uint32_t DdiEncodeBase::getPictureParameterBufferSize()
{
    return 0xffffffff;
}

uint32_t DdiEncodeBase::getQMatrixBufferSize()
{
    return 0xffffffff;
}

void DdiEncodeBase::ClearPicParams()
{
}
