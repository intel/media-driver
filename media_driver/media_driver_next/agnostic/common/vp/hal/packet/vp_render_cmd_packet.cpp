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

static inline RENDERHAL_SURFACE_TYPE InitRenderHalSurfType(VPHAL_SURFACE_TYPE vpSurfType)
{
    switch (vpSurfType)
    {
    case SURF_IN_BACKGROUND:
        return RENDERHAL_SURF_IN_BACKGROUND;

    case SURF_IN_PRIMARY:
        return RENDERHAL_SURF_IN_PRIMARY;

    case SURF_IN_SUBSTREAM:
        return RENDERHAL_SURF_IN_SUBSTREAM;

    case SURF_IN_REFERENCE:
        return RENDERHAL_SURF_IN_REFERENCE;

    case SURF_OUT_RENDERTARGET:
        return RENDERHAL_SURF_OUT_RENDERTARGET;

    case SURF_NONE:
    default:
        return RENDERHAL_SURF_NONE;
    }
}
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

    //m_kernelCount = IDR_VP_TOTAL_NUM_KERNELS;
    RENDER_KERNEL_PARAMS kernelParams;
    MOS_ZeroMemory(&kernelParams, sizeof(RENDER_KERNEL_PARAMS));
    kernelParams.kernelId      = &m_kernelId;
    kernelParams.surfacesGroup = &m_surfacesGroup;

    VP_RENDER_CHK_STATUS_RETURN(m_kernelSet->CreateKernelObjects(kernelParams, m_kernelObjs));

    // for multi-kernel prepare together
    for (auto it = m_kernelObjs.begin(); it != m_kernelObjs.end(); it++)
    {
        m_kernel = it->second;

        VP_RENDER_CHK_NULL_RETURN(m_kernel);

        // reset render Data for current kernel
        MOS_ZeroMemory(&m_renderData, sizeof(m_renderData));

        VP_RENDER_CHK_STATUS_RETURN(RenderEngineSetup());

        VP_RENDER_CHK_STATUS_RETURN(KernelStateSetup());

        VP_RENDER_CHK_STATUS_RETURN(SetupSurfaceState()); // once Surface setup done, surface index should be created here

        VP_RENDER_CHK_STATUS_RETURN(SetupCurbeState());  // Set Curbe with updated surface index

        VP_RENDER_CHK_STATUS_RETURN(LoadKernel());

    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderCmdPacket::Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase)
{
    if (m_kernelObjs.empty())
    {
        VP_RENDER_ASSERTMESSAGE("No Kernel Object Creation");
        return MOS_STATUS_NULL_POINTER;
    }
    // for multi-kernel submit in one BB for prepared kernels
    for (auto it = m_kernelObjs.begin(); it != m_kernelObjs.end(); it++)
    {
        VP_RENDER_CHK_STATUS_RETURN(SetupMediaWalker());

        VP_RENDER_CHK_STATUS_RETURN(RenderCmdPacket::Submit(commandBuffer, packetPhase));
    }

    VP_RENDER_CHK_STATUS_RETURN(m_kernelSet->DestroyKernelObjects(m_kernelObjs));

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

    //VP_RENDER_CHK_NULL_RETURN(m_kernelSet);
    VP_RENDER_CHK_NULL_RETURN(m_kernel);

    // Processing kernel State setup
    int32_t iKUID = 0;                                         // Kernel Unique ID (DNDI uses combined kernels)
    int32_t kernelSize = 0;
    void* kernelBinary = nullptr;

    // // Initialize States
    MOS_ZeroMemory(m_filter, sizeof(m_filter));
    MOS_ZeroMemory(&m_renderData.KernelEntry, sizeof(Kdll_CacheEntry));

    // Store pointer to Kernel Parameter
    VP_RENDER_CHK_STATUS_RETURN(m_kernel->GetKernelSettings(m_renderData.KernelParam));
    VP_RENDER_CHK_STATUS_RETURN(m_kernel->GetKernelID(iKUID));
    kernelSize   = m_kernel->GetKernelSize();
    kernelBinary = m_kernel->GetKernelBinary();
    VP_RENDER_CHK_NULL_RETURN(kernelBinary);

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
    m_kernel->GetInlineState(&InlineData, iInlineLength);
    m_renderData.iInlineLength = iInlineLength;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderCmdPacket::SetupSurfaceState()
{
    VP_RENDER_CHK_NULL_RETURN(m_kernel);

    for (auto it : m_kernel->GetProcessingSurfaces())
    {
        auto surface = m_kernel->GetKernelSurfaceConfig().find(it);

        if (surface != m_kernel->GetKernelSurfaceConfig().end())
        {
            KERNEL_SURFACE2D_STATE_PARAM kernelSurfaceParam = surface->second;

            RENDERHAL_SURFACE_NEXT renderHalSurface;
            MOS_ZeroMemory(&renderHalSurface, sizeof(RENDERHAL_SURFACE_NEXT));

            RENDERHAL_SURFACE_STATE_PARAMS renderSurfaceParams;
            MOS_ZeroMemory(&renderSurfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
            if (kernelSurfaceParam.surfaceOverwriteParams.updatedSurfaceParams)
            {
                renderSurfaceParams = kernelSurfaceParam.surfaceOverwriteParams.renderSurfaceParams;
            }
            else
            {
                renderSurfaceParams.bRenderTarget    = (kernelSurfaceParam.renderTarget == true) ? 1 : 0;
                renderSurfaceParams.Boundary         = RENDERHAL_SS_BOUNDARY_ORIGINAL; // Add conditional in future for Surfaces out of range
                renderSurfaceParams.bWidth16Align    = false;
                renderSurfaceParams.bWidthInDword_Y  = true;
                renderSurfaceParams.bWidthInDword_UV = true;
            }

            VP_SURFACE* vpSurface = m_surfacesGroup.find(it)->second;

            VP_RENDER_CHK_NULL_RETURN(vpSurface);

            VP_RENDER_CHK_STATUS_RETURN(InitRenderHalSurface(*vpSurface, renderHalSurface));

            VP_RENDER_CHK_STATUS_RETURN(UpdateRenderSurface(renderHalSurface, kernelSurfaceParam));

            uint32_t index = SetSurfaceForHwAccess(
                vpSurface->osSurface,
                &renderHalSurface,
                &renderSurfaceParams,
                renderSurfaceParams.bRenderTarget);

            VP_RENDER_CHK_STATUS_RETURN(m_kernel->UpdateCurbeBindingIndex(it, index));
        }
        else
        {
            VP_RENDER_ASSERTMESSAGE("Kernel Obj Issue: Didn't find correct kernel surface here, return error");
            return MOS_STATUS_UNIMPLEMENTED;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderCmdPacket::SetupCurbeState()
{
    VP_RENDER_CHK_NULL_RETURN(m_kernel);

    // set the Curbe Data length
    void* curbeData = nullptr;
    uint32_t curbeLength = 0;
    m_kernel->GetCurbeState(curbeData, curbeLength);
    m_renderData.iCurbeLength = curbeLength;

    VP_RENDER_CHK_STATUS_RETURN(SetupCurbe(
        curbeData,
        curbeLength,
        m_renderData.KernelParam.Thread_Count));

    return MOS_STATUS_SUCCESS;
}

VP_SURFACE* VpRenderCmdPacket::GetSurface(SurfaceType type)
{
    auto it = m_surfacesGroup.find(type);
    VP_SURFACE* surf = (m_surfacesGroup.end() != it) ? it->second : nullptr;

    return surf;
}

MOS_STATUS VpRenderCmdPacket::SetupMediaWalker()
{
    VP_RENDER_CHK_NULL_RETURN(m_kernel);

    VP_RENDER_CHK_STATUS_RETURN(m_kernel->GetWalkerSetting(m_renderData.walkerParam));

    switch (m_walkerType)
    {
    case WALKER_TYPE_MEDIA:
        MOS_ZeroMemory(&m_mediaWalkerParams, sizeof(MHW_WALKER_PARAMS));
        // Prepare Media Walker Params
        VP_RENDER_CHK_STATUS_RETURN(PrepareMediaWalkerParams(m_renderData.walkerParam, m_mediaWalkerParams));
        break;
    case WALKER_TYPE_COMPUTE:
        // Parepare Compute Walker Param
        MOS_ZeroMemory(&m_gpgpuWalkerParams, sizeof(MHW_GPGPU_WALKER_PARAMS));
        VP_RENDER_CHK_STATUS_RETURN(PrepareComputeWalkerParams(m_renderData.walkerParam, m_gpgpuWalkerParams));
        break;
    case WALKER_TYPE_DISABLED:
    default:
        // using BB for walker setting
        return MOS_STATUS_UNIMPLEMENTED;
    }

    return MOS_STATUS_SUCCESS;
}
MOS_STATUS VpRenderCmdPacket::InitRenderHalSurface(VP_SURFACE& surface, RENDERHAL_SURFACE& renderSurface)
{
    VP_RENDER_CHK_NULL_RETURN(surface.osSurface);
    VP_RENDER_CHK_STATUS_RETURN(RenderCmdPacket::InitRenderHalSurface(*surface.osSurface, &renderSurface));

    renderSurface.rcSrc    = surface.rcSrc;
    renderSurface.rcDst    = surface.rcDst;
    renderSurface.rcMaxSrc = surface.rcMaxSrc;
    renderSurface.SurfType =
        InitRenderHalSurfType(surface.SurfType);

    return MOS_STATUS_SUCCESS;
}
MOS_STATUS VpRenderCmdPacket::UpdateRenderSurface(RENDERHAL_SURFACE_NEXT& renderSurface, KERNEL_SURFACE2D_STATE_PARAM& kernelParams)
{
    auto& overwriteParam = kernelParams.surfaceOverwriteParams;
    if (overwriteParam.updatedSurfaceParams)
    {
        if (overwriteParam.width && overwriteParam.height)
        {
            renderSurface.OsSurface.dwWidth = overwriteParam.width;
            renderSurface.OsSurface.dwHeight = overwriteParam.height;
        }

        renderSurface.OsSurface.dwPitch = overwriteParam.pitch != 0 ? overwriteParam.pitch : renderSurface.OsSurface.dwPitch;

        if (renderSurface.OsSurface.dwPitch < renderSurface.OsSurface.dwWidth)
        {
            VP_RENDER_ASSERTMESSAGE("Invalid Surface where Pitch < Width, return invalid Overwrite Params");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        renderSurface.OsSurface.Format = (overwriteParam.format != 0) ? overwriteParam.format : renderSurface.OsSurface.Format;
    }

    return MOS_STATUS_SUCCESS;
}
}
