/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     media_context.cpp
//! \brief    Defines the common interface for media context
//! \details  The media context interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#include "media_context.h"
#include "mos_interface.h"
#include "mos_os_virtualengine_next.h"
#if !EMUL
#include "codec_hw_next.h"
#include "decode_scalability_defs.h"
#endif
#include "mos_os_cp_interface_specific.h"
MediaContext::MediaContext(uint8_t componentType, void *hwInterface, PMOS_INTERFACE osInterface)
{
    if (!hwInterface)
    {
        MOS_OS_ASSERTMESSAGE("null HW interface, failed to create Media Context");
        return;
    }

    if (!osInterface)
    {
        MOS_OS_ASSERTMESSAGE("null OS interface, failed to create Media Context");
        return;
    }

    if (componentType >= scalabilityTotal)
    {
        MOS_OS_ASSERTMESSAGE("Invalid component type, failed to create Media Context");
        return;
    }

    m_hwInterface = hwInterface;
    m_osInterface = osInterface;
    m_userSettingPtr = osInterface->pfnGetUserSettingInstance(osInterface);
    m_componentType = componentType;

    m_streamId = m_osInterface->streamIndex;
    m_gpuContextAttributeTable.clear();
}

MediaContext::~MediaContext()
{
    if (m_osInterface && m_osInterface->pfnWaitAllCmdCompletion)
    {
        m_osInterface->pfnWaitAllCmdCompletion(m_osInterface);
    }

    if ((MOS_VE_MULTINODESCALING_SUPPORTED(m_osInterface)) &&
       (m_curNodeOrdinal == MOS_GPU_NODE_VIDEO || m_curNodeOrdinal == MOS_GPU_NODE_VIDEO2))
    {
        m_osInterface->pfnDestroyVideoNodeAssociation(m_osInterface, m_curNodeOrdinal);
        if (m_osInterface->osStreamState && m_osInterface->osStreamState->component == COMPONENT_Encode)
        {
            m_osInterface->pfnSetLatestVirtualNode(m_osInterface, MOS_GPU_NODE_MAX);
        }
    }

    for (auto& curAttribute : m_gpuContextAttributeTable)
    {
        if (curAttribute.scalabilityState)
        {
            MOS_STATUS status = curAttribute.scalabilityState->Destroy();
            if (MOS_FAILED(status))
            {
                MOS_OS_ASSERTMESSAGE("scalabilityState destroy fail.");
            }
            MOS_Delete(curAttribute.scalabilityState);
            // set legacy MOS ve interface to null to be compatible to legacy MOS
            if (m_osInterface)
            {
                m_osInterface->pVEInterf = nullptr;
            }
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("scalabilityState is nullptr, something must be wrong");
            return;
        }

        if (!m_osInterface || !m_osInterface->pOsContext)
        {
            MOS_OS_ASSERTMESSAGE("m_osInterface and OsContext cannot be nullptr");
            return;
        }

        if (curAttribute.gpuContext != MOS_GPU_CONTEXT_INVALID_HANDLE)
        {
            if (m_osInterface->apoMosEnabled || m_osInterface->modularizedGpuCtxEnabled)
            {
                auto status = m_osInterface->pfnDestroyGpuContextByHandle(m_osInterface, curAttribute.gpuContext);

                if (status != MOS_STATUS_SUCCESS)
                {
                    MOS_OS_NORMALMESSAGE("Gpu Context destory failed, something must be wrong");
                    return;
                }
            }
            else
            {
                auto status = m_osInterface->pfnDestroyGpuContext(
                    m_osInterface,
                    curAttribute.ctxForLegacyMos);
                if (status != MOS_STATUS_SUCCESS)
                {
                    MOS_OS_NORMALMESSAGE("Gpu Context destory failed, something must be wrong");
                    return;
                }
            }
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Invalid gpu Context handle in entry, something must be wrong");
            return;
        }
#if !EMUL
        // Be compatible to legacy MOS
        m_osInterface->pfnSetGpuContextHandle(m_osInterface, MOS_GPU_CONTEXT_INVALID_HANDLE, curAttribute.ctxForLegacyMos);
#endif
    }

    m_gpuContextAttributeTable.clear();
}

MOS_STATUS MediaContext::SwitchContext(MediaFunction func, ContextRequirement *requirement, MediaScalability **scalabilityState)
{
    MOS_OS_FUNCTION_ENTER;
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL_RETURN(m_osInterface);
    MOS_OS_CHK_NULL_RETURN(m_osInterface->pOsContext);
    MOS_OS_CHK_NULL_RETURN(scalabilityState);
    MOS_OS_CHK_NULL_RETURN(requirement);

    if (func >= INVALID_MEDIA_FUNCTION)
    {
        MOS_OS_ASSERTMESSAGE("Func required is invalid");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MediaScalability * veStateProvided = nullptr;

    if (IS_RENDER_ENGINE_FUNCTION(func))
    {
        auto scalPars = (ScalabilityPars *)requirement;
        MOS_OS_CHK_NULL_RETURN(scalPars);
        MOS_OS_CHK_NULL_RETURN(m_osInterface->osCpInterface);
        scalPars->raMode = m_osInterface->osCpInterface->IsHMEnabled() ? MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrRAMode) : 0;
        scalPars->protectMode = m_osInterface->osCpInterface->IsHMEnabled() ? MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrProtectedEnableBitRequired) : 0;
        if (scalPars->raMode)
        {
            MOS_OS_NORMALMESSAGE("request RA mode context for protected render workload");
            ReportUserSetting(m_userSettingPtr,
                              __MEDIA_USER_FEATURE_VALUE_RA_MODE_ENABLE,
                              uint32_t(1),
                              MediaUserSetting::Group::Device);
        }
        if (scalPars->protectMode)
        {
            MOS_OS_NORMALMESSAGE("request protect mode context for protected render workload");
            ReportUserSetting(m_userSettingPtr,
                              __MEDIA_USER_FEATURE_VALUE_PROTECT_MODE_ENABLE,
                              uint32_t(1),
                              MediaUserSetting::Group::Device);
        }
    }

    uint32_t index = m_invalidContextAttribute;
#if !EMUL
    MOS_OS_CHK_STATUS_RETURN(SearchContext<ScalabilityPars*>(func, (ScalabilityPars*)requirement, index));
#endif
    if (index == m_invalidContextAttribute)
    {
        MOS_OS_CHK_STATUS_RETURN(CreateContext<ScalabilityPars*>(func, (ScalabilityPars*)requirement, index));
    }
    if (index == m_invalidContextAttribute || index >= m_gpuContextAttributeTable.size())
    {
        MOS_OS_ASSERTMESSAGE("Incorrect index get from Context attribute table");
        return MOS_STATUS_UNKNOWN;
    }

    // Be compatible to legacy MOS
    MOS_OS_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(m_osInterface, m_gpuContextAttributeTable[index].ctxForLegacyMos));
    veStateProvided = m_gpuContextAttributeTable[index].scalabilityState;
    MOS_OS_NORMALMESSAGE("Switched to GpuContext %d, index %d", m_gpuContextAttributeTable[index].ctxForLegacyMos, index);

    if (requirement->IsEnc)
    {
        m_osInterface->pfnSetEncodeEncContext(m_osInterface, m_gpuContextAttributeTable[index].ctxForLegacyMos);
    }
    if (requirement->IsPak)
    {
        m_osInterface->pfnSetEncodePakContext(m_osInterface, m_gpuContextAttributeTable[index].ctxForLegacyMos);
    }

    if (!requirement->IsContextSwitchBack)
    {
        m_osInterface->pfnResetOsStates(m_osInterface);
    }

    *scalabilityState = veStateProvided;

    return status;
}

template<typename T>
MOS_STATUS MediaContext::SearchContext(MediaFunction func, T params, uint32_t& indexFound)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(m_osInterface);

    indexFound = m_invalidContextAttribute;
    uint32_t index = 0;

    for (auto& curAttribute : m_gpuContextAttributeTable)
    {
        if (curAttribute.func == func)
        {
            MOS_OS_CHK_NULL_RETURN(curAttribute.scalabilityState);

            if (curAttribute.scalabilityState->IsScalabilityModeMatched(params))
            {
                // Found the matching GPU context and scalability state
                indexFound = index;

                // Be compatible to legacy MOS
                MOS_OS_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContextHandle(m_osInterface, curAttribute.gpuContext, curAttribute.ctxForLegacyMos));
                // set legacy MOS ve interface to current reused scalability state's ve interface
                m_osInterface->pVEInterf = curAttribute.scalabilityState->m_veInterface;
                if (m_osInterface->apoMosEnabled || m_osInterface->apoMosForLegacyRuntime)
                {
                    if (curAttribute.scalabilityState->m_veState)
                    {
                        MOS_OS_CHK_NULL_RETURN(m_osInterface->osStreamState);
                        m_osInterface->osStreamState->virtualEngineInterface = curAttribute.scalabilityState->m_veState;
                    }
                }
                break;
            }
        }

        index++;
    }

    return MOS_STATUS_SUCCESS;
}

template<typename T>
MOS_STATUS MediaContext::CreateContext(MediaFunction func, T params, uint32_t& indexReturn)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(m_osInterface);

    // Reach the max number of Gpu Context attr entries in current Media Context
    if (m_gpuContextAttributeTable.size() == m_maxContextAttribute)
    {
        MOS_OS_ASSERTMESSAGE("Reached max num of entries of gpuContextAttributeTable: 4096. Cannot create more Gpu Contexts");
        return MOS_STATUS_NOT_ENOUGH_BUFFER;
    }

    GpuContextAttribute newAttr;

    // Setup func
    newAttr.func = func;
    if (newAttr.func >= INVALID_MEDIA_FUNCTION)
    {
        MOS_OS_ASSERTMESSAGE("Func required is invalid");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Create scalabilityState
    MOS_GPUCTX_CREATOPTIONS_ENHANCED option;
    MediaScalabilityFactory<T> scalabilityFactory;
    newAttr.scalabilityState = scalabilityFactory.CreateScalability(
        m_componentType, params, m_hwInterface, this, &option);
    if (newAttr.scalabilityState == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to create scalability state");
        return MOS_STATUS_NO_SPACE;
    }

    // request to create or reuse gpuContext
    MOS_GPU_NODE node = MOS_GPU_NODE_MAX;
    MOS_OS_CHK_STATUS_RETURN(FunctionToNode(func, option, node));

    // WA for legacy MOS
    MOS_OS_CHK_STATUS_RETURN(FunctionToGpuContext(func, option, node, newAttr.ctxForLegacyMos));

    if(m_osInterface->bSetHandleInvalid)
    {
        MOS_OS_NORMALMESSAGE("Force set INVALID_HANDLE for GpuContext %d", newAttr.ctxForLegacyMos);
        MOS_OS_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContextHandle(m_osInterface, MOS_GPU_CONTEXT_INVALID_HANDLE, newAttr.ctxForLegacyMos));
    }

    MOS_OS_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(m_osInterface, newAttr.ctxForLegacyMos, node, &option));
    m_osInterface->pfnSetGpuContext(m_osInterface, newAttr.ctxForLegacyMos);
    newAttr.gpuContext = m_osInterface->CurrentGpuContextHandle;
    MOS_OS_NORMALMESSAGE("Create GpuContext %d, node %d, handle 0x%x, protectMode %d, raMode %d",
        newAttr.ctxForLegacyMos, node, newAttr.gpuContext, option.ProtectMode, option.RAMode);

    // Add entry to the table
    indexReturn = m_gpuContextAttributeTable.size();
    m_gpuContextAttributeTable.push_back(newAttr);

    if (func == VeboxVppFunc || func == ComputeVppFunc)
    {
         m_osInterface->pfnRegisterBBCompleteNotifyEvent(
            m_osInterface,
            newAttr.ctxForLegacyMos);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaContext::SwitchContext(
    MediaFunction func,
    MediaScalabilityOption &scalabilityOption,
    MediaScalability **scalabilityState,
    bool isEnc,
    bool isPak)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(m_osInterface);
    MOS_OS_CHK_NULL_RETURN(m_osInterface->pOsContext);
    MOS_OS_CHK_NULL_RETURN(scalabilityState);

    if (func >= INVALID_MEDIA_FUNCTION)
    {
        MOS_OS_ASSERTMESSAGE("Func required is invalid");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MediaScalability * veStateProvided = nullptr;

    uint32_t index = m_invalidContextAttribute;
#if !EMUL
    MOS_OS_CHK_STATUS_RETURN(SearchContext<MediaScalabilityOption&>(func, scalabilityOption, index));
#endif
    if (index == m_invalidContextAttribute)
    {
        MOS_OS_CHK_STATUS_RETURN(CreateContext<MediaScalabilityOption *>(func, &scalabilityOption, index));
    }
    if (index == m_invalidContextAttribute || index >= m_gpuContextAttributeTable.size())
    {
        MOS_OS_ASSERTMESSAGE("Incorrect index get from Context attribute table");
        return MOS_STATUS_UNKNOWN;
    }

    // Be compatible to legacy MOS
    MOS_OS_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(m_osInterface, m_gpuContextAttributeTable[index].ctxForLegacyMos));
    veStateProvided = m_gpuContextAttributeTable[index].scalabilityState;

    if (isEnc)
    {
        m_osInterface->pfnSetEncodeEncContext(m_osInterface, m_gpuContextAttributeTable[index].ctxForLegacyMos);
    }
    if (isPak)
    {
        m_osInterface->pfnSetEncodePakContext(m_osInterface, m_gpuContextAttributeTable[index].ctxForLegacyMos);
    }

    m_osInterface->pfnResetOsStates(m_osInterface);

    *scalabilityState = veStateProvided;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaContext::FunctionToNode(MediaFunction func, const MOS_GPUCTX_CREATOPTIONS_ENHANCED &option, MOS_GPU_NODE& node)
{
    MOS_OS_FUNCTION_ENTER;
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    if (func >= INVALID_MEDIA_FUNCTION)
    {
        MOS_OS_ASSERTMESSAGE("Func required is invalid");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    switch (func)
    {
    case RenderGenericFunc:
        node = MOS_GPU_NODE_3D;
        break;
    case VdboxEncodeFunc:
    case VdboxDecodeFunc:
    case VdboxDecodeVirtualNode0Func:
    case VdboxDecodeVirtualNode1Func:
        if (option.LRCACount >= 2)
        {
            node = MOS_GPU_NODE_VIDEO; // multiple pipe decode or encode always using VIDEO node
            if (MOS_VE_MULTINODESCALING_SUPPORTED(m_osInterface))
            {
                if (m_curNodeOrdinal == MOS_GPU_NODE_VIDEO || m_curNodeOrdinal == MOS_GPU_NODE_VIDEO2)
                {
                    m_osInterface->pfnDestroyVideoNodeAssociation(m_osInterface, m_curNodeOrdinal);
                    if (func == VdboxEncodeFunc)
                    {
                        m_osInterface->pfnSetLatestVirtualNode(m_osInterface, MOS_GPU_NODE_MAX);
                    }
                }
                MOS_OS_CHK_STATUS_RETURN(m_osInterface->pfnCreateVideoNodeAssociation(
                    m_osInterface,
                    true,
                    &node));
                if (func == VdboxDecodeFunc || func == VdboxDecodeVirtualNode0Func || func == VdboxDecodeVirtualNode1Func)
                {
                    m_osInterface->pfnSetDecoderVirtualNodePerStream(m_osInterface, node);
                }
                else if (func == VdboxEncodeFunc)
                {
                    m_osInterface->pfnSetLatestVirtualNode(m_osInterface, node);
                }
            }
            m_curNodeOrdinal = node;
        }
#if !EMUL
        else
        {
            MOS_OS_CHK_STATUS_RETURN(FunctionToNodeCodec(func, node));
        }
#endif
        break;
    case VdboxDecodeWaFunc:
    case VdboxDecrpytFunc:
    case VdboxCpFunc:
        node = MOS_GPU_NODE_VIDEO;
        break;
    case VeboxVppFunc:
        node = MOS_GPU_NODE_VE;
        break;
    case ComputeMdfFunc:
    case ComputeVppFunc:
        node = MOS_GPU_NODE_COMPUTE;
        break;
    default:
        MOS_OS_ASSERTMESSAGE("Cannot find the GPU node by the func");
        status = MOS_STATUS_INVALID_PARAMETER;
        node = MOS_GPU_NODE_MAX;
        break;
    }

    return status;
}
#if !EMUL
MOS_STATUS MediaContext::FunctionToNodeCodec(MediaFunction func, MOS_GPU_NODE& node)
{
    MHW_VDBOX_GPUNODE_LIMIT gpuNodeLimit = {0};

    MOS_OS_CHK_STATUS_RETURN(FindGpuNodeToUse(func , &gpuNodeLimit));

    node = (MOS_GPU_NODE)(gpuNodeLimit.dwGpuNodeToUse);

    return MOS_STATUS_SUCCESS;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS MediaContext::CheckScalabilityOverrideValidity()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MEDIA_SYSTEM_INFO *gtSystemInfo = nullptr;
    uint32_t           forceVdbox   = 0;
    bool               scalableDecMode = false;
    bool               useVD1          = false;
    bool               useVD2          = false;

    MHW_MI_CHK_NULL(m_osInterface);
    scalableDecMode = m_osInterface->bHcpDecScalabilityMode ? true : false;
    forceVdbox      = m_osInterface->eForceVdbox;
    gtSystemInfo    = m_osInterface->pfnGetGtSystemInfo(m_osInterface);
    MHW_MI_CHK_NULL(gtSystemInfo);

    if (forceVdbox != MOS_FORCE_VDBOX_NONE &&
        forceVdbox != MOS_FORCE_VDBOX_1 &&
        forceVdbox != MOS_FORCE_VDBOX_2 &&
        // 2 pipes, VDBOX1-BE1, VDBOX2-BE2
        forceVdbox != MOS_FORCE_VDBOX_1_1_2 &&
        forceVdbox != MOS_FORCE_VDBOX_2_1_2)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("user feature forceVdbox value is invalid.");
        return eStatus;
    }
    if (!scalableDecMode &&
        (forceVdbox == MOS_FORCE_VDBOX_1_1_2 ||
            forceVdbox == MOS_FORCE_VDBOX_2_1_2))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("user feature forceVdbox valude does not consistent with regkey scalability mode.");
        return eStatus;
    }

    if (scalableDecMode && !m_scalabilitySupported)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("user feature scalability mode is not allowed on current platform!");
        return eStatus;
    }

    if (forceVdbox == 0)
    {
        useVD1 = true;
    }
    else
    {
        MHW_VDBOX_IS_VDBOX_SPECIFIED(forceVdbox, MOS_FORCE_VDBOX_1, MOS_FORCEVDBOX_VDBOXID_BITSNUM, MOS_FORCEVDBOX_MASK, useVD1);
        MHW_VDBOX_IS_VDBOX_SPECIFIED(forceVdbox, MOS_FORCE_VDBOX_2, MOS_FORCEVDBOX_VDBOXID_BITSNUM, MOS_FORCEVDBOX_MASK, useVD2);
    }
    if (!gtSystemInfo->VDBoxInfo.IsValid ||
        (useVD1 && !gtSystemInfo->VDBoxInfo.Instances.Bits.VDBox0Enabled) ||
        (useVD2 && !gtSystemInfo->VDBoxInfo.Instances.Bits.VDBox1Enabled))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("the forced VDBOX is not enabled in current platform.");
        return eStatus;
    }

    return eStatus;
}
#endif

MOS_STATUS MediaContext::FindGpuNodeToUse(MediaFunction func, PMHW_VDBOX_GPUNODE_LIMIT gpuNodeLimit)
{
    bool       setVideoNode = false;
    MOS_STATUS eStatus      = MOS_STATUS_SUCCESS;

    MOS_GPU_NODE videoGpuNode = MOS_GPU_NODE_VIDEO;

    if (MOS_VE_MULTINODESCALING_SUPPORTED(m_osInterface))
    {
        if (GetNumVdbox() == 1)
        {
            videoGpuNode = MOS_GPU_NODE_VIDEO;
        }
        else
        {
            if (m_curNodeOrdinal == MOS_GPU_NODE_VIDEO || m_curNodeOrdinal == MOS_GPU_NODE_VIDEO2)
            {
                m_osInterface->pfnDestroyVideoNodeAssociation(m_osInterface, m_curNodeOrdinal);
                if (func == VdboxEncodeFunc)
                {
                    m_osInterface->pfnSetLatestVirtualNode(m_osInterface, MOS_GPU_NODE_MAX);
                }
            }
            if (func == VdboxDecodeVirtualNode0Func)
            {
                setVideoNode = true;
                videoGpuNode = MOS_GPU_NODE_VIDEO;
            }
            else if (func == VdboxDecodeVirtualNode1Func)
            {
                setVideoNode = true;
                videoGpuNode = MOS_GPU_NODE_VIDEO2;
            }
            else if (func == VdboxEncodeFunc)
            {
                MOS_GPU_NODE node = m_osInterface->pfnGetLatestVirtualNode(m_osInterface, COMPONENT_Decode);
                if (node != MOS_GPU_NODE_MAX)
                {
                    setVideoNode = true;
                    videoGpuNode = (node == MOS_GPU_NODE_VIDEO) ? MOS_GPU_NODE_VIDEO2 : MOS_GPU_NODE_VIDEO;
                }
            }
            MHW_MI_CHK_STATUS(m_osInterface->pfnCreateVideoNodeAssociation(
                m_osInterface,
                setVideoNode,
                &videoGpuNode));
            m_curNodeOrdinal = videoGpuNode;
            if (func == VdboxDecodeFunc || func == VdboxDecodeVirtualNode0Func || func == VdboxDecodeVirtualNode1Func)
            {
                m_osInterface->pfnSetDecoderVirtualNodePerStream(m_osInterface, m_curNodeOrdinal);
            }
            else if (func == VdboxEncodeFunc)
            {
                m_osInterface->pfnSetLatestVirtualNode(m_osInterface, m_curNodeOrdinal);
            }
        }
    }
#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_osInterface != nullptr && m_osInterface->bEnableDbgOvrdInVE &&
        (!m_osInterface->bSupportVirtualEngine || !m_scalabilitySupported))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("not support DebugOverrid on current OS or Platform.");
        return eStatus;
    }

    if (m_osInterface != nullptr && m_osInterface->bEnableDbgOvrdInVE)
    {
        MHW_MI_CHK_STATUS(CheckScalabilityOverrideValidity());
    }
#endif
    gpuNodeLimit->dwGpuNodeToUse = videoGpuNode;

    return eStatus;
}
#endif

MOS_STATUS MediaContext::FunctionToGpuContext(MediaFunction func, const MOS_GPUCTX_CREATOPTIONS_ENHANCED &option, const MOS_GPU_NODE &node, MOS_GPU_CONTEXT &ctx)
{
    MOS_OS_FUNCTION_ENTER;

    if (func >= INVALID_MEDIA_FUNCTION)
    {
        MOS_OS_ASSERTMESSAGE("Func is invalid");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    switch (func)
    {
    case VdboxEncodeFunc:
        MOS_OS_CHK_STATUS_RETURN(FunctionToGpuContextEncode(option, ctx));
        break;
    case VdboxDecodeFunc:
    case VdboxDecodeVirtualNode0Func:
    case VdboxDecodeVirtualNode1Func:
        MOS_OS_CHK_STATUS_RETURN(FunctionToGpuContextDecode(option, node, ctx));
        break;
    case VdboxDecodeWaFunc:
        ctx = MOS_GPU_CONTEXT_VIDEO2;
        break;
    case VdboxDecrpytFunc:
        ctx = MOS_GPU_CONTEXT_VDBOX2_VIDEO2;
        break;
    case VeboxVppFunc:
        ctx = MOS_GPU_CONTEXT_VEBOX;
        break;
    case RenderGenericFunc:
        ctx = option.RAMode ? MOS_GPU_CONTEXT_RENDER_RA : MOS_GPU_CONTEXT_RENDER;
        break;
    case ComputeVppFunc:
        ctx = option.RAMode ? MOS_GPU_CONTEXT_COMPUTE_RA : MOS_GPU_CONTEXT_COMPUTE;
        break;
    case ComputeMdfFunc:
        ctx = MOS_GPU_CONTEXT_CM_COMPUTE;
        break;
    case VdboxCpFunc:
        ctx = MOS_GPU_CONTEXT_VIDEO;
        break;
    default:
        ctx = MOS_GPU_CONTEXT_MAX;
        MOS_OS_ASSERTMESSAGE("Func is invalid");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaContext::FunctionToGpuContextDecode(const MOS_GPUCTX_CREATOPTIONS_ENHANCED &option, const MOS_GPU_NODE &node, MOS_GPU_CONTEXT &ctx)
{
    if (option.UsingSFC)
    {
        ctx = MOS_GPU_CONTEXT_VIDEO4;
    }
    else
    {
        switch (option.LRCACount)
        {
        case 0:
        case 1:
            ctx = (node == MOS_GPU_NODE_VIDEO) ? MOS_GPU_CONTEXT_VIDEO : MOS_GPU_CONTEXT_VDBOX2_VIDEO;
            break;
        case 2:
            ctx = MOS_GPU_CONTEXT_VIDEO5;
            break;
        case 3:
            ctx = MOS_GPU_CONTEXT_VIDEO7;
            break;
        default:
            ctx = MOS_GPU_CONTEXT_VIDEO;
            break;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaContext::FunctionToGpuContextEncode(const MOS_GPUCTX_CREATOPTIONS_ENHANCED &option, MOS_GPU_CONTEXT &ctx)
{
    switch (option.LRCACount)
    {
    case 0:
    case 1:
        ctx = MOS_GPU_CONTEXT_VIDEO3;
        break;
    case 2:
        ctx = MOS_GPU_CONTEXT_VIDEO6;
        break;
    case 4:
        ctx = MOS_GPU_CONTEXT_VIDEO6;  //Change to proper slot when MOS_GPU_CONTEXT num extended
        break;
    default:
        ctx = MOS_GPU_CONTEXT_VIDEO3;
        break;
    }

    return MOS_STATUS_SUCCESS;
}

void MediaContext::SetLatestDecoderVirtualNode()
{
    if (m_curNodeOrdinal == MOS_GPU_NODE_VIDEO || m_curNodeOrdinal == MOS_GPU_NODE_VIDEO2)
    {
        m_osInterface->pfnSetLatestVirtualNode(m_osInterface, m_curNodeOrdinal);
    }
}

MOS_STATUS MediaContext::ReassignContextForDecoder(uint32_t frameNum, ContextRequirement *requirement, MediaScalability **scalabilityState)
{
    if (frameNum == 0)
    {
        // if encoder is working, always reassign decoder to different virtual node
        MOS_GPU_NODE encoderNode = m_osInterface->pfnGetLatestVirtualNode(m_osInterface, COMPONENT_Encode);
        if (encoderNode == MOS_GPU_NODE_VIDEO)
        {
            MOS_OS_CHK_STATUS_RETURN(SwitchContext(VdboxDecodeVirtualNode1Func, requirement, scalabilityState));
        }
        else if (encoderNode == MOS_GPU_NODE_VIDEO2)
        {
            MOS_OS_CHK_STATUS_RETURN(SwitchContext(VdboxDecodeVirtualNode0Func, requirement, scalabilityState));
        }
        else
        {
            // if only decoders are working, reassign decoder to different virtual node compared to latest used virtual node
            MOS_GPU_NODE decoderNode = m_osInterface->pfnGetLatestVirtualNode(m_osInterface, COMPONENT_Decode);
            if (decoderNode == MOS_GPU_NODE_VIDEO)
            {
                MOS_OS_CHK_STATUS_RETURN(SwitchContext(VdboxDecodeVirtualNode1Func, requirement, scalabilityState));
            }
            else if (decoderNode == MOS_GPU_NODE_VIDEO2)
            {
                MOS_OS_CHK_STATUS_RETURN(SwitchContext(VdboxDecodeVirtualNode0Func, requirement, scalabilityState));
            }
            else
            {
                MOS_OS_CHK_STATUS_RETURN(SwitchContext(VdboxDecodeFunc, requirement, scalabilityState));
            }
        }
    }
    else
    {
        // for remaining frames, don't change virtual node assignment
        MOS_GPU_NODE decoderNode = m_osInterface->pfnGetDecoderVirtualNodePerStream(m_osInterface);
        if (decoderNode == MOS_GPU_NODE_VIDEO)
        {
            MOS_OS_CHK_STATUS_RETURN(SwitchContext(VdboxDecodeVirtualNode0Func, requirement, scalabilityState));
        }
        else if (decoderNode == MOS_GPU_NODE_VIDEO2)
        {
            MOS_OS_CHK_STATUS_RETURN(SwitchContext(VdboxDecodeVirtualNode1Func, requirement, scalabilityState));
        }
        else
        {
            MOS_OS_CHK_STATUS_RETURN(SwitchContext(VdboxDecodeFunc, requirement, scalabilityState));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaContext::ReassignContextForDecoder(uint32_t frameNum, MediaScalabilityOption &scalabilityOption, MediaScalability **scalabilityState)
{
    if (frameNum == 0)
    {
        // if encoder is working, always reassign decoder to different virtual node
        MOS_GPU_NODE encoderNode = m_osInterface->pfnGetLatestVirtualNode(m_osInterface, COMPONENT_Encode);
        if (encoderNode == MOS_GPU_NODE_VIDEO)
        {
            MOS_OS_CHK_STATUS_RETURN(SwitchContext(VdboxDecodeVirtualNode1Func, scalabilityOption, scalabilityState));
        }
        else if (encoderNode == MOS_GPU_NODE_VIDEO2)
        {
            MOS_OS_CHK_STATUS_RETURN(SwitchContext(VdboxDecodeVirtualNode0Func, scalabilityOption, scalabilityState));
        }
        else
        {
            // if only decoders are working, reassign decoder to different virtual node compared to latest used virtual node
            MOS_GPU_NODE decoderNode = m_osInterface->pfnGetLatestVirtualNode(m_osInterface, COMPONENT_Decode);
            if (decoderNode == MOS_GPU_NODE_VIDEO)
            {
                MOS_OS_CHK_STATUS_RETURN(SwitchContext(VdboxDecodeVirtualNode1Func, scalabilityOption, scalabilityState));
            }
            else if (decoderNode == MOS_GPU_NODE_VIDEO2)
            {
                MOS_OS_CHK_STATUS_RETURN(SwitchContext(VdboxDecodeVirtualNode0Func, scalabilityOption, scalabilityState));
            }
            else
            {
                MOS_OS_CHK_STATUS_RETURN(SwitchContext(VdboxDecodeFunc, scalabilityOption, scalabilityState));
            }
        }
    }
    else
    {
        // for remaining frames, don't change virtual node assignment
        MOS_GPU_NODE decoderNode = m_osInterface->pfnGetDecoderVirtualNodePerStream(m_osInterface);
        if (decoderNode == MOS_GPU_NODE_VIDEO)
        {
            MOS_OS_CHK_STATUS_RETURN(SwitchContext(VdboxDecodeVirtualNode0Func, scalabilityOption, scalabilityState));
        }
        else if (decoderNode == MOS_GPU_NODE_VIDEO2)
        {
            MOS_OS_CHK_STATUS_RETURN(SwitchContext(VdboxDecodeVirtualNode1Func, scalabilityOption, scalabilityState));
        }
        else
        {
            MOS_OS_CHK_STATUS_RETURN(SwitchContext(VdboxDecodeFunc, scalabilityOption, scalabilityState));
        }
    }

    return MOS_STATUS_SUCCESS;
}
