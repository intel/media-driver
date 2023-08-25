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
//! \file     encode_jpeg_packer_feature.cpp
//! \brief    Defines header packing logic for avc encode
//!

#include "encode_jpeg_packer_feature.h"
#include "encode_utils.h"

namespace encode
{

JpegPackerFeature::JpegPackerFeature(
    MediaFeatureManager *featureManager,
    EncodeAllocator *    allocator,
    CodechalHwInterfaceNext *hwInterface,
    void *               constSettings) : 
    MediaFeature(constSettings)
{
    m_featureManager = featureManager;
}

MOS_STATUS JpegPackerFeature::PackSOI(BSBuffer *buffer)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(buffer);
    // Add SOI = 0xFFD8
    buffer->pBase = (uint8_t *)MOS_AllocAndZeroMemory(2);
    ENCODE_CHK_NULL_RETURN(buffer->pBase);

    *(buffer->pBase)     = (m_jpegEncodeSoi >> 8) & 0xFF;
    *(buffer->pBase + 1) = (m_jpegEncodeSoi & 0xFF);
    buffer->BitOffset    = 0;
    buffer->BufferSize   = 16;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPackerFeature::PackApplicationData(
    BSBuffer *buffer,
    uint8_t  *appDataChunk,
    uint32_t size)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(buffer);
    ENCODE_CHK_NULL_RETURN(appDataChunk);

    buffer->pBase      = appDataChunk;
    buffer->BitOffset  = 0;
    buffer->BufferSize = (size * sizeof(uint8_t) * 8);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPackerFeature::PackQuantTable(
    BSBuffer *          buffer,
    CodecJpegComponents componentType)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(buffer);

    auto basicFeature = dynamic_cast<JpegBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);

    EncodeJpegQuantHeader *quantHeader = (EncodeJpegQuantHeader *)MOS_AllocAndZeroMemory(sizeof(EncodeJpegQuantHeader));
    ENCODE_CHK_NULL_RETURN(quantHeader);

    quantHeader->m_dqt = 0xDBFF;
    // Header size including marker
    uint16_t hdrSize = sizeof(uint16_t) * 2 + sizeof(uint8_t) + sizeof(uint8_t) * JPEG_NUM_QUANTMATRIX;
    quantHeader->m_lq = (((hdrSize - 2) & 0xFF) << 8) | (((hdrSize - 2) & 0xFF00) >> 8);

    quantHeader->m_tablePrecisionAndDestination = ((basicFeature->m_jpegQuantTables->m_quantTable[componentType].m_precision & 0xF) << 4) | (componentType & 0xF);

    for (auto i = 0; i < JPEG_NUM_QUANTMATRIX; i++)
    {
        quantHeader->m_qk[i] = (uint8_t)basicFeature->m_jpegQuantTables->m_quantTable[componentType].m_qm[i];
    }

    buffer->pBase      = (uint8_t *)quantHeader;
    buffer->BitOffset  = 0;
    buffer->BufferSize = hdrSize * 8;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPackerFeature::PackFrameHeader(
    BSBuffer *buffer,
    bool      useSingleDefaultQuantTable)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(buffer);

    JpegBasicFeature *jpegBasicFeature = dynamic_cast<JpegBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(jpegBasicFeature);

    EncodeJpegFrameHeader *frameHeader = (EncodeJpegFrameHeader *)MOS_AllocAndZeroMemory(sizeof(EncodeJpegFrameHeader));
    ENCODE_CHK_NULL_RETURN(frameHeader);

    auto jpegPicParams = jpegBasicFeature->m_jpegPicParams;

    frameHeader->m_sof = 0xC0FF;
    frameHeader->m_nf  = (uint8_t)jpegPicParams->m_numComponent;

    // Calculate lf - switch btes to match with simulation
    uint16_t value    = 8 + (3 * frameHeader->m_nf);  // does not include sof
    frameHeader->m_lf = ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);

    frameHeader->m_p = 8;
    frameHeader->m_y = ((jpegPicParams->m_picHeight & 0xFF) << 8) | ((jpegPicParams->m_picHeight & 0xFF00) >> 8);
    frameHeader->m_x = ((jpegPicParams->m_picWidth & 0xFF) << 8) | ((jpegPicParams->m_picWidth & 0xFF00) >> 8);

    for (uint8_t i = 0; i < frameHeader->m_nf; i++)
    {
        frameHeader->m_codechalJpegFrameComponent[i].m_ci = (uint8_t)jpegPicParams->m_componentID[i];

        if (useSingleDefaultQuantTable)
        {
            frameHeader->m_codechalJpegFrameComponent[i].m_tqi = 0;  // 0/1/2 based on Y/U/V
        }
        else
        {
            frameHeader->m_codechalJpegFrameComponent[i].m_tqi = i;  // 0/1/2 based on Y/U/V
        }

        // For all supported formats on JPEG encode, U and V vertical and horizontal sampling is 1
        uint32_t horizontalSamplingFactor = 0, verticalSamplingFactor = 0;
        if (i == 0)
        {
            horizontalSamplingFactor = jpegBasicFeature->GetJpegHorizontalSamplingFactorForY((CodecEncodeJpegInputSurfaceFormat)jpegPicParams->m_inputSurfaceFormat);
            verticalSamplingFactor   = jpegBasicFeature->GetJpegVerticalSamplingFactorForY((CodecEncodeJpegInputSurfaceFormat)jpegPicParams->m_inputSurfaceFormat);
        }
        else
        {
            horizontalSamplingFactor = 1;
            verticalSamplingFactor   = 1;
        }

        frameHeader->m_codechalJpegFrameComponent[i].m_samplingFactori = 0;
        frameHeader->m_codechalJpegFrameComponent[i].m_samplingFactori = ((horizontalSamplingFactor & 0xF) << 4) | (verticalSamplingFactor & 0xF);
    }

    buffer->pBase      = (uint8_t *)frameHeader;
    buffer->BitOffset  = 0;
    buffer->BufferSize = (2 * sizeof(uint8_t) * 8) + (4 * sizeof(uint16_t) * 8) + (sizeof(frameHeader->m_codechalJpegFrameComponent[0]) * 8 * jpegPicParams->m_numComponent);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPackerFeature::PackHuffmanTable(
    BSBuffer *buffer,
    uint32_t  tableIndex)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(buffer);

    auto basicFeature = dynamic_cast<JpegBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);

    EncodeJpegHuffmanHeader *huffmanHeader = (EncodeJpegHuffmanHeader *)MOS_AllocAndZeroMemory(sizeof(EncodeJpegHuffmanHeader));
    ENCODE_CHK_NULL_RETURN(huffmanHeader);

    huffmanHeader->m_dht = 0xC4FF;
    huffmanHeader->m_tableClassAndDestn =
        ((basicFeature->m_jpegHuffmanTable->m_huffmanData[tableIndex].m_tableClass & 0xF) << 4) | ((tableIndex / 2) & 0xF);

    uint16_t totalHuffValues = 0;
    for (auto i = 0; i < JPEG_NUM_HUFF_TABLE_AC_BITS; i++)
    {
        huffmanHeader->m_li[i] = (uint8_t)basicFeature->m_jpegHuffmanTable->m_huffmanData[tableIndex].m_bits[i];
        totalHuffValues += huffmanHeader->m_li[i];
    }

    uint16_t hdrSize    = 19 + totalHuffValues;
    huffmanHeader->m_lh = ((hdrSize & 0xFF) << 8) | ((hdrSize & 0xFF00) >> 8);

    for (auto i = 0; i < totalHuffValues; i++)
    {
        huffmanHeader->m_vij[i] = (uint8_t)basicFeature->m_jpegHuffmanTable->m_huffmanData[tableIndex].m_huffVal[i];
    }

    buffer->pBase     = (uint8_t *)huffmanHeader;
    buffer->BitOffset = 0;
    buffer->BufferSize = (2 * sizeof(uint16_t) * 8) + (sizeof(uint8_t) * 8) + (JPEG_NUM_HUFF_TABLE_AC_BITS * 8) + (totalHuffValues * sizeof(uint8_t) * 8);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPackerFeature::PackRestartInterval(
    BSBuffer *buffer)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(buffer);

    auto basicFeature = dynamic_cast<JpegBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);

    EncodeJpegRestartHeader *restartHeader = (EncodeJpegRestartHeader *)MOS_AllocAndZeroMemory(sizeof(EncodeJpegRestartHeader));
    ENCODE_CHK_NULL_RETURN(restartHeader);

    restartHeader->m_dri = 0xDDFF;
    uint16_t hdrSize     = sizeof(uint16_t) * 3;
    restartHeader->m_lr  = (((hdrSize - 2) & 0xFF) << 8) | (((hdrSize - 2) & 0xFF00) >> 8);
    restartHeader->m_ri  = (uint16_t)(((basicFeature->m_jpegScanParams->m_restartInterval & 0xFF) << 8) |
                                     ((basicFeature->m_jpegScanParams->m_restartInterval & 0xFF00) >> 8));

    buffer->pBase      = (uint8_t *)restartHeader;
    buffer->BitOffset  = 0;
    buffer->BufferSize = hdrSize * 8;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPackerFeature::PackScanHeader(
    BSBuffer *buffer)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(buffer);

    JpegBasicFeature *jpegBasicFeature = dynamic_cast<JpegBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(jpegBasicFeature);
    auto jpegPicParams = jpegBasicFeature->m_jpegPicParams;

    // Size of Scan header in bytes = sos (2 bytes) + ls (2 bytes) + ns (1 byte)
    // + ss (1 byte) + se (1 byte) + ahl (1 byte) + scanComponent (2 bytes) * Number of scan components
    uint16_t hdrSize = 8 + 2 * jpegPicParams->m_numComponent;

    uint8_t *scanHeader = (uint8_t *)MOS_AllocAndZeroMemory(hdrSize);
    ENCODE_CHK_NULL_RETURN(scanHeader);

    buffer->pBase = (uint8_t *)scanHeader;

    // scanHeader->sos
    *scanHeader = (m_jpegEncodeSos >> 8) & 0xFF;
    scanHeader += 1;
    *scanHeader = (m_jpegEncodeSos & 0xFF);
    scanHeader += 1;

    // scanHeader->ls
    *scanHeader = ((hdrSize - 2) >> 8) & 0xFF;
    scanHeader += 1;
    *scanHeader = (hdrSize - 2) & 0xFF;
    scanHeader += 1;

    // scanHeader->ns
    *scanHeader = (uint8_t)jpegPicParams->m_numComponent;
    scanHeader += 1;

    for (uint32_t j = 0; j < jpegPicParams->m_numComponent; j++)
    {
        *scanHeader = (uint8_t)jpegPicParams->m_componentID[j];
        scanHeader += 1;

        // For Y8 image format there is only one scan component, so scanComponent[1] and scanComponent[2] should not be added to the header
        // scanHeader->scanComponent[j].Tdaj
        if (j == 0)
        {
            *scanHeader = (uint8_t)(((jpegBasicFeature->m_jpegHuffmanTable->m_huffmanData[0].m_tableID & 0x0F) << 4) |
                                    ((jpegBasicFeature->m_jpegHuffmanTable->m_huffmanData[1].m_tableID & 0x0F)));
            scanHeader += 1;
        }
        else
        {
            *scanHeader = (uint8_t)(((jpegBasicFeature->m_jpegHuffmanTable->m_huffmanData[2].m_tableID & 0x0F) << 4) |
                                    ((jpegBasicFeature->m_jpegHuffmanTable->m_huffmanData[3].m_tableID & 0x0F)));
            scanHeader += 1;
        }
    }

    // scanHeader->ss
    *scanHeader = 0;
    scanHeader += 1;
    // scanHeader->se
    *scanHeader = 63;
    scanHeader += 1;
    // scanHeader->ahl
    *scanHeader = 0;
    scanHeader += 1;

    buffer->BitOffset = 0;
    // Buffer size in bits
    buffer->BufferSize = hdrSize * 8;

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
