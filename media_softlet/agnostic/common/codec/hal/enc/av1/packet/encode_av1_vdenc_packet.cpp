/*
* Copyright (c) 2019-2023, Intel Corporation
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
//! \file     encode_av1_vdenc_packet.cpp
//! \brief    Defines the interface for av1 encode vdenc packet
//!
#include <numeric>
#include "encode_av1_vdenc_packet.h"
#include "encode_status_report_defs.h"
#include "codec_def_common_av1.h"
#include "media_perf_profiler.h"
#include "hal_oca_interface_next.h"
#include "mos_solo_generic.h"

namespace encode{
    Av1VdencPkt::Av1VdencPkt(MediaPipeline* pipeline, MediaTask* task, CodechalHwInterfaceNext* hwInterface) :
        CmdPacket(task),
        m_pipeline(dynamic_cast<Av1VdencPipeline*>(pipeline)),
        m_hwInterface(hwInterface)
    {
        ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_pipeline);

        m_osInterface = hwInterface->GetOsInterface();
        m_statusReport = m_pipeline->GetStatusReportInstance();
        m_featureManager = m_pipeline->GetPacketLevelFeatureManager(Av1Pipeline::Av1VdencPacket);

        m_vdencItf = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(m_hwInterface->GetVdencInterfaceNext());
        m_avpItf   = std::static_pointer_cast<mhw::vdbox::avp::Itf>(m_hwInterface->GetAvpInterfaceNext());
        m_miItf    = std::static_pointer_cast<mhw::mi::Itf> (m_hwInterface->GetMiInterfaceNext());
        
        if (m_vdencItf)
        {
            mhw::vdbox::vdenc::RowStorePar par = {};

            par.mode = mhw::vdbox::vdenc::RowStorePar::AV1;

            ENCODE_CHK_STATUS_NO_STATUS_RETURN(m_vdencItf->SetRowstoreCachingOffsets(par));
        }
        if(m_osInterface)
        {
            m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
        }
        if (!m_userSettingPtr)
        {
            ENCODE_NORMALMESSAGE("Initialize m_userSettingPtr instance failed!");
        }
    }

    MOS_STATUS Av1VdencPkt::StartStatusReport(
        uint32_t            srType,
        MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(MediaPacket::StartStatusReportNext(srType, cmdBuffer));

        MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectStartCmd(
            (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::AddPictureVdencCommands(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        SETPAR_AND_ADDCMD(VDENC_CONTROL_STATE, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_PIPE_MODE_SELECT, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_SRC_SURFACE_STATE, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_REF_SURFACE_STATE, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_DS_REF_SURFACE_STATE, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_PIPE_BUF_ADDR_STATE, m_vdencItf, &cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(ReadAvpStatus(m_vdboxIndex, m_statusReport, *cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(MediaPacket::EndStatusReportNext(srType, cmdBuffer));

        MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd(
            (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::ReadAvpStatus(MHW_VDBOX_NODE_IND vdboxIndex, MediaStatusReport *statusReport, MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        CODEC_HW_FUNCTION_ENTER;

        ENCODE_CHK_NULL_RETURN(statusReport);
        ENCODE_CHK_NULL_RETURN(m_hwInterface);

        MOS_RESOURCE *osResource = nullptr;
        uint32_t      offset     = 0;

        EncodeStatusReadParams params;
        MOS_ZeroMemory(&params, sizeof(params));

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportMfxBitstreamByteCountPerFrame, osResource, offset));
        params.resBitstreamByteCountPerFrame    = osResource;
        params.bitstreamByteCountPerFrameOffset = offset;

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportQPStatusCount, osResource, offset));
        params.resQpStatusCount    = osResource;
        params.qpStatusCountOffset = offset;

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportImageStatusMask, osResource, offset));
        params.resImageStatusMask    = osResource;
        params.imageStatusMaskOffset = offset;

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportImageStatusCtrl, osResource, offset));
        params.resImageStatusCtrl    = osResource;
        params.imageStatusCtrlOffset = offset;

        CODEC_HW_CHK_COND_RETURN((vdboxIndex > m_hwInterface->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");

        auto &flushDwParams = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams       = {};
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

        ENCODE_CHK_NULL_RETURN(m_avpItf);
        auto mmioRegisters = m_avpItf->GetMmioRegisters(vdboxIndex);

        auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = params.resBitstreamByteCountPerFrame;
        miStoreRegMemParams.dwOffset        = params.bitstreamByteCountPerFrameOffset;
        miStoreRegMemParams.dwRegister      = mmioRegisters->avpAv1BitstreamByteCountTileRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));

        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = params.resQpStatusCount;
        miStoreRegMemParams.dwOffset        = params.qpStatusCountOffset;
        miStoreRegMemParams.dwRegister      = mmioRegisters->avpAv1QpStatusCountRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));

        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = params.resImageStatusMask;
        miStoreRegMemParams.dwOffset        = params.imageStatusMaskOffset;
        miStoreRegMemParams.dwRegister      = mmioRegisters->avpAv1ImageStatusMaskRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));

        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = params.resImageStatusCtrl;
        miStoreRegMemParams.dwOffset        = params.imageStatusCtrlOffset;
        miStoreRegMemParams.dwRegister      = mmioRegisters->avpAv1ImageStatusControlRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));

        flushDwParams = {};
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::Init()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_statusReport);
        ENCODE_CHK_STATUS_RETURN(CmdPacket::Init());

        m_basicFeature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

#ifdef _MMC_SUPPORTED
        m_mmcState = m_pipeline->GetMmcState();
        ENCODE_CHK_NULL_RETURN(m_mmcState);
        m_basicFeature->m_mmcState = m_mmcState;
#endif
        m_allocator = m_pipeline->GetEncodeAllocator();
        ENCODE_CHK_STATUS_RETURN(AllocateResources());

        ENCODE_CHK_STATUS_RETURN(m_statusReport->RegistObserver(this));

        CalculateVdencCommandsSize();
        CalculateAvpCommandsSize();

        m_usePatchList = m_osInterface->bUsesPatchList;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(commandBuffer);
        MOS_COMMAND_BUFFER &cmdBuffer = *commandBuffer;
        ENCODE_CHK_STATUS_RETURN(Mos_Solo_PreProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));

        // Ensure the input is ready to be read.
        // Currently, mos RegisterResource has sync limitation for Raw resource.
        // Temporaly, call Resource Wait to do the sync explicitly.
        // TODO, Refine it when MOS refactor ready.
        MOS_SYNC_PARAMS syncParams;
        syncParams                  = g_cInitSyncParams;
        syncParams.GpuContext       = m_osInterface->pfnGetGpuContext(m_osInterface);
        syncParams.presSyncResource = &m_basicFeature->m_rawSurface.OsResource;
        syncParams.bReadOnly        = true;
        ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));
        m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

        // Set flag to boost GPU frequency for low latency in remote gaming scenario
        cmdBuffer.Attributes.bFrequencyBoost = (m_av1SeqParams->ScenarioInfo == ESCENARIO_REMOTEGAMING);

        ENCODE_CHK_STATUS_RETURN(RegisterPostCdef());

        ENCODE_CHK_STATUS_RETURN(PatchPictureLevelCommands(packetPhase, cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(PatchTileLevelCommands(cmdBuffer, packetPhase));

        ENCODE_CHK_STATUS_RETURN(PrepareHWMetaData(&cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(Mos_Solo_PostProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));
#if MHW_HWCMDPARSER_ENABLED
        auto instance = mhw::HwcmdParser::GetInstance();
        if (instance)
        {
            instance->ParseCmdBuf(IGFX_UNKNOWN, cmdBuffer.pCmdBase, cmdBuffer.iOffset / sizeof(uint32_t));
        }
#endif
#if USE_CODECHAL_DEBUG_TOOL
        ENCODE_CHK_STATUS_RETURN(DumpStatistics());
        ENCODE_CHK_STATUS_RETURN(Av1VdencPkt::DumpInput());
#endif  // USE_CODECHAL_DEBUG_TOOL
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::AddCondBBEndFor2ndPass(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        if (m_pipeline->IsFirstPass() || m_pipeline->GetPassNum() == 1)
        {
            return MOS_STATUS_SUCCESS;
        }

        auto &miConditionalBatchBufferEndParams = m_miItf->MHW_GETPAR_F(MI_CONDITIONAL_BATCH_BUFFER_END)();
        miConditionalBatchBufferEndParams       = {};

        // VDENC uses HuC FW generated semaphore for conditional 2nd pass
        miConditionalBatchBufferEndParams.presSemaphoreBuffer =
            m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);

        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_CONDITIONAL_BATCH_BUFFER_END)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::Construct3rdLevelBatch()
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        //To be added. When BRC is enabled, some of the commands
        //will be added into 3rd level batch

        return eStatus;
    }

    MOS_STATUS Av1VdencPkt::UpdateUserFeatureKey(PMOS_SURFACE surface)
    {
        if (m_userFeatureUpdated_post_cdef)
        {
            return MOS_STATUS_SUCCESS;
        }
        m_userFeatureUpdated_post_cdef = true;

        ReportUserSetting(m_userSettingPtr,
            "AV1 Post CDEF Recon Compressible",
            surface->bCompressible,
            MediaUserSetting::Group::Sequence);
        ReportUserSetting(m_userSettingPtr,
            "AV1 Post CDEF Recon Compress Mode",
            surface->MmcState,
            MediaUserSetting::Group::Sequence);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER  &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(m_basicFeature->m_frameWidth, m_basicFeature->m_frameHeight, true));

        SetPerfTag();

        bool firstTaskInPhase = packetPhase & firstPacket;
        if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
        {
            ENCODE_CHK_STATUS_RETURN(AddForceWakeup(cmdBuffer));

            // Send command buffer header at the beginning (OS dependent)
            ENCODE_CHK_STATUS_RETURN(SendPrologCmds(cmdBuffer));
        }

        if (m_pipeline->IsDualEncEnabled())
        {
            auto scalability = m_pipeline->GetMediaScalability();
            ENCODE_CHK_NULL_RETURN(scalability);
            ENCODE_CHK_STATUS_RETURN(scalability->SyncPipe(syncOtherPipesForOne, 0, &cmdBuffer));
        }

        ENCODE_CHK_STATUS_RETURN(AddCondBBEndFor2ndPass(cmdBuffer));

        if(m_pipeline->IsDualEncEnabled() && m_pipeline->IsFirstPipe())
        {
            PMOS_RESOURCE bsSizeBuf = m_basicFeature->m_recycleBuf->GetBuffer(PakInfo, 0);
            ENCODE_CHK_NULL_RETURN(bsSizeBuf);
            // clear bitstream size buffer at first tile
            auto &miStoreDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
            miStoreDataParams                  = {};
            miStoreDataParams.pOsResource      = bsSizeBuf;
            miStoreDataParams.dwResourceOffset = 0;
            miStoreDataParams.dwValue          = 0;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(&cmdBuffer));
        }

        if (m_pipeline->IsFirstPipe())
        {
            ENCODE_CHK_STATUS_RETURN(StartStatusReport(statusReportMfx, &cmdBuffer));
        }
        else{
            // add perf record for other pipes - first pipe perf record within StartStatusReport
            MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
            ENCODE_CHK_NULL_RETURN(perfProfiler);
            ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectStartCmd(
                (void *)m_pipeline, m_osInterface, m_miItf, &cmdBuffer));
        }

        ENCODE_CHK_STATUS_RETURN(AddPictureVdencCommands(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    void Av1VdencPkt::SetPerfTag()
    {
        ENCODE_FUNC_CALL();

        uint16_t callType = m_pipeline->IsFirstPass() ? CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE :
            CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE_SECOND_PASS;
        uint16_t picType  = (m_basicFeature->m_pictureCodingType == I_TYPE) ? 1 :
            (m_basicFeature->m_ref.IsLowDelay() ? (m_basicFeature->m_ref.IsPFrame() ? 2 : 0) : 3);

        PerfTagSetting perfTag;
        perfTag.Value             = 0;
        perfTag.Mode              = (uint16_t)m_basicFeature->m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
        perfTag.CallType          = callType;
        perfTag.PictureCodingType = picType;
        m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);
        m_osInterface->pfnIncPerfBufferID(m_osInterface);
    }

    MOS_STATUS Av1VdencPkt::PatchTileLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        uint16_t numTileColumns = 1;
        uint16_t numTileRows    = 1;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

        ENCODE_CHK_NULL_RETURN(m_pipeline);
        if (!m_pipeline->IsDualEncEnabled())
        {
            for (uint32_t tileRow = 0; tileRow < numTileRows; tileRow++)
            {
                for (uint32_t tileCol = 0; tileCol < numTileColumns; tileCol++)
                {
                    ENCODE_CHK_STATUS_RETURN(AddOneTileCommands(
                        cmdBuffer,
                        tileRow,
                        tileCol));
                }
            }
        }
        else
        {
            if (numTileRows != 1)  // dual encode only support column based workload submission
            {
                ENCODE_ASSERTMESSAGE("dual encode cannot support multi rows submission yet.");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            uint8_t dummyIdx = 0;
            RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetDummyIdx, dummyIdx);
            if (m_pipeline->GetCurrentPipe() == 0)
            {
                for (auto i = 0; i < dummyIdx; i++)
                {
                    ENCODE_CHK_STATUS_RETURN(AddOneTileCommands(
                        cmdBuffer,
                        0,
                        i));
                }
                ENCODE_CHK_STATUS_RETURN(AddOneTileCommands(
                    cmdBuffer,
                    0,
                    dummyIdx,
                    1));
            }
            else
            {
                for (auto i = dummyIdx; i < numTileColumns; i++)
                {
                    ENCODE_CHK_STATUS_RETURN(AddOneTileCommands(
                        cmdBuffer,
                        0,
                        i));
                }
            }
        }

        m_basicFeature->m_flushCmd = Av1BasicFeature::waitAvp;
        SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, &cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(cmdBuffer));

        // Wait all pipe cmds done for the packet
        auto scalability = m_pipeline->GetMediaScalability();
        ENCODE_CHK_NULL_RETURN(scalability);
        ENCODE_CHK_STATUS_RETURN(scalability->SyncPipe(syncOnePipeWaitOthers, 0, &cmdBuffer));

        if (m_pipeline->IsFirstPipe())
        {
            if (m_pipeline->IsDualEncEnabled())
            {
                for (auto i = 0; i < m_pipeline->GetPipeNum(); ++i)
                {
                    ENCODE_CHK_STATUS_RETURN(scalability->ResetSemaphore(syncOnePipeWaitOthers, i, &cmdBuffer));
                }
            }
            ENCODE_CHK_STATUS_RETURN(EndStatusReport(statusReportMfx, &cmdBuffer));
        }
        else {
            // add perf record for other pipes - first pipe perf record within EndStatusReport
            MediaPerfProfiler* perfProfiler = MediaPerfProfiler::Instance();
            ENCODE_CHK_NULL_RETURN(perfProfiler);
            ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd(
                (void*)m_pipeline, m_osInterface, m_miItf, &cmdBuffer));
        }

        auto brcFeature = dynamic_cast<Av1Brc *>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);

        if (Mos_Solo_Extension((MOS_CONTEXT_HANDLE)m_osInterface->pOsContext))
        {
            ENCODE_CHK_STATUS_RETURN(MediaPacket::UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));
        }
        else if (brcFeature->IsBRCEnabled() && m_osInterface->bInlineCodecStatusUpdate)
        {
            ENCODE_CHK_STATUS_RETURN(UpdateStatusReport(statusReportGlobalCount, &cmdBuffer));
        }
        else if (m_pipeline->IsLastPass() && m_pipeline->IsFirstPipe())
        {
            ENCODE_CHK_STATUS_RETURN(MediaPacket::UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));
        }

        if (m_pipeline->IsDualEncEnabled())
        {
            SETPAR_AND_ADDCMD(VDENC_CONTROL_STATE, m_vdencItf, &cmdBuffer);
        }

        CODECHAL_DEBUG_TOOL(
            if (m_mmcState) {
                m_mmcState->UpdateUserFeatureKey(&(m_basicFeature->m_reconSurface));
            })

        if (m_basicFeature->m_postCdefReconSurfaceFlag)
        {
            PCODEC_REF_LIST currRefList  = m_basicFeature->m_ref.GetCurrRefList();
            ENCODE_CHK_NULL_RETURN(currRefList);
            MOS_SURFACE *postCdefSurface = m_basicFeature->m_trackedBuf->GetSurface(
                BufferType::postCdefReconSurface, currRefList->ucScalingIdx);
            ENCODE_CHK_NULL_RETURN(postCdefSurface);
            CODECHAL_DEBUG_TOOL(
                if (m_mmcState) { 
                    UpdateUserFeatureKey(postCdefSurface); 
            })
        }

        UpdateParameters();

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        auto &forceWakeupParams                     = m_miItf->MHW_GETPAR_F(MI_FORCE_WAKEUP)();
        forceWakeupParams                           = {};
        forceWakeupParams.bMFXPowerWellControl      = true;
        forceWakeupParams.bMFXPowerWellControlMask  = true;
        forceWakeupParams.bHEVCPowerWellControl     = true;
        forceWakeupParams.bHEVCPowerWellControlMask = true;

        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FORCE_WAKEUP)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::SendPrologCmds(
        MOS_COMMAND_BUFFER &cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

#ifdef _MMC_SUPPORTED
        ENCODE_CHK_NULL_RETURN(m_mmcState);
        ENCODE_CHK_STATUS_RETURN(m_mmcState->SendPrologCmd(&cmdBuffer, false));
#endif

        MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
        MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
        genericPrologParams.pOsInterface  = m_osInterface;
        genericPrologParams.pvMiInterface = nullptr;
        genericPrologParams.bMmcEnabled   = m_mmcState ? m_mmcState->IsMmcEnabled() : false;
        ENCODE_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmdNext(&cmdBuffer, &genericPrologParams, m_miItf));

        return eStatus;
    }

    void Av1VdencPkt::UpdateParameters()
    {
        ENCODE_FUNC_CALL();

        m_prevFrameType  = m_av1PicParams->PicFlags.fields.frame_type;
        if(m_pipeline->IsLastPass() && m_pipeline->IsFirstPipe())
        {
            m_basicFeature->m_encodedFrameNum++;
        }

        if (!m_pipeline->IsSingleTaskPhaseSupported())
        {
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }
    }

    MOS_STATUS Av1VdencPkt::SetPipeBufAddr(
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams,
        MHW_VDBOX_SURFACE_PARAMS &      srcSurfaceParams,
        MHW_VDBOX_SURFACE_PARAMS &      reconSurfaceParams)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(srcSurfaceParams.psSurface);
        ENCODE_CHK_NULL_RETURN(reconSurfaceParams.psSurface);

#ifdef _MMC_SUPPORTED
        ENCODE_CHK_NULL_RETURN(m_mmcState);
        if (m_mmcState->IsMmcEnabled())
        {
            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(&m_basicFeature->m_reconSurface, &pipeBufAddrParams->PreDeblockSurfMmcState));
            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(&m_basicFeature->m_rawSurface, &pipeBufAddrParams->RawSurfMmcState));
            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcFormat(srcSurfaceParams.psSurface, &srcSurfaceParams.dwCompressionFormat));
            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcFormat(reconSurfaceParams.psSurface, &reconSurfaceParams.dwCompressionFormat));
        }
        else
        {
            pipeBufAddrParams->PreDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
            pipeBufAddrParams->RawSurfMmcState        = MOS_MEMCOMP_DISABLED;
        }
#endif

        CODECHAL_DEBUG_TOOL(
            m_basicFeature->m_reconSurface.MmcState = pipeBufAddrParams->PreDeblockSurfMmcState;)

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::SetSurfaceState(
        PMHW_VDBOX_SURFACE_PARAMS surfaceStateParams)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_CHK_NULL_RETURN(surfaceStateParams);
        ENCODE_CHK_NULL_RETURN(surfaceStateParams->psSurface);

        ENCODE_FUNC_CALL();

#ifdef _MMC_SUPPORTED
        ENCODE_CHK_NULL_RETURN(m_mmcState);
        if (m_mmcState->IsMmcEnabled())
        {
            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(surfaceStateParams->psSurface, &surfaceStateParams->mmcState));
            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcFormat(surfaceStateParams->psSurface, &surfaceStateParams->dwCompressionFormat));
        }
        else
        {
            surfaceStateParams->mmcState = MOS_MEMCOMP_DISABLED;
        }
#endif
        return eStatus;
    }

    MOS_STATUS Av1VdencPkt::Prepare()
    {
        ENCODE_FUNC_CALL();

        Av1Pipeline *pipeline = dynamic_cast<Av1Pipeline *>(m_pipeline);
        ENCODE_CHK_NULL_RETURN(pipeline);

        m_av1SeqParams  = ((Av1BasicFeature *)m_basicFeature)->m_av1SeqParams;
        m_av1PicParams  = ((Av1BasicFeature *)m_basicFeature)->m_av1PicParams;
        m_nalUnitParams = ((Av1BasicFeature *)m_basicFeature)->m_nalUnitParams;

        SetRowstoreCachingOffsets();

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::SetRowstoreCachingOffsets()
    {
        // Get row store cache offset as all the needed information is got here
        if (m_avpItf->IsRowStoreCachingSupported())
        {
            MHW_VDBOX_ROWSTORE_PARAMS rowStoreParams;

            rowStoreParams.Mode             = codechalEncodeModeAv1;
            rowStoreParams.dwPicWidth       = MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, av1MinBlockWidth);
            rowStoreParams.ucChromaFormat   = m_basicFeature->m_outputChromaFormat;
            rowStoreParams.ucBitDepthMinus8 = m_basicFeature->m_is10Bit ? 2 : 0;

            ENCODE_CHK_STATUS_RETURN(m_hwInterface->SetRowstoreCachingOffsets(&rowStoreParams));
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(mfxStatus);
        ENCODE_CHK_NULL_RETURN(statusReport);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        EncodeStatusMfx        *encodeStatusMfx  = (EncodeStatusMfx *)mfxStatus;
        EncodeStatusReportData *statusReportData = (EncodeStatusReportData *)statusReport;

        uint32_t statBufIdx     = statusReportData->currOriginalPic.FrameIdx;
        const EncodeReportTileData *tileReportData = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetReportTileData, statBufIdx, tileReportData);
        ENCODE_CHK_NULL_RETURN(tileReportData);

        MOS_RESOURCE *tileRecordBuffer = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileRecordBuffer, statBufIdx, tileRecordBuffer);
        ENCODE_CHK_NULL_RETURN(tileRecordBuffer);

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        PakHwTileSizeRecord *tileRecord =
            (PakHwTileSizeRecord *)m_allocator->Lock(tileRecordBuffer, &lockFlags);
        ENCODE_CHK_NULL_RETURN(tileRecord);

        statusReportData->bitstreamSize = 0;
        for (uint32_t i = 0; i < statusReportData->numberTilesInFrame; i++)
        {
            if (tileRecord[i].Length == 0)
            {
                statusReportData->codecStatus = CODECHAL_STATUS_INCOMPLETE;
                return MOS_STATUS_SUCCESS;
            }

            statusReportData->bitstreamSize += tileRecord[i].Length;
        }

#if (_DEBUG || _RELEASE_INTERNAL)
        if (m_basicFeature->m_enableSWStitching)
        {
            PerformSwStitch(tileReportData, tileRecord, statusReportData);
        }
#endif
        if (tileRecord)
        {
            m_allocator->UnLock(tileRecordBuffer);
        }

        statusReportData->numberPasses = (uint8_t)encodeStatusMfx->imageStatusCtrl.avpTotalNumPass + 1;  //initial pass is considered to be 0,hence +1 to report;
        ENCODE_VERBOSEMESSAGE("statusReportData->numberPasses: %d\n", statusReportData->numberPasses);

        uint32_t log2MaxSbSize   = av1MiSizeLog2 + av1MinMibSizeLog2;
        uint32_t frameWidthInSb  = MOS_ALIGN_CEIL(statusReportData->frameWidth, (1 << log2MaxSbSize)) >> log2MaxSbSize;
        uint32_t frameHeightInSb = MOS_ALIGN_CEIL(statusReportData->frameHeight, (1 << log2MaxSbSize)) >> log2MaxSbSize;
        if (frameWidthInSb != 0 && frameHeightInSb != 0)
        {
            ENCODE_CHK_NULL_RETURN(m_basicFeature->m_av1SeqParams);
            statusReportData->qpY = (uint8_t)(((uint32_t)encodeStatusMfx->qpStatusCount.avpCumulativeQP) / (frameWidthInSb * frameHeightInSb));
            ENCODE_VERBOSEMESSAGE("statusReportData->qpY: %d\n", statusReportData->qpY);
        }

        CODECHAL_DEBUG_TOOL(
            ENCODE_CHK_STATUS_RETURN(DumpResources(encodeStatusMfx, statusReportData)););

        m_basicFeature->Reset((CODEC_REF_LIST *)statusReportData->currRefList);

        return MOS_STATUS_SUCCESS;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_STATUS Av1VdencPkt::PerformSwStitch(
        const EncodeReportTileData *tileReportData,
        PakHwTileSizeRecord *       tileRecord,
        EncodeStatusReportData *    statusReportData)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(tileReportData);
        ENCODE_CHK_NULL_RETURN(tileRecord);

        uint8_t *tempBsBuffer = nullptr, *bufPtr = nullptr;
        tempBsBuffer = bufPtr = (uint8_t *)MOS_AllocAndZeroMemory(statusReportData->bitstreamSize);
        ENCODE_CHK_NULL_RETURN(tempBsBuffer);

        PCODEC_REF_LIST currRefList = (PCODEC_REF_LIST)statusReportData->currRefList;

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.ReadOnly = 1;
        uint8_t *bitstream = (uint8_t *)m_allocator->Lock(
            &currRefList->resBitstreamBuffer,
            &lockFlags);
        if (bitstream == nullptr)
        {
            MOS_FreeMemory(tempBsBuffer);
            ENCODE_CHK_NULL_RETURN(nullptr);
        }

        for (uint32_t i = 0; i < statusReportData->numberTilesInFrame; i++)
        {
            uint32_t offset = MOS_ALIGN_CEIL(tileReportData[i].bitstreamByteOffset * CODECHAL_CACHELINE_SIZE, MOS_PAGE_SIZE);
            uint32_t len    = tileRecord[i].Length;

            MOS_SecureMemcpy(bufPtr, len, &bitstream[offset], len);
            bufPtr += len;
        }

        MOS_SecureMemcpy(bitstream, statusReportData->bitstreamSize, tempBsBuffer, statusReportData->bitstreamSize);
        MOS_ZeroMemory(&bitstream[statusReportData->bitstreamSize], m_basicFeature->m_bitstreamSize - statusReportData->bitstreamSize);

        if (bitstream)
        {
            m_allocator->UnLock(&currRefList->resBitstreamBuffer);
        }

        MOS_FreeMemory(tempBsBuffer);

        return MOS_STATUS_SUCCESS;
    }
#endif

    MOS_STATUS Av1VdencPkt::Destroy()
    {
        ENCODE_FUNC_CALL();

        m_statusReport->UnregistObserver(this);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::AllocateResources()
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_allocator);
        ENCODE_CHK_NULL_RETURN(m_pipeline);
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type               = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType           = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format             = Format_Buffer;
        allocParamsForBufferLinear.Flags.bNotLockable = true;

        uint32_t maxTileNumber              = CODECHAL_GET_WIDTH_IN_BLOCKS(m_basicFeature->m_frameWidth, av1MinTileWidth) *
                                              CODECHAL_GET_HEIGHT_IN_BLOCKS(m_basicFeature->m_frameHeight, av1MinTileHeight);

        allocParamsForBufferLinear.dwBytes  = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, av1SuperBlockWidth) * MHW_CACHELINE_SIZE * 2 * 2;
        allocParamsForBufferLinear.pBufName = "vdencIntraRowStoreScratch";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
        m_vdencIntraRowStoreScratch         = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
        ENCODE_CHK_NULL_RETURN(m_vdencIntraRowStoreScratch);

        allocParamsForBufferLinear.Flags.bNotLockable = !(m_basicFeature->m_lockableResource);
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_basicFeature->m_vdencBrcStatsBufferSize * maxTileNumber, MHW_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDEncStatsBuffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
        m_resVDEncStatsBuffer               = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
        ENCODE_CHK_NULL_RETURN(m_resVDEncStatsBuffer);

        if (m_osInterface->bInlineCodecStatusUpdate)
        {
            MOS_LOCK_PARAMS lockFlagsWriteOnly;
            MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
            lockFlagsWriteOnly.WriteOnly = 1;

            m_atomicScratchBuf.size = MOS_ALIGN_CEIL(sizeof(AtomicScratchBuffer), sizeof(uint64_t));
            allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format   = Format_Buffer;

            uint32_t size        = MHW_CACHELINE_SIZE * 4 * 2;  //  each set of scratch is 4 cacheline size, and allocate 2 set.
            allocParamsForBufferLinear.dwBytes      = size;
            allocParamsForBufferLinear.pBufName     = "atomic sratch buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;

            m_atomicScratchBuf.resAtomicScratchBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, false);

            ENCODE_CHK_NULL_RETURN(m_atomicScratchBuf.resAtomicScratchBuffer);

            uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(
                m_osInterface,
                m_atomicScratchBuf.resAtomicScratchBuffer,
                &lockFlagsWriteOnly);

            ENCODE_CHK_NULL_RETURN(data);

            MOS_ZeroMemory(data, size);
            m_atomicScratchBuf.pData             = (uint32_t *)data;
            m_atomicScratchBuf.size              = size;
            m_atomicScratchBuf.zeroValueOffset   = 0;
            m_atomicScratchBuf.operand1Offset    = MHW_CACHELINE_SIZE;
            m_atomicScratchBuf.operand2Offset    = MHW_CACHELINE_SIZE * 2;
            m_atomicScratchBuf.operand3Offset    = MHW_CACHELINE_SIZE * 3;
            m_atomicScratchBuf.encodeUpdateIndex = 0;
            m_atomicScratchBuf.tearDownIndex     = 1;
            m_atomicScratchBuf.operandSetSize    = MHW_CACHELINE_SIZE * 4;

            ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(m_osInterface, m_atomicScratchBuf.resAtomicScratchBuffer));
        }

        mhw::vdbox::avp::AvpBufferSizePar avpBufSizeParam;
        memset(&avpBufSizeParam, 0, sizeof(avpBufSizeParam));
        avpBufSizeParam.bitDepthIdc      = (m_basicFeature->m_bitDepth - 8) >> 1;
        avpBufSizeParam.height           = CODECHAL_GET_HEIGHT_IN_BLOCKS(m_basicFeature->m_frameHeight, av1SuperBlockHeight);
        avpBufSizeParam.width            = CODECHAL_GET_WIDTH_IN_BLOCKS(m_basicFeature->m_frameWidth, av1SuperBlockWidth);
        avpBufSizeParam.tileWidth        = CODECHAL_GET_HEIGHT_IN_BLOCKS(av1MaxTileWidth, av1SuperBlockWidth);
        avpBufSizeParam.isSb128x128      = 0;
        avpBufSizeParam.curFrameTileNum  = av1MaxTileNum;
        avpBufSizeParam.numTileCol       = av1MaxTileColumn;
        avpBufSizeParam.numOfActivePipes = 1;
        avpBufSizeParam.chromaFormat     = m_basicFeature->m_chromaFormat;

        MOS_ALLOC_GFXRES_PARAMS allocParams;
        MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParams.Type               = MOS_GFXRES_BUFFER;
        allocParams.TileType           = MOS_TILE_LINEAR;
        allocParams.Format             = Format_Buffer;
        allocParams.Flags.bNotLockable = false;
        uint32_t numResources          = m_pipeline->IsDualEncEnabled() ? AV1_NUM_OF_DUAL_CTX : 1;
        for (auto i = 0; i < AV1_NUM_OF_DUAL_CTX; i++)
        {
            // Bistream decode Tile Line rowstore buffer
            if (!m_avpItf->IsBufferRowstoreCacheEnabled(mhw::vdbox::avp::bsdTileLineBuffer))
            {
                ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::bsdTileLineBuffer, &avpBufSizeParam));
                allocParams.dwBytes                                      = avpBufSizeParam.bufferSize;
                allocParams.pBufName                                     = "Bitstream Decoder Encoder Tile Line Rowstore Read Write buffer";
                m_basicFeature->m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer[i] = m_allocator->AllocateResource(allocParams, false);
            }

            // Deblocker Filter Tile Line Read/Write Y Buffer
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockTileLineYBuffer, &avpBufSizeParam));
            allocParams.dwBytes = avpBufSizeParam.bufferSize;
            allocParams.pBufName = "Deblocker Filter Tile Line Read Write Y Buffer";
            m_basicFeature->m_deblockerFilterTileLineReadWriteYBuffer[i] = m_allocator->AllocateResource(allocParams, false);

            // Deblocker Filter Tile Line Read/Write U Buffer
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockTileLineUBuffer, &avpBufSizeParam));
            allocParams.dwBytes = avpBufSizeParam.bufferSize;
            allocParams.pBufName = "Deblocker Filter Tile Line Read Write U Buffer";
            m_basicFeature->m_deblockerFilterTileLineReadWriteUBuffer[i] = m_allocator->AllocateResource(allocParams, false);

            // Deblocker Filter Tile Line Read/Write V Buffer
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockTileLineVBuffer, &avpBufSizeParam));
            allocParams.dwBytes = avpBufSizeParam.bufferSize;
            allocParams.pBufName = "Deblocker Filter Tile Line Read Write V Buffer";
            m_basicFeature->m_deblockerFilterTileLineReadWriteVBuffer[i] = m_allocator->AllocateResource(allocParams, false);

            // Deblocker Filter Tile Column Read/Write Y Buffer
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockTileColYBuffer, &avpBufSizeParam));
            allocParams.dwBytes = avpBufSizeParam.bufferSize;
            allocParams.pBufName = "Deblocker Filter Tile Column Read Write Y Buffer";
            m_basicFeature->m_deblockerFilterTileColumnReadWriteYBuffer[i]  = m_allocator->AllocateResource(allocParams, false);

            // Deblocker Filter Tile Column Read/Write U Buffer
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockTileColUBuffer, &avpBufSizeParam));
            allocParams.dwBytes = avpBufSizeParam.bufferSize;
            allocParams.pBufName = "Deblocker Filter Tile Column Read Write U Buffer";
            m_basicFeature->m_deblockerFilterTileColumnReadWriteUBuffer[i]  = m_allocator->AllocateResource(allocParams, false);

            // Deblocker Filter Tile Column Read/Write V Buffer
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockTileColVBuffer, &avpBufSizeParam));
            allocParams.dwBytes = avpBufSizeParam.bufferSize;
            allocParams.pBufName = "Deblocker Filter Tile Column Read Write V Buffer";
            m_basicFeature->m_deblockerFilterTileColumnReadWriteVBuffer[i]  = m_allocator->AllocateResource(allocParams, false);

            // CDEF Filter Line Read/Write Buffer
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::cdefLineBuffer, &avpBufSizeParam));
            allocParams.dwBytes                             = avpBufSizeParam.bufferSize;
            allocParams.pBufName                            = "CDEF Filter Line Read Write Buffer";
            m_basicFeature->m_cdefFilterLineReadWriteBuffer[i] = m_allocator->AllocateResource(allocParams, false);

            // CDEF Filter Tile Line Read/Write Buffer
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::cdefTileLineBuffer, &avpBufSizeParam));
            allocParams.dwBytes                                 = avpBufSizeParam.bufferSize;
            allocParams.pBufName                                = "CDEF Filter Tile Line Read Write Buffer";
            m_basicFeature->m_cdefFilterTileLineReadWriteBuffer[i] = m_allocator->AllocateResource(allocParams, false);

            // CDEF Filter Tile Column Read/Write Buffer
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::cdefTileColBuffer, &avpBufSizeParam));
            allocParams.dwBytes                                   = avpBufSizeParam.bufferSize;
            allocParams.pBufName                                  = "CDEF Filter Tile Column Read Write Buffer";
            m_basicFeature->m_cdefFilterTileColumnReadWriteBuffer[i] = m_allocator->AllocateResource(allocParams, false);

            // CDEF Filter Meta Tile Line Read Write Buffer
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::cdefMetaTileLineBuffer, &avpBufSizeParam));
            allocParams.dwBytes                                     = avpBufSizeParam.bufferSize;
            allocParams.pBufName                                    = "CDEF Filter Meta Tile Line Read Write Buffer";
            m_basicFeature->m_cdefFilterMetaTileLineReadWriteBuffer[i] = m_allocator->AllocateResource(allocParams, false);

            // CDEF Filter Meta Tile Column Read Write Buffer
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::cdefMetaTileColBuffer, &avpBufSizeParam));
            allocParams.dwBytes                                       = avpBufSizeParam.bufferSize;
            allocParams.pBufName                                      = "CDEF Filter Meta Tile Column Read Write Buffer";
            m_basicFeature->m_cdefFilterMetaTileColumnReadWriteBuffer[i] = m_allocator->AllocateResource(allocParams, false);

            // CDEF Filter Top Left Corner Read Write Buffer
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::cdefTopLeftCornerBuffer, &avpBufSizeParam));
            allocParams.dwBytes                                      = avpBufSizeParam.bufferSize;
            allocParams.pBufName                                     = "CDEF Filter Top Left Corner Read Write Buffer";
            m_basicFeature->m_cdefFilterTopLeftCornerReadWriteBuffer[i] = m_allocator->AllocateResource(allocParams, false);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::ReadPakMmioRegisters(PMOS_COMMAND_BUFFER cmdBuf, bool firstTile)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(cmdBuf);

        auto mmioRegs = m_miItf->GetMmioRegisters();
        auto mmioRegsAvp = m_avpItf->GetMmioRegisters(MHW_VDBOX_NODE_1);
        ENCODE_CHK_NULL_RETURN(mmioRegs);
        PMOS_RESOURCE bsSizeBuf = m_basicFeature->m_recycleBuf->GetBuffer(PakInfo, 0);
        ENCODE_CHK_NULL_RETURN(bsSizeBuf);

        if (firstTile)
        {
            // clear bitstream size buffer at first tile
            auto &miStoreDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
            miStoreDataParams                  = {};
            miStoreDataParams.pOsResource      = bsSizeBuf;
            miStoreDataParams.dwResourceOffset = 0;
            miStoreDataParams.dwValue          = 0;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuf));
        }

        // load current tile size to VCS_GPR0_Lo
        auto &miLoadRegaParams         = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_REG)();
        miLoadRegaParams               = {};
        miLoadRegaParams.dwSrcRegister = mmioRegsAvp->avpAv1BitstreamByteCountTileRegOffset;
        miLoadRegaParams.dwDstRegister = mmioRegs->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_REG)(cmdBuf));
      
        // load bitstream size buffer to VCS_GPR4_Lo
        auto &miLoadRegMemParams           = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        miLoadRegMemParams                 = {};
        miLoadRegMemParams.presStoreBuffer = bsSizeBuf;
        miLoadRegMemParams.dwOffset        = 0;
        miLoadRegMemParams.dwRegister      = mmioRegs->generalPurposeRegister4LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuf));

        mhw::mi::MHW_MI_ALU_PARAMS aluParams[4] = {};
        int32_t aluCount               = 0;

        //load1 srca, reg1
        aluParams[aluCount].AluOpcode  = MHW_MI_ALU_LOAD;
        aluParams[aluCount].Operand1   = MHW_MI_ALU_SRCA;
        aluParams[aluCount].Operand2   = MHW_MI_ALU_GPREG0;
        ++aluCount;

        //load2 srcb, reg2
        aluParams[aluCount].AluOpcode  = MHW_MI_ALU_LOAD;
        aluParams[aluCount].Operand1   = MHW_MI_ALU_SRCB;
        aluParams[aluCount].Operand2   = MHW_MI_ALU_GPREG4;
        ++aluCount;
            
        //add srca + srcb
        aluParams[aluCount].AluOpcode  = MHW_MI_ALU_ADD;
        ++aluCount;
            
        //store reg1, accu
        aluParams[aluCount].AluOpcode  = MHW_MI_ALU_STORE;
        aluParams[aluCount].Operand1   = MHW_MI_ALU_GPREG0;
        aluParams[aluCount].Operand2   = MHW_MI_ALU_ACCU;
        ++aluCount;

        auto &miMathParams              = m_miItf->MHW_GETPAR_F(MI_MATH)();
        miMathParams                    = {};
        miMathParams.dwNumAluParams     = aluCount;
        miMathParams.pAluPayload        = aluParams;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_MATH)(cmdBuf));

        //store VCS_GPR0_Lo to bitstream size buffer
        auto &miStoreRegMemParams              = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        miStoreRegMemParams                                  = {};
        miStoreRegMemParams.presStoreBuffer                  = bsSizeBuf;
        miStoreRegMemParams.dwOffset                         = 0;
        miStoreRegMemParams.dwRegister                       = mmioRegs->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuf));

        // Make Flush DW call to make sure all previous work is done
        auto &flushDwParams              = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams                    = {};
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuf));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::ReadPakMmioRegistersAtomic(PMOS_COMMAND_BUFFER cmdBuf)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(cmdBuf);

        auto mmioRegs = m_miItf->GetMmioRegisters();
        auto mmioRegsAvp = m_avpItf->GetMmioRegisters(MHW_VDBOX_NODE_1);
        ENCODE_CHK_NULL_RETURN(mmioRegs);

        PMOS_RESOURCE bsSizeBuf = m_basicFeature->m_recycleBuf->GetBuffer(PakInfo, 0);
        ENCODE_CHK_NULL_RETURN(bsSizeBuf);
        ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSkipResourceSync(bsSizeBuf));

        // load current tile size to VCS_GPR0_Lo
        auto &miLoadRegaParams         = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_REG)();
        miLoadRegaParams               = {};
        miLoadRegaParams.dwSrcRegister = mmioRegsAvp->avpAv1BitstreamByteCountTileRegOffset;
        miLoadRegaParams.dwDstRegister = mmioRegs->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_REG)(cmdBuf));

        // clear VCS_GPR0_Hi for sanity
        auto &miLoadRegImmParams         = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
        miLoadRegImmParams               = {};
        miLoadRegImmParams.dwData        = 0;
        miLoadRegImmParams.dwRegister    = mmioRegs->generalPurposeRegister0HiOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuf));

        m_hwInterface->SendMiAtomicDwordIndirectDataCmd(bsSizeBuf, MHW_MI_ATOMIC_ADD, cmdBuf);

        // Make Flush DW call to make sure all previous work is done
        auto &flushDwParams              = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams                    = {};
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuf));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        commandBufferSize = CalculateCommandBufferSize();
        requestedPatchListSize = CalculatePatchListSize();
        return MOS_STATUS_SUCCESS;
    }

    uint32_t Av1VdencPkt::CalculateCommandBufferSize()
    {
        ENCODE_FUNC_CALL();
        uint32_t commandBufferSize = 0;

        uint32_t tileNum = 1;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileNum, tileNum);

        // To be refined later, differentiate BRC and CQP
        commandBufferSize =
            m_pictureStatesSize +
            (m_tileStatesSize * tileNum);

        // 4K align since allocation is in chunks of 4K bytes.
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

        return commandBufferSize;
    }

    uint32_t Av1VdencPkt::CalculatePatchListSize()
    {
        ENCODE_FUNC_CALL();
        uint32_t requestedPatchListSize = 0;

        uint32_t tileNum = 1;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileNum, tileNum);

        if (m_usePatchList)
        {
            requestedPatchListSize =
                m_picturePatchListSize +
                (m_tilePatchListSize * tileNum);
        }

        return requestedPatchListSize;
    }

    MOS_STATUS Av1VdencPkt::CalculateVdencCommandsSize()
    {
        uint32_t vdencPictureStatesSize    = 0;
        uint32_t vdencPicturePatchListSize = 0;
        uint32_t vdencTileStatesSize       = 0;
        uint32_t vdencTilePatchListSize    = 0;

        // Picture Level Commands
        ENCODE_CHK_STATUS_RETURN(GetVdencStateCommandsDataSize(
            &vdencPictureStatesSize,
            &vdencPicturePatchListSize));

        m_pictureStatesSize    += vdencPictureStatesSize;
        m_picturePatchListSize += vdencPicturePatchListSize;

        // Tile Level Commands
        ENCODE_CHK_STATUS_RETURN(GetVdencPrimitiveCommandsDataSize(
            &vdencTileStatesSize,
            &vdencTilePatchListSize));

        m_tileStatesSize    += vdencTileStatesSize;
        m_tilePatchListSize += vdencTilePatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::CalculateAvpPictureStateCommandSize(uint32_t * commandsSize, uint32_t * patchListSize)
    {
        *commandsSize = m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() * 25 +                                           //8 for UpdateStatusReport, 2 for profiler, 15 for metadata
                        m_miItf->MHW_GETSIZE_F(MI_STORE_DATA_IMM)() * 16 +                                     //4 For UpdateStatusReport, 6 for profiler, 6 for metadata
                        m_miItf->MHW_GETSIZE_F(MI_STORE_REGISTER_MEM)() * 31 +                                 //16 for profiler 15 for metadata
                        m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_MEM)() * 15 +                                 //16 for profiler 15 for metadata
                        m_miItf->MHW_GETSIZE_F(MI_ATOMIC)() * 19 +                                              //For UpdateStatusReport, 15 for metadata
                        m_miItf->MHW_GETSIZE_F(MI_COPY_MEM_MEM)() * (sizeof(MetadataAV1PostFeature) / 4 + 4);  //for metadata
                                                                                                                                                                                                    
        *patchListSize = PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) * 25            +
                         PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_DATA_IMM_CMD) * 16      +
                         PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_REGISTER_MEM_CMD) * 31  +
                         PATCH_LIST_COMMAND(mhw::mi::Itf::MI_LOAD_REGISTER_MEM_CMD) * 15   +
                         PATCH_LIST_COMMAND(mhw::mi::Itf::MI_ATOMIC_CMD) * 19              +
                         PATCH_LIST_COMMAND(mhw::mi::Itf::MI_COPY_MEM_MEM_CMD) * (sizeof(MetadataAV1PostFeature) / 4 + 4);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::CalculateAvpCommandsSize()
    {
        uint32_t avpPictureStatesSize    = 0;
        uint32_t avpPicturePatchListSize = 0;
        uint32_t avpTileStatesSize       = 0;
        uint32_t avpTilePatchListSize    = 0;

        // Picture Level Commands
        ENCODE_CHK_STATUS_RETURN(CalculateAvpPictureStateCommandSize(&avpPictureStatesSize, &avpPicturePatchListSize));

        m_pictureStatesSize    += avpPictureStatesSize;
        m_picturePatchListSize += avpPicturePatchListSize;

        // Tile Level Commands
        ENCODE_CHK_STATUS_RETURN(GetAvpPrimitiveCommandsDataSize(
            &avpTileStatesSize,
            &avpTilePatchListSize));

        m_tileStatesSize    += avpTileStatesSize;
        m_tilePatchListSize += avpTilePatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VD_PIPELINE_FLUSH, Av1VdencPkt)
    {
        switch (m_basicFeature->m_flushCmd)
        {
        case Av1BasicFeature::waitVdenc:
            params                           = {};
            params.waitDoneVDCmdMsgParser    = true;
            params.waitDoneVDENC             = true;
            params.flushVDENC                = true;
            params.flushAV1                  = true;
            params.waitDoneAV1               = true;
            break;
        case Av1BasicFeature::waitAvp:
            params                           = {};
            params.waitDoneVDCmdMsgParser    = true;
            params.waitDoneAV1               = true;
            params.flushAV1                  = true;
            break;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, Av1VdencPkt)
    {
        params.pakObjCmdStreamOut = m_vdencPakObjCmdStreamOutEnabled;

        // needs to be enabled for 1st pass in multi-pass case
        // This bit is ignored if PAK only second pass is enabled.
        if ((m_pipeline->GetCurrentPass() == 0) && !m_pipeline->IsLastPass())
        {
            params.pakObjCmdStreamOut = true;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, Av1VdencPkt)
    {
        params.intraRowStoreScratchBuffer       = m_vdencIntraRowStoreScratch;
        params.tileRowStoreBuffer               = m_vdencTileRowStoreBuffer;
        params.cumulativeCuCountStreamOutBuffer = m_resCumulativeCuCountStreamoutBuffer;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, Av1VdencPkt)
    {
        switch (m_pipeline->GetPipeNum())
        {
        case 0:
        case 1:
            params.numPipe = VDENC_PIPE_SINGLE_PIPE;
            break;
        case 2:
            params.numPipe = VDENC_PIPE_TWO_PIPE;
            break;
        case 4:
            params.numPipe = VDENC_PIPE_FOUR_PIPE;
            break;
        default:
            params.numPipe = VDENC_PIPE_INVALID;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_SURFACE_STATE, Av1VdencPkt)
    {
        params.surfaceStateId = m_curAvpSurfStateId;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIPE_MODE_SELECT, Av1VdencPkt)
    {
        params.multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
        params.pipeWorkingMode = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
        
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIPE_BUF_ADDR_STATE, Av1VdencPkt)
    {

        uint32_t idx                             = m_pipeline->IsDualEncEnabled() ? m_pipeline->GetCurrentPipe() : 0;
        params.bsTileLineRowstoreBuffer          = m_basicFeature->m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer[idx];
        params.deblockTileLineYBuffer            = m_basicFeature->m_deblockerFilterTileLineReadWriteYBuffer[idx];
        params.deblockTileLineUBuffer            = m_basicFeature->m_deblockerFilterTileLineReadWriteUBuffer[idx];
        params.deblockTileLineVBuffer            = m_basicFeature->m_deblockerFilterTileLineReadWriteVBuffer[idx];
        params.deblockTileColumnYBuffer          = m_basicFeature->m_deblockerFilterTileColumnReadWriteYBuffer[idx];
        params.deblockTileColumnUBuffer          = m_basicFeature->m_deblockerFilterTileColumnReadWriteUBuffer[idx];
        params.deblockTileColumnVBuffer          = m_basicFeature->m_deblockerFilterTileColumnReadWriteVBuffer[idx];
        params.cdefLineBuffer                    = m_basicFeature->m_cdefFilterLineReadWriteBuffer[idx];
        params.cdefTileLineBuffer                = m_basicFeature->m_cdefFilterTileLineReadWriteBuffer[idx];
        params.cdefTileColumnBuffer              = m_basicFeature->m_cdefFilterTileColumnReadWriteBuffer[idx];
        params.cdefMetaTileLineBuffer            = m_basicFeature->m_cdefFilterMetaTileLineReadWriteBuffer[idx];
        params.cdefMetaTileColumnBuffer          = m_basicFeature->m_cdefFilterMetaTileColumnReadWriteBuffer[idx];
        params.cdefTopLeftCornerBuffer           = m_basicFeature->m_cdefFilterTopLeftCornerReadWriteBuffer[idx];

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_IND_OBJ_BASE_ADDR_STATE, Av1VdencPkt)
    {
        params.mvObjectOffset = m_mvOffset;
        params.mvObjectSize   = m_basicFeature->m_mbCodeSize - m_mvOffset;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1VdencPkt)
    {
        params.notFirstPass = !m_pipeline->IsFirstPass();

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_TILE_CODING, Av1VdencPkt)
    {
        uint32_t tileIdx = 0;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileIdx, tileIdx);
        params.disableFrameContextUpdateFlag = m_av1PicParams->PicFlags.fields.disable_frame_end_update_cdf || (tileIdx != m_av1PicParams->context_update_tile_id);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::AddAllCmds_AVP_SURFACE_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(cmdBuffer);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        m_curAvpSurfStateId = srcInputPic;
        SETPAR_AND_ADDCMD(AVP_SURFACE_STATE, m_avpItf, cmdBuffer);

        m_curAvpSurfStateId = origUpscaledSrc;
        SETPAR_AND_ADDCMD(AVP_SURFACE_STATE, m_avpItf, cmdBuffer);

        m_curAvpSurfStateId = reconPic;
        SETPAR_AND_ADDCMD(AVP_SURFACE_STATE, m_avpItf, cmdBuffer);

        m_curAvpSurfStateId = av1CdefPixelsStreamout;
        SETPAR_AND_ADDCMD(AVP_SURFACE_STATE, m_avpItf, cmdBuffer);

        if (m_av1PicParams->PicFlags.fields.frame_type != keyFrame && m_av1PicParams->PicFlags.fields.frame_type != intraOnlyFrame)
        {
            for (uint8_t i = 0; i < av1TotalRefsPerFrame; i++)
            {
                m_curAvpSurfStateId = i + av1IntraFrame;
                SETPAR_AND_ADDCMD(AVP_SURFACE_STATE, m_avpItf, cmdBuffer);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::AddAllCmds_AVP_PAK_INSERT_OBJECT(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_osInterface);
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        auto &params = m_avpItf->MHW_GETPAR_F(AVP_PAK_INSERT_OBJECT)();
        params       = {};

        auto GetExtraData = [&]() { return params.bsBuffer->pBase + params.offset; };
        auto GetExtraSize = [&]() { return (params.bitSize + 7) >> 3; };

        // First, Send all other OBU bit streams other than tile group OBU when it's first tile in frame
        uint32_t   tileIdx    = 0;
        const bool tgOBUValid = m_basicFeature->m_slcData[0].BitSize > 0 ? true : false;

        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileIdx, tileIdx);
        auto brcFeature = dynamic_cast<Av1Brc *>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);
        if (tileIdx == 0)
        {
            uint32_t nalNum = 0;
            for (uint8_t i = 0; i < MAX_NUM_OBU_TYPES && m_nalUnitParams[i]->uiSize > 0; i++)
            {
                nalNum++;
            }

            params.bsBuffer             = &m_basicFeature->m_bsBuffer;
            params.endOfHeaderInsertion = false;

            // Support multiple packed header buffer
            for (uint32_t i = 0; i < nalNum; i++)
            {
                const uint32_t nalUnitSize   = m_nalUnitParams[i]->uiSize;
                const uint32_t nalUnitOffset = m_nalUnitParams[i]->uiOffset;

                ENCODE_ASSERT(nalUnitSize < CODECHAL_ENCODE_AV1_PAK_INSERT_UNCOMPRESSED_HEADER);

                params.bitSize    = nalUnitSize * 8;
                params.offset     = nalUnitOffset;
                params.lastHeader = !tgOBUValid && (i+1 == nalNum);

                if (IsFrameHeader(*(m_basicFeature->m_bsBuffer.pBase + nalUnitOffset)))
                {
                    if (brcFeature->IsBRCEnabled())
                    {
                        auto pakInsertOutputBatchBuffer = brcFeature->GetPakInsertOutputBatchBuffer(m_pipeline->m_currRecycledBufIdx);
                        ENCODE_CHK_NULL_RETURN(pakInsertOutputBatchBuffer);
                        // send pak insert obj cmds after back annotation
                        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(cmdBuffer, pakInsertOutputBatchBuffer));
                        auto slbbData = brcFeature->GetSLBData();
                        HalOcaInterfaceNext::OnSubLevelBBStart(
                            *cmdBuffer,
                            m_osInterface->pOsContext,
                            &pakInsertOutputBatchBuffer->OsResource,
                            pakInsertOutputBatchBuffer->dwOffset,
                            false,
                            slbbData.pakInsertSlbSize);

                    }
                    else
                    {
                        m_avpItf->MHW_ADDCMD_F(AVP_PAK_INSERT_OBJECT)(cmdBuffer);
                        m_osInterface->pfnAddCommand(cmdBuffer, GetExtraData(), GetExtraSize());
                    }
                }
                else
                {
                    m_avpItf->MHW_ADDCMD_F(AVP_PAK_INSERT_OBJECT)(cmdBuffer);
                    m_osInterface->pfnAddCommand(cmdBuffer, GetExtraData(), GetExtraSize());
                }
            }
        }

        // Second, Send tile group OBU when it is first tile in tile group
        if (tgOBUValid)
        {
            ENCODE_CHK_NULL_RETURN(m_featureManager);

            auto tileFeature = dynamic_cast<Av1EncodeTile *>(m_featureManager->GetFeature(Av1FeatureIDs::encodeTile));
            ENCODE_CHK_NULL_RETURN(tileFeature);

            MHW_CHK_STATUS_RETURN(tileFeature->MHW_SETPAR_F(AVP_PAK_INSERT_OBJECT)(params));
            if (params.bitSize)
            {
                m_avpItf->MHW_ADDCMD_F(AVP_PAK_INSERT_OBJECT)(cmdBuffer);
                m_osInterface->pfnAddCommand(cmdBuffer, GetExtraData(), GetExtraSize());
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::AddAllCmds_AVP_PIPE_MODE_SELECT(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        auto &vdControlStateParams          = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
        vdControlStateParams                = {};
        vdControlStateParams.initialization = true;
        vdControlStateParams.avpEnabled     = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(cmdBuffer));

        // for Gen11+, we need to add MFX wait for both KIN and VRT before and after AVP Pipemode select...
        SETPAR_AND_ADDCMD(MFX_WAIT, m_miItf, cmdBuffer);

        SETPAR_AND_ADDCMD(AVP_PIPE_MODE_SELECT, m_avpItf, cmdBuffer);

        // for Gen11+, we need to add MFX wait for both KIN and VRT before and after AVP Pipemode select...
        SETPAR_AND_ADDCMD(MFX_WAIT, m_miItf, cmdBuffer);

        if (m_pipeline->IsDualEncEnabled())
        {
            vdControlStateParams = {};
            vdControlStateParams.avpEnabled           = true;
            vdControlStateParams.scalableModePipeLock = true;

            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(cmdBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::AddAllCmds_AVP_SEGMENT_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(cmdBuffer);
        ENCODE_CHK_NULL_RETURN(m_featureManager);

        auto &par           = m_avpItf->MHW_GETPAR_F(AVP_SEGMENT_STATE)();
        par                 = {};
        auto segmentFeature = dynamic_cast<Av1Segmentation *>(m_featureManager->GetFeature(Av1FeatureIDs::av1Segmentation));
        ENCODE_CHK_NULL_RETURN(segmentFeature);

        MHW_CHK_STATUS_RETURN(segmentFeature->MHW_SETPAR_F(AVP_SEGMENT_STATE)(par));

        const bool segmentEnabled = par.av1SegmentParams.m_enabled;

        for (uint8_t i = 0; i < av1MaxSegments; i++)
        {
            par.currentSegmentId = i;
            m_avpItf->MHW_ADDCMD_F(AVP_SEGMENT_STATE)(cmdBuffer);

            // If segmentation is not enabled, then AV1_SEGMENT_STATE must still be sent once for SegmentID = 0
            // If i == numSegments -1, means all segments are issued, break the loop
            if (!segmentEnabled || (i == par.numSegments - 1))
            {
                break;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::GetVdencStateCommandsDataSize(uint32_t *commandsSize, uint32_t *patchListSize) const
    {
        uint32_t            maxSize          = 0;
        uint32_t            patchListMaxSize = 0;

        maxSize = maxSize +
            m_vdencItf->MHW_GETSIZE_F(VDENC_CONTROL_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_PIPE_MODE_SELECT)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_SRC_SURFACE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_REF_SURFACE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_DS_REF_SURFACE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_PIPE_BUF_ADDR_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_WALKER_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VD_PIPELINE_FLUSH)();

        patchListMaxSize = patchListMaxSize +
            PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) +
            PATCH_LIST_COMMAND(mhw::mi::Itf::MI_BATCH_BUFFER_START_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::vdenc::Itf::VDENC_PIPE_BUF_ADDR_STATE_CMD);

        maxSize = maxSize +
            m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_IMM)()*8 +
            m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() +
            m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_START)() +
            m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();

        ENCODE_CHK_NULL_RETURN(commandsSize);
        ENCODE_CHK_NULL_RETURN(patchListSize);
        *commandsSize  = maxSize;
        *patchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::UpdateStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        //initialize following
        MOS_RESOURCE *osResourceInline = nullptr;
        uint32_t      offsetInline     = 0;
        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportGlobalCount, osResourceInline, offsetInline));
        offsetInline             = m_atomicScratchBuf.operandSetSize * m_atomicScratchBuf.encodeUpdateIndex;
        uint32_t zeroValueOffset = offsetInline;
        uint32_t operand1Offset  = offsetInline + m_atomicScratchBuf.operand1Offset;
        uint32_t operand2Offset  = offsetInline + m_atomicScratchBuf.operand2Offset;
        uint32_t operand3Offset  = offsetInline + m_atomicScratchBuf.operand3Offset;
        auto     mmioRegisters   = m_hwInterface->GetVdencInterfaceNext()->GetMmioRegisters(m_vdboxIndex);

        // Make Flush DW call to make sure all previous work is done
        auto &flushDwParams = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams       = {};
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        // n1_lo = 0x00
        auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        storeDataParams                  = {};
        storeDataParams.pOsResource      = m_atomicScratchBuf.resAtomicScratchBuffer;
        storeDataParams.dwResourceOffset = operand1Offset;
        storeDataParams.dwValue          = 0x00;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        // n2_lo = dwImageStatusMask
        auto &copyMemMemParams       = m_miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
        copyMemMemParams             = {};
        copyMemMemParams.presSrc     = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
        copyMemMemParams.dwSrcOffset = (sizeof(uint32_t) * 1);
        copyMemMemParams.presDst     = m_atomicScratchBuf.resAtomicScratchBuffer;
        copyMemMemParams.dwDstOffset = operand2Offset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));

        // VCS_GPR0_Lo = ImageStatusCtrl
        auto &registerMemParams           = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
        registerMemParams.dwOffset        = (sizeof(uint32_t) * 0);
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;  // VCS_GPR0_Lo
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // Reset GPR4_Lo
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = zeroValueOffset;                                 //Offset 0, has value of 0.
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4LoOffset;  // VCS_GPR4
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // Make Flush DW call to make sure all previous work is done
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        // step-1: n2_lo = n2_lo & VCS_GPR0_Lo = dwImageStatusMask & ImageStatusCtrl
        auto &atomicParams            = m_miItf->MHW_GETPAR_F(MI_ATOMIC)();
        atomicParams                  = {};
        atomicParams.pOsResource      = m_atomicScratchBuf.resAtomicScratchBuffer;
        atomicParams.dwResourceOffset = operand2Offset;
        atomicParams.dwDataSize       = sizeof(uint32_t);
        atomicParams.Operation        = mhw::mi::MHW_MI_ATOMIC_AND;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_ATOMIC)(cmdBuffer));

        // n3_lo = 0
        storeDataParams                  = {};
        storeDataParams.pOsResource      = m_atomicScratchBuf.resAtomicScratchBuffer;
        storeDataParams.dwResourceOffset = operand3Offset;
        storeDataParams.dwValue          = 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        // Make Flush DW call to make sure all previous work is done
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        // GPR0_lo = n1_lo = 0
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = operand1Offset;
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;  // VCS_GPR0
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // Reset GPR4_Lo
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = zeroValueOffset;                                 //Offset 0, has value of 0.
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4LoOffset;  // VCS_GPR4
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // Make Flush DW call to make sure all previous work is done
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        // step-2: n2_lo == n1_lo ? 0 : n2_lo
        // compare n1 vs n2. i.e. GRP0 vs. memory of operand2
        atomicParams                  = {};
        atomicParams.pOsResource      = m_atomicScratchBuf.resAtomicScratchBuffer;
        atomicParams.dwResourceOffset = operand2Offset;
        atomicParams.dwDataSize       = sizeof(uint32_t);
        atomicParams.Operation        = mhw::mi::MHW_MI_ATOMIC_CMP;
        atomicParams.bReturnData      = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_ATOMIC)(cmdBuffer));

        // n2_hi = 1
        storeDataParams                  = {};
        storeDataParams.pOsResource      = m_atomicScratchBuf.resAtomicScratchBuffer;
        storeDataParams.dwResourceOffset = operand2Offset + sizeof(uint32_t);
        storeDataParams.dwValue          = 1;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        // n3_hi = 1
        storeDataParams                  = {};
        storeDataParams.pOsResource      = m_atomicScratchBuf.resAtomicScratchBuffer;
        storeDataParams.dwResourceOffset = operand3Offset + sizeof(uint32_t);
        storeDataParams.dwValue          = 1;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        // VCS_GPR0_Lo = n3_lo = 0
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = operand3Offset;
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;  // VCS_GPR0_Lo
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // GPR0_Hi = n2_hi = 1
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = operand2Offset + sizeof(uint32_t);               // update 1
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0HiOffset;  // VCS_GPR0_Hi
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // Reset GPR4_Lo and GPR4_Hi
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = zeroValueOffset;
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4LoOffset;  // VCS_GPR4_Lo
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = zeroValueOffset;
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4HiOffset;  // VCS_GPR4_Hi
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // Make Flush DW call to make sure all previous work is done
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        // step-3: n2 = (n2 == 0:1) ? 0:0 : n2      // uint64_t CMP
        // If n2==0 (Lo) and 1 (Hi), covert n2 to 0 (Lo)and 0 (Hi), else no change.
        // n2 == 0:1 means encoding completsion. the n2 memory will be updated with 0:0, otherwise, no change.
        atomicParams                  = {};
        atomicParams.pOsResource      = m_atomicScratchBuf.resAtomicScratchBuffer;
        atomicParams.dwResourceOffset = operand2Offset;
        atomicParams.dwDataSize       = sizeof(uint64_t);
        atomicParams.Operation        = mhw::mi::MHW_MI_ATOMIC_CMP;
        atomicParams.bReturnData      = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_ATOMIC)(cmdBuffer));

        // Make Flush DW call to make sure all previous work is done
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        // VCS_GPR0_Lo = n3_hi = 1
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = operand3Offset + sizeof(uint32_t);
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;  // VCS_GPR0_Lo
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // Make Flush DW call to make sure all previous work is done
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        // step-4: n2_hi = n2_hi ^ VCS_GPR0_Lo = n2_hi ^ n3_hi
        atomicParams                  = {};
        atomicParams.pOsResource      = m_atomicScratchBuf.resAtomicScratchBuffer;
        atomicParams.dwResourceOffset = operand2Offset + sizeof(uint32_t);
        atomicParams.dwDataSize       = sizeof(uint32_t);
        atomicParams.Operation        = mhw::mi::MHW_MI_ATOMIC_XOR;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_ATOMIC)(cmdBuffer));

        // VCS_GPR0_Lo = n2_hi
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = operand2Offset + sizeof(uint32_t);
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;  // VCS_GPR0_Lo
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // step-5: m_storeData = m_storeData + VCS_GPR0_Lo = m_storeData + n2_hi
        // if not completed n2_hi should be 0, then m_storeData = m_storeData + 0
        // if completed, n2_hi should be 1, then m_storeData = m_storeData + 1
        auto &miLoadRegMemParams           = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        miLoadRegMemParams                 = {};
        miLoadRegMemParams.presStoreBuffer = osResourceInline;
        miLoadRegMemParams.dwOffset        = 0;
        miLoadRegMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        mhw::mi::MHW_MI_ALU_PARAMS aluParams[4] = {0};
        int                        aluCount     = 0;

        //load1 srca, reg1
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
        aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCA;
        aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG0;
        ++aluCount;
        //load srcb, reg2
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
        aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCB;
        aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG4;
        ++aluCount;
        //add
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_ADD;
        ++aluCount;
        //store reg1, accu
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
        aluParams[aluCount].Operand1  = MHW_MI_ALU_GPREG0;
        aluParams[aluCount].Operand2  = MHW_MI_ALU_ACCU;
        ++aluCount;

        auto &miMathParams          = m_miItf->MHW_GETPAR_F(MI_MATH)();
        miMathParams                = {};
        miMathParams.dwNumAluParams = aluCount;
        miMathParams.pAluPayload    = aluParams;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_MATH)(cmdBuffer));

        auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = osResourceInline;
        miStoreRegMemParams.dwOffset        = 0;
        miStoreRegMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

        // Make Flush DW call to make sure all previous work is done
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::GetVdencPrimitiveCommandsDataSize(uint32_t *commandsSize, uint32_t *patchListSize) const
    {
        uint32_t            maxSize          = 0;
        uint32_t            patchListMaxSize = 0;

        maxSize = maxSize +
            m_vdencItf->MHW_GETSIZE_F(VDENC_HEVC_VP9_TILE_SLICE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_CMD1)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_CMD2)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_WALKER_STATE)();

        ENCODE_CHK_NULL_RETURN(commandsSize);
        ENCODE_CHK_NULL_RETURN(patchListSize);
        *commandsSize  = maxSize;
        *patchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::GetAvpPrimitiveCommandsDataSize(uint32_t *commandsSize, uint32_t *patchListSize) const
    {
        uint32_t maxSize          = 0;
        uint32_t patchListMaxSize = 0;

        maxSize = m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_START)() * AV1_MAX_NUM_OF_BATCH_BUFFER +
            m_miItf->MHW_GETSIZE_F(VD_CONTROL_STATE)()                                          +
            m_miItf->MHW_GETSIZE_F(MFX_WAIT)()                                                  +
            m_avpItf->MHW_GETSIZE_F(AVP_PIPE_MODE_SELECT)()                                     +
            m_miItf->MHW_GETSIZE_F(MFX_WAIT)()                                                  +
            m_avpItf->MHW_GETSIZE_F(AVP_SURFACE_STATE)() * av1SurfaceNums                       +
            m_avpItf->MHW_GETSIZE_F(AVP_PIPE_BUF_ADDR_STATE)()                                  +
            m_avpItf->MHW_GETSIZE_F(AVP_IND_OBJ_BASE_ADDR_STATE)()                              +
            m_avpItf->MHW_GETSIZE_F(AVP_PIC_STATE)()                                            +
            m_avpItf->MHW_GETSIZE_F(AVP_INTER_PRED_STATE)()                                     +
            m_avpItf->MHW_GETSIZE_F(AVP_SEGMENT_STATE)() * AV1_MAX_NUM_OF_SEGMENTS              +
            m_avpItf->MHW_GETSIZE_F(AVP_INLOOP_FILTER_STATE)()                                  +
            m_avpItf->MHW_GETSIZE_F(AVP_TILE_CODING)()                                          +
            m_avpItf->MHW_GETSIZE_F(AVP_PAK_INSERT_OBJECT)() * MAX_NUM_OBU_TYPES                +
            m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();

        patchListMaxSize =
            PATCH_LIST_COMMAND(mhw::mi::Itf::MI_BATCH_BUFFER_START_CMD) * AV1_MAX_NUM_OF_BATCH_BUFFER +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::VD_PIPELINE_FLUSH_CMD)                           +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PIPE_MODE_SELECT_CMD)                        +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_SURFACE_STATE_CMD) * av1SurfaceNums          +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PIPE_BUF_ADDR_STATE_CMD)                     +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_IND_OBJ_BASE_ADDR_STATE_CMD)                 +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PIC_STATE_CMD)                               +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_INTER_PRED_STATE_CMD)                        +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_SEGMENT_STATE_CMD) * AV1_MAX_NUM_OF_SEGMENTS +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_INLOOP_FILTER_STATE_CMD)                     +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_TILE_CODING_CMD)                             +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PAK_INSERT_OBJECT_CMD) * MAX_NUM_OBU_TYPES;

        ENCODE_CHK_NULL_RETURN(commandsSize);
        ENCODE_CHK_NULL_RETURN(patchListSize);
        *commandsSize  = maxSize;
        *patchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::PrepareHWMetaData(MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        if (!m_basicFeature->m_resMetadataBuffer)
        {
            return MOS_STATUS_SUCCESS;
        }

        AV1MetaDataOffset AV1ResourceOffset = m_basicFeature->m_AV1metaDataOffset;
        MetaDataOffset    resourceOffset    = m_basicFeature->m_metaDataOffset;

        //ENCODER_OUTPUT_METADATA
        auto &storeDataParams = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        storeDataParams       = {};

        ENCODE_CHK_STATUS_RETURN(PrepareHWMetaDataFromDriver(cmdBuffer, resourceOffset, AV1ResourceOffset));
        ENCODE_CHK_STATUS_RETURN(PrepareHWMetaDataFromStreamout(cmdBuffer, resourceOffset, AV1ResourceOffset));
        ENCODE_CHK_STATUS_RETURN(PrepareHWMetaDataFromRegister(cmdBuffer, resourceOffset));

        return MOS_STATUS_SUCCESS;
    }
    MOS_STATUS Av1VdencPkt::PrepareHWMetaDataFromStreamout(MOS_COMMAND_BUFFER *cmdBuffer, const MetaDataOffset resourceOffset, const AV1MetaDataOffset AV1ResourceOffset)
    {
        ENCODE_FUNC_CALL();
        uint32_t tileNum     = 0;
        auto     tileFeature = dynamic_cast<Av1EncodeTile *>(m_featureManager->GetFeature(Av1FeatureIDs::encodeTile));
        ENCODE_CHK_NULL_RETURN(tileFeature);
        ENCODE_CHK_STATUS_RETURN(tileFeature->GetTileNum(tileNum));

        uint32_t      frameSubregionOffset = resourceOffset.dwMetaDataSize;
        MOS_RESOURCE *tileRecordBuffer     = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileRecordBuffer, m_basicFeature->m_currOriginalPic.FrameIdx, tileRecordBuffer);  //curroriginalpic.frameidx
        ENCODE_CHK_NULL_RETURN(tileRecordBuffer);

        m_basicFeature->m_numSlices = tileNum;
        auto &miCpyMemMemParams     = m_miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
        auto &storeDataParams       = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();

        //memeset EncodedBitstreamWrittenBytesCount
        storeDataParams             = {};
        storeDataParams.pOsResource = m_basicFeature->m_resMetadataBuffer;
        storeDataParams.dwResourceOffset = resourceOffset.dwEncodedBitstreamWrittenBytesCount;
        storeDataParams.dwValue          = 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        for (uint32_t tileIdx = 0; tileIdx < tileNum; tileIdx++)
        {
            uint32_t frameSubregionOffset = resourceOffset.dwMetaDataSize + tileIdx * resourceOffset.dwMetaDataSubRegionSize;
            miCpyMemMemParams.presSrc     = tileRecordBuffer;
            miCpyMemMemParams.dwSrcOffset = tileIdx * sizeof(PakHwTileSizeRecord) + 2 * sizeof(uint32_t);  //skip dword0 dword1
            miCpyMemMemParams.presDst     = m_basicFeature->m_resMetadataBuffer;
            miCpyMemMemParams.dwDstOffset = frameSubregionOffset + resourceOffset.dwbSize;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));

            storeDataParams.dwResourceOffset = frameSubregionOffset + resourceOffset.dwbStartOffset;
            if (tileIdx == tileNum - 1)
                storeDataParams.dwValue = 0;
            else
                storeDataParams.dwValue = TILE_SIZE_BYTES;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

            storeDataParams.dwResourceOffset = frameSubregionOffset + resourceOffset.dwbHeaderSize;
            storeDataParams.dwValue          = 0;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

            //count for fame bytescount
            ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodedBitstreamWrittenBytesCount, tileRecordBuffer, tileIdx * sizeof(PakHwTileSizeRecord) + 2 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));
        }
        if (m_basicFeature->m_av1SeqParams->RateControlMethod != RATECONTROL_CQP)
        {
            auto brcFeature = dynamic_cast<Av1Brc *>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
            ENCODE_CHK_NULL_RETURN(brcFeature);
            auto vdenc2ndLevelBatchBuffer = brcFeature->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);
            auto slbbData                 = brcFeature->GetSLBData();
            
            //qp
            uint32_t qpOffset     = resourceOffset.dwMetaDataSize + tileNum * resourceOffset.dwMetaDataSubRegionSize + resourceOffset.dwTilePartitionSize + AV1ResourceOffset.dwQuantization + AV1ResourceOffset.dwBaseQIndex;
            uint32_t QPOffsetSLBB = slbbData.secondAvpPicStateOffset + 4 * sizeof(uint32_t);//dword 4
            ENCODE_CHK_STATUS_RETURN(readBRCMetaDataFromSLBB(cmdBuffer, m_basicFeature->m_resMetadataBuffer, qpOffset, &vdenc2ndLevelBatchBuffer->OsResource, QPOffsetSLBB, 0xFF0000));
            
            //loop filter
            uint32_t loopFilterOffset     = resourceOffset.dwMetaDataSize + tileNum * resourceOffset.dwMetaDataSubRegionSize + resourceOffset.dwTilePartitionSize + AV1ResourceOffset.dwLoopFilter;
            uint32_t loopFilterOffsetSLBB = slbbData.avpInloopFilterStateOffset + sizeof(uint32_t);  //dword 1 
            ENCODE_CHK_STATUS_RETURN(readBRCMetaDataFromSLBB(cmdBuffer, m_basicFeature->m_resMetadataBuffer, loopFilterOffset + AV1ResourceOffset.dwLoopFilterLevel, &vdenc2ndLevelBatchBuffer->OsResource, loopFilterOffsetSLBB, 0x3F));
            ENCODE_CHK_STATUS_RETURN(readBRCMetaDataFromSLBB(cmdBuffer, m_basicFeature->m_resMetadataBuffer, loopFilterOffset + AV1ResourceOffset.dwLoopFilterLevel + sizeof(uint64_t), &vdenc2ndLevelBatchBuffer->OsResource, loopFilterOffsetSLBB, 0xFC0));
            ENCODE_CHK_STATUS_RETURN(readBRCMetaDataFromSLBB(cmdBuffer, m_basicFeature->m_resMetadataBuffer, loopFilterOffset + AV1ResourceOffset.dwLoopFilterLevelU, &vdenc2ndLevelBatchBuffer->OsResource, loopFilterOffsetSLBB, 0x3F000));
            ENCODE_CHK_STATUS_RETURN(readBRCMetaDataFromSLBB(cmdBuffer, m_basicFeature->m_resMetadataBuffer, loopFilterOffset + AV1ResourceOffset.dwLoopFilterLevelV, &vdenc2ndLevelBatchBuffer->OsResource, loopFilterOffsetSLBB, 0xFC0000));
            ENCODE_CHK_STATUS_RETURN(readBRCMetaDataFromSLBB(cmdBuffer, m_basicFeature->m_resMetadataBuffer, loopFilterOffset + AV1ResourceOffset.dwLoopFilterSharpnessLevel, &vdenc2ndLevelBatchBuffer->OsResource, loopFilterOffsetSLBB, 0x7000000));

            //cdef
            uint32_t CDEFOffset     = resourceOffset.dwMetaDataSize + tileNum * resourceOffset.dwMetaDataSubRegionSize + resourceOffset.dwTilePartitionSize + AV1ResourceOffset.dwCDEF;
            uint32_t CDEFOffsetSLBB = slbbData.avpInloopFilterStateOffset + 5 * sizeof(uint32_t);  //dword 5 
            ENCODE_CHK_STATUS_RETURN(readBRCMetaDataFromSLBB(cmdBuffer, m_basicFeature->m_resMetadataBuffer, CDEFOffset + AV1ResourceOffset.dwCdefBits, &vdenc2ndLevelBatchBuffer->OsResource, CDEFOffsetSLBB, 0x30000000));
            ENCODE_CHK_STATUS_RETURN(readBRCMetaDataFromSLBB(cmdBuffer, m_basicFeature->m_resMetadataBuffer, CDEFOffset + AV1ResourceOffset.dwCdefDampingMinus3, &vdenc2ndLevelBatchBuffer->OsResource, CDEFOffsetSLBB, 0xC0000000));
            for (uint32_t i = 0; i < 8; i++)
            {
                ENCODE_CHK_STATUS_RETURN(readBRCMetaDataFromSLBB(cmdBuffer, m_basicFeature->m_resMetadataBuffer, CDEFOffset + AV1ResourceOffset.dwCdefYPriStrength + i * sizeof(uint64_t), &vdenc2ndLevelBatchBuffer->OsResource, CDEFOffsetSLBB + (i / 4) * sizeof(uint32_t), 0x3c << (i % 4 * 6)));
                ENCODE_CHK_STATUS_RETURN(readBRCMetaDataFromSLBB(cmdBuffer, m_basicFeature->m_resMetadataBuffer, CDEFOffset + AV1ResourceOffset.dwCdefYSecStrength + i * sizeof(uint64_t), &vdenc2ndLevelBatchBuffer->OsResource, CDEFOffsetSLBB + (i / 4) * sizeof(uint32_t), 0x03 << (i % 4 * 6)));
                ENCODE_CHK_STATUS_RETURN(readBRCMetaDataFromSLBB(cmdBuffer, m_basicFeature->m_resMetadataBuffer, CDEFOffset + AV1ResourceOffset.dwCdefUVPriStrength + i * sizeof(uint64_t), &vdenc2ndLevelBatchBuffer->OsResource, CDEFOffsetSLBB + (2 + i / 4) * sizeof(uint32_t), 0x3c << (i % 4 * 6))); 
                ENCODE_CHK_STATUS_RETURN(readBRCMetaDataFromSLBB(cmdBuffer, m_basicFeature->m_resMetadataBuffer, CDEFOffset + AV1ResourceOffset.dwCdefUVSecStrength + i * sizeof(uint64_t), &vdenc2ndLevelBatchBuffer->OsResource, CDEFOffsetSLBB + (2 + i / 4) * sizeof(uint32_t), 0x03 << (i % 4 * 6)));
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::PrepareHWMetaDataFromRegister(MOS_COMMAND_BUFFER *cmdBuffer, const MetaDataOffset resourceOffset)
    {
        ENCODE_FUNC_CALL();

        //qpy
        auto  mmioRegisters               = m_avpItf->GetMmioRegisters(m_vdboxIndex);
        ENCODE_CHK_NULL_RETURN(mmioRegisters);
        auto &storeRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        storeRegMemParams                 = {};
        storeRegMemParams.presStoreBuffer = m_basicFeature->m_resMetadataBuffer;
        storeRegMemParams.dwOffset        = resourceOffset.dwEncodeStats + resourceOffset.dwAverageQP;
        storeRegMemParams.dwRegister      = mmioRegisters->avpAv1QpStatusCountRegOffset;  //This register stores cummulative QP for all SBs
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::PrepareHWMetaDataFromDriver(MOS_COMMAND_BUFFER *cmdBuffer, const MetaDataOffset resourceOffset, const AV1MetaDataOffset AV1ResourceOffset)
    {
        ENCODE_FUNC_CALL();

        auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        storeDataParams                  = {};
        storeDataParams.pOsResource      = m_basicFeature->m_resMetadataBuffer;
        storeDataParams.pOsResource      = m_basicFeature->m_resMetadataBuffer;
        storeDataParams.dwResourceOffset = resourceOffset.dwEncodeErrorFlags;
        storeDataParams.dwValue          = 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        uint32_t tileNum     = 0;
        auto     tileFeature = dynamic_cast<Av1EncodeTile *>(m_featureManager->GetFeature(Av1FeatureIDs::encodeTile));
        ENCODE_CHK_NULL_RETURN(tileFeature);
        ENCODE_CHK_STATUS_RETURN(tileFeature->GetTileNum(tileNum));
        storeDataParams.dwResourceOffset = resourceOffset.dwWrittenSubregionsCount;
        storeDataParams.dwValue          = tileNum;  //frame subregion num
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        PMOS_RESOURCE           previewMetadataBuffer = nullptr;
        MOS_ALLOC_GFXRES_PARAMS allocParams;
        MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParams.Type               = MOS_GFXRES_BUFFER;
        allocParams.TileType           = MOS_TILE_LINEAR;
        allocParams.Format             = Format_Buffer;
        allocParams.Flags.bNotLockable = false;
        allocParams.dwBytes            = sizeof(MetadataAV1PostFeature);
        allocParams.pBufName           = "feature values for resMetadataBuffer";
        previewMetadataBuffer          = m_allocator->AllocateResource(allocParams, false);

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        uint8_t *data       = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            previewMetadataBuffer,
            &lockFlags);

        MetadataAV1PostFeature *previewMetadata = reinterpret_cast<MetadataAV1PostFeature *>(data);
        ENCODE_CHK_NULL_RETURN(previewMetadata);

        uint16_t numTileRows = 0;
        uint16_t numTileCols = 0;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileCols);
        previewMetadata->tilePartition.ColCount            = numTileCols;
        previewMetadata->tilePartition.ContextUpdateTileId = m_basicFeature->m_av1PicParams->context_update_tile_id;
        previewMetadata->tilePartition.RowCount            = numTileRows;
        for (int rowIdx = 0; rowIdx < numTileRows; rowIdx++)
        {
            previewMetadata->tilePartition.RowHeights[rowIdx] = m_basicFeature->m_av1PicParams->height_in_sbs_minus_1[rowIdx] + 1;
        }
        for (int colIdx = 0; colIdx < numTileCols; colIdx++)
        {
            previewMetadata->tilePartition.ColWidths[colIdx] = m_basicFeature->m_av1PicParams->width_in_sbs_minus_1[colIdx] + 1;
        }

        previewMetadata->postFeature.CompoundPredictionType = m_basicFeature->m_av1PicParams->dwModeControlFlags.fields.reference_mode == referenceModeSelect;

        previewMetadata->postFeature.LoopFilter.LoopFilterLevel[0]       = m_basicFeature->m_av1PicParams->filter_level[0];
        previewMetadata->postFeature.LoopFilter.LoopFilterLevel[1]       = m_basicFeature->m_av1PicParams->filter_level[1];
        previewMetadata->postFeature.LoopFilter.LoopFilterLevelU         = m_basicFeature->m_av1PicParams->filter_level_u;
        previewMetadata->postFeature.LoopFilter.LoopFilterLevelV         = m_basicFeature->m_av1PicParams->filter_level_v;
        previewMetadata->postFeature.LoopFilter.LoopFilterSharpnessLevel = m_basicFeature->m_av1PicParams->cLoopFilterInfoFlags.fields.sharpness_level;
        previewMetadata->postFeature.LoopFilter.LoopFilterDeltaEnabled   = m_basicFeature->m_av1PicParams->cLoopFilterInfoFlags.fields.mode_ref_delta_enabled;
        previewMetadata->postFeature.LoopFilter.UpdateRefDelta           = m_basicFeature->m_av1PicParams->cLoopFilterInfoFlags.fields.mode_ref_delta_update;
        previewMetadata->postFeature.LoopFilter.UpdateModeDelta          = m_basicFeature->m_av1PicParams->cLoopFilterInfoFlags.fields.mode_ref_delta_update;
        for (int i = 0; i < 8; i++)
            previewMetadata->postFeature.LoopFilter.RefDeltas[i] = m_basicFeature->m_av1PicParams->ref_deltas[i];
        for (int i = 0; i < 2; i++)
            previewMetadata->postFeature.LoopFilter.ModeDeltas[i] = m_basicFeature->m_av1PicParams->mode_deltas[i];

        previewMetadata->postFeature.LoopFilterDelta.DeltaLFPresent = m_basicFeature->m_av1PicParams->dwModeControlFlags.fields.delta_lf_present_flag;
        previewMetadata->postFeature.LoopFilterDelta.DeltaLFMulti   = m_basicFeature->m_av1PicParams->dwModeControlFlags.fields.delta_lf_multi;
        previewMetadata->postFeature.LoopFilterDelta.DeltaLFRes     = m_basicFeature->m_av1PicParams->dwModeControlFlags.fields.log2_delta_lf_res;

        previewMetadata->postFeature.Quantization.BaseQIndex   = m_basicFeature->m_av1PicParams->base_qindex;
        previewMetadata->postFeature.Quantization.QMY          = m_basicFeature->m_av1PicParams->wQMatrixFlags.fields.qm_y;
        previewMetadata->postFeature.Quantization.QMU          = m_basicFeature->m_av1PicParams->wQMatrixFlags.fields.qm_u;
        previewMetadata->postFeature.Quantization.QMV          = m_basicFeature->m_av1PicParams->wQMatrixFlags.fields.qm_v;
        previewMetadata->postFeature.Quantization.UsingQMatrix = m_basicFeature->m_av1PicParams->wQMatrixFlags.fields.using_qmatrix;
        previewMetadata->postFeature.Quantization.UACDeltaQ    = m_basicFeature->m_av1PicParams->u_ac_delta_q;
        previewMetadata->postFeature.Quantization.UDCDeltaQ    = m_basicFeature->m_av1PicParams->u_dc_delta_q;
        previewMetadata->postFeature.Quantization.VACDeltaQ    = m_basicFeature->m_av1PicParams->v_ac_delta_q;
        previewMetadata->postFeature.Quantization.VDCDeltaQ    = m_basicFeature->m_av1PicParams->v_dc_delta_q;
        previewMetadata->postFeature.Quantization.YDCDeltaQ    = m_basicFeature->m_av1PicParams->y_dc_delta_q;

        previewMetadata->postFeature.QuantizationDelta.DeltaQPresent = m_basicFeature->m_av1PicParams->dwModeControlFlags.fields.delta_q_present_flag;
        previewMetadata->postFeature.QuantizationDelta.DeltaQRes     = m_basicFeature->m_av1PicParams->dwModeControlFlags.fields.log2_delta_q_res;

        previewMetadata->postFeature.CDEF.CdefBits          = m_basicFeature->m_av1PicParams->cdef_bits;
        previewMetadata->postFeature.CDEF.CdefDampingMinus3 = m_basicFeature->m_av1PicParams->cdef_damping_minus_3;
        for (int i = 0; i < 8; i++)
        {
            previewMetadata->postFeature.CDEF.CdefYPriStrength[i]  = m_basicFeature->m_av1PicParams->cdef_y_strengths[i] / 4;
            previewMetadata->postFeature.CDEF.CdefUVPriStrength[i] = m_basicFeature->m_av1PicParams->cdef_uv_strengths[i] / 4;
            previewMetadata->postFeature.CDEF.CdefYSecStrength[i]  = m_basicFeature->m_av1PicParams->cdef_y_strengths[i] % 4;
            previewMetadata->postFeature.CDEF.CdefUVSecStrength[i] = m_basicFeature->m_av1PicParams->cdef_uv_strengths[i] % 4;
        }

        previewMetadata->postFeature.SegmentationConfig.UpdateMap      = m_basicFeature->m_av1PicParams->stAV1Segments.SegmentFlags.fields.update_map;
        previewMetadata->postFeature.SegmentationConfig.UpdateData     = m_basicFeature->m_av1PicParams->stAV1Segments.SegmentFlags.fields.update_map;
        previewMetadata->postFeature.SegmentationConfig.TemporalUpdate = m_av1PicParams->stAV1Segments.SegmentFlags.fields.temporal_update;
        previewMetadata->postFeature.SegmentationConfig.NumSegments    = m_basicFeature->m_av1PicParams->stAV1Segments.SegmentFlags.fields.SegmentNumber;
        for (int i = 0; i < 8; i++)
        {
            previewMetadata->postFeature.SegmentationConfig.SegmentsData[i].EnabledFeatures = m_basicFeature->m_av1PicParams->stAV1Segments.feature_mask[i];
            for (int j = 0; j < 8; j++)
                previewMetadata->postFeature.SegmentationConfig.SegmentsData[i].FeatureValue[j] = m_basicFeature->m_av1PicParams->stAV1Segments.feature_data[i][j];
        }

        previewMetadata->postFeature.PrimaryRefFrame = m_basicFeature->m_av1PicParams->primary_ref_frame;
        for (int i = 0; i < 7; i++)
            previewMetadata->postFeature.ReferenceIndices[i] = m_basicFeature->m_av1PicParams->ref_frame_idx[i];

        ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(m_osInterface, previewMetadataBuffer));

        auto &miCpyMemMemParams   = m_miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
        miCpyMemMemParams.presSrc = previewMetadataBuffer;
        for (uint32_t i = 0; i < allocParams.dwBytes; i += 4)
        {
            miCpyMemMemParams.dwSrcOffset = i;
            miCpyMemMemParams.presDst     = m_basicFeature->m_resMetadataBuffer;
            miCpyMemMemParams.dwDstOffset = resourceOffset.dwMetaDataSize + tileNum * resourceOffset.dwMetaDataSubRegionSize + i;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }
    MOS_STATUS Av1VdencPkt::readBRCMetaDataFromSLBB(MOS_COMMAND_BUFFER *cmdBuffer, PMOS_RESOURCE presDst, uint32_t dstOffset, PMOS_RESOURCE presSrc, uint32_t srcOffset, uint32_t significantBits)
    {
        auto &miCpyMemMemParams = m_miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
        miCpyMemMemParams             = {};
        
        miCpyMemMemParams.presSrc     = presSrc;
        miCpyMemMemParams.dwSrcOffset = srcOffset;
        miCpyMemMemParams.presDst     = presDst;
        miCpyMemMemParams.dwDstOffset = dstOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));

        auto &atomicParams             = m_miItf->MHW_GETPAR_F(MI_ATOMIC)();
        atomicParams                   = {};
        atomicParams.pOsResource       = presDst;
        atomicParams.dwResourceOffset  = dstOffset;
        atomicParams.dwDataSize        = sizeof(uint32_t);
        atomicParams.Operation         = mhw::mi::MHW_MI_ATOMIC_AND;
        atomicParams.bInlineData       = true;
        atomicParams.dwOperand1Data[0] = significantBits;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_ATOMIC)(cmdBuffer));

        // load current tile size to VCS_GPR0_Lo
        auto  mmioRegs                     = m_miItf->GetMmioRegisters();
        auto &miLoadRegMemParams           = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        miLoadRegMemParams                 = {};
        miLoadRegMemParams.presStoreBuffer = presDst;
        miLoadRegMemParams.dwOffset        = dstOffset;
        miLoadRegMemParams.dwRegister      = mmioRegs->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));


        uint32_t SHRNum          = static_cast<uint32_t>(log2(significantBits & (significantBits - 1) ^ significantBits));
        uint32_t miMathCmdNum    = 0;
        uint32_t SHRbit[6]       = {};
        auto getSHRbits = [](uint32_t SHRNum, uint32_t &miMathCmdNum, uint32_t *SHRbit) {
            uint32_t x = SHRNum;
            uint32_t y = SHRNum;
            while (x > 0)
            {
                x &= (x - 1);
                SHRbit[miMathCmdNum] = y ^ x;
                y                    = x;
                miMathCmdNum++;
            }
        };
        getSHRbits(SHRNum, miMathCmdNum, SHRbit);

        for (uint32_t i = 0; i < miMathCmdNum; i++)
        {
            mhw::mi::MHW_MI_ALU_PARAMS aluParams[4] = {};
            uint32_t                   aluCount     = 0;
            //load shr bits to register4
            auto &miLoadRegImmParams      = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
            miLoadRegImmParams            = {};
            miLoadRegImmParams.dwData     = SHRbit[i];
            miLoadRegImmParams.dwRegister = mmioRegs->generalPurposeRegister4LoOffset;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

            //load1 srca, reg1
            aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
            aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCA;
            aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG0;
            ++aluCount;

            //load2 srcb, reg2
            aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
            aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCB;
            aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG4;
            ++aluCount;

            aluParams[aluCount].AluOpcode = MHW_MI_ALU_SHR;
            ++aluCount;

            aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
            aluParams[aluCount].Operand1  = MHW_MI_ALU_GPREG0;
            aluParams[aluCount].Operand2  = MHW_MI_ALU_ACCU;
            ++aluCount;

            auto &miMathParams          = m_miItf->MHW_GETPAR_F(MI_MATH)();
            miMathParams                = {};
            miMathParams.dwNumAluParams = aluCount;
            miMathParams.pAluPayload    = aluParams;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_MATH)(cmdBuffer));
        }

        //store VCS_GPR0_Lo metadata buffer
        auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = presDst;
        miStoreRegMemParams.dwOffset        = dstOffset;
        miStoreRegMemParams.dwRegister      = mmioRegs->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

        // Make Flush DW call to make sure all previous work is done
        auto &flushDwParams = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams       = {};
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::PrepareHWMetaDataFromStreamoutTileLevel(MOS_COMMAND_BUFFER *cmdBuffer, uint32_t tileCol, uint32_t tileRow) 
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        if (!m_basicFeature->m_resMetadataBuffer)
        {
            return MOS_STATUS_SUCCESS;
        }
        MetaDataOffset    resourceOffset    = m_basicFeature->m_metaDataOffset;

        //lcu count Intra/Inter/Skip CU Cnt
        PMOS_RESOURCE tileStatisticsPakStreamoutBuffer = m_basicFeature->m_tileStatisticsPakStreamoutBuffer;
        ENCODE_CHK_NULL_RETURN(tileStatisticsPakStreamoutBuffer);

        auto &miCpyMemMemParams       = m_miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
        auto &storeDataParams         = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        storeDataParams               = {};
        storeDataParams.pOsResource   = m_basicFeature->m_resMetadataBuffer;

        if (tileCol == 0 && tileRow == 0)
        {
            // LCUSkipIn8x8Unit
            miCpyMemMemParams.presSrc     = tileStatisticsPakStreamoutBuffer;
            miCpyMemMemParams.dwSrcOffset = 61 * sizeof(uint32_t);
            miCpyMemMemParams.presDst     = m_basicFeature->m_resMetadataBuffer;
            miCpyMemMemParams.dwDstOffset = resourceOffset.dwEncodeStats + resourceOffset.dwSkipCodingUnitsCount;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));

            // NumCU_IntraDC, NumCU_IntraPlanar, NumCU_IntraAngular
            miCpyMemMemParams.dwSrcOffset = 24 * sizeof(uint32_t);
            miCpyMemMemParams.dwDstOffset = resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));

            // NumCU_MVdirL0, NumCU_MVdirL1, NumCU_MVdirBi
            miCpyMemMemParams.dwSrcOffset = 85 * sizeof(uint32_t);
            miCpyMemMemParams.dwDstOffset = resourceOffset.dwEncodeStats + resourceOffset.dwInterCodingUnitsCount;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));
        }
        else
        {
            ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwSkipCodingUnitsCount, tileStatisticsPakStreamoutBuffer, 61 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));
            ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount, tileStatisticsPakStreamoutBuffer, 24 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));
            ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwInterCodingUnitsCount, tileStatisticsPakStreamoutBuffer, 85 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));
        }
        // NumCU_IntraDC, NumCU_IntraPlanar, NumCU_IntraAngular
        ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount, tileStatisticsPakStreamoutBuffer, 25 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount, tileStatisticsPakStreamoutBuffer, 26 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount, tileStatisticsPakStreamoutBuffer, 27 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount, tileStatisticsPakStreamoutBuffer, 28 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount, tileStatisticsPakStreamoutBuffer, 29 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount, tileStatisticsPakStreamoutBuffer, 30 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount, tileStatisticsPakStreamoutBuffer, 31 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount, tileStatisticsPakStreamoutBuffer, 32 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount, tileStatisticsPakStreamoutBuffer, 33 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount, tileStatisticsPakStreamoutBuffer, 34 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount, tileStatisticsPakStreamoutBuffer, 35 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount, tileStatisticsPakStreamoutBuffer, 36 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));

        // NumCU_MVdirL0, NumCU_MVdirL1, NumCU_MVdirBi
        ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwInterCodingUnitsCount, tileStatisticsPakStreamoutBuffer, 86 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(CalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwInterCodingUnitsCount, tileStatisticsPakStreamoutBuffer, 87 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD, cmdBuffer));

        // Average MV_X/MV_Y, report (0,0) as temp solution, later may need kernel involved
        storeDataParams.dwResourceOffset = resourceOffset.dwEncodeStats + resourceOffset.dwAverageMotionEstimationXDirection;
        storeDataParams.dwValue          = 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        storeDataParams.dwResourceOffset = resourceOffset.dwEncodeStats + resourceOffset.dwAverageMotionEstimationYDirection;
        storeDataParams.dwValue          = 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));
    
        return MOS_STATUS_SUCCESS;
    }

    inline MOS_STATUS Av1VdencPkt::CalAtomic(PMOS_RESOURCE presDst, uint32_t dstOffset, PMOS_RESOURCE presSrc, uint32_t srcOffset, mhw::mi::MHW_COMMON_MI_ATOMIC_OPCODE opCode, MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        auto  mmioRegisters      = m_hwInterface->GetVdencInterfaceNext()->GetMmioRegisters(m_vdboxIndex);
        ENCODE_CHK_NULL_RETURN(mmioRegisters);
        auto &miLoadRegMemParams = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        auto &flushDwParams      = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        auto &atomicParams       = m_miItf->MHW_GETPAR_F(MI_ATOMIC)();

        miLoadRegMemParams = {};
        flushDwParams      = {};
        atomicParams       = {};

        miLoadRegMemParams.presStoreBuffer = presSrc;
        miLoadRegMemParams.dwOffset        = srcOffset;
        miLoadRegMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        atomicParams.pOsResource      = presDst;
        atomicParams.dwResourceOffset = dstOffset;
        atomicParams.dwDataSize       = sizeof(uint32_t);
        atomicParams.Operation        = opCode;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_ATOMIC)(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS Av1VdencPkt::DumpInput()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_pipeline);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        debugInterface->m_DumpInputNum         = m_basicFeature->m_frameNum - 1;

        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_ref.GetCurrRefList());
        CODEC_REF_LIST currRefList             = *((CODEC_REF_LIST *)m_basicFeature->m_ref.GetCurrRefList());

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
            &currRefList.sRefRawBuffer,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "SrcSurf"));
        ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_ref.DumpInput(m_pipeline));

        return MOS_STATUS_SUCCESS;
    }
    MOS_STATUS Av1VdencPkt::DumpResources(EncodeStatusMfx *encodeStatusMfx, EncodeStatusReportData *statusReportData)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(encodeStatusMfx);
        ENCODE_CHK_NULL_RETURN(statusReportData);
        ENCODE_CHK_NULL_RETURN(m_pipeline);
        ENCODE_CHK_NULL_RETURN(m_statusReport);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_trackedBuf);

        CodechalDebugInterface *debugInterface = m_pipeline->GetStatusReportDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        CODEC_REF_LIST currRefList = *((CODEC_REF_LIST *)statusReportData->currRefList);
        currRefList.RefPic         = statusReportData->currOriginalPic;

        debugInterface->m_currPic            = statusReportData->currOriginalPic;
        debugInterface->m_bufferDumpFrameNum = m_statusReport->GetReportedCount();
        debugInterface->m_frameType          = encodeStatusMfx->pictureCodingType;

        if (m_resVDEncPakObjCmdStreamOutBuffer != nullptr)
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
                m_resVDEncPakObjCmdStreamOutBuffer,
                CodechalDbgAttr::attrPakObjStreamout,
                "_PakObj",
                m_basicFeature->m_mbCodeSize + 8 * CODECHAL_CACHELINE_SIZE,
                0,
                CODECHAL_NUM_MEDIA_STATES));
        }

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            &currRefList.resBitstreamBuffer,
            CodechalDbgAttr::attrBitstream,
            "_PAK",
            statusReportData->bitstreamSize,
            0,
            CODECHAL_NUM_MEDIA_STATES));

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpData(
            statusReportData,
            sizeof(EncodeStatusReportData),
            CodechalDbgAttr::attrStatusReport,
            "EncodeStatusReport_Buffer"));

        MOS_SURFACE *ds4xSurface = m_basicFeature->m_trackedBuf->GetSurface(
            BufferType::ds4xSurface, currRefList.ucScalingIdx);

        if (ds4xSurface != nullptr)
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
                ds4xSurface,
                CodechalDbgAttr::attrReconstructedSurface,
                "4xScaledSurf"))
        }

        MOS_SURFACE *ds8xSurface = m_basicFeature->m_trackedBuf->GetSurface(
            BufferType::ds8xSurface, currRefList.ucScalingIdx);

        if (ds8xSurface != nullptr)
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
                ds8xSurface,
                CodechalDbgAttr::attrReconstructedSurface,
                "8xScaledSurf"))
        }

        MOS_RESOURCE *mbCodedBuffer = m_basicFeature->m_trackedBuf->GetBuffer(
            BufferType::mbCodedBuffer, currRefList.ucScalingIdx);
        if (mbCodedBuffer != nullptr)
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
                mbCodedBuffer,
                CodechalDbgAttr::attrVdencOutput,
                "_MbCode",
                m_basicFeature->m_mbCodeSize,
                0,
                CODECHAL_NUM_MEDIA_STATES));
        }

        auto streamInBufferSize = (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32) * (MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 64) / 32) * CODECHAL_CACHELINE_SIZE;
        PMOS_RESOURCE streamInBuffer = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::StreamInBuffer, debugInterface->m_bufferDumpFrameNum);
        if (streamInBuffer != nullptr)
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
                streamInBuffer,
                CodechalDbgAttr::attrStreamIn,
                "StreamIn",
                streamInBufferSize,
                0,
                CODECHAL_NUM_MEDIA_STATES));
        }

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
            &currRefList.sRefReconBuffer,
            CodechalDbgAttr::attrReconstructedSurface,
            "ReconSurf",
            CODECHAL_NUM_MEDIA_STATES,
            m_basicFeature->m_frameWidth,
            m_basicFeature->m_frameHeight))

        MOS_SURFACE* postCdefSurface = m_basicFeature->m_trackedBuf->GetSurface(
            BufferType::postCdefReconSurface, currRefList.ucScalingIdx);
        if (postCdefSurface != nullptr)
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
                postCdefSurface,
                CodechalDbgAttr::attrReconstructedSurface,
                "PostCdefReconSurf",
                CODECHAL_NUM_MEDIA_STATES,
                m_basicFeature->m_frameWidth,
                m_basicFeature->m_frameHeight))
        }

        return MOS_STATUS_SUCCESS;
    }
    MOS_STATUS Av1VdencPkt::DumpStatistics()
    {
        CodechalDebugInterface *debugInterface =  m_pipeline->GetStatusReportDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            m_basicFeature->m_tileStatisticsPakStreamoutBuffer,
            CodechalDbgAttr::attrTileBasedStats,
            "Pak_Tile_Stats",
            512,
            0,
            CODECHAL_NUM_MEDIA_STATES));

        MOS_RESOURCE* tileStatisticsBuffer = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, FeatureIDs::encodeTile, GetTileBasedStatisticsBuffer, 0, tileStatisticsBuffer);
        ENCODE_CHK_NULL_RETURN(tileStatisticsBuffer);
        uint32_t offset = 0;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileStatsOffset, offset);

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            tileStatisticsBuffer,
            CodechalDbgAttr::attrFrameState,
            "VDEnc_Frame_Stats",
            m_hwInterface->m_pakIntTileStatsSize,
            offset,
            CODECHAL_NUM_MEDIA_STATES));

        MOS_RESOURCE *pakinfo = m_basicFeature->m_recycleBuf->GetBuffer(PakInfo, 0);
        ENCODE_CHK_NULL_RETURN(pakinfo);
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            pakinfo,
            CodechalDbgAttr::attrFrameState,
            "VDEnc_PAK_INFO",
            MOS_ALIGN_CEIL(sizeof(Av1VdencPakInfo), CODECHAL_PAGE_SIZE),
            0,
            CODECHAL_NUM_MEDIA_STATES));

        return MOS_STATUS_SUCCESS;
    }
#endif

}  // namespace encode
