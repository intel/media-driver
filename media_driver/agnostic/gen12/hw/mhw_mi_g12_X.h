/*
* Copyright (c) 2015-2019, Intel Corporation
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
//! \file     mhw_mi_g12_X.h
//! \brief    Defines functions for constructing HW commands on Gen12-based platforms
//!

#ifndef __MHW_MI_G12_X_H__
#define __MHW_MI_G12_X_H__

#include "mhw_mi_generic.h"
#include "mhw_mi_hwcmd_g12_X.h"
#include "mhw_mmio_g12.h"

typedef struct _MHW_MI_VD_CONTROL_STATE_PARAMS
{
    bool    vdencEnabled;
    bool    initialization;
    bool    vdencInitialization;
    bool    scalableModePipeLock;
    bool    scalableModePipeUnlock;
    bool    memoryImplicitFlush;
}MHW_MI_VD_CONTROL_STATE_PARAMS, *PMHW_MI_VD_CONTROL_STATE_PARAMS;

typedef struct _MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS : public _MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS
{
    bool                        enableEndCurrentBatchBuffLevel;
    uint32_t                    compareOperation;
    enum PARAMS_TYPE
    {
        ENHANCED_PARAMS = 1
    };
} MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS, *PMHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS;

//!
//! \brief    Gen12 MHW MI command interface
//! \details  The Gen12 MHW MI interface contains functions to add Gen12 MI commands to command buffer or batch buffer
//!
struct MhwMiInterfaceG12 : public MhwMiInterfaceGeneric<mhw_mi_g12_X>
{
    MhwMiInterfaceG12(
        MhwCpInterface      *cpInterface,
        PMOS_INTERFACE      osInterface) :
        MhwMiInterfaceGeneric(cpInterface, osInterface)
        {
            MHW_FUNCTION_ENTER;
            InitMmioRegisters();
        }

    ~MhwMiInterfaceG12() { MHW_FUNCTION_ENTER; };

    MOS_STATUS AddMiConditionalBatchBufferEndCmd(
        PMOS_COMMAND_BUFFER                         cmdBuffer,
        PMHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS params);

    MOS_STATUS AddMiSetPredicateCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        MHW_MI_SET_PREDICATE_ENABLE         enableFlag);

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

    MOS_STATUS AddMiForceWakeupCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_FORCE_WAKEUP_PARAMS         params);

    //!
    //! \brief    Adds Mi Vd control state cmd in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddMiVdControlStateCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_VD_CONTROL_STATE_PARAMS     params);

    void InitMmioRegisters();

    MOS_STATUS SetWatchdogTimerThreshold(
        uint32_t                             frameWidth,
        uint32_t                             frameHeight,
        bool                                 isEncoder = true);

    MOS_STATUS SetWatchdogTimerRegisterOffset(
        MOS_GPU_CONTEXT                      gpuContext);

    MOS_STATUS AddWatchdogTimerStartCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer);

    MOS_STATUS AddWatchdogTimerStopCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer);

private:
    // MMIO Range 0x1C0000 - 0x200000 is used for Media VDBox or VEBox
    // Each media engine has a range from 0 to 0x3FFF for relative access
    //
    static const uint32_t m_mmioMaxRelativeOffset   = M_MMIO_MAX_RELATIVE_OFFSET;               //!< Max reg relative offset in an engine
    static const uint32_t m_mmioMediaLowOffset      = M_MMIO_MEDIA_LOW_OFFSET;             //!< Low bound of VDBox and VEBox MMIO offset
    static const uint32_t m_mmioMediaHighOffset     = M_MMIO_MEDIA_HIGH_OFFSET;             //!< High bound of VDBox and VEBox MMIO offset 

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

    //!
    //! \brief    Check RCS and CCS remap offset
    //! \details  Check if a RCS register offset is set and remap it to RCS/CCS register offset if so.
    //! \param    [in] reg
    //!           Register to be checked and converted
    //! \return   bool
    //!           Return true if it is RCS register
    //!
    bool IsRemappingMMIO(uint32_t &reg)
    {
        MOS_GPU_CONTEXT gpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);

        if (MOS_RCS_ENGINE_USED(gpuContext) &&
            ((M_MMIO_RCS_HW_FE_REMAP_RANGE_BEGIN <= reg && reg <= M_MMIO_RCS_HW_FE_REMAP_RANGE_END) 
           ||(M_MMIO_RCS_AUX_TBL_REMAP_RANGE_BEGIN <= reg && reg <= M_MMIO_RCS_AUX_TBL_REMAP_RANGE_END)
           ||(M_MMIO_RCS_TRTT_REMAP_RANGE_BEGIN <= reg && reg <= M_MMIO_RCS_TRTT_REMAP_RANGE_END)))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

public:
    static const uint32_t m_mmioRcsAuxTableBaseLow      = M_MMIO_RCS_AUX_TABLE_BASE_LOW;
    static const uint32_t m_mmioRcsAuxTableBaseHigh     = M_MMIO_RCS_AUX_TABLE_BASE_HIGH;
    static const uint32_t m_mmioRcsAuxTableInvalidate   = M_MMIO_RCS_AUX_TABLE_INVALIDATE;
    static const uint32_t m_mmioVd0AuxTableBaseLow      = M_MMIO_VD0_AUX_TABLE_BASE_LOW;
    static const uint32_t m_mmioVd0AuxTableBaseHigh     = M_MMIO_VD0_AUX_TABLE_BASE_HIGH;
    static const uint32_t m_mmioVd0AuxTableInvalidate   = M_MMIO_VD0_AUX_TABLE_INVALIDATE;
    static const uint32_t m_mmioVd1AuxTableBaseLow      = M_MMIO_VD1_AUX_TABLE_BASE_LOW;
    static const uint32_t m_mmioVd1AuxTableBaseHigh     = M_MMIO_VD1_AUX_TABLE_BASE_HIGH;
    static const uint32_t m_mmioVd1AuxTableInvalidate   = M_MMIO_VD1_AUX_TABLE_INVALIDATE;
    static const uint32_t m_mmioVd2AuxTableBaseLow      = M_MMIO_VD2_AUX_TABLE_BASE_LOW;
    static const uint32_t m_mmioVd2AuxTableBaseHigh     = M_MMIO_VD2_AUX_TABLE_BASE_HIGH;
    static const uint32_t m_mmioVd2AuxTableInvalidate   = M_MMIO_VD2_AUX_TABLE_INVALIDATE;
    static const uint32_t m_mmioVd3AuxTableBaseLow      = M_MMIO_VD3_AUX_TABLE_BASE_LOW;
    static const uint32_t m_mmioVd3AuxTableBaseHigh     = M_MMIO_VD3_AUX_TABLE_BASE_HIGH;
    static const uint32_t m_mmioVd3AuxTableInvalidate   = M_MMIO_VD3_AUX_TABLE_INVALIDATE;
    static const uint32_t m_mmioVe0AuxTableBaseLow      = M_MMIO_VE0_AUX_TABLE_BASE_LOW;
    static const uint32_t m_mmioVe0AuxTableBaseHigh     = M_MMIO_VE0_AUX_TABLE_BASE_HIGH;
    static const uint32_t m_mmioVe0AuxTableInvalidate   = M_MMIO_VE0_AUX_TABLE_INVALIDATE;
    static const uint32_t m_mmioVe1AuxTableBaseLow      = M_MMIO_VE1_AUX_TABLE_BASE_LOW;
    static const uint32_t m_mmioVe1AuxTableBaseHigh     = M_MMIO_VE1_AUX_TABLE_BASE_HIGH;
    static const uint32_t m_mmioVe1AuxTableInvalidate   = M_MMIO_VE1_AUX_TABLE_INVALIDATE;
    static const uint32_t m_mmioCcs0AuxTableBaseLow     = M_MMIO_CCS0_AUX_TABLE_BASE_LOW;
    static const uint32_t m_mmioCcs0AuxTableBaseHigh    = M_MMIO_CCS0_AUX_TABLE_BASE_HIGH;
    static const uint32_t m_mmioCcs0AuxTableInvalidate  = M_MMIO_CCS0_AUX_TABLE_INVALIDATE;
};
#endif
