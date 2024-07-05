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
//! \file     capstable_data_hevc_decode_xe2_lpm_r0_specific.h
//! \brief    This file register all caps data
//!

#ifndef __CAPSTABLE_DATA_HEVC_DECODE_XE2_LPM_R0_SPECIFIC_H__
#define __CAPSTABLE_DATA_HEVC_DECODE_XE2_LPM_R0_SPECIFIC_H__

#include "capstable_data_xe2_lpm_r0_specific.h"
#include "codec_def_common.h"

#ifndef VA_ENCRYPTION_TYPE_NONE
#define VA_ENCRYPTION_TYPE_NONE 0x00000000
#endif

//!
//! \brief  Definition for ConfigDataList
//!
static ConfigDataList configDataList_VAProfileHEVCMain422_10_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING}
};

static ConfigDataList configDataList_VAProfileHEVCMain444_10_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING}
};

static ConfigDataList configDataList_VAProfileHEVCSccMain_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING}
};

static ConfigDataList configDataList_VAProfileHEVCSccMain444_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING}
};

static ConfigDataList configDataList_VAProfileHEVCMain444_12_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING}
};

static ConfigDataList configDataList_VAProfileHEVCSccMain444_10_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING}
};

static ConfigDataList configDataList_VAProfileHEVCSccMain10_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING}
};

static ConfigDataList configDataList_VAProfileHEVCMain422_12_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING}
};

static ConfigDataList configDataList_VAProfileHEVCMain10_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING}
};

static ConfigDataList configDataList_VAProfileHEVCMain_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING}
};

static ConfigDataList configDataList_VAProfileHEVCMain12_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING}
};

static ConfigDataList configDataList_VAProfileHEVCMain444_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_NORMAL, VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING_NONE},
  {VA_DEC_SLICE_MODE_BASE,   VA_ENCRYPTION_TYPE_NONE, VA_DEC_PROCESSING}
};

//!
//! \brief  Definition for AttribList
//!
static const AttribList attribList_VAProfileHEVCMain_VAEntrypointVLD_Xe2_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_16K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

static const AttribList attribList_VAProfileHEVCMain10_VAEntrypointVLD_Xe2_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_16K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

static const AttribList attribList_VAProfileHEVCMain12_VAEntrypointVLD_Xe2_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV420_12 | VA_RT_FORMAT_YUV400},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_16K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

static const AttribList attribList_VAProfileHEVCMain422_10_VAEntrypointVLD_Xe2_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV422_10 | VA_RT_FORMAT_YUV400},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_16K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

static const AttribList attribList_VAProfileHEVCMain422_12_VAEntrypointVLD_Xe2_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV420_12 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV422_10 | VA_RT_FORMAT_YUV422_12 | VA_RT_FORMAT_YUV400},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_16K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

static const AttribList attribList_VAProfileHEVCMain444_VAEntrypointVLD_Xe2_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV400},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_16K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

static const AttribList attribList_VAProfileHEVCSccMain444_VAEntrypointVLD_Xe2_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV400},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_16K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

static const AttribList attribList_VAProfileHEVCMain444_10_VAEntrypointVLD_Xe2_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV422_10 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_YUV400},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_16K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

static const AttribList attribList_VAProfileHEVCSccMain444_10_VAEntrypointVLD_Xe2_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV422_10 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_YUV400},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_16K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

static const AttribList attribList_VAProfileHEVCMain444_12_VAEntrypointVLD_Xe2_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV420_12 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV422_10 | VA_RT_FORMAT_YUV422_12 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_YUV444_12 | VA_RT_FORMAT_YUV400},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_16K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

static const AttribList attribList_VAProfileHEVCSccMain_VAEntrypointVLD_Xe2_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV400},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_16K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

static const AttribList attribList_VAProfileHEVCSccMain10_VAEntrypointVLD_Xe2_Lpm_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV400},
   {VAConfigAttribDecSliceMode, VA_DEC_SLICE_MODE_NORMAL},
   {VAConfigAttribDecProcessing, VA_DEC_PROCESSING},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_16K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncryption, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_DECODE},
   {VAConfigAttribCustomRoundingControl, 1},
};

//!
//! \brief   Definition for ProfileSurfaceAttribInfo
//!
static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCMain_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_NV12}}},
  {VASurfaceAttribMaxWidth,    VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight,   VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCSccMain_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_NV12}}},
  {VASurfaceAttribMaxWidth,    VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight,   VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCMain10_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_P010}}},
  {VASurfaceAttribMaxWidth,    VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight,   VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCMain12_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_P012}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_P016}}},
  {VASurfaceAttribMaxWidth,    VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight,   VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCMain422_10_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_YUY2}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y210}}},
  {VASurfaceAttribMaxWidth,    VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight,   VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCMain422_12_VAEntrypointVLD_Xe2_Lpm_r0 =
{
#if VA_CHECK_VERSION(1, 9, 0)
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y212}}},
#endif
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y216}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_P012}}},
  {VASurfaceAttribMaxWidth,    VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight,   VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCMain444_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_AYUV}}},
#if VA_CHECK_VERSION(1, 13, 0)
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_XYUV}}},
#endif
  {VASurfaceAttribMaxWidth,    VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight,   VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCMain444_10_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y410}}},
  {VASurfaceAttribMaxWidth,    VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight,   VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCMain444_12_VAEntrypointVLD_Xe2_Lpm_r0 =
{
#if VA_CHECK_VERSION(1, 9, 0)
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y412}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y212}}},
#endif
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y416}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_P012}}},
  {VASurfaceAttribMaxWidth,    VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight,   VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCSccMain10_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_P010}}},
  {VASurfaceAttribMaxWidth,    VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight,   VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCSccMain444_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_YUY2}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y210}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_AYUV}}},
#if VA_CHECK_VERSION(1, 13, 0)
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_XYUV}}},
#endif
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y410}}},
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_NV12}}},
  {VASurfaceAttribMaxWidth,    VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight,   VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCSccMain444_10_VAEntrypointVLD_Xe2_Lpm_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_AYUV}}},
#if VA_CHECK_VERSION(1, 13, 0)
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_XYUV}}},
#endif
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y410}}},
  {VASurfaceAttribMaxWidth,    VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight,   VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

//!
//! \brief  Definition for EntrypointMap
//!
static const EntrypointData entrypointMap_VAProfileHEVCMainDec_Data_Xe2_Lpm_r0
{
    &attribList_VAProfileHEVCMain_VAEntrypointVLD_Xe2_Lpm_r0,
    &configDataList_VAProfileHEVCMain_VAEntrypointVLD_Xe2_Lpm_r0,
    &surfaceAttribInfo_VAProfileHEVCMain_VAEntrypointVLD_Xe2_Lpm_r0
};

static const EntrypointData entrypointMap_VAProfileHEVCMain10Dec_Data_Xe2_Lpm_r0
{
    &attribList_VAProfileHEVCMain10_VAEntrypointVLD_Xe2_Lpm_r0,
    &configDataList_VAProfileHEVCMain10_VAEntrypointVLD_Xe2_Lpm_r0,
    &surfaceAttribInfo_VAProfileHEVCMain10_VAEntrypointVLD_Xe2_Lpm_r0
};

static const EntrypointData entrypointMap_VAProfileHEVCMain12Dec_Data_Xe2_Lpm_r0
{
    &attribList_VAProfileHEVCMain12_VAEntrypointVLD_Xe2_Lpm_r0,
    &configDataList_VAProfileHEVCMain12_VAEntrypointVLD_Xe2_Lpm_r0,
    &surfaceAttribInfo_VAProfileHEVCMain12_VAEntrypointVLD_Xe2_Lpm_r0
};

static const EntrypointData entrypointMap_VAProfileHEVCMain422_10Dec_Data_Xe2_Lpm_r0
{
    &attribList_VAProfileHEVCMain422_10_VAEntrypointVLD_Xe2_Lpm_r0,
    &configDataList_VAProfileHEVCMain422_10_VAEntrypointVLD_Xe2_Lpm_r0,
    &surfaceAttribInfo_VAProfileHEVCMain422_10_VAEntrypointVLD_Xe2_Lpm_r0
};

static const EntrypointData entrypointMap_VAProfileHEVCMain422_12Dec_Data_Xe2_Lpm_r0
{
    &attribList_VAProfileHEVCMain422_12_VAEntrypointVLD_Xe2_Lpm_r0,
    &configDataList_VAProfileHEVCMain422_12_VAEntrypointVLD_Xe2_Lpm_r0,
    &surfaceAttribInfo_VAProfileHEVCMain422_12_VAEntrypointVLD_Xe2_Lpm_r0
};

static const EntrypointData entrypointMap_VAProfileHEVCMain444Dec_Data_Xe2_Lpm_r0
{
    &attribList_VAProfileHEVCMain444_VAEntrypointVLD_Xe2_Lpm_r0,
    &configDataList_VAProfileHEVCMain444_VAEntrypointVLD_Xe2_Lpm_r0,
    &surfaceAttribInfo_VAProfileHEVCMain444_VAEntrypointVLD_Xe2_Lpm_r0
};

static const EntrypointData entrypointMap_VAProfileHEVCSccMain444Dec_Data_Xe2_Lpm_r0
{
    &attribList_VAProfileHEVCSccMain444_VAEntrypointVLD_Xe2_Lpm_r0,
    &configDataList_VAProfileHEVCSccMain444_VAEntrypointVLD_Xe2_Lpm_r0,
    &surfaceAttribInfo_VAProfileHEVCSccMain444_VAEntrypointVLD_Xe2_Lpm_r0
};

static const EntrypointData entrypointMap_VAProfileHEVCMain444_10Dec_Data_Xe2_Lpm_r0
{
    &attribList_VAProfileHEVCMain444_10_VAEntrypointVLD_Xe2_Lpm_r0,
    &configDataList_VAProfileHEVCMain444_10_VAEntrypointVLD_Xe2_Lpm_r0,
    &surfaceAttribInfo_VAProfileHEVCMain444_10_VAEntrypointVLD_Xe2_Lpm_r0
};

static const EntrypointData entrypointMap_VAProfileHEVCSccMain444_10Dec_Data_Xe2_Lpm_r0
{
    &attribList_VAProfileHEVCSccMain444_10_VAEntrypointVLD_Xe2_Lpm_r0,
    &configDataList_VAProfileHEVCSccMain444_10_VAEntrypointVLD_Xe2_Lpm_r0,
    &surfaceAttribInfo_VAProfileHEVCSccMain444_10_VAEntrypointVLD_Xe2_Lpm_r0
};

static const EntrypointData entrypointMap_VAProfileHEVCMain444_12Dec_Data_Xe2_Lpm_r0
{
    &attribList_VAProfileHEVCMain444_12_VAEntrypointVLD_Xe2_Lpm_r0,
    &configDataList_VAProfileHEVCMain444_12_VAEntrypointVLD_Xe2_Lpm_r0,
    &surfaceAttribInfo_VAProfileHEVCMain444_12_VAEntrypointVLD_Xe2_Lpm_r0
};

static const EntrypointData entrypointMap_VAProfileHEVCSccMainDec_Data_Xe2_Lpm_r0
{
    &attribList_VAProfileHEVCSccMain_VAEntrypointVLD_Xe2_Lpm_r0,
    &configDataList_VAProfileHEVCSccMain_VAEntrypointVLD_Xe2_Lpm_r0,
    &surfaceAttribInfo_VAProfileHEVCSccMain_VAEntrypointVLD_Xe2_Lpm_r0
};

static const EntrypointData entrypointMap_VAProfileHEVCSccMain10Dec_Data_Xe2_Lpm_r0
{
    &attribList_VAProfileHEVCSccMain10_VAEntrypointVLD_Xe2_Lpm_r0,
    &configDataList_VAProfileHEVCSccMain10_VAEntrypointVLD_Xe2_Lpm_r0,
    &surfaceAttribInfo_VAProfileHEVCSccMain10_VAEntrypointVLD_Xe2_Lpm_r0
};

#endif
