/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     decode_av1_downsampling_packet_xe2_lpm_base.h
//! \brief    Defines the common interface for Av1 deocde down sampling sub packet
//! \details  The Av1 decode down sampling sub packet interface is unified interface
//!           for down sampling function, used by main packet on demand.
//!

#ifndef __DECODE_AV1_DOWNSAMPLING_PACKET_XE2_LPM_BASE_H__
#define __DECODE_AV1_DOWNSAMPLING_PACKET_XE2_LPM_BASE_H__

#include "decode_downsampling_packet.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{
class Av1DownSamplingPktXe2_Lpm_Base : public DecodeDownSamplingPkt
{
public:
    //!
    //! \brief  Decode down sampling sub packet constructor
    //!
    Av1DownSamplingPktXe2_Lpm_Base(DecodePipeline *pipeline, CodechalHwInterfaceNext *hwInterface);

    virtual ~Av1DownSamplingPktXe2_Lpm_Base() {}

    //!
    //! \brief  Initialize the Hevc down sampling sub packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

protected:
    virtual MOS_STATUS InitSfcParams(VDBOX_SFC_PARAMS &sfcParams) override;
    virtual MOS_STATUS SetSfcMode(MEDIA_SFC_INTERFACE_MODE &mode) override;

MEDIA_CLASS_DEFINE_END(decode__Av1DownSamplingPktXe2_Lpm_Base)
};

}  // namespace decode

#endif  // !_DECODE_PROCESSING_SUPPORTED
#endif  // !__DECODE_AV1_DOWNSAMPLING_PACKET_XE2_LPM_BASE_H__