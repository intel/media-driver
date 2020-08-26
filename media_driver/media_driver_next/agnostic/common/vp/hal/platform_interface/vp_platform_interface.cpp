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
//! \file     vp_platform_interface.cpp
//! \brief    render packet which used in by mediapipline.
//! \details  render packet provide the structures and generate the cmd buffer which mediapipline will used.
//!
#include "vp_platform_interface.h"
using namespace vp;

MOS_STATUS VpRenderKernel::InitVPKernel(
    const Kdll_RuleEntry *kernelRules,
    const uint32_t       *kernelBin,
    uint32_t              kernelSize,
    const uint32_t       *patchKernelBin,
    uint32_t              patchKernelSize,
    void(*ModifyFunctionPointers)(PKdll_State) = nullptr)
{
    m_kernelDllRules = kernelRules;
    m_kernelBin = (const void*)kernelBin;
    m_kernelBinSize = kernelSize;
    m_fcPatchBin = (const void*)patchKernelBin;
    m_fcPatchBinSize = patchKernelSize;

    void* pKernelBin = nullptr;
    void* pFcPatchBin = nullptr;

    pKernelBin = MOS_AllocMemory(m_kernelBinSize);
    if (!pKernelBin)
    {
        VP_RENDER_ASSERTMESSAGE("local creat surface faile, retun no space");
        MOS_SafeFreeMemory(pKernelBin);
        return MOS_STATUS_NO_SPACE;
    }

    MOS_SecureMemcpy(pKernelBin,
        m_kernelBinSize,
        m_kernelBin,
        m_kernelBinSize);

    if ((m_fcPatchBin != nullptr) && (m_fcPatchBinSize != 0))
    {
        pFcPatchBin = MOS_AllocMemory(m_fcPatchBinSize);
        if (!pFcPatchBin)
        {
            VP_RENDER_ASSERTMESSAGE("local creat surface faile, retun no space");
            MOS_SafeFreeMemory(pKernelBin);
            MOS_SafeFreeMemory(pFcPatchBin);
            return MOS_STATUS_NO_SPACE;
        }

        MOS_SecureMemcpy(pFcPatchBin,
            m_fcPatchBinSize,
            m_fcPatchBin,
            m_fcPatchBinSize);
    }

    // Allocate KDLL state (Kernel Dynamic Linking)
    m_kernelDllState = KernelDll_AllocateStates(
        pKernelBin,
        m_kernelBinSize,
        pFcPatchBin,
        m_fcPatchBinSize,
        m_kernelDllRules,
        ModifyFunctionPointers);
    if (!m_kernelDllState)
    {
        VP_RENDER_ASSERTMESSAGE("Failed to allocate KDLL state.");
        MOS_SafeFreeMemory(pKernelBin);
        MOS_SafeFreeMemory(pFcPatchBin);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS vp::VpRenderKernel::Destroy()
{
    if (m_kernelDllState)
    {
        KernelDll_ReleaseStates(m_kernelDllState);
    }

    return MOS_STATUS_SUCCESS;
}
