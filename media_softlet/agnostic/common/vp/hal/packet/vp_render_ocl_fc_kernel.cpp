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

#include "vp_render_ocl_fc_kernel.h"

using namespace vp;

VpRenderOclFcKernel::VpRenderOclFcKernel(PVP_MHWINTERFACE hwInterface, VpKernelID kernelID, uint32_t kernelIndex, PVpAllocator allocator) : VpRenderKernelObj(hwInterface, kernelID, kernelIndex, "", allocator)
{
    m_renderHal   = hwInterface ? hwInterface->m_renderHal : nullptr;
    m_kernelIndex = kernelIndex;

    switch (kernelID)
    {
    case kernelOclFcCommon:
        m_kernelName = "FastComp_fc_common";
        break;
    case kernelOclFcFP:
        m_kernelName = "FastExpress_fc_fp";
        break;
    case kernelOclFc444PL3Input:
        m_kernelName = "ImageRead_fc_444PL3_input";
        break;
    case kernelOclFc444PL3Output:
        m_kernelName = "ImageWrite_fc_444PL3_output";
        break;
    case kernelOclFc420PL3Input:
        m_kernelName = "ImageRead_fc_420PL3_input";
        break;
    case kernelOclFc420PL3Output:
        m_kernelName = "ImageWrite_fc_420PL3_output";
        break;
    case kernelOclFc422HVInput:
        m_kernelName = "ImageRead_fc_422HV_input";
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

VpRenderOclFcKernel::~VpRenderOclFcKernel()
{
    MOS_SafeFreeMemory(m_curbe);
    m_curbe = nullptr;
}

MOS_STATUS VpRenderOclFcKernel::Init(VpRenderKernel &kernel)
{
    VP_FUNC_CALL();

    VP_RENDER_NORMALMESSAGE("Initializing OCL FC krn %s", kernel.GetKernelName().c_str());

    m_kernelSize = kernel.GetKernelSize();

    uint8_t *pKernelBin = (uint8_t *)kernel.GetKernelBinPointer();
    VP_RENDER_CHK_NULL_RETURN(pKernelBin);

    m_kernelBinary = pKernelBin + kernel.GetKernelBinOffset();

    m_kernelArgs.clear();
    for (auto &arg : kernel.GetKernelArgs())
    {
        arg.pData = nullptr;
        m_kernelArgs.insert(std::make_pair(arg.uIndex,arg));
    }

    m_kernelBtis = kernel.GetKernelBtis();

    m_kernelEnv = kernel.GetKernelExeEnv();

    m_curbeSize = kernel.GetCurbeSize();

    m_inlineData.resize(m_kernelEnv.uInlineDataPayloadSize);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderOclFcKernel::SetSamplerStates(KERNEL_SAMPLER_STATE_GROUP &samplerStateGroup)
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

        samplerStateParam = {};
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

MOS_STATUS VpRenderOclFcKernel::SetKernelArgs(KERNEL_ARGS &kernelArgs, VP_PACKET_SHARED_CONTEXT *sharedContext)
{
    VP_FUNC_CALL();

    //All pData will be free in VpOclFcFilter::Destroy so no need to free here
    for (KRN_ARG &srcArg : kernelArgs)
    {
        auto handle = m_kernelArgs.find(srcArg.uIndex);

        if (srcArg.eArgKind == ARG_KIND_GENERAL || srcArg.eArgKind == ARG_KIND_INLINE)
        {
            if (handle != m_kernelArgs.end())
            {
                KRN_ARG &dstArg = handle->second;
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
        }
        else if (srcArg.eArgKind == ARG_KIND_SAMPLER)
        {
            if (handle != m_kernelArgs.end())
            {
                KRN_ARG &dstArg = handle->second;
                if (srcArg.pData == nullptr)
                {
                    VP_RENDER_ASSERTMESSAGE("The Kernel Argument Sampler Data is null! KernelID %d, argIndex %d", m_kernelId, dstArg.uIndex);
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
                        VP_RENDER_ASSERTMESSAGE("The Kernel Argument Sampler Data is INVALID TYPE! KernelID %d, argIndex %d, type %d", m_kernelId, dstArg.uIndex, *(uint32_t *)srcArg.pData);
                        return MOS_STATUS_INVALID_PARAMETER;
                    }
                }
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

MOS_STATUS VpRenderOclFcKernel::GetCurbeState(void *&curbe, uint32_t &curbeLength)
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

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderOclFcKernel::SetupSurfaceState()
{
    VP_FUNC_CALL();

    KERNEL_SURFACE_STATE_PARAM kernelSurfaceParam;
    m_surfaceState.clear();
    for (auto it = m_kernelBtis.begin(); it != m_kernelBtis.end(); ++it)
    {
        uint32_t argIndex = it->first;
        uint32_t bti      = it->second;

        VP_RENDER_NORMALMESSAGE("Setting Surface State for OCL FC. KernelID %d, layer %d, argIndex %d , bti %d", m_kernelId, m_kernelIndex, argIndex, bti);

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
        if (m_kernelId == kernelOclFcCommon ||
            m_kernelId == kernelOclFcFP)
        {
            kernelSurfaceParam.surfaceOverwriteParams.updatedSurfaceParams = true;
            kernelSurfaceParam.surfaceOverwriteParams.format               = surf->second->osSurface->Format;
            kernelSurfaceParam.surfaceOverwriteParams.width                = MOS_MIN(static_cast<uint16_t>(surf->second->osSurface->dwWidth), static_cast<uint16_t>(surf->second->rcSrc.right));
            kernelSurfaceParam.surfaceOverwriteParams.height               = MOS_MIN(static_cast<uint16_t>(surf->second->osSurface->dwHeight), static_cast<uint16_t>(surf->second->rcSrc.bottom));
        }

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

MOS_STATUS VpRenderOclFcKernel::GetWalkerSetting(KERNEL_WALKER_PARAMS &walkerParam, KERNEL_PACKET_RENDER_DATA &renderData)
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
MOS_STATUS VpRenderOclFcKernel::SetWalkerSetting(KERNEL_THREAD_SPACE &threadSpace, bool bSyncFlag, bool flushL1)
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

    for (auto &handle : m_kernelArgs)
    {
        KRN_ARG &arg = handle.second;
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

    m_walkerParam.slmSize    = m_kernelEnv.uiSlmSize;
    m_walkerParam.hasBarrier = (m_kernelEnv.uBarrierCount > 0);
    
    if (m_kernelEnv.uSimdSize != 1)
    {
        m_walkerParam.isEmitInlineParameter = true;
        m_walkerParam.isGenerateLocalID     = true;
        m_walkerParam.emitLocal             = MHW_EMIT_LOCAL_XYZ;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderOclFcKernel::SetKernelConfigs(KERNEL_CONFIGS &kernelConfigs)
{
    VP_FUNC_CALL();

    auto handle = kernelConfigs.find(m_kernelId);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(handle, &kernelConfigs);
    
    OCL_FC_KERNEL_CONFIG *kernelConfig = (OCL_FC_KERNEL_CONFIG *)handle->second;
    VP_PUBLIC_CHK_NULL_RETURN(kernelConfig);

    m_kernelConfig = *kernelConfig;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderOclFcKernel::SetPerfTag()
{
    auto pOsInterface = m_hwInterface->m_osInterface;
    VP_RENDER_CHK_NULL_RETURN(pOsInterface);
    VP_RENDER_CHK_NULL_RETURN(pOsInterface->pfnSetPerfTag);

    pOsInterface->pfnSetPerfTag(pOsInterface, m_kernelConfig.perfTag);
    
    return MOS_STATUS_SUCCESS;
}