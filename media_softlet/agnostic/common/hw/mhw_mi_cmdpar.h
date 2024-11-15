/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     mhw_mi_cmdpar.h
//! \brief    MHW command parameters
//! \details
//!

#ifndef __MHW_MI_CMDPAR_H__
#define __MHW_MI_CMDPAR_H__

#include "mhw_cmdpar.h"
#include "mhw_state_heap.h"
#include "mhw_mmio_common.h"

namespace mhw
{
namespace mi
{

    enum MHW_COMMON_MI_SEMAPHORE_COMPARE_OPERATION
    {
        MHW_MI_SAD_GREATER_THAN_SDD          = 0,
        MHW_MI_SAD_GREATER_THAN_OR_EQUAL_SDD = 1,
        MHW_MI_SAD_LESS_THAN_SDD             = 2,
        MHW_MI_SAD_LESS_THAN_OR_EQUAL_SDD    = 3,
        MHW_MI_SAD_EQUAL_SDD                 = 4,
        MHW_MI_SAD_NOT_EQUAL_SDD             = 5,
    };

    enum MHW_COMMON_MI_ATOMIC_OPCODE
    {
        MHW_MI_ATOMIC_NONE = 0,
        MHW_MI_ATOMIC_AND  = 1,
        MHW_MI_ATOMIC_OR   = 2,
        MHW_MI_ATOMIC_XOR  = 3,
        MHW_MI_ATOMIC_MOVE = 4,
        MHW_MI_ATOMIC_INC  = 5,
        MHW_MI_ATOMIC_DEC  = 6,
        MHW_MI_ATOMIC_ADD  = 7,
        MHW_MI_ATOMIC_SUB  = 8,
        MHW_MI_ATOMIC_RSUB = 9,
        MHW_MI_ATOMIC_IMAX = 10,
        MHW_MI_ATOMIC_IMIN = 11,
        MHW_MI_ATOMIC_UMAX = 12,
        MHW_MI_ATOMIC_UMIN = 13,
        MHW_MI_ATOMIC_CMP  = 14,
        MHW_MI_ATOMIC_MAX  = 15,

        MHW_MI_ATOMIC_DWORD = 0,
        MHW_MI_ATOMIC_QWORD = 0x20,
        MHW_MI_ATOMIC_OCTWORD = 0x40,
    };

    enum MHW_MI_POST_SYNC_OPERATION
    {
        MHW_FLUSH_NOWRITE               = 0,
        MHW_FLUSH_WRITE_IMMEDIATE_DATA  = 1,
        MHW_FLUSH_WRITE_TIMESTAMP_REG   = 3
    };

    enum MHW_COMMON_MI_ADDRESS_SHIFT
    {
        MHW_COMMON_MI_GENERAL_SHIFT                         = 2,
        MHW_COMMON_MI_PIPE_CONTROL_SHIFT                    = 3,
        MHW_COMMON_MI_FLUSH_DW_SHIFT                        = 3,
        MHW_COMMON_MI_STORE_DATA_DW_SHIFT                   = 2,          //when write DW to memory, algin with 4 bytes.
        MHW_COMMON_MI_STORE_DATA_QW_SHIFT                   = 3,          //when write QW to memory, algin with 8 bytes
        MHW_COMMON_MI_CONDITIONAL_BATCH_BUFFER_END_SHIFT    = 3,
    };

    enum MHW_FLUSH_OPERATION
    {
        MHW_FLUSH_NONE = 0,              // No flush
        MHW_FLUSH_WRITE_CACHE,           // Flush write cache
        MHW_FLUSH_READ_CACHE,            // Flush read cache
        MHW_FLUSH_CUSTOM                 // Flush with custom parameters
    };

    enum MHW_MMIO_REGISTER_OPCODE
    {
        MHW_MMIO_RCS_AUX_TABLE_NONE        = 0,
        MHW_MMIO_RCS_AUX_TABLE_BASE_LOW    = 1,
        MHW_MMIO_RCS_AUX_TABLE_BASE_HIGH   = 2,
        MHW_MMIO_RCS_AUX_TABLE_INVALIDATE  = 3,
        MHW_MMIO_VD0_AUX_TABLE_BASE_LOW    = 4,
        MHW_MMIO_VD0_AUX_TABLE_BASE_HIGH   = 5,
        MHW_MMIO_VD0_AUX_TABLE_INVALIDATE  = 6,
        MHW_MMIO_VD1_AUX_TABLE_BASE_LOW    = 7,
        MHW_MMIO_VD1_AUX_TABLE_BASE_HIGH   = 8,
        MHW_MMIO_VD1_AUX_TABLE_INVALIDATE  = 9,
        MHW_MMIO_VD2_AUX_TABLE_BASE_LOW    = 10,
        MHW_MMIO_VD2_AUX_TABLE_BASE_HIGH   = 11,
        MHW_MMIO_VD2_AUX_TABLE_INVALIDATE  = 12,
        MHW_MMIO_VD3_AUX_TABLE_BASE_LOW    = 13,
        MHW_MMIO_VD3_AUX_TABLE_BASE_HIGH   = 14,
        MHW_MMIO_VD3_AUX_TABLE_INVALIDATE  = 15,
        MHW_MMIO_VE0_AUX_TABLE_BASE_LOW    = 16,
        MHW_MMIO_VE0_AUX_TABLE_BASE_HIGH   = 17,
        MHW_MMIO_VE0_AUX_TABLE_INVALIDATE  = 18,
        MHW_MMIO_VE1_AUX_TABLE_BASE_LOW    = 19,
        MHW_MMIO_VE1_AUX_TABLE_BASE_HIGH   = 20,
        MHW_MMIO_VE1_AUX_TABLE_INVALIDATE  = 21,
        MHW_MMIO_CCS0_AUX_TABLE_BASE_LOW   = 22,
        MHW_MMIO_CCS0_AUX_TABLE_BASE_HIGH  = 23,
        MHW_MMIO_BLT_AUX_TABLE_BASE_LOW    = 24,
        MHW_MMIO_BLT_AUX_TABLE_BASE_HIGH   = 25,
        MHW_MMIO_CCS0_AUX_TABLE_INVALIDATE = 26,
    };

    struct MHW_MI_ALU_PARAMS
    {
        // DW 0
        union
        {
            struct
            {
                uint32_t    Operand2    : MOS_BITFIELD_RANGE(0, 9);      // Operand-2
                uint32_t    Operand1    : MOS_BITFIELD_RANGE(10, 19);    // Operand-1
                uint32_t    AluOpcode   : MOS_BITFIELD_RANGE(20, 31);    // ALU OPCODE
            };
            uint32_t        Value;
        };
    };

    struct MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS
    {
        PMOS_RESOURCE               presSemaphoreBuffer = nullptr;
        uint32_t                    dwOffset            = 0;
        uint32_t                    dwValue             = 0;
        bool                        bDisableCompareMask = false;
        uint32_t                    dwParamsType        = 0;         //reserved
    };

    struct MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS : public MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS
    {
        bool                        enableEndCurrentBatchBuffLevel = false;
        uint32_t                    compareOperation               = 0;
        enum PARAMS_TYPE
        {
            ENHANCED_PARAMS = 1
        };
    };

    struct _MHW_PAR_T(MI_SEMAPHORE_WAIT)
    {
        PMOS_RESOURCE               presSemaphoreMem   = nullptr;        // Semaphore memory Resource
        uint64_t                    gpuVirtualAddress  = 0;
        uint32_t                    dwResourceOffset   = 0;
        bool                        bRegisterPollMode  = false;
        bool                        bPollingWaitMode   = false;
        uint32_t                    dwCompareOperation = 0;
        uint32_t                    dwSemaphoreData    = 0;
        bool                        b64bCompareEnableWithGPR = 0;
        MHW_COMMON_MI_SEMAPHORE_COMPARE_OPERATION CompareOperation = {};
    };

    struct _MHW_PAR_T(PIPE_CONTROL)
    {
        PMOS_RESOURCE           presDest                      = nullptr;
        uint32_t                dwResourceOffset              = 0;
        uint32_t                dwDataDW1                     = 0;
        uint32_t                dwDataDW2                     = 0;
        uint32_t                dwFlushMode                   = 0;
        uint32_t                dwPostSyncOp                  = 0;
        bool                    bDisableCSStall               = false;
        bool                    bInvalidateStateCache         = false;
        bool                    bInvalidateConstantCache      = false;
        bool                    bInvalidateVFECache           = false;
        bool                    bInvalidateInstructionCache   = false;
        bool                    bFlushRenderTargetCache       = false;
        bool                    bTlbInvalidate                = false;
        bool                    bInvalidateTextureCache       = false;
        bool                    bGenericMediaStateClear       = false;
        bool                    bIndirectStatePointersDisable = false;
        bool                    bUnTypedDataPortCacheFlush    = false;
        bool                    bHdcPipelineFlush             = false;
        bool                    bKernelFenceEnabled           = false;
        bool                    bPPCFlush                     = false;
    };

    struct _MHW_PAR_T(MI_BATCH_BUFFER_START)
    {
        PMOS_RESOURCE               presResource           = nullptr;
        bool                        secondLevelBatchBuffer = true;
    };

    struct _MHW_PAR_T(MI_CONDITIONAL_BATCH_BUFFER_END)
    {
        PMOS_RESOURCE               presSemaphoreBuffer            = nullptr;
        uint32_t                    dwOffset                       = 0;
        uint32_t                    dwValue                        = 0;
        bool                        bDisableCompareMask            = false;
        uint32_t                    dwParamsType                   = 0;         //reserved
        bool                        enableEndCurrentBatchBuffLevel = false;
        uint32_t                    compareOperation               = 0;
    };

    struct _MHW_PAR_T(MI_SET_PREDICATE)
    {
        uint32_t                    PredicateEnable = 0;      // Debug Counter Control
    };

    struct _MHW_PAR_T(MI_STORE_REGISTER_MEM)
    {
        PMOS_RESOURCE               presStoreBuffer = nullptr;
        uint32_t                    dwOffset        = 0;
        uint32_t                    dwRegister      = 0;
        uint32_t                    dwOption        = 0;
    };

    struct _MHW_PAR_T(MI_LOAD_REGISTER_MEM)
    {
        PMOS_RESOURCE               presStoreBuffer = nullptr;
        uint32_t                    dwOffset        = 0;
        uint32_t                    dwRegister      = 0;
        uint32_t                    dwOption        = 0;
    };

    struct _MHW_PAR_T(MI_LOAD_REGISTER_IMM)
    {
        uint32_t                    dwRegister = 0;
        uint32_t                    dwData     = 0;
        bool                        bMMIORemap = 0;
    };

    struct _MHW_PAR_T(MI_LOAD_REGISTER_REG)
    {
        uint32_t                    dwSrcRegister = 0;
        uint32_t                    dwDstRegister = 0;
    };

    struct _MHW_PAR_T(MI_FORCE_WAKEUP)
    {
        bool               bForceMediaSlice0Awake     = false; //!< Force Media-Slice0 Awake
        bool               bForceRenderAwake          = false; //!< Force Render Awake
        bool               bForceMediaSlice1Awake     = false; //!< Force Media-Slice1 Awake
        bool               bForceMediaSlice2Awake     = false; //!< Force Media-Slice2 Awake
        bool               bForceMediaSlice3Awake     = false; //!< Force Media-Slice3 Awake
        bool               bHEVCPowerWellControl      = false; //!< HEVC Power Well Control
        bool               bMFXPowerWellControl       = false; //!< MFX Power Well Control
        bool               bForceMediaSlice0AwakeMask = false; //!< Force Media-Slice0 Awake Mask
        bool               bForceRenderAwakeMask      = false; //!< Force Render Awake Mask
        bool               bForceMediaSlice1AwakeMask = false; //!< Force Media-Slice1 Awake Mask
        bool               bForceMediaSlice2AwakeMask = false; //!< Force Media-Slice2 Awake Mask
        bool               bForceMediaSlice3AwakeMask = false; //!< Force Media-Slice3 Awake Mask
        bool               bHEVCPowerWellControlMask  = false; //!< HEVC Power Well Control Mask
        bool               bMFXPowerWellControlMask   = false; //!< MFX Power Well Control Mask
    };

    struct _MHW_PAR_T(MEDIA_STATE_FLUSH)
    {
        bool                bFlushToGo                   = false;
        uint8_t             ui8InterfaceDescriptorOffset = 0;
    };

    struct _MHW_PAR_T(MI_FLUSH_DW)
    {
        PMOS_RESOURCE               pOsResource                   = nullptr;          // Target OS Resource
        uint32_t                    dwResourceOffset              = 0;
        uint32_t                    dwDataDW1                     = 0;                // Value to Write
        uint32_t                    dwDataDW2                     = 0;
        bool                        bVideoPipelineCacheInvalidate = false;
        uint32_t                    postSyncOperation             = 0;
        uint32_t                    bQWordEnable                  = 0;
        bool                        bEnablePPCFlush               = false;
    };

    struct _MHW_PAR_T(VD_CONTROL_STATE)
    {
        bool                        vdencEnabled           = false;
        bool                        avpEnabled             = false;
        bool                        initialization         = false;
        bool                        vdencInitialization    = false;
        bool                        scalableModePipeLock   = false;
        bool                        scalableModePipeUnlock = false;
        bool                        memoryImplicitFlush    = false;
    };

    struct _MHW_PAR_T(MI_BATCH_BUFFER_END)
    {
        MOS_RESOURCE                OsResource              = {};
        int32_t                     iRemaining              = 0;       //!< Remaining space in the BB
        int32_t                     iSize                   = 0;       //!< Command buffer size
        uint32_t                    count                   = 0;       //!< Actual batch count in this resource. If larger than 1, multiple buffer has equal size and resource size count * size.
        int32_t                     iCurrent                = 0;       //!< Current offset in CB
        bool                        bLocked                 = false;   //!< True if locked in memory (pData must be valid)
        uint8_t                     *pData                  = nullptr; //!< Pointer to BB data
#if (_DEBUG || _RELEASE_INTERNAL)
        int32_t                     iLastCurrent            = 0;       //!< Save offset in CB (for debug plug-in/out)
#endif

        // User defined
        bool                        bSecondLevel            = false;   //!< REMOVE REMOVE
        uint32_t                    dwOffset                = 0;       //!< Offset to the data in the OS resource

        // Batch Buffer synchronization logic
        bool                        bBusy                   = false;   //!< Busy flag (clear when Sync Tag is reached)
        uint32_t                    dwCmdBufId              = 0;       //!< Command Buffer ID for the workload
        PMHW_BATCH_BUFFER           pNext                   = nullptr; //!< Next BB in the sync list
        PMHW_BATCH_BUFFER           pPrev                   = nullptr; //!< Prev BB in the sync list

        // Batch Buffer Client Private Data
        uint32_t                    dwSyncTag               = 0;
        bool                        bMatch                  = false;
        int32_t                     iPrivateType            = 0;       //!< Indicates the BB client
        int32_t                     iPrivateSize            = 0;       //!< Size of the current render args
        void                        *pPrivateData           = nullptr; //!< Pointer to private BB data
    };

    struct _MHW_PAR_T(MI_NOOP)
    {
    };

    struct _MHW_PAR_T(MI_ATOMIC)
    {
        PMOS_RESOURCE               pOsResource       = nullptr;     // Target OS Resource
        uint32_t                    dwResourceOffset  = 0;
        bool                        bReturnData       = false;
        bool                        bInlineData       = false;
        uint32_t                    dwOperand1Data[4] = {};          // Values to Write
        uint32_t                    dwOperand2Data[4] = {};          // Values to Write
        uint32_t                    dwDataSize        = 0;
        MHW_COMMON_MI_ATOMIC_OPCODE Operation         = {};
    };

    struct _MHW_PAR_T(MI_STORE_DATA_IMM)
    {
        PMOS_RESOURCE               pOsResource      = nullptr;       // Target OS Resource
        uint32_t                    dwResourceOffset = 0;
        uint32_t                    dwValue          = 0;             // Value to Write
    };

    struct _MHW_PAR_T(MI_MATH)
    {
        MHW_MI_ALU_PARAMS           *pAluPayload   = nullptr;
        uint32_t                    dwNumAluParams = 0;
    };

    struct _MHW_PAR_T(MI_COPY_MEM_MEM)
    {
        PMOS_RESOURCE               presSrc        = nullptr;
        uint32_t                    dwSrcOffset    = 0;
        PMOS_RESOURCE               presDst        = nullptr;
        uint32_t                    dwDstOffset    = 0;
    };
    
    struct _MHW_PAR_T(MFX_WAIT)
    {
        bool                        iStallVdboxPipeline = false;
    };
    

}  // namespace mi
}  // namespace mhw

#endif  // __MHW_MI_CMDPAR_H__
