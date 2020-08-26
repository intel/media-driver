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
//!
//! \file     vp_render_cmd_packet.cpp
//! \brief    render packet which used in by mediapipline.
//! \details  render packet provide the structures and generate the cmd buffer which mediapipline will used.
//!
#include "vp_render_cmd_packet.h"
#include "vp_platform_interface.h"
#include "vp_pipeline_common.h"
#include "vp_render_kernel_obj.h"
namespace vp {
    VpRenderCmdPacket::VpRenderCmdPacket(MediaTask* task, PVP_MHWINTERFACE hwInterface, PVpAllocator& allocator, VPMediaMemComp* mmc, VpKernelSet* kernelSet) :
        CmdPacket(task),
        RenderCmdPacket(task, hwInterface->m_osInterface, hwInterface->m_renderHal),
        VpCmdPacket(task, hwInterface, allocator, mmc, VP_PIPELINE_PACKET_RENDER),
        m_filter(nullptr),
        m_firstFrame(true),
        m_kernelSet(kernelSet)
{
}
MOS_STATUS VpRenderCmdPacket::Prepare()
{
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_vpPlatformInterface);

    m_PacketId = VP_PIPELINE_PACKET_RENDER;
    VP_RENDER_CHK_STATUS_RETURN(RenderEngineSetup());

    VP_RENDER_CHK_STATUS_RETURN(KernelStateSetup());

    VP_RENDER_CHK_STATUS_RETURN(SetUpSurfaceState());

    VP_RENDER_CHK_STATUS_RETURN(SetUpCurbeState());

    VP_RENDER_CHK_STATUS_RETURN(LoadKernel());

    VP_RENDER_CHK_STATUS_RETURN(SetupMediaWalker());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderCmdPacket::SetVeboxUpdateParams(PVEBOX_UPDATE_PARAMS params)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderCmdPacket::PacketInit(
    VP_SURFACE* inputSurface, 
    VP_SURFACE* outputSurface, 
    VP_SURFACE* previousSurface, 
    std::map<SurfaceType, VP_SURFACE*>& internalSurfaces, 
    VP_EXECUTE_CAPS packetCaps)
{
    // will remodify when normal render path enabled
    VP_UNUSED(inputSurface);
    VP_UNUSED(outputSurface);
    VP_UNUSED(previousSurface);

    m_PacketCaps    = packetCaps;
    m_surfacesGroup = internalSurfaces;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderCmdPacket::KernelStateSetup()
{
    if (m_kernelId.empty())
    {
        VP_RENDER_ASSERTMESSAGE("No VP Kernel Processing needed");
    }

    VP_RENDER_CHK_NULL_RETURN(m_kernelSet);

    //m_kernelCount = IDR_VP_TOTAL_NUM_KERNELS;
    RENDER_KERNEL_PARAMS kernelParams;
    MOS_ZeroMemory(&kernelParams, sizeof(RENDER_KERNEL_PARAMS));
    kernelParams.kernelId      = &m_kernelId;
    kernelParams.surfacesGroup = &m_surfacesGroup;

    m_kernelSet->CreateKernelObjects(kernelParams);

    if (m_kernelSet->GetKernelObjs()       &&
        m_kernelSet->GetKernelObjs()->empty())
    {
        VP_RENDER_ASSERTMESSAGE("No VP Kernel Creation Fail");
    }

    // So Far only 1 Kernel executed. if adding two+ kernels, m_renderdata need to update
    for (auto it = m_kernelSet->GetKernelObjs()->begin(); it != m_kernelSet->GetKernelObjs()->end(); it ++)
    {
        int32_t iKUID = 0;                                         // Kernel Unique ID (DNDI uses combined kernels)
        int32_t kernelSize = 0;
        void* kernelBinary = nullptr;

        VpRenderKernelObj* kernel = it->second;

        // // Initialize States
        MOS_ZeroMemory(m_filter, sizeof(m_filter));
        MOS_ZeroMemory(&m_renderData.KernelEntry, sizeof(Kdll_CacheEntry));

        // Store pointer to Kernel Parameter
        m_renderData.KernelParam = m_hwInterface->m_vpPlatformInterface->GetVeboxKernelSettings(kernel->GetKernelID());
        VP_RENDER_CHK_STATUS_RETURN(kernel->GetKernelID(iKUID));
        VP_RENDER_CHK_STATUS_RETURN(m_kernelSet->GetKernelInfo(iKUID, kernelSize, kernelBinary));

        // Set Parameters for Kernel Entry
        m_renderData.KernelEntry.iKUID       = iKUID;
        m_renderData.KernelEntry.iKCID       = -1;
        m_renderData.KernelEntry.iFilterSize = 2;
        m_renderData.KernelEntry.pFilter     = m_filter;
        m_renderData.KernelEntry.iSize       = kernelSize;
        m_renderData.KernelEntry.pBinary     = (uint8_t*)kernelBinary;

        // set the Inline Data length
        void* InlineData = nullptr;
        uint32_t iInlineLength = 0;
        kernel->GetInlineState(&InlineData, iInlineLength);
        m_renderData.iInlineLength = iInlineLength;
    }

    return MOS_STATUS_SUCCESS;
}

VP_SURFACE* VpRenderCmdPacket::GetSurface(SurfaceType type)
{
    auto it = m_surfacesGroup.find(type);
    VP_SURFACE* surf = (m_surfacesGroup.end() != it) ? it->second : nullptr;

    return surf;
}
uint32_t VpRenderCmdPacket::GetSurfaceIndex(SurfaceType type)
{
    auto it = m_surfacesIndex.find(type);
    uint32_t index = (m_surfacesIndex.end() != it) ? it->second : 0;

    return index;
}
}
