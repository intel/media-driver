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
//! \file     codechal_encode_jpeg.cpp
//! \brief    Defines state class for JPEG encoder.
//!

#include "codechal_encode_jpeg.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug.h"
#endif

MOS_STATUS CodechalEncodeJpegState::Initialize(CodechalSetting  *settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_miInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(settings);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::Initialize(settings));

    // Picture Level Commands
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_hwInterface->GetMfxStateCommandsDataSize(
            CODECHAL_ENCODE_MODE_JPEG,
            &m_pictureStatesSize,
            &m_picturePatchListSize,
            0));

    // Slice Level Commands (cannot be placed in 2nd level batch)
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_hwInterface->GetMfxPrimitiveCommandsDataSize(
            CODECHAL_ENCODE_MODE_JPEG,
            &m_sliceStatesSize,
            &m_slicePatchListSize,
            0));

    return eStatus;
}

MOS_STATUS CodechalEncodeJpegState::AllocateResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::AllocateResources());

    // Allocate Ref Lists
    CodecHalAllocateDataList(
        m_refList,
        CODECHAL_NUM_UNCOMPRESSED_SURFACE_JPEG);

    return eStatus;
}

void CodechalEncodeJpegState::FreeResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalEncoderState::FreeResources();

    // Release Ref Lists
    CodecHalFreeDataList(m_refList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_JPEG);
}

MOS_STATUS CodechalEncodeJpegState::InitializePicture(const EncoderParams& params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_bitstreamUpperBound = params.dwBitstreamSize;

    m_jpegPicParams         = (CodecEncodeJpegPictureParams *)(params.pPicParams);
    m_jpegScanParams        = (CodecEncodeJpegScanHeader *)(params.pSliceParams);
    m_jpegQuantTables       = (CodecEncodeJpegQuantTable *)(params.pQuantizationTable);
    m_jpegHuffmanTable      = (CodecEncodeJpegHuffmanDataArray *)(params.pHuffmanTable);
    m_applicationData       = params.pApplicationData;
    m_appDataSize           = params.dwAppDataSize;
    m_jpegQuantMatrixSent   = params.bJpegQuantMatrixSent;
    m_fullHeaderInAppData   = params.fullHeaderInAppData;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_jpegPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_jpegScanParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_jpegQuantTables);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_jpegHuffmanTable);

    // Set Status Report Feedback Number
    m_statusReportFeedbackNumber = m_jpegPicParams->m_statusReportFeedbackNumber;
    m_currRefList                = m_refList[m_currOriginalPic.FrameIdx];

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetStatusReportParams(m_refList[m_currOriginalPic.FrameIdx]));

    m_currRefList->resBitstreamBuffer = m_resBitstreamBuffer;
    m_currRefList->sRefRawBuffer      = m_rawSurface;

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_debugInterface);

         if (m_jpegPicParams)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpPicParams(
                m_jpegPicParams));
        }

        if (m_jpegScanParams)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpScanParams(
                m_jpegScanParams));
        }

        if (m_jpegHuffmanTable)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpHuffmanTable(
                m_jpegHuffmanTable));
        }

        if (m_jpegQuantTables)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpQuantTables(
                m_jpegQuantTables));
        }
    )

    return eStatus;
}

MOS_STATUS CodechalEncodeJpegState::CheckResChangeAndCsc()
{
    return MOS_STATUS_SUCCESS;
}

// Implemented based on table K.5 in JPEG spec
uint8_t CodechalEncodeJpegState::MapHuffValIndex(uint8_t huffValIndex)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t mappedIndex = 0;

    if (huffValIndex < 0xF0)
    {
        mappedIndex = (((huffValIndex >> 4) & 0x0F) * 0xA) + (huffValIndex & 0x0F);
    }
    else
    {
        mappedIndex = (((huffValIndex >> 4) & 0x0F) * 0xA) + (huffValIndex & 0x0F) + 1;
    }

    return mappedIndex;
}

// Implemented based on Flowchart in figure C.1 in JPEG spec
MOS_STATUS CodechalEncodeJpegState::GenerateSizeTable(
    uint8_t     bits[],
    uint8_t     huffSize[],
    uint8_t&    lastK)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint8_t i = 1, j = 1;
    uint8_t k = 0;
    while (i <= 16)
    {
        while (j <= (int8_t)bits[i - 1]) // bits index is from 0 to 15
        {
            huffSize[k] = i;
            k = k + 1;
            j = j + 1;
        }

        i++;
        j = 1;
    };

    huffSize[k] = 0;
    lastK = k;

    return eStatus;
}

// Implemented based on Flowchart in figure C.2 in JPEG spec
MOS_STATUS CodechalEncodeJpegState::GenerateCodeTable(
    uint8_t     huffSize[],
    uint16_t    huffCode[])
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint8_t    k = 0;
    uint8_t    si = huffSize[0];
    uint16_t   code = 0;
    while (huffSize[k] != 0)
    {
        while (huffSize[k] == si)
        {
            if (code == 0xFFFF)
            {
                // Invalid code generated - replace with all zeroes
                code = 0x0000;
            }

            huffCode[k] = code;
            code = code + 1;
            k = k + 1;
        };

        code <<= 1;
        si = si + 1;
    }

    return eStatus;
}

// Implemented based on Flowchart in figure C.3 in JPEG spec
MOS_STATUS CodechalEncodeJpegState::OrderCodes(
    uint8_t     huffVal[],
    uint8_t     huffSize[],
    uint16_t    huffCode[],
    uint8_t     lastK)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint16_t eHuffCo[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL];
    uint8_t  eHuffSi[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL];
    MOS_ZeroMemory(&eHuffCo[0], JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint16_t));
    MOS_ZeroMemory(&eHuffSi[0], JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint8_t));

    uint8_t k = 0;
    do
    {
        uint8_t i = MapHuffValIndex((uint8_t)huffVal[k]);
        if (i >= JPEG_NUM_HUFF_TABLE_AC_HUFFVAL)
        {
            CODECHAL_ENCODE_ASSERT(false);
            return MOS_STATUS_UNKNOWN;
        }
        eHuffCo[i] = huffCode[k];
        eHuffSi[i] = huffSize[k];
        k++;
    } while (k < lastK);

    // copy over the first 162 values of reordered arrays to Huffman Code and size arrays
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(&huffCode[0], JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint16_t), &eHuffCo[0], JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint16_t)));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(&huffSize[0], JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint8_t), &eHuffSi[0], JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint8_t)));

    return eStatus;
}

MOS_STATUS CodechalEncodeJpegState::ConvertHuffDataToTable(
    CodecEncodeJpegHuffData             huffmanData,
    CodechalEncodeJpegHuffTable         *huffmanTable)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    huffmanTable->m_tableClass = huffmanData.m_tableClass;
    huffmanTable->m_tableID    = huffmanData.m_tableID;

    uint8_t lastK = 0;

    // Step 1 : Generate size table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GenerateSizeTable(huffmanData.m_bits, huffmanTable->m_huffSize, lastK));

    // Step2: Generate code table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GenerateCodeTable(huffmanTable->m_huffSize, huffmanTable->m_huffCode));

    // Step 3: Order codes
    CODECHAL_ENCODE_CHK_STATUS_RETURN(OrderCodes(huffmanData.m_huffVal, huffmanTable->m_huffSize, huffmanTable->m_huffCode, lastK));

    return eStatus;
}

MOS_STATUS CodechalEncodeJpegState::PackSOI(BSBuffer *buffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(buffer);
    // Add SOI = 0xFFD8
    buffer->pBase = (uint8_t*)MOS_AllocAndZeroMemory(2);
    CODECHAL_ENCODE_CHK_NULL_RETURN(buffer->pBase);

    *(buffer->pBase)     = (m_jpegEncodeSoi >> 8) & 0xFF;
    *(buffer->pBase + 1) = (m_jpegEncodeSoi & 0xFF);
    buffer->BitOffset    = 0;
    buffer->BufferSize   = 16;

    return eStatus;
}

MOS_STATUS CodechalEncodeJpegState::PackApplicationData(
    BSBuffer                        *buffer,
    uint8_t                         *appDataChunk,
    uint32_t                           size)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(appDataChunk);

    buffer->pBase        = appDataChunk;
    buffer->BitOffset    = 0;
    buffer->BufferSize   = (size * sizeof(uint8_t) * 8);

    return eStatus;
}

MOS_STATUS CodechalEncodeJpegState::PackFrameHeader(
    BSBuffer                        *buffer,
    bool                            useSingleDefaultQuantTable)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CodechalEncodeJpegFrameHeader *frameHeader = (CodechalEncodeJpegFrameHeader *)MOS_AllocAndZeroMemory(sizeof(CodechalEncodeJpegFrameHeader));
    CODECHAL_ENCODE_CHK_NULL_RETURN(frameHeader);

    frameHeader->m_sof   = 0xC0FF;
    frameHeader->m_nf    = (uint8_t)m_jpegPicParams->m_numComponent;

    // Calculate lf - switch btes to match with simulation
    uint16_t value = 8 + (3 * frameHeader->m_nf); // does not include sof
    frameHeader->m_lf    = ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);

    frameHeader->m_p     = 8;
    frameHeader->m_y     = ((m_jpegPicParams->m_picHeight & 0xFF) << 8) | ((m_jpegPicParams->m_picHeight & 0xFF00) >> 8);
    frameHeader->m_x     = ((m_jpegPicParams->m_picWidth & 0xFF) << 8) | ((m_jpegPicParams->m_picWidth & 0xFF00) >> 8);

    for (uint8_t i = 0; i < frameHeader->m_nf; i++)
    {
        frameHeader->m_codechalJpegFrameComponent[i].m_ci = (uint8_t)m_jpegPicParams->m_componentID[i];

        if (useSingleDefaultQuantTable)
        {
            frameHeader->m_codechalJpegFrameComponent[i].m_tqi = 0; // 0/1/2 based on Y/U/V
        }
        else
        {
            frameHeader->m_codechalJpegFrameComponent[i].m_tqi = i; // 0/1/2 based on Y/U/V
        }

        // For all supported formats on JPEG encode, U and V vertical and horizontal sampling is 1
        uint32_t horizontalSamplingFactor = 0, verticalSamplingFactor = 0;
        if (i == 0)
        {
            horizontalSamplingFactor    = m_mfxInterface->GetJpegHorizontalSamplingFactorForY((CodecEncodeJpegInputSurfaceFormat)m_jpegPicParams->m_inputSurfaceFormat);
            verticalSamplingFactor      = m_mfxInterface->GetJpegVerticalSamplingFactorForY((CodecEncodeJpegInputSurfaceFormat)m_jpegPicParams->m_inputSurfaceFormat);
        }
        else
        {
            horizontalSamplingFactor    = 1;
            verticalSamplingFactor      = 1;
        }

        frameHeader->m_codechalJpegFrameComponent[i].m_samplingFactori = 0;
        frameHeader->m_codechalJpegFrameComponent[i].m_samplingFactori = ((horizontalSamplingFactor & 0xF) << 4) | (verticalSamplingFactor & 0xF);
    }

    buffer->pBase        = (uint8_t*)frameHeader;
    buffer->BitOffset    = 0;
    buffer->BufferSize   = (2 * sizeof(uint8_t) * 8) + (4 * sizeof(uint16_t) * 8) + (sizeof(frameHeader->m_codechalJpegFrameComponent[0]) * 8 * m_jpegPicParams->m_numComponent);

    return eStatus;
}

MOS_STATUS CodechalEncodeJpegState::PackHuffmanTable(
    BSBuffer                        *buffer,
    uint32_t                        tableIndex)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CodechalJpegHuffmanHeader *huffmanHeader = (CodechalJpegHuffmanHeader *)MOS_AllocAndZeroMemory(sizeof(CodechalJpegHuffmanHeader));
    CODECHAL_ENCODE_CHK_NULL_RETURN(huffmanHeader);

    huffmanHeader->m_dht = 0xC4FF;
    huffmanHeader->m_tableClassAndDestn =
        ((m_jpegHuffmanTable->m_huffmanData[tableIndex].m_tableClass & 0xF) << 4) | ((tableIndex / 2) & 0xF);

    uint16_t totalHuffValues = 0;
    for (auto i = 0; i < JPEG_NUM_HUFF_TABLE_AC_BITS; i++)
    {
        huffmanHeader->m_li[i] = (uint8_t)m_jpegHuffmanTable->m_huffmanData[tableIndex].m_bits[i];
        totalHuffValues += huffmanHeader->m_li[i];
    }

    uint16_t hdrSize = 19 + totalHuffValues;
    huffmanHeader->m_lh = ((hdrSize & 0xFF) << 8) | ((hdrSize & 0xFF00) >> 8);

    for (auto i = 0; i < totalHuffValues; i++)
    {
        huffmanHeader->m_vij[i] = (uint8_t)m_jpegHuffmanTable->m_huffmanData[tableIndex].m_huffVal[i];
    }

    buffer->pBase = (uint8_t*)huffmanHeader;
    buffer->BitOffset = 0;

    if (m_jpegHuffmanTable->m_huffmanData[tableIndex].m_tableClass == 0) // DC table
    {
        buffer->BufferSize = (2 * sizeof(uint16_t) * 8) + (sizeof(uint8_t) * 8) + (JPEG_NUM_HUFF_TABLE_AC_BITS * 8) + (totalHuffValues * sizeof(uint8_t) * 8);
    }
    else
    {
        buffer->BufferSize = (2 * sizeof(uint16_t) * 8) + (sizeof(uint8_t) * 8) + (JPEG_NUM_HUFF_TABLE_AC_BITS * 8) + (totalHuffValues * sizeof(uint8_t) * 8);
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeJpegState::PackQuantTable(
    BSBuffer                        *buffer,
    CodecJpegComponents             componentType)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CodechalEncodeJpegQuantHeader *quantHeader = (CodechalEncodeJpegQuantHeader *)MOS_AllocAndZeroMemory(sizeof(CodechalEncodeJpegQuantHeader));
    CODECHAL_ENCODE_CHK_NULL_RETURN(quantHeader);

    quantHeader->m_dqt = 0xDBFF;
    // Header size including marker
    uint16_t hdrSize = sizeof(uint16_t) * 2 + sizeof(uint8_t) + sizeof(uint8_t) * JPEG_NUM_QUANTMATRIX;
    quantHeader->m_lq = (((hdrSize - 2) & 0xFF) << 8) | (((hdrSize - 2) & 0xFF00) >> 8);
    quantHeader->m_tablePrecisionAndDestination
        = ((m_jpegQuantTables->m_quantTable[componentType].m_precision & 0xF) << 4)
        | (componentType & 0xF);

    for (auto i = 0; i < JPEG_NUM_QUANTMATRIX; i++)
    {
        quantHeader->m_qk[i] = (uint8_t)m_jpegQuantTables->m_quantTable[componentType].m_qm[i];
    }

    buffer->pBase        = (uint8_t*)quantHeader;
    buffer->BitOffset    = 0;
    buffer->BufferSize   = hdrSize * 8;

    return eStatus;
}

MOS_STATUS CodechalEncodeJpegState::PackRestartInterval(
    BSBuffer                        *buffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CodechalEncodeJpegRestartHeader *restartHeader = (CodechalEncodeJpegRestartHeader *)MOS_AllocAndZeroMemory(sizeof(CodechalEncodeJpegRestartHeader));
    CODECHAL_ENCODE_CHK_NULL_RETURN(restartHeader);

    restartHeader->m_dri = 0xDDFF;
    uint16_t hdrSize  = sizeof(uint16_t) * 3;
    restartHeader->m_lr  = (((hdrSize - 2) & 0xFF) << 8) | (((hdrSize - 2) & 0xFF00) >> 8);
    restartHeader->m_ri  = (uint16_t)(((m_jpegScanParams->m_restartInterval & 0xFF) << 8) |
        ((m_jpegScanParams->m_restartInterval & 0xFF00) >> 8));

    buffer->pBase        = (uint8_t*)restartHeader;
    buffer->BitOffset    = 0;
    buffer->BufferSize   = hdrSize * 8;

    return eStatus;
}

MOS_STATUS CodechalEncodeJpegState::PackScanHeader(
    BSBuffer                        *buffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // Size of Scan header in bytes = sos (2 bytes) + ls (2 bytes) + ns (1 byte)
    // + ss (1 byte) + se (1 byte) + ahl (1 byte) + scanComponent (2 bytes) * Number of scan components
    uint16_t hdrSize = 8 + 2 * m_jpegPicParams->m_numComponent;

    uint8_t *scanHeader = (uint8_t*)MOS_AllocAndZeroMemory(hdrSize);
    CODECHAL_ENCODE_CHK_NULL_RETURN(scanHeader);

    buffer->pBase = (uint8_t*)scanHeader;

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
    *scanHeader = (uint8_t)m_jpegPicParams->m_numComponent;
    scanHeader += 1;

    for (uint32_t j = 0; j < m_jpegPicParams->m_numComponent; j++)
    {
        *scanHeader = (uint8_t)m_jpegPicParams->m_componentID[j];
        scanHeader += 1;

        // For Y8 image format there is only one scan component, so scanComponent[1] and scanComponent[2] should not be added to the header
        // scanHeader->scanComponent[j].Tdaj
        if (j == 0)
        {
            *scanHeader = (uint8_t)(((m_jpegHuffmanTable->m_huffmanData[0].m_tableID & 0x0F) << 4)
                | ((m_jpegHuffmanTable->m_huffmanData[1].m_tableID & 0x0F)));
            scanHeader += 1;
        }
        else
        {
            *scanHeader = (uint8_t)(((m_jpegHuffmanTable->m_huffmanData[2].m_tableID & 0x0F) << 4)
                | ((m_jpegHuffmanTable->m_huffmanData[3].m_tableID & 0x0F)));
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

    return eStatus;
}

MOS_STATUS CodechalEncodeJpegState::ExecutePictureLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    m_mode = CODECHAL_ENCODE_MODE_JPEG;

    // set MFX_PIPE_MODE_SELECT
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode                   = m_mode;
    pipeModeSelectParams.bStreamOutEnabled      = false;
    pipeModeSelectParams.bShortFormatInUse      = false;
    pipeModeSelectParams.bPreDeblockOutEnable   = false;
    pipeModeSelectParams.bPostDeblockOutEnable  = false;

    // set MFX_SURFACE_STATE
    MHW_VDBOX_SURFACE_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.Mode      = m_mode;
    surfaceParams.psSurface = &m_rawSurface; // original picture to be encoded

    // set MFX_PIPE_BUF_ADDR_STATE
    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams;
    pipeBufAddrParams.Mode          = m_mode;
    pipeBufAddrParams.psRawSurface  = &m_rawSurface; // original picture to be encoded

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            &m_rawSurface,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "SrcSurf"))
    );

    // No references for JPEG
    // Setting invalid entries to nullptr
    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME; i++)
    {
        pipeBufAddrParams.presReferences[i] = nullptr;
    }

    // set MFX_IND_OBJ_BASE_ADDR_STATE
    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode                       = m_mode;
    indObjBaseAddrParams.presPakBaseObjectBuffer    = &m_resBitstreamBuffer;
    indObjBaseAddrParams.dwPakBaseObjectSize        = m_bitstreamUpperBound;

    // set MFX_JPEG_PIC_STATE
    MhwVdboxJpegEncodePicState jpegPicState;
    MOS_ZeroMemory(&jpegPicState, sizeof(jpegPicState));
    jpegPicState.pJpegEncodePicParams   = m_jpegPicParams;
    jpegPicState.mode                   = m_mode;

    // Send command buffer header at the beginning (OS dependent)
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &surfaceParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxJpegEncodePicStateCmd(&cmdBuffer, &jpegPicState));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return eStatus;
}

MOS_STATUS CodechalEncodeJpegState::ExecuteSliceLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    if (m_encodeParams.dwNumSlices != 1)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("JPEG encode only one scan is supported.");
    }

    MOS_SURFACE *surface = &m_rawSurface;

    bool useSingleDefaultQuantTable = (m_jpegQuantMatrixSent == false &&
                                       ((surface->Format == Format_A8R8G8B8) ||
                                        (surface->Format == Format_X8R8G8B8) ||
                                        (surface->Format == Format_A8B8G8R8) ||
                                        (surface->Format == Format_X8B8G8R8)));

    CodecJpegQuantMatrix *tempJpegQuantMatrix = (CodecJpegQuantMatrix *)MOS_AllocAndZeroMemory(sizeof(CodecJpegQuantMatrix));
    CODECHAL_ENCODE_CHK_NULL_RETURN(tempJpegQuantMatrix);

    uint32_t numQuantTables = JPEG_MAX_NUM_QUANT_TABLE_INDEX;
    for (uint32_t scanCount = 0; scanCount < m_encodeParams.dwNumSlices; scanCount++)
    {
        MHW_VDBOX_QM_PARAMS fqmParams;
        MOS_ZeroMemory(&fqmParams, sizeof(fqmParams));

        // set MFX_FQM_STATE
        fqmParams.pJpegQuantMatrix = tempJpegQuantMatrix;

        // For monochrome inputs there will be only 1 quantization table and huffman table sent
        if (m_jpegPicParams->m_inputSurfaceFormat == codechalJpegY8)
        {
            numQuantTables = 1;
            m_encodeParams.dwNumHuffBuffers = 2; //for Y8 only 2 huff tables
        }
        // If there is only 1 quantization table copy over the table to 2nd and 3rd table in JPEG state (used for frame header)
        // OR For RGB input surfaces, if the app does not send quantization tables, then use luma quant table for all 3 components
        else if (m_jpegPicParams->m_numQuantTable == 1 || useSingleDefaultQuantTable)
        {
            for (auto i = 1; i < JPEG_MAX_NUM_QUANT_TABLE_INDEX; i++)
            {
                m_jpegQuantTables->m_quantTable[i].m_precision = m_jpegQuantTables->m_quantTable[0].m_precision;
                m_jpegQuantTables->m_quantTable[i].m_tableID = m_jpegQuantTables->m_quantTable[0].m_tableID;

                eStatus = MOS_SecureMemcpy(&m_jpegQuantTables->m_quantTable[i].m_qm[0], JPEG_NUM_QUANTMATRIX * sizeof(uint16_t),
                    &m_jpegQuantTables->m_quantTable[0].m_qm[0], JPEG_NUM_QUANTMATRIX * sizeof(uint16_t));
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
                    MOS_SafeFreeMemory(tempJpegQuantMatrix);
                    return eStatus;
                }
            }
        }
        // If there are 2 quantization tables copy over the second table to 3rd table in JPEG state since U and V share the same table (used for frame header)
        else if (m_jpegPicParams->m_numQuantTable == 2)
        {
            m_jpegQuantTables->m_quantTable[2].m_precision    = m_jpegQuantTables->m_quantTable[1].m_precision;
            m_jpegQuantTables->m_quantTable[2].m_tableID      = m_jpegQuantTables->m_quantTable[1].m_tableID;

            eStatus = MOS_SecureMemcpy(&m_jpegQuantTables->m_quantTable[2].m_qm[0], JPEG_NUM_QUANTMATRIX * sizeof(uint16_t),
                &m_jpegQuantTables->m_quantTable[1].m_qm[0], JPEG_NUM_QUANTMATRIX * sizeof(uint16_t));
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
                MOS_SafeFreeMemory(tempJpegQuantMatrix);
                return eStatus;
            }

        }
        // else 3 quantization tables are sent by the application for non monochrome input formats. In that case, do nothing.

        for (uint32_t i = 0; i < numQuantTables; i++)
        {
            fqmParams.pJpegQuantMatrix->m_jpegQMTableType[i] = m_jpegQuantTables->m_quantTable[i].m_tableID; // Used to distinguish between Y,U,V quantization tables for the same scan

            for (auto j = 0; j < JPEG_NUM_QUANTMATRIX; j++)
            {
                uint32_t k = jpeg_qm_scan_8x8[j];

                // copy over Quant matrix in raster order from zig zag
                fqmParams.pJpegQuantMatrix->m_quantMatrix[i][k] = (uint8_t)m_jpegQuantTables->m_quantTable[i].m_qm[j];
            }
        }

        eStatus = (MOS_STATUS) m_mfxInterface->AddMfxJpegFqmCmd(&cmdBuffer, &fqmParams, numQuantTables);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_SafeFreeMemory(tempJpegQuantMatrix);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
        }

        // set MFC_JPEG_HUFF_TABLE - Convert encoded huffman table to actual table for HW
        // We need a different params struct for JPEG Encode Huffman table because JPEG decode huffman table has Bits and codes,
        // whereas JPEG encode huffman table has huffman code lengths and values
        MHW_VDBOX_ENCODE_HUFF_TABLE_PARAMS   huffTableParams[JPEG_MAX_NUM_HUFF_TABLE_INDEX];
        for (uint32_t i = 0; i < m_encodeParams.dwNumHuffBuffers; i++)
        {
            CodechalEncodeJpegHuffTable huffmanTable;// intermediate table for each AC/DC component which will be copied to huffTableParams
            MOS_ZeroMemory(&huffmanTable, sizeof(huffmanTable));

            eStatus = (MOS_STATUS) ConvertHuffDataToTable(m_jpegHuffmanTable->m_huffmanData[i], &huffmanTable);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(tempJpegQuantMatrix);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
            }

            huffTableParams[m_jpegHuffmanTable->m_huffmanData[i].m_tableID].HuffTableID = m_jpegHuffmanTable->m_huffmanData[i].m_tableID;

            if (m_jpegHuffmanTable->m_huffmanData[i].m_tableClass == 0) // DC table
            {
                eStatus = (MOS_STATUS) MOS_SecureMemcpy(
                    huffTableParams[m_jpegHuffmanTable->m_huffmanData[i].m_tableID].pDCCodeValues,
                    JPEG_NUM_HUFF_TABLE_DC_HUFFVAL * sizeof(uint16_t),
                    &huffmanTable.m_huffCode,
                    JPEG_NUM_HUFF_TABLE_DC_HUFFVAL * sizeof(uint16_t));
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_SafeFreeMemory(tempJpegQuantMatrix);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
                }

                eStatus = (MOS_STATUS) MOS_SecureMemcpy(huffTableParams[m_jpegHuffmanTable->m_huffmanData[i].m_tableID].pDCCodeLength,
                    JPEG_NUM_HUFF_TABLE_DC_HUFFVAL * sizeof(uint8_t),
                    &huffmanTable.m_huffSize,
                    JPEG_NUM_HUFF_TABLE_DC_HUFFVAL * sizeof(uint8_t));
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_SafeFreeMemory(tempJpegQuantMatrix);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
                }
            }
            else // AC Table
            {
                eStatus = (MOS_STATUS) MOS_SecureMemcpy(huffTableParams[m_jpegHuffmanTable->m_huffmanData[i].m_tableID].pACCodeValues,
                    JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint16_t),
                    &huffmanTable.m_huffCode,
                    JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint16_t));
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_SafeFreeMemory(tempJpegQuantMatrix);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
                }

                eStatus = (MOS_STATUS) MOS_SecureMemcpy(huffTableParams[m_jpegHuffmanTable->m_huffmanData[i].m_tableID].pACCodeLength,
                    JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint8_t),
                    &huffmanTable.m_huffSize,
                    JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint8_t));
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_SafeFreeMemory(tempJpegQuantMatrix);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
                }
            }
        }

        // Send 2 huffman table commands - 1 for Luma and one for chroma for non-monchrome input formats
        // If only one table is sent by the app (2 buffers), send the same table for Luma and chroma
        bool repeatHuffTable = false;
        if ((m_encodeParams.dwNumHuffBuffers / 2 < JPEG_MAX_NUM_HUFF_TABLE_INDEX)
            && (m_jpegPicParams->m_inputSurfaceFormat != codechalJpegY8))
        {
            repeatHuffTable = true;

            // Copy over huffman data to the other two data buffers for JPEG picture header
            for (uint32_t i = 0; i < m_encodeParams.dwNumHuffBuffers; i++)
            {
                m_jpegHuffmanTable->m_huffmanData[i + 2].m_tableClass = m_jpegHuffmanTable->m_huffmanData[i].m_tableClass;
                m_jpegHuffmanTable->m_huffmanData[i + 2].m_tableID    = m_jpegHuffmanTable->m_huffmanData[i].m_tableID;

                eStatus = MOS_SecureMemcpy(&m_jpegHuffmanTable->m_huffmanData[i + 2].m_bits[0],
                    sizeof(uint8_t) * JPEG_NUM_HUFF_TABLE_AC_BITS,
                    &m_jpegHuffmanTable->m_huffmanData[i].m_bits[0],
                    sizeof(uint8_t) * JPEG_NUM_HUFF_TABLE_AC_BITS);
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
                    MOS_SafeFreeMemory(tempJpegQuantMatrix);
                    return eStatus;
                }

                eStatus = MOS_SecureMemcpy(&m_jpegHuffmanTable->m_huffmanData[i + 2].m_huffVal[0],
                    sizeof(uint8_t) * JPEG_NUM_HUFF_TABLE_AC_HUFFVAL,
                    &m_jpegHuffmanTable->m_huffmanData[i].m_huffVal[0],
                    sizeof(uint8_t) * JPEG_NUM_HUFF_TABLE_AC_HUFFVAL);
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
                    MOS_SafeFreeMemory(tempJpegQuantMatrix);
                    return eStatus;
                }
            }
        }

        // the number of huffman commands is half of the huffman buffers sent by the app, since AC and DC buffers are combined into one command
        for (uint32_t i = 0; i < m_encodeParams.dwNumHuffBuffers / 2; i++)
        {
            if (repeatHuffTable)
            {
                eStatus = (MOS_STATUS) (m_mfxInterface->AddMfcJpegHuffTableStateCmd(&cmdBuffer, &huffTableParams[i]));
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_SafeFreeMemory(tempJpegQuantMatrix);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
                }
            }

            eStatus = (MOS_STATUS) m_mfxInterface->AddMfcJpegHuffTableStateCmd(&cmdBuffer, &huffTableParams[i]);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(tempJpegQuantMatrix);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
            }
        }

        // set MFC_JPEG_SCAN_OBJECT
        MhwVdboxJpegScanParams scanObjectParams;
        scanObjectParams.mode                   = m_mode;
        scanObjectParams.inputSurfaceFormat     = (CodecEncodeJpegInputSurfaceFormat)m_jpegPicParams->m_inputSurfaceFormat;
        scanObjectParams.dwPicWidth             = m_jpegPicParams->m_picWidth;
        scanObjectParams.dwPicHeight            = m_jpegPicParams->m_picHeight;
        scanObjectParams.pJpegEncodeScanParams  = m_jpegScanParams;

        eStatus = (MOS_STATUS) m_mfxInterface->AddMfcJpegScanObjCmd(&cmdBuffer, &scanObjectParams);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_SafeFreeMemory(tempJpegQuantMatrix);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
        }
        // set MFC_JPEG_PAK_INSERT_OBJECT
        MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
        MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));

        // The largest component written through the MFC_JPEG_PAK_INSERT_OBJECT command is Huffman table
        pakInsertObjectParams.pBsBuffer = (BSBuffer *)MOS_AllocAndZeroMemory(sizeof(CodechalEncodeJpegFrameHeader));
        if (pakInsertObjectParams.pBsBuffer == nullptr)
        {
            MOS_SafeFreeMemory(tempJpegQuantMatrix);
            CODECHAL_ENCODE_CHK_NULL_RETURN(nullptr);
        }
        if(!m_fullHeaderInAppData)
        {
            // Add SOI (0xFFD8) (only if it was sent by the application)
            eStatus = (MOS_STATUS)PackSOI(pakInsertObjectParams.pBsBuffer);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
                MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
                MOS_SafeFreeMemory(tempJpegQuantMatrix);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
            }
            pakInsertObjectParams.dwOffset                      = 0;
            pakInsertObjectParams.dwBitSize                     = pakInsertObjectParams.pBsBuffer->BufferSize;
            pakInsertObjectParams.bLastHeader                   = false;
            pakInsertObjectParams.bEndOfSlice                   = false;
            pakInsertObjectParams.bResetBitstreamStartingPos    = 1; // from discussion with HW Architect
            eStatus = (MOS_STATUS) m_mfxInterface->AddMfxPakInsertObject(&cmdBuffer, nullptr, &pakInsertObjectParams);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
                MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
                MOS_SafeFreeMemory(tempJpegQuantMatrix);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
            }
            MOS_FreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
        }
        // Add Application data if it was sent by application
        if (m_applicationData != nullptr)
        {
            uint8_t* appDataChunk = nullptr;
            uint32_t appDataChunkSize = m_appDataSize;

            // We can write a maximum of 1020 words per command, so if the size of the app data is
            // more than 1020 we need to send multiple commands for writing out app data
            uint32_t numAppDataCmdsNeeded = 1;
            uint32_t appDataCmdSizeResidue = 0;
            if (m_appDataSize > 1020)
            {
                numAppDataCmdsNeeded  = m_appDataSize / 1020;
                appDataCmdSizeResidue = m_appDataSize % 1020;

                appDataChunkSize = 1020;
            }

            appDataChunk = (uint8_t*)MOS_AllocAndZeroMemory(appDataChunkSize);
            if (appDataChunk == nullptr)
            {
                MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
                MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
                MOS_SafeFreeMemory(tempJpegQuantMatrix);
                CODECHAL_ENCODE_CHK_NULL_RETURN(nullptr);
            }

            for (uint32_t i = 0; i < numAppDataCmdsNeeded; i++)
            {
                uint8_t *copyAddress = (uint8_t*)(m_applicationData) + (i * appDataChunkSize);

                MOS_SecureMemcpy(appDataChunk, appDataChunkSize,
                    copyAddress, appDataChunkSize);

                eStatus = (MOS_STATUS)PackApplicationData(pakInsertObjectParams.pBsBuffer, appDataChunk, appDataChunkSize);
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
                    MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
                    MOS_SafeFreeMemory(tempJpegQuantMatrix);
                    MOS_SafeFreeMemory(appDataChunk);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
                }
                pakInsertObjectParams.dwOffset                      = 0;
                pakInsertObjectParams.dwBitSize                     = pakInsertObjectParams.pBsBuffer->BufferSize;
                //if full header is included in application data, it will be the last insert headers
                if((appDataCmdSizeResidue == 0) && m_fullHeaderInAppData)
                {
                    pakInsertObjectParams.bLastHeader                   = true;
                    pakInsertObjectParams.bEndOfSlice                   = true;
                }
                else
                {
                    pakInsertObjectParams.bLastHeader                   = false;
                    pakInsertObjectParams.bEndOfSlice                   = false;
                }
                pakInsertObjectParams.bResetBitstreamStartingPos    = 1; // from discussion with HW Architect
                eStatus = (MOS_STATUS)m_mfxInterface->AddMfxPakInsertObject(&cmdBuffer, nullptr,
                    &pakInsertObjectParams);
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
                    MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
                    MOS_SafeFreeMemory(tempJpegQuantMatrix);
                    MOS_SafeFreeMemory(appDataChunk);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
                }
            }

            if (appDataCmdSizeResidue != 0)
            {
                uint8_t* lastAddress = (uint8_t*)(m_applicationData) + (numAppDataCmdsNeeded * appDataChunkSize);
                appDataChunkSize = appDataCmdSizeResidue;

                MOS_SecureMemcpy(appDataChunk, appDataChunkSize,
                    lastAddress,
                    appDataChunkSize);

                eStatus = (MOS_STATUS)PackApplicationData(pakInsertObjectParams.pBsBuffer, appDataChunk, appDataCmdSizeResidue);
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
                    MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
                    MOS_SafeFreeMemory(tempJpegQuantMatrix);
                    MOS_SafeFreeMemory(appDataChunk);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
                }
                pakInsertObjectParams.dwOffset                      = 0;
                pakInsertObjectParams.dwBitSize                     = pakInsertObjectParams.pBsBuffer->BufferSize;
                //if full header is included in application data, it will be the last insert headers
                if(m_fullHeaderInAppData)
                {
                    pakInsertObjectParams.bLastHeader                   = true;
                    pakInsertObjectParams.bEndOfSlice                   = true;
                }
                else
                {
                    pakInsertObjectParams.bLastHeader                   = false;
                    pakInsertObjectParams.bEndOfSlice                   = false;
                }
                pakInsertObjectParams.bResetBitstreamStartingPos    = 1; // from discussion with HW Architect
                eStatus = (MOS_STATUS)m_mfxInterface->AddMfxPakInsertObject(&cmdBuffer, nullptr,
                    &pakInsertObjectParams);
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
                    MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
                    MOS_SafeFreeMemory(tempJpegQuantMatrix);
                    MOS_SafeFreeMemory(appDataChunk);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
                }
            }

            MOS_FreeMemory(appDataChunk);
        }
        if(!m_fullHeaderInAppData)
        {
        // Add Quant Table for Y
        eStatus = (MOS_STATUS)PackQuantTable(pakInsertObjectParams.pBsBuffer, jpegComponentY);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
            MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
            MOS_SafeFreeMemory(tempJpegQuantMatrix);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
        }
        pakInsertObjectParams.dwOffset                      = 0;
        pakInsertObjectParams.dwBitSize                     = pakInsertObjectParams.pBsBuffer->BufferSize;
        pakInsertObjectParams.bLastHeader                   = false;
        pakInsertObjectParams.bEndOfSlice                   = false;
        pakInsertObjectParams.bResetBitstreamStartingPos    = 1; // from discussion with HW Architect
        eStatus = (MOS_STATUS)m_mfxInterface->AddMfxPakInsertObject(&cmdBuffer, nullptr,
            &pakInsertObjectParams);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
            MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
            MOS_SafeFreeMemory(tempJpegQuantMatrix);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
        }
        MOS_FreeMemory(pakInsertObjectParams.pBsBuffer->pBase);

        if (!useSingleDefaultQuantTable)
        {
            // Since there is no U and V in monochrome format, donot add Quantization table header for U and V components
            if (m_jpegPicParams->m_inputSurfaceFormat != codechalJpegY8)
            {
                // Add quant table for U
                eStatus = (MOS_STATUS)PackQuantTable(pakInsertObjectParams.pBsBuffer, jpegComponentU);
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
                    MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
                    MOS_SafeFreeMemory(tempJpegQuantMatrix);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
                }
                pakInsertObjectParams.dwOffset                      = 0;
                pakInsertObjectParams.dwBitSize                     = pakInsertObjectParams.pBsBuffer->BufferSize;
                pakInsertObjectParams.bLastHeader                   = false;
                pakInsertObjectParams.bEndOfSlice                   = false;
                pakInsertObjectParams.bResetBitstreamStartingPos    = 1; // from discussion with HW Architect
                eStatus = (MOS_STATUS)m_mfxInterface->AddMfxPakInsertObject(&cmdBuffer, nullptr,
                    &pakInsertObjectParams);
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
                    MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
                    MOS_SafeFreeMemory(tempJpegQuantMatrix);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
                }
                MOS_FreeMemory(pakInsertObjectParams.pBsBuffer->pBase);

                // Add quant table for V
                eStatus = (MOS_STATUS)PackQuantTable(pakInsertObjectParams.pBsBuffer, jpegComponentV);
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
                    MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
                    MOS_SafeFreeMemory(tempJpegQuantMatrix);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
                }
                pakInsertObjectParams.dwOffset                      = 0;
                pakInsertObjectParams.dwBitSize                     = pakInsertObjectParams.pBsBuffer->BufferSize;
                pakInsertObjectParams.bLastHeader                   = false;
                pakInsertObjectParams.bEndOfSlice                   = false;
                pakInsertObjectParams.bResetBitstreamStartingPos    = 1; // from discussion with HW Architect
                eStatus = (MOS_STATUS)m_mfxInterface->AddMfxPakInsertObject(&cmdBuffer, nullptr,
                    &pakInsertObjectParams);
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
                    MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
                    MOS_SafeFreeMemory(tempJpegQuantMatrix);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
                }
                MOS_FreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
            }
        }

        // Add Frame Header
        eStatus = (MOS_STATUS)PackFrameHeader(pakInsertObjectParams.pBsBuffer, useSingleDefaultQuantTable);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
            MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
            MOS_SafeFreeMemory(tempJpegQuantMatrix);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
        }
        pakInsertObjectParams.dwOffset                      = 0;
        pakInsertObjectParams.dwBitSize                     = pakInsertObjectParams.pBsBuffer->BufferSize;
        pakInsertObjectParams.bLastHeader                   = false;
        pakInsertObjectParams.bEndOfSlice                   = false;
        pakInsertObjectParams.bResetBitstreamStartingPos    = 1; // from discussion with HW Architect
        eStatus = (MOS_STATUS)m_mfxInterface->AddMfxPakInsertObject(&cmdBuffer, nullptr,
            &pakInsertObjectParams);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
            MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
            MOS_SafeFreeMemory(tempJpegQuantMatrix);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
        }
        MOS_FreeMemory(pakInsertObjectParams.pBsBuffer->pBase);

        // Add Huffman Table for Y - DC table, Y- AC table, U/V - DC table, U/V - AC table
        for (uint32_t i = 0; i < m_encodeParams.dwNumHuffBuffers; i++)
        {
            eStatus = (MOS_STATUS)PackHuffmanTable(pakInsertObjectParams.pBsBuffer, i);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
                MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
                MOS_SafeFreeMemory(tempJpegQuantMatrix);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
            }
            pakInsertObjectParams.dwOffset                      = 0;
            pakInsertObjectParams.dwBitSize                     = pakInsertObjectParams.pBsBuffer->BufferSize;
            pakInsertObjectParams.bLastHeader                   = false;
            pakInsertObjectParams.bEndOfSlice                   = false;
            pakInsertObjectParams.bResetBitstreamStartingPos    = 1; // from discussion with HW Architect
            eStatus = (MOS_STATUS)m_mfxInterface->AddMfxPakInsertObject(&cmdBuffer, nullptr,
                &pakInsertObjectParams);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
                MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
                MOS_SafeFreeMemory(tempJpegQuantMatrix);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
            }
            MOS_FreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
        }

        // Restart Interval - Add only if the restart interval is not zero
        if (m_jpegScanParams->m_restartInterval != 0)
        {
            eStatus = (MOS_STATUS)PackRestartInterval(pakInsertObjectParams.pBsBuffer);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
                MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
                MOS_SafeFreeMemory(tempJpegQuantMatrix);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
            }
            pakInsertObjectParams.dwOffset                      = 0;
            pakInsertObjectParams.dwBitSize                     = pakInsertObjectParams.pBsBuffer->BufferSize;
            pakInsertObjectParams.bLastHeader                   = false;
            pakInsertObjectParams.bEndOfSlice                   = false;
            pakInsertObjectParams.bResetBitstreamStartingPos    = 1; // from discussion with HW Architect
            eStatus = (MOS_STATUS)m_mfxInterface->AddMfxPakInsertObject(&cmdBuffer, nullptr,
                &pakInsertObjectParams);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
                MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
                MOS_SafeFreeMemory(tempJpegQuantMatrix);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
            }
            MOS_FreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
        }

        // Add scan header
        eStatus = (MOS_STATUS)PackScanHeader(pakInsertObjectParams.pBsBuffer);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
            MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
            MOS_SafeFreeMemory(tempJpegQuantMatrix);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
        }
        pakInsertObjectParams.dwOffset                      = 0;
        pakInsertObjectParams.dwBitSize                     = pakInsertObjectParams.pBsBuffer->BufferSize;
        pakInsertObjectParams.bLastHeader                   = true;
        pakInsertObjectParams.bEndOfSlice                   = true;
        pakInsertObjectParams.bResetBitstreamStartingPos    = 1; // from discussion with HW Architect
        eStatus = (MOS_STATUS)m_mfxInterface->AddMfxPakInsertObject(&cmdBuffer, nullptr,
            &pakInsertObjectParams);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
            MOS_SafeFreeMemory(pakInsertObjectParams.pBsBuffer);
            MOS_SafeFreeMemory(tempJpegQuantMatrix);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
        }
        MOS_FreeMemory(pakInsertObjectParams.pBsBuffer->pBase);
        }
        MOS_FreeMemory(pakInsertObjectParams.pBsBuffer);
    }

    eStatus = ReadMfcStatus(&cmdBuffer);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_SafeFreeMemory(tempJpegQuantMatrix);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
    }

    eStatus = EndStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_SafeFreeMemory(tempJpegQuantMatrix);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
    }

    eStatus = m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_SafeFreeMemory(tempJpegQuantMatrix);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
    }

    std::string pakPassName = "PAK_PASS" + std::to_string(static_cast<uint32_t>(m_currPass));
    CODECHAL_DEBUG_TOOL(
        eStatus = m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            pakPassName.data());
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_SafeFreeMemory(tempJpegQuantMatrix);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
        }

    //CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
    //    m_debugInterface,
    //    &cmdBuffer));
    )

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    eStatus = SubmitCommandBuffer(&cmdBuffer, m_renderContextUsesNullHw);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_SafeFreeMemory(tempJpegQuantMatrix);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
    }

    if (tempJpegQuantMatrix != nullptr)
    {
        MOS_FreeMemory(tempJpegQuantMatrix);
        tempJpegQuantMatrix = nullptr;
    }

    return eStatus;
}

uint32_t CodechalEncodeJpegState::CalculateCommandBufferSize()
{
    uint32_t commandBufferSize =
        m_pictureStatesSize        +
        m_extraPictureStatesSize   +
        (m_sliceStatesSize * m_numSlices);

    // For JPEG encoder, add the size of PAK_INSERT_OBJ commands which is also part of command buffer
    if(m_standard == CODECHAL_JPEG)
    {
        // Add PAK_INSERT_OBJ for app data
        // PAK_INSERT_OBJ contains 2 DWORDS + bytes of payload data
        // There is a max of 1024 payload bytes of app data per PAK_INSERT_OBJ command, so adding 2 DWORDS for each of them
        // Total payload data is the same size as app data
        commandBufferSize += (m_encodeParams.dwAppDataSize + (2 * sizeof(uint32_t) * (m_encodeParams.dwAppDataSize / 1020+1)));  //to be consistent with how we split app data into chunks.

        // Add number of bytes of data added through PAK_INSERT_OBJ command
        commandBufferSize += (2 + // SOI = 2 bytes
                // Frame header - add sizes of each component of CodechalEncodeJpegFrameHeader
                (2 * sizeof(uint8_t)) + (4 * sizeof(uint16_t)) + 3 * sizeof(uint8_t)*  jpegNumComponent +
                // AC and DC Huffman tables - 2 Huffman tables for each component, and 3 components
                (2 * 3 * sizeof(CodechalJpegHuffmanHeader)) +
                // Quant tables - 1 for Quant table of each component, so 3 quant tables per frame
                (3 * sizeof(CodechalEncodeJpegQuantHeader)) +
                // Restart interval - 1 per frame
                sizeof(CodechalEncodeJpegRestartHeader) +
                // Scan header - 1 per frame
                sizeof(CodechalEncodeJpegScanHeader));
    }

    if (m_singleTaskPhaseSupported)
    {
        commandBufferSize *= (m_numPasses + 1);
    }

    // 4K align since allocation is in chunks of 4K bytes.
    commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, 0x1000);

    return commandBufferSize;
}

CodechalEncodeJpegState::CodechalEncodeJpegState(
        CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo)
        :CodechalEncoderState(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    memset(m_refList, 0, sizeof(m_refList));
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalEncodeJpegState::DumpQuantTables(
    CodecEncodeJpegQuantTable *quantTable)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrIqParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(quantTable);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    //Dump quantTable[JPEG_MAX_NUM_QUANT_TABLE_INDEX]
    for (uint32_t i = 0; i < JPEG_MAX_NUM_QUANT_TABLE_INDEX; ++i)
    {
        // Dump Table ID
        oss << "TableID: " << +quantTable->m_quantTable[i].m_tableID << std::endl;
        // Dump Precision
        oss << "TableID: " << +quantTable->m_quantTable[i].m_precision << std::endl;
        //Dump usQm[JPEG_NUM_QUANTMATRIX];
        oss << "quantTable[" << +i << "].usQm[0-" << (JPEG_NUM_QUANTMATRIX - 1) << "]: " << std::endl;

        for (uint32_t j = 0; j < JPEG_NUM_QUANTMATRIX; ++j)
        {
            oss << +quantTable->m_quantTable[i].m_qm[j];
            if (j % 6 == 5 || j == JPEG_NUM_QUANTMATRIX - 1)
            {
                oss << std::endl;
            }
        }
    }
    const char *fileName = m_debugInterface->CreateFileName(
        "_ENC",
        CodechalDbgBufferType::bufIqParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeJpegState::DumpPicParams(
    CodecEncodeJpegPictureParams *picParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;
    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "Profile: " << +picParams->m_profile << std::endl;
    oss << "Progressive: " << +picParams->m_progressive << std::endl;
    oss << "Huffman: " << +picParams->m_huffman << std::endl;
    oss << "Interleaved: " << +picParams->m_interleaved << std::endl;
    oss << "Differential: " << +picParams->m_differential << std::endl;
    oss << "PicWidth: " << +picParams->m_picWidth << std::endl;
    oss << "PicHeight: " << +picParams->m_picHeight << std::endl;
    oss << "InputSurfaceFormat: " << +picParams->m_inputSurfaceFormat << std::endl;
    oss << "SampleBitDepth: " << +picParams->m_sampleBitDepth << std::endl;
    oss << "uiNumComponent: " << +picParams->m_numComponent << std::endl;

    //Dump componentIdentifier[jpegNumComponent]
    for (uint32_t i = 0; i < jpegNumComponent; ++i)
    {
        oss << "ComponentIdentifier[" << +i << "]: " << +picParams->m_componentID[i] << std::endl;
    }

    //Dump quantTableSelector[jpegNumComponent]
    for (uint32_t i = 0; i < jpegNumComponent; ++i)
    {
        oss << "QuantTableSelector[" << +i << "]: " << +picParams->m_quantTableSelector[i] << std::endl;
    }
    oss << "Quality: " << +picParams->m_quality << std::endl;
    oss << "NumScan: " << +picParams->m_numScan << std::endl;
    oss << "NumQuantTable: " << +picParams->m_numQuantTable << std::endl;
    oss << "NumCodingTable: " << +picParams->m_numCodingTable << std::endl;
    oss << "StatusReportFeedbackNumber: " << +picParams->m_statusReportFeedbackNumber << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_ENC",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeJpegState::DumpScanParams(
    CodecEncodeJpegScanHeader *scanParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrScanParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(scanParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "RestartInterval: " << +scanParams->m_restartInterval << std::endl;
    oss << "NumComponents: " << +scanParams->m_numComponent << std::endl;

    //Dump ComponentSelector[jpegNumComponent]
    for (uint32_t i = 0; i < jpegNumComponent; ++i)
    {
        oss << "ComponentSelector[" << +i << "]: " << +scanParams->m_componentSelector[i] << std::endl;
    }

    //Dump DcHuffTblSelector[jpegNumComponent]
    for (uint32_t i = 0; i < jpegNumComponent; ++i)
    {
        oss << "DcHuffTblSelector[" << +i << "]: " << +scanParams->m_dcCodingTblSelector[i] << std::endl;
    }

    //Dump AcHuffTblSelector[jpegNumComponent]
    for (uint32_t i = 0; i < jpegNumComponent; ++i)
    {
        oss << "AcHuffTblSelector[" << +i << "]: " << +scanParams->m_acCodingTblSelector[i] << std::endl;
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_ENC",
        CodechalDbgBufferType::bufScanParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeJpegState::DumpHuffmanTable(
    CodecEncodeJpegHuffmanDataArray *huffmanTable)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrHuffmanTbl))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(huffmanTable);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    //Dump HuffTable[JPEG_MAX_NUM_HUFF_TABLE_INDEX]
    for (uint32_t i = 0; i < JPEG_NUM_ENCODE_HUFF_BUFF; ++i)
    {
        // Dump Table Class
        oss << "TableClass: " << +huffmanTable->m_huffmanData[i].m_tableClass << std::endl;
        // Dump Table ID
        oss << "TableID: " << +huffmanTable->m_huffmanData[i].m_tableID << std::endl;
        //Dump ucBits[JPEG_NUM_HUFF_TABLE_AC_BITS]
        oss << "HuffTable[" << +i << "].ucBits[0-" << (JPEG_NUM_HUFF_TABLE_AC_BITS - 1) << "]: " << std::endl;

        for (uint32_t j = 0; j < JPEG_NUM_HUFF_TABLE_AC_BITS; ++j)
        {
            oss << +huffmanTable->m_huffmanData[i].m_bits[j];
            if (j % 6 == 5 || j == JPEG_NUM_HUFF_TABLE_AC_BITS - 1)
            {
                oss << std::endl;
            }
        }

        //Dump ucHuffVal[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL]
        oss << "HuffTable[" << +i << "].ucHuffVal[0-" << (JPEG_NUM_HUFF_TABLE_AC_HUFFVAL - 1) << "]: " << std::endl;

        for (uint32_t j = 0; j < JPEG_NUM_HUFF_TABLE_AC_HUFFVAL; ++j)
        {
            oss << +huffmanTable->m_huffmanData[i].m_huffVal[j];
            if (j % 6 == 5 || j == JPEG_NUM_HUFF_TABLE_AC_HUFFVAL - 1)
            {
                oss << std::endl;
            }
        }
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_ENC",
        CodechalDbgBufferType::bufHuffmanTbl,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

#endif
