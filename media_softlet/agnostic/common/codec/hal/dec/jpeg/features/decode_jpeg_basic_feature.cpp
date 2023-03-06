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
//! \file     decode_jpeg_basic_features.cpp
//! \brief    Defines the common interface for decode jpeg basic feature
//!
#include "decode_jpeg_basic_feature.h"
#include "decode_utils.h"
#include "decode_allocator.h"
#include "decode_resource_array.h"

namespace decode
{
JpegBasicFeature::~JpegBasicFeature()
{
}

MOS_STATUS JpegBasicFeature::Init(void *setting)
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);
    DECODE_CHK_STATUS(DecodeBasicFeature::Init(setting));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegBasicFeature::Update(void *params)
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_NULL(params);
    DECODE_CHK_STATUS(DecodeBasicFeature::Update(params));

    CodechalDecodeParams *decodeParams = (CodechalDecodeParams *)params;

    m_jpegPicParams    = (CodecDecodeJpegPicParams *)decodeParams->m_picParams;
    m_jpegPicParams    = (CodecDecodeJpegPicParams *)decodeParams->m_picParams;
    m_jpegQMatrix      = (CodecJpegQuantMatrix *)decodeParams->m_iqMatrixBuffer;
    m_jpegHuffmanTable = (PCODECHAL_DECODE_JPEG_HUFFMAN_TABLE)decodeParams->m_huffmanTable;
    m_jpegScanParams   = (CodecDecodeJpegScanParameter *)decodeParams->m_sliceParams;
    DECODE_CHK_NULL(m_jpegPicParams);

    GetRenderTargetFormat(&m_destSurface.Format);

    DECODE_CHK_STATUS(CheckSupportedFormat(
        &m_destSurface.Format));

    uint32_t totalDataLength;
    
    totalDataLength = m_jpegScanParams->ScanHeader[m_jpegScanParams->NumScans - 1].DataOffset +
                      m_jpegScanParams->ScanHeader[m_jpegScanParams->NumScans - 1].DataLength;

    //update m_datasize
    DECODE_CHK_STATUS(SetRequiredBitstreamSize(totalDataLength));

    DECODE_CHK_STATUS(SetPictureStructs());

    return MOS_STATUS_SUCCESS;
}

void JpegBasicFeature::GetRenderTargetFormat(PMOS_FORMAT format)
{
    if (*format == Format_420O || *format == Format_AYUV)
    {
        if (m_osInterface != nullptr)
        {
            *format = m_osInterface->pfnOsFmtToMosFmt((MOS_OS_FORMAT)m_jpegPicParams->m_renderTargetFormat);
        }
    }
}

MOS_STATUS JpegBasicFeature::CheckSupportedFormat(PMOS_FORMAT format)
{
    DECODE_FUNC_CALL();

    //No support for RGBP/BGRP channel swap or YUV/RGB conversion!
    switch (*format)
    {
    case Format_BGRP:
        if (m_jpegPicParams->m_chromaType == jpegRGB ||
            m_jpegPicParams->m_chromaType == jpegYUV444)
        {
            return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
        }
        break;
    case Format_RGBP:
        if (m_jpegPicParams->m_chromaType == jpegYUV444)
        {
            return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
        }
        break;
    case Format_Y416:
    case Format_AYUV:
    case Format_AUYV:
    case Format_Y410:
        if (m_jpegPicParams->m_chromaType == jpegRGB ||
            m_jpegPicParams->m_chromaType == jpegBGR)
        {
            return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
        }
        break;
    default:
        break;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegBasicFeature::SetRequiredBitstreamSize(uint32_t requiredSize)
{
    DECODE_FUNC_CALL();

    if (requiredSize > m_dataSize)
    {
        m_dataOffset = 0;
        m_dataSize   = MOS_ALIGN_CEIL(requiredSize, MHW_CACHELINE_SIZE);
    }

    DECODE_NORMALMESSAGE("Estimate bitstream size in this Frame: %u", requiredSize);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegBasicFeature::SetPictureStructs()
{
    DECODE_FUNC_CALL();
    uint32_t widthAlign  = 0;
    uint32_t heightAlign = 0;

    // Overwriting surface width and height of destination surface, so it comes from Picture Parameters struct
    if (!m_jpegPicParams->m_interleavedData)
    {
        widthAlign  = MOS_ALIGN_CEIL(m_jpegPicParams->m_frameWidth, CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE);
        heightAlign = MOS_ALIGN_CEIL(m_jpegPicParams->m_frameHeight, CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE);
    }
    else
    {
        switch (m_jpegPicParams->m_chromaType)
        {
        case jpegYUV400:
        case jpegYUV444:
        case jpegRGB:
        case jpegBGR:
            widthAlign  = MOS_ALIGN_CEIL(m_jpegPicParams->m_frameWidth, CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE);
            heightAlign = MOS_ALIGN_CEIL(m_jpegPicParams->m_frameHeight, CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE);
            break;
        case jpegYUV422V2Y:
            widthAlign  = MOS_ALIGN_CEIL(m_jpegPicParams->m_frameWidth, CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE);
            heightAlign = MOS_ALIGN_CEIL(m_jpegPicParams->m_frameHeight, CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE_X2);
            break;
        case jpegYUV422H2Y:
            widthAlign  = MOS_ALIGN_CEIL(m_jpegPicParams->m_frameWidth, CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE_X2);
            heightAlign = MOS_ALIGN_CEIL(m_jpegPicParams->m_frameHeight, CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE);
            break;
        case jpegYUV411:
            widthAlign  = MOS_ALIGN_CEIL(m_jpegPicParams->m_frameWidth, CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE_X4);
            heightAlign = MOS_ALIGN_CEIL(m_jpegPicParams->m_frameHeight, CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE);
            break;
        default:  // YUV422H_4Y, YUV422V_4Y & YUV420
            widthAlign  = MOS_ALIGN_CEIL(m_jpegPicParams->m_frameWidth, CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE_X2);
            heightAlign = MOS_ALIGN_CEIL(m_jpegPicParams->m_frameHeight, CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE_X2);
            break;
        }
    }

    if ((m_jpegPicParams->m_rotation == jpegRotation90) || (m_jpegPicParams->m_rotation == jpegRotation270))
    {
        // Interchanging picture width and height for 90/270 degree rotation
        m_destSurface.dwWidth  = heightAlign;
        m_destSurface.dwHeight = widthAlign;
    }
    else
    {
        m_destSurface.dwWidth  = widthAlign;
        m_destSurface.dwHeight = heightAlign;
    }

    return MOS_STATUS_SUCCESS;
}
}  // namespace decode
