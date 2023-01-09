/*===================== begin_copyright_notice ==================================

* Copyright (c) 2021, Intel Corporation
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

======================= end_copyright_notice ==================================*/

//!
//! \file     mhw_render_xe_hp_base.cpp
//! \brief    Constructs render engine commands on Gen12_5_plus platforms
//! \details  Each client facing function both creates a HW command and adds
//!           that command to a command or batch buffer.
//!

#include "mhw_render_xe_hp_base.h"
#include "mhw_render_hwcmd_xe_hp_base.h"

MOS_STATUS MhwRenderInterfaceXe_Xpm_Base::AddCfeStateCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VFE_PARAMS     params)
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

        cmd.DW3.MaximumNumberOfThreads = (params->dwMaximumNumberofThreads) ? params->dwMaximumNumberofThreads - 1 : params->pKernelState->KernelParams.iThreadCount - 1;
    }
    else
    {
        cmd.DW3.MaximumNumberOfThreads = (params->dwMaximumNumberofThreads) ? params->dwMaximumNumberofThreads - 1 : m_hwCaps.dwMaxThreads - 1;
    }

    MHW_VFE_PARAMS_G12 *paramsG12 = dynamic_cast<MHW_VFE_PARAMS_G12 *>(params);
    if (paramsG12 != nullptr)
    {
        cmd.DW1_2.ScratchSpaceBuffer       = paramsG12->scratchStateOffset >> 6;
        cmd.DW3.FusedEuDispatch            = paramsG12->bFusedEuDispatch ? false : true;  // disabled if DW3.FusedEuDispath = 1
        cmd.DW3.NumberOfWalkers            = paramsG12->numOfWalkers;
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

MOS_STATUS MhwRenderInterfaceXe_Xpm_Base::AddComputeWalkerCmd(
    PMOS_COMMAND_BUFFER      cmdBuffer,
    PMHW_GPGPU_WALKER_PARAMS gpgpuWalkerParams,
    PMHW_ID_ENTRY_PARAMS     interfaceDescriptorParams,
    PMOS_RESOURCE            postsyncResource,
    uint32_t                 resourceOffset)
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

    cmd.DW2.IndirectDataLength       = gpgpuWalkerParams->IndirectDataLength;
    cmd.DW3.IndirectDataStartAddress = gpgpuWalkerParams->IndirectDataStartAddress >> MHW_COMPUTE_INDIRECT_SHIFT;

    cmd.DW4.SIMDSize = mhw_render_xe_xpm_base::COMPUTE_WALKER_CMD::SIMD_SIZE_SIMD32;

    cmd.DW5.ExecutionMask = 0xffffffff;
    cmd.DW6.LocalXMaximum = gpgpuWalkerParams->ThreadWidth - 1;
    cmd.DW6.LocalYMaximum = gpgpuWalkerParams->ThreadHeight - 1;
    cmd.DW6.LocalZMaximum = gpgpuWalkerParams->ThreadDepth - 1;

    cmd.DW7.ThreadGroupIDXDimension = gpgpuWalkerParams->GroupWidth - gpgpuWalkerParams->GroupStartingX;
    cmd.DW8.ThreadGroupIDYDimension = gpgpuWalkerParams->GroupHeight - gpgpuWalkerParams->GroupStartingY;
    cmd.DW9.ThreadGroupIDZDimension = gpgpuWalkerParams->GroupDepth;
    cmd.DW10.ThreadGroupIDStartingX = gpgpuWalkerParams->GroupStartingX;
    cmd.DW11.ThreadGroupIDStartingY = gpgpuWalkerParams->GroupStartingY;
    cmd.DW12.ThreadGroupIDStartingZ = gpgpuWalkerParams->GroupStartingZ;

    cmd.interface_descriptor_data.DW0_1.KernelStartPointer              = interfaceDescriptorParams->dwKernelOffset >> MHW_KERNEL_OFFSET_SHIFT;
    cmd.interface_descriptor_data.DW3.SamplerCount                      = interfaceDescriptorParams->dwSamplerCount;
    cmd.interface_descriptor_data.DW3.SamplerStatePointer               = interfaceDescriptorParams->dwSamplerOffset >> MHW_SAMPLER_SHIFT;
    cmd.interface_descriptor_data.DW4.BindingTablePointer               = MOS_ROUNDUP_SHIFT(interfaceDescriptorParams->dwBindingTableOffset, MHW_BINDING_TABLE_ID_SHIFT);
    cmd.interface_descriptor_data.DW5.BarrierEnable                     = interfaceDescriptorParams->bBarrierEnable;
    cmd.interface_descriptor_data.DW5.NumberOfThreadsInGpgpuThreadGroup = interfaceDescriptorParams->dwNumberofThreadsInGPGPUGroup;
    cmd.interface_descriptor_data.DW5.SharedLocalMemorySize             = interfaceDescriptorParams->dwSharedLocalMemorySize;

    if (nullptr != postsyncResource)
    {
        MHW_RESOURCE_PARAMS resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        InitMocsParams(resourceParams, &cmd.postsync_data.DW0.Value, 5, 10);
        resourceParams.presResource    = postsyncResource;
        resourceParams.pdwCmd          = cmd.postsync_data.DW1_2.Value;
        resourceParams.dwLocationInCmd = 24;
        resourceParams.dwOffset        = resourceOffset;
        resourceParams.bIsWritable     = true;
        MHW_MI_CHK_STATUS(AddResourceToCmd(m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.postsync_data.DW0.Operation = mhw_render_xe_xpm_base::COMPUTE_WALKER_CMD::POSTSYNC_DATA_CMD ::POSTSYNC_OPERATION_WRITE_TIMESTAMP;
    }

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));
    return eStatus;
}

MOS_STATUS MhwRenderInterfaceXe_Xpm_Base::AddStateComputeModeCmd(
    const MHW_STATE_COMPUTE_MODE_PARAMS &computeStateMode,
    MOS_COMMAND_BUFFER *                 cmdBuffer)
{
    MHW_MI_CHK_NULL(m_osInterface);
    mhw_render_xe_xpm_base::STATE_COMPUTE_MODE_CMD cmd;
    cmd.DW1.MaskBits         = 0xFFFF;
    cmd.DW1.LargeGrfMode     = computeStateMode.enableLargeGrf ? 1 : 0;
    cmd.DW1.ForceNonCoherent = 2;
    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwRenderInterfaceXe_Xpm_Base::AddStateBaseAddrCmd(
    PMOS_COMMAND_BUFFER         cmdBuffer,
    PMHW_STATE_BASE_ADDR_PARAMS params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    MHW_RESOURCE_PARAMS resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.dwLsbNum      = MHW_RENDER_ENGINE_STATE_BASE_ADDRESS_SHIFT;
    resourceParams.HwCommandType = MOS_STATE_BASE_ADDR;

    mhw_render_xe_xpm_base::STATE_BASE_ADDRESS_CMD cmd;

    if (params->presGeneralState)
    {
        cmd.DW1_2.GeneralStateBaseAddressModifyEnable  = true;
        cmd.DW12.GeneralStateBufferSizeModifyEnable    = true;
        resourceParams.presResource                    = params->presGeneralState;
        resourceParams.dwOffset                        = 0;
        resourceParams.pdwCmd                          = cmd.DW1_2.Value;
        resourceParams.dwLocationInCmd                 = 1;

        // upper bound of the allocated resource will not be set
        resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

        InitMocsParams(resourceParams, &cmd.DW1_2.Value[0], 5, 10);

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        if (params->mocs4GeneralState != 0)
        {
            cmd.DW1_2.GeneralStateMemoryObjectControlState = params->mocs4GeneralState;
        }
        cmd.DW12.GeneralStateBufferSize = (params->dwGeneralStateSize + MHW_PAGE_SIZE - 1) / MHW_PAGE_SIZE;
    }

    if (m_osInterface->bNoParsingAssistanceInKmd)
    {
        uint32_t indirectStateOffset, indirectStateSize;
        MHW_MI_CHK_STATUS(m_osInterface->pfnGetIndirectState(m_osInterface, &indirectStateOffset, &indirectStateSize));

        // When KMD parsing assistance is not used,
        // UMD is required to set up the SSH
        // in the STATE_BASE_ADDRESS command.
        // All addresses used in the STATE_BASE_ADDRESS
        // command need to have the modify
        // bit associated with it set to 1.
        cmd.DW4_5.SurfaceStateBaseAddressModifyEnable  = true;
        resourceParams.presResource                    = &cmdBuffer->OsResource;
        resourceParams.dwOffset                        = indirectStateOffset;
        resourceParams.pdwCmd                          = cmd.DW4_5.Value;
        resourceParams.dwLocationInCmd                 = 4;

        // upper bound of the allocated resource will not be set
        resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

        InitMocsParams(resourceParams, &cmd.DW4_5.Value[0], 5, 10);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        if (params->mocs4SurfaceState != 0)
        {
            cmd.DW4_5.SurfaceStateMemoryObjectControlState = params->mocs4SurfaceState;
        }
    }

    if (params->presDynamicState)
    {
        cmd.DW6_7.DynamicStateBaseAddressModifyEnable  = true;
        cmd.DW13.DynamicStateBufferSizeModifyEnable    = true;
        resourceParams.presResource                    = params->presDynamicState;
        resourceParams.dwOffset                        = 0;
        resourceParams.pdwCmd                          = cmd.DW6_7.Value;
        resourceParams.dwLocationInCmd                 = 6;
        resourceParams.bIsWritable                     = params->bDynamicStateRenderTarget;

        // upper bound of the allocated resource will not be set
        resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

        InitMocsParams(resourceParams, &cmd.DW6_7.Value[0], 5, 10);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        if (params->mocs4DynamicState != 0)
        {
            cmd.DW6_7.DynamicStateMemoryObjectControlState = params->mocs4DynamicState;
        }

        cmd.DW13.DynamicStateBufferSize = (params->dwDynamicStateSize + MHW_PAGE_SIZE - 1) / MHW_PAGE_SIZE;

        //Reset bRenderTarget as it should be enabled only for Dynamic State
        resourceParams.bIsWritable = false;
    }

    if (params->presIndirectObjectBuffer)
    {
        cmd.DW8_9.IndirectObjectBaseAddressModifyEnable  = true;
        cmd.DW14.IndirectObjectBufferSizeModifyEnable    = true;
        resourceParams.presResource                      = params->presIndirectObjectBuffer;
        resourceParams.dwOffset                          = 0;
        resourceParams.pdwCmd                            = cmd.DW8_9.Value;
        resourceParams.dwLocationInCmd                   = 8;

        // upper bound of the allocated resource will not be set
        resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

        InitMocsParams(resourceParams, &cmd.DW8_9.Value[0], 5, 10);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        if (params->mocs4IndirectObjectBuffer != 0)
        {
            cmd.DW8_9.IndirectObjectMemoryObjectControlState = params->mocs4IndirectObjectBuffer;
        }
        cmd.DW14.IndirectObjectBufferSize = (params->dwIndirectObjectBufferSize + MHW_PAGE_SIZE - 1) / MHW_PAGE_SIZE;
    }

    if (params->presInstructionBuffer)
    {
        cmd.DW10_11.InstructionBaseAddressModifyEnable  = true;
        cmd.DW15.InstructionBufferSizeModifyEnable      = true;
        resourceParams.presResource                     = params->presInstructionBuffer;
        resourceParams.dwOffset                         = 0;
        resourceParams.pdwCmd                           = cmd.DW10_11.Value;
        resourceParams.dwLocationInCmd                  = 10;

        // upper bound of the allocated resource will not be set
        resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

        InitMocsParams(resourceParams, &cmd.DW10_11.Value[0], 5, 10);

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        if (params->mocs4InstructionCache != 0)
        {
            cmd.DW10_11.InstructionMemoryObjectControlState = params->mocs4InstructionCache;
        }
        cmd.DW15.InstructionBufferSize = (params->dwInstructionBufferSize + MHW_PAGE_SIZE - 1) / MHW_PAGE_SIZE;
    }

    // stateless dataport access
    cmd.DW3.StatelessDataPortAccessMemoryObjectControlState = params->mocs4StatelessDataport;
    cmd.DW3.L1CachePolicy                                   = params->l1CacheConfig;

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwRenderInterfaceXe_Xpm_Base::Add3DStateBindingTablePoolAllocCmd(
    MOS_COMMAND_BUFFER *                                       cmdBuffer,
    mhw_render_xe_xpm_base::_3DSTATE_BINDING_TABLE_POOL_ALLOC_CMD cmd)
{
    uint32_t indirect_state_offset = 0, indirect_state_size = 0;
    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_STATUS(m_osInterface->pfnGetIndirectState(m_osInterface,
        &indirect_state_offset,
        &indirect_state_size));

    MHW_RESOURCE_PARAMS resource_params;
    MOS_ZeroMemory(&resource_params, sizeof(resource_params));

    InitMocsParams(resource_params, &cmd.DW1_2.Value[0], 1, 6);
    resource_params.dwLsbNum        = MHW_RENDER_ENGINE_STATE_BASE_ADDRESS_SHIFT;
    resource_params.HwCommandType   = MOS_STATE_BASE_ADDR;
    resource_params.presResource    = &cmdBuffer->OsResource;
    resource_params.dwOffset        = indirect_state_offset;
    resource_params.pdwCmd          = cmd.DW1_2.Value;
    resource_params.dwLocationInCmd = 1;
    MHW_MI_CHK_STATUS(AddResourceToCmd(
        m_osInterface,
        cmdBuffer,
        &resource_params));

    cmd.DW3.BindingTablePoolBufferSize = indirect_state_size;
    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));
    return MOS_STATUS_SUCCESS;
}
