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
//! \file     decode_jpeg_picture_xe_m_base_packet.cpp
//! \brief    Defines the interface for jpeg decode picture packet
//!
#include "codechal_utilities.h"
#include "decode_jpeg_picture_xe_m_base_packet.h"
#include "codechal_debug.h"
#include "decode_common_feature_defs.h"

namespace decode{

MOS_STATUS JpegDecodePicPktXe_M_Base::Init()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_hwInterface);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_miInterface);
    DECODE_CHK_NULL(m_jpegPipeline);
    DECODE_CHK_NULL(m_mfxInterface);

    m_jpegBasicFeature = dynamic_cast<JpegBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_jpegBasicFeature);

#ifdef _DECODE_PROCESSING_SUPPORTED
    m_downSamplingFeature      = dynamic_cast<DecodeDownSamplingFeature *>(m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
    DecodeSubPacket *subPacket = m_jpegPipeline->GetSubPacket(DecodePacketId(m_jpegPipeline, downSamplingSubPacketId));
    m_downSamplingPkt          = dynamic_cast<DecodeDownSamplingPkt *>(subPacket);
#endif

    m_allocator = m_pipeline ->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    DECODE_CHK_STATUS(AllocateFixedResources());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPktXe_M_Base::Prepare()
{
    DECODE_FUNC_CALL();

    m_jpegPicParams = m_jpegBasicFeature->m_jpegPicParams;
    DECODE_CHK_NULL(m_jpegPicParams);

#ifdef _MMC_SUPPORTED
    m_mmcState = m_jpegPipeline->GetMmcState();
    DECODE_CHK_NULL(m_mmcState);
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPktXe_M_Base::AllocateFixedResources()
{
    DECODE_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

void JpegDecodePicPktXe_M_Base::SetMfxPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 &pipeModeSelectParams)
{
    DECODE_FUNC_CALL();

    pipeModeSelectParams.Mode                      = m_jpegBasicFeature->m_mode;
    pipeModeSelectParams.bStreamOutEnabled         = false; //Is here correct?
    pipeModeSelectParams.bDeblockerStreamOutEnable = false;
    pipeModeSelectParams.bPostDeblockOutEnable     = false;
    pipeModeSelectParams.bPreDeblockOutEnable      = true;
}

MOS_STATUS JpegDecodePicPktXe_M_Base::SetMfxSurfaceParams(MHW_VDBOX_SURFACE_PARAMS &dstSurfaceParams)
{
    DECODE_FUNC_CALL();

    MOS_ZeroMemory(&dstSurfaceParams, sizeof(dstSurfaceParams));
    dstSurfaceParams.Mode      = m_jpegBasicFeature->m_mode;
    dstSurfaceParams.psSurface = &m_jpegBasicFeature->m_destSurface;
    dstSurfaceParams.ChromaType = m_jpegBasicFeature->m_chromaFormat;

#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(m_mmcState->SetSurfaceMmcState(&(m_jpegBasicFeature->m_destSurface)));
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(dstSurfaceParams.psSurface, &dstSurfaceParams.mmcState));
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcFormat(dstSurfaceParams.psSurface, &dstSurfaceParams.dwCompressionFormat));
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPktXe_M_Base::AddMfxSurfacesCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_SURFACE_PARAMS dstSurfaceParams;
    DECODE_CHK_STATUS(SetMfxSurfaceParams(dstSurfaceParams));
    DECODE_CHK_STATUS(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &dstSurfaceParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPktXe_M_Base::SetMfxPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams)
{
    DECODE_FUNC_CALL();

    pipeBufAddrParams.Mode = m_jpegBasicFeature->m_mode;
    //Predeblock surface is the same as destination surface here because there is no deblocking for JPEG
    pipeBufAddrParams.psPreDeblockSurface = &m_jpegBasicFeature->m_destSurface;

#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(pipeBufAddrParams.psPreDeblockSurface, &pipeBufAddrParams.PreDeblockSurfMmcState));
    if (m_mmcState->IsMmcEnabled())
    {
        pipeBufAddrParams.bMmcEnabled = true;
    }
#endif

    CODECHAL_DEBUG_TOOL(DumpResources(pipeBufAddrParams));

    return MOS_STATUS_SUCCESS;
}

void JpegDecodePicPktXe_M_Base::SetMfxIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS &indObjBaseAddrParams)
{
    DECODE_FUNC_CALL();

    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode            = m_jpegBasicFeature->m_mode;;
    indObjBaseAddrParams.dwDataSize      = m_jpegBasicFeature->m_dataSize;
    indObjBaseAddrParams.presDataBuffer  = &(m_jpegBasicFeature->m_resDataBuffer.OsResource);
}

MOS_STATUS JpegDecodePicPktXe_M_Base::AddMfxIndObjBaseAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    SetMfxIndObjBaseAddrParams(indObjBaseAddrParams);
    DECODE_CHK_STATUS(m_mfxInterface->AddMfxIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

    return MOS_STATUS_SUCCESS;
}

void JpegDecodePicPktXe_M_Base::SetMfxJpegPicStateParams(MHW_VDBOX_JPEG_DECODE_PIC_STATE &jpegPicState)
{
    DECODE_FUNC_CALL();

    MOS_ZeroMemory(&jpegPicState, sizeof(jpegPicState));
    jpegPicState.Mode           = m_jpegBasicFeature->m_mode;
    jpegPicState.pJpegPicParams = m_jpegBasicFeature->m_jpegPicParams;
    jpegPicState.dwOutputFormat = m_jpegBasicFeature->m_destSurface.Format;

    if (m_jpegBasicFeature->m_jpegPicParams->m_rotation == jpegRotation90 || m_jpegBasicFeature->m_jpegPicParams->m_rotation == jpegRotation270)
    {
        jpegPicState.dwWidthInBlocks = (m_jpegBasicFeature->m_destSurface.dwHeight / CODEC_DECODE_JPEG_BLOCK_SIZE) - 1;
        jpegPicState.dwHeightInBlocks = (m_jpegBasicFeature->m_destSurface.dwWidth / CODEC_DECODE_JPEG_BLOCK_SIZE) - 1;
    }
    else
    {
        jpegPicState.dwWidthInBlocks  = (m_jpegBasicFeature->m_destSurface.dwWidth / CODEC_DECODE_JPEG_BLOCK_SIZE) - 1;
        jpegPicState.dwHeightInBlocks = (m_jpegBasicFeature->m_destSurface.dwHeight / CODEC_DECODE_JPEG_BLOCK_SIZE) - 1;
    }
}

MOS_STATUS JpegDecodePicPktXe_M_Base::AddMfxJpegPicCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_JPEG_DECODE_PIC_STATE jpegPicState;
    SetMfxJpegPicStateParams(jpegPicState);
    DECODE_CHK_STATUS(m_mfxInterface->AddMfxJpegPicCmd(&cmdBuffer, &jpegPicState));

    return MOS_STATUS_SUCCESS;
}

void JpegDecodePicPktXe_M_Base::SetMfxQmParams(MHW_VDBOX_QM_PARAMS &qmParams)
{
    DECODE_FUNC_CALL();

    MOS_ZeroMemory(&qmParams, sizeof(qmParams));
    qmParams.Standard = CODECHAL_JPEG;
    qmParams.pJpegQuantMatrix = m_jpegBasicFeature->m_jpegQMatrix;

    // Swapping QM(x,y) to QM(y,x) for 90/270 degree rotation
    if (m_jpegBasicFeature->m_jpegPicParams->m_rotation == jpegRotation90 ||
        m_jpegBasicFeature->m_jpegPicParams->m_rotation == jpegRotation270)
    {
        qmParams.bJpegQMRotation = true;
    }
    else
    {
        qmParams.bJpegQMRotation = false;
    }
}

MOS_STATUS JpegDecodePicPktXe_M_Base::AddMfxQmCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_QM_PARAMS qmParams;
    SetMfxQmParams(qmParams);

    if (m_jpegPicParams->m_numCompInFrame > jpegNumComponent)
    {
        DECODE_ASSERTMESSAGE("Unsupported Component Number in JPEG Picture parameter.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    for (uint16_t scanCount = 0; scanCount < m_jpegPicParams->m_numCompInFrame; scanCount++)
    {
        // Using scanCount here because the same command is used for JPEG decode and encode
        uint32_t quantTableSelector                                      = m_jpegPicParams->m_quantTableSelector[scanCount];
        if (quantTableSelector >= JPEG_MAX_NUM_OF_QUANTMATRIX)
        {
            DECODE_ASSERTMESSAGE("Unsupported QuantTableSelector in JPEG Picture parameter.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        qmParams.pJpegQuantMatrix->m_jpegQMTableType[quantTableSelector] = scanCount;
        qmParams.JpegQMTableSelector                                     = quantTableSelector;
        DECODE_CHK_STATUS(m_mfxInterface->AddMfxQmCmd(
            &cmdBuffer,
            &qmParams));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPktXe_M_Base::AddMfxJpegHuffTableCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    uint32_t dcCurHuffTblIndex[2] = {0xff, 0xff};
    uint32_t acCurHuffTblIndex[2] = {0xff, 0xff};

    if (m_jpegBasicFeature->m_jpegScanParams->NumScans > jpegNumComponent)
    {
        DECODE_ASSERTMESSAGE("Unsupported Component Number in JPEG Scan parameter.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    for (uint16_t scanCount = 0; scanCount < m_jpegBasicFeature->m_jpegScanParams->NumScans; scanCount++)
    {
        // MFX_JPEG_HUFF_TABLE
        uint16_t numComponents = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].NumComponents;
        if (numComponents > jpegNumComponent)
        {
            DECODE_ASSERTMESSAGE("Unsupported Component Number in JPEG Scan parameter.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        for (uint16_t scanComponent = 0; scanComponent < numComponents; scanComponent++)
        {
            // Determine which huffman table we will be writing to
            // For gray image, componentIdentifier[jpegComponentU] and componentIdentifier[jpegComponentV] are initialized to 0,
            // and when componentSelector[scanComponent] is equal 0, variable huffTableID is set to 1, and wrong Huffman table is used,
            // so it is more reasonable to use componentIdentifier[jpegComponentY] to determine which huffman table we will be writing to.
            uint8_t componentSelector =
                m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].ComponentSelector[scanComponent];
            uint16_t huffTableID = 0;
            if (componentSelector == m_jpegBasicFeature->m_jpegPicParams->m_componentIdentifier[jpegComponentY])
            {
                huffTableID = 0;
            }
            else
            {
                huffTableID = 1;
            }

            int32_t acTableSelector =
                m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].AcHuffTblSelector[scanComponent];
            int32_t dcTableSelector =
                m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].DcHuffTblSelector[scanComponent];

            // Send the huffman table state command only if the table changed
            if ((dcTableSelector != dcCurHuffTblIndex[huffTableID]) ||
                (acTableSelector != acCurHuffTblIndex[huffTableID]))
            {
                MHW_VDBOX_HUFF_TABLE_PARAMS huffmanTableParams;
                MOS_ZeroMemory(&huffmanTableParams, sizeof(huffmanTableParams));
                huffmanTableParams.HuffTableID = huffTableID;

                if (acTableSelector >= JPEG_MAX_NUM_HUFF_TABLE_INDEX || dcTableSelector >= JPEG_MAX_NUM_HUFF_TABLE_INDEX)
                {
                    MEDIA_ASSERTMESSAGE("acTableSelector and dcTableSelector cannot exceed 2 by spec.");
                    return MOS_STATUS_INVALID_PARAMETER;
                }

                huffmanTableParams.pACBits   = &m_jpegBasicFeature->m_jpegHuffmanTable->HuffTable[acTableSelector].AC_BITS[0];
                huffmanTableParams.pDCBits   = &m_jpegBasicFeature->m_jpegHuffmanTable->HuffTable[dcTableSelector].DC_BITS[0];
                huffmanTableParams.pACValues = &m_jpegBasicFeature->m_jpegHuffmanTable->HuffTable[acTableSelector].AC_HUFFVAL[0];
                huffmanTableParams.pDCValues = &m_jpegBasicFeature->m_jpegHuffmanTable->HuffTable[dcTableSelector].DC_HUFFVAL[0];

                DECODE_CHK_STATUS(m_mfxInterface->AddMfxJpegHuffTableCmd(
                    &cmdBuffer,
                    &huffmanTableParams));

                // Set the current huffman table indices for the next scan
                dcCurHuffTblIndex[huffTableID] = dcTableSelector;
                acCurHuffTblIndex[huffTableID] = acTableSelector;
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPktXe_M_Base::AddMfxBsdObjectParams(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();
    MHW_VDBOX_JPEG_BSD_PARAMS bsdParams;
    for (uint16_t scanCount = 0; scanCount < m_jpegBasicFeature->m_jpegScanParams->NumScans; scanCount++)
    {
        uint16_t numComponents = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].NumComponents;
        MOS_ZeroMemory(&bsdParams, sizeof(bsdParams));
        // MFX_JPEG_BSD_OBJECT
        bsdParams.dwIndirectDataLength         = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].DataLength;
        bsdParams.dwDataStartAddress           = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].DataOffset;
        bsdParams.dwScanHorizontalPosition     = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].ScanHoriPosition;
        bsdParams.dwScanVerticalPosition       = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].ScanVertPosition;
        bsdParams.bInterleaved                 = (numComponents > 1) ? 1 : 0;
        bsdParams.dwMCUCount                   = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].MCUCount;
        bsdParams.dwRestartInterval            = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].RestartInterval;

        uint16_t scanComponentIndex = 0;

        for (uint16_t scanComponent = 0; scanComponent < numComponents; scanComponent++)
        {
            uint8_t componentSelector =
                m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].ComponentSelector[scanComponent];

            if (componentSelector == m_jpegBasicFeature->m_jpegPicParams->m_componentIdentifier[jpegComponentY])
            {
                scanComponentIndex = 0;
            }
            else if (componentSelector == m_jpegBasicFeature->m_jpegPicParams->m_componentIdentifier[jpegComponentU])
            {
                scanComponentIndex = 1;
            }
            else if (componentSelector == m_jpegBasicFeature->m_jpegPicParams->m_componentIdentifier[jpegComponentV])
            {
                scanComponentIndex = 2;
            }
            // Add logic for component identifier JPEG_A

            bsdParams.sScanComponent |= (1 << scanComponentIndex);
        }

        DECODE_CHK_STATUS(m_mfxInterface->AddMfxJpegBsdObjCmd(
            &cmdBuffer,
            &bsdParams));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPktXe_M_Base::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    commandBufferSize = m_pictureStatesSize;
    requestedPatchListSize = m_picturePatchListSize;

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS JpegDecodePicPktXe_M_Base::DumpResources(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams)
{
    DECODE_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}
#endif

}