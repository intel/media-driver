/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     decode_jpeg_picture_packet.cpp
//! \brief    Defines the interface for jpeg decode picture packet
//!
#include "decode_jpeg_picture_packet.h"
#include "codechal_debug.h"
#include "decode_common_feature_defs.h"

namespace decode{

JpegDecodePicPkt::~JpegDecodePicPkt()
{
    FreeResources();
}

MOS_STATUS JpegDecodePicPkt::FreeResources()
{
    DECODE_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPkt::Init()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_hwInterface);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_miItf);
    DECODE_CHK_NULL(m_jpegPipeline);
    DECODE_CHK_NULL(m_mfxItf);

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

MOS_STATUS JpegDecodePicPkt::Prepare()
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

MOS_STATUS JpegDecodePicPkt::AllocateFixedResources()
{
    DECODE_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    commandBufferSize = m_pictureStatesSize;
    requestedPatchListSize = m_picturePatchListSize;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_PIPE_MODE_SELECT, JpegDecodePicPkt)
{
    params.Mode                                           = m_jpegBasicFeature->m_mode;
    params.streamOutEnabled                               = false;
    params.deblockerStreamOutEnable                       = false;
    params.preDeblockingOutputEnablePredeblockoutenable   = true;
    params.postDeblockingOutputEnablePostdeblockoutenable = false;

    params.codecSelect = 0;
    if (CodecHalIsDecodeModeVLD(params.Mode))
    {
        params.decoderModeSelect = 0;
    }
    else if (CodecHalIsDecodeModeIT(params.Mode))
    {
        params.decoderModeSelect = 1;
    }
    params.standardSelect = CodecHal_GetStandardFromMode(params.Mode);
    params.decoderShortFormatMode = 1;
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_SURFACE_STATE, JpegDecodePicPkt)
{
    params.psSurface          = &m_jpegBasicFeature->m_destSurface;
    uint32_t uvPlaneAlignment = MHW_VDBOX_MFX_RECON_UV_PLANE_ALIGNMENT;

    params.tilemode = m_mfxItf->MosGetHWTileType(params.psSurface->TileType, params.psSurface->TileModeGMM, params.psSurface->bGMMTileEnabled);

    params.height       = params.psSurface->dwHeight - 1;
    params.width        = params.psSurface->dwWidth - 1;
    params.surfacePitch = params.psSurface->dwPitch - 1;

    if (params.surfaceId == CODECHAL_MFX_SRC_SURFACE_ID)
    {
        uvPlaneAlignment = MHW_VDBOX_MFX_RAW_UV_PLANE_ALIGNMENT_GEN9;
    }
    else if ((params.surfaceId == CODECHAL_MFX_REF_SURFACE_ID) || params.surfaceId == CODECHAL_MFX_DSRECON_SURFACE_ID)
    {
        uvPlaneAlignment = params.uvPlaneAlignment ? params.uvPlaneAlignment : MHW_VDBOX_MFX_RECON_UV_PLANE_ALIGNMENT;
    }
    else
    {
        uvPlaneAlignment = MHW_VDBOX_MFX_UV_PLANE_ALIGNMENT_LEGACY;
    }

    params.interleaveChroma = 0;
    params.surfaceFormat    = m_mfxItf->GetJpegDecodeFormat(params.psSurface->Format);

    if (params.psSurface->Format == Format_P8)
    {
        params.interleaveChroma = 0;
    }

    params.yOffsetForUCb = params.yOffsetForVCr =
        MOS_ALIGN_CEIL((params.psSurface->UPlaneOffset.iSurfaceOffset - params.psSurface->dwOffset) / params.psSurface->dwPitch + params.psSurface->RenderOffset.YUV.U.YOffset, uvPlaneAlignment);
    if (m_mfxItf->IsVPlanePresent(params.psSurface->Format))
    {
        params.yOffsetForVCr =
            MOS_ALIGN_CEIL((params.psSurface->VPlaneOffset.iSurfaceOffset - params.psSurface->dwOffset) / params.psSurface->dwPitch + params.psSurface->RenderOffset.YUV.V.YOffset, uvPlaneAlignment);
    }

#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(m_mmcState->SetSurfaceMmcState(&(m_jpegBasicFeature->m_destSurface)));
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(params.psSurface, &params.mmcState));
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcFormat(&m_jpegBasicFeature->m_destSurface, &params.compressionFormat));
#endif
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_PIPE_BUF_ADDR_STATE, JpegDecodePicPkt)
{
    params.decodeInUse         = true;
    params.Mode                = m_jpegBasicFeature->m_mode;
    params.psPreDeblockSurface = &m_jpegBasicFeature->m_destSurface;

    params.references = params.presReferences;
#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(params.psPreDeblockSurface, &params.PreDeblockSurfMmcState));
#endif
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_IND_OBJ_BASE_ADDR_STATE, JpegDecodePicPkt)
{
    params.Mode           = m_jpegBasicFeature->m_mode;
    params.dwDataSize     = m_jpegBasicFeature->m_dataSize;
    params.presDataBuffer = &(m_jpegBasicFeature->m_resDataBuffer.OsResource);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_JPEG_PIC_STATE, JpegDecodePicPkt)
{
    params.decodeInUse    = true;
    params.Mode           = m_jpegBasicFeature->m_mode;
    params.pJpegPicParams = m_jpegBasicFeature->m_jpegPicParams;
    params.dwOutputFormat = m_jpegBasicFeature->m_destSurface.Format;

    if (m_jpegBasicFeature->m_jpegPicParams->m_rotation == jpegRotation90 || m_jpegBasicFeature->m_jpegPicParams->m_rotation == jpegRotation270)
    {
        params.dwWidthInBlocks  = (m_jpegBasicFeature->m_destSurface.dwHeight / CODEC_DECODE_JPEG_BLOCK_SIZE) - 1;
        params.dwHeightInBlocks = (m_jpegBasicFeature->m_destSurface.dwWidth / CODEC_DECODE_JPEG_BLOCK_SIZE) - 1;
    }
    else
    {
        params.dwWidthInBlocks  = (m_jpegBasicFeature->m_destSurface.dwWidth / CODEC_DECODE_JPEG_BLOCK_SIZE) - 1;
        params.dwHeightInBlocks = (m_jpegBasicFeature->m_destSurface.dwHeight / CODEC_DECODE_JPEG_BLOCK_SIZE) - 1;
    }

    auto picParams = params.pJpegPicParams;

    if (picParams->m_chromaType == jpegRGB || picParams->m_chromaType == jpegBGR)
    {
        params.inputFormatYuv = jpegYUV444;
    }
    else
    {
        params.inputFormatYuv = picParams->m_chromaType;
    }
    params.rotation              = picParams->m_rotation;
    params.outputFormatYuv = m_mfxItf->GetJpegDecodeFormat((MOS_FORMAT)params.dwOutputFormat);

    if (params.dwOutputFormat == Format_NV12)
    {
        if (picParams->m_chromaType == jpegYUV422H2Y ||
            picParams->m_chromaType == jpegYUV422H4Y)
        {
            params.verticalDownSamplingEnable = 1;
        }
        else if (picParams->m_chromaType == jpegYUV422V2Y ||
                 picParams->m_chromaType == jpegYUV422V4Y)
        {
            params.horizontalDownSamplingEnable = 1;
        }
    }
    else if (params.dwOutputFormat == Format_UYVY ||
             params.dwOutputFormat == Format_YUY2)
    {
        if (picParams->m_chromaType == jpegYUV420)
        {
            params.verticalUpSamplingEnable = 1;
        }
    }
    params.frameWidthInBlocksMinus1  = params.dwWidthInBlocks;
    params.frameHeightInBlocksMinus1 = params.dwHeightInBlocks;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPkt::AddAllCmds_MFX_QM_STATE(PMOS_COMMAND_BUFFER cmdBuffer)
{
    auto pJpegQuantMatrix = m_jpegBasicFeature->m_jpegQMatrix;
    MHW_MI_CHK_NULL(pJpegQuantMatrix);

    auto &params         = m_mfxItf->MHW_GETPAR_F(MFX_QM_STATE)();
    params               = {};
    bool bJpegQMRotation = false;
    // Swapping QM(x,y) to QM(y,x) for 90/270 degree rotation
    if (m_jpegBasicFeature->m_jpegPicParams->m_rotation == jpegRotation90 ||
        m_jpegBasicFeature->m_jpegPicParams->m_rotation == jpegRotation270)
    {
        bJpegQMRotation = true;
    }
    else
    {
        bJpegQMRotation = false;
    }
    for (uint16_t scanCount = 0; scanCount < m_jpegPicParams->m_numCompInFrame; scanCount++)
    {
        // Using scanCount here because the same command is used for JPEG decode and encode
        uint32_t quantTableSelector                             = m_jpegPicParams->m_quantTableSelector[scanCount];
        pJpegQuantMatrix->m_jpegQMTableType[quantTableSelector] = scanCount;
        auto JpegQMTableSelector                                = quantTableSelector;

        uint8_t *qMatrix = (uint8_t *)params.quantizermatrix;

        params.qmType = pJpegQuantMatrix->m_jpegQMTableType[JpegQMTableSelector];

        if (bJpegQMRotation)
        {
            for (auto i = 0; i < 8; i++)
            {
                for (auto ii = 0; ii < 8; ii++)
                {
                    qMatrix[i + 8 * ii] = pJpegQuantMatrix->m_quantMatrix[JpegQMTableSelector][i * 8 + ii];
                }
            }
        }
        else
        {
            for (auto i = 0; i < 64; i++)
            {
                qMatrix[i] = pJpegQuantMatrix->m_quantMatrix[JpegQMTableSelector][i];
            }
        }

        m_mfxItf->MHW_ADDCMD_F(MFX_QM_STATE)(cmdBuffer);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPkt::AddAllCmds_MFX_JPEG_HUFF_TABLE_STATE(PMOS_COMMAND_BUFFER cmdBuffer)
{
    auto &params = m_mfxItf->MHW_GETPAR_F(MFX_JPEG_HUFF_TABLE_STATE)();
    params       = {};

    uint32_t dcCurHuffTblIndex[2] = {0xff, 0xff};
    uint32_t acCurHuffTblIndex[2] = {0xff, 0xff};

    for (uint16_t scanCount = 0; scanCount < m_jpegBasicFeature->m_jpegScanParams->NumScans; scanCount++)
    {
        // MFX_JPEG_HUFF_TABLE
        uint16_t numComponents = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].NumComponents;
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
                //MHW_VDBOX_HUFF_TABLE_PARAMS huffmanTableParams;
                //MOS_ZeroMemory(&huffmanTableParams, sizeof(huffmanTableParams));
                params.huffTableID = huffTableID;

                params.pACBits   = &m_jpegBasicFeature->m_jpegHuffmanTable->HuffTable[acTableSelector].AC_BITS[0];
                params.pDCBits   = &m_jpegBasicFeature->m_jpegHuffmanTable->HuffTable[dcTableSelector].DC_BITS[0];
                params.pACValues = &m_jpegBasicFeature->m_jpegHuffmanTable->HuffTable[acTableSelector].AC_HUFFVAL[0];
                params.pDCValues = &m_jpegBasicFeature->m_jpegHuffmanTable->HuffTable[dcTableSelector].DC_HUFFVAL[0];

                m_mfxItf->MHW_ADDCMD_F(MFX_JPEG_HUFF_TABLE_STATE)(cmdBuffer);

                // Set the current huffman table indices for the next scan
                dcCurHuffTblIndex[huffTableID] = dcTableSelector;
                acCurHuffTblIndex[huffTableID] = acTableSelector;
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPkt::AddAllCmds_MFD_JPEG_BSD_OBJECT(PMOS_COMMAND_BUFFER cmdBuffer)
{
    auto &params = m_mfxItf->MHW_GETPAR_F(MFD_JPEG_BSD_OBJECT)();
    params       = {};
    for (uint16_t scanCount = 0; scanCount < m_jpegBasicFeature->m_jpegScanParams->NumScans; scanCount++)
    {
        uint16_t numComponents = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].NumComponents;

        params.indirectDataLength     = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].DataLength;
        params.dataStartAddress       = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].DataOffset;
        params.scanHorizontalPosition = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].ScanHoriPosition;
        params.scanVerticalPosition   = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].ScanVertPosition;
        params.interleaved             = (numComponents > 1) ? 1 : 0;
        params.mcuCount               = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].MCUCount;
        params.restartInterval        = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[scanCount].RestartInterval;

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

            params.scanComponent |= (1 << scanComponentIndex);
        }

        m_mfxItf->MHW_ADDCMD_F(MFD_JPEG_BSD_OBJECT)(cmdBuffer);
    }
    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS JpegDecodePicPkt::DumpResources(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams)
{
    DECODE_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

#endif

}
