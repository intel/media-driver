/*
* Copyright (c) 2020-2023, Intel Corporation
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
//! \file     encode_avc_vdenc_packet.cpp
//! \brief    Defines the interface for avc encode vdenc packet
//!

#include "encode_avc_vdenc_packet.h"
#include "encode_avc_vdenc_weighted_prediction.h"
#include "encode_avc_vdenc_stream_in_feature.h"
#include "encode_avc_rounding.h"
#include "encode_avc_brc.h"
#include "encode_avc_vdenc_const_settings.h"
#include "media_avc_feature_defs.h"
#include "mos_solo_generic.h"
#include "encode_avc_header_packer.h"
#include "media_perf_profiler.h"
#include "mos_os_cp_interface_specific.h"
#include "hal_oca_interface_next.h"

#include "media_packet.h"

#define CODEC_AVC_MIN_BLOCK_HEIGHT 16

namespace encode {

    AvcVdencPkt::AvcVdencPkt(
        MediaPipeline *pipeline,
        MediaTask *task,
        CodechalHwInterfaceNext *hwInterface) :
        CmdPacket(task),
        m_pipeline(dynamic_cast<AvcVdencPipeline *>(pipeline)),
        m_hwInterface(dynamic_cast<CodechalHwInterfaceNext *>(hwInterface))
    {
        ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_pipeline);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);

        m_osInterface    = hwInterface->GetOsInterface();
        m_statusReport   = m_pipeline->GetStatusReportInstance();
        m_legacyFeatureManager = m_pipeline->GetFeatureManager();
        m_featureManager = m_pipeline->GetPacketLevelFeatureManager(AvcVdencPipeline::VdencPacket);
        m_encodecp       = m_pipeline->GetEncodeCp();
        m_vdencItf       = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(m_hwInterface->GetVdencInterfaceNext());
        m_miItf          = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
        m_mfxItf         = std::static_pointer_cast<mhw::vdbox::mfx::Itf>(m_hwInterface->GetMfxInterfaceNext());
    }

    AvcVdencPkt::~AvcVdencPkt()
    {
        FreeResources();
    }

    MOS_STATUS AvcVdencPkt::FreeResources()
    {
        ENCODE_FUNC_CALL();

        if (m_vdencBrcImgStatAllocated)
        {
            for (uint8_t i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
            {
                ENCODE_CHK_STATUS_RETURN(Mhw_FreeBb(m_osInterface, &m_batchBufferForVdencImgStat[i], nullptr));
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::Init()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_statusReport);

        ENCODE_CHK_STATUS_RETURN(CmdPacket::Init());

        m_basicFeature = dynamic_cast<AvcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

#ifdef _MMC_SUPPORTED
        m_mmcState = m_pipeline->GetMmcState();
        ENCODE_CHK_NULL_RETURN(m_mmcState);
        m_basicFeature->m_mmcState = m_mmcState;
#endif
        m_allocator = m_pipeline->GetEncodeAllocator();
        ENCODE_CHK_STATUS_RETURN(AllocateResources());

        ENCODE_CHK_STATUS_RETURN(m_statusReport->RegistObserver(this));

        m_usePatchList = m_osInterface->bUsesPatchList;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::Prepare()
    {
        ENCODE_FUNC_CALL();

        AvcVdencPipeline *pipeline = dynamic_cast<AvcVdencPipeline *>(m_pipeline);
        ENCODE_CHK_NULL_RETURN(pipeline);

        m_seqParam    = m_basicFeature->m_seqParam;
        m_picParam    = m_basicFeature->m_picParam;
        m_sliceParams = m_basicFeature->m_sliceParams;

        ENCODE_CHK_STATUS_RETURN(ValidateVdboxIdx(m_vdboxIndex));
        ENCODE_CHK_STATUS_RETURN(SetRowstoreCachingOffsets());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::SetRowstoreCachingOffsets()
    {
        ENCODE_CHK_NULL_RETURN(m_mfxItf);
        // Get row store cache offset as all the needed information is got here
        if (m_mfxItf->IsRowStoreCachingSupported())
        {
            MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
            MOS_ZeroMemory(&rowstoreParams, sizeof(rowstoreParams));
            rowstoreParams.Mode       = CODECHAL_ENCODE_MODE_AVC;
            rowstoreParams.dwPicWidth = m_basicFeature->m_frameWidth;
            rowstoreParams.bIsFrame   = (m_seqParam->frame_mbs_only_flag == 1);
            rowstoreParams.ucChromaFormat = m_basicFeature->m_chromaFormat;
            ENCODE_CHK_STATUS_RETURN(m_hwInterface->SetRowstoreCachingOffsets(&rowstoreParams));

            if (m_vdencItf)
            {
                mhw::vdbox::vdenc::RowStorePar par = {};

                par.mode    = mhw::vdbox::vdenc::RowStorePar::AVC;
                par.isField = (m_seqParam->frame_mbs_only_flag != 1);

                ENCODE_CHK_STATUS_RETURN(m_vdencItf->SetRowstoreCachingOffsets(par));
            }
            if (m_mfxItf)
            {
                ENCODE_CHK_STATUS_RETURN(m_mfxItf->GetRowstoreCachingAddrs(&rowstoreParams));
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::Destroy()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_STATUS_RETURN(m_statusReport->UnregistObserver(this));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::Submit(
        MOS_COMMAND_BUFFER* commandBuffer,
        uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        MOS_COMMAND_BUFFER& cmdBuffer = *commandBuffer;
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

        ENCODE_CHK_STATUS_RETURN(PatchPictureLevelCommands(packetPhase, cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(PatchSliceLevelCommands(cmdBuffer, packetPhase));

        ENCODE_CHK_STATUS_RETURN(Mos_Solo_PostProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::AllocateResources()
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_allocator);

        auto settings = static_cast<AvcVdencFeatureSettings *>(m_legacyFeatureManager->GetFeatureSettings()->GetConstSettings());
        ENCODE_CHK_NULL_RETURN(settings);

        auto brcSettings = settings->brcSettings;

        // initiate allocation parameters and lock flags
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;

        // PAK Slice Size Streamout Buffer
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(CODECHAL_ENCODE_SLICESIZE_BUF_SIZE, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "PAK Slice Size Streamout Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_recycleBuf->RegisterResource(PakSliceSizeStreamOutBuffer, allocParamsForBufferLinear));

        // VDENC Intra Row Store Scratch buffer
        // 1 cacheline per MB
        allocParamsForBufferLinear.dwBytes  = m_basicFeature->m_picWidthInMb * CODECHAL_CACHELINE_SIZE;
        allocParamsForBufferLinear.pBufName = "VDENC Intra Row Store Scratch Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
        m_vdencIntraRowStoreScratch         = m_allocator->AllocateResource(allocParamsForBufferLinear, false);

        // PAK Statistics buffer
        allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(brcSettings.vdencBrcPakStatsBufferSize, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDENC BRC PAK Statistics Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_recycleBuf->RegisterResource(BrcPakStatisticBuffer, allocParamsForBufferLinear, 1));

        // Here allocate the buffer for MB+FrameLevel PAK statistics.
        MOS_ALLOC_GFXRES_PARAMS allocParamsForStatisticBufferFull = allocParamsForBufferLinear;
        uint32_t size = brcSettings.vdencBrcPakStatsBufferSize + m_basicFeature->m_picWidthInMb * m_basicFeature->m_picHeightInMb * 64;
        allocParamsForStatisticBufferFull.dwBytes = MOS_ALIGN_CEIL(size, CODECHAL_PAGE_SIZE);
        allocParamsForStatisticBufferFull.pBufName = "VDENC BRC PAK Statistics Buffer Full";
        allocParamsForStatisticBufferFull.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        if (m_osInterface->osCpInterface == nullptr || !m_osInterface->osCpInterface->IsCpEnabled())
        {
            allocParamsForStatisticBufferFull.dwMemType = MOS_MEMPOOL_SYSTEMMEMORY;
            allocParamsForStatisticBufferFull.Flags.bCacheable = true;
        }
        ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_recycleBuf->RegisterResource(BrcPakStatisticBufferFull, allocParamsForStatisticBufferFull));

        if (m_mfxItf->IsDeblockingFilterRowstoreCacheEnabled() == false)
        {
            // Deblocking Filter Row Store Scratch buffer
            allocParamsForBufferLinear.dwBytes  = m_basicFeature->m_picWidthInMb * 4 * CODECHAL_CACHELINE_SIZE; // 4 cachelines per MB
            allocParamsForBufferLinear.pBufName = "Deblocking Filter Row Store Scratch Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
            m_resDeblockingFilterRowStoreScratchBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
        }

        if (m_mfxItf->IsIntraRowstoreCacheEnabled() == false)
        {
            // Intra Row Store Scratch buffer
            // 1 cacheline per MB
            allocParamsForBufferLinear.dwBytes  = m_basicFeature->m_picWidthInMb * CODECHAL_CACHELINE_SIZE;
            allocParamsForBufferLinear.pBufName = "Intra Row Store Scratch Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
            m_intraRowStoreScratchBuffer        = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
        }

        if (m_mfxItf->IsBsdMpcRowstoreCacheEnabled() == false)
        {
            // MPC Row Store Scratch buffer
            allocParamsForBufferLinear.dwBytes  = m_basicFeature->m_picWidthInMb * 2 * 64; // 2 cachelines per MB
            allocParamsForBufferLinear.pBufName = "MPC Row Store Scratch Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
            m_resMPCRowStoreScratchBuffer       = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
        }

        auto brcFeature = dynamic_cast<AvcEncodeBRC*>(m_featureManager->GetFeature(AvcFeatureIDs::avcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);

        // ToDo: we always go to BRC disabld case because do not know RCM here. Same to legacy implementation
        // VDENC uses second level batch buffer for image state cmds
        if (!brcFeature->IsVdencBrcEnabled())
        {
            // CQP mode needs a set of buffers for concurrency between SFD and VDEnc
            for (uint8_t i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
            {
                MOS_ZeroMemory(
                    &m_batchBufferForVdencImgStat[i],
                    sizeof(m_batchBufferForVdencImgStat[i]));
                m_batchBufferForVdencImgStat[i].bSecondLevel = true;
                ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
                    m_osInterface,
                    &m_batchBufferForVdencImgStat[i],
                    nullptr,
                    m_hwInterface->m_vdencBrcImgStateBufferSize));
            }
            m_vdencBrcImgStatAllocated = true;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER  &cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_trackedBuf);
        ENCODE_CHK_NULL_RETURN(m_seqParam);

        // Set flag bIsMdfLoad in remote gaming scenario to boost GPU frequency for low latency
        cmdBuffer.Attributes.bFrequencyBoost = (m_seqParam->ScenarioInfo == ESCENARIO_REMOTEGAMING);

        ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(m_basicFeature->m_frameWidth, m_basicFeature->m_frameHeight, true));

        SetPerfTag(m_pipeline->IsFirstPass() ? CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE : CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE_SECOND_PASS,
            (uint16_t)m_basicFeature->m_mode,
            m_basicFeature->m_pictureCodingType);

        auto brcFeature = dynamic_cast<AvcEncodeBRC*>(m_featureManager->GetFeature(AvcFeatureIDs::avcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);

        if (!m_pipeline->IsSingleTaskPhaseSupported() || (m_pipeline->IsFirstPass() && !brcFeature->IsVdencBrcEnabled()))
        {
            SETPAR_AND_ADDCMD(MI_FORCE_WAKEUP, m_miItf, &cmdBuffer);

            // Send command buffer header at the beginning (OS dependent)
            ENCODE_CHK_STATUS_RETURN(SendPrologCmds(cmdBuffer));
        }

        if (brcFeature->IsVdencBrcEnabled())
        {
#if _SW_BRC
            if (!brcFeature->m_swBrc)
            {
#endif
            // Insert conditional batch buffer end for HuC valid IMEM loaded check
            m_pResource = brcFeature->GetHucStatus2Buffer();
            SETPAR_AND_ADDCMD(MI_CONDITIONAL_BATCH_BUFFER_END, m_miItf, &cmdBuffer);
#if _SW_BRC
            }
#endif
        }

        if (m_pipeline->GetCurrentPass())
        {
            if ((Mos_Solo_Extension((MOS_CONTEXT_HANDLE)m_osInterface->pOsContext) || m_osInterface->bInlineCodecStatusUpdate)
                && brcFeature->IsVdencBrcEnabled())
            {
                // increment dwStoreData conditionaly
                ENCODE_CHK_STATUS_RETURN(MediaPacket::UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));
            }

            // Insert conditional batch buffer end
            // VDENC uses HuC BRC FW generated semaphore for conditional 2nd pass
            m_pResource =
                m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
            SETPAR_AND_ADDCMD(MI_CONDITIONAL_BATCH_BUFFER_END, m_miItf, &cmdBuffer);
        }

        if (m_pipeline->IsFirstPipe())
        {
            ENCODE_CHK_STATUS_RETURN(StartStatusReport(statusReportMfx, &cmdBuffer));
        }

        SETPAR_AND_ADDCMD(VDENC_CONTROL_STATE, m_vdencItf, &cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(AddPictureMfxCommands(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(AddPictureVdencCommands(cmdBuffer));

        PMHW_BATCH_BUFFER secondLevelBatchBufferUsed = nullptr;

        // VDENC CQP case
        if (!brcFeature->IsVdencBrcEnabled())
        {
            // VDENC case uses multiple buffers for concurrency between SFD and VDENC
            secondLevelBatchBufferUsed = &(m_batchBufferForVdencImgStat[m_pipeline->m_currRecycledBufIdx]);

            // CQP case, driver programs the 2nd Level BB
            MOS_STATUS status = Mhw_LockBb(m_osInterface, secondLevelBatchBufferUsed);
            if (status != MOS_STATUS_SUCCESS)
            {
                ENCODE_NORMALMESSAGE("ERROR - Recycled buffer index exceed the maximum");
                SETPAR_AND_ADDCMD(MI_BATCH_BUFFER_END, m_miItf, &cmdBuffer);
                return status;
            }

            SETPAR_AND_ADDCMD(MFX_AVC_IMG_STATE, m_mfxItf, nullptr, secondLevelBatchBufferUsed);
            SETPAR_AND_ADDCMD(VDENC_CMD3, m_vdencItf, nullptr, secondLevelBatchBufferUsed);
            SETPAR_AND_ADDCMD(VDENC_AVC_IMG_STATE, m_vdencItf, nullptr, secondLevelBatchBufferUsed);

            ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, secondLevelBatchBufferUsed));

            CODECHAL_DEBUG_TOOL(
                ENCODE_CHK_STATUS_RETURN(PopulatePakParam(
                    nullptr,
                    secondLevelBatchBufferUsed));

                ENCODE_CHK_STATUS_RETURN(PopulateEncParam(
                    0,
                    nullptr));

                ENCODE_CHK_STATUS_RETURN(DumpEncodeImgStats(nullptr));
            )

            ENCODE_CHK_STATUS_RETURN(Mhw_UnlockBb(m_osInterface, secondLevelBatchBufferUsed, true));
        }
        else
        {
            secondLevelBatchBufferUsed = brcFeature->GetBatchBufferForVdencImgStat();
            // current location to add cmds in 2nd level batch buffer
            secondLevelBatchBufferUsed->iCurrent = 0;
            // reset starting location (offset) executing 2nd level batch buffer for each frame & each pass
            secondLevelBatchBufferUsed->dwOffset = 0;
        }

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

        ENCODE_CHK_STATUS_RETURN(AddAllCmds_MFX_QM_STATE(&cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(AddAllCmds_MFX_FQM_STATE(&cmdBuffer));

        if (m_basicFeature->m_pictureCodingType == B_TYPE)
        {
            SETPAR_AND_ADDCMD(MFX_AVC_DIRECTMODE_STATE, m_mfxItf, &cmdBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::PatchSliceLevelCommands(MOS_COMMAND_BUFFER  &cmdBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        auto slcData = m_basicFeature->m_slcData;

        // For use with the single task phase implementation
        if (m_basicFeature->m_sliceStructCaps < CODECHAL_SLICE_STRUCT_ARBITRARYROWSLICE)
        {
            uint32_t numSlc = (m_basicFeature->m_frameFieldHeightInMb + m_basicFeature->m_sliceHeight - 1) / m_basicFeature->m_sliceHeight;
            if (numSlc != m_basicFeature->m_numSlices)
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }

        ENCODE_CHK_STATUS_RETURN(LockBatchBufferForPakSlices());

        CODECHAL_DEBUG_TOOL(
            ENCODE_CHK_STATUS_RETURN(SetSliceStateCommonParams(m_basicFeature->sliceState)))

        for (uint16_t slcCount = 0; slcCount < m_basicFeature->m_numSlices; slcCount++)
        {
            m_basicFeature->m_curNumSlices = slcCount;
            if (m_pipeline->IsFirstPass())
            {
                if (m_basicFeature->m_acceleratorHeaderPackingCaps)
                {
                    slcData[slcCount].SliceOffset = m_basicFeature->m_bsBuffer.SliceOffset;
                    ENCODE_CHK_STATUS_RETURN(PackSliceHeader(slcCount));
                    slcData[slcCount].BitSize = m_basicFeature->m_bsBuffer.BitSize;
                }
                if (m_basicFeature->m_sliceStructCaps != CODECHAL_SLICE_STRUCT_ARBITRARYMBSLICE)
                {
                    slcData[slcCount].CmdOffset = slcCount * m_basicFeature->m_sliceHeight * m_basicFeature->m_picWidthInMb * 16 * 4;
                }
                else
                {
                    slcData[slcCount].CmdOffset = m_sliceParams[slcCount].first_mb_in_slice * 16 * 4;
                }
            }

            CODECHAL_DEBUG_TOOL(
                ENCODE_CHK_STATUS_RETURN(SetSliceStateParams(m_basicFeature->sliceState, slcData, slcCount)))

            ENCODE_CHK_STATUS_RETURN(SendSlice(&cmdBuffer));
            ENCODE_CHK_STATUS_RETURN(ReportSliceSizeMetaData(&cmdBuffer, slcCount));

            m_lastSlice = (slcCount == (m_basicFeature->m_numSlices) - 1);
            SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, &cmdBuffer);

            // Do not send MI_FLUSH for last Super slice now
            if (!m_lastSlice)
            {
                SETPAR_AND_ADDCMD(MI_FLUSH_DW, m_miItf, &cmdBuffer);
            }
        }

        ENCODE_CHK_STATUS_RETURN(UnlockBatchBufferForPakSlices());

        // Insert end of sequence/stream if set
        if (m_basicFeature->m_lastPicInStream || m_basicFeature->m_lastPicInSeq)
        {
            m_lastPic = true;
            ENCODE_CHK_STATUS_RETURN(InsertSeqStreamEnd(cmdBuffer));
            m_lastPic = false;
        }

        ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(PrepareHWMetaData(&cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(ReadMfcStatus(cmdBuffer));

        auto brcFeature = dynamic_cast<AvcEncodeBRC*>(m_featureManager->GetFeature(AvcFeatureIDs::avcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);

        if (brcFeature->IsVdencBrcEnabled())
        {
            ENCODE_CHK_STATUS_RETURN(StoreNumPasses(cmdBuffer));

#if USE_CODECHAL_DEBUG_TOOL
            uint32_t sizeInByte      = 0;
            bool     isIframe        = m_basicFeature->m_pictureCodingType == I_TYPE;
            auto     packetUtilities = m_pipeline->GetPacketUtilities();
            ENCODE_CHK_NULL_RETURN(packetUtilities);
            EncodeStatusReadParams params;
            MOS_ZeroMemory(&params, sizeof(params));
            RUN_FEATURE_INTERFACE_RETURN(AvcEncodeBRC, AvcFeatureIDs::avcBrcFeature, SetMfcStatusParams, params);
            if (packetUtilities->GetFakeHeaderSettings(sizeInByte, isIframe))
            {
                ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);
                ENCODE_CHK_STATUS_RETURN(packetUtilities->ModifyEncodedFrameSizeWithFakeHeaderSizeAVC(
                    &cmdBuffer,
                    sizeInByte,
                    params.resVdencBrcUpdateDmemBufferPtr,
                    0,
                    m_basicFeature->m_recycleBuf->GetBuffer(BrcPakStatisticBuffer, m_basicFeature->m_frameNum),
                    sizeof(uint32_t) * 4));
            }
#endif
        }

        if (m_picParam->StatusReportEnable.fields.FrameStats)
        {
            ENCODE_CHK_STATUS_RETURN(GetAvcVdencFrameLevelStatusExt(m_picParam->StatusReportFeedbackNumber, &cmdBuffer));
        }

        ENCODE_CHK_STATUS_RETURN(EndStatusReport(statusReportMfx, &cmdBuffer));

        if (m_pipeline->IsLastPass() && m_pipeline->IsFirstPipe())
        {
            // increment dwStoreData conditionaly
            MediaPacket::UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer);

            CODECHAL_DEBUG_TOOL(m_mmcState->UpdateUserFeatureKey(&(m_basicFeature->m_reconSurface)));
        }

        // Reset parameters for next PAK execution
        if (m_pipeline->IsLastPass())
        {
            UpdateParameters();
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::ValidateVdboxIdx(const MHW_VDBOX_NODE_IND &vdboxIndex)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        if (vdboxIndex > m_mfxItf->GetMaxVdboxIndex())
        {
            //ENCODE_ASSERTMESSAGE("ERROR - vdbox index exceed the maximum");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
        }

        return eStatus;
    }

    MOS_STATUS AvcVdencPkt::PrepareHWMetaData(PMOS_COMMAND_BUFFER cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        PMOS_RESOURCE presMetadataBuffer = m_basicFeature->m_resMetadataBuffer;
        MetaDataOffset resourceOffset    = m_basicFeature->m_metaDataOffset;
        if ((presMetadataBuffer == nullptr) || !m_pipeline->IsLastPass())
        {
            return MOS_STATUS_SUCCESS;
        }

        m_pResource = presMetadataBuffer;
        m_dwOffset  = resourceOffset.dwEncodeErrorFlags;
        m_dwValue   = 0;
        SETPAR_AND_ADDCMD(MI_STORE_DATA_IMM, m_miItf, cmdBuffer);

        m_dwOffset = resourceOffset.dwWrittenSubregionsCount;
        m_dwValue  = m_basicFeature->m_numSlices;
        SETPAR_AND_ADDCMD(MI_STORE_DATA_IMM, m_miItf, cmdBuffer);

        ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_mfxItf->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");
        MmioRegistersMfx *mmioRegisters = SelectVdboxAndGetMmioRegister(m_vdboxIndex, cmdBuffer);
        CODEC_HW_CHK_NULL_RETURN(mmioRegisters);

        m_dwOffset = resourceOffset.dwEncodedBitstreamWrittenBytesCount;
        m_dwValue  = mmioRegisters->mfcBitstreamBytecountFrameRegOffset;
        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, cmdBuffer);

        // Statistics
        // Average QP
        if (m_seqParam->RateControlMethod == RATECONTROL_CQP)
        {
            m_dwOffset = resourceOffset.dwEncodeStats + resourceOffset.dwAverageQP;
            m_dwValue  = m_picParam->QpY + m_sliceParams->slice_qp_delta;
            SETPAR_AND_ADDCMD(MI_STORE_DATA_IMM, m_miItf, cmdBuffer);
        }
        else
        {
            ENCODE_NORMALMESSAGE("RC mode is temporarily not supported");
        }

        // PAK frame status pointer
        MOS_RESOURCE *pPakFrameStat = (m_basicFeature->m_perMBStreamOutEnable) ?
            m_basicFeature->m_recycleBuf->GetBuffer(BrcPakStatisticBufferFull, m_basicFeature->m_frameNum) :
            m_basicFeature->m_recycleBuf->GetBuffer(BrcPakStatisticBuffer, 0);

        auto &miLoadRegImmParams = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
        auto &miLoadRegMemParams = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        auto &miLoadRegRegParams = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_REG)();
        auto &flushDwParams      = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();

        // Intra/Inter/Skip statistics counted by number of MBs (not sub-blocks)

        // Intra16x16 + Intra8x8 + Intra4x4
        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwRegister = mmioRegisters->generalPurposeRegister4LoOffset;
        miLoadRegImmParams.dwData     = 0xFFFF0000;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwData     = 0;
        miLoadRegImmParams.dwRegister = mmioRegisters->generalPurposeRegister4HiOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        // DW4 Intra16x16:Intra8x8
        miLoadRegMemParams                 = {};
        miLoadRegMemParams.presStoreBuffer = pPakFrameStat;
        miLoadRegMemParams.dwOffset        = 4 * sizeof(uint32_t);
        miLoadRegMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwRegister = mmioRegisters->generalPurposeRegister0HiOffset;
        miLoadRegImmParams.dwData     = 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        mhw::mi::MHW_MI_ALU_PARAMS aluParams[4 + 16 * 4];
        int                        aluCount;

        auto Reg0OpReg4ToReg0 = [&](MHW_MI_ALU_OPCODE opCode) {
            aluCount = 0;
            // load  SrcA, reg0
            aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
            aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCA;
            aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG0;
            ++aluCount;
            // load  SrcB, reg4
            aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
            aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCB;
            aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG4;
            ++aluCount;
            // and   SrcA, SrcB
            aluParams[aluCount].AluOpcode = opCode;
            ++aluCount;

            // store reg0, accu
            aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
            aluParams[aluCount].Operand1  = MHW_MI_ALU_GPREG0;
            aluParams[aluCount].Operand2  = MHW_MI_ALU_ACCU;
            ++aluCount;

            auto &miMathParams          = m_miItf->MHW_GETPAR_F(MI_MATH)();
            miMathParams                = {};
            miMathParams.dwNumAluParams = aluCount;
            miMathParams.pAluPayload    = aluParams;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_MATH)(cmdBuffer));

            return MOS_STATUS_SUCCESS;
        };

        ENCODE_CHK_STATUS_RETURN(Reg0OpReg4ToReg0(MHW_MI_ALU_AND));  // reg0 0:0:intra16x16:0

        // DW5 Intra4x4:Inter16x16
        miLoadRegMemParams                 = {};
        miLoadRegMemParams.presStoreBuffer = pPakFrameStat;
        miLoadRegMemParams.dwOffset        = 5 * sizeof(uint32_t);
        miLoadRegMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwRegister = mmioRegisters->generalPurposeRegister4HiOffset;
        miLoadRegImmParams.dwData     = 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));  // reg4 0:0:intra4x4:inter16x16(garb)

        auto AddHighShortsOfReg0Reg4ToReg0 = [&]() {
            aluCount = 0;
            // load  SrcA, reg0
            aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
            aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCA;
            aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG0;
            ++aluCount;
            // load  SrcB, reg4
            aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
            aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCB;
            aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG4;
            ++aluCount;
            // add   SrcA, SrcB
            aluParams[aluCount].AluOpcode = MHW_MI_ALU_ADD;
            ++aluCount;

            // ACCU keeps now 0:0:reg0+reg4:0

            // 16bit shift left
            for (int i = 0; i < 16; ++i)
            {
                // store reg0, accu
                aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
                aluParams[aluCount].Operand1  = MHW_MI_ALU_GPREG0;
                aluParams[aluCount].Operand2  = MHW_MI_ALU_ACCU;
                ++aluCount;
                // load SrcA, accu
                aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
                aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCA;
                aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG0;
                ++aluCount;
                // load SrcB, accu
                aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
                aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCB;
                aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG0;
                ++aluCount;
                // add  SrcA, SrcB
                aluParams[aluCount].AluOpcode = MHW_MI_ALU_ADD;
                ++aluCount;
            }

            // store reg0, accu
            aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
            aluParams[aluCount].Operand1  = MHW_MI_ALU_GPREG0;
            aluParams[aluCount].Operand2  = MHW_MI_ALU_ACCU;
            ++aluCount;

            auto &miMathParams          = m_miItf->MHW_GETPAR_F(MI_MATH)();
            miMathParams                = {};
            miMathParams.dwNumAluParams = aluCount;
            miMathParams.pAluPayload    = aluParams;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_MATH)(cmdBuffer));

            // move from reg0hi to reg0lo
            miLoadRegRegParams               = {};
            miLoadRegRegParams.dwSrcRegister = mmioRegisters->generalPurposeRegister0HiOffset;
            miLoadRegRegParams.dwDstRegister = mmioRegisters->generalPurposeRegister0LoOffset;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_REG)(cmdBuffer));

            miLoadRegImmParams            = {};
            miLoadRegImmParams.dwData     = 0;
            miLoadRegImmParams.dwRegister = mmioRegisters->generalPurposeRegister0HiOffset;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

            return MOS_STATUS_SUCCESS;
        };

        ENCODE_CHK_STATUS_RETURN(AddHighShortsOfReg0Reg4ToReg0());  // reg0 0:0:(Intra4x4+Intra16x16).hi:(Intra4x4+Intra16x16).lo

        // Temp store from reg0 to presMetadataBuffer
        m_pResource = presMetadataBuffer;
        m_dwOffset  = resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount;
        m_dwValue   = mmioRegisters->generalPurposeRegister0LoOffset;
        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, cmdBuffer);

        // DW4 Intra16x16:Intra8x8
        miLoadRegMemParams                 = {};
        miLoadRegMemParams.presStoreBuffer = pPakFrameStat;
        miLoadRegMemParams.dwOffset        = 4 * sizeof(uint32_t);
        miLoadRegMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwRegister = mmioRegisters->generalPurposeRegister4LoOffset;
        miLoadRegImmParams.dwData     = 0x0000FFFF;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwData     = 0;
        miLoadRegImmParams.dwRegister = mmioRegisters->generalPurposeRegister4HiOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(Reg0OpReg4ToReg0(MHW_MI_ALU_AND));  // reg0 0:0:0:Intra8x8

        flushDwParams = {};
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        miLoadRegMemParams                 = {};
        miLoadRegMemParams.presStoreBuffer = presMetadataBuffer;
        miLoadRegMemParams.dwOffset        = resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount;
        miLoadRegMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(Reg0OpReg4ToReg0(MHW_MI_ALU_ADD));

        // Store from reg0 to presMetadataBuffer
        m_pResource = presMetadataBuffer;
        m_dwOffset  = resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount;
        m_dwValue   = mmioRegisters->generalPurposeRegister0LoOffset;
        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, cmdBuffer);

        // Inter16x16 + Inter16x8 + Inter8x16 + Inter8x8
        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwRegister = mmioRegisters->generalPurposeRegister4LoOffset;
        miLoadRegImmParams.dwData     = 0xFFFF0000;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwData     = 0;
        miLoadRegImmParams.dwRegister = mmioRegisters->generalPurposeRegister4HiOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        // DW6 Inter16x8:Inter8x16
        miLoadRegMemParams                 = {};
        miLoadRegMemParams.presStoreBuffer = pPakFrameStat;
        miLoadRegMemParams.dwOffset        = 6 * sizeof(uint32_t);
        miLoadRegMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwRegister = mmioRegisters->generalPurposeRegister0HiOffset;
        miLoadRegImmParams.dwData     = 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(Reg0OpReg4ToReg0(MHW_MI_ALU_AND));  // reg0 0:0:inter16x8:0

        // DW7 Inter8x8:InterSkip16x16
        miLoadRegMemParams                 = {};
        miLoadRegMemParams.presStoreBuffer = pPakFrameStat;
        miLoadRegMemParams.dwOffset        = 7 * sizeof(uint32_t);
        miLoadRegMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwRegister = mmioRegisters->generalPurposeRegister4HiOffset;
        miLoadRegImmParams.dwData     = 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));  // reg4 0:0:inter8x8:0

        ENCODE_CHK_STATUS_RETURN(AddHighShortsOfReg0Reg4ToReg0());  // reg0 0:0:(Inter16x8+Inter8x8).hi:(Inter16x8+Inter8x8).lo;

        // Temp store from reg0 to presMetadataBuffer
        m_pResource = presMetadataBuffer;
        m_dwOffset  = resourceOffset.dwEncodeStats + resourceOffset.dwInterCodingUnitsCount;
        m_dwValue   = mmioRegisters->generalPurposeRegister0LoOffset;
        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, cmdBuffer);

        // DW6 Inter16x8:Inter8x16
        miLoadRegMemParams                 = {};
        miLoadRegMemParams.presStoreBuffer = pPakFrameStat;
        miLoadRegMemParams.dwOffset        = 6 * sizeof(uint32_t);
        miLoadRegMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0HiOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // DW5 Intra4x4 : Inter16x16
        miLoadRegMemParams                 = {};
        miLoadRegMemParams.presStoreBuffer = pPakFrameStat;
        miLoadRegMemParams.dwOffset        = 5 * sizeof(uint32_t);
        miLoadRegMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwRegister = mmioRegisters->generalPurposeRegister4LoOffset;
        miLoadRegImmParams.dwData     = 0x0000FFFF;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwRegister = mmioRegisters->generalPurposeRegister4HiOffset;
        miLoadRegImmParams.dwData     = 0x0000FFFF;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(Reg0OpReg4ToReg0(MHW_MI_ALU_AND));  // reg0 0:Inter8x16:0:Inter16x16

        // move from reg0hi to reg4lo
        miLoadRegRegParams               = {};
        miLoadRegRegParams.dwSrcRegister = mmioRegisters->generalPurposeRegister0HiOffset;
        miLoadRegRegParams.dwDstRegister = mmioRegisters->generalPurposeRegister4LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_REG)(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(Reg0OpReg4ToReg0(MHW_MI_ALU_ADD));  // reg0 0:0:(Inter8x16+Inter16x16).hi::(Inter8x16+Inter16x16).hi

        flushDwParams = {};
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        miLoadRegMemParams                 = {};
        miLoadRegMemParams.presStoreBuffer = presMetadataBuffer;
        miLoadRegMemParams.dwOffset        = resourceOffset.dwEncodeStats + resourceOffset.dwInterCodingUnitsCount;
        miLoadRegMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(Reg0OpReg4ToReg0(MHW_MI_ALU_ADD));

        // Store from reg0 to presMetadataBuffer
        m_pResource = presMetadataBuffer;
        m_dwOffset  = resourceOffset.dwEncodeStats + resourceOffset.dwInterCodingUnitsCount;
        m_dwValue   = mmioRegisters->generalPurposeRegister0LoOffset;
        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, cmdBuffer);

        // Inter skip 16x16
        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwRegister = mmioRegisters->generalPurposeRegister4LoOffset;
        miLoadRegImmParams.dwData     = 0x0000FFFF;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        miLoadRegImmParams            = {};
        miLoadRegImmParams.dwData     = 0;
        miLoadRegImmParams.dwRegister = mmioRegisters->generalPurposeRegister4HiOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

        // DW7 Inter8x8:InterSkip16x16
        miLoadRegMemParams                 = {};
        miLoadRegMemParams.presStoreBuffer = pPakFrameStat;
        miLoadRegMemParams.dwOffset        = 7 * sizeof(uint32_t);
        miLoadRegMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(Reg0OpReg4ToReg0(MHW_MI_ALU_AND));

        // Store from reg0 to presMetadataBuffer
        m_pResource = presMetadataBuffer;
        m_dwOffset  = resourceOffset.dwEncodeStats + resourceOffset.dwSkipCodingUnitsCount;
        m_dwValue   = mmioRegisters->generalPurposeRegister0LoOffset;
        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, cmdBuffer);

        // Average MV_X/MV_Y, report (0,0) as temp solution, later may need kernel involved
        m_dwOffset = resourceOffset.dwEncodeStats + resourceOffset.dwAverageMotionEstimationXDirection;
        m_dwValue  = 0;
        SETPAR_AND_ADDCMD(MI_STORE_DATA_IMM, m_miItf, cmdBuffer);

        m_dwOffset = resourceOffset.dwEncodeStats + resourceOffset.dwAverageMotionEstimationYDirection;
        m_dwValue  = 0;
        SETPAR_AND_ADDCMD(MI_STORE_DATA_IMM, m_miItf, cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::ReportSliceSizeMetaData(
        PMOS_COMMAND_BUFFER cmdBuffer,
        uint32_t            slcCount)
    {
        ENCODE_FUNC_CALL();

        PMOS_RESOURCE  presMetadataBuffer = m_basicFeature->m_resMetadataBuffer;
        MetaDataOffset resourceOffset     = m_basicFeature->m_metaDataOffset;
        if ((presMetadataBuffer == nullptr) || !m_pipeline->IsLastPass())
        {
            return MOS_STATUS_SUCCESS;
        }

        uint32_t subRegionSartOffset = resourceOffset.dwMetaDataSize + slcCount * resourceOffset.dwMetaDataSubRegionSize;

        SETPAR_AND_ADDCMD(MI_FLUSH_DW, m_miItf, cmdBuffer);

        m_pResource = presMetadataBuffer;
        m_dwOffset = subRegionSartOffset + resourceOffset.dwbStartOffset;
        m_dwValue   = 0;
        SETPAR_AND_ADDCMD(MI_STORE_DATA_IMM, m_miItf, cmdBuffer);

        m_dwOffset = subRegionSartOffset + resourceOffset.dwbHeaderSize;
        m_dwValue  = m_basicFeature->m_slcData[slcCount].BitSize;
        SETPAR_AND_ADDCMD(MI_STORE_DATA_IMM, m_miItf, cmdBuffer);

        MmioRegistersMfx *mmioRegisters = SelectVdboxAndGetMmioRegister(m_vdboxIndex, cmdBuffer);
        CODEC_HW_CHK_NULL_RETURN(mmioRegisters);
        ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_mfxItf->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");
        m_pResource = presMetadataBuffer;
        m_dwOffset  = subRegionSartOffset + resourceOffset.dwbSize;
        m_dwValue   = mmioRegisters->mfcBitstreamBytecountSliceRegOffset;
        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::SetSliceStateCommonParams(MHW_VDBOX_AVC_SLICE_STATE &sliceState)
    {
        ENCODE_FUNC_CALL();

        MOS_ZeroMemory(&sliceState, sizeof(sliceState));
        sliceState.presDataBuffer         = m_basicFeature->m_resMbCodeBuffer;
        sliceState.pAvcPicIdx             = m_basicFeature->m_ref->GetPicIndex();
        sliceState.pEncodeAvcSeqParams    = m_seqParam;
        sliceState.pEncodeAvcPicParams    = m_picParam;
        sliceState.pBsBuffer              = &(m_basicFeature->m_bsBuffer);
        sliceState.ppNalUnitParams        = m_basicFeature->m_nalUnitParams;
        sliceState.bBrcEnabled            = false;
        sliceState.bVdencInUse            = true;
        sliceState.oneOnOneMapping        = false;
        sliceState.bFirstPass             = m_pipeline->IsFirstPass();
        sliceState.bLastPass              = m_pipeline->IsLastPass();
        // Disable Panic mode when min/max QP control is on. kernel may disable it, but disable in driver also.
        sliceState.bRCPanicEnable         = m_basicFeature->m_panicEnable && (!m_basicFeature->m_minMaxQpControlEnabled);
        sliceState.wFrameFieldHeightInMB  = m_basicFeature->m_frameFieldHeightInMb;
        // App handles tail insertion for VDEnc dynamic slice in non-cp case
        sliceState.bVdencNoTailInsertion  = m_basicFeature->m_vdencNoTailInsertion;
        sliceState.bAcceleratorHeaderPackingCaps = m_basicFeature->m_acceleratorHeaderPackingCaps;

        uint32_t batchBufferForPakSlicesStartOffset = (uint32_t)m_batchBufferForPakSlices[m_pipeline->m_currRecycledBufIdx].iCurrent;
        if (m_useBatchBufferForPakSlices)
        {
            sliceState.pBatchBufferForPakSlices             = &m_batchBufferForPakSlices[m_pipeline->m_currRecycledBufIdx];
            sliceState.bSingleTaskPhaseSupported            = true;
            sliceState.dwBatchBufferForPakSlicesStartOffset = batchBufferForPakSlicesStartOffset;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::SetSliceStateParams(
        MHW_VDBOX_AVC_SLICE_STATE   &sliceState,
        PCODEC_ENCODER_SLCDATA      slcData,
        uint16_t                    slcCount)
    {
        ENCODE_FUNC_CALL();

        sliceState.pEncodeAvcSliceParams     = &m_sliceParams[slcCount];
        sliceState.dwDataBufferOffset        = slcData[slcCount].CmdOffset;
        sliceState.dwOffset                  = slcData[slcCount].SliceOffset;
        sliceState.dwLength                  = slcData[slcCount].BitSize;
        sliceState.uiSkipEmulationCheckCount = slcData[slcCount].SkipEmulationByteCount;
        sliceState.dwSliceIndex              = (uint32_t)slcCount;
        sliceState.bInsertBeforeSliceHeaders = (slcCount == 0);

        RUN_FEATURE_INTERFACE_RETURN(AvcEncodeRounding, AvcFeatureIDs::avcRoundingFeature, SetRoundingParams, sliceState);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::PackSliceHeader(uint16_t slcCount)
    {
        ENCODE_FUNC_CALL();

        CODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS packSlcHeaderParams;
        packSlcHeaderParams.pBsBuffer          = &(m_basicFeature->m_bsBuffer);
        packSlcHeaderParams.pPicParams         = m_picParam;
        packSlcHeaderParams.pSeqParams         = m_seqParam;
        packSlcHeaderParams.ppRefList          = m_basicFeature->m_ref->GetRefList();
        packSlcHeaderParams.CurrPic            = m_basicFeature->m_currOriginalPic;
        packSlcHeaderParams.CurrReconPic       = m_basicFeature->m_currReconstructedPic;
        packSlcHeaderParams.UserFlags          = m_basicFeature->m_userFlags;
        packSlcHeaderParams.NalUnitType        = m_basicFeature->m_nalUnitType;
        packSlcHeaderParams.wPictureCodingType = m_basicFeature->m_pictureCodingType;
        packSlcHeaderParams.bVdencEnabled      = true;
        packSlcHeaderParams.pAvcSliceParams    = &m_sliceParams[slcCount];

        ENCODE_CHK_STATUS_RETURN(AvcEncodeHeaderPacker::PackSliceHeader(&packSlcHeaderParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::InsertSeqStreamEnd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(AddAllCmds_MFX_PAK_INSERT_OBJECT(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(mfxStatus);
        ENCODE_CHK_NULL_RETURN(statusReport);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        EncodeStatusMfx *       encodeStatusMfx  = (EncodeStatusMfx *)mfxStatus;
        EncodeStatusReportData *statusReportData = (EncodeStatusReportData *)statusReport;

        if (statusReportData->hwCtr)
        {
            m_encodecp->UpdateCpStatusReport(statusReport);
        }

        statusReportData->codecStatus = CODECHAL_STATUS_SUCCESSFUL;
        statusReportData->numberPasses = (uint8_t)encodeStatusMfx->numberPasses;
        ENCODE_VERBOSEMESSAGE("statusReportData->numberPasses: %d\n", statusReportData->numberPasses);

        ENCODE_CHK_STATUS_RETURN(ReportExtStatistics(*encodeStatusMfx, *statusReportData));

        CODECHAL_DEBUG_TOOL(
            ENCODE_CHK_STATUS_RETURN(DumpResources(encodeStatusMfx, statusReportData)););

        m_basicFeature->Reset((CODEC_REF_LIST *)statusReportData->currRefList);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::StartStatusReport(
        uint32_t srType,
        MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(MediaPacket::StartStatusReportNext(srType, cmdBuffer));
        m_encodecp->StartCpStatusReport(cmdBuffer);

        MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectStartCmd(
            (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        ENCODE_CHK_STATUS_RETURN(CalculateMfxCommandsSize());
        ENCODE_CHK_STATUS_RETURN(CalculateVdencCommandsSize());
        commandBufferSize = CalculateCommandBufferSize();
        requestedPatchListSize = CalculatePatchListSize();
        return MOS_STATUS_SUCCESS;
    }

    uint32_t AvcVdencPkt::CalculateCommandBufferSize()
    {
        ENCODE_FUNC_CALL();
        uint32_t commandBufferSize = 0;

        commandBufferSize =
            m_pictureStatesSize +
            m_basicFeature->m_extraPictureStatesSize   +
            (m_sliceStatesSize * m_basicFeature->m_numSlices);

        if (m_pipeline->IsSingleTaskPhaseSupported())
        {
            commandBufferSize *= m_pipeline->GetPassNum();
        }

        // 4K align since allocation is in chunks of 4K bytes.
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

        return commandBufferSize;
    }

    uint32_t AvcVdencPkt::CalculatePatchListSize()
    {
        ENCODE_FUNC_CALL();
        uint32_t requestedPatchListSize = 0;
        if (m_usePatchList)
        {
            requestedPatchListSize =
            m_picturePatchListSize +
            (m_slicePatchListSize * m_basicFeature->m_numSlices);

            if (m_pipeline->IsSingleTaskPhaseSupported())
            {
                requestedPatchListSize *= m_pipeline->GetPassNum();
            }
        }
        return requestedPatchListSize;
    }

    MOS_STATUS AvcVdencPkt::CalculateVdencCommandsSize()
    {
        ENCODE_FUNC_CALL();

        MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;
        
        uint32_t hucCommandsSize = 0;
        uint32_t hucPatchListSize = 0;
        ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
            CODECHAL_ENCODE_MODE_AVC, (uint32_t *)&hucCommandsSize, (uint32_t *)&hucPatchListSize, &stateCmdSizeParams));
        m_pictureStatesSize += hucCommandsSize;
        m_picturePatchListSize += hucPatchListSize;

        // Picture Level Commands
        uint32_t vdencPictureStatesSize = 0;
        uint32_t vdencPicturePatchListSize = 0;
        vdencPictureStatesSize = 
            m_vdencItf->MHW_GETSIZE_F(VDENC_PIPE_MODE_SELECT)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_SRC_SURFACE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_REF_SURFACE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_DS_REF_SURFACE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_PIPE_BUF_ADDR_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_AVC_IMG_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_WALKER_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VD_PIPELINE_FLUSH)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_CMD3)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_AVC_SLICE_STATE)();
        vdencPicturePatchListSize = mhw::vdbox::vdenc::Itf::VDENC_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES;

        m_pictureStatesSize += vdencPictureStatesSize;
        m_picturePatchListSize += vdencPicturePatchListSize;

        // Slice Level Commands
        uint32_t vdencSliceStatesSize = 0;
        uint32_t vdencSlicePatchListSize = 0;
        vdencSliceStatesSize = 
            m_vdencItf->MHW_GETSIZE_F(VDENC_WEIGHTSOFFSETS_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_AVC_SLICE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_WALKER_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VD_PIPELINE_FLUSH)();
        vdencSlicePatchListSize = mhw::vdbox::vdenc::Itf::VDENC_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES;

        m_sliceStatesSize += vdencSliceStatesSize;
        m_slicePatchListSize += vdencSlicePatchListSize;

#if USE_CODECHAL_DEBUG_TOOL
        // for ModifyEncodedFrameSizeWithFakeHeaderSize
        // total sum is 368 (108*2 + 152)
        auto packetUtilities = m_pipeline->GetPacketUtilities();
        ENCODE_CHK_NULL_RETURN(packetUtilities);
        uint32_t sizeInByte = 0;
        bool     isIframe   = m_basicFeature->m_pictureCodingType == I_TYPE;
        if (packetUtilities->GetFakeHeaderSettings(sizeInByte, isIframe))
        {
            m_pictureStatesSize +=
                // 2x AddBufferWithIMMValue to change frame size
                (
                    m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() +
                    m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_MEM)() +
                    m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_MEM)() * 3 +
                    m_miItf->MHW_GETSIZE_F(MI_MATH)() + sizeof(mhw::mi::MHW_MI_ALU_PARAMS) * 4 +
                    m_miItf->MHW_GETSIZE_F(MI_STORE_REGISTER_MEM)()) *
                    2 +
                // SetBufferWithIMMValueU16 to change header size
                (m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() +
                    m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_MEM)() +
                    m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_MEM)() * 5 +
                    2 * (m_miItf->MHW_GETSIZE_F(MI_MATH)() + sizeof(mhw::mi::MHW_MI_ALU_PARAMS) * 4) +
                    m_miItf->MHW_GETSIZE_F(MI_STORE_REGISTER_MEM)());
        }
#endif

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::CalculateMfxCommandsSize()
    {
        ENCODE_FUNC_CALL();

        // PAK Slice Level Commands
        ENCODE_CHK_STATUS_RETURN(GetMfxPrimitiveCommandsDataSize(&m_pakSliceSize, &m_pakSlicePatchListSize, false))

        // Picture Level Commands
        ENCODE_CHK_STATUS_RETURN(GetMfxStateCommandsDataSize(&m_pictureStatesSize, &m_picturePatchListSize, false))

        // Slice Level Commands
        ENCODE_CHK_STATUS_RETURN(GetMfxPrimitiveCommandsDataSize(&m_sliceStatesSize, &m_slicePatchListSize, false))

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::GetMfxPrimitiveCommandsDataSize(
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      isModeSpecific)
    {
        ENCODE_FUNC_CALL()
        uint32_t cpCmdsize       = 0;
        uint32_t cpPatchListSize = 0;

        if (m_mfxItf && m_miItf)
        {
            uint32_t maxSize = 0, patchListMaxSize = 0;
            // 1 PAK_INSERT_OBJECT inserted for every end of frame/stream with 1 DW payload
            maxSize          = m_mfxItf->MHW_GETSIZE_F(MFX_PAK_INSERT_OBJECT)() + sizeof(uint32_t);
            patchListMaxSize = PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFC_AVC_PAK_INSERT_OBJECT_CMD);

            maxSize +=
                (2 * m_mfxItf->MHW_GETSIZE_F(MFX_AVC_REF_IDX_STATE)()) +
                (2 * m_mfxItf->MHW_GETSIZE_F(MFX_AVC_WEIGHTOFFSET_STATE)()) +
                m_mfxItf->MHW_GETSIZE_F(MFX_AVC_SLICE_STATE)() +
                MHW_VDBOX_PAK_SLICE_HEADER_OVERFLOW_SIZE +  // slice header payload
                (2 * m_mfxItf->MHW_GETSIZE_F(MFX_PAK_INSERT_OBJECT)()) +
                m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_START)() +
                m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)();

            patchListMaxSize +=
                (2 * PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_AVC_REF_IDX_STATE_CMD)) +
                (2 * PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_AVC_WEIGHTOFFSET_STATE_CMD)) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_AVC_SLICE_STATE_CMD) +
                (2 * PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFC_AVC_PAK_INSERT_OBJECT_CMD)) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MI_BATCH_BUFFER_START_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_FLUSH_DW_CMD);

            *commandsSize  = maxSize;
            *patchListSize = patchListMaxSize;

            m_hwInterface->GetCpInterface()->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
        }

        *commandsSize += (uint32_t)cpCmdsize;
        *patchListSize += (uint32_t)cpPatchListSize;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::GetMfxStateCommandsDataSize(
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      isShortFormat)
    {
        ENCODE_FUNC_CALL()
        ENCODE_CHK_NULL_RETURN(commandsSize);
        ENCODE_CHK_NULL_RETURN(patchListSize);

        uint32_t cpCmdsize       = 0;
        uint32_t cpPatchListSize = 0;

        if (m_mfxItf && m_miItf)
        {
            uint32_t maxSize =
                m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() +
                m_mfxItf->MHW_GETSIZE_F(MFX_PIPE_MODE_SELECT)() +
                m_mfxItf->MHW_GETSIZE_F(MFX_SURFACE_STATE)() +
                m_mfxItf->MHW_GETSIZE_F(MFX_PIPE_BUF_ADDR_STATE)() +
                m_mfxItf->MHW_GETSIZE_F(MFX_IND_OBJ_BASE_ADDR_STATE)() +
                2 * m_miItf->MHW_GETSIZE_F(MI_STORE_DATA_IMM)() +
                2 * m_miItf->MHW_GETSIZE_F(MI_STORE_REGISTER_MEM)() +
                8 * m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_REG)();

            uint32_t patchListMaxSize =
                PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_FLUSH_DW_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_PIPE_MODE_SELECT_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_SURFACE_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_PIPE_BUF_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_IND_OBJ_BASE_ADDR_STATE_CMD) +
                (2 * PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_STORE_DATA_IMM_CMD)) +
                (2 * PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_STORE_REGISTER_MEM_CMD));

            maxSize +=
                m_mfxItf->MHW_GETSIZE_F(MFX_BSP_BUF_BASE_ADDR_STATE)() +
                m_mfxItf->MHW_GETSIZE_F(MFD_AVC_PICID_STATE)() +
                m_mfxItf->MHW_GETSIZE_F(MFX_AVC_DIRECTMODE_STATE)() +
                m_mfxItf->MHW_GETSIZE_F(MFX_AVC_IMG_STATE)() +
                m_mfxItf->MHW_GETSIZE_F(MFX_QM_STATE)() * 4;  // QM_State sent 4 times

            patchListMaxSize +=
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_BSP_BUF_BASE_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFD_AVC_PICID_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_AVC_DIRECTMODE_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_AVC_IMG_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_QM_STATE_CMD) * 4;

            maxSize +=
                m_miItf->MHW_GETSIZE_F(MI_CONDITIONAL_BATCH_BUFFER_END)() +
                m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() * 3 +            // 3 extra MI_FLUSH_DWs for encode
                m_mfxItf->MHW_GETSIZE_F(MFX_FQM_STATE)() * 4 +         // FQM_State sent 4 times
                m_miItf->MHW_GETSIZE_F(MI_STORE_REGISTER_MEM)() * 8 +  // 5 extra register queries for encode, 3 extra slice level commands for BrcPakStatistics
                m_miItf->MHW_GETSIZE_F(MI_STORE_DATA_IMM)() * 3 +      // slice level commands for StatusReport, BrcPakStatistics
                MHW_VDBOX_PAK_BITSTREAM_OVERFLOW_SIZE +                // accounting for the max DW payload for PAK_INSERT_OBJECT, for frame header payload
                m_mfxItf->MHW_GETSIZE_F(MFX_PAK_INSERT_OBJECT)() * 4;  // for inserting AU, SPS, PSP, SEI headers before first slice header

            patchListMaxSize +=
                PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_CONDITIONAL_BATCH_BUFFER_END_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_FLUSH_DW_CMD) * 3 +            // 3 extra MI_FLUSH_DWs for encode
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_FQM_STATE_CMD) * 4 +  // FQM_State sent 4 times
                PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_STORE_REGISTER_MEM_CMD) * 8 +  // 5 extra register queries for encode, 3 extra slice level commands for BrcPakStatistics
                PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_STORE_DATA_IMM_CMD) * 3;       // slice level commands for StatusReport, BrcPakStatistics
            PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFC_AVC_PAK_INSERT_OBJECT_CMD) * 4;  // for inserting AU, SPS, PSP, SEI headers before first slice header

            *commandsSize  = maxSize;
            *patchListSize = patchListMaxSize;

            m_hwInterface->GetCpInterface()->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);
        }

        *commandsSize += (uint32_t)cpCmdsize;
        *patchListSize += (uint32_t)cpPatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MmioRegistersMfx * AvcVdencPkt::SelectVdboxAndGetMmioRegister(
        MHW_VDBOX_NODE_IND  index,
        PMOS_COMMAND_BUFFER pCmdBuffer)
    {
        if (m_hwInterface->m_getVdboxNodeByUMD)
        {
            pCmdBuffer->iVdboxNodeIndex = m_osInterface->pfnGetVdboxNodeId(m_osInterface, pCmdBuffer);
            switch (pCmdBuffer->iVdboxNodeIndex)
            {
            case MOS_VDBOX_NODE_1:
                index = MHW_VDBOX_NODE_1;
                break;
            case MOS_VDBOX_NODE_2:
                index = MHW_VDBOX_NODE_2;
                break;
            case MOS_VDBOX_NODE_INVALID:
                // That's a legal case meaning that we were not assigned with per-bb index because
                // balancing algorithm can't work (forcedly diabled or miss kernel support).
                // If that's the case we just proceed with the further static context assignment.
                break;
            default:
                // That's the case when MHW and MOS enumerations mismatch. We again proceed with the
                // best effort (static context assignment, but provide debug note).
                MHW_ASSERTMESSAGE("MOS and MHW VDBOX enumerations mismatch! Adjust HW description!");
                break;
            }
        }

        if (m_vdencItf)
        {
            return m_vdencItf->GetMmioRegisters(index);
        }
        else
        {
            MHW_ASSERTMESSAGE("Get vdenc interface failed!");
            return nullptr;
        }
    }

    void AvcVdencPkt::SetPerfTag(uint16_t type, uint16_t mode, uint16_t picCodingType)
    {
        ENCODE_FUNC_CALL();

        PerfTagSetting perfTag;
        perfTag.Value             = 0;
        perfTag.Mode              = mode & CODECHAL_ENCODE_MODE_BIT_MASK;
        perfTag.CallType          = type;
        perfTag.PictureCodingType = picCodingType > 3 ? 0 : picCodingType;
        m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);
        m_osInterface->pfnIncPerfBufferID(m_osInterface);
    }

    MOS_STATUS AvcVdencPkt::SendPrologCmds(
        MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        auto packetUtilities = m_pipeline->GetPacketUtilities();
        ENCODE_CHK_NULL_RETURN(packetUtilities);
        if (m_basicFeature->m_setMarkerEnabled)
        {
            PMOS_RESOURCE presSetMarker = m_osInterface->pfnGetMarkerResource(m_osInterface);
            ENCODE_CHK_STATUS_RETURN(packetUtilities->SendMarkerCommand(&cmdBuffer, presSetMarker));
        }

    #ifdef _MMC_SUPPORTED
        ENCODE_CHK_NULL_RETURN(m_mmcState);
        ENCODE_CHK_STATUS_RETURN(m_mmcState->SendPrologCmd(&cmdBuffer, false));
    #endif

        MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
        MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
        genericPrologParams.pOsInterface     = m_osInterface;
        genericPrologParams.pvMiInterface    = nullptr;
        genericPrologParams.bMmcEnabled      = m_mmcState ? m_mmcState->IsMmcEnabled() : false;
        ENCODE_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmdNext(&cmdBuffer, &genericPrologParams, m_miItf));

        // Send predication command
        if (m_basicFeature->m_predicationEnabled)
        {
            ENCODE_CHK_STATUS_RETURN(packetUtilities->SendPredicationCommand(&cmdBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::AllocateBatchBufferForPakSlices(
        uint32_t numSlices,
        uint16_t  numPakPasses,
        uint8_t  currRecycledBufIdx)
    {
        ENCODE_FUNC_CALL();

        MOS_ZeroMemory(
            &m_batchBufferForPakSlices[currRecycledBufIdx],
            sizeof(MHW_BATCH_BUFFER));

        // Get the slice size
        uint32_t size = numPakPasses * numSlices * m_pakSliceSize;

        m_batchBufferForPakSlices[currRecycledBufIdx].bSecondLevel = true;
        ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_osInterface,
            &m_batchBufferForPakSlices[currRecycledBufIdx],
            nullptr,
            size));

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        uint8_t *data       = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_batchBufferForPakSlices[currRecycledBufIdx].OsResource,
            &lockFlags);

        ENCODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(data, size);
        ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_batchBufferForPakSlices[currRecycledBufIdx].OsResource));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::ReleaseBatchBufferForPakSlices(
        uint8_t currRecycledBufIdx)
    {
        ENCODE_FUNC_CALL();

        if (m_batchBufferForPakSlices[currRecycledBufIdx].iSize)
        {
            ENCODE_CHK_STATUS_RETURN(Mhw_FreeBb(m_osInterface, &m_batchBufferForPakSlices[currRecycledBufIdx], nullptr));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::AddPictureMfxCommands(
        MOS_COMMAND_BUFFER & cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        //for gen 12, we need to add MFX wait for both KIN and VRT before and after MFX Pipemode select...
        SETPAR_AND_ADDCMD(MFX_WAIT, m_miItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_PIPE_MODE_SELECT, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_WAIT, m_miItf, &cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(AddAllCmds_MFX_SURFACE_STATE(&cmdBuffer));

        SETPAR_AND_ADDCMD(MFX_PIPE_BUF_ADDR_STATE, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_IND_OBJ_BASE_ADDR_STATE, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_BSP_BUF_BASE_ADDR_STATE, m_mfxItf, &cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::AddPictureVdencCommands(MOS_COMMAND_BUFFER & cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        SETPAR_AND_ADDCMD(VDENC_PIPE_MODE_SELECT, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_SRC_SURFACE_STATE, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_REF_SURFACE_STATE, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_DS_REF_SURFACE_STATE, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_PIPE_BUF_ADDR_STATE, m_vdencItf, &cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::SendSlice(PMOS_COMMAND_BUFFER cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(AddAllCmds_MFX_AVC_REF_IDX_STATE(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(AddAllCmds_MFX_AVC_WEIGHTOFFSET_STATE(cmdBuffer));

        auto brcFeature = dynamic_cast<AvcEncodeBRC *>(m_featureManager->GetFeature(AvcFeatureIDs::avcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);

        if (!brcFeature->IsVdencBrcEnabled())
        {
            SETPAR_AND_ADDCMD(MFX_AVC_SLICE_STATE, m_mfxItf, cmdBuffer);
            SETPAR_AND_ADDCMD(VDENC_AVC_SLICE_STATE, m_vdencItf, cmdBuffer);
        }
        else
        {
            PMHW_BATCH_BUFFER secondLevelBatchBuffer = brcFeature->GetBatchBufferForVdencImgStat();
            // current location to add cmds in 2nd level batch buffer
            secondLevelBatchBuffer->iCurrent = 0;
            // reset starting location (offset) executing 2nd level batch buffer for each frame & each pass
            // base part of 2nd lvl BB must be aligned for CODECHAL_CACHELINE_SIZE
            secondLevelBatchBuffer->dwOffset = MOS_ALIGN_CEIL(m_hwInterface->m_vdencBrcImgStateBufferSize, CODECHAL_CACHELINE_SIZE) +
                                               m_basicFeature->m_curNumSlices * brcFeature->GetVdencOneSliceStateSize();
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(cmdBuffer, secondLevelBatchBuffer));
            HalOcaInterfaceNext::OnSubLevelBBStart(
                *cmdBuffer,
                m_osInterface->pOsContext,
                &secondLevelBatchBuffer->OsResource,
                secondLevelBatchBuffer->dwOffset,
                false,
                brcFeature->GetVdencOneSliceStateSize());
        }

        ENCODE_CHK_STATUS_RETURN(AddAllCmds_MFX_PAK_INSERT_OBJECT(cmdBuffer));

        SETPAR_AND_ADDCMD(VDENC_WEIGHTSOFFSETS_STATE, m_vdencItf, cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_WALKER_STATE, m_vdencItf, cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::EndStatusReport(
        uint32_t            srType,
        MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(MediaPacket::EndStatusReportNext(srType, cmdBuffer));

        MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd(
            (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    void AvcVdencPkt::UpdateParameters()
    {
        ENCODE_FUNC_CALL();

        if (!m_pipeline->IsSingleTaskPhaseSupported())
        {
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }

        m_basicFeature->m_newPpsHeader = 0;
        m_basicFeature->m_newSeqHeader = 0;
    }

    MOS_STATUS AvcVdencPkt::EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        // Send MI_FLUSH command
        auto &flushDwParams                         = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams                               = {};
        flushDwParams.bVideoPipelineCacheInvalidate = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::ReadMfcStatus(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_hwInterface);

        MOS_RESOURCE *osResource = nullptr;
        uint32_t     offset = 0;

        EncodeStatusReadParams params;
        MOS_ZeroMemory(&params, sizeof(params));

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(encode::statusReportMfxBitstreamByteCountPerFrame, osResource, offset));
        params.resBitstreamByteCountPerFrame    = osResource;
        params.bitstreamByteCountPerFrameOffset = offset;

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(encode::statusReportMfxBitstreamSyntaxElementOnlyBitCount, osResource, offset));
        params.resBitstreamSyntaxElementOnlyBitCount    = osResource;
        params.bitstreamSyntaxElementOnlyBitCountOffset = offset;

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(encode::statusReportQPStatusCount, osResource, offset));
        params.resQpStatusCount    = osResource;
        params.qpStatusCountOffset = offset;

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(encode::statusReportImageStatusMask, osResource, offset));
        params.resImageStatusMask    = osResource;
        params.imageStatusMaskOffset = offset;

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(encode::statusReportImageStatusCtrl, osResource, offset));
        params.resImageStatusCtrl    = osResource;
        params.imageStatusCtrlOffset = offset;

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(encode::statusReportNumSlices, osResource, offset));
        params.resNumSlices          = osResource;
        params.numSlicesOffset = offset;

        RUN_FEATURE_INTERFACE_RETURN(AvcEncodeBRC, AvcFeatureIDs::avcBrcFeature, SetMfcStatusParams, params);

        ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_mfxItf->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");

        SETPAR_AND_ADDCMD(MI_FLUSH_DW, m_miItf, &cmdBuffer);

        MmioRegistersMfx *mmioRegisters = SelectVdboxAndGetMmioRegister(m_vdboxIndex, &cmdBuffer);
        CODEC_HW_CHK_NULL_RETURN(mmioRegisters);
        m_pResource                     = params.resBitstreamByteCountPerFrame;
        m_dwOffset                      = params.bitstreamByteCountPerFrameOffset;
        m_dwValue                       = mmioRegisters->mfcBitstreamBytecountFrameRegOffset;
        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, &cmdBuffer);

        m_pResource = params.resBitstreamSyntaxElementOnlyBitCount;
        m_dwOffset  = params.bitstreamSyntaxElementOnlyBitCountOffset;
        m_dwValue   = mmioRegisters->mfcBitstreamSeBitcountFrameRegOffset;
        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, &cmdBuffer);

        m_pResource = params.resQpStatusCount;
        m_dwOffset  = params.qpStatusCountOffset;
        m_dwValue   = mmioRegisters->mfcQPStatusCountOffset;
        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, &cmdBuffer);

        if (mmioRegisters->mfcAvcNumSlicesRegOffset > 0)
        {
            m_pResource = params.resNumSlices;
            m_dwOffset  = params.numSlicesOffset;
            m_dwValue   = mmioRegisters->mfcAvcNumSlicesRegOffset;
            SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, &cmdBuffer);
        }

        if (params.vdencBrcEnabled)
        {
            // Store PAK FrameSize MMIO to DMEM for HuC next BRC pass of current frame and first pass of next frame.
            for (int i = 0; i < 2; i++)
            {
                if (params.resVdencBrcUpdateDmemBufferPtr[i])
                {
                    m_pResource = params.resVdencBrcUpdateDmemBufferPtr[i];
                    m_dwOffset  = 5 * sizeof(uint32_t);
                    m_dwValue   = mmioRegisters->mfcBitstreamBytecountFrameRegOffset;
                    SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, &cmdBuffer);

                    if (params.vdencBrcNumOfSliceOffset)
                    {
                        m_pResource = params.resVdencBrcUpdateDmemBufferPtr[i];
                        m_dwOffset  = params.vdencBrcNumOfSliceOffset;
                        m_dwValue   = mmioRegisters->mfcAvcNumSlicesRegOffset;
                        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, &cmdBuffer);
                    }
                }
            }
        }

        ENCODE_CHK_STATUS_RETURN(ReadImageStatus(params, &cmdBuffer))

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::ReadImageStatus(
        const EncodeStatusReadParams& params,
        PMOS_COMMAND_BUFFER           cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        CODEC_HW_FUNCTION_ENTER;

        CODEC_HW_CHK_NULL_RETURN(cmdBuffer);

        CODEC_HW_CHK_COND_RETURN((m_vdboxIndex > m_mfxItf->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");

        MmioRegistersMfx *mmioRegisters = SelectVdboxAndGetMmioRegister(m_vdboxIndex, cmdBuffer);
        CODEC_HW_CHK_NULL_RETURN(mmioRegisters);
        m_pResource                     = params.resImageStatusMask;
        m_dwOffset                      = params.imageStatusMaskOffset;
        m_dwValue                       = mmioRegisters->mfcImageStatusMaskRegOffset;
        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, cmdBuffer);

        m_pResource = params.resImageStatusCtrl;
        m_dwOffset  = params.imageStatusCtrlOffset;
        m_dwValue   = mmioRegisters->mfcImageStatusCtrlRegOffset;
        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, cmdBuffer);

        // VDEnc dynamic slice overflow semaphore, DW0 is SW programmed mask(MFX_IMAGE_MASK does not support), DW1 is MFX_IMAGE_STATUS_CONTROL
        if (params.vdencBrcEnabled)
        {
            MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;

            // Added for VDEnc slice overflow bit in MFC_IMAGE_STATUS_CONTROL
            // The bit is connected on the non-AVC encoder side of MMIO register.
            // Need a dummy MFX_PIPE_MODE_SELECT to decoder and read this register.
            if (params.waReadVDEncOverflowStatus)
            {
                auto &mfxPipeModeSelectParams = m_mfxItf->MHW_GETPAR_F(MFX_PIPE_MODE_SELECT)();
                mfxPipeModeSelectParams.Mode  = CODECHAL_DECODE_MODE_AVCVLD;
                SETPAR_AND_ADDCMD(MFX_PIPE_MODE_SELECT, m_mfxItf, cmdBuffer);
            }

            // Store MFC_IMAGE_STATUS_CONTROL MMIO to DMEM for HuC next BRC pass of current frame and first pass of next frame.
            for (int i = 0; i < 2; i++)
            {
                if (params.resVdencBrcUpdateDmemBufferPtr[i])
                {
                    m_pResource = params.resVdencBrcUpdateDmemBufferPtr[i];
                    m_dwOffset  = 7 * sizeof(uint32_t); // offset of SliceSizeViolation in HUC_BRC_UPDATE_DMEM
                    m_dwValue   = mmioRegisters->mfcImageStatusCtrlRegOffset;
                    SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, cmdBuffer);
                }
            }

            // Restore MFX_PIPE_MODE_SELECT to encode mode
            if (params.waReadVDEncOverflowStatus)
            {
                auto &mfxPipeModeSelectParams     = m_mfxItf->MHW_GETPAR_F(MFX_PIPE_MODE_SELECT)();
                mfxPipeModeSelectParams.Mode      = params.mode;
                mfxPipeModeSelectParams.vdencMode = 1;
                SETPAR_AND_ADDCMD(MFX_PIPE_MODE_SELECT, m_mfxItf, cmdBuffer);
            }
        }

        SETPAR_AND_ADDCMD(MI_FLUSH_DW, m_miItf, cmdBuffer);

        return eStatus;
    }

    MOS_STATUS AvcVdencPkt::StoreNumPasses(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        MOS_RESOURCE * osResource = nullptr;
        uint32_t       offset = 0;
        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportNumberPasses, osResource, offset));

        m_pResource = osResource;
        m_dwOffset  = offset;
        m_dwValue   = m_pipeline->GetCurrentPass() + 1;
        SETPAR_AND_ADDCMD(MI_STORE_DATA_IMM, m_miItf, &cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::LockBatchBufferForPakSlices()
    {
        ENCODE_FUNC_CALL();

        m_useBatchBufferForPakSlices = false;
        if (m_pipeline->IsSingleTaskPhaseSupported() && m_pipeline->IsSingleTaskPhaseSupportedInPak())
        {
            if (m_pipeline->IsFirstPass())
            {
                // The same buffer is used for all slices for all passes.
                uint32_t batchBufferForPakSlicesSize = m_pipeline->GetPassNum() * m_basicFeature->m_numSlices * m_pakSliceSize;
                if (batchBufferForPakSlicesSize >
                    (uint32_t)m_batchBufferForPakSlices[m_pipeline->m_currRecycledBufIdx].iSize)
                {
                    if (m_batchBufferForPakSlices[m_pipeline->m_currRecycledBufIdx].iSize)
                    {
                        ENCODE_CHK_STATUS_RETURN(ReleaseBatchBufferForPakSlices(m_pipeline->m_currRecycledBufIdx));
                    }

                    ENCODE_CHK_STATUS_RETURN(AllocateBatchBufferForPakSlices(
                        m_basicFeature->m_numSlices,
                        m_pipeline->GetPassNum(),
                        m_pipeline->m_currRecycledBufIdx));
                }
            }
            ENCODE_CHK_STATUS_RETURN(Mhw_LockBb(
                m_osInterface,
                &m_batchBufferForPakSlices[m_pipeline->m_currRecycledBufIdx]));
            m_useBatchBufferForPakSlices = true;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::UnlockBatchBufferForPakSlices()
    {
        ENCODE_FUNC_CALL();

        if (m_useBatchBufferForPakSlices)
        {
            ENCODE_CHK_STATUS_RETURN(Mhw_UnlockBb(
                m_osInterface,
                &m_batchBufferForPakSlices[m_pipeline->m_currRecycledBufIdx],
                m_pipeline->IsLastPass()));
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, AvcVdencPkt)
    {
        params.intraRowStoreScratchBuffer     = m_vdencIntraRowStoreScratch;
        params.mfdIntraRowStoreScratchBuffer  = m_intraRowStoreScratchBuffer;
        params.numActiveRefL0                 = m_sliceParams->num_ref_idx_l0_active_minus1 + 1;
        params.numActiveRefL1                 = m_sliceParams->num_ref_idx_l1_active_minus1 + 1;

        ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_ref->MHW_SETPAR_F(VDENC_PIPE_BUF_ADDR_STATE)(params));

        auto settings = static_cast<AvcVdencFeatureSettings *>(m_legacyFeatureManager->GetFeatureSettings()->GetConstSettings());
        ENCODE_CHK_NULL_RETURN(settings);

        // PerfMode; replace all 4x Ds refs with the 1st L0 ref
        if (m_vdencItf->IsPerfModeSupported() &&
            settings->perfModeEnabled[m_seqParam->TargetUsage] &&
            params.numActiveRefL0 == 1)
        {
            params.numActiveRefL0   = 2;
            params.refs[1]          = nullptr;
            params.refsDsStage1[1]  = params.refsDsStage1[0];
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VD_PIPELINE_FLUSH, AvcVdencPkt)
    {
        // MfxPipeDone should be set for all super slices except the last super slice and should not be set for tail insertion.
        params.waitDoneMFX = m_lastSlice ?
            ((m_basicFeature->m_lastPicInStream || m_basicFeature->m_lastPicInSeq) ? false : true) : true;
        params.waitDoneVDENC          = true;
        params.flushVDENC             = true;
        params.waitDoneVDCmdMsgParser = true;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::AddAllCmds_MFX_QM_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        MHW_MI_CHK_NULL(m_basicFeature->m_iqWeightScaleLists);

        auto &params = m_mfxItf->MHW_GETPAR_F(MFX_QM_STATE)();
        params = {};

        auto    iqMatrix = (PMHW_VDBOX_AVC_QM_PARAMS)m_basicFeature->m_iqWeightScaleLists;
        uint8_t *qMatrix = (uint8_t *)params.quantizermatrix;

        for (uint8_t i = 0; i < 16; i++)
        {
            params.quantizermatrix[i] = 0;
        }

        params.qmType = avcQmIntra4x4;
        for (auto i = 0; i < 3; i++)
        {
            for (auto ii = 0; ii < 16; ii++)
            {
                qMatrix[i * 16 + ii] = iqMatrix->List4x4[i][ii];
            }
        }
        m_mfxItf->MHW_ADDCMD_F(MFX_QM_STATE)(cmdBuffer);

        params.qmType = avcQmInter4x4;
        for (auto i = 3; i < 6; i++)
        {
            for (auto ii = 0; ii < 16; ii++)
            {
                qMatrix[(i - 3) * 16 + ii] = iqMatrix->List4x4[i][ii];
            }
        }
        m_mfxItf->MHW_ADDCMD_F(MFX_QM_STATE)(cmdBuffer);

        params.qmType = avcQmIntra8x8;
        for (auto ii = 0; ii < 64; ii++)
        {
            qMatrix[ii] = iqMatrix->List8x8[0][ii];
        }
        m_mfxItf->MHW_ADDCMD_F(MFX_QM_STATE)(cmdBuffer);

        params.qmType = avcQmInter8x8;
        for (auto ii = 0; ii < 64; ii++)
        {
            qMatrix[ii] = iqMatrix->List8x8[1][ii];
        }
        m_mfxItf->MHW_ADDCMD_F(MFX_QM_STATE)(cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::AddAllCmds_MFX_FQM_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        auto settings = static_cast<AvcVdencFeatureSettings *>(m_legacyFeatureManager->GetFeatureSettings()->GetConstSettings());
        ENCODE_CHK_NULL_RETURN(settings);

        MHW_MI_CHK_NULL(m_basicFeature->m_iqWeightScaleLists);

        auto &params = m_mfxItf->MHW_GETPAR_F(MFX_FQM_STATE)();
        params = {};

        auto    iqMatrix = (PMHW_VDBOX_AVC_QM_PARAMS)m_basicFeature->m_iqWeightScaleLists;
        uint16_t *fqMatrix = (uint16_t*)params.quantizermatrix;

        for (uint8_t i = 0; i < 32; i++)
        {
            params.quantizermatrix[i] = 0;
        }

        params.qmType = avcQmIntra4x4;
        for (auto i = 0; i < 3; i++)
        {
            for (auto ii = 0; ii < 16; ii++)
            {
                fqMatrix[i * 16 + ii] = GetReciprocalScalingValue(iqMatrix->List4x4[i][settings->columnScan4x4[ii]]);
            }
        }
        m_mfxItf->MHW_ADDCMD_F(MFX_FQM_STATE)(cmdBuffer);

        params.qmType = avcQmInter4x4;
        for (auto i = 0; i < 3; i++)
        {
            for (auto ii = 0; ii < 16; ii++)
            {
                fqMatrix[i * 16 + ii] = GetReciprocalScalingValue(iqMatrix->List4x4[i + 3][settings->columnScan4x4[ii]]);
            }
        }
        m_mfxItf->MHW_ADDCMD_F(MFX_FQM_STATE)(cmdBuffer);

        params.qmType = avcQmIntra8x8;
        for (auto i = 0; i < 64; i++)
        {
            fqMatrix[i] = GetReciprocalScalingValue(iqMatrix->List8x8[0][settings->columnScan8x8[i]]);
        }
        m_mfxItf->MHW_ADDCMD_F(MFX_FQM_STATE)(cmdBuffer);

        params.qmType = avcQmInter8x8;
        for (auto i = 0; i < 64; i++)
        {
            fqMatrix[i] = GetReciprocalScalingValue(iqMatrix->List8x8[1][settings->columnScan8x8[i]]);
        }
        m_mfxItf->MHW_ADDCMD_F(MFX_FQM_STATE)(cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    void AvcVdencPkt::fill_pad_with_value(PMOS_SURFACE psSurface, uint32_t real_height, uint32_t aligned_height) const
    {
        ENCODE_CHK_NULL_NO_STATUS_RETURN(psSurface);

        // unaligned surfaces only
        if (aligned_height <= real_height || aligned_height > psSurface->dwHeight)
        {
            return;
        }

        if (psSurface->OsResource.TileType == MOS_TILE_INVALID)
        {
            return;
        }

        if (psSurface->Format == Format_NV12 || psSurface->Format == Format_P010)
        {
            uint32_t pitch         = psSurface->dwPitch;
            uint32_t UVPlaneOffset = psSurface->UPlaneOffset.iSurfaceOffset;
            uint32_t YPlaneOffset  = psSurface->dwOffset;
            uint32_t pad_rows      = aligned_height - real_height;
            uint32_t y_plane_size  = pitch * real_height;
            uint32_t uv_plane_size = pitch * real_height / 2;

            MOS_LOCK_PARAMS lockFlags;
            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.WriteOnly = 1;

            // padding for the linear format buffer.
            if (psSurface->OsResource.TileType == MOS_TILE_LINEAR)
            {
                #include "media_packet.h"
                uint8_t *src_data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &(psSurface->OsResource), &lockFlags);

                if (!src_data)
                    return;

                uint8_t *src_data_y     = src_data + YPlaneOffset;
                uint8_t *src_data_y_end = src_data_y + y_plane_size;
                for (uint32_t i = 0; i < pad_rows; i++)
                {
                    MOS_SecureMemcpy(src_data_y_end + i * pitch, pitch, src_data_y_end - pitch, pitch);
                }

                uint8_t *src_data_uv     = src_data + UVPlaneOffset;
                uint8_t *src_data_uv_end = src_data_uv + uv_plane_size;
                for (uint32_t i = 0; i < pad_rows / 2; i++)
                {
                    MOS_SecureMemcpy(src_data_uv_end + i * pitch, pitch, src_data_uv_end - pitch, pitch);
                }

                m_osInterface->pfnUnlockResource(m_osInterface, &(psSurface->OsResource));
            }
            else
            {
                // we don't copy out the whole tiled buffer to linear and padding on the tiled buffer directly.
                lockFlags.TiledAsTiled = 1;

                uint8_t *src_data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &(psSurface->OsResource), &lockFlags);
                if (!src_data)
                    return;

                uint8_t *padding_data = (uint8_t *)MOS_AllocMemory(pitch * pad_rows);

                // Copy last Y row data to linear padding data.
                GMM_RES_COPY_BLT gmmResCopyBlt = {0};
                gmmResCopyBlt.Gpu.pData        = src_data;
                gmmResCopyBlt.Gpu.OffsetX      = 0;
                gmmResCopyBlt.Gpu.OffsetY      = (YPlaneOffset + y_plane_size - pitch) / pitch;
                gmmResCopyBlt.Sys.pData        = padding_data;
                gmmResCopyBlt.Sys.RowPitch     = pitch;
                gmmResCopyBlt.Sys.BufferSize   = pitch * pad_rows;
                gmmResCopyBlt.Sys.SlicePitch   = pitch;
                gmmResCopyBlt.Blt.Slices       = 1;
                gmmResCopyBlt.Blt.Upload       = false;
                gmmResCopyBlt.Blt.Width        = psSurface->dwWidth;
                gmmResCopyBlt.Blt.Height       = 1;
                psSurface->OsResource.pGmmResInfo->CpuBlt(&gmmResCopyBlt);
                // Fill the remain padding lines with last Y row data.
                for (uint32_t i = 1; i < pad_rows; i++)
                {
                    MOS_SecureMemcpy(padding_data + i * pitch, pitch, padding_data, pitch);
                }
                // Filling the padding for Y.
                gmmResCopyBlt.Gpu.pData      = src_data;
                gmmResCopyBlt.Gpu.OffsetX    = 0;
                gmmResCopyBlt.Gpu.OffsetY    = (YPlaneOffset + y_plane_size) / pitch;
                gmmResCopyBlt.Sys.pData      = padding_data;
                gmmResCopyBlt.Sys.RowPitch   = pitch;
                gmmResCopyBlt.Sys.BufferSize = pitch * pad_rows;
                gmmResCopyBlt.Sys.SlicePitch = pitch;
                gmmResCopyBlt.Blt.Slices     = 1;
                gmmResCopyBlt.Blt.Upload     = true;
                gmmResCopyBlt.Blt.Width      = psSurface->dwWidth;
                gmmResCopyBlt.Blt.Height     = pad_rows;
                psSurface->OsResource.pGmmResInfo->CpuBlt(&gmmResCopyBlt);

                // Copy last UV row data to linear padding data.
                gmmResCopyBlt.Gpu.pData      = src_data;
                gmmResCopyBlt.Gpu.OffsetX    = 0;
                gmmResCopyBlt.Gpu.OffsetY    = (UVPlaneOffset + uv_plane_size - pitch) / pitch;
                gmmResCopyBlt.Sys.pData      = padding_data;
                gmmResCopyBlt.Sys.RowPitch   = pitch;
                gmmResCopyBlt.Sys.BufferSize = pitch * pad_rows / 2;
                gmmResCopyBlt.Sys.SlicePitch = pitch;
                gmmResCopyBlt.Blt.Slices     = 1;
                gmmResCopyBlt.Blt.Upload     = false;
                gmmResCopyBlt.Blt.Width      = psSurface->dwWidth;
                gmmResCopyBlt.Blt.Height     = 1;
                psSurface->OsResource.pGmmResInfo->CpuBlt(&gmmResCopyBlt);
                // Fill the remain padding lines with last UV row data.
                for (uint32_t i = 1; i < pad_rows / 2; i++)
                {
                    MOS_SecureMemcpy(padding_data + i * pitch, pitch, padding_data, pitch);
                }
                // Filling the padding for UV.
                gmmResCopyBlt.Gpu.pData      = src_data;
                gmmResCopyBlt.Gpu.OffsetX    = 0;
                gmmResCopyBlt.Gpu.OffsetY    = (UVPlaneOffset + uv_plane_size) / pitch;
                gmmResCopyBlt.Sys.pData      = padding_data;
                gmmResCopyBlt.Sys.RowPitch   = pitch;
                gmmResCopyBlt.Sys.BufferSize = pitch * pad_rows / 2;
                gmmResCopyBlt.Sys.SlicePitch = pitch;
                gmmResCopyBlt.Blt.Slices     = 1;
                gmmResCopyBlt.Blt.Upload     = true;
                gmmResCopyBlt.Blt.Width      = psSurface->dwWidth;
                gmmResCopyBlt.Blt.Height     = pad_rows / 2;
                psSurface->OsResource.pGmmResInfo->CpuBlt(&gmmResCopyBlt);

                MOS_FreeMemory(padding_data);
                padding_data = nullptr;
                m_osInterface->pfnUnlockResource(m_osInterface, &(psSurface->OsResource));
            }
        }
    }

    MOS_STATUS AvcVdencPkt::AddAllCmds_MFX_SURFACE_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        m_curMfxSurfStateId = CODECHAL_MFX_REF_SURFACE_ID;
        SETPAR_AND_ADDCMD(MFX_SURFACE_STATE, m_mfxItf, cmdBuffer);

        m_curMfxSurfStateId = CODECHAL_MFX_SRC_SURFACE_ID;
        SETPAR_AND_ADDCMD(MFX_SURFACE_STATE, m_mfxItf, cmdBuffer);

        m_curMfxSurfStateId = CODECHAL_MFX_DSRECON_SURFACE_ID;
        SETPAR_AND_ADDCMD(MFX_SURFACE_STATE, m_mfxItf, cmdBuffer);

        //add fill_pad_with_value function
        auto waTable = m_osInterface == nullptr ? nullptr : m_osInterface->pfnGetWaTable(m_osInterface);
        if (waTable)
        {
            if (MEDIA_IS_WA(waTable, Wa_AvcUnalignedHeight))
            {
                if (m_basicFeature->m_frame_cropping_flag)
                {
                    uint32_t crop_unit_y    = 2 * (2 - m_basicFeature->m_frame_mbs_only_flag);
                    uint32_t real_height    = m_basicFeature->m_oriFrameHeight - (m_basicFeature->m_frame_crop_bottom_offset * crop_unit_y);
                    uint32_t aligned_height = MOS_ALIGN_CEIL(real_height, CODEC_AVC_MIN_BLOCK_HEIGHT);

                    fill_pad_with_value(m_basicFeature->m_rawSurfaceToPak, real_height, aligned_height);
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::AddAllCmds_MFX_PAK_INSERT_OBJECT(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        bool  bLastPicInSeq    = m_basicFeature->m_lastPicInSeq;
        bool  bLastPicInStream = m_basicFeature->m_lastPicInStream;
        auto &params           = m_mfxItf->MHW_GETPAR_F(MFX_PAK_INSERT_OBJECT)();
        params                 = {};

        if (m_lastPic && (bLastPicInSeq || bLastPicInStream))    // used by AVC, MPEG2
        {
            params.dwPadding                                        = bLastPicInSeq + bLastPicInStream;
            params.bitstreamstartresetResetbitstreamstartingpos     = false;
            params.endofsliceflagLastdstdatainsertcommandflag       = true;
            params.lastheaderflagLastsrcheaderdatainsertcommandflag = true;
            params.emulationflagEmulationbytebitsinsertenable       = false;
            params.skipemulbytecntSkipEmulationByteCount            = 0;
            params.databitsinlastdwSrcdataendingbitinclusion50      = 32;
            params.sliceHeaderIndicator                             = false;
            params.headerlengthexcludefrmsize                       = true;

            m_mfxItf->MHW_ADDCMD_F(MFX_PAK_INSERT_OBJECT)(cmdBuffer);

            if (bLastPicInSeq) // only used by AVC, not used by MPEG2
            {
                uint32_t lastPicInSeqData = (uint32_t)((1 << 16) | CODECHAL_ENCODE_AVC_NAL_UT_EOSEQ << 24);
                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, &lastPicInSeqData, sizeof(lastPicInSeqData)));
            }

            if (bLastPicInStream)  // used by AVC, MPEG2
            {
                uint32_t lastPicInStreamData = (uint32_t)((1 << 16) | CODECHAL_ENCODE_AVC_NAL_UT_EOSTREAM << 24);
                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, &lastPicInStreamData, sizeof(lastPicInStreamData)));
            }
        }
        else // used by AVC, MPEG2, JPEG
        {
            bool     insertZeroByteWA = false;

            MEDIA_WA_TABLE *waTable = m_basicFeature->GetWaTable();
            ENCODE_CHK_NULL_RETURN(waTable);

            //insert AU, SPS, PSP headers before first slice header
            if (m_basicFeature->m_curNumSlices == 0)
            {
                uint32_t maxBytesInPakInsertObjCmd = ((2 << 11) - 1) * 4;  // 12 bits for DwordLength field in PAK_INSERT_OBJ cmd

                uint8_t *dataBase  = (uint8_t *)(m_basicFeature->m_bsBuffer.pBase);
                uint32_t startCode = ((*dataBase) << 24) + ((*(dataBase + 1)) << 16) + ((*(dataBase + 2)) << 8) + (*(dataBase + 3));
                // Only apply the WaSuperSliceHeaderPacking for the cases with 00 00 00 01 start code
                if (startCode == 0x00000001)
                {
                    insertZeroByteWA = true;
                }

                for (auto i = 0; i < CODECHAL_ENCODE_AVC_MAX_NAL_TYPE; i++)
                {
                    if (m_basicFeature->m_nalUnitParams[i]->bInsertEmulationBytes)
                    {
                        ENCODE_VERBOSEMESSAGE("The emulation prevention bytes are not inserted by the app and are requested to be inserted by HW.");
                    }

                    uint32_t nalunitPosiSize   = m_basicFeature->m_nalUnitParams[i]->uiSize;
                    uint32_t nalunitPosiOffset = m_basicFeature->m_nalUnitParams[i]->uiOffset;
                    while (nalunitPosiSize > 0)
                    {
                        uint32_t dwBitSize = MOS_MIN(maxBytesInPakInsertObjCmd * 8, nalunitPosiSize * 8);
                        uint32_t byteSize = (dwBitSize + 7) >> 3;
                        uint32_t dataBitsInLastDw = dwBitSize % 32;

                        if (dataBitsInLastDw == 0)
                        {
                            dataBitsInLastDw = 32;
                        }

                        params = {};
                        params.dwPadding = ((byteSize + 3) >> 2);
                        params.bitstreamstartresetResetbitstreamstartingpos = false;
                        params.endofsliceflagLastdstdatainsertcommandflag = false;
                        params.lastheaderflagLastsrcheaderdatainsertcommandflag = false;
                        params.emulationflagEmulationbytebitsinsertenable = m_basicFeature->m_nalUnitParams[i]->bInsertEmulationBytes;
                        params.skipemulbytecntSkipEmulationByteCount = m_basicFeature->m_nalUnitParams[i]->uiSkipEmulationCheckCount;
                        params.databitsinlastdwSrcdataendingbitinclusion50 = dataBitsInLastDw;
                        params.sliceHeaderIndicator = false;
                        params.headerlengthexcludefrmsize = params.emulationflagEmulationbytebitsinsertenable ? false : true; // Cannot be set to true if emulation byte bit insertion is enabled

                        m_mfxItf->MHW_ADDCMD_F(MFX_PAK_INSERT_OBJECT)(cmdBuffer);

                        // Add actual data
                        uint8_t* data = (uint8_t*)(m_basicFeature->m_bsBuffer.pBase + nalunitPosiOffset);
                        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize));

                        if (nalunitPosiSize > maxBytesInPakInsertObjCmd)
                        {
                            nalunitPosiSize -= maxBytesInPakInsertObjCmd;
                            nalunitPosiOffset += maxBytesInPakInsertObjCmd;
                        }
                        else
                        {
                            nalunitPosiSize = 0;
                        }

                        insertZeroByteWA = false;
                    }
                }
            }

            uint8_t *dataBase  = (uint8_t *)(m_basicFeature->m_bsBuffer.pBase + m_basicFeature->m_slcData[m_basicFeature->m_curNumSlices].SliceOffset);
            uint32_t startCode = ((*dataBase) << 24) + ((*(dataBase + 1)) << 16) + ((*(dataBase + 2)) << 8) + (*(dataBase + 3));
            if (startCode == 0x00000001)
            {
                insertZeroByteWA = true;
            }

            // Insert 0x00 for super slice case when PPS/AUD is not inserted
            if (insertZeroByteWA)
            {
                uint32_t byteSize = 1;

                params = {};
                params.dwPadding = ((byteSize + 3) >> 2);
                params.bitstreamstartresetResetbitstreamstartingpos = false;
                params.endofsliceflagLastdstdatainsertcommandflag = false;
                params.lastheaderflagLastsrcheaderdatainsertcommandflag = false;
                params.emulationflagEmulationbytebitsinsertenable = false;
                params.skipemulbytecntSkipEmulationByteCount = 0;
                params.databitsinlastdwSrcdataendingbitinclusion50 = 8;
                params.sliceHeaderIndicator = false;
                params.headerlengthexcludefrmsize = false;

                m_mfxItf->MHW_ADDCMD_F(MFX_PAK_INSERT_OBJECT)(cmdBuffer);

                // Add actual data
                uint8_t* data = (uint8_t*)(m_basicFeature->m_bsBuffer.pBase + m_basicFeature->m_slcData[m_basicFeature->m_curNumSlices].SliceOffset);
                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize));
            }

            // Insert slice header
            uint32_t uiSkipEmulationCheckCount = 0;
            if (m_basicFeature->m_acceleratorHeaderPackingCaps)
            {
                // If driver does slice header packing set the skip count to 4
                uiSkipEmulationCheckCount = 4;
            }
            else
            {
                // App does the slice header packing, set the skip count passed by the app
                uiSkipEmulationCheckCount = m_basicFeature->m_slcData[m_basicFeature->m_curNumSlices].SkipEmulationByteCount;
            }

            // Remove one byte of 00 for super slice case when PPS/AUD is not inserted, so that HW could patch slice header correctly
            uint32_t dwBitSize = 0, dwOffset = 0;
            if (insertZeroByteWA)
            {
                dwBitSize = m_basicFeature->m_slcData[m_basicFeature->m_curNumSlices].BitSize - 8;
                dwOffset  = m_basicFeature->m_slcData[m_basicFeature->m_curNumSlices].SliceOffset + 1;
            }
            else
            {
                dwBitSize = m_basicFeature->m_slcData[m_basicFeature->m_curNumSlices].BitSize;
                dwOffset  = m_basicFeature->m_slcData[m_basicFeature->m_curNumSlices].SliceOffset;
            }

            uint32_t byteSize = (dwBitSize + 7) >> 3;
            uint32_t dataBitsInLastDw = dwBitSize % 32;

            if (dataBitsInLastDw == 0)
            {
                dataBitsInLastDw = 32;
            }

            params = {};
            params.dwPadding = ((byteSize + 3) >> 2);
            params.bitstreamstartresetResetbitstreamstartingpos = false;
            params.endofsliceflagLastdstdatainsertcommandflag = false;
            params.lastheaderflagLastsrcheaderdatainsertcommandflag = true;
            params.emulationflagEmulationbytebitsinsertenable = true;
            params.skipemulbytecntSkipEmulationByteCount = uiSkipEmulationCheckCount;
            params.databitsinlastdwSrcdataendingbitinclusion50 = dataBitsInLastDw;
            params.sliceHeaderIndicator = true;
            params.headerlengthexcludefrmsize = false;

            m_mfxItf->MHW_ADDCMD_F(MFX_PAK_INSERT_OBJECT)(cmdBuffer);

            // Add actual data
            uint8_t* data = (uint8_t*)(m_basicFeature->m_bsBuffer.pBase + dwOffset);
            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::AddAllCmds_MFX_AVC_REF_IDX_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        PCODEC_AVC_ENCODE_SLICE_PARAMS slcParams = &m_sliceParams[m_basicFeature->m_curNumSlices];

        auto &params = m_mfxItf->MHW_GETPAR_F(MFX_AVC_REF_IDX_STATE)();
        params       = {};

        if (Slice_Type[slcParams->slice_type] == SLICE_P ||
            Slice_Type[slcParams->slice_type] == SLICE_B)
        {
            params.uiList = LIST_0;
            ENCODE_CHK_STATUS_RETURN(m_basicFeature->MHW_SETPAR_F(MFX_AVC_REF_IDX_STATE)(params));
            m_mfxItf->MHW_ADDCMD_F(MFX_AVC_REF_IDX_STATE)(cmdBuffer);
        }

        if (Slice_Type[slcParams->slice_type] == SLICE_B)
        {
            params.uiList = LIST_1;
            ENCODE_CHK_STATUS_RETURN(m_basicFeature->MHW_SETPAR_F(MFX_AVC_REF_IDX_STATE)(params));
            m_mfxItf->MHW_ADDCMD_F(MFX_AVC_REF_IDX_STATE)(cmdBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPkt::AddAllCmds_MFX_AVC_WEIGHTOFFSET_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        PCODEC_AVC_ENCODE_SLICE_PARAMS slcParams = &m_sliceParams[m_basicFeature->m_curNumSlices];

        auto &params = m_mfxItf->MHW_GETPAR_F(MFX_AVC_WEIGHTOFFSET_STATE)();
        params       = {};

        auto wpFeature = dynamic_cast<AvcVdencWeightedPred *>(m_featureManager->GetFeature(AvcFeatureIDs::avcVdencWpFeature));
        ENCODE_CHK_NULL_RETURN(wpFeature);

        if ((Slice_Type[slcParams->slice_type] == SLICE_P) &&
            (m_picParam->weighted_pred_flag == EXPLICIT_WEIGHTED_INTER_PRED_MODE) ||
            (Slice_Type[slcParams->slice_type] == SLICE_B) &&
            (m_picParam->weighted_bipred_idc == EXPLICIT_WEIGHTED_INTER_PRED_MODE))
        {
            params.uiList = LIST_0;
            ENCODE_CHK_STATUS_RETURN(wpFeature->MHW_SETPAR_F(MFX_AVC_WEIGHTOFFSET_STATE)(params));
            m_mfxItf->MHW_ADDCMD_F(MFX_AVC_WEIGHTOFFSET_STATE)(cmdBuffer);
        }

        if ((Slice_Type[slcParams->slice_type] == SLICE_B) &&
            (m_picParam->weighted_bipred_idc == EXPLICIT_WEIGHTED_INTER_PRED_MODE))
        {
            params.uiList = LIST_1;
            ENCODE_CHK_STATUS_RETURN(wpFeature->MHW_SETPAR_F(MFX_AVC_WEIGHTOFFSET_STATE)(params));
            m_mfxItf->MHW_ADDCMD_F(MFX_AVC_WEIGHTOFFSET_STATE)(cmdBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(MFX_SURFACE_STATE, AvcVdencPkt)
    {
        params.surfaceId = m_curMfxSurfStateId;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(MFX_PIPE_BUF_ADDR_STATE, AvcVdencPkt)
    {
        params.presMfdDeblockingFilterRowStoreScratchBuffer = m_resDeblockingFilterRowStoreScratchBuffer;
        params.presMfdIntraRowStoreScratchBuffer            = m_intraRowStoreScratchBuffer;

        if (m_basicFeature->m_perMBStreamOutEnable)
        {
            // Using frame and PerMB level buffer to get PerMB StreamOut PAK Statistic.
            params.presStreamOutBuffer = m_basicFeature->m_recycleBuf->GetBuffer(BrcPakStatisticBufferFull, m_basicFeature->m_frameNum);
        }
        else
        {
            params.presStreamOutBuffer = m_basicFeature->m_recycleBuf->GetBuffer(BrcPakStatisticBuffer, 0);
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(MFX_BSP_BUF_BASE_ADDR_STATE, AvcVdencPkt)
    {
        params.presBsdMpcRowStoreScratchBuffer = m_resMPCRowStoreScratchBuffer;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(MFX_AVC_IMG_STATE, AvcVdencPkt)
    {
        auto brcFeature = dynamic_cast<AvcEncodeBRC*>(m_featureManager->GetFeature(AvcFeatureIDs::avcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);

        bool bIPCMPass = m_pipeline->GetCurrentPass() && m_pipeline->IsLastPass() && (!brcFeature->IsVdencBrcEnabled());
        params.mbstatenabled = bIPCMPass ? true : false; // Disable for the first pass

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(MI_CONDITIONAL_BATCH_BUFFER_END, AvcVdencPkt)
    {
        params.presSemaphoreBuffer = m_pResource;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(MI_STORE_REGISTER_MEM, AvcVdencPkt)
    {
        params.presStoreBuffer = m_pResource;
        params.dwOffset        = m_dwOffset;
        params.dwRegister      = m_dwValue;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(MI_STORE_DATA_IMM, AvcVdencPkt)
    {
        params.pOsResource      = m_pResource;
        params.dwResourceOffset = m_dwOffset;
        params.dwValue          = m_dwValue;

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS AvcVdencPkt::DumpResources(
        EncodeStatusMfx *       encodeStatusMfx,
        EncodeStatusReportData *statusReportData)
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
        debugInterface->m_bufferDumpFrameNum = m_statusReport->GetReportedCount() + 1; // ToDo: for debug purpose
        debugInterface->m_frameType          = encodeStatusMfx->pictureCodingType;

        auto settings = static_cast<AvcVdencFeatureSettings *>(m_legacyFeatureManager->GetFeatureSettings()->GetConstSettings());
        ENCODE_CHK_NULL_RETURN(settings);
        auto brcSettings = settings->brcSettings;

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

        // BRC non-native ROI dump as HuC_region8[in], HuC_region9[in] and HuC_region10[out]
        auto brcFeature = dynamic_cast<AvcEncodeBRC*>(m_featureManager->GetFeature(AvcFeatureIDs::avcBrcFeature));
        auto streamInFeature = dynamic_cast<AvcVdencStreamInFeature*>(m_featureManager->GetFeature(AvcFeatureIDs::avcVdencStreamInFeature));
        bool isVdencBrcEnabled = brcFeature && brcFeature->IsVdencBrcEnabled();
        if (streamInFeature && (!isVdencBrcEnabled || m_basicFeature->m_picParam->bNativeROI))
        {
            ENCODE_CHK_STATUS_RETURN(streamInFeature->Dump(debugInterface, m_basicFeature->m_mbQpDataEnabled ? "_MBQP" : "_ROI"));
        }

        MOS_SURFACE *ds4xSurface = m_basicFeature->m_trackedBuf->GetSurface(
            BufferType::ds4xSurface, currRefList.ucScalingIdx);
        if (ds4xSurface != nullptr)
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
                ds4xSurface,
                CodechalDbgAttr::attrReconstructedSurface,
                "4XScaling"))
        }

        if (MEDIA_IS_SKU(m_hwInterface->GetSkuTable(), FtrFlatPhysCCS))
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBltOutput(
                &currRefList.sRefReconBuffer,
                CodechalDbgAttr::attrDecodeBltOutput));
        }
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
            &currRefList.sRefReconBuffer,
            CodechalDbgAttr::attrReconstructedSurface,
            "ReconSurf"))

        if (MEDIA_IS_SKU(m_hwInterface->GetSkuTable(), FtrFlatPhysCCS))
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBltOutput(
                &currRefList.sRefRawBuffer,
                CodechalDbgAttr::attrDecodeBltOutput));
        }
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
            &currRefList.sRefRawBuffer,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "SrcSurf"))

        if (currRefList.bUsedAsRef) {
            auto curColBuf= (currRefList.bIsIntra) ? m_basicFeature->m_colocatedMVBufferForIFrames : m_basicFeature->m_trackedBuf->GetBuffer(BufferType::mvTemporalBuffer, currRefList.ucScalingIdx);
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
                curColBuf,
                CodechalDbgAttr::attrMvData,
                "_CoLocated_Out",
                m_basicFeature->m_colocatedMVBufferSize));
        }

        //  Slice size Buffer dump
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            m_basicFeature->m_recycleBuf->GetBuffer(PakSliceSizeStreamOutBuffer, m_statusReport->GetReportedCount()),
            CodechalDbgAttr::attrSliceSizeStreamout,
            "_SliceSizeStreamOut",
            CODECHAL_ENCODE_SLICESIZE_BUF_SIZE));

        //  here add the dump buffer for PAK statistics
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            m_basicFeature->m_recycleBuf->GetBuffer(BrcPakStatisticBufferFull, m_statusReport->GetReportedCount()),
            CodechalDbgAttr::attrPakOutput,
            "MB and FrameLevel PAK staistics vdenc",
            brcSettings.vdencBrcPakStatsBufferSize + m_basicFeature->m_picWidthInMb * m_basicFeature->m_picHeightInMb * 64));

        return MOS_STATUS_SUCCESS;
    }

    bool SearchNALHeader(
        PMHW_VDBOX_AVC_SLICE_STATE sliceState,
        uint32_t                   startCode)
    {
        ENCODE_FUNC_CALL();

        for (auto i = 0; i < CODECHAL_ENCODE_AVC_MAX_NAL_TYPE; i++)
        {
            if (sliceState->ppNalUnitParams[i]->uiSize > 0)
            {
                uint32_t offset   = 0;
                uint8_t *dataBase = (uint8_t *)(sliceState->pBsBuffer->pBase + sliceState->ppNalUnitParams[i]->uiOffset);

                while (offset < sliceState->ppNalUnitParams[i]->uiSize - 3)
                {
                    uint8_t *dataBuf = dataBase + offset;

                    if (dataBuf[0] == 0 && dataBuf[1] == 0 && dataBuf[2] == 1 && (dataBuf[3] + 0x100) == startCode)
                        return true;

                    offset++;
                }
            }
        }

        return false;
    }

    MOS_STATUS AvcVdencPkt::DumpEncodeImgStats(
        PMOS_COMMAND_BUFFER cmdbuffer)
    {
        ENCODE_FUNC_CALL();

        CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        if (!debugInterface->DumpIsEnabled(CodechalDbgAttr::attrImageState))
        {
            return MOS_STATUS_SUCCESS;
        }

        std::string SurfName = "Pak_VDEnc_Pass[" + std::to_string(static_cast<uint32_t>(m_pipeline->GetCurrentPass())) + "]";

        auto brcFeature = dynamic_cast<AvcEncodeBRC *>(m_featureManager->GetFeature(AvcFeatureIDs::avcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);

        // MFX_AVC_IMG_STATE
        if (!brcFeature->IsVdencBrcEnabled())
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
                &m_batchBufferForVdencImgStat[m_pipeline->m_currRecycledBufIdx].OsResource,
                CodechalDbgAttr::attrImageState,
                SurfName.c_str(),
                m_hwInterface->m_vdencBrcImgStateBufferSize,
                0,
                CODECHAL_NUM_MEDIA_STATES));
        }
        return MOS_STATUS_SUCCESS;
    }

#endif

}
