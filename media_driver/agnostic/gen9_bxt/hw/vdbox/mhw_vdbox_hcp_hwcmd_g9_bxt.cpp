/*
* Copyright (c) 2017, Intel Corporation
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
//! \file   mhw_vdbox_hcp_hwcmd_g9_bxt.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

#include "mhw_vdbox_hcp_hwcmd_g9_bxt.h"
#include "mos_utilities.h"

mhw_vdbox_hcp_g9_bxt::HCP_PIPE_MODE_SELECT_CMD::HCP_PIPE_MODE_SELECT_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPPIPEMODESELECT;
    DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        
    DW1.CodecSelect                                  = CODEC_SELECT_DECODE;
    DW1.DeblockerStreamoutEnable                     = DEBLOCKER_STREAMOUT_ENABLE_DISABLE;
    DW1.PakPipelineStreamoutEnable                   = PAK_PIPELINE_STREAMOUT_ENABLE_DISABLEPIPELINESTATESANDPARAMETERSSTREAMOUT;
    DW1.PicStatusErrorReportEnable                   = PIC_STATUSERROR_REPORT_ENABLE_DISABLE;
    DW1.CodecStandardSelect                          = CODEC_STANDARD_SELECT_HEVC;

    DW2.Value                                        = 0;        
    DW2.MediaSoftResetCounterPer1000Clocks           = MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS_DISABLE;

    DW3.Value                                        = 0;        
    DW3.PicStatusErrorReportId                       = PIC_STATUSERROR_REPORT_ID_32_BITUNSIGNED;

}

mhw_vdbox_hcp_g9_bxt::HCP_SURFACE_STATE_CMD::HCP_SURFACE_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPSURFACESTATE;
    DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        
    DW1.SurfaceId                                    = SURFACE_ID_HEVCFORCURRENTDECODEDPICTURE;

    DW2.Value                                        = 0;        
    DW2.SurfaceFormat                                = 0;

}

mhw_vdbox_hcp_g9_bxt::GRAPHICSADDRESS63_6_CMD::GRAPHICSADDRESS63_6_CMD()
{
    DW0_1.Value[0] = DW0_1.Value[1]                  = 0;        

}

mhw_vdbox_hcp_g9_bxt::HCP_PIPE_BUF_ADDR_STATE_CMD::HCP_PIPE_BUF_ADDR_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPPIPEBUFADDRSTATE;
    DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW3.Value                                        = 0;        
    DW3.ArbitrationPriorityControl                   = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW3.Rowstorescratchbuffercacheselect             = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW6.Value                                        = 0;        
    DW6.ArbitrationPriorityControl                   = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW6.Rowstorescratchbuffercacheselect             = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW9.Value                                        = 0;        
    DW9.ArbitrationPriorityControl                   = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW9.Rowstorescratchbuffercacheselect             = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW12.Value                                       = 0;        
    DW12.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW12.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW15.Value                                       = 0;        
    DW15.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW15.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW18.Value                                       = 0;        
    DW18.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW18.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW21.Value                                       = 0;        
    DW21.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW21.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW24.Value                                       = 0;        
    DW24.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW24.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW27.Value                                       = 0;        
    DW27.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW27.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW30.Value                                       = 0;        
    DW30.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW30.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW33.Value                                       = 0;        
    DW33.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW33.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW34_35.Value[0] = DW34_35.Value[1]              = 0;        

    DW36.Value                                       = 0;        

    DW53.Value                                       = 0;        
    DW53.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW53.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW56.Value                                       = 0;        
    DW56.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW56.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW59.Value                                       = 0;        
    DW59.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW59.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW62.Value                                       = 0;        
    DW62.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW62.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW65.Value                                       = 0;        
    DW65.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW65.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW82.Value                                       = 0;        
    DW82.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW82.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW85.Value                                       = 0;        
    DW85.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW85.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW86_87.Value[0] = DW86_87.Value[1]              = 0;        

    DW88.Value                                       = 0;        
    DW88.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW88.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW91.Value                                       = 0;        
    DW91.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW91.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

    DW94.Value                                       = 0;        
    DW94.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW94.Rowstorescratchbuffercacheselect            = ROWSTORESCRATCHBUFFERCACHESELECT_BUFFERTOLLC;

}

mhw_vdbox_hcp_g9_bxt::HCP_IND_OBJ_BASE_ADDR_STATE_CMD::HCP_IND_OBJ_BASE_ADDR_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPINDOBJBASEADDRSTATE;
    DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0;        

    DW3.Value                                        = 0;        
    DW3.ArbitrationPriorityControl                   = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW4_5.Value[0] = DW4_5.Value[1]                  = 0;        

    DW6_7.Value[0] = DW6_7.Value[1]                  = 0;        

    DW8.Value                                        = 0;        
    DW8.ArbitrationPriorityControl                   = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW9_10.Value[0] = DW9_10.Value[1]                = 0;        

    DW11.Value                                       = 0;        
    DW11.ArbitrationPriorityControl                  = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW12_13.Value[0] = DW12_13.Value[1]              = 0;        

}

mhw_vdbox_hcp_g9_bxt::HCP_QM_STATE_CMD::HCP_QM_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPQMSTATE;
    DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        
    DW1.PredictionType                               = PREDICTION_TYPE_INTRA;
    DW1.Sizeid                                       = SIZEID_4X_4;
    DW1.ColorComponent                               = COLOR_COMPONENT_LUMA;

    MOS_ZeroMemory(&Quantizermatrix, sizeof(Quantizermatrix));        
}

mhw_vdbox_hcp_g9_bxt::HCP_PIC_STATE_CMD::HCP_PIC_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPPICSTATE;
    DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        
    DW2.Mincusize                                    = MINCUSIZE_8X8;
    DW2.CtbsizeLcusize                               = CTBSIZE_LCUSIZE_ILLEGALRESERVED;
    DW2.Mintusize                                    = MINTUSIZE_4X_4;
    DW2.Maxtusize                                    = MAXTUSIZE_4X_4;
    DW2.Minpcmsize                                   = MINPCMSIZE_8X8;
    DW2.Maxpcmsize                                   = MAXPCMSIZE_8X8;

    DW3.Value                                        = 0;        
    DW3.Colpicisi                                    = COLPICISI_COLLOCATEDPICTUREHASATLEASTONEPORBSLICE;
    DW3.Curpicisi                                    = CURPICISI_CURRENTPICTUREHASATLEASTONEPORBSLICE;
    DW3.Inserttestflag                               = INSERTTESTFLAG_UNNAMED0;

    DW4.Value                                        = 0;        
    DW4.CuQpDeltaEnabledFlag                         = CU_QP_DELTA_ENABLED_FLAG_DISABLE;
    DW4.SignDataHidingFlag                           = SIGN_DATA_HIDING_FLAG_DISABLE;
    DW4.Fieldpic                                     = FIELDPIC_VIDEOFRAME;
    DW4.Bottomfield                                  = BOTTOMFIELD_BOTTOMFIELD;
    DW4.TransformSkipEnabledFlag                     = TRANSFORM_SKIP_ENABLED_FLAG_DISABLE;
    DW4.AmpEnabledFlag                               = AMP_ENABLED_FLAG_DISABLE;
    DW4.TransquantBypassEnableFlag                   = TRANSQUANT_BYPASS_ENABLE_FLAG_DISABLE;

    DW5.Value                                        = 0;        
    DW5.BitDepthChromaMinus8                         = BIT_DEPTH_CHROMA_MINUS8_CHROMA8BIT;
    DW5.BitDepthLumaMinus8                           = BIT_DEPTH_LUMA_MINUS8_LUMA8BIT;

    DW6.Value                                        = 0;        
    DW6.Nonfirstpassflag                             = NONFIRSTPASSFLAG_DISABLE;
    DW6.LcumaxbitstatusenLcumaxsizereportmask        = LCUMAXBITSTATUSEN_LCUMAXSIZEREPORTMASK_DISABLE;
    DW6.FrameszoverstatusenFramebitratemaxreportmask = FRAMESZOVERSTATUSEN_FRAMEBITRATEMAXREPORTMASK_DISABLE;
    DW6.FrameszunderstatusenFramebitrateminreportmask = FRAMESZUNDERSTATUSEN_FRAMEBITRATEMINREPORTMASK_DISABLE;
    DW6.LoadSlicePointerFlag                         = LOAD_SLICE_POINTER_FLAG_DISABLE;

    DW7.Value                                        = 0;        
    DW7.Framebitratemaxunit                          = FRAMEBITRATEMAXUNIT_BYTE;

    DW8.Value                                        = 0;        
    DW8.Framebitrateminunit                          = FRAMEBITRATEMINUNIT_BYTE;

    DW9.Value                                        = 0;        
    DW9.Framebitratemindelta                         = FRAMEBITRATEMINDELTA_UNNAMED0;
    DW9.Framebitratemaxdelta                         = FRAMEBITRATEMAXDELTA_UNNAMED0;

    DW10_11.Value[0] = DW10_11.Value[1]              = 0;        

    DW12_13.Value[0] = DW12_13.Value[1]              = 0;        

    DW14_15.Value[0] = DW14_15.Value[1]              = 0;        

    DW16_17.Value[0] = DW16_17.Value[1]              = 0;        

    DW18.Value                                       = 0;        
    DW18.Minframesize                                = MINFRAMESIZE_UNNAMED0;
    DW18.Minframesizeunits                           = MINFRAMESIZEUNITS_4KB;

}

mhw_vdbox_hcp_g9_bxt::COLUMN_POSITION_IN_CTB_CMD::COLUMN_POSITION_IN_CTB_CMD()
{
    DW0.Value                                        = 0;        

}

mhw_vdbox_hcp_g9_bxt::ROW_POSITION_IN_CTB_CMD::ROW_POSITION_IN_CTB_CMD()
{
    DW0.Value                                        = 0;        

}

mhw_vdbox_hcp_g9_bxt::HCP_TILE_STATE_CMD::HCP_TILE_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPTILESTATE;
    DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        

    DW12.Value                                       = 0;        

}

mhw_vdbox_hcp_g9_bxt::HEVC_REF_LIST_WRITE_CMD::HEVC_REF_LIST_WRITE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.ChromaWeightLxFlag                           = CHROMA_WEIGHT_LX_FLAG_DEFAULTWEIGHTEDPREDICTIONFORCHROMA;
    DW0.LumaWeightLxFlag                             = LUMA_WEIGHT_LX_FLAG_DEFAULTWEIGHTEDPREDICTIONFORLUMA;
    DW0.Longtermreference                            = LONGTERMREFERENCE_SHORTTERMREFERENCE;
    DW0.FieldPicFlag                                 = FIELD_PIC_FLAG_VIDEOFRAME;
    DW0.BottomFieldFlag                              = BOTTOM_FIELD_FLAG_BOTTOMFIELD;

}

mhw_vdbox_hcp_g9_bxt::HCP_REF_IDX_STATE_CMD::HCP_REF_IDX_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPREFIDXSTATE;
    DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        
    DW1.Refpiclistnum                                = REFPICLISTNUM_REFERENCEPICTURELIST0;

}

mhw_vdbox_hcp_g9_bxt::HEVC_LUMA_WEIGHT_OFFSET_WRITE_CMD::HEVC_LUMA_WEIGHT_OFFSET_WRITE_CMD()
{
    DW0.Value                                        = 0;        

}

mhw_vdbox_hcp_g9_bxt::HEVC_CHROMA_WEIGHT_OFFSET_WRITE_CMD::HEVC_CHROMA_WEIGHT_OFFSET_WRITE_CMD()
{
    DW0.Value                                        = 0;        

}

mhw_vdbox_hcp_g9_bxt::HCP_WEIGHTOFFSET_STATE_CMD::HCP_WEIGHTOFFSET_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPWEIGHTOFFSETSTATE;
    DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        
    DW1.Refpiclistnum                                = REFPICLISTNUM_REFERENCEPICTURELIST0;

}

mhw_vdbox_hcp_g9_bxt::HCP_SLICE_STATE_CMD::HCP_SLICE_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPSLICESTATE;
    DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        
    DW3.SliceType                                    = SLICE_TYPE_B_SLICE;
    DW3.Lastsliceofpic                               = LASTSLICEOFPIC_NOTTHELASTSLICEOFTHEPICTURE;
    DW3.SliceCbQpOffset                              = SLICE_CB_QP_OFFSET_0;
    DW3.SliceCrQpOffset                              = SLICE_CR_QP_OFFSET_0;

    DW4.Value                                        = 0;        
    DW4.Maxmergeidx                                  = MAXMERGEIDX_0;

    DW5.Value                                        = 0;        

    DW6.Value                                        = 0;        
    DW6.Roundintra                                   = ROUNDINTRA_532;
    DW6.Roundinter                                   = ROUNDINTER_532;

    DW7.Value                                        = 0;        
    DW7.Cabaczerowordinsertionenable                 = CABACZEROWORDINSERTIONENABLE_UNNAMED0;
    DW7.Emulationbytesliceinsertenable               = EMULATIONBYTESLICEINSERTENABLE_OUTPUTTINGRBSP;
    DW7.TailInsertionEnable                          = TAIL_INSERTION_ENABLE_UNNAMED0;
    DW7.SlicedataEnable                              = SLICEDATA_ENABLE_UNNAMED0;
    DW7.HeaderInsertionEnable                        = HEADER_INSERTION_ENABLE_UNNAMED0;

    DW8.Value                                        = 0;        

}

mhw_vdbox_hcp_g9_bxt::HCP_BSD_OBJECT_CMD::HCP_BSD_OBJECT_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPBSDOBJECTSTATE;
    DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

}

mhw_vdbox_hcp_g9_bxt::HCP_VP9_SEGMENT_STATE_CMD::HCP_VP9_SEGMENT_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPVP9SEGMENTSTATE;
    DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        

    DW4.Value                                        = 0;        

    DW5.Value                                        = 0;        

    DW6.Value                                        = 0;        
}

mhw_vdbox_hcp_g9_bxt::HCP_FQM_STATE_CMD::HCP_FQM_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPFQMSTATE;
    DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        
    DW1.IntraInter                                   = INTRAINTER_INTRA;
    DW1.Sizeid                                       = SIZEID_SIZEID0_4X_4;
    DW1.ColorComponent                               = COLOR_COMPONENT_LUMA;

    MOS_ZeroMemory(&Quantizermatrix, sizeof(Quantizermatrix));        
}

mhw_vdbox_hcp_g9_bxt::HCP_PAK_INSERT_OBJECT_CMD::HCP_PAK_INSERT_OBJECT_CMD()
{
    DW0.Value                                        = 0;        
    DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPPAKINSERTOBJECT;
    DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        
    DW1.EmulationflagEmulationbytebitsinsertenable   = 0;
    DW1.Headerlengthexcludefrmsize                   = HEADERLENGTHEXCLUDEFRMSIZE_ALLBITSACCUMULATED;
    DW1.IndirectPayloadEnable                        = INDIRECT_PAYLOAD_ENABLE_INLINEPAYLOADISUSED;
}

mhw_vdbox_hcp_g9_bxt::HCP_VP9_PIC_STATE_CMD::HCP_VP9_PIC_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_HCPVP9PICSTATE;
    DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME;
    DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        
    DW2.FrameType                                    = FRAME_TYPE_KEYFRAME;
    DW2.AdaptProbabilitiesFlag                       = ADAPT_PROBABILITIES_FLAG_0DONOTADAPTERRORRESILIENTORFRAMEPARALLELMODEARESET;
    DW2.AllowHiPrecisionMv                           = ALLOW_HI_PRECISION_MV_NORMALMODE;
    DW2.McompFilterType                              = MCOMP_FILTER_TYPE_EIGHT_TAP;
    DW2.HybridPredictionMode                         = HYBRID_PREDICTION_MODE_COMPPREDICTIONMODEHYBRID_ENCODERDOESNOTPACKCOMPPREDMODEINTERPREDCOMPINPAKOBJINTOBITSTREAM;
    DW2.SelectableTxMode                             = SELECTABLE_TX_MODE_ENCODERDOESNOTPACKTUSIZEINTOBITSTREAMTHISHELPSREDUCEBITSTREAMSIZEFURTHER;
    DW2.LastFrameType                                = LAST_FRAME_TYPE_KEYFRAME;
    DW2.RefreshFrameContext                          = REFRESH_FRAME_CONTEXT_DISABLE;
    DW2.ErrorResilientMode                           = ERROR_RESILIENT_MODE_DISABLE;
    DW2.FrameParallelDecodingMode                    = FRAME_PARALLEL_DECODING_MODE_DISABLE;
    DW2.SegmentationEnabled                          = SEGMENTATION_ENABLED_ALLBLOCKSAREIMPLIEDTOBELONGTOSEGMENT0;
    DW2.SegmentationUpdateMap                        = SEGMENTATION_UPDATE_MAP_UNNAMED0;
    DW2.SegmentationTemporalUpdate                   = SEGMENTATION_TEMPORAL_UPDATE_DECODESEGIDFROMBITSTREAM;
    DW2.LosslessMode                                 = LOSSLESS_MODE_NORMALMODE;
    DW2.SegmentIdStreamoutEnable                     = SEGMENT_ID_STREAMOUT_ENABLE_DISABLE;
    DW2.SegmentIdStreaminEnable                      = SEGMENT_ID_STREAMIN_ENABLE_DISABLE;

    DW3.Value                                        = 0;        
    DW3.Log2TileColumn                               = LOG2_TILE_COLUMN_1TILECOLUMN;
    DW3.Log2TileRow                                  = LOG2_TILE_ROW_1TILEROW;

    DW4.Value                                        = 0;        

    DW5.Value                                        = 0;        

    DW6.Value                                        = 0;        

    DW7.Value                                        = 0;        

    DW8.Value                                        = 0;        

    DW9.Value                                        = 0;        

    DW10.Value                                       = 0;        

    DW11.Value                                       = 0;        
    DW11.MotionCompScaling                           = MOTION_COMP_SCALING_POST_DEC13VERSION;

}

mhw_vdbox_hcp_g9_bxt::HEVC_VP9_RDOQ_LAMBDA_FIELDS_CMD::HEVC_VP9_RDOQ_LAMBDA_FIELDS_CMD()
{
    DW0.Value                                        = 0;        

}

mhw_vdbox_hcp_g9_bxt::HEVC_VP9_RDOQ_STATE_CMD::HEVC_VP9_RDOQ_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopb                                       = SUBOPB_UNNAMED8;
    DW0.Subopa                                       = SUBOPA_UNNAMED0;
    DW0.Opcode                                       = OPCODE_UNNAMED7;
    DW0.Pipeline                                     = PIPELINE_UNNAMED2;
    DW0.CommandType                                  = COMMAND_TYPE_UNNAMED3;

    DW1.Value                                        = 0;        

}

