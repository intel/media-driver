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
//! \file     mhw_mi.h
//! \brief    MHW interface for MI and generic flush commands across all engines
//! \details  Impelements the functionalities common across all platforms for MHW_MI
//!

#ifndef __MHW_MI_H__
#define __MHW_MI_H__

class MhwCpInterface;

#include "mos_os.h"
#include "mhw_utilities.h"
#include "mhw_cp_interface.h"
#include "mhw_mmio.h"

#define MHW_MI_WATCHDOG_ENABLE_COUNTER                  0x0
#define MHW_MI_WATCHDOG_DISABLE_COUNTER                 0x1
#define MHW_MI_DEFAULT_WATCHDOG_THRESHOLD_IN_MS         60
#define MHW_MI_ENCODER_16K_WATCHDOG_THRESHOLD_IN_MS     2000
#define MHW_MI_ENCODER_8K_WATCHDOG_THRESHOLD_IN_MS      500
#define MHW_MI_ENCODER_4K_WATCHDOG_THRESHOLD_IN_MS      100
#define MHW_MI_ENCODER_FHD_WATCHDOG_THRESHOLD_IN_MS     50
#define MHW_MI_DECODER_720P_WATCHDOG_THRESHOLD_IN_MS    10
#define MHW_MI_DECODER_16K_WATCHDOG_THRESHOLD_IN_MS     180
#define MHW_MI_DECODER_16Kx16K_WATCHDOG_THRESHOLD_IN_MS 256
#define MHW_MI_WATCHDOG_COUNTS_PER_MILLISECOND         (19200123 / 1000)   // Time stamp counts per millisecond

typedef enum _MHW_COMMON_MI_ADDRESS_SHIFT
{
    MHW_COMMON_MI_GENERAL_SHIFT                         = 2,
    MHW_COMMON_MI_PIPE_CONTROL_SHIFT                    = 3,
    MHW_COMMON_MI_FLUSH_DW_SHIFT                        = 3,
    MHW_COMMON_MI_STORE_DATA_DW_SHIFT                   = 2,          //when write DW to memory, algin with 4 bytes.
    MHW_COMMON_MI_STORE_DATA_QW_SHIFT                   = 3,          //when write QW to memory, algin with 8 bytes
    MHW_COMMON_MI_CONDITIONAL_BATCH_BUFFER_END_SHIFT    = 3,
} MHW_COMMON_MI_ADDRESS_SHIFT;

typedef enum _MHW_MI_SET_PREDICATE_ENABLE
{
    MHW_MI_SET_PREDICATE_ENABLE_ALWAYS   = 0x0,
    MHW_MI_SET_PREDICATE_ENABLE_ON_CLEAR,
    MHW_MI_SET_PREDICATE_ENABLE_ON_SET,
    MHW_MI_SET_PREDICATE_DISABLE,
} MHW_MI_SET_PREDICATE_ENABLE;

typedef enum _MHW_MI_POST_SYNC_OPERATION
{
    MHW_FLUSH_NOWRITE               = 0,
    MHW_FLUSH_WRITE_IMMEDIATE_DATA  = 1,
    MHW_FLUSH_WRITE_TIMESTAMP_REG   = 3
} MHW_MI_POST_SYNC_OPERATION;

typedef enum _MHW_COMMON_MI_ATOMIC_OPCODE
{
    MHW_MI_ATOMIC_NONE = 0,
    MHW_MI_ATOMIC_AND = 1,
    MHW_MI_ATOMIC_OR = 2,
    MHW_MI_ATOMIC_XOR = 3,
    MHW_MI_ATOMIC_MOVE = 4,
    MHW_MI_ATOMIC_INC = 5,
    MHW_MI_ATOMIC_DEC = 6,
    MHW_MI_ATOMIC_ADD = 7,
    MHW_MI_ATOMIC_SUB = 8,
    MHW_MI_ATOMIC_RSUB = 9,
    MHW_MI_ATOMIC_IMAX = 10,
    MHW_MI_ATOMIC_IMIN = 11,
    MHW_MI_ATOMIC_UMAX = 12,
    MHW_MI_ATOMIC_UMIN = 13,
    MHW_MI_ATOMIC_CMP = 14,
    MHW_MI_ATOMIC_MAX = 15
} MHW_COMMON_MI_ATOMIC_OPCODE;

typedef enum _MHW_COMMON_MI_SEMAPHORE_COMPARE_OPERATION
{
    MHW_MI_SAD_GREATER_THAN_SDD             = 0,
    MHW_MI_SAD_GREATER_THAN_OR_EQUAL_SDD    = 1,
    MHW_MI_SAD_LESS_THAN_SDD                = 2,
    MHW_MI_SAD_LESS_THAN_OR_EQUAL_SDD       = 3,
    MHW_MI_SAD_EQUAL_SDD                    = 4,
    MHW_MI_SAD_NOT_EQUAL_SDD                = 5,
} MHW_COMMON_MI_SEMAPHORE_COMPARE_OPERATION;

typedef enum _MHW_MI_ALU_OPCODE
{
    MHW_MI_ALU_NOOP = 0x0,
    MHW_MI_ALU_LOAD = 0x80,
    MHW_MI_ALU_LOADINV = 0x480,
    MHW_MI_ALU_LOAD0 = 0x81,
    MHW_MI_ALU_LOAD1 = 0x481,
    MHW_MI_ALU_ADD = 0x100,
    MHW_MI_ALU_SUB = 0x101,
    MHW_MI_ALU_AND = 0x102,
    MHW_MI_ALU_OR = 0x103,
    MHW_MI_ALU_XOR = 0x104,
    MHW_MI_ALU_STORE = 0x180,
    MHW_MI_ALU_STOREINV = 0x580
} MHW_MI_ALU_OPCODE;

typedef enum _MHW_MI_ALU_REG
{
    MHW_MI_ALU_GPREG0 = 0,
    MHW_MI_ALU_GPREG1,
    MHW_MI_ALU_GPREG2,
    MHW_MI_ALU_GPREG3,
    MHW_MI_ALU_GPREG4,
    MHW_MI_ALU_GPREG5,
    MHW_MI_ALU_GPREG6,
    MHW_MI_ALU_GPREG7,
    MHW_MI_ALU_GPREG8,
    MHW_MI_ALU_GPREG9,
    MHW_MI_ALU_GPREG10,
    MHW_MI_ALU_GPREG11,
    MHW_MI_ALU_GPREG12,
    MHW_MI_ALU_GPREG13,
    MHW_MI_ALU_GPREG14,
    MHW_MI_ALU_GPREG15,
    MHW_MI_ALU_SRCA = 0x20,
    MHW_MI_ALU_SRCB = 0x21,
    MHW_MI_ALU_ACCU = 0x31,
    MHW_MI_ALU_ZF = 0x32,
    MHW_MI_ALU_CF = 0x33
} MHW_MI_ALU_REG;

typedef enum _MHW_FLUSH_OPERATION
{
    MHW_FLUSH_NONE = 0,              // No flush
    MHW_FLUSH_WRITE_CACHE,           // Flush write cache
    MHW_FLUSH_READ_CACHE,            // Flush read cache
    MHW_FLUSH_CUSTOM                 // Flush with custom parameters
} MHW_FLUSH_OPERATION;

typedef struct _MHW_PIPE_CONTROL_PARAMS
{
    PMOS_RESOURCE           presDest;
    uint32_t                dwResourceOffset;
    uint32_t                dwDataDW1;
    uint32_t                dwDataDW2;
    uint32_t                dwFlushMode;
    uint32_t                dwPostSyncOp;
    uint32_t                bDisableCSStall                : 1;
    uint32_t                bInvalidateStateCache          : 1;
    uint32_t                bInvalidateConstantCache       : 1;
    uint32_t                bInvalidateVFECache            : 1;
    uint32_t                bInvalidateInstructionCache    : 1;
    uint32_t                bFlushRenderTargetCache        : 1;
    uint32_t                bTlbInvalidate                 : 1;
    uint32_t                bInvalidateTextureCache        : 1;
    uint32_t                bGenericMediaStateClear        : 1;
    uint32_t                bIndirectStatePointersDisable  : 1;
} MHW_PIPE_CONTROL_PARAMS, *PMHW_PIPE_CONTROL_PARAMS;

typedef struct _MHW_MI_COPY_MEM_MEM_PARAMS
{
    PMOS_RESOURCE               presSrc;
    uint32_t                    dwSrcOffset;
    PMOS_RESOURCE               presDst;
    uint32_t                    dwDstOffset;
} MHW_MI_COPY_MEM_MEM_PARAMS, *PMHW_MI_COPY_MEM_MEM_PARAMS;

typedef struct _MHW_MI_STORE_REGISTER_MEM_PARAMS
{
    PMOS_RESOURCE               presStoreBuffer;
    uint32_t                    dwOffset;
    uint32_t                    dwRegister;
} MHW_MI_STORE_REGISTER_MEM_PARAMS, *PMHW_MI_STORE_REGISTER_MEM_PARAMS;

typedef MHW_MI_STORE_REGISTER_MEM_PARAMS MHW_MI_LOAD_REGISTER_MEM_PARAMS, *PMHW_MI_LOAD_REGISTER_MEM_PARAMS;

typedef struct _MHW_MI_LOAD_REGISTER_REG_PARAMS
{
    uint32_t                    dwSrcRegister;
    uint32_t                    dwDstRegister;
} MHW_MI_LOAD_REGISTER_REG_PARAMS, *PMHW_MI_LOAD_REGISTER_REG_PARAMS;

typedef struct _MHW_MI_ALU_PARAMS
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
} MHW_MI_ALU_PARAMS, *PMHW_MI_ALU_PARAMS;

typedef struct _MHW_MI_MATH_PARAMS
{
    PMHW_MI_ALU_PARAMS          pAluPayload;
    uint32_t                    dwNumAluParams;
} MHW_MI_MATH_PARAMS, *PMHW_MI_MATH_PARAMS;

typedef struct _MHW_MI_LOAD_REGISTER_IMM_PARAMS
{
    uint32_t                    dwRegister;
    uint32_t                    dwData;
} MHW_MI_LOAD_REGISTER_IMM_PARAMS, *PMHW_MI_LOAD_REGISTER_IMM_PARAMS;

typedef struct _MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS
{
    PMOS_RESOURCE               presSemaphoreBuffer;
    uint32_t                    dwOffset;
    uint32_t                    dwValue;
    bool                        bDisableCompareMask;
    uint32_t                    dwParamsType;         //reserved
} MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS, *PMHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS;

typedef struct _MHW_MI_STORE_DATA_PARAMS
{
    PMOS_RESOURCE               pOsResource;                                    // Target OS Resource
    uint32_t                    dwResourceOffset;
    uint32_t                    dwValue;                                        // Value to Write
} MHW_MI_STORE_DATA_PARAMS, *PMHW_MI_STORE_DATA_PARAMS;

typedef struct _MHW_MI_FLUSH_DW_PARAMS
{
    PMOS_RESOURCE               pOsResource;                                    // Target OS Resource
    uint32_t                    dwResourceOffset;
    uint32_t                    dwDataDW1;                                        // Value to Write
    uint32_t                    dwDataDW2;
    bool                        bVideoPipelineCacheInvalidate;
    uint32_t                    postSyncOperation;
    uint32_t                    bQWordEnable;
} MHW_MI_FLUSH_DW_PARAMS, *PMHW_MI_FLUSH_DW_PARAMS;

typedef struct _MHW_MI_ATOMIC_PARAMS
{
    PMOS_RESOURCE               pOsResource;        // Target OS Resource
    uint32_t                    dwResourceOffset;
    bool                        bReturnData;
    bool                        bInlineData;
    uint32_t                    dwOperand1Data[4];  // Values to Write
    uint32_t                    dwOperand2Data[4];  // Values to Write
    uint32_t                    dwDataSize;
    MHW_COMMON_MI_ATOMIC_OPCODE Operation;
} MHW_MI_ATOMIC_PARAMS, *PMHW_MI_ATOMIC_PARAMS;

typedef struct _MHW_MI_SEMAPHORE_WAIT_PARAMS
{
    PMOS_RESOURCE               presSemaphoreMem;        // Semaphore memory Resource
    uint32_t                    dwResourceOffset;
    bool                        bRegisterPollMode;
    bool                        bPollingWaitMode;
    uint32_t                    dwCompareOperation;
    uint32_t                    dwSemaphoreData;
    MHW_COMMON_MI_SEMAPHORE_COMPARE_OPERATION       CompareOperation;
}MHW_MI_SEMAPHORE_WAIT_PARAMS, *PMHW_MI_SEMAPHORE_WAIT_PARAMS;

typedef struct _MHW_MEDIA_STATE_FLUSH_PARAM
{
    bool                bFlushToGo;
    uint8_t             ui8InterfaceDescriptorOffset;
} MHW_MEDIA_STATE_FLUSH_PARAM, *PMHW_MEDIA_STATE_FLUSH_PARAM;

typedef struct _MHW_MI_FORCE_WAKEUP_PARAMS
{
    uint32_t               bForceMediaSlice0Awake          : 1; //!< Force Media-Slice0 Awake
    uint32_t               bForceRenderAwake               : 1; //!< Force Render Awake
    uint32_t               bForceMediaSlice1Awake          : 1; //!< Force Media-Slice1 Awake
    uint32_t               bForceMediaSlice2Awake          : 1; //!< Force Media-Slice2 Awake
    uint32_t               bForceMediaSlice3Awake          : 1; //!< Force Media-Slice3 Awake
    uint32_t               Reserved5                       : 3; //!< Reserved
    uint32_t               bHEVCPowerWellControl           : 1; //!< HEVC Power Well Control
    uint32_t               bMFXPowerWellControl            : 1; //!< MFX Power Well Control
    uint32_t               Reserved10                      : 6; //!< Reserved
    uint32_t               bForceMediaSlice0AwakeMask      : 1; //!< Force Media-Slice0 Awake Mask
    uint32_t               bForceRenderAwakeMask           : 1; //!< Force Render Awake Mask
    uint32_t               bForceMediaSlice1AwakeMask      : 1; //!< Force Media-Slice1 Awake Mask
    uint32_t               bForceMediaSlice2AwakeMask      : 1; //!< Force Media-Slice2 Awake Mask
    uint32_t               bForceMediaSlice3AwakeMask      : 1; //!< Force Media-Slice3 Awake Mask
    uint32_t               ReservedMask23_21               : 3; //!< Reserved Mask
    uint32_t               bHEVCPowerWellControlMask       : 1; //!< HEVC Power Well Control Mask
    uint32_t               bMFXPowerWellControlMask        : 1; //!< MFX Power Well Control Mask
    uint32_t               Reserved31_26                   : 6; //!< Reserved Mask
} MHW_MI_FORCE_WAKEUP_PARAMS, *PMHW_MI_FORCE_WAKEUP_PARAMS;


class MhwMiInterface
{
public:
    virtual ~MhwMiInterface() { MHW_FUNCTION_ENTER; }

    //!
    //! \brief    Adds MI_NOOP_CMD to the buffer provided
    //! \details  Either the command or batch buffer must be valid
    //! \param    [in] cmdBuffer
    //!           If valid, command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           If valid, batch buffer to which HW command is added
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiNoop(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER   batchBuffer) = 0;

    //!
    //! \brief    Adds MI_BATCH_BUFFER_END to the buffer provided
    //! \details  Either the command or batch buffer must be valid
    //! \param    [in] cmdBuffer
    //!           If valid, command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           If valid, batch buffer to which HW command is added
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiBatchBufferEnd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer) = 0;

    //!
    //! \brief    Add batch buffer end insertion flag
    //! 
    //! \param    [in/out] constructedCmdBuf
    //!           Constructed cmd buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddBatchBufferEndInsertionFlag(
        MOS_COMMAND_BUFFER &constructedCmdBuf) = 0;

    //!
    //! \brief    Adds MI_BATCH_BUFFER_END to the buffer provided without any WA
    //! \details  Either the command or batch buffer must be valid
    //! \param    [in] cmdBuffer
    //!           If valid, command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           If valid, batch buffer to which HW command is added
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiBatchBufferEndOnly(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer) = 0;

    //!
    //! \brief    Adds MI_CONDITIONAL_BATCH_BUFFER_END to the command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which requested command is added
    //! \param    [in] params
    //!           Parameters used to populate the requested command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiConditionalBatchBufferEndCmd(
        PMOS_COMMAND_BUFFER                             cmdBuffer,
        PMHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS     params) = 0;

    //!
    //! \brief    Adds MI_BATCH_BUFFER_START to the command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which requested command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to refer to in MI_BATCH_BUFFER_START
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiBatchBufferStartCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer) = 0;

    //!
    //! \brief    Adds MI_STORE_DATA_IMM to the command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which requested command is added
    //! \param    [in] params
    //!           Parameters used to populate the requested command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiStoreDataImmCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_MI_STORE_DATA_PARAMS       params) = 0;

    //!
    //! \brief    Adds MI_FLUSH_DW to the command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which requested command is added
    //! \param    [in] params
    //!           Parameters used to populate the requested command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiFlushDwCmd(
        PMOS_COMMAND_BUFFER         cmdBuffer,
        PMHW_MI_FLUSH_DW_PARAMS     params) = 0;

    //!
    //! \brief    Adds MI_FORCE_WAKEUP to the command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which requested command is added
    //! \param    [in] params
    //!           Parameters used to populate the requested command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiForceWakeupCmd(
        PMOS_COMMAND_BUFFER         cmdBuffer,
        PMHW_MI_FORCE_WAKEUP_PARAMS     params)
    {
        return MOS_STATUS_SUCCESS;
    };

    //!
    //! \brief    Adds MI_COPY_MEM_MEM to the command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which requested command is added
    //! \param    [in] params
    //!           Parameters used to populate the requested command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiCopyMemMemCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_COPY_MEM_MEM_PARAMS         params) = 0;

    //!
    //! \brief    Adds MI_STORE_REGISTER_MEM to the command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which requested command is added
    //! \param    [in] params
    //!           Parameters used to populate the requested command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiStoreRegisterMemCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_STORE_REGISTER_MEM_PARAMS   params) = 0;

    //!
    //! \brief    Adds MI_LOAD_REGISTER_MEM to the command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which requested command is added
    //! \param    [in] params
    //!           Parameters used to populate the requested command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiLoadRegisterMemCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_STORE_REGISTER_MEM_PARAMS   params) = 0;

    //!
    //! \brief    Adds MI_LOAD_REGISTER_IMM to the command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which requested command is added
    //! \param    [in] params
    //!           Parameters used to populate the requested command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiLoadRegisterImmCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_LOAD_REGISTER_IMM_PARAMS    params) = 0;

    //!
    //! \brief    Adds MI_LOAD_REGISTER_REG to the command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which requested command is added
    //! \param    [in] params
    //!           Parameters used to populate the requested command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiLoadRegisterRegCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_LOAD_REGISTER_REG_PARAMS    params) = 0;

    //!
    //! \brief    Adds MI_MATH to the command buffer
    //! \details  MI_MATH is a variable length command requiring the ALU payload
    //!           for completion.
    //! \param    [in] cmdBuffer
    //!           Command buffer to which requested command is added
    //! \param    [in] params
    //!           Parameters used to populate the requested command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiMathCmd(
        PMOS_COMMAND_BUFFER      cmdBuffer,
        PMHW_MI_MATH_PARAMS      params) = 0;

    //!
    //! \brief    Adds MI_SET_PREDICATE to the command buffer
    //!           for completion.
    //! \param    [in] cmdBuffer
    //!           Command buffer to which requested command is added
    //! \param    [in] params
    //!           Parameters used to populate the requested command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiSetPredicateCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        MHW_MI_SET_PREDICATE_ENABLE         enableFlag) = 0;

    //!
    //! \brief    Adds MI_ATOMIC to the command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which requested command is added
    //! \param    [in] params
    //!           Parameters used to populate the requested command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiAtomicCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_ATOMIC_PARAMS               params) = 0;

    //!
    //! \brief    Adds MI_SEMAPHORE_WAIT to the command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which requested command is added
    //! \param    [in] params
    //!           Parameters used to populate the requested command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiSemaphoreWaitCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_MI_SEMAPHORE_WAIT_PARAMS       params) = 0;

    //!
    //! \brief    Adds MI_ARB_CHECK to the command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which requested command is added
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiArbCheckCmd(
        PMOS_COMMAND_BUFFER         cmdBuffer) = 0;

    //!
    //! \brief    Adds PIPE_CONTROL to the command buffer
    //! \details  Although this function is not an MI function, all flushses are part of
    //!           the common MI interface, either the command or batch buffer must be valid
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added, if valid batchBuffer should be null
    //! \param    [in] batchBuffer
    //!           Batch buffer to which HW command is added, if valid cmdBuffer should be null
    //! \param    [in] params
    //!           Parameters used to populate the requested command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddPipeControl(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        PMHW_PIPE_CONTROL_PARAMS        params) = 0;

    //!
    //! \brief    Adds MFX_WAIT_CMD to the buffer provided
    //! \details  Either the command or batch buffer must be valid
    //! \param    [in]  cmdBuffer
    //!           If valid, command buffer to which command are added
    //! \param    [in]  batchBuffer
    //!           If valid, batch buffer to which the command is added
    //! \param    [in] stallVdboxPipeline
    //!           Indicates whether or not the MFX_WAIT will wait on VDBOX pipelines (HuC/HCP/MFX)
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxWaitCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_BATCH_BUFFER                   batchBuffer,
        bool                                stallVdboxPipeline) = 0;

    //!
    //! \brief    Adds MEDIA_STATE_FLUSH to valid buffer provided
    //! \details  Client facing function to add MEDIA_STATE_FLUSH to either the
    //!           command buffer or batch buffer (whichever is valid)
    //! \param    [in] cmdBuffer
    //!           If valid, command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           If valid, Batch buffer to which HW command is added
    //! \param    [in] params
    //!           Parameters used to populate the requested command, may be nullptr if not needed
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMediaStateFlush(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        PMHW_MEDIA_STATE_FLUSH_PARAM    params = nullptr) = 0;

    //!
    //! \brief    Skips batch buffer end command in a batch buffer
    //! \details  Inserts the space equivalent to what would have been inserted during
    //!           AddBatchBufferEnd to the batch buffer provided.
    //! \param    [in] batchBuffer
    //!           Batch buffer which HW command is skipped
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SkipMiBatchBufferEndBb(
        PMHW_BATCH_BUFFER               batchBuffer) = 0;

    //!
    //! \brief    Adds prolog for protected content
    //! \param    [in] cmdBuffer
    //!           Command buffer into which prolog is inserted
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddProtectedProlog(MOS_COMMAND_BUFFER *cmdBuffer);

    //!
    //! \brief    Get mmio registers address
    //! \details  Get mmio registers address
    //! \return   [out] PMHW_MI_MMIOREGISTERS*
    //!           mmio registers got.
    //!
    inline PMHW_MI_MMIOREGISTERS GetMmioRegisters()
    {
        return &m_mmioRegisters;
    }

    //!
    //! \brief    get the size of hw command
    //! \details  Internal function to get the size of MI_FLUSH_DW_CMD
    //! \return   commandSize
    //!           The command size
    //!
    virtual uint32_t GetMiFlushDwCmdSize() = 0;

    //!
    //! \brief    get the size of hw command
    //! \details  Internal function to get the size of MI_BATCH_BUFFER_START_CMD
    //! \return   commandSize
    //!           The command size
    //!
    virtual uint32_t GetMiBatchBufferStartCmdSize() = 0;

    //!
    //! \brief    get the size of hw command
    //! \details  Internal function to get the size of MI_BATCH_BUFFER_END_CMD
    //! \return   commandSize
    //!           The command size
    //!
    virtual uint32_t GetMiBatchBufferEndCmdSize() = 0;

    //!
    //! \brief    Set Watchdog Timer Threshold
    //! \details  Set Watchdog Timer Threshold
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetWatchdogTimerThreshold(uint32_t frameWidth, uint32_t frameHeight, bool isEncoder = true) = 0;

    //!
    //! \brief    Set Watchdog Timer Register Offset
    //! \details  Set Watchdog Timer Register Offset
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetWatchdogTimerRegisterOffset(MOS_GPU_CONTEXT gpuContext) = 0;

    //!
    //! \brief    Add Watchdog Timer Start Cmd
    //! \details  Add Watchdog Timer Start Cmd
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddWatchdogTimerStartCmd(PMOS_COMMAND_BUFFER cmdBuffer) = 0;

    //!
    //! \brief    Add Watchdog Timer Stop Cmd
    //! \details  Add Watchdog Timer Stop Cmd
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddWatchdogTimerStopCmd(PMOS_COMMAND_BUFFER cmdBuffer) = 0;

protected:
    //!
    //! \brief    Initializes the MI interface
    //! \details  Internal MHW function to initialize all function pointers and some parameters
    //!           Assumes that the caller has checked pointer validity and whether or not an
    //!           addressing method has been selected in the OS interface (bUsesGfxAddress or
    //!           bUsesPatchList).
    //! \param    [in] pCpInterface
    //!           CP interface, must be valid
    //! \param    [in] pOsInterface
    //!           OS interface, must be valid
    //!
    MhwMiInterface(
        MhwCpInterface      *cpInterface,
        PMOS_INTERFACE      osInterface);

    //!
    //! \brief    Adds a resource to the command buffer or indirect state (SSH)
    //! \details  Internal MHW function to add either a graphics address of a resource or
    //!           add the resource to the patch list for the requested buffer or state
    //! \param    [in] pOsInterface
    //!           OS interface
    //! \param    [in] cmdBuffer
    //!           If adding a resource to the command buffer, the buffer to which the resource
    //!           is added
    //! \param    [in] params
    //!           Parameters necessary to add the graphics address
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS (*AddResourceToCmd) (
        PMOS_INTERFACE                  pOsInterface,
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_RESOURCE_PARAMS            params) = nullptr;

    //!
    //! \brief    Helper function to get GTT type (PGTT or GGTT)
    //! \return   bool
    //!           true for GGTT, false for PPGTT
    //!
    bool IsGlobalGttInUse();

    MhwCpInterface      *m_cpInterface = nullptr; //!< Responsible for CP functionality
    PMOS_INTERFACE      m_osInterface = nullptr;   //!< Responsible for interaction with OS

    //! \brief Indicates the global GTT setting on each engine.
    struct
    {
        uint8_t m_cs : 1;    //!< GGTT in use for the render engine.
        uint8_t m_vcs : 1;   //!< GGTT in use for VDBOX.
        uint8_t m_vecs : 1;  //!< GGTT in use for VEBOX.
    } UseGlobalGtt;

    //! \brief Indicates the MediaReset Parameter.
    struct
    {
        uint32_t watchdogCountThreshold;
        uint32_t watchdogCountCtrlOffset;
        uint32_t watchdogCountThresholdOffset;
    } MediaResetParam;

    //! \brief Mmio registers address
    MHW_MI_MMIOREGISTERS       m_mmioRegisters = {};  //!< mfx mmio registers

};

#endif // __MHW_MI_H__
