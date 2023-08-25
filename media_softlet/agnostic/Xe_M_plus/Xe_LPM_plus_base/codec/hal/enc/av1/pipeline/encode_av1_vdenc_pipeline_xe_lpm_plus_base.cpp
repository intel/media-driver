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
//! \file     encode_av1_vdenc_pipeline_xe_lpm_plus_base.cpp
//! \brief    Defines the interface for av1 vdenc encode pipeline Xe_LPM_plus+
//!
#include "encode_av1_vdenc_pipeline_xe_lpm_plus_base.h"
#include "encode_av1_vdenc_packet_xe_lpm_plus_base.h"
#include "encode_av1_vdenc_feature_manager_xe_lpm_plus_base.h"
#include "codechal_debug.h"
#include "encode_mem_compression_xe_lpm_plus_base.h"
#include "encode_av1_superres.h"

namespace encode
{
MOS_STATUS Av1VdencPipelineXe_Lpm_Plus_Base::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(settings);

    ENCODE_CHK_STATUS_RETURN(Initialize(settings));

    MediaTask *task = CreateTask(MediaTask::TaskType::cmdTask);
    ENCODE_CHK_NULL_RETURN(task);

    auto*av1Vdencpkt = MOS_New(Av1VdencPktXe_Lpm_Plus_Base, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1VdencPacket, av1Vdencpkt));
    ENCODE_CHK_STATUS_RETURN(av1Vdencpkt->Init());

    Av1BackAnnotationPkt *av1BackAnnotationpkt = MOS_New(Av1BackAnnotationPkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1BackAnnotation, av1BackAnnotationpkt));
    ENCODE_CHK_STATUS_RETURN(av1BackAnnotationpkt->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_Lpm_Plus_Base::Prepare(void *params)
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

    uint16_t numTileRows    = 0;
    uint16_t numTileColumns = 0;
    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

    ENCODE_CHK_STATUS_RETURN(SwitchContext(feature->m_outputChromaFormat, numTileRows, numTileColumns));

    EncoderStatusParameters inputParameters = {};

    ENCODE_CHK_STATUS_RETURN(FillStatusReportParameters(&inputParameters, encodeParams));

    ENCODE_CHK_STATUS_RETURN(m_statusReport->Init(&inputParameters));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_Lpm_Plus_Base::Execute()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_featureManager);
    auto superResFeature = dynamic_cast<Av1SuperRes *>(m_featureManager->GetFeature(Av1FeatureIDs::av1SuperRes));
    ENCODE_CHK_NULL_RETURN(superResFeature);
    if (superResFeature->IsEnabled())
    {
        if (superResFeature->IsSuperResUsed())
        {
            MEDIA_SFC_INTERFACE_MODE sfcMode = {};
            sfcMode.vdboxSfcEnabled          = false;
            sfcMode.veboxSfcEnabled          = true;
            if (!m_sfcItf->IsRenderInitialized())
            {
                m_sfcItf->Initialize(sfcMode);
            }
            ENCODE_CHK_STATUS_RETURN(m_sfcItf->Render(superResFeature->GetDownScalingParams()));
            ContextSwitchBack();
        }
    }

    ENCODE_CHK_STATUS_RETURN(ActivateVdencVideoPackets());
    ENCODE_CHK_STATUS_RETURN(ExecuteActivePackets());

    ENCODE_CHK_STATUS_RETURN(ResetParams());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_Lpm_Plus_Base::ActivateVdencVideoPackets()
{
    ENCODE_FUNC_CALL();

    bool immediateSubmit = !m_singleTaskPhaseSupported;

    ENCODE_CHK_NULL_RETURN(m_featureManager);

    auto basicFeature = dynamic_cast<Av1BasicFeature*>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);

    auto brcFeature = dynamic_cast<Av1Brc*>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    bool preEncEnabled = false;
    uint32_t encodeMode = 0;
    RUN_FEATURE_INTERFACE_RETURN(Av1VdencPreEnc, Av1FeatureIDs::preEncFeature, IsEnabled, preEncEnabled);
    RUN_FEATURE_INTERFACE_RETURN(Av1VdencPreEnc, FeatureIDs::preEncFeature, GetEncodeMode, encodeMode);
    if (preEncEnabled)
    {
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(encodePreEncPacket, immediateSubmit, 0, 0));
#if USE_CODECHAL_DEBUG_TOOL
        if (encodeMode == MediaEncodeMode::MANUAL_RES_PRE_ENC || encodeMode == MediaEncodeMode::AUTO_RES_PRE_ENC)
        {
            m_activePacketList.back().immediateSubmit = true;
            return MOS_STATUS_SUCCESS;
        }
#endif
    }

    if (brcFeature->IsBRCInitRequired())
    {
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(Av1HucBrcInit, immediateSubmit, 0, 0));
    }

    for (uint8_t curPass = 0; curPass < GetPassNum(); curPass++)
    {
        if (brcFeature->IsBRCEnabled())
        {
            ENCODE_CHK_STATUS_RETURN(ActivatePacket(Av1HucBrcUpdate, immediateSubmit, curPass, 0));
        }

        for (uint8_t curPipe = 0; curPipe < GetPipeNum(); curPipe++)
        {
            // force immediate submit to false irrespective of single/multi task phase at pipe 0 when dual enc enabled
            ENCODE_CHK_STATUS_RETURN(ActivatePacket(Av1VdencPacket, m_dualEncEnable && curPipe == 0 ? false : immediateSubmit, curPass, curPipe, GetPipeNum()));
        }

        if ((basicFeature->m_enableTileStitchByHW || !basicFeature->m_enableSWStitching || brcFeature->IsBRCEnabled()) && m_dualEncEnable)
        {
            ENCODE_CHK_STATUS_RETURN(ActivatePacket(Av1PakIntegrate, immediateSubmit, curPass, 0));
        }

        if (!basicFeature->m_enableSWBackAnnotation)
        {
            ENCODE_CHK_STATUS_RETURN(ActivatePacket(Av1BackAnnotation, immediateSubmit, curPass, 0));
        }
    }

    SetFrameTrackingForMultiTaskPhase();

    // Last element in m_activePacketList must be immediately submitted
    m_activePacketList.back().immediateSubmit = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_Lpm_Plus_Base::GetStatusReport(void *status, uint16_t numStatus)
{
    ENCODE_FUNC_CALL();
    m_statusReport->GetReport(numStatus, status);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_Lpm_Plus_Base::Destroy()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(Uninitialize());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_Lpm_Plus_Base::CreateFeatureManager()
{
    ENCODE_FUNC_CALL();
    m_featureManager = MOS_New(EncodeAv1VdencFeatureManagerXe_Lpm_Plus_Base, m_allocator, m_hwInterface, m_trackedBuf, m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_Lpm_Plus_Base::ResetParams()
{
    ENCODE_FUNC_CALL();

    m_currRecycledBufIdx =
        (m_currRecycledBufIdx + 1) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;

    if (m_currRecycledBufIdx == 0)
    {
        MOS_ZeroMemory(m_recycledBufStatusNum, sizeof(m_recycledBufStatusNum));
    }

    auto feature = dynamic_cast<EncodeBasicFeature *>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
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

MOS_STATUS Av1VdencPipelineXe_Lpm_Plus_Base::UserFeatureReport()
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

MOS_STATUS Av1VdencPipelineXe_Lpm_Plus_Base::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    m_mmcState = MOS_New(EncodeMemCompXe_Lpm_Plus_Base, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
