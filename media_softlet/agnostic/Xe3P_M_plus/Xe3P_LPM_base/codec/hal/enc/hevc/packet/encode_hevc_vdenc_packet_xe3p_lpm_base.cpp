/*
* Copyright (c) 2023-2026, Intel Corporation
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
//! \file     encode_hevc_vdenc_packet_xe3p_lpm_base.cpp
//! \brief    Defines the interface for hevc encode vdenc packet of Xe3p_LPM_Base
//!

#include "encode_hevc_vdenc_packet_xe3p_lpm_base.h"
#include "encode_hevc_basic_feature_xe3p_lpm_base.h"
#include "encode_hevc_brc.h"
#include "encode_hevc_cqp.h"
#include "encode_hevc_aqm.h"
#include "hal_oca_interface_next.h"

namespace encode
{
MOS_STATUS HevcVdencPktXe3P_Lpm_Base::AddPicStateWithNoTile(
    MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    bool tileEnabled = false;
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsEnabled, tileEnabled);
    if (tileEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    // Both BRC and CQP use MI_BATCH_BUFFER_START to SLBB output
    // BRC: HuC BRC writes updated SLBB to brcFeature's buffer
    // CQP: HuC SLBB Update writes updated SLBB to basicFeature's buffer
    PMHW_BATCH_BUFFER vdenc2ndLevelBatchBuffer = nullptr;
    if (brcFeature->IsBRCUpdateRequired())
    {
        vdenc2ndLevelBatchBuffer = brcFeature->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);
    }
    else
    {
        auto basicFeatureXe3pLpm = dynamic_cast<HevcBasicFeatureXe3p_Lpm_Base *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(basicFeatureXe3pLpm);
        vdenc2ndLevelBatchBuffer = basicFeatureXe3pLpm->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);
    }
    ENCODE_CHK_NULL_RETURN(vdenc2ndLevelBatchBuffer);
    vdenc2ndLevelBatchBuffer->dwOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize;

    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(&cmdBuffer, vdenc2ndLevelBatchBuffer));
    HalOcaInterfaceNext::OnSubLevelBBStart(
        cmdBuffer,
        m_osInterface->pOsContext,
        &vdenc2ndLevelBatchBuffer->OsResource,
        vdenc2ndLevelBatchBuffer->dwOffset,
        false,
        m_hwInterface->m_vdencBatchBuffer2ndGroupSize);

    auto rdoqFeature = dynamic_cast<HevcEncodeCqp *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcCqpFeature));
    ENCODE_CHK_NULL_RETURN(rdoqFeature);
    if (rdoqFeature->IsRDOQEnabled())
    {
        SETPAR_AND_ADDCMD(HEVC_VP9_RDOQ_STATE, m_hcpItf, &cmdBuffer);
    }

    auto aqmFeature = dynamic_cast<HevcEncodeAqm *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcAqm));
    ENCODE_CHK_NULL_RETURN(aqmFeature);
    if (aqmFeature->IsEnabled())
    {
        SETPAR_AND_ADDCMD(AQM_VD_CONTROL_STATE, m_aqmItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(AQM_PIPE_BUF_ADDR_STATE, m_aqmItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(AQM_PIC_STATE, m_aqmItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(AQM_TILE_CODING, m_aqmItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(AQM_FRAME_START, m_aqmItf, &cmdBuffer);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPktXe3P_Lpm_Base::AddPicStateWithTile(
    MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    bool tileEnabled = false;
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsEnabled, tileEnabled);
    if (!tileEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    // Both BRC and CQP use MI_BATCH_BUFFER_START to SLBB output
    PMHW_BATCH_BUFFER vdenc2ndLevelBatchBuffer = nullptr;
    if (brcFeature->IsBRCUpdateRequired())
    {
        vdenc2ndLevelBatchBuffer = brcFeature->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);
    }
    else
    {
        auto basicFeatureXe3pLpm = dynamic_cast<HevcBasicFeatureXe3p_Lpm_Base *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(basicFeatureXe3pLpm);
        vdenc2ndLevelBatchBuffer = basicFeatureXe3pLpm->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);
    }
    ENCODE_CHK_NULL_RETURN(vdenc2ndLevelBatchBuffer);
    vdenc2ndLevelBatchBuffer->dwOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize;

    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(&cmdBuffer, vdenc2ndLevelBatchBuffer));
    HalOcaInterfaceNext::OnSubLevelBBStart(
        cmdBuffer,
        m_osInterface->pOsContext,
        &vdenc2ndLevelBatchBuffer->OsResource,
        vdenc2ndLevelBatchBuffer->dwOffset,
        false,
        m_hwInterface->m_vdencBatchBuffer2ndGroupSize);

    // Send HEVC_VP9_RDOQ_STATE command
    SETPAR_AND_ADDCMD(HEVC_VP9_RDOQ_STATE, m_hcpItf, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPktXe3P_Lpm_Base::AddAQMCommands(
    MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();
    auto eStatus = MOS_STATUS_SUCCESS;

    auto aqmFeature = dynamic_cast<HevcEncodeAqm *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcAqm));
    ENCODE_CHK_NULL_RETURN(aqmFeature);
    if (aqmFeature->IsEnabled())
    {
        SETPAR_AND_ADDCMD(AQM_VD_CONTROL_STATE, m_aqmItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(AQM_PIPE_BUF_ADDR_STATE, m_aqmItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(AQM_PIC_STATE, m_aqmItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(AQM_TILE_CODING, m_aqmItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(AQM_FRAME_START, m_aqmItf, &cmdBuffer);
    }

    return eStatus;
}

MOS_STATUS HevcVdencPktXe3P_Lpm_Base::AddOneTileCommands(
    MOS_COMMAND_BUFFER &cmdBuffer,
    uint32_t            tileRow,
    uint32_t            tileCol,
    uint32_t            tileRowPass)
{
    ENCODE_FUNC_CALL();
    PMOS_COMMAND_BUFFER tempCmdBuffer        = &cmdBuffer;
    PMHW_BATCH_BUFFER   tileLevelBatchBuffer = nullptr;
    auto                eStatus              = MOS_STATUS_SUCCESS;
    MOS_COMMAND_BUFFER constructTileBatchBuf = {};

    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, SetCurrentTile, tileRow, tileCol, m_pipeline);

    if ((m_pipeline->GetPipeNum() > 1) && (tileCol != m_pipeline->GetCurrentPipe()))
    {
        return MOS_STATUS_SUCCESS;
    }

    if (!m_osInterface->bUsesPatchList)
    {
        // Begin patching tile level batch cmds
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, BeginPatchTileLevelBatch, tileRowPass, constructTileBatchBuf);

        // Add batch buffer start for tile
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileLevelBatchBuffer, tileLevelBatchBuffer);
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

    // HCP Lock for multiple pipe mode
    if (m_pipeline->GetPipeNum() > 1)
    {
        auto &vdControlStateParams                = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
        vdControlStateParams                      = {};
        vdControlStateParams.scalableModePipeLock = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(tempCmdBuffer));
    }

    SETPAR_AND_ADDCMD(VDENC_PIPE_MODE_SELECT, m_vdencItf, tempCmdBuffer);

    // for Gen11+, we need to add MFX wait for both KIN and VRT before and after HCP Pipemode select...
    auto &mfxWaitParams                 = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
    mfxWaitParams                       = {};
    mfxWaitParams.iStallVdboxPipeline   = true;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(tempCmdBuffer));
    SETPAR_AND_ADDCMD(HCP_PIPE_MODE_SELECT, m_hcpItf, tempCmdBuffer);
    mfxWaitParams                       = {};
    mfxWaitParams.iStallVdboxPipeline   = true;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(tempCmdBuffer));

    ENCODE_CHK_STATUS_RETURN(AddPicStateWithTile(*tempCmdBuffer));

    SETPAR_AND_ADDCMD(HCP_TILE_CODING, m_hcpItf, tempCmdBuffer);

    ENCODE_CHK_STATUS_RETURN(AddAQMCommands(*tempCmdBuffer));
    ENCODE_CHK_STATUS_RETURN(AddSlicesCommandsInTile(*tempCmdBuffer));

    //HCP unLock for multiple pipe mode
    if (m_pipeline->GetPipeNum() > 1)
    {
        auto &vdControlStateParams                  = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
        vdControlStateParams                        = {};
        vdControlStateParams.scalableModePipeUnlock = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(tempCmdBuffer));
    }

    m_flushCmd = waitHevc;
    SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, tempCmdBuffer);

    ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(*tempCmdBuffer));

    if (!m_osInterface->bUsesPatchList)
    {
        // For 2nd level BB, we must use tileLevelBatchBuffer to prevent adding Epilogue before MI_BATCH_BUFFER_END
        ENCODE_CHK_NULL_RETURN(tileLevelBatchBuffer);
        tileLevelBatchBuffer->iCurrent   = tempCmdBuffer->iOffset;
        tileLevelBatchBuffer->iRemaining = tempCmdBuffer->iRemaining;
        ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, tileLevelBatchBuffer));
        HalOcaInterfaceNext::OnSubLevelBBStart(
            cmdBuffer,
            m_osInterface->pOsContext,
            &tempCmdBuffer->OsResource,
            0,
            false,
            tempCmdBuffer->iOffset);
        HalOcaInterfaceNext::On1stLevelBBEnd(*tempCmdBuffer, *m_osInterface);

#if USE_CODECHAL_DEBUG_TOOL
        if (tempCmdBuffer->pCmdPtr && tempCmdBuffer->pCmdBase &&
            tempCmdBuffer->pCmdPtr > tempCmdBuffer->pCmdBase)
        {
            CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
            std::string             name("TileLevelBatchBuffer");
            name += "Row" + std::to_string(tileRow) + "Col" + std::to_string(tileCol);

            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpData(
                tempCmdBuffer->pCmdBase,
                (uint32_t)(4 * (tempCmdBuffer->pCmdPtr - tempCmdBuffer->pCmdBase)),
                CodechalDbgAttr::attrCmdBufferMfx,
                name.c_str()));
        }
#endif
    }

    // End patching tile level batch cmds
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, EndPatchTileLevelBatch);

    return eStatus;
}

MOS_STATUS HevcVdencPktXe3P_Lpm_Base::SendHwSliceEncodeCommand(const PCODEC_ENCODER_SLCDATA slcData, const uint32_t currSlcIdx, MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    if (brcFeature->IsBRCUpdateRequired())
    {
        // BRC path: use base class
        if (m_basicFeature->m_hevcPicParams->AdaptiveTUEnabled == 0)
        {
            eStatus = HevcVdencPkt::SendHwSliceEncodeCommand(slcData, currSlcIdx, cmdBuffer);
        }
        else
        {
            // VDENC does not use batch buffer for slice state
            // add HCP_REF_IDX command
            ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_REF_IDX_STATE(&cmdBuffer));

            bool              vdencHucInUse    = false;
            PMHW_BATCH_BUFFER vdencBatchBuffer = nullptr;

            RUN_FEATURE_INTERFACE_RETURN(HEVCEncodeBRC, HevcFeatureIDs::hevcBrcFeature, SetVdencBatchBufferState, m_pipeline->m_currRecycledBufIdx, currSlcIdx, vdencBatchBuffer, vdencHucInUse);
            PMHW_BATCH_BUFFER secondLevelBatchBufferUsed = vdencBatchBuffer;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START(&cmdBuffer, secondLevelBatchBufferUsed)));
            HalOcaInterfaceNext::OnSubLevelBBStart(
                cmdBuffer,
                m_osInterface->pOsContext,
                &secondLevelBatchBufferUsed->OsResource,
                secondLevelBatchBufferUsed->dwOffset,
                false,
                m_basicFeature->m_vdencBatchBufferPerSlicePart2Start[currSlcIdx] - secondLevelBatchBufferUsed->dwOffset);
            ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_PAK_INSERT_OBJECT_BRC(&cmdBuffer));
            secondLevelBatchBufferUsed->dwOffset = m_basicFeature->m_vdencBatchBufferPerSlicePart2Start[currSlcIdx];
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START(&cmdBuffer, secondLevelBatchBufferUsed)));
            HalOcaInterfaceNext::OnSubLevelBBStart(
                cmdBuffer,
                m_osInterface->pOsContext,
                &secondLevelBatchBufferUsed->OsResource,
                secondLevelBatchBufferUsed->dwOffset,
                false,
                m_basicFeature->m_vdencBatchBufferPerSlicePart2Size[currSlcIdx]);
            // TILE_SLICE_STATE via SLBB (contains HuC-applied SCC sensitive fields)
            auto basicFeatureXe3pLpm = dynamic_cast<HevcBasicFeatureXe3p_Lpm_Base *>(m_basicFeature);
            ENCODE_CHK_NULL_RETURN(basicFeatureXe3pLpm);
            PMHW_BATCH_BUFFER slbbBuffer = basicFeatureXe3pLpm->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);
            ENCODE_CHK_NULL_RETURN(slbbBuffer);
            slbbBuffer->dwOffset = m_basicFeature->m_vdencBatchBufferTileSliceStart[currSlcIdx];
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(&cmdBuffer, slbbBuffer));

            SETPAR_AND_ADDCMD(VDENC_WALKER_STATE, m_vdencItf, &cmdBuffer);
        }
    }
    else
    {
        // CQP path: emit Group3 inline, but use MI_BATCH_BUFFER_START for
        // VDENC_HEVC_VP9_TILE_SLICE_STATE from SLBB output (HuC-updated)
        ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_REF_IDX_STATE(&cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_WEIGHTOFFSET_STATE(&cmdBuffer));
        m_basicFeature->m_useDefaultRoundingForHcpSliceState = false;
        SETPAR_AND_ADDCMD(HCP_SLICE_STATE, m_hcpItf, &cmdBuffer);
        ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_PAK_INSERT_OBJECT(&cmdBuffer));
        SETPAR_AND_ADDCMD(VDENC_WEIGHTSOFFSETS_STATE, m_vdencItf, &cmdBuffer);

        // TILE_SLICE_STATE via SLBB (contains HuC-applied SCC sensitive fields)
        auto basicFeatureXe3pLpm = dynamic_cast<HevcBasicFeatureXe3p_Lpm_Base *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(basicFeatureXe3pLpm);
        PMHW_BATCH_BUFFER slbbBuffer = basicFeatureXe3pLpm->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);
        ENCODE_CHK_NULL_RETURN(slbbBuffer);
        slbbBuffer->dwOffset = m_basicFeature->m_vdencBatchBufferTileSliceStart[currSlcIdx];
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(&cmdBuffer, slbbBuffer));

        SETPAR_AND_ADDCMD(VDENC_WALKER_STATE, m_vdencItf, &cmdBuffer);
    }

    auto aqmFeature = dynamic_cast<HevcEncodeAqm *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcAqm));
    ENCODE_CHK_NULL_RETURN(aqmFeature);
    if (aqmFeature->IsEnabled())
    {
        SETPAR_AND_ADDCMD(AQM_SLICE_STATE, m_aqmItf, &cmdBuffer);
    }

    return eStatus;
}

MOS_STATUS HevcVdencPktXe3P_Lpm_Base::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
{
    ENCODE_FUNC_CALL();
    auto eStatus = MOS_STATUS_SUCCESS;
    eStatus      = HevcVdencPkt::Completed(mfxStatus, rcsStatus, statusReport);

    auto aqmFeature = dynamic_cast<HevcEncodeAqm *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcAqm));
    ENCODE_CHK_NULL_RETURN(aqmFeature);
    if (aqmFeature->IsEnabled())
    {
        EncodeStatusReportData *statusReportData = (EncodeStatusReportData *)statusReport;
        uint32_t                statBufIdx       = statusReportData->currOriginalPic.FrameIdx;
        if (m_basicFeature->m_hevcPicParams->QualityInfoSupportFlags.fields.enable_frame)
        {
            ENCODE_CHK_STATUS_RETURN(aqmFeature->ReportQualityInfoFrame(statBufIdx, *statusReportData));
        }

#if _MEDIA_RESERVED
#if USE_CODECHAL_DEBUG_TOOL
        ENCODE_CHK_STATUS_RETURN(aqmFeature->DumpVdaqmOutput(statBufIdx, *statusReportData));
#endif
#endif // _MEDIA_RESERVED
    }

    return eStatus;
}

MOS_STATUS HevcVdencPktXe3P_Lpm_Base::EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    // Send MI_FLUSH command
    auto &flushDwParams                         = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams                               = {};
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    auto *skuTable = m_hwInterface->GetSkuTable();
    if (skuTable && MEDIA_IS_SKU(skuTable, FtrEnablePPCFlush))
    {
        // Add PPC fulsh
        flushDwParams.bEnablePPCFlush = true;
    }
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS HevcVdencPktXe3P_Lpm_Base::DumpResources(
    EncodeStatusMfx        *encodeStatusMfx,
    EncodeStatusReportData *statusReportData)
{
        ENCODE_FUNC_CALL();
        ENCODE_CHK_STATUS_RETURN(HevcVdencPkt::DumpResources(encodeStatusMfx, statusReportData));

        CodechalDebugInterface *debugInterface = m_pipeline->GetStatusReportDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_ref.DumpReferences(*debugInterface));

        return MOS_STATUS_SUCCESS;
}
#endif
MHW_SETPAR_DECL_SRC(VD_PIPELINE_FLUSH, HevcVdencPktXe3P_Lpm_Base)
{
    HevcVdencPkt::MHW_SETPAR_F(VD_PIPELINE_FLUSH)(params);

    auto aqmFeature = dynamic_cast<HevcEncodeAqm *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcAqm));
    ENCODE_CHK_NULL_RETURN(aqmFeature);
    if (aqmFeature->IsEnabled())
    {
        switch (m_flushCmd)
        {
        case waitVdenc:
        case waitHevcVdenc:
            params.waitDoneVDAQM = true;
            params.flushVDAQM    = true;
            break;
        default:
            break;
        }
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
