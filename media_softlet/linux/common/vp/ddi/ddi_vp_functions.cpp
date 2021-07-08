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
//! \file     ddi_vp_functions.cpp
//! \brief    ddi vp functions implementaion.
//!
#include "ddi_vp_functions.h"

VAStatus DdiVpFunctions::CreateContext (
    VADriverContextP  ctx,
    VAConfigID        configId,
    int32_t           pictureWidth,
    int32_t           pictureHeight,
    int32_t           flag,
    VASurfaceID       *renderTargets,
    int32_t           renderTargetsNum,
    VAContextID       *context)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DestroyContext (
    VADriverContextP  ctx,
    VAContextID       context)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::CreateBuffer (
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferType      type,
    uint32_t          size,
    uint32_t          elementsNum,
    void              *data,
    VABufferID        *bufId)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::BeginPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VASurfaceID       renderTarget)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::RenderPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferID        *buffers,
    int32_t           buffersNum)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::EndPicture (
    VADriverContextP  ctx,
    VAContextID       context)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::CreateConfig (
    VADriverContextP  ctx,
    VAProfile         profile,
    VAEntrypoint      entrypoint,
    VAConfigAttrib    *attribList,
    int32_t           attribsNum,
    VAConfigID        *configId)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::QueryVideoProcFilters(
    VADriverContextP  ctx,
    VAContextID       context,
    VAProcFilterType  *filters,
    uint32_t          *filtersNum)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::QueryVideoProcFilterCaps(
    VADriverContextP  ctx,
    VAContextID       context,
    VAProcFilterType  type,
    void              *filterCaps,
    uint32_t          *filterCapsNum)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::QueryVideoProcPipelineCaps(
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID          *filters,
    uint32_t            filtersNum,
    VAProcPipelineCaps  *pipelineCaps)
{
    return VA_STATUS_SUCCESS;
}