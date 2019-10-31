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
//! \file   mhw_state_heap_hwcmd_g12_X.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

// DO NOT EDIT

#include "mhw_state_heap_hwcmd_g12_X.h"
#include <string.h>

mhw_state_heap_g12_X::INTERFACE_DESCRIPTOR_DATA_CMD::INTERFACE_DESCRIPTOR_DATA_CMD()
{
    DW0.Value                                        = 0x00000000;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;
    //DW2.FloatingPointMode                            = FLOATING_POINT_MODE_IEEE_754;
    //DW2.ThreadPriority                               = THREAD_PRIORITY_NORMALPRIORITY;
    //DW2.SingleProgramFlow                            = SINGLE_PROGRAM_FLOW_MULTIPLE;
    //DW2.DenormMode                                   = DENORM_MODE_FTZ;
    //DW2.ThreadPreemptionDisable                      = THREAD_PREEMPTION_DISABLE_DISABLE;

    DW3.Value                                        = 0x00000000;
    //DW3.SamplerCount                                 = SAMPLER_COUNT_NOSAMPLERSUSED;

    DW4.Value                                        = 0x00000000;

    DW5.Value                                        = 0x00000000;

    DW6.Value                                        = 0x00000000;
    //DW6.OverDispatchControl                          = OVER_DISPATCH_CONTROL_NONE;
    //DW6.SharedLocalMemorySize                        = SHARED_LOCAL_MEMORY_SIZE_ENCODES0K;
    //DW6.RoundingMode                                 = ROUNDING_MODE_RTNE;

    DW7.Value                                        = 0x00000000;

}

mhw_state_heap_g12_X::BINDING_TABLE_STATE_CMD::BINDING_TABLE_STATE_CMD()
{
    DW0.Value                                        = 0;        

}

mhw_state_heap_g12_X::RENDER_SURFACE_STATE_CMD::RENDER_SURFACE_STATE_CMD()
{
    DW0.Value                                        = 0x0001c000;
    //DW0.MediaBoundaryPixelMode                       = MEDIA_BOUNDARY_PIXEL_MODE_NORMALMODE;
    //DW0.RenderCacheReadWriteMode                     = RENDER_CACHE_READ_WRITE_MODE_WRITE_ONLYCACHE;
    //DW0.TileMode                                     = TILE_MODE_LINEAR;
    //DW0.SurfaceHorizontalAlignment                   = SURFACE_HORIZONTAL_ALIGNMENT_HALIGN16;
    //DW0.SurfaceVerticalAlignment                     = SURFACE_VERTICAL_ALIGNMENT_VALIGN4;
    //DW0.SurfaceFormat                                = SURFACE_FORMAT_R32G32B32A32FLOAT;
    //DW0.SurfaceType                                  = SURFACE_TYPE_SURFTYPE1D;

    DW1.Value                                        = 0x80000000;
    //DW1.SampleTapDiscardDisable                      = SAMPLE_TAP_DISCARD_DISABLE_DISABLE;
    //DW1.DoubleFetchDisable                           = DOUBLE_FETCH_DISABLE_ENABLE;
    //DW1.CornerTexelMode                              = CORNER_TEXEL_MODE_DISABLE;
    //DW1.EnableUnormPathInColorPipe                   = ENABLE_UNORM_PATH_IN_COLOR_PIPE_ENABLE;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;
    //DW3.NullProbingEnable                            = NULL_PROBING_ENABLE_DISABLE;
    //DW3.StandardTilingModeExtensions                 = STANDARD_TILING_MODE_EXTENSIONS_DISABLE;
    //DW3.TileAddressMappingMode                       = TILE_ADDRESS_MAPPING_MODE_GEN9;

    DW4.Value                                        = 0x00000000;
    //DW4.NumberOfMultisamples                         = NUMBER_OF_MULTISAMPLES_MULTISAMPLECOUNT1;
    //DW4.MultisampledSurfaceStorageFormat             = MULTISAMPLED_SURFACE_STORAGE_FORMAT_MSS;
    //DW4.RenderTargetAndSampleUnormRotation           = RENDER_TARGET_AND_SAMPLE_UNORM_ROTATION_0DEG;
    //DW4.DecompressInL3                               = DECOMPRESS_IN_L3_DISABLE;

    DW5.Value                                        = 0x00000000;
    //DW5.CoherencyType                                = COHERENCY_TYPE_GPUCOHERENT;
    //DW5.TiledResourceMode                            = TILED_RESOURCE_MODE_NONE;
    //DW5.EwaDisableForCube                            = EWA_DISABLE_FOR_CUBE_ENABLE;

    DW6.Value                                        = 0x00000000;
    //DW6.Obj0.AuxiliarySurfaceMode                    = AUXILIARY_SURFACE_MODE_AUXNONE;
    //DW6.Obj2.HalfPitchForChroma                      = HALF_PITCH_FOR_CHROMA_DISABLE;
    //DW6.Obj3.YuvInterpolationEnable                  = YUV_INTERPOLATION_ENABLE_DISABLE;

    DW7.Value                                        = 0x00000000;
    //DW7.ShaderChannelSelectAlpha                     = SHADER_CHANNEL_SELECT_ALPHA_ZERO;
    //DW7.ShaderChannelSelectBlue                      = SHADER_CHANNEL_SELECT_BLUE_ZERO;
    //DW7.ShaderChannelSelectGreen                     = SHADER_CHANNEL_SELECT_GREEN_ZERO;
    //DW7.ShaderChannelSelectRed                       = SHADER_CHANNEL_SELECT_RED_ZERO;
    //DW7.MemoryCompressionMode                        = MEMORY_COMPRESSION_MODE_HORIZONTAL;

    DW8_9.Value[0] = DW8_9.Value[1]                  = 0x00000000;

    DW10_11.Value[0] = DW10_11.Value[1]              = 0x00000000;
    //DW10_11.Obj0.ClearValueAddressEnable             = CLEAR_VALUE_ADDRESS_ENABLE_DISABLE;

    DW12.Value                                       = 0x00000000;

    DW13.Value                                       = 0x00000000;

    DW14.Value                                       = 0x00000000;

    DW15.Value                                       = 0x00000000;

}

mhw_state_heap_g12_X::MEDIA_SURFACE_STATE_CMD::MEDIA_SURFACE_STATE_CMD()
{
    DW0.Value                                        = 0x00000000;
    //DW0.Rotation                                     = ROTATION_NOROTATIONOR0DEGREE;

    DW1.Value                                        = 0x00000000;
    //DW1.CrVCbUPixelOffsetVDirection                  = CRVCBU_PIXEL_OFFSET_V_DIRECTION_UNNAMED0;
    //DW1.PictureStructure                             = PICTURE_STRUCTURE_FRAMEPICTURE;

    DW2.Value                                        = 0x00000000;
    //DW2.TileMode                                     = TILE_MODE_TILEMODELINEAR;
    //DW2.AddressControl                               = ADDRESS_CONTROL_CLAMP;
    //DW2.MemoryCompressionType                        = MEMORY_COMPRESSION_TYPE_MEDIACOMPRESSION;
    //DW2.CrVCbUPixelOffsetVDirectionMsb               = CRVCBU_PIXEL_OFFSET_V_DIRECTION_MSB_UNNAMED0;
    //DW2.CrVCbUPixelOffsetUDirection                  = CRVCBU_PIXEL_OFFSET_U_DIRECTION_UNNAMED0;
    //DW2.SurfaceFormat                                = SURFACE_FORMAT_YCRCBNORMAL;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;

    DW5.Value                                        = 0x00000000;
    //DW5.SurfaceMemoryObjectControlState              = SURFACE_MEMORY_OBJECT_CONTROL_STATE_DEFAULTVAUEDESC;
    //DW5.TiledResourceMode                            = TILED_RESOURCE_MODE_TRMODENONE;

    DW6.Value                                        = 0x00000000;

    DW7.Value                                        = 0x00000000;

}

mhw_state_heap_g12_X::SAMPLER_STATE_CMD::SAMPLER_STATE_CMD()
{
    DW0.Value                                        = 0x00000000;
    //DW0.LodAlgorithm                                 = LOD_ALGORITHM_LEGACY;
    //DW0.MinModeFilter                                = MIN_MODE_FILTER_NEAREST;
    //DW0.MagModeFilter                                = MAG_MODE_FILTER_NEAREST;
    //DW0.MipModeFilter                                = MIP_MODE_FILTER_NONE;
    //DW0.CoarseLodQualityMode                         = COARSE_LOD_QUALITY_MODE_DISABLED;
    //DW0.LodPreclampMode                              = LOD_PRECLAMP_MODE_NONE;
    //DW0.TextureBorderColorMode                       = TEXTURE_BORDER_COLOR_MODE_OGL;

    DW1.Value                                        = 0x00000000;
    //DW1.CubeSurfaceControlMode                       = CUBE_SURFACE_CONTROL_MODE_PROGRAMMED;
    //DW1.ShadowFunction                               = SHADOW_FUNCTION_PREFILTEROPALWAYS;
    //DW1.ChromakeyMode                                = CHROMAKEY_MODE_KEYFILTERKILLONANYMATCH;

    DW2.Value                                        = 0x00000000;
    //DW2.LodClampMagnificationMode                    = LOD_CLAMP_MAGNIFICATION_MODE_MIPNONE;
    //DW2.SrgbDecode                                   = SRGB_DECODE_DECODEEXT;
    //DW2.ReturnFilterWeightForNullTexels              = RETURN_FILTER_WEIGHT_FOR_NULL_TEXELS_DISABLE;
    //DW2.ReturnFilterWeightForBorderTexels            = RETURN_FILTER_WEIGHT_FOR_BORDER_TEXELS_DISABLE;

    DW3.Value                                        = 0x00000000;
    //DW3.TczAddressControlMode                        = TCZ_ADDRESS_CONTROL_MODE_WRAP;
    //DW3.TcyAddressControlMode                        = TCY_ADDRESS_CONTROL_MODE_WRAP;
    //DW3.TcxAddressControlMode                        = TCX_ADDRESS_CONTROL_MODE_WRAP;
    //DW3.TrilinearFilterQuality                       = TRILINEAR_FILTER_QUALITY_FULL;
    //DW3.MaximumAnisotropy                            = MAXIMUM_ANISOTROPY_RATIO21;
    //DW3.ReductionType                                = REDUCTION_TYPE_STDFILTER;

}

mhw_state_heap_g12_X::SAMPLER_STATE_8x8_AVS_COEFFICIENTS_CMD::SAMPLER_STATE_8x8_AVS_COEFFICIENTS_CMD()
{
    DW0.Value                                        = 0x00000000;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;

    DW5.Value                                        = 0x00000000;

    DW6.Value                                        = 0x00000000;

    DW7.Value                                        = 0x00000000;

}

mhw_state_heap_g12_X::SAMPLER_STATE_8x8_AVS_CMD::SAMPLER_STATE_8x8_AVS_CMD()
{
    memset(&Reserved0, 0, sizeof(Reserved0));

    DW3.Value                                        = 0x00000000;
    //DW3.Enable8TapFilter                             = ENABLE_8_TAP_FILTER_UNNAMED0;

    DW4.Value                                        = 0x00000000;
    //DW4.ShuffleOutputwritebackForSample8X8           = SHUFFLE_OUTPUTWRITEBACK_FOR_SAMPLE_8X8_UNNAMED0;

    memset(&Reserved160, 0, sizeof(Reserved160));

    DW152.Value                                      = 0x00000000;
    //DW152.DefaultSharpnessLevel                      = DEFAULT_SHARPNESS_LEVEL_UNNAMED0;

    DW153.Value                                      = 0x00000000;
    //DW153.RgbAdaptive                                = RGB_ADAPTIVE_DISBLE;
    //DW153.AdaptiveFilterForAllChannels               = ADAPTIVE_FILTER_FOR_ALL_CHANNELS_DISBLE;
    //DW153.BypassYAdaptiveFiltering                   = BYPASS_Y_ADAPTIVE_FILTERING_ENABLE;
    //DW153.BypassXAdaptiveFiltering                   = BYPASS_X_ADAPTIVE_FILTERING_ENABLE;

    memset(&Reserved4928, 0, sizeof(Reserved4928));

}

mhw_state_heap_g12_X::SAMPLER_INDIRECT_STATE_CMD::SAMPLER_INDIRECT_STATE_CMD()
{
    DW0.Value                                        = 0x00000000;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    memset(&Reserved128, 0, sizeof(Reserved128));

}

