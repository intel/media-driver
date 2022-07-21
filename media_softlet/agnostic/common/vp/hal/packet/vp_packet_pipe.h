/*
* Copyright (c) 2018-2021, Intel Corporation
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
#include "vp_debug_interface.h"

#include <vector>
#include "hw_filter.h"

#include "vp_packet_shared_context.h"
//#include "vp_kernelset.h"

class MediaScalability;
class MediaContext;
class VpDebugInterface;

namespace vp {

class VpPlatformInterface;
class VpKernelSet;

class PacketFactory
{
public:
    PacketFactory(VpPlatformInterface *vpPlatformInterface);
    virtual ~PacketFactory();
    MOS_STATUS   Initialize(MediaTask *pTask, PVP_MHWINTERFACE pHwInterface, PVpAllocator pAllocator, VPMediaMemComp *pMmc, VP_PACKET_SHARED_CONTEXT *packetSharedContext, VpKernelSet *vpKernels, void *debugInterface);
    VpCmdPacket *CreatePacket(EngineType type);
    void ReturnPacket(VpCmdPacket *&pPacket);

    VpDebugInterface       *m_debugInterface = nullptr;

protected:
    VpCmdPacket *CreateVeboxPacket();
    VpCmdPacket *CreateRenderPacket();

    void ClearPacketPool(std::vector<VpCmdPacket *> &pool);

    std::vector<VpCmdPacket *> m_VeboxPacketPool;
    std::vector<VpCmdPacket *> m_RenderPacketPool;

    MediaTask           *m_pTask = nullptr;
    PVP_MHWINTERFACE    m_pHwInterface = nullptr;
    PVpAllocator        m_pAllocator = nullptr;
    VPMediaMemComp      *m_pMmc = nullptr;
    VpPlatformInterface *m_vpPlatformInterface = nullptr;
    VpKernelSet         *m_kernelSet = nullptr;
    VP_PACKET_SHARED_CONTEXT *m_packetSharedContext = nullptr;

MEDIA_CLASS_DEFINE_END(vp__PacketFactory)
};

class PacketPipe
{
public:
    PacketPipe(PacketFactory &packetFactory);
    virtual ~PacketPipe();
    MOS_STATUS Clean();
    MOS_STATUS AddPacket(HwFilter &hwFilter);
    MOS_STATUS Execute(MediaStatusReport *statusReport, MediaScalability *&scalability, MediaContext *mediaContext, bool bEnableVirtualEngine, uint8_t numVebox);
    VPHAL_OUTPUT_PIPE_MODE GetOutputPipeMode()
    {
        return m_outputPipeMode;
    }

    bool IsVeboxFeatureInuse()
    {
        return m_veboxFeatureInuse;
    }

    uint32_t PacketNum()
    {
        return m_Pipe.size();
    }

    VpCmdPacket *GetPacket(uint32_t idx)
    {
        return idx < m_Pipe.size() ? m_Pipe[idx] : nullptr;
    }

    static MOS_STATUS SwitchContext(PacketType type, MediaScalability *&scalability, MediaContext *mediaContext, bool bEnableVirtualEngine, uint8_t numVebox);

private:
    VpCmdPacket *CreatePacket(EngineType type);
    MOS_STATUS SetOutputPipeMode(EngineType engineType);

    PacketFactory &m_PacketFactory;
    std::vector<VpCmdPacket *> m_Pipe;
    VPHAL_OUTPUT_PIPE_MODE m_outputPipeMode = VPHAL_OUTPUT_PIPE_MODE_INVALID;
    bool m_veboxFeatureInuse = false;

MEDIA_CLASS_DEFINE_END(vp__PacketPipe)
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

MEDIA_CLASS_DEFINE_END(vp__PacketPipeFactory)
};

}
#endif // !__VP_PACKET_PIPE_H__
