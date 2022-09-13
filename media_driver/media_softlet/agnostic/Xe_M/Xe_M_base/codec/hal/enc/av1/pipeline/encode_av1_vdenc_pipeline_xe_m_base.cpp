/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     encode_av1_vdenc_pipeline_xe_m_base.cpp
//! \brief    Defines the interface for av1 vdenc encode pipeline xe m base
//!
#include "encode_av1_vdenc_pipeline_xe_m_base.h"
#include "encode_av1_vdenc_packet_xe_m_base.h"
#include "encode_av1_brc_init_packet.h"
#include "encode_av1_brc_update_packet.h"
#include "encode_mem_compression_g12.h"

namespace encode {

Av1VdencPipelineXe_M_Base::Av1VdencPipelineXe_M_Base(
    CodechalHwInterface     *hwInterface,
    CodechalDebugInterface  *debugInterface)
    : Av1VdencPipeline(hwInterface, debugInterface)
{

}

MOS_STATUS Av1VdencPipelineXe_M_Base::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(settings);

    ENCODE_CHK_STATUS_RETURN(Initialize(settings));

    MediaTask* task = CreateTask(MediaTask::TaskType::cmdTask);
    ENCODE_CHK_NULL_RETURN(task);

    Av1BrcInitPkt* brcInitpkt = MOS_New(Av1BrcInitPkt, this, task, m_hwInterface);
    RegisterPacket(Av1HucBrcInit, brcInitpkt);
    brcInitpkt->Init();

    Av1BrcUpdatePkt* brcUpdatepkt = MOS_New(Av1BrcUpdatePkt, this, task, m_hwInterface);
    RegisterPacket(Av1HucBrcUpdate, brcUpdatepkt);
    brcUpdatepkt->Init();

    Av1VdencPktXe_M_Base *av1Vdencpkt = MOS_New(Av1VdencPktXe_M_Base, this, task, m_hwInterface);
    RegisterPacket(Av1VdencPacket, av1Vdencpkt);
    av1Vdencpkt->Init();

    Av1BackAnnotationPkt *av1BackAnnotationpkt = MOS_New(Av1BackAnnotationPkt, this, task, m_hwInterface);
    RegisterPacket(Av1BackAnnotation, av1BackAnnotationpkt);
    av1BackAnnotationpkt->Init();

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS Av1VdencPipelineXe_M_Base::Prepare(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);
    EncoderParams *encodeParams = (EncoderParams *)params;

    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS av1SeqParams = static_cast<PCODEC_AV1_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(av1SeqParams);

    ENCODE_CHK_STATUS_RETURN(Av1VdencPipeline::Prepare(params));

    PCODEC_AV1_ENCODE_PICTURE_PARAMS picParams = static_cast<PCODEC_AV1_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(picParams);

    auto feature = dynamic_cast<Av1BasicFeature*>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(feature);

    uint16_t numTileRows = 0;
    uint16_t numTileColumns = 0;
    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileRowColumns,
        numTileRows, numTileColumns);

    ENCODE_CHK_STATUS_RETURN(SwitchContext(feature->m_outputChromaFormat, numTileRows, numTileColumns));

    EncoderStatusParameters inputParameters = {};
    MOS_ZeroMemory(&inputParameters, sizeof(EncoderStatusParameters));

    inputParameters.statusReportFeedbackNumber = picParams->StatusReportFeedbackNumber;
    inputParameters.codecFunction              = encodeParams->ExecCodecFunction;
    inputParameters.currRefList                = feature->m_ref.GetCurrRefList();
    inputParameters.picWidthInMb               = feature->m_picWidthInMb;
    inputParameters.frameFieldHeightInMb       = feature->m_frameFieldHeightInMb;
    inputParameters.currOriginalPic            = feature->m_currOriginalPic;
    inputParameters.pictureCodingType          = feature->m_pictureCodingType;
    inputParameters.numUsedVdbox               = m_numVdbox;
    inputParameters.hwWalker                   = false;
    inputParameters.maxNumSlicesAllowed        = 0;

    inputParameters.numberTilesInFrame         = numTileRows * numTileColumns;

    m_statusReport->Init(&inputParameters);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_M_Base::Execute()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(ActivateVdencVideoPackets());
    ENCODE_CHK_STATUS_RETURN(ExecuteActivePackets());

    ENCODE_CHK_STATUS_RETURN(ResetParams());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_M_Base::GetStatusReport(void *status, uint16_t numStatus)
{
    ENCODE_FUNC_CALL();
    m_statusReport->GetReport(numStatus, status);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_M_Base::Destroy()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(Uninitialize());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_M_Base::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Av1VdencPipeline::Initialize(settings));
    ENCODE_CHK_STATUS_RETURN(InitMmcState());

    CODECHAL_DEBUG_TOOL
    (
        if (m_debugInterface != nullptr)
        {
            MOS_Delete(m_debugInterface);
        }
        m_debugInterface = MOS_New(CodechalDebugInterface);
        ENCODE_CHK_NULL_RETURN(m_debugInterface);
        ENCODE_CHK_STATUS_RETURN(
            m_debugInterface->Initialize(m_hwInterface, m_codecFunction, m_mediaCopy));

        if (m_statusReportDebugInterface != nullptr)
        {
            MOS_Delete(m_statusReportDebugInterface);
        }
        m_statusReportDebugInterface = MOS_New(CodechalDebugInterface);
        ENCODE_CHK_NULL_RETURN(m_statusReportDebugInterface);
        ENCODE_CHK_STATUS_RETURN(
            m_statusReportDebugInterface->Initialize(m_hwInterface, m_codecFunction, m_mediaCopy));
    );


    ENCODE_CHK_STATUS_RETURN(GetSystemVdboxNumber());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_M_Base::Uninitialize()
{
    ENCODE_FUNC_CALL();

    if (m_mmcState != nullptr)
    {
        MOS_Delete(m_mmcState);
    }

    ENCODE_CHK_STATUS_RETURN(Av1VdencPipeline::Uninitialize());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_M_Base::ResetParams()
{
    ENCODE_FUNC_CALL();

    m_currRecycledBufIdx =
        (m_currRecycledBufIdx + 1) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;

    if (m_currRecycledBufIdx == 0)
    {
        MOS_ZeroMemory(m_recycledBufStatusNum, sizeof(m_recycledBufStatusNum));
    }

    auto feature = dynamic_cast<EncodeBasicFeature*>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(feature);

    // Only update user features for first frame.
    if (feature->m_frameNum == 0)
    {
        ENCODE_CHK_STATUS_RETURN(UserFeatureReport());
    }

    feature->m_frameNum++;

    ENCODE_CHK_STATUS_RETURN(m_statusReport->Reset());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_M_Base::UserFeatureReport()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(Av1VdencPipeline::UserFeatureReport());

#if (_DEBUG || _RELEASE_INTERNAL)
    ReportUserSettingForDebug(
        m_userSettingPtr,
        "Enable Encode VE CtxBasedScheduling",
        MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface),
        MediaUserSetting::Group::Sequence);
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_M_Base::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    m_mmcState = MOS_New(EncodeMemCompG12, m_hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

}
