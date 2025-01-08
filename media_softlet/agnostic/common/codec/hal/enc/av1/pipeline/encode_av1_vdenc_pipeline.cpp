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
//! \file     encode_av1_vdenc_pipeline.cpp
//! \brief    Defines the interface for av1 vdenc encode pipeline
//!
#include "encode_av1_vdenc_pipeline.h"
#include "codec_def_encode.h"
#include "encode_utils.h"
#include "encode_av1_tile.h"
#include "encode_av1_basic_feature.h"
#include "encode_av1_vdenc_preenc.h"
#include "codechal_debug.h"

namespace encode {

Av1VdencPipeline::Av1VdencPipeline(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : Av1Pipeline(hwInterface, debugInterface)
{

}

MOS_STATUS Av1VdencPipeline::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Av1Pipeline::Initialize(settings));

    // Init hwInterface
    CodechalSetting *codecSettings = (CodechalSetting *)settings;
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(m_hwInterface->Initialize(codecSettings));

    ENCODE_CHK_STATUS_RETURN(InitMmcState());

    ENCODE_CHK_STATUS_RETURN(GetSystemVdboxNumber());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipeline::Destroy()
{
    ENCODE_FUNC_CALL();

    MOS_Delete(m_mmcState);
    
    return Av1Pipeline::Uninitialize();
}

MOS_STATUS Av1VdencPipeline::UserFeatureReport()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Av1Pipeline::UserFeatureReport());

    ReportUserSetting(
        m_userSettingPtr,
        "AV1 Encode Mode",
        m_codecFunction,
        MediaUserSetting::Group::Sequence);

#if (_DEBUG || _RELEASE_INTERNAL)
    ReportUserSettingForDebug(
        m_userSettingPtr,
        "VDENC In Use",
        1,
        MediaUserSetting::Group::Sequence);

    ReportUserSettingForDebug(
        m_userSettingPtr,
        "Enable Encode VE CtxBasedScheduling",
        MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface),
        MediaUserSetting::Group::Sequence);
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipeline::Prepare(void *params)
{
    EncoderParams *encodeParams = (EncoderParams *)params;

    ENCODE_CHK_NULL_RETURN(encodeParams);

    if (encodeParams->ExecCodecFunction != CODECHAL_FUNCTION_ENC_VDENC_PAK)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    auto feature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(feature);
    feature->m_dualEncEnable = m_dualEncEnable;

    ENCODE_CHK_STATUS_RETURN(Av1Pipeline::Prepare(params));

    uint16_t numTileRows = 0;
    uint16_t numTileColumns = 0;
    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

    ENCODE_CHK_STATUS_RETURN(SwitchContext(feature->m_outputChromaFormat, numTileRows, numTileColumns));

    EncoderStatusParameters inputParameters = {};

    ENCODE_CHK_STATUS_RETURN(FillStatusReportParameters(&inputParameters, encodeParams));

    ENCODE_CHK_STATUS_RETURN(m_statusReport->Init(&inputParameters));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipeline::ResetParams()
{
    ENCODE_FUNC_CALL();

    m_currRecycledBufIdx = (m_currRecycledBufIdx + 1) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;

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

MOS_STATUS Av1VdencPipeline::GetStatusReport(void *status, uint16_t numStatus)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(m_statusReport->GetReport(numStatus, status));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipeline::Execute()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(ActivateVdencVideoPackets());
    ENCODE_CHK_STATUS_RETURN(ExecuteActivePackets());

    ENCODE_CHK_STATUS_RETURN(ResetParams());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipeline::HuCCheckAndInit()
{
    ENCODE_FUNC_CALL();

    bool immediateSubmit = !m_singleTaskPhaseSupported;
    ENCODE_CHK_STATUS_RETURN(ActivatePacket(Av1HucBrcInit, immediateSubmit, 0, 0));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipeline::ActivateVdencVideoPackets()
{
    ENCODE_FUNC_CALL();

    bool immediateSubmit = !m_singleTaskPhaseSupported;

    ENCODE_CHK_NULL_RETURN(m_featureManager);

    auto basicFeature = dynamic_cast<Av1BasicFeature*>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);

    auto brcFeature = dynamic_cast<Av1Brc*>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    if (m_preEncEnabled)
    {
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(encodePreEncPacket, immediateSubmit, 0, 0));
#if USE_CODECHAL_DEBUG_TOOL
        uint32_t encodeMode = 0; 
        RUN_FEATURE_INTERFACE_RETURN(Av1VdencPreEnc, FeatureIDs::preEncFeature, GetEncodeMode, encodeMode);
        if (encodeMode == MediaEncodeMode::MANUAL_RES_PRE_ENC || encodeMode == MediaEncodeMode::AUTO_RES_PRE_ENC)
        {
            m_activePacketList.back().immediateSubmit = true;
            return MOS_STATUS_SUCCESS;
        }
#endif
    }

    if (brcFeature->IsBRCInitRequired())
    {
        ENCODE_CHK_STATUS_RETURN(HuCCheckAndInit());
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

        if ((basicFeature->m_enableTileStitchByHW || !basicFeature -> m_enableSWStitching || brcFeature->IsBRCEnabled()) && m_dualEncEnable)
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

MOS_STATUS Av1VdencPipeline::CreateFeatureManager()
{
    ENCODE_FUNC_CALL();
    m_featureManager = MOS_New(EncodeAv1VdencFeatureManager, m_allocator, m_hwInterface, m_trackedBuf, m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipeline::SwitchContext(uint8_t outputChromaFormat, uint16_t numTileRows, uint16_t numTileColumns)
{
    ENCODE_FUNC_CALL();

    if (!m_scalPars)
    {
        m_scalPars = std::make_shared<EncodeScalabilityPars>();
    }

    *m_scalPars = {};
    m_scalPars->enableVDEnc = true;
    m_scalPars->enableVE = MOS_VE_SUPPORTED(m_osInterface);

    //force to disable scalability for AV1 VDENC
    if(m_dualEncEnable)
    {
        m_scalPars->numVdbox = m_numVdbox;
        m_scalPars->forceMultiPipe = true;
        m_scalPars->allowSwArbitarySplit = true;
    }
    else
    {
        m_scalPars->numVdbox = 1;
        m_scalPars->forceMultiPipe = false;
        m_scalPars->allowSwArbitarySplit = false;
    }

    m_scalPars->outputChromaFormat = outputChromaFormat;

    m_scalPars->numTileRows = numTileRows;
    m_scalPars->numTileColumns = numTileColumns;

    m_scalPars->IsPak = true;

    m_mediaContext->SwitchContext(VdboxEncodeFunc, &*m_scalPars, &m_scalability);
    ENCODE_CHK_NULL_RETURN(m_scalability);

    m_scalability->SetPassNumber(m_featureManager->GetNumPass());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipeline::FillStatusReportParameters(EncoderStatusParameters* pPar, EncoderParams* encodeParams)
{
    ENCODE_CHK_NULL_RETURN(pPar);
    ENCODE_CHK_NULL_RETURN(encodeParams);

    PCODEC_AV1_ENCODE_PICTURE_PARAMS picParams = static_cast<PCODEC_AV1_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(picParams);

    auto feature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(feature);

    uint16_t numTileRows = 0;
    uint16_t numTileColumns = 0;
    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileRowColumns,
        numTileRows, numTileColumns);

    pPar->statusReportFeedbackNumber = picParams->StatusReportFeedbackNumber;
    pPar->pBlkQualityInfo            = picParams->pBlkQualityInfo;
    pPar->codecFunction              = encodeParams->ExecCodecFunction;
    pPar->currRefList                = feature->m_ref.GetCurrRefList();
    pPar->picWidthInMb               = feature->m_picWidthInMb;
    pPar->frameFieldHeightInMb       = feature->m_frameFieldHeightInMb;
    pPar->currOriginalPic            = feature->m_currOriginalPic;
    pPar->pictureCodingType          = feature->m_pictureCodingType;
    pPar->numUsedVdbox               = m_numVdbox;
    pPar->hwWalker                   = false;
    pPar->maxNumSlicesAllowed        = 0;

    pPar->numberTilesInFrame         = numTileRows * numTileColumns;

    pPar->av1EnableFrameObu            = feature->m_av1PicParams->PicFlags.fields.EnableFrameOBU;
    pPar->av1FrameHdrOBUSizeByteOffset = feature->m_frameHdrOBUSizeByteOffset;
    pPar->frameWidth                   = feature->m_frameWidth;
    pPar->frameHeight                  = feature->m_frameHeight;

    return MOS_STATUS_SUCCESS;
}

}
