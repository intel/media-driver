/*
* Copyright (c) 2014-2018, Intel Corporation
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
//! \file     mhw_mi_g8_X.cpp
//! \brief    Constructs MI commands on Gen8-based platforms
//! \details  Each client facing function both creates a HW command and adds
//!           that command to a command or batch buffer.
//!

#include "mhw_mi_g8_X.h"
#include "mhw_mi_hwcmd_g8_X.h"
#include "mhw_mmio_g8.h"

MOS_STATUS MhwMiInterfaceG8::AddMiBatchBufferStartCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    PMHW_BATCH_BUFFER                   batchBuffer)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(batchBuffer);

    bool vcsEngineUsed =
        MOS_VCS_ENGINE_USED(m_osInterface->pfnGetGpuContext(m_osInterface));

    mhw_mi_g8_X::MI_BATCH_BUFFER_START_CMD  cmd;
    MHW_RESOURCE_PARAMS                     resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.presResource     = &batchBuffer->OsResource;
    resourceParams.dwOffset         = batchBuffer->dwOffset;
    resourceParams.pdwCmd           = &(cmd.DW1.Value);
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
    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG8::AddMiConditionalBatchBufferEndCmd(
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

    mhw_mi_g8_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD cmd;
    cmd.DW0.UseGlobalGtt = IsGlobalGttInUse();
    cmd.DW0.CompareSemaphore = 1; // CompareDataDword is always assumed to be set
    cmd.DW1.CompareDataDword = params->dwValue;

    MHW_RESOURCE_PARAMS resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.presResource     = params->presSemaphoreBuffer;
    resourceParams.dwOffset         = params->dwOffset;
    resourceParams.pdwCmd           = &(cmd.DW2.Value);
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

MOS_STATUS MhwMiInterfaceG8::AddMediaStateFlush(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_BATCH_BUFFER               batchBuffer,
    PMHW_MEDIA_STATE_FLUSH_PARAM    params)
{
    MHW_FUNCTION_ENTER;

    mhw_mi_g8_X::MEDIA_STATE_FLUSH_CMD *cmd = nullptr;

    if (cmdBuffer)
    {
        MHW_MI_CHK_NULL(cmdBuffer->pCmdPtr);
        cmd = (mhw_mi_g8_X::MEDIA_STATE_FLUSH_CMD*)cmdBuffer->pCmdPtr;
    }
    else if (batchBuffer)
    {
        MHW_MI_CHK_NULL(batchBuffer->pData);
        cmd = (mhw_mi_g8_X::MEDIA_STATE_FLUSH_CMD*)(batchBuffer->pData + batchBuffer->iCurrent);
    }
    else
    {
        MHW_ASSERTMESSAGE("No valid buffer to add the command to!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MHW_MI_CHK_STATUS(MhwMiInterfaceGeneric<mhw_mi_g8_X>::AddMediaStateFlush(cmdBuffer, batchBuffer, params));

    mhw_mi_g8_X::MEDIA_STATE_FLUSH_CMD AddMediaStateFlushcmd;

    if (params != nullptr)
    {
        AddMediaStateFlushcmd.DW1.FlushToGo = params->bFlushToGo;
        AddMediaStateFlushcmd.DW1.InterfaceDescriptorOffset = params->ui8InterfaceDescriptorOffset;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, &AddMediaStateFlushcmd, AddMediaStateFlushcmd.byteSize));

#if (_DEBUG || _RELEASE_INTERNAL)
    if (batchBuffer)
    {
        batchBuffer->iLastCurrent = batchBuffer->iCurrent;
    }
#endif

    auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);
    MHW_MI_CHK_NULL(waTable);
    MHW_MI_CHK_NULL(cmd);

    if (MEDIA_IS_WA(waTable, WaMSFWithNoWatermarkTSGHang))
    {
        cmd->DW1.WatermarkRequired = 0;
        cmd->DW1.FlushToGo = 1;
    }

    return MOS_STATUS_SUCCESS;
}

void MhwMiInterfaceG8::InitMmioRegisters()
{
    MHW_MI_MMIOREGISTERS *mmioRegisters = &m_mmioRegisters;

    mmioRegisters->generalPurposeRegister0LoOffset            = GP_REGISTER0_LO_OFFSET_G8;
    mmioRegisters->generalPurposeRegister0HiOffset            = GP_REGISTER0_HI_OFFSET_G8;
    mmioRegisters->generalPurposeRegister4LoOffset            = GP_REGISTER4_LO_OFFSET_G8;
    mmioRegisters->generalPurposeRegister4HiOffset            = GP_REGISTER4_HI_OFFSET_G8;
    mmioRegisters->generalPurposeRegister11LoOffset           = GP_REGISTER11_LO_OFFSET_G8;
    mmioRegisters->generalPurposeRegister11HiOffset           = GP_REGISTER11_HI_OFFSET_G8;
    mmioRegisters->generalPurposeRegister12LoOffset           = GP_REGISTER12_LO_OFFSET_G8;
    mmioRegisters->generalPurposeRegister12HiOffset           = GP_REGISTER12_HI_OFFSET_G8;
}

MOS_STATUS MhwMiInterfaceG8::SkipMiBatchBufferEndBb(
    PMHW_BATCH_BUFFER               batchBuffer)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_STATUS(MhwMiInterfaceGeneric<mhw_mi_g8_X>::SkipMiBatchBufferEndBb(batchBuffer));

    auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);
    MHW_MI_CHK_NULL(waTable);

    // This WA does not apply for video or other engines, render requirement only
    bool isRender =
        MOS_RCS_ENGINE_USED(m_osInterface->pfnGetGpuContext(m_osInterface));

    if (isRender && (MEDIA_IS_WA(waTable, WaMSFWithNoWatermarkTSGHang) ||
                        MEDIA_IS_WA(waTable, WaAddMediaStateFlushCmd)))
    {
        mhw_mi_g8_X::MEDIA_STATE_FLUSH_CMD FlushCmd;
        MHW_MI_CHK_STATUS(Mhw_AddCommandBB(
            batchBuffer,
            nullptr,
            FlushCmd.byteSize));
    }

    mhw_mi_g8_X::MI_BATCH_BUFFER_END_CMD cmd;
    MHW_MI_CHK_STATUS(Mhw_AddCommandBB(
        batchBuffer,
        nullptr,
        cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG8::AddMiFlushDwCmd(
    PMOS_COMMAND_BUFFER     cmdBuffer,
    PMHW_MI_FLUSH_DW_PARAMS params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_STATUS(MhwMiInterfaceGeneric<mhw_mi_g8_X>::AddMiFlushDwCmd(cmdBuffer, params));

    mhw_mi_g8_X::MI_FLUSH_DW_CMD cmd;

    // set the protection bit based on CP status
    MHW_MI_CHK_STATUS(m_cpInterface->SetProtectionSettingsForMiFlushDw(m_osInterface, &cmd));

    cmd.DW0.VideoPipelineCacheInvalidate = params->bVideoPipelineCacheInvalidate;
    cmd.DW0.PostSyncOperation            = cmd.POST_SYNC_OPERATION_NOWRITE;
    cmd.DW3_4.Value[0]                   = params->dwDataDW1;

    if (params->pOsResource)
    {
        cmd.DW0.PostSyncOperation        = cmd.POST_SYNC_OPERATION_WRITEIMMEDIATEDATA;
        cmd.DW1_2.DestinationAddressType = UseGlobalGtt.m_vcs;

        MHW_RESOURCE_PARAMS resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource    = params->pOsResource;
        resourceParams.dwOffset        = params->dwResourceOffset;
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
