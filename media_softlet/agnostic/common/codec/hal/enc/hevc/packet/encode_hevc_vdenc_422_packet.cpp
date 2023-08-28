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
//! \file     encode_hevc_vdenc_422_packet.cpp
//! \brief    Defines the interface to adapt to HEVC VDENC pipeline
//!

#include "encode_hevc_vdenc_422_packet.h"

namespace encode
{
MOS_STATUS HevcVdencPkt422::AllocateResources()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(HevcVdencPkt::AllocateResources());

    const uint32_t picWidthInLCU  = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, 64);
    const uint32_t picHeightInLCU = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameHeight, 64);
    m_422maxNumLCUs               = picWidthInLCU * picHeightInLCU;
    m_422mvOffset                 = MOS_ALIGN_CEIL((m_422maxNumLCUs * (MOS_BYTES_TO_DWORDS(sizeof(PakRSVD1))) * sizeof(uint32_t)), CODECHAL_PAGE_SIZE);
    m_422mbCodeSize               = m_422mvOffset + MOS_ALIGN_CEIL((m_422maxNumLCUs * 64 * sizeof(PakRSVD2)), CODECHAL_PAGE_SIZE);

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type             = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType         = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format           = Format_Buffer;
    allocParamsForBufferLinear.dwMemType        = MOS_MEMPOOL_SYSTEMMEMORY;
    allocParamsForBufferLinear.Flags.bCacheable = true;
    // for now we set ResUsageType to MOS_HW_RESOURCE_USAGE_ENCODE_OUTPUT_BITSTREAM rather than
    // MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE to enable coherency in gmm
    allocParamsForBufferLinear.ResUsageType     = MOS_HW_RESOURCE_USAGE_ENCODE_OUTPUT_BITSTREAM;

    allocParamsForBufferLinear.dwBytes  = m_422mbCodeSize;
    allocParamsForBufferLinear.pBufName = "Standalone PAK Input Buffer";
    m_res422MbCodeBuffer                = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
    ENCODE_CHK_NULL_RETURN(m_res422MbCodeBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPkt422::Conversion()
{
    ENCODE_FUNC_CALL();

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly       = 1;
    uint8_t *mbCode420Buffer = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, m_basicFeature->m_resMbCodeBuffer, &lockFlags);
    ENCODE_CHK_NULL_RETURN(mbCode420Buffer);
    lockFlags.ReadOnly       = 0;
    lockFlags.WriteOnly      = 1;
    uint8_t *mbCode422Buffer = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, m_res422MbCodeBuffer, &lockFlags);
    ENCODE_CHK_NULL_RETURN(mbCode422Buffer);

    uint8_t *data         = mbCode420Buffer;
    uint8_t *tempPakRsvd1   = mbCode422Buffer;
    uint8_t *tempPakRsvd2 = mbCode422Buffer + m_422mvOffset;

    uint32_t opcode = 0x73A10000;
    while ((uint32_t)(data - mbCode420Buffer) < m_basicFeature->m_mbCodeSize)
    {
        if (*((uint32_t *)data) == opcode)
            break;
        else
            data += 4;
    }

    for (uint32_t i = 0; i < m_422maxNumLCUs; i++)
    {
        if (*((uint32_t *)data) != opcode)
        {
            ENCODE_ASSERTMESSAGE("VDEnc 420 pass not finished.");
            break;
        }

        StreamoutRSVD1 *streamoutRsvd1 = (StreamoutRSVD1 *)data;
        data += sizeof(StreamoutRSVD1);
        for (int j = 0; j < 5; j++)
        {
            uint32_t *tempdata = (uint32_t *)tempPakRsvd1 + j;
            *tempdata          = j ? streamoutRsvd1->DW[j].DW0 : 0x73A10003;
        }
        PakRSVD1 *pakRsvd1 = (PakRSVD1 *)tempPakRsvd1;
        tempPakRsvd1 += sizeof(PakRSVD1);

        if ((pakRsvd1->DW1>>31)&0x01)
        {
            pakRsvd1->DW5 = 0x05000000;
        }

        uint32_t cuCount = 0;
        for (; cuCount < 64; cuCount++)
        {
            StreamoutRSVD2 *streamoutRsvd2 = (StreamoutRSVD2 *)data;
            data += sizeof(StreamoutRSVD2);
            for (int j = 0; j < 8; j++)
            {
                uint32_t *tempdata = (uint32_t *)tempPakRsvd2 + j;
                *tempdata          = streamoutRsvd2->DW[j].DW0;
            }
            PakRSVD2 *pakRsvd2 = (PakRSVD2 *)tempPakRsvd2;
            tempPakRsvd2 += sizeof(PakRSVD2);
            pakRsvd2->DW7 &= 0xFF7FFFFF;

            pakRsvd2->DW6 &= 0xFFBFFFFF;
            pakRsvd2->DW4 &= 0x0000FFFF;

            if(((pakRsvd2->DW6>>23)&0X01) || cuCount == 63)
            {
                pakRsvd1->DW1 &=0xC0FFFFFF;
                pakRsvd1->DW1 += ((cuCount&0x3F)<<24);
                pakRsvd2->DW6 &= 0xFF7FFFFF;
                break;
            }
           pakRsvd2->DW6 &= 0xFF7FFFFF;
        }
        tempPakRsvd2 += (63 - cuCount) * sizeof(PakRSVD2);
    }
    PakRSVD1 *pakRsvd1 = (PakRSVD1 *)tempPakRsvd1;
    pakRsvd1--;
    pakRsvd1->DW5 = 0x05000000;

    ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(m_osInterface, m_basicFeature->m_resMbCodeBuffer));
    ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(m_osInterface, m_res422MbCodeBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPkt422::Prepare()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_422State);
    ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_422State->Revert422Format(m_basicFeature->m_hevcSeqParams,
        m_basicFeature->m_outputChromaFormat,
        m_basicFeature->m_reconSurface.Format,
        m_basicFeature->m_is10Bit));

    ENCODE_CHK_STATUS_RETURN(HevcVdencPkt::Prepare());
    ENCODE_CHK_STATUS_RETURN(Conversion());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPkt422::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_422State);

    if (m_basicFeature->m_422State && m_basicFeature->m_422State->GetFeature422Flag())
    {
        ENCODE_CHK_STATUS_RETURN(HevcVdencPkt::Completed(mfxStatus, rcsStatus, statusReport));
    }
    else
    {
        // When 422 wa feature is not enabled, not need above complete options
        return MOS_STATUS_SUCCESS;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPkt422::PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(m_basicFeature->m_frameWidth, m_basicFeature->m_frameHeight, true));

    SetPerfTag();

    ENCODE_CHK_STATUS_RETURN(AddForceWakeup(cmdBuffer));

    // Send command buffer header at the beginning (OS dependent)
    ENCODE_CHK_STATUS_RETURN(SendPrologCmds(cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(StartStatusReport(statusReportMfx, &cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(AddPictureHcpCommands(cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(AddPicStateWithNoTile(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPkt422::PatchSliceLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();

    if (m_hevcPicParams->tiles_enabled_flag)
    {
        return MOS_STATUS_SUCCESS;
    }
    ENCODE_CHK_STATUS_RETURN(SetBatchBufferForPakSlices());

    PCODEC_ENCODER_SLCDATA slcData = m_basicFeature->m_slcData;
    for (uint32_t startLcu = 0, slcCount = 0; slcCount < m_basicFeature->m_numSlices; slcCount++)
    {
        if (m_pipeline->IsFirstPass())
        {
            slcData[slcCount].CmdOffset = startLcu * (m_hcpItf->GetHcpPakObjSize()) * sizeof(uint32_t);
        }

        m_basicFeature->m_curNumSlices = slcCount;

        ENCODE_CHK_STATUS_RETURN(SendHwSliceEncodeCommand(slcData, slcCount, cmdBuffer));

        startLcu += m_hevcSliceParams[slcCount].NumLCUsInSlice;

        m_batchBufferForPakSlicesStartOffset = (uint32_t)m_batchBufferForPakSlices[m_basicFeature->m_currPakSliceIdx].iCurrent;
    }

    if (m_useBatchBufferForPakSlices)
    {
        ENCODE_CHK_STATUS_RETURN(Mhw_UnlockBb(
            m_osInterface,
            &m_batchBufferForPakSlices[m_basicFeature->m_currPakSliceIdx],
            m_lastTaskInPhase));
    }

    // Insert end of sequence/stream if set
    if (m_basicFeature->m_lastPicInSeq || m_basicFeature->m_lastPicInStream)
    {
        ENCODE_CHK_STATUS_RETURN(InsertSeqStreamEnd(cmdBuffer));
    }

    ENCODE_CHK_STATUS_RETURN(ReadHcpStatus(m_vdboxIndex, m_statusReport, cmdBuffer));
    ENCODE_CHK_STATUS_RETURN(ReadExtStatistics(cmdBuffer));
    ENCODE_CHK_STATUS_RETURN(ReadSliceSize(cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(EndStatusReport(statusReportMfx, &cmdBuffer));

    if (m_osInterface->bInlineCodecStatusUpdate)
    {
        ENCODE_CHK_STATUS_RETURN(UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));
    }

    CODECHAL_DEBUG_TOOL(
        if (m_mmcState) {
            m_mmcState->UpdateUserFeatureKey(&(m_basicFeature->m_reconSurface));
        })
    // Reset parameters for next PAK execution
    if (false == m_pipeline->IsFrameTrackingEnabled() && m_pipeline->IsLastPass() && m_pipeline->IsLastPipe())
    {
        UpdateParameters();
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPkt422::AddPicStateWithNoTile(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    bool tileEnabled = false;
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsEnabled, tileEnabled);
    if (tileEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    SETPAR_AND_ADDCMD(HCP_PIC_STATE, m_hcpItf, &cmdBuffer);

    SETPAR_AND_ADDCMD(HEVC_VP9_RDOQ_STATE, m_hcpItf, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPkt422::SendHwSliceEncodeCommand(const PCODEC_ENCODER_SLCDATA slcData, const uint32_t currSlcIdx, MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    PMHW_BATCH_BUFFER   batchBufferInUse = nullptr;
    PMOS_COMMAND_BUFFER cmdBufferInUse   = nullptr;

    if (m_useBatchBufferForPakSlices)
    {
        batchBufferInUse = &m_batchBufferForPakSlices[m_basicFeature->m_currPakSliceIdx];
        ENCODE_CHK_NULL_RETURN(batchBufferInUse);
    }
    else
    {
        cmdBufferInUse = &cmdBuffer;
    }

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_REF_IDX_STATE(&cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_WEIGHTOFFSET_STATE(&cmdBuffer));

    m_basicFeature->m_useDefaultRoundingForHcpSliceState = true;
    SETPAR_AND_ADDCMD(HCP_SLICE_STATE, m_hcpItf, &cmdBuffer);

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_PAK_INSERT_OBJECT(&cmdBuffer));

    if (m_useBatchBufferForPakSlices && batchBufferInUse)
    {
        ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, batchBufferInUse));

        MHW_BATCH_BUFFER secondLevelBatchBuffer;
        MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(MHW_BATCH_BUFFER));
        secondLevelBatchBuffer.OsResource   = batchBufferInUse->OsResource;
        secondLevelBatchBuffer.dwOffset     = m_batchBufferForPakSlicesStartOffset;
        secondLevelBatchBuffer.bSecondLevel = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START(&cmdBuffer, &secondLevelBatchBuffer)));
    }

    // Insert Batch Buffer Start command to send HCP_PAK_OBJ data for LCUs in this slice
    MHW_BATCH_BUFFER secondLevelBatchBuffer;
    MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(MHW_BATCH_BUFFER));
    secondLevelBatchBuffer.OsResource   = *m_res422MbCodeBuffer;
    secondLevelBatchBuffer.dwOffset     = slcData[currSlcIdx].CmdOffset;
    secondLevelBatchBuffer.bSecondLevel = true;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START(&cmdBuffer, &secondLevelBatchBuffer)));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPkt422::AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    auto &forceWakeupParams                     = m_miItf->MHW_GETPAR_F(MI_FORCE_WAKEUP)();
    forceWakeupParams                           = {};
    forceWakeupParams.bMFXPowerWellControl      = false;
    forceWakeupParams.bMFXPowerWellControlMask  = true;
    forceWakeupParams.bHEVCPowerWellControl     = true;
    forceWakeupParams.bHEVCPowerWellControlMask = true;

    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FORCE_WAKEUP)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPkt422::AddHcpPipeModeSelect(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    auto &vdControlStateParams          = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
    vdControlStateParams                = {};
    vdControlStateParams.initialization = true;

    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(&cmdBuffer));

    // for Gen11+, we need to add MFX wait for both KIN and VRT before and after HCP Pipemode select...
    auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = true;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer));

    SETPAR_AND_ADDCMD(HCP_PIPE_MODE_SELECT, m_hcpItf, &cmdBuffer);

    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = true;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, HevcVdencPkt422)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(HevcVdencPkt::MHW_SETPAR_F(HCP_PIPE_MODE_SELECT)(params));

    params.bVdencEnabled              = false;
    params.bBRCEnabled                = false;
    params.bAdvancedRateControlEnable = false;

    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_hevcSeqParams);
    params.bStreamOutEnabled = m_basicFeature->m_hevcSeqParams->RateControlMethod != RATECONTROL_CQP;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_SLICE_STATE, HevcVdencPkt422)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    uint32_t                          currSlcIdx    = m_basicFeature->m_curNumSlices;
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams = (CODEC_HEVC_ENCODE_PICTURE_PARAMS *)m_hevcPicParams;

    params.cabaczerowordinsertionenable = 1;
    params.intrareffetchdisable         = false;
    params.tailInsertionEnable          = (hevcPicParams->bLastPicInSeq || hevcPicParams->bLastPicInStream) && ((currSlcIdx == m_basicFeature->m_numSlices - 1));
    params.roundintra                   = m_basicFeature->m_roundingIntra;
    params.roundinter                   = m_basicFeature->m_roundingInter;
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_IND_OBJ_BASE_ADDR_STATE, HevcVdencPkt422)
{
    ENCODE_FUNC_CALL();

    params.presMvObjectBuffer      = m_res422MbCodeBuffer;
    params.dwMvObjectOffset        = m_422mvOffset;
    params.dwMvObjectSize          = m_422mbCodeSize - m_422mvOffset;
    params.presPakBaseObjectBuffer = &m_basicFeature->m_resBitstreamBuffer;
    params.dwPakBaseObjectSize     = m_basicFeature->m_bitstreamSize;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIC_STATE, HevcVdencPkt422)
{
    params.sseEnable                    = false;
    params.rhodomainRateControlEnable   = false;
    params.fractionalQpAdjustmentEnable = false;

    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_hevcSeqParams);
    if (m_basicFeature->m_hevcSeqParams->RateControlMethod == RATECONTROL_CBR)
    {
        auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);

        auto vdenc2ndLevelBatchBuffer = brcFeature->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);
        ENCODE_CHK_NULL_RETURN(vdenc2ndLevelBatchBuffer);

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.ReadOnly = 1;
        uint32_t *data     = (uint32_t *)m_osInterface->pfnLockResource(m_osInterface, &(vdenc2ndLevelBatchBuffer->OsResource), &lockFlags);
        ENCODE_CHK_NULL_RETURN(data);

        uint32_t value           = *(data + m_basicFeature->m_picStateCmdStartInBytes / 4 + MINFRAMESIZE_OFFSET);
        params.minframesize      = (uint16_t)value;
        params.minframesizeunits = (uint8_t)(value >> 30);

        m_osInterface->pfnUnlockResource(m_osInterface, &(vdenc2ndLevelBatchBuffer->OsResource));
    }

    return MOS_STATUS_SUCCESS;
}


}  // namespace encode
