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
//! \file     capstable_data_avc_encode_xe2_lpm_r0_specific.h
//! \brief    This file register all caps data
//!

#ifndef __CAPSTABLE_DATA_AVC_ENCODE_XE2_LPM_R0_SPECIFIC_H__
#define __CAPSTABLE_DATA_AVC_ENCODE_XE2_LPM_R0_SPECIFIC_H__

#include "capstable_data_xe2_lpm_r0_specific.h"
#include "codec_def_encode_avc.h"
//!
//! \brief  Definition for bitset value
//!
static const VAConfigAttribValEncROI VAProfileH264ConstrainedBaseline_VAEntrypointEncSlice_encROI_Xe2_Lpm_r0
{
    {ENCODE_VDENC_AVC_MAX_ROI_NUMBER_ADV,0,1,0}
};

static const VAConfigAttribValMaxFrameSize VAProfileH264ConstrainedBaseline_VAEntrypointEncSlice_maxFrameSize_Xe2_Lpm_r0
{
    {1,1,0}
};

//! \brief  Definition for ConfigDataList
static const ConfigDataList configDataList_VAProfileH264Main_VAEntrypointEncSlice_Xe2_Lpm_r0 =
{
    {VA_RC_CQP, 0},
    {VA_RC_CBR, 0},
    {VA_RC_VBR, 0},
    {VA_RC_ICQ, 0},
    {VA_RC_QVBR, 0},
#if VA_CHECK_VERSION(1, 10, 0)
    {VA_RC_TCBRC, 0},
#endif
    {VA_RC_CBR | VA_RC_MB, 0},
    {VA_RC_VBR | VA_RC_MB, 0},
};

static const ConfigDataList configDataList_VAProfileH264High_VAEntrypointEncSlice_Xe2_Lpm_r0 =
{
    {VA_RC_CQP, 0},
    {VA_RC_CBR, 0},
    {VA_RC_VBR, 0},
    {VA_RC_ICQ, 0},
    {VA_RC_QVBR, 0},
#if VA_CHECK_VERSION(1, 10, 0)
    {VA_RC_TCBRC, 0},
#endif
    {VA_RC_CBR | VA_RC_MB, 0},
    {VA_RC_VBR | VA_RC_MB, 0},
};

static const ConfigDataList configDataList_VAProfileH264ConstrainedBaseline_VAEntrypointEncSlice_Xe2_Lpm_r0 =
{
    {VA_RC_CQP, 0},
    {VA_RC_CBR, 0},
    {VA_RC_VBR, 0},
    {VA_RC_ICQ, 0},
    {VA_RC_QVBR, 0},
#if VA_CHECK_VERSION(1, 10, 0)
    {VA_RC_TCBRC, 0},
#endif
    {VA_RC_CBR | VA_RC_MB, 0},
    {VA_RC_VBR | VA_RC_MB, 0},

};

//!
//! \brief  Definition for AttribList
//!
static const AttribList AttribList_VAProfileH264Main_VAEntrypointEncSlice_Xe2_Lpm_r0
{
    {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_RGB32},
    {VAConfigAttribEncQualityRange, NUM_TARGET_USAGE_MODES - 1},
    {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_PICTURE | VA_ENC_PACKED_HEADER_SEQUENCE | VA_ENC_PACKED_HEADER_SLICE | VA_ENC_PACKED_HEADER_RAW_DATA | VA_ENC_PACKED_HEADER_MISC},
    {VAConfigAttribRateControl, VA_RC_CQP | VA_RC_CBR | VA_RC_VBR | VA_RC_QVBR | VA_RC_MB | VA_RC_ICQ
#if VA_CHECK_VERSION(1, 10, 0)
     | VA_RC_TCBRC
#endif
     },
    {VAConfigAttribEncParallelRateControl, 0},
    {VAConfigAttribEncInterlaced, VA_ENC_INTERLACED_NONE},
    {VAConfigAttribEncMaxRefFrames, DDI_CODEC_VDENC_MAX_L0_REF_FRAMES | (DDI_CODEC_VDENC_MAX_L1_REF_FRAMES_RAB_AVC << DDI_CODEC_LEFT_SHIFT_FOR_REFLIST1)},
    {VAConfigAttribEncMaxSlices, ENCODE_AVC_MAX_SLICES_SUPPORTED},
    {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE | VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS | VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS},
    {VAConfigAttribEncQuantization, VA_ENC_QUANTIZATION_TRELLIS_SUPPORTED},
    {VAConfigAttribEncIntraRefresh, VA_ENC_INTRA_REFRESH_ROLLING_COLUMN | VA_ENC_INTRA_REFRESH_ROLLING_ROW},
    {VAConfigAttribEncSkipFrame, 1},
    {VAConfigAttribEncROI, VAProfileH264ConstrainedBaseline_VAEntrypointEncSlice_encROI_Xe2_Lpm_r0.value},
    {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_ENCODE},
    {VAConfigAttribEncDirtyRect, 4},
    {VAConfigAttribMaxFrameSize, VAProfileH264ConstrainedBaseline_VAEntrypointEncSlice_maxFrameSize_Xe2_Lpm_r0.value},
    {VAConfigAttribMaxPictureWidth, CODEC_4K_MAX_PIC_WIDTH},
    {VAConfigAttribMaxPictureHeight, CODEC_4K_MAX_PIC_HEIGHT},
    {VAConfigAttribCustomRoundingControl, 1},
    {VAConfigAttribFEIMVPredictors, DDI_CODEC_FEI_MAX_NUM_MVPREDICTOR},
};

static const AttribList AttribList_VAProfileH264High_VAEntrypointEncSlice_Xe2_Lpm_r0
{
    {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_RGB32},
    {VAConfigAttribEncQualityRange, NUM_TARGET_USAGE_MODES - 1},
    {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_PICTURE | VA_ENC_PACKED_HEADER_SEQUENCE | VA_ENC_PACKED_HEADER_SLICE | VA_ENC_PACKED_HEADER_RAW_DATA | VA_ENC_PACKED_HEADER_MISC},
    {VAConfigAttribRateControl, VA_RC_CQP | VA_RC_CBR | VA_RC_VBR | VA_RC_QVBR | VA_RC_MB | VA_RC_ICQ
#if VA_CHECK_VERSION(1, 10, 0)
    | VA_RC_TCBRC
#endif
    },
    {VAConfigAttribEncParallelRateControl, 0},
    {VAConfigAttribEncInterlaced, VA_ENC_INTERLACED_NONE},
    {VAConfigAttribEncMaxRefFrames, DDI_CODEC_VDENC_MAX_L0_REF_FRAMES | (DDI_CODEC_VDENC_MAX_L1_REF_FRAMES_RAB_AVC << DDI_CODEC_LEFT_SHIFT_FOR_REFLIST1)},
    {VAConfigAttribEncMaxSlices, ENCODE_AVC_MAX_SLICES_SUPPORTED},
    {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE | VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS | VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS},
    {VAConfigAttribEncQuantization, VA_ENC_QUANTIZATION_TRELLIS_SUPPORTED},
    {VAConfigAttribEncIntraRefresh, VA_ENC_INTRA_REFRESH_ROLLING_COLUMN | VA_ENC_INTRA_REFRESH_ROLLING_ROW},
    {VAConfigAttribEncSkipFrame, 1},
    {VAConfigAttribEncROI, VAProfileH264ConstrainedBaseline_VAEntrypointEncSlice_encROI_Xe2_Lpm_r0.value},
    {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_ENCODE},
    {VAConfigAttribEncDirtyRect, 4},
    {VAConfigAttribMaxFrameSize, VAProfileH264ConstrainedBaseline_VAEntrypointEncSlice_maxFrameSize_Xe2_Lpm_r0.value},
    {VAConfigAttribMaxPictureWidth, CODEC_4K_MAX_PIC_WIDTH},
    {VAConfigAttribMaxPictureHeight, CODEC_4K_MAX_PIC_HEIGHT},
    {VAConfigAttribCustomRoundingControl, 1},
    {VAConfigAttribFEIMVPredictors, DDI_CODEC_FEI_MAX_NUM_MVPREDICTOR},
};

static const AttribList AttribList_VAProfileH264ConstrainedBaseline_VAEntrypointEncSlice_Xe2_Lpm_r0
{
    {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_RGB32},
    {VAConfigAttribEncQualityRange, NUM_TARGET_USAGE_MODES - 1},
    {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_PICTURE | VA_ENC_PACKED_HEADER_SEQUENCE | VA_ENC_PACKED_HEADER_SLICE | VA_ENC_PACKED_HEADER_RAW_DATA | VA_ENC_PACKED_HEADER_MISC},
    {VAConfigAttribRateControl, VA_RC_CQP | VA_RC_CBR | VA_RC_VBR | VA_RC_QVBR | VA_RC_MB | VA_RC_ICQ
#if VA_CHECK_VERSION(1, 10, 0)
    |  VA_RC_TCBRC
#endif
    },
    {VAConfigAttribEncParallelRateControl, 0},
    {VAConfigAttribEncInterlaced, VA_ENC_INTERLACED_NONE},
    {VAConfigAttribEncMaxRefFrames, DDI_CODEC_VDENC_MAX_L0_REF_FRAMES | (DDI_CODEC_VDENC_MAX_L1_REF_FRAMES_RAB_AVC << DDI_CODEC_LEFT_SHIFT_FOR_REFLIST1)},
    {VAConfigAttribEncMaxSlices, ENCODE_AVC_MAX_SLICES_SUPPORTED},
    {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE | VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS | VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS},
    {VAConfigAttribEncQuantization, VA_ENC_QUANTIZATION_TRELLIS_SUPPORTED},
    {VAConfigAttribEncIntraRefresh, VA_ENC_INTRA_REFRESH_ROLLING_COLUMN | VA_ENC_INTRA_REFRESH_ROLLING_ROW},
    {VAConfigAttribEncSkipFrame, 1},
    {VAConfigAttribEncROI, VAProfileH264ConstrainedBaseline_VAEntrypointEncSlice_encROI_Xe2_Lpm_r0.value},
    {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_ENCODE},
    {VAConfigAttribEncDirtyRect, 4},
    {VAConfigAttribMaxFrameSize, VAProfileH264ConstrainedBaseline_VAEntrypointEncSlice_maxFrameSize_Xe2_Lpm_r0.value},
    {VAConfigAttribMaxPictureWidth, CODEC_4K_MAX_PIC_WIDTH},
    {VAConfigAttribMaxPictureHeight, CODEC_4K_MAX_PIC_HEIGHT},
    {VAConfigAttribCustomRoundingControl, 1},
    {VAConfigAttribFEIMVPredictors, DDI_CODEC_FEI_MAX_NUM_MVPREDICTOR},
};

//!
static const ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileH264Main_VAEntrypointEncSlice_Xe2_Lpm_r0 =
{
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_NV12}}},
    {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_4K_MAX_PIC_WIDTH}}},
    {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_4K_MAX_PIC_HEIGHT}}},
    {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {32}}},
    {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {32}}},
    {VASurfaceAttribMemoryType, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

//!
static const ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileH264High_VAEntrypointEncSlice_Xe2_Lpm_r0 =
{
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_NV12}}},
    {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_4K_MAX_PIC_WIDTH}}},
    {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_4K_MAX_PIC_HEIGHT}}},
    {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {32}}},
    {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {32}}},
    {VASurfaceAttribMemoryType, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

//!
static const ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileH264ConstrainedBaseline_VAEntrypointEncSlice_Xe2_Lpm_r0 =
{
    {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_NV12}}},
    {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_4K_MAX_PIC_WIDTH}}},
    {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_4K_MAX_PIC_HEIGHT}}},
    {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {32}}},
    {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {32}}},
    {VASurfaceAttribMemoryType, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}}
};

//!
//! \brief  Definition for EmtrypointMap
//!
static const EntrypointData entrypointMap_VAProfileH264Main_Data_Xe2_Lpm_r0
{
    &AttribList_VAProfileH264Main_VAEntrypointEncSlice_Xe2_Lpm_r0,
    &configDataList_VAProfileH264Main_VAEntrypointEncSlice_Xe2_Lpm_r0,
    &surfaceAttribInfo_VAProfileH264Main_VAEntrypointEncSlice_Xe2_Lpm_r0
};

static const EntrypointData entrypointMap_VAProfileH264High_Data_Xe2_Lpm_r0
{
    &AttribList_VAProfileH264High_VAEntrypointEncSlice_Xe2_Lpm_r0,
    &configDataList_VAProfileH264High_VAEntrypointEncSlice_Xe2_Lpm_r0,
    &surfaceAttribInfo_VAProfileH264High_VAEntrypointEncSlice_Xe2_Lpm_r0
};

static const EntrypointData entrypointMap_VAProfileH264ConstrainedBaseline_Data_Xe2_Lpm_r0
{
    &AttribList_VAProfileH264ConstrainedBaseline_VAEntrypointEncSlice_Xe2_Lpm_r0,
    &configDataList_VAProfileH264ConstrainedBaseline_VAEntrypointEncSlice_Xe2_Lpm_r0,
    &surfaceAttribInfo_VAProfileH264ConstrainedBaseline_VAEntrypointEncSlice_Xe2_Lpm_r0
};

#endif
