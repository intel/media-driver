/*
* Copyright (c) 2020, Intel Corporation
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
#ifndef __VP_RENDER_CMD_PACKET_EXT_H__
#define __VP_RENDER_CMD_PACKET_EXT_H__
#include "media_render_cmd_packet.h"
#include "vp_allocator.h"
#include "vp_cmd_packet.h"
#include "vp_kernelset.h"

namespace vp
{
//!
//! \brief Secure Block Copy kernel inline data size
//!
#define SECURE_BLOCK_COPY_KERNEL_INLINE_SIZE    (1 * sizeof(uint32_t))
//!
//! \brief Secure Block Copy kernel width
//!
#define SECURE_BLOCK_COPY_KERNEL_SURF_WIDTH     64

//!
//! \brief Secure Block Copy kernel block height
//!
#define SECURE_BLOCK_COPY_KERNEL_BLOCK_HEIGHT   24

class VpRenderCmdPacket : virtual public RenderCmdPacket, virtual public VpCmdPacket
{
public:
    VpRenderCmdPacket(MediaTask* task, PVP_MHWINTERFACE hwInterface, PVpAllocator& allocator, VPMediaMemComp* mmc, VpKernelSet* kernelSet);
    virtual ~VpRenderCmdPacket() 
    {
    };

    MOS_STATUS Prepare() override;

    MOS_STATUS Init() override
    {
        return RenderCmdPacket::Init();
    }

    MOS_STATUS Destroy() override
    {
        return RenderCmdPacket::Destroy();
    }

    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase = otherPacket) override
    {
        return RenderCmdPacket::Submit(commandBuffer, packetPhase);
    }

    MOS_STATUS PacketInit(
        VP_SURFACE* inputSurface,
        VP_SURFACE* outputSurface,
        VP_SURFACE* previousSurface,
        std::map<SurfaceType, VP_SURFACE*>& internalSurfaces,
        VP_EXECUTE_CAPS                     packetCaps) override;

protected:
    MOS_STATUS KernelStateSetup();

    virtual MOS_STATUS SetUpSurfaceState()
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS SetUpCurbeState()
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual VP_SURFACE* GetSurface(SurfaceType type);

    virtual uint32_t GetSurfaceIndex(SurfaceType type);

    virtual MOS_STATUS SetupMediaWalker() override
    {
        return MOS_STATUS_SUCCESS;
    }

protected:
    int32_t                            m_kernelIndex = 0;
    Kdll_FilterEntry                  *m_filter = nullptr;                                       // Kernel Filter (points to base of filter array)
    bool                               m_firstFrame = true;
    std::map<SurfaceType, uint32_t>    m_surfacesIndex; // map <surfaceType, surfaceIndex>
    std::vector<KernelId>              m_kernelId;
    VpKernelSet                       *m_kernelSet = nullptr;;
};
}

#endif
