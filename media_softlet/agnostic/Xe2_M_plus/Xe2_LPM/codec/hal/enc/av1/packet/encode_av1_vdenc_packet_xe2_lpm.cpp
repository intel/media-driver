/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     encode_av1_vdenc_packet_xe2_lpm.cpp
//! \brief    Defines the interface for av1 encode vdenc packet of Xe2_LPM
//!
#include "encode_av1_vdenc_packet_xe2_lpm.h"
#include "mhw_vdbox_avp_impl_xe2_lpm.h"
#include "mhw_vdbox_vdenc_impl_xe2_lpm.h"
#include "encode_av1_aqm.h"
#include "encode_av1_scc.h"
#include "hal_oca_interface_next.h"

namespace encode
{
Av1VdencPktXe2_Lpm::~Av1VdencPktXe2_Lpm()
{

}

MOS_STATUS Av1VdencPktXe2_Lpm::Init()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Av1VdencPktXe2_Lpm_Base::Init());

    CalculateAqmCommandsSize();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPktXe2_Lpm::AddAllCmds_AVP_SURFACE_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(cmdBuffer);

    Av1VdencPktXe2_Lpm_Base::AddAllCmds_AVP_SURFACE_STATE(cmdBuffer);

    m_curAvpSurfStateId = intrabcDecodedFrame;
    SETPAR_AND_ADDCMD(AVP_SURFACE_STATE, m_avpItf, cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPktXe2_Lpm::AddOneTileCommands(
    MOS_COMMAND_BUFFER &cmdBuffer,
    uint32_t            tileRow,
    uint32_t            tileCol,
    uint32_t            tileRowPass)
{
    ENCODE_FUNC_CALL();
    auto eStatus = MOS_STATUS_SUCCESS;

    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, SetCurrentTile, tileRow, tileCol, m_pipeline);

    //update IBC status for current tile
    RUN_FEATURE_INTERFACE_RETURN(Av1Scc, Av1FeatureIDs::av1Scc, UpdateIBCStatusForCurrentTile);
    
    if ((m_pipeline->GetPipeNum() > 1) && (tileCol != m_pipeline->GetCurrentPipe()))
    {
        return MOS_STATUS_SUCCESS;
    }

    // Begin patching tile level batch cmds
    MOS_COMMAND_BUFFER constructTileBatchBuf = {};
    PMOS_COMMAND_BUFFER tempCmdBuffer        = &cmdBuffer;

    // Add batch buffer start for tile
    PMHW_BATCH_BUFFER tileLevelBatchBuffer   = nullptr;

    if (!m_osInterface->bUsesPatchList)
    {
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, BeginPatchTileLevelBatch, tileRowPass, constructTileBatchBuf);

        // Add batch buffer start for tile
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileLevelBatchBuffer, tileLevelBatchBuffer);
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(&cmdBuffer, tileLevelBatchBuffer));

        tempCmdBuffer = &constructTileBatchBuf;
        MHW_MI_MMIOREGISTERS mmioRegister;
        if (m_vdencItf->ConvertToMiRegister(MHW_VDBOX_NODE_1, mmioRegister))
        {
            HalOcaInterfaceNext::On1stLevelBBStart(
                *tempCmdBuffer,
                (MOS_CONTEXT_HANDLE)m_osInterface->pOsContext,
                m_osInterface->CurrentGpuContextHandle,
                m_miItf,
                mmioRegister);
        }
    }

    auto brcFeature = dynamic_cast<Av1Brc *>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);
    auto vdenc2ndLevelBatchBuffer = brcFeature->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);
    auto slbbData                 = brcFeature->GetSLBData();

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_PIPE_MODE_SELECT(tempCmdBuffer));
    ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_SURFACE_STATE(tempCmdBuffer));

    SETPAR_AND_ADDCMD(AVP_PIPE_BUF_ADDR_STATE, m_avpItf, tempCmdBuffer);
    SETPAR_AND_ADDCMD(AVP_IND_OBJ_BASE_ADDR_STATE, m_avpItf, tempCmdBuffer);
    bool firstTileInGroup = false;
    if (brcFeature->IsBRCEnabled())
    {
        uint32_t tileGroupIdx     = 0;
        RUN_FEATURE_INTERFACE_NO_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, IsFirstTileInGroup, firstTileInGroup, tileGroupIdx);
        vdenc2ndLevelBatchBuffer->dwOffset = firstTileInGroup ? slbbData.avpPicStateOffset : slbbData.secondAvpPicStateOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(tempCmdBuffer, vdenc2ndLevelBatchBuffer));
        HalOcaInterfaceNext::OnSubLevelBBStart(
            *tempCmdBuffer,
            m_osInterface->pOsContext,
            &vdenc2ndLevelBatchBuffer->OsResource,
            vdenc2ndLevelBatchBuffer->dwOffset,
            false,
            slbbData.slbSize - vdenc2ndLevelBatchBuffer->dwOffset);
    }
    else
    {
        SETPAR_AND_ADDCMD(AVP_PIC_STATE, m_avpItf, tempCmdBuffer);
    }

    SETPAR_AND_ADDCMD(AVP_INTER_PRED_STATE, m_avpItf, tempCmdBuffer);

    if (brcFeature->IsBRCEnabled())
    {
        vdenc2ndLevelBatchBuffer->dwOffset = slbbData.avpSegmentStateOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(tempCmdBuffer, vdenc2ndLevelBatchBuffer));
        HalOcaInterfaceNext::OnSubLevelBBStart(
            *tempCmdBuffer,
            m_osInterface->pOsContext,
            &vdenc2ndLevelBatchBuffer->OsResource,
            vdenc2ndLevelBatchBuffer->dwOffset,
            false,
            slbbData.vdencCmd1Offset - vdenc2ndLevelBatchBuffer->dwOffset);
    }
    else
    {
        ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_SEGMENT_STATE(tempCmdBuffer));
        SETPAR_AND_ADDCMD(AVP_INLOOP_FILTER_STATE, m_avpItf, tempCmdBuffer);
    }

    SETPAR_AND_ADDCMD(AVP_TILE_CODING, m_avpItf, tempCmdBuffer);

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_PAK_INSERT_OBJECT(tempCmdBuffer));

    SETPAR_AND_ADDCMD(VDENC_WEIGHTSOFFSETS_STATE, m_vdencItf, tempCmdBuffer);

    if (brcFeature->IsBRCEnabled())
    {
        vdenc2ndLevelBatchBuffer->dwOffset = slbbData.vdencCmd1Offset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(tempCmdBuffer, vdenc2ndLevelBatchBuffer));
        HalOcaInterfaceNext::OnSubLevelBBStart(
            *tempCmdBuffer,
            m_osInterface->pOsContext,
            &vdenc2ndLevelBatchBuffer->OsResource,
            vdenc2ndLevelBatchBuffer->dwOffset,
            false,
            slbbData.vdencCmd2Offset - vdenc2ndLevelBatchBuffer->dwOffset);
    }
    else
    {
        SETPAR_AND_ADDCMD(VDENC_CMD1, m_vdencItf, tempCmdBuffer);
    }

    if (brcFeature->IsBRCEnabled() && m_basicFeature->m_av1PicParams->PicFlags.fields.PaletteModeEnable)
    {
        uint32_t tileIdx = 0;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileIdx, tileIdx);

        vdenc2ndLevelBatchBuffer->dwOffset = m_basicFeature->m_vdencTileSliceStart[tileIdx];
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(tempCmdBuffer, vdenc2ndLevelBatchBuffer));
        HalOcaInterfaceNext::OnSubLevelBBStart(
            *tempCmdBuffer,
            m_osInterface->pOsContext,
            &vdenc2ndLevelBatchBuffer->OsResource,
            vdenc2ndLevelBatchBuffer->dwOffset,
            false,
            slbbData.slbSize - vdenc2ndLevelBatchBuffer->dwOffset);
    }
    else
    {
        SETPAR_AND_ADDCMD(VDENC_HEVC_VP9_TILE_SLICE_STATE, m_vdencItf, tempCmdBuffer);
    }

    if (brcFeature->IsBRCEnabled())
    {
        vdenc2ndLevelBatchBuffer->dwOffset = slbbData.vdencCmd2Offset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(tempCmdBuffer, vdenc2ndLevelBatchBuffer));
        HalOcaInterfaceNext::OnSubLevelBBStart(
            *tempCmdBuffer,
            m_osInterface->pOsContext,
            &vdenc2ndLevelBatchBuffer->OsResource,
            vdenc2ndLevelBatchBuffer->dwOffset,
            false,
            (firstTileInGroup ? slbbData.avpPicStateOffset : slbbData.secondAvpPicStateOffset) - vdenc2ndLevelBatchBuffer->dwOffset);
    }
    else
    {
        SETPAR_AND_ADDCMD(VDENC_CMD2, m_vdencItf, tempCmdBuffer);
    }

    ENCODE_CHK_STATUS_RETURN(AddAqmCommands(tempCmdBuffer));

    m_basicFeature->m_flushCmd = Av1BasicFeature::waitVdenc;
    SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, tempCmdBuffer);

    ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(*tempCmdBuffer));

    if (!m_osInterface->bUsesPatchList)
    {
        // For 2nd level BB, we must use tileLevelBatchBuffer to prevent adding Epilogue before MI_BATCH_BUFFER_END
        ENCODE_CHK_NULL_RETURN(tileLevelBatchBuffer);
        tileLevelBatchBuffer->iCurrent   = constructTileBatchBuf.iOffset;
        tileLevelBatchBuffer->iRemaining = constructTileBatchBuf.iRemaining;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_END)(nullptr, tileLevelBatchBuffer));
        HalOcaInterfaceNext::OnSubLevelBBStart(
            cmdBuffer,
            m_osInterface->pOsContext,
            &tempCmdBuffer->OsResource,
            0,
            false,
            tempCmdBuffer->iOffset);
        HalOcaInterfaceNext::On1stLevelBBEnd(*tempCmdBuffer, *m_osInterface);
    }

#if USE_CODECHAL_DEBUG_TOOL
    std::string             name           = std::to_string(tileRow) + std::to_string(tileCol) + std::to_string(tileRowPass) + "_TILE_CMD_BUFFER";
    CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpCmdBuffer(
        &constructTileBatchBuf,
        CODECHAL_NUM_MEDIA_STATES,
        name.c_str()));
#endif

    // End patching tile level batch cmds
    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, EndPatchTileLevelBatch);

    ENCODE_CHK_STATUS_RETURN(ReadPakMmioRegisters(&cmdBuffer, tileRow == 0 && tileCol == 0));

    ENCODE_CHK_STATUS_RETURN(PrepareHWMetaDataFromStreamoutTileLevel(&cmdBuffer, tileCol, tileRow));

    return eStatus;
}

MOS_STATUS Av1VdencPktXe2_Lpm::AddAqmCommands(PMOS_COMMAND_BUFFER cmdBuffer)
{
    ENCODE_FUNC_CALL();
    auto eStatus = MOS_STATUS_SUCCESS;

    auto aqmFeature = dynamic_cast<Av1EncodeAqm *>(m_featureManager->GetFeature(Av1FeatureIDs::av1Aqm));
    ENCODE_CHK_NULL_RETURN(aqmFeature);

    if (aqmFeature->IsEnabled())
    {
        SETPAR_AND_ADDCMD(AQM_VD_CONTROL_STATE, m_aqmItf, cmdBuffer);
    }
    if (aqmFeature->IsEnabled())
    {
        SETPAR_AND_ADDCMD(AQM_PIPE_BUF_ADDR_STATE, m_aqmItf, cmdBuffer);
        SETPAR_AND_ADDCMD(AQM_PIC_STATE, m_aqmItf, cmdBuffer);
    }

    if (aqmFeature->IsEnabled())
    {
        SETPAR_AND_ADDCMD(AQM_TILE_CODING, m_aqmItf, cmdBuffer);
        SETPAR_AND_ADDCMD(AQM_FRAME_START, m_aqmItf, cmdBuffer);
    }
    SETPAR_AND_ADDCMD(VDENC_WALKER_STATE, m_vdencItf, cmdBuffer);
    if (aqmFeature->IsEnabled())
    {
        SETPAR_AND_ADDCMD(AQM_SLICE_STATE, m_aqmItf, cmdBuffer);
    }

    return eStatus;
}

MOS_STATUS Av1VdencPktXe2_Lpm::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
{
    ENCODE_FUNC_CALL();
    auto eStatus = MOS_STATUS_SUCCESS;
    eStatus = Av1VdencPkt::Completed(mfxStatus, rcsStatus, statusReport);

    auto aqmFeature = dynamic_cast<Av1EncodeAqm *>(m_featureManager->GetFeature(Av1FeatureIDs::av1Aqm));
    ENCODE_CHK_NULL_RETURN(aqmFeature);
    if (aqmFeature->IsEnabled())
    {
        EncodeStatusReportData *statusReportData = (EncodeStatusReportData *)statusReport;
        uint32_t                statBufIdx       = statusReportData->currOriginalPic.FrameIdx;
        if (m_basicFeature->m_av1PicParams->QualityInfoSupportFlags.fields.enable_frame)
        {
            ENCODE_CHK_STATUS_RETURN(aqmFeature->ReportQualityInfoFrame(statBufIdx, *statusReportData));
        }
#if _MEDIA_RESERVED
        if (m_basicFeature->m_av1PicParams->QualityInfoSupportFlags.fields.enable_block)
        {
            ENCODE_CHK_STATUS_RETURN(aqmFeature->ReportQualityInfoBlock(statBufIdx, *statusReportData));
        }
#if USE_CODECHAL_DEBUG_TOOL
        ENCODE_CHK_STATUS_RETURN(aqmFeature->DumpVdaqmOutput(statBufIdx, *statusReportData));
#endif
#endif 
    }

    return eStatus;
}

MOS_STATUS Av1VdencPktXe2_Lpm::CalculateAqmCommandsSize()
{
    uint32_t aqmTileStatesSize    = 0;
    uint32_t aqmTilePatchListSize = 0;

    // Tile Level Commands
    ENCODE_CHK_STATUS_RETURN(GetAqmPrimitiveCommandsDataSize(
        &aqmTileStatesSize,
        &aqmTilePatchListSize));

    m_tileStatesSize += aqmTileStatesSize;
    m_tilePatchListSize += aqmTilePatchListSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPktXe2_Lpm::GetAqmPrimitiveCommandsDataSize(uint32_t *commandsSize, uint32_t *patchListSize) const
{
    uint32_t maxSize          = 0;
    uint32_t patchListMaxSize = 0;

    ENCODE_CHK_NULL_RETURN(commandsSize);
    ENCODE_CHK_NULL_RETURN(patchListSize);

    auto aqmFeature = dynamic_cast<Av1EncodeAqm *>(m_featureManager->GetFeature(Av1FeatureIDs::av1Aqm));
    ENCODE_CHK_NULL_RETURN(aqmFeature);
    if (aqmFeature->IsEnabled())
    {
        maxSize += m_aqmItf->MHW_GETSIZE_F(AQM_VD_CONTROL_STATE)() +
                   m_aqmItf->MHW_GETSIZE_F(AQM_PIPE_BUF_ADDR_STATE)() +
                   m_aqmItf->MHW_GETSIZE_F(AQM_PIC_STATE)() +
                   m_aqmItf->MHW_GETSIZE_F(AQM_TILE_CODING)() +
                   m_aqmItf->MHW_GETSIZE_F(AQM_FRAME_START)() +
                   m_aqmItf->MHW_GETSIZE_F(AQM_SLICE_STATE)();

        patchListMaxSize +=
            PATCH_LIST_COMMAND(mhw::vdbox::aqm::Itf::AQM_VD_CONTROL_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::aqm::Itf::AQM_PIPE_BUF_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::aqm::Itf::AQM_PIC_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::aqm::Itf::AQM_TILE_CODING_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::aqm::Itf::AQM_FRAME_START_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::aqm::Itf::AQM_SLICE_STATE_CMD);
    }

    *commandsSize  = maxSize;
    *patchListSize = patchListMaxSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPktXe2_Lpm::EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(cmdBuffer);
    ENCODE_CHK_STATUS_RETURN(Av1VdencPkt::EndStatusReport(srType, cmdBuffer));

    MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
    ENCODE_CHK_NULL_RETURN(perfProfiler);

    //store bitstream size to UMD profiler
    ENCODE_CHK_NULL_RETURN(m_avpItf);
    auto mmioRegisters = m_avpItf->GetMmioRegisters(m_vdboxIndex);
    ENCODE_CHK_STATUS_RETURN(perfProfiler->AddStoreBitstreamSizeCmd(
        (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer, mmioRegisters->avpAv1BitstreamByteCountTileRegOffset));
#if _MEDIA_RESERVED
    //store quality metric to UMD profiler
    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeAqm, Av1FeatureIDs::av1Aqm, AddStoreQualityMetricCmd, (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer);
#endif
    return MOS_STATUS_SUCCESS;
}
}  // namespace encode
