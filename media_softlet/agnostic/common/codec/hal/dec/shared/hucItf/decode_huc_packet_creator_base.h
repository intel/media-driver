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
//! \file     decode_huc_packet_creator_base.h
//!

#ifndef __CODECHAL_HUC_PACKET_CREATOR_BASE_H__
#define __CODECHAL_HUC_PACKET_CREATOR_BASE_H__

#include "decode_huc_copy_packet_itf.h"
#include "media_cmd_packet.h"
#include "media_pipeline.h"
#include "codec_hw_next.h"

namespace decode
{
class HucPacketCreatorBase
{
public:    

    HucPacketCreatorBase()
    {
    }

    virtual ~HucPacketCreatorBase() {}

    virtual HucCopyPktItf *CreateHucCopyPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
    {
        return nullptr;
    }
    virtual CmdPacket *CreateProbUpdatePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
    {
        return nullptr;
    }

    virtual HucCopyPktItf *CreateStreamOutInterface(
        MediaPipeline       *pipeline,
        MediaTask           *task,
        CodechalHwInterfaceNext *hwInterface)
    {
        return nullptr;
    }

MEDIA_CLASS_DEFINE_END(decode__HucPacketCreatorBase)
};

}  // namespace decode
#endif