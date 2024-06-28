
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
//! \file   mhw_render_hwcmd_xe2_hpg_next.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

// DO NOT EDIT

#include "mhw_render_hwcmd_xe2_hpg_next.h"
#include <string.h>

using namespace mhw::render::xe2_hpg_next;

Cmd::PIPELINE_SELECT_CMD::PIPELINE_SELECT_CMD()
{
    DW0.Value                                        = 0x69040000;
    //DW0.PipelineSelection                            = PIPELINE_SELECTION_3D;
    //DW0.RenderSliceCommonPowerGateEnable             = RENDER_SLICE_COMMON_POWER_GATE_ENABLE_DISABLED;
    //DW0.RenderSamplerPowerGateEnable                 = RENDER_SAMPLER_POWER_GATE_ENABLE_DISABLED;
    //DW0.SystolicModeEnable                           = SYSTOLIC_MODE_ENABLE_SYSTOLICMODEDISABLED;
    //DW0._3DCommandSubOpcode                          = _3D_COMMAND_SUB_OPCODE_PIPELINESELECT;
    //DW0._3DCommandOpcode                             = _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED;
    //DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPESINGLEDW;
    //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

}

Cmd::STATE_BASE_ADDRESS_CMD::STATE_BASE_ADDRESS_CMD()
{
    DW0.Value                                        = 0x61010014;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0._3DCommandSubOpcode                          = _3D_COMMAND_SUB_OPCODE_STATEBASEADDRESS;
    //DW0._3DCommandOpcode                             = _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED;
    //DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPECOMMON;
    //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0x00000000;
    //DW1_2.GeneralStateBaseAddressModifyEnable        = GENERAL_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

    DW3.Value                                        = 0x00000000;
    //DW3.CoherencySettingModifyEnable                 = COHERENCY_SETTING_MODIFY_ENABLE_DISABLEWRITETOTHISDW;
    //DW3.DisableSupportForMultiGpuAtomicsForStatelessAccesses = DISABLE_SUPPORT_FOR_MULTI_GPU_ATOMICS_FOR_STATELESS_ACCESSES_ENABLE;
    //DW3.DisableSupportForMultiGpuPartialWritesForStatelessMessages = DISABLE_SUPPORT_FOR_MULTI_GPU_PARTIAL_WRITES_FOR_STATELESS_MESSAGES_ENABLED;

    DW4_5.Value[0] = DW4_5.Value[1]                  = 0x00000000;
    //DW4_5.SurfaceStateBaseAddressModifyEnable        = SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

    DW6_7.Value[0] = DW6_7.Value[1]                  = 0x00000000;
    //DW6_7.DynamicStateBaseAddressModifyEnable        = DYNAMIC_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

    DW8_9.Value[0] = DW8_9.Value[1]                  = 0x00000000;
    //DW8_9.IndirectObjectBaseAddressModifyEnable      = INDIRECT_OBJECT_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

    DW10_11.Value[0] = DW10_11.Value[1]              = 0x00000000;
    //DW10_11.InstructionBaseAddressModifyEnable       = INSTRUCTION_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

    DW12.Value                                       = 0x00000000;
    //DW12.GeneralStateBufferSizeModifyEnable          = GENERAL_STATE_BUFFER_SIZE_MODIFY_ENABLE_DISABLE;

    DW13.Value                                       = 0x00000000;
    //DW13.DynamicStateBufferSizeModifyEnable          = DYNAMIC_STATE_BUFFER_SIZE_MODIFY_ENABLE_DISABLE;

    DW14.Value                                       = 0x00000000;
    //DW14.IndirectObjectBufferSizeModifyEnable        = INDIRECT_OBJECT_BUFFER_SIZE_MODIFY_ENABLE_DISABLE;

    DW15.Value                                       = 0x00000000;
    //DW15.InstructionBufferSizeModifyEnable           = INSTRUCTION_BUFFER_SIZE_MODIFY_ENABLE_DISABLE;

    DW16_17.Value[0] = DW16_17.Value[1]              = 0x00000000;
    //DW16_17.BindlessSurfaceStateBaseAddressModifyEnable = BINDLESS_SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

    DW18.Value                                       = 0x00000000;

    DW19_20.Value[0] = DW19_20.Value[1]              = 0x00000000;
    //DW19_20.BindlessSamplerStateBaseAddressModifyEnable = BINDLESS_SAMPLER_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

    DW21.Value                                       = 0x00000000;

}

Cmd::_3DSTATE_CHROMA_KEY_CMD::_3DSTATE_CHROMA_KEY_CMD()
{
    DW0.Value                                        = 0x79040002;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0._3DCommandSubOpcode                          = _3D_COMMAND_SUB_OPCODE_3DSTATECHROMAKEY;
    //DW0._3DCommandOpcode                             = _3D_COMMAND_OPCODE_3DSTATENONPIPELINED;
    //DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPE3D;
    //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

}

Cmd::STATE_SIP_CMD::STATE_SIP_CMD()
{
    DW0.Value                                        = 0x61020001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0._3DCommandSubOpcode                          = _3D_COMMAND_SUB_OPCODE_STATESIP;
    //DW0._3DCommandOpcode                             = _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED;
    //DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPECOMMON;
    //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0x00000000;

}

Cmd::_3DSTATE_BINDING_TABLE_POOL_ALLOC_CMD::_3DSTATE_BINDING_TABLE_POOL_ALLOC_CMD()
{
    DW0.Value                                        = 0x79190002;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0._3DCommandSubOpcode                          = _3D_COMMAND_SUB_OPCODE_3DSTATEBINDINGTABLEPOOLALLOC;
    //DW0._3DCommandOpcode                             = _3D_COMMAND_OPCODE_3DSTATENONPIPELINED;
    //DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPE3D;
    //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0x00000000;

    DW3.Value                                        = 0x00000000;
    //DW3.BindingTablePoolBufferSize                   = BINDING_TABLE_POOL_BUFFER_SIZE_NOVALIDDATA;

}

Cmd::COMPUTE_WALKER_CMD::COMPUTE_WALKER_CMD()
{
    DW0.Value                                        = 0x72080026;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.CfeSubopcodeVariant                          = CFE_SUBOPCODE_VARIANT_STANDARD;
    //DW0.CfeSubopcode                                 = CFE_SUBOPCODE_COMPUTEWALKER;
    //DW0.ComputeCommandOpcode                         = COMPUTE_COMMAND_OPCODE_NEWCFECOMMAND;
    //DW0.Pipeline                                     = PIPELINE_COMPUTE;
    //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;
    //DW2.PartitionType                                = PARTITION_TYPE_DISABLED;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;
    //DW4.MessageSimd                                  = 0;
    //DW4.TileLayout                                   = TILE_LAYOUT_LINEAR;
    //DW4.WalkOrder                                    = WALK_ORDER_WALK012;
    //DW4.EmitLocal                                    = EMIT_LOCAL_EMITNONE;
    //DW4.SimdSize                                     = 0;

    DW5.Value                                        = 0x00000000;

    DW6.Value                                        = 0x00000000;

    DW7.Value                                        = 0x00000000;

    DW8.Value                                        = 0x00000000;

    DW9.Value                                        = 0x00000000;

    DW10.Value                                       = 0x00000000;

    DW11.Value                                       = 0x00000000;

    DW12.Value                                       = 0x00000000;

    DW13.Value                                       = 0x00000000;

    DW14.Value                                       = 0x00000000;

    DW15.Value                                       = 0x00000000;

    DW16.Value                                       = 0x00000000;

    DW17.Value                                       = 0x00000000;

    DW18.Value                                       = 0x00000000;

    InterfaceDescriptor = {};

    PostSync = {};

    InlineData = {};

}

Cmd::CFE_STATE_CMD::CFE_STATE_CMD()
{
    DW0.Value                                        = 0x72000004;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.CfeSubopcodeVariant                          = CFE_SUBOPCODE_VARIANT_STANDARD;
    //DW0.CfeSubopcode                                 = CFE_SUBOPCODE_CFESTATE;
    //DW0.ComputeCommandOpcode                         = COMPUTE_COMMAND_OPCODE_NEWCFECOMMAND;
    //DW0.Pipeline                                     = PIPELINE_COMPUTE;
    //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00008000;
    //DW3.ControlsTheNumberOfStackidsForRayTracingSubsystem = CONTROLS_THE_NUMBER_OF_STACKIDS_FOR_RAY_TRACING_SUBSYSTEM_2K;
    //DW3.LargeGrfThreadAdjustDisable                  = LARGE_GRF_THREAD_ADJUST_DISABLE_ENABLED;
    //DW3.ComputeOverdispatchDisable                   = COMPUTE_OVERDISPATCH_DISABLE_ENABLED;
    //DW3.ComputeDispatchAllWalkerEnable               = COMPUTE_DISPATCH_ALL_WALKER_ENABLE_DISABLED;
    //DW3.OverDispatchControl                          = OVER_DISPATCH_CONTROL_NORMAL;

    DW4.Value                                        = 0x00000000;

    DW5.Value                                        = 0x00000000;

}

Cmd::STATE_COMPUTE_MODE_CMD::STATE_COMPUTE_MODE_CMD()
{
    DW0.Value                                        = 0x61050001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0._3DCommandSubOpcode                          = _3D_COMMAND_SUB_OPCODE_STATECOMPUTEMODE;
    //DW0._3DCommandOpcode                             = _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED;
    //DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPECOMMON;
    //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.ZPassAsyncComputeThreadLimit                 = Z_PASS_ASYNC_COMPUTE_THREAD_LIMIT_MAX60;
    //DW1.NpZAsyncThrottleSettings                     = NP_Z_ASYNC_THROTTLE_SETTINGS_UNNAMED0;
    //DW1.AsyncComputeThreadLimit                      = ASYNC_COMPUTE_THREAD_LIMIT_DISABLED;
    //DW1.EuThreadSchedulingModeOverride               = EU_THREAD_SCHEDULING_MODE_OVERRIDE_HWDEFAULT;
    //DW1.LargeGrfMode                                 = LARGE_GRF_MODE_UNNAMED0;

    DW2.Value                                        = 0x00000000;
    //DW2.MidthreadPreemptionDelayTimer                = MIDTHREAD_PREEMPTION_DELAY_TIMER_MTPTIMERVAL0;
    //DW2.MidthreadPreemptionOverdispatchThreadGroupCount = MIDTHREAD_PREEMPTION_OVERDISPATCH_THREAD_GROUP_COUNT_ODTGM2;
    //DW2.MidthreadPreemptionOverdispatchTestMode      = MIDTHREAD_PREEMPTION_OVERDISPATCH_TEST_MODE_REGULAR;
    //DW2.UavCoherencyMode                             = UAV_COHERENCY_MODE_DRAIN_DATAPORT_MODE;
    //DW2.MemoryAllocationForScratchAndMidthreadPreemptionBuffers = MEMORY_ALLOCATION_FOR_SCRATCH_AND_MIDTHREAD_PREEMPTION_BUFFERS_FULL;

}

Cmd::COMPUTE_WALKER_CMD::INTERFACE_DESCRIPTOR_DATA_CMD::INTERFACE_DESCRIPTOR_DATA_CMD()
{
    DW0.Value = 0x00000000;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;
    //DW2.FloatingPointMode                            = FLOATING_POINT_MODE_IEEE_754;
    //DW2.SingleProgramFlow                            = SINGLE_PROGRAM_FLOW_MULTIPLE;
    //DW2.DenormMode                                   = DENORM_MODE_FTZ;
    //DW2.ThreadPreemption                             = THREAD_PREEMPTION_DISABLE;

    DW3.Value = 0x00000000;
    //DW3.SamplerCount                                 = SAMPLER_COUNT_NOSAMPLERSUSED;

    DW4.Value = 0x00000000;
    //DW4.BindingTableEntryCount                       = BINDING_TABLE_ENTRY_COUNT_PREFETCHDISABLED;

    DW5.Value = 0x00000000;
    //DW5.ThreadGroupForwardProgressGuarantee          = THREAD_GROUP_FORWARD_PROGRESS_GUARANTEE_DISABLE;
    //DW5.SharedLocalMemorySize                        = SHARED_LOCAL_MEMORY_SIZE_SLMENCODES0K;
    //DW5.RoundingMode                                 = ROUNDING_MODE_RTNE;
    //DW5.ThreadGroupDispatchSize                      = THREAD_GROUP_DISPATCH_SIZE_TGSIZE8;
    //DW5.NumberOfBarriers                             = NUMBER_OF_BARRIERS_NONE;
    //DW5.BtdMode                                      = BTD_MODE_DISABLE;

    DW6.Value = 0x00000000;

    DW7.Value = 0x00000000;
    //DW7.PreferredSlmAllocationSizePerSubslice        = PREFERRED_SLM_ALLOCATION_SIZE_PER_SUBSLICE_SLMENCODES0K;
}

Cmd::COMPUTE_WALKER_CMD::POSTSYNC_DATA_CMD::POSTSYNC_DATA_CMD()
{
    DW0.Value = 0x00000000;
    //DW0.Operation                                    = OPERATION_NOWRITE;

    DW1_2.Value[0] = DW1_2.Value[1] = 0x00000000;

    DW3_4.Value[0] = DW3_4.Value[1] = 0x00000000;
}

Cmd::PALETTE_ENTRY_CMD::PALETTE_ENTRY_CMD()
{
    DW0.Value = 0x00000000;
}

Cmd::GPGPU_CSR_BASE_ADDRESS_CMD::GPGPU_CSR_BASE_ADDRESS_CMD()
{
    DW0.Value = 0x61040001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0._3DCommandSubOpcode                          = _3D_COMMAND_SUB_OPCODE_GPGPUCSRBASEADDRESS;
    //DW0._3DCommandOpcode                             = _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED;
    //DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPECOMMON;
    //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1_2.Value[0] = DW1_2.Value[1] = 0x00000000;
}

