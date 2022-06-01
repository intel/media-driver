/*
* Copyright (c) 2019-2020, Intel Corporation
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
//! \file     decode_av1_tile_packet_g12.h
//! \brief    Defines the implementation of av1 decode tile packet for GEN12
//!

#ifndef __DECODE_AV1_TILE_PACKET_G12_H__
#define __DECODE_AV1_TILE_PACKET_G12_H__

#include "media_cmd_packet.h"
#include "decode_av1_pipeline_g12_base.h"
#include "decode_utils.h"
#include "decode_av1_basic_feature_g12.h"
#include "decode_av1_tile_packet_g12_base.h"

namespace decode
{

class Av1DecodeTilePktG12 : public Av1DecodeTilePkt_G12_Base
{
public:
    Av1DecodeTilePktG12(Av1PipelineG12_Base *pipeline, CodechalHwInterface *hwInterface)
        : Av1DecodeTilePkt_G12_Base(pipeline, hwInterface)
    {
    }
    virtual ~Av1DecodeTilePktG12() {};

    MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer, int16_t tileIdx) override;

MEDIA_CLASS_DEFINE_END(decode__Av1DecodeTilePktG12)
};

}  // namespace decode
#endif
