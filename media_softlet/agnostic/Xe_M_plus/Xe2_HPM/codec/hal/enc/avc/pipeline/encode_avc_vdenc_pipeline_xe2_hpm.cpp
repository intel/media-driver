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
//! \file     encode_avc_vdenc_pipeline_xe2_hpm.cpp
//! \brief    Defines the interface for avc vdenc encode pipeline Xe2_HPM
//!
#include "encode_avc_vdenc_pipeline_xe2_hpm.h"
#include "encode_avc_vdenc_feature_manager_xe2_hpm.h"

#include "encode_avc_huc_brc_init_packet.h"
#include "encode_avc_huc_brc_update_packet.h"

#include "encode_avc_vdenc_packet_xe2_hpm.h"
#include "encode_preenc_packet.h"
#include "encode_avc_vdenc_preenc.h"

namespace encode
{
AvcVdencPipelineXe2_Hpm::AvcVdencPipelineXe2_Hpm(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : AvcVdencPipelineXe_Lpm_Plus_Base(hwInterface, debugInterface)
{
}

MOS_STATUS AvcVdencPipelineXe2_Hpm::CreateFeatureManager()
{
    ENCODE_FUNC_CALL();
    m_featureManager = MOS_New(EncodeAvcVdencFeatureManagerXe2_Hpm, m_allocator, m_hwInterface, m_trackedBuf, m_recycleBuf, m_mediaCopyWrapper);
    ENCODE_CHK_NULL_RETURN(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPipelineXe2_Hpm::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(settings);

    ENCODE_CHK_STATUS_RETURN(Initialize(settings));

    MediaTask *task = CreateTask(MediaTask::TaskType::cmdTask);
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

    AvcHucBrcInitPkt *brcInitpkt = MOS_New(AvcHucBrcInitPkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucBrcInit, brcInitpkt));
    ENCODE_CHK_STATUS_RETURN(brcInitpkt->Init());

    AvcHucBrcUpdatePkt *brcUpdatepkt = MOS_New(AvcHucBrcUpdatePkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(HucBrcUpdate, brcUpdatepkt));
    ENCODE_CHK_STATUS_RETURN(brcUpdatepkt->Init());

    AvcVdencPktXe2_Hpm *avcVdencpkt = MOS_New(AvcVdencPktXe2_Hpm, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(VdencPacket, avcVdencpkt));
    ENCODE_CHK_STATUS_RETURN(avcVdencpkt->Init());

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
