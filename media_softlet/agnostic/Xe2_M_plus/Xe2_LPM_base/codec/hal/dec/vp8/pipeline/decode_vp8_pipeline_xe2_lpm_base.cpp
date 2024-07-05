/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     decode_vp8_pipeline_xe2_lpm_base.cpp
//! \brief    Defines the interface for vp8 decode pipeline
//!
#include "decode_vp8_pipeline_xe2_lpm_base.h"
#include "decode_vp8_picture_packet_xe2_lpm_base.h"
#include "decode_vp8_slice_packet_xe2_lpm_base.h"
#include "decode_vp8_packet_xe2_lpm_base.h"
#include "decode_vp8_mem_compression_xe2_lpm_base.h"
#include "decode_utils.h"
#include "codechal_debug.h"
#include "decode_common_feature_defs.h"

namespace decode
{
Vp8PipelineXe2_Lpm_Base::Vp8PipelineXe2_Lpm_Base(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : Vp8Pipeline(hwInterface, debugInterface)
{
}

MOS_STATUS Vp8PipelineXe2_Lpm_Base::Init(void *settings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(settings);
    DECODE_CHK_STATUS(Initialize(settings));

    vp8DecodePkt = MOS_New(Vp8DecodePktXe2_Lpm_Base, this, m_task, m_hwInterface);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, vp8DecodePacketId), vp8DecodePkt));
    DECODE_CHK_STATUS(vp8DecodePkt->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8PipelineXe2_Lpm_Base::Prepare(void *params)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(params);
    DecodePipelineParams *pipelineParams = (DecodePipelineParams *)params;
    m_pipeMode                           = pipelineParams->m_pipeMode;

    PERF_UTILITY_AUTO((__FUNCTION__ + std::to_string((int)m_pipeMode)).c_str(), PERF_DECODE, PERF_LEVEL_HAL);

    if (IsFirstProcessPipe(*pipelineParams))
    {
        DECODE_CHK_STATUS(Vp8Pipeline::Prepare(params));
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
            inputParameters.statusReportFeedbackNumber = m_basicFeature->m_vp8PicParams->uiStatusReportFeedbackNumber;
            inputParameters.codecFunction              = m_basicFeature->m_codecFunction;
            inputParameters.picWidthInMb               = m_basicFeature->m_picWidthInMb;
            inputParameters.pictureCodingType          = m_basicFeature->m_pictureCodingType;
            inputParameters.currOriginalPic            = m_basicFeature->m_curRenderPic;
            inputParameters.currDecodedPicRes          = m_basicFeature->m_destSurface.OsResource;
            inputParameters.numUsedVdbox               = m_numVdbox;

            m_statusReport->Init(&inputParameters);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8PipelineXe2_Lpm_Base::Execute()
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO((__FUNCTION__ + std::to_string((int)m_pipeMode)).c_str(), PERF_DECODE, PERF_LEVEL_HAL);

    if (m_pipeMode == decodePipeModeProcess)
    {
        DECODE_CHK_STATUS(m_preSubPipeline->Execute());

        if (IsCompleteBitstream())
        {
            DECODE_CHK_STATUS(Vp8Pipeline::Execute());

#if (_DEBUG || _RELEASE_INTERNAL)
            DECODE_CHK_STATUS(StatusCheck());
#ifdef _MMC_SUPPORTED
            if (m_mmcState != nullptr)
            {
                m_mmcState->ReportSurfaceMmcMode(&(m_basicFeature->m_destSurface));
            }
#endif
#endif
            // Only update user features for the first frame.
            if (m_basicFeature->m_frameNum == 0)
            {
                DECODE_CHK_STATUS(UserFeatureReport());
            }
            
            DecodeFrameIndex++;
            m_basicFeature->m_frameNum = DecodeFrameIndex;

            DECODE_CHK_STATUS(m_statusReport->Reset());

        }
        DECODE_CHK_STATUS(m_postSubPipeline->Execute());
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8PipelineXe2_Lpm_Base::GetStatusReport(void *status, uint16_t numStatus)
{
    DECODE_FUNC_CALL();

    m_statusReport->GetReport(numStatus, status);

    return MOS_STATUS_SUCCESS;
}

uint32_t Vp8PipelineXe2_Lpm_Base::GetCompletedReport()
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

MOS_STATUS Vp8PipelineXe2_Lpm_Base::Destroy()
{
    DECODE_FUNC_CALL();

    Uninitialize();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8PipelineXe2_Lpm_Base::Initialize(void *settings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(Vp8Pipeline::Initialize(settings));
#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(InitMmcState());
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8PipelineXe2_Lpm_Base::Uninitialize()
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

    return Vp8Pipeline::Uninitialize();
}

MOS_STATUS Vp8PipelineXe2_Lpm_Base::UserFeatureReport()
{
    DECODE_FUNC_CALL();

    return Vp8Pipeline::UserFeatureReport();
}

MOS_STATUS Vp8PipelineXe2_Lpm_Base::CreateSubPackets(DecodeSubPacketManager &subPacketManager, CodechalSetting &codecSettings)
{
    DECODE_CHK_STATUS(Vp8Pipeline::CreateSubPackets(subPacketManager, codecSettings));

    Vp8DecodePicPktXe2_Lpm_Base *pictureDecodePkt = MOS_New(Vp8DecodePicPktXe2_Lpm_Base, this, m_hwInterface);
    DECODE_CHK_NULL(pictureDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, vp8PictureSubPacketId), *pictureDecodePkt));

    //VP8 slice packet, only 1 slice
    Vp8DecodeSlcPktXe2_Lpm_Base *sliceDecodePkt = MOS_New(Vp8DecodeSlcPktXe2_Lpm_Base, this, m_hwInterface);
    DECODE_CHK_NULL(sliceDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, vp8SliceSubPacketId), *sliceDecodePkt));


    return MOS_STATUS_SUCCESS;
}

#ifdef _MMC_SUPPORTED
MOS_STATUS Vp8PipelineXe2_Lpm_Base::InitMmcState()
{
    DECODE_CHK_NULL(m_hwInterface);
    m_mmcState = MOS_New(Vp8DecodeMemCompXe2_Lpm_Base, m_hwInterface);
    DECODE_CHK_NULL(m_mmcState);
    DECODE_CHK_STATUS(m_basicFeature->SetMmcState(m_mmcState->IsMmcEnabled()));
    return MOS_STATUS_SUCCESS;
}
#endif

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Vp8PipelineXe2_Lpm_Base::DumpParams(Vp8BasicFeature &basicFeature)
{
    basicFeature.m_vp8PicParams->CurrPic.PicFlags = PICTURE_FRAME;
    m_debugInterface->m_currPic                   = basicFeature.m_curRenderPic;
    m_debugInterface->m_frameType                 = basicFeature.m_pictureCodingType;
    m_debugInterface->m_secondField               = basicFeature.m_secondField;
    m_debugInterface->m_bufferDumpFrameNum        = basicFeature.m_frameNum;

    DECODE_CHK_STATUS(DumpPicParams(basicFeature.m_vp8PicParams));
    DECODE_CHK_STATUS(DumpSliceParams(basicFeature.m_vp8SliceParams));
    DECODE_CHK_STATUS(DumpIQParams(basicFeature.m_vp8IqMatrixParams));
    DECODE_CHK_STATUS(DumpBitstream(&basicFeature.m_resDataBuffer.OsResource, basicFeature.m_dataSize, 0));

    if (basicFeature.m_bitstreamLockingInUse)
    {
        DECODE_CHK_STATUS(DumpCoefProbBuffer(
            &(basicFeature.m_resCoefProbBufferInternal->OsResource)));
    }
    else
    {
        DECODE_CHK_STATUS(DumpCoefProbBuffer(
            &(basicFeature.m_resCoefProbBufferExternal)));
    }

    return MOS_STATUS_SUCCESS;
}
#endif

}  // namespace decode
