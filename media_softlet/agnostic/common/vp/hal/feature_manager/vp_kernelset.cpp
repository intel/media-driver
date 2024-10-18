/* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     vp_render_kernelset.cpp
//! \brief    The header file of the base class of kernel set
//! \details  The kernel set will include kernel generation from binary.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!
#include "vp_kernelset.h"
#include "vp_render_fc_kernel.h"
#include "vp_render_ocl_fc_kernel.h"
#include "vp_render_vebox_hdr_3dlut_kernel.h"
#include "vp_render_vebox_hvs_kernel.h"
#include "vp_render_hdr_kernel.h"
#include "vp_render_vebox_hdr_3dlut_l0_kernel.h"

using namespace vp;

VpKernelSet::VpKernelSet(PVP_MHWINTERFACE hwInterface, PVpAllocator allocator) :
    m_hwInterface(hwInterface),
    m_allocator(allocator)
{
    m_pKernelPool = &hwInterface->m_vpPlatformInterface->GetKernelPool();
}

MOS_STATUS VpKernelSet::GetKernelInfo(std::string kernelName, uint32_t kuid, uint32_t& size, void*& kernel)
{
    VP_FUNC_CALL();

    auto it = m_pKernelPool->find(kernelName);

    if (m_pKernelPool->end() == it)
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    Kdll_State* kernelState = it->second.GetKdllState();

    if (kernelState)
    {
        // For FC case, the kernel binary and size are gotten during GetKernelEntry.
        if (IDR_VP_EOT == kuid)
        {
            size = 0;
            kernel = nullptr;
        }
        else
        {
            size   = kernelState->ComponentKernelCache.pCacheEntries[kuid].iSize;
            kernel = kernelState->ComponentKernelCache.pCacheEntries[kuid].pBinary;
        }
        return MOS_STATUS_SUCCESS;
    }
    else if (kernelName == VpRenderKernel::s_kernelNameNonAdvKernels)
    {
        VP_PUBLIC_ASSERTMESSAGE("Kernel State not inplenmented, return error");
        return MOS_STATUS_UNINITIALIZED;
    }
    else
    {
        size = it->second.GetKernelSize();
        kernel = (void*)it->second.GetKernelBinPointer();
        return MOS_STATUS_SUCCESS;
    }
}

// For Adv kernel
MOS_STATUS VpKernelSet::FindAndInitKernelObj(VpRenderKernelObj* kernelObj)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(kernelObj);
    VP_RENDER_CHK_NULL_RETURN(m_pKernelPool);

    bool bFind = false;
    VP_RENDER_CHK_STATUS_RETURN(m_hwInterface->m_vpPlatformInterface->InitializeDelayedKernels(kernelObj->GetKernelType()));

    if (m_pKernelPool)
    {
        auto it = m_pKernelPool->find(kernelObj->GetKernelName());
        if (m_pKernelPool->end() != it)
        {
            auto &kernel = it->second;
            VP_RENDER_CHK_STATUS_RETURN(kernelObj->Init(kernel));
            bFind = true;
        }
    }

    if (!bFind)
    {
        VP_RENDER_ASSERTMESSAGE("The kernel is not found!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpKernelSet::CreateSingleKernelObject(
    VpRenderKernelObj *&kernel,
    VpKernelID kernelId,
    KernelIndex kernelIndex)
{
    VP_FUNC_CALL();
    kernel = nullptr;
    switch ((uint32_t)kernelId)
    {
    case kernelCombinedFc:
        kernel = (VpRenderKernelObj*)MOS_New(VpRenderFcKernel, m_hwInterface, m_allocator);
        VP_RENDER_CHK_NULL_RETURN(kernel);
        break;
    case kernelOclFcCommon:
    case kernelOclFcFP:
    case kernelOclFc420PL3Input:
    case kernelOclFc420PL3Output:
    case kernelOclFc444PL3Input:
    case kernelOclFc444PL3Output:
        kernel = (VpRenderKernelObj *)MOS_New(VpRenderOclFcKernel, m_hwInterface, kernelId, kernelIndex, m_allocator);
        VP_RENDER_CHK_NULL_RETURN(kernel);
        break;
    case kernelHdr3DLutCalc:
        if (m_pKernelPool->find(VP_HDR_KERNEL_NAME_L0) != m_pKernelPool->end())
        {
            VP_RENDER_NORMALMESSAGE("HDR 3dlut kernel use l0 hdr_3dlut_l0 kernel");
            kernel = (VpRenderKernelObj *)MOS_New(VpRenderHdr3DLutKernel, m_hwInterface, m_allocator);
        }
        else
        {
            VP_RENDER_NORMALMESSAGE("HDR 3dlut kernel use isa hdr_3dlut kernel");
            kernel = (VpRenderKernelObj *)MOS_New(VpRenderHdr3DLutKernelCM, m_hwInterface, kernelId, kernelIndex, m_allocator);
        }
        VP_RENDER_CHK_NULL_RETURN(kernel);
        break;
    case kernelHdr3DLutCalcL0:
        VP_RENDER_NORMALMESSAGE("HDR 3dlut kernel use l0 fillLutTable_3dlut kernel");
        kernel = (VpRenderKernelObj *)MOS_New(VpRenderHdr3DLutL0Kernel, m_hwInterface, m_allocator);
        VP_RENDER_CHK_NULL_RETURN(kernel);
        break;
    case kernelHVSCalc:
        kernel = (VpRenderKernelObj *)MOS_New(VpRenderHVSKernel, m_hwInterface, kernelId, kernelIndex, m_allocator);
        VP_RENDER_CHK_NULL_RETURN(kernel);
        break;
    case kernelHdrMandatory:
        kernel = (VpRenderKernelObj *)MOS_New(VpRenderHdrKernel, m_hwInterface, m_allocator);
        VP_RENDER_CHK_NULL_RETURN(kernel);
        break;
    default:
        VP_RENDER_ASSERTMESSAGE("No supported kernel, return");
        return MOS_STATUS_UNIMPLEMENTED;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpKernelSet::CreateKernelObjects(
    KERNEL_PARAMS_LIST& kernelParams,
    VP_SURFACE_GROUP& surfacesGroup,
    KERNEL_SAMPLER_STATE_GROUP& samplerStateGroup,
    KERNEL_CONFIGS& kernelConfigs,
    KERNEL_OBJECTS& kernelObjs,
    VP_RENDER_CACHE_CNTL& surfMemCacheCtl,
    VP_PACKET_SHARED_CONTEXT *sharedContext)
{
    VP_FUNC_CALL();

    VpRenderKernelObj* kernel = nullptr;

    auto VpStatusHandler = [&] (MOS_STATUS status)
    {
        if (MOS_FAILED(status))
        {
            if (kernel)
            {
                auto it = m_cachedKernels.find(kernel->GetKernelId());
                if (it != m_cachedKernels.end() && it->second == kernel)
                {
                    m_cachedKernels.erase(it);
                }
                MOS_Delete(kernel);
            }
        }
        return status;
    };

    kernelObjs.clear();

    for (uint32_t kernelIndex = 0; kernelIndex < kernelParams.size(); ++kernelIndex)
    {
        auto it = m_cachedKernels.find(kernelParams[kernelIndex].kernelId);
        if (m_cachedKernels.end() == it)
        {
            VP_RENDER_CHK_STATUS_RETURN(CreateSingleKernelObject(
                kernel,
                kernelParams[kernelIndex].kernelId,
                kernelIndex));
            if (kernel->IsKernelCached())
            {
                m_cachedKernels.insert(std::make_pair(kernelParams[kernelIndex].kernelId, kernel));
            }
        }
        else
        {
            kernel = it->second;
        }

        VP_RENDER_CHK_NULL_RETURN(kernel);

        if (!kernel->IsAdvKernel())
        {
            void* binary = nullptr;
            uint32_t kernelSize = 0;

            VP_RENDER_CHK_NULL_RETURN(kernel);

            VP_RENDER_CHK_STATUS_RETURN(VpStatusHandler(GetKernelInfo(kernel->GetKernelName(), kernel->GetKernelBinaryID(), kernelSize, binary)));

            VP_RENDER_CHK_STATUS_RETURN(VpStatusHandler(kernel->InitKernel(binary, kernelSize, kernelConfigs, surfacesGroup, surfMemCacheCtl)));

            kernelObjs.insert(std::make_pair(kernelIndex, kernel));
        }
        else
        {
            VP_RENDER_CHK_STATUS_RETURN(VpStatusHandler(FindAndInitKernelObj(kernel)));

            VP_RENDER_CHK_STATUS_RETURN(VpStatusHandler(kernel->SetKernelConfigs(kernelParams[kernelIndex], surfacesGroup, samplerStateGroup, kernelConfigs, sharedContext)));

            kernelObjs.insert(std::make_pair(kernelIndex, kernel));
        }
    }

    kernelParams.clear();

    if (kernelObjs.empty())
    {
        VP_RENDER_ASSERTMESSAGE("Kernel Object Fail, return Error");
        return MOS_STATUS_NULL_POINTER;
    }

    return MOS_STATUS_SUCCESS;
}