/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     codechal_encode_jpeg_g11.cpp
//! \brief    Defines state class for JPEG encoder.
//!

#include "codechal_encode_jpeg.h"
#include "codechal_encode_jpeg_g11.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "mos_util_user_interface.h"
#endif


CodechalEncodeJpegStateG11::CodechalEncodeJpegStateG11(
        CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo): CodechalEncodeJpegState(hwInterface, debugInterface, standardInfo),
    m_sinlgePipeVeState(nullptr)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    memset(m_refList, 0, sizeof(m_refList));

    Mos_CheckVirtualEngineSupported(m_osInterface, false, true);
    Mos_SetVirtualEngineSupported(m_osInterface, true);
}


CodechalEncodeJpegStateG11::~CodechalEncodeJpegStateG11()
{
    if (m_sinlgePipeVeState)
    {
        MOS_FreeMemAndSetNull(m_sinlgePipeVeState);
    }
}

MOS_STATUS CodechalEncodeJpegStateG11::SetAndPopulateVEHintParams(
    PMOS_COMMAND_BUFFER  cmdBuffer)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!MOS_VE_SUPPORTED(m_osInterface))
    {
        return eStatus;
    }

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        MOS_VIRTUALENGINE_SET_PARAMS  vesetParams;
        MOS_ZeroMemory(&vesetParams, sizeof(vesetParams));
        vesetParams.bNeedSyncWithPrevious = true;
        vesetParams.bSFCInUse = false;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_SetHintParams(m_sinlgePipeVeState, &vesetParams));
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_PopulateHintParams(m_sinlgePipeVeState, cmdBuffer, true));

    return eStatus;
}

MOS_STATUS CodechalEncodeJpegStateG11::UserFeatureKeyReport()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::UserFeatureKeyReport());

#if (_DEBUG || _RELEASE_INTERNAL)

    // VE2.0 Reporting
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENABLE_ENCODE_VE_CTXSCHEDULING_ID, MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface));

#endif // _DEBUG || _RELEASE_INTERNAL
    return eStatus;
}

MOS_STATUS CodechalEncodeJpegStateG11::ExecuteSliceLevel()
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
            m_jpegQuantTables->m_quantTable[2].m_precision = m_jpegQuantTables->m_quantTable[1].m_precision;
            m_jpegQuantTables->m_quantTable[2].m_tableID = m_jpegQuantTables->m_quantTable[1].m_tableID;

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
                m_jpegHuffmanTable->m_huffmanData[i + 2].m_tableID = m_jpegHuffmanTable->m_huffmanData[i].m_tableID;

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
        scanObjectParams.mode = m_mode;
        scanObjectParams.inputSurfaceFormat = (CodecEncodeJpegInputSurfaceFormat)m_jpegPicParams->m_inputSurfaceFormat;
        scanObjectParams.dwPicWidth = m_jpegPicParams->m_picWidth;
        scanObjectParams.dwPicHeight = m_jpegPicParams->m_picHeight;
        scanObjectParams.pJpegEncodeScanParams = m_jpegScanParams;

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
            pakInsertObjectParams.dwOffset = 0;
            pakInsertObjectParams.dwBitSize = pakInsertObjectParams.pBsBuffer->BufferSize;
            pakInsertObjectParams.bLastHeader = false;
            pakInsertObjectParams.bEndOfSlice = false;
            pakInsertObjectParams.bResetBitstreamStartingPos = 1; // from discussion with HW Architect
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
                numAppDataCmdsNeeded = m_appDataSize / 1020;
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
                uint8_t *copyAddress = (uint8_t*)(m_applicationData)+(i * appDataChunkSize);

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
                pakInsertObjectParams.dwOffset = 0;
                pakInsertObjectParams.dwBitSize = pakInsertObjectParams.pBsBuffer->BufferSize;
                if((appDataCmdSizeResidue == 0) && m_fullHeaderInAppData)
                {
                    pakInsertObjectParams.bLastHeader = true;
                    pakInsertObjectParams.bEndOfSlice = true;
                }
                else
                {
                    pakInsertObjectParams.bLastHeader = false;
                    pakInsertObjectParams.bEndOfSlice = false;
                }
                pakInsertObjectParams.bResetBitstreamStartingPos = 1; // from discussion with HW Architect
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
                uint8_t* lastAddress = (uint8_t*)(m_applicationData)+(numAppDataCmdsNeeded * appDataChunkSize);
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
                pakInsertObjectParams.dwOffset = 0;
                pakInsertObjectParams.dwBitSize = pakInsertObjectParams.pBsBuffer->BufferSize;
                if(m_fullHeaderInAppData)
                {
                    pakInsertObjectParams.bLastHeader = true;
                    pakInsertObjectParams.bEndOfSlice = true;
                }
                else
                {
                    pakInsertObjectParams.bLastHeader = false;
                    pakInsertObjectParams.bEndOfSlice = false;
                }
                pakInsertObjectParams.bResetBitstreamStartingPos = 1; // from discussion with HW Architect
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

        pakInsertObjectParams.dwOffset = 0;
        pakInsertObjectParams.dwBitSize = pakInsertObjectParams.pBsBuffer->BufferSize;
        pakInsertObjectParams.bLastHeader = false;
        pakInsertObjectParams.bEndOfSlice = false;
        pakInsertObjectParams.bResetBitstreamStartingPos = 1; // from discussion with HW Architect
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
                pakInsertObjectParams.dwOffset = 0;
                pakInsertObjectParams.dwBitSize = pakInsertObjectParams.pBsBuffer->BufferSize;
                pakInsertObjectParams.bLastHeader = false;
                pakInsertObjectParams.bEndOfSlice = false;
                pakInsertObjectParams.bResetBitstreamStartingPos = 1; // from discussion with HW Architect
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
                pakInsertObjectParams.dwOffset = 0;
                pakInsertObjectParams.dwBitSize = pakInsertObjectParams.pBsBuffer->BufferSize;
                pakInsertObjectParams.bLastHeader = false;
                pakInsertObjectParams.bEndOfSlice = false;
                pakInsertObjectParams.bResetBitstreamStartingPos = 1; // from discussion with HW Architect
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
        pakInsertObjectParams.dwOffset = 0;
        pakInsertObjectParams.dwBitSize = pakInsertObjectParams.pBsBuffer->BufferSize;
        pakInsertObjectParams.bLastHeader = false;
        pakInsertObjectParams.bEndOfSlice = false;
        pakInsertObjectParams.bResetBitstreamStartingPos = 1; // from discussion with HW Architect
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
            pakInsertObjectParams.dwOffset = 0;
            pakInsertObjectParams.dwBitSize = pakInsertObjectParams.pBsBuffer->BufferSize;
            pakInsertObjectParams.bLastHeader = false;
            pakInsertObjectParams.bEndOfSlice = false;
            pakInsertObjectParams.bResetBitstreamStartingPos = 1; // from discussion with HW Architect
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
            pakInsertObjectParams.dwOffset = 0;
            pakInsertObjectParams.dwBitSize = pakInsertObjectParams.pBsBuffer->BufferSize;
            pakInsertObjectParams.bLastHeader = false;
            pakInsertObjectParams.bEndOfSlice = false;
            pakInsertObjectParams.bResetBitstreamStartingPos = 1; // from discussion with HW Architect
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
        pakInsertObjectParams.dwOffset = 0;
        pakInsertObjectParams.dwBitSize = pakInsertObjectParams.pBsBuffer->BufferSize;
        pakInsertObjectParams.bLastHeader = true;
        pakInsertObjectParams.bEndOfSlice = true;
        pakInsertObjectParams.bResetBitstreamStartingPos = 1; // from discussion with HW Architect
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

    eStatus = SetAndPopulateVEHintParams(&cmdBuffer);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_SafeFreeMemory(tempJpegQuantMatrix);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
    }
    eStatus = m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
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

MOS_STATUS CodechalEncodeJpegStateG11::Initialize(CodechalSetting  *settings)
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

    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        m_sinlgePipeVeState = (PCODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE));
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_sinlgePipeVeState);
        eStatus = (MOS_STATUS) CodecHalEncodeSinglePipeVE_InitInterface(m_hwInterface, m_sinlgePipeVeState);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_SafeFreeMemory(m_sinlgePipeVeState);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeJpegStateG11::SetGpuCtxCreatOption()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::SetGpuCtxCreatOption());
    }
    else
    {
        m_gpuCtxCreatOpt = MOS_New(MOS_GPUCTX_CREATOPTIONS_ENHANCED);
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_gpuCtxCreatOpt);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_ConstructParmsForGpuCtxCreation(
            m_sinlgePipeVeState,
            (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt));
    }

    return eStatus;
}

