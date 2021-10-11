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

#ifndef __CAPSTABLE_DATA_HEVC_VDENC_M12_0_R0_SPECIFIC_H__
#define __CAPSTABLE_DATA_HEVC_VDENC_M12_0_R0_SPECIFIC_H__

#include "media_capstable_specific.h"

static ConfigDataList configDataList_VAProfileHEVCMain_VAEntrypointEncSliceLP_M12_0_r0 =
{
    {VA_RC_CQP,                    0},
    {VA_RC_CBR,                    0}, 
    {VA_RC_VBR,                    0},
    {VA_RC_ICQ,                    0},
    {VA_RC_VCM,                    0},
    {VA_RC_QVBR,                   0},
    {VA_RC_AVBR,                   0},
    {VA_RC_TCBRC,                  0},
    {VA_RC_CBR   | VA_RC_PARALLEL, 0}, 
    {VA_RC_VBR   | VA_RC_PARALLEL, 0},
    {VA_RC_ICQ   | VA_RC_PARALLEL, 0},
    {VA_RC_VCM   | VA_RC_PARALLEL, 0},
    {VA_RC_QVBR  | VA_RC_PARALLEL, 0},
    {VA_RC_AVBR  | VA_RC_PARALLEL, 0},
    {VA_RC_TCBRC | VA_RC_PARALLEL, 0},
};

static VAConfigAttribValEncROI HEVCCommon_VAEntrypointEncSliceLP_encROI_M12_0_r0
{
    {CODECHAL_ENCODE_HEVC_MAX_NUM_ROI,0,1,0}
};

static AttribList VAConfigAttribList_VAProfileHEVCMain_VAEntrypointEncSliceLP_M12_0_r0 = 
{
    {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_RGB32},
    {VAConfigAttribRateControl, VA_RC_CBR | VA_RC_VBR | VA_RC_QVBR | VA_RC_MB | VA_RC_VCM},
    {VAConfigAttribEncParallelRateControl, 0},
    {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_PICTURE | VA_ENC_PACKED_HEADER_SEQUENCE | VA_ENC_PACKED_HEADER_SLICE | VA_ENC_PACKED_HEADER_RAW_DATA | VA_ENC_PACKED_HEADER_MISC},
    {VAConfigAttribEncInterlaced, 0},
    {VAConfigAttribEncMaxRefFrames, DDI_CODEC_VDENC_MAX_L0_REF_FRAMES | (DDI_CODEC_VDENC_MAX_L1_REF_FRAMES << DDI_CODEC_LEFT_SHIFT_FOR_REFLIST1)},
    {VAConfigAttribEncMaxSlices, ENCODE_HEVC_VDENC_NUM_MAX_SLICES},
    {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE | VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS},
    {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
    {VAConfigAttribMaxPictureHeight, CODEC_12K_MAX_PIC_HEIGHT},
    {VAConfigAttribEncQualityRange, NUM_TARGET_USAGE_MODES - 1},
    {VAConfigAttribEncIntraRefresh, VA_ENC_INTRA_REFRESH_ROLLING_COLUMN | VA_ENC_INTRA_REFRESH_ROLLING_ROW},
    {VAConfigAttribEncROI, HEVCCommon_VAEntrypointEncSliceLP_encROI_M12_0_r0.value},
    {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_ENCODE},
    {VAConfigAttribEncDirtyRect, 16},
    {VAConfigAttribEncTileSupport, 1},
    {VAConfigAttribFrameSizeToleranceSupport, 1},
    {VAConfigAttribCustomRoundingControl, 0},
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAEntrypointEncSliceLP_VAProfileHEVCMain_M12_0_r0
{
    {VASurfaceAttribPixelFormat, VAGenericValueTypeInteger, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, VA_FOURCC_P010},
    {VASurfaceAttribMaxWidth,    VAGenericValueTypeInteger, VA_SURFACE_ATTRIB_GETTABLE,                              16384},
    {VASurfaceAttribMaxHeight,   VAGenericValueTypeInteger, VA_SURFACE_ATTRIB_GETTABLE,                              12288},
    {VASurfaceAttribMinWidth,    VAGenericValueTypeInteger, VA_SURFACE_ATTRIB_GETTABLE,                              128},
    {VASurfaceAttribMinHeight,   VAGenericValueTypeInteger, VA_SURFACE_ATTRIB_GETTABLE,                              128},
};

//!
//! \brief  Definion for EntrypointMap
//!
static EntrypointData entrypointMap_VAProfileHEVCMain_Data_M12_0_r0
{
    &VAConfigAttribList_VAProfileHEVCMain_VAEntrypointEncSliceLP_M12_0_r0,
    &configDataList_VAProfileHEVCMain_VAEntrypointEncSliceLP_M12_0_r0,
    &surfaceAttribInfo_VAEntrypointEncSliceLP_VAProfileHEVCMain_M12_0_r0,
};

static EntrypointMap entrypointMap_VAProfileHEVCMain_M12_0_r0
{
    {VAEntrypointEncSliceLP, &entrypointMap_VAProfileHEVCMain_Data_M12_0_r0},
};

#endif //__CAPSTABLE_DATA_HEVC_VDENC_M12_0_R0_SPECIFIC_H__