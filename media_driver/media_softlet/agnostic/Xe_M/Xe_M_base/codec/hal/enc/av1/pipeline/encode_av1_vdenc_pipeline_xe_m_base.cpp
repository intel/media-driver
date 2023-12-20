/*
* Copyright (c) 2019-2023, Intel Corporation
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
    CodechalHwInterfaceNext     *hwInterface,
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
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1HucBrcInit, brcInitpkt));
    ENCODE_CHK_STATUS_RETURN(brcInitpkt->Init());

    Av1BrcUpdatePkt* brcUpdatepkt = MOS_New(Av1BrcUpdatePkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1HucBrcUpdate, brcUpdatepkt));
    ENCODE_CHK_STATUS_RETURN(brcUpdatepkt->Init());

    Av1VdencPktXe_M_Base *av1Vdencpkt = MOS_New(Av1VdencPktXe_M_Base, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1VdencPacket, av1Vdencpkt));
    ENCODE_CHK_STATUS_RETURN(av1Vdencpkt->Init());

    Av1BackAnnotationPkt *av1BackAnnotationpkt = MOS_New(Av1BackAnnotationPkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1BackAnnotation, av1BackAnnotationpkt));
    ENCODE_CHK_STATUS_RETURN(av1BackAnnotationpkt->Init());

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS Av1VdencPipelineXe_M_Base::Prepare(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);
    EncoderParams *encodeParams = (EncoderParams *)params;

    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS av1SeqParams = static_cast<PCODEC_AV1_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(av1SeqParams);

    auto feature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(feature);

    feature->m_dualEncEnable = m_dualEncEnable;

    ENCODE_CHK_STATUS_RETURN(Av1VdencPipeline::Prepare(params));

    uint16_t numTileRows = 0;
    uint16_t numTileColumns = 0;
    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileRowColumns,
        numTileRows, numTileColumns);

    ENCODE_CHK_STATUS_RETURN(SwitchContext(feature->m_outputChromaFormat, numTileRows, numTileColumns));

    EncoderStatusParameters inputParameters = {};

    ENCODE_CHK_STATUS_RETURN(FillStatusReportParameters(&inputParameters, encodeParams));

    ENCODE_CHK_STATUS_RETURN(m_statusReport->Init(&inputParameters));

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
    ENCODE_CHK_STATUS_RETURN(m_statusReport->GetReport(numStatus, status));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_M_Base::Destroy()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(Uninitialize());

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
    ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

}
