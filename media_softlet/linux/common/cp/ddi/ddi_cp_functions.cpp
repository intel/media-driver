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
//! \file     ddi_cp_functions.cpp
//! \brief    ddi cp functions implementaion.
//!
#include "ddi_cp_functions.h"

#if VA_CHECK_VERSION(1,11,0)
VAStatus DdiCpFunctions::CreateProtectedSession(
    VADriverContextP      ctx,
    VAConfigID            configId,
    VAProtectedSessionID  *protectedSession)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiCpFunctions::DestroyProtectedSession(
    VADriverContextP      ctx,
    VAProtectedSessionID  protectedSession)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiCpFunctions::AttachProtectedSession(
    VADriverContextP      ctx,
    VAContextID           context,
    VAProtectedSessionID  protectedSession)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiCpFunctions::DetachProtectedSession(
    VADriverContextP  ctx,
    VAContextID       context)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiCpFunctions::ProtectedSessionExecute(
    VADriverContextP      ctx,
    VAProtectedSessionID  protectedSession,
    VABufferID            data)
{
    return VA_STATUS_SUCCESS;
}
#endif

VAStatus DdiCpFunctions::CreateBuffer(
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