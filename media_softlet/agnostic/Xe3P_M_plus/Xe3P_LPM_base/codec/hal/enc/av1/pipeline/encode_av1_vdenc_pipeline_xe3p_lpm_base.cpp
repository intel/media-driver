/*
* Copyright (c) 2023-2026, Intel Corporation
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
//! \file     encode_av1_vdenc_pipeline_xe3p_lpm_base.cpp
//! \brief    Defines the interface for av1 vdenc encode pipeline Xe3P_LPM_Base
//!
#include "encode_av1_vdenc_pipeline_xe3p_lpm_base.h"
#include "encode_utils.h"
#include "encode_av1_vdenc_packet_xe3p_lpm_base.h"
#if defined(_MEDIA_RESERVED)
#include "encode_av1_vdenc_packet_xe3p_lpm_base_ext.h"
#endif
#include "encode_av1_vdenc_feature_manager_xe3p_lpm_base.h"
#include "encode_status_report_defs.h"
#include "encode_scalability_defs.h"
#include "codechal_debug.h"
#include "encode_mem_compression_xe_lpm_plus_base.h"
#include "encode_av1_superres.h"
#include "encode_av1_brc_init_packet_xe3p_lpm_base.h"
#include "encode_av1_brc_update_packet_xe3p_lpm_base.h"
#include "encode_av1_vdenc_preenc.h"
#include "encode_preenc_packet.h"
#include "encode_av1_huc_slbb_update_packet.h"

namespace encode
{
MOS_STATUS Av1VdencPipelineXe3P_Lpm_Base::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(settings);

    ENCODE_CHK_STATUS_RETURN(Initialize(settings));

    MediaTask *task = CreateTask(MediaTask::TaskType::cmdTask);
    ENCODE_CHK_NULL_RETURN(task);

    RUN_FEATURE_INTERFACE_RETURN(Av1VdencPreEnc, Av1FeatureIDs::preEncFeature, IsEnabled, m_preEncEnabled);
    if (m_preEncEnabled)
    {
        EncodePreEncPacket* av1PreEncPkt = MOS_New(EncodePreEncPacket, this, task, m_hwInterface);
        ENCODE_CHK_STATUS_RETURN(RegisterPacket(encodePreEncPacket, av1PreEncPkt));
        ENCODE_CHK_STATUS_RETURN(av1PreEncPkt->Init());
#if USE_CODECHAL_DEBUG_TOOL
        uint32_t encodeMode = 0;
        RUN_FEATURE_INTERFACE_RETURN(Av1VdencPreEnc, Av1FeatureIDs::preEncFeature, GetEncodeMode, encodeMode);
        if (encodeMode == MediaEncodeMode::MANUAL_RES_PRE_ENC || encodeMode == MediaEncodeMode::AUTO_RES_PRE_ENC)
        {
            return MOS_STATUS_SUCCESS;
        }
#endif
    }

#if defined(_MEDIA_RESERVED)
    Av1VdencPktXe3P_Lpm_Base *av1Vdencpkt = MOS_New(Av1VdencPktXe3P_Lpm_BaseExt, this, task, m_hwInterface);
#else
    Av1VdencPktXe3P_Lpm_Base *av1Vdencpkt = MOS_New(Av1VdencPktXe3P_Lpm_Base, this, task, m_hwInterface);
#endif
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1VdencPacket, av1Vdencpkt));
    ENCODE_CHK_STATUS_RETURN(av1Vdencpkt->Init());

    Av1BrcInitPkt *brcInitpkt = MOS_New(Av1BrcInitPktXe3p_Lpm_Base, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1HucBrcInit, brcInitpkt));
    ENCODE_CHK_STATUS_RETURN(brcInitpkt->Init());
    Av1BrcUpdatePkt *brcUpdatepkt = MOS_New(Av1BrcUpdatePktXe3p_Lpm_Base, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1HucBrcUpdate, brcUpdatepkt));
    ENCODE_CHK_STATUS_RETURN(brcUpdatepkt->Init());

    // Register HucSLBBUpdate packet
    AV1HucSLBBUpdatePkt *slbbUpdatePkt = MOS_New(AV1HucSLBBUpdatePkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucSLBBUpdate, slbbUpdatePkt));
    ENCODE_CHK_STATUS_RETURN(slbbUpdatePkt->Init());

    Av1BackAnnotationPkt *av1BackAnnotationpkt = MOS_New(Av1BackAnnotationPkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1BackAnnotation, av1BackAnnotationpkt));
    ENCODE_CHK_STATUS_RETURN(av1BackAnnotationpkt->Init());

    m_sfcItf = m_hwInterface->GetMediaSfcInterface();
    ENCODE_CHK_NULL_RETURN(m_sfcItf);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe3P_Lpm_Base::Execute()
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

    ENCODE_CHK_STATUS_RETURN(Av1VdencPipeline::Execute());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe3P_Lpm_Base::CreateFeatureManager()
{
    ENCODE_FUNC_CALL();
    m_featureManager = MOS_New(EncodeAv1VdencFeatureManagerXe3P_Lpm_Base, m_allocator, m_hwInterface, m_trackedBuf, m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe3P_Lpm_Base::InitMmcState()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    m_mmcState = MOS_New(EncodeMemCompXe_Lpm_Plus_Base, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(m_mmcState);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe3P_Lpm_Base::ActivateVdencVideoPackets()
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
        RUN_FEATURE_INTERFACE_RETURN(Av1VdencPreEnc, Av1FeatureIDs::preEncFeature, GetEncodeMode, encodeMode);
        if (encodeMode == MediaEncodeMode::MANUAL_RES_PRE_ENC || encodeMode == MediaEncodeMode::AUTO_RES_PRE_ENC)
        {
            m_activePacketList.back().immediateSubmit = true;
            return MOS_STATUS_SUCCESS;
        }
#endif
    }

    // Activate HucSLBBUpdate packet after PreEnc and before BrcInit
    ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucSLBBUpdate, immediateSubmit, 0, 0));

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

}  // namespace encode
