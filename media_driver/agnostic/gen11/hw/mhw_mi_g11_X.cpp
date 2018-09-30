/*
* Copyright (c) 2016-2018, Intel Corporation
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
//! \file     mhw_mi_g11_X.cpp
//! \brief    Constructs MI commands on Gen11-based platforms
//! \details  Each client facing function both creates a HW command and adds
//!           that command to a command or batch buffer.
//!

#include "mhw_mi_g11_X.h"
#include "mhw_mi_hwcmd_g11_X.h"
#include "mhw_mmio_g11.h"

MOS_STATUS MhwMiInterfaceG11::AddMiSemaphoreWaitCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_MI_SEMAPHORE_WAIT_PARAMS   params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(cmdBuffer->pCmdPtr);
    MHW_MI_CHK_NULL(params);

    mhw_mi_g11_X::MI_SEMAPHORE_WAIT_CMD *cmd =
        (mhw_mi_g11_X::MI_SEMAPHORE_WAIT_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwMiInterfaceGeneric<mhw_mi_g11_X>::AddMiSemaphoreWaitCmd(cmdBuffer, params));

    cmd->DW0.RegisterPollMode = params->bRegisterPollMode;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG11::AddMiBatchBufferStartCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    PMHW_BATCH_BUFFER                   batchBuffer)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(batchBuffer);

    bool vcsEngineUsed =
        MOS_VCS_ENGINE_USED(m_osInterface->pfnGetGpuContext(m_osInterface));

    mhw_mi_g11_X::MI_BATCH_BUFFER_START_CMD cmd;
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
    cmd.DW0.SecondLevelBatchBuffer = true;
    cmd.DW0.AddressSpaceIndicator = !IsGlobalGttInUse();

    // Send BB start command
    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG11::AddMiConditionalBatchBufferEndCmd(
    PMOS_COMMAND_BUFFER                             cmdBuffer,
    PMHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS     params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->presSemaphoreBuffer);

    // Case 1 - Batch buffer condition matches - If this is not present then conditional 
    //          batch buffer will  exit to ring with terminating CP.
    // Case 2 - Batch buffer condition DOES NOT match - Although this will disable CP 
    //          but after end of conditional batch buffer CP will be re-enabled.
    MHW_MI_CHK_STATUS(m_cpInterface->AddEpilog(m_osInterface, cmdBuffer));

    mhw_mi_g11_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD cmd;
    cmd.DW0.UseGlobalGtt        = IsGlobalGttInUse();
    cmd.DW0.CompareSemaphore    = 1; // CompareDataDword is always assumed to be set
    cmd.DW0.CompareMaskMode     = !params->bDisableCompareMask; 
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
    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));

    //Re-enable CP for Case 2
    MHW_MI_CHK_STATUS(m_cpInterface->AddProlog(m_osInterface, cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG11::AddMiStoreRegisterMemCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    PMHW_MI_STORE_REGISTER_MEM_PARAMS   params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(cmdBuffer->pCmdPtr);
    MHW_MI_CHK_NULL(params);

    mhw_mi_g11_X::MI_STORE_REGISTER_MEM_CMD *cmd =
        (mhw_mi_g11_X::MI_STORE_REGISTER_MEM_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwMiInterfaceGeneric<mhw_mi_g11_X>::AddMiStoreRegisterMemCmd(cmdBuffer, params));

    if (IsRelativeMMIO(params->dwRegister))
    {
        cmd->DW0.AddCsMmioStartOffset = 1;
        cmd->DW1.RegisterAddress = params->dwRegister >> 2;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG11::AddMiLoadRegisterMemCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    PMHW_MI_LOAD_REGISTER_MEM_PARAMS    params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(cmdBuffer->pCmdPtr);
    MHW_MI_CHK_NULL(params);

    mhw_mi_g11_X::MI_LOAD_REGISTER_MEM_CMD *cmd =
        (mhw_mi_g11_X::MI_LOAD_REGISTER_MEM_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwMiInterfaceGeneric<mhw_mi_g11_X>::AddMiLoadRegisterMemCmd(cmdBuffer, params));

    if (IsRelativeMMIO(params->dwRegister))
    {
        cmd->DW0.AddCsMmioStartOffset = 1;
        cmd->DW1.RegisterAddress = params->dwRegister >> 2;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG11::AddMiLoadRegisterImmCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    PMHW_MI_LOAD_REGISTER_IMM_PARAMS    params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(cmdBuffer->pCmdPtr);
    MHW_MI_CHK_NULL(params);

    mhw_mi_g11_X::MI_LOAD_REGISTER_IMM_CMD *cmd =
        (mhw_mi_g11_X::MI_LOAD_REGISTER_IMM_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwMiInterfaceGeneric<mhw_mi_g11_X>::AddMiLoadRegisterImmCmd(cmdBuffer, params));

    if (IsRelativeMMIO(params->dwRegister))
    {
        cmd->DW0.AddCsMmioStartOffset = 1;
        cmd->DW1.RegisterOffset = params->dwRegister >> 2;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG11::AddMiLoadRegisterRegCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    PMHW_MI_LOAD_REGISTER_REG_PARAMS    params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(cmdBuffer->pCmdPtr);
    MHW_MI_CHK_NULL(params);

    mhw_mi_g11_X::MI_LOAD_REGISTER_REG_CMD *cmd =
        (mhw_mi_g11_X::MI_LOAD_REGISTER_REG_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwMiInterfaceGeneric<mhw_mi_g11_X>::AddMiLoadRegisterRegCmd(cmdBuffer, params));

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

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG11::SetWatchdogTimerThreshold(uint32_t frameWidth, uint32_t frameHeight)
{
    MHW_FUNCTION_ENTER;

    if (m_osInterface->bMediaReset == false || 
        m_osInterface->umdMediaResetEnable == false)
    {
        return MOS_STATUS_SUCCESS;
    }

    if ((frameWidth * frameHeight) >= (7680 * 4320))
    {
        MediaResetParam.watchdogCountThreshold = MHW_MI_16K_WATCHDOG_THRESHOLD_IN_MS;
    }
    else if ((frameWidth * frameHeight) >= (3840 * 2160))
    {
        MediaResetParam.watchdogCountThreshold = MHW_MI_8K_WATCHDOG_THRESHOLD_IN_MS;
    }
    else if ((frameWidth * frameHeight) >= (1920 * 1080))
    {
        MediaResetParam.watchdogCountThreshold = MHW_MI_4K_WATCHDOG_THRESHOLD_IN_MS;
    }
    else
    {
        MediaResetParam.watchdogCountThreshold = MHW_MI_FHD_WATCHDOG_THRESHOLD_IN_MS;
    }

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
    // User feature config of watchdog timer threshold
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_MEDIA_RESET_TH_ID,
        &userFeatureData);
    if (userFeatureData.u32Data != 0)
    {
        MediaResetParam.watchdogCountThreshold = userFeatureData.u32Data;
    }
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG11::SetWatchdogTimerRegisterOffset(
    MOS_GPU_CONTEXT                 gpuContext)
{
    MHW_FUNCTION_ENTER;

    if (m_osInterface->bMediaReset == false ||
        m_osInterface->umdMediaResetEnable == false)
    {
        return MOS_STATUS_SUCCESS;
    }

    switch (gpuContext)
    {
        // RCS
    case MOS_GPU_CONTEXT_RENDER:
    case MOS_GPU_CONTEXT_RENDER2:
    case MOS_GPU_CONTEXT_RENDER3:
    case MOS_GPU_CONTEXT_RENDER4:
        MediaResetParam.watchdogCountCtrlOffset = WATCHDOG_COUNT_CTRL_OFFSET_RCS_G11;
        MediaResetParam.watchdogCountThresholdOffset = WATCHDOG_COUNT_THRESTHOLD_OFFSET_RCS_G11;
        break;
        // VCS0
    case MOS_GPU_CONTEXT_VIDEO:
    case MOS_GPU_CONTEXT_VIDEO2:
    case MOS_GPU_CONTEXT_VIDEO3:
    case MOS_GPU_CONTEXT_VIDEO4:
        MediaResetParam.watchdogCountCtrlOffset = WATCHDOG_COUNT_CTRL_OFFSET_VCS0_G11;
        MediaResetParam.watchdogCountThresholdOffset = WATCHDOG_COUNT_THRESTHOLD_OFFSET_VCS0_G11;
        break;
        // VCS1
    case MOS_GPU_CONTEXT_VDBOX2_VIDEO:
    case MOS_GPU_CONTEXT_VDBOX2_VIDEO2:
    case MOS_GPU_CONTEXT_VDBOX2_VIDEO3:
        MediaResetParam.watchdogCountCtrlOffset = WATCHDOG_COUNT_CTRL_OFFSET_VCS1_G11;
        MediaResetParam.watchdogCountThresholdOffset = WATCHDOG_COUNT_THRESTHOLD_OFFSET_VCS1_G11;
        break;
        // VECS
    case MOS_GPU_CONTEXT_VEBOX:
        MediaResetParam.watchdogCountCtrlOffset = WATCHDOG_COUNT_CTRL_OFFSET_VECS_G11;
        MediaResetParam.watchdogCountThresholdOffset = WATCHDOG_COUNT_THRESTHOLD_OFFSET_VECS_G11;
        break;
        // Default
    default:
        break;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG11::AddWatchdogTimerStartCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer)
{
    MOS_GPU_CONTEXT     gpuContext;

    MHW_FUNCTION_ENTER;

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

MOS_STATUS MhwMiInterfaceG11::AddWatchdogTimerStopCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer)
{
    MOS_GPU_CONTEXT gpuContext;

    MHW_FUNCTION_ENTER;

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

void MhwMiInterfaceG11::InitMmioRegisters()
{
    MHW_MI_MMIOREGISTERS *mmioRegisters = &m_mmioRegisters;

    mmioRegisters->generalPurposeRegister0LoOffset            = GP_REGISTER0_LO_OFFSET_G11;
    mmioRegisters->generalPurposeRegister0HiOffset            = GP_REGISTER0_HI_OFFSET_G11;
    mmioRegisters->generalPurposeRegister4LoOffset            = GP_REGISTER4_LO_OFFSET_G11;
    mmioRegisters->generalPurposeRegister4HiOffset            = GP_REGISTER4_HI_OFFSET_G11;
}
