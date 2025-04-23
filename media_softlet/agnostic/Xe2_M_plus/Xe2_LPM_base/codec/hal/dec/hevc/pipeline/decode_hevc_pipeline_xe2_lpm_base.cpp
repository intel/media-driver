/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file     decode_hevc_pipeline_xe2_lpm_base.cpp
//! \brief    Defines the interface for hevc decode pipeline
//!
#include "decode_hevc_pipeline_xe2_lpm_base.h"
#include "decode_huc_s2l_packet_xe2_lpm_base.h"
#include "decode_hevc_packet_front_end_xe2_lpm_base.h"
#include "decode_hevc_packet_back_end_xe2_lpm_base.h"
#include "decode_hevc_packet_real_tile_xe2_lpm_base.h"
#include "decode_hevc_picture_packet_xe2_lpm_base.h"
#include "decode_hevc_slice_packet_xe2_lpm_base.h"
#include "decode_utils.h"
#include "decode_common_feature_defs.h"
#include "decode_hevc_mem_compression_xe2_lpm_base.h"

namespace decode {

HevcPipelineXe2_Lpm_Base::HevcPipelineXe2_Lpm_Base(CodechalHwInterfaceNext *hwInterface, CodechalDebugInterface *debugInterface)
    : HevcPipeline(hwInterface, debugInterface)
{
}

MOS_STATUS HevcPipelineXe2_Lpm_Base::Init(void *settings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(settings);

    DECODE_CHK_STATUS(Initialize(settings));

    if (m_basicFeature->m_shortFormatInUse)
    {
        HucS2lPktXe2_Lpm_Base *hucS2lPkt = MOS_New(HucS2lPktXe2_Lpm_Base, this, m_task, m_hwInterface);
        DECODE_CHK_NULL(hucS2lPkt);
        DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, hucS2lPacketId), hucS2lPkt));
        DECODE_CHK_STATUS(hucS2lPkt->Init());
    }

    auto hevcDecodePktLong = MOS_New(HevcDecodeLongPktXe2_Lpm_Base, this, m_task, m_hwInterface);
    DECODE_CHK_NULL(hevcDecodePktLong);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, hevcLongPacketId), hevcDecodePktLong));
    DECODE_CHK_STATUS(hevcDecodePktLong->Init());

    auto hevcDecodePktFrontEnd = MOS_New(HevcDecodeFrontEndPktXe2_Lpm_Base, this, m_task, m_hwInterface);
    DECODE_CHK_NULL(hevcDecodePktFrontEnd);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, hevcFrontEndPacketId), hevcDecodePktFrontEnd));
    DECODE_CHK_STATUS(hevcDecodePktFrontEnd->Init());

    auto hevcDecodePktBackEnd = MOS_New(HevcDecodeBackEndPktXe2_Lpm_Base, this, m_task, m_hwInterface);
    DECODE_CHK_NULL(hevcDecodePktBackEnd);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, hevcBackEndPacketId), hevcDecodePktBackEnd));
    DECODE_CHK_STATUS(hevcDecodePktBackEnd->Init());

    auto hevcDecodePktRealTile = MOS_New(HevcDecodeRealTilePktXe2_Lpm_Base, this, m_task, m_hwInterface);
    DECODE_CHK_NULL(hevcDecodePktRealTile);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, hevcRealTilePacketId), hevcDecodePktRealTile));
    DECODE_CHK_STATUS(hevcDecodePktRealTile->Init());

    if (m_numVdbox == 2)
    {
        m_allowVirtualNodeReassign = true;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipelineXe2_Lpm_Base::InitScalabOption(HevcBasicFeature &basicFeature)
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
        ReadUserFeature(m_userSettingPtr, "HCP Decode User Pipe Num", MediaUserSetting::Group::Sequence).Get<uint8_t>();
#endif
    // Long format real tile requires subset params
    if (!basicFeature.m_shortFormatInUse && basicFeature.m_hevcSubsetParams == nullptr)
    {
        scalPars.disableRealTile = true;
    }

    scalPars.disableVirtualTile = true;

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

MOS_STATUS HevcPipelineXe2_Lpm_Base::AllocateResources(HevcBasicFeature &basicFeature)
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

MOS_STATUS HevcPipelineXe2_Lpm_Base::Prepare(void *params)
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

MOS_STATUS HevcPipelineXe2_Lpm_Base::Execute()
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
#ifdef _MMC_SUPPORTED
            if (m_mmcState != nullptr)
            {
                m_mmcState->ReportSurfaceMmcMode(&(m_basicFeature->m_destSurface));
            }
#endif
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

MOS_STATUS HevcPipelineXe2_Lpm_Base::GetStatusReport(void *status, uint16_t numStatus)
{
    DECODE_FUNC_CALL();
    m_statusReport->GetReport(numStatus, status);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipelineXe2_Lpm_Base::Destroy()
{
    DECODE_FUNC_CALL();
    if (m_allocator != nullptr)
    {
        DECODE_CHK_STATUS(m_allocator->Destroy(m_secondLevelBBArray));
    }
    DECODE_CHK_STATUS(Uninitialize());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipelineXe2_Lpm_Base::Initialize(void *settings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(HevcPipeline::Initialize(settings));
#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(InitMmcState());
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipelineXe2_Lpm_Base::Uninitialize()
{
    DECODE_FUNC_CALL();

#if (_DEBUG || _RELEASE_INTERNAL)
    // Report real tile frame count and virtual tile frame count
    ReportUserSettingForDebug(
        m_userSettingPtr,
        "RT Decoded Count",
        m_rtFrameCount,
        MediaUserSetting::Group::Sequence);
    ReportUserSettingForDebug(
        m_userSettingPtr,
        "VT Decoded Count",
        m_vtFrameCount,
        MediaUserSetting::Group::Sequence);
    ReportUserSettingForDebug(
        m_userSettingPtr,
        "SP Decoded Count",
        m_spFrameCount,
        MediaUserSetting::Group::Sequence);
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

MOS_STATUS HevcPipelineXe2_Lpm_Base::UserFeatureReport()
{
    DECODE_FUNC_CALL();

    return HevcPipeline::UserFeatureReport();
}

uint32_t HevcPipelineXe2_Lpm_Base::GetCompletedReport()
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

MOS_STATUS HevcPipelineXe2_Lpm_Base::CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings)
{
    DECODE_CHK_STATUS(HevcPipeline::CreateSubPackets(subPacketManager, codecSettings));

    HevcDecodePicPktXe2_Lpm_Base *pictureDecodePkt = MOS_New(HevcDecodePicPktXe2_Lpm_Base, this, m_hwInterface);
    DECODE_CHK_NULL(pictureDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
                        DecodePacketId(this, hevcPictureSubPacketId), *pictureDecodePkt));

    HevcDecodeSlcPktXe2_Lpm_Base *sliceDecodePkt = MOS_New(HevcDecodeSlcPktXe2_Lpm_Base, this, m_hwInterface);
    DECODE_CHK_NULL(sliceDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
                        DecodePacketId(this, hevcSliceSubPacketId), *sliceDecodePkt));

    HevcDecodeTilePktXe2_Lpm_Base *tileDecodePkt = MOS_New(HevcDecodeTilePktXe2_Lpm_Base, this, m_hwInterface);
    DECODE_CHK_NULL(tileDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
                        DecodePacketId(this, hevcTileSubPacketId), *tileDecodePkt));

    return MOS_STATUS_SUCCESS;
}

#ifdef _MMC_SUPPORTED
MOS_STATUS HevcPipelineXe2_Lpm_Base::InitMmcState()
{
    DECODE_FUNC_CALL();

    m_mmcState = MOS_New(HevcDecodeMemCompXe2_Lpm_Base, m_hwInterface);
    DECODE_CHK_NULL(m_mmcState);

    DECODE_CHK_STATUS(m_basicFeature->SetMmcState(m_mmcState->IsMmcEnabled()));
    return MOS_STATUS_SUCCESS;
}
#endif

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS HevcPipelineXe2_Lpm_Base::DumpParams(HevcBasicFeature &basicFeature)
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
#endif

uint8_t HevcPipelineXe2_Lpm_Base::GetSystemVdboxNumber()
{
    uint8_t numVdbox = 1;

    MEDIA_ENGINE_INFO mediaSysInfo;
    MOS_ZeroMemory(&mediaSysInfo, sizeof(MEDIA_ENGINE_INFO));
    MOS_STATUS eStatus = m_osInterface->pfnGetMediaEngineInfo(m_osInterface, mediaSysInfo);
    if (eStatus == MOS_STATUS_SUCCESS)
    {
        numVdbox = (uint8_t)(mediaSysInfo.VDBoxInfo.NumberOfVDBoxEnabled);
    }
    else
    {
        DECODE_ASSERTMESSAGE("Failed to query media engine info!!");
    }

    return numVdbox;
}

}
