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
//!
//! \file   mhw_sfc_hwcmd_g12_X.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

#include "mhw_sfc_hwcmd_g12_X.h"
#include "mos_utilities.h"

mhw_sfc_g12_X::SFC_AVS_STATE_CMD::SFC_AVS_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopcodeb                                   = SUBOPCODEB_SFCAVSSTATE;
    DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
    DW0.MediaCommandOpcode                           = 0;
    DW0.Pipeline                                     = PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        
    DW1.SharpnessLevel                               = SHARPNESS_LEVEL_UNNAMED0;

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        
    DW3.InputVerticalSitingSpecifiesTheVerticalSitingOfTheInput = INPUT_VERTICAL_SITING_SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT_0;
    DW3.InputHorizontalSitingValueSpecifiesTheHorizontalSitingOfTheInput = INPUT_HORIZONTAL_SITING_VALUE_SPECIFIES_THE_HORIZONTAL_SITING_OF_THE_INPUT_0_FRACTIONININTEGER;

}

mhw_sfc_g12_X::SFC_IEF_STATE_CMD::SFC_IEF_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopcodeb                                   = SUBOPCODEB_SFCIEFSTATE;
    DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
    DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMFXVEBOXSFCMODE;
    DW0.Pipeline                                     = PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        
    DW1.GainFactor                                   = GAIN_FACTOR_UNNAMED_4_4;
    DW1.WeakEdgeThreshold                            = WEAK_EDGE_THRESHOLD_UNNAMED1;
    DW1.StrongEdgeThreshold                          = STRONG_EDGE_THRESHOLD_UNNAMED8;
    DW1.R3XCoefficient                               = R3X_COEFFICIENT_UNNAMED5;
    DW1.R3CCoefficient                               = R3C_COEFFICIENT_UNNAMED5;

    DW2.Value                                        = 0;        
    DW2.GlobalNoiseEstimation                        = GLOBAL_NOISE_ESTIMATION_UNNAMED255;
    DW2.NonEdgeWeight                                = NON_EDGE_WEIGHT_UNNAMED1;
    DW2.RegularWeight                                = REGULAR_WEIGHT_UNNAMED2;
    DW2.StrongEdgeWeight                             = STRONG_EDGE_WEIGHT_UNNAMED7;
    DW2.R5XCoefficient                               = R5X_COEFFICIENT_UNNAMED7;
    DW2.R5CxCoefficient                              = R5CX_COEFFICIENT_UNNAMED7;
    DW2.R5CCoefficient                               = R5C_COEFFICIENT_UNNAMED7;

    DW3.Value                                        = 0;        
    DW3.SatMax                                       = SAT_MAX_UNNAMED31;
    DW3.HueMax                                       = HUE_MAX_UNNAMED1_4;

    DW4.Value                                        = 0;        
    DW4.DiamondMargin                                = DIAMOND_MARGIN_UNNAMED_4;
    DW4.UMid                                         = U_MID_UNNAMED110;
    DW4.VMid                                         = V_MID_UNNAMED15_4;

    DW5.Value                                        = 0;        
    DW5.DiamondDv                                    = DIAMOND_DV_UNNAMED0;
    DW5.DiamondTh                                    = DIAMOND_TH_UNNAMED35;
    DW5.HsMargin                                     = HS_MARGIN_UNNAMED3;
    DW5.DiamondDu                                    = DIAMOND_DU_UNNAMED0;
    DW5.SkinDetailFactor                             = SKIN_DETAIL_FACTOR_DETAILREVEALED;

    DW6.Value                                        = 0;        
    DW6.YPoint1                                      = Y_POINT_1_UNNAMED_46;
    DW6.YPoint2                                      = Y_POINT_2_UNNAMED_47;
    DW6.YPoint3                                      = Y_POINT_3_UNNAMED25_4;
    DW6.YPoint4                                      = Y_POINT_4_UNNAMED255;

    DW7.Value                                        = 0;        

    DW8.Value                                        = 0;        
    DW8.P0L                                          = P0L_UNNAMED_46;
    DW8.P1L                                          = P1L_UNNAMED216;

    DW9.Value                                        = 0;        
    DW9.P2L                                          = P2L_UNNAMED236;
    DW9.P3L                                          = P3L_UNNAMED236;
    DW9.B0L                                          = B0L_UNNAMED133;
    DW9.B1L                                          = B1L_UNNAMED130;

    DW10.Value                                       = 0;        
    DW10.B2L                                         = B2L_UNNAMED130;
    DW10.B3L                                         = B3L_UNNAMED130;

    DW11.Value                                       = 0;        

    DW12.Value                                       = 0;        
    DW12.P0U                                         = P0U_UNNAMED_46;
    DW12.P1U                                         = P1U_UNNAMED66;

    DW13.Value                                       = 0;        
    DW13.P2U                                         = P2U_UNNAMED150;
    DW13.P3U                                         = P3U_UNNAMED236;
    DW13.B0U                                         = B0U_UNNAMED1_43;
    DW13.B1U                                         = B1U_UNNAMED163;

    DW14.Value                                       = 0;        
    DW14.B2U                                         = B2U_UNNAMED200;
    DW14.B3U                                         = B3U_UNNAMED1_40;

    DW15.Value                                       = 0;        

    DW16.Value                                       = 0;        
    DW16.C0                                          = C0_UNNAMED102_4;
    DW16.C1                                          = C1_UNNAMED0;

    DW17.Value                                       = 0;        
    DW17.C2                                          = C2_UNNAMED0;
    DW17.C3                                          = C3_UNNAMED0;

    DW18.Value                                       = 0;        
    DW18.C4                                          = C4_UNNAMED102_4;
    DW18.C5                                          = C5_UNNAMED0;

    DW19.Value                                       = 0;        
    DW19.C6                                          = C6_UNNAMED0;
    DW19.C7                                          = C7_UNNAMED0;

    DW20.Value                                       = 0;        
    DW20.C8                                          = C8_UNNAMED102_4;

    DW21.Value                                       = 0;        
    DW21.OffsetIn1                                   = OFFSET_IN_1_UNNAMED0;
    DW21.OffsetOut1                                  = OFFSET_OUT_1_UNNAMED0;

    DW22.Value                                       = 0;        
    DW22.OffsetIn2                                   = OFFSET_IN_2_UNNAMED0;
    DW22.OffsetOut2                                  = OFFSET_OUT_2_UNNAMED0;

    DW23.Value                                       = 0;        
    DW23.OffsetIn3                                   = OFFSET_IN_3_UNNAMED0;
    DW23.OffsetOut3                                  = OFFSET_OUT_3_UNNAMED0;

}

mhw_sfc_g12_X::SFC_FRAME_START_CMD::SFC_FRAME_START_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopcodeb                                   = SUBOPCODEB_SFCFRAMESTART;
    DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
    DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMFXVEBOXSFCMODE;
    DW0.Pipeline                                     = PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        

}

mhw_sfc_g12_X::SFC_LOCK_CMD::SFC_LOCK_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopcodeb                                   = SUBOPCODEB_SFCLOCK;
    DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
    DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMFXVEBOXSFCMODE;
    DW0.Pipeline                                     = PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        

}

mhw_sfc_g12_X::SFC_STATE_CMD::SFC_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopcodeb                                   = SUBOPCODEB_SFCSTATE;
    DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
    DW0.MediaCommandOpcode                           = 0;
    DW0.Pipeline                                     = PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        
    DW1.SfcPipeMode                                  = SFC_PIPE_MODE_UNNAMED0;
    DW1.SfcInputChromaSubSampling                    = SFC_INPUT_CHROMA_SUB_SAMPLING_400;
    DW1.VdVeInputOrderingMode                        = VDVE_INPUT_ORDERING_MODE_UNNAMED0;

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        
    DW3.OutputSurfaceFormatType                      = OUTPUT_SURFACE_FORMAT_TYPE_AYUV;
    DW3.RgbaChannelSwapEnable                        = RGBA_CHANNEL_SWAP_ENABLE_UNNAMED0;
    DW3.OutputChromaDownsamplingCoSitingPositionVerticalDirection = OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_08_LEFTFULLPIXEL;
    DW3.OutputChromaDownsamplingCoSitingPositionHorizontalDirection = OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_08_LEFTFULLPIXEL;
    DW3.InputColorSpace0Yuv1Rgb                      = INPUT_COLOR_SPACE_0_YUV1_RGB_YUVCOLORSPACE;

    DW4.Value                                        = 0;        
    DW4.IefEnable                                    = IEF_ENABLE_DISABLE;
    DW4.Ief4SmoothEnable                             = IEF4SMOOTH_ENABLE_UNNAMED0;
    DW4.AvsFilterMode                                = AVS_FILTER_MODE_5X5POLY_PHASEFILTERBILINEAR_ADAPTIVE;
    DW4.AdaptiveFilterForAllChannels                 = ADAPTIVE_FILTER_FOR_ALL_CHANNELS_DISABLEADAPTIVEFILTERONUVRBCHANNELS;
    DW4.AvsScalingEnable                             = AVS_SCALING_ENABLE_DISABLE;
    DW4.BypassYAdaptiveFiltering                     = BYPASS_Y_ADAPTIVE_FILTERING_ENABLEYADAPTIVEFILTERING;
    DW4.BypassXAdaptiveFiltering                     = BYPASS_X_ADAPTIVE_FILTERING_ENABLEXADAPTIVEFILTERING;
    DW4.RotationMode                                 = ROTATION_MODE_0_DEGREES;
    DW4.Bitdepth                                     = BITDEPTH_10BITFORMAT;

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
    DW14.ScalingFactorHeight                         = SCALING_FACTOR_HEIGHT_UNNAMED0;

    DW15.Value                                       = 0;        
    DW15.ScalingFactorWidth                          = SCALING_FACTOR_WIDTH_UNNAMED0;

    DW16.Value                                       = 0;        

    DW17.Value                                       = 0;        

    DW18.Value                                       = 0;        

    DW19.Value                                       = 0;        
    DW19.OutputFrameSurfaceBaseAddressMemoryCompressionMode = OUTPUT_FRAME_SURFACE_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSION;
    DW19.OutputFrameSurfaceBaseAddressRowStoreScratchBufferCacheSelect = OUTPUT_FRAME_SURFACE_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_DISABLE;
    DW19.OutputSurfaceTiledMode                      = OUTPUT_SURFACE_TILED_MODE_TRMODENONE;

    DW20.Value                                       = 0;        

    DW21.Value                                       = 0;        

    DW22.Value                                       = 0;        
    DW22.AvsLineBufferBaseAddressMemoryCompressionEnable = AVS_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE;
    DW22.AvsLineBufferBaseAddressMemoryCompressionMode = AVS_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE;
    DW22.AvsLineBufferBaseAddressRowStoreScratchBufferCacheSelect = AVS_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;
    DW22.AvsLineBufferTiledMode                      = AVS_LINE_BUFFER_TILED_MODE_TRMODENONE;

    DW23.Value                                       = 0;        

    DW24.Value                                       = 0;        

    DW25.Value                                       = 0;        
    DW25.IefLineBufferBaseAddressMemoryCompressionEnable = IEF_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE;
    DW25.IefLineBufferBaseAddressMemoryCompressionMode = IEF_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0;
    DW25.IefLineBufferBaseAddressRowStoreScratchBufferCacheSelect = IEF_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;
    DW25.IefLineBufferTiledMode                      = IEF_LINE_BUFFER_TILED_MODE_TRMODENONE;

    DW26.Value                                       = 0;        

    DW27.Value                                       = 0;        

    DW28.Value                                       = 0;        
    DW28.SfdLineBufferBaseAddressMemoryCompressionEnable = SFD_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE;
    DW28.SfdLineBufferBaseAddressMemoryCompressionMode = SFD_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0;
    DW28.SfdLineBufferBaseAddressRowStoreScratchBufferCacheSelect = SFD_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;
    DW28.SfdLineBufferTiledMode                      = SFD_LINE_BUFFER_TILED_MODE_TRMODENONE;

    DW29.Value                                       = 0;        
    DW29.OutputSurfaceTileWalk                       = OUTPUT_SURFACE_TILE_WALK_TILEWALKXMAJOR;
    DW29.OutputSurfaceTiled                          = OUTPUT_SURFACE_TILED_FALSE;

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
    DW40.AvsLineTileBufferBaseAddressMemoryCompressionEnable = AVS_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE;
    DW40.AvsLineTileBufferBaseAddressMemoryCompressionMode = AVS_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0;
    DW40.AvsLineTileBufferBaseAddressRowStoreScratchBufferCacheSelect = AVS_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;
    DW40.AvsLineTileBufferTiledMode                  = AVS_LINE_TILE_BUFFER_TILED_MODE_TRMODENONE;

    DW41.Value                                       = 0;        

    DW42.Value                                       = 0;        

    DW43.Value                                       = 0;        
    DW43.IefLineTileBufferBaseAddressMemoryCompressionEnable = IEF_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE;
    DW43.IefLineTileBufferBaseAddressMemoryCompressionMode = IEF_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0;
    DW43.IefLineTileBufferBaseAddressRowStoreScratchBufferCacheSelect = IEF_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;
    DW43.IefLineTileBufferTiledMode                  = IEF_LINE_TILE_BUFFER_TILED_MODE_TRMODENONE;

    DW44.Value                                       = 0;        

    DW45.Value                                       = 0;        

    DW46.Value                                       = 0;        
    DW46.SfdLineTileBufferBaseAddressMemoryCompressionEnable = SFD_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE;
    DW46.SfdLineTileBufferBaseAddressMemoryCompressionMode = SFD_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0;
    DW46.SfdLineTileBufferBaseAddressRowStoreScratchBufferCacheSelect = SFD_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;
    DW46.SfdLineTileBufferTiledMode                  = SFD_LINE_TILE_BUFFER_TILED_MODE_TRMODENONE;

    DW47.Value                                       = 0;
    DW48.Value                                       = 0;
    DW49.Value                                       = 0;
}

mhw_sfc_g12_X::SFC_AVS_LUMA_Coeff_Table_CMD::SFC_AVS_LUMA_Coeff_Table_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopcodeb                                   = SUBOPCODEB_SFCAVSLUMACOEFFTABLE;
    DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
    DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMFXVEBOXSFCMODE;
    DW0.Pipeline                                     = PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        

    DW4.Value                                        = 0;        

    MOS_ZeroMemory(&FilterCoefficients, sizeof(FilterCoefficients));        
}

mhw_sfc_g12_X::SFC_AVS_CHROMA_Coeff_Table_CMD::SFC_AVS_CHROMA_Coeff_Table_CMD()
{
    DW0.Value                                        = 0;        
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopcodeb                                   = SUBOPCODEB_SFCAVSCHROMACOEFFTABLE;
    DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
    DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMFXVEBOXSFCMODE;
    DW0.Pipeline                                     = PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

    MOS_ZeroMemory(&FilterCoefficients, sizeof(FilterCoefficients));        
}

