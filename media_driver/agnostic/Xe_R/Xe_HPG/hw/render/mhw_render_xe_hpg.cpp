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
//! \file     mhw_render_xe_hpg.cpp
//! \brief    Constructs render engine commands on Xe_HPG platforms
//! \details  Each client facing function both creates a HW command and adds
//!           that command to a command or batch buffer.
//!

#include "mhw_render_xe_hpg.h"

MOS_STATUS MhwRenderInterfaceXe_Hpg::AddCfeStateCmd(
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
        MHW_MI_CHK_NULL(params->pKernelState);

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
        cmd.DW3.FusedEuDispatch = false; // enabled Fused EU Mode
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

MOS_STATUS
MhwRenderInterfaceXe_Hpg::AddComputeWalkerCmd(MOS_COMMAND_BUFFER *cmdBuffer,
                                              MHW_GPGPU_WALKER_PARAMS *gpgpuWalkerParams,
                                              MHW_ID_ENTRY_PARAMS *interfaceDescriptorParams,
                                              MOS_RESOURCE *postsyncResource,
                                              uint32_t resourceOffset)
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
    cmd.DW2.IndirectDataLength = gpgpuWalkerParams->IndirectDataLength;
    cmd.DW3.IndirectDataStartAddress
            = gpgpuWalkerParams->IndirectDataStartAddress >> MHW_COMPUTE_INDIRECT_SHIFT;

    cmd.DW4.SIMDSize = mhw_render_xe_xpm_base::COMPUTE_WALKER_CMD::SIMD_SIZE_SIMD32;
    cmd.DW5.ExecutionMask = 0xffffffff;
    cmd.DW6.LocalXMaximum = gpgpuWalkerParams->ThreadWidth - 1;
    cmd.DW6.LocalYMaximum = gpgpuWalkerParams->ThreadHeight - 1;
    cmd.DW6.LocalZMaximum = gpgpuWalkerParams->ThreadDepth - 1;
    cmd.DW7.ThreadGroupIDXDimension = gpgpuWalkerParams->GroupWidth;
    cmd.DW8.ThreadGroupIDYDimension = gpgpuWalkerParams->GroupHeight;
    cmd.DW9.ThreadGroupIDZDimension = gpgpuWalkerParams->GroupDepth;
    cmd.DW10.ThreadGroupIDStartingX = gpgpuWalkerParams->GroupStartingX;
    cmd.DW11.ThreadGroupIDStartingY = gpgpuWalkerParams->GroupStartingY;
    cmd.DW12.ThreadGroupIDStartingZ = gpgpuWalkerParams->GroupStartingZ;

    cmd.interface_descriptor_data.DW0_1.KernelStartPointer
            = interfaceDescriptorParams->dwKernelOffset >> MHW_KERNEL_OFFSET_SHIFT;
    cmd.interface_descriptor_data.DW3.SamplerCount = interfaceDescriptorParams->dwSamplerCount;
    cmd.interface_descriptor_data.DW3.SamplerStatePointer
            = interfaceDescriptorParams->dwSamplerOffset >> MHW_SAMPLER_SHIFT;
    cmd.interface_descriptor_data.DW4.BindingTablePointer
            = MOS_ROUNDUP_SHIFT(interfaceDescriptorParams->dwBindingTableOffset,
                                MHW_BINDING_TABLE_ID_SHIFT);

    cmd.interface_descriptor_data.DW5.NumberOfThreadsInGpgpuThreadGroup
            = interfaceDescriptorParams->dwNumberofThreadsInGPGPUGroup;
    cmd.interface_descriptor_data.DW5.SharedLocalMemorySize
            = interfaceDescriptorParams->dwSharedLocalMemorySize;

    // when Barriers is not 0, the EU fusion will close.
    // Assigns barrier count.
    MHW_NORMALMESSAGE(" bBarrierEnable  = %d, block threads x=%d, y = %d", interfaceDescriptorParams->bBarrierEnable,
        cmd.DW7.ThreadGroupIDXDimension, cmd.DW8.ThreadGroupIDYDimension);

    if (interfaceDescriptorParams->bBarrierEnable)
    {   // Bits [28:30] represent the number of barriers on DG2.
        cmd.interface_descriptor_data.DW5.Reserved188 = 1;
    }

    if (nullptr != postsyncResource)
    {
        MHW_RESOURCE_PARAMS resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        InitMocsParams(resourceParams, &cmd.postsync_data.DW0.Value, 5, 10);
        resourceParams.presResource = postsyncResource;
        resourceParams.pdwCmd = cmd.postsync_data.DW1_2.Value;
        resourceParams.dwLocationInCmd = 24;
        resourceParams.dwOffset = resourceOffset;
        resourceParams.bIsWritable = true;
        MHW_MI_CHK_STATUS(AddResourceToCmd(m_osInterface, cmdBuffer, &resourceParams));
        cmd.postsync_data.DW0.Operation
                = mhw_render_xe_xpm_base::COMPUTE_WALKER_CMD::POSTSYNC_DATA_CMD
                ::POSTSYNC_OPERATION_WRITE_TIMESTAMP;
    }

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));
    return eStatus;
}

MOS_STATUS MhwRenderInterfaceXe_Hpg::AddChromaKeyCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_CHROMAKEY_PARAMS           params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(m_osInterface);
    MEDIA_WA_TABLE *pWaTable = m_osInterface->pfnGetWaTable(m_osInterface);
    MHW_MI_CHK_NULL(pWaTable);
    MOS_GPU_CONTEXT renderGpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);

    // Program stalling pipecontrol with HDC pipeline flush enabled before programming 3DSTATE_CHROMA_KEY for CCS W/L.
    if ((renderGpuContext == MOS_GPU_CONTEXT_COMPUTE)    ||
        (renderGpuContext == MOS_GPU_CONTEXT_CM_COMPUTE) ||
        (renderGpuContext == MOS_GPU_CONTEXT_COMPUTE_RA))
    {
        if (MEDIA_IS_WA(pWaTable, Wa_16011481064))
        {
            MHW_PIPE_CONTROL_PARAMS PipeControlParams;

            MOS_ZeroMemory(&PipeControlParams, sizeof(PipeControlParams));
            PipeControlParams.dwFlushMode                   = MHW_FLUSH_WRITE_CACHE;
            PipeControlParams.bGenericMediaStateClear       = true;
            PipeControlParams.bIndirectStatePointersDisable = true;
            PipeControlParams.bDisableCSStall               = false;
            PipeControlParams.bHdcPipelineFlush             = true;
            MHW_MI_CHK_STATUS(m_miInterface->AddPipeControl(cmdBuffer, nullptr, &PipeControlParams));
        }
    }

    mhw_render_xe_xpm_base::_3DSTATE_CHROMA_KEY_CMD cmd;
    cmd.DW1.ChromakeyTableIndex = params->dwIndex;
    cmd.DW2.ChromakeyLowValue   = params->dwLow;
    cmd.DW3.ChromakeyHighValue  = params->dwHigh;

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}
