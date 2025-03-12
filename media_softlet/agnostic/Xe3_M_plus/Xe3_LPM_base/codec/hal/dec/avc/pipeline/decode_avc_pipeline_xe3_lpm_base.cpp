/*
* Copyright (c) 2022-2024, Intel Corporation
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
//! \file     decode_avc_pipeline_xe3_lpm_base.cpp
//! \brief    Defines the interface for avc decode pipeline
//!
#include "decode_avc_pipeline_xe3_lpm_base.h"
#include "decode_utils.h"
#include "decode_mem_compression_xe3_lpm_base.h"
#include "decode_avc_packet_xe3_lpm_base.h"
#include "decode_avc_slice_packet_xe3_lpm_base.h"
#include "decode_avc_picture_packet_xe3_lpm_base.h"
#include "decode_common_feature_defs.h"
#include "decode_avc_downsampling_packet.h"
#include "decode_avc_feature_manager_xe3_lpm_base.h"

namespace decode {

AvcPipelineXe3_Lpm_Base::AvcPipelineXe3_Lpm_Base(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : AvcPipeline(hwInterface, debugInterface)
{

}

MOS_STATUS AvcPipelineXe3_Lpm_Base::Init(void *settings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(settings);
    DECODE_CHK_STATUS(Initialize(settings));

    m_avcDecodePkt = MOS_New(AvcDecodePktXe3_Lpm_Base, this, m_task, m_hwInterface);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, avcDecodePacketId), m_avcDecodePkt));
    DECODE_CHK_STATUS(m_avcDecodePkt->Init());

    if (m_numVdbox == 2)
    {
        m_allowVirtualNodeReassign = true;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcPipelineXe3_Lpm_Base::GetStatusReport(void *status, uint16_t numStatus)
{
    DECODE_FUNC_CALL();

     m_statusReport->GetReport(numStatus, status);

    return MOS_STATUS_SUCCESS;
}

uint32_t AvcPipelineXe3_Lpm_Base::GetCompletedReport()
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

MOS_STATUS AvcPipelineXe3_Lpm_Base::Destroy()
{
    DECODE_FUNC_CALL();

    Uninitialize();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcPipelineXe3_Lpm_Base::Initialize(void *settings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(AvcPipeline::Initialize(settings));
    DECODE_CHK_STATUS(InitMmcState());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcPipelineXe3_Lpm_Base::CreateSubPackets(DecodeSubPacketManager &subPacketManager, CodechalSetting &codecSettings)
{
    DECODE_CHK_STATUS(AvcPipeline::CreateSubPackets(subPacketManager, codecSettings));

#ifdef _DECODE_PROCESSING_SUPPORTED
    AvcDownSamplingPkt *downSamplingPkt = MOS_New(AvcDownSamplingPkt, this, m_hwInterface);
    DECODE_CHK_NULL(downSamplingPkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, downSamplingSubPacketId), *downSamplingPkt));
#endif

    AvcDecodePicPktXe3_Lpm_Base *pictureDecodePkt = MOS_New(AvcDecodePicPktXe3_Lpm_Base, this, m_hwInterface);
    DECODE_CHK_NULL(pictureDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
                        DecodePacketId(this, avcPictureSubPacketId), *pictureDecodePkt));

    AvcDecodeSlcPktXe3_Lpm_Base *sliceDecodePkt = MOS_New(AvcDecodeSlcPktXe3_Lpm_Base, this, m_hwInterface);
    DECODE_CHK_NULL(sliceDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
                        DecodePacketId(this, avcSliceSubPacketId), *sliceDecodePkt));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcPipelineXe3_Lpm_Base::Uninitialize()
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

    return DecodePipeline::Uninitialize();
}

MOS_STATUS AvcPipelineXe3_Lpm_Base::InitContext()
{
    DECODE_FUNC_CALL();

    DecodeScalabilityPars scalPars;
    MOS_ZeroMemory(&scalPars, sizeof(scalPars));
    scalPars.disableScalability = true;
    scalPars.disableRealTile = true;
    scalPars.enableVE = MOS_VE_SUPPORTED(m_osInterface);
    scalPars.numVdbox = m_numVdbox;
#ifdef _DECODE_PROCESSING_SUPPORTED
    DecodeDownSamplingFeature* downSamplingFeature = dynamic_cast<DecodeDownSamplingFeature*>(
        m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
     if (downSamplingFeature != nullptr && downSamplingFeature->IsEnabled())
    {
        scalPars.usingSfc = true;
    }
#endif

    if (m_allowVirtualNodeReassign)
    {
        // reassign decoder virtual node at the first frame for each stream
        DECODE_CHK_STATUS(m_mediaContext->ReassignContextForDecoder(m_basicFeature->m_frameNum, &scalPars, &m_scalability));
        m_mediaContext->SetLatestDecoderVirtualNode();
    }
    else
    {
        DECODE_CHK_STATUS(m_mediaContext->SwitchContext(VdboxDecodeFunc, &scalPars, &m_scalability));
    }
    DECODE_CHK_NULL(m_scalability);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcPipelineXe3_Lpm_Base::Prepare(void *params)
{
    DecodePipelineParams *pipelineParams = (DecodePipelineParams *)params;
    m_pipeMode = pipelineParams->m_pipeMode;

    PERF_UTILITY_AUTO((__FUNCTION__ + std::to_string((int)m_pipeMode)).c_str(), PERF_DECODE, PERF_LEVEL_HAL);

    if (IsFirstProcessPipe(*pipelineParams))
    {
        DECODE_CHK_STATUS(AvcPipeline::Prepare(params));
    }

    DECODE_CHK_STATUS(m_preSubPipeline->Prepare(*pipelineParams));
    DECODE_CHK_STATUS(m_postSubPipeline->Prepare(*pipelineParams));

    if (m_pipeMode == decodePipeModeProcess)
    {
        if (IsCompleteBitstream())
        {
            if (m_pCodechalOcaDumper)
            {
                m_pCodechalOcaDumper->SetAvcDecodeParam(
                    m_basicFeature->m_avcPicParams,
                    m_basicFeature->m_avcSliceParams,
                    m_basicFeature->m_numSlices);
            }

            CODECHAL_DEBUG_TOOL(DECODE_CHK_STATUS(DumpParams(*m_basicFeature)));
            DecodeStatusParameters inputParameters = {};
            MOS_ZeroMemory(&inputParameters, sizeof(DecodeStatusParameters));
            inputParameters.statusReportFeedbackNumber = m_basicFeature->m_avcPicParams->StatusReportFeedbackNumber;
            inputParameters.codecFunction              = m_basicFeature->m_codecFunction;
            inputParameters.picWidthInMb               = m_basicFeature->m_picWidthInMb;
            inputParameters.pictureCodingType          = m_basicFeature->m_pictureCodingType;
            inputParameters.currOriginalPic            = m_basicFeature->m_curRenderPic;
            inputParameters.currDecodedPicRes          = m_basicFeature->m_destSurface.OsResource;
            inputParameters.numUsedVdbox               = m_numVdbox;

#ifdef _DECODE_PROCESSING_SUPPORTED
            CODECHAL_DEBUG_TOOL(
                DecodeDownSamplingFeature* downSamplingFeature = dynamic_cast<DecodeDownSamplingFeature*>(
                    m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
                if (downSamplingFeature != nullptr)
                {
                    auto frameIdx = m_basicFeature->m_curRenderPic.FrameIdx;
                    inputParameters.sfcOutputSurface = &downSamplingFeature->m_outputSurfaceList[frameIdx];
                    DumpDownSamplingParams(*downSamplingFeature);
                });
#endif
            m_statusReport->Init(&inputParameters);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcPipelineXe3_Lpm_Base::Execute()
{
    PERF_UTILITY_AUTO((__FUNCTION__ + std::to_string((int)m_pipeMode)).c_str(), PERF_DECODE, PERF_LEVEL_HAL);

    if (m_pipeMode == decodePipeModeProcess)
    {
        DECODE_CHK_STATUS(m_preSubPipeline->Execute());

        if (IsCompleteBitstream())
        {
            DECODE_CHK_STATUS(InitContext());
            DECODE_CHK_STATUS(ActivateDecodePackets());
            DECODE_CHK_STATUS(ExecuteActivePackets());
            DECODE_CHK_STATUS(HandleRefOnlySurfaces());

#if (_DEBUG || _RELEASE_INTERNAL)
            DECODE_CHK_STATUS(StatusCheck());
#endif
 
            // Only update user features for the first frame.
            if (m_basicFeature->m_frameNum == 0)
            {
                DECODE_CHK_STATUS(UserFeatureReport());
            }

            if (m_basicFeature->m_avcPicParams)
            {
                if (m_basicFeature->m_secondField || CodecHal_PictureIsFrame(m_basicFeature->m_avcPicParams->CurrPic))
                {
                    DecodeFrameIndex++;
                    m_basicFeature->m_frameNum = DecodeFrameIndex;
                }
            }
            DECODE_CHK_STATUS(m_statusReport->Reset());
        }
        DECODE_CHK_STATUS(m_postSubPipeline->Execute());
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcPipelineXe3_Lpm_Base::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    DECODE_CHK_NULL(m_hwInterface);
    m_mmcState = MOS_New(DecodeMemCompXe3_Lpm_Base, m_hwInterface);
    DECODE_CHK_NULL(m_mmcState);

    DECODE_CHK_STATUS(m_basicFeature->SetMmcState(m_mmcState->IsMmcEnabled()));
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcPipelineXe3_Lpm_Base::UserFeatureReport()
{
    DECODE_FUNC_CALL();

    return AvcPipeline::UserFeatureReport();
}

MOS_STATUS AvcPipelineXe3_Lpm_Base::CreateFeatureManager()
{
    DECODE_FUNC_CALL()
    m_featureManager = MOS_New(DecodeAvcFeatureManagerXe3_Lpm_Base, m_allocator, m_hwInterface, m_osInterface);
    DECODE_CHK_NULL(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS AvcPipelineXe3_Lpm_Base::DumpParams(AvcBasicFeature &basicFeature)
{
    m_debugInterface->m_frameType = m_basicFeature->m_avcPicParams->pic_fields.IntraPicFlag ? I_TYPE: MIXED_TYPE;
    m_debugInterface->m_currPic   = m_basicFeature->m_avcPicParams->CurrPic;
    m_debugInterface->m_secondField = m_basicFeature->m_secondField;
    m_debugInterface->m_bufferDumpFrameNum = m_basicFeature->m_frameNum;

    DECODE_CHK_STATUS(DumpPicParams(basicFeature.m_avcPicParams));
    DECODE_CHK_STATUS(DumpIQParams(basicFeature.m_avcIqMatrixParams));
    DECODE_CHK_STATUS(DumpSliceParams(basicFeature.m_avcSliceParams, basicFeature.m_numSlices, basicFeature.m_shortFormatInUse));
    DECODE_CHK_STATUS(DumpBitstream(&basicFeature.m_resDataBuffer.OsResource, basicFeature.m_dataSize, 0));

    return MOS_STATUS_SUCCESS;
}

#endif
}
