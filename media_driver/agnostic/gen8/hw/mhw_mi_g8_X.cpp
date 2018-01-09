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

MOS_STATUS MhwMiInterfaceG8::AddMiBatchBufferStartCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    PMHW_BATCH_BUFFER                   batchBuffer)
{
    MHW_FUNCTION_ENTER;

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
    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwMiInterfaceG8::AddMiConditionalBatchBufferEndCmd(
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
    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));

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

    mmioRegisters->generalPurposeRegister0LoOffset            = 0x1A600;
    mmioRegisters->generalPurposeRegister0HiOffset            = 0x1A604;
    mmioRegisters->generalPurposeRegister4LoOffset            = 0x1A620;
    mmioRegisters->generalPurposeRegister4HiOffset            = 0x1A624;
}
