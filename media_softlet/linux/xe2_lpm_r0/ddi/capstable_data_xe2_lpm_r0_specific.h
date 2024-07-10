/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     capstable_data_xe2_lpm_r0_specific.h
//! \brief    This file defines the data for xe2_lpm r0
//!

#ifndef __CAPSTABLE_DATA_XE2_LPM_R0_SPECIFIC_H__
#define __CAPSTABLE_DATA_XE2_LPM_R0_SPECIFIC_H__

#include "capstable_data_vp_xe2_lpm_r0_specific.h"
#if defined(_HEVC_ENCODE_VDENC_SUPPORTED)
#include "capstable_data_hevc_encode_xe2_lpm_r0_specific.h"
#endif
#if defined(_AVC_ENCODE_VDENC_SUPPORTED)
#include "capstable_data_avc_encode_xe2_lpm_r0_specific.h"
#endif
#if defined(_JPEG_ENCODE_SUPPORTED)
#include "capstable_data_jpeg_encode_xe2_lpm_r0_specific.h"
#endif
#if defined(_AV1_ENCODE_VDENC_SUPPORTED)
#include "capstable_data_av1_encode_xe2_lpm_r0_specific.h"
#endif
#if defined(_VP9_ENCODE_VDENC_SUPPORTED)
#include "capstable_data_vp9_encode_xe2_lpm_r0_specific.h"
#endif

#if defined(_HEVC_DECODE_SUPPORTED)
#include "capstable_data_hevc_decode_xe2_lpm_r0_specific.h"
#endif
#if defined(_AVC_DECODE_SUPPORTED)
#include "capstable_data_avc_decode_xe2_lpm_r0_specific.h"
#endif
#if defined(_AV1_DECODE_SUPPORTED)
#include "capstable_data_av1_decode_xe2_lpm_r0_specific.h"
#endif
#if defined(_JPEG_DECODE_SUPPORTED)
#include "capstable_data_jpeg_decode_xe2_lpm_r0_specific.h"
#endif
#if defined(_MPEG2_DECODE_SUPPORTED)
#include "capstable_data_mpeg2_decode_xe2_lpm_r0_specific.h"
#endif
#if defined(_VP8_DECODE_SUPPORTED)
#include "capstable_data_vp8_decode_xe2_lpm_r0_specific.h"
#endif
#if defined(_VP9_DECODE_SUPPORTED)
#include "capstable_data_vp9_decode_xe2_lpm_r0_specific.h"
#endif
#if defined(_VVC_DECODE_SUPPORTED)
#include "capstable_data_vvc_decode_xe2_lpm_r0_specific.h"
#endif

#include "capstable_data_image_format_definition.h"

#define IP_VERSION_XE2_LPM 0x1500
const PlatformInfo plt_Xe2_Lpm_r0 = {IP_VERSION_XE2_LPM, 0};

static const std::map<const uint32_t, const VAImageFormat*> imgtbl_Xe2_lpm_r0
{
    {VA_FOURCC_BGRA, &formatBGRA},
    {VA_FOURCC_RGBA, &formatRGBA},
    {VA_FOURCC_BGRX, &formatBGRX},
    {VA_FOURCC_RGBX, &formatRGBX},
    {VA_FOURCC_A2R10G10B10, &formatA2R10G10B10},
    {VA_FOURCC_A2B10G10R10, &formatA2B10G10R10},
    {VA_FOURCC_X2R10G10B10, &formatX2R10G10B10},
    {VA_FOURCC_X2B10G10R10, &formatX2B10G10R10},
    {VA_FOURCC_RGB565, &formatRGB565},
    {VA_FOURCC_AYUV, &formatAYUV},
    {VA_FOURCC_Y800, &formatY800},
    {VA_FOURCC_NV12, &formatNV12},
    {VA_FOURCC_NV21, &formatNV21},
    {VA_FOURCC_YUY2, &formatYUY2},
    {VA_FOURCC_UYVY, &formatUYVY},
    {VA_FOURCC_YV12, &formatYV12},
    {VA_FOURCC_I420, &formatI420},
    {VA_FOURCC_411P, &format411P},
    {VA_FOURCC_422H, &format422H},
    {VA_FOURCC_422V, &format422V},
    {VA_FOURCC_444P, &format444P},
    {VA_FOURCC_IMC3, &formatIMC3},
    {VA_FOURCC_P010, &formatP010},
    {VA_FOURCC_P012, &formatP012},
    {VA_FOURCC_P016, &formatP016},
    {VA_FOURCC_Y210, &formatY210},
#if VA_CHECK_VERSION(1, 9, 0)
    {VA_FOURCC_Y212, &formatY212},
#endif
    {VA_FOURCC_Y216, &formatY216},
    {VA_FOURCC_Y410, &formatY410},
#if VA_CHECK_VERSION(1, 9, 0)
    {VA_FOURCC_Y412, &formatY412},
#endif
    {VA_FOURCC_Y416, &formatY416},
    {VA_FOURCC_RGBP, &formatRGBP},
    {VA_FOURCC_BGRP, &formatBGRP},
#if VA_CHECK_VERSION(1, 13, 0)
    {VA_FOURCC_XYUV, &formatXYUV},
#endif
};

static const EntrypointMap entrypointMap_VAProfileHEVCSccMain444_10_Xe2_Lpm_r0
{
#if defined(_HEVC_ENCODE_VDENC_SUPPORTED)
    {VAEntrypointEncSlice, &entrypointMap_VAProfileHEVCSccMain444_10_Data_Xe2_Lpm_r0},
#endif

#if defined(_HEVC_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileHEVCSccMain444_10Dec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileHEVCSccMain444_Xe2_Lpm_r0
{
#if defined(_HEVC_ENCODE_VDENC_SUPPORTED)
    {VAEntrypointEncSlice, &entrypointMap_VAProfileHEVCSccMain444_Data_Xe2_Lpm_r0},
#endif

#if defined(_HEVC_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileHEVCSccMain444Dec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileHEVCSccMain10_Xe2_Lpm_r0
{
#if defined(_HEVC_ENCODE_VDENC_SUPPORTED)
    {VAEntrypointEncSlice, &entrypointMap_VAProfileHEVCSccMain10_Data_Xe2_Lpm_r0},
#endif

#if defined(_HEVC_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileHEVCSccMain10Dec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileHEVCSccMain_Xe2_Lpm_r0
{
#if defined(_HEVC_ENCODE_VDENC_SUPPORTED)
    {VAEntrypointEncSlice, &entrypointMap_VAProfileHEVCSccMain_Data_Xe2_Lpm_r0},
#endif

#if defined(_HEVC_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileHEVCSccMainDec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileHEVCMain444_10_Xe2_Lpm_r0
{
#if defined(_HEVC_ENCODE_VDENC_SUPPORTED)
    {VAEntrypointEncSlice, &entrypointMap_VAProfileHEVCMain444_10_Data_Xe2_Lpm_r0},
#endif

#if defined(_HEVC_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileHEVCMain444_10Dec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileHEVCMain444_Xe2_Lpm_r0
{
#if defined(_HEVC_ENCODE_VDENC_SUPPORTED)
    {VAEntrypointEncSlice, &entrypointMap_VAProfileHEVCMain444_Data_Xe2_Lpm_r0},
#endif

#if defined(_HEVC_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileHEVCMain444Dec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileHEVCMain10_Xe2_Lpm_r0
{
#if defined(_HEVC_ENCODE_VDENC_SUPPORTED)
    {VAEntrypointEncSlice, &entrypointMap_VAProfileHEVCMain10_Data_Xe2_Lpm_r0},
#endif

#if defined(_HEVC_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileHEVCMain10Dec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileHEVCMain_Xe2_Lpm_r0
{
#if defined(_HEVC_ENCODE_VDENC_SUPPORTED)
    {VAEntrypointEncSlice, &entrypointMap_VAProfileHEVCMain_Data_Xe2_Lpm_r0},
#endif

#if defined(_HEVC_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileHEVCMainDec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileHEVCMain12_Xe2_Lpm_r0
{
#if defined(_HEVC_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileHEVCMain12Dec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileHEVCMain422_10_Xe2_Lpm_r0
{
#if defined(_HEVC_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileHEVCMain422_10Dec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileHEVCMain422_12_Xe2_Lpm_r0
{
#if defined(_HEVC_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileHEVCMain422_12Dec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileHEVCMain444_12_Xe2_Lpm_r0
{
#if defined(_HEVC_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileHEVCMain444_12Dec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileH264Main_Xe2_Lpm_r0
{
#if defined(_AVC_ENCODE_VDENC_SUPPORTED)
    {VAEntrypointEncSlice, &entrypointMap_VAProfileH264Main_Data_Xe2_Lpm_r0},
#endif

#if defined(_AVC_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileH264MainDec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileH264High_Xe2_Lpm_r0
{
#if defined(_AVC_ENCODE_VDENC_SUPPORTED)
    {VAEntrypointEncSlice, &entrypointMap_VAProfileH264High_Data_Xe2_Lpm_r0},
#endif

#if defined(_AVC_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileH264HighDec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileH264ConstrainedBaseline_Xe2_Lpm_r0
{
#if defined(_AVC_ENCODE_VDENC_SUPPORTED)
    {VAEntrypointEncSlice, &entrypointMap_VAProfileH264ConstrainedBaseline_Data_Xe2_Lpm_r0},
#endif

#if defined(_AVC_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileH264ConstrainedBaselineDec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileJPEGBaseline_Xe2_Lpm_r0
{
#if defined(_JPEG_ENCODE_SUPPORTED)
    {VAEntrypointEncPicture, &entrypointMap_VAProfileJPEGBaseline_Data_Xe2_Lpm_r0},
#endif

#if defined(_JPEG_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileJPEGBaselineDec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileAV1Profile0_Xe2_Lpm_r0
{
#if defined(_AV1_ENCODE_VDENC_SUPPORTED)
    {VAEntrypointEncSlice, &entrypointMap_VAProfileAV1Profile0_Data_Xe2_Lpm_r0},
#endif

#if defined(_AV1_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileAV1Profile0Dec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileVP9Profile0_Xe2_Lpm_r0
{
#if defined(_VP9_ENCODE_VDENC_SUPPORTED)
    {VAEntrypointEncSlice, &entrypointMap_VAProfileVP9Profile0_Data_Xe2_Lpm_r0},
#endif

#if defined(_VP9_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileVP9Profile0Dec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileVP9Profile1_Xe2_Lpm_r0
{
#if defined(_VP9_ENCODE_VDENC_SUPPORTED)
    {VAEntrypointEncSlice, &entrypointMap_VAProfileVP9Profile1_Data_Xe2_Lpm_r0},
#endif

#if defined(_VP9_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileVP9Profile1Dec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileVP9Profile2_Xe2_Lpm_r0
{
#if defined(_VP9_ENCODE_VDENC_SUPPORTED)
    {VAEntrypointEncSlice, &entrypointMap_VAProfileVP9Profile2_Data_Xe2_Lpm_r0},
#endif

#if defined(_VP9_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileVP9Profile2Dec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileVP9Profile3_Xe2_Lpm_r0
{
#if defined(_VP9_ENCODE_VDENC_SUPPORTED)
    {VAEntrypointEncSlice, &entrypointMap_VAProfileVP9Profile3_Data_Xe2_Lpm_r0},
#endif

#if defined(_VP9_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileVP9Profile3Dec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileMPEG2Simple_Xe2_Lpm_r0
{
#if defined(_MPEG2_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileMPEG2SimpleDec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileMPEG2Main_Xe2_Lpm_r0
{
#if defined(_MPEG2_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileMPEG2MainDec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileVP8Version0_3_Xe2_Lpm_r0
{
#if defined(_VP8_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileVP8Version0_3Dec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileVVCMain10_Xe2_Lpm_r0
{
#if defined(_VVC_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileVVCMain10Dec_Data_Xe2_Lpm_r0},
#endif
};

static const EntrypointMap entrypointMap_VAProfileVVCMultilayerMain10_Xe2_Lpm_r0
{
#if defined(_VVC_DECODE_SUPPORTED)
    {VAEntrypointVLD, &entrypointMap_VAProfileVVCMultilayerMain10Dec_Data_Xe2_Lpm_r0},
#endif
};

//!
//! \brief  ProfileMap for Xe2_Lpm_r0
//!
//! \brief  Definion for ProfileMap
//!
static const ProfileMap profileMap_Xe2_Lpm_r0
{
    {VAProfileNone, &entrypointMap_VAProfileNone_Xe2_lpm_r0},
#if defined(_HEVC_ENCODE_VDENC_SUPPORTED) || defined(_HEVC_DECODE_SUPPORTED)
    {VAProfileHEVCSccMain444_10, &entrypointMap_VAProfileHEVCSccMain444_10_Xe2_Lpm_r0},
    {VAProfileHEVCSccMain444, &entrypointMap_VAProfileHEVCSccMain444_Xe2_Lpm_r0},
    {VAProfileHEVCSccMain10, &entrypointMap_VAProfileHEVCSccMain10_Xe2_Lpm_r0},
    {VAProfileHEVCSccMain, &entrypointMap_VAProfileHEVCSccMain_Xe2_Lpm_r0},
    {VAProfileHEVCMain444_10, &entrypointMap_VAProfileHEVCMain444_10_Xe2_Lpm_r0},
    {VAProfileHEVCMain444, &entrypointMap_VAProfileHEVCMain444_Xe2_Lpm_r0},
    {VAProfileHEVCMain10, &entrypointMap_VAProfileHEVCMain10_Xe2_Lpm_r0},
    {VAProfileHEVCMain, &entrypointMap_VAProfileHEVCMain_Xe2_Lpm_r0},
#endif

#if defined(_HEVC_DECODE_SUPPORTED)
    {VAProfileHEVCMain12, &entrypointMap_VAProfileHEVCMain12_Xe2_Lpm_r0},
    {VAProfileHEVCMain422_10, &entrypointMap_VAProfileHEVCMain422_10_Xe2_Lpm_r0},
    {VAProfileHEVCMain422_12, &entrypointMap_VAProfileHEVCMain422_12_Xe2_Lpm_r0},
    {VAProfileHEVCMain444_12, &entrypointMap_VAProfileHEVCMain444_12_Xe2_Lpm_r0},
#endif

#if defined(_AVC_ENCODE_VDENC_SUPPORTED) || defined(_AVC_DECODE_SUPPORTED)
    {VAProfileH264Main, &entrypointMap_VAProfileH264Main_Xe2_Lpm_r0},
    {VAProfileH264High, &entrypointMap_VAProfileH264High_Xe2_Lpm_r0},
    {VAProfileH264ConstrainedBaseline, &entrypointMap_VAProfileH264ConstrainedBaseline_Xe2_Lpm_r0},
#endif

#if defined(_JPEG_ENCODE_SUPPORTED) || defined(_JPEG_DECODE_SUPPORTED)
    {VAProfileJPEGBaseline, &entrypointMap_VAProfileJPEGBaseline_Xe2_Lpm_r0},
#endif

#if defined(_AV1_ENCODE_VDENC_SUPPORTED) || defined(_AV1_DECODE_SUPPORTED)
    {VAProfileAV1Profile0, &entrypointMap_VAProfileAV1Profile0_Xe2_Lpm_r0},
#endif

#if defined(_VP9_ENCODE_VDENC_SUPPORTED) || defined(_VP9_DECODE_SUPPORTED)
    {VAProfileVP9Profile3, &entrypointMap_VAProfileVP9Profile3_Xe2_Lpm_r0},
    {VAProfileVP9Profile2, &entrypointMap_VAProfileVP9Profile2_Xe2_Lpm_r0},
    {VAProfileVP9Profile1, &entrypointMap_VAProfileVP9Profile1_Xe2_Lpm_r0},
    {VAProfileVP9Profile0, &entrypointMap_VAProfileVP9Profile0_Xe2_Lpm_r0},
#endif

#if defined(_MPEG2_DECODE_SUPPORTED)
    {VAProfileMPEG2Simple, &entrypointMap_VAProfileMPEG2Simple_Xe2_Lpm_r0},
    {VAProfileMPEG2Main,   &entrypointMap_VAProfileMPEG2Main_Xe2_Lpm_r0},
#endif

#if defined(_VP8_DECODE_SUPPORTED)
    {VAProfileVP8Version0_3, &entrypointMap_VAProfileVP8Version0_3_Xe2_Lpm_r0},
#endif

#if defined(_VVC_DECODE_SUPPORTED)
    {(VAProfile)VAProfileVVCMain10, &entrypointMap_VAProfileVVCMain10_Xe2_Lpm_r0},
    {(VAProfile)VAProfileVVCMultilayerMain10, &entrypointMap_VAProfileVVCMultilayerMain10_Xe2_Lpm_r0},
#endif
};

static const CapsData capsData_Xe2_Lpm_r0
{
  &profileMap_Xe2_Lpm_r0,
  &imgtbl_Xe2_lpm_r0
};

#endif //__CAPSTABLE_DATA_XE2_LPM_R0_SPECIFIC_H__
