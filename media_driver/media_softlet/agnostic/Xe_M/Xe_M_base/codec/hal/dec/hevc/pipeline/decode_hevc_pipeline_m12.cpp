/*
* Copyright (c) 2018-2021, Intel Corporation
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
#include "media_user_settings_mgr_g12.h"
#include "decode_utils.h"
#include "decode_common_feature_defs.h"
#include "decode_hevc_mem_compression_m12.h"

namespace decode {

HevcPipelineM12::HevcPipelineM12(CodechalHwInterface *hwInterface, CodechalDebugInterface *debugInterface)
    : HevcPipeline(hwInterface, debugInterface)
{
}

MOS_STATUS HevcPipelineM12::Init(void *settings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(settings);

    DECODE_CHK_STATUS(Initialize(settings));

    if (m_shortFormatInUse)
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

    return MOS_STATUS_SUCCESS;
}

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
        ReadUserFeature(__MEDIA_USER_FEATURE_VALUE_DISABLE_HEVC_REALTILE_DECODE_ID, m_osInterface->pOsContext).u32Data ?
        true : false;

    bool enableRealTileMultiPhase =
        ReadUserFeature(__MEDIA_USER_FEATURE_VALUE_ENABLE_HEVC_REALTILE_MULTI_PHASE_DECODE_ID, m_osInterface->pOsContext).u32Data ?
        true : false;
    if (!enableRealTileMultiPhase)
    {
        scalPars.maxTileColumn = 2;
    }

    scalPars.userPipeNum =
        uint8_t(ReadUserFeature(__MEDIA_USER_FEATURE_VALUE_HCP_DECODE_USER_PIPE_NUM_ID, m_osInterface->pOsContext).u32Data);
#endif
    // Long format real tile requires subset params
    if (!m_shortFormatInUse && basicFeature.m_hevcSubsetParams == nullptr)
    {
        scalPars.disableRealTile = true;
    }
    scalPars.surfaceFormat  = basicFeature.m_destSurface.Format;
    scalPars.frameWidth     = basicFeature.m_width;
    scalPars.frameHeight    = basicFeature.m_height;
    scalPars.numVdbox       = m_numVdbox;
    scalPars.numTileRows    = picParams->tiles_enabled_flag ?
                                (picParams->num_tile_rows_minus1 + 1) : 0;
    scalPars.numTileColumns = picParams->tiles_enabled_flag ?
                                (picParams->num_tile_columns_minus1 + 1) : 0;

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
            size, count, m_secondLevelBBNum, true, m_shortFormatInUse ? notLockableVideoMem : lockableVideoMem);
        DECODE_CHK_NULL(m_secondLevelBBArray);
        PMHW_BATCH_BUFFER &batchBuf = m_secondLevelBBArray->Fetch();
        DECODE_CHK_NULL(batchBuf);
    }
    else
    {
        PMHW_BATCH_BUFFER &batchBuf = m_secondLevelBBArray->Fetch();
        DECODE_CHK_NULL(batchBuf);
        DECODE_CHK_STATUS(m_allocator->Resize(
            batchBuf, size, count, m_shortFormatInUse ? notLockableVideoMem : lockableVideoMem));
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
    }

    DECODE_CHK_STATUS(m_preSubPipeline->Prepare(*pipelineParams));
    DECODE_CHK_STATUS(m_postSubPipeline->Prepare(*pipelineParams));

    if (m_pipeMode == decodePipeModeProcess)
    {
        if (IsCompleteBitstream())
        {
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
#if (_DEBUG || _RELEASE_INTERNAL)
#ifdef _DECODE_PROCESSING_SUPPORTED
            DecodeDownSamplingFeature* downSamplingFeature = dynamic_cast<DecodeDownSamplingFeature*>(
                m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
            if (downSamplingFeature != nullptr)
            {
                auto frameIdx = m_basicFeature->m_curRenderPic.FrameIdx;
                inputParameters.sfcOutputPicRes = &downSamplingFeature->m_outputSurfaceList[frameIdx].OsResource;
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

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
            if (MOS_GetTraceEventKeyword() & EVENT_DECODE_BUFFER_KEYWORD)
            {
                TraceDataDump2ndLevelBB(GetSliceLvlCmdBuffer());
            }
#endif

            // Only update user features for first frame.
            if (m_basicFeature->m_frameNum == 0)
            {
                DECODE_CHK_STATUS(UserFeatureReport());
            }
            m_basicFeature->m_frameNum++;

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

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipelineM12::Initialize(void *settings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(HevcPipeline::Initialize(settings));
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
    DECODE_CHK_STATUS(HevcPipeline::CreateSubPackets(subPacketManager, codecSettings));

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

    DECODE_CHK_STATUS(m_debugInterface->DumpBuffer(
        &basicFeature.m_resDataBuffer.OsResource, CodechalDbgAttr::attrBitstream, "_DEC",
        basicFeature.m_dataSize, 0, CODECHAL_NUM_MEDIA_STATES));

    DECODE_CHK_STATUS(DumpPicParams(
        basicFeature.m_hevcPicParams,
        basicFeature.m_hevcRextPicParams,
        basicFeature.m_hevcSccPicParams));

    if (basicFeature.m_hevcIqMatrixParams != nullptr)
    {
        DECODE_CHK_STATUS(DumpIQParams(basicFeature.m_hevcIqMatrixParams));
    }

    if (basicFeature.m_hevcSliceParams != nullptr)
    {
        DECODE_CHK_STATUS(DumpSliceParams(
            basicFeature.m_hevcSliceParams,
            basicFeature.m_hevcRextSliceParams,
            basicFeature.m_numSlices,
            m_shortFormatInUse));
    }

    if(basicFeature.m_hevcSubsetParams != nullptr)
    {
        DECODE_CHK_STATUS(DumpSubsetsParams(basicFeature.m_hevcSubsetParams));
    }

    return MOS_STATUS_SUCCESS;
}

#endif

}
