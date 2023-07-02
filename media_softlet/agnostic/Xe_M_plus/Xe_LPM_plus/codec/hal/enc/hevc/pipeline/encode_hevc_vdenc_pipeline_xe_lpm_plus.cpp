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
//! \file     encode_hevc_vdenc_pipeline_xe_lpm_plus.cpp
//! \brief    Defines the interface for hevc vdenc encode pipeline Xe_Lpm_Plus
//!
#include "encode_hevc_vdenc_pipeline_xe_lpm_plus.h"
#include "encode_hevc_vdenc_feature_manager_xe_lpm_plus.h"
#include "encode_hevc_vdenc_422_packet.h"

namespace encode {

HevcVdencPipelineXe_Lpm_Plus::HevcVdencPipelineXe_Lpm_Plus(
    CodechalHwInterfaceNext     *hwInterface,
    CodechalDebugInterface  *debugInterface)
    : HevcVdencPipelineXe_Lpm_Plus_Base(hwInterface, debugInterface)
{

}

MOS_STATUS HevcVdencPipelineXe_Lpm_Plus::CreateFeatureManager()
{
    ENCODE_FUNC_CALL();
    m_featureManager = MOS_New(EncodeHevcVdencFeatureManagerXe_Lpm_Plus, m_allocator, m_hwInterface, m_trackedBuf, m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe_Lpm_Plus::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(HevcVdencPipelineXe_Lpm_Plus_Base::Init(settings));

    MediaTask* task = GetTask(MediaTask::TaskType::cmdTask);
    ENCODE_CHK_NULL_RETURN(task);

    HevcVdencPkt422* hevcVdencpkt422 = MOS_New(HevcVdencPkt422, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(hevcVdencPacket422, hevcVdencpkt422));
    ENCODE_CHK_STATUS_RETURN(hevcVdencpkt422->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe_Lpm_Plus::ActivateVdencVideoPackets()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(HevcVdencPipelineXe_Lpm_Plus_Base::ActivateVdencVideoPackets());

    auto basicFeature = dynamic_cast<HevcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);
    if (basicFeature->m_422State && basicFeature->m_422State->GetFeature422Flag())
    {
        m_activePacketList.front().frameTrackingRequested = false;
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(hevcVdencPacket422, true, 0, 0));
    }

    SetFrameTrackingForMultiTaskPhase();
    return MOS_STATUS_SUCCESS;
}

}

