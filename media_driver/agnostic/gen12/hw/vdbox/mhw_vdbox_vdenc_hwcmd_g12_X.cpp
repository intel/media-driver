/*===================== begin_copyright_notice ==================================

Copyright (c) 2017-2019, Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file   mhw_vdbox_vdenc_hwcmd_g12_X.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

#include "mhw_vdbox_vdenc_hwcmd_g12_X.h"
#include "mos_utilities.h"

mhw_vdbox_vdenc_g12_X::VDENC_64B_Aligned_Lower_Address_CMD::VDENC_64B_Aligned_Lower_Address_CMD()
{
    DW0.Value                                        = 0;        

}

mhw_vdbox_vdenc_g12_X::VDENC_64B_Aligned_Upper_Address_CMD::VDENC_64B_Aligned_Upper_Address_CMD()
{
    DW0.Value                                        = 0;        

}

mhw_vdbox_vdenc_g12_X::VDENC_Surface_Control_Bits_CMD::VDENC_Surface_Control_Bits_CMD()
{
    DW0.Value                                        = 0;        
    DW0.ArbitrationPriorityControl                   = ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;
    DW0.MemoryCompressionEnable                      = MEMORY_COMPRESSION_ENABLE_DISABLE;
    DW0.CompressionType                              = 0;
    DW0.CacheSelect                                  = CACHE_SELECT_UNNAMED0;
    DW0.TiledResourceMode                            = TILED_RESOURCE_MODE_TRMODENONE;

}

mhw_vdbox_vdenc_g12_X::VDENC_Sub_Mb_Pred_Mode_CMD::VDENC_Sub_Mb_Pred_Mode_CMD()
{
    DW0.Value                                        = 0;        

}

mhw_vdbox_vdenc_g12_X::VDENC_Block_8x8_4_CMD::VDENC_Block_8x8_4_CMD()
{
    DW0.Value                                        = 0;        

}

mhw_vdbox_vdenc_g12_X::VDENC_Delta_MV_XY_CMD::VDENC_Delta_MV_XY_CMD()
{
    DW0.Value                                        = 0;        
    DW0.X0                                           = X0_UNNAMED0;
    DW0.Y0                                           = Y0_UNNAMED0;

    DW1.Value                                        = 0;        
    DW1.X1                                           = X1_UNNAMED0;
    DW1.Y1                                           = Y1_UNNAMED0;

    DW2.Value                                        = 0;        
    DW2.X2                                           = X2_UNNAMED0;
    DW2.Y2                                           = Y2_UNNAMED0;

    DW3.Value                                        = 0;        
    DW3.X3                                           = X3_UNNAMED0;
    DW3.Y3                                           = Y3_UNNAMED0;

}

mhw_vdbox_vdenc_g12_X::VDENC_Colocated_MV_Picture_CMD::VDENC_Colocated_MV_Picture_CMD()
{
}

mhw_vdbox_vdenc_g12_X::VDENC_Down_Scaled_Reference_Picture_CMD::VDENC_Down_Scaled_Reference_Picture_CMD()
{
}

mhw_vdbox_vdenc_g12_X::VDENC_FRAME_BASED_STATISTICS_STREAMOUT_CMD::VDENC_FRAME_BASED_STATISTICS_STREAMOUT_CMD()
{
    DW0.Value                                        = 0;        

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        

    DW4.Value                                        = 0;        

    MOS_ZeroMemory(&Reserved160, sizeof(Reserved160));        
    DW17.Value                                       = 0;        

    DW18.Value                                       = 0;        

    DW19.Value                                       = 0;        

}

mhw_vdbox_vdenc_g12_X::VDENC_Mode_StreamOut_Data_CMD::VDENC_Mode_StreamOut_Data_CMD()
{
    DW0.Value                                        = 0;        

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        
    DW2.IntermbmodeChromaPredictionMode              = INTERMBMODECHROMA_PREDICTION_MODE_UNNAMED0;
    DW2.Intrambmode                                  = INTRAMBMODE_UNNAMED0;
    DW2.Intrambflag                                  = INTRAMBFLAG_INTER;
    DW2.Lastmbflag                                   = LASTMBFLAG_NOTLAST;

    DW3.Value                                        = 0;        

    DW4.Value                                        = 0;        

    DW13.Value                                       = 0;        

    DW14.Value                                       = 0;        

    DW15.Value                                       = 0;        

}

mhw_vdbox_vdenc_g12_X::VDENC_Original_Uncompressed_Picture_CMD::VDENC_Original_Uncompressed_Picture_CMD()
{
}

mhw_vdbox_vdenc_g12_X::VDENC_Reference_Picture_CMD::VDENC_Reference_Picture_CMD()
{
}

mhw_vdbox_vdenc_g12_X::VDENC_Row_Store_Scratch_Buffer_Picture_CMD::VDENC_Row_Store_Scratch_Buffer_Picture_CMD()
{
}

mhw_vdbox_vdenc_g12_X::VDENC_Statistics_Streamout_CMD::VDENC_Statistics_Streamout_CMD()
{
}

mhw_vdbox_vdenc_g12_X::VDENC_Streamin_Data_Picture_CMD::VDENC_Streamin_Data_Picture_CMD()
{
}

mhw_vdbox_vdenc_g12_X::VDENC_STREAMIN_STATE_CMD::VDENC_STREAMIN_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.Forceintra                                   = FORCEINTRA_DISABLE;
    DW0.Forceskip                                    = FORCESKIP_DISABLE;

    DW1.Value                                        = 0;        
    DW1.Qpprimey                                     = QPPRIMEY_UNNAMED0;

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        

    DW4.Value                                        = 0;        

    MOS_ZeroMemory(&Reserved160, sizeof(Reserved160));        
}

mhw_vdbox_vdenc_g12_X::VDENC_HEVC_VP9_FRAME_BASED_STATISTICS_STREAMOUT_CMD::VDENC_HEVC_VP9_FRAME_BASED_STATISTICS_STREAMOUT_CMD()
{
    DW0.Value                                        = 0;        

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

    DW11.Value                                       = 0;        

    DW12.Value                                       = 0;        

    DW13.Value                                       = 0;        

    DW14.Value                                       = 0;        

    DW15.Value                                       = 0;        

    DW16.Value                                       = 0;        

    DW17.Value                                       = 0;        

    DW18.Value                                       = 0;        

    DW19.Value                                       = 0;        

    DW20.Value                                       = 0;        

    DW21.Value                                       = 0;        

    DW22.Value                                       = 0;        

    DW23.Value                                       = 0;        

    MOS_ZeroMemory(&Reserved768, sizeof(Reserved768));        
    DW29.Value                                       = 0;        

    DW30.Value                                       = 0;        

    DW31.Value                                       = 0;        

}

mhw_vdbox_vdenc_g12_X::VDENC_HEVC_VP9_STREAMIN_STATE_CMD::VDENC_HEVC_VP9_STREAMIN_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.Numimepredictors                             = NUMIMEPREDICTORS_UNNAMED0;

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

    DW11.Value                                       = 0;        

    DW12.Value                                       = 0;        

    DW13.Value                                       = 0;        

    DW14.Value                                       = 0;        

    DW15.Value                                       = 0;        

}

mhw_vdbox_vdenc_g12_X::VDENC_Surface_State_Fields_CMD::VDENC_Surface_State_Fields_CMD()
{
    DW0.Value                                        = 0;        

    DW1.Value                                        = 0;        
    DW1.TileWalk                                     = TILE_WALK_YMAJOR;
    DW1.TiledSurface                                 = TILED_SURFACE_TRUE;
    DW1.HalfPitchForChroma                           = HALF_PITCH_FOR_CHROMA_DISABLE;

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        

}

mhw_vdbox_vdenc_g12_X::VD_PIPELINE_FLUSH_CMD::VD_PIPELINE_FLUSH_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordCountN                                  = GetOpLength(dwSize);
    DW0.Subopcodeb                                   = SUBOPCODEB_UNNAMED0;
    DW0.Subopcodea                                   = SUBOPCODEA_UNNAMED0;
    DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_EXTENDEDCOMMAND;
    DW0.Pipeline                                     = PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        

}

mhw_vdbox_vdenc_g12_X::VDENC_WEIGHTSOFFSETS_STATE_CMD::VDENC_WEIGHTSOFFSETS_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwLength                                     = GetOpLength(dwSize);
    DW0.Subopb                                       = SUBOPB_VDENCAVCWEIGHTSOFFSETSTATE;
    DW0.Subopa                                       = SUBOPA_UNNAMED0;
    DW0.Opcode                                       = OPCODE_VDENCPIPE;
    DW0.Pipeline                                     = PIPELINE_MFXCOMMON;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

}

mhw_vdbox_vdenc_g12_X::VDENC_CONST_QPT_STATE_CMD::VDENC_CONST_QPT_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopb                                       = SUBOPB_VDENCCONSTQPTSTATE;
    DW0.Subopa                                       = SUBOPA_UNNAMED0;
    DW0.Opcode                                       = OPCODE_VDENCPIPE;
    DW0.Pipeline                                     = PIPELINE_MFXCOMMON;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;     

    MOS_ZeroMemory(DW1_10.Value, sizeof(DW1_10.Value));
    DW11.Value = 0;
    MOS_ZeroMemory(DW12_24.Value, sizeof(DW12_24.Value));
    DW25.Value = 0;
    MOS_ZeroMemory(DW26_38.Value, sizeof(DW26_38.Value));
    DW39.Value = 0;
    MOS_ZeroMemory(DW40_45.Value, sizeof(DW40_45.Value));
    DW46.Value = 0;
    MOS_ZeroMemory(DW47_52.Value, sizeof(DW47_52.Value));
    DW53.Value = 0;
    MOS_ZeroMemory(DW54_59.Value, sizeof(DW54_59.Value));
    DW60.Value = 0;
}

mhw_vdbox_vdenc_g12_X::VDENC_DS_REF_SURFACE_STATE_CMD::VDENC_DS_REF_SURFACE_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopb                                       = SUBOPB_VDENCDSREFSURFACESTATE;
    DW0.Subopa                                       = SUBOPA_UNNAMED0;
    DW0.Opcode                                       = OPCODE_VDENCPIPE;
    DW0.Pipeline                                     = PIPELINE_MFXCOMMON;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        

}

mhw_vdbox_vdenc_g12_X::VDENC_IMG_STATE_CMD::VDENC_IMG_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopb                                       = SUBOPB_VDENCIMGSTATE;
    DW0.Subopa                                       = SUBOPA_UNNAMED0;
    DW0.Opcode                                       = OPCODE_VDENCPIPE;
    DW0.Pipeline                                     = PIPELINE_MFXCOMMON;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        
    DW1.BidirectionalMixDisable                      = BIDIRECTIONAL_MIX_DISABLE_SUBBLOCKENABLED;
    DW1.VdencPerfmode                                = VDENC_PERFMODE_NORMAL;
    DW1.TimeBudgetOverflowCheck                      = TIME_BUDGET_OVERFLOW_CHECK_DISABLED;
    DW1.VdencExtendedPakObjCmdEnable                 = VDENC_EXTENDED_PAK_OBJ_CMD_ENABLE_DISABLE;
    DW1.Transform8X8Flag                             = TRANSFORM_8X8_FLAG_DISABLED;
    DW1.VdencL1CachePriority                         = VDENC_L1_CACHE_PRIORITY_UNNAMED0;

    DW2.Value                                        = 0;        
    DW2.BidirectionalWeight                          = 0;

    DW3.Value                                        = 0;        

    DW4.Value                                        = 0;        
    DW4.SubPelMode                                   = SUB_PEL_MODE_UNNAMED0;
    DW4.ForwardTransformSkipCheckEnable              = FORWARD_TRANSFORM_SKIP_CHECK_ENABLE_FTDISABLED;
    DW4.BmeDisableForFbrMessage                      = BME_DISABLE_FOR_FBR_MESSAGE_BMEENABLED;
    DW4.BlockBasedSkipEnabled                        = BLOCK_BASED_SKIP_ENABLED_UNNAMED0;
    DW4.InterSadMeasureAdjustment                    = INTER_SAD_MEASURE_ADJUSTMENT_NONE;
    DW4.IntraSadMeasureAdjustment                    = INTRA_SAD_MEASURE_ADJUSTMENT_NONE;
    DW4.SubMacroblockSubPartitionMask                = 0;
    DW4.BlockBasedSkipType                           = BLOCK_BASED_SKIP_TYPE_UNNAMED0;

    DW5.Value                                        = 0;        
    DW5.CrePrefetchEnable                            = CRE_PREFETCH_ENABLE_UNNAMED0;
    DW5.HmeRef1Disable                               = HME_REF1_DISABLE_UNNAMED0;
    DW5.ConstrainedIntraPredictionFlag               = CONSTRAINED_INTRA_PREDICTION_FLAG_UNNAMED0;
    DW5.PictureType                                  = PICTURE_TYPE_I;

    DW6.Value                                        = 0;        

    DW7.Value                                        = 0;        

    DW8.Value                                        = 0;        
    DW8.LumaIntraPartitionMask                       = 0;
    DW8.MvCostScalingFactor                          = MV_COST_SCALING_FACTOR_QPEL;
    DW8.RefidCostModeSelect                          = REFID_COST_MODE_SELECT_MODE0;

    DW9.Value                                        = 0;        

    DW10.Value                                       = 0;        

    DW11.Value                                       = 0;        

    DW12.Value                                       = 0; 

    DW13.Value                                       = 0; 

    DW14.Value                                       = 0;        

    DW15.Value                                       = 0;        

    DW16.Value                                       = 0;        

    DW17.Value                                       = 0;        

    DW18.Value                                       = 0;        
    DW18.AvcIntra16X16ModeMask                       = 0;
    DW18.AvcIntraChromaModeMask                      = 0;
    DW18.IntraComputeTypeIntracomputetype            = INTRA_COMPUTE_TYPE_INTRACOMPUTETYPE_UNNAMED0;

    DW19.Value                                       = 0;        

    DW20.Value                                       = 0;        

    DW21.Value                                       = 0;        
    DW21.IntraRefreshEnableRollingIEnable            = INTRAREFRESHENABLE_ROLLING_I_ENABLE_DISABLE;
    DW21.IntraRefreshMode                            = INTRAREFRESHMODE_ROWBASED;

    DW22.Value                                       = 0;        

    DW23.Value                                       = 0;        

    DW24.Value                                       = 0;        

    DW25.Value                                       = 0;        

    DW26.Value                                       = 0;        
    DW26.HmeRefWindowsCombiningThreshold             = HME_REF_WINDOWS_COMBINING_THRESHOLD_UNNAMED0;

    DW27.Value                                       = 0;        

    DW28.Value                                       = 0;  

    DW29.Value                                       = 0;  

    DW30.Value                                       = 0;        

    DW31.Value                                       = 0;        

    DW32.Value                                       = 0;        

    DW33.Value                                       = 0;        

    DW34.Value                                       = 0;        
    DW34.PpmvDisable                                 = PPMV_DISABLE_UNNAMED0;
    DW34.LongtermReferenceFrameBwdRef0Indicator      = LONGTERM_REFERENCE_FRAME_BWD_REF0_INDICATOR_SHORT_TERMREFERENCE;
    DW34.LongtermReferenceFrameFwdRef2Indicator      = LONGTERM_REFERENCE_FRAME_FWD_REF2_INDICATOR_SHORT_TERMREFERENCE;
    DW34.LongtermReferenceFrameFwdRef1Indicator      = LONGTERM_REFERENCE_FRAME_FWD_REF1_INDICATOR_SHORT_TERMREFERENCE;
    DW34.LongtermReferenceFrameFwdRef0Indicator      = LONGTERM_REFERENCE_FRAME_FWD_REF0_INDICATOR_SHORT_TERMREFERENCE;

}

mhw_vdbox_vdenc_g12_X::VDENC_PIPE_BUF_ADDR_STATE_CMD::VDENC_PIPE_BUF_ADDR_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopb                                       = SUBOPB_VDENCPIPEBUFADDRSTATE;
    DW0.Subopa                                       = SUBOPA_UNNAMED0;
    DW0.Opcode                                       = OPCODE_VDENCPIPE;
    DW0.Pipeline                                     = PIPELINE_MFXCOMMON;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;
    DW61.Value                                       = 0;
    DsFwdRef1 = {};
    DsFwdRef14X = {};
}

mhw_vdbox_vdenc_g12_X::VDENC_PIPE_MODE_SELECT_CMD::VDENC_PIPE_MODE_SELECT_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopb                                       = SUBOPB_VDENCPIPEMODESELECT;
    DW0.Subopa                                       = SUBOPA_UNNAMED0;
    DW0.Opcode                                       = OPCODE_VDENCPIPE;
    DW0.Pipeline                                     = PIPELINE_MFXCOMMON;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        
    DW1.StandardSelect                               = 0;
    DW1.FrameStatisticsStreamOutEnable               = FRAME_STATISTICS_STREAM_OUT_ENABLE_DISABLE;
    DW1.TlbPrefetchEnable                            = TLB_PREFETCH_ENABLE_DISABLE;
    DW1.PakThresholdCheckEnable                      = PAK_THRESHOLD_CHECK_ENABLE_DISABLESTATICSLICEMODE;
    DW1.VdencStreamInEnable                          = VDENC_STREAM_IN_ENABLE_DISABLE;
    DW1.BitDepth                                     = BIT_DEPTH_8BIT;
    DW1.PakChromaSubSamplingType                     = 0;
    DW1.PrimaryChannelSelectionForRgbEncoding        = PRIMARY_CHANNEL_SELECTION_FOR_RGB_ENCODING_UNNAMED1;
    DW1.FirstSecondaryChannelSelectionForRgbEncoding = FIRST_SECONDARY_CHANNEL_SELECTION_FOR_RGB_ENCODING_UNNAMED2;
    DW1.StreamingBufferConfig                        = STREAMING_BUFFER_UNSUPPORTED;

    DW2.Value                                        = 0;        
    DW2.HmeRegionPreFetchenable                      = HME_REGION_PRE_FETCHENABLE_UNNAMED1;
    DW2.Topprefetchenablemode                        = TOPPREFETCHENABLEMODE_UNNAMED1;
    DW2.LeftpreFetchatwraparound                     = LEFTPRE_FETCHATWRAPAROUND_UNNAMED1;
    DW2.Verticalshift32Minus1                        = VERTICALSHIFT32MINUS1_UNNAMED0;
    DW2.Hzshift32Minus1                              = HZSHIFT32MINUS1_UNNAMED3;
    DW2.NumVerticalReqMinus1                         = NUMVERTICALREQMINUS1_UNNAMED11;
    DW2.Numhzreqminus1                               = NUMHZREQMINUS1_UNNAMED2;
    DW2.PreFetchOffsetForReferenceIn16PixelIncrement = PRE_FETCH_OFFSET_FOR_REFERENCE_IN_16_PIXEL_INCREMENT_UNNAMED0;

    DW3.Value                                        = 0;        
    DW3.SourceLumaPackedDataTlbPreFetchenable        = SOURCE_LUMAPACKED_DATA_TLB_PRE_FETCHENABLE_UNNAMED1;
    DW3.SourceChromaTlbPreFetchenable                = SOURCE_CHROMA_TLB_PRE_FETCHENABLE_UNNAMED1;
    DW3.Verticalshift32Minus1Src                     = VERTICALSHIFT32MINUS1SRC_UNNAMED0;
    DW3.Hzshift32Minus1Src                           = HZSHIFT32MINUS1SRC_UNNAMED3;
    DW3.Numverticalreqminus1Src                      = NUMVERTICALREQMINUS1SRC_UNNAMED0;
    DW3.Numhzreqminus1Src                            = NUMHZREQMINUS1SRC_UNNAMED0;
    DW3.PreFetchoffsetforsource                      = PRE_FETCHOFFSETFORSOURCE_UNNAMED_4;

    DW4.Value                                        = 0;

    DW5.Value                                        = 0;
    DW5.CaptureMode                                  = CAPTURE_MODE_UNNAMED0;
    DW5.ParallelCaptureAndEncodeSessionId            = PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID_UNNAMED0;

}

mhw_vdbox_vdenc_g12_X::VDENC_REF_SURFACE_STATE_CMD::VDENC_REF_SURFACE_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopb                                       = SUBOPB_VDENCREFSURFACESTATE;
    DW0.Subopa                                       = SUBOPA_UNNAMED0;
    DW0.Opcode                                       = OPCODE_VDENCPIPE;
    DW0.Pipeline                                     = PIPELINE_MFXCOMMON;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        

}

mhw_vdbox_vdenc_g12_X::VDENC_SRC_SURFACE_STATE_CMD::VDENC_SRC_SURFACE_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopb                                       = SUBOPB_VDENCSRCSURFACESTATE;
    DW0.Subopa                                       = SUBOPA_UNNAMED0;
    DW0.Opcode                                       = OPCODE_VDENCPIPE;
    DW0.Pipeline                                     = PIPELINE_MFXCOMMON;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        

}

mhw_vdbox_vdenc_g12_X::VDENC_WALKER_STATE_CMD::VDENC_WALKER_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopb                                       = SUBOPB_VDENCWALKERSTATE;
    DW0.Subopa                                       = SUBOPA_UNNAMED0;
    DW0.Opcode                                       = OPCODE_VDENCPIPE;
    DW0.Pipeline                                     = PIPELINE_MFXCOMMON;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        

    DW4.Value                                        = 0;        

    DW5.Value                                        = 0;        

    DW6.Value                                        = 0;        

    DW7.Value                                        = 0;        

    DW8.Value                                        = 0;        

    DW9.Value                                        = 0;        

}

mhw_vdbox_vdenc_g12_X::VDENC_CONTROL_STATE_CMD::VDENC_CONTROL_STATE_CMD()
{
    DW0.Value                                        = 0;
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.MediaInstructionCommand                      = 0xB;
    DW0.MediaInstructionOpcode                       = 0x1;
    DW0.PipelineType                                 = 0x2;
    DW0.CommandType                                  = 0x3;

    DW1.Value                                        = 0;
}

mhw_vdbox_vdenc_g12_X::VDENC_CMD1_CMD::VDENC_CMD1_CMD()
{
    DW0.Value                                        = 0;
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopb                                       = SUBOPB_VDENCCMD1CMD;
    DW0.Subopa                                       = SUBOPA_UNNAMED0;
    DW0.Opcode                                       = OPCODE_VDENCPIPE;
    DW0.Pipeline                                     = PIPELINE_MFXCOMMON;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

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

    DW11.Value                                       = 0;

    DW12.Value                                       = 0;

    DW13.Value                                       = 0;

    DW14.Value                                       = 0;

    DW15.Value                                       = 0;

    DW16.Value                                       = 0;

    DW17.Value                                       = 0;

    DW18.Value                                       = 0;

    DW19.Value                                       = 0;

    DW20.Value                                       = 0;

    DW21.Value                                       = 0;

    DW22.Value                                       = 0;

    DW23.Value                                       = 0;

    DW24.Value                                       = 0;

    DW25.Value                                       = 0;

    DW26.Value                                       = 0;

    DW27.Value                                       = 0;

    DW28.Value                                       = 0;

    DW29.Value                                       = 0;

    DW30.Value                                       = 0;
}

mhw_vdbox_vdenc_g12_X::VDENC_CMD2_CMD::VDENC_CMD2_CMD()
{
    DW0.Value                                        = 0;
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopb                                       = SUBOPB_VDENCCMD2CMD;
    DW0.Subopa                                       = SUBOPA_UNNAMED0;
    DW0.Opcode                                       = OPCODE_VDENCPIPE;
    DW0.Pipeline                                     = PIPELINE_MFXCOMMON;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

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

    DW11.Value                                       = 0;

    DW12.Value                                       = 0;

    DW13.Value                                       = 0;

    DW14.Value                                       = 0;

    DW15.Value                                       = 0;

    DW16.Value                                       = 0;

    DW17.Value                                       = 0;

    DW18.Value                                       = 0;

    DW19.Value                                       = 0;

    DW20.Value                                       = 0;

    DW21.Value                                       = 0;

    DW22.Value                                       = 0;

    DW23.Value                                       = 0;

    DW24.Value                                       = 0;

    DW25.Value                                       = 0;

    DW26.Value                                       = 0;

    DW27.Value                                       = 0;

    DW28.Value                                       = 0;

    DW29.Value                                       = 0;

    DW30.Value                                       = 0;

    DW31.Value                                       = 0;

    DW32.Value                                       = 0;

    DW33.Value                                       = 0;

    DW34.Value                                       = 0;

    DW35.Value                                       = 0;

    DW36.Value                                       = 0;

    DW37.Value                                       = 0;

    DW38.Value                                       = 0;

    DW39.Value                                       = 0;

    DW40.Value                                       = 0;

    DW41.Value                                       = 0;

    DW42.Value                                       = 0;

    DW43.Value                                       = 0;

    DW44.Value                                       = 0;

    DW45.Value                                       = 0;

    DW46.Value                                       = 0;

    DW47.Value                                       = 0;

    DW48.Value                                       = 0;

    DW49.Value                                       = 0;

    DW50.Value                                       = 0;
}
