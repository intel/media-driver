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

MosMediaCopy::MosMediaCopy(PMOS_CONTEXT mosCtx):
    MosMediaCopyBase(mosCtx)
{
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(mosCtx);
    if(m_mediaCopyState == nullptr)
    {
        m_mediaCopyState = static_cast<MediaCopyBaseState *>(McpyDevice::CreateFactory(mosCtx));
    }
}

MediaCopyBaseState **MosMediaCopy::GetMediaCopyState()
{
    return &m_mediaCopyState;
}
