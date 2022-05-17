/*
* Copyright (c) 2020-2021, Intel Corporation
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

namespace decode
{
Vp9PipelineG12::Vp9PipelineG12(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : Vp9Pipeline(hwInterface, debugInterface)
{
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

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
            TraceDataDumpInternalBuffers(*m_basicFeature);
#endif

            DecodeStatusParameters inputParameters = {};
            MOS_ZeroMemory(&inputParameters, sizeof(DecodeStatusParameters));
            inputParameters.statusReportFeedbackNumber = m_basicFeature->m_vp9PicParams->StatusReportFeedbackNumber;
            inputParameters.codecFunction              = m_basicFeature->m_codecFunction;
            inputParameters.picWidthInMb               = m_basicFeature->m_picWidthInMb;
            inputParameters.pictureCodingType          = m_basicFeature->m_pictureCodingType;
            inputParameters.currOriginalPic            = m_basicFeature->m_curRenderPic;
            inputParameters.currDecodedPicRes          = m_basicFeature->m_destSurface.OsResource;
            inputParameters.numUsedVdbox               = m_numVdbox;
#if (_DEBUG || _RELEASE_INTERNAL)
#ifdef _DECODE_PROCESSING_SUPPORTED
            DecodeDownSamplingFeature *downSamplingFeature = dynamic_cast<DecodeDownSamplingFeature *>(
                m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
            if (downSamplingFeature != nullptr)
            {
                auto frameIdx                   = m_basicFeature->m_curRenderPic.FrameIdx;
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
            m_basicFeature->m_frameNum++;

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

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PipelineG12::Initialize(void *settings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(Vp9Pipeline::Initialize(settings));
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
    DECODE_CHK_STATUS(Vp9Pipeline::CreateSubPackets(subPacketManager, codecSettings));

#ifdef _DECODE_PROCESSING_SUPPORTED
    Vp9DownSamplingPkt *downSamplingPkt = MOS_New(Vp9DownSamplingPkt, this, m_hwInterface);
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

    DECODE_CHK_STATUS(DumpPicParams(
        basicFeature.m_vp9PicParams));

    DECODE_CHK_STATUS(DumpSegmentParams(
        basicFeature.m_vp9SegmentParams));

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

    //dump bitstream
    DECODE_CHK_STATUS(m_debugInterface->DumpBuffer(
        &basicFeature.m_resDataBuffer.OsResource, 
        CodechalDbgAttr::attrBitstream, 
        "_DEC", 
        basicFeature.m_dataSize, 0, CODECHAL_NUM_MEDIA_STATES));

    return MOS_STATUS_SUCCESS;
}
#endif

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
MOS_STATUS Vp9PipelineG12::TraceDataDumpInternalBuffers(Vp9BasicFeature &basicFeature)
{  
    if (MOS_TraceKeyEnabled(TR_KEY_DECODE_INTERNAL))
    {
        if (!m_allocator->ResourceIsNull(&(basicFeature.m_resVp9SegmentIdBuffer->OsResource)))
        {
            ResourceAutoLock resLock(m_allocator, &(basicFeature.m_resVp9SegmentIdBuffer->OsResource));
            auto             pData = (uint8_t *)resLock.LockResourceForRead();
            DECODE_CHK_NULL(pData);

            MOS_TraceDataDump(
                "Decode_Vp9SegmentIdBeforeHCP",
                0,
                pData,
                basicFeature.m_allocatedWidthInSb * basicFeature.m_allocatedHeightInSb * CODECHAL_CACHELINE_SIZE);

            m_allocator->UnLock(&(basicFeature.m_resVp9SegmentIdBuffer->OsResource));
        }

        if (!m_allocator->ResourceIsNull(&(basicFeature.m_resVp9ProbBuffer[basicFeature.m_frameCtxIdx]->OsResource)))
        {
            ResourceAutoLock resLock(m_allocator, &(basicFeature.m_resVp9ProbBuffer[basicFeature.m_frameCtxIdx]->OsResource));
            auto             pData = (uint8_t *)resLock.LockResourceForRead();
            DECODE_CHK_NULL(pData);

            MOS_TraceDataDump(
                "Decode_Vp9CoeffProbsBeforeHCP",
                0,
                pData,
                CODEC_VP9_PROB_MAX_NUM_ELEM);

            m_allocator->UnLock(&(basicFeature.m_resVp9ProbBuffer[basicFeature.m_frameCtxIdx]->OsResource));
        }
    }

    return MOS_STATUS_SUCCESS;
}
#endif

}  // namespace decode
