/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     mhw_mi_itf.h
//! \brief    MHW MI interface common base
//! \details
//!

#ifndef __MHW_MI_ITF_H__
#define __MHW_MI_ITF_H__

#include "mhw_itf.h"
#include "mhw_mi_cmdpar.h"
#include "mhw_cp_interface.h"
#include "media_defs.h"

#define _MI_CMD_DEF(DEF)                  \
    DEF(MI_SEMAPHORE_WAIT);               \
    DEF(MI_SEMAPHORE_SIGNAL);             \
    DEF(MI_CONDITIONAL_BATCH_BUFFER_END); \
    DEF(PIPE_CONTROL);                    \
    DEF(MI_BATCH_BUFFER_START);           \
    DEF(MI_SET_PREDICATE);                \
    DEF(MI_STORE_REGISTER_MEM);           \
    DEF(MI_LOAD_REGISTER_MEM);            \
    DEF(MI_LOAD_REGISTER_IMM);            \
    DEF(MI_LOAD_REGISTER_REG);            \
    DEF(MI_FORCE_WAKEUP);                 \
    DEF(VD_CONTROL_STATE);                \
    DEF(MEDIA_STATE_FLUSH);               \
    DEF(MI_BATCH_BUFFER_END);             \
    DEF(MI_FLUSH_DW);                     \
    DEF(MI_NOOP);                         \
    DEF(MI_ATOMIC);                       \
    DEF(MI_STORE_DATA_IMM);               \
    DEF(MI_MATH);                         \
    DEF(MI_COPY_MEM_MEM);                 \
    DEF(MFX_WAIT);                        \
    DEF(MI_USER_INTERRUPT)

namespace mhw
{
namespace mi
{
class Itf
{
public:

    enum CommandsNumberOfAddresses
    {
        MFX_WAIT_CMD_NUMBER_OF_ADDRESSES                        = 0,
        MI_BATCH_BUFFER_START_CMD_NUMBER_OF_ADDRESSES           = 1,
        MI_STORE_DATA_IMM_CMD_NUMBER_OF_ADDRESSES               = 1,
        MI_FLUSH_DW_CMD_NUMBER_OF_ADDRESSES                     = 1,
        MI_CONDITIONAL_BATCH_BUFFER_END_CMD_NUMBER_OF_ADDRESSES = 1,
        MI_STORE_REGISTER_MEM_CMD_NUMBER_OF_ADDRESSES           = 1,
        MI_LOAD_REGISTER_MEM_CMD_NUMBER_OF_ADDRESSES            = 1,
        MI_COPY_MEM_MEM_CMD_NUMBER_OF_ADDRESSES                 = 4,
        MI_SEMAPHORE_WAIT_CMD_NUMBER_OF_ADDRESSES               = 1,
        MI_ATOMIC_CMD_NUMBER_OF_ADDRESSES                       = 1
    };

    class ParSetting
    {
    public:
        virtual ~ParSetting() = default;

        _MI_CMD_DEF(_MHW_SETPAR_DEF);
    };

    virtual ~Itf() = default;

    virtual MOS_STATUS SetWatchdogTimerThreshold(uint32_t frameWidth, uint32_t frameHeight, bool isEncoder, uint32_t codecMode = CODECHAL_STANDARD_MAX) = 0;

    virtual MOS_STATUS SetWatchdogTimerRegisterOffset(MOS_GPU_CONTEXT gpuContext) = 0;

    virtual MOS_STATUS AddWatchdogTimerStartCmd(PMOS_COMMAND_BUFFER cmdBuffer) = 0;

    virtual MOS_STATUS AddWatchdogTimerStopCmd(PMOS_COMMAND_BUFFER cmdBuffer) = 0;

    virtual MOS_STATUS AddMiBatchBufferEnd(PMOS_COMMAND_BUFFER cmdBuffer, PMHW_BATCH_BUFFER batchBuffer) = 0;

    virtual MOS_STATUS AddMiBatchBufferEndOnly(PMOS_COMMAND_BUFFER cmdBuffer, PMHW_BATCH_BUFFER batchBuffer) = 0;

    virtual MOS_STATUS AddBatchBufferEndInsertionFlag(MOS_COMMAND_BUFFER &constructedCmdBuf) = 0;

    virtual MHW_MI_MMIOREGISTERS* GetMmioRegisters() = 0;

    virtual MOS_STATUS SetCpInterface(MhwCpInterface *cpInterface, std::shared_ptr<mhw::mi::Itf> m_miItf) = 0;

    virtual uint32_t GetMmioInterfaces(MHW_MMIO_REGISTER_OPCODE opCode) = 0;

    virtual MOS_STATUS AddProtectedProlog(MOS_COMMAND_BUFFER *cmdBuffer) = 0;

    virtual MOS_STATUS AddVeboxMMIOPrologCmd(PMOS_COMMAND_BUFFER CmdBuffer) = 0;

    virtual MOS_STATUS AddBLTMMIOPrologCmd(PMOS_COMMAND_BUFFER cmdBuffer) = 0;

    virtual MOS_STATUS AddWaitInSyncBatchBuffer(
        uint64_t fenceTokenValue,
        uint64_t gpuVirtualAddress,
        uint64_t waitValue,
        MHW_BATCH_BUFFER *batchBuffer,
        MHW_SEMAPHORE_WATI_REGISTERS &tokenRegister,
        PMOS_COMMAND_BUFFER cmdbuffer) = 0;

     virtual MOS_STATUS AddSignalInSyncBatchBuffer(
         uint64_t fenceTokenValue,
         uint64_t currentValueGpuVA,
         uint64_t monitoredValueGpuVA,
         uint64_t signalValue,
         MHW_SEMAPHORE_WATI_REGISTERS &tokenRegister,
         PMOS_COMMAND_BUFFER cmdbuffer)
     {
         return MOS_STATUS_SUCCESS;
     }
    _MI_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_ITF);
MEDIA_CLASS_DEFINE_END(mhw__mi__Itf)
};
}  // namespace mi
}  // namespace mhw

#endif  // __MHW_MI_ITF_H__
