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

    // for multi-kernel submit together
    for (auto it = m_kernelSet->GetKernelObjs()->begin(); it != m_kernelSet->GetKernelObjs()->end(); it++)
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

        VP_RENDER_CHK_STATUS_RETURN(SetupMediaWalker());
    }

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
    VP_RENDER_CHK_NULL_RETURN(m_kernel);

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
    m_kernel->GetInlineState(&InlineData, iInlineLength);
    m_renderData.iInlineLength = iInlineLength;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderCmdPacket::SetupSurfaceState()
{
    VP_RENDER_CHK_NULL_RETURN(m_kernel);

    VP_RENDER_CHK_STATUS_RETURN(m_kernel->SetupSurfaceState());

    for (auto it : m_kernel->GetProcessingSurfaces())
    {
        auto surface = m_kernel->GetKernelSurfaceConfig().find(it);

        if (surface != m_kernel->GetKernelSurfaceConfig().end())
        {
            KERNEL_SURFACE2D_STATE_PARAM surfaceParams = surface->second;

            RENDERHAL_SURFACE_NEXT renderHalSurface;
            MOS_ZeroMemory(&renderHalSurface, sizeof(RENDERHAL_SURFACE_NEXT));

            RENDERHAL_SURFACE_STATE_PARAMS renderSurfaceParams;
            MOS_ZeroMemory(&renderSurfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
            if (surfaceParams.updatedsurfaceParams)
            {
                renderSurfaceParams = surfaceParams.renderSurfaceParams;
            }
            else
            {
                renderSurfaceParams.bRenderTarget    = (surfaceParams.renderTarget == true) ? 1 : 0;
                renderSurfaceParams.Boundary         = RENDERHAL_SS_BOUNDARY_ORIGINAL; // Add conditional in future for Surfaces out of range
                renderSurfaceParams.bWidth16Align    = false;
                renderSurfaceParams.bWidthInDword_Y  = true;
                renderSurfaceParams.bWidthInDword_UV = true;
            }

            VP_SURFACE* vpSurface = m_surfacesGroup.find(it)->second;

            VP_RENDER_CHK_NULL_RETURN(vpSurface);

            if (surfaceParams.surfaceUpdated)
            {
                UpdateRenderSurface(renderHalSurface, surfaceParams, *vpSurface);
            }

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
uint32_t VpRenderCmdPacket::GetSurfaceIndex(SurfaceType type)
{
    auto it = m_surfacesIndex.find(type);
    uint32_t index = (m_surfacesIndex.end() != it) ? it->second : 0;

    return index;
}
MOS_STATUS VpRenderCmdPacket::UpdateRenderSurface(RENDERHAL_SURFACE_NEXT& renderSurface, KERNEL_SURFACE2D_STATE_PARAM& kernelParams, VP_SURFACE &surface)
{
    renderSurface.dwWidthInUse  = (kernelParams.width != 0)   ? kernelParams.width : 0;
    renderSurface.dwHeightInUse = (kernelParams.height != 0) ? kernelParams.height : 0;

    renderSurface.OsSurface = *surface.osSurface;

    if (renderSurface.dwWidthInUse || renderSurface.dwHeightInUse)
    {
        renderSurface.OsSurface.dwWidth  = renderSurface.dwWidthInUse;
        renderSurface.OsSurface.dwHeight = renderSurface.dwHeightInUse;
        renderSurface.OsSurface.dwPitch  = renderSurface.dwWidthInUse;
        renderSurface.OsSurface.Format   = (kernelParams.format != 0) ? kernelParams.format : renderSurface.OsSurface.Format;
    }

    renderSurface.rcSrc = surface.rcSrc;
    renderSurface.rcDst = surface.rcDst;
    renderSurface.rcMaxSrc = surface.rcMaxSrc;
    renderSurface.SurfType =
        InitRenderHalSurfType(surface.SurfType);

    return MOS_STATUS_SUCCESS;
}
}
