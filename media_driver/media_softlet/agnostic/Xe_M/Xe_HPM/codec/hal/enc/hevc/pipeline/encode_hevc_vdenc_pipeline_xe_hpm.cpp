/*
* Copyright (c) 2019-2020, Intel Corporation
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
//! \file     encode_hevc_vdenc_pipeline_xe_hpm.cpp
//! \brief    Defines the interface for hevc vdenc encode pipeline Xe_HPM
//!
#include "encode_hevc_vdenc_pipeline_xe_hpm.h"
#include "encode_hevc_vdenc_packet.h"
#include "encode_huc_brc_init_packet.h"
#include "encode_huc_brc_update_packet.h"
#include "encode_pak_integrate_packet.h"
#include "encode_hevc_tile_replay_packet.h"
#include "codechal_debug.h"
#include "encode_huc_la_init_packet.h"
#include "encode_huc_la_update_packet.h"
#include "encode_hevc_vdenc_422_packet.h"
#include "encode_check_huc_load_packet.h"

#if _ENCODE_RESERVED
#include "encode_hevc_vdenc_packet_xe_hpm_ext.h"
#endif
namespace encode {

HevcVdencPipelineXe_Hpm::HevcVdencPipelineXe_Hpm(
    CodechalHwInterfaceNext     *hwInterface,
    CodechalDebugInterface  *debugInterface)
    : HevcVdencPipelineXe_Xpm_Base(hwInterface, debugInterface)
{

}

MOS_STATUS HevcVdencPipelineXe_Hpm::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(settings);

    ENCODE_CHK_STATUS_RETURN(Initialize(settings));

    MediaTask* task = CreateTask(MediaTask::TaskType::cmdTask);
    ENCODE_CHK_NULL_RETURN(task);

    RegisterPacket(HucLaInit, [=]() -> MediaPacket * { return MOS_New(HucLaInitPkt, this, task, m_hwInterface); });

    RegisterPacket(HucLaUpdate, [=]() -> MediaPacket * { return MOS_New(HucLaUpdatePkt, this, task, m_hwInterface); });

    RegisterPacket(HucBrcInit, [=]() -> MediaPacket * { return MOS_New(HucBrcInitPkt, this, task, m_hwInterface); });

    RegisterPacket(HucBrcUpdate, [=]() -> MediaPacket * { return MOS_New(HucBrcUpdatePkt, this, task, m_hwInterface); });

#ifdef _ENCODE_RESERVED
    RegisterPacket(hevcVdencPacket, [=]() -> MediaPacket * { return MOS_New(HevcVdencPktXeHpmExt, this, task, m_hwInterface); });
#else
    RegisterPacket(hevcVdencPacket, [=]() -> MediaPacket * { return MOS_New(HevcVdencPkt, this, task, m_hwInterface); });
#endif

    RegisterPacket(hevcPakIntegrate, [=]() -> MediaPacket * { return MOS_New(HevcPakIntegratePkt, this, task, m_hwInterface); });

    RegisterPacket(hevcVdencPicPacket, [=]() -> MediaPacket * {
        auto vdencPkt = dynamic_cast<HevcVdencPkt *>(GetOrCreate(hevcVdencPacket));
        return vdencPkt == nullptr ? nullptr : MOS_New(HevcVdencPicPacket, task, vdencPkt);
    });

    RegisterPacket(hevcVdencTileRowPacket, [=]() -> MediaPacket * {
        auto vdencPkt = dynamic_cast<HevcVdencPkt *>(GetOrCreate(hevcVdencPacket));
        return vdencPkt == nullptr ? nullptr : MOS_New(HevcVdencTileRowPkt, task, vdencPkt);
    });

    RegisterPacket(hevcVdencPacket422, [=]() -> MediaPacket * { return MOS_New(HevcVdencPkt422, this, task, m_hwInterface); });

    RegisterPacket(EncodeCheckHucLoad, [=]() -> MediaPacket * { return MOS_New(EncodeCheckHucLoadPkt, this, task, m_hwInterface); });

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe_Hpm::CreateFeatureManager()
{
    ENCODE_FUNC_CALL();
    m_featureManager = MOS_New(EncodeHevcVdencFeatureManagerXe_Hpm, m_allocator, m_hwInterface, m_trackedBuf, m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipelineXe_Hpm::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();

    CodechalSetting *codecSettings = (CodechalSetting *)settings;
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(m_hwInterface->Initialize(codecSettings));
    ENCODE_CHK_STATUS_RETURN(HevcVdencPipelineXe_Xpm_Base::Initialize(settings));

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

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS HevcVdencPipelineXe_Hpm::HuCCheckAndInit()
{
    ENCODE_FUNC_CALL();

    bool immediateSubmit = !m_singleTaskPhaseSupported;

    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    MEDIA_WA_TABLE *waTable = m_hwInterface->GetWaTable();
    if (waTable && MEDIA_IS_WA(waTable, WaCheckHucAuthenticationStatus))
    {
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(EncodeCheckHucLoad, immediateSubmit, 0, 0));
    }

    ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucBrcInit, immediateSubmit, 0, 0));

    return MOS_STATUS_SUCCESS;
}

}