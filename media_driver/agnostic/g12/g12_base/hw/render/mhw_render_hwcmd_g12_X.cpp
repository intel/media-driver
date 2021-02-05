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
//! \file   mhw_render_hwcmd_g12_X.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

#include "mhw_render_hwcmd_g12_X.h"
#include "mos_utilities.h"

mhw_render_g12_X::MEDIA_OBJECT_CMD::MEDIA_OBJECT_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = __CODEGEN_OP_LENGTH(dwSize);
    DW0.MediaCommandSubOpcode                        = MEDIA_COMMAND_SUB_OPCODE_MEDIAOBJECTSUBOP;
    DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAOBJECT;
    DW0.MediaCommandPipeline                         = MEDIA_COMMAND_PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        
    DW2.SubsliceDestinationSelect                    = SUBSLICE_DESTINATION_SELECT_SUBSLICE0;
    DW2.SliceDestinationSelect                       = SLICE_DESTINATION_SELECT_SLICE0;
    DW2.ThreadSynchronization                        = THREAD_SYNCHRONIZATION_NOTHREADSYNCHRONIZATION;

    DW3.Value                                        = 0;        

    DW4.Value                                        = 0;        

    DW5.Value                                        = 0;        

}

mhw_render_g12_X::PIPELINE_SELECT_CMD::PIPELINE_SELECT_CMD()
{
    DW0.Value                                        = 0;        
    DW0.PipelineSelection                            = PIPELINE_SELECTION_3D;
    DW0.RenderSliceCommonPowerGateEnable             = RENDER_SLICE_COMMON_POWER_GATE_ENABLE_DISABLED;
    DW0.RenderSamplerPowerGateEnable                 = RENDER_SAMPLER_POWER_GATE_ENABLE_DISABLED;
    DW0.MediaSamplerDopClockGateEnable               = MEDIA_SAMPLER_DOP_CLOCK_GATE_ENABLE_DISABLED;
    DW0.ForceMediaAwake                              = FORCE_MEDIA_AWAKE_DISABLED;
    DW0.Command3DSubOpcode                           = _3D_COMMAND_SUB_OPCODE_PIPELINESELECT;
    DW0.Command3DOpcode                              = _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED;
    DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPESINGLEDW;
    DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

}

mhw_render_g12_X::STATE_BASE_ADDRESS_CMD::STATE_BASE_ADDRESS_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = __CODEGEN_OP_LENGTH(dwSize);
    DW0.Command3DSubOpcode                           = _3D_COMMAND_SUB_OPCODE_STATEBASEADDRESS;
    DW0.Command3DOpcode                              = _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED;
    DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPECOMMON;
    DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0;
    DW1_2.GeneralStateBaseAddressModifyEnable        = GENERAL_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

    DW3.Value                                        = 0;
    //To improve performance disable support for MultipleGpuParitalWrites
    //Current kernels don't support cross tiles writes
    DW3.DisableSupportForMultiGpuPartialWritesForStatelessMessages = 1;
    DW3.DisableSupportforMultiGPUAtomicsforStatelessAccesses = 1;

    DW4_5.Value[0] = DW4_5.Value[1]                  = 0;
    DW4_5.SurfaceStateBaseAddressModifyEnable        = SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

    DW6_7.Value[0] = DW6_7.Value[1]                  = 0;
    DW6_7.DynamicStateBaseAddressModifyEnable        = DYNAMIC_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

    DW8_9.Value[0] = DW8_9.Value[1]                  = 0;
    DW8_9.IndirectObjectBaseAddressModifyEnable      = INDIRECT_OBJECT_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

    DW10_11.Value[0] = DW10_11.Value[1]              = 0;
    DW10_11.InstructionBaseAddressModifyEnable       = INSTRUCTION_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

    DW12.Value                                       = 0;
    DW12.GeneralStateBufferSizeModifyEnable          = GENERAL_STATE_BUFFER_SIZE_MODIFY_ENABLE_DISABLE;

    DW13.Value                                       = 0;
    DW13.DynamicStateBufferSizeModifyEnable          = DYNAMIC_STATE_BUFFER_SIZE_MODIFY_ENABLE_DISABLE;

    DW14.Value                                       = 0;
    DW14.IndirectObjectBufferSizeModifyEnable        = INDIRECT_OBJECT_BUFFER_SIZE_MODIFY_ENABLE_DISABLE;

    DW15.Value                                       = 0;
    DW15.InstructionBufferSizeModifyEnable           = INSTRUCTION_BUFFER_SIZE_MODIFY_ENABLE_DISABLE;

    DW16_17.Value[0] = DW16_17.Value[1]              = 0;
    DW16_17.BindlessSurfaceStateBaseAddressModifyEnable = BINDLESS_SURFACE_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

    DW18.Value                                       = 0;

    DW19_20.Value[0] = DW19_20.Value[1]              = 0;
    DW19_20.BindlessSamplerStateBaseAddressModifyEnable = BINDLESS_SAMPLER_STATE_BASE_ADDRESS_MODIFY_ENABLE_DISABLE;

    DW21.Value                                       = 0;

}

mhw_render_g12_X::MEDIA_VFE_STATE_CMD::MEDIA_VFE_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = __CODEGEN_OP_LENGTH(dwSize);
    DW0.Subopcode                                    = SUBOPCODE_MEDIAVFESTATESUBOP;
    DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAVFESTATE;
    DW0.Pipeline                                     = PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        
    DW3.DispatchLoadBalance                          = DISPATCH_LOAD_BALANCE_LEASTLOADED;
    DW3.FusedEuDispatch                              = FUSED_EU_DISPATCH_LEGACYMODE_THREADSARENOTFUSED;

    DW4.Value                                        = 0;        

    DW5.Value                                        = 0;        

    DW6.Value                                        = 0;        

    DW7.Value                                        = 0;        

    DW8.Value                                        = 0;        

}

mhw_render_g12_X::MEDIA_CURBE_LOAD_CMD::MEDIA_CURBE_LOAD_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = __CODEGEN_OP_LENGTH(dwSize);
    DW0.Subopcode                                    = SUBOPCODE_MEDIACURBELOADSUBOP;
    DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIACURBELOAD;
    DW0.Pipeline                                     = PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        

}

mhw_render_g12_X::MEDIA_INTERFACE_DESCRIPTOR_LOAD_CMD::MEDIA_INTERFACE_DESCRIPTOR_LOAD_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = __CODEGEN_OP_LENGTH(dwSize);
    DW0.Subopcode                                    = SUBOPCODE_MEDIAINTERFACEDESCRIPTORLOADSUBOP;
    DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAINTERFACEDESCRIPTORLOAD;
    DW0.Pipeline                                     = PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        

}

mhw_render_g12_X::MEDIA_OBJECT_WALKER_CMD::MEDIA_OBJECT_WALKER_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = __CODEGEN_OP_LENGTH(dwSize);
    DW0.Subopcode                                    = SUBOPCODE_MEDIAOBJECTWALKERSUBOP;
    DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAOBJECTWALKER;
    DW0.Pipeline                                     = PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        
    DW2.MaskedDispatch                               = MASKED_DISPATCH_UNNAMED0;
    DW2.ThreadSynchronization                        = THREAD_SYNCHRONIZATION_NOTHREADSYNCHRONIZATION;

    DW3.Value                                        = 0;        

    DW4.Value                                        = 0;        

    DW5.Value                                        = 0;        
    DW5.GroupIdLoopSelect                            = GROUP_ID_LOOP_SELECT_NOGROUPS;

    DW6.Value                                        = 0;        

    DW7.Value                                        = 0;        

    DW8.Value                                        = 0;        

    DW9.Value                                        = 0;        

    DW10.Value                                       = 0;        

    DW11.Value                                       = 0;        

    DW12.Value                                       = 0;        

    DW13.Value                                       = 0;        

    DW14.Value                                       = 0;        

    DW15.Value                                       = 0;        

    DW16.Value                                       = 0;        

}

mhw_render_g12_X::GPGPU_WALKER_CMD::GPGPU_WALKER_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = __CODEGEN_OP_LENGTH(dwSize);
    DW0.Subopcode                                    = SUBOPCODE_GPGPUWALKERSUBOP;
    DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_GPGPUWALKER;
    DW0.Pipeline                                     = PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        

    DW4.Value                                        = 0;        
    DW4.SimdSize                                     = SIMD_SIZE_SIMD8;

    DW5.Value                                        = 0;        

    DW6.Value                                        = 0;        

    DW7.Value                                        = 0;        

    DW8.Value                                        = 0;        

    DW9.Value                                        = 0;        

    DW10.Value                                       = 0;        

    DW11.Value                                       = 0;        

    DW12.Value                                       = 0;        

    DW13.Value                                       = 0;        

    DW14.Value                                       = 0;        

}

mhw_render_g12_X::_3DSTATE_CHROMA_KEY_CMD::_3DSTATE_CHROMA_KEY_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = __CODEGEN_OP_LENGTH(dwSize);
    DW0.Command3DSubOpcode                           = _3D_COMMAND_SUB_OPCODE_3DSTATECHROMAKEY;
    DW0.Command3DOpcode                              = _3D_COMMAND_OPCODE_3DSTATENONPIPELINED;
    DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPE3D;
    DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        

}

mhw_render_g12_X::PALETTE_ENTRY_CMD::PALETTE_ENTRY_CMD()
{
    DW0.Value                                        = 0;        

}

mhw_render_g12_X::STATE_SIP_CMD::STATE_SIP_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = __CODEGEN_OP_LENGTH(dwSize);
    DW0.Command3DSubOpcode                           = _3D_COMMAND_SUB_OPCODE_STATESIP;
    DW0.Command3DOpcode                              = _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED;
    DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPECOMMON;
    DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0;        

}

mhw_render_g12_X::GPGPU_CSR_BASE_ADDRESS_CMD::GPGPU_CSR_BASE_ADDRESS_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = __CODEGEN_OP_LENGTH(dwSize);
    DW0.Command3DSubOpcode                           = _3D_COMMAND_SUB_OPCODE_GPGPUCSRBASEADDRESS;
    DW0.Command3DOpcode                              = _3D_COMMAND_OPCODE_GFXPIPENONPIPELINED;
    DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPECOMMON;
    DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0;        

}