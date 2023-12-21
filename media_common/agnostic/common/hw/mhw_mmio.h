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
//! \file     mhw_mmio.h
//! \brief    Define the common MMIO registers for all Gens  
//! \details  
//!

#ifndef __MHW_MMIO_H__
#define __MHW_MMIO_H__

// VEBOX 
#define MHW__PWR_CLK_STATE_REG                           0x20C8  //MMIO register for power clock state

// RCS register
#define CS_GPR_R0                                        0x2600

//| MMIO register offsets used for the EU debug support
#define INSTPM                                           0x20c0
#define INSTPM_GLOBAL_DEBUG_ENABLE                      (1 << 4)
#define CS_DEBUG_MODE1                                   0x20ec
#define CS_DEBUG_MODE2                                   0x20d8
#define CS_DEBUG_MODE1_GLOBAL_DEBUG                     (1 << 6)
#define CS_DEBUG_MODE2_GLOBAL_DEBUG                     (1 << 5)
#define TD_CTL                                           0xe400
#define TD_CTL_FORCE_THREAD_BKPT_ENABLE                 (1 << 4)
#define TD_CTL_FORCE_EXT_EXCEPTION_ENABLE               (1 << 7)
// RENDER
#define MHW_RENDER_ENGINE_PREEMPTION_CONTROL_OFFSET      0x2580
#define MHW_RENDER_ENGINE_MID_THREAD_PREEMPT_VALUE       0x00060000
#define MHW_RENDER_ENGINE_THREAD_GROUP_PREEMPT_VALUE     0x00060002
#define MHW_RENDER_ENGINE_MID_BATCH_PREEMPT_VALUE        0x00060004

#define M_L3_CACHE_CNTL_REG_OFFSET                       0x7034
#define M_L3_CACHE_CNTL_REG_VALUE_DEFAULT                0

typedef struct _MHW_MI_MMIOREGISTERS
{
    uint32_t            generalPurposeRegister0LoOffset;
    uint32_t            generalPurposeRegister0HiOffset;
    uint32_t            generalPurposeRegister4LoOffset;
    uint32_t            generalPurposeRegister4HiOffset;
    uint32_t            generalPurposeRegister11LoOffset;       //!< __OCA_BUFFER_ADDR_LOW_MMIO
    uint32_t            generalPurposeRegister11HiOffset;       //!< __OCA_BUFFER_ADDR_HIGH_MMIO
    uint32_t            generalPurposeRegister12LoOffset;       //!< __OCA_BUFFER_IND_STATE_SECTION_OFFSET_MMIO
    uint32_t            generalPurposeRegister12HiOffset;       //!< __OCA_BUFFER_BB_SECTION_OFFSET_MMIO
} MHW_MI_MMIOREGISTERS, *PMHW_MI_MMIOREGISTERS;


//!
//! \struct   MmioRegistersHcp
//! \brief    MMIO registers HCP
//!
struct MmioRegistersHcp
{
    uint32_t                   watchdogCountCtrlOffset;
    uint32_t                   watchdogCountThresholdOffset;
    uint32_t                   hcpDebugFEStreamOutSizeRegOffset;
    uint32_t                   hcpEncImageStatusMaskRegOffset;
    uint32_t                   hcpEncImageStatusCtrlRegOffset;
    uint32_t                   hcpEncBitstreamBytecountFrameRegOffset;
    uint32_t                   hcpEncBitstreamSeBitcountFrameRegOffset;
    uint32_t                   hcpEncBitstreamBytecountFrameNoHeaderRegOffset;
    uint32_t                   hcpEncQpStatusCountRegOffset;
    uint32_t                   hcpEncSliceCountRegOffset;
    uint32_t                   hcpEncVdencModeTimerRegOffset;
    uint32_t                   hcpVp9EncBitstreamBytecountFrameRegOffset;
    uint32_t                   hcpVp9EncBitstreamBytecountFrameNoHeaderRegOffset;
    uint32_t                   hcpVp9EncImageStatusMaskRegOffset;
    uint32_t                   hcpVp9EncImageStatusCtrlRegOffset;
    uint32_t                   csEngineIdOffset;
    uint32_t                   hcpDecStatusRegOffset;
    uint32_t                   hcpCabacStatusRegOffset;
    uint32_t                   hcpFrameCrcRegOffset;
};


struct MmioRegistersHuc
{
    uint32_t                    hucStatusRegOffset = 0;
    uint32_t                    hucUKernelHdrInfoRegOffset = 0;
    uint32_t                    hucStatus2RegOffset = 0;
    uint32_t                    hucLoadInfoOffset = 0;
};

//!
//! \struct    MmioRegistersMfx
//! \brief     MM IO register MFX
//!
struct MmioRegistersMfx
{
    uint32_t            generalPurposeRegister0LoOffset = 0;
    uint32_t            generalPurposeRegister0HiOffset = 0;
    uint32_t            generalPurposeRegister4LoOffset = 0;
    uint32_t            generalPurposeRegister4HiOffset = 0;
    uint32_t            generalPurposeRegister11LoOffset = 0;   //!< __OCA_BUFFER_ADDR_LOW_MMIO
    uint32_t            generalPurposeRegister11HiOffset = 0;   //!< __OCA_BUFFER_ADDR_HIGH_MMIO
    uint32_t            generalPurposeRegister12LoOffset = 0;   //!< __OCA_BUFFER_IND_STATE_SECTION_OFFSET_MMIO
    uint32_t            generalPurposeRegister12HiOffset = 0;   //!< __OCA_BUFFER_BB_SECTION_OFFSET_MMIO
    uint32_t            mfcImageStatusMaskRegOffset = 0;
    uint32_t            mfcImageStatusCtrlRegOffset = 0;
    uint32_t            mfcAvcNumSlicesRegOffset = 0;
    uint32_t            mfcQPStatusCountOffset = 0;
    uint32_t            mfxErrorFlagsRegOffset = 0;
    uint32_t            mfxFrameCrcRegOffset = 0;
    uint32_t            mfxMBCountRegOffset = 0;
    uint32_t            mfcBitstreamBytecountFrameRegOffset = 0;
    uint32_t            mfcBitstreamSeBitcountFrameRegOffset = 0;
    uint32_t            mfcBitstreamBytecountSliceRegOffset = 0;
    uint32_t            mfcVP8BitstreamBytecountFrameRegOffset = 0;
    uint32_t            mfcVP8ImageStatusMaskRegOffset = 0;
    uint32_t            mfcVP8ImageStatusCtrlRegOffset = 0;
    uint32_t            mfxVP8BrcDQIndexRegOffset = 0;
    uint32_t            mfxVP8BrcDLoopFilterRegOffset = 0;
    uint32_t            mfxVP8BrcCumulativeDQIndex01RegOffset = 0;
    uint32_t            mfxVP8BrcCumulativeDQIndex23RegOffset = 0;
    uint32_t            mfxVP8BrcCumulativeDLoopFilter01RegOffset = 0;
    uint32_t            mfxVP8BrcCumulativeDLoopFilter23RegOffset = 0;
    uint32_t            mfxVP8BrcConvergenceStatusRegOffset = 0;
    uint32_t            mfxLra0RegOffset = 0;
    uint32_t            mfxLra1RegOffset = 0;
    uint32_t            mfxLra2RegOffset = 0;
};

typedef MmioRegistersMfx MmioRegistersVdbox;

//!
//! \brief  MHW VEBOX MMIO Structure
//!
typedef struct _MHW_VEBOX_MMIO
{
    uint32_t dwWatchdogCountCtrlOffset;
    uint32_t dwWatchdogCountThresholdOffset;
} MHW_VEBOX_MMIO, *PMHW_VEBOX_MMIO;


#endif // __MHW_MMIO_H__
