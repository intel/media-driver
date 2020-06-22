/*
* Copyright (c) 2018-2020, Intel Corporation
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
#ifndef __VP_CMD_PACKET_H__
#define __VP_CMD_PACKET_H__

#include "media_cmd_packet.h"
#include "mhw_sfc.h"
#include "vp_pipeline_common.h"
#include "vp_allocator.h"
#include "vp_packet_shared_context.h"

namespace vp {

enum _PacketType
{
    VP_PIPELINE_PACKET_FF = 0,
    VP_PIPELINE_PACKET_COMP
};
using PacketType           = _PacketType;

class VpCmdPacket : public CmdPacket
{
public:
    VpCmdPacket(MediaTask *task, PVP_MHWINTERFACE hwInterface, PVpAllocator &allocator, VPMediaMemComp *mmc, PacketType packetId);
    virtual ~VpCmdPacket() {};

    void SetPacketSharedContext(VP_PACKET_SHARED_CONTEXT *context)
    {
        m_packetSharedContext = context;
    }

    // Need to remove vphal surface dependence from VpCmdPacket later.
    virtual MOS_STATUS PacketInit(
        VP_SURFACE                          *inputSurface,
        VP_SURFACE                          *outputSurface,
        VP_SURFACE                          *previousSurface,
        std::map<SurfaceType, VP_SURFACE*>  &internalSurfaces,
        VP_EXECUTE_CAPS                     packetCaps) = 0;

    virtual MOS_STATUS Prepare()
    {
        return MOS_STATUS_SUCCESS;
    };

    PacketType GetPacketId()
    {
        return m_PacketId;
    }

protected:
    virtual MOS_STATUS VpCmdPacketInit();
    bool IsOutputPipeVebox()
    {
        return m_PacketCaps.bVebox && !m_PacketCaps.bSFC && !m_PacketCaps.bRender;
    }

public:
    // HW intface to access MHW
    PVP_MHWINTERFACE    m_hwInterface = nullptr;
    VP_EXECUTE_CAPS     m_PacketCaps = {};
    PVpAllocator        &m_allocator;
    VPMediaMemComp      *m_mmc = nullptr;

protected:
    PacketType          m_PacketId = VP_PIPELINE_PACKET_FF;
    VP_PACKET_SHARED_CONTEXT *m_packetSharedContext = nullptr;
};
}
#endif // !__VP_CMD_PACKET_H__
