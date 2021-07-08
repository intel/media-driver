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
//! \file     ddi_encode_functions.cpp
//! \brief    ddi encode functions implementaion.
//!
#include "ddi_encode_functions.h"

VAStatus DdiEncodeFunctions::CreateContext (
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

VAStatus DdiEncodeFunctions::DestroyContext (
    VADriverContextP  ctx,
    VAContextID       context)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeFunctions::CreateBuffer (
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

VAStatus DdiEncodeFunctions::BeginPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VASurfaceID       renderTarget)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeFunctions::RenderPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferID        *buffers,
    int32_t           buffersNum)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeFunctions::EndPicture (
    VADriverContextP  ctx,
    VAContextID       context)
{
    return VA_STATUS_SUCCESS;
}