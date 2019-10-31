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
//! \file     mhw_render_hwcmd_g12_X.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of g12_X as other components
//!           should use MHW interface to interact with MHW commands and states.
//!
#ifndef __MHW_RENDER_HWCMD_G12_X_H__
#define __MHW_RENDER_HWCMD_G12_X_H__

#pragma once
#pragma pack(1)

#include <cstdint>
#include <cstddef>

class mhw_render_g12_X
{
public:
    // Internal Macros
    #define __CODEGEN_MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
    #define __CODEGEN_BITFIELD(l, h) (h) - (l) + 1
    #define __CODEGEN_OP_LENGTH_BIAS 2
    #define __CODEGEN_OP_LENGTH(x) (uint32_t)((__CODEGEN_MAX(x, __CODEGEN_OP_LENGTH_BIAS)) - __CODEGEN_OP_LENGTH_BIAS)

    static uint32_t GetOpLength(uint32_t uiLength) { return __CODEGEN_OP_LENGTH(uiLength); }

    //!
    //! \brief MEDIA_OBJECT
    //! \details
    //!
    //!
    struct MEDIA_OBJECT_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< DWORD_LENGTH
                uint32_t                 MediaCommandSubOpcode                            : __CODEGEN_BITFIELD(16, 23)    ; //!< MEDIA_COMMAND_SUB_OPCODE
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 MediaCommandPipeline                             : __CODEGEN_BITFIELD(27, 28)    ; //!< MEDIA_COMMAND_PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 InterfaceDescriptorOffset                        : __CODEGEN_BITFIELD( 0,  5)    ; //!< Interface Descriptor Offset
                uint32_t                 Reserved38                                       : __CODEGEN_BITFIELD( 6, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 IndirectDataLength                               : __CODEGEN_BITFIELD( 0, 16)    ; //!< Indirect Data Length
                uint32_t                 SubsliceDestinationSelect                        : __CODEGEN_BITFIELD(17, 18)    ; //!< SUBSLICE_DESTINATION_SELECT
                uint32_t                 SliceDestinationSelect                           : __CODEGEN_BITFIELD(19, 20)    ; //!< SLICE_DESTINATION_SELECT
                uint32_t                 Reserved85                                       : __CODEGEN_BITFIELD(21, 21)    ; //!< Reserved
                uint32_t                 ForceDestination                                 : __CODEGEN_BITFIELD(22, 22)    ; //!< Force Destination
                uint32_t                 Reserved87                                       : __CODEGEN_BITFIELD(23, 23)    ; //!< Reserved
                uint32_t                 ThreadSynchronization                            : __CODEGEN_BITFIELD(24, 24)    ; //!< THREAD_SYNCHRONIZATION
                uint32_t                 SliceDestinationSelectMsbs                       : __CODEGEN_BITFIELD(25, 26)    ; //!< Slice Destination Select MSBs
                uint32_t                 Reserved91                                       : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 IndirectDataStartAddress                                                         ; //!< Indirect Data Start Address
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 XPosition                                        : __CODEGEN_BITFIELD( 0,  8)    ; //!< X Position
                uint32_t                 Reserved137                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved
                uint32_t                 YPosition                                        : __CODEGEN_BITFIELD(16, 24)    ; //!< Y Position
                uint32_t                 Reserved153                                      : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 Reserved160                                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Reserved
                uint32_t                 BlockColor                                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Block Color
                uint32_t                 Reserved184                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;

        //! \name Local enumerations

        enum MEDIA_COMMAND_SUB_OPCODE
        {
            MEDIA_COMMAND_SUB_OPCODE_MEDIAOBJECTSUBOP                        = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MEDIAOBJECT                                 = 1, //!< No additional details
        };

        enum MEDIA_COMMAND_PIPELINE
        {
            MEDIA_COMMAND_PIPELINE_MEDIA                                     = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_GFXPIPE                                             = 3, //!< No additional details
        };

        //! \brief SUBSLICE_DESTINATION_SELECT
        //! \details
        //!     This field selects the SubSlice that this thread must be sent to.
        //!     Ignored if <b>Force Destination</b> = 0
        enum SUBSLICE_DESTINATION_SELECT
        {
            SUBSLICE_DESTINATION_SELECT_SUBSLICE0                            = 0, //!< No additional details
            SUBSLICE_DESTINATION_SELECT_SUBSLICE1                            = 1, //!< No additional details
            SUBSLICE_DESTINATION_SELECT_SUBSLICE2                            = 2, //!< No additional details
            SUBSLICE_DESTINATION_SELECT_SUBSLICE3                            = 3, //!< No additional details
        };

        //! \brief SLICE_DESTINATION_SELECT
        //! \details
        //!     This bit along with the subslice destination select determines the slice
        //!     that this thread must be sent to.  Ignored if <b>Force Destination</b> =
        //!     0, or if product only has 1 slice.
        enum SLICE_DESTINATION_SELECT
        {
            SLICE_DESTINATION_SELECT_SLICE0                                  = 0, //!< No additional details
            SLICE_DESTINATION_SELECT_SLICE1                                  = 1, //!< Cannot be used in products without a Slice 1.
            SLICE_DESTINATION_SELECT_SLICE2                                  = 2, //!< Cannot be used in products without a Slice 2.
        };

        //! \brief THREAD_SYNCHRONIZATION
        //! \details
        //!     This field when set indicates that the dispatch of the thread originated
        //!     from this command is based on the "spawn root thread" message.
        enum THREAD_SYNCHRONIZATION
        {
            THREAD_SYNCHRONIZATION_NOTHREADSYNCHRONIZATION                   = 0, //!< No additional details
            THREAD_SYNCHRONIZATION_THREADDISPATCHISSYNCHRONIZEDBYTHESPAWNROOTTHREADMESSAGE = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MEDIA_OBJECT_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief PIPELINE_SELECT
    //! \details
    //!     The PIPELINE_SELECT command is used to specify which GPE pipeline is to
    //!     be considered the 'current'  active pipeline. Issuing
    //!     3D-pipeline-specific commands when the Media pipeline is selected, or
    //!     vice versa, is UNDEFINED.
    //!
    //!     Issuing 3D-pipeline-specific commands when the GPGPU pipeline is
    //!     selected, or vice versa, is UNDEFINED.
    //!
    //!     Programming common non pipeline commands (e.g., STATE_BASE_ADDRESS) is
    //!     allowed in all pipeline modes.
    //!
    //!     Software must ensure all the write caches are flushed through a stalling
    //!     PIPE_CONTROL command followed by another PIPE_CONTROL command to
    //!     invalidate read only caches prior to programming MI_PIPELINE_SELECT
    //!     command to change the Pipeline Select Mode. Example: ... Workload-3Dmode
    //!     PIPE_CONTROL  (CS Stall, Depth Cache Flush Enable, Render Target Cache
    //!     Flush Enable, DC Flush Enable) PIPE_CONTROL  (Constant Cache Invalidate,
    //!     Texture Cache Invalidate, Instruction Cache Invalidate, State Cache
    //!     invalidate) PIPELINE_SELECT ( GPGPU)
    //!
    struct PIPELINE_SELECT_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 PipelineSelection                                : __CODEGEN_BITFIELD( 0,  1)    ; //!< PIPELINE_SELECTION
                uint32_t                 RenderSliceCommonPowerGateEnable                 : __CODEGEN_BITFIELD( 2,  2)    ; //!< RENDER_SLICE_COMMON_POWER_GATE_ENABLE
                uint32_t                 RenderSamplerPowerGateEnable                     : __CODEGEN_BITFIELD( 3,  3)    ; //!< RENDER_SAMPLER_POWER_GATE_ENABLE
                uint32_t                 MediaSamplerDopClockGateEnable                   : __CODEGEN_BITFIELD( 4,  4)    ; //!< MEDIA_SAMPLER_DOP_CLOCK_GATE_ENABLE
                uint32_t                 ForceMediaAwake                                  : __CODEGEN_BITFIELD( 5,  5)    ; //!< FORCE_MEDIA_AWAKE
                uint32_t                 MediaSamplerPowerClockGateDisable                : __CODEGEN_BITFIELD( 6,  6)    ; //!< Media Sampler Power Clock Gate Disable
                uint32_t                 Reserved7                                        : __CODEGEN_BITFIELD( 7,  7)    ; //!< Reserved
                uint32_t                 MaskBits                                         : __CODEGEN_BITFIELD( 8, 15)    ; //!< Mask Bits
                uint32_t                 Command3DSubOpcode                               : __CODEGEN_BITFIELD(16, 23)    ; //!< _3D_COMMAND_SUB_OPCODE
                uint32_t                 Command3DOpcode                                  : __CODEGEN_BITFIELD(24, 26)    ; //!< _3D_COMMAND_OPCODE
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
            PIPELINE_SELECTION_MEDIA                                         = 1, //!< Media pipeline is selected (Includes HD optical disc playback, HD video playback, and generic media workloads)
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

        //! \brief MEDIA_SAMPLER_DOP_CLOCK_GATE_ENABLE
        //! \details
        //!     Mask bit [12] has to be set for HW to look at this field when
        //!     PIPELINE_SELECT command is parsed.
        enum MEDIA_SAMPLER_DOP_CLOCK_GATE_ENABLE
        {
            MEDIA_SAMPLER_DOP_CLOCK_GATE_ENABLE_DISABLED                     = 0, //!< Command Streamer sends message to PM to disable sampler DOP Clock Gating.
            MEDIA_SAMPLER_DOP_CLOCK_GATE_ENABLE_ENABLED                      = 1, //!< Command Streamer sends message to PM to enable media sampler DOP Clock Gating.
        };

        //! \brief FORCE_MEDIA_AWAKE
        //! \details
        //!     Mask bit [13] has to be set for HW to look at this field when
        //!     PIPELINE_SELECT command is parsed.
        //!
        //!                         Example for usage model:
        //!
        //!                         <b>RCS Ring Buffer</b>:
        //!                         <b>PIPELINE_SELECT (Force Media Awake set to '1')</b>
        //!                         MI_SEMPAHORE_SINGAL (Signal context id 0xABC to Render Command
        //!     Streamer)
        //!                         <b>PIPELINE_SELECT (Force Media Awake set to '0')</b>
        //!                         <b>MI_BATCH_BUFFER_START</b>
        //!                         STATE Commands ..
        //!                         …………
        //!                         <b>PIPELINE_SELECT (Force Media Awake set to '1')</b>
        //!                         MI_LOAD_REGISTER_IMM (Load register 0x23XX in render command
        //!     streamer with data 0xFFF)
        //!                         <b>PIPELINE_SELECT (Force Media Awake set to '0')</b>
        //!                         …………
        //!                         <b>MI_BATCH_BUFFER_END</b>
        enum FORCE_MEDIA_AWAKE
        {
            FORCE_MEDIA_AWAKE_DISABLED                                       = 0, //!< Command streamer sends message to PM to disable force awake of media engine (next instructions do not require the media engine to be awake). Command streamer waits for acknowledge from PM before parsing the next command.
            FORCE_MEDIA_AWAKE_ENABLED                                        = 1, //!< Command streamer sends message to PM to force awake media engine (next instructions require media engine awake). Command streamer waits for acknowledge from PM before parsing the next command.
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
    //!     state, instruction, and media indirect object accesses by the GPE.
    //!     For more information see the Base Address Utilization table in the
    //!     Memory Access Indirection narrative topic.
    //!
    //!     The following commands must be reissued following any change to the base
    //!     addresses:  3DSTATE_CC_POINTERS
    //!      3DSTATE_BINDING_TABLE_POINTERS
    //!      3DSTATE_SAMPLER_STATE_POINTERS
    //!      3DSTATE_VIEWPORT_STATE_POINTERS
    //!      MEDIA_STATE_POINTERS
    //!        Execution of this command causes a full pipeline flush, thus its use
    //!     should be minimized for higher performance.
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
    struct STATE_BASE_ADDRESS_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 Command3DSubOpcode                               : __CODEGEN_BITFIELD(16, 23)    ; //!< _3D_COMMAND_SUB_OPCODE
                uint32_t                 Command3DOpcode                                  : __CODEGEN_BITFIELD(24, 26)    ; //!< _3D_COMMAND_OPCODE
                uint32_t                 CommandSubtype                                   : __CODEGEN_BITFIELD(27, 28)    ; //!< COMMAND_SUBTYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1..2
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
            //!< DWORD 3
            struct
            {
                uint32_t                 Reserved96                                                 : __CODEGEN_BITFIELD( 0, 13)    ; //!< Reserved
                uint32_t                 DisableSupportforMultiGPUAtomicsforStatelessAccesses       : __CODEGEN_BITFIELD(14, 14)    ; //!<
                uint32_t                 DisableSupportForMultiGpuPartialWritesForStatelessMessages : __CODEGEN_BITFIELD(15, 15)    ; //!<
                uint32_t                 StatelessDataPortAccessMemoryObjectControlState            : __CODEGEN_BITFIELD(16, 22)    ; //!< Stateless Data Port Access Memory Object Control State
                uint32_t                 Reserved119                                                : __CODEGEN_BITFIELD(23, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4..5
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
            //!< DWORD 6..7
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
            //!< DWORD 8..9
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
            //!< DWORD 10..11
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
            //!< DWORD 12
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
            //!< DWORD 13
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
            //!< DWORD 14
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
            //!< DWORD 15
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
            //!< DWORD 16..17
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
            //!< DWORD 18
            struct
            {
                uint32_t                 Reserved576                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 BindlessSurfaceStateSize                         : __CODEGEN_BITFIELD(12, 31)    ; //!< Bindless Surface State Size
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            //!< DWORD 19..20
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
            //!< DWORD 21
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
        //!     <p>The other fields in this DWord and the following DWord are updated
        //!     only when this bit is set.</p>
        enum GENERAL_STATE_BASE_ADDRESS_MODIFY_ENABLE
        {
            GENERAL_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE                 = 0, //!< Ignore the updated address.
            GENERAL_STATE_BASE_ADDRESS_MODIFY_ENABLE_ENABLE                  = 1, //!< Modify the address.
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
        //!     <p>The other fields in this DWord and the following DWord are updated
        //!     only when this bit is set.</p>
        enum INDIRECT_OBJECT_BASE_ADDRESS_MODIFY_ENABLE
        {
            INDIRECT_OBJECT_BASE_ADDRESS_MODIFY_ENABLE_DISABLE               = 0, //!< Ignore the updated address.
            INDIRECT_OBJECT_BASE_ADDRESS_MODIFY_ENABLE_ENABLE                = 1, //!< Modify the address.
        };

        //! \brief INSTRUCTION_BASE_ADDRESS_MODIFY_ENABLE
        //! \details
        //!     <p>The other fields in this DWord and the following DWord are updated
        //!     only when this bit is set.</p>
        enum INSTRUCTION_BASE_ADDRESS_MODIFY_ENABLE
        {
            INSTRUCTION_BASE_ADDRESS_MODIFY_ENABLE_DISABLE                   = 0, //!< Ignore the updated address.
            INSTRUCTION_BASE_ADDRESS_MODIFY_ENABLE_ENABLE                    = 1, //!< Modify the address.
        };

        //! \brief GENERAL_STATE_BUFFER_SIZE_MODIFY_ENABLE
        //! \details
        //!     The bound in this DWord is updated only when this bit is set.
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
        //!     <p>The other fields in this DWord are updated only when this bit is
        //!     set.</p>
        enum BINDLESS_SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE
        {
            BINDLESS_SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE        = 0, //!< Ignore the updated address
            BINDLESS_SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE_ENABLE         = 1, //!< Modify the address
        };

        //! \brief BINDLESS_SAMPLER_STATE_BASE_ADDRESS_MODIFY_ENABLE
        //! \details
        //!     <p>The other fields in this DWord and the following two DWords are
        //!     updated only when this bit is set.</p>
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
    //! \brief MEDIA_VFE_STATE
    //! \details
    //!     A stalling PIPE_CONTROL is required before MEDIA_VFE_STATE unless the
    //!     only bits that are changed are scoreboard related: Scoreboard Enable,
    //!     Scoreboard Type, Scoreboard Mask, Scoreboard * Delta. For these
    //!     scoreboard related states, a MEDIA_STATE_FLUSH is sufficient.
    //!
    //!     MEDIA_STATE_FLUSH (optional, only if barrier dependency is needed)
    //!
    //!     MEDIA_INTERFACE_DESCRIPTOR_LOAD (optional)
    //!
    //!
    //!
    struct MEDIA_VFE_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< DWORD_LENGTH
                uint32_t                 Subopcode                                        : __CODEGEN_BITFIELD(16, 23)    ; //!< SUBOPCODE
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 PerThreadScratchSpace                            : __CODEGEN_BITFIELD( 0,  3)    ; //!< Per Thread Scratch Space
                uint32_t                 StackSize                                        : __CODEGEN_BITFIELD( 4,  7)    ; //!< Stack Size
                uint32_t                 Reserved40                                       : __CODEGEN_BITFIELD( 8,  9)    ; //!< Reserved
                uint32_t                 ScratchSpaceBasePointer                          : __CODEGEN_BITFIELD(10, 31)    ; //!< Scratch Space Base Pointer
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 ScratchSpaceBasePointerHigh                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Scratch Space Base Pointer High
                uint32_t                 Reserved80                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 Reserved96                                       : __CODEGEN_BITFIELD( 0,  1)    ; //!< Reserved
                uint32_t                 DispatchLoadBalance                              : __CODEGEN_BITFIELD( 2,  2)    ; //!< DISPATCH_LOAD_BALANCE
                uint32_t                 Reserved99                                       : __CODEGEN_BITFIELD( 3,  5)    ; //!< Reserved
                uint32_t                 FusedEuDispatch                                  : __CODEGEN_BITFIELD( 6,  6)    ; //!< FUSED_EU_DISPATCH
                uint32_t                 Reserved103                                      : __CODEGEN_BITFIELD( 7,  7)    ; //!< Reserved
                uint32_t                 NumberOfUrbEntries                               : __CODEGEN_BITFIELD( 8, 15)    ; //!< Number of URB Entries
                uint32_t                 MaximumNumberOfThreads                           : __CODEGEN_BITFIELD(16, 31)    ; //!< Maximum Number of Threads
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 MaximumNumberOfDualSubslices                     : __CODEGEN_BITFIELD( 0,  7)    ; //!< Maximum Number of Dual-Subslices
                uint32_t                 Reserved136                                      : __CODEGEN_BITFIELD( 8, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 CurbeAllocationSize                              : __CODEGEN_BITFIELD( 0, 15)    ; //!< CURBE Allocation Size
                uint32_t                 UrbEntryAllocationSize                           : __CODEGEN_BITFIELD(16, 31)    ; //!< URB Entry Allocation Size
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 Reserved192                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 Reserved224                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 Reserved256                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;

        //! \name Local enumerations

        enum SUBOPCODE
        {
            SUBOPCODE_MEDIAVFESTATESUBOP                                     = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MEDIAVFESTATE                               = 0, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MEDIA                                                   = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_GFXPIPE                                             = 3, //!< No additional details
        };

        //! \brief DISPATCH_LOAD_BALANCE
        //! \details
        //!     This bit determines how media threads are dispatched between the various
        //!     dual subslices.  GPGPU threads are not impacted by this bit.
        enum DISPATCH_LOAD_BALANCE
        {
            DISPATCH_LOAD_BALANCE_LEASTLOADED                                = 0, //!< When this value is used the threads are sent to the least loaded dual subslice of all active dual subslices. If media with groups is being used then each group is kept in the same dual subslice.
            DISPATCH_LOAD_BALANCE_COLORLSB                                   = 1, //!< When this value is used the threads are split into two groups depending on the LSB of the color value for media threads.  One group will be sent to even dual subslices and the other to odd dual subslices.  The least loaded active dual subslice available will selected for threads in that group.  This allows color to be used to separate workloads with different operations to get better cache coherency.If media with groups is being used then each group is kept in the same dual subslices.
        };

        //! \brief FUSED_EU_DISPATCH
        //! \details
        //!     This field determine if threads will be dispatched in sets to fused EUs
        //!     if set or if they will be dispatched individually. Depending of the
        //!     project the set size can be 2 or 4. If dispatched in sets the fused
        //!     threads will all be part of the same thread group for GPGPU threads or
        //!     will be part of the same iteration of the inner local loop if media
        //!     threads.
        enum FUSED_EU_DISPATCH
        {
            FUSED_EU_DISPATCH_FUSEDEUMODE                                    = 0, //!< No additional details
            FUSED_EU_DISPATCH_LEGACYMODE_THREADSARENOTFUSED                  = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MEDIA_VFE_STATE_CMD();

        static const size_t dwSize = 9;
        static const size_t byteSize = 36;
    };

    //!
    //! \brief MEDIA_CURBE_LOAD
    //! \details
    //!     See "GPGPU Command" section for additional
    //!     programming constraints for this command.
    //!
    struct MEDIA_CURBE_LOAD_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< DWORD_LENGTH
                uint32_t                 Subopcode                                        : __CODEGEN_BITFIELD(16, 23)    ; //!< SUBOPCODE
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Reserved32                                                                       ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 CurbeTotalDataLength                             : __CODEGEN_BITFIELD( 0, 16)    ; //!< CURBE Total Data Length
                uint32_t                 Reserved81                                       : __CODEGEN_BITFIELD(17, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 CurbeDataStartAddress                                                            ; //!< CURBE Data Start Address
            };
            uint32_t                     Value;
        } DW3;

        //! \name Local enumerations

        enum SUBOPCODE
        {
            SUBOPCODE_MEDIACURBELOADSUBOP                                    = 1, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MEDIACURBELOAD                              = 0, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MEDIA                                                   = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_GFXPIPE                                             = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MEDIA_CURBE_LOAD_CMD();

        static const size_t dwSize = 4;
        static const size_t byteSize = 16;
    };

    //!
    //! \brief MEDIA_INTERFACE_DESCRIPTOR_LOAD
    //! \details
    //!     A Media_State_Flush should be used before this command to ensure that
    //!     the temporary Interface Descriptor storage is cleared.
    //!
    //!     See "GPGPU Command" section for additional
    //!     programming constraints for this command.
    //!
    struct MEDIA_INTERFACE_DESCRIPTOR_LOAD_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< DWORD_LENGTH
                uint32_t                 Subopcode                                        : __CODEGEN_BITFIELD(16, 23)    ; //!< SUBOPCODE
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Reserved32                                                                       ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 InterfaceDescriptorTotalLength                   : __CODEGEN_BITFIELD( 0, 16)    ; //!< Interface Descriptor Total Length
                uint32_t                 Reserved81                                       : __CODEGEN_BITFIELD(17, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 InterfaceDescriptorDataStartAddress                                              ; //!< Interface Descriptor Data Start Address
            };
            uint32_t                     Value;
        } DW3;

        //! \name Local enumerations

        enum SUBOPCODE
        {
            SUBOPCODE_MEDIAINTERFACEDESCRIPTORLOADSUBOP                      = 2, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MEDIAINTERFACEDESCRIPTORLOAD                = 0, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MEDIA                                                   = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_GFXPIPE                                             = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MEDIA_INTERFACE_DESCRIPTOR_LOAD_CMD();

        static const size_t dwSize = 4;
        static const size_t byteSize = 16;
    };

    //!
    //! \brief MEDIA_OBJECT_WALKER
    //! \details
    //!
    //!
    struct MEDIA_OBJECT_WALKER_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< DWORD_LENGTH
                uint32_t                 Subopcode                                        : __CODEGEN_BITFIELD(16, 23)    ; //!< SUBOPCODE
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 InterfaceDescriptorOffset                        : __CODEGEN_BITFIELD( 0,  5)    ; //!< Interface Descriptor Offset
                uint32_t                 Reserved38                                       : __CODEGEN_BITFIELD( 6, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 IndirectDataLength                               : __CODEGEN_BITFIELD( 0, 16)    ; //!< Indirect Data Length
                uint32_t                 Reserved81                                       : __CODEGEN_BITFIELD(17, 21)    ; //!< Reserved
                uint32_t                 MaskedDispatch                                   : __CODEGEN_BITFIELD(22, 23)    ; //!< MASKED_DISPATCH
                uint32_t                 ThreadSynchronization                            : __CODEGEN_BITFIELD(24, 24)    ; //!< THREAD_SYNCHRONIZATION
                uint32_t                 Reserved89                                       : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 IndirectDataStartAddress                                                         ; //!< Indirect Data Start Address
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Reserved128                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 Reserved160                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< Reserved
                uint32_t                 GroupIdLoopSelect                                : __CODEGEN_BITFIELD( 8, 31)    ; //!< GROUP_ID_LOOP_SELECT
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 Reserved192                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< Reserved
                uint32_t                 MidLoopUnitX                                     : __CODEGEN_BITFIELD( 8,  9)    ; //!< Mid-Loop Unit X
                uint32_t                 Reserved202                                      : __CODEGEN_BITFIELD(10, 11)    ; //!< Reserved
                uint32_t                 LocalMidLoopUnitY                                : __CODEGEN_BITFIELD(12, 13)    ; //!< Local Mid-Loop Unit Y
                uint32_t                 Reserved206                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 MiddleLoopExtraSteps                             : __CODEGEN_BITFIELD(16, 20)    ; //!< Middle Loop Extra Steps
                uint32_t                 Reserved213                                      : __CODEGEN_BITFIELD(21, 23)    ; //!< Reserved
                uint32_t                 ColorCountMinusOne                               : __CODEGEN_BITFIELD(24, 31)    ; //!< Color Count Minus One
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 LocalLoopExecCount                               : __CODEGEN_BITFIELD( 0, 11)    ; //!< Local Loop Exec Count
                uint32_t                 Reserved236                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 GlobalLoopExecCount                              : __CODEGEN_BITFIELD(16, 27)    ; //!< Global Loop Exec Count
                uint32_t                 Reserved252                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 BlockResolutionX                                 : __CODEGEN_BITFIELD( 0, 10)    ; //!< Block Resolution X
                uint32_t                 Reserved267                                      : __CODEGEN_BITFIELD(11, 15)    ; //!< Reserved
                uint32_t                 BlockResolutionY                                 : __CODEGEN_BITFIELD(16, 26)    ; //!< Block Resolution Y
                uint32_t                 Reserved283                                      : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 LocalStartX                                      : __CODEGEN_BITFIELD( 0, 10)    ; //!< Local Start X
                uint32_t                 Reserved299                                      : __CODEGEN_BITFIELD(11, 15)    ; //!< Reserved
                uint32_t                 LocalStartY                                      : __CODEGEN_BITFIELD(16, 26)    ; //!< Local Start Y
                uint32_t                 Reserved315                                      : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 Reserved320                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 LocalOuterLoopStrideX                            : __CODEGEN_BITFIELD( 0, 11)    ; //!< Local Outer Loop Stride X
                uint32_t                 Reserved364                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 LocalOuterLoopStrideY                            : __CODEGEN_BITFIELD(16, 27)    ; //!< Local Outer Loop Stride Y
                uint32_t                 Reserved380                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 LocalInnerLoopUnitX                              : __CODEGEN_BITFIELD( 0, 11)    ; //!< Local Inner Loop Unit X
                uint32_t                 Reserved396                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 LocalInnerLoopUnitY                              : __CODEGEN_BITFIELD(16, 27)    ; //!< Local Inner Loop Unit Y
                uint32_t                 Reserved412                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 GlobalResolutionX                                : __CODEGEN_BITFIELD( 0, 10)    ; //!< Global Resolution X
                uint32_t                 Reserved427                                      : __CODEGEN_BITFIELD(11, 15)    ; //!< Reserved
                uint32_t                 GlobalResolutionY                                : __CODEGEN_BITFIELD(16, 26)    ; //!< Global Resolution Y
                uint32_t                 Reserved443                                      : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 GlobalStartX                                     : __CODEGEN_BITFIELD( 0, 11)    ; //!< Global Start X
                uint32_t                 Reserved460                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 GlobalStartY                                     : __CODEGEN_BITFIELD(16, 27)    ; //!< Global Start Y
                uint32_t                 Reserved476                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 GlobalOuterLoopStrideX                           : __CODEGEN_BITFIELD( 0, 11)    ; //!< Global Outer Loop Stride X
                uint32_t                 Reserved492                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 GlobalOuterLoopStrideY                           : __CODEGEN_BITFIELD(16, 27)    ; //!< Global Outer Loop Stride Y
                uint32_t                 Reserved508                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            //!< DWORD 16
            struct
            {
                uint32_t                 GlobalInnerLoopUnitX                             : __CODEGEN_BITFIELD( 0, 11)    ; //!< Global Inner Loop Unit X
                uint32_t                 Reserved524                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 GlobalInnerLoopUnitY                             : __CODEGEN_BITFIELD(16, 27)    ; //!< Global Inner Loop Unit Y
                uint32_t                 Reserved540                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW16;

        //! \name Local enumerations

        enum SUBOPCODE
        {
            SUBOPCODE_MEDIAOBJECTWALKERSUBOP                                 = 3, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MEDIAOBJECTWALKER                           = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MEDIA                                                   = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_GFXPIPE                                             = 3, //!< No additional details
        };

        //! \brief MASKED_DISPATCH
        //! \details
        //!     Enable the masking of the dispatch of individual threads based on a
        //!     bitmask read from CURBE, and specifies the pitch of the CURBE surface.
        //!                         If enabled, CURBE will not be used for thread payload.
        enum MASKED_DISPATCH
        {
            MASKED_DISPATCH_UNNAMED0                                         = 0, //!< Masked Dispatch Disabled
            MASKED_DISPATCH_UNNAMED1                                         = 1, //!< Masked Dispatch with 128-bit pitch in CURBE
            MASKED_DISPATCH_UNNAMED2                                         = 2, //!< Masked Dispatch with 256-bit pitch in CURBE
            MASKED_DISPATCH_UNNAMED3                                         = 3, //!< Masked Dispatch with 512-bit pitch in CURBE
        };

        //! \brief THREAD_SYNCHRONIZATION
        //! \details
        //!     This field when set indicates that the dispatch of the thread originated
        //!     from this command is based on the "spawn root thread" message.
        enum THREAD_SYNCHRONIZATION
        {
            THREAD_SYNCHRONIZATION_NOTHREADSYNCHRONIZATION                   = 0, //!< No additional details
            THREAD_SYNCHRONIZATION_THREADDISPATCHISSYNCHRONIZEDBYTHESPAWNROOTTHREADMESSAGE = 1, //!< No additional details
        };

        //! \brief GROUP_ID_LOOP_SELECT
        //! \details
        //!     This bit field chooses which of the nested loops of the walker are used
        //!     to identify threads which share a group id and therefore a shared
        //!     barrier and SLM.  The programmer must ensure that each group will fit
        //!     into a single subslice. When barriers are enabled every group must have
        //!     the same number of threads matching the number specified in the
        //!     Interface Descriptor.
        enum GROUP_ID_LOOP_SELECT
        {
            GROUP_ID_LOOP_SELECT_NOGROUPS                                    = 0, //!< Groups are not created, barriers and SLM are not allocated
            GROUP_ID_LOOP_SELECT_COLORGROUPS                                 = 1, //!< Each complete iteration of the Color loop defines a group, the group id is the concatenation of the Outer global, Inner global, Outer local, Mid local and Inner local loop execution counts.
            GROUP_ID_LOOP_SELECT_INNERLOCALGROUPS                            = 2, //!< Each complete iteration of the Inner local loop and Color loop defines a group, the group id is the concatenation of the Outer global loop to the Mid local loop execution counts.
            GROUP_ID_LOOP_SELECT_MIDLOCALGROUPS                              = 3, //!< Each complete iteration of the Mid local loop and lower loops defines a group, the group id is the concatenation of the Outer global loop to the Outer local loop execution counts.
            GROUP_ID_LOOP_SELECT_OUTERLOCALGROUPS                            = 4, //!< Each complete iteration of the Outer local loop and lower loops defines a group, the group id is the concatenation of the Outer global loop and the Inner global loop execution counts.
            GROUP_ID_LOOP_SELECT_INNERGLOBALGROUPS                           = 5, //!< Each complete iteration of the Inner global loop and lower loops defines a group, the group id is the Outer global loop execution count.
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MEDIA_OBJECT_WALKER_CMD();

        static const size_t dwSize = 17;
        static const size_t byteSize = 68;
    };

    //!
    //! \brief GPGPU_WALKER
    //! \details
    //!     If the threads spawned by this command are required to observe memory
    //!     writes performed by threads spawned from a previous command, software
    //!     must precede this command with a command that performs a memory flush
    //!     (e.g., MI_FLUSH).
    //!
    struct GPGPU_WALKER_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 PredicateEnable                                  : __CODEGEN_BITFIELD( 8,  8)    ; //!< Predicate Enable
                uint32_t                 Reserved9                                        : __CODEGEN_BITFIELD( 9,  9)    ; //!< Reserved
                uint32_t                 IndirectParameterEnable                          : __CODEGEN_BITFIELD(10, 10)    ; //!< Indirect Parameter Enable
                uint32_t                 Reserved11                                       : __CODEGEN_BITFIELD(11, 15)    ; //!< Reserved
                uint32_t                 Subopcode                                        : __CODEGEN_BITFIELD(16, 23)    ; //!< SUBOPCODE
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 InterfaceDescriptorOffset                        : __CODEGEN_BITFIELD( 0,  5)    ; //!< Interface Descriptor Offset
                uint32_t                 Reserved38                                       : __CODEGEN_BITFIELD( 6, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 IndirectDataLength                               : __CODEGEN_BITFIELD( 0, 16)    ; //!< Indirect Data Length
                uint32_t                 Reserved81                                       : __CODEGEN_BITFIELD(17, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 Reserved96                                       : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 IndirectDataStartAddress                         : __CODEGEN_BITFIELD( 6, 31)    ; //!< Indirect Data Start Address
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 ThreadWidthCounterMaximum                        : __CODEGEN_BITFIELD( 0,  5)    ; //!< Thread Width Counter Maximum
                uint32_t                 Reserved134                                      : __CODEGEN_BITFIELD( 6,  7)    ; //!< Reserved
                uint32_t                 ThreadHeightCounterMaximum                       : __CODEGEN_BITFIELD( 8, 13)    ; //!< Thread Height Counter Maximum
                uint32_t                 Reserved142                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 ThreadDepthCounterMaximum                        : __CODEGEN_BITFIELD(16, 21)    ; //!< Thread Depth Counter Maximum
                uint32_t                 Reserved150                                      : __CODEGEN_BITFIELD(22, 29)    ; //!< Reserved
                uint32_t                 SimdSize                                         : __CODEGEN_BITFIELD(30, 31)    ; //!< SIMD_SIZE
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 ThreadGroupIdStartingX                                                           ; //!< Thread Group ID Starting X
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 Reserved192                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 ThreadGroupIdXDimension                                                          ; //!< Thread Group ID X Dimension
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 ThreadGroupIdStartingY                                                           ; //!< Thread Group ID Starting Y
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 Reserved288                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 ThreadGroupIdYDimension                                                          ; //!< Thread Group ID Y Dimension
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 ThreadGroupIdStartingResumeZ                                                     ; //!< Thread Group ID Starting/Resume Z
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 ThreadGroupIdZDimension                                                          ; //!< Thread Group ID Z Dimension
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 RightExecutionMask                                                               ; //!< Right Execution Mask
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 BottomExecutionMask                                                              ; //!< Bottom Execution Mask
            };
            uint32_t                     Value;
        } DW14;

        //! \name Local enumerations

        enum SUBOPCODE
        {
            SUBOPCODE_GPGPUWALKERSUBOP                                       = 5, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_GPGPUWALKER                                 = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MEDIA                                                   = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_GFXPIPE                                             = 3, //!< No additional details
        };

        //! \brief SIMD_SIZE
        //! \details
        //!     This field determines the size of the payload and the number of bits of
        //!     the execution mask that are expected.  The kernel pointed to by the
        //!     interface descriptor should match the SIMD declared here.
        enum SIMD_SIZE
        {
            SIMD_SIZE_SIMD8                                                  = 0, //!< 8 LSBs of the execution mask are used
            SIMD_SIZE_SIMD16                                                 = 1, //!< 16 LSBs used in execution mask
            SIMD_SIZE_SIMD32                                                 = 2, //!< 32 bits of execution mask used
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        GPGPU_WALKER_CMD();

        static const size_t dwSize = 15;
        static const size_t byteSize = 60;
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
    struct _3DSTATE_CHROMA_KEY_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 Command3DSubOpcode                               : __CODEGEN_BITFIELD(16, 23)    ; //!< _3D_COMMAND_SUB_OPCODE
                uint32_t                 Command3DOpcode                                  : __CODEGEN_BITFIELD(24, 26)    ; //!< _3D_COMMAND_OPCODE
                uint32_t                 CommandSubtype                                   : __CODEGEN_BITFIELD(27, 28)    ; //!< COMMAND_SUBTYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0, 29)    ; //!< Reserved
                uint32_t                 ChromakeyTableIndex                              : __CODEGEN_BITFIELD(30, 31)    ; //!< ChromaKey Table Index
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 ChromakeyLowValue                                                                ; //!< ChromaKey Low Value
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
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
    //! \brief PALETTE_ENTRY
    //! \details
    //!
    //!
    struct PALETTE_ENTRY_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 Blue                                             : __CODEGEN_BITFIELD( 0,  7)    ; //!< Blue
                uint32_t                 Green                                            : __CODEGEN_BITFIELD( 8, 15)    ; //!< Green
                uint32_t                 Red                                              : __CODEGEN_BITFIELD(16, 23)    ; //!< Red
                uint32_t                 Alpha                                            : __CODEGEN_BITFIELD(24, 31)    ; //!< Alpha
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
    //! \brief STATE_SIP
    //! \details
    //!     The STATE_SIP command specifies the starting instruction location of the
    //!     System Routine that is shared by all threads in execution.
    //!
    struct STATE_SIP_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 Command3DSubOpcode                               : __CODEGEN_BITFIELD(16, 23)    ; //!< _3D_COMMAND_SUB_OPCODE
                uint32_t                 Command3DOpcode                                  : __CODEGEN_BITFIELD(24, 26)    ; //!< _3D_COMMAND_OPCODE
                uint32_t                 CommandSubtype                                   : __CODEGEN_BITFIELD(27, 28)    ; //!< COMMAND_SUBTYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1..2
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
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 Command3DSubOpcode                               : __CODEGEN_BITFIELD(16, 23)    ; //!< _3D_COMMAND_SUB_OPCODE
                uint32_t                 Command3DOpcode                                  : __CODEGEN_BITFIELD(24, 26)    ; //!< _3D_COMMAND_OPCODE
                uint32_t                 CommandSubtype                                   : __CODEGEN_BITFIELD(27, 28)    ; //!< COMMAND_SUBTYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1..2
            struct
            {
                uint64_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint64_t                 GpgpuCsrBaseAddress                              : __CODEGEN_BITFIELD(12, 63)    ; //!< GPGPU CSR Base Address
            };
            uint32_t                     Value[2];
        } DW1_2;

        //! \name Local enumerations

        enum _3D_COMMAND_SUB_OPCODE
        {
            _3D_COMMAND_SUB_OPCODE_GPGPUCSRBASEADDRESS                       = 4, //!< No additional details
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
        GPGPU_CSR_BASE_ADDRESS_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };
};

#pragma pack()

#endif  // __MHW_RENDER_HWCMD_G12_X_H__
