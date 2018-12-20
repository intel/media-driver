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
//! \file   mhw_state_heap_hwcmd_g11_X.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

#include "mhw_state_heap_hwcmd_g11_X.h"
#include "mos_utilities.h"

mhw_state_heap_g11_X::INTERFACE_DESCRIPTOR_DATA_CMD::INTERFACE_DESCRIPTOR_DATA_CMD()
{
    DW0.Value                                        = 0;        

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        
    DW2.FloatingPointMode                            = FLOATING_POINT_MODE_IEEE_754;
    DW2.ThreadPriority                               = THREAD_PRIORITY_NORMALPRIORITY;
    DW2.SingleProgramFlow                            = SINGLE_PROGRAM_FLOW_MULTIPLE;
    DW2.DenormMode                                   = DENORM_MODE_FTZ;
    DW2.ThreadPreemptionDisable                      = THREAD_PREEMPTION_DISABLE_DISABLE;

    DW3.Value                                        = 0;        
    DW3.SamplerCount                                 = SAMPLER_COUNT_NOSAMPLERSUSED;

    DW4.Value                                        = 0;        

    DW5.Value                                        = 0;        

    DW6.Value                                        = 0;        
    DW6.SharedLocalMemorySize                        = SHARED_LOCAL_MEMORY_SIZE_ENCODES0K;
    DW6.RoundingMode                                 = ROUNDING_MODE_RTNE;

    DW7.Value                                        = 0;        

}

mhw_state_heap_g11_X::BINDING_TABLE_STATE_CMD::BINDING_TABLE_STATE_CMD()
{
    DW0.Value                                        = 0;        

}

mhw_state_heap_g11_X::RENDER_SURFACE_STATE_CMD::RENDER_SURFACE_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.MediaBoundaryPixelMode                       = MEDIA_BOUNDARY_PIXEL_MODE_NORMALMODE;
    DW0.RenderCacheReadWriteMode                     = RENDER_CACHE_READ_WRITE_MODE_WRITE_ONLYCACHE;
    DW0.TileMode                                     = TILE_MODE_LINEAR;
    DW0.SurfaceHorizontalAlignment                   = 0;
    DW0.SurfaceVerticalAlignment                     = 0;
    DW0.SurfaceFormat                                = SURFACE_FORMAT_R32G32B32A32FLOAT;
    DW0.SurfaceType                                  = SURFACE_TYPE_SURFTYPE1D;

    DW1.Value                                        = 0;        
    DW1.SampleTapDiscardDisable                      = SAMPLE_TAP_DISCARD_DISABLE_DISABLE;
    DW1.CornerTexelMode                              = CORNER_TEXEL_MODE_DISABLE;
    DW1.EnableUnormPathInColorPipe                   = ENABLE_UNORM_PATH_IN_COLOR_PIPE_UNNAMED1;

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        
    DW3.StandardTilingModeExtensions                 = STANDARD_TILING_MODE_EXTENSIONS_DISABLE;
    DW3.TileAddressMappingMode                       = TILE_ADDRESS_MAPPING_MODE_GEN9;

    DW4.Value                                        = 0;        
    DW4.NumberOfMultisamples                         = NUMBER_OF_MULTISAMPLES_MULTISAMPLECOUNT1;
    DW4.MultisampledSurfaceStorageFormat             = MULTISAMPLED_SURFACE_STORAGE_FORMAT_MSS;
    DW4.RenderTargetAndSampleUnormRotation           = RENDER_TARGET_AND_SAMPLE_UNORM_ROTATION_0DEG;

    DW5.Value                                        = 0;        
    DW5.CoherencyType                                = COHERENCY_TYPE_GPUCOHERENT;
    DW5.TiledResourceMode                            = TILED_RESOURCE_MODE_NONE;
    DW5.EwaDisableForCube                            = EWA_DISABLE_FOR_CUBE_ENABLE;

    DW6.Value                                        = 0;        
    DW6.Obj0.HalfPitchForChroma                      = HALF_PITCH_FOR_CHROMA_DISABLE;
    DW6.Obj1.AuxiliarySurfaceMode                    = AUXILIARY_SURFACE_MODE_AUXNONE;
    DW6.Obj2.YuvInterpolationEnable                  = YUV_INTERPOLATION_ENABLE_DISABLE;

    DW7.Value                                        = 0;        
    DW7.ShaderChannelSelectAlpha                     = SHADER_CHANNEL_SELECT_ALPHA_ZERO;
    DW7.ShaderChannelSelectBlue                      = SHADER_CHANNEL_SELECT_BLUE_ZERO;
    DW7.ShaderChannelSelectGreen                     = SHADER_CHANNEL_SELECT_GREEN_ZERO;
    DW7.ShaderChannelSelectRed                       = SHADER_CHANNEL_SELECT_RED_ZERO;
    DW7.MemoryCompressionMode                        = MEMORY_COMPRESSION_MODE_HORIZONTAL;

    DW8_9.Value[0] = DW8_9.Value[1]                  = 0;        

    DW10_11.Value[0] = DW10_11.Value[1]              = 0;        
    DW10_11.Obj0.ClearValueAddressEnable             = CLEAR_VALUE_ADDRESS_ENABLE_DISABLE;

    DW12.Value                                       = 0;        

    DW13.Value                                       = 0;        

    DW14.Value                                       = 0;        

    DW15.Value                                       = 0;        

}

mhw_state_heap_g11_X::MEDIA_SURFACE_STATE_CMD::MEDIA_SURFACE_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.Rotation                                     = ROTATION_NOROTATIONOR0DEGREE;

    DW1.Value                                        = 0;        
    DW1.CrVCbUPixelOffsetVDirection                  = CRVCBU_PIXEL_OFFSET_V_DIRECTION_UNNAMED0;
    DW1.PictureStructure                             = PICTURE_STRUCTURE_FRAMEPICTURE;

    DW2.Value                                        = 0;        
    DW2.TileMode                                     = TILE_MODE_TILEMODELINEAR;
    DW2.AddressControl                               = ADDRESS_CONTROL_CLAMP;
    DW2.MemoryCompressionMode                        = MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE;
    DW2.CrVCbUPixelOffsetVDirectionMsb               = CRVCBU_PIXEL_OFFSET_V_DIRECTION_MSB_UNNAMED0;
    DW2.CrVCbUPixelOffsetUDirection                  = CRVCBU_PIXEL_OFFSET_U_DIRECTION_UNNAMED0;
    DW2.SurfaceFormat                                = SURFACE_FORMAT_YCRCBNORMAL;

    DW3.Value                                        = 0;        

    DW4.Value                                        = 0;        

    DW5.Value                                        = 0;        
    DW5.SurfaceMemoryObjectControlState              = SURFACE_MEMORY_OBJECT_CONTROL_STATE_DEFAULTVAUEDESC;
    DW5.TiledResourceMode                            = TILED_RESOURCE_MODE_TRMODENONE;

    DW6.Value                                        = 0;        

    DW7.Value                                        = 0;        

}

mhw_state_heap_g11_X::SAMPLER_STATE_CMD::SAMPLER_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.LodAlgorithm                                 = LOD_ALGORITHM_LEGACY;
    DW0.MinModeFilter                                = MIN_MODE_FILTER_NEAREST;
    DW0.MagModeFilter                                = MAG_MODE_FILTER_NEAREST;
    DW0.MipModeFilter                                = MIP_MODE_FILTER_NONE;
    DW0.CoarseLodQualityMode                         = COARSE_LOD_QUALITY_MODE_DISABLED;
    DW0.LodPreclampMode                              = LOD_PRECLAMP_MODE_NONE;
    DW0.TextureBorderColorMode                       = TEXTURE_BORDER_COLOR_MODE_OGL;

    DW1.Value                                        = 0;        
    DW1.CubeSurfaceControlMode                       = CUBE_SURFACE_CONTROL_MODE_PROGRAMMED;
    DW1.ShadowFunction                               = SHADOW_FUNCTION_PREFILTEROPALWAYS;
    DW1.ChromakeyMode                                = CHROMAKEY_MODE_KEYFILTERKILLONANYMATCH;

    DW2.Value                                        = 0;        
    DW2.LodClampMagnificationMode                    = LOD_CLAMP_MAGNIFICATION_MODE_MIPNONE;
    DW2.SrgbDecode                                   = SRGB_DECODE_DECODEEXT;
    DW2.ReturnFilterWeightForNullTexels              = RETURN_FILTER_WEIGHT_FOR_NULL_TEXELS_DISABLE;
    DW2.ReturnFilterWeightForBorderTexels            = RETURN_FILTER_WEIGHT_FOR_BORDER_TEXELS_DISABLE;

    DW3.Value                                        = 0;        
    DW3.TczAddressControlMode                        = TCZ_ADDRESS_CONTROL_MODE_WRAP;
    DW3.TcyAddressControlMode                        = TCY_ADDRESS_CONTROL_MODE_WRAP;
    DW3.TcxAddressControlMode                        = TCX_ADDRESS_CONTROL_MODE_WRAP;
    DW3.TrilinearFilterQuality                       = TRILINEAR_FILTER_QUALITY_FULL;
    DW3.MaximumAnisotropy                            = MAXIMUM_ANISOTROPY_RATIO21;
    DW3.ReductionType                                = REDUCTION_TYPE_STDFILTER;

}

mhw_state_heap_g11_X::SAMPLER_STATE_8x8_AVS_COEFFICIENTS_CMD::SAMPLER_STATE_8x8_AVS_COEFFICIENTS_CMD()
{
    DW0.Value                                        = 0;        

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        

    DW4.Value                                        = 0;        

    DW5.Value                                        = 0;        

    DW6.Value                                        = 0;        

    DW7.Value                                        = 0;        

}

mhw_state_heap_g11_X::SAMPLER_STATE_8x8_AVS_CMD::SAMPLER_STATE_8x8_AVS_CMD()
{
    DW0.Value                                        = 0;        
    DW0.GainFactor                                   = GAIN_FACTOR_UNNAMED44;
    DW0.WeakEdgeThreshold                            = WEAK_EDGE_THRESHOLD_UNNAMED1;
    DW0.StrongEdgeThreshold                          = STRONG_EDGE_THRESHOLD_UNNAMED8;
    DW0.R3XCoefficient                               = R3X_COEFFICIENT_UNNAMED5;
    DW0.R3CCoefficient                               = R3C_COEFFICIENT_UNNAMED5;

    DW1.Value                                        = 0;        

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
    DW3.HueMax                                       = HUE_MAX_UNNAMED14;
    DW3.Enable8TapFilter                             = ENABLE_8_TAP_FILTER_UNNAMED0;
    DW3.Ief4SmoothEnable                             = IEF4SMOOTH_ENABLE_UNNAMED0;
    DW3.SkinToneTunedIefEnable                       = SKIN_TONE_TUNED_IEF_ENABLE_UNNAMED1;

    DW4.Value                                        = 0;        
    DW4.ShuffleOutputwritebackForSample8X8           = SHUFFLE_OUTPUTWRITEBACK_FOR_SAMPLE_8X8_UNNAMED0;
    DW4.DiamondMargin                                = DIAMOND_MARGIN_UNNAMED4;
    DW4.UMid                                         = U_MID_UNNAMED110;
    DW4.VMid                                         = V_MID_UNNAMED154;

    DW5.Value                                        = 0;        
    DW5.DiamondDv                                    = DIAMOND_DV_UNNAMED0;
    DW5.DiamondTh                                    = DIAMOND_TH_UNNAMED35;
    DW5.HsMargin                                     = HS_MARGIN_UNNAMED3;
    DW5.DiamondDu                                    = DIAMOND_DU_UNNAMED2;
    DW5.Skindetailfactor                             = SKINDETAILFACTOR_UNNAMED0;

    DW6.Value                                        = 0;        
    DW6.YPoint1                                      = Y_POINT_1_UNNAMED46;
    DW6.YPoint2                                      = Y_POINT_2_UNNAMED47;
    DW6.YPoint3                                      = Y_POINT_3_UNNAMED254;
    DW6.YPoint4                                      = Y_POINT_4_UNNAMED255;

    DW7.Value                                        = 0;        

    DW8.Value                                        = 0;        
    DW8.P0L                                          = P0L_UNNAMED46;
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
    DW12.P0U                                         = P0U_UNNAMED46;
    DW12.P1U                                         = P1U_UNNAMED66;

    DW13.Value                                       = 0;        
    DW13.P2U                                         = P2U_UNNAMED150;
    DW13.P3U                                         = P3U_UNNAMED236;
    DW13.B0U                                         = B0U_UNNAMED143;
    DW13.B1U                                         = B1U_UNNAMED163;

    DW14.Value                                       = 0;        
    DW14.B2U                                         = B2U_UNNAMED200;
    DW14.B3U                                         = B3U_UNNAMED140;

    DW15.Value                                       = 0;        

    DW152.Value                                      = 0;        
    DW152.DefaultSharpnessLevel                      = DEFAULT_SHARPNESS_LEVEL_UNNAMED0;

    DW153.Value                                      = 0;        
    DW153.RgbAdaptive                                = RGB_ADAPTIVE_DISBLE;
    DW153.AdaptiveFilterForAllChannels               = ADAPTIVE_FILTER_FOR_ALL_CHANNELS_DISBLE;
    DW153.BypassYAdaptiveFiltering                   = BYPASS_Y_ADAPTIVE_FILTERING_ENABLE;
    DW153.BypassXAdaptiveFiltering                   = BYPASS_X_ADAPTIVE_FILTERING_ENABLE;

    MOS_ZeroMemory(&Reserved4928, sizeof(Reserved4928));        
}

mhw_state_heap_g11_X::SAMPLER_STATE_8x8_CONVOLVE_COEFFICIENTS_CMD::SAMPLER_STATE_8x8_CONVOLVE_COEFFICIENTS_CMD()
{
    DW0.Value                                        = 0;        

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        

    DW4.Value                                        = 0;        

    DW5.Value                                        = 0;        

    DW6.Value                                        = 0;        

    DW7.Value                                        = 0;        

}

mhw_state_heap_g11_X::SAMPLER_STATE_8x8_CONVOLVE_CMD::SAMPLER_STATE_8x8_CONVOLVE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.SizeOfTheCoefficient                         = SIZE_OF_THE_COEFFICIENT_8BIT;
    DW0.MsbHeight                                    = MSB_HEIGHT_NOCHANGE;
    DW0.MsbWidth                                     = MSB_WIDTH_NOCHANGE;

    MOS_ZeroMemory(&Reserved32, sizeof(Reserved32));        
}

mhw_state_heap_g11_X::SAMPLER_STATE_8x8_ERODE_DILATE_MINMAXFILTER_CMD::SAMPLER_STATE_8x8_ERODE_DILATE_MINMAXFILTER_CMD()
{
    DW0.Value                                        = 0;        

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        

    DW4.Value                                        = 0;        

    DW5.Value                                        = 0;        

    DW6.Value                                        = 0;        

    DW7.Value                                        = 0;        

}

mhw_state_heap_g11_X::SAMPLER_INDIRECT_STATE_CMD::SAMPLER_INDIRECT_STATE_CMD()
{
    DW0.Value                                        = 0;        

    DW1.Value                                        = 0;        

    DW2.Value                                        = 0;        

    DW3.Value                                        = 0;        

    MOS_ZeroMemory(&Reserved128, sizeof(Reserved128));        
}

