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
//! \file     encode_hevc_vdenc_packet_xe3p_lpm_base.h
//! \brief    Defines the interface to adapt to hevc vdenc encode pipeline Xe3p_LPM_Base
//!

#ifndef __ENCODE_HEVC_VDENC_PACKET_XE3P_LPM_BASE_H__
#define __ENCODE_HEVC_VDENC_PACKET_XE3P_LPM_BASE_H__

#include "encode_hevc_vdenc_packet.h"
#include "codec_hw_xe3p_lpm_base.h"
#include "encode_hevc_basic_feature_xe3p_lpm_base.h"

namespace encode
{
class HevcVdencPktXe3P_Lpm_Base : public HevcVdencPkt, public mhw::vdbox::aqm::Itf::ParSetting
{
public:
    HevcVdencPktXe3P_Lpm_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) : HevcVdencPkt(pipeline, task, hwInterface)
    {
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);

        m_aqmItf = m_hwInterface->GetAqmInterfaceNext();
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_aqmItf);
    }

    MOS_STATUS Completed(void *mfxStatus, void *rcsStatus, void *statusReport) override;
#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpResources(EncodeStatusMfx *encodeStatusMfx, EncodeStatusReportData *statusReportData);
#endif
protected:
    MOS_STATUS AddPicStateWithNoTile(MOS_COMMAND_BUFFER &cmdBuffer) override;
    MOS_STATUS AddPicStateWithTile(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS AddAQMCommands(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS AddOneTileCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t tileRow, uint32_t tileCol, uint32_t tileRowPass) override;
    MOS_STATUS SendHwSliceEncodeCommand(const PCODEC_ENCODER_SLCDATA slcData, const uint32_t currSlcIdx, MOS_COMMAND_BUFFER &cmdBuffer) override;
    MOS_STATUS EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer) override;

    MHW_SETPAR_DECL_HDR(VD_PIPELINE_FLUSH);

    std::shared_ptr<mhw::vdbox::aqm::Itf> m_aqmItf = nullptr;

MEDIA_CLASS_DEFINE_END(encode__HevcVdencPktXe3P_Lpm_Base)
};

}

#endif  //__ENCODE_HEVC_VDENC_PACKET_XE3P_LPM_BASE_H__
