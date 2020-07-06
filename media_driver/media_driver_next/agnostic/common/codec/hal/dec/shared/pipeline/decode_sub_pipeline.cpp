/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     decode_sub_pipeline.cpp
//! \brief    Defines the common interface for decode sub pipeline
//! \details  The decode sub pipeline interface is further sub-divided by decode standard,
//!           this file is for the base interface which is shared by all decoders.
//!
#include "decode_sub_pipeline.h"
#include "decode_pipeline.h"
#include "decode_utils.h"
#include "media_packet.h"

namespace decode {

DecodeSubPipeline::DecodeSubPipeline(DecodePipeline* pipeline, MediaTask* task, uint8_t numVdbox)
    : m_pipeline(pipeline), m_task(task), m_numVdbox(numVdbox)
{
    DECODE_ASSERT(m_pipeline != nullptr);
    DECODE_ASSERT(m_task != nullptr);
}

DecodeSubPipeline::~DecodeSubPipeline()
{
    for (auto iter : m_packetList)
    {
        MOS_Delete(iter.second);
    }

    m_packetList.clear();
    m_activePacketList.clear();
}

DecodeSubPipeline::PacketListType & DecodeSubPipeline::GetPacketList()
{
    return m_packetList;
}

DecodeSubPipeline::ActivePacketListType & DecodeSubPipeline::GetActivePackets()
{
    return m_activePacketList;
}

DecodeScalabilityPars& DecodeSubPipeline::GetScalabilityPars()
{
    return m_decodeScalabilityPars;
}

MOS_STATUS DecodeSubPipeline::RegisterPacket(uint32_t packetId, MediaPacket& packet)
{
    auto iter = m_packetList.find(packetId);
    if (iter == m_packetList.end())
    {
        m_packetList.insert(std::make_pair(packetId, &packet));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeSubPipeline::ActivatePacket(uint32_t packetId, bool immediateSubmit,
                                             uint8_t pass, uint8_t pipe, uint8_t pipeNum)
{
    auto iter = m_packetList.find(packetId);
    if (iter == m_packetList.end())
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    PacketProperty prop;
    prop.packetId                         = iter->first;
    prop.packet                           = iter->second;
    prop.immediateSubmit                  = immediateSubmit;

    prop.stateProperty.currentPass        = pass;
    prop.stateProperty.currentPipe        = pipe;
    prop.stateProperty.pipeIndexForSubmit = pipeNum;

    m_activePacketList.push_back(prop);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeSubPipeline::Reset()
{
    m_activePacketList.clear();

    return MOS_STATUS_SUCCESS;
}

}
