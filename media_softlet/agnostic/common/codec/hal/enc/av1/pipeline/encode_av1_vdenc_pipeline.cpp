/*
* Copyright (c) 2019, Intel Corporation
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

namespace encode {

Av1VdencPipeline::Av1VdencPipeline(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : Av1Pipeline(hwInterface, debugInterface)
{

}

MOS_STATUS Av1VdencPipeline::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Av1Pipeline::Initialize(settings));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipeline::Uninitialize()
{
    ENCODE_FUNC_CALL();
    return Av1Pipeline::Uninitialize();
}

MOS_STATUS Av1VdencPipeline::UserFeatureReport()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Av1Pipeline::UserFeatureReport());

#if (_DEBUG || _RELEASE_INTERNAL)
    ReportUserSettingForDebug(
        m_userSettingPtr,
        "VDENC In Use",
        1,
        MediaUserSetting::Group::Sequence);

    ReportUserSetting(
        m_userSettingPtr,
        "AV1 Encode Mode",
        m_codecFunction,
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

    ENCODE_CHK_STATUS_RETURN(Av1Pipeline::Prepare(params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipeline::ContextSwitchBack()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_scalPars);

    m_scalPars->IsContextSwitchBack = true;
    m_mediaContext->SwitchContext(VdboxEncodeFunc, m_scalPars.get(), &m_scalability);
    m_scalPars->IsContextSwitchBack = false;

    ENCODE_CHK_NULL_RETURN(m_scalability);

    m_scalability->SetPassNumber(m_featureManager->GetNumPass());

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
            ENCODE_CHK_STATUS_RETURN(ActivatePacket(Av1VdencPacket, immediateSubmit, curPass, curPipe, GetPipeNum()));
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
    m_scalPars->numVdbox = 1;
    m_scalPars->forceMultiPipe = false;

    m_scalPars->outputChromaFormat = outputChromaFormat;

    m_scalPars->numTileRows = numTileRows;
    m_scalPars->numTileColumns = numTileColumns;

    m_scalPars->IsPak = true;

    m_mediaContext->SwitchContext(VdboxEncodeFunc, &*m_scalPars, &m_scalability);
    ENCODE_CHK_NULL_RETURN(m_scalability);

    m_scalability->SetPassNumber(m_featureManager->GetNumPass());

    return MOS_STATUS_SUCCESS;
}
}
