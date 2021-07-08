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
#include "media_libva_util.h"

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
    return VA_STATUS_ERROR_UNIMPLEMENTED;
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