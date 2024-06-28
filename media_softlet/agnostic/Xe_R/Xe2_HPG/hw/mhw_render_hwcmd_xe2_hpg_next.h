
/*===================== begin_copyright_notice ==================================

* Copyright (c) 2024, Intel Corporation
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

======================= end_copyright_notice ==================================*/
//!
//! \file     mhw_render_hwcmd_xe2_hpg_next.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of xe2_hpg as other components
//!           should use MHW interface to interact with MHW commands and states.
//!

// DO NOT EDIT

#ifndef __MHW_RENDER_HWCMD_XE2_HPG_NEXT_H__
#define __MHW_RENDER_HWCMD_XE2_HPG_NEXT_H__

#include "mhw_hwcmd.h"
#pragma once
#pragma pack(1)

#include <cstdint>
#include <cstddef>

namespace mhw
{
namespace render
{
namespace xe2_hpg_next
{
struct Cmd
{
public:
    // Internal Macros
    #define __CODEGEN_MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
    #define __CODEGEN_BITFIELD(l, h) (h) - (l) + 1
    #define __CODEGEN_OP_LENGTH_BIAS 2
    #define __CODEGEN_OP_LENGTH(x) (uint32_t)((__CODEGEN_MAX(x, __CODEGEN_OP_LENGTH_BIAS)) - __CODEGEN_OP_LENGTH_BIAS)

    static uint32_t GetOpLength(uint32_t uiLength) { return __CODEGEN_OP_LENGTH(uiLength); }

    //!
    //! \brief PIPELINE_SELECT
    //! \details
    //!     The PIPELINE_SELECT command is used to specify which GPE pipeline is to
    //!     be considered the 'current' active pipeline. Issuing
    //!     3D-pipeline-specific commands when the Media pipeline is selected, or
    //!     viceversa, is UNDEFINED.
    //!
    //!     Issuing 3D-pipeline-specific commands when the GPGPU pipeline is
    //!     selected, or vice versa, is UNDEFINED.
    //!
    //!     Programming common non pipeline commands (e.g., STATE_BASE_ADDRESS) is
    //!     allowed in all pipeline modes.
    //!
    //!     Software must ensure Render Cache, Depth Cache and Dataport are flushed
    //!     through a stalling PIPE_CONTROL command prior to programming of
    //!     PIPELINE_SELECT command transitioning Pipeline Select from 3D to
    //!     GPGPU/Media. Similarly software must ensure Dataport flush is issued
    //!     through a stalling PIPE_CONTROL command prior to programming of
    //!     PIPELINE_SELECT command transitioning Pipeline Select from GPGPU/Media
    //!     to 3D.
    //!     Example:
    //!
    //!     Workload-3Dmode,
    //!
    //!     PIPE_CONTROL (CS Stall, Depth Cache Flush Enable, Render Target Cache
    //!     Flush Enable, Dataport Flush Enable) ,
    //!
    //!     PIPELINE_SELECT ( GPGPU),
    //!
    //!     Workload-GPGPUmode,
    //!
    //!     PIPE_CONTROL (CS Stall, Dataport Flush Enable),
    //!
    //!     PIPELINE_SELECT ( 3D) ...
    //!
    //!
    //!
    //!     Workaround
    //!
    //!     : This command must be followed by a PIPE_CONTROL with CS Stall bit
    //!     set.,
    //!
    //!     "Pipe Selection" must be never set to "3D" in PIPELINE_SELECT command
    //!     programmed for workloads submitted to ComputeCS.
    //!
    //!     style="margin:0in 0in 0.0001pt">  While GPU is operating in GPGPU mode
    //!     of operation and when a Mid Thread Preemption (if enabled) occurs on a
    //!     PIPELINE_SELECT command with Media Sampler DOP CG Enable reset along
    //!     with Pipeline Select Mode set to 3D and on resubmission of this context
    //!     on context restore Sampler DOP CG Enable will be reset. This would mean
    //!     the GPGPU mid thread preempted threads restored will get executed with
    //!     media sampler DOP clock not gated consuming media sampler DOP power
    //!     until all GPGPU threads have retired. style="margin:0in 0in 0.0001pt" />
    //!      style="margin:0in 0in 0.0001pt">  Programming of the PIPELINE_SELECT
    //!     can be modified to avoid the above inefficiency. This can be done by
    //!     programming Pipeline Selection and Media Sampler DOP CG Enable fields in
    //!     two different PIPELINE_SELECT commands instead of on single
    //!     PIPELINE_SELECTcommand. style="margin:0in 0in 0.0001pt" />
    //!     style="margin:0in 0in 0.0001pt">  Example: style="margin:0in 0in
    //!     0.0001pt">  PIPELINE_SELECT ( Pipeline Selection = 3D, Media Sampler DOP
    //!     CG Enable = False) style="margin:0in 0in 0.0001pt" />  style="margin:0in
    //!     0in 0.0001pt">  To style="margin:0in 0in 0.0001pt">  PIPELINE_SELECT (
    //!     Pipeline Selection = 3D)style="margin:0in 0in 0.0001pt">
    //!     PIPELINE_SELECT (Media Sampler DOP CG Enable = False)
    //!
    struct PIPELINE_SELECT_CMD
    {
        union
        {
            struct
            {
                uint32_t                 PipelineSelection                                : __CODEGEN_BITFIELD( 0,  1)    ; //!< PIPELINE_SELECTION
                uint32_t                 RenderSliceCommonPowerGateEnable                 : __CODEGEN_BITFIELD( 2,  2)    ; //!< RENDER_SLICE_COMMON_POWER_GATE_ENABLE
                uint32_t                 RenderSamplerPowerGateEnable                     : __CODEGEN_BITFIELD( 3,  3)    ; //!< RENDER_SAMPLER_POWER_GATE_ENABLE
                uint32_t                 Reserved4                                        : __CODEGEN_BITFIELD( 4,  4)    ; //!< Reserved
                uint32_t                 EnableComputeTo3DPerformanceMode                 : __CODEGEN_BITFIELD( 5,  5)    ; //!< Enable Compute to 3D performance mode
                uint32_t                 Reserved6                                        : __CODEGEN_BITFIELD( 6,  6)    ; //!< Reserved
                uint32_t                 SystolicModeEnable                               : __CODEGEN_BITFIELD( 7,  7)    ; //!< SYSTOLIC_MODE_ENABLE
                uint32_t                 MaskBits                                         : __CODEGEN_BITFIELD( 8, 15)    ; //!< Mask Bits
                uint32_t                 _3DCommandSubOpcode                              : __CODEGEN_BITFIELD(16, 23)    ; //!< _3D_COMMAND_SUB_OPCODE
                uint32_t                 _3DCommandOpcode                                 : __CODEGEN_BITFIELD(24, 26)    ; //!< _3D_COMMAND_OPCODE
                uint32_t                 CommandSubtype                                   : __CODEGEN_BITFIELD(27, 28)    ; //!< COMMAND_SUBTYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \brief PIPELINE_SELECTION
        //! \details
        //!     Mask bits [9:8] has to be set for HW to look at this field when
        //!     PIPELINE_SELECT command is parsed. Setting only one of the mask bit [9]
        //!     or [8] is illegal.
        enum PIPELINE_SELECTION
        {
            PIPELINE_SELECTION_3D                                            = 0, //!< 3D pipeline is selected
            PIPELINE_SELECTION_GPGPU                                         = 2, //!< GPGPU pipeline is selected
        };

        //! \brief RENDER_SLICE_COMMON_POWER_GATE_ENABLE
        //! \details
        //!     Mask bit [10] has to be set for HW to look at this field when
        //!     PIPELINE_SELECT command is parsed.
        enum RENDER_SLICE_COMMON_POWER_GATE_ENABLE
        {
            RENDER_SLICE_COMMON_POWER_GATE_ENABLE_DISABLED                   = 0, //!< Command Streamer sends message to PM to disable render slice common Power Gating.
            RENDER_SLICE_COMMON_POWER_GATE_ENABLE_ENABLED                    = 1, //!< Command Streamer sends message to PM to enable render slice common Power Gating.
        };

        //! \brief RENDER_SAMPLER_POWER_GATE_ENABLE
        //! \details
        //!     Mask bit [11] has to be set for HW to look at this field when
        //!     PIPELINE_SELECT command is parsed.
        enum RENDER_SAMPLER_POWER_GATE_ENABLE
        {
            RENDER_SAMPLER_POWER_GATE_ENABLE_DISABLED                        = 0, //!< Command Streamer sends message to PM to disable render sampler Power Gating.
            RENDER_SAMPLER_POWER_GATE_ENABLE_ENABLED                         = 1, //!< Command Streamer sends message to PM to enable render sampler Power Gating.
        };

        //! \brief SYSTOLIC_MODE_ENABLE
        //! \details
        //!     When set, this will enable systolic mode for the following
        //!     COMPUTE_WALKER commands. This will lower the Fmax to avoid ICC current
        //!     issues when executing systolic array commands in the execution units. If
        //!     this is not set prior to executing systolic array operations, the
        //!     context will be halted to avoid any ICC issues.
        enum SYSTOLIC_MODE_ENABLE
        {
            SYSTOLIC_MODE_ENABLE_SYSTOLICMODEDISABLED                        = 0, //!< No additional details
            SYSTOLIC_MODE_ENABLE_SYSTOLICMODEENABLED                         = 1, //!< No additional details
        };

        enum _3D_COMMAND_SUB_OPCODE
        {
            _3D_COMMAND_SUB_OPCODE_PIPELINESELECT                            = 4, //!< No additional details
        };

        enum _3D_COMMAND_OPCODE
        {
            _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED                           = 1, //!< No additional details
        };

        enum COMMAND_SUBTYPE
        {
            COMMAND_SUBTYPE_GFXPIPESINGLEDW                                  = 1, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_GFXPIPE                                             = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        PIPELINE_SELECT_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief STATE_BASE_ADDRESS
    //! \details
    //!     The STATE_BASE_ADDRESS command sets the base pointers for subsequent
    //!     state, instruction, and media indirect object accesses by the GPE.For
    //!     more information see the Base Address Utilization table in the Memory
    //!     Access Indirection narrative topic.
    //!
    //!     The following commands must be reissued following any change to the base
    //!     addresses:  3DSTATE_CC_POINTERS
    //!     3DSTATE_BINDING_TABLE_POINTERS
    //!     3DSTATE_SAMPLER_STATE_POINTERS
    //!     3DSTATE_VIEWPORT_STATE_POINTERS
    //!       />  Execution of this command causes a full pipeline flush, thus its
    //!     use should be minimized for higher performance.
    //!
    //!     If 3DSTATE_PS_EXTRA::Pixel Shader Is Per Coarse Pixel == 1, the
    //!     3DSTATE_CPS_POINTERS command must be reissued following any change to
    //!     the dynamic state base address.
    //!
    //!     SW must always program PIPE_CONTROL with "CS Stall" and "Render Target
    //!     Cache Flush Enable" set before programming STATE_BASE_ADDRESS command
    //!     for GPGPU workloads i.e when pipeline select is GPGPU via
    //!     PIPELINE_SELECT command. This is required to achieve better GPGPU
    //!     preemption latencies in certain workload programming sequences. If
    //!     programming PIPE_CONTROL has performance implications then preemption
    //!     latencies can be traded off against performance by not implementing this
    //!     programming note.
    //!
    //!     SW must always program PIPE_CONTROL command with HDC Pipleine FLush set
    //!     prior to programming of STATE_BASE_ADDRESS command for GPGPU/Media
    //!     workloads i.e when pipeline select is GPGPU or Media via PIPELINE_SELECT
    //!     command. This is required to ensure thewrite data out of the prior
    //!     thread group are flushed out prior to the state changes due to the
    //!     programming of STATE_BASE_ADDRESS command take place.
    //!
    struct STATE_BASE_ADDRESS_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 _3DCommandSubOpcode                              : __CODEGEN_BITFIELD(16, 23)    ; //!< _3D_COMMAND_SUB_OPCODE
                uint32_t                 _3DCommandOpcode                                 : __CODEGEN_BITFIELD(24, 26)    ; //!< _3D_COMMAND_OPCODE
                uint32_t                 CommandSubtype                                   : __CODEGEN_BITFIELD(27, 28)    ; //!< COMMAND_SUBTYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint64_t                 GeneralStateBaseAddressModifyEnable              : __CODEGEN_BITFIELD( 0,  0)    ; //!< GENERAL_STATE_BASE_ADDRESS_MODIFY_ENABLE
                uint64_t                 Reserved33                                       : __CODEGEN_BITFIELD( 1,  3)    ; //!< Reserved
                uint64_t                 GeneralStateMemoryObjectControlState             : __CODEGEN_BITFIELD( 4, 10)    ; //!< General State Memory Object Control State
                uint64_t                 Reserved43                                       : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint64_t                 GeneralStateBaseAddress                          : __CODEGEN_BITFIELD(12, 63)    ; //!< General State Base Address
            };
            uint32_t                     Value[2];
        } DW1_2;
        union
        {
            struct
            {
                uint32_t                 CoherencySettingModifyEnable                     : __CODEGEN_BITFIELD( 0,  0)    ; //!< COHERENCY_SETTING_MODIFY_ENABLE
                uint32_t                 Reserved97                                       : __CODEGEN_BITFIELD( 1, 13)    ; //!< Reserved
                uint32_t                 DisableSupportForMultiGpuAtomicsForStatelessAccesses : __CODEGEN_BITFIELD(14, 14)    ; //!< DISABLE_SUPPORT_FOR_MULTI_GPU_ATOMICS_FOR_STATELESS_ACCESSES
                uint32_t                 DisableSupportForMultiGpuPartialWritesForStatelessMessages : __CODEGEN_BITFIELD(15, 15)    ; //!< DISABLE_SUPPORT_FOR_MULTI_GPU_PARTIAL_WRITES_FOR_STATELESS_MESSAGES
                uint32_t                 StatelessDataPortAccessMemoryObjectControlState  : __CODEGEN_BITFIELD(16, 22)    ; //!< Stateless Data Port Access Memory Object Control State
                uint32_t                 L1CacheControl                                   : __CODEGEN_BITFIELD(23, 25)    ; //!< L1 Cache Control
                uint32_t                 Reserved122                                      : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint64_t                 SurfaceStateBaseAddressModifyEnable              : __CODEGEN_BITFIELD( 0,  0)    ; //!< SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE
                uint64_t                 Reserved129                                      : __CODEGEN_BITFIELD( 1,  3)    ; //!< Reserved
                uint64_t                 SurfaceStateMemoryObjectControlState             : __CODEGEN_BITFIELD( 4, 10)    ; //!< Surface State Memory Object Control State
                uint64_t                 Reserved139                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint64_t                 SurfaceStateBaseAddress                          : __CODEGEN_BITFIELD(12, 63)    ; //!< Surface State Base Address
            };
            uint32_t                     Value[2];
        } DW4_5;
        union
        {
            struct
            {
                uint64_t                 DynamicStateBaseAddressModifyEnable              : __CODEGEN_BITFIELD( 0,  0)    ; //!< DYNAMIC_STATE_BASE_ADDRESS_MODIFY_ENABLE
                uint64_t                 Reserved193                                      : __CODEGEN_BITFIELD( 1,  3)    ; //!< Reserved
                uint64_t                 DynamicStateMemoryObjectControlState             : __CODEGEN_BITFIELD( 4, 10)    ; //!< Dynamic State Memory Object Control State
                uint64_t                 Reserved203                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint64_t                 DynamicStateBaseAddress                          : __CODEGEN_BITFIELD(12, 63)    ; //!< Dynamic State Base Address
            };
            uint32_t                     Value[2];
        } DW6_7;
        union
        {
            struct
            {
                uint64_t                 IndirectObjectBaseAddressModifyEnable            : __CODEGEN_BITFIELD( 0,  0)    ; //!< INDIRECT_OBJECT_BASE_ADDRESS_MODIFY_ENABLE
                uint64_t                 Reserved257                                      : __CODEGEN_BITFIELD( 1,  3)    ; //!< Reserved
                uint64_t                 IndirectObjectMemoryObjectControlState           : __CODEGEN_BITFIELD( 4, 10)    ; //!< Indirect Object Memory Object Control State
                uint64_t                 Reserved267                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint64_t                 IndirectObjectBaseAddress                        : __CODEGEN_BITFIELD(12, 63)    ; //!< Indirect Object Base Address
            };
            uint32_t                     Value[2];
        } DW8_9;
        union
        {
            struct
            {
                uint64_t                 InstructionBaseAddressModifyEnable               : __CODEGEN_BITFIELD( 0,  0)    ; //!< INSTRUCTION_BASE_ADDRESS_MODIFY_ENABLE
                uint64_t                 Reserved321                                      : __CODEGEN_BITFIELD( 1,  3)    ; //!< Reserved
                uint64_t                 InstructionMemoryObjectControlState              : __CODEGEN_BITFIELD( 4, 10)    ; //!< Instruction Memory Object Control State
                uint64_t                 Reserved331                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint64_t                 InstructionBaseAddress                           : __CODEGEN_BITFIELD(12, 63)    ; //!< Instruction Base Address
            };
            uint32_t                     Value[2];
        } DW10_11;
        union
        {
            struct
            {
                uint32_t                 GeneralStateBufferSizeModifyEnable               : __CODEGEN_BITFIELD( 0,  0)    ; //!< GENERAL_STATE_BUFFER_SIZE_MODIFY_ENABLE
                uint32_t                 Reserved385                                      : __CODEGEN_BITFIELD( 1, 11)    ; //!< Reserved
                uint32_t                 GeneralStateBufferSize                           : __CODEGEN_BITFIELD(12, 31)    ; //!< General State Buffer Size
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            struct
            {
                uint32_t                 DynamicStateBufferSizeModifyEnable               : __CODEGEN_BITFIELD( 0,  0)    ; //!< DYNAMIC_STATE_BUFFER_SIZE_MODIFY_ENABLE
                uint32_t                 Reserved417                                      : __CODEGEN_BITFIELD( 1, 11)    ; //!< Reserved
                uint32_t                 DynamicStateBufferSize                           : __CODEGEN_BITFIELD(12, 31)    ; //!< Dynamic State Buffer Size
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            struct
            {
                uint32_t                 IndirectObjectBufferSizeModifyEnable             : __CODEGEN_BITFIELD( 0,  0)    ; //!< INDIRECT_OBJECT_BUFFER_SIZE_MODIFY_ENABLE
                uint32_t                 Reserved449                                      : __CODEGEN_BITFIELD( 1, 11)    ; //!< Reserved
                uint32_t                 IndirectObjectBufferSize                         : __CODEGEN_BITFIELD(12, 31)    ; //!< Indirect Object Buffer Size
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            struct
            {
                uint32_t                 InstructionBufferSizeModifyEnable                : __CODEGEN_BITFIELD( 0,  0)    ; //!< INSTRUCTION_BUFFER_SIZE_MODIFY_ENABLE
                uint32_t                 Reserved481                                      : __CODEGEN_BITFIELD( 1, 11)    ; //!< Reserved
                uint32_t                 InstructionBufferSize                            : __CODEGEN_BITFIELD(12, 31)    ; //!< Instruction Buffer Size
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            struct
            {
                uint64_t                 BindlessSurfaceStateBaseAddressModifyEnable      : __CODEGEN_BITFIELD( 0,  0)    ; //!< BINDLESS_SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE
                uint64_t                 Reserved513                                      : __CODEGEN_BITFIELD( 1,  3)    ; //!< Reserved
                uint64_t                 BindlessSurfaceStateMemoryObjectControlState     : __CODEGEN_BITFIELD( 4, 10)    ; //!< Bindless Surface State Memory Object Control State
                uint64_t                 Reserved523                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint64_t                 BindlessSurfaceStateBaseAddress                  : __CODEGEN_BITFIELD(12, 63)    ; //!< Bindless Surface State Base Address
            };
            uint32_t                     Value[2];
        } DW16_17;
        union
        {
            struct
            {
                uint32_t                 BindlessSurfaceStateSize                                                         ; //!< Bindless Surface State Size
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            struct
            {
                uint64_t                 BindlessSamplerStateBaseAddressModifyEnable      : __CODEGEN_BITFIELD( 0,  0)    ; //!< BINDLESS_SAMPLER_STATE_BASE_ADDRESS_MODIFY_ENABLE
                uint64_t                 Reserved609                                      : __CODEGEN_BITFIELD( 1,  3)    ; //!< Reserved
                uint64_t                 BindlessSamplerStateMemoryObjectControlState     : __CODEGEN_BITFIELD( 4, 10)    ; //!< Bindless Sampler State Memory Object Control State
                uint64_t                 Reserved619                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint64_t                 BindlessSamplerStateBaseAddress                  : __CODEGEN_BITFIELD(12, 63)    ; //!< Bindless Sampler State Base Address
            };
            uint32_t                     Value[2];
        } DW19_20;
        union
        {
            struct
            {
                uint32_t                 Reserved672                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 BindlessSamplerStateBufferSize                   : __CODEGEN_BITFIELD(12, 31)    ; //!< Bindless Sampler State Buffer Size
            };
            uint32_t                     Value;
        } DW21;

        //! \name Local enumerations

        enum _3D_COMMAND_SUB_OPCODE
        {
            _3D_COMMAND_SUB_OPCODE_STATEBASEADDRESS                          = 1, //!< No additional details
        };

        enum _3D_COMMAND_OPCODE
        {
            _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED                           = 1, //!< No additional details
        };

        enum COMMAND_SUBTYPE
        {
            COMMAND_SUBTYPE_GFXPIPECOMMON                                    = 0, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_GFXPIPE                                             = 3, //!< No additional details
        };

        //! \brief GENERAL_STATE_BASE_ADDRESS_MODIFY_ENABLE
        //! \details
        //!     The other fields in this DWord and the following DWord are updated
        //!     only when this bit is set.
        enum GENERAL_STATE_BASE_ADDRESS_MODIFY_ENABLE
        {
            GENERAL_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE                 = 0, //!< Ignore the updated address.
            GENERAL_STATE_BASE_ADDRESS_MODIFY_ENABLE_ENABLE                  = 1, //!< Modify the address.
        };

        //! \brief COHERENCY_SETTING_MODIFY_ENABLE
        //! \details
        //!     All the fields in this DW is only updated when this bit is set.
        enum COHERENCY_SETTING_MODIFY_ENABLE
        {
            COHERENCY_SETTING_MODIFY_ENABLE_DISABLEWRITETOTHISDW             = 0, //!< No additional details
            COHERENCY_SETTING_MODIFY_ENABLE_ENABLEWRITETOTHISDW              = 1, //!< No additional details
        };

        //! \brief DISABLE_SUPPORT_FOR_MULTI_GPU_ATOMICS_FOR_STATELESS_ACCESSES
        //! \details
        //!     Specifies whether sequential consistency of atomic memory operations are
        //!     supported across multiple GPUs.
        enum DISABLE_SUPPORT_FOR_MULTI_GPU_ATOMICS_FOR_STATELESS_ACCESSES
        {
            DISABLE_SUPPORT_FOR_MULTI_GPU_ATOMICS_FOR_STATELESS_ACCESSES_ENABLE = 0, //!< Atomic memory operations from all GPUs to the same address is sequentially consistent.
            DISABLE_SUPPORT_FOR_MULTI_GPU_ATOMICS_FOR_STATELESS_ACCESSES_DISABLE = 1, //!< Disable multi-GPU Atomic consistency. Atomic memory operations to the same address is sequentially consistent only if the operations are from the same GPU.
        };

        //! \brief DISABLE_SUPPORT_FOR_MULTI_GPU_PARTIAL_WRITES_FOR_STATELESS_MESSAGES
        //! \details
        //!     Specifies whether data-consistency on partial memory write operations
        //!     are supported across multiple GPUs.
        enum DISABLE_SUPPORT_FOR_MULTI_GPU_PARTIAL_WRITES_FOR_STATELESS_MESSAGES
        {
            DISABLE_SUPPORT_FOR_MULTI_GPU_PARTIAL_WRITES_FOR_STATELESS_MESSAGES_ENABLED = 0, //!< Enable data consistency on multi-GPU partial memory writes.
            DISABLE_SUPPORT_FOR_MULTI_GPU_PARTIAL_WRITES_FOR_STATELESS_MESSAGES_DISABLED = 1, //!< Disable data consistency on multi-GPU partial memory writes. If multiple GPUs write different bytes of the same cacheline, the data may be corrupted.
        };

        //! \brief SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE
        //! \details
        //!     The other fields in this DWord and the following DWord are updated only
        //!     when this bit is set.
        enum SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE
        {
            SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE                 = 0, //!< Ignore the updated address.
            SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE_ENABLE                  = 1, //!< Modify the address.
        };

        //! \brief DYNAMIC_STATE_BASE_ADDRESS_MODIFY_ENABLE
        //! \details
        //!     The other fields in this DWord and the following DWord are updated only
        //!     when this bit is set.
        enum DYNAMIC_STATE_BASE_ADDRESS_MODIFY_ENABLE
        {
            DYNAMIC_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE                 = 0, //!< Ignore the updated address.
            DYNAMIC_STATE_BASE_ADDRESS_MODIFY_ENABLE_ENABLE                  = 1, //!< Modify the address.
        };

        //! \brief INDIRECT_OBJECT_BASE_ADDRESS_MODIFY_ENABLE
        //! \details
        //!     The other fields in this DWord and the following DWord are updated
        //!     only when this bit is set.
        enum INDIRECT_OBJECT_BASE_ADDRESS_MODIFY_ENABLE
        {
            INDIRECT_OBJECT_BASE_ADDRESS_MODIFY_ENABLE_DISABLE               = 0, //!< Ignore the updated address.
            INDIRECT_OBJECT_BASE_ADDRESS_MODIFY_ENABLE_ENABLE                = 1, //!< Modify the address.
        };

        //! \brief INSTRUCTION_BASE_ADDRESS_MODIFY_ENABLE
        //! \details
        //!     The other fields in this DWord and the following DWord are updated
        //!     only when this bit is set.
        enum INSTRUCTION_BASE_ADDRESS_MODIFY_ENABLE
        {
            INSTRUCTION_BASE_ADDRESS_MODIFY_ENABLE_DISABLE                   = 0, //!< Ignore the updated address.
            INSTRUCTION_BASE_ADDRESS_MODIFY_ENABLE_ENABLE                    = 1, //!< Modify the address.
        };

        //! \brief GENERAL_STATE_BUFFER_SIZE_MODIFY_ENABLE
        //! \details
        //!     The fields in this DWord are updated only when this bit is set.
        enum GENERAL_STATE_BUFFER_SIZE_MODIFY_ENABLE
        {
            GENERAL_STATE_BUFFER_SIZE_MODIFY_ENABLE_DISABLE                  = 0, //!< Ignore the updated bound.
            GENERAL_STATE_BUFFER_SIZE_MODIFY_ENABLE_ENABLE                   = 1, //!< Modify the updated bound.
        };

        //! \brief DYNAMIC_STATE_BUFFER_SIZE_MODIFY_ENABLE
        //! \details
        //!     FormatDesc
        enum DYNAMIC_STATE_BUFFER_SIZE_MODIFY_ENABLE
        {
            DYNAMIC_STATE_BUFFER_SIZE_MODIFY_ENABLE_DISABLE                  = 0, //!< Ignore the updated bound.
            DYNAMIC_STATE_BUFFER_SIZE_MODIFY_ENABLE_ENABLE                   = 1, //!< Modify the updated bound.
        };

        //! \brief INDIRECT_OBJECT_BUFFER_SIZE_MODIFY_ENABLE
        //! \details
        //!     FormatDesc
        enum INDIRECT_OBJECT_BUFFER_SIZE_MODIFY_ENABLE
        {
            INDIRECT_OBJECT_BUFFER_SIZE_MODIFY_ENABLE_DISABLE                = 0, //!< Ignore the updated bound.
            INDIRECT_OBJECT_BUFFER_SIZE_MODIFY_ENABLE_ENABLE                 = 1, //!< Modify the updated bound.
        };

        //! \brief INSTRUCTION_BUFFER_SIZE_MODIFY_ENABLE
        //! \details
        //!     FormatDesc
        enum INSTRUCTION_BUFFER_SIZE_MODIFY_ENABLE
        {
            INSTRUCTION_BUFFER_SIZE_MODIFY_ENABLE_DISABLE                    = 0, //!< Ignore the updated bound.
        };

        //! \brief BINDLESS_SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE
        //! \details
        //!     The other fields in this DWord and the following two DWords are
        //!     updated only when this bit is set.
        enum BINDLESS_SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE
        {
            BINDLESS_SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE        = 0, //!< Ignore the updated address
            BINDLESS_SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE_ENABLE         = 1, //!< Modify the address
        };

        //! \brief BINDLESS_SAMPLER_STATE_BASE_ADDRESS_MODIFY_ENABLE
        //! \details
        //!     The other fields in this DWord and the following two DWords are
        //!     updated only when this bit is set.
        enum BINDLESS_SAMPLER_STATE_BASE_ADDRESS_MODIFY_ENABLE
        {
            BINDLESS_SAMPLER_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE        = 0, //!< Ignore the updated address
            BINDLESS_SAMPLER_STATE_BASE_ADDRESS_MODIFY_ENABLE_ENABLE         = 1, //!< Modify the address
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        STATE_BASE_ADDRESS_CMD();

        static const size_t dwSize = 22;
        static const size_t byteSize = 88;
    };

    //!
    //! \brief _3DSTATE_CHROMA_KEY
    //! \details
    //!     The 3DSTATE_CHROMA_KEY instruction is used to program texture
    //!     color/chroma-key key values. A table containing four set of values is
    //!     supported. The ChromaKey Index sampler state variable is used to select
    //!     which table entry is associated with the map. Texture chromakey
    //!     functions are enabled and controlled via use of the ChromaKey Enable
    //!     texture sampler state variable.Texture Color Key (keying on a paletted
    //!     texture index) is not supported.
    //!
    //!     Workaround
    //!
    //!     : This command must be followed by a PIPE_CONTROL with CS Stall bit
    //!     set.,
    //!
    struct _3DSTATE_CHROMA_KEY_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 _3DCommandSubOpcode                              : __CODEGEN_BITFIELD(16, 23)    ; //!< _3D_COMMAND_SUB_OPCODE
                uint32_t                 _3DCommandOpcode                                 : __CODEGEN_BITFIELD(24, 26)    ; //!< _3D_COMMAND_OPCODE
                uint32_t                 CommandSubtype                                   : __CODEGEN_BITFIELD(27, 28)    ; //!< COMMAND_SUBTYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0, 29)    ; //!< Reserved
                uint32_t                 ChromakeyTableIndex                              : __CODEGEN_BITFIELD(30, 31)    ; //!< ChromaKey Table Index
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 ChromakeyLowValue                                                                ; //!< ChromaKey Low Value
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 ChromakeyHighValue                                                               ; //!< ChromaKey High Value
            };
            uint32_t                     Value;
        } DW3;

        //! \name Local enumerations

        enum _3D_COMMAND_SUB_OPCODE
        {
            _3D_COMMAND_SUB_OPCODE_3DSTATECHROMAKEY                          = 4, //!< No additional details
        };

        enum _3D_COMMAND_OPCODE
        {
            _3D_COMMAND_OPCODE_3DSTATENONPIPELINED                           = 1, //!< No additional details
        };

        enum COMMAND_SUBTYPE
        {
            COMMAND_SUBTYPE_GFXPIPE3D                                        = 3, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_GFXPIPE                                             = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        _3DSTATE_CHROMA_KEY_CMD();

        static const size_t dwSize = 4;
        static const size_t byteSize = 16;
    };

    //!
    //! \brief STATE_SIP
    //! \details
    //!     The STATE_SIP command specifies the starting instruction location of the
    //!     System Routine that is shared by all threads in execution.
    //!
    //!     Workaround
    //!
    //!     : This command must be followed by a PIPE_CONTROL with CS Stall bit
    //!     set.,
    //!
    struct STATE_SIP_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 _3DCommandSubOpcode                              : __CODEGEN_BITFIELD(16, 23)    ; //!< _3D_COMMAND_SUB_OPCODE
                uint32_t                 _3DCommandOpcode                                 : __CODEGEN_BITFIELD(24, 26)    ; //!< _3D_COMMAND_OPCODE
                uint32_t                 CommandSubtype                                   : __CODEGEN_BITFIELD(27, 28)    ; //!< COMMAND_SUBTYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint64_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0,  3)    ; //!< Reserved
                uint64_t                 SystemInstructionPointer                         : __CODEGEN_BITFIELD( 4, 63)    ; //!< System Instruction Pointer
            };
            uint32_t                     Value[2];
        } DW1_2;

        //! \name Local enumerations

        enum _3D_COMMAND_SUB_OPCODE
        {
            _3D_COMMAND_SUB_OPCODE_STATESIP                                  = 2, //!< No additional details
        };

        enum _3D_COMMAND_OPCODE
        {
            _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED                           = 1, //!< No additional details
        };

        enum COMMAND_SUBTYPE
        {
            COMMAND_SUBTYPE_GFXPIPECOMMON                                    = 0, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_GFXPIPE                                             = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        STATE_SIP_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief _3DSTATE_BINDING_TABLE_POOL_ALLOC
    //! \details
    //!     This command is to program the base address and size of the binding
    //!     table pool. The address to fetch the binding table is based on the
    //!     Binding Table Pool Base Address and the binding table pointer if the
    //!     Binding Table Pool is enabled. Otherwise the binding table pointer is an
    //!     offset from the Surface Base Address.
    //!
    struct _3DSTATE_BINDING_TABLE_POOL_ALLOC_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 _3DCommandSubOpcode                              : __CODEGEN_BITFIELD(16, 23)    ; //!< _3D_COMMAND_SUB_OPCODE
                uint32_t                 _3DCommandOpcode                                 : __CODEGEN_BITFIELD(24, 26)    ; //!< _3D_COMMAND_OPCODE
                uint32_t                 CommandSubtype                                   : __CODEGEN_BITFIELD(27, 28)    ; //!< COMMAND_SUBTYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint64_t                 SurfaceObjectControlState                        : __CODEGEN_BITFIELD( 0,  6)    ; //!< Surface Object Control State
                uint64_t                 Reserved39                                       : __CODEGEN_BITFIELD( 7, 11)    ; //!< Reserved
                uint64_t                 BindingTablePoolBaseAddress                      : __CODEGEN_BITFIELD(12, 63)    ; //!< Binding Table Pool Base Address
            };
            uint32_t                     Value[2];
        } DW1_2;
        union
        {
            struct
            {
                uint32_t                 Reserved96                                       : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 BindingTablePoolBufferSize                       : __CODEGEN_BITFIELD(12, 31)    ; //!< BINDING_TABLE_POOL_BUFFER_SIZE
            };
            uint32_t                     Value;
        } DW3;

        //! \name Local enumerations

        enum _3D_COMMAND_SUB_OPCODE
        {
            _3D_COMMAND_SUB_OPCODE_3DSTATEBINDINGTABLEPOOLALLOC              = 25, //!< No additional details
        };

        enum _3D_COMMAND_OPCODE
        {
            _3D_COMMAND_OPCODE_3DSTATENONPIPELINED                           = 1, //!< No additional details
        };

        enum COMMAND_SUBTYPE
        {
            COMMAND_SUBTYPE_GFXPIPE3D                                        = 3, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_GFXPIPE                                             = 3, //!< No additional details
        };

        //! \brief BINDING_TABLE_POOL_BUFFER_SIZE
        //! \details
        //!     This field specifies the size of the buffer in 4K pages. Any access
        //!     which straddle or go past the end of the buffer will return 0.
        enum BINDING_TABLE_POOL_BUFFER_SIZE
        {
            BINDING_TABLE_POOL_BUFFER_SIZE_NOVALIDDATA                       = 0, //!< There is no valid data in the buffer
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        _3DSTATE_BINDING_TABLE_POOL_ALLOC_CMD();

        static const size_t dwSize = 4;
        static const size_t byteSize = 16;
    };

    //!
    //! \brief COMPUTE_WALKER
    //! \details
    //!     COMPUTE_WALKER spawns threadgroups in 1, 2, or 3 dimensions (X, Y, Z).
    //!     Each threadgroup is described by Interface Descriptor in this command.
    //!       Each dispatched thread has a standard payload delivered in R0,
    //!     including the Indirect Address to fetch the thread's parameters.
    //!       After the Walker completes dispatching its threads and those threads
    //!     have completed running, a PostSync operation can write a completion code
    //!     or a timestamp.
    //!
    //!     If the threads spawned by this command are required to observe memory
    //!     writes performed by threads spawned from a previous command, and if
    //!     those threads did not perform a Memory Fence before they exited, then
    //!     software must precede this command with a PIPE_CONTROL with Dataport
    //!     Flush control.
    //!
    struct COMPUTE_WALKER_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 PredicateEnable                                  : __CODEGEN_BITFIELD( 8,  8)    ; //!< Predicate Enable
                uint32_t                 WorkloadPartitionEnable                          : __CODEGEN_BITFIELD( 9,  9)    ; //!< Workload Partition Enable
                uint32_t                 IndirectParameterEnable                          : __CODEGEN_BITFIELD(10, 10)    ; //!< Indirect Parameter Enable
                uint32_t                 Reserved11                                       : __CODEGEN_BITFIELD(11, 12)    ; //!< Reserved
                uint32_t                 DispatchComplete                                 : __CODEGEN_BITFIELD(13, 13)    ; //!< Dispatch Complete
                uint32_t                 SystolicModeEnable                               : __CODEGEN_BITFIELD(14, 14)    ; //!< Systolic Mode Enable
                uint32_t                 CfeSubopcodeVariant                              : __CODEGEN_BITFIELD(15, 17)    ; //!< CFE_SUBOPCODE_VARIANT
                uint32_t                 CfeSubopcode                                     : __CODEGEN_BITFIELD(18, 23)    ; //!< CFE_SUBOPCODE
                uint32_t                 ComputeCommandOpcode                             : __CODEGEN_BITFIELD(24, 26)    ; //!< COMPUTE_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Reserved
                uint32_t                 DebugObjectId                                    : __CODEGEN_BITFIELD( 8, 31)    ; //!< Debug: Object ID
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 IndirectDataLength                               : __CODEGEN_BITFIELD( 0, 16)    ; //!< Indirect Data Length
                uint32_t                 L3PrefetchDisable                                : __CODEGEN_BITFIELD(17, 17)    ; //!< L3 prefetch disable
                uint32_t                 PartitionDispatchParameter                       : __CODEGEN_BITFIELD(18, 29)    ; //!< Partition Dispatch Parameter
                uint32_t                 PartitionType                                    : __CODEGEN_BITFIELD(30, 31)    ; //!< PARTITION_TYPE
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 Reserved96                                       : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 IndirectDataStartAddress                         : __CODEGEN_BITFIELD( 6, 31)    ; //!< Indirect Data Start Address
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 Reserved128                                      : __CODEGEN_BITFIELD( 0, 16)    ; //!< Reserved
                uint32_t                 MessageSIMD                                      : __CODEGEN_BITFIELD(17, 18)    ; //!< MESSAGE_SIMD
                uint32_t                 TileLayout                                       : __CODEGEN_BITFIELD(19, 21)    ; //!< TILE_LAYOUT
                uint32_t                 WalkOrder                                        : __CODEGEN_BITFIELD(22, 24)    ; //!< WALK_ORDER
                uint32_t                 EmitInlineParameter                              : __CODEGEN_BITFIELD(25, 25)    ; //!< Emit Inline Parameter
                uint32_t                 EmitLocal                                        : __CODEGEN_BITFIELD(26, 28)    ; //!< EMIT_LOCAL
                uint32_t                 GenerateLocalId                                  : __CODEGEN_BITFIELD(29, 29)    ; //!< Generate Local ID
                uint32_t                 SIMDSize                                         : __CODEGEN_BITFIELD(30, 31)    ; //!< SIMD_SIZE
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 ExecutionMask                                                                    ; //!< Execution Mask
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 LocalXMaximum                                    : __CODEGEN_BITFIELD( 0,  9)    ; //!< Local X Maximum
                uint32_t                 LocalYMaximum                                    : __CODEGEN_BITFIELD(10, 19)    ; //!< Local Y Maximum
                uint32_t                 LocalZMaximum                                    : __CODEGEN_BITFIELD(20, 29)    ; //!< Local Z Maximum
                uint32_t                 Reserved222                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            struct
            {
                uint32_t                 ThreadGroupIDXDimension                                                          ; //!< Thread Group ID X Dimension
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            struct
            {
                uint32_t                 ThreadGroupIDYDimension                                                          ; //!< Thread Group ID Y Dimension
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            struct
            {
                uint32_t                 ThreadGroupIDZDimension                                                          ; //!< Thread Group ID Z Dimension
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            struct
            {
                uint32_t                 ThreadGroupIDStartingX                                                           ; //!< Thread Group ID Starting X
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            struct
            {
                uint32_t                 ThreadGroupIDStartingY                                                           ; //!< Thread Group ID Starting Y
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            struct
            {
                uint32_t                 ThreadGroupIDStartingZ                                                           ; //!< Thread Group ID Starting Z
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            struct
            {
                uint32_t                 PartitionId                                                                      ; //!< Partition ID
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            struct
            {
                uint32_t                 PartitionSize                                                                    ; //!< Partition Size
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            struct
            {
                uint32_t                 PreemptX                                                                         ; //!< Preempt X
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            struct
            {
                uint32_t                 PreemptY                                                                         ; //!< Preempt Y
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            struct
            {
                uint32_t                 PreemptZ                                                                         ; //!< Preempt Z
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            struct
            {
                uint32_t                 WalkerId                                         : __CODEGEN_BITFIELD( 0,  3)    ; //!< Walker ID
                uint32_t                 Reserved580                                      : __CODEGEN_BITFIELD( 4,  7)    ; //!< Reserved
                uint32_t                 OverDispatchTgCount                              : __CODEGEN_BITFIELD( 8, 23)    ; //!< Over dispatch TG count
                uint32_t                 Reserved600                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW18;

        //!
        //! \brief INTERFACE_DESCRIPTOR_DATA
        //! \details
        //!
        //!
        struct INTERFACE_DESCRIPTOR_DATA_CMD
        {
            union
            {
                struct
                {
                    uint32_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                    uint32_t                 KernelStartPointer                               : __CODEGEN_BITFIELD( 6, 31)    ; //!< Kernel Start Pointer
                };
                uint32_t                     Value;
            } DW0;
            union
            {
                struct
                {
                    uint32_t                 Reserved32                                                                       ; //!< Reserved
                };
                uint32_t                     Value;
            } DW1;
            union
            {
                struct
                {
                    uint32_t                 Reserved64                                       : __CODEGEN_BITFIELD( 0,  6)    ; //!< Reserved
                    uint32_t                 SoftwareExceptionEnable                          : __CODEGEN_BITFIELD( 7,  7)    ; //!< Software Exception Enable
                    uint32_t                 Reserved72                                       : __CODEGEN_BITFIELD( 8, 10)    ; //!< Reserved
                    uint32_t                 MaskStackExceptionEnable                         : __CODEGEN_BITFIELD(11, 11)    ; //!< Mask Stack Exception Enable
                    uint32_t                 Reserved76                                       : __CODEGEN_BITFIELD(12, 12)    ; //!< Reserved
                    uint32_t                 IllegalOpcodeExceptionEnable                     : __CODEGEN_BITFIELD(13, 13)    ; //!< Illegal Opcode Exception Enable
                    uint32_t                 Reserved78                                       : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                    uint32_t                 FloatingPointMode                                : __CODEGEN_BITFIELD(16, 16)    ; //!< FLOATING_POINT_MODE
                    uint32_t                 Reserved81                                       : __CODEGEN_BITFIELD(17, 17)    ; //!< Reserved
                    uint32_t                 SingleProgramFlow                                : __CODEGEN_BITFIELD(18, 18)    ; //!< SINGLE_PROGRAM_FLOW
                    uint32_t                 DenormMode                                       : __CODEGEN_BITFIELD(19, 19)    ; //!< DENORM_MODE
                    uint32_t                 ThreadPreemption                                 : __CODEGEN_BITFIELD(20, 20)    ; //!< THREAD_PREEMPTION
                    uint32_t                 Reserved85                                       : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
                };
                uint32_t                     Value;
            } DW2;
            union
            {
                struct
                {
                    uint32_t                 Reserved96                                       : __CODEGEN_BITFIELD( 0,  1)    ; //!< Reserved
                    uint32_t                 SamplerCount                                     : __CODEGEN_BITFIELD( 2,  4)    ; //!< SAMPLER_COUNT
                    uint32_t                 SamplerStatePointer                              : __CODEGEN_BITFIELD( 5, 31)    ; //!< Sampler State Pointer
                };
                uint32_t                     Value;
            } DW3;
            union
            {
                struct
                {
                    uint32_t                 BindingTableEntryCount                           : __CODEGEN_BITFIELD( 0,  4)    ; //!< BINDING_TABLE_ENTRY_COUNT
                    uint32_t                 BindingTablePointer                              : __CODEGEN_BITFIELD( 5, 20)    ; //!< Binding Table Pointer
                    uint32_t                 Reserved149                                      : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
                };
                uint32_t                     Value;
            } DW4;
            union
            {
                struct
                {
                    uint32_t                 NumberOfThreadsInGpgpuThreadGroup                : __CODEGEN_BITFIELD( 0,  9)    ; //!< Number of Threads in GPGPU Thread Group
                    uint32_t                 Reserved170                                      : __CODEGEN_BITFIELD(10, 12)    ; //!< Reserved
                    uint32_t                 ThreadGroupForwardProgressGuarantee              : __CODEGEN_BITFIELD(13, 13)    ; //!< THREAD_GROUP_FORWARD_PROGRESS_GUARANTEE
                    uint32_t                 Reserved174                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                    uint32_t                 SharedLocalMemorySize                            : __CODEGEN_BITFIELD(16, 20)    ; //!< SHARED_LOCAL_MEMORY_SIZE
                    uint32_t                 Reserved181                                      : __CODEGEN_BITFIELD(21, 21)    ; //!< Reserved
                    uint32_t                 RoundingMode                                     : __CODEGEN_BITFIELD(22, 23)    ; //!< ROUNDING_MODE
                    uint32_t                 Reserved184                                      : __CODEGEN_BITFIELD(24, 25)    ; //!< Reserved
                    uint32_t                 ThreadGroupDispatchSize                          : __CODEGEN_BITFIELD(26, 27)    ; //!< THREAD_GROUP_DISPATCH_SIZE
                    uint32_t                 NumberOfBarriers                                 : __CODEGEN_BITFIELD(28, 30)    ; //!< NUMBER_OF_BARRIERS
                    uint32_t                 BtdMode                                          : __CODEGEN_BITFIELD(31, 31)    ; //!< BTD_MODE
                };
                uint32_t                     Value;
            } DW5;
            union
            {
                struct
                {
                    uint32_t                 Reserved192                                                                      ; //!< Reserved
                };
                uint32_t                     Value;
            } DW6;
            union
            {
                struct
                {
                    uint32_t                 PreferredSlmAllocationSizePerSubslice            : __CODEGEN_BITFIELD( 0,  3)    ; //!< PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE
                    uint32_t                 Reserved228                                      : __CODEGEN_BITFIELD( 4, 31)    ; //!< Reserved
                };
                uint32_t                     Value;
            } DW7;

            //! \name Local enumerations

            //! \brief FLOATING_POINT_MODE
            //! \details
            //!     Specifies the floating point mode used by the dispatched thread.
            enum FLOATING_POINT_MODE
            {
                FLOATING_POINT_MODE_IEEE_754                                     = 0, //!< No additional details
                FLOATING_POINT_MODE_ALTERNATE                                    = 1, //!< No additional details
            };

            //! \brief SINGLE_PROGRAM_FLOW
            //! \details
            //!     Specifies whether the kernel program has a single program flow (SIMDnxm
            //!     with m = 1) or multiple program flows (SIMDnxm with m > 1).
            enum SINGLE_PROGRAM_FLOW
            {
                SINGLE_PROGRAM_FLOW_MULTIPLE                                     = 0, //!< No additional details
                SINGLE_PROGRAM_FLOW_SINGLE                                       = 1, //!< No additional details
            };

            //! \brief DENORM_MODE
            //! \details
            //!     This field specifies how Float denormalized numbers are handles in the
            //!     dispatched thread.
            enum DENORM_MODE
            {
                DENORM_MODE_FTZ                                                  = 0, //!< Float denorms will be flushed to zero when appearing as inputs; denorms will never come out of instructions. Double precision float and half precision float numbers are not flushed to zero.
                DENORM_MODE_SETBYKERNEL                                          = 1, //!< Denorms will be handled in by kernel.
            };

            //! \brief THREAD_PREEMPTION
            //! \details
            //!     This field specifies whether, when dispatched, the thread is allowed to
            //!     stop in middle on receiving mid-thread pre-emption request.
            enum THREAD_PREEMPTION
            {
                THREAD_PREEMPTION_DISABLE                                        = 0, //!< Thread is pre-empted only in case of page-fault.
                THREAD_PREEMPTION_ENABLE                                         = 1, //!< Thread is pre-empted on receiving pre-emption indication.
            };

            //! \brief SAMPLER_COUNT
            //! \details
            //!     Specifies how many samplers (in multiples of 4) the kernel uses. Used
            //!     only for prefetching the associated sampler state entries.
            enum SAMPLER_COUNT
            {
                SAMPLER_COUNT_NOSAMPLERSUSED                                     = 0, //!< No additional details
                SAMPLER_COUNT_BETWEEN1AND4SAMPLERSUSED                           = 1, //!< No additional details
                SAMPLER_COUNT_BETWEEN5AND8SAMPLERSUSED                           = 2, //!< No additional details
                SAMPLER_COUNT_BETWEEN9AND12SAMPLERSUSED                          = 3, //!< No additional details
                SAMPLER_COUNT_BETWEEN13AND16SAMPLERSUSED                         = 4, //!< No additional details
            };

            //! \brief BINDING_TABLE_ENTRY_COUNT
            //! \details
            //!     Specifies how many binding table entries the kernel uses. Used only for
            //!     prefetching of the binding table entries and associated surface state.
            enum BINDING_TABLE_ENTRY_COUNT
            {
                BINDING_TABLE_ENTRY_COUNT_PREFETCHDISABLED                       = 0, //!< No additional details
            };

            //! \brief THREAD_GROUP_FORWARD_PROGRESS_GUARANTEE
            //! \details
            //!     Set by the kernel if TG requires synchronization at memory. If this bit
            //!     is set, HW must enable the barrier when mid thread preemption is
            //!     enabled.
            enum THREAD_GROUP_FORWARD_PROGRESS_GUARANTEE
            {
                THREAD_GROUP_FORWARD_PROGRESS_GUARANTEE_DISABLE                  = 0, //!< CFEG HW does not set an implicit barrier when WMTP is enabled
                THREAD_GROUP_FORWARD_PROGRESS_GUARANTEE_ENABLE                   = 1, //!< CFEG HW forces an implicit barrier when WMTP is enabled.
            };

            //! \brief SHARED_LOCAL_MEMORY_SIZE
            //! \details
            //!     This field indicates how much Shared Local Memory the thread group
            //!     requires.
            //!     If the barriers are not enabled,HW will enable at least 1 barrier for
            //!     Mid thread preemption to work.
            enum SHARED_LOCAL_MEMORY_SIZE
            {
                SHARED_LOCAL_MEMORY_SIZE_SLMENCODES0K                            = 0, //!< No additional details
                SHARED_LOCAL_MEMORY_SIZE_SLMENCODES1K                            = 1, //!< No additional details
                SHARED_LOCAL_MEMORY_SIZE_SLMENCODES2K                            = 2, //!< No additional details
                SHARED_LOCAL_MEMORY_SIZE_SLMENCODES4K                            = 3, //!< No additional details
                SHARED_LOCAL_MEMORY_SIZE_SLMENCODES8K                            = 4, //!< No additional details
                SHARED_LOCAL_MEMORY_SIZE_SLMENCODES16K                           = 5, //!< No additional details
                SHARED_LOCAL_MEMORY_SIZE_SLMENCODES32K                           = 6, //!< No additional details
                SHARED_LOCAL_MEMORY_SIZE_SLMENCODES64K                           = 7, //!< No additional details
                SHARED_LOCAL_MEMORY_SIZE_SLMENCODES24K                           = 8, //!< No additional details
                SHARED_LOCAL_MEMORY_SIZE_SLMENCODES48K                           = 9, //!< No additional details
                SHARED_LOCAL_MEMORY_SIZE_SLMENCODES96K                           = 10, //!< No additional details
                SHARED_LOCAL_MEMORY_SIZE_SLMENCODES128K                          = 11, //!< No additional details
                SHARED_LOCAL_MEMORY_SIZE_SLMENCODES192K                          = 12, //!< No additional details
                SHARED_LOCAL_MEMORY_SIZE_SLMENCODES256K                          = 13, //!< No additional details
                SHARED_LOCAL_MEMORY_SIZE_SLMENCODES384K                          = 14, //!< No additional details
            };

            enum ROUNDING_MODE
            {
                ROUNDING_MODE_RTNE                                               = 0, //!< Round to Nearest Even
                ROUNDING_MODE_RU                                                 = 1, //!< Round toward +Infinity
                ROUNDING_MODE_RD                                                 = 2, //!< Round toward -Infinity
                ROUNDING_MODE_RTZ                                                = 3, //!< Round toward Zero
            };

            //! \brief THREAD_GROUP_DISPATCH_SIZE
            //! \details
            //!     Provides a mechanism for Software to tune the settings based on WLs
            //!     to evenly distribute the threads across the entire m/c.
            //!     The recommended settings is just a guidance and not a programming
            //!     requirement.
            enum THREAD_GROUP_DISPATCH_SIZE
            {
                THREAD_GROUP_DISPATCH_SIZE_TGSIZE8                               = 0, //!< The dispatch size is 8 thread groups.
                THREAD_GROUP_DISPATCH_SIZE_TGSIZE4                               = 1, //!< The dispatch size is 4 thread groups.
                THREAD_GROUP_DISPATCH_SIZE_TGSIZE2                               = 2, //!< The dispatch size is 2 thread groups.
                THREAD_GROUP_DISPATCH_SIZE_TGSIZE1                               = 3, //!< The dispatch size is 1 thread groups.
            };

            //! \brief NUMBER_OF_BARRIERS
            //! \details
            //!     Specifies number of barriers in the threadgroup.
            enum NUMBER_OF_BARRIERS
            {
                NUMBER_OF_BARRIERS_NONE                                          = 0, //!< No additional details
                NUMBER_OF_BARRIERS_B1                                            = 1, //!< No additional details
                NUMBER_OF_BARRIERS_B2                                            = 2, //!< No additional details
                NUMBER_OF_BARRIERS_B4                                            = 3, //!< No additional details
                NUMBER_OF_BARRIERS_B8                                            = 4, //!< No additional details
                NUMBER_OF_BARRIERS_B16                                           = 5, //!< No additional details
                NUMBER_OF_BARRIERS_B24                                           = 6, //!< No additional details
                NUMBER_OF_BARRIERS_B32                                           = 7, //!< No additional details
            };

            //! \brief BTD_MODE
            //! \details
            //!     If this field is valid, it means that the Compute pipeline is
            //!     dispatching BTD threads.
            enum BTD_MODE
            {
                BTD_MODE_DISABLE                                                 = 0, //!< Normal thread dispatch
                BTD_MODE_ENABLE                                                  = 1, //!< When walker dispatched compute kernels either perform messages to the Bindless Thread Dispatch (BTD) shared function or Ray Tracing HW shared function, this bit must be enabled.When this bit is enabled, neither SLM nor barrier is available.
            };

            //! \brief PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE
            //! \details
            //!     For products where SLM and Subslice L1 cacheshares a common,
            //!     re-partitionable RAM, this field indicates the preferred SLM size per
            //!     Subslice for this dispatch. The SLM size programmed here should be >=
            //!     the per thread-group SLM size programmed in DW[5][20:16].
            enum PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE
            {
                PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES0K          = 0, //!< No additional details
                PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES16K         = 1, //!< No additional details
                PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES32K         = 2, //!< No additional details
                PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES64K         = 3, //!< No additional details
                PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES96K         = 4, //!< No additional details
                PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES128K        = 5, //!< No additional details
                PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES160K        = 6, //!< No additional details
                PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES192K        = 7, //!< No additional details
                PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES224K        = 8, //!< No additional details
                PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES256K        = 9, //!< No additional details
                PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES384K        = 10, //!< No additional details
            };

            //! \name Initializations

            //! \brief Explicit member initialization function
            INTERFACE_DESCRIPTOR_DATA_CMD();

            static const size_t dwSize = 8;
            static const size_t byteSize = 32;
        } InterfaceDescriptor;

        //!
        //! \brief POSTSYNC_DATA
        //! \details
        //!
        //!
        struct POSTSYNC_DATA_CMD
        {
            union
            {
                struct
                {
                    uint32_t                 Operation                                        : __CODEGEN_BITFIELD( 0,  1)    ; //!< OPERATION
                    uint32_t                 DataportPipelineFlush                            : __CODEGEN_BITFIELD( 2,  2)    ; //!< Dataport Pipeline Flush
                    uint32_t                 Reserved3                                        : __CODEGEN_BITFIELD( 3,  3)    ; //!< Reserved
                    uint32_t                 Mocs                                             : __CODEGEN_BITFIELD( 4, 10)    ; //!< MOCS
                    uint32_t                 SystemMemoryFenceRequest                         : __CODEGEN_BITFIELD(11, 11)    ; //!< System Memory Fence Request
                    uint32_t                 DataportSubsliceCacheFlush                       : __CODEGEN_BITFIELD(12, 12)    ; //!< Dataport Subslice Cache Flush
                    uint32_t                 Reserved13                                       : __CODEGEN_BITFIELD(13, 31)    ; //!< Reserved
                };
                uint32_t                     Value;
            } DW0;
            union
            {
                struct
                {
                    uint64_t                 DestinationAddress                                                               ; //!< Destination Address
                };
                uint32_t                     Value[2];
            } DW1_2;
            union
            {
                struct
                {
                    uint64_t                 ImmediateData                                                                    ; //!< Immediate Data
                };
                uint32_t                     Value[2];
            } DW3_4;

            //! \name Local enumerations

            enum OPERATION
            {
                OPERATION_NOWRITE                                                = 0, //!< The Destination Address and Immediate Data fields are ignored.
                OPERATION_WRITEIMMEDIATEDATA                                     = 1, //!< Writes 8 bytes (64 bits) of Immediate Data to the Destination Address.
                OPERATION_WRITETIMESTAMP                                         = 3, //!< Writes 32 bytes (256bits) of Timestamp Data to the Destination Address.The Immediate Data field is ignored.The timestamp layout :[0] = 64b Context Timestamp Start[1] = 64b Global Timestamp Start[2] = 64b Context Timestamp End[3] = 64b Global Timestamp End
            };

            //! \name Initializations

            //! \brief Explicit member initialization function
            POSTSYNC_DATA_CMD();

            static const size_t dwSize = 5;
            static const size_t byteSize = 20;
        } PostSync;

        struct INLINE_DATA_CMD
        {
            uint32_t Value[8] = {0};

            //! \brief Explicit member initialization function
            INLINE_DATA_CMD() {}
            static const size_t dwSize   = 8;
            static const size_t byteSize = 32;
        } InlineData;

        //! \name Local enumerations

        enum CFE_SUBOPCODE_VARIANT
        {
            CFE_SUBOPCODE_VARIANT_STANDARD                                   = 0, //!< No additional details
            CFE_SUBOPCODE_VARIANT_PASS1_RESUME                               = 1, //!< Resumption of Compute Walkers that has Thread groups only in Pass1 (EOT Preempt Saved), recorded in context image to continue execution after preemption.
            CFE_SUBOPCODE_VARIANT_PASS2_RESUME                               = 2, //!< Resumption of Compute Walkers that only has Pass2 (EOT NOT-RUN) thread groups recorded in context image to continue execution after preemption.
            CFE_SUBOPCODE_VARIANT_BTDPASS2                                   = 3, //!< This encoding is used by BTD over dispatched threads.  This value is not saved in the Walker Context image
            CFE_SUBOPCODE_VARIANT_PASS1PASS2_RESUME                          = 4, //!< Resumption of Compute Walkers that has a Mix of Pass1 (EOT Preempt Saved) and Pass2 (EOT NOT-RUN) thread groups recorded in context image to continue execution after preemption.
            CFE_SUBOPCODE_VARIANT_TG_RESUME                                  = 5, //!< Resumption of COMPUTE_WALKER that was preempted at a Thread group and has completed execution of previous TGs, recorded in context image to continue execution after preemption.
            CFE_SUBOPCODE_VARIANT_WDONE                                      = 7, //!< Walker completed execution of all TGs.
        };

        enum CFE_SUBOPCODE
        {
            CFE_SUBOPCODE_COMPUTEWALKER                                      = 2, //!< No additional details
        };

        enum COMPUTE_COMMAND_OPCODE
        {
            COMPUTE_COMMAND_OPCODE_NEWCFECOMMAND                             = 2, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_COMPUTE                                                 = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_GFXPIPE                                             = 3, //!< No additional details
        };

        //! \brief PARTITION_TYPE
        //! \details
        //!     Specifies whether the command is executed by multiple partitions.
        //!       When partitioned, the X or Y or Z dispatches are split at
        //!     Partition Size boundaries.
        enum PARTITION_TYPE
        {
            PARTITION_TYPE_DISABLED                                          = 0, //!< The command is not partitioned. Partition ID and Partition Size are ignored.
            PARTITION_TYPE_X                                                 = 1, //!< The command is partitioned in the X dimension. The X walk is between (PartitionID * PartitionSize) <= X < ((PartitionID+1)*PartitionSize). All Y and Z walks are performed in this partition.
            PARTITION_TYPE_Y                                                 = 2, //!< The command is partitioned in the Y dimension. The Y walk is between (PartitionID * PartitionSize) <= Y < ((PartitionID+1)*PartitionSize). All X and Z walks are performed in this partition.
            PARTITION_TYPE_Z                                                 = 3, //!< The command is partitioned in the Z dimension. The Z walk is between (PartitionID * PartitionSize) <= Z < ((PartitionID+1)*PartitionSize). All X and Y walks are performed in this partition.
        };

        //! \brief MESSAGE_SIMD
        //! \details
        //!     Specifies the SIMD size of the messages used to access the local data.
        //!     When the message size is less than the thread SIMD size, then the Local
        //!     ID are batched so that the smaller message SIMD size keep full cache
        //!     lines together in fused threads.
        enum MESSAGE_SIMD
        {
            MESSAGE_SIMD_SIMT16                                              = 1, //!< No additional details
            MESSAGE_SIMD_SIMT32                                              = 2, //!< No additional details
        };

        //! \brief TILE_LAYOUT
        //! \details
        //!     Specifies whether 2D and 3D surfaces are stored in Linear or TileY
        //!     layouts. The local ID values are batched together to keep full cache
        //!     lines together in the same SIMD thread.
        enum TILE_LAYOUT
        {
            TILE_LAYOUT_LINEAR                                               = 0, //!< No additional details
            TILE_LAYOUT_TILEY32BPE                                           = 1, //!< No additional details
            TILE_LAYOUT_TILEY64BPE                                           = 2, //!< No additional details
            TILE_LAYOUT_TILEY128BPE                                          = 3, //!< No additional details
        };

        //! \brief WALK_ORDER
        //! \details
        //!     Specifies which dimensions are the first and second priority order for
        //!     binding together in SIMD threads. In the values below, 0 is the first
        //!     priority and 1 is the second priority.
        enum WALK_ORDER
        {
            WALK_ORDER_WALK012                                               = 0, //!< Normal Linear walk order
            WALK_ORDER_WALK021                                               = 1, //!< No additional details
            WALK_ORDER_WALK102                                               = 2, //!< Normal TileY walk order
            WALK_ORDER_WALK120                                               = 3, //!< No additional details
            WALK_ORDER_WALK201                                               = 4, //!< No additional details
            WALK_ORDER_WALK210                                               = 5, //!< No additional details
        };

        //! \brief EMIT_LOCAL
        //! \details
        //!     These bits identify whether the register payload for Local X/Y/Z
        //!     indices will be present. Bit 26 is X, Bit 27 is Y, and Bit 28 is Z.
        //!     Separate <a
        //!     href="https:gfxspecs.intel.com/Predator/Home/Index/55403">GPGPU_LOCALID</a>
        //!     register payloads aregenerated when the corresponding bit isset.
        //!     If Generate Local ID is enabled, then the thread dispatcher generates
        //!     the corresponding Local X/Y/Z index values, using the Local X/Y/Z
        //!     Maximum values from DW6 of this command. For any enable bit that is not
        //!     set, the corresponding Local ID will not be generated and that register
        //!     will not be emitted into the per-thread payload. When an enable bit is
        //!     not set, its corresponding Local Maximum value in DW6 must be 0.
        enum EMIT_LOCAL
        {
            EMIT_LOCAL_EMITNONE                                              = 0, //!< No additional details
            EMIT_LOCAL_EMITX                                                 = 1, //!< No additional details
            EMIT_LOCAL_EMITXY                                                = 3, //!< No additional details
            EMIT_LOCAL_EMITXYZ                                               = 7, //!< No additional details
        };

        //! \brief SIMD_SIZE
        //! \details
        //!     This field determines the size of the payload and the number of bits of
        //!     the execution mask that are expected.  The kernel pointed to by the
        //!     interface descriptor should match the SIMD declared here.
        enum SIMD_SIZE
        {
            SIMD_SIZE_SIMT16                                                 = 1, //!< 16 LSBs of the execution mask are used
            SIMD_SIZE_SIMT32                                                 = 2, //!< 32 bits of execution mask used
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        COMPUTE_WALKER_CMD();

        static const size_t dwSize = 40;
        static const size_t byteSize = 160;
    };

    //!
    //! \brief CFE_STATE
    //! \details
    //!     Set the compute pipeline state.
    //!
    struct CFE_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 CfeSubopcodeVariant                              : __CODEGEN_BITFIELD(16, 17)    ; //!< CFE_SUBOPCODE_VARIANT
                uint32_t                 CfeSubopcode                                     : __CODEGEN_BITFIELD(18, 23)    ; //!< CFE_SUBOPCODE
                uint32_t                 ComputeCommandOpcode                             : __CODEGEN_BITFIELD(24, 26)    ; //!< COMPUTE_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0,  9)    ; //!< Reserved
                uint32_t                 ScratchSpaceBuffer                               : __CODEGEN_BITFIELD(10, 31)    ; //!< Scratch Space Buffer
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 Reserved64                                                                       ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 ControlsTheNumberOfStackidsForRayTracingSubsystem : __CODEGEN_BITFIELD( 0,  1)    ; //!< CONTROLS_THE_NUMBER_OF_STACKIDS_FOR_RAY_TRACING_SUBSYSTEM
                uint32_t                 Reserved98                                       : __CODEGEN_BITFIELD( 2,  9)    ; //!< Reserved
                uint32_t                 LargeGrfThreadAdjustDisable                      : __CODEGEN_BITFIELD(10, 10)    ; //!< LARGE_GRF_THREAD_ADJUST_DISABLE
                uint32_t                 ComputeOverdispatchDisable                       : __CODEGEN_BITFIELD(11, 11)    ; //!< COMPUTE_OVERDISPATCH_DISABLE
                uint32_t                 ComputeDispatchAllWalkerEnable                   : __CODEGEN_BITFIELD(12, 12)    ; //!< COMPUTE_DISPATCH_ALL_WALKER_ENABLE
                uint32_t                 Reserved109                                      : __CODEGEN_BITFIELD(13, 13)    ; //!< Reserved
                uint32_t                 OverDispatchControl                              : __CODEGEN_BITFIELD(14, 15)    ; //!< OVER_DISPATCH_CONTROL
                uint32_t                 MaximumNumberOfThreads                           : __CODEGEN_BITFIELD(16, 31)    ; //!< Maximum Number of Threads
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 Reserved128                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 ResumeIndicatorDebugkey                          : __CODEGEN_BITFIELD( 0,  0)    ; //!< Resume indicator debugkey
                uint32_t                 WalkerNumberDebugkey                             : __CODEGEN_BITFIELD( 1, 10)    ; //!< Walker number debugkey
                uint32_t                 Reserved171                                      : __CODEGEN_BITFIELD(11, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;

        //! \name Local enumerations

        enum CFE_SUBOPCODE_VARIANT
        {
            CFE_SUBOPCODE_VARIANT_STANDARD                                   = 0, //!< No additional details
        };

        enum CFE_SUBOPCODE
        {
            CFE_SUBOPCODE_CFESTATE                                           = 0, //!< No additional details
        };

        enum COMPUTE_COMMAND_OPCODE
        {
            COMPUTE_COMMAND_OPCODE_NEWCFECOMMAND                             = 2, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_COMPUTE                                                 = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_GFXPIPE                                             = 3, //!< No additional details
        };

        //! \brief CONTROLS_THE_NUMBER_OF_STACKIDS_FOR_RAY_TRACING_SUBSYSTEM
        //! \details
        //!     This field allows controlling the number stackIDs (i.e. #unique rays in
        //!     the Ray Tracing subsytem.
        enum CONTROLS_THE_NUMBER_OF_STACKIDS_FOR_RAY_TRACING_SUBSYSTEM
        {
            CONTROLS_THE_NUMBER_OF_STACKIDS_FOR_RAY_TRACING_SUBSYSTEM_2K     = 0, //!< Number of stackIDs = 2048
            CONTROLS_THE_NUMBER_OF_STACKIDS_FOR_RAY_TRACING_SUBSYSTEM_1K     = 1, //!< Number of stackIDs = 1024
            CONTROLS_THE_NUMBER_OF_STACKIDS_FOR_RAY_TRACING_SUBSYSTEM_512    = 2, //!< Number of stackIDs = 512
            CONTROLS_THE_NUMBER_OF_STACKIDS_FOR_RAY_TRACING_SUBSYSTEM_256    = 3, //!< Number of stackIDs = 256
        };

        //! \brief LARGE_GRF_THREAD_ADJUST_DISABLE
        //! \details
        //!     When this bit is set, the thread dispatch logic will not cap the
        //!     max_outstanding_threads based on the large grf mode. This is a Chicken
        //!     bit.
        enum LARGE_GRF_THREAD_ADJUST_DISABLE
        {
            LARGE_GRF_THREAD_ADJUST_DISABLE_ENABLED                          = 0, //!< No additional details
            LARGE_GRF_THREAD_ADJUST_DISABLE_DISABLED                         = 1, //!< No additional details
        };

        //! \brief COMPUTE_OVERDISPATCH_DISABLE
        //! \details
        //!     When this bit is set, the thread dispatch logic will disable over
        //!     dispatching of threads to the DSS.
        enum COMPUTE_OVERDISPATCH_DISABLE
        {
            COMPUTE_OVERDISPATCH_DISABLE_ENABLED                             = 0, //!< No additional details
            COMPUTE_OVERDISPATCH_DISABLE_DISABLED                            = 1, //!< No additional details
        };

        //! \brief COMPUTE_DISPATCH_ALL_WALKER_ENABLE
        //! \details
        //!     When this bit is set, the thread dispatch logic does a forced
        //!     round-robin dispatch to all the enabled DSS in this context.
        enum COMPUTE_DISPATCH_ALL_WALKER_ENABLE
        {
            COMPUTE_DISPATCH_ALL_WALKER_ENABLE_DISABLED                      = 0, //!< No additional details
            COMPUTE_DISPATCH_ALL_WALKER_ENABLE_ENABLED                       = 1, //!< No additional details
        };

        //! \brief OVER_DISPATCH_CONTROL
        //! \details
        //!     Enables the amount of GPGPU thread over dispatch.
        enum OVER_DISPATCH_CONTROL
        {
            OVER_DISPATCH_CONTROL_NONE                                       = 0, //!< 0% overdispatch
            OVER_DISPATCH_CONTROL_LOW                                        = 1, //!< 6.25% overdispatch
            OVER_DISPATCH_CONTROL_NORMAL                                     = 2, //!< 12.5% overdispatch
            OVER_DISPATCH_CONTROL_HIGH                                       = 3, //!< 25% overdispatch
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        CFE_STATE_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief STATE_COMPUTE_MODE
    //! \details
    //!     This is a non-pipeline state command and is a general compute
    //!     programming state that can be shared from the top to bottom of the
    //!     pipeline.
    //!
    struct STATE_COMPUTE_MODE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 _3DCommandSubOpcode                              : __CODEGEN_BITFIELD(16, 23)    ; //!< _3D_COMMAND_SUB_OPCODE
                uint32_t                 _3DCommandOpcode                                 : __CODEGEN_BITFIELD(24, 26)    ; //!< _3D_COMMAND_OPCODE
                uint32_t                 CommandSubtype                                   : __CODEGEN_BITFIELD(27, 28)    ; //!< COMMAND_SUBTYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 ZPassAsyncComputeThreadLimit                     : __CODEGEN_BITFIELD( 0,  2)    ; //!< Z_PASS_ASYNC_COMPUTE_THREAD_LIMIT
                uint32_t                 NpZAsyncThrottleSettings                         : __CODEGEN_BITFIELD( 3,  4)    ; //!< NP_Z_ASYNC_THROTTLE_SETTINGS
                uint32_t                 Reserved37                                       : __CODEGEN_BITFIELD( 5,  6)    ; //!< Reserved
                uint32_t                 AsyncComputeThreadLimit                          : __CODEGEN_BITFIELD( 7,  9)    ; //!< ASYNC_COMPUTE_THREAD_LIMIT
                uint32_t                 Reserved42                                       : __CODEGEN_BITFIELD(10, 12)    ; //!< Reserved
                uint32_t                 EuThreadSchedulingModeOverride                   : __CODEGEN_BITFIELD(13, 14)    ; //!< EU_THREAD_SCHEDULING_MODE_OVERRIDE
                uint32_t                 LargeGrfMode                                     : __CODEGEN_BITFIELD(15, 15)    ; //!< LARGE_GRF_MODE
                uint32_t                 Mask1                                            : __CODEGEN_BITFIELD(16, 31)    ; //!< Mask1
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 MidthreadPreemptionDelayTimer                    : __CODEGEN_BITFIELD( 0,  2)    ; //!< MIDTHREAD_PREEMPTION_DELAY_TIMER
                uint32_t                 MidthreadPreemptionOverdispatchThreadGroupCount  : __CODEGEN_BITFIELD( 3,  4)    ; //!< MIDTHREAD_PREEMPTION_OVERDISPATCH_THREAD_GROUP_COUNT
                uint32_t                 MidthreadPreemptionOverdispatchTestMode          : __CODEGEN_BITFIELD( 5,  5)    ; //!< MIDTHREAD_PREEMPTION_OVERDISPATCH_TEST_MODE
                uint32_t                 UavCoherencyMode                                 : __CODEGEN_BITFIELD( 6,  6)    ; //!< UAV_COHERENCY_MODE
                uint32_t                 Reserved71                                       : __CODEGEN_BITFIELD( 7, 10)    ; //!< Reserved
                uint32_t                 MemoryAllocationForScratchAndMidthreadPreemptionBuffers : __CODEGEN_BITFIELD(11, 11)    ; //!< MEMORY_ALLOCATION_FOR_SCRATCH_AND_MIDTHREAD_PREEMPTION_BUFFERS
                uint32_t                 Reserved76                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Mask2                                            : __CODEGEN_BITFIELD(16, 31)    ; //!< Mask2
            };
            uint32_t                     Value;
        } DW2;

        //! \name Local enumerations

        enum _3D_COMMAND_SUB_OPCODE
        {
            _3D_COMMAND_SUB_OPCODE_STATECOMPUTEMODE                          = 5, //!< No additional details
        };

        enum _3D_COMMAND_OPCODE
        {
            _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED                           = 1, //!< No additional details
        };

        enum COMMAND_SUBTYPE
        {
            COMMAND_SUBTYPE_GFXPIPECOMMON                                    = 0, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_GFXPIPE                                             = 3, //!< No additional details
        };

        //! \brief Z_PASS_ASYNC_COMPUTE_THREAD_LIMIT
        //! \details
        //!     Specifies the maximum number of active Compute CS threads to run in a
        //!     DSS when the 3D Pipe is active and a Z-pass is not running. When the 3D
        //!     Pipe is not active or when a Z-pass is running, the maximum number of
        //!     active Compute CS threads is specified by Maximum Number of
        //!     Threads in CFE_STATE command.
        enum Z_PASS_ASYNC_COMPUTE_THREAD_LIMIT
        {
            Z_PASS_ASYNC_COMPUTE_THREAD_LIMIT_MAX60                          = 0, //!< Maximum of upto 1 thread per fused EU reserved for 3D.
            Z_PASS_ASYNC_COMPUTE_THREAD_LIMIT_MAX64                          = 1, //!< No limit applied. Maximum Number of Threads is the only limit on Compute CS threads.
            Z_PASS_ASYNC_COMPUTE_THREAD_LIMIT_MAX56                          = 2, //!< Maximum of 1 thread per fused EU reserved for 3D .
            Z_PASS_ASYNC_COMPUTE_THREAD_LIMIT_MAX48                          = 3, //!< Maximum of 2 thread per fused EU reserved for 3D .
            Z_PASS_ASYNC_COMPUTE_THREAD_LIMIT_MAX40                          = 4, //!< Maximum of 3 thread per fused EU reserved for 3D .
            Z_PASS_ASYNC_COMPUTE_THREAD_LIMIT_MAX32                          = 5, //!< Maximum of 4 thread per fused EU reserved for 3D .
        };

        enum NP_Z_ASYNC_THROTTLE_SETTINGS
        {
            NP_Z_ASYNC_THROTTLE_SETTINGS_UNNAMED0                            = 0, //!< Use the same settings as the Pixel shader Async compute settings i.e bits[9:7] of this DW.
            NP_Z_ASYNC_THROTTLE_SETTINGS_MAX32                               = 1, //!< Maximum of 4 thread per fused EU reserved for 3D .
            NP_Z_ASYNC_THROTTLE_SETTINGS_MAX40                               = 2, //!< Maximum of 3 thread per fused EU reserved for 3D .
            NP_Z_ASYNC_THROTTLE_SETTINGS_MAX48                               = 3, //!< Maximum of 2 thread per fused EU reserved for 3D .
        };

        //! \brief ASYNC_COMPUTE_THREAD_LIMIT
        //! \details
        //!     Specifies the maximum number of active Compute CS threads to run in a
        //!     DSS when the 3D Pipe is active and a Z-pass is not running. When the 3D
        //!     Pipe is not active or when a Z-pass is running, the maximum number of
        //!     active Compute CS threads is specified by Maximum Number of
        //!     Threads in CFE_STATE command.
        enum ASYNC_COMPUTE_THREAD_LIMIT
        {
            ASYNC_COMPUTE_THREAD_LIMIT_DISABLED                              = 0, //!< No limit applied. Maximum Number of Threads is the only limit on Compute CS threads.
            ASYNC_COMPUTE_THREAD_LIMIT_MAX2                                  = 1, //!< Maximum of 2 EU threads per DSS, when 3D Pipe is active. This sets the Async Compute thread limit to about 1 thread per EU row.
            ASYNC_COMPUTE_THREAD_LIMIT_MAX8                                  = 2, //!< Maximum of 8 EU threads per DSS, when 3D Pipe is active. This sets the Async Compute thread limit to about 1 thread per EU .
            ASYNC_COMPUTE_THREAD_LIMIT_MAX16                                 = 3, //!< Maximum of 16 EU threads per DSS, when 3D Pipe is active. This sets the Async Compute thread limit to about 2 threads per EU .
            ASYNC_COMPUTE_THREAD_LIMIT_MAX24                                 = 4, //!< Maximum of 24 EU threads per DSS, when 3D Pipe is active.
            ASYNC_COMPUTE_THREAD_LIMIT_MAX32                                 = 5, //!< Maximum of 32 EU threads per DSS, when 3D Pipe is active.
            ASYNC_COMPUTE_THREAD_LIMIT_MAX40                                 = 6, //!< Maximum of 40 EU threads per DSS, when 3D Pipe is active.
            ASYNC_COMPUTE_THREAD_LIMIT_MAX48                                 = 7, //!< Maximum of 48 EU threads per DSS, when 3D Pipe is active.
        };

        //! \brief EU_THREAD_SCHEDULING_MODE_OVERRIDE
        //! \details
        //!     Override the thread scheduling policy in EU.
        enum EU_THREAD_SCHEDULING_MODE_OVERRIDE
        {
            EU_THREAD_SCHEDULING_MODE_OVERRIDE_HWDEFAULT                     = 0, //!< No override - HW selects optimal scheduling policy. For DGT and PVC, HW uses Oldest First scheduling.
            EU_THREAD_SCHEDULING_MODE_OVERRIDE_OLDESTFIRST                   = 1, //!< EU will always schedule the oldest ready thread.
            EU_THREAD_SCHEDULING_MODE_OVERRIDE_ROUNDROBIN                    = 2, //!< EU will schedule threads in a round-robin manner, switching to the next ready thread every cycle.
            EU_THREAD_SCHEDULING_MODE_OVERRIDE_STALLBASEDROUNDROBIN          = 3, //!< EU will schedule threads in a round-robin manner, but scheduling will only switch when the current thread is stalled due to dependency.
        };

        //! \brief LARGE_GRF_MODE
        //! \details
        //!     This bit controls the Large GRF Mode Vs Regular GRF Mode in Execution
        //!     Units.
        enum LARGE_GRF_MODE
        {
            LARGE_GRF_MODE_UNNAMED0                                          = 0, //!< Regular GRF mode of operation.
            LARGE_GRF_MODE_UNNAMED1                                          = 1, //!< Large GRF mode of operation.
        };

        //! \brief MIDTHREAD_PREEMPTION_DELAY_TIMER
        //! \details
        //!     The delay timer gives the option of giving time for threads that are on
        //!     the verge of exiting to completely exit thereby avoiding saving the
        //!     thread state.
        enum MIDTHREAD_PREEMPTION_DELAY_TIMER
        {
            MIDTHREAD_PREEMPTION_DELAY_TIMER_MTPTIMERVAL0                    = 0, //!< Mid thread preemption event is signalled to the EU as soon as CFEG receives it from CS.
            MIDTHREAD_PREEMPTION_DELAY_TIMER_MTPTIMERVAL50                   = 1, //!< Mid thread preemption timer of 50usec.
            MIDTHREAD_PREEMPTION_DELAY_TIMER_MTPTIMERVAL100                  = 2, //!< Mid thread preemption timer of 100usec
            MIDTHREAD_PREEMPTION_DELAY_TIMER_MTPTIMERVAL150                  = 3, //!< Mid thread preemption timer of 150usec
        };

        //! \brief MIDTHREAD_PREEMPTION_OVERDISPATCH_THREAD_GROUP_COUNT
        //! \details
        //!     The overdispatch TG buffer a.k.a ODB buffer stores the XYZ ID of the
        //!     TG that were dispatched by HW but did not land on the EU after the
        //!     preemption is signaled. This buffer is in PPGGT space and it has to be
        //!     managed by the CFEG to avoid overflowing.
        //!     The ODB buffer size is programmed based on the Physical machine
        //!     size.
        enum MIDTHREAD_PREEMPTION_OVERDISPATCH_THREAD_GROUP_COUNT
        {
            MIDTHREAD_PREEMPTION_OVERDISPATCH_THREAD_GROUP_COUNT_ODTGM2      = 0, //!< HW Caps the Over dispatched TG count in ODB buffer to MAX_NUM_THRDS / 2.
            MIDTHREAD_PREEMPTION_OVERDISPATCH_THREAD_GROUP_COUNT_ODTGM4      = 1, //!< HW Caps the Over dispatched TG count in ODB buffer to MAX_NUM_THRDS / 4.
            MIDTHREAD_PREEMPTION_OVERDISPATCH_THREAD_GROUP_COUNT_ODTGM8      = 2, //!< HW Caps the Over dispatched TG count in ODB buffer to MAX_NUM_THRDS / 8
            MIDTHREAD_PREEMPTION_OVERDISPATCH_THREAD_GROUP_COUNT_ODTGM16     = 3, //!< HW Caps the Over dispatched TG count in ODB buffer to MAX_NUM_THRDS / 16
        };

        //! \brief MIDTHREAD_PREEMPTION_OVERDISPATCH_TEST_MODE
        //! \details
        //!     This bit provides a mechanism to limit the number of ODB TGs to a
        //!     much smaller number in simulation.
        enum MIDTHREAD_PREEMPTION_OVERDISPATCH_TEST_MODE
        {
            MIDTHREAD_PREEMPTION_OVERDISPATCH_TEST_MODE_REGULAR              = 0, //!< Regular mode operation.  The MAX_ODB_CNT is derived from the value programmed in bits [4:3]
            MIDTHREAD_PREEMPTION_OVERDISPATCH_TEST_MODE_TESTMODE             = 1, //!< Caps the MAX_ODB_CNT to 64 .
        };

        //! \brief UAV_COHERENCY_MODE
        //! \details
        //!     This field controls whether UAV operations in the HW will flush the
        //!     L1 Dataport Cache or only drain the dataport pipe. If UAV's are cached,
        //!     SW must set this bit to ensure coherency of UAV's that are made coherent
        //!     through UAV barrier and UAV resource changes with RESOURCE_BARRIER(L1
        //!     Dataport UAV Flush).
        enum UAV_COHERENCY_MODE
        {
            UAV_COHERENCY_MODE_DRAIN_DATAPORT_MODE                            = 0, //!< Untyped L1 is neither flushed or invalidated for both UAV Barrier Coherency and BARRIER_RESOURCE with L1 Dataport UAV Flush.
            UAV_COHERENCY_MODE_FLUSH_DATAPORTL1                               = 1, //!< Untyped L1 is flushed,for both UAV Barrier Coherency and BARRIER_RESOURCE with L1 Dataport UAV Flush.
        };

        //! \brief MEMORY_ALLOCATION_FOR_SCRATCH_AND_MIDTHREAD_PREEMPTION_BUFFERS
        //! \details
        //!     This bit only applies for Compute-only contexts initiated from
        //!     CCS.
        //!     If set to 1, HW uses the Virtual Subslice ID instead of the Physical
        //!     Subslice ID to index the scratch and Midthread Preemption Thread
        //!     statebuffers (TSB). This reduces the memory footprint when running
        //!     multiple compute contexts.
        enum MEMORY_ALLOCATION_FOR_SCRATCH_AND_MIDTHREAD_PREEMPTION_BUFFERS
        {
            MEMORY_ALLOCATION_FOR_SCRATCH_AND_MIDTHREAD_PREEMPTION_BUFFERS_FULL = 0, //!< SW allocates the MAX memory (full GPU size) for scratch and Midthread Preemption buffers (TSB) for all CCS contexts
            MEMORY_ALLOCATION_FOR_SCRATCH_AND_MIDTHREAD_PREEMPTION_BUFFERS_MIN = 1, //!< SW allocates the minimum required memory for scratch and Midthread Preemption buffers (TSB) based on the number of Subslices assigned to this CCS context as defined in the CCS_MODE register .
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        STATE_COMPUTE_MODE_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief PALETTE_ENTRY
    //! \details
    //!
    //!
    struct PALETTE_ENTRY_CMD
    {
        union
        {
            struct
            {
                uint32_t                 Blue : __CODEGEN_BITFIELD(0, 7); //!< Blue
                uint32_t                 Green : __CODEGEN_BITFIELD(8, 15); //!< Green
                uint32_t                 Red : __CODEGEN_BITFIELD(16, 23); //!< Red
                uint32_t                 Alpha : __CODEGEN_BITFIELD(24, 31); //!< Alpha
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        PALETTE_ENTRY_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief GPGPU_CSR_BASE_ADDRESS
    //! \details
    //!     The GPGPU_CSR_BASE_ADDRESS command sets the base pointers for EU and L3
    //!     to Context Save and Restore EU State and SLM for GPGPU mid-thread
    //!     preemption.
    //!
    //!     Execution of this command causes a full pipeline flush, thus its use
    //!     should be minimized for higher performance. State and instruction caches
    //!     are flushed on completion of the flush.
    //!
    //!     SW must always program PIPE_CONTROL with "CS Stall" and "Render Target
    //!     Cache Flush Enable" set prior to programming GPGPU_CSR_BASE_ADDRESS
    //!     command for GPGPU workloads i.e when pipeline select is GPGPU via
    //!     PIPELINE_SELECT command. This is required to achieve better GPGPU
    //!     preemption latencies for certain programming sequences. If programming
    //!     PIPE_CONTROL has performance implications then preemption latencies can
    //!     be trade off against performance by not implementing this programming
    //!     note.
    //!
    struct GPGPU_CSR_BASE_ADDRESS_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength : __CODEGEN_BITFIELD(0, 7); //!< DWORD_LENGTH
                uint32_t                 Reserved8 : __CODEGEN_BITFIELD(8, 15); //!< Reserved
                uint32_t                 _3DCommandSubOpcode : __CODEGEN_BITFIELD(16, 23); //!< _3D_COMMAND_SUB_OPCODE
                uint32_t                 _3DCommandOpcode : __CODEGEN_BITFIELD(24, 26); //!< _3D_COMMAND_OPCODE
                uint32_t                 CommandSubtype : __CODEGEN_BITFIELD(27, 28); //!< COMMAND_SUBTYPE
                uint32_t                 CommandType : __CODEGEN_BITFIELD(29, 31); //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint64_t                 Reserved32 : __CODEGEN_BITFIELD(0, 11); //!< Reserved
                uint64_t                 GpgpuCsrBaseAddress : __CODEGEN_BITFIELD(12, 63); //!< GPGPU CSR Base Address
            };
            uint32_t                     Value[2];
        } DW1_2;

        //! \name Local enumerations

        enum _3D_COMMAND_SUB_OPCODE
        {
            _3D_COMMAND_SUB_OPCODE_GPGPUCSRBASEADDRESS = 4, //!< No additional details
        };

        enum _3D_COMMAND_OPCODE
        {
            _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED = 1, //!< No additional details
        };

        enum COMMAND_SUBTYPE
        {
            COMMAND_SUBTYPE_GFXPIPECOMMON = 0, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_GFXPIPE = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        GPGPU_CSR_BASE_ADDRESS_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };
};
}  // namespace xe2_hpg_next
}  // namespace render
}  // namespace mhw
#pragma pack()

#endif  // __MHW_RENDER_HWCMD_XE2_HPG_NEXT_H__