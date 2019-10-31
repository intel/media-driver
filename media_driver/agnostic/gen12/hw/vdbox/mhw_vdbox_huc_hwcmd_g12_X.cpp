/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file   mhw_vdbox_huc_hwcmd_g12_X.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

// DO NOT EDIT

#include "mhw_vdbox_huc_hwcmd_g12_X.h"
#include <string.h>

mhw_vdbox_huc_g12_X::MEMORYADDRESSATTRIBUTES_CMD::MEMORYADDRESSATTRIBUTES_CMD()
{
    DW0.Value                                        = 0x00000000;
    //DW0.CompressionType                              = COMPRESSION_TYPE_MEDIACOMPRESSIONENABLE;
    //DW0.BaseAddressRowStoreScratchBufferCacheSelect  = BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED0;
    //DW0.BaseAddressTiledResourceMode                 = BASE_ADDRESS_TILED_RESOURCE_MODE_TRMODENONE;

}

mhw_vdbox_huc_g12_X::SPLITBASEADDRESS64BYTEALIGNED_CMD::SPLITBASEADDRESS64BYTEALIGNED_CMD()
{
    DW0_1.Value[0] = DW0_1.Value[1]                  = 0x00000000;

}

mhw_vdbox_huc_g12_X::SPLITBASEADDRESS4KBYTEALIGNED_CMD::SPLITBASEADDRESS4KBYTEALIGNED_CMD()
{
    DW0_1.Value[0] = DW0_1.Value[1]                  = 0x00000000;

}

mhw_vdbox_huc_g12_X::HUC_PIPE_MODE_SELECT_CMD::HUC_PIPE_MODE_SELECT_CMD()
{
    DW0.Value                                        = 0x75800001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HUCPIPEMODESELECT;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.IndirectStreamOutEnable                      = INDIRECT_STREAM_OUT_ENABLE_DISABLEINDIRECTSTREAMOUT;

    DW2.Value                                        = 0x00000000;
    //DW2.MediaSoftResetCounterPer1000Clocks           = MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS_DISABLE;

}

mhw_vdbox_huc_g12_X::HUC_IMEM_STATE_CMD::HUC_IMEM_STATE_CMD()
{
    DW0.Value                                        = 0x75810003;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HUCIMEMSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;
    //DW4.HucFirmwareDescriptor                        = HUC_FIRMWARE_DESCRIPTOR_UNNAMED0;

}

mhw_vdbox_huc_g12_X::HUC_DMEM_STATE_CMD::HUC_DMEM_STATE_CMD()
{
    DW0.Value                                        = 0x75820004;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HUCDMEMSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW4.Value                                        = 0x00000000;

    DW5.Value                                        = 0x00000000;

}

mhw_vdbox_huc_g12_X::HUC_CFG_STATE_CMD::HUC_CFG_STATE_CMD()
{
    DW0.Value                                        = 0x75830000;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HUCCFGSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.P24CMinuteia                                 = P24C_MINUTEIA_NORMALOPERATION;

}

mhw_vdbox_huc_g12_X::HUC_VIRTUAL_ADDR_REGION_CMD::HUC_VIRTUAL_ADDR_REGION_CMD()
{
}

mhw_vdbox_huc_g12_X::HUC_VIRTUAL_ADDR_STATE_CMD::HUC_VIRTUAL_ADDR_STATE_CMD()
{
    DW0.Value                                        = 0x7584002f;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HUCVIRTUALADDRSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

}

mhw_vdbox_huc_g12_X::HUC_IND_OBJ_BASE_ADDR_STATE_CMD::HUC_IND_OBJ_BASE_ADDR_STATE_CMD()
{
    DW0.Value                                        = 0x75850009;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HUCINDOBJBASEADDRSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

}

mhw_vdbox_huc_g12_X::HUC_STREAM_OBJECT_CMD::HUC_STREAM_OBJECT_CMD()
{
    DW0.Value                                        = 0x75a00003;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HUCSTREAMOBJECT;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;
    //DW2.HucProcessing                                = 0;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;
    //DW4.StartCodeSearchEngine                        = START_CODE_SEARCH_ENGINE_DISABLE;
    //DW4.EmulationPreventionByteRemoval               = EMULATION_PREVENTION_BYTE_REMOVAL_DISABLE;
    //DW4.StreamOut                                    = STREAM_OUT_DISABLE;
    //DW4.Drmlengthmode                                = DRMLENGTHMODE_STARTCODEMODE;
    //DW4.HucBitstreamEnable                           = HUC_BITSTREAM_ENABLE_DISABLE;

}

mhw_vdbox_huc_g12_X::HUC_START_CMD::HUC_START_CMD()
{
    DW0.Value                                        = 0x75a10000;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HUCSTART;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.Laststreamobject                             = LASTSTREAMOBJECT_NOTLASTSTREAMOBJECT;

}

