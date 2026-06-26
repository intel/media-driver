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
//! \file     encode_avc_vdenc_packet_xe3p_lpm.cpp
//! \brief    Defines the interface for avc encode vdenc packet of Xe3P_LPM
//!

#include "encode_avc_vdenc_packet_xe3p_lpm.h"
#include "encode_avc_vdenc_const_settings.h"
#include "encode_avc_aqm.h"
#include "encode_avc_brc.h"
#include "encode_avc_basic_feature_xe3p_lpm.h"
#include "media_avc_feature_defs.h"
#include "hal_oca_interface_next.h"
#include "mos_solo_generic.h"
#if (_DEBUG || _RELEASE_INTERNAL)
#include "encode_avc_vdenc_pipeline_xe3p_lpm_base.h"
#endif

namespace encode
{

MOS_STATUS AvcVdencPktXe3P_Lpm::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    auto *avcPipeline = dynamic_cast<AvcVdencPipelineXe3P_Lpm_Base *>(m_pipeline);
    BypassHwLegacy *bypassHW = avcPipeline ? avcPipeline->GetBypassHW() : nullptr;
    if (bypassHW)
    {
        m_bypassHwLegacyEnabled = true;
        // AVC codecSettings->chromaFormat = 0 means default 4:2:0 (not MONOCHROME).
        // Normalize to HCP_CHROMA_FORMAT_YUV420 (= 1) so cfg file "420" entries match.
        uint32_t avcChromaFmt = m_basicFeature->m_chromaFormat == 0 ? 1 : m_basicFeature->m_chromaFormat;
        // Use MB-aligned dimensions (m_frameWidth/Height) to match AVC decode, which
        // can only derive MB-aligned dimensions from pic_*_in_mbs_minus1. Config file
        // entries for AVC must use MB-aligned height (e.g. 1088 for 1080p, not 1080).
        bypassHW->SetPipelineCharacteristics(
            CODECHAL_AVC,
            avcChromaFmt,
            m_basicFeature->m_frameWidth,
            m_basicFeature->m_frameHeight,
            m_basicFeature->m_bitDepth,
            m_basicFeature->m_targetUsage);
    }
#endif
    return AvcVdencPkt::Submit(commandBuffer, packetPhase);
}

    MOS_STATUS AvcVdencPktXe3P_Lpm::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
    {
        ENCODE_FUNC_CALL();
        auto eStatus = MOS_STATUS_SUCCESS;
        eStatus      = AvcVdencPkt::Completed(mfxStatus, rcsStatus, statusReport);

        auto aqmFeature = dynamic_cast<AvcEncodeAqm *>(m_featureManager->GetFeature(AvcFeatureIDs::avcAqm));
        ENCODE_CHK_NULL_RETURN(aqmFeature);
        if (aqmFeature->IsEnabled())
        {
            EncodeStatusReportData *statusReportData = (EncodeStatusReportData *)statusReport;
            uint32_t                statBufIdx       = statusReportData->currOriginalPic.FrameIdx;

            if (m_basicFeature->m_picParam->QualityInfoSupportFlags.fields.enable_frame)
            {
                ENCODE_CHK_STATUS_RETURN(aqmFeature->ReportQualityInfoFrame(statBufIdx, *statusReportData));
            }

#if _MEDIA_RESERVED
#if USE_CODECHAL_DEBUG_TOOL
            ENCODE_CHK_STATUS_RETURN(aqmFeature->DumpVdaqmOutput(statBufIdx, *statusReportData));

            CodechalDebugInterface *debugInterface = m_pipeline->GetStatusReportDebugInterface();
            ENCODE_CHK_NULL_RETURN(debugInterface);

            if (debugInterface->DumpIsEnabled(CodechalDbgAttr::attrQualityReport))
            {
                std::ostringstream oss;
                oss.setf(std::ios::showbase | std::ios::uppercase);

                aqmFeature->DumpVdaqmFrameStatVerbose(statBufIdx, oss);

                // Dump per frame VDAQM frame statistic file
                const char *fileName = debugInterface->CreateFileName("EncodeFrame", "VDAQMFrameStat", CodechalDbgExtType::txt);

                std::ofstream ofs(fileName, std::ios::out);
                ofs << oss.str();
                ofs.close();                
            }
#endif
#endif
        }

        return eStatus;
    }

    MOS_STATUS AvcVdencPktXe3P_Lpm::AddPictureVdencCommands(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        AvcVdencPkt::AddPictureVdencCommands(cmdBuffer);

        auto aqmFeature = dynamic_cast<AvcEncodeAqm *>(m_featureManager->GetFeature(AvcFeatureIDs::avcAqm));
        ENCODE_CHK_NULL_RETURN(aqmFeature);
        if (aqmFeature->IsEnabled())
        {
            SETPAR_AND_ADDCMD(AQM_VD_CONTROL_STATE, m_aqmItf, &cmdBuffer);
            SETPAR_AND_ADDCMD(AQM_PIPE_BUF_ADDR_STATE, m_aqmItf, &cmdBuffer);
            SETPAR_AND_ADDCMD(AQM_PIC_STATE, m_aqmItf, &cmdBuffer);
            SETPAR_AND_ADDCMD(AQM_FRAME_START, m_aqmItf, &cmdBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPktXe3P_Lpm::SendSlice(PMOS_COMMAND_BUFFER cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        AvcVdencPkt::SendSlice(cmdBuffer);

        auto aqmFeature = dynamic_cast<AvcEncodeAqm *>(m_featureManager->GetFeature(AvcFeatureIDs::avcAqm));
        ENCODE_CHK_NULL_RETURN(aqmFeature);
        if (aqmFeature->IsEnabled())
        {
            SETPAR_AND_ADDCMD(AQM_SLICE_STATE, m_aqmItf, cmdBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VD_PIPELINE_FLUSH, AvcVdencPktXe3P_Lpm)
    {
        AvcVdencPkt::MHW_SETPAR_F(VD_PIPELINE_FLUSH)(params);

        auto aqmFeature = dynamic_cast<AvcEncodeAqm *>(m_featureManager->GetFeature(AvcFeatureIDs::avcAqm));
        ENCODE_CHK_NULL_RETURN(aqmFeature);
        if (aqmFeature->IsEnabled())
        {
            params.waitDoneVDAQM = m_lastSlice ? true : false;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPktXe3P_Lpm::EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

#if (_DEBUG || _RELEASE_INTERNAL)
        {
            auto *avcPipeline = dynamic_cast<AvcVdencPipelineXe3P_Lpm_Base *>(m_pipeline);
            BypassHwLegacy *bypassHW = avcPipeline ? avcPipeline->GetBypassHW() : nullptr;
            if (bypassHW)
            {
                ENCODE_CHK_STATUS_RETURN(bypassHW->StopPredicate(&cmdBuffer));
            }
        }
#endif

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

    MOS_STATUS AvcVdencPktXe3P_Lpm::PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_trackedBuf);
        ENCODE_CHK_NULL_RETURN(m_seqParam);

        cmdBuffer.Attributes.bFrequencyBoost = (m_seqParam->ScenarioInfo == ESCENARIO_REMOTEGAMING);

        ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(m_basicFeature->m_frameWidth, m_basicFeature->m_frameHeight, true));

        SetPerfTag(m_pipeline->IsFirstPass() ? CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE : CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE_SECOND_PASS,
            (uint16_t)m_basicFeature->m_mode,
            m_basicFeature->m_pictureCodingType);

        auto brcFeature = dynamic_cast<AvcEncodeBRC *>(m_featureManager->GetFeature(AvcFeatureIDs::avcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);

#if (_DEBUG || _RELEASE_INTERNAL)
        auto *avcPipeline = dynamic_cast<AvcVdencPipelineXe3P_Lpm_Base *>(m_pipeline);
        BypassHwLegacy *bypassHW = avcPipeline ? avcPipeline->GetBypassHW() : nullptr;
        bool isNullHwVecs = bypassHW && (avcPipeline->GetGpuNode() == MOS_GPU_NODE_VE);
#endif

        if (!m_pipeline->IsSingleTaskPhaseSupported() || (m_pipeline->IsFirstPass() && !brcFeature->IsVdencBrcEnabled()))
        {
            SETPAR_AND_ADDCMD(MI_FORCE_WAKEUP, m_miItf, &cmdBuffer);
            ENCODE_CHK_STATUS_RETURN(SendPrologCmds(cmdBuffer));
        }

        if (brcFeature->IsVdencBrcEnabled())
        {
#if _SW_BRC
            if (!brcFeature->m_swBrc)
            {
#endif
#if (_DEBUG || _RELEASE_INTERNAL)
            // In NullHW mode HuC never runs so HucStatus2=0 would always fire the early-exit.
            // Skip the check to allow profiler and codec commands to execute.
            if (!bypassHW)
#endif
            {
                m_pResource = brcFeature->GetHucStatus2Buffer();
                SETPAR_AND_ADDCMD(MI_CONDITIONAL_BATCH_BUFFER_END, m_miItf, &cmdBuffer);
            }
#if _SW_BRC
            }
#endif
        }

        if (m_pipeline->GetCurrentPass())
        {
            if ((Mos_Solo_Extension((MOS_CONTEXT_HANDLE)m_osInterface->pOsContext) || m_osInterface->bInlineCodecStatusUpdate)
                && brcFeature->IsVdencBrcEnabled())
            {
                ENCODE_CHK_STATUS_RETURN(MediaPacket::UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));
            }

#if (_DEBUG || _RELEASE_INTERNAL)
            if (bypassHW)
            {
                // NullHW: unconditionally end batch buffer to skip entire second pass.
                // In NullHW mode, HuC BRC never runs, so VdencBrcPakMmioBuffer is uninitialized.
                // Adding MI_BATCH_BUFFER_END here terminates second pass immediately,
                // preventing all subsequent commands (proxy, predicate zone, HW commands, MI_FLUSH) from executing.
                SETPAR_AND_ADDCMD(MI_BATCH_BUFFER_END, m_miItf, &cmdBuffer);
            }
            else
#endif
            {
                m_pResource =
                    m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
                SETPAR_AND_ADDCMD(MI_CONDITIONAL_BATCH_BUFFER_END, m_miItf, &cmdBuffer);
            }
        }

        if (m_pipeline->IsFirstPipe())
        {
            ENCODE_CHK_STATUS_RETURN(StartStatusReport(statusReportMfx, &cmdBuffer));
        }

#if (_DEBUG || _RELEASE_INTERNAL)
        if (bypassHW)
        {
            ENCODE_CHK_STATUS_RETURN(bypassHW->AddNullHwProxyCmd(&cmdBuffer, true));
            ENCODE_CHK_STATUS_RETURN(bypassHW->StartPredicate(&cmdBuffer));
        }
#endif

        SETPAR_AND_ADDCMD(VDENC_CONTROL_STATE, m_vdencItf, &cmdBuffer);

#if (_DEBUG || _RELEASE_INTERNAL)
        if (isNullHwVecs)
        {
            // Cannot call AddPictureMfxCommands on VE node: it issues MFX_WAIT(iStallVdboxPipeline=true),
            // which causes GPU device failure on VEBOX engine that has no VDBox pipeline.
            SETPAR_AND_ADDCMD(MFX_PIPE_MODE_SELECT, m_mfxItf, &cmdBuffer);
            ENCODE_CHK_STATUS_RETURN(AddAllCmds_MFX_SURFACE_STATE(&cmdBuffer));
            SETPAR_AND_ADDCMD(MFX_PIPE_BUF_ADDR_STATE, m_mfxItf, &cmdBuffer);
            SETPAR_AND_ADDCMD(MFX_IND_OBJ_BASE_ADDR_STATE, m_mfxItf, &cmdBuffer);
            SETPAR_AND_ADDCMD(MFX_BSP_BUF_BASE_ADDR_STATE, m_mfxItf, &cmdBuffer);
        }
        else
#endif
        {
            ENCODE_CHK_STATUS_RETURN(AddPictureMfxCommands(cmdBuffer));
        }

        ENCODE_CHK_STATUS_RETURN(AddPictureVdencCommands(cmdBuffer));

        PMHW_BATCH_BUFFER secondLevelBatchBufferUsed = nullptr;
        if (brcFeature->IsVdencBrcEnabled())
        {
            // BRC mode: HuC BRC Update writes its output to batchBufferForVdencImgStat (Region 6)
            secondLevelBatchBufferUsed = brcFeature->GetBatchBufferForVdencImgStat();
        }
        else
        {
            // CQP mode: use the SLBB output from HuC SLBB Update
            auto basicFeatureXe3pLpm = dynamic_cast<AvcBasicFeatureXe3P_Lpm *>(m_basicFeature);
            ENCODE_CHK_NULL_RETURN(basicFeatureXe3pLpm);
            secondLevelBatchBufferUsed = basicFeatureXe3pLpm->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);
        }
        ENCODE_CHK_NULL_RETURN(secondLevelBatchBufferUsed);
        secondLevelBatchBufferUsed->iCurrent = 0;
        secondLevelBatchBufferUsed->dwOffset = 0;

#if (_DEBUG || _RELEASE_INTERNAL)
        // MI_BATCH_BUFFER_START is an MI command — it is NOT predicated by MI_SET_PREDICATE.
        // It always executes even inside a StartPredicate(ENABLE_ALWAYS) region.
        // In NullHW/BRC mode HuC never ran, so the BRC img-state batch buffer is
        // uninitialized. Jumping to it causes a GPU hang (MFX_ERR_DEVICE_FAILED).
        // Skip the sub-level batch entirely in NullHW mode.
        if (!bypassHW)
#endif
        {
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(&cmdBuffer, secondLevelBatchBufferUsed));
            HalOcaInterfaceNext::OnSubLevelBBStart(
                cmdBuffer,
                m_osInterface->pOsContext,
                &secondLevelBatchBufferUsed->OsResource,
                secondLevelBatchBufferUsed->dwOffset,
                false,
                MOS_ALIGN_CEIL(m_hwInterface->m_vdencBrcImgStateBufferSize, CODECHAL_CACHELINE_SIZE));

            CODECHAL_DEBUG_TOOL
            (
                CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
                ENCODE_CHK_STATUS_RETURN(debugInterface->Dump2ndLvlBatch(
                    secondLevelBatchBufferUsed,
                    CODECHAL_MEDIA_STATE_ENC_NORMAL,
                    nullptr));
            )
        }

        ENCODE_CHK_STATUS_RETURN(AddAllCmds_MFX_QM_STATE(&cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(AddAllCmds_MFX_FQM_STATE(&cmdBuffer));

        if (m_basicFeature->m_pictureCodingType == B_TYPE)
        {
            SETPAR_AND_ADDCMD(MFX_AVC_DIRECTMODE_STATE, m_mfxItf, &cmdBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }
}

