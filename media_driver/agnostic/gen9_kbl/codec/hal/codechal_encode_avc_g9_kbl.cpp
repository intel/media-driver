/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_encode_avc_g9_kbl.cpp
//! \brief    AVC dual-pipe encoder for GEN9 KBL.
//!
#include "codechal_encode_avc_g9_kbl.h"
#include "igcodeckrn_g9_kbl.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "mhw_vdbox_mfx_hwcmd_g9_kbl.h"
#endif

#define MBENC_NUM_TARGET_USAGES_CM_G9_KBL                       3
#define CODECHAL_ENCODE_AVC_BRC_HISTORY_BUFFER_SIZE_G9_KBL      880
#define MBENC_BRC_BUFFER_SIZE_G9_KBL                            128
#define BRC_CONSTANTSURFACE_LAMBDA_SIZE_G9_KBL                  512
#define BRC_CONSTANTSURFACE_FTQ25_SIZE_G9_KBL                   64

#define BRC_CONSTANTSURFACE_WIDTH_G9_KBL                        64
#define BRC_CONSTANTSURFACE_HEIGHT_G9_KBL                       53

#define DEFAULT_TRELLIS_QUANT_INTRA_ROUNDING_G9_KBL             5
#define CODECHAL_ENCODE_AVC_BRC_HISTORY_BUFFER_OFFSET_SCENE_CHANGED         0x2FC // (370 + 12)*2 = 764

typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_MBENC_G9_KBL
{
    CODECHAL_ENCODE_AVC_MBENC_BRC_CURBE_DATA_G9_KBL = 39,
    CODECHAL_ENCODE_AVC_MBENC_FORCE_NONSKIP_MB_MAP_G9_KBL = 40,
    CODECHAL_ENCODE_AVC_MBENC_ADV_WA_G9_KBL = 41,
    CODECHAL_ENCODE_AVC_MBENC_SFD_COST_TABLE_G9_KBL = 42,
    CODECHAL_ENCODE_AVC_MBENC_NUM_SURFACES_G9_KBL = 43
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_MBENC_G9_KBL;

typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_MB_BRC_UPDATE_G9_KBL
{
    CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_HISTORY_G9_KBL               = 0,
    CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_MB_QP_G9_KBL                 = 1,
    CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_ROI_G9_KBL                   = 2,
    CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_MB_STAT_G9_KBL               = 3,
    CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_NUM_SURFACES_G9_KBL          = 4
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_MB_BRC_UPDATE_G9_KBL;

typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_FRAME_BRC_UPDATE_G9_KBL
{
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_HISTORY_G9_KBL = 0,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_PAK_STATISTICS_OUTPUT_G9_KBL = 1,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_IMAGE_STATE_READ_G9_KBL = 2,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_IMAGE_STATE_WRITE_G9_KBL = 3,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_MBENC_CURBE_WRITE_G9_KBL = 4,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_DISTORTION_G9_KBL = 5,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_CONSTANT_DATA_G9_KBL = 6,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_MB_STAT_G9_KBL = 7,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_MV_G9_KBL = 8,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_NUM_SURFACES_G9_KBL=9
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_FRAME_BRC_UPDATE_G9_KBL;

static const CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G95 g_cInit_CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G95 =
{
    // uint32_t 0
    {
        {
            0
        }
    },

    // uint32_t 1
    {
        {
            0
        }
    },

    // uint32_t 2
    {
        {
            0
        }
    },

    // uint32_t 3
    {
        {
            0
        }
    },

    // uint32_t 4
    {
        {
            0
        }
    },

    // uint32_t 5
    {
        {
            0
        }
    },

    // uint32_t 6
    {
        {
            0
        }
    },

    // uint32_t 7
    {
        {
            0
        }
    },

    // uint32_t 8
    {
        {
            0,
            0
        }
    },

    // uint32_t 9
    {
        {
            0,
            0
        }
    },

    // uint32_t 10
    {
        {
            0,
            0
        }
    },

    // uint32_t 11
    {
        {
            0,
            1
        }
    },

    // uint32_t 12
    {
        {
            51,
            0
        }
    },

    // uint32_t 13
    {
        {
            40,
            60,
            80,
            120
        }
    },

    // uint32_t 14
    {
        {
            35,
            60,
            80,
            120
        }
    },

    // uint32_t 15
    {
        {
            40,
            60,
            90,
            115
        }
    },

    // uint32_t 16
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 17
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 18
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 19
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 20
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 21
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 22
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 23
    {
        {
            0
        }
    },

    // uint32_t 24
    {
        {
            0,
            0
        }
    },

    // uint32_t 25
    {
        {
            0
        }
    },

    // uint32_t 26
    {
        {
            0
        }
    },

    // uint32_t 27
    {
        {
            0
        }
    },

    // uint32_t 28
    {
        {
            0
        }
    },

    // uint32_t 29
    {
        {
            0
        }
    },

    // uint32_t 30
    {
        {
            0
        }
    },

    // uint32_t 31
    {
        {
            0
        }
    },

        // uint32_t 32
    {
        {
            0
        }
    },

        // uint32_t 33
    {
        {
            0
        }
    },

};

static const CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G95 g_cInit_CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G95 =
{
    // uint32_t 0
    {
        {
            0
        }
    },

    // uint32_t 1
    {
        {
            0
        }
    },

    // uint32_t 2
    {
        {
            0
        }
    },

    // uint32_t 3
    {
        {
            10,
            50
        }
    },

    // uint32_t 4
    {
        {
            100,
            150
        }
    },

    // uint32_t 5
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 6
    {
        {
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 7
    {
        {
            0
        }
    },

    // uint32_t 8
    {
        {
            1,
            1,
            3,
            2
        }
    },

    // uint32_t 9
    {
        {
            1,
            40,
            5,
            5
        }
    },

    // uint32_t 10
    {
        {
            3,
            1,
            7,
            18
        }
    },

    // uint32_t 11
    {
        {
            25,
            37,
            40,
            75
        }
    },

    // uint32_t 12
    {
        {
            97,
            103,
            125,
            160
        }
    },

    // uint32_t 13
    {
        {
            MOS_BITFIELD_VALUE((uint32_t)-3, 8),
            MOS_BITFIELD_VALUE((uint32_t)-2, 8),
            MOS_BITFIELD_VALUE((uint32_t)-1, 8),
            0
        }
    },

    // uint32_t 14
    {
        {
            1,
            2,
            3,
            0xff
        }
    },

    // uint32_t 15
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 16
    {
        {
            0
        }
    },

    // uint32_t 17
    {
        {
            0
        }
    },

    // uint32_t 18
    {
        {
            0
        }
    },

    // uint32_t 19
    {
        {
            0
        }
    },

    // uint32_t 20
    {
        {
            0
        }
    },

    // uint32_t 21
    {
        {
            0
        }
    },

    // uint32_t 22
    {
        {
            0
        }
    },

    // uint32_t 23
    {
        {
            0
        }
    },

        // uint32_t 24
    {
        {
            0
        }
    },

        // uint32_t 25
    {
        {
            0
        }
    },

        // uint32_t 26
    {
        {
            0
        }
    },

        // uint32_t 27
    {
        {
            0
        }
    },

        // uint32_t 28
    {
        {
            0
        }
    },

        // uint32_t 29
    {
        {
            0
        }
    },

        // uint32_t 30
    {
        {
            0
        }
    },
        // uint32_t 31
    {
        {
            0
        }
    },

        // uint32_t 32
    {
        {
            0
        }
    },
};

static const CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_CURBE_G95 g_cInit_CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_CURBE_G95 =
{
    // uint32_t 0
    {
        {
            0,
            0,
            0,
            0
        }
    },

    // uint32_t 1
    {
        {
            0
        }
    },

    // uint32_t 2
    {
        {
            0
        }
    },

    // uint32_t 3
    {
        {
            0
        }
    },

    // uint32_t 4
    {
        {
            0
        }
    },

    // uint32_t 5
    {
        {
            0
        }
    },

    // uint32_t 6
    {
        {
            0
        }
    },

    // uint32_t 7
    {
        {
            0
        }
    },

    // uint32_t 8
    {
        {
            0
        }
    },

    // uint32_t 9
    {
        {
            0
        }
    },

    // uint32_t 10
    {
        {
            0
        }
    },

    // uint32_t 11
    {
        {
            0
        }
    },
};

typedef struct _CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL
{
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn                          : MOS_BITFIELD_BIT(       0);
            uint32_t   AdaptiveEn                          : MOS_BITFIELD_BIT(       1);
            uint32_t   BiMixDis                            : MOS_BITFIELD_BIT(       2);
            uint32_t                                       : MOS_BITFIELD_RANGE( 3,  4);
            uint32_t   EarlyImeSuccessEn                   : MOS_BITFIELD_BIT(       5);
            uint32_t                                       : MOS_BITFIELD_BIT(       6);
            uint32_t   T8x8FlagForInterEn                  : MOS_BITFIELD_BIT(       7);
            uint32_t                                       : MOS_BITFIELD_RANGE( 8, 23);
            uint32_t   EarlyImeStop                        : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   MaxNumMVs                           : MOS_BITFIELD_RANGE( 0,  5);
            uint32_t   ExtendedMvCostRange                 : MOS_BITFIELD_BIT(       6);
            uint32_t                                       : MOS_BITFIELD_RANGE( 7, 15);
            uint32_t   BiWeight                            : MOS_BITFIELD_RANGE(16, 21);
            uint32_t                                       : MOS_BITFIELD_RANGE(22, 27);
            uint32_t   UniMixDisable                       : MOS_BITFIELD_BIT(      28);
            uint32_t                                       : MOS_BITFIELD_RANGE(29, 31);
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
            uint32_t   LenSP                               : MOS_BITFIELD_RANGE( 0,  7);
            uint32_t   MaxNumSU                            : MOS_BITFIELD_RANGE( 8, 15);
            uint32_t   PicWidth                            : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   SrcSize                             : MOS_BITFIELD_RANGE( 0,  1);
            uint32_t                                       : MOS_BITFIELD_RANGE( 2,  3);
            uint32_t   MbTypeRemap                         : MOS_BITFIELD_RANGE( 4,  5);
            uint32_t   SrcAccess                           : MOS_BITFIELD_BIT(       6);
            uint32_t   RefAccess                           : MOS_BITFIELD_BIT(       7);
            uint32_t   SearchCtrl                          : MOS_BITFIELD_RANGE( 8, 10);
            uint32_t   DualSearchPathOption                : MOS_BITFIELD_BIT(      11);
            uint32_t   SubPelMode                          : MOS_BITFIELD_RANGE(12, 13);
            uint32_t   SkipType                            : MOS_BITFIELD_BIT(      14);
            uint32_t   DisableFieldCacheAlloc              : MOS_BITFIELD_BIT(      15);
            uint32_t   InterChromaMode                     : MOS_BITFIELD_BIT(      16);
            uint32_t   FTEnable                            : MOS_BITFIELD_BIT(      17);
            uint32_t   BMEDisableFBR                       : MOS_BITFIELD_BIT(      18);
            uint32_t   BlockBasedSkipEnable                : MOS_BITFIELD_BIT(      19);
            uint32_t   InterSAD                            : MOS_BITFIELD_RANGE(20, 21);
            uint32_t   IntraSAD                            : MOS_BITFIELD_RANGE(22, 23);
            uint32_t   SubMbPartMask                       : MOS_BITFIELD_RANGE(24, 30);
            uint32_t                                       : MOS_BITFIELD_BIT(      31);
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
            uint32_t   PicHeightMinus1                     : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   MvRestrictionInSliceEnable          : MOS_BITFIELD_BIT(      16);
            uint32_t   DeltaMvEnable                       : MOS_BITFIELD_BIT(      17);
            uint32_t   TrueDistortionEnable                : MOS_BITFIELD_BIT(      18);
            uint32_t   EnableWavefrontOptimization         : MOS_BITFIELD_BIT(      19);
            uint32_t   EnableFBRBypass                     : MOS_BITFIELD_BIT(      20);
            uint32_t   EnableIntraCostScalingForStaticFrame: MOS_BITFIELD_BIT(      21);
            uint32_t   EnableIntraRefresh                  : MOS_BITFIELD_BIT(      22);
            uint32_t   Reserved                            : MOS_BITFIELD_BIT(      23);
            uint32_t   EnableDirtyRect                     : MOS_BITFIELD_BIT(      24);
            uint32_t   bCurFldIDR                          : MOS_BITFIELD_BIT(      25);
            uint32_t   ConstrainedIntraPredFlag            : MOS_BITFIELD_BIT(      26);
            uint32_t   FieldParityFlag                     : MOS_BITFIELD_BIT(      27);
            uint32_t   HMEEnable                           : MOS_BITFIELD_BIT(      28);
            uint32_t   PictureType                         : MOS_BITFIELD_RANGE(29, 30);
            uint32_t   UseActualRefQPValue                 : MOS_BITFIELD_BIT(      31);
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
            uint32_t   SliceMbHeight                       : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   RefWidth                            : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   RefHeight                           : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   BatchBufferEnd                      : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   IntraPartMask                       : MOS_BITFIELD_RANGE( 0,  4);
            uint32_t   NonSkipZMvAdded                     : MOS_BITFIELD_BIT(       5);
            uint32_t   NonSkipModeAdded                    : MOS_BITFIELD_BIT(       6);
            uint32_t   LumaIntraSrcCornerSwap              : MOS_BITFIELD_BIT(       7);
            uint32_t                                       : MOS_BITFIELD_RANGE( 8, 15);
            uint32_t   MVCostScaleFactor                   : MOS_BITFIELD_RANGE(16, 17);
            uint32_t   BilinearEnable                      : MOS_BITFIELD_BIT(      18);
            uint32_t   SrcFieldPolarity                    : MOS_BITFIELD_BIT(      19);
            uint32_t   WeightedSADHAAR                     : MOS_BITFIELD_BIT(      20);
            uint32_t   AConlyHAAR                          : MOS_BITFIELD_BIT(      21);
            uint32_t   RefIDCostMode                       : MOS_BITFIELD_BIT(      22);
            uint32_t                                       : MOS_BITFIELD_BIT(      23);
            uint32_t   SkipCenterMask                      : MOS_BITFIELD_RANGE(24, 31);
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
                uint32_t   Mode0Cost : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   Mode1Cost : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   Mode2Cost : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   Mode3Cost : MOS_BITFIELD_RANGE(24, 31);
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
                uint32_t   Mode4Cost : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   Mode5Cost : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   Mode6Cost : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   Mode7Cost : MOS_BITFIELD_RANGE(24, 31);
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
                uint32_t   Mode8Cost : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   Mode9Cost : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   RefIDCost : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   ChromaIntraModeCost : MOS_BITFIELD_RANGE(24, 31);
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
                uint32_t   MV0Cost : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   MV1Cost : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   MV2Cost : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   MV3Cost : MOS_BITFIELD_RANGE(24, 31);
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
                uint32_t   MV4Cost : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   MV5Cost : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   MV6Cost : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   MV7Cost : MOS_BITFIELD_RANGE(24, 31);
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
                uint32_t   QpPrimeY : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   QpPrimeCb : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   QpPrimeCr : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   TargetSizeInWord : MOS_BITFIELD_RANGE(24, 31);
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
                uint32_t   SICFwdTransCoeffThreshold_0 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   SICFwdTransCoeffThreshold_1 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   SICFwdTransCoeffThreshold_2 : MOS_BITFIELD_RANGE(24, 31);
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
                uint32_t   SICFwdTransCoeffThreshold_3 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   SICFwdTransCoeffThreshold_4 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   SICFwdTransCoeffThreshold_5 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   SICFwdTransCoeffThreshold_6 : MOS_BITFIELD_RANGE(24, 31);    // Highest Freq
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
                uint32_t   Intra4x4ModeMask : MOS_BITFIELD_RANGE(0, 8);
            uint32_t: MOS_BITFIELD_RANGE(9, 15);
                uint32_t   Intra8x8ModeMask : MOS_BITFIELD_RANGE(16, 24);
            uint32_t: MOS_BITFIELD_RANGE(25, 31);
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
                uint32_t   Intra16x16ModeMask : MOS_BITFIELD_RANGE(0, 3);
                uint32_t   IntraChromaModeMask : MOS_BITFIELD_RANGE(4, 7);
                uint32_t   IntraComputeType : MOS_BITFIELD_RANGE(8, 9);
            uint32_t: MOS_BITFIELD_RANGE(10, 31);
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
            uint32_t   SkipVal                             : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   MultiPredL0Disable                  : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   MultiPredL1Disable                  : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Intra16x16NonDCPredPenalty          : MOS_BITFIELD_RANGE( 0,  7);
            uint32_t   Intra8x8NonDCPredPenalty            : MOS_BITFIELD_RANGE( 8, 15);
            uint32_t   Intra4x4NonDCPredPenalty            : MOS_BITFIELD_RANGE(16, 23);
            uint32_t                                       : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   List0RefID0FieldParity              : MOS_BITFIELD_BIT(       0);
            uint32_t   List0RefID1FieldParity              : MOS_BITFIELD_BIT(       1);
            uint32_t   List0RefID2FieldParity              : MOS_BITFIELD_BIT(       2);
            uint32_t   List0RefID3FieldParity              : MOS_BITFIELD_BIT(       3);
            uint32_t   List0RefID4FieldParity              : MOS_BITFIELD_BIT(       4);
            uint32_t   List0RefID5FieldParity              : MOS_BITFIELD_BIT(       5);
            uint32_t   List0RefID6FieldParity              : MOS_BITFIELD_BIT(       6);
            uint32_t   List0RefID7FieldParity              : MOS_BITFIELD_BIT(       7);
            uint32_t   List1RefID0FrameFieldFlag           : MOS_BITFIELD_BIT(       8);
            uint32_t   List1RefID1FrameFieldFlag           : MOS_BITFIELD_BIT(       9);
            uint32_t   IntraRefreshEn                      : MOS_BITFIELD_RANGE(10, 11);
            uint32_t   ArbitraryNumMbsPerSlice             : MOS_BITFIELD_BIT(      12);
            uint32_t   TQEnable                            : MOS_BITFIELD_BIT(      13);
            uint32_t   ForceNonSkipMbEnable                : MOS_BITFIELD_BIT(      14);
            uint32_t   DisableEncSkipCheck                 : MOS_BITFIELD_BIT(      15);
            uint32_t   EnableDirectBiasAdjustment          : MOS_BITFIELD_BIT(      16);
            uint32_t   bForceToSkip                        : MOS_BITFIELD_BIT(      17);
            uint32_t   EnableGlobalMotionBiasAdjustment    : MOS_BITFIELD_BIT(      18);
            uint32_t   EnableAdaptiveTxDecision            : MOS_BITFIELD_BIT(      19);
            uint32_t   EnablePerMBStaticCheck              : MOS_BITFIELD_BIT(      20);
            uint32_t   EnableAdaptiveSearchWindowSize      : MOS_BITFIELD_BIT(      21);
            uint32_t   RemoveIntraRefreshOverlap           : MOS_BITFIELD_BIT(      22);
            uint32_t   CQPFlag                             : MOS_BITFIELD_BIT(      23);
            uint32_t   List1RefID0FieldParity              : MOS_BITFIELD_BIT(      24);
            uint32_t   List1RefID1FieldParity              : MOS_BITFIELD_BIT(      25);
            uint32_t   MADEnableFlag                       : MOS_BITFIELD_BIT(      26);
            uint32_t   ROIEnableFlag                       : MOS_BITFIELD_BIT(      27);
            uint32_t   EnableMBFlatnessChkOptimization     : MOS_BITFIELD_BIT(      28);
            uint32_t   bDirectMode                         : MOS_BITFIELD_BIT(      29);
            uint32_t   MBBrcEnable                         : MOS_BITFIELD_BIT(      30);
            uint32_t   bOriginalBff                        : MOS_BITFIELD_BIT(      31);
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
            uint32_t   PanicModeMBThreshold                : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   SmallMbSizeInWord                   : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   LargeMbSizeInWord                   : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   NumRefIdxL0MinusOne                 : MOS_BITFIELD_RANGE( 0,  7);
            uint32_t   HMECombinedExtraSUs                 : MOS_BITFIELD_RANGE( 8, 15);
            uint32_t   NumRefIdxL1MinusOne                 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t                                       : MOS_BITFIELD_RANGE(24, 26);
            uint32_t   MBInputEnable                       : MOS_BITFIELD_BIT(      27);
            uint32_t   IsFwdFrameShortTermRef              : MOS_BITFIELD_BIT(      28);
            uint32_t   CheckAllFractionalEnable            : MOS_BITFIELD_BIT(      29);
            uint32_t   HMECombineOverlap                   : MOS_BITFIELD_RANGE(30, 31);
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
            uint32_t   SkipModeEn                          : MOS_BITFIELD_BIT(       0);
            uint32_t   AdaptiveEn                          : MOS_BITFIELD_BIT(       1);
            uint32_t   BiMixDis                            : MOS_BITFIELD_BIT(       2);
            uint32_t                                       : MOS_BITFIELD_RANGE( 3,  4);
            uint32_t   EarlyImeSuccessEn                   : MOS_BITFIELD_BIT(       5);
            uint32_t                                       : MOS_BITFIELD_BIT(       6);
            uint32_t   T8x8FlagForInterEn                  : MOS_BITFIELD_BIT(       7);
            uint32_t                                       : MOS_BITFIELD_RANGE( 8, 23);
            uint32_t   EarlyImeStop                        : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   LenSP                               : MOS_BITFIELD_RANGE( 0,  7);
            uint32_t   MaxNumSU                            : MOS_BITFIELD_RANGE( 8, 15);
            uint32_t   RefThreshold                        : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t                                       : MOS_BITFIELD_RANGE( 0,  7);
            uint32_t   HMERefWindowsCombThreshold          : MOS_BITFIELD_RANGE( 8, 15);
            uint32_t   RefWidth                            : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   RefHeight                           : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   DistScaleFactorRefID0List0          : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   DistScaleFactorRefID1List0          : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   DistScaleFactorRefID2List0          : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   DistScaleFactorRefID3List0          : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   DistScaleFactorRefID4List0          : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   DistScaleFactorRefID5List0          : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   DistScaleFactorRefID6List0          : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   DistScaleFactorRefID7List0          : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ActualQPValueForRefID0List0         : MOS_BITFIELD_RANGE( 0,  7);
            uint32_t   ActualQPValueForRefID1List0         : MOS_BITFIELD_RANGE( 8, 15);
            uint32_t   ActualQPValueForRefID2List0         : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ActualQPValueForRefID3List0         : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   ActualQPValueForRefID4List0         : MOS_BITFIELD_RANGE( 0,  7);
            uint32_t   ActualQPValueForRefID5List0         : MOS_BITFIELD_RANGE( 8, 15);
            uint32_t   ActualQPValueForRefID6List0         : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ActualQPValueForRefID7List0         : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   ActualQPValueForRefID0List1         : MOS_BITFIELD_RANGE( 0,  7);
            uint32_t   ActualQPValueForRefID1List1         : MOS_BITFIELD_RANGE( 8, 15);
            uint32_t   RefCost                             : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   MbQpReadFactor                      : MOS_BITFIELD_RANGE( 0,  7);
            uint32_t   IntraCostSF                         : MOS_BITFIELD_RANGE( 8, 15);
            uint32_t   MaxVmvR                             : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   IntraRefreshMBx                     : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   IntraRefreshUnitInMBMinus1          : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   IntraRefreshQPDelta                 : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   ROI1_X_left                         : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   ROI1_Y_top                          : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ROI1_X_right                        : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   ROI1_Y_bottom                       : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ROI2_X_left                         : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   ROI2_Y_top                          : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ROI2_X_right                        : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   ROI2_Y_bottom                       : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ROI3_X_left                         : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   ROI3_Y_top                          : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ROI3_X_right                        : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   ROI3_Y_bottom                       : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ROI4_X_left                         : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   ROI4_Y_top                          : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ROI4_X_right                        : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   ROI4_Y_bottom                       : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   ROI1_dQpPrimeY                      : MOS_BITFIELD_RANGE( 0,  7);
            uint32_t   ROI2_dQpPrimeY                      : MOS_BITFIELD_RANGE( 8, 15);
            uint32_t   ROI3_dQpPrimeY                      : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   ROI4_dQpPrimeY                      : MOS_BITFIELD_RANGE(24, 31);
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
            uint32_t   Lambda_8x8Inter                     : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   Lambda_8x8Intra                     : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   Lambda_Inter                        : MOS_BITFIELD_RANGE( 0, 15);
            uint32_t   Lambda_Intra                        : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   MBTextureThreshold                  : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   TxDecisonThreshold                  : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   HMEMVCostScalingFactor              : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   IntraRefreshMBy                     : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t   IPCM_QP0                            : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   IPCM_QP1                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   IPCM_QP2                            : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   IPCM_QP3                            : MOS_BITFIELD_RANGE( 24,31 );
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
            uint32_t   IPCM_QP4                            : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   IPCM_Thresh0                        : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   IPCM_Thresh1                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   IPCM_Thresh2                        : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   IPCM_Thresh3                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   IPCM_Thresh4                        : MOS_BITFIELD_RANGE( 16,31 );
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
            uint32_t   MBDataSurfIndex                     : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   MVDataSurfIndex                     : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   IDistSurfIndex                      : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   SrcYSurfIndex                       : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   MBSpecificDataSurfIndex             : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   AuxVmeOutSurfIndex                  : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   CurrRefPicSelSurfIndex              : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   HMEMVPredFwdBwdSurfIndex            : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   HMEDistSurfIndex                    : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   SliceMapSurfIndex                   : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   FwdFrmMBDataSurfIndex               : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   FwdFrmMVSurfIndex                   : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   MBQPBuffer                          : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   MBBRCLut                            : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   VMEInterPredictionSurfIndex         : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   VMEInterPredictionMRSurfIndex       : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   MBStatsSurfIndex                : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   MADSurfIndex                        : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   BRCCurbeSurfIndex                   : MOS_BITFIELD_RANGE( 0, 31);
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
            uint32_t   ForceNonSkipMBmapSurface : MOS_BITFIELD_RANGE(0, 31);
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
             uint32_t   ReservedIndex                      : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   StaticDetectionCostTableIndex       : MOS_BITFIELD_RANGE( 0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW87;

} CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL, *PCODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL)) == 88);

static const uint32_t CODECHAL_ENCODE_AVC_TrellisQuantizationRounding_G9_KBL[NUM_TARGET_USAGE_MODES] =
{
    0, 3, 0, 0, 0, 0, 0, 0
};

// AVC MBEnc CURBE init data for KBL Kernel
const uint32_t CodechalEncodeAvcEncG9Kbl::MBEnc_CURBE_normal_I_frame[88] =
{
    0x00000082, 0x00000000, 0x00003910, 0x00a83000, 0x00000000, 0x28300000, 0x05000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80800000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncodeAvcEncG9Kbl::MBEnc_CURBE_normal_I_field[88] =
{
    0x00000082, 0x00000000, 0x00003910, 0x00a830c0, 0x02000000, 0x28300000, 0x05000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80800000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncodeAvcEncG9Kbl::MBEnc_CURBE_normal_P_frame[88] =
{
    0x000000a3, 0x00000008, 0x00003910, 0x00ae3000, 0x30000000, 0x28300000, 0x05000000, 0x01400060,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x60000000, 0x000000a1, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x08000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncodeAvcEncG9Kbl::MBEnc_CURBE_normal_P_field[88] =
{
    0x000000a3, 0x00000008, 0x00003910, 0x00ae30c0, 0x30000000, 0x28300000, 0x05000000, 0x01400060,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x000000a1, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncodeAvcEncG9Kbl::MBEnc_CURBE_normal_B_frame[88] =
{
    0x000000a3, 0x00200008, 0x00003910, 0x00aa7700, 0x50020000, 0x20200000, 0x05000000, 0xff400000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x01010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x60000000, 0x000000a1, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x08000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncodeAvcEncG9Kbl::MBEnc_CURBE_normal_B_field[88] =
{
    0x000000a3, 0x00200008, 0x00003919, 0x00aa77c0, 0x50020000, 0x20200000, 0x05000000, 0xff400000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x01010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x000000a1, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000
};

// AVC I_DIST CURBE init data for KBL CM Kernel
const uint32_t CodechalEncodeAvcEncG9Kbl::MBEnc_CURBE_I_frame_DIST[88] =
{
    0x00000082, 0x00200008, 0x001e3910, 0x00a83000, 0x90000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000100,
    0x80800000, 0x00000000, 0x00000800, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const uint16_t CodechalEncodeAvcEncG9Kbl::Lambda_data[256] = {
     9,     7,     9,     6,    12,     8,    12,     8,    15,    10,    15,     9,    19,    13,    19,    12,    24,
    17,    24,    15,    30,    21,    30,    19,    38,    27,    38,    24,    48,    34,    48,    31,    60,    43,
    60,    39,    76,    54,    76,    49,    96,    68,    96,    62,   121,    85,   121,    78,   153,   108,   153,
    99,   193,   135,   193,   125,   243,   171,   243,   157,   306,   215,   307,   199,   385,   271,   387,   251,
   485,   342,   488,   317,   612,   431,   616,   400,   771,   543,   777,   505,   971,   684,   981,   638,  1224,
   862,  1237,   806,  1542,  1086,  1562,  1018,  1991,  1402,  1971,  1287,  2534,  1785,  2488,  1626,  3077,  2167,
  3141,  2054,  3982,  2805,  3966,  2596,  4887,  3442,  5007,  3281,  6154,  4335,  6322,  4148,  7783,  5482,  7984,
  5243,  9774,  6885, 10082,  6629, 12489,  8797, 12733,  8382, 15566, 10965, 16082, 10599, 19729, 13897, 20313, 13404,
 24797, 17467, 25660, 16954, 31313, 22057, 32415, 21445, 39458, 27795, 40953, 27129, 49594, 34935, 51742, 34323, 61440,
 43987, 61440, 43428, 61440, 55462, 61440, 54954, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440,
 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440,
 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440,
 61440, 61440, 61440, 61440,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,
};

const uint8_t CodechalEncodeAvcEncG9Kbl::FTQ25[64] = //27 value 4 dummy
{
    0,                                      //qp=0
    0, 0, 0, 0, 0, 0,                       //qp=1,2;3,4;5,6;
    1, 1, 3, 3, 6, 6, 8, 8, 11, 11,         //qp=7,8;9,10;11,12;13,14;15;16
    13, 13, 16, 16, 19, 19, 22, 22, 26, 26, //qp=17,18;19,20;21,22;23,24;25,26
    30, 30, 34, 34, 39, 39, 44, 44, 50, 50, //qp=27,28;29,30;31,32;33,34;35,36
    56, 56, 62, 62, 69, 69, 77, 77, 85, 85, //qp=37,38;39,40;41,42;43,44;45,46
    94, 94, 104, 104, 115, 115,             //qp=47,48;49,50;51
    0, 0, 0, 0, 0, 0, 0, 0                  //dummy
};

const int32_t CodechalEncodeAvcEncG9Kbl::BRC_BTCOUNTS[CODECHAL_ENCODE_BRC_IDX_NUM] = {
    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_NUM_SURFACES,
    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_NUM_SURFACES_G9_KBL,
    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_NUM_SURFACES,
    CODECHAL_ENCODE_AVC_MBENC_NUM_SURFACES_CM_G9,
    CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_NUM_SURFACES,
    CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_NUM_SURFACES_G9_KBL     // MbBRCUpdate kernel starting GEN9
};

const int32_t CodechalEncodeAvcEncG9Kbl::BRC_CURBE_SIZE[CODECHAL_ENCODE_BRC_IDX_NUM] = {
    (sizeof(CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G95)),
    (sizeof(CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G95)),
    (sizeof(CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G95)),
    (sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL)),
    0,
    (sizeof(CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_CURBE_G95))     // MbBRCUpdate kernel starting GEN9
};

typedef struct _CODECHAL_ENCODE_AVC_KERNEL_HEADER_G9_KBL {
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

    // BRC init frame
    CODECHAL_KERNEL_HEADER InitFrameBRC;

    // Frame BRC update
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

} CODECHAL_ENCODE_AVC_KERNEL_HEADER_G9_KBL, *PCODECHAL_ENCODE_AVC_KERNEL_HEADER_G9_KBL;

MOS_STATUS CodechalEncodeAvcEncG9Kbl::GetKernelHeaderAndSize(
    void                           *binary,
    EncOperation                   operation,
    uint32_t                       krnStateIdx,
    void                           *krnHeader,
    uint32_t                       *krnSize)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    auto kernelHeaderTable = (PCODECHAL_ENCODE_AVC_KERNEL_HEADER_G9_KBL)binary;

    auto invalidEntry = &(kernelHeaderTable->AVC_StaticFrameDetection) + 1;
    uint32_t nextKrnOffset = *krnSize;
    PCODECHAL_KERNEL_HEADER currKrnHeader = nullptr;

    if (operation == ENC_SCALING4X)
    {
        currKrnHeader = &kernelHeaderTable->PLY_DScale_PLY;
    }
    else if (operation == ENC_SCALING2X)
    {
        currKrnHeader = &kernelHeaderTable->PLY_2xDScale_PLY;
    }
    else if (operation == ENC_ME)
    {
        currKrnHeader = &kernelHeaderTable->AVC_ME_P;
    }
    else if (operation == VDENC_ME)
    {
        currKrnHeader = &kernelHeaderTable->AVC_ME_VDENC;
    }
    else if (operation == ENC_BRC)
    {
        currKrnHeader = &kernelHeaderTable->InitFrameBRC;
    }
    else if (operation == ENC_MBENC)
    {
        currKrnHeader = &kernelHeaderTable->AVCMBEnc_Qlty_I;
    }
    else if (operation == ENC_MBENC_ADV)
    {
        currKrnHeader = &kernelHeaderTable->AVCMBEnc_Adv_I;
    }
    else if (operation == ENC_WP)
    {
        currKrnHeader = &kernelHeaderTable->AVC_WeightedPrediction;
    }
    else if (operation == ENC_SFD)
    {
        currKrnHeader = &kernelHeaderTable->AVC_StaticFrameDetection;
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    currKrnHeader += krnStateIdx;
    *((PCODECHAL_KERNEL_HEADER)krnHeader) = *currKrnHeader;

    auto nextKrnHeader = (currKrnHeader + 1);
    if (nextKrnHeader < invalidEntry)
    {
        nextKrnOffset = nextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *krnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Kbl::SetCurbeAvcMbEnc(
        PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS params)
{

    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSlcParams);

    auto picParams = params->pPicParams;
    auto seqParams = params->pSeqParams;
    auto slcParams = params->pSlcParams;

    MHW_VDBOX_AVC_SLICE_STATE sliceState;
    MOS_ZeroMemory(&sliceState, sizeof(sliceState));
    sliceState.pEncodeAvcSeqParams = seqParams;
    sliceState.pEncodeAvcPicParams = picParams;
    sliceState.pEncodeAvcSliceParams = slcParams;

    CODECHAL_ENCODE_ASSERT(seqParams->TargetUsage < NUM_TARGET_USAGE_MODES);

    uint8_t meMethod =
        (m_pictureCodingType == B_TYPE) ? m_bMeMethodGeneric[seqParams->TargetUsage] : m_meMethodGeneric[seqParams->TargetUsage];
    // set sliceQP to MAX_SLICE_QP for MbEnc Adv kernel, we can use it to verify whether QP is changed or not
    uint8_t sliceQP = (params->bUseMbEncAdvKernel && params->bBrcEnabled) ? CODECHAL_ENCODE_AVC_MAX_SLICE_QP : picParams->pic_init_qp_minus26 + 26 + slcParams->slice_qp_delta;
    bool framePicture = CodecHal_PictureIsFrame(picParams->CurrOriginalPic);
    bool topField = CodecHal_PictureIsTopField(picParams->CurrOriginalPic);
    bool bottomField = CodecHal_PictureIsBottomField(picParams->CurrOriginalPic);

    CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL cmd;

    if (params->bMbEncIFrameDistEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &cmd,
            sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL),
            MBEnc_CURBE_I_frame_DIST,
            sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL)));
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
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL),
                    MBEnc_CURBE_normal_I_frame,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL)));
            }
            else
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &cmd,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL),
                    MBEnc_CURBE_normal_I_field,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL)));
            }
            break;

        case P_TYPE:
            if (framePicture)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &cmd,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL),
                    MBEnc_CURBE_normal_P_frame,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL)));
            }
            else
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &cmd,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL),
                    MBEnc_CURBE_normal_P_field,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL)));
            }
            break;

        case B_TYPE:
            if (framePicture)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &cmd,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL),
                    MBEnc_CURBE_normal_B_frame,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL)));
            }
            else
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &cmd,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL),
                    MBEnc_CURBE_normal_B_field,
                    sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL)));
            }
            break;

        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture coding type.");
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }
    }

    // r1
    cmd.DW0.AdaptiveEn =
        cmd.DW37.AdaptiveEn = EnableAdaptiveSearch[seqParams->TargetUsage];
    cmd.DW0.T8x8FlagForInterEn =
        cmd.DW37.T8x8FlagForInterEn = picParams->transform_8x8_mode_flag;
    cmd.DW2.LenSP = MaxLenSP[seqParams->TargetUsage];

    cmd.DW1.ExtendedMvCostRange = bExtendedMvCostRange;
    cmd.DW36.MBInputEnable = bMbSpecificDataEnabled;
    cmd.DW38.LenSP = 0; // MBZ
    cmd.DW3.SrcAccess =
        cmd.DW3.RefAccess = framePicture ? 0 : 1;
    if (m_pictureCodingType != I_TYPE &&  bFTQEnable)
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
    if (picParams->UserFlags.bDisableSubMBPartition)
    {
        cmd.DW3.SubMbPartMask = CODECHAL_ENCODE_AVC_DISABLE_4X4_SUB_MB_PARTITION | CODECHAL_ENCODE_AVC_DISABLE_4X8_SUB_MB_PARTITION | CODECHAL_ENCODE_AVC_DISABLE_8X4_SUB_MB_PARTITION;
    }
    cmd.DW2.PicWidth                             = params->wPicWidthInMb;
    cmd.DW4.PicHeightMinus1                      = params->wFieldFrameHeightInMb - 1;
    cmd.DW4.FieldParityFlag                      = cmd.DW7.SrcFieldPolarity = bottomField ? 1 : 0;
    cmd.DW4.EnableFBRBypass                      =  bFBRBypassEnable;
    cmd.DW4.EnableIntraCostScalingForStaticFrame = params->bStaticFrameDetectionEnabled;
    cmd.DW4.bCurFldIDR                           = framePicture ? 0 : (picParams->bIdrPic || m_firstFieldIdrPic);
    cmd.DW4.ConstrainedIntraPredFlag             = picParams->constrained_intra_pred_flag;
    cmd.DW4.HMEEnable                            = m_hmeEnabled;
    cmd.DW4.PictureType                          = m_pictureCodingType - 1;
    cmd.DW4.UseActualRefQPValue                  = m_hmeEnabled ? (MRDisableQPCheck[seqParams->TargetUsage] == 0) : false;
    cmd.DW5.SliceMbHeight                        = params->usSliceHeight;
    cmd.DW7.IntraPartMask                        = picParams->transform_8x8_mode_flag ? 0 : 0x2;  // Disable 8x8 if flag is not set

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

        if (m_pictureCodingType == I_TYPE &&  bOldModeCostEnable)
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
        eStatus = MOS_SecureMemcpy(&(cmd.SPDelta), 16 * sizeof(uint32_t), CodechalEncoderState::m_encodeSearchPath[tableIdx][meMethod], 16 * sizeof(uint32_t));
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

    if ( bMultiPredEnable && (m_pictureCodingType != I_TYPE))
    {
        switch (MultiPred[seqParams->TargetUsage])
        {
        case 0: // Disable multipred for both P & B picture types
            cmd.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            cmd.DW32.MultiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;

        case 1: // Enable multipred for P pictures only
            cmd.DW32.MultiPredL0Disable = (m_pictureCodingType == P_TYPE) ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            cmd.DW32.MultiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;

        case 2: // Enable multipred for B pictures only
            cmd.DW32.MultiPredL0Disable = (m_pictureCodingType == B_TYPE) ?
            CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            cmd.DW32.MultiPredL1Disable = (m_pictureCodingType == B_TYPE) ?
            CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;

        case 3: // Enable multipred for both P & B picture types
            cmd.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE;
            cmd.DW32.MultiPredL1Disable = (m_pictureCodingType == B_TYPE) ?
            CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;
        }
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

    if (m_adaptiveTransformDecisionEnabled)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            cmd.DW34.EnableAdaptiveTxDecision = true;
        }
        cmd.DW60.TxDecisonThreshold = CODECHAL_ENCODE_AVC_ADAPTIVE_TX_DECISION_THRESHOLD_G9;
    }
    if (m_adaptiveTransformDecisionEnabled || m_flatnessCheckEnabled)
    {
        cmd.DW60.MBTextureThreshold = CODECHAL_ENCODE_AVC_MB_TEXTURE_THRESHOLD_G9;
    }
    if (m_pictureCodingType == B_TYPE)
    {
        cmd.DW34.List1RefID0FrameFieldFlag = GetRefPicFieldFlag(params, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        cmd.DW34.List1RefID1FrameFieldFlag = GetRefPicFieldFlag(params, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        cmd.DW34.bDirectMode = slcParams->direct_spatial_mv_pred_flag;
    }

    cmd.DW34.EnablePerMBStaticCheck         = params->bStaticFrameDetectionEnabled;
    cmd.DW34.EnableAdaptiveSearchWindowSize = params->bApdatvieSearchWindowSizeEnabled;
    cmd.DW34.RemoveIntraRefreshOverlap      = picParams->bDisableRollingIntraRefreshOverlap;
    cmd.DW34.bOriginalBff = framePicture ? 0 :
        ((m_firstField && (bottomField)) || (!m_firstField && (!bottomField)));
    cmd.DW34.EnableMBFlatnessChkOptimization = m_flatnessCheckEnabled;
    cmd.DW34.ROIEnableFlag = params->bRoiEnabled;
    cmd.DW34.MADEnableFlag                   = m_madEnabled;
    cmd.DW34.MBBrcEnable =  bMbBrcEnabled ||  bMbQpDataEnabled;
    cmd.DW34.ArbitraryNumMbsPerSlice = m_arbitraryNumMbsInSlice;
    cmd.DW34.ForceNonSkipMbEnable = params->bMbDisableSkipMapEnabled;
    if (params->pAvcQCParams && !cmd.DW34.ForceNonSkipMbEnable) // ignore DisableEncSkipCheck if Mb Disable Skip Map is available
    {
        cmd.DW34.DisableEncSkipCheck = params->pAvcQCParams->skipCheckDisable;
    }
    cmd.DW34.TQEnable = m_trellisQuantParams.dwTqEnabled; //Enabled for KBL
    cmd.DW34.CQPFlag = ! bBrcEnabled; // 1 - Rate Control is CQP, 0 - Rate Control is BRC
    cmd.DW36.CheckAllFractionalEnable =  bCAFEnable;
    cmd.DW38.RefThreshold = m_refThreshold;
    cmd.DW39.HMERefWindowsCombThreshold = (m_pictureCodingType == B_TYPE) ?
        HMEBCombineLen[seqParams->TargetUsage] : HMECombineLen[seqParams->TargetUsage];

    // Default:2 used for MBBRC (MB QP Surface width and height are 4x downscaled picture in MB unit * 4  bytes)
    // 0 used for MBQP data surface (MB QP Surface width and height are same as the input picture size in MB unit * 1bytes)
    // BRC use split kernel, MB QP surface is same size as input picture
    cmd.DW47.MbQpReadFactor = ( bMbBrcEnabled ||  bMbQpDataEnabled) ? 0 : 2;

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

    // Picture Coding Type dependent parameters
    if (m_pictureCodingType == I_TYPE)
    {
        cmd.DW0.SkipModeEn = 0;
        cmd.DW37.SkipModeEn = 0;
        cmd.DW36.HMECombineOverlap = 0;
        cmd.DW47.IntraCostSF = 16; // This is not used but recommended to set this to 16 by Kernel team
        cmd.DW34.EnableDirectBiasAdjustment = 0;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        cmd.DW1.MaxNumMVs = GetMaxMvsPer2Mb(seqParams->Level) / 2;
        cmd.DW3.BMEDisableFBR = 1;
        cmd.DW5.RefWidth = SearchX[seqParams->TargetUsage];
        cmd.DW5.RefHeight = SearchY[seqParams->TargetUsage];
        cmd.DW7.NonSkipZMvAdded = 1;
        cmd.DW7.NonSkipModeAdded = 1;
        cmd.DW7.SkipCenterMask = 1;
        cmd.DW47.IntraCostSF =
             bAdaptiveIntraScalingEnable ?
            AdaptiveIntraScalingFactor_Cm_Common[sliceQP] :
            IntraScalingFactor_Cm_Common[sliceQP];
        cmd.DW47.MaxVmvR = (framePicture) ? CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) * 4 : (CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) >> 1) * 4;
        cmd.DW36.HMECombineOverlap = 1;
        cmd.DW36.NumRefIdxL0MinusOne =  bMultiPredEnable ? slcParams->num_ref_idx_l0_active_minus1 : 0;
        cmd.DW39.RefWidth = SearchX[seqParams->TargetUsage];
        cmd.DW39.RefHeight = SearchY[seqParams->TargetUsage];
        cmd.DW34.EnableDirectBiasAdjustment = 0;
        if (params->pAvcQCParams)
        {
            cmd.DW34.EnableGlobalMotionBiasAdjustment = params->pAvcQCParams->globalMotionBiasAdjustmentEnable;
        }
    }
    else
    {
        // B_TYPE
        cmd.DW1.MaxNumMVs = GetMaxMvsPer2Mb(seqParams->Level) / 2;
        cmd.DW1.BiWeight = m_biWeight;
        cmd.DW3.SearchCtrl = 7;
        cmd.DW3.SkipType = 1;
        cmd.DW5.RefWidth = BSearchX[seqParams->TargetUsage];
        cmd.DW5.RefHeight = BSearchY[seqParams->TargetUsage];
        cmd.DW7.SkipCenterMask = 0xFF;
        cmd.DW47.IntraCostSF =
             bAdaptiveIntraScalingEnable ?
            AdaptiveIntraScalingFactor_Cm_Common[sliceQP] :
            IntraScalingFactor_Cm_Common[sliceQP];
        cmd.DW47.MaxVmvR = (framePicture) ? CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) * 4 : (CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) >> 1) * 4;
        cmd.DW36.HMECombineOverlap = 1;
        // Checking if the forward frame (List 1 index 0) is a short term reference
        {
            CODEC_PICTURE codecHalPic = params->pSlcParams->RefPicList[LIST_1][0];
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
        cmd.DW36.NumRefIdxL0MinusOne =  bMultiPredEnable ? slcParams->num_ref_idx_l0_active_minus1 : 0;
        cmd.DW36.NumRefIdxL1MinusOne =  bMultiPredEnable ? slcParams->num_ref_idx_l1_active_minus1 : 0;
        cmd.DW39.RefWidth = BSearchX[seqParams->TargetUsage];
        cmd.DW39.RefHeight = BSearchY[seqParams->TargetUsage];
        cmd.DW40.DistScaleFactorRefID0List0 = m_distScaleFactorList0[0];
        cmd.DW40.DistScaleFactorRefID1List0 = m_distScaleFactorList0[1];
        cmd.DW41.DistScaleFactorRefID2List0 = m_distScaleFactorList0[2];
        cmd.DW41.DistScaleFactorRefID3List0 = m_distScaleFactorList0[3];
        cmd.DW42.DistScaleFactorRefID4List0 = m_distScaleFactorList0[4];
        cmd.DW42.DistScaleFactorRefID5List0 = m_distScaleFactorList0[5];
        cmd.DW43.DistScaleFactorRefID6List0 = m_distScaleFactorList0[6];
        cmd.DW43.DistScaleFactorRefID7List0 = m_distScaleFactorList0[7];
        if (params->pAvcQCParams)
        {
            cmd.DW34.EnableDirectBiasAdjustment = params->pAvcQCParams->directBiasAdjustmentEnable;
            if (cmd.DW34.EnableDirectBiasAdjustment)
            {
                cmd.DW7.NonSkipModeAdded = 1;
                cmd.DW7.NonSkipZMvAdded  = 1;
            }

            cmd.DW34.EnableGlobalMotionBiasAdjustment = params->pAvcQCParams->globalMotionBiasAdjustmentEnable;
        }
    }

    *params->pdwBlockBasedSkipEn = cmd.DW3.BlockBasedSkipEnable;

    if (picParams->EnableRollingIntraRefresh)
    {
        cmd.DW34.IntraRefreshEn = picParams->EnableRollingIntraRefresh;

        /* Multiple predictor should be completely disabled for the RollingI feature. This does not lead to much quality drop for P frames especially for TU as 1 */
        cmd.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;

        /* Pass the same IntraRefreshUnit to the kernel w/o the adjustment by -1, so as to have an overlap of one MB row or column of Intra macroblocks
         across one P frame to another P frame, as needed by the RollingI algo */
        if (ROLLING_I_SQUARE == picParams->EnableRollingIntraRefresh && RATECONTROL_CQP != seqParams->RateControlMethod)
        {
            /*BRC update kernel updates these CURBE to MBEnc*/
            cmd.DW4.EnableIntraRefresh = false;
            cmd.DW34.IntraRefreshEn = ROLLING_I_DISABLED;
            cmd.DW48.IntraRefreshMBx = 0; /* MB column number */
            cmd.DW61.IntraRefreshMBy = 0; /* MB row number */
        }
        else
        {
            cmd.DW4.EnableIntraRefresh = true;
            cmd.DW34.IntraRefreshEn = picParams->EnableRollingIntraRefresh;
            cmd.DW48.IntraRefreshMBx = picParams->IntraRefreshMBx; /* MB column number */
            cmd.DW61.IntraRefreshMBy = picParams->IntraRefreshMBy; /* MB row number */
        }
        cmd.DW48.IntraRefreshUnitInMBMinus1 = picParams->IntraRefreshUnitinMB;
        cmd.DW48.IntraRefreshQPDelta = picParams->IntraRefreshQPDelta;
    }
    else
    {
        cmd.DW34.IntraRefreshEn = 0;
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

        if ( bBrcEnabled == false)
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
    else if (params->bDirtyRoiEnabled)
    {
        // enable Dirty Rect flag
        cmd.DW4.EnableDirtyRect = true;

        cmd.DW49.ROI1_X_left      = params->pPicParams->DirtyROI[0].Left;
        cmd.DW49.ROI1_Y_top       = params->pPicParams->DirtyROI[0].Top;
        cmd.DW50.ROI1_X_right     = params->pPicParams->DirtyROI[0].Right;
        cmd.DW50.ROI1_Y_bottom    = params->pPicParams->DirtyROI[0].Bottom;

        cmd.DW51.ROI2_X_left      = params->pPicParams->DirtyROI[1].Left;
        cmd.DW51.ROI2_Y_top       = params->pPicParams->DirtyROI[1].Top;
        cmd.DW52.ROI2_X_right     = params->pPicParams->DirtyROI[1].Right;
        cmd.DW52.ROI2_Y_bottom    = params->pPicParams->DirtyROI[1].Bottom;

        cmd.DW53.ROI3_X_left      = params->pPicParams->DirtyROI[2].Left;
        cmd.DW53.ROI3_Y_top       = params->pPicParams->DirtyROI[2].Top;
        cmd.DW54.ROI3_X_right     = params->pPicParams->DirtyROI[2].Right;
        cmd.DW54.ROI3_Y_bottom    = params->pPicParams->DirtyROI[2].Bottom;

        cmd.DW55.ROI4_X_left      = params->pPicParams->DirtyROI[3].Left;
        cmd.DW55.ROI4_Y_top       = params->pPicParams->DirtyROI[3].Top;
        cmd.DW56.ROI4_X_right     = params->pPicParams->DirtyROI[3].Right;
        cmd.DW56.ROI4_Y_bottom    = params->pPicParams->DirtyROI[3].Bottom;
    }

    if (m_trellisQuantParams.dwTqEnabled)
    {
        // Lambda values for TQ
        if (m_pictureCodingType == I_TYPE)
        {
            cmd.DW58.Value = TQ_LAMBDA_I_FRAME[sliceQP][0];
            cmd.DW59.Value = TQ_LAMBDA_I_FRAME[sliceQP][1];
        }
        else if (m_pictureCodingType == P_TYPE)
        {
            cmd.DW58.Value = TQ_LAMBDA_P_FRAME[sliceQP][0];
            cmd.DW59.Value = TQ_LAMBDA_P_FRAME[sliceQP][1];

        }
        else
        {
            cmd.DW58.Value = TQ_LAMBDA_B_FRAME[sliceQP][0];
            cmd.DW59.Value = TQ_LAMBDA_B_FRAME[sliceQP][1];
        }

        // check if Lambda is greater than max value
        CODECHAL_ENCODE_CHK_STATUS_RETURN( GetInterRounding(&sliceState));

        if (cmd.DW58.Lambda_8x8Inter > CODECHAL_ENCODE_AVC_MAX_LAMBDA)
        {
            cmd.DW58.Lambda_8x8Inter = 0xf000 + sliceState.dwRoundingValue;
        }

        if (cmd.DW58.Lambda_8x8Intra > CODECHAL_ENCODE_AVC_MAX_LAMBDA)
        {

            cmd.DW58.Lambda_8x8Intra = 0xf000 + DEFAULT_TRELLIS_QUANT_INTRA_ROUNDING_G9_KBL;
        }

        // check if Lambda is greater than max value
        if (cmd.DW59.Lambda_Inter > CODECHAL_ENCODE_AVC_MAX_LAMBDA)
        {
            cmd.DW59.Lambda_Inter = 0xf000 + sliceState.dwRoundingValue;
        }

        if (cmd.DW59.Lambda_Intra > CODECHAL_ENCODE_AVC_MAX_LAMBDA)
        {
            cmd.DW59.Lambda_Intra = 0xf000 + DEFAULT_TRELLIS_QUANT_INTRA_ROUNDING_G9_KBL;
        }
    }

    //IPCM QP and threshold
    cmd.DW62.IPCM_QP0     = IPCM_Threshold_Table[0].QP;
    cmd.DW62.IPCM_QP1     = IPCM_Threshold_Table[1].QP;
    cmd.DW62.IPCM_QP2     = IPCM_Threshold_Table[2].QP;
    cmd.DW62.IPCM_QP3     = IPCM_Threshold_Table[3].QP;
    cmd.DW63.IPCM_QP4     = IPCM_Threshold_Table[4].QP;

    cmd.DW63.IPCM_Thresh0 = IPCM_Threshold_Table[0].Threshold;
    cmd.DW64.IPCM_Thresh1 = IPCM_Threshold_Table[1].Threshold;
    cmd.DW64.IPCM_Thresh2 = IPCM_Threshold_Table[2].Threshold;
    cmd.DW65.IPCM_Thresh3 = IPCM_Threshold_Table[3].Threshold;
    cmd.DW65.IPCM_Thresh4 = IPCM_Threshold_Table[4].Threshold;

    cmd.DW66.MBDataSurfIndex                = CODECHAL_ENCODE_AVC_MBENC_MFC_AVC_PAK_OBJ_G9;
    cmd.DW67.MVDataSurfIndex                = CODECHAL_ENCODE_AVC_MBENC_IND_MV_DATA_G9;
    cmd.DW68.IDistSurfIndex                 = CODECHAL_ENCODE_AVC_MBENC_BRC_DISTORTION_G9;
    cmd.DW69.SrcYSurfIndex                  = CODECHAL_ENCODE_AVC_MBENC_CURR_Y_G9;
    cmd.DW70.MBSpecificDataSurfIndex        = CODECHAL_ENCODE_AVC_MBENC_MB_SPECIFIC_DATA_G9;
    cmd.DW71.AuxVmeOutSurfIndex             = CODECHAL_ENCODE_AVC_MBENC_AUX_VME_OUT_G9;
    cmd.DW72.CurrRefPicSelSurfIndex         = CODECHAL_ENCODE_AVC_MBENC_REFPICSELECT_L0_G9;
    cmd.DW73.HMEMVPredFwdBwdSurfIndex       = CODECHAL_ENCODE_AVC_MBENC_MV_DATA_FROM_ME_G9;
    cmd.DW74.HMEDistSurfIndex               = CODECHAL_ENCODE_AVC_MBENC_4xME_DISTORTION_G9;
    cmd.DW75.SliceMapSurfIndex              = CODECHAL_ENCODE_AVC_MBENC_SLICEMAP_DATA_G9;
    cmd.DW76.FwdFrmMBDataSurfIndex          = CODECHAL_ENCODE_AVC_MBENC_FWD_MB_DATA_G9;
    cmd.DW77.FwdFrmMVSurfIndex              = CODECHAL_ENCODE_AVC_MBENC_FWD_MV_DATA_G9;
    cmd.DW78.MBQPBuffer                     = CODECHAL_ENCODE_AVC_MBENC_MBQP_G9;
    cmd.DW79.MBBRCLut                       = CODECHAL_ENCODE_AVC_MBENC_MBBRC_CONST_DATA_G9;
    cmd.DW80.VMEInterPredictionSurfIndex    = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_G9;
    cmd.DW81.VMEInterPredictionMRSurfIndex  = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_G9;
    cmd.DW82.MBStatsSurfIndex               = CODECHAL_ENCODE_AVC_MBENC_MB_STATS_G9;
    cmd.DW83.MADSurfIndex                   = CODECHAL_ENCODE_AVC_MBENC_MAD_DATA_G9;
    cmd.DW84.BRCCurbeSurfIndex              = CODECHAL_ENCODE_AVC_MBENC_BRC_CURBE_DATA_G9_KBL;
    cmd.DW85.ForceNonSkipMBmapSurface       = CODECHAL_ENCODE_AVC_MBENC_FORCE_NONSKIP_MB_MAP_G9_KBL;
    cmd.DW86.ReservedIndex                  = CODECHAL_ENCODE_AVC_MBENC_ADV_WA_G9_KBL;
    cmd.DW87.StaticDetectionCostTableIndex  = CODECHAL_ENCODE_AVC_MBENC_SFD_COST_TABLE_G9_KBL;
    auto stateHeapInterface =  m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateEncParam(
            meMethod,
            &cmd));
    )

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Kbl::GetTrellisQuantization(
        PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS    params,
        PCODECHAL_ENCODE_AVC_TQ_PARAMS          trellisQuantParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(trellisQuantParams);

    trellisQuantParams->dwTqEnabled    = TrellisQuantizationEnable[params->ucTargetUsage];
    trellisQuantParams->dwTqRounding   =
    trellisQuantParams->dwTqEnabled ? CODECHAL_ENCODE_AVC_TrellisQuantizationRounding_G9_KBL[params->ucTargetUsage]  : 0;

    // If AdaptiveTrellisQuantization is enabled then disable trellis quantization for
    // B-frames with QP > 26 only in CQP mode
    if(trellisQuantParams->dwTqEnabled
        && EnableAdaptiveTrellisQuantization[params->ucTargetUsage]
        && params->wPictureCodingType == B_TYPE
        && !params->bBrcEnabled && params->ucQP > 26)
    {
        trellisQuantParams->dwTqEnabled  = 0;
        trellisQuantParams->dwTqRounding = 0;
    }
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Kbl::InitBrcConstantBuffer(
        PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS        params)
{
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pOsInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);

    uint8_t tableIdx = params->wPictureCodingType - 1;
    bool blockBasedSkipEn = params->dwMbEncBlockBasedSkipEn ? true : false;
    bool transform_8x8_mode_flag = params->pPicParams->transform_8x8_mode_flag ? true : false;

    if (tableIdx >= 3)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid input parameter.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    MOS_LOCK_PARAMS     lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    auto data = (uint8_t*)params->pOsInterface->pfnLockResource(
        params->pOsInterface,
        &params->sBrcConstantDataBuffer.OsResource,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(data, params->sBrcConstantDataBuffer.dwWidth * params->sBrcConstantDataBuffer.dwHeight);

    // Fill surface with QP Adjustment table, Distortion threshold table, MaxFrame threshold table, Distortion QP Adjustment Table
    eStatus = MOS_SecureMemcpy(
        data,
        sizeof(m_qpDistMaxFrameAdjustmentCm),
        (void*)m_qpDistMaxFrameAdjustmentCm,
        sizeof(m_qpDistMaxFrameAdjustmentCm));
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    data += sizeof(m_qpDistMaxFrameAdjustmentCm);

    // Fill surface with Skip Threshold Table
    switch (params->wPictureCodingType)
    {
    case P_TYPE:
        eStatus = MOS_SecureMemcpy(
            data,
            m_brcConstantSurfaceEarlySkipTableSize,
            (void*)&SkipVal_P_Common[blockBasedSkipEn][transform_8x8_mode_flag][0],
            m_brcConstantSurfaceEarlySkipTableSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
        break;
    case B_TYPE:
        eStatus = MOS_SecureMemcpy(
            data,
            m_brcConstantSurfaceEarlySkipTableSize,
            (void*)&SkipVal_B_Common[blockBasedSkipEn][transform_8x8_mode_flag][0],
            m_brcConstantSurfaceEarlySkipTableSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
        break;
    default:
        // do nothing for I TYPE
        break;
    }

    if ((params->wPictureCodingType != I_TYPE) && (params->pAvcQCParams != nullptr) && (params->pAvcQCParams->NonFTQSkipThresholdLUTInput))
    {
        for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            *(data + 1 + (qp * 2)) = (uint8_t)CalcSkipVal((params->dwMbEncBlockBasedSkipEn ? true : false), (params->pPicParams->transform_8x8_mode_flag ? true : false), params->pAvcQCParams->NonFTQSkipThresholdLUT[qp]);
        }
    }

    data += m_brcConstantSurfaceEarlySkipTableSize;

    // Fill surface with QP list

    // Initialize to -1 (0xff)
    MOS_FillMemory(data, CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_G9, 0xff);
    MOS_FillMemory(data
        + CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_G9
        + CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_RESERVED_G9,
        CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_1_G9, 0xff);

    switch (params->wPictureCodingType)
    {
    case B_TYPE:
        data += (CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_G9 + CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_RESERVED_G9);

        for (uint8_t refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l1_active_minus1; refIdx++)
        {
            CODEC_PICTURE refPic = params->pAvcSlcParams->RefPicList[LIST_1][refIdx];
            if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
            {
                *(data + refIdx) = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            }
        }
        data -= (CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_G9 + CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_RESERVED_G9);
        // break statement omitted intentionally
    case P_TYPE:
        for (uint8_t refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l0_active_minus1; refIdx++)
        {
            CODEC_PICTURE refPic = params->pAvcSlcParams->RefPicList[LIST_0][refIdx];
            if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
            {
                *(data + refIdx) = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            }
        }
        break;
    default:
        // do nothing for I type
        break;
    }

    data += (CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_G9 + CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_0_RESERVED_G9
        + CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_1_G9 + CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_QP_LIST_1_RESERVED_G9);

    // Fill surface with Mode cost and MV cost
    eStatus = MOS_SecureMemcpy(
        data,
        m_brcConstantSurfacModeMvCostSize,
        (void*)ModeMvCost_Cm[tableIdx],
        m_brcConstantSurfacModeMvCostSize);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    // If old mode cost is used the update the table
    if (params->wPictureCodingType == I_TYPE && params->bOldModeCostEnable)
    {
        auto pdwDataTemp = (uint32_t *)data;
        for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            // Writing to DW0 in each sub-array of 16 DWs
            *pdwDataTemp = (uint32_t)OldIntraModeCost_Cm_Common[qp];
            pdwDataTemp += 16;
        }
    }

    if (params->pAvcQCParams)
    {
        for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            if (params->pAvcQCParams->FTQSkipThresholdLUTInput)
            {
                *(data + (qp * 32) + 24) =
                    *(data + (qp * 32) + 25) =
                    *(data + (qp * 32) + 27) =
                    *(data + (qp * 32) + 28) =
                    *(data + (qp * 32) + 29) =
                    *(data + (qp * 32) + 30) =
                    *(data + (qp * 32) + 31) = params->pAvcQCParams->FTQSkipThresholdLUT[qp];
            }
        }
    }

    data += m_brcConstantSurfacModeMvCostSize;

    // Fill surface with Refcost
    eStatus = MOS_SecureMemcpy(
        data,
        m_brcConstantSurfaceRefCostSize,
        (void*)&RefCost_MultiRefQp[tableIdx][0],
        m_brcConstantSurfaceRefCostSize);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }
    data += m_brcConstantSurfaceRefCostSize;

    //Fill surface with Intra cost scaling Factor
    if (params->bAdaptiveIntraScalingEnable)
    {
        eStatus = MOS_SecureMemcpy(
            data,
            CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_INTRACOST_SCALING_FACTOR_G9,
            (void*)&AdaptiveIntraScalingFactor_Cm_Common[0],
            CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_INTRACOST_SCALING_FACTOR_G9);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }
    else
    {
        eStatus = MOS_SecureMemcpy(
            data,
            CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_INTRACOST_SCALING_FACTOR_G9,
            (void*)&IntraScalingFactor_Cm_Common[0],
            CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_INTRACOST_SCALING_FACTOR_G9);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }

    data += CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_INTRACOST_SCALING_FACTOR_G9;

    eStatus = MOS_SecureMemcpy(
        data,
        BRC_CONSTANTSURFACE_LAMBDA_SIZE_G9_KBL,
        (void*)&Lambda_data[0],
        BRC_CONSTANTSURFACE_LAMBDA_SIZE_G9_KBL);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    data += BRC_CONSTANTSURFACE_LAMBDA_SIZE_G9_KBL;

    eStatus = MOS_SecureMemcpy(
        data,
        BRC_CONSTANTSURFACE_FTQ25_SIZE_G9_KBL,
        (void*)&FTQ25[0],
        BRC_CONSTANTSURFACE_FTQ25_SIZE_G9_KBL);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    params->pOsInterface->pfnUnlockResource(
        params->pOsInterface,
        &params->sBrcConstantDataBuffer.OsResource);

    return eStatus;
}

CodechalEncodeAvcEncG9Kbl::CodechalEncodeAvcEncG9Kbl(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo) : CodechalEncodeAvcEncG9(hwInterface, debugInterface, standardInfo)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    m_cmKernelEnable           = true;
    m_mbStatsSupported         = true;
    bKernelTrellis             = true;
    bExtendedMvCostRange       = false;
    bBrcSplitEnable            = true;
    bDecoupleMbEncCurbeFromBRC = true;
    bHighTextureModeCostEnable = true;

    this->pfnGetKernelHeaderAndSize         = this->GetKernelHeaderAndSize;

    m_kernelBase = (uint8_t *)IGCODECKRN_G9_KBL;
    AddIshSize(m_kuid, m_kernelBase);
}

MOS_STATUS CodechalEncodeAvcEncG9Kbl::InitializeState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcEncG9::InitializeState());

    m_brcHistoryBufferSize      = CODECHAL_ENCODE_AVC_BRC_HISTORY_BUFFER_SIZE_G9_KBL;
    m_mbencBrcBufferSize        = MBENC_BRC_BUFFER_SIZE_G9_KBL;
    dwBrcConstantSurfaceWidth  = BRC_CONSTANTSURFACE_WIDTH_G9_KBL;
    dwBrcConstantSurfaceHeight = BRC_CONSTANTSURFACE_HEIGHT_G9_KBL;
    m_forceBrcMbStatsEnabled    = true;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Kbl::SetCurbeAvcBrcInitReset(PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    auto picParams = m_avcPicParam;
    auto seqParams = m_avcSeqParam;
    auto vuiParams = m_avcVuiParams;
    uint32_t profileLevelMaxFrame;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_GetProfileLevelMaxFrameSize(
        seqParams,
        this,
        (uint32_t*)&profileLevelMaxFrame));

    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G95 cmd = g_cInit_CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G95;
    cmd.DW0.ProfileLevelMaxFrame = profileLevelMaxFrame;
    cmd.DW1.InitBufFullInBits = seqParams->InitVBVBufferFullnessInBit;
    cmd.DW2.BufSizeInBits = seqParams->VBVBufferSizeInBit;
    cmd.DW3.AverageBitRate = seqParams->TargetBitRate;
    cmd.DW4.MaxBitRate = seqParams->MaxBitRate;
    cmd.DW8.GopP =
        (seqParams->GopRefDist) ? ((seqParams->GopPicSize - 1) / seqParams->GopRefDist) : 0;
    cmd.DW9.GopB = seqParams->GopPicSize - 1 - cmd.DW8.GopP;
    cmd.DW9.FrameWidthInBytes = m_frameWidth;
    cmd.DW10.FrameHeightInBytes = m_frameHeight;
    cmd.DW12.NoSlices = m_numSlices;
    cmd.DW32.SurfaceIndexhistorybuffer = CODECHAL_ENCODE_AVC_BRC_INIT_RESET_HISTORY;
    cmd.DW33.SurfaceIndexdistortionbuffer = CODECHAL_ENCODE_AVC_BRC_INIT_RESET_DISTORTION;

    // if VUI present, VUI data has high priority
    if (seqParams->vui_parameters_present_flag && seqParams->RateControlMethod != RATECONTROL_AVBR)
    {
        cmd.DW4.MaxBitRate =
            ((vuiParams->bit_rate_value_minus1[0] + 1) << (6 + vuiParams->bit_rate_scale));

        if (seqParams->RateControlMethod == RATECONTROL_CBR)
        {
            cmd.DW3.AverageBitRate = cmd.DW4.MaxBitRate;
        }
    }

    cmd.DW6.FrameRateM = seqParams->FramesPer100Sec;
    cmd.DW7.FrameRateD = 100;
    cmd.DW8.BRCFlag = (CodecHal_PictureIsFrame(m_currOriginalPic)) ? 0 : CODECHAL_ENCODE_BRCINIT_FIELD_PIC;
    // MBBRC should be skipped when BRC ROI is on
    cmd.DW8.BRCFlag |= (bMbBrcEnabled && !bBrcRoiEnabled) ? 0 : CODECHAL_ENCODE_BRCINIT_DISABLE_MBBRC;

    if (seqParams->RateControlMethod == RATECONTROL_CBR)
    {
        cmd.DW4.MaxBitRate = cmd.DW3.AverageBitRate;
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISCBR;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_VBR)
    {
        if (cmd.DW4.MaxBitRate < cmd.DW3.AverageBitRate)
        {
            cmd.DW3.AverageBitRate = cmd.DW4.MaxBitRate; // Use max bit rate for HRD compliance
        }
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISVBR;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISAVBR;
        // For AVBR, max bitrate = target bitrate,
        cmd.DW4.MaxBitRate = cmd.DW3.AverageBitRate;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_ICQ)
    {
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISICQ;
        cmd.DW23.ACQP = seqParams->ICQQualityFactor;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_VCM)
    {
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISVCM;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_QVBR)
    {
        if (cmd.DW4.MaxBitRate < cmd.DW3.AverageBitRate)
        {
            cmd.DW3.AverageBitRate = cmd.DW4.MaxBitRate; // Use max bit rate for HRD compliance
        }
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISQVBR;
        // use ICQQualityFactor to determine the larger Qp for each MB
        cmd.DW23.ACQP = seqParams->ICQQualityFactor;
    }

    if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        usAVBRAccuracy    = (uint16_t)seqParams->AVBRAccuracy;
        usAVBRConvergence = (uint16_t)seqParams->AVBRConvergence;
    }

    cmd.DW10.AVBRAccuracy = usAVBRAccuracy;
    cmd.DW11.AVBRConvergence = usAVBRConvergence;

    // Set dynamic thresholds
    double inputBitsPerFrame =
        ((double)(cmd.DW4.MaxBitRate) * (double)(cmd.DW7.FrameRateD) /
        (double)(cmd.DW6.FrameRateM));
    if (CodecHal_PictureIsField(m_currOriginalPic))
    {
        inputBitsPerFrame *= 0.5;
    }

    if (cmd.DW2.BufSizeInBits == 0)
    {
        cmd.DW2.BufSizeInBits = (uint32_t)inputBitsPerFrame * 4;
    }

    if (cmd.DW1.InitBufFullInBits == 0)
    {
        cmd.DW1.InitBufFullInBits = 7 * cmd.DW2.BufSizeInBits / 8;
    }
    if (cmd.DW1.InitBufFullInBits < (uint32_t)(inputBitsPerFrame * 2))
    {
        cmd.DW1.InitBufFullInBits = (uint32_t)(inputBitsPerFrame * 2);
    }
    if (cmd.DW1.InitBufFullInBits > cmd.DW2.BufSizeInBits)
    {
        cmd.DW1.InitBufFullInBits = cmd.DW2.BufSizeInBits;
    }

    if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        // For AVBR, Buffer size =  2*Bitrate, InitVBV = 0.75 * BufferSize
        cmd.DW2.BufSizeInBits = 2 * seqParams->TargetBitRate;
        cmd.DW1.InitBufFullInBits = (uint32_t)(0.75 * cmd.DW2.BufSizeInBits);
    }

    double dBpsRatio = inputBitsPerFrame / ((double)(cmd.DW2.BufSizeInBits) / 30);
    dBpsRatio = (dBpsRatio < 0.1) ? 0.1 : (dBpsRatio > 3.5) ? 3.5 : dBpsRatio;

    cmd.DW16.DeviationThreshold0ForPandB = (uint32_t)(-50 * pow(0.90, dBpsRatio));
    cmd.DW16.DeviationThreshold1ForPandB = (uint32_t)(-50 * pow(0.66, dBpsRatio));
    cmd.DW16.DeviationThreshold2ForPandB = (uint32_t)(-50 * pow(0.46, dBpsRatio));
    cmd.DW16.DeviationThreshold3ForPandB = (uint32_t)(-50 * pow(0.3, dBpsRatio));
    cmd.DW17.DeviationThreshold4ForPandB = (uint32_t)(50 * pow(0.3, dBpsRatio));
    cmd.DW17.DeviationThreshold5ForPandB = (uint32_t)(50 * pow(0.46, dBpsRatio));
    cmd.DW17.DeviationThreshold6ForPandB = (uint32_t)(50 * pow(0.7, dBpsRatio));
    cmd.DW17.DeviationThreshold7ForPandB = (uint32_t)(50 * pow(0.9, dBpsRatio));
    cmd.DW18.DeviationThreshold0ForVBR = (uint32_t)(-50 * pow(0.9, dBpsRatio));
    cmd.DW18.DeviationThreshold1ForVBR = (uint32_t)(-50 * pow(0.7, dBpsRatio));
    cmd.DW18.DeviationThreshold2ForVBR = (uint32_t)(-50 * pow(0.5, dBpsRatio));
    cmd.DW18.DeviationThreshold3ForVBR = (uint32_t)(-50 * pow(0.3, dBpsRatio));
    cmd.DW19.DeviationThreshold4ForVBR = (uint32_t)(100 * pow(0.4, dBpsRatio));
    cmd.DW19.DeviationThreshold5ForVBR = (uint32_t)(100 * pow(0.5, dBpsRatio));
    cmd.DW19.DeviationThreshold6ForVBR = (uint32_t)(100 * pow(0.75, dBpsRatio));
    cmd.DW19.DeviationThreshold7ForVBR = (uint32_t)(100 * pow(0.9, dBpsRatio));
    cmd.DW20.DeviationThreshold0ForI = (uint32_t)(-50 * pow(0.8, dBpsRatio));
    cmd.DW20.DeviationThreshold1ForI = (uint32_t)(-50 * pow(0.6, dBpsRatio));
    cmd.DW20.DeviationThreshold2ForI = (uint32_t)(-50 * pow(0.34, dBpsRatio));
    cmd.DW20.DeviationThreshold3ForI = (uint32_t)(-50 * pow(0.2, dBpsRatio));
    cmd.DW21.DeviationThreshold4ForI = (uint32_t)(50 * pow(0.2, dBpsRatio));
    cmd.DW21.DeviationThreshold5ForI = (uint32_t)(50 * pow(0.4, dBpsRatio));
    cmd.DW21.DeviationThreshold6ForI = (uint32_t)(50 * pow(0.66, dBpsRatio));
    cmd.DW21.DeviationThreshold7ForI = (uint32_t)(50 * pow(0.9, dBpsRatio));

    cmd.DW22.SlidingWindowSize = dwSlidingWindowSize;

    if (bBrcInit)
    {
        *params->pdBrcInitCurrentTargetBufFullInBits = cmd.DW1.InitBufFullInBits;
    }

    *params->pdwBrcInitResetBufSizeInBits = cmd.DW2.BufSizeInBits;
    *params->pdBrcInitResetInputBitsPerFrame = inputBitsPerFrame;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcInitParam(
            &cmd));
    )

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Kbl::SetCurbeAvcFrameBrcUpdate(PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    auto seqParams = m_avcSeqParam;
    auto picParams = m_avcPicParam;
    auto slcParams = m_avcSliceParams;

    MHW_VDBOX_AVC_SLICE_STATE sliceState;
    MOS_ZeroMemory(&sliceState, sizeof(sliceState));
    sliceState.pEncodeAvcSeqParams = seqParams;
    sliceState.pEncodeAvcPicParams = picParams;
    sliceState.pEncodeAvcSliceParams = slcParams;

    CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G95   cmd = g_cInit_CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G95;

    cmd.DW5.TargetSizeFlag = 0;
    if (*params->pdBrcInitCurrentTargetBufFullInBits > (double)dwBrcInitResetBufSizeInBits)
    {
        *params->pdBrcInitCurrentTargetBufFullInBits -= (double)dwBrcInitResetBufSizeInBits;
        cmd.DW5.TargetSizeFlag = 1;
    }

    // skipped frame handling
    if (params->dwNumSkipFrames)
    {
        // pass num/size of skipped frames to update BRC
        cmd.DW6.NumSkipFrames = params->dwNumSkipFrames;
        cmd.DW7.SizeSkipFrames = params->dwSizeSkipFrames;

        // account for skipped frame in calculating CurrentTargetBufFullInBits
        *params->pdBrcInitCurrentTargetBufFullInBits += dBrcInitResetInputBitsPerFrame * params->dwNumSkipFrames;
    }

    cmd.DW0.TargetSize = (uint32_t)(*params->pdBrcInitCurrentTargetBufFullInBits);
    cmd.DW1.FrameNumber = m_storeData - 1;
    cmd.DW2.SizeofPicHeaders = m_headerBytesInserted << 3;   // kernel uses how many bits instead of bytes
    cmd.DW5.CurrFrameType =
        ((m_pictureCodingType - 2) < 0) ? 2 : (m_pictureCodingType - 2);
    cmd.DW5.BRCFlag =
        (CodecHal_PictureIsTopField(m_currOriginalPic)) ? brcUpdateIsField :
        ((CodecHal_PictureIsBottomField(m_currOriginalPic)) ? (brcUpdateIsField | brcUpdateIsBottomField) : 0);
    cmd.DW5.BRCFlag |= (m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef) ?
        brcUpdateIsReference : 0;

    if (bMultiRefQpEnabled)
    {
        cmd.DW5.BRCFlag |= brcUpdateIsActualQp;
        cmd.DW14.QPIndexOfCurPic = m_currOriginalPic.FrameIdx;
    }

    cmd.DW5.BRCFlag |= seqParams->bAutoMaxPBFrameSizeForSceneChange ?
        brcUpdateAutoPbFrameSize : 0;

    cmd.DW5.MaxNumPAKs = m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses();

    cmd.DW6.MinimumQP = params->ucMinQP;
    cmd.DW6.MaximumQP = params->ucMaxQP;
    cmd.DW6.EnableForceToSkip = (bForceToSkipEnable && !m_avcPicParam->bDisableFrameSkip);
    cmd.DW6.EnableSlidingWindow = (seqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW);
    cmd.DW6.EnableExtremLowDelay = (seqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW);
    cmd.DW6.DisableVarCompute = bBRCVarCompuBypass;

    *params->pdBrcInitCurrentTargetBufFullInBits += dBrcInitResetInputBitsPerFrame;

    if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        usAVBRConvergence = (uint16_t)seqParams->AVBRConvergence;
        usAVBRAccuracy = (uint16_t)seqParams->AVBRAccuracy;

        cmd.DW3.startGAdjFrame0 = (uint32_t)((10 * usAVBRConvergence) / (double)150);
        cmd.DW3.startGAdjFrame1 = (uint32_t)((50 * usAVBRConvergence) / (double)150);
        cmd.DW4.startGAdjFrame2 = (uint32_t)((100 * usAVBRConvergence) / (double)150);
        cmd.DW4.startGAdjFrame3 = (uint32_t)((150 * usAVBRConvergence) / (double)150);
        cmd.DW11.gRateRatioThreshold0 =
            (uint32_t)((100 - (usAVBRAccuracy / (double)30)*(100 - 40)));
        cmd.DW11.gRateRatioThreshold1 =
            (uint32_t)((100 - (usAVBRAccuracy / (double)30)*(100 - 75)));
        cmd.DW12.gRateRatioThreshold2 = (uint32_t)((100 - (usAVBRAccuracy / (double)30)*(100 - 97)));
        cmd.DW12.gRateRatioThreshold3 = (uint32_t)((100 + (usAVBRAccuracy / (double)30)*(103 - 100)));
        cmd.DW12.gRateRatioThreshold4 = (uint32_t)((100 + (usAVBRAccuracy / (double)30)*(125 - 100)));
        cmd.DW12.gRateRatioThreshold5 = (uint32_t)((100 + (usAVBRAccuracy / (double)30)*(160 - 100)));
    }

    cmd.DW15.EnableROI = params->ucEnableROI;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetInterRounding(&sliceState));

    cmd.DW15.RoundingIntra = 5;
    cmd.DW15.RoundingInter = sliceState.dwRoundingValue;

    uint32_t profileLevelMaxFrame;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_GetProfileLevelMaxFrameSize(
        seqParams,
        this,
        &profileLevelMaxFrame));

    cmd.DW19.UserMaxFrame = profileLevelMaxFrame;

    cmd.DW24.SurfaceIndexBRChistorybuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_HISTORY_G9_KBL;
    cmd.DW25.SurfaceIndexPreciousPAKstatisticsoutputbuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_PAK_STATISTICS_OUTPUT_G9_KBL;
    cmd.DW26.SurfaceIndexAVCIMGstateinputbuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_IMAGE_STATE_READ_G9_KBL;
    cmd.DW27.SurfaceIndexAVCIMGstateoutputbuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_IMAGE_STATE_WRITE_G9_KBL;
    cmd.DW28.SurfaceIndexAVC_Encbuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_MBENC_CURBE_WRITE_G9_KBL;
    cmd.DW29.SurfaceIndexAVCDISTORTIONbuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_DISTORTION_G9_KBL;
    cmd.DW30.SurfaceIndexBRCconstdatabuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_CONSTANT_DATA_G9_KBL;
    cmd.DW31.SurfaceIndexMBStatsBuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_MB_STAT_G9_KBL;
    cmd.DW32.SurfaceIndexMotionvectorbuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_MV_G9_KBL;
    auto stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcUpdateParam(
            &cmd));
    )

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Kbl::SetCurbeAvcMbBrcUpdate(PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    auto curbe = g_cInit_CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_CURBE_G95;

    // BRC curbe requires: 2 for I-frame, 0 for P-frame, 1 for B-frame
    curbe.DW0.CurrFrameType = (m_pictureCodingType + 1) % 3;
    if (params->ucEnableROI)
    {
        if (bROIValueInDeltaQP)
        {
            curbe.DW0.EnableROI = 2; // 1-Enabled ROI priority, 2-Enable ROI QP Delta,  0- disabled
            curbe.DW0.ROIRatio = 0;
        }
        else
        {
            curbe.DW0.EnableROI = 1; // 1-Enabled ROI priority, 2-Enable ROI QP Delta,  0- disabled

            uint32_t roisize = 0;
            uint32_t roiratio = 0;

            for (uint32_t i = 0; i < m_avcPicParam->NumROI; ++i)
            {
                CODECHAL_ENCODE_VERBOSEMESSAGE("ROI[%d] = {%d, %d, %d, %d} {%d}, size = %d", i,
                    m_avcPicParam->ROI[i].Left, m_avcPicParam->ROI[i].Top,
                    m_avcPicParam->ROI[i].Bottom, m_avcPicParam->ROI[i].Right,
                    m_avcPicParam->ROI[i].PriorityLevelOrDQp,
                    (CODECHAL_MACROBLOCK_HEIGHT * MOS_ABS(m_avcPicParam->ROI[i].Top - m_avcPicParam->ROI[i].Bottom)) *
                    (CODECHAL_MACROBLOCK_WIDTH * MOS_ABS(m_avcPicParam->ROI[i].Right - m_avcPicParam->ROI[i].Left)));
                roisize += (CODECHAL_MACROBLOCK_HEIGHT * MOS_ABS(m_avcPicParam->ROI[i].Top - m_avcPicParam->ROI[i].Bottom)) *
                    (CODECHAL_MACROBLOCK_WIDTH * MOS_ABS(m_avcPicParam->ROI[i].Right - m_avcPicParam->ROI[i].Left));
            }

            if (roisize)
            {
                uint32_t numMBs = m_picWidthInMb * m_picHeightInMb;
                roiratio = 2 * (numMBs * 256 / roisize - 1);
                roiratio = MOS_MIN(51, roiratio); // clip QP from 0-51
            }
            CODECHAL_ENCODE_VERBOSEMESSAGE("ROIRatio = %d", roiratio);
            curbe.DW0.ROIRatio = roiratio;
        }
    }
    else
    {
        curbe.DW0.ROIRatio = 0;
    }
    curbe.DW8.HistorybufferIndex = CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_HISTORY_G9_KBL;
    curbe.DW9.MBQPbufferIndex = CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_MB_QP_G9_KBL;
    curbe.DW10.ROIbufferIndex = CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_ROI_G9_KBL;
    curbe.DW11.MBstatisticalbufferIndex = CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_MB_STAT_G9_KBL;
    auto stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &curbe,
        params->pKernelState->dwCurbeOffset,
        sizeof(curbe)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Kbl::InitKernelStateMbEnc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    dwNumMbEncEncKrnStates =
        MBENC_TARGET_USAGE_CM * MBENC_NUM_TARGET_USAGES_CM_G9_KBL;
    dwNumMbEncEncKrnStates += MBENC_TARGET_USAGE_CM;
    pMbEncKernelStates =
        MOS_NewArray(MHW_KERNEL_STATE, dwNumMbEncEncKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pMbEncKernelStates);

    PMHW_KERNEL_STATE                           kernelStatePtr = pMbEncKernelStates;
    CODECHAL_KERNEL_HEADER                      currKrnHeader;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    for (uint32_t krnStateIdx = 0; krnStateIdx < dwNumMbEncEncKrnStates; krnStateIdx++)
    {
        bool kernelState = (krnStateIdx >= MBENC_TARGET_USAGE_CM * MBENC_NUM_TARGET_USAGES_CM_G9_KBL);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
            kernelBinary,
            (kernelState ? ENC_MBENC_ADV : ENC_MBENC),
            (kernelState ? krnStateIdx - MBENC_TARGET_USAGE_CM * MBENC_NUM_TARGET_USAGES_CM_G9_KBL : krnStateIdx),
            (void*)&currKrnHeader,
            &kernelSize));

        kernelStatePtr->KernelParams.iBTCount = CODECHAL_ENCODE_AVC_MBENC_NUM_SURFACES_G9_KBL;
        kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        kernelStatePtr->KernelParams.iCurbeLength = sizeof(CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL);
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

        kernelStatePtr++;
    }

    // Until a better way can be found, maintain old binding table structures
    auto bindingTable = &MbEncBindingTable;

    bindingTable->dwAvcMBEncMfcAvcPakObj = CODECHAL_ENCODE_AVC_MBENC_MFC_AVC_PAK_OBJ_G9;
    bindingTable->dwAvcMBEncIndMVData = CODECHAL_ENCODE_AVC_MBENC_IND_MV_DATA_G9;
    bindingTable->dwAvcMBEncBRCDist = CODECHAL_ENCODE_AVC_MBENC_BRC_DISTORTION_G9;
    bindingTable->dwAvcMBEncCurrY = CODECHAL_ENCODE_AVC_MBENC_CURR_Y_G9;
    bindingTable->dwAvcMBEncCurrUV = CODECHAL_ENCODE_AVC_MBENC_CURR_UV_G9;
    bindingTable->dwAvcMBEncMbSpecificData = CODECHAL_ENCODE_AVC_MBENC_MB_SPECIFIC_DATA_G9;

    bindingTable->dwAvcMBEncRefPicSelectL0 = CODECHAL_ENCODE_AVC_MBENC_REFPICSELECT_L0_G9;
    bindingTable->dwAvcMBEncMVDataFromME = CODECHAL_ENCODE_AVC_MBENC_MV_DATA_FROM_ME_G9;
    bindingTable->dwAvcMBEncMEDist = CODECHAL_ENCODE_AVC_MBENC_4xME_DISTORTION_G9;
    bindingTable->dwAvcMBEncSliceMapData = CODECHAL_ENCODE_AVC_MBENC_SLICEMAP_DATA_G9;
    bindingTable->dwAvcMBEncBwdRefMBData = CODECHAL_ENCODE_AVC_MBENC_FWD_MB_DATA_G9;
    bindingTable->dwAvcMBEncBwdRefMVData = CODECHAL_ENCODE_AVC_MBENC_FWD_MV_DATA_G9;
    bindingTable->dwAvcMBEncMbBrcConstData = CODECHAL_ENCODE_AVC_MBENC_MBBRC_CONST_DATA_G9;
    bindingTable->dwAvcMBEncMBStats = CODECHAL_ENCODE_AVC_MBENC_MB_STATS_G9;
    bindingTable->dwAvcMBEncMADData = CODECHAL_ENCODE_AVC_MBENC_MAD_DATA_G9;
    bindingTable->dwAvcMBEncMbNonSkipMap = CODECHAL_ENCODE_AVC_MBENC_FORCE_NONSKIP_MB_MAP_G9_KBL;
    bindingTable->dwAvcMBEncAdv = CODECHAL_ENCODE_AVC_MBENC_ADV_WA_G9_KBL;
    bindingTable->dwAvcMbEncBRCCurbeData = CODECHAL_ENCODE_AVC_MBENC_BRC_CURBE_DATA_G9_KBL;
    bindingTable->dwAvcMBEncStaticDetectionCostTable = CODECHAL_ENCODE_AVC_MBENC_SFD_COST_TABLE_G9_KBL;

    // Frame
    bindingTable->dwAvcMBEncMbQpFrame = CODECHAL_ENCODE_AVC_MBENC_MBQP_G9;
    bindingTable->dwAvcMBEncCurrPicFrame[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_G9;
    bindingTable->dwAvcMBEncFwdPicFrame[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX0_G9;
    bindingTable->dwAvcMBEncBwdPicFrame[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_G9;
    bindingTable->dwAvcMBEncFwdPicFrame[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX1_G9;
    bindingTable->dwAvcMBEncBwdPicFrame[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_G9;
    bindingTable->dwAvcMBEncFwdPicFrame[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX2_G9;
    bindingTable->dwAvcMBEncFwdPicFrame[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX3_G9;
    bindingTable->dwAvcMBEncFwdPicFrame[4] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX4_G9;
    bindingTable->dwAvcMBEncFwdPicFrame[5] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX5_G9;
    bindingTable->dwAvcMBEncFwdPicFrame[6] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX6_G9;
    bindingTable->dwAvcMBEncFwdPicFrame[7] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX7_G9;
    bindingTable->dwAvcMBEncCurrPicFrame[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_G9;
    bindingTable->dwAvcMBEncBwdPicFrame[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_G9;
    bindingTable->dwAvcMBEncBwdPicFrame[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_G9;

    // Field
    bindingTable->dwAvcMBEncMbQpField = CODECHAL_ENCODE_AVC_MBENC_MBQP_G9;
    bindingTable->dwAvcMBEncFieldCurrPic[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_0_G9;
    bindingTable->dwAvcMBEncFwdPicTopField[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX0_G9;
    bindingTable->dwAvcMBEncBwdPicTopField[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_G9;
    bindingTable->dwAvcMBEncFwdPicBotField[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX0_G9;
    bindingTable->dwAvcMBEncBwdPicBotField[0] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_0_G9;
    bindingTable->dwAvcMBEncFwdPicTopField[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX1_G9;
    bindingTable->dwAvcMBEncBwdPicTopField[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_G9;
    bindingTable->dwAvcMBEncFwdPicBotField[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX1_G9;
    bindingTable->dwAvcMBEncBwdPicBotField[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_0_G9;
    bindingTable->dwAvcMBEncFwdPicTopField[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX2_G9;
    bindingTable->dwAvcMBEncFwdPicBotField[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX2_G9;
    bindingTable->dwAvcMBEncFwdPicTopField[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX3_G9;
    bindingTable->dwAvcMBEncFwdPicBotField[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX3_G9;
    bindingTable->dwAvcMBEncFwdPicTopField[4] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX4_G9;
    bindingTable->dwAvcMBEncFwdPicBotField[4] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX4_G9;
    bindingTable->dwAvcMBEncFwdPicTopField[5] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX5_G9;
    bindingTable->dwAvcMBEncFwdPicBotField[5] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX5_G9;
    bindingTable->dwAvcMBEncFwdPicTopField[6] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX6_G9;
    bindingTable->dwAvcMBEncFwdPicBotField[6] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX6_G9;
    bindingTable->dwAvcMBEncFwdPicTopField[7] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX7_G9;
    bindingTable->dwAvcMBEncFwdPicBotField[7] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_FWD_PIC_IDX7_G9;
    bindingTable->dwAvcMBEncFieldCurrPic[1] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_CURR_PIC_IDX_1_G9;
    bindingTable->dwAvcMBEncBwdPicTopField[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_G9;
    bindingTable->dwAvcMBEncBwdPicBotField[2] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX0_1_G9;
    bindingTable->dwAvcMBEncBwdPicTopField[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_G9;
    bindingTable->dwAvcMBEncBwdPicBotField[3] = CODECHAL_ENCODE_AVC_MBENC_VME_INTER_PRED_BWD_PIC_IDX1_1_G9;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Kbl::InitKernelStateBrc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    CODECHAL_KERNEL_HEADER currKrnHeader;
    for (uint32_t krnStateIdx = 0; krnStateIdx < CODECHAL_ENCODE_BRC_IDX_NUM; krnStateIdx++)
    {
        auto kernelStatePtr = &BrcKernelStates[krnStateIdx];
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
            kernelBinary,
            ENC_BRC,
            krnStateIdx,
            (void*)&currKrnHeader,
            &kernelSize));

        kernelStatePtr->KernelParams.iBTCount = BRC_BTCOUNTS[krnStateIdx];
        kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        kernelStatePtr->KernelParams.iCurbeLength = BRC_CURBE_SIZE[krnStateIdx];
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
    auto bindingTable = &BrcUpdateBindingTable;
    bindingTable->dwFrameBrcHistoryBuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_HISTORY_G9_KBL;
    bindingTable->dwFrameBrcPakStatisticsOutputBuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_PAK_STATISTICS_OUTPUT_G9_KBL;
    bindingTable->dwFrameBrcImageStateReadBuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_IMAGE_STATE_READ_G9_KBL;
    bindingTable->dwFrameBrcImageStateWriteBuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_IMAGE_STATE_WRITE_G9_KBL;

    bindingTable->dwFrameBrcMbEncCurbeWriteData = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_MBENC_CURBE_WRITE_G9_KBL;
    bindingTable->dwFrameBrcDistortionBuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_DISTORTION_G9_KBL;
    bindingTable->dwFrameBrcConstantData = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_CONSTANT_DATA_G9_KBL;
    bindingTable->dwFrameBrcMbStatBuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_MB_STAT_G9_KBL;
    bindingTable->dwFrameBrcMvDataBuffer = CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_MV_G9_KBL;

    // starting GEN9 BRC kernel has split into a frame level update, and an MB level update.
    // above is BTI for frame level, below is BTI for MB level
    bindingTable->dwMbBrcHistoryBuffer = CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_HISTORY_G9_KBL;
    bindingTable->dwMbBrcMbQpBuffer = CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_MB_QP_G9_KBL;
    bindingTable->dwMbBrcROISurface = CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_ROI_G9_KBL;
    bindingTable->dwMbBrcMbStatBuffer = CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_MB_STAT_G9_KBL;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Kbl::SetCurbeAvcWP(PCODECHAL_ENCODE_AVC_WP_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    int16_t    weight;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    auto slcParams = m_avcSliceParams;
    auto seqParams = m_avcSeqParam;
    auto kernelState = pWPKernelState;
    CODECHAL_ENCODE_ASSERT(seqParams->TargetUsage < NUM_TARGET_USAGE_MODES);

    CODECHAL_ENCODE_AVC_WP_CURBE_G9_KBL cmd;
    MOS_ZeroMemory(&cmd, sizeof(CODECHAL_ENCODE_AVC_WP_CURBE_G9_KBL));

    /* Weights[i][j][k][m] is interpreted as:

    i refers to reference picture list 0 or 1;
    j refers to reference list entry 0-31;
    k refers to data for the luma (Y) component when it is 0, the Cb chroma component when it is 1 and the Cr chroma component when it is 2;
    m refers to weight when it is 0 and offset when it is 1
    */
    weight =  slcParams->Weights[params->RefPicListIdx][params->WPIdx][0][0];
    cmd.DW0.DefaultWeight = (weight << 6) >> (slcParams->luma_log2_weight_denom);
    cmd.DW0.DefaultOffset = slcParams->Weights[params->RefPicListIdx][params->WPIdx][0][1];

    cmd.DW49.InputSurface = CODECHAL_ENCODE_AVC_WP_INPUT_REF_SURFACE_G9;
    cmd.DW50.OutputSurface = CODECHAL_ENCODE_AVC_WP_OUTPUT_SCALED_SURFACE_G9;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &cmd,
        kernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9Kbl::SceneChangeReport(PMOS_COMMAND_BUFFER    cmdBuffer, PCODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS params)
{

    MHW_MI_COPY_MEM_MEM_PARAMS                      copyMemMemParams;
    uint32_t offset = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize)
        + (sizeof(uint32_t) * 2) + m_encodeStatusBuf.dwSceneChangedOffset;

    MOS_ZeroMemory(&copyMemMemParams, sizeof(copyMemMemParams));
    copyMemMemParams.presSrc = params->presBrcHistoryBuffer;
    copyMemMemParams.dwSrcOffset = CODECHAL_ENCODE_AVC_BRC_HISTORY_BUFFER_OFFSET_SCENE_CHANGED;
    copyMemMemParams.presDst = &m_encodeStatusBuf.resStatusBuffer;
    copyMemMemParams.dwDstOffset = offset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(
        cmdBuffer,
        &copyMemMemParams));

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalEncodeAvcEncG9Kbl::PopulateBrcInitParam(
    void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G95 *curbe = (CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G95 *)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->MBBRCEnable          = bMbBrcEnabled;
        m_avcPar->MBRC                 = bMbBrcEnabled;
        m_avcPar->BitRate              = curbe->DW3.AverageBitRate;
        m_avcPar->InitVbvFullnessInBit = curbe->DW1.InitBufFullInBits;
        m_avcPar->MaxBitRate           = curbe->DW4.MaxBitRate;
        m_avcPar->VbvSzInBit           = curbe->DW2.BufSizeInBits;
        m_avcPar->AvbrAccuracy         = curbe->DW10.AVBRAccuracy;
        m_avcPar->AvbrConvergence      = curbe->DW11.AVBRConvergence;
        m_avcPar->SlidingWindowSize    = curbe->DW22.SlidingWindowSize;
        m_avcPar->LongTermInterval     = curbe->DW24.LongTermInterval;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG9Kbl::PopulateBrcUpdateParam(
    void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G95 *curbe = (CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G95 *)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->EnableMultipass     = (curbe->DW5.MaxNumPAKs > 0) ? 1 : 0;
        m_avcPar->MaxNumPakPasses     = curbe->DW5.MaxNumPAKs;
        m_avcPar->SlidingWindowEnable = curbe->DW6.EnableSlidingWindow;
        m_avcPar->FrameSkipEnable     = curbe->DW6.EnableForceToSkip;
        m_avcPar->UserMaxFrame        = curbe->DW19.UserMaxFrame;
    }
    else
    {
        m_avcPar->UserMaxFrameP = curbe->DW19.UserMaxFrame;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG9Kbl::PopulateEncParam(
    uint8_t meMethod,
    void    *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL *curbe = (CODECHAL_ENCODE_AVC_MBENC_CURBE_G9_KBL *)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->MRDisableQPCheck = MRDisableQPCheck[m_targetUsage];
        m_avcPar->AllFractional =
            CODECHAL_ENCODE_AVC_AllFractional_Common[m_targetUsage & 0x7];
        m_avcPar->DisableAllFractionalCheckForHighRes =
            CODECHAL_ENCODE_AVC_DisableAllFractionalCheckForHighRes_Common[m_targetUsage & 0x7];
        m_avcPar->EnableAdaptiveSearch              = curbe->DW37.AdaptiveEn;
        m_avcPar->EnableFBRBypass                   = curbe->DW4.EnableFBRBypass;
        m_avcPar->BlockBasedSkip                    = curbe->DW3.BlockBasedSkipEnable;
        m_avcPar->MADEnableFlag                     = curbe->DW34.MADEnableFlag;
        m_avcPar->MBTextureThreshold                = curbe->DW60.MBTextureThreshold;
        m_avcPar->EnableMBFlatnessCheckOptimization = curbe->DW34.EnableMBFlatnessChkOptimization;
        m_avcPar->EnableArbitrarySliceSize          = curbe->DW34.ArbitraryNumMbsPerSlice;
        m_avcPar->RefThresh                         = curbe->DW38.RefThreshold;
        m_avcPar->EnableWavefrontOptimization       = curbe->DW4.EnableWavefrontOptimization;
        m_avcPar->MaxLenSP                          = curbe->DW2.LenSP;
        m_avcPar->DisableExtendedMvCostRange        = !curbe->DW1.ExtendedMvCostRange;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        m_avcPar->MEMethod                             = meMethod;
        m_avcPar->HMECombineLen                        = HMECombineLen[m_targetUsage];
        m_avcPar->FTQBasedSkip                         = FTQBasedSkip[m_targetUsage];
        m_avcPar->MultiplePred                         = MultiPred[m_targetUsage];
        m_avcPar->EnableAdaptiveIntraScaling           = bAdaptiveIntraScalingEnable;
        m_avcPar->StaticFrameIntraCostScalingRatioP    = 240;
        m_avcPar->SubPelMode                           = curbe->DW3.SubPelMode;
        m_avcPar->HMECombineOverlap                    = curbe->DW36.HMECombineOverlap;
        m_avcPar->SearchX                              = curbe->DW5.RefWidth;
        m_avcPar->SearchY                              = curbe->DW5.RefHeight;
        m_avcPar->SearchControl                        = curbe->DW3.SearchCtrl;
        m_avcPar->EnableAdaptiveTxDecision             = curbe->DW34.EnableAdaptiveTxDecision;
        m_avcPar->TxDecisionThr                        = curbe->DW60.TxDecisonThreshold;
        m_avcPar->EnablePerMBStaticCheck               = curbe->DW34.EnablePerMBStaticCheck;
        m_avcPar->EnableAdaptiveSearchWindowSize       = curbe->DW34.EnableAdaptiveSearchWindowSize;
        m_avcPar->EnableIntraCostScalingForStaticFrame = curbe->DW4.EnableIntraCostScalingForStaticFrame;
        m_avcPar->BiMixDisable                         = curbe->DW0.BiMixDis;
        m_avcPar->SurvivedSkipCost                     = (curbe->DW7.NonSkipZMvAdded << 1) + curbe->DW7.NonSkipModeAdded;
        m_avcPar->UniMixDisable                        = curbe->DW1.UniMixDisable;
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        m_avcPar->BMEMethod                         = meMethod;
        m_avcPar->HMEBCombineLen                    = HMEBCombineLen[m_targetUsage];
        m_avcPar->StaticFrameIntraCostScalingRatioB = 200;
        m_avcPar->BSearchX                          = curbe->DW5.RefWidth;
        m_avcPar->BSearchY                          = curbe->DW5.RefHeight;
        m_avcPar->BSearchControl                    = curbe->DW3.SearchCtrl;
        m_avcPar->BSkipType                         = curbe->DW3.SkipType;
        m_avcPar->DirectMode                        = curbe->DW34.bDirectMode;
        m_avcPar->BiWeight                          = curbe->DW1.BiWeight;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG9Kbl::PopulatePakParam(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_BATCH_BUFFER   secondLevelBatchBuffer)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    uint8_t         *data = nullptr;
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;

    if (cmdBuffer != nullptr)
    {
        data = (uint8_t*)(cmdBuffer->pCmdPtr - (mhw_vdbox_mfx_g9_kbl::MFX_AVC_IMG_STATE_CMD::byteSize / sizeof(uint32_t)));
    }
    else if (secondLevelBatchBuffer != nullptr)
    {
        data = secondLevelBatchBuffer->pData;
    }
    else
    {
        data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &BrcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx], &lockFlags);
    }

    CODECHAL_DEBUG_CHK_NULL(data);

    mhw_vdbox_mfx_g9_kbl::MFX_AVC_IMG_STATE_CMD mfxCmd;
    mfxCmd = *(mhw_vdbox_mfx_g9_kbl::MFX_AVC_IMG_STATE_CMD *)(data);

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->TrellisQuantizationEnable         = mfxCmd.DW5.TrellisQuantizationEnabledTqenb;
        m_avcPar->EnableAdaptiveTrellisQuantization = mfxCmd.DW5.TrellisQuantizationEnabledTqenb;
        m_avcPar->TrellisQuantizationRounding       = mfxCmd.DW5.TrellisQuantizationRoundingTqr;
        m_avcPar->TrellisQuantizationChromaDisable  = mfxCmd.DW5.TrellisQuantizationChromaDisableTqchromadisable;
        m_avcPar->ExtendedRhoDomainEn               = mfxCmd.DW16_17.ExtendedRhodomainStatisticsEnable;
    }

    if (data && (cmdBuffer == nullptr) && (secondLevelBatchBuffer == nullptr))
    {
        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &BrcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx]);
    }

    return MOS_STATUS_SUCCESS;
}
#endif
