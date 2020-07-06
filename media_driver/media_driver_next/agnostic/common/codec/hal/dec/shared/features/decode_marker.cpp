/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     decode_marker.cpp
//! \brief    Defines the common interface for decode marker
//! \details  The decode marker interface implements the decode marker feature.
//!
#include "decode_marker.h"
#include "decode_utils.h"

namespace decode {

DecodeMarker::DecodeMarker(DecodeAllocator& allocator) :
    m_allocator(&allocator)
{
}

DecodeMarker::~DecodeMarker()
{
    MOS_Delete(m_markerBuffer);
}

MOS_STATUS DecodeMarker::Init(CodechalDecodeParams& params)
{
    DECODE_FUNC_CALL();

    m_setMarkerEnabled = params.m_setMarkerEnabled;
    m_setMarkerNumTs   = params.setMarkerNumTs;

    if (m_setMarkerEnabled)
    {
        DECODE_CHK_NULL(params.m_presSetMarker);
    }

    m_markerBuffer = MOS_New(MOS_BUFFER);
    if (params.m_presSetMarker != nullptr)
    {
        m_markerBuffer->OsResource = *params.m_presSetMarker;
    }
    else
    {
        MOS_ZeroMemory(m_markerBuffer, sizeof(MOS_BUFFER));
    }

    return MOS_STATUS_SUCCESS;
}

}
