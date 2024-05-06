/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     vp_render_vebox_update_cmd_packet.cpp
//! \brief    render packet which used in by mediapipline.
//! \details  render packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

#include "vp_render_l0_fc_kernel.h"

using namespace vp;

VpRenderL0FcKernel::VpRenderL0FcKernel(PVP_MHWINTERFACE hwInterface, VpKernelID kernelID, uint32_t kernelIndex, PVpAllocator allocator) : VpRenderKernelObj(hwInterface, kernelID, kernelIndex, "", allocator)
{
    m_renderHal = hwInterface ? hwInterface->m_renderHal : nullptr;
    m_layer     = kernelIndex;

    switch (kernelID)
    {
    case kernelFcDScale444:
        m_kernelName = "PA_444D_fc_scale";
        break;
    default:
        m_kernelName.assign("");
        VP_RENDER_ASSERTMESSAGE("Kernel ID cannot map to Kernel Name");
        break;
    }
    m_isAdvKernel                = true;
    m_useIndependentSamplerGroup = true;
    m_kernelBinaryID             = VP_ADV_KERNEL_BINARY_ID(kernelID);
}

VpRenderL0FcKernel::~VpRenderL0FcKernel()
{
    MOS_SafeFreeMemory(m_curbe);
    m_curbe = nullptr;
}

MOS_STATUS VpRenderL0FcKernel::Init(VpRenderKernel &kernel)
{
    VP_FUNC_CALL();

    VP_RENDER_NORMALMESSAGE("Initializing L0 FC krn %s", kernel.GetKernelName().c_str());

    m_kernelSize = kernel.GetKernelSize();

    uint8_t *pKernelBin = (uint8_t *)kernel.GetKernelBinPointer();
    VP_RENDER_CHK_NULL_RETURN(pKernelBin);

    m_kernelBinary = pKernelBin + kernel.GetKernelBinOffset();

    m_kernelArgs = kernel.GetKernelArgs();

    for (auto arg : m_kernelArgs)
    {
        arg.pData = nullptr;
    }

    m_kernelBtis = kernel.GetKernelBtis();

    m_kernelEnv = kernel.GetKernelExeEnv();

    m_curbeSize = kernel.GetCurbeSize();

    m_inlineData.resize(m_kernelEnv.uInlineDataPayloadSize);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderL0FcKernel::SetSamplerStates(KERNEL_SAMPLER_STATE_GROUP &samplerStateGroup)
{
    VP_FUNC_CALL();

    if (m_kernelEnv.bHasSample)
    {
        samplerStateGroup.clear();
        MHW_SAMPLER_STATE_PARAM samplerStateParam = {};

        samplerStateParam.Unorm.SamplerFilterMode = MHW_SAMPLER_FILTER_BILINEAR;
        samplerStateParam.Unorm.MagFilter = MHW_GFX3DSTATE_MAPFILTER_LINEAR;
        samplerStateParam.Unorm.MinFilter = MHW_GFX3DSTATE_MAPFILTER_LINEAR;

        samplerStateParam.Unorm.AddressU = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
        samplerStateParam.Unorm.AddressV = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
        samplerStateParam.Unorm.AddressW = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;

        samplerStateParam.bInUse      = true;
        samplerStateParam.SamplerType = MHW_SAMPLER_TYPE_3D;

        samplerStateGroup.insert(std::make_pair(m_samplerIndex, samplerStateParam));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderL0FcKernel::SetKernelArgs(KERNEL_ARGS &kernelArgs, VP_PACKET_SHARED_CONTEXT *sharedContext)
{
    VP_FUNC_CALL();

    //All pData will be free in VpL0FcFilter::Destroy so no need to free here
    for (KRN_ARG &srcArg : kernelArgs)
    {
        for (KRN_ARG &dstArg : m_kernelArgs)
        {
            if (srcArg.uIndex == dstArg.uIndex)
            {
                if (dstArg.eArgKind == ARG_KIND_GENERAL || dstArg.eArgKind == ARG_KIND_INLINE)
                {
                    if (srcArg.pData == nullptr)
                    {
                        VP_RENDER_ASSERTMESSAGE("The Kernel Argument General Data is null! KernelID %d, argIndex %d", m_kernelId, dstArg.uIndex);
                        return MOS_STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        dstArg.eArgKind = srcArg.eArgKind;
                        dstArg.pData    = srcArg.pData;
                        srcArg.pData    = nullptr;
                    }
                }
                else if (dstArg.eArgKind == ARG_KIND_SAMPLER)
                {
                    m_samplerIndex = dstArg.uOffsetInPayload;
                }
            }
        }

        if (srcArg.eArgKind == ARG_KIND_SURFACE)
        {
            if (srcArg.pData == nullptr)
            {
                VP_RENDER_ASSERTMESSAGE("The Kernel Argument Surface is null! KernelID %d, argIndex %d", m_kernelId, srcArg.uIndex);
                return MOS_STATUS_INVALID_PARAMETER;
            }
            else
            {
                SURFACE_PARAMS surfaceParams;
                surfaceParams.isOutput = srcArg.isOutput;
                surfaceParams.surfType = *(SurfaceType *)srcArg.pData;
                m_argIndexSurfMap.insert(std::make_pair(srcArg.uIndex, surfaceParams));
                srcArg.pData = nullptr;
            }
        }

        if (srcArg.pData != nullptr)
        {
            srcArg.pData = nullptr;
            VP_RENDER_ASSERTMESSAGE("The Kernel Argument is set but not used. KernelID %d, argIndex %d", m_kernelId, srcArg.uIndex);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderL0FcKernel::GetCurbeState(void *&curbe, uint32_t &curbeLength)
{
    VP_FUNC_CALL();
    curbeLength = m_curbeSize;

    VP_RENDER_NORMALMESSAGE("KernelID %d, Curbe Size %d\n", m_kernelId, curbeLength);

    if (curbeLength == 0)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    uint8_t *pCurbe = (uint8_t *)MOS_AllocAndZeroMemory(curbeLength);
    VP_RENDER_CHK_NULL_RETURN(pCurbe);
    MOS_FreeMemAndSetNull(m_curbe);
    m_curbe = pCurbe;

    for (auto &arg : m_kernelArgs)
    {
        switch (arg.eArgKind)
        {
        case ARG_KIND_GENERAL:
        case ARG_KIND_SURFACE:
            if (arg.pData != nullptr)
            {
                MOS_SecureMemcpy(pCurbe + arg.uOffsetInPayload, arg.uSize, arg.pData, arg.uSize);
                VP_RENDER_NORMALMESSAGE("Setting Curbe State KernelID %d, index %d , value %d, argKind %d", m_kernelId, arg.uIndex, *(uint32_t *)arg.pData, arg.eArgKind);
            }
            else
            {
                VP_RENDER_NORMALMESSAGE("KernelID %d, index %d, argKind %d is empty", m_kernelId, arg.uIndex, arg.eArgKind);
            }
            break;
        case ARG_KIND_INLINE:
        case ARG_KIND_SAMPLER:
            break;
        default:
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
        }
    }

    curbe = pCurbe;

    PrintCurbe(pCurbe, curbeLength);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderL0FcKernel::SetupSurfaceState()
{
    VP_FUNC_CALL();

    KERNEL_SURFACE_STATE_PARAM kernelSurfaceParam;

    for (auto it = m_kernelBtis.begin(); it != m_kernelBtis.end(); ++it)
    {
        uint32_t argIndex = it->first;
        uint32_t bti      = it->second;

        VP_RENDER_NORMALMESSAGE("Setting Surface State for L0 FC. KernelID %d, layer %d, argIndex %d , bti %d", m_kernelId, m_layer, argIndex, bti);

        MOS_ZeroMemory(&kernelSurfaceParam, sizeof(KERNEL_SURFACE_STATE_PARAM));
        kernelSurfaceParam.surfaceOverwriteParams.updatedRenderSurfaces = true;
        kernelSurfaceParam.surfaceOverwriteParams.bindedKernel          = true;
        PRENDERHAL_SURFACE_STATE_PARAMS pRenderSurfaceParams            = &kernelSurfaceParam.surfaceOverwriteParams.renderSurfaceParams;
        pRenderSurfaceParams->bAVS                                      = false;
        pRenderSurfaceParams->Boundary                                  = RENDERHAL_SS_BOUNDARY_ORIGINAL;
        SurfaceType         surfType                                    = SurfaceTypeInvalid;
        MOS_HW_RESOURCE_DEF resourceType                                = MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER;

        auto surfHandle = m_argIndexSurfMap.find(argIndex);
        VP_PUBLIC_CHK_NOT_FOUND_RETURN(surfHandle, &m_argIndexSurfMap);
        surfType = surfHandle->second.surfType;
        if (surfType == SurfaceTypeInvalid)
        {
            VP_RENDER_NORMALMESSAGE("Will skip surface argIndex %d, bti %d for it is set as invalid", argIndex, bti);
            continue;
        }
        pRenderSurfaceParams->isOutput = surfHandle->second.isOutput;
        if (m_surfaceState.find(surfType) != m_surfaceState.end())
        {
            UpdateCurbeBindingIndex(surfType, bti);
            continue;
        }
        auto surf = m_surfaceGroup->find(surfType);
        if (m_surfaceGroup->end() == surf)
        {
            VP_RENDER_ASSERTMESSAGE("surf was not found %d", surfType);
            return MOS_STATUS_NULL_POINTER;
        }
        VP_RENDER_CHK_NULL_RETURN(surf->second);
        VP_RENDER_CHK_NULL_RETURN(surf->second->osSurface);
        kernelSurfaceParam.surfaceOverwriteParams.updatedSurfaceParams = true;
        kernelSurfaceParam.surfaceOverwriteParams.format = surf->second->osSurface->Format;
        kernelSurfaceParam.surfaceOverwriteParams.width  = MOS_MIN(surf->second->osSurface->dwWidth, static_cast<uint64_t>(surf->second->rcSrc.right));
        kernelSurfaceParam.surfaceOverwriteParams.height = MOS_MIN(surf->second->osSurface->dwHeight, static_cast<uint64_t>(surf->second->rcSrc.bottom));

        pRenderSurfaceParams->MemObjCtl = (m_renderHal->pOsInterface->pfnCachePolicyGetMemoryObject(
                                               resourceType,
                                               m_renderHal->pOsInterface->pfnGetGmmClientContext(m_renderHal->pOsInterface)))
                                              .DwordValue;
        pRenderSurfaceParams->Component   = COMPONENT_VPCommon;

        if (surf->second->osSurface->Type == MOS_GFXRES_BUFFER)
        {
            kernelSurfaceParam.surfaceOverwriteParams.updatedSurfaceParams = true;
            kernelSurfaceParam.surfaceOverwriteParams.bufferResource       = true;
        }

        m_surfaceState.insert(std::make_pair(surfType, kernelSurfaceParam));

        UpdateCurbeBindingIndex(surfType, bti);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderL0FcKernel::GetWalkerSetting(KERNEL_WALKER_PARAMS &walkerParam, KERNEL_PACKET_RENDER_DATA &renderData)
{
    VP_FUNC_CALL();

    walkerParam = m_walkerParam;

    walkerParam.iBindingTable = renderData.bindingTable;
    walkerParam.iMediaID      = renderData.mediaID;
    walkerParam.iCurbeOffset  = renderData.iCurbeOffset;
    // Should use renderData.iCurbeLength instead of kernelSettings.CURBE_Length.
    // kernelSettings.CURBE_Length is 32 aligned with 5 bits shift.
    // renderData.iCurbeLength is RENDERHAL_CURBE_BLOCK_ALIGN(64) aligned.
    walkerParam.iCurbeLength = renderData.iCurbeLength;

    return MOS_STATUS_SUCCESS;
}

// Only for Adv kernels.
MOS_STATUS VpRenderL0FcKernel::SetWalkerSetting(KERNEL_THREAD_SPACE &threadSpace, bool bSyncFlag, bool flushL1)
{
    VP_FUNC_CALL();
    MOS_ZeroMemory(&m_walkerParam, sizeof(KERNEL_WALKER_PARAMS));

    m_walkerParam.iBlocksX          = threadSpace.uWidth;
    m_walkerParam.iBlocksY          = threadSpace.uHeight;
    m_walkerParam.threadWidth       = threadSpace.uLocalWidth;
    m_walkerParam.threadHeight      = threadSpace.uLocalHeight;
    m_walkerParam.threadDepth       = 1;
    m_walkerParam.isVerticalPattern = false;
    m_walkerParam.bSyncFlag         = bSyncFlag;

    m_walkerParam.pipeControlParams.bUpdateNeeded              = true;
    m_walkerParam.pipeControlParams.bEnableDataPortFlush       = true;
    m_walkerParam.pipeControlParams.bUnTypedDataPortCacheFlush = true;
    m_walkerParam.pipeControlParams.bFlushRenderTargetCache    = false;
    m_walkerParam.pipeControlParams.bInvalidateTextureCache    = false;

    for (auto &arg : m_kernelArgs)
    {
        if (arg.eArgKind == ARG_KIND_INLINE)
        {
            if (arg.pData != nullptr)
            {
                MOS_SecureMemcpy(m_inlineData.data() + arg.uOffsetInPayload, arg.uSize, arg.pData, arg.uSize);
                VP_RENDER_NORMALMESSAGE("Setting Inline Data KernelID %d, index %d , value %d, argKind %d", m_kernelId, arg.uIndex, *(uint32_t *)arg.pData, arg.eArgKind);
            }
            else
            {
                VP_RENDER_NORMALMESSAGE("KernelID %d, index %d, argKind %d is empty", m_kernelId, arg.uIndex, arg.eArgKind);
            }
        }
    }
    m_walkerParam.inlineDataLength = m_inlineData.size();
    m_walkerParam.inlineData       = m_inlineData.data();
    
    if (m_kernelEnv.uSimdSize != 1 &&
        (m_kernelEnv.uiWorkGroupWalkOrderDimensions[0] != 0 || 
         m_kernelEnv.uiWorkGroupWalkOrderDimensions[1] != 0 || 
         m_kernelEnv.uiWorkGroupWalkOrderDimensions[2] != 0))
    {
        m_walkerParam.isEmitInlineParameter = true;
        m_walkerParam.isGenerateLocalID     = true;
        m_walkerParam.emitLocal             = MHW_EMIT_LOCAL_XYZ;
    }

    return MOS_STATUS_SUCCESS;
}