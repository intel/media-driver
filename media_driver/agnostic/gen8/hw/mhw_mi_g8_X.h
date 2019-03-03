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
//! \file     mhw_mi_g8_X.h
//! \brief    Defines functions for constructing HW commands on Gen8-based platforms
//!

#ifndef __MHW_MI_G8_X_H__
#define __MHW_MI_G8_X_H__

#include "mhw_mi_generic.h"
#include "mhw_mi_hwcmd_g8_X.h"

struct MhwMiInterfaceG8 : public MhwMiInterfaceGeneric<mhw_mi_g8_X>
{
    MhwMiInterfaceG8(
        MhwCpInterface      *cpInterface,
        PMOS_INTERFACE      osInterface) :
        MhwMiInterfaceGeneric(cpInterface, osInterface)
        {
            MHW_FUNCTION_ENTER;
            InitMmioRegisters();
        }

    ~MhwMiInterfaceG8() { MHW_FUNCTION_ENTER; };

    MOS_STATUS AddMiConditionalBatchBufferEndCmd(
        PMOS_COMMAND_BUFFER                         cmdBuffer,
        PMHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS params);

    MOS_STATUS AddMiBatchBufferStartCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer);

    MOS_STATUS AddMediaStateFlush(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        PMHW_MEDIA_STATE_FLUSH_PARAM    params = nullptr);

    void InitMmioRegisters();
};

#endif // __MHW_MI_G8_X_H__
