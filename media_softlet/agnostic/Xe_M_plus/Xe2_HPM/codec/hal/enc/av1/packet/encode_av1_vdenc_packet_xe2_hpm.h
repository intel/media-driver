/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     encode_av1_vdenc_packet_xe2_hpm.h
//! \brief    Defines the interface to av1 vdenc encode packet Xe2_HPM
//!

#ifndef _ENCODE_AV1_VDENC_PACKET_XE2_HPM_H__
#define _ENCODE_AV1_VDENC_PACKET_XE2_HPM_H__

#include "encode_av1_vdenc_packet_xe_lpm_plus_base.h"
#include "mhw_vdbox_aqm_impl_xe2_hpm.h"

namespace encode
{
class Av1VdencPktXe2_Hpm : public Av1VdencPktXe_Lpm_Plus_Base
{
public:
    Av1VdencPktXe2_Hpm(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) : Av1VdencPktXe_Lpm_Plus_Base(pipeline, task, hwInterface)
    {
        m_aqmItf = std::make_shared<mhw::vdbox::aqm::xe2_hpm::Impl>(m_osInterface);
    }

    virtual ~Av1VdencPktXe2_Hpm();

    MOS_STATUS Init() override;

    MOS_STATUS AddAllCmds_AVP_SURFACE_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const override;

    MOS_STATUS Completed(void *mfxStatus, void *rcsStatus, void *statusReport) override;

protected:
    MOS_STATUS AddOneTileCommands(
        MOS_COMMAND_BUFFER &cmdBuffer,
        uint32_t            tileRow,
        uint32_t            tileCol,
        uint32_t            tileRowPass = 0) override;

        std::shared_ptr<mhw::vdbox::aqm::Itf> m_aqmItf = nullptr;

    virtual MOS_STATUS CalculateAqmCommandsSize();

    virtual MOS_STATUS GetAqmPrimitiveCommandsDataSize(uint32_t *commandsSize, uint32_t *patchListSize) const;

    MOS_STATUS EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer) override;

MEDIA_CLASS_DEFINE_END(encode__Av1VdencPktXe2_Hpm)
};
}  // namespace encode
#endif  //_ENCODE_AV1_VDENC_PACKET_XE2_HPM_H__
