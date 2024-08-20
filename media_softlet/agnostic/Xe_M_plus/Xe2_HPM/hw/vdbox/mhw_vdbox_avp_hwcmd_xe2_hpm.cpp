/*===================== begin_copyright_notice ==================================
# Copyright (c) 2020-2024, Intel Corporation
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
//! \file   mhw_vdbox_avp_hwcmd_xe2_hpm.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

// DO NOT EDIT

#include "mhw_vdbox_avp_hwcmd_xe2_hpm.h"
#include <string.h>

using namespace mhw::vdbox::avp::xe_lpm_plus_base::v1;

Cmd::MEMORYADDRESSATTRIBUTES_CMD::MEMORYADDRESSATTRIBUTES_CMD()
{
    DW0.Value = 0x00000000;
    //DW0.CompressionType                              = COMPRESSION_TYPE_MEDIACOMPRESSIONENABLE;
    //DW0.BaseAddressRowStoreScratchBufferCacheSelect  = BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED0;
    //DW0.Tilemode                                     = TILEMODE_LINEAR;
    //DW0.MediaLevel2CachingEnable                     = MEDIA_LEVEL_2_CACHING_ENABLE_DISABLEMEDIACACHE;

}

Cmd::SPLITBASEADDRESS4KBYTEALIGNED_CMD::SPLITBASEADDRESS4KBYTEALIGNED_CMD()
{
    DW0_1.Value[0] = DW0_1.Value[1] = 0x00000000;

}

Cmd::SPLITBASEADDRESS64BYTEALIGNED_CMD::SPLITBASEADDRESS64BYTEALIGNED_CMD()
{
    DW0_1.Value[0] = DW0_1.Value[1] = 0x00000000;

}

Cmd::VD_CONTROL_STATE_BODY_CMD::VD_CONTROL_STATE_BODY_CMD()
{
    DW0.Value = 0x00000000;

    DW1.Value = 0x00000000;

}

Cmd::AVP_BSD_OBJECT_CMD::AVP_BSD_OBJECT_CMD()
{
    DW0.Value = 0x71a00001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPBSDOBJECTSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;

}

Cmd::AVP_PIC_STATE_CMD::AVP_PIC_STATE_CMD()
{
    DW0.Value = 0x71b0004a;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPPICSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;
    //DW2.SequenceChromaSubsamplingFormat              = 0;

    DW3.Value = 0x00000000;
    //DW3.ErrorResilientModeFlag                       = ERROR_RESILIENT_MODE_FLAG_DISABLE;

    DW4.Value = 0x00000000;
    //DW4.SegmentationEnableFlag                       = SEGMENTATION_ENABLE_FLAG_ALLBLOCKSAREIMPLIEDTOBELONGTOSEGMENT0;
    //DW4.SegmentationTemporalUpdateFlag               = SEGMENTATION_TEMPORAL_UPDATE_FLAG_DECODESEGIDFROMBITSTREAM;
    //DW4.FrameCodedLosslessMode                       = FRAME_CODED_LOSSLESS_MODE_NORMALMODE;

    DW5.Value = 0x00000000;

    DW6.Value = 0x00000000;
    //DW6.McompFilterType                              = MCOMP_FILTER_TYPE_EIGHT_TAP;

    DW7.Value = 0x00000000;

    DW8.Value = 0x00000000;

    memset(&WarpParametersArrayReference1To7Projectioncoeff0To5, 0, sizeof(WarpParametersArrayReference1To7Projectioncoeff0To5));

    DW30.Value = 0x00000000;

    DW31.Value = 0x00000000;

    DW32.Value = 0x00000000;

    DW33.Value = 0x00000000;

    DW34.Value = 0x00000000;

    DW35.Value = 0x00000000;

    DW36.Value = 0x00000000;

    DW37.Value = 0x00000000;

    DW38.Value = 0x00000000;

    DW39.Value = 0x00000000;

    DW40.Value = 0x00000000;

    DW41.Value = 0x00000000;

    DW42.Value = 0x00000000;

    DW43.Value = 0x00000000;

    DW44.Value = 0x00000000;

    DW45.Value = 0x00000000;

    DW46.Value = 0x00000000;

    DW47.Value = 0x00000000;

    DW48.Value = 0x00000000;

    DW49.Value = 0x00000000;

    DW50.Value = 0x00000000;

    DW51.Value = 0x00000000;
    //DW51.Nonfirstpassflag                            = NONFIRSTPASSFLAG_DISABLE;
    //DW51.FrameszoverstatusenFramebitratemaxreportmask = FRAMESZOVERSTATUSEN_FRAMEBITRATEMAXREPORTMASK_DISABLE;
    //DW51.FrameszunderstatusenFramebitrateminreportmask = FRAMESZUNDERSTATUSEN_FRAMEBITRATEMINREPORTMASK_DISABLE;

    DW52.Value = 0x00000000;
    //DW52.Framebitratemaxunit                         = FRAMEBITRATEMAXUNIT_BYTE;

    DW53.Value = 0x00000000;
    //DW53.Framebitrateminunit                         = FRAMEBITRATEMINUNIT_BYTE;

    DW54_55.Value[0] = DW54_55.Value[1] = 0x00000000;

    DW56.Value = 0x00000000;

    DW57_58.Value[0] = DW57_58.Value[1] = 0x00000000;

    DW59.Value = 0x00000000;

    DW60_61.Value[0] = DW60_61.Value[1] = 0x00000000;

    DW62.Value = 0x00000000;

    DW63.Value = 0x00000000;
    //DW63.Minframesizeunits                           = MINFRAMESIZEUNITS_4KB;

    DW64.Value = 0x00000000;

    DW65.Value = 0x00000000;

    memset(&SseThresholdsForClass18, 0, sizeof(SseThresholdsForClass18));

    DW74.Value = 0x00000000;

    DW75.Value = 0x00000000;

}

Cmd::AVP_PIPE_MODE_SELECT_CMD::AVP_PIPE_MODE_SELECT_CMD()
{
    DW0.Value = 0x71800005;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPPIPEMODESELECT;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.CodecSelect                                  = CODEC_SELECT_DECODE;
    //DW1.PicStatusErrorReportEnable                   = PIC_STATUSERROR_REPORT_ENABLE_DISABLE;
    //DW1.CodecStandardSelect                          = 0;
    //DW1.MultiEngineMode                              = MULTI_ENGINE_MODE_SINGLEENGINEMODEORMSACFEONLYDECODEMODE;
    //DW1.PipeWorkingMode                              = PIPE_WORKING_MODE_LEGACYDECODERENCODERMODE_SINGLEPIPE;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;

    DW4.Value = 0x00000000;

    DW5.Value = 0x00000000;

    DW6.Value = 0x00000000;

}

Cmd::AVP_IND_OBJ_BASE_ADDR_STATE_CMD::AVP_IND_OBJ_BASE_ADDR_STATE_CMD()
{
    DW0.Value = 0x71830007;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPINDOBJBASEADDRSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

}

Cmd::AVP_TILE_CODING_CMD::AVP_TILE_CODING_CMD()
{
    DW0.Value = 0x71950005;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPTILECODING;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;

    DW4.Value = 0x00000000;

    DW5.Value = 0x00000000;

    DW6.Value = 0x00000000;

}


Cmd::AVP_SURFACE_STATE_CMD::AVP_SURFACE_STATE_CMD()
{
    DW0.Value = 0x71810003;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_SURFACESTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.SurfaceId                                    = SURFACE_ID_RECONSTRUCTEDPICTURE;

    DW2.Value = 0x00000000;
    //DW2.VariantFormatLsbPackedEnable                 = VARIANT_FORMAT_LSB_PACKED_ENABLE_LSBUNPACKED;
    //DW2.SurfaceFormat                                = 0;

    DW3.Value = 0x00000000;

    DW4.Value = 0x00000000;
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

}

Cmd::AVP_SEGMENT_STATE_CMD::AVP_SEGMENT_STATE_CMD()
{
    DW0.Value = 0x71b20002;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPSEGMENTSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;

}

Cmd::AVP_PIPE_BUF_ADDR_STATE_CMD::AVP_PIPE_BUF_ADDR_STATE_CMD()
{
    DW0.Value = 0x718200d5;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPPIPEBUFADDRSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    memset(&Reserved3904, 0, sizeof(Reserved3904));

    memset(&Reserved4192, 0, sizeof(Reserved4192));

    memset(&Reserved5344, 0, sizeof(Reserved5344));

    memset(&Reserved5824, 0, sizeof(Reserved5824));

    memset(&Reserved5920, 0, sizeof(Reserved5920));

}

Cmd::AVP_INLOOP_FILTER_STATE_CMD::AVP_INLOOP_FILTER_STATE_CMD()
{
    DW0.Value = 0x71b3000d;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPINLOOPFILTERSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;

    DW4.Value = 0x00000000;

    DW5.Value = 0x00000000;

    DW6.Value = 0x00000000;

    DW7.Value = 0x00000000;

    DW8.Value = 0x00000000;

    DW9.Value = 0x00000000;

    DW10.Value = 0x00000000;

    DW11.Value = 0x00000000;

    DW12.Value = 0x00000000;

    DW13.Value = 0x00000000;

    DW14.Value = 0x00000000;

}

Cmd::AVP_INTER_PRED_STATE_CMD::AVP_INTER_PRED_STATE_CMD()
{
    DW0.Value = 0x7192000d;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPINTERPREDSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;

    DW4.Value = 0x00000000;

    DW5.Value = 0x00000000;

    DW6.Value = 0x00000000;

    DW7.Value = 0x00000000;

    DW8.Value = 0x00000000;

    DW9.Value = 0x00000000;

    DW10.Value = 0x00000000;

    DW11.Value = 0x00000000;

    DW12.Value = 0x00000000;

    DW13.Value = 0x00000000;

    DW14.Value = 0x00000000;

}

Cmd::AVP_FILM_GRAIN_STATE_CMD::AVP_FILM_GRAIN_STATE_CMD()
{
    DW0.Value = 0x71b4002b;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPFILMGRAINSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.Type                                         = TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;

    memset(&PointLumaValueI0To13, 0, sizeof(PointLumaValueI0To13));

    memset(&PointLumaScalingI0To13, 0, sizeof(PointLumaScalingI0To13));

    memset(&PointCbValueI0To9, 0, sizeof(PointCbValueI0To9));

    memset(&PointCbScalingI0To9, 0, sizeof(PointCbScalingI0To9));

    memset(&PointCrValueI0To9, 0, sizeof(PointCrValueI0To9));

    memset(&PointCrScalingI0To9, 0, sizeof(PointCrScalingI0To9));

    memset(&ArCoeffLumaPlus128I023, 0, sizeof(ArCoeffLumaPlus128I023));

    memset(&ArCoeffChromaCbPlus128I024, 0, sizeof(ArCoeffChromaCbPlus128I024));

    memset(&ArCoeffChromaCrPlus128I024, 0, sizeof(ArCoeffChromaCrPlus128I024));

    DW43.Value = 0x00000000;

    DW44.Value = 0x00000000;
}

Cmd::AVP_PAK_INSERT_OBJECT_CMD::AVP_PAK_INSERT_OBJECT_CMD()
{
    DW0.Value = 0x71a20000;
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPPAKINSERTOBJECT;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;
    //DW1.IndirectPayloadEnable                        = INDIRECT_PAYLOAD_ENABLE_INLINEPAYLOADISUSED;
}


Cmd::AVP_PAK_OBJECT_CMD::AVP_PAK_OBJECT_CMD()
{
    DW0.Value = 0x71a10003;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPPAKOBJECT;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;

    DW4.Value = 0x00000000;

}

Cmd::AVP_VD_CONTROL_STATE_CMD::AVP_VD_CONTROL_STATE_CMD()
{
    DW0.Value = 0x718a0001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_VDCONTROLSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEFORAVP;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

}
