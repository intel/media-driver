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
//! \file     encode_avc_vdenc_packet_xe2_hpm.h
//! \brief    Defines the interface to adapt to AVC VDENC pipeline
//!

#ifndef __CODECHAL_AVC_VDENC_PACKET_XE2_HPM_H__
#define __CODECHAL_AVC_VDENC_PACKET_XE2_HPM_H__

#include "encode_avc_vdenc_packet.h"
#include "mhw_vdbox_aqm_impl_xe2_hpm.h"
#include "mhw_vdbox_aqm_itf.h"

namespace encode
{

class AvcVdencPktXe2_Hpm : public AvcVdencPkt, public mhw::vdbox::aqm::Itf::ParSetting
    {
public:
    AvcVdencPktXe2_Hpm(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) : AvcVdencPkt(pipeline, task, hwInterface)
    {
        m_aqmItf = std::make_shared<mhw::vdbox::aqm::xe2_hpm::Impl>(m_osInterface);
    };

    virtual MOS_STATUS Completed(void *mfxStatus, void *rcsStatus, void *statusReport) override;
protected:
    virtual MOS_STATUS AddPictureVdencCommands(MOS_COMMAND_BUFFER &cmdBuffer);
    virtual MOS_STATUS SendSlice(PMOS_COMMAND_BUFFER cmdBuffer);
    MOS_STATUS EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer) override;

    MHW_SETPAR_DECL_HDR(VD_PIPELINE_FLUSH);

protected:
    std::shared_ptr<mhw::vdbox::aqm::Itf> m_aqmItf = nullptr;

MEDIA_CLASS_DEFINE_END(encode__AvcVdencPktXe2_Hpm)
};

}  // namespace encode

#endif  // !__CODECHAL_AVC_VDENC_PACKET_XE2_HPM_H__
