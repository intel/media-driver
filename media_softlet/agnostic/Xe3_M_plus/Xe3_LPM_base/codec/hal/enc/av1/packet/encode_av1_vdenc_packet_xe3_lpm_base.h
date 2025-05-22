/*
* Copyright (c) 2022 - 2023, Intel Corporation
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
//! \file     encode_av1_vdenc_packet_xe3_lpm_base.h
//! \brief    Defines the interface to adapt to avc vdenc encode pipeline Xe3_LPM+
//!

#ifndef __CODECHAL_AV1_VDENC_PACKET_XE3_LPM_BASE_H__
#define __CODECHAL_AV1_VDENC_PACKET_XE3_LPM_BASE_H__

#include "encode_av1_vdenc_packet.h"
#include "encode_utils.h"
#include "codec_hw_xe3_lpm_base.h"
#include "mhw_vdbox_vdenc_cmdpar.h"
#include "mhw_vdbox_avp_cmdpar.h"
#include "mhw_vdbox_aqm_impl_xe3_lpm.h"

namespace encode
{
class Av1VdencPktXe3_Lpm_Base : public Av1VdencPkt
{
public:
    Av1VdencPktXe3_Lpm_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) : Av1VdencPkt(pipeline, task, hwInterface)
    {
        auto hw = dynamic_cast<CodechalHwInterfaceXe3_Lpm_Base *>(m_hwInterface);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(hw);

        m_avpItf = std::static_pointer_cast<mhw::vdbox::avp::Itf>(hw->GetAvpInterfaceNext());
        m_aqmItf = std::make_shared<mhw::vdbox::aqm::xe3_lpm::Impl>(m_osInterface);
    }

    MOS_STATUS Init() override;

    MOS_STATUS Completed(void *mfxStatus, void *rcsStatus, void *statusReport) override;

protected:
    virtual MOS_STATUS AddOneTileCommands(
        MOS_COMMAND_BUFFER &cmdBuffer,
        uint32_t            tileRow,
        uint32_t            tileCol,
        uint32_t            tileRowPass = 0) override;

    MOS_STATUS EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer) override;

    virtual MOS_STATUS AllocateResources() override;

    MOS_STATUS RegisterPostCdef() override;

    MOS_STATUS AddAllCmds_AVP_SURFACE_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const override;

    virtual MOS_STATUS CalculateAqmCommandsSize();

    virtual MOS_STATUS AddAqmCommands(PMOS_COMMAND_BUFFER cmdBuffer);

    virtual MOS_STATUS GetAqmPrimitiveCommandsDataSize(uint32_t *commandsSize, uint32_t *patchListSize) const;

    MOS_STATUS EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer) override;

    uint16_t      m_tileColStartSb[64]                                     = {};       //!< tile column start SB
    uint16_t      m_tileRowStartSb[64]                                     = {};       //!< tile row start SB

    std::shared_ptr<mhw::vdbox::aqm::Itf> m_aqmItf = nullptr;

MEDIA_CLASS_DEFINE_END(encode__Av1VdencPktXe3_Lpm_Base)
};
}  // namespace encode

#endif
