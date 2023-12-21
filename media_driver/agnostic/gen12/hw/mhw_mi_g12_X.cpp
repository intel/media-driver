/*
* Copyright (c) 2015-2020, Intel Corporation
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
//! \file     mhw_mi_g12_X.cpp
//! \brief    Constructs MI commands on Gen12-based platforms
//! \details  Each client facing function both creates a HW command and adds
//!           that command to a command or batch buffer.
//!

#include "mhw_mi_g12_X.h"
#include "mhw_mi_hwcmd_g12_X.h"
#include "media_skuwa_specific.h"

MOS_STATUS MhwMiInterfaceG12::AddMiSemaphoreWaitCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_MI_SEMAPHORE_WAIT_PARAMS   params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(cmdBuffer->pCmdPtr);
    MHW_MI_CHK_NULL(params);

    mhw_mi_g12_X::MI_SEMAPHORE_WAIT_CMD *cmd =
        (mhw_mi_g12_X::MI_SEMAPHORE_WAIT_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwMiInterfaceGeneric<mhw_mi_g12_X>::AddMiSemaphoreWaitCmd(cmdBuffer, params));

    cmd->DW0.RegisterPollMode = params->bRegisterPollMode;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG12::AddPipeControl(
    PMOS_COMMAND_BUFFER      cmdBuffer,
    PMHW_BATCH_BUFFER        batchBuffer,
    PMHW_PIPE_CONTROL_PARAMS params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    MHW_MI_CHK_NULL(m_osInterface);
    MEDIA_WA_TABLE *pWaTable = m_osInterface->pfnGetWaTable(m_osInterface);
    MHW_MI_CHK_NULL(pWaTable);

    if (cmdBuffer == nullptr && batchBuffer == nullptr)
    {
        MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
        return MOS_STATUS_NULL_POINTER;
    }

    mhw_mi_g12_X::PIPE_CONTROL_CMD     cmd;
    cmd.DW1.PipeControlFlushEnable     = params->bKernelFenceEnabled ? false : true;
    cmd.DW1.CommandStreamerStallEnable = !params->bDisableCSStall;
    cmd.DW4_5.Value[0]                 = params->dwDataDW1;
    cmd.DW4_5.Value[1]                 = params->dwDataDW2;

    if (params->presDest)
    {
        cmd.DW1.PostSyncOperation      = params->dwPostSyncOp;
        cmd.DW1.DestinationAddressType = UseGlobalGtt.m_cs;

        MHW_RESOURCE_PARAMS resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource    = params->presDest;
        resourceParams.dwOffset        = params->dwResourceOffset;
        resourceParams.pdwCmd          = &(cmd.DW2.Value);
        resourceParams.dwLocationInCmd = 2;
        resourceParams.dwLsbNum        = MHW_COMMON_MI_PIPE_CONTROL_SHIFT;
        resourceParams.bIsWritable     = true;
        resourceParams.HwCommandType   = MOS_PIPE_CONTROL;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }
    else
    {
        if (MEDIA_IS_WA(pWaTable, Wa_14010840176))
        {
            cmd.DW0.HdcPipelineFlush                = true;
            cmd.DW1.ConstantCacheInvalidationEnable = false;
        }
        else
        {
            cmd.DW1.ConstantCacheInvalidationEnable = true;
        }
        cmd.DW1.StateCacheInvalidationEnable     = true;
        cmd.DW1.VfCacheInvalidationEnable        = true;
        cmd.DW1.InstructionCacheInvalidateEnable = true;
        cmd.DW1.RenderTargetCacheFlushEnable     = true;
        cmd.DW1.PostSyncOperation                = cmd.POST_SYNC_OPERATION_NOWRITE;
    }

    // Cache flush mode
    switch (params->dwFlushMode)
    {
    // Flush all Write caches
    case MHW_FLUSH_WRITE_CACHE:
        cmd.DW1.RenderTargetCacheFlushEnable = true;
        cmd.DW1.DcFlushEnable                = true;
        break;

    // Invalidate all Read-only caches
    case MHW_FLUSH_READ_CACHE:
        if (MEDIA_IS_WA(pWaTable, Wa_14010840176))
        {
            cmd.DW0.HdcPipelineFlush                = true;
            cmd.DW1.ConstantCacheInvalidationEnable = false;
        }
        else
        {
            cmd.DW1.ConstantCacheInvalidationEnable = true;
        }
        cmd.DW1.RenderTargetCacheFlushEnable     = false;
        cmd.DW1.StateCacheInvalidationEnable     = true;
        cmd.DW1.VfCacheInvalidationEnable        = true;
        cmd.DW1.InstructionCacheInvalidateEnable = true;
        break;

    // Custom flush parameters
    case MHW_FLUSH_CUSTOM:
        if (MEDIA_IS_WA(pWaTable, Wa_14010840176) && params->bInvalidateConstantCache)
        {
            cmd.DW0.HdcPipelineFlush                = true;
            cmd.DW1.StateCacheInvalidationEnable    = true;
            cmd.DW1.ConstantCacheInvalidationEnable = false;
        }
        else
        {
            cmd.DW1.StateCacheInvalidationEnable    = params->bInvalidateStateCache;
            cmd.DW1.ConstantCacheInvalidationEnable = params->bInvalidateConstantCache;
        }

        cmd.DW1.RenderTargetCacheFlushEnable     = params->bFlushRenderTargetCache;
        cmd.DW1.DcFlushEnable                    = params->bFlushRenderTargetCache;  // same as above
        cmd.DW1.VfCacheInvalidationEnable        = params->bInvalidateVFECache;
        cmd.DW1.InstructionCacheInvalidateEnable = params->bInvalidateInstructionCache;
        cmd.DW1.TlbInvalidate                    = params->bTlbInvalidate;
        cmd.DW1.TextureCacheInvalidationEnable   = params->bInvalidateTextureCache;
        break;

    // No-flush operation requested
    case MHW_FLUSH_NONE:
    default:
        cmd.DW1.RenderTargetCacheFlushEnable = false;
        break;
    }

    // When PIPE_CONTROL stall bit is set, one of the following must also be set, otherwise set stall bit to 0
    if (cmd.DW1.CommandStreamerStallEnable &&
        (cmd.DW1.DcFlushEnable == 0 && cmd.DW1.NotifyEnable == 0 && cmd.DW1.PostSyncOperation == 0 &&
            cmd.DW1.DepthStallEnable == 0 && cmd.DW1.StallAtPixelScoreboard == 0 && cmd.DW1.DepthCacheFlushEnable == 0 &&
            cmd.DW1.RenderTargetCacheFlushEnable == 0) &&
            !params->bKernelFenceEnabled)
    {
        cmd.DW1.CommandStreamerStallEnable = 0;
    }

    if (params->bGenericMediaStateClear)
    {
        cmd.DW1.GenericMediaStateClear = true;
    }

    if (params->bIndirectStatePointersDisable)
    {
        cmd.DW1.IndirectStatePointersDisable = true;
    }

    if (params->bHdcPipelineFlush)
    {
        cmd.DW0.HdcPipelineFlush = true;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG12::AddMiBatchBufferStartCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    PMHW_BATCH_BUFFER                   batchBuffer)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(batchBuffer);
    MHW_MI_CHK_NULL(m_osInterface);
    bool vcsEngineUsed =
        MOS_VCS_ENGINE_USED(m_osInterface->pfnGetGpuContext(m_osInterface));

    mhw_mi_g12_X::MI_BATCH_BUFFER_START_CMD cmd;
    MHW_RESOURCE_PARAMS                     resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.presResource     = &batchBuffer->OsResource;
    resourceParams.dwOffset         = batchBuffer->dwOffset;
    resourceParams.pdwCmd           = cmd.DW1_2.Value;
    resourceParams.dwLocationInCmd  = 1;
    resourceParams.dwLsbNum         = MHW_COMMON_MI_GENERAL_SHIFT;
    resourceParams.HwCommandType    = vcsEngineUsed ?
        MOS_MI_BATCH_BUFFER_START : MOS_MI_BATCH_BUFFER_START_RCS;

    MHW_MI_CHK_STATUS(AddResourceToCmd(
        m_osInterface,
        cmdBuffer,
        &resourceParams));

    // Set BB start
    cmd.DW0.Obj3.SecondLevelBatchBuffer = true;
    cmd.DW0.Obj0.AddressSpaceIndicator  = !IsGlobalGttInUse();

    // Send BB start command
    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG12::AddMiBatchBufferStartCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_BATCH_BUFFER   batchBuffer,
    bool useChainedBB)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(batchBuffer);
    MHW_MI_CHK_NULL(m_osInterface);
    bool vcsEngineUsed =
        MOS_VCS_ENGINE_USED(m_osInterface->pfnGetGpuContext(m_osInterface));

    mhw_mi_g12_X::MI_BATCH_BUFFER_START_CMD cmd;
    MHW_RESOURCE_PARAMS                     resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.presResource    = &batchBuffer->OsResource;
    resourceParams.dwOffset        = batchBuffer->dwOffset;
    resourceParams.pdwCmd          = cmd.DW1_2.Value;
    resourceParams.dwLocationInCmd = 1;
    resourceParams.dwLsbNum        = MHW_COMMON_MI_GENERAL_SHIFT;
    resourceParams.HwCommandType   = vcsEngineUsed ? MOS_MI_BATCH_BUFFER_START : MOS_MI_BATCH_BUFFER_START_RCS;

    MHW_MI_CHK_STATUS(AddResourceToCmd(
        m_osInterface,
        cmdBuffer,
        &resourceParams));

    // Set BB start
    cmd.DW0.Obj3.SecondLevelBatchBuffer = useChainedBB ? false : true;
    cmd.DW0.Obj0.AddressSpaceIndicator  = !IsGlobalGttInUse();

    // Send BB start command
    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG12::AddMiConditionalBatchBufferEndCmd(
    PMOS_COMMAND_BUFFER                             cmdBuffer,
    PMHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS     params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->presSemaphoreBuffer);

    // Case 1 - Batch buffer condition matches - If this is not present then conditional 
    //          batch buffer will  exit to ring with terminating CP.
    // Case 2 - Batch buffer condition DOES NOT match - Although this will disable CP 
    //          but after end of conditional batch buffer CP will be re-enabled.
    MHW_MI_CHK_STATUS(m_cpInterface->AddEpilog(m_osInterface, cmdBuffer));

    mhw_mi_g12_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD cmd;
    cmd.DW0.UseGlobalGtt        = IsGlobalGttInUse();
    cmd.DW0.CompareSemaphore    = 1; // CompareDataDword is always assumed to be set
    cmd.DW0.CompareMaskMode     = !params->bDisableCompareMask;
    if (params->dwParamsType == MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS::ENHANCED_PARAMS)
    {
        cmd.DW0.EndCurrentBatchBufferLevel
            = static_cast<MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS*>(params)->enableEndCurrentBatchBuffLevel;
        cmd.DW0.CompareOperation
            = static_cast<MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS*>(params)->compareOperation;
    }
    cmd.DW1.CompareDataDword    = params->dwValue;

    MHW_RESOURCE_PARAMS resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.presResource     = params->presSemaphoreBuffer;
    resourceParams.dwOffset         = params->dwOffset;
    resourceParams.pdwCmd           = cmd.DW2_3.Value;
    resourceParams.dwLocationInCmd  = 2;
    resourceParams.dwLsbNum         = MHW_COMMON_MI_CONDITIONAL_BATCH_BUFFER_END_SHIFT;
    resourceParams.HwCommandType    = MOS_MI_CONDITIONAL_BATCH_BUFFER_END;

    MHW_MI_CHK_STATUS(AddResourceToCmd(
        m_osInterface,
        cmdBuffer,
        &resourceParams));

    // Send Conditional Batch Buffer End command
    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    //Re-enable CP for Case 2
    MHW_MI_CHK_STATUS(m_cpInterface->AddProlog(m_osInterface, cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG12::AddMiSetPredicateCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    MHW_MI_SET_PREDICATE_ENABLE         enableFlag)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);

    mhw_mi_g12_X::MI_SET_PREDICATE_CMD cmd;
    cmd.DW0.PredicateEnable = enableFlag;
    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG12::AddMiStoreRegisterMemCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    PMHW_MI_STORE_REGISTER_MEM_PARAMS   params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(cmdBuffer->pCmdPtr);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(m_osInterface);

    mhw_mi_g12_X::MI_STORE_REGISTER_MEM_CMD cmd{};
    MHW_RESOURCE_PARAMS resourceParams{};
    resourceParams.presResource     = params->presStoreBuffer;
    resourceParams.dwOffset         = params->dwOffset;
    resourceParams.pdwCmd           = cmd.DW2_3.Value;
    resourceParams.dwLocationInCmd  = 2;
    resourceParams.dwLsbNum         = MHW_COMMON_MI_GENERAL_SHIFT;
    resourceParams.HwCommandType    = MOS_MI_STORE_REGISTER_MEM;
    resourceParams.bIsWritable      = true;

    MHW_MI_CHK_STATUS(AddResourceToCmd(
        m_osInterface,
        cmdBuffer,
        &resourceParams));

    cmd.DW0.UseGlobalGtt = IsGlobalGttInUse();
    cmd.DW1.RegisterAddress = params->dwRegister >> 2;

    if (IsRelativeMMIO(params->dwRegister))
    {
        cmd.DW0.AddCsMmioStartOffset = 1;
        cmd.DW1.RegisterAddress = params->dwRegister >> 2;
    }

    if (params->dwOption == CCS_HW_FRONT_END_MMIO_REMAP)
    {
        MOS_GPU_CONTEXT gpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);

        if (MOS_RCS_ENGINE_USED(gpuContext))
        {
            params->dwRegister &= M_CCS_HW_FRONT_END_MMIO_MASK;
            params->dwRegister += M_MMIO_CCS0_HW_FRONT_END_BASE_BEGIN;
        }
    }

    cmd.DW0.MmioRemapEnable = IsRemappingMMIO(params->dwRegister);

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG12::AddMiLoadRegisterMemCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    PMHW_MI_LOAD_REGISTER_MEM_PARAMS    params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(cmdBuffer->pCmdPtr);
    MHW_MI_CHK_NULL(params);

    mhw_mi_g12_X::MI_LOAD_REGISTER_MEM_CMD *cmd =
        (mhw_mi_g12_X::MI_LOAD_REGISTER_MEM_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwMiInterfaceGeneric<mhw_mi_g12_X>::AddMiLoadRegisterMemCmd(cmdBuffer, params));

    if (IsRelativeMMIO(params->dwRegister))
    {
        cmd->DW0.AddCsMmioStartOffset = 1;
        cmd->DW1.RegisterAddress = params->dwRegister >> 2;
    }

    cmd->DW0.MmioRemapEnable = IsRemappingMMIO(params->dwRegister);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG12::AddMiLoadRegisterImmCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    PMHW_MI_LOAD_REGISTER_IMM_PARAMS    params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(cmdBuffer->pCmdPtr);
    MHW_MI_CHK_NULL(params);

    mhw_mi_g12_X::MI_LOAD_REGISTER_IMM_CMD *cmd =
        (mhw_mi_g12_X::MI_LOAD_REGISTER_IMM_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwMiInterfaceGeneric<mhw_mi_g12_X>::AddMiLoadRegisterImmCmd(cmdBuffer, params));

    if (IsRelativeMMIO(params->dwRegister))
    {
        cmd->DW0.AddCsMmioStartOffset = 1;
        cmd->DW1.RegisterOffset = params->dwRegister >> 2;
    }

    cmd->DW0.MmioRemapEnable = IsRemappingMMIO(params->dwRegister);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG12::AddMiLoadRegisterRegCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    PMHW_MI_LOAD_REGISTER_REG_PARAMS    params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(cmdBuffer->pCmdPtr);
    MHW_MI_CHK_NULL(params);

    mhw_mi_g12_X::MI_LOAD_REGISTER_REG_CMD *cmd =
        (mhw_mi_g12_X::MI_LOAD_REGISTER_REG_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwMiInterfaceGeneric<mhw_mi_g12_X>::AddMiLoadRegisterRegCmd(cmdBuffer, params));

    if (IsRelativeMMIO(params->dwSrcRegister))
    {
        cmd->DW0.AddCsMmioStartOffsetSource = 1;
        cmd->DW1.SourceRegisterAddress = params->dwSrcRegister >> 2;
    }
    if (IsRelativeMMIO(params->dwDstRegister))
    {
        cmd->DW0.AddCsMmioStartOffsetDestination = 1;
        cmd->DW2.DestinationRegisterAddress = params->dwDstRegister >> 2;
    }
    
    cmd->DW0.MmioRemapEnableSource      = IsRemappingMMIO(params->dwSrcRegister);
    cmd->DW0.MmioRemapEnableDestination = IsRemappingMMIO(params->dwDstRegister);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG12::AddMiForceWakeupCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    PMHW_MI_FORCE_WAKEUP_PARAMS         params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(cmdBuffer->pCmdPtr);
    MHW_MI_CHK_NULL(params);

    mhw_mi_g12_X::MI_FORCE_WAKEUP_CMD cmd;
    cmd.DW1.ForceMediaSlice0Awake               = params->bForceMediaSlice0Awake;
    cmd.DW1.ForceRenderAwake                    = params->bForceRenderAwake;
    cmd.DW1.ForceMediaSlice1Awake               = params->bForceMediaSlice1Awake;
    cmd.DW1.ForceMediaSlice2Awake               = params->bForceMediaSlice2Awake;
    cmd.DW1.ForceMediaSlice3Awake               = params->bForceMediaSlice3Awake;
    cmd.DW1.HevcPowerWellControl                = params->bHEVCPowerWellControl;
    cmd.DW1.MfxPowerWellControl                 = params->bMFXPowerWellControl;
    cmd.DW1.MaskBits                            = params->bForceMediaSlice0AwakeMask;
    cmd.DW1.MaskBits                            += (params->bForceRenderAwakeMask << 1);
    cmd.DW1.MaskBits                            += (params->bForceMediaSlice1AwakeMask << 2);
    cmd.DW1.MaskBits                            += (params->bForceMediaSlice2AwakeMask << 3);
    cmd.DW1.MaskBits                            += (params->bForceMediaSlice3AwakeMask << 4);
    cmd.DW1.MaskBits                            += (params->bHEVCPowerWellControlMask  << 8);
    cmd.DW1.MaskBits                            += (params->bMFXPowerWellControlMask   << 9);

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG12::AddMiVdControlStateCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_MI_VD_CONTROL_STATE_PARAMS      params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    mhw_mi_g12_X::VD_CONTROL_STATE_CMD cmd;

    if (params->vdencEnabled)
    {
        cmd.DW0.MediaInstructionCommand =
            mhw_mi_g12_X::VD_CONTROL_STATE_CMD::MEDIA_INSTRUCTION_COMMAND_VDCONTROLSTATEFORVDENC;
        cmd.DW0.MediaInstructionOpcode  =
            mhw_mi_g12_X::VD_CONTROL_STATE_CMD::MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEFORVDENC;
    }
    else
    {
        cmd.DW0.MediaInstructionCommand =
            mhw_mi_g12_X::VD_CONTROL_STATE_CMD::MEDIA_INSTRUCTION_COMMAND_VDCONTROLSTATEFORHCP;
        if (params->avpEnabled)
        {
            cmd.DW0.MediaInstructionOpcode =
                mhw_mi_g12_X::VD_CONTROL_STATE_CMD::MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEFORAVP;
        }
        else
        {
            cmd.DW0.MediaInstructionOpcode =
                mhw_mi_g12_X::VD_CONTROL_STATE_CMD::MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEFORHCP;
        }

        cmd.DW1.PipelineInitialization  = params->initialization;
        cmd.DW2.MemoryImplicitFlush     = params->memoryImplicitFlush;
        cmd.DW2.ScalableModePipeLock    = params->scalableModePipeLock;
        cmd.DW2.ScalableModePipeUnlock  = params->scalableModePipeUnlock;
    }

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return MOS_STATUS_SUCCESS;
}

void MhwMiInterfaceG12::InitMmioRegisters()
{
    MHW_MI_MMIOREGISTERS *mmioRegisters = &m_mmioRegisters;

    mmioRegisters->generalPurposeRegister0LoOffset            = GP_REGISTER0_LO_OFFSET_G12;
    mmioRegisters->generalPurposeRegister0HiOffset            = GP_REGISTER0_HI_OFFSET_G12;
    mmioRegisters->generalPurposeRegister4LoOffset            = GP_REGISTER4_LO_OFFSET_G12;
    mmioRegisters->generalPurposeRegister4HiOffset            = GP_REGISTER4_HI_OFFSET_G12;
    mmioRegisters->generalPurposeRegister11LoOffset           = GP_REGISTER11_LO_OFFSET_G12;
    mmioRegisters->generalPurposeRegister11HiOffset           = GP_REGISTER11_HI_OFFSET_G12;
    mmioRegisters->generalPurposeRegister12LoOffset           = GP_REGISTER12_LO_OFFSET_G12;
    mmioRegisters->generalPurposeRegister12HiOffset           = GP_REGISTER12_HI_OFFSET_G12;
}

MOS_STATUS MhwMiInterfaceG12::SetWatchdogTimerThreshold(uint32_t frameWidth, uint32_t frameHeight, bool isEncoder)
{
    MEDIA_WA_TABLE *waTable = nullptr;

    MHW_FUNCTION_ENTER;
    MHW_MI_CHK_NULL(m_osInterface);
    if (m_osInterface->bMediaReset == false ||
        m_osInterface->umdMediaResetEnable == false)
    {
        return MOS_STATUS_SUCCESS;
    }

    waTable = m_osInterface->pfnGetWaTable(m_osInterface);
    MHW_CHK_NULL_RETURN(waTable);

    if (isEncoder)
    {
        if ((frameWidth * frameHeight) >= (7680 * 4320))
        {
            MediaResetParam.watchdogCountThreshold = MHW_MI_ENCODER_16K_WATCHDOG_THRESHOLD_IN_MS;
        }
        else if ((frameWidth * frameHeight) >= (3840 * 2160))
        {
            MediaResetParam.watchdogCountThreshold = MHW_MI_ENCODER_8K_WATCHDOG_THRESHOLD_IN_MS;
        }
        else if ((frameWidth * frameHeight) >= (1920 * 1080))
        {
            MediaResetParam.watchdogCountThreshold = MHW_MI_ENCODER_4K_WATCHDOG_THRESHOLD_IN_MS;
        }
        else
        {
            MediaResetParam.watchdogCountThreshold = MHW_MI_ENCODER_FHD_WATCHDOG_THRESHOLD_IN_MS;
        }
    }
    else
    {
        if ((frameWidth * frameHeight) >= (7680 * 4320))
        {
            MediaResetParam.watchdogCountThreshold = MHW_MI_DECODER_8K_WATCHDOG_THRESHOLD_IN_MS;
        }
        else if ((frameWidth * frameHeight) >= (3840 * 2160))
        {
            MediaResetParam.watchdogCountThreshold = MHW_MI_DECODER_4K_WATCHDOG_THRESHOLD_IN_MS;
        }
        else
        {
            MediaResetParam.watchdogCountThreshold = MHW_MI_DECODER_720P_WATCHDOG_THRESHOLD_IN_MS;
        }
    }

    GetWatchdogThreshold(m_osInterface);
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG12::SetWatchdogTimerRegisterOffset(
    MOS_GPU_CONTEXT                 gpuContext)
{
    MHW_FUNCTION_ENTER;

    switch (gpuContext)
    {
        // RCS
    case MOS_GPU_CONTEXT_RENDER:
    case MOS_GPU_CONTEXT_RENDER2:
    case MOS_GPU_CONTEXT_RENDER3:
    case MOS_GPU_CONTEXT_RENDER4:
    case MOS_GPU_CONTEXT_COMPUTE:
    case MOS_GPU_CONTEXT_CM_COMPUTE:
    case MOS_GPU_CONTEXT_RENDER_RA:
    case MOS_GPU_CONTEXT_COMPUTE_RA:
        MediaResetParam.watchdogCountCtrlOffset = WATCHDOG_COUNT_CTRL_OFFSET_RCS_G12;
        MediaResetParam.watchdogCountThresholdOffset = WATCHDOG_COUNT_THRESTHOLD_OFFSET_RCS_G12;
        break;
        // VCS0
    case MOS_GPU_CONTEXT_VIDEO:
    case MOS_GPU_CONTEXT_VIDEO2:
    case MOS_GPU_CONTEXT_VIDEO3:
    case MOS_GPU_CONTEXT_VIDEO4:
    case MOS_GPU_CONTEXT_VIDEO5:
    case MOS_GPU_CONTEXT_VIDEO6:
    case MOS_GPU_CONTEXT_VIDEO7:
        MediaResetParam.watchdogCountCtrlOffset = WATCHDOG_COUNT_CTRL_OFFSET_VCS0_G12;
        MediaResetParam.watchdogCountThresholdOffset = WATCHDOG_COUNT_THRESTHOLD_OFFSET_VCS0_G12;
        break;
        // VCS1
    case MOS_GPU_CONTEXT_VDBOX2_VIDEO:
    case MOS_GPU_CONTEXT_VDBOX2_VIDEO2:
    case MOS_GPU_CONTEXT_VDBOX2_VIDEO3:
        MediaResetParam.watchdogCountCtrlOffset = WATCHDOG_COUNT_CTRL_OFFSET_VCS1_G12;
        MediaResetParam.watchdogCountThresholdOffset = WATCHDOG_COUNT_THRESTHOLD_OFFSET_VCS1_G12;
        break;
        // VECS
    case MOS_GPU_CONTEXT_VEBOX:
        MediaResetParam.watchdogCountCtrlOffset = WATCHDOG_COUNT_CTRL_OFFSET_VECS_G12;
        MediaResetParam.watchdogCountThresholdOffset = WATCHDOG_COUNT_THRESTHOLD_OFFSET_VECS_G12;
        break;
        // Default
    default:
        break;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG12::AddWatchdogTimerStartCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer)
{
    MOS_GPU_CONTEXT gpuContext;

    MHW_FUNCTION_ENTER;
    MHW_MI_CHK_NULL(m_osInterface);
    if (m_osInterface->bMediaReset == false ||
        m_osInterface->umdMediaResetEnable == false)
    {
        return MOS_STATUS_SUCCESS;
    }

    MHW_MI_CHK_NULL(cmdBuffer);

    // Set Watchdog Timer Register Offset
    gpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);
    MHW_MI_CHK_STATUS(SetWatchdogTimerRegisterOffset(gpuContext));

    // Send Stop before Start is to help recover from incorrect wdt state if previous submission 
    // cause hang and not have a chance to execute the stop cmd in the end of batch buffer. 
    MHW_MI_CHK_STATUS(AddWatchdogTimerStopCmd(cmdBuffer));

    //Configure Watchdog timer Threshold
    MHW_MI_LOAD_REGISTER_IMM_PARAMS registerImmParams;
    MOS_ZeroMemory(&registerImmParams, sizeof(registerImmParams));
    registerImmParams.dwData = MHW_MI_WATCHDOG_COUNTS_PER_MILLISECOND * MediaResetParam.watchdogCountThreshold *
        (m_osInterface->bSimIsActive ? 2 : 1);
    registerImmParams.dwRegister = MediaResetParam.watchdogCountThresholdOffset;
    MHW_MI_CHK_STATUS(AddMiLoadRegisterImmCmd(
        cmdBuffer,
        &registerImmParams));

    MHW_VERBOSEMESSAGE("MediaReset Threshold is %d", MediaResetParam.watchdogCountThreshold * (m_osInterface->bSimIsActive ? 2 : 1));

    //Start Watchdog Timer
    registerImmParams.dwData = MHW_MI_WATCHDOG_ENABLE_COUNTER;
    registerImmParams.dwRegister = MediaResetParam.watchdogCountCtrlOffset;
    MHW_MI_CHK_STATUS(AddMiLoadRegisterImmCmd(
        cmdBuffer,
        &registerImmParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG12::AddWatchdogTimerStopCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer)
{
    MOS_GPU_CONTEXT gpuContext;

    MHW_FUNCTION_ENTER;
    MHW_MI_CHK_NULL(m_osInterface);
    if (m_osInterface->bMediaReset == false ||
        m_osInterface->umdMediaResetEnable == false)
    {
        return MOS_STATUS_SUCCESS;
    }

    MHW_MI_CHK_NULL(cmdBuffer);

    // Set Watchdog Timer Register Offset
    gpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);
    MHW_MI_CHK_STATUS(SetWatchdogTimerRegisterOffset(gpuContext));

    //Stop Watchdog Timer
    MHW_MI_LOAD_REGISTER_IMM_PARAMS registerImmParams;
    MOS_ZeroMemory(&registerImmParams, sizeof(registerImmParams));
    registerImmParams.dwData = MHW_MI_WATCHDOG_DISABLE_COUNTER;
    registerImmParams.dwRegister = MediaResetParam.watchdogCountCtrlOffset;
    MHW_MI_CHK_STATUS(AddMiLoadRegisterImmCmd(
        cmdBuffer,
        &registerImmParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG12::AddMediaStateFlush(
    PMOS_COMMAND_BUFFER          cmdBuffer,
    PMHW_BATCH_BUFFER            batchBuffer,
    PMHW_MEDIA_STATE_FLUSH_PARAM params)
{
    MHW_MI_CHK_STATUS(MhwMiInterfaceGeneric<mhw_mi_g12_X>::AddMediaStateFlush(cmdBuffer, batchBuffer, params));

    mhw_mi_g12_X::MEDIA_STATE_FLUSH_CMD cmd;

    if (params != nullptr)
    {
        cmd.DW1.FlushToGo                 = params->bFlushToGo;
        cmd.DW1.InterfaceDescriptorOffset = params->ui8InterfaceDescriptorOffset;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, &cmd, cmd.byteSize));

#if (_DEBUG || _RELEASE_INTERNAL)
    if (batchBuffer)
    {
        batchBuffer->iLastCurrent = batchBuffer->iCurrent;
    }
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG12::SkipMiBatchBufferEndBb(
    PMHW_BATCH_BUFFER batchBuffer)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_STATUS(MhwMiInterfaceGeneric<mhw_mi_g12_X>::SkipMiBatchBufferEndBb(batchBuffer));
    MHW_MI_CHK_NULL(m_osInterface);
    auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);
    MHW_MI_CHK_NULL(waTable);

    // This WA does not apply for video or other engines, render requirement only
    bool isRender =
        MOS_RCS_ENGINE_USED(m_osInterface->pfnGetGpuContext(m_osInterface));

    if (isRender && (MEDIA_IS_WA(waTable, WaMSFWithNoWatermarkTSGHang) ||
                        MEDIA_IS_WA(waTable, WaAddMediaStateFlushCmd)))
    {
        mhw_mi_g12_X::MEDIA_STATE_FLUSH_CMD FlushCmd;
        MHW_MI_CHK_STATUS(Mhw_AddCommandBB(
            batchBuffer,
            nullptr,
            FlushCmd.byteSize));
    }

    mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD cmd;
    MHW_MI_CHK_STATUS(Mhw_AddCommandBB(
        batchBuffer,
        nullptr,
        cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG12::AddMiFlushDwCmd(
    PMOS_COMMAND_BUFFER     cmdBuffer,
    PMHW_MI_FLUSH_DW_PARAMS params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_STATUS(MhwMiInterfaceGeneric<mhw_mi_g12_X>::AddMiFlushDwCmd(cmdBuffer, params));

    mhw_mi_g12_X::MI_FLUSH_DW_CMD cmd;

    // set the protection bit based on CP status
    MHW_MI_CHK_STATUS(m_cpInterface->SetProtectionSettingsForMiFlushDw(m_osInterface, &cmd));

    cmd.DW0.VideoPipelineCacheInvalidate = params->bVideoPipelineCacheInvalidate;
    cmd.DW0.PostSyncOperation = cmd.POST_SYNC_OPERATION_NOWRITE;
    cmd.DW3_4.Value[0]        = params->dwDataDW1;

    if (params->pOsResource)
    {
        cmd.DW0.PostSyncOperation = cmd.POST_SYNC_OPERATION_WRITEIMMEDIATEDATA;
        cmd.DW1_2.DestinationAddressType = UseGlobalGtt.m_vcs;

        MHW_RESOURCE_PARAMS resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource = params->pOsResource;
        resourceParams.dwOffset     = params->dwResourceOffset;
        resourceParams.pdwCmd          = cmd.DW1_2.Value;
        resourceParams.dwLocationInCmd = 1;
        resourceParams.dwLsbNum        = MHW_COMMON_MI_FLUSH_DW_SHIFT;
        resourceParams.HwCommandType   = MOS_MI_FLUSH_DW;
        resourceParams.bIsWritable     = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->postSyncOperation)
    {
        cmd.DW0.PostSyncOperation = params->postSyncOperation;
    }

    if (params->dwDataDW2 || params->bQWordEnable)
    {
        cmd.DW3_4.Value[1] = params->dwDataDW2;
    }
    else
    {
        cmd.DW0.DwordLength--;
    }

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}