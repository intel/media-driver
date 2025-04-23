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
//! \file     decode_vp9_pipeline_M12.cpp
//! \brief    Defines the interface for vp9 decode pipeline
//!
#include "decode_vp9_pipeline_m12.h"
#include "decode_vp9_packet_single_m12.h"
#include "decode_vp9_packet_front_end_m12.h"
#include "decode_vp9_packet_back_end_m12.h"
#include "decode_vp9_picture_packet_m12.h"
#include "decode_vp9_slice_packet_m12.h"
#include "decode_utils.h"
#include "codechal_debug.h"
#include "decode_mem_compression_g12.h"
#include "decode_vp9_mem_compression_m12.h"
#include "decode_vp9_downsampling_packet.h"
#include "decode_common_feature_defs.h"
#include "decode_resource_auto_lock.h"
#include "decode_vp9_feature_manager_m12.h"
#include "decode_vp9_buffer_update_m12.h"
#include "decode_sfc_histogram_postsubpipeline_m12.h"
#include "decode_input_bitstream_m12.h"
#include "decode_cp_bitstream_m12.h"
#include "decode_marker_packet_g12.h"
#include "decode_predication_packet_g12.h"
#include "mos_interface.h"

namespace decode
{
Vp9PipelineG12::Vp9PipelineG12(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : Vp9Pipeline(*hwInterface, debugInterface)
{
    m_hwInterface = hwInterface;
}

MOS_STATUS Vp9PipelineG12::Init(void *settings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(settings);
    DECODE_CHK_STATUS(Initialize(settings));

    auto vp9DecodePktSingle = MOS_New(Vp9DecodeSinglePktM12, this, m_task, m_hwInterface);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, vp9SinglePacketId), vp9DecodePktSingle));
    DECODE_CHK_STATUS(vp9DecodePktSingle->Init());

    auto vp9DecodePktFrontEnd = MOS_New(Vp9DecodeFrontEndPktM12, this, m_task, m_hwInterface);
    DECODE_CHK_NULL(vp9DecodePktFrontEnd);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, vp9FrontEndPacketId), vp9DecodePktFrontEnd));
    DECODE_CHK_STATUS(vp9DecodePktFrontEnd->Init());

    auto vp9DecodePktBackEnd = MOS_New(Vp9DecodeBackEndPktM12, this, m_task, m_hwInterface);
    DECODE_CHK_NULL(vp9DecodePktBackEnd);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, vp9BackEndPacketId), vp9DecodePktBackEnd));
    DECODE_CHK_STATUS(vp9DecodePktBackEnd->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PipelineG12::Prepare(void *params)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(params);
    DecodePipelineParams *pipelineParams = (DecodePipelineParams *)params;
    m_pipeMode                           = pipelineParams->m_pipeMode;

    PERF_UTILITY_AUTO((__FUNCTION__ + std::to_string((int)m_pipeMode)).c_str(), PERF_DECODE, PERF_LEVEL_HAL);

    if (IsFirstProcessPipe(*pipelineParams))
    {
        DECODE_CHK_STATUS(Vp9Pipeline::Prepare(params));
    }

    DECODE_CHK_STATUS(m_preSubPipeline->Prepare(*pipelineParams));
    DECODE_CHK_STATUS(m_postSubPipeline->Prepare(*pipelineParams));

    if (m_pipeMode == decodePipeModeProcess)
    {
        if (IsCompleteBitstream())
        {
            CODECHAL_DEBUG_TOOL(
                m_debugInterface->m_bufferDumpFrameNum = m_basicFeature->m_frameNum;
                DECODE_CHK_STATUS(DumpParams(*m_basicFeature));
                );

            DecodeStatusParameters inputParameters = {};
            MOS_ZeroMemory(&inputParameters, sizeof(DecodeStatusParameters));
            inputParameters.statusReportFeedbackNumber = m_basicFeature->m_vp9PicParams->StatusReportFeedbackNumber;
            inputParameters.codecFunction              = m_basicFeature->m_codecFunction;
            inputParameters.picWidthInMb               = m_basicFeature->m_picWidthInMb;
            inputParameters.pictureCodingType          = m_basicFeature->m_pictureCodingType;
            inputParameters.currOriginalPic            = m_basicFeature->m_curRenderPic;
            inputParameters.currDecodedPicRes          = m_basicFeature->m_destSurface.OsResource;
            inputParameters.numUsedVdbox               = m_numVdbox;
            
            CODECHAL_DEBUG_TOOL(
                if (m_streamout != nullptr)  
                {  
                    DECODE_CHK_STATUS(m_streamout->InitStatusReportParam(inputParameters));  
                }  
            );

#if (_DEBUG || _RELEASE_INTERNAL)
#ifdef _DECODE_PROCESSING_SUPPORTED
            DecodeDownSamplingFeature *downSamplingFeature = dynamic_cast<DecodeDownSamplingFeature *>(
                m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
            if (downSamplingFeature != nullptr)
            {
                auto frameIdx                   = m_basicFeature->m_curRenderPic.FrameIdx;
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

MOS_STATUS Vp9PipelineG12::InitContexOption(Vp9BasicFeature &basicFeature)
{
    DecodeScalabilityPars scalPars;
    MOS_ZeroMemory(&scalPars, sizeof(scalPars));

    scalPars.usingHcp           = true;
    scalPars.enableVE           = MOS_VE_SUPPORTED(m_osInterface);
    scalPars.disableScalability = m_hwInterface->IsDisableScalability();
    scalPars.surfaceFormat      = basicFeature.m_destSurface.Format;
    scalPars.frameWidth         = basicFeature.m_frameWidthAlignedMinBlk;
    scalPars.frameHeight        = basicFeature.m_frameHeightAlignedMinBlk;
    scalPars.numVdbox           = m_numVdbox;
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
    scalPars.userPipeNum =
        ReadUserFeature(m_userSettingPtr, "HCP Decode User Pipe Num", MediaUserSetting::Group::Sequence).Get<uint8_t>();
#endif

#ifdef _DECODE_PROCESSING_SUPPORTED
    DecodeDownSamplingFeature *downSamplingFeature = dynamic_cast<DecodeDownSamplingFeature *>(
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

    if (MEDIA_IS_SKU(m_skuTable, FtrVirtualTileScalabilityDisable))
    {
        scalPars.disableScalability = true;
        scalPars.disableVirtualTile = true;
    }

    if (!scalPars.disableScalability)
        m_osInterface->pfnSetMultiEngineEnabled(m_osInterface, COMPONENT_Decode, true);

    DECODE_CHK_STATUS(m_scalabOption.SetScalabilityOption(&scalPars));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PipelineG12::Execute()
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO((__FUNCTION__ + std::to_string((int)m_pipeMode)).c_str(), PERF_DECODE, PERF_LEVEL_HAL);

    if (m_pipeMode == decodePipeModeProcess)
    {
        DECODE_CHK_STATUS(m_preSubPipeline->Execute());

        if (IsCompleteBitstream())
        {
            DECODE_CHK_STATUS(InitContexOption(*m_basicFeature));
            DECODE_CHK_STATUS(InitDecodeMode(m_scalabOption.GetMode()));
            DECODE_CHK_STATUS(Vp9Pipeline::CreatePhaseList(
                m_scalabOption.GetMode(), m_scalabOption.GetNumPipe()));
            DECODE_CHK_STATUS(Vp9Pipeline::Execute());

#if (_DEBUG || _RELEASE_INTERNAL)
            DECODE_CHK_STATUS(StatusCheck());
#endif
            // Only update user features for the first frame.
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

MOS_STATUS Vp9PipelineG12::GetStatusReport(void *status, uint16_t numStatus)
{
    DECODE_FUNC_CALL();

    m_statusReport->GetReport(numStatus, status);

    return MOS_STATUS_SUCCESS;
}

uint32_t Vp9PipelineG12::GetCompletedReport()
{
    DECODE_FUNC_CALL();

    uint32_t completedCount = m_statusReport->GetCompletedCount();
    uint32_t reportedCount  = m_statusReport->GetReportedCount();

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

MOS_STATUS Vp9PipelineG12::Destroy()
{
    DECODE_FUNC_CALL();

    Uninitialize();

    m_osInterface->pfnSetMultiEngineEnabled(m_osInterface, COMPONENT_Decode, false);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PipelineG12::CreateFeatureManager()
{
    DECODE_FUNC_CALL();
    m_featureManager = MOS_New(DecodeVp9FeatureManagerM12, m_allocator, m_hwInterface, m_osInterface);
    DECODE_CHK_NULL(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PipelineG12::Initialize(void *settings)
{
    DECODE_FUNC_CALL();

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

    m_basicFeature = dynamic_cast<Vp9BasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

    auto *bufferUpdatePipeline = MOS_New(DecodeVp9BufferUpdateM12, this, m_task, m_numVdbox, m_hwInterface);
    DECODE_CHK_NULL(bufferUpdatePipeline);
    DECODE_CHK_STATUS(m_preSubPipeline->Register(*bufferUpdatePipeline));
    DECODE_CHK_STATUS(bufferUpdatePipeline->Init(*codecSettings));
#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(InitMmcState());
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PipelineG12::Uninitialize()
{
    DECODE_FUNC_CALL();

    for (auto pair : m_packetList)
    {
        pair.second->Destroy();
    }

    if (m_mmcState != nullptr)
    {
        MOS_Delete(m_mmcState);
    }

    return Vp9Pipeline::Uninitialize();
}

MOS_STATUS Vp9PipelineG12::UserFeatureReport()
{
    DECODE_FUNC_CALL();

    return Vp9Pipeline::UserFeatureReport();
}

MOS_STATUS Vp9PipelineG12::CreateSubPackets(DecodeSubPacketManager &subPacketManager, CodechalSetting &codecSettings)
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
    Vp9DownSamplingPkt *downSamplingPkt = MOS_New(Vp9DownSamplingPkt, this, *m_hwInterface);
    DECODE_CHK_NULL(downSamplingPkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, downSamplingSubPacketId), *downSamplingPkt));
#endif

    Vp9DecodePicPktM12 *pictureDecodePkt = MOS_New(Vp9DecodePicPktM12, this, m_hwInterface);
    DECODE_CHK_NULL(pictureDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, vp9PictureSubPacketId), *pictureDecodePkt));

    //VP9 slice packet, only 1 slice
    Vp9DecodeSlcPktM12 *sliceDecodePkt = MOS_New(Vp9DecodeSlcPktM12, this, m_hwInterface);
    DECODE_CHK_NULL(sliceDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, vp9SliceSubPacketId), *sliceDecodePkt));

    Vp9DecodeTilePktM12 *tileDecodePkt = MOS_New(Vp9DecodeTilePktM12, this, m_hwInterface);
    DECODE_CHK_NULL(tileDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, vp9TileSubPacketId), *tileDecodePkt));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PipelineG12::CreatePostSubPipeLines(DecodeSubPipelineManager &subPipelineManager)
{
    DECODE_FUNC_CALL();

#ifdef _DECODE_PROCESSING_SUPPORTED
    auto sfcHistogramPostSubPipeline = MOS_New(DecodeSfcHistogramSubPipelineM12, this, m_task, m_numVdbox, m_hwInterface);
    DECODE_CHK_NULL(sfcHistogramPostSubPipeline);
    DECODE_CHK_STATUS(m_postSubPipeline->Register(*sfcHistogramPostSubPipeline));
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PipelineG12::CreatePreSubPipeLines(DecodeSubPipelineManager &subPipelineManager)
{
    m_bitstream = MOS_New(DecodeInputBitstreamM12, this, m_task, m_numVdbox, m_hwInterface);
    DECODE_CHK_NULL(m_bitstream);
    DECODE_CHK_STATUS(subPipelineManager.Register(*m_bitstream));

    m_streamout = MOS_New(DecodeStreamOutM12, this, m_task, m_numVdbox, m_hwInterface);
    DECODE_CHK_NULL(m_streamout);
    DECODE_CHK_STATUS(subPipelineManager.Register(*m_streamout));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PipelineG12::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    DECODE_CHK_NULL(m_hwInterface);
    m_mmcState = MOS_New(Vp9DecodeMemCompM12, m_hwInterface);
    DECODE_CHK_NULL(m_mmcState);
    DECODE_CHK_STATUS(m_basicFeature->SetMmcState(m_mmcState->IsMmcEnabled()));
#endif
    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Vp9PipelineG12::DumpParams(Vp9BasicFeature &basicFeature)
{
    basicFeature.m_vp9PicParams->CurrPic.PicFlags = PICTURE_FRAME;
    m_debugInterface->m_currPic                   = basicFeature.m_curRenderPic;
    m_debugInterface->m_frameType                 = basicFeature.m_pictureCodingType;
    m_debugInterface->m_secondField               = basicFeature.m_secondField;
    m_debugInterface->m_bufferDumpFrameNum        = basicFeature.m_frameNum;

    DECODE_CHK_STATUS(DumpPicParams(basicFeature.m_vp9PicParams));
    DECODE_CHK_STATUS(DumpSliceParams(basicFeature.m_vp9SliceParams));
    DECODE_CHK_STATUS(DumpSegmentParams(basicFeature.m_vp9SegmentParams));
    DECODE_CHK_STATUS(DumpBitstream(&basicFeature.m_resDataBuffer.OsResource, basicFeature.m_dataSize, 0));

    DECODE_CHK_STATUS(m_debugInterface->DumpBuffer(
        &(basicFeature.m_resVp9SegmentIdBuffer->OsResource),
        CodechalDbgAttr::attrSegId,
        "SegId_beforeHCP",
        (basicFeature.m_allocatedWidthInSb * basicFeature.m_allocatedHeightInSb * CODECHAL_CACHELINE_SIZE)));

    DECODE_CHK_STATUS(m_debugInterface->DumpBuffer(
         &(basicFeature.m_resVp9ProbBuffer[basicFeature.m_frameCtxIdx]->OsResource),
        CodechalDbgAttr::attrCoefProb,
        "PakHwCoeffProbs_beforeHCP",
        CODEC_VP9_PROB_MAX_NUM_ELEM));

    return MOS_STATUS_SUCCESS;
}
#endif

}  // namespace decode
