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
//! \file     decode_mpeg2_pipeline_xe_lpm_plus_base.cpp
//! \brief    Defines the interface for mpeg2 decode pipeline
//!
#include "decode_mpeg2_pipeline_xe_lpm_plus_base.h"
#include "decode_utils.h"
#include "decode_mpeg2_mem_compression_xe_lpm_plus_base.h"
#include "decode_mpeg2_packet_xe_lpm_plus_base.h"
#include "decode_mpeg2_slice_packet_xe_lpm_plus_base.h"
#include "decode_mpeg2_mb_packet_xe_lpm_plus_base.h"
#include "decode_mpeg2_picture_packet_xe_lpm_plus_base.h"
#include "decode_common_feature_defs.h"

namespace decode
{
Mpeg2PipelineXe_Lpm_Plus_Base::Mpeg2PipelineXe_Lpm_Plus_Base(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : Mpeg2Pipeline(hwInterface, debugInterface)
{
}

MOS_STATUS Mpeg2PipelineXe_Lpm_Plus_Base::Init(void *settings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(settings);
    DECODE_CHK_STATUS(Initialize(settings));

    m_mpeg2DecodePkt = MOS_New(Mpeg2DecodePktXe_Lpm_Plus_Base, this, m_task, m_hwInterface);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, mpeg2DecodePacketId), m_mpeg2DecodePkt));
    DECODE_CHK_STATUS(m_mpeg2DecodePkt->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2PipelineXe_Lpm_Plus_Base::GetStatusReport(void *status, uint16_t numStatus)
{
    DECODE_FUNC_CALL();

    m_statusReport->GetReport(numStatus, status);

    return MOS_STATUS_SUCCESS;
}

uint32_t Mpeg2PipelineXe_Lpm_Plus_Base::GetCompletedReport()
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

MOS_STATUS Mpeg2PipelineXe_Lpm_Plus_Base::Destroy()
{
    DECODE_FUNC_CALL();

    Uninitialize();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2PipelineXe_Lpm_Plus_Base::Initialize(void *settings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(Mpeg2Pipeline::Initialize(settings));
#ifdef _MMC_SUPPORTED
    DECODE_CHK_STATUS(InitMmcState());
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2PipelineXe_Lpm_Plus_Base::CreateSubPackets(DecodeSubPacketManager &subPacketManager, CodechalSetting &codecSettings)
{
    DECODE_CHK_STATUS(Mpeg2Pipeline::CreateSubPackets(subPacketManager, codecSettings));

    Mpeg2DecodePicPktXe_Lpm_Plus_Base *pictureDecodePkt = MOS_New(Mpeg2DecodePicPktXe_Lpm_Plus_Base, this, m_hwInterface);
    DECODE_CHK_NULL(pictureDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, mpeg2PictureSubPacketId), *pictureDecodePkt));

    if (codecSettings.mode == CODECHAL_DECODE_MODE_MPEG2VLD)
    {
        Mpeg2DecodeSlcPktXe_Lpm_Plus_Base *sliceDecodePkt = MOS_New(Mpeg2DecodeSlcPktXe_Lpm_Plus_Base, this, m_hwInterface);
        DECODE_CHK_NULL(sliceDecodePkt);
        DECODE_CHK_STATUS(subPacketManager.Register(
            DecodePacketId(this, mpeg2SliceSubPacketId), *sliceDecodePkt));
    }
    else
    {
        Mpeg2DecodeMbPktXe_Lpm_Plus_Base *mbDecodePkt = MOS_New(Mpeg2DecodeMbPktXe_Lpm_Plus_Base, this, m_hwInterface);
        DECODE_CHK_NULL(mbDecodePkt);
        DECODE_CHK_STATUS(subPacketManager.Register(
            DecodePacketId(this, mpeg2MbSubPacketId), *mbDecodePkt));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2PipelineXe_Lpm_Plus_Base::Uninitialize()
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

MOS_STATUS Mpeg2PipelineXe_Lpm_Plus_Base::InitContext()
{
    DECODE_FUNC_CALL();

    DecodeScalabilityPars scalPars;
    MOS_ZeroMemory(&scalPars, sizeof(scalPars));
    scalPars.disableScalability = true;
    scalPars.disableRealTile    = true;
    scalPars.enableVE           = MOS_VE_SUPPORTED(m_osInterface);
    scalPars.numVdbox           = m_numVdbox;
    m_mediaContext->SwitchContext(VdboxDecodeFunc, &scalPars, &m_scalability);
    DECODE_CHK_NULL(m_scalability);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2PipelineXe_Lpm_Plus_Base::Prepare(void *params)
{
    DecodePipelineParams *pipelineParams = (DecodePipelineParams *)params;
    m_pipeMode                           = pipelineParams->m_pipeMode;

    PERF_UTILITY_AUTO((__FUNCTION__ + std::to_string((int)m_pipeMode)).c_str(), PERF_DECODE, PERF_LEVEL_HAL);

    if (m_pipeMode == decodePipeModeProcess)
    {
        DECODE_CHK_STATUS(Mpeg2Pipeline::Prepare(params));
    }

    DECODE_CHK_STATUS(m_preSubPipeline->Prepare(*pipelineParams));
    DECODE_CHK_STATUS(m_postSubPipeline->Prepare(*pipelineParams));

    if (IsFirstProcessPipe(*pipelineParams))
    {
        CODECHAL_DEBUG_TOOL(DECODE_CHK_STATUS(DumpParams(*m_basicFeature)));
        DecodeStatusParameters inputParameters = {};
        MOS_ZeroMemory(&inputParameters, sizeof(DecodeStatusParameters));
        inputParameters.statusReportFeedbackNumber = m_basicFeature->m_mpeg2PicParams->m_statusReportFeedbackNumber;
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

        m_statusReport->Init(&inputParameters);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2PipelineXe_Lpm_Plus_Base::Execute()
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO((__FUNCTION__ + std::to_string((int)m_pipeMode)).c_str(), PERF_DECODE, PERF_LEVEL_HAL);

    if (m_pipeMode == decodePipeModeProcess)
    {
        DECODE_CHK_STATUS(m_preSubPipeline->Execute());

        if (!m_basicFeature->m_incompletePicture)
        {
            DECODE_CHK_STATUS(InitContext());
            DECODE_CHK_STATUS(CopyDummyBitstream());
            DECODE_CHK_STATUS(ActivateDecodePackets());
            DECODE_CHK_STATUS(ExecuteActivePackets());

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

            if (m_basicFeature->m_secondField || CodecHal_PictureIsFrame(m_basicFeature->m_curRenderPic))
            {
                DecodeFrameIndex++;
                m_basicFeature->m_frameNum = DecodeFrameIndex;
            }

            DECODE_CHK_STATUS(m_statusReport->Reset());
        }
        DECODE_CHK_STATUS(m_postSubPipeline->Execute());
    }
    else if (m_pipeMode == decodePipeModeEnd)
    {
        if (m_basicFeature->m_incompletePicture)
        {
            // Insert phantom slices in EndFrame call if it is still an incomplete
            // picture and submit mpeg2 decode command buffer at this time.
            DECODE_CHK_STATUS(CopyDummyBitstream());
            DECODE_CHK_STATUS(ActivateDecodePackets());
            DECODE_CHK_STATUS(ExecuteActivePackets());

            // Only update user features for the first frame.
            if (m_basicFeature->m_frameNum == 0)
            {
                DECODE_CHK_STATUS(UserFeatureReport());
            }

            if (m_basicFeature->m_secondField || CodecHal_PictureIsFrame(m_basicFeature->m_curRenderPic))
            {
                DecodeFrameIndex++;
                m_basicFeature->m_frameNum = DecodeFrameIndex;
            }

            DECODE_CHK_STATUS(m_statusReport->Reset());
        }
    }

    return MOS_STATUS_SUCCESS;
}

#ifdef _MMC_SUPPORTED
MOS_STATUS Mpeg2PipelineXe_Lpm_Plus_Base::InitMmcState()
{
    DECODE_FUNC_CALL();

    m_mmcState = MOS_New(Mpeg2DecodeMemCompXe_Lpm_Plus_Base, m_hwInterface);
    DECODE_CHK_NULL(m_mmcState);
    DECODE_CHK_STATUS(m_basicFeature->SetMmcState(m_mmcState->IsMmcEnabled()));

    return MOS_STATUS_SUCCESS;
}
#endif

MOS_STATUS Mpeg2PipelineXe_Lpm_Plus_Base::UserFeatureReport()
{
    DECODE_FUNC_CALL();
    return Mpeg2Pipeline::UserFeatureReport();
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Mpeg2PipelineXe_Lpm_Plus_Base::DumpParams(Mpeg2BasicFeature &basicFeature)
{
    m_debugInterface->m_frameType          = basicFeature.m_pictureCodingType;
    m_debugInterface->m_currPic            = basicFeature.m_curRenderPic;
    m_debugInterface->m_secondField        = basicFeature.m_secondField;
    m_debugInterface->m_bufferDumpFrameNum = basicFeature.m_frameNum;

    DECODE_CHK_STATUS(DumpPicParams(basicFeature.m_mpeg2PicParams));
    DECODE_CHK_STATUS(DumpSliceParams(basicFeature.m_mpeg2SliceParams, basicFeature.m_numSlices));
    DECODE_CHK_STATUS(DumpMbParams(basicFeature.m_mpeg2MbParams, basicFeature.m_numMacroblocks));
    DECODE_CHK_STATUS(DumpIQParams(basicFeature.m_mpeg2IqMatrixBuffer));
    DECODE_CHK_STATUS(DumpBitstream(&basicFeature.m_resDataBuffer.OsResource, basicFeature.m_dataSize, 0));

    return MOS_STATUS_SUCCESS;
}

#endif
}  // namespace decode
