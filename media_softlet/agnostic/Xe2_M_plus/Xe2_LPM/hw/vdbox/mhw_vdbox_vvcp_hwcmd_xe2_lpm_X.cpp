
/*===================== begin_copyright_notice ==================================

# Copyright (c) 2024, Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file   mhw_vdbox_vvcp_hwcmd_xe2_lpm_X.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

// DO NOT EDIT

#include "mhw_vdbox_vvcp_hwcmd_xe2_lpm_X.h"
#include <string.h>

using namespace mhw::vdbox::vvcp::xe2_lpm_base::xe2_lpm;

Cmd::MEMORYADDRESSATTRIBUTES_CMD::MEMORYADDRESSATTRIBUTES_CMD()
{
    DW0.Value                                        = 0x00000000;
    //DW0.CompressionType                              = COMPRESSION_TYPE_MEDIACOMPRESSIONENABLE;
    //DW0.BaseAddressRowStoreScratchBufferCacheSelect  = BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED0;
    //DW0.Tilemode                                     = TILEMODE_LINEAR;

}

Cmd::SPLITBASEADDRESS4KBYTEALIGNED_CMD::SPLITBASEADDRESS4KBYTEALIGNED_CMD()
{
    DW0_1.Value[0] = DW0_1.Value[1]                  = 0x00000000;

}

Cmd::SPLITBASEADDRESS64BYTEALIGNED_CMD::SPLITBASEADDRESS64BYTEALIGNED_CMD()
{
    DW0_1.Value[0] = DW0_1.Value[1]                  = 0x00000000;

}

Cmd::VVCP_APS_ALF_PARAMSET::VVCP_APS_ALF_PARAMSET()
{
    DW0.Value                                        = 0x00000000;

    DW30_Reserved                                    = 0x00000000;
    DW31_Reserved                                    = 0x00000000;
    D127_Reserved                                    = 0x00000000;
}

Cmd::VVCP_APS_LMCS_PARAMSET_CMD::VVCP_APS_LMCS_PARAMSET_CMD()
{
    DW0.Value                                        = 0x00000000;

    memset(&DW1_8.Value, 0, sizeof(DW1_8.Value));
    memset(&DW9_16.Value, 0, sizeof(DW9_16.Value));
    memset(&DW17_24.Value, 0, sizeof(DW17_24.Value));
    memset(&DW25_32.Value, 0, sizeof(DW25_32.Value));
}

Cmd::VVCP_APS_SCALINGLIST_PARAMSET_CMD::VVCP_APS_SCALINGLIST_PARAMSET_CMD()
{
    memset(&Onescalingmatrixset, 0, sizeof(Onescalingmatrixset));

}

Cmd::VVCP_DPB_ENTRY_CMD::VVCP_DPB_ENTRY_CMD()
{
    DW0.Value                                        = 0x00000000;
    DW1.Value                                        = 0x00000000;
    DW2.Value                                        = 0x00000000;
    DW3.Value                                        = 0x00000000;
    DW4.Value                                        = 0x00000000;

}

Cmd::VVCP_PIC_ALF_PARAMETER_ENTRY_CMD::VVCP_PIC_ALF_PARAMETER_ENTRY_CMD()
{
    DW0.Value                                        = 0x00000000;

}

Cmd::VVCP_REF_LIST_ENTRY_CMD::VVCP_REF_LIST_ENTRY_CMD()
{
    DW0.Value                                        = 0x00000000;
    //DW0.StRefPicFlagListidxRplsidxI                  = ST_REF_PIC_FLAG_LISTIDX_RPLSIDX_I_LONGTERMREFERENCE;

}

Cmd::VVCP_SPS_CHROMAQPTABLE_CMD::VVCP_SPS_CHROMAQPTABLE_CMD()
{
    memset(&Chromaqptable, 0, sizeof(Chromaqptable));

}

Cmd::VD_CONTROL_STATE_BODY_CMD::VD_CONTROL_STATE_BODY_CMD()
{
    DW0.Value                                        = 0x00000000;
    //DW0.VdboxPipelineArchitectureClockgateDisable    = VDBOX_PIPELINE_ARCHITECTURE_CLOCKGATE_DISABLE_ENABLE;

    DW1.Value                                        = 0x00000000;

}

Cmd::VVCP_WEIGHTOFFSET_LUMA_ENTRY_CMD::VVCP_WEIGHTOFFSET_LUMA_ENTRY_CMD()
{
    DW0.Value                                        = 0x00000000;

}

Cmd::VVCP_WEIGHTOFFSET_CHROMA_ENTRY_CMD::VVCP_WEIGHTOFFSET_CHROMA_ENTRY_CMD()
{
    DW0.Value                                        = 0x00000000;

}


Cmd::VVCP_BSD_OBJECT_CMD::VVCP_BSD_OBJECT_CMD()
{
    DW0.Value                                        = 0x73e00001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_VVCPBSDOBJECTSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

}

Cmd::VVCP_DPB_STATE_CMD::VVCP_DPB_STATE_CMD()
{
    DW0.Value                                        = 0x73c40059;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_VVCPDPBSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

}

Cmd::VVCP_IND_OBJ_BASE_ADDR_STATE_CMD::VVCP_IND_OBJ_BASE_ADDR_STATE_CMD()
{
    DW0.Value                                        = 0x73c30002;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_VVCPINDOBJBASEADDRSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

}

Cmd::VVCP_MEM_DATA_ACCESS_CMD::VVCP_MEM_DATA_ACCESS_CMD()
{
    DW0.Value                                        = 0x73cf0004;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_VVCPMEMDATAACCESS;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEFORVVCP;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.DataId                                       = DATA_ID_VVCPUNITDONE;
    //DW1.AccessType                                   = ACCESS_TYPE_READ;

    DW5.Value                                        = 0x00000000;

}

Cmd::VVCP_PIC_STATE_CMD::VVCP_PIC_STATE_CMD()
{
    DW0.Value                                          = 0x73d0001f;
    //DW0.Lengthfield                                  = 0;
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_VVCPPICSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEVVC;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;
    //DW4.SpsChromaFormatIdc                           = 0;
    //DW4.SpsLog2CtuSizeMinus5                         = SPS_LOG2_CTU_SIZE_MINUS5_CTUSIZE32;
    //DW4.SpsBitdepthMinus8                            = SPS_BITDEPTH_MINUS8_8_BIT;
    //DW4.SpsLog2MinLumaCodingBlockSizeMinus2          = SPS_LOG2_MIN_LUMA_CODING_BLOCK_SIZE_MINUS2_4X4PIXELBLOCKSIZE;

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

    DW19.Value                                       = 0x00000000;

    DW20.Value                                       = 0x00000000;

    DW21.Value                                       = 0x00000000;

    DW22.Value                                       = 0x00000000;

    DW23.Value                                       = 0x00000000;

    DW24.Value                                       = 0x00000000;

    DW25.Value                                       = 0x00000000;

    DW26.Value                                       = 0x00000000;

    DW27.Value                                       = 0x00000000;

    DW28.Value                                       = 0x00000000;
}

Cmd::VVCP_REFERENCE_PICTURE_ENTRY_CMD::VVCP_REFERENCE_PICTURE_ENTRY_CMD()
{
    ReferencePictureBaseAddress0Refaddr.DW0_1.Value[0]        = 0x00000000;
    ReferencePictureBaseAddress0Refaddr.DW0_1.Value[1]        = 0x00000000;
    ReferencePictureMemoryAddressAttributes.DW0.Value         = 0x00000000;
}

Cmd::VVCP_PIPE_BUF_ADDR_STATE_CMD::VVCP_PIPE_BUF_ADDR_STATE_CMD()
{
    DW0.Value                                        = 0x73c200b6;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_VVCPPIPEBUFADDRSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

}

Cmd::VVCP_PIPE_MODE_SELECT_CMD::VVCP_PIPE_MODE_SELECT_CMD()
{
    DW0.Value                                        = 0x73c00004;

    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_VVCPPIPEMODESELECT;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.CodecSelect                                  = CODEC_SELECT_DECODE;
    //DW1.PicStatusErrorReportEnable                   = PIC_STATUSERROR_REPORT_ENABLE_DISABLE;
    //DW1.CodecStandardSelect                          = 0;
    //DW1.MultiEngineMode                              = MULTI_ENGINE_MODE_SINGLEENGINEMODE;
    //DW1.PipeWorkingMode                              = PIPE_WORKING_MODE_LEGACYDECODERENCODERMODE_SINGLEPIPE;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;

    DW5.Value                                        = 0x00000000;
}

Cmd::VVCP_REF_IDX_STATE_CMD::VVCP_REF_IDX_STATE_CMD()
{
    DW0.Value                                        = 0x73d2000f;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_VVCPREFIDXSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.Listidx                                      = LISTIDX_REFERENCEPICTURELIST0;

}

Cmd::VVCP_SLICE_STATE_CMD::VVCP_SLICE_STATE_CMD()
{
    DW0.Value                                        = 0x73d4002e;
    //DW0.Lengthfield                                  = 0;
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_VVCPSLICESTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEVVC;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;
    //DW3.ShSliceType                                  = SH_SLICE_TYPE_B_SLICE;

    DW4.Value                                        = 0x00000000;

    DW5.Value                                        = 0x00000000;

    DW6.Value                                        = 0x00000000;

    DW7.Value                                        = 0x00000000;

    DW8.Value                                        = 0x00000000;

    DW9.Value                                        = 0x00000000;

    memset(&DW10_17.Value, 0, sizeof(DW10_17.Value));
    memset(&DW18_25.Value, 0, sizeof(DW18_25.Value));
    memset(&DW26_33.Value, 0, sizeof(DW26_33.Value));
    memset(&Lmcspivot161, 0, sizeof(Lmcspivot161));

    DW42.Value                                       = 0x00000000;

    DW43.Value                                       = 0x00000000;

    DW44.Value                                       = 0x00000000;

    DW45.Value                                       = 0x00000000;

    DW46.Value                                       = 0x00000000;

    DW47.Value                                       = 0x00000000;

}

Cmd::VVCP_SURFACE_STATE_CMD::VVCP_SURFACE_STATE_CMD()
{
    DW0.Value                                        = 0x73c10003;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_VVCPSURFACESTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.SurfaceId                                    = SURFACE_ID_RECONSTRUCTEDPICTURE;

    DW2.Value                                        = 0x00000000;
    //DW2.SurfaceFormat                                = 0;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;

}

Cmd::VVCP_TILE_CODING_CMD::VVCP_TILE_CODING_CMD()
{
    DW0.Value                                        = 0x73d50003;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_VVCPTILECODING;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEVVC;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;

}

Cmd::VVCP_VD_CONTROL_STATE_CMD::VVCP_VD_CONTROL_STATE_CMD()
{
    DW0.Value                                        = 0x73ca0001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_VDCONTROLSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEFORVVCP;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

}

Cmd::VVCP_WEIGHTOFFSET_STATE_CMD::VVCP_WEIGHTOFFSET_STATE_CMD()
{
    DW0.Value                                        = 0x73d3002e;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_VVCPWEIGHTOFFSETSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.Listidx                                      = LISTIDX_REFERENCEPICTURELIST0;

    DW2.Value                                        = 0x00000000;

}

