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
//! \file     capstable_data_hevc_vdenc_m12_0_r0_specific.h
//! \brief    This file defines the data for hevc vdenc m12.0 r0
//!

#ifndef __CAPSTABLE_DATA_M12_0_R0_SPECIFIC_H__
#define __CAPSTABLE_DATA_M12_0_R0_SPECIFIC_H__

#include "capstable_data_hevc_vdenc_m12_0_r0_specific.h"
#include "capstable_data_image_format_definition.h"

#define IP_VERSION_M12_0 0x1200
const PlatformInfo plt_M12_0_r0 = {IP_VERSION_M12_0, 0};

static std::map<uint32_t, VAImageFormat*> imgtbl_M12_0_r0
{
    {VA_FOURCC_BGRA,         &formatBGRA},
    {VA_FOURCC_ARGB,         &formatARGB},
    {VA_FOURCC_RGBA,         &formatRGBA},
    {VA_FOURCC_ABGR,         &formatABGR},
    {VA_FOURCC_BGRX,         &formatBGRX},
    {VA_FOURCC_XRGB,         &formatXRGB},
    {VA_FOURCC_RGBX,         &formatRGBX},
    {VA_FOURCC_XBGR,         &formatXBGR},
    {VA_FOURCC_A2R10G10B10,  &formatA2R10G10B10},
    {VA_FOURCC_A2B10G10R10,  &formatA2B10G10R10},
    {VA_FOURCC_X2R10G10B10,  &formatX2R10G10B10},
    {VA_FOURCC_X2B10G10R10,  &formatX2B10G10R10},
    {VA_FOURCC_RGB565,       &formatRGB565},
    {VA_FOURCC_AYUV,         &formatAYUV},
#if VA_CHECK_VERSION(1, 13, 0)
    {VA_FOURCC_XYUV,         &formatXYUV},
#endif
    {VA_FOURCC_Y800,         &formatY800},
    {VA_FOURCC_NV12,         &formatNV12},
    {VA_FOURCC_NV21,         &formatNV21},
    {VA_FOURCC_YUY2,         &formatYUY2},
    {VA_FOURCC_UYVY,         &formatUYVY},
    {VA_FOURCC_YV12,         &formatYV12},
    {VA_FOURCC_I420,         &formatI420},
    {VA_FOURCC_411P,         &format411P},
    {VA_FOURCC_422H,         &format422H},
    {VA_FOURCC_422V,         &format422V},
    {VA_FOURCC_444P,         &format444P},
    {VA_FOURCC_IMC3,         &formatIMC3},
    {VA_FOURCC_P010,         &formatP010},
    {VA_FOURCC_P012,         &formatP012},
    {VA_FOURCC_P016,         &formatP016},
    {VA_FOURCC_Y210,         &formatY210},
    {VA_FOURCC_Y212,         &formatY212},
    {VA_FOURCC_Y216,         &formatY216},
    {VA_FOURCC_Y410,         &formatY410},
    {VA_FOURCC_Y412,         &formatY412},
    {VA_FOURCC_Y416,         &formatY416},
    {VA_FOURCC_RGBP,         &formatRGBP},
    {VA_FOURCC_BGRP,         &formatBGRP},
};

static ProfileMap profileMap_M12_0_r0
{
    {VAProfileHEVCMain, &entrypointMap_VAProfileHEVCMain_M12_0_r0},
};

static CapsData capsDataM12_0_r0
{
    &profileMap_M12_0_r0,
    &imgtbl_M12_0_r0
};

#endif //__CAPSTABLE_DATA_M12_0_R0_SPECIFIC_H__