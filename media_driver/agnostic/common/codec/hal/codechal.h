/*
* Copyright (c) 2011-2023, Intel Corporation
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
//! \file     codechal.h
//! \brief    Defines the public interface for CodecHal.
//!
#ifndef __CODECHAL_H__
#define __CODECHAL_H__

#include "codechal_common.h"
#include "mhw_cp_interface.h"
#include "codec_def_common.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define CODECHAL_PUBLIC_CHK_NULL(_ptr)                                                  \
    MOS_CHK_NULL(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_PUBLIC, _ptr)

#define CODECHAL_PUBLIC_CHK_STATUS(_stmt)                                               \
    MOS_CHK_STATUS(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_PUBLIC, _stmt)

#define CODECHAL_PUBLIC_CHK_NULL_NO_STATUS(_ptr)                                        \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_PUBLIC, _ptr)

//-----------------------------------------------------------------------------
// Forward declaration -
// IMPORTANT - DDI interfaces are NOT to access internal CODECHAL states
//-----------------------------------------------------------------------------
class CodechalDecode;
class CodechalEncoderState;

// Forward Declarations
class USERMODE_DEVICE_CONTEXT;

#if (_DEBUG || _RELEASE_INTERNAL)

#define CODECHAL_UPDATE_ENCODE_MMC_USER_FEATURE(surface, mosCtx)                                                \
{                                                                                                               \
    MOS_USER_FEATURE_VALUE_WRITE_DATA       userFeatureWriteData;                                               \
    \
    userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;                                              \
    userFeatureWriteData.Value.i32Data = surface.bCompressible;                                                \
    userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_MMC_ENC_RECON_COMPRESSIBLE_ID;                        \
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, mosCtx);                                      \
    \
    userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;                                              \
    userFeatureWriteData.Value.i32Data = surface.MmcState;                                                     \
    userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_MMC_ENC_RECON_COMPRESSMODE_ID;                        \
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, mosCtx);                                       \
}
#endif

#define CODECHAL_UPDATE_VDBOX_USER_FEATURE(videoGpuNode, mosCtx)                                                \
do                                                                                                              \
{                                                                                                               \
    MOS_USER_FEATURE_VALUE_ID               valueID;                                                            \
    MOS_USER_FEATURE_VALUE_DATA             userFeatureData;                                                    \
    MOS_USER_FEATURE_VALUE_WRITE_DATA       userFeatureWriteData;                                               \
                                                                                                                \
    valueID = ((videoGpuNode == MOS_GPU_NODE_VIDEO2) ?                                                          \
        __MEDIA_USER_FEATURE_VALUE_NUMBER_OF_CODEC_DEVICES_ON_VDBOX2_ID :                                           \
        __MEDIA_USER_FEATURE_VALUE_NUMBER_OF_CODEC_DEVICES_ON_VDBOX1_ID);                                           \
                                                                                                                \
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));                                                  \
    MOS_UserFeature_ReadValue_ID(                                                                               \
        nullptr,                                                                                                \
        valueID,                                                                                                \
        &userFeatureData,                                                                                       \
        mosCtx);                                                                                                \
                                                                                                                \
    userFeatureData.i32Data++;                                                                                  \
    MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));                                        \
    userFeatureWriteData.ValueID = valueID;                                                                     \
    userFeatureWriteData.Value.i32Data = userFeatureData.i32Data;                                               \
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, mosCtx);                                  \
} while (0)

#define CODECHAL_UPDATE_USED_VDBOX_ID_USER_FEATURE(instanceId, mosCtx)                                          \
do                                                                                                              \
{                                                                                                               \
    MOS_USER_FEATURE_VALUE_ID               valueID;                                                            \
    MOS_USER_FEATURE_VALUE_DATA             userFeatureData;                                                    \
    MOS_USER_FEATURE_VALUE_WRITE_DATA       userFeatureWriteData;                                               \
                                                                                                                \
    valueID = __MEDIA_USER_FEATURE_VALUE_VDBOX_ID_USED;                                                         \
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));                                                  \
    MOS_UserFeature_ReadValue_ID(                                                                               \
        nullptr,                                                                                                \
        valueID,                                                                                                \
        &userFeatureData,                                                                                       \
        mosCtx);                                                                                                \
                                                                                                                \
    if(!(userFeatureData.i32DataFlag & (1 << ((instanceId) << 2))))                                             \
    {                                                                                                           \
        userFeatureData.i32Data |= 1 << ((instanceId) << 2);                                                    \
        userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;                                          \
        userFeatureWriteData.ValueID = valueID;                                                                 \
        userFeatureWriteData.Value.i32Data = userFeatureData.i32Data;                                           \
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, mosCtx);                              \
    }                                                                                                           \
} while (0)

// VP8 Coefficient Probability data
typedef struct _CODECHAL_VP8_COEFFPROB_DATA
{
    uint8_t   CoeffProbs[4][8][3][11];
} CODECHAL_VP8_COEFFPROB_DATA, *PCODECHAL_VP8_COEFFPROB_DATA;

typedef struct _CODECHAL_VP8_HYBRIDPAK_FRAMEUPDATE
{
    uint32_t  PrevFrameSize;
    uint8_t   TwoPrevFrameFlag;
    uint16_t  RefFrameCost[4];
    uint16_t  IntraModeCost[4][4];
    uint16_t  InterModeCost[4];
    uint8_t   IntraNonDCPenalty16x16[4];
    uint8_t   IntraNonDCPenalty4x4[4];
    uint8_t   RefQpIndex[3];
}CODECHAL_VP8_HYBRIDPAK_FRAMEUPDATE, *PCODECHAL_VP8_HYBRIDPAK_FRAMEUPDATE;

/*! \brief Information pertaining to the PAK object and MV data.
*/
typedef struct _CODEC_ENCODE_MBDATA_LAYOUT
{
    uint32_t    uiMbCodeBottomFieldOffset;  //!< Offset to the PAK objects for the bottom field
    uint32_t    uiMvOffset;                 //!< Base offset of the MV data
    uint32_t    uiMvBottomFieldOffset;      //!< Offset to the MV data for the bottom field
} CODEC_ENCODE_MBDATA_LAYOUT, *PCODEC_ENCODE_MBDATA_LAYOUT;

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __CODECHAL_H__
