/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file     media_ddi_decode_jpeg.cpp
//! \brief    The class implementation of DdiDecodeJPEG  for JPEG decode
//!
//!

#include <va/va_dec_jpeg.h>
#include "media_libva_decoder.h"
#include "media_libva_util.h"
#include "media_ddi_decode_jpeg.h"
#include "mos_solo_generic.h"
#include "codechal_decode_jpeg.h"
#include "media_ddi_decode_const.h"
#include "media_ddi_factory.h"

typedef enum _DDI_DECODE_JPEG_BUFFER_STATE
{
    BUFFER_UNLOADED = 0,
    BUFFER_LOADED   = 1,
} DDI_DECODE_JPEG_BUFFER_STATE;

#define DDI_DECODE_JPEG_MAXIMUM_HUFFMAN_TABLE    2
#define DDI_DECODE_JPEG_MAXIMUM_QMATRIX_NUM      4
#define DDI_DECODE_JPEG_MAXIMUM_QMATRIX_ENTRIES  64

#define DDI_DECODE_JPEG_SLICE_PARAM_BUF_NUM      0x4     //According to JPEG SPEC, max slice per frame is 4

static const uint32_t zigzag_order[64] =
{
    0,   1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

VAStatus DdiDecodeJPEG::ParseSliceParams(
    DDI_MEDIA_CONTEXT                   *mediaCtx,
    VASliceParameterBufferJPEGBaseline  *slcParam,
    uint32_t                             numSlices)
{
    CodecDecodeJpegScanParameter *jpegSliceParam =
        (CodecDecodeJpegScanParameter *)(m_ddiDecodeCtx->DecodeParams.m_sliceParams);

    CodecDecodeJpegPicParams *picParam = (CodecDecodeJpegPicParams *)(m_ddiDecodeCtx->DecodeParams.m_picParams);

    if ((jpegSliceParam == nullptr) ||
        (picParam == nullptr) ||
        (slcParam == nullptr))
    {
        DDI_ASSERTMESSAGE("Invalid Parameter for Parsing JPEG Slice parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    jpegSliceParam->NumScans += numSlices;
    picParam->m_totalScans += numSlices;
    if (picParam->m_totalScans == 1 && slcParam[0].num_components > 1)
    {
        picParam->m_interleavedData = 1;
    }

    uint32_t j, i;
    int32_t startIdx = m_numScans;
    for (j = 0; j < numSlices; j++)
    {
        for (i = 0; i < slcParam[j].num_components; i++)
        {
            jpegSliceParam->ScanHeader[j + startIdx].ComponentSelector[i] = slcParam[j].components[i].component_selector;
            jpegSliceParam->ScanHeader[j + startIdx].DcHuffTblSelector[i] = slcParam[j].components[i].dc_table_selector;
            jpegSliceParam->ScanHeader[j + startIdx].AcHuffTblSelector[i] = slcParam[j].components[i].ac_table_selector;
        }
        jpegSliceParam->ScanHeader[j + startIdx].NumComponents    = slcParam[j].num_components;
        jpegSliceParam->ScanHeader[j + startIdx].RestartInterval  = slcParam[j].restart_interval;
        jpegSliceParam->ScanHeader[j + startIdx].MCUCount         = slcParam[j].num_mcus;
        jpegSliceParam->ScanHeader[j + startIdx].ScanHoriPosition = slcParam[j].slice_horizontal_position;
        jpegSliceParam->ScanHeader[j + startIdx].ScanVertPosition = slcParam[j].slice_vertical_position;
        jpegSliceParam->ScanHeader[j + startIdx].DataOffset       = slcParam[j].slice_data_offset;
        jpegSliceParam->ScanHeader[j + startIdx].DataLength       = slcParam[j].slice_data_size;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeJPEG::ParsePicParams(
    DDI_MEDIA_CONTEXT                    *mediaCtx,
    VAPictureParameterBufferJPEGBaseline *picParam)
{
    CodecDecodeJpegPicParams *jpegPicParam = (CodecDecodeJpegPicParams *)(m_ddiDecodeCtx->DecodeParams.m_picParams);

    if ((jpegPicParam == nullptr) ||
        (picParam == nullptr))
    {
        DDI_ASSERTMESSAGE("Null Parameter for Parsing JPEG Picture parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    jpegPicParam->m_frameWidth     = picParam->picture_width;
    jpegPicParam->m_frameHeight    = picParam->picture_height;
    jpegPicParam->m_numCompInFrame = picParam->num_components;

    switch (picParam->rotation)
    {
    case VA_ROTATION_NONE:
        jpegPicParam->m_rotation = jpegRotation0;
        break;
    case VA_ROTATION_90:
        jpegPicParam->m_rotation = jpegRotation90;
        break;
    case VA_ROTATION_180:
        jpegPicParam->m_rotation = jpegRotation180;
        break;
    case VA_ROTATION_270:
        jpegPicParam->m_rotation = jpegRotation270;
        break;
    default:
        /* For the other rotation type, the rotation is disabled. */
        jpegPicParam->m_rotation = jpegRotation0;
        break;
        ;
    }

    if (jpegPicParam->m_numCompInFrame == 1)
    {
        jpegPicParam->m_chromaType = jpegYUV400;
    }
    else if (jpegPicParam->m_numCompInFrame == 3)
    {
        int32_t h1 = picParam->components[0].h_sampling_factor;
        int32_t h2 = picParam->components[1].h_sampling_factor;
        int32_t h3 = picParam->components[2].h_sampling_factor;
        int32_t v1 = picParam->components[0].v_sampling_factor;
        int32_t v2 = picParam->components[1].v_sampling_factor;
        int32_t v3 = picParam->components[2].v_sampling_factor;

        if (h1 == 2 && h2 == 1 && h3 == 1 &&
            v1 == 2 && v2 == 1 && v3 == 1)
        {
            jpegPicParam->m_chromaType = jpegYUV420;
        }
        else if (h1 == 2 && h2 == 1 && h3 == 1 &&
                 v1 == 1 && v2 == 1 && v3 == 1)
        {
            jpegPicParam->m_chromaType = jpegYUV422H2Y;
        }
        else if (h1 == 1 && h2 == 1 && h3 == 1 &&
                 v1 == 1 && v2 == 1 && v3 == 1)
        {
            switch (picParam->color_space)
            {
            case 0:  //YUV
                jpegPicParam->m_chromaType = jpegYUV444;
                break;
            case 1:  //RGB
                jpegPicParam->m_chromaType = jpegRGB;
                break;
            case 2:  //BGR
                jpegPicParam->m_chromaType = jpegBGR;
                break;
            default:
                /* For the other type, the default YUV444 is used */
                jpegPicParam->m_chromaType = jpegYUV444;
                break;
                ;
            }
        }
        else if (h1 == 4 && h2 == 1 && h3 == 1 &&
                 v1 == 1 && v2 == 1 && v3 == 1)
        {
            jpegPicParam->m_chromaType = jpegYUV411;
        }
        else if (h1 == 1 && h2 == 1 && h3 == 1 &&
                 v1 == 2 && v2 == 1 && v3 == 1)
        {
            jpegPicParam->m_chromaType = jpegYUV422V2Y;
        }
        else if (h1 == 2 && h2 == 1 && h3 == 1 &&
                 v1 == 2 && v2 == 2 && v3 == 2)
        {
            jpegPicParam->m_chromaType = jpegYUV422H4Y;
        }
        else if (h1 == 2 && h2 == 2 && h3 == 2 &&
                 v1 == 2 && v2 == 1 && v3 == 1)
        {
            jpegPicParam->m_chromaType = jpegYUV422V4Y;
        }
        else
        {
            DDI_NORMALMESSAGE("Unsupported sampling factor in JPEG Picture  parameter\n");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
    }

    memset(jpegPicParam->m_componentIdentifier, 0, jpegNumComponent);
    memset(jpegPicParam->m_quantTableSelector, 0, jpegNumComponent);

    if (picParam->num_components > jpegNumComponent)
    {
        DDI_NORMALMESSAGE("Unsupported component num in JPEG Picture  parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    for (int32_t i = 0; i < picParam->num_components; i++)
    {
        jpegPicParam->m_componentIdentifier[i] = picParam->components[i].component_id;
        jpegPicParam->m_quantTableSelector[i]  = picParam->components[i].quantiser_table_selector;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeJPEG::ParseHuffmanTbl(
    DDI_MEDIA_CONTEXT *               mediaCtx,
    VAHuffmanTableBufferJPEGBaseline *huffmanTbl)
{
    PCODECHAL_DECODE_JPEG_HUFFMAN_TABLE jpegHuffTbl = (PCODECHAL_DECODE_JPEG_HUFFMAN_TABLE)(m_ddiDecodeCtx->DecodeParams.m_huffmanTable);

    if ((jpegHuffTbl == nullptr) ||
        (huffmanTbl == nullptr))
    {
        DDI_ASSERTMESSAGE("Null Parameter for Parsing JPEG Huffman Tableparameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    memset(jpegHuffTbl, 0, sizeof(CODECHAL_DECODE_JPEG_HUFFMAN_TABLE));

    for (int32_t i = 0; i < DDI_DECODE_JPEG_MAXIMUM_HUFFMAN_TABLE; i++)
    {
        if (huffmanTbl->load_huffman_table[i] == BUFFER_LOADED)
        {
            //the size of jpegHuffTbl->HuffTable[i].DC_BITS is 12 (defined in driver)
            //the size of huffmanTbl->huffman_table[i].num_dc_codes is 16 (defined in libva)
            //it is using the size of "DC_BITS" for solve the overflow
            MOS_SecureMemcpy(jpegHuffTbl->HuffTable[i].DC_BITS,
                sizeof(jpegHuffTbl->HuffTable[i].DC_BITS),
                huffmanTbl->huffman_table[i].num_dc_codes,
                sizeof(jpegHuffTbl->HuffTable[i].DC_BITS));
            MOS_SecureMemcpy(jpegHuffTbl->HuffTable[i].DC_HUFFVAL,
                sizeof(jpegHuffTbl->HuffTable[i].DC_HUFFVAL),
                huffmanTbl->huffman_table[i].dc_values,
                sizeof(huffmanTbl->huffman_table[i].dc_values));
            MOS_SecureMemcpy(jpegHuffTbl->HuffTable[i].AC_BITS,
                sizeof(jpegHuffTbl->HuffTable[i].AC_BITS),
                huffmanTbl->huffman_table[i].num_ac_codes,
                sizeof(huffmanTbl->huffman_table[i].num_ac_codes));
            MOS_SecureMemcpy(jpegHuffTbl->HuffTable[i].AC_HUFFVAL,
                sizeof(jpegHuffTbl->HuffTable[i].AC_HUFFVAL),
                huffmanTbl->huffman_table[i].ac_values,
                sizeof(huffmanTbl->huffman_table[i].ac_values));
        }
    }

    return VA_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//   Function:    JpegQMatrixDecode
//   Description: Parse the QMatrix table from VAAPI, and load the valid Qmatrix to the Buffer used by
//                    CodecHal
//
/////////////////////////////////////////////////////////////////////////////////////////
VAStatus DdiDecodeJPEG::ParseIQMatrix(
        DDI_MEDIA_CONTEXT            *mediaCtx,
        VAIQMatrixBufferJPEGBaseline *matrix)
{
    CodecJpegQuantMatrix *jpegQMatrix = (CodecJpegQuantMatrix *)(m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer);

    if ((matrix == nullptr) || (jpegQMatrix == nullptr))
    {
        DDI_ASSERTMESSAGE("Null Parameter for Parsing JPEG IQMatrix \n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    memset(jpegQMatrix, 0, sizeof(CodecJpegQuantMatrix));

    int32_t idx, idx2;
    for (idx = 0; idx < DDI_DECODE_JPEG_MAXIMUM_QMATRIX_NUM; idx++)
    {
        if (matrix->load_quantiser_table[idx] == BUFFER_LOADED)
        {
            for (idx2 = 0; idx2 < DDI_DECODE_JPEG_MAXIMUM_QMATRIX_ENTRIES; idx2++)
            {
                jpegQMatrix->m_quantMatrix[idx][zigzag_order[idx2]] = matrix->quantiser_table[idx][idx2];
            }
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeJPEG::SetBufferRendered(VABufferID bufferID)
{
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_ddiDecodeCtx->BufMgr);

    if (bufMgr == nullptr)
    {
        DDI_ASSERTMESSAGE("Null Parameter for Parsing Data buffer for JPEG\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    bool renderFlag = false;
    for (uint32_t i = 0; i < bufMgr->dwNumSliceData; i++)
    {
        // Depend on the ID we tracked, if application want to rendered one of them
        // we set some flags
        if (bufMgr->pSliceData[i].vaBufferId == bufferID)
        {
            if (bufMgr->pSliceData[i].bRendered == false)
            {
                // set the rendered flags. which will be used when EndPicture.
                bufMgr->pSliceData[i].bRendered = true;
                // calculate the size of a bunch of rendered buffers. for endpicture to allocate appropriate memory size of GPU.
                bufMgr->dwSizeOfRenderedSliceData += bufMgr->pSliceData[i].uiLength;
                // keep record the render order, so that we can calculate the offset correctly when endpicture.
                // in this array we save the buffer index in pSliceData which render in sequence. if application create buffers like: 4,3,5,1,2.
                // but only render 2,3,5. the content in this array is: [4,1,2], which is the index of created buffer.
                bufMgr->pRenderedOrder[bufMgr->dwNumOfRenderedSliceData] = i;
                bufMgr->dwNumOfRenderedSliceData++;
            }
            renderFlag = true;
            break;
        }
    }

    if (renderFlag)
    {
        return VA_STATUS_SUCCESS;
    }
    else
    {
        return VA_STATUS_ERROR_INVALID_BUFFER;
    }
}

VAStatus DdiDecodeJPEG::RenderPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VABufferID       *buffers,
    int32_t          numBuffers)
{
    DDI_FUNCTION_ENTER();

    VAStatus           va = VA_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);

    void              *data = nullptr;
    for (int32_t i = 0; i < numBuffers; i++)
    {
        if (!buffers || (buffers[i] == VA_INVALID_ID))
        {
            return VA_STATUS_ERROR_INVALID_BUFFER;
        }

        DDI_MEDIA_BUFFER *buf = DdiMedia_GetBufferFromVABufferID(mediaCtx, buffers[i]);
        if (nullptr == buf)
        {
            return VA_STATUS_ERROR_INVALID_BUFFER;
        }

        uint32_t dataSize = buf->iSize;
        DdiMedia_MapBuffer(ctx, buffers[i], &data);

        if (data == nullptr)
        {
            return VA_STATUS_ERROR_INVALID_BUFFER;
        }

        switch ((int32_t)buf->uiType)
        {
        case VASliceDataBufferType:
        {
            DDI_CHK_RET(SetBufferRendered(buffers[i]),"SetBufferRendered failed!");
            m_ddiDecodeCtx->DecodeParams.m_dataSize += dataSize;
            break;
        }
        case VASliceParameterBufferType:
        {
            if (buf->uiNumElements == 0)
            {
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }

            uint32_t numSlices = buf->uiNumElements;

            if ((m_numScans + numSlices) > jpegNumComponent)
            {
                DDI_NORMALMESSAGE("the total number of JPEG scans are beyond the supported num(4)\n");
                return VA_STATUS_ERROR_INVALID_PARAMETER;
            }
            DDI_CHK_RET(AllocSliceParamContext(numSlices),"AllocSliceParamContext failed!");
            VASliceParameterBufferJPEGBaseline *slcInfo = (VASliceParameterBufferJPEGBaseline *)data;
            DDI_CHK_RET(ParseSliceParams(mediaCtx, slcInfo, numSlices),"ParseSliceParams failed!");
            m_ddiDecodeCtx->BufMgr.pNumOfRenderedSliceParaForOneBuffer[m_ddiDecodeCtx->BufMgr.dwNumOfRenderedSlicePara] = numSlices;
            m_ddiDecodeCtx->BufMgr.dwNumOfRenderedSlicePara ++;

            m_ddiDecodeCtx->DecodeParams.m_numSlices += numSlices;
            m_numScans += numSlices;
            m_groupIndex++;
            break;
        }
        case VAIQMatrixBufferType:
        {
            VAIQMatrixBufferJPEGBaseline *imxBuf = (VAIQMatrixBufferJPEGBaseline *)data;
            DDI_CHK_RET(ParseIQMatrix(mediaCtx, imxBuf),"ParseIQMatrix failed!");
            break;
        }
        case VAPictureParameterBufferType:
        {
            VAPictureParameterBufferJPEGBaseline *picParam = (VAPictureParameterBufferJPEGBaseline *)data;
            DDI_CHK_RET(ParsePicParams(mediaCtx, picParam),"ParsePicParams failed!");
            break;
        }
        case VAHuffmanTableBufferType:
        {
            VAHuffmanTableBufferJPEGBaseline *huffTbl = (VAHuffmanTableBufferJPEGBaseline *)data;
            DDI_CHK_RET(ParseHuffmanTbl(mediaCtx, huffTbl),"ParseHuffmanTbl failed!");
            break;
        }
        case VAProcPipelineParameterBufferType:
        {
            DDI_NORMALMESSAGE("ProcPipeline is not supported for JPEGBaseline decoding\n");
            break;
        }
        case VADecodeStreamoutBufferType:
        {
            DdiMedia_MediaBufferToMosResource(buf, &m_ddiDecodeCtx->BufMgr.resExternalStreamOutBuffer);
            m_streamOutEnabled = true;
            break;
        }
        default:
            va = VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
            break;
        }
        DdiMedia_UnmapBuffer(ctx, buffers[i]);
    }

    DDI_FUNCTION_EXIT(va);
    return va;
}

VAStatus DdiDecodeJPEG::BeginPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VASurfaceID      renderTarget)
{
    VAStatus vaStatus = DdiMediaDecode::BeginPicture(ctx, context, renderTarget);

    if (vaStatus != VA_STATUS_SUCCESS)
    {
        return vaStatus;
    }

    if (m_jpegBitstreamBuf)
    {
        DdiMediaUtil_FreeBuffer(m_jpegBitstreamBuf);
        MOS_FreeMemory(m_jpegBitstreamBuf);
        m_jpegBitstreamBuf = nullptr;
    }

    CodecDecodeJpegScanParameter *jpegSliceParam =
        (CodecDecodeJpegScanParameter *)(m_ddiDecodeCtx->DecodeParams.m_sliceParams);
    jpegSliceParam->NumScans = 0;

    CodecDecodeJpegPicParams *picParam = (CodecDecodeJpegPicParams *)(m_ddiDecodeCtx->DecodeParams.m_picParams);
    picParam->m_totalScans             = 0;

    m_numScans = 0;
    return vaStatus;
}

VAStatus DdiDecodeJPEG::InitDecodeParams(
    VADriverContextP ctx,
    VAContextID      context)
{
    /* skip the mediaCtx check as it is checked in caller */
    PDDI_MEDIA_CONTEXT mediaCtx;
    mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_RET(DecodeCombineBitstream(mediaCtx),"DecodeCombineBitstream failed!");
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_ddiDecodeCtx->BufMgr);
    bufMgr->dwNumSliceControl    = 0;
    memset(&m_destSurface, 0, sizeof(MOS_SURFACE));
    m_destSurface.dwOffset = 0;
    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_ddiDecodeCtx->RTtbl);

    if ((rtTbl == nullptr) || (rtTbl->pCurrentRT == nullptr))
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeJPEG::SetDecodeParams()
{
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_ddiDecodeCtx->BufMgr);

    // we do not support mismatched usecase.
    if ((bufMgr->dwNumOfRenderedSlicePara != bufMgr->dwNumOfRenderedSliceData) ||
        (bufMgr->dwNumOfRenderedSlicePara == 0))
    {
        DDI_NORMALMESSAGE("DDI: Unsupported buffer mismatch usage!\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    // Allocate GPU Buffer description keeper.
    m_jpegBitstreamBuf = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
    if (m_jpegBitstreamBuf == nullptr)
    {
        DDI_ASSERTMESSAGE("DDI: Allocate  Jpeg Media Buffer Failed!\n");
        return VA_STATUS_ERROR_UNKNOWN;
    }

    m_jpegBitstreamBuf->iSize         = bufMgr->dwSizeOfRenderedSliceData;
    m_jpegBitstreamBuf->uiNumElements = bufMgr->dwNumOfRenderedSliceData;
    m_jpegBitstreamBuf->uiType        = VASliceDataBufferType;
    m_jpegBitstreamBuf->format        = Media_Format_Buffer;
    m_jpegBitstreamBuf->uiOffset      = 0;
    m_jpegBitstreamBuf->bCFlushReq    = false;
    m_jpegBitstreamBuf->pMediaCtx     = m_ddiDecodeCtx->pMediaCtx;

    // Create GPU buffer
    VAStatus va  = DdiMediaUtil_CreateBuffer(m_jpegBitstreamBuf, m_ddiDecodeCtx->pMediaCtx->pDrmBufMgr);
    if (va  != VA_STATUS_SUCCESS)
    {
        DdiMediaUtil_FreeBuffer(m_jpegBitstreamBuf);
        MOS_FreeMemory(m_jpegBitstreamBuf);
        m_jpegBitstreamBuf = nullptr;
        return va;
    }

    // For the first time you call DdiMediaUtil_LockBuffer for a fresh GPU memory, it will map GPU address to a virtual address.
    // and then, DdiMediaUtil_LockBuffer is acutally to increase the reference count. so we used here for 2 resaons:
    //
    // 1. Map GPU address to virtual at the beginning when we combine the CPU -> GPU.
    uint8_t *mappedBuf = (uint8_t *)DdiMediaUtil_LockBuffer(m_jpegBitstreamBuf, MOS_LOCKFLAG_WRITEONLY);

    if (mappedBuf == nullptr)
    {
        DdiMediaUtil_FreeBuffer(m_jpegBitstreamBuf);
        MOS_FreeMemory(m_jpegBitstreamBuf);
        m_jpegBitstreamBuf = nullptr;
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    // get the JPEG Slice Header Params for offset recaculated.
    CodecDecodeJpegScanParameter *sliceParam =
        (CodecDecodeJpegScanParameter *)(m_ddiDecodeCtx->DecodeParams.m_sliceParams);

    uint32_t bufOffset      = 0;
    int32_t  orderSlicePara = 0;
    int32_t  orderSliceData = 0;
    for (uint32_t i = 0; i < bufMgr->dwNumOfRenderedSliceData; i++)
    {
        // get the rendered slice data index in rendered order.
        int32_t renderedBufIdx = bufMgr->pRenderedOrder[i];
        if (bufMgr->pSliceData[renderedBufIdx].bRendered)
        {
            MOS_SecureMemcpy((void *)(mappedBuf + bufOffset),
                bufMgr->pSliceData[renderedBufIdx].uiLength,
                bufMgr->pSliceData[renderedBufIdx].pBaseAddress,
                bufMgr->pSliceData[renderedBufIdx].uiLength);

            // since we assume application must make sure ONE slice parameter buffer ONE slice data buffer, so we recaculate header offset here.
            for (int32_t j = 0; j < bufMgr->pNumOfRenderedSliceParaForOneBuffer[orderSliceData]; j++)
            {
                sliceParam->ScanHeader[orderSlicePara].DataOffset += bufOffset;
                orderSlicePara++;
            }
            bufOffset += bufMgr->pSliceData[renderedBufIdx].uiLength;
            bufMgr->pNumOfRenderedSliceParaForOneBuffer[orderSliceData] = 0;
            orderSliceData++;
            bufMgr->pSliceData[renderedBufIdx].bRendered = false;
        }
    }
    DdiMediaUtil_UnlockBuffer(m_jpegBitstreamBuf);
    DdiMedia_MediaBufferToMosResource(m_jpegBitstreamBuf, &(bufMgr->resBitstreamBuffer));
    bufMgr->dwNumOfRenderedSliceData  = 0;
    bufMgr->dwNumOfRenderedSlicePara  = 0;
    bufMgr->dwSizeOfRenderedSliceData = 0;

    m_destSurface.dwOffset = 0;
    m_destSurface.Format   = Format_NV12;

    CodecDecodeJpegPicParams *jpegPicParam = (CodecDecodeJpegPicParams *)(m_ddiDecodeCtx->DecodeParams.m_picParams);
    if((m_ddiDecodeCtx->RTtbl.pCurrentRT->format == Media_Format_NV12)
        &&(jpegPicParam->m_chromaType == jpegYUV444))
    {
        m_ddiDecodeCtx->RTtbl.pCurrentRT = DdiMedia_ReplaceSurfaceWithNewFormat(m_ddiDecodeCtx->RTtbl.pCurrentRT, Media_Format_444P);
    }
    if(m_ddiDecodeCtx->RTtbl.pCurrentRT != nullptr)
    {
        DdiMedia_MediaSurfaceToMosResource((&(m_ddiDecodeCtx->RTtbl))->pCurrentRT, &(m_destSurface.OsResource));
    }

    (&m_ddiDecodeCtx->DecodeParams)->m_destSurface = &m_destSurface;

    (&m_ddiDecodeCtx->DecodeParams)->m_deblockSurface = nullptr;

    (&m_ddiDecodeCtx->DecodeParams)->m_dataBuffer       = &bufMgr->resBitstreamBuffer;
    (&m_ddiDecodeCtx->DecodeParams)->m_bitStreamBufData = bufMgr->pBitstreamBuffer;
    Mos_Solo_OverrideBufferSize((&m_ddiDecodeCtx->DecodeParams)->m_dataSize, (&m_ddiDecodeCtx->DecodeParams)->m_dataBuffer);

    (&m_ddiDecodeCtx->DecodeParams)->m_bitplaneBuffer = nullptr;

    if (m_streamOutEnabled)
    {
        (&m_ddiDecodeCtx->DecodeParams)->m_streamOutEnabled        = true;
        (&m_ddiDecodeCtx->DecodeParams)->m_externalStreamOutBuffer = &bufMgr->resExternalStreamOutBuffer;
    }
    else
    {
        (&m_ddiDecodeCtx->DecodeParams)->m_streamOutEnabled        = false;
        (&m_ddiDecodeCtx->DecodeParams)->m_externalStreamOutBuffer = nullptr;
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeJPEG::AllocSliceParamContext(
    uint32_t numSlices)
{
    uint32_t baseSize = sizeof(CodecDecodeJpegScanParameter);

    if (m_sliceParamBufNum < (m_ddiDecodeCtx->DecodeParams.m_numSlices + numSlices))
    {
        // in order to avoid that the buffer is reallocated multi-times,
        // extra 10 slices are added.
        uint32_t extraSlices = numSlices + 10;

        m_ddiDecodeCtx->DecodeParams.m_sliceParams = realloc(m_ddiDecodeCtx->DecodeParams.m_sliceParams,
            baseSize * (m_sliceParamBufNum + extraSlices));

        if (m_ddiDecodeCtx->DecodeParams.m_sliceParams == nullptr)
        {
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }

        memset((void *)((uint8_t *)m_ddiDecodeCtx->DecodeParams.m_sliceParams + baseSize * m_sliceParamBufNum), 0, baseSize * extraSlices);
        m_sliceParamBufNum += extraSlices;
    }

    return VA_STATUS_SUCCESS;
}

void DdiDecodeJPEG::DestroyContext(
    VADriverContextP ctx)
{
    FreeResourceBuffer();
    // explicitly call the base function to do the further clean-up
    DdiMediaDecode::DestroyContext(ctx);
}

uint8_t* DdiDecodeJPEG::GetPicParamBuf(
    DDI_CODEC_COM_BUFFER_MGR    *bufMgr)
{
    return (uint8_t*)(&(bufMgr->Codec_Param.Codec_Param_JPEG.PicParamJPEG));
}

VAStatus DdiDecodeJPEG::AllocBsBuffer(
    DDI_CODEC_COM_BUFFER_MGR   *bufMgr,
    DDI_MEDIA_BUFFER           *buf)
{
    // Allocate JPEG slice data memory from CPU.
    uint8_t                   *bsAddr;
    uint32_t                   index;

    index = bufMgr->dwNumSliceData;

    /* the pSliceData needs to be reallocated in order to contain more SliceDataBuf */
    if (index >= bufMgr->m_maxNumSliceData)
    {
        /* In theroy it can resize the m_maxNumSliceData one by one. But in order to
         * avoid calling realloc frequently, it will try to allocate 10 to  hold more
         * SliceDataBuf. This is only for the optimized purpose.
         */
        int32_t reallocSize = bufMgr->m_maxNumSliceData + 10;

        bufMgr->pSliceData = (DDI_CODEC_BITSTREAM_BUFFER_INFO *)realloc(bufMgr->pSliceData, sizeof(bufMgr->pSliceData[0]) * reallocSize);

        if (bufMgr->pSliceData == nullptr)
        {
            DDI_ASSERTMESSAGE("fail to reallocate pSliceData for JPEG\n.");
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        memset((void *)(bufMgr->pSliceData + bufMgr->m_maxNumSliceData), 0,
               sizeof(bufMgr->pSliceData[0]) * 10);

        bufMgr->m_maxNumSliceData += 10;
    }

    bsAddr = (uint8_t*)MOS_AllocAndZeroMemory(buf->iSize);
    if(bsAddr == 0)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    buf->pData                             = bsAddr;
    buf->format                            = Media_Format_CPU;
    buf->bCFlushReq                        = false;
    buf->uiOffset                          = 0;
    bufMgr->pSliceData[index].uiLength     = buf->iSize;
    bufMgr->pSliceData[index].uiOffset     = buf->uiOffset;
    bufMgr->pSliceData[index].pBaseAddress = buf->pData;
    bufMgr->dwNumSliceData ++;
    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeJPEG::AllocSliceControlBuffer(
    DDI_MEDIA_BUFFER       *buf)
{
    DDI_CODEC_COM_BUFFER_MGR   *bufMgr;
    uint32_t                    availSize;
    uint32_t                    newSize;

    bufMgr     = &(m_ddiDecodeCtx->BufMgr);
    availSize = m_sliceCtrlBufNum - bufMgr->dwNumSliceControl;

    if(availSize < buf->uiNumElements)
    {
        newSize   = sizeof(VASliceParameterBufferJPEGBaseline) * (m_sliceCtrlBufNum - availSize + buf->uiNumElements);
        bufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG = (VASliceParameterBufferJPEGBaseline *)realloc(bufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG, newSize);
        if(bufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG == nullptr)
        {
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        MOS_ZeroMemory(bufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG + m_sliceCtrlBufNum, sizeof(VASliceParameterBufferJPEGBaseline) * (buf->uiNumElements - availSize));
        m_sliceCtrlBufNum = m_sliceCtrlBufNum - availSize + buf->uiNumElements;
    }
    buf->pData      = (uint8_t*)bufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG;
    buf->uiOffset   = sizeof(VASliceParameterBufferJPEGBaseline) * bufMgr->dwNumSliceControl;

    bufMgr->dwNumSliceControl += buf->uiNumElements;

    return VA_STATUS_SUCCESS;
}

void DdiDecodeJPEG::ContextInit(
    int32_t picWidth,
    int32_t picHeight)
{
    // call the function in base class to initialize it.
    DdiMediaDecode::ContextInit(picWidth, picHeight);

    m_ddiDecodeCtx->wMode    = CODECHAL_DECODE_MODE_JPEG;
}

VAStatus DdiDecodeJPEG::InitResourceBuffer()
{
    VAStatus                  vaStatus = VA_STATUS_SUCCESS;
    DDI_CODEC_COM_BUFFER_MGR *bufMgr   = &(m_ddiDecodeCtx->BufMgr);
    bufMgr->pSliceData                 = nullptr;

    bufMgr->ui64BitstreamOrder = 0;
    bufMgr->dwMaxBsSize        = m_width *
                          m_height * 3 / 2;

    bufMgr->dwNumSliceData    = 0;
    bufMgr->dwNumSliceControl = 0;

    m_sliceCtrlBufNum = DDI_DECODE_JPEG_SLICE_PARAM_BUF_NUM;
    bufMgr->m_maxNumSliceData         = DDI_DECODE_JPEG_SLICE_PARAM_BUF_NUM;
    bufMgr->pSliceData                = (DDI_CODEC_BITSTREAM_BUFFER_INFO *)MOS_AllocAndZeroMemory(sizeof(bufMgr->pSliceData[0]) * DDI_DECODE_JPEG_SLICE_PARAM_BUF_NUM);
    if (bufMgr->pSliceData == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }
    bufMgr->dwNumOfRenderedSlicePara                         = 0;
    bufMgr->dwNumOfRenderedSliceData                         = 0;
    bufMgr->pNumOfRenderedSliceParaForOneBuffer              = (int32_t *)MOS_AllocAndZeroMemory(sizeof(bufMgr->pNumOfRenderedSliceParaForOneBuffer[0]) * m_sliceCtrlBufNum);
    bufMgr->pRenderedOrder                                   = (int32_t *)MOS_AllocAndZeroMemory(sizeof(bufMgr->pRenderedOrder[0]) * m_sliceCtrlBufNum);
    bufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG = (VASliceParameterBufferJPEGBaseline *)MOS_AllocAndZeroMemory(sizeof(VASliceParameterBufferJPEGBaseline) * m_sliceCtrlBufNum);
    if (bufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    return VA_STATUS_SUCCESS;

finish:
    FreeResourceBuffer();
    return vaStatus;
}

void DdiDecodeJPEG::FreeResourceBuffer()
{
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_ddiDecodeCtx->BufMgr);

    if (bufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG)
    {
        MOS_FreeMemory(bufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG);
        bufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG = nullptr;
    }
    bufMgr->dwNumOfRenderedSlicePara = 0;
    bufMgr->dwNumOfRenderedSliceData = 0;
    MOS_FreeMemory(bufMgr->pNumOfRenderedSliceParaForOneBuffer);
    bufMgr->pNumOfRenderedSliceParaForOneBuffer = nullptr;
    MOS_FreeMemory(bufMgr->pRenderedOrder);
    bufMgr->pRenderedOrder = nullptr;

    for (uint32_t i = 0; i < bufMgr->dwNumSliceData && (bufMgr->pSliceData != nullptr); i++)
    {
        if (bufMgr->pSliceData[i].pBaseAddress != nullptr)
        {
            MOS_FreeMemory(bufMgr->pSliceData[i].pBaseAddress);
            bufMgr->pSliceData[i].pBaseAddress = nullptr;
        }
    }

    bufMgr->dwNumSliceData    = 0;

    if (m_jpegBitstreamBuf)
    {
        DdiMediaUtil_FreeBuffer(m_jpegBitstreamBuf);
        MOS_FreeMemory(m_jpegBitstreamBuf);
        m_jpegBitstreamBuf = nullptr;
    }

    // free decode bitstream buffer object
    MOS_FreeMemory(bufMgr->pSliceData);
    bufMgr->pSliceData = nullptr;

    return;
}

VAStatus DdiDecodeJPEG::CodecHalInit(
    DDI_MEDIA_CONTEXT *mediaCtx,
    void              *ptr)
{
    VAStatus     vaStatus = VA_STATUS_SUCCESS;
    MOS_CONTEXT *mosCtx   = (MOS_CONTEXT *)ptr;

    CODECHAL_FUNCTION codecFunction = CODECHAL_FUNCTION_DECODE;
    m_ddiDecodeCtx->pCpDdiInterface->SetCpParams(m_ddiDecodeAttr->uiEncryptionType, m_codechalSettings);

    CODECHAL_STANDARD_INFO standardInfo;
    memset(&standardInfo, 0, sizeof(standardInfo));

    standardInfo.CodecFunction = codecFunction;
    standardInfo.Mode          = (CODECHAL_MODE)m_ddiDecodeCtx->wMode;

    m_codechalSettings->codecFunction                = codecFunction;
    m_codechalSettings->width                      = m_width;
    m_codechalSettings->height                     = m_height;
    m_codechalSettings->intelEntrypointInUse        = false;

    m_codechalSettings->lumaChromaDepth = CODECHAL_LUMA_CHROMA_DEPTH_8_BITS;

    m_codechalSettings->shortFormatInUse = m_ddiDecodeCtx->bShortFormatInUse;

    m_codechalSettings->mode     = CODECHAL_DECODE_MODE_JPEG;
    m_codechalSettings->standard = CODECHAL_JPEG;

#ifdef _DECODE_PROCESSING_SUPPORTED
    m_codechalSettings->sfcEnablingHinted = true;
#endif

    m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer = MOS_AllocAndZeroMemory(sizeof(CodecJpegQuantMatrix));
    if (m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }
    m_ddiDecodeCtx->DecodeParams.m_picParams = MOS_AllocAndZeroMemory(sizeof(CodecDecodeJpegPicParams));
    if (m_ddiDecodeCtx->DecodeParams.m_picParams == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }

    m_ddiDecodeCtx->DecodeParams.m_huffmanTable = (void*)MOS_AllocAndZeroMemory(sizeof(CODECHAL_DECODE_JPEG_HUFFMAN_TABLE));
    if (!m_ddiDecodeCtx->DecodeParams.m_huffmanTable)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }

    m_sliceParamBufNum         = DDI_DECODE_JPEG_SLICE_PARAM_BUF_NUM;
    m_ddiDecodeCtx->DecodeParams.m_sliceParams = (void *)MOS_AllocAndZeroMemory(m_sliceParamBufNum * sizeof(CodecDecodeJpegScanParameter));

    if (m_ddiDecodeCtx->DecodeParams.m_sliceParams == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }

    vaStatus = CreateCodecHal(mediaCtx,
        ptr,
        &standardInfo);

    if (vaStatus != VA_STATUS_SUCCESS)
    {
        goto CleanUpandReturn;
    }

    if (InitResourceBuffer() != VA_STATUS_SUCCESS)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }

    return vaStatus;

CleanUpandReturn:
    FreeResourceBuffer();

    if (m_ddiDecodeCtx->pCodecHal)
    {
        m_ddiDecodeCtx->pCodecHal->Destroy();
        MOS_Delete(m_ddiDecodeCtx->pCodecHal);
        m_ddiDecodeCtx->pCodecHal = nullptr;
    }

    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer);
    m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer = nullptr;
    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_picParams);
    m_ddiDecodeCtx->DecodeParams.m_picParams = nullptr;
    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_huffmanTable);
    m_ddiDecodeCtx->DecodeParams.m_huffmanTable = nullptr;
    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_sliceParams);
    m_ddiDecodeCtx->DecodeParams.m_sliceParams = nullptr;

    return vaStatus;
}

extern template class MediaDdiFactory<DdiMediaDecode, DDI_DECODE_CONFIG_ATTR>;

static bool jpegRegistered =
    MediaDdiFactory<DdiMediaDecode, DDI_DECODE_CONFIG_ATTR>::RegisterCodec<DdiDecodeJPEG>(DECODE_ID_JPEG);
