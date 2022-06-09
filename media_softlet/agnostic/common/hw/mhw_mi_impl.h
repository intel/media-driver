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
//! \file     mhw_mi_impl.h
//! \brief    MHW MI interface common base
//! \details
//!

#ifndef __MHW_MI_IMPL_H__
#define __MHW_MI_IMPL_H__

#include "mhw_mi_itf.h"
#include "mhw_impl.h"

#ifdef IGFX_MI_INTERFACE_EXT_SUPPORT
#include "mhw_mi_impl_ext.h"
#endif

namespace mhw
{
namespace mi
{
static constexpr uint32_t GENERAL_PURPOSE_REGISTER0_LO_OFFSET_NODE_1_INIT  = 0x1C0600;
static constexpr uint32_t GENERAL_PURPOSE_REGISTER0_HI_OFFSET_NODE_1_INIT  = 0x1C0604;
static constexpr uint32_t GENERAL_PURPOSE_REGISTER4_LO_OFFSET_NODE_1_INIT  = 0x1C0620;
static constexpr uint32_t GENERAL_PURPOSE_REGISTER4_HI_OFFSET_NODE_1_INIT  = 0x1C0624;
static constexpr uint32_t GENERAL_PURPOSE_REGISTER11_LO_OFFSET_NODE_1_INIT = 0x1C0658;
static constexpr uint32_t GENERAL_PURPOSE_REGISTER11_HI_OFFSET_NODE_1_INIT = 0x1C065C;
static constexpr uint32_t GENERAL_PURPOSE_REGISTER12_LO_OFFSET_NODE_1_INIT = 0x1C0660;
static constexpr uint32_t GENERAL_PURPOSE_REGISTER12_HI_OFFSET_NODE_1_INIT = 0x1C0664;

template <typename cmd_t>
class Impl : public Itf, public mhw::Impl
    {
        _MI_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_IMPL);

public:

    //! \brief Indicates the global GTT setting on each engine.
    struct
    {
        bool m_cs   = false;    //!< GGTT in use for the render engine.
        bool m_vcs  = false;   //!< GGTT in use for VDBOX.
        bool m_vecs = false;  //!< GGTT in use for VEBOX.
    } UseGlobalGtt;

    bool IsGlobalGttInUse()
    {
        MOS_GPU_CONTEXT gpuContext = this->m_osItf->pfnGetGpuContext(this->m_osItf);
        bool vcsEngineUsed = MOS_VCS_ENGINE_USED(gpuContext);
        bool renderEngineUsed = MOS_RCS_ENGINE_USED(gpuContext);

        bool globalGttInUse = renderEngineUsed ? UseGlobalGtt.m_cs :
            (vcsEngineUsed ? UseGlobalGtt.m_vcs : UseGlobalGtt.m_vecs);

        return globalGttInUse;
    }

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
            formattedOpCode += MHW_MI_ATOMIC_AND;
            break;
        case MHW_MI_ATOMIC_OR:
            formattedOpCode += MHW_MI_ATOMIC_OR;
            break;
        case MHW_MI_ATOMIC_XOR:
            formattedOpCode += MHW_MI_ATOMIC_XOR;
            break;
        case MHW_MI_ATOMIC_MOVE:
            formattedOpCode += MHW_MI_ATOMIC_MOVE;
            break;
        case MHW_MI_ATOMIC_INC:
            formattedOpCode += MHW_MI_ATOMIC_INC;
            break;
        case MHW_MI_ATOMIC_DEC:
            formattedOpCode += MHW_MI_ATOMIC_DEC;
            break;
        case MHW_MI_ATOMIC_ADD:
            formattedOpCode += MHW_MI_ATOMIC_ADD;
            break;
        case MHW_MI_ATOMIC_SUB:
            formattedOpCode += MHW_MI_ATOMIC_SUB;
            break;
        case MHW_MI_ATOMIC_RSUB:
            formattedOpCode += MHW_MI_ATOMIC_RSUB;
            break;
        case MHW_MI_ATOMIC_IMAX:
            formattedOpCode += MHW_MI_ATOMIC_IMAX;
            break;
        case MHW_MI_ATOMIC_IMIN:
            formattedOpCode += MHW_MI_ATOMIC_IMIN;
            break;
        case MHW_MI_ATOMIC_UMAX:
            formattedOpCode += MHW_MI_ATOMIC_UMAX;
            break;
        case MHW_MI_ATOMIC_UMIN:
            formattedOpCode += MHW_MI_ATOMIC_UMIN;
            break;
        case MHW_MI_ATOMIC_CMP:
            formattedOpCode += MHW_MI_ATOMIC_CMP;
            break;
        default:
            formattedOpCode = MHW_MI_ATOMIC_NONE;
            break;
        }

        return formattedOpCode;
    }

    MOS_STATUS SetWatchdogTimerThreshold(uint32_t frameWidth, uint32_t frameHeight, bool isEncoder) override
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetWatchdogTimerRegisterOffset(MOS_GPU_CONTEXT gpuContext) override
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddWatchdogTimerStartCmd(PMOS_COMMAND_BUFFER cmdBuffer) override
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddWatchdogTimerStopCmd(PMOS_COMMAND_BUFFER cmdBuffer) override
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMiBatchBufferEnd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer) override
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddBatchBufferEndInsertionFlag(MOS_COMMAND_BUFFER &constructedCmdBuf)
    {
        MHW_FUNCTION_ENTER;

        typename cmd_t::MI_BATCH_BUFFER_END_CMD cmd;

        MHW_CHK_NULL_RETURN(constructedCmdBuf.pCmdPtr);
        *((typename cmd_t::MI_BATCH_BUFFER_END_CMD *)(constructedCmdBuf.pCmdPtr)) = cmd;

        return MOS_STATUS_SUCCESS;
    }

    MHW_MI_MMIOREGISTERS* GetMmioRegisters() override
    {
        return &m_mmioRegisters;
    }

    virtual MOS_STATUS SetCpInterface(MhwCpInterface* cpInterface)
    {
        m_cpInterface = cpInterface;
        return MOS_STATUS_SUCCESS;
    }

    virtual uint32_t GetMmioInterfaces(MHW_MMIO_REGISTER_OPCODE opCode) = 0;

    MOS_STATUS AddProtectedProlog(MOS_COMMAND_BUFFER *cmdBuffer)
    {
        MHW_MI_CHK_NULL(cmdBuffer);

        MHW_MI_CHK_STATUS(m_cpInterface->AddProlog(this->m_osItf, cmdBuffer));
        MHW_MI_CHK_STATUS(m_cpInterface->AddCheckForEarlyExit(this->m_osItf, cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetPrologCmd(
        PMOS_COMMAND_BUFFER cmdBuffer)
    {
        MOS_STATUS eStatus          = MOS_STATUS_SUCCESS;
        uint64_t   auxTableBaseAddr = 0;

        MHW_CHK_NULL_RETURN(cmdBuffer);
        MHW_CHK_NULL_RETURN(this->m_osItf);

        auxTableBaseAddr = this->m_osItf->pfnGetAuxTableBaseAddr(this->m_osItf);

        if (auxTableBaseAddr)
        {
            auto &par      = MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
            par            = {};
            par.dwData     = (auxTableBaseAddr & 0xffffffff);
            par.dwRegister = GetMmioInterfaces(mhw::mi::MHW_MMIO_VE0_AUX_TABLE_BASE_LOW);
            MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer);

            par.dwData     = ((auxTableBaseAddr >> 32) & 0xffffffff);
            par.dwRegister = GetMmioInterfaces(mhw::mi::MHW_MMIO_VE0_AUX_TABLE_BASE_HIGH);
            MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer);
        }

        return eStatus;
    }

protected:
    using base_t = Itf;

    MHW_MI_MMIOREGISTERS    m_mmioRegisters = {};
    MhwCpInterface          *m_cpInterface  = nullptr;

public:
    Impl(PMOS_INTERFACE osItf) : mhw::Impl(osItf)
    {
        MHW_FUNCTION_ENTER;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_SEMAPHORE_WAIT)
    {
        _MHW_SETCMD_CALLBASE(MI_SEMAPHORE_WAIT);

        MHW_MI_CHK_NULL(this->m_currentCmdBuf);
        MHW_MI_CHK_NULL(params.presSemaphoreMem);

        MHW_RESOURCE_PARAMS  resourceParams ={};
        resourceParams.presResource    = params.presSemaphoreMem;
        resourceParams.dwOffset        = params.dwResourceOffset;
        resourceParams.pdwCmd          = cmd.DW2_3.Value;
        resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DW2_3.Value);;
        resourceParams.dwLsbNum        = MHW_COMMON_MI_GENERAL_SHIFT;
        resourceParams.HwCommandType   = MOS_MI_SEMAPHORE_WAIT;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));

        cmd.DW0.MemoryType         = IsGlobalGttInUse();
        cmd.DW0.WaitMode           = params.bPollingWaitMode;
        cmd.DW0.CompareOperation   = params.CompareOperation;
        cmd.DW1.SemaphoreDataDword = params.dwSemaphoreData;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(PIPE_CONTROL)
    {
        _MHW_SETCMD_CALLBASE(PIPE_CONTROL);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_BATCH_BUFFER_START)
    {
        _MHW_SETCMD_CALLBASE(MI_BATCH_BUFFER_START);

        return MOS_STATUS_SUCCESS;
    }


    _MHW_SETCMD_OVERRIDE_DECL(MI_CONDITIONAL_BATCH_BUFFER_END)
    {
        _MHW_SETCMD_CALLBASE(MI_CONDITIONAL_BATCH_BUFFER_END);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_SET_PREDICATE)
    {
        _MHW_SETCMD_CALLBASE(MI_SET_PREDICATE);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_STORE_REGISTER_MEM)
    {
        _MHW_SETCMD_CALLBASE(MI_STORE_REGISTER_MEM);

        MHW_MI_CHK_NULL(this->m_currentCmdBuf);
        MHW_MI_CHK_NULL(params.presStoreBuffer);

        MHW_RESOURCE_PARAMS   resourceParams = {};
        resourceParams.presResource    = params.presStoreBuffer;
        resourceParams.dwOffset        = params.dwOffset;
        resourceParams.pdwCmd          = cmd.DW2_3.Value;
        resourceParams.dwLocationInCmd =  _MHW_CMD_DW_LOCATION(DW2_3.Value);;
        resourceParams.dwLsbNum        = MHW_COMMON_MI_GENERAL_SHIFT;
        resourceParams.HwCommandType   = MOS_MI_STORE_REGISTER_MEM;
        resourceParams.bIsWritable     = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));

        cmd.DW0.UseGlobalGtt    = IsGlobalGttInUse();
        cmd.DW1.RegisterAddress = params.dwRegister >> 2;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_LOAD_REGISTER_MEM)
    {
        _MHW_SETCMD_CALLBASE(MI_LOAD_REGISTER_MEM);

        MHW_MI_CHK_NULL(this->m_currentCmdBuf);
        MHW_MI_CHK_NULL(params.presStoreBuffer);

        MHW_RESOURCE_PARAMS  resourceParams = {};
        resourceParams.presResource    = params.presStoreBuffer;
        resourceParams.dwOffset        = params.dwOffset;
        resourceParams.pdwCmd          = cmd.DW2_3.Value;
        resourceParams.dwLocationInCmd =  _MHW_CMD_DW_LOCATION(DW2_3.Value);;
        resourceParams.dwLsbNum        = MHW_COMMON_MI_GENERAL_SHIFT;
        resourceParams.HwCommandType   = MOS_MI_LOAD_REGISTER_MEM;
        resourceParams.bIsWritable     = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));

        cmd.DW0.UseGlobalGtt    = IsGlobalGttInUse();
        cmd.DW1.RegisterAddress = params.dwRegister >> 2;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_LOAD_REGISTER_IMM)
    {
        _MHW_SETCMD_CALLBASE(MI_LOAD_REGISTER_IMM);

        cmd.DW1.RegisterOffset = params.dwRegister >> 2;
        cmd.DW2.DataDword      = params.dwData;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_LOAD_REGISTER_REG)
    {
        _MHW_SETCMD_CALLBASE(MI_LOAD_REGISTER_REG);

        cmd.DW1.SourceRegisterAddress      = params.dwSrcRegister >> 2;
        cmd.DW2.DestinationRegisterAddress = params.dwDstRegister >> 2;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_FORCE_WAKEUP)
    {
        _MHW_SETCMD_CALLBASE(MI_FORCE_WAKEUP);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(VD_CONTROL_STATE)
    {
        _MHW_SETCMD_CALLBASE(VD_CONTROL_STATE);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MEDIA_STATE_FLUSH)
    {
        _MHW_SETCMD_CALLBASE(MEDIA_STATE_FLUSH);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_BATCH_BUFFER_END)
    {
         _MHW_SETCMD_CALLBASE(MI_BATCH_BUFFER_END);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_FLUSH_DW)
    {
        _MHW_SETCMD_CALLBASE(MI_FLUSH_DW);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_NOOP)
    {
        _MHW_SETCMD_CALLBASE(MI_NOOP);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_ATOMIC)
    {
        _MHW_SETCMD_CALLBASE(MI_ATOMIC);
        MHW_RESOURCE_PARAMS     resourceParams = {};
        resourceParams.presResource    = params.pOsResource;
        resourceParams.dwOffset        = params.dwResourceOffset;
        resourceParams.pdwCmd          = &(cmd.DW1.Value);
        resourceParams.dwLocationInCmd =  _MHW_CMD_DW_LOCATION(DW1.Value);;
        resourceParams.dwLsbNum        = MHW_COMMON_MI_GENERAL_SHIFT;
        resourceParams.HwCommandType   = MOS_MI_ATOMIC;
        resourceParams.bIsWritable     = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));

        cmd.DW0.DwordLength       = params.bInlineData ? 1 : 9;
        cmd.DW0.MemoryType        = IsGlobalGttInUse();
        cmd.DW0.ReturnDataControl = params.bReturnData;
        if (params.dwDataSize == sizeof(uint32_t))
        {
            cmd.DW0.DataSize = cmd.DATA_SIZE_DWORD;
        }
        else if (params.dwDataSize == sizeof(uint64_t))
        {
            cmd.DW0.DataSize = cmd.DATA_SIZE_QWORD;
        }
        else if (params.dwDataSize == sizeof(uint64_t) * 2)
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
            cmd.DW0.AtomicOpcode = MHW_MI_ATOMIC_QWORD;
        }
        else if (cmd.DW0.DataSize == cmd.DATA_SIZE_OCTWORD)
        {
            if (params.Operation != MHW_MI_ATOMIC_CMP)
            {
                MHW_ASSERTMESSAGE("An OCTWORD may only be used in the case of a compare operation!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            cmd.DW0.AtomicOpcode = MHW_MI_ATOMIC_OCTWORD;
        }
        cmd.DW0.AtomicOpcode = CreateMiAtomicOpcode(
            cmd.DW0.AtomicOpcode,
            params.Operation);
        if (cmd.DW0.AtomicOpcode == MHW_MI_ATOMIC_NONE)
        {
            MHW_ASSERTMESSAGE("No MI_ATOMIC opcode could be generated");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        cmd.DW0.InlineData  = params.bInlineData;
        cmd.DW0.DwordLength = params.bInlineData ? 9 : 1;

        if (params.bInlineData)
        {
            cmd.DW3.Operand1DataDword0  = params.dwOperand1Data[0];
            cmd.DW4.Operand2DataDword0  = params.dwOperand2Data[0];
            cmd.DW5.Operand1DataDword1  = params.dwOperand1Data[1];
            cmd.DW6.Operand2DataDword1  = params.dwOperand2Data[1];
            cmd.DW7.Operand1DataDword2  = params.dwOperand1Data[2];
            cmd.DW8.Operand2DataDword2  = params.dwOperand2Data[3];
            cmd.DW9.Operand1DataDword3  = params.dwOperand1Data[3];
            cmd.DW10.Operand2DataDword3 = params.dwOperand2Data[3];
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_STORE_DATA_IMM)
    {
        _MHW_SETCMD_CALLBASE(MI_STORE_DATA_IMM);
        MHW_MI_CHK_NULL(this->m_currentCmdBuf);
        MHW_MI_CHK_NULL(params.pOsResource);

        MHW_RESOURCE_PARAMS     resourceParams = {};
        resourceParams.presResource    = params.pOsResource;
        resourceParams.dwOffset        = params.dwResourceOffset;
        resourceParams.pdwCmd          = cmd.DW1_2.Value;
        resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DW1_2.Value);
        resourceParams.dwLsbNum        = MHW_COMMON_MI_STORE_DATA_DW_SHIFT;
        resourceParams.HwCommandType   = MOS_MI_STORE_DATA_IMM;
        resourceParams.bIsWritable     = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));

        cmd.DW0.UseGlobalGtt = IsGlobalGttInUse();
        // Force single DW write, driver never writes a QW
        cmd.DW0.StoreQword = 0;
        cmd.DW0.DwordLength--;

        cmd.DW3.DataDword0 = params.dwValue;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_MATH)
    {
        _MHW_SETCMD_CALLBASE(MI_MATH);
        MHW_MI_CHK_NULL(this->m_currentCmdBuf);

        if (params.dwNumAluParams == 0 || params.pAluPayload == nullptr)
        {
            MHW_ASSERTMESSAGE("MI_MATH requires a valid payload");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        cmd.DW0.DwordLength = params.dwNumAluParams - 1;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_COPY_MEM_MEM)
    {
        _MHW_SETCMD_CALLBASE(MI_COPY_MEM_MEM);

        MHW_MI_CHK_NULL(this->m_currentCmdBuf);
        MHW_MI_CHK_NULL(params.presSrc);
        MHW_MI_CHK_NULL(params.presDst);

        cmd.DW0.UseGlobalGttDestination = IsGlobalGttInUse();
        cmd.DW0.UseGlobalGttSource      = IsGlobalGttInUse();

        MHW_RESOURCE_PARAMS resourceParams = {};
        resourceParams.presResource     = params.presDst;
        resourceParams.dwOffset         = params.dwDstOffset;
        resourceParams.pdwCmd           = cmd.DW1_2.Value;
        resourceParams.dwLocationInCmd  = _MHW_CMD_DW_LOCATION(DW1_2.Value);
        resourceParams.dwLsbNum         = MHW_COMMON_MI_GENERAL_SHIFT;
        resourceParams.HwCommandType    = MOS_MI_COPY_MEM_MEM;
        resourceParams.bIsWritable      = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));

        resourceParams = {};
        resourceParams.presResource     = params.presSrc;
        resourceParams.dwOffset         = params.dwSrcOffset;
        resourceParams.pdwCmd           = cmd.DW3_4.Value;
        resourceParams.dwLocationInCmd  = _MHW_CMD_DW_LOCATION(DW3_4.Value);
        resourceParams.dwLsbNum         = MHW_COMMON_MI_GENERAL_SHIFT;
        resourceParams.HwCommandType    = MOS_MI_COPY_MEM_MEM;
        resourceParams.bIsWritable      = false;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MFX_WAIT)
    {
        _MHW_SETCMD_CALLBASE(MFX_WAIT);

        cmd.DW0.MfxSyncControlFlag = params.iStallVdboxPipeline;

        // set the protection bit based on CP status
        MHW_MI_CHK_STATUS(m_cpInterface->SetProtectionSettingsForMfxWait(this->m_osItf, &cmd));

        return MOS_STATUS_SUCCESS;
    }
MEDIA_CLASS_DEFINE_END(mhw__mi__Impl)
};
}  // namespace render
}  // namespace mhw

#endif  // __MHW_RENDER_IMPL_H__
