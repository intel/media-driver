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
#include "codechal_decode_histogram.h"

CodechalDecodeHistogram::CodechalDecodeHistogram(
    CodechalHwInterface *hwInterface,
    MOS_INTERFACE *osInterface)
{
    m_hwInterface = hwInterface;
    m_osInterface = osInterface;
    MOS_ZeroMemory(&m_resHistogram, sizeof(m_resHistogram));
    MOS_ZeroMemory(m_inputHistogramSurfaces, sizeof(m_inputHistogramSurfaces));
}

CodechalDecodeHistogram::~CodechalDecodeHistogram()
{
    if (!Mos_ResourceIsNull(&m_resHistogram))
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_resHistogram);
    }
}

void CodechalDecodeHistogram::setHistogramComponent(uint8_t component)
{
    m_histogramComponent = component;
}

PMOS_SURFACE CodechalDecodeHistogram::GetHistogramSurface()
{
    return &m_inputHistogramSurfaces[m_histogramComponent];
}

MOS_STATUS CodechalDecodeHistogram::RenderHistogram(
    CodechalDecode *codechalDecoder,
    MOS_SURFACE *inputSurface)
{
    return MOS_STATUS_SUCCESS;
}
