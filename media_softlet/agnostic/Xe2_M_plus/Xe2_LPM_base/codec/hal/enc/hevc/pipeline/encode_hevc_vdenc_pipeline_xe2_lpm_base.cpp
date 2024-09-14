/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     encode_hevc_vdenc_pipeline_xe2_lpm_base.cpp
//! \brief    Defines the interface for hevc vdenc encode pipeline Xe2_LPM+
//!

#include "encode_hevc_vdenc_pipeline_xe2_lpm_base.h"
#include "encode_utils.h"
#include "encode_hevc_tile_replay_packet.h"
#include "encode_hevc_vdenc_packet_xe2_lpm_base.h"
#include "encode_huc_la_init_packet.h"
#include "encode_huc_la_update_packet.h"
#include "encode_huc_brc_init_packet.h"
#include "encode_huc_brc_update_packet.h"
#include "encode_pak_integrate_packet.h"
#include "encode_status_report_defs.h"
#include "encode_scalability_defs.h"
#include "codechal_debug.h"
#include "encode_mem_compression_xe_lpm_plus_base.h"
#include "encode_hevc_vdenc_feature_manager_xe2_lpm_base.h"
#include "encode_preenc_packet.h"
#include "encode_vdenc_lpla_analysis.h"

namespace encode {

HevcVdencPipelineXe2_Lpm_Base::HevcVdencPipelineXe2_Lpm_Base(
    CodechalHwInterfaceNext     *hwInterface,
    CodechalDebugInterface  *debugInterface)
    : HevcVdencPipeline(hwInterface, debugInterface)
{

}

MOS_STATUS HevcVdencPipelineXe2_Lpm_Base::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(settings);

    ENCODE_CHK_STATUS_RETURN(Initialize(settings));

    MediaTask* task = CreateTask(MediaTask::TaskType::cmdTask);
    ENCODE_CHK_NULL_RETURN(task);

    RUN_FEATURE_INTERFACE_RETURN(HevcVdencPreEnc, FeatureIDs::preEncFeature, IsEnabled, m_preEncEnabled);
    if (m_preEncEnabled)
    {
        EncodePreEncPacket *hevcPreEncpkt = MOS_New(EncodePreEncPacket, this, task, m_hwInterface);
        ENCODE_CHK_STATUS_RETURN(RegisterPacket(encodePreEncPacket, hevcPreEncpkt));
        ENCODE_CHK_STATUS_RETURN(hevcPreEncpkt->Init());

        RUN_FEATURE_INTERFACE_RETURN(HevcVdencPreEnc, HevcFeatureIDs::preEncFeature, GetEncodeMode, m_encodeMode);
        if (m_encodeMode == MediaEncodeMode::MANUAL_RES_PRE_ENC)
        {
            return MOS_STATUS_SUCCESS;
        }
    }

    HucLaInitPkt *laInitpkt = MOS_New(HucLaInitPkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucLaInit, laInitpkt));
    ENCODE_CHK_STATUS_RETURN(laInitpkt->Init());

    HucLaUpdatePkt *laUpdatepkt = MOS_New(HucLaUpdatePkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucLaUpdate, laUpdatepkt));
    ENCODE_CHK_STATUS_RETURN(laUpdatepkt->Init());

    HucBrcInitPkt *brcInitpkt = MOS_New(HucBrcInitPkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucBrcInit, brcInitpkt));
    ENCODE_CHK_STATUS_RETURN(brcInitpkt->Init());

    HucBrcUpdatePkt *brcUpdatepkt = MOS_New(HucBrcUpdatePkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucBrcUpdate, brcUpdatepkt));
    ENCODE_CHK_STATUS_RETURN(brcUpdatepkt->Init());

    HevcVdencPktXe2_Lpm_Base *hevcVdencpkt = MOS_New(HevcVdencPktXe2_Lpm_Base, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(hevcVdencPacket, hevcVdencpkt));
    ENCODE_CHK_STATUS_RETURN(hevcVdencpkt->Init());

    HevcPakIntegratePkt *pakIntPkt = MOS_New(HevcPakIntegratePkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(hevcPakIntegrate, pakIntPkt));
    ENCODE_CHK_STATUS_RETURN(pakIntPkt->Init());

    HevcVdencPicPacket* hevcVdencPicPkt = MOS_New(HevcVdencPicPacket, task, hevcVdencpkt);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(hevcVdencPicPacket, hevcVdencPicPkt));
    ENCODE_CHK_STATUS_RETURN(hevcVdencPicPkt->Init());

    /*
    HucBrcTileRowUpdatePkt *brcTileRowUpdatePkt = MOS_New(HucBrcTileRowUpdatePkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucBrcTileRowUpdate, brcTileRowUpdatePkt));
    ENCODE_CHK_STATUS_RETURN(brcTileRowUpdatePkt->Init());
    */

    HevcVdencTileRowPkt* hevcVdencTileRowPkt = MOS_New(HevcVdencTileRowPkt, task, hevcVdencpkt);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(hevcVdencTileRowPacket, hevcVdencTileRowPkt));
    ENCODE_CHK_STATUS_RETURN(hevcVdencTileRowPkt->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe2_Lpm_Base::CreateFeatureManager()
{
    ENCODE_FUNC_CALL();
    m_featureManager = MOS_New(EncodeHevcVdencFeatureManagerXe2_Lpm_Base, m_allocator, m_hwInterface, m_trackedBuf, m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe2_Lpm_Base::GetSystemVdboxNumber()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(EncodePipeline::GetSystemVdboxNumber());

    MediaUserSetting::Value outValue;
    MOS_STATUS statusKey = MOS_STATUS_SUCCESS;
    statusKey            = ReadUserSetting(
        m_userSettingPtr,
        outValue,
        "Disable Media Encode Scalability",
        MediaUserSetting::Group::Sequence);

    bool disableScalability = false;
    if (statusKey == MOS_STATUS_SUCCESS)
    {
        disableScalability = outValue.Get<bool>();
    }

    if (disableScalability)
    {
        m_numVdbox = 1;
    }

    return eStatus;
}

MOS_STATUS HevcVdencPipelineXe2_Lpm_Base::Prepare(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);
    EncoderParams *encodeParams = (EncoderParams *)params;

    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams = static_cast<PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(hevcSeqParams);

    ENCODE_CHK_STATUS_RETURN(HevcVdencPipeline::Prepare(params));

    PCODEC_HEVC_ENCODE_PICTURE_PARAMS picParams = static_cast<PCODEC_HEVC_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(picParams);

    auto feature = dynamic_cast<HevcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(feature);

    uint16_t numTileRows    = 0;
    uint16_t numTileColumns = 0;
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

    bool enableTileReplay = false;
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, FeatureIDs::encodeTile, IsTileReplayEnabled, enableTileReplay);

    // ToDo: How to set the Media Function
    ENCODE_CHK_STATUS_RETURN(SwitchContext(feature->m_outputChromaFormat, numTileRows, numTileColumns, enableTileReplay));

    // Only multi-pipe contain tile report data
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, FeatureIDs::encodeTile, SetTileReportDataVaild, (GetPipeNum() > 1));

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
    inputParameters.numberTilesInFrame         = (picParams->num_tile_columns_minus1 + 1)*(picParams->num_tile_rows_minus1 + 1);

    m_statusReport->Init(&inputParameters);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe2_Lpm_Base::ActivateVdencVideoPackets()
{
    ENCODE_FUNC_CALL();
    
    bool immediateSubmit = !m_singleTaskPhaseSupported;

    if (m_preEncEnabled)
    {
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(encodePreEncPacket, immediateSubmit, 0, 0));
        if (m_encodeMode == MediaEncodeMode::MANUAL_RES_PRE_ENC)
        {
            m_activePacketList.back().immediateSubmit = true;
            return MOS_STATUS_SUCCESS;
        }
    }
    
    return HevcVdencPipeline::ActivateVdencVideoPackets();
}

MOS_STATUS HevcVdencPipelineXe2_Lpm_Base::Execute()
{
    ENCODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_ENCODE, PERF_LEVEL_HAL);

    bool isTileReplayEnabled = false;
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, FeatureIDs::encodeTile, IsTileReplayEnabled, isTileReplayEnabled);
    if (isTileReplayEnabled)
    {
        ENCODE_CHK_STATUS_RETURN(ActivateVdencTileReplayVideoPackets());
    }
    else
    {
        ENCODE_CHK_STATUS_RETURN(ActivateVdencVideoPackets());
    }

    ENCODE_CHK_STATUS_RETURN(ExecuteActivePackets());

    ENCODE_CHK_STATUS_RETURN(ResetParams());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe2_Lpm_Base::GetStatusReport(void *status, uint16_t numStatus)
{
    ENCODE_FUNC_CALL();
    m_statusReport->GetReport(numStatus, status);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe2_Lpm_Base::Destroy()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(Uninitialize());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe2_Lpm_Base::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();

    CodechalSetting *codecSettings = (CodechalSetting *)settings;
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(m_hwInterface->Initialize(codecSettings));
    ENCODE_CHK_STATUS_RETURN(InitMmcState());
    codecSettings ->isMmcEnabled = m_mmcState->IsMmcEnabled();
    ENCODE_CHK_STATUS_RETURN(HevcVdencPipeline::Initialize(settings));

    CODECHAL_DEBUG_TOOL(
        if (m_debugInterface != nullptr) {
            MOS_Delete(m_debugInterface);
        }
        m_debugInterface = MOS_New(CodechalDebugInterface);
        ENCODE_CHK_NULL_RETURN(m_debugInterface);
        ENCODE_CHK_NULL_RETURN(m_mediaCopyWrapper);
        ENCODE_CHK_STATUS_RETURN(
            m_debugInterface->Initialize(m_hwInterface, m_codecFunction, m_mediaCopyWrapper));

        if (m_statusReportDebugInterface != nullptr) {
            MOS_Delete(m_statusReportDebugInterface);
        }
        m_statusReportDebugInterface = MOS_New(CodechalDebugInterface);
        ENCODE_CHK_NULL_RETURN(m_statusReportDebugInterface);
        ENCODE_CHK_STATUS_RETURN(
            m_statusReportDebugInterface->Initialize(m_hwInterface, m_codecFunction, m_mediaCopyWrapper)););
        ENCODE_CHK_STATUS_RETURN(GetSystemVdboxNumber());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe2_Lpm_Base::Uninitialize()
{
    ENCODE_FUNC_CALL();

    if (m_mmcState != nullptr)
    {
        MOS_Delete(m_mmcState);
    }

    ENCODE_CHK_STATUS_RETURN(HevcVdencPipeline::Uninitialize());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe2_Lpm_Base::ResetParams()
{
    ENCODE_FUNC_CALL();

    m_currRecycledBufIdx =
        (m_currRecycledBufIdx + 1) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;

    if (m_currRecycledBufIdx == 0)
    {
        MOS_ZeroMemory(m_recycledBufStatusNum, sizeof(m_recycledBufStatusNum));
    }

    auto feature = dynamic_cast<EncodeBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(feature);

    // Only update user features for first frame.
    if (feature->m_frameNum == 0)
    {
        ENCODE_CHK_STATUS_RETURN(UserFeatureReport());
    }

    feature->m_frameNum++;

    RUN_FEATURE_INTERFACE_RETURN(VdencLplaAnalysis, HevcFeatureIDs::vdencLplaAnalysisFeature, UpdateLaDataIdx);

    ENCODE_CHK_STATUS_RETURN(m_statusReport->Reset());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe2_Lpm_Base::UserFeatureReport()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(HevcVdencPipeline::UserFeatureReport());

#if (_DEBUG || _RELEASE_INTERNAL)
    ReportUserSettingForDebug(
        m_userSettingPtr,
        "Enable Encode VE CtxBasedScheduling",
        MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface),
        MediaUserSetting::Group::Sequence);
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe2_Lpm_Base::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    m_mmcState = MOS_New(EncodeMemCompXe_Lpm_Plus_Base, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

}

