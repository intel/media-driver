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
//! \file     codechal_decode_histogram.cpp
//! \brief    implements the base class of decode histogram.
//! \details  decode histogram base class implementation.
//!
#include "codechal_decode_histogram_g12.h"

CodechalDecodeHistogramG12::CodechalDecodeHistogramG12(
    CodechalHwInterface *hwInterface,
    MOS_INTERFACE *osInterface) : CodechalDecodeHistogram(hwInterface, osInterface)
{
    m_hwInterface = hwInterface;
    m_osInterface = osInterface;
}

CodechalDecodeHistogramG12::~CodechalDecodeHistogramG12()
{

}

void CodechalDecodeHistogramG12::SetSrcHistogramSurface(PMOS_SURFACE refSurface)
{
    if(!m_inputSurface)
    {
        m_inputSurface = refSurface;
    }
}

MOS_STATUS CodechalDecodeHistogramG12::RenderHistogram(
    CodechalDecode *codechalDecoder,
    MOS_SURFACE *inputSurface)
{
    m_decoder = codechalDecoder;

    MOS_COMMAND_BUFFER cmdBuffer;
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
    CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0));

    // copy histogram to input buffer
    m_osInterface->pfnSetPerfTag(
        m_osInterface,
        (uint16_t)(((m_decoder->GetMode() << 4) & 0xF0) | COPY_TYPE));
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    PMOS_SURFACE destSurface = GetHistogramSurface();

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_decoder->HucCopy(
        &cmdBuffer,
        &m_inputSurface->OsResource,
        &destSurface->OsResource,
        HISTOGRAM_BINCOUNT * 4,
        0,
        destSurface->dwOffset));

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiFlushDwCmd(
        &cmdBuffer,
        &flushDwParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        m_decoder->GetVideoContextUsesNullHw()));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(
        m_osInterface,
        m_decoder->GetVideoContext()));

    return MOS_STATUS_SUCCESS;
}
