/*===================== begin_copyright_notice ==================================

* Copyright (c) 2024, Intel Corporation
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

======================= end_copyright_notice ==================================*/
//!
//! \file   mhw_sfc_hwcmd_xe2_lpm_base.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

// DO NOT EDIT

#include "mhw_sfc_hwcmd_xe2_lpm_base_next.h"
#include "mos_utilities.h"

mhw::sfc::xe2_lpm_base_next::Cmd::SFC_AVS_STATE_CMD::SFC_AVS_STATE_CMD()
{
    DW0.Value                                        = 0x75020002;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.Subopcodeb                                   = SUBOPCODEB_SFCAVSSTATE;
    //DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMISC;
    //DW0.Pipeline                                     = PIPELINE_MEDIA;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.SharpnessLevel                               = SHARPNESS_LEVEL_UNNAMED0;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;
    //DW3.InputVerticalSitingSpecifiesTheVerticalSitingOfTheInput = INPUT_VERTICAL_SITING_SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT_0;

}

mhw::sfc::xe2_lpm_base_next::Cmd::SFC_IEF_STATE_CMD::SFC_IEF_STATE_CMD()
{
    DW0.Value                                        = 0x75030016;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.Subopcodeb                                   = SUBOPCODEB_SFCIEFSTATE;
    //DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMISC;
    //DW0.Pipeline                                     = PIPELINE_MEDIA;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x0294806c;
    //DW1.GainFactor                                   = GAIN_FACTOR_UNNAMED44;
    //DW1.WeakEdgeThreshold                            = WEAK_EDGE_THRESHOLD_UNNAMED1;
    //DW1.StrongEdgeThreshold                          = STRONG_EDGE_THRESHOLD_UNNAMED8;
    //DW1.R3XCoefficient                               = R3X_COEFFICIENT_UNNAMED5;
    //DW1.R3CCoefficient                               = R3C_COEFFICIENT_UNNAMED5;

    DW2.Value                                        = 0x39cfd1ff;
    //DW2.GlobalNoiseEstimation                        = GLOBAL_NOISE_ESTIMATION_UNNAMED255;
    //DW2.NonEdgeWeight                                = NON_EDGE_WEIGHT_UNNAMED1;
    //DW2.RegularWeight                                = REGULAR_WEIGHT_UNNAMED2;
    //DW2.StrongEdgeWeight                             = STRONG_EDGE_WEIGHT_UNNAMED7;
    //DW2.R5XCoefficient                               = R5X_COEFFICIENT_UNNAMED7;
    //DW2.R5CxCoefficient                              = R5CX_COEFFICIENT_UNNAMED7;
    //DW2.R5CCoefficient                               = R5C_COEFFICIENT_UNNAMED7;

    DW3.Value                                        = 0x039f0000;
    //DW3.SatMax                                       = SAT_MAX_UNNAMED31;
    //DW3.HueMax                                       = HUE_MAX_UNNAMED14;

    DW4.Value                                        = 0x9a6e4000;
    //DW4.DiamondMargin                                = DIAMOND_MARGIN_UNNAMED4;
    //DW4.UMid                                         = U_MID_UNNAMED110;
    //DW4.VMid                                         = V_MID_UNNAMED154;

    DW5.Value                                        = 0x00601180;
    //DW5.DiamondDv                                    = DIAMOND_DV_UNNAMED0;
    //DW5.DiamondTh                                    = DIAMOND_TH_UNNAMED35;
    //DW5.HsMargin                                     = HS_MARGIN_UNNAMED3;
    //DW5.DiamondDu                                    = DIAMOND_DU_UNNAMED0;
    //DW5.SkinDetailFactor                             = SKIN_DETAIL_FACTOR_DETAILREVEALED;

    DW6.Value                                        = 0xfffe2f2e;
    //DW6.YPoint1                                      = Y_POINT_1_UNNAMED46;
    //DW6.YPoint2                                      = Y_POINT_2_UNNAMED47;
    //DW6.YPoint3                                      = Y_POINT_3_UNNAMED254;
    //DW6.YPoint4                                      = Y_POINT_4_UNNAMED255;

    DW7.Value                                        = 0x00000000;

    DW8.Value                                        = 0xd82e0000;
    //DW8.P0L                                          = P0L_UNNAMED46;
    //DW8.P1L                                          = P1L_UNNAMED216;

    DW9.Value                                        = 0x8285ecec;
    //DW9.P2L                                          = P2L_UNNAMED236;
    //DW9.P3L                                          = P3L_UNNAMED236;
    //DW9.B0L                                          = B0L_UNNAMED133;
    //DW9.B1L                                          = B1L_UNNAMED130;

    DW10.Value                                       = 0x00008282;
    //DW10.B2L                                         = B2L_UNNAMED130;
    //DW10.B3L                                         = B3L_UNNAMED130;

    DW11.Value                                       = 0x00000000;

    DW12.Value                                       = 0x02117000;
    //DW12.P0U                                         = P0U_UNNAMED46;
    //DW12.P1U                                         = P1U_UNNAMED66;

    DW13.Value                                       = 0xa38fec96;
    //DW13.P2U                                         = P2U_UNNAMED150;
    //DW13.P3U                                         = P3U_UNNAMED236;
    //DW13.B0U                                         = B0U_UNNAMED143;
    //DW13.B1U                                         = B1U_UNNAMED163;

    DW14.Value                                       = 0x00008cc8;
    //DW14.B2U                                         = B2U_UNNAMED200;
    //DW14.B3U                                         = B3U_UNNAMED140;

    DW15.Value                                       = 0x00000000;

    DW16.Value                                       = 0x00002000;
    //DW16.C0                                          = C0_UNNAMED1024;
    //DW16.C1                                          = C1_UNNAMED0;

    DW17.Value                                       = 0x00000000;
    //DW17.C2                                          = C2_UNNAMED0;
    //DW17.C3                                          = C3_UNNAMED0;

    DW18.Value                                       = 0x00000400;
    //DW18.C4                                          = C4_UNNAMED1024;
    //DW18.C5                                          = C5_UNNAMED0;

    DW19.Value                                       = 0x00000000;
    //DW19.C6                                          = C6_UNNAMED0;
    //DW19.C7                                          = C7_UNNAMED0;

    DW20.Value                                       = 0x00000400;
    //DW20.C8                                          = C8_UNNAMED1024;

    DW21.Value                                       = 0x00000000;
    //DW21.OffsetIn1                                   = OFFSET_IN_1_UNNAMED0;
    //DW21.OffsetOut1                                  = OFFSET_OUT_1_UNNAMED0;

    DW22.Value                                       = 0x00000000;
    //DW22.OffsetIn2                                   = OFFSET_IN_2_UNNAMED0;
    //DW22.OffsetOut2                                  = OFFSET_OUT_2_UNNAMED0;

    DW23.Value                                       = 0x00000000;
    //DW23.OffsetIn3                                   = OFFSET_IN_3_UNNAMED0;
    //DW23.OffsetOut3                                  = OFFSET_OUT_3_UNNAMED0;

}

mhw::sfc::xe2_lpm_base_next::Cmd::SFC_FRAME_START_CMD::SFC_FRAME_START_CMD()
{
    DW0.Value                                        = 0x75040000;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.Subopcodeb                                   = SUBOPCODEB_SFCFRAMESTART;
    //DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMISC;
    //DW0.Pipeline                                     = PIPELINE_MEDIA;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

}

mhw::sfc::xe2_lpm_base_next::Cmd::SFC_LOCK_CMD::SFC_LOCK_CMD()
{
    DW0.Value                                        = 0x75000000;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.Subopcodeb                                   = SUBOPCODEB_SFCLOCK;
    //DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMISC;
    //DW0.Pipeline                                     = PIPELINE_MEDIA;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

}

mhw::sfc::xe2_lpm_base_next::Cmd::SFC_STATE_CMD::SFC_STATE_CMD()
{
    DW0.Value                                        = 0x7501003d;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.Subopcodeb                                   = SUBOPCODEB_SFCSTATE;
    //DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMFXVEBOXSFCMODE;
    //DW0.Pipeline                                     = PIPELINE_MEDIA;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;
    //DW1.SfcPipeMode                                  = SFC_PIPE_MODE_UNNAMED0;
    //DW1.SfcInputChromaSubSampling                    = SFC_INPUT_CHROMA_SUB_SAMPLING_400;
    //DW1.VdVeInputOrderingMode                        = VDVE_INPUT_ORDERING_MODE_UNNAMED0;
    //DW1.SfcEngineMode                                = SFC_ENGINE_MODE_SINGLESFC;
    //DW1.InputFrameDataFormat                         = INPUT_FRAME_DATA_FORMAT_PROGRESSIVE;
    //DW1.OutputFrameDataFormat                        = OUTPUT_FRAME_DATA_FORMAT_PROGRESSIVE;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;
    //DW3.OutputSurfaceFormatType                      = OUTPUT_SURFACE_FORMAT_TYPE_AYUV;
    //DW3.OutputChromaDownsamplingCoSitingPositionVerticalDirection = OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_08_LEFTFULLPIXEL;
    //DW3.OutputChromaDownsamplingCoSitingPositionHorizontalDirection = OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_08_LEFTFULLPIXEL;
    //DW3.InputColorSpace0Yuv1Rgb                      = INPUT_COLOR_SPACE_0_YUV1_RGB_YUVCOLORSPACE;
    //DW3.OutputCompressionFormat                      = OUTPUT_COMPRESSION_FORMAT_CMFR8;

    DW4.Value                                        = 0x00000000;
    //DW4.IefEnable                                    = IEF_ENABLE_DISABLE;
    //DW4.Ief4SmoothEnable                             = IEF4SMOOTH_ENABLE_UNNAMED0;
    //DW4.AvsFilterMode                                = AVS_FILTER_MODE_5X5POLY_PHASEFILTERBILINEAR_ADAPTIVE;
    //DW4.AdaptiveFilterForAllChannels                 = ADAPTIVE_FILTER_FOR_ALL_CHANNELS_DISABLEADAPTIVEFILTERONUVRBCHANNELS;
    //DW4.AvsScalingEnable                             = AVS_SCALING_ENABLE_DISABLE;
    //DW4.BypassYAdaptiveFiltering                     = BYPASS_Y_ADAPTIVE_FILTERING_ENABLEYADAPTIVEFILTERING;
    //DW4.BypassXAdaptiveFiltering                     = BYPASS_X_ADAPTIVE_FILTERING_ENABLEXADAPTIVEFILTERING;
    //DW4.MirrorType                                   = MIRROR_TYPE_HORIZONTALFLIP;
    //DW4.MirrorMode                                   = MIRROR_MODE_MIRRORMODEDISABLED;
    //DW4.RotationMode                                 = ROTATION_MODE_0_DEGREES;
    //DW4.Bitdepth                                     = BITDEPTH_10BITFORMAT;

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
    //DW19.OutputFrameSurfaceBaseAddressRowStoreScratchBufferCacheSelect = OUTPUT_FRAME_SURFACE_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_DISABLE;

    DW20.Value                                       = 0x00000000;

    DW21.Value                                       = 0x00000000;

    DW22.Value                                       = 0x00000000;
    //DW22.AvsLineBufferBaseAddressMemoryCompressionMode = AVS_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE;
    //DW22.AvsLineBufferBaseAddressRowStoreScratchBufferCacheSelect = AVS_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;

    DW23.Value                                       = 0x00000000;

    DW24.Value                                       = 0x00000000;

    DW25.Value                                       = 0x00000000;
    //DW25.IefLineBufferBaseAddressMemoryCompressionMode = IEF_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0;
    //DW25.IefLineBufferBaseAddressRowStoreScratchBufferCacheSelect = IEF_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;

    DW26.Value                                       = 0x00000000;

    DW27.Value                                       = 0x00000000;

    DW28.Value                                       = 0x00000000;
    //DW28.SfdLineBufferBaseAddressMemoryCompressionMode = SFD_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0;
    //DW28.SfdLineBufferBaseAddressRowStoreScratchBufferCacheSelect = SFD_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;

    DW29.Value                                       = 0x00000000;
    //DW29.TiledMode                                   = TILED_MODE_LINEAR;

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
    //DW40.AvsLineTileBufferBaseAddressMemoryCompressionMode = AVS_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0;
    //DW40.AvsLineTileBufferBaseAddressRowStoreScratchBufferCacheSelect = AVS_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;

    DW41.Value                                       = 0x00000000;

    DW42.Value                                       = 0x00000000;

    DW43.Value                                       = 0x00000000;
    //DW43.IefLineTileBufferBaseAddressMemoryCompressionMode = IEF_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0;
    //DW43.IefLineTileBufferBaseAddressRowStoreScratchBufferCacheSelect = IEF_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;

    DW44.Value                                       = 0x00000000;

    DW45.Value                                       = 0x00000000;

    DW46.Value                                       = 0x00000000;
    //DW46.SfdLineTileBufferBaseAddressMemoryCompressionMode = SFD_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0;
    //DW46.SfdLineTileBufferBaseAddressRowStoreScratchBufferCacheSelect = SFD_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;

    DW47.Value                                       = 0x00000000;

    DW48.Value                                       = 0x00000000;

    DW49.Value                                       = 0x00000000;
    //DW49.HistogramBaseAddressMemoryCompressionType   = HISTOGRAM_BASE_ADDRESS_MEMORY_COMPRESSION_TYPE_UNNAMED0;
    //DW49.HistogramBaseAddressCacheSelect             = HISTOGRAM_BASE_ADDRESS_CACHE_SELECT_LLC;

    DW50.Value                                       = 0x00000000;

    DW51.Value                                       = 0x00000000;

    DW52.Value                                       = 0x00000000;

    DW53.Value                                       = 0x00000000;

    DW54.Value                                       = 0x00000000;

    DW55.Value                                       = 0x00000000;

    DW56.Value                                       = 0x00000000;

    DW57.Value                                       = 0x00000000;
    //DW57.BottomFiledSurfaceBaseAddressMemoryCompressionType = BOTTOM_FILED_SURFACE_BASE_ADDRESS_MEMORY_COMPRESSION_TYPE_MEDIACOMPRESSIONENABLED;

    DW58.Value                                       = 0x00000000;
    //DW58.BottomFieldSurfaceTileWalk                  = BOTTOM_FIELD_SURFACE_TILE_WALK_TILEWALKXMAJOR;
    //DW58.BottomFieldSurfaceTiled                     = BOTTOM_FIELD_SURFACE_TILED_FALSE;

    DW59.Value                                       = 0x00000000;

    DW60.Value                                       = 0x00000000;

    DW61.Value                                       = 0x00000000;

    DW62.Value                                       = 0x00000000;

}

mhw::sfc::xe2_lpm_base_next::Cmd::SFC_AVS_LUMA_Coeff_Table_CMD::SFC_AVS_LUMA_Coeff_Table_CMD()
{
    DW0.Value                                        = 0x7505007f;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.Subopcodeb                                   = SUBOPCODEB_SFCAVSLUMACOEFFTABLE;
    //DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMISC;
    //DW0.Pipeline                                     = PIPELINE_MEDIA;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;

    MOS_ZeroMemory(&FilterCoefficients, sizeof(FilterCoefficients));
}

mhw::sfc::xe2_lpm_base_next::Cmd::SFC_AVS_CHROMA_Coeff_Table_CMD::SFC_AVS_CHROMA_Coeff_Table_CMD()
{
    DW0.Value                                        = 0x7506003f;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.Subopcodeb                                   = SUBOPCODEB_SFCAVSCHROMACOEFFTABLE;
    //DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
    //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMISC;
    //DW0.Pipeline                                     = PIPELINE_MEDIA;
    //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    MOS_ZeroMemory(&FilterCoefficients, sizeof(FilterCoefficients));
}

