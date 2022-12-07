/*
* Copyright (c) 2020 - 2022, Intel Corporation
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
//! \file     encode_av1_vdenc_pipeline_xe_lpm_plus.cpp
//! \brief    Defines the interface for av1 vdenc encode pipeline Xe_LPM_plus
//!

#include "encode_av1_vdenc_pipeline_xe_lpm_plus.h"
#include "encode_av1_vdenc_packet_xe_lpm_plus.h"
#include "encode_av1_brc_init_packet.h"
#include "encode_av1_brc_update_packet.h"
#include "codechal_debug.h"
#include "encode_av1_vdenc_feature_manager_xe_lpm_plus_base.h"
#include "encode_preenc_packet.h"

namespace encode
{
MOS_STATUS Av1VdencPipelineXe_LPM_Plus::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(settings);

    ENCODE_CHK_STATUS_RETURN(Initialize(settings));

    MediaTask *task = CreateTask(MediaTask::TaskType::cmdTask);
    ENCODE_CHK_NULL_RETURN(task);

    bool preEncEnabled = false;
    RUN_FEATURE_INTERFACE_RETURN(Av1VdencPreEnc, Av1FeatureIDs::preEncFeature, IsEnabled, preEncEnabled);
    if (preEncEnabled)
    {
        EncodePreEncPacket* av1PreEncPkt = MOS_New(EncodePreEncPacket, this, task, m_hwInterface);
        ENCODE_CHK_STATUS_RETURN(RegisterPacket(encodePreEncPacket, av1PreEncPkt));
        ENCODE_CHK_STATUS_RETURN(av1PreEncPkt->Init());
    }

    Av1BrcInitPkt *brcInitpkt = MOS_New(Av1BrcInitPkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1HucBrcInit, brcInitpkt));
    ENCODE_CHK_STATUS_RETURN(brcInitpkt->Init());

    Av1BrcUpdatePkt *brcUpdatepkt = MOS_New(Av1BrcUpdatePkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1HucBrcUpdate, brcUpdatepkt));
    ENCODE_CHK_STATUS_RETURN(brcUpdatepkt->Init());

    auto av1Vdencpkt = MOS_New(Av1VdencPktXe_Lpm_Plus, this, task, m_hwInterface);
    RegisterPacket(Av1VdencPacket, av1Vdencpkt);
    av1Vdencpkt->Init();

    auto av1BackAnnotationpkt = MOS_New(Av1BackAnnotationPkt, this, task, m_hwInterface);
    RegisterPacket(Av1BackAnnotation, av1BackAnnotationpkt);
    av1BackAnnotationpkt->Init();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_LPM_Plus::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();

    CodechalSetting *codecSettings = (CodechalSetting *)settings;
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(m_hwInterface->Initialize(codecSettings));
    ENCODE_CHK_STATUS_RETURN(Av1VdencPipelineXe_Lpm_Plus_Base::Initialize(settings));

    CODECHAL_DEBUG_TOOL(
        if (m_debugInterface != nullptr) {
            MOS_Delete(m_debugInterface);
        } m_debugInterface = MOS_New(CodechalDebugInterface);
        ENCODE_CHK_NULL_RETURN(m_debugInterface);
        ENCODE_CHK_STATUS_RETURN(m_debugInterface->Initialize(m_hwInterface, m_codecFunction, m_mediaCopy));

        if (m_statusReportDebugInterface != nullptr) {
            MOS_Delete(m_statusReportDebugInterface);
        } m_statusReportDebugInterface = MOS_New(CodechalDebugInterface);
        ENCODE_CHK_NULL_RETURN(m_statusReportDebugInterface);
        ENCODE_CHK_STATUS_RETURN(m_statusReportDebugInterface->Initialize(m_hwInterface, m_codecFunction, m_mediaCopy)););

    return MOS_STATUS_SUCCESS;
}
}  // namespace encode
