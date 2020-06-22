/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file     codechal_encode_avc_g9.h
//! \brief    This file defines the C++ class/interface for Gen9 platform's AVC
//!           DualPipe encoding to be used across CODECHAL components.
//!

#ifndef __CODECHAL_ENCODE_AVC_G9_H__
#define __CODECHAL_ENCODE_AVC_G9_H__

#include "codechal_encode_avc.h"

#define CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_G9                32
#define CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_RESERVED_G9       32
#define CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_1_G9                32
#define CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_1_RESERVED_G9       160

#define CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_INTRACOST_SCALING_FACTOR_G9 64
#define CODECHAL_ENCODE_AVC_BRC_HISTORY_BUFFER_SIZE_G9                      864

#define CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_WIDTH_G9                    64
#define CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_HEIGHT_G9                   44

#define CODECHAL_ENCODE_AVC_MFE_MBENC_KERNEL_IDX                            27

#define CODECHAL_ENCODE_AVC_ADAPTIVE_TX_DECISION_THRESHOLD_G9           128
#define CODECHAL_ENCODE_AVC_MB_TEXTURE_THRESHOLD_G9                     1024
#define CODECHAL_ENCODE_AVC_SFD_COST_TABLE_BUFFER_SIZE_G9               52

// Unified curbe size for both legacy and fei.
#define CODECHAL_ENCODE_AVC_MBENC_CURBE_SIZE_IN_DWORD_G9                104

#define CODECHAL_ENCODE_AVC_MFE_MAX_FRAMES_G9                           4

typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_ME_CM_G9
{
    CODECHAL_ENCODE_AVC_ME_MV_DATA_SURFACE_CM_G9       = 0,
    CODECHAL_ENCODE_AVC_16xME_MV_DATA_SURFACE_CM_G9    = 1,
    CODECHAL_ENCODE_AVC_32xME_MV_DATA_SURFACE_CM_G9    = 1,
    CODECHAL_ENCODE_AVC_ME_DISTORTION_SURFACE_CM_G9    = 2,
    CODECHAL_ENCODE_AVC_ME_BRC_DISTORTION_CM_G9        = 3,
    CODECHAL_ENCODE_AVC_ME_RESERVED0_CM_G9             = 4,
    CODECHAL_ENCODE_AVC_ME_CURR_FOR_FWD_REF_CM_G9      = 5,
    CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX0_CM_G9          = 6,
    CODECHAL_ENCODE_AVC_ME_RESERVED1_CM_G9             = 7,
    CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX1_CM_G9          = 8,
    CODECHAL_ENCODE_AVC_ME_RESERVED2_CM_G9             = 9,
    CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX2_CM_G9          = 10,
    CODECHAL_ENCODE_AVC_ME_RESERVED3_CM_G9             = 11,
    CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX3_CM_G9          = 12,
    CODECHAL_ENCODE_AVC_ME_RESERVED4_CM_G9             = 13,
    CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX4_CM_G9          = 14,
    CODECHAL_ENCODE_AVC_ME_RESERVED5_CM_G9             = 15,
    CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX5_CM_G9          = 16,
    CODECHAL_ENCODE_AVC_ME_RESERVED6_CM_G9             = 17,
    CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX6_CM_G9          = 18,
    CODECHAL_ENCODE_AVC_ME_RESERVED7_CM_G9             = 19,
    CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX7_CM_G9          = 20,
    CODECHAL_ENCODE_AVC_ME_RESERVED8_CM_G9             = 21,
    CODECHAL_ENCODE_AVC_ME_CURR_FOR_BWD_REF_CM_G9      = 22,
    CODECHAL_ENCODE_AVC_ME_BWD_REF_IDX0_CM_G9          = 23,
    CODECHAL_ENCODE_AVC_ME_RESERVED9_CM_G9             = 24,
    CODECHAL_ENCODE_AVC_ME_BWD_REF_IDX1_CM_G9          = 25,
    CODECHAL_ENCODE_AVC_ME_VDENC_STREAMIN_CM_G9        = 26,
    CODECHAL_ENCODE_AVC_ME_NUM_SURFACES_CM_G9          = 27
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_ME_CM_G9;

typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_MBENC_G9
{
    CODECHAL_ENCODE_AVC_MBENC_MFC_AVC_PAK_OBJ_G9                    =  0,
    CODECHAL_ENCODE_AVC_MBENC_IND_MV_DATA_G9                        =  1,
    CODECHAL_ENCODE_AVC_MBENC_BRC_DISTORTION_G9                     =  2,    // For BRC distortion for I
    CODECHAL_ENCODE_AVC_MBENC_CURR_Y_G9                             =  3,
    CODECHAL_ENCODE_AVC_MBENC_CURR_UV_G9                            =  4,
    CODECHAL_ENCODE_AVC_MBENC_MB_SPECIFIC_DATA_G9                   =  5,
    CODECHAL_ENCODE_AVC_MBENC_AUX_VME_OUT_G9                        =  6,
    CODECHAL_ENCODE_AVC_MBENC_REFPICSELECT_L0_G9                    =  7,
    CODECHAL_ENCODE_AVC_MBENC_MV_DATA_FROM_ME_G9                    =  8,
    CODECHAL_ENCODE_AVC_MBENC_4xME_DISTORTION_G9                    =  9,
    CODECHAL_ENCODE_AVC_MBENC_SLICEMAP_DATA_G9                      = 10,
    CODECHAL_ENCODE_AVC_MBENC_FWD_MB_DATA_G9                        = 11,
    CODECHAL_ENCODE_AVC_MBENC_FWD_MV_DATA_G9                        = 12,
    CODECHAL_ENCODE_AVC_MBENC_MBQP_G9                               = 13,
    CODECHAL_ENCODE_AVC_MBENC_MBBRC_CONST_DATA_G9                   = 14,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_G9      = 15,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX0_G9        = 16,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_G9      = 17,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX1_G9        = 18,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_G9      = 19,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX2_G9        = 20,
    CODECHAL_ENCODE_AVC_MBENC_RESERVED0_G9                          = 21,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX3_G9        = 22,
    CODECHAL_ENCODE_AVC_MBENC_RESERVED1_G9                          = 23,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX4_G9        = 24,
    CODECHAL_ENCODE_AVC_MBENC_RESERVED2_G9                          = 25,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX5_G9        = 26,
    CODECHAL_ENCODE_AVC_MBENC_RESERVED3_G9                          = 27,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX6_G9        = 28,
    CODECHAL_ENCODE_AVC_MBENC_RESERVED4_G9                          = 29,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX7_G9        = 30,
    CODECHAL_ENCODE_AVC_MBENC_RESERVED5_G9                          = 31,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_G9      = 32,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_G9      = 33,
    CODECHAL_ENCODE_AVC_MBENC_RESERVED6_G9                          = 34,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_G9      = 35,
    CODECHAL_ENCODE_AVC_MBENC_RESERVED7_G9                          = 36,
    CODECHAL_ENCODE_AVC_MBENC_MB_STATS_G9                           = 37,
    CODECHAL_ENCODE_AVC_MBENC_MAD_DATA_G9                           = 38,
    CODECHAL_ENCODE_AVC_MBENC_FORCE_NONSKIP_MB_MAP_G9               = 39,
    CODECHAL_ENCODE_AVC_MBENC_ADV_WA_G9                             = 40,
    CODECHAL_ENCODE_AVC_MBENC_BRC_CURBE_DATA_G9                     = 41,
    CODECHAL_ENCODE_AVC_MBENC_SFD_COST_TABLE_G9                     = 42,
    CODECHAL_ENCODE_AVC_MBENC_MV_PREDICTOR_G9                       = 43,
    CODECHAL_ENCODE_AVC_MBENC_NUM_SURFACES_G9                       = 44
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_MBENC_G9;

typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_FRAME_BRC_UPDATE_G9
{
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_HISTORY_G9                = 0,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_PAK_STATISTICS_OUTPUT_G9  = 1,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_IMAGE_STATE_READ_G9       = 2,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_IMAGE_STATE_WRITE_G9      = 3,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_MBENC_CURBE_READ_G9       = 4,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_MBENC_CURBE_WRITE_G9      = 5,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_DISTORTION_G9             = 6,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_CONSTANT_DATA_G9          = 7,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_MB_STAT_G9                = 8,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_NUM_SURFACES_G9           = 9
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_FRAME_BRC_UPDATE_G9;

typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_MB_BRC_UPDATE_G9
{
    CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_HISTORY_G9               = 0,
    CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_MB_QP_G9                 = 1,
    CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_ROI_G9                   = 2,
    CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_MB_STAT_G9               = 3,
    CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_NUM_SURFACES_G9          = 4
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_MB_BRC_UPDATE_G9;

typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_WP_G9
{
    CODECHAL_ENCODE_AVC_WP_INPUT_REF_SURFACE_G9                 = 0,
    CODECHAL_ENCODE_AVC_WP_OUTPUT_SCALED_SURFACE_G9             = 1,
    CODECHAL_ENCODE_AVC_WP_NUM_SURFACES_G9                      = 2
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_WP_G9;

typedef struct _CODECHAL_ENCODE_AVC_ME_CURBE_CM_G9
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
            uint32_t   MaxLenSP                                    : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MaxNumSU                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t                                               : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   RefWidth                            : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                           : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t                                               : MOS_BITFIELD_RANGE(0, 2);
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
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   _4xMeMvOutputDataSurfIndex                  : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   _16xOr32xMeMvInputDataSurfIndex              : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   _4xMeOutputDistSurfIndex                    : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   _4xMeOutputBrcDistSurfIndex                  : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   VMEFwdInterPredictionSurfIndex              : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   VMEBwdInterPredictionSurfIndex              : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   VDEncStreamInSurfIndex                      : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW38;

} CODECHAL_ENCODE_AVC_ME_CURBE_CM_G9, *PCODECHAL_ENCODE_AVC_ME_CURBE_CM_G9;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_ME_CURBE_CM_G9)) == 39);

typedef struct _CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_COMMON
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
            uint32_t   LenSP                               : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MaxNumSU                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   PicWidth                            : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   PicHeightMinus1                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   MvRestrictionInSliceEnable          : MOS_BITFIELD_BIT(      16 );
            uint32_t   DeltaMvEnable                       : MOS_BITFIELD_BIT(      17 );
            uint32_t   TrueDistortionEnable                : MOS_BITFIELD_BIT(      18 );
            uint32_t   EnableWavefrontOptimization         : MOS_BITFIELD_BIT(      19 );
            uint32_t   EnableFBRBypass                     : MOS_BITFIELD_BIT(      20 );
            uint32_t   EnableIntraCostScalingForStaticFrame: MOS_BITFIELD_BIT(      21 );
            uint32_t                                       : MOS_BITFIELD_BIT(      22 );
            uint32_t   Reserved                            : MOS_BITFIELD_BIT(      23 );
            uint32_t   EnableDirtyRect                     : MOS_BITFIELD_BIT(      24 );
            uint32_t   bCurFldIDR                          : MOS_BITFIELD_BIT(      25 );
            uint32_t   ConstrainedIntraPredFlag            : MOS_BITFIELD_BIT(      26 );
            uint32_t   FieldParityFlag                     : MOS_BITFIELD_BIT(      27 );
            uint32_t   HMEEnable                           : MOS_BITFIELD_BIT(      28 );
            uint32_t   PictureType                         : MOS_BITFIELD_RANGE( 29,30 );
            uint32_t   UseActualRefQPValue                 : MOS_BITFIELD_BIT(      31 );
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
            uint32_t   SliceMbHeight                       : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   RefWidth                            : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                           : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   BatchBufferEnd                      : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   IntraPartMask                       : MOS_BITFIELD_RANGE(  0, 4 );
            uint32_t   NonSkipZMvAdded                     : MOS_BITFIELD_BIT(       5 );
            uint32_t   NonSkipModeAdded                    : MOS_BITFIELD_BIT(       6 );
            uint32_t   LumaIntraSrcCornerSwap              : MOS_BITFIELD_BIT(       7 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  8,15 );
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
            uint32_t   QpPrimeY                            : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   QpPrimeCb                           : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   QpPrimeCr                           : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   TargetSizeInWord                    : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   SICFwdTransCoeffThreshold_0         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   SICFwdTransCoeffThreshold_1         : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   SICFwdTransCoeffThreshold_2         : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   SICFwdTransCoeffThreshold_3         : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   SICFwdTransCoeffThreshold_4         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   SICFwdTransCoeffThreshold_5         : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   SICFwdTransCoeffThreshold_6         : MOS_BITFIELD_RANGE( 24,31 );    // Highest Freq
        };
        struct
        {
            uint32_t   Value;
        };
    } DW15;

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

    // DW30
    union
    {
        struct
        {
            uint32_t   Intra4x4ModeMask                    : MOS_BITFIELD_RANGE(  0, 8 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  9,15 );
            uint32_t   Intra8x8ModeMask                    : MOS_BITFIELD_RANGE( 16,24 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 25,31 );
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
            uint32_t   Intra16x16ModeMask                  : MOS_BITFIELD_RANGE(  0, 3 );
            uint32_t   IntraChromaModeMask                 : MOS_BITFIELD_RANGE(  4, 7 );
            uint32_t   IntraComputeType                    : MOS_BITFIELD_RANGE(  8, 9 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 10,31 );
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
            uint32_t   SkipVal                             : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   MultiPredL0Disable                  : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MultiPredL1Disable                  : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   Intra16x16NonDCPredPenalty          : MOS_BITFIELD_RANGE(  0,7 );
            uint32_t   Intra8x8NonDCPredPenalty            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Intra4x4NonDCPredPenalty            : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   List0RefID0FieldParity              : MOS_BITFIELD_BIT(       0 );
            uint32_t   List0RefID1FieldParity              : MOS_BITFIELD_BIT(       1 );
            uint32_t   List0RefID2FieldParity              : MOS_BITFIELD_BIT(       2 );
            uint32_t   List0RefID3FieldParity              : MOS_BITFIELD_BIT(       3 );
            uint32_t   List0RefID4FieldParity              : MOS_BITFIELD_BIT(       4 );
            uint32_t   List0RefID5FieldParity              : MOS_BITFIELD_BIT(       5 );
            uint32_t   List0RefID6FieldParity              : MOS_BITFIELD_BIT(       6 );
            uint32_t   List0RefID7FieldParity              : MOS_BITFIELD_BIT(       7 );
            uint32_t   List1RefID0FrameFieldFlag           : MOS_BITFIELD_BIT(       8 );
            uint32_t   List1RefID1FrameFieldFlag           : MOS_BITFIELD_BIT(       9 );
            uint32_t   IntraRefreshEn                      : MOS_BITFIELD_RANGE( 10,11 );
            uint32_t   ArbitraryNumMbsPerSlice             : MOS_BITFIELD_BIT(      12 );
            uint32_t   EnableAdaptiveTxDecision            : MOS_BITFIELD_BIT(      13 );
            uint32_t   ForceNonSkipMbEnable                : MOS_BITFIELD_BIT(      14 );
            uint32_t   DisableEncSkipCheck                 : MOS_BITFIELD_BIT(      15 );
            uint32_t   EnableDirectBiasAdjustment          : MOS_BITFIELD_BIT(      16 );
            uint32_t   bForceToSkip                        : MOS_BITFIELD_BIT(      17 );
            uint32_t   EnableGlobalMotionBiasAdjustment    : MOS_BITFIELD_BIT(      18 );
            uint32_t   EnableAdaptiveSearchWindowSize      : MOS_BITFIELD_BIT(      19 );
            uint32_t   EnablePerMBStaticCheck              : MOS_BITFIELD_BIT(      20 );
            uint32_t   RemoveIntraRefreshOverlap           : MOS_BITFIELD_BIT(      21 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 22,23 );
            uint32_t   List1RefID0FieldParity              : MOS_BITFIELD_BIT(      24 );
            uint32_t   List1RefID1FieldParity              : MOS_BITFIELD_BIT(      25 );
            uint32_t   MADEnableFlag                       : MOS_BITFIELD_BIT(      26 );
            uint32_t   ROIEnableFlag                       : MOS_BITFIELD_BIT(      27 );
            uint32_t   EnableMBFlatnessChkOptimization     : MOS_BITFIELD_BIT(      28 );
            uint32_t   bDirectMode                         : MOS_BITFIELD_BIT(      29 );
            uint32_t   MBBrcEnable                         : MOS_BITFIELD_BIT(      30 );
            uint32_t   bOriginalBff                        : MOS_BITFIELD_BIT(      31 );
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
            uint32_t   PanicModeMBThreshold                : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   SmallMbSizeInWord                   : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   LargeMbSizeInWord                   : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   NumRefIdxL0MinusOne                 : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   HMECombinedExtraSUs                 : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   NumRefIdxL1MinusOne                 : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 24,27 );
            uint32_t   IsFwdFrameShortTermRef              : MOS_BITFIELD_BIT(      28 );
            uint32_t   CheckAllFractionalEnable            : MOS_BITFIELD_BIT(      29 );
            uint32_t   HMECombineOverlap                   : MOS_BITFIELD_RANGE( 30,31 );
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
    } DW37;

    // DW38
    union
    {
        struct
        {
            uint32_t   LenSP                               : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MaxNumSU                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefThreshold                        : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t                                       : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   HMERefWindowsCombThreshold          : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefWidth                            : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                           : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   DistScaleFactorRefID0List0          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID1List0          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   DistScaleFactorRefID2List0          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID3List0          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   DistScaleFactorRefID4List0          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID5List0          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   DistScaleFactorRefID6List0          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID7List0          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ActualQPValueForRefID0List0         : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID1List0         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ActualQPValueForRefID2List0         : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ActualQPValueForRefID3List0         : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   ActualQPValueForRefID4List0         : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID5List0         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ActualQPValueForRefID6List0         : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ActualQPValueForRefID7List0         : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   ActualQPValueForRefID0List1         : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID1List1         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefCost                             : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW46;

    // DW47
    union
    {
        struct
        {
            uint32_t   MbQpReadFactor                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   IntraCostSF                         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MaxVmvR                             : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW47;

    //DW48
    union
    {
        struct
        {
            uint32_t   IntraRefreshMBNum                   : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   IntraRefreshUnitInMBMinus1          : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   IntraRefreshQPDelta                 : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW48;

    // DW49
    union
    {
        struct
        {
            uint32_t   ROI1_X_left                         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1_Y_top                          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW49;

    // DW50
    union
    {
        struct
        {
            uint32_t   ROI1_X_right                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1_Y_bottom                       : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW50;

    // DW51
    union
    {
        struct
        {
            uint32_t   ROI2_X_left                         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2_Y_top                          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW51;

    // DW52
    union
    {
        struct
        {
            uint32_t   ROI2_X_right                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2_Y_bottom                       : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW52;

    // DW53
    union
    {
        struct
        {
            uint32_t   ROI3_X_left                         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3_Y_top                          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW53;

    // DW54
    union
    {
        struct
        {
            uint32_t   ROI3_X_right                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3_Y_bottom                       : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW54;

    // DW55
    union
    {
        struct
        {
            uint32_t   ROI4_X_left                         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4_Y_top                          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW55;

    // DW56
    union
    {
        struct
        {
            uint32_t   ROI4_X_right                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4_Y_bottom                       : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW56;

    // DW57
    union
    {
        struct
        {
            uint32_t   ROI1_dQpPrimeY                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ROI2_dQpPrimeY                      : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ROI3_dQpPrimeY                      : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ROI4_dQpPrimeY                      : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW57;

    // DW58
    union
    {
        struct
        {
            uint32_t   MBTextureThreshold                  : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   TxDecisonThreshold                  : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW58;

    // DW59
    union
    {
        struct
        {
            uint32_t   HMEMVCostScalingFactor              : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  8,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW59;

    // DW60
    union
    {
        struct
        {
            uint32_t   IPCM_QP0                            : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   IPCM_QP1                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   IPCM_QP2                            : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   IPCM_QP3                            : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW60;

    // DW61
    union
    {
        struct
        {
            uint32_t   IPCM_QP4                            : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   IPCM_Thresh0                        : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW61;

    // DW62
    union
    {
        struct
        {
            uint32_t   IPCM_Thresh1                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   IPCM_Thresh2                        : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW62;

    // DW63
    union
    {
        struct
        {
            uint32_t   IPCM_Thresh3                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   IPCM_Thresh4                        : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW63;

    // DW64
    union
    {
        struct
        {
            uint32_t   NumMVPredictorsL0                   : MOS_BITFIELD_RANGE(  0, 3 );
            uint32_t   Reserved1                           : MOS_BITFIELD_BIT(       4 );
            uint32_t   Reserved2                           : MOS_BITFIELD_BIT(       5 );
            uint32_t   VMEDistortionOutputEnable           : MOS_BITFIELD_BIT(       6 );
            uint32_t   PerMBQpEnable                       : MOS_BITFIELD_BIT(       7 );
            uint32_t   MBInputEnable                       : MOS_BITFIELD_BIT(       8 );
            uint32_t   FEIMode                             : MOS_BITFIELD_BIT(       9 );
            uint32_t   NumMVPredictorsL1                   : MOS_BITFIELD_RANGE( 10,13 );
            uint32_t   DefaultMVCompare                    : MOS_BITFIELD_RANGE( 14,15 );
            uint32_t   NumberOfMVPBipredCalls              : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   EnableColorBleedWAforIntraSlice     : MOS_BITFIELD_BIT(      24 );
            uint32_t   L1ListRef0PictureCodingType         : MOS_BITFIELD_RANGE( 25,26 ); // PAFF WA Fix
            uint32_t   Reserved3                           : MOS_BITFIELD_RANGE( 27,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW64;

    // DW65
    union
    {
        struct
        {
            uint32_t   FlatnessThreshold                   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW65;

    // DW66
    union
    {
        struct
        {
            uint32_t   BottomFieldOffsetL1ListRef0MV       : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW66;

    // DW67
    union
    {
        struct
        {
            uint32_t   BottomFieldOffsetL1ListRef0MBCode   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW67;

    // DW68
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW68;

    // DW69
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW69;

    // DW70
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW70;

    // DW71
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW71;

    // DW72
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW72;

    // DW73
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW73;

    // DW74
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW74;

    // DW75
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW75;

    // DW76
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW76;

    // DW77
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW77;

    // DW78
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW78;

    // DW79
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW79;
} CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_COMMON, *PCODECHAL_ENCODE_AVC_MBENC_CURBE_G9_COMMON;

typedef struct _CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_SURFACES
{
    // DW80
    union
    {
        struct
        {
            uint32_t   MBDataSurfIndex                     : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW80;

    // DW81
    union
    {
        struct
        {
            uint32_t   MVDataSurfIndex                     : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW81;

    // DW82
    union
    {
        struct
        {
            uint32_t   IDistSurfIndex                      : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW82;

    // DW83
    union
    {
        struct
        {
            uint32_t   SrcYSurfIndex                       : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW83;

    // DW84
    union
    {
        struct
        {
            uint32_t   MBSpecificDataSurfIndex             : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW84;

    // DW85
    union
    {
        struct
        {
            uint32_t   AuxVmeOutSurfIndex                  : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW85;

    // DW86
    union
    {
        struct
        {
            uint32_t   CurrRefPicSelSurfIndex              : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW86;

    // DW87
    union
    {
        struct
        {
            uint32_t   HMEMVPredFwdBwdSurfIndex            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW87;

    // DW88
    union
    {
        struct
        {
            uint32_t   HMEDistSurfIndex                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW88;

    // DW89
    union
    {
        struct
        {
            uint32_t   SliceMapSurfIndex                   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW89;

    // DW90
    union
    {
        struct
        {
            uint32_t   FwdFrmMBDataSurfIndex               : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW90;

    // DW91
    union
    {
        struct
        {
            uint32_t   FwdFrmMVSurfIndex                   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW91;

    // DW92
    union
    {
        struct
        {
            uint32_t   MBQPBuffer                          : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW92;

    // DW93
    union
    {
        struct
        {
            uint32_t   MBBRCLut                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW93;

    // DW94
    union
    {
        struct
        {
            uint32_t   VMEInterPredictionSurfIndex         : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW94;

    // DW95
    union
    {
        struct
        {
            uint32_t   VMEInterPredictionMRSurfIndex       : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW95;

    // DW96
    union
    {
        struct
        {
            uint32_t   MbStatsSurfIndex                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW96;

    // DW97
    union
    {
        struct
        {
            uint32_t   MADSurfIndex                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW97;

    // DW98
    union
    {
        struct
        {
            uint32_t   ForceNonSkipMBmapSurface            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW98;

    // DW99
    union
    {
        struct
        {
            uint32_t   ReservedIndex                       : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW99;

    // DW100
    union
    {
        struct
        {
            uint32_t   BRCCurbeSurfIndex                   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW100;

    // DW101
    union
    {
        struct
        {
            uint32_t   StaticDetectionCostTableIndex       : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW101;

    // DW102
    union
    {
        struct
        {
            uint32_t   FEIMVPredictorSurfIndex             : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW102;

    // DW103
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW103;
} CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_SURFACES, *PCODECHAL_ENCODE_AVC_MBENC_CURBE_G9_SURFACES;

typedef struct _CODECHAL_ENCODE_AVC_MBENC_CURBE_G9
{
    CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_COMMON common;
    CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_SURFACES surfaces;
} CODECHAL_ENCODE_AVC_MBENC_CURBE_G9, *PCODECHAL_ENCODE_AVC_MBENC_CURBE_G9;

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9)) == CODECHAL_ENCODE_AVC_MBENC_CURBE_SIZE_IN_DWORD_G9);

typedef struct _CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9
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
            uint32_t   LenSP                               : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MaxNumSU                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   PicWidth                            : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   PicHeightMinus1                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   MvRestrictionInSliceEnable          : MOS_BITFIELD_BIT(      16 );
            uint32_t   DeltaMvEnable                       : MOS_BITFIELD_BIT(      17 );
            uint32_t   TrueDistortionEnable                : MOS_BITFIELD_BIT(      18 );
            uint32_t   EnableWavefrontOptimization         : MOS_BITFIELD_BIT(      19 );
            uint32_t   EnableFBRBypass                     : MOS_BITFIELD_BIT(      20 );
            uint32_t   EnableIntraCostScalingForStaticFrame: MOS_BITFIELD_BIT(      21 );
            uint32_t                                       : MOS_BITFIELD_BIT(      22 );
            uint32_t   Reserved                            : MOS_BITFIELD_BIT(      23 );
            uint32_t   EnableDirtyRect                     : MOS_BITFIELD_BIT(      24 );
            uint32_t   bCurFldIDR                          : MOS_BITFIELD_BIT(      25 );
            uint32_t   ConstrainedIntraPredFlag            : MOS_BITFIELD_BIT(      26 );
            uint32_t   FieldParityFlag                     : MOS_BITFIELD_BIT(      27 );
            uint32_t   HMEEnable                           : MOS_BITFIELD_BIT(      28 );
            uint32_t   PictureType                         : MOS_BITFIELD_RANGE( 29,30 );
            uint32_t   UseActualRefQPValue                 : MOS_BITFIELD_BIT(      31 );
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
            uint32_t   SliceMbHeight                       : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   RefWidth                            : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                           : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   BatchBufferEnd                      : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   IntraPartMask                       : MOS_BITFIELD_RANGE(  0, 4 );
            uint32_t   NonSkipZMvAdded                     : MOS_BITFIELD_BIT(       5 );
            uint32_t   NonSkipModeAdded                    : MOS_BITFIELD_BIT(       6 );
            uint32_t   LumaIntraSrcCornerSwap              : MOS_BITFIELD_BIT(       7 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  8,15 );
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

    struct
    {
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
                uint32_t   QpPrimeY                            : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   QpPrimeCb                           : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   QpPrimeCr                           : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   TargetSizeInWord                    : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   SICFwdTransCoeffThreshold_0         : MOS_BITFIELD_RANGE(  0,15 );
                uint32_t   SICFwdTransCoeffThreshold_1         : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   SICFwdTransCoeffThreshold_2         : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   SICFwdTransCoeffThreshold_3         : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   SICFwdTransCoeffThreshold_4         : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   SICFwdTransCoeffThreshold_5         : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   SICFwdTransCoeffThreshold_6         : MOS_BITFIELD_RANGE( 24,31 );    // Highest Freq
            };
            struct
            {
                uint32_t   Value;
            };
        } DW15;
    } ModeMvCost;

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

        // DW30
        union
        {
            struct
            {
                uint32_t   Intra4x4ModeMask                    : MOS_BITFIELD_RANGE(  0, 8 );
                uint32_t                                       : MOS_BITFIELD_RANGE(  9,15 );
                uint32_t   Intra8x8ModeMask                    : MOS_BITFIELD_RANGE( 16,24 );
                uint32_t                                       : MOS_BITFIELD_RANGE( 25,31 );
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
                uint32_t   Intra16x16ModeMask                  : MOS_BITFIELD_RANGE(  0, 3 );
                uint32_t   IntraChromaModeMask                 : MOS_BITFIELD_RANGE(  4, 7 );
                uint32_t   IntraComputeType                    : MOS_BITFIELD_RANGE(  8, 9 );
                uint32_t                                       : MOS_BITFIELD_RANGE( 10,31 );
            };
            struct
            {
                uint32_t   Value;
            };
        } DW31;
    } SPDelta;

    // DW32
    union
    {
        struct
        {
            uint32_t   SkipVal                             : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   MultiPredL0Disable                  : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MultiPredL1Disable                  : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   Intra16x16NonDCPredPenalty          : MOS_BITFIELD_RANGE(  0,7 );
            uint32_t   Intra8x8NonDCPredPenalty            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Intra4x4NonDCPredPenalty            : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   List0RefID0FieldParity              : MOS_BITFIELD_BIT(       0 );
            uint32_t   List0RefID1FieldParity              : MOS_BITFIELD_BIT(       1 );
            uint32_t   List0RefID2FieldParity              : MOS_BITFIELD_BIT(       2 );
            uint32_t   List0RefID3FieldParity              : MOS_BITFIELD_BIT(       3 );
            uint32_t   List0RefID4FieldParity              : MOS_BITFIELD_BIT(       4 );
            uint32_t   List0RefID5FieldParity              : MOS_BITFIELD_BIT(       5 );
            uint32_t   List0RefID6FieldParity              : MOS_BITFIELD_BIT(       6 );
            uint32_t   List0RefID7FieldParity              : MOS_BITFIELD_BIT(       7 );
            uint32_t   List1RefID0FrameFieldFlag           : MOS_BITFIELD_BIT(       8 );
            uint32_t   List1RefID1FrameFieldFlag           : MOS_BITFIELD_BIT(       9 );
            uint32_t   IntraRefreshEn                      : MOS_BITFIELD_RANGE( 10,11 );
            uint32_t   ArbitraryNumMbsPerSlice             : MOS_BITFIELD_BIT(      12 );
            uint32_t   EnableAdaptiveTxDecision            : MOS_BITFIELD_BIT(      13 );
            uint32_t   ForceNonSkipMbEnable                : MOS_BITFIELD_BIT(      14 );
            uint32_t   DisableEncSkipCheck                 : MOS_BITFIELD_BIT(      15 );
            uint32_t   EnableDirectBiasAdjustment          : MOS_BITFIELD_BIT(      16 );
            uint32_t   bForceToSkip                        : MOS_BITFIELD_BIT(      17 );
            uint32_t   EnableGlobalMotionBiasAdjustment    : MOS_BITFIELD_BIT(      18 );
            uint32_t   EnableAdaptiveSearchWindowSize      : MOS_BITFIELD_BIT(      19 );
            uint32_t   EnablePerMBStaticCheck              : MOS_BITFIELD_BIT(      20 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 21,23 );
            uint32_t   List1RefID0FieldParity              : MOS_BITFIELD_BIT(      24 );
            uint32_t   List1RefID1FieldParity              : MOS_BITFIELD_BIT(      25 );
            uint32_t   MADEnableFlag                       : MOS_BITFIELD_BIT(      26 );
            uint32_t   ROIEnableFlag                       : MOS_BITFIELD_BIT(      27 );
            uint32_t   EnableMBFlatnessChkOptimization     : MOS_BITFIELD_BIT(      28 );
            uint32_t   bDirectMode                         : MOS_BITFIELD_BIT(      29 );
            uint32_t   MBBrcEnable                         : MOS_BITFIELD_BIT(      30 );
            uint32_t   bOriginalBff                        : MOS_BITFIELD_BIT(      31 );
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
            uint32_t   PanicModeMBThreshold                : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   SmallMbSizeInWord                   : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   LargeMbSizeInWord                   : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   NumRefIdxL0MinusOne                 : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   HMECombinedExtraSUs                 : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   NumRefIdxL1MinusOne                 : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 24,27 );
            uint32_t   IsFwdFrameShortTermRef              : MOS_BITFIELD_BIT(      28 );
            uint32_t   CheckAllFractionalEnable            : MOS_BITFIELD_BIT(      29 );
            uint32_t   HMECombineOverlap                   : MOS_BITFIELD_RANGE( 30,31 );
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
    } DW37;

    // DW38
    union
    {
        struct
        {
            uint32_t   LenSP                               : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MaxNumSU                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefThreshold                        : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t                                       : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   HMERefWindowsCombThreshold          : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefWidth                            : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                           : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   DistScaleFactorRefID0List0          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID1List0          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   DistScaleFactorRefID2List0          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID3List0          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   DistScaleFactorRefID4List0          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID5List0          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   DistScaleFactorRefID6List0          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID7List0          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ActualQPValueForRefID0List0         : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID1List0         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ActualQPValueForRefID2List0         : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ActualQPValueForRefID3List0         : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   ActualQPValueForRefID4List0         : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID5List0         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ActualQPValueForRefID6List0         : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ActualQPValueForRefID7List0         : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   ActualQPValueForRefID0List1         : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID1List1         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefCost                             : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW46;

    // DW47
    union
    {
        struct
        {
            uint32_t   MbQpReadFactor                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   IntraCostSF                         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MaxVmvR                             : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW47;

    //DW48
    union
    {
        struct
        {
            uint32_t   IntraRefreshMBNum                   : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   IntraRefreshUnitInMBMinus1          : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   IntraRefreshQPDelta                 : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW48;

    // DW49
    union
    {
        struct
        {
            uint32_t   ROI1_X_left                         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1_Y_top                          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW49;

    // DW50
    union
    {
        struct
        {
            uint32_t   ROI1_X_right                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1_Y_bottom                       : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW50;

    // DW51
    union
    {
        struct
        {
            uint32_t   ROI2_X_left                         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2_Y_top                          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW51;

    // DW52
    union
    {
        struct
        {
            uint32_t   ROI2_X_right                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2_Y_bottom                       : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW52;

    // DW53
    union
    {
        struct
        {
            uint32_t   ROI3_X_left                         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3_Y_top                          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW53;

    // DW54
    union
    {
        struct
        {
            uint32_t   ROI3_X_right                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3_Y_bottom                       : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW54;

    // DW55
    union
    {
        struct
        {
            uint32_t   ROI4_X_left                         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4_Y_top                          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW55;

    // DW56
    union
    {
        struct
        {
            uint32_t   ROI4_X_right                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4_Y_bottom                       : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW56;

    // DW57
    union
    {
        struct
        {
            uint32_t   ROI1_dQpPrimeY                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ROI2_dQpPrimeY                      : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ROI3_dQpPrimeY                      : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ROI4_dQpPrimeY                      : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW57;

    // DW58
    union
    {
        struct
        {
            uint32_t   MBTextureThreshold                  : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   TxDecisonThreshold                  : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW58;

    // DW59
    union
    {
        struct
        {
            uint32_t   HMEMVCostScalingFactor              : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  8,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW59;

    // DW60
    union
    {
        struct
        {
            uint32_t   IPCM_QP0                            : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   IPCM_QP1                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   IPCM_QP2                            : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   IPCM_QP3                            : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW60;

    // DW61
    union
    {
        struct
        {
            uint32_t   IPCM_QP4                            : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   IPCM_Thresh0                        : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW61;

    // DW62
    union
    {
        struct
        {
            uint32_t   IPCM_Thresh1                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   IPCM_Thresh2                        : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW62;

    // DW63
    union
    {
        struct
        {
            uint32_t   IPCM_Thresh3                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   IPCM_Thresh4                        : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW63;

    // DW64
    union
    {
        struct
        {
            uint32_t   NumMVPredictorsL0                   : MOS_BITFIELD_RANGE(  0, 3 );
            uint32_t   FEIEnable                           : MOS_BITFIELD_BIT(       4 );
            uint32_t   MultipleMVPredictorPerMBEnable      : MOS_BITFIELD_BIT(       5 );
            uint32_t   VMEDistortionOutputEnable           : MOS_BITFIELD_BIT(       6 );
            uint32_t   PerMBQpEnable                       : MOS_BITFIELD_BIT(       7 );
            uint32_t   MBInputEnable                       : MOS_BITFIELD_BIT(       8 );
            uint32_t   FEIMode                             : MOS_BITFIELD_BIT(       9 );
            uint32_t   NumMVPredictorsL1                   : MOS_BITFIELD_RANGE( 10,13 );
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE( 14,24 );
            uint32_t   L1ListRef0PictureCodingType         : MOS_BITFIELD_RANGE( 25,26 );  // PAFF WA Fix, 0-invalid, 1-TFF, 2-invalid, 3-BFF
            uint32_t   Reserved1                           : MOS_BITFIELD_RANGE( 27,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW64;

    // DW65
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW65;

    // DW66
    union
    {
        struct
        {
            uint32_t   BottomFieldOffsetL1ListRef0MV       : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW66;

     // DW67
    union
    {
        struct
        {
            uint32_t   BottomFieldOffsetL1ListRef0MBCode   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW67;

    // DW68
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW68;

    // DW69
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW69;

    // DW70
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW70;

   // DW71
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW71;

     // DW72
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW72;

     // DW73
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW73;

    // DW74
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW74;

    // DW75
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW75;

     // DW76
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW76;

     // DW77
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW77;

     // DW78
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW78;

     // DW79
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW79;

    // DW80
    union
    {
        struct
        {
            uint32_t   MBDataSurfIndex                     : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW80;

    // DW81
    union
    {
        struct
        {
            uint32_t   MVDataSurfIndex                     : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW81;

    // DW82
    union
    {
        struct
        {
            uint32_t   IDistSurfIndex                      : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW82;

    // DW83
    union
    {
        struct
        {
            uint32_t   SrcYSurfIndex                       : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW83;

    // DW84
    union
    {
        struct
        {
            uint32_t   MBSpecificDataSurfIndex             : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW84;

    // DW85
    union
    {
        struct
        {
            uint32_t   AuxVmeOutSurfIndex                  : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW85;

    // DW86
    union
    {
        struct
        {
            uint32_t   CurrRefPicSelSurfIndex              : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW86;

    // DW87
    union
    {
        struct
        {
            uint32_t   HMEMVPredFwdBwdSurfIndex            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW87;

    // DW88
    union
    {
        struct
        {
            uint32_t   HMEDistSurfIndex                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW88;

    // DW89
    union
    {
        struct
        {
            uint32_t   SliceMapSurfIndex                   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW89;

    // DW90
    union
    {
        struct
        {
            uint32_t   FwdFrmMBDataSurfIndex               : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW90;

    // DW91
    union
    {
        struct
        {
            uint32_t   FwdFrmMVSurfIndex                   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW91;

    // DW92
    union
    {
        struct
        {
            uint32_t   MBQPBuffer                          : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW92;

    // DW93
    union
    {
        struct
        {
            uint32_t   MBBRCLut                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW93;

    // DW94
    union
    {
        struct
        {
            uint32_t   VMEInterPredictionSurfIndex         : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW94;

    // DW95
    union
    {
        struct
        {
            uint32_t   VMEInterPredictionMRSurfIndex       : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW95;

    // DW96
    union
    {
        struct
        {
            uint32_t   MbStatsSurfIndex                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW96;

    // DW97
    union
    {
        struct
        {
            uint32_t   MADSurfIndex                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW97;

    // DW98
    union
    {
        struct
        {
            uint32_t   ForceNonSkipMBmapSurface            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW98;

    // DW99
    union
    {
        struct
        {
            uint32_t   ReservedIndex                       : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW99;

    // DW100
    union
    {
        struct
        {
            uint32_t   BRCCurbeSurfIndex                   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW100;

    // DW101
    union
    {
        struct
        {
            uint32_t   StaticDetectionCostTableIndex       : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW101;

    // DW102
    union
    {
        struct
        {
            uint32_t   FEIMVPredictorSurfIndex             : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW102;

    // DW103
    union
    {
        struct
        {
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW103;
} CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9, *PCODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9;

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9)) == CODECHAL_ENCODE_AVC_MBENC_CURBE_SIZE_IN_DWORD_G9);
// AVC Gen 9 WP kernel CURBE, defined in Gen9 AVC WP Kernel
typedef struct _CODECHAL_ENCODE_AVC_WP_CURBE_G9
{
    // DW0
    union
    {
        struct
        {
            uint32_t   DefaultWeight       : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DefaultOffset       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI0_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI0_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI0_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI0_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI0Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI0Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI1_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI1_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI1Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI2_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI2_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI2Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI3_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI3_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI3Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI4_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI4_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI4Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4Offset          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW15;

    // DW16
    union
    {
        struct
        {
            uint32_t   ROI5_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI5_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI5_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI5_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI5Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI5Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI6_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI6_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI6_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI6_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI6Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI6Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI7_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI7_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI7_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI7_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI7Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI7Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI8_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI8_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI8_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI8_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI8Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI8Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI9_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI9_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI9_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI9_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW29;

    // DW30
    union
    {
        struct
        {
            uint32_t   ROI9Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI9Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI10_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI10_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI10_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI10_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI10Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI10Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI11_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI11_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI11_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI11_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI11Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI11Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI12_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI12_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI12_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI12_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI12Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI12Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI13_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI13_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI13_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI13_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI13Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI13Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI14_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI14_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI14_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI14_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI14Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI14Offset          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI15_X_left         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI15_Y_top          : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI15_X_right        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI15_Y_bottom       : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW47;

    // DW48
    union
    {
        struct
        {
            uint32_t   ROI15Weight          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI15Offset          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW48;

    // DW49
    union
    {
        struct
        {
            uint32_t   Log2WeightDenom      : MOS_BITFIELD_RANGE(  0,2  );
            uint32_t   reserve1             : MOS_BITFIELD_RANGE(  3,7  );
            uint32_t   ROI_enabled          : MOS_BITFIELD_RANGE(  8,8  );
            uint32_t   reserve2             : MOS_BITFIELD_RANGE(  9,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW49;

    // DW50
    union
    {
        struct
        {
            uint32_t   InputSurface          : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW50;

    // DW51
    union
    {
        struct
        {
            uint32_t   OutputSurface         : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW51;

} CODECHAL_ENCODE_AVC_WP_CURBE_G9, *PCODECHAL_ENCODE_AVC_WP_CURBE_G9;

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_WP_CURBE_G9)) == 52);

typedef struct _CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_CURBE_G9
{
    union
    {
        struct
        {
            uint32_t   TargetSize                                  : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    union
    {
        struct
        {
            uint32_t   FrameNumber                                 : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    union
    {
        struct
        {
            uint32_t   SizeofPicHeaders                            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    union
    {
        struct
        {
            uint32_t   startGAdjFrame0                             : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   startGAdjFrame1                             : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW3;

    union
    {
        struct
        {
            uint32_t   startGAdjFrame2                             : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   startGAdjFrame3                             : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW4;

    union
    {
        struct
        {
            uint32_t   TargetSizeFlag                              : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   BRCFlag                                     : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MaxNumPAKs                                  : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   CurrFrameType                               : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW5;

    union
    {
        struct
        {
            uint32_t   NumSkipFrames                               : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MinimumQP                                   : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MaximumQP                                   : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   EnableForceToSkip                           : MOS_BITFIELD_BIT(      24 );
            uint32_t   EnableSlidingWindow                         : MOS_BITFIELD_BIT(      25 );
            uint32_t   EnableExtremLowDelay                        : MOS_BITFIELD_BIT(      26 );
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE( 27,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW6;

    union
    {
        struct
        {
            uint32_t    SizeSkipFrames                             : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW7;

    union
    {
        struct
        {
            uint32_t   StartGlobalAdjustMult0                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   StartGlobalAdjustMult1                      : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   StartGlobalAdjustMult2                      : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   StartGlobalAdjustMult3                      : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW8;

    union
    {
        struct
        {
            uint32_t   StartGlobalAdjustMult4                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   StartGlobalAdjustDiv0                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   StartGlobalAdjustDiv1                       : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   StartGlobalAdjustDiv2                       : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW9;

    union
    {
        struct
        {
            uint32_t   StartGlobalAdjustDiv3                       : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   StartGlobalAdjustDiv4                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   QPThreshold0                                : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   QPThreshold1                                : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW10;

    union
    {
        struct
        {
            uint32_t   QPThreshold2                                : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   QPThreshold3                                : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   gRateRatioThreshold0                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   gRateRatioThreshold1                        : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW11;

    union
    {
        struct
        {
            uint32_t   gRateRatioThreshold2                        : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   gRateRatioThreshold3                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   gRateRatioThreshold4                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   gRateRatioThreshold5                        : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW12;

    union
    {
        struct
        {
            uint32_t   gRateRatioThresholdQP0                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   gRateRatioThresholdQP1                      : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   gRateRatioThresholdQP2                      : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   gRateRatioThresholdQP3                      : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW13;

    union
    {
        struct
        {
            uint32_t   gRateRatioThresholdQP4                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   gRateRatioThresholdQP5                      : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   gRateRatioThresholdQP6                      : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   QPIndexOfCurPic                             : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW14;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,7  );
            uint32_t   EnableROI                                   : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RoundingIntra                               : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RoundingInter                               : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW15;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW16;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW17;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW18;

    union
    {
        struct
        {
            uint32_t   UserMaxFrame                                : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW19;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW20;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW21;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW22;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW23;
} CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_CURBE_G9, *PCODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_CURBE_G9;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(_CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_CURBE_G9)) == 24);

typedef struct _CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_CURBE_G9
{
    union
    {
        struct
        {
            uint32_t   CurrFrameType                               : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   EnableROI                                   : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ROIRatio                                    : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW3;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW4;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW5;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW6;
} CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_CURBE_G9, *PCODECHAL_ENCODE_AVC_MB_BRC_UPDATE_CURBE_G9;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_CURBE_G9)) == 7);

typedef struct _CODECHAL_ENCODE_AVC_KERNEL_HEADER_G9 {
    int nKernelCount;

    // Quality mode for Frame/Field
    CODECHAL_KERNEL_HEADER AVCMBEnc_Qlty_I;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Qlty_P;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Qlty_B;
    // Normal mode for Frame/Field
    CODECHAL_KERNEL_HEADER AVCMBEnc_Norm_I;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Norm_P;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Norm_B;
    // Performance modes for Frame/Field
    CODECHAL_KERNEL_HEADER AVCMBEnc_Perf_I;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Perf_P;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Perf_B;
    // Modes for Frame/Field
    CODECHAL_KERNEL_HEADER AVCMBEnc_Adv_I;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Adv_P;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Adv_B;

    // HME
    CODECHAL_KERNEL_HEADER AVC_ME_P;
    CODECHAL_KERNEL_HEADER AVC_ME_B;

    // DownScaling
    CODECHAL_KERNEL_HEADER PLY_DScale_PLY;
    CODECHAL_KERNEL_HEADER PLY_DScale_2f_PLY_2f;

    // BRC Init frame
    CODECHAL_KERNEL_HEADER InitFrameBRC;

    // FrameBRC Update
    CODECHAL_KERNEL_HEADER FrameENCUpdate;

    // BRC Reset frame
    CODECHAL_KERNEL_HEADER BRC_ResetFrame;

    // BRC I Frame Distortion
    CODECHAL_KERNEL_HEADER BRC_IFrame_Dist;

    // BRCBlockCopy
    CODECHAL_KERNEL_HEADER BRCBlockCopy;

    // MbBRC Update
    CODECHAL_KERNEL_HEADER MbBRCUpdate;

    // 2x DownScaling
    CODECHAL_KERNEL_HEADER PLY_2xDScale_PLY;
    CODECHAL_KERNEL_HEADER PLY_2xDScale_2f_PLY_2f;

    //Motion estimation kernel for the VDENC StreamIN
    CODECHAL_KERNEL_HEADER AVC_ME_VDENC;

    //Weighted Prediction Kernel
    CODECHAL_KERNEL_HEADER AVC_WeightedPrediction;

    // Static frame detection Kernel
    CODECHAL_KERNEL_HEADER AVC_StaticFrameDetection;

    // MFE MBEnc kernel
    CODECHAL_KERNEL_HEADER AVCMBEnc_Qlty_MFE;
} CODECHAL_ENCODE_AVC_KERNEL_HEADER_G9, *PCODECHAL_ENCODE_AVC_KERNEL_HEADER_G9;

typedef struct _CODECHAL_ENCODE_AVC_KERNEL_HEADER_FEI_G9 {
    int nKernelCount;

    // MbEnc FEI for Frame/Field
    CODECHAL_KERNEL_HEADER AVCMBEnc_Fei_I;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Fei_P;
    CODECHAL_KERNEL_HEADER AVCMBEnc_Fei_B;
    // PreProc
    CODECHAL_KERNEL_HEADER AVC_Fei_ProProc;
    // HME
    CODECHAL_KERNEL_HEADER AVC_ME_P;
    CODECHAL_KERNEL_HEADER AVC_ME_B;
    // DownScaling
    CODECHAL_KERNEL_HEADER PLY_DScale_PLY;
    CODECHAL_KERNEL_HEADER PLY_DScale_2f_PLY_2f;
    // BRC_I Frame Distortion
    CODECHAL_KERNEL_HEADER BRC_IFrame_Dist;
    // 2x DownScaling
    CODECHAL_KERNEL_HEADER PLY_2xDScale_PLY;
    CODECHAL_KERNEL_HEADER PLY_2xDScale_2f_PLY_2f;
    //Weighted Prediction Kernel
    CODECHAL_KERNEL_HEADER AVC_WeightedPrediction;
} CODECHAL_ENCODE_AVC_KERNEL_HEADER_FEI_G9, *PCODECHAL_ENCODE_AVC_KERNEL_HEADER_FEI_G9;

typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_MBENC_CM_G9
{
    CODECHAL_ENCODE_AVC_MBENC_MFC_AVC_PAK_OBJ_CM_G9 = 0,
    CODECHAL_ENCODE_AVC_MBENC_IND_MV_DATA_CM_G9 = 1,
    CODECHAL_ENCODE_AVC_MBENC_BRC_DISTORTION_CM_G9 = 2,    // For BRC distortion for I
    CODECHAL_ENCODE_AVC_MBENC_CURR_Y_CM_G9 = 3,
    CODECHAL_ENCODE_AVC_MBENC_CURR_UV_CM_G9 = 4,
    CODECHAL_ENCODE_AVC_MBENC_MB_SPECIFIC_DATA_CM_G9 = 5,
    CODECHAL_ENCODE_AVC_MBENC_AUX_VME_OUT_CM_G9 = 6,
    CODECHAL_ENCODE_AVC_MBENC_REFPICSELECT_L0_CM_G9 = 7,
    CODECHAL_ENCODE_AVC_MBENC_MV_DATA_FROM_ME_CM_G9 = 8,
    CODECHAL_ENCODE_AVC_MBENC_4xME_DISTORTION_CM_G9 = 9,
    CODECHAL_ENCODE_AVC_MBENC_REFPICSELECT_L1_CM_G9 = 10,
    CODECHAL_ENCODE_AVC_MBENC_FWD_MB_DATA_CM_G9 = 11,
    CODECHAL_ENCODE_AVC_MBENC_FWD_MV_DATA_CM_G9 = 12,
    CODECHAL_ENCODE_AVC_MBENC_MBQP_CM_G9 = 13,
    CODECHAL_ENCODE_AVC_MBENC_MBBRC_CONST_DATA_CM_G9 = 14,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_CM_G9 = 15,
    CODECHAL_ENCODE_AVC_MBENC_RESERVED0_CM_G9 = 21,
    CODECHAL_ENCODE_AVC_MBENC_RESERVED1_CM_G9 = 23,
    CODECHAL_ENCODE_AVC_MBENC_RESERVED2_CM_G9 = 25,
    CODECHAL_ENCODE_AVC_MBENC_RESERVED3_CM_G9 = 27,
    CODECHAL_ENCODE_AVC_MBENC_RESERVED4_CM_G9 = 29,
    CODECHAL_ENCODE_AVC_MBENC_RESERVED5_CM_G9 = 31,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_CM_G9 = 32,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_CM_G9 = 33,
    CODECHAL_ENCODE_AVC_MBENC_RESERVED6_CM_G9 = 34,
    CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_CM_G9 = 35,
    CODECHAL_ENCODE_AVC_MBENC_RESERVED7_CM_G9 = 36,
    CODECHAL_ENCODE_AVC_MBENC_FLATNESS_CHECK_CM_G9 = 37,
    CODECHAL_ENCODE_AVC_MBENC_MAD_DATA_CM_G9 = 38,
    CODECHAL_ENCODE_AVC_MBENC_INTER_DISTORTION_CM_G9 = 39,
    CODECHAL_ENCODE_AVC_MBENC_BEST_INTER_INTRA_CM_G9 = 40,
    CODECHAL_ENCODE_AVC_MBENC_BRC_CURBE_DATA_CM_G9 = 41,
    CODECHAL_ENCODE_AVC_MBENC_MV_PREDICTOR_CM_G9 = 42,
    CODECHAL_ENCODE_AVC_MBENC_NUM_SURFACES_CM_G9 = 43
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_MBENC_CM_G9;

typedef struct _CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G9
{
    union
    {
        struct
        {
            uint32_t   ProfileLevelMaxFrame : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    union
    {
        struct
        {
            uint32_t   InitBufFullInBits : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    union
    {
        struct
        {
            uint32_t   BufSizeInBits : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    union
    {
        struct
        {
            uint32_t   AverageBitRate : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW3;

    union
    {
        struct
        {
            uint32_t   MaxBitRate : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW4;

    union
    {
        struct
        {
            uint32_t   MinBitRate : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW5;

    union
    {
        struct
        {
            uint32_t   FrameRateM : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW6;

    union
    {
        struct
        {
            uint32_t   FrameRateD : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW7;

    union
    {
        struct
        {
            uint32_t   BRCFlag : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   GopP : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW8;

    union
    {
        struct
        {
            uint32_t   GopB : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   FrameWidthInBytes : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW9;

    union
    {
        struct
        {
            uint32_t   FrameHeightInBytes : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   AVBRAccuracy : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW10;

    union
    {
        struct
        {
            uint32_t   AVBRConvergence : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   MinQP : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW11;

    union
    {
        struct
        {
            uint32_t   MaxQP : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   NoSlices : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW12;

    union
    {
        struct
        {
            uint32_t   InstantRateThreshold0ForP : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   InstantRateThreshold1ForP : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   InstantRateThreshold2ForP : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   InstantRateThreshold3ForP : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW13;

    union
    {
        struct
        {
            uint32_t   InstantRateThreshold0ForB : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   InstantRateThreshold1ForB : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   InstantRateThreshold2ForB : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   InstantRateThreshold3ForB : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW14;

    union
    {
        struct
        {
            uint32_t   InstantRateThreshold0ForI : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   InstantRateThreshold1ForI : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   InstantRateThreshold2ForI : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   InstantRateThreshold3ForI : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW15;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold0ForPandB : MOS_BITFIELD_RANGE(0, 7);     // Signed byte
            uint32_t   DeviationThreshold1ForPandB : MOS_BITFIELD_RANGE(8, 15);     // Signed byte
            uint32_t   DeviationThreshold2ForPandB : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   DeviationThreshold3ForPandB : MOS_BITFIELD_RANGE(24, 31);     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW16;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold4ForPandB : MOS_BITFIELD_RANGE(0, 7);     // Signed byte
            uint32_t   DeviationThreshold5ForPandB : MOS_BITFIELD_RANGE(8, 15);     // Signed byte
            uint32_t   DeviationThreshold6ForPandB : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   DeviationThreshold7ForPandB : MOS_BITFIELD_RANGE(24, 31);     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW17;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold0ForVBR : MOS_BITFIELD_RANGE(0, 7);     // Signed byte
            uint32_t   DeviationThreshold1ForVBR : MOS_BITFIELD_RANGE(8, 15);     // Signed byte
            uint32_t   DeviationThreshold2ForVBR : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   DeviationThreshold3ForVBR : MOS_BITFIELD_RANGE(24, 31);     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW18;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold4ForVBR : MOS_BITFIELD_RANGE(0, 7);     // Signed byte
            uint32_t   DeviationThreshold5ForVBR : MOS_BITFIELD_RANGE(8, 15);     // Signed byte
            uint32_t   DeviationThreshold6ForVBR : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   DeviationThreshold7ForVBR : MOS_BITFIELD_RANGE(24, 31);     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW19;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold0ForI : MOS_BITFIELD_RANGE(0, 7);        // Signed byte
            uint32_t   DeviationThreshold1ForI : MOS_BITFIELD_RANGE(8, 15);       // Signed byte
            uint32_t   DeviationThreshold2ForI : MOS_BITFIELD_RANGE(16, 23);      // Signed byte
            uint32_t   DeviationThreshold3ForI : MOS_BITFIELD_RANGE(24, 31);      // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW20;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold4ForI : MOS_BITFIELD_RANGE(0, 7);      // Signed byte
            uint32_t   DeviationThreshold5ForI : MOS_BITFIELD_RANGE(8, 15);      // Signed byte
            uint32_t   DeviationThreshold6ForI : MOS_BITFIELD_RANGE(16, 23);      // Signed byte
            uint32_t   DeviationThreshold7ForI : MOS_BITFIELD_RANGE(24, 31);      // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW21;

    union
    {
        struct
        {
            uint32_t   InitialQPForI : MOS_BITFIELD_RANGE(0, 7);     // Signed byte
            uint32_t   InitialQPForP : MOS_BITFIELD_RANGE(8, 15);     // Signed byte
            uint32_t   InitialQPForB : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   SlidingWindowSize : MOS_BITFIELD_RANGE(24, 31);     // unsigned byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW22;

    union
    {
        struct
        {
            uint32_t   ACQP : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW23;
} CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G9, *PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G9;

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G9)) == 24);

class CodechalEncodeAvcEncG9 : public CodechalEncodeAvcEnc
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalEncodeAvcEncG9(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    ~CodechalEncodeAvcEncG9();

    virtual MOS_STATUS InitializeState() override;

    virtual MOS_STATUS InitMbBrcConstantDataBuffer(
        PCODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS params) override;

    virtual MOS_STATUS InitKernelStateWP() override;

    virtual MOS_STATUS GetMbEncKernelStateIdx(
        CodechalEncodeIdOffsetParams*          params,
        uint32_t*                              kernelOffset) override;

    virtual MOS_STATUS SetCurbeAvcWP(
        PCODECHAL_ENCODE_AVC_WP_CURBE_PARAMS params) override;

    virtual MOS_STATUS SceneChangeReport(
        PMOS_COMMAND_BUFFER       cmdBuffer,
        PCODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS   params) override;

    virtual MOS_STATUS SetCurbeAvcBrcInitReset(
        PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS params) override;

    virtual MOS_STATUS SetCurbeAvcFrameBrcUpdate(
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params) override;

    virtual MOS_STATUS SetCurbeAvcMbBrcUpdate(
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params) override;

    virtual MOS_STATUS SetCurbeAvcBrcBlockCopy(
        PCODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_PARAMS params) override;

    virtual MOS_STATUS SendAvcMbEncSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS params) override;

    virtual MOS_STATUS SendAvcWPSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_WP_SURFACE_PARAMS params) override;

    virtual MOS_STATUS SendAvcBrcFrameUpdateSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params) override;

    virtual MOS_STATUS SendAvcBrcMbUpdateSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params) override;

    virtual MOS_STATUS SetupROISurface() override;

    virtual bool IsMfeMbEncEnabled(bool mbEncIFrameDistInUse = false) override;

    MOS_STATUS GetStatusReport(
        void *status,
        uint16_t numStatus) override;

#if USE_CODECHAL_DEBUG_TOOL
protected:
    virtual MOS_STATUS PopulateBrcInitParam(
        void *cmd) override;

    virtual MOS_STATUS PopulateBrcUpdateParam(
        void *cmd) override;

    virtual MOS_STATUS PopulateEncParam(
        uint8_t meMethod,
        void    *cmd) override;
#endif

protected:
    static const CODECHAL_ENCODE_AVC_IPCM_THRESHOLD IPCM_Threshold_Table[5];
    static const uint32_t IntraModeCostForHighTextureMB[CODEC_AVC_NUM_QP];
};

#endif  // __CODECHAL_ENCODE_AVC_G9_H__
