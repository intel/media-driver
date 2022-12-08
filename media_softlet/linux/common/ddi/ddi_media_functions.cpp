/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     ddi_media_functions.cpp
//! \brief    ddi media functions implementaion.
//!
#include "ddi_media_functions.h"
#include "media_libva_util_next.h"
#include "media_libva_interface_next.h"
#include "media_libva_caps_next.h"

const VAProcColorStandardType DdiMediaFunctions::m_vpInputColorStd[DDI_VP_NUM_INPUT_COLOR_STD] = {
    VAProcColorStandardBT601,
    VAProcColorStandardBT709,
    VAProcColorStandardSRGB,
    VAProcColorStandardSTRGB,
    VAProcColorStandardBT2020,
    VAProcColorStandardExplicit
};

const VAProcColorStandardType DdiMediaFunctions::m_vpOutputColorStd[DDI_VP_NUM_OUT_COLOR_STD] = {
    VAProcColorStandardBT601,
    VAProcColorStandardBT709,
    VAProcColorStandardSRGB,
    VAProcColorStandardSTRGB,
    VAProcColorStandardBT2020,
    VAProcColorStandardExplicit
};

VAStatus DdiMediaFunctions::CreateContext (
    VADriverContextP  ctx,
    VAConfigID        configId,
    int32_t           pictureWidth,
    int32_t           pictureHeight,
    int32_t           flag,
    VASurfaceID       *renderTargets,
    int32_t           renderTargetsNum,
    VAContextID       *context)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaFunctions::DestroyContext (
    VADriverContextP  ctx,
    VAContextID       context)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaFunctions::CreateBuffer (
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferType      type,
    uint32_t          size,
    uint32_t          elementsNum,
    void              *data,
    VABufferID        *bufId)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaFunctions::MapBufferInternal(
    DDI_MEDIA_CONTEXT   *mediaCtx,
    VABufferID          bufId,
    void                **buf,
    uint32_t            flag)
{
    VAStatus         vaStatus  = VA_STATUS_SUCCESS;
    DDI_MEDIA_BUFFER *mediaBuf = nullptr;

    DDI_FUNC_ENTER;

    mediaBuf = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, bufId);
    DDI_CHK_NULL(mediaBuf, "nullptr mediaBuf", VA_STATUS_ERROR_INVALID_BUFFER);

    switch((int32_t)mediaBuf->uiType)
    {
        case VAImageBufferType:
        default:
            if((mediaBuf->format != Media_Format_CPU) && (MediaLibvaInterfaceNext::MediaFormatToOsFormat(mediaBuf->format) != VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT))
            {
                MosUtilities::MosLockMutex(&mediaCtx->BufferMutex);
                // A critical section starts.
                // Make sure not to bailout with a return until the section ends.
                if (nullptr != mediaBuf->pSurface && Media_Format_CPU != mediaBuf->format)
                {
                    vaStatus = MediaLibvaInterfaceNext::MediaMemoryDecompress(mediaCtx, mediaBuf->pSurface);
                }

                if (VA_STATUS_SUCCESS == vaStatus)
                {
                    *buf = MediaLibvaUtilNext::LockBuffer(mediaBuf, flag);
                    if (nullptr == *buf)
                    {
                        vaStatus = VA_STATUS_ERROR_OPERATION_FAILED;
                    }
                }

                // The critical section ends.
                MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);
            }
            else
            {
                *buf = (void *)(mediaBuf->pData + mediaBuf->uiOffset);
            }
            break;
    }
    return vaStatus;
}

VAStatus DdiMediaFunctions::UnmapBuffer (
    DDI_MEDIA_CONTEXT  *mediaCtx,
    VABufferID         bufId)
{
    DDI_MEDIA_BUFFER   *mediaBuf = nullptr;
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    mediaBuf = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx,  bufId);
    DDI_CHK_NULL(mediaBuf, "nullptr mediaBuf", VA_STATUS_ERROR_INVALID_BUFFER);

    switch ((int32_t)mediaBuf->uiType)
    {
        case VAImageBufferType:
        default:
            if((mediaBuf->format != Media_Format_CPU) && (MediaLibvaInterfaceNext::MediaFormatToOsFormat(mediaBuf->format) != VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT))
            {
                MosUtilities::MosLockMutex(&mediaCtx->BufferMutex);
                MediaLibvaUtilNext::UnlockBuffer(mediaBuf);
                MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);
            }
        break;
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiMediaFunctions::DestroyBuffer(
    DDI_MEDIA_CONTEXT  *mediaCtx,
    VABufferID         bufId)
{
    DDI_MEDIA_BUFFER   *buf = nullptr;
    DDI_FUNC_ENTER;

    buf = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, bufId);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_BUFFER);

    switch((int32_t)buf->uiType)
    {
        case VAImageBufferType:
            if(buf->format == Media_Format_CPU)
            {
                MOS_DeleteArray(buf->pData);
            }
            else
            {
                MediaLibvaUtilNext::UnRefBufObjInMediaBuffer(buf);

                if (buf->uiExportcount)
                {
                    buf->bPostponedBufFree = true;
                    MOS_TraceEventExt(EVENT_VA_FREE_BUFFER, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
                    return VA_STATUS_SUCCESS;
                }
            }
            break;
        case VAStatsStatisticsParameterBufferType:
        default: // do not handle any un-listed buffer type
            MOS_DeleteArray(buf->pData);
            break;
    }
    MOS_Delete(buf);
    MediaLibvaInterfaceNext::DestroyBufFromVABufferID(mediaCtx, bufId);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiMediaFunctions::BeginPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VASurfaceID       renderTarget)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaFunctions::RenderPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferID        *buffers,
    int32_t           buffersNum)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaFunctions::EndPicture (
    VADriverContextP  ctx,
    VAContextID       context)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaFunctions::CreateConfig (
    VADriverContextP  ctx,
    VAProfile         profile,
    VAEntrypoint      entrypoint,
    VAConfigAttrib    *attribList,
    int32_t           attribsNum,
    VAConfigID        *configId)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    DDI_CODEC_FUNC_ENTER;

    VAStatus status = VA_STATUS_SUCCESS;
    DDI_CODEC_CHK_NULL(configId,   "nullptr configId",   VA_STATUS_ERROR_INVALID_PARAMETER);
    if(attribsNum)
    {
        DDI_CODEC_CHK_NULL(attribList, "nullptr attribList", VA_STATUS_ERROR_INVALID_PARAMETER);
    }

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(mediaCtx->m_capsNext, "nullptr m_caps", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CODEC_CHK_NULL(mediaCtx->m_capsNext->m_capsTable, "nullptr m_capsTable", VA_STATUS_ERROR_INVALID_PARAMETER);

    status = mediaCtx->m_capsNext->CreateConfig(profile, entrypoint, attribList, attribsNum, configId);
    DDI_CODEC_CHK_RET(status, "Create common config failed");

    return status;
}

VAStatus DdiMediaFunctions::QueryVideoProcFilters(
    VADriverContextP  ctx,
    VAContextID       context,
    VAProcFilterType  *filters,
    uint32_t          *filtersNum)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaFunctions::QueryVideoProcFilterCaps(
    VADriverContextP  ctx,
    VAContextID       context,
    VAProcFilterType  type,
    void              *filterCaps,
    uint32_t          *filterCapsNum)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaFunctions::QueryVideoProcPipelineCaps(
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID          *filters,
    uint32_t            filtersNum,
    VAProcPipelineCaps  *pipelineCaps)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

#if VA_CHECK_VERSION(1,11,0)
VAStatus DdiMediaFunctions::CreateProtectedSession(
    VADriverContextP      ctx,
    VAConfigID            configId,
    VAProtectedSessionID  *protectedSession)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaFunctions::DestroyProtectedSession(
    VADriverContextP      ctx,
    VAProtectedSessionID  protectedSession)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaFunctions::AttachProtectedSession(
    VADriverContextP      ctx,
    VAContextID           context,
    VAProtectedSessionID  protectedSession)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaFunctions::DetachProtectedSession(
    VADriverContextP  ctx,
    VAContextID       context)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaFunctions::ProtectedSessionExecute(
    VADriverContextP      ctx,
    VAProtectedSessionID  protectedSession,
    VABufferID            data)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}
#endif

VAStatus DdiMediaFunctions::StatusCheck(
    PDDI_MEDIA_CONTEXT mediaCtx,
    DDI_MEDIA_SURFACE  *surface,
    VASurfaceID        surfaceId)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiMediaFunctions::QuerySurfaceError(
    VADriverContextP ctx,
    VASurfaceID      renderTarget,
    VAStatus         errorStatus,
    void             **errorInfo)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaFunctions::PutSurface(
    VADriverContextP ctx,
    VASurfaceID      surface,
    void             *draw,
    int16_t          srcx,
    int16_t          srcy,
    uint16_t         srcw,
    uint16_t         srch,
    int16_t          destx,
    int16_t          desty,
    uint16_t         destw,
    uint16_t         desth,
    VARectangle      *cliprects,
    uint32_t         numberCliprects,
    uint32_t         flags)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaFunctions::ProcessPipeline(
    VADriverContextP    vaDrvCtx,
    VAContextID         ctxID,
    VASurfaceID         srcSurface,
    VARectangle         *srcRect,
    VASurfaceID         dstSurface,
    VARectangle         *dstRect
)
{
    DDI_ASSERTMESSAGE("Unsupported function call.");
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}