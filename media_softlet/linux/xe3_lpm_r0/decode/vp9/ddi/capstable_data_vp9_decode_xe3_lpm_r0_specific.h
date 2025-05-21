/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     capstable_data_vp9_decode_xe3_lpm_r0_specific.h
//! \brief    This file register all caps data
//!

#ifndef __CAPSTABLE_DATA_VP9_DECODE_XE3_LPM_R0_SPECIFIC_H__
#define __CAPSTABLE_DATA_VP9_DECODE_XE3_LPM_R0_SPECIFIC_H__

#include "capstable_data_xe3_lpm_r0_specific.h"
#include "codec_def_common.h"

#ifndef VA_ENCRYPTION_TYPE_NONE
#define VA_ENCRYPTION_TYPE_NONE 0x00000000
#endif

//!
//! \brief  Definition for ConfigDataList
//!
static ConfigDataList configDataList_VAProfileVP9Profile0_VAEntrypointVLD_Xe3_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING},
  {VA_DEC_SLICE_MODE_BASE, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_BASE, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING}
};

static ConfigDataList configDataList_VAProfileVP9Profile2_VAEntrypointVLD_Xe3_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING},
  {VA_DEC_SLICE_MODE_BASE, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_BASE, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING}
};

static ConfigDataList configDataList_VAProfileVP9Profile1_VAEntrypointVLD_Xe3_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING},
  {VA_DEC_SLICE_MODE_BASE, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_BASE, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING}
};

static ConfigDataList configDataList_VAProfileVP9Profile3_VAEntrypointVLD_Xe3_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING},
  {VA_DEC_SLICE_MODE_BASE, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_BASE, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING}
};

//!
//! \brief  Definition for AttribList
//!
static const AttribList attribList_VAProfileVP9Profile0_VAEntrypointVLD_Xe3_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL | VA_DEC_SLICE_MODE_BASE},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_16K_MAX_PIC_HEIGHT},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

static const AttribList attribList_VAProfileVP9Profile1_VAEntrypointVLD_Xe3_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV444},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL | VA_DEC_SLICE_MODE_BASE},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_16K_MAX_PIC_HEIGHT},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

static const AttribList attribList_VAProfileVP9Profile2_VAEntrypointVLD_Xe3_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV420_12},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL | VA_DEC_SLICE_MODE_BASE},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_16K_MAX_PIC_HEIGHT},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

static const AttribList attribList_VAProfileVP9Profile3_VAEntrypointVLD_Xe3_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV422_10 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_YUV422_12 | VA_RT_FORMAT_YUV444_12},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL | VA_DEC_SLICE_MODE_BASE},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_16K_MAX_PIC_HEIGHT},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

//!
//! \brief   Definition for ProfileSurfaceAttribInfo
//!
static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileVP9Profile2_VAEntrypointVLD_Xe3_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_P010}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, { VA_FOURCC_P012}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_P016}}},
  {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileVP9Profile1_VAEntrypointVLD_Xe3_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_AYUV}}},
#if VA_CHECK_VERSION(1, 13, 0)
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_XYUV}}},
#endif
  {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileVP9Profile3_VAEntrypointVLD_Xe3_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y410}}},
#if VA_CHECK_VERSION(1, 9, 0)
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y412}}},
#endif
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y416}}},
  {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileVP9Profile0_VAEntrypointVLD_Xe3_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_NV12}}},
  {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

//!
//! \brief  Definition for EntrypointMap
//!
static const EntrypointData entrypointMap_VAProfileVP9Profile0Dec_Data_Xe3_Lpm_r0
{
    &attribList_VAProfileVP9Profile0_VAEntrypointVLD_Xe3_Lpm_r0,
    &configDataList_VAProfileVP9Profile0_VAEntrypointVLD_Xe3_Lpm_r0,
    &surfaceAttribInfo_VAProfileVP9Profile0_VAEntrypointVLD_Xe3_Lpm_r0
};

static const EntrypointData entrypointMap_VAProfileVP9Profile1Dec_Data_Xe3_Lpm_r0
{
    &attribList_VAProfileVP9Profile1_VAEntrypointVLD_Xe3_Lpm_r0,
    &configDataList_VAProfileVP9Profile1_VAEntrypointVLD_Xe3_Lpm_r0,
    &surfaceAttribInfo_VAProfileVP9Profile1_VAEntrypointVLD_Xe3_Lpm_r0
};

static const EntrypointData entrypointMap_VAProfileVP9Profile2Dec_Data_Xe3_Lpm_r0
{
    &attribList_VAProfileVP9Profile2_VAEntrypointVLD_Xe3_Lpm_r0,
    &configDataList_VAProfileVP9Profile2_VAEntrypointVLD_Xe3_Lpm_r0,
    &surfaceAttribInfo_VAProfileVP9Profile2_VAEntrypointVLD_Xe3_Lpm_r0
};

static const EntrypointData entrypointMap_VAProfileVP9Profile3Dec_Data_Xe3_Lpm_r0
{
    &attribList_VAProfileVP9Profile3_VAEntrypointVLD_Xe3_Lpm_r0,
    &configDataList_VAProfileVP9Profile3_VAEntrypointVLD_Xe3_Lpm_r0,
    &surfaceAttribInfo_VAProfileVP9Profile3_VAEntrypointVLD_Xe3_Lpm_r0
};

#endif
