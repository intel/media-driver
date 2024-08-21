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
//! \file     decode_vp9_downsampling_packet_xe2_hpm.h
//! \brief    Defines the common interface for Hevc deocde down sampling sub packet
//! \details  The Vp9 decode down sampling sub packet interface is unified interface
//!           for down sampling function, used by main packet on demand.
//!

#ifndef __DECODE_VP9_DOWNSAMPLING_PACKET_XE2_HPM_H__
#define __DECODE_VP9_DOWNSAMPLING_PACKET_XE2_HPM_H__

#include "decode_vp9_downsampling_packet.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{

class Vp9DownSamplingPktXe2_Hpm : public Vp9DownSamplingPkt
{
public:
    //!
    //! \brief  Decode down sampling sub packet constructor
    //!
    Vp9DownSamplingPktXe2_Hpm(DecodePipeline *pipeline, CodechalHwInterfaceNext *hwInterface);

    virtual ~Vp9DownSamplingPktXe2_Hpm() {}

protected:
    virtual MOS_STATUS InitSfcParams(VDBOX_SFC_PARAMS &sfcParams) override;

MEDIA_CLASS_DEFINE_END(decode__Vp9DownSamplingPktXe2_Hpm)
};

}

#endif
#endif
