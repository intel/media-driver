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
//! \file     decode_huc_packet_creator.h
//!

#ifndef __CODECHAL_HUC_PACKET_CREATOR_H__
#define __CODECHAL_HUC_PACKET_CREATOR_H__

#include "decode_huc_packet_creator_base.h"

#define DECALRE_HUC_KERNEL(hucName, platform)                                                                                    \
    using hucName##platform##CreatorFunc = std::function<CmdPacket *(MediaPipeline *, MediaTask *, CodechalHwInterfaceNext *)>;  \
                                                                                                                                 \
public:                                                                                                                          \
    static bool Set##hucName##platform##CreatorFunc(hucName##platform##CreatorFunc creatorEntry, bool forceOverwrite)            \
    {                                                                                                                            \
        hucName##platform##CreatorFunc &entry = Get##hucName##platform##CreatorFunc();                                           \
        if (forceOverwrite || !entry)                                                                                            \
        {                                                                                                                        \
            entry = creatorEntry;                                                                                                \
        }                                                                                                                        \
        return true;                                                                                                             \
    }                                                                                                                            \
                                                                                                                                 \
    CmdPacket *Create##hucName##platform##Packet(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) \
    {                                                                                                                            \
        auto createFunc = Get##hucName##platform##CreatorFunc();                                                                 \
        return createFunc(pipeline, task, hwInterface);                                                                          \
    }                                                                                                                            \
                                                                                                                                 \
private:                                                                                                                         \
    static hucName##platform##CreatorFunc &Get##hucName##platform##CreatorFunc()                                                 \
    {                                                                                                                            \
        static hucName##platform##CreatorFunc m_##hucName##platform##HucPktCreatorFunc;                                          \
        return m_##hucName##platform##HucPktCreatorFunc;                                                                         \
    }

#define CREATE_HUC_PACKET(hucName, platform, pipeline, task, hwInterface) Create##hucName##platform##Packet(pipeline, task, hwInterface)

#define REGISTER_HUC_PACKET(hucName, platform, hucPacket, forceOverWrite)                                                                  \
    static CmdPacket *Create##hucName##platform##hucPacket(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) \
    {                                                                                                                                      \
        CmdPacket *hucName##platform##Pkt = MOS_New(hucPacket, pipeline, task, hwInterface);                                               \
        return hucName##platform##Pkt;                                                                                                     \
    }                                                                                                                                      \
    static bool volatile hucPacket##platform##packetCreatorFlag = HucPacketCreator::Set##hucName##platform##CreatorFunc(Create##hucName##platform##hucPacket, forceOverWrite);

namespace decode
{
class HucPacketCreator : public HucPacketCreatorBase
{
public:    

    HucPacketCreator()
    {
    }

    virtual ~HucPacketCreator() {}

    virtual HucCopyPktItf *CreateHucCopyPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) override;
    virtual CmdPacket *    CreateProbUpdatePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) override;

    virtual HucCopyPktItf *CreateStreamOutInterface(
        MediaPipeline       *pipeline,
        MediaTask           *task,
        CodechalHwInterfaceNext *hwInterface) override;
    
    DECALRE_HUC_KERNEL(VvcS2L, Xe2Lpm)
    DECALRE_HUC_KERNEL(VvcS2L, Xe3Lpm);

MEDIA_CLASS_DEFINE_END(decode__HucPacketCreator)
};

}  // namespace decode
#endif
