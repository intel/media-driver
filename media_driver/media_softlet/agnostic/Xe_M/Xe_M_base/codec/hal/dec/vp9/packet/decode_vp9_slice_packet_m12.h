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
//! \file     decode_vp9_slice_packet_m12.h
//! \brief    Defines the implementation of vp9 decode slice packet for M12
//!

#ifndef __DECODE_VP9_SLICE_PACKET_M12_H__
#define __DECODE_VP9_SLICE_PACKET_M12_H__

#include "media_cmd_packet.h"
#include "decode_vp9_pipeline.h"
#include "decode_utils.h"
#include "decode_vp9_basic_feature.h"
#include "decode_vp9_slice_packet_xe_m_base.h"

namespace decode
{

class Vp9DecodeSlcPktM12 : public Vp9DecodeSlcPktXe_M_Base
{
public:
    Vp9DecodeSlcPktM12(Vp9Pipeline *pipeline, CodechalHwInterface *hwInterface)
        : Vp9DecodeSlcPktXe_M_Base(pipeline, hwInterface)
    {
    }
    virtual ~Vp9DecodeSlcPktM12();

    MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint32_t sliceIdx, uint32_t subTileIdx) override;

protected:
     

MEDIA_CLASS_DEFINE_END(decode__Vp9DecodeSlcPktM12)
};

}  // namespace decode
#endif
