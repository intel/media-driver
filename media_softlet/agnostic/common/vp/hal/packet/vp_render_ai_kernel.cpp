/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     vp_render_ai_kernel.cpp
//! \brief    ai kernel obj which used in by mediapipline.
//! \details  ai kernel obj which used in by mediapipline.
//!

#include "vp_render_ai_kernel.h"

using namespace vp;

VpRenderAiKernel::VpRenderAiKernel(PVP_MHWINTERFACE hwInterface, std::string kernelName, uint32_t kernelIndex, PVpAllocator allocator) : VpRenderKernelObj(hwInterface, kernelAiCommon, kernelIndex, kernelName, allocator)
{
    m_renderHal                  = hwInterface ? hwInterface->m_renderHal : nullptr;
    m_kernelIndex                = kernelIndex;
    m_isAdvKernel                = true;
    m_useIndependentSamplerGroup = true;
    m_kernelBinaryID             = VP_ADV_KERNEL_BINARY_ID(kernelAiCommon);
}

VpRenderAiKernel::~VpRenderAiKernel()
{
    MOS_SafeFreeMemory(m_curbe);
    m_curbe = nullptr;
}

MOS_STATUS VpRenderAiKernel::Init(VpRenderKernel &kernel)
{
    VP_FUNC_CALL();

    VP_RENDER_NORMALMESSAGE("Initializing AI krn %s", kernel.GetKernelName().c_str());

    m_kernelSize = kernel.GetKernelSize();

    uint8_t *pKernelBin = (uint8_t *)kernel.GetKernelBinPointer();
    VP_RENDER_CHK_NULL_RETURN(pKernelBin);

    m_kernelBinary = pKernelBin + kernel.GetKernelBinOffset();

    m_kernelArgs.clear();
    for (auto &arg : kernel.GetKernelArgs())
    {
        arg.pData = nullptr;
        m_kernelArgs.insert(std::make_pair(arg.uIndex, arg));
    }

    m_kernelBtis = kernel.GetKernelBtis();

    m_kernelEnv = kernel.GetKernelExeEnv();

    m_curbeSize = kernel.GetCurbeSize();

    m_inlineData.resize(m_kernelEnv.uInlineDataPayloadSize);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderAiKernel::SetSamplerStates(KERNEL_SAMPLER_STATE_GROUP &samplerStateGroup)
{
    VP_FUNC_CALL();

    if (m_kernelEnv.bHasSample)
    {
        samplerStateGroup.clear();

        MHW_SAMPLER_STATE_PARAM samplerStateParam = {};
        samplerStateParam.Unorm.SamplerFilterMode = MHW_SAMPLER_FILTER_BILINEAR;
        samplerStateParam.Unorm.MagFilter         = MHW_GFX3DSTATE_MAPFILTER_LINEAR;
        samplerStateParam.Unorm.MinFilter         = MHW_GFX3DSTATE_MAPFILTER_LINEAR;
        samplerStateParam.Unorm.AddressU          = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
        samplerStateParam.Unorm.AddressV          = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
        samplerStateParam.Unorm.AddressW          = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
        samplerStateParam.bInUse                  = true;
        samplerStateParam.SamplerType             = MHW_SAMPLER_TYPE_3D;
        if (m_linearSamplerIndex >= 0)
        {
            VP_RENDER_NORMALMESSAGE("Bilinear Sampler Set on Sampler Index %d", m_linearSamplerIndex);
            samplerStateGroup.insert(std::make_pair(m_linearSamplerIndex, samplerStateParam));
        }
        else
        {
            VP_RENDER_NORMALMESSAGE("Bilinear Sampler NOT SET for Invalid Index %d", m_linearSamplerIndex);
        }

        samplerStateParam                         = {};
        samplerStateParam.Unorm.SamplerFilterMode = MHW_SAMPLER_FILTER_NEAREST;
        samplerStateParam.Unorm.MagFilter         = MHW_GFX3DSTATE_MAPFILTER_NEAREST;
        samplerStateParam.Unorm.MinFilter         = MHW_GFX3DSTATE_MAPFILTER_NEAREST;
        samplerStateParam.Unorm.AddressU          = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
        samplerStateParam.Unorm.AddressV          = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
        samplerStateParam.Unorm.AddressW          = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
        samplerStateParam.bInUse                  = true;
        samplerStateParam.SamplerType             = MHW_SAMPLER_TYPE_3D;
        if (m_nearestSamplerIndex >= 0)
        {
            VP_RENDER_NORMALMESSAGE("Nearest Sampler Set on Sampler Index %d", m_nearestSamplerIndex);
            samplerStateGroup.insert(std::make_pair(m_nearestSamplerIndex, samplerStateParam));
        }
        else
        {
            VP_RENDER_NORMALMESSAGE("Nearest Sampler NOT SET for Invalid Index %d", m_nearestSamplerIndex);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderAiKernel::SetKernelArgs(KERNEL_ARGS &kernelArgs, VP_PACKET_SHARED_CONTEXT *sharedContext)
{
    VP_FUNC_CALL();

    for (KRN_ARG &srcArg : kernelArgs)
    {
        auto handle = m_kernelArgs.find(srcArg.uIndex);
        
        if (handle != m_kernelArgs.end())
        {
            if (srcArg.eArgKind == ARG_KIND_GENERAL || srcArg.eArgKind == ARG_KIND_INLINE)
            {
                KRN_ARG &dstArg = handle->second;
                if (srcArg.pData == nullptr)
                {
                    VP_RENDER_ASSERTMESSAGE("The Kernel Argument General Data is null! KernelName %s, argIndex %d", m_kernelName.c_str(), dstArg.uIndex);
                    return MOS_STATUS_INVALID_PARAMETER;
                }
                else
                {
                    dstArg.eArgKind = srcArg.eArgKind;
                    dstArg.pData    = srcArg.pData;
                    srcArg.pData    = nullptr;
                }
            }
            else if (srcArg.eArgKind == ARG_KIND_SURFACE && srcArg.addressMode == AddressingModeStateless)
            {
                KRN_ARG &dstArg = handle->second;
                //leave it to SetStatelessSurface() to handle, because here m_surfaceGroup is still nullptr now
                dstArg.pData = srcArg.pData;
                srcArg.pData = nullptr;
            }
            else if (srcArg.eArgKind == ARG_KIND_SAMPLER)
            {
                KRN_ARG &dstArg = handle->second;
                if (srcArg.pData == nullptr)
                {
                    VP_RENDER_ASSERTMESSAGE("The Kernel Argument Sampler Data is null! KernelName %s, argIndex %d", m_kernelName.c_str(), dstArg.uIndex);
                    return MOS_STATUS_INVALID_PARAMETER;
                }
                else
                {
                    if (*(uint32_t *)srcArg.pData == MHW_SAMPLER_FILTER_BILINEAR)
                    {
                        m_linearSamplerIndex = dstArg.uOffsetInPayload;
                        srcArg.pData         = nullptr;
                    }
                    else if (*(uint32_t *)srcArg.pData == MHW_SAMPLER_FILTER_NEAREST)
                    {
                        m_nearestSamplerIndex = dstArg.uOffsetInPayload;
                        srcArg.pData          = nullptr;
                    }
                    else
                    {
                        VP_RENDER_ASSERTMESSAGE("The Kernel Argument Sampler Data is INVALID TYPE! KernelName %s, argIndex %d, type %d", m_kernelName.c_str(), dstArg.uIndex, *(uint32_t *)srcArg.pData);
                        return MOS_STATUS_INVALID_PARAMETER;
                    }
                }
            }
        }

        if (srcArg.pData != nullptr)
        {
            srcArg.pData = nullptr;
            VP_RENDER_ASSERTMESSAGE("The Kernel Argument is set but not used. KernelName %s, argIndex %d", m_kernelName.c_str(), srcArg.uIndex);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderAiKernel::SetupStatelessBuffer()
{
    VP_PUBLIC_CHK_NULL_RETURN(m_surfaceGroup);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface);
    PMOS_INTERFACE osInterface = m_hwInterface->m_osInterface;
    VP_PUBLIC_CHK_NULL_RETURN(osInterface);

    for (auto& handle : m_kernelArgs)
    {
        KRN_ARG &arg = handle.second;
        if (arg.eArgKind == ARG_KIND_SURFACE && arg.addressMode == AddressingModeStateless)
        {
            if(arg.pData == nullptr)
            {
                VP_RENDER_ASSERTMESSAGE("The Kernel Argument Stateless Surface Data is null! KernelName %s, argIndex %d", m_kernelName.c_str(), arg.uIndex);
                return MOS_STATUS_INVALID_PARAMETER;
            }
            SURFACE_PARAMS &surfaceParam = *(SURFACE_PARAMS *)arg.pData;
            if (surfaceParam.surfType == SurfaceTypeSubPlane || surfaceParam.surfType == SurfaceTypeInvalid)
            {
                VP_RENDER_NORMALMESSAGE("Will skip stateless surface argIndex %d for its surf type is set as %d", arg.uIndex, surfaceParam.surfType);
                arg.pData = nullptr;
            }
            else
            {
                auto surfHandle = m_surfaceGroup->find(surfaceParam.surfType);
                VP_PUBLIC_CHK_NOT_FOUND_RETURN(surfHandle, m_surfaceGroup);
                VP_PUBLIC_CHK_NULL_RETURN(surfHandle->second);
                uint64_t ui64GfxAddress = osInterface->pfnGetResourceGfxAddress(osInterface, &surfHandle->second->osSurface->OsResource);
                VP_RENDER_CHK_STATUS_RETURN(osInterface->pfnRegisterResource(
                    osInterface,
                    &surfHandle->second->osSurface->OsResource,
                    surfaceParam.isOutput,
                    true));
                *(uint64_t *)arg.pData = ui64GfxAddress;
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderAiKernel::GetCurbeState(void *&curbe, uint32_t &curbeLength)
{
    VP_FUNC_CALL();
    curbeLength = m_curbeSize;

    VP_RENDER_NORMALMESSAGE("KernelID %d, Kernel Name %s, Curbe Size %d\n", m_kernelId, m_kernelName.c_str(), curbeLength);

    if (curbeLength == 0)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    uint8_t *pCurbe = (uint8_t *)MOS_AllocAndZeroMemory(curbeLength);
    VP_RENDER_CHK_NULL_RETURN(pCurbe);
    MOS_FreeMemAndSetNull(m_curbe);
    m_curbe = pCurbe;

    for (auto &handle : m_kernelArgs)
    {
        KRN_ARG &arg = handle.second;
        switch (arg.eArgKind)
        {
        case ARG_KIND_GENERAL:
        case ARG_KIND_SURFACE:
            if (arg.pData != nullptr)
            {
                MOS_SecureMemcpy(pCurbe + arg.uOffsetInPayload, arg.uSize, arg.pData, arg.uSize);
                VP_RENDER_NORMALMESSAGE("Setting Curbe State KernelName %s, index %d , value %d, argKind %d", m_kernelName.c_str(), arg.uIndex, *(uint32_t *)arg.pData, arg.eArgKind);
            }
            else
            {
                VP_RENDER_NORMALMESSAGE("KernelName %s, index %d, argKind %d is empty", m_kernelName.c_str(), arg.uIndex, arg.eArgKind);
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

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderAiKernel::SetupSurfaceState()
{
    VP_FUNC_CALL();

    KERNEL_SURFACE_STATE_PARAM kernelSurfaceParam;
    m_surfaceState.clear();
    for (auto it = m_kernelBtis.begin(); it != m_kernelBtis.end(); ++it)
    {
        uint32_t argIndex = it->first;
        uint32_t bti      = it->second;

        VP_RENDER_NORMALMESSAGE("Setting Surface State for AI Kernel. KernelName %s, layer %d, argIndex %d , bti %d", m_kernelName.c_str(), m_kernelIndex, argIndex, bti);

        MOS_ZeroMemory(&kernelSurfaceParam, sizeof(KERNEL_SURFACE_STATE_PARAM));
        kernelSurfaceParam.surfaceOverwriteParams.updatedRenderSurfaces = true;
        kernelSurfaceParam.surfaceOverwriteParams.bindedKernel          = true;
        PRENDERHAL_SURFACE_STATE_PARAMS pRenderSurfaceParams            = &kernelSurfaceParam.surfaceOverwriteParams.renderSurfaceParams;
        pRenderSurfaceParams->bAVS                                      = false;
        pRenderSurfaceParams->Boundary                                  = RENDERHAL_SS_BOUNDARY_ORIGINAL;
        pRenderSurfaceParams->b2PlaneNV12NeededByKernel                 = true;
        pRenderSurfaceParams->forceCommonSurfaceMessage                 = true;
        SurfaceType         surfType                                    = SurfaceTypeInvalid;
        MOS_HW_RESOURCE_DEF resourceType                                = MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER;

        auto surfHandle = m_argIndexSurfMap.find(argIndex);
        VP_PUBLIC_CHK_NOT_FOUND_RETURN(surfHandle, &m_argIndexSurfMap);
        if (surfHandle->second.combineChannelY)
        {
            pRenderSurfaceParams->combineChannelY = true;
        }
        surfType = surfHandle->second.surfType;
        if (surfType == SurfaceTypeSubPlane || surfType == SurfaceTypeInvalid)
        {
            VP_RENDER_NORMALMESSAGE("Will skip surface argIndex %d, bti %d for its surf type is set as %d", argIndex, bti, surfType);
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

        pRenderSurfaceParams->MemObjCtl = (m_renderHal->pOsInterface->pfnCachePolicyGetMemoryObject(
                                               resourceType,
                                               m_renderHal->pOsInterface->pfnGetGmmClientContext(m_renderHal->pOsInterface)))
                                              .DwordValue;
        pRenderSurfaceParams->Component = COMPONENT_VPCommon;

        if (surfHandle->second.needVerticalStirde)
        {
            switch (surf->second->SampleType)
            {
            case SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD:
            case SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD:
                pRenderSurfaceParams->bVertStride     = true;
                pRenderSurfaceParams->bVertStrideOffs = 0;
                break;
            case SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD:
            case SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD:
                pRenderSurfaceParams->bVertStride     = true;
                pRenderSurfaceParams->bVertStrideOffs = 1;
                break;
            default:
                pRenderSurfaceParams->bVertStride     = false;
                pRenderSurfaceParams->bVertStrideOffs = 0;
                break;
            }
        }

        if (surf->second->osSurface->Format == Format_Buffer)
        {
            kernelSurfaceParam.surfaceOverwriteParams.updatedSurfaceParams = true;
            kernelSurfaceParam.surfaceOverwriteParams.bufferResource       = true;
        }

        m_surfaceState.insert(std::make_pair(surfType, kernelSurfaceParam));

        UpdateCurbeBindingIndex(surfType, bti);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderAiKernel::GetWalkerSetting(KERNEL_WALKER_PARAMS &walkerParam, KERNEL_PACKET_RENDER_DATA &renderData)
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
MOS_STATUS VpRenderAiKernel::SetWalkerSetting(KERNEL_THREAD_SPACE &threadSpace, bool bSyncFlag, bool flushL1)
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
    m_walkerParam.simdSize          = m_kernelEnv.uSimdSize;

    m_walkerParam.pipeControlParams.bUpdateNeeded              = true;
    m_walkerParam.pipeControlParams.bEnableDataPortFlush       = true;
    m_walkerParam.pipeControlParams.bUnTypedDataPortCacheFlush = true;
    m_walkerParam.pipeControlParams.bFlushRenderTargetCache    = false;
    m_walkerParam.pipeControlParams.bInvalidateTextureCache    = false;

    for (auto &handle : m_kernelArgs)
    {
        KRN_ARG &arg = handle.second;
        if (arg.eArgKind == ARG_KIND_INLINE)
        {
            if (arg.pData != nullptr)
            {
                MOS_SecureMemcpy(m_inlineData.data() + arg.uOffsetInPayload, arg.uSize, arg.pData, arg.uSize);
                VP_RENDER_NORMALMESSAGE("Setting Inline Data KernelName %s, index %d , value %d, argKind %d", m_kernelName.c_str(), arg.uIndex, *(uint32_t *)arg.pData, arg.eArgKind);
            }
            else
            {
                VP_RENDER_NORMALMESSAGE("KernelName %s, index %d, argKind %d is empty", m_kernelName.c_str(), arg.uIndex, arg.eArgKind);
            }
        }
    }
    m_walkerParam.inlineDataLength = m_inlineData.size();
    m_walkerParam.inlineData       = m_inlineData.data();

    m_walkerParam.slmSize    = m_kernelEnv.uiSlmSize;
    m_walkerParam.hasBarrier = (m_kernelEnv.uBarrierCount > 0);

    if (m_kernelEnv.uSimdSize != 1)
    {
        m_walkerParam.isEmitInlineParameter = true;
        if (m_kernelEnv.bHasDPAS)
        {
            m_walkerParam.isGenerateLocalID = false;
            m_walkerParam.emitLocal         = MHW_EMIT_LOCAL_NONE;
        }
        else
        {
            m_walkerParam.isGenerateLocalID = true;
            m_walkerParam.emitLocal         = MHW_EMIT_LOCAL_XYZ;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderAiKernel::SetKernelConfigs(KERNEL_CONFIGS &kernelConfigs)
{
    VP_FUNC_CALL();

    auto handle = kernelConfigs.find(m_kernelId);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(handle, &kernelConfigs);

    AI_KERNEL_CONFIG *kernelConfig = (AI_KERNEL_CONFIG *)handle->second;
    VP_PUBLIC_CHK_NULL_RETURN(kernelConfig);

    m_kernelConfig = *kernelConfig;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderAiKernel::SetPerfTag()
{
    auto pOsInterface = m_hwInterface->m_osInterface;
    VP_RENDER_CHK_NULL_RETURN(pOsInterface);
    VP_RENDER_CHK_NULL_RETURN(pOsInterface->pfnSetPerfTag);

    pOsInterface->pfnSetPerfTag(pOsInterface, m_kernelConfig.perfTag);

    return MOS_STATUS_SUCCESS;
}