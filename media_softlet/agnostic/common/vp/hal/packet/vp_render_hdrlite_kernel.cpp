#include "vp_render_hdrlite_kernel.h"

using namespace vp;

VpRenderHDRLITEKernel::VpRenderHDRLITEKernel(PVP_MHWINTERFACE hwInterface, VpKernelID kernelID, uint32_t kernelIndex, PVpAllocator allocator) : VpRenderKernelObj(hwInterface, kernelID, kernelIndex, "", allocator)
{
    m_renderHal   = hwInterface ? hwInterface->m_renderHal : nullptr;
    m_kernelIndex = kernelIndex;

    switch (static_cast<int>(kernelID))
    {
    case static_cast<int>(kernelHdrMandatoryLite):
        m_kernelName = "HDR_mandatory_HdrRender";  // This is the exact name in igvp_xxx.cpp:InitVpDelayedNativeAdvKernel
        break;
    default:
        m_kernelName.assign("");
        VP_RENDER_ASSERTMESSAGE("Kernel ID cannot map to Kernel Name");
        break;
    }
    m_isAdvKernel                = true;
    m_useIndependentSamplerGroup = true;
    m_kernelBinaryID             = VP_ADV_KERNEL_BINARY_ID(kernelHdrMandatoryLite);
}

VpRenderHDRLITEKernel::~VpRenderHDRLITEKernel()
{
    MOS_SafeFreeMemory(m_curbe);
    m_curbe = nullptr;
}

MOS_STATUS VpRenderHDRLITEKernel::Init(VpRenderKernel &kernel)
{
    VP_FUNC_CALL();

    VP_RENDER_NORMALMESSAGE("Initializing HDRLITE krn %s", kernel.GetKernelName().c_str());

    m_kernelSize = kernel.GetKernelSize();

    uint8_t *pKernelBin = (uint8_t *)kernel.GetKernelBinPointer();
    VP_RENDER_CHK_NULL_RETURN(pKernelBin);

    m_kernelBinary = pKernelBin + kernel.GetKernelBinOffset();

    m_kernelArgs.clear();
    for (auto &arg : kernel.GetKernelArgs())
    {
        arg.pData = nullptr;
        m_kernelArgs.emplace(arg.uIndex,arg);
    }

    m_kernelBtis = kernel.GetKernelBtis();

    m_kernelEnv = kernel.GetKernelExeEnv();

    m_curbeLocation.size = kernel.GetCurbeSize();

    m_inlineData.resize(m_kernelEnv.uInlineDataPayloadSize);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHDRLITEKernel::SetSamplerStates(KERNEL_SAMPLER_STATE_GROUP &samplerStateGroup)
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
            samplerStateGroup.emplace(m_linearSamplerIndex, samplerStateParam);
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
            samplerStateGroup.emplace(m_nearestSamplerIndex, samplerStateParam);
        }
        else
        {
            VP_RENDER_NORMALMESSAGE("Nearest Sampler NOT SET for Invalid Index %d", m_nearestSamplerIndex);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHDRLITEKernel::SetKernelArgs(KERNEL_ARGS &kernelArgs, VP_PACKET_SHARED_CONTEXT *sharedContext)
{
    VP_FUNC_CALL();

    //All pData will be free in VpFilter::Destroy so no need to free here
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
                        dstArg.pData         = srcArg.pData;
                        srcArg.pData         = nullptr;
                    }
                    else if (*(uint32_t *)srcArg.pData == MHW_SAMPLER_FILTER_NEAREST)
                    {
                        dstArg.pData          = srcArg.pData;
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

MOS_STATUS VpRenderHDRLITEKernel::GetCurbeState(void *&curbe, uint32_t &curbeLength)
{
    VP_FUNC_CALL();
    curbeLength = m_curbeLocation.size;

    bool isLocalIdGeneratedByRuntime = IsLocalIdGeneratedByRuntime(m_kernelEnv, m_kernelPerThreadArgInfo, m_walkerParam.threadWidth, m_walkerParam.threadHeight, m_walkerParam.threadDepth);
    if (isLocalIdGeneratedByRuntime)
    {
        VP_RENDER_CHK_STATUS_RETURN(PaddingPerThreadCurbe(curbeLength, m_walkerParam.threadWidth, m_walkerParam.threadHeight, m_walkerParam.threadDepth));
    }

    VP_RENDER_NORMALMESSAGE("KernelID %d, Kernel Name %s, Curbe Size %d\n", m_kernelId, m_kernelName.c_str(), curbeLength);

    if (curbeLength == 0)
    {
        VP_RENDER_NORMALMESSAGE("Skip Allocate Curbe for its Size is 0");
        curbe = nullptr;
        return MOS_STATUS_SUCCESS;
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
        case ARG_KIND_SURFACE:
            if (arg.addressMode == AddressingModeBindless)
            {
                // this is bindless surface
                VP_PUBLIC_CHK_STATUS_RETURN(SetBindlessSurfaceStateToResourceList(arg));
                VP_RENDER_NORMALMESSAGE("Setting Curbe State Bindless Surface KernelID %d, index %d, argKind %d", m_kernelId, arg.uIndex, arg.eArgKind);
            }
            else if(arg.addressMode == AddressingModeStateless)
            {
                VP_PUBLIC_CHK_NULL_RETURN(arg.pData)
                VP_PUBLIC_CHK_NULL_RETURN(m_surfaceGroup);
                SurfaceType surfType = *((SurfaceType *)arg.pData);
                auto        it       = m_surfaceGroup->find(surfType);
                VP_PUBLIC_CHK_NOT_FOUND_RETURN(it, m_surfaceGroup);
                PVP_SURFACE surface = it->second;
                VP_PUBLIC_CHK_NULL_RETURN(surface);
                VP_PUBLIC_CHK_NULL_RETURN(surface->osSurface);

                MHW_INDIRECT_STATE_RESOURCE_PARAMS params = {};
                params.isWrite                            = arg.isOutput;
                params.resource                           = &surface->osSurface->OsResource;
                params.stateOffset                        = arg.uOffsetInPayload;
                m_curbeResourceList.push_back(params);
            }
            else
            {
                VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
            }
            break;
        case ARG_KIND_INLINE:
            break;
        case ARG_KIND_SAMPLER:
            if (arg.addressMode == AddressingModeBindless)
            {
                VP_RENDER_CHK_NULL_RETURN(arg.pData);
                uint32_t samplerIndex = (*(uint32_t *)arg.pData == MHW_SAMPLER_FILTER_NEAREST) ? m_nearestSamplerIndex : m_linearSamplerIndex;
                VP_RENDER_CHK_STATUS_RETURN(SetBindlessSamplerToResourceList(arg, samplerIndex));
                VP_RENDER_NORMALMESSAGE("Setting Curbe State Bindless Sampler KernelID %d, index %d, argKind %d", m_kernelId, arg.uIndex, arg.eArgKind);
            }
            break;
        default:
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
        }
    }

    if (isLocalIdGeneratedByRuntime)
    {
        VP_RENDER_CHK_STATUS_RETURN(SetPerThreadCurbe(pCurbe, m_curbeLocation.size, curbeLength, m_kernelPerThreadArgInfo, m_walkerParam.threadWidth, m_walkerParam.threadHeight, m_walkerParam.threadDepth));
    }

    curbe = pCurbe;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHDRLITEKernel::GetKernelSurfaceParam(SURFACE_PARAMS &surfParam, KERNEL_SURFACE_STATE_PARAM &kernelSurfaceParam)
{
    MOS_ZeroMemory(&kernelSurfaceParam, sizeof(KERNEL_SURFACE_STATE_PARAM));
    kernelSurfaceParam.surfaceOverwriteParams.updatedRenderSurfaces = true;
    kernelSurfaceParam.surfaceOverwriteParams.bindedKernel          = false;
    PRENDERHAL_SURFACE_STATE_PARAMS pRenderSurfaceParams            = &kernelSurfaceParam.surfaceOverwriteParams.renderSurfaceParams;
    pRenderSurfaceParams->bAVS                                      = false;
    pRenderSurfaceParams->Boundary                                  = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    pRenderSurfaceParams->b2PlaneNV12NeededByKernel                 = true;
    pRenderSurfaceParams->forceCommonSurfaceMessage                 = true;
    MOS_HW_RESOURCE_DEF resourceType                                = MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER;
    SurfaceType         surfType                                    = surfParam.surfType;

    if (surfParam.combineChannelY)
    {
        pRenderSurfaceParams->combineChannelY = true;
    }
    pRenderSurfaceParams->isOutput = surfParam.isOutput;
    
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
                                           m_renderHal->pOsInterface))
                                          .DwordValue;
    pRenderSurfaceParams->Component = COMPONENT_VPCommon;

    if (surfParam.needVerticalStirde)
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

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHDRLITEKernel::SetupSurfaceState()
{
    VP_FUNC_CALL();

    KERNEL_SURFACE_STATE_PARAM kernelSurfaceParam;
    m_surfaceState.clear();
    for (auto &handle : m_kernelArgs)
    {
        KRN_ARG &arg = handle.second;

        if (arg.addressMode != AddressingModeBindless || arg.eArgKind != ARG_KIND_SURFACE)
        {
            continue;
        }
        uint32_t argIndex   = arg.uIndex;
        auto     surfHandle = m_argIndexSurfMap.find(argIndex);
        VP_PUBLIC_CHK_NOT_FOUND_RETURN(surfHandle, &m_argIndexSurfMap);
        SURFACE_PARAMS &surfParam = surfHandle->second;
        SurfaceType     surfType  = surfParam.surfType;
        if (surfParam.planeIndex != 0 || surfType == SurfaceTypeSubPlane || surfType == SurfaceTypeInvalid)
        {
            VP_RENDER_NORMALMESSAGE("Will skip bindless surface argIndex %d for its planeIndex is set as %d, surfType %d", argIndex, surfParam.planeIndex, surfType);
            continue;
        }
        if (m_surfaceState.find(surfType) != m_surfaceState.end())
        {
            continue;
        }

        VP_PUBLIC_CHK_STATUS_RETURN(GetKernelSurfaceParam(surfParam, kernelSurfaceParam));

        m_surfaceState.insert(std::make_pair(surfType, kernelSurfaceParam));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHDRLITEKernel::GetWalkerSetting(KERNEL_WALKER_PARAMS &walkerParam, KERNEL_PACKET_RENDER_DATA &renderData)
{
    VP_FUNC_CALL();

    for (auto &handle : m_kernelArgs)
    {
        KRN_ARG &arg = handle.second;
        if (arg.eArgKind == ARG_KIND_INLINE)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(SetInlineDataParameter(arg, m_inlineData.data()));
        }
    }

    walkerParam = m_walkerParam;

    walkerParam.iBindingTable = renderData.bindingTable;
    walkerParam.iMediaID      = renderData.mediaID;
    walkerParam.iCurbeOffset  = renderData.iCurbeOffset;
    // Should use renderData.iCurbeLength instead of kernelSettings.CURBE_Length.
    // kernelSettings.CURBE_Length is 32 aligned with 5 bits shift.
    // renderData.iCurbeLength is RENDERHAL_CURBE_BLOCK_ALIGN(64) aligned.
    walkerParam.iCurbeLength = renderData.iCurbeLength;

    walkerParam.curbeResourceList      = m_curbeResourceList.data();
    walkerParam.curbeResourceListSize  = m_curbeResourceList.size();
    walkerParam.inlineResourceList     = m_inlineResourceList.data();
    walkerParam.inlineResourceListSize = m_inlineResourceList.size();

    return MOS_STATUS_SUCCESS;
}

// Only for Adv kernels.
MOS_STATUS VpRenderHDRLITEKernel::SetWalkerSetting(KERNEL_THREAD_SPACE &threadSpace, bool bSyncFlag, bool flushL1)
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

    m_walkerParam.inlineDataLength = m_inlineData.size();
    m_walkerParam.inlineData       = m_inlineData.data();

    m_walkerParam.slmSize    = m_kernelEnv.uiSlmSize;
    m_walkerParam.hasBarrier = (m_kernelEnv.uBarrierCount > 0);

    m_walkerParam.isEmitInlineParameter = m_kernelEnv.uInlineDataPayloadSize > 0;
    
    if (m_kernelEnv.uSimdSize != 1 && m_kernelPerThreadArgInfo.localIdSize > 0)
    {
        m_walkerParam.isGenerateLocalID     = true;
        m_walkerParam.emitLocal             = MHW_EMIT_LOCAL_XYZ;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHDRLITEKernel::SetKernelConfigs(KERNEL_CONFIGS &kernelConfigs)
{
    VP_FUNC_CALL();

    //Skip config

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHDRLITEKernel::SetPerfTag()
{
    auto pOsInterface = m_hwInterface->m_osInterface;
    VP_RENDER_CHK_NULL_RETURN(pOsInterface);
    VP_RENDER_CHK_NULL_RETURN(pOsInterface->pfnSetPerfTag);

    //skip perf tag
    
    return MOS_STATUS_SUCCESS;
}