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

    if (m_packetResourcesdPrepared)
    {
        VP_RENDER_NORMALMESSAGE("Resource Prepared, skip this time");
        return MOS_STATUS_SUCCESS;
    }

    m_PacketId = VP_PIPELINE_PACKET_RENDER;

    //m_kernelCount = IDR_VP_TOTAL_NUM_KERNELS;
    RENDER_KERNEL_PARAMS kernelParams;
    MOS_ZeroMemory(&kernelParams, sizeof(RENDER_KERNEL_PARAMS));
    kernelParams.kernelId      = &m_kernelId;
    kernelParams.surfacesGroup = &m_surfSetting.surfGroup;

    VP_RENDER_CHK_STATUS_RETURN(m_kernelSet->CreateKernelObjects(kernelParams, m_kernelObjs));

    // for multi-kernel prepare together
    for (auto it = m_kernelObjs.begin(); it != m_kernelObjs.end(); it++)
    {
        void* kernelParams = nullptr;
        KernelId kernelExecuteID = it->first;
        m_kernel                 = it->second;

        VP_RENDER_CHK_NULL_RETURN(m_kernel);

        if (m_kernelConfigs.find(kernelExecuteID) != m_kernelConfigs.end())
        {
            kernelParams = m_kernelConfigs.find(kernelExecuteID)->second;
        }

        m_kernel->SetKernelConfigs(kernelParams);

        // reset render Data for current kernel
        MOS_ZeroMemory(&m_renderData, sizeof(m_renderData));

        VP_RENDER_CHK_STATUS_RETURN(RenderEngineSetup());

        VP_RENDER_CHK_STATUS_RETURN(KernelStateSetup((KernelID)kernelExecuteID));

        VP_RENDER_CHK_STATUS_RETURN(SetupSurfaceState()); // once Surface setup done, surface index should be created here

        VP_RENDER_CHK_STATUS_RETURN(SetupCurbeState());  // Set Curbe with updated surface index

        VP_RENDER_CHK_STATUS_RETURN(LoadKernel());

        m_kernelRenderData.insert(std::make_pair(kernelExecuteID, m_renderData));

    }

    m_packetResourcesdPrepared = true;

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
        KernelId kernelExecuteID = it->first;
        VpRenderKernelObj* kernel = it->second;

        VP_RENDER_CHK_NULL_RETURN(kernel);

        if (m_kernelRenderData.find(kernelExecuteID) != m_kernelRenderData.end())
        {
            m_renderData = m_kernelRenderData.find(kernelExecuteID)->second;
        }

        VP_RENDER_CHK_STATUS_RETURN(SetupMediaWalker(kernel));

        VP_RENDER_CHK_STATUS_RETURN(RenderCmdPacket::Submit(commandBuffer, packetPhase));
    }

    VP_RENDER_CHK_STATUS_RETURN(m_kernelSet->DestroyKernelObjects(m_kernelObjs));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderCmdPacket::SetVeboxUpdateParams(PVEBOX_UPDATE_PARAMS params)
{
    VP_RENDER_CHK_NULL_RETURN(params);

    if (params->kernelGroup.empty())
    {
        VP_RENDER_ASSERTMESSAGE("No Kernel need to be processed");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    for (auto it : params->kernelGroup)
    {
        uint32_t kernel = it;
        m_kernelId.push_back(kernel);
        m_kernelConfigs.insert(std::make_pair((KernelId)kernel, (void*)params));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderCmdPacket::SetSecureCopyParams(bool copyNeeded)
{
    return MOS_STATUS();
}

MOS_STATUS VpRenderCmdPacket::PacketInit(
    VP_SURFACE* inputSurface, 
    VP_SURFACE* outputSurface, 
    VP_SURFACE* previousSurface, 
    VP_SURFACE_SETTING& surfSetting, 
    VP_EXECUTE_CAPS packetCaps)
{
    // will remodify when normal render path enabled
    VP_UNUSED(inputSurface);
    VP_UNUSED(outputSurface);
    VP_UNUSED(previousSurface);

    m_PacketCaps    = packetCaps;

    // Init packet surface params.
    m_surfSetting = surfSetting;

    m_packetResourcesdPrepared = false;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderCmdPacket::KernelStateSetup(KernelID kernelExecuteID)
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
    VP_RENDER_CHK_STATUS_RETURN(m_kernel->GetKernelSettings(m_renderData.KernelParam, kernelExecuteID));
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

    VP_RENDER_CHK_STATUS_RETURN(m_kernel->SetProcessSurface(m_surfSetting.surfGroup));

    if (!m_kernel->GetKernelSurfaceConfig().empty())
    {
        for (auto surface = m_kernel->GetKernelSurfaceConfig().begin(); surface != m_kernel->GetKernelSurfaceConfig().end(); surface++)
        {
            KERNEL_SURFACE2D_STATE_PARAM *kernelSurfaceParam = &surface->second;
            SurfaceType type = surface->first;

            RENDERHAL_SURFACE_NEXT renderHalSurface;
            MOS_ZeroMemory(&renderHalSurface, sizeof(RENDERHAL_SURFACE_NEXT));

            RENDERHAL_SURFACE_STATE_PARAMS renderSurfaceParams;
            MOS_ZeroMemory(&renderSurfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
            if (kernelSurfaceParam->surfaceOverwriteParams.updatedRenderSurfaces)
            {
                renderSurfaceParams = kernelSurfaceParam->surfaceOverwriteParams.renderSurfaceParams;
            }
            else
            {
                renderSurfaceParams.bRenderTarget    = (kernelSurfaceParam->renderTarget == true) ? 1 : 0;
                renderSurfaceParams.Boundary         = RENDERHAL_SS_BOUNDARY_ORIGINAL; // Add conditional in future for Surfaces out of range
                renderSurfaceParams.bWidth16Align    = false;
                renderSurfaceParams.bWidthInDword_Y  = true;
                renderSurfaceParams.bWidthInDword_UV = true;
            }

            VP_SURFACE* vpSurface = nullptr;

            if (m_surfSetting.surfGroup.find(type) != m_surfSetting.surfGroup.end())
            {
                vpSurface = m_surfSetting.surfGroup.find(type)->second;
            }
            else
            {
                vpSurface = nullptr;
            }

            if (vpSurface)
            {
                // Prepare surfaces tracked in Resource manager
                VP_RENDER_CHK_STATUS_RETURN(InitRenderHalSurface(*vpSurface, renderHalSurface));
            }
            else
            {
                // State Heaps are not tracked in resource manager till now
                VP_RENDER_CHK_STATUS_RETURN(InitStateHeapSurface(type, renderHalSurface));
            }

            VP_RENDER_CHK_STATUS_RETURN(UpdateRenderSurface(renderHalSurface, *kernelSurfaceParam));

            uint32_t index = 0;

            if (kernelSurfaceParam->surfaceOverwriteParams.bindedKernel)
            {
                index = SetSurfaceForHwAccess(
                    &renderHalSurface.OsSurface,
                    &renderHalSurface,
                    &renderSurfaceParams,
                    kernelSurfaceParam->surfaceOverwriteParams.bindIndex,
                    renderSurfaceParams.bRenderTarget);
            }
            else
            {
                if ((kernelSurfaceParam->surfaceOverwriteParams.updatedSurfaceParams  &&
                     kernelSurfaceParam->surfaceOverwriteParams.bufferResource)       ||
                    (!kernelSurfaceParam->surfaceOverwriteParams.updatedSurfaceParams &&
                      renderHalSurface.OsSurface.Type == MOS_GFXRES_BUFFER))
                {
                    index = SetBufferForHwAccess(
                        &renderHalSurface.OsSurface,
                        &renderHalSurface,
                        &renderSurfaceParams,
                        renderSurfaceParams.bRenderTarget);
                }
                else
                {
                    index = SetSurfaceForHwAccess(
                        &renderHalSurface.OsSurface,
                        &renderHalSurface,
                        &renderSurfaceParams,
                        renderSurfaceParams.bRenderTarget);
                }
            }

            VP_RENDER_CHK_STATUS_RETURN(m_kernel->UpdateCurbeBindingIndex(type, index));
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
    auto it = m_surfSetting.surfGroup.find(type);
    VP_SURFACE* surf = (m_surfSetting.surfGroup.end() != it) ? it->second : nullptr;

    return surf;
}

MOS_STATUS VpRenderCmdPacket::SetupMediaWalker(VpRenderKernelObj* kernel)
{
    VP_RENDER_CHK_NULL_RETURN(kernel);

    VP_RENDER_CHK_STATUS_RETURN(kernel->GetWalkerSetting(m_renderData.walkerParam));

    MOS_ZeroMemory(&m_mediaWalkerParams, sizeof(MHW_WALKER_PARAMS));
    MOS_ZeroMemory(&m_gpgpuWalkerParams, sizeof(MHW_GPGPU_WALKER_PARAMS));

    switch (m_walkerType)
    {
    case WALKER_TYPE_MEDIA:
        // Prepare Media Walker Params
        VP_RENDER_CHK_STATUS_RETURN(PrepareMediaWalkerParams(m_renderData.walkerParam, m_mediaWalkerParams));
        break;
    case WALKER_TYPE_COMPUTE:
        // Parepare Compute Walker Param
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
MOS_STATUS VpRenderCmdPacket::InitStateHeapSurface(SurfaceType type, RENDERHAL_SURFACE& renderSurface)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_SURFACE mosSurface;

    MOS_ZeroMemory(&mosSurface, sizeof(MOS_SURFACE));

    // Check for Vebox Heap readiness
    const MHW_VEBOX_HEAP* pVeboxHeap = nullptr;
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_veboxInterface);

    VP_RENDER_CHK_STATUS_RETURN(m_hwInterface->m_veboxInterface->GetVeboxHeapInfo(
        &pVeboxHeap));
    VP_RENDER_CHK_NULL_RETURN(pVeboxHeap);

    switch (type)
    {
    case SurfaceTypeVeboxStateHeap_Drv:
        mosSurface.OsResource = pVeboxHeap->DriverResource;
        break;
    case SurfaceTypeVeboxStateHeap_Knr:
        mosSurface.OsResource = pVeboxHeap->KernelResource;
        break;
    default:
        eStatus = MOS_STATUS_UNIMPLEMENTED;
        VP_RENDER_ASSERTMESSAGE("Not Inplenmented in driver now, return fail");
        break;
    }

    VP_RENDER_CHK_STATUS_RETURN(RenderCmdPacket::InitRenderHalSurface(mosSurface, &renderSurface));

    return eStatus;
}
MOS_STATUS VpRenderCmdPacket::UpdateRenderSurface(RENDERHAL_SURFACE_NEXT& renderSurface, KERNEL_SURFACE2D_STATE_PARAM& kernelParams)
{
    auto& overwriteParam = kernelParams.surfaceOverwriteParams;
    if (overwriteParam.updatedSurfaceParams)
    {
        if (overwriteParam.width && overwriteParam.height)
        {
            renderSurface.OsSurface.dwWidth  = overwriteParam.width;
            renderSurface.OsSurface.dwHeight = overwriteParam.height;
            renderSurface.OsSurface.dwQPitch = overwriteParam.height;
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
