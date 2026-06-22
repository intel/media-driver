/*
* Copyright (c) 2026, Intel Corporation
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
//! \file     encode_avc_vdenc_pipeline_xe3p_lpm_base.cpp
//! \brief    Defines the interface for avc vdenc encode pipeline Xe3P_LPM_Base
//!
#include "encode_avc_vdenc_pipeline_xe3p_lpm_base.h"
#include "encode_avc_huc_brc_init_packet_xe3p_lpm_base.h"
#include "encode_avc_huc_brc_update_packet_xe3p_lpm_base.h"
#include "encode_avc_huc_slbb_update_packet.h"
#include "encode_avc_vdenc_packet_xe3p_lpm.h"
#include "encode_avc_vdenc_feature_manager.h"
#include "encode_mem_compression_xe3p_lpm_base.h"
#include "encode_preenc_packet.h"
#include "encode_avc_brc.h"
#include "encode_avc_vdenc_preenc.h"
#include "media_avc_feature_defs.h"
#include "media_perf_profiler.h"

namespace encode {

MOS_STATUS AvcVdencPipelineXe3P_Lpm_Base::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(AvcVdencPipeline::Initialize(settings));

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_osInterface && m_osInterface->bNullHwIsEnabled)
    {
        m_bypassHW = MOS_New(BypassHwLegacy);
        ENCODE_CHK_NULL_RETURN(m_bypassHW);

        MOS_STATUS status = m_bypassHW->Initialize(m_osInterface, m_hwInterface->GetMiInterfaceNext());
        if (status == MOS_STATUS_SUCCESS)
        {
            m_osInterface->bNullHwIsEnabled = true;

            auto *codecSettings = static_cast<CodechalSetting *>(settings);
            ENCODE_CHK_NULL_RETURN(codecSettings);

            uint32_t bitDepth = (codecSettings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_12_BITS) ?
                12 : ((codecSettings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_10_BITS) ? 10 : 8);

            ENCODE_CHK_STATUS_RETURN(m_bypassHW->FetchDummyVdNode(
                m_gpuNode,
                CODECHAL_AVC,
                true,
                codecSettings->width,
                codecSettings->height,
                static_cast<uint8_t>(codecSettings->chromaFormat),
                static_cast<uint8_t>(bitDepth),
                0));
        }
        else
        {
            MOS_Delete(m_bypassHW);
            m_bypassHW = nullptr;
        }
    }
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipelineXe3P_Lpm_Base::Uninitialize()
{
    ENCODE_FUNC_CALL();

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_bypassHW != nullptr)
    {
        m_bypassHW->Destroy();
        MOS_Delete(m_bypassHW);
        m_bypassHW = nullptr;
    }
#endif

    return AvcVdencPipeline::Uninitialize();
}

MOS_STATUS AvcVdencPipelineXe3P_Lpm_Base::SwitchContext(uint8_t outputChromaFormat)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_osInterface && m_osInterface->bNullHwIsEnabled)
    {
        ENCODE_FUNC_CALL();

        if (!m_scalPars)
        {
            m_scalPars = std::make_shared<EncodeScalabilityPars>();
        }

        *m_scalPars                    = {};
        m_scalPars->enableVDEnc        = true;
        m_scalPars->enableVE           = MOS_VE_SUPPORTED(m_osInterface);
        m_scalPars->numVdbox           = 1;
        m_scalPars->forceMultiPipe     = false;
        m_scalPars->outputChromaFormat = outputChromaFormat;
        m_scalPars->numTileRows        = 1;
        m_scalPars->numTileColumns     = 1;
        m_scalPars->IsPak              = true;

        MediaFunction encFunc = (m_gpuNode == MOS_GPU_NODE_VE) ? VeboxVppFunc : VdboxEncodeFunc;
        ENCODE_CHK_STATUS_RETURN(m_mediaContext->SwitchContext(encFunc, &*m_scalPars, &m_scalability));
        ENCODE_CHK_NULL_RETURN(m_scalability);

        m_scalability->SetPassNumber(m_featureManager->GetNumPass());

        return MOS_STATUS_SUCCESS;
    }
#endif
    return AvcVdencPipeline::SwitchContext(outputChromaFormat);
}

MOS_STATUS AvcVdencPipelineXe3P_Lpm_Base::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(settings);

    ENCODE_CHK_STATUS_RETURN(Initialize(settings));

    MediaTask* task = CreateTask(MediaTask::TaskType::cmdTask);
    ENCODE_CHK_NULL_RETURN(task);

    RUN_FEATURE_INTERFACE_RETURN(AvcVdencPreEnc, FeatureIDs::preEncFeature, IsEnabled, m_preEncEnabled);
    if (m_preEncEnabled)
    {
        EncodePreEncPacket *avcPreEncpkt = MOS_New(EncodePreEncPacket, this, task, m_hwInterface);
        ENCODE_CHK_STATUS_RETURN(RegisterPacket(encodePreEncPacket, avcPreEncpkt));
        ENCODE_CHK_STATUS_RETURN(avcPreEncpkt->Init());

        RUN_FEATURE_INTERFACE_RETURN(AvcVdencPreEnc, FeatureIDs::preEncFeature, GetEncodeMode, m_encodeMode);
        if (m_encodeMode == MediaEncodeMode::MANUAL_RES_PRE_ENC || m_encodeMode == MediaEncodeMode::AUTO_RES_PRE_ENC)
        {
            return MOS_STATUS_SUCCESS;
        }
    }

    AvcHucBrcInitPktXe3p_Lpm_Base *brcInitpkt = MOS_New(AvcHucBrcInitPktXe3p_Lpm_Base, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucBrcInit, brcInitpkt));
    ENCODE_CHK_STATUS_RETURN(brcInitpkt->Init());

    AvcHucBrcUpdatePktXe3p_Lpm_Base *brcUpdatepkt = MOS_New(AvcHucBrcUpdatePktXe3p_Lpm_Base, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucBrcUpdate, brcUpdatepkt));
    ENCODE_CHK_STATUS_RETURN(brcUpdatepkt->Init());

    // Register HucSLBBUpdate packet
    AVCHucSLBBUpdatePkt *slbbUpdatePkt = MOS_New(AVCHucSLBBUpdatePkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucSLBBUpdate, slbbUpdatePkt));
    ENCODE_CHK_STATUS_RETURN(slbbUpdatePkt->Init());

    AvcVdencPktXe3P_Lpm *avcVdencpkt = MOS_New(AvcVdencPktXe3P_Lpm, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(VdencPacket, avcVdencpkt));
    ENCODE_CHK_STATUS_RETURN(avcVdencpkt->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipelineXe3P_Lpm_Base::ActivateVdencVideoPackets()
{
    ENCODE_FUNC_CALL();

    auto brcFeature = dynamic_cast<AvcEncodeBRC*>(m_featureManager->GetFeature(AvcFeatureIDs::avcBrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);
    bool immediateSubmit = !m_singleTaskPhaseSupported;

    // Activate PreEnc packet if enabled
    if (m_preEncEnabled)
    {
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(encodePreEncPacket, immediateSubmit, 0, 0));
        if (m_encodeMode == MediaEncodeMode::MANUAL_RES_PRE_ENC || m_encodeMode == MediaEncodeMode::AUTO_RES_PRE_ENC)
        {
            m_activePacketList.back().immediateSubmit = true;
            return MOS_STATUS_SUCCESS;
        }
    }

    // Activate HucSLBBUpdate packet after PreEnc and before BrcInit
    ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucSLBBUpdate, immediateSubmit, 0, 0));

    // Activate BrcInit if required
    if (brcFeature->IsBRCInitRequired())
    {
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucBrcInit, immediateSubmit, 0, 0));
    }

    // Activate BrcUpdate and Vdenc for each pass
    for (uint8_t curPass = 0; curPass < GetPassNum(); curPass++)
    {
        if (brcFeature->IsBRCUpdateRequired())
        {
            ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucBrcUpdate, immediateSubmit, curPass, 0));
        }
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(VdencPacket, immediateSubmit, curPass, 0));
    }

    SetFrameTrackingForMultiTaskPhase();
    m_activePacketList.back().immediateSubmit = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipelineXe3P_Lpm_Base::InitMmcState()
{
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    m_mmcState = MOS_New(EncodeMemCompXe3P_Lpm_Base, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(m_mmcState);
    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
