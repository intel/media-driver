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
//! \file     codechal_encode_tracked_buffer.cpp
//! \brief    Class to manage tracked buffer used in encoder
//!

#include "codechal_encode_tracked_buffer_hevc.h"
#include "codechal_encoder_base.h"

void CodechalEncodeTrackedBufferHevc::LookUpBufIndexMbCode()
{
    // the value of current index will recycle in CODEC_NUM_REF_BUFFERS, CODEC_NUM_REF_BUFFERS + 2, CODEC_NUM_REF_BUFFERS + 1
    m_mbCodeAnteIdx = m_mbCodePenuIdx;
    m_mbCodePenuIdx = m_mbCodeCurrIdx;
    m_mbCodeCurrIdx = m_mbCodeCurrIdx % CODEC_NUM_NON_REF_BUFFERS;
    m_mbCodeCurrIdx += CODEC_NUM_REF_BUFFERS;
}

MOS_STATUS CodechalEncodeTrackedBufferHevc::AllocateMvTemporalBuffer()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_trackedBufCurrMvTemporal = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, mvTemporalBuffer, m_trackedBufCurrIdx))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hevcState);

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBufCurrMvTemporal = (MOS_RESOURCE*)m_allocator->AllocateResource(
        m_standard, m_hevcState->dwSizeOfMvTemporalBuffer, 1, mvTemporalBuffer, m_trackedBufCurrIdx, true));

    return MOS_STATUS_SUCCESS;
}

void CodechalEncodeTrackedBufferHevc::ReleaseBufferOnResChange()
{
    if ((m_trackedBufAnteIdx != m_trackedBufPenuIdx) &&
        (m_trackedBufAnteIdx != m_trackedBufCurrIdx))
    {
        ReleaseMvData(m_trackedBufAnteIdx);
        ReleaseDsRecon(m_trackedBufAnteIdx);
#ifndef _FULL_OPEN_SOURCE
        ReleaseSurfaceDS(m_trackedBufAnteIdx);
#endif
        m_trackedBuffer[m_trackedBufAnteIdx].ucSurfIndex7bits = PICTURE_MAX_7BITS;
        CODECHAL_ENCODE_NORMALMESSAGE("Tracked buffer = %d re-allocated", m_trackedBufAnteIdx);
    }

    // release MbCode buffer
    if ((m_mbCodeAnteIdx != m_mbCodePenuIdx) &&
        (m_mbCodeAnteIdx != m_mbCodeCurrIdx))
    {
        ReleaseMbCode(m_mbCodeAnteIdx);
    }    
}

CodechalEncodeTrackedBufferHevc::CodechalEncodeTrackedBufferHevc(CodechalEncoderState* encoder)
    : CodechalEncodeTrackedBuffer(encoder)
{
    m_hevcState = dynamic_cast<CodechalEncodeHevcBase*>(encoder);

    // HEVC does not need to keep ref frame's PakObject, can use a size-3 ring buffer instead
    m_mbCodeIsTracked = false;

    m_mbCodeAnteIdx = m_mbCodePenuIdx = m_mbCodeCurrIdx = 0;
}
