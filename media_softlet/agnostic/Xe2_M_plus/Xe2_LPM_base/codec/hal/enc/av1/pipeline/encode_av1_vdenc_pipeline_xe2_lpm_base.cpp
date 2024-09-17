/*
* Copyright (c) 2021 - 2023, Intel Corporation
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
//! \file     encode_av1_vdenc_pipeline_xe2_lpm_base.cpp
//! \brief    Defines the interface for av1 vdenc encode pipeline Xe2_LPM+
//!
#include "encode_av1_vdenc_pipeline_xe2_lpm_base.h"
#include "encode_utils.h"
#include "encode_av1_vdenc_packet_xe2_lpm_base.h"
#include "encode_av1_vdenc_feature_manager_xe2_lpm_base.h"
#include "encode_status_report_defs.h"
#include "encode_scalability_defs.h"
#include "codechal_debug.h"
#include "encode_mem_compression_xe_lpm_plus_base.h"
#include "encode_av1_superres.h"
#include "encode_av1_vdenc_preenc.h"
#include "encode_preenc_packet.h"

namespace encode
{
MOS_STATUS Av1VdencPipelineXe2_Lpm_Base::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(settings);

    ENCODE_CHK_STATUS_RETURN(Initialize(settings));

    MediaTask *task = CreateTask(MediaTask::TaskType::cmdTask);
    ENCODE_CHK_NULL_RETURN(task);

    auto*av1Vdencpkt = MOS_New(Av1VdencPktXe2_Lpm_Base, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1VdencPacket, av1Vdencpkt));
    ENCODE_CHK_STATUS_RETURN(av1Vdencpkt->Init());

    Av1BackAnnotationPkt *av1BackAnnotationpkt = MOS_New(Av1BackAnnotationPkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1BackAnnotation, av1BackAnnotationpkt));
    ENCODE_CHK_STATUS_RETURN(av1BackAnnotationpkt->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe2_Lpm_Base::Execute()
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

MOS_STATUS Av1VdencPipelineXe2_Lpm_Base::CreateFeatureManager()
{
    ENCODE_FUNC_CALL();
    m_featureManager = MOS_New(EncodeAv1VdencFeatureManagerXe2_Lpm_Base, m_allocator, m_hwInterface, m_trackedBuf, m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe2_Lpm_Base::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    
    m_mmcState = MOS_New(EncodeMemCompXe_Lpm_Plus_Base, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
