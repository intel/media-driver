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
//! \file     capstable_data_vvc_decode_xe3_lpm_r0_specific.h
//! \brief    This file register all caps data
//!

#ifndef __CAPSTABLE_DATA_VVC_DECODE_XE3_LPM_R0_SPECIFIC_H__
#define __CAPSTABLE_DATA_VVC_DECODE_XE3_LPM_R0_SPECIFIC_H__

#include <va/va.h>
#include <va/va_dec_vvc.h>
#include "capstable_data_xe3_lpm_r0_specific.h"
#include "codec_def_common.h"
#include "codec_def_decode_vvc.h"

#ifndef VA_CENC_TYPE_NONE
#define VA_CENC_TYPE_NONE 0x00000000
#endif

//!
//! \brief  Definition for ConfigDataList
//!
static ConfigDataList configDataList_VAProfileVVCMain10_VAEntrypointVLD_Xe3_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_CENC_TYPE_NONE, VA_DEC_PROCESSING_NONE},
};

//!
//! \brief  Definition for AttribList
//!
static const AttribList attribList_VAProfileVVCMain10_VAEntrypointVLD_Xe3_Lpm_r0
{
  {VAConfigAttribRTFormat,              VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10},
  {VAConfigAttribMaxPictureWidth,       CODEC_16K_VVC_MAX_PIC_WIDTH},
  {VAConfigAttribMaxPictureHeight,      CODEC_16K_VVC_MAX_PIC_HEIGHT},
  {VAConfigAttribDecSliceMode,          VA_DEC_SLICE_MODE_NORMAL},
  {VAConfigAttribDecProcessing,         VA_DEC_PROCESSING_NONE},
  {VAConfigAttribEncryption,            VA_ATTRIB_NOT_SUPPORTED},
  {VAConfigAttribProcessingRate,        VA_PROCESSING_RATE_DECODE},
  {VAConfigAttribCustomRoundingControl, 1},
};

//!
//! \brief   Definition for ProfileSurfaceAttribInfo
//!
static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileVVCMain10_VAEntrypointVLD_Xe3_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_NV12}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_P010}}},
  {VASurfaceAttribMaxWidth,    VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_VVC_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight,   VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_VVC_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

//!
//! \brief  Definition for EmtrypointMap
//!
static const EntrypointData entrypointMap_VAProfileVVCMain10Dec_Data_Xe3_Lpm_r0
{
  &attribList_VAProfileVVCMain10_VAEntrypointVLD_Xe3_Lpm_r0,
  &configDataList_VAProfileVVCMain10_VAEntrypointVLD_Xe3_Lpm_r0,
  &surfaceAttribInfo_VAProfileVVCMain10_VAEntrypointVLD_Xe3_Lpm_r0
};

//!
//! \brief  Definition for ConfigDataList
//!
static ConfigDataList configDataList_VAProfileVVCMultilayerMain10_VAEntrypointVLD_Xe3_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_CENC_TYPE_NONE, VA_DEC_PROCESSING_NONE},
};

//!
//! \brief  Definition for AttribList
//!
static const AttribList attribList_VAProfileVVCMultilayerMain10_VAEntrypointVLD_Xe3_Lpm_r0
{
  {VAConfigAttribRTFormat,              VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10},
  {VAConfigAttribMaxPictureWidth,       CODEC_16K_VVC_MAX_PIC_WIDTH},
  {VAConfigAttribMaxPictureHeight,      CODEC_16K_VVC_MAX_PIC_HEIGHT},
  {VAConfigAttribDecSliceMode,          VA_DEC_SLICE_MODE_NORMAL},
  {VAConfigAttribDecProcessing,         VA_DEC_PROCESSING_NONE},
  {VAConfigAttribEncryption,            VA_ATTRIB_NOT_SUPPORTED},
  {VAConfigAttribProcessingRate,        VA_PROCESSING_RATE_DECODE},
  {VAConfigAttribCustomRoundingControl, 1},
};

//!
//! \brief   Definition for ProfileSurfaceAttribInfo
//!
static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileVVCMultilayerMain10_VAEntrypointVLD_Xe3_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_NV12}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_P010}}},
  {VASurfaceAttribMaxWidth,    VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_VVC_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight,   VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_VVC_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

//!
//! \brief  Definition for EmtrypointMap
//!
static const EntrypointData entrypointMap_VAProfileVVCMultilayerMain10Dec_Data_Xe3_Lpm_r0
{
  &attribList_VAProfileVVCMultilayerMain10_VAEntrypointVLD_Xe3_Lpm_r0,
  &configDataList_VAProfileVVCMultilayerMain10_VAEntrypointVLD_Xe3_Lpm_r0,
  &surfaceAttribInfo_VAProfileVVCMultilayerMain10_VAEntrypointVLD_Xe3_Lpm_r0
};

#endif
