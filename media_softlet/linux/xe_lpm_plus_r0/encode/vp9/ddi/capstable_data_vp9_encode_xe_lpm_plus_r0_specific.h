/*
* Copyright (c) 2022, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
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
//! \file     capstable_data_vp9_encode_xe_lpm_plus_r0_specific.h
//! \brief    This file register all caps data
//!

#ifndef __CAPSTABLE_DATA_VP9_ENCODE_XE_LPM_PLUS_R0_SPECIFIC_H__
#define __CAPSTABLE_DATA_VP9_ENCODE_XE_LPM_PLUS_R0_SPECIFIC_H__

#include "capstable_data_xe_lpm_plus_r0_specific.h"
#include "codec_def_encode_vp9.h"

//!
//! \brief  Definition for bitset value
//!
static const VAConfigAttribValEncROI VP9Common_VAEntrypointEncSlice_encROI
{
    {0,0,1,0}
};

static const VAConfigAttribValEncRateControlExt VP9Common_VAEntrypointEncSlice_encRateControlExt
{
    {CODECHAL_ENCODE_VP9_MAX_NUM_TEMPORAL_LAYERS - 1,1,0}
};

#if VA_CHECK_VERSION(1, 23, 0)
static const VAConfigAttribValEncVP9 VP9Common_VAEntrypointEncSlice_encVP9
{
    {15,0,0}
};
#endif

//! \brief  Definition for ConfigDataList
static ConfigDataList configDataList_VAProfileVP9Profile0_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{ 
  {VA_RC_CQP, 0}, 
  {VA_RC_CBR, 0}, 
  {VA_RC_VBR, 0}, 
  {VA_RC_ICQ, 0} 
}; 

static ConfigDataList configDataList_VAProfileVP9Profile1_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{ 
  {VA_RC_CQP, 0}, 
  {VA_RC_CBR, 0}, 
  {VA_RC_VBR, 0}, 
  {VA_RC_ICQ, 0} 
}; 

static ConfigDataList configDataList_VAProfileVP9Profile2_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{ 
  {VA_RC_CQP, 0}, 
  {VA_RC_CBR, 0}, 
  {VA_RC_VBR, 0}, 
  {VA_RC_ICQ, 0} 
}; 

static ConfigDataList configDataList_VAProfileVP9Profile3_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{ 
  {VA_RC_CQP, 0}, 
  {VA_RC_CBR, 0}, 
  {VA_RC_VBR, 0}, 
  {VA_RC_ICQ, 0} 
}; 

//!
//! \brief  Definition for AttribList
//!
static const AttribList attribList_VAProfileVP9Profile0_VAEntrypointEncSlice_Xe_Lpm_plus_r0
{
   //{VAConfigAttribRTFormat, VA_RT_FORMAT_YUV444_10},
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 | VA_RT_FORMAT_RGB32_10},
   {VAConfigAttribMaxPictureWidth, CODEC_8K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_8K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_RAW_DATA},
   //just to match with legacy correct value 2
   {VAConfigAttribEncMaxRefFrames, 3},
#if VA_CHECK_VERSION(1, 23, 0)
   {VAConfigAttribEncVP9, VP9Common_VAEntrypointEncSlice_encVP9.value},
#endif
   {VAConfigAttribRateControl, VA_RC_CQP | VA_RC_ICQ | VA_RC_CBR | VA_RC_VBR | VA_RC_MB},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_ENCODE},
   {VAConfigAttribEncTileSupport, 1},
   {VAConfigAttribEncDirtyRect, 4},
   {VAConfigAttribEncDynamicScaling, 1},
   {VAConfigAttribEncQualityRange, NUM_TARGET_USAGE_MODES - 1},
   {VAConfigAttribEncInterlaced, VA_ENC_INTERLACED_NONE},
   {VAConfigAttribEncQuantization, VA_ENC_QUANTIZATION_NONE},
   {VAConfigAttribEncIntraRefresh, VA_ENC_INTRA_REFRESH_NONE},
   {VAConfigAttribEncSkipFrame, 0},
   {VAConfigAttribEncROI, VP9Common_VAEntrypointEncSlice_encROI.value},
   {VAConfigAttribEncParallelRateControl, 0},
   {VAConfigAttribFEIMVPredictors, 0},
   {VAConfigAttribEncRateControlExt, VP9Common_VAEntrypointEncSlice_encRateControlExt.value},
};

static const AttribList attribList_VAProfileVP9Profile1_VAEntrypointEncSlice_Xe_Lpm_plus_r0
{
   //{VAConfigAttribRTFormat, VA_RT_FORMAT_YUV444_10},
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 | VA_RT_FORMAT_RGB32_10},
   {VAConfigAttribMaxPictureWidth, CODEC_8K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_8K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_RAW_DATA},
   //just to match with legacy correct value 2
   {VAConfigAttribEncMaxRefFrames, 3},
#if VA_CHECK_VERSION(1, 23, 0)
   {VAConfigAttribEncVP9, VP9Common_VAEntrypointEncSlice_encVP9.value},
#endif
   {VAConfigAttribRateControl, VA_RC_CQP | VA_RC_ICQ | VA_RC_CBR | VA_RC_VBR | VA_RC_MB},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_ENCODE},
   {VAConfigAttribEncTileSupport, 1},
   {VAConfigAttribEncDirtyRect, 4},
   {VAConfigAttribEncDynamicScaling, 1},
   {VAConfigAttribEncQualityRange, NUM_TARGET_USAGE_MODES - 1},
   {VAConfigAttribEncInterlaced, VA_ENC_INTERLACED_NONE},
   {VAConfigAttribEncQuantization, VA_ENC_QUANTIZATION_NONE},
   {VAConfigAttribEncIntraRefresh, VA_ENC_INTRA_REFRESH_NONE},
   {VAConfigAttribEncSkipFrame, 0},
   {VAConfigAttribEncROI, VP9Common_VAEntrypointEncSlice_encROI.value},
   {VAConfigAttribEncParallelRateControl, 0},
   {VAConfigAttribFEIMVPredictors, 0},
   {VAConfigAttribEncRateControlExt, VP9Common_VAEntrypointEncSlice_encRateControlExt.value},
};

static const AttribList attribList_VAProfileVP9Profile2_VAEntrypointEncSlice_Xe_Lpm_plus_r0
{
   //{VAConfigAttribRTFormat, VA_RT_FORMAT_YUV444_10},
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 | VA_RT_FORMAT_RGB32_10},
   {VAConfigAttribMaxPictureWidth, CODEC_8K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_8K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_RAW_DATA},
   //just to match with legacy correct value 2
   {VAConfigAttribEncMaxRefFrames, 3},
#if VA_CHECK_VERSION(1, 23, 0)
   {VAConfigAttribEncVP9, VP9Common_VAEntrypointEncSlice_encVP9.value},
#endif
   {VAConfigAttribRateControl, VA_RC_CQP | VA_RC_ICQ | VA_RC_CBR | VA_RC_VBR | VA_RC_MB},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_ENCODE},
   {VAConfigAttribEncTileSupport, 1},
   {VAConfigAttribEncDirtyRect, 4},
   {VAConfigAttribEncDynamicScaling, 1},
   {VAConfigAttribEncQualityRange, NUM_TARGET_USAGE_MODES - 1},
   {VAConfigAttribEncInterlaced, VA_ENC_INTERLACED_NONE},
   {VAConfigAttribEncQuantization, VA_ENC_QUANTIZATION_NONE},
   {VAConfigAttribEncIntraRefresh, VA_ENC_INTRA_REFRESH_NONE},
   {VAConfigAttribEncSkipFrame, 0},
   {VAConfigAttribEncROI, VP9Common_VAEntrypointEncSlice_encROI.value},
   {VAConfigAttribEncParallelRateControl, 0},
   {VAConfigAttribFEIMVPredictors, 0},
   {VAConfigAttribEncRateControlExt, VP9Common_VAEntrypointEncSlice_encRateControlExt.value},
};

static const AttribList attribList_VAProfileVP9Profile3_VAEntrypointEncSlice_Xe_Lpm_plus_r0
{
   //{VAConfigAttribRTFormat, VA_RT_FORMAT_YUV444_10},
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 | VA_RT_FORMAT_RGB32_10},
   {VAConfigAttribMaxPictureWidth, CODEC_8K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_8K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_RAW_DATA},
   //just to match with legacy correct value 2
   {VAConfigAttribEncMaxRefFrames, 3},
#if VA_CHECK_VERSION(1, 23, 0)
   {VAConfigAttribEncVP9, VP9Common_VAEntrypointEncSlice_encVP9.value},
#endif
   {VAConfigAttribRateControl, VA_RC_CQP | VA_RC_ICQ | VA_RC_CBR | VA_RC_VBR | VA_RC_MB},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_ENCODE},
   {VAConfigAttribEncTileSupport, 1},
   {VAConfigAttribEncDirtyRect, 4},
   {VAConfigAttribEncDynamicScaling, 1},
   {VAConfigAttribEncQualityRange, NUM_TARGET_USAGE_MODES - 1},
   {VAConfigAttribEncInterlaced, VA_ENC_INTERLACED_NONE},
   {VAConfigAttribEncQuantization, VA_ENC_QUANTIZATION_NONE},
   {VAConfigAttribEncIntraRefresh, VA_ENC_INTRA_REFRESH_NONE},
   {VAConfigAttribEncSkipFrame, 0},
   {VAConfigAttribEncROI, VP9Common_VAEntrypointEncSlice_encROI.value},
   {VAConfigAttribEncParallelRateControl, 0},
   {VAConfigAttribFEIMVPredictors, 0},
   {VAConfigAttribEncRateControlExt, VP9Common_VAEntrypointEncSlice_encRateControlExt.value},
};

//!
static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileVP9Profile0_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{ 
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_NV12}}}, 
  {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {8192}}}, 
  {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {8192}}}, 
  {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {128}}}, 
  {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {96}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE,
    {VAGenericValueTypeInteger, {
#if VA_CHECK_VERSION(1, 21, 0)
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 |
#endif
        VA_SURFACE_ATTRIB_MEM_TYPE_VA | 
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2
    }}}
}; 

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileVP9Profile1_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{ 
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_AYUV}}}, 
#if VA_CHECK_VERSION(1, 13, 0)
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_XYUV}}}, 
#endif
  {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {8192}}}, 
  {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {8192}}}, 
  {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {128}}}, 
  {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {96}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE,
    {VAGenericValueTypeInteger, {
#if VA_CHECK_VERSION(1, 21, 0)
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 |
#endif
        VA_SURFACE_ATTRIB_MEM_TYPE_VA | 
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2
    }}}
}; 

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileVP9Profile2_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{ 
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_P010}}}, 
  {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {8192}}}, 
  {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {8192}}}, 
  {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {128}}}, 
  {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {96}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE,
    {VAGenericValueTypeInteger, {
#if VA_CHECK_VERSION(1, 21, 0)
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 |
#endif
        VA_SURFACE_ATTRIB_MEM_TYPE_VA | 
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2
    }}}
}; 

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileVP9Profile3_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{ 
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y410}}}, 
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_ARGB}}}, 
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_ABGR}}}, 
  {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {8192}}}, 
  {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {8192}}}, 
  {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {128}}}, 
  {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {96}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE,
    {VAGenericValueTypeInteger, {
#if VA_CHECK_VERSION(1, 21, 0)
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 |
#endif
        VA_SURFACE_ATTRIB_MEM_TYPE_VA | 
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2
    }}}
}; 

//!
//! \brief  Definition for EmtrypointMap
//!
static const EntrypointData entrypointMap_VAProfileVP9Profile0_Data_Xe_Lpm_plus_r0
{
    &attribList_VAProfileVP9Profile0_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &configDataList_VAProfileVP9Profile0_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &surfaceAttribInfo_VAProfileVP9Profile0_VAEntrypointEncSlice_Xe_Lpm_plus_r0
};

static const EntrypointData entrypointMap_VAProfileVP9Profile1_Data_Xe_Lpm_plus_r0
{
    &attribList_VAProfileVP9Profile1_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &configDataList_VAProfileVP9Profile1_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &surfaceAttribInfo_VAProfileVP9Profile1_VAEntrypointEncSlice_Xe_Lpm_plus_r0
};

static const EntrypointData entrypointMap_VAProfileVP9Profile2_Data_Xe_Lpm_plus_r0
{
    &attribList_VAProfileVP9Profile2_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &configDataList_VAProfileVP9Profile2_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &surfaceAttribInfo_VAProfileVP9Profile2_VAEntrypointEncSlice_Xe_Lpm_plus_r0
};

static const EntrypointData entrypointMap_VAProfileVP9Profile3_Data_Xe_Lpm_plus_r0
{
    &attribList_VAProfileVP9Profile3_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &configDataList_VAProfileVP9Profile3_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &surfaceAttribInfo_VAProfileVP9Profile3_VAEntrypointEncSlice_Xe_Lpm_plus_r0
};

#endif
