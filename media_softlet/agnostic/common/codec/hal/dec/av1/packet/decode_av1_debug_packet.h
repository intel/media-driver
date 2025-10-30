/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     decode_av1_debug_packet.h
//! \brief    Defines the common interface for AV1 decode debug sub packet
//! \details  The AV1 decode debug sub packet interface provides debug functionality
//!           for AV1 decode operations, including AVP debug data collection and CRC verification.
//!

#ifndef __DECODE_AV1_DEBUG_PACKET_H__
#define __DECODE_AV1_DEBUG_PACKET_H__

#if (_DEBUG || _RELEASE_INTERNAL)

#include "decode_sub_packet.h"
#include "codechal_setting.h"
#include "decode_pipeline.h"

namespace decode
{

class Av1DecodeDebugPkt : public DecodeSubPacket
{
public:
    //!
    //! \brief  AV1 decode debug sub packet constructor
    //!
    Av1DecodeDebugPkt(DecodePipeline *pipeline, CodechalHwInterfaceNext *hwInterface);

    //!
    //! \brief  AV1 decode debug sub packet destructor
    //!
    virtual ~Av1DecodeDebugPkt() {};

    //!
    //! \brief  Initialize the debug sub packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

    //!
    //! \brief  Prepare the parameters for debug sub packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare() override;

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
    virtual MOS_STATUS CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize) override;

    //!
    //! \brief  Destroy the debug sub packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Destroy();

    //!
    //! \brief  Execute debug sub packet
    //! \param  [in] cmdBuffer
    //!         Command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer);

    //!
    //! \brief  Complete debug operations after decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Completed();

protected:
    std::shared_ptr<mhw::mi::Itf> m_miItf = nullptr;

MEDIA_CLASS_DEFINE_END(decode__Av1DecodeDebugPkt)
};

}

#endif  // _DEBUG || _RELEASE_INTERNAL

#endif  // !__DECODE_AV1_DEBUG_PACKET_H__