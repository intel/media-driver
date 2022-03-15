/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     mos_mediacopy.cpp
//! \brief    MOS media copy functions
//! \details  MOS media copy functions
//!

#include "mos_mediacopy.h"
#include "media_interfaces_mcpy.h"

MosMediaCopy::MosMediaCopy(PMOS_CONTEXT mosCtx)
{
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(mosCtx);
    m_mediaCopyState = static_cast<MediaCopyBaseState *>(McpyDevice::CreateFactory(mosCtx));
}

MosMediaCopy::~MosMediaCopy()
{
    MOS_Delete(m_mediaCopyState);
}

MediaCopyBaseState **MosMediaCopy::GetMediaCopyState()
{
    return &m_mediaCopyState;
}

MOS_STATUS MosMediaCopy::MediaCopy(
    PMOS_RESOURCE inputResource,
    PMOS_RESOURCE outputResource,
    MCPY_METHOD   preferMethod)
{
    MOS_OS_CHK_STATUS_RETURN(m_mediaCopyState->SurfaceCopy(
        inputResource,
        outputResource,
        preferMethod));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosMediaCopy::MediaCopy(
    MEDIAUMD_RESOURCE inputResource,
    uint32_t          inputResourceIndex,
    MEDIAUMD_RESOURCE outputResource,
    uint32_t          outputResourceIndex,
    MCPY_METHOD       preferMethod)
{
    return MOS_STATUS_UNIMPLEMENTED;
}