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

#include "mhw_mi_hwcmd_g12_X.h"
#include "mos_utilities.h"

mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::MI_BATCH_BUFFER_END_CMD()
{
    DW0.Value                                        = 0;        
    DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MIBATCHBUFFEREND;
    DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

}

mhw_mi_g12_X::MI_NOOP_CMD::MI_NOOP_CMD()
{
    DW0.Value                                        = 0;        
    DW0.IdentificationNumberRegisterWriteEnable      = 0;
    DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MINOOP;
    DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

}

mhw_mi_g12_X::MI_ARB_CHECK_CMD::MI_ARB_CHECK_CMD()
{
    DW0.Value                                        = 0;        
    DW0.PreFetchDisable                              = PRE_FETCH_DISABLE_UNNAMED0;
    DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MIARBCHECK;
    DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

}

mhw_mi_g12_X::MI_LOAD_REGISTER_IMM_CMD::MI_LOAD_REGISTER_IMM_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MmioRemapEnable                              = MMIO_REMAP_ENABLE_UNNAMED0;
    DW0.AddCsMmioStartOffset                         = ADD_CS_MMIO_START_OFFSET_UNNAMED0;
    DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MILOADREGISTERIMM;
    DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

}

mhw_mi_g12_X::MI_LOAD_REGISTER_MEM_CMD::MI_LOAD_REGISTER_MEM_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MemoryObjectControlStateEnable               = MEMORY_OBJECT_CONTROL_STATE_ENABLE_USEDEFAULT;
    DW0.MmioRemapEnable                              = MMIO_REMAP_ENABLE_UNNAMED0;
    DW0.AddCsMmioStartOffset                         = ADD_CS_MMIO_START_OFFSET_UNNAMED0;
    DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MILOADREGISTERMEM;
    DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0;        

    DW2_3.Value[0] = DW2_3.Value[1]                  = 0;        

}

mhw_mi_g12_X::MI_LOAD_REGISTER_REG_CMD::MI_LOAD_REGISTER_REG_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MmioRemapEnableSource                        = MMIO_REMAP_ENABLE_SOURCE_UNNAMED0;
    DW0.MmioRemapEnableDestination                   = MMIO_REMAP_ENABLE_DESTINATION_UNNAMED0;
    DW0.AddCsMmioStartOffsetSource                   = ADD_CS_MMIO_START_OFFSET_SOURCE_UNNAMED0;
    DW0.AddCsMmioStartOffsetDestination              = ADD_CS_MMIO_START_OFFSET_DESTINATION_UNNAMED0;
    DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MILOADREGISTERREG;
    DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

}

mhw_mi_g12_X::MI_STORE_REGISTER_MEM_CMD::MI_STORE_REGISTER_MEM_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MISTOREREGISTERMEM;
    DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0;        

    DW2_3.Value[0] = DW2_3.Value[1]                  = 0;   
       

}

mhw_mi_g12_X::MI_BATCH_BUFFER_START_CMD::MI_BATCH_BUFFER_START_CMD()
{
    DW0.Value                                        = 0;        
    DW0.Obj0.DwordLength                             = GetOpLength(dwSize);
    DW0.Obj0.AddressSpaceIndicator                   = ADDRESS_SPACE_INDICATOR_GGTT;
    DW0.Obj1.EnableCommandCache                      = ENABLE_COMMAND_CACHE_UNNAMED0;
    DW0.Obj1.PoshEnable                              = POSH_ENABLE_UNNAMED0;
    DW0.Obj1.PoshStart                               = POSH_START_UNNAMED0;
    DW0.Obj1.MiCommandOpcode                         = MI_COMMAND_OPCODE_MIBATCHBUFFERSTART;
    DW0.Obj1.CommandType                             = COMMAND_TYPE_MICOMMAND;
    DW0.Obj2.NestedLevelBatchBuffer                  = NESTED_LEVEL_BATCH_BUFFER_CHAIN;
    DW0.Obj3.SecondLevelBatchBuffer                  = SECOND_LEVEL_BATCH_BUFFER_FIRSTLEVELBATCH;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0;        

}

mhw_mi_g12_X::MI_SET_PREDICATE_CMD::MI_SET_PREDICATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.PredicateEnable                              = PREDICATE_ENABLE_PREDICATEDISABLE;
    DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MISETPREDICATE;
    DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

}

mhw_mi_g12_X::MI_COPY_MEM_MEM_CMD::MI_COPY_MEM_MEM_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MemoryObjectControlStateEnable               = MEMORY_OBJECT_CONTROL_STATE_ENABLE_USEDEFAULT;
    DW0.UseGlobalGttDestination                      = USE_GLOBAL_GTT_DESTINATION_PERPROCESSGRAPHICSADDRESS;
    DW0.UseGlobalGttSource                           = USE_GLOBAL_GTT_SOURCE_PERPROCESSGRAPHICSADDRESS;
    DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MICOPYMEMTOMEM;
    DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0;        

    DW3_4.Value[0] = DW3_4.Value[1]                  = 0;        

}

mhw_mi_g12_X::MI_STORE_DATA_IMM_CMD::MI_STORE_DATA_IMM_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MemoryObjectControlStateEnable               = MEMORY_OBJECT_CONTROL_STATE_ENABLE_USEDEFAULT;
    DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MISTOREDATAIMM;
    DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0;        

    DW3.Value                                        = 0;        

    DW4.Value                                        = 0;        

}

mhw_mi_g12_X::MI_SEMAPHORE_SIGNAL_CMD::MI_SEMAPHORE_SIGNAL_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.PostSyncOperation                            = POST_SYNC_OPERATION_NOPOSTSYNCOPERATION;
    DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MISEMAPHORESIGNAL;
    DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0;        

}

mhw_mi_g12_X::MI_SEMAPHORE_WAIT_CMD::MI_SEMAPHORE_WAIT_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.CompareOperation                             = COMPARE_OPERATION_SADGREATERTHANSDD;
    DW0.WaitMode                                     = WAIT_MODE_SIGNALMODE;
    DW0.RegisterPollMode                             = REGISTER_POLL_MODE_REGISTERPOLL;
    DW0.MemoryType                                   = MEMORY_TYPE_PERPROCESSGRAPHICSADDRESS;
    DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MISEMAPHOREWAIT;
    DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0;        

    DW2_3.Value[0] = DW2_3.Value[1]                  = 0;        

    DW4.Value                                        = 0;        

}

mhw_mi_g12_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::MI_CONDITIONAL_BATCH_BUFFER_END_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.CompareOperation                             = COMPARE_OPERATION_MADGREATERTHANIDD;
    DW0.EndCurrentBatchBufferLevel                   = END_CURRENT_BATCH_BUFFER_LEVEL_UNNAMED0;
    DW0.CompareMaskMode                              = COMPARE_MASK_MODE_COMPAREMASKMODEDISABLED;
    DW0.CompareSemaphore                             = COMPARE_SEMAPHORE_UNNAMED0;
    DW0.UseGlobalGtt                                 = USE_GLOBAL_GTT_UNNAMED0;
    DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MICONDITIONALBATCHBUFFEREND;
    DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0;        

    DW2_3.Value[0] = DW2_3.Value[1]                  = 0;        

}

mhw_mi_g12_X::MI_ATOMIC_CMD::MI_ATOMIC_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.DataSize                                     = DATA_SIZE_DWORD;
    DW0.PostSyncOperation                            = POST_SYNC_OPERATION_NOPOSTSYNCOPERATION;
    DW0.MemoryType                                   = MEMORY_TYPE_PERPROCESSGRAPHICSADDRESS;
    DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MIATOMIC;
    DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        

    DW4.Value                                        = 0;        

    DW5.Value                                        = 0;        

    DW6.Value                                        = 0;        

    DW7.Value                                        = 0;        

    DW8.Value                                        = 0;        

    DW9.Value                                        = 0;        

    DW10.Value                                       = 0;        

}

mhw_mi_g12_X::MI_MATH_CMD::MI_MATH_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MIMATH;
    DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

}

mhw_mi_g12_X::MI_FLUSH_DW_CMD::MI_FLUSH_DW_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.PostSyncOperation                            = POST_SYNC_OPERATION_NOWRITE;
    DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MIFLUSHDW;
    DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0;        
    DW1_2.DestinationAddressType                     = DESTINATION_ADDRESS_TYPE_PPGTT;

    DW3_4.Value[0] = DW3_4.Value[1]                  = 0;        

}

mhw_mi_g12_X::MI_FORCE_WAKEUP_CMD::MI_FORCE_WAKEUP_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MIFORCEWAKEUP;
    DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0;        
    DW1.HevcPowerWellControl                         = HEVC_POWER_WELL_CONTROL_DISABLEPOWERWELL;
    DW1.MfxPowerWellControl                          = MFX_POWER_WELL_CONTROL_DISABLEPOWERWELL;

}

mhw_mi_g12_X::PIPE_CONTROL_CMD::PIPE_CONTROL_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.PostSyncOperationL3CacheabilityControl       = POST_SYNC_OPERATION_L3_CACHEABILITY_CONTROL_DEFAULTMOCS;
    DW0.Command3DSubOpcode                           = _3D_COMMAND_SUB_OPCODE_PIPECONTROL;
    DW0.Command3DOpcode                              = _3D_COMMAND_OPCODE_PIPECONTROL;
    DW0.CommandSubtype                               = COMMAND_SUBTYPE_GFXPIPE3D;
    DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1.Value                                        = 0;        
    DW1.DepthCacheFlushEnable                        = DEPTH_CACHE_FLUSH_ENABLE_FLUSHDISABLED;
    DW1.StallAtPixelScoreboard                       = STALL_AT_PIXEL_SCOREBOARD_DISABLE;
    DW1.RenderTargetCacheFlushEnable                 = RENDER_TARGET_CACHE_FLUSH_ENABLE_DISABLEFLUSH;
    DW1.DepthStallEnable                             = DEPTH_STALL_ENABLE_DISABLE;
    DW1.PostSyncOperation                            = POST_SYNC_OPERATION_NOWRITE;
    DW1.GlobalSnapshotCountReset                     = GLOBAL_SNAPSHOT_COUNT_RESET_DONTRESET;
    DW1.LriPostSyncOperation                         = LRI_POST_SYNC_OPERATION_NOLRIOPERATION;
    DW1.DestinationAddressType                       = DESTINATION_ADDRESS_TYPE_PPGTT;
    DW1.TileCacheFlushEnable                         = TILE_CACHE_FLUSH_ENABLE_UNNAMED0;

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        

    DW4_5.Value[0] = DW4_5.Value[1]                  = 0;        

}

mhw_mi_g12_X::MFX_WAIT_CMD::MFX_WAIT_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.SubOpcode                                    = SUB_OPCODE_MFXWAIT;
    DW0.CommandSubtype                               = COMMAND_SUBTYPE_MFXSINGLEDW;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

}

mhw_mi_g12_X::MEDIA_STATE_FLUSH_CMD::MEDIA_STATE_FLUSH_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopcode                                    = SUBOPCODE_MEDIASTATEFLUSHSUBOP;
    DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIASTATEFLUSH;
    DW0.Pipeline                                     = PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_GFXPIPE;

    DW1.Value                                        = 0;        

}

mhw_mi_g12_X::VD_CONTROL_STATE_CMD::VD_CONTROL_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MediaInstructionCommand                      = 0;
    DW0.MediaInstructionOpcode                       = 0;
    DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

}

