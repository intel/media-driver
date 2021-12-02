/*===================== begin_copyright_notice ==================================

* Copyright (c) 2021, Intel Corporation
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
//! \file   mhw_render_hwcmd_xe_hp_base.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

// DO NOT EDIT

#include "mhw_render_hwcmd_xe_hp_base.h"
#include <string.h>

mhw_render_xe_xpm_base::PIPELINE_SELECT_CMD::PIPELINE_SELECT_CMD()
{
    DW0.Value                                        = 0x69040000;
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

mhw_render_xe_xpm_base::STATE_BASE_ADDRESS_CMD::STATE_BASE_ADDRESS_CMD()
{
    DW0.Value                                        = 0x61010014;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.Command3DSubOpcode                          = _3D_COMMAND_SUB_OPCODE_STATEBASEADDRESS;
    //DW0.Command3DOpcode                             = _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED;
    //DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPECOMMON;
    //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0x00000000;
    //DW1_2.GeneralStateBaseAddressModifyEnable        = GENERAL_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

    DW3.Value                                        = 0x00000000;
    //DW3.CoherencySettingModifyEnable                 = COHERENCY_SETTING_MODIFY_ENABLE_DISABLEWRITETOTHISDW;
    //DW3.EnableMemoryCompressionForAllStatelessAccesses = ENABLE_MEMORY_COMPRESSION_FOR_ALL_STATELESS_ACCESSES_DISABLED;
    //DW3.DisableSupportForMultiGpuAtomicsForStatelessAccesses = DISABLE_SUPPORT_FOR_MULTI_GPU_ATOMICS_FOR_STATELESS_ACCESSES_ENABLE;

    //To improve performance disable support for MultipleGpu sync features 
    //Current kernels don't support cross tiles writes
    DW3.DisableSupportForMultiGpuAtomicsForStatelessAccesses = DISABLE_SUPPORT_FOR_MULTI_GPU_ATOMICS_FOR_STATELESS_ACCESSES_DISABLE;
    DW3.DisableSupportForMultiGpuPartialWritesForStatelessMessages = DISABLE_SUPPORT_FOR_MULTI_GPU_PARTIAL_WRITES_FOR_STATELESS_MESSAGES_DISABLED;

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

mhw_render_xe_xpm_base::_3DSTATE_CHROMA_KEY_CMD::_3DSTATE_CHROMA_KEY_CMD()
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

mhw_render_xe_xpm_base::PALETTE_ENTRY_CMD::PALETTE_ENTRY_CMD()
{
    DW0.Value                                        = 0x00000000;

}

mhw_render_xe_xpm_base::STATE_SIP_CMD::STATE_SIP_CMD()
{
    DW0.Value                                        = 0x61020001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0._3DCommandSubOpcode                          = _3D_COMMAND_SUB_OPCODE_STATESIP;
    //DW0._3DCommandOpcode                             = _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED;
    //DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPECOMMON;
    //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0x00000000;

}

mhw_render_xe_xpm_base::GPGPU_CSR_BASE_ADDRESS_CMD::GPGPU_CSR_BASE_ADDRESS_CMD()
{
    DW0.Value                                        = 0x61040001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0._3DCommandSubOpcode                          = _3D_COMMAND_SUB_OPCODE_GPGPUCSRBASEADDRESS;
    //DW0._3DCommandOpcode                             = _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED;
    //DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPECOMMON;
    //DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0x00000000;

}

mhw_render_xe_xpm_base::_3DSTATE_BINDING_TABLE_POOL_ALLOC_CMD::_3DSTATE_BINDING_TABLE_POOL_ALLOC_CMD()
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

mhw_render_xe_xpm_base::CFE_STATE_CMD::CFE_STATE_CMD()
{
    DW0.DwordLength                                 = __CODEGEN_OP_LENGTH(dwSize);
    DW0.CFESubOpcodeVariant                         = 0;
    DW0.CFESubOpcode                                = 0;
    DW0.ComputeCommandOpcode                        = COMPUTE_COMMAND_OPCODE_CFE_COMMAND;
    DW0.Pipeline                                    = PIPELINE_COMPUTE;
    DW0.CommandType                                 = COMMAND_TYPE_GFXPIPE;

    DW3.OverDispatchControl                         = VER_DISPATCH_CONTROL_NORMAL;
}

mhw_render_xe_xpm_base::COMPUTE_WALKER_CMD::COMPUTE_WALKER_CMD()
{
    DW0.DWordLength                                 = __CODEGEN_OP_LENGTH(dwSize);
    DW0.CFESubOpcodeVariant                         = CFE_SUBOPCODE_VARIANT_STANDARD;
    DW0.CFESubOpcode                                = CFE_SUBOPCODE_COMPUTE_WALKER;
    DW0.ComputeCommandOpcode                        = COMPUTE_COMMAND_OPCODE_CFE_COMMAND;
    DW0.Pipeline                                    = PIPELINE_COMPUTE;
    DW0.CommandType                                 = COMMAND_TYPE_GFXPIPE;
}

mhw_render_xe_xpm_base::COMPUTE_WALKER_CMD::INTERFACE_DESCRIPTOR_DATA_G12HP_CMD::INTERFACE_DESCRIPTOR_DATA_G12HP_CMD()
{
}

mhw_render_xe_xpm_base::COMPUTE_WALKER_CMD::POSTSYNC_DATA_CMD::POSTSYNC_DATA_CMD()
{
    DW3_4.Value = 0;
    DW1_2.Value[1] = DW1_2.Value[0] = DW0.Value = 0;
}

mhw_render_xe_xpm_base::STATE_COMPUTE_MODE_CMD::STATE_COMPUTE_MODE_CMD()
{
    DW0.DwordLength = __CODEGEN_OP_LENGTH(dwSize);
    DW0.CommandSubOpcode = STATE_COMPUTE_MODE;
    DW0.CommandOpcode = GFXPIPE_NONPIPELINED;
    DW0.CommandSubType = GFXPIPE_COMMON;
    DW0.CommandType = GFXPIPE;

    //To improve performance disable support for DisableSupportForMultiGpuAtomics
    //and MultipleGpuParitalWrites, current kernels don't support cross tiles writes
    DW1.DisableSupportForMultiGpuAtomics = 1;
    DW1.DisableSupportMultiGpuPartialWrites = 1;
}