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
//! \file     decode_sub_packet_manager.cpp
//! \brief    Defines the common interface for decode sub packet manager implementation
//! \details  The decocode sub packet manager interface is further sub-divided by different packet usages,
//!           this file is for the base interface which is shared by all decode packets.
//!

#include "decode_sub_packet_manager.h"
#include "decode_utils.h"

namespace decode
{

DecodeSubPacketManager::~DecodeSubPacketManager()
{
    for (auto subPacket : m_subPacketList)
    {
        MOS_Delete(subPacket.second);
    }

    m_subPacketList.clear();
}

MOS_STATUS DecodeSubPacketManager::Register(uint32_t packetId, DecodeSubPacket& subPacket)
{
    auto iter = m_subPacketList.find(packetId);
    DECODE_CHK_COND(iter != m_subPacketList.end(), "Failed to register sub packet %d", packetId);

    m_subPacketList.insert(std::make_pair(packetId, &subPacket));
    return MOS_STATUS_SUCCESS;
}

DecodeSubPacket* DecodeSubPacketManager::GetSubPacket(uint32_t packetId)
{
    auto iter = m_subPacketList.find(packetId);
    if (iter == m_subPacketList.end())
    {
        return nullptr;
    }
    return iter->second;
}

MOS_STATUS DecodeSubPacketManager::Init()
{
    for (auto subPacket : m_subPacketList)
    {
        subPacket.second->Init();
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeSubPacketManager::Prepare()
{
    for (auto subPacket : m_subPacketList)
    {
        DECODE_CHK_STATUS(subPacket.second->Prepare());
    }
    return MOS_STATUS_SUCCESS;
}

}
