/*
* Copyright (c) 2022-2024, Intel Corporation
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
//! \file     mhw_mi_x3_lpm_base_impl.h
//! \brief    MHW MI interface common base for Xe3_LPM
//! \details
//!

#ifndef __MHW_MI_XE3_LPM_BASE_IMPL_H__
#define __MHW_MI_XE3_LPM_BASE_IMPL_H__

#include "mhw_mi_impl.h"
#include "mhw_mi_hwcmd_xe3_lpm_base.h"
#include "mhw_mi_itf.h"
#include "mhw_impl.h"
#include "mhw_mmio_xe3_lpm_base.h"

namespace mhw
{
namespace mi
{
namespace xe3_lpm_base
{
class Impl : public mi::Impl<Cmd>
{
protected:
    using base_t = mi::Impl<Cmd>;

public:
    Impl(PMOS_INTERFACE osItf) : base_t(osItf)
    {
        MHW_FUNCTION_ENTER;
        InitMhwMiInterface();
        InitMmioRegisters();

        m_watchdogOffsets.ctrlRcs      = WATCHDOG_COUNT_CTRL_OFFSET_RCS;
        m_watchdogOffsets.thresholdRcs = WATCHDOG_COUNT_THRESTHOLD_OFFSET_RCS;
        m_watchdogOffsets.ctrlVcs0      = WATCHDOG_COUNT_CTRL_OFFSET_VCS0;
        m_watchdogOffsets.thresholdVcs0 = WATCHDOG_COUNT_THRESTHOLD_OFFSET_VCS0;
        m_watchdogOffsets.ctrlVcs1      = WATCHDOG_COUNT_CTRL_OFFSET_VCS1;
        m_watchdogOffsets.thresholdVcs1 = WATCHDOG_COUNT_THRESTHOLD_OFFSET_VCS1;
        m_watchdogOffsets.ctrlVecs      = WATCHDOG_COUNT_CTRL_OFFSET_VECS;
        m_watchdogOffsets.thresholdVecs = WATCHDOG_COUNT_THRESTHOLD_OFFSET_VECS;
        m_watchdogOffsets.teecsCtrl      = WATCHDOG_COUNT_CTRL_OFFSET_TEECS;
        m_watchdogOffsets.thresholdTeecs = WATCHDOG_COUNT_THRESTHOLD_OFFSET_TEECS;
    };

    void SetDecoderWatchdogThreshold(uint32_t frameWidth, uint32_t frameHeight, uint32_t codecMode) override
    {
        if ((frameWidth * frameHeight) > (7680 * 4320))
        {
            MediaResetParam.watchdogCountThreshold = MHW_MI_DECODER_16K_WATCHDOG_THRESHOLD_IN_MS_PLUS;
        }
        else if ((frameWidth * frameHeight) > (3840 * 2160))
        {
            MediaResetParam.watchdogCountThreshold = MHW_MI_DECODER_8K_WATCHDOG_THRESHOLD_IN_MS_PLUS;
        }
        else if ((frameWidth * frameHeight) > (1920 * 1080))
        {
            MediaResetParam.watchdogCountThreshold = MHW_MI_DECODER_4K_WATCHDOG_THRESHOLD_IN_MS_PLUS;
        }
        else
        {
            MediaResetParam.watchdogCountThreshold = MHW_MI_DECODER_FHD_WATCHDOG_THRESHOLD_IN_MS_PLUS;
        }
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_SEMAPHORE_WAIT)
    {
        _MHW_SETCMD_CALLBASE(MI_SEMAPHORE_WAIT);

        cmd.DW0.RegisterPollMode   = params.bRegisterPollMode;
        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(PIPE_CONTROL)
    {
        _MHW_SETCMD_CALLBASE(PIPE_CONTROL);
        MEDIA_WA_TABLE *pWaTable = this->m_osItf->pfnGetWaTable(this->m_osItf);
        MHW_MI_CHK_NULL(pWaTable);

        if (this->m_currentCmdBuf == nullptr && this->m_currentBatchBuf == nullptr)
        {
            MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
            return MOS_STATUS_NULL_POINTER;
        }

        cmd.DW1.PipeControlFlushEnable     = true;
        cmd.DW1.CommandStreamerStallEnable = !params.bDisableCSStall;
        cmd.DW4_5.Value[0]                 = params.dwDataDW1;
        cmd.DW4_5.Value[1]                 = params.dwDataDW2;

        if (params.presDest)
        {
            cmd.DW1.PostSyncOperation      = params.dwPostSyncOp;
            cmd.DW1.DestinationAddressType = UseGlobalGtt.m_cs;

            MHW_RESOURCE_PARAMS resourceParams = {};
            resourceParams.presResource    = params.presDest;
            resourceParams.dwOffset        = params.dwResourceOffset;
            resourceParams.pdwCmd          = &(cmd.DW2.Value);
            resourceParams.dwLocationInCmd = 2;
            resourceParams.dwLsbNum        = MHW_COMMON_MI_PIPE_CONTROL_SHIFT;
            resourceParams.bIsWritable     = true;
            resourceParams.HwCommandType   = MOS_PIPE_CONTROL;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }
        else
        {
            if (MEDIA_IS_WA(pWaTable, Wa_14010840176))
            {
                cmd.DW0.DataportFlush                   = true;
                cmd.DW1.ConstantCacheInvalidationEnable = false;
            }
            else
            {
                cmd.DW1.ConstantCacheInvalidationEnable = true;
            }
            cmd.DW1.StateCacheInvalidationEnable     = true;
            cmd.DW1.VfCacheInvalidationEnable        = true;
            cmd.DW1.InstructionCacheInvalidateEnable = true;
            cmd.DW1.RenderTargetCacheFlushEnable     = true;
            cmd.DW1.PostSyncOperation                = cmd.POST_SYNC_OPERATION_NOWRITE;
        }

        // Cache flush mode
        switch (params.dwFlushMode)
        {
        // Flush all Write caches
        case MHW_FLUSH_WRITE_CACHE:
            cmd.DW1.RenderTargetCacheFlushEnable = true;
            cmd.DW1.DcFlushEnable                = true;
            break;

        // Invalidate all Read-only caches
        case MHW_FLUSH_READ_CACHE:
            if (MEDIA_IS_WA(pWaTable, Wa_14010840176))
            {
                cmd.DW0.DataportFlush                   = true;
                cmd.DW1.ConstantCacheInvalidationEnable = false;
            }
            else
            {
                cmd.DW1.ConstantCacheInvalidationEnable = true;
            }
            cmd.DW1.RenderTargetCacheFlushEnable     = false;
            cmd.DW1.StateCacheInvalidationEnable     = true;
            cmd.DW1.VfCacheInvalidationEnable        = true;
            cmd.DW1.InstructionCacheInvalidateEnable = true;
            break;

        // Custom flush parameters
        case MHW_FLUSH_CUSTOM:
            if (MEDIA_IS_WA(pWaTable, Wa_14010840176) && params.bInvalidateConstantCache)
            {
                cmd.DW0.DataportFlush                   = true;
                cmd.DW1.StateCacheInvalidationEnable    = true;
                cmd.DW1.ConstantCacheInvalidationEnable = false;
            }
            else
            {
                cmd.DW1.StateCacheInvalidationEnable    = params.bInvalidateStateCache;
                cmd.DW1.ConstantCacheInvalidationEnable = params.bInvalidateConstantCache;
            }

            cmd.DW0.DataportFlush                    = params.bHdcPipelineFlush;
            cmd.DW0.UnTypedDataPortCacheFlush        = params.bUnTypedDataPortCacheFlush;
            cmd.DW1.RenderTargetCacheFlushEnable     = params.bFlushRenderTargetCache;
            cmd.DW1.DcFlushEnable                    = params.bFlushRenderTargetCache;  // same as above
            cmd.DW1.VfCacheInvalidationEnable        = params.bInvalidateVFECache;
            cmd.DW1.InstructionCacheInvalidateEnable = params.bInvalidateInstructionCache;
            cmd.DW1.TlbInvalidate                    = params.bTlbInvalidate;
            cmd.DW1.TextureCacheInvalidationEnable   = params.bInvalidateTextureCache;
            break;

        // No-flush operation requested
        case MHW_FLUSH_NONE:
        default:
            cmd.DW1.RenderTargetCacheFlushEnable = false;
            break;
        }

        // When PIPE_CONTROL stall bit is set, one of the following must also be set, otherwise set stall bit to 0
        if (cmd.DW1.CommandStreamerStallEnable &&
            (cmd.DW1.DcFlushEnable == 0 && cmd.DW1.NotifyEnable == 0 && cmd.DW1.PostSyncOperation == 0 &&
             cmd.DW1.DepthStallEnable == 0 && cmd.DW1.StallAtPixelScoreboard == 0 && cmd.DW1.DepthCacheFlushEnable == 0 &&
             cmd.DW1.RenderTargetCacheFlushEnable == 0))
        {
            cmd.DW1.CommandStreamerStallEnable = 0;
        }

        cmd.DW1.GenericMediaStateClear       = params.bGenericMediaStateClear;
        cmd.DW1.IndirectStatePointersDisable = params.bIndirectStatePointersDisable;

        if (params.bPPCFlush)
        {
            cmd.DW0.CompressionControlSurfaceCcsFlush = true;
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_BATCH_BUFFER_START)
    {
        _MHW_SETCMD_CALLBASE(MI_BATCH_BUFFER_START);
        MHW_MI_CHK_NULL(this->m_currentCmdBuf);
        MHW_MI_CHK_NULL(this->m_currentBatchBuf);
        MHW_MI_CHK_NULL(this->m_osItf);
        bool vcsEngineUsed =
            MOS_VCS_ENGINE_USED(this->m_osItf->pfnGetGpuContext(this->m_osItf));

        MHW_RESOURCE_PARAMS    resourceParams = {};
        resourceParams.presResource    = &this->m_currentBatchBuf->OsResource;
        resourceParams.dwOffset        = this->m_currentBatchBuf->dwOffset;
        resourceParams.pdwCmd          = cmd.DW1_2.Value;
        resourceParams.dwLocationInCmd = 1;
        resourceParams.dwLsbNum        = MHW_COMMON_MI_GENERAL_SHIFT;
        resourceParams.HwCommandType   = vcsEngineUsed ? MOS_MI_BATCH_BUFFER_START : MOS_MI_BATCH_BUFFER_START_RCS;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));

        // Set BB start
        cmd.DW0.Obj3.SecondLevelBatchBuffer = true;
        cmd.DW0.Obj0.AddressSpaceIndicator  = !IsGlobalGttInUse();

        return MOS_STATUS_SUCCESS;
    }

    __MHW_ADDCMD_DECL(MI_CONDITIONAL_BATCH_BUFFER_END) override
    {
        // Case 1 - Batch buffer condition matches - If this is not present then conditional
        //          batch buffer will  exit to ring with terminating CP.
        // Case 2 - Batch buffer condition DOES NOT match - Although this will disable CP
        //          but after end of conditional batch buffer CP will be re-enabled.
        MHW_MI_CHK_STATUS(m_cpInterface->AddEpilog(this->m_osItf, cmdBuf));

        base_t::MHW_ADDCMD_F(MI_CONDITIONAL_BATCH_BUFFER_END)(cmdBuf, batchBuf);

        MHW_MI_CHK_STATUS(m_cpInterface->AddProlog(this->m_osItf, cmdBuf));

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_CONDITIONAL_BATCH_BUFFER_END)
    {
        _MHW_SETCMD_CALLBASE(MI_CONDITIONAL_BATCH_BUFFER_END);

        MHW_MI_CHK_NULL(params.presSemaphoreBuffer);

        cmd.DW0.UseGlobalGtt     = IsGlobalGttInUse();
        cmd.DW0.CompareSemaphore = 1;  // CompareDataDword is always assumed to be set
        cmd.DW0.CompareMaskMode  = !params.bDisableCompareMask;
        if (params.dwParamsType == MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS::ENHANCED_PARAMS)
        {
            cmd.DW0.EndCurrentBatchBufferLevel = params.enableEndCurrentBatchBuffLevel;
            cmd.DW0.CompareOperation           = params.compareOperation;
        }
        cmd.DW1.CompareDataDword = params.dwValue;

        MHW_RESOURCE_PARAMS resourceParams = {};
        resourceParams.presResource    = params.presSemaphoreBuffer;
        resourceParams.dwOffset        = params.dwOffset;
        resourceParams.pdwCmd          = cmd.DW2_3.Value;
        resourceParams.dwLocationInCmd = 2;
        resourceParams.dwLsbNum        = MHW_COMMON_MI_CONDITIONAL_BATCH_BUFFER_END_SHIFT;
        resourceParams.HwCommandType   = MOS_MI_CONDITIONAL_BATCH_BUFFER_END;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_SET_PREDICATE)
    {
        _MHW_SETCMD_CALLBASE(MI_SET_PREDICATE);
        cmd.DW0.PredicateEnable = params.PredicateEnable;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_STORE_REGISTER_MEM)
    {
        _MHW_SETCMD_CALLBASE(MI_STORE_REGISTER_MEM);
        uint32_t reg = params.dwRegister;
        if (IsRelativeMMIO(reg))
        {
            cmd.DW0.AddCsMmioStartOffset = 1;
            cmd.DW1.RegisterAddress      = reg >> 2;
        }

        if (params.dwOption == CCS_HW_FRONT_END_MMIO_REMAP)
        {
            MOS_GPU_CONTEXT gpuContext = m_osItf->pfnGetGpuContext(m_osItf);
            if (MOS_RCS_ENGINE_USED(gpuContext))
            {
                reg &= M_CCS_HW_FRONT_END_MMIO_MASK;
                reg += M_MMIO_CCS0_HW_FRONT_END_BASE_BEGIN;
            }
        }

        cmd.DW0.MmioRemapEnable = IsRemappingMMIO(reg);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_LOAD_REGISTER_MEM)
    {
        _MHW_SETCMD_CALLBASE(MI_LOAD_REGISTER_MEM);
        uint32_t Reg = params.dwRegister;
        //"Add CS MMIO Start Offset" must not be enabled when "MMIO Remap" is Enabled and Vice-versa.
        if (IsRelativeMMIO(Reg) && params.bMMIORemap == false)
        {
            cmd.DW0.AddCsMmioStartOffset = 1;
            cmd.DW1.RegisterAddress      = Reg >> 2;
        }

        cmd.DW0.MmioRemapEnable = IsRemappingMMIO(Reg) | params.bMMIORemap;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_LOAD_REGISTER_IMM)
    {
        _MHW_SETCMD_CALLBASE(MI_LOAD_REGISTER_IMM);
        uint32_t Reg = params.dwRegister;
        //"Add CS MMIO Start Offset" must not be enabled when "MMIO Remap" is Enabled and Vice-versa.
        if (IsRelativeMMIO(Reg) && params.bMMIORemap == false)
        {
            cmd.DW0.AddCsMmioStartOffset = 1;
            cmd.DW1.RegisterOffset       = Reg >> 2;
        }

        cmd.DW0.MmioRemapEnable = IsRemappingMMIO(Reg) | params.bMMIORemap;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_LOAD_REGISTER_REG)
    {
        _MHW_SETCMD_CALLBASE(MI_LOAD_REGISTER_REG);
        uint32_t srcReg = params.dwSrcRegister;
        //"Add CS MMIO Start Offset" must not be enabled when "MMIO Remap" is Enabled and Vice-versa.
        if (IsRelativeMMIO(srcReg) && params.bMMIORemap == false)
        {
            cmd.DW0.AddCsMmioStartOffsetSource = 1;
            cmd.DW1.SourceRegisterAddress      = srcReg >> 2;
        }
        uint32_t dstReg = params.dwDstRegister;
        //"Add CS MMIO Start Offset" must not be enabled when "MMIO Remap" is Enabled and Vice-versa.
        if (IsRelativeMMIO(dstReg) && params.bMMIORemap == false)
        {
            cmd.DW0.AddCsMmioStartOffsetDestination = 1;
            cmd.DW2.DestinationRegisterAddress      = dstReg >> 2;
        }

        cmd.DW0.MmioRemapEnableSource      = IsRemappingMMIO(srcReg) | params.bMMIORemap;
        cmd.DW0.MmioRemapEnableDestination = IsRemappingMMIO(dstReg) | params.bMMIORemap;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_FORCE_WAKEUP)
    {
        _MHW_SETCMD_CALLBASE(MI_FORCE_WAKEUP);

        cmd.DW1.ForceMediaSlice0Awake = params.bForceMediaSlice0Awake;
        cmd.DW1.ForceRenderAwake      = params.bForceRenderAwake;
        cmd.DW1.ForceMediaSlice1Awake = params.bForceMediaSlice1Awake;
        cmd.DW1.ForceMediaSlice2Awake = params.bForceMediaSlice2Awake;
        cmd.DW1.ForceMediaSlice3Awake = params.bForceMediaSlice3Awake;
        cmd.DW1.HevcPowerWellControl  = params.bHEVCPowerWellControl;
        cmd.DW1.MfxPowerWellControl   = params.bMFXPowerWellControl;
        cmd.DW1.MaskBits              = params.bForceMediaSlice0AwakeMask;
        cmd.DW1.MaskBits += (params.bForceRenderAwakeMask << 1);
        cmd.DW1.MaskBits += (params.bForceMediaSlice1AwakeMask << 2);
        cmd.DW1.MaskBits += (params.bForceMediaSlice2AwakeMask << 3);
        cmd.DW1.MaskBits += (params.bForceMediaSlice3AwakeMask << 4);
        cmd.DW1.MaskBits += (params.bHEVCPowerWellControlMask << 8);
        cmd.DW1.MaskBits += (params.bMFXPowerWellControlMask << 9);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(VD_CONTROL_STATE)
    {
        _MHW_SETCMD_CALLBASE(VD_CONTROL_STATE);

        if (params.vdencEnabled)
        {
            cmd.DW0.MediaInstructionCommand =
                mhw::mi::xe3_lpm_base::Cmd::VD_CONTROL_STATE_CMD::MEDIA_INSTRUCTION_COMMAND_VDCONTROLSTATEFORVDENC;
            cmd.DW0.MediaInstructionOpcode =
                mhw::mi::xe3_lpm_base::Cmd::VD_CONTROL_STATE_CMD::MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEFORVDENC;
        }
        else
        {
            cmd.DW0.MediaInstructionCommand =
                mhw::mi::xe3_lpm_base::Cmd::VD_CONTROL_STATE_CMD::MEDIA_INSTRUCTION_COMMAND_VDCONTROLSTATEFORHCP;
            if (params.avpEnabled)
            {
                cmd.DW0.MediaInstructionOpcode =
                    mhw::mi::xe3_lpm_base::Cmd::VD_CONTROL_STATE_CMD::MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEFORAVP;
            }
            else
            {
                cmd.DW0.MediaInstructionOpcode =
                    mhw::mi::xe3_lpm_base::Cmd::VD_CONTROL_STATE_CMD::MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEFORHCP;
            }

            cmd.DW1.PipelineInitialization = params.initialization;
            cmd.DW2.MemoryImplicitFlush    = params.memoryImplicitFlush;
            cmd.DW2.ScalableModePipeLock   = params.scalableModePipeLock;
            cmd.DW2.ScalableModePipeUnlock = params.scalableModePipeUnlock;
        }
        MEDIA_WA_TABLE *pWaTable = this->m_osItf->pfnGetWaTable(this->m_osItf);
        MHW_MI_CHK_NULL(pWaTable);
        if (MEDIA_IS_WA(pWaTable, Wa_16021867713))
        {
            cmd.DW1.VdboxPipelineArchitectureClockgateDisable = 1;
        }

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

        // set the protection bit based on CP status
#if !EMUL
        MHW_MI_CHK_STATUS(m_cpInterface->SetProtectionSettingsForMiFlushDw(this->m_osItf, &cmd));
#endif
        cmd.DW0.VideoPipelineCacheInvalidate = params.bVideoPipelineCacheInvalidate;
        cmd.DW0.PostSyncOperation            = cmd.POST_SYNC_OPERATION_NOWRITE;
        cmd.DW3_4.Value[0]                   = params.dwDataDW1;

        if (!Mos_ResourceIsNull(params.pOsResource))
        {
            cmd.DW0.PostSyncOperation        = cmd.POST_SYNC_OPERATION_WRITEIMMEDIATEDATAQWORD;
            cmd.DW1_2.DestinationAddressType = UseGlobalGtt.m_vcs;

            MHW_RESOURCE_PARAMS resourceParams = {};
            resourceParams.presResource    = params.pOsResource;
            resourceParams.dwOffset        = params.dwResourceOffset;
            resourceParams.pdwCmd          = cmd.DW1_2.Value;
            resourceParams.dwLocationInCmd = 1;
            resourceParams.dwLsbNum        = MHW_COMMON_MI_FLUSH_DW_SHIFT;
            resourceParams.HwCommandType   = MOS_MI_FLUSH_DW;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (params.postSyncOperation)
        {
            cmd.DW0.PostSyncOperation = params.postSyncOperation;
        }

        if (params.dwDataDW2 || params.bQWordEnable)
        {
            cmd.DW3_4.Value[1] = params.dwDataDW2;
        }
        else
        {
            cmd.DW0.DwordLength--;
        }

        if (params.bEnablePPCFlush && !m_osItf->bNullHwIsEnabled)
        {
            cmd.DW0.FlushCcs = params.bEnablePPCFlush;
        }

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

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_STORE_DATA_IMM)
    {
        _MHW_SETCMD_CALLBASE(MI_STORE_DATA_IMM);

        return MOS_STATUS_SUCCESS;
    }

    __MHW_ADDCMD_DECL(MI_MATH) override
    {
        MHW_FUNCTION_ENTER;
        const auto &params = __MHW_CMDINFO_M(MI_MATH)->first;

        if (params.dwNumAluParams == 0 || params.pAluPayload == nullptr)
        {
            MHW_ASSERTMESSAGE("MI_MATH requires a valid payload");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        base_t::MHW_ADDCMD_F(MI_MATH)(cmdBuf, batchBuf);

        return Mhw_AddCommandCmdOrBB(m_osItf, cmdBuf, nullptr, &params.pAluPayload[0],
            sizeof(MHW_MI_ALU_PARAMS)* params.dwNumAluParams);
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_MATH)
    {
        _MHW_SETCMD_CALLBASE(MI_MATH);

        cmd.DW0.DwordLength = params.dwNumAluParams - 1;

        return MOS_STATUS_SUCCESS;
    }

     virtual MOS_STATUS AddWaitInSyncBatchBuffer(
         uint64_t fenceTokenValue,
         uint64_t gpuVirtualAddress,
         uint64_t waitValue,
         MHW_BATCH_BUFFER *batchBuffer,
         MHW_SEMAPHORE_WATI_REGISTERS &tokenRegister,
         PMOS_COMMAND_BUFFER cmdbuffer) override
     {
         MOS_STATUS status = MOS_STATUS_SUCCESS;

         //set fence token value to token register
         auto &miLoadRegImmParams      = MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
         miLoadRegImmParams            = {};
         miLoadRegImmParams.dwData     = static_cast<uint32_t>(fenceTokenValue);
         miLoadRegImmParams.dwRegister = tokenRegister.m_tokenRegister;
         miLoadRegImmParams.bMMIORemap = tokenRegister.m_bMMIORemap;
         status = MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdbuffer, batchBuffer); //with fenceToken
         MHW_CHK_STATUS_RETURN(status);

         //MI_SEMAPHORE_WAIT
         uint32_t valueLow         = static_cast<uint32_t>(waitValue & 0xffffffff);

         //Only support fence value up to uint32_t max,
         //but the fence definition is in uint64_t.
         //If fence value exceeds uint32_t max, time out is expected.
         auto &params            = MHW_GETPAR_F(MI_SEMAPHORE_WAIT)();
         params                  = {};
         params.gpuVirtualAddress = gpuVirtualAddress;
         params.bPollingWaitMode = true;
         params.dwSemaphoreData  = valueLow;
         params.CompareOperation = (mhw::mi::MHW_COMMON_MI_SEMAPHORE_COMPARE_OPERATION) MHW_MI_SAD_GREATER_THAN_OR_EQUAL_SDD;
         params.dwResourceOffset = 0;
         params.bRegisterPollMode = false;
         params.waitTokenNumber   = static_cast<uint32_t>(fenceTokenValue);
         status                  = MHW_ADDCMD_F(MI_SEMAPHORE_WAIT)(cmdbuffer, batchBuffer);
         MHW_CHK_STATUS_RETURN(status);

         //clear the token register
         miLoadRegImmParams            = {};
         miLoadRegImmParams.dwData     = 0;
         miLoadRegImmParams.dwRegister = tokenRegister.m_tokenRegister;
         miLoadRegImmParams.bMMIORemap = tokenRegister.m_bMMIORemap;
         status = MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdbuffer, batchBuffer); //with fenceToken
         MHW_CHK_STATUS_RETURN(status);

         return status;
     }

     virtual MOS_STATUS AddSignalInSyncBatchBuffer(
         uint64_t fenceTokenValue,
         uint64_t currentValueGpuVA,
         uint64_t monitoredValueGpuVA,
         uint64_t signalValue,
         MHW_SEMAPHORE_WATI_REGISTERS &tokenRegister,
         PMOS_COMMAND_BUFFER cmdbuffer) override
     {
         auto &atomiParams             = MHW_GETPAR_F(MI_ATOMIC)();
         atomiParams                   = {};
         atomiParams.gpuVirtualAddress = currentValueGpuVA;
         atomiParams.dwDataSize        = sizeof(uint64_t);
         atomiParams.Operation         = (mhw::mi::MHW_COMMON_MI_ATOMIC_OPCODE) MHW_MI_ATOMIC_MOVE;
         atomiParams.bInlineData       = true;
         atomiParams.dwOperand1Data[0] = static_cast<uint32_t>(signalValue & 0xffffffff);
         atomiParams.dwOperand1Data[1] = (static_cast<uint32_t>(signalValue >> 32));
         auto status = MHW_ADDCMD_F(MI_ATOMIC)(cmdbuffer);
         MHW_CHK_STATUS_RETURN(status);

         //set fence token value to token register
         auto &miLoadRegImmParams      = MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
         miLoadRegImmParams            = {};
         miLoadRegImmParams.dwData     = static_cast<uint32_t>(fenceTokenValue);
         miLoadRegImmParams.dwRegister = tokenRegister.m_tokenRegister;
         miLoadRegImmParams.bMMIORemap = tokenRegister.m_bMMIORemap;
         status = MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdbuffer, nullptr); //with fenceToken
         MHW_CHK_STATUS_RETURN(status);

         //MI_SEMAPHORE_SIGNAL
         auto &params            = MHW_GETPAR_F(MI_SEMAPHORE_SIGNAL)();
         params                  = {};
         status = MHW_ADDCMD_F(MI_SEMAPHORE_SIGNAL)(cmdbuffer, nullptr);
         MHW_CHK_STATUS_RETURN(status);

         //clear the token register
         miLoadRegImmParams            = {};
         miLoadRegImmParams.dwData     = 0;
         miLoadRegImmParams.dwRegister = tokenRegister.m_tokenRegister;
         miLoadRegImmParams.bMMIORemap = tokenRegister.m_bMMIORemap;
         status = MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdbuffer, nullptr); //with fenceToken
         MHW_CHK_STATUS_RETURN(status);

         //signal to cpu conditionally, MI_USER_INTERRUPT if CurrentValue >= MonitoredValue
         {
            //Load current value to gpr2
            auto &miLoadRegMemParams                = MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
            miLoadRegMemParams                      = {};
            miLoadRegMemParams.gpuVirtualAddress    = currentValueGpuVA;
            miLoadRegMemParams.dwRegister           = tokenRegister.m_gpr2Lo;
            miLoadRegMemParams.bMMIORemap           = tokenRegister.m_bMMIORemap;
            status = MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdbuffer, nullptr);
            MHW_CHK_STATUS_RETURN(status);
            miLoadRegMemParams                      = {};
            miLoadRegMemParams.gpuVirtualAddress    = currentValueGpuVA + sizeof(uint32_t);
            miLoadRegMemParams.dwRegister           = tokenRegister.m_gpr2Hi;
            miLoadRegMemParams.bMMIORemap           = tokenRegister.m_bMMIORemap;
            status = MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdbuffer, nullptr);
            MHW_CHK_STATUS_RETURN(status);
            //Load monitored value to gpr3
            miLoadRegMemParams                      = {};
            miLoadRegMemParams.gpuVirtualAddress    = monitoredValueGpuVA;
            miLoadRegMemParams.dwRegister           = tokenRegister.m_gpr3Lo;
            miLoadRegMemParams.bMMIORemap           = tokenRegister.m_bMMIORemap;
            status = MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdbuffer, nullptr);
            MHW_CHK_STATUS_RETURN(status);
            miLoadRegMemParams                      = {};
            miLoadRegMemParams.gpuVirtualAddress    = monitoredValueGpuVA + sizeof(uint32_t);
            miLoadRegMemParams.dwRegister           = tokenRegister.m_gpr3Hi;
            miLoadRegMemParams.bMMIORemap           = tokenRegister.m_bMMIORemap;
            status = MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdbuffer, nullptr);
            MHW_CHK_STATUS_RETURN(status);

            //math: cv - mv
            {
                // If CV>=MV, signal cpu.
                // If CF is set, i.e., MV > CV --> No interrupt or NOOP on Set
                // If CF isn't set i.e., CV >= MV --> Generate Interrupt
                //Prepare math payload
                MHW_MI_ALU_PARAMS miAluParams[4] = {};
                // LOAD(SRCA, R2)
                miAluParams[0].AluOpcode    = MHW_MI_ALU_LOAD;
                miAluParams[0].Operand1     = MHW_MI_ALU_SRCA;
                miAluParams[0].Operand2     = MHW_MI_ALU_GPREG2;

                // LOAD(SRCB, R3)
                miAluParams[1].AluOpcode    = MHW_MI_ALU_LOAD;
                miAluParams[1].Operand1     = MHW_MI_ALU_SRCB;
                miAluParams[1].Operand2     = MHW_MI_ALU_GPREG3;

                // SUB, ACCU = CV - MV
                miAluParams[2].AluOpcode    = MHW_MI_ALU_SUB;

                // STORE(R0, CF)
                miAluParams[3].AluOpcode    = MHW_MI_ALU_STORE;
                miAluParams[3].Operand1     = MHW_MI_ALU_GPREG0;
                miAluParams[3].Operand2     = MHW_MI_ALU_CF;

                auto &miMathParams = MHW_GETPAR_F(MI_MATH)();
                miMathParams       = {};
                miMathParams.pAluPayload = miAluParams;
                miMathParams.dwNumAluParams = 4;  // four ALU commands needed for this substract opertaion. see following ALU commands.
                status = MHW_ADDCMD_F(MI_MATH)(cmdbuffer, nullptr);
                MHW_CHK_STATUS_RETURN(status);
            }

            //conditional interrupt if CurrentValue >= MonitoredValue
            {
                // MI_LOAD_REGISTER_REG(GPR_R0, MI_PREDICATE_RESULT_2) Load CF into Result2 register
                auto &miLoadRgRegParams = MHW_GETPAR_F(MI_LOAD_REGISTER_REG)();
                miLoadRgRegParams                      = {};
                miLoadRgRegParams.dwSrcRegister       = tokenRegister.m_gpr0Lo;
                miLoadRgRegParams.dwDstRegister       = tokenRegister.m_predicateResult2;
                miLoadRgRegParams.bMMIORemap          = tokenRegister.m_bMMIORemap;
                status = MHW_ADDCMD_F(MI_LOAD_REGISTER_REG)(cmdbuffer, nullptr);
                MHW_CHK_STATUS_RETURN(status);

                //MI_SET_PREDICATE(Predicate_Enable = NOOP_on_Result2_Set)
                auto &miSetPredicateParams = MHW_GETPAR_F(MI_SET_PREDICATE)();
                miSetPredicateParams = {};
                miSetPredicateParams.PredicateEnable = MHW_MI_SET_PREDICATE_ENABLE_ON_CLEAR; 
                status = MHW_ADDCMD_F(MI_SET_PREDICATE)(cmdbuffer, nullptr); //MI_SET_PREDICATE(Predicate_Enable = NOOP_on_Result2_Set)
                MHW_CHK_STATUS_RETURN(status);

                //mi_user_interrupt
                auto &miUserInterruptParams = MHW_GETPAR_F(MI_USER_INTERRUPT)();
                miUserInterruptParams = {};
                status = MHW_ADDCMD_F(MI_USER_INTERRUPT)(cmdbuffer, nullptr); //MI_SET_PREDICATE(Predicate_Enable = NOOP_on_Result2_Set)
                MHW_CHK_STATUS_RETURN(status);

                //MI_SET_PREDICATE(Predicate_Enable = NOOP_Never) // Disable predication
                miSetPredicateParams = {};
                miSetPredicateParams.PredicateEnable = MHW_MI_SET_PREDICATE_DISABLE; 
                status = MHW_ADDCMD_F(MI_SET_PREDICATE)(cmdbuffer, nullptr);
                MHW_CHK_STATUS_RETURN(status);
            }
         }//signal to cpu conditionally
         return MOS_STATUS_SUCCESS;
     }

MEDIA_CLASS_DEFINE_END(mhw__mi__xe3_lpm_base__Impl)
};

}  // namespace xe3_lpm_base
}  // namespace mi
}  // namespace mhw

#endif  // __MHW_MI_XE3_LPM_BASE_IMPL_H__
