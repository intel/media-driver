/*
* Copyright (c) 2011-2018, Intel Corporation
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
//! \file     codechal_decode_jpeg.cpp
//! \brief    Implements the decode interface extension for JPEG.
//! \details  Implements all functions required by CodecHal for JPEG decoding.
//!

#include "codechal_decode_jpeg.h"
#include "codechal_mmc_decode_jpeg.h"
#include "hal_oca_interface.h"
#if USE_CODECHAL_DEBUG_TOOL
#include <sstream>
#include "codechal_debug.h"
#endif

#define CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE       8
#define CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE_X2    16
#define CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE_X4    32
#define CODECHAL_DECODE_JPEG_ERR_FRAME_WIDTH        32
#define CODECHAL_DECODE_JPEG_ERR_FRAME_HEIGHT       32

CodechalDecodeJpeg::~CodechalDecodeJpeg()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    m_osInterface->pfnDestroySyncResource(m_osInterface, &m_resSyncObjectWaContextInUse);
    m_osInterface->pfnDestroySyncResource(m_osInterface, &m_resSyncObjectVideoContextInUse);

    if (!Mos_ResourceIsNull(&m_resCopiedDataBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_resCopiedDataBuffer);
    }

#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_sfcState)
    {
        MOS_Delete(m_sfcState);
        m_sfcState = nullptr;
    }
#endif

    return;
}

CodechalDecodeJpeg::CodechalDecodeJpeg(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) : CodechalDecode(hwInterface, debugInterface, standardInfo),
                                            m_dataSize(0),
                                            m_dataOffset(0),
                                            m_copiedDataBufferSize(0),
                                            m_nextCopiedDataOffset(0),
                                            m_totalDataLength(0),
                                            m_preNumScans(0),
                                            m_copiedDataBufferInUse(false)

{
    CODECHAL_DECODE_FUNCTION_ENTER;

    MOS_ZeroMemory(&m_resCopiedDataBuffer, sizeof(m_resCopiedDataBuffer));
    MOS_ZeroMemory(&m_destSurface, sizeof(m_destSurface));
    MOS_ZeroMemory(&m_jpegHuffmanTable, sizeof(m_jpegHuffmanTable));
    MOS_ZeroMemory(&m_resDataBuffer, sizeof(m_resDataBuffer));
    MOS_ZeroMemory(&m_resSyncObjectWaContextInUse, sizeof(m_resSyncObjectWaContextInUse));
    MOS_ZeroMemory(&m_resSyncObjectVideoContextInUse, sizeof(m_resSyncObjectVideoContextInUse));
}

MOS_STATUS CodechalDecodeJpeg::InitializeBeginFrame()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    m_incompletePicture = false;
    m_incompleteJpegScan = false;
    m_copiedDataBufferInUse = false;
    m_nextCopiedDataOffset  = 0;
    m_totalDataLength       = 0;
    m_preNumScans           = 0;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeJpeg::CopyDataSurface()
{
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_hwInterface->m_noHuC)
    {
        uint32_t               alignedSize = MOS_ALIGN_CEIL(m_dataSize, 16);  // 16 byte aligned
        CodechalDataCopyParams dataCopyParams;
        MOS_ZeroMemory(&dataCopyParams, sizeof(CodechalDataCopyParams));
        dataCopyParams.srcResource = &m_resDataBuffer;
        dataCopyParams.srcSize = alignedSize;
        dataCopyParams.srcOffset = 0;
        dataCopyParams.dstResource = &m_resCopiedDataBuffer;
        dataCopyParams.dstSize = alignedSize;
        dataCopyParams.dstOffset   = m_nextCopiedDataOffset;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->CopyDataSourceWithDrv(
            &dataCopyParams));

        m_nextCopiedDataOffset += MOS_ALIGN_CEIL(m_dataSize, MHW_CACHELINE_SIZE);  // 64-byte aligned
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DECODE_CHK_COND_RETURN(
        ((m_nextCopiedDataOffset + m_dataSize) > m_copiedDataBufferSize),
        "Copied data buffer is not large enough.");

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(
        m_osInterface,
        m_videoContextForWa));
    m_osInterface->pfnResetOsStates(m_osInterface);

    m_osInterface->pfnSetPerfTag(
        m_osInterface,
        (uint16_t)(((m_mode << 4) & 0xF0) | COPY_TYPE));
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0));

    // Send command buffer header at the beginning (OS dependent)
    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &cmdBuffer,
        false));

    // Use huc stream out to do the copy
    CODECHAL_DECODE_CHK_STATUS_RETURN(HucCopy(
        &cmdBuffer,                // pCmdBuffer
        &m_resDataBuffer,          // presSrc
        &m_resCopiedDataBuffer,    // presDst
        m_dataSize,                // u32CopyLength
        0,                         // u32CopyInputOffset
        m_nextCopiedDataOffset));  // u32CopyOutputOffset

    m_nextCopiedDataOffset += MOS_ALIGN_CEIL(m_dataSize, MHW_CACHELINE_SIZE);

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &flushDwParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_incompletePicture)
    {
        MOS_SYNC_PARAMS syncParams;
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(
            m_osInterface,
            &syncParams));

        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContextForWa;
        syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(
            m_osInterface,
            &syncParams));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        m_videoContextForWaUsesNullHw));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(
        m_osInterface,
        m_videoContext));

    return eStatus;
}

MOS_STATUS CodechalDecodeJpeg::CheckAndCopyIncompleteBitStream()
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    uint32_t maxBufferSize =
        MOS_ALIGN_CEIL(m_jpegPicParams->m_frameWidth * m_jpegPicParams->m_frameHeight * 3, 64);

    if (m_jpegPicParams->m_totalScans == 1)  // Single scan
    {
        if (!m_incompleteJpegScan) // The first bitstream buffer
        {
            m_totalDataLength =
                m_jpegScanParams->ScanHeader[0].DataOffset + m_jpegScanParams->ScanHeader[0].DataLength;

            if (m_dataSize < m_totalDataLength)  // if the bitstream data is incomplete
            {
                CODECHAL_DECODE_CHK_COND_RETURN(
                    m_totalDataLength > maxBufferSize,
                    "The bitstream size exceeds the copied data buffer size.");

                CODECHAL_DECODE_CHK_COND_RETURN(
                    m_dataSize & 0x3f,
                    "The data size of the incomplete bitstream is not aligned with 64.");

                // Allocate the copy data buffer.
                if (Mos_ResourceIsNull(&m_resCopiedDataBuffer))
                {
                    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                                  &m_resCopiedDataBuffer,
                                                                  maxBufferSize,
                                                                  "CopiedDataBuffer"),
                        "Failed to allocate copied data Buffer.");
                }
                m_copiedDataBufferSize = maxBufferSize;

                // copy the bitstream buffer
                if (m_dataSize)
                {
                    CODECHAL_DECODE_CHK_STATUS_RETURN(CopyDataSurface());
                    m_copiedDataBufferInUse = true;
                }

                m_incompleteJpegScan = true;
                m_incompletePicture = true;
            }
            else //the bitstream data is complete
            {
                m_incompleteJpegScan = false;
                m_incompletePicture = false;
            }
        }
        else // the next bitstream buffers
        {
            CODECHAL_DECODE_CHK_COND_RETURN(
                m_nextCopiedDataOffset + m_dataSize > m_copiedDataBufferSize,
                "The bitstream size exceeds the copied data buffer size.")

            CODECHAL_DECODE_CHK_COND_RETURN(
                (m_nextCopiedDataOffset + m_dataSize < m_totalDataLength) && (m_dataSize & 0x3f),
                "The data size of the incomplete bitstream is not aligned with 64.");

            // copy the bitstream
            if (m_dataSize)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(CopyDataSurface());
            }

            if (m_nextCopiedDataOffset >= m_totalDataLength)
            {
                m_incompleteJpegScan = false;
                m_incompletePicture = false;
            }

        }
    }
    else  // multi-scans
    {
        if (!m_incompleteJpegScan) // The first bitstream buffer of each scan;
        {
            for (uint32_t idxScan = m_preNumScans; idxScan < m_jpegScanParams->NumScans; idxScan++)
            {
                m_jpegScanParams->ScanHeader[idxScan].DataOffset += m_nextCopiedDataOffset;  // modify the data offset for the new incoming scan data
            }
            m_totalDataLength = m_jpegScanParams->ScanHeader[m_jpegScanParams->NumScans - 1].DataOffset + m_jpegScanParams->ScanHeader[m_jpegScanParams->NumScans - 1].DataLength;
            m_preNumScans     = m_jpegScanParams->NumScans;

            // judge whether the bitstream is complete in the first execute() call
            if (IsFirstExecuteCall() &&
                m_dataSize <= m_jpegScanParams->ScanHeader[0].DataOffset + m_jpegScanParams->ScanHeader[0].DataLength)
            {
                CODECHAL_DECODE_CHK_COND_RETURN(
                    (m_nextCopiedDataOffset + m_dataSize < m_totalDataLength) && (m_dataSize & 0x3f),
                    "The buffer size of the incomplete bitstream is not aligned with 64.");

                // Allocate the copy data buffer.
                if (Mos_ResourceIsNull(&m_resCopiedDataBuffer))
                {
                    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                                  &m_resCopiedDataBuffer,
                                                                  maxBufferSize,
                                                                  "CopiedDataBuffer"),
                        "Failed to allocate copied data Buffer.");
                }
                m_copiedDataBufferSize = maxBufferSize;

                // copy the bitstream buffer
                if (m_dataSize)
                {
                    CODECHAL_DECODE_CHK_STATUS_RETURN(CopyDataSurface());
                    m_copiedDataBufferInUse = true;
                }

                m_incompleteJpegScan = m_nextCopiedDataOffset < m_totalDataLength;
                m_incompletePicture  = m_incompleteJpegScan || m_jpegScanParams->NumScans < m_jpegPicParams->m_totalScans;
            }
            else // the bitstream is complete
            {
                m_incompleteJpegScan = false;
                if (m_jpegScanParams->NumScans == m_jpegPicParams->m_totalScans)
                {
                    m_incompletePicture = false;
                }
                else
                {
                    m_incompletePicture = true;
                }
            }
        }
        else //The next bitstream buffer of each scan
        {
            CODECHAL_DECODE_CHK_COND_RETURN(
                m_nextCopiedDataOffset + m_dataSize > m_copiedDataBufferSize,
                "The bitstream size exceeds the copied data buffer size.")

            CODECHAL_DECODE_CHK_COND_RETURN(
                (m_nextCopiedDataOffset + m_dataSize < m_totalDataLength) && (m_dataSize & 0x3f),
                "The data size of the incomplete bitstream is not aligned with 64.");

            // copy the bitstream buffer
            if (m_dataSize)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(CopyDataSurface());
            }

            if (m_nextCopiedDataOffset >= m_totalDataLength)
            {
                m_incompleteJpegScan = false;
                if (m_jpegScanParams->NumScans >= m_jpegPicParams->m_totalScans)
                {
                    m_incompletePicture = false;
                }
            }

        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeJpeg::CheckSupportedFormat(
    PMOS_FORMAT format)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // App must use 420_OPAQUE as DecodeRT for other JPEG output formats except NV12 and YUY2 due to runtime
    // restriction, so the real JPEG format is passed to driver in PPS data. The code here is just to get the real output format.
    // On SKL+, app would use AYUV (instead of 420_OPAQUE) as DecodeRT for direct YUV to ARGB8888 conversion; in such case,
    // real output format (ARGB8888) should also be from JPEG PPS; MSDK would handle the details of treating AYUV as ARGB.
    if (*format == Format_420O || *format == Format_AYUV)
    {
        *format = m_osInterface->pfnFmt_OsToMos((MOS_OS_FORMAT)m_jpegPicParams->m_renderTargetFormat);
    }

    //No support for RGBP/BGRP channel swap or YUV/RGB conversion!
    switch (*format)
    {
    case Format_BGRP:
        if (m_jpegPicParams->m_chromaType == jpegRGB ||
            m_jpegPicParams->m_chromaType == jpegYUV444)
        {
            eStatus = MOS_STATUS_PLATFORM_NOT_SUPPORTED;
        }
        break;
    case Format_RGBP:
        if (m_jpegPicParams->m_chromaType == jpegYUV444)
        {
            eStatus = MOS_STATUS_PLATFORM_NOT_SUPPORTED;
        }
        break;
    case Format_Y416:
    case Format_AYUV:
    case Format_AUYV:
    case Format_Y410:
        if (m_jpegPicParams->m_chromaType == jpegRGB ||
            m_jpegPicParams->m_chromaType == jpegBGR)
        {
            eStatus = MOS_STATUS_PLATFORM_NOT_SUPPORTED;
        }
        break;
    default:
        break;
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeJpeg::SetFrameStates()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_destSurface);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_dataBuffer);

    //Set wPerfType as I_TYPE so that PerTag can be recognized by performance reportor
    m_perfType = I_TYPE;

    m_dataSize         = m_decodeParams.m_dataSize;
    m_dataOffset       = m_decodeParams.m_dataOffset;
    m_resDataBuffer    = *(m_decodeParams.m_dataBuffer);
    m_jpegPicParams    = (CodecDecodeJpegPicParams *)m_decodeParams.m_picParams;
    m_jpegQMatrix      = (CodecJpegQuantMatrix *)m_decodeParams.m_iqMatrixBuffer;
    m_jpegHuffmanTable = (PCODECHAL_DECODE_JPEG_HUFFMAN_TABLE)m_decodeParams.m_huffmanTable;
    m_jpegScanParams   = (CodecDecodeJpegScanParameter *)m_decodeParams.m_sliceParams;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_jpegPicParams);

    CODECHAL_DECODE_CHK_STATUS_RETURN(CheckSupportedFormat(
        &m_decodeParams.m_destSurface->Format));

    m_hwInterface->GetCpInterface()->SetCpSecurityType();

    if (IsFirstExecuteCall())
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(InitializeBeginFrame());
    }

    // Check whether the bitstream buffer is completed. If not, allocate a larger buffer and copy the bitstream.
    CODECHAL_DECODE_CHK_STATUS_RETURN(CheckAndCopyIncompleteBitStream());

    // if the bitstream is not completed, don't do any decoding work.
    if (m_incompletePicture)
    {
        return MOS_STATUS_SUCCESS;
    }

    uint32_t widthAlign = 0;
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
        default: // YUV422H_4Y, YUV422V_4Y & YUV420
            widthAlign  = MOS_ALIGN_CEIL(m_jpegPicParams->m_frameWidth, CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE_X2);
            heightAlign = MOS_ALIGN_CEIL(m_jpegPicParams->m_frameHeight, CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE_X2);
            break;
        }
    }

    //BDW has a limitation:Height should aligned by 16 when input is YUV422H_2Y and output is NV12.
    if (MEDIA_IS_WA(m_waTable, WaJPEGHeightAlignYUV422H2YToNV12) &&
        m_jpegPicParams->m_chromaType == jpegYUV422H2Y &&
        m_decodeParams.m_destSurface->Format == Format_NV12)
    {
        heightAlign = MOS_ALIGN_CEIL(m_jpegPicParams->m_frameHeight, CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE_X2);
    }

    if ((m_jpegPicParams->m_rotation == jpegRotation90) || (m_jpegPicParams->m_rotation == jpegRotation270))
    {
        // Interchanging picture width and height for 90/270 degree rotation
        m_decodeParams.m_destSurface->dwWidth  = heightAlign;
        m_decodeParams.m_destSurface->dwHeight = widthAlign;
    }
    else
    {
        m_decodeParams.m_destSurface->dwWidth  = widthAlign;
        m_decodeParams.m_destSurface->dwHeight = heightAlign;
    }

    m_destSurface = *(m_decodeParams.m_destSurface);
    if (m_copiedDataBufferInUse)
    {
        m_resDataBuffer = m_resCopiedDataBuffer;  // set resDataBuffer to copy data buffer
    }

    m_statusReportFeedbackNumber = m_jpegPicParams->m_statusReportFeedbackNumber;

#ifdef _DECODE_PROCESSING_SUPPORTED
    m_sfcState->CheckAndInitialize(&m_destSurface, m_jpegPicParams);
#endif

    CODECHAL_DEBUG_TOOL(
        if (m_jpegPicParams) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpPicParams(m_jpegPicParams))
        }

        if (m_jpegScanParams) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpScanParams(m_jpegScanParams))
        }

        if (m_jpegHuffmanTable) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpHuffmanTable(m_jpegHuffmanTable))
        }

        if (m_jpegQMatrix) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpIQParams(m_jpegQMatrix))
        }

        if (&(m_resDataBuffer)) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_resDataBuffer,
                CodechalDbgAttr::attrBitstream,
                "_DEC",
                m_dataSize,
                0,
                CODECHAL_NUM_MEDIA_STATES));
        })

    return eStatus;
}

MOS_STATUS CodechalDecodeJpeg::AllocateResources()
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface,
        &m_resSyncObjectWaContextInUse));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface,
        &m_resSyncObjectVideoContextInUse));

    return eStatus;
}

void CodechalDecodeJpeg::SetOutputSurfaceLayout(
    CodecDecodeJpegImageLayout *outputSurfLayout)
{
    uint32_t ucbOffset = MOS_ALIGN_CEIL(m_destSurface.UPlaneOffset.iYOffset, MHW_VDBOX_MFX_UV_PLANE_ALIGNMENT_LEGACY);
    uint32_t vcrOffset = MOS_ALIGN_CEIL(m_destSurface.VPlaneOffset.iYOffset, MHW_VDBOX_MFX_UV_PLANE_ALIGNMENT_LEGACY);

    uint32_t ucbOffsetInBytes = ucbOffset * m_destSurface.dwPitch;
    uint32_t vcrOffsetInBytes = vcrOffset * m_destSurface.dwPitch;

    outputSurfLayout->m_pitch = m_destSurface.dwPitch;

    for (uint32_t scanCount = 0; scanCount < m_jpegScanParams->NumScans; scanCount++)
    {
        for (uint32_t scanComponent = 0; scanComponent < m_jpegScanParams->ScanHeader[scanCount].NumComponents; scanComponent++)
        {
            if (m_jpegScanParams->ScanHeader[scanCount].ComponentSelector[scanComponent] == m_jpegPicParams->m_componentIdentifier[jpegComponentY])
            {
                outputSurfLayout->m_componentDataOffset[jpegComponentY] = 0;
            }
            else if (m_jpegScanParams->ScanHeader[scanCount].ComponentSelector[scanComponent] == m_jpegPicParams->m_componentIdentifier[jpegComponentU])
            {
                outputSurfLayout->m_componentDataOffset[jpegComponentU] = ucbOffsetInBytes;
            }
            else if (m_jpegScanParams->ScanHeader[scanCount].ComponentSelector[scanComponent] == m_jpegPicParams->m_componentIdentifier[jpegComponentV])
            {
                outputSurfLayout->m_componentDataOffset[jpegComponentV] = vcrOffsetInBytes;
            }
        }
    }
}

MOS_STATUS CodechalDecodeJpeg::DecodeStateLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    MHW_VDBOX_JPEG_DECODE_PIC_STATE jpegPicState;
    jpegPicState.dwOutputFormat = m_decodeParams.m_destSurface->Format;

#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_sfcState->m_sfcPipeOut)
    {
        jpegPicState.dwOutputFormat = m_sfcState->m_sfcInSurface.Format;
    }
#endif

    //Three new formats from HSW C0,HSW ULT can only be supported in specific conditions.
    if (jpegPicState.dwOutputFormat == Format_NV12 ||
        jpegPicState.dwOutputFormat == Format_YUY2 ||
        jpegPicState.dwOutputFormat == Format_UYVY)
    {
        //Only interleaved single scan are supported.
        if (m_jpegPicParams->m_totalScans != 1 ||
            m_jpegPicParams->m_interleavedData == 0)
        {
            return MOS_STATUS_UNKNOWN;
        }

        switch (m_jpegPicParams->m_chromaType)
        {
        case jpegYUV420:
        case jpegYUV422H2Y:
        case jpegYUV422H4Y:
            break;
        case jpegYUV422V2Y:
        case jpegYUV422V4Y:
            if (GFX_IS_GEN_8_OR_LATER(m_hwInterface->GetPlatform()) &&
                jpegPicState.dwOutputFormat == Format_NV12)
            {
                break;
            }
        default:
            return MOS_STATUS_UNKNOWN;
        }
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0));

    auto mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);
    HalOcaInterface::On1stLevelBBStart(cmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &cmdBuffer, true));

    // Set PIPE_MODE_SELECT
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode                       = CODECHAL_DECODE_MODE_JPEG;
    pipeModeSelectParams.bStreamOutEnabled          = m_streamOutEnabled;
    pipeModeSelectParams.bDeblockerStreamOutEnable  = false;
    pipeModeSelectParams.bPostDeblockOutEnable      = false;
    pipeModeSelectParams.bPreDeblockOutEnable       = true;

    // Set CMD_MFX_SURFACE_STATE
    MHW_VDBOX_SURFACE_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.Mode          = CODECHAL_DECODE_MODE_JPEG;
    surfaceParams.psSurface     = &m_destSurface;
    surfaceParams.ChromaType    = m_jpegPicParams->m_chromaType;

#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_sfcState->m_sfcPipeOut)
    {
        surfaceParams.psSurface = &m_sfcState->m_sfcInSurface;
    }
#endif

    // Set MFX_PIPE_BUF_ADDR_STATE_CMD
    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams;
    pipeBufAddrParams.Mode = CODECHAL_DECODE_MODE_JPEG;
    // Predeblock surface is the same as destination surface here because there is no deblocking for JPEG
    pipeBufAddrParams.psPreDeblockSurface = &m_destSurface;

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetPipeBufAddr(&pipeBufAddrParams));
#endif

    // Set MFX_IND_OBJ_BASE_ADDR_STATE_CMD
    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode = CODECHAL_DECODE_MODE_JPEG;
    indObjBaseAddrParams.dwDataSize     = m_copiedDataBufferInUse ? m_nextCopiedDataOffset : m_dataSize;
    indObjBaseAddrParams.presDataBuffer = &m_resDataBuffer;

    // Set MFX_JPEG_PIC_STATE_CMD
    jpegPicState.pJpegPicParams = m_jpegPicParams;
    if ((m_jpegPicParams->m_rotation == jpegRotation90) || (m_jpegPicParams->m_rotation == jpegRotation270))
    {
        jpegPicState.dwWidthInBlocks  = (m_destSurface.dwHeight / CODECHAL_DECODE_JPEG_BLOCK_SIZE) - 1;
        jpegPicState.dwHeightInBlocks = (m_destSurface.dwWidth / CODECHAL_DECODE_JPEG_BLOCK_SIZE) - 1;
    }
    else
    {
        jpegPicState.dwWidthInBlocks  = (m_destSurface.dwWidth / CODECHAL_DECODE_JPEG_BLOCK_SIZE) - 1;
        jpegPicState.dwHeightInBlocks = (m_destSurface.dwHeight / CODECHAL_DECODE_JPEG_BLOCK_SIZE) - 1;
    }

    // Add commands to command buffer
    // MI_FLUSH_DW command -> must be before to MFX_PIPE_MODE_SELECT
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &flushDwParams));

    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(StartStatusReport(
            &cmdBuffer));
    }

    // MFX_PIPE_MODE_SELECT_CMD
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeModeSelectCmd(
        &cmdBuffer,
        &pipeModeSelectParams));

#ifdef _DECODE_PROCESSING_SUPPORTED
    // Output decode result through SFC
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_sfcState->AddSfcCommands(&cmdBuffer));
#endif

    // CMD_MFX_SURFACE_STATE
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxSurfaceCmd(
        &cmdBuffer,
        &surfaceParams));

    // MFX_PIPE_BUF_ADDR_STATE_CMD
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeBufAddrCmd(
        &cmdBuffer,
        &pipeBufAddrParams));

    // MFX_IND_OBJ_BASE_ADDR_STATE_CMD
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxIndObjBaseAddrCmd(
        &cmdBuffer,
        &indObjBaseAddrParams));

    // MFX_JPEG_PIC_STATE_CMD
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxJpegPicCmd(
        &cmdBuffer,
        &jpegPicState));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return eStatus;
}

MOS_STATUS CodechalDecodeJpeg::DecodePrimitiveLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    // if the bitstream is not complete, don't do any decoding work.
    if (m_incompletePicture)
    {
        return MOS_STATUS_SUCCESS;

    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0));

    // MFX_QM_STATE_CMD
    MHW_VDBOX_QM_PARAMS qmParams;
    MOS_ZeroMemory(&qmParams, sizeof(qmParams));
    qmParams.Standard = CODECHAL_JPEG;
    qmParams.pJpegQuantMatrix = (CodecJpegQuantMatrix *)m_jpegQMatrix;

    // Swapping QM(x,y) to QM(y,x) for 90/270 degree rotation
    if ((m_jpegPicParams->m_rotation == jpegRotation90) ||
        (m_jpegPicParams->m_rotation == jpegRotation270))
    {
        qmParams.bJpegQMRotation = true;
    }
    else
    {
        qmParams.bJpegQMRotation = false;
    }

    for (uint16_t scanCount = 0; scanCount < m_jpegPicParams->m_numCompInFrame; scanCount++)
    {
        // Using scanCount here because the same command is used for JPEG decode and encode
        uint32_t quantTableSelector                                      = m_jpegPicParams->m_quantTableSelector[scanCount];
        qmParams.pJpegQuantMatrix->m_jpegQMTableType[quantTableSelector] = scanCount;
        qmParams.JpegQMTableSelector = quantTableSelector;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxQmCmd(
            &cmdBuffer,
            &qmParams));
    }

    uint32_t dcCurHuffTblIndex[2] = { 0xff, 0xff };
    uint32_t acCurHuffTblIndex[2] = { 0xff, 0xff };

    for (uint16_t scanCount = 0; scanCount < m_jpegScanParams->NumScans; scanCount++)
    {
        // MFX_JPEG_HUFF_TABLE
        uint16_t numComponents = m_jpegScanParams->ScanHeader[scanCount].NumComponents;
        for (uint16_t scanComponent = 0; scanComponent < numComponents; scanComponent++)
        {
            // Determine which huffman table we will be writing to
            // For gray image, componentIdentifier[jpegComponentU] and componentIdentifier[jpegComponentV] are initialized to 0,
            // and when ComponentSelector[scanComponent] is equal 0, variable huffTableID is set to 1, and wrong Huffman table is used,
            // so it is more reasonable to use componentIdentifier[jpegComponentY] to determine which huffman table we will be writing to.
            uint8_t ComponentSelector =
                m_jpegScanParams->ScanHeader[scanCount].ComponentSelector[scanComponent];
            uint16_t huffTableID = 0;
            if (ComponentSelector == m_jpegPicParams->m_componentIdentifier[jpegComponentY])
            {
                huffTableID = 0;
            }
            else
            {
                huffTableID = 1;
            }

            uint32_t AcTableSelector =
                m_jpegScanParams->ScanHeader[scanCount].AcHuffTblSelector[scanComponent];
            uint32_t DcTableSelector =
                m_jpegScanParams->ScanHeader[scanCount].DcHuffTblSelector[scanComponent];

            // Send the huffman table state command only if the table changed
            if ((DcTableSelector != dcCurHuffTblIndex[huffTableID]) ||
                (AcTableSelector != acCurHuffTblIndex[huffTableID]))
            {
                MHW_VDBOX_HUFF_TABLE_PARAMS huffmanTableParams;
                MOS_ZeroMemory(&huffmanTableParams, sizeof(huffmanTableParams));

                huffmanTableParams.HuffTableID = huffTableID;

                huffmanTableParams.pACBits   = &m_jpegHuffmanTable->HuffTable[AcTableSelector].AC_BITS[0];
                huffmanTableParams.pDCBits   = &m_jpegHuffmanTable->HuffTable[DcTableSelector].DC_BITS[0];
                huffmanTableParams.pACValues = &m_jpegHuffmanTable->HuffTable[AcTableSelector].AC_HUFFVAL[0];
                huffmanTableParams.pDCValues = &m_jpegHuffmanTable->HuffTable[DcTableSelector].DC_HUFFVAL[0];

                CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxJpegHuffTableCmd(
                    &cmdBuffer,
                    &huffmanTableParams));

                // Set the current huffman table indices for the next scan
                dcCurHuffTblIndex[huffTableID] = DcTableSelector;
                acCurHuffTblIndex[huffTableID] = AcTableSelector;
            }
        }

        MHW_VDBOX_JPEG_BSD_PARAMS jpegBsdObject;
        MOS_ZeroMemory(&jpegBsdObject, sizeof(jpegBsdObject));

        // MFX_JPEG_BSD_OBJECT
        jpegBsdObject.dwIndirectDataLength     = m_jpegScanParams->ScanHeader[scanCount].DataLength;
        jpegBsdObject.dwDataStartAddress       = m_jpegScanParams->ScanHeader[scanCount].DataOffset;
        jpegBsdObject.dwScanHorizontalPosition = m_jpegScanParams->ScanHeader[scanCount].ScanHoriPosition;
        jpegBsdObject.dwScanVerticalPosition   = m_jpegScanParams->ScanHeader[scanCount].ScanVertPosition;
        jpegBsdObject.bInterleaved = (numComponents > 1) ? 1 : 0;
        jpegBsdObject.dwMCUCount               = m_jpegScanParams->ScanHeader[scanCount].MCUCount;
        jpegBsdObject.dwRestartInterval        = m_jpegScanParams->ScanHeader[scanCount].RestartInterval;

        uint16_t scanComponentIndex = 0;

        for (uint16_t scanComponent = 0; scanComponent < numComponents; scanComponent++)
        {
            uint8_t ComponentSelector =
                m_jpegScanParams->ScanHeader[scanCount].ComponentSelector[scanComponent];

            if (ComponentSelector == m_jpegPicParams->m_componentIdentifier[jpegComponentY])
            {
                scanComponentIndex = 0;
            }
            else if (ComponentSelector == m_jpegPicParams->m_componentIdentifier[jpegComponentU])
            {
                scanComponentIndex = 1;
            }
            else if (ComponentSelector == m_jpegPicParams->m_componentIdentifier[jpegComponentV])
            {
                scanComponentIndex = 2;
            }
            // Add logic for component identifier JPEG_A

            jpegBsdObject.sScanComponent |= (1 << scanComponentIndex);
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxJpegBsdObjCmd(
            &cmdBuffer,
            &jpegBsdObject));
    }

    // Check if destination surface needs to be synchronized
    MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_videoContext;
    syncParams.presSyncResource         = &m_destSurface.OsResource;
    syncParams.bReadOnly = false;
    syncParams.bDisableDecodeSyncLock = m_disableDecodeSyncLock;
    syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(
        m_osInterface,
        &syncParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(
        m_osInterface,
        &syncParams));

    // Update the resource tag (s/w tag) for On-Demand Sync
    m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &flushDwParams));

    // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
    if (m_osInterface->bTagResourceSync)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(
            &cmdBuffer,
            &syncParams));
    }

    if (m_statusQueryReportingEnabled)
    {
        CodechalDecodeStatusReport decodeStatusReport;
        decodeStatusReport.m_statusReportNumber = m_statusReportFeedbackNumber;
        decodeStatusReport.m_codecStatus = CODECHAL_STATUS_UNAVAILABLE;
        decodeStatusReport.m_currDecodedPicRes  = m_destSurface.OsResource;

        CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(
            decodeStatusReport,
            &cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_DEC"));
    )

    if (m_copiedDataBufferInUse)
    {
        //Sync up complete frame
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContextForWa;
        syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(
            m_osInterface,
            &syncParams));

        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(
            m_osInterface,
            &syncParams));
    }

    HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        m_videoContextUsesNullHw));

    CODECHAL_DEBUG_TOOL(
        m_mmc->UpdateUserFeatureKey(&m_destSurface);)

    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(
            m_videoContextUsesNullHw));
        }

    // Set output surface layout
    SetOutputSurfaceLayout(&m_decodeParams.m_outputSurfLayout);

    // Send the signal to indicate decode completion, in case On-Demand Sync is not present
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(
        m_osInterface,
        &syncParams));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            &m_destSurface,
            CodechalDbgAttr::attrDecodeOutputSurface,
            "DstSurf"));)
    return eStatus;
}

MOS_STATUS CodechalDecodeJpeg::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    m_mmc = MOS_New(CodechalMmcDecodeJpeg, m_hwInterface, this);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_mmc);
#endif

    return MOS_STATUS_SUCCESS;
}

#ifdef _DECODE_PROCESSING_SUPPORTED
MOS_STATUS CodechalDecodeJpeg::InitSfcState()
{
    m_sfcState = MOS_New(CodechalJpegSfcState);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_sfcState);

    return MOS_STATUS_SUCCESS;
}
#endif

MOS_STATUS CodechalDecodeJpeg::AllocateStandard(
    CodechalSetting *settings)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(settings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitMmcState());
#ifdef _DECODE_PROCESSING_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(InitSfcState());
#endif

    m_width = settings->width;
    m_height = settings->height;

#ifdef _DECODE_PROCESSING_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_sfcState->InitializeSfcState(
        this,
        m_hwInterface,
        m_osInterface));
#endif

    CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateResources());

    return eStatus;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalDecodeJpeg::DumpIQParams(
    CodecJpegQuantMatrix *matrixData)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrIqParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(matrixData);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    for (uint32_t j = 0; j < jpegNumComponent; j++)
    {
        oss << "Qmatrix " << std::dec << +j << ": " << std::endl;

        for (int8_t i = 0; i < 56; i += 8)
        {
            oss << "Qmatrix[" << std::dec << +i / 8 << "]:";
            for (uint8_t k = 0; k < 8; k++)
                oss << std::hex << +matrixData->m_quantMatrix[j][i + k]<< " ";
            oss << std::endl;
        }
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufIqParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeJpeg::DumpPicParams(
    CodecDecodeJpegPicParams *picParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);
    oss.setf(std::ios::hex, std::ios::basefield);

    oss << "destPic.FrameIdx: " << +picParams->m_destPic.FrameIdx << std::endl;
    oss << "destPic.PicFlags: " << +picParams->m_destPic.PicFlags << std::endl;
    oss << "frameWidth: " << +picParams->m_frameWidth << std::endl;
    oss << "frameHeight: " << +picParams->m_frameHeight << std::endl;
    oss << "numCompInFrame: " << +picParams->m_numCompInFrame << std::endl;

    //Dump componentIdentifier[jpegNumComponent]
    for (uint32_t i = 0; i < jpegNumComponent; ++i)
    {
        oss << "componentIdentifier[" << +i << "]: " << +picParams->m_componentIdentifier[i] << std::endl;
    }

    //Dump quantTableSelector[jpegNumComponent]
    for (uint32_t i = 0; i < jpegNumComponent; ++i)
    {
        oss << "quantTableSelector[" << +i << "]: " << +picParams->m_quantTableSelector[i] << std::endl;
    }
    oss << "chromaType: " << +picParams->m_chromaType << std::endl;
    oss << "rotation: " << +picParams->m_rotation << std::endl;
    oss << "totalScans: " << +picParams->m_totalScans << std::endl;
    oss << "interleavedData: " << +picParams->m_interleavedData << std::endl;
    oss << "reserved: " << +picParams->m_reserved << std::endl;
    oss << "statusReportFeedbackNumber: " << +picParams->m_statusReportFeedbackNumber << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeJpeg::DumpScanParams(
    CodecDecodeJpegScanParameter *scanParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrScanParams))
    {
        return MOS_STATUS_SUCCESS;
    }
    CODECHAL_DEBUG_CHK_NULL(scanParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    //Dump ScanHeader[jpegNumComponent]
    for (uint32_t i = 0; i < jpegNumComponent; ++i)
    {
        oss << "ScanHeader[" << +i << "].NumComponents: " << +scanParams->ScanHeader[i].NumComponents << std::endl;
        //Dump ComponentSelector[jpegNumComponent]
        for (uint32_t j = 0; j < jpegNumComponent; ++j)
        {
            oss << "ScanHeader[" << +i << "].ComponentSelector[" << +j << "]: " << +scanParams->ScanHeader[i].ComponentSelector[j] << std::endl;
        }

        //Dump DcHuffTblSelector[jpegNumComponent]
        for (uint32_t j = 0; j < jpegNumComponent; ++j)
        {
            oss << "ScanHeader[" << +i << "].DcHuffTblSelector[" << +j << "]: " << +scanParams->ScanHeader[i].DcHuffTblSelector[j] << std::endl;
        }

        //Dump AcHuffTblSelector[jpegNumComponent]
        for (uint32_t j = 0; j < jpegNumComponent; ++j)
        {
            oss << "ScanHeader[" << +i << "].AcHuffTblSelector[" << +j << "]: " << +scanParams->ScanHeader[i].AcHuffTblSelector[j] << std::endl;
        }
        oss << "ScanHeader[" << +i << "].RestartInterval: " << +scanParams->ScanHeader[i].RestartInterval << std::endl;
        oss << "ScanHeader[" << +i << "].MCUCount: " << +scanParams->ScanHeader[i].MCUCount << std::endl;
        oss << "ScanHeader[" << +i << "].ScanHoriPosition: " << +scanParams->ScanHeader[i].ScanHoriPosition << std::endl;
        oss << "ScanHeader[" << +i << "].ScanVertPosition: " << +scanParams->ScanHeader[i].ScanVertPosition << std::endl;
        oss << "ScanHeader[" << +i << "].DataOffset: " << +scanParams->ScanHeader[i].DataOffset << std::endl;
        oss << "ScanHeader[" << +i << "].DataLength: " << +scanParams->ScanHeader[i].DataLength << std::endl;
    }

    oss << "NumScans: " << +scanParams->NumScans << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufScanParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeJpeg::DumpHuffmanTable(
    PCODECHAL_DECODE_JPEG_HUFFMAN_TABLE huffmanTable)
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
    for (uint32_t i = 0; i < JPEG_MAX_NUM_HUFF_TABLE_INDEX; ++i)
    {
        //Dump DC_BITS[JPEG_NUM_HUFF_TABLE_DC_BITS]
        oss << "HuffTable[" << +i << "].DC_BITS[0-" << (JPEG_NUM_HUFF_TABLE_DC_BITS - 1) << "]: " << std::endl;

        for (uint32_t j = 0; j < JPEG_NUM_HUFF_TABLE_DC_BITS; ++j)
        {
            oss << +huffmanTable->HuffTable[i].DC_BITS[j] << " ";
            if (j % 6 == 5 || j == JPEG_NUM_HUFF_TABLE_DC_BITS - 1)
            {
                oss << std::endl;
            }
        }
        //Dump DC_HUFFVAL[JPEG_NUM_HUFF_TABLE_DC_HUFFVAL]
        oss << "HuffTable[" << +i << "].DC_HUFFVAL[0-" << (JPEG_NUM_HUFF_TABLE_DC_HUFFVAL - 1) << "]: " << std::endl;
        for (uint32_t j = 0; j < JPEG_NUM_HUFF_TABLE_DC_HUFFVAL; ++j)
        {
            oss << +huffmanTable->HuffTable[i].DC_HUFFVAL[j] << ' ';
            if (j % 6 == 5 || j == JPEG_NUM_HUFF_TABLE_DC_HUFFVAL - 1)
            {
                oss << std::endl;
            }
        }
        //Dump AC_BITS[JPEG_NUM_HUFF_TABLE_AC_BITS]
        oss << "HuffTable[" << +i << "].AC_BITS[0-" << (JPEG_NUM_HUFF_TABLE_AC_BITS - 1) << "]: " << std::endl;

        for (uint32_t j = 0; j < JPEG_NUM_HUFF_TABLE_AC_BITS; ++j)
        {
            oss << +huffmanTable->HuffTable[i].AC_BITS[j] << ' ';
            if (j % 8 == 7 || j == JPEG_NUM_HUFF_TABLE_AC_BITS - 1)
            {
                oss << std::endl;
            }
        }

        //Dump AC_HUFFVAL[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL]
        oss << "HuffTable[" << +i << "].AC_HUFFVAL[0-" << (JPEG_NUM_HUFF_TABLE_AC_HUFFVAL - 1) << "]: " << std::endl;

        for (uint32_t j = 0; j < JPEG_NUM_HUFF_TABLE_AC_HUFFVAL; ++j)
        {
            oss << +huffmanTable->HuffTable[i].AC_HUFFVAL[j] << ' ';
            if (j % 9 == 8 || j == JPEG_NUM_HUFF_TABLE_AC_HUFFVAL - 1)
            {
                oss << std::endl;
            }
        }
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufHuffmanTbl,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

#endif
