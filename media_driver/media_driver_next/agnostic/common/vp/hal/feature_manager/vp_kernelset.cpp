/* Copyright (c) 2020-2021, Intel Corporation
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

using namespace vp;

VpKernelSet::VpKernelSet(PVP_MHWINTERFACE hwInterface, PVpAllocator allocator) :
    m_hwInterface(hwInterface),
    m_allocator(allocator)
{
    m_pKernelPool = &hwInterface->m_vpPlatformInterface->GetKernelPool();
}

MOS_STATUS VpKernelSet::GetKernelInfo(uint32_t kuid, uint32_t& size, void*& kernel)
{
    Kdll_State* kernelState = m_pKernelPool->at(0).GetKdllState();

    if (kernelState)
    {
        size   = kernelState->ComponentKernelCache.pCacheEntries[kuid].iSize;
        kernel = kernelState->ComponentKernelCache.pCacheEntries[kuid].pBinary;
        return MOS_STATUS_SUCCESS;
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Kernel State not inplenmented, return error");
        return MOS_STATUS_UNINITIALIZED;
    }
}

MOS_STATUS VpKernelSet::FindAndInitKernelObj(VpRenderKernelObj* kernelObj)
{
    VP_RENDER_CHK_NULL_RETURN(kernelObj);
    VP_RENDER_CHK_NULL_RETURN(m_pKernelPool);

    bool bFind = false;
    for (auto &kernel : *m_pKernelPool)
    {
        if (!kernel.GetKernelName().compare(kernelObj->GetKernelName()))
        {
            VP_RENDER_CHK_STATUS_RETURN(kernelObj->Init(kernel));
            bFind = true;
            break;
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
    switch (kernelId)
    {
    case kernelCombinedFc:
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
    KERNEL_OBJECTS& kernelObjs)
{
    VP_FUNC_CALL();

    VpRenderKernelObj* kernel = nullptr;

    auto VpStatusHandler = [&] (MOS_STATUS status)
    {
        if (MOS_FAILED(status))
        {
            if (kernel)
            {
                MOS_Delete(kernel);
            }
        }
        return status;
    };

    for (uint32_t kernelIndex = 0; kernelIndex < kernelParams.size(); ++kernelIndex)
    {

        VP_RENDER_CHK_STATUS_RETURN(CreateSingleKernelObject(
            kernel,
            kernelParams[kernelIndex].kernelId,
            kernelIndex));

        VP_RENDER_CHK_NULL_RETURN(kernel);

        if (!kernel->IsAdvKernel())
        {
            void* binary = nullptr;
            uint32_t kernelSize = 0;

            VP_RENDER_CHK_NULL_RETURN(kernel);

            VP_RENDER_CHK_STATUS_RETURN(VpStatusHandler(GetKernelInfo(kernel->GetKernelBinaryID(), kernelSize, binary)));

            VP_RENDER_CHK_STATUS_RETURN(VpStatusHandler(kernel->InitKernel(binary, kernelSize, kernelConfigs, surfacesGroup)));

            kernelObjs.insert(std::make_pair(kernelIndex, kernel));
        }
        else
        {
            VP_RENDER_CHK_STATUS_RETURN(VpStatusHandler(FindAndInitKernelObj(kernel)));

            VP_RENDER_CHK_STATUS_RETURN(VpStatusHandler(kernel->SetKernelConfigs(kernelParams[kernelIndex], surfacesGroup, samplerStateGroup)));

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