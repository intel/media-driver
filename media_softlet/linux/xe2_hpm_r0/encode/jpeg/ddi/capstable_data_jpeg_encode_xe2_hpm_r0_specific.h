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
//! \file     capstable_data_jpeg_encode_xe2_hpm_r0_specific.h
//! \brief    This file register all caps data
//!

#ifndef __CAPSTABLE_DATA_JPEG_ENCODE_XE2_HPM_R0_SPECIFIC_H__
#define __CAPSTABLE_DATA_JPEG_ENCODE_XE2_HPM_R0_SPECIFIC_H__

#include "capstable_data_xe2_hpm_r0_specific.h"

//! \brief  Definition for ConfigDataList
static const ConfigDataList configDataList_VAProfileJPEGBaseline_VAEntrypointEncPicture_Xe2_Hpm_r0 =
{
    {VA_RC_NONE, 0},
};

//!
//! \brief  Definition for AttribList
//!
static const AttribList AttribList_VAProfileJPEGBaseline_VAEntrypointEncPicture_Xe2_Hpm_r0
{
    {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV400 | VA_RT_FORMAT_YUV411 | VA_RT_FORMAT_RGB16 | VA_RT_FORMAT_RGB32},
    {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_RAW_DATA},
    {VAConfigAttribMaxPictureWidth, ENCODE_JPEG_MAX_PIC_WIDTH},
    {VAConfigAttribMaxPictureHeight, ENCODE_JPEG_MAX_PIC_HEIGHT},
    {VAConfigAttribEncJPEG, 53424},
    {VAConfigAttribEncQualityRange, 1},
};

//!
static const ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileJPEGBaseline_VAEntrypointEncPicture_Xe2_Hpm_r0 =
{ 
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_NV12}}}, 
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_YUY2}}}, 
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_UYVY}}}, 
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y800}}}, 
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_ABGR}}}, 
    {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {ENCODE_JPEG_MAX_PIC_WIDTH}}},
    {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {ENCODE_JPEG_MAX_PIC_HEIGHT}}},
    {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {16}}},
    {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {16}}},
    {VASurfaceAttribMemoryType, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

//!
//! \brief  Definition for EmtrypointMap
//!
static const EntrypointData entrypointMap_VAProfileJPEGBaseline_Data_Xe2_Hpm_r0
{
    &AttribList_VAProfileJPEGBaseline_VAEntrypointEncPicture_Xe2_Hpm_r0,
    &configDataList_VAProfileJPEGBaseline_VAEntrypointEncPicture_Xe2_Hpm_r0,
    &surfaceAttribInfo_VAProfileJPEGBaseline_VAEntrypointEncPicture_Xe2_Hpm_r0
};

#endif
