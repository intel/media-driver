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
//! \file     decode_sub_packet_manager.h
//! \brief    Defines the common interface for decode sub packet manager
//! \details  The decode sub packet manager interface is shared by decode pipelines.
//!

#ifndef __DECODE_SUB_PACKET_MANAGER_H__
#define __DECODE_SUB_PACKET_MANAGER_H__

#include "decode_sub_packet.h"

namespace decode
{

class DecodeSubPacketManager
{
public:
    //!
    //! \brief  Decode sub packet constructor
    //!
    DecodeSubPacketManager() {};

    //!
    //! \brief  Decode sub packet destructor
    //!
    virtual ~DecodeSubPacketManager();

    //!
    //! \brief  Register sub packet
    //! \param  [in] subPacket
    //!         Reference to the sub packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Register(uint32_t packetId, DecodeSubPacket& subPacket);

    //!
    //! \brief  Get sub packet
    //! \param  [in] packetId
    //!         sub packet id to get the sub packet
    //! \return DecodeSubPacket*
    //!         Pointer of the decode sub packet
    //!
    DecodeSubPacket* GetSubPacket(uint32_t packetId);

    //!
    //! \brief  Initialize the decode sub packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init();

    //!
    //! \brief  Update the parameters for decode sub packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Prepare();

protected:
    std::map<uint32_t, DecodeSubPacket *> m_subPacketList; //!< sub packet list

MEDIA_CLASS_DEFINE_END(decode__DecodeSubPacketManager)
};

}
#endif  // !__DECODE_SUB_PACKET_MANAGER_H__
