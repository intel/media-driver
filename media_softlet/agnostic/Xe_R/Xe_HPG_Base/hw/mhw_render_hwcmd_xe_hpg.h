/*===================== begin_copyright_notice ==================================

* Copyright (c) 2015-2020, Intel Corporation
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
//! \file     mhw_render_hwcmd_xe_hpg.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of Xe_HPG as other components
//!           should use MHW interface to interact with MHW commands and states.
//!
#ifndef __MHW_RENDER_HWCMD_XE_HPG_H__
#define __MHW_RENDER_HWCMD_XE_HPG_H__

#include "mhw_hwcmd.h"
#pragma once
#pragma pack(1)

namespace mhw
{
namespace render
{
namespace xe_hpg
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

    struct PIPELINE_SELECT_CMD
        {
            union
            {
                struct
                {
                    uint32_t PipelineSelection : __CODEGEN_BITFIELD(0, 1);                  //!< PIPELINE_SELECTION
                    uint32_t RenderSliceCommonPowerGateEnable : __CODEGEN_BITFIELD(2, 2);   //!< RENDER_SLICE_COMMON_POWER_GATE_ENABLE
                    uint32_t RenderSamplerPowerGateEnable : __CODEGEN_BITFIELD(3, 3);       //!< RENDER_SAMPLER_POWER_GATE_ENABLE
                    uint32_t MediaSamplerDopClockGateEnable : __CODEGEN_BITFIELD(4, 4);     //!< MEDIA_SAMPLER_DOP_CLOCK_GATE_ENABLE
                    uint32_t Reserved5 : __CODEGEN_BITFIELD(5, 5);                          //!< Reserved
                    uint32_t MediaSamplerPowerClockGateDisable : __CODEGEN_BITFIELD(6, 6);  //!< Media Sampler Power Clock Gate Disable
                    uint32_t SystolicModeEnable : __CODEGEN_BITFIELD(7, 7);                 //!< SYSTOLIC_MODE_ENABLE
                    uint32_t MaskBits : __CODEGEN_BITFIELD(8, 15);                          //!< Mask Bits
                    uint32_t _3DCommandSubOpcode : __CODEGEN_BITFIELD(16, 23);              //!< _3D_COMMAND_SUB_OPCODE
                    uint32_t _3DCommandOpcode : __CODEGEN_BITFIELD(24, 26);                 //!< _3D_COMMAND_OPCODE
                    uint32_t CommandSubtype : __CODEGEN_BITFIELD(27, 28);                   //!< COMMAND_SUBTYPE
                    uint32_t CommandType : __CODEGEN_BITFIELD(29, 31);                      //!< COMMAND_TYPE
                };
                uint32_t Value;
            } DW0;

            //! \name Local enumerations

            //! \brief PIPELINE_SELECTION
            //! \details
            //!     Mask bits [9:8] has to be set for HW to look at this field when
            //!     PIPELINE_SELECT command is parsed. Setting only one of the mask bit [9]
            //!     or [8] is illegal.
            enum PIPELINE_SELECTION
            {
                PIPELINE_SELECTION_3D    = 0,  //!< 3D pipeline is selected
                PIPELINE_SELECTION_MEDIA = 1,  //!< Media pipeline is selected (Includes HD optical disc playback, HD video playback, and generic media workloads)
                PIPELINE_SELECTION_GPGPU = 2,  //!< GPGPU pipeline is selected
            };

            //! \brief RENDER_SLICE_COMMON_POWER_GATE_ENABLE
            //! \details
            //!     Mask bit [10] has to be set for HW to look at this field when
            //!     PIPELINE_SELECT command is parsed.
            enum RENDER_SLICE_COMMON_POWER_GATE_ENABLE
            {
                RENDER_SLICE_COMMON_POWER_GATE_ENABLE_DISABLED = 0,  //!< Command Streamer sends message to PM to disable render slice common Power Gating.
                RENDER_SLICE_COMMON_POWER_GATE_ENABLE_ENABLED  = 1,  //!< Command Streamer sends message to PM to enable render slice common Power Gating.
            };

            //! \brief RENDER_SAMPLER_POWER_GATE_ENABLE
            //! \details
            //!     Mask bit [11] has to be set for HW to look at this field when
            //!     PIPELINE_SELECT command is parsed.
            enum RENDER_SAMPLER_POWER_GATE_ENABLE
            {
                RENDER_SAMPLER_POWER_GATE_ENABLE_DISABLED = 0,  //!< Command Streamer sends message to PM to disable render sampler Power Gating.
                RENDER_SAMPLER_POWER_GATE_ENABLE_ENABLED  = 1,  //!< Command Streamer sends message to PM to enable render sampler Power Gating.
            };

            //! \brief MEDIA_SAMPLER_DOP_CLOCK_GATE_ENABLE
            //! \details
            //!     Mask bit [12] has to be set for HW to look at this field when
            //!     PIPELINE_SELECT command is parsed.
            enum MEDIA_SAMPLER_DOP_CLOCK_GATE_ENABLE
            {
                MEDIA_SAMPLER_DOP_CLOCK_GATE_ENABLE_DISABLED = 0,  //!< Command Streamer sends message to PM to disable sampler DOP Clock Gating.
                MEDIA_SAMPLER_DOP_CLOCK_GATE_ENABLE_ENABLED  = 1,  //!< Command Streamer sends message to PM to enable media sampler DOP Clock Gating.
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
                SYSTOLIC_MODE_ENABLE_SYSTOLICMODEDISABLED = 0,  //!< No additional details
                SYSTOLIC_MODE_ENABLE_SYSTOLICMODEENABLED  = 1,  //!< No additional details
            };

            enum _3D_COMMAND_SUB_OPCODE
            {
                _3D_COMMAND_SUB_OPCODE_PIPELINESELECT = 4,  //!< No additional details
            };

            enum _3D_COMMAND_OPCODE
            {
                _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED = 1,  //!< No additional details
            };

            enum COMMAND_SUBTYPE
            {
                COMMAND_SUBTYPE_GFXPIPESINGLEDW = 1,  //!< No additional details
            };

            enum COMMAND_TYPE
            {
                COMMAND_TYPE_GFXPIPE = 3,  //!< No additional details
            };

            //! \name Initializations

            //! \brief Explicit member initialization function
            PIPELINE_SELECT_CMD()
            {
                DW0.Value = 0x69040000;
                //DW0.PipelineSelection                            = PIPELINE_SELECTION_3D;
                //DW0.RenderSliceCommonPowerGateEnable             = RENDER_SLICE_COMMON_POWER_GATE_ENABLE_DISABLED;
                //DW0.RenderSamplerPowerGateEnable                 = RENDER_SAMPLER_POWER_GATE_ENABLE_DISABLED;
                //DW0.MediaSamplerDopClockGateEnable               = MEDIA_SAMPLER_DOP_CLOCK_GATE_ENABLE_DISABLED;
                //DW0.SystolicModeEnable                           = SYSTOLIC_MODE_ENABLE_SYSTOLICMODEDISABLED;
                //DW0._3DCommandSubOpcode                          = _3D_COMMAND_SUB_OPCODE_PIPELINESELECT;
                //DW0._3DCommandOpcode                             = _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED;
                //DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPESINGLEDW;
                //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;
            }

            static const size_t dwSize   = 1;
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
        //!     addresses:
        //!
        //!     3DSTATE_CC_POINTERS
        //!
        //!     3DSTATE_BINDING_TABLE_POINTERS
        //!
        //!     3DSTATE_SAMPLER_STATE_POINTERS
        //!
        //!     3DSTATE_VIEWPORT_STATE_POINTERS
        //!
        //!
        //!
        //!
        //!
        //!     Execution of this command causes a full pipeline flush, thus its use
        //!     should be minimized for higher performance.
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
        //!     SW must always program PIPE_CONTROL command with "HDC Pipleine FLush"
        //!     set prior to programming of STATE_BASE_ADDRESS command for GPGPU/Media
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
                    uint32_t DwordLength : __CODEGEN_BITFIELD(0, 7);           //!< DWORD_LENGTH
                    uint32_t Reserved8 : __CODEGEN_BITFIELD(8, 15);            //!< Reserved
                    uint32_t Command3DSubOpcode : __CODEGEN_BITFIELD(16, 23);  //!< _3D_COMMAND_SUB_OPCODE
                    uint32_t Command3DOpcode : __CODEGEN_BITFIELD(24, 26);     //!< _3D_COMMAND_OPCODE
                    uint32_t CommandSubtype : __CODEGEN_BITFIELD(27, 28);      //!< COMMAND_SUBTYPE
                    uint32_t CommandType : __CODEGEN_BITFIELD(29, 31);         //!< COMMAND_TYPE
                };
                uint32_t Value;
            } DW0;
            union
            {
                struct
                {
                    uint64_t GeneralStateBaseAddressModifyEnable : __CODEGEN_BITFIELD(0, 0);    //!< GENERAL_STATE_BASE_ADDRESS_MODIFY_ENABLE
                    uint64_t Reserved33 : __CODEGEN_BITFIELD(1, 3);                             //!< Reserved
                    uint64_t GeneralStateMemoryObjectControlState : __CODEGEN_BITFIELD(4, 10);  //!< General State Memory Object Control State
                    uint64_t Reserved43 : __CODEGEN_BITFIELD(11, 11);                           //!< Reserved
                    uint64_t GeneralStateBaseAddress : __CODEGEN_BITFIELD(12, 63);              //!< General State Base Address
                };
                uint32_t Value[2];
            } DW1_2;
            union
            {
                struct
                {
                    uint32_t CoherencySettingModifyEnable : __CODEGEN_BITFIELD(0, 0);                                  //!< COHERENCY_SETTING_MODIFY_ENABLE
                    uint32_t Reserved97 : __CODEGEN_BITFIELD(1, 12);                                                   //!< Reserved
                    uint32_t EnableMemoryCompressionForAllStatelessAccesses : __CODEGEN_BITFIELD(13, 13);              //!< ENABLE_MEMORY_COMPRESSION_FOR_ALL_STATELESS_ACCESSES
                    uint32_t DisableSupportForMultiGpuAtomicsForStatelessAccesses : __CODEGEN_BITFIELD(14, 14);        //!< DISABLE_SUPPORT_FOR_MULTI_GPU_ATOMICS_FOR_STATELESS_ACCESSES
                    uint32_t DisableSupportForMultiGpuPartialWritesForStatelessMessages : __CODEGEN_BITFIELD(15, 15);  //!< DISABLE_SUPPORT_FOR_MULTI_GPU_PARTIAL_WRITES_FOR_STATELESS_MESSAGES
                    uint32_t StatelessDataPortAccessMemoryObjectControlState : __CODEGEN_BITFIELD(16, 22);             //!< Stateless Data Port Access Memory Object Control State
                    uint32_t L1CachePolicy : __CODEGEN_BITFIELD(23, 25);                                               //!< L1 Cache Policy for stateless accesses
                    uint32_t Reserved119 : __CODEGEN_BITFIELD(26, 31);                                                 //!< Reserved
                };
                uint32_t Value;
            } DW3;
            union
            {
                struct
                {
                    uint64_t SurfaceStateBaseAddressModifyEnable : __CODEGEN_BITFIELD(0, 0);    //!< SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE
                    uint64_t Reserved129 : __CODEGEN_BITFIELD(1, 3);                            //!< Reserved
                    uint64_t SurfaceStateMemoryObjectControlState : __CODEGEN_BITFIELD(4, 10);  //!< Surface State Memory Object Control State
                    uint64_t Reserved139 : __CODEGEN_BITFIELD(11, 11);                          //!< Reserved
                    uint64_t SurfaceStateBaseAddress : __CODEGEN_BITFIELD(12, 63);              //!< Surface State Base Address
                };
                uint32_t Value[2];
            } DW4_5;
            union
            {
                struct
                {
                    uint64_t DynamicStateBaseAddressModifyEnable : __CODEGEN_BITFIELD(0, 0);    //!< DYNAMIC_STATE_BASE_ADDRESS_MODIFY_ENABLE
                    uint64_t Reserved193 : __CODEGEN_BITFIELD(1, 3);                            //!< Reserved
                    uint64_t DynamicStateMemoryObjectControlState : __CODEGEN_BITFIELD(4, 10);  //!< Dynamic State Memory Object Control State
                    uint64_t Reserved203 : __CODEGEN_BITFIELD(11, 11);                          //!< Reserved
                    uint64_t DynamicStateBaseAddress : __CODEGEN_BITFIELD(12, 63);              //!< Dynamic State Base Address
                };
                uint32_t Value[2];
            } DW6_7;
            union
            {
                struct
                {
                    uint64_t IndirectObjectBaseAddressModifyEnable : __CODEGEN_BITFIELD(0, 0);    //!< INDIRECT_OBJECT_BASE_ADDRESS_MODIFY_ENABLE
                    uint64_t Reserved257 : __CODEGEN_BITFIELD(1, 3);                              //!< Reserved
                    uint64_t IndirectObjectMemoryObjectControlState : __CODEGEN_BITFIELD(4, 10);  //!< Indirect Object Memory Object Control State
                    uint64_t Reserved267 : __CODEGEN_BITFIELD(11, 11);                            //!< Reserved
                    uint64_t IndirectObjectBaseAddress : __CODEGEN_BITFIELD(12, 63);              //!< Indirect Object Base Address
                };
                uint32_t Value[2];
            } DW8_9;
            union
            {
                struct
                {
                    uint64_t InstructionBaseAddressModifyEnable : __CODEGEN_BITFIELD(0, 0);    //!< INSTRUCTION_BASE_ADDRESS_MODIFY_ENABLE
                    uint64_t Reserved321 : __CODEGEN_BITFIELD(1, 3);                           //!< Reserved
                    uint64_t InstructionMemoryObjectControlState : __CODEGEN_BITFIELD(4, 10);  //!< Instruction Memory Object Control State
                    uint64_t Reserved331 : __CODEGEN_BITFIELD(11, 11);                         //!< Reserved
                    uint64_t InstructionBaseAddress : __CODEGEN_BITFIELD(12, 63);              //!< Instruction Base Address
                };
                uint32_t Value[2];
            } DW10_11;
            union
            {
                struct
                {
                    uint32_t GeneralStateBufferSizeModifyEnable : __CODEGEN_BITFIELD(0, 0);  //!< GENERAL_STATE_BUFFER_SIZE_MODIFY_ENABLE
                    uint32_t Reserved385 : __CODEGEN_BITFIELD(1, 11);                        //!< Reserved
                    uint32_t GeneralStateBufferSize : __CODEGEN_BITFIELD(12, 31);            //!< General State Buffer Size
                };
                uint32_t Value;
            } DW12;
            union
            {
                struct
                {
                    uint32_t DynamicStateBufferSizeModifyEnable : __CODEGEN_BITFIELD(0, 0);  //!< DYNAMIC_STATE_BUFFER_SIZE_MODIFY_ENABLE
                    uint32_t Reserved417 : __CODEGEN_BITFIELD(1, 11);                        //!< Reserved
                    uint32_t DynamicStateBufferSize : __CODEGEN_BITFIELD(12, 31);            //!< Dynamic State Buffer Size
                };
                uint32_t Value;
            } DW13;
            union
            {
                struct
                {
                    uint32_t IndirectObjectBufferSizeModifyEnable : __CODEGEN_BITFIELD(0, 0);  //!< INDIRECT_OBJECT_BUFFER_SIZE_MODIFY_ENABLE
                    uint32_t Reserved449 : __CODEGEN_BITFIELD(1, 11);                          //!< Reserved
                    uint32_t IndirectObjectBufferSize : __CODEGEN_BITFIELD(12, 31);            //!< Indirect Object Buffer Size
                };
                uint32_t Value;
            } DW14;
            union
            {
                struct
                {
                    uint32_t InstructionBufferSizeModifyEnable : __CODEGEN_BITFIELD(0, 0);  //!< INSTRUCTION_BUFFER_SIZE_MODIFY_ENABLE
                    uint32_t Reserved481 : __CODEGEN_BITFIELD(1, 11);                       //!< Reserved
                    uint32_t InstructionBufferSize : __CODEGEN_BITFIELD(12, 31);            //!< Instruction Buffer Size
                };
                uint32_t Value;
            } DW15;
            union
            {
                struct
                {
                    uint64_t BindlessSurfaceStateBaseAddressModifyEnable : __CODEGEN_BITFIELD(0, 0);    //!< BINDLESS_SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE
                    uint64_t Reserved513 : __CODEGEN_BITFIELD(1, 3);                                    //!< Reserved
                    uint64_t BindlessSurfaceStateMemoryObjectControlState : __CODEGEN_BITFIELD(4, 10);  //!< Bindless Surface State Memory Object Control State
                    uint64_t Reserved523 : __CODEGEN_BITFIELD(11, 11);                                  //!< Reserved
                    uint64_t BindlessSurfaceStateBaseAddress : __CODEGEN_BITFIELD(12, 63);              //!< Bindless Surface State Base Address
                };
                uint32_t Value[2];
            } DW16_17;
            union
            {
                struct
                {
                    uint32_t BindlessSurfaceStateSize;  //!< Bindless Surface State Size
                };
                uint32_t Value;
            } DW18;
            union
            {
                struct
                {
                    uint64_t BindlessSamplerStateBaseAddressModifyEnable : __CODEGEN_BITFIELD(0, 0);    //!< BINDLESS_SAMPLER_STATE_BASE_ADDRESS_MODIFY_ENABLE
                    uint64_t Reserved609 : __CODEGEN_BITFIELD(1, 3);                                    //!< Reserved
                    uint64_t BindlessSamplerStateMemoryObjectControlState : __CODEGEN_BITFIELD(4, 10);  //!< Bindless Sampler State Memory Object Control State
                    uint64_t Reserved619 : __CODEGEN_BITFIELD(11, 11);                                  //!< Reserved
                    uint64_t BindlessSamplerStateBaseAddress : __CODEGEN_BITFIELD(12, 63);              //!< Bindless Sampler State Base Address
                };
                uint32_t Value[2];
            } DW19_20;
            union
            {
                struct
                {
                    uint32_t Reserved672 : __CODEGEN_BITFIELD(0, 11);                      //!< Reserved
                    uint32_t BindlessSamplerStateBufferSize : __CODEGEN_BITFIELD(12, 31);  //!< Bindless Sampler State Buffer Size
                };
                uint32_t Value;
            } DW21;

            //! \name Local enumerations

            enum _3D_COMMAND_SUB_OPCODE
            {
                _3D_COMMAND_SUB_OPCODE_STATEBASEADDRESS = 1,  //!< No additional details
            };

            enum _3D_COMMAND_OPCODE
            {
                _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED = 1,  //!< No additional details
            };

            enum COMMAND_SUBTYPE
            {
                COMMAND_SUBTYPE_GFXPIPECOMMON = 0,  //!< No additional details
            };

            enum COMMAND_TYPE
            {
                COMMAND_TYPE_GFXPIPE = 3,  //!< No additional details
            };

            //! \brief GENERAL_STATE_BASE_ADDRESS_MODIFY_ENABLE
            //! \details
            //!     The other fields in this DWord and the following DWord are updated
            //!     only when this bit is set.
            enum GENERAL_STATE_BASE_ADDRESS_MODIFY_ENABLE
            {
                GENERAL_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE = 0,  //!< Ignore the updated address.
                GENERAL_STATE_BASE_ADDRESS_MODIFY_ENABLE_ENABLE  = 1,  //!< Modify the address.
            };

            //! \brief COHERENCY_SETTING_MODIFY_ENABLE
            //! \details
            //!     All the fields in this DW is only updated when this bit is set.
            enum COHERENCY_SETTING_MODIFY_ENABLE
            {
                COHERENCY_SETTING_MODIFY_ENABLE_DISABLEWRITETOTHISDW = 0,  //!< No additional details
                COHERENCY_SETTING_MODIFY_ENABLE_ENABLEWRITETOTHISDW  = 1,  //!< No additional details
            };

            //! \brief ENABLE_MEMORY_COMPRESSION_FOR_ALL_STATELESS_ACCESSES
            //! \details
            //!     Enable compression for stateless memory accesses.
            enum ENABLE_MEMORY_COMPRESSION_FOR_ALL_STATELESS_ACCESSES
            {
                ENABLE_MEMORY_COMPRESSION_FOR_ALL_STATELESS_ACCESSES_DISABLED = 0,  //!< No additional details
                ENABLE_MEMORY_COMPRESSION_FOR_ALL_STATELESS_ACCESSES_ENABLED  = 1,  //!< No additional details
            };

            //! \brief DISABLE_SUPPORT_FOR_MULTI_GPU_ATOMICS_FOR_STATELESS_ACCESSES
            //! \details
            //!     Specifies whether sequential consistency of atomic memory operations are
            //!     supported across multiple GPUs.
            enum DISABLE_SUPPORT_FOR_MULTI_GPU_ATOMICS_FOR_STATELESS_ACCESSES
            {
                DISABLE_SUPPORT_FOR_MULTI_GPU_ATOMICS_FOR_STATELESS_ACCESSES_ENABLE  = 0,  //!< Atomic memory operations from all GPUs to the same address is sequentially consistent.
                DISABLE_SUPPORT_FOR_MULTI_GPU_ATOMICS_FOR_STATELESS_ACCESSES_DISABLE = 1,  //!< Disable multi-GPU Atomic consistency. Atomic memory operations to the same address is sequentially consistent only if the operations are from the same GPU.
            };

            //! \brief DISABLE_SUPPORT_FOR_MULTI_GPU_PARTIAL_WRITES_FOR_STATELESS_MESSAGES
            //! \details
            //!     Specifies whether data-consistency on partial memory write operations
            //!     are supported across multiple GPUs.
            enum DISABLE_SUPPORT_FOR_MULTI_GPU_PARTIAL_WRITES_FOR_STATELESS_MESSAGES
            {
                DISABLE_SUPPORT_FOR_MULTI_GPU_PARTIAL_WRITES_FOR_STATELESS_MESSAGES_ENABLED  = 0,  //!< Enable data consistency on multi-GPU partial memory writes.
                DISABLE_SUPPORT_FOR_MULTI_GPU_PARTIAL_WRITES_FOR_STATELESS_MESSAGES_DISABLED = 1,  //!< Disable data consistency on multi-GPU partial memory writes. If multiple GPUs write different bytes of the same cacheline, the data may be corrupted.
            };

            //! \brief SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE
            //! \details
            //!     The other fields in this DWord and the following DWord are updated only
            //!     when this bit is set.
            enum SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE
            {
                SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE = 0,  //!< Ignore the updated address.
                SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE_ENABLE  = 1,  //!< Modify the address.
            };

            //! \brief DYNAMIC_STATE_BASE_ADDRESS_MODIFY_ENABLE
            //! \details
            //!     The other fields in this DWord and the following DWord are updated only
            //!     when this bit is set.
            enum DYNAMIC_STATE_BASE_ADDRESS_MODIFY_ENABLE
            {
                DYNAMIC_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE = 0,  //!< Ignore the updated address.
                DYNAMIC_STATE_BASE_ADDRESS_MODIFY_ENABLE_ENABLE  = 1,  //!< Modify the address.
            };

            //! \brief INDIRECT_OBJECT_BASE_ADDRESS_MODIFY_ENABLE
            //! \details
            //!     The other fields in this DWord and the following DWord are updated
            //!     only when this bit is set.
            enum INDIRECT_OBJECT_BASE_ADDRESS_MODIFY_ENABLE
            {
                INDIRECT_OBJECT_BASE_ADDRESS_MODIFY_ENABLE_DISABLE = 0,  //!< Ignore the updated address.
                INDIRECT_OBJECT_BASE_ADDRESS_MODIFY_ENABLE_ENABLE  = 1,  //!< Modify the address.
            };

            //! \brief INSTRUCTION_BASE_ADDRESS_MODIFY_ENABLE
            //! \details
            //!     The other fields in this DWord and the following DWord are updated
            //!     only when this bit is set.
            enum INSTRUCTION_BASE_ADDRESS_MODIFY_ENABLE
            {
                INSTRUCTION_BASE_ADDRESS_MODIFY_ENABLE_DISABLE = 0,  //!< Ignore the updated address.
                INSTRUCTION_BASE_ADDRESS_MODIFY_ENABLE_ENABLE  = 1,  //!< Modify the address.
            };

            //! \brief GENERAL_STATE_BUFFER_SIZE_MODIFY_ENABLE
            //! \details
            //!     The bound in this DWord is updated only when this bit is set.
            enum GENERAL_STATE_BUFFER_SIZE_MODIFY_ENABLE
            {
                GENERAL_STATE_BUFFER_SIZE_MODIFY_ENABLE_DISABLE = 0,  //!< Ignore the updated bound.
                GENERAL_STATE_BUFFER_SIZE_MODIFY_ENABLE_ENABLE  = 1,  //!< Modify the updated bound.
            };

            //! \brief DYNAMIC_STATE_BUFFER_SIZE_MODIFY_ENABLE
            //! \details
            //!     FormatDesc
            enum DYNAMIC_STATE_BUFFER_SIZE_MODIFY_ENABLE
            {
                DYNAMIC_STATE_BUFFER_SIZE_MODIFY_ENABLE_DISABLE = 0,  //!< Ignore the updated bound.
                DYNAMIC_STATE_BUFFER_SIZE_MODIFY_ENABLE_ENABLE  = 1,  //!< Modify the updated bound.
            };

            //! \brief INDIRECT_OBJECT_BUFFER_SIZE_MODIFY_ENABLE
            //! \details
            //!     FormatDesc
            enum INDIRECT_OBJECT_BUFFER_SIZE_MODIFY_ENABLE
            {
                INDIRECT_OBJECT_BUFFER_SIZE_MODIFY_ENABLE_DISABLE = 0,  //!< Ignore the updated bound.
                INDIRECT_OBJECT_BUFFER_SIZE_MODIFY_ENABLE_ENABLE  = 1,  //!< Modify the updated bound.
            };

            //! \brief INSTRUCTION_BUFFER_SIZE_MODIFY_ENABLE
            //! \details
            //!     FormatDesc
            enum INSTRUCTION_BUFFER_SIZE_MODIFY_ENABLE
            {
                INSTRUCTION_BUFFER_SIZE_MODIFY_ENABLE_DISABLE = 0,  //!< Ignore the updated bound.
            };

            //! \brief BINDLESS_SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE
            //! \details
            //!     The other fields in this DWord and the following two DWords are
            //!     updated only when this bit is set.
            enum BINDLESS_SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE
            {
                BINDLESS_SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE = 0,  //!< Ignore the updated address
                BINDLESS_SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE_ENABLE  = 1,  //!< Modify the address
            };

            //! \brief BINDLESS_SAMPLER_STATE_BASE_ADDRESS_MODIFY_ENABLE
            //! \details
            //!     The other fields in this DWord and the following two DWords are
            //!     updated only when this bit is set.
            enum BINDLESS_SAMPLER_STATE_BASE_ADDRESS_MODIFY_ENABLE
            {
                BINDLESS_SAMPLER_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE = 0,  //!< Ignore the updated address
                BINDLESS_SAMPLER_STATE_BASE_ADDRESS_MODIFY_ENABLE_ENABLE  = 1,  //!< Modify the address
            };

            //! \name Initializations

            //! \brief Explicit member initialization function
            STATE_BASE_ADDRESS_CMD()
            {
                DW0.Value = 0x61010014;
                //DW0.DwordLength                                  = GetOpLength(dwSize);
                //DW0.Command3DSubOpcode                          = _3D_COMMAND_SUB_OPCODE_STATEBASEADDRESS;
                //DW0.Command3DOpcode                             = _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED;
                //DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPECOMMON;
                //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

                DW1_2.Value[0] = DW1_2.Value[1] = 0x00000000;
                //DW1_2.GeneralStateBaseAddressModifyEnable        = GENERAL_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

                DW3.Value = 0x00000000;
                //DW3.CoherencySettingModifyEnable                 = COHERENCY_SETTING_MODIFY_ENABLE_DISABLEWRITETOTHISDW;
                //DW3.EnableMemoryCompressionForAllStatelessAccesses = ENABLE_MEMORY_COMPRESSION_FOR_ALL_STATELESS_ACCESSES_DISABLED;
                //DW3.DisableSupportForMultiGpuAtomicsForStatelessAccesses = DISABLE_SUPPORT_FOR_MULTI_GPU_ATOMICS_FOR_STATELESS_ACCESSES_ENABLE;

                //To improve performance disable support for MultipleGpu sync features
                //Current kernels don't support cross tiles writes
                DW3.DisableSupportForMultiGpuAtomicsForStatelessAccesses       = DISABLE_SUPPORT_FOR_MULTI_GPU_ATOMICS_FOR_STATELESS_ACCESSES_DISABLE;
                DW3.DisableSupportForMultiGpuPartialWritesForStatelessMessages = DISABLE_SUPPORT_FOR_MULTI_GPU_PARTIAL_WRITES_FOR_STATELESS_MESSAGES_DISABLED;

                DW4_5.Value[0] = DW4_5.Value[1] = 0x00000000;
                //DW4_5.SurfaceStateBaseAddressModifyEnable        = SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

                DW6_7.Value[0] = DW6_7.Value[1] = 0x00000000;
                //DW6_7.DynamicStateBaseAddressModifyEnable        = DYNAMIC_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

                DW8_9.Value[0] = DW8_9.Value[1] = 0x00000000;
                //DW8_9.IndirectObjectBaseAddressModifyEnable      = INDIRECT_OBJECT_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

                DW10_11.Value[0] = DW10_11.Value[1] = 0x00000000;
                //DW10_11.InstructionBaseAddressModifyEnable       = INSTRUCTION_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

                DW12.Value = 0x00000000;
                //DW12.GeneralStateBufferSizeModifyEnable          = GENERAL_STATE_BUFFER_SIZE_MODIFY_ENABLE_DISABLE;

                DW13.Value = 0x00000000;
                //DW13.DynamicStateBufferSizeModifyEnable          = DYNAMIC_STATE_BUFFER_SIZE_MODIFY_ENABLE_DISABLE;

                DW14.Value = 0x00000000;
                //DW14.IndirectObjectBufferSizeModifyEnable        = INDIRECT_OBJECT_BUFFER_SIZE_MODIFY_ENABLE_DISABLE;

                DW15.Value = 0x00000000;
                //DW15.InstructionBufferSizeModifyEnable           = INSTRUCTION_BUFFER_SIZE_MODIFY_ENABLE_DISABLE;

                DW16_17.Value[0] = DW16_17.Value[1] = 0x00000000;
                //DW16_17.BindlessSurfaceStateBaseAddressModifyEnable = BINDLESS_SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

                DW18.Value = 0x00000000;

                DW19_20.Value[0] = DW19_20.Value[1] = 0x00000000;
                //DW19_20.BindlessSamplerStateBaseAddressModifyEnable = BINDLESS_SAMPLER_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

                DW21.Value = 0x00000000;
            }

            static const size_t dwSize   = 22;
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
        struct _3DSTATE_CHROMA_KEY_CMD
        {
            union
            {
                struct
                {
                    uint32_t DwordLength : __CODEGEN_BITFIELD(0, 7);            //!< DWORD_LENGTH
                    uint32_t Reserved8 : __CODEGEN_BITFIELD(8, 15);             //!< Reserved
                    uint32_t _3DCommandSubOpcode : __CODEGEN_BITFIELD(16, 23);  //!< _3D_COMMAND_SUB_OPCODE
                    uint32_t _3DCommandOpcode : __CODEGEN_BITFIELD(24, 26);     //!< _3D_COMMAND_OPCODE
                    uint32_t CommandSubtype : __CODEGEN_BITFIELD(27, 28);       //!< COMMAND_SUBTYPE
                    uint32_t CommandType : __CODEGEN_BITFIELD(29, 31);          //!< COMMAND_TYPE
                };
                uint32_t Value;
            } DW0;
            union
            {
                struct
                {
                    uint32_t Reserved32 : __CODEGEN_BITFIELD(0, 29);            //!< Reserved
                    uint32_t ChromakeyTableIndex : __CODEGEN_BITFIELD(30, 31);  //!< ChromaKey Table Index
                };
                uint32_t Value;
            } DW1;
            union
            {
                struct
                {
                    uint32_t ChromakeyLowValue;  //!< ChromaKey Low Value
                };
                uint32_t Value;
            } DW2;
            union
            {
                struct
                {
                    uint32_t ChromakeyHighValue;  //!< ChromaKey High Value
                };
                uint32_t Value;
            } DW3;

            //! \name Local enumerations

            enum _3D_COMMAND_SUB_OPCODE
            {
                _3D_COMMAND_SUB_OPCODE_3DSTATECHROMAKEY = 4,  //!< No additional details
            };

            enum _3D_COMMAND_OPCODE
            {
                _3D_COMMAND_OPCODE_3DSTATENONPIPELINED = 1,  //!< No additional details
            };

            enum COMMAND_SUBTYPE
            {
                COMMAND_SUBTYPE_GFXPIPE3D = 3,  //!< No additional details
            };

            enum COMMAND_TYPE
            {
                COMMAND_TYPE_GFXPIPE = 3,  //!< No additional details
            };

            //! \name Initializations

            //! \brief Explicit member initialization function
            _3DSTATE_CHROMA_KEY_CMD()
            {
                DW0.Value = 0x79040002;
                //DW0.DwordLength                                  = GetOpLength(dwSize);
                //DW0._3DCommandSubOpcode                          = _3D_COMMAND_SUB_OPCODE_3DSTATECHROMAKEY;
                //DW0._3DCommandOpcode                             = _3D_COMMAND_OPCODE_3DSTATENONPIPELINED;
                //DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPE3D;
                //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

                DW1.Value = 0x00000000;

                DW2.Value = 0x00000000;

                DW3.Value = 0x00000000;
            }

            static const size_t dwSize   = 4;
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
                struct
                {
                    uint32_t Blue : __CODEGEN_BITFIELD(0, 7);     //!< Blue
                    uint32_t Green : __CODEGEN_BITFIELD(8, 15);   //!< Green
                    uint32_t Red : __CODEGEN_BITFIELD(16, 23);    //!< Red
                    uint32_t Alpha : __CODEGEN_BITFIELD(24, 31);  //!< Alpha
                };
                uint32_t Value;
            } DW0;

            //! \name Local enumerations

            //! \name Initializations

            //! \brief Explicit member initialization function
            PALETTE_ENTRY_CMD()
            {
                DW0.Value = 0x00000000;
            }

            static const size_t dwSize   = 1;
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
                struct
                {
                    uint32_t DwordLength : __CODEGEN_BITFIELD(0, 7);            //!< DWORD_LENGTH
                    uint32_t Reserved8 : __CODEGEN_BITFIELD(8, 15);             //!< Reserved
                    uint32_t _3DCommandSubOpcode : __CODEGEN_BITFIELD(16, 23);  //!< _3D_COMMAND_SUB_OPCODE
                    uint32_t _3DCommandOpcode : __CODEGEN_BITFIELD(24, 26);     //!< _3D_COMMAND_OPCODE
                    uint32_t CommandSubtype : __CODEGEN_BITFIELD(27, 28);       //!< COMMAND_SUBTYPE
                    uint32_t CommandType : __CODEGEN_BITFIELD(29, 31);          //!< COMMAND_TYPE
                };
                uint32_t Value;
            } DW0;
            union
            {
                struct
                {
                    uint64_t Reserved32 : __CODEGEN_BITFIELD(0, 3);                 //!< Reserved
                    uint64_t SystemInstructionPointer : __CODEGEN_BITFIELD(4, 63);  //!< System Instruction Pointer
                };
                uint32_t Value[2];
            } DW1_2;

            //! \name Local enumerations

            enum _3D_COMMAND_SUB_OPCODE
            {
                _3D_COMMAND_SUB_OPCODE_STATESIP = 2,  //!< No additional details
            };

            enum _3D_COMMAND_OPCODE
            {
                _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED = 1,  //!< No additional details
            };

            enum COMMAND_SUBTYPE
            {
                COMMAND_SUBTYPE_GFXPIPECOMMON = 0,  //!< No additional details
            };

            enum COMMAND_TYPE
            {
                COMMAND_TYPE_GFXPIPE = 3,  //!< No additional details
            };

            //! \name Initializations

            //! \brief Explicit member initialization function
            STATE_SIP_CMD()
            {
                DW0.Value = 0x61020001;
                //DW0.DwordLength                                  = GetOpLength(dwSize);
                //DW0._3DCommandSubOpcode                          = _3D_COMMAND_SUB_OPCODE_STATESIP;
                //DW0._3DCommandOpcode                             = _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED;
                //DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPECOMMON;
                //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

                DW1_2.Value[0] = DW1_2.Value[1] = 0x00000000;
            }

            static const size_t dwSize   = 3;
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
                struct
                {
                    uint32_t DwordLength : __CODEGEN_BITFIELD(0, 7);            //!< DWORD_LENGTH
                    uint32_t Reserved8 : __CODEGEN_BITFIELD(8, 15);             //!< Reserved
                    uint32_t _3DCommandSubOpcode : __CODEGEN_BITFIELD(16, 23);  //!< _3D_COMMAND_SUB_OPCODE
                    uint32_t _3DCommandOpcode : __CODEGEN_BITFIELD(24, 26);     //!< _3D_COMMAND_OPCODE
                    uint32_t CommandSubtype : __CODEGEN_BITFIELD(27, 28);       //!< COMMAND_SUBTYPE
                    uint32_t CommandType : __CODEGEN_BITFIELD(29, 31);          //!< COMMAND_TYPE
                };
                uint32_t Value;
            } DW0;
            union
            {
                struct
                {
                    uint64_t Reserved32 : __CODEGEN_BITFIELD(0, 11);            //!< Reserved
                    uint64_t GpgpuCsrBaseAddress : __CODEGEN_BITFIELD(12, 63);  //!< GPGPU CSR Base Address
                };
                uint32_t Value[2];
            } DW1_2;

            //! \name Local enumerations

            enum _3D_COMMAND_SUB_OPCODE
            {
                _3D_COMMAND_SUB_OPCODE_GPGPUCSRBASEADDRESS = 4,  //!< No additional details
            };

            enum _3D_COMMAND_OPCODE
            {
                _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED = 1,  //!< No additional details
            };

            enum COMMAND_SUBTYPE
            {
                COMMAND_SUBTYPE_GFXPIPECOMMON = 0,  //!< No additional details
            };

            enum COMMAND_TYPE
            {
                COMMAND_TYPE_GFXPIPE = 3,  //!< No additional details
            };

            //! \name Initializations

            //! \brief Explicit member initialization function
            GPGPU_CSR_BASE_ADDRESS_CMD()
            {
                DW0.Value = 0x61040001;
                //DW0.DwordLength                                  = GetOpLength(dwSize);
                //DW0._3DCommandSubOpcode                          = _3D_COMMAND_SUB_OPCODE_GPGPUCSRBASEADDRESS;
                //DW0._3DCommandOpcode                             = _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED;
                //DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPECOMMON;
                //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

                DW1_2.Value[0] = DW1_2.Value[1] = 0x00000000;
            }
            static const size_t dwSize   = 3;
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
                    uint32_t DwordLength : __CODEGEN_BITFIELD(0, 7);            //!< DWORD_LENGTH
                    uint32_t Reserved8 : __CODEGEN_BITFIELD(8, 15);             //!< Reserved
                    uint32_t _3DCommandSubOpcode : __CODEGEN_BITFIELD(16, 23);  //!< _3D_COMMAND_SUB_OPCODE
                    uint32_t _3DCommandOpcode : __CODEGEN_BITFIELD(24, 26);     //!< _3D_COMMAND_OPCODE
                    uint32_t CommandSubtype : __CODEGEN_BITFIELD(27, 28);       //!< COMMAND_SUBTYPE
                    uint32_t CommandType : __CODEGEN_BITFIELD(29, 31);          //!< COMMAND_TYPE
                };
                uint32_t Value;
            } DW0;
            union
            {
                struct
                {
                    uint64_t SurfaceObjectControlState : __CODEGEN_BITFIELD(0, 6);      //!< Surface Object Control State
                    uint64_t Reserved39 : __CODEGEN_BITFIELD(7, 11);                    //!< Reserved
                    uint64_t BindingTablePoolBaseAddress : __CODEGEN_BITFIELD(12, 63);  //!< Binding Table Pool Base Address
                };
                uint32_t Value[2];
            } DW1_2;
            union
            {
                struct
                {
                    uint32_t Reserved96 : __CODEGEN_BITFIELD(0, 11);                   //!< Reserved
                    uint32_t BindingTablePoolBufferSize : __CODEGEN_BITFIELD(12, 31);  //!< BINDING_TABLE_POOL_BUFFER_SIZE
                };
                uint32_t Value;
            } DW3;

            //! \name Local enumerations

            enum _3D_COMMAND_SUB_OPCODE
            {
                _3D_COMMAND_SUB_OPCODE_3DSTATEBINDINGTABLEPOOLALLOC = 25,  //!< No additional details
            };

            enum _3D_COMMAND_OPCODE
            {
                _3D_COMMAND_OPCODE_3DSTATENONPIPELINED = 1,  //!< No additional details
            };

            enum COMMAND_SUBTYPE
            {
                COMMAND_SUBTYPE_GFXPIPE3D = 3,  //!< No additional details
            };

            enum COMMAND_TYPE
            {
                COMMAND_TYPE_GFXPIPE = 3,  //!< No additional details
            };

            //! \brief BINDING_TABLE_POOL_BUFFER_SIZE
            //! \details
            //!     This field specifies the size of the buffer in 4K pages. Any access
            //!     which straddle or go past the end of the buffer will return 0.
            enum BINDING_TABLE_POOL_BUFFER_SIZE
            {
                BINDING_TABLE_POOL_BUFFER_SIZE_NOVALIDDATA = 0,  //!< There is no valid data in the buffer
            };

            //! \name Initializations

            //! \brief Explicit member initialization function
            _3DSTATE_BINDING_TABLE_POOL_ALLOC_CMD()
            {
                DW0.Value = 0x79190002;
                //DW0.DwordLength                                  = GetOpLength(dwSize);
                //DW0._3DCommandSubOpcode                          = _3D_COMMAND_SUB_OPCODE_3DSTATEBINDINGTABLEPOOLALLOC;
                //DW0._3DCommandOpcode                             = _3D_COMMAND_OPCODE_3DSTATENONPIPELINED;
                //DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPE3D;
                //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

                DW1_2.Value[0] = DW1_2.Value[1] = 0x00000000;

                DW3.Value = 0x00000000;
                //DW3.BindingTablePoolBufferSize                   = BINDING_TABLE_POOL_BUFFER_SIZE_NOVALIDDATA;
            }

            static const size_t dwSize   = 4;
            static const size_t byteSize = 16;
        };

        //!
        //! \brief CFE_STATE
        //! \details
        //!
        //!
        struct CFE_STATE_CMD
        {
            union
            {
                //!< DWORD 0
                struct
                {
                    uint32_t DwordLength : __CODEGEN_BITFIELD(0, 7);             //!< DWord Length
                    uint32_t Reserved8 : __CODEGEN_BITFIELD(8, 15);              //!< Reserved
                    uint32_t CFESubOpcodeVariant : __CODEGEN_BITFIELD(16, 17);   //!< CFE SubOpcode Variant
                    uint32_t CFESubOpcode : __CODEGEN_BITFIELD(18, 23);          //!< CFE SubOpcode
                    uint32_t ComputeCommandOpcode : __CODEGEN_BITFIELD(24, 26);  //!< Compute Command Opcode
                    uint32_t Pipeline : __CODEGEN_BITFIELD(27, 28);              //!< Pipeline
                    uint32_t CommandType : __CODEGEN_BITFIELD(29, 31);           //!< Command Type
                };
                uint32_t Value = 0;
            } DW0;
            union
            {
                //!< DWORD 1_2
                struct
                {
                    uint64_t Reserved32 : __CODEGEN_BITFIELD(0, 9);            //!< Reserved
                    uint64_t ScratchSpaceBuffer : __CODEGEN_BITFIELD(10, 31);  //!< Scratch Space Base Pointer
                    uint64_t Reserved64 : __CODEGEN_BITFIELD(32, 63);          //!< Reserved
                };
                uint64_t Value = 0;
            } DW1_2;
            union
            {
                //!< DWORD 3
                struct
                {
                    uint32_t Reserved96 : __CODEGEN_BITFIELD(0, 2);                    //!< Reserved
                    uint32_t NumberOfWalkers : __CODEGEN_BITFIELD(3, 5);               //!< Number Of Walkers
                    uint32_t FusedEuDispatch : __CODEGEN_BITFIELD(6, 6);               //!< Fused EU Dispatch
                    uint32_t Reserved103 : __CODEGEN_BITFIELD(7, 12);                  //!< Reserved
                    uint32_t SingleSliceDispatchCcsMode : __CODEGEN_BITFIELD(13, 13);  //!< Single Slice Dispatch CCS Mode
                    uint32_t OverDispatchControl : __CODEGEN_BITFIELD(14, 15);         //!< Over Dispatch Control
                    uint32_t MaximumNumberOfThreads : __CODEGEN_BITFIELD(16, 31);      //!< Maximum Number of Threads
                };
                uint32_t Value = 0;
            } DW3;
            union
            {
                //!< DWORD 4
                struct
                {
                    uint32_t Reserved128 : __CODEGEN_BITFIELD(0, 23);               //!< Reserved
                    uint32_t StopAndDrainTimer : __CODEGEN_BITFIELD(24, 30);        //!< Stop And Drain Timer
                    uint32_t StopAndDrainTimerEnable : __CODEGEN_BITFIELD(31, 31);  //!< Stop And Drain Timer Enable
                };
                uint32_t Value = 0;
            } DW4;
            union
            {
                //!< DWORD 5
                struct
                {
                    uint32_t DebugCounterControl : __CODEGEN_BITFIELD(0, 1);  //!< Debug Counter Control
                    uint32_t Reserved162 : __CODEGEN_BITFIELD(2, 7);          //!< Reserved
                    uint32_t DebugObjectID : __CODEGEN_BITFIELD(8, 31);       //!< Debug Object ID
                };
                uint32_t Value = 0;
            } DW5;

            //! \name Local enumerations
            enum PIPELINE
            {
                PIPELINE_COMPUTE = 2,  //!< No additional details
            };

            enum COMMAND_TYPE
            {
                COMMAND_TYPE_GFXPIPE = 3,  //!< No additional details
            };

            enum COMPUTE_COMMAND_OPCODE
            {
                COMPUTE_COMMAND_OPCODE_CFE_COMMAND = 2,  //!< No additional details
            };

            enum OVER_DISPATCH_CONTROL
            {
                VER_DISPATCH_CONTROL_NONE   = 0,  //!< 0% overdispatch
                VER_DISPATCH_CONTROL_LOW    = 1,  //!< 25% overdispatch
                VER_DISPATCH_CONTROL_NORMAL = 2,  //!< 50% overdispatch
                VER_DISPATCH_CONTROL_HIGH   = 3,  //!< 75% overdispatch
            };

            enum SINGLE_SLICE_DISPATCH_CCS_MODE
            {
                SINGLE_SLICE_DISPATCH_CCS_MODE_DISABLE = 0,  //!< No additional details
                SINGLE_SLICE_DISPATCH_CCS_MODE_ENABLE  = 1,  //!< No additional details
            };

            enum FUSED_EU_DISPATCH
            {
                FUSED_EU_DISPATCH_LEGACY_MODE   = 1,  //!< Legacy Mode, threads are not fused
                FUSED_EU_DISPATCH_FUSED_EU_MODE = 0,  //!< Fused EU Mode
            };

            //! \name Initializations

            //! \brief Explicit member initialization function
            CFE_STATE_CMD()
            {
                DW0.DwordLength          = __CODEGEN_OP_LENGTH(dwSize);
                DW0.CFESubOpcodeVariant  = 0;
                DW0.CFESubOpcode         = 0;
                DW0.ComputeCommandOpcode = COMPUTE_COMMAND_OPCODE_CFE_COMMAND;
                DW0.Pipeline             = PIPELINE_COMPUTE;
                DW0.CommandType          = COMMAND_TYPE_GFXPIPE;

                DW3.OverDispatchControl = VER_DISPATCH_CONTROL_NORMAL;
            }

            static const size_t dwSize   = 6;
            static const size_t byteSize = 24;
        };

        //!
        //! \brief    COMPUTE_WALKER for GEN12 HP
        //! \details  COMPUTE_WALKER spawns threadgroups in 1, 2, or 3 dimensions (X, Y, Z).
        //!           Each threadgroup is described by Interface Descriptor in this command.
        //!           Each dispatched thread has a standard payload delivered in R0 and R1,
        //!           including the Indirect Address to fetch the thread's parameters.
        //!           After the Walker completes dispatching its threads and those threads have
        //!           completed running, a PostSync operation can write a completion code or a
        //!           timestamp.
        //!
        struct COMPUTE_WALKER_CMD
        {
            union
            {
                //!< DWORD 0
                struct
                {
                    uint32_t DWordLength : __CODEGEN_BITFIELD(0, 7);                //!< DWord Length
                    uint32_t PredicateEnable : __CODEGEN_BITFIELD(8, 8);            //!< Predicate Enable
                    uint32_t WorkloadPartitionEnable : __CODEGEN_BITFIELD(9, 9);    //!< Workload Partition Enable
                    uint32_t IndirectParameterEnable : __CODEGEN_BITFIELD(10, 10);  //!< Indirect Parameter Enable
                    uint32_t UAVWaitToProduce : __CODEGEN_BITFIELD(11, 11);         //!< UAV Wait to Produce
                    uint32_t UAVProducer : __CODEGEN_BITFIELD(12, 12);              //!< UAV Producer
                    uint32_t UAVConsumer : __CODEGEN_BITFIELD(13, 13);              //!< UAV Consumer
                    uint32_t SystolicModeEnable : __CODEGEN_BITFIELD(14, 14);       //!< Systolic Mode Enable
                    uint32_t Reserved : __CODEGEN_BITFIELD(15, 15);                 //!< Reserved
                    uint32_t CFESubOpcodeVariant : __CODEGEN_BITFIELD(16, 17);      //!< CFE SubOpcode Variant
                    uint32_t CFESubOpcode : __CODEGEN_BITFIELD(18, 23);             //!< CFE SubOpcode
                    uint32_t ComputeCommandOpcode : __CODEGEN_BITFIELD(24, 26);     //!< Compute Command Opcode
                    uint32_t Pipeline : __CODEGEN_BITFIELD(27, 28);                 //!< Pipeline
                    uint32_t CommandType : __CODEGEN_BITFIELD(29, 31);              //!< Command Type
                };
                uint32_t Value = 0;
            } DW0;
            union
            {
                //!< DWORD 1
                struct
                {
                    uint32_t Reserved32 : __CODEGEN_BITFIELD(0, 7);      //!< Reserved
                    uint32_t DebugObjectID : __CODEGEN_BITFIELD(8, 31);  //!< Debug Object ID
                };
                uint32_t Value = 0;
            } DW1;
            union
            {
                //!< DWORD 2
                struct
                {
                    uint32_t IndirectDataLength : __CODEGEN_BITFIELD(0, 16);           //!< Indirect Data Length
                    uint32_t L3PrefetchDisable : __CODEGEN_BITFIELD(17, 17);           //!< L3 prefetch disable
                    uint32_t PartitionDispatchParameter : __CODEGEN_BITFIELD(18, 29);  //!< Partition Dispatch Parameter
                    uint32_t PartitionType : __CODEGEN_BITFIELD(30, 31);               //!< PartitionType
                };
                uint32_t Value = 0;
            } DW2;
            union
            {
                //!< DWORD 3
                struct
                {
                    uint32_t Reserved96 : __CODEGEN_BITFIELD(0, 5);                 //!< Reserved
                    uint32_t IndirectDataStartAddress : __CODEGEN_BITFIELD(6, 31);  //!< Indirect Data Start Address
                };
                uint32_t Value = 0;
            } DW3;
            union
            {
                //!< DWORD 4
                struct
                {
                    uint32_t Reserved128 : __CODEGEN_BITFIELD(0, 16);           //!< Reserved
                    uint32_t MessageSIMD : __CODEGEN_BITFIELD(17, 18);          //!< Message SIMD
                    uint32_t TileLayout : __CODEGEN_BITFIELD(19, 21);           //!< Tile Layout
                    uint32_t WalkOrder : __CODEGEN_BITFIELD(22, 24);            //!< Walk Order
                    uint32_t EmitInlineParameter : __CODEGEN_BITFIELD(25, 25);  //!< Emit Inline Parameter
                    uint32_t EmitLocal : __CODEGEN_BITFIELD(26, 28);            //!< Emit Local
                    uint32_t GenerateLocalID : __CODEGEN_BITFIELD(29, 29);      //!< Generate Local ID
                    uint32_t SIMDSize : __CODEGEN_BITFIELD(30, 31);             //!< SIMD Size
                };
                uint32_t Value = 0;
            } DW4;
            union
            {
                //!< DWORD 5
                struct
                {
                    uint32_t ExecutionMask : __CODEGEN_BITFIELD(0, 31);  //!< Execution Mask
                };
                uint32_t Value = 0;
            } DW5;
            union
            {
                //!< DWORD 6
                struct
                {
                    uint32_t LocalXMaximum : __CODEGEN_BITFIELD(0, 9);    //!< Local X Maximum
                    uint32_t LocalYMaximum : __CODEGEN_BITFIELD(10, 19);  //!< Local Y Maximum
                    uint32_t LocalZMaximum : __CODEGEN_BITFIELD(20, 29);  //!< Local Z Maximum
                    uint32_t Reserved222 : __CODEGEN_BITFIELD(30, 31);    //!< Reserved
                };
                uint32_t Value = 0;
            } DW6;
            union
            {
                //!< DWORD 7
                struct
                {
                    uint32_t ThreadGroupIDXDimension : __CODEGEN_BITFIELD(0, 31);  //!< Thread Group ID X Dimension
                };
                uint32_t Value = 0;
            } DW7;
            union
            {
                //!< DWORD 8
                struct
                {
                    uint32_t ThreadGroupIDYDimension : __CODEGEN_BITFIELD(0, 31);  //!< Thread Group ID Y Dimension
                };
                uint32_t Value = 0;
            } DW8;
            union
            {
                //!< DWORD 9
                struct
                {
                    uint32_t ThreadGroupIDZDimension : __CODEGEN_BITFIELD(0, 31);  //!< Thread Group ID Z Dimension
                };
                uint32_t Value = 0;
            } DW9;
            union
            {
                //!< DWORD 10
                struct
                {
                    uint32_t ThreadGroupIDStartingX : __CODEGEN_BITFIELD(0, 31);  //!< Thread Group ID Starting X
                };
                uint32_t Value = 0;
            } DW10;
            union
            {
                //!< DWORD 11
                struct
                {
                    uint32_t ThreadGroupIDStartingY : __CODEGEN_BITFIELD(0, 31);  //!< Thread Group ID Starting Y
                };
                uint32_t Value = 0;
            } DW11;
            union
            {
                //!< DWORD 12
                struct
                {
                    uint32_t ThreadGroupIDStartingZ : __CODEGEN_BITFIELD(0, 31);  //!< Thread Group ID Starting Z
                };
                uint32_t Value = 0;
            } DW12;
            union
            {
                //!< DWORD 13_14
                struct
                {
                    uint64_t PartitionID : __CODEGEN_BITFIELD(0, 31);     //!< Partition ID
                    uint64_t PartitionSize : __CODEGEN_BITFIELD(32, 63);  //!< Partition Size
                };
                uint64_t Value = 0;
            } DW13_14;

            union
            {
                struct
                {
                    uint32_t PreemptX;
                };
                uint32_t Value = 0;
            } DW15;

            union
            {
                struct
                {
                    uint32_t PreemptY;
                };
                uint32_t Value = 0;
            } DW16;

            union
            {
                struct
                {
                    uint32_t PreemptZ;
                };
                uint32_t Value = 0;
            } DW17;

            //!
            //! \brief     INTERFACE_DESCRIPTOR_DATA for Gen12 HP
            //! \details   The Interface Descriptor describes the thread state common for all threads
            //!            in the dispatch, including the Kernel base address, the binding tables,
            //!            threadgroup size, and SLM size.
            //!
            struct INTERFACE_DESCRIPTOR_DATA_G12HP_CMD
            {
                union
                {
                    //!< DWORD 0_1
                    struct
                    {
                        uint64_t Reserved0 : __CODEGEN_BITFIELD(0, 5);            //!< Reserved
                        uint64_t KernelStartPointer : __CODEGEN_BITFIELD(6, 31);  //!< Kernel Start Pointer
                        uint64_t Reserved32 : __CODEGEN_BITFIELD(32, 63);         //!< Reserved
                    };
                    uint32_t Value[2] = {0};
                } DW0_1;
                union
                {
                    //!< DWORD 2
                    struct
                    {
                        uint32_t Reserved64 : __CODEGEN_BITFIELD(0, 6);                      //!< Reserved
                        uint32_t SoftwareExceptionEnable : __CODEGEN_BITFIELD(7, 7);         //!< Software Exception Enable
                        uint32_t Reserved72 : __CODEGEN_BITFIELD(8, 10);                     //!< Reserved
                        uint32_t MaskStackExceptionEnable : __CODEGEN_BITFIELD(11, 11);      //!< Mask Stack Exception Enable
                        uint32_t Reserved76 : __CODEGEN_BITFIELD(12, 12);                    //!< Reserved
                        uint32_t IllegalOpcodeExceptionEnable : __CODEGEN_BITFIELD(13, 13);  //!< Illegal Opcode Exception Enable
                        uint32_t Reserved78 : __CODEGEN_BITFIELD(14, 15);                    //!< Reserved
                        uint32_t FloatingPointMode : __CODEGEN_BITFIELD(16, 16);             //!< Floating Point Mode
                        uint32_t Reserved81 : __CODEGEN_BITFIELD(17, 17);                    //!< Reserved
                        uint32_t SingleProgramFlow : __CODEGEN_BITFIELD(18, 18);             //!< Single Program Flow
                        uint32_t DenormMode : __CODEGEN_BITFIELD(19, 19);                    //!< Denorm Mode
                        uint32_t ThreadPreemptionDisable : __CODEGEN_BITFIELD(20, 20);       //!< Thread Preemption Disable
                        uint32_t Reserved85 : __CODEGEN_BITFIELD(21, 31);                    //!< Reserved
                    };
                    uint32_t Value = 0;
                } DW2;
                union
                {
                    //!< DWORD 3
                    struct
                    {
                        uint32_t Reserved96 : __CODEGEN_BITFIELD(0, 1);            //!< Reserved
                        uint32_t SamplerCount : __CODEGEN_BITFIELD(2, 4);          //!< Sampler Count
                        uint32_t SamplerStatePointer : __CODEGEN_BITFIELD(5, 31);  //!< Sampler State Pointer
                    };
                    uint32_t Value = 0;
                } DW3;
                union
                {
                    //!< DWORD 4
                    struct
                    {
                        uint32_t BindingTableEntryCount : __CODEGEN_BITFIELD(0, 4);  //!< Binding Table Entry Count
                        uint32_t BindingTablePointer : __CODEGEN_BITFIELD(5, 20);    //!< Binding Table Pointer
                        uint32_t Reserved149 : __CODEGEN_BITFIELD(21, 31);           //!< Reserved
                    };
                    uint32_t Value = 0;
                } DW4;
                union
                {
                    //!< DWORD 5
                    struct
                    {
                        uint32_t NumberOfThreadsInGpgpuThreadGroup : __CODEGEN_BITFIELD(0, 9);  //!< Number of Threads in GPGPU Thread Group
                        uint32_t Reserved170 : __CODEGEN_BITFIELD(10, 15);                      //!< Reserved
                        uint32_t SharedLocalMemorySize : __CODEGEN_BITFIELD(16, 20);            //!< Shared Local Memory Size
                        uint32_t BarrierEnable : __CODEGEN_BITFIELD(21, 21);                    //!< Barrier Enable
                        uint32_t RoundingMode : __CODEGEN_BITFIELD(22, 23);                     //!< Rounding Mode
                        uint32_t Reserved184 : __CODEGEN_BITFIELD(24, 25);                      //!< Reserved
                        uint32_t ThreadGroupDispatchSize : __CODEGEN_BITFIELD(26, 27);          //!< Thread group dispatch size
                        uint32_t Reserved188 : __CODEGEN_BITFIELD(28, 30);                      //!< Reserved
                        uint32_t BTDMode : __CODEGEN_BITFIELD(31, 31);                          //!< BTD mode
                    };
                    uint32_t Value = 0;
                } DW5;
                union
                {
                    //!< DWORD 6_7
                    struct
                    {
                        uint64_t PreferredSlmAllocationSizePerSubslice : __CODEGEN_BITFIELD(0, 3);  //!< PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE
                        uint64_t Reserved192 : __CODEGEN_BITFIELD(4, 63);  //!< Reserved
                    };
                    uint64_t Value = 0;
                } DW6_7;

                //! \name Initializations

                //! \brief SHARED_LOCAL_MEMORY_SIZE
                //! \details
                //!     This field indicates how much Shared Local Memory the thread group
                //!     requires.
                //!     If the barriers are not enabled,HW will enable at least 1 barrier for
                //!     Mid thread preemption to work.
                enum SHARED_LOCAL_MEMORY_SIZE
                {
                    SHARED_LOCAL_MEMORY_SIZE_SLMENCODES0K   = 0,   //!< No additional details
                    SHARED_LOCAL_MEMORY_SIZE_SLMENCODES1K   = 1,   //!< No additional details
                    SHARED_LOCAL_MEMORY_SIZE_SLMENCODES2K   = 2,   //!< No additional details
                    SHARED_LOCAL_MEMORY_SIZE_SLMENCODES4K   = 3,   //!< No additional details
                    SHARED_LOCAL_MEMORY_SIZE_SLMENCODES8K   = 4,   //!< No additional details
                    SHARED_LOCAL_MEMORY_SIZE_SLMENCODES16K  = 5,   //!< No additional details
                    SHARED_LOCAL_MEMORY_SIZE_SLMENCODES32K  = 6,   //!< No additional details
                    SHARED_LOCAL_MEMORY_SIZE_SLMENCODES64K  = 7,   //!< No additional details
                };

                //! \brief PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE
                //! \details
                //!     For products where SLM and Subslice L1 cacheshares a common,
                //!     re-partitionable RAM, this field indicates the preferred SLM size per
                //!     Subslice for this dispatch. The SLM size programmed here should be >=
                //!     the per thread-group SLM size programmed in DW[5][20:16].
                enum PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE
                {
                    PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODESMAX  = 0x0,   //!< No additional details
                    PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES0K   = 0x8,   //!< No additional details
                    PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES16K  = 0x9,   //!< No additional details
                    PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES32K  = 0xa,   //!< No additional details
                    PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES64K  = 0xb,   //!< No additional details
                    PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES96K  = 0xc,   //!< No additional details
                    PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES128K = 0xd,   //!< No additional details
                };

                //! \brief Explicit member initialization function
                INTERFACE_DESCRIPTOR_DATA_G12HP_CMD()
                {
                }

                static const size_t dwSize   = 8;
                static const size_t byteSize = 32;
            } interface_descriptor_data;

            //!
            //! \brief     POSTSYNC_DATA_CMD
            //! \detail    Post Sync command payload includes the operation, the address, a MOCS field, and an Immediate Data Value.
            //!
            struct POSTSYNC_DATA_CMD
            {
                union
                {
                    //!< DWORD 0
                    struct
                    {
                        uint32_t Operation : __CODEGEN_BITFIELD(0, 1);         //!< Operation
                        uint32_t HDCPipelineFlush : __CODEGEN_BITFIELD(2, 2);  //!< HDC Pipeline Flush
                        uint32_t L3Flush : __CODEGEN_BITFIELD(3, 3);           //!< Reserved
                        uint32_t MOCS : __CODEGEN_BITFIELD(4, 10);             //!< MOCS
                        uint32_t Reserved11 : __CODEGEN_BITFIELD(11, 31);      //!< Reserved
                    };
                    uint32_t Value = 0;
                } DW0;
                union
                {
                    //!< DWORD 1_2
                    struct
                    {
                        uint64_t DestinationAddress : __CODEGEN_BITFIELD(0, 63);  //!< Destination Address
                    };
                    uint32_t Value[2] = {0};
                } DW1_2;
                union
                {
                    //!< DWORD 3_4
                    struct
                    {
                        uint64_t ImmediateData : __CODEGEN_BITFIELD(0, 63);  //!< Immediate Data
                    };
                    uint64_t Value = 0;
                } DW3_4;

                //! \name local enumerations.
                enum POSTSYNC_OPERATION
                {
                    POSTSYNC_OPERATION_NO_WRITE             = 0,  //!< The destination address and immediate data fields are ignored.
                    POSTSYNC_OPERATION_WRITE_IMMEDIATE_DATA = 1,  //!< Writes 8 bytes (64 bits) of immediate data to the destination address.
                    POSTSYNC_OPERATION_WRITE_TIMESTAMP      = 3,  //!< Writes 16 bytes (128 bits) of timestamp data to the destination address.
                };

                //! \brief Explicit member initialization function
                POSTSYNC_DATA_CMD()
                {
                    DW3_4.Value    = 0;
                    DW1_2.Value[1] = DW1_2.Value[0] = DW0.Value = 0;
                }

                static const size_t dwSize   = 5;
                static const size_t byteSize = 20;
            } postsync_data;

            struct INLINE_DATA_CMD
            {
                uint32_t Value[8] = {0};

                //! \brief Explicit member initialization function
                INLINE_DATA_CMD() {}
                static const size_t dwSize   = 8;
                static const size_t byteSize = 32;
            } inline_data;

            //! \name Local enumerations
            enum PIPELINE
            {
                PIPELINE_COMPUTE = 2,  //!< No additional details
            };

            enum COMMAND_TYPE
            {
                COMMAND_TYPE_GFXPIPE = 3,  //!< No additional details
            };

            enum CFE_SUBOPCODE
            {
                CFE_SUBOPCODE_COMPUTE_WALKER = 2,  //!< No additional details
            };

            enum COMPUTE_COMMAND_OPCODE
            {
                COMPUTE_COMMAND_OPCODE_CFE_COMMAND = 2,  //!< No additional details
            };

            enum CFE_SUBOPCODE_VARIANT
            {
                CFE_SUBOPCODE_VARIANT_STANDARD = 0,  //!< No additional details
                CFE_SUBOPCODE_VARIANT_RESUME   = 1,  //!< Resumption of GPGPU_WALKER command, recorded in context image to continue execution after preemption.
            };

            //! \brief    SIMD_SIZE
            //! \details  This field determines the size of the payload and the number of bits of
            //!           the execution mask that are expected.  The kernel pointed to by the
            //!           interface descriptor should match the SIMD declared here.
            enum SIMD_SIZE
            {
                SIMD_SIZE_SIMD8  = 0,  //!< 8 LSBs of the execution mask are used
                SIMD_SIZE_SIMD16 = 1,  //!< 16 LSBs used in execution mask
                SIMD_SIZE_SIMD32 = 2,  //!< 32 bits of execution mask used
            };

            enum WALKER_ORDER
            {
                WALKER_ORDER_WALK012 = 0,  //!< Normal Linear walk order
                WALKER_ORDER_WALK021 = 1,  //!< No additional details
                WALKER_ORDER_WALK102 = 2,  //!< Normal TileY walk order
                WALKER_ORDER_WALK120 = 3,  //!< No additional details
                WALKER_ORDER_WALK201 = 4,  //!< No additional details
                WALKER_ORDER_WALK210 = 5,  //!< No additional details
            };

            //! \name Initializations

            //! \brief Explicit member initialization function
            COMPUTE_WALKER_CMD()
            {
                DW0.DWordLength          = __CODEGEN_OP_LENGTH(dwSize);
                DW0.CFESubOpcodeVariant  = CFE_SUBOPCODE_VARIANT_STANDARD;
                DW0.CFESubOpcode         = CFE_SUBOPCODE_COMPUTE_WALKER;
                DW0.ComputeCommandOpcode = COMPUTE_COMMAND_OPCODE_CFE_COMMAND;
                DW0.Pipeline             = PIPELINE_COMPUTE;
                DW0.CommandType          = COMMAND_TYPE_GFXPIPE;
            }

            static const size_t dwSize   = 39;
            static const size_t byteSize = 156;
        };

        //!
        //! \brief STATE_COMPUTE_MODE command, a general compute programming state shared in the
        //!        pipeline.
        //!
        struct STATE_COMPUTE_MODE_CMD
        {
            union
            {
                struct
                {
                    uint32_t DwordLength : __CODEGEN_BITFIELD(0, 7);
                    uint32_t Reserved8 : __CODEGEN_BITFIELD(8, 15);
                    uint32_t CommandSubOpcode : __CODEGEN_BITFIELD(16, 23);
                    uint32_t CommandOpcode : __CODEGEN_BITFIELD(24, 26);
                    uint32_t CommandSubType : __CODEGEN_BITFIELD(27, 28);
                    uint32_t CommandType : __CODEGEN_BITFIELD(29, 31);
                };
                uint32_t Value = 0;
            } DW0;

            union
            {
                struct
                {
                    uint32_t DisableSupportForMultiGpuFence : __CODEGEN_BITFIELD(0, 0);
                    uint32_t DisableSupportForMultiGpuAtomics : __CODEGEN_BITFIELD(1, 1);

                    uint32_t DisableSupportMultiGpuPartialWrites : __CODEGEN_BITFIELD(2, 2);

                    uint32_t ForceNonCoherent : __CODEGEN_BITFIELD(3, 4);
                    uint32_t FastClearDisabledOnCompressedSurface : __CODEGEN_BITFIELD(5, 5);

                    uint32_t DisableSlmReadMergeOptimization : __CODEGEN_BITFIELD(6, 6);

                    uint32_t AsyncComputeThreadLimit : __CODEGEN_BITFIELD(7, 9);
                    uint32_t BindingTableAlignment : __CODEGEN_BITFIELD(10, 10);
                    uint32_t DisableAtomicOnClearData : __CODEGEN_BITFIELD(11, 11);
                    uint32_t CoherentAccessL1CacheDisable : __CODEGEN_BITFIELD(12, 12);

                    uint32_t DisableL1InvalidateForNonL1CacheableWrites : __CODEGEN_BITFIELD(13, 13);

                    uint32_t Reserved46 : __CODEGEN_BITFIELD(14, 14);
                    uint32_t LargeGrfMode : __CODEGEN_BITFIELD(15, 15);
                    uint32_t MaskBits : __CODEGEN_BITFIELD(16, 31);
                };
                uint32_t Value = 0;
            } DW1;

            STATE_COMPUTE_MODE_CMD()
            {
                DW0.DwordLength      = __CODEGEN_OP_LENGTH(dwSize);
                DW0.CommandSubOpcode = STATE_COMPUTE_MODE;
                DW0.CommandOpcode    = GFXPIPE_NONPIPELINED;
                DW0.CommandSubType   = GFXPIPE_COMMON;
                DW0.CommandType      = GFXPIPE;

                //To improve performance disable support for DisableSupportForMultiGpuAtomics
                //and MultipleGpuParitalWrites, current kernels don't support cross tiles writes
                DW1.DisableSupportForMultiGpuAtomics    = 1;
                DW1.DisableSupportMultiGpuPartialWrites = 1;
            }

            static const size_t   dwSize               = 2;
            static const size_t   byteSize             = 8;
            static const uint32_t GFXPIPE              = 3;
            static const uint32_t GFXPIPE_COMMON       = 0;
            static const uint32_t GFXPIPE_NONPIPELINED = 1;
            static const uint32_t STATE_COMPUTE_MODE   = 5;
        };
    };

}  // namespace xe_hpg
}  // namespace render
}  // namespace mhw

#pragma pack()

#endif  // __MHW_RENDER_HWCMD_XE_HPG_H__
