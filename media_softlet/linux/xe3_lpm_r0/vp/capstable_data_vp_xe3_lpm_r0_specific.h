/*
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
*/
//!
//! \file     capstable_data_vp_xe3_lpm_r0_specific.h
//! \brief    This file defines the data for vp xe3_lpm r0
//!

#ifndef __CAPSTABLE_DATA_VP_XE3_LPM_R0_SPECIFIC_H__
#define __CAPSTABLE_DATA_VP_XE3_LPM_R0_SPECIFIC_H__

#include "media_capstable_specific.h"

static AttribList VAConfigAttribList_VAProfileNone_VAEntrypointVideoProc_Xe3_lpm_r0 =
{
    {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 |
                             VA_RT_FORMAT_YUV422 |
                             VA_RT_FORMAT_YUV444 |
                             VA_RT_FORMAT_YUV400 |
                             VA_RT_FORMAT_YUV411 |
                             VA_RT_FORMAT_RGB16 |
                             VA_RT_FORMAT_RGB32 |
                             VA_RT_FORMAT_RGBP}
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAEntrypointVideoProc_VAProfileNone_Xe3_lpm_r0 =
{
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC('N', 'V', '1', '2')}}},
    {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {16384}}},
    {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {16384}}},
    {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {16}}},
    {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {16}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC('I', '4', '2', '0')}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC('Y', 'V', '1', '2')}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC('Y', 'U', 'Y', '2')}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC('4', '2', '2', 'H')}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC('4', '2', '2', 'V')}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC('R', 'G', 'B', 'A')}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC('B', 'G', 'R', 'A')}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC('R', 'G', 'B', 'P')}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC('R', 'G', 'B', 'X')}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC('P', '0', '1', '0')}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC('R', 'G', '2', '4')}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_ARGB}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_ABGR}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_A2R10G10B10}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_A2B10G10R10}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_X2R10G10B10}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_X2B10G10R10}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_AYUV}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y210}}},
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y410}}},
#if VA_CHECK_VERSION(1, 13, 0)
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_XYUV}}},
#else
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {0}}},
#endif
    {VASurfaceAttribMemoryType, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA |
                                                                                                                    VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR |
                                                                                                                    VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM |
                                                                                                                    VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME |
                                                                                                                    VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}},
    {VASurfaceAttribExternalBufferDescriptor, VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypePointer, {0}}}
};

//!
//! \brief  Definion for EntrypointMap
//!
static EntrypointData entrypointMap_VAProfileNone_Data_Xe3_lpm_r0
{
    &VAConfigAttribList_VAProfileNone_VAEntrypointVideoProc_Xe3_lpm_r0,
    nullptr,     // not needed for vp
    &surfaceAttribInfo_VAEntrypointVideoProc_VAProfileNone_Xe3_lpm_r0
};

static EntrypointMap entrypointMap_VAProfileNone_Xe3_lpm_r0
{
    {VAEntrypointVideoProc, &entrypointMap_VAProfileNone_Data_Xe3_lpm_r0},
};

#endif