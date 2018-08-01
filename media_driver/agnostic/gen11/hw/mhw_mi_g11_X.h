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
//! \file     mhw_mi_g11_X.h
//! \brief    Defines functions for constructing HW commands on Gen11-based platforms
//!

#ifndef __MHW_MI_G11_X_H__
#define __MHW_MI_G11_X_H__

#include "mhw_mi_generic.h"
#include "mhw_mi_hwcmd_g11_X.h"

struct MhwMiInterfaceG11 : public MhwMiInterfaceGeneric<mhw_mi_g11_X>
{
    MhwMiInterfaceG11(
        MhwCpInterface      *cpInterface,
        PMOS_INTERFACE      osInterface) :
        MhwMiInterfaceGeneric(cpInterface, osInterface)
        {
            MHW_FUNCTION_ENTER;
            InitMmioRegisters();
        }

    ~MhwMiInterfaceG11() { MHW_FUNCTION_ENTER; };

    MOS_STATUS AddMiConditionalBatchBufferEndCmd(
        PMOS_COMMAND_BUFFER                         cmdBuffer,
        PMHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS params);

    MOS_STATUS AddMiBatchBufferStartCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer);

    MOS_STATUS AddMiStoreRegisterMemCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_STORE_REGISTER_MEM_PARAMS   params);

    MOS_STATUS AddMiLoadRegisterMemCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_STORE_REGISTER_MEM_PARAMS   params);

    MOS_STATUS AddMiLoadRegisterImmCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_LOAD_REGISTER_IMM_PARAMS    params);

    MOS_STATUS AddMiLoadRegisterRegCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_LOAD_REGISTER_REG_PARAMS    params);

    MOS_STATUS AddMiSemaphoreWaitCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_SEMAPHORE_WAIT_PARAMS       params);

    MOS_STATUS SetWatchdogTimerThreshold(
        uint32_t                            frameWidth,
        uint32_t                            frameHeight);

    MOS_STATUS SetWatchdogTimerRegisterOffset(
        MOS_GPU_CONTEXT                     gpuContext);

    MOS_STATUS AddWatchdogTimerStartCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer);

    MOS_STATUS AddWatchdogTimerStopCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer);

    void InitMmioRegisters();

private:
    // MMIO Range 0x1C0000 - 0x200000 is used for Media VDBox or VEBox
    // Each media engine has a range from 0 to 0x3FFF for relative access
    //
    static const uint32_t m_mmioMaxRelativeOffset   = 0x3FFF;               //!< Max reg relative offset in an engine
    static const uint32_t m_mmioMediaLowOffset      = 0x1C0000;             //!< Low bound of VDBox and VEBox MMIO offset
    static const uint32_t m_mmioMediaHighOffset     = 0x200000;             //!< High bound of VDBox and VEBox MMIO offset 

    //!
    //! \brief    Check and convert meida registers to relative offset
    //! \details  Check if an abusolute register offset is VDbox or VEBox register and convert it to relative if so
    //! \param    [in/out] reg
    //!           Register to be checked and converted
    //! \return   bool
    //!           Return true if it is VDBox or VEBox register
    //!
    bool IsRelativeMMIO(uint32_t &reg)
    {
        MOS_GPU_CONTEXT gpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);

        if ((MOS_VCS_ENGINE_USED(gpuContext) || MOS_VECS_ENGINE_USED(gpuContext)) &&
            (reg >= m_mmioMediaLowOffset && reg < m_mmioMediaHighOffset))
        {
            reg &= m_mmioMaxRelativeOffset;
            return true;
        }
        return false;
    }
};

#endif
