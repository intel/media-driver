/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     vp_render_kernel_obj.cpp
//! \brief    vp render kernel base object.
//! \details  vp render kernel base object will provided interface where sub kernels processing ways
//!
#include "vp_render_kernel_obj.h"
using namespace vp;

vp::MEDIA_OBJECT_KA2_INLINE_DATA VpRenderKernelObj::g_cInit_VP_MEDIA_OBJECT_KA2_INLINE_DATA =
{
    // DWORD 0
    {
        0,                                      // DestinationBlockHorizontalOrigin
        0                                       // DestinationBlockVerticalOrigin
    },

    // DWORD 1
    {
        0,                                      // HorizontalBlockCompositeMaskLayer0
        0                                       // VerticalBlockCompositeMaskLayer0
    },

    // DWORD 2
    {
        0,                                      // HorizontalBlockCompositeMaskLayer1
        0                                       // VerticalBlockCompositeMaskLayer1
    },

    // DWORD 3
    {
        0,                                      // HorizontalBlockCompositeMaskLayer2
        0                                       // VerticalBlockCompositeMaskLayer2
    },

    // DWORD 4
    0.0F,                                       // VideoXScalingStep

    // DWORD 5
    0.0F,                                       // VideoStepDelta

    // DWORD 6
    {
        0,                                      // VerticalBlockNumber
        0                                       // AreaOfInterest
    },

    // DWORD 7
    {
        0,                                      // GroupIDNumber
    },

    // DWORD 8
    {
        0,                                      // HorizontalBlockCompositeMaskLayer3
        0                                       // VerticalBlockCompositeMaskLayer3
    },

    // DWORD 9
    {
        0,                                      // HorizontalBlockCompositeMaskLayer4
        0                                       // VerticalBlockCompositeMaskLayer4
    },

    // DWORD 10
    {
        0,                                      // HorizontalBlockCompositeMaskLayer5
        0                                       // VerticalBlockCompositeMaskLayer5
    },

    // DWORD 11
    {
        0,                                      // HorizontalBlockCompositeMaskLayer6
        0                                       // VerticalBlockCompositeMaskLayer6
    },

    // DWORD 12
    {
        0,                                      // HorizontalBlockCompositeMaskLayer7
        0                                       // VerticalBlockCompositeMaskLayer7
    },

    // DWORD 13
    0,                                          // Reserved

    // DWORD 14
    0,                                          // Reserved

    // DWORD 15
    0                                           // Reserved
};

VpRenderKernelObj::VpRenderKernelObj(PVP_MHWINTERFACE hwInterface) :
    m_hwInterface(hwInterface)
{
}

VpRenderKernelObj::VpRenderKernelObj(PVP_MHWINTERFACE hwInterface, uint32_t kernelID, uint32_t kernelIndex) :
    m_hwInterface(hwInterface)
{
    KernelId id = KernelId(kernelID);
    switch(id)
    {
      case Kernel_Invalidate:
      default:
        m_kernelName.assign("");
        break;
    }
    SetKernelID(id);
    SetKernelIndex(kernelIndex);
}

VpRenderKernelObj::~VpRenderKernelObj()
{
    for (auto arg : m_kernelArgs)
    {
        MOS_FreeMemAndSetNull(arg.pData);
    }
}

MOS_STATUS VpRenderKernelObj::Init(VpRenderKernel& kernel)
{
    VP_FUNC_CALL();
    m_kernelSize = kernel.GetKernelSize() + KERNEL_BINARY_PADDING_SIZE;

    uint8_t* pKernelBin = (uint8_t *)kernel.GetKernelBinPointer();
    VP_RENDER_CHK_NULL_RETURN(pKernelBin);

    m_kernelBinary = pKernelBin + kernel.GetKernelBinOffset();

    m_kernelArgs = kernel.GetKernelArgs();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::GetCurbeState(void*& curbe, uint32_t& curbeLength)
{
    VP_FUNC_CALL();
    curbeLength = 0;
    for (auto arg : m_kernelArgs)
    {
        curbeLength += arg.uSize;
    }

    if (curbeLength == 0)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    uint8_t* pCurbe = (uint8_t *)MOS_AllocAndZeroMemory(curbeLength);

    uint32_t samplerIndex = 0;
    uint32_t samplerAvsIndex = 1;
    uint32_t sampler3DIndex = 0;

    for (auto& arg : m_kernelArgs)
    {
        if (arg.eArgKind == ARG_KIND_SURFACE_2D ||
            arg.eArgKind == ARG_KIND_SURFACE_1D ||
            arg.eArgKind == ARG_KIND_SURFACE_SAMPLER8X8_AVS)
        {
            uint32_t* pSurfaceindex = static_cast<uint32_t*>(arg.pData);
            auto it = m_surfaceBindingIndex.find(*pSurfaceindex);
            if (it == m_surfaceBindingIndex.end())
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }
            *((uint32_t *)(pCurbe + arg.uOffsetInPayload)) = it->second;
        }
        else if (arg.eArgKind == ARG_KIND_GENERAL)
        {
            MOS_SecureMemcpy(pCurbe + arg.uOffsetInPayload, arg.uSize, arg.pData, arg.uSize);
        }
        else if (arg.eArgKind == ARG_KIND_SAMPLER)
        {
            auto samplerState = m_samplerStates.at(samplerIndex);
            if (samplerState.SamplerType == MHW_SAMPLER_TYPE_3D)
            {
                MOS_SecureMemcpy(pCurbe + arg.uOffsetInPayload, arg.uSize, &sampler3DIndex, arg.uSize);
                sampler3DIndex++;
            }
            else if (samplerState.SamplerType == MHW_SAMPLER_TYPE_AVS)
            {
                MOS_SecureMemcpy(pCurbe + arg.uOffsetInPayload, arg.uSize, &samplerAvsIndex, arg.uSize);
                samplerAvsIndex++;
            }
            else
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }

            samplerIndex++;
        }
        else
        {
            return MOS_STATUS_UNIMPLEMENTED;
        }
    }

    curbe = pCurbe;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::SetKernelID(uint32_t kid)
{
    VP_FUNC_CALL();
    m_kernelID = kid;

    return MOS_STATUS_SUCCESS;
}

uint32_t VpRenderKernelObj::GetKernelID()
{
    return m_kernelID;
}

MOS_STATUS VpRenderKernelObj::SetKernelIndex(uint32_t kid)
{
    VP_FUNC_CALL();
    m_kernelIndex = kid;

    return MOS_STATUS_SUCCESS;
}

uint32_t VpRenderKernelObj::GetKernelIndex()
{
    return m_kernelIndex;
}

KERNEL_WALKER_PARAMS& VpRenderKernelObj::GetWalkerSetting()
{
    return m_walkerParam;
}

MOS_STATUS VpRenderKernelObj::SetupSurfaceState()
{
    VP_FUNC_CALL();
    KERNEL_SURFACE_STATE_PARAM kernelSurfaceParam;
    for (auto arg : m_kernelArgs)
    {
        VP_RENDER_CHK_NULL_RETURN(arg.pData);

        if (arg.eArgKind == ARG_KIND_SURFACE_2D ||
            arg.eArgKind == ARG_KIND_SURFACE_1D)
        {
            MOS_ZeroMemory(&kernelSurfaceParam, sizeof(KERNEL_SURFACE_STATE_PARAM));
            kernelSurfaceParam.surfaceOverwriteParams.updatedRenderSurfaces = true;
            PRENDERHAL_SURFACE_STATE_PARAMS pRenderSurfaceParams = &kernelSurfaceParam.surfaceOverwriteParams.renderSurfaceParams;
            pRenderSurfaceParams->bAVS = false;
            pRenderSurfaceParams->bRenderTarget = true;
            pRenderSurfaceParams->bWidthInDword_UV = true;
            pRenderSurfaceParams->bWidthInDword_Y = true;
            m_surfaceState.insert(std::make_pair(*(uint32_t*)arg.pData, kernelSurfaceParam));
        }
        else if (arg.eArgKind == ARG_KIND_SURFACE_SAMPLER8X8_AVS)
        {
            MOS_ZeroMemory(&kernelSurfaceParam, sizeof(KERNEL_SURFACE_STATE_PARAM));
            kernelSurfaceParam.surfaceOverwriteParams.updatedRenderSurfaces = true;
            PRENDERHAL_SURFACE_STATE_PARAMS pRenderSurfaceParams = &kernelSurfaceParam.surfaceOverwriteParams.renderSurfaceParams;
            pRenderSurfaceParams->bAVS = true;
            pRenderSurfaceParams->bRenderTarget = true;
            m_surfaceState.insert(std::make_pair(*(uint32_t*)arg.pData, kernelSurfaceParam));
        }
        else if (arg.eArgKind == ARG_KIND_SURFACE_SAMPLER)
        {
            MOS_ZeroMemory(&kernelSurfaceParam, sizeof(KERNEL_SURFACE_STATE_PARAM));
            kernelSurfaceParam.surfaceOverwriteParams.updatedRenderSurfaces = true;
            PRENDERHAL_SURFACE_STATE_PARAMS pRenderSurfaceParams = &kernelSurfaceParam.surfaceOverwriteParams.renderSurfaceParams;
            pRenderSurfaceParams->bAVS = false;
            pRenderSurfaceParams->bRenderTarget = true;
            pRenderSurfaceParams->bWidthInDword_UV = true;
            pRenderSurfaceParams->bWidthInDword_Y = true;
            m_surfaceState.insert(std::make_pair(*(uint32_t*)arg.pData, kernelSurfaceParam));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::SetSamplerStates(KERNEL_SAMPLER_STATE_GROUP& samplerStateGroup, KERNEL_SAMPLER_INDEX &kernelSamplerIndex)
{
    VP_FUNC_CALL();
    for (auto index : kernelSamplerIndex)
    {
        auto it = samplerStateGroup.find(index);
        if (it != samplerStateGroup.end())
        {
            m_samplerStates.push_back(it->second);
        }
        else
        {
            VP_RENDER_ASSERTMESSAGE("The Sampler State corresponding to index %d can't be found!", index);
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    return MOS_STATUS_SUCCESS;
}

KERNEL_SAMPLER_STATES& VpRenderKernelObj::GetSamplerStates()
{
    return m_samplerStates;
}

MOS_STATUS VpRenderKernelObj::SetWalkerSetting(KERNEL_THREAD_SPACE& threadSpace, bool bSyncFlag)
{
    VP_FUNC_CALL();
    MOS_ZeroMemory(&m_walkerParam, sizeof(KERNEL_WALKER_PARAMS));

    m_walkerParam.iBlocksX = threadSpace.uWidth;
    m_walkerParam.iBlocksY = threadSpace.uHeight;
    m_walkerParam.walkerNeeded = true;
    m_walkerParam.rotationNeeded = false;
    m_walkerParam.bSyncFlag = bSyncFlag;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::SetKernelArgs(KERNEL_ARGS& kernelArgs)
{
    VP_FUNC_CALL();
    if (kernelArgs.size() != m_kernelArgs.size())
    {
        VP_RENDER_ASSERTMESSAGE("The Kernel Arguments is not aligned!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    for (KRN_ARG& srcArg : kernelArgs)
    {
        for (KRN_ARG& dstArg : m_kernelArgs)
        {
            if (srcArg.uIndex == dstArg.uIndex)
            {
                if (srcArg.uSize == dstArg.uSize)
                {
                    if (srcArg.eArgKind == dstArg.eArgKind ||
                        dstArg.eArgKind == (srcArg.eArgKind & ~SURFACE_MASK))
                    {
                        if (srcArg.pData == nullptr)
                        {
                            VP_RENDER_ASSERTMESSAGE("The Kernel Argument is null!");
                            return MOS_STATUS_INVALID_PARAMETER;
                        }
                        else
                        {
                            dstArg.eArgKind = srcArg.eArgKind;
                            dstArg.pData = srcArg.pData;
                            srcArg.pData = nullptr;
                        }
                    }
                }
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::SetKernelConfigs(
    KERNEL_CONFIGS& kernelConfigs,
    KERNEL_PARAMS& kernelParams,
    VP_SURFACE_GROUP& surfaces,
    KERNEL_SAMPLER_STATE_GROUP& samplerStateGroup)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_STATUS_RETURN(SetKernelArgs(kernelParams.kernelArgs));

    VP_RENDER_CHK_STATUS_RETURN(SetProcessSurfaceGroup(surfaces));

    VP_RENDER_CHK_STATUS_RETURN(SetSamplerStates(samplerStateGroup, kernelParams.kernelSamplerIndex));

    VP_RENDER_CHK_STATUS_RETURN(SetWalkerSetting(kernelParams.kernelThreadSpace, kernelParams.syncFlag));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::SetKernelConfigs(KERNEL_CONFIGS& kernelConfigs, uint32_t kernelExecuteID)
{
    //For legacy kernel usage
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::GetInlineState(void** inlineData, uint32_t& inlineLength)
{
    return MOS_STATUS_SUCCESS;
}