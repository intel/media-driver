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
//! \file     encode_av1_vdenc_pipeline_xe_hpm.cpp
//! \brief    Defines the interface for av1 vdenc encode pipeline Xe_HPM
//!
#include "encode_av1_vdenc_pipeline_xe_hpm.h"
#include "encode_av1_vdenc_packet_xe_hpm.h"
#include "encode_av1_vdenc_feature_manager_xe_hpm.h"
#include "encode_av1_brc_init_packet.h"
#include "encode_av1_brc_update_packet.h"
#include "encode_av1_pak_integrate_packet.h"
#include "encode_check_huc_load_packet.h"
#include "codechal_debug.h"

namespace encode {

Av1VdencPipelineXe_Hpm::Av1VdencPipelineXe_Hpm(
    CodechalHwInterfaceNext     *hwInterface,
    CodechalDebugInterface  *debugInterface)
    : Av1VdencPipelineXe_M_Base(hwInterface, debugInterface)
{

}

MOS_STATUS Av1VdencPipelineXe_Hpm::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(settings);

    ENCODE_CHK_STATUS_RETURN(Initialize(settings));

    MediaTask* task = CreateTask(MediaTask::TaskType::cmdTask);
    ENCODE_CHK_NULL_RETURN(task);

    EncodeCheckHucLoadPkt* checkHucLoadpkt = MOS_New(EncodeCheckHucLoadPkt, this, task, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(checkHucLoadpkt);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(EncodeCheckHucLoad, checkHucLoadpkt));
    ENCODE_CHK_STATUS_RETURN(checkHucLoadpkt->Init());

    Av1BrcInitPkt* brcInitpkt = MOS_New(Av1BrcInitPkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1HucBrcInit, brcInitpkt));
    ENCODE_CHK_STATUS_RETURN(brcInitpkt->Init());

    Av1BrcUpdatePkt* brcUpdatepkt = MOS_New(Av1BrcUpdatePkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1HucBrcUpdate, brcUpdatepkt));
    ENCODE_CHK_STATUS_RETURN(brcUpdatepkt->Init());

    Av1VdencPktXe_Hpm *av1Vdencpkt = MOS_New(Av1VdencPktXe_Hpm, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1VdencPacket, av1Vdencpkt));
    ENCODE_CHK_STATUS_RETURN(av1Vdencpkt->Init());

    if (m_dualEncEnable)
    {
        Av1PakIntegratePkt *av1PakIntPkt = MOS_New(Av1PakIntegratePkt, this, task, m_hwInterface);
        ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1PakIntegrate, av1PakIntPkt));
        ENCODE_CHK_STATUS_RETURN(av1PakIntPkt->Init());
    }

    Av1BackAnnotationPkt *av1BackAnnotationpkt = MOS_New(Av1BackAnnotationPkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1BackAnnotation, av1BackAnnotationpkt));
    ENCODE_CHK_STATUS_RETURN(av1BackAnnotationpkt->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_Hpm::CreateFeatureManager()
{
    ENCODE_FUNC_CALL();
    m_featureManager = MOS_New(EncodeAv1VdencFeatureManagerXe_Hpm, m_allocator, m_hwInterface, m_trackedBuf, m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe_Hpm::HuCCheckAndInit()
{
    ENCODE_FUNC_CALL();

    bool immediateSubmit = !m_singleTaskPhaseSupported;

    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    MEDIA_WA_TABLE *waTable = m_hwInterface->GetWaTable();
    if (waTable && MEDIA_IS_WA(waTable, WaCheckHucAuthenticationStatus))
    {
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(EncodeCheckHucLoad, immediateSubmit, 0, 0));
    }

    ENCODE_CHK_STATUS_RETURN(ActivatePacket(Av1HucBrcInit, immediateSubmit, 0, 0));

    return MOS_STATUS_SUCCESS;
}

}
