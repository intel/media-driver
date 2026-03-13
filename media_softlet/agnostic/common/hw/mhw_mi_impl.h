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
#include "mos_os_cp_interface_specific.h"

#ifdef IGFX_MI_INTERFACE_EXT_SUPPORT
#include "mhw_mi_impl_ext.h"
#endif

// Common MMIO register defines used by base Impl<cmd_t> methods.
// Platform-specific headers (mhw_mmio_xe*_lpm*.h) define these same values;
// the #ifndef guards allow the platform definition to take precedence when
// the platform header is included first.
#ifndef M_MMIO_MAX_RELATIVE_OFFSET
#define M_MMIO_MAX_RELATIVE_OFFSET                   0x3FFF
#define M_MMIO_MEDIA_LOW_OFFSET                      0x1C0000
#define M_MMIO_MEDIA_HIGH_OFFSET                     0x200000
#endif

#ifndef M_MMIO_RCS_HW_FE_REMAP_RANGE_BEGIN
#define M_MMIO_RCS_HW_FE_REMAP_RANGE_BEGIN           0x2000
#define M_MMIO_RCS_HW_FE_REMAP_RANGE_END             0x27FF
#define M_MMIO_RCS_AUX_TBL_REMAP_RANGE_BEGIN         0x4200
#define M_MMIO_RCS_AUX_TBL_REMAP_RANGE_END           0x420F
#define M_MMIO_RCS_TRTT_REMAP_RANGE_BEGIN            0x4400
#define M_MMIO_RCS_TRTT_REMAP_RANGE_END              0x441F
#endif

#ifndef M_MMIO_CCS0_HW_FRONT_END_BASE_BEGIN
#define M_MMIO_CCS0_HW_FRONT_END_BASE_BEGIN          0x1A000
#define M_MMIO_CCS0_HW_FRONT_END_BASE_END            0x1A7FF
#define M_MMIO_CCS1_HW_FRONT_END_BASE_BEGIN          0x1C000
#define M_MMIO_CCS1_HW_FRONT_END_BASE_END            0x1C7FF
#define M_MMIO_CCS2_HW_FRONT_END_BASE_BEGIN          0x1E000
#define M_MMIO_CCS2_HW_FRONT_END_BASE_END            0x1E7FF
#define M_MMIO_CCS3_HW_FRONT_END_BASE_BEGIN          0x26000
#define M_MMIO_CCS3_HW_FRONT_END_BASE_END            0x267FF
#endif

#ifndef M_MMIO_RCS_AUX_TABLE_BASE_LOW
#define M_MMIO_RCS_AUX_TABLE_BASE_LOW                0x4200
#define M_MMIO_RCS_AUX_TABLE_BASE_HIGH               0x4204
#define M_MMIO_RCS_AUX_TABLE_INVALIDATE              0x4208
#endif

#ifndef M_MMIO_VD0_AUX_TABLE_BASE_LOW
#define M_MMIO_VD0_AUX_TABLE_BASE_LOW                0x4210
#define M_MMIO_VD0_AUX_TABLE_BASE_HIGH               0x4214
#define M_MMIO_VD0_AUX_TABLE_INVALIDATE              0x4218
#define M_MMIO_VD1_AUX_TABLE_BASE_LOW                0x4220
#define M_MMIO_VD1_AUX_TABLE_BASE_HIGH               0x4224
#define M_MMIO_VD1_AUX_TABLE_INVALIDATE              0x4228
#define M_MMIO_VD2_AUX_TABLE_BASE_LOW                0x4290
#define M_MMIO_VD2_AUX_TABLE_BASE_HIGH               0x4294
#define M_MMIO_VD2_AUX_TABLE_INVALIDATE              0x4298
#define M_MMIO_VD3_AUX_TABLE_BASE_LOW                0x42A0
#define M_MMIO_VD3_AUX_TABLE_BASE_HIGH               0x42A4
#define M_MMIO_VD3_AUX_TABLE_INVALIDATE              0x42A8
#endif

#ifndef M_MMIO_VE0_AUX_TABLE_BASE_LOW
#define M_MMIO_VE0_AUX_TABLE_BASE_LOW                0x4230
#define M_MMIO_VE0_AUX_TABLE_BASE_HIGH               0x4234
#define M_MMIO_VE0_AUX_TABLE_INVALIDATE              0x4238
#define M_MMIO_VE1_AUX_TABLE_BASE_LOW                0x42B0
#define M_MMIO_VE1_AUX_TABLE_BASE_HIGH               0x42B4
#define M_MMIO_VE1_AUX_TABLE_INVALIDATE              0x42B8
#endif

#ifndef M_MMIO_CCS0_AUX_TABLE_BASE_LOW
#define M_MMIO_CCS0_AUX_TABLE_BASE_LOW               0x42C0
#define M_MMIO_CCS0_AUX_TABLE_BASE_HIGH              0x42C4
#define M_MMIO_CCS0_AUX_TABLE_INVALIDATE             0x42C8
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

    MOS_STATUS SetWatchdogTimerThreshold(uint32_t frameWidth, uint32_t frameHeight, bool isEncoder, uint32_t codecMode, bool isTee) override
    {
        MHW_FUNCTION_ENTER;
        MHW_MI_CHK_NULL(this->m_osItf);
        if (this->m_osItf->bMediaReset == false ||
            this->m_osItf->umdMediaResetEnable == false)
        {
            return MOS_STATUS_SUCCESS;
        }

        if (isTee)
        {
            MHW_MI_CHK_STATUS(SetTeeWatchdogThreshold());
        }
        else if (isEncoder)
        {
            if ((frameWidth * frameHeight) >= (7680 * 4320))
            {
                MediaResetParam.watchdogCountThreshold = MHW_MI_ENCODER_16K_WATCHDOG_THRESHOLD_IN_MS;
            }
            else if ((frameWidth * frameHeight) >= (3840 * 2160))
            {
                MediaResetParam.watchdogCountThreshold = MHW_MI_ENCODER_8K_WATCHDOG_THRESHOLD_IN_MS;
            }
            else if ((frameWidth * frameHeight) >= (1920 * 1080))
            {
                MediaResetParam.watchdogCountThreshold = MHW_MI_ENCODER_4K_WATCHDOG_THRESHOLD_IN_MS;
            }
            else
            {
                MediaResetParam.watchdogCountThreshold = MHW_MI_ENCODER_FHD_WATCHDOG_THRESHOLD_IN_MS;
            }
        }
        else
        {
            SetDecoderWatchdogThreshold(frameWidth, frameHeight, codecMode);
        }

        GetWatchdogThreshold(this->m_osItf);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetWatchdogTimerRegisterOffset(MOS_GPU_CONTEXT gpuContext) override
    {
        MHW_FUNCTION_ENTER;

        switch (gpuContext)
        {
            // RCS
        case MOS_GPU_CONTEXT_RENDER:
        case MOS_GPU_CONTEXT_RENDER2:
        case MOS_GPU_CONTEXT_RENDER3:
        case MOS_GPU_CONTEXT_RENDER4:
        case MOS_GPU_CONTEXT_COMPUTE:
        case MOS_GPU_CONTEXT_CM_COMPUTE:
        case MOS_GPU_CONTEXT_RENDER_RA:
        case MOS_GPU_CONTEXT_COMPUTE_RA:
            MediaResetParam.watchdogCountCtrlOffset      = m_watchdogOffsets.ctrlRcs;
            MediaResetParam.watchdogCountThresholdOffset = m_watchdogOffsets.thresholdRcs;
            break;
            // VCS0
        case MOS_GPU_CONTEXT_VIDEO:
        case MOS_GPU_CONTEXT_VIDEO2:
        case MOS_GPU_CONTEXT_VIDEO3:
        case MOS_GPU_CONTEXT_VIDEO4:
        case MOS_GPU_CONTEXT_VIDEO5:
        case MOS_GPU_CONTEXT_VIDEO6:
        case MOS_GPU_CONTEXT_VIDEO7:
            MediaResetParam.watchdogCountCtrlOffset      = m_watchdogOffsets.ctrlVcs0;
            MediaResetParam.watchdogCountThresholdOffset = m_watchdogOffsets.thresholdVcs0;
            break;
            // VCS1
        case MOS_GPU_CONTEXT_VDBOX2_VIDEO:
        case MOS_GPU_CONTEXT_VDBOX2_VIDEO2:
        case MOS_GPU_CONTEXT_VDBOX2_VIDEO3:
            MediaResetParam.watchdogCountCtrlOffset      = m_watchdogOffsets.ctrlVcs1;
            MediaResetParam.watchdogCountThresholdOffset = m_watchdogOffsets.thresholdVcs1;
            break;
            // VECS
        case MOS_GPU_CONTEXT_VEBOX:
            MediaResetParam.watchdogCountCtrlOffset      = m_watchdogOffsets.ctrlVecs;
            MediaResetParam.watchdogCountThresholdOffset = m_watchdogOffsets.thresholdVecs;
            break;
            // TEE
        case MOS_GPU_CONTEXT_TEE:
            MediaResetParam.watchdogCountCtrlOffset      = m_watchdogOffsets.teecsCtrl;
            MediaResetParam.watchdogCountThresholdOffset = m_watchdogOffsets.thresholdTeecs;
            break;
            // Default
        default:
            break;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddWatchdogTimerStartCmd(PMOS_COMMAND_BUFFER cmdBuffer) override
    {
        MOS_GPU_CONTEXT gpuContext;

        MHW_FUNCTION_ENTER;
        MHW_MI_CHK_NULL(this->m_osItf);
        if (this->m_osItf->bMediaReset == false ||
            this->m_osItf->umdMediaResetEnable == false)
        {
            return MOS_STATUS_SUCCESS;
        }

        MHW_MI_CHK_NULL(cmdBuffer);

        // Set Watchdog Timer Register Offset
        gpuContext = this->m_osItf->pfnGetGpuContext(this->m_osItf);
        MHW_MI_CHK_STATUS(SetWatchdogTimerRegisterOffset(gpuContext));

        // Send Stop before Start is to help recover from incorrect wdt state if previous submission
        // cause hang and not have a chance to execute the stop cmd in the end of batch buffer.
        MHW_MI_CHK_STATUS(AddWatchdogTimerStopCmd(cmdBuffer));

        //Configure Watchdog timer Threshold
        auto& par = MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
        par = {};
        par.dwData = MHW_MI_WATCHDOG_COUNTS_PER_MILLISECOND * MediaResetParam.watchdogCountThreshold *
            (this->m_osItf->bSimIsActive ? 2 : 1);
        par.dwRegister = MediaResetParam.watchdogCountThresholdOffset;
        MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer);

        MHW_VERBOSEMESSAGE("MediaReset Threshold is %d for register 0x%x", MediaResetParam.watchdogCountThreshold * (this->m_osItf->bSimIsActive ? 2 : 1), par.dwRegister);

        //Start Watchdog Timer
        auto& par1 = MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
        par1 = {};
        par1.dwData     = MHW_MI_WATCHDOG_ENABLE_COUNTER;
        par1.dwRegister = MediaResetParam.watchdogCountCtrlOffset;
        MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddWatchdogTimerStopCmd(PMOS_COMMAND_BUFFER cmdBuffer) override
    {
        MOS_GPU_CONTEXT gpuContext;

        MHW_FUNCTION_ENTER;
        MHW_MI_CHK_NULL(this->m_osItf);
        if (this->m_osItf->bMediaReset == false ||
            this->m_osItf->umdMediaResetEnable == false)
        {
            return MOS_STATUS_SUCCESS;
        }

        MHW_MI_CHK_NULL(cmdBuffer);

        // Set Watchdog Timer Register Offset
        gpuContext = this->m_osItf->pfnGetGpuContext(this->m_osItf);
        MHW_MI_CHK_STATUS(SetWatchdogTimerRegisterOffset(gpuContext));

        //Stop Watchdog Timer
        auto& par = MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
        par = {};
        par.dwData     = MHW_MI_WATCHDOG_DISABLE_COUNTER;
        par.dwRegister = MediaResetParam.watchdogCountCtrlOffset;
        MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMiBatchBufferEnd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer) override
    {
        MHW_FUNCTION_ENTER;

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
            return MOS_STATUS_NULL_POINTER;
        }

        auto waTable = this->m_osItf->pfnGetWaTable(this->m_osItf);
        MHW_MI_CHK_NULL(waTable);

        // This WA does not apply for video or other engines, render requirement only
        bool isRender =
            MOS_RCS_ENGINE_USED(this->m_osItf->pfnGetGpuContext(this->m_osItf));

        if (isRender &&
            (MEDIA_IS_WA(waTable, WaMSFWithNoWatermarkTSGHang) ||
            MEDIA_IS_WA(waTable, WaAddMediaStateFlushCmd)))
        {
            auto& params = MHW_GETPAR_F(MEDIA_STATE_FLUSH)();
            params = {};
            MHW_ADDCMD_F(MEDIA_STATE_FLUSH)(cmdBuffer, batchBuffer);
        }

        // Mhw_CommonMi_AddMiBatchBufferEnd() is designed to handle both 1st level
        // and 2nd level BB.  It inserts MI_BATCH_BUFFER_END in both cases.
        // However, since the 2nd level BB always returens to the 1st level BB and
        // no chained BB scenario in Media, Epilog is only needed in the 1st level BB.
        // Therefre, here only the 1st level BB case needs an Epilog inserted.
        if (cmdBuffer && cmdBuffer->is1stLvlBB)
        {
            MHW_MI_CHK_STATUS(m_cpInterface->AddEpilog(this->m_osItf, cmdBuffer));
        }

        auto& params = MHW_GETPAR_F(MI_BATCH_BUFFER_END)();
        params = {};
        MHW_ADDCMD_F(MI_BATCH_BUFFER_END)(cmdBuffer, batchBuffer);

        if (!cmdBuffer) // Don't need BB not nullptr chk b/c if both are nullptr it won't get this far
        {
#if (_DEBUG || _RELEASE_INTERNAL)
            batchBuffer->iLastCurrent = batchBuffer->iCurrent;
#endif
        }

        // Send End Marker command
        if (this->m_osItf->pfnIsSetMarkerEnabled(this->m_osItf) && cmdBuffer && cmdBuffer->is1stLvlBB)
        {
            PMOS_RESOURCE   resMarker = nullptr;
            resMarker = this->m_osItf->pfnGetMarkerResource(this->m_osItf);
            MHW_MI_CHK_NULL(resMarker);

            if (isRender)
            {
                // Send pipe_control to get the timestamp
                auto& params = MHW_GETPAR_F(PIPE_CONTROL)();
                params = {};
                params.presDest = resMarker;
                params.dwResourceOffset = sizeof(uint64_t);
                params.dwPostSyncOp = MHW_FLUSH_WRITE_TIMESTAMP_REG;
                params.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
                MHW_ADDCMD_F(PIPE_CONTROL)(cmdBuffer, batchBuffer);
            }
            else
            {
                // Send flush_dw to get the timestamp
                auto& params = MHW_GETPAR_F(MI_FLUSH_DW)();
                params = {};
                params.pOsResource = resMarker;
                params.dwResourceOffset = sizeof(uint64_t);
                params.postSyncOperation = MHW_FLUSH_WRITE_TIMESTAMP_REG;
                params.bQWordEnable = 1;
                MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer, batchBuffer);
            }

            if (!this->m_osItf->apoMosEnabled)
            {
                MOS_SafeFreeMemory(resMarker);
            }
        }
        MHW_MI_CHK_STATUS(this->m_osItf->osCpInterface->PermeateBBPatchForHM());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMiBatchBufferEndOnly(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER   batchBuffer) override
    {
        MHW_FUNCTION_ENTER;

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
            return MOS_STATUS_NULL_POINTER;
        }

        // This WA does not apply for video or other engines, render requirement only
        bool isRender =
            MOS_RCS_ENGINE_USED(this->m_osItf->pfnGetGpuContext(this->m_osItf));

        // Mhw_CommonMi_AddMiBatchBufferEnd() is designed to handle both 1st level
        // and 2nd level BB.  It inserts MI_BATCH_BUFFER_END in both cases.
        // However, since the 2nd level BB always returens to the 1st level BB and
        // no chained BB scenario in Media, Epilog is only needed in the 1st level BB.
        // Therefre, here only the 1st level BB case needs an Epilog inserted.
        if (cmdBuffer && cmdBuffer->is1stLvlBB)
        {
            MHW_MI_CHK_STATUS(m_cpInterface->AddEpilog(this->m_osItf, cmdBuffer));
        }

        auto &params = MHW_GETPAR_F(MI_BATCH_BUFFER_END)();
        params       = {};
        MHW_ADDCMD_F(MI_BATCH_BUFFER_END)
        (cmdBuffer, batchBuffer);

        if (!cmdBuffer)  // Don't need BB not nullptr chk b/c if both are nullptr it won't get this far
        {
#if (_DEBUG || _RELEASE_INTERNAL)
            batchBuffer->iLastCurrent = batchBuffer->iCurrent;
#endif
        }

        // Send End Marker command
        if (this->m_osItf->pfnIsSetMarkerEnabled(this->m_osItf) && cmdBuffer && cmdBuffer->is1stLvlBB)
        {
            PMOS_RESOURCE resMarker = nullptr;
            resMarker               = this->m_osItf->pfnGetMarkerResource(this->m_osItf);
            MHW_MI_CHK_NULL(resMarker);

            if (isRender)
            {
                // Send pipe_control to get the timestamp
                auto &params            = MHW_GETPAR_F(PIPE_CONTROL)();
                params                  = {};
                params.presDest         = resMarker;
                params.dwResourceOffset = sizeof(uint64_t);
                params.dwPostSyncOp     = MHW_FLUSH_WRITE_TIMESTAMP_REG;
                params.dwFlushMode      = MHW_FLUSH_WRITE_CACHE;
                MHW_ADDCMD_F(PIPE_CONTROL)
                (cmdBuffer, batchBuffer);
            }
            else
            {
                // Send flush_dw to get the timestamp
                auto &params             = MHW_GETPAR_F(MI_FLUSH_DW)();
                params                   = {};
                params.pOsResource       = resMarker;
                params.dwResourceOffset  = sizeof(uint64_t);
                params.postSyncOperation = MHW_FLUSH_WRITE_TIMESTAMP_REG;
                params.bQWordEnable      = 1;
                MHW_ADDCMD_F(MI_FLUSH_DW)
                (cmdBuffer, batchBuffer);
            }

            if (!this->m_osItf->apoMosEnabled)
            {
                MOS_SafeFreeMemory(resMarker);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddBatchBufferEndInsertionFlag(MOS_COMMAND_BUFFER &constructedCmdBuf) override
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

    virtual MOS_STATUS SetCpInterface(MhwCpInterface *cpInterface, std::shared_ptr<mhw::mi::Itf> m_miItf) override
    {
        m_cpInterface = cpInterface;
        MHW_CHK_NULL_RETURN(m_cpInterface);
        MHW_MI_CHK_STATUS(m_cpInterface->RegisterMiInterfaceNext(m_miItf));
        return MOS_STATUS_SUCCESS;
    }

    virtual uint32_t GetMmioInterfaces(MHW_MMIO_REGISTER_OPCODE opCode) override
    {
        uint32_t mmioRegisters = MHW_MMIO_RCS_AUX_TABLE_NONE;

        switch (opCode)
        {
        case MHW_MMIO_RCS_AUX_TABLE_BASE_LOW:
            mmioRegisters = M_MMIO_RCS_AUX_TABLE_BASE_LOW;
            break;
        case MHW_MMIO_RCS_AUX_TABLE_BASE_HIGH:
            mmioRegisters = M_MMIO_RCS_AUX_TABLE_BASE_HIGH;
            break;
        case MHW_MMIO_RCS_AUX_TABLE_INVALIDATE:
            mmioRegisters = M_MMIO_RCS_AUX_TABLE_INVALIDATE;
            break;
        case MHW_MMIO_VD0_AUX_TABLE_BASE_LOW:
            mmioRegisters = M_MMIO_VD0_AUX_TABLE_BASE_LOW;
            break;
        case MHW_MMIO_VD0_AUX_TABLE_BASE_HIGH:
            mmioRegisters = M_MMIO_VD0_AUX_TABLE_BASE_HIGH;
            break;
        case MHW_MMIO_VD0_AUX_TABLE_INVALIDATE:
            mmioRegisters = M_MMIO_VD0_AUX_TABLE_INVALIDATE;
            break;
        case MHW_MMIO_VD1_AUX_TABLE_BASE_LOW:
            mmioRegisters = M_MMIO_VD1_AUX_TABLE_BASE_LOW;
            break;
        case MHW_MMIO_VD1_AUX_TABLE_BASE_HIGH:
            mmioRegisters = M_MMIO_VD1_AUX_TABLE_BASE_HIGH;
            break;
        case MHW_MMIO_VD1_AUX_TABLE_INVALIDATE:
            mmioRegisters = M_MMIO_VD1_AUX_TABLE_INVALIDATE;
            break;
        case MHW_MMIO_VD2_AUX_TABLE_BASE_LOW:
            mmioRegisters = M_MMIO_VD2_AUX_TABLE_BASE_LOW;
            break;
        case MHW_MMIO_VD2_AUX_TABLE_BASE_HIGH:
            mmioRegisters = M_MMIO_VD2_AUX_TABLE_BASE_HIGH;
            break;
        case MHW_MMIO_VD2_AUX_TABLE_INVALIDATE:
            mmioRegisters = M_MMIO_VD2_AUX_TABLE_INVALIDATE;
            break;
        case MHW_MMIO_VD3_AUX_TABLE_BASE_LOW:
            mmioRegisters = M_MMIO_VD3_AUX_TABLE_BASE_LOW;
            break;
        case MHW_MMIO_VD3_AUX_TABLE_BASE_HIGH:
            mmioRegisters = M_MMIO_VD3_AUX_TABLE_BASE_HIGH;
            break;
        case MHW_MMIO_VD3_AUX_TABLE_INVALIDATE:
            mmioRegisters = M_MMIO_VD3_AUX_TABLE_INVALIDATE;
            break;
        case MHW_MMIO_VE0_AUX_TABLE_BASE_LOW:
            mmioRegisters = M_MMIO_VE0_AUX_TABLE_BASE_LOW;
            break;
        case MHW_MMIO_VE0_AUX_TABLE_BASE_HIGH:
            mmioRegisters = M_MMIO_VE0_AUX_TABLE_BASE_HIGH;
            break;
        case MHW_MMIO_VE0_AUX_TABLE_INVALIDATE:
            mmioRegisters = M_MMIO_VE0_AUX_TABLE_INVALIDATE;
            break;
        case MHW_MMIO_VE1_AUX_TABLE_BASE_LOW:
            mmioRegisters = M_MMIO_VE1_AUX_TABLE_BASE_LOW;
            break;
        case MHW_MMIO_VE1_AUX_TABLE_BASE_HIGH:
            mmioRegisters = M_MMIO_VE1_AUX_TABLE_BASE_HIGH;
            break;
        case MHW_MMIO_VE1_AUX_TABLE_INVALIDATE:
            mmioRegisters = M_MMIO_VE1_AUX_TABLE_INVALIDATE;
            break;
        case MHW_MMIO_CCS0_AUX_TABLE_BASE_LOW:
            mmioRegisters = M_MMIO_CCS0_AUX_TABLE_BASE_LOW;
            break;
        case MHW_MMIO_CCS0_AUX_TABLE_BASE_HIGH:
            mmioRegisters = M_MMIO_CCS0_AUX_TABLE_BASE_HIGH;
            break;
        case MHW_MMIO_CCS0_AUX_TABLE_INVALIDATE:
            mmioRegisters = M_MMIO_CCS0_AUX_TABLE_INVALIDATE;
            break;
        default:
            MHW_ASSERTMESSAGE("Invalid mmio data provided");
            break;
        }

        return mmioRegisters;
    }

    MOS_STATUS AddProtectedProlog(MOS_COMMAND_BUFFER *cmdBuffer) override
    {
        MHW_MI_CHK_NULL(cmdBuffer);

        MHW_MI_CHK_STATUS(m_cpInterface->AddProlog(this->m_osItf, cmdBuffer));
        MHW_MI_CHK_STATUS(m_cpInterface->AddCheckForEarlyExit(this->m_osItf, cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVeboxMMIOPrologCmd(
        PMOS_COMMAND_BUFFER cmdBuffer) override
    {
        return MOS_STATUS_SUCCESS;
    }

     virtual MOS_STATUS AddBLTMMIOPrologCmd(
        PMOS_COMMAND_BUFFER cmdBuffer) override
    { 
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
         return MOS_STATUS_SUCCESS;
     }

     virtual MOS_STATUS AddSignalInSyncBatchBuffer(
         uint64_t fenceTokenValue,
         uint64_t currentValueGpuVA,
         uint64_t monitoredValueGpuVA,
         uint64_t signalValue,
         MHW_SEMAPHORE_WATI_REGISTERS &tokenRegister,
         PMOS_COMMAND_BUFFER cmdbuffer) override
     {
         return MOS_STATUS_SUCCESS;
     }
protected:
    using base_t = Itf;

    MHW_MI_MMIOREGISTERS    m_mmioRegisters = {};
    MhwCpInterface          *m_cpInterface  = nullptr;

    //! \brief Indicates the MediaReset Parameter.
    struct
    {
        uint32_t watchdogCountThreshold      = 0;
        uint32_t watchdogCountCtrlOffset     = 0;
        uint32_t watchdogCountThresholdOffset = 0;
    } MediaResetParam;

    //! \brief Watchdog timer register offsets per engine, initialized by platform.
    struct
    {
        uint32_t ctrlRcs      = 0;
        uint32_t thresholdRcs = 0;
        uint32_t ctrlVcs0      = 0;
        uint32_t thresholdVcs0 = 0;
        uint32_t ctrlVcs1      = 0;
        uint32_t thresholdVcs1 = 0;
        uint32_t ctrlVecs      = 0;
        uint32_t thresholdVecs = 0;
        uint32_t teecsCtrl      = 0;
        uint32_t thresholdTeecs = 0;
    } m_watchdogOffsets;

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
        if (nullptr == this->m_osItf)
        {
            MHW_ASSERTMESSAGE("invalid m_osInterface for RemappingMMIO");
            return false;
        }
        MOS_GPU_CONTEXT gpuContext = this->m_osItf->pfnGetGpuContext(this->m_osItf);

        if (MOS_RCS_ENGINE_USED(gpuContext) &&
            ((M_MMIO_RCS_HW_FE_REMAP_RANGE_BEGIN <= reg && reg <= M_MMIO_RCS_HW_FE_REMAP_RANGE_END)
           ||(M_MMIO_RCS_AUX_TBL_REMAP_RANGE_BEGIN <= reg && reg <= M_MMIO_RCS_AUX_TBL_REMAP_RANGE_END)
           ||(M_MMIO_RCS_TRTT_REMAP_RANGE_BEGIN <= reg && reg <= M_MMIO_RCS_TRTT_REMAP_RANGE_END)
           ||(M_MMIO_CCS0_HW_FRONT_END_BASE_BEGIN <= reg && reg <= M_MMIO_CCS0_HW_FRONT_END_BASE_END)
           ||(M_MMIO_CCS1_HW_FRONT_END_BASE_BEGIN <= reg && reg <= M_MMIO_CCS1_HW_FRONT_END_BASE_END)
           ||(M_MMIO_CCS2_HW_FRONT_END_BASE_BEGIN <= reg && reg <= M_MMIO_CCS2_HW_FRONT_END_BASE_END)
           ||(M_MMIO_CCS3_HW_FRONT_END_BASE_BEGIN <= reg && reg <= M_MMIO_CCS3_HW_FRONT_END_BASE_END)))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool IsRelativeMMIO(uint32_t &reg)
    {
        if (nullptr == this->m_osItf)
        {
            MHW_ASSERTMESSAGE("invalid m_osInterface for RelativeMMIO");
            return false;
        }
        MOS_GPU_CONTEXT gpuContext = this->m_osItf->pfnGetGpuContext(this->m_osItf);

        if ((MOS_VCS_ENGINE_USED(gpuContext) ||
             MOS_VECS_ENGINE_USED(gpuContext) ||
             MOS_TEECS_ENGINE_USED(gpuContext)) &&
            (reg >= M_MMIO_MEDIA_LOW_OFFSET && reg < M_MMIO_MEDIA_HIGH_OFFSET))
        {
            reg &= M_MMIO_MAX_RELATIVE_OFFSET;
            return true;
        }
        return false;
    }

    //! \brief    Set TEE watchdog threshold if isTee is true.
    //!           Override in platform implementations that do not support TEE watchdog.
    virtual MOS_STATUS SetTeeWatchdogThreshold()
    {
        MediaResetParam.watchdogCountThreshold = WATCHDOG_TEE_DEFAULT_WATCHDOG_THRESHOLD_IN_MS;
        return MOS_STATUS_SUCCESS;
    }

    virtual void GetWatchdogThreshold(PMOS_INTERFACE osInterface)
    {
        uint32_t countThreshold = 0;
#if (_DEBUG || _RELEASE_INTERNAL)
        // User feature config of watchdog timer threshold
        ReadUserSettingForDebug(
            m_userSettingPtr,
            countThreshold,
            __MEDIA_USER_FEATURE_VALUE_MEDIA_RESET_TH,
            MediaUserSetting::Group::Device);
        if (countThreshold != 0)
        {
            MediaResetParam.watchdogCountThreshold = countThreshold;
        }
#endif
    }

    //! \brief    Set decoder-specific watchdog threshold based on resolution.
    //!           Override in platform implementations for different threshold tiers.
    virtual void SetDecoderWatchdogThreshold(uint32_t frameWidth, uint32_t frameHeight, uint32_t codecMode)
    {
        // Default: Group A behavior
        if ((frameWidth * frameHeight) >= (7680 * 4320))
        {
            MediaResetParam.watchdogCountThreshold = MHW_MI_DECODER_8K_WATCHDOG_THRESHOLD_IN_MS;
        }
        else if ((frameWidth * frameHeight) >= (3840 * 2160))
        {
            MediaResetParam.watchdogCountThreshold = MHW_MI_DECODER_4K_WATCHDOG_THRESHOLD_IN_MS;
        }
        else
        {
            MediaResetParam.watchdogCountThreshold = MHW_MI_DECODER_720P_WATCHDOG_THRESHOLD_IN_MS;
        }

        if ((CODECHAL_STANDARD)codecMode == CODECHAL_AV1)
        {
            // This is temporary solution to address the inappropriate threshold setting for high bit-rate AV1 decode.
            // The final solution will incorporate bitstream size, increasing the setting when the bit-rate is high.
            MediaResetParam.watchdogCountThreshold = MHW_MI_DECODER_AV1_WATCHDOG_THRESHOLD_IN_MS;
        }
    }

    void InitMhwMiInterface()
    {
        MHW_FUNCTION_ENTER;

        UseGlobalGtt    = {};
        MediaResetParam = {};

        if (this->m_osItf == nullptr)
        {
            MHW_ASSERTMESSAGE("Invalid input pointers provided");
            return;
        }

        if (!this->m_osItf->bUsesGfxAddress && !this->m_osItf->bUsesPatchList)
        {
            MHW_ASSERTMESSAGE("No valid addressing mode indicated");
            return;
        }

        UseGlobalGtt.m_cs =
            UseGlobalGtt.m_vcs =
            UseGlobalGtt.m_vecs = MEDIA_IS_WA(this->m_osItf->pfnGetWaTable(this->m_osItf), WaForceGlobalGTT) ||
            !MEDIA_IS_SKU(this->m_osItf->pfnGetSkuTable(this->m_osItf), FtrPPGTT);

        MediaResetParam.watchdogCountThreshold = MHW_MI_DEFAULT_WATCHDOG_THRESHOLD_IN_MS;

        GetWatchdogThreshold(this->m_osItf);

        if (this->m_osItf->bUsesGfxAddress)
        {
            AddResourceToCmd = Mhw_AddResourceToCmd_GfxAddress;
        }
        else // if (pOsInterface->bUsesPatchList)
        {
            AddResourceToCmd = Mhw_AddResourceToCmd_PatchList;
        }
    }

    void InitMmioRegisters()
    {
        MHW_FUNCTION_ENTER;
        MHW_MI_MMIOREGISTERS* mmioRegisters = &m_mmioRegisters;

        mmioRegisters->generalPurposeRegister0LoOffset  = GP_REGISTER0_LO_OFFSET;
        mmioRegisters->generalPurposeRegister0HiOffset  = GP_REGISTER0_HI_OFFSET;
        mmioRegisters->generalPurposeRegister4LoOffset  = GP_REGISTER4_LO_OFFSET;
        mmioRegisters->generalPurposeRegister4HiOffset  = GP_REGISTER4_HI_OFFSET;
        mmioRegisters->generalPurposeRegister11LoOffset = GP_REGISTER11_LO_OFFSET;
        mmioRegisters->generalPurposeRegister11HiOffset = GP_REGISTER11_HI_OFFSET;
        mmioRegisters->generalPurposeRegister12LoOffset = GP_REGISTER12_LO_OFFSET;
        mmioRegisters->generalPurposeRegister12HiOffset = GP_REGISTER12_HI_OFFSET;
    }

public:
    Impl(PMOS_INTERFACE osItf) : mhw::Impl(osItf)
    {
        MHW_FUNCTION_ENTER;
    }
    _MHW_SETCMD_OVERRIDE_DECL(MI_SEMAPHORE_SIGNAL)
    {
        _MHW_SETCMD_CALLBASE(MI_SEMAPHORE_SIGNAL);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(MI_SEMAPHORE_WAIT)
    {
        _MHW_SETCMD_CALLBASE(MI_SEMAPHORE_WAIT);

        if (params.presSemaphoreMem)
        {
            MHW_MI_CHK_NULL(this->m_currentCmdBuf);
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
        }
        else if (params.gpuVirtualAddress != 0)
        {
            cmd.DW2_3.SemaphoreAddress = (params.gpuVirtualAddress) >> MHW_COMMON_MI_GENERAL_SHIFT;
        }
        else
        {
            MHW_ASSERTMESSAGE("Invalid parameter, both resource and gpuva zero.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        cmd.DW0.MemoryType         = IsGlobalGttInUse();
        cmd.DW0.WaitMode           = params.bPollingWaitMode;
        cmd.DW0.CompareOperation   = params.CompareOperation;
        cmd.DW1.SemaphoreDataDword = params.dwSemaphoreData;
        cmd.DW4.WaitTokenNumber    = params.waitTokenNumber;

        return MOS_STATUS_SUCCESS;
    }

#ifdef _MEDIA_RESERVED
    _MHW_SETCMD_OVERRIDE_DECL(MI_SEMAPHORE_WAIT_64)
    {
        return MOS_STATUS_SUCCESS;
    }
#endif

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

        if (params.presStoreBuffer)
        {
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
        }
        else if (params.gpuVirtualAddress)
        {
            cmd.DW2_3.MemoryAddress = (params.gpuVirtualAddress) >> MHW_COMMON_MI_GENERAL_SHIFT;
        }
        else
        {
            MHW_ASSERTMESSAGE("Invalid parameter, both resource and gpuva zero.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

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
        if (params.pOsResource)
        {
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
        }
        else if (params.gpuVirtualAddress != 0)
        {
            cmd.DW1.MemoryAddress = (params.gpuVirtualAddress) >> MHW_COMMON_MI_GENERAL_SHIFT;
            cmd.DW2.MemoryAddressHigh = ((params.gpuVirtualAddress & 0xFFFFFFFF00000000) >> 32);
        }

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
