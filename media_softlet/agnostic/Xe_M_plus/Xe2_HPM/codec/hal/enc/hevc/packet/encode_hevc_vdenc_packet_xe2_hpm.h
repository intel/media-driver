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
//! \file     encode_hevc_vdenc_packet_xe2_hpm.h
//! \brief    Defines the interface to adapt to hevc vdenc encode pipeline Xe2 HPM
//!

#ifndef __CODECHAL_HEVC_VDENC_PACKET_XE2_HPM_H__
#define __CODECHAL_HEVC_VDENC_PACKET_XE2_HPM_H__

#include "encode_hevc_vdenc_packet.h"
#include "mhw_vdbox_aqm_impl_xe2_hpm.h"

namespace encode
{
class HevcVdencPktXe2_Hpm : public HevcVdencPkt, public mhw::vdbox::aqm::Itf::ParSetting
{
public:
    HevcVdencPktXe2_Hpm(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) : HevcVdencPkt(pipeline, task, hwInterface)
    {
        m_aqmItf = std::make_shared<mhw::vdbox::aqm::xe2_hpm::Impl>(m_osInterface);
    }

    MOS_STATUS Completed(void *mfxStatus, void *rcsStatus, void *statusReport) override;

protected:
    MOS_STATUS AddPicStateWithNoTile(MOS_COMMAND_BUFFER &cmdBuffer) override;
    MOS_STATUS AddAQMCommands(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS AddOneTileCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t tileRow, uint32_t tileCol, uint32_t tileRowPass) override;
    MOS_STATUS SendHwSliceEncodeCommand(const PCODEC_ENCODER_SLCDATA slcData, const uint32_t currSlcIdx, MOS_COMMAND_BUFFER &cmdBuffer) override;
    MOS_STATUS EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer) override;

    MHW_SETPAR_DECL_HDR(VD_PIPELINE_FLUSH);

    std::shared_ptr<mhw::vdbox::aqm::Itf> m_aqmItf = nullptr;

MEDIA_CLASS_DEFINE_END(encode__HevcVdencPktXe2_Hpm)
};

}

#endif  //__CODECHAL_HEVC_VDENC_PACKET_XE2_HPM_H__