/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     decode_vvc_pipeline_xe3_lpm_base.cpp
//! \brief    Defines the interface for vvc decode pipeline
//!
#include "decode_vvc_pipeline_xe3_lpm_base.h"
#include "decode_vvc_packet.h"
#include "decode_vvc_s2l_packet_register_xe3_lpm_base.h"

namespace decode
{

VvcPipelineXe3_Lpm_Base::VvcPipelineXe3_Lpm_Base(
    CodechalHwInterfaceNext *hwInterface,
    CodechalDebugInterface  *debugInterface)
    : VvcPipeline(hwInterface, debugInterface)
{
    m_hwInterface = hwInterface;
}

MOS_STATUS VvcPipelineXe3_Lpm_Base::Init(void *settings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(settings);
    DECODE_CHK_STATUS(Initialize(settings));

    if (MEDIA_IS_SKU(m_skuTable, FtrWithSlimVdbox))
    {
        m_numVdbox = 1;
    }
    if (m_basicFeature->m_shortFormatInUse)
    {
        HucPacketCreator *hucPktCreator = dynamic_cast<HucPacketCreator *>(this);
        DECODE_CHK_NULL(hucPktCreator);
        m_vvcDecodeS2LPkt = hucPktCreator->CREATE_HUC_PACKET(VvcS2L, Xe3Lpm, this, m_task, m_hwInterface);
        DECODE_CHK_NULL(m_vvcDecodeS2LPkt);
        DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, vvcDecodeS2LPktId), m_vvcDecodeS2LPkt));
        DECODE_CHK_STATUS(m_vvcDecodeS2LPkt->Init());
    }
    m_vvcDecodePkt = MOS_New(VvcDecodePkt, this, m_task, m_hwInterface);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, vvcDecodePacketId), m_vvcDecodePkt));
    DECODE_CHK_STATUS(m_vvcDecodePkt->Init());
    return MOS_STATUS_SUCCESS;
}
}  // namespace decode
