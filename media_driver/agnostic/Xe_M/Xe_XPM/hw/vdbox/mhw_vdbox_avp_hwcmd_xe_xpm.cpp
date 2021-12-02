
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
//! \file   mhw_vdbox_avp_hwcmd_xe_xpm.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

// DO NOT EDIT

#include "mhw_vdbox_avp_hwcmd_xe_xpm.h"
#include <string.h>

mhw_vdbox_avp_xe_xpm::MEMORYADDRESSATTRIBUTES_CMD::MEMORYADDRESSATTRIBUTES_CMD()
{
    DW0.Value                                        = 0x00000000;
    //DW0.CompressionType                              = COMPRESSION_TYPE_MEDIACOMPRESSIONENABLE;
    //DW0.BaseAddressRowStoreScratchBufferCacheSelect  = BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED0;
    //DW0.Tilemode                                     = TILEMODE_LINEAR;

}

mhw_vdbox_avp_xe_xpm::SPLITBASEADDRESS4KBYTEALIGNED_CMD::SPLITBASEADDRESS4KBYTEALIGNED_CMD()
{
    DW0_1.Value[0] = DW0_1.Value[1]                  = 0x00000000;

}

mhw_vdbox_avp_xe_xpm::SPLITBASEADDRESS64BYTEALIGNED_CMD::SPLITBASEADDRESS64BYTEALIGNED_CMD()
{
    DW0_1.Value[0] = DW0_1.Value[1]                  = 0x00000000;

}

mhw_vdbox_avp_xe_xpm::AVP_BSD_OBJECT_CMD::AVP_BSD_OBJECT_CMD()
{
    DW0.Value                                        = 0x71a00001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPBSDOBJECTSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

}

mhw_vdbox_avp_xe_xpm::AVP_PIC_STATE_CMD::AVP_PIC_STATE_CMD()
{
    DW0.Value                                        = 0x71b00031;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPPICSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;
    //DW2.SequenceChromaSubsamplingFormat              = 0;

    DW3.Value                                        = 0x00000000;
    //DW3.ErrorResilientModeFlag                       = ERROR_RESILIENT_MODE_FLAG_DISABLE;

    DW4.Value                                        = 0x00000000;
    //DW4.SegmentationEnableFlag                       = SEGMENTATION_ENABLE_FLAG_ALLBLOCKSAREIMPLIEDTOBELONGTOSEGMENT0;
    //DW4.SegmentationTemporalUpdateFlag               = SEGMENTATION_TEMPORAL_UPDATE_FLAG_DECODESEGIDFROMBITSTREAM;
    //DW4.FrameCodedLosslessMode                       = FRAME_CODED_LOSSLESS_MODE_NORMALMODE;

    DW5.Value                                        = 0x00000000;

    DW6.Value                                        = 0x00000000;
    //DW6.McompFilterType                              = MCOMP_FILTER_TYPE_EIGHT_TAP;

    DW7.Value                                        = 0x00000000;

    DW8.Value                                        = 0x00000000;

    memset(&WarpParametersArrayReference1To7Projectioncoeff0To5, 0, sizeof(WarpParametersArrayReference1To7Projectioncoeff0To5));

    DW30.Value                                       = 0x00000000;

    DW31.Value                                       = 0x00000000;

    DW32.Value                                       = 0x00000000;

    DW33.Value                                       = 0x00000000;

    DW34.Value                                       = 0x00000000;

    DW35.Value                                       = 0x00000000;

    DW36.Value                                       = 0x00000000;

    DW37.Value                                       = 0x00000000;

    DW38.Value                                       = 0x00000000;

    DW39.Value                                       = 0x00000000;

    DW40.Value                                       = 0x00000000;

    DW41.Value                                       = 0x00000000;

    DW42.Value                                       = 0x00000000;

    DW43.Value                                       = 0x00000000;

    DW44.Value                                       = 0x00000000;

    DW45.Value                                       = 0x00000000;

    DW46.Value                                       = 0x00000000;

    DW47.Value                                       = 0x00000000;

    DW48.Value                                       = 0x00000000;

    DW49.Value                                       = 0x00000000;

    DW50.Value                                       = 0x00000000;

}

mhw_vdbox_avp_xe_xpm::AVP_PIPE_MODE_SELECT_CMD::AVP_PIPE_MODE_SELECT_CMD()
{
    DW0.Value                                        = 0x71800005;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPPIPEMODESELECT;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.CodecSelect                                  = CODEC_SELECT_DECODE;
    //DW1.CdefOutputStreamoutEnableFlag                = CDEF_OUTPUT_STREAMOUT_ENABLE_FLAG_DISABLE;
    //DW1.LoopRestorationOutputStreamoutEnableFlag     = LOOP_RESTORATION_OUTPUT_STREAMOUT_ENABLE_FLAG_DISABLE;
    //DW1.PicStatusErrorReportEnable                   = PIC_STATUSERROR_REPORT_ENABLE_DISABLE;
    //DW1.CodecStandardSelect                          = CODEC_STANDARD_SELECT_HEVC;
    //DW1.MultiEngineMode                              = MULTI_ENGINE_MODE_SINGLEENGINEMODEORMSACFEONLYDECODEMODE;
    //DW1.PipeWorkingMode                              = PIPE_WORKING_MODE_LEGACYDECODERENCODERMODE_SINGLEPIPE;

    DW2.Value                                        = 0x00000000;
    //DW2.MediaSoftResetCounterPer1000Clocks           = MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS_DISABLE;

    DW3.Value                                        = 0x00000000;
    //DW3.PicStatusErrorReportId                       = PIC_STATUSERROR_REPORT_ID_32_BITUNSIGNED;

    DW4.Value                                        = 0x00000000;

    DW5.Value                                        = 0x00000000;
    //DW5.PhaseIndicator                               = PHASE_INDICATOR_FIRSTPHASE;

    DW6.Value                                        = 0x00000000;

}

mhw_vdbox_avp_xe_xpm::AVP_IND_OBJ_BASE_ADDR_STATE_CMD::AVP_IND_OBJ_BASE_ADDR_STATE_CMD()
{
    DW0.Value                                        = 0x71830004;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPINDOBJBASEADDRSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

}

mhw_vdbox_avp_xe_xpm::AVP_TILE_CODING_CMD::AVP_TILE_CODING_CMD()
{
    DW0.Value                                        = 0x71950004;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPTILECODING;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;
    //DW4.FilmGrainSampleTemplateWriteReadControl      = FILM_GRAIN_SAMPLE_TEMPLATE_WRITEREAD_CONTROL_READ;

    DW5.Value                                        = 0x00000000;

}

mhw_vdbox_avp_xe_xpm::AVP_SURFACE_STATE_CMD::AVP_SURFACE_STATE_CMD()
{
    DW0.Value                                        = 0x71810003;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_SURFACESTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.SurfaceId                                    = SURFACE_ID_RECONSTRUCTEDPICTURE;

    DW2.Value                                        = 0x00000000;
    //DW2.SurfaceFormat                                = 0;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;
    //DW4.MemoryCompressionEnableForAv1IntraFrame      = MEMORY_COMPRESSION_ENABLE_FOR_AV1_INTRA_FRAME_MEMORYCOMPRESSIONDISABLE;
    //DW4.MemoryCompressionEnableForAv1LastFrame       = MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST_FRAME_MEMORYCOMPRESSIONDISABLE;
    //DW4.MemoryCompressionEnableForAv1Last2Frame      = MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST2_FRAME_MEMORYCOMPRESSIONDISABLE;
    //DW4.MemoryCompressionEnableForAv1Last3Frame      = MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST3_FRAME_MEMORYCOMPRESSIONDISABLE;
    //DW4.MemoryCompressionEnableForAv1GoldenFrame     = MEMORY_COMPRESSION_ENABLE_FOR_AV1_GOLDEN_FRAME_MEMORYCOMPRESSIONDISABLE;
    //DW4.MemoryCompressionEnableForAv1BwdrefFrame     = MEMORY_COMPRESSION_ENABLE_FOR_AV1_BWDREF_FRAME_MEMORYCOMPRESSIONDISABLE;
    //DW4.MemoryCompressionEnableForAv1Altref2Frame    = MEMORY_COMPRESSION_ENABLE_FOR_AV1_ALTREF2_FRAME_MEMORYCOMPRESSIONDISABLE;
    //DW4.MemoryCompressionEnableForAv1AltrefFrame     = MEMORY_COMPRESSION_ENABLE_FOR_AV1_ALTREF_FRAME_MEMORYCOMPRESSIONDISABLE;
    //DW4.CompressionTypeForIntraFrame                 = COMPRESSION_TYPE_FOR_INTRA_FRAME_MEDIACOMPRESSIONENABLED;
    //DW4.CompressionTypeForLastFrame                  = COMPRESSION_TYPE_FOR_LAST_FRAME_MEDIACOMPRESSIONENABLED;
    //DW4.CompressionTypeForLast2Frame                 = COMPRESSION_TYPE_FOR_LAST2_FRAME_MEDIACOMPRESSIONENABLED;
    //DW4.CompressionTypeForLast3Frame                 = COMPRESSION_TYPE_FOR_LAST3_FRAME_MEDIACOMPRESSIONENABLED;
    //DW4.CompressionTypeForGoldenFrame                = COMPRESSION_TYPE_FOR_GOLDEN_FRAME_MEDIACOMPRESSIONENABLED;
    //DW4.CompressionTypeForBwdrefFrame                = COMPRESSION_TYPE_FOR_BWDREF_FRAME_MEDIACOMPRESSIONENABLED;
    //DW4.CompressionTypeForAltref2Frame               = COMPRESSION_TYPE_FOR_ALTREF2_FRAME_MEDIACOMPRESSIONENABLED;
    //DW4.CompressionTypeForAltrefFrame                = COMPRESSION_TYPE_FOR_ALTREF_FRAME_MEDIACOMPRESSIONENABLED;
    //DW4.CompressionFormat                            = 0;

}

mhw_vdbox_avp_xe_xpm::AVP_SEGMENT_STATE_CMD::AVP_SEGMENT_STATE_CMD()
{
    DW0.Value                                        = 0x71b20002;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPSEGMENTSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

}

mhw_vdbox_avp_xe_xpm::AVP_PIPE_BUF_ADDR_STATE_CMD::AVP_PIPE_BUF_ADDR_STATE_CMD()
{
    DW0.Value                                        = 0x718200ba;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPPIPEBUFADDRSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    memset(&Reserved672, 0, sizeof(Reserved672));

    memset(&Reserved1888, 0, sizeof(Reserved1888));

    memset(&Reserved3904, 0, sizeof(Reserved3904));

    memset(&Reserved4192, 0, sizeof(Reserved4192));

    memset(&Reserved5344, 0, sizeof(Reserved5344));

    memset(&Reserved5824, 0, sizeof(Reserved5824));

    memset(&Reserved5920, 0, sizeof(Reserved5920));

}

mhw_vdbox_avp_xe_xpm::AVP_INLOOP_FILTER_STATE_CMD::AVP_INLOOP_FILTER_STATE_CMD()
{
    DW0.Value                                        = 0x71b3000d;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPINLOOPFILTERSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;

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

}

mhw_vdbox_avp_xe_xpm::AVP_INTER_PRED_STATE_CMD::AVP_INTER_PRED_STATE_CMD()
{
    DW0.Value                                        = 0x7192000d;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPINTERPREDSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;

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

}

