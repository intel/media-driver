/*
* Copyright (c) 2018-2022, Intel Corporation
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
//! \file     encode_vp9_vdenc_pipeline.cpp
//! \brief    Defines the interface for vp9 vdenc encode pipeline
//!
#include "encode_vp9_brc.h"
#include "encode_vp9_hpu.h"
#include "encode_vp9_tile.h"
#include "encode_vp9_vdenc_pipeline.h"
#include "encode_vp9_vdenc_feature_manager.h"
#include "encode_utils.h"

namespace encode
{
Vp9VdencPipeline::Vp9VdencPipeline(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : Vp9Pipeline(hwInterface, debugInterface)
{

}

MOS_STATUS Vp9VdencPipeline::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Vp9Pipeline::Initialize(settings));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPipeline::Uninitialize()
{
    ENCODE_FUNC_CALL();
    return Vp9Pipeline::Uninitialize();
}

MOS_STATUS Vp9VdencPipeline::UserFeatureReport()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Vp9Pipeline::UserFeatureReport());

    auto basicFeature = dynamic_cast<Vp9BasicFeature *>(m_featureManager->GetFeature(Vp9FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);
    auto brcFeature = dynamic_cast<Vp9EncodeBrc *>(m_featureManager->GetFeature(Vp9FeatureIDs::vp9BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);
    auto hpuFeature = dynamic_cast<Vp9EncodeHpu *>(m_featureManager->GetFeature(Vp9FeatureIDs::vp9HpuFeature));
    ENCODE_CHK_NULL_RETURN(hpuFeature);

    ENCODE_CHK_STATUS_RETURN(
        ReportUserSettingValue("VP9 Encode Multipass BRC In Use",
            brcFeature->IsMultipassBrcSupported(),
            MediaUserSetting::Group::Sequence));

    ENCODE_CHK_STATUS_RETURN(
        ReportUserSettingValue("VP9 Encode Adaptive RePAK In Use",
            basicFeature->m_adaptiveRepakSupported,
            MediaUserSetting::Group::Sequence));

    ENCODE_CHK_STATUS_RETURN(
        ReportUserSettingValue("VP9 Encode HME",
            basicFeature->m_hmeSupported,
            MediaUserSetting::Group::Sequence));

    ENCODE_CHK_STATUS_RETURN(
        ReportUserSettingValue("VP9 Encode SuperHME",
            basicFeature->m_16xMeSupported,
            MediaUserSetting::Group::Sequence));

    ENCODE_CHK_STATUS_RETURN(
        ReportUserSettingValue("VP9 Encode HUC Enable",
            basicFeature->m_hucEnabled,
            MediaUserSetting::Group::Sequence));

    ENCODE_CHK_STATUS_RETURN(
        ReportUserSettingValue("Encode BRC In Use",
            brcFeature->IsBrcEnabled(),
            MediaUserSetting::Group::Sequence));

    ENCODE_CHK_STATUS_RETURN(
        ReportUserSettingValue("VDENC In Use",
            1,
            MediaUserSetting::Group::Sequence));

    ENCODE_CHK_STATUS_RETURN(
        ReportUserSettingValue("Disable Media Encode Scalability",
            !basicFeature->m_scalableMode,
            MediaUserSetting::Group::Sequence));

    ENCODE_CHK_STATUS_RETURN(
        ReportUserSettingValue("Enable Media Encode Scalability",
            basicFeature->m_scalableMode,
            MediaUserSetting::Group::Sequence));

    ENCODE_CHK_STATUS_RETURN(
        ReportUserSettingValue("VP9 Encode Mode",
            basicFeature->m_codecFunction,
            MediaUserSetting::Group::Sequence));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPipeline::Prepare(void *params)
{
    EncoderParams *encodeParams = (EncoderParams *)params;

    ENCODE_CHK_NULL_RETURN(encodeParams);

    // TODO: Should check with m_codecFunction
    if (encodeParams->ExecCodecFunction != CODECHAL_FUNCTION_ENC_VDENC_PAK)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    ENCODE_CHK_STATUS_RETURN(Vp9Pipeline::Prepare(params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPipeline::ActivateVdencVideoPackets()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureManager);

    auto basicFeature = dynamic_cast<Vp9BasicFeature *>(m_featureManager->GetFeature(Vp9FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);
    auto brcFeature = dynamic_cast<Vp9EncodeBrc *>(m_featureManager->GetFeature(Vp9FeatureIDs::vp9BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);
    bool immediateSubmit = !m_singleTaskPhaseSupported;

    auto dysRefFrameFlags = basicFeature->m_ref.DysRefFrameFlags();

    if (dysRefFrameFlags != DYS_REF_NONE && !basicFeature->m_dysVdencMultiPassEnabled)
    {
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(Vp9DynamicScal, true, 0, 0));
        m_activePacketList.back().frameTrackingRequested = false;
    }

    if (brcFeature->IsBrcInitRequired())
    {
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucBrcInit, immediateSubmit, 0, 0))
    }

    bool tileEnabled = false;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, IsEnabled, tileEnabled);

    for (uint8_t curPass = 0; curPass < GetPassNum(); curPass++)
    {
        auto isFirstPass = (curPass == 0);
        auto isLastPass  = (curPass == GetPassNum() - 1);

        if (brcFeature->IsBrcUpdateRequired())
        {
            if (!isLastPass || (curPass == 0 && GetPassNum() == 0))
            {
                if (basicFeature->m_dysBrc && dysRefFrameFlags != DYS_REF_NONE)
                {
                    ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucBrcUpdate, true, curPass, 0));
                    m_activePacketList.back().frameTrackingRequested = false;
                }
                else
                {
                    ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucBrcUpdate, immediateSubmit, curPass, 0));
                }
            }
        }

        if (basicFeature->m_resolutionChanged && !brcFeature->IsBrcInit() && curPass == 0)
        {
            brcFeature->BrcReset(true);
            ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucBrcInit, immediateSubmit, 0, 0))
        }

        if (basicFeature->m_hucEnabled)
        {
            if (isFirstPass || isLastPass || brcFeature->IsVdencBrcEnabled())
            {
                ENCODE_CHK_STATUS_RETURN(ActivatePacket(Vp9HucProb, immediateSubmit, curPass, 0));
            }
        }

        for (uint8_t curPipe = 0; curPipe < GetPipeNum(); curPipe++)
        {
            ENCODE_CHK_STATUS_RETURN(ActivatePacket(Vp9VdencPacket, immediateSubmit, curPass, curPipe, GetPipeNum()));
        }

        if (basicFeature->m_hucEnabled && tileEnabled && basicFeature->m_scalableMode)
        {
            ENCODE_CHK_STATUS_RETURN(ActivatePacket(Vp9PakIntegrate, immediateSubmit, curPass, 0));
        }
    }

    SetFrameTrackingForMultiTaskPhase();

    // Last element in m_activePacketList must be immediately submitted
    m_activePacketList.back().immediateSubmit = true;
    // In the case of Temporal Scalability, we need wait to request frame tracking until the last submission, in the super frame pass
    if (dysRefFrameFlags == DYS_REF_NONE)
    {
        m_activePacketList.front().frameTrackingRequested = !basicFeature->m_vp9PicParams->PicFlags.fields.super_frame;
    }

    if (basicFeature->m_hucEnabled)
    {
        // For Temporal scaling, super frame pass need to explicitly submit the command buffer here to call HuC
        if (basicFeature->m_vp9PicParams->PicFlags.fields.super_frame && basicFeature->m_tsEnabled)
        {
            ENCODE_CHK_STATUS_RETURN(ActivatePacket(Vp9HucSuperFrame, true, GetPassNum() - 1, GetPipeNum() - 1));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPipeline::CreateFeatureManager()
{
    ENCODE_FUNC_CALL();
    m_featureManager = MOS_New(EncodeVp9VdencFeatureManager, m_allocator, m_hwInterface, m_trackedBuf, m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_featureManager);

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
