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
//! \file     codechal_fei_avc_g8.cpp
//! \brief    This file implements the C++ class/interface for Gen8 platform's AVC
//!           FEI encoding to be used across CODECHAL components.
//!

#include "codechal_fei_avc_g8.h"
#include "codeckrnheader.h"
#include "igcodeckrn_g8.h"

#ifdef FEI_ENABLE_CMRT
static const char *                 strDsIsaName        = "/opt/intel/mediasdk/lib64/hme_downscale_gen8.isa";
static const char *                 strMbEncIsaName     = "/opt/intel/mediasdk/lib64/AVCEncKernel_BDW_genx.isa";
static const char *                 strPreProcIsaName   = "/opt/intel/mediasdk/lib64/FEI_gen8.isa";
static const char *                 strMeIsaName        = "/opt/intel/mediasdk/lib64/hme_gen8.isa";
#endif

#define CODECHAL_ENCODE_AVC_MAX_NUM_REF_L0                      4
#define CODECHAL_ENCODE_AVC_MAX_NUM_REF_L1                      2

//this enum should be moved into base
typedef enum _BINDING_TABLE_OFFSET_2xSCALING_CM
{
    SCALING_2x_FRAME_SRC_Y_CM = 0,
    SCALING_2x_FRAME_DST_Y_CM = 1,
    SCALING_2x_FIELD_TOP_SRC_Y_CM = 0,
    SCALING_2x_FIELD_TOP_DST_Y_CM = 1,
    SCALING_2x_FIELD_BOT_SRC_Y_CM = 2,
    SCALINGL_2x_FIELD_BOT_DST_Y_CM = 3,
    SCALING_2x_NUM_SURFACES_CM = 4
}BINDING_TABLE_OFFSET_2xSCALING_CM;

typedef struct _KERNEL_HEADER_FEI_CM {
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
    // 2x DownScaling
    CODECHAL_KERNEL_HEADER PLY_2xDScale_PLY;
    CODECHAL_KERNEL_HEADER PLY_2xDScale_2f_PLY_2f;
    //Weighted Prediction Kernel
    CODECHAL_KERNEL_HEADER AVC_WeightedPrediction;
} KERNEL_HEADER_FEI_CM, *PKERNEL_HEADER_FEI_CM;

static const CODECHAL_ENCODE_AVC_IPCM_THRESHOLD g_cInit_CODECHAL_ENCODE_AVC_IPCM_Threshold_Table_G8[5] =
{
    { 2,    3000 },
    { 4,    3600 },
    { 6,    5000 },
    { 10,   8000 },
    { 18,   9000 },
};

typedef enum _MBENC_BINDING_TABLE_OFFSET_CM_FEI
{
    MBENC_MFC_AVC_PAK_OBJ_CM_FEI                   =  0,
    MBENC_IND_MV_DATA_CM_FEI                       =  1,
    MBENC_BRC_DISTORTION_CM_FEI                    =  2,    // For BRC distortion for I
    MBENC_CURR_Y_CM_FEI                            =  3,
    MBENC_CURR_UV_CM_FEI                           =  4,
    MBENC_MB_SPECIFIC_DATA_CM_FEI                  =  5,
    MBENC_AUX_VME_OUT_CM_FEI                       =  6,
    MBENC_REFPICSELECT_L0_CM_FEI                   =  7,
    MBENC_MV_DATA_FROM_ME_CM_FEI                   =  8,
    MBENC_4xME_DISTORTION_CM_FEI                   =  9,
    MBENC_REFPICSELECT_L1_CM_FEI                   = 10,
    MBENC_FWD_MB_DATA_CM_FEI                       = 11,
    MBENC_FWD_MV_DATA_CM_FEI                       = 12,
    MBENC_MBQP_CM_FEI                              = 13,
    MBENC_MBBRC_CONST_DATA_CM_FEI                  = 14,
    MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_CM_FEI     = 15,
    MBENC_RESERVED0_CM_FEI                         = 21,
    MBENC_RESERVED1_CM_FEI                         = 23,
    MBENC_RESERVED2_CM_FEI                         = 25,
    MBENC_RESERVED3_CM_FEI                         = 27,
    MBENC_RESERVED4_CM_FEI                         = 29,
    MBENC_RESERVED5_CM_FEI                         = 31,
    MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_CM_FEI     = 32,
    MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_CM_FEI     = 33,
    MBENC_RESERVED6_CM_FEI                         = 34,
    MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_CM_FEI     = 35,
    MBENC_RESERVED7_CM_FEI                         = 36,
    MBENC_FLATNESS_CHECK_CM_FEI                    = 37,
    MBENC_MAD_DATA_CM_FEI                          = 38,
    MBENC_INTER_DISTORTION_CM_FEI                  = 39,
    MBENC_BEST_INTER_INTRA_CM_FEI                  = 40,
    MBENC_BRC_CURBE_DATA_CM_FEI                    = 41,
    MBENC_MV_PREDICTOR_CM_FEI                      = 42,
    MBENC_NUM_SURFACES_CM_FEI                      = 43
} MBENC_BINDING_TABLE_OFFSET_CM_FEI;

//should be moved into fei base use base!
typedef struct _MEDIA_OBJECT_2xSCALING_STATIC_DATA_CM_FEI_G8
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
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  1,31 );
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
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  1,31 );
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
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  1,31 );
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
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  1,31 );
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
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  1,31 );
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
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  1,31 );
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
    } DW8;

    // DW9
    union
    {
        struct
        {
            uint32_t   OutputYBTIFrame                      : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   OutputYBTITopField                   : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   InputYBTIBottomField                   : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   OutputYBTIBottomField                   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW11;
} MEDIA_OBJECT_2xSCALING_STATIC_DATA_CM_FEI_G8, *PMEDIA_OBJECT_2xSCALING_STATIC_DATA_CM_FEI_G8;

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(MEDIA_OBJECT_2xSCALING_STATIC_DATA_CM_FEI_G8)) == 12);
//different with g9, but seem as legacy
typedef struct _ME_CURBE_CM_FEI
{
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn                      : MOS_BITFIELD_BIT(       0 );
            uint32_t   AdaptiveEn                      : MOS_BITFIELD_BIT(       1 );
            uint32_t   BiMixDis                        : MOS_BITFIELD_BIT(       2 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  3, 4 );
            uint32_t   EarlyImeSuccessEn               : MOS_BITFIELD_BIT(       5 );
            uint32_t                                   : MOS_BITFIELD_BIT(       6 );
            uint32_t   T8x8FlagForInterEn              : MOS_BITFIELD_BIT(       7 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  8,23 );
            uint32_t   EarlyImeStop                    : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW0;

    // DW1
    union
    {
        struct
        {
            uint32_t   MaxNumMVs                       : MOS_BITFIELD_RANGE(  0, 5 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  6,15 );
            uint32_t   BiWeight                        : MOS_BITFIELD_RANGE( 16,21 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 22,27 );
            uint32_t   UniMixDisable                   : MOS_BITFIELD_BIT(      28 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 29,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW1;

    // DW2
    union
    {
        struct
        {
            uint32_t   LenSP                           : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MaxNumSU                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW2;

    // DW3
    union
    {
        struct
        {
            uint32_t   SrcSize                         : MOS_BITFIELD_RANGE(  0, 1 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  2, 3 );
            uint32_t   MbTypeRemap                     : MOS_BITFIELD_RANGE(  4, 5 );
            uint32_t   SrcAccess                       : MOS_BITFIELD_BIT(       6 );
            uint32_t   RefAccess                       : MOS_BITFIELD_BIT(       7 );
            uint32_t   SearchCtrl                      : MOS_BITFIELD_RANGE(  8,10 );
            uint32_t   DualSearchPathOption            : MOS_BITFIELD_BIT(      11 );
            uint32_t   SubPelMode                      : MOS_BITFIELD_RANGE( 12,13 );
            uint32_t   SkipType                        : MOS_BITFIELD_BIT(      14 );
            uint32_t   DisableFieldCacheAlloc          : MOS_BITFIELD_BIT(      15 );
            uint32_t   InterChromaMode                 : MOS_BITFIELD_BIT(      16 );
            uint32_t   FTEnable                        : MOS_BITFIELD_BIT(      17 );
            uint32_t   BMEDisableFBR                   : MOS_BITFIELD_BIT(      18 );
            uint32_t   BlockBasedSkipEnable            : MOS_BITFIELD_BIT(      19 );
            uint32_t   InterSAD                        : MOS_BITFIELD_RANGE( 20,21 );
            uint32_t   IntraSAD                        : MOS_BITFIELD_RANGE( 22,23 );
            uint32_t   SubMbPartMask                   : MOS_BITFIELD_RANGE( 24,30 );
            uint32_t                                   : MOS_BITFIELD_BIT(      31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW3;

    // DW4
    union
    {
        struct
        {
            uint32_t                                   : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   PictureHeightMinus1             : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   PictureWidth                    : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW4;

    // DW5
    union
    {
        struct
        {
            uint32_t                                   : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   QpPrimeY                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefWidth                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                       : MOS_BITFIELD_RANGE( 24,31 );

        };
        struct
        {
            uint32_t       Value;
        };
    } DW5;

    // DW6
    union
    {
        struct
        {
            uint32_t                                   : MOS_BITFIELD_RANGE(  0, 2 );
            uint32_t   WriteDistortions                : MOS_BITFIELD_BIT(       3 );
            uint32_t   UseMvFromPrevStep               : MOS_BITFIELD_BIT(       4 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  5, 7 );
            uint32_t   SuperCombineDist                : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MaxVmvR                         : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW6;

    // DW7
    union
    {
        struct
        {
            uint32_t                                   : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   MVCostScaleFactor               : MOS_BITFIELD_RANGE( 16,17 );
            uint32_t   BilinearEnable                  : MOS_BITFIELD_BIT(      18 );
            uint32_t   SrcFieldPolarity                : MOS_BITFIELD_BIT(      19 );
            uint32_t   WeightedSADHAAR                 : MOS_BITFIELD_BIT(      20 );
            uint32_t   AConlyHAAR                      : MOS_BITFIELD_BIT(      21 );
            uint32_t   RefIDCostMode                   : MOS_BITFIELD_BIT(      22 );
            uint32_t                                   : MOS_BITFIELD_BIT(      23 );
            uint32_t   SkipCenterMask                  : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW7;

    // DW8
    union
    {
        struct
        {
            uint32_t   Mode0Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode1Cost                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Mode2Cost                       : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   Mode3Cost                       : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW8;

    // DW9
    union
    {
        struct
        {
            uint32_t   Mode4Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode5Cost                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Mode6Cost                       : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   Mode7Cost                       : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW9;

    // DW10
    union
    {
        struct
        {
            uint32_t   Mode8Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Mode9Cost                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefIDCost                       : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ChromaIntraModeCost             : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW10;

    // DW11
    union
    {
        struct
        {
            uint32_t   MV0Cost                         : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MV1Cost                         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MV2Cost                         : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MV3Cost                         : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW11;

    // DW12
    union
    {
        struct
        {
            uint32_t   MV4Cost                         : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MV5Cost                         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MV6Cost                         : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MV7Cost                         : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW12;

    // DW13
    union
    {
        struct
        {
            uint32_t   NumRefIdxL0MinusOne             : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   NumRefIdxL1MinusOne             : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ActualMBWidth                   : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ActualMBHeight                  : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW13;

    // DW14
    union
    {
        struct
        {
            uint32_t   List0RefID0FieldParity          : MOS_BITFIELD_BIT(       0 );
            uint32_t   List0RefID1FieldParity          : MOS_BITFIELD_BIT(       1 );
            uint32_t   List0RefID2FieldParity          : MOS_BITFIELD_BIT(       2 );
            uint32_t   List0RefID3FieldParity          : MOS_BITFIELD_BIT(       3 );
            uint32_t   List0RefID4FieldParity          : MOS_BITFIELD_BIT(       4 );
            uint32_t   List0RefID5FieldParity          : MOS_BITFIELD_BIT(       5 );
            uint32_t   List0RefID6FieldParity          : MOS_BITFIELD_BIT(       6 );
            uint32_t   List0RefID7FieldParity          : MOS_BITFIELD_BIT(       7 );
            uint32_t   List1RefID0FieldParity          : MOS_BITFIELD_BIT(       8 );
            uint32_t   List1RefID1FieldParity          : MOS_BITFIELD_BIT(       9 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 10,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW14;

    // DW15
    union
    {
        struct
        {
            uint32_t   PrevMvReadPosFactor             : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MvShiftFactor                   : MOS_BITFIELD_RANGE( 8, 15 );
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
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
                uint32_t       Value;
            };
        } DW29;
    } SPDelta;

    // DW30
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW30;

    // DW31
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW31;

    // DW32
    union
    {
        struct
        {
            uint32_t   _4xMeMvOutputDataSurfIndex      : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW32;

    // DW33
    union
    {
        struct
        {
            uint32_t   _16xOr32xMeMvInputDataSurfIndex : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW33;

    // DW34
    union
    {
        struct
        {
            uint32_t   _4xMeOutputDistSurfIndex        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW34;

    // DW35
    union
    {
        struct
        {
            uint32_t   _4xMeOutputBrcDistSurfIndex     : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW35;

    // DW36
    union
    {
        struct
        {
            uint32_t   VMEFwdInterPredictionSurfIndex  : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW36;

    // DW37
    union
    {
        struct
        {
            uint32_t   VMEBwdInterPredictionSurfIndex  : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW37;

    // DW38
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t       Value;
        };
    } DW38;

} ME_CURBE_CM_FEI, *PME_CURBE_CM_FEI;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(ME_CURBE_CM_FEI)) == 39);
//seem as legacy
const uint32_t CodechalEncodeAvcEncFeiG8::m_modeMvCost_Cm_PreProc[3][CODEC_AVC_NUM_QP][8] =
{
    // I-Frame
    {
        { 0x1e03000a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff000000, 0x00000000, 0x00000000 },
        { 0x1e03000a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff010101, 0x00000000, 0x00000000 },
        { 0x1e03000a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff020202, 0x00000000, 0x00000000 },
        { 0x1e03000a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff030303, 0x00000000, 0x00000000 },
        { 0x1e03000a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff040404, 0x00000000, 0x00000000 },
        { 0x1e03000a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff050505, 0x00000000, 0x00000000 },
        { 0x1e03000a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff060606, 0x00000000, 0x00000000 },
        { 0x1e03000a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff070707, 0x01010001, 0x01010101 },
        { 0x1e03000a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff080808, 0x01010001, 0x01010101 },
        { 0x1e03000a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff090909, 0x03030003, 0x03030303 },
        { 0x1e03000a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0a0a0a, 0x03030003, 0x03030303 },
        { 0x1e03000a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0b0b0b, 0x06060006, 0x06060606 },
        { 0x1e03000a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0c0c0c, 0x06060006, 0x06060606 },
        { 0x1e03000a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0d0d0d, 0x08080008, 0x08080808 },
        { 0x1e03000a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0e0e0e, 0x08080008, 0x08080808 },
        { 0x1e03000a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff0f0f0f, 0x0b0b000b, 0x0b0b0b0b },
        { 0x2e06001a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff101010, 0x0b0b000b, 0x0b0b0b0b },
        { 0x2e06001a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff111111, 0x0d0d000d, 0x0d0d0d0d },
        { 0x2e06001a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff121212, 0x0d0d000d, 0x0d0d0d0d },
        { 0x2e06001a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff131313, 0x10100010, 0x10101010 },
        { 0x3b09001f, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff141414, 0x10100010, 0x10101010 },
        { 0x3b09001f, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff151515, 0x13130013, 0x13131313 },
        { 0x3b09001f, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff161616, 0x13130013, 0x13131313 },
        { 0x3e0c002a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff171717, 0x16160016, 0x16161616 },
        { 0x3e0c002a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff181818, 0x16160016, 0x16161616 },
        { 0x3e0c002a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff191919, 0x1a1a001a, 0x1a1a1a1a },
        { 0x490f002d, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff1a1a1a, 0x1a1a001a, 0x1a1a1a1a },
        { 0x4b19002f, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff1b1b1b, 0x1e1e001e, 0x1e1e1e1e },
        { 0x4b19002f, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff1c1c1c, 0x1e1e001e, 0x1e1e1e1e },
        { 0x4c1b0039, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff1d1d1d, 0x22220022, 0x22222222 },
        { 0x4e1c003a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff1e1e1e, 0x22220022, 0x22222222 },
        { 0x581e003b, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff1f1f1f, 0x27270027, 0x27272727 },
        { 0x591f003d, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff202020, 0x27270027, 0x27272727 },
        { 0x5a28003e, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff212121, 0x2c2c002c, 0x2c2c2c2c },
        { 0x5b2a0048, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff222222, 0x2c2c002c, 0x2c2c2c2c },
        { 0x5c2b0049, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff232323, 0x32320032, 0x32323232 },
        { 0x5e2c004a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff242424, 0x32320032, 0x32323232 },
        { 0x682e004b, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff252525, 0x38380038, 0x38383838 },
        { 0x692f004d, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff262626, 0x38380038, 0x38383838 },
        { 0x6a39004e, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff272727, 0x3e3e003e, 0x3e3e3e3e },
        { 0x6b390058, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff282828, 0x3e3e003e, 0x3e3e3e3e },
        { 0x6d3b0059, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff292929, 0x45450045, 0x45454545 },
        { 0x6e3c005a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff2a2a2a, 0x45450045, 0x45454545 },
        { 0x783e005b, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff2b2b2b, 0x4d4d004d, 0x4d4d4d4d },
        { 0x793f005d, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff2c2c2c, 0x4d4d004d, 0x4d4d4d4d },
        { 0x7a48005e, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff2d2d2d, 0x55550055, 0x55555555 },
        { 0x7b4a0068, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff2e2e2e, 0x55550055, 0x55555555 },
        { 0x7c4b0069, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff2f2f2f, 0x5e5e005e, 0x5e5e5e5e },
        { 0x7e4c006a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff303030, 0x5e5e005e, 0x5e5e5e5e },
        { 0x884e006b, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff313131, 0x68680068, 0x68686868 },
        { 0x894f006d, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff323232, 0x68680068, 0x68686868 },
        { 0x8a59006e, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff333333, 0x73730073, 0x73737373 }
    },
    // P-Frame
    {
        { 0x391e1a07, 0x06040208, 0x00040005, 0x09050401, 0x0f0e0c0a, 0xff000000, 0x00000000, 0x00000000 },
        { 0x391e1a07, 0x06040208, 0x00040005, 0x09050401, 0x0f0e0c0a, 0xff010101, 0x00000000, 0x00000000 },
        { 0x391e1a07, 0x06040208, 0x00040005, 0x09050401, 0x0f0e0c0a, 0xff020202, 0x00000000, 0x00000000 },
        { 0x391e1a07, 0x06040208, 0x00040005, 0x09050401, 0x0f0e0c0a, 0xff030303, 0x00000000, 0x00000000 },
        { 0x391e1a07, 0x06040208, 0x00040005, 0x09050401, 0x0f0e0c0a, 0xff040404, 0x00000000, 0x00000000 },
        { 0x391e1a07, 0x06040208, 0x00040005, 0x09050401, 0x0f0e0c0a, 0xff050505, 0x00000000, 0x00000000 },
        { 0x391e1a07, 0x06040208, 0x00040005, 0x09050401, 0x0f0e0c0a, 0xff060606, 0x00000000, 0x00000000 },
        { 0x391e1a07, 0x06040208, 0x00040005, 0x09050401, 0x0f0e0c0a, 0xff070707, 0x01010001, 0x01010101 },
        { 0x391e1a07, 0x06040208, 0x00040005, 0x09050401, 0x0f0e0c0a, 0xff080808, 0x01010001, 0x01010101 },
        { 0x391e1a07, 0x06040208, 0x00040005, 0x09050401, 0x0f0e0c0a, 0xff090909, 0x03030003, 0x03030303 },
        { 0x391e1a07, 0x06040208, 0x00040005, 0x09050401, 0x0f0e0c0a, 0xff0a0a0a, 0x03030003, 0x03030303 },
        { 0x391e1a07, 0x06040208, 0x00040005, 0x09050401, 0x0f0e0c0a, 0xff0b0b0b, 0x06060006, 0x06060606 },
        { 0x391e1a07, 0x06040208, 0x00040005, 0x09050401, 0x0f0e0c0a, 0xff0c0c0c, 0x06060006, 0x06060606 },
        { 0x391e1a07, 0x06040208, 0x00040005, 0x09050401, 0x0f0e0c0a, 0xff0d0d0d, 0x08080008, 0x08080808 },
        { 0x391e1a07, 0x06040208, 0x00040005, 0x09050401, 0x0f0e0c0a, 0xff0e0e0e, 0x08080008, 0x08080808 },
        { 0x391e1a07, 0x06040208, 0x00040005, 0x09050401, 0x0f0e0c0a, 0xff0f0f0f, 0x0b0b000b, 0x0b0b0b0b },
        { 0x492e2a0e, 0x0d090519, 0x0008000b, 0x190a0802, 0x1f1e1c1a, 0xff101010, 0x0b0b000b, 0x0b0b0b0b },
        { 0x492e2a0e, 0x0d090519, 0x0008000b, 0x190a0802, 0x1f1e1c1a, 0xff111111, 0x0d0d000d, 0x0d0d0d0d },
        { 0x492e2a0e, 0x0d090519, 0x0008000b, 0x190a0802, 0x1f1e1c1a, 0xff121212, 0x0d0d000d, 0x0d0d0d0d },
        { 0x492e2a0e, 0x0d090519, 0x0008000b, 0x190a0802, 0x1f1e1c1a, 0xff131313, 0x10100010, 0x10101010 },
        { 0x4d3b2f1b, 0x1a0d071d, 0x000c0018, 0x1e0f0c03, 0x2b2b291f, 0xff141414, 0x10100010, 0x10101010 },
        { 0x4d3b2f1b, 0x1a0d071d, 0x000c0018, 0x1e0f0c03, 0x2b2b291f, 0xff151515, 0x13130013, 0x13131313 },
        { 0x4d3b2f1b, 0x1a0d071d, 0x000c0018, 0x1e0f0c03, 0x2b2b291f, 0xff161616, 0x13130013, 0x13131313 },
        { 0x593e3a1e, 0x1d190a29, 0x0018001b, 0x291a1804, 0x2f2e2c2a, 0xff171717, 0x16160016, 0x16161616 },
        { 0x593e3a1e, 0x1d190a29, 0x0018001b, 0x291a1804, 0x2f2e2c2a, 0xff181818, 0x16160016, 0x16161616 },
        { 0x593e3a1e, 0x1d190a29, 0x0018001b, 0x291a1804, 0x2f2e2c2a, 0xff191919, 0x1a1a001a, 0x1a1a1a1a },
        { 0x5b493d29, 0x281c0d2b, 0x001a001e, 0x2b1d1a05, 0x39392f2d, 0xff1a1a1a, 0x1a1a001a, 0x1a1a1a1a },
        { 0x5d4b3f2b, 0x2a1e0f2d, 0x001c0028, 0x2e1f1c06, 0x3b3b392f, 0xff1b1b1b, 0x1e1e001e, 0x1e1e1e1e },
        { 0x5d4b3f2b, 0x2a1e0f2d, 0x001c0028, 0x2e1f1c06, 0x3b3b392f, 0xff1c1c1c, 0x1e1e001e, 0x1e1e1e1e },
        { 0x5f4c492c, 0x2c28192f, 0x001e002a, 0x38291e07, 0x3d3c3b39, 0xff1d1d1d, 0x22220022, 0x22222222 },
        { 0x694e4a2e, 0x2d291b39, 0x0028002b, 0x392a2808, 0x3f3e3c3a, 0xff1e1e1e, 0x22220022, 0x22222222 },
        { 0x6a584b38, 0x2f2a1c3a, 0x0029002c, 0x3a2b2909, 0x48483e3b, 0xff1f1f1f, 0x27270027, 0x27272727 },
        { 0x6b594d39, 0x382c1d3b, 0x002a002e, 0x3b2d2a0a, 0x49493f3d, 0xff202020, 0x27270027, 0x27272727 },
        { 0x6c5a4e3a, 0x392d1f3c, 0x002b002f, 0x3c2e2b0b, 0x4a4a483e, 0xff212121, 0x2c2c002c, 0x2c2c2c2c },
        { 0x6e5b583b, 0x3b2f293e, 0x002d0039, 0x3f382d0d, 0x4c4b4a48, 0xff222222, 0x2c2c002c, 0x2c2c2c2c },
        { 0x6f5c593c, 0x3c38293f, 0x002e003a, 0x48392e0e, 0x4d4c4b49, 0xff232323, 0x32320032, 0x32323232 },
        { 0x795e5a3e, 0x3d392b49, 0x0038003b, 0x493a3818, 0x4f4e4c4a, 0xff242424, 0x32320032, 0x32323232 },
        { 0x7a685b48, 0x3f3a2c4a, 0x0039003c, 0x4a3b3919, 0x58584e4b, 0xff252525, 0x38380038, 0x38383838 },
        { 0x7b695d49, 0x483c2d4b, 0x003a003e, 0x4b3d3a1a, 0x59594f4d, 0xff262626, 0x38380038, 0x38383838 },
        { 0x7d6a5e4a, 0x4a3d2f4c, 0x003c0048, 0x4d3e3c1c, 0x5b5a594e, 0xff272727, 0x3e3e003e, 0x3e3e3e3e },
        { 0x7e6b684b, 0x4a3e384d, 0x003d0049, 0x4e483d1d, 0x5c5b5958, 0xff282828, 0x3e3e003e, 0x3e3e3e3e },
        { 0x886d694d, 0x4c483a4f, 0x003f004a, 0x58493f1f, 0x5e5d5b59, 0xff292929, 0x45450045, 0x45454545 },
        { 0x896e6a4e, 0x4d493b59, 0x0048004b, 0x594a4828, 0x5f5e5c5a, 0xff2a2a2a, 0x45450045, 0x45454545 },
        { 0x8a786b58, 0x4f4a3c5a, 0x0049004c, 0x5a4b4929, 0x68685e5b, 0xff2b2b2b, 0x4d4d004d, 0x4d4d4d4d },
        { 0x8b796d59, 0x584c3d5b, 0x004a004e, 0x5b4d4a2a, 0x69695f5d, 0xff2c2c2c, 0x4d4d004d, 0x4d4d4d4d },
        { 0x8c7a6e5a, 0x594d3f5c, 0x004b004f, 0x5d4e4b2b, 0x6b6a685e, 0xff2d2d2d, 0x55550055, 0x55555555 },
        { 0x8e7b785b, 0x5b4f485e, 0x004d0059, 0x5e584d2d, 0x6c6b6a68, 0xff2e2e2e, 0x55550055, 0x55555555 },
        { 0x8f7c795c, 0x5c58495f, 0x004e005a, 0x68594e2e, 0x6d6c6b69, 0xff2f2f2f, 0x5e5e005e, 0x5e5e5e5e },
        { 0x8f7e7a5e, 0x5d594b69, 0x0058005b, 0x695a5838, 0x6f6e6c6a, 0xff303030, 0x5e5e005e, 0x5e5e5e5e },
        { 0x8f887b68, 0x5f5a4c6a, 0x0059005c, 0x6a5b5939, 0x6f6f6e6b, 0xff313131, 0x68680068, 0x68686868 },
        { 0x8f897d69, 0x685c4d6b, 0x005a005e, 0x6b5d5a3a, 0x6f6f6f6d, 0xff323232, 0x68680068, 0x68686868 },
        { 0x8f8a7e6a, 0x695d4f6c, 0x005b0068, 0x6d5e5b3b, 0x6f6f6f6e, 0xff333333, 0x73730073, 0x73737373 }
    },
    // B-Frame
    {
        { 0x3a2a2907, 0x0a08060c, 0x00040206, 0x06020200, 0x180e0c0a, 0xff000000, 0x00000000, 0x00000000 },
        { 0x3a2a2907, 0x0a08060c, 0x00040206, 0x06020200, 0x180e0c0a, 0xff010101, 0x00000000, 0x00000000 },
        { 0x3a2a2907, 0x0a08060c, 0x00040206, 0x06020200, 0x180e0c0a, 0xff020202, 0x00000000, 0x00000000 },
        { 0x3a2a2907, 0x0a08060c, 0x00040206, 0x06020200, 0x180e0c0a, 0xff030303, 0x00000000, 0x00000000 },
        { 0x3a2a2907, 0x0a08060c, 0x00040206, 0x06020200, 0x180e0c0a, 0xff040404, 0x00000000, 0x00000000 },
        { 0x3a2a2907, 0x0a08060c, 0x00040206, 0x06020200, 0x180e0c0a, 0xff050505, 0x00000000, 0x00000000 },
        { 0x3a2a2907, 0x0a08060c, 0x00040206, 0x06020200, 0x180e0c0a, 0xff060606, 0x00000000, 0x00000000 },
        { 0x3a2a2907, 0x0a08060c, 0x00040206, 0x06020200, 0x180e0c0a, 0xff070707, 0x01010001, 0x01010101 },
        { 0x3a2a2907, 0x0a08060c, 0x00040206, 0x06020200, 0x180e0c0a, 0xff080808, 0x01010001, 0x01010101 },
        { 0x3a2a2907, 0x0a08060c, 0x00040206, 0x06020200, 0x180e0c0a, 0xff090909, 0x03030003, 0x03030303 },
        { 0x3a2a2907, 0x0a08060c, 0x00040206, 0x06020200, 0x180e0c0a, 0xff0a0a0a, 0x03030003, 0x03030303 },
        { 0x3a2a2907, 0x0a08060c, 0x00040206, 0x06020200, 0x180e0c0a, 0xff0b0b0b, 0x06060006, 0x06060606 },
        { 0x3a2a2907, 0x0a08060c, 0x00040206, 0x06020200, 0x180e0c0a, 0xff0c0c0c, 0x06060006, 0x06060606 },
        { 0x3a2a2907, 0x0a08060c, 0x00040206, 0x06020200, 0x180e0c0a, 0xff0d0d0d, 0x08080008, 0x08080808 },
        { 0x3a2a2907, 0x0a08060c, 0x00040206, 0x06020200, 0x180e0c0a, 0xff0e0e0e, 0x08080008, 0x08080808 },
        { 0x3a2a2907, 0x0a08060c, 0x00040206, 0x06020200, 0x180e0c0a, 0xff0f0f0f, 0x0b0b000b, 0x0b0b0b0b },
        { 0x4a3a390e, 0x1b190d1c, 0x0008040c, 0x0c040400, 0x281e1c1a, 0xff101010, 0x0b0b000b, 0x0b0b0b0b },
        { 0x4a3a390e, 0x1b190d1c, 0x0008040c, 0x0c040400, 0x281e1c1a, 0xff111111, 0x0d0d000d, 0x0d0d0d0d },
        { 0x4a3a390e, 0x1b190d1c, 0x0008040c, 0x0c040400, 0x281e1c1a, 0xff121212, 0x0d0d000d, 0x0d0d0d0d },
        { 0x4a3a390e, 0x1b190d1c, 0x0008040c, 0x0c040400, 0x281e1c1a, 0xff131313, 0x10100010, 0x10101010 },
        { 0x4f3f3d1b, 0x281d1a29, 0x000c0619, 0x19060600, 0x2c2b291f, 0xff141414, 0x10100010, 0x10101010 },
        { 0x4f3f3d1b, 0x281d1a29, 0x000c0619, 0x19060600, 0x2c2b291f, 0xff151515, 0x13130013, 0x13131313 },
        { 0x4f3f3d1b, 0x281d1a29, 0x000c0619, 0x19060600, 0x2c2b291f, 0xff161616, 0x13130013, 0x13131313 },
        { 0x5a4a491e, 0x2b291d2c, 0x0018081c, 0x1c080800, 0x382e2c2a, 0xff171717, 0x16160016, 0x16161616 },
        { 0x5a4a491e, 0x2b291d2c, 0x0018081c, 0x1c080800, 0x382e2c2a, 0xff181818, 0x16160016, 0x16161616 },
        { 0x5a4a491e, 0x2b291d2c, 0x0018081c, 0x1c080800, 0x382e2c2a, 0xff191919, 0x1a1a001a, 0x1a1a1a1a },
        { 0x5d4d4b29, 0x2d2b282f, 0x001a0a1f, 0x1f0a0a00, 0x3a392f2d, 0xff1a1a1a, 0x1a1a001a, 0x1a1a1a1a },
        { 0x5f4f4d2b, 0x382d2a39, 0x001c0c29, 0x290c0c00, 0x3c3b392f, 0xff1b1b1b, 0x1e1e001e, 0x1e1e1e1e },
        { 0x5f4f4d2b, 0x382d2a39, 0x001c0c29, 0x290c0c00, 0x3c3b392f, 0xff1c1c1c, 0x1e1e001e, 0x1e1e1e1e },
        { 0x69594f2c, 0x392f2b3b, 0x001e0e2b, 0x2b0e0e00, 0x3e3c3b39, 0xff1d1d1d, 0x22220022, 0x22222222 },
        { 0x6a5a592e, 0x3b392d3c, 0x0028182c, 0x2c181800, 0x483e3c3a, 0xff1e1e1e, 0x22220022, 0x22222222 },
        { 0x6b5b5a38, 0x3c3a2f3e, 0x0029192e, 0x2e191900, 0x49483e3b, 0xff1f1f1f, 0x27270027, 0x27272727 },
        { 0x6d5d5b39, 0x3d3b383f, 0x002a1a2f, 0x2f1a1a00, 0x4a493f3d, 0xff202020, 0x27270027, 0x27272727 },
        { 0x6e5e5c3a, 0x3e3c3948, 0x002b1b38, 0x381b1b00, 0x4b4a483e, 0xff212121, 0x2c2c002c, 0x2c2c2c2c },
        { 0x78685e3b, 0x493e3b4a, 0x002d1d3a, 0x3a1d1d00, 0x4d4b4a48, 0xff222222, 0x2c2c002c, 0x2c2c2c2c },
        { 0x79695f3c, 0x493f3b4b, 0x002e1e3b, 0x3b1e1e00, 0x4e4c4b49, 0xff232323, 0x32320032, 0x32323232 },
        { 0x7a6a693e, 0x4b493d4c, 0x0038283c, 0x3c282800, 0x584e4c4a, 0xff242424, 0x32320032, 0x32323232 },
        { 0x7b6b6a48, 0x4c4a3f4e, 0x0039293e, 0x3e292900, 0x59584e4b, 0xff252525, 0x38380038, 0x38383838 },
        { 0x7d6d6b49, 0x4d4b484f, 0x003a2a3f, 0x3f2a2a00, 0x5a594f4d, 0xff262626, 0x38380038, 0x38383838 },
        { 0x7e6e6c4a, 0x4f4c4959, 0x003c2c49, 0x492c2c00, 0x5c5a594e, 0xff272727, 0x3e3e003e, 0x3e3e3e3e },
        { 0x88786d4b, 0x584d4a59, 0x003d2d49, 0x492d2d00, 0x5d5b5958, 0xff282828, 0x3e3e003e, 0x3e3e3e3e },
        { 0x89796f4d, 0x5a4f4c5b, 0x003f2f4b, 0x4b2f2f00, 0x5f5d5b59, 0xff292929, 0x45450045, 0x45454545 },
        { 0x8a7a794e, 0x5b594d5c, 0x0048384c, 0x4c383800, 0x685e5c5a, 0xff2a2a2a, 0x45450045, 0x45454545 },
        { 0x8b7b7a58, 0x5c5a4f5e, 0x0049394e, 0x4e393900, 0x69685e5b, 0xff2b2b2b, 0x4d4d004d, 0x4d4d4d4d },
        { 0x8d7d7b59, 0x5d5b585f, 0x004a3a4f, 0x4f3a3a00, 0x6a695f5d, 0xff2c2c2c, 0x4d4d004d, 0x4d4d4d4d },
        { 0x8e7e7c5a, 0x5f5c5968, 0x004b3b58, 0x583b3b00, 0x6b6a685e, 0xff2d2d2d, 0x55550055, 0x55555555 },
        { 0x8f887e5b, 0x685e5a6a, 0x004d3d5a, 0x5a3d3d00, 0x6d6b6a68, 0xff2e2e2e, 0x55550055, 0x55555555 },
        { 0x8f897f5c, 0x695f5c6b, 0x004e3e5b, 0x5b3e3e00, 0x6e6c6b69, 0xff2f2f2f, 0x5e5e005e, 0x5e5e5e5e },
        { 0x8f8a895e, 0x6b695d6c, 0x0058485c, 0x5c484800, 0x6f6e6c6a, 0xff303030, 0x5e5e005e, 0x5e5e5e5e },
        { 0x8f8b8a68, 0x6c6a5f6e, 0x0059495e, 0x5e494900, 0x6f6f6e6b, 0xff313131, 0x68680068, 0x68686868 },
        { 0x8f8d8b69, 0x6d6b686f, 0x005a4a5f, 0x5f4a4a00, 0x6f6f6f6d, 0xff323232, 0x68680068, 0x68686868 },
        { 0x8f8e8c6a, 0x6f6c6979, 0x005b4b69, 0x694b4b00, 0x6f6f6f6e, 0xff333333, 0x73730073, 0x73737373 }
    }
};

const uint32_t CodechalEncodeAvcEncFeiG8::m_meCurbeCmFei[39] =
{
    0x00000000, 0x00200010, 0x00003939, 0x77a43000, 0x00000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

// AVC PreProc CURBE init data for G9 CM Kernel
const uint32_t CodechalEncodeAvcEncFeiG8::m_preProcCurbeCmNormalIFrame[49] =
{
    0x00000082, 0x00000000, 0x00003910, 0x00a83000, 0x00000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00040c24, 0x00000000, 0x00000000, 0x40000000, 0x00000080, 0x00003900, 0x28301000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG8::m_preProcCurbeCmNormalIfield[49] =
{
    0x00000082, 0x00000000, 0x00003910, 0x00a830c0, 0x00000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00040c24, 0x00000000, 0x00000000, 0x40000000, 0x00000080, 0x00003900, 0x28301000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG8::m_preProcCurbeCmNormalPFrame[49] =
{
    0x000000a3, 0x00000008, 0x00003910, 0x00ae3000, 0x00000000, 0x28300000, 0x00000000, 0x01400060,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00040c24, 0x00000000, 0x00000000, 0x60000000, 0x000000a1, 0x00003900, 0x28301000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG8::m_preProcCurbeCmNormalPField[49] =
{
    0x000000a3, 0x00000008, 0x00003910, 0x00ae30c0, 0x00000000, 0x28300000, 0x00000000, 0x01400060,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00040c24, 0x00000000, 0x00000000, 0x40000000, 0x000000a1, 0x00003900, 0x28301000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG8::m_preProcCurbeCmNormalBFrame[49] =
{
    0x000000a3, 0x00200008, 0x00003910, 0x00aa7700, 0x00000000, 0x20200000, 0x00000000, 0xff400000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00040c24, 0x00000000, 0x00000000, 0x60000000, 0x000000a1, 0x00003900, 0x28301000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG8::m_preProcCurbeCmNormalBField[49] =
{
    0x000000a3, 0x00200008, 0x00003919, 0x00aa77c0, 0x00000000, 0x20200000, 0x00000000, 0xff400000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00040c24, 0x00000000, 0x00000000, 0x40000000, 0x000000a1, 0x00003900, 0x28301000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff
};

typedef struct _MBENC_CURBE_CM_FEI
{
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn                      : MOS_BITFIELD_BIT(       0 );
            uint32_t   AdaptiveEn                      : MOS_BITFIELD_BIT(       1 );
            uint32_t   BiMixDis                        : MOS_BITFIELD_BIT(       2 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  3, 4 );
            uint32_t   EarlyImeSuccessEn               : MOS_BITFIELD_BIT(       5 );
            uint32_t                                   : MOS_BITFIELD_BIT(       6 );
            uint32_t   T8x8FlagForInterEn              : MOS_BITFIELD_BIT(       7 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  8,23 );
            uint32_t   EarlyImeStop                    : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   MaxNumMVs                       : MOS_BITFIELD_RANGE(  0, 5 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  6,15 );
            uint32_t   BiWeight                        : MOS_BITFIELD_RANGE( 16,21 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 22,27 );
            uint32_t   UniMixDisable                   : MOS_BITFIELD_BIT(      28 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 29,31 );
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
            uint32_t   LenSP                           : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MaxNumSU                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   PicWidth                        : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   SrcSize                         : MOS_BITFIELD_RANGE(  0, 1 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  2, 3 );
            uint32_t   MbTypeRemap                     : MOS_BITFIELD_RANGE(  4, 5 );
            uint32_t   SrcAccess                       : MOS_BITFIELD_BIT(       6 );
            uint32_t   RefAccess                       : MOS_BITFIELD_BIT(       7 );
            uint32_t   SearchCtrl                      : MOS_BITFIELD_RANGE(  8,10 );
            uint32_t   DualSearchPathOption            : MOS_BITFIELD_BIT(      11 );
            uint32_t   SubPelMode                      : MOS_BITFIELD_RANGE( 12,13 );
            uint32_t   SkipType                        : MOS_BITFIELD_BIT(      14 );
            uint32_t   DisableFieldCacheAlloc          : MOS_BITFIELD_BIT(      15 );
            uint32_t   InterChromaMode                 : MOS_BITFIELD_BIT(      16 );
            uint32_t   FTEnable                        : MOS_BITFIELD_BIT(      17 );
            uint32_t   BMEDisableFBR                   : MOS_BITFIELD_BIT(      18 );
            uint32_t   BlockBasedSkipEnable            : MOS_BITFIELD_BIT(      19 );
            uint32_t   InterSAD                        : MOS_BITFIELD_RANGE( 20,21 );
            uint32_t   IntraSAD                        : MOS_BITFIELD_RANGE( 22,23 );
            uint32_t   SubMbPartMask                   : MOS_BITFIELD_RANGE( 24,30 );
            uint32_t                                   : MOS_BITFIELD_BIT(      31 );
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
            uint32_t   PicHeightMinus1                 : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   MvRestrictionInSliceEnable      : MOS_BITFIELD_BIT(      16 );
            uint32_t   DeltaMvEnable                   : MOS_BITFIELD_BIT(      17 );
            uint32_t   TrueDistortionEnable            : MOS_BITFIELD_BIT(      18 );
            uint32_t   FEIEnable                       : MOS_BITFIELD_BIT(      19);
            uint32_t   MultipleMVPredictorPerMBEnable  : MOS_BITFIELD_BIT(      20);
            uint32_t   VMEDistortionOutputEnable       : MOS_BITFIELD_BIT(      21);
            uint32_t   PerMBQpEnable                   : MOS_BITFIELD_BIT(      22);
            uint32_t   MBInputEnable                   : MOS_BITFIELD_BIT(      23);
            uint32_t                                   : MOS_BITFIELD_BIT(      24);
            uint32_t   bCurFldIDR                      : MOS_BITFIELD_BIT(      25 );
            uint32_t   ConstrainedIntraPredFlag        : MOS_BITFIELD_BIT(      26 );
            uint32_t   FieldParityFlag                 : MOS_BITFIELD_BIT(      27 );
            uint32_t   HMEEnable                       : MOS_BITFIELD_BIT(      28 );
            uint32_t   PictureType                     : MOS_BITFIELD_RANGE( 29,30 );
            uint32_t   UseActualRefQPValue             : MOS_BITFIELD_BIT(      31 );
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
            uint32_t   SliceMbHeight                   : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   RefWidth                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                       : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   BatchBufferEnd                  : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   IntraPartMask                   : MOS_BITFIELD_RANGE(  0, 4 );
            uint32_t   NonSkipZMvAdded                 : MOS_BITFIELD_BIT(       5 );
            uint32_t   NonSkipModeAdded                : MOS_BITFIELD_BIT(       6 );
            uint32_t   LumaIntraSrcCornerSwap          : MOS_BITFIELD_BIT(       7 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MVCostScaleFactor               : MOS_BITFIELD_RANGE( 16,17 );
            uint32_t   BilinearEnable                  : MOS_BITFIELD_BIT(      18 );
            uint32_t   SrcFieldPolarity                : MOS_BITFIELD_BIT(      19 );
            uint32_t   WeightedSADHAAR                 : MOS_BITFIELD_BIT(      20 );
            uint32_t   AConlyHAAR                      : MOS_BITFIELD_BIT(      21 );
            uint32_t   RefIDCostMode                   : MOS_BITFIELD_BIT(      22 );
            uint32_t                                   : MOS_BITFIELD_BIT(      23 );
            uint32_t   SkipCenterMask                  : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   Mode0Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   Mode1Cost                       : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   Mode2Cost                       : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   Mode3Cost                       : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   Mode4Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   Mode5Cost                       : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   Mode6Cost                       : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   Mode7Cost                       : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   Mode8Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   Mode9Cost                       : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   RefIDCost                       : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   ChromaIntraModeCost             : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   MV0Cost                         : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   MV1Cost                         : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   MV2Cost                         : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   MV3Cost                         : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   MV4Cost                         : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   MV5Cost                         : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   MV6Cost                         : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   MV7Cost                         : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   QpPrimeY                        : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   QpPrimeCb                       : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   QpPrimeCr                       : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   TargetSizeInWord                : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   SICFwdTransCoeffThreshold_0     : MOS_BITFIELD_RANGE(  0,15 );
                uint32_t   SICFwdTransCoeffThreshold_1     : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   SICFwdTransCoeffThreshold_2     : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   SICFwdTransCoeffThreshold_3     : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   SICFwdTransCoeffThreshold_4     : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   SICFwdTransCoeffThreshold_5     : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   SICFwdTransCoeffThreshold_6     : MOS_BITFIELD_RANGE( 24,31 );    // Highest Freq
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
                uint32_t   Intra4x4ModeMask                : MOS_BITFIELD_RANGE(  0, 8 );
                uint32_t                                   : MOS_BITFIELD_RANGE(  9,15 );
                uint32_t   Intra8x8ModeMask                : MOS_BITFIELD_RANGE( 16,24 );
                uint32_t                                   : MOS_BITFIELD_RANGE( 25,31 );
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
                uint32_t   Intra16x16ModeMask              : MOS_BITFIELD_RANGE(  0, 3 );
                uint32_t   IntraChromaModeMask             : MOS_BITFIELD_RANGE(  4, 7 );
                uint32_t   IntraComputeType                : MOS_BITFIELD_RANGE(  8, 9 );
                uint32_t                                   : MOS_BITFIELD_RANGE( 10,31 );
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
            uint32_t   SkipVal                         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   MultiPredL0Disable              : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MultiPredL1Disable              : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   Intra16x16NonDCPredPenalty      : MOS_BITFIELD_RANGE(  0,7 );
            uint32_t   Intra8x8NonDCPredPenalty        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Intra4x4NonDCPredPenalty        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   List0RefID0FieldParity          : MOS_BITFIELD_BIT(       0 );
            uint32_t   List0RefID1FieldParity          : MOS_BITFIELD_BIT(       1 );
            uint32_t   List0RefID2FieldParity          : MOS_BITFIELD_BIT(       2 );
            uint32_t   List0RefID3FieldParity          : MOS_BITFIELD_BIT(       3 );
            uint32_t   List0RefID4FieldParity          : MOS_BITFIELD_BIT(       4 );
            uint32_t   List0RefID5FieldParity          : MOS_BITFIELD_BIT(       5 );
            uint32_t   List0RefID6FieldParity          : MOS_BITFIELD_BIT(       6 );
            uint32_t   List0RefID7FieldParity          : MOS_BITFIELD_BIT(       7 );
            uint32_t   List1RefID0FrameFieldFlag       : MOS_BITFIELD_BIT(       8 );
            uint32_t   List1RefID1FrameFieldFlag       : MOS_BITFIELD_BIT(       9 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 10,19 );
            uint32_t   NumMVPredictorsL1               : MOS_BITFIELD_RANGE( 20,23 );
            uint32_t   List1RefID0FieldParity          : MOS_BITFIELD_BIT(      24 );
            uint32_t   List1RefID1FieldParity          : MOS_BITFIELD_BIT(      25 );
            uint32_t   MADEnableFlag                   : MOS_BITFIELD_BIT(      26 );
            uint32_t   ROIEnableFlag                   : MOS_BITFIELD_BIT(      27 );
            uint32_t   EnableMBFlatnessChkOptimization : MOS_BITFIELD_BIT(      28 );
            uint32_t   bDirectMode                     : MOS_BITFIELD_BIT(      29 );
            uint32_t   MBBrcEnable                     : MOS_BITFIELD_BIT(      30 );
            uint32_t   bOriginalBff                    : MOS_BITFIELD_BIT(      31 );
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
            uint32_t   PanicModeMBThreshold            : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   SmallMbSizeInWord               : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   LargeMbSizeInWord               : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   NumRefIdxL0MinusOne             : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   HMECombinedExtraSUs             : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   NumRefIdxL1MinusOne             : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   NumMVPredictors                 : MOS_BITFIELD_RANGE( 24,27 );  // FEI only
            uint32_t   IsFwdFrameShortTermRef          : MOS_BITFIELD_BIT(      28 );
            uint32_t   CheckAllFractionalEnable        : MOS_BITFIELD_BIT(      29 );
            uint32_t   HMECombineOverlap               : MOS_BITFIELD_RANGE( 30,31 );
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
            uint32_t   SkipModeEn                      : MOS_BITFIELD_BIT(       0 );
            uint32_t   AdaptiveEn                      : MOS_BITFIELD_BIT(       1 );
            uint32_t   BiMixDis                        : MOS_BITFIELD_BIT(       2 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  3, 4 );
            uint32_t   EarlyImeSuccessEn               : MOS_BITFIELD_BIT(       5 );
            uint32_t                                   : MOS_BITFIELD_BIT(       6 );
            uint32_t   T8x8FlagForInterEn              : MOS_BITFIELD_BIT(       7 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  8,23 );
            uint32_t   EarlyImeStop                    : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   LenSP                           : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MaxNumSU                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefThreshold                    : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t                                   : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   HMERefWindowsCombThreshold      : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefWidth                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                       : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   DistScaleFactorRefID0List0      : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID1List0      : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   DistScaleFactorRefID2List0      : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID3List0      : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   DistScaleFactorRefID4List0      : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID5List0      : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   DistScaleFactorRefID6List0      : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID7List0      : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ActualQPValueForRefID0List0     : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID1List0     : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ActualQPValueForRefID2List0     : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ActualQPValueForRefID3List0     : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   ActualQPValueForRefID4List0     : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID5List0     : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ActualQPValueForRefID6List0     : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ActualQPValueForRefID7List0     : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   ActualQPValueForRefID0List1     : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID1List1     : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefCost                         : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t                                   : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   IntraCostSF                     : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MaxVmvR                         : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW47;

    //DW48
    union
    {
        struct
        {
            uint32_t   IntraRefreshMBNum               : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   IntraRefreshUnitInMBMinus1      : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   IntraRefreshQPDelta             : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   ROI1_X_left                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1_Y_top                      : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI1_X_right                    : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1_Y_bottom                   : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   ROI2_X_left                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2_Y_top                      : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW51;

    // DW52
    union
    {
        struct
        {
            uint32_t   ROI2_X_right                    : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2_Y_bottom                   : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW52;

    // DW53
    union
    {
        struct
        {
            uint32_t   ROI3_X_left                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3_Y_top                      : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW53;

    // DW54
    union
    {
        struct
        {
            uint32_t   ROI3_X_right                    : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3_Y_bottom                   : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW54;

    // DW55
    union
    {
        struct
        {
            uint32_t   ROI4_X_left                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4_Y_top                      : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW55;

    // DW56
    union
    {
        struct
        {
            uint32_t   ROI4_X_right                    : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4_Y_bottom                   : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW56;

    // DW57
    union
    {
        struct
        {
            uint32_t   ROI1_dQpPrimeY                   : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ROI2_dQpPrimeY                   : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ROI3_dQpPrimeY                   : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ROI4_dQpPrimeY                   : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW57;

    // DW58
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW58;

    // DW59
    union
    {
        struct
        {
            uint32_t   IPCM_QP0                        : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   IPCM_QP1                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   IPCM_QP2                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   IPCM_QP3                        : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   IPCM_QP4                        : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   IPCM_Thresh0                    : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   IPCM_Thresh1                    : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   IPCM_Thresh2                    : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   IPCM_Thresh3                    : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   IPCM_Thresh4                    : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   L1ListRef0PictureCodingType     : MOS_BITFIELD_RANGE(  0, 1 ); // 0-invalid, 1-TFF, 2-invalid, 3-BFF
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  2,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW63;

    // DW64
    union
    {
        struct
        {
            uint32_t   BottomFieldOffsetL1ListRef0MV   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW64;

    // DW65
    union
    {
        struct
        {
            uint32_t   BottomFieldOffsetL1ListRef0MBCode : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW65;

    // DW66
    union
    {
        struct
        {
            uint32_t   MBDataSurfIndex                 : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW66;

    // DW67
    union
    {
        struct
        {
            uint32_t   MVDataSurfIndex                 : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW67;

    // DW68
    union
    {
        struct
        {
            uint32_t   IDistSurfIndex                  : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW68;

    // DW69
    union
    {
        struct
        {
            uint32_t   SrcYSurfIndex                   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW69;

    // DW70
    union
    {
        struct
        {
            uint32_t   MBSpecificDataSurfIndex         : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW70;

    // DW71
    union
    {
        struct
        {
            uint32_t   AuxVmeOutSurfIndex              : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW71;

    // DW72
    union
    {
        struct
        {
            uint32_t   CurrRefPicSelSurfIndex          : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW72;

    // DW73
    union
    {
        struct
        {
            uint32_t   HMEMVPredFwdBwdSurfIndex        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW73;

    // DW74
    union
    {
        struct
        {
            uint32_t   HMEDistSurfIndex                : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW74;

    // DW75
    union
    {
        struct
        {
            uint32_t   RefPicSelectL1SurfIndex         : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW75;

    // DW76
    union
    {
        struct
        {
            uint32_t   FwdFrmMBDataSurfIndex           : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW76;

    // DW77
    union
    {
        struct
        {
            uint32_t   FwdFrmMVSurfIndex               : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW77;

    // DW78
    union
    {
        struct
        {
            uint32_t   MBQPBuffer                      : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW78;

    // DW79
    union
    {
        struct
        {
            uint32_t   MBBRCLut                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW79;

    // DW80
    union
    {
        struct
        {
            uint32_t   VMEInterPredictionSurfIndex     : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW80;

    // DW81
    union
    {
        struct
        {
            uint32_t   VMEInterPredictionMRSurfIndex   : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW81;

    // DW82
    union
    {
        struct
        {
            uint32_t   FlatnessChkSurfIndex            : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW82;

    // DW83
    union
    {
        struct
        {
            uint32_t   MADSurfIndex                    : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW83;

    // DW84
    union
    {
        struct
        {
            uint32_t   InterDistortionSurfIndex        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW84;

    // DW85
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW85;

    // DW86
    union
    {
        struct
        {
            uint32_t   BRCCurbeSurfIndex               : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW86;

    // DW87
    union
    {
        struct
        {
            uint32_t   MvPredictorSurfIndex            : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW87;

    // DW88
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW88;

    // DW89
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW89;

    // DW90
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW90;

    // DW91
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW91;

    // DW92
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW92;

    // DW93
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW93;

    // DW94
    union
    {
        struct
        {
            uint32_t   Reserved                        : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW94;

}MBENC_CURBE_CM_FEI, *PMBENC_CURBE_CM_FEI;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(MBENC_CURBE_CM_FEI)) == CodechalEncodeAvcEncFeiG8::m_feiMBEncCurbeSizeInDword);

typedef struct _PREPROC_CURBE_CM
{
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn                      : MOS_BITFIELD_BIT(       0 );
            uint32_t   AdaptiveEn                      : MOS_BITFIELD_BIT(       1 );
            uint32_t   BiMixDis                        : MOS_BITFIELD_BIT(       2 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  3, 4 );
            uint32_t   EarlyImeSuccessEn               : MOS_BITFIELD_BIT(       5 );
            uint32_t                                   : MOS_BITFIELD_BIT(       6 );
            uint32_t   T8x8FlagForInterEn              : MOS_BITFIELD_BIT(       7 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  8,23 );
            uint32_t   EarlyImeStop                    : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   MaxNumMVs                       : MOS_BITFIELD_RANGE(  0, 5 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  6,15 );
            uint32_t   BiWeight                        : MOS_BITFIELD_RANGE( 16,21 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 22,27 );
            uint32_t   UniMixDisable                   : MOS_BITFIELD_BIT(      28 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 29,31 );
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
            uint32_t   LenSP                           : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MaxNumSU                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   PicWidth                        : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   SrcSize                         : MOS_BITFIELD_RANGE(  0, 1 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  2, 3 );
            uint32_t   MbTypeRemap                     : MOS_BITFIELD_RANGE(  4, 5 );
            uint32_t   SrcAccess                       : MOS_BITFIELD_BIT(       6 );
            uint32_t   RefAccess                       : MOS_BITFIELD_BIT(       7 );
            uint32_t   SearchCtrl                      : MOS_BITFIELD_RANGE(  8,10 );
            uint32_t   DualSearchPathOption            : MOS_BITFIELD_BIT(      11 );
            uint32_t   SubPelMode                      : MOS_BITFIELD_RANGE( 12,13 );
            uint32_t   SkipType                        : MOS_BITFIELD_BIT(      14 );
            uint32_t   DisableFieldCacheAlloc          : MOS_BITFIELD_BIT(      15 );
            uint32_t   InterChromaMode                 : MOS_BITFIELD_BIT(      16 );
            uint32_t   FTEnable                        : MOS_BITFIELD_BIT(      17 );
            uint32_t   BMEDisableFBR                   : MOS_BITFIELD_BIT(      18 );
            uint32_t   BlockBasedSkipEnable            : MOS_BITFIELD_BIT(      19 );
            uint32_t   InterSAD                        : MOS_BITFIELD_RANGE( 20,21 );
            uint32_t   IntraSAD                        : MOS_BITFIELD_RANGE( 22,23 );
            uint32_t   SubMbPartMask                   : MOS_BITFIELD_RANGE( 24,30 );
            uint32_t                                   : MOS_BITFIELD_BIT(      31 );
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
            uint32_t   FrameQp                         : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   PerMBQpEnable                   : MOS_BITFIELD_BIT(       8 );
            uint32_t   FieldParityFlag                 : MOS_BITFIELD_BIT(       9 );
            uint32_t   HMEEnable                       : MOS_BITFIELD_BIT(      10 );
            uint32_t   MultipleMVPredictorPerMBEnable  : MOS_BITFIELD_RANGE( 11,12 );
            uint32_t   DisableMvOutput                 : MOS_BITFIELD_BIT(      13 );
            uint32_t   DisableMbStats                  : MOS_BITFIELD_BIT(      14 );
            uint32_t   BwdRefPicFrameFieldFlag         : MOS_BITFIELD_BIT(      15 );
            uint32_t   FwdRefPicFrameFieldFlag         : MOS_BITFIELD_BIT(      16 );
            uint32_t   BwdRefPicFieldParityFlag        : MOS_BITFIELD_BIT(      17 );
            uint32_t   FwdRefPicFieldParityFlag        : MOS_BITFIELD_BIT(      18 );
            uint32_t   CurrPicFieldParityFlag          : MOS_BITFIELD_BIT(      19 );
            uint32_t   BwdRefPicEnable                 : MOS_BITFIELD_BIT(      20 );
            uint32_t   FwdRefPicEnable                 : MOS_BITFIELD_BIT(      21 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 22,31 );
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
            uint32_t   SliceMbHeight                   : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   RefWidth                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                       : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   PicHeight                       : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   IntraPartMask                   : MOS_BITFIELD_RANGE(  0, 4 );
            uint32_t   NonSkipZMvAdded                 : MOS_BITFIELD_BIT(       5 );
            uint32_t   NonSkipModeAdded                : MOS_BITFIELD_BIT(       6 );
            uint32_t   LumaIntraSrcCornerSwap          : MOS_BITFIELD_BIT(       7 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MVCostScaleFactor               : MOS_BITFIELD_RANGE( 16,17 );
            uint32_t   BilinearEnable                  : MOS_BITFIELD_BIT(      18 );
            uint32_t   SrcFieldPolarity                : MOS_BITFIELD_BIT(      19 );
            uint32_t   WeightedSADHAAR                 : MOS_BITFIELD_BIT(      20 );
            uint32_t   AConlyHAAR                      : MOS_BITFIELD_BIT(      21 );
            uint32_t   RefIDCostMode                   : MOS_BITFIELD_BIT(      22 );
            uint32_t                                   : MOS_BITFIELD_BIT(      23 );
            uint32_t   SkipCenterMask                  : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   Mode0Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   Mode1Cost                       : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   Mode2Cost                       : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   Mode3Cost                       : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   Mode4Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   Mode5Cost                       : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   Mode6Cost                       : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   Mode7Cost                       : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   Mode8Cost                       : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   Mode9Cost                       : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   RefIDCost                       : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   ChromaIntraModeCost             : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   MV0Cost                         : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   MV1Cost                         : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   MV2Cost                         : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   MV3Cost                         : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   MV4Cost                         : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   MV5Cost                         : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   MV6Cost                         : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   MV7Cost                         : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t                                   : MOS_BITFIELD_RANGE(  0, 31 );
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
                uint32_t   SICFwdTransCoeffThreshold_0     : MOS_BITFIELD_RANGE(  0,15 );
                uint32_t   SICFwdTransCoeffThreshold_1     : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   SICFwdTransCoeffThreshold_2     : MOS_BITFIELD_RANGE( 24,31 );
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
                uint32_t   SICFwdTransCoeffThreshold_3     : MOS_BITFIELD_RANGE(  0, 7 );
                uint32_t   SICFwdTransCoeffThreshold_4     : MOS_BITFIELD_RANGE(  8,15 );
                uint32_t   SICFwdTransCoeffThreshold_5     : MOS_BITFIELD_RANGE( 16,23 );
                uint32_t   SICFwdTransCoeffThreshold_6     : MOS_BITFIELD_RANGE( 24,31 );    // Highest Freq
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
                uint32_t   Intra4x4ModeMask                : MOS_BITFIELD_RANGE(  0, 8 );
                uint32_t                                   : MOS_BITFIELD_RANGE(  9,15 );
                uint32_t   Intra8x8ModeMask                : MOS_BITFIELD_RANGE( 16,24 );
                uint32_t                                   : MOS_BITFIELD_RANGE( 25,31 );
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
                uint32_t   Intra16x16ModeMask              : MOS_BITFIELD_RANGE(  0, 3 );
                uint32_t   IntraChromaModeMask             : MOS_BITFIELD_RANGE(  4, 7 );
                uint32_t   IntraComputeType                : MOS_BITFIELD_RANGE(  8, 9 );
                uint32_t                                   : MOS_BITFIELD_RANGE( 10,31 );
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
            uint32_t   MaxVmvR                         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   Intra16x16NonDCPredPenalty      : MOS_BITFIELD_RANGE(  0,7 );
            uint32_t   Intra8x8NonDCPredPenalty        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Intra4x4NonDCPredPenalty        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t                                   : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t                                   : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t                                   : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   HMECombinedExtraSUs             : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t                                   : MOS_BITFIELD_RANGE( 16,29 );
            uint32_t   HMECombineOverlap               : MOS_BITFIELD_RANGE( 30,31 );
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
            uint32_t   SkipModeEn                      : MOS_BITFIELD_BIT(       0 );
            uint32_t   AdaptiveEn                      : MOS_BITFIELD_BIT(       1 );
            uint32_t   BiMixDis                        : MOS_BITFIELD_BIT(       2 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  3, 4 );
            uint32_t   EarlyImeSuccessEn               : MOS_BITFIELD_BIT(       5 );
            uint32_t                                   : MOS_BITFIELD_BIT(       6 );
            uint32_t   T8x8FlagForInterEn              : MOS_BITFIELD_BIT(       7 );
            uint32_t                                   : MOS_BITFIELD_RANGE(  8,23 );
            uint32_t   EarlyImeStop                    : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   LenSP                           : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MaxNumSU                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefThreshold                    : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t                                   : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   HMERefWindowsCombThreshold      : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefWidth                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                       : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   CurrPicSurfIndex                : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   HMEMvDataSurfIndex              : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   MvPredictorSurfIndex            : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   MbQpSurfIndex                   : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   MvDataOutSurfIndex              : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   MbStatsOutSurfIndex             : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   VMEInterPredictionSurfIndex     : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   VMEInterPredictionMRSurfIndex   : MOS_BITFIELD_RANGE(  0,31 );
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
            uint32_t   FtqLutSurfIndex                 : MOS_BITFIELD_RANGE(  0,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } DW48;

} PREPROC_CURBE_CM, *PPREPROC_CURBE_CM;

#ifdef FEI_ENABLE_CMRT
typedef struct _CODECHAL_ENCODE_AVC_SURFACE_IDX{
    union{
        struct{
            SurfaceIndex * pCmSurfIdx[28];
        };
        struct{
            SurfaceIndex * MBDataSurfIndex;
            SurfaceIndex * MVDataSurfIndex;
            SurfaceIndex * MBDistIndex;
            SurfaceIndex * SrcYSurfIndex;
            SurfaceIndex * MBSpecficDataSurfIndex;
            SurfaceIndex * AuxVmeOutSurfIndex;
            SurfaceIndex * CurrRefPicSelSurfIndex;
            SurfaceIndex * HMEMVPredFwdBwdSurfIndex;
            SurfaceIndex * HMEDistSurfIndex;
            SurfaceIndex * SliceMapSurfIndex;
            SurfaceIndex * FwdFrmMBDataSurfIndex;
            SurfaceIndex * FwdFrmMVSurfIndex;
            SurfaceIndex * MBQPBuffer;
            SurfaceIndex * MBBRCLut;
            SurfaceIndex * MBVMEInterPredictionSurfIndex;
            SurfaceIndex * MBVMEInterPredictionMRSurfIndex;
            SurfaceIndex * MBstats;
            SurfaceIndex * MADSurfIndex;
            SurfaceIndex * ForceNonSkipMBMap;
            SurfaceIndex * ReservedIndex;
            SurfaceIndex * BRCCurbeSurfIndex;
            SurfaceIndex * StaticDetectionCostTableIndex;
            SurfaceIndex * FEI_MVPredSurfIndex;
            SurfaceIndex * L1RefPicSelSurfIndex;
            SurfaceIndex * Flatnesschk;
            SurfaceIndex * InterDistortionIndex;
            SurfaceIndex * BestInterIntraSurfIndex;
            SurfaceIndex * Reserved0;
        };
    };
}CODECHAL_ENCODE_AVC_SURFACE_IDX, *PCODECHAL_ENCODE_AVC_SURFACE_IDX;

typedef struct _CODECHAL_ENCODE_AVC_MBENC_DISPATCH_PARAMS
{
    CodechalEncodeMdfKernelResource*        kernelRes;
    PCODECHAL_ENCODE_AVC_SURFACE_IDX        avcMBEncSurface;
    uint16_t                                frameWidthInMBs;
    uint16_t                                sliceHeight;
    uint16_t                                numSlices;
    uint16_t                                sliceType;
    uint32_t                                numMBs;
    bool                                    enableArbitrarySliceSize;
    uint8_t                                 enableWavefrontOptimization;
    uint8_t                                 reserved0;
    uint8_t                                 reserved1;
    uint8_t                                 reserved2;
}CODECHAL_ENCODE_AVC_MBENC_DISPATCH_PARAMS, *PCODECHAL_ENCODE_AVC_MBENC_DISPATCH_PARAMS;
#endif

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(PREPROC_CURBE_CM)) == 49);

// AVC FEI MBEnc CURBE init data for G9 Kernel
const uint32_t CodechalEncodeAvcEncFeiG8::m_feiMbEncCurbeNormalIFrame[m_feiMBEncCurbeSizeInDword] =
{
    0x00000082, 0x00000000, 0x00003910, 0x00a83000, 0x00000000, 0x28300000, 0x05000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80800000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG8::m_feiMbEncCurbeNormalIField[m_feiMBEncCurbeSizeInDword] =
{
    0x00000082, 0x00000000, 0x00003910, 0x00a830c0, 0x02000000, 0x28300000, 0x05000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80800000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG8::m_feiMbEncCurbeNormalPFrame[m_feiMBEncCurbeSizeInDword] =
{
    0x000000a3, 0x00000008, 0x00003910, 0x00ae3000, 0x30000000, 0x28300000, 0x05000000, 0x01400060,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x60000000, 0x000000a1, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x08000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG8::m_feiMbEncCurbeNormalPfield[m_feiMBEncCurbeSizeInDword] =
{
    0x000000a3, 0x00000008, 0x00003910, 0x00ae30c0, 0x30000000, 0x28300000, 0x05000000, 0x01400060,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x000000a1, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff

};

const uint32_t CodechalEncodeAvcEncFeiG8::m_feiMbEncCurbeNormalBFrame[m_feiMBEncCurbeSizeInDword] =
{
    0x000000a3, 0x00200008, 0x00003910, 0x00aa7700, 0x50020000, 0x20200000, 0x05000000, 0xff400000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x01010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x60000000, 0x000000a1, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x08000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG8::m_feiMbEncCurbeNormalBField[m_feiMBEncCurbeSizeInDword] =
{
    0x000000a3, 0x00200008, 0x00003919, 0x00aa77c0, 0x50020000, 0x20200000, 0x05000000, 0xff400000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x01010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x000000a1, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff

};

// FEI AVC I_DIST CURBE init data for G9 Kernel
const uint32_t CodechalEncodeAvcEncFeiG8::m_feiMbEncCurbeIFrameDist[m_feiMBEncCurbeSizeInDword] =
{
    0x00000082, 0x00200008, 0x001e3910, 0x00a83000, 0x90000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000100,
    0x80800000, 0x00000000, 0x00000800, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_PREPROC_CM_G8
{
    CODECHAL_ENCODE_AVC_PREPROC_CURR_Y_CM_G8             = 0,
    CODECHAL_ENCODE_AVC_PREPROC_CURR_UV_CM_G8            = 1,
    CODECHAL_ENCODE_AVC_PREPROC_HME_MV_DATA_CM_G8        = 2,
    CODECHAL_ENCODE_AVC_PREPROC_MV_PREDICTOR_CM_G8       = 3,
    CODECHAL_ENCODE_AVC_PREPROC_MBQP_CM_G8               = 4,
    CODECHAL_ENCODE_AVC_PREPROC_MV_DATA_CM_G8            = 5,
    CODECHAL_ENCODE_AVC_PREPROC_MB_STATS_CM_G8           = 6,
    CODECHAL_ENCODE_AVC_PREPROC_VME_CURR_PIC_IDX_0_CM_G8 = 7,
    CODECHAL_ENCODE_AVC_PREPROC_VME_FWD_PIC_IDX0_CM_G8   = 8,
    CODECHAL_ENCODE_AVC_PREPROC_VME_BWD_PIC_IDX0_0_CM_G8 = 9,
    CODECHAL_ENCODE_AVC_PREPROC_VME_CURR_PIC_IDX_1_CM_G8 = 10,
    CODECHAL_ENCODE_AVC_PREPROC_VME_BWD_PIC_IDX0_1_CM_G8 = 11,
    CODECHAL_ENCODE_AVC_PREPROC_RESERVED1_CM_G8          = 12,
    CODECHAL_ENCODE_AVC_PREPROC_FTQ_LUT_CM_G8            = 13,
    CODECHAL_ENCODE_AVC_PREPROC_NUM_SURFACES_CM_G8       = 14
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_PREPROC_CM_G8;

typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_PREPROC_FIELD_CM_G8
{
    CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_CURR_PIC_IDX_0_CM_G8 = 7,
    CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_FWD_PIC_IDX0_0_CM_G8 = 8,
    CODECHAL_ENCODE_AVC_PREPROC_FIELD_RESERVED0_CM_G8          = 9,
    CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_FWD_PIC_IDX0_1_CM_G8 = 10,
    CODECHAL_ENCODE_AVC_PREPROC_FIELD_RESERVED1_CM_G8          = 11,
    CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_CURR_PIC_IDX_1_CM_G8 = 12,
    CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_BWD_PIC_IDX0_0_CM_G8 = 13,
    CODECHAL_ENCODE_AVC_PREPROC_FIELD_RESERVED2_CM_G8          = 14,
    CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_BWD_PIC_IDX0_1_CM_G8 = 15,
    CODECHAL_ENCODE_AVC_PREPROC_FIELD_RESERVED3_CM_G8          = 16,
    CODECHAL_ENCODE_AVC_PREPROC_FIELD_FTQ_LUT_CM_G8            = 17,
    CODECHAL_ENCODE_AVC_PREPROC_FIELD_NUM_SURFACES_CM_G8       = 18
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_PREPROC_FIELD_CM_G8;

CodechalEncodeAvcEncFeiG8::CodechalEncodeAvcEncFeiG8(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo) : CodechalEncodeAvcEncG8(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_cmKernelEnable = true;
    bHighTextureModeCostEnable = false;

#ifdef FEI_ENABLE_CMRT
    m_avcCmSurfIdx = nullptr;
    m_avcCmSurfIdx = MOS_New(CODECHAL_ENCODE_AVC_SURFACE_IDX);
#endif
    this->pfnGetKernelHeaderAndSize = this->EncodeGetKernelHeaderAndSize;
    m_feiEnable = true;

    if (m_codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC)
    {
        m_hmeSupported = true;
        m_flatnessCheckSupported = true;
    }

    m_16xMeSupported = false;
    m_32xMeSupported = false;

    //FEI output Stats which is a superset of MbStats buffer, so no need for MbStats
    m_mbStatsSupported = false;
    m_kuid = IDR_CODEC_AllAVCEnc_FEI;
    AddIshSize(m_kuid, m_kernelBase);
}

CodechalEncodeAvcEncFeiG8::~CodechalEncodeAvcEncFeiG8()
{
#ifdef FEI_ENABLE_CMRT
    CodecHalEncode_DestroyMDFKernelResource(this,&m_resMBEncKernel);
    CodecHalEncode_DestroyMDFKernelResource(this,&m_resPreProcKernel);
    CodecHalEncode_DestroyMDFKernelResource(this,&m_resMEKernel);
    CodecHalEncode_DestroyMDFKernelResource(this,&resDSKernel);
    if(nullptr != m_avcCmSurfIdx)
    {
        MOS_Delete((PCODECHAL_ENCODE_AVC_SURFACE_IDX)m_avcCmSurfIdx);
        m_avcCmSurfIdx = nullptr;
    }
#endif
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::InitializePicture(const EncoderParams& params)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    if (m_codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC)
    {
        return EncodePreEncInitialize(params);
    }
    else
    {
        return CodechalEncodeAvcEnc::InitializePicture( params);
    }
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::EncodePreEncInitialize(const EncoderParams& params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto preEncParams = (FeiPreEncParams*)params.pPreEncParams;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(preEncParams->psCurrOriginalSurface);

    auto avcRefList = &m_refList[0];
    auto avcPicIdx = &m_picIdx[0];

    CodecHalGetResourceInfo(m_osInterface, preEncParams->psCurrOriginalSurface);
    m_rawSurface = *(preEncParams->psCurrOriginalSurface);
    m_rawSurfaceToEnc =
        m_rawSurfaceToPak = &m_rawSurface;
    m_targetUsage = TARGETUSAGE_RT_SPEED;
    m_kernelMode = CodecHal_TargetUsageToMode_AVC[m_targetUsage];
    m_flatnessCheckEnabled = m_flatnessCheckSupported;

    auto prevPic = m_currOriginalPic;
    uint8_t prevIdx = prevPic.FrameIdx;
    uint8_t currIdx = preEncParams->CurrOriginalPicture.FrameIdx;
    bool firstFieldInput = (prevPic.PicFlags == PICTURE_INVALID) && ((preEncParams->CurrOriginalPicture.PicFlags == PICTURE_TOP_FIELD) || (preEncParams->CurrOriginalPicture.PicFlags == PICTURE_BOTTOM_FIELD));

    avcRefList[currIdx]->sRefBuffer = avcRefList[currIdx]->sRefRawBuffer = m_rawSurface;
    avcRefList[currIdx]->RefPic = m_currOriginalPic;
    avcRefList[currIdx]->bUsedAsRef = true;

    // FEI PreEnc doesn't have CurrReconstructedPicture, here we set m_currReconstructedPic = pPreEncParams->CurrOriginalPicture
    // so it can resue the AVC functions.
    m_currOriginalPic = preEncParams->CurrOriginalPicture;
    m_currReconstructedPic = preEncParams->CurrOriginalPicture;
    m_currRefList = m_refList[m_currOriginalPic.FrameIdx];

    uint8_t numRef = 0;
    for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        avcPicIdx[i].bValid = false;
    }

    // FEI only support one past and one future reference picture now
    uint8_t index;
    if (preEncParams->dwNumPastReferences > 0)
    {
        CODECHAL_ENCODE_ASSERT(preEncParams->dwNumPastReferences == 1);
        CODECHAL_ENCODE_ASSERT(!CodecHal_PictureIsInvalid(preEncParams->PastRefPicture));
        index = preEncParams->PastRefPicture.FrameIdx;
        avcPicIdx[numRef].bValid = true;
        avcPicIdx[numRef].ucPicIdx = index;
        avcRefList[index]->RefPic.PicFlags =
            CodecHal_CombinePictureFlags(avcRefList[index]->RefPic, preEncParams->PastRefPicture);
        avcRefList[currIdx]->RefList[numRef] = preEncParams->PastRefPicture;
        numRef++;
    }

    if (preEncParams->dwNumFutureReferences > 0)
    {
        CODECHAL_ENCODE_ASSERT(preEncParams->dwNumFutureReferences == 1);
        CODECHAL_ENCODE_ASSERT(!CodecHal_PictureIsInvalid(preEncParams->FutureRefPicture))
            index = preEncParams->FutureRefPicture.FrameIdx;
        avcPicIdx[numRef].bValid = true;
        avcPicIdx[numRef].ucPicIdx = index;
        avcRefList[index]->RefPic.PicFlags =
            CodecHal_CombinePictureFlags(avcRefList[index]->RefPic, preEncParams->FutureRefPicture);
        avcRefList[currIdx]->RefList[numRef] = preEncParams->FutureRefPicture;
        numRef++;
    }
    avcRefList[currIdx]->ucNumRef = numRef;

    m_verticalLineStride = CODECHAL_VLINESTRIDE_FRAME;
    m_verticalLineStrideOffset = CODECHAL_VLINESTRIDEOFFSET_TOP_FIELD;
    m_meDistortionBottomFieldOffset = 0;
    m_meMvBottomFieldOffset = 0;

    if (CodecHal_PictureIsField(m_currOriginalPic))
    {
        m_frameFieldHeight = ((m_frameHeight + 1) >> 1);
        m_frameFieldHeightInMb = ((m_picHeightInMb + 1) >> 1);
        m_downscaledFrameFieldHeightInMb4x = ((m_downscaledHeightInMb4x + 1) >> 1);
        m_downscaledFrameFieldHeightInMb16x = ((m_downscaledHeightInMb16x + 1) >> 1);
        m_downscaledFrameFieldHeightInMb32x = ((m_downscaledHeightInMb32x + 1) >> 1);
        if (CodecHal_PictureIsFrame(prevPic) || prevIdx != currIdx || firstFieldInput)
        {
            m_firstField = 1;
        }
        else
        {
            m_firstField = 0;
        }

        m_verticalLineStride = CODECHAL_VLINESTRIDE_FIELD;
        m_frameHeight = m_frameFieldHeightInMb * 2 * 16;
        m_picHeightInMb = (uint16_t)(m_frameHeight / 16);
        m_mbvProcStatsBottomFieldOffset = 0;

        if (CodecHal_PictureIsBottomField(m_currOriginalPic))
        {
            m_verticalLineStrideOffset = CODECHAL_VLINESTRIDEOFFSET_BOT_FIELD;

            m_meDistortionBottomFieldOffset =
                MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) *
                MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4 * 10), 8);
            m_meMvBottomFieldOffset =
                MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64) *
                (m_downscaledFrameFieldHeightInMb4x * 4);
        }
    }
    else
    {
        m_frameFieldHeight = m_frameHeight;
        m_frameFieldHeightInMb = m_picHeightInMb;
        m_downscaledFrameFieldHeightInMb4x = m_downscaledHeightInMb4x;
        m_downscaledFrameFieldHeightInMb16x = m_downscaledHeightInMb16x;
        m_downscaledFrameFieldHeightInMb32x = m_downscaledHeightInMb32x;
        m_firstField = 1;
    }

    if ((preEncParams->bDisableMVOutput == 1) && (preEncParams->bDisableStatisticsOutput == 1))
    {
        m_hmeEnabled = false;
    }
    else
    {
        m_hmeEnabled = (preEncParams->dwNumPastReferences + preEncParams->dwNumFutureReferences) > 0 ? true : false;
    }
    m_scalingEnabled = m_firstField && preEncParams->bInputUpdated;
    m_useRawForRef = m_userFlags.bUseRawPicForRef;

    m_scalingEnabled = (m_hmeSupported || bBrcEnabled) && m_firstField;

    // PREENC doesn't differentiate picture by its "PictureCodingType". However in order to reuse the CODECHAL function,
    // we manually assign the PictureCodingType given its past/future references.
    if (preEncParams->dwNumFutureReferences > 0)
    {
        m_pictureCodingType = B_TYPE;
    }
    else if (preEncParams->dwNumPastReferences > 0)
    {
        m_pictureCodingType = P_TYPE;
    }
    else
    {
        m_pictureCodingType = I_TYPE;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetStatusReportParams(
        m_refList[m_currOriginalPic.FrameIdx]));

    CODECHAL_DEBUG_TOOL(
        m_debugInterface->m_currPic            = m_currOriginalPic;
        m_debugInterface->m_bufferDumpFrameNum = m_storeData;
        m_debugInterface->m_frameType          = m_pictureCodingType;)

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::ExecuteKernelFunctions()
{
    if (m_codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC)
    {
        return EncodePreEncKernelFunctions();
    }
    else
    {
#ifdef FEI_ENABLE_CMRT
        return EncodeMbEncKernelFunctions();
#else
        return CodechalEncodeAvcEncG8::ExecuteKernelFunctions();
#endif
    }
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::EncodePreEncKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto preEncParams = (FeiPreEncParams*)m_encodeParams.pPreEncParams;
    auto avcRefList = &m_refList[0];

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
        m_rawSurfaceToEnc,
        CodechalDbgAttr::attrEncodeRawInputSurface,
        "SrcSurf"))
    );

    // save these 2 flags
    auto firstField = m_firstField;
    auto currRefList = m_currRefList;

    // Scaling, HME and PreProc are included in the same task phase
    m_lastEncPhase = true;
    m_firstTaskInPhase = true;

    UpdateSSDSliceCount();

    m_trackedBuf->ResetUsedForCurrFrame();

    bool dsSurfaceInCache;
    uint8_t scaledIdx = m_trackedBuf->PreencLookUpBufIndex(m_currOriginalPic.FrameIdx, &dsSurfaceInCache);
    m_trackedBuf->AllocateForCurrFramePreenc(scaledIdx);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_trackedBuf->AllocateSurfaceDS());

    bool dsPastRefInCache = false;
    bool callDsPastRef = false;
    uint8_t pastRefScaledIdx = 0;
    if (preEncParams->dwNumPastReferences > 0)
    {
        pastRefScaledIdx = m_trackedBuf->PreencLookUpBufIndex(preEncParams->PastRefPicture.FrameIdx, &dsPastRefInCache);
        if ((!preEncParams->bPastRefUpdated) && dsPastRefInCache)
        {
            CODECHAL_ENCODE_VERBOSEMESSAGE("find PastRef Downscaled Surface in cache, so skip DS");
        }
        else
        {
            callDsPastRef = true;
        }
    }

    bool dsFutureRefInCache = false;
    bool callDsFutureRef = false;
    uint8_t futureRefScaledIdx = 0;
    if (preEncParams->dwNumFutureReferences > 0)
    {
        futureRefScaledIdx = m_trackedBuf->PreencLookUpBufIndex(preEncParams->FutureRefPicture.FrameIdx, &dsFutureRefInCache);
        if ((!preEncParams->bFutureRefUpdated) && dsFutureRefInCache)
        {
            CODECHAL_ENCODE_VERBOSEMESSAGE("find FutureRef Downscaled Surface in cache, so skip DS");
        }
        else
        {
            callDsFutureRef = true;
        }
    }

    bool callPreEncKernel = (preEncParams->bDisableMVOutput == 0) || (preEncParams->bDisableStatisticsOutput == 0);

    CodechalEncodeCscDs::KernelParams cscScalingKernelParams;
#ifdef FEI_ENABLE_CMRT
    m_dsKernelIdx = 0;
#endif
    if ((!preEncParams->bCurPicUpdated) && dsSurfaceInCache)
    {
        CODECHAL_ENCODE_VERBOSEMESSAGE("find Downscaled Surface in cache, so skip DS");
    }
    else
    {
        if (!preEncParams->bCurPicUpdated)
        {
            CODECHAL_ENCODE_VERBOSEMESSAGE("Cannot find matched Downscaled Surface, DS again");
        }
        if (scaledIdx == CODEC_NUM_TRACKED_BUFFERS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Cannot find empty DS slot for preenc.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        m_firstField = preEncParams->bCurPicUpdated ? 1 : m_firstField;
        m_currRefList->ucScalingIdx = scaledIdx;
        m_currRefList->b4xScalingUsed = false;
        m_currRefList->b16xScalingUsed = false;
        m_currRefList->b32xScalingUsed = false;

        MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));
        cscScalingKernelParams.bLastTaskInPhase4xDS = !(callDsPastRef || callDsFutureRef || m_hmeEnabled || callPreEncKernel);
        cscScalingKernelParams.b32xScalingInUse = false;
        cscScalingKernelParams.b16xScalingInUse = false;
        cscScalingKernelParams.bLastTaskInPhase4xDS = !m_hmeEnabled;
#ifdef FEI_ENABLE_CMRT
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeScalingKernel(&cscScalingKernelParams));
        m_dsKernelIdx ++;
#else
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->DsKernel(&cscScalingKernelParams));
#endif

    }

    // Scaling for Past ref
    if (callDsPastRef)
    {
        if (!preEncParams->bPastRefUpdated)
        {
            CODECHAL_ENCODE_VERBOSEMESSAGE("Cannot find matched Downscaled Surface, DS again");
        }
        if (pastRefScaledIdx == CODEC_NUM_TRACKED_BUFFERS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Cannot find empty DS slot for preenc.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        uint8_t pastRefIdx = preEncParams->PastRefPicture.FrameIdx;
        avcRefList[pastRefIdx]->sRefBuffer = avcRefList[pastRefIdx]->sRefRawBuffer = preEncParams->sPastRefSurface;
        avcRefList[pastRefIdx]->RefPic = preEncParams->PastRefPicture;
        avcRefList[pastRefIdx]->bUsedAsRef = true;
        m_firstField = true;
        m_currRefList = avcRefList[pastRefIdx];
        m_currRefList->ucScalingIdx = pastRefScaledIdx;
        m_currRefList->b4xScalingUsed = false;
        m_currRefList->b16xScalingUsed = false;
        m_currRefList->b32xScalingUsed = false;

        MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));
        cscScalingKernelParams.bLastTaskInPhase4xDS = !(callDsFutureRef || m_hmeEnabled || callPreEncKernel);
        cscScalingKernelParams.b32xScalingInUse = false;
        cscScalingKernelParams.b16xScalingInUse = false;
        cscScalingKernelParams.bRawInputProvided = true;
        cscScalingKernelParams.bScalingforRef = true;
        cscScalingKernelParams.sInputRawSurface = preEncParams->sPastRefSurface;
        cscScalingKernelParams.inputPicture = preEncParams->PastRefPicture;

        if (preEncParams->bPastRefStatsNeeded)
        {
            cscScalingKernelParams.sInputStatsBuffer = preEncParams->sPastRefStatsBuffer;
            if (CodecHal_PictureIsField(m_currOriginalPic))
            {
                cscScalingKernelParams.sInputStatsBotFieldBuffer = preEncParams->sPastRefStatsBotFieldBuffer;
            }
            cscScalingKernelParams.bStatsInputProvided = true;
        }
#ifdef FEI_ENABLE_CMRT
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeScalingKernel(&cscScalingKernelParams));
        m_dsKernelIdx++;
#else
        m_trackedBuf->AllocateForCurrFramePreenc(pastRefScaledIdx);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->DsKernel(&cscScalingKernelParams));
#endif
    }

    // Scaling for Future ref
    if (callDsFutureRef)
    {
        if (!preEncParams->bFutureRefUpdated)
        {
            CODECHAL_ENCODE_VERBOSEMESSAGE("Cannot find matched Downscaled Surface, DS again");
        }
        if (futureRefScaledIdx == CODEC_NUM_TRACKED_BUFFERS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Cannot find empty DS slot for preenc.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        uint8_t futureRefIdx = preEncParams->FutureRefPicture.FrameIdx;
        avcRefList[futureRefIdx]->sRefBuffer = avcRefList[futureRefIdx]->sRefRawBuffer = preEncParams->sFutureRefSurface;
        avcRefList[futureRefIdx]->RefPic = preEncParams->FutureRefPicture;
        avcRefList[futureRefIdx]->bUsedAsRef = true;
        m_firstField = true;
        m_currRefList = avcRefList[futureRefIdx];
        m_currRefList->ucScalingIdx = futureRefScaledIdx;
        m_currRefList->b4xScalingUsed = false;
        m_currRefList->b16xScalingUsed = false;
        m_currRefList->b32xScalingUsed = false;

        m_lastTaskInPhase = false;
        MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));
        cscScalingKernelParams.bLastTaskInPhase4xDS = !(m_hmeEnabled || callPreEncKernel);
        cscScalingKernelParams.b32xScalingInUse = false;
        cscScalingKernelParams.b16xScalingInUse = false;
        cscScalingKernelParams.bRawInputProvided = true;
        cscScalingKernelParams.bScalingforRef = true;
        cscScalingKernelParams.sInputRawSurface = preEncParams->sFutureRefSurface;
        cscScalingKernelParams.inputPicture = preEncParams->FutureRefPicture;

        if (preEncParams->bFutureRefStatsNeeded)
        {
            cscScalingKernelParams.sInputStatsBuffer = preEncParams->sFutureRefStatsBuffer;
            if (CodecHal_PictureIsField(m_currOriginalPic))
            {
                cscScalingKernelParams.sInputStatsBotFieldBuffer = preEncParams->sFutureRefStatsBotFieldBuffer;
            }
            cscScalingKernelParams.bStatsInputProvided = true;
        }
#ifdef FEI_ENABLE_CMRT
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeScalingKernel(&cscScalingKernelParams));
        m_dsKernelIdx++;
#else
        m_trackedBuf->AllocateForCurrFramePreenc(futureRefScaledIdx);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->DsKernel(&cscScalingKernelParams));
#endif
    }

    m_firstField = firstField;
    m_currRefList = currRefList;

    if (m_hmeEnabled)
    {
        CODEC_AVC_ENCODE_SLICE_PARAMS     avcSliceParams;

        memset(&avcSliceParams, 0, sizeof(CODEC_AVC_ENCODE_SLICE_PARAMS));
        if (preEncParams->dwNumPastReferences > 0)
        {
            avcSliceParams.num_ref_idx_l0_active_minus1 = 0;
            avcSliceParams.RefPicList[LIST_0][0] = preEncParams->PastRefPicture;
            avcSliceParams.RefPicList[LIST_0][0].FrameIdx = 0;
        }
        else
        {
            avcSliceParams.RefPicList[LIST_0][0].PicFlags = PICTURE_INVALID;
        }

        if (preEncParams->dwNumFutureReferences > 0)
        {
            avcSliceParams.num_ref_idx_l1_active_minus1 = 0;
            avcSliceParams.RefPicList[LIST_1][0] = preEncParams->FutureRefPicture;
            avcSliceParams.RefPicList[LIST_1][0].FrameIdx = (preEncParams->dwNumPastReferences > 0) ? 1 : 0;
        }
        else
        {
            avcSliceParams.RefPicList[LIST_1][0].PicFlags = PICTURE_INVALID;
        }

        m_avcSliceParams = &avcSliceParams;
        m_lastTaskInPhase = !callPreEncKernel;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GenericEncodeMeKernel(&BrcBuffers, HME_LEVEL_4x));

    }

    m_lastTaskInPhase = true;
    if (callPreEncKernel)
    {
        // Execute the PreEnc kernel only when MV and/or Statistics output required.
        // Especially for I frame case, if user disables the statistics output, the PreEnc can be skipped
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PreProcKernel());
        if(preEncParams->bMBQp){
            CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                     &preEncParams->resMbQpBuffer,
                     CodechalDbgAttr::attrInput,
                     "MbQp",
                      m_picWidthInMb * m_frameFieldHeightInMb,
                     0,
                     CODECHAL_MEDIA_STATE_PREPROC)));
        }
        if(preEncParams->dwMVPredictorCtrl){
            CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                     &preEncParams->resMvPredBuffer,
                     CodechalDbgAttr::attrInput,
                     "MvPredictor",
                     m_picWidthInMb * m_frameFieldHeightInMb * 8,
                     0,
                     CODECHAL_MEDIA_STATE_PREPROC)));
        }

    }
#ifdef FEI_ENABLE_CMRT
    CodecHalEncode_FreeMDFKernelSurfaces(this, &m_resPreProcKernel);
    CodecHalEncode_FreeMDFKernelSurfaces(this, &m_resMEKernel);
    CodecHalEncode_FreeMDFKernelSurfaces(this, &resDSKernel);
#endif
    // Reset buffer ID used for MbEnc kernel performance reports
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    m_setRequestedEUSlices = false;

    return eStatus;
}

#ifdef FEI_ENABLE_CMRT
MOS_STATUS CodechalEncodeAvcEncFeiG8::DispatchKernelMe(SurfaceIndex** surfIdxArray,uint16_t width,uint16_t height,bool isBFrame)
{
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmDev);

    auto kernelRes = &m_resMEKernel;
    CmKernel * pKernel;
    if (isBFrame)
    {
       pKernel = kernelRes->ppKernel[1];
    }
    else
    {
       pKernel = kernelRes->ppKernel[0];
    }

    //identify field or frame from curbe
    bool isField = (kernelRes->pCurbe[12] >> 7) & 1;
    // config thread space
    uint32_t           threadWidth   = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(width);
    uint32_t           threadHeight  = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(height);
    if (isField)
    {
        threadHeight = (height + 31) >> 5;
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetThreadCount(threadWidth * threadHeight));

    uint32_t kernelArgIdx = 0;
    //curbe data
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(kernelArgIdx++, m_feiMeCurbeDataSize, kernelRes->pCurbe));
    // output surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(kernelArgIdx++, sizeof(SurfaceIndex), surfIdxArray[0]));
    // input MV surface. if not provided, set to output surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(kernelArgIdx++, sizeof(SurfaceIndex), surfIdxArray[1]));
    // dist surfaces
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(kernelArgIdx++, sizeof(SurfaceIndex), surfIdxArray[2]));
    // dist
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(kernelArgIdx++, sizeof(SurfaceIndex), surfIdxArray[3]));
    // fwd ref surfaces. if not provided, set to output surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(kernelArgIdx++, sizeof(SurfaceIndex), surfIdxArray[4]));
    // bwd ref surfaces. if not provided, set to output surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(kernelArgIdx++, sizeof(SurfaceIndex), surfIdxArray[5]));
    if(kernelRes->pTS == nullptr)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateThreadSpace(threadWidth, threadHeight, kernelRes->pTS));
    }

    bool            isEnqueue = false;
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        isEnqueue = true;
        m_lastTaskInPhase = false;
    }
    AddKernelMdf(pCmDev,pCmQueue,pKernel,pCmTask, kernelRes->pTS,kernelRes->e,isEnqueue);
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::EncodeMeKernel(EncodeBrcBuffers* brcBuffers, hmeLevel hmeLevel)
{
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmDev);

    auto kernelRes             = &m_resMEKernel;
    auto cmSurf                = kernelRes->ppCmSurf;
    auto cmVmeSurfIdx        = kernelRes->ppCmVmeSurf;

    // Setup AVC Curbe
    MeCurbeParams    meParams;
    MOS_ZeroMemory(&meParams, sizeof(meParams));
    meParams.hmeLvl = hmeLevel;
    meParams.pCurbeBinary = kernelRes->pCurbe;

    SetCurbeMe(&meParams);
    SurfaceIndex *cmSurfIdx[6];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&m_4xMeMvDataBuffer.OsResource, cmSurf[0]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[0]->GetIndex(cmSurfIdx[0]));
    cmSurfIdx[1] = cmSurfIdx[0];
    cmSurfIdx[3] = (SurfaceIndex *)CM_NULL_SURFACE;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&m_4xMeDistortionBuffer.OsResource, cmSurf[2]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[2]->GetIndex(cmSurfIdx[2]));

    //maxium reference L0 is 4
    uint32_t baseIdx = 4;//FWD_BASE_INDEX;
    //for FEI, m_currReconstructedPic = m_currOriginalPic
    //scaledIdx = m_refList[m_currReconstructedPic.FrameIdx]->ucScalingIdx;
    uint8_t scaledIdx = m_refList[m_currOriginalPic.FrameIdx]->ucScalingIdx;
    PMOS_SURFACE currScaledSurface = m_trackedBuf->Get4xDsSurface(scaledIdx);

    SurfaceIndex                *cmTmpSurfIdx[3];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&currScaledSurface->OsResource, cmSurf[baseIdx]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[baseIdx]->GetIndex(cmTmpSurfIdx[0]));

    uint32_t numRef = 0;
    CmSurface2D             * surfArray[8];
    for(uint8_t refIdx = 0; refIdx <= m_avcSliceParams->num_ref_idx_l0_active_minus1;refIdx ++)
    {
        CODEC_PICTURE refPic =  m_avcSliceParams->RefPicList[LIST_0][refIdx];
        if(!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
        {
            uint8_t refPicIdx = m_picIdx[refPic.FrameIdx].ucPicIdx;
            uint8_t scaledIdx = m_refList[refPicIdx]->ucScalingIdx;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&m_trackedBuf->Get4xDsSurface(scaledIdx)->OsResource, cmSurf[baseIdx + 1 + refIdx]));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[baseIdx + 1 + refIdx]->GetIndex(cmTmpSurfIdx[1]));

            surfArray[numRef] = (CmSurface2D*)cmSurf[baseIdx + 1 + refIdx];
            numRef ++;
        }
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateVmeSurfaceG7_5(cmSurf[baseIdx], surfArray, nullptr, numRef, 0, cmVmeSurfIdx[0]));

    baseIdx = 9;
    numRef = 0;
    auto cmSurfForVME = cmSurf[4];
    //maxium L1 size is 2
    for(uint8_t refIdx = 0; refIdx <= m_avcSliceParams->num_ref_idx_l1_active_minus1;refIdx ++)
    {
        CODEC_PICTURE refPic =  m_avcSliceParams->RefPicList[LIST_1][refIdx];
        if(!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
        {
            uint8_t refPicIdx = m_picIdx[refPic.FrameIdx].ucPicIdx;
            uint8_t scaledIdx = m_refList[refPicIdx]->ucScalingIdx;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&m_trackedBuf->Get4xDsSurface(scaledIdx)->OsResource, cmSurf[baseIdx + 1 + refIdx]));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[baseIdx + 1 + refIdx]->GetIndex(cmTmpSurfIdx[2]));

            surfArray[numRef] = (CmSurface2D*)cmSurf[baseIdx + 1 + refIdx];
            numRef ++;
        }
     }
     CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateVmeSurfaceG7_5( cmSurfForVME, surfArray, nullptr, numRef, 0, cmVmeSurfIdx[1]));
     cmSurfIdx[4] = cmVmeSurfIdx[0];
     cmSurfIdx[5] = cmVmeSurfIdx[1];

     bool isB = (m_pictureCodingType == B_TYPE);

     CODECHAL_ENCODE_CHK_STATUS_RETURN(DispatchKernelMe(
                                           &cmSurfIdx[0],
                                           (uint16_t)(m_frameWidth / 4), //CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth / 4),
                                           (uint16_t)(m_frameHeight/4),//CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / 4),
                                           isB));
     return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::InitKernelStateMe()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto kernelRes = &m_resMEKernel;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncode_CreateMDFKernelResource(this, kernelRes, 2, m_mdfMeBufSize, m_mdfMeSurfSize, m_mdfMeVmeSurfSize, m_feiMeCurbeDataSize));
    uint32_t codeSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_ReadFileToPtr(strMeIsaName, &codeSize, &kernelRes->pCommonISA));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->LoadProgram(kernelRes->pCommonISA, codeSize, kernelRes->pCmProgram, "-nojitter"));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateKernel(kernelRes->pCmProgram, "HME_P", kernelRes->ppKernel[0]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateKernel(kernelRes->pCmProgram, "HME_B", kernelRes->ppKernel[1]));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::InitKernelStateScaling(PCODECHAL_ENCODER avcEncoder)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(avcEncoder->pCmDev);

    auto kernelRes  = &avcEncoder->resDSKernel;
    CodecHalEncode_CreateMDFKernelResource(this, kernelRes, 6, m_mdfDsBufSize * 3, m_mdfDsSurfSize * 3, m_mdfDsVmeSurfSize, 0);

    uint32_t codeSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_ReadFileToPtr(strDsIsaName, &codeSize, &kernelRes->pCommonISA));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(avcEncoder->pCmDev->LoadProgram(kernelRes->pCommonISA, codeSize, kernelRes->pCmProgram, "-nojitter"));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(avcEncoder->pCmDev->CreateKernel(kernelRes->pCmProgram, "hme_frame_downscale", kernelRes->ppKernel[0]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(avcEncoder->pCmDev->CreateKernel(kernelRes->pCmProgram, "hme_frame_downscale", kernelRes->ppKernel[1]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(avcEncoder->pCmDev->CreateKernel(kernelRes->pCmProgram, "hme_frame_downscale", kernelRes->ppKernel[2]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(avcEncoder->pCmDev->CreateKernel(kernelRes->pCmProgram, "hme_field_downscale", kernelRes->ppKernel[3]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(avcEncoder->pCmDev->CreateKernel(kernelRes->pCmProgram, "hme_field_downscale", kernelRes->ppKernel[4]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(avcEncoder->pCmDev->CreateKernel(kernelRes->pCmProgram, "hme_field_downscale", kernelRes->ppKernel[5]));
    kernelRes->e = CM_NO_EVENT;
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::DispatchKernelScaling(
                                                uint32_t flatnessThreshold,
                                                uint32_t options,
                                                uint16_t sourceWidth,
                                                uint16_t sourceHeight,
                                                uint32_t kernelType,
                                                SurfaceIndex** surfIdxArray)
{

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmDev);

    uint16_t  sourceFieldHeight         = (sourceHeight + 1) >> 1;
    uint16_t  ds4xWidth                  = ((sourceWidth + 32) >> 6) << 4;
    uint16_t  dx4xHeight                 = ((sourceHeight + 32) >> 6) << 4;
    uint16_t  ds4xFieldWidth            = ds4xWidth;
    uint16_t  ds4xFieldHeight           = ((sourceFieldHeight + 32) >> 6) << 4;

    //if the width or height is less than 48 which is the search window size, ceil it to 48
    if (ds4xWidth<48)
    {
        ds4xWidth = 48;
        ds4xFieldWidth = 48;
    }
    if (dx4xHeight<48)
    {
        dx4xHeight = 48;
        ds4xFieldHeight = 24;
    }

    int  threadSpaceWidth = (ds4xWidth + 4)>>3;           // Each 8x8 pixel output is completed by 1 thread
    int  threadSpaceHeight = (dx4xHeight + 4) >> 3;       // Each 8x8 pixel output is completed by 1 thread
    if (kernelType == 1){
        threadSpaceWidth = (ds4xFieldWidth + 4) >> 3;    // Each 4x8 pixel output is completed by 1 thread
        threadSpaceHeight = (ds4xFieldHeight + 2) >> 2;  // Each 4x8 pixel output is completed by 1 thread
    }

    uint32_t reserved[3];
    reserved[0] = 0;
    reserved[1] = 0;
    reserved[2] = 0;
    auto kernelRes = &resDSKernel;

    SurfaceIndex* surfaceIndex_SrcSurf_Top = nullptr;
    SurfaceIndex* surfaceIndex_SrcSurf_Bot = nullptr;
    SurfaceIndex* surfaceIndex_DSSurf_Top  = nullptr;
    SurfaceIndex* surfaceIndex_DSSurf_Bot  = nullptr;
    SurfaceIndex* surfaceIndex_MB_VProc_Stats_Top = nullptr;
    SurfaceIndex* surfaceIndex_MB_VProc_Stats_Bot = nullptr;

    CmKernel* kernel;
    if (kernelType == 0)
    {
        kernel = kernelRes->ppKernel[m_dsKernelIdx];
        CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetThreadCount(threadSpaceWidth*threadSpaceHeight));

        surfaceIndex_SrcSurf_Top = surfIdxArray[0];
        surfaceIndex_DSSurf_Top = surfIdxArray[1];
        surfaceIndex_MB_VProc_Stats_Top = surfIdxArray[4];
        surfaceIndex_MB_VProc_Stats_Bot = surfIdxArray[4];

        CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(4, sizeof(reserved[0]), &reserved[0]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(5, sizeof(reserved[1]), &reserved[1]));
    }
    else
    {
        kernel = kernelRes->ppKernel[m_dsKernelIdx + 3];
        CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetThreadCount(threadSpaceWidth*threadSpaceHeight));

        surfaceIndex_SrcSurf_Top = surfIdxArray[0];
        surfaceIndex_DSSurf_Top  = surfIdxArray[1];
        surfaceIndex_SrcSurf_Bot = surfIdxArray[2];
        surfaceIndex_DSSurf_Bot  = surfIdxArray[3];

        surfaceIndex_MB_VProc_Stats_Top = surfIdxArray[4];
        surfaceIndex_MB_VProc_Stats_Bot = surfIdxArray[5];
        CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(4, sizeof(SurfaceIndex), surfaceIndex_SrcSurf_Bot)); // DW3
        CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(5, sizeof(SurfaceIndex), surfaceIndex_DSSurf_Bot)); // DW4
        CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(10, sizeof(SurfaceIndex), surfaceIndex_MB_VProc_Stats_Bot)); // DW9
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(0, sizeof(uint16_t), &sourceWidth));      // DW0
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(1, sizeof(uint16_t), &sourceHeight));     // DW0
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(2, sizeof(SurfaceIndex), surfaceIndex_SrcSurf_Top)); // DW1
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(3, sizeof(SurfaceIndex), surfaceIndex_DSSurf_Top)); // DW2
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(6, sizeof(uint32_t), &flatnessThreshold));    // DW5
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(7, sizeof(uint32_t), &options));                   // DW6
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(8, sizeof(uint32_t), &reserved[2]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(9, sizeof(SurfaceIndex), surfaceIndex_MB_VProc_Stats_Top));// DW8

    // Setup Dispatch Pattern ======================================================
    bool            isEnqueue                  = false;

    if(kernelRes->pTS == nullptr)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateThreadSpace(threadSpaceWidth, threadSpaceHeight, kernelRes->pTS));
    }
    if((!m_singleTaskPhaseSupported) || m_lastTaskInPhase)
    {
        isEnqueue = true;
        m_lastTaskInPhase = false;
    }
    return AddKernelMdf(pCmDev,pCmQueue,kernel,pCmTask,kernelRes->pTS,kernelRes->e,isEnqueue);

}

MOS_STATUS CodechalEncodeAvcEncFeiG8::EncodeScalingKernel(CodechalEncodeCscDs::KernelParams* pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto kernelRes = &resDSKernel;
    auto cmSurf    = &kernelRes->ppCmSurf[m_dsKernelIdx * m_mdfDsBufSize];
    auto cmBuf     = &kernelRes->ppCmBuf[m_dsKernelIdx * m_mdfDsBufSize];

    m_lastTaskInPhase  = pParams->bLastTaskInPhase4xDS;

    // setup kernel required parameters
    if (!m_firstField)
    {
        // Both fields are scaled when the first field comes in, no need to scale again
        return eStatus;
    }

    auto preEncParams = (FeiPreEncParams*)m_encodeParams.pPreEncParams;
    CODECHAL_ENCODE_CHK_NULL_RETURN(preEncParams);
    bool fieldPicture = CodecHal_PictureIsField(m_currOriginalPic);

    //only 4x
    uint16_t srcWidth  = (uint16_t)m_oriFrameWidth;
    uint16_t srcHeight = (uint16_t)m_oriFrameHeight;
    auto inputSurface  = (pParams->bRawInputProvided)? &pParams->sInputRawSurface : m_rawSurfaceToEnc;
    auto outputSurface = m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
    m_currRefList->b4xScalingUsed = true;

    uint32_t enableFlatness   = 0;
    uint32_t enableVarianceOutput = 0;
    uint32_t enableAverageOutput = 0;
    uint32_t enable8x8Stats = 0;
    uint32_t flatnessTh  = 0;

    if((!pParams->bScalingforRef) || (pParams->bStatsInputProvided))
    {
        enableFlatness       = (!pParams->b32xScalingInUse && !pParams->b16xScalingInUse) ? m_flatnessCheckEnabled : 0;
        enableVarianceOutput = (preEncParams) ? !preEncParams->bDisableStatisticsOutput : ((!pParams->b32xScalingInUse && !pParams->b16xScalingInUse) ? m_mbStatsEnabled : 0);
        enableAverageOutput  = (preEncParams) ? !preEncParams->bDisableStatisticsOutput : ((!pParams->b32xScalingInUse && !pParams->b16xScalingInUse) ? m_mbStatsEnabled : 0);
        flatnessTh           = 128;
        enable8x8Stats       = (preEncParams) ? preEncParams->bEnable8x8Statistics : 0;
    }

    SurfaceIndex                            *cmSurfIdx[6];
    if(!fieldPicture)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&inputSurface->OsResource, cmSurf[0]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[0]->GetIndex(cmSurfIdx[0]));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&outputSurface->OsResource, cmSurf[1]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[1]->GetIndex(cmSurfIdx[1]));
    }
    else
    {
        // src top
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&inputSurface->OsResource, cmSurf[0]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[0]->GetIndex(cmSurfIdx[0]));
        cmSurf[0]->SetProperty(CM_TOP_FIELD);
        // dst top
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&outputSurface->OsResource, cmSurf[1]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[1]->GetIndex(cmSurfIdx[1]));
        cmSurf[1]->SetProperty(CM_TOP_FIELD);

        // src bottom
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&inputSurface->OsResource, cmSurf[2]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[2]->GetIndex(cmSurfIdx[2]));
        cmSurf[2]->SetProperty(CM_BOTTOM_FIELD);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&outputSurface->OsResource, cmSurf[3]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[3]->GetIndex(cmSurfIdx[3]));
        cmSurf[3]->SetProperty(CM_BOTTOM_FIELD);

        if((pParams->bScalingforRef) && (pParams->bStatsInputProvided))
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&pParams->sInputStatsBotFieldBuffer, cmBuf[1]));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[1]->GetIndex(cmSurfIdx[5]));
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&preEncParams->resStatsBotFieldBuffer, cmBuf[1]));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[1]->GetIndex(cmSurfIdx[5]));
        }
    }
    if((pParams->bScalingforRef)&&(pParams->bStatsInputProvided))
    {
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&pParams->sInputStatsBuffer, cmBuf[0]));
        }
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&preEncParams->resStatsBuffer, cmBuf[0]));
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[0]->GetIndex(cmSurfIdx[4]));

    uint32_t curbe_R1_5    = flatnessTh;
    uint32_t curbe_R1_6    = enableFlatness | (enableVarianceOutput << 1) | (enableAverageOutput << 2) | (enable8x8Stats << 3);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(DispatchKernelScaling(curbe_R1_5, curbe_R1_6,
                                         srcWidth, srcHeight, fieldPicture, cmSurfIdx));
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::InitKernelStateMbEnc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmDev);

    auto kernelRes = &m_resMBEncKernel;
    CodecHalEncode_CreateMDFKernelResource(this, kernelRes, 3, m_mdfMbencBufSize, m_mdfMbencSurfSize, m_mdfMbencVmeSurfSize, m_feiMBEncCurbeDataSizeExcludeSurfaceIdx);
    uint32_t codeSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_ReadFileToPtr(strMbEncIsaName, &codeSize, &kernelRes->pCommonISA));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->LoadProgram(kernelRes->pCommonISA, codeSize, kernelRes->pCmProgram, "-nojitter"));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateKernel(kernelRes->pCmProgram, "AVCEncMB_I", kernelRes->ppKernel[0]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateKernel(kernelRes->pCmProgram, "AVCEncMB_P", kernelRes->ppKernel[1]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateKernel(kernelRes->pCmProgram, "AVCEncMB_B", kernelRes->ppKernel[2]));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::InitKernelStatePreProc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmDev);

    auto kernelRes = &m_resPreProcKernel;
    CodecHalEncode_CreateMDFKernelResource(this, kernelRes, 1, m_mdfPreProcBufSize, m_mdfPreProcSurfSize, m_mdfPreProcVmeSurfSize,m_feiPreProcCurbeDataSize);
    uint32_t codeSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_ReadFileToPtr(strPreProcIsaName, &codeSize, &kernelRes->pCommonISA));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->LoadProgram(kernelRes->pCommonISA, codeSize, kernelRes->pCmProgram, "-nojitter"));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateKernel(kernelRes->pCmProgram, "FEI_PreEnc", kernelRes->ppKernel[0]));

    return eStatus;
}

//private function, not export the parameters.
MOS_STATUS CodechalEncodeAvcEncFeiG8::DispatchKernelMbEnc(
    void*      params)
{
    MOS_STATUS eStatus  = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto dispatchParams = (PCODECHAL_ENCODE_AVC_MBENC_DISPATCH_PARAMS) params;
    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmDev);
    CODECHAL_ENCODE_CHK_NULL_RETURN(dispatchParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(dispatchParams->kernelRes);
    CODECHAL_ENCODE_CHK_NULL_RETURN(dispatchParams->avcMBEncSurface);

    uint32_t threadCount = dispatchParams->frameWidthInMBs * dispatchParams->sliceHeight * dispatchParams->numSlices;

    uint32_t index = 0;
    auto  kernel                     = dispatchParams->kernelRes->ppKernel[dispatchParams->sliceType - 1];
    auto  avcMbEncSurface            = dispatchParams->avcMBEncSurface;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetThreadCount(threadCount));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, dispatchParams->kernelRes->wCurbeSize, dispatchParams->kernelRes->pCurbe));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->MBDataSurfIndex));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->MVDataSurfIndex));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->MBDistIndex));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->SrcYSurfIndex));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->MBSpecficDataSurfIndex));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->AuxVmeOutSurfIndex));      //FEI distortion output
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->CurrRefPicSelSurfIndex));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->HMEMVPredFwdBwdSurfIndex));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->HMEDistSurfIndex));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->L1RefPicSelSurfIndex));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->FwdFrmMBDataSurfIndex));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->FwdFrmMVSurfIndex));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->MBQPBuffer));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->MBBRCLut));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->MBVMEInterPredictionSurfIndex));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->MBVMEInterPredictionMRSurfIndex));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->Flatnesschk));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->MADSurfIndex));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->InterDistortionIndex));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->BestInterIntraSurfIndex));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->BRCCurbeSurfIndex));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(index++, sizeof(SurfaceIndex), avcMbEncSurface->FEI_MVPredSurfIndex));
    //currently , it's always false.
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateThreadSpace(dispatchParams->frameWidthInMBs, dispatchParams->sliceHeight, dispatchParams->kernelRes->pTS));
    auto cmThreadSpace = dispatchParams->kernelRes->pTS;
    if (I_TYPE == dispatchParams->sliceType) // INTRASLICE
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmThreadSpace->SelectThreadDependencyPattern(CM_WAVEFRONT));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmThreadSpace->SelectThreadDependencyPattern(CM_WAVEFRONT26));
    }
    cmThreadSpace->SetThreadSpaceColorCount(dispatchParams->numSlices);
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::SendAvcMbEncSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS params)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmDev);

    auto kernelRes   = &m_resMBEncKernel;
    auto cmSurfaces  = (PCODECHAL_ENCODE_AVC_SURFACE_IDX)m_avcCmSurfIdx;

    CodecEncodeAvcFeiPicParams *feiPicParams = (CodecEncodeAvcFeiPicParams *)params->pFeiPicParams;

    bool currFieldPicture = CodecHal_PictureIsField(*(params->pCurrOriginalPic)) ? 1 : 0;
    bool currBottomField = CodecHal_PictureIsBottomField(*(params->pCurrOriginalPic)) ? 1 : 0;
    auto currPicRefListEntry = params->ppRefList[params->pCurrReconstructedPic->FrameIdx];
    auto mbCodeBuffer = &currPicRefListEntry->resRefMbCodeBuffer;
    auto mvDataBuffer = &currPicRefListEntry->resRefMvDataBuffer;

    for(uint8_t refIdx = 0; refIdx < 28; refIdx ++)
    {
        cmSurfaces->pCmSurfIdx[refIdx] = (SurfaceIndex*)CM_NULL_SURFACE;
    }

    auto cmSurf         = kernelRes->ppCmSurf;
    auto cmBuf          = kernelRes->ppCmBuf;
    auto cmVmeSurfIdx   = kernelRes->ppCmVmeSurf;

    uint32_t refMbCodeBottomFieldOffset;
    uint32_t refMvBottomFieldOffset;

    if (feiPicParams->MbCodeMvEnable)
    {
        refMbCodeBottomFieldOffset = 0;
        refMvBottomFieldOffset = 0;
    }
    else
    {
        refMbCodeBottomFieldOffset = m_mbcodeBottomFieldOffset;
        refMvBottomFieldOffset =
            MOS_ALIGN_CEIL(params->dwFrameFieldHeightInMb * params->dwFrameWidthInMb * (32 * 4), CODECHAL_PAGE_SIZE);
    }

    uint32_t mbCodeSize = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16 * 4;  // 11DW + 5DW padding
    uint32_t mvSize = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 32 * 4;

    // PAK Obj command buffer
    //dwSize = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16 * 4;  // 11DW + 5DW padding
    // mb code offset     SurfaceCodecParams.dwOffset = params->dwMbCodeBottomFieldOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(mbCodeBuffer,cmBuf[0]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(mvDataBuffer,cmBuf[1]));
    if((feiPicParams->MbCodeMvEnable) || ((params->dwMbCodeBottomFieldOffset == 0) && (params->dwMvBottomFieldOffset == 0)))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[0]->GetIndex(cmSurfaces->MBDataSurfIndex));
        //MV data offset SurfaceCodecParams.dwOffset = params->dwMvBottomFieldOffset;
        //dwSize = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 32 * 4;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[1]->GetIndex(cmSurfaces->MVDataSurfIndex));
    }
    else
    {
        SurfaceIndex *aliasIndex;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBufferAlias(cmBuf[0], aliasIndex));
        CM_BUFFER_STATE_PARAM param;
        MOS_ZeroMemory(&param,sizeof(param));
        param.uiSize    = mbCodeSize;
        param.uiBaseAddressOffset  = params->dwMbCodeBottomFieldOffset;
        cmBuf[0]->SetSurfaceStateParam(aliasIndex, &param);
        cmSurfaces->MBDataSurfIndex = aliasIndex;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBufferAlias(cmBuf[1], aliasIndex));
        param.uiSize    = mvSize;
        param.uiBaseAddressOffset  = params->dwMvBottomFieldOffset;
        cmBuf[1]->SetSurfaceStateParam(aliasIndex, &param);
        cmSurfaces->MVDataSurfIndex = aliasIndex;
    }

    // Current picture Y    2D, and offset = params->dwCurrPicSurfaceOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&params->psCurrPicSurface->OsResource,cmSurf[0]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[0]->GetIndex(cmSurfaces->SrcYSurfIndex));

    if(currFieldPicture)
    {
        if(CodecHal_PictureIsBottomField((*params->pCurrOriginalPic)))
        {
            cmSurf[0]->SetProperty(CM_BOTTOM_FIELD);
        }
        else
        {
            cmSurf[0]->SetProperty(CM_TOP_FIELD);
        }
    }
    // AVC_ME MV data buffer
    if (params->bHmeEnabled)
    {
        //2D, and offset  = params->dwMeMvBottomFieldOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&params->ps4xMeMvDataBuffer->OsResource,cmSurf[1]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[1]->GetIndex(cmSurfaces->HMEMVPredFwdBwdSurfIndex));

        //HMEdistortion 2D, offset = params->dwMeDistortionBottomFieldOffset;
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeDistortionBuffer);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&params->ps4xMeDistortionBuffer->OsResource,cmSurf[2]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[2]->GetIndex(cmSurfaces->HMEDistSurfIndex));
    }
    //current piture Y take  two binding table offset , dwAvcMBEncCurrPicFrame[0] dwAvcMBEncCurrPicFrame[1] dwAvcMBEncFieldCurrPic[0] dwAvcMBEncFieldCurrPic[1]

    //CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&params->psCurrPicSurface->OsResource,pCmSurfForVME));
    // Setup references 1...n
    // LIST 0 references
    uint8_t refNum0 = 0;
    uint8_t refNum1 = 0;
    CmSurface2D *surfArrayL0[8];
    CmSurface2D *surfArrayL1[8];

    CmSurface2D *cmSurfForVME = cmSurf[0];
    MOS_ZeroMemory(&surfArrayL0[0],8 * sizeof(CmSurface2D *));
    MOS_ZeroMemory(&surfArrayL1[0],8 * sizeof(CmSurface2D *));
    for (uint8_t refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l0_active_minus1; refIdx++)
    {
        CODEC_PICTURE refPic = params->pAvcSlcParams->RefPicList[LIST_0][refIdx];
        if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
        {
            uint8_t refPicIdx = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
            refNum0 ++;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&params->ppRefList[refPicIdx]->sRefBuffer.OsResource,cmSurf[3 + refIdx]))
            surfArrayL0[refIdx] = (CmSurface2D *)cmSurf[3 + refIdx];

        }
    }
    for (uint8_t refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l1_active_minus1; refIdx++)
    {
        CODEC_PICTURE refPic = params->pAvcSlcParams->RefPicList[LIST_1][refIdx];
        if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
        {
            uint8_t refPicIdx = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&params->ppRefList[refPicIdx]->sRefBuffer.OsResource,cmSurf[9 + refIdx]))
            surfArrayL1[refIdx] = (CmSurface2D *)cmSurf[9 + refIdx];

            refNum1 ++;
            if (refIdx == 0)
            {
                // MB data buffer   offset = dwRefMbCodeBottomFieldOffsetUsed;
                if((feiPicParams->MbCodeMvEnable) && currFieldPicture)
                {
                    if(refBottomField)
                    {
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&params->ppRefList[refPicIdx]->resRefBotFieldMbCodeBuffer, cmBuf[3]));
                    }
                    else
                    {
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&params->ppRefList[refPicIdx]->resRefTopFieldMbCodeBuffer, cmBuf[3]));
                    }
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[3]->GetIndex(cmSurfaces->FwdFrmMBDataSurfIndex));
                }
                else
                {
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&params->ppRefList[refPicIdx]->resRefMbCodeBuffer, cmBuf[3]));

                    if(currFieldPicture && refBottomField)
                    {
                        SurfaceIndex *aliasIndex;
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBufferAlias(cmBuf[3], aliasIndex));
                        CM_BUFFER_STATE_PARAM param;
                        MOS_ZeroMemory(&param,sizeof(param));
                        param.uiSize    = mbCodeSize;
                        param.uiBaseAddressOffset  = refMbCodeBottomFieldOffset;
                        cmBuf[3]->SetSurfaceStateParam(aliasIndex, &param);
                        cmSurfaces->FwdFrmMBDataSurfIndex = aliasIndex;
                    }
                    else
                    {
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[3]->GetIndex(cmSurfaces->FwdFrmMBDataSurfIndex));
                    }
                }

                // MV data buffer   offset = dwRefMvBottomFieldOffsetUsed;
                if ( (feiPicParams->MbCodeMvEnable) && currFieldPicture)
                {
                    if(refBottomField)
                    {
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&params->ppRefList[refPicIdx]->resRefBotFieldMvDataBuffer, cmBuf[4]));
                    }
                    else
                    {
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&params->ppRefList[refPicIdx]->resRefTopFieldMvDataBuffer, cmBuf[4]));
                    }
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[4]->GetIndex(cmSurfaces->FwdFrmMVSurfIndex));

                }
                else
                {
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&params->ppRefList[refPicIdx]->resRefMvDataBuffer, cmBuf[4]));
                    if(currFieldPicture && refBottomField)
                    {
                        SurfaceIndex *aliasIndex;
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBufferAlias(cmBuf[4], aliasIndex));
                        CM_BUFFER_STATE_PARAM param;
                        MOS_ZeroMemory(&param,sizeof(param));
                        param.uiSize    = mvSize;
                        param.uiBaseAddressOffset  = refMvBottomFieldOffset;
                        cmBuf[4]->SetSurfaceStateParam(aliasIndex, &param);
                        cmSurfaces->FwdFrmMVSurfIndex = aliasIndex;
                    }
                    else
                    {
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[4]->GetIndex(cmSurfaces->FwdFrmMVSurfIndex));
                    }
                }
            }
        }
    }
    pCmDev->CreateVmeSurfaceG7_5( cmSurfForVME,&surfArrayL0[0],&surfArrayL1[0], refNum0, refNum1,cmVmeSurfIdx[0]);
    cmSurfaces->MBVMEInterPredictionSurfIndex = cmVmeSurfIdx[0];

    pCmDev->CreateVmeSurfaceG7_5( cmSurfForVME,&surfArrayL1[0],&surfArrayL1[0], refNum1, refNum1,cmVmeSurfIdx[1]);
    cmSurfaces->MBVMEInterPredictionMRSurfIndex = cmVmeSurfIdx[1];

    CM_VME_SURFACE_STATE_PARAM vmeDimensionParam;
    vmeDimensionParam.width = params->dwFrameWidthInMb * 16;
    vmeDimensionParam.height = params->dwFrameHeightInMb * 16;
    pCmDev->SetVmeSurfaceStateParam(cmSurfaces->MBVMEInterPredictionSurfIndex, &vmeDimensionParam);
    pCmDev->SetVmeSurfaceStateParam(cmSurfaces->MBVMEInterPredictionMRSurfIndex, &vmeDimensionParam);

    // BRC distortion data buffer for I frame
    if (params->bMbEncIFrameDistInUse)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&params->psMeBrcDistortionBuffer->OsResource,cmSurf[11]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[11]->GetIndex(cmSurfaces->MBDistIndex));
    }
    // RefPicSelect of Current picture
    if (params->bUsedAsRef)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&currPicRefListEntry->pRefPicSelectListEntry->sBuffer.OsResource, cmSurf[12]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[12]->GetIndex(cmSurfaces->CurrRefPicSelSurfIndex));
    }
    if (params->bFlatnessCheckEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&params->psFlatnessCheckSurface->OsResource, cmSurf[13]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[13]->GetIndex(cmSurfaces->Flatnesschk));
    }
    if (params->wPictureCodingType == B_TYPE)
    {
        auto refPic = params->pAvcSlcParams->RefPicList[LIST_1][0];
        if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
        {
            auto refPicIdx = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&params->ppRefList[refPicIdx]->pRefPicSelectListEntry->sBuffer.OsResource, cmSurf[14]));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[14]->GetIndex(cmSurfaces->L1RefPicSelSurfIndex));
        }
    }
    if (params->bMADEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(params->presMADDataBuffer, cmBuf[5]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[5]->GetIndex(cmSurfaces->MADSurfIndex));
    }
    if (feiPicParams->bPerMBInput)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&(feiPicParams->resMBCtrl), cmBuf[6]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[6]->GetIndex(cmSurfaces->MBSpecficDataSurfIndex));
    }
    if (feiPicParams->MVPredictorEnable)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&(feiPicParams->resMVPredictor), cmBuf[7]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[7]->GetIndex(cmSurfaces->FEI_MVPredSurfIndex));
    }
    if (feiPicParams->DistortionEnable)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&(feiPicParams->resDistortion), cmBuf[8]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[8]->GetIndex(cmSurfaces->AuxVmeOutSurfIndex));
    }
    if (feiPicParams->bMBQp)
    {
        // 16 DWs per QP value
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(params->presMbBrcConstDataBuffer, cmBuf[9]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[9]->GetIndex(cmSurfaces->MBBRCLut));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&(feiPicParams->resMBQp), cmBuf[10]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[10]->GetIndex(cmSurfaces->MBQPBuffer));
    }
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::EncodeMbEncKernelFunctions()
{
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_refList);

    auto kernelRes = &m_resMBEncKernel;

    uint8_t ppsIdx = m_avcSliceParams->pic_parameter_set_id;
    uint8_t spsIdx = m_avcPicParams[ppsIdx]->seq_parameter_set_id;
    auto refList = &m_refList[0];
    auto currRefList = m_refList[m_currReconstructedPic.FrameIdx];

    bool roiEnabled = (m_avcPicParams[ppsIdx]->NumROI > 0) ? true : false;
    uint8_t currScaledIdx = m_refList[m_currReconstructedPic.FrameIdx]->ucScalingIdx;

    uint8_t refPicListIdx = m_avcSliceParams[ppsIdx].RefPicList[0][0].FrameIdx;
    uint8_t refFrameListIdx = m_avcPicParam[ppsIdx].RefFrameList[refPicListIdx].FrameIdx;

    bool dirtyRoiEnabled = (m_pictureCodingType == P_TYPE
        && m_avcPicParams[ppsIdx]->NumDirtyROI > 0
        && m_prevReconFrameIdx == refFrameListIdx);

    //  Two flags(bMbConstDataBufferNeeded, bMbQpBufferNeeded)
    //  would be used as there are two buffers and not all cases need both the buffers
    //  Constant Data buffer  needed for MBBRC, MBQP, ROI, RollingIntraRefresh
    //  Please note that this surface needs to be programmed for
    //  all usage cases(including CQP cases) because DWord13 includes mode cost for high texture MBs cost.
    bool mbConstDataBufferInUse = bMbBrcEnabled || bMbQpDataEnabled || roiEnabled || dirtyRoiEnabled ||
        m_avcPicParam->EnableRollingIntraRefresh || bHighTextureModeCostEnable;

    bool mbQpBufferInUse =  bMbBrcEnabled ||  bBrcRoiEnabled ||  bMbQpDataEnabled;

    if (m_feiEnable)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_avcFeiPicParams);
        mbConstDataBufferInUse |= m_avcFeiPicParams->bMBQp;
        mbQpBufferInUse        |= m_avcFeiPicParams->bMBQp;
    }

    // Setup AVC Curbe
    CODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS      mbEncCurbeParams;
    MOS_ZeroMemory(&mbEncCurbeParams, sizeof(mbEncCurbeParams));
    mbEncCurbeParams.pPicParams = m_avcPicParams[ppsIdx];
    mbEncCurbeParams.pSeqParams = m_avcSeqParams[spsIdx];
    mbEncCurbeParams.pSlcParams = m_avcSliceParams;
    mbEncCurbeParams.ppRefList = &(m_refList[0]);
    mbEncCurbeParams.pPicIdx = &(m_picIdx[0]);
    mbEncCurbeParams.bRoiEnabled = roiEnabled;
    mbEncCurbeParams.bDirtyRoiEnabled = dirtyRoiEnabled;
    mbEncCurbeParams.bMbEncIFrameDistEnabled = false;
    mbEncCurbeParams.pdwBlockBasedSkipEn = & dwMbEncBlockBasedSkipEn;
    mbEncCurbeParams.bBrcEnabled =  bBrcEnabled;
    mbEncCurbeParams.wPicWidthInMb = m_picWidthInMb;
    mbEncCurbeParams.wFieldFrameHeightInMb = m_frameFieldHeightInMb;
    mbEncCurbeParams.usSliceHeight = (m_arbitraryNumMbsInSlice) ?
        m_frameFieldHeightInMb : m_sliceHeight;
    mbEncCurbeParams.bUseMbEncAdvKernel =  bUseMbEncAdvKernel;
    mbEncCurbeParams.pAvcQCParams =  m_avcQCParams;
    mbEncCurbeParams.bMbDisableSkipMapEnabled =  bMbDisableSkipMapEnabled;
    mbEncCurbeParams.bStaticFrameDetectionEnabled =  bStaticFrameDetectionEnable && m_hmeEnabled;
    mbEncCurbeParams.bApdatvieSearchWindowSizeEnabled =  bApdatvieSearchWindowEnable;
    mbEncCurbeParams.bSquareRollingIEnabled =  bSquareRollingIEnabled;
    mbEncCurbeParams.pCurbeBinary = kernelRes->pCurbe;
    CODECHAL_ENCODE_CHK_STATUS_RETURN( SetCurbeAvcMbEnc(&mbEncCurbeParams));

    for (int i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        if (m_picIdx[i].bValid)
        {
            uint8_t index = m_picIdx[i].ucPicIdx;
            refList[index]->sRefBuffer = m_userFlags.bUseRawPicForRef ?
                refList[index]->sRefRawBuffer : refList[index]->sRefReconBuffer;

            CodecHalGetResourceInfo(m_osInterface, &refList[index]->sRefBuffer);
        }
    }

    // Set up MB BRC Constant Data Buffer if there is QP change within a frame
    if (mbConstDataBufferInUse)
    {
        CODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS initMbBrcConstantDataBufferParams;

        MOS_ZeroMemory(&initMbBrcConstantDataBufferParams, sizeof(initMbBrcConstantDataBufferParams));
        initMbBrcConstantDataBufferParams.pOsInterface = m_osInterface;
        initMbBrcConstantDataBufferParams.presBrcConstantDataBuffer =
            & BrcBuffers.resMbBrcConstDataBuffer[m_currRecycledBufIdx];
        initMbBrcConstantDataBufferParams.dwMbEncBlockBasedSkipEn =  dwMbEncBlockBasedSkipEn;
        initMbBrcConstantDataBufferParams.pPicParams =  m_avcPicParams[ppsIdx];
        initMbBrcConstantDataBufferParams.wPictureCodingType = m_pictureCodingType;
        initMbBrcConstantDataBufferParams.bSkipBiasAdjustmentEnable = m_skipBiasAdjustmentEnable;
        initMbBrcConstantDataBufferParams.bAdaptiveIntraScalingEnable =  bAdaptiveIntraScalingEnable;
        initMbBrcConstantDataBufferParams.bOldModeCostEnable =  bOldModeCostEnable;
        initMbBrcConstantDataBufferParams.pAvcQCParams              =  m_avcQCParams;
        initMbBrcConstantDataBufferParams.bEnableKernelTrellis =  bKernelTrellis &&  m_trellisQuantParams.dwTqEnabled;

        // Kernel controlled Trellis Quantization
        if ( bKernelTrellis &&  m_trellisQuantParams.dwTqEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN( CalcLambdaTable(
                m_pictureCodingType,
                &initMbBrcConstantDataBufferParams.Lambda[0][0]));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN( InitMbBrcConstantDataBuffer(&initMbBrcConstantDataBufferParams));
    }

    CODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS    mbEncSurfaceParams;
    //Add surface states
    MOS_ZeroMemory(&mbEncSurfaceParams, sizeof(mbEncSurfaceParams));
    mbEncSurfaceParams.pAvcSlcParams                        = m_avcSliceParams;
    mbEncSurfaceParams.ppRefList                            = &m_refList[0];
    mbEncSurfaceParams.pAvcPicIdx                           = &m_picIdx[0];
    mbEncSurfaceParams.pCurrOriginalPic                     = &m_currOriginalPic;
    mbEncSurfaceParams.pCurrReconstructedPic                = &m_currReconstructedPic;
    mbEncSurfaceParams.wPictureCodingType                   = m_pictureCodingType;
    mbEncSurfaceParams.psCurrPicSurface                     = m_rawSurfaceToEnc;
    mbEncSurfaceParams.dwMbCodeBottomFieldOffset            = m_mbcodeBottomFieldOffset;
    mbEncSurfaceParams.dwMvBottomFieldOffset                = m_mvBottomFieldOffset;
    mbEncSurfaceParams.ps4xMeMvDataBuffer                   = &m_4xMeMvDataBuffer;
    mbEncSurfaceParams.ps4xMeDistortionBuffer               = &m_4xMeDistortionBuffer;
    mbEncSurfaceParams.dwMeMvBottomFieldOffset              = m_meMvBottomFieldOffset;
    mbEncSurfaceParams.psMeBrcDistortionBuffer              = & BrcBuffers.sMeBrcDistortionBuffer;
    mbEncSurfaceParams.dwMeBrcDistortionBottomFieldOffset   = BrcBuffers.dwMeBrcDistortionBottomFieldOffset;
    mbEncSurfaceParams.dwMeDistortionBottomFieldOffset      = m_meDistortionBottomFieldOffset;
    mbEncSurfaceParams.dwRefPicSelectBottomFieldOffset      = ulRefPicSelectBottomFieldOffset;
    mbEncSurfaceParams.dwFrameWidthInMb                     = m_picWidthInMb;
    mbEncSurfaceParams.dwFrameFieldHeightInMb               = m_frameFieldHeightInMb;
    mbEncSurfaceParams.dwFrameHeightInMb                    = m_picHeightInMb;
    // Interleaved input surfaces
    mbEncSurfaceParams.dwVerticalLineStride                 = m_verticalLineStride;
    mbEncSurfaceParams.dwVerticalLineStrideOffset           = m_verticalLineStrideOffset;
    // Vertical line stride is not used for the case of scaled surfaces saved as separate fields
    mbEncSurfaceParams.bHmeEnabled                          = m_hmeSupported;
    mbEncSurfaceParams.presMbBrcConstDataBuffer             = & BrcBuffers.resMbBrcConstDataBuffer[m_currRecycledBufIdx];
    mbEncSurfaceParams.psMbQpBuffer                         =
         bMbQpDataEnabled ? & sMbQpDataSurface : & BrcBuffers.sBrcMbQpBuffer;
    mbEncSurfaceParams.dwMbQpBottomFieldOffset              = bMbQpDataEnabled ? 0 :  BrcBuffers.dwBrcMbQpBottomFieldOffset;
    mbEncSurfaceParams.bUsedAsRef                           = m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef;
    mbEncSurfaceParams.presMADDataBuffer                    = &m_resMadDataBuffer[m_currMadBufferIdx];
    mbEncSurfaceParams.bMbQpBufferInUse                     = mbQpBufferInUse;
    mbEncSurfaceParams.bMbConstDataBufferInUse              = mbConstDataBufferInUse;
    mbEncSurfaceParams.bMADEnabled                          = m_bMadEnabled;
    mbEncSurfaceParams.bUseMbEncAdvKernel                   = bUseMbEncAdvKernel;
    mbEncSurfaceParams.presMbEncCurbeBuffer                 = & BrcBuffers.resMbEncAdvancedDsh;
    mbEncSurfaceParams.presMbEncBRCBuffer                   = & BrcBuffers.resMbEncBrcBuffer;

    if ( bDecoupleMbEncCurbeFromBRC)  //Starting from Gen95
    {
        mbEncSurfaceParams.dwMbEncBRCBufferSize             = m_mbencBrcBufferSize;
    }

    mbEncSurfaceParams.bUseAdvancedDsh                      = bAdvancedDshInUse;
    mbEncSurfaceParams.bBrcEnabled                          = bBrcEnabled;
    mbEncSurfaceParams.bArbitraryNumMbsInSlice              = m_arbitraryNumMbsInSlice;
    mbEncSurfaceParams.psSliceMapSurface                    = &m_sliceMapSurface[m_currRecycledBufIdx];
    mbEncSurfaceParams.dwSliceMapBottomFieldOffset          = m_sliceMapBottomFieldOffset;
    mbEncSurfaceParams.pMbEncBindingTable                   = & MbEncBindingTable;

    if(m_mbStatsSupported)
    {
        mbEncSurfaceParams.bMBVProcStatsEnabled             = (m_flatnessCheckEnabled || bAdaptiveTransformDecisionEnabled);
        mbEncSurfaceParams.presMBVProcStatsBuffer           = &m_resMbStatsBuffer;
        mbEncSurfaceParams.dwMBVProcStatsBottomFieldOffset  = m_mbStatsBottomFieldOffset;
    }
    else
    {
        mbEncSurfaceParams.bFlatnessCheckEnabled            = m_flatnessCheckEnabled;
        mbEncSurfaceParams.psFlatnessCheckSurface           = &m_flatnessCheckSurface;
        mbEncSurfaceParams.dwFlatnessCheckBottomFieldOffset = m_flatnessCheckBottomFieldOffset;
    }

    // Set up pFeiPicParams
    mbEncSurfaceParams.pFeiPicParams                        = m_avcFeiPicParams;

    mbEncSurfaceParams.bMbDisableSkipMapEnabled             = bMbDisableSkipMapEnabled;
    mbEncSurfaceParams.psMbDisableSkipMapSurface            = psMbDisableSkipMapSurface;

    if( bUseWeightedSurfaceForL0 ||  bUseWeightedSurfaceForL1)
    {
        mbEncSurfaceParams.pWeightedPredOutputPicSelectList = &WeightedPredOutputPicSelectList[0];
        mbEncSurfaceParams.bUseWeightedSurfaceForL0         = bUseWeightedSurfaceForL0;
        mbEncSurfaceParams.bUseWeightedSurfaceForL1         = bUseWeightedSurfaceForL1;
    }

    // Clear the MAD buffer -- the kernel requires it to be 0 as it accumulates the result
    if (mbEncSurfaceParams.bMADEnabled)
    {
        MOS_LOCK_PARAMS     lockFlags;
        // set lock flag to WRITE_ONLY
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;

        uint8_t* pbData = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            mbEncSurfaceParams.presMADDataBuffer,
            &lockFlags);

        CODECHAL_ENCODE_CHK_NULL_RETURN(pbData);

        MOS_ZeroMemory(pbData, CODECHAL_MAD_BUFFER_SIZE);

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            mbEncSurfaceParams.presMADDataBuffer);

        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_resMadDataBuffer[m_currMadBufferIdx],
            CodechalDbgAttr::attrOutput,
            "MADRead",
            CODECHAL_MAD_BUFFER_SIZE,
            0,
            CODECHAL_MEDIA_STATE_ENC_QUALITY)));
    }

    // static frame detection buffer
    mbEncSurfaceParams.bStaticFrameDetectionEnabled =  bStaticFrameDetectionEnable && m_hmeEnabled;
    mbEncSurfaceParams.presSFDOutputBuffer          = & resSFDOutputBuffer[0];
    if (m_pictureCodingType == P_TYPE)
    {
        mbEncSurfaceParams.presSFDCostTableBuffer = & resSFDCostTablePFrameBuffer;
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        mbEncSurfaceParams.presSFDCostTableBuffer = & resSFDCostTableBFrameBuffer;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN( SendAvcMbEncSurfaces(nullptr, &mbEncSurfaceParams));
    //CODECHAL_ENCODE_CHK_STATUS_RETURN( pfnSendAvcMbEncSurfaces(m_hwInterface, &CmdBuffer, &mbEncSurfaceParams));
    //pay attention height //0 P 1 B 2 I
    CODECHAL_ENCODE_AVC_MBENC_DISPATCH_PARAMS   dispatchParams;
    MOS_ZeroMemory(&dispatchParams,sizeof(dispatchParams));
    dispatchParams.kernelRes                   = kernelRes;
    dispatchParams.avcMBEncSurface             = (PCODECHAL_ENCODE_AVC_SURFACE_IDX)m_avcCmSurfIdx;
    dispatchParams.enableArbitrarySliceSize     = m_arbitraryNumMbsInSlice;
    dispatchParams.frameWidthInMBs             = m_picWidthInMb;
    dispatchParams.sliceHeight                 = mbEncCurbeParams.usSliceHeight;
    dispatchParams.numSlices                   = 1;
    dispatchParams.sliceType                   = m_pictureCodingType;
    dispatchParams.numMBs                     = m_frameFieldHeightInMb * m_picWidthInMb;
    dispatchParams.enableWavefrontOptimization  = false;
    CODECHAL_ENCODE_CHK_STATUS_RETURN( DispatchKernelMbEnc(&dispatchParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddKernelMdf(pCmDev,pCmQueue,kernelRes->ppKernel[m_pictureCodingType - 1],pCmTask,kernelRes->pTS,kernelRes->e,true));
    if(pCmEvent[nCmEventIdx] )
    {
        pCmQueue->DestroyEvent(pCmEvent[nCmEventIdx]);
    }
    pCmEvent[nCmEventIdx]   = kernelRes->e;
    nCmEventIdx ++;
    nCmEventIdx %= CM_EVENT_NUM;

    CodecHalEncode_FreeMDFKernelSurfaces(this, kernelRes);
    currRefList->ucMADBufferIdx = m_currMadBufferIdx;
    currRefList->bMADEnabled = m_bMadEnabled;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::DispatchKernelPreProc(
    SurfaceIndex** surfIndexArray,
    uint16_t width,
    uint16_t height)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto kernelRes = &m_resPreProcKernel;
    auto kernel = kernelRes->ppKernel[0];
    // config thread space
    uint32_t threadswidth  = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(width);
    uint32_t threadsheight = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(height + 15);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetThreadCount(threadswidth * threadsheight));

    uint32_t kernelArgIdx = 0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(kernelArgIdx++, kernelRes->wCurbeSize, kernelRes->pCurbe));
    // Current Input Picture surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(kernelArgIdx++, sizeof(SurfaceIndex), surfIndexArray[0]));
    // HME MV Input Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(kernelArgIdx++, sizeof(SurfaceIndex), surfIndexArray[1]));
    // App Provided Prediction surfaces
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(kernelArgIdx++, sizeof(SurfaceIndex), surfIndexArray[2]));
    // Qp Per MB Input Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(kernelArgIdx++, sizeof(SurfaceIndex), surfIndexArray[3]));
    // MV Data Output Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(kernelArgIdx++, sizeof(SurfaceIndex), surfIndexArray[4]));
    // MB VProc Stats Buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(kernelArgIdx++, sizeof(SurfaceIndex), surfIndexArray[5]));
    // fwd ref surfaces. if not provided, set to output surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(kernelArgIdx++, sizeof(SurfaceIndex), surfIndexArray[6]));
    // bwd ref surfaces. if not provided, set to output surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(kernelArgIdx++, sizeof(SurfaceIndex), surfIndexArray[7]));
    // Qp and FTQ LUT
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->SetKernelArg(kernelArgIdx++, sizeof(SurfaceIndex), surfIndexArray[8]));

    if(nullptr == kernelRes->pTS)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateThreadSpace(threadswidth, threadsheight, kernelRes->pTS));
    }
    bool  isEnqueue = false;
    if((!m_singleTaskPhaseSupported) || m_lastTaskInPhase)
    {
        isEnqueue = true;
        m_lastTaskInPhase = false;
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddKernelMdf(pCmDev,pCmQueue,kernel,pCmTask,kernelRes->pTS,kernelRes->e,isEnqueue));

    // assign the Cm event to global array
    // codechal will check the event to process further
    if (pCmEvent[nCmEventIdx])
    {
        pCmQueue->DestroyEvent(pCmEvent[nCmEventIdx]);
    }
    pCmEvent[nCmEventIdx] = kernelRes->e;
    nCmEventIdx = (nCmEventIdx + 1 ) % CM_EVENT_NUM;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::PreProcKernel()
{
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmDev);

    auto kernelRes         = &m_resPreProcKernel;
    auto cmSurf           = kernelRes->ppCmSurf;
    auto cmBuf            = kernelRes->ppCmBuf;
    auto cmVmeSurfIdx     = kernelRes->ppCmVmeSurf;

    auto preEncParams      = (FeiPreEncParams*)m_encodeParams.pPreEncParams;
    auto refList           = &m_refList[0];

    CODECHAL_ENCODE_AVC_PREPROC_CURBE_PARAMS    preProcCurbeParams;
    // Setup Curbe
    MOS_ZeroMemory(&preProcCurbeParams, sizeof(preProcCurbeParams));
    preProcCurbeParams.pPreEncParams = preEncParams;
    preProcCurbeParams.wPicWidthInMb = m_picWidthInMb;
    preProcCurbeParams.wFieldFrameHeightInMb = m_frameFieldHeightInMb;
    preProcCurbeParams.pCurbeBinary = kernelRes->pCurbe;

    SetCurbeAvcPreProc(&preProcCurbeParams);

    for (int i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        if (m_picIdx[i].bValid)
        {
            uint8_t index = m_picIdx[i].ucPicIdx;
            refList[index]->sRefBuffer = refList[index]->sRefRawBuffer;
            CodecHalGetResourceInfo(m_osInterface, &refList[index]->sRefBuffer);
        }
    }

    // Set up FtqLut Buffer if there is QP change within a frame
    if (preEncParams->bMBQp)
    {
        CODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS initMbBrcConstantDataBufferParams;

        MOS_ZeroMemory(&initMbBrcConstantDataBufferParams, sizeof(initMbBrcConstantDataBufferParams));
        initMbBrcConstantDataBufferParams.pOsInterface = m_osInterface;
        initMbBrcConstantDataBufferParams.presBrcConstantDataBuffer =
            &BrcBuffers.resMbBrcConstDataBuffer[m_currRecycledBufIdx];
        initMbBrcConstantDataBufferParams.bPreProcEnable = true;
        initMbBrcConstantDataBufferParams.bEnableKernelTrellis = bKernelTrellis && m_trellisQuantParams.dwTqEnabled;;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitMbBrcConstantDataBuffer(&initMbBrcConstantDataBufferParams));
    }
    SurfaceIndex    *cmSurfIdx[9];
    for(int i = 0; i < 9; i ++)
    {
        cmSurfIdx[i] = (SurfaceIndex*)CM_NULL_SURFACE;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&m_rawSurfaceToEnc->OsResource, cmSurf[0]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[0]->GetIndex(cmSurfIdx[0])); // current input surface

    if(CodecHal_PictureIsField(m_currOriginalPic))
    {
        if(CodecHal_PictureIsTopField(m_currOriginalPic))
        {
            cmSurf[0]->SetProperty(CM_TOP_FIELD);
        }
        else
        {
            cmSurf[0]->SetProperty(CM_BOTTOM_FIELD);
        }
    }

    // HME MV input surface
    if(m_hmeEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&m_4xMeMvDataBuffer.OsResource, cmSurf[1]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmSurf[1]->GetIndex(cmSurfIdx[1]));
    }

    // App Provided Prediction surfaces
    if(preEncParams->dwMVPredictorCtrl)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&preEncParams->resMvPredBuffer, cmBuf[0]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[0]->GetIndex(cmSurfIdx[2]));
    }

    if (preEncParams->bMBQp)
    {
        // Qp Per MB Input Surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&preEncParams->resMbQpBuffer, cmBuf[1]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[1]->GetIndex(cmSurfIdx[3]));
        // Qp and FTQ LUT
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&BrcBuffers.resMbBrcConstDataBuffer[m_currRecycledBufIdx], cmBuf[2]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[2]->GetIndex(cmSurfIdx[8]));
    }

    // MV Data Output Surface
    if(!preEncParams->bDisableMVOutput)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&preEncParams->resMvBuffer, cmBuf[3]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[3]->GetIndex(cmSurfIdx[4]));
    }

    if(!preEncParams->bDisableStatisticsOutput)
    {
        if(CodecHal_PictureIsBottomField(m_currOriginalPic))
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&preEncParams->resStatsBotFieldBuffer, cmBuf[4]));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[4]->GetIndex(cmSurfIdx[5]));
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&preEncParams->resStatsBuffer, cmBuf[4]));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(cmBuf[4]->GetIndex(cmSurfIdx[5]));
        }
    }

    uint8_t baseIdx = 1;
    auto cmSurfForVme = cmSurf[0];
    uint8_t refIdx = 0;
    CmSurface2D *surfArray[8];//[NUM_SURFACES];
    if(preEncParams->dwNumPastReferences)
    {
        CODEC_PICTURE refPic = preEncParams->PastRefPicture;
        uint8_t refPicIdx = refPic.FrameIdx;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&m_refList[refPicIdx]->sRefBuffer.OsResource, cmSurf[baseIdx + 1]));
        surfArray[0] = (CmSurface2D*)cmSurf[baseIdx + 1];

        refIdx = 1;
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateVmeSurfaceG7_5(cmSurfForVme, &surfArray[0], nullptr, refIdx, 0, cmVmeSurfIdx[0]));
    cmSurfIdx[6] = cmVmeSurfIdx[0];

    baseIdx = 2;
    refIdx = 0;
    if(preEncParams->dwNumFutureReferences)
    {
        CODEC_PICTURE refPic = preEncParams->FutureRefPicture;
        uint8_t refPicIdx = refPic.FrameIdx;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&m_refList[refPicIdx]->sRefBuffer.OsResource, cmSurf[baseIdx + 1]));
        surfArray[0] = (CmSurface2D*)cmSurf[baseIdx + 1];
        refIdx = 1;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateVmeSurfaceG7_5(cmSurfForVme, &surfArray[0], nullptr, refIdx, 0, cmVmeSurfIdx[1]));
    cmSurfIdx[7] = cmVmeSurfIdx[1];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(DispatchKernelPreProc(cmSurfIdx,(uint16_t)m_frameWidth, (uint16_t)m_frameFieldHeight));
    return eStatus;
}

#else
MOS_STATUS CodechalEncodeAvcEncFeiG8::InitKernelStateMe()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize));

    for (uint32_t krnStateIdx = 0; krnStateIdx < 2; krnStateIdx++)
    {
        auto kernelStatePtr = &m_meKernelStates[krnStateIdx];
        CODECHAL_KERNEL_HEADER              currKrnHeader;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeGetKernelHeaderAndSize(
            kernelBinary,
            ENC_ME,
            krnStateIdx,
            &currKrnHeader,
            &kernelSize));

        kernelStatePtr->KernelParams.iBTCount = ME_NUM_SURFACES_CM;
        kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        kernelStatePtr->KernelParams.iCurbeLength = sizeof(ME_CURBE_CM_FEI);
        kernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
        kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        kernelStatePtr->KernelParams.iIdCount = 1;

        kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary = kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize = kernelSize;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));
    }

    // Until a better way can be found, maintain old binding table structures
    auto bindingTable = &m_meBindingTable;
    bindingTable->dwMEMVDataSurface = ME_MV_DATA_SURFACE_CM;
    bindingTable->dw16xMEMVDataSurface = ME_16x_MV_DATA_SURFACE_CM;
    bindingTable->dw32xMEMVDataSurface = ME_32x_MV_DATA_SURFACE_CM;
    bindingTable->dwMEDist = ME_DISTORTION_SURFACE_CM;
    bindingTable->dwMEBRCDist = ME_BRC_DISTORTION_CM;
    bindingTable->dwMECurrForFwdRef = ME_CURR_FOR_FWD_REF_CM;
    bindingTable->dwMEFwdRefPicIdx[0] = ME_FWD_REF_IDX0_CM;
    bindingTable->dwMEFwdRefPicIdx[1] = ME_FWD_REF_IDX1_CM;
    bindingTable->dwMEFwdRefPicIdx[2] = ME_FWD_REF_IDX2_CM;
    bindingTable->dwMEFwdRefPicIdx[3] = ME_FWD_REF_IDX3_CM;
    bindingTable->dwMEFwdRefPicIdx[4] = ME_FWD_REF_IDX4_CM;
    bindingTable->dwMEFwdRefPicIdx[5] = ME_FWD_REF_IDX5_CM;
    bindingTable->dwMEFwdRefPicIdx[6] = ME_FWD_REF_IDX6_CM;
    bindingTable->dwMEFwdRefPicIdx[7] = ME_FWD_REF_IDX7_CM;
    bindingTable->dwMECurrForBwdRef = ME_CURR_FOR_BWD_REF_CM;
    bindingTable->dwMEBwdRefPicIdx[0] = ME_BWD_REF_IDX0_CM;
    bindingTable->dwMEBwdRefPicIdx[1] = ME_BWD_REF_IDX1_CM;
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::InitKernelStateMbEnc()
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface()->GetHwCaps());

    uint32_t numMbEncKrnStates =
        MBENC_TARGET_USAGE_CM * m_mbencNumTargetUsagesCmFei;
    pMbEncKernelStates =
        MOS_NewArray(MHW_KERNEL_STATE, numMbEncKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pMbEncKernelStates);

    auto kernelStatePtr = pMbEncKernelStates;
    uint8_t* kernelBinary;
    uint32_t kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize));

    for (uint32_t krnStateIdx = 0; krnStateIdx < numMbEncKrnStates; krnStateIdx++)
    {
        bool kernelState = (krnStateIdx >= MBENC_TARGET_USAGE_CM);

        CODECHAL_KERNEL_HEADER  currKrnHeader;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
            kernelBinary,
            (kernelState ? ENC_MBENC_ADV : ENC_MBENC),
            (kernelState ? krnStateIdx - MBENC_TARGET_USAGE_CM : krnStateIdx),
            &currKrnHeader,
            &kernelSize));

        kernelStatePtr->KernelParams.iBTCount = MBENC_NUM_SURFACES_CM;
        kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        kernelStatePtr->KernelParams.iCurbeLength = sizeof(MBENC_CURBE_CM_FEI);
        kernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
        kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        kernelStatePtr->KernelParams.iIdCount = 1;

        CODECHAL_ENCODE_CHK_NULL_RETURN(m_stateHeapInterface);
        kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary = kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));

        kernelStatePtr++;
    }

    // Until a better way can be found, maintain old binding table structures
    auto bindingTable = &MbEncBindingTable;

    bindingTable->dwAvcMBEncMfcAvcPakObj       = MBENC_MFC_AVC_PAK_OBJ_CM_FEI;
    bindingTable->dwAvcMBEncIndMVData          = MBENC_IND_MV_DATA_CM_FEI;
    bindingTable->dwAvcMBEncBRCDist            = MBENC_BRC_DISTORTION_CM_FEI;
    bindingTable->dwAvcMBEncCurrY              = MBENC_CURR_Y_CM_FEI;
    bindingTable->dwAvcMBEncCurrUV             = MBENC_CURR_UV_CM_FEI;
    bindingTable->dwAvcMBEncMbSpecificData     = MBENC_MB_SPECIFIC_DATA_CM_FEI;
    bindingTable->dwAvcMBEncRefPicSelectL0     = MBENC_REFPICSELECT_L0_CM_FEI;
    bindingTable->dwAvcMBEncMVDataFromME       = MBENC_MV_DATA_FROM_ME_CM_FEI;
    bindingTable->dwAvcMBEncMEDist             = MBENC_4xME_DISTORTION_CM_FEI;
    bindingTable->dwAvcMBEncRefPicSelectL1     = MBENC_REFPICSELECT_L1_CM_FEI;
    bindingTable->dwAvcMBEncBwdRefMBData       = MBENC_FWD_MB_DATA_CM_FEI;
    bindingTable->dwAvcMBEncBwdRefMVData       = MBENC_FWD_MV_DATA_CM_FEI;
    bindingTable->dwAvcMBEncMbBrcConstData     = MBENC_MBBRC_CONST_DATA_CM_FEI;
    bindingTable->dwAvcMBEncFlatnessChk        = MBENC_FLATNESS_CHECK_CM_FEI;
    bindingTable->dwAvcMBEncMADData            = MBENC_MAD_DATA_CM_FEI;
    bindingTable->dwAvcMBEncVMEDistortion      = MBENC_INTER_DISTORTION_CM_FEI;
    bindingTable->dwAvcMbEncBRCCurbeData       = MBENC_BRC_CURBE_DATA_CM_FEI;
    bindingTable->dwAvcMBEncMvPrediction       = MBENC_MV_PREDICTOR_CM_FEI;

     // Frame
    bindingTable->dwAvcMBEncMbQpFrame          = MBENC_MBQP_CM;
    bindingTable->dwAvcMBEncCurrPicFrame[0]    = MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_CM;
    bindingTable->dwAvcMBEncFwdPicFrame[0]     = MBENC_VME_INTER_PRED_FWD_PIC_IDX0_CM;
    bindingTable->dwAvcMBEncBwdPicFrame[0]     = MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_CM;
    bindingTable->dwAvcMBEncFwdPicFrame[1]     = MBENC_VME_INTER_PRED_FWD_PIC_IDX1_CM;
    bindingTable->dwAvcMBEncBwdPicFrame[1]     = MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_CM;
    bindingTable->dwAvcMBEncFwdPicFrame[2]     = MBENC_VME_INTER_PRED_FWD_PIC_IDX2_CM;
    bindingTable->dwAvcMBEncFwdPicFrame[3]     = MBENC_VME_INTER_PRED_FWD_PIC_IDX3_CM;
    bindingTable->dwAvcMBEncFwdPicFrame[4]     = MBENC_VME_INTER_PRED_FWD_PIC_IDX4_CM;
    bindingTable->dwAvcMBEncFwdPicFrame[5]     = MBENC_VME_INTER_PRED_FWD_PIC_IDX5_CM;
    bindingTable->dwAvcMBEncFwdPicFrame[6]     = MBENC_VME_INTER_PRED_FWD_PIC_IDX6_CM;
    bindingTable->dwAvcMBEncFwdPicFrame[7]     = MBENC_VME_INTER_PRED_FWD_PIC_IDX7_CM;
    bindingTable->dwAvcMBEncCurrPicFrame[1]    = MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_CM;
    bindingTable->dwAvcMBEncBwdPicFrame[2]     = MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_CM;
    bindingTable->dwAvcMBEncBwdPicFrame[3]     = MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_CM;

    // Field
    bindingTable->dwAvcMBEncMbQpField          = MBENC_MBQP_CM;
    bindingTable->dwAvcMBEncFieldCurrPic[0]    = MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_CM;
    bindingTable->dwAvcMBEncFwdPicTopField[0]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX0_CM;
    bindingTable->dwAvcMBEncBwdPicTopField[0]  = MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_CM;
    bindingTable->dwAvcMBEncFwdPicBotField[0]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX0_CM;
    bindingTable->dwAvcMBEncBwdPicBotField[0]  = MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_CM;
    bindingTable->dwAvcMBEncFwdPicTopField[1]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX1_CM;
    bindingTable->dwAvcMBEncBwdPicTopField[1]  = MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_CM;
    bindingTable->dwAvcMBEncFwdPicBotField[1]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX1_CM;
    bindingTable->dwAvcMBEncBwdPicBotField[1]  = MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_CM;
    bindingTable->dwAvcMBEncFwdPicTopField[2]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX2_CM;
    bindingTable->dwAvcMBEncFwdPicBotField[2]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX2_CM;
    bindingTable->dwAvcMBEncFwdPicTopField[3]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX3_CM;
    bindingTable->dwAvcMBEncFwdPicBotField[3]  = MBENC_VME_INTER_PRED_FWD_PIC_IDX3_CM;
    bindingTable->dwAvcMBEncFieldCurrPic[1]    = MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_CM;
    bindingTable->dwAvcMBEncBwdPicTopField[2]  = MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_CM;
    bindingTable->dwAvcMBEncBwdPicBotField[2]  = MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_CM;
    bindingTable->dwAvcMBEncBwdPicTopField[3]  = MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_CM;
    bindingTable->dwAvcMBEncBwdPicBotField[3]  = MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_CM;
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::InitKernelStatePreProc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto kernelStatePtr = &PreProcKernelState;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize));

    uint32_t krnStateIdx = 0;
    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeGetKernelHeaderAndSize(
        kernelBinary,
        ENC_PREPROC,
        krnStateIdx,
        &currKrnHeader,
        &kernelSize));

    kernelStatePtr->KernelParams.iBTCount = CODECHAL_ENCODE_AVC_PREPROC_FIELD_NUM_SURFACES_CM_G8;
    kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelStatePtr->KernelParams.iCurbeLength = sizeof(PREPROC_CURBE_CM);
    kernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
    kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
    kernelStatePtr->KernelParams.iIdCount = 1;

    kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    kernelStatePtr->KernelParams.pBinary = kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    kernelStatePtr->KernelParams.iSize = kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        kernelStatePtr->KernelParams.iBTCount,
        &kernelStatePtr->dwSshSize,
        &kernelStatePtr->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));

    // Until a better way can be found, maintain old binding table structures
    auto bindingTable = &PreProcBindingTable;

    bindingTable->dwAvcPreProcCurrY = CODECHAL_ENCODE_AVC_PREPROC_CURR_Y_CM_G8;
    bindingTable->dwAvcPreProcCurrUV = CODECHAL_ENCODE_AVC_PREPROC_CURR_UV_CM_G8;
    bindingTable->dwAvcPreProcMVDataFromHME = CODECHAL_ENCODE_AVC_PREPROC_HME_MV_DATA_CM_G8;
    bindingTable->dwAvcPreProcMvPredictor = CODECHAL_ENCODE_AVC_PREPROC_MV_PREDICTOR_CM_G8;
    bindingTable->dwAvcPreProcMbQp = CODECHAL_ENCODE_AVC_PREPROC_MBQP_CM_G8;
    bindingTable->dwAvcPreProcMvDataOut = CODECHAL_ENCODE_AVC_PREPROC_MV_DATA_CM_G8;
    bindingTable->dwAvcPreProcMbStatsOut = CODECHAL_ENCODE_AVC_PREPROC_MB_STATS_CM_G8;
    bindingTable->dwAvcPreProcVMECurrPicFrame[0] = CODECHAL_ENCODE_AVC_PREPROC_VME_CURR_PIC_IDX_0_CM_G8;
    bindingTable->dwAvcPreProcVMEFwdPicFrame = CODECHAL_ENCODE_AVC_PREPROC_VME_FWD_PIC_IDX0_CM_G8;
    bindingTable->dwAvcPreProcVMEBwdPicFrame[0] = CODECHAL_ENCODE_AVC_PREPROC_VME_BWD_PIC_IDX0_0_CM_G8;
    bindingTable->dwAvcPreProcVMECurrPicFrame[1] = CODECHAL_ENCODE_AVC_PREPROC_VME_CURR_PIC_IDX_1_CM_G8;
    bindingTable->dwAvcPreProcVMEBwdPicFrame[1] = CODECHAL_ENCODE_AVC_PREPROC_VME_BWD_PIC_IDX0_1_CM_G8;
    bindingTable->dwAvcPreProcFtqLut = CODECHAL_ENCODE_AVC_PREPROC_FTQ_LUT_CM_G8;

    //field case
    bindingTable->dwAvcPreProcVMECurrPicField[0] = CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_CURR_PIC_IDX_0_CM_G8;
    bindingTable->dwAvcPreProcVMEFwdPicField[0] = CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_FWD_PIC_IDX0_0_CM_G8;

    bindingTable->dwAvcPreProcVMEFwdPicField[1] = CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_FWD_PIC_IDX0_1_CM_G8;

    bindingTable->dwAvcPreProcVMECurrPicField[1] = CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_CURR_PIC_IDX_1_CM_G8;
    bindingTable->dwAvcPreProcVMEBwdPicField[0] = CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_BWD_PIC_IDX0_0_CM_G8;

    bindingTable->dwAvcPreProcVMEBwdPicField[1] = CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_BWD_PIC_IDX0_1_CM_G8;

    bindingTable->dwAvcPreProcFtqLutField = CODECHAL_ENCODE_AVC_PREPROC_FIELD_FTQ_LUT_CM_G8;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::SendAvcMbEncSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS params)
{
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pAvcSlcParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->ppRefList);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pCurrOriginalPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pCurrReconstructedPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psCurrPicSurface);

    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pMbEncBindingTable);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pFeiPicParams);

    auto kernelState = params->pKernelState;

    auto feiPicParams = (CodecEncodeAvcFeiPicParams *)params->pFeiPicParams;
    auto avcMbEncBindingTable = params->pMbEncBindingTable;

    bool currFieldPicture = CodecHal_PictureIsField(*(params->pCurrOriginalPic)) ? 1 : 0;
    bool currBottomField = CodecHal_PictureIsBottomField(*(params->pCurrOriginalPic)) ? 1 : 0;
    auto currPicRefListEntry = params->ppRefList[params->pCurrReconstructedPic->FrameIdx];
    auto mbCodeBuffer = &currPicRefListEntry->resRefMbCodeBuffer;
    auto mvDataBuffer = &currPicRefListEntry->resRefMvDataBuffer;
    uint32_t refMbCodeBottomFieldOffset;
    uint32_t refMvBottomFieldOffset;
    if (feiPicParams->MbCodeMvEnable)
    {
        refMbCodeBottomFieldOffset = 0;
        refMvBottomFieldOffset = 0;
    }
    else
    {
        refMbCodeBottomFieldOffset =
            params->dwFrameFieldHeightInMb * params->dwFrameWidthInMb * 64;
        refMvBottomFieldOffset =
            MOS_ALIGN_CEIL(params->dwFrameFieldHeightInMb * params->dwFrameWidthInMb * (32 * 4), 0x1000);
    }
    uint8_t vDirection;
    if (params->bMbEncIFrameDistInUse)
    {
        vDirection = CODECHAL_VDIRECTION_FRAME;
    }
    else
    {
        vDirection = (CodecHal_PictureIsFrame(*(params->pCurrOriginalPic))) ? CODECHAL_VDIRECTION_FRAME :
            (currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD;
    }

    // PAK Obj command buffer
    uint32_t size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16 * 4;  // 11DW + 5DW padding
    CODECHAL_SURFACE_CODEC_PARAMS  surfaceCodecParams;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = mbCodeBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwOffset = params->dwMbCodeBottomFieldOffset;
    surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMfcAvcPakObj;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // MV data buffer
    size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 32 * 4;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = mvDataBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwOffset = params->dwMvBottomFieldOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncIndMVData;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Current Picture Y
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bUseUVPlane = true;
    surfaceCodecParams.psSurface = params->psCurrPicSurface;
    surfaceCodecParams.dwOffset = params->dwCurrPicSurfaceOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncCurrY;
    surfaceCodecParams.dwUVBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncCurrUV;
    surfaceCodecParams.dwVerticalLineStride = params->dwVerticalLineStride;
    surfaceCodecParams.dwVerticalLineStrideOffset = params->dwVerticalLineStrideOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // AVC_ME MV data buffer
    if (params->bHmeEnabled)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeMvDataBuffer);

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->ps4xMeMvDataBuffer;
        surfaceCodecParams.dwOffset = params->dwMeMvBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMVDataFromME;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeDistortionBuffer);

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->ps4xMeDistortionBuffer;
        surfaceCodecParams.dwOffset = params->dwMeDistortionBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMEDist;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // Current Picture Y - VME
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bUseAdvState = true;
    surfaceCodecParams.psSurface = params->psCurrPicSurface;
    surfaceCodecParams.dwOffset = params->dwCurrPicSurfaceOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = currFieldPicture ?
        avcMbEncBindingTable->dwAvcMBEncFieldCurrPic[0] : avcMbEncBindingTable->dwAvcMBEncCurrPicFrame[0];
    surfaceCodecParams.ucVDirection = vDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    surfaceCodecParams.dwBindingTableOffset = currFieldPicture ?
        avcMbEncBindingTable->dwAvcMBEncFieldCurrPic[1] : avcMbEncBindingTable->dwAvcMBEncCurrPicFrame[1];
    surfaceCodecParams.ucVDirection = vDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Setup references 1...n
    // LIST 0 references
    for (uint8_t refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l0_active_minus1; refIdx++)
    {
        auto refPic = params->pAvcSlcParams->RefPicList[LIST_0][refIdx];
        if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
        {
            uint8_t refPicIdx = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
            uint32_t refBindingTableOffset;
            // Program the surface based on current picture's field/frame mode
            uint8_t refVDirection;
            if (currFieldPicture) // if current picture is field
            {
                if (refBottomField)
                {
                    refVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
                    refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncFwdPicBotField[refIdx];
                }
                else
                {
                    refVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
                    refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncFwdPicTopField[refIdx];
                }
            }
            else // if current picture is frame
            {
                refVDirection = CODECHAL_VDIRECTION_FRAME;
                refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncFwdPicFrame[refIdx];
            }

            // Picture Y VME
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            if((params->bUseWeightedSurfaceForL0) &&
               (params->pAvcSlcParams->luma_weight_flag[LIST_0] & (1 << refIdx)) &&
               (refIdx < CODEC_AVC_MAX_FORWARD_WP_FRAME))
            {
                surfaceCodecParams.psSurface = &params->pWeightedPredOutputPicSelectList[CODEC_AVC_WP_OUTPUT_L0_START + refIdx].sBuffer;
            }
            else
            {
                surfaceCodecParams.psSurface = &params->ppRefList[refPicIdx]->sRefBuffer;
            }
            surfaceCodecParams.dwWidthInUse = params->dwFrameWidthInMb * 16;
            surfaceCodecParams.dwHeightInUse = params->dwFrameHeightInMb * 16;

            surfaceCodecParams.dwBindingTableOffset = refBindingTableOffset;
            surfaceCodecParams.ucVDirection = refVDirection;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

    // Setup references 1...n
    // LIST 1 references
    for (uint8_t refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l1_active_minus1; refIdx++)
    {
        if (!currFieldPicture && refIdx > 0)
        {
            // Only 1 LIST 1 reference required here since only single ref is supported in frame case
            break;
        }

        auto refPic = params->pAvcSlcParams->RefPicList[LIST_1][refIdx];
        if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
        {
            uint8_t refPicIdx = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
            uint32_t refMbCodeBottomFieldOffsetUsed;
            uint32_t refMvBottomFieldOffsetUsed;
            uint8_t refVDirection;
            uint32_t refBindingTableOffset;
            // Program the surface based on current picture's field/frame mode
            if (currFieldPicture) // if current picture is field
            {
                if (refBottomField)
                {
                    refVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
                    refMbCodeBottomFieldOffsetUsed = refMbCodeBottomFieldOffset;
                    refMvBottomFieldOffsetUsed = refMvBottomFieldOffset;
                    refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicBotField[refIdx];
                }
                else
                {
                    refVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
                    refMbCodeBottomFieldOffsetUsed = 0;
                    refMvBottomFieldOffsetUsed = 0;
                    refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicTopField[refIdx];
                }
            }
            else // if current picture is frame
            {
                refVDirection = CODECHAL_VDIRECTION_FRAME;
                refMbCodeBottomFieldOffsetUsed = 0;
                refMvBottomFieldOffsetUsed = 0;
                refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicFrame[refIdx];
            }

            // Picture Y VME
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            surfaceCodecParams.dwWidthInUse = params->dwFrameWidthInMb * 16;
            surfaceCodecParams.dwHeightInUse = params->dwFrameHeightInMb * 16;
            // max backforward weight prediction surface is 2
            if((params->bUseWeightedSurfaceForL1) &&
               (params->pAvcSlcParams->luma_weight_flag[LIST_1]&(1<<refIdx)) &&
               (refIdx < CODEC_AVC_MAX_BACKWARD_WP_FRAME))
            {
                surfaceCodecParams.psSurface = &params->pWeightedPredOutputPicSelectList[CODEC_AVC_WP_OUTPUT_L1_START + refIdx].sBuffer;
            }
            else
            {
                surfaceCodecParams.psSurface = &params->ppRefList[refPicIdx]->sRefBuffer;
            }
            surfaceCodecParams.dwBindingTableOffset = refBindingTableOffset;
            surfaceCodecParams.ucVDirection = refVDirection;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));

            if (refIdx == 0)
            {
                if(currFieldPicture && (params->ppRefList[refPicIdx]->ucAvcPictureCodingType == CODEC_AVC_PIC_CODING_TYPE_FRAME || params->ppRefList[refPicIdx]->ucAvcPictureCodingType == CODEC_AVC_PIC_CODING_TYPE_INVALID))
                {
                    refMbCodeBottomFieldOffsetUsed = 0;
                    refMvBottomFieldOffsetUsed     = 0;
                }
                // MB data buffer
                size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16 * 4;

                MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.dwSize = size;
                if (feiPicParams->MbCodeMvEnable && currFieldPicture)
                {
                    surfaceCodecParams.presBuffer = refBottomField ? &params->ppRefList[refPicIdx]->resRefBotFieldMbCodeBuffer :
                        &params->ppRefList[refPicIdx]->resRefTopFieldMbCodeBuffer;
                }
                else
                {
                    surfaceCodecParams.presBuffer = &params->ppRefList[refPicIdx]->resRefMbCodeBuffer;
                }
                surfaceCodecParams.dwOffset = refMbCodeBottomFieldOffsetUsed;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
                surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdRefMBData;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));

                // MV data buffer
                size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 32 * 4;
                MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.dwSize = size;
                if (feiPicParams->MbCodeMvEnable && currFieldPicture)
                {
                    surfaceCodecParams.presBuffer = refBottomField ? &params->ppRefList[refPicIdx]->resRefBotFieldMvDataBuffer :
                        &params->ppRefList[refPicIdx]->resRefTopFieldMvDataBuffer;
                }
                else
                {
                    surfaceCodecParams.presBuffer = &params->ppRefList[refPicIdx]->resRefMvDataBuffer;
                }
                surfaceCodecParams.dwOffset = refMvBottomFieldOffsetUsed;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
                surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdRefMVData;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }

            if (refIdx < CODECHAL_ENCODE_NUM_MAX_VME_L1_REF)
            {
                if (currFieldPicture)
                {
                    // The binding table contains multiple entries for IDX0 backwards references
                    if (refBottomField)
                    {
                        refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicBotField[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                    }
                    else
                    {
                        refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicTopField[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                    }
                }
                else
                {
                    refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicFrame[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                }

                // Picture Y VME
                MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.bUseAdvState = true;

                surfaceCodecParams.dwWidthInUse = params->dwFrameWidthInMb * 16;
                surfaceCodecParams.dwHeightInUse = params->dwFrameHeightInMb * 16;

                surfaceCodecParams.psSurface = &params->ppRefList[refPicIdx]->sRefBuffer;
                surfaceCodecParams.dwBindingTableOffset = refBindingTableOffset;
                surfaceCodecParams.ucVDirection = refVDirection;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }
        }
    }

    // BRC distortion data buffer for I frame
    if (params->bMbEncIFrameDistInUse)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psMeBrcDistortionBuffer;
        surfaceCodecParams.dwOffset = params->dwMeBrcDistortionBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBRCDist;
        surfaceCodecParams.bIsWritable = true;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // RefPicSelect of Current Picture
    if (params->bUsedAsRef)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = &currPicRefListEntry->pRefPicSelectListEntry->sBuffer;
        surfaceCodecParams.psSurface->dwHeight = MOS_ALIGN_CEIL(surfaceCodecParams.psSurface->dwHeight, 8);
        surfaceCodecParams.dwOffset = params->dwRefPicSelectBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncRefPicSelectL0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (params->bFlatnessCheckEnabled)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psFlatnessCheckSurface;
        surfaceCodecParams.dwOffset = currBottomField ? params->dwFlatnessCheckBottomFieldOffset : 0;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncFlatnessChk;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_FLATNESS_CHECK_ENCODE].Value;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (params->wPictureCodingType == B_TYPE)
    {
        auto refPic = params->pAvcSlcParams->RefPicList[LIST_1][0];
        if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
        {
            bool refBottomField = CodecHal_PictureIsBottomField(refPic);
            auto refPicIdx = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;

            if (params->ppRefList[refPicIdx]->pRefPicSelectListEntry == nullptr)
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("The RefPicSelectList entry for L1, IDX0 is not valid.");
                return eStatus;
            }

            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bIs2DSurface = true;
            surfaceCodecParams.bMediaBlockRW = true;
            surfaceCodecParams.psSurface =
                &params->ppRefList[refPicIdx]->pRefPicSelectListEntry->sBuffer;
            surfaceCodecParams.dwOffset = refBottomField ? params->dwRefPicSelectBottomFieldOffset : 0;
            surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncRefPicSelectL1;
            surfaceCodecParams.dwCacheabilityControl =
                m_hwInterface->ComposeSurfaceCacheabilityControl(
                MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE,
                codechalLLC);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

    if (params->bMADEnabled)
    {
        size = CODECHAL_MAD_BUFFER_SIZE;

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bRawSurface = true;
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = params->presMADDataBuffer;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMADData;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // FEI Mb Specific Data surface
    if (feiPicParams->bPerMBInput)
    {
        size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16;
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = &(feiPicParams->resMBCtrl);
        surfaceCodecParams.dwOffset = 0;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMbSpecificData;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // FEI Multi Mv Predictor Surface
    if (feiPicParams->MVPredictorEnable)
    {
        size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 40;
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = &(feiPicParams->resMVPredictor);
        surfaceCodecParams.dwOffset = 0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMvPrediction;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // FEI distortion surface
    if (feiPicParams->DistortionEnable)
    {
        size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 48;
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = &(feiPicParams->resDistortion);
        surfaceCodecParams.dwOffset = 0;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncVMEDistortion;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (feiPicParams->bMBQp)
    {
        // 16 DWs per QP value
        size = 16 * 52 * sizeof(uint32_t);
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = params->presMbBrcConstDataBuffer;
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMbBrcConstData;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb + 3;
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = &(feiPicParams->resMBQp);
        surfaceCodecParams.dwOffset = 0;
        surfaceCodecParams.dwBindingTableOffset = currFieldPicture ? avcMbEncBindingTable->dwAvcMBEncMbQpField :
            avcMbEncBindingTable->dwAvcMBEncMbQpFrame;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::PreProcKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto preEncParams = (FeiPreEncParams*)m_encodeParams.pPreEncParams;
    auto refList = &m_refList[0];

    auto encFunctionType = CODECHAL_MEDIA_STATE_PREPROC;
    auto kernelState = &PreProcKernelState;

    PerfTagSetting perfTag;
    perfTag.Value = 0;
    perfTag.Mode = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_PREPROC_KERNEL;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase == true || !m_singleTaskPhaseSupported)
    {
        uint32_t dwMaxBtCount = m_singleTaskPhaseSupported ?
            m_maxBtCount : kernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
            m_stateHeapInterface,
            dwMaxBtCount));
        m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(dwMaxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    // Set up the DSH/SSH as normal
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        1,
        &idParams));

    // Setup Curbe
    CODECHAL_ENCODE_AVC_PREPROC_CURBE_PARAMS preProcCurbeParams;
    MOS_ZeroMemory(&preProcCurbeParams, sizeof(preProcCurbeParams));
    preProcCurbeParams.pPreEncParams = preEncParams;
    preProcCurbeParams.wPicWidthInMb = m_picWidthInMb;
    preProcCurbeParams.wFieldFrameHeightInMb = m_frameFieldHeightInMb;
    preProcCurbeParams.pKernelState = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeAvcPreProc(
        &preProcCurbeParams));

    CODECHAL_DEBUG_TOOL(
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
        encFunctionType,
        MHW_DSH_TYPE,
        kernelState));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
        encFunctionType,
        kernelState));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
        encFunctionType,
        MHW_ISH_TYPE,
        kernelState));
    )

    for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        if (m_picIdx[i].bValid)
        {
            uint8_t index = m_picIdx[i].ucPicIdx;
            refList[index]->sRefBuffer = refList[index]->sRefRawBuffer;

            CodecHalGetResourceInfo(m_osInterface, &refList[index]->sRefBuffer);
        }
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.ucDmvPredFlag = (m_pictureCodingType == I_TYPE) ? 0 : m_avcSliceParams->direct_spatial_mv_pred_flag;

    sendKernelCmdsParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Set up FtqLut Buffer if there is QP change within a frame
    if (preEncParams->bMBQp)
    {
        CODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS initMbBrcConstantDataBufferParams;

        MOS_ZeroMemory(&initMbBrcConstantDataBufferParams, sizeof(initMbBrcConstantDataBufferParams));
        initMbBrcConstantDataBufferParams.pOsInterface = m_osInterface;
        initMbBrcConstantDataBufferParams.presBrcConstantDataBuffer =
            &BrcBuffers.resMbBrcConstDataBuffer[m_currRecycledBufIdx];
        initMbBrcConstantDataBufferParams.bPreProcEnable = true;
        initMbBrcConstantDataBufferParams.bEnableKernelTrellis = bKernelTrellis && m_trellisQuantParams.dwTqEnabled;;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitMbBrcConstantDataBuffer(&initMbBrcConstantDataBufferParams));
    }

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    //Add surface states
    CODECHAL_ENCODE_AVC_PREPROC_SURFACE_PARAMS preProcSurfaceParams;
    MOS_ZeroMemory(&preProcSurfaceParams, sizeof(preProcSurfaceParams));
    preProcSurfaceParams.pPreEncParams = preEncParams;
    preProcSurfaceParams.ppRefList = &m_refList[0];
    preProcSurfaceParams.pCurrOriginalPic = &m_currOriginalPic;
    preProcSurfaceParams.psCurrPicSurface = m_rawSurfaceToEnc;
    preProcSurfaceParams.ps4xMeMvDataBuffer = &m_4xMeMvDataBuffer;
    preProcSurfaceParams.presFtqLutBuffer = &BrcBuffers.resMbBrcConstDataBuffer[m_currRecycledBufIdx];
    preProcSurfaceParams.dwMeMvBottomFieldOffset = m_meMvBottomFieldOffset;
    preProcSurfaceParams.dwFrameWidthInMb = (uint32_t)m_picWidthInMb;
    preProcSurfaceParams.dwFrameFieldHeightInMb = (uint32_t)m_frameFieldHeightInMb;
    preProcSurfaceParams.dwMBVProcStatsBottomFieldOffset =
        CodecHal_PictureIsBottomField(m_currOriginalPic) ? m_mbvProcStatsBottomFieldOffset : 0;
    // Interleaved input surfaces
    preProcSurfaceParams.dwVerticalLineStride = m_verticalLineStride;
    preProcSurfaceParams.dwVerticalLineStrideOffset = m_verticalLineStrideOffset;
    preProcSurfaceParams.bHmeEnabled = m_hmeEnabled;
    preProcSurfaceParams.pPreProcBindingTable = &PreProcBindingTable;
    preProcSurfaceParams.pKernelState = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendAvcPreProcSurfaces(&cmdBuffer, &preProcSurfaceParams));

    uint32_t resolutionX = (uint32_t)m_picWidthInMb;
    uint32_t resolutionY = (uint32_t)m_frameFieldHeightInMb;

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode = m_walkerMode;
    walkerCodecParams.dwResolutionX = resolutionX;
    walkerCodecParams.dwResolutionY = resolutionY;
    walkerCodecParams.bNoDependency = true;
    walkerCodecParams.bMbaff = m_mbaffEnabled;
    walkerCodecParams.bGroupIdSelectSupported = m_groupIdSelectSupported;
    walkerCodecParams.ucGroupId = m_groupId;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    // Add dump for MBEnc surface state heap here
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));
    )

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if ((!m_singleTaskPhaseSupported || m_lastTaskInPhase))
    {
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

#endif
MOS_STATUS CodechalEncodeAvcEncFeiG8::SetCurbeMe(MeCurbeParams* params)
{
    MOS_STATUS   eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    auto preEncParams = (FeiPreEncParams*)m_encodeParams.pPreEncParams;
    CODECHAL_ENCODE_CHK_NULL_RETURN(preEncParams);

    auto slcParams = m_avcSliceParams;
    bool framePicture = CodecHal_PictureIsFrame(m_currOriginalPic);
    uint32_t scaleFactor;
    uint8_t mvShiftFactor = 0, prevMvReadPosFactor = 0;
    bool useMvFromPrevStep, writeDistortions;
    switch (params->hmeLvl)
    {
    case HME_LEVEL_4x:
        useMvFromPrevStep = false;
        writeDistortions = false;
        scaleFactor = SCALE_FACTOR_4x;
        mvShiftFactor = 2;
        prevMvReadPosFactor = 0;
        break;
    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
        break;
    }

    ME_CURBE_CM_FEI cmd;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        &cmd,
        sizeof(ME_CURBE_CM_FEI),
        m_meCurbeCmFei,
        sizeof(ME_CURBE_CM_FEI)));

    cmd.DW3.SubPelMode = 3;
    if (m_fieldScalingOutputInterleaved)
    {
        cmd.DW3.SrcAccess =
            cmd.DW3.RefAccess = CodecHal_PictureIsField(m_currOriginalPic) ? 1 : 0;
        cmd.DW7.SrcFieldPolarity = CodecHal_PictureIsBottomField(m_currOriginalPic) ? 1 : 0;
    }

    cmd.DW4.PictureHeightMinus1 =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / scaleFactor) - 1;
    cmd.DW4.PictureWidth =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth / scaleFactor);
    cmd.DW5.QpPrimeY = preEncParams->dwFrameQp;
    cmd.DW6.WriteDistortions = writeDistortions;
    cmd.DW6.UseMvFromPrevStep = useMvFromPrevStep;

    cmd.DW6.SuperCombineDist = m_superCombineDistGeneric[m_targetUsage];
    cmd.DW6.MaxVmvR = (framePicture) ?
        CodecHalAvcEncode_GetMaxMvLen(CODEC_AVC_LEVEL_52) * 4 :
        (CodecHalAvcEncode_GetMaxMvLen(CODEC_AVC_LEVEL_52) >> 1) * 4;

    if (m_pictureCodingType == B_TYPE)
    {
        // This field is irrelevant since we are not using the bi-direct search.
        // set it to 32
        cmd.DW1.BiWeight = 32;
        cmd.DW13.NumRefIdxL1MinusOne = slcParams->num_ref_idx_l1_active_minus1;
    }

    if (m_pictureCodingType == P_TYPE ||
        m_pictureCodingType == B_TYPE)
    {
        cmd.DW13.NumRefIdxL0MinusOne = slcParams->num_ref_idx_l0_active_minus1;
    }

    if (!framePicture)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            cmd.DW14.List0RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_0);
            cmd.DW14.List0RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_1);
            cmd.DW14.List0RefID2FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_2);
            cmd.DW14.List0RefID3FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_3);
            cmd.DW14.List0RefID4FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_4);
            cmd.DW14.List0RefID5FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_5);
            cmd.DW14.List0RefID6FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_6);
            cmd.DW14.List0RefID7FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        }
        if (m_pictureCodingType == B_TYPE)
        {
            cmd.DW14.List1RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
            cmd.DW14.List1RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        }
    }

    cmd.DW15.MvShiftFactor = mvShiftFactor;
    cmd.DW15.PrevMvReadPosFactor = prevMvReadPosFactor;

    // r3 & r4
    uint8_t meMethod = (m_pictureCodingType == B_TYPE) ? m_bMeMethodGeneric[m_targetUsage] : m_meMethodGeneric[m_targetUsage];
    uint8_t tableIdx = (m_pictureCodingType == B_TYPE) ? 1 : 0;
    eStatus = MOS_SecureMemcpy(&(cmd.SPDelta), 14 * sizeof(uint32_t), CodechalEncoderState::m_encodeSearchPath[tableIdx][meMethod], 14 * sizeof(uint32_t));
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }
    if(params->pCurbeBinary)
    {
        MOS_SecureMemcpy(params->pCurbeBinary,m_feiMeCurbeDataSize,&cmd,m_feiMeCurbeDataSize);
        return eStatus;
    }
    // r5
    cmd.DW32._4xMeMvOutputDataSurfIndex = ME_MV_DATA_SURFACE_CM;
    cmd.DW33._16xOr32xMeMvInputDataSurfIndex = (params->hmeLvl == HME_LEVEL_32x) ?
        ME_32x_MV_DATA_SURFACE_CM : ME_16x_MV_DATA_SURFACE_CM;
    cmd.DW34._4xMeOutputDistSurfIndex = ME_DISTORTION_SURFACE_CM;
    cmd.DW35._4xMeOutputBrcDistSurfIndex = ME_BRC_DISTORTION_CM;
    cmd.DW36.VMEFwdInterPredictionSurfIndex = ME_CURR_FOR_FWD_REF_CM;
    cmd.DW37.VMEBwdInterPredictionSurfIndex = ME_CURR_FOR_BWD_REF_CM;
    cmd.DW38.Value = 0; //MBZ for BDW

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::SendMeSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, MeSurfaceParams* params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pCurrOriginalPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeMvDataBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psMeBrcDistortionBuffer);

    // only 4x HME is supported for FEI
    CODECHAL_ENCODE_ASSERT(!params->b32xMeInUse && !params->b16xMeInUse);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pMeBindingTable);
    auto meBindingTable = params->pMeBindingTable;
    bool currFieldPicture = CodecHal_PictureIsField(*(params->pCurrOriginalPic)) ? 1 : 0;
    bool currBottomField = CodecHal_PictureIsBottomField(*(params->pCurrOriginalPic)) ? 1 : 0;
    uint8_t currVDirection = (!currFieldPicture) ? CODECHAL_VDIRECTION_FRAME :
        ((currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
    uint8_t scaledIdx = params->ppRefList[m_currReconstructedPic.FrameIdx]->ucScalingIdx;
    auto currScaledSurface = m_trackedBuf->Get4xDsSurface(scaledIdx);
    CODECHAL_ENCODE_CHK_NULL_RETURN(currScaledSurface);
    auto meMvDataBuffer = params->ps4xMeMvDataBuffer;
    uint32_t meMvBottomFieldOffset = params->dw4xMeMvBottomFieldOffset;
    uint32_t currScaledBottomFieldOffset = params->dw4xScaledBottomFieldOffset;

    // Reference height and width information should be taken from the current scaled surface rather
    // than from the reference scaled surface in the case of PAFF.
    auto refScaledSurface = *currScaledSurface;

    uint32_t width = MOS_ALIGN_CEIL(params->dwDownscaledWidthInMb * 32, 64);
    uint32_t height = params->dwDownscaledHeightInMb * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER;

    // Force the values
    meMvDataBuffer->dwWidth = width;
    meMvDataBuffer->dwHeight = height;
    meMvDataBuffer->dwPitch = width;

    CODECHAL_SURFACE_CODEC_PARAMS   surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface = true;
    surfaceParams.bMediaBlockRW = true;
    surfaceParams.psSurface = meMvDataBuffer;
    surfaceParams.dwOffset = meMvBottomFieldOffset;
    surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    surfaceParams.dwBindingTableOffset = meBindingTable->dwMEMVDataSurface;
    surfaceParams.bIsWritable = true;
    surfaceParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));

    // Insert Distortion buffers only for 4xMe case
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface = true;
    surfaceParams.bMediaBlockRW = true;
    surfaceParams.psSurface = params->psMeBrcDistortionBuffer;
    surfaceParams.dwOffset = params->dwMeBrcDistortionBottomFieldOffset;
    surfaceParams.dwBindingTableOffset = meBindingTable->dwMEBRCDist;
    surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value;
    surfaceParams.bIsWritable = true;
    surfaceParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));

    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface = true;
    surfaceParams.bMediaBlockRW = true;
    surfaceParams.psSurface = params->psMeDistortionBuffer;
    surfaceParams.dwOffset = params->dwMeDistortionBottomFieldOffset;
    surfaceParams.dwBindingTableOffset = meBindingTable->dwMEDist;
    surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
    surfaceParams.bIsWritable = true;
    surfaceParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));

    // Setup references 1...n
    // LIST 0 references
    for (uint8_t refIdx = 0; refIdx <= params->dwNumRefIdxL0ActiveMinus1; refIdx++)
    {
        auto refPic = params->pL0RefFrameList[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && params->pPicIdx[refPic.FrameIdx].bValid)
        {
            if (refIdx == 0)
            {
                // Current Picture Y - VME
                MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
                surfaceParams.bUseAdvState = true;
                surfaceParams.psSurface = currScaledSurface;
                surfaceParams.dwOffset = currBottomField ? currScaledBottomFieldOffset : 0;
                surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
                surfaceParams.dwBindingTableOffset = meBindingTable->dwMECurrForFwdRef;
                surfaceParams.ucVDirection = currVDirection;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceParams,
                    params->pKernelState));
            }

            bool refFieldPicture = CodecHal_PictureIsField(refPic) ? 1 : 0;
            bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
            uint8_t refPicIdx = params->pPicIdx[refPic.FrameIdx].ucPicIdx;
            scaledIdx = params->ppRefList[refPicIdx]->ucScalingIdx;

            MOS_SURFACE* p4xSurface = m_trackedBuf->Get4xDsSurface(scaledIdx);
            if (p4xSurface != nullptr)
            {
                refScaledSurface.OsResource = p4xSurface->OsResource;
            }
            else
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
            }
            uint32_t refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;

            // L0 Reference Picture Y - VME
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.bUseAdvState = true;
            surfaceParams.psSurface = &refScaledSurface;
            surfaceParams.dwOffset = refBottomField ? refScaledBottomFieldOffset : 0;
            surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
            surfaceParams.dwBindingTableOffset = meBindingTable->dwMEFwdRefPicIdx[refIdx];
            surfaceParams.ucVDirection = !currFieldPicture ? CODECHAL_VDIRECTION_FRAME :
                ((refBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceParams,
                params->pKernelState));

            if (currFieldPicture)
            {
                // VME needs to set ref index 1 too for field case
                surfaceParams.dwBindingTableOffset = meBindingTable->dwMEFwdRefPicIdx[refIdx + 1];
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceParams,
                    params->pKernelState));
            }
        }
    }

    // Setup references 1...n
    // LIST 1 references
    for (uint8_t refIdx = 0; refIdx <= params->dwNumRefIdxL1ActiveMinus1; refIdx++)
    {
        auto refPic = params->pL1RefFrameList[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && params->pPicIdx[refPic.FrameIdx].bValid)
        {
            if (refIdx == 0)
            {
                // Current Picture Y - VME
                MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
                surfaceParams.bUseAdvState = true;
                surfaceParams.psSurface = currScaledSurface;
                surfaceParams.dwOffset = currBottomField ? currScaledBottomFieldOffset : 0;
                surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
                surfaceParams.dwBindingTableOffset = meBindingTable->dwMECurrForBwdRef;
                surfaceParams.ucVDirection = currVDirection;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceParams,
                    params->pKernelState));
            }

            bool refFieldPicture = CodecHal_PictureIsField(refPic) ? 1 : 0;
            bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
            uint8_t refPicIdx = params->pPicIdx[refPic.FrameIdx].ucPicIdx;
            scaledIdx = params->ppRefList[refPicIdx]->ucScalingIdx;

            MOS_SURFACE* p4xSurface = m_trackedBuf->Get4xDsSurface(scaledIdx);
            if (p4xSurface != nullptr)
            {
                refScaledSurface.OsResource = p4xSurface->OsResource;
            }
            else
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
            }
            uint32_t refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;

            // L1 Reference Picture Y - VME
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.bUseAdvState = true;
            surfaceParams.psSurface = &refScaledSurface;
            surfaceParams.dwOffset = refBottomField ? refScaledBottomFieldOffset : 0;
            surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
            surfaceParams.dwBindingTableOffset = meBindingTable->dwMEBwdRefPicIdx[refIdx];
            surfaceParams.ucVDirection = (!currFieldPicture) ? CODECHAL_VDIRECTION_FRAME :
                ((refBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceParams,
                params->pKernelState));

            if (currFieldPicture)
            {
                // VME needs to set ref index 1 too for field case
                surfaceParams.dwBindingTableOffset = meBindingTable->dwMEBwdRefPicIdx[refIdx + 1];
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceParams,
                    params->pKernelState));
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::EncodeGetKernelHeaderAndSize(void *pvBinary, EncOperation operation, uint32_t krnStateIdx, void* pvKrnHeader, uint32_t *pdwKrnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pvBinary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pvKrnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pdwKrnSize);

    auto kernelHeaderTable = (PKERNEL_HEADER_FEI_CM)pvBinary;
    PCODECHAL_KERNEL_HEADER invalidEntry = &(kernelHeaderTable->AVC_WeightedPrediction) + 1;
    uint32_t nextKrnOffset = *pdwKrnSize;

    PCODECHAL_KERNEL_HEADER currKrnHeader;
    if (operation == ENC_SCALING4X)
    {
        currKrnHeader = &kernelHeaderTable->PLY_DScale_PLY;
    }
    else if (operation == ENC_ME)
    {
        currKrnHeader = &kernelHeaderTable->AVC_ME_P;
    }
    else if (operation == ENC_MBENC)
    {
        currKrnHeader = &kernelHeaderTable->AVCMBEnc_Fei_I;
    }
    else if (operation == ENC_PREPROC)
    {
        currKrnHeader = &kernelHeaderTable->AVC_Fei_ProProc;
    }
    else if (operation == ENC_WP)
    {
        currKrnHeader = &kernelHeaderTable->AVC_WeightedPrediction;
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    currKrnHeader += krnStateIdx;
    *((PCODECHAL_KERNEL_HEADER)pvKrnHeader) = *currKrnHeader;

    PCODECHAL_KERNEL_HEADER nextKrnHeader = (currKrnHeader + 1);
    if (nextKrnHeader < invalidEntry)
    {
        nextKrnOffset = nextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *pdwKrnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::InitializeState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcEncG8::InitializeState());

    if (m_codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC)
    {
        m_hmeSupported = true;
        m_flatnessCheckSupported = true;
    }

    if (m_feiEnable)
    {
        if (m_codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC)
        {
            m_hmeSupported = true;
            m_flatnessCheckSupported = true;
        }

        m_16xMeSupported = false;
        m_32xMeSupported = false;
    }

    bWeightedPredictionSupported = true;
    m_brcHistoryBufferSize = m_brcHistoryBufferSize;
    dwBrcConstantSurfaceWidth = m_brcConstantSurfaceWidth;
    dwBrcConstantSurfaceHeight = m_brcConstantSurfaceHeight;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::ValidateNumReferences(PCODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pAvcSliceParams);

    uint8_t numRefIdx0MinusOne = params->pAvcSliceParams->num_ref_idx_l0_active_minus1;
    uint8_t numRefIdx1MinusOne = params->pAvcSliceParams->num_ref_idx_l1_active_minus1;

    // Nothing to do here if numRefIdx = 0 and frame encoded
    if (numRefIdx0MinusOne == 0 && !CodecHal_PictureIsField(params->pPicParams->CurrOriginalPic))
    {
        if (params->wPictureCodingType == P_TYPE ||
            (params->wPictureCodingType == B_TYPE && numRefIdx1MinusOne == 0))
        {
            return eStatus;
        }
    }

    if (params->wPictureCodingType == P_TYPE || params->wPictureCodingType == B_TYPE)
    {
        uint8_t maxPNumRefIdx0MinusOne = CODECHAL_ENCODE_AVC_MAX_NUM_REF_L0 - 1;
        uint8_t maxPNumRefIdx1MinusOne = CODECHAL_ENCODE_AVC_MAX_NUM_REF_L1 - 1;
        if(params->bPAKonly)
        {
            maxPNumRefIdx0MinusOne = CODEC_AVC_MAX_NUM_REF_FRAME - 1;
            maxPNumRefIdx1MinusOne = CODEC_AVC_MAX_NUM_REF_FRAME - 1;
        }
        if (params->wPictureCodingType == P_TYPE)
        {
            if (numRefIdx0MinusOne > maxPNumRefIdx0MinusOne)
            {
                CODECHAL_ENCODE_NORMALMESSAGE("Invalid active reference list size.");
                numRefIdx0MinusOne = maxPNumRefIdx0MinusOne;
            }
            numRefIdx1MinusOne = 0;
        }
        else // B_TYPE
        {
            if (numRefIdx0MinusOne > maxPNumRefIdx0MinusOne)
            {
                CODECHAL_ENCODE_NORMALMESSAGE("Invalid active reference list size.");
                numRefIdx0MinusOne = maxPNumRefIdx0MinusOne;
            }

            if (numRefIdx1MinusOne > maxPNumRefIdx1MinusOne)
            {
                CODECHAL_ENCODE_NORMALMESSAGE("Invalid active reference list size.");
                numRefIdx1MinusOne = maxPNumRefIdx1MinusOne;
            }

            // supports at most 1 L1 ref for frame picture for non-PAK only case
            if (CodecHal_PictureIsFrame(params->pPicParams->CurrOriginalPic) && numRefIdx1MinusOne > 0  && (!params->bPAKonly))
            {
                numRefIdx1MinusOne = 0;
            }
        }
    }

    // Override number of references used by VME. PAK uses value from DDI (num_ref_idx_l*_active_minus1_from_DDI)
    params->pAvcSliceParams->num_ref_idx_l0_active_minus1 = numRefIdx0MinusOne;
    params->pAvcSliceParams->num_ref_idx_l1_active_minus1 = numRefIdx1MinusOne;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::InitKernelStateWP()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    pWPKernelState = MOS_New(MHW_KERNEL_STATE);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pWPKernelState);

    auto kernelStatePtr = pWPKernelState;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize));

    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeGetKernelHeaderAndSize(
        kernelBinary,
        ENC_WP,
        0,
        &currKrnHeader,
        &kernelSize));
    kernelStatePtr->KernelParams.iBTCount = WP_NUM_SURFACES;
    kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelStatePtr->KernelParams.iCurbeLength = sizeof(WP_CURBE);
    kernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
    kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
    kernelStatePtr->KernelParams.iIdCount = 1;
    kernelStatePtr->KernelParams.iInlineDataLength = 0;

    kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    kernelStatePtr->KernelParams.pBinary = kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    kernelStatePtr->KernelParams.iSize = kernelSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        kernelStatePtr->KernelParams.iBTCount,
        &kernelStatePtr->dwSshSize,
        &kernelStatePtr->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::GetMbEncKernelStateIdx(CodechalEncodeIdOffsetParams* params, uint32_t* pdwKernelOffset)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pdwKernelOffset);

    *pdwKernelOffset = MBENC_I_OFFSET_CM;

    if (params->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_ADV)
    {
        *pdwKernelOffset += MBENC_TARGET_USAGE_CM;
    }

    if (params->wPictureCodingType == P_TYPE)
    {
        *pdwKernelOffset += MBENC_P_OFFSET_CM;
    }
    else if (params->wPictureCodingType == B_TYPE)
    {
        *pdwKernelOffset += MBENC_B_OFFSET_CM;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::SetCurbeAvcMbEnc(PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSlcParams);

    auto seqParams = params->pSeqParams;
    CODECHAL_ENCODE_ASSERT(seqParams->TargetUsage == TARGETUSAGE_RT_SPEED);
    // set sliceQP to MAX_SLICE_QP for  MbEnc kernel, we can use it to verify whether QP is changed or not
    auto picParams = params->pPicParams;
    auto slcParams = params->pSlcParams;
    uint8_t sliceQP = (params->bUseMbEncAdvKernel && params->bBrcEnabled) ? CODECHAL_ENCODE_AVC_MAX_SLICE_QP : picParams->pic_init_qp_minus26 + 26 + slcParams->slice_qp_delta;
    bool framePicture = CodecHal_PictureIsFrame(picParams->CurrOriginalPic);

    MBENC_CURBE_CM_FEI  cmd;

    if (params->bMbEncIFrameDistEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &cmd,
            sizeof(MBENC_CURBE_CM_FEI),
            m_feiMbEncCurbeIFrameDist,
            sizeof(MBENC_CURBE_CM_FEI)));
    }
    else
    {
        switch (m_pictureCodingType)
        {
        case I_TYPE:
            if (framePicture)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &cmd,
                    sizeof(MBENC_CURBE_CM_FEI),
                    m_feiMbEncCurbeNormalIFrame,
                    sizeof(MBENC_CURBE_CM_FEI)));
            }
            else
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &cmd,
                    sizeof(MBENC_CURBE_CM_FEI),
                    m_feiMbEncCurbeNormalIField,
                    sizeof(MBENC_CURBE_CM_FEI)));
            }
            break;

        case P_TYPE:
            if (framePicture)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &cmd,
                    sizeof(MBENC_CURBE_CM_FEI),
                    m_feiMbEncCurbeNormalPFrame,
                    sizeof(MBENC_CURBE_CM_FEI)));
            }
            else
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &cmd,
                    sizeof(MBENC_CURBE_CM_FEI),
                    m_feiMbEncCurbeNormalPfield,
                    sizeof(MBENC_CURBE_CM_FEI)));
            }
            break;

        case B_TYPE:
            if (framePicture)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &cmd,
                    sizeof(MBENC_CURBE_CM_FEI),
                    m_feiMbEncCurbeNormalBFrame,
                    sizeof(MBENC_CURBE_CM_FEI)));
            }
            else
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &cmd,
                    sizeof(MBENC_CURBE_CM_FEI),
                    m_feiMbEncCurbeNormalBField,
                    sizeof(MBENC_CURBE_CM_FEI)));
            }
            break;

        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture coding type.");
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }
    }
    auto feiPicParams = (CodecEncodeAvcFeiPicParams *)(m_avcFeiPicParams);
    uint8_t ucMeMethod  = (feiPicParams->SearchWindow == 5) || (feiPicParams->SearchWindow == 8) ? 4 : 6; // 4 means full search, 6 means diamand search
    uint32_t refWidth    = feiPicParams->RefWidth;
    uint32_t refHeight   = feiPicParams->RefHeight;
    uint32_t lenSP       = feiPicParams->LenSP;
    switch (feiPicParams->SearchWindow)
    {
    case 0:
        // not use predefined search window
        if((feiPicParams->SearchPath != 0) && (feiPicParams->SearchPath != 1) && (feiPicParams->SearchPath != 2))
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI MB ENC input SearchPath for SearchWindow=0 case!!!.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
        ucMeMethod = (feiPicParams->SearchPath == 1) ? 6 : 4; // 4 means full search, 6 means diamand search
        if(((refWidth * refHeight) > 2048) || (refWidth > 64) || (refHeight > 64))
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI MB ENC input refWidth/refHeight size for SearchWindow=0 case!!!.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
        break;
    case 1:
        // Tiny SUs 24x24 window
        refWidth  = 24;
        refHeight = 24;
        lenSP     = 4;
        break;
    case 2:
        // Small SUs 28x28 window
        refWidth  = 28;
        refHeight = 28;
        lenSP     = 9;
        break;
    case 3:
        // Diamond SUs 48x40 window
        refWidth  = 48;
        refHeight = 40;
        lenSP     = 16;
        break;
    case 4:
        // Large Diamond SUs 48x40 window
        refWidth  = 48;
        refHeight = 40;
        lenSP     = 32;
        break;
    case 5:
        // Exhaustive SUs 48x40 window
        refWidth  = 48;
        refHeight = 40;
        lenSP     = 48;
        break;
    case 6:
        // Diamond SUs 64x32 window
        refWidth  = 64;
        refHeight = 32;
        lenSP     = 16;
        break;
    case 7:
        // Large Diamond  SUs 64x32 window
        refWidth  = 64;
        refHeight = 32;
        lenSP     = 32;
    case 8:
        // Exhaustive SUs 64x32 window
        refWidth  = 64;
        refHeight = 32;
        lenSP     = 48;
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI MB ENC SearchWindow value!!!.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if((m_pictureCodingType == B_TYPE) && (cmd.DW3.BMEDisableFBR == 0))
    {
        if(refWidth > 32)
        {
            refWidth  = 32;
        }
        if(refHeight > 32)
        {
            refHeight = 32;
        }
    }

    // r1
    cmd.DW0.AdaptiveEn =
        cmd.DW37.AdaptiveEn = feiPicParams->AdaptiveSearch;
    cmd.DW0.T8x8FlagForInterEn =
        cmd.DW37.T8x8FlagForInterEn = picParams->transform_8x8_mode_flag;
    cmd.DW2.LenSP = lenSP;
    cmd.DW38.LenSP = 0; // MBZ
    cmd.DW2.MaxNumSU = cmd.DW38.MaxNumSU = 57;
    cmd.DW3.SrcAccess =
        cmd.DW3.RefAccess = framePicture ? 0 : 1;
    if (m_pictureCodingType != I_TYPE && bFTQEnable)
    {
        if (m_pictureCodingType == P_TYPE)
        {
            cmd.DW3.FTEnable = FTQBasedSkip[seqParams->TargetUsage] & 0x01;
        }
        else // B_TYPE
        {
            cmd.DW3.FTEnable = (FTQBasedSkip[seqParams->TargetUsage] >> 1) & 0x01;
        }
    }
    else
    {
        cmd.DW3.FTEnable = 0;
    }

    cmd.DW2.PicWidth = params->wPicWidthInMb;
    cmd.DW3.SubMbPartMask = feiPicParams->SubMBPartMask;
    cmd.DW3.SubPelMode = feiPicParams->SubPelMode;
    cmd.DW3.InterSAD = feiPicParams->InterSAD;
    cmd.DW3.IntraSAD = feiPicParams->IntraSAD;
    cmd.DW3.SearchCtrl = (m_pictureCodingType == B_TYPE) ? 7 : 0;
    cmd.DW4.PicHeightMinus1 = params->wFieldFrameHeightInMb - 1;
    cmd.DW4.TrueDistortionEnable = feiPicParams->DistortionType == 0 ? 1 : 0;

    bool bottomField = CodecHal_PictureIsBottomField(picParams->CurrOriginalPic);
    cmd.DW4.FieldParityFlag =
        cmd.DW7.SrcFieldPolarity = bottomField ? 1 : 0;
    cmd.DW4.bCurFldIDR = framePicture ? 0 : (picParams->bIdrPic || m_firstFieldIdrPic);
    cmd.DW4.ConstrainedIntraPredFlag = picParams->constrained_intra_pred_flag;
    cmd.DW4.HMEEnable = m_hmeEnabled;
    cmd.DW4.PictureType = m_pictureCodingType - 1;
    cmd.DW4.UseActualRefQPValue = m_hmeEnabled ? (MRDisableQPCheck[seqParams->TargetUsage] == 0) : false;
    cmd.DW4.FEIEnable = 1;
    cmd.DW4.MultipleMVPredictorPerMBEnable = feiPicParams->MVPredictorEnable;
    cmd.DW4.VMEDistortionOutputEnable = feiPicParams->DistortionEnable;
    cmd.DW4.PerMBQpEnable = feiPicParams->bMBQp;
    cmd.DW4.MBInputEnable = feiPicParams->bPerMBInput;
    cmd.DW4.HMEEnable = 0;
    cmd.DW5.SliceMbHeight = params->usSliceHeight;
    cmd.DW7.IntraPartMask = feiPicParams->IntraPartMask;

    // r2
    if (params->bMbEncIFrameDistEnabled)
    {
        cmd.DW6.BatchBufferEnd = 0;
    }
    else
    {
        uint8_t tableIdx = m_pictureCodingType - 1;
        eStatus = MOS_SecureMemcpy(&(cmd.ModeMvCost), 8 * sizeof(uint32_t), ModeMvCost_Cm[tableIdx][sliceQP], 8 * sizeof(uint32_t));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }

        if (m_pictureCodingType == I_TYPE && bOldModeCostEnable)
        {
            // Old intra mode cost needs to be used if bOldModeCostEnable is 1
            cmd.ModeMvCost.DW8.Value = OldIntraModeCost_Cm_Common[sliceQP];
        }
        else if (m_skipBiasAdjustmentEnable)
        {
            // Load different MvCost for P picture when SkipBiasAdjustment is enabled
            // No need to check for P picture as the flag is only enabled for P picture
            cmd.ModeMvCost.DW11.Value = MvCost_PSkipAdjustment_Cm_Common[sliceQP];
        }
    }

    // r3 & r4
    if (params->bMbEncIFrameDistEnabled)
    {
        cmd.SPDelta.DW31.IntraComputeType = 1;
    }
    else
    {
        uint8_t tableIdx = (m_pictureCodingType == B_TYPE) ? 1 : 0;
        eStatus = MOS_SecureMemcpy(&(cmd.SPDelta), 16 * sizeof(uint32_t), CodechalEncoderState::m_encodeSearchPath[tableIdx][ucMeMethod], 16 * sizeof(uint32_t));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }

    // r5
    if (m_pictureCodingType == P_TYPE)
    {
        cmd.DW32.SkipVal = SkipVal_P_Common
            [cmd.DW3.BlockBasedSkipEnable]
        [picParams->transform_8x8_mode_flag]
        [sliceQP];
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        cmd.DW32.SkipVal = SkipVal_B_Common
            [cmd.DW3.BlockBasedSkipEnable]
        [picParams->transform_8x8_mode_flag]
        [sliceQP];
    }

    cmd.ModeMvCost.DW13.QpPrimeY = sliceQP;
    // QpPrimeCb and QpPrimeCr are not used by Kernel. Following settings are for CModel matching.
    cmd.ModeMvCost.DW13.QpPrimeCb = sliceQP;
    cmd.ModeMvCost.DW13.QpPrimeCr = sliceQP;
    cmd.ModeMvCost.DW13.TargetSizeInWord = 0xff; // hardcoded for BRC disabled

    if (bMultiPredEnable && (m_pictureCodingType != I_TYPE))
    {
        cmd.DW32.MultiPredL0Disable = feiPicParams->MultiPredL0 ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
        cmd.DW32.MultiPredL1Disable = (m_pictureCodingType == B_TYPE && feiPicParams->MultiPredL1) ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
    }
    else
    {
        cmd.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
        cmd.DW32.MultiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
    }

    if (!framePicture)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            cmd.DW34.List0RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_0);
            cmd.DW34.List0RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_1);
            cmd.DW34.List0RefID2FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_2);
            cmd.DW34.List0RefID3FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_3);
            cmd.DW34.List0RefID4FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_4);
            cmd.DW34.List0RefID5FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_5);
            cmd.DW34.List0RefID6FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_6);
            cmd.DW34.List0RefID7FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        }
        if (m_pictureCodingType == B_TYPE)
        {
            cmd.DW34.List1RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
            cmd.DW34.List1RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        }
    }

    if (m_pictureCodingType == B_TYPE)
    {
        cmd.DW34.List1RefID0FrameFieldFlag = GetRefPicFieldFlag(params, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        cmd.DW34.List1RefID1FrameFieldFlag = GetRefPicFieldFlag(params, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        cmd.DW34.bDirectMode = slcParams->direct_spatial_mv_pred_flag;
    }
    cmd.DW34.bOriginalBff = framePicture ? 0 :
        ((m_firstField && (bottomField)) || (!m_firstField && (!bottomField)));
    cmd.DW34.EnableMBFlatnessChkOptimization = m_flatnessCheckEnabled;
    cmd.DW34.ROIEnableFlag = params->bRoiEnabled;
    cmd.DW34.MADEnableFlag                   = m_madEnabled;
    cmd.DW34.MBBrcEnable = 0;
    cmd.DW36.CheckAllFractionalEnable = bCAFEnable;
    cmd.DW38.RefThreshold = m_refThreshold;
    cmd.DW39.HMERefWindowsCombThreshold = (m_pictureCodingType == B_TYPE) ?
        HMEBCombineLen[seqParams->TargetUsage] : HMECombineLen[seqParams->TargetUsage];

    // Those fields are not really used for I_dist kernel,
    // but set them to 0 to get bit-exact match with kernel prototype
    if (params->bMbEncIFrameDistEnabled)
    {
        cmd.ModeMvCost.DW13.QpPrimeY = 0;
        cmd.ModeMvCost.DW13.QpPrimeCb = 0;
        cmd.ModeMvCost.DW13.QpPrimeCr = 0;
        cmd.DW33.Intra16x16NonDCPredPenalty = 0;
        cmd.DW33.Intra4x4NonDCPredPenalty = 0;
        cmd.DW33.Intra8x8NonDCPredPenalty = 0;
    }

    //r6
    if (cmd.DW4.UseActualRefQPValue)
    {
        cmd.DW44.ActualQPValueForRefID0List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_0);
        cmd.DW44.ActualQPValueForRefID1List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_1);
        cmd.DW44.ActualQPValueForRefID2List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_2);
        cmd.DW44.ActualQPValueForRefID3List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_3);
        cmd.DW45.ActualQPValueForRefID4List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_4);
        cmd.DW45.ActualQPValueForRefID5List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_5);
        cmd.DW45.ActualQPValueForRefID6List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_6);
        cmd.DW45.ActualQPValueForRefID7List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        cmd.DW46.ActualQPValueForRefID0List1 = AVCGetQPValueFromRefList(params, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        cmd.DW46.ActualQPValueForRefID1List1 = AVCGetQPValueFromRefList(params, LIST_1, CODECHAL_ENCODE_REF_ID_1);
    }

    uint8_t tableIdx = m_pictureCodingType - 1;
    cmd.DW46.RefCost = RefCost_MultiRefQp[tableIdx][sliceQP];

    cmd.DW34.NumMVPredictorsL1 = 0;
    // Picture Coding Type dependent parameters
    if (m_pictureCodingType == I_TYPE)
    {
        cmd.DW0.SkipModeEn = 0;
        cmd.DW37.SkipModeEn = 0;
        cmd.DW36.HMECombineOverlap = 0;
        cmd.DW36.CheckAllFractionalEnable = 0;
        cmd.DW47.IntraCostSF = 16; // This is not used but recommended to set this to 16 by Kernel team
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        cmd.DW1.MaxNumMVs = GetMaxMvsPer2Mb(seqParams->Level) / 2;
        cmd.DW3.BMEDisableFBR = 1;
        cmd.DW5.RefWidth = cmd.DW39.RefWidth = refWidth;
        cmd.DW5.RefHeight = cmd.DW39.RefHeight = refHeight;
        cmd.DW7.NonSkipZMvAdded = 1;
        cmd.DW7.NonSkipModeAdded = 1;
        cmd.DW7.SkipCenterMask = 1;
        cmd.DW47.IntraCostSF =
            bAdaptiveIntraScalingEnable ?
            AdaptiveIntraScalingFactor_Cm_Common[sliceQP] :
            IntraScalingFactor_Cm_Common[sliceQP];
        cmd.DW47.MaxVmvR = (framePicture) ? CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) * 4 : (CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) >> 1) * 4;
        cmd.DW36.HMECombineOverlap = 1;
        cmd.DW36.NumRefIdxL0MinusOne = bMultiPredEnable ? slcParams->num_ref_idx_l0_active_minus1 : 0;
        cmd.DW36.NumMVPredictors = feiPicParams->NumMVPredictorsL0;
        cmd.DW34.NumMVPredictorsL1 = 0;
        cmd.DW36.CheckAllFractionalEnable = feiPicParams->RepartitionCheckEnable;
    }
    else
    {
        // B_TYPE
        cmd.DW1.MaxNumMVs = GetMaxMvsPer2Mb(seqParams->Level) / 2;
        cmd.DW1.BiWeight = m_biWeight;
        cmd.DW3.SkipType = 1;
        cmd.DW5.RefWidth = cmd.DW39.RefWidth = refWidth;
        cmd.DW5.RefHeight = cmd.DW39.RefHeight = refHeight;
        cmd.DW7.SkipCenterMask = 0xFF;
        cmd.DW47.IntraCostSF =
            bAdaptiveIntraScalingEnable ?
            AdaptiveIntraScalingFactor_Cm_Common[sliceQP] :
            IntraScalingFactor_Cm_Common[sliceQP];
        cmd.DW47.MaxVmvR = (framePicture) ? CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) * 4 : (CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) >> 1) * 4;
        cmd.DW36.HMECombineOverlap = 1;
        // Checking if the forward frame (List 1 index 0) is a short term reference
        {
            auto codecHalPic = params->pSlcParams->RefPicList[LIST_1][0];
            if (codecHalPic.PicFlags != PICTURE_INVALID &&
                codecHalPic.FrameIdx != CODECHAL_ENCODE_AVC_INVALID_PIC_ID &&
                params->pPicIdx[codecHalPic.FrameIdx].bValid)
            {
                // Although its name is FWD, it actually means the future frame or the backward reference frame
                cmd.DW36.IsFwdFrameShortTermRef = CodecHal_PictureIsShortTermRef(params->pPicParams->RefFrameList[codecHalPic.FrameIdx]);
            }
            else
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Invalid backward reference frame.");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
        }
        cmd.DW36.NumRefIdxL0MinusOne = bMultiPredEnable ? slcParams->num_ref_idx_l0_active_minus1 : 0;
        cmd.DW36.NumRefIdxL1MinusOne = bMultiPredEnable ? slcParams->num_ref_idx_l1_active_minus1 : 0;
        cmd.DW36.NumMVPredictors = feiPicParams->NumMVPredictorsL0;
        cmd.DW34.NumMVPredictorsL1 = feiPicParams->NumMVPredictorsL1;
        cmd.DW36.CheckAllFractionalEnable = feiPicParams->RepartitionCheckEnable;
        cmd.DW40.DistScaleFactorRefID0List0 = m_distScaleFactorList0[0];
        cmd.DW40.DistScaleFactorRefID1List0 = m_distScaleFactorList0[1];
        cmd.DW41.DistScaleFactorRefID2List0 = m_distScaleFactorList0[2];
        cmd.DW41.DistScaleFactorRefID3List0 = m_distScaleFactorList0[3];
        cmd.DW42.DistScaleFactorRefID4List0 = m_distScaleFactorList0[4];
        cmd.DW42.DistScaleFactorRefID5List0 = m_distScaleFactorList0[5];
        cmd.DW43.DistScaleFactorRefID6List0 = m_distScaleFactorList0[6];
        cmd.DW43.DistScaleFactorRefID7List0 = m_distScaleFactorList0[7];
        {
            CODEC_PICTURE    refPic;
            refPic = slcParams->RefPicList[LIST_1][0];
            cmd.DW63.L1ListRef0PictureCodingType = m_refList[m_picIdx[refPic.FrameIdx].ucPicIdx]->ucAvcPictureCodingType;
            if(framePicture && ((cmd.DW63.L1ListRef0PictureCodingType == CODEC_AVC_PIC_CODING_TYPE_TFF_FIELD) || (cmd.DW63.L1ListRef0PictureCodingType == CODEC_AVC_PIC_CODING_TYPE_BFF_FIELD)))
            {
                uint16_t fieldHeightInMb = (params->wFieldFrameHeightInMb + 1) >> 1;
                cmd.DW64.BottomFieldOffsetL1ListRef0MV     = MOS_ALIGN_CEIL(fieldHeightInMb * params->wPicWidthInMb * (32 * 4), 0x1000);
                cmd.DW65.BottomFieldOffsetL1ListRef0MBCode = fieldHeightInMb * params->wPicWidthInMb * 64;
        }
        }
    }

    *params->pdwBlockBasedSkipEn = cmd.DW3.BlockBasedSkipEnable;

    if (picParams->EnableRollingIntraRefresh)
    {
        /* Multiple predictor should be completely disabled for the RollingI feature. This does not lead to much quality drop for P frames especially for TU as 1 */
        cmd.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;

        /* Pass the same IntraRefreshUnit to the kernel w/o the adjustment by -1, so as to have an overlap of one MB row or column of Intra macroblocks
        across one P frame to another P frame, as needed by the RollingI algo */
        cmd.DW48.IntraRefreshMBNum = picParams->IntraRefreshMBNum; /* MB row or column number */
        cmd.DW48.IntraRefreshUnitInMBMinus1 = picParams->IntraRefreshUnitinMB;
        cmd.DW48.IntraRefreshQPDelta = picParams->IntraRefreshQPDelta;
    }

    if (true == params->bRoiEnabled)
    {
        cmd.DW49.ROI1_X_left = picParams->ROI[0].Left;
        cmd.DW49.ROI1_Y_top = picParams->ROI[0].Top;
        cmd.DW50.ROI1_X_right = picParams->ROI[0].Right;
        cmd.DW50.ROI1_Y_bottom = picParams->ROI[0].Bottom;

        cmd.DW51.ROI2_X_left = picParams->ROI[1].Left;
        cmd.DW51.ROI2_Y_top = picParams->ROI[1].Top;
        cmd.DW52.ROI2_X_right = picParams->ROI[1].Right;
        cmd.DW52.ROI2_Y_bottom = picParams->ROI[1].Bottom;

        cmd.DW53.ROI3_X_left = picParams->ROI[2].Left;
        cmd.DW53.ROI3_Y_top = picParams->ROI[2].Top;
        cmd.DW54.ROI3_X_right = picParams->ROI[2].Right;
        cmd.DW54.ROI3_Y_bottom = picParams->ROI[2].Bottom;

        cmd.DW55.ROI4_X_left = picParams->ROI[3].Left;
        cmd.DW55.ROI4_Y_top = picParams->ROI[3].Top;
        cmd.DW56.ROI4_X_right = picParams->ROI[3].Right;
        cmd.DW56.ROI4_Y_bottom = picParams->ROI[3].Bottom;

        if (bBrcEnabled == false)
        {
            uint16_t numROI = picParams->NumROI;
            char priorityLevelOrDQp[CODECHAL_ENCODE_AVC_MAX_ROI_NUMBER] = { 0 };

            // cqp case
            for (unsigned int i = 0; i < numROI; i += 1)
            {
                char dQpRoi = picParams->ROI[i].PriorityLevelOrDQp;

                // clip qp roi in order to have (qp + qpY) in range [0, 51]
                priorityLevelOrDQp[i] = (char)CodecHal_Clip3(-sliceQP, CODECHAL_ENCODE_AVC_MAX_SLICE_QP - sliceQP, dQpRoi);
            }

            cmd.DW57.ROI1_dQpPrimeY = priorityLevelOrDQp[0];
            cmd.DW57.ROI2_dQpPrimeY = priorityLevelOrDQp[1];
            cmd.DW57.ROI3_dQpPrimeY = priorityLevelOrDQp[2];
            cmd.DW57.ROI4_dQpPrimeY = priorityLevelOrDQp[3];
        }
        else
        {
            // kernel does not support BRC case
            cmd.DW34.ROIEnableFlag = 0;
        }
    }
    //older isa curbe length is 236, new is 252
    if(params->pCurbeBinary)
    {
        MOS_SecureMemcpy(params->pCurbeBinary,m_feiMBEncCurbeDataSizeExcludeSurfaceIdx,&cmd,m_feiMBEncCurbeDataSizeExcludeSurfaceIdx);
        return eStatus;
    }
    //IPCM QP and threshold
    cmd.DW59.IPCM_QP0                       = g_cInit_CODECHAL_ENCODE_AVC_IPCM_Threshold_Table_G8[0].QP;
    cmd.DW59.IPCM_QP1                       = g_cInit_CODECHAL_ENCODE_AVC_IPCM_Threshold_Table_G8[1].QP;
    cmd.DW59.IPCM_QP2                       = g_cInit_CODECHAL_ENCODE_AVC_IPCM_Threshold_Table_G8[2].QP;
    cmd.DW59.IPCM_QP3                       = g_cInit_CODECHAL_ENCODE_AVC_IPCM_Threshold_Table_G8[3].QP;
    cmd.DW60.IPCM_QP4                       = g_cInit_CODECHAL_ENCODE_AVC_IPCM_Threshold_Table_G8[4].QP;
    cmd.DW60.IPCM_Thresh0                   = g_cInit_CODECHAL_ENCODE_AVC_IPCM_Threshold_Table_G8[0].Threshold;
    cmd.DW61.IPCM_Thresh1                   = g_cInit_CODECHAL_ENCODE_AVC_IPCM_Threshold_Table_G8[1].Threshold;
    cmd.DW61.IPCM_Thresh2                   = g_cInit_CODECHAL_ENCODE_AVC_IPCM_Threshold_Table_G8[2].Threshold;
    cmd.DW62.IPCM_Thresh3                   = g_cInit_CODECHAL_ENCODE_AVC_IPCM_Threshold_Table_G8[3].Threshold;
    cmd.DW62.IPCM_Thresh4                   = g_cInit_CODECHAL_ENCODE_AVC_IPCM_Threshold_Table_G8[4].Threshold;

    cmd.DW66.MBDataSurfIndex                = MBENC_MFC_AVC_PAK_OBJ_CM_FEI;
    cmd.DW67.MVDataSurfIndex                = MBENC_IND_MV_DATA_CM_FEI;
    cmd.DW68.IDistSurfIndex                 = MBENC_BRC_DISTORTION_CM_FEI;
    cmd.DW69.SrcYSurfIndex                  = MBENC_CURR_Y_CM_FEI;
    cmd.DW70.MBSpecificDataSurfIndex        = MBENC_MB_SPECIFIC_DATA_CM_FEI;
    cmd.DW71.AuxVmeOutSurfIndex             = MBENC_AUX_VME_OUT_CM_FEI;
    cmd.DW72.CurrRefPicSelSurfIndex         = MBENC_REFPICSELECT_L0_CM_FEI;
    cmd.DW73.HMEMVPredFwdBwdSurfIndex       = MBENC_MV_DATA_FROM_ME_CM_FEI;
    cmd.DW74.HMEDistSurfIndex               = MBENC_4xME_DISTORTION_CM_FEI;
    cmd.DW75.RefPicSelectL1SurfIndex        = MBENC_REFPICSELECT_L1_CM_FEI;
    cmd.DW76.FwdFrmMBDataSurfIndex          = MBENC_FWD_MB_DATA_CM_FEI;
    cmd.DW77.FwdFrmMVSurfIndex              = MBENC_FWD_MV_DATA_CM_FEI;
    cmd.DW78.MBQPBuffer                     = MBENC_MBQP_CM_FEI;
    cmd.DW79.MBBRCLut                       = MBENC_MBBRC_CONST_DATA_CM_FEI;
    cmd.DW80.VMEInterPredictionSurfIndex    = MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_CM_FEI;
    cmd.DW81.VMEInterPredictionMRSurfIndex  = MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_CM_FEI;
    cmd.DW82.FlatnessChkSurfIndex           = MBENC_FLATNESS_CHECK_CM_FEI;
    cmd.DW83.MADSurfIndex                   = MBENC_MAD_DATA_CM_FEI;
    cmd.DW84.InterDistortionSurfIndex       = MBENC_INTER_DISTORTION_CM_FEI;
    cmd.DW86.BRCCurbeSurfIndex              = MBENC_BRC_CURBE_DATA_CM_FEI;
    cmd.DW87.MvPredictorSurfIndex           = MBENC_MV_PREDICTOR_CM_FEI;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;

}

MOS_STATUS CodechalEncodeAvcEncFeiG8::SetCurbeAvcPreProc(PCODECHAL_ENCODE_AVC_PREPROC_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPreEncParams);

    auto preEncParams = params->pPreEncParams;

    uint32_t sliceQp = preEncParams->dwFrameQp;
    bool framePicture = CodecHal_PictureIsFrame(m_currOriginalPic);
    bool bottomField = CodecHal_PictureIsBottomField(m_currOriginalPic);
    PREPROC_CURBE_CM cmd;
    switch (m_pictureCodingType)
    {
    case I_TYPE:
        if (framePicture)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &cmd,
                sizeof(PREPROC_CURBE_CM),
                m_preProcCurbeCmNormalIFrame,
                sizeof(PREPROC_CURBE_CM)));
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &cmd,
                sizeof(PREPROC_CURBE_CM),
                m_preProcCurbeCmNormalIfield,
                sizeof(PREPROC_CURBE_CM)));
        }
        break;

    case P_TYPE:
        if (framePicture)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &cmd,
                sizeof(PREPROC_CURBE_CM),
                m_preProcCurbeCmNormalPFrame,
                sizeof(PREPROC_CURBE_CM)));
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &cmd,
                sizeof(PREPROC_CURBE_CM),
                m_preProcCurbeCmNormalPField,
                sizeof(PREPROC_CURBE_CM)));
        }
        break;

    case B_TYPE:
        if (framePicture)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &cmd,
                sizeof(PREPROC_CURBE_CM),
                m_preProcCurbeCmNormalBFrame,
                sizeof(PREPROC_CURBE_CM)));
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &cmd,
                sizeof(PREPROC_CURBE_CM),
                m_preProcCurbeCmNormalBField,
                sizeof(PREPROC_CURBE_CM)));
        }
        break;

    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture coding type.");
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    uint8_t meMethod = (preEncParams->dwSearchWindow == 5) || (preEncParams->dwSearchWindow == 8) ? 4 : 6; // 4 means full search, 6 means diamand search
    uint32_t refWidth = preEncParams->dwRefWidth;
    uint32_t refHeight = preEncParams->dwRefHeight;
    uint32_t lenSP = preEncParams->dwLenSP;
    switch (preEncParams->dwSearchWindow)
    {
    case 0:
        // not use predefined search window and search patch
        if ((preEncParams->dwSearchPath != 0) && (preEncParams->dwSearchPath != 1) && (preEncParams->dwSearchPath != 2))
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI PREENC input SearchPath for SearchWindow=0 case!!!.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
        meMethod = (preEncParams->dwSearchPath == 1) ? 6 : 4; // 4 means full search, 6 means diamand search
        if (((refWidth * refHeight) > 2048) || (refWidth > 64) || (refHeight > 64))
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI PREENC PreProc input refWidth/refHeight size for SearchWindow=0 case!!!.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
        break;
    case 1:
        // Tiny 4 SUs 24x24 window
        refWidth = 24;
        refHeight = 24;
        lenSP = 4;
        break;
    case 2:
        // Small 9 SUs 28x28 window
        refWidth = 28;
        refHeight = 28;
        lenSP = 9;
        break;
    case 3:
        // Diamond 16 SUs 48x40 window
        refWidth = 48;
        refHeight = 40;
        lenSP = 16;
        break;
    case 4:
        // Large Diamond 32 SUs 48x40 window
        refWidth = 48;
        refHeight = 40;
        lenSP = 32;
        break;
    case 5:
        // Exhaustive 48 SUs 48x40 window
        refWidth = 48;
        refHeight = 40;
        lenSP = 48;
        break;
    case 6:
        // Diamond 16 SUs 64x32 window
        refWidth = 64;
        refHeight = 32;
        lenSP = 16;
        break;
    case 7:
        // Large Diamond 32 SUs 64x32 window
        refWidth = 64;
        refHeight = 32;
        lenSP = 32;
    case 8:
        // Exhaustive 48 SUs 64x32 window
        refWidth = 64;
        refHeight = 32;
        lenSP = 48;
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI PREENC PreProc SearchWindow value!!!.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    // r1
    cmd.DW0.AdaptiveEn =
        cmd.DW37.AdaptiveEn = preEncParams->bAdaptiveSearch;
    cmd.DW2.LenSP = lenSP;
    cmd.DW38.LenSP = 0; // MBZ
    cmd.DW2.MaxNumSU = cmd.DW38.MaxNumSU = 57;

    cmd.DW3.SrcAccess =
        cmd.DW3.RefAccess = framePicture ? 0 : 1;

    if (m_pictureCodingType != I_TYPE && bFTQEnable)
    {
        cmd.DW3.FTEnable = preEncParams->bFTEnable;
    }
    else
    {
        cmd.DW3.FTEnable = 0;
    }
    cmd.DW3.SubMbPartMask = preEncParams->dwSubMBPartMask;
    cmd.DW3.SubPelMode = preEncParams->dwSubPelMode;
    cmd.DW3.IntraSAD = preEncParams->dwIntraSAD;
    cmd.DW3.InterSAD = preEncParams->dwInterSAD;

    cmd.DW2.PicWidth = params->wPicWidthInMb;
    cmd.DW6.PicHeight = cmd.DW5.SliceMbHeight = params->wFieldFrameHeightInMb;

    cmd.DW4.FieldParityFlag = cmd.DW4.CurrPicFieldParityFlag =
        cmd.DW7.SrcFieldPolarity = bottomField ? 1 : 0;
    cmd.DW4.HMEEnable = m_hmeEnabled;
    cmd.DW4.FrameQp = sliceQp;
    cmd.DW4.PerMBQpEnable = preEncParams->bMBQp;
    cmd.DW4.MultipleMVPredictorPerMBEnable = (m_pictureCodingType == I_TYPE) ? 0 : preEncParams->dwMVPredictorCtrl;
    cmd.DW4.DisableMvOutput = (m_pictureCodingType == I_TYPE) ? 1 : preEncParams->bDisableMVOutput;
    cmd.DW4.DisableMbStats = preEncParams->bDisableStatisticsOutput;
    cmd.DW4.FwdRefPicEnable = preEncParams->dwNumPastReferences > 0 ? 1 : 0;
    cmd.DW4.FwdRefPicFrameFieldFlag = CodecHal_PictureIsFrame(preEncParams->PastRefPicture) ? 0 : 1;
    cmd.DW4.FwdRefPicFieldParityFlag = CodecHal_PictureIsBottomField(preEncParams->PastRefPicture);
    cmd.DW4.BwdRefPicEnable = preEncParams->dwNumFutureReferences > 0 ? 1 : 0;
    cmd.DW4.BwdRefPicFrameFieldFlag = CodecHal_PictureIsFrame(preEncParams->FutureRefPicture) ? 0 : 1;
    cmd.DW4.BwdRefPicFieldParityFlag = CodecHal_PictureIsBottomField(preEncParams->FutureRefPicture);
    cmd.DW7.IntraPartMask = preEncParams->dwIntraPartMask;

    uint8_t tableIdx = m_pictureCodingType - 1;
    eStatus = MOS_SecureMemcpy(&(cmd.ModeMvCost), 8 * sizeof(uint32_t), m_modeMvCost_Cm_PreProc[tableIdx][sliceQp], 8 * sizeof(uint32_t));
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }
    cmd.ModeMvCost.DW8.Value = 0;
    cmd.ModeMvCost.DW9.Value = 0;
    cmd.ModeMvCost.DW10.Value = 0;
    cmd.ModeMvCost.DW11.Value = 0;
    cmd.ModeMvCost.DW12.Value = 0;
    cmd.ModeMvCost.DW13.Value = 0;

    if (!framePicture && m_pictureCodingType != I_TYPE)
    {
        /* for field, R2.2 upper word should be set to zero for P and B frames */
        cmd.ModeMvCost.DW10.Value &= 0x0000FFFF;
    }

    tableIdx = (m_pictureCodingType == B_TYPE) ? 1 : 0;
    eStatus = MOS_SecureMemcpy(&(cmd.SPDelta), 16 * sizeof(uint32_t), CodechalEncoderState::m_encodeSearchPath[tableIdx][meMethod], 16 * sizeof(uint32_t));
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }
    // disable Intra Compute when disabling all mode in dwIntraPartMask
    if (preEncParams->dwIntraPartMask == 0x7)
    {
        cmd.SPDelta.DW31.IntraComputeType = 3;
    }

    cmd.DW38.RefThreshold = m_refThreshold;
    cmd.DW39.HMERefWindowsCombThreshold = (m_pictureCodingType == B_TYPE) ?
        HMEBCombineLen[m_targetUsage] : HMECombineLen[m_targetUsage];

    // Picture Coding Type dependent parameters
    if (m_pictureCodingType == I_TYPE)
    {
        cmd.DW0.SkipModeEn = 0;
        cmd.DW37.SkipModeEn = 0;
        cmd.DW36.HMECombineOverlap = 0;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        cmd.DW1.MaxNumMVs = GetMaxMvsPer2Mb(CODEC_AVC_LEVEL_52) / 2;
        cmd.DW3.BMEDisableFBR = 1;
        cmd.DW3.SearchCtrl = 0;
        cmd.DW5.RefWidth = cmd.DW39.RefWidth = refWidth;
        cmd.DW5.RefHeight = cmd.DW39.RefHeight = refHeight;
        cmd.DW7.NonSkipZMvAdded = 1;
        cmd.DW7.NonSkipModeAdded = 1;
        cmd.DW7.SkipCenterMask = 1;
        cmd.DW32.MaxVmvR = (framePicture) ? CodecHalAvcEncode_GetMaxMvLen(CODEC_AVC_LEVEL_52) * 4 : (CodecHalAvcEncode_GetMaxMvLen(CODEC_AVC_LEVEL_52) >> 1) * 4;
        cmd.DW36.HMECombineOverlap = 1;
    }
    else
    {
        // B_TYPE
        cmd.DW1.MaxNumMVs = GetMaxMvsPer2Mb(CODEC_AVC_LEVEL_52) / 2;
        cmd.DW3.SearchCtrl = 0;
        cmd.DW3.SkipType = 1;
        cmd.DW5.RefWidth = cmd.DW39.RefWidth = refWidth;
        cmd.DW5.RefHeight = cmd.DW39.RefHeight = refHeight;
        cmd.DW7.SkipCenterMask = 0xFF;
        cmd.DW32.MaxVmvR = (framePicture) ? CodecHalAvcEncode_GetMaxMvLen(CODEC_AVC_LEVEL_52) * 4 : (CodecHalAvcEncode_GetMaxMvLen(CODEC_AVC_LEVEL_52) >> 1) * 4;
        cmd.DW36.HMECombineOverlap = 1;
    }
    if(params->pCurbeBinary)
    {
        MOS_SecureMemcpy(params->pCurbeBinary,m_feiPreProcCurbeDataSize,&cmd,m_feiPreProcCurbeDataSize);
        return eStatus;
    }

    cmd.DW40.CurrPicSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_CURR_Y_CM_G8;
    cmd.DW41.HMEMvDataSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_HME_MV_DATA_CM_G8;
    cmd.DW42.MvPredictorSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_MV_PREDICTOR_CM_G8;
    cmd.DW43.MbQpSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_MBQP_CM_G8;
    cmd.DW44.MvDataOutSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_MV_DATA_CM_G8;
    cmd.DW45.MbStatsOutSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_MB_STATS_CM_G8;
    if (framePicture)
    {
        cmd.DW46.VMEInterPredictionSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_VME_CURR_PIC_IDX_0_CM_G8;
        cmd.DW47.VMEInterPredictionMRSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_VME_CURR_PIC_IDX_1_CM_G8;
        cmd.DW48.FtqLutSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_FTQ_LUT_CM_G8;
    }
    else
    {
        cmd.DW46.VMEInterPredictionSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_CURR_PIC_IDX_0_CM_G8;
        cmd.DW47.VMEInterPredictionMRSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_CURR_PIC_IDX_1_CM_G8;
        cmd.DW48.FtqLutSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_FIELD_FTQ_LUT_CM_G8;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG8::SendAvcPreProcSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_PREPROC_SURFACE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetOsInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pCurrOriginalPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psCurrPicSurface);

    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPreProcBindingTable);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPreEncParams);

    auto kernelState = params->pKernelState;

    auto preEncParams = (FeiPreEncParams*)params->pPreEncParams;
    auto avcPreProcBindingTable = params->pPreProcBindingTable;

    bool currFieldPicture = CodecHal_PictureIsField(*(params->pCurrOriginalPic)) ? 1 : 0;
    bool currBottomField = CodecHal_PictureIsBottomField(*(params->pCurrOriginalPic)) ? 1 : 0;

    uint8_t vDirection = (CodecHal_PictureIsFrame(*(params->pCurrOriginalPic))) ? CODECHAL_VDIRECTION_FRAME :
        (currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD;

    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true; // Use media block RW for DP 2D surface access
    surfaceCodecParams.bUseUVPlane = true;
    surfaceCodecParams.psSurface = params->psCurrPicSurface;
    surfaceCodecParams.dwOffset = 0;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = avcPreProcBindingTable->dwAvcPreProcCurrY;
    surfaceCodecParams.dwUVBindingTableOffset = avcPreProcBindingTable->dwAvcPreProcCurrUV;
    surfaceCodecParams.dwVerticalLineStride = params->dwVerticalLineStride;
    surfaceCodecParams.dwVerticalLineStrideOffset = params->dwVerticalLineStrideOffset;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // AVC_ME MV data buffer
    if (params->bHmeEnabled)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeMvDataBuffer);

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->ps4xMeMvDataBuffer;
        surfaceCodecParams.dwOffset = params->dwMeMvBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcPreProcBindingTable->dwAvcPreProcMVDataFromHME;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (preEncParams->dwMVPredictorCtrl)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));

        uint32_t size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 8;
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = &(preEncParams->resMvPredBuffer);
        surfaceCodecParams.dwOffset = 0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcPreProcBindingTable->dwAvcPreProcMvPredictor;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (preEncParams->bMBQp)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));

        uint32_t size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb;
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = &(preEncParams->resMbQpBuffer);
        surfaceCodecParams.dwOffset = 0;
        surfaceCodecParams.dwBindingTableOffset = avcPreProcBindingTable->dwAvcPreProcMbQp;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // 16 DWs per QP value
        size = 16 * 52 * sizeof(uint32_t);

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = params->presFtqLutBuffer;
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.dwBindingTableOffset = currFieldPicture ? avcPreProcBindingTable->dwAvcPreProcFtqLutField : avcPreProcBindingTable->dwAvcPreProcFtqLut;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

    }

    if (!preEncParams->bDisableMVOutput)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));

        uint32_t size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 32 * 4;
        surfaceCodecParams.presBuffer = &(preEncParams->resMvBuffer);
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.dwOffset = 0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcPreProcBindingTable->dwAvcPreProcMvDataOut;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (!preEncParams->bDisableStatisticsOutput)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));

        uint32_t size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 64;
        if (currBottomField)
        {
            surfaceCodecParams.presBuffer = &(preEncParams->resStatsBotFieldBuffer);
        }
        else
        {
            surfaceCodecParams.presBuffer = &(preEncParams->resStatsBuffer);
        }
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.dwOffset = params->dwMBVProcStatsBottomFieldOffset;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        surfaceCodecParams.dwBindingTableOffset = avcPreProcBindingTable->dwAvcPreProcMbStatsOut;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // Current Picture Y - VME
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bUseAdvState = true;
    surfaceCodecParams.psSurface = params->psCurrPicSurface;
    surfaceCodecParams.dwOffset = 0;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = currFieldPicture ? avcPreProcBindingTable->dwAvcPreProcVMECurrPicField[0] : avcPreProcBindingTable->dwAvcPreProcVMECurrPicFrame[0];
    surfaceCodecParams.ucVDirection = vDirection;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    surfaceCodecParams.dwBindingTableOffset = currFieldPicture ? avcPreProcBindingTable->dwAvcPreProcVMECurrPicField[1] : avcPreProcBindingTable->dwAvcPreProcVMECurrPicFrame[1];
    surfaceCodecParams.ucVDirection = vDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    uint32_t refBindingTableOffset;
    if (preEncParams->dwNumPastReferences > 0)
    {
        auto refPic = preEncParams->PastRefPicture;
        uint8_t refPicIdx = refPic.FrameIdx;
        bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
        uint8_t refVDirection;
        // Program the surface based on current picture's field/frame mode
        if (currFieldPicture) // if current picture is field
        {
            if (refBottomField)
            {
                refVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
            }
            else
            {
                refVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
            }
            refBindingTableOffset = avcPreProcBindingTable->dwAvcPreProcVMEFwdPicField[0];
        }
        else // if current picture is frame
        {
            refVDirection = CODECHAL_VDIRECTION_FRAME;
            refBindingTableOffset = avcPreProcBindingTable->dwAvcPreProcVMEFwdPicFrame;
        }

        // Picture Y VME
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bUseAdvState = true;
        surfaceCodecParams.psSurface = &params->ppRefList[refPicIdx]->sRefBuffer;
        surfaceCodecParams.dwBindingTableOffset = refBindingTableOffset;
        surfaceCodecParams.ucVDirection = refVDirection;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        if (currFieldPicture) // if current picture is field
        {
            surfaceCodecParams.dwBindingTableOffset = avcPreProcBindingTable->dwAvcPreProcVMEFwdPicField[1];

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

    if (preEncParams->dwNumFutureReferences > 0)
    {
        auto refPic = preEncParams->FutureRefPicture;
        uint8_t refPicIdx = refPic.FrameIdx;
        bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
        uint8_t refVDirection;

        // Program the surface based on current picture's field/frame mode
        if (currFieldPicture) // if current picture is field
        {
            if (refBottomField)
            {
                refVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
            }
            else
            {
                refVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
            }
            refBindingTableOffset = avcPreProcBindingTable->dwAvcPreProcVMEBwdPicField[0];
        }
        else // if current picture is frame
        {
            refVDirection = CODECHAL_VDIRECTION_FRAME;
            refBindingTableOffset = avcPreProcBindingTable->dwAvcPreProcVMEBwdPicFrame[0];
        }

        // Picture Y VME
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bUseAdvState = true;
        surfaceCodecParams.psSurface = &params->ppRefList[refPicIdx]->sRefBuffer;
        surfaceCodecParams.dwBindingTableOffset = refBindingTableOffset;
        surfaceCodecParams.ucVDirection = refVDirection;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        if (currFieldPicture) // if current picture is field
        {
            refBindingTableOffset = avcPreProcBindingTable->dwAvcPreProcVMEBwdPicField[1];
        }
        else
        {
            refBindingTableOffset = avcPreProcBindingTable->dwAvcPreProcVMEBwdPicFrame[1];
        }
        surfaceCodecParams.dwBindingTableOffset = refBindingTableOffset;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    return eStatus;
}

void CodechalEncodeAvcEncFeiG8::UpdateSSDSliceCount()
{
    CodechalEncodeAvcBase::UpdateSSDSliceCount();

    uint32_t sliceCount;
    if (m_frameHeight * m_frameWidth >= 960*540)
    {
        sliceCount = 2;
    }
    else
    {
        sliceCount = 1;
    }

    if (m_osInterface->pfnSetSliceCount)
    {
        m_osInterface->pfnSetSliceCount(m_osInterface, &sliceCount);
    }
}

