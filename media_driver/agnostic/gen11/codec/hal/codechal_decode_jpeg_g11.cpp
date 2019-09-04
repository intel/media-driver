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
//! \file     codechal_decode_jpeg_g11.cpp
//! \brief    Implements the decode interface extension for JPEG.
//! \details  Implements all functions and constants required by CodecHal for JPEG decoding.
//!

#include "codechal_decoder.h"
#include "codechal_decode_jpeg_g11.h"
#include "mhw_vdbox_mfx_g11_X.h"
#include "hal_oca_interface.h"

CodechalDecodeJpegG11::~CodechalDecodeJpegG11()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_veState != nullptr)
    {
        MOS_FreeMemAndSetNull(m_veState);
        m_veState = nullptr;
    }
}

MOS_STATUS CodechalDecodeJpegG11::SetGpuCtxCreatOption(
    CodechalSetting *          codecHalSetting)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    
    CODECHAL_DECODE_FUNCTION_ENTER;

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        CodechalDecode::SetGpuCtxCreatOption(codecHalSetting);
    }
    else
    {
        m_gpuCtxCreatOpt = MOS_New(MOS_GPUCTX_CREATOPTIONS_ENHANCED);

        bool sfcInUse = IsSfcInUse(codecHalSetting);

        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_ConstructParmsForGpuCtxCreation(    
            m_veState,
            (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt,
            sfcInUse));

        if (sfcInUse)
        {
            m_videoContext = MOS_GPU_CONTEXT_VIDEO4;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
                m_osInterface,
                m_videoContext,
                MOS_GPU_NODE_VIDEO,
                m_gpuCtxCreatOpt));

            MOS_GPUCTX_CREATOPTIONS createOption;
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
                m_osInterface,
                MOS_GPU_CONTEXT_VIDEO,
                MOS_GPU_NODE_VIDEO,
                &createOption));
        }
        else
        {
            m_videoContext = MOS_GPU_CONTEXT_VIDEO;
        }
    }
        
    return eStatus;
}

MOS_STATUS CodechalDecodeJpegG11::SetFrameStates()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeJpeg::SetFrameStates());

    if ( MOS_VE_SUPPORTED(m_osInterface))
    {
        if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
        {
            MOS_VIRTUALENGINE_SET_PARAMS  vesetParams;

            MOS_ZeroMemory(&vesetParams, sizeof(vesetParams));
#ifdef _DECODE_PROCESSING_SUPPORTED
            vesetParams.bSFCInUse = m_sfcState->m_sfcPipeOut;
#else
            vesetParams.bSFCInUse                   = false;
#endif
            vesetParams.bNeedSyncWithPrevious       = true;
            vesetParams.bSameEngineAsLastSubmission = false;
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_SetHintParams(
                m_veState,
                &vesetParams));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeJpegG11::DecodeStateLevel()
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
    pipeModeSelectParams.Mode = CODECHAL_DECODE_MODE_JPEG;
    pipeModeSelectParams.bStreamOutEnabled = m_streamOutEnabled;
    pipeModeSelectParams.bDeblockerStreamOutEnable = false;
    pipeModeSelectParams.bPostDeblockOutEnable = false;
    pipeModeSelectParams.bPreDeblockOutEnable = true;

    // Set CMD_MFX_SURFACE_STATE
    MHW_VDBOX_SURFACE_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.Mode = CODECHAL_DECODE_MODE_JPEG;
    surfaceParams.psSurface  = &m_destSurface;
    surfaceParams.ChromaType = m_jpegPicParams->m_chromaType;

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

    pipeBufAddrParams.pDecodedReconParam = &surfaceParams;
    pipeBufAddrParams.psRawSurface = nullptr;

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetPipeBufAddr(&pipeBufAddrParams, &cmdBuffer));
#endif

    pipeBufAddrParams.pDecodedReconParam = nullptr;

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

MOS_STATUS CodechalDecodeJpegG11::DecodePrimitiveLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    // if the bitstream is not complete, don't do any decoding work.
    if (m_incompletePicture)
    {
        return MOS_STATUS_SUCCESS;

    }

    CODECHAL_DECODE_CHK_NULL_RETURN(m_osInterface);
 
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
            // and when componentSelector[scanComponent] is equal 0, variable huffTableID is set to 1, and wrong Huffman table is used,
            // so it is more reasonable to use componentIdentifier[jpegComponentY] to determine which huffman table we will be writing to.
            uint8_t componentSelector =
                m_jpegScanParams->ScanHeader[scanCount].ComponentSelector[scanComponent];
            uint16_t huffTableID = 0;
            if (componentSelector == m_jpegPicParams->m_componentIdentifier[jpegComponentY])
            {
                huffTableID = 0;
            }
            else
            {
                huffTableID = 1;
            }

            int32_t acTableSelector =
                m_jpegScanParams->ScanHeader[scanCount].AcHuffTblSelector[scanComponent];
            int32_t dcTableSelector =
                m_jpegScanParams->ScanHeader[scanCount].DcHuffTblSelector[scanComponent];

            // Send the huffman table state command only if the table changed
            if ((dcTableSelector != dcCurHuffTblIndex[huffTableID]) ||
                (acTableSelector != acCurHuffTblIndex[huffTableID]))
            {
                MHW_VDBOX_HUFF_TABLE_PARAMS huffmanTableParams;
                MOS_ZeroMemory(&huffmanTableParams, sizeof(huffmanTableParams));

                huffmanTableParams.HuffTableID = huffTableID;

                huffmanTableParams.pACBits   = &m_jpegHuffmanTable->HuffTable[acTableSelector].AC_BITS[0];
                huffmanTableParams.pDCBits   = &m_jpegHuffmanTable->HuffTable[dcTableSelector].DC_BITS[0];
                huffmanTableParams.pACValues = &m_jpegHuffmanTable->HuffTable[acTableSelector].AC_HUFFVAL[0];
                huffmanTableParams.pDCValues = &m_jpegHuffmanTable->HuffTable[dcTableSelector].DC_HUFFVAL[0];

                CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxJpegHuffTableCmd(
                    &cmdBuffer,
                    &huffmanTableParams));

                // Set the current huffman table indices for the next scan
                dcCurHuffTblIndex[huffTableID] = dcTableSelector;
                acCurHuffTblIndex[huffTableID] = acTableSelector;
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
            uint8_t componentSelector =
                m_jpegScanParams->ScanHeader[scanCount].ComponentSelector[scanComponent];

            if (componentSelector == m_jpegPicParams->m_componentIdentifier[jpegComponentY])
            {
                scanComponentIndex = 0;
            }
            else if (componentSelector == m_jpegPicParams->m_componentIdentifier[jpegComponentU])
            {
                scanComponentIndex = 1;
            }
            else if (componentSelector == m_jpegPicParams->m_componentIdentifier[jpegComponentV])
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
        syncParams                  = g_cInitSyncParams;
        syncParams.GpuContext       = m_videoContextForWa;
        syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(
            m_osInterface,
            &syncParams));

        syncParams                  = g_cInitSyncParams;
        syncParams.GpuContext       = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(
            m_osInterface,
            &syncParams));
        }

    if ( MOS_VE_SUPPORTED(m_osInterface))
    {
        CodecHalDecodeSinglePipeVE_PopulateHintParams(m_veState, &cmdBuffer, true);
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

MOS_STATUS CodechalDecodeJpegG11::AllocateStandard(
    CodechalSetting *          settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(settings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeJpeg::AllocateStandard(settings));

    if ( MOS_VE_SUPPORTED(m_osInterface))
    {
        static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->DisableScalabilitySupport();

        //single pipe VE initialize
        m_veState = (PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE));
        CODECHAL_DECODE_CHK_NULL_RETURN(m_veState);
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_InitInterface(m_osInterface, m_veState));
    }

    return eStatus;
}

CodechalDecodeJpegG11::CodechalDecodeJpegG11(
    CodechalHwInterface   *hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) :
    CodechalDecodeJpeg(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);

    Mos_CheckVirtualEngineSupported(m_osInterface, true, true);
}

