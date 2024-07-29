/*
* Copyright (c) 2018-2023, Intel Corporation
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
//! \file     decode_hevc_pipeline_m12.cpp
//! \brief    Defines the interface for hevc decode pipeline
//!
#include "decode_hevc_pipeline_m12.h"
#include "decode_huc_s2l_packet_m12.h"
#include "decode_hevc_packet_front_end_m12.h"
#include "decode_hevc_packet_back_end_m12.h"
#include "decode_hevc_packet_real_tile_m12.h"
#include "decode_hevc_picture_packet_m12.h"
#include "decode_hevc_slice_packet_m12.h"
#include "decode_utils.h"
#include "decode_common_feature_defs.h"
#include "decode_hevc_mem_compression_m12.h"
#include "decode_sfc_histogram_postsubpipeline_m12.h"
#include "decode_hevc_feature_manager.h"
#include "decode_input_bitstream_m12.h"
#include "decode_cp_bitstream_m12.h"
#include "decode_hevc_downsampling_packet.h"
#include "decode_marker_packet_g12.h"
#include "decode_predication_packet_g12.h"
#include "mos_interface.h"

namespace decode {

HevcPipelineM12::HevcPipelineM12(CodechalHwInterface *hwInterface, CodechalDebugInterface *debugInterface)
    : HevcPipeline(*hwInterface, debugInterface)
{
    m_hwInterface = hwInterface;
}

MOS_STATUS HevcPipelineM12::Init(void *settings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(settings);

    DECODE_CHK_STATUS(Initialize(settings));

    if (m_basicFeature->m_shortFormatInUse)
    {
        HucS2lPktM12 *hucS2lPkt = MOS_New(HucS2lPktM12, this, m_task, m_hwInterface);
        DECODE_CHK_NULL(hucS2lPkt);
        DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, hucS2lPacketId), hucS2lPkt));
        DECODE_CHK_STATUS(hucS2lPkt->Init());
    }

    m_hevcDecodePktLong = MOS_New(HevcDecodeLongPktM12, this, m_task, m_hwInterface);
    DECODE_CHK_NULL(m_hevcDecodePktLong);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, hevcLongPacketId), m_hevcDecodePktLong));
    DECODE_CHK_STATUS(m_hevcDecodePktLong->Init());

    auto hevcDecodePktFrontEnd = MOS_New(HevcDecodeFrontEndPktM12, this, m_task, m_hwInterface);
    DECODE_CHK_NULL(hevcDecodePktFrontEnd);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, hevcFrontEndPacketId), hevcDecodePktFrontEnd));
    DECODE_CHK_STATUS(hevcDecodePktFrontEnd->Init());

    auto hevcDecodePktBackEnd = MOS_New(HevcDecodeBackEndPktM12, this, m_task, m_hwInterface);
    DECODE_CHK_NULL(hevcDecodePktBackEnd);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, hevcBackEndPacketId), hevcDecodePktBackEnd));
    DECODE_CHK_STATUS(hevcDecodePktBackEnd->Init());

    auto hevcDecodePktRealTile = MOS_New(HevcDecodeRealTilePktM12, this, m_task, m_hwInterface);
    DECODE_CHK_NULL(hevcDecodePktRealTile);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, hevcRealTilePacketId), hevcDecodePktRealTile));
    DECODE_CHK_STATUS(hevcDecodePktRealTile->Init());

    if (m_numVdbox == 2)
    {
        m_allowVirtualNodeReassign = true;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipelineM12::InitContexOption(HevcScalabilityPars &scalPars)
{
    scalPars.usingHcp           = true;
    scalPars.enableVE           = MOS_VE_SUPPORTED(m_osInterface);
    scalPars.disableScalability = m_hwInterface->IsDisableScalability();
    bool isMultiDevices = false, isMultiEngine = false;
    m_osInterface->pfnGetMultiEngineStatus(m_osInterface, nullptr, COMPONENT_Encode, isMultiDevices, isMultiEngine);
    if (isMultiDevices && !isMultiEngine)
    {
        scalPars.disableScalability = true;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_osInterface->bHcpDecScalabilityMode == MOS_SCALABILITY_ENABLE_MODE_FALSE)
    {
        scalPars.disableScalability = true;
    }
    else if (m_osInterface->bHcpDecScalabilityMode == MOS_SCALABILITY_ENABLE_MODE_USER_FORCE)
    {
        scalPars.disableScalability = false;
    }
    scalPars.modeSwithThreshold1 =
        ReadUserFeature(m_userSettingPtr, "HCP Decode Mode Switch TH1", MediaUserSetting::Group::Sequence).Get<uint32_t>();
    scalPars.modeSwithThreshold2 =
        ReadUserFeature(m_userSettingPtr, "HCP Decode Mode Switch TH2", MediaUserSetting::Group::Sequence).Get<uint32_t>();
    scalPars.forceMultiPipe =
        ReadUserFeature(m_userSettingPtr, "HCP Decode Always Frame Split", MediaUserSetting::Group::Sequence).Get<bool>();
#endif

    if (!scalPars.disableScalability)
        m_osInterface->pfnSetMultiEngineEnabled(m_osInterface, COMPONENT_Decode, true);

    return MOS_STATUS_SUCCESS;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS HevcPipelineM12::HwStatusCheck(const DecodeStatusMfx &status)
{
    DECODE_FUNC_CALL();

    if (m_basicFeature->m_shortFormatInUse)
    {
        // Check HuC_status2 Imem loaded bit, if 0, return error
        if (((status.m_hucErrorStatus2 >> 32) && (m_hwInterface->GetHucInterface()->GetHucStatus2ImemLoadedMask())) == 0)
        {
            if (!m_reportHucStatus)
            {
                WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_HUC_LOAD_STATUS_ID, 1, m_osInterface->pOsContext);
                m_reportHucStatus = true;
            }
            DECODE_ASSERTMESSAGE("Huc IMEM Loaded fails");
            MT_ERR1(MT_DEC_HEVC, MT_DEC_HUC_ERROR_STATUS2, (status.m_hucErrorStatus2 >> 32));
        }

        // Check Huc_status None Critical Error bit, bit 15. If 0, return error.
        if (((status.m_hucErrorStatus >> 32) & m_hwInterface->GetHucInterface()->GetHucStatusHevcS2lFailureMask()) == 0)
        {
            if (!m_reportHucCriticalError)
            {
                WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_HUC_REPORT_CRITICAL_ERROR_ID, 1, m_osInterface->pOsContext);
                m_reportHucCriticalError = true;
            }
            DECODE_ASSERTMESSAGE("Huc Report Critical Error!");
            MT_ERR1(MT_DEC_HEVC, MT_DEC_HUC_STATUS_CRITICAL_ERROR, (status.m_hucErrorStatus >> 32));
        }
    }

    return MOS_STATUS_SUCCESS;
}
#endif

MOS_STATUS HevcPipelineM12::InitScalabOption(HevcBasicFeature &basicFeature)
{
    DECODE_FUNC_CALL();

    PCODEC_HEVC_PIC_PARAMS picParams = basicFeature.m_hevcPicParams;
    DECODE_CHK_NULL(picParams);

    HevcScalabilityPars scalPars;
    MOS_ZeroMemory(&scalPars, sizeof(scalPars));
    DECODE_CHK_STATUS(InitContexOption(scalPars));
    scalPars.isSCC         = (basicFeature.m_hevcSccPicParams != nullptr);
#ifdef _DECODE_PROCESSING_SUPPORTED
    DecodeDownSamplingFeature* downSamplingFeature = dynamic_cast<DecodeDownSamplingFeature*>(
        m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
    if (downSamplingFeature != nullptr && downSamplingFeature->IsEnabled())
    {
        scalPars.usingSfc = true;
        if (!MEDIA_IS_SKU(m_skuTable, FtrSfcScalability))
        {
            scalPars.disableScalability = true;
        }
    }
    //Disable Scalability when histogram is enabled
    if (downSamplingFeature != nullptr && (downSamplingFeature->m_histogramDestSurf || downSamplingFeature->m_histogramDebug))
    {
        scalPars.disableScalability = true;
    }
#endif
    scalPars.maxTileColumn = HEVC_NUM_MAX_TILE_COLUMN;
    scalPars.maxTileRow    = HEVC_NUM_MAX_TILE_ROW;
#if (_DEBUG || _RELEASE_INTERNAL)
    scalPars.disableRealTile =
        ReadUserFeature(m_userSettingPtr, "Disable HEVC Real Tile Decode", MediaUserSetting::Group::Sequence).Get<bool>();
    bool enableRealTileMultiPhase =
        ReadUserFeature(m_userSettingPtr, "Enable HEVC Real Tile Multi Phase Decode", MediaUserSetting::Group::Sequence).Get<bool>();
        if (!enableRealTileMultiPhase)
    {
        scalPars.maxTileColumn = 2;
    }

    scalPars.userPipeNum =
        uint8_t(ReadUserFeature(m_userSettingPtr, "HCP Decode User Pipe Num", MediaUserSetting::Group::Sequence).Get<uint32_t>());
#endif
    // Long format real tile requires subset params
    if (!basicFeature.m_shortFormatInUse && basicFeature.m_hevcSubsetParams == nullptr)
    {
        scalPars.disableRealTile = true;
    }

    if (MEDIA_IS_SKU(m_skuTable, FtrVirtualTileScalabilityDisable))
    {
        scalPars.disableVirtualTile = true;
    }

    scalPars.surfaceFormat  = basicFeature.m_destSurface.Format;
    scalPars.frameWidth     = basicFeature.m_width;
    scalPars.frameHeight    = basicFeature.m_height;
    scalPars.numVdbox       = m_numVdbox;
    scalPars.numTileRows    = picParams->tiles_enabled_flag ?
                                (picParams->num_tile_rows_minus1 + 1) : 0;
    scalPars.numTileColumns = picParams->tiles_enabled_flag ?
                                (picParams->num_tile_columns_minus1 + 1) : 0;

    // HEVC 422 8b/10b && <8k - disable virtual tile sclability
    if (MEDIA_IS_SKU(m_skuTable, FtrDecodeHEVC422VTScalaDisable))
    {
        if ((scalPars.surfaceFormat == Format_YUY2 || scalPars.surfaceFormat == Format_Y210) &&
            ((scalPars.frameWidth * scalPars.frameHeight) < (7680 * 4320)))
        {
            scalPars.disableVirtualTile = true;
            SCALABILITY_VERBOSEMESSAGE("HEVC 422 && Resolution < 8k - Disable VT Scalability ");
        }
    }

    DECODE_CHK_STATUS(m_scalabOption.SetScalabilityOption(&scalPars));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipelineM12::AllocateResources(HevcBasicFeature &basicFeature)
{
    DECODE_FUNC_CALL();

    PCODEC_HEVC_PIC_PARAMS picParams = basicFeature.m_hevcPicParams;
    DECODE_CHK_NULL(picParams);

    uint32_t sliceStatesSize    = 0;
    uint32_t slicePatchListSize = 0;
    DECODE_CHK_STATUS(m_hwInterface->GetHcpPrimitiveCommandSize(
        CODECHAL_DECODE_MODE_HEVCVLD, &sliceStatesSize, &slicePatchListSize, false));

    uint32_t count, size;
    if (realTileDecodeMode == m_decodeMode)
    {
        count = picParams->num_tile_columns_minus1 + 1;
        size = sliceStatesSize * (basicFeature.m_numSlices + 1 + picParams->num_tile_rows_minus1);
    }
    else if (separateTileDecodeMode == m_decodeMode)
    {
        count = 1;
        uint32_t tileNum = (1 + picParams->num_tile_rows_minus1) *
                           (1 + picParams->num_tile_columns_minus1);
        size = sliceStatesSize * (basicFeature.m_numSlices + tileNum);
    }
    else
    {
        count = 1;
        size = sliceStatesSize * basicFeature.m_numSlices;
    }

    // In hevc short format decode, second level command buffer is programmed by Huc, so not need lock it.
    // In against hevc long format decode driver have to program second level command buffer, so it should
    // be lockable.
    if (m_secondLevelBBArray == nullptr)
    {
        m_secondLevelBBArray = m_allocator->AllocateBatchBufferArray(
            size, count, m_secondLevelBBNum, true, basicFeature.m_shortFormatInUse ? notLockableVideoMem : lockableVideoMem);
        DECODE_CHK_NULL(m_secondLevelBBArray);
        PMHW_BATCH_BUFFER &batchBuf = m_secondLevelBBArray->Fetch();
        DECODE_CHK_NULL(batchBuf);
    }
    else
    {
        PMHW_BATCH_BUFFER &batchBuf = m_secondLevelBBArray->Fetch();
        DECODE_CHK_NULL(batchBuf);
        DECODE_CHK_STATUS(m_allocator->Resize(
            batchBuf, size, count, basicFeature.m_shortFormatInUse ? notLockableVideoMem : lockableVideoMem));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipelineM12::Prepare(void *params)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(params);
    DecodePipelineParams *pipelineParams = (DecodePipelineParams *)params;
    m_pipeMode = pipelineParams->m_pipeMode;

    PERF_UTILITY_AUTO((__FUNCTION__ + std::to_string((int)m_pipeMode)).c_str(), PERF_DECODE, PERF_LEVEL_HAL);

    if (IsFirstProcessPipe(*pipelineParams))
    {
        DECODE_CHK_STATUS(HevcPipeline::Prepare(params));
        DECODE_CHK_STATUS(HevcPipeline::DestoryPhaseList());
    }

    DECODE_CHK_STATUS(m_preSubPipeline->Prepare(*pipelineParams));
    DECODE_CHK_STATUS(m_postSubPipeline->Prepare(*pipelineParams));

    if (m_pipeMode == decodePipeModeProcess)
    {
        if (IsCompleteBitstream())
        {
            if (m_pCodechalOcaDumper)
            {
                m_pCodechalOcaDumper->SetHevcDecodeParam(
                    m_basicFeature->m_hevcPicParams,
                    m_basicFeature->m_hevcRextPicParams,
                    m_basicFeature->m_hevcSccPicParams,
                    m_basicFeature->m_hevcSliceParams,
                    m_basicFeature->m_hevcRextSliceParams,
                    m_basicFeature->m_numSlices,
                    m_basicFeature->m_shortFormatInUse);
            }

            CODECHAL_DEBUG_TOOL(
                m_debugInterface->m_bufferDumpFrameNum = m_basicFeature->m_frameNum;
                DECODE_CHK_STATUS(DumpParams(*m_basicFeature))
                );

            DecodeStatusParameters inputParameters = {};
            MOS_ZeroMemory(&inputParameters, sizeof(DecodeStatusParameters));
            inputParameters.statusReportFeedbackNumber = m_basicFeature->m_hevcPicParams->StatusReportFeedbackNumber;
            inputParameters.pictureCodingType          = m_basicFeature->m_pictureCodingType;
            inputParameters.codecFunction              = m_basicFeature->m_codecFunction;
            inputParameters.picWidthInMb               = m_basicFeature->m_picWidthInMb;
            inputParameters.currOriginalPic            = m_basicFeature->m_curRenderPic;
            inputParameters.numUsedVdbox               = m_numVdbox;
            inputParameters.numSlices                  = m_basicFeature->m_numSlices;
            inputParameters.currDecodedPicRes          = m_basicFeature->m_destSurface.OsResource;

            CODECHAL_DEBUG_TOOL(
                if (m_streamout != nullptr)  
                {  
                    DECODE_CHK_STATUS(m_streamout->InitStatusReportParam(inputParameters));  
                }  
            );

#if (_DEBUG || _RELEASE_INTERNAL)
#ifdef _DECODE_PROCESSING_SUPPORTED
            DecodeDownSamplingFeature* downSamplingFeature = dynamic_cast<DecodeDownSamplingFeature*>(
                m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
            if (downSamplingFeature != nullptr)
            {
                auto frameIdx = m_basicFeature->m_curRenderPic.FrameIdx;
                inputParameters.sfcOutputSurface = &downSamplingFeature->m_outputSurfaceList[frameIdx];
                if (downSamplingFeature->m_histogramBuffer != nullptr)
                {
                    inputParameters.histogramOutputBuf = &downSamplingFeature->m_histogramBuffer->OsResource;
                }
                CODECHAL_DEBUG_TOOL(DumpDownSamplingParams(*downSamplingFeature));
            }
#endif
#endif
            m_statusReport->Init(&inputParameters);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipelineM12::Execute()
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO((__FUNCTION__ + std::to_string((int)m_pipeMode)).c_str(), PERF_DECODE, PERF_LEVEL_HAL);

    if (m_pipeMode == decodePipeModeProcess)
    {
        DECODE_CHK_STATUS(m_preSubPipeline->Execute());

        if (IsCompleteBitstream())
        {
            DECODE_CHK_STATUS(InitScalabOption(*m_basicFeature));
            DECODE_CHK_STATUS(InitDecodeMode(m_scalabOption.GetMode(), *m_basicFeature));
            if (m_decodeMode == realTileDecodeMode || m_decodeMode == separateTileDecodeMode)
            {
                DECODE_CHK_STATUS(m_basicFeature->m_tileCoding.UpdateSliceTileInfo());
            }

            DECODE_CHK_STATUS(AllocateResources(*m_basicFeature));
            DECODE_CHK_STATUS(HevcPipeline::CreatePhaseList(
                *m_basicFeature, m_scalabOption.GetMode(), m_scalabOption.GetNumPipe()));
            DECODE_CHK_STATUS(HevcPipeline::Execute());

#if (_DEBUG || _RELEASE_INTERNAL)
            DECODE_CHK_STATUS(StatusCheck());
#endif

            // Recover RefList for SCC IBC mode
            DECODE_CHK_STATUS(StoreDestToRefList(*m_basicFeature));

            CODECHAL_DEBUG_TOOL(DECODE_CHK_STATUS(DumpSecondLevelBatchBuffer()));

            // Only update user features for first frame.
            if (m_basicFeature->m_frameNum == 0)
            {
                DECODE_CHK_STATUS(UserFeatureReport());
            }

            DecodeFrameIndex++;
            m_basicFeature->m_frameNum = DecodeFrameIndex;

            DECODE_CHK_STATUS(m_statusReport->Reset());

            DECODE_CHK_STATUS(DestoryPhaseList());
        }
        DECODE_CHK_STATUS(m_postSubPipeline->Execute());
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipelineM12::GetStatusReport(void *status, uint16_t numStatus)
{
    DECODE_FUNC_CALL();
    m_statusReport->GetReport(numStatus, status);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipelineM12::Destroy()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(m_allocator->Destroy(m_secondLevelBBArray));
    DECODE_CHK_STATUS(Uninitialize());

    m_osInterface->pfnSetMultiEngineEnabled(m_osInterface, COMPONENT_Decode, false);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipelineM12::CreateFeatureManager()
{
    DECODE_FUNC_CALL();
    m_featureManager = MOS_New(DecodeHevcFeatureManager, m_allocator, m_hwInterface, m_osInterface);
    DECODE_CHK_NULL(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipelineM12::Initialize(void *settings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(settings);

    DECODE_CHK_STATUS(MediaPipeline::InitPlatform());
    DECODE_CHK_STATUS(MediaPipeline::CreateMediaCopyWrapper());
    DECODE_CHK_NULL(m_mediaCopyWrapper);

    DECODE_CHK_NULL(m_waTable);

    auto *codecSettings = (CodechalSetting *)settings;
    DECODE_CHK_NULL(m_hwInterface);
    DECODE_CHK_STATUS(m_hwInterface->Initialize(codecSettings));

    if (m_mediaCopyWrapper->MediaCopyStateIsNull())
    {
        m_mediaCopyWrapper->SetMediaCopyState(m_hwInterface->CreateMediaCopy(m_osInterface));
    }

    CODECHAL_DEBUG_TOOL(
        m_debugInterface = MOS_New(CodechalDebugInterface);
        DECODE_CHK_NULL(m_debugInterface);
        DECODE_CHK_STATUS(
            m_debugInterface->Initialize(m_hwInterface, codecSettings->codecFunction, m_mediaCopyWrapper)););

    if (m_hwInterface->m_hwInterfaceNext)
    {
        m_hwInterface->m_hwInterfaceNext->legacyHwInterface = m_hwInterface;
    }
    m_mediaContext = MOS_New(MediaContext, scalabilityDecoder, m_hwInterface->m_hwInterfaceNext, m_osInterface);
    DECODE_CHK_NULL(m_mediaContext);

    m_task = CreateTask(MediaTask::TaskType::cmdTask);
    DECODE_CHK_NULL(m_task);

    m_numVdbox = GetSystemVdboxNumber();

    bool limitedLMemBar = MEDIA_IS_SKU(m_skuTable, FtrLimitedLMemBar) ? true : false;
    m_allocator         = MOS_New(DecodeAllocator, m_osInterface, limitedLMemBar);
    DECODE_CHK_NULL(m_allocator);

    DECODE_CHK_STATUS(CreateStatusReport());

    m_decodecp = Create_DecodeCpInterface(codecSettings, m_hwInterface->GetCpInterface(), m_hwInterface->GetOsInterface());
    if (m_decodecp)
    {
        DECODE_CHK_STATUS(m_decodecp->RegisterParams(codecSettings));
    }
    DECODE_CHK_STATUS(CreateFeatureManager());
    DECODE_CHK_STATUS(m_featureManager->Init(codecSettings));

    DECODE_CHK_STATUS(CreateSubPipeLineManager(codecSettings));
    DECODE_CHK_STATUS(CreateSubPacketManager(codecSettings));

    // Create basic GPU context
    DecodeScalabilityPars scalPars;
    MOS_ZeroMemory(&scalPars, sizeof(scalPars));
    DECODE_CHK_STATUS(m_mediaContext->SwitchContext(VdboxDecodeFunc, &scalPars, &m_scalability));
    m_decodeContext = m_osInterface->pfnGetGpuContext(m_osInterface);

    m_basicFeature = dynamic_cast<HevcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(InitMmcState());
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipelineM12::Uninitialize()
{
    DECODE_FUNC_CALL();

#if (_DEBUG || _RELEASE_INTERNAL)
    // Report real tile frame count and virtual tile frame count
    MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;

    userFeatureWriteData.Value.i32Data = m_rtFrameCount;
    userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_ENABLE_HEVC_DECODE_RT_FRAME_COUNT_ID;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, m_osInterface->pOsContext);

    userFeatureWriteData.Value.i32Data = m_vtFrameCount;
    userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_ENABLE_HEVC_DECODE_VT_FRAME_COUNT_ID;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, m_osInterface->pOsContext);

    userFeatureWriteData.Value.i32Data = m_spFrameCount;
    userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_ENABLE_HEVC_DECODE_SP_FRAME_COUNT_ID;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, m_osInterface->pOsContext);
#endif

    for (auto pair : m_packetList)
    {
        pair.second->Destroy();
    }

#ifdef _MMC_SUPPORTED
    if (m_mmcState != nullptr)
    {
        MOS_Delete(m_mmcState);
    }
#endif

    return HevcPipeline::Uninitialize();
}

MOS_STATUS HevcPipelineM12::UserFeatureReport()
{
    DECODE_FUNC_CALL();
    return HevcPipeline::UserFeatureReport();
}

uint32_t HevcPipelineM12::GetCompletedReport()
{
    DECODE_FUNC_CALL();

    uint32_t completedCount = m_statusReport->GetCompletedCount();
    uint32_t reportedCount = m_statusReport->GetReportedCount();

    if (reportedCount > completedCount)
    {
        DECODE_ASSERTMESSAGE("No report available at all");
        return 0;
    }
    else
    {
        uint32_t availableCount = completedCount - reportedCount;
        return availableCount;
    }
}

MOS_STATUS HevcPipelineM12::CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings)
{
    DecodePredicationPktG12 *predicationPkt = MOS_New(DecodePredicationPktG12, this, m_hwInterface);
    DECODE_CHK_NULL(predicationPkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, predicationSubPacketId), *predicationPkt));

    DecodeMarkerPktG12 *markerPkt = MOS_New(DecodeMarkerPktG12, this, m_hwInterface);
    DECODE_CHK_NULL(markerPkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, markerSubPacketId), *markerPkt));
    
#ifdef _DECODE_PROCESSING_SUPPORTED
    HevcDownSamplingPkt *downSamplingPkt = MOS_New(HevcDownSamplingPkt, this, *m_hwInterface);
    DECODE_CHK_NULL(downSamplingPkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, downSamplingSubPacketId), *downSamplingPkt));
#endif

    HevcDecodePicPktM12 *pictureDecodePkt = MOS_New(HevcDecodePicPktM12, this, m_hwInterface);
    DECODE_CHK_NULL(pictureDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
                        DecodePacketId(this, hevcPictureSubPacketId), *pictureDecodePkt));

    HevcDecodeSlcPktM12 *sliceDecodePkt = MOS_New(HevcDecodeSlcPktM12, this, m_hwInterface);
    DECODE_CHK_NULL(sliceDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
                        DecodePacketId(this, hevcSliceSubPacketId), *sliceDecodePkt));

    HevcDecodeTilePktM12 *tileDecodePkt = MOS_New(HevcDecodeTilePktM12, this, m_hwInterface);
    DECODE_CHK_NULL(tileDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
                        DecodePacketId(this, hevcTileSubPacketId), *tileDecodePkt));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipelineM12::CreatePostSubPipeLines(DecodeSubPipelineManager &subPipelineManager)
{
    DECODE_FUNC_CALL();

#ifdef _DECODE_PROCESSING_SUPPORTED
    auto sfcHistogramPostSubPipeline = MOS_New(DecodeSfcHistogramSubPipelineM12, this, m_task, m_numVdbox, m_hwInterface);
    DECODE_CHK_NULL(sfcHistogramPostSubPipeline);
    DECODE_CHK_STATUS(m_postSubPipeline->Register(*sfcHistogramPostSubPipeline));
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipelineM12::CreatePreSubPipeLines(DecodeSubPipelineManager &subPipelineManager)
{
    m_bitstream = MOS_New(DecodeInputBitstreamM12, this, m_task, m_numVdbox, m_hwInterface);
    DECODE_CHK_NULL(m_bitstream);
    DECODE_CHK_STATUS(subPipelineManager.Register(*m_bitstream));

    m_streamout = MOS_New(DecodeStreamOutM12, this, m_task, m_numVdbox, m_hwInterface);
    DECODE_CHK_NULL(m_streamout);
    DECODE_CHK_STATUS(subPipelineManager.Register(*m_streamout));
    return MOS_STATUS_SUCCESS;
}

#ifdef _MMC_SUPPORTED
MOS_STATUS HevcPipelineM12::InitMmcState()
{
    DECODE_FUNC_CALL();

    m_mmcState = MOS_New(HevcDecodeMemCompM12, m_hwInterface);
    DECODE_CHK_NULL(m_mmcState);

    DECODE_CHK_STATUS(m_basicFeature->SetMmcState(m_mmcState->IsMmcEnabled()));
    return MOS_STATUS_SUCCESS;
}
#endif

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS HevcPipelineM12::DumpParams(HevcBasicFeature &basicFeature)
{
    m_debugInterface->m_bufferDumpFrameNum = basicFeature.m_frameNum;
    m_debugInterface->m_currPic            = basicFeature.m_curRenderPic;
    m_debugInterface->m_secondField        = basicFeature.m_secondField;
    m_debugInterface->m_frameType          = basicFeature.m_pictureCodingType;

    DECODE_CHK_STATUS(DumpPicParams(
        basicFeature.m_hevcPicParams, 
        basicFeature.m_hevcRextPicParams, 
        basicFeature.m_hevcSccPicParams));

    DECODE_CHK_STATUS(DumpSliceParams(
        basicFeature.m_hevcSliceParams, 
        basicFeature.m_hevcRextSliceParams, 
        basicFeature.m_numSlices, 
        basicFeature.m_shortFormatInUse));

    DECODE_CHK_STATUS(DumpIQParams(basicFeature.m_hevcIqMatrixParams));

    DECODE_CHK_STATUS(DumpBitstream(&basicFeature.m_resDataBuffer.OsResource, basicFeature.m_dataSize, 0));

    if (!basicFeature.m_shortFormatInUse)
    {
        DECODE_CHK_STATUS(DumpSubsetsParams(basicFeature.m_hevcSubsetParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipelineM12::DumpSecondLevelBatchBuffer()
{
    DECODE_CHK_STATUS(HevcPipeline::DumpSecondLevelBatchBuffer());

    if (m_basicFeature->m_shortFormatInUse)
    {
        // Dump HuC auth chained BB
        auto hucS2LPkt = dynamic_cast<HucS2lPktM12 *>(GetOrCreate(DecodePacketId(this, hucS2lPacketId)));
        DECODE_CHK_NULL(hucS2LPkt);

        PMHW_BATCH_BUFFER batchBuffer = hucS2LPkt->GetHucAuthCmdBuffer();

        if (batchBuffer != nullptr)
        {
            batchBuffer->iLastCurrent = batchBuffer->iSize * batchBuffer->count;
            batchBuffer->dwOffset     = 0;
            DECODE_CHK_STATUS(m_debugInterface->Dump2ndLvlBatch(
                batchBuffer,
                CODECHAL_NUM_MEDIA_STATES,
                "HEVC_DEC_HucAuth"));
        }
    }

    return MOS_STATUS_SUCCESS;
}
#endif

}
