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
//! \file     capstable_data_av1_encode_xe_lpm_plus_r0_specific.h
//! \brief    This file register all caps data
//!

#ifndef __CAPSTABLE_DATA_AV1_ENCODE_XE_LPM_PLUS_R0_SPECIFIC_H__
#define __CAPSTABLE_DATA_AV1_ENCODE_XE_LPM_PLUS_R0_SPECIFIC_H__

#include "capstable_data_xe_lpm_plus_r0_specific.h"
#include "codec_def_encode_av1.h"

//!
//! \brief  Definition for bitset value
//!
static const VAConfigAttribValEncAV1Ext2 VAProfileAV1Profile0_VAEntrypointEncSliceLP_encAV1Ext2
{
    {TILE_SIZE_BYTES - 1,0,AV1_TX_MODE_LARGEST_SUPPORTED,0,0}
};

static const VAConfigAttribValEncAV1 VAProfileAV1Profile0_VAEntrypointEncSliceLP_encAV1
{
    {0,0,0,0,0,0,0,0,0,0,0,0,0,1,0}
};

static const VAConfigAttribValEncAV1Ext1 VAProfileAV1Profile0_VAEntrypointEncSliceLP_encAV1Ext1
{
    {31,32,1,0}
};

static const VAConfigAttribValEncROI VAProfileAV1Profile0_VAEntrypointEncSliceLP_encROI
{
    {0,0,1,0}
};

//! \brief  Definition for ConfigDataList
static ConfigDataList configDataList_VAProfileAV1Profile0_VAEntrypointEncSliceLP_Xe_Lpm_plus_r0 =
{ 
  {VA_RC_CQP, 0}, 
  {VA_RC_CBR, 0}, 
  {VA_RC_VBR, 0} 
}; 

//!
//! \brief  Definition for AttribList
//!
static const AttribList attribList_VAProfileAV1Profile0_VAEntrypointEncSliceLP_Xe_Lpm_plus_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10},
   {VAConfigAttribEncDynamicScaling, 0},
   {VAConfigAttribEncDirtyRect, VA_ATTRIB_NOT_SUPPORTED},
   {VAConfigAttribEncAV1Ext2, VAProfileAV1Profile0_VAEntrypointEncSliceLP_encAV1Ext2.value},
   {VAConfigAttribEncAV1, VAProfileAV1Profile0_VAEntrypointEncSliceLP_encAV1.value},
   {VAConfigAttribMaxPictureWidth, CODEC_8K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_8K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncAV1Ext1, VAProfileAV1Profile0_VAEntrypointEncSliceLP_encAV1Ext1.value},
   {VAConfigAttribRateControl, VA_RC_CQP | VA_RC_CBR | VA_RC_VBR},
   {VAConfigAttribEncTileSupport, 1},
   {VAConfigAttribEncMaxRefFrames, CODEC_AV1_NUM_REFL0P_FRAMES | CODEC_AV1_NUM_REFL0B_FRAMES<<8 | CODEC_AV1_NUM_REFL1B_FRAMES<<16},
   {VAConfigAttribEncJPEG, (JPEG_MAX_QUANT_TABLE << 14)| (JPEG_MAX_NUM_HUFF_TABLE_INDEX << 11)|(1 << 7)|(jpegNumComponent << 4)},
   {VAConfigAttribEncQualityRange, NUM_TARGET_USAGE_MODES - 1},
   {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_PICTURE | VA_ENC_PACKED_HEADER_SEQUENCE | VA_ENC_PACKED_HEADER_SLICE | VA_ENC_PACKED_HEADER_RAW_DATA | VA_ENC_PACKED_HEADER_MISC},
   {VAConfigAttribEncInterlaced, VA_ENC_INTERLACED_NONE},
   //just to match with legacy correct value 0
   {VAConfigAttribEncMaxSlices, 3},
   {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE | VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS | VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS},
   {VAConfigAttribEncQuantization, 0},
   {VAConfigAttribEncIntraRefresh, 0},
   {VAConfigAttribEncSkipFrame, 0},
   {VAConfigAttribEncROI, VAProfileAV1Profile0_VAEntrypointEncSliceLP_encROI.value},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_ENCODE},
   {VAConfigAttribEncParallelRateControl, 0},
   {VAConfigAttribFEIMVPredictors, 0},
   {VAConfigAttribCustomRoundingControl, 1},
};

//!
static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileAV1Profile0_VAEntrypointEncSliceLP_Xe_Lpm_plus_r0 =
{ 
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_NV12}}}, 
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_P010}}}, 
  {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {8192}}}, 
  {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {8192}}}, 
  {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {128}}}, 
  {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {96}}}, 
  {VASurfaceAttribMemoryType, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_SURFACE_ATTRIB_MEM_TYPE_VA | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2}}} 
}; 

//!
//! \brief  Definition for EmtrypointMap
//!
static const EntrypointData entrypointMap_VAProfileAV1Profile0_Data_Xe_Lpm_plus_r0
{
    &attribList_VAProfileAV1Profile0_VAEntrypointEncSliceLP_Xe_Lpm_plus_r0,
    &configDataList_VAProfileAV1Profile0_VAEntrypointEncSliceLP_Xe_Lpm_plus_r0,
    &surfaceAttribInfo_VAProfileAV1Profile0_VAEntrypointEncSliceLP_Xe_Lpm_plus_r0
};


#endif
