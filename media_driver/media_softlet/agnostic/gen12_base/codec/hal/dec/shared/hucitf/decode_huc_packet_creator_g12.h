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
//! \file     decode_huc_packet_creator_g12.h
//!

#ifndef __CODECHAL_HUC_PACKET_CREATOR_G12_H__
#define __CODECHAL_HUC_PACKET_CREATOR_G12_H__

#include "decode_huc_packet_creator_base.h"
#include "codechal_hw.h"

namespace decode
{
class HucPacketCreatorG12 : public HucPacketCreatorBase
{
public:    

    HucPacketCreatorG12()
    {
    }

    virtual ~HucPacketCreatorG12() {}

    virtual HucCopyPktItf *CreateHucCopyPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface);
    virtual CmdPacket     *CreateProbUpdatePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface);

    virtual HucCopyPktItf *CreateStreamOutInterface(
        MediaPipeline       *pipeline,
        MediaTask           *task,
        CodechalHwInterface *hwInterface);
        
    using HucPacketCreatorBase::CreateHucCopyPkt;
    using HucPacketCreatorBase::CreateProbUpdatePkt;
    using HucPacketCreatorBase::CreateStreamOutInterface;
    MEDIA_CLASS_DEFINE_END(decode__HucPacketCreatorG12)
};

}  // namespace decode
#endif