/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     media_ddi_encode_jpeg.cpp
//! \brief    Defines class for DDI media jpeg encode.
//!

#include "media_libva_encoder.h"
#include "media_libva_util.h"
#include "media_ddi_encode_jpeg.h"
#include "media_ddi_encode_const.h"
#include "media_ddi_factory.h"

extern template class MediaDdiFactoryNoArg<DdiEncodeBase>;
static bool isEncodeJpegRegistered =
    MediaDdiFactoryNoArg<DdiEncodeBase>::RegisterCodec<DdiEncodeJpeg>(ENCODE_ID_JPEG);

DdiEncodeJpeg::~DdiEncodeJpeg()
{
    if (m_encodeCtx == nullptr)
    {
        return;
    }

    MOS_FreeMemory(m_encodeCtx->pPicParams);
    m_encodeCtx->pPicParams = nullptr;

    MOS_FreeMemory(m_encodeCtx->pEncodeStatusReport);
    m_encodeCtx->pEncodeStatusReport = nullptr;

    MOS_FreeMemory(m_huffmanTable);
    m_huffmanTable = nullptr;

    MOS_FreeMemory(m_encodeCtx->pQmatrixParams);
    m_encodeCtx->pQmatrixParams = nullptr;

    MOS_FreeMemory(m_encodeCtx->pSliceParams);
    m_encodeCtx->pSliceParams = nullptr;

    MOS_FreeMemory(m_encodeCtx->pbsBuffer);
    m_encodeCtx->pbsBuffer = nullptr;

    MOS_FreeMemory(m_appData);
    m_appData = nullptr;
}

VAStatus DdiEncodeJpeg::ContextInitialize(CodechalSetting *codecHalSettings)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_encodeCtx->pCpDdiInterface, "nullptr m_encodeCtx->pCpDdiInterface.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(codecHalSettings, "nullptr codecHalSettings.", VA_STATUS_ERROR_INVALID_PARAMETER);

    codecHalSettings->codecFunction = CODECHAL_FUNCTION_PAK;
    codecHalSettings->width       = m_encodeCtx->dwFrameWidth;
    codecHalSettings->height      = m_encodeCtx->dwFrameHeight;
    codecHalSettings->mode          = m_encodeCtx->wModeType;
    codecHalSettings->standard      = CODECHAL_JPEG;

    VAStatus vaStatus = VA_STATUS_SUCCESS;

    m_quantSupplied                 = false;
    m_appDataSize      = 0;
    m_appDataTotalSize = 0;
    m_appDataWholeHeader  = false;

    m_encodeCtx->pPicParams = (void *)MOS_AllocAndZeroMemory(sizeof(CodecEncodeJpegPictureParams));
    DDI_CHK_NULL(m_encodeCtx->pPicParams, "nullptr m_encodeCtx->pPicParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pbsBuffer = (BSBuffer *)MOS_AllocAndZeroMemory(sizeof(BSBuffer));
    DDI_CHK_NULL(m_encodeCtx->pbsBuffer, "nullptr m_encodeCtx->pbsBuffer.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Allocate Encode Status Report
    m_encodeCtx->pEncodeStatusReport = (void *)MOS_AllocAndZeroMemory(CODECHAL_ENCODE_STATUS_NUM * sizeof(EncodeStatusReport));
    DDI_CHK_NULL(m_encodeCtx->pEncodeStatusReport, "nullptr m_encodeCtx->pEncodeStatusReport.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // for scan header from application
    m_encodeCtx->pSliceParams = (void *)MOS_AllocAndZeroMemory(sizeof(CodecEncodeJpegScanHeader));
    DDI_CHK_NULL(m_encodeCtx->pSliceParams, "nullptr m_encodeCtx->pSliceParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // for Quant table
    m_encodeCtx->pQmatrixParams = (void *)MOS_AllocAndZeroMemory(sizeof(CodecEncodeJpegQuantTable));
    DDI_CHK_NULL(m_encodeCtx->pQmatrixParams, "nullptr m_encodeCtx->pQmatrixParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // for pHuffmanTable
    m_huffmanTable = (CodecEncodeJpegHuffmanDataArray *)MOS_AllocAndZeroMemory(sizeof(CodecEncodeJpegHuffmanDataArray));
    DDI_CHK_NULL(m_huffmanTable, "nullptr m_huffmanTable.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    return vaStatus;
}

VAStatus DdiEncodeJpeg::RenderPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VABufferID       *buffers,
    int32_t          numBuffers)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr context", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_MEDIA_CONTEXT *mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    for (int32_t i = 0; i < numBuffers; i++)
    {
        DDI_MEDIA_BUFFER *buf = DdiMedia_GetBufferFromVABufferID(mediaCtx, buffers[i]);
        DDI_CHK_NULL(buf, "Invalid buffer.", VA_STATUS_ERROR_INVALID_BUFFER);
        if (buf->uiType == VAEncMacroblockDisableSkipMapBufferType)
        {
            DdiMedia_MediaBufferToMosResource(buf, &(m_encodeCtx->resPerMBSkipMapBuffer));
            m_encodeCtx->bMbDisableSkipMapEnabled = true;
            continue;
        }
        uint32_t dataSize = buf->iSize;
        // can use internal function instead of DdiMedia_MapBuffer here?
        void *data = nullptr;
        DdiMedia_MapBuffer(ctx, buffers[i], &data);
        DDI_CHK_NULL(data, "nullptr data.", VA_STATUS_ERROR_INVALID_BUFFER);

        switch (buf->uiType)
        {
        case VAIQMatrixBufferType:
        case VAQMatrixBufferType:
            DDI_CHK_STATUS(Qmatrix(data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncPictureParameterBufferType:
            DDI_CHK_STATUS(ParsePicParams(mediaCtx, data), VA_STATUS_ERROR_INVALID_BUFFER);
            DDI_CHK_STATUS(
                    AddToStatusReportQueue((void *)m_encodeCtx->resBitstreamBuffer.bo),
                    VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncSliceParameterBufferType:
        {
            uint32_t numSlices = buf->uiNumElements;
            DDI_CHK_STATUS(ParseSlcParams(mediaCtx, data, numSlices), VA_STATUS_ERROR_INVALID_BUFFER);
            break;
        }

        case VAEncPackedHeaderParameterBufferType:
            if ((*((int32_t *)data) == VAEncPackedHeaderRawData) || (*((int32_t *)data) == VA_ENC_PACKED_HEADER_MISC))
            {
                m_appDataSize = (((VAEncPackedHeaderParameterBuffer *)data)->bit_length + 7) >> 3;
            }
            else
            {
                vaStatus = VA_STATUS_ERROR_INVALID_BUFFER;
            }
            break;

        case VAEncPackedHeaderDataBufferType:
            {
                uint8_t *tmpAppData = (uint8_t *)data;
                //by default m_appDataWholeHeader is false, it means it only include headers between 0xFFE0 to 0xFFEF;
                //follow JPEG spec definition of application segment definition
                //if the packed header is start with 0xFFD8, a new SOI, it should include whole jpeg headers
                if((tmpAppData[0] == 0xFF) && (tmpAppData[1] == 0xD8))
                {
                    m_appDataWholeHeader = true;
                }
                if(m_appDataWholeHeader)
                {
                    vaStatus = ParseAppData(data,m_appDataSize);
                }
                else
                {
                    vaStatus = ParseAppData(data, buf->iSize);
                }
                m_appDataSize = 0;
            }
            break;

        case VAHuffmanTableBufferType:
            DDI_CHK_STATUS(ParseHuffmanParams(data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncQPBufferType:
            DdiMedia_MediaBufferToMosResource(buf, &m_encodeCtx->resMBQpBuffer);
            m_encodeCtx->bMBQpEnable = true;
            break;

        default:
            DDI_ASSERTMESSAGE("not supported buffer type.");
            break;
        }
        DdiMedia_UnmapBuffer(ctx, buffers[i]);
    }

    DDI_FUNCTION_EXIT(vaStatus);
    return vaStatus;
}

// reset the parameters before each frame
VAStatus DdiEncodeJpeg::ResetAtFrameLevel()
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);

    // Set the render target format
    CodecEncodeJpegPictureParams *picParams = (CodecEncodeJpegPictureParams *)m_encodeCtx->pPicParams;
    DDI_CHK_NULL(picParams, "nullptr picParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    picParams->m_inputSurfaceFormat = ConvertMediaFormatToInputSurfaceFormat(m_encodeCtx->RTtbl.pCurrentRT->format);

    m_appDataSize = 0;
    m_appDataTotalSize = 0;
    m_appDataWholeHeader = false;
    m_quantSupplied = false;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeJpeg::ParsePicParams(DDI_MEDIA_CONTEXT *mediaCtx, void *ptr)
{
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncPictureParameterBufferJPEG *picParams = (VAEncPictureParameterBufferJPEG *)ptr;

    CodecEncodeJpegPictureParams *jpegPicParams = (CodecEncodeJpegPictureParams *)m_encodeCtx->pPicParams;
    DDI_CHK_NULL(jpegPicParams, "nullptr jpegPicParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (jpegPicParams->m_inputSurfaceFormat == DDI_ENCODE_JPEG_INPUTFORMAT_RESERVED)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    DDI_MEDIA_BUFFER *buf = DdiMedia_GetBufferFromVABufferID(mediaCtx, picParams->coded_buf);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);
    RemoveFromStatusReportQueue(buf);
    DdiMedia_MediaBufferToMosResource(buf, &(m_encodeCtx->resBitstreamBuffer));

    jpegPicParams->m_profile      = picParams->pic_flags.bits.profile;
    jpegPicParams->m_progressive  = picParams->pic_flags.bits.progressive;
    jpegPicParams->m_huffman      = picParams->pic_flags.bits.huffman;
    jpegPicParams->m_interleaved  = picParams->pic_flags.bits.interleaved;
    jpegPicParams->m_differential = picParams->pic_flags.bits.differential;

    jpegPicParams->m_picWidth                   = picParams->picture_width;
    jpegPicParams->m_picHeight                  = picParams->picture_height;
    jpegPicParams->m_sampleBitDepth             = picParams->sample_bit_depth;
    jpegPicParams->m_numComponent               = picParams->num_components;
    jpegPicParams->m_quality                    = picParams->quality;
    jpegPicParams->m_numScan                    = picParams->num_scan;
    jpegPicParams->m_statusReportFeedbackNumber = 1;

    for (int32_t i = 0; i < jpegNumComponent; i++)
    {
        jpegPicParams->m_componentID[i]        = picParams->component_id[i];
        jpegPicParams->m_quantTableSelector[i] = picParams->quantiser_table_selector[i];
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeJpeg::ParseSlcParams(DDI_MEDIA_CONTEXT *mediaCtx, void *ptr, uint32_t numSlices)
{
    DDI_UNUSED(mediaCtx);

    if (numSlices != 1)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_huffmanTable, "nullptr m_huffmanTable", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncSliceParameterBufferJPEG *scanParams = (VAEncSliceParameterBufferJPEG *)ptr;

    CodecEncodeJpegScanHeader *scanData = (CodecEncodeJpegScanHeader *)m_encodeCtx->pSliceParams;
    DDI_CHK_NULL(scanData, "nullptr scanData", VA_STATUS_ERROR_INVALID_PARAMETER);

    m_encodeCtx->dwNumSlices = numSlices;

    // Only 1 scan is supported for JPEG
    scanData->m_restartInterval = scanParams->restart_interval;
    scanData->m_numComponent    = scanParams->num_components;

    for (int32_t componentCount = 0; componentCount < jpegNumComponent; componentCount++)
    {
        scanData->m_componentSelector[componentCount]   = scanParams->components[componentCount].component_selector;
        scanData->m_dcCodingTblSelector[componentCount] = scanParams->components[componentCount].dc_table_selector;
        scanData->m_acCodingTblSelector[componentCount] = scanParams->components[componentCount].ac_table_selector;

        // AC and DC table selectors always have the same value for android
        m_huffmanTable->m_huffmanData[componentCount].m_tableID = scanData->m_dcCodingTblSelector[componentCount];
    }

    // Table ID for DC table for luma
    m_huffmanTable->m_huffmanData[0].m_tableID = scanData->m_dcCodingTblSelector[0];

    //Table ID for AC table for luma
    m_huffmanTable->m_huffmanData[1].m_tableID = scanData->m_acCodingTblSelector[0];

    // Table ID for DC table for chroma
    m_huffmanTable->m_huffmanData[2].m_tableID = scanData->m_dcCodingTblSelector[1];

    // Table ID for AC table for chroma
    m_huffmanTable->m_huffmanData[3].m_tableID = scanData->m_dcCodingTblSelector[1];

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeJpeg::Qmatrix(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAQMatrixBufferJPEG *quantParams = (VAQMatrixBufferJPEG *)ptr;

    CodecEncodeJpegQuantTable *quantMatrix = (CodecEncodeJpegQuantTable *)m_encodeCtx->pQmatrixParams;
    DDI_CHK_NULL(quantMatrix, "nullptr quantMatrix", VA_STATUS_ERROR_INVALID_PARAMETER);

    // Setting number of Quantization tables in Picture Params
    CodecEncodeJpegPictureParams *picParams = (CodecEncodeJpegPictureParams *)m_encodeCtx->pPicParams;
    DDI_CHK_NULL(picParams, "nullptr picParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    picParams->m_numQuantTable = 0;

    if (quantParams->load_lum_quantiser_matrix == 1)
    {
        quantMatrix->m_quantTable[0].m_precision = 0;  // only 8 bit precision is supported
        quantMatrix->m_quantTable[0].m_tableID   = 0;  // tableID is 0 for luma

        picParams->m_numQuantTable++;

        for (int32_t i = 0; i < quantMatrixSize; i++)
        {
            quantMatrix->m_quantTable[0].m_qm[i] = quantParams->lum_quantiser_matrix[i] & 0xFF;
        }
    }
    else  // no luma quantization table present - invalid argument
    {
        // switch to default quantization table
        m_quantSupplied = false;
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (quantParams->load_chroma_quantiser_matrix == 1)
    {
        quantMatrix->m_quantTable[1].m_precision = 0;  // only 8 bit precision is supported
        quantMatrix->m_quantTable[1].m_tableID   = 1;  // table ID is 1 and 2 for U and V

        picParams->m_numQuantTable++;

        for (int32_t i = 0; i < quantMatrixSize; i++)
        {
            quantMatrix->m_quantTable[1].m_qm[i] = quantParams->chroma_quantiser_matrix[i] & 0xFF;
        }
    }

    m_quantSupplied = true;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeJpeg::ParseHuffmanParams(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_huffmanTable, "nullptr m_huffmanTable", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAHuffmanTableBufferJPEGBaseline *params = (VAHuffmanTableBufferJPEGBaseline *)ptr;

    // Setting number of Huffman tables in Picture Params
    CodecEncodeJpegPictureParams *picParams = (CodecEncodeJpegPictureParams *)m_encodeCtx->pPicParams;
    DDI_CHK_NULL(picParams, "nullptr picParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    // For setting the tableIDs
    CodecEncodeJpegScanHeader *scanData = (CodecEncodeJpegScanHeader *)m_encodeCtx->pSliceParams;
    DDI_CHK_NULL(scanData, "nullptr scanData", VA_STATUS_ERROR_INVALID_PARAMETER);

    picParams->m_numCodingTable = 0;

    uint32_t numHuffBuffers = 0;  // To check how many Huffman buffers the app sends

    // Max number of huffman tables that can be sent by the app is 2
    for (int32_t tblCount = 0; tblCount < maxNumHuffTables; tblCount++)
    {
        if (params->load_huffman_table[tblCount] != 0)
        {
            numHuffBuffers++;

            // first copy DC table
            m_huffmanTable->m_huffmanData[tblCount * 2].m_tableClass = 0;
            m_huffmanTable->m_huffmanData[tblCount * 2].m_tableID    = scanData->m_dcCodingTblSelector[tblCount];

            for (int32_t i = 0; i < JPEG_NUM_HUFF_TABLE_DC_BITS; i++)
            {
                m_huffmanTable->m_huffmanData[tblCount * 2].m_bits[i] = params->huffman_table[tblCount].num_dc_codes[i] & 0xFF;
            }
            for (int32_t i = 0; i < JPEG_NUM_HUFF_TABLE_DC_HUFFVAL; i++)
            {
                m_huffmanTable->m_huffmanData[tblCount * 2].m_huffVal[i] = params->huffman_table[tblCount].dc_values[i] & 0xFF;
            }

            // Now copy AC table
            m_huffmanTable->m_huffmanData[(tblCount * 2) + 1].m_tableClass = 1;
            m_huffmanTable->m_huffmanData[(tblCount * 2) + 1].m_tableID    = scanData->m_acCodingTblSelector[tblCount];

            for (int32_t i = 0; i < JPEG_NUM_HUFF_TABLE_AC_BITS; i++)
            {
                m_huffmanTable->m_huffmanData[(tblCount * 2) + 1].m_bits[i] = params->huffman_table[tblCount].num_ac_codes[i] & 0xFF;
            }
            for (int32_t i = 0; i < JPEG_NUM_HUFF_TABLE_AC_HUFFVAL; i++)
            {
                m_huffmanTable->m_huffmanData[(tblCount * 2) + 1].m_huffVal[i] = params->huffman_table[tblCount].ac_values[i] & 0xFF;
            }
        }
    }

    if (numHuffBuffers > (JPEG_NUM_ENCODE_HUFF_BUFF / 2))
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    // Multiplying with 2 because each table contains both AC and DC buffers
    picParams->m_numCodingTable += numHuffBuffers * 2;

    return VA_STATUS_SUCCESS;
};

VAStatus DdiEncodeJpeg::ParseAppData(void *ptr, int32_t size)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx.", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr.", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t prevAppDataSize = m_appDataTotalSize;

    if (m_appData == nullptr)
    {
        m_appData = (void *)MOS_AllocAndZeroMemory(size);

        if (!m_appData)
        {
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }

        MOS_SecureMemcpy(m_appData, size, ptr, size);
    }
    else  // app data had been sent before
    {
        void *tempAppData = (void *)MOS_AllocAndZeroMemory(size + (int32_t)prevAppDataSize);

        if (nullptr == tempAppData)
        {
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }

        // Copy over previous app data to a new location
        MOS_SecureMemcpy(tempAppData, prevAppDataSize, (uint8_t *)m_appData, prevAppDataSize);

        uint8_t *newAddress = (uint8_t *)tempAppData + prevAppDataSize;

        // Add new app data buffer to the new location
        MOS_SecureMemcpy(newAddress, size, (uint8_t *)ptr, size);

        // Now free the previous location containing app data and overwrite with new app data buffer
        MOS_FreeMemory(m_appData);
        m_appData = tempAppData;

    }

    m_appDataTotalSize += size;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeJpeg::EncodeInCodecHal(uint32_t numSlices)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_encodeCtx->pCodecHal, "nullptr m_encodeCtx->pCodecHal", VA_STATUS_ERROR_INVALID_CONTEXT);

    if (numSlices != 1)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_encodeCtx->RTtbl);

    CodecEncodeJpegPictureParams *picParams = (CodecEncodeJpegPictureParams *)(m_encodeCtx->pPicParams);

    CodecEncodeJpegScanHeader *scanData = (CodecEncodeJpegScanHeader *)m_encodeCtx->pSliceParams;

    EncoderParams encodeParams;
    MOS_ZeroMemory(&encodeParams, sizeof(EncoderParams));
    encodeParams.ExecCodecFunction = CODECHAL_FUNCTION_PAK;

    // If app sends whole header - driver must extract QMatrix from it and ignore QMatrix buffer
    // If app sends only QMatrix - driver must use it without scaling
    // otherwise - driver must scale default QMatrix
    DDI_NORMALMESSAGE("Func %s: m_appDataWholeHeader: %u, m_quantSupplied: %d", __FUNCTION__, m_appDataWholeHeader, m_quantSupplied);
    if (m_appDataWholeHeader)
    {
        DDI_CHK_RET(QmatrixFromHeader(), "Failed to parse Quant Matrix from header");
    }
    else if (!m_quantSupplied)
    {
        DDI_CHK_RET(DefaultQmatrix(), "Failed to load default Quant Matrix");
    }

    // Raw Surface
    MOS_SURFACE rawSurface;
    MOS_ZeroMemory(&rawSurface, sizeof(MOS_SURFACE));
    rawSurface.Format   = (MOS_FORMAT)picParams->m_inputSurfaceFormat;
    rawSurface.dwOffset = 0;

    DdiMedia_MediaSurfaceToMosResource(rtTbl->pCurrentRT, &(rawSurface.OsResource));

    encodeParams.bJpegQuantMatrixSent = m_quantSupplied;

    // Bitstream surface
    MOS_RESOURCE bitstreamSurface;
    MOS_ZeroMemory(&bitstreamSurface, sizeof(MOS_RESOURCE));
    bitstreamSurface        = m_encodeCtx->resBitstreamBuffer;  // in render picture
    bitstreamSurface.Format = Format_Buffer;

    encodeParams.psRawSurface        = &rawSurface;
    encodeParams.psReconSurface      = nullptr;
    encodeParams.presBitstreamBuffer = &bitstreamSurface;

    encodeParams.pPicParams       = m_encodeCtx->pPicParams;
    encodeParams.pSliceParams     = m_encodeCtx->pSliceParams;
    encodeParams.pApplicationData = m_appData;

    // Slice level data
    encodeParams.dwNumSlices      = numSlices;
    encodeParams.dwNumHuffBuffers = picParams->m_numCodingTable;
    encodeParams.dwAppDataSize    = m_appDataTotalSize;
    encodeParams.fullHeaderInAppData = m_appDataWholeHeader;

    encodeParams.pQuantizationTable = m_encodeCtx->pQmatrixParams;
    encodeParams.pHuffmanTable      = m_huffmanTable;
    encodeParams.pBSBuffer          = m_encodeCtx->pbsBuffer;
    encodeParams.pSlcHeaderData     = (void *)m_encodeCtx->pSliceHeaderData;

    if (scanData->m_numComponent == 1)  // Y8 input format
    {
        // Take the first table sent by the app
        encodeParams.dwNumHuffBuffers = 2;
    }

    MOS_STATUS status = m_encodeCtx->pCodecHal->Execute(&encodeParams);
    if (MOS_STATUS_SUCCESS != status)
    {
        return VA_STATUS_ERROR_ENCODING_ERROR;
    }

    return VA_STATUS_SUCCESS;
}

uint32_t DdiEncodeJpeg::ConvertMediaFormatToInputSurfaceFormat(DDI_MEDIA_FORMAT format)
{
    switch (format)
    {
    case Media_Format_NV12:
        return DDI_ENCODE_JPEG_INPUTFORMAT_NV12;
    case Media_Format_422H:
    case Media_Format_422V:
    case Media_Format_UYVY:
        return DDI_ENCODE_JPEG_INPUTFORMAT_UYVY;
    case Media_Format_YUY2:
        return DDI_ENCODE_JPEG_INPUTFORMAT_YUY2;
    case Media_Format_400P:
        return DDI_ENCODE_JPEG_INPUTFORMAT_Y8;
    case Media_Format_X8R8G8B8:
    case Media_Format_A8R8G8B8:
    case Media_Format_X8B8G8R8:
    case Media_Format_R8G8B8A8:
    case Media_Format_A8B8G8R8:
    case Media_Format_444P:
        return DDI_ENCODE_JPEG_INPUTFORMAT_RGB;
    default:
        return DDI_ENCODE_JPEG_INPUTFORMAT_RESERVED;
    }
}

VAStatus DdiEncodeJpeg::DefaultQmatrix()
{
    DDI_FUNCTION_ENTER();
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);

    CodecEncodeJpegQuantTable *quantMatrix = (CodecEncodeJpegQuantTable *)m_encodeCtx->pQmatrixParams;
    DDI_CHK_NULL(quantMatrix, "nullptr quantMatrix", VA_STATUS_ERROR_INVALID_PARAMETER);

    // To get Quality from Pic Params
    CodecEncodeJpegPictureParams *picParams = (CodecEncodeJpegPictureParams *)m_encodeCtx->pPicParams;
    DDI_CHK_NULL(picParams, "nullptr picParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t quality = 0;
    if (picParams->m_quality < 50)
    {
        if(picParams->m_quality == 0)
        {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        quality = 5000 / picParams->m_quality;
    }
    else
    {
        quality = 200 - (picParams->m_quality * 2);
    }

    // 2 tables - one for luma and one for chroma
    for (int32_t qMatrixCount = 0; qMatrixCount < maxNumQuantTableIndex; qMatrixCount++)
    {
        quantMatrix->m_quantTable[qMatrixCount].m_precision = 0;
        quantMatrix->m_quantTable[qMatrixCount].m_tableID   = qMatrixCount;

        for (int32_t i = 0; i < quantMatrixSize; i++)
        {
            uint32_t quantValue = 0;
            if (qMatrixCount == 0)
            {
                quantValue = (defaultLumaQuant[i] * quality + 50) / 100;
            }
            else
            {
                quantValue = (defaultChromaQuant[i] * quality + 50) / 100;
            }

            // Clamp the value to range between 1 and 255
            if (quantValue < 1)
            {
                quantValue = 1;
            }
            else if (quantValue > 255)
            {
                quantValue = 255;
            }

            quantMatrix->m_quantTable[qMatrixCount].m_qm[i] = (uint16_t)quantValue;
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeJpeg::QmatrixFromHeader()
{
    DDI_FUNCTION_ENTER();
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_appData, "nullptr m_appData", VA_STATUS_ERROR_INVALID_PARAMETER);

    CodecEncodeJpegQuantTable *quantMatrix = (CodecEncodeJpegQuantTable *)m_encodeCtx->pQmatrixParams;
    DDI_CHK_NULL(quantMatrix, "nullptr quantMatrix", VA_STATUS_ERROR_INVALID_PARAMETER);

    // To get Quality from Pic Params
    CodecEncodeJpegPictureParams *picParams = (CodecEncodeJpegPictureParams *)m_encodeCtx->pPicParams;
    DDI_CHK_NULL(picParams, "nullptr picParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    picParams->m_numQuantTable = 0; // Will be extracted from header
    m_quantSupplied = false;

    const uint8_t hdrStartCode    = 0xFF;
    const int32_t startCodeBlockSize = 2;
    const int32_t qTblLengthBlockSize = 2;
    const int32_t qTblPrecisionTypeBlockSize = 1;
    uint8_t *appQTblData    = ((uint8_t*)m_appData);
    uint8_t *appQTblDataEnd = ((uint8_t*)m_appData) + m_appDataTotalSize;
    while (appQTblData = (uint8_t*)memchr(appQTblData, hdrStartCode, appQTblDataEnd - appQTblData))
    {
        if (appQTblDataEnd - appQTblData < startCodeBlockSize + qTblLengthBlockSize) // Don't find header start code or tbl length
            break;

        if (appQTblData[1] == 0xDA) // "Start of scan" 2nd start code
        {
            break;
        }
        else if (appQTblData[1] != 0xDB) // Not "Define Quantization Table" 2nd start code
        {
            appQTblData += startCodeBlockSize;
            continue;
        }

        // Handle "Define Quantization Table"
        // Structure: [16Bits start code][16Bits length]{ [4Bits precision][4Bits type][8 * 64Bits QTable] } * num_quant_tables
        //
        // Note: qTableLength include
        //     - 16Bits for length block size 
        //     - { 8 bits for Precision&Type + (8 * 64Bits) for QTable } * num_quant_tables
        int32_t qTableLength = (appQTblData[2] << 8) + appQTblData[3];
        appQTblData += startCodeBlockSize;

        int32_t  numQMatrix = (qTableLength - qTblLengthBlockSize) / (qTblPrecisionTypeBlockSize + quantMatrixSize);
        DDI_CHK_CONDITION(appQTblData + qTableLength > appQTblDataEnd, "Table length is greater than jpeg header", VA_STATUS_ERROR_INVALID_PARAMETER);
        DDI_CHK_CONDITION((qTableLength - qTblLengthBlockSize) % (qTblPrecisionTypeBlockSize + quantMatrixSize),
            "Invalid qtable length. It must contain 2Bytes for qtable length and (1Byte for Precision&Type + 64Bytes for QTable) * num_qtables", VA_STATUS_ERROR_INVALID_PARAMETER);

        appQTblData += qTblLengthBlockSize;
        for (int qMatrixIdx = 0; qMatrixIdx < numQMatrix; ++qMatrixIdx)
        {
            uint8_t  tablePrecision   =  appQTblData[0] >> 4;
            uint8_t  tableType        =  appQTblData[0] & 0xF;
            appQTblData++;
            DDI_CHK_CONDITION(tableType >= JPEG_MAX_NUM_QUANT_TABLE_INDEX, "QTable type is equal or greater than JPEG_MAX_NUM_QUANT_TABLE (3)", VA_STATUS_ERROR_INVALID_PARAMETER);

            picParams->m_numQuantTable++;
            quantMatrix->m_quantTable[tableType].m_precision = tablePrecision;
            quantMatrix->m_quantTable[tableType].m_tableID   = tableType;
            for (int32_t j = 0; j < quantMatrixSize; ++appQTblData, ++j)
                quantMatrix->m_quantTable[tableType].m_qm[j] = *appQTblData;

            if (picParams->m_numQuantTable == JPEG_MAX_NUM_QUANT_TABLE_INDEX) // Max 3 QTables for encode
                break;
        }
    }
    DDI_CHK_CONDITION(picParams->m_numQuantTable == 0, "No quant tables were found in header", VA_STATUS_ERROR_INVALID_PARAMETER);

    m_quantSupplied = true;
    return VA_STATUS_SUCCESS;
}

uint32_t DdiEncodeJpeg::getSliceParameterBufferSize()
{
    return sizeof(VAEncSliceParameterBufferJPEG);
}

uint32_t DdiEncodeJpeg::getPictureParameterBufferSize()
{
    return sizeof(VAEncPictureParameterBufferJPEG);
}

uint32_t DdiEncodeJpeg::getQMatrixBufferSize()
{
    return sizeof(VAQMatrixBufferJPEG);
}
