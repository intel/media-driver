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
//! \file     encode_vp9_vdenc_pipeline_xe_lpm_plus.cpp
//! \brief    Defines the interface for vp9 vdenc encode pipeline
//!
#include "encode_vp9_vdenc_pipeline_xe_lpm_plus.h"
#include "encode_vp9_vdenc_packet_xe_lpm_plus.h"
#include "encode_vp9_huc_brc_init_packet.h"
#include "encode_vp9_huc_brc_update_packet.h"
#include "encode_vp9_hpu_packet.h"
#include "encode_vp9_hpu_super_frame_packet.h"
#include "encode_vp9_dynamic_scal_packet_xe_lpm_plus.h"
#include "encode_vp9_pak_integrate_packet_xe_lpm_plus.h"

namespace encode
{
Vp9VdencPipelineXe_Lpm_Plus::Vp9VdencPipelineXe_Lpm_Plus(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : Vp9VdencPipelineXe_Lpm_Plus_Base(hwInterface, debugInterface)
{
}

MOS_STATUS Vp9VdencPipelineXe_Lpm_Plus::Init(void *settings)
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
    ENCODE_CHK_STATUS_RETURN(brcUpdatePkt->Init())

    Vp9HpuPkt *hucProbPkt = MOS_New(Vp9HpuPkt, this, task, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(hucProbPkt);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Vp9HucProb, hucProbPkt));
    ENCODE_CHK_STATUS_RETURN(hucProbPkt->Init());

    Vp9HpuSuperFramePkt *hucSuperFramePkt = MOS_New(Vp9HpuSuperFramePkt, task, hucProbPkt);
    ENCODE_CHK_NULL_RETURN(hucSuperFramePkt);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Vp9HucSuperFrame, hucSuperFramePkt));
    ENCODE_CHK_STATUS_RETURN(hucSuperFramePkt->Init());

    Vp9DynamicScalPktXe_Lpm_Plus *vp9DynamicScalPkt = MOS_New(Vp9DynamicScalPktXe_Lpm_Plus, this, task, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(vp9DynamicScalPkt);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Vp9DynamicScal, vp9DynamicScalPkt));
    ENCODE_CHK_STATUS_RETURN(vp9DynamicScalPkt->Init());

    Vp9VdencPktXe_Lpm_Plus *vp9VdencPkt = MOS_New(Vp9VdencPktXe_Lpm_Plus, this, task, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(vp9VdencPkt);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Vp9VdencPacket, vp9VdencPkt));
    ENCODE_CHK_STATUS_RETURN(vp9VdencPkt->Init());

    Vp9PakIntegratePktXe_Lpm_Plus *pakIntPkt = MOS_New(Vp9PakIntegratePktXe_Lpm_Plus, this, task, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(pakIntPkt);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Vp9PakIntegrate, pakIntPkt));
    ENCODE_CHK_STATUS_RETURN(pakIntPkt->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPipelineXe_Lpm_Plus::CreateFeatureManager()
{
    ENCODE_FUNC_CALL();
    m_featureManager = MOS_New(EncodeVp9VdencFeatureManagerXe_Lpm_Plus, m_allocator, m_hwInterface, m_trackedBuf, m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_featureManager);
    return MOS_STATUS_SUCCESS;
}
}  // namespace encode
