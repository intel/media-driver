/*
* Copyright (c) 2018, Intel Corporation
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
#ifndef __VP_PACKET_PIPE_H__
#define __VP_PACKET_PIPE_H__

#include "media_cmd_packet.h"
#include "mhw_sfc.h"
#include "vp_pipeline_common.h"
#include "vp_allocator.h"

#include <vector>
#include "hw_filter.h"

class MediaScalability;
class MediaContext;

namespace vp {

class PacketFactory
{
public:
    PacketFactory();
    virtual ~PacketFactory();
    MOS_STATUS Initialize(MediaTask *pTask, PVP_MHWINTERFACE pHwInterface, PVpAllocator pAllocator, VPMediaMemComp *pMmc);
    VpCmdPacket *CreatePacket(EngineType type);
    void ReturnPacket(VpCmdPacket *&pPacket);

protected:
    virtual VpCmdPacket *CreateVeboxPacket() = 0;
    virtual VpCmdPacket *CreateRenderPacket() = 0;

    void ClearPacketPool(std::vector<VpCmdPacket *> &pool);

    std::vector<VpCmdPacket *> m_VeboxPacketPool;
    std::vector<VpCmdPacket *> m_RenderPacketPool;

    MediaTask           *m_pTask = nullptr;
    PVP_MHWINTERFACE    m_pHwInterface = nullptr;
    PVpAllocator        m_pAllocator = nullptr;
    VPMediaMemComp      *m_pMmc = nullptr;
};

class PacketPipe
{
public:
    PacketPipe(PacketFactory &packetFactory);
    virtual ~PacketPipe();
    MOS_STATUS Clean();
    MOS_STATUS AddPacket(HwFilter &hwFilter);
    MOS_STATUS SwitchContext(PacketType type, MediaScalability *&scalability, MediaContext *mediaContext, bool bEnableVirtualEngine, uint8_t numVebox);
    MOS_STATUS Execute(MediaStatusReport *statusReport, MediaScalability *&scalability, MediaContext *mediaContext, bool bEnableVirtualEngine, uint8_t numVebox);

private:
    VpCmdPacket *CreatePacket(EngineType type);

    PacketFactory &m_PacketFactory;
    std::vector<VpCmdPacket *> m_Pipe;
};

class PacketPipeFactory
{
public:
    PacketPipeFactory(PacketFactory &pPacketFactory);
    virtual ~PacketPipeFactory();
    PacketPipe *CreatePacketPipe();
    void ReturnPacketPipe(PacketPipe *&pPipe);

private:
    PacketFactory &m_pPacketFactory;
    std::vector<PacketPipe *> m_Pool;
};

}
#endif // !__VP_PACKET_PIPE_H__
