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
//! \file     encode_vp9_dynamic_scal_packet_xe_lpm_plus_base.cpp
//! \brief    Implementation of vp9 dynamic scaling (reference frame scaling) packet
//!

#include "encode_vp9_dynamic_scal_packet_xe_lpm_plus_base.h"
#include "mos_solo_generic.h"
#include "encode_status_report.h"
#include "encode_vp9_pak.h"
#include "encode_vp9_hpu.h"
#include "encode_vp9_brc.h"
#include "encode_vp9_cqp.h"
#include "encode_vp9_tile.h"
#include "encode_vp9_segmentation.h"
#include "encode_vp9_hpu.h"

namespace encode
{

//// loop filter value based on qp index look up table
//extern const uint8_t LF_VALUE_QP_LOOKUP[CODEC_VP9_QINDEX_RANGE] = {
//    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03,
//    0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07,
//    0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
//    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x0a, 0x0a, 0x0a, 0x0a,
//    0x0a, 0x0b, 0x0b, 0x0b, 0x0b, 0x0c, 0x0c, 0x0c, 0x0c, 0x0d, 0x0d, 0x0d, 0x0d, 0x0e, 0x0e, 0x0e,
//    0x0e, 0x0f, 0x0f, 0x0f, 0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x12, 0x12, 0x12, 0x13, 0x13,
//    0x13, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x16, 0x16, 0x17, 0x17, 0x17, 0x18, 0x18, 0x18, 0x19,
//    0x19, 0x19, 0x1a, 0x1a, 0x1b, 0x1b, 0x1b, 0x1c, 0x1c, 0x1d, 0x1d, 0x1d, 0x1e, 0x1e, 0x1e, 0x1f,
//    0x1f, 0x20, 0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x22, 0x23, 0x23, 0x24, 0x24, 0x25, 0x25, 0x25,
//    0x26, 0x26, 0x27, 0x27, 0x27, 0x28, 0x28, 0x29, 0x29, 0x29, 0x2a, 0x2a, 0x2b, 0x2b, 0x2b, 0x2c,
//    0x2c, 0x2d, 0x2d, 0x2d, 0x2e, 0x2e, 0x2f, 0x2f, 0x2f, 0x30, 0x30, 0x31, 0x31, 0x31, 0x32, 0x32,
//    0x32, 0x33, 0x33, 0x34, 0x34, 0x34, 0x35, 0x35, 0x35, 0x36, 0x36, 0x36, 0x37, 0x37, 0x37, 0x38,
//    0x38, 0x39, 0x39, 0x39, 0x3a, 0x3a, 0x3a, 0x3a, 0x3b, 0x3b, 0x3b, 0x3c, 0x3c, 0x3c, 0x3d, 0x3d,
//    0x3d, 0x3e, 0x3e, 0x3e, 0x3e, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
//    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
//    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
//};

MOS_STATUS Vp9DynamicScalPktXe_Lpm_Plus_Base::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_vp9PicParams);

    // For VDENC dynamic scaling, here are the steps we need to process
    // 1. Use PAK to down scale the reference picture (PASS 0)
    // 2. Run VDENC to stream out PakObjCmd (PASS 0)
    // 3. Run VDENC (with PAK only multi pass enabled) to stream in PakObjCmd from previous pass (PASS 0)
    // 4. Repak (PASS 1) it is only for CQP mode
    // 5. Extra note: Repak is disabled for BRC Dynamic scaling single pass mode

    auto dysRefFrameFlags = m_basicFeature->m_ref.DysRefFrameFlags();
    if (dysRefFrameFlags == DYS_REF_NONE)
    {
        return MOS_STATUS_SUCCESS;
    }

    ENCODE_CHK_STATUS_RETURN(Mos_Solo_PreProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));

    // Ensure the input is ready to be read.
    // Currently, mos RegisterResource has sync limitation for Raw resource.
    // Temporaly, call Resource wait to do the sync explicitly.
    // TODO, refine it when MOS refactor ready
    MOS_SYNC_PARAMS syncParams;
    syncParams                  = g_cInitSyncParams;
    syncParams.GpuContext       = m_osInterface->pfnGetGpuContext(m_osInterface);
    syncParams.presSyncResource = &m_basicFeature->m_rawSurface.OsResource;
    syncParams.bReadOnly        = true;
    ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));
    m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

    MOS_COMMAND_BUFFER &cmdBuffer = *commandBuffer;

    auto segmentParams = static_cast<PCODEC_VP9_ENCODE_SEGMENT_PARAMS>(m_basicFeature->m_vp9SegmentParams);
    ENCODE_CHK_NULL_RETURN(segmentParams);

    // Turn off scalability and tiling for dynamic scaling pass 0 for reference scaling
    uint8_t logTileRows    = m_basicFeature->m_vp9PicParams->log2_tile_rows;
    uint8_t logTileColumns = m_basicFeature->m_vp9PicParams->log2_tile_columns;
    bool    scalableMode   = m_basicFeature->m_scalableMode;

    m_basicFeature->m_vp9PicParams->log2_tile_rows    = 0;
    m_basicFeature->m_vp9PicParams->log2_tile_columns = 0;
    m_basicFeature->m_scalableMode                    = false;
    
    // Save current state
    // We only need to run pak to get the recon picture, so disable HuC and VDenc here
    auto        dysHucEnabled    = m_basicFeature->m_hucEnabled;
    m_basicFeature->m_hucEnabled = false;
    MOS_SURFACE origReconSurface = m_basicFeature->m_reconSurface;
    // Set the downscaled surface as the recon output surface
    m_basicFeature->m_reconSurface = m_basicFeature->m_ref.GetCurrDysRefList()->sDysSurface;
    // Save the ucNumPasses and set the ucNumPasses = ucCurrPass + 1.
    // Otherwise SliceLevel will mistakenly treat current pass as last pass.
    auto scalability = m_pipeline->GetMediaScalability();
    ENCODE_CHK_NULL_RETURN(scalability);
    auto origNumPasses = scalability->GetPassNumber();
    scalability->SetPassNumber(scalability->GetCurrentPass() + 1);

    bool origSegmentSkip[CODEC_VP9_MAX_SEGMENTS] = {false};
    for (auto i = 0; i < CODEC_VP9_MAX_SEGMENTS; ++i)
    {
        origSegmentSkip[i] = segmentParams->SegData[i].SegmentFlags.fields.SegmentSkipped;
        segmentParams->SegData[i].SegmentFlags.fields.SegmentSkipped = true;
    }

    ENCODE_CHK_STATUS_RETURN(PatchPictureLevelCommands(cmdBuffer, packetPhase));
    ENCODE_CHK_STATUS_RETURN(PatchSliceLevelCommands(cmdBuffer, packetPhase));

    // Restore saved state
    scalability->SetPassNumber(origNumPasses);
    m_basicFeature->m_reconSurface = origReconSurface;
    m_basicFeature->m_hucEnabled   = (dysHucEnabled && !m_basicFeature->m_dysVdencMultiPassEnabled);

    for (auto i = 0; i < CODEC_VP9_MAX_SEGMENTS; ++i)
    {
        segmentParams->SegData[i].SegmentFlags.fields.SegmentSkipped = origSegmentSkip[i];
    }

    // Restore scalability and tiling status for subsequent passes
    m_basicFeature->m_vp9PicParams->log2_tile_rows = logTileRows;
    m_basicFeature->m_vp9PicParams->log2_tile_columns = logTileColumns;
    m_basicFeature->m_scalableMode = scalableMode;

    ENCODE_CHK_STATUS_RETURN(Mos_Solo_PostProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));

    CODECHAL_DEBUG_TOOL(
        ENCODE_CHK_STATUS_RETURN(DumpInput());
    )

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DynamicScalPktXe_Lpm_Plus_Base::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
{
    ENCODE_FUNC_CALL();

    auto dysRefFrameFlags = m_basicFeature->m_ref.DysRefFrameFlags();
    if (dysRefFrameFlags == DYS_REF_NONE)
    {
        return MOS_STATUS_SUCCESS;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DynamicScalPktXe_Lpm_Plus_Base::DumpOutput()
{
    ENCODE_FUNC_CALL();

#if USE_CODECHAL_DEBUG_TOOL
    // Dump output (resource, information, etc) after command submitted
    CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    CODEC_REF_LIST currRefList = *((CODEC_REF_LIST *)m_basicFeature->m_ref.GetCurrRefList());

    std::stringstream pipeIdxStrStream;
    pipeIdxStrStream << "_" << (int)m_pipeline->GetCurrentPipe();

    std::string bufferPassName = GetPacketName();
    bufferPassName += pipeIdxStrStream.str() + "_output";

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
        &currRefList.resBitstreamBuffer,
        CodechalDbgAttr::attrBitstream,
        bufferPassName.data(),
        m_basicFeature->m_bitstreamSize,
        0,
        CODECHAL_NUM_MEDIA_STATES));

    MOS_RESOURCE *mbCodedBuffer = m_basicFeature->m_trackedBuf->GetBuffer(
        BufferType::mbCodedBuffer, currRefList.ucScalingIdx);
    if (mbCodedBuffer)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            mbCodedBuffer,
            CodechalDbgAttr::attrVdencOutput,
            (bufferPassName + "_MbCode").data(),
            m_basicFeature->m_mbCodeSize + 8 * CODECHAL_CACHELINE_SIZE,
            0,
            CODECHAL_NUM_MEDIA_STATES));
    }

    MOS_SURFACE dysSurface = m_basicFeature->m_ref.GetCurrDysRefList()->sDysSurface;
    uint8_t     dysRefIdx  = m_basicFeature->m_ref.GetDysRefIndex();

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
        &dysSurface,
        CodechalDbgAttr::attrReferenceSurfaces,
        (dysRefIdx == 1) ? "DysLastScaledSurf" : (dysRefIdx == 2) ? "DysGoldenScaledSurf" : "DysAltScaledSurf"));

#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DynamicScalPktXe_Lpm_Plus_Base::AddVdControlInitialize(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_miItf);

    auto &vdControlStateParams          = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
    vdControlStateParams                = {};
    vdControlStateParams.initialization = true;

    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DynamicScalPktXe_Lpm_Plus_Base::AddHcpPipeModeSelectCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    SETPAR_AND_ADDCMD(MFX_WAIT, m_miItf, &cmdBuffer);
    SETPAR_AND_ADDCMD(HCP_PIPE_MODE_SELECT, m_hcpInterfaceNew, &cmdBuffer);
    SETPAR_AND_ADDCMD(MFX_WAIT, m_miItf, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DynamicScalPktXe_Lpm_Plus_Base::SetHcpSurfacesParams(MHW_VDBOX_SURFACE_PARAMS *surfacesParams)
{
    ENCODE_FUNC_CALL();

    for (uint8_t i = 0; i <= CODECHAL_HCP_ALTREF_SURFACE_ID; ++i)
    {
        MOS_ZeroMemory(&surfacesParams[i], sizeof(surfacesParams[i]));
        surfacesParams[i].Mode             = m_basicFeature->m_mode;
        surfacesParams[i].ucSurfaceStateId = i;
        surfacesParams[i].ChromaType       = m_basicFeature->m_outputChromaFormat;

        switch (m_vp9SeqParams->SeqFlags.fields.EncodedBitDepth)
        {
        case VP9_ENCODED_BIT_DEPTH_10:
            surfacesParams[i].ucBitDepthChromaMinus8 = 2;
            surfacesParams[i].ucBitDepthLumaMinus8   = 2;
            break;
        default:
            surfacesParams[i].ucBitDepthChromaMinus8 = 0;
            surfacesParams[i].ucBitDepthLumaMinus8   = 0;
            break;
        }
    }

    ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_ref.SetDysHcpSurfaceParams(surfacesParams));

    // Program surface params for reconstructed surface
    surfacesParams[CODECHAL_HCP_DECODED_SURFACE_ID].psSurface         = &m_basicFeature->m_reconSurface;
    surfacesParams[CODECHAL_HCP_DECODED_SURFACE_ID].dwReconSurfHeight = m_basicFeature->m_rawSurfaceToPak->dwHeight;

    // Program surface params for source surface
    surfacesParams[CODECHAL_HCP_SRC_SURFACE_ID].psSurface             = m_basicFeature->m_rawSurfaceToPak;
    surfacesParams[CODECHAL_HCP_SRC_SURFACE_ID].bDisplayFormatSwizzle = m_vp9SeqParams->SeqFlags.fields.DisplayFormatSwizzle;
    surfacesParams[CODECHAL_HCP_SRC_SURFACE_ID].dwActualWidth         = MOS_ALIGN_CEIL(m_basicFeature->m_oriFrameWidth, CODEC_VP9_MIN_BLOCK_WIDTH);
    surfacesParams[CODECHAL_HCP_SRC_SURFACE_ID].dwActualHeight        = MOS_ALIGN_CEIL(m_basicFeature->m_oriFrameHeight, CODEC_VP9_MIN_BLOCK_HEIGHT);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DynamicScalPktXe_Lpm_Plus_Base::AddHcpPipeBufAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeHpu, Vp9FeatureIDs::vp9HpuFeature, SetIsLastPass, m_pipeline->IsLastPass());
    ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_ref.SetDysValue(true));
    SETPAR_AND_ADDCMD(HCP_PIPE_BUF_ADDR_STATE, m_hcpInterfaceNew, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DynamicScalPktXe_Lpm_Plus_Base::PatchPictureLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();

    SetPerfTag(CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE, (uint16_t)m_basicFeature->m_mode, m_basicFeature->m_pictureCodingType);

    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodePak, Vp9FeatureIDs::vp9PakFeature, ConstructPakInsertObjBatchBuffer);
    HucBrcBuffers *hucBrcBuffers = nullptr;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeBrc, Vp9FeatureIDs::vp9BrcFeature, GetHucBrcBuffers, hucBrcBuffers);
    ENCODE_CHK_NULL_RETURN(hucBrcBuffers);
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodePak, Vp9FeatureIDs::vp9PakFeature, PakConstructPicStateBatchBuffer, &hucBrcBuffers->resPicStateBrcWriteHucReadBuffer);

    bool firstTaskInPhase = ((packetPhase & firstPacket) == firstPacket);

    if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
    {
        ENCODE_CHK_STATUS_RETURN(AddForceWakeup(cmdBuffer));
        // Send command buffer at the beginning (OS dependent)
        ENCODE_CHK_STATUS_RETURN(SendPrologCmds(cmdBuffer));
    }

    // Current pass and number of passes
    auto currPass  = m_pipeline->GetCurrentPass();
    auto numPasses = m_pipeline->GetPassNum();

    // Making sure ImgStatusCtrl is zeroed out before first PAK pass
    // HW supposedly does this before start each frame. Remove this after confirming
    auto mmioRegisters = m_hcpInterfaceNew->GetMmioRegisters(m_vdboxIndex);
    if (currPass == 0)
    {
        auto &miLoadRegImmParams      = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwData     = 0;
        miLoadRegImmParams.dwRegister = mmioRegisters->hcpVp9EncImageStatusCtrlRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(&cmdBuffer));
    }

    // Read image status before running PAK, to get correct cumulative delta applied for final pass.
    if (currPass != numPasses)
    {
        ENCODE_CHK_STATUS_RETURN(ReadImageStatus(cmdBuffer));
    }

    // updating the number of pak passes in encode status buffer. should not update for repak
    if (currPass < numPasses)
    {
        PMOS_RESOURCE osResource = nullptr;
        uint32_t      offset     = 0;
        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportNumberPasses, osResource, offset));
        ENCODE_CHK_NULL_RETURN(osResource);

        auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        storeDataParams                  = {};
        storeDataParams.pOsResource      = osResource;
        storeDataParams.dwResourceOffset = offset;
        storeDataParams.dwValue          = currPass + 1;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(&cmdBuffer));
    }

    if (!currPass && m_osInterface->bTagResourceSync)
    {
        // This is a short term WA to solve the sync tag issue: the sync tag write for PAK is inserted at the end of 2nd pass PAK BB
        // which may be skipped in multi-pass PAK enabled case. The idea here is to insert the previous frame's tag at the beginning
        // of the BB and keep the current frame's tag at the end of the BB. There will be a delay for tag update but it should be fine
        // as long as Dec/VP/Enc won't depend on this PAK so soon.
        PMOS_RESOURCE globalGpuContextSyncTagBuffer = nullptr;
        ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetGpuStatusBufferResource(
            m_osInterface,
            globalGpuContextSyncTagBuffer));
        ENCODE_CHK_NULL_RETURN(globalGpuContextSyncTagBuffer);

        uint32_t value                   = m_osInterface->pfnGetGpuStatusTag(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        auto &   storeDataParams         = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        storeDataParams                  = {};
        storeDataParams.pOsResource      = globalGpuContextSyncTagBuffer;
        storeDataParams.dwResourceOffset = m_osInterface->pfnGetGpuStatusTagOffset(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        storeDataParams.dwValue          = (value > 0) ? (value - 1) : 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(&cmdBuffer));
    }

    ENCODE_CHK_STATUS_RETURN(StartStatusReport(statusReportMfx, &cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(AddVdControlInitialize(cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(AddHcpPipeModeSelectCmd(cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_SURFACE_STATE(&cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(AddHcpPipeBufAddrCmd(cmdBuffer));

    SETPAR_AND_ADDCMD(HCP_IND_OBJ_BASE_ADDR_STATE, m_hcpInterfaceNew, &cmdBuffer);

    // Using picstate zero with updated QP and LF deltas by HuC for repak, irrespective of how many Pak passes were run in multi-pass mode.
    MHW_BATCH_BUFFER secondLevelBatchBuffer;
    MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(secondLevelBatchBuffer));
    secondLevelBatchBuffer.dwOffset     = (numPasses > 0) ? CODECHAL_ENCODE_VP9_PIC_STATE_BUFFER_SIZE_PER_PASS * (currPass % numPasses) : 0;
    secondLevelBatchBuffer.bSecondLevel = true;
    //As Huc is disabled for Ref frame scaling, use the ReadBuffer
    secondLevelBatchBuffer.OsResource = hucBrcBuffers->resPicStateBrcWriteHucReadBuffer;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(&cmdBuffer, &secondLevelBatchBuffer));

    // HCP_VP9_SEGMENT_STATE
    uint8_t segmentCount = (m_basicFeature->m_vp9PicParams->PicFlags.fields.segmentation_enabled) ? CODEC_VP9_MAX_SEGMENTS : 1;

    for (uint8_t i = 0; i < segmentCount; i++)
    {
        RUN_FEATURE_INTERFACE_RETURN(Vp9Segmentation, Vp9FeatureIDs::vp9Segmentation, SetSegmentId, i);
        SETPAR_AND_ADDCMD(HCP_VP9_SEGMENT_STATE, m_hcpInterfaceNew, &cmdBuffer);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DynamicScalPktXe_Lpm_Plus_Base::AddHcpTileCodingCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, SetHcpTileCodingParams, m_pipeline->GetPipeNum());

    SETPAR_AND_ADDCMD(HCP_TILE_CODING, m_hcpInterfaceNew, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DynamicScalPktXe_Lpm_Plus_Base::AddOneTileCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t tileRow, uint32_t tileCol, uint32_t tileRowPass)
{
    ENCODE_FUNC_CALL();

    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, SetCurrentTile, tileRow, tileCol, m_pipeline);

    // Begin patching tile level batch commands
    MOS_COMMAND_BUFFER constructTileBatchBuf = {};
    RUN_FEATURE_INTERFACE_RETURN(
        Vp9EncodeTile, Vp9FeatureIDs::encodeTile, BeginPatchTileLevelBatch, tileRowPass, constructTileBatchBuf);

    // Add batch buffer start for tile
    PMHW_BATCH_BUFFER tileLevelBatchBuffer = nullptr;
    RUN_FEATURE_INTERFACE_RETURN(
        Vp9EncodeTile, Vp9FeatureIDs::encodeTile, GetTileLevelBatchBuffer, tileLevelBatchBuffer);
    ENCODE_CHK_NULL_RETURN(tileLevelBatchBuffer);

    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(&cmdBuffer, tileLevelBatchBuffer));

    // VP9 tile commands

    // Add hcp tile coding command
    ENCODE_CHK_STATUS_RETURN(AddHcpTileCodingCmd(constructTileBatchBuf));

    // For 2nd level BB, we must use tileLevelBatchBuffer to prevent adding Epilogue before MI_BATCH_BUFFER_END
    tileLevelBatchBuffer->iCurrent   = constructTileBatchBuf.iOffset;
    tileLevelBatchBuffer->iRemaining = constructTileBatchBuf.iRemaining;
    ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, tileLevelBatchBuffer));

    CODECHAL_DEBUG_TOOL(
        CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        std::string tileLevelBatchName = "_TLB_Dys_Pass";
        tileLevelBatchName += std::to_string((uint32_t)m_pipeline->GetCurrentPass());
        tileLevelBatchName += ("_" + std::to_string((uint32_t)m_pipeline->GetCurrentPipe()));
        tileLevelBatchName += ("_r" + std::to_string(tileRow) + "_c" + std::to_string(tileCol));

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpCmdBuffer(
            &constructTileBatchBuf,
            CODECHAL_NUM_MEDIA_STATES,
            tileLevelBatchName.c_str()));)

    // End patching tile level batch commands
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, EndPatchTileLevelBatch);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DynamicScalPktXe_Lpm_Plus_Base::AddOneTileCommandsNoTLBB(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t tileRow, uint32_t tileCol, uint32_t tileRowPass)
{
    ENCODE_FUNC_CALL();

    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, SetCurrentTile, tileRow, tileCol, m_pipeline);

    // Add hcp tile coding command
    ENCODE_CHK_STATUS_RETURN(AddHcpTileCodingCmd(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DynamicScalPktXe_Lpm_Plus_Base::PatchSliceLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();

    MHW_BATCH_BUFFER secondLevelBatchBuffer;
    MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(secondLevelBatchBuffer));
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodePak, Vp9FeatureIDs::vp9PakFeature, SetHucPakInsertObjBatchBuffer, secondLevelBatchBuffer);
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(&cmdBuffer, &secondLevelBatchBuffer));

    // Setup tile level pak commands
    uint16_t numTileColumns = 1;
    uint16_t numTileRows    = 1;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

    for (uint32_t tileRow = 0; tileRow < numTileRows; ++tileRow)
    {
        for (uint32_t tileCol = 0; tileCol < numTileColumns; ++tileCol)
        {
            ENCODE_CHK_STATUS_RETURN(AddOneTileCommands(cmdBuffer, tileRow, tileCol));
        }
    }

    MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(MHW_BATCH_BUFFER));
    secondLevelBatchBuffer.OsResource   = *m_basicFeature->m_resMbCodeBuffer;
    secondLevelBatchBuffer.dwOffset     = 0;
    secondLevelBatchBuffer.bSecondLevel = true;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(&cmdBuffer, &secondLevelBatchBuffer));

    // Send VD_PIPELINE_FLUSH command
    // MFXPipeDone should not be set for tail insertion (?)
    auto &vdPipelineFlushParams                  = m_vdencInterfaceNew->MHW_GETPAR_F(VD_PIPELINE_FLUSH)();
    vdPipelineFlushParams                        = {};
    vdPipelineFlushParams.waitDoneMFX            = (m_basicFeature->m_lastPicInStream || m_basicFeature->m_lastPicInSeq) ? false : true;
    vdPipelineFlushParams.waitDoneHEVC           = true;
    vdPipelineFlushParams.flushHEVC              = true;
    vdPipelineFlushParams.waitDoneVDCmdMsgParser = true;
    ENCODE_CHK_STATUS_RETURN(m_vdencInterfaceNew->MHW_ADDCMD_F(VD_PIPELINE_FLUSH)(&cmdBuffer));

    // Flush the engine to ensure memory written out
    auto &flushDwParams = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams       = {};
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(EndStatusReport(statusReportMfx, &cmdBuffer));

    if (!m_basicFeature->m_scalableMode)
    {
        ENCODE_CHK_STATUS_RETURN(ReadHcpStatus(m_vdboxIndex, m_statusReport, cmdBuffer));
    }

    ENCODE_CHK_STATUS_RETURN(UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DynamicScalPktXe_Lpm_Plus_Base::ReadImageStatus(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    auto mmioRegisters = m_hwInterface->GetVdencInterfaceNext()->GetMmioRegisters(m_vdboxIndex);
    ENCODE_CHK_NULL_RETURN(mmioRegisters);

    PMOS_RESOURCE osResource = nullptr;
    uint32_t      offset     = 0;
    ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportImageStatusMask, osResource, offset));
    ENCODE_CHK_NULL_RETURN(osResource);

    auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
    miStoreRegMemParams                 = {};
    miStoreRegMemParams.presStoreBuffer = osResource;
    miStoreRegMemParams.dwOffset        = offset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcImageStatusMaskRegOffset;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportImageStatusCtrl, osResource, offset));
    ENCODE_CHK_NULL_RETURN(osResource);

    miStoreRegMemParams.presStoreBuffer = osResource;
    miStoreRegMemParams.dwOffset        = offset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcImageStatusCtrlRegOffset;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));

    auto brcFeature = dynamic_cast<Vp9EncodeBrc *>(m_featureManager->GetFeature(Vp9FeatureIDs::vp9BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    // VDEnc dynamic slice overflow semaphore, 
    // DW0 is SW programmed mask (MFX_IMAGE_MASK does not support),
    // DW1 is MFX_IMAGE_STATUS_CONTROL
    if (brcFeature->IsVdencBrcEnabled())
    {
        MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
        
        auto mfxInterfaceNew = std::static_pointer_cast<mhw::vdbox::mfx::Itf>(m_hwInterface->GetMfxInterfaceNext());
        ENCODE_CHK_NULL_RETURN(mfxInterfaceNew);

        // Added for VDEnc slice overflow bit in MFC_IMAGE_STATUS_CONTROL.
        // The bit is connected on the non-AVC encoder side of MMIO register.
        // Need a dummy MFX_PIPE_MODE_SELECT to decoder and read this register
        if (m_waReadVDEncOverflowStatus)
        {
            SETPAR_AND_ADDCMD(MFX_WAIT, m_miItf, &cmdBuffer);
            m_basicFeature->SetDecodeInUse(true);
            SETPAR_AND_ADDCMD(MFX_PIPE_MODE_SELECT, mfxInterfaceNew, &cmdBuffer);
            SETPAR_AND_ADDCMD(MFX_WAIT, m_miItf, &cmdBuffer);
        }

        // Store MFC_IMAGE_STATUS_CONTROL MMIO to DMEM for HuC next BRC pass of current frame and first pass of next frame
        for (int i = 0; i < 2; ++i)
        {
            if (m_resVdencBrcUpdateDmemBufferPtr[i])
            {
                miStoreRegMemParams.presStoreBuffer = m_resVdencBrcUpdateDmemBufferPtr[i];
                miStoreRegMemParams.dwOffset        = 7 * sizeof(uint32_t); // offset of SliceSizeViolation in HUC_BRC_UPDATE_DMEM
                miStoreRegMemParams.dwRegister      = mmioRegisters->mfcImageStatusCtrlRegOffset;
                ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));
            }
        }

        // Restore MFX_PIPE_MODE_SELECT to encode mode
        if (m_waReadVDEncOverflowStatus)
        {
            SETPAR_AND_ADDCMD(MFX_WAIT, m_miItf, &cmdBuffer);
            m_basicFeature->SetDecodeInUse(false);
            SETPAR_AND_ADDCMD(MFX_PIPE_MODE_SELECT, mfxInterfaceNew, &cmdBuffer);
            SETPAR_AND_ADDCMD(MFX_WAIT, m_miItf, &cmdBuffer);
        }
    }

    auto &flushDwParams = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams       = {};
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DynamicScalPktXe_Lpm_Plus_Base::DumpInput()
{
    ENCODE_FUNC_CALL();

#if USE_CODECHAL_DEBUG_TOOL

    // Dump resource, information, etc. before command submitted
    CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    CODEC_REF_LIST currRefList = *((CODEC_REF_LIST *)m_basicFeature->m_ref.GetCurrRefList());

    std::stringstream pipeIdxStrStream;
    pipeIdxStrStream << "_" << (int)m_pipeline->GetCurrentPipe();

    std::string bufferPassName = GetPacketName();
    bufferPassName += pipeIdxStrStream.str() + "_input";

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
        &currRefList.resBitstreamBuffer,
        CodechalDbgAttr::attrBitstream,
        bufferPassName.data(),
        m_basicFeature->m_bitstreamSize,
        0,
        CODECHAL_NUM_MEDIA_STATES));

    MOS_RESOURCE *mbCodedBuffer = m_basicFeature->m_trackedBuf->GetBuffer(
        BufferType::mbCodedBuffer, currRefList.ucScalingIdx);
    if (mbCodedBuffer != nullptr)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            mbCodedBuffer,
            CodechalDbgAttr::attrVdencOutput,
            (bufferPassName + "_MbCode").data(),
            m_basicFeature->m_mbCodeSize + 8 * CODECHAL_CACHELINE_SIZE,
            0,
            CODECHAL_NUM_MEDIA_STATES));
    }

#endif

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, Vp9DynamicScalPktXe_Lpm_Plus_Base)
{
    ENCODE_FUNC_CALL();

    params.codecStandardSelect = CODEC_STANDARD_SELECT_VP9;
    params.codecSelect         = CODEC_SELECT_ENCODE;

    auto brcFeature = dynamic_cast<Vp9EncodeBrc *>(m_featureManager->GetFeature(Vp9FeatureIDs::vp9BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    params.bStreamOutEnabled      = brcFeature->IsVdencBrcEnabled();

    auto dysRefFrameFlags         = m_basicFeature->m_ref.DysRefFrameFlags();
    auto dysVdencMultiPassEnabled = m_basicFeature->m_dysVdencMultiPassEnabled;

    params.bStreamOutEnabled      = false;
    params.bVdencEnabled          = false;
    params.bDynamicScalingEnabled = (dysRefFrameFlags != DYS_REF_NONE) && !dysVdencMultiPassEnabled;

    params.multiEngineMode        = getMultiEngineMode();
    params.pipeWorkMode           = getPipeWorkMode();

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_IND_OBJ_BASE_ADDR_STATE, Vp9DynamicScalPktXe_Lpm_Plus_Base)
{
    ENCODE_FUNC_CALL();

    params.presPakBaseObjectBuffer = &m_basicFeature->m_resBitstreamBuffer;
    params.dwPakBaseObjectSize     = m_basicFeature->m_bitstreamUpperBound;
    params.presMvObjectBuffer      = m_basicFeature->m_resMbCodeBuffer;
    params.dwMvObjectOffset        = m_basicFeature->m_mvOffset;
    params.dwMvObjectSize          = m_basicFeature->m_mbCodeSize - m_basicFeature->m_mvOffset;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, Vp9DynamicScalPktXe_Lpm_Plus_Base)
{
    ENCODE_FUNC_CALL();

    params.psPreDeblockSurface    = &m_basicFeature->m_reconSurface;
    params.psPostDeblockSurface   = &m_basicFeature->m_reconSurface;
    params.psRawSurface           = m_basicFeature->m_rawSurfaceToPak;
    params.presCurMvTempBuffer    = m_basicFeature->m_resMvTemporalBuffer;
    params.presVp9SegmentIdBuffer = m_basicFeature->m_resSegmentIdBuffer;

    m_basicFeature->m_ref.MHW_SETPAR_F(HCP_PIPE_BUF_ADDR_STATE)(params);

#ifdef _MMC_SUPPORTED
    ENCODE_CHK_NULL_RETURN(m_mmcState);

    if (m_mmcState->IsMmcEnabled())
    {
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(&m_basicFeature->m_reconSurface, &params.PreDeblockSurfMmcState));
        params.PostDeblockSurfMmcState = params.PreDeblockSurfMmcState;
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(&m_basicFeature->m_rawSurface, &params.RawSurfMmcState));
    }
    else
    {
        params.PreDeblockSurfMmcState  = MOS_MEMCOMP_DISABLED;
        params.PostDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
        params.RawSurfMmcState         = MOS_MEMCOMP_DISABLED;
    }

    CODECHAL_DEBUG_TOOL(m_basicFeature->m_reconSurface.MmcState = params.PreDeblockSurfMmcState;)
#endif

    return MOS_STATUS_SUCCESS;
}

}

