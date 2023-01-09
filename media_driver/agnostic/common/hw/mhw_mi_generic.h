/*
* Copyright (c) 2015-2018, Intel Corporation
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
//! \file     mhw_mi_generic.h
//! \brief    MHW interface templates for MI and generic flush commands across all engines
//! \details  Impelements shared HW command construction functions across all platforms as templates
//!

#ifndef __MHW_MI_GENERIC_H__
#define __MHW_MI_GENERIC_H__

#include "mhw_mi.h"
#include "mhw_cp_interface.h"
#include "mos_os_cp_interface_specific.h"

template <class TMiCmds>
class MhwMiInterfaceGeneric : public MhwMiInterface
{
private:
    enum AtomicOpCode
    {
        atomicInvalid = 0,
        atomicAnd = 1,
        atomicOr = 2,
        atomicXor = 3,
        atomicMove = 4,
        atomicInc = 5,
        atomicDec = 6,
        atomicAdd = 7,
        atomicSub = 8,
        atomicRSub = 9,
        atomicIMax = 10,
        atomicIMin = 11,
        atomicUMax = 12,
        atomicUMin = 13,
        atomicCmp = 14,
        atomicDword = 0,
        atomicQword = 0x20,
        atomicOctword = 0x40,
    };

    //!
    //! \brief    Helper function to compose opcode for MI_ATOMIC
    //! \return   uint32_t
    //!           Composed opcode for MI_ATOMIC or 0
    //!
    uint32_t CreateMiAtomicOpcode(
        uint32_t                    dataSize,
        MHW_COMMON_MI_ATOMIC_OPCODE opCode)
    {
        uint32_t formattedOpCode = dataSize;

        switch (opCode) {
        case MHW_MI_ATOMIC_AND:
            formattedOpCode += atomicAnd;
            break;
        case MHW_MI_ATOMIC_OR:
            formattedOpCode += atomicOr;
            break;
        case MHW_MI_ATOMIC_XOR:
            formattedOpCode += atomicXor;
            break;
        case MHW_MI_ATOMIC_MOVE:
            formattedOpCode += atomicMove;
            break;
        case MHW_MI_ATOMIC_INC:
            formattedOpCode += atomicInc;
            break;
        case MHW_MI_ATOMIC_DEC:
            formattedOpCode += atomicDec;
            break;
        case MHW_MI_ATOMIC_ADD:
            formattedOpCode += atomicAdd;
            break;
        case MHW_MI_ATOMIC_SUB:
            formattedOpCode += atomicSub;
            break;
        case MHW_MI_ATOMIC_RSUB:
            formattedOpCode += atomicRSub;
            break;
        case MHW_MI_ATOMIC_IMAX:
            formattedOpCode += atomicIMax;
            break;
        case MHW_MI_ATOMIC_IMIN:
            formattedOpCode += atomicIMin;
            break;
        case MHW_MI_ATOMIC_UMAX:
            formattedOpCode += atomicUMax;
            break;
        case MHW_MI_ATOMIC_UMIN:
            formattedOpCode += atomicUMin;
            break;
        case MHW_MI_ATOMIC_CMP:
            formattedOpCode += atomicCmp;
            break;
        default:
            formattedOpCode = atomicInvalid;
            break;
        }

        return formattedOpCode;
    }

    MOS_STATUS SendMarkerCommand(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool isRender)
    {
        MOS_STATUS      eStatus     = MOS_STATUS_SUCCESS;
        PMOS_RESOURCE   resMarker   = nullptr;

        MHW_FUNCTION_ENTER;

        resMarker = m_osInterface->pfnGetMarkerResource(m_osInterface);
        MHW_CHK_NULL_RETURN(resMarker);

        if (isRender)
        {
            // Send pipe_control to get the timestamp
            MHW_PIPE_CONTROL_PARAMS             pipeControlParams;
            MOS_ZeroMemory(&pipeControlParams, sizeof(pipeControlParams));
            pipeControlParams.presDest          = resMarker;
            pipeControlParams.dwResourceOffset  = sizeof(uint64_t);
            pipeControlParams.dwPostSyncOp      = MHW_FLUSH_WRITE_TIMESTAMP_REG;
            pipeControlParams.dwFlushMode       = MHW_FLUSH_WRITE_CACHE;

            MHW_MI_CHK_STATUS(AddPipeControl(cmdBuffer, NULL, &pipeControlParams));
        }
        else
        {
            // Send flush_dw to get the timestamp
            MHW_MI_FLUSH_DW_PARAMS  flushDwParams;
            MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
            flushDwParams.pOsResource           = resMarker;
            flushDwParams.dwResourceOffset      = sizeof(uint64_t);
            flushDwParams.postSyncOperation     = MHW_FLUSH_WRITE_TIMESTAMP_REG;
            flushDwParams.bQWordEnable          = 1;

            MHW_MI_CHK_STATUS(AddMiFlushDwCmd(cmdBuffer, &flushDwParams));
        }
        if (!m_osInterface->apoMosEnabled)
        {
            MOS_SafeFreeMemory(resMarker);
        }
        return eStatus;
    }

protected:
    MhwMiInterfaceGeneric(
        MhwCpInterface      *cpInterface,
        PMOS_INTERFACE      osInterface) : MhwMiInterface(cpInterface, osInterface) {}

public:
    virtual ~MhwMiInterfaceGeneric() { MHW_FUNCTION_ENTER; }

    MOS_STATUS AddMiNoop(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER   batchBuffer)
    {
        MHW_FUNCTION_ENTER;

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
            return MOS_STATUS_NULL_POINTER;
        }

        typename TMiCmds::MI_NOOP_CMD cmd;
        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(
            m_osInterface,
            cmdBuffer,
            batchBuffer,
            &cmd,
            cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMiBatchBufferEnd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer)
    {
        MHW_FUNCTION_ENTER;

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
            return MOS_STATUS_NULL_POINTER;
        }

        auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);
        MHW_MI_CHK_NULL(waTable);

        // This WA does not apply for video or other engines, render requirement only
        bool isRender =
            MOS_RCS_ENGINE_USED(m_osInterface->pfnGetGpuContext(m_osInterface));

        if (isRender &&
            (MEDIA_IS_WA(waTable, WaMSFWithNoWatermarkTSGHang) ||
            MEDIA_IS_WA(waTable, WaAddMediaStateFlushCmd)))
        {
            MHW_MI_CHK_STATUS(AddMediaStateFlush(cmdBuffer, batchBuffer));
        }

        // Mhw_CommonMi_AddMiBatchBufferEnd() is designed to handle both 1st level
        // and 2nd level BB.  It inserts MI_BATCH_BUFFER_END in both cases.
        // However, since the 2nd level BB always returens to the 1st level BB and
        // no chained BB scenario in Media, Epilog is only needed in the 1st level BB.
        // Therefre, here only the 1st level BB case needs an Epilog inserted.
        if (cmdBuffer && cmdBuffer->is1stLvlBB)
        {
            MHW_MI_CHK_STATUS(m_cpInterface->AddEpilog(m_osInterface, cmdBuffer));
        }

        typename TMiCmds::MI_BATCH_BUFFER_END_CMD cmd;
        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(
            m_osInterface,
            cmdBuffer,
            batchBuffer,
            &cmd,
            cmd.byteSize));

        if (!cmdBuffer) // Don't need BB not nullptr chk b/c if both are nullptr it won't get this far
        {
    #if (_DEBUG || _RELEASE_INTERNAL)
            batchBuffer->iLastCurrent = batchBuffer->iCurrent;
    #endif
        }

        // Send End Marker command
        if (m_osInterface->pfnIsSetMarkerEnabled(m_osInterface) && cmdBuffer && cmdBuffer->is1stLvlBB)
        {
            MHW_MI_CHK_STATUS(SendMarkerCommand(
                cmdBuffer, MOS_RCS_ENGINE_USED(m_osInterface->pfnGetGpuContext(m_osInterface))));
        }

        MHW_MI_CHK_STATUS(m_osInterface->osCpInterface->PermeateBBPatchForHM());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMiBatchBufferEndOnly(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer)
    {
        MHW_FUNCTION_ENTER;

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
            return MOS_STATUS_NULL_POINTER;
        }

        // Mhw_CommonMi_AddMiBatchBufferEnd() is designed to handle both 1st level
        // and 2nd level BB.  It inserts MI_BATCH_BUFFER_END in both cases.
        // However, since the 2nd level BB always returens to the 1st level BB and
        // no chained BB scenario in Media, Epilog is only needed in the 1st level BB.
        // Therefre, here only the 1st level BB case needs an Epilog inserted.
        if (cmdBuffer && cmdBuffer->is1stLvlBB)
        {
            MHW_MI_CHK_STATUS(m_cpInterface->AddEpilog(m_osInterface, cmdBuffer));
        }

        typename TMiCmds::MI_BATCH_BUFFER_END_CMD cmd;
        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(
            m_osInterface,
            cmdBuffer,
            batchBuffer,
            &cmd,
            cmd.byteSize));

        if (!cmdBuffer) // Don't need BB not nullptr chk b/c if both are nullptr it won't get this far
        {
    #if (_DEBUG || _RELEASE_INTERNAL)
            batchBuffer->iLastCurrent = batchBuffer->iCurrent;
    #endif
        }

        // Send End Marker command
        if (m_osInterface->pfnIsSetMarkerEnabled(m_osInterface) && cmdBuffer && cmdBuffer->is1stLvlBB)
        {
            MHW_MI_CHK_STATUS(SendMarkerCommand(
                cmdBuffer, MOS_RCS_ENGINE_USED(m_osInterface->pfnGetGpuContext(m_osInterface))));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMiStoreDataImmCmd(
        PMOS_COMMAND_BUFFER         cmdBuffer,
        PMHW_MI_STORE_DATA_PARAMS   params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pOsResource);

        typename TMiCmds::MI_STORE_DATA_IMM_CMD cmd;
        MHW_RESOURCE_PARAMS                 resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource     = params->pOsResource;
        resourceParams.dwOffset         = params->dwResourceOffset;
        resourceParams.pdwCmd           = cmd.DW1_2.Value;
        resourceParams.dwLocationInCmd  = 1;
        resourceParams.dwLsbNum         = MHW_COMMON_MI_STORE_DATA_DW_SHIFT;
        resourceParams.HwCommandType    = MOS_MI_STORE_DATA_IMM;
        resourceParams.bIsWritable      = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.DW0.UseGlobalGtt = IsGlobalGttInUse();
        // Force single DW write, driver never writes a QW
        cmd.DW0.StoreQword = 0;
        cmd.DW0.DwordLength--;

        cmd.DW3.DataDword0 = params->dwValue;

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMiFlushDwCmd(
        PMOS_COMMAND_BUFFER         cmdBuffer,
        PMHW_MI_FLUSH_DW_PARAMS     params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMiCopyMemMemCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_COPY_MEM_MEM_PARAMS         params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->presSrc);
        MHW_MI_CHK_NULL(params->presDst);

        typename TMiCmds::MI_COPY_MEM_MEM_CMD cmd;
        cmd.DW0.UseGlobalGttDestination = IsGlobalGttInUse();
        cmd.DW0.UseGlobalGttSource      = IsGlobalGttInUse();

        MHW_RESOURCE_PARAMS resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource     = params->presDst;
        resourceParams.dwOffset         = params->dwDstOffset;
        resourceParams.pdwCmd           = cmd.DW1_2.Value;
        resourceParams.dwLocationInCmd  = 1;
        resourceParams.dwLsbNum         = MHW_COMMON_MI_GENERAL_SHIFT;
        resourceParams.HwCommandType    = MOS_MI_COPY_MEM_MEM;
        resourceParams.bIsWritable      = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource     = params->presSrc;
        resourceParams.dwOffset         = params->dwSrcOffset;
        resourceParams.pdwCmd           = cmd.DW3_4.Value;
        resourceParams.dwLocationInCmd  = 3;
        resourceParams.dwLsbNum         = MHW_COMMON_MI_GENERAL_SHIFT;
        resourceParams.HwCommandType    = MOS_MI_COPY_MEM_MEM;
        resourceParams.bIsWritable      = false;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMiStoreRegisterMemCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_STORE_REGISTER_MEM_PARAMS   params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->presStoreBuffer);

        typename TMiCmds::MI_STORE_REGISTER_MEM_CMD  cmd;
        MHW_RESOURCE_PARAMS                 resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
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

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMiLoadRegisterMemCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_LOAD_REGISTER_MEM_PARAMS    params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->presStoreBuffer);

        typename TMiCmds::MI_LOAD_REGISTER_MEM_CMD   cmd;
        MHW_RESOURCE_PARAMS                 resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource     = params->presStoreBuffer;
        resourceParams.dwOffset         = params->dwOffset;
        resourceParams.pdwCmd           = cmd.DW2_3.Value;
        resourceParams.dwLocationInCmd  = 2;
        resourceParams.dwLsbNum         = MHW_COMMON_MI_GENERAL_SHIFT;
        resourceParams.HwCommandType    = MOS_MI_LOAD_REGISTER_MEM;
        resourceParams.bIsWritable      = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.DW0.UseGlobalGtt    = IsGlobalGttInUse();
        cmd.DW1.RegisterAddress = params->dwRegister >> 2;

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMiLoadRegisterImmCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_LOAD_REGISTER_IMM_PARAMS    params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TMiCmds::MI_LOAD_REGISTER_IMM_CMD cmd;
        cmd.DW1.RegisterOffset = params->dwRegister >> 2;
        cmd.DW2.DataDword = params->dwData;

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMiLoadRegisterRegCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_LOAD_REGISTER_REG_PARAMS    params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TMiCmds::MI_LOAD_REGISTER_REG_CMD cmd;
        cmd.DW1.SourceRegisterAddress = params->dwSrcRegister >> 2;
        cmd.DW2.DestinationRegisterAddress = params->dwDstRegister >> 2;

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMiMathCmd(
        PMOS_COMMAND_BUFFER      cmdBuffer,
        PMHW_MI_MATH_PARAMS      params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        if (params->dwNumAluParams == 0 || params->pAluPayload == nullptr)
        {
            MHW_ASSERTMESSAGE("MI_MATH requires a valid payload");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        typename TMiCmds::MI_MATH_CMD cmd;
        cmd.DW0.DwordLength = params->dwNumAluParams - 1;

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(
            cmdBuffer,
            &params->pAluPayload[0],
            sizeof(MHW_MI_ALU_PARAMS)* params->dwNumAluParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMiSetPredicateCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        MHW_MI_SET_PREDICATE_ENABLE         enableFlag)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);

        typename TMiCmds::MI_SET_PREDICATE_CMD cmd;
        cmd.DW0.PredicateEnable = enableFlag;
        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMiAtomicCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_ATOMIC_PARAMS               params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pOsResource);

        typename TMiCmds::MI_ATOMIC_CMD  cmd;
        MHW_RESOURCE_PARAMS     resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource = params->pOsResource;
        resourceParams.dwOffset = params->dwResourceOffset;
        resourceParams.pdwCmd = &(cmd.DW1.Value);
        resourceParams.dwLocationInCmd = 1;
        resourceParams.dwLsbNum = MHW_COMMON_MI_GENERAL_SHIFT;
        resourceParams.HwCommandType = MOS_MI_ATOMIC;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.DW0.DwordLength = params->bInlineData ? 1 : 9;
        cmd.DW0.MemoryType = IsGlobalGttInUse();
        cmd.DW0.ReturnDataControl = params->bReturnData;
        if (params->dwDataSize == sizeof(uint32_t))
        {
            cmd.DW0.DataSize = cmd.DATA_SIZE_DWORD;
        }
        else if (params->dwDataSize == sizeof(uint64_t))
        {
            cmd.DW0.DataSize = cmd.DATA_SIZE_QWORD;
        }
        else if (params->dwDataSize == sizeof(uint64_t)* 2)
        {
            cmd.DW0.DataSize = cmd.DATA_SIZE_OCTWORD;
        }
        else
        {
            MHW_ASSERTMESSAGE("Invalid data size provided");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (cmd.DW0.DataSize == cmd.DATA_SIZE_QWORD)
        {
            cmd.DW0.AtomicOpcode = atomicQword;
        }
        else if (cmd.DW0.DataSize == cmd.DATA_SIZE_OCTWORD)
        {
            if (params->Operation != MHW_MI_ATOMIC_CMP)
            {
                MHW_ASSERTMESSAGE("An OCTWORD may only be used in the case of a compare operation!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            cmd.DW0.AtomicOpcode = atomicOctword;
        }
        cmd.DW0.AtomicOpcode = CreateMiAtomicOpcode(
            cmd.DW0.AtomicOpcode,
            params->Operation);
        if (cmd.DW0.AtomicOpcode == atomicInvalid)
        {
            MHW_ASSERTMESSAGE("No MI_ATOMIC opcode could be generated");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        cmd.DW0.InlineData = params->bInlineData;
        cmd.DW0.DwordLength = params->bInlineData ? 9 : 1;

        if (params->bInlineData)
        {
            cmd.DW3.Operand1DataDword0 = params->dwOperand1Data[0];
            cmd.DW4.Operand2DataDword0 = params->dwOperand2Data[0];
            cmd.DW5.Operand1DataDword1 = params->dwOperand1Data[1];
            cmd.DW6.Operand2DataDword1 = params->dwOperand2Data[1];
            cmd.DW7.Operand1DataDword2 = params->dwOperand1Data[2];
            cmd.DW8.Operand2DataDword2 = params->dwOperand2Data[3];
            cmd.DW9.Operand1DataDword3 = params->dwOperand1Data[3];
            cmd.DW10.Operand2DataDword3 = params->dwOperand2Data[3];
        }

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMiSemaphoreWaitCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_MI_SEMAPHORE_WAIT_PARAMS   params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->presSemaphoreMem);

        typename TMiCmds::MI_SEMAPHORE_WAIT_CMD  cmd;
        MHW_RESOURCE_PARAMS             resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource     = params->presSemaphoreMem;
        resourceParams.dwOffset         = params->dwResourceOffset;
        resourceParams.pdwCmd           = cmd.DW2_3.Value;
        resourceParams.dwLocationInCmd  = 2;
        resourceParams.dwLsbNum         = MHW_COMMON_MI_GENERAL_SHIFT;
        resourceParams.HwCommandType    = MOS_MI_SEMAPHORE_WAIT;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.DW0.MemoryType          = IsGlobalGttInUse();
        cmd.DW0.WaitMode            = params->bPollingWaitMode;

        cmd.DW0.CompareOperation    = params->CompareOperation;
        cmd.DW1.SemaphoreDataDword  = params->dwSemaphoreData;

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMiArbCheckCmd(
        PMOS_COMMAND_BUFFER         cmdBuffer)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);

        typename TMiCmds::MI_ARB_CHECK_CMD cmd;
        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddPipeControl(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        PMHW_PIPE_CONTROL_PARAMS        params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
            return MOS_STATUS_NULL_POINTER;
        }

        typename TMiCmds::PIPE_CONTROL_CMD cmd;
        cmd.DW1.PipeControlFlushEnable      = true;
        cmd.DW1.CommandStreamerStallEnable  = !params->bDisableCSStall;
        cmd.DW4_5.Value[0]                  = params->dwDataDW1;
        cmd.DW4_5.Value[1]                  = params->dwDataDW2;

        if (params->presDest)
        {
            cmd.DW1.PostSyncOperation       = params->dwPostSyncOp;
            cmd.DW1.DestinationAddressType  = UseGlobalGtt.m_cs;

            MHW_RESOURCE_PARAMS resourceParams;
            MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
            resourceParams.presResource     = params->presDest;
            resourceParams.dwOffset         = params->dwResourceOffset;
            resourceParams.pdwCmd           = &(cmd.DW2.Value);
            resourceParams.dwLocationInCmd  = 2;
            resourceParams.dwLsbNum         = MHW_COMMON_MI_PIPE_CONTROL_SHIFT;
            resourceParams.bIsWritable      = true;
            resourceParams.HwCommandType    = MOS_PIPE_CONTROL;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
        else
        {
            cmd.DW1.StateCacheInvalidationEnable        = true;
            cmd.DW1.ConstantCacheInvalidationEnable     = true;
            cmd.DW1.VfCacheInvalidationEnable           = true;
            cmd.DW1.InstructionCacheInvalidateEnable    = true;
            cmd.DW1.RenderTargetCacheFlushEnable        = true;
            cmd.DW1.PostSyncOperation                   = cmd.POST_SYNC_OPERATION_NOWRITE;
        }

        // Cache flush mode
        switch (params->dwFlushMode)
        {
            // Flush all Write caches
            case MHW_FLUSH_WRITE_CACHE:
                cmd.DW1.RenderTargetCacheFlushEnable        = true;
                cmd.DW1.DcFlushEnable                       = true;
                break;

            // Invalidate all Read-only caches
            case MHW_FLUSH_READ_CACHE:
                cmd.DW1.RenderTargetCacheFlushEnable        = false;
                cmd.DW1.StateCacheInvalidationEnable        = true;
                cmd.DW1.ConstantCacheInvalidationEnable     = true;
                cmd.DW1.VfCacheInvalidationEnable           = true;
                cmd.DW1.InstructionCacheInvalidateEnable    = true;
                break;

            // Custom flush parameters
            case MHW_FLUSH_CUSTOM:
                cmd.DW1.RenderTargetCacheFlushEnable      = params->bFlushRenderTargetCache;
                cmd.DW1.DcFlushEnable                     = params->bFlushRenderTargetCache; // same as above
                cmd.DW1.StateCacheInvalidationEnable      = params->bInvalidateStateCache;
                cmd.DW1.ConstantCacheInvalidationEnable   = params->bInvalidateConstantCache;
                cmd.DW1.VfCacheInvalidationEnable         = params->bInvalidateVFECache;
                cmd.DW1.InstructionCacheInvalidateEnable  = params->bInvalidateInstructionCache;
                cmd.DW1.TlbInvalidate                     = params->bTlbInvalidate;
                cmd.DW1.TextureCacheInvalidationEnable    = params->bInvalidateTextureCache;
                break;

            // No-flush operation requested
            case MHW_FLUSH_NONE:
            default:
                cmd.DW1.RenderTargetCacheFlushEnable      = false;
                break;
        }

        // When PIPE_CONTROL stall bit is set, one of the following must also be set, otherwise set stall bit to 0
        if (cmd.DW1.CommandStreamerStallEnable &&
            (cmd.DW1.DcFlushEnable == 0 && cmd.DW1.NotifyEnable == 0 && cmd.DW1.PostSyncOperation == 0 &&
             cmd.DW1.DepthStallEnable == 0 && cmd.DW1.StallAtPixelScoreboard == 0 && cmd.DW1.DepthCacheFlushEnable == 0  &&
             cmd.DW1.RenderTargetCacheFlushEnable == 0))
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

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMfxWaitCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_BATCH_BUFFER                   batchBuffer,
        bool                                stallVdboxPipeline)
    {
        MHW_FUNCTION_ENTER;

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
            return MOS_STATUS_NULL_POINTER;
        }

        typename TMiCmds::MFX_WAIT_CMD cmd;
        cmd.DW0.MfxSyncControlFlag = stallVdboxPipeline;

        // set the protection bit based on CP status
        MHW_MI_CHK_STATUS(m_cpInterface->SetProtectionSettingsForMfxWait(m_osInterface, &cmd));

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMediaStateFlush(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        PMHW_MEDIA_STATE_FLUSH_PARAM    params = nullptr)
    {
        MHW_FUNCTION_ENTER;

       if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
            return MOS_STATUS_NULL_POINTER;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SkipMiBatchBufferEndBb(
        PMHW_BATCH_BUFFER               batchBuffer)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(batchBuffer);

        return MOS_STATUS_SUCCESS;
    }

    inline uint32_t GetMiFlushDwCmdSize()
    {
        return TMiCmds::MI_FLUSH_DW_CMD::byteSize;
    }

    inline uint32_t GetMiBatchBufferStartCmdSize()
    {
        return TMiCmds::MI_BATCH_BUFFER_START_CMD::byteSize;
    }

    inline uint32_t GetMiBatchBufferEndCmdSize()
    {
        return TMiCmds::MI_BATCH_BUFFER_END_CMD::byteSize;
    }

    MOS_STATUS AddBatchBufferEndInsertionFlag(
        MOS_COMMAND_BUFFER &constructedCmdBuf)
    {
        MHW_FUNCTION_ENTER;

        typename TMiCmds::MI_BATCH_BUFFER_END_CMD cmd;

        MHW_CHK_NULL_RETURN(constructedCmdBuf.pCmdPtr);
        *((typename TMiCmds::MI_BATCH_BUFFER_END_CMD *)(constructedCmdBuf.pCmdPtr)) = cmd;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetWatchdogTimerThreshold(
        uint32_t frameWidth,
        uint32_t frameHeight,
        bool isEncoder)
    {
        MHW_FUNCTION_ENTER;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetWatchdogTimerRegisterOffset(
        MOS_GPU_CONTEXT gpuContext)
    {
        MHW_FUNCTION_ENTER;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddWatchdogTimerStartCmd(
        PMOS_COMMAND_BUFFER cmdBuffer)
    {
        MHW_FUNCTION_ENTER;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddWatchdogTimerStopCmd(
        PMOS_COMMAND_BUFFER cmdBuffer)
    {
        MHW_FUNCTION_ENTER;

        return MOS_STATUS_SUCCESS;
    }
};

#endif // __MHW_MI_GENERIC_H__
