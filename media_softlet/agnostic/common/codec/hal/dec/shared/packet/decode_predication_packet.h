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
//! \file     decode_predication_packet.h
//! \brief    Defines the common interface for decode predication sub packet
//! \details  The decocode predication sub packet interface is further sub-divided by different 
//!           packet usages,this file is for the base interface which is shared by all decode packets.
//!

#ifndef __DECODE_PREDICATION_PACKET_H__
#define __DECODE_PREDICATION_PACKET_H__

#include "decode_sub_packet.h"
#include "codechal_setting.h"
#include "decode_pipeline.h"
#include "decode_predication.h"

namespace decode
{

class DecodePredicationPkt : public DecodeSubPacket
{
public:
    //!
    //! \brief  Decode predication sub packet constructor
    //!
    DecodePredicationPkt(DecodePipeline *pipeline, CodechalHwInterfaceNext *hwInterface);

    //!
    //! \brief  Decode predication sub packet destructor
    //!
    virtual ~DecodePredicationPkt() {};

    //!
    //! \brief  Initialize the predication sub packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

    //!
    //! \brief  Update the parameters for predication sub packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare() override;

    //!
    //! \brief  Execute sub packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer);

    //!
    //! \brief  Calculate Command Size
    //!
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested size
    //! \return MOS_STATUS
    //!         status
    //!
    MOS_STATUS CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize) override;

protected:
    std::shared_ptr<mhw::mi::Itf> m_miItf       = nullptr;
    DecodePredication* m_predication = nullptr;

MEDIA_CLASS_DEFINE_END(decode__DecodePredicationPkt)
};

}

#endif  // !__DECODE_PREDICATION_PACKET_H__
