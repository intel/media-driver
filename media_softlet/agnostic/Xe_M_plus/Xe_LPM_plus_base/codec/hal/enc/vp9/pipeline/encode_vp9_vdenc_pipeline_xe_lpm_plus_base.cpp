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
//! \file     encode_vp9_vdenc_pipeline_xe_lpm_plus_base.cpp
//! \brief    Defines the interface for vp9 vdenc encode pipeline
//!
#include "encode_vp9_vdenc_pipeline_xe_lpm_plus_base.h"
#include "encode_utils.h"
#include "encode_vp9_vdenc_packet_xe_lpm_plus_base.h"
#include "encode_vp9_huc_brc_init_packet.h"
#include "encode_vp9_huc_brc_update_packet.h"
#include "encode_vp9_hpu_packet.h"
#include "encode_vp9_hpu_super_frame_packet.h"
#include "encode_vp9_dynamic_scal_packet_xe_lpm_plus_base.h"
#include "encode_vp9_pak_integrate_packet.h"
#include "encode_status_report_defs.h"
#include "encode_scalability_defs.h"
#include "codechal_debug.h"
#include "encode_mem_compression_xe_lpm_plus_base.h"

namespace encode
{
Vp9VdencPipelineXe_Lpm_Plus_Base::Vp9VdencPipelineXe_Lpm_Plus_Base(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : Vp9VdencPipeline(hwInterface, debugInterface)
{
}

MOS_STATUS Vp9VdencPipelineXe_Lpm_Plus_Base::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(settings);

    ENCODE_CHK_STATUS_RETURN(Initialize(settings));

    MediaTask *task = CreateTask(MediaTask::TaskType::cmdTask);
    ENCODE_CHK_NULL_RETURN(task);

    Vp9HucBrcInitPkt *brcInitPkt = MOS_New(Vp9HucBrcInitPkt, this, task, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(brcInitPkt);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucBrcInit, brcInitPkt));
    ENCODE_CHK_STATUS_RETURN(brcInitPkt->Init());

    Vp9HucBrcUpdatePkt *brcUpdatePkt = MOS_New(Vp9HucBrcUpdatePkt, this, task, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(brcUpdatePkt);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucBrcUpdate, brcUpdatePkt));
    ENCODE_CHK_STATUS_RETURN(brcUpdatePkt->Init());

    Vp9HpuPkt *hucProbPkt = MOS_New(Vp9HpuPkt, this, task, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(hucProbPkt);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Vp9HucProb, hucProbPkt));
    ENCODE_CHK_STATUS_RETURN(hucProbPkt->Init());

    Vp9HpuSuperFramePkt *hucSuperFramePkt = MOS_New(Vp9HpuSuperFramePkt, task, hucProbPkt);
    ENCODE_CHK_NULL_RETURN(hucSuperFramePkt);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Vp9HucSuperFrame, hucSuperFramePkt));
    ENCODE_CHK_STATUS_RETURN(hucSuperFramePkt->Init());

    Vp9DynamicScalPktXe_Lpm_Plus_Base *vp9DynamicScalPkt = MOS_New(Vp9DynamicScalPktXe_Lpm_Plus_Base, this, task, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(vp9DynamicScalPkt);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Vp9DynamicScal, vp9DynamicScalPkt));
    ENCODE_CHK_STATUS_RETURN(vp9DynamicScalPkt->Init());

    Vp9VdencPktXe_Lpm_Plus_Base *vp9VdencPkt = MOS_New(Vp9VdencPktXe_Lpm_Plus_Base, this, task, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(vp9VdencPkt);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Vp9VdencPacket, vp9VdencPkt));
    ENCODE_CHK_STATUS_RETURN(vp9VdencPkt->Init());

    Vp9PakIntegratePkt *pakIntPkt = MOS_New(Vp9PakIntegratePkt, this, task, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(pakIntPkt);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Vp9PakIntegrate, pakIntPkt));
    ENCODE_CHK_STATUS_RETURN(pakIntPkt->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPipelineXe_Lpm_Plus_Base::GetSystemVdboxNumber()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(EncodePipeline::GetSystemVdboxNumber());

    MediaUserSetting::Value outValue;
    MOS_STATUS              statusKey = MOS_STATUS_SUCCESS;
    statusKey                         = ReadUserSetting(
        m_userSettingPtr,
        outValue,
        "Enable Media Encode Scalability",
        MediaUserSetting::Group::Sequence);

    bool disableScalability = m_hwInterface->IsDisableScalability();
    if (statusKey == MOS_STATUS_SUCCESS)
    {
        disableScalability = !outValue.Get<bool>();
    }

    MEDIA_ENGINE_INFO mediaSysInfo;
    MOS_ZeroMemory(&mediaSysInfo, sizeof(MEDIA_ENGINE_INFO));
    eStatus = m_osInterface->pfnGetMediaEngineInfo(m_osInterface, mediaSysInfo);

    if (eStatus == MOS_STATUS_SUCCESS && (!MEDIA_IS_SKU(m_skuTable, FtrWithSlimVdbox)) && disableScalability == false)
    {
        m_numVdbox = (uint8_t)(mediaSysInfo.VDBoxInfo.NumberOfVDBoxEnabled);
    }
    else
    {
        m_numVdbox = 1;
    }

    return eStatus;
}

MOS_STATUS Vp9VdencPipelineXe_Lpm_Plus_Base::Prepare(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);
    EncoderParams *encodeParams = (EncoderParams *)params;

    MOS_GPUCTX_CREATOPTIONS createOption;

    PCODEC_VP9_ENCODE_SEQUENCE_PARAMS seqParams = static_cast<PCODEC_VP9_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(seqParams);

    ENCODE_CHK_STATUS_RETURN(Vp9VdencPipeline::Prepare(params));

    PCODEC_VP9_ENCODE_PIC_PARAMS picParams = static_cast<PCODEC_VP9_ENCODE_PIC_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(picParams);

    auto basicFeature = dynamic_cast<Vp9BasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);

    // Number of tile rows and columns
    uint16_t numTileRows    = 0;
    uint16_t numTileColumns = 0;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

    // EncodeScalabilityPars settings for switch context
    EncodeScalabilityPars scalPars;
    MOS_ZeroMemory(&scalPars, sizeof(EncodeScalabilityPars));
    scalPars.enableVDEnc = true;
    scalPars.enableVE    = MOS_VE_SUPPORTED(m_osInterface);
    scalPars.numVdbox    = m_numVdbox;

    scalPars.forceMultiPipe     = true;
    scalPars.outputChromaFormat = basicFeature->m_outputChromaFormat;
    scalPars.numTileRows        = numTileRows;
    scalPars.numTileColumns     = numTileColumns;

    scalPars.IsPak = true;

    // Switch the media encode function context
    m_mediaContext->SwitchContext(VdboxEncodeFunc, &scalPars, &m_scalability);
    ENCODE_CHK_NULL_RETURN(m_scalability);

    m_scalability->SetPassNumber(m_featureManager->GetNumPass());

    // KMD VE is now enabled by default. MediaSolo can also use the VE interface.
    basicFeature->m_scalableMode = (m_scalability->GetPipeNumber() > 1);
    // Last place where scalable mode is decided
    if (basicFeature->m_frameNum == 0)
    {
        basicFeature->m_lastFrameScalableMode = basicFeature->m_scalableMode;
    }

    EncoderStatusParameters inputParameters = {};
    MOS_ZeroMemory(&inputParameters, sizeof(EncoderStatusParameters));

    inputParameters.statusReportFeedbackNumber = picParams->StatusReportFeedbackNumber;
    inputParameters.codecFunction              = encodeParams->ExecCodecFunction;
    inputParameters.currRefList                = basicFeature->m_ref.GetCurrRefList();
    inputParameters.picWidthInMb               = basicFeature->m_picWidthInMb;
    inputParameters.frameFieldHeightInMb       = basicFeature->m_frameFieldHeightInMb;
    inputParameters.currOriginalPic            = basicFeature->m_currOriginalPic;
    inputParameters.pictureCodingType          = basicFeature->m_pictureCodingType;
    inputParameters.numUsedVdbox               = m_numVdbox;
    inputParameters.hwWalker                   = false;
    inputParameters.maxNumSlicesAllowed        = 0;

    inputParameters.numberTilesInFrame = numTileRows * numTileColumns;

    m_statusReport->Init(&inputParameters);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPipelineXe_Lpm_Plus_Base::Execute()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(ActivateVdencVideoPackets());
    ENCODE_CHK_STATUS_RETURN(ExecuteActivePackets());

    ENCODE_CHK_STATUS_RETURN(ResetParams());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPipelineXe_Lpm_Plus_Base::GetStatusReport(void *status, uint16_t numStatus)
{
    ENCODE_FUNC_CALL();
    m_statusReport->GetReport(numStatus, status);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPipelineXe_Lpm_Plus_Base::Destroy()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(Uninitialize());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPipelineXe_Lpm_Plus_Base::InitMmcState()
{
    ENCODE_FUNC_CALL();

#ifdef _MMC_SUPPORTED
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    m_mmcState = MOS_New(EncodeMemCompXe_Lpm_Plus_Base, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPipelineXe_Lpm_Plus_Base::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();
    CodechalSetting *codecSettings = (CodechalSetting *)settings;
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(m_hwInterface->Initialize(codecSettings));
    ENCODE_CHK_STATUS_RETURN(InitMmcState());
    ENCODE_CHK_STATUS_RETURN(Vp9VdencPipeline::Initialize(settings));

    CODECHAL_DEBUG_TOOL(
        if (m_debugInterface != nullptr) {
            MOS_Delete(m_debugInterface);
        } m_debugInterface = MOS_New(CodechalDebugInterface);
        ENCODE_CHK_NULL_RETURN(m_debugInterface);
        ENCODE_CHK_NULL_RETURN(m_mediaCopyWrapper);
        ENCODE_CHK_STATUS_RETURN(
            m_debugInterface->Initialize(m_hwInterface, m_codecFunction, m_mediaCopyWrapper));

        if (m_statusReportDebugInterface != nullptr) {
            MOS_Delete(m_statusReportDebugInterface);
        } m_statusReportDebugInterface = MOS_New(CodechalDebugInterface);
        ENCODE_CHK_NULL_RETURN(m_statusReportDebugInterface);
        ENCODE_CHK_STATUS_RETURN(
            m_statusReportDebugInterface->Initialize(m_hwInterface, m_codecFunction, m_mediaCopyWrapper)););

    ENCODE_CHK_STATUS_RETURN(GetSystemVdboxNumber());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPipelineXe_Lpm_Plus_Base::Uninitialize()
{
    ENCODE_FUNC_CALL();

    if (m_mmcState != nullptr)
    {
        MOS_Delete(m_mmcState);
    }

    ENCODE_CHK_STATUS_RETURN(Vp9VdencPipeline::Uninitialize());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPipelineXe_Lpm_Plus_Base::ResetParams()
{
    ENCODE_FUNC_CALL();

    m_currRecycledBufIdx =
        (m_currRecycledBufIdx + 1) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;

    if (m_currRecycledBufIdx == 0)
    {
        MOS_ZeroMemory(m_recycledBufStatusNum, sizeof(m_recycledBufStatusNum));
    }

    auto basicFeature = dynamic_cast<Vp9BasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);

    // Only update user features for first frame.
    if (basicFeature->m_frameNum == 0)
    {
        ENCODE_CHK_STATUS_RETURN(UserFeatureReport());
    }

    basicFeature->m_frameNum++;
    // Save the last frame's scalable mode flag to prevent switching buffers when doing next pass
    basicFeature->m_lastFrameScalableMode = basicFeature->m_scalableMode;
    // store m_currRecycledBufIdx in basic feature class for other features classes access it
    basicFeature->m_currRecycledBufIdx = m_currRecycledBufIdx;

    ENCODE_CHK_STATUS_RETURN(m_statusReport->Reset());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPipelineXe_Lpm_Plus_Base::UserFeatureReport()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(Vp9VdencPipeline::UserFeatureReport());

    ENCODE_CHK_STATUS_RETURN(
        ReportUserSettingValue("Enable Encode VE CtxBasedScheduling",
            MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface),
            MediaUserSetting::Group::Sequence));

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode