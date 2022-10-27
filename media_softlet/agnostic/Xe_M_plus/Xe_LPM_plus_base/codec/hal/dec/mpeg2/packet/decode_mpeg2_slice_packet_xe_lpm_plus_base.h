/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_mpeg2_slice_packet_xe_lpm_plus_base.h
//! \brief    Defines the implementation of mpeg2 decode slice packet for MXe_LPM_plus+13
//!

#ifndef __DECODE_MPEG2_SLICE_PACKET_XE_LPM_PLUS_BASE_H__
#define __DECODE_MPEG2_SLICE_PACKET_XE_LPM_PLUS_BASE_H__

#include "media_cmd_packet.h"
#include "decode_mpeg2_pipeline.h"
#include "decode_utils.h"
#include "decode_mpeg2_basic_feature.h"
#include "decode_mpeg2_slice_packet.h"

namespace decode
{
class Mpeg2DecodeSlcPktXe_Lpm_Plus_Base : public Mpeg2DecodeSlcPkt
{
public:
    Mpeg2DecodeSlcPktXe_Lpm_Plus_Base(Mpeg2Pipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
        : Mpeg2DecodeSlcPkt(pipeline, hwInterface)
    {
    }
    virtual ~Mpeg2DecodeSlcPktXe_Lpm_Plus_Base(){};

    virtual MOS_STATUS Execute(MHW_BATCH_BUFFER &batchBuffer, uint16_t slcIdx) override;

MEDIA_CLASS_DEFINE_END(decode__Mpeg2DecodeSlcPktXe_Lpm_Plus_Base)
};
}  // namespace decode
#endif
