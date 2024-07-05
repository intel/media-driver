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
//! \file     capstable_data_mpeg2_decode_xe2_lpm_r0_specific.h
//! \brief    This file register all caps data
//!

#ifndef __CAPSTABLE_DATA_MPEG2_DECODE_XE2_LPM_R0_SPECIFIC_H__
#define __CAPSTABLE_DATA_MPEG2_DECODE_XE2_LPM_R0_SPECIFIC_H__

#include "capstable_data_xe2_lpm_r0_specific.h"
#include "codec_def_common.h"

#ifndef VA_ENCRYPTION_TYPE_NONE
#define VA_ENCRYPTION_TYPE_NONE 0x00000000
#endif

//!
//! \brief  Definition for bitset value
//!
//! \brief  Definition for ConfigDataList
static ConfigDataList configDataList_VAProfileMPEG2Simple_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE}
};

static ConfigDataList configDataList_VAProfileMPEG2Main_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE}
};

//!
//! \brief  Definition for AttribList
//!
static const AttribList attribList_VAProfileMPEG2Simple_VAEntrypointVLD_Xe2_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_RGB32},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING_NONE},
   {VAConfigAttribMaxPictureWidth, CODEC_2K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_2K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

static const AttribList attribList_VAProfileMPEG2Main_VAEntrypointVLD_Xe2_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_RGB32},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING_NONE},
   {VAConfigAttribMaxPictureWidth, CODEC_2K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_2K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

//!
//! \brief   Definition for ProfileSurfaceAttribInfo
//!
static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileMPEG2Simple_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_NV12}}},
  {VASurfaceAttribMaxWidth,    VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_2K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight,   VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_2K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileMPEG2Main_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_NV12}}},
  {VASurfaceAttribMaxWidth,    VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_2K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight,   VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_2K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

//!
//! \brief  Definition for EntrypointMap
//!
static const EntrypointData entrypointMap_VAProfileMPEG2SimpleDec_Data_Xe2_Lpm_r0
{
    &attribList_VAProfileMPEG2Simple_VAEntrypointVLD_Xe2_Lpm_r0,
    &configDataList_VAProfileMPEG2Simple_VAEntrypointVLD_Xe2_Lpm_r0,
    &surfaceAttribInfo_VAProfileMPEG2Simple_VAEntrypointVLD_Xe2_Lpm_r0
};

static const EntrypointData entrypointMap_VAProfileMPEG2MainDec_Data_Xe2_Lpm_r0
{
    &attribList_VAProfileMPEG2Main_VAEntrypointVLD_Xe2_Lpm_r0,
    &configDataList_VAProfileMPEG2Main_VAEntrypointVLD_Xe2_Lpm_r0,
    &surfaceAttribInfo_VAProfileMPEG2Main_VAEntrypointVLD_Xe2_Lpm_r0
};

#endif
