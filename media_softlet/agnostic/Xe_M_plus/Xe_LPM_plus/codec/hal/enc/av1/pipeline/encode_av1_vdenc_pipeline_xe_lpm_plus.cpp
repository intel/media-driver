/*
* Copyright (c) 2020 - 2023, Intel Corporation
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
#if _MEDIA_RESERVED
#include "encode_av1_vdenc_packet_xe_lpm_plus_ext.h"
#endif // _MEDIA_RESERVED
#include "encode_av1_vdenc_packet_xe_lpm_plus.h"
#include "encode_av1_brc_init_packet.h"
#include "encode_av1_brc_update_packet.h"
#include "encode_av1_pak_integrate_packet.h"
#include "codechal_debug.h"
#include "encode_av1_vdenc_feature_manager_xe_lpm_plus_base.h"
#include "encode_preenc_packet.h"
#include "encode_av1_superres.h"

namespace encode
{
MOS_STATUS Av1VdencPipelineXe_LPM_Plus::Init(void *settings)
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

    Av1BrcInitPkt *brcInitpkt = MOS_New(Av1BrcInitPkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1HucBrcInit, brcInitpkt));
    ENCODE_CHK_STATUS_RETURN(brcInitpkt->Init());

    Av1BrcUpdatePkt *brcUpdatepkt = MOS_New(Av1BrcUpdatePkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1HucBrcUpdate, brcUpdatepkt));
    ENCODE_CHK_STATUS_RETURN(brcUpdatepkt->Init());

#if _MEDIA_RESERVED
    auto av1Vdencpkt = MOS_New(Av1VdencPktXe_Lpm_PlusExt, this, task, m_hwInterface);
#else
    auto av1Vdencpkt = MOS_New(Av1VdencPktXe_Lpm_Plus, this, task, m_hwInterface);
#endif  // !(_MEDIA_RESERVED)
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1VdencPacket, av1Vdencpkt));
    ENCODE_CHK_STATUS_RETURN(av1Vdencpkt->Init());

    if (m_dualEncEnable)
    {
        Av1PakIntegratePkt* av1PakIntPkt = MOS_New(Av1PakIntegratePkt, this, task, m_hwInterface);
        ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1PakIntegrate, av1PakIntPkt));
        ENCODE_CHK_STATUS_RETURN(av1PakIntPkt->Init());
    }

    auto av1BackAnnotationpkt = MOS_New(Av1BackAnnotationPkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(Av1BackAnnotation, av1BackAnnotationpkt));
    ENCODE_CHK_STATUS_RETURN(av1BackAnnotationpkt->Init());

    m_sfcItf = m_hwInterface->GetMediaSfcInterface();
    ENCODE_CHK_NULL_RETURN(m_sfcItf);

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
