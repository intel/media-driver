/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
** Copyright (c) 2019-2020 Intel Corporation.
**
** INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
** LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
** ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
** PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
** DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
** PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
** including liability for infringement of any proprietary rights, relating to
** use of the code. No license, express or implied, by estoppel or otherwise,
** to any intellectual property rights is granted herein.
**
**
** File Name  : codechal_encode_vp9_g9.h
**
** Abstract   : This modules implements Render interface layer for VP9 encoding to be used
**              on all operating systems/DDIs, across CODECHAL components.
**
** Environment: All Platforms (XP, Vista, Win7, Android, etc)
**
** Notes      : This module must not contain:
**
**              - OS dependent code
**              - DDI layer dependencies
**
**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#ifndef __CODECHAL_ENCODER_VP9_G9_H__
#define __CODECHAL_ENCODER_VP9_G9_H__

#include "codechal_encode_vp9.h"

#define CODECHAL_ENCODE_VP9_CURBE_SIZE_MBENC_G9                  \
    (sizeof(CODECHAL_ENCODE_VP9_MBENC_STATIC_DATA_G9))

#define CODECHAL_ENCODE_VP9_CURBE_SIZE_BRC_G9                  \
    (sizeof(CODECHAL_ENCODE_VP9_BRC_STATIC_DATA_G9))

#define CODECHAL_ENCODE_VP9_CURBE_SIZE_ME_G9                     \
    (sizeof(CODECHAL_ENCODE_VP9_ME_STATIC_DATA_CM_G9))

typedef struct _CODECHAL_ENCODE_VP9_KERNEL_HEADER_G9 {
    int nKernelCount;

    // DownScaling
    CODECHAL_KERNEL_HEADER PLY_DSCALE;

    // HME 
    CODECHAL_KERNEL_HEADER VP9_ME_P;

    // 32x32 I ENC KERNEL
    CODECHAL_KERNEL_HEADER VP9_Enc_I_32x32;

    // 16x16 I ENC KERNEL
    CODECHAL_KERNEL_HEADER VP9_Enc_I_16x16;

    // P ENC KERNEL
    CODECHAL_KERNEL_HEADER VP9_Enc_P;

    // TX ENC KERNEL
    CODECHAL_KERNEL_HEADER VP9_Enc_TX;

    // DYS KERNEL
    CODECHAL_KERNEL_HEADER VP9_DYS;

    //Intra Distortion
    CODECHAL_KERNEL_HEADER VP9BRC_Intra_Distortion;

    // BRC Init
    CODECHAL_KERNEL_HEADER VP9BRC_Init;

    // BRC Reset
    CODECHAL_KERNEL_HEADER VP9BRC_Reset;

    // BRC Update
    CODECHAL_KERNEL_HEADER VP9BRC_Update;
} CODECHAL_ENCODE_VP9_KERNEL_HEADER_G9, *PCODECHAL_ENCODE_VP9_KERNEL_HEADER_G9;

typedef enum _CODECHAL_ENCODE_VP9_BINDING_TABLE_OFFSET_ME_G9
{
    CODECHAL_ENCODE_VP9_ME_MV_DATA_SURFACE_G9              = 0,
    CODECHAL_ENCODE_VP9_16xME_MV_DATA_SURFACE_G9           = 1,
    CODECHAL_ENCODE_VP9_ME_DISTORTION_SURFACE_G9           = 2,
    CODECHAL_ENCODE_VP9_ME_BRC_DISTORTION_SURFACE_G9       = 3,
    CODECHAL_ENCODE_VP9_ME_CURR_PIC_L0_G9                  = 4,
    CODECHAL_ENCODE_VP9_ME_CURR_PIC_L1_G9                  = CODECHAL_ENCODE_VP9_ME_CURR_PIC_L0_G9 + 17,
    CODECHAL_ENCODE_VP9_ME_NUM_SURFACES_G9                 = CODECHAL_ENCODE_VP9_ME_CURR_PIC_L1_G9 + 5
} CODECHAL_ENCODE_VP9_BINDING_TABLE_OFFSET_ME_G9;

typedef enum _CODECHAL_ENCODE_VP9_BINDING_TABLE_OFFSET_MBENC_G9
{
    CODECHAL_ENCODE_VP9_MBENC_CURR_Y_G9                    = 0,
    CODECHAL_ENCODE_VP9_MBENC_CURR_UV_G9                   = 1,
    CODECHAL_ENCODE_VP9_MBENC_CURR_NV12_G9                 = 2,
    CODECHAL_ENCODE_VP9_MBENC_LAST_NV12_G9                 = 3,
    CODECHAL_ENCODE_VP9_MBENC_GOLD_NV12_G9                 = 5,
    CODECHAL_ENCODE_VP9_MBENC_ALTREF_NV12_G9               = 7,
    CODECHAL_ENCODE_VP9_MBENC_SEGMENTATION_MAP_G9          = 8,
    CODECHAL_ENCODE_VP9_MBENC_TX_CURBE_G9                  = 9,
    CODECHAL_ENCODE_VP9_MBENC_HME_MV_DATA_G9               = 10,
    CODECHAL_ENCODE_VP9_MBENC_HME_DISTORTION_G9            = 11,
    CODECHAL_ENCODE_VP9_MBENC_MODE_DECISION_PREV_G9        = 12,
    CODECHAL_ENCODE_VP9_MBENC_MODE_DECISION_G9             = 13,
    CODECHAL_ENCODE_VP9_MBENC_OUT_16x16_INTER_MODES_G9     = 14,
    CODECHAL_ENCODE_VP9_MBENC_CU_RECORDS_G9                = 15,
    CODECHAL_ENCODE_VP9_MBENC_PAK_DATA_G9                  = 16,
    CODECHAL_ENCODE_VP9_MBENC_NUM_SURFACES_G9              = 17,
} CODECHAL_ENCODE_VP9_BINDING_TABLE_OFFSET_MBENC_G9;

typedef enum _CODECHAL_ENCODE_VP9_BINDING_TABLE_OFFSET_BRC_G9
{
    CODECHAL_ENCODE_VP9_BRC_SRCY4X_G9                          = 0,
    CODECHAL_ENCODE_VP9_BRC_VME_COARSE_INTRA_G9                = 1,
    CODECHAL_ENCODE_VP9_BRC_HISTORY_G9                         = 2, 
    CODECHAL_ENCODE_VP9_BRC_CONSTANT_DATA_G9                   = 3,
    CODECHAL_ENCODE_VP9_BRC_DISTORTION_G9                      = 4, 
    CODECHAL_ENCODE_VP9_BRC_MSDK_PAK_OUTPUT_G9                 = 5,
    CODECHAL_ENCODE_VP9_BRC_MBENC_CURBE_INPUT_G9               = 6,
    CODECHAL_ENCODE_VP9_BRC_MBENC_CURBE_OUTPUT_G9              = 7,
    CODECHAL_ENCODE_VP9_BRC_PIC_STATE_INPUT_G9                 = 8,
    CODECHAL_ENCODE_VP9_BRC_PIC_STATE_OUTPUT_G9                = 9,
    CODECHAL_ENCODE_VP9_BRC_SEGMENT_STATE_INPUT_G9             = 10,
    CODECHAL_ENCODE_VP9_BRC_SEGMENT_STATE_OUTPUT_G9            = 11,
    CODECHAL_ENCODE_VP9_BRC_BITSTREAM_SIZE_G9                  = 12,
    CODECHAL_ENCODE_VP9_BRC_HUC_DATA_G9                        = 13,
    CODECHAL_ENCODE_VP9_BRC_NUM_SURFACES_G9                    = 14,
} CODECHAL_ENCODE_VP9_BINDING_TABLE_OFFSET_BRC_G9;

typedef struct _CODECHAL_ENCODE_VP9_ME_STATIC_DATA_CM_G9
{
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn                   : MOS_BITFIELD_BIT(0);
            uint32_t   AdaptiveEn                   : MOS_BITFIELD_BIT(1);
            uint32_t   BiMixDis                     : MOS_BITFIELD_BIT(2);
            uint32_t                                : MOS_BITFIELD_RANGE(3, 4);
            uint32_t   EarlyImeSuccessEn            : MOS_BITFIELD_BIT(5);
            uint32_t                                : MOS_BITFIELD_BIT(6);
            uint32_t   T8x8FlagForInterEn           : MOS_BITFIELD_BIT(7);
            uint32_t                                : MOS_BITFIELD_RANGE(8, 23);
            uint32_t   EarlyImeStop                 : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MaxNumMVs                    : MOS_BITFIELD_RANGE(0, 5);
            uint32_t                                : MOS_BITFIELD_RANGE(6, 15);
            uint32_t   BiWeight                     : MOS_BITFIELD_RANGE(16, 21);
            uint32_t                                : MOS_BITFIELD_RANGE(22, 27);
            uint32_t   UniMixDisable                : MOS_BITFIELD_BIT(28);
            uint32_t                                : MOS_BITFIELD_RANGE(29, 31);
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
            uint32_t   MaxLenSP                     : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MaxNumSU                     : MOS_BITFIELD_RANGE(8, 15);
            uint32_t                                : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   SrcSize                      : MOS_BITFIELD_RANGE(0, 1);
            uint32_t                                : MOS_BITFIELD_RANGE(2, 3);
            uint32_t   MbTypeRemap                  : MOS_BITFIELD_RANGE(4, 5);
            uint32_t   SrcAccess                    : MOS_BITFIELD_BIT(6);
            uint32_t   RefAccess                    : MOS_BITFIELD_BIT(7);
            uint32_t   SearchCtrl                   : MOS_BITFIELD_RANGE(8, 10);
            uint32_t   DualSearchPathOption         : MOS_BITFIELD_BIT(11);
            uint32_t   SubPelMode                   : MOS_BITFIELD_RANGE(12, 13);
            uint32_t   SkipType                     : MOS_BITFIELD_BIT(14);
            uint32_t   DisableFieldCacheAlloc       : MOS_BITFIELD_BIT(15);
            uint32_t   InterChromaMode              : MOS_BITFIELD_BIT(16);
            uint32_t   FTEnable                     : MOS_BITFIELD_BIT(17);
            uint32_t   BMEDisableFBR                : MOS_BITFIELD_BIT(18);
            uint32_t   BlockBasedSkipEnable         : MOS_BITFIELD_BIT(19);
            uint32_t   InterSAD                     : MOS_BITFIELD_RANGE(20, 21);
            uint32_t   IntraSAD                     : MOS_BITFIELD_RANGE(22, 23);
            uint32_t   SubMbPartMask                : MOS_BITFIELD_RANGE(24, 30);
            uint32_t                                : MOS_BITFIELD_BIT(31);
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
            uint32_t                                : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   PictureHeightMinus1          : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   PictureWidth                 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t                                : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t                                : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   QpPrimeY                     : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefWidth                     : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   RefHeight                    : MOS_BITFIELD_RANGE(24, 31);

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
            uint32_t                                : MOS_BITFIELD_RANGE(0, 2);
            uint32_t   WriteDistortions             : MOS_BITFIELD_BIT(3);
            uint32_t   UseMvFromPrevStep            : MOS_BITFIELD_BIT(4);
            uint32_t                                : MOS_BITFIELD_RANGE(5, 7);
            uint32_t   SuperCombineDist             : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MaxVmvR                      : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t                                : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   MVCostScaleFactor            : MOS_BITFIELD_RANGE(16, 17);
            uint32_t   BilinearEnable               : MOS_BITFIELD_BIT(18);
            uint32_t   SrcFieldPolarity             : MOS_BITFIELD_BIT(19);
            uint32_t   WeightedSADHAAR              : MOS_BITFIELD_BIT(20);
            uint32_t   AConlyHAAR                   : MOS_BITFIELD_BIT(21);
            uint32_t   RefIDCostMode                : MOS_BITFIELD_BIT(22);
            uint32_t                                : MOS_BITFIELD_BIT(23);
            uint32_t   SkipCenterMask               : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode0Cost                    : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode1Cost                    : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Mode2Cost                    : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Mode3Cost                    : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode4Cost                    : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode5Cost                    : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Mode6Cost                    : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Mode7Cost                    : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Mode8Cost                    : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Mode9Cost                    : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefIDCost                    : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ChromaIntraModeCost          : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MV0Cost                      : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MV1Cost                      : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MV2Cost                      : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   MV3Cost                      : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MV4Cost                      : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MV5Cost                      : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MV6Cost                      : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   MV7Cost                      : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   NumRefIdxL0MinusOne          : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   NumRefIdxL1MinusOne          : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   ActualMBWidth                : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ActualMBHeight               : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   L0RefPicPolarityBits         : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   L1RefPicPolarityBits         : MOS_BITFIELD_RANGE(8, 9);
            uint32_t   Reserved                     : MOS_BITFIELD_RANGE(10, 31);
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
            uint32_t   PrevMvReadPosFactor          : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MvShiftFactor                : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Reserved                     : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t       Value;
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

    // DW30
    union
    {
        struct
        {
            uint32_t   Reserved                     : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                     : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   _4xMeMvOutputDataSurfIndex   : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   _16xOr32xMeMvInputDataSurfIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   _4xMeOutputDistSurfIndex     : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   _4xMeOutputBrcDistSurfIndex  : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   VMEFwdInterPredictionSurfIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   VMEBwdInterPredictionSurfIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                     : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t       Value;
        };
    } DW38;

} CODECHAL_ENCODE_VP9_ME_STATIC_DATA_CM_G9, *PCODECHAL_ENCODE_VP9_ME_STATIC_DATA_CM_G9;

C_ASSERT(SIZE32(CODECHAL_ENCODE_VP9_ME_STATIC_DATA_CM_G9) == 39);

// MbEnc CURBE
typedef struct _CODECHAL_ENCODE_VP9_MBENC_STATIC_DATA_G9
{
    // DW0
    union
    {
        struct
        {
            uint32_t   FrameWidth                   : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   FrameHeight                  : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   FrameType                    : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   SegmentationEnable           : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefFrameFlags                : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Min16For32Check              : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MultiPred                    : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   LenSP                        : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   SearchX                      : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   SearchY                      : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   HmeEnable                    : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MultiRefQPCheck              : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   DisableTempPredictor         : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   MinRefFor32RefCheck          : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Skip16Threshold              : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   DisableMRThreshold           : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   EnableMBRC                   : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   InterRound                   : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   IntraRound                   : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   FrameQPIndex                 : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Reserved                     : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                     : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   LastRefQP                    : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   GoldRefQP                    : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   AltRefQP                     : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   Reserved                     : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   SumIntraDist                 : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   SumInterDist                 : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   NumIntra                     : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   NumLastRef                   : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   NumGoldRef                   : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   NumAltRef                    : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   IMESearchPathDelta03         : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   IMESearchPathDelta47         : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   IMESearchPathDelta811        : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   IMESearchPathDelta1215       : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   IMESearchPathDelta1619       : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   IMESearchPathDelta2023       : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   IMESearchPathDelta2427       : MOS_BITFIELD_RANGE(0, 31);

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
            uint32_t   IMESearchPathDelta2831       : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   IMESearchPathDelta3235       : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   IMESearchPathDelta3639       : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   IMESearchPathDelta4043       : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   IMESearchPathDelta4447       : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   IMESearchPathDelta4851       : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   IMESearchPathDelta5255       : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                     : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                     : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW31;

    struct
    {
        // DW32
        union
        {
            struct
            {
                uint32_t   SegmentQPIndex           : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   IntraNonDCPenalty16x16   : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   IntraNonDCPenalty8x8     : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   IntraNonDCPenalty4x4     : MOS_BITFIELD_RANGE(24, 31);
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
                uint32_t   IntraNonDCPenalty32x32   : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   Reserved                 : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   ZeroCost                 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   NearCost                 : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   NearestCost              : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   RefIDCost                : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   MVCost0                  : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   MVCost1                  : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   MVCost2                  : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   MVCost3                  : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   MVCost4                  : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   MVCost5                  : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   MVCost6                  : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   MVCost7                  : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   MVCost0                  : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   MVCost1                  : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   MVCost2                  : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   MVCost3                  : MOS_BITFIELD_RANGE(24, 31);
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
                uint32_t   MVCost4                  : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   MVCost5                  : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   MVCost6                  : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   MVCost7                  : MOS_BITFIELD_RANGE(24, 31);
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
                uint32_t   ModeCost0                : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   ModeCost1                : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   ModeCost2                : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   ModeCost3                : MOS_BITFIELD_RANGE(24, 31);
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
                uint32_t   ModeCost4                : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   ModeCost5                : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   ModeCost6                : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   ModeCost7                : MOS_BITFIELD_RANGE(24, 31);
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
                uint32_t   ModeCost8                : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   ModeCost9                : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   RefIDCost                : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   Reserved                 : MOS_BITFIELD_RANGE(24, 31);
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
                uint32_t   ModeCostIntra32x32       : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ModeCostInter32x32       : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   ModeCostInter32x16       : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   Reserved                 : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   LambdaIntra              : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   LambdaInter              : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW47;
    } Segments[8];

    /*
    Segment 0: DW32 - DW47
    Segment 1 : DW48 - DW63
    Segment 2 : DW64 - DW79
    Segment 3 : DW80 - DW95
    Segment 4 : DW96 - DW111
    Segment 5 : DW112 - DW127
    Segment 6 : DW128 - DW143
    Segment 7 : DW144 - DW159
    */

    // DW160
    union
    {
        struct
        {
            uint32_t   EncCurrYSurfBTI              : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW160;

    // DW161
    union
    {
        struct
        {
            uint32_t   Reserved                     : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW161;

    // DW162
    union
    {
        struct
        {
            uint32_t   EncCurrNV12SurfBTI           : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW162;

    // DW163
    union
    {
        struct
        {
            uint32_t   Reserved                     : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW163;

    // DW164
    union
    {
        struct
        {
            uint32_t   Reserved                     : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW164;

    // DW165
    union
    {
        struct
        {
            uint32_t   Reserved                     : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW165;

    // DW166
    union
    {
        struct
        {
            uint32_t   SegmentationMapBTI           : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW166;

    // DW167
    union
    {
        struct
        {
            uint32_t   TxCurbeBTI                   : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW167;

    // DW168
    union
    {
        struct
        {
            uint32_t   HMEMVDataBTI                : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW168;

    // DW169
    union
    {
        struct
        {
            uint32_t   HMEDistortionBTI             : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW169;

    // DW170
    union
    {
        struct
        {
            uint32_t   Reserved                     : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW170;

    // DW171
    union
    {
        struct
        {
            uint32_t   ModeDecisionPreviousBTI      : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW171;

    // DW172
    union
    {
        struct
        {
            uint32_t   ModeDecisionBTI              : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW172;

    // DW173
    union
    {
        struct
        {
            uint32_t   Output16x16InterModesBTI     : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW173;

    // DW174
    union
    {
        struct
        {
            uint32_t   CURecordBTI                  : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW174;

    // DW175
    union
    {
        struct
        {
            uint32_t   PakDataBTI                   : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW175;

} CODECHAL_ENCODE_VP9_MBENC_STATIC_DATA_G9, *PCODECHAL_ENCODE_VP9_MBENC_STATIC_DATA_G9;

C_ASSERT(SIZE32(CODECHAL_ENCODE_VP9_MBENC_STATIC_DATA_G9) == 176);

//BRC CURBE
typedef struct _CODECHAL_ENCODE_VP9_BRC_STATIC_DATA_G9
{
    // DW0
    union
    {
        struct
        {
            uint32_t   FrameWidth                   : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   FrameHeight                  : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   FrameType                   : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   SegmentationEnable          : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RefFrameFlags               : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   NumTLevels                  : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   IntraModeDisable            : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   LoopFilterType              : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MaxLevelRatioT0             : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MaxLevelRatioT1             : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MaxLevelRatioT2             : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   MaxLevelRatioT3             : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   ProfileLevelMaxFrame        : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   InitBufFullness             : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   BufSize                     : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   TargetBitRate               : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   MaxBitRate                  : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   MinBitRate                  : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   FrameRateM                  : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   FrameRateD                  : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   BRCFlag                     : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   GopP                        : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   InitFrameWidth              : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   InitFrameHeight             : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   AvbrAccuracy                : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   AvbrConvergence             : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   MinimumQP                   : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   MaximumQP                   : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   CQLevel                     : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   EnableDynamicScaling        : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   BrcOvershootCbrPct          : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   PFrameDeviationThreshold0   : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   PFrameDeviationThreshold1   : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   PFrameDeviationThreshold2   : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   PFrameDeviationThreshold3   : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   PFrameDeviationThreshold4   : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   PFrameDeviationThreshold5   : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   PFrameDeviationThreshold6   : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   PFrameDeviationThreshold7   : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   VbrDeviationThreshold0      : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   VbrDeviationThreshold1      : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   VbrDeviationThreshold2      : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   VbrDeviationThreshold3      : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   VbrDeviationThreshold4      : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   VbrDeviationThreshold5      : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   VbrDeviationThreshold6      : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   VbrDeviationThreshold7      : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   IFrameDeviationThreshold0   : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   IFrameDeviationThreshold1   : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   IFrameDeviationThreshold2   : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   IFrameDeviationThreshold3   : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   IFrameDeviationThreshold4   : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   IFrameDeviationThreshold5   : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   IFrameDeviationThreshold6   : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   IFrameDeviationThreshold7   : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   TargetSize                  : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   FrameNumber                 : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   HRDBufferFullnessUpperLimit : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   HRDBufferFullnessLowerLimit : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   SegDeltaQP0                 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   SegDeltaQP1                 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   SegDeltaQP2                 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   SegDeltaQP3                 : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   SegDeltaQP4                 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   SegDeltaQP5                 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   SegDeltaQP6                 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   SegDeltaQP7                 : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   TemporalID                  : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MultiRefQPCheck             : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   MaxNumPakPasses             : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   SyncAsync                   : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Overflow                    : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   MBRC                        : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Reserved1                   : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   Segmentation                : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Reserved2                   : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   CurQPIndex                  : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   LastRefQPIndex              : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   GoldRefQPIndex              : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   AltRefQPIndex               : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   QDeltaYDC                   : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   QDeltaUVAC                  : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   QDeltaUVDC                  : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   BrcY4XInputSurfBTI             : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   BrcVmeCoarseIntraInputSurfBTI  : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   BrcHistoryBufferBTI        : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   BrcConstDataInputSurfBTI    : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   BrcDistortionSurfBTI      : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   BrcMsdkPakOutputSurfBTI        : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   BrcEncCurbeInputSurfBTI        : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   BrcEncCurbeOutputSurfBTI       : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   BrcPicStateInputSurfBTI        : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   BrcPicStateOutputSurfBTI       : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   BrcSegStateInputSurfBTI        : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   BrcSegStateOutputSurfBTI       : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW59;

    // DW60
    union
    {
        struct
        {
            uint32_t   BrcBitstreamSizeDataSurfBTI    : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW60;

    // DW61
    union
    {
        struct
        {
            uint32_t   BrcHucDataOutputSurfBTI        : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW61;

    // DW62
    union
    {
        struct
        {
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW62;

    // DW63
    union
    {
        struct
        {
            uint32_t   Reserved                    : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW63;
} CODECHAL_ENCODE_VP9_BRC_STATIC_DATA_G9, *PCODECHAL_ENCODE_VP9_BRC_STATIC_DATA_G9;

C_ASSERT(SIZE32(CODECHAL_ENCODE_VP9_BRC_STATIC_DATA_G9) == 64);

class CodechalEncodeVp9G9 : public CodechalEncodeVp9
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalEncodeVp9G9(
        CodechalHwInterface*    hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeVp9G9() {};

protected:
    MOS_STATUS Initialize(CodechalSetting * codecHalSettings) override;

    MOS_STATUS InitKernelStateHelper(struct CodechalEncodeVp9InitKernelStateParams* params);

    MOS_STATUS InitKernelState() override;
    
    //!
    //! \brief    Initialize kernel state for ME
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateMe();

    //!
    //! \brief    Initialize kernel state for MBENC
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateMbEnc();

    //!
    //! \brief    Initialize kernel state for BRC
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateBrc();

    MOS_STATUS InitBrcConstantBuffer(struct CodechalVp9InitBrcConstantBufferParams*  params) override;
    MOS_STATUS SetBrcCurbe(struct CodechalVp9BrcCurbeParams* params) override;

    MOS_STATUS SetMbEncCurbe(struct CodechalVp9MbencCurbeParams* params) override;

    MOS_STATUS SetMeCurbe(struct CodechalVp9MeCurbeParams* params) override;

    MOS_STATUS SendBrcInitResetSurfaces(
          PMOS_COMMAND_BUFFER                         cmdBuffer,
          struct CodechalVp9BrcInitResetSurfaceParams* params) override;

    MOS_STATUS SendMbEncSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        struct CodechalVp9MbencSurfaceParams*  params,
	CODECHAL_MEDIA_STATE_TYPE EncFunctionType) override;

    MOS_STATUS SendMeSurfaces(
   	PMOS_COMMAND_BUFFER                pCmdBuffer,
        struct CodechalVp9MeSurfaceParams *pParams) override;

    MOS_STATUS SendBrcIntraDistSurfaces(
   	PMOS_COMMAND_BUFFER                pCmdBuffer,
        struct CodechalVp9BrcIntraDistSurfaceParams* pParams) override;

    MOS_STATUS SendBrcUpdateSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        struct CodechalVp9BrcUpdateSurfaceParams* params) override;

    //!
    //! \brief    Get Header and Size of Kernel
    //!
    //! \param    [in] binary
    //!           Pointer to void
    //! \param    [in] operation
    //!           Type of encode operation
    //! \param    [in] krnStateIdx
    //!           Index of Kernel State
    //! \param    [in] krnHeader
    //!           Pointer to void
    //! \param    [in] krnSize
    //!           Pointer to the kernel size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    static MOS_STATUS GetKernelHeaderAndSize(
        void                                *binary,
        EncOperation                        operation,
        uint32_t                            krnStateIdx,
        void                                *krnHeader,
        uint32_t                            *krnSize);
};

#endif  // __CODECHAL_ENCODER_VP9_G9_H__
