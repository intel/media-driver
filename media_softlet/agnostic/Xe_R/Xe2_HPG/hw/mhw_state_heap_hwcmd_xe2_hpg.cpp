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
//! \file   mhw_state_heap_xe2_hpg.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

#include "mhw_state_heap_hwcmd_xe2_hpg.h"
#include <string.h>

mhw_state_heap_xe2_hpg::INTERFACE_DESCRIPTOR_DATA_CMD::INTERFACE_DESCRIPTOR_DATA_CMD()
{
    DW0.Value                                        = 0x00000000;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;
    //DW2.RegistersPerThread                           = REGISTERS_PER_THREAD_DEFAULT;
    //DW2.FloatingPointMode                            = FLOATING_POINT_MODE_IEEE_754;
    //DW2.SingleProgramFlow                            = SINGLE_PROGRAM_FLOW_MULTIPLE;
    //DW2.DenormMode                                   = DENORM_MODE_FTZ;
    //DW2.ThreadPreemption                             = THREAD_PREEMPTION_DISABLE;

    DW3.Value                                        = 0x00000000;
    //DW3.SamplerCount                                 = SAMPLER_COUNT_NOSAMPLERSUSED;

    DW4.Value                                        = 0x00000000;
    //DW4.BindingTableEntryCount                       = BINDING_TABLE_ENTRY_COUNT_PREFETCHDISABLED;

    DW5.Value                                        = 0x00000000;
    //DW5.ThreadGroupForwardProgressGuarantee          = THREAD_GROUP_FORWARD_PROGRESS_GUARANTEE_DISABLE;
    //DW5.SharedLocalMemorySize                        = SHARED_LOCAL_MEMORY_SIZE_ENCODES0KB;
    //DW5.RoundingMode                                 = ROUNDING_MODE_RTNE;
    //DW5.ThreadGroupDispatchSize                      = THREAD_GROUP_DISPATCH_SIZE_TGSIZE8;
    //DW5.NumberOfBarriers                             = NUMBER_OF_BARRIERS_NONE;
    //DW5.BtdMode                                      = BTD_MODE_DISABLE;

    DW6.Value                                        = 0x00000000;
    //DW6.PreferredSlmSizeOverride                     = PREFERRED_SLM_SIZE_OVERRIDE_UNNAMED0;

    DW7.Value                                        = 0x00000000;

}

mhw_state_heap_xe2_hpg::BINDING_TABLE_STATE_CMD::BINDING_TABLE_STATE_CMD()
{
    DW0.Value = 0;
}

mhw_state_heap_xe2_hpg::RENDER_SURFACE_STATE_CMD::RENDER_SURFACE_STATE_CMD()
{
    DW0.Value                                        = 0x00010000;
    //DW0.MediaBoundaryPixelMode                       = MEDIA_BOUNDARY_PIXEL_MODE_NORMALMODE;
    //DW0.RenderCacheReadWriteMode                     = RENDER_CACHE_READ_WRITE_MODE_WRITE_ONLYCACHE;
    //DW0.TileMode                                     = TILE_MODE_LINEAR;
    //DW0.SurfaceHorizontalAlignment                   = SURFACE_HORIZONTAL_ALIGNMENT_HALIGN16;
    //DW0.SurfaceVerticalAlignment                     = SURFACE_VERTICAL_ALIGNMENT_VALIGN4;
    //DW0.SurfaceFormat                                = SURFACE_FORMAT_R32G32B32A32FLOAT;
    //DW0.SurfaceType                                  = SURFACE_TYPE_SURFTYPE1D;

    DW1.Value                                        = 0x00000000;
    //DW1.SampleTapDiscardDisable                      = SAMPLE_TAP_DISCARD_DISABLE_DISABLE;
    //DW1.CornerTexelMode                              = CORNER_TEXEL_MODE_DISABLE;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;
    //DW4.NumberOfMultisamples                         = NUMBER_OF_MULTISAMPLES_MULTISAMPLECOUNT1;
    //DW4.MultisampledSurfaceStorageFormat             = MULTISAMPLED_SURFACE_STORAGE_FORMAT_MSS;
    //DW4.RenderTargetAndSampleUnormRotation           = RENDER_TARGET_AND_SAMPLE_UNORM_ROTATION_0DEG;
    //DW4.DecompressInL3                               = DECOMPRESS_IN_L3_DISABLE;

    DW5.Value                                        = 0x00000F00;
    //DW5.CoherencyType                                = COHERENCY_TYPE_SINGLE_GPUCOHERENT;
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

    DW8_9.Value[0] = DW8_9.Value[1]                  = 0x00000000;

    DW10_11.Value[0] = DW10_11.Value[1]              = 0x00000000;

    DW12.Value                                       = 0x00000000;
    //DW12.Compressionformat                           = COMPRESSIONFORMAT_CMFR8;

    DW13.Value                                       = 0x00000000;

    DW14.Value                                       = 0x00000000;

    DW15.Value                                       = 0x00000000;

}

mhw_state_heap_xe2_hpg::MEDIA_SURFACE_STATE_CMD::MEDIA_SURFACE_STATE_CMD()
{
    DW0.Value                                        = 0x00000000;
    //DW0.CompressionFormat                            = COMPRESSION_FORMAT_CMFR8;
    //DW0.Rotation                                     = ROTATION_NOROTATIONOR0DEGREE;

    DW1.Value                                        = 0x00000000;
    //DW1.CrVCbUPixelOffsetVDirection                  = CRVCBU_PIXEL_OFFSET_V_DIRECTION_UNNAMED0;
    //DW1.PictureStructure                             = PICTURE_STRUCTURE_FRAMEPICTURE;

    DW2.Value                                        = 0x00000000;
    //DW2.TileMode                                     = TILE_MODE_TILEMODELINEAR;
    //DW2.AddressControl                               = ADDRESS_CONTROL_CLAMP;
    //DW2.CrVCbUPixelOffsetVDirectionMsb               = CRVCBU_PIXEL_OFFSET_V_DIRECTION_MSB_UNNAMED0;
    //DW2.CrVCbUPixelOffsetUDirection                  = CRVCBU_PIXEL_OFFSET_U_DIRECTION_UNNAMED0;
    //DW2.SurfaceFormat                                = SURFACE_FORMAT_YCRCBNORMAL;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;

    DW5.Value                                        = 0x00000000;
    //DW5.SurfaceMemoryObjectControlState              = SURFACE_MEMORY_OBJECT_CONTROL_STATE_DEFAULTVAUEDESC;

    DW6.Value                                        = 0x00000000;

    DW7.Value                                        = 0x00000000;

}

mhw_state_heap_xe2_hpg::SAMPLER_STATE_CMD::SAMPLER_STATE_CMD()
{
    DW0.Value                                        = 0x00000000;
    //DW0.LodAlgorithm                                 = LOD_ALGORITHM_LEGACY;
    //DW0.MinModeFilter                                = MIN_MODE_FILTER_NEAREST;
    //DW0.MagModeFilter                                = MAG_MODE_FILTER_NEAREST;
    //DW0.MipModeFilter                                = MIP_MODE_FILTER_NONE;
    //DW0.LowQualityCubeCornerModeEnable               = LOW_QUALITY_CUBE_CORNER_MODE_ENABLE_DISABLE;
    //DW0.LodPreclampMode                              = LOD_PRECLAMP_MODE_NONE;

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
    //DW3.MipLinearFilterQuality                       = MIP_LINEAR_FILTER_QUALITY_FULLQUALITY;
    //DW3.MaximumAnisotropy                            = MAXIMUM_ANISOTROPY_RATIO21;
    //DW3.ReductionType                                = REDUCTION_TYPE_STDFILTER;
    //DW3.LowQualityFilter                             = LOW_QUALITY_FILTER_DISABLE;

}

mhw_state_heap_xe2_hpg::SAMPLER_STATE_8x8_AVS_COEFFICIENTS_CMD::SAMPLER_STATE_8x8_AVS_COEFFICIENTS_CMD()
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

mhw_state_heap_xe2_hpg::SAMPLER_STATE_8x8_AVS_CMD::SAMPLER_STATE_8x8_AVS_CMD()
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

mhw_state_heap_xe2_hpg::SAMPLER_INDIRECT_STATE_CMD::SAMPLER_INDIRECT_STATE_CMD()
{
    DW0.Value                                        = 0x00000000;

    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    memset(&Reserved128, 0, sizeof(Reserved128));

}