/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     encode_hevc_vdenc_pipeline_xe2_hpm.cpp
//! \brief    Defines the interface for hevc vdenc encode pipeline xe2 hpm
//!
#include "encode_hevc_vdenc_pipeline_xe2_hpm.h"
#include "encode_hevc_vdenc_feature_manager_xe2_hpm.h"
#include "encode_hevc_tile_replay_packet.h"
#include "encode_hevc_vdenc_packet_xe2_hpm.h"
#include "encode_huc_la_init_packet.h"
#include "encode_huc_la_update_packet.h"
#include "encode_huc_brc_init_packet.h"
#include "encode_huc_brc_update_packet.h"
#include "encode_pak_integrate_packet.h"
#include "encode_preenc_packet.h"

namespace encode
{
HevcVdencPipelineXe2_Hpm::HevcVdencPipelineXe2_Hpm(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : HevcVdencPipelineXe_Lpm_Plus_Base(hwInterface, debugInterface)
{
}

MOS_STATUS HevcVdencPipelineXe2_Hpm::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(settings);

    ENCODE_CHK_STATUS_RETURN(Initialize(settings));

    MediaTask *task = CreateTask(MediaTask::TaskType::cmdTask);
    ENCODE_CHK_NULL_RETURN(task);

    RUN_FEATURE_INTERFACE_RETURN(HevcVdencPreEnc, FeatureIDs::preEncFeature, IsEnabled, m_preEncEnabled);
    if (m_preEncEnabled)
    {
        EncodePreEncPacket *hevcPreEncpkt = MOS_New(EncodePreEncPacket, this, task, m_hwInterface);
        ENCODE_CHK_STATUS_RETURN(RegisterPacket(encodePreEncPacket, hevcPreEncpkt));
        ENCODE_CHK_STATUS_RETURN(hevcPreEncpkt->Init());

        RUN_FEATURE_INTERFACE_RETURN(HevcVdencPreEnc, HevcFeatureIDs::preEncFeature, GetEncodeMode, m_encodeMode);
        if (m_encodeMode == MediaEncodeMode::MANUAL_RES_PRE_ENC)
        {
            return MOS_STATUS_SUCCESS;
        }
    }

    HucLaInitPkt *laInitpkt = MOS_New(HucLaInitPkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucLaInit, laInitpkt));
    ENCODE_CHK_STATUS_RETURN(laInitpkt->Init());

    HucLaUpdatePkt *laUpdatepkt = MOS_New(HucLaUpdatePkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucLaUpdate, laUpdatepkt));
    ENCODE_CHK_STATUS_RETURN(laUpdatepkt->Init());

    HucBrcInitPkt *brcInitpkt = MOS_New(HucBrcInitPkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucBrcInit, brcInitpkt));
    ENCODE_CHK_STATUS_RETURN(brcInitpkt->Init());

    HucBrcUpdatePkt *brcUpdatepkt = MOS_New(HucBrcUpdatePkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucBrcUpdate, brcUpdatepkt));
    ENCODE_CHK_STATUS_RETURN(brcUpdatepkt->Init());

    HevcVdencPktXe2_Hpm *hevcVdencpkt = MOS_New(HevcVdencPktXe2_Hpm, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(hevcVdencPacket, hevcVdencpkt));
    ENCODE_CHK_STATUS_RETURN(hevcVdencpkt->Init());

    HevcPakIntegratePkt *pakIntPkt = MOS_New(HevcPakIntegratePkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(hevcPakIntegrate, pakIntPkt));
    ENCODE_CHK_STATUS_RETURN(pakIntPkt->Init());

    HevcVdencPicPacket *hevcVdencPicPkt = MOS_New(HevcVdencPicPacket, task, hevcVdencpkt);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(hevcVdencPicPacket, hevcVdencPicPkt));
    ENCODE_CHK_STATUS_RETURN(hevcVdencPicPkt->Init());

    /*
    HucBrcTileRowUpdatePkt *brcTileRowUpdatePkt = MOS_New(HucBrcTileRowUpdatePkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucBrcTileRowUpdate, brcTileRowUpdatePkt));
    ENCODE_CHK_STATUS_RETURN(brcTileRowUpdatePkt->Init());
    */

    HevcVdencTileRowPkt *hevcVdencTileRowPkt = MOS_New(HevcVdencTileRowPkt, task, hevcVdencpkt);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(hevcVdencTileRowPacket, hevcVdencTileRowPkt));
    ENCODE_CHK_STATUS_RETURN(hevcVdencTileRowPkt->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe2_Hpm::CreateFeatureManager()
{
    ENCODE_FUNC_CALL();
    m_featureManager = MOS_New(EncodeHevcVdencFeatureManagerXe2_Hpm, m_allocator, m_hwInterface, m_trackedBuf, m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
