/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     capstable_data_image_format_definition.h
//! \brief    This file defines all image format definition
//!

#ifndef __CAPSTABLE_DATA_IMAGE_FORMAT_DEFINITION_H__
#define __CAPSTABLE_DATA_IMAGE_FORMAT_DEFINITION_H__

#include "va/va.h"

// "VA_LSB_FIRST" is to identify how following bit masks mapped to address instead of char order in VA_FOURCC_RGBA naming.
static VAImageFormat formatBGRA        = {VA_FOURCC_BGRA,        VA_LSB_FIRST,  32, 32, 0x00ff0000, 0x0000ff00, 0x000000ff,  0xff000000}; /* [31:0] A:R:G:B 8:8:8:8 little endian */
static VAImageFormat formatARGB        = {VA_FOURCC_ARGB,        VA_LSB_FIRST,  32, 32, 0x00ff0000, 0x0000ff00, 0x000000ff,  0xff000000 }; /* [31:0] A:R:G:B 8:8:8:8 little endian */
static VAImageFormat formatRGBA        = {VA_FOURCC_RGBA,        VA_LSB_FIRST,  32, 32, 0x000000ff, 0x0000ff00, 0x00ff0000,  0xff000000}; /* [31:0] A:B:G:R 8:8:8:8 little endian */
static VAImageFormat formatABGR        = {VA_FOURCC_ABGR,        VA_LSB_FIRST,  32, 32, 0x000000ff, 0x0000ff00, 0x00ff0000,  0xff000000 }; /* [31:0] A:B:G:R 8:8:8:8 little endian */
static VAImageFormat formatBGRX        = {VA_FOURCC_BGRX,        VA_LSB_FIRST,  32, 24, 0x00ff0000, 0x0000ff00, 0x000000ff,  0};          /* [31:0] X:R:G:B 8:8:8:8 little endian */
static VAImageFormat formatXRGB        = {VA_FOURCC_XRGB,        VA_LSB_FIRST,  32, 24, 0x00ff0000, 0x0000ff00, 0x000000ff,  0 };          /* [31:0] x:R:G:B 8:8:8:8 little endian */
static VAImageFormat formatRGBX        = {VA_FOURCC_RGBX,        VA_LSB_FIRST,  32, 24, 0x000000ff, 0x0000ff00, 0x00ff0000,  0};          /* [31:0] X:B:G:R 8:8:8:8 little endian */
static VAImageFormat formatXBGR        = {VA_FOURCC_XBGR,        VA_LSB_FIRST,  32, 24, 0x000000ff, 0x0000ff00, 0x00ff0000,  0 };          /* [31:0] x:B:G:R 8:8:8:8 little endian */
static VAImageFormat formatA2R10G10B10 = {VA_FOURCC_A2R10G10B10, VA_LSB_FIRST,  32, 30, 0x3ff00000, 0x000ffc00, 0x000003ff,  0x30000000 }; /* [31:0] A:R:G:B 2:10:10:10 little endian */
static VAImageFormat formatA2B10G10R10 = {VA_FOURCC_A2B10G10R10, VA_LSB_FIRST,  32, 30, 0x000003ff, 0x000ffc00, 0x3ff00000,  0x30000000 }; /* [31:0] A:B:G:R 2:10:10:10 little endian */
static VAImageFormat formatX2R10G10B10 = {VA_FOURCC_X2R10G10B10, VA_LSB_FIRST,  32, 30, 0x3ff00000, 0x000ffc00, 0x000003ff,  0 };          /* [31:0] X:R:G:B 2:10:10:10 little endian */
static VAImageFormat formatX2B10G10R10 = {VA_FOURCC_X2B10G10R10, VA_LSB_FIRST,  32, 30, 0x000003ff, 0x000ffc00, 0x3ff00000,  0 };          /* [31:0] X:B:G:R 2:10:10:10 little endian */
static VAImageFormat formatRGB565      = {VA_FOURCC_RGB565,      VA_LSB_FIRST,  16, 16, 0xf800,     0x07e0,     0x001f,      0 };          /* [15:0] R:G:B 5:6:5 little endian */
static VAImageFormat formatAYUV        = {VA_FOURCC_AYUV,        VA_LSB_FIRST,  32, 0,0,0,0,0 };
#if VA_CHECK_VERSION(1, 13, 0)
static VAImageFormat formatXYUV        = {VA_FOURCC_XYUV,        VA_LSB_FIRST,  32, 0,0,0,0,0 };
#endif
static VAImageFormat formatY800        = {VA_FOURCC_Y800,        VA_LSB_FIRST,  8,  0,0,0,0,0 };
static VAImageFormat formatNV12        = {VA_FOURCC_NV12,        VA_LSB_FIRST,  12, 0,0,0,0,0 };
static VAImageFormat formatNV21        = {VA_FOURCC_NV21,        VA_LSB_FIRST,  12, 0,0,0,0,0 };
static VAImageFormat formatYUY2        = {VA_FOURCC_YUY2,        VA_LSB_FIRST,  16, 0,0,0,0,0 };
static VAImageFormat formatUYVY        = {VA_FOURCC_UYVY,        VA_LSB_FIRST,  16, 0,0,0,0,0 };
static VAImageFormat formatYV12        = {VA_FOURCC_YV12,        VA_LSB_FIRST,  12, 0,0,0,0,0 };
static VAImageFormat formatI420        = {VA_FOURCC_I420,        VA_LSB_FIRST,  12, 0,0,0,0,0 };
static VAImageFormat format411P        = {VA_FOURCC_411P,        VA_LSB_FIRST,  12, 0,0,0,0,0 };
static VAImageFormat format422H        = {VA_FOURCC_422H,        VA_LSB_FIRST,  16, 0,0,0,0,0 };
static VAImageFormat format422V        = {VA_FOURCC_422V,        VA_LSB_FIRST,  16, 0,0,0,0,0 };
static VAImageFormat format444P        = {VA_FOURCC_444P,        VA_LSB_FIRST,  24, 0,0,0,0,0 };
static VAImageFormat formatIMC3        = {VA_FOURCC_IMC3,        VA_LSB_FIRST,  16, 0,0,0,0,0 };
static VAImageFormat formatP010        = {VA_FOURCC_P010,        VA_LSB_FIRST,  24, 0,0,0,0,0 };
static VAImageFormat formatP012        = {VA_FOURCC_P012,        VA_LSB_FIRST,  24, 0,0,0,0,0 };
static VAImageFormat formatP016        = {VA_FOURCC_P016,        VA_LSB_FIRST,  24, 0,0,0,0,0 };
static VAImageFormat formatY210        = {VA_FOURCC_Y210,        VA_LSB_FIRST,  32, 0,0,0,0,0 };
static VAImageFormat formatY212        = {VA_FOURCC_Y212,        VA_LSB_FIRST,  32, 0,0,0,0,0 };
static VAImageFormat formatY216        = {VA_FOURCC_Y216,        VA_LSB_FIRST,  32, 0,0,0,0,0 };
static VAImageFormat formatY410        = {VA_FOURCC_Y410,        VA_LSB_FIRST,  32, 0,0,0,0,0 };
static VAImageFormat formatY412        = {VA_FOURCC_Y412,        VA_LSB_FIRST,  64, 0,0,0,0,0 };
static VAImageFormat formatY416        = {VA_FOURCC_Y416,        VA_LSB_FIRST,  64, 0,0,0,0,0 };
static VAImageFormat formatRGBP        = {VA_FOURCC_RGBP,        VA_LSB_FIRST,  24, 24,0,0,0,0};
static VAImageFormat formatBGRP        = {VA_FOURCC_BGRP,        VA_LSB_FIRST,  24, 24,0,0,0,0};

#endif //__CAPSTABLE_DATA_IMAGE_FORMAT_DEFINITION_H__
