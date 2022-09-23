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
//! \file     decode_vp9_buffer_update_m12.cpp
//! \brief    Defines the interface for vp9 buffer update
//! \details  Defines the interface to handle the vp9 buffer update for
//!           segment id buffer and probability buffer.
//!
#include "decode_vp9_buffer_update_m12.h"
#include "decode_huc_packet_creator_g12.h"

namespace decode
{
DecodeVp9BufferUpdateM12::DecodeVp9BufferUpdateM12(Vp9Pipeline *pipeline, MediaTask *task, uint8_t numVdbox, CodechalHwInterface *hwInterface)
    : DecodeVp9BufferUpdate(pipeline, task, numVdbox)
{
    m_hwInterface = hwInterface;
}

MOS_STATUS DecodeVp9BufferUpdateM12::Init(CodechalSetting &settings)
{
    DECODE_CHK_NULL(m_pipeline);

    CodechalHwInterface *hwInterface = m_hwInterface;
    DECODE_CHK_NULL(hwInterface);
    PMOS_INTERFACE osInterface = hwInterface->GetOsInterface();
    DECODE_CHK_NULL(osInterface);
    InitScalabilityPars(osInterface);

    m_allocator     = m_pipeline->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    MediaFeatureManager *featureManager = m_pipeline->GetFeatureManager();
    DECODE_CHK_NULL(featureManager);
    m_basicFeature = dynamic_cast<Vp9BasicFeature *>(featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

    HucPacketCreatorG12 *hucPktCreator = dynamic_cast<HucPacketCreatorG12 *>(m_pipeline);
    DECODE_CHK_NULL(hucPktCreator);
    m_sgementbufferResetPkt = hucPktCreator->CreateHucCopyPkt(m_pipeline, m_task, hwInterface);
    DECODE_CHK_NULL(m_sgementbufferResetPkt);
    MediaPacket *packet     = dynamic_cast<MediaPacket *>(m_sgementbufferResetPkt);
    DECODE_CHK_NULL(packet);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(m_pipeline, hucCopyPacketId), *packet));
    DECODE_CHK_STATUS(packet->Init());

    HucPacketCreatorG12 *probUpdateCreator = dynamic_cast<HucPacketCreatorG12 *>(m_pipeline);
    DECODE_CHK_NULL(probUpdateCreator);
    auto probUpdatePkt = probUpdateCreator->CreateProbUpdatePkt(m_pipeline, m_task, hwInterface);
    DECODE_CHK_NULL(probUpdatePkt);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, HucVp9ProbUpdatePktId), *probUpdatePkt));
    DECODE_CHK_STATUS(probUpdatePkt->Init());

    return MOS_STATUS_SUCCESS;
}
}  // namespace decode
