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
//! \file     codechal_fei_avc_g9.cpp
//! \brief    This file implements the C++ class/interface for Gen9 platform's AVC 
//!           FEI encoding to be used across CODECHAL components.
//!

#include "codechal_fei_avc_g9.h"
#include "codechal_fei_avc_g9_skl.h"
#include "codechal_encoder_g9.h"
#include "igcodeckrn_g9.h"
#include "codeckrnheader.h"

#ifdef FEI_ENABLE_CMRT
static const char *                 strDsIsaName        = "/opt/intel/mediasdk/lib64/hme_downscale_gen9.isa";
static const char *                 strPreProcIsaName   = "/opt/intel/mediasdk/lib64/FEI_gen9.isa";
static const char *                 strMeIsaName        = "/opt/intel/mediasdk/lib64/hme_gen9.isa";
#endif
static const char *                 strMbEncIsaName     = "/opt/intel/mediasdk/lib64/AVCEncKernel_SKL_genx.isa";

typedef enum _CODECHAL_BINDING_TABLE_OFFSET_2xSCALING_CM_G9
{
    CODECHAL_2xSCALING_FRAME_SRC_Y_CM_G9 = 0,
    CODECHAL_2xSCALING_FRAME_DST_Y_CM_G9 = 1,
    CODECHAL_2xSCALING_FIELD_TOP_SRC_Y_CM_G9 = 0,
    CODECHAL_2xSCALING_FIELD_TOP_DST_Y_CM_G9 = 1,
    CODECHAL_2xSCALING_FIELD_BOT_SRC_Y_CM_G9 = 2,
    CODECHAL_2xSCALING_FIELD_BOT_DST_Y_CM_G9 = 3,
    CODECHAL_2xSCALING_NUM_SURFACES_CM_G9 = 4
}CODECHAL_BINDING_TABLE_OFFSET_2xSCALING_CM_G9;

typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_PREPROC_CM_G9
{
    CODECHAL_ENCODE_AVC_PREPROC_CURR_Y_CM_G9 = 0,
    CODECHAL_ENCODE_AVC_PREPROC_CURR_UV_CM_G9 = 1,
    CODECHAL_ENCODE_AVC_PREPROC_HME_MV_DATA_CM_G9 = 2,
    CODECHAL_ENCODE_AVC_PREPROC_MV_PREDICTOR_CM_G9 = 3,
    CODECHAL_ENCODE_AVC_PREPROC_MBQP_CM_G9 = 4,
    CODECHAL_ENCODE_AVC_PREPROC_MV_DATA_CM_G9 = 5,
    CODECHAL_ENCODE_AVC_PREPROC_MB_STATS_CM_G9 = 6,
    CODECHAL_ENCODE_AVC_PREPROC_VME_CURR_PIC_IDX_0_CM_G9 = 7,
    CODECHAL_ENCODE_AVC_PREPROC_VME_FWD_PIC_IDX0_CM_G9 = 8,
    CODECHAL_ENCODE_AVC_PREPROC_VME_BWD_PIC_IDX0_0_CM_G9 = 9,
    CODECHAL_ENCODE_AVC_PREPROC_VME_CURR_PIC_IDX_1_CM_G9 = 10,
    CODECHAL_ENCODE_AVC_PREPROC_VME_BWD_PIC_IDX0_1_CM_G9 = 11,
    CODECHAL_ENCODE_AVC_PREPROC_RESERVED1_CM_G9 = 12,
    CODECHAL_ENCODE_AVC_PREPROC_FTQ_LUT_CM_G9 = 13,
    CODECHAL_ENCODE_AVC_PREPROC_NUM_SURFACES_CM_G9 = 14
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_PREPROC_CM_G9;

typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_PREPROC_FIELD_CM_G9
{
    CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_CURR_PIC_IDX_0_CM_G9 = 7,
    CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_FWD_PIC_IDX0_0_CM_G9 = 8,
    CODECHAL_ENCODE_AVC_PREPROC_FIELD_RESERVED0_CM_G9 = 9,
    CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_FWD_PIC_IDX0_1_CM_G9 = 10,
    CODECHAL_ENCODE_AVC_PREPROC_FIELD_RESERVED1_CM_G9 = 11,
    CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_CURR_PIC_IDX_1_CM_G9 = 12,
    CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_BWD_PIC_IDX0_0_CM_G9 = 13,
    CODECHAL_ENCODE_AVC_PREPROC_FIELD_RESERVED2_CM_G9 = 14,
    CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_BWD_PIC_IDX0_1_CM_G9 = 15,
    CODECHAL_ENCODE_AVC_PREPROC_FIELD_RESERVED3_CM_G9 = 16,
    CODECHAL_ENCODE_AVC_PREPROC_FIELD_FTQ_LUT_CM_G9 = 17,
    CODECHAL_ENCODE_AVC_PREPROC_FIELD_NUM_SURFACES_CM_G9 = 18
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_PREPROC_FIELD_CM_G9;

const uint32_t CodechalEncodeAvcEncFeiG9::HMEBCombineLen_fei[NUM_TARGET_USAGE_MODES] =
{
    0, 8, 8, 8, 8, 8, 8, 8
};

const uint32_t CodechalEncodeAvcEncFeiG9::HMECombineLen_fei[NUM_TARGET_USAGE_MODES] =
{
    0, 8, 8, 8, 8, 8, 16, 8
};

typedef struct _MEDIA_OBJECT_2xSCALING_STATIC_DATA_CM_FEI_G9
{
// DWORD 0 - GRF R1.0
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
} MEDIA_OBJECT_2xSCALING_STATIC_DATA_CM_FEI_G9, *PMEDIA_OBJECT_2xSCALING_STATIC_DATA_CM_FEI_G9;

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(MEDIA_OBJECT_2xSCALING_STATIC_DATA_CM_FEI_G9)) == 12);

typedef struct _CODECHAL_ENCODE_AVC_ME_CURBE_CM_FEI_G9
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

} CODECHAL_ENCODE_AVC_ME_CURBE_CM_FEI_G9, *PCODECHAL_ENCODE_AVC_ME_CURBE_CM_FEI_G9;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_ME_CURBE_CM_FEI_G9)) == 39);

const uint32_t CodechalEncodeAvcEncFeiG9::ModeMvCost_Cm_PreProc[3][CODEC_AVC_NUM_QP][8] =
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

const uint32_t CodechalEncodeAvcEncFeiG9::ME_CURBE_CM_FEI[39] =
{
    0x00000000, 0x00200010, 0x00003939, 0x77a43000, 0x00000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

// AVC PreProc CURBE init data for G9 CM Kernel
const uint32_t CodechalEncodeAvcEncFeiG9::PreProc_CURBE_CM_normal_I_frame[49] =
{
    0x00000082, 0x00000000, 0x00003910, 0x00a83000, 0x00000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00040c24, 0x00000000, 0x00000000, 0x40000000, 0x00000080, 0x00003900, 0x28301000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG9::PreProc_CURBE_CM_normal_I_field[49] =
{
    0x00000082, 0x00000000, 0x00003910, 0x00a830c0, 0x00000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00040c24, 0x00000000, 0x00000000, 0x40000000, 0x00000080, 0x00003900, 0x28301000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG9::PreProc_CURBE_CM_normal_P_frame[49] =
{
    0x000000a3, 0x00000008, 0x00003910, 0x00ae3000, 0x00000000, 0x28300000, 0x00000000, 0x01400060,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00040c24, 0x00000000, 0x00000000, 0x60000000, 0x000000a1, 0x00003900, 0x28301000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG9::PreProc_CURBE_CM_normal_P_field[49] =
{
    0x000000a3, 0x00000008, 0x00003910, 0x00ae30c0, 0x00000000, 0x28300000, 0x00000000, 0x01400060,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00040c24, 0x00000000, 0x00000000, 0x40000000, 0x000000a1, 0x00003900, 0x28301000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG9::PreProc_CURBE_CM_normal_B_frame[49] =
{
    0x000000a3, 0x00200008, 0x00003910, 0x00aa7700, 0x00000000, 0x20200000, 0x00000000, 0xff400000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00040c24, 0x00000000, 0x00000000, 0x60000000, 0x000000a1, 0x00003900, 0x28301000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG9::PreProc_CURBE_CM_normal_B_field[49] =
{
    0x000000a3, 0x00200008, 0x00003919, 0x00aa77c0, 0x00000000, 0x20200000, 0x00000000, 0xff400000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00040c24, 0x00000000, 0x00000000, 0x40000000, 0x000000a1, 0x00003900, 0x28301000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff
};

// AVC MBEnc RefCost tables, index [CodingType][QP]
// QP is from 0 - 51, pad it to 64 since BRC needs each subarray size to be 128bytes
const uint16_t CodechalEncodeAvcEncFeiG9::RefCost_MultiRefQp_Fei[NUM_PIC_TYPES][64] =
{
    // I-frame
    {
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000
    },
    // P-slice
    {
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000
    },
    //B-slice
    {
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000
    }
};

typedef struct _CODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9
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

} CODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9, *PCODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9;

typedef struct _CODECHAL_ENCODE_AVC_MBENC_DISPATCH_PARAMS
{
    CodechalEncodeMdfKernelResource*        pKernelRes;
    struct CodechalEncodeAvcSurfaceIdx      *avcMBEncSurface;
    uint32_t                                frameWidthInMBs;
    uint16_t                                wSliceHeight;
    uint32_t                                numSlices;
    uint16_t                                wSliceType; 
    uint32_t                                dwNumMBs;
    bool                                    EnableArbitrarySliceSize;
    uint8_t                                 EnableWavefrontOptimization; 
    uint8_t                                 Reserved0;
    uint8_t                                 Reserved1;
    uint8_t                                 Reserved2;
}CODECHAL_ENCODE_AVC_MBENC_DISPATCH_PARAMS, *PCODECHAL_ENCODE_AVC_MBENC_DISPATCH_PARAMS;

C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9)) == 49);

// AVC FEI MBEnc CURBE init data for G9 Kernel
const uint32_t CodechalEncodeAvcEncFeiG9::FEI_MBEnc_CURBE_normal_I_frame[104] =
{
    0x00000082, 0x00000000, 0x00003910, 0x00a83000, 0x00000000, 0x28300000, 0x05000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80800000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG9::FEI_MBEnc_CURBE_normal_I_field[104] =
{
    0x00000082, 0x00000000, 0x00003910, 0x00a830c0, 0x02000000, 0x28300000, 0x05000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80800000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG9::FEI_MBEnc_CURBE_normal_P_frame[104] =
{
    0x000000a3, 0x00000008, 0x00003910, 0x00ae3000, 0x30000000, 0x28300000, 0x05000000, 0x01400060,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x60000000, 0x000000a1, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x08000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG9::FEI_MBEnc_CURBE_normal_P_field[104] =
{
    0x000000a3, 0x00000008, 0x00003910, 0x00ae30c0, 0x30000000, 0x28300000, 0x05000000, 0x01400060,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x000000a1, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG9::FEI_MBEnc_CURBE_normal_B_frame[104] =
{
    0x000000a3, 0x00200008, 0x00003910, 0x00aa7700, 0x50020000, 0x20200000, 0x05000000, 0xff400000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x01010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x60000000, 0x000000a1, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x08000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

const uint32_t CodechalEncodeAvcEncFeiG9::FEI_MBEnc_CURBE_normal_B_field[104] =
{
    0x000000a3, 0x00200008, 0x00003919, 0x00aa77c0, 0x50020000, 0x20200000, 0x05000000, 0xff400000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x01010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x000000a1, 0x00003900, 0x28301000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

// FEI AVC I_DIST CURBE init data for G9 Kernel
const uint32_t CodechalEncodeAvcEncFeiG9::FEI_MBEnc_CURBE_I_frame_DIST[104] =
{
    0x00000082, 0x00200008, 0x001e3910, 0x00a83000, 0x90000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000100,
    0x80800000, 0x00000000, 0x00000800, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

CodechalEncodeAvcEncFeiG9::CodechalEncodeAvcEncFeiG9(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo) : CodechalEncodeAvcEncG9(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    
    m_cmKernelEnable = true;
    bHighTextureModeCostEnable = true;

#ifdef FEI_ENABLE_CMRT
#endif
    m_cmSurfIdx = MOS_New(struct CodechalEncodeAvcSurfaceIdx);

    this->pfnGetKernelHeaderAndSize = this->EncodeGetKernelHeaderAndSize;
    m_feiEnable = true;

    //FEI output Stats which is a superset of MbStats buffer, so no need for MbStats
    m_mbStatsSupported = false;
    m_kuid = IDR_CODEC_AllAVCEnc_FEI;
    m_kernelBase = (uint8_t *)IGCODECKRN_G9;
    AddIshSize(m_kuid, m_kernelBase);
}

CodechalEncodeAvcEncFeiG9::~CodechalEncodeAvcEncFeiG9()
{
#ifdef FEI_ENABLE_CMRT
    DestroyMDFKernelResource(&m_resPreProcKernel);
    DestroyMDFKernelResource(&m_resMeKernel);
    DestroyMDFKernelResource(&resDSKernel);
#endif
    DestroyMDFKernelResource(m_resMbencKernel);
    MOS_FreeMemory(m_resMbencKernel);

    if(nullptr != m_cmSurfIdx)
    {
        MOS_Delete(m_cmSurfIdx);
        m_cmSurfIdx = nullptr;
    }
    if(nullptr != m_vmeSurface)
    {
        delete [] m_vmeSurface;
    }
    if(nullptr != m_commonSurface)
    {
        delete [] m_commonSurface;
    }
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::InitializePicture(const EncoderParams& params)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    if (m_codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC)
    {
        return EncodePreEncInitialize(params);
    }
    else
    {
        return CodechalEncodeAvcEnc::InitializePicture(params);
    }
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::EncodePreEncInitialize(const EncoderParams& params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto pPreEncParams = (FeiPreEncParams*)params.pPreEncParams;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pPreEncParams->psCurrOriginalSurface);

    auto ppAvcRefList = &m_refList[0];
    auto pAvcPicIdx = &m_picIdx[0];

    CodecHal_GetResourceInfo(m_osInterface, pPreEncParams->psCurrOriginalSurface);
    m_rawSurface = *(pPreEncParams->psCurrOriginalSurface);
    m_rawSurfaceToEnc =
        m_rawSurfaceToPak = &m_rawSurface;
    m_targetUsage = TARGETUSAGE_RT_SPEED;
    m_kernelMode = CodecHal_TargetUsageToMode_AVC[m_targetUsage];
    m_flatnessCheckEnabled = m_flatnessCheckSupported;

    auto prevPic = m_currOriginalPic;
    uint8_t prevIdx = prevPic.FrameIdx;
    uint8_t currIdx = pPreEncParams->CurrOriginalPicture.FrameIdx;
    bool bFirstFieldInput = (prevPic.PicFlags == PICTURE_INVALID) && ((pPreEncParams->CurrOriginalPicture.PicFlags == PICTURE_TOP_FIELD) || (pPreEncParams->CurrOriginalPicture.PicFlags == PICTURE_BOTTOM_FIELD));

    ppAvcRefList[currIdx]->sRefBuffer = ppAvcRefList[currIdx]->sRefRawBuffer = m_rawSurface;
    ppAvcRefList[currIdx]->RefPic = m_currOriginalPic;
    ppAvcRefList[currIdx]->bUsedAsRef = true;

    // FEI PreEnc doesn't have CurrReconstructedPicture, here we set m_currReconstructedPic = pPreEncParams->CurrOriginalPicture
    // so it can resue the AVC functions.
    m_currOriginalPic = pPreEncParams->CurrOriginalPicture;
    m_currReconstructedPic = pPreEncParams->CurrOriginalPicture;
    m_currRefList = m_refList[m_currOriginalPic.FrameIdx];

    uint8_t ucNumRef = 0;
    for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        pAvcPicIdx[i].bValid = false;
    }

    // FEI only support one past and one future reference picture now
    uint8_t ucIndex;
    if (pPreEncParams->dwNumPastReferences > 0)
    {
        CODECHAL_ENCODE_ASSERT(pPreEncParams->dwNumPastReferences == 1);
        CODECHAL_ENCODE_ASSERT(!CodecHal_PictureIsInvalid(pPreEncParams->PastRefPicture));
        ucIndex = pPreEncParams->PastRefPicture.FrameIdx;
        pAvcPicIdx[ucNumRef].bValid = true;
        pAvcPicIdx[ucNumRef].ucPicIdx = ucIndex;
        ppAvcRefList[ucIndex]->RefPic.PicFlags =
            CodecHal_CombinePictureFlags(ppAvcRefList[ucIndex]->RefPic, pPreEncParams->PastRefPicture);
        ppAvcRefList[currIdx]->RefList[ucNumRef] = pPreEncParams->PastRefPicture;
        ucNumRef++;
    }

    if (pPreEncParams->dwNumFutureReferences > 0)
    {
        CODECHAL_ENCODE_ASSERT(pPreEncParams->dwNumFutureReferences == 1);
        CODECHAL_ENCODE_ASSERT(!CodecHal_PictureIsInvalid(pPreEncParams->FutureRefPicture))
            ucIndex = pPreEncParams->FutureRefPicture.FrameIdx;
        pAvcPicIdx[ucNumRef].bValid = true;
        pAvcPicIdx[ucNumRef].ucPicIdx = ucIndex;
        ppAvcRefList[ucIndex]->RefPic.PicFlags =
            CodecHal_CombinePictureFlags(ppAvcRefList[ucIndex]->RefPic, pPreEncParams->FutureRefPicture);
        ppAvcRefList[currIdx]->RefList[ucNumRef] = pPreEncParams->FutureRefPicture;
        ucNumRef++;
    }
    ppAvcRefList[currIdx]->ucNumRef = ucNumRef;

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
        if (CodecHal_PictureIsFrame(prevPic) || prevIdx != currIdx || bFirstFieldInput)
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
        dwMBVProcStatsBottomFieldOffset = 0;

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

    if ((pPreEncParams->bDisableMVOutput == 1) && (pPreEncParams->bDisableStatisticsOutput == 1))
    {
        m_hmeEnabled = false;
    }
    else
    {
        m_hmeEnabled = (pPreEncParams->dwNumPastReferences + pPreEncParams->dwNumFutureReferences) > 0 ? true : false;
    }
    m_scalingEnabled = m_firstField && pPreEncParams->bInputUpdated;
    m_useRawForRef = m_userFlags.bUseRawPicForRef;

    m_singleTaskPhaseSupported = true;
    m_scalingEnabled = (m_hmeSupported || bBrcEnabled) && m_firstField;

    // PREENC doesn't differentiate picture by its "PictureCodingType". However in order to reuse the CODECHAL function,
    // we manually assign the PictureCodingType given its past/future references.
    if (pPreEncParams->dwNumFutureReferences > 0)
    {
        m_pictureCodingType = B_TYPE;
    }
    else if (pPreEncParams->dwNumPastReferences > 0)
    {
        m_pictureCodingType = P_TYPE;
    }
    else
    {
        m_pictureCodingType = I_TYPE;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetStatusReportParams(m_refList[m_currOriginalPic.FrameIdx]));

    CODECHAL_DEBUG_TOOL(
        m_debugInterface->CurrPic = m_currOriginalPic;
        m_debugInterface->dwBufferDumpFrameNum = m_storeData;
        m_debugInterface->wFrameType = m_pictureCodingType;
    )

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::ExecuteKernelFunctions()
{
    if (m_codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC)
    {
        return EncodePreEncKernelFunctions();
    }
    else
    {
        if (m_mfeEnabled)
        {
            return EncodeMbEncKernelFunctions();
        }
        else
        {
            return CodechalEncodeAvcEncG9::ExecuteKernelFunctions();
        }
    }
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::EncodePreEncKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto pPreEncParams = (FeiPreEncParams*)m_encodeParams.pPreEncParams;
    auto ppAvcRefList = &m_refList[0];

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

    bool dsSurfaceInCache;
    uint8_t ucScaledIdx = m_currScalingIdx = PreencLookUpBufIndex(this, m_currOriginalPic.FrameIdx, true, &dsSurfaceInCache);

    bool dsPastRefInCache = false;
    bool callDsPastRef = false;
    uint8_t pastRefScaledIdx = 0;
    if (pPreEncParams->dwNumPastReferences > 0)
    {
        pastRefScaledIdx = PreencLookUpBufIndex(this, pPreEncParams->PastRefPicture.FrameIdx, false, &dsPastRefInCache);
        if ((!pPreEncParams->bPastRefUpdated) && dsPastRefInCache)
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
    if (pPreEncParams->dwNumFutureReferences > 0)
    {
        futureRefScaledIdx = PreencLookUpBufIndex(this, pPreEncParams->FutureRefPicture.FrameIdx, false, &dsFutureRefInCache);
        if ((!pPreEncParams->bFutureRefUpdated) && dsFutureRefInCache)
        {
            CODECHAL_ENCODE_VERBOSEMESSAGE("find FutureRef Downscaled Surface in cache, so skip DS");
        }
        else
        {
            callDsFutureRef = true;
        }
    }

    bool callPreEncKernel = (pPreEncParams->bDisableMVOutput == 0) || (pPreEncParams->bDisableStatisticsOutput == 0);

    CodechalEncodeCscDs::KernelParams CscScalingKernelParams;
#ifdef FEI_ENABLE_CMRT
    m_dsIdx = 0;
#endif    
    if ((!pPreEncParams->bCurPicUpdated) && dsSurfaceInCache)
    {
        CODECHAL_ENCODE_VERBOSEMESSAGE("find Downscaled Surface in cache, so skip DS");
    }
    else
    {
        if (!pPreEncParams->bCurPicUpdated)
        {
            CODECHAL_ENCODE_VERBOSEMESSAGE("Cannot find matched Downscaled Surface, DS again");
        }
        if (m_currScalingIdx == CODEC_NUM_TRACKED_BUFFERS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Cannot find empty DS slot for preenc.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        m_firstField = pPreEncParams->bCurPicUpdated ? 1 : m_firstField;
        m_currRefList->ucScalingIdx = m_currScalingIdx;
        m_currRefList->b4xScalingUsed = false;
        m_currRefList->b16xScalingUsed = false;
        m_currRefList->b32xScalingUsed = false;

        MOS_ZeroMemory(&CscScalingKernelParams, sizeof(CscScalingKernelParams));
        CscScalingKernelParams.bLastTaskInPhase4xDS = !(callDsPastRef || callDsFutureRef || m_hmeEnabled || callPreEncKernel);
        CscScalingKernelParams.b32xScalingInUse = false;
        CscScalingKernelParams.b16xScalingInUse = false;
#ifdef FEI_ENABLE_CMRT
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeScalingKernel(&CscScalingKernelParams));
        m_dsIdx ++;
#else
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->DsKernel(&CscScalingKernelParams));
#endif

    }

    // Scaling for Past ref 
    if (callDsPastRef)
    {
        m_currScalingIdx = pastRefScaledIdx;

        if (!pPreEncParams->bPastRefUpdated)
        {
            CODECHAL_ENCODE_VERBOSEMESSAGE("Cannot find matched Downscaled Surface, DS again");
        }
        if (m_currScalingIdx == CODEC_NUM_TRACKED_BUFFERS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Cannot find empty DS slot for preenc.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        uint8_t pastRefIdx = pPreEncParams->PastRefPicture.FrameIdx;
        ppAvcRefList[pastRefIdx]->sRefBuffer = ppAvcRefList[pastRefIdx]->sRefRawBuffer = pPreEncParams->sPastRefSurface;
        ppAvcRefList[pastRefIdx]->RefPic = pPreEncParams->PastRefPicture;
        ppAvcRefList[pastRefIdx]->bUsedAsRef = true;
        m_firstField = true;
        m_currRefList = ppAvcRefList[pastRefIdx];
        m_currRefList->ucScalingIdx = m_currScalingIdx;
        m_currRefList->b4xScalingUsed = false;
        m_currRefList->b16xScalingUsed = false;
        m_currRefList->b32xScalingUsed = false;

        MOS_ZeroMemory(&CscScalingKernelParams, sizeof(CscScalingKernelParams));
        CscScalingKernelParams.bLastTaskInPhase4xDS = !(callDsFutureRef || m_hmeEnabled || callPreEncKernel);
        CscScalingKernelParams.b32xScalingInUse = false;
        CscScalingKernelParams.b16xScalingInUse = false;
        CscScalingKernelParams.bRawInputProvided = true;
        CscScalingKernelParams.bScalingforRef = true;
        CscScalingKernelParams.sInputRawSurface = pPreEncParams->sPastRefSurface;
        CscScalingKernelParams.inputPicture = pPreEncParams->PastRefPicture;

        if (pPreEncParams->bPastRefStatsNeeded)
        {
            CscScalingKernelParams.sInputStatsBuffer = pPreEncParams->sPastRefStatsBuffer;
            if (CodecHal_PictureIsField(m_currOriginalPic))
            {
                CscScalingKernelParams.sInputStatsBotFieldBuffer = pPreEncParams->sPastRefStatsBotFieldBuffer;
            }
            CscScalingKernelParams.bStatsInputProvided = true;
        }
#ifdef FEI_ENABLE_CMRT
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeScalingKernel(&CscScalingKernelParams));
        m_dsIdx++;
#else
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->DsKernel(&CscScalingKernelParams));
#endif
    }

    // Scaling for Future ref 
    if (callDsFutureRef)
    {
        m_currScalingIdx = futureRefScaledIdx;

        if (!pPreEncParams->bFutureRefUpdated)
        {
            CODECHAL_ENCODE_VERBOSEMESSAGE("Cannot find matched Downscaled Surface, DS again");
        }
        if (m_currScalingIdx == CODEC_NUM_TRACKED_BUFFERS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Cannot find empty DS slot for preenc.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        uint8_t futureRefIdx = pPreEncParams->FutureRefPicture.FrameIdx;
        ppAvcRefList[futureRefIdx]->sRefBuffer = ppAvcRefList[futureRefIdx]->sRefRawBuffer = pPreEncParams->sFutureRefSurface;
        ppAvcRefList[futureRefIdx]->RefPic = pPreEncParams->FutureRefPicture;
        ppAvcRefList[futureRefIdx]->bUsedAsRef = true;
        m_firstField = true;
        m_currRefList = ppAvcRefList[futureRefIdx];
        m_currRefList->ucScalingIdx = m_currScalingIdx;
        m_currRefList->b4xScalingUsed = false;
        m_currRefList->b16xScalingUsed = false;
        m_currRefList->b32xScalingUsed = false;

        MOS_ZeroMemory(&CscScalingKernelParams, sizeof(CscScalingKernelParams));
        CscScalingKernelParams.bLastTaskInPhase4xDS = !(m_hmeEnabled || callPreEncKernel);
        CscScalingKernelParams.b32xScalingInUse = false;
        CscScalingKernelParams.b16xScalingInUse = false;
        CscScalingKernelParams.bRawInputProvided = true;
        CscScalingKernelParams.bScalingforRef = true;
        CscScalingKernelParams.sInputRawSurface = pPreEncParams->sFutureRefSurface;
        CscScalingKernelParams.inputPicture = pPreEncParams->FutureRefPicture;

        if (pPreEncParams->bFutureRefStatsNeeded)
        {
            CscScalingKernelParams.sInputStatsBuffer = pPreEncParams->sFutureRefStatsBuffer;
            if (CodecHal_PictureIsField(m_currOriginalPic))
            {
                CscScalingKernelParams.sInputStatsBotFieldBuffer = pPreEncParams->sFutureRefStatsBotFieldBuffer;
            }
            CscScalingKernelParams.bStatsInputProvided = true;
        }
#ifdef FEI_ENABLE_CMRT
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeScalingKernel(&CscScalingKernelParams));
        m_dsIdx++;
#else
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->DsKernel(&CscScalingKernelParams));
#endif
    }

    m_currScalingIdx = ucScaledIdx;
    m_firstField = firstField;
    m_currRefList = currRefList;

    if (m_hmeEnabled)
    {
        CODEC_AVC_ENCODE_SLICE_PARAMS     AvcSliceParams;

        memset(&AvcSliceParams, 0, sizeof(CODEC_AVC_ENCODE_SLICE_PARAMS));
        if (pPreEncParams->dwNumPastReferences > 0)
        {
            AvcSliceParams.num_ref_idx_l0_active_minus1 = 0;
            AvcSliceParams.RefPicList[LIST_0][0] = pPreEncParams->PastRefPicture;
            AvcSliceParams.RefPicList[LIST_0][0].FrameIdx = 0;
        }
        else
        {
            AvcSliceParams.RefPicList[LIST_0][0].PicFlags = PICTURE_INVALID;
        }

        if (pPreEncParams->dwNumFutureReferences > 0)
        {
            AvcSliceParams.num_ref_idx_l1_active_minus1 = 0;
            AvcSliceParams.RefPicList[LIST_1][0] = pPreEncParams->FutureRefPicture;
            AvcSliceParams.RefPicList[LIST_1][0].FrameIdx = (pPreEncParams->dwNumPastReferences > 0) ? 1 : 0;
        }
        else
        {
            AvcSliceParams.RefPicList[LIST_1][0].PicFlags = PICTURE_INVALID;
        }

        m_avcSliceParams = &AvcSliceParams;
        m_lastTaskInPhase = !callPreEncKernel;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GenericEncodeMeKernel(&BrcBuffers, HME_LEVEL_4x));

    }

    m_lastTaskInPhase = true;
    if (callPreEncKernel)
    {
        // Execute the PreEnc kernel only when MV and/or Statistics output required. 
        // Especially for I frame case, if user disables the statistics output, the PreEnc can be skipped
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PreProcKernel());

        if(pPreEncParams->bMBQp){
            CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                     &pPreEncParams->resMbQpBuffer,
                     CodechalDbgAttr::attrInput,
                     "MbQp",
                      m_picWidthInMb * m_frameFieldHeightInMb,
                     0,
                     CODECHAL_MEDIA_STATE_PREPROC)));
        }
        if(pPreEncParams->dwMVPredictorCtrl){
            CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                     &pPreEncParams->resMvPredBuffer,
                     CodechalDbgAttr::attrInput,
                     "MvPredictor",
                     m_picWidthInMb * m_frameFieldHeightInMb * 8,
                     0,
                     CODECHAL_MEDIA_STATE_PREPROC)));
        }
    }
#ifdef FEI_ENABLE_CMRT    
    CodecHalEncode_FreeMDFKernelSurfaces(this, &m_resPreProcKernel);
    CodecHalEncode_FreeMDFKernelSurfaces(this, &m_resMeKernel);
    CodecHalEncode_FreeMDFKernelSurfaces(this, &resDSKernel);
#endif
    // Reset buffer ID used for MbEnc kernel performance reports
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    m_setRequestedEUSlices = false;

    return eStatus;
}

#ifdef FEI_ENABLE_CMRT
MOS_STATUS CodechalEncodeAvcEncFeiG9::DispatchKernelMe(SurfaceIndex** ppSurfIndex,uint16_t wWidth,uint16_t wHeight,bool isBFrame)
{
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmDev);

    auto pKernelRes = &m_resMeKernel;
    CmKernel * pKernel;
    if (isBFrame)
    {
       pKernel = pKernelRes->ppKernel[1];
    }
    else
    {
       pKernel = pKernelRes->ppKernel[0];
    }
    uint32_t dwKernelArg = 0;
    //curbe data       
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, m_meCurbeDataSizeFei, pKernelRes->pCurbe));
    // output surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, sizeof(SurfaceIndex), ppSurfIndex[0]));
    // input MV surface. if not provided, set to output surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, sizeof(SurfaceIndex), ppSurfIndex[1]));
    // dist surfaces
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, sizeof(SurfaceIndex), ppSurfIndex[2]));
    // dist 
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, sizeof(SurfaceIndex), ppSurfIndex[3]));
    // fwd ref surfaces. if not provided, set to output surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, sizeof(SurfaceIndex), ppSurfIndex[4]));
    // bwd ref surfaces. if not provided, set to output surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, sizeof(SurfaceIndex), ppSurfIndex[5]));
    //only difference between G9 & G8
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, sizeof(SurfaceIndex), ppSurfIndex[0]));
    //identify field or frame from curbe
    bool isField = (pKernelRes->pCurbe[12] >> 7) & 1;
    // config thread space
    uint32_t           dwThreadWidth   = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(wWidth);
    uint32_t           dwThreadHeight  = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(wHeight);
    if (isField)
    {
        dwThreadHeight = (wHeight + 31) >> 5;
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetThreadCount(dwThreadWidth * dwThreadHeight));
    if(pKernelRes->pTS == nullptr)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateThreadSpace(dwThreadWidth, dwThreadHeight, pKernelRes->pTS));
    }

    bool            bIsEnqueue = false;
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        bIsEnqueue = true;
        m_lastTaskInPhase = false;
    }
    AddKernelMdf(pCmDev,pCmQueue,pKernel,pCmTask, pKernelRes->pTS,pKernelRes->e,bIsEnqueue);
    return eStatus;    
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::GenericEncodeMeKernel(EncodeBrcBuffers* pBrcBuffers, HmeLevel hmeLevel)
{
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    
    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmDev);
    
    auto pKernelRes             = &m_resMeKernel;
    auto ppCmSurf               = pKernelRes->ppCmSurf;
    auto ppCmVmeSurfIdx         = pKernelRes->ppCmVmeSurf;
        
    // Setup AVC Curbe
    MeCurbeParams    MeParams;    
    MOS_ZeroMemory(&MeParams, sizeof(MeParams));
    MeParams.hmeLvl = hmeLevel;
    MeParams.pCurbeBinary = pKernelRes->pCurbe;

    SetCurbeMe(&MeParams);
    SurfaceIndex *cmSurfIdx[6];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&m_4xMeMvDataBuffer.OsResource, ppCmSurf[0]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[0]->GetIndex(cmSurfIdx[0]));
    cmSurfIdx[1] = cmSurfIdx[0];
    cmSurfIdx[3] = (SurfaceIndex *)CM_NULL_SURFACE;
    
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&m_4xMeDistortionBuffer.OsResource, ppCmSurf[2]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[2]->GetIndex(cmSurfIdx[2]));
    
    //maxium reference L0 is 4
    uint32_t iBaseIdx = 4;//FWD_BASE_INDEX;
    //for FEI, m_currReconstructedPic = m_currOriginalPic
    //ucScaledIdx = m_refList[m_currReconstructedPic.FrameIdx]->ucScalingIdx;
    uint8_t ucScaledIdx = m_refList[m_currOriginalPic.FrameIdx]->ucScalingIdx;
    PMOS_SURFACE psCurrScaledSurface = &m_trackedBuffer[ucScaledIdx].sScaled4xSurface;

    SurfaceIndex                *cmTmpSurfIdx[3];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&psCurrScaledSurface->OsResource, ppCmSurf[iBaseIdx]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[iBaseIdx]->GetIndex(cmTmpSurfIdx[0])); 
    
    uint32_t num_ref = 0;
    CmSurface2D             * surfArray[8];
    for(uint8_t ucRefIdx = 0; ucRefIdx <= m_avcSliceParams->num_ref_idx_l0_active_minus1;ucRefIdx ++)
    {
        CODEC_PICTURE RefPic =  m_avcSliceParams->RefPicList[LIST_0][ucRefIdx];
        if(!CodecHal_PictureIsInvalid(RefPic) && m_picIdx[RefPic.FrameIdx].bValid)
        {
            uint8_t ucRefPicIdx = m_picIdx[RefPic.FrameIdx].ucPicIdx;
            uint8_t ucScaledIdx = m_refList[ucRefPicIdx]->ucScalingIdx;
                    
            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&m_trackedBuffer[ucScaledIdx].sScaled4xSurface.OsResource, ppCmSurf[iBaseIdx + 1 + ucRefIdx]));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[iBaseIdx + 1 + ucRefIdx]->GetIndex(cmTmpSurfIdx[1])); 
         
            surfArray[num_ref] = (CmSurface2D*)ppCmSurf[iBaseIdx + 1 + ucRefIdx];
            num_ref ++;
        }
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateVmeSurfaceG7_5(ppCmSurf[iBaseIdx], surfArray, nullptr, num_ref, 0, ppCmVmeSurfIdx[0]));
             
    iBaseIdx = 9; 
    num_ref = 0;
    auto pCmSurfForVME = ppCmSurf[4];
    //maxium L1 size is 2
    for(uint8_t ucRefIdx = 0; ucRefIdx <= m_avcSliceParams->num_ref_idx_l1_active_minus1;ucRefIdx ++)
    {
        CODEC_PICTURE RefPic =  m_avcSliceParams->RefPicList[LIST_1][ucRefIdx];
        if(!CodecHal_PictureIsInvalid(RefPic) && m_picIdx[RefPic.FrameIdx].bValid)
        {
            uint8_t ucRefPicIdx = m_picIdx[RefPic.FrameIdx].ucPicIdx;
            uint8_t ucScaledIdx = m_refList[ucRefPicIdx]->ucScalingIdx;
                    
            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&m_trackedBuffer[ucScaledIdx].sScaled4xSurface.OsResource, ppCmSurf[iBaseIdx + 1 + ucRefIdx]));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[iBaseIdx + 1 + ucRefIdx]->GetIndex(cmTmpSurfIdx[2])); 
                
            surfArray[num_ref] = (CmSurface2D*)ppCmSurf[iBaseIdx + 1 + ucRefIdx];
            num_ref ++;
        }
     }
     CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateVmeSurfaceG7_5( pCmSurfForVME, surfArray, nullptr, num_ref, 0, ppCmVmeSurfIdx[1]));
     cmSurfIdx[4] = ppCmVmeSurfIdx[0];
     cmSurfIdx[5] = ppCmVmeSurfIdx[1];

     bool isB = (m_pictureCodingType == B_TYPE);
   
     CODECHAL_ENCODE_CHK_STATUS_RETURN(DispatchKernelMe(
                                           &cmSurfIdx[0],
                                           (uint16_t)(m_frameWidth / 4), //CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth / 4),
                                           (uint16_t)(m_frameHeight/4),//CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / 4),
                                           isB));
     return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::InitKernelStateMe()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    
    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto pKernelRes = &m_resMeKernel;
    
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncode_CreateMDFKernelResource(this, pKernelRes, 2, m_mdfMeBufSize, m_mdfMeSurfSize, m_mdfMeVmeSurfSize, m_meCurbeDataSizeFei));
    uint32_t codeSize;    
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_ReadFileToPtr(strMeIsaName, (uint32_t*)&codeSize, &pKernelRes->pCommonISA));
 
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->LoadProgram(pKernelRes->pCommonISA, codeSize, pKernelRes->pCmProgram, "-nojitter")); 
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateKernel(pKernelRes->pCmProgram, "HME_P", pKernelRes->ppKernel[0])); 
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateKernel(pKernelRes->pCmProgram, "HME_B", pKernelRes->ppKernel[1])); 

    return eStatus;    
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::InitKernelStateScaling(PCODECHAL_ENCODER pAvcEncoder)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pAvcEncoder->pCmDev);
    
    auto pKernelRes  = &pAvcEncoder->resDSKernel;
    CodecHalEncode_CreateMDFKernelResource(this, pKernelRes, 6, m_mdfDsBufSize * 3, m_mdfDsSurfSize * 3, m_mdfDsVmeSurfSize, 0);

    uint32_t codeSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_ReadFileToPtr(strDsIsaName, (uint32_t*)&codeSize, &pKernelRes->pCommonISA));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pAvcEncoder->pCmDev->LoadProgram(pKernelRes->pCommonISA, codeSize, pKernelRes->pCmProgram, "-nojitter")); 
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pAvcEncoder->pCmDev->CreateKernel(pKernelRes->pCmProgram, "hme_frame_downscale", pKernelRes->ppKernel[0])); 
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pAvcEncoder->pCmDev->CreateKernel(pKernelRes->pCmProgram, "hme_frame_downscale", pKernelRes->ppKernel[1])); 
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pAvcEncoder->pCmDev->CreateKernel(pKernelRes->pCmProgram, "hme_frame_downscale", pKernelRes->ppKernel[2])); 
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pAvcEncoder->pCmDev->CreateKernel(pKernelRes->pCmProgram, "hme_field_downscale", pKernelRes->ppKernel[3])); 
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pAvcEncoder->pCmDev->CreateKernel(pKernelRes->pCmProgram, "hme_field_downscale", pKernelRes->ppKernel[4])); 
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pAvcEncoder->pCmDev->CreateKernel(pKernelRes->pCmProgram, "hme_field_downscale", pKernelRes->ppKernel[5]));   
    pKernelRes->e = CM_NO_EVENT;        
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::DispatchKernelScaling(uint32_t dwFlatnessThreshold,uint32_t dwOptions,uint16_t  wSource_Width,uint16_t  wSource_Height,uint32_t dwKernel_Type,SurfaceIndex** ppSurfIdx)
{

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmDev);
    
    uint16_t  Source_Field_Height         = (wSource_Height + 1) >> 1;
    uint16_t  DS4X_Width                  = ((wSource_Width + 32) >> 6) << 4;
    uint16_t  DS4X_Height                 = ((wSource_Height + 32) >> 6) << 4;
    uint16_t  DS4X_Field_Width            = DS4X_Width;
    uint16_t  DS4X_Field_Height           = ((Source_Field_Height + 32) >> 6) << 4;

    //if the width or height is less than 48 which is the search window size, ceil it to 48
    if (DS4X_Width<48)
    {
        DS4X_Width = 48;
        DS4X_Field_Width = 48;
    }
    if (DS4X_Height<48)
    {
        DS4X_Height = 48;
        DS4X_Field_Height = 24;
    }

    int  ThreadSpace_Width = (DS4X_Width + 4)>>3;           // Each 8x8 pixel output is completed by 1 thread
    int  ThreadSpace_Height = (DS4X_Height + 4) >> 3;       // Each 8x8 pixel output is completed by 1 thread
    if (dwKernel_Type == 1){
        ThreadSpace_Width = (DS4X_Field_Width + 4) >> 3;    // Each 4x8 pixel output is completed by 1 thread
        ThreadSpace_Height = (DS4X_Field_Height + 2) >> 2;  // Each 4x8 pixel output is completed by 1 thread
    }

    uint32_t reserved[3];
    reserved[0] = 0;
    reserved[1] = 0;
    reserved[2] = 0;
    auto pKernelRes = &resDSKernel;

    SurfaceIndex* pSurfaceIndex_SrcSurf_Top = nullptr;
    SurfaceIndex* pSurfaceIndex_SrcSurf_Bot = nullptr;
    SurfaceIndex* pSurfaceIndex_DSSurf_Top  = nullptr;
    SurfaceIndex* pSurfaceIndex_DSSurf_Bot  = nullptr;
    SurfaceIndex* pSurfaceIndex_MB_VProc_Stats_Top = nullptr;
    SurfaceIndex* pSurfaceIndex_MB_VProc_Stats_Bot = nullptr;  

    CmKernel* pKernel;
    if (dwKernel_Type == 0) 
    {
        pKernel = pKernelRes->ppKernel[m_dsIdx];
        pSurfaceIndex_SrcSurf_Top = ppSurfIdx[0];
        pSurfaceIndex_DSSurf_Top = ppSurfIdx[1];
        pSurfaceIndex_MB_VProc_Stats_Top = ppSurfIdx[4];
        pSurfaceIndex_MB_VProc_Stats_Bot = ppSurfIdx[4];
        
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(4, sizeof(reserved[0]), &reserved[0]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(5, sizeof(reserved[1]), &reserved[1]));
    }
    else
    {
        pKernel = pKernelRes->ppKernel[m_dsIdx + 3];
        pSurfaceIndex_SrcSurf_Top = ppSurfIdx[0];
        pSurfaceIndex_DSSurf_Top  = ppSurfIdx[1];
        pSurfaceIndex_SrcSurf_Bot = ppSurfIdx[2];
        pSurfaceIndex_DSSurf_Bot  = ppSurfIdx[3];

        pSurfaceIndex_MB_VProc_Stats_Top = ppSurfIdx[4];
        pSurfaceIndex_MB_VProc_Stats_Bot = ppSurfIdx[5];
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(4, sizeof(SurfaceIndex), pSurfaceIndex_SrcSurf_Bot)); // DW3
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(5, sizeof(SurfaceIndex), pSurfaceIndex_DSSurf_Bot)); // DW4
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(10, sizeof(SurfaceIndex), pSurfaceIndex_MB_VProc_Stats_Bot)); // DW9
    }        
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(0, sizeof(uint16_t), &wSource_Width));      // DW0
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(1, sizeof(uint16_t), &wSource_Height));     // DW0
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(2, sizeof(SurfaceIndex), pSurfaceIndex_SrcSurf_Top)); // DW1
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(3, sizeof(SurfaceIndex), pSurfaceIndex_DSSurf_Top)); // DW2
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(6, sizeof(uint32_t), &dwFlatnessThreshold));    // DW5
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(7, sizeof(uint32_t), &dwOptions));                   // DW6
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(8, sizeof(uint32_t), &reserved[2]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(9, sizeof(SurfaceIndex), pSurfaceIndex_MB_VProc_Stats_Top));// DW8
    
    // Setup Dispatch Pattern ====================================================== 
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetThreadCount(ThreadSpace_Width*ThreadSpace_Height));
    bool            bIsEnqueue                  = false;
    
    if(pKernelRes->pTS == nullptr)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateThreadSpace(ThreadSpace_Width, ThreadSpace_Height, pKernelRes->pTS));
    }
    if((!m_singleTaskPhaseSupported) || m_lastTaskInPhase)
    {
        bIsEnqueue = true;
        m_lastTaskInPhase = false;
    }
    return AddKernelMdf(pCmDev,pCmQueue,pKernel,pCmTask,pKernelRes->pTS,pKernelRes->e,bIsEnqueue);

}

MOS_STATUS CodechalEncodeAvcEncFeiG9::EncodeScalingKernel(CodechalEncodeCscDs::KernelParams* pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    
    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto pKernelRes = &resDSKernel;
    auto ppCmSurf    = &pKernelRes->ppCmSurf[m_dsIdx * m_mdfDsBufSize];
    auto ppCmBuf     = &pKernelRes->ppCmBuf[m_dsIdx * m_mdfDsBufSize];
    
    m_lastTaskInPhase  = pParams->bLastTaskInPhase4xDS;

    // setup kernel required parameters
    if (!m_firstField)
    {
        // Both fields are scaled when the first field comes in, no need to scale again
        return eStatus;
    }
    
    auto pPreEncParams = (FeiPreEncParams*)m_encodeParams.pPreEncParams;
    CODECHAL_ENCODE_CHK_NULL_RETURN(pPreEncParams);
    bool bFieldPicture = CodecHal_PictureIsField(m_currOriginalPic);
    auto pTrackedBuffer = &m_trackedBuffer[m_currScalingIdx];
    
    //only 4x 
    uint16_t wSrcWidth  = (uint16_t)m_oriFrameWidth;
    uint16_t wSrcHeight = (uint16_t)m_oriFrameHeight;
    auto psInputSurface  = (pParams->bRawInputProvided)? &pParams->sInputRawSurface : m_rawSurfaceToEnc;
    auto psOutputSurface = &pTrackedBuffer->sScaled4xSurface;
    m_currRefList->b4xScalingUsed = true;

    uint32_t dwEnableFlatness   = 0;
    uint32_t dwEnableVarianceOutput = 0;
    uint32_t dwEnableAverageOutput = 0;
    uint32_t dwEnable8x8Stats = 0;
    uint32_t dwFlatnessTh  = 0;
    
    if((!pParams->bScalingforRef) || (pParams->bStatsInputProvided))
    {
        dwEnableFlatness       = (!pParams->b32xScalingInUse && !pParams->b16xScalingInUse) ? m_flatnessCheckEnabled : 0;
        dwEnableVarianceOutput = (pPreEncParams) ? !pPreEncParams->bDisableStatisticsOutput : ((!pParams->b32xScalingInUse && !pParams->b16xScalingInUse) ? m_mbStatsEnabled : 0);
        dwEnableAverageOutput  = (pPreEncParams) ? !pPreEncParams->bDisableStatisticsOutput : ((!pParams->b32xScalingInUse && !pParams->b16xScalingInUse) ? m_mbStatsEnabled : 0);
        dwFlatnessTh           = 128;  
        dwEnable8x8Stats       = (pPreEncParams) ? pPreEncParams->bEnable8x8Statistics : 0; 
    }
    
    SurfaceIndex                            *cmSurfIdx[6];
    if(!bFieldPicture)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&psInputSurface->OsResource, ppCmSurf[0]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[0]->GetIndex(cmSurfIdx[0]));
        
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&psOutputSurface->OsResource, ppCmSurf[1]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[1]->GetIndex(cmSurfIdx[1]));
    }
    else
    {
        // src top
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&psInputSurface->OsResource, ppCmSurf[0]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[0]->GetIndex(cmSurfIdx[0])); 
        ppCmSurf[0]->SetProperty(CM_TOP_FIELD);
        // dst top
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&psOutputSurface->OsResource, ppCmSurf[1]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[1]->GetIndex(cmSurfIdx[1]));
        ppCmSurf[1]->SetProperty(CM_TOP_FIELD);

        // src bottom
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&psInputSurface->OsResource, ppCmSurf[2]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[2]->GetIndex(cmSurfIdx[2])); 
        ppCmSurf[2]->SetProperty(CM_BOTTOM_FIELD);
        
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&psOutputSurface->OsResource, ppCmSurf[3]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[3]->GetIndex(cmSurfIdx[3]));
        ppCmSurf[3]->SetProperty(CM_BOTTOM_FIELD);

        if((pParams->bScalingforRef) && (pParams->bStatsInputProvided))
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&pParams->sInputStatsBotFieldBuffer, ppCmBuf[1]));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[1]->GetIndex(cmSurfIdx[5]));
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&pPreEncParams->resStatsBotFieldBuffer, ppCmBuf[1]));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[1]->GetIndex(cmSurfIdx[5]));
        }
    }
    if((pParams->bScalingforRef)&&(pParams->bStatsInputProvided))
    {
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&pParams->sInputStatsBuffer, ppCmBuf[0]));
        }
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&pPreEncParams->resStatsBuffer, ppCmBuf[0]));
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[0]->GetIndex(cmSurfIdx[4]));

    uint32_t dwCurbe_R1_5    = dwFlatnessTh;
    uint32_t dwCurbe_R1_6    = dwEnableFlatness | (dwEnableVarianceOutput << 1) | (dwEnableAverageOutput << 2) | (dwEnable8x8Stats << 3);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(DispatchKernelScaling(dwCurbe_R1_5, dwCurbe_R1_6,
                                         wSrcWidth, wSrcHeight, bFieldPicture, cmSurfIdx));
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::InitKernelStatePreProc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    
    CODECHAL_ENCODE_FUNCTION_ENTER;
    
    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmDev);
    
    auto pKernelRes = &m_resPreProcKernel;
    CodecHalEncode_CreateMDFKernelResource(this, pKernelRes, 1, m_mdfPreProcBufSize, m_mdfPreProcSurfSize, m_mdfPreProcVmeSurfSize,m_preProcCurbeDataSizeFei); 
    uint32_t codeSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_ReadFileToPtr(strPreProcIsaName, (uint32_t*)&codeSize, &pKernelRes->pCommonISA));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->LoadProgram(pKernelRes->pCommonISA, codeSize, pKernelRes->pCmProgram, "-nojitter"));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateKernel(pKernelRes->pCmProgram, "FEI_PreEnc", pKernelRes->ppKernel[0])); 

    return eStatus;     
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::DispatchKernelPreProc(
    SurfaceIndex** ppSurfIndex,
    uint16_t wWidth,
    uint16_t wHeight)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    
    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto pKernelRes = &m_resPreProcKernel;
    auto pKernel = pKernelRes->ppKernel[0];
    // config thread space
    uint32_t threadswidth  = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(wWidth);
    uint32_t threadsheight = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(wHeight + 15);
    uint32_t dwKernelArg = 0;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, pKernelRes->wCurbeSize, pKernelRes->pCurbe));
    // Current Input Picture surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, sizeof(SurfaceIndex), ppSurfIndex[0]));
    // HME MV Input Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, sizeof(SurfaceIndex), ppSurfIndex[1]));
    // App Provided Prediction surfaces
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, sizeof(SurfaceIndex), ppSurfIndex[2]));
    // Qp Per MB Input Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, sizeof(SurfaceIndex), ppSurfIndex[3]));
    // MV Data Output Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, sizeof(SurfaceIndex), ppSurfIndex[4]));
    // MB VProc Stats Buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, sizeof(SurfaceIndex), ppSurfIndex[5]));
    // fwd ref surfaces. if not provided, set to output surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, sizeof(SurfaceIndex), ppSurfIndex[6]));
    // bwd ref surfaces. if not provided, set to output surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, sizeof(SurfaceIndex), ppSurfIndex[7]));
    // Qp and FTQ LUT
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(dwKernelArg++, sizeof(SurfaceIndex), ppSurfIndex[8]));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetThreadCount(threadswidth * threadsheight));

    if(nullptr == pKernelRes->pTS)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateThreadSpace(threadswidth, threadsheight, pKernelRes->pTS));
    }
    bool  bIsEnqueue = false;
    if((!m_singleTaskPhaseSupported) || m_lastTaskInPhase)
    {
        bIsEnqueue = true;
        m_lastTaskInPhase = false;
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddKernelMdf(pCmDev,pCmQueue,pKernel,pCmTask,pKernelRes->pTS,pKernelRes->e,bIsEnqueue));

    // assign the Cm event to global array
    // codechal will check the event to process further
    if (pCmEvent[nCmEventIdx]) 
    {
        pCmQueue->DestroyEvent(pCmEvent[nCmEventIdx]);
    }
    pCmEvent[nCmEventIdx] = pKernelRes->e;
    nCmEventIdx = (nCmEventIdx + 1 ) % CM_EVENT_NUM;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::PreProcKernel()
{    
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
 
    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmDev);

    auto pKernelRes         = &m_resPreProcKernel;
    auto ppCmSurf           = pKernelRes->ppCmSurf;
    auto ppCmBuf            = pKernelRes->ppCmBuf;
    auto ppCmVmeSurfIdx     = pKernelRes->ppCmVmeSurf;
   
    auto pPreEncParams      = (FeiPreEncParams*)m_encodeParams.pPreEncParams;
    auto pRefList           = &m_refList[0];

    CODECHAL_ENCODE_AVC_PREPROC_CURBE_PARAMS    PreProcCurbeParams;
    // Setup Curbe
    MOS_ZeroMemory(&PreProcCurbeParams, sizeof(PreProcCurbeParams));
    PreProcCurbeParams.pPreEncParams = pPreEncParams;
    PreProcCurbeParams.wPicWidthInMb = m_picWidthInMb;
    PreProcCurbeParams.wFieldFrameHeightInMb = m_frameFieldHeightInMb;
    PreProcCurbeParams.pCurbeBinary = pKernelRes->pCurbe;

    SetCurbeAvcPreProc(&PreProcCurbeParams);

    for (int i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        if (m_picIdx[i].bValid)
        {
            uint8_t ucIndex = m_picIdx[i].ucPicIdx;
            pRefList[ucIndex]->sRefBuffer = pRefList[ucIndex]->sRefRawBuffer;
            CodecHal_GetResourceInfo(m_osInterface, &pRefList[ucIndex]->sRefBuffer);
        }
    }
    
    // Set up FtqLut Buffer if there is QP change within a frame
    if (pPreEncParams->bMBQp)
    {
        CODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS InitMbBrcConstantDataBufferParams;

        MOS_ZeroMemory(&InitMbBrcConstantDataBufferParams, sizeof(InitMbBrcConstantDataBufferParams));
        InitMbBrcConstantDataBufferParams.pOsInterface = m_osInterface;
        InitMbBrcConstantDataBufferParams.presBrcConstantDataBuffer =
            &BrcBuffers.resMbBrcConstDataBuffer[m_currRecycledBufIdx];
        InitMbBrcConstantDataBufferParams.bPreProcEnable = true;
        InitMbBrcConstantDataBufferParams.bEnableKernelTrellis = bKernelTrellis && m_trellisQuantParams.dwTqEnabled;;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitMbBrcConstantDataBuffer(&InitMbBrcConstantDataBufferParams));
    }
    SurfaceIndex    *cmSurfIdx[9];
    for(int i = 0; i < 9; i ++)
    {
        cmSurfIdx[i] = (SurfaceIndex*)CM_NULL_SURFACE;
    }
    
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&m_rawSurfaceToEnc->OsResource, ppCmSurf[0]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[0]->GetIndex(cmSurfIdx[0])); // current input surface

    if(CodecHal_PictureIsField(m_currOriginalPic))
    {
        if(CodecHal_PictureIsTopField(m_currOriginalPic))
        {
            ppCmSurf[0]->SetProperty(CM_TOP_FIELD);
        }
        else
        {
            ppCmSurf[0]->SetProperty(CM_BOTTOM_FIELD);
        }
    }

    // HME MV input surface
    if(m_hmeEnabled)
    { 
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&m_4xMeMvDataBuffer.OsResource, ppCmSurf[1]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[1]->GetIndex(cmSurfIdx[1]));
    }

    // App Provided Prediction surfaces
    if(pPreEncParams->dwMVPredictorCtrl)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&pPreEncParams->resMvPredBuffer, ppCmBuf[0]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[0]->GetIndex(cmSurfIdx[2]));
    }
  
    if (pPreEncParams->bMBQp)
    {
        // Qp Per MB Input Surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&pPreEncParams->resMbQpBuffer, ppCmBuf[1]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[1]->GetIndex(cmSurfIdx[3]));
        // Qp and FTQ LUT
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&BrcBuffers.resMbBrcConstDataBuffer[m_currRecycledBufIdx], ppCmBuf[2]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[2]->GetIndex(cmSurfIdx[8]));
    } 

    // MV Data Output Surface
    if(!pPreEncParams->bDisableMVOutput)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&pPreEncParams->resMvBuffer, ppCmBuf[3]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[3]->GetIndex(cmSurfIdx[4]));
    } 
 
    if(!pPreEncParams->bDisableStatisticsOutput) 
    {
        if(CodecHal_PictureIsBottomField(m_currOriginalPic))
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&pPreEncParams->resStatsBotFieldBuffer, ppCmBuf[4]));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[4]->GetIndex(cmSurfIdx[5]));
        }
        else
        {       
            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&pPreEncParams->resStatsBuffer, ppCmBuf[4]));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[4]->GetIndex(cmSurfIdx[5]));
        }        
    }

    uint8_t BaseIdx = 1;
    auto pCmSurfForVME = ppCmSurf[0];
    uint8_t ucRefIdx = 0;
    CmSurface2D *surfArray[8];//[NUM_SURFACES];
    if(pPreEncParams->dwNumPastReferences) 
    {
        CODEC_PICTURE RefPic = pPreEncParams->PastRefPicture; 
        uint8_t ucRefPicIdx = RefPic.FrameIdx;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&m_refList[ucRefPicIdx]->sRefBuffer.OsResource, ppCmSurf[BaseIdx + 1]));
        surfArray[0] = (CmSurface2D*)ppCmSurf[BaseIdx + 1];

        ucRefIdx = 1;
    } 
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateVmeSurfaceG7_5(pCmSurfForVME, &surfArray[0], nullptr, ucRefIdx, 0, ppCmVmeSurfIdx[0]));  
    cmSurfIdx[6] = ppCmVmeSurfIdx[0];
         
    BaseIdx = 2;
    ucRefIdx = 0;
    if(pPreEncParams->dwNumFutureReferences) 
    {
        CODEC_PICTURE RefPic = pPreEncParams->FutureRefPicture; 
        uint8_t ucRefPicIdx = RefPic.FrameIdx;
                
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&m_refList[ucRefPicIdx]->sRefBuffer.OsResource, ppCmSurf[BaseIdx + 1]));
        surfArray[0] = (CmSurface2D*)ppCmSurf[BaseIdx + 1];
        ucRefIdx = 1;
    } 

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateVmeSurfaceG7_5(pCmSurfForVME, &surfArray[0], nullptr, ucRefIdx, 0, ppCmVmeSurfIdx[1]));
    cmSurfIdx[7] = ppCmVmeSurfIdx[1];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(DispatchKernelPreProc(cmSurfIdx,(uint16_t)m_frameWidth, (uint16_t)m_frameFieldHeight));
    return eStatus;
}

#else
MOS_STATUS CodechalEncodeAvcEncFeiG9::InitKernelStateMe()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHal_GetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    for (uint32_t dwKrnStateIdx = 0; dwKrnStateIdx < 2; dwKrnStateIdx++)
    {
        auto pKernelStatePtr = &m_meKernelStates[dwKrnStateIdx];
        CODECHAL_KERNEL_HEADER              CurrKrnHeader;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeGetKernelHeaderAndSize(
            kernelBinary,
            ENC_ME,
            dwKrnStateIdx,
            &CurrKrnHeader,
            &kernelSize));

        pKernelStatePtr->KernelParams.iBTCount = CODECHAL_ENCODE_AVC_ME_NUM_SURFACES_CM_G9;
        pKernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        pKernelStatePtr->KernelParams.iCurbeLength = sizeof(CODECHAL_ENCODE_AVC_ME_CURBE_CM_FEI_G9);
        pKernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
        pKernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        pKernelStatePtr->KernelParams.iIdCount = 1;

        pKernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        pKernelStatePtr->KernelParams.pBinary = kernelBinary + (CurrKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        pKernelStatePtr->KernelParams.iSize = kernelSize;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            pKernelStatePtr->KernelParams.iBTCount,
            &pKernelStatePtr->dwSshSize,
            &pKernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_MhwInitISH(m_stateHeapInterface, pKernelStatePtr));
    }

    // Until a better way can be found, maintain old binding table structures
    auto pBindingTable = &m_meBindingTable;
    pBindingTable->dwMEMVDataSurface = CODECHAL_ENCODE_AVC_ME_MV_DATA_SURFACE_CM_G9;
    pBindingTable->dw16xMEMVDataSurface = CODECHAL_ENCODE_AVC_16xME_MV_DATA_SURFACE_CM_G9;
    pBindingTable->dw32xMEMVDataSurface = CODECHAL_ENCODE_AVC_32xME_MV_DATA_SURFACE_CM_G9;
    pBindingTable->dwMEDist = CODECHAL_ENCODE_AVC_ME_DISTORTION_SURFACE_CM_G9;
    pBindingTable->dwMEBRCDist = CODECHAL_ENCODE_AVC_ME_BRC_DISTORTION_CM_G9;
    pBindingTable->dwMECurrForFwdRef = CODECHAL_ENCODE_AVC_ME_CURR_FOR_FWD_REF_CM_G9;
    pBindingTable->dwMEFwdRefPicIdx[0] = CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX0_CM_G9;
    pBindingTable->dwMEFwdRefPicIdx[1] = CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX1_CM_G9;
    pBindingTable->dwMEFwdRefPicIdx[2] = CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX2_CM_G9;
    pBindingTable->dwMEFwdRefPicIdx[3] = CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX3_CM_G9;
    pBindingTable->dwMEFwdRefPicIdx[4] = CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX4_CM_G9;
    pBindingTable->dwMEFwdRefPicIdx[5] = CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX5_CM_G9;
    pBindingTable->dwMEFwdRefPicIdx[6] = CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX6_CM_G9;
    pBindingTable->dwMEFwdRefPicIdx[7] = CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX7_CM_G9;
    pBindingTable->dwMECurrForBwdRef = CODECHAL_ENCODE_AVC_ME_CURR_FOR_BWD_REF_CM_G9;
    pBindingTable->dwMEBwdRefPicIdx[0] = CODECHAL_ENCODE_AVC_ME_BWD_REF_IDX0_CM_G9;
    pBindingTable->dwMEBwdRefPicIdx[1] = CODECHAL_ENCODE_AVC_ME_BWD_REF_IDX1_CM_G9;


    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::InitKernelStatePreProc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_GetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize));

    auto pKernelStatePtr = &PreProcKernelState;
    uint32_t dwKrnStateIdx = 0;
    CODECHAL_KERNEL_HEADER CurrKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeGetKernelHeaderAndSize(
        kernelBinary,
        ENC_PREPROC,
        dwKrnStateIdx,
        &CurrKrnHeader,
        &kernelSize));

    pKernelStatePtr->KernelParams.iBTCount = CODECHAL_ENCODE_AVC_PREPROC_FIELD_NUM_SURFACES_CM_G9;
    pKernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    pKernelStatePtr->KernelParams.iCurbeLength = sizeof(CODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9);
    pKernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
    pKernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
    pKernelStatePtr->KernelParams.iIdCount = 1;

    pKernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    pKernelStatePtr->KernelParams.pBinary = kernelBinary + (CurrKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    pKernelStatePtr->KernelParams.iSize = kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        pKernelStatePtr->KernelParams.iBTCount,
        &pKernelStatePtr->dwSshSize,
        &pKernelStatePtr->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_MhwInitISH(m_stateHeapInterface, pKernelStatePtr));

    // Until a better way can be found, maintain old binding table structures
    auto pBindingTable = &PreProcBindingTable;

    pBindingTable->dwAvcPreProcCurrY = CODECHAL_ENCODE_AVC_PREPROC_CURR_Y_CM_G9;
    pBindingTable->dwAvcPreProcCurrUV = CODECHAL_ENCODE_AVC_PREPROC_CURR_UV_CM_G9;
    pBindingTable->dwAvcPreProcMVDataFromHME = CODECHAL_ENCODE_AVC_PREPROC_HME_MV_DATA_CM_G9;
    pBindingTable->dwAvcPreProcMvPredictor = CODECHAL_ENCODE_AVC_PREPROC_MV_PREDICTOR_CM_G9;
    pBindingTable->dwAvcPreProcMbQp = CODECHAL_ENCODE_AVC_PREPROC_MBQP_CM_G9;
    pBindingTable->dwAvcPreProcMvDataOut = CODECHAL_ENCODE_AVC_PREPROC_MV_DATA_CM_G9;
    pBindingTable->dwAvcPreProcMbStatsOut = CODECHAL_ENCODE_AVC_PREPROC_MB_STATS_CM_G9;
    pBindingTable->dwAvcPreProcVMECurrPicFrame[0] = CODECHAL_ENCODE_AVC_PREPROC_VME_CURR_PIC_IDX_0_CM_G9;
    pBindingTable->dwAvcPreProcVMEFwdPicFrame = CODECHAL_ENCODE_AVC_PREPROC_VME_FWD_PIC_IDX0_CM_G9;
    pBindingTable->dwAvcPreProcVMEBwdPicFrame[0] = CODECHAL_ENCODE_AVC_PREPROC_VME_BWD_PIC_IDX0_0_CM_G9;
    pBindingTable->dwAvcPreProcVMECurrPicFrame[1] = CODECHAL_ENCODE_AVC_PREPROC_VME_CURR_PIC_IDX_1_CM_G9;
    pBindingTable->dwAvcPreProcVMEBwdPicFrame[1] = CODECHAL_ENCODE_AVC_PREPROC_VME_BWD_PIC_IDX0_1_CM_G9;
    pBindingTable->dwAvcPreProcFtqLut = CODECHAL_ENCODE_AVC_PREPROC_FTQ_LUT_CM_G9;

    //field case
    pBindingTable->dwAvcPreProcVMECurrPicField[0] = CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_CURR_PIC_IDX_0_CM_G9;
    pBindingTable->dwAvcPreProcVMEFwdPicField[0] = CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_FWD_PIC_IDX0_0_CM_G9;

    pBindingTable->dwAvcPreProcVMEFwdPicField[1] = CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_FWD_PIC_IDX0_1_CM_G9;

    pBindingTable->dwAvcPreProcVMECurrPicField[1] = CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_CURR_PIC_IDX_1_CM_G9;
    pBindingTable->dwAvcPreProcVMEBwdPicField[0] = CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_BWD_PIC_IDX0_0_CM_G9;

    pBindingTable->dwAvcPreProcVMEBwdPicField[1] = CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_BWD_PIC_IDX0_1_CM_G9;

    pBindingTable->dwAvcPreProcFtqLutField = CODECHAL_ENCODE_AVC_PREPROC_FIELD_FTQ_LUT_CM_G9;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::PreProcKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto pPreEncParams = (FeiPreEncParams*)m_encodeParams.pPreEncParams;
    auto pRefList = &m_refList[0];

    auto EncFunctionType = CODECHAL_MEDIA_STATE_PREPROC;
    auto pKernelState = &PreProcKernelState;

    PerfTagSetting PerfTag;
    PerfTag.Value = 0;
    PerfTag.Mode = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    PerfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_PREPROC_KERNEL;
    PerfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, PerfTag.Value);

    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase == true || !m_singleTaskPhaseSupported)
    {
        uint32_t dwMaxBtCount = m_singleTaskPhaseSupported ?
            m_maxBtCount : pKernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
            m_stateHeapInterface,
            dwMaxBtCount));
        m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(dwMaxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    // Set up the DSH/SSH as normal
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_AssignDshAndSshSpace(
        m_stateHeapInterface,
        pKernelState,
        false,
        0,
        false,
        m_storeData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS IdParams;
    MOS_ZeroMemory(&IdParams, sizeof(IdParams));
    IdParams.pKernelState = pKernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        1,
        &IdParams));

    // Setup Curbe
    CODECHAL_ENCODE_AVC_PREPROC_CURBE_PARAMS PreProcCurbeParams;
    MOS_ZeroMemory(&PreProcCurbeParams, sizeof(PreProcCurbeParams));
    PreProcCurbeParams.pPreEncParams = pPreEncParams;
    PreProcCurbeParams.wPicWidthInMb = m_picWidthInMb;
    PreProcCurbeParams.wFieldFrameHeightInMb = m_frameFieldHeightInMb;
    PreProcCurbeParams.pKernelState = pKernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeAvcPreProc(
        &PreProcCurbeParams));

    CODECHAL_DEBUG_TOOL(
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
        EncFunctionType,
        MHW_DSH_TYPE,
        pKernelState));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
        EncFunctionType,
        pKernelState));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
        EncFunctionType,
        MHW_ISH_TYPE,
        pKernelState));
    )

    for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        if (m_picIdx[i].bValid)
        {
            uint8_t ucIndex = m_picIdx[i].ucPicIdx;
            pRefList[ucIndex]->sRefBuffer = pRefList[ucIndex]->sRefRawBuffer;

            CodecHal_GetResourceInfo(m_osInterface, &pRefList[ucIndex]->sRefBuffer);
        }
    }

    MOS_COMMAND_BUFFER CmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &CmdBuffer, 0));

    SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = EncFunctionType;
    sendKernelCmdsParams.ucDmvPredFlag = (m_pictureCodingType == I_TYPE) ? 0 : m_avcSliceParams->direct_spatial_mv_pred_flag;

    sendKernelCmdsParams.pKernelState = pKernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&CmdBuffer, &sendKernelCmdsParams));

    // Set up FtqLut Buffer if there is QP change within a frame
    if (pPreEncParams->bMBQp)
    {
        CODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS InitMbBrcConstantDataBufferParams;

        MOS_ZeroMemory(&InitMbBrcConstantDataBufferParams, sizeof(InitMbBrcConstantDataBufferParams));
        InitMbBrcConstantDataBufferParams.pOsInterface = m_osInterface;
        InitMbBrcConstantDataBufferParams.presBrcConstantDataBuffer =
            &BrcBuffers.resMbBrcConstDataBuffer[m_currRecycledBufIdx];
        InitMbBrcConstantDataBufferParams.bPreProcEnable = true;
        InitMbBrcConstantDataBufferParams.bEnableKernelTrellis = bKernelTrellis && m_trellisQuantParams.dwTqEnabled;;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitMbBrcConstantDataBuffer(&InitMbBrcConstantDataBufferParams));
    }

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        pKernelState));

    //Add surface states
    CODECHAL_ENCODE_AVC_PREPROC_SURFACE_PARAMS PreProcSurfaceParams;
    MOS_ZeroMemory(&PreProcSurfaceParams, sizeof(PreProcSurfaceParams));
    PreProcSurfaceParams.pPreEncParams = pPreEncParams;
    PreProcSurfaceParams.ppRefList = &m_refList[0];
    PreProcSurfaceParams.pCurrOriginalPic = &m_currOriginalPic;
    PreProcSurfaceParams.psCurrPicSurface = m_rawSurfaceToEnc;
    PreProcSurfaceParams.ps4xMeMvDataBuffer = &m_4xMeMvDataBuffer;
    PreProcSurfaceParams.presFtqLutBuffer = &BrcBuffers.resMbBrcConstDataBuffer[m_currRecycledBufIdx];
    PreProcSurfaceParams.dwMeMvBottomFieldOffset = m_meMvBottomFieldOffset;
    PreProcSurfaceParams.dwFrameWidthInMb = (uint32_t)m_picWidthInMb;
    PreProcSurfaceParams.dwFrameFieldHeightInMb = (uint32_t)m_frameFieldHeightInMb;
    PreProcSurfaceParams.dwMBVProcStatsBottomFieldOffset =
        CodecHal_PictureIsBottomField(m_currOriginalPic) ? dwMBVProcStatsBottomFieldOffset : 0;
    // Interleaved input surfaces
    PreProcSurfaceParams.dwVerticalLineStride = m_verticalLineStride;
    PreProcSurfaceParams.dwVerticalLineStrideOffset = m_verticalLineStrideOffset;
    PreProcSurfaceParams.bHmeEnabled = m_hmeEnabled;
    PreProcSurfaceParams.pPreProcBindingTable = &PreProcBindingTable;
    PreProcSurfaceParams.pKernelState = pKernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendAvcPreProcSurfaces(&CmdBuffer, &PreProcSurfaceParams));

    uint32_t dwResolutionX = (uint32_t)m_picWidthInMb;
    uint32_t dwResolutionY = (uint32_t)m_frameFieldHeightInMb;

    CODECHAL_WALKER_CODEC_PARAMS WalkerCodecParams;
    MOS_ZeroMemory(&WalkerCodecParams, sizeof(WalkerCodecParams));
    WalkerCodecParams.WalkerMode = m_walkerMode;
    WalkerCodecParams.dwResolutionX = dwResolutionX;
    WalkerCodecParams.dwResolutionY = dwResolutionY;
    WalkerCodecParams.bNoDependency = true;
    WalkerCodecParams.bMbaff = m_mbaffEnabled;
    WalkerCodecParams.bGroupIdSelectSupported = m_groupIdSelectSupported;
    WalkerCodecParams.ucGroupId = m_groupId;
    
    MHW_WALKER_PARAMS WalkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_InitMediaObjectWalkerParams(
        m_hwInterface,
        &WalkerParams,
        &WalkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &CmdBuffer,
        &WalkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&CmdBuffer, EncFunctionType));

    // Add dump for MBEnc surface state heap here
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            EncFunctionType,
            MHW_SSH_TYPE,
            pKernelState));
    )

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &CmdBuffer,
        EncFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        pKernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&CmdBuffer, nullptr));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&CmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &CmdBuffer, 0);

    if ((!m_singleTaskPhaseSupported || m_lastTaskInPhase))
    {
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &CmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

#endif
MOS_STATUS CodechalEncodeAvcEncFeiG9::SetCurbeMe(MeCurbeParams* pParams)
{
    MOS_STATUS   eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);

    auto pPreEncParams = (FeiPreEncParams*)m_encodeParams.pPreEncParams;
    CODECHAL_ENCODE_CHK_NULL_RETURN(pPreEncParams);

    auto pSlcParams = m_avcSliceParams;
    bool bFramePicture = CodecHal_PictureIsFrame(m_currOriginalPic);
    uint32_t dwScaleFactor;
    uint8_t ucMvShiftFactor = 0, ucPrevMvReadPosFactor = 0;
    bool bUseMvFromPrevStep, bWriteDistortions;
    switch (pParams->hmeLvl)
    {
    case HME_LEVEL_4x:
        bUseMvFromPrevStep = false;
        bWriteDistortions = false;
        dwScaleFactor = SCALE_FACTOR_4x;
        ucMvShiftFactor = 2;
        ucPrevMvReadPosFactor = 0;
        break;
    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
        break;
    }

    CODECHAL_ENCODE_AVC_ME_CURBE_CM_FEI_G9 Cmd;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        &Cmd,
        sizeof(CODECHAL_ENCODE_AVC_ME_CURBE_CM_FEI_G9),
        ME_CURBE_CM_FEI,
        sizeof(CODECHAL_ENCODE_AVC_ME_CURBE_CM_FEI_G9)));


    Cmd.DW3.SubPelMode = 3;
    if (m_fieldScalingOutputInterleaved)
    {
        Cmd.DW3.SrcAccess =
            Cmd.DW3.RefAccess = CodecHal_PictureIsField(m_currOriginalPic) ? 1 : 0;
        Cmd.DW7.SrcFieldPolarity = CodecHal_PictureIsBottomField(m_currOriginalPic) ? 1 : 0;
    }

    Cmd.DW4.PictureHeightMinus1 =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / dwScaleFactor) - 1;
    Cmd.DW4.PictureWidth =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth / dwScaleFactor);
    Cmd.DW5.QpPrimeY = pPreEncParams->dwFrameQp;
    Cmd.DW6.WriteDistortions = bWriteDistortions;
    Cmd.DW6.UseMvFromPrevStep = bUseMvFromPrevStep;

    Cmd.DW6.SuperCombineDist = m_superCombineDistGeneric[m_targetUsage];
    Cmd.DW6.MaxVmvR = (bFramePicture) ?
        CodecHalAvcEncode_GetMaxMvLen(CODECHAL_AVC_LEVEL_52) * 4 :
        (CodecHalAvcEncode_GetMaxMvLen(CODECHAL_AVC_LEVEL_52) >> 1) * 4;

    if (m_pictureCodingType == B_TYPE)
    {
        // This field is irrelevant since we are not using the bi-direct search.
        // set it to 32
        Cmd.DW1.BiWeight = 32;
        Cmd.DW13.NumRefIdxL1MinusOne = pSlcParams->num_ref_idx_l1_active_minus1;
    }

    if (m_pictureCodingType == P_TYPE ||
        m_pictureCodingType == B_TYPE)
    {
        Cmd.DW13.NumRefIdxL0MinusOne = pSlcParams->num_ref_idx_l0_active_minus1;
    }

    if (!bFramePicture)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            Cmd.DW14.List0RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_0);
            Cmd.DW14.List0RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_1);
            Cmd.DW14.List0RefID2FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_2);
            Cmd.DW14.List0RefID3FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_3);
            Cmd.DW14.List0RefID4FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_4);
            Cmd.DW14.List0RefID5FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_5);
            Cmd.DW14.List0RefID6FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_6);
            Cmd.DW14.List0RefID7FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        }
        if (m_pictureCodingType == B_TYPE)
        {
            Cmd.DW14.List1RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
            Cmd.DW14.List1RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        }
    }

    Cmd.DW15.MvShiftFactor = ucMvShiftFactor;
    Cmd.DW15.PrevMvReadPosFactor = ucPrevMvReadPosFactor;

    // r3 & r4
    uint8_t ucMeMethod = (m_pictureCodingType == B_TYPE) ?
        m_BMeMethodGeneric[m_targetUsage] :
        m_MeMethodGeneric[m_targetUsage];
    uint8_t ucTableIdx = (m_pictureCodingType == B_TYPE) ? 1 : 0;
    eStatus = MOS_SecureMemcpy(&(Cmd.SPDelta), 14 * sizeof(uint32_t), CodechalEncoderState::m_encodeSearchPath[ucTableIdx][ucMeMethod], 14 * sizeof(uint32_t));
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }
    if(pParams->pCurbeBinary)
    {
        MOS_SecureMemcpy(pParams->pCurbeBinary,m_meCurbeDataSizeFei,&Cmd,m_meCurbeDataSizeFei);
        return eStatus;
    }
    // r5
    Cmd.DW32._4xMeMvOutputDataSurfIndex = CODECHAL_ENCODE_AVC_ME_MV_DATA_SURFACE_CM_G9;
    Cmd.DW33._16xOr32xMeMvInputDataSurfIndex = (pParams->hmeLvl == HME_LEVEL_32x) ?
        CODECHAL_ENCODE_AVC_32xME_MV_DATA_SURFACE_CM_G9 : CODECHAL_ENCODE_AVC_16xME_MV_DATA_SURFACE_CM_G9;
    Cmd.DW34._4xMeOutputDistSurfIndex = CODECHAL_ENCODE_AVC_ME_DISTORTION_SURFACE_CM_G9;
    Cmd.DW35._4xMeOutputBrcDistSurfIndex = CODECHAL_ENCODE_AVC_ME_BRC_DISTORTION_CM_G9;
    Cmd.DW36.VMEFwdInterPredictionSurfIndex = CODECHAL_ENCODE_AVC_ME_CURR_FOR_FWD_REF_CM_G9;
    Cmd.DW37.VMEBwdInterPredictionSurfIndex = CODECHAL_ENCODE_AVC_ME_CURR_FOR_BWD_REF_CM_G9;
    Cmd.DW38.Value = 0;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pParams->pKernelState->m_dshRegion.AddData(
        &Cmd,
        pParams->pKernelState->dwCurbeOffset,
        sizeof(Cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::SendMeSurfaces(PMOS_COMMAND_BUFFER pCmdBuffer, MeSurfaceParams* pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pCurrOriginalPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pCurrReconstructedPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->ps4xMeMvDataBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pTrackedBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->psMeBrcDistortionBuffer);

    // only 4x HME is supported for FEI
    CODECHAL_ENCODE_ASSERT(!pParams->b32xMeInUse && !pParams->b16xMeInUse);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pMeBindingTable);
    auto pMeBindingTable = pParams->pMeBindingTable;

    uint8_t ucScaledIdx = pParams->ppRefList[pParams->pCurrReconstructedPic->FrameIdx]->ucScalingIdx;
    bool bCurrFieldPicture = CodecHal_PictureIsField(*(pParams->pCurrOriginalPic)) ? 1 : 0;
    bool bCurrBottomField = CodecHal_PictureIsBottomField(*(pParams->pCurrOriginalPic)) ? 1 : 0;
    uint8_t ucCurrVDirection = (!bCurrFieldPicture) ? CODECHAL_VDIRECTION_FRAME :
        ((bCurrBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);

    auto psCurrScaledSurface = &pParams->pTrackedBuffer[ucScaledIdx].sScaled4xSurface;
    auto psMeMvDataBuffer = pParams->ps4xMeMvDataBuffer;
    uint32_t dwMeMvBottomFieldOffset = pParams->dw4xMeMvBottomFieldOffset;
    uint32_t dwCurrScaledBottomFieldOffset = pParams->dw4xScaledBottomFieldOffset;

    // Reference height and width information should be taken from the current scaled surface rather
    // than from the reference scaled surface in the case of PAFF.
    auto sRefScaledSurface = *psCurrScaledSurface;

    uint32_t dwWidth = MOS_ALIGN_CEIL(pParams->dwDownscaledWidthInMb * 32, 64);
    uint32_t dwHeight = pParams->dwDownscaledHeightInMb * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER;

    // Force the values
    psMeMvDataBuffer->dwWidth = dwWidth;
    psMeMvDataBuffer->dwHeight = dwHeight;
    psMeMvDataBuffer->dwPitch = dwWidth;

    CODECHAL_SURFACE_CODEC_PARAMS   SurfaceParams;
    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));
    SurfaceParams.bIs2DSurface = true;
    SurfaceParams.bMediaBlockRW = true;
    SurfaceParams.psSurface = psMeMvDataBuffer;
    SurfaceParams.dwOffset = dwMeMvBottomFieldOffset;
    SurfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    SurfaceParams.dwBindingTableOffset = pMeBindingTable->dwMEMVDataSurface;
    SurfaceParams.bIsWritable = true;
    SurfaceParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceParams,
        pParams->pKernelState));

    // Insert Distortion buffers only for 4xMe case
    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));
    SurfaceParams.bIs2DSurface = true;
    SurfaceParams.bMediaBlockRW = true;
    SurfaceParams.psSurface = pParams->psMeBrcDistortionBuffer;
    SurfaceParams.dwOffset = pParams->dwMeBrcDistortionBottomFieldOffset;
    SurfaceParams.dwBindingTableOffset = pMeBindingTable->dwMEBRCDist;
    SurfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value;
    SurfaceParams.bIsWritable = true;
    SurfaceParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceParams,
        pParams->pKernelState));

    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));
    SurfaceParams.bIs2DSurface = true;
    SurfaceParams.bMediaBlockRW = true;
    SurfaceParams.psSurface = pParams->psMeDistortionBuffer;
    SurfaceParams.dwOffset = pParams->dwMeDistortionBottomFieldOffset;
    SurfaceParams.dwBindingTableOffset = pMeBindingTable->dwMEDist;
    SurfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
    SurfaceParams.bIsWritable = true;
    SurfaceParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceParams,
        pParams->pKernelState));

    // Setup references 1...n
    // LIST 0 references
    uint32_t dwRefScaledBottomFieldOffset;
    uint8_t  ucRefPicIdx, ucRefIdx;
    bool bRefFieldPicture, bRefBottomField;
    for (ucRefIdx = 0; ucRefIdx <= pParams->dwNumRefIdxL0ActiveMinus1; ucRefIdx++)
    {
        auto RefPic = pParams->pL0RefFrameList[ucRefIdx];

        if (!CodecHal_PictureIsInvalid(RefPic) && pParams->pPicIdx[RefPic.FrameIdx].bValid)
        {
            if (ucRefIdx == 0)
            {
                // Current Picture Y - VME
                MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));
                SurfaceParams.bUseAdvState = true;
                SurfaceParams.psSurface = psCurrScaledSurface;
                SurfaceParams.dwOffset = bCurrBottomField ? dwCurrScaledBottomFieldOffset : 0;
                SurfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
                SurfaceParams.dwBindingTableOffset = pMeBindingTable->dwMECurrForFwdRef;
                SurfaceParams.ucVDirection = ucCurrVDirection;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                    m_hwInterface,
                    pCmdBuffer,
                    &SurfaceParams,
                    pParams->pKernelState));
            }

            bRefFieldPicture = CodecHal_PictureIsField(RefPic) ? 1 : 0;
            bRefBottomField = (CodecHal_PictureIsBottomField(RefPic)) ? 1 : 0;
            ucRefPicIdx = pParams->pPicIdx[RefPic.FrameIdx].ucPicIdx;
            ucScaledIdx = pParams->ppRefList[ucRefPicIdx]->ucScalingIdx;

            sRefScaledSurface.OsResource = pParams->pTrackedBuffer[ucScaledIdx].sScaled4xSurface.OsResource;
            dwRefScaledBottomFieldOffset = bRefBottomField ? dwCurrScaledBottomFieldOffset : 0;

            // L0 Reference Picture Y - VME
            MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));
            SurfaceParams.bUseAdvState = true;
            SurfaceParams.psSurface = &sRefScaledSurface;
            SurfaceParams.dwOffset = bRefBottomField ? dwRefScaledBottomFieldOffset : 0;
            SurfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
            SurfaceParams.dwBindingTableOffset = pMeBindingTable->dwMEFwdRefPicIdx[ucRefIdx];
            SurfaceParams.ucVDirection = !bCurrFieldPicture ? CODECHAL_VDIRECTION_FRAME :
                ((bRefBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                m_hwInterface,
                pCmdBuffer,
                &SurfaceParams,
                pParams->pKernelState));

            if (bCurrFieldPicture)
            {
                // VME needs to set ref index 1 too for field case
                SurfaceParams.dwBindingTableOffset = pMeBindingTable->dwMEFwdRefPicIdx[ucRefIdx + 1];
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                    m_hwInterface,
                    pCmdBuffer,
                    &SurfaceParams,
                    pParams->pKernelState));
            }
        }
    }

    // Setup references 1...n
    // LIST 1 references
    for (ucRefIdx = 0; ucRefIdx <= pParams->dwNumRefIdxL1ActiveMinus1; ucRefIdx++)
    {
        auto RefPic = pParams->pL1RefFrameList[ucRefIdx];

        if (!CodecHal_PictureIsInvalid(RefPic) && pParams->pPicIdx[RefPic.FrameIdx].bValid)
        {
            if (ucRefIdx == 0)
            {
                // Current Picture Y - VME
                MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));
                SurfaceParams.bUseAdvState = true;
                SurfaceParams.psSurface = psCurrScaledSurface;
                SurfaceParams.dwOffset = bCurrBottomField ? dwCurrScaledBottomFieldOffset : 0;
                SurfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
                SurfaceParams.dwBindingTableOffset = pMeBindingTable->dwMECurrForBwdRef;
                SurfaceParams.ucVDirection = ucCurrVDirection;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                    m_hwInterface,
                    pCmdBuffer,
                    &SurfaceParams,
                    pParams->pKernelState));
            }

            bRefFieldPicture = CodecHal_PictureIsField(RefPic) ? 1 : 0;
            bRefBottomField = (CodecHal_PictureIsBottomField(RefPic)) ? 1 : 0;
            ucRefPicIdx = pParams->pPicIdx[RefPic.FrameIdx].ucPicIdx;
            ucScaledIdx = pParams->ppRefList[ucRefPicIdx]->ucScalingIdx;

            sRefScaledSurface.OsResource = pParams->pTrackedBuffer[ucScaledIdx].sScaled4xSurface.OsResource;
            dwRefScaledBottomFieldOffset = bRefBottomField ? dwCurrScaledBottomFieldOffset : 0;

            // L1 Reference Picture Y - VME
            MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));
            SurfaceParams.bUseAdvState = true;
            SurfaceParams.psSurface = &sRefScaledSurface;
            SurfaceParams.dwOffset = bRefBottomField ? dwRefScaledBottomFieldOffset : 0;
            SurfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
            SurfaceParams.dwBindingTableOffset = pMeBindingTable->dwMEBwdRefPicIdx[ucRefIdx];
            SurfaceParams.ucVDirection = (!bCurrFieldPicture) ? CODECHAL_VDIRECTION_FRAME :
                ((bRefBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                m_hwInterface,
                pCmdBuffer,
                &SurfaceParams,
                pParams->pKernelState));

            if (bCurrFieldPicture)
            {
                // VME needs to set ref index 1 too for field case
                SurfaceParams.dwBindingTableOffset = pMeBindingTable->dwMEBwdRefPicIdx[ucRefIdx + 1];
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                    m_hwInterface,
                    pCmdBuffer,
                    &SurfaceParams,
                    pParams->pKernelState));
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::EncodeGetKernelHeaderAndSize(void *pvBinary, EncOperation Operation, uint32_t dwKrnStateIdx, void* pvKrnHeader, uint32_t *pdwKrnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pvBinary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pvKrnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pdwKrnSize);

    auto pKernelHeaderTable = (PCODECHAL_ENCODE_AVC_KERNEL_HEADER_FEI_G9)pvBinary;
    PCODECHAL_KERNEL_HEADER pInvalidEntry = &(pKernelHeaderTable->AVC_WeightedPrediction) + 1;
    uint32_t dwNextKrnOffset = *pdwKrnSize;

    PCODECHAL_KERNEL_HEADER pCurrKrnHeader, pNextKrnHeader;
    if (Operation == ENC_SCALING4X)
    {
        pCurrKrnHeader = &pKernelHeaderTable->PLY_DScale_PLY;
    }
    else if (Operation == ENC_ME)
    {
        pCurrKrnHeader = &pKernelHeaderTable->AVC_ME_P;
    }
    else if (Operation == ENC_MBENC)
    {
        pCurrKrnHeader = &pKernelHeaderTable->AVCMBEnc_Fei_I;
    }
    else if (Operation == ENC_PREPROC)
    {
        pCurrKrnHeader = &pKernelHeaderTable->AVC_Fei_ProProc;
    }
    else if (Operation == ENC_WP)
    {
        pCurrKrnHeader = &pKernelHeaderTable->AVC_WeightedPrediction;
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    pCurrKrnHeader += dwKrnStateIdx;
    *((PCODECHAL_KERNEL_HEADER)pvKrnHeader) = *pCurrKrnHeader;

    pNextKrnHeader = (pCurrKrnHeader + 1);
    if (pNextKrnHeader < pInvalidEntry)
    {
        dwNextKrnOffset = pNextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *pdwKrnSize = dwNextKrnOffset - (pCurrKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::InitializeState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcEnc::InitializeState());

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

    m_brcHistoryBufferSize = CODECHAL_ENCODE_AVC_BRC_HISTORY_BUFFER_SIZE_G9;
    m_mbencBrcBufferSize   = m_mbencCurbeDataSizeFei;
    dwBrcConstantSurfaceWidth = CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_WIDTH_G9;
    dwBrcConstantSurfaceHeight = m_brcConstantSurfaceHeightFei;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::ValidateNumReferences(PCODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pAvcSliceParams);

    uint8_t ucNumRefIdx0MinusOne = pParams->pAvcSliceParams->num_ref_idx_l0_active_minus1;
    uint8_t ucNumRefIdx1MinusOne = pParams->pAvcSliceParams->num_ref_idx_l1_active_minus1;

    // Nothing to do here if numRefIdx = 0 and frame encoded
    if (ucNumRefIdx0MinusOne == 0 && !CodecHal_PictureIsField(pParams->pPicParams->CurrOriginalPic))
    {
        if (pParams->wPictureCodingType == P_TYPE ||
            (pParams->wPictureCodingType == B_TYPE && ucNumRefIdx1MinusOne == 0))
        {
            return eStatus;
        }
    }

    if (pParams->wPictureCodingType == P_TYPE || pParams->wPictureCodingType == B_TYPE)
    {
        uint8_t   ucMaxPNumRefIdx0MinusOne = 3;
        uint8_t   ucMaxPNumRefIdx1MinusOne = 1;
        if(pParams->bPAKonly)
        {
            ucMaxPNumRefIdx0MinusOne = 15;
            ucMaxPNumRefIdx1MinusOne = 15;
        }
        if (pParams->wPictureCodingType == P_TYPE)
        {
            if (ucNumRefIdx0MinusOne > ucMaxPNumRefIdx0MinusOne)
            {
                CODECHAL_ENCODE_NORMALMESSAGE("Invalid active reference list size.");
                ucNumRefIdx0MinusOne = ucMaxPNumRefIdx0MinusOne;
            }
            ucNumRefIdx1MinusOne = 0;
        }
        else // B_TYPE
        {
            if (ucNumRefIdx0MinusOne > ucMaxPNumRefIdx0MinusOne)
            {
                CODECHAL_ENCODE_NORMALMESSAGE("Invalid active reference list size.");
                ucNumRefIdx0MinusOne = ucMaxPNumRefIdx0MinusOne;
            }

            if (ucNumRefIdx1MinusOne > ucMaxPNumRefIdx1MinusOne)
            {
                CODECHAL_ENCODE_NORMALMESSAGE("Invalid active reference list size.");
                ucNumRefIdx1MinusOne = ucMaxPNumRefIdx1MinusOne;
            }

            // supports at most 1 L1 ref for frame picture for non-PAK only case
            if (CodecHal_PictureIsFrame(pParams->pPicParams->CurrOriginalPic) && ucNumRefIdx1MinusOne > 0  && (!pParams->bPAKonly))
            {
                ucNumRefIdx1MinusOne = 0;
            }
        }
    }

    // Override number of references used by VME. PAK uses value from DDI (num_ref_idx_l*_active_minus1_from_DDI)
    pParams->pAvcSliceParams->num_ref_idx_l0_active_minus1 = ucNumRefIdx0MinusOne;
    pParams->pAvcSliceParams->num_ref_idx_l1_active_minus1 = ucNumRefIdx1MinusOne;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::InitKernelStateWP()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    pWPKernelState = MOS_New(MHW_KERNEL_STATE);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pWPKernelState);

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_GetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize));
    
    auto pKernelStatePtr = pWPKernelState;
    CODECHAL_KERNEL_HEADER          CurrKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeGetKernelHeaderAndSize(
        kernelBinary,
        ENC_WP,
        0,
        &CurrKrnHeader,
        &kernelSize));

    pKernelStatePtr->KernelParams.iBTCount = CODECHAL_ENCODE_AVC_WP_NUM_SURFACES_G9;
    pKernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    pKernelStatePtr->KernelParams.iCurbeLength = sizeof(CODECHAL_ENCODE_AVC_WP_CURBE_G9);
    pKernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
    pKernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
    pKernelStatePtr->KernelParams.iIdCount = 1;
    pKernelStatePtr->KernelParams.iInlineDataLength = 0;

    pKernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    pKernelStatePtr->KernelParams.pBinary = kernelBinary + (CurrKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    pKernelStatePtr->KernelParams.iSize = kernelSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        pKernelStatePtr->KernelParams.iBTCount,
        &pKernelStatePtr->dwSshSize,
        &pKernelStatePtr->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_MhwInitISH(m_stateHeapInterface, pKernelStatePtr));

    return eStatus;
}


MOS_STATUS CodechalEncodeAvcEncFeiG9::GetMbEncKernelStateIdx(PCODECHAL_ENCODE_ID_OFFSET_PARAMS pParams, uint32_t* pdwKernelOffset)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pdwKernelOffset);

    *pdwKernelOffset = MBENC_I_OFFSET_CM;

    if (pParams->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_ADV)
    {
        *pdwKernelOffset += MBENC_TARGET_USAGE_CM;
    }

    if (pParams->wPictureCodingType == P_TYPE)
    {
        *pdwKernelOffset += MBENC_P_OFFSET_CM;
    }
    else if (pParams->wPictureCodingType == B_TYPE)
    {
        *pdwKernelOffset += MBENC_B_OFFSET_CM;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::SetCurbeAvcMbEnc(PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pSlcParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pdwBlockBasedSkipEn);

    auto pPicParams = pParams->pPicParams;
    auto pSeqParams = pParams->pSeqParams;
    auto pSlcParams = pParams->pSlcParams;
    auto pFeiPicParams = (CodecEncodeAvcFeiPicParams *)(m_avcFeiPicParams);

    CODECHAL_ENCODE_ASSERT(pSeqParams->TargetUsage < NUM_TARGET_USAGE_MODES);

    // set SliceQP to MAX_SLICE_QP for  MbEnc kernel, we can use it to verify whether QP is changed or not
    uint8_t SliceQP = (pParams->bUseMbEncAdvKernel && pParams->bBrcEnabled) ? CODECHAL_ENCODE_AVC_MAX_SLICE_QP : pPicParams->pic_init_qp_minus26 + 26 + pSlcParams->slice_qp_delta;
    bool bFramePicture = CodecHal_PictureIsFrame(pPicParams->CurrOriginalPic);
    bool bTopField = CodecHal_PictureIsTopField(pPicParams->CurrOriginalPic);
    bool bBottomField = CodecHal_PictureIsBottomField(pPicParams->CurrOriginalPic);
    CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9 Cmd;
    if (pParams->bMbEncIFrameDistEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &Cmd,
            sizeof(CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9),
            FEI_MBEnc_CURBE_I_frame_DIST,
            sizeof(CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9)));
    }
    else
    {
        switch (m_pictureCodingType)
        {
        case I_TYPE:
            if (bFramePicture)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &Cmd,
                    sizeof(CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9),
                    FEI_MBEnc_CURBE_normal_I_frame,
                    sizeof(CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9)));
            }
            else
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &Cmd,
                    sizeof(CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9),
                    FEI_MBEnc_CURBE_normal_I_field,
                    sizeof(CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9)));
            }
            break;

        case P_TYPE:
            if (bFramePicture)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &Cmd,
                    sizeof(CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9),
                    FEI_MBEnc_CURBE_normal_P_frame,
                    sizeof(CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9)));
            }
            else
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &Cmd,
                    sizeof(CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9),
                    FEI_MBEnc_CURBE_normal_P_field,
                    sizeof(CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9)));
            }
            break;

        case B_TYPE:
            if (bFramePicture)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &Cmd,
                    sizeof(CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9),
                    FEI_MBEnc_CURBE_normal_B_frame,
                    sizeof(CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9)));
            }
            else
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &Cmd,
                    sizeof(CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9),
                    FEI_MBEnc_CURBE_normal_B_field,
                    sizeof(CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9)));
            }
            break;

        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture coding type.");
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }
    }

    uint8_t ucMeMethod = (pFeiPicParams->SearchWindow == 5) || (pFeiPicParams->SearchWindow == 8) ? 4 : 6; // 4 means full search, 6 means diamand search
    uint32_t RefWidth = pFeiPicParams->RefWidth;
    uint32_t RefHeight = pFeiPicParams->RefHeight;
    uint32_t LenSP = pFeiPicParams->LenSP;
    switch (pFeiPicParams->SearchWindow)
    {
    case 0:
        // not use predefined search window 
        if ((pFeiPicParams->SearchPath != 0) && (pFeiPicParams->SearchPath != 1) && (pFeiPicParams->SearchPath != 2))
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI MB ENC input SearchPath for SearchWindow=0 case!!!.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
        ucMeMethod = (pFeiPicParams->SearchPath == 1) ? 6 : 4; // 4 means full search, 6 means diamand search
        if (((RefWidth * RefHeight) > 2048) || (RefWidth > 64) || (RefHeight > 64))
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI MB ENC input RefWidth/RefHeight size for SearchWindow=0 case!!!.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
        break;
    case 1:
        // Tiny - 4 SUs 24x24 window
        RefWidth = 24;
        RefHeight = 24;
        LenSP = 4;
        break;
    case 2:
        // Small - 9 SUs 28x28 window
        RefWidth = 28;
        RefHeight = 28;
        LenSP = 9;
        break;
    case 3:
        // Diamond - 16 SUs 48x40 window
        RefWidth = 48;
        RefHeight = 40;
        LenSP = 16;
        break;
    case 4:
        // Large Diamond - 32 SUs 48x40 window
        RefWidth = 48;
        RefHeight = 40;
        LenSP = 32;
        break;
    case 5:
        // Exhaustive - 48 SUs 48x40 window
        RefWidth = 48;
        RefHeight = 40;
        LenSP = 48;
        break;
    case 6:
        // Diamond - 16 SUs 64x32 window
        RefWidth = 64;
        RefHeight = 32;
        LenSP = 16;
        break;
    case 7:
        // Large Diamond - 32 SUs 64x32 window
        RefWidth = 64;
        RefHeight = 32;
        LenSP = 32;
    case 8:
        // Exhaustive - 48 SUs 64x32 window
        RefWidth = 64;
        RefHeight = 32;
        LenSP = 48;
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI MB ENC SearchWindow value!!!.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if ((m_pictureCodingType == B_TYPE) && (Cmd.DW3.BMEDisableFBR == 0))
    {
        if (RefWidth > 32)
        {
            RefWidth = 32;
        }
        if (RefHeight > 32)
        {
            RefHeight = 32;
        }
    }

    // r1
    Cmd.DW0.AdaptiveEn =
        Cmd.DW37.AdaptiveEn = pFeiPicParams->AdaptiveSearch;
    Cmd.DW0.T8x8FlagForInterEn =
        Cmd.DW37.T8x8FlagForInterEn = pPicParams->transform_8x8_mode_flag;
    Cmd.DW2.LenSP = LenSP;
    Cmd.DW38.LenSP = 0; // MBZ
    Cmd.DW2.MaxNumSU = Cmd.DW38.MaxNumSU = 57;
    Cmd.DW3.SrcAccess =
        Cmd.DW3.RefAccess = bFramePicture ? 0 : 1;
    if (m_pictureCodingType != I_TYPE && bFTQEnable)
    {
        if (pParams->pAvcQCParams && pParams->pAvcQCParams->FTQOverride)
        {
            Cmd.DW3.FTEnable = pParams->pAvcQCParams->FTQEnable;
        }
        else
        {
            if (m_pictureCodingType == P_TYPE)
            {
                Cmd.DW3.FTEnable = FTQBasedSkip[pSeqParams->TargetUsage] & 0x01;
            }
            else // B_TYPE
            {
                Cmd.DW3.FTEnable = (FTQBasedSkip[pSeqParams->TargetUsage] >> 1) & 0x01;
            }
        }
    }
    else
    {
        Cmd.DW3.FTEnable = 0;
    }
    if (pPicParams->UserFlags.bDisableSubMBPartition)
    {
        Cmd.DW3.SubMbPartMask = CODECHAL_ENCODE_AVC_DISABLE_4X4_SUB_MB_PARTITION | CODECHAL_ENCODE_AVC_DISABLE_4X8_SUB_MB_PARTITION | CODECHAL_ENCODE_AVC_DISABLE_8X4_SUB_MB_PARTITION;
    }
    Cmd.DW2.PicWidth = pParams->wPicWidthInMb;
    Cmd.DW3.SubMbPartMask = pFeiPicParams->SubMBPartMask;
    Cmd.DW3.SubPelMode = pFeiPicParams->SubPelMode;
    Cmd.DW3.InterSAD = pFeiPicParams->InterSAD;
    Cmd.DW3.IntraSAD = pFeiPicParams->IntraSAD;
    Cmd.DW3.SearchCtrl = (m_pictureCodingType == B_TYPE) ? 7 : 0;
    Cmd.DW4.PicHeightMinus1 = pParams->wFieldFrameHeightInMb - 1;
    Cmd.DW4.EnableFBRBypass = bFBRBypassEnable;
    Cmd.DW4.EnableIntraCostScalingForStaticFrame = pParams->bStaticFrameDetectionEnabled;
    Cmd.DW4.TrueDistortionEnable = pFeiPicParams->DistortionType == 0 ? 1 : 0;
    Cmd.DW4.FieldParityFlag = bBottomField;
    Cmd.DW4.bCurFldIDR = !bFramePicture && (pPicParams->bIdrPic || m_firstFieldIdrPic);
    Cmd.DW4.ConstrainedIntraPredFlag = pPicParams->constrained_intra_pred_flag;
    Cmd.DW4.HMEEnable = m_hmeEnabled;
    Cmd.DW4.PictureType = m_pictureCodingType - 1;
    Cmd.DW4.UseActualRefQPValue = 0;
    Cmd.DW4.HMEEnable = 0;
    Cmd.DW5.SliceMbHeight = pParams->usSliceHeight;
    Cmd.DW7.IntraPartMask = pFeiPicParams->IntraPartMask;
    Cmd.DW7.SrcFieldPolarity = bBottomField;

    // r2
    if (pParams->bMbEncIFrameDistEnabled)
    {
        Cmd.DW6.BatchBufferEnd = 0;
    }
    else
    {
        uint8_t ucTableIdx = m_pictureCodingType - 1;
        eStatus = MOS_SecureMemcpy(&(Cmd.ModeMvCost), 8 * sizeof(uint32_t), ModeMvCost_Cm[ucTableIdx][SliceQP], 8 * sizeof(uint32_t));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }

        if (m_pictureCodingType == I_TYPE && bOldModeCostEnable)
        {
            // Old intra mode cost needs to be used if bOldModeCostEnable is 1
            Cmd.ModeMvCost.DW8.Value = OldIntraModeCost_Cm_Common[SliceQP];
        }
        else if (m_skipBiasAdjustmentEnable)
        {
            // Load different MvCost for P picture when SkipBiasAdjustment is enabled
            // No need to check for P picture as the flag is only enabled for P picture
            Cmd.ModeMvCost.DW11.Value = MvCost_PSkipAdjustment_Cm_Common[SliceQP];
        }
    }

    if (pParams->pAvcQCParams && pParams->pAvcQCParams->FTQSkipThresholdLUTInput)
    {
        Cmd.ModeMvCost.DW14.SICFwdTransCoeffThreshold_0 = pParams->pAvcQCParams->FTQSkipThresholdLUT[SliceQP];
        Cmd.ModeMvCost.DW14.SICFwdTransCoeffThreshold_1 = pParams->pAvcQCParams->FTQSkipThresholdLUT[SliceQP];
        Cmd.ModeMvCost.DW14.SICFwdTransCoeffThreshold_2 = pParams->pAvcQCParams->FTQSkipThresholdLUT[SliceQP];
        Cmd.ModeMvCost.DW15.SICFwdTransCoeffThreshold_3 = pParams->pAvcQCParams->FTQSkipThresholdLUT[SliceQP];
        Cmd.ModeMvCost.DW15.SICFwdTransCoeffThreshold_4 = pParams->pAvcQCParams->FTQSkipThresholdLUT[SliceQP];
        Cmd.ModeMvCost.DW15.SICFwdTransCoeffThreshold_5 = pParams->pAvcQCParams->FTQSkipThresholdLUT[SliceQP];
        Cmd.ModeMvCost.DW15.SICFwdTransCoeffThreshold_6 = pParams->pAvcQCParams->FTQSkipThresholdLUT[SliceQP];
    }

    // r3 & r4
    if (pParams->bMbEncIFrameDistEnabled)
    {
        Cmd.SPDelta.DW31.IntraComputeType = 1;
    }
    else
    {
        uint8_t ucTableIdx = (m_pictureCodingType == B_TYPE) ? 1 : 0;
        eStatus = MOS_SecureMemcpy(&(Cmd.SPDelta), 16 * sizeof(uint32_t), CodechalEncoderState::m_encodeSearchPath[ucTableIdx][ucMeMethod], 16 * sizeof(uint32_t));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }

    // r5
    if (m_pictureCodingType != I_TYPE && pParams->pAvcQCParams && pParams->pAvcQCParams->NonFTQSkipThresholdLUTInput)
    {
        Cmd.DW32.SkipVal = (uint16_t)CalcSkipVal(Cmd.DW3.BlockBasedSkipEnable, pPicParams->transform_8x8_mode_flag,
            pParams->pAvcQCParams->NonFTQSkipThresholdLUT[SliceQP]);

    }
    else
    {
        if (m_pictureCodingType == P_TYPE)
        {
            Cmd.DW32.SkipVal = SkipVal_P_Common
                [Cmd.DW3.BlockBasedSkipEnable]
            [pPicParams->transform_8x8_mode_flag]
            [SliceQP];
        }
        else if (m_pictureCodingType == B_TYPE)
        {
            Cmd.DW32.SkipVal = SkipVal_B_Common
                [Cmd.DW3.BlockBasedSkipEnable]
            [pPicParams->transform_8x8_mode_flag]
            [SliceQP];
        }
    }

    Cmd.ModeMvCost.DW13.QpPrimeY = SliceQP;
    // QpPrimeCb and QpPrimeCr are not used by Kernel. Following settings are for CModel matching.
    Cmd.ModeMvCost.DW13.QpPrimeCb = SliceQP;
    Cmd.ModeMvCost.DW13.QpPrimeCr = SliceQP;
    Cmd.ModeMvCost.DW13.TargetSizeInWord = 0xff; // hardcoded for BRC disabled

    if (bMultiPredEnable && (m_pictureCodingType != I_TYPE))
    {
        Cmd.DW32.MultiPredL0Disable = pFeiPicParams->MultiPredL0 ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
        Cmd.DW32.MultiPredL1Disable = (m_pictureCodingType == B_TYPE && pFeiPicParams->MultiPredL1) ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
    }
    else
    {
        Cmd.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
        Cmd.DW32.MultiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
    }

    if (!bFramePicture)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            Cmd.DW34.List0RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_0);
            Cmd.DW34.List0RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_1);
            Cmd.DW34.List0RefID2FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_2);
            Cmd.DW34.List0RefID3FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_3);
            Cmd.DW34.List0RefID4FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_4);
            Cmd.DW34.List0RefID5FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_5);
            Cmd.DW34.List0RefID6FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_6);
            Cmd.DW34.List0RefID7FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        }
        if (m_pictureCodingType == B_TYPE)
        {
            Cmd.DW34.List1RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
            Cmd.DW34.List1RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(pSlcParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        }
    }

    if (bAdaptiveTransformDecisionEnabled)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            Cmd.DW34.EnableAdaptiveTxDecision = true;
        }

        Cmd.DW58.MBTextureThreshold = CODECHAL_ENCODE_AVC_MB_TEXTURE_THRESHOLD_G9;
        Cmd.DW58.TxDecisonThreshold = CODECHAL_ENCODE_AVC_ADAPTIVE_TX_DECISION_THRESHOLD_G9;
    }
    Cmd.DW34.EnableAdaptiveTxDecision = false;
    if (m_pictureCodingType == B_TYPE)
    {
        Cmd.DW34.List1RefID0FrameFieldFlag = GetRefPicFieldFlag(pParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        Cmd.DW34.List1RefID1FrameFieldFlag = GetRefPicFieldFlag(pParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        Cmd.DW34.bDirectMode = pSlcParams->direct_spatial_mv_pred_flag;
    }
    Cmd.DW34.bOriginalBff = bFramePicture ? 0 :
        ((m_firstField && (bBottomField)) || (!m_firstField && (!bBottomField)));
    Cmd.DW34.EnableMBFlatnessChkOptimization = m_flatnessCheckEnabled;
    Cmd.DW34.ROIEnableFlag = pParams->bRoiEnabled;
    Cmd.DW34.MADEnableFlag = m_bMadEnabled;
    Cmd.DW34.MBBrcEnable = 0;
    Cmd.DW34.ArbitraryNumMbsPerSlice = m_arbitraryNumMbsInSlice;
    Cmd.DW34.ForceNonSkipMbEnable = pParams->bMbDisableSkipMapEnabled;
    if (pParams->pAvcQCParams && !Cmd.DW34.ForceNonSkipMbEnable) // ignore DisableEncSkipCheck if Mb Disable Skip Map is available
    {
        Cmd.DW34.DisableEncSkipCheck = pParams->pAvcQCParams->skipCheckDisable;
    }
    Cmd.DW36.CheckAllFractionalEnable = bCAFEnable;
    Cmd.DW38.RefThreshold = m_refThresholdFei;
    Cmd.DW39.HMERefWindowsCombThreshold = (m_pictureCodingType == B_TYPE) ?
        HMEBCombineLen_fei[pSeqParams->TargetUsage] : HMECombineLen_fei[pSeqParams->TargetUsage];

    // Default:2 used for MBBRC (MB QP Surface width and height are 4x downscaled picture in MB unit * 4  bytes)
    // 0 used for MBQP data surface (MB QP Surface width and height are same as the input picture size in MB unit * 1bytes)
    // starting GEN9, BRC use split kernel, MB QP surface is same size as input picture
    Cmd.DW47.MbQpReadFactor = (bMbBrcEnabled || bMbQpDataEnabled) ? 0 : 2;

    // Those fields are not really used for I_dist kernel,
    // but set them to 0 to get bit-exact match with kernel prototype
    if (pParams->bMbEncIFrameDistEnabled)
    {
        Cmd.ModeMvCost.DW13.QpPrimeY = 0;
        Cmd.ModeMvCost.DW13.QpPrimeCb = 0;
        Cmd.ModeMvCost.DW13.QpPrimeCr = 0;
        Cmd.DW33.Intra16x16NonDCPredPenalty = 0;
        Cmd.DW33.Intra4x4NonDCPredPenalty = 0;
        Cmd.DW33.Intra8x8NonDCPredPenalty = 0;
    }

    //r6
    if (Cmd.DW4.UseActualRefQPValue)
    {
        Cmd.DW44.ActualQPValueForRefID0List0 = AVCGetQPValueFromRefList(pParams, LIST_0, CODECHAL_ENCODE_REF_ID_0);
        Cmd.DW44.ActualQPValueForRefID1List0 = AVCGetQPValueFromRefList(pParams, LIST_0, CODECHAL_ENCODE_REF_ID_1);
        Cmd.DW44.ActualQPValueForRefID2List0 = AVCGetQPValueFromRefList(pParams, LIST_0, CODECHAL_ENCODE_REF_ID_2);
        Cmd.DW44.ActualQPValueForRefID3List0 = AVCGetQPValueFromRefList(pParams, LIST_0, CODECHAL_ENCODE_REF_ID_3);
        Cmd.DW45.ActualQPValueForRefID4List0 = AVCGetQPValueFromRefList(pParams, LIST_0, CODECHAL_ENCODE_REF_ID_4);
        Cmd.DW45.ActualQPValueForRefID5List0 = AVCGetQPValueFromRefList(pParams, LIST_0, CODECHAL_ENCODE_REF_ID_5);
        Cmd.DW45.ActualQPValueForRefID6List0 = AVCGetQPValueFromRefList(pParams, LIST_0, CODECHAL_ENCODE_REF_ID_6);
        Cmd.DW45.ActualQPValueForRefID7List0 = AVCGetQPValueFromRefList(pParams, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        Cmd.DW46.ActualQPValueForRefID0List1 = AVCGetQPValueFromRefList(pParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        Cmd.DW46.ActualQPValueForRefID1List1 = AVCGetQPValueFromRefList(pParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);
    }

    uint8_t ucTableIdx = m_pictureCodingType - 1;
    Cmd.DW46.RefCost = RefCost_MultiRefQp_Fei[ucTableIdx][SliceQP];

    // Picture Coding Type dependent parameters
    if (m_pictureCodingType == I_TYPE)
    {
        Cmd.DW0.SkipModeEn = 0;
        Cmd.DW37.SkipModeEn = 0;
        Cmd.DW36.HMECombineOverlap = 0;
        Cmd.DW36.CheckAllFractionalEnable = 0;
        Cmd.DW47.IntraCostSF = 16; // This is not used but recommended to set this to 16 by Kernel team
        Cmd.DW34.EnableDirectBiasAdjustment = 0;
        Cmd.DW34.EnableGlobalMotionBiasAdjustment = 0;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        Cmd.DW1.MaxNumMVs = GetMaxMvsPer2Mb(pSeqParams->Level) / 2;
        Cmd.DW3.BMEDisableFBR = 1;
        Cmd.DW5.RefWidth = Cmd.DW39.RefWidth = RefWidth;
        Cmd.DW5.RefHeight = Cmd.DW39.RefHeight = RefHeight;
        Cmd.DW7.NonSkipZMvAdded = 1;
        Cmd.DW7.NonSkipModeAdded = 1;
        Cmd.DW7.SkipCenterMask = 1;
        Cmd.DW47.IntraCostSF =
            bAdaptiveIntraScalingEnable ?
            AdaptiveIntraScalingFactor_Cm_Common[SliceQP] :
            IntraScalingFactor_Cm_Common[SliceQP];
        Cmd.DW47.MaxVmvR = (bFramePicture) ? CodecHalAvcEncode_GetMaxMvLen(pSeqParams->Level) * 4 : (CodecHalAvcEncode_GetMaxMvLen(pSeqParams->Level) >> 1) * 4;
        Cmd.DW36.HMECombineOverlap = 1;
        Cmd.DW36.NumRefIdxL0MinusOne = bMultiPredEnable ? pSlcParams->num_ref_idx_l0_active_minus1 : 0;
        Cmd.DW36.CheckAllFractionalEnable = pFeiPicParams->RepartitionCheckEnable;
        Cmd.DW34.EnableDirectBiasAdjustment = 0;
        if (pParams->pAvcQCParams)
        {
            Cmd.DW34.EnableGlobalMotionBiasAdjustment = pParams->pAvcQCParams->globalMotionBiasAdjustmentEnable;
            if (Cmd.DW34.EnableGlobalMotionBiasAdjustment)
            {
                Cmd.DW59.HMEMVCostScalingFactor = pParams->pAvcQCParams->HMEMVCostScalingFactor;
            }
        }

        Cmd.DW64.NumMVPredictorsL0 = pFeiPicParams->NumMVPredictorsL0;
        Cmd.DW64.NumMVPredictorsL1 = 0;
    }
    else
    {
        // B_TYPE
        Cmd.DW1.MaxNumMVs = GetMaxMvsPer2Mb(pSeqParams->Level) / 2;
        Cmd.DW1.BiWeight = m_biWeight;
        Cmd.DW3.SearchCtrl = 7;
        Cmd.DW3.SkipType = 1;
        Cmd.DW5.RefWidth = Cmd.DW39.RefWidth = RefWidth;
        Cmd.DW5.RefHeight = Cmd.DW39.RefHeight = RefHeight;
        Cmd.DW7.SkipCenterMask = 0xFF;
        Cmd.DW47.IntraCostSF =
            bAdaptiveIntraScalingEnable ?
            AdaptiveIntraScalingFactor_Cm_Common[SliceQP] :
            IntraScalingFactor_Cm_Common[SliceQP];
        Cmd.DW47.MaxVmvR = (bFramePicture) ? CodecHalAvcEncode_GetMaxMvLen(pSeqParams->Level) * 4 : (CodecHalAvcEncode_GetMaxMvLen(pSeqParams->Level) >> 1) * 4;
        Cmd.DW36.HMECombineOverlap = 1;
        // Checking if the forward frame (List 1 index 0) is a short term reference
        {
            auto CodecHalPic = pParams->pSlcParams->RefPicList[LIST_1][0];
            if (CodecHalPic.PicFlags != PICTURE_INVALID &&
                CodecHalPic.FrameIdx != CODECHAL_ENCODE_AVC_INVALID_PIC_ID &&
                pParams->pPicIdx[CodecHalPic.FrameIdx].bValid)
            {
                // Although its name is FWD, it actually means the future frame or the backward reference frame
                Cmd.DW36.IsFwdFrameShortTermRef = CodecHal_PictureIsShortTermRef(pParams->pPicParams->RefFrameList[CodecHalPic.FrameIdx]);
            }
            else
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Invalid backward reference frame.");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
        }
        Cmd.DW36.NumRefIdxL0MinusOne = bMultiPredEnable ? pSlcParams->num_ref_idx_l0_active_minus1 : 0;
        Cmd.DW36.NumRefIdxL1MinusOne = bMultiPredEnable ? pSlcParams->num_ref_idx_l1_active_minus1 : 0;
        Cmd.DW36.CheckAllFractionalEnable = pFeiPicParams->RepartitionCheckEnable;
        Cmd.DW40.DistScaleFactorRefID0List0 = m_distScaleFactorList0[0];
        Cmd.DW40.DistScaleFactorRefID1List0 = m_distScaleFactorList0[1];
        Cmd.DW41.DistScaleFactorRefID2List0 = m_distScaleFactorList0[2];
        Cmd.DW41.DistScaleFactorRefID3List0 = m_distScaleFactorList0[3];
        Cmd.DW42.DistScaleFactorRefID4List0 = m_distScaleFactorList0[4];
        Cmd.DW42.DistScaleFactorRefID5List0 = m_distScaleFactorList0[5];
        Cmd.DW43.DistScaleFactorRefID6List0 = m_distScaleFactorList0[6];
        Cmd.DW43.DistScaleFactorRefID7List0 = m_distScaleFactorList0[7];
        if (pParams->pAvcQCParams)
        {
            Cmd.DW34.EnableDirectBiasAdjustment = pParams->pAvcQCParams->directBiasAdjustmentEnable;
            if (Cmd.DW34.EnableDirectBiasAdjustment)
            {
                Cmd.DW7.NonSkipModeAdded = 1;
                Cmd.DW7.NonSkipZMvAdded = 1;
            }

            Cmd.DW34.EnableGlobalMotionBiasAdjustment = pParams->pAvcQCParams->globalMotionBiasAdjustmentEnable;
            if (Cmd.DW34.EnableGlobalMotionBiasAdjustment)
            {
                Cmd.DW59.HMEMVCostScalingFactor = pParams->pAvcQCParams->HMEMVCostScalingFactor;
            }
        }

        Cmd.DW64.NumMVPredictorsL0 = pFeiPicParams->NumMVPredictorsL0;
        Cmd.DW64.NumMVPredictorsL1 = pFeiPicParams->NumMVPredictorsL1;

        CODEC_PICTURE    RefPic;
        RefPic = pSlcParams->RefPicList[LIST_1][0];
        Cmd.DW64.L1ListRef0PictureCodingType = m_refList[m_picIdx[RefPic.FrameIdx].ucPicIdx]->ucAvcPictureCodingType;
        if(bFramePicture && ((Cmd.DW64.L1ListRef0PictureCodingType == CODEC_AVC_PIC_CODING_TYPE_TFF_FIELD) || (Cmd.DW64.L1ListRef0PictureCodingType == CODEC_AVC_PIC_CODING_TYPE_BFF_FIELD)))
        {
            uint16_t wFieldHeightInMb = (pParams->wFieldFrameHeightInMb + 1) >> 1;
            Cmd.DW66.BottomFieldOffsetL1ListRef0MV     = MOS_ALIGN_CEIL(wFieldHeightInMb * pParams->wPicWidthInMb * (32 * 4), 0x1000);
            Cmd.DW67.BottomFieldOffsetL1ListRef0MBCode = wFieldHeightInMb * pParams->wPicWidthInMb * 64;
        }
    }

    *pParams->pdwBlockBasedSkipEn = Cmd.DW3.BlockBasedSkipEnable;

    if (pPicParams->EnableRollingIntraRefresh)
    {
        Cmd.DW34.IntraRefreshEn = pPicParams->EnableRollingIntraRefresh;

        /* Multiple predictor should be completely disabled for the RollingI feature. This does not lead to much quality drop for P frames especially for TU as 1 */
        Cmd.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;

        /* Pass the same IntraRefreshUnit to the kernel w/o the adjustment by -1, so as to have an overlap of one MB row or column of Intra macroblocks
        across one P frame to another P frame, as needed by the RollingI algo */
        Cmd.DW48.IntraRefreshMBNum = pPicParams->IntraRefreshMBNum; /* MB row or column number */
        Cmd.DW48.IntraRefreshUnitInMBMinus1 = pPicParams->IntraRefreshUnitinMB;
        Cmd.DW48.IntraRefreshQPDelta = pPicParams->IntraRefreshQPDelta;
    }
    else
    {
        Cmd.DW34.IntraRefreshEn = 0;
    }

    Cmd.DW34.EnablePerMBStaticCheck = pParams->bStaticFrameDetectionEnabled;
    Cmd.DW34.EnableAdaptiveSearchWindowSize = pParams->bApdatvieSearchWindowSizeEnabled;

    if (true == pParams->bRoiEnabled)
    {
        Cmd.DW49.ROI1_X_left = pPicParams->ROI[0].Left;
        Cmd.DW49.ROI1_Y_top = pPicParams->ROI[0].Top;
        Cmd.DW50.ROI1_X_right = pPicParams->ROI[0].Right;
        Cmd.DW50.ROI1_Y_bottom = pPicParams->ROI[0].Bottom;

        Cmd.DW51.ROI2_X_left = pPicParams->ROI[1].Left;
        Cmd.DW51.ROI2_Y_top = pPicParams->ROI[1].Top;
        Cmd.DW52.ROI2_X_right = pPicParams->ROI[1].Right;
        Cmd.DW52.ROI2_Y_bottom = pPicParams->ROI[1].Bottom;

        Cmd.DW53.ROI3_X_left = pPicParams->ROI[2].Left;
        Cmd.DW53.ROI3_Y_top = pPicParams->ROI[2].Top;
        Cmd.DW54.ROI3_X_right = pPicParams->ROI[2].Right;
        Cmd.DW54.ROI3_Y_bottom = pPicParams->ROI[2].Bottom;

        Cmd.DW55.ROI4_X_left = pPicParams->ROI[3].Left;
        Cmd.DW55.ROI4_Y_top = pPicParams->ROI[3].Top;
        Cmd.DW56.ROI4_X_right = pPicParams->ROI[3].Right;
        Cmd.DW56.ROI4_Y_bottom = pPicParams->ROI[3].Bottom;

        if (bBrcEnabled == false)
        {
            uint16_t numROI = pPicParams->NumROI;
            char priorityLevelOrDQp[CODECHAL_ENCODE_AVC_MAX_ROI_NUMBER] = { 0 };

            // cqp case
            for (unsigned int i = 0; i < numROI; i += 1)
            {
                char dQpRoi = pPicParams->ROI[i].PriorityLevelOrDQp;

                // clip qp roi in order to have (qp + qpY) in range [0, 51]
                priorityLevelOrDQp[i] = (char)CodecHal_Clip3(-SliceQP, CODECHAL_ENCODE_AVC_MAX_SLICE_QP - SliceQP, dQpRoi);
            }

            Cmd.DW57.ROI1_dQpPrimeY = priorityLevelOrDQp[0];
            Cmd.DW57.ROI2_dQpPrimeY = priorityLevelOrDQp[1];
            Cmd.DW57.ROI3_dQpPrimeY = priorityLevelOrDQp[2];
            Cmd.DW57.ROI4_dQpPrimeY = priorityLevelOrDQp[3];
        }
        else
        {
            // kernel does not support BRC case
            Cmd.DW34.ROIEnableFlag = 0;
        }
    }
    else if (pParams->bDirtyRoiEnabled)
    {
        // enable Dirty Rect flag
        Cmd.DW4.EnableDirtyRect = true;

        Cmd.DW49.ROI1_X_left = pParams->pPicParams->DirtyROI[0].Left;
        Cmd.DW49.ROI1_Y_top = pParams->pPicParams->DirtyROI[0].Top;
        Cmd.DW50.ROI1_X_right = pParams->pPicParams->DirtyROI[0].Right;
        Cmd.DW50.ROI1_Y_bottom = pParams->pPicParams->DirtyROI[0].Bottom;

        Cmd.DW51.ROI2_X_left = pParams->pPicParams->DirtyROI[1].Left;
        Cmd.DW51.ROI2_Y_top = pParams->pPicParams->DirtyROI[1].Top;
        Cmd.DW52.ROI2_X_right = pParams->pPicParams->DirtyROI[1].Right;
        Cmd.DW52.ROI2_Y_bottom = pParams->pPicParams->DirtyROI[1].Bottom;

        Cmd.DW53.ROI3_X_left = pParams->pPicParams->DirtyROI[2].Left;
        Cmd.DW53.ROI3_Y_top = pParams->pPicParams->DirtyROI[2].Top;
        Cmd.DW54.ROI3_X_right = pParams->pPicParams->DirtyROI[2].Right;
        Cmd.DW54.ROI3_Y_bottom = pParams->pPicParams->DirtyROI[2].Bottom;

        Cmd.DW55.ROI4_X_left = pParams->pPicParams->DirtyROI[3].Left;
        Cmd.DW55.ROI4_Y_top = pParams->pPicParams->DirtyROI[3].Top;
        Cmd.DW56.ROI4_X_right = pParams->pPicParams->DirtyROI[3].Right;
        Cmd.DW56.ROI4_Y_bottom = pParams->pPicParams->DirtyROI[3].Bottom;
    }

    //IPCM QP and threshold
    Cmd.DW60.IPCM_QP0                       = IPCM_Threshold_Table[0].QP;
    Cmd.DW60.IPCM_QP1                       = IPCM_Threshold_Table[1].QP;
    Cmd.DW60.IPCM_QP2                       = IPCM_Threshold_Table[2].QP;
    Cmd.DW60.IPCM_QP3                       = IPCM_Threshold_Table[3].QP;
    Cmd.DW61.IPCM_QP4                       = IPCM_Threshold_Table[4].QP;

    Cmd.DW61.IPCM_Thresh0                   = IPCM_Threshold_Table[0].Threshold;
    Cmd.DW62.IPCM_Thresh1                   = IPCM_Threshold_Table[1].Threshold;
    Cmd.DW62.IPCM_Thresh2                   = IPCM_Threshold_Table[2].Threshold;
    Cmd.DW63.IPCM_Thresh3                   = IPCM_Threshold_Table[3].Threshold;
    Cmd.DW63.IPCM_Thresh4                   = IPCM_Threshold_Table[4].Threshold;

    // r64: FEI specific fields
    Cmd.DW64.FEIEnable = 1;
    Cmd.DW64.MultipleMVPredictorPerMBEnable = pFeiPicParams->MVPredictorEnable;
    Cmd.DW64.VMEDistortionOutputEnable = pFeiPicParams->DistortionEnable;
    Cmd.DW64.PerMBQpEnable = pFeiPicParams->bMBQp;
    Cmd.DW64.MBInputEnable = pFeiPicParams->bPerMBInput;

    // FEI mode is disabled when external MVP is available
    if (pFeiPicParams->MVPredictorEnable)
        Cmd.DW64.FEIMode = 0;
    else
        Cmd.DW64.FEIMode = 1;

    if (IsMfeMbEncEnabled())
    {
        MOS_LOCK_PARAMS LockFlagsWriteOnly;
        MOS_ZeroMemory(&LockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
        LockFlagsWriteOnly.WriteOnly = 1;

        uint8_t* pData = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &(BrcBuffers.resMbEncBrcBuffer),
            &LockFlagsWriteOnly);
        CODECHAL_ENCODE_CHK_NULL_RETURN(pData);

        MOS_SecureMemcpy(pData, m_mbencCurbeDataSizeFei, (void *)&Cmd, m_mbencCurbeDataSizeFei);

        m_osInterface->pfnUnlockResource(m_osInterface, &BrcBuffers.resMbEncBrcBuffer);
        return eStatus;
    }

    Cmd.DW80.MBDataSurfIndex = CODECHAL_ENCODE_AVC_MBENC_MFC_AVC_PAK_OBJ_G9;
    Cmd.DW81.MVDataSurfIndex = CODECHAL_ENCODE_AVC_MBENC_IND_MV_DATA_G9;
    Cmd.DW82.IDistSurfIndex = CODECHAL_ENCODE_AVC_MBENC_BRC_DISTORTION_G9;
    Cmd.DW83.SrcYSurfIndex = CODECHAL_ENCODE_AVC_MBENC_CURR_Y_G9;
    Cmd.DW84.MBSpecificDataSurfIndex = CODECHAL_ENCODE_AVC_MBENC_MB_SPECIFIC_DATA_G9;
    Cmd.DW85.AuxVmeOutSurfIndex = CODECHAL_ENCODE_AVC_MBENC_AUX_VME_OUT_G9;
    Cmd.DW86.CurrRefPicSelSurfIndex = CODECHAL_ENCODE_AVC_MBENC_REFPICSELECT_L0_G9;
    Cmd.DW87.HMEMVPredFwdBwdSurfIndex = CODECHAL_ENCODE_AVC_MBENC_MV_DATA_FROM_ME_G9;
    Cmd.DW88.HMEDistSurfIndex = CODECHAL_ENCODE_AVC_MBENC_4xME_DISTORTION_G9;
    Cmd.DW89.SliceMapSurfIndex = CODECHAL_ENCODE_AVC_MBENC_SLICEMAP_DATA_G9;
    Cmd.DW90.FwdFrmMBDataSurfIndex = CODECHAL_ENCODE_AVC_MBENC_FWD_MB_DATA_G9;
    Cmd.DW91.FwdFrmMVSurfIndex = CODECHAL_ENCODE_AVC_MBENC_FWD_MV_DATA_G9;
    Cmd.DW92.MBQPBuffer = CODECHAL_ENCODE_AVC_MBENC_MBQP_G9;
    Cmd.DW93.MBBRCLut = CODECHAL_ENCODE_AVC_MBENC_MBBRC_CONST_DATA_G9;
    Cmd.DW94.VMEInterPredictionSurfIndex = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_G9;
    Cmd.DW95.VMEInterPredictionMRSurfIndex = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_G9;
    Cmd.DW96.MbStatsSurfIndex = CODECHAL_ENCODE_AVC_MBENC_MB_STATS_G9;
    Cmd.DW97.MADSurfIndex = CODECHAL_ENCODE_AVC_MBENC_MAD_DATA_G9;
    Cmd.DW98.ForceNonSkipMBmapSurface = CODECHAL_ENCODE_AVC_MBENC_FORCE_NONSKIP_MB_MAP_G9;
    Cmd.DW99.ReservedIndex = CODECHAL_ENCODE_AVC_MBENC_ADV_WA_G9;
    Cmd.DW100.BRCCurbeSurfIndex = CODECHAL_ENCODE_AVC_MBENC_BRC_CURBE_DATA_G9;
    Cmd.DW101.StaticDetectionCostTableIndex = CODECHAL_ENCODE_AVC_MBENC_SFD_COST_TABLE_G9;
    Cmd.DW102.FEIMVPredictorSurfIndex = CODECHAL_ENCODE_AVC_MBENC_MV_PREDICTOR_G9;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pParams->pKernelState->m_dshRegion.AddData(
        &Cmd,
        pParams->pKernelState->dwCurbeOffset,
        sizeof(Cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::SetCurbeAvcPreProc(PCODECHAL_ENCODE_AVC_PREPROC_CURBE_PARAMS pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pPreEncParams);

    auto pPreEncParams = pParams->pPreEncParams;

    uint32_t SliceQP = pPreEncParams->dwFrameQp;
    bool bFramePicture = CodecHal_PictureIsFrame(m_currOriginalPic);
    bool bBottomField = CodecHal_PictureIsBottomField(m_currOriginalPic);
    CODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9 Cmd;
    switch (m_pictureCodingType)
    {
    case I_TYPE:
        if (bFramePicture)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &Cmd,
                sizeof(CODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9),
                PreProc_CURBE_CM_normal_I_frame,
                sizeof(CODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9)));
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &Cmd,
                sizeof(CODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9),
                PreProc_CURBE_CM_normal_I_field,
                sizeof(CODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9)));
        }
        break;

    case P_TYPE:
        if (bFramePicture)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &Cmd,
                sizeof(CODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9),
                PreProc_CURBE_CM_normal_P_frame,
                sizeof(CODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9)));
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &Cmd,
                sizeof(CODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9),
                PreProc_CURBE_CM_normal_P_field,
                sizeof(CODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9)));
        }
        break;

    case B_TYPE:
        if (bFramePicture)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &Cmd,
                sizeof(CODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9),
                PreProc_CURBE_CM_normal_B_frame,
                sizeof(CODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9)));
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &Cmd,
                sizeof(CODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9),
                PreProc_CURBE_CM_normal_B_field,
                sizeof(CODECHAL_ENCODE_AVC_PREPROC_CURBE_CM_G9)));
        }
        break;

    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture coding type.");
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    uint8_t  ucMeMethod = (pPreEncParams->dwSearchWindow == 5) || (pPreEncParams->dwSearchWindow == 8) ? 4 : 6; // 4 means full search, 6 means diamand search
    uint32_t RefWidth = pPreEncParams->dwRefWidth;
    uint32_t RefHeight = pPreEncParams->dwRefHeight;
    uint32_t LenSP = pPreEncParams->dwLenSP;
    switch (pPreEncParams->dwSearchWindow)
    {
    case 0:
        // not use predefined search window and search patch
        if ((pPreEncParams->dwSearchPath != 0) && (pPreEncParams->dwSearchPath != 1) && (pPreEncParams->dwSearchPath != 2))
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI PREENC input SearchPath for SearchWindow=0 case!!!.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
        ucMeMethod = (pPreEncParams->dwSearchPath == 1) ? 6 : 4; // 4 means full search, 6 means diamand search
        if (((RefWidth * RefHeight) > 2048) || (RefWidth > 64) || (RefHeight > 64))
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI PREENC PreProc input RefWidth/RefHeight size for SearchWindow=0 case!!!.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
        break;
    case 1:
        // Tiny 4 SUs 24x24 window
        RefWidth = 24;
        RefHeight = 24;
        LenSP = 4;
        break;
    case 2:
        // Small 9 SUs 28x28 window
        RefWidth = 28;
        RefHeight = 28;
        LenSP = 9;
        break;
    case 3:
        // Diamond 16 SUs 48x40 window
        RefWidth = 48;
        RefHeight = 40;
        LenSP = 16;
        break;
    case 4:
        // Large Diamond 32 SUs 48x40 window
        RefWidth = 48;
        RefHeight = 40;
        LenSP = 32;
        break;
    case 5:
        // Exhaustive 48 SUs 48x40 window
        RefWidth = 48;
        RefHeight = 40;
        LenSP = 48;
        break;
    case 6:
        // Diamond 16 SUs 64x32 window
        RefWidth = 64;
        RefHeight = 32;
        LenSP = 16;
        break;
    case 7:
        // Large Diamond 32 SUs 64x32 window
        RefWidth = 64;
        RefHeight = 32;
        LenSP = 32;
    case 8:
        // Exhaustive 48 SUs 64x32 window
        RefWidth = 64;
        RefHeight = 32;
        LenSP = 48;
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture FEI PREENC PreProc SearchWindow value!!!.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    // r1
    Cmd.DW0.AdaptiveEn =
        Cmd.DW37.AdaptiveEn = pPreEncParams->bAdaptiveSearch;
    Cmd.DW2.LenSP = LenSP;
    Cmd.DW38.LenSP = 0; // MBZ
    Cmd.DW2.MaxNumSU = Cmd.DW38.MaxNumSU = 57;

    Cmd.DW3.SrcAccess =
        Cmd.DW3.RefAccess = bFramePicture ? 0 : 1;

    if (m_pictureCodingType != I_TYPE && bFTQEnable)
    {
        Cmd.DW3.FTEnable = pPreEncParams->bFTEnable;
    }
    else
    {
        Cmd.DW3.FTEnable = 0;
    }
    Cmd.DW3.SubMbPartMask = pPreEncParams->dwSubMBPartMask;
    Cmd.DW3.SubPelMode = pPreEncParams->dwSubPelMode;
    Cmd.DW3.IntraSAD = pPreEncParams->dwIntraSAD;
    Cmd.DW3.InterSAD = pPreEncParams->dwInterSAD;

    Cmd.DW2.PicWidth = pParams->wPicWidthInMb;
    Cmd.DW6.PicHeight = Cmd.DW5.SliceMbHeight = pParams->wFieldFrameHeightInMb;

    Cmd.DW4.FieldParityFlag = Cmd.DW4.CurrPicFieldParityFlag =
        Cmd.DW7.SrcFieldPolarity = bBottomField ? 1 : 0;
    Cmd.DW4.HMEEnable = m_hmeEnabled;
    Cmd.DW4.FrameQp = SliceQP;
    Cmd.DW4.PerMBQpEnable = pPreEncParams->bMBQp;
    Cmd.DW4.MultipleMVPredictorPerMBEnable = (m_pictureCodingType == I_TYPE) ? 0 : pPreEncParams->dwMVPredictorCtrl;
    Cmd.DW4.DisableMvOutput = (m_pictureCodingType == I_TYPE) ? 1 : pPreEncParams->bDisableMVOutput;
    Cmd.DW4.DisableMbStats = pPreEncParams->bDisableStatisticsOutput;
    Cmd.DW4.FwdRefPicEnable = pPreEncParams->dwNumPastReferences > 0 ? 1 : 0;
    Cmd.DW4.FwdRefPicFrameFieldFlag = CodecHal_PictureIsFrame(pPreEncParams->PastRefPicture) ? 0 : 1;
    Cmd.DW4.FwdRefPicFieldParityFlag = CodecHal_PictureIsBottomField(pPreEncParams->PastRefPicture);
    Cmd.DW4.BwdRefPicEnable = pPreEncParams->dwNumFutureReferences > 0 ? 1 : 0;
    Cmd.DW4.BwdRefPicFrameFieldFlag = CodecHal_PictureIsFrame(pPreEncParams->FutureRefPicture) ? 0 : 1;
    Cmd.DW4.BwdRefPicFieldParityFlag = CodecHal_PictureIsBottomField(pPreEncParams->FutureRefPicture);
    Cmd.DW7.IntraPartMask = pPreEncParams->dwIntraPartMask;

    uint8_t ucTableIdx = m_pictureCodingType - 1;
    eStatus = MOS_SecureMemcpy(&(Cmd.ModeMvCost), 8 * sizeof(uint32_t), ModeMvCost_Cm_PreProc[ucTableIdx][SliceQP], 8 * sizeof(uint32_t));
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }
    Cmd.ModeMvCost.DW8.Value = 0;
    Cmd.ModeMvCost.DW9.Value = 0;
    Cmd.ModeMvCost.DW10.Value = 0;
    Cmd.ModeMvCost.DW11.Value = 0;
    Cmd.ModeMvCost.DW12.Value = 0;
    Cmd.ModeMvCost.DW13.Value = 0;

    if (!bFramePicture && m_pictureCodingType != I_TYPE)
    {
        /* for field, R2.2 upper word should be set to zero for P and B frames */
        Cmd.ModeMvCost.DW10.Value &= 0x0000FFFF;
    }

    ucTableIdx = (m_pictureCodingType == B_TYPE) ? 1 : 0;
    eStatus = MOS_SecureMemcpy(&(Cmd.SPDelta), 16 * sizeof(uint32_t), CodechalEncoderState::m_encodeSearchPath[ucTableIdx][ucMeMethod], 16 * sizeof(uint32_t));
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }
    // disable Intra Compute when disabling all mode in dwIntraPartMask
    if (pPreEncParams->dwIntraPartMask == 0x7)
    {
        Cmd.SPDelta.DW31.IntraComputeType = 3;
    }

    Cmd.DW38.RefThreshold = m_refThresholdFei;
    Cmd.DW39.HMERefWindowsCombThreshold = (m_pictureCodingType == B_TYPE) ?
        HMEBCombineLen_fei[m_targetUsage] : HMECombineLen_fei[m_targetUsage];

    // Picture Coding Type dependent parameters
    if (m_pictureCodingType == I_TYPE)
    {
        Cmd.DW0.SkipModeEn = 0;
        Cmd.DW37.SkipModeEn = 0;
        Cmd.DW36.HMECombineOverlap = 0;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        Cmd.DW1.MaxNumMVs = GetMaxMvsPer2Mb(CODECHAL_AVC_LEVEL_52) / 2;
        Cmd.DW3.BMEDisableFBR = 1;
        Cmd.DW3.SearchCtrl = 0;
        Cmd.DW5.RefWidth = Cmd.DW39.RefWidth = RefWidth;
        Cmd.DW5.RefHeight = Cmd.DW39.RefHeight = RefHeight;
        Cmd.DW7.NonSkipZMvAdded = 1;
        Cmd.DW7.NonSkipModeAdded = 1;
        Cmd.DW7.SkipCenterMask = 1;
        Cmd.DW32.MaxVmvR = (bFramePicture) ? CodecHalAvcEncode_GetMaxMvLen(CODECHAL_AVC_LEVEL_52) * 4 : (CodecHalAvcEncode_GetMaxMvLen(CODECHAL_AVC_LEVEL_52) >> 1) * 4;
        Cmd.DW36.HMECombineOverlap = 1;
    }
    else
    {
        // B_TYPE
        Cmd.DW1.MaxNumMVs = GetMaxMvsPer2Mb(CODECHAL_AVC_LEVEL_52) / 2;
        Cmd.DW3.SearchCtrl = 0;
        Cmd.DW3.SkipType = 1;
        Cmd.DW5.RefWidth = Cmd.DW39.RefWidth = RefWidth;
        Cmd.DW5.RefHeight = Cmd.DW39.RefHeight = RefHeight;
        Cmd.DW7.SkipCenterMask = 0xFF;
        Cmd.DW32.MaxVmvR = (bFramePicture) ? CodecHalAvcEncode_GetMaxMvLen(CODECHAL_AVC_LEVEL_52) * 4 : (CodecHalAvcEncode_GetMaxMvLen(CODECHAL_AVC_LEVEL_52) >> 1) * 4;
        Cmd.DW36.HMECombineOverlap = 1;
    }
    if(pParams->pCurbeBinary)
    {
        MOS_SecureMemcpy(pParams->pCurbeBinary,m_preProcCurbeDataSizeFei,&Cmd,m_preProcCurbeDataSizeFei);
        return eStatus;
    }

    Cmd.DW40.CurrPicSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_CURR_Y_CM_G9;
    Cmd.DW41.HMEMvDataSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_HME_MV_DATA_CM_G9;
    Cmd.DW42.MvPredictorSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_MV_PREDICTOR_CM_G9;
    Cmd.DW43.MbQpSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_MBQP_CM_G9;
    Cmd.DW44.MvDataOutSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_MV_DATA_CM_G9;
    Cmd.DW45.MbStatsOutSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_MB_STATS_CM_G9;
    if (bFramePicture)
    {
        Cmd.DW46.VMEInterPredictionSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_VME_CURR_PIC_IDX_0_CM_G9;
        Cmd.DW47.VMEInterPredictionMRSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_VME_CURR_PIC_IDX_1_CM_G9;
        Cmd.DW48.FtqLutSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_FTQ_LUT_CM_G9;
    }
    else
    {
        Cmd.DW46.VMEInterPredictionSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_CURR_PIC_IDX_0_CM_G9;
        Cmd.DW47.VMEInterPredictionMRSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_VME_FIELD_CURR_PIC_IDX_1_CM_G9;
        Cmd.DW48.FtqLutSurfIndex = CODECHAL_ENCODE_AVC_PREPROC_FIELD_FTQ_LUT_CM_G9;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pParams->pKernelState->m_dshRegion.AddData(
        &Cmd,
        pParams->pKernelState->dwCurbeOffset,
        sizeof(Cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::SendAvcPreProcSurfaces(PMOS_COMMAND_BUFFER pCmdBuffer, PCODECHAL_ENCODE_AVC_PREPROC_SURFACE_PARAMS pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetOsInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pCurrOriginalPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->psCurrPicSurface);

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pPreProcBindingTable);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pPreEncParams);

    auto pKernelState = pParams->pKernelState;

    auto pPreEncParams = (FeiPreEncParams*)pParams->pPreEncParams;
    auto pAvcPreProcBindingTable = pParams->pPreProcBindingTable;

    bool bCurrFieldPicture = CodecHal_PictureIsField(*(pParams->pCurrOriginalPic)) ? 1 : 0;
    bool bCurrBottomField = CodecHal_PictureIsBottomField(*(pParams->pCurrOriginalPic)) ? 1 : 0;

    uint8_t ucVDirection = (CodecHal_PictureIsFrame(*(pParams->pCurrOriginalPic))) ? CODECHAL_VDIRECTION_FRAME :
        (bCurrBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD;

    CODECHAL_SURFACE_CODEC_PARAMS SurfaceCodecParams;
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.bIs2DSurface = true;
    SurfaceCodecParams.bMediaBlockRW = true; // Use media block RW for DP 2D surface access
    SurfaceCodecParams.bUseUVPlane = true;
    SurfaceCodecParams.psSurface = pParams->psCurrPicSurface;
    SurfaceCodecParams.dwOffset = 0;
    SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    SurfaceCodecParams.dwBindingTableOffset = pAvcPreProcBindingTable->dwAvcPreProcCurrY;
    SurfaceCodecParams.dwUVBindingTableOffset = pAvcPreProcBindingTable->dwAvcPreProcCurrUV;
    SurfaceCodecParams.dwVerticalLineStride = pParams->dwVerticalLineStride;
    SurfaceCodecParams.dwVerticalLineStrideOffset = pParams->dwVerticalLineStrideOffset;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    // AVC_ME MV data buffer
    if (pParams->bHmeEnabled)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->ps4xMeMvDataBuffer);

        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bIs2DSurface = true;
        SurfaceCodecParams.bMediaBlockRW = true;
        SurfaceCodecParams.psSurface = pParams->ps4xMeMvDataBuffer;
        SurfaceCodecParams.dwOffset = pParams->dwMeMvBottomFieldOffset;
        SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        SurfaceCodecParams.dwBindingTableOffset = pAvcPreProcBindingTable->dwAvcPreProcMVDataFromHME;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    if (pPreEncParams->dwMVPredictorCtrl)
    {
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));

        uint32_t dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 8;
        SurfaceCodecParams.dwSize = dwSize;
        SurfaceCodecParams.presBuffer = &(pPreEncParams->resMvPredBuffer);
        SurfaceCodecParams.dwOffset = 0;
        SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        SurfaceCodecParams.dwBindingTableOffset = pAvcPreProcBindingTable->dwAvcPreProcMvPredictor;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    if (pPreEncParams->bMBQp)
    {
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));

        uint32_t dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb;
        SurfaceCodecParams.dwSize = dwSize;
        SurfaceCodecParams.presBuffer = &(pPreEncParams->resMbQpBuffer);
        SurfaceCodecParams.dwOffset = 0;
        SurfaceCodecParams.dwBindingTableOffset = pAvcPreProcBindingTable->dwAvcPreProcMbQp;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));

        // 16 DWs per QP value
        dwSize = 16 * 52 * sizeof(uint32_t);

        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.presBuffer = pParams->presFtqLutBuffer;
        SurfaceCodecParams.dwSize = dwSize;
        SurfaceCodecParams.dwBindingTableOffset = bCurrFieldPicture ? pAvcPreProcBindingTable->dwAvcPreProcFtqLutField : pAvcPreProcBindingTable->dwAvcPreProcFtqLut;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));

    }

    if (!pPreEncParams->bDisableMVOutput)
    {
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));

        uint32_t dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 32 * 4;
        SurfaceCodecParams.presBuffer = &(pPreEncParams->resMvBuffer);
        SurfaceCodecParams.dwSize = dwSize;
        SurfaceCodecParams.dwOffset = 0;
        SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        SurfaceCodecParams.dwBindingTableOffset = pAvcPreProcBindingTable->dwAvcPreProcMvDataOut;
        SurfaceCodecParams.bRenderTarget = true;
        SurfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    if (!pPreEncParams->bDisableStatisticsOutput)
    {
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));

        uint32_t dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 64;
        if (bCurrBottomField)
        {
            SurfaceCodecParams.presBuffer = &(pPreEncParams->resStatsBotFieldBuffer);
        }
        else
        {
            SurfaceCodecParams.presBuffer = &(pPreEncParams->resStatsBuffer);
        }
        SurfaceCodecParams.dwSize = dwSize;
        SurfaceCodecParams.dwOffset = pParams->dwMBVProcStatsBottomFieldOffset;
        SurfaceCodecParams.bRenderTarget = true;
        SurfaceCodecParams.bIsWritable = true;
        SurfaceCodecParams.dwBindingTableOffset = pAvcPreProcBindingTable->dwAvcPreProcMbStatsOut;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    // Current Picture Y - VME
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.bUseAdvState = true;
    SurfaceCodecParams.psSurface = pParams->psCurrPicSurface;
    SurfaceCodecParams.dwOffset = 0;
    SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    SurfaceCodecParams.dwBindingTableOffset = bCurrFieldPicture ? pAvcPreProcBindingTable->dwAvcPreProcVMECurrPicField[0] : pAvcPreProcBindingTable->dwAvcPreProcVMECurrPicFrame[0];
    SurfaceCodecParams.ucVDirection = ucVDirection;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    SurfaceCodecParams.dwBindingTableOffset = bCurrFieldPicture ? pAvcPreProcBindingTable->dwAvcPreProcVMECurrPicField[1] : pAvcPreProcBindingTable->dwAvcPreProcVMECurrPicFrame[1];
    SurfaceCodecParams.ucVDirection = ucVDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    uint32_t dwRefBindingTableOffset;
    if (pPreEncParams->dwNumPastReferences > 0)
    {
        auto RefPic = pPreEncParams->PastRefPicture;
        uint8_t ucRefPicIdx = RefPic.FrameIdx;
        bool bRefBottomField = (CodecHal_PictureIsBottomField(RefPic)) ? 1 : 0;
        uint8_t ucRefVDirection;
        // Program the surface based on current picture's field/frame mode
        if (bCurrFieldPicture) // if current picture is field
        {
            if (bRefBottomField)
            {
                ucRefVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
            }
            else
            {
                ucRefVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
            }
            dwRefBindingTableOffset = pAvcPreProcBindingTable->dwAvcPreProcVMEFwdPicField[0];
        }
        else // if current picture is frame
        {
            ucRefVDirection = CODECHAL_VDIRECTION_FRAME;
            dwRefBindingTableOffset = pAvcPreProcBindingTable->dwAvcPreProcVMEFwdPicFrame;
        }

        // Picture Y VME
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bUseAdvState = true;
        SurfaceCodecParams.psSurface = &pParams->ppRefList[ucRefPicIdx]->sRefBuffer;
        SurfaceCodecParams.dwBindingTableOffset = dwRefBindingTableOffset;
        SurfaceCodecParams.ucVDirection = ucRefVDirection;
        SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));

        if (bCurrFieldPicture) // if current picture is field
        {
            SurfaceCodecParams.dwBindingTableOffset = pAvcPreProcBindingTable->dwAvcPreProcVMEFwdPicField[1];

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                m_hwInterface,
                pCmdBuffer,
                &SurfaceCodecParams,
                pKernelState));
        }
    }

    if (pPreEncParams->dwNumFutureReferences > 0)
    {
        auto RefPic = pPreEncParams->FutureRefPicture;
        uint8_t ucRefPicIdx = RefPic.FrameIdx;
        bool bRefBottomField = (CodecHal_PictureIsBottomField(RefPic)) ? 1 : 0;
        uint8_t ucRefVDirection;

        // Program the surface based on current picture's field/frame mode
        if (bCurrFieldPicture) // if current picture is field
        {
            if (bRefBottomField)
            {
                ucRefVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
            }
            else
            {
                ucRefVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
            }
            dwRefBindingTableOffset = pAvcPreProcBindingTable->dwAvcPreProcVMEBwdPicField[0];
        }
        else // if current picture is frame
        {
            ucRefVDirection = CODECHAL_VDIRECTION_FRAME;
            dwRefBindingTableOffset = pAvcPreProcBindingTable->dwAvcPreProcVMEBwdPicFrame[0];
        }

        // Picture Y VME
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bUseAdvState = true;
        SurfaceCodecParams.psSurface = &pParams->ppRefList[ucRefPicIdx]->sRefBuffer;
        SurfaceCodecParams.dwBindingTableOffset = dwRefBindingTableOffset;
        SurfaceCodecParams.ucVDirection = ucRefVDirection;
        SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));

        if (bCurrFieldPicture) // if current picture is field
        {
            dwRefBindingTableOffset = pAvcPreProcBindingTable->dwAvcPreProcVMEBwdPicField[1];
        }
        else
        {
            dwRefBindingTableOffset = pAvcPreProcBindingTable->dwAvcPreProcVMEBwdPicFrame[1];
        }
        SurfaceCodecParams.dwBindingTableOffset = dwRefBindingTableOffset;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::InitKernelStateMbEnc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    dwNumMbEncEncKrnStates =
        MBENC_TARGET_USAGE_CM * m_mbencNumTargetUsagesCmFei;
    pMbEncKernelStates = MOS_NewArray(MHW_KERNEL_STATE, dwNumMbEncEncKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pMbEncKernelStates);

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_GetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize));
    
    auto pKernelStatePtr = pMbEncKernelStates;

    for (uint32_t dwKrnStateIdx = 0; dwKrnStateIdx < dwNumMbEncEncKrnStates; dwKrnStateIdx++)
    {
        bool bKernelState = (dwKrnStateIdx >= MBENC_TARGET_USAGE_CM);
        CODECHAL_KERNEL_HEADER CurrKrnHeader;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeGetKernelHeaderAndSize(
            kernelBinary,
            (bKernelState ?ENC_MBENC_ADV : ENC_MBENC),
            (bKernelState ? dwKrnStateIdx - MBENC_TARGET_USAGE_CM : dwKrnStateIdx),
            &CurrKrnHeader,
            &kernelSize));

        pKernelStatePtr->KernelParams.iBTCount = CODECHAL_ENCODE_AVC_MBENC_NUM_SURFACES_G9;
        pKernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        pKernelStatePtr->KernelParams.iCurbeLength = sizeof(CODECHAL_ENCODE_AVC_FEI_MBENC_STATIC_DATA_G9);
        pKernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
        pKernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        pKernelStatePtr->KernelParams.iIdCount = 1;

        pKernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        pKernelStatePtr->KernelParams.pBinary = kernelBinary + (CurrKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        pKernelStatePtr->KernelParams.iSize = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            pKernelStatePtr->KernelParams.iBTCount,
            &pKernelStatePtr->dwSshSize,
            &pKernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_MhwInitISH(m_stateHeapInterface, pKernelStatePtr));

        pKernelStatePtr++;
    }

    // Until a better way can be found, maintain old binding table structures
    auto pBindingTable = &MbEncBindingTable;

    pBindingTable->dwAvcMBEncMfcAvcPakObj = CODECHAL_ENCODE_AVC_MBENC_MFC_AVC_PAK_OBJ_G9;
    pBindingTable->dwAvcMBEncIndMVData = CODECHAL_ENCODE_AVC_MBENC_IND_MV_DATA_G9;
    pBindingTable->dwAvcMBEncBRCDist = CODECHAL_ENCODE_AVC_MBENC_BRC_DISTORTION_G9;
    pBindingTable->dwAvcMBEncCurrY = CODECHAL_ENCODE_AVC_MBENC_CURR_Y_G9;
    pBindingTable->dwAvcMBEncCurrUV = CODECHAL_ENCODE_AVC_MBENC_CURR_UV_G9;
    pBindingTable->dwAvcMBEncMbSpecificData = CODECHAL_ENCODE_AVC_MBENC_MB_SPECIFIC_DATA_G9;

    pBindingTable->dwAvcMBEncRefPicSelectL0 = CODECHAL_ENCODE_AVC_MBENC_REFPICSELECT_L0_G9;
    pBindingTable->dwAvcMBEncMVDataFromME = CODECHAL_ENCODE_AVC_MBENC_MV_DATA_FROM_ME_G9;
    pBindingTable->dwAvcMBEncMEDist = CODECHAL_ENCODE_AVC_MBENC_4xME_DISTORTION_G9;
    pBindingTable->dwAvcMBEncSliceMapData = CODECHAL_ENCODE_AVC_MBENC_SLICEMAP_DATA_G9;
    pBindingTable->dwAvcMBEncBwdRefMBData = CODECHAL_ENCODE_AVC_MBENC_FWD_MB_DATA_G9;
    pBindingTable->dwAvcMBEncBwdRefMVData = CODECHAL_ENCODE_AVC_MBENC_FWD_MV_DATA_G9;
    pBindingTable->dwAvcMBEncMbBrcConstData = CODECHAL_ENCODE_AVC_MBENC_MBBRC_CONST_DATA_G9;
    pBindingTable->dwAvcMBEncMBStats = CODECHAL_ENCODE_AVC_MBENC_MB_STATS_G9;
    pBindingTable->dwAvcMBEncMADData = CODECHAL_ENCODE_AVC_MBENC_MAD_DATA_G9;
    pBindingTable->dwAvcMBEncMbNonSkipMap = CODECHAL_ENCODE_AVC_MBENC_FORCE_NONSKIP_MB_MAP_G9;
    pBindingTable->dwAvcMBEncAdv = CODECHAL_ENCODE_AVC_MBENC_ADV_WA_G9;
    pBindingTable->dwAvcMbEncBRCCurbeData = CODECHAL_ENCODE_AVC_MBENC_BRC_CURBE_DATA_G9;
    pBindingTable->dwAvcMBEncFlatnessChk = CODECHAL_ENCODE_AVC_MBENC_FLATNESS_CHECK_CM_G9;

    pBindingTable->dwAvcMBEncVMEDistortion = CODECHAL_ENCODE_AVC_MBENC_AUX_VME_OUT_G9;
    pBindingTable->dwAvcMBEncMvPrediction = CODECHAL_ENCODE_AVC_MBENC_MV_PREDICTOR_G9;

    // Frame
    pBindingTable->dwAvcMBEncMbQpFrame = CODECHAL_ENCODE_AVC_MBENC_MBQP_G9;
    pBindingTable->dwAvcMBEncCurrPicFrame[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_G9;
    pBindingTable->dwAvcMBEncFwdPicFrame[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX0_G9;
    pBindingTable->dwAvcMBEncBwdPicFrame[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_G9;
    pBindingTable->dwAvcMBEncFwdPicFrame[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX1_G9;
    pBindingTable->dwAvcMBEncBwdPicFrame[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_G9;
    pBindingTable->dwAvcMBEncFwdPicFrame[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX2_G9;
    pBindingTable->dwAvcMBEncFwdPicFrame[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX3_G9;
    pBindingTable->dwAvcMBEncFwdPicFrame[4] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX4_G9;
    pBindingTable->dwAvcMBEncFwdPicFrame[5] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX5_G9;
    pBindingTable->dwAvcMBEncFwdPicFrame[6] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX6_G9;
    pBindingTable->dwAvcMBEncFwdPicFrame[7] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX7_G9;
    pBindingTable->dwAvcMBEncCurrPicFrame[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_G9;
    pBindingTable->dwAvcMBEncBwdPicFrame[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_G9;
    pBindingTable->dwAvcMBEncBwdPicFrame[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_G9;

    // Field
    pBindingTable->dwAvcMBEncMbQpField = CODECHAL_ENCODE_AVC_MBENC_MBQP_G9;
    pBindingTable->dwAvcMBEncFieldCurrPic[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_G9;
    pBindingTable->dwAvcMBEncFwdPicTopField[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX0_G9;
    pBindingTable->dwAvcMBEncBwdPicTopField[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_G9;
    pBindingTable->dwAvcMBEncFwdPicBotField[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX0_G9;
    pBindingTable->dwAvcMBEncBwdPicBotField[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_G9;
    pBindingTable->dwAvcMBEncFwdPicTopField[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX1_G9;
    pBindingTable->dwAvcMBEncBwdPicTopField[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_G9;
    pBindingTable->dwAvcMBEncFwdPicBotField[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX1_G9;
    pBindingTable->dwAvcMBEncBwdPicBotField[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_G9;
    pBindingTable->dwAvcMBEncFwdPicTopField[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX2_G9;
    pBindingTable->dwAvcMBEncFwdPicBotField[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX2_G9;
    pBindingTable->dwAvcMBEncFwdPicTopField[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX3_G9;
    pBindingTable->dwAvcMBEncFwdPicBotField[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX3_G9;
    pBindingTable->dwAvcMBEncFwdPicTopField[4] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX4_G9;
    pBindingTable->dwAvcMBEncFwdPicBotField[4] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX4_G9;
    pBindingTable->dwAvcMBEncFwdPicTopField[5] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX5_G9;
    pBindingTable->dwAvcMBEncFwdPicBotField[5] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX5_G9;
    pBindingTable->dwAvcMBEncFwdPicTopField[6] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX6_G9;
    pBindingTable->dwAvcMBEncFwdPicBotField[6] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX6_G9;
    pBindingTable->dwAvcMBEncFwdPicTopField[7] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX7_G9;
    pBindingTable->dwAvcMBEncFwdPicBotField[7] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX7_G9;
    pBindingTable->dwAvcMBEncFieldCurrPic[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_G9;
    pBindingTable->dwAvcMBEncBwdPicTopField[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_G9;
    pBindingTable->dwAvcMBEncBwdPicBotField[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_G9;
    pBindingTable->dwAvcMBEncBwdPicTopField[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_G9;
    pBindingTable->dwAvcMBEncBwdPicBotField[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_G9;



    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::SendAvcMbEncSurfaces(PMOS_COMMAND_BUFFER pCmdBuffer, PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pAvcSlcParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->ppRefList);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pCurrOriginalPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pCurrReconstructedPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->psCurrPicSurface);

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pMbEncBindingTable);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pFeiPicParams);

    auto pKernelState = pParams->pKernelState;

    auto pFeiPicParams = (CodecEncodeAvcFeiPicParams *)pParams->pFeiPicParams;
    auto pAvcMbEncBindingTable = pParams->pMbEncBindingTable;

    bool bCurrFieldPicture = CodecHal_PictureIsField(*(pParams->pCurrOriginalPic)) ? 1 : 0;
    bool bCurrBottomField = CodecHal_PictureIsBottomField(*(pParams->pCurrOriginalPic)) ? 1 : 0;
    auto CurrPicRefListEntry = pParams->ppRefList[pParams->pCurrReconstructedPic->FrameIdx];
    auto presMbCodeBuffer = &CurrPicRefListEntry->resRefMbCodeBuffer;
    auto presMvDataBuffer = &CurrPicRefListEntry->resRefMvDataBuffer;
    uint32_t dwRefMbCodeBottomFieldOffset, dwRefMbCodeBottomFieldOffsetUsed;
    uint32_t dwRefMvBottomFieldOffset, dwRefMvBottomFieldOffsetUsed;
    if (pFeiPicParams->MbCodeMvEnable)
    {
        dwRefMbCodeBottomFieldOffset = 0;
        dwRefMvBottomFieldOffset = 0;
    }
    else
    {
        dwRefMbCodeBottomFieldOffset =
            pParams->dwFrameFieldHeightInMb * pParams->dwFrameWidthInMb * 64;
        dwRefMvBottomFieldOffset =
            MOS_ALIGN_CEIL(pParams->dwFrameFieldHeightInMb * pParams->dwFrameWidthInMb * (32 * 4), 0x1000);
    }

    uint8_t ucVDirection;
    if (pParams->bMbEncIFrameDistInUse)
    {
        ucVDirection = CODECHAL_VDIRECTION_FRAME;
    }
    else
    {
        ucVDirection = (CodecHal_PictureIsFrame(*(pParams->pCurrOriginalPic))) ? CODECHAL_VDIRECTION_FRAME :
            (bCurrBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD;
    }

    // PAK Obj command buffer
    uint32_t dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 16 * 4;  // 11DW + 5DW padding
    CODECHAL_SURFACE_CODEC_PARAMS               SurfaceCodecParams;
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.presBuffer = presMbCodeBuffer;
    SurfaceCodecParams.dwSize = dwSize;
    SurfaceCodecParams.dwOffset = pParams->dwMbCodeBottomFieldOffset;
    SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncMfcAvcPakObj;
    SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
    SurfaceCodecParams.bRenderTarget = true;
    SurfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    // MV data buffer
    dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 32 * 4;
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.presBuffer = presMvDataBuffer;
    SurfaceCodecParams.dwSize = dwSize;
    SurfaceCodecParams.dwOffset = pParams->dwMvBottomFieldOffset;
    SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncIndMVData;
    SurfaceCodecParams.bRenderTarget = true;
    SurfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    // Current Picture Y
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.bIs2DSurface = true;
    SurfaceCodecParams.bUseUVPlane = true;
    SurfaceCodecParams.psSurface = pParams->psCurrPicSurface;
    SurfaceCodecParams.dwOffset = pParams->dwCurrPicSurfaceOffset;
    SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncCurrY;
    SurfaceCodecParams.dwUVBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncCurrUV;
    SurfaceCodecParams.dwVerticalLineStride = pParams->dwVerticalLineStride;
    SurfaceCodecParams.dwVerticalLineStrideOffset = pParams->dwVerticalLineStrideOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    // AVC_ME MV data buffer
    if (pParams->bHmeEnabled)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->ps4xMeMvDataBuffer);

        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bIs2DSurface = true;
        SurfaceCodecParams.bMediaBlockRW = true;
        SurfaceCodecParams.psSurface = pParams->ps4xMeMvDataBuffer;
        SurfaceCodecParams.dwOffset = pParams->dwMeMvBottomFieldOffset;
        SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncMVDataFromME;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));

        CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->ps4xMeDistortionBuffer);

        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bIs2DSurface = true;
        SurfaceCodecParams.bMediaBlockRW = true;
        SurfaceCodecParams.psSurface = pParams->ps4xMeDistortionBuffer;
        SurfaceCodecParams.dwOffset = pParams->dwMeDistortionBottomFieldOffset;
        SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncMEDist;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    // Current Picture Y - VME
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.bUseAdvState = true;
    SurfaceCodecParams.psSurface = pParams->psCurrPicSurface;
    SurfaceCodecParams.dwOffset = pParams->dwCurrPicSurfaceOffset;
    SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    SurfaceCodecParams.dwBindingTableOffset = bCurrFieldPicture ?
        pAvcMbEncBindingTable->dwAvcMBEncFieldCurrPic[0] : pAvcMbEncBindingTable->dwAvcMBEncCurrPicFrame[0];
    SurfaceCodecParams.ucVDirection = ucVDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    SurfaceCodecParams.dwBindingTableOffset = bCurrFieldPicture ?
        pAvcMbEncBindingTable->dwAvcMBEncFieldCurrPic[1] : pAvcMbEncBindingTable->dwAvcMBEncCurrPicFrame[1];
    SurfaceCodecParams.ucVDirection = ucVDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    // Setup references 1...n
    // LIST 0 references
    for (uint8_t ucRefIdx = 0; ucRefIdx <= pParams->pAvcSlcParams->num_ref_idx_l0_active_minus1; ucRefIdx++)
    {
        auto RefPic = pParams->pAvcSlcParams->RefPicList[LIST_0][ucRefIdx];
        if (!CodecHal_PictureIsInvalid(RefPic) && pParams->pAvcPicIdx[RefPic.FrameIdx].bValid)
        {
            uint8_t  ucRefPicIdx = pParams->pAvcPicIdx[RefPic.FrameIdx].ucPicIdx;
            bool     bRefBottomField = (CodecHal_PictureIsBottomField(RefPic)) ? 1 : 0;
            uint32_t dwRefBindingTableOffset;
            uint8_t  ucRefVDirection;
            // Program the surface based on current picture's field/frame mode
            if (bCurrFieldPicture) // if current picture is field
            {
                if (bRefBottomField)
                {
                    ucRefVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
                    dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncFwdPicBotField[ucRefIdx];
                }
                else
                {
                    ucRefVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
                    dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncFwdPicTopField[ucRefIdx];
                }
            }
            else // if current picture is frame
            {
                ucRefVDirection = CODECHAL_VDIRECTION_FRAME;
                dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncFwdPicFrame[ucRefIdx];
            }

            // Picture Y VME
            MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            SurfaceCodecParams.bUseAdvState = true;
            SurfaceCodecParams.dwWidthInUse = pParams->dwFrameWidthInMb * 16;
            SurfaceCodecParams.dwHeightInUse = pParams->dwFrameHeightInMb * 16;
            if((pParams->bUseWeightedSurfaceForL0) &&
               (pParams->pAvcSlcParams->luma_weight_flag[LIST_0] & (1 << ucRefIdx)) &&
               (ucRefIdx < CODEC_AVC_MAX_FORWARD_WP_FRAME))
            {
                SurfaceCodecParams.psSurface = &pParams->pWeightedPredOutputPicSelectList[CODEC_AVC_WP_OUTPUT_L0_START + ucRefIdx].sBuffer;
            }
            else
            {
                SurfaceCodecParams.psSurface = &pParams->ppRefList[ucRefPicIdx]->sRefBuffer;
            }
            SurfaceCodecParams.dwBindingTableOffset = dwRefBindingTableOffset;
            SurfaceCodecParams.ucVDirection = ucRefVDirection;
            SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                m_hwInterface,
                pCmdBuffer,
                &SurfaceCodecParams,
                pKernelState));
        }
    }

    // Setup references 1...n
    // LIST 1 references
    for (uint8_t ucRefIdx = 0; ucRefIdx <= pParams->pAvcSlcParams->num_ref_idx_l1_active_minus1; ucRefIdx++)
    {
        if (!bCurrFieldPicture && ucRefIdx > 0)
        {
            // Only 1 LIST 1 reference required here since only single ref is supported in frame case
            break;
        }

        auto RefPic = pParams->pAvcSlcParams->RefPicList[LIST_1][ucRefIdx];
        if (!CodecHal_PictureIsInvalid(RefPic) && pParams->pAvcPicIdx[RefPic.FrameIdx].bValid)
        {
            uint8_t  ucRefPicIdx = pParams->pAvcPicIdx[RefPic.FrameIdx].ucPicIdx;
            bool     bRefBottomField = (CodecHal_PictureIsBottomField(RefPic)) ? 1 : 0;
            uint32_t dwRefBindingTableOffset;
            uint8_t  ucRefVDirection;
            // Program the surface based on current picture's field/frame mode
            if (bCurrFieldPicture) // if current picture is field
            {
                if (bRefBottomField)
                {
                    ucRefVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
                    dwRefMbCodeBottomFieldOffsetUsed = dwRefMbCodeBottomFieldOffset;
                    dwRefMvBottomFieldOffsetUsed = dwRefMvBottomFieldOffset;
                    dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBwdPicBotField[ucRefIdx];
                }
                else
                {
                    ucRefVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
                    dwRefMbCodeBottomFieldOffsetUsed = 0;
                    dwRefMvBottomFieldOffsetUsed = 0;
                    dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBwdPicTopField[ucRefIdx];
                }
            }
            else // if current picture is frame
            {
                ucRefVDirection = CODECHAL_VDIRECTION_FRAME;
                dwRefMbCodeBottomFieldOffsetUsed = 0;
                dwRefMvBottomFieldOffsetUsed = 0;
                dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBwdPicFrame[ucRefIdx];
            }

            // Picture Y VME
            MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            SurfaceCodecParams.bUseAdvState = true;

            SurfaceCodecParams.dwWidthInUse = pParams->dwFrameWidthInMb * 16;
            SurfaceCodecParams.dwHeightInUse = pParams->dwFrameHeightInMb * 16;
            if((pParams->bUseWeightedSurfaceForL1) &&
               (pParams->pAvcSlcParams->luma_weight_flag[LIST_1] & (1<<ucRefIdx)) &&
               (ucRefIdx < CODEC_AVC_MAX_BACKWARD_WP_FRAME))
            {
                SurfaceCodecParams.psSurface = &pParams->pWeightedPredOutputPicSelectList[CODEC_AVC_WP_OUTPUT_L1_START + ucRefIdx].sBuffer;
            }
            else
            {
                SurfaceCodecParams.psSurface = &pParams->ppRefList[ucRefPicIdx]->sRefBuffer;
            }
            SurfaceCodecParams.dwBindingTableOffset = dwRefBindingTableOffset;
            SurfaceCodecParams.ucVDirection = ucRefVDirection;
            SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                m_hwInterface,
                pCmdBuffer,
                &SurfaceCodecParams,
                pKernelState));

            if (ucRefIdx == 0)
            {
                if(bCurrFieldPicture && (pParams->ppRefList[ucRefPicIdx]->ucAvcPictureCodingType == CODEC_AVC_PIC_CODING_TYPE_FRAME || pParams->ppRefList[ucRefPicIdx]->ucAvcPictureCodingType == CODEC_AVC_PIC_CODING_TYPE_INVALID))
                {
                    dwRefMbCodeBottomFieldOffsetUsed = 0;
                    dwRefMvBottomFieldOffsetUsed     = 0;
                }
                // MB data buffer
                dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 16 * 4;

                MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                SurfaceCodecParams.dwSize = dwSize;
                if (pFeiPicParams->MbCodeMvEnable && bCurrFieldPicture)
                {
                    SurfaceCodecParams.presBuffer = bRefBottomField ? &pParams->ppRefList[ucRefPicIdx]->resRefBotFieldMbCodeBuffer :
                        &pParams->ppRefList[ucRefPicIdx]->resRefTopFieldMbCodeBuffer;
                }
                else
                {
                    SurfaceCodecParams.presBuffer = &pParams->ppRefList[ucRefPicIdx]->resRefMbCodeBuffer;
                }
                SurfaceCodecParams.dwOffset = dwRefMbCodeBottomFieldOffsetUsed;
                SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
                SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBwdRefMBData;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                    m_hwInterface,
                    pCmdBuffer,
                    &SurfaceCodecParams,
                    pKernelState));

                // MV data buffer
                dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 32 * 4;
                MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                SurfaceCodecParams.dwSize = dwSize;
                if (pFeiPicParams->MbCodeMvEnable && bCurrFieldPicture)
                {
                    SurfaceCodecParams.presBuffer = bRefBottomField ? &pParams->ppRefList[ucRefPicIdx]->resRefBotFieldMvDataBuffer :
                        &pParams->ppRefList[ucRefPicIdx]->resRefTopFieldMvDataBuffer;
                }
                else
                {
                    SurfaceCodecParams.presBuffer = &pParams->ppRefList[ucRefPicIdx]->resRefMvDataBuffer;
                }
                SurfaceCodecParams.dwOffset = dwRefMvBottomFieldOffsetUsed;
                SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
                SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBwdRefMVData;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                    m_hwInterface,
                    pCmdBuffer,
                    &SurfaceCodecParams,
                    pKernelState));
            }

            if (ucRefIdx < CODECHAL_ENCODE_NUM_MAX_VME_L1_REF)
            {
                if (bCurrFieldPicture)
                {
                    // The binding table contains multiple entries for IDX0 backwards references
                    if (bRefBottomField)
                    {
                        dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBwdPicBotField[ucRefIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                    }
                    else
                    {
                        dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBwdPicTopField[ucRefIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                    }
                }
                else
                {
                    dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBwdPicFrame[ucRefIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                }

                // Picture Y VME
                MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                SurfaceCodecParams.bUseAdvState = true;
                SurfaceCodecParams.dwWidthInUse = pParams->dwFrameWidthInMb * 16;
                SurfaceCodecParams.dwHeightInUse = pParams->dwFrameHeightInMb * 16;
                SurfaceCodecParams.psSurface = &pParams->ppRefList[ucRefPicIdx]->sRefBuffer;
                SurfaceCodecParams.dwBindingTableOffset = dwRefBindingTableOffset;
                SurfaceCodecParams.ucVDirection = ucRefVDirection;
                SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                    m_hwInterface,
                    pCmdBuffer,
                    &SurfaceCodecParams,
                    pKernelState));
            }
        }
    }

    // BRC distortion data buffer for I frame
    if (pParams->bMbEncIFrameDistInUse)
    {
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bIs2DSurface = true;
        SurfaceCodecParams.bMediaBlockRW = true;
        SurfaceCodecParams.psSurface = pParams->psMeBrcDistortionBuffer;
        SurfaceCodecParams.dwOffset = pParams->dwMeBrcDistortionBottomFieldOffset;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBRCDist;
        SurfaceCodecParams.bIsWritable = true;
        SurfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    // RefPicSelect of Current Picture
    if (pParams->bUsedAsRef)
    {
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bIs2DSurface = true;
        SurfaceCodecParams.bMediaBlockRW = true;
        SurfaceCodecParams.psSurface = &CurrPicRefListEntry->pRefPicSelectListEntry->sBuffer;
        SurfaceCodecParams.psSurface->dwHeight = MOS_ALIGN_CEIL(SurfaceCodecParams.psSurface->dwHeight, 8);
        SurfaceCodecParams.dwOffset = pParams->dwRefPicSelectBottomFieldOffset;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncRefPicSelectL0;
        SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
        SurfaceCodecParams.bRenderTarget = true;
        SurfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    if (pParams->bFlatnessCheckEnabled)
    {
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bIs2DSurface = true;
        SurfaceCodecParams.bMediaBlockRW = true;
        SurfaceCodecParams.psSurface = pParams->psFlatnessCheckSurface;
        SurfaceCodecParams.dwOffset = bCurrBottomField ? pParams->dwFlatnessCheckBottomFieldOffset : 0;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncFlatnessChk;
        SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_FLATNESS_CHECK_ENCODE].Value;
        SurfaceCodecParams.bRenderTarget = true;
        SurfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    if (pParams->bMADEnabled)
    {
        dwSize = CODECHAL_MAD_BUFFER_SIZE;

        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bRawSurface = true;
        SurfaceCodecParams.dwSize = dwSize;
        SurfaceCodecParams.presBuffer = pParams->presMADDataBuffer;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncMADData;
        SurfaceCodecParams.bRenderTarget = true;
        SurfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    if (pParams->bArbitraryNumMbsInSlice)
    {
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bIs2DSurface = true;
        SurfaceCodecParams.bMediaBlockRW = true;
        SurfaceCodecParams.psSurface = pParams->psSliceMapSurface;
        SurfaceCodecParams.bRenderTarget = false;
        SurfaceCodecParams.bIsWritable = false;
        SurfaceCodecParams.dwOffset = bCurrBottomField ? pParams->dwSliceMapBottomFieldOffset : 0;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncSliceMapData;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    // FEI Mb Specific Data surface
    if (pFeiPicParams->bPerMBInput)
    {
        dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 16;
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.dwSize = dwSize;
        SurfaceCodecParams.presBuffer = &(pFeiPicParams->resMBCtrl);
        SurfaceCodecParams.dwOffset = 0;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncMbSpecificData;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    // FEI Multi Mv Predictor Surface
    if (pFeiPicParams->MVPredictorEnable)
    {
        dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 40;
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.dwSize = dwSize;
        SurfaceCodecParams.presBuffer = &(pFeiPicParams->resMVPredictor);
        SurfaceCodecParams.dwOffset = 0;
        SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncMvPrediction;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    // FEI distortion surface
    if (pFeiPicParams->DistortionEnable)
    {
        dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 48;
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.presBuffer = &(pFeiPicParams->resDistortion);
        SurfaceCodecParams.dwOffset = 0;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncVMEDistortion;
        SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        SurfaceCodecParams.bRenderTarget = true;
        SurfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    if (pFeiPicParams->bMBQp)
    {
        // 16 DWs per QP value
        dwSize = 16 * 52 * sizeof(uint32_t);
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.presBuffer = pParams->presMbBrcConstDataBuffer;
        SurfaceCodecParams.dwSize = dwSize;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncMbBrcConstData;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));

        dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb + 3;
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.presBuffer = &(pFeiPicParams->resMBQp);
        SurfaceCodecParams.dwOffset = 0;
        SurfaceCodecParams.dwBindingTableOffset = bCurrFieldPicture ? pAvcMbEncBindingTable->dwAvcMBEncMbQpField :
            pAvcMbEncBindingTable->dwAvcMBEncMbQpFrame;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::InitMfe()
{
    if (!m_mfeEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    m_mfeLastStream   = (m_mfeEncodeParams.submitIndex == m_mfeEncodeParams.submitNumber - 1);
    m_mfeFirstStream  = (m_mfeEncodeParams.submitIndex == 0);

    // Defer allocate some MFE specific resources and init flags
    if (!m_mfeInitialized)
    {
        uint32_t size = MOS_ALIGN_CEIL(m_mbencBrcBufferSize,
                m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());

        MOS_LOCK_PARAMS LockFlagsWriteOnly;
        MOS_ZeroMemory(&LockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
        LockFlagsWriteOnly.WriteOnly = 1;

        MOS_ALLOC_GFXRES_PARAMS AllocParamsForBufferLinear;
        MOS_ZeroMemory(&AllocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        AllocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        AllocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        AllocParamsForBufferLinear.Format = Format_Buffer;
        AllocParamsForBufferLinear.dwBytes = size;
        AllocParamsForBufferLinear.pBufName = "MbEnc BRC buffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &BrcBuffers.resMbEncBrcBuffer));

        uint8_t* pData = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &(BrcBuffers.resMbEncBrcBuffer),
            &LockFlagsWriteOnly);
        CODECHAL_ENCODE_CHK_NULL_RETURN(pData);

        MOS_ZeroMemory(pData, size);
        m_osInterface->pfnUnlockResource(m_osInterface, &BrcBuffers.resMbEncBrcBuffer);
        CODECHAL_DEBUG_TOOL(
            m_debugInterface->dwStreamId = m_mfeEncodeParams.streamId;
        )

        pOrigCmDev = pCmDev;
        m_origResMbencKernel = m_resMbencKernel;

        // Whether mfe mbenc kernel is enabled or not
        MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        CodecHal_UserFeature_ReadValue(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_MFE_MBENC_ENABLE_ID,
            &UserFeatureData);
        m_mfeMbEncEanbled = (UserFeatureData.i32Data) ? true : false;

        m_mfeInitialized = true;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::InitKernelStateMfeMbEnc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
       
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmDev);
    m_resMbencKernel = (CodechalEncodeMdfKernelResource*)MOS_AllocAndZeroMemory(
        sizeof(CodechalEncodeMdfKernelResource));
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_resMbencKernel);

    m_vmeSurface = new (std::nothrow) SurfaceIndex[m_vmeSurfaceSize];
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_vmeSurface);

    m_commonSurface = new (std::nothrow) SurfaceIndex[m_commonSurfaceSize];
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_commonSurface);

    auto pKernelRes = m_resMbencKernel;

    CreateMDFKernelResource(pKernelRes, 1,
                m_mdfMbencBufSize * CODECHAL_ENCODE_AVC_MFE_MAX_FRAMES_G9,
                m_mdfMbencSurfSize * CODECHAL_ENCODE_AVC_MFE_MAX_FRAMES_G9,
                m_mdfMbencVmeSurfSize * CODECHAL_ENCODE_AVC_MFE_MAX_FRAMES_G9,
                0);

    uint32_t codeSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_ReadFileToPtr(strMbEncIsaName, (uint32_t*)&codeSize, &pKernelRes->pCommonISA));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->LoadProgram(pKernelRes->pCommonISA, codeSize, pKernelRes->pCmProgram, "-nojitter"));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateKernel(pKernelRes->pCmProgram, "AVCEncMB_MFE", pKernelRes->ppKernel[0]));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::SendCurbeAvcMfeMbEnc()
{
    MOS_STATUS eStatus  = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto  avcMBEncSurface = m_cmSurfIdx;
    uint32_t vmeIdx = 0;
    uint32_t surfIdx = 0;
    SurfaceIndex *vmeSurface = m_vmeSurface;
    SurfaceIndex *commonSurface = m_commonSurface;

    if (IsMfeMbEncEnabled())
    {
        vmeIdx = m_mfeEncodeParams.submitIndex * m_vmeSurfacePerStreamSize;
        surfIdx = m_mfeEncodeParams.submitIndex * m_commonSurfacePerStreamSize;
        vmeSurface = m_mfeEncodeSharedState->vmeSurface;
        commonSurface = m_mfeEncodeSharedState->commonSurface;
    }

#define SET_SURFINDEX(surfaceIndex) (surfaceIndex == (SurfaceIndex *)CM_NULL_SURFACE ? CM_NULL_SURFACE : *surfaceIndex)

    vmeSurface[vmeIdx++] = SET_SURFINDEX(avcMBEncSurface->MBVMEInterPredictionSurfIndex);
    vmeSurface[vmeIdx++] = SET_SURFINDEX(avcMBEncSurface->MBVMEInterPredictionMRSurfIndex);

    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->BRCCurbeSurfIndex);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->MBDataSurfIndex);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->MVDataSurfIndex);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->FwdFrmMBDataSurfIndex);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->FwdFrmMVSurfIndex);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->HMEMVPredFwdBwdSurfIndex);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->HMEDistSurfIndex);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->MBDistIndex);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->SrcYSurfIndex);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->MBBRCLut);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->MADSurfIndex);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->ReservedIndex);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->StaticDetectionCostTableIndex);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->CurrRefPicSelSurfIndex);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->MBstats);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->MBSpecficDataSurfIndex);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->ForceNonSkipMBMap);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->SliceMapSurfIndex);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->MBQPBuffer);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->AuxVmeOutSurfIndex);
    commonSurface[surfIdx++] = SET_SURFINDEX(avcMBEncSurface->FEI_MVPredSurfIndex);

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::DispatchKernelMbEnc(
    void      *pParams)
{
    MOS_STATUS eStatus  = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto pDispatchParams = (PCODECHAL_ENCODE_AVC_MBENC_DISPATCH_PARAMS) pParams;
    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmDev);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pDispatchParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pDispatchParams->pKernelRes);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pDispatchParams->avcMBEncSurface);

    auto  pKernel = pDispatchParams->pKernelRes->ppKernel[0];

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(0, sizeof(SurfaceIndex) * m_vmeSurfaceSize, m_vmeSurface));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetKernelArg(1, sizeof(SurfaceIndex) * m_commonSurfaceSize, m_commonSurface));

    uint32_t   dwNumMBRows  = (pDispatchParams->dwNumMBs + 1) / pDispatchParams->frameWidthInMBs;
    uint32_t   dwThreadCount = (pDispatchParams->frameWidthInMBs) * dwNumMBRows;

    if (!pDispatchParams->EnableWavefrontOptimization)
    {
        if (!pDispatchParams->EnableArbitrarySliceSize )
        {
            dwThreadCount = pDispatchParams->frameWidthInMBs * pDispatchParams->wSliceHeight * pDispatchParams->numSlices;//new API
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernel->SetThreadCount(dwThreadCount));
    //currently , it's always false.
    if (!pDispatchParams->EnableWavefrontOptimization)
    {
        if(!pDispatchParams->EnableArbitrarySliceSize)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateThreadSpace(pDispatchParams->frameWidthInMBs, pDispatchParams->wSliceHeight, pDispatchParams->pKernelRes->pTS));
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateThreadSpace(pDispatchParams->frameWidthInMBs, dwNumMBRows, pDispatchParams->pKernelRes->pTS));
        }

        auto pCmThreadSpace = pDispatchParams->pKernelRes->pTS;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmThreadSpace->SelectThreadDependencyPattern(CM_WAVEFRONT26));

        if (!pDispatchParams->EnableArbitrarySliceSize)
        {
            CODECHAL_ENCODE_ASSERT(pDispatchParams->numSlices <= 16);
            pCmThreadSpace->SetThreadSpaceColorCount(pDispatchParams->numSlices );
        }

    }
    else
    {
        /* current not support this path code is incomplete */
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateThreadSpace(pDispatchParams->frameWidthInMBs, dwNumMBRows, pDispatchParams->pKernelRes->pTS));
        auto pCmThreadSpace = pDispatchParams->pKernelRes->pTS;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmThreadSpace->SetThreadSpaceColorCount(2));
    }
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::SendAvcMfeMbEncSurfaces(PMOS_COMMAND_BUFFER pCmdBuffer, PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS pParams)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    
    CODECHAL_ENCODE_FUNCTION_ENTER;
   
    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmDev);

    auto pKernelRes   = m_resMbencKernel;
    auto pCmSurfaces  = m_cmSurfIdx;
    
    CodecEncodeAvcFeiPicParams *pFeiPicParams = (CodecEncodeAvcFeiPicParams *)pParams->pFeiPicParams;

    bool bCurrFieldPicture = CodecHal_PictureIsField(*(pParams->pCurrOriginalPic)) ? 1 : 0;
    bool bCurrBottomField = CodecHal_PictureIsBottomField(*(pParams->pCurrOriginalPic)) ? 1 : 0;
    auto CurrPicRefListEntry = pParams->ppRefList[pParams->pCurrReconstructedPic->FrameIdx];
    auto presMbCodeBuffer = &CurrPicRefListEntry->resRefMbCodeBuffer;
    auto presMvDataBuffer = &CurrPicRefListEntry->resRefMvDataBuffer;

    for(uint8_t ucRefIdx = 0; ucRefIdx < sizeof (pCmSurfaces->pCmSurfIdx) / sizeof (pCmSurfaces->pCmSurfIdx[0]); ucRefIdx ++)
    {
        pCmSurfaces->pCmSurfIdx[ucRefIdx] = (SurfaceIndex*)CM_NULL_SURFACE;
    }
    
    auto ppCmSurf         = pKernelRes->ppCmSurf;
    auto ppCmBuf          = pKernelRes->ppCmBuf;
    auto ppCmVmeSurfIdx   = pKernelRes->ppCmVmeSurf;

    uint32_t dwRefMbCodeBottomFieldOffset;
    uint32_t dwRefMvBottomFieldOffset;
    
    if (pFeiPicParams->MbCodeMvEnable)
    {
        dwRefMbCodeBottomFieldOffset = 0;
        dwRefMvBottomFieldOffset = 0;
    }
    else
    {
        dwRefMbCodeBottomFieldOffset = m_mbcodeBottomFieldOffset;
        dwRefMvBottomFieldOffset =
            MOS_ALIGN_CEIL(pParams->dwFrameFieldHeightInMb * pParams->dwFrameWidthInMb * (32 * 4), CODECHAL_PAGE_SIZE);
    }

    uint32_t dwMBCodeSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 16 * 4;  // 11DW + 5DW padding
    uint32_t dwMvSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 32 * 4;

    // PAK Obj command buffer
    //dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 16 * 4;  // 11DW + 5DW padding
    // mb code offset     SurfaceCodecParams.dwOffset = pParams->dwMbCodeBottomFieldOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(presMbCodeBuffer, ppCmBuf[mbEncMbCodeBuffer]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(presMvDataBuffer, ppCmBuf[mbEncMvDataBuffer]));
    if((pFeiPicParams->MbCodeMvEnable) || ((pParams->dwMbCodeBottomFieldOffset == 0) && (pParams->dwMvBottomFieldOffset == 0)))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[mbEncMbCodeBuffer]->GetIndex(pCmSurfaces->MBDataSurfIndex));
        //MV data offset SurfaceCodecParams.dwOffset = pParams->dwMvBottomFieldOffset;
        //dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 32 * 4;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[mbEncMvDataBuffer]->GetIndex(pCmSurfaces->MVDataSurfIndex));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[mbEncMbCodeBuffer]->GetIndex(pCmSurfaces->MBDataSurfIndex));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[mbEncMvDataBuffer]->GetIndex(pCmSurfaces->MVDataSurfIndex));

        CM_BUFFER_STATE_PARAM param;
        MOS_ZeroMemory(&param,sizeof(param));
        param.uiSize    = dwMBCodeSize;
        param.uiBaseAddressOffset  = pParams->dwMbCodeBottomFieldOffset;
        ppCmBuf[mbEncMbCodeBuffer]->SetSurfaceStateParam(pCmSurfaces->MBDataSurfIndex, &param);

        param.uiSize    = dwMvSize;
        param.uiBaseAddressOffset  = pParams->dwMvBottomFieldOffset;
        ppCmBuf[mbEncMvDataBuffer]->SetSurfaceStateParam(pCmSurfaces->MVDataSurfIndex, &param);
    }
    
    // Current Picture Y    2D, and offset = pParams->dwCurrPicSurfaceOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&pParams->psCurrPicSurface->OsResource,ppCmSurf[mbEncSrcYSurf]));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[mbEncSrcYSurf]->GetIndex(pCmSurfaces->SrcYSurfIndex));

    if(bCurrFieldPicture)
    {
        if(CodecHal_PictureIsBottomField((*pParams->pCurrOriginalPic)))
        {
            ppCmSurf[mbEncSrcYSurf]->SetProperty(CM_BOTTOM_FIELD);
        }
        else
        {
            ppCmSurf[mbEncSrcYSurf]->SetProperty(CM_TOP_FIELD);
        }
    }  
    // AVC_ME MV data buffer
    if (pParams->bHmeEnabled)
    {
        //2D, and offset  = pParams->dwMeMvBottomFieldOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&pParams->ps4xMeMvDataBuffer->OsResource,ppCmSurf[mbEncHMEMVPredFwdBwdSurf]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[mbEncHMEMVPredFwdBwdSurf]->GetIndex(pCmSurfaces->HMEMVPredFwdBwdSurfIndex));

        //HMEdistortion 2D, Offset = pParams->dwMeDistortionBottomFieldOffset;
        CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->ps4xMeDistortionBuffer);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&pParams->ps4xMeDistortionBuffer->OsResource,ppCmSurf[mbEncHMEDistSurf]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[mbEncHMEDistSurf]->GetIndex(pCmSurfaces->HMEDistSurfIndex));
    }
    //current piture Y take  two binding table offset , dwAvcMBEncCurrPicFrame[0] dwAvcMBEncCurrPicFrame[1] dwAvcMBEncFieldCurrPic[0] dwAvcMBEncFieldCurrPic[1]

    //CODECHAL_ENCODE_CHK_STATUS(pCmDev->CreateSurface2D(&pParams->psCurrPicSurface->OsResource,pCmSurfForVME));
    // Setup references 1...n
    // LIST 0 references
    uint8_t ucRefIdx = 0;
    uint8_t ucRefIdx1 = 0;

    uint8_t ucRefNum0 = 0;
    uint8_t ucRefNum1 = 0;

    CmSurface2D                             *pCmSurfForVME;
    CmSurface2D                             *surfArrayL0[8];
    CmSurface2D                             *surfArrayL1[8];
    
    pCmSurfForVME = ppCmSurf[0];
    MOS_ZeroMemory(&surfArrayL0[0],8 * sizeof(CmSurface2D *));
    MOS_ZeroMemory(&surfArrayL1[0],8 * sizeof(CmSurface2D *));
    for (ucRefIdx = 0; ucRefIdx <= pParams->pAvcSlcParams->num_ref_idx_l0_active_minus1; ucRefIdx++)
    {
        CODEC_PICTURE RefPic = pParams->pAvcSlcParams->RefPicList[LIST_0][ucRefIdx];
        if (!CodecHal_PictureIsInvalid(RefPic) && pParams->pAvcPicIdx[RefPic.FrameIdx].bValid)
        {
            uint8_t ucRefPicIdx = pParams->pAvcPicIdx[RefPic.FrameIdx].ucPicIdx;
            bool bRefBottomField = (CodecHal_PictureIsBottomField(RefPic)) ? 1 : 0;
            ucRefNum0 ++;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&pParams->ppRefList[ucRefPicIdx]->sRefBuffer.OsResource,ppCmSurf[mbEncVMEInterPredictionSurf + ucRefIdx]))
            surfArrayL0[ucRefIdx] = (CmSurface2D *)ppCmSurf[mbEncVMEInterPredictionSurf + ucRefIdx];
            
        }
    }
    for (ucRefIdx1 = 0; ucRefIdx1 <= pParams->pAvcSlcParams->num_ref_idx_l1_active_minus1; ucRefIdx1++)
    {
        CODEC_PICTURE RefPic = pParams->pAvcSlcParams->RefPicList[LIST_1][ucRefIdx1];
        if (!CodecHal_PictureIsInvalid(RefPic) && pParams->pAvcPicIdx[RefPic.FrameIdx].bValid)
        {
            uint8_t ucRefPicIdx = pParams->pAvcPicIdx[RefPic.FrameIdx].ucPicIdx;
            bool bRefBottomField = (CodecHal_PictureIsBottomField(RefPic)) ? 1 : 0;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&pParams->ppRefList[ucRefPicIdx]->sRefBuffer.OsResource,ppCmSurf[mbEncVMEInterPredictionMRSurf + ucRefIdx1]))
            surfArrayL1[ucRefIdx1] = (CmSurface2D *)ppCmSurf[mbEncVMEInterPredictionMRSurf + ucRefIdx1];

            ucRefNum1 ++;
            if (ucRefIdx1 == 0)
            {
                // MB data buffer   Offset = dwRefMbCodeBottomFieldOffsetUsed;
                if((pFeiPicParams->MbCodeMvEnable) && bCurrFieldPicture)
                {
                    if(bRefBottomField)
                    {
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&pParams->ppRefList[ucRefPicIdx]->resRefBotFieldMbCodeBuffer, ppCmBuf[mbEncFwdMbCodeBuffer]));
                    }
                    else
                    {
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&pParams->ppRefList[ucRefPicIdx]->resRefTopFieldMbCodeBuffer, ppCmBuf[mbEncFwdMbCodeBuffer]));
                    }
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[mbEncFwdMbCodeBuffer]->GetIndex(pCmSurfaces->FwdFrmMBDataSurfIndex));
                }
                else
                {
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&pParams->ppRefList[ucRefPicIdx]->resRefMbCodeBuffer, ppCmBuf[mbEncFwdMbCodeBuffer]));
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[mbEncFwdMbCodeBuffer]->GetIndex(pCmSurfaces->FwdFrmMBDataSurfIndex));

                    if(bCurrFieldPicture && bRefBottomField)
                    {
                        CM_BUFFER_STATE_PARAM param;
                        MOS_ZeroMemory(&param,sizeof(param));
                        param.uiSize    = dwMBCodeSize;
                        param.uiBaseAddressOffset  = dwRefMbCodeBottomFieldOffset;
                        ppCmBuf[mbEncFwdMbCodeBuffer]->SetSurfaceStateParam(pCmSurfaces->FwdFrmMBDataSurfIndex, &param);
                    }
                    else
                    {
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[mbEncFwdMbCodeBuffer]->GetIndex(pCmSurfaces->FwdFrmMBDataSurfIndex));
                    }
                }
            
                // MV data buffer   Offset = dwRefMvBottomFieldOffsetUsed;
                if ( (pFeiPicParams->MbCodeMvEnable) && bCurrFieldPicture)
                {
                    if(bRefBottomField)
                    {
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&pParams->ppRefList[ucRefPicIdx]->resRefBotFieldMvDataBuffer, ppCmBuf[mbEncFwdMvDataBuffer]));
                    }
                    else
                    {
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&pParams->ppRefList[ucRefPicIdx]->resRefTopFieldMvDataBuffer, ppCmBuf[mbEncFwdMvDataBuffer]));
                    }
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[mbEncFwdMvDataBuffer]->GetIndex(pCmSurfaces->FwdFrmMVSurfIndex));

                }
                else
                {
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&pParams->ppRefList[ucRefPicIdx]->resRefMvDataBuffer, ppCmBuf[mbEncFwdMvDataBuffer]));
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[mbEncFwdMvDataBuffer]->GetIndex(pCmSurfaces->FwdFrmMVSurfIndex));

                    if(bCurrFieldPicture && bRefBottomField)
                    {
                        CM_BUFFER_STATE_PARAM param;
                        MOS_ZeroMemory(&param,sizeof(param));
                        param.uiSize    = dwMvSize;
                        param.uiBaseAddressOffset  = dwRefMvBottomFieldOffset;
                        ppCmBuf[mbEncFwdMvDataBuffer]->SetSurfaceStateParam(pCmSurfaces->FwdFrmMVSurfIndex, &param);
                    }
                    else
                    {
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[mbEncFwdMvDataBuffer]->GetIndex(pCmSurfaces->FwdFrmMVSurfIndex));
                    }
                }
            }
        }
    }
    pCmDev->CreateVmeSurfaceG7_5( pCmSurfForVME,&surfArrayL0[0],&surfArrayL1[0], ucRefNum0, ucRefNum1,ppCmVmeSurfIdx[0]);
    pCmSurfaces->MBVMEInterPredictionSurfIndex = ppCmVmeSurfIdx[0];
    
    pCmDev->CreateVmeSurfaceG7_5( pCmSurfForVME,&surfArrayL1[0],&surfArrayL1[0], ucRefNum1, ucRefNum1,ppCmVmeSurfIdx[1]);
    pCmSurfaces->MBVMEInterPredictionMRSurfIndex = ppCmVmeSurfIdx[1];

    CM_VME_SURFACE_STATE_PARAM VmeDimensionParam;
    VmeDimensionParam.width = pParams->dwFrameWidthInMb * 16;
    VmeDimensionParam.height = pParams->dwFrameHeightInMb * 16;
    pCmDev->SetVmeSurfaceStateParam(pCmSurfaces->MBVMEInterPredictionSurfIndex, &VmeDimensionParam);
    pCmDev->SetVmeSurfaceStateParam(pCmSurfaces->MBVMEInterPredictionMRSurfIndex, &VmeDimensionParam);
    
    // BRC distortion data buffer for I frame
    if (pParams->bMbEncIFrameDistInUse)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&pParams->psMeBrcDistortionBuffer->OsResource,ppCmSurf[mbEncMBDistSurf]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[mbEncMBDistSurf]->GetIndex(pCmSurfaces->MBDistIndex));
    }
    // RefPicSelect of Current Picture
    if (pParams->bUsedAsRef)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&CurrPicRefListEntry->pRefPicSelectListEntry->sBuffer.OsResource, ppCmSurf[mbEncCurrRefPicSelSurf]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[mbEncCurrRefPicSelSurf]->GetIndex(pCmSurfaces->CurrRefPicSelSurfIndex));
    }
    if (pParams->bFlatnessCheckEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&pParams->psFlatnessCheckSurface->OsResource, ppCmSurf[mbEncMBstatsSurf]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[mbEncMBstatsSurf]->GetIndex(pCmSurfaces->MBstats));
    }
    if (pParams->bMADEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(pParams->presMADDataBuffer, ppCmBuf[mbEncMADDataBuffer]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[mbEncMADDataBuffer]->GetIndex(pCmSurfaces->MADSurfIndex));
    }
    if (pParams->bArbitraryNumMbsInSlice)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateSurface2D(&pParams->psSliceMapSurface->OsResource, ppCmSurf[mbEncSliceMapSurf]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmSurf[mbEncSliceMapSurf]->GetIndex(pCmSurfaces->SliceMapSurfIndex));
    }
    if (pFeiPicParams->bPerMBInput)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&(pFeiPicParams->resMBCtrl), ppCmBuf[mbEncMBSpecficDataBuffer]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[mbEncMBSpecficDataBuffer]->GetIndex(pCmSurfaces->MBSpecficDataSurfIndex));
    }
    if (pFeiPicParams->MVPredictorEnable)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&(pFeiPicParams->resMVPredictor), ppCmBuf[mbEncMVPredictorBuffer]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[mbEncMVPredictorBuffer]->GetIndex(pCmSurfaces->FEI_MVPredSurfIndex));
    }
    if (pFeiPicParams->DistortionEnable)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&(pFeiPicParams->resDistortion), ppCmBuf[mbEncAuxVmeOutBuffer]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[mbEncAuxVmeOutBuffer]->GetIndex(pCmSurfaces->AuxVmeOutSurfIndex));
    }

    if (pFeiPicParams->bMBQp)
    {
        // 16 DWs per QP value
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(pParams->presMbBrcConstDataBuffer, ppCmBuf[mbEncMBBRCLutBuffer]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[mbEncMBBRCLutBuffer]->GetIndex(pCmSurfaces->MBBRCLut));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(&(pFeiPicParams->resMBQp), ppCmBuf[mbEncMBQPBuffer]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[mbEncMBQPBuffer]->GetIndex(pCmSurfaces->MBQPBuffer));
    }

    if (pParams->dwMbEncBRCBufferSize > 0)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCmDev->CreateBuffer(pParams->presMbEncBRCBuffer, ppCmBuf[mbEncBRCCurbeBuffer]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ppCmBuf[mbEncBRCCurbeBuffer]->GetIndex(pCmSurfaces->BRCCurbeSurfIndex));
    }

    return eStatus;    
}

MOS_STATUS CodechalEncodeAvcEncFeiG9::EncodeMbEncKernelFunctions()
{
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_refList);

    if (IsMfeMbEncEnabled())
    {
        if (m_mfeFirstStream)
        {
            for (int i = 0; i < m_vmeSurfaceSize; i++)
            {
                m_vmeSurface[i] = CM_NULL_SURFACE;
            }
            m_mfeEncodeSharedState->pCmDev = pCmDev;
            m_mfeEncodeSharedState->resMbencKernel = m_resMbencKernel;
            m_mfeEncodeSharedState->vmeSurface = m_vmeSurface;
            m_mfeEncodeSharedState->commonSurface = m_commonSurface;
        }
        else
        {
            // All the streams share the Cm device and Mbenc kernel state from first stream
            pCmDev = m_mfeEncodeSharedState->pCmDev;
            m_resMbencKernel = m_mfeEncodeSharedState->resMbencKernel;
        }
        // Set maximum width/height, it is used for initializing media walker parameters
        // during submitting the command buffer at the last stream.
        if (m_picWidthInMb > m_mfeEncodeSharedState->dwPicWidthInMB)
        {
            m_mfeEncodeSharedState->dwPicWidthInMB = m_picWidthInMb;
        }
        if (m_frameFieldHeightInMb > m_mfeEncodeSharedState->dwPicHeightInMB)
        {
            m_mfeEncodeSharedState->dwPicHeightInMB = m_frameFieldHeightInMb;
        }
        if (m_sliceHeight > m_mfeEncodeSharedState->sliceHeight)
        {
            m_mfeEncodeSharedState->sliceHeight = m_sliceHeight;
        }
        m_mfeEncodeSharedState->encoders.push_back(this);
    }

    auto pKernelRes = m_resMbencKernel;

    uint8_t ucPPSIdx = m_avcSliceParams->pic_parameter_set_id;
    CODECHAL_ENCODE_CHK_COND_RETURN((ucPPSIdx >= CODECHAL_AVC_MAX_PPS_NUM), "ERROR - Invalid pic parameter set id");

    uint8_t ucSPSIdx = m_avcPicParams[ucPPSIdx]->seq_parameter_set_id;
    auto pRefList = &m_refList[0];
    auto pCurrRefList = m_refList[m_currReconstructedPic.FrameIdx];

    bool bRoiEnabled = (m_avcPicParams[ucPPSIdx]->NumROI > 0) ? true : false;
    uint8_t ucCurrScaledIdx = m_refList[m_currReconstructedPic.FrameIdx]->ucScalingIdx;

    uint8_t ucRefPicListIdx = m_avcSliceParams[ucPPSIdx].RefPicList[0][0].FrameIdx;
    uint8_t ucRefFrameListIdx = m_avcPicParam[ucPPSIdx].RefFrameList[ucRefPicListIdx].FrameIdx;

    bool bDirtyRoiEnabled = (m_pictureCodingType == P_TYPE 
        && m_avcPicParams[ucPPSIdx]->NumDirtyROI > 0
        && m_prevReconFrameIdx == ucRefFrameListIdx);
    
    //  Two flags(bMbConstDataBufferNeeded, bMbQpBufferNeeded) 
    //  would be used as there are two buffers and not all cases need both the buffers
    //  Constant Data buffer  needed for MBBRC, MBQP, ROI, RollingIntraRefresh
    //  Please note that this surface needs to be programmed for
    //  all usage cases(including CQP cases) because DWord13 includes mode cost for high texture MBs cost.
    bool bMbConstDataBufferInUse = bMbBrcEnabled || bMbQpDataEnabled || bRoiEnabled || bDirtyRoiEnabled ||
        m_avcPicParam->EnableRollingIntraRefresh || bHighTextureModeCostEnable;

    bool bMbQpBufferInUse =  bMbBrcEnabled ||  bBrcRoiEnabled ||  bMbQpDataEnabled;

    if (m_feiEnable)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_avcFeiPicParams);
        bMbConstDataBufferInUse |= m_avcFeiPicParams->bMBQp;
        bMbQpBufferInUse        |= m_avcFeiPicParams->bMBQp;
    }

    PerfTagSetting PerfTag;
    PerfTag.Value = 0;
    PerfTag.Mode = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    PerfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_MBENC_KERNEL;
    PerfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, PerfTag.Value);

    // Setup AVC Curbe
    CODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS      MbEncCurbeParams;
    MOS_ZeroMemory(&MbEncCurbeParams, sizeof(MbEncCurbeParams));
    MbEncCurbeParams.pPicParams = m_avcPicParams[ucPPSIdx];
    MbEncCurbeParams.pSeqParams = m_avcSeqParams[ucSPSIdx];
    MbEncCurbeParams.pSlcParams = m_avcSliceParams;
    MbEncCurbeParams.ppRefList = &(m_refList[0]);
    MbEncCurbeParams.pPicIdx = &(m_picIdx[0]);
    MbEncCurbeParams.bRoiEnabled = bRoiEnabled;
    MbEncCurbeParams.bDirtyRoiEnabled = bDirtyRoiEnabled;
    MbEncCurbeParams.bMbEncIFrameDistEnabled = false;
    MbEncCurbeParams.pdwBlockBasedSkipEn = & dwMbEncBlockBasedSkipEn;
    MbEncCurbeParams.bBrcEnabled =  bBrcEnabled;
    MbEncCurbeParams.wPicWidthInMb = m_picWidthInMb;
    MbEncCurbeParams.wFieldFrameHeightInMb = m_frameFieldHeightInMb;
    MbEncCurbeParams.usSliceHeight = (m_arbitraryNumMbsInSlice) ?
        m_frameFieldHeightInMb : m_sliceHeight;
    MbEncCurbeParams.bUseMbEncAdvKernel =  bUseMbEncAdvKernel;
    MbEncCurbeParams.pAvcQCParams =  m_avcQCParams ;
    MbEncCurbeParams.bMbDisableSkipMapEnabled =  bMbDisableSkipMapEnabled;
    MbEncCurbeParams.bStaticFrameDetectionEnabled =  bStaticFrameDetectionEnable && m_hmeEnabled;
    MbEncCurbeParams.bApdatvieSearchWindowSizeEnabled =  bApdatvieSearchWindowEnable;
    MbEncCurbeParams.bSquareRollingIEnabled =  bSquareRollingIEnabled;
    MbEncCurbeParams.pCurbeBinary = pKernelRes->pCurbe;
    CODECHAL_ENCODE_CHK_STATUS_RETURN( SetCurbeAvcMbEnc(&MbEncCurbeParams));

    for (int i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        if (m_picIdx[i].bValid)
        {
            uint8_t ucIndex = m_picIdx[i].ucPicIdx;
            pRefList[ucIndex]->sRefBuffer = m_userFlags.bUseRawPicForRef ?
                pRefList[ucIndex]->sRefRawBuffer : pRefList[ucIndex]->sRefReconBuffer;

            CodecHal_GetResourceInfo(m_osInterface, &pRefList[ucIndex]->sRefBuffer);
        }
    }

    // Set up MB BRC Constant Data Buffer if there is QP change within a frame
    if (bMbConstDataBufferInUse)
    {
        CODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS InitMbBrcConstantDataBufferParams;

        MOS_ZeroMemory(&InitMbBrcConstantDataBufferParams, sizeof(InitMbBrcConstantDataBufferParams));
        InitMbBrcConstantDataBufferParams.pOsInterface = m_osInterface;
        InitMbBrcConstantDataBufferParams.presBrcConstantDataBuffer =
            & BrcBuffers.resMbBrcConstDataBuffer[m_currRecycledBufIdx];
        InitMbBrcConstantDataBufferParams.dwMbEncBlockBasedSkipEn =  dwMbEncBlockBasedSkipEn;
        InitMbBrcConstantDataBufferParams.pPicParams =  m_avcPicParams[ucPPSIdx];
        InitMbBrcConstantDataBufferParams.wPictureCodingType = m_pictureCodingType;
        InitMbBrcConstantDataBufferParams.bSkipBiasAdjustmentEnable = m_skipBiasAdjustmentEnable;
        InitMbBrcConstantDataBufferParams.bAdaptiveIntraScalingEnable =  bAdaptiveIntraScalingEnable;
        InitMbBrcConstantDataBufferParams.bOldModeCostEnable =  bOldModeCostEnable;
        InitMbBrcConstantDataBufferParams.pAvcQCParams              =  m_avcQCParams ;
        InitMbBrcConstantDataBufferParams.bEnableKernelTrellis =  bKernelTrellis &&  m_trellisQuantParams.dwTqEnabled;

        // Kernel controlled Trellis Quantization
        if ( bKernelTrellis &&  m_trellisQuantParams.dwTqEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN( CalcLambdaTable(
                m_pictureCodingType,
                &InitMbBrcConstantDataBufferParams.Lambda[0][0]));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN( InitMbBrcConstantDataBuffer(&InitMbBrcConstantDataBufferParams));
    }

    CODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS    MbEncSurfaceParams;
    //Add surface states
    MOS_ZeroMemory(&MbEncSurfaceParams, sizeof(MbEncSurfaceParams));
    MbEncSurfaceParams.pAvcSlcParams            = m_avcSliceParams;
    MbEncSurfaceParams.ppRefList                = &m_refList[0];
    MbEncSurfaceParams.pAvcPicIdx               = &m_picIdx[0];
    MbEncSurfaceParams.pCurrOriginalPic         = &m_currOriginalPic;
    MbEncSurfaceParams.pCurrReconstructedPic    = &m_currReconstructedPic;
    MbEncSurfaceParams.wPictureCodingType       = m_pictureCodingType;
    MbEncSurfaceParams.psCurrPicSurface         = m_rawSurfaceToEnc;
    MbEncSurfaceParams.dwMbCodeBottomFieldOffset            = m_mbcodeBottomFieldOffset;
    MbEncSurfaceParams.dwMvBottomFieldOffset                = m_mvBottomFieldOffset;
    MbEncSurfaceParams.ps4xMeMvDataBuffer                   = &m_4xMeMvDataBuffer;
    MbEncSurfaceParams.ps4xMeDistortionBuffer               = &m_4xMeDistortionBuffer;
    MbEncSurfaceParams.dwMeMvBottomFieldOffset              = m_meMvBottomFieldOffset;
    MbEncSurfaceParams.psMeBrcDistortionBuffer              = & BrcBuffers.sMeBrcDistortionBuffer;
    MbEncSurfaceParams.dwMeBrcDistortionBottomFieldOffset   =  BrcBuffers.dwMeBrcDistortionBottomFieldOffset;
    MbEncSurfaceParams.dwMeDistortionBottomFieldOffset      = m_meDistortionBottomFieldOffset;
    MbEncSurfaceParams.dwRefPicSelectBottomFieldOffset      = ulRefPicSelectBottomFieldOffset;
    MbEncSurfaceParams.dwFrameWidthInMb                     = (uint32_t)m_picWidthInMb;
    MbEncSurfaceParams.dwFrameFieldHeightInMb               = (uint32_t)m_frameFieldHeightInMb;
    MbEncSurfaceParams.dwFrameHeightInMb                    = (uint32_t)m_picHeightInMb;
    // Interleaved input surfaces
    MbEncSurfaceParams.dwVerticalLineStride                 = m_verticalLineStride;
    MbEncSurfaceParams.dwVerticalLineStrideOffset           = m_verticalLineStrideOffset;
    // Vertical line stride is not used for the case of scaled surfaces saved as separate fields
    MbEncSurfaceParams.bHmeEnabled                          = m_hmeSupported;
    MbEncSurfaceParams.presMbBrcConstDataBuffer             = & BrcBuffers.resMbBrcConstDataBuffer[m_currRecycledBufIdx];
    MbEncSurfaceParams.psMbQpBuffer                         =
         bMbQpDataEnabled ? & sMbQpDataSurface : & BrcBuffers.sBrcMbQpBuffer;
    MbEncSurfaceParams.dwMbQpBottomFieldOffset              =  bMbQpDataEnabled ? 0 :  BrcBuffers.dwBrcMbQpBottomFieldOffset;
    MbEncSurfaceParams.bUsedAsRef                           =
        m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef;
    MbEncSurfaceParams.presMADDataBuffer                    = &m_resMadDataBuffer[m_currMadBufferIdx];
    MbEncSurfaceParams.bMbQpBufferInUse                     = bMbQpBufferInUse;
    MbEncSurfaceParams.bMbConstDataBufferInUse              = bMbConstDataBufferInUse;
    MbEncSurfaceParams.bMADEnabled                          = m_bMadEnabled;
    MbEncSurfaceParams.bUseMbEncAdvKernel                   = bUseMbEncAdvKernel;
    MbEncSurfaceParams.presMbEncCurbeBuffer                 = & BrcBuffers.resMbEncAdvancedDsh;
    MbEncSurfaceParams.presMbEncBRCBuffer                   = & BrcBuffers.resMbEncBrcBuffer;

    if ( bDecoupleMbEncCurbeFromBRC || IsMfeMbEncEnabled())
    {
        MbEncSurfaceParams.dwMbEncBRCBufferSize             = m_mbencBrcBufferSize;
    }
    
    MbEncSurfaceParams.bUseAdvancedDsh                      =  bAdvancedDshInUse;
    MbEncSurfaceParams.bBrcEnabled                          =  bBrcEnabled;
    MbEncSurfaceParams.bArbitraryNumMbsInSlice              = m_arbitraryNumMbsInSlice;
    MbEncSurfaceParams.psSliceMapSurface                    = &m_sliceMapSurface[m_currRecycledBufIdx];
    MbEncSurfaceParams.dwSliceMapBottomFieldOffset          = m_sliceMapBottomFieldOffset;
    MbEncSurfaceParams.pMbEncBindingTable                   = & MbEncBindingTable;

    if(m_mbStatsSupported)
    {
        MbEncSurfaceParams.bMBVProcStatsEnabled             = (m_flatnessCheckEnabled || bAdaptiveTransformDecisionEnabled);
        MbEncSurfaceParams.presMBVProcStatsBuffer           = &m_resMbStatsBuffer;
        MbEncSurfaceParams.dwMBVProcStatsBottomFieldOffset  = m_mbStatsBottomFieldOffset;
    }
    else
    {
        MbEncSurfaceParams.bFlatnessCheckEnabled                = m_flatnessCheckEnabled;
        MbEncSurfaceParams.psFlatnessCheckSurface               = &m_flatnessCheckSurface;
        MbEncSurfaceParams.dwFlatnessCheckBottomFieldOffset     = m_flatnessCheckBottomFieldOffset;
    }

    // Set up pFeiPicParams
    MbEncSurfaceParams.pFeiPicParams                        =  m_avcFeiPicParams;

    MbEncSurfaceParams.bMbDisableSkipMapEnabled             =  bMbDisableSkipMapEnabled;
    MbEncSurfaceParams.psMbDisableSkipMapSurface            =  psMbDisableSkipMapSurface;

    if( bUseWeightedSurfaceForL0 ||  bUseWeightedSurfaceForL1)
    {
        MbEncSurfaceParams.pWeightedPredOutputPicSelectList     = & WeightedPredOutputPicSelectList[0];
        MbEncSurfaceParams.bUseWeightedSurfaceForL0             =  bUseWeightedSurfaceForL0;
        MbEncSurfaceParams.bUseWeightedSurfaceForL1             =  bUseWeightedSurfaceForL1;
    }

    // Clear the MAD buffer -- the kernel requires it to be 0 as it accumulates the result
    if (MbEncSurfaceParams.bMADEnabled)
    {
        MOS_LOCK_PARAMS     LockFlags;
        // set lock flag to WRITE_ONLY
        MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));
        LockFlags.WriteOnly = 1;

        uint8_t* pbData = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            MbEncSurfaceParams.presMADDataBuffer,
            &LockFlags);

        CODECHAL_ENCODE_CHK_NULL_RETURN(pbData);

        MOS_ZeroMemory(pbData, CODECHAL_MAD_BUFFER_SIZE);

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            MbEncSurfaceParams.presMADDataBuffer);

        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_resMadDataBuffer[m_currMadBufferIdx],
            CodechalDbgAttr::attrOutput,
            "MADRead",
            CODECHAL_MAD_BUFFER_SIZE,
            0,
            CODECHAL_MEDIA_STATE_ENC_QUALITY)));
    }

    // static frame detection buffer
    MbEncSurfaceParams.bStaticFrameDetectionEnabled =  bStaticFrameDetectionEnable && m_hmeEnabled;
    MbEncSurfaceParams.presSFDOutputBuffer          = & resSFDOutputBuffer[0];
    if (m_pictureCodingType == P_TYPE)
    {
        MbEncSurfaceParams.presSFDCostTableBuffer = & resSFDCostTablePFrameBuffer;
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        MbEncSurfaceParams.presSFDCostTableBuffer = & resSFDCostTableBFrameBuffer;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN( SendAvcMfeMbEncSurfaces(nullptr, &MbEncSurfaceParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN( SendCurbeAvcMfeMbEnc());

    if (!IsMfeMbEncEnabled() || m_mfeLastStream)
    {
        CODECHAL_ENCODE_AVC_MBENC_DISPATCH_PARAMS   DispatchParams;
        MOS_ZeroMemory(&DispatchParams,sizeof(DispatchParams));
        DispatchParams.pKernelRes                   = pKernelRes;
        DispatchParams.avcMBEncSurface              = m_cmSurfIdx;
        DispatchParams.EnableArbitrarySliceSize     = m_arbitraryNumMbsInSlice;
        DispatchParams.EnableWavefrontOptimization  = false;
        DispatchParams.wSliceType                   = m_pictureCodingType;

        if (IsMfeMbEncEnabled())
        {
            DispatchParams.frameWidthInMBs              = m_mfeEncodeSharedState->dwPicWidthInMB;
            DispatchParams.wSliceHeight                 = m_mfeEncodeSharedState->sliceHeight;
            DispatchParams.numSlices                    = m_mfeEncodeParams.submitNumber;
            DispatchParams.dwNumMBs                     = m_mfeEncodeSharedState->dwPicHeightInMB * m_mfeEncodeSharedState->dwPicWidthInMB;
        }
        else
        {
            DispatchParams.frameWidthInMBs              = m_picWidthInMb;
            DispatchParams.wSliceHeight                 = MbEncCurbeParams.usSliceHeight;
            DispatchParams.numSlices                    = 1;
            DispatchParams.dwNumMBs                     = m_frameFieldHeightInMb * m_picWidthInMb;
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(DispatchKernelMbEnc(&DispatchParams));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AddKernelMdf(pCmDev,pCmQueue,pKernelRes->ppKernel[0],pCmTask,pKernelRes->pTS,pKernelRes->e,true));
        if (IsMfeMbEncEnabled())
        {
            CodechalEncoderState *encoder = m_mfeEncodeSharedState->encoders[0];
            // The owner stream is responsible for destroying the shared event
            if(encoder->pSharedCmEvent[encoder->nSharedCmEventIdx] )
            {
                encoder->pCmQueue->DestroyEvent(encoder->pSharedCmEvent[encoder->nSharedCmEventIdx]);
            }
            encoder->pSharedCmEvent[encoder->nCmEventIdx]   = pKernelRes->e;
            encoder->nSharedCmEventIdx ++;
            encoder->nSharedCmEventIdx %= CM_EVENT_NUM;
            // All the Mfe streams wait for the same event
            for (uint32_t i = 0; i < m_mfeEncodeSharedState->encoders.size(); i++)
            {
                encoder = m_mfeEncodeSharedState->encoders[i];
                encoder->pCmEvent[encoder->nCmEventIdx]   = pKernelRes->e;
                encoder->nCmEventIdx ++;
                encoder->nCmEventIdx %= CM_EVENT_NUM;
            }
            m_mfeEncodeSharedState->encoders.clear();
        }
        else
        {
            if(pCmEvent[nCmEventIdx] )
            {
                pCmQueue->DestroyEvent(pCmEvent[nCmEventIdx]);
            }
            pCmEvent[nCmEventIdx]   = pKernelRes->e;
            nCmEventIdx ++;
            nCmEventIdx %= CM_EVENT_NUM;
        }

        FreeMDFKernelSurfaces(pKernelRes);
    }

    pCurrRefList->ucMADBufferIdx = m_currMadBufferIdx;
    pCurrRefList->bMADEnabled = m_bMadEnabled;

    if (IsMfeMbEncEnabled())
    {
        pCmDev = pOrigCmDev;
        m_resMbencKernel  = m_origResMbencKernel;
    }

    return eStatus;
}

