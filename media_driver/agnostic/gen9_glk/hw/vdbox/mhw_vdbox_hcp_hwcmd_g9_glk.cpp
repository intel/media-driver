/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file   mhw_vdbox_hcp_hwcmd_g9_glk.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

// DO NOT EDIT

#include "mhw_vdbox_hcp_hwcmd_g9_glk.h"
#include <string.h>

mhw_vdbox_hcp_g9_glk::MEMORYADDRESSATTRIBUTES_CMD::MEMORYADDRESSATTRIBUTES_CMD()
{
    DW0.Value                                        = 0x00000000;
    //DW0.BaseAddressRowStoreScratchBufferCacheSelect  = BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED0;
    //DW0.BaseAddressTiledResourceMode                 = BASE_ADDRESS_TILED_RESOURCE_MODE_TRMODENONE;

}

mhw_vdbox_hcp_g9_glk::SPLITBASEADDRESS64BYTEALIGNED_CMD::SPLITBASEADDRESS64BYTEALIGNED_CMD()
{
    DW0_1.Value[0] = DW0_1.Value[1]                  = 0x00000000;

}

mhw_vdbox_hcp_g9_glk::SPLITBASEADDRESS4KBYTEALIGNED_CMD::SPLITBASEADDRESS4KBYTEALIGNED_CMD()
{
    DW0_1.Value[0] = DW0_1.Value[1]                  = 0x00000000;

}

mhw_vdbox_hcp_g9_glk::HCP_PIPE_MODE_SELECT_CMD::HCP_PIPE_MODE_SELECT_CMD()
{
    DW0.Value                                        = 0x73800004;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPPIPEMODESELECT;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.CodecSelect                                  = CODEC_SELECT_DECODE;
    //DW1.DeblockerStreamoutEnable                     = DEBLOCKER_STREAMOUT_ENABLE_DISABLE;
    //DW1.PakPipelineStreamoutEnable                   = PAK_PIPELINE_STREAMOUT_ENABLE_DISABLEPIPELINESTATESANDPARAMETERSSTREAMOUT;
    //DW1.PicStatusErrorReportEnable                   = PIC_STATUSERROR_REPORT_ENABLE_DISABLE;
    //DW1.CodecStandardSelect                          = CODEC_STANDARD_SELECT_HEVC;

    DW2.Value                                        = 0x00000000;
    //DW2.MediaSoftResetCounterPer1000Clocks           = MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS_DISABLE;

    DW3.Value                                        = 0x00000000;
    //DW3.PicStatusErrorReportId                       = PIC_STATUSERROR_REPORT_ID_32_BITUNSIGNED;

    DW4.Value                                        = 0x00000000;

    DW5.Value                                        = 0x00000000;

}

mhw_vdbox_hcp_g9_glk::HCP_SURFACE_STATE_CMD::HCP_SURFACE_STATE_CMD()
{
    DW0.Value                                        = 0x73810001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPSURFACESTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.SurfaceId                                    = SURFACE_ID_HEVCFORCURRENTDECODEDPICTURE;

    DW2.Value                                        = 0x00000000;
    //DW2.SurfaceFormat                                = 0;

}

mhw_vdbox_hcp_g9_glk::HCP_PIPE_BUF_ADDR_STATE_CMD::HCP_PIPE_BUF_ADDR_STATE_CMD()
{
    DW0.Value                                        = 0x7382005d;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPPIPEBUFADDRSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW34_35.Value[0] = DW34_35.Value[1]              = 0x00000000;

    DW36.Value                                       = 0x00000000;

    DW86_87.Value[0] = DW86_87.Value[1]              = 0x00000000;

}

mhw_vdbox_hcp_g9_glk::HCP_IND_OBJ_BASE_ADDR_STATE_CMD::HCP_IND_OBJ_BASE_ADDR_STATE_CMD()
{
    DW0.Value                                        = 0x7383000c;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPINDOBJBASEADDRSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW6_7.Value[0] = DW6_7.Value[1]                  = 0x00000000;

    DW9_10.Value[0] = DW9_10.Value[1]                = 0x00000000;

}

mhw_vdbox_hcp_g9_glk::HCP_QM_STATE_CMD::HCP_QM_STATE_CMD()
{
    DW0.Value                                        = 0x73840010;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPQMSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.PredictionType                               = PREDICTION_TYPE_INTRA;
    //DW1.Sizeid                                       = SIZEID_4X4;
    //DW1.ColorComponent                               = COLOR_COMPONENT_LUMA;

    memset(&Quantizermatrix, 0, sizeof(Quantizermatrix));

}

mhw_vdbox_hcp_g9_glk::HCP_PIC_STATE_CMD::HCP_PIC_STATE_CMD()
{
    DW0.Value                                        = 0x73900011;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPPICSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;
    //DW2.Mincusize                                    = MINCUSIZE_8X8;
    //DW2.CtbsizeLcusize                               = CTBSIZE_LCUSIZE_ILLEGALRESERVED;
    //DW2.Mintusize                                    = MINTUSIZE_4X4;
    //DW2.Maxtusize                                    = MAXTUSIZE_4X4;
    //DW2.Minpcmsize                                   = MINPCMSIZE_8X8;
    //DW2.Maxpcmsize                                   = MAXPCMSIZE_8X8;

    DW3.Value                                        = 0x00000000;
    //DW3.Colpicisi                                    = COLPICISI_COLLOCATEDPICTUREHASATLEASTONEPORBSLICE;
    //DW3.Curpicisi                                    = CURPICISI_CURRENTPICTUREHASATLEASTONEPORBSLICE;
    //DW3.Inserttestflag                               = INSERTTESTFLAG_UNNAMED0;

    DW4.Value                                        = 0x00000000;
    //DW4.CuQpDeltaEnabledFlag                         = CU_QP_DELTA_ENABLED_FLAG_DISABLE;
    //DW4.SignDataHidingFlag                           = SIGN_DATA_HIDING_FLAG_DISABLE;
    //DW4.Fieldpic                                     = FIELDPIC_VIDEOFRAME;
    //DW4.Bottomfield                                  = BOTTOMFIELD_BOTTOMFIELD;
    //DW4.TransformSkipEnabledFlag                     = TRANSFORM_SKIP_ENABLED_FLAG_DISABLE;
    //DW4.AmpEnabledFlag                               = AMP_ENABLED_FLAG_DISABLE;
    //DW4.TransquantBypassEnableFlag                   = TRANSQUANT_BYPASS_ENABLE_FLAG_DISABLE;

    DW5.Value                                        = 0x00000000;
    //DW5.BitDepthChromaMinus8                         = BIT_DEPTH_CHROMA_MINUS8_CHROMA8BIT;
    //DW5.BitDepthLumaMinus8                           = BIT_DEPTH_LUMA_MINUS8_LUMA8BIT;

    DW6.Value                                        = 0x00000000;
    //DW6.LcumaxbitstatusenLcumaxsizereportmask        = LCUMAXBITSTATUSEN_LCUMAXSIZEREPORTMASK_DISABLE;
    //DW6.FrameszoverstatusenFramebitratemaxreportmask = FRAMESZOVERSTATUSEN_FRAMEBITRATEMAXREPORTMASK_DISABLE;
    //DW6.FrameszunderstatusenFramebitrateminreportmask = FRAMESZUNDERSTATUSEN_FRAMEBITRATEMINREPORTMASK_DISABLE;
    //DW6.LoadSlicePointerFlag                         = LOAD_SLICE_POINTER_FLAG_DISABLE;

    DW7.Value                                        = 0x00000000;
    //DW7.Framebitratemaxunit                          = FRAMEBITRATEMAXUNIT_BYTE;

    DW8.Value                                        = 0x00000000;
    //DW8.Framebitrateminunit                          = FRAMEBITRATEMINUNIT_BYTE;

    DW9.Value                                        = 0x00000000;
    //DW9.Framebitratemindelta                         = FRAMEBITRATEMINDELTA_UNNAMED0;
    //DW9.Framebitratemaxdelta                         = FRAMEBITRATEMAXDELTA_UNNAMED0;

    DW10_11.Value[0] = DW10_11.Value[1]              = 0x00000000;

    DW12_13.Value[0] = DW12_13.Value[1]              = 0x00000000;

    DW14_15.Value[0] = DW14_15.Value[1]              = 0x00000000;

    DW16_17.Value[0] = DW16_17.Value[1]              = 0x00000000;

    DW18.Value                                       = 0x00000000;
    //DW18.Minframesize                                = MINFRAMESIZE_UNNAMED0;
    //DW18.Minframesizeunits                           = MINFRAMESIZEUNITS_4KB;

}

mhw_vdbox_hcp_g9_glk::HCP_TILE_POSITION_IN_CTB_CMD::HCP_TILE_POSITION_IN_CTB_CMD()
{
    DW0.Value                                        = 0x00000000;

}

mhw_vdbox_hcp_g9_glk::HCP_TILE_STATE_CMD::HCP_TILE_STATE_CMD()
{
    DW0.Value                                        = 0x7391000b;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPTILESTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

}

mhw_vdbox_hcp_g9_glk::HCP_REF_LIST_ENTRY_CMD::HCP_REF_LIST_ENTRY_CMD()
{
    DW0.Value                                        = 0x00000000;
    //DW0.ChromaWeightLxFlag                           = CHROMA_WEIGHT_LX_FLAG_DEFAULTWEIGHTEDPREDICTIONFORCHROMA;
    //DW0.LumaWeightLxFlag                             = LUMA_WEIGHT_LX_FLAG_DEFAULTWEIGHTEDPREDICTIONFORLUMA;
    //DW0.Longtermreference                            = LONGTERMREFERENCE_SHORTTERMREFERENCE;
    //DW0.FieldPicFlag                                 = FIELD_PIC_FLAG_VIDEOFRAME;
    //DW0.BottomFieldFlag                              = BOTTOM_FIELD_FLAG_BOTTOMFIELD;

}

mhw_vdbox_hcp_g9_glk::HCP_REF_IDX_STATE_CMD::HCP_REF_IDX_STATE_CMD()
{
    DW0.Value                                        = 0x73920010;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPREFIDXSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.Refpiclistnum                                = REFPICLISTNUM_REFERENCEPICTURELIST0;

}

mhw_vdbox_hcp_g9_glk::HCP_WEIGHTOFFSET_LUMA_ENTRY_CMD::HCP_WEIGHTOFFSET_LUMA_ENTRY_CMD()
{
    DW0.Value                                        = 0x00000000;

}

mhw_vdbox_hcp_g9_glk::HCP_WEIGHTOFFSET_CHROMA_ENTRY_CMD::HCP_WEIGHTOFFSET_CHROMA_ENTRY_CMD()
{
    DW0.Value                                        = 0x00000000;

}

mhw_vdbox_hcp_g9_glk::HCP_WEIGHTOFFSET_STATE_CMD::HCP_WEIGHTOFFSET_STATE_CMD()
{
    DW0.Value                                        = 0x73930020;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPWEIGHTOFFSETSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.Refpiclistnum                                = REFPICLISTNUM_REFERENCEPICTURELIST0;

}

mhw_vdbox_hcp_g9_glk::HCP_SLICE_STATE_CMD::HCP_SLICE_STATE_CMD()
{
    DW0.Value                                        = 0x73940009;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPSLICESTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;
    //DW3.SliceType                                    = SLICE_TYPE_B_SLICE;
    //DW3.Lastsliceofpic                               = LASTSLICEOFPIC_NOTTHELASTSLICEOFTHEPICTURE;
    //DW3.SliceCbQpOffset                              = SLICE_CB_QP_OFFSET_0;
    //DW3.SliceCrQpOffset                              = SLICE_CR_QP_OFFSET_0;

    DW4.Value                                        = 0x00000000;
    //DW4.Maxmergeidx                                  = MAXMERGEIDX_0;

    DW5.Value                                        = 0x00000000;

    DW6.Value                                        = 0x10400000;
    //DW6.Roundintra                                   = ROUNDINTRA_532;
    //DW6.Roundinter                                   = ROUNDINTER_532;

    DW7.Value                                        = 0x00000000;
    //DW7.Cabaczerowordinsertionenable                 = CABACZEROWORDINSERTIONENABLE_UNNAMED0;
    //DW7.Emulationbytesliceinsertenable               = EMULATIONBYTESLICEINSERTENABLE_OUTPUTTINGRBSP;
    //DW7.TailInsertionEnable                          = TAIL_INSERTION_ENABLE_UNNAMED0;
    //DW7.SlicedataEnable                              = SLICEDATA_ENABLE_UNNAMED0;
    //DW7.HeaderInsertionEnable                        = HEADER_INSERTION_ENABLE_UNNAMED0;

    DW8.Value                                        = 0x00000000;

    DW9.Value                                        = 0x00000000;

    DW10.Value                                       = 0x00000000;

}

mhw_vdbox_hcp_g9_glk::HCP_BSD_OBJECT_CMD::HCP_BSD_OBJECT_CMD()
{
    DW0.Value                                        = 0x73a00001;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPBSDOBJECTSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

}

mhw_vdbox_hcp_g9_glk::HCP_VP9_SEGMENT_STATE_CMD::HCP_VP9_SEGMENT_STATE_CMD()
{
    DW0.Value                                        = 0x73b20005;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPVP9SEGMENTSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;

    DW5.Value                                        = 0x00000000;

    DW6.Value                                        = 0x00000000;

}

mhw_vdbox_hcp_g9_glk::HCP_FQM_STATE_CMD::HCP_FQM_STATE_CMD()
{
    DW0.Value                                        = 0x73850020;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPFQMSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.IntraInter                                   = INTRAINTER_INTRA;
    //DW1.Sizeid                                       = SIZEID_SIZEID04X4;
    //DW1.ColorComponent                               = COLOR_COMPONENT_LUMA;

    memset(&Quantizermatrix, 0, sizeof(Quantizermatrix));

}

mhw_vdbox_hcp_g9_glk::HCP_PAK_INSERT_OBJECT_CMD::HCP_PAK_INSERT_OBJECT_CMD()
{
    DW0.Value                                        = 0x73a20000;
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPPAKINSERTOBJECT;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.EmulationflagEmulationbytebitsinsertenable   = 0;
    //DW1.Headerlengthexcludefrmsize                   = HEADERLENGTHEXCLUDEFRMSIZE_ALLBITSACCUMULATED;
    //DW1.IndirectPayloadEnable                        = INDIRECT_PAYLOAD_ENABLE_INLINEPAYLOADISUSED;

}

mhw_vdbox_hcp_g9_glk::HCP_VP9_PIC_STATE_CMD::HCP_VP9_PIC_STATE_CMD()
{
    DW0.Value                                        = 0x73b00011;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPVP9PICSTATE;
    //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;
    //DW2.FrameType                                    = FRAME_TYPE_KEYFRAME;
    //DW2.AdaptProbabilitiesFlag                       = ADAPT_PROBABILITIES_FLAG_0DONOTADAPT_ERRORRESILIENTORFRAMEPARALLELMODEARESET;
    //DW2.AllowHiPrecisionMv                           = ALLOW_HI_PRECISION_MV_NORMALMODE;
    //DW2.McompFilterType                              = MCOMP_FILTER_TYPE_EIGHT_TAP;
    //DW2.HybridPredictionMode                         = HYBRID_PREDICTION_MODE_COMPPREDICTIONMODEHYBRID_ENCODERDOESNOTPACKCOMPPREDMODEINTERPREDCOMPINPAKOBJINTOBITSTREAM;
    //DW2.SelectableTxMode                             = SELECTABLE_TX_MODE_ENCODERDOESNOTPACKTUSIZEINTOBITSTREAMTHISHELPSREDUCEBITSTREAMSIZEFURTHER;
    //DW2.LastFrameType                                = LAST_FRAME_TYPE_KEYFRAME;
    //DW2.RefreshFrameContext                          = REFRESH_FRAME_CONTEXT_DISABLE;
    //DW2.ErrorResilientMode                           = ERROR_RESILIENT_MODE_DISABLE;
    //DW2.FrameParallelDecodingMode                    = FRAME_PARALLEL_DECODING_MODE_DISABLE;
    //DW2.SegmentationEnabled                          = SEGMENTATION_ENABLED_ALLBLOCKSAREIMPLIEDTOBELONGTOSEGMENT0;
    //DW2.SegmentationUpdateMap                        = SEGMENTATION_UPDATE_MAP_UNNAMED0;
    //DW2.SegmentationTemporalUpdate                   = SEGMENTATION_TEMPORAL_UPDATE_DECODESEGIDFROMBITSTREAM;
    //DW2.LosslessMode                                 = LOSSLESS_MODE_NORMALMODE;
    //DW2.SegmentIdStreamoutEnable                     = SEGMENT_ID_STREAMOUT_ENABLE_DISABLE;
    //DW2.SegmentIdStreaminEnable                      = SEGMENT_ID_STREAMIN_ENABLE_DISABLE;

    DW3.Value                                        = 0x00000000;
    //DW3.Log2TileColumn                               = LOG2_TILE_COLUMN_1TILECOLUMN;
    //DW3.Log2TileRow                                  = LOG2_TILE_ROW_1TILEROW;
    //DW3.ChromaSamplingFormat                         = CHROMA_SAMPLING_FORMAT_FORMAT420;
    //DW3.Bitdepthminus8                               = BITDEPTHMINUS8_BITDEPTH8;
    //DW3.ProfileLevel                                 = PROFILE_LEVEL_PROFILE0;

    DW4.Value                                        = 0x00000000;

    DW5.Value                                        = 0x00000000;

    DW6.Value                                        = 0x00000000;

    DW7.Value                                        = 0x00000000;

    DW8.Value                                        = 0x00000000;

    DW9.Value                                        = 0x00000000;

    DW10.Value                                       = 0x00000000;

    DW11.Value                                       = 0x00000002;
    //DW11.MotionCompScalingEnableBit                  = MOTION_COMP_SCALING_ENABLE_BIT_ENABLE;

    DW12.Value                                       = 0x00000000;

    DW13.Value                                       = 0x00000000;

    DW14.Value                                       = 0x00000000;

    DW15.Value                                       = 0x00000000;

    DW16.Value                                       = 0x00000000;

    DW17.Value                                       = 0x00000000;

    DW18.Value                                       = 0x00000000;

}

mhw_vdbox_hcp_g9_glk::HEVC_VP9_RDOQ_LAMBDA_FIELDS_CMD::HEVC_VP9_RDOQ_LAMBDA_FIELDS_CMD()
{
    DW0.Value                                        = 0x00000000;

}

mhw_vdbox_hcp_g9_glk::HEVC_VP9_RDOQ_STATE_CMD::HEVC_VP9_RDOQ_STATE_CMD()
{
    DW0.Value                                        = 0x73880080;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.Subopb                                       = SUBOPB_UNNAMED8;
    //DW0.Subopa                                       = SUBOPA_UNNAMED0;
    //DW0.Opcode                                       = OPCODE_UNNAMED7;
    //DW0.Pipeline                                     = PIPELINE_UNNAMED2;
    //DW0.CommandType                                  = COMMAND_TYPE_UNNAMED3;

    DW1.Value                                        = 0x00000000;

}

