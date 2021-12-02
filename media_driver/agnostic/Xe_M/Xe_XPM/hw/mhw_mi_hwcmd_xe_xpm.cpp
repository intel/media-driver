
/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file   mhw_mi_hwcmd_xe_xpm.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

// DO NOT EDIT

#include "mhw_mi_hwcmd_xe_xpm.h"
#include <string.h>

mhw_mi_xe_xpm::MI_BATCH_BUFFER_END_CMD::MI_BATCH_BUFFER_END_CMD()
{
    DW0.Value                                        = 0x05000000;
    //DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MIBATCHBUFFEREND;
    //DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

}

mhw_mi_xe_xpm::MI_NOOP_CMD::MI_NOOP_CMD()
{
    DW0.Value                                        = 0x00000000;
    //DW0.IdentificationNumberRegisterWriteEnable      = 0;
    //DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MINOOP;
    //DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

}

mhw_mi_xe_xpm::MI_ARB_CHECK_CMD::MI_ARB_CHECK_CMD()
{
    DW0.Value                                        = 0x02800000;
    //DW0.PreFetchDisable                              = PRE_FETCH_DISABLE_UNNAMED0;
    //DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MIARBCHECK;
    //DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

}

mhw_mi_xe_xpm::MI_LOAD_REGISTER_IMM_CMD::MI_LOAD_REGISTER_IMM_CMD()
{
    DW0.Value                                        = 0x11000001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MmioRemapEnable                              = MMIO_REMAP_ENABLE_UNNAMED0;
    //DW0.AddCsMmioStartOffset                         = ADD_CS_MMIO_START_OFFSET_UNNAMED0;
    //DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MILOADREGISTERIMM;
    //DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

}

mhw_mi_xe_xpm::MI_LOAD_REGISTER_MEM_CMD::MI_LOAD_REGISTER_MEM_CMD()
{
    DW0.Value                                        = 0x14800002;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.WorkloadPartitionIdOffsetEnable              = WORKLOAD_PARTITION_ID_OFFSET_ENABLE_UNNAMED0;
    //DW0.MmioRemapEnable                              = MMIO_REMAP_ENABLE_UNNAMED0;
    //DW0.AddCsMmioStartOffset                         = ADD_CS_MMIO_START_OFFSET_UNNAMED0;
    //DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MILOADREGISTERMEM;
    //DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0x00000000;

    DW2_3.Value[0] = DW2_3.Value[1]                  = 0x00000000;

}

mhw_mi_xe_xpm::MI_LOAD_REGISTER_REG_CMD::MI_LOAD_REGISTER_REG_CMD()
{
    DW0.Value                                        = 0x15000001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MmioRemapEnableSource                        = MMIO_REMAP_ENABLE_SOURCE_UNNAMED0;
    //DW0.MmioRemapEnableDestination                   = MMIO_REMAP_ENABLE_DESTINATION_UNNAMED0;
    //DW0.AddCsMmioStartOffsetSource                   = ADD_CS_MMIO_START_OFFSET_SOURCE_UNNAMED0;
    //DW0.AddCsMmioStartOffsetDestination              = ADD_CS_MMIO_START_OFFSET_DESTINATION_UNNAMED0;
    //DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MILOADREGISTERREG;
    //DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

}

mhw_mi_xe_xpm::MI_STORE_REGISTER_MEM_CMD::MI_STORE_REGISTER_MEM_CMD()
{
    DW0.Value                                        = 0x12000002;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.WorkloadPartitionIdOffsetEnable              = WORKLOAD_PARTITION_ID_OFFSET_ENABLE_UNNAMED0;
    //DW0.MmioRemapEnable                              = MMIO_REMAP_ENABLE_UNNAMED0;
    //DW0.AddCsMmioStartOffset                         = ADD_CS_MMIO_START_OFFSET_UNNAMED0;
    //DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MISTOREREGISTERMEM;
    //DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0x00000000;

    DW2_3.Value[0] = DW2_3.Value[1]                  = 0x00000000;

}

mhw_mi_xe_xpm::MI_BATCH_BUFFER_START_CMD::MI_BATCH_BUFFER_START_CMD()
{
    DW0.Value                                        = 0x18800001;
    //DW0.Obj0.DwordLength                             = GetOpLength(dwSize);
    //DW0.Obj0.AddressSpaceIndicator                   = ADDRESS_SPACE_INDICATOR_GGTT;
    //DW0.Obj1.NestedLevelBatchBuffer                  = NESTED_LEVEL_BATCH_BUFFER_CHAIN;
    //DW0.Obj2.SecondLevelBatchBuffer                  = SECOND_LEVEL_BATCH_BUFFER_FIRSTLEVELBATCH;
    //DW0.Obj3.MiCommandOpcode                         = MI_COMMAND_OPCODE_MIBATCHBUFFERSTART;
    //DW0.Obj3.CommandType                             = COMMAND_TYPE_MICOMMAND;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0x00000000;

}

mhw_mi_xe_xpm::MI_SET_PREDICATE_CMD::MI_SET_PREDICATE_CMD()
{
    DW0.Value                                        = 0x00800000;
    //DW0.PredicateEnable                              = PREDICATE_ENABLE_PREDICATEDISABLE;
    //DW0.PredicateEnableWparid                        = PREDICATE_ENABLE_WPARID_NOOPNEVER;
    //DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MISETPREDICATE;
    //DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

}

mhw_mi_xe_xpm::MI_COPY_MEM_MEM_CMD::MI_COPY_MEM_MEM_CMD()
{
    DW0.Value                                        = 0x17000003;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.UseGlobalGttDestination                      = USE_GLOBAL_GTT_DESTINATION_PERPROCESSGRAPHICSADDRESS;
    //DW0.UseGlobalGttSource                           = USE_GLOBAL_GTT_SOURCE_PERPROCESSGRAPHICSADDRESS;
    //DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MIMEMTOMEM;
    //DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0x00000000;

    DW3_4.Value[0] = DW3_4.Value[1]                  = 0x00000000;

}

mhw_mi_xe_xpm::MI_STORE_DATA_IMM_CMD::MI_STORE_DATA_IMM_CMD()
{
    DW0.Value                                        = 0x10000003;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.ForceWriteCompletionCheck                    = FORCE_WRITE_COMPLETION_CHECK_UNNAMED0;
    //DW0.WorkloadPartitionIdOffsetEnable              = WORKLOAD_PARTITION_ID_OFFSET_ENABLE_UNNAMED0;
    //DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MISTOREDATAIMM;
    //DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;

}

mhw_mi_xe_xpm::MI_SEMAPHORE_SIGNAL_CMD::MI_SEMAPHORE_SIGNAL_CMD()
{
    DW0.Value                                        = 0x0d800000;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MISEMAPHORESIGNAL;
    //DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0x00000000;

}

mhw_mi_xe_xpm::MI_SEMAPHORE_WAIT_CMD::MI_SEMAPHORE_WAIT_CMD()
{
    DW0.Value                                        = 0x0e010003;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.CompareOperation                             = COMPARE_OPERATION_SADGREATERTHANSDD;
    //DW0.WaitMode                                     = WAIT_MODE_SIGNALMODE;
    //DW0.RegisterPollMode                             = REGISTER_POLL_MODE_REGISTERPOLL;
    //DW0.IndirectSemaphoreDataDword                   = INDIRECT_SEMAPHORE_DATA_DWORD_UNNAMED0;
    //DW0.WorkloadPartitionIdOffsetEnable              = WORKLOAD_PARTITION_ID_OFFSET_ENABLE_UNNAMED0;
    //DW0.MemoryType                                   = MEMORY_TYPE_PERPROCESSGRAPHICSADDRESS;
    //DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MISEMAPHOREWAIT;
    //DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0x00000000;

    DW2_3.Value[0] = DW2_3.Value[1]                  = 0x00000000;

    DW4.Value                                        = 0x00000000;

}

mhw_mi_xe_xpm::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::MI_CONDITIONAL_BATCH_BUFFER_END_CMD()
{
    DW0.Value                                        = 0x1b000002;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.CompareOperation                             = COMPARE_OPERATION_MADGREATERTHANIDD;
    //DW0.EndCurrentBatchBufferLevel                   = END_CURRENT_BATCH_BUFFER_LEVEL_UNNAMED0;
    //DW0.CompareMaskMode                              = COMPARE_MASK_MODE_COMPAREMASKMODEDISABLED;
    //DW0.CompareSemaphore                             = COMPARE_SEMAPHORE_UNNAMED0;
    //DW0.UseGlobalGtt                                 = USE_GLOBAL_GTT_UNNAMED0;
    //DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MICONDITIONALBATCHBUFFEREND;
    //DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0x00000000;

    DW2_3.Value[0] = DW2_3.Value[1]                  = 0x00000000;

}

mhw_mi_xe_xpm::MI_ATOMIC_CMD::MI_ATOMIC_CMD()
{
    DW0.Value                                        = 0x17800009;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.DataSize                                     = DATA_SIZE_DWORD;
    //DW0.MemoryType                                   = MEMORY_TYPE_PERPROCESSGRAPHICSADDRESS;
    //DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MIATOMIC;
    //DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0x00000000;
    //DW1.WorkloadPartitionIdOffsetEnable              = WORKLOAD_PARTITION_ID_OFFSET_ENABLE_UNNAMED0;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;

    DW5.Value                                        = 0x00000000;

    DW6.Value                                        = 0x00000000;

    DW7.Value                                        = 0x00000000;

    DW8.Value                                        = 0x00000000;

    DW9.Value                                        = 0x00000000;

    DW10.Value                                       = 0x00000000;

}

mhw_mi_xe_xpm::MI_FLUSH_DW_CMD::MI_FLUSH_DW_CMD()
{
    DW0.Value                                        = 0x13000003;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.PostSyncOperation                            = POST_SYNC_OPERATION_NOWRITE;
    //DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MIFLUSHDW;
    //DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0x00000000;
    //DW1_2.DestinationAddressType                     = DESTINATION_ADDRESS_TYPE_PPGTT;

    DW3_4.Value[0] = DW3_4.Value[1]                  = 0x00000000;

}

mhw_mi_xe_xpm::MI_FORCE_WAKEUP_CMD::MI_FORCE_WAKEUP_CMD()
{
    DW0.Value                                        = 0x0e800000;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MiCommandOpcode                              = MI_COMMAND_OPCODE_MIFORCEWAKEUP;
    //DW0.CommandType                                  = COMMAND_TYPE_MICOMMAND;

    DW1.Value                                        = 0x00000000;
    //DW1.HevcPowerWellControl                         = HEVC_POWER_WELL_CONTROL_DISABLEPOWERWELL;
    //DW1.MfxPowerWellControl                          = MFX_POWER_WELL_CONTROL_DISABLEPOWERWELL;

}

mhw_mi_xe_xpm::MFX_WAIT_CMD::MFX_WAIT_CMD()
{
    DW0.Value                                        = 0x68000000;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.SubOpcode                                    = SUB_OPCODE_MFXWAIT;
    //DW0.CommandSubtype                               = COMMAND_SUBTYPE_MFXSINGLEDW;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

}

mhw_mi_xe_xpm::VD_CONTROL_STATE_CMD::VD_CONTROL_STATE_CMD()
{
    DW0.Value                                        = 0x738a0001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_VDCONTROLSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEFORHCP;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0x00000000;

}

