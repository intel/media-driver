/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     decode_av1_tile_packet_xe3_lpm_base.h
//! \brief    Defines the implementation of av1 decode tile packet for Xe3_LPM+
//!

#ifndef __DECODE_AV1_TILE_PACKET_XE3_LPM_BASE_H__
#define __DECODE_AV1_TILE_PACKET_XE3_LPM_BASE_H__

#include "media_cmd_packet.h"
#include "decode_av1_pipeline.h"
#include "decode_utils.h"
#include "decode_av1_basic_feature.h"
#include "decode_av1_tile_packet.h"

namespace decode
{
class Av1DecodeTilePktXe3_Lpm_Base : public Av1DecodeTilePkt
{
public:
    Av1DecodeTilePktXe3_Lpm_Base(Av1Pipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
        : Av1DecodeTilePkt(pipeline, hwInterface) {
        if (m_hwInterface != nullptr)
        {
            m_avpItf   = std::static_pointer_cast<mhw::vdbox::avp::Itf>(m_hwInterface->GetAvpInterfaceNext());
            m_miItf    = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
        }
    }
    virtual ~Av1DecodeTilePktXe3_Lpm_Base() {}

    MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer, int16_t tileIdx) override;
    MOS_STATUS CalculateTileStateCommandSize() override;
    virtual MOS_STATUS GetAvpPrimitiveCommandSize(
        uint32_t  mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize);

    virtual MOS_STATUS GetAvpPrimitiveCmdSize(uint32_t *commandsSize, 
        uint32_t *patchListSize, 
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params);

MEDIA_CLASS_DEFINE_END(decode__Av1DecodeTilePktXe3_Lpm_Base)
};

}  // namespace decode
#endif  // !__DECODE_AV1_TILE_PACKET_XE3_LPM_BASE_H__
