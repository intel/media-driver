/*
* Copyright (c) 2015-2017, Intel Corporation
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
//! \file        codechal_encoder_g10.h   
//! \brief        This modules implements Render interface layer for decoding to be used               on all operating systems/DDIs, across CODECHAL components.   
//!
#ifndef __CODECHAL_ENCODER_G10_H__
#define __CODECHAL_ENCODER_G10_H__

#include "codechal_encoder_base.h"

typedef struct _MEDIA_OBJECT_DOWNSCALING_CONVERSION_STATIC_DATA_G10
{
    // DWORD 0
    uint32_t   DW0_InputBitDepthForChroma                          : MOS_BITFIELD_RANGE( 0,  7);
    uint32_t   DW0_InputBitDepthForLuma                            : MOS_BITFIELD_RANGE( 8, 15);
    uint32_t   DW0_OutputBitDepthForChroma                         : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW0_OutputBitDepthForLuma                           : MOS_BITFIELD_RANGE(24, 30);
    uint32_t   DW0_RoundingEnable                                  : MOS_BITFIELD_BIT(      31);

    // DWORD 1
    uint32_t   DW1_PictureFormat                                   : MOS_BITFIELD_RANGE( 0,  7);
    uint32_t   DW1_ConvertFlag                                     : MOS_BITFIELD_BIT(       8);
    uint32_t   DW1_DownscaleStage                                  : MOS_BITFIELD_RANGE( 9, 11);
    uint32_t   DW1_MbStatisticsDumpFlag                            : MOS_BITFIELD_BIT(      12);
    uint32_t   DW1_Reserved_0                                      : MOS_BITFIELD_RANGE(13, 14);
    uint32_t   DW1_LcuSize                                         : MOS_BITFIELD_BIT(      15);
    uint32_t   DW1_JobQueueSize                                    : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 2
    uint32_t   DW2_OriginalPicWidthInSamples                       : MOS_BITFIELD_RANGE( 0, 15);
    uint32_t   DW2_OriginalPicHeightInSamples                      : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 3
    uint32_t   DW3_BTI_InputConversionSurface                      : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 4
    union{
        uint32_t   DW4_BTI_OutputConversionSurface                 : MOS_BITFIELD_RANGE( 0, 31);
        uint32_t   DW4_BTI_InputDsSurface                          : MOS_BITFIELD_RANGE( 0, 31);
        uint32_t   DW4_BTI_Value                                   : MOS_BITFIELD_RANGE( 0, 31);
    };

    // DWORD 5
    uint32_t   DW5_BTI_4xDsSurface                                 : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 6
    uint32_t   DW6_BTI_MBStatsSurface                              : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 7
    uint32_t   DW7_BTI_2xDsSurface                                 : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 8
    uint32_t   DW8_BTI_MB_Split_Surface                            : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 9
    uint32_t   DW9_BTI_LCU32_JobQueueScratchBufferSurface          : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 10
    uint32_t   DW10_BTI_LCU64_CU32_JobQueueScratchBufferSurface    : MOS_BITFIELD_RANGE( 0, 31);

    // DWORD 11
    uint32_t   DW11_BTI_LCU64_CU32_64x64_DistortionSurface         : MOS_BITFIELD_RANGE( 0, 31);

}MEDIA_OBJECT_DOWNSCALING_CONVERSION_STATIC_DATA_G10, *PMEDIA_OBJECT_DOWNSCALING_CONVERSION_STATIC_DATA_G10;

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(MEDIA_OBJECT_DOWNSCALING_CONVERSION_STATIC_DATA_G10)) == 12);

const MEDIA_OBJECT_DOWNSCALING_CONVERSION_STATIC_DATA_G10 g_cInit_MEDIA_OBJECT_DOWNSCALING_CONVERSION_STATIC_DATA_G10 =
{
    10,             // DW0_InputBitDepthForChroma
    10,             // DW0_InputBitDepthForLuma
    8,              // DW0_OutputBitDepthForChroma
    8,              // DW0_OutputBitDepthForLuma
    1,              // DW0_RoundingEnable
    0,              // DW1_PictureFormat
    0,              // DW1_ConvertFlag
    dsDisabled,     // DW1_DownscaleStage
    0,              // DW1_MbStatisticsDumpFlag
    0,              // DW1_Reserved_0
    0,              // DW1_LcuSize
    2656,           // DW1_JobQueueSize
    0,              // DW2_OriginalPicWidthInSamples
    0,              // DW2_OriginalPicHeightInSamples
    0xffff,         // DW3_BTI_InputConversionSurface
    { 0xffff },       // DW4_BTI_OutputConversionSurface / DW4_BTI_InputDsSurface
    0xffff,         // DW5_BTI_4xDsSurface
    0xffff,         // DW6_BTI_MBStatsSurface
    0xffff,         // DW7_BTI_2xDsSurface
    0xffff,         // DW8_BTI_MB_Split_Surface
    0xffff,         // DW9_BTI_LCU32_JobQueueScratchBufferSurface
    0xffff,         // DW10_BTI_LCU64_CU32_JobQueueScratchBufferSurface
    0xffff          // DW11_BTI_LCU64_CU32_64x64_DistortionSurface
};

typedef struct _MEDIA_OBJECT_SCALING_STATIC_DATA_G95
{
    // uint32_t 0 - GRF R1.0
    union
    {
        struct
        {
            uint32_t   InputPictureWidth                   : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   InputPictureHeight                  : MOS_BITFIELD_RANGE( 16,31 );
        };

        uint32_t       Value;
    } DW0;

    // DW1
    union
    {
        struct
        {
            uint32_t   InputYBTIFrame                      : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   InputYBTITopField                   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    // DW2
    union
    {
        struct
        {
            uint32_t   OutputYBTIFrame                     : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   OutputYBTITopField                  : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    // DW3
    union
    {
        struct
        {
            uint32_t   InputYBTIBottomField                : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW3;

    // DW4
    union
    {
        struct
        {
            uint32_t   OutputYBTIBottomField               : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW4;

    // DW5
    union
    {
        struct
        {
            uint32_t   FlatnessThreshold                   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW5;

    // DW6
    union
    {
        struct
        {
            uint32_t   EnableMBFlatnessCheck               : MOS_BITFIELD_BIT(       0 );
            uint32_t   EnableMBVarianceOutput              : MOS_BITFIELD_BIT(       1 );  
            uint32_t   EnableMBPixelAverageOutput          : MOS_BITFIELD_BIT(       2 );  
            uint32_t   EnableBlock8x8StatisticsOutput      : MOS_BITFIELD_BIT(       3 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  4,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW6;


    // DW7
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  1,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW7;


    // DW8
    union
    {
        struct
        {
            uint32_t   MBVProcStatsBTIFrame                : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   MBVProcStatsBTITopField             : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };

    } DW8;

    // DW9
    union
    {
        struct
        {
            uint32_t   MBVProcStatsBTIBottomField          : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };

    } DW9;

    // DW10
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE( 0,31 );
        };
        struct
        {
            uint32_t   Value;
        };

    } DW10;

    // DW11
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE( 0,31 );
        };
        struct
        {
            uint32_t   Value;
        };

    } DW11;

     //DW12
    union
    {
        struct
        {
            uint32_t   Reserved                             : MOS_BITFIELD_RANGE( 0, 31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW12;

    //DW13
    union
    {
        struct
        {
            uint32_t   Reserved                              : MOS_BITFIELD_RANGE( 0, 31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW13;

    //DW14
    union
    {
        struct
        {
            uint32_t   Reserved                              : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW14;

    //DW15
    union
    {
        struct
        {
            uint32_t   Reserved                               : MOS_BITFIELD_RANGE( 0, 31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW15;

} MEDIA_OBJECT_SCALING_STATIC_DATA_G95, *PMEDIA_OBJECT_SCALING_STATIC_DATA_G95;

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(MEDIA_OBJECT_SCALING_STATIC_DATA_G95)) == 16);

typedef enum _CODECHAL_BINDING_TABLE_OFFSET_HEVC_VP9_VDENC_KERNEL_G10
{
    // VDEnc HME kernel
    CODECHAL_VDENC_HME_BEGIN_G10 = 0,
    CODECHAL_VDENC_HME_MV_DATA_SURFACE_CM_G10 = CODECHAL_VDENC_HME_BEGIN_G10,
    CODECHAL_VDENC_16xME_MV_DATA_SURFACE_CM_G10,
    CODECHAL_VDENC_32xME_MV_DATA_SURFACE_CM_G10 = CODECHAL_VDENC_16xME_MV_DATA_SURFACE_CM_G10,
    CODECHAL_VDENC_HME_DISTORTION_SURFACE_CM_G10,
    CODECHAL_VDENC_HME_BRC_DISTORTION_CM_G10,
    CODECHAL_VDENC_HME_CURR_FOR_FWD_REF_CM_G10,
    CODECHAL_VDENC_HME_FWD_REF_IDX0_CM_G10,
    CODECHAL_VDENC_HME_RESERVED1_CM_G10,
    CODECHAL_VDENC_HME_FWD_REF_IDX1_CM_G10,
    CODECHAL_VDENC_HME_RESERVED2_CM_G10,
    CODECHAL_VDENC_HME_FWD_REF_IDX2_CM_G10,
    CODECHAL_VDENC_HME_RESERVED3_CM_G10,
    CODECHAL_VDENC_HME_FWD_REF_IDX3_CM_G10,
    CODECHAL_VDENC_HME_RESERVED4_CM_G10,
    CODECHAL_VDENC_HME_FWD_REF_IDX4_CM_G10,
    CODECHAL_VDENC_HME_RESERVED5_CM_G10,
    CODECHAL_VDENC_HME_FWD_REF_IDX5_CM_G10,
    CODECHAL_VDENC_HME_RESERVED6_CM_G10,
    CODECHAL_VDENC_HME_FWD_REF_IDX6_CM_G10,
    CODECHAL_VDENC_HME_RESERVED7_CM_G10,
    CODECHAL_VDENC_HME_FWD_REF_IDX7_CM_G10,
    CODECHAL_VDENC_HME_RESERVED8_CM_G10,
    CODECHAL_VDENC_HME_CURR_FOR_BWD_REF_CM_G10,
    CODECHAL_VDENC_HME_BWD_REF_IDX0_CM_G10,
    CODECHAL_VDENC_HME_RESERVED9_CM_G10,
    CODECHAL_VDENC_HME_BWD_REF_IDX1_CM_G10,
    CODECHAL_VDENC_HME_RESERVED10_CM_G10,
    CODECHAL_VDENC_HME_VDENC_STREAMIN_OUTPUT_CM_G10,
    CODECHAL_VDENC_HME_VDENC_STREAMIN_INPUT_CM_G10,
    CODECHAL_VDENC_HME_END_G10,
} CODECHAL_BINDING_TABLE_OFFSET_HEVC_VP9_VDENC_KERNEL_G10;

typedef struct _CODECHAL_HEVC_VP9_VDENC_KERNEL_HEADER_G10 
{
    int nKernelCount;

    union
    {
        struct
        {
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_DS4X_Frame;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_DS4X_Field;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_DS2X_Frame;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_DS2X_Field;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_HME_P;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_HME_B;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_HME_Streamin;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_HME_HEVC_Streamin;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_HMEDetection;
        };
    };

} CODECHAL_HEVC_VP9_VDENC_KERNEL_HEADER_G10, *PCODECHAL_HEVC_VP9_VDENC_KERNEL_HEADER_G10;

typedef struct _MEDIA_OBJECT_HEVC_VP9_VDENC_ME_CURBE_G10
{
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn                          : MOS_BITFIELD_BIT(       0 );
            uint32_t   AdaptiveEn                          : MOS_BITFIELD_BIT(       1 );
            uint32_t   BiMixDis                            : MOS_BITFIELD_BIT(       2 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  3, 4 );
            uint32_t   EarlyImeSuccessEn                   : MOS_BITFIELD_BIT(       5 );
            uint32_t                                       : MOS_BITFIELD_BIT(       6 );
            uint32_t   T8x8FlagForInterEn                  : MOS_BITFIELD_BIT(       7 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  8,23 );
            uint32_t   EarlyImeStop                        : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    // DW1
    union
    {
        struct
        {
            uint32_t   MaxNumMVs                           : MOS_BITFIELD_RANGE(  0, 5 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  6,15 );
            uint32_t   BiWeight                            : MOS_BITFIELD_RANGE( 16,21 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 22,27 );
            uint32_t   UniMixDisable                       : MOS_BITFIELD_BIT(      28 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 29,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    // DW2
    union
    {
        struct
        {
            uint32_t   MaxLenSP                                    : MOS_BITFIELD_RANGE( 0, 7 );
            uint32_t   MaxNumSU                                    : MOS_BITFIELD_RANGE( 8, 15 );
            uint32_t                                               : MOS_BITFIELD_RANGE( 16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    // DW3
    union
    {
        struct
        {
            uint32_t   SrcSize                             : MOS_BITFIELD_RANGE(  0, 1 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  2, 3 );
            uint32_t   MbTypeRemap                         : MOS_BITFIELD_RANGE(  4, 5 );
            uint32_t   SrcAccess                           : MOS_BITFIELD_BIT(       6 );
            uint32_t   RefAccess                           : MOS_BITFIELD_BIT(       7 );
            uint32_t   SearchCtrl                          : MOS_BITFIELD_RANGE(  8,10 );
            uint32_t   DualSearchPathOption                : MOS_BITFIELD_BIT(      11 );
            uint32_t   SubPelMode                          : MOS_BITFIELD_RANGE( 12,13 );
            uint32_t   SkipType                            : MOS_BITFIELD_BIT(      14 );
            uint32_t   DisableFieldCacheAlloc              : MOS_BITFIELD_BIT(      15 );
            uint32_t   InterChromaMode                     : MOS_BITFIELD_BIT(      16 );
            uint32_t   FTEnable                            : MOS_BITFIELD_BIT(      17 );
            uint32_t   BMEDisableFBR                       : MOS_BITFIELD_BIT(      18 );
            uint32_t   BlockBasedSkipEnable                : MOS_BITFIELD_BIT(      19 );
            uint32_t   InterSAD                            : MOS_BITFIELD_RANGE( 20,21 );
            uint32_t   IntraSAD                            : MOS_BITFIELD_RANGE( 22,23 );
            uint32_t   SubMbPartMask                       : MOS_BITFIELD_RANGE( 24,30 );
            uint32_t                                       : MOS_BITFIELD_BIT(      31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW3;

    // DW4
    union
    {
        struct
        {
            uint32_t                                               : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   PictureHeightMinus1                         : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   PictureWidth                                : MOS_BITFIELD_RANGE(16, 23);
            uint32_t                                               : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW4;

    // DW5
    union
    {
        struct
        {
            uint32_t                                               : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   QpPrimeY                                    : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefWidth                                    : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                                   : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW5;

    // DW6
    union
    {
        struct
        {
            uint32_t                                               : MOS_BITFIELD_BIT(0);
            uint32_t   InputStreamInEn                             : MOS_BITFIELD_BIT(1);
            uint32_t   LCUSize                                     : MOS_BITFIELD_BIT(2);
            uint32_t   WriteDistortions                            : MOS_BITFIELD_BIT(3);
            uint32_t   UseMvFromPrevStep                           : MOS_BITFIELD_BIT(4);
            uint32_t                                               : MOS_BITFIELD_RANGE(5, 7);
            uint32_t   SuperCombineDist                            : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MaxVmvR                                     : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW6;

    // DW7
    union
    {
        struct
        {
            uint32_t                                               : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   MVCostScaleFactor                   : MOS_BITFIELD_RANGE( 16,17 );
            uint32_t   BilinearEnable                      : MOS_BITFIELD_BIT(      18 );
            uint32_t   SrcFieldPolarity                    : MOS_BITFIELD_BIT(      19 );
            uint32_t   WeightedSADHAAR                     : MOS_BITFIELD_BIT(      20 );
            uint32_t   AConlyHAAR                          : MOS_BITFIELD_BIT(      21 );
            uint32_t   RefIDCostMode                       : MOS_BITFIELD_BIT(      22 );
            uint32_t                                       : MOS_BITFIELD_BIT(      23 );
            uint32_t   SkipCenterMask                      : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW7;

    // DW8
    union
    {
        struct
        {
            uint32_t   Mode0Cost                           : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode1Cost                           : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Mode2Cost                           : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   Mode3Cost                           : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW8;

    // DW9
    union
    {
        struct
        {
            uint32_t   Mode4Cost                           : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode5Cost                           : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Mode6Cost                           : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   Mode7Cost                           : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW9;

    // DW10
    union
    {
        struct
        {
            uint32_t   Mode8Cost                           : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode9Cost                           : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefIDCost                           : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ChromaIntraModeCost                 : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW10;

    // DW11
    union
    {
        struct
        {
            uint32_t   MV0Cost                             : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MV1Cost                             : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MV2Cost                             : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MV3Cost                             : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW11;

    // DW12
    union
    {
        struct
        {
            uint32_t   MV4Cost                             : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MV5Cost                             : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MV6Cost                             : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MV7Cost                             : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW12;

    // DW13
    union
    {
        struct
        {
            uint32_t   NumRefIdxL0MinusOne                         : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   NumRefIdxL1MinusOne                         : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefStreaminCost                             : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ROIEnable                                   : MOS_BITFIELD_RANGE(24, 26);
            uint32_t                                               : MOS_BITFIELD_RANGE(27, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW13;

    // DW14
    union
    {
        struct
        {
            uint32_t   List0RefID0FieldParity                      : MOS_BITFIELD_BIT(0);
            uint32_t   List0RefID1FieldParity                      : MOS_BITFIELD_BIT(1);
            uint32_t   List0RefID2FieldParity                      : MOS_BITFIELD_BIT(2);
            uint32_t   List0RefID3FieldParity                      : MOS_BITFIELD_BIT(3);
            uint32_t   List0RefID4FieldParity                      : MOS_BITFIELD_BIT(4);
            uint32_t   List0RefID5FieldParity                      : MOS_BITFIELD_BIT(5);
            uint32_t   List0RefID6FieldParity                      : MOS_BITFIELD_BIT(6);
            uint32_t   List0RefID7FieldParity                      : MOS_BITFIELD_BIT(7);
            uint32_t   List1RefID0FieldParity                      : MOS_BITFIELD_BIT(8);
            uint32_t   List1RefID1FieldParity                      : MOS_BITFIELD_BIT(9);
            uint32_t                                               : MOS_BITFIELD_RANGE(10, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW14;

    // DW15
    union
    {
        struct
        {
            uint32_t   PrevMvReadPosFactor                         : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MvShiftFactor                               : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW15;

    struct
    {
        // DW16
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_0;
                SearchPathDelta   SPDelta_1;
                SearchPathDelta   SPDelta_2;
                SearchPathDelta   SPDelta_3;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW16;

        // DW17
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_4;
                SearchPathDelta   SPDelta_5;
                SearchPathDelta   SPDelta_6;
                SearchPathDelta   SPDelta_7;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW17;

        // DW18
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_8;
                SearchPathDelta   SPDelta_9;
                SearchPathDelta   SPDelta_10;
                SearchPathDelta   SPDelta_11;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW18;

        // DW19
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_12;
                SearchPathDelta   SPDelta_13;
                SearchPathDelta   SPDelta_14;
                SearchPathDelta   SPDelta_15;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW19;

        // DW20
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_16;
                SearchPathDelta   SPDelta_17;
                SearchPathDelta   SPDelta_18;
                SearchPathDelta   SPDelta_19;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW20;

        // DW21
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_20;
                SearchPathDelta   SPDelta_21;
                SearchPathDelta   SPDelta_22;
                SearchPathDelta   SPDelta_23;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW21;

        // DW22
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_24;
                SearchPathDelta   SPDelta_25;
                SearchPathDelta   SPDelta_26;
                SearchPathDelta   SPDelta_27;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW22;

        // DW23
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_28;
                SearchPathDelta   SPDelta_29;
                SearchPathDelta   SPDelta_30;
                SearchPathDelta   SPDelta_31;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW23;

        // DW24
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_32;
                SearchPathDelta   SPDelta_33;
                SearchPathDelta   SPDelta_34;
                SearchPathDelta   SPDelta_35;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW24;

        // DW25
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_36;
                SearchPathDelta   SPDelta_37;
                SearchPathDelta   SPDelta_38;
                SearchPathDelta   SPDelta_39;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW25;

        // DW26
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_40;
                SearchPathDelta   SPDelta_41;
                SearchPathDelta   SPDelta_42;
                SearchPathDelta   SPDelta_43;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW26;

        // DW27
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_44;
                SearchPathDelta   SPDelta_45;
                SearchPathDelta   SPDelta_46;
                SearchPathDelta   SPDelta_47;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW27;

        // DW28
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_48;
                SearchPathDelta   SPDelta_49;
                SearchPathDelta   SPDelta_50;
                SearchPathDelta   SPDelta_51;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW28;

        // DW29
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_52;
                SearchPathDelta   SPDelta_53;
                SearchPathDelta   SPDelta_54;
                SearchPathDelta   SPDelta_55;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW29;
    } SPDelta;

    // DW30
    union
    {
        struct
        {
            uint32_t   ActualMBWidth                            : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ActualMBHeight                           : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW30;

    // DW31
    union
    {
        struct
        {
            uint32_t   RoiCtrl                                      : MOS_BITFIELD_RANGE( 0, 7 );
            uint32_t   MaxTuSize                                    : MOS_BITFIELD_RANGE( 8, 9 );
            uint32_t   MaxCuSize                                    : MOS_BITFIELD_RANGE( 10, 11 );
            uint32_t   NumImePredictors                             : MOS_BITFIELD_RANGE( 12, 15 );
            uint32_t   Reserved                                     : MOS_BITFIELD_RANGE( 16, 23 );
            uint32_t   PuTypeCtrl                                   : MOS_BITFIELD_RANGE( 24, 31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW31;

    // DW32
    union
    {
        struct
        {
            uint32_t   ForceMvx0                                    : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy0                                    : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW32;

     // DW33
    union
    {
        struct
        {
            uint32_t   ForceMvx1                                    : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy1                                    : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW33;

    // DW34
    union
    {
        struct
        {
            uint32_t   ForceMvx2                                    : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy2                                    : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW34;

    // DW35
    union
    {
        struct
        {
            uint32_t   ForceMvx3                                    : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ForceMvy3                                    : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW35;

    // DW36
    union
    {
        struct
        {
            uint32_t   ForceRefIdx0                                  : MOS_BITFIELD_RANGE(0, 3);
            uint32_t   ForceRefIdx1                                  : MOS_BITFIELD_RANGE(4, 7);
            uint32_t   ForceRefIdx2                                  : MOS_BITFIELD_RANGE(8, 11);
            uint32_t   ForceRefIdx3                                  : MOS_BITFIELD_RANGE(12, 15);
            uint32_t   NumMergeCandCu8x8                             : MOS_BITFIELD_RANGE(16, 19);
            uint32_t   NumMergeCandCu16x16                           : MOS_BITFIELD_RANGE(20, 23);
            uint32_t   NumMergeCandCu32x32                           : MOS_BITFIELD_RANGE(24, 27);
            uint32_t   NumMergeCandCu64x64                           : MOS_BITFIELD_RANGE(28, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW36;

    // DW37
    union
    {
        struct
        {
            uint32_t   SegID                                          : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   QpEnable                                       : MOS_BITFIELD_RANGE(16, 19);
            uint32_t   SegIDEnable                                    : MOS_BITFIELD_BIT(20);
            uint32_t   Reserved                                       : MOS_BITFIELD_RANGE(21, 22);
            uint32_t   ForceRefIdEnable                               : MOS_BITFIELD_BIT(23);
            uint32_t   Reserved1                                      : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW37;

    // DW38
    union
    {
        struct
        {
            uint32_t   ForceQp0                                       : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   ForceQp1                                       : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   ForceQp2                                       : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ForceQp3                                       : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW38;

     // DW39
    union
    {
        struct
        {
            uint32_t   Reserved                                       : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW39;

     // DW40
    union
    {
        struct
        {
            uint32_t   _4xMeMvOutputDataSurfIndex                  : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW40;

    // DW41
    union
    {
        struct
        {
            uint32_t   _16xOr32xMeMvInputDataSurfIndex              : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW41;

    // DW42
    union
    {
        struct
        {
            uint32_t   _4xMeOutputDistSurfIndex                    : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW42;

    // DW43
    union
    {
        struct
        {
            uint32_t   _4xMeOutputBrcDistSurfIndex                  : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW43;

    // DW44
    union
    {
        struct
        {
            uint32_t   VMEFwdInterPredictionSurfIndex              : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW44;

    // DW45
    union
    {
        struct
        {
            uint32_t   VMEBwdInterPredictionSurfIndex              : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW45;

    // DW46
    union
    {
        struct
        {
            uint32_t   VDEncStreamInOutputSurfIndex                 : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW46;

    // DW47
    union
    {
        struct
        {
            uint32_t   VDEncStreamInInputSurfIndex                   : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW47;
} MEDIA_OBJECT_HEVC_VP9_VDENC_ME_CURBE_G10, *PMEDIA_OBJECT_HEVC_VP9_VDENC_ME_CURBE_G10;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(MEDIA_OBJECT_HEVC_VP9_VDENC_ME_CURBE_G10)) == 48);

typedef struct _CODECHAL_ENCODE_VDENC_ME_STATE
{
    PCODEC_REF_LIST           pRefList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC];
    CODEC_PIC_ID              PicIdx[CODEC_MAX_NUM_REF_FRAME_HEVC];
    MOS_SURFACE               s16xMeMvDataBuffer;
    MOS_SURFACE               s4xMeMvDataBuffer;
    MOS_SURFACE               s32xMeMvDataBuffer;
    MOS_SURFACE               s4xMeDistortionBuffer;
    uint8_t                   Level;
    uint16_t                  direct_spatial_mv_pred_flag;
    uint32_t                  dwBiWeight;
    bool                      bFirstFieldIdrPic;
    bool                      bMbaff;
    bool                      b16xMeInUse;
    bool                      b4xMeInUse;

    //Sequence Params
    uint8_t                   TargetUsage;
    uint8_t                   GopRefDist;

    //Picture Params
    CODEC_PICTURE             CurrOriginalPic;
    char                      QpY;

    //Slice Params
    CODEC_PICTURE             RefPicList[2][CODEC_MAX_NUM_REF_FRAME_HEVC];
    uint8_t                   num_ref_idx_l0_active_minus1;   // [0..15]
    uint8_t                   num_ref_idx_l1_active_minus1;   // [0..15]
    char                      slice_qp_delta;
}CODECHAL_ENCODE_VDENC_ME_STATE, *PCODECHAL_ENCODE_VDENC_ME_STATE;

#endif  // __CODECHAL_ENCODER_G10_H__
