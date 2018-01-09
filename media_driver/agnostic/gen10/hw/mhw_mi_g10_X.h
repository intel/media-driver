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
//! \file     mhw_mi_g10_X.h
//! \brief    Defines functions for constructing HW commands on Gen10-based platforms
//!

#ifndef __MHW_MI_G10_X_H__
#define __MHW_MI_G10_X_H__

#include "mhw_mi_generic.h"
#include "mhw_mi_hwcmd_g10_X.h"

struct MhwMiInterfaceG10 : public MhwMiInterfaceGeneric<mhw_mi_g10_X>
{
    MhwMiInterfaceG10(
        MhwCpInterface      *cpInterface,
        PMOS_INTERFACE      osInterface) :
        MhwMiInterfaceGeneric(cpInterface, osInterface)
        {
            MHW_FUNCTION_ENTER;
            InitMmioRegisters();
        }

    ~MhwMiInterfaceG10() { MHW_FUNCTION_ENTER; }

    MOS_STATUS AddMiConditionalBatchBufferEndCmd(
        PMOS_COMMAND_BUFFER                         cmdBuffer,
        PMHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS params);

    MOS_STATUS AddMiBatchBufferStartCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer);

    MOS_STATUS AddMiSemaphoreWaitCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_SEMAPHORE_WAIT_PARAMS       params);

    void InitMmioRegisters();

    MOS_STATUS SetWatchdogTimerThreshold(
        uint32_t frameWidth,
        uint32_t frameHeight);

    MOS_STATUS SetWatchdogTimerRegisterOffset(
        MOS_GPU_CONTEXT gpuContext);

    MOS_STATUS AddWatchdogTimerStartCmd(
        PMOS_COMMAND_BUFFER cmdBuffer);

    MOS_STATUS AddWatchdogTimerStopCmd(
        PMOS_COMMAND_BUFFER cmdBuffer);
};

#endif
