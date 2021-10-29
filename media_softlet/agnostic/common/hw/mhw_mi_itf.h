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
//! \file     mhw_mi_itf.h
//! \brief    MHW MI interface common base
//! \details
//!

#ifndef __MHW_MI_ITF_H__
#define __MHW_MI_ITF_H__

#include "mhw_itf.h"
#include "mhw_mi_cmdpar.h"

#define _MI_CMD_DEF(DEF)                  \
    DEF(MI_SEMAPHORE_WAIT);               \
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
    DEF(MI_MATH)

namespace mhw
{
namespace mi
{
class Itf
{
public:
    class ParSetting
    {
    public:
        virtual ~ParSetting() = default;

        _MI_CMD_DEF(_MHW_SETPAR_DEF);
    };

    virtual ~Itf() = default;

    virtual MOS_STATUS SetWatchdogTimerThreshold(uint32_t frameWidth, uint32_t frameHeight, bool isEncoder) = 0;

    virtual MOS_STATUS SetWatchdogTimerRegisterOffset(MOS_GPU_CONTEXT gpuContext) = 0;

    virtual MOS_STATUS AddWatchdogTimerStartCmd(PMOS_COMMAND_BUFFER cmdBuffer) = 0;

    virtual MOS_STATUS AddWatchdogTimerStopCmd(PMOS_COMMAND_BUFFER cmdBuffer) = 0;

    virtual MOS_STATUS AddMiBatchBufferEnd(PMOS_COMMAND_BUFFER cmdBuffer, PMHW_BATCH_BUFFER batchBuffer) = 0;

    virtual MHW_MI_MMIOREGISTERS* GetMmioRegisters() = 0;

    virtual MOS_STATUS SetCpInterface(MhwCpInterface *cpInterface) = 0;

    virtual uint32_t GetMmioInterfaces(MHW_MMIO_REGISTER_OPCODE opCode) = 0;

    _MI_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_ITF);
};
}  // namespace mi
}  // namespace mhw

#endif  // __MHW_MI_ITF_H__
