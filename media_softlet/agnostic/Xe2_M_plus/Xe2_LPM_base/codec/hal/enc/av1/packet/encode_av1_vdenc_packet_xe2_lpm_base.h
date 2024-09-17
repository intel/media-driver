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
//! \file     encode_av1_vdenc_packet_xe2_lpm_base.h
//! \brief    Defines the interface to adapt to avc vdenc encode pipeline Xe2_LPM+
//!

#ifndef __CODECHAL_AV1_VDENC_PACKET_XE2_LPM_BASE_H__
#define __CODECHAL_AV1_VDENC_PACKET_XE2_LPM_BASE_H__

#include "encode_av1_vdenc_packet.h"
#include "encode_utils.h"
#include "codec_hw_xe2_lpm_base.h"
#include "mhw_vdbox_vdenc_cmdpar.h"
#include "mhw_vdbox_avp_cmdpar.h"

namespace encode
{
class Av1VdencPktXe2_Lpm_Base : public Av1VdencPkt
{
public:
    Av1VdencPktXe2_Lpm_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) : Av1VdencPkt(pipeline, task, hwInterface)
    {
        auto hw = dynamic_cast<CodechalHwInterfaceXe2_Lpm_Base *>(m_hwInterface);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(hw);

        m_avpItf       = std::static_pointer_cast<mhw::vdbox::avp::Itf>(hw->GetAvpInterfaceNext());
    }

    MOS_STATUS Init() override;

protected:
    virtual MOS_STATUS AddOneTileCommands(
        MOS_COMMAND_BUFFER &cmdBuffer,
        uint32_t            tileRow,
        uint32_t            tileCol,
        uint32_t            tileRowPass = 0) override;

    MOS_STATUS EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer) override;

    virtual MOS_STATUS AllocateResources() override;

    MOS_STATUS RegisterPostCdef() override;

    uint16_t      m_tileColStartSb[64]                                     = {};       //!< tile column start SB
    uint16_t      m_tileRowStartSb[64]                                     = {};       //!< tile row start SB

MEDIA_CLASS_DEFINE_END(encode__Av1VdencPktXe2_Lpm_Base)
};
}  // namespace encode

#endif
