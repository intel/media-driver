/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     encode_hevc_vdenc_pipeline_xe_xpm_base.cpp
//! \brief    Defines the interface for hevc vdenc encode pipeline
//!
#include "encode_hevc_vdenc_pipeline_xe_xpm_base.h"
#include "encode_utils.h"
#include "encode_status_report_defs.h"
#include "encode_scalability_defs.h"
#include "encode_mem_compression_g12.h"
#include "encode_vdenc_lpla_analysis.h"

namespace encode {

HevcVdencPipelineXe_Xpm_Base::HevcVdencPipelineXe_Xpm_Base(
    CodechalHwInterfaceNext     *hwInterface,
    CodechalDebugInterface  *debugInterface)
    : HevcVdencPipeline(hwInterface, debugInterface)
{

}

MOS_STATUS HevcVdencPipelineXe_Xpm_Base::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe_Xpm_Base::GetSystemVdboxNumber()
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
    bool disableScalability = m_hwInterface->IsDisableScalability();
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

MOS_STATUS HevcVdencPipelineXe_Xpm_Base::Prepare(void *params)
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

    EncodeScalabilityPars scalPars;
    MOS_ZeroMemory(&scalPars, sizeof(EncodeScalabilityPars));
    scalPars.enableVDEnc = true;
    scalPars.enableVE = MOS_VE_SUPPORTED(m_osInterface);
    scalPars.numVdbox = m_numVdbox;

    scalPars.forceMultiPipe = true;
    scalPars.outputChromaFormat = feature->m_outputChromaFormat;

    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, FeatureIDs::encodeTile, GetTileRowColumns,
        scalPars.numTileRows, scalPars.numTileColumns);

    scalPars.IsPak = true;

    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, FeatureIDs::encodeTile, IsTileReplayEnabled, scalPars.enableTileReplay);

    m_mediaContext->SwitchContext(VdboxEncodeFunc, &scalPars, &m_scalability);
    ENCODE_CHK_NULL_RETURN(m_scalability);

    // Only multi-pipe contain tile report data
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, FeatureIDs::encodeTile, SetTileReportDataVaild,
        (GetPipeNum() > 1));

    m_scalability->SetPassNumber(m_featureManager->GetNumPass());

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

MOS_STATUS HevcVdencPipelineXe_Xpm_Base::Execute()
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

MOS_STATUS HevcVdencPipelineXe_Xpm_Base::GetStatusReport(void *status, uint16_t numStatus)
{
    ENCODE_FUNC_CALL();
    m_statusReport->GetReport(numStatus, status);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe_Xpm_Base::Destroy()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(Uninitialize());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe_Xpm_Base::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(InitMmcState());
    CodechalSetting* codecSettings = (CodechalSetting*)settings;
    codecSettings ->isMmcEnabled = m_mmcState ? m_mmcState->IsMmcEnabled() : false;
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

MOS_STATUS HevcVdencPipelineXe_Xpm_Base::Uninitialize()
{
    ENCODE_FUNC_CALL();

    if (m_mmcState != nullptr)
    {
        MOS_Delete(m_mmcState);
    }

    ENCODE_CHK_STATUS_RETURN(HevcVdencPipeline::Uninitialize());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe_Xpm_Base::ResetParams()
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

MOS_STATUS HevcVdencPipelineXe_Xpm_Base::UserFeatureReport()
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

MOS_STATUS HevcVdencPipelineXe_Xpm_Base::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    m_mmcState = MOS_New(EncodeMemCompG12, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe_Xpm_Base::CreateFeatureManager()
{
    ENCODE_FUNC_CALL();
    m_featureManager = MOS_New(EncodeHevcVdencFeatureManagerXe_Xpm_Base, m_allocator, m_hwInterface, m_trackedBuf, m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe_Xpm_Base::Reformat()
{
    ENCODE_FUNC_CALL();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe_Xpm_Base::PrepareReformat()
{
    ENCODE_FUNC_CALL();
    return MOS_STATUS_SUCCESS;
}

}
