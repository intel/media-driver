/*===================== begin_copyright_notice ==================================

# Copyright (c) 2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/

//!
//! \file     mhw_render_xe_hpc.cpp
//! \brief    Constructs render engine commands on PVC platforms
//! \details  Each client facing function both creates a HW command and adds
//!           that command to a command or batch buffer.
//!

#include "mhw_render_xe_hpc.h"

MOS_STATUS MhwRenderInterfaceXe_Hpc::AddCfeStateCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VFE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    mhw_render_xe_xpm_base::CFE_STATE_CMD cmd;

    if (params->pKernelState)
    {
        cmd.DW3.MaximumNumberOfThreads = (params->dwMaximumNumberofThreads) ?
                                         params->dwMaximumNumberofThreads - 1 :
                                         params->pKernelState->KernelParams.iThreadCount - 1;
    }
    else
    {
        cmd.DW3.MaximumNumberOfThreads = (params->dwMaximumNumberofThreads) ?
                                         params->dwMaximumNumberofThreads - 1 :
                                         m_hwCaps.dwMaxThreads - 1;
    }

    MHW_VFE_PARAMS_G12 *paramsG12 = dynamic_cast<MHW_VFE_PARAMS_G12 *> (params);
    if (paramsG12 != nullptr)
    {
        cmd.DW1_2.ScratchSpaceBuffer = paramsG12->scratchStateOffset >> 6;
        cmd.DW3.FusedEuDispatch = false; //PVC remove FusedEuDispatch.
        cmd.DW3.NumberOfWalkers = paramsG12->numOfWalkers;
        cmd.DW3.SingleSliceDispatchCcsMode = paramsG12->enableSingleSliceDispatchCcsMode;
    }
    else
    {
        MHW_ASSERTMESSAGE("Gen12-Specific CFE Params are needed.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwRenderInterfaceXe_Hpc::AddComputeWalkerCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_GPGPU_WALKER_PARAMS        gpgpuWalkerParams,
    PMHW_ID_ENTRY_PARAMS            interfaceDescriptorParams,
    PMOS_RESOURCE                   postsyncResource,
    uint32_t                        resourceOffset)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(gpgpuWalkerParams);
    MHW_MI_CHK_NULL(interfaceDescriptorParams);
    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(m_osInterface->pfnGetPlatform);

    if (gpgpuWalkerParams->ThreadDepth == 0)
    {
        gpgpuWalkerParams->ThreadDepth = 1;
    }
    if (gpgpuWalkerParams->GroupDepth == 0)
    {
        gpgpuWalkerParams->GroupDepth = 1;
    }

    mhw_render_xe_xpm_base::COMPUTE_WALKER_CMD cmd;

    cmd.DW2.IndirectDataLength               = gpgpuWalkerParams->IndirectDataLength;
    cmd.DW3.IndirectDataStartAddress         = gpgpuWalkerParams->IndirectDataStartAddress >> MHW_COMPUTE_INDIRECT_SHIFT;

    cmd.DW4.SIMDSize                         = mhw_render_xe_xpm_base::COMPUTE_WALKER_CMD::SIMD_SIZE_SIMD32;
    cmd.DW4.MessageSIMD                      = cmd.DW4.SIMDSize;

    cmd.DW5.ExecutionMask                    = 0xffffffff;
    cmd.DW6.LocalXMaximum                    = gpgpuWalkerParams->ThreadWidth - 1;
    cmd.DW6.LocalYMaximum                    = gpgpuWalkerParams->ThreadHeight - 1;
    cmd.DW6.LocalZMaximum                    = gpgpuWalkerParams->ThreadDepth - 1;

    cmd.DW7.ThreadGroupIDXDimension          = gpgpuWalkerParams->GroupWidth;
    cmd.DW8.ThreadGroupIDYDimension          = gpgpuWalkerParams->GroupHeight;
    cmd.DW9.ThreadGroupIDZDimension          = gpgpuWalkerParams->GroupDepth;
    cmd.DW10.ThreadGroupIDStartingX          = gpgpuWalkerParams->GroupStartingX;
    cmd.DW11.ThreadGroupIDStartingY          = gpgpuWalkerParams->GroupStartingY;
    cmd.DW12.ThreadGroupIDStartingZ          = gpgpuWalkerParams->GroupStartingZ;

    cmd.interface_descriptor_data.DW0_1.KernelStartPointer = interfaceDescriptorParams->dwKernelOffset >> MHW_KERNEL_OFFSET_SHIFT;
    cmd.interface_descriptor_data.DW3.SamplerCount = interfaceDescriptorParams->dwSamplerCount;
    cmd.interface_descriptor_data.DW3.SamplerStatePointer = interfaceDescriptorParams->dwSamplerOffset >> MHW_SAMPLER_SHIFT;
    cmd.interface_descriptor_data.DW4.BindingTablePointer = MOS_ROUNDUP_SHIFT(interfaceDescriptorParams->dwBindingTableOffset, MHW_BINDING_TABLE_ID_SHIFT);
    cmd.interface_descriptor_data.DW5.BarrierEnable = interfaceDescriptorParams->bBarrierEnable;
    cmd.interface_descriptor_data.DW5.NumberOfThreadsInGpgpuThreadGroup = interfaceDescriptorParams->dwNumberofThreadsInGPGPUGroup;
    cmd.interface_descriptor_data.DW5.SharedLocalMemorySize = interfaceDescriptorParams->dwSharedLocalMemorySize;

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));
    return eStatus;
}