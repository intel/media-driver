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
//! \file     decode_sub_packet.h
//! \brief    Defines the common interface for decode sub packet
//! \details  The decocode sub packet interface is further sub-divided by different packet usages,
//!           this file is for the base interface which is shared by all decode packets.
//!

#ifndef __DECODE_SUB_PACKET_H__
#define __DECODE_SUB_PACKET_H__

#include "mos_defs.h"
#include "codechal_setting.h"
#include "codec_hw_next.h"
#include "codec_def_decode.h"
#include "media_feature_manager.h"
#include "decodecp_interface.h"

namespace decode
{

class DecodePipeline;

class DecodeSubPacket
{
public:
    //!
    //! \brief  Decode sub packet constructor
    //!
    DecodeSubPacket(DecodePipeline *pipeline, CodechalHwInterfaceNext *hwInterface);

    //!
    //! \brief  Decode sub packet destructor
    //!
    virtual ~DecodeSubPacket() {};

    //!
    //! \brief  Initialize the media packet, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() = 0;

    //!
    //! \brief  Prepare the parameters for command submission
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare() = 0;

    //! \brief  Calculate Command Size
    //!
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested size
    //! \return uint32_t
    //!         Command size calculated
    //!
    virtual MOS_STATUS CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize) = 0;

protected:
    DecodePipeline *        m_pipeline = nullptr;
    MediaFeatureManager *   m_featureManager = nullptr;
    CodechalHwInterfaceNext      *m_hwInterface    = nullptr;
    PMOS_INTERFACE          m_osInterface = nullptr;
    DecodeCpInterface *     m_decodecp = nullptr;
    std::shared_ptr<mhw::mi::Itf> m_miItf = nullptr;

MEDIA_CLASS_DEFINE_END(decode__DecodeSubPacket)
};

}

#endif  // !__DECODE_SUB_PACKET_H__
