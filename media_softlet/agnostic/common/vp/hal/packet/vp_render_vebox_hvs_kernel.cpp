/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     vp_render_vebox_hvs_kernel.cpp
//! \brief    render packet which used in by mediapipline.
//! \details  render packet provide the structures and generate the cmd buffer which mediapipline will used.
//!
#include "vp_render_vebox_hvs_kernel.h"
#include "vp_dumper.h"
#include "vp_kernelset.h"

using namespace vp;

#define VP_HVS_KERNEL_NAME "UpdateDNDITable"

VpRenderHVSKernel::VpRenderHVSKernel(PVP_MHWINTERFACE hwInterface, VpKernelID kernelID, uint32_t kernelIndex, PVpAllocator allocator) :
    VpRenderKernelObj(hwInterface, kernelID, kernelIndex, VP_HVS_KERNEL_NAME, allocator)
{
    m_kernelBinaryID = VP_ADV_KERNEL_BINARY_ID(kernelID);
    m_isAdvKernel    = true;
}

VpRenderHVSKernel::~VpRenderHVSKernel()
{
    // No need to destroy dstArg.pData, which points to the local variable
    // in VpDnFilter.
}

MOS_STATUS VpRenderHVSKernel::Init(VpRenderKernel &kernel)
{
    VP_FUNC_CALL();
    m_kernelSize = kernel.GetKernelSize() + KERNEL_BINARY_PADDING_SIZE;
    m_kernelPaddingSize = KERNEL_BINARY_PADDING_SIZE;
    uint8_t *pKernelBin = (uint8_t *)kernel.GetKernelBinPointer();
    VP_RENDER_CHK_NULL_RETURN(pKernelBin);

    m_kernelBinary = pKernelBin + kernel.GetKernelBinOffset();

    m_kernelArgs = kernel.GetKernelArgs();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHVSKernel::GetWalkerSetting(KERNEL_WALKER_PARAMS &walkerParam, KERNEL_PACKET_RENDER_DATA &renderData)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_STATUS_RETURN(VpRenderKernelObj::GetWalkerSetting(m_walkerParam, renderData));

    walkerParam = m_walkerParam;
    return MOS_STATUS_SUCCESS;
}

// Only for Adv kernels.
MOS_STATUS VpRenderHVSKernel::SetWalkerSetting(KERNEL_THREAD_SPACE &threadSpace, bool bSyncFlag, bool flushL1)
{
    VP_FUNC_CALL();
    MOS_ZeroMemory(&m_walkerParam, sizeof(KERNEL_WALKER_PARAMS));

    m_walkerParam.iBlocksX          = threadSpace.uWidth;
    m_walkerParam.iBlocksY          = threadSpace.uHeight;
    m_walkerParam.isVerticalPattern = false;
    m_walkerParam.bSyncFlag         = bSyncFlag;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHVSKernel::SetKernelArgs(KERNEL_ARGS &kernelArgs, VP_PACKET_SHARED_CONTEXT *sharedContext)
{
    VP_FUNC_CALL();
    auto   &hvsParams = sharedContext->hvsParams;
    KRN_ARG krnArg    = {};

    krnArg.uIndex   = 1;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 2;
    krnArg.pData    = &hvsParams.Mode;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 2;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 2;
    krnArg.pData    = &hvsParams.Format;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 3;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 2;
    krnArg.pData    = &hvsParams.Width;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 4;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 2;
    krnArg.pData    = &hvsParams.Height;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 5;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 4;
    krnArg.pData    = &hvsParams.dwGlobalNoiseLevel;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 6;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 4;
    krnArg.pData    = &hvsParams.dwGlobalNoiseLevelU;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 7;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 4;
    krnArg.pData    = &hvsParams.dwGlobalNoiseLevelV;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 8;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 4;
    krnArg.pData    = &hvsParams.Sgne_Level;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 9;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 4;
    krnArg.pData    = &hvsParams.Sgne_LevelU;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 10;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 4;
    krnArg.pData    = &hvsParams.Sgne_LevelV;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 11;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 4;
    krnArg.pData    = &hvsParams.Sgne_Count;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 12;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 4;
    krnArg.pData    = &hvsParams.Sgne_Count;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 13;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 4;
    krnArg.pData    = &hvsParams.Sgne_CountV;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 14;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 4;
    krnArg.pData    = &hvsParams.PrevNslvTemporal;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 15;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 4;
    krnArg.pData    = &hvsParams.PrevNslvTemporalU;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 16;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 4;
    krnArg.pData    = &hvsParams.PrevNslvTemporalV;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 17;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 2;
    krnArg.pData    = &hvsParams.QP;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 18;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 2;
    krnArg.pData    = &hvsParams.FirstFrame;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 19;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 2;
    krnArg.pData    = &hvsParams.TgneFirstFrame;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 20;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 2;
    krnArg.pData    = &hvsParams.Fallback;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 21;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 2;
    krnArg.pData    = &hvsParams.EnableChroma;
    kernelArgs.push_back(krnArg);

    krnArg.uIndex   = 22;
    krnArg.eArgKind = ARG_KIND_GENERAL;
    krnArg.uSize    = 2;
    krnArg.pData    = &hvsParams.EnableTemporalGNE;
    kernelArgs.push_back(krnArg);

    if (kernelArgs.size() != m_kernelArgs.size())
    {
        VP_RENDER_ASSERTMESSAGE("The Kernel Arguments is not aligned!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t i = 0; i < m_kernelArgs.size(); ++i)
    {
        if (i >= kernelArgs.size())
        {
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        KRN_ARG &srcArg = kernelArgs[i];
        KRN_ARG &dstArg = m_kernelArgs[i];

        if (srcArg.uIndex   != dstArg.uIndex    ||
            srcArg.uSize    != dstArg.uSize     ||
            srcArg.eArgKind != dstArg.eArgKind  &&
            dstArg.eArgKind != (srcArg.eArgKind & ~SURFACE_MASK)    ||
            srcArg.pData == nullptr)
        {
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        dstArg.eArgKind = srcArg.eArgKind;
        dstArg.pData    = srcArg.pData;
        srcArg.pData    = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHVSKernel::GetCurbeState(void *&curbe, uint32_t &curbeLength)
{
    VP_FUNC_CALL();
    curbeLength = 0;
    for (auto arg : m_kernelArgs)
    {
        curbeLength += arg.uSize;
    }

    if (sizeof(m_curbe) != curbeLength)
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    uint8_t *data = (uint8_t *)&m_curbe;

    for (auto &arg : m_kernelArgs)
    {
        if (arg.eArgKind == ARG_KIND_SURFACE)
        {
            // Resource need be added.
            uint32_t *pSurfaceindex = static_cast<uint32_t *>(arg.pData);
            auto      bindingMap    = GetSurfaceBindingIndex((SurfaceType)*pSurfaceindex);
            if (bindingMap.empty())
            {
                VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
            }
            *((uint32_t *)(data + arg.uOffsetInPayload)) = *bindingMap.begin();
        }
        else if (arg.eArgKind == ARG_KIND_GENERAL)
        {
            MOS_SecureMemcpy(data + arg.uOffsetInPayload, arg.uSize, arg.pData, arg.uSize);
        }
        else
        {
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
        }
    }

    curbe = data;
    VP_RENDER_NORMALMESSAGE("HVS Kernel curbelength %d", curbeLength);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHVSKernel::SetupSurfaceState()
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(m_surfaceGroup);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);

    PRENDERHAL_INTERFACE renderHal = m_hwInterface->m_renderHal;
    PMOS_INTERFACE osInterface = m_hwInterface->m_osInterface;
    m_surfaceBindingIndex.clear();
    m_surfaceState.clear();
    for (auto arg : m_kernelArgs)
    {
        VP_RENDER_CHK_NULL_RETURN(arg.pData);

        if (arg.eArgKind == ARG_KIND_SURFACE)
        {
            SurfaceType surfType = *(SurfaceType *)arg.pData;

            auto it = m_surfaceGroup->find(surfType);
            if (m_surfaceGroup->end() == it)
            {
                VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
            }

            auto surf = it->second;
            VP_RENDER_CHK_NULL_RETURN(surf);
            VP_RENDER_CHK_NULL_RETURN(surf->osSurface);

            uint32_t width = surf->bufferWidth;
            uint32_t height = surf->bufferHeight;

            KERNEL_SURFACE_STATE_PARAM kernelSurfaceParam = {};
            kernelSurfaceParam.surfaceOverwriteParams.updatedSurfaceParams  = true;
            kernelSurfaceParam.surfaceOverwriteParams.width                 = surf->bufferWidth;
            kernelSurfaceParam.surfaceOverwriteParams.height                = 256;
            kernelSurfaceParam.surfaceOverwriteParams.pitch                 = 0;
            kernelSurfaceParam.surfaceOverwriteParams.format                = Format_A8R8G8B8;
            kernelSurfaceParam.surfaceOverwriteParams.tileType              = surf->osSurface->TileType;
            kernelSurfaceParam.surfaceOverwriteParams.surface_offset        = 0;

            //kernelSurfaceParam.surfaceOverwriteParams.updatedRenderSurfaces                 = true;
            //kernelSurfaceParam.surfaceOverwriteParams.renderSurfaceParams.Type              = renderHal->SurfaceTypeDefault;
            //kernelSurfaceParam.surfaceOverwriteParams.renderSurfaceParams.isOutput     = true; // true if no need sync for read.
            //kernelSurfaceParam.surfaceOverwriteParams.renderSurfaceParams.bWidthInDword_Y   = true;
            //kernelSurfaceParam.surfaceOverwriteParams.renderSurfaceParams.bWidthInDword_UV  = true;
            //kernelSurfaceParam.surfaceOverwriteParams.renderSurfaceParams.Boundary          = RENDERHAL_SS_BOUNDARY_ORIGINAL;
            //kernelSurfaceParam.surfaceOverwriteParams.renderSurfaceParams.bWidth16Align     = false;
            ////set mem object control for cache
            //kernelSurfaceParam.surfaceOverwriteParams.renderSurfaceParams.MemObjCtl = (osInterface->pfnCachePolicyGetMemoryObject(
            //            MOS_MP_RESOURCE_USAGE_DEFAULT,
            //            osInterface->pfnGetGmmClientContext(osInterface))).DwordValue;

            m_surfaceState.insert(std::make_pair(*(SurfaceType *)arg.pData, kernelSurfaceParam));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHVSKernel::SetKernelConfigs(KERNEL_CONFIGS& kernelConfigs)
{
    VP_FUNC_CALL();
    auto it = kernelConfigs.find((VpKernelID)kernelHVSCalc);

    if (kernelConfigs.end() == it || nullptr == it->second)
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    PRENDER_DN_HVS_CAL_PARAMS params = (PRENDER_DN_HVS_CAL_PARAMS)it->second;

    return MOS_STATUS_SUCCESS;
}
