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
//! \file     encode_avc_vdenc_pipeline_xe3p_lpm.cpp
//! \brief    Defines the interface for avc vdenc encode pipeline Xe3P_LPM
//!

#include "encode_avc_vdenc_pipeline_xe3p_lpm.h"
#include "encode_avc_vdenc_packet_xe3p_lpm.h"
#include "encode_avc_huc_brc_init_packet_xe3p_lpm_base.h"
#include "encode_avc_huc_brc_update_packet_xe3p_lpm_base.h"
#include "encode_avc_huc_slbb_update_packet.h"
#include "encode_avc_vdenc_feature_manager_xe3p_lpm.h"
#include "codechal_debug.h"
#include "encode_mem_compression_xe3p_lpm_base.h"
#include "encode_avc_vdenc_preenc.h"
#include "encode_avc_vdenc_pipeline.h"
#include "encode_preenc_packet.h"

namespace encode {

MOS_STATUS AvcVdencPipelineXe3P_Lpm::Init(void *settings)
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

    AVCHucSLBBUpdatePkt *slbbUpdatePkt = MOS_New(AVCHucSLBBUpdatePkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucSLBBUpdate, slbbUpdatePkt));
    ENCODE_CHK_STATUS_RETURN(slbbUpdatePkt->Init());

    AvcVdencPktXe3P_Lpm *avcVdencpkt = MOS_New(AvcVdencPktXe3P_Lpm, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(VdencPacket, avcVdencpkt));
    ENCODE_CHK_STATUS_RETURN(avcVdencpkt->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipelineXe3P_Lpm::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();

    CodechalSetting *codecSettings = (CodechalSetting *)settings;
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(m_hwInterface->Initialize(codecSettings));
    ENCODE_CHK_STATUS_RETURN(InitMmcState());
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

MOS_STATUS AvcVdencPipelineXe3P_Lpm::CreateFeatureManager()
{
    ENCODE_FUNC_CALL();

    m_featureManager = MOS_New(EncodeAvcVdencFeatureManagerXe3P_Lpm, m_allocator, m_hwInterface, m_trackedBuf, m_recycleBuf, m_mediaCopyWrapper);
    ENCODE_CHK_NULL_RETURN(m_featureManager);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipelineXe3P_Lpm::InitMmcState()
{
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    m_mmcState = MOS_New(EncodeMemCompXe3P_Lpm_Base, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(m_mmcState);

    return MOS_STATUS_SUCCESS;
}

}
