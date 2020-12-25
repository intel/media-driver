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
#include "vp_render_kernel_obj.h"

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

    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase = otherPacket) override;

    MOS_STATUS SetVeboxUpdateParams(PVEBOX_UPDATE_PARAMS params);

    MOS_STATUS SetSecureCopyParams(bool copyNeeded);

    MOS_STATUS PacketInit(
        VP_SURFACE* inputSurface,
        VP_SURFACE* outputSurface,
        VP_SURFACE* previousSurface,
        VP_SURFACE_SETTING& surfSetting,
        VP_EXECUTE_CAPS packetCaps) override;

protected:
    MOS_STATUS KernelStateSetup(KernelID kernelExecuteID);

    virtual MOS_STATUS SetupSurfaceState();

    virtual MOS_STATUS SetupCurbeState();

    virtual VP_SURFACE* GetSurface(SurfaceType type);

    virtual MOS_STATUS SetupMediaWalker(VpRenderKernelObj * kernel);

    MOS_STATUS InitRenderHalSurface(
        VP_SURFACE         &surface,
        RENDERHAL_SURFACE  &renderSurface);

    MOS_STATUS InitStateHeapSurface(
        SurfaceType        type,
        RENDERHAL_SURFACE& renderSurface);

    // comments here: Hight overwite params if needed
    MOS_STATUS UpdateRenderSurface(RENDERHAL_SURFACE_NEXT &renderSurface, KERNEL_SURFACE2D_STATE_PARAM& kernelParams);

protected:

    KERNEL_OBJECTS                     m_kernelObjs;
    KERNEL_CONFIGS                     m_kernelConfigs;
    KERNEL_RENDER_DATA                 m_kernelRenderData;

    int32_t                            m_kernelIndex = 0;
    Kdll_FilterEntry                  *m_filter = nullptr;                                       // Kernel Filter (points to base of filter array)
    bool                               m_firstFrame = true;
    std::vector<uint32_t>              m_kernelId;
    VpKernelSet                       *m_kernelSet = nullptr;
    VpRenderKernelObj                 *m_kernel    = nullptr; // processing kernel pointer
};
}

#endif
