/*
* Copyright (c) 2017, Intel Corporation
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
//! \file      cm_rt_g9.h
//! \brief     Contains Definitions for CM on Gen 9
//!

#ifndef __CM_RT_G9_H__
#define __CM_RT_G9_H__

#define SKL_L3_PLANE_DEFAULT    CM_L3_PLANE_DEFAULT
#define SKL_L3_PLANE_1          CM_L3_PLANE_1
#define SKL_L3_PLANE_2          CM_L3_PLANE_2
#define SKL_L3_PLANE_3          CM_L3_PLANE_3
#define SKL_L3_PLANE_4          CM_L3_PLANE_4
#define SKL_L3_PLANE_5          CM_L3_PLANE_5
#define SKL_L3_PLANE_6          CM_L3_PLANE_6
#define SKL_L3_PLANE_7          CM_L3_PLANE_7
#define SKL_L3_CONFIG_COUNT     8

#define SKL_SLM_PLANE_DEFAULT = SKL_L3_PLANE_5

static const L3ConfigRegisterValues SKL_L3_PLANE[SKL_L3_CONFIG_COUNT] =
{                                                                                 // SLM    URB   Rest   DC    RO    I/S    C     T     Sum
    {0x00000000, 0x00000000, 0x00000000, 0x60000060},                             //{0,     48,    48,    0,    0,    0,    0,    0,    96},
    {0x00000000, 0x00000000, 0x00000000,0x00808060},                              //{0,     48,    0,    16,   32,    0,    0,    0,    96},
    {0x00000000, 0x00000000, 0x00000000,0x00818040},                              //{0,     32,    0,    16,   48,    0,    0,    0,    96},
    {0x00000000, 0x00000000, 0x00000000,0x00030040},                              //{0,     32,    0,     0,   64,    0,    0,    0,    96},
    {0x00000000, 0x00000000, 0x00000000,0x80000040},                              //{0,     32,    64,    0,    0,    0,    0,    0,    96},
    {0x00000000, 0x00000000, 0x00000000,0x60000121},                              //{32,    16,    48,    0,    0,    0,    0,    0,    96},
    {0x00000000, 0x00000000, 0x00000000,0x00410121},                              //{32,    16,    0,    16,   32,    0,    0,    0,    96},
    {0x00000000, 0x00000000, 0x00000000,0x00808121}                               //{32,    16,    0,    32,   16,    0,    0,    0,    96}
};

typedef struct _VEBOX_SURFACE_CONTROL_BITS_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       Reserved0 : BITFIELD_BIT(0);
            DWORD       IndexToMemoryObjectControlStateMocsTables : BITFIELD_RANGE(1, 6);
            DWORD       MemoryCompressionEnable : BITFIELD_BIT(7);
            DWORD       MemoryCompressionMode : BITFIELD_BIT(8);
            DWORD       TiledResourceMode : BITFIELD_RANGE(9, 10);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(11, 31);
        };
        DWORD Value;
    } DW0;
} VEBOX_SURFACE_CONTROL_BITS_G9, *PVEBOX_SURFACE_CONTROL_BITS_G9;


// Defined in vol2c "Vebox"
typedef struct _VEBOX_DNDI_STATE_G9
{
    // Temporal DN
    // DWORD 0
    union {
        struct {
            DWORD       DenoiseMovingPixelThreshold : BITFIELD_RANGE(0, 4);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(5, 7);
            DWORD       DenoiseHistoryIncrease : BITFIELD_RANGE(8, 11);
            DWORD       DenoiseMaximumHistory : BITFIELD_RANGE(12, 19);
            DWORD       DenoiseStadThreshold : BITFIELD_RANGE(20, 31);
        };
        DWORD       Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       LowTemporalDifferenceThreshold : BITFIELD_RANGE(0, 9);
            DWORD       TemporalDifferenceThreshold : BITFIELD_RANGE(10, 19);
            DWORD       DenoiseAsdThreshold : BITFIELD_RANGE(20, 31);
        };
        DWORD       Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(0, 9);
            DWORD       InitialDenoiseHistory : BITFIELD_RANGE(10, 15);
            DWORD       DenoiseThresholdForSumOfComplexityMeasure : BITFIELD_RANGE(16, 27);
            DWORD       ProgressiveDn : BITFIELD_BIT(28);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(29, 31);
        };
        DWORD       Value;
    } DW2;

    // Global Noise estimate and hot pixel detection
    // DWORD 3
    union {
        struct {
            DWORD       BlockNoiseEstimateNoiseThreshold : BITFIELD_RANGE(0, 11);
            DWORD       BlockNoiseEstimateEdgeThreshold : BITFIELD_RANGE(12, 19);
            DWORD       HotPixelThreshold : BITFIELD_RANGE(20, 27);
            DWORD       HotPixelCount : BITFIELD_RANGE(28, 31);
        };
        DWORD       Value;
    } DW3;

    // Chroma DN
    // DWORD 4
    union {
        struct {
            DWORD       ChromaLowTemporalDifferenceThreshold : BITFIELD_RANGE(0, 5);
            DWORD       ChromaTemporalDifferenceThreshold : BITFIELD_RANGE(6, 11);
            DWORD       ChromaDenoiseEnable : BITFIELD_BIT(12);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(13, 15);
            DWORD       ChromaDenoiseStadThreshold : BITFIELD_RANGE(16, 23);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(24, 31);
        };
        DWORD       Value;
    } DW4;

    // 5x5 Spatial Denoise
    // DWORD 5
    union {
        struct {
            DWORD       DnWr0 : BITFIELD_RANGE(0, 4);
            DWORD       DnWr1 : BITFIELD_RANGE(5, 9);
            DWORD       DnWr2 : BITFIELD_RANGE(10, 14);
            DWORD       DnWr3 : BITFIELD_RANGE(15, 19);
            DWORD       DnWr4 : BITFIELD_RANGE(20, 24);
            DWORD       DnWr5 : BITFIELD_RANGE(25, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD       Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            DWORD       DnThmin : BITFIELD_RANGE(0, 12);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(13, 15);
            DWORD       DnThmax : BITFIELD_RANGE(16, 28);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(29, 31);
        };
        DWORD       Value;
    } DW6;

    // DWORD 7
    union {
        struct {
            DWORD       DnDynThmin : BITFIELD_RANGE(0, 12);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(13, 15);
            DWORD       DnPrt5 : BITFIELD_RANGE(16, 28);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(29, 31);
        };
        DWORD       Value;
    } DW7;

    // DWORD 8
    union {
        struct {
            DWORD       DnPrt3 : BITFIELD_RANGE(0, 12);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(13, 15);
            DWORD       DnPrt4 : BITFIELD_RANGE(16, 28);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(29, 31);
        };
        DWORD       Value;
    } DW8;

    // DWORD 9
    union {
        struct {
            DWORD       DnPrt1 : BITFIELD_RANGE(0, 12);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(13, 15);
            DWORD       DnPrt2 : BITFIELD_RANGE(16, 28);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(29, 31);
        };
        DWORD       Value;
    } DW9;

    // DWORD 10
    union {
        struct {
            DWORD       DnWd20 : BITFIELD_RANGE(0, 4);
            DWORD       DnWd21 : BITFIELD_RANGE(5, 9);
            DWORD       DnWd22 : BITFIELD_RANGE(10, 14);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(15);
            DWORD       DnPrt0 : BITFIELD_RANGE(16, 28);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(29, 31);
        };
        DWORD       Value;
    } DW10;

    // DWORD 11
    union {
        struct {
            DWORD       DnWd00 : BITFIELD_RANGE(0, 4);
            DWORD       DnWd01 : BITFIELD_RANGE(5, 9);
            DWORD       DnWd02 : BITFIELD_RANGE(10, 14);
            DWORD       DnWd10 : BITFIELD_RANGE(15, 19);
            DWORD       DnWd11 : BITFIELD_RANGE(20, 24);
            DWORD       DnWd12 : BITFIELD_RANGE(25, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD       Value;
    } DW11;

    // FMD and DI
    // DWORD 12
    union {
        struct {
            DWORD       SmoothMvThreshold : BITFIELD_RANGE(0, 1);
            DWORD       SadTightThreshold : BITFIELD_RANGE(2, 5);
            DWORD       ContentAdaptiveThresholdSlope : BITFIELD_RANGE(6, 9);
            DWORD       StmmC2 : BITFIELD_RANGE(10, 12);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(13, 31);
        };
        DWORD       Value;
    } DW12;

    // DWORD 13
    union {
        struct {
            DWORD       MaximumStmm : BITFIELD_RANGE(0, 7);
            DWORD       MultiplierForVecm : BITFIELD_RANGE(8, 13);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(14, 15);
            DWORD       BlendingConstantAcrossTimeForSmallValuesOfStmm : BITFIELD_RANGE(16, 23);
            DWORD       BlendingConstantAcrossTimeForLargeValuesOfStmm : BITFIELD_RANGE(24, 30);
            DWORD       StmmBlendingConstantSelect : BITFIELD_BIT(31);
        };
        DWORD       Value;
    } DW13;

    // DWORD 14
    union {
        struct {
            DWORD       SdiDelta : BITFIELD_RANGE(0, 7);
            DWORD       SdiThreshold : BITFIELD_RANGE(8, 15);
            DWORD       StmmOutputShift : BITFIELD_RANGE(16, 19);
            DWORD       StmmShiftUp : BITFIELD_RANGE(20, 21);
            DWORD       StmmShiftDown : BITFIELD_RANGE(22, 23);
            DWORD       MinimumStmm : BITFIELD_RANGE(24, 31);
        };
        DWORD       Value;
    } DW14;

    // DWORD 15
    union {
        struct {
            DWORD       FmdTemporalDifferenceThreshold : BITFIELD_RANGE(0, 7);
            DWORD       SdiFallbackMode2ConstantAngle2X1 : BITFIELD_RANGE(8, 15);
            DWORD       SdiFallbackMode1T2Constant : BITFIELD_RANGE(16, 23);
            DWORD       SdiFallbackMode1T1Constant : BITFIELD_RANGE(24, 31);
        };
        DWORD       Value;
    } DW15;

    // DWORD 16
    union {
        struct {
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(0, 2);
            DWORD       DnDiTopFirst : BITFIELD_BIT(3);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(4, 6);
            DWORD       McdiEnable : BITFIELD_BIT(7);
            DWORD       FmdTearThreshold : BITFIELD_RANGE(8, 13);
            DWORD       CatThreshold : BITFIELD_RANGE(14, 15);
            DWORD       Fmd2VerticalDifferenceThreshold : BITFIELD_RANGE(16, 23);
            DWORD       Fmd1VerticalDifferenceThreshold : BITFIELD_RANGE(24, 31);
        };
        DWORD       Value;
    } DW16;

    // DWORD 17
    union {
        struct {
            DWORD       SadTha : BITFIELD_RANGE(0, 3);
            DWORD       SadThb : BITFIELD_RANGE(4, 7);
            DWORD       FmdFor1StFieldOfCurrentFrame : BITFIELD_RANGE(8, 9);
            DWORD       McPixelConsistencyThreshold : BITFIELD_RANGE(10, 15);
            DWORD       FmdFor2NdFieldOfPreviousFrame : BITFIELD_RANGE(16, 17);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(18);
            DWORD       NeighborPixelThreshold : BITFIELD_RANGE(19, 22);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(23, 31);
        };
        DWORD       Value;
    } DW17;
} VEBOX_DNDI_STATE_G9, *PVEBOX_DNDI_STATE_G9;

// Defined in vol2c "Vebox"
typedef struct _VEBOX_STD_STE_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       StdEnable : BITFIELD_BIT(0);
            DWORD       SteEnable : BITFIELD_BIT(1);
            DWORD       OutputControl : BITFIELD_BIT(2);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(3);
            DWORD       SatMax : BITFIELD_RANGE(4, 9);
            DWORD       HueMax : BITFIELD_RANGE(10, 15);
            DWORD       UMid : BITFIELD_RANGE(16, 23);
            DWORD       VMid : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       Sin : BITFIELD_RANGE(0, 7);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(8, 9);
            DWORD       Cos : BITFIELD_RANGE(10, 17);
            DWORD       HsMargin : BITFIELD_RANGE(18, 20);
            DWORD       DiamondDu : BITFIELD_RANGE(21, 27);
            DWORD       DiamondMargin : BITFIELD_RANGE(28, 30);
            DWORD       StdScoreOutput : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       DiamondDv : BITFIELD_RANGE(0, 6);
            DWORD       DiamondTh : BITFIELD_RANGE(7, 12);
            DWORD       DiamondAlpha : BITFIELD_RANGE(13, 20);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(21, 31);
        };
        DWORD           Value;
    } DW2;

    // DWORD 3
    union {
        struct {
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(0, 6);
            DWORD       VyStdEnable : BITFIELD_BIT(7);
            DWORD       YPoint1 : BITFIELD_RANGE(8, 15);
            DWORD       YPoint2 : BITFIELD_RANGE(16, 23);
            DWORD       YPoint3 : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW3;

    // DWORD 4
    union {
        struct {
            DWORD       YPoint4 : BITFIELD_RANGE(0, 7);
            DWORD       YSlope1 : BITFIELD_RANGE(8, 12);
            DWORD       YSlope2 : BITFIELD_RANGE(13, 17);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(18, 31);
        };
        DWORD           Value;
    } DW4;

    // DWORD 5
    union {
        struct {
            DWORD       InvMarginVyl : BITFIELD_RANGE(0, 15);
            DWORD       InvSkinTypesMargin : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            DWORD       InvMarginVyu : BITFIELD_RANGE(0, 15);
            DWORD       P0L : BITFIELD_RANGE(16, 23);
            DWORD       P1L : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW6;

    // DWORD 7
    union {
        struct {
            DWORD       P2L : BITFIELD_RANGE(0, 7);
            DWORD       P3L : BITFIELD_RANGE(8, 15);
            DWORD       B0L : BITFIELD_RANGE(16, 23);
            DWORD       B1L : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW7;

    // DWORD 8
    union {
        struct {
            DWORD       B2L : BITFIELD_RANGE(0, 7);
            DWORD       B3L : BITFIELD_RANGE(8, 15);
            DWORD       S0L : BITFIELD_RANGE(16, 26);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(27, 31);
        };
        DWORD           Value;
    } DW8;

    // DWORD 9
    union {
        struct {
            DWORD       S1L : BITFIELD_RANGE(0, 10);
            DWORD       S2L : BITFIELD_RANGE(11, 21);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW9;

    // DWORD 10
    union {
        struct {
            DWORD       S3L : BITFIELD_RANGE(0, 10);
            DWORD       P0U : BITFIELD_RANGE(11, 18);
            DWORD       P1U : BITFIELD_RANGE(19, 26);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(27, 31);
        };
        DWORD           Value;
    } DW10;

    // DWORD 11
    union {
        struct {
            DWORD       P2U : BITFIELD_RANGE(0, 7);
            DWORD       P3U : BITFIELD_RANGE(8, 15);
            DWORD       B0U : BITFIELD_RANGE(16, 23);
            DWORD       B1U : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW11;

    // DWORD 12
    union {
        struct {
            DWORD       B2U : BITFIELD_RANGE(0, 7);
            DWORD       B3U : BITFIELD_RANGE(8, 15);
            DWORD       S0U : BITFIELD_RANGE(16, 26);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(27, 31);
        };
        DWORD           Value;
    } DW12;

    // DWORD 13
    union {
        struct {
            DWORD       S1U : BITFIELD_RANGE(0, 10);
            DWORD       S2U : BITFIELD_RANGE(11, 21);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW13;

    // DWORD 14
    union {
        struct {
            DWORD       S3U : BITFIELD_RANGE(0, 10);
            DWORD       SkinTypesEnable : BITFIELD_BIT(11);
            DWORD       SkinTypesThresh : BITFIELD_RANGE(12, 19);
            DWORD       SkinTypesMargin : BITFIELD_RANGE(20, 27);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(28, 31);
        };
        DWORD           Value;
    } DW14;

    // DWORD 15
    union {
        struct {
            DWORD       Satp1 : BITFIELD_RANGE(0, 6);
            DWORD       Satp2 : BITFIELD_RANGE(7, 13);
            DWORD       Satp3 : BITFIELD_RANGE(14, 20);
            DWORD       Satb1 : BITFIELD_RANGE(21, 30);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW15;

    // DWORD 16
    union {
        struct {
            DWORD       Satb2 : BITFIELD_RANGE(0, 9);
            DWORD       Satb3 : BITFIELD_RANGE(10, 19);
            DWORD       Sats0 : BITFIELD_RANGE(20, 30);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW16;

    // DWORD 17
    union {
        struct {
            DWORD       Sats1 : BITFIELD_RANGE(0, 10);
            DWORD       Sats2 : BITFIELD_RANGE(11, 21);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW17;

    // DWORD 18
    union {
        struct {
            DWORD       Sats3 : BITFIELD_RANGE(0, 10);
            DWORD       Huep1 : BITFIELD_RANGE(11, 17);
            DWORD       Huep2 : BITFIELD_RANGE(18, 24);
            DWORD       Huep3 : BITFIELD_RANGE(25, 31);
        };
        DWORD           Value;
    } DW18;

    // DWORD 19
    union {
        struct {
            DWORD       Hueb1 : BITFIELD_RANGE(0, 9);
            DWORD       Hueb2 : BITFIELD_RANGE(10, 19);
            DWORD       Hueb3 : BITFIELD_RANGE(20, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD           Value;
    } DW19;

    // DWORD 20
    union {
        struct {
            DWORD       Hues0 : BITFIELD_RANGE(0, 10);
            DWORD       Hues1 : BITFIELD_RANGE(11, 21);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW20;

    // DWORD 21
    union {
        struct {
            DWORD       Hues2 : BITFIELD_RANGE(0, 10);
            DWORD       Hues3 : BITFIELD_RANGE(11, 21);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW21;

    // DWORD 22
    union {
        struct {
            DWORD       Satp1Dark : BITFIELD_RANGE(0, 6);
            DWORD       Satp2Dark : BITFIELD_RANGE(7, 13);
            DWORD       Satp3Dark : BITFIELD_RANGE(14, 20);
            DWORD       Satb1Dark : BITFIELD_RANGE(21, 30);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW22;

    // DWORD 23
    union {
        struct {
            DWORD       Satb2Dark : BITFIELD_RANGE(0, 9);
            DWORD       Satb3Dark : BITFIELD_RANGE(10, 19);
            DWORD       Sats0Dark : BITFIELD_RANGE(20, 30);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW23;

    // DWORD 24
    union {
        struct {
            DWORD       Sats1Dark : BITFIELD_RANGE(0, 10);
            DWORD       Sats2Dark : BITFIELD_RANGE(11, 21);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW24;

    // DWORD 25
    union {
        struct {
            DWORD       Sats3Dark : BITFIELD_RANGE(0, 10);
            DWORD       Huep1Dark : BITFIELD_RANGE(11, 17);
            DWORD       Huep2Dark : BITFIELD_RANGE(18, 24);
            DWORD       Huep3Dark : BITFIELD_RANGE(25, 31);
        };
        DWORD           Value;
    } DW25;

    // DWORD 26
    union {
        struct {
            DWORD       Hueb1Dark : BITFIELD_RANGE(0, 9);
            DWORD       Hueb2Dark : BITFIELD_RANGE(10, 19);
            DWORD       Hueb3Dark : BITFIELD_RANGE(20, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD           Value;
    } DW26;

    // DWORD 27
    union {
        struct {
            DWORD       Hues0Dark : BITFIELD_RANGE(0, 10);
            DWORD       Hues1Dark : BITFIELD_RANGE(11, 21);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW27;

    // DWORD 28
    union {
        struct {
            DWORD       Hues2Dark : BITFIELD_RANGE(0, 10);
            DWORD       Hues3Dark : BITFIELD_RANGE(11, 21);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW28;
} VEBOX_STD_STE_STATE_G9, *PVEBOX_STD_STE_STATE_G9;


// Defined in vol2c "Vebox"
typedef struct _VEBOX_ACE_LACE_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       AceEnable : BITFIELD_BIT(0);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(1);
            DWORD       SkinThreshold : BITFIELD_RANGE(2, 6);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(7, 11);
            DWORD       LaceHistogramEnable : BITFIELD_BIT(12);
            DWORD       LaceHistogramSize : BITFIELD_BIT(13);
            DWORD       LaceSingleHistogramSet : BITFIELD_RANGE(14, 15);
            DWORD       MinAceLuma : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       Ymin : BITFIELD_RANGE(0, 7);
            DWORD       Y1 : BITFIELD_RANGE(8, 15);
            DWORD       Y2 : BITFIELD_RANGE(16, 23);
            DWORD       Y3 : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       Y4 : BITFIELD_RANGE(0, 7);
            DWORD       Y5 : BITFIELD_RANGE(8, 15);
            DWORD       Y6 : BITFIELD_RANGE(16, 23);
            DWORD       Y7 : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW2;

    // DWORD 3
    union {
        struct {
            DWORD       Y8 : BITFIELD_RANGE(0, 7);
            DWORD       Y9 : BITFIELD_RANGE(8, 15);
            DWORD       Y10 : BITFIELD_RANGE(16, 23);
            DWORD       Ymax : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW3;

    // DWORD 4
    union {
        struct {
            DWORD       B1 : BITFIELD_RANGE(0, 7);
            DWORD       B2 : BITFIELD_RANGE(8, 15);
            DWORD       B3 : BITFIELD_RANGE(16, 23);
            DWORD       B4 : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW4;

    // DWORD 5
    union {
        struct {
            DWORD       B5 : BITFIELD_RANGE(0, 7);
            DWORD       B6 : BITFIELD_RANGE(8, 15);
            DWORD       B7 : BITFIELD_RANGE(16, 23);
            DWORD       B8 : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            DWORD       B9 : BITFIELD_RANGE(0, 7);
            DWORD       B10 : BITFIELD_RANGE(8, 15);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW6;

    // DWORD 7
    union {
        struct {
            DWORD       S0 : BITFIELD_RANGE(0, 10);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(11, 15);
            DWORD       S1 : BITFIELD_RANGE(16, 26);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(27, 31);
        };
        DWORD           Value;
    } DW7;

    // DWORD 8
    union {
        struct {
            DWORD       S2 : BITFIELD_RANGE(0, 10);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(11, 15);
            DWORD       S3 : BITFIELD_RANGE(16, 26);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(27, 31);
        };
        DWORD           Value;
    } DW8;

    // DWORD 9
    union {
        struct {
            DWORD       S4 : BITFIELD_RANGE(0, 10);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(11, 15);
            DWORD       S5 : BITFIELD_RANGE(16, 26);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(27, 31);
        };
        DWORD           Value;
    } DW9;

    // DWORD 10
    union {
        struct {
            DWORD       S6 : BITFIELD_RANGE(0, 10);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(11, 15);
            DWORD       S7 : BITFIELD_RANGE(16, 26);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(27, 31);
        };
        DWORD           Value;
    } DW10;

    // DWORD 11
    union {
        struct {
            DWORD       S8 : BITFIELD_RANGE(0, 10);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(11, 15);
            DWORD       S9 : BITFIELD_RANGE(16, 26);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(27, 31);
        };
        DWORD           Value;
    } DW11;

    // DWORD 12
    union {
        struct {
            DWORD       S10 : BITFIELD_RANGE(0, 10);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(11, 15);
            DWORD       MaxAceLuma : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW12;

} VEBOX_ACE_LACE_STATE_G9, *PVEBOX_ACE_LACE_STATE_G9;


// Defined in vol2c "Vebox"
typedef struct _VEBOX_TCC_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(0, 6);
            DWORD       TccEnable : BITFIELD_BIT(7);
            DWORD       SatFactor1 : BITFIELD_RANGE(8, 15);
            DWORD       SatFactor2 : BITFIELD_RANGE(16, 23);
            DWORD       SatFactor3 : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(0, 7);
            DWORD       SatFactor4 : BITFIELD_RANGE(8, 15);
            DWORD       SatFactor5 : BITFIELD_RANGE(16, 23);
            DWORD       SatFactor6 : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       BaseColor1 : BITFIELD_RANGE(0, 9);
            DWORD       BaseColor2 : BITFIELD_RANGE(10, 19);
            DWORD       BaseColor3 : BITFIELD_RANGE(20, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD           Value;
    } DW2;

    // DWORD 3
    union {
        struct {
            DWORD       BaseColo4 : BITFIELD_RANGE(0, 9);
            DWORD       BaseColor5 : BITFIELD_RANGE(10, 19);
            DWORD       BaseColor6 : BITFIELD_RANGE(20, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD           Value;
    } DW3;

    // DWORD 4
    union {
        struct {
            DWORD       ColorTransitSlope2 : BITFIELD_RANGE(0, 15);
            DWORD       ColorTransitSlope23 : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW4;

    // DWORD 5
    union {
        struct {
            DWORD       ColorTransitSlope34 : BITFIELD_RANGE(0, 15);
            DWORD       ColorTransitSlope45 : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            DWORD       ColorTransitSlope56 : BITFIELD_RANGE(0, 15);
            DWORD       ColorTransitSlope61 : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW6;

    // DWORD 7
    union {
        struct {
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(0, 1);
            DWORD       ColorBias1 : BITFIELD_RANGE(2, 11);
            DWORD       ColorBias2 : BITFIELD_RANGE(12, 21);
            DWORD       ColorBias3 : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW7;

    // DWORD 8
    union {
        struct {
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(0, 1);
            DWORD       ColorBias4 : BITFIELD_RANGE(2, 11);
            DWORD       ColorBias5 : BITFIELD_RANGE(12, 21);
            DWORD       ColorBias6 : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW8;

    // DWORD 9
    union {
        struct {
            DWORD       SteSlopeBits : BITFIELD_RANGE(0, 2);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(3, 7);
            DWORD       SteThreshold : BITFIELD_RANGE(8, 12);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(13, 15);
            DWORD       UvThresholdBits : BITFIELD_RANGE(16, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 23);
            DWORD       UvThreshold : BITFIELD_RANGE(24, 30);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW9;

    // DWORD 10
    union {
        struct {
            DWORD       UVMaxColor : BITFIELD_RANGE(0, 8);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(9, 15);
            DWORD       InvUvmaxColor : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW10;

} VEBOX_TCC_STATE_G9, *PVEBOX_TCC_STATE_G9;


// Defined in vol2c "Vebox"
typedef struct _VEBOX_PROCAMP_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       ProcampEnable : BITFIELD_BIT(0);
            DWORD       Brightness : BITFIELD_RANGE(1, 12);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(13, 16);
            DWORD       Contrast : BITFIELD_RANGE(17, 27);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(28, 31);
        };
        DWORD           Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       SinCS : BITFIELD_RANGE(0, 15);
            DWORD       CosCS : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW1;

} VEBOX_PROCAMP_STATE_G9, *PVEBOX_PROCAMP_STATE_G9;


// Defined in vol2c "Vebox"
typedef struct _VEBOX_CSC_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       C0 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 29);
            DWORD       YuvChannelSwap : BITFIELD_BIT(30);
            DWORD       TransformEnable : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       C1 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       C2 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW2;

    // DWORD 3
    union {
        struct {
            DWORD       C3 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW3;

    // DWORD 4
    union {
        struct {
            DWORD       C4 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW4;

    // DWORD 5
    union {
        struct {
            DWORD       C5 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            DWORD       C6 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW6;

    // DWORD 7
    union {
        struct {
            DWORD       C7 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW7;

    // DWORD 8
    union {
        struct {
            DWORD       C8 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW8;

    // DWORD 9
    union {
        struct {
            DWORD       OffsetIn1 : BITFIELD_RANGE(0, 15);
            DWORD       OffsetOut1 : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW9;

    // DWORD 10
    union {
        struct {
            DWORD       OffsetIn2 : BITFIELD_RANGE(0, 15);
            DWORD       OffsetOut2 : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW10;

    // DWORD 11
    union {
        struct {
            DWORD       OffsetIn3 : BITFIELD_RANGE(0, 15);
            DWORD       OffsetOut3 : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW11;

} VEBOX_CSC_STATE_G9, *PVEBOX_CSC_STATE_G9;


// Defined in vol2c "Vebox"
typedef struct _VEBOX_ALPHA_AOI_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       ColorPipeAlpha : BITFIELD_RANGE(0, 15);
            DWORD       AlphaFromStateSelect : BITFIELD_BIT(16);
            DWORD       FullImageHistogram : BITFIELD_BIT(17);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(18, 31);
        };
        DWORD           Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       AoiMinX : BITFIELD_RANGE(0, 13);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(14, 15);
            DWORD       AoiMaxX : BITFIELD_RANGE(16, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD           Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       AoiMinY : BITFIELD_RANGE(0, 13);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(14, 15);
            DWORD       AoiMaxY : BITFIELD_RANGE(16, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD           Value;
    } DW2;

} VEBOX_ALPHA_AOI_STATE_G9, *PVEBOX_ALPHA_AOI_STATE_G9;


// Defined in vol2c "Vebox"
typedef struct _VEBOX_CCM_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       C1 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 30);
            DWORD       ColorCorrectionMatrixEnable : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       C0 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 31);
        };
        DWORD           Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       C3 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 31);
        };
        DWORD           Value;
    } DW2;

    // DWORD 3
    union {
        struct {
            DWORD       C2 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 31);
        };
        DWORD           Value;
    } DW3;

    // DWORD 4
    union {
        struct {
            DWORD       C5 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 31);
        };
        DWORD           Value;
    } DW4;

    // DWORD 5
    union {
        struct {
            DWORD       C4 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 31);
        };
        DWORD           Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            DWORD       C7 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 31);
        };
        DWORD           Value;
    } DW6;

    // DWORD 7
    union {
        struct {
            DWORD       C6 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 31);
        };
        DWORD           Value;
    } DW7;

    // DWORD 8
    union {
        struct {
            DWORD       C8 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 31);
        };
        DWORD           Value;
    } DW8;

} VEBOX_CCM_STATE_G9, *PVEBOX_CCM_STATE_G9;

// Defined in vol2c "Vebox"
typedef struct _VEBOX_FRONT_END_CSC_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       FecscC0TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 30);
            DWORD       FrontEndCscTransformEnable : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       FecscC1TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       FecscC2TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW2;

    // DWORD 3
    union {
        struct {
            DWORD       FecscC3TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW3;

    // DWORD 4
    union {
        struct {
            DWORD       FecscC4TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW4;

    // DWORD 5
    union {
        struct {
            DWORD       FecscC5TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            DWORD       FecscC6TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW6;

    // DWORD 7
    union {
        struct {
            DWORD       FecscC7TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW7;

    // DWORD 8
    union {
        struct {
            DWORD       FecscC8TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW8;

    // DWORD 9
    union {
        struct {
            DWORD       FecScOffsetIn1OffsetInForYR : BITFIELD_RANGE(0, 15);
            DWORD       FecScOffsetOut1OffsetOutForYR : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW9;

    // DWORD 10
    union {
        struct {
            DWORD       FecScOffsetIn2OffsetOutForUG : BITFIELD_RANGE(0, 15);
            DWORD       FecScOffsetOut2OffsetOutForUG : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW10;

    // DWORD 11
    union {
        struct {
            DWORD       FecScOffsetIn3OffsetOutForVB : BITFIELD_RANGE(0, 15);
            DWORD       FecScOffsetOut3OffsetOutForVB : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW11;

} VEBOX_FRONT_END_CSC_STATE_G9, *PVEBOX_FRONT_END_CSC_STATE_G9;

// Defined in vol2c "Vebox"
typedef struct _VEBOX_IECP_STATE_G9
{
    // DWORD 0_28
    VEBOX_STD_STE_STATE_G9                      StdSteState;

    // DWORD 29_41
    VEBOX_ACE_LACE_STATE_G9                     AceLaceState;

    // DWORD 42_52
    VEBOX_TCC_STATE_G9                          TccState;

    // DWORD 53_54
    VEBOX_PROCAMP_STATE_G9                      ProcAmpState;

    // DWORD 55_66
    VEBOX_CSC_STATE_G9                          CscState;

    // DWORD 67_69
    VEBOX_ALPHA_AOI_STATE_G9                    AlphaAoiState;

    // DWORD 70_78
    VEBOX_CCM_STATE_G9                          CcmState;

    // DWORD 79_90
    VEBOX_FRONT_END_CSC_STATE_G9                FrontEndCscState;
} VEBOX_IECP_STATE_G9, *PVEBOX_IECP_STATE_G9;


// Defined in vol2c "VEBOX"
typedef struct _VEBOX_CAPTURE_PIPE_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       GoodPixelNeighborThreshold : BITFIELD_RANGE(0, 5);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(6, 7);
            DWORD       AverageColorThreshold : BITFIELD_RANGE(8, 15);
            DWORD       GreenImbalanceThreshold : BITFIELD_RANGE(16, 19);
            DWORD       ShiftMinCost : BITFIELD_RANGE(20, 22);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(23);
            DWORD       GoodPixelThreshold : BITFIELD_RANGE(24, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       BadColorThreshold3 : BITFIELD_RANGE(0, 3);
            DWORD       NumberBigPixelTheshold : BITFIELD_RANGE(4, 7);
            DWORD       BadColorThreshold2 : BITFIELD_RANGE(8, 15);
            DWORD       BadColorThreshold1 : BITFIELD_RANGE(16, 23);
            DWORD       GoodIntesityThreshold : BITFIELD_RANGE(24, 27);
            DWORD       ScaleForMinCost : BITFIELD_RANGE(28, 31);
        };
        DWORD Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       WhiteBalanceCorrectionEnable : BITFIELD_BIT(0);
            DWORD       BlackPointCorrectionEnable : BITFIELD_BIT(1);
            DWORD       VignetteCorrectionFormat : BITFIELD_BIT(2);
            DWORD       RgbHistogramEnable : BITFIELD_BIT(3);
            DWORD       BlackPointOffsetGreenBottomMsb : BITFIELD_BIT(4);
            DWORD       BlackPointOffsetBlueMsb : BITFIELD_BIT(5);
            DWORD       BlackPointOffsetGreenTopMsb : BITFIELD_BIT(6);
            DWORD       BlackPointOffsetRedMsb : BITFIELD_BIT(7);
            DWORD       UvThresholdValue : BITFIELD_RANGE(8, 15);
            DWORD       YOutlierValue : BITFIELD_RANGE(16, 23);
            DWORD       YBrightValue : BITFIELD_RANGE(24, 31);
        };
        DWORD Value;
    } DW2;

    // DWORD 3
    union {
        struct {
            DWORD       BlackPointOffsetGreenTop : BITFIELD_RANGE(0, 15);
            DWORD       BlackPointOffsetRed : BITFIELD_RANGE(16, 31);
        };
        DWORD Value;
    } DW3;

    // DWORD 4
    union {
        struct {
            DWORD       BlackPointOffsetGreenBottom : BITFIELD_RANGE(0, 15);
            DWORD       BlackPointOffsetBlue : BITFIELD_RANGE(16, 31);
        };
        DWORD Value;
    } DW4;

    // DWORD 5
    union {
        struct {
            DWORD       WhiteBalanceGreenTopCorrection : BITFIELD_RANGE(0, 15);
            DWORD       WhiteBalanceRedCorrection : BITFIELD_RANGE(16, 31);
        };
        DWORD Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            DWORD       WhiteBalanceGreenBottomCorrection : BITFIELD_RANGE(0, 15);
            DWORD       WhiteBalanceBlueCorrection : BITFIELD_RANGE(16, 31);
        };
        DWORD Value;
    } DW6;

} VEBOX_CAPTURE_PIPE_STATE_G9, *PVEBOX_CAPTURE_PIPE_STATE_G9;




typedef struct __CM_VEBOX_PARAM_G9
{
    PVEBOX_DNDI_STATE_G9         pDndiState;
    unsigned char                padding1[4024];
    PVEBOX_IECP_STATE_G9         pIecpState;
    unsigned char                padding2[3732];
    PVEBOX_GAMUT_STATE_G75       pGamutState;
    unsigned char                padding3[3936];
    PVEBOX_VERTEX_TABLE_G75      pVertexTable;
    unsigned char                padding4[2048];
    PVEBOX_CAPTURE_PIPE_STATE_G9 pCapturePipe;
}CM_VEBOX_PARAM_G9, PCM_VEBOX_PARAM_G9;

#endif //__CM_RT_G9_H__
