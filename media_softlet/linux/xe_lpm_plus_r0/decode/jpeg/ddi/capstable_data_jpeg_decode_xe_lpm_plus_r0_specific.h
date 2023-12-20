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
//! \file     capstable_data_jpeg_decode_xe_lpm_plus_r0_specific.h
//! \brief    This file register all caps data
//!

#ifndef __CAPSTABLE_DATA_JPEG_DECODE_XE_LPM_PLUS_R0_SPECIFIC_H__
#define __CAPSTABLE_DATA_JPEG_DECODE_XE_LPM_PLUS_R0_SPECIFIC_H__

#include "capstable_data_xe_lpm_plus_r0_specific.h"

#ifndef VA_ENCRYPTION_TYPE_NONE
#define VA_ENCRYPTION_TYPE_NONE 0x00000000
#endif

//!
//! \brief  Definition for ConfigDataList
//!
static ConfigDataList configDataList_VAProfileJPEGBaseline_VAEntrypointVLD_Xe_Lpm_plus_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE}
};

//!
//! \brief  Definition for AttribList
//!
static const AttribList attribList_VAProfileJPEGBaseline_VAEntrypointVLD_Xe_Lpm_plus_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV400 | VA_RT_FORMAT_YUV411 | VA_RT_FORMAT_RGB16 | VA_RT_FORMAT_RGB32},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_16K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribDecJPEG, ((1 << VA_ROTATION_NONE) | (1 << VA_ROTATION_90) | (1 << VA_ROTATION_180) | (1 << VA_ROTATION_270))},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

//!
//! \brief   Definition for ProfileSurfaceAttribInfo
//!
static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileJPEGBaseline_VAEntrypointVLD_Xe_Lpm_plus_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_NV12}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_IMC3}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y800}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_411P}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_422H}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_422V}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_444P}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_RGBP}}},
  {VASurfaceAttribMaxWidth,    VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight,   VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_HEIGHT}}},
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
//! \brief  Definition for EntrypointMap
//!
static const EntrypointData entrypointMap_VAProfileJPEGBaselineDec_Data_Xe_Lpm_plus_r0
{
    &attribList_VAProfileJPEGBaseline_VAEntrypointVLD_Xe_Lpm_plus_r0,
    &configDataList_VAProfileJPEGBaseline_VAEntrypointVLD_Xe_Lpm_plus_r0,
    &surfaceAttribInfo_VAProfileJPEGBaseline_VAEntrypointVLD_Xe_Lpm_plus_r0
};

#endif
