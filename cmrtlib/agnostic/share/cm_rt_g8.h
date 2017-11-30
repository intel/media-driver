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
//! \file      cm_rt_g8.h
//! \brief     Contains Definitions for CM on Gen 8
//!

#ifndef __CM_RT_G8_H__
#define __CM_RT_G8_H__

#define BDW_L3_PLANE_DEFAULT    CM_L3_PLANE_DEFAULT
#define BDW_L3_PLANE_1          CM_L3_PLANE_1
#define BDW_L3_PLANE_2          CM_L3_PLANE_2
#define BDW_L3_PLANE_3          CM_L3_PLANE_3
#define BDW_L3_PLANE_4          CM_L3_PLANE_4
#define BDW_L3_PLANE_5          CM_L3_PLANE_5
#define BDW_L3_PLANE_6          CM_L3_PLANE_6
#define BDW_L3_PLANE_7          CM_L3_PLANE_7
#define BDW_L3_CONFIG_COUNT     8

#define BDW_SLM_PLANE_DEFAULT = BDW_L3_PLANE_5

static const L3ConfigRegisterValues BDW_L3_PLANE[BDW_L3_CONFIG_COUNT] =
{                                                 // SLM    URB   Rest     DC     RO     I/S    C    T      Sum ( BDW GT2; for GT1, half of the values; for GT3, double the values )
    { 0, 0, 0, 0x60000060 },                      //{  0,   384,    384,     0,     0,    0,    0,    0,    768},
    { 0, 0, 0, 0x00410060 },                      //{  0,   384,      0,   128,   256,    0,    0,    0,    768},
    { 0, 0, 0, 0x00418040 },                      //{  0,   256,      0,   128,   384,    0,    0,    0,    768},
    { 0, 0, 0, 0x00020040 },                      //{  0,   256,      0,     0,   512,    0,    0,    0,    768},
    { 0, 0, 0, 0x80000040 },                      //{  0,   256,    512,     0,     0,    0,    0,    0,    768},
    { 0, 0, 0, 0x60000021 },                      //{192,   128,    384,     0,     0,    0,    0,    0,    768},
    { 0, 0, 0, 0x00410021 },                      //{192,   128,      0,   128,   256,    0,    0,    0,    768},
    { 0, 0, 0, 0x00808021 }                       //{192,   128,      0,   256,   128,    0,    0,    0,    768}
};

// Defined in vol2b "Media"
typedef struct _VEBOX_DNDI_STATE_G75
{
    // DWORD 0
    union
    {
        struct
        {
            DWORD       DWordLength                         : 12;
            DWORD                                           : 4;
            DWORD       InstructionSubOpcodeB               : 5;
            DWORD       InstructionSubOpcodeA               : 3;
            DWORD       InstructionOpcode                   : 3;
            DWORD       InstructionPipeline                 : 2;
            DWORD       CommandType                         : 3;
        };
        struct
        {
            DWORD       Value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            DWORD       DenoiseASDThreshold                 : 8; // U8
            DWORD       DnmhDelta                           : 4; // UINT4
            DWORD                                           : 4; // Reserved
            DWORD       DnmhHistoryMax                      : 8; // U8
            DWORD       DenoiseSTADThreshold                : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW1;

    // DWORD 2
    union
    {
        struct
        {
            DWORD       SCMDenoiseThreshold                 : 8; // U8
            DWORD       DenoiseMovingPixelThreshold         : 5; // U5
            DWORD       STMMC2                              : 3; // U3
            DWORD       LowTemporalDifferenceThreshold      : 6; // U6
            DWORD                                           : 2; // Reserved
            DWORD       TemporalDifferenceThreshold         : 6; // U6
            DWORD                                           : 2; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW2;

    // DWORD 3
    union
    {
        struct
        {
            DWORD       BlockNoiseEstimateNoiseThreshold    : 8; // U8
            DWORD       BneEdgeTh                           : 4; // UINT4
            DWORD                                           : 2; // Reserved
            DWORD       SmoothMvTh                          : 2; // U2
            DWORD       SADTightTh                          : 4; // U4
            DWORD       CATSlopeMinus1                      : 4; // U4
            DWORD       GoodNeighborThreshold               : 6; // UINT6
            DWORD                                           : 2; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW3;

    // DWORD 4
    union
    {
        struct
        {
            DWORD       MaximumSTMM                         : 8; // U8
            DWORD       MultiplierforVECM                   : 6; // U6
            DWORD                                           : 2;
            DWORD       BlendingConstantForSmallSTMM        : 8; // U8
            DWORD       BlendingConstantForLargeSTMM        : 7; // U7
            DWORD       STMMBlendingConstantSelect          : 1; // U1
        };
        struct
        {
            DWORD       Value;
        };
    } DW4;

    // DWORD 5
    union
    {
        struct
        {
            DWORD       SDIDelta                            : 8; // U8
            DWORD       SDIThreshold                        : 8; // U8
            DWORD       STMMOutputShift                     : 4; // U4
            DWORD       STMMShiftUp                         : 2; // U2
            DWORD       STMMShiftDown                       : 2; // U2
            DWORD       MinimumSTMM                         : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW5;

    // DWORD 6
    union
    {
        struct
        {
            DWORD       FMDTemporalDifferenceThreshold      : 8; // U8
            DWORD       SDIFallbackMode2Constant            : 8; // U8
            DWORD       SDIFallbackMode1T2Constant          : 8; // U8
            DWORD       SDIFallbackMode1T1Constant          : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW6;

    // DWORD 7
    union
    {
        struct
        {
            DWORD                                           : 3; // Reserved
            DWORD       DNDITopFirst                        : 1; // Enable
            DWORD                                           : 2; // Reserved
            DWORD       ProgressiveDN                       : 1; // Enable
            DWORD       MCDIEnable                          : 1;
            DWORD       FMDTearThreshold                    : 6; // U6
            DWORD       CATTh1                              : 2; // U2
            DWORD       FMD2VerticalDifferenceThreshold     : 8; // U8
            DWORD       FMD1VerticalDifferenceThreshold     : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW7;

    // DWORD 8
    union
    {
        struct
        {
            DWORD       SADTHA                              : 4; // U4
            DWORD       SADTHB                              : 4; // U4
            DWORD       FMDFirstFieldCurrentFrame           : 2; // U2
            DWORD       MCPixelConsistencyTh                : 6; // U6
            DWORD       FMDSecondFieldPreviousFrame         : 2; // U2
            DWORD                                           : 1; // Reserved
            DWORD       NeighborPixelTh                     : 4; // U4
            DWORD       DnmhHistoryInit                     : 6; // U6
            DWORD                                           : 3; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW8;

    // DWORD 9
    union
    {
        struct
        {
            DWORD       ChromaLTDThreshold                  : 6; // U6
            DWORD       ChromaTDTheshold                    : 6; // U6
            DWORD       ChromaDenoiseEnable                 : 1; // Enable
            DWORD                                           : 3; // Reserved
            DWORD       ChromaDnSTADThreshold               : 8; // U8
            DWORD                                           : 8; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW9;

    // Padding for 32-byte alignment, VEBOX_DNDI_STATE_G75 is 10 DWORDs
    DWORD dwPad[6];
} VEBOX_DNDI_STATE_G75, *PVEBOX_DNDI_STATE_G75;

// Defined in vol2b "Media"
typedef struct _VEBOX_IECP_STATE_G75
{
    // STD/STE state
    // DWORD 0
    union
    {
        struct
        {
            DWORD       STDEnable                       : 1;    
            DWORD       STEEnable                       : 1;    
            DWORD       OutputCtrl                      : 1;    
            DWORD                                       : 1;    
            DWORD       SatMax                          : 6;    // U6;
            DWORD       HueMax                          : 6;    // U6;
            DWORD       UMid                            : 8;    // U8;
            DWORD       VMid                            : 8;    // U8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            DWORD       SinAlpha                        : 8;    // S0.7
            DWORD                                       : 2; 
            DWORD       CosAlpha                        : 8;    // S0.7    
            DWORD       HSMargin                        : 3;    // U3   
            DWORD       DiamondDu                       : 7;    // S7    
            DWORD       DiamondMargin                   : 3;    // U3    
            DWORD                                       : 1;    
        };
        struct
        {
            DWORD       Value;
        };
    } DW1;

    // DWORD 2
    union
    {
        struct
        {
            DWORD       DiamondDv                       : 7;    // S7  
            DWORD       DiamondTh                       : 6;    // U6  
            DWORD       DiamondAlpha                    : 8;    // U2.6  
            DWORD                                       : 11;    
        };
        struct
        {
            DWORD       Value;
        };
    } DW2;

    // DWORD 3
    union
    {
        struct
        {
            DWORD                                       : 7;  
            DWORD       VYSTDEnable                     : 1;    
            DWORD       YPoint1                         : 8;    // U8   
            DWORD       YPoint2                         : 8;    // U8   
            DWORD       YPoint3                         : 8;    // U8   
        };
        struct
        {
            DWORD       Value;
        };
    } DW3;

    // DWORD 4
    union
    {
        struct
        {
            DWORD       YPoint4                         : 8;    // U8
            DWORD       YSlope1                         : 5;    // U2.3
            DWORD       YSlope2                         : 5;    // U2.3
            DWORD                                       : 14;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW4;

    // DWORD 5
    union
    {
        struct
        {
            DWORD       INVMarginVYL                    : 16;    // U0.16
            DWORD       INVSkinTypesMargin              : 16;    // U0.16
        };
        struct
        {
            DWORD       Value;
        };
    } DW5;

    // DWORD 6
    union
    {
        struct
        {
            DWORD       INVMarginVYU                    : 16;    // U0.16
            DWORD       P0L                             : 8;     // U8
            DWORD       P1L                             : 8;     // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW6;

    // DWORD 7
    union
    {
        struct
        {
            DWORD       P2L                             : 8;     // U8
            DWORD       P3L                             : 8;     // U8
            DWORD       B0L                             : 8;     // U8
            DWORD       B1L                             : 8;     // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW7;

    // DWORD 8
    union
    {
        struct
        {
            DWORD       B2L                             : 8;     // U8
            DWORD       B3L                             : 8;     // U8
            DWORD       S0L                             : 11;    // S2.8
            DWORD                                       : 5;     
        };
        struct
        {
            DWORD       Value;
        };
    } DW8;

    // DWORD 9
    union
    {
        struct
        {
            DWORD       S1L                             : 11;    // S2.8
            DWORD       S2L                             : 11;    // S2.8
            DWORD                                       : 10;     
        };
        struct
        {
            DWORD       Value;
        };
    } DW9;

    // DWORD 10
    union
    {
        struct
        {
            DWORD       S3L                             : 11;    // S2.8
            DWORD       P0U                             : 8;     // U8
            DWORD       P1U                             : 8;     // U8
            DWORD                                       : 5;     
        };
        struct
        {
            DWORD       Value;
        };
    } DW10;

    // DWORD 11
    union
    {
        struct
        {
            DWORD       P2U                             : 8;     // U8
            DWORD       P3U                             : 8;     // U8
            DWORD       B0U                             : 8;     // U8
            DWORD       B1U                             : 8;     // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW11;

    // DWORD 12
    union
    {
        struct
        {
            DWORD       B2U                             : 8;     // U8
            DWORD       B3U                             : 8;     // U8
            DWORD       S0U                             : 11;    // S2.8
            DWORD                                       : 5;     
        };
        struct
        {
            DWORD       Value;
        };
    } DW12;

    // DWORD 13
    union
    {
        struct
        {
            DWORD       S1U                             : 11;     // S2.8
            DWORD       S2U                             : 11;     // S2.8
            DWORD                                       : 10;    
        };
        struct
        {
            DWORD       Value;
        };
    } DW13;

    // DWORD 14
    union
    {
        struct
        {
            DWORD       S3U                             : 11;     // S2.8
            DWORD       SkinTypesEnable                 : 1;
            DWORD       SkinTypesThresh                 : 8;      // U8
            DWORD       SkinTypesMargin                 : 8;      // U8
            DWORD                                       : 4;    
        };
        struct
        {
            DWORD       Value;
        };
    } DW14;

    // DWORD 15
    union
    {
        struct
        {
            DWORD       SATP1                           : 7;     // S6
            DWORD       SATP2                           : 7;     // S6
            DWORD       SATP3                           : 7;     // S6
            DWORD       SATB1                           : 10;    // S7.2
            DWORD                                       : 1;    
        };
        struct
        {
            DWORD       Value;
        };
    } DW15;

    // DWORD 16
    union
    {
        struct
        {
            DWORD       SATB2                           : 10;     // S7.2
            DWORD       SATB3                           : 10;     // S7.2
            DWORD       SATS0                           : 11;     // U3.8
            DWORD                                       : 1;    
        };
        struct
        {
            DWORD       Value;
        };
    } DW16;

    // DWORD 17
    union
    {
        struct
        {
            DWORD       SATS1                           : 11;     // U3.8
            DWORD       SATS2                           : 11;     // U3.8
            DWORD                                       : 10;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW17;

    // DWORD 18
    union
    {
        struct
        {
            DWORD       SATS3                           : 11;    // U3.8
            DWORD       HUEP1                           : 7;     // U3.8
            DWORD       HUEP2                           : 7;     // U3.8
            DWORD       HUEP3                           : 7;     // U3.8
        };
        struct
        {
            DWORD       Value;
        };
    } DW18;

    // DWORD 19
    union
    {
        struct
        {
            DWORD       HUEB1                           : 10;    // S7.2
            DWORD       HUEB2                           : 10;    // S7.2
            DWORD       HUEB3                           : 10;    // S7.2
            DWORD                                       : 2; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW19;

    // DWORD 20
    union
    {
        struct
        {
            DWORD       HUES0                           : 11;    // U3.8
            DWORD       HUES1                           : 11;    // U3.8
            DWORD                                       : 10; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW20;

    // DWORD 21
    union
    {
        struct
        {
            DWORD       HUES2                           : 11;    // U3.8
            DWORD       HUES3                           : 11;    // U3.8
            DWORD                                       : 10; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW21;

    // DWORD 22
    union
    {
        struct
        {
            DWORD       SATP1DARK                      : 7;     // S6
            DWORD       SATP2DARK                      : 7;     // S6
            DWORD       SATP3DARK                      : 7;     // S6
            DWORD       SATB1DARK                      : 10;    // S7.2
            DWORD                                      : 1; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW22;

    // DWORD 23
    union
    {
        struct
        {
            DWORD       SATB2DARK                      : 10;    // S7.2
            DWORD       SATB3DARK                      : 10;    // S7.2
            DWORD       SATS0DARK                      : 11;    // U3.8
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW23;

    // DWORD 24
    union
    {
        struct
        {
            DWORD       SATS1DARK                      : 11;    // U3.8
            DWORD       SATS2DARK                      : 11;    // U3.8
            DWORD                                      : 10; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW24;

    // DWORD 25
    union
    {
        struct
        {
            DWORD       SATS3DARK                      : 11;   // U3.8
            DWORD       HUEP1DARK                      : 7;    // S6
            DWORD       HUEP2DARK                      : 7;    // S6
            DWORD       HUEP3DARK                      : 7;    // S6
        };
        struct
        {
            DWORD       Value;
        };
    } DW25;

    // DWORD 26
    union
    {
        struct
        {
            DWORD       HUEB1DARK                      : 10;    // S7.2
            DWORD       HUEB2DARK                      : 10;    // S7.2
            DWORD       HUEB3DARK                      : 10;    // S7.2
            DWORD                                      : 2; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW26;

    // DWORD 27
    union
    {
        struct
        {
            DWORD       HUES0DARK                      : 11;    // U3.8
            DWORD       HUES1DARK                      : 11;    // U3.8
            DWORD                                      : 10; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW27;

    // DWORD 28
    union
    {
        struct
        {
            DWORD       HUES2DARK                      : 11;    // U3.8
            DWORD       HUES3DARK                      : 11;    // U3.8
            DWORD                                      : 10; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW28;

    // DWORD 29
    union
    {
        struct
        {
            DWORD       ACEEnable                       : 1;  
            DWORD       FullImageHistogram              : 1;
            DWORD       SkinThreshold                   : 5;    // U5
            DWORD                                       : 25; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW29;

    // DWORD 30
    union
    {
        struct
        {
            DWORD       Ymin                            : 8;  
            DWORD       Y1                              : 8;
            DWORD       Y2                              : 8;
            DWORD       Y3                              : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW30;

    // DWORD 31
    union
    {
        struct
        {
            DWORD       Y4                              : 8;  
            DWORD       Y5                              : 8;
            DWORD       Y6                              : 8;
            DWORD       Y7                              : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW31;

    // DWORD 32
    union
    {
        struct
        {
            DWORD       Y8                              : 8;  
            DWORD       Y9                              : 8;
            DWORD       Y10                             : 8;
            DWORD       Ymax                            : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW32;

    // DWORD 33
    union
    {
        struct
        {
            DWORD       B1                              : 8;  // U8
            DWORD       B2                              : 8;  // U8
            DWORD       B3                              : 8;  // U8
            DWORD       B4                              : 8;  // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW33;

    // DWORD 34
    union
    {
        struct
        {
            DWORD       B5                              : 8;  // U8
            DWORD       B6                              : 8;  // U8
            DWORD       B7                              : 8;  // U8
            DWORD       B8                              : 8;  // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW34;

    // DWORD 35
    union
    {
        struct
        {
            DWORD       B9                              : 8;   // U8
            DWORD       B10                             : 8;   // U8
            DWORD                                       : 16;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW35;

    // DWORD 36
    union
    {
        struct
        {
            DWORD       S0                              : 11;   // U11
            DWORD                                       : 5; 
            DWORD       S1                              : 11;   // U11
            DWORD                                       : 5; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW36;

    // DWORD 37
    union
    {
        struct
        {
            DWORD       S2                              : 11;   // U11
            DWORD                                       : 5; 
            DWORD       S3                              : 11;   // U11
            DWORD                                       : 5; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW37;

    // DWORD 38
    union
    {
        struct
        {
            DWORD       S4                              : 11;   // U11
            DWORD                                       : 5; 
            DWORD       S5                              : 11;   // U11
            DWORD                                       : 5; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW38;

    // DWORD 39
    union
    {
        struct
        {
            DWORD       S6                              : 11;   // U11
            DWORD                                       : 5; 
            DWORD       S7                              : 11;   // U11
            DWORD                                       : 5; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW39;

    // DWORD 40
    union
    {
        struct
        {
            DWORD       S8                              : 11;   // U11
            DWORD                                       : 5; 
            DWORD       S9                              : 11;   // U11
            DWORD                                       : 5; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW40;

    // DWORD 41
    union
    {
        struct
        {
            DWORD       S10                             : 11;   // U11
            DWORD                                       : 21; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW41;

    // TCC State
    // DWORD 42
    union
    {
        struct
        {
            DWORD                                       : 7; 
            DWORD       TCCEnable                       : 1; 
            DWORD       SatFactor1                      : 8;    // U8 
            DWORD       SatFactor2                      : 8;    // U8 
            DWORD       SatFactor3                      : 8;    // U8 
        };
        struct
        {
            DWORD       Value;
        };
    } DW42;

    // DWORD 43
    union
    {
        struct
        {
            DWORD                                       : 8; 
            DWORD       SatFactor4                      : 8;    // U8 
            DWORD       SatFactor5                      : 8;    // U8 
            DWORD       SatFactor6                      : 8;    // U8 
        };
        struct
        {
            DWORD       Value;
        };
    } DW43;

    // DWORD 44
    union
    {
        struct
        {
            DWORD       BaseColor1                      : 10;    // U10 
            DWORD       BaseColor2                      : 10;    // U10 
            DWORD       BaseColor3                      : 10;    // U10 
            DWORD                                       : 2; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW44;

    // DWORD 45
    union
    {
        struct
        {
            DWORD       BaseColor4                      : 10;    // U10 
            DWORD       BaseColor5                      : 10;    // U10 
            DWORD       BaseColor6                      : 10;    // U10 
            DWORD                                       : 2; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW45;

    // DWORD 46
    union
    {
        struct
        {
            DWORD       ColorTransitSlope12              : 16;    // U16
            DWORD       ColorTransitSlope23              : 16;    // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW46;

    // DWORD 47
    union
    {
        struct
        {
            DWORD       ColorTransitSlope34              : 16;    // U16
            DWORD       ColorTransitSlope45              : 16;    // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW47;

    // DWORD 48
    union
    {
        struct
        {
            DWORD       ColorTransitSlope56              : 16;    // U16
            DWORD       ColorTransitSlope61              : 16;    // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW48;

    // DWORD 49
    union
    {
        struct
        {
            DWORD                                       : 2;    
            DWORD       ColorBias1                      : 10;    // U10
            DWORD       ColorBias2                      : 10;    // U10
            DWORD       ColorBias3                      : 10;    // U10
        };
        struct
        {
            DWORD       Value;
        };
    } DW49;

    // DWORD 50
    union
    {
        struct
        {
            DWORD                                       : 2;    
            DWORD       ColorBias4                      : 10;    // U10
            DWORD       ColorBias5                      : 10;    // U10
            DWORD       ColorBias6                      : 10;    // U10
        };
        struct
        {
            DWORD       Value;
        };
    } DW50;

    // DWORD 51
    union
    {
        struct
        {
            DWORD       STESlopeBits                    : 3;    // U3
            DWORD                                       : 5;
            DWORD       STEThreshold                    : 5;    // U5
            DWORD                                       : 3;
            DWORD       UVThresholdBits                 : 3;    // U5
            DWORD                                       : 5;
            DWORD       UVThreshold                     : 7;    // U7
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW51;

    // DWORD 52
    union
    {
        struct
        {
            DWORD       UVMaxColor                      : 9;    // U9
            DWORD                                       : 7;
            DWORD       InvUVMaxColor                   : 16;   // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW52;

    // ProcAmp
    // DWORD 53
    union
    {
        struct
        {
            DWORD       ProcAmpEnable                   : 1;    
            DWORD       Brightness                      : 12;    // S7.4
            DWORD                                       : 4;
            DWORD       Contrast                        : 11;    // U7.4
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW53;

    // DWORD 54
    union
    {
        struct
        {
            DWORD       SINCS                           : 16;    // S7.8    
            DWORD       COSCS                           : 16;    // S7.8
        };
        struct
        {
            DWORD       Value;
        };
    } DW54;

    // CSC State
    // DWORD 55
    union
    {
        struct
        {
            DWORD       TransformEnable                 : 1;
            DWORD       YUVChannelSwap                  : 1;
            DWORD                                       : 1;
            DWORD       C0                              : 13;  // S2.10
            DWORD       C1                              : 13;  // S2.10
            DWORD                                       : 3;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW55;

    // DWORD 56
    union
    {
        struct
        {
            DWORD       C2                              : 13;  // S2.10
            DWORD       C3                              : 13;  // S2.10
            DWORD                                       : 6;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW56;

    // DWORD 57
    union
    {
        struct
        {
            DWORD       C4                              : 13;  // S2.10
            DWORD       C5                              : 13;  // S2.10
            DWORD                                       : 6;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW57;

    // DWORD 58
    union
    {
        struct
        {
            DWORD       C6                              : 13;  // S2.10
            DWORD       C7                              : 13;  // S2.10
            DWORD                                       : 6;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW58;

    // DWORD 59
    union
    {
        struct
        {
            DWORD       C8                              : 13;   // S2.10
            DWORD                                       : 19;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW59;

    // DWORD 60
    union
    {
        struct
        {
            DWORD       OffsetIn1                       : 11;   // S8.2
            DWORD       OffsetOut1                      : 11;   // S8.2
            DWORD                                       : 10;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW60;

    // DWORD 61
    union
    {
        struct
        {
            DWORD       OffsetIn2                       : 11;   // S8.2
            DWORD       OffsetOut2                      : 11;   // S8.2
            DWORD                                       : 10;
        };
        struct
        {
            DWORD       Value;
        };
    } DW61;

    // DWORD 62
    union
    {
        struct
        {
            DWORD       OffsetIn3                       : 11;  // S8.2
            DWORD       OffsetOut3                      : 11;  // S8.2
            DWORD                                       : 10;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW62;

    // DWORD 63
    union
    {
        struct
        {
            DWORD       ColorPipeAlpha                  : 12;  // U12
            DWORD                                       : 4;
            DWORD       AlphaFromStateSelect            : 1;   
            DWORD                                       : 15;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW63;

    // Area of Interest
    // DWORD 64
    union
    {
        struct
        {
            DWORD       AOIMinX                         : 16;  // U16
            DWORD       AOIMaxX                         : 16;  // U16
    };
        struct
        {
            DWORD       Value;
        };
    } DW64;

    // DWORD 65
    union
    {
        struct
        {
            DWORD       AOIMinY                         : 16;  // U16
            DWORD       AOIMaxY                         : 16;  // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW65;

    // Padding for 32-byte alignment, VEBOX_IECP_STATE_G75 is 66 DWORDs
    DWORD dwPad[6];
} VEBOX_IECP_STATE_G75, *PVEBOX_IECP_STATE_G75;

// Defined in vol2b "Media"
typedef struct _VEBOX_GAMUT_STATE_G75
{
    // GEC State
    // DWORD 0
    union
    {
        struct
        {
            DWORD       WeightingFactorForGainFactor    : 10;
            DWORD                                       : 5;
            DWORD       GlobalModeEnable                : 1;
            DWORD       GainFactorR                     : 9;
            DWORD                                       : 7;
        };
        struct
        {
            DWORD       Value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            DWORD       GainFactorB                     : 7;
            DWORD                                       : 1;
            DWORD       GainFactorG                     : 7;
            DWORD                                       : 1;
            DWORD       AccurateColorComponentScaling   : 10;
            DWORD                                       : 6;
        };
        struct
        {
            DWORD       Value;
        };
    } DW1;

    // DWORD 2
    union
    {
        struct
        {
            DWORD       RedOffset                       : 8;
            DWORD       AccurateColorComponentOffset    : 8;
            DWORD       RedScaling                      : 10;
            DWORD                                           : 6;
        };
        struct
        {
            DWORD       Value;
        };
    } DW2;

    // 3x3 Transform Coefficient
    // DWORD 3
    union
    {
        struct
        {
            DWORD       C0Coeff                         : 15; 
            DWORD                                       : 1;
            DWORD       C1Coeff                         : 15;
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW3;

    // DWORD 4
    union
    {
        struct
        {
            DWORD       C2Coeff                         : 15;
            DWORD                                       : 1;
            DWORD       C3Coeff                         : 15;
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW4;

    // DWORD 5
    union
    {
        struct
        {
            DWORD       C4Coeff                         : 15;
            DWORD                                       : 1;
            DWORD       C5Coeff                         : 15;
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW5;

    // DWORD 6
    union
    {
        struct
        {
            DWORD       C6Coeff                         : 15;
            DWORD                                       : 1;
            DWORD       C7Coeff                         : 15;
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW6;

    // DWORD 7
    union
    {
        struct
        {
            DWORD       C8Coeff                         : 15;
            DWORD                                       : 17;
        };
        struct
        {
            DWORD       Value;
        };
    } DW7;
    
    // PWL Values for Gamma Correction
    // DWORD 8
    union
    {
        struct
        {
            DWORD       PWLGammaPoint1                  : 8;
            DWORD       PWLGammaPoint2                  : 8;
            DWORD       PWLGammaPoint3                  : 8;
            DWORD       PWLGammaPoint4                  : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW8;

    // DWORD 9
    union
    {
        struct
        {
            DWORD       PWLGammaPoint5                  : 8;
            DWORD       PWLGammaPoint6                  : 8;
            DWORD       PWLGammaPoint7                  : 8;
            DWORD       PWLGammaPoint8                  : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW9;

    // DWORD 10
    union
    {
        struct
        {
            DWORD       PWLGammaPoint9                  : 8;
            DWORD       PWLGammaPoint10                 : 8;
            DWORD       PWLGammaPoint11                 : 8;
            DWORD                                       : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW10;

    // DWORD 11
    union
    {
        struct
        {
            DWORD       PWLGammaBias1                   : 8;
            DWORD       PWLGammaBias2                   : 8;
            DWORD       PWLGammaBias3                   : 8;
            DWORD       PWLGammaBias4                   : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW11;


    // DWORD 12
    union
    {
        struct
        {
            DWORD       PWLGammaBias5                   : 8;
            DWORD       PWLGammaBias6                   : 8;
            DWORD       PWLGammaBias7                   : 8;
            DWORD       PWLGammaBias8                   : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW12;

    // DWORD 13
    union
    {
        struct
        {
            DWORD       PWLGammaBias9                   : 8;
            DWORD       PWLGammaBias10                  : 8;   
            DWORD       PWLGammaBias11                  : 8;
            DWORD                                       : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW13;

    // DWORD 14
    union
    {
        struct
        {
            DWORD       PWLGammaSlope0                  : 12;
            DWORD                                       : 4; 
            DWORD       PWLGammaSlope1                  : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW14;

    // DWORD 15
    union
    {
        struct
        {
            DWORD       PWLGammaSlope2                  : 12;
            DWORD                                       : 4; 
            DWORD       PWLGammaSlope3                  : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW15;

    // DWORD 16
    union
    {
        struct
        {
            DWORD       PWLGammaSlope4                  : 12;
            DWORD                                       : 4; 
            DWORD       PWLGammaSlope5                  : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW16;

    // DWORD 17
    union
    {
        struct
        {
            DWORD       PWLGammaSlope6                  : 12;
            DWORD                                       : 4; 
            DWORD       PWLGammaSlope7                  : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW17;

    // DWORD 18
    union
    {
        struct
        {
            DWORD       PWLGammaSlope8                  : 12;
            DWORD                                       : 4; 
            DWORD       PWLGammaSlope9                  : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW18;

    // DWORD 19
    union
    {
        struct
        {
            DWORD       PWLGammaSlope10                 : 12;
            DWORD                                       : 4; 
            DWORD       PWLGammaSlope11                 : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW19;

    // PWL Values for Inverse Gamma Correction
    // DWORD 20
    union
    {
        struct
        {
            DWORD       PWLInvGammaPoint1               : 8;
            DWORD       PWLInvGammaPoint2               : 8;
            DWORD       PWLInvGammaPoint3               : 8;
            DWORD       PWLInvGammaPoint4               : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW20;

    // DWORD 21
    union
    {
        struct
        {
            DWORD       PWLInvGammaPoint5               : 8;
            DWORD       PWLInvGammaPoint6               : 8;
            DWORD       PWLInvGammaPoint7               : 8;
            DWORD       PWLInvGammaPoint8               : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW21;

    // DWORD 22
    union
    {
        struct
        {
            DWORD       PWLInvGammaPoint9               : 8;
            DWORD       PWLInvGammaPoint10              : 8;
            DWORD       PWLInvGammaPoint11              : 8;
            DWORD                                       : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW22;

    // DWORD 23
    union
    {
        struct
        {
            DWORD       PWLInvGammaBias1                : 8;
            DWORD       PWLInvGammaBias2                : 8;
            DWORD       PWLInvGammaBias3                : 8;
            DWORD       PWLInvGammaBias4                : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW23;

    // DWORD 24
    union
    {
        struct
        {
            DWORD       PWLInvGammaBias5                : 8;
            DWORD       PWLInvGammaBias6                : 8;
            DWORD       PWLInvGammaBias7                : 8;
            DWORD       PWLInvGammaBias8                : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW24;

    // DWORD 25
    union
    {
        struct
        {
            DWORD       PWLInvGammaBias9                : 8;
            DWORD       PWLInvGammaBias10               : 8;
            DWORD       PWLInvGammaBias11               : 8;
            DWORD                                       : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW25;

    // DWORD 26
    union
    {
        struct
        {
            DWORD       PWLInvGammaSlope0               : 12;
            DWORD                                       : 4;
            DWORD       PWLInvGammaSlope1               : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW26;

    // DWORD 27
    union
    {
        struct
        {
            DWORD       PWLInvGammaSlope2               : 12;
            DWORD                                       : 4;
            DWORD       PWLInvGammaSlope3               : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW27;

    // DWORD 28
    union
    {
        struct
        {
            DWORD       PWLInvGammaSlope4               : 12;
            DWORD                                       : 4;
            DWORD       PWLInvGammaSlope5               : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW28;

    // DWORD 29
    union
    {
        struct
        {
            DWORD       PWLInvGammaSlope6               : 12;
            DWORD                                       : 4;
            DWORD       PWLInvGammaSlope7               : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW29;

    // DWORD 30
    union
    {
        struct
        {
            DWORD       PWLInvGammaSlope8               : 12;
            DWORD                                       : 4;
            DWORD       PWLInvGammaSlope9               : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW30;

    // DWORD 31
    union
    {
        struct
        {
            DWORD       PWLInvGammaSlope10              : 12;
            DWORD                                       : 4;
            DWORD       PWLInvGammaSlope11              : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW31;

    // Offset value for R, G, B for the Transform
    // DWORD 32
    union
    {
        struct
        {
            DWORD       OffsetInR                       : 15; 
            DWORD                                       : 1;
            DWORD       OffsetInG                       : 15;
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW32;

    // DWORD 33
    union
    {
        struct
        {
            DWORD       OffsetInB                       : 15; 
            DWORD                                       : 1;
            DWORD       OffsetOutB                      : 15;
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW33;

    // DWORD 34
    union
    {
        struct
        {
            DWORD       OffsetOutR                      : 15; 
            DWORD                                       : 1;
            DWORD       OffsetOutG                      : 15;
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW34;

    // GCC State
    // DWORD 35
    union
    {
        struct
        {
            DWORD       OuterTriangleMappingLengthBelow : 10;  // U10
            DWORD       OuterTriangleMappingLength      : 10;  // U10
            DWORD       InnerTriangleMappingLength      : 10;  // U10
            DWORD       FullRangeMappingEnable          : 1;
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW35;

    // DWORD 36
    union
    {
        struct
        {
            DWORD       InnerTriangleMappingLengthBelow : 10;   // U10
            DWORD                                       : 18;
            DWORD       CompressionLineShift            : 3;
            DWORD       xvYccDecEncEnable               : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW36;

    // DWORD 37
    union
    {
        struct
        {
            DWORD       CpiOverride                     : 1;
            DWORD                                       : 10;
            DWORD       BasicModeScalingFactor          : 14;
            DWORD                                       : 4;
            DWORD       LumaChromaOnlyCorrection        : 1;
            DWORD       GCCBasicModeSelection           : 2;
        };
        struct
        {
            DWORD       Value;
        };
    } DW37;

    // Padding for 32-byte alignment, VEBOX_GAMUT_STATE_G75 is 38 DWORDs
    DWORD dwPad[2];
} VEBOX_GAMUT_STATE_G75, *PVEBOX_GAMUT_STATE_G75;

#define CM_NUM_VERTEX_TABLE_ENTRIES_G75        512
// Defined in vol2b "Media"
typedef struct _VEBOX_VERTEX_TABLE_ENTRY_G75
{
    union
    {
        struct
        {
            DWORD       VertexTableEntryCv                  : 12;
            DWORD                                           : 4;
            DWORD       VertexTableEntryLv                  : 12;
            DWORD                                           : 4;
        };
        struct
        {
            DWORD       Value;
        };
    };
} VEBOX_VERTEX_TABLE_ENTRY_G75, *PVEBOX_VERTEX_TABLE_ENTRY_G75;

// Defined in vol2b "Media"
typedef struct _VEBOX_VERTEX_TABLE_G75
{
    VEBOX_VERTEX_TABLE_ENTRY_G75 VertexTableEntry[CM_NUM_VERTEX_TABLE_ENTRIES_G75];
} VEBOX_VERTEX_TABLE_G75, *PVEBOX_VERTEX_TABLE_G75;



typedef struct _VEBOX_DNDI_STATE_G8
{
    // DWORD 0
    union
    {
        struct
        {
            DWORD       DenoiseASDThreshold : 8; // U8
            DWORD       DnmhDelta : 4; // UINT4
        DWORD: 4; // Reserved
            DWORD       DnmhHistoryMax : 8; // U8
            DWORD       DenoiseSTADThreshold : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            DWORD       SCMDenoiseThreshold : 8; // U8
            DWORD       DenoiseMovingPixelThreshold : 5; // U5
            DWORD       STMMC2 : 3; // U3
            DWORD       LowTemporalDifferenceThreshold : 6; // U6
        DWORD: 2; // Reserved
            DWORD       TemporalDifferenceThreshold : 6; // U6
        DWORD: 2; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW1;

    // DWORD 2
    union
    {
        struct
        {
            DWORD       BlockNoiseEstimateNoiseThreshold : 8; // U8
            DWORD       BneEdgeTh : 4; // UINT4
        DWORD: 2; // Reserved
            DWORD       SmoothMvTh : 2; // U2
            DWORD       SADTightTh : 4; // U4
            DWORD       CATSlopeMinus1 : 4; // U4
            DWORD       GoodNeighborThreshold : 6; // UINT6
        DWORD: 2; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW2;

    // DWORD 3
    union
    {
        struct
        {
            DWORD       MaximumSTMM : 8; // U8
            DWORD       MultiplierforVECM : 6; // U6
        DWORD: 2;
            DWORD       BlendingConstantForSmallSTMM : 8; // U8
            DWORD       BlendingConstantForLargeSTMM : 7; // U7
            DWORD       STMMBlendingConstantSelect : 1; // U1
        };
        struct
        {
            DWORD       Value;
        };
    } DW3;

    // DWORD 4
    union
    {
        struct
        {
            DWORD       SDIDelta : 8; // U8
            DWORD       SDIThreshold : 8; // U8
            DWORD       STMMOutputShift : 4; // U4
            DWORD       STMMShiftUp : 2; // U2
            DWORD       STMMShiftDown : 2; // U2
            DWORD       MinimumSTMM : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW4;

    // DWORD 5
    union
    {
        struct
        {
            DWORD       FMDTemporalDifferenceThreshold : 8; // U8
            DWORD       SDIFallbackMode2Constant : 8; // U8
            DWORD       SDIFallbackMode1T2Constant : 8; // U8
            DWORD       SDIFallbackMode1T1Constant : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW5;

    // DWORD 6
    union
    {
        struct
        {
        DWORD: 3; // Reserved
            DWORD       DNDITopFirst : 1; // Enable
        DWORD: 2; // Reserved
            DWORD       ProgressiveDN : 1; // Enable
            DWORD       MCDIEnable : 1;
            DWORD       FMDTearThreshold : 6; // U6
            DWORD       CATTh1 : 2; // U2
            DWORD       FMD2VerticalDifferenceThreshold : 8; // U8
            DWORD       FMD1VerticalDifferenceThreshold : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW6;

    // DWORD 7
    union
    {
        struct
        {
            DWORD       SADTHA : 4; // U4
            DWORD       SADTHB : 4; // U4
            DWORD       FMDFirstFieldCurrentFrame : 2; // U2
            DWORD       MCPixelConsistencyTh : 6; // U6
            DWORD       FMDSecondFieldPreviousFrame : 2; // U2
        DWORD: 1; // Reserved
            DWORD       NeighborPixelTh : 4; // U4
            DWORD       DnmhHistoryInit : 6; // U6
        DWORD: 3; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW7;

    // DWORD 8
    union
    {
        struct
        {
            DWORD       ChromaLTDThreshold : 6; // U6
            DWORD       ChromaTDTheshold : 6; // U6
            DWORD       ChromaDenoiseEnable : 1; // Enable
        DWORD: 3; // Reserved
            DWORD       ChromaDnSTADThreshold : 8; // U8
        DWORD: 8; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW8;

    // DWORD 9
    union
    {
        struct
        {
            DWORD       HotPixelThreshold : 8;
            DWORD       HotPixelCount : 4;
        DWORD: 20; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW9;

    // Padding for 32-byte alignment, VEBOX_DNDI_STATE_G8 is 10 DWORDs
    DWORD dwPad[6];
} VEBOX_DNDI_STATE_G8, *PVEBOX_DNDI_STATE_G8;

// Defined in vol2b "Media"
typedef struct _VEBOX_IECP_STATE_G8
{
    // STD/STE state
    // DWORD 0
    union
    {
        struct
        {
            DWORD       STDEnable : BITFIELD_BIT(0);
            DWORD       STEEnable : BITFIELD_BIT(1);
            DWORD       OutputCtrl : BITFIELD_BIT(2);
        DWORD: BITFIELD_BIT(3);
            DWORD       SatMax : BITFIELD_RANGE(4, 9);      // U6;
            DWORD       HueMax : BITFIELD_RANGE(10, 15);    // U6;
            DWORD       UMid : BITFIELD_RANGE(16, 23);    // U8;
            DWORD       VMid : BITFIELD_RANGE(24, 31);    // U8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            DWORD       SinAlpha : BITFIELD_RANGE(0, 7);    // S0.7
        DWORD: BITFIELD_RANGE(8, 9);
            DWORD       CosAlpha : BITFIELD_RANGE(10, 17);  // S0.7
            DWORD       HSMargin : BITFIELD_RANGE(18, 20);  // U3   
            DWORD       DiamondDu : BITFIELD_RANGE(21, 27);  // S7    
            DWORD       DiamondMargin : BITFIELD_RANGE(28, 30);  // U3    
        DWORD: BITFIELD_BIT(31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW1;

    // DWORD 2
    union
    {
        struct
        {
            DWORD       DiamondDv : BITFIELD_RANGE(0, 6);    // S8.0
            DWORD       DiamondTh : BITFIELD_RANGE(7, 12);   // U6  
            DWORD       DiamondAlpha : BITFIELD_RANGE(13, 20);  // U1.6
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW2;

    // DWORD 3
    union
    {
        struct
        {
        DWORD: BITFIELD_RANGE(0, 6);
            DWORD       VYSTDEnable : BITFIELD_BIT(7);
            DWORD       YPoint1 : BITFIELD_RANGE(8, 15);    // U8   
            DWORD       YPoint2 : BITFIELD_RANGE(16, 23);   // U8   
            DWORD       YPoint3 : BITFIELD_RANGE(24, 31);   // U8   
        };
        struct
        {
            DWORD       Value;
        };
    } DW3;

    // DWORD 4
    union
    {
        struct
        {
            DWORD       YPoint4 : BITFIELD_RANGE(0, 7);    // U8
            DWORD       YSlope1 : BITFIELD_RANGE(8, 12);   // U2.3
            DWORD       YSlope2 : BITFIELD_RANGE(13, 17);  // U2.3
        DWORD: BITFIELD_RANGE(18, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW4;

    // DWORD 5
    union
    {
        struct
        {
            DWORD       INVMarginVYL : BITFIELD_RANGE(0, 15);    // U0.16
            DWORD       INVSkinTypesMargin : BITFIELD_RANGE(16, 31);   // U0.16
        };
        struct
        {
            DWORD       Value;
        };
    } DW5;

    // DWORD 6
    union
    {
        struct
        {
            DWORD       INVMarginVYU : BITFIELD_RANGE(0, 15);    // U0.16
            DWORD       P0L : BITFIELD_RANGE(16, 23);   // U8
            DWORD       P1L : BITFIELD_RANGE(24, 31);   // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW6;

    // DWORD 7
    union
    {
        struct
        {
            DWORD       P2L : BITFIELD_RANGE(0, 7);     // U8
            DWORD       P3L : BITFIELD_RANGE(8, 15);    // U8
            DWORD       B0L : BITFIELD_RANGE(16, 23);   // U8
            DWORD       B1L : BITFIELD_RANGE(24, 31);   // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW7;

    // DWORD 8
    union
    {
        struct
        {
            DWORD       B2L : BITFIELD_RANGE(0, 7);     // U8
            DWORD       B3L : BITFIELD_RANGE(8, 15);    // U8
            DWORD       S0L : BITFIELD_RANGE(16, 26);   // S2.8
        DWORD: BITFIELD_RANGE(27, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW8;

    // DWORD 9
    union
    {
        struct
        {
            DWORD       S1L : BITFIELD_RANGE(0, 10);    // S2.8
            DWORD       S2L : BITFIELD_RANGE(11, 21);   // S2.8
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW9;

    // DWORD 10
    union
    {
        struct
        {
            DWORD       S3L : BITFIELD_RANGE(0, 10);    // S2.8
            DWORD       P0U : BITFIELD_RANGE(11, 18);   // U8
            DWORD       P1U : BITFIELD_RANGE(19, 26);   // U8
        DWORD: BITFIELD_RANGE(27, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW10;

    // DWORD 11
    union
    {
        struct
        {
            DWORD       P2U : BITFIELD_RANGE(0, 7);     // U8
            DWORD       P3U : BITFIELD_RANGE(8, 15);    // U8
            DWORD       B0U : BITFIELD_RANGE(16, 23);   // U8
            DWORD       B1U : BITFIELD_RANGE(24, 31);   // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW11;

    // DWORD 12
    union
    {
        struct
        {
            DWORD       B2U : BITFIELD_RANGE(0, 7);     // U8
            DWORD       B3U : BITFIELD_RANGE(8, 15);    // U8
            DWORD       S0U : BITFIELD_RANGE(16, 26);   // S2.8
        DWORD: BITFIELD_RANGE(27, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW12;

    // DWORD 13
    union
    {
        struct
        {
            DWORD       S1U : BITFIELD_RANGE(0, 10);     // S2.8
            DWORD       S2U : BITFIELD_RANGE(11, 21);    // S2.8
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW13;

    // DWORD 14
    union
    {
        struct
        {
            DWORD       S3U : BITFIELD_RANGE(0, 10);     // S2.8
            DWORD       SkinTypesEnable : BITFIELD_BIT(11);
            DWORD       SkinTypesThresh : BITFIELD_RANGE(12, 19);    // U8
            DWORD       SkinTypesMargin : BITFIELD_RANGE(20, 27);    // U8
        DWORD: BITFIELD_RANGE(28, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW14;

    // DWORD 15
    union
    {
        struct
        {
            DWORD       SATP1 : BITFIELD_RANGE(0, 6);     // S6
            DWORD       SATP2 : BITFIELD_RANGE(7, 13);    // S6
            DWORD       SATP3 : BITFIELD_RANGE(14, 20);   // S6
            DWORD       SATB1 : BITFIELD_RANGE(21, 30);   // S2.7
        DWORD: BITFIELD_BIT(31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW15;

    // DWORD 16
    union
    {
        struct
        {
            DWORD       SATB2 : BITFIELD_RANGE(0, 9);     // S2.7
            DWORD       SATB3 : BITFIELD_RANGE(10, 19);   // S2.7
            DWORD       SATS0 : BITFIELD_RANGE(20, 30);   // U3.8
        DWORD: BITFIELD_BIT(31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW16;

    // DWORD 17
    union
    {
        struct
        {
            DWORD       SATS1 : BITFIELD_RANGE(0, 10);     // U3.8
            DWORD       SATS2 : BITFIELD_RANGE(11, 21);    // U3.8
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW17;

    // DWORD 18
    union
    {
        struct
        {
            DWORD       SATS3 : BITFIELD_RANGE(0, 10);    // U3.8
            DWORD       HUEP1 : BITFIELD_RANGE(11, 17);   // U3.8
            DWORD       HUEP2 : BITFIELD_RANGE(18, 24);   // U3.8
            DWORD       HUEP3 : BITFIELD_RANGE(25, 31);   // U3.8
        };
        struct
        {
            DWORD       Value;
        };
    } DW18;

    // DWORD 19
    union
    {
        struct
        {
            DWORD       HUEB1 : BITFIELD_RANGE(0, 9);    // S2.7
            DWORD       HUEB2 : BITFIELD_RANGE(10, 19);  // S2.7
            DWORD       HUEB3 : BITFIELD_RANGE(20, 29);  // S2.7
        DWORD: BITFIELD_RANGE(30, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW19;

    // DWORD 20
    union
    {
        struct
        {
            DWORD       HUES0 : BITFIELD_RANGE(0, 10);    // U3.8
            DWORD       HUES1 : BITFIELD_RANGE(11, 21);   // U3.8
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW20;

    // DWORD 21
    union
    {
        struct
        {
            DWORD       HUES2 : BITFIELD_RANGE(0, 10);    // U3.8
            DWORD       HUES3 : BITFIELD_RANGE(11, 21);   // U3.8
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW21;

    // DWORD 22
    union
    {
        struct
        {
            DWORD       SATP1DARK : BITFIELD_RANGE(0, 6);     // S6
            DWORD       SATP2DARK : BITFIELD_RANGE(7, 13);    // S6
            DWORD       SATP3DARK : BITFIELD_RANGE(14, 20);   // S6
            DWORD       SATB1DARK : BITFIELD_RANGE(21, 30);   // S2.7
        DWORD: BITFIELD_BIT(31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW22;

    // DWORD 23
    union
    {
        struct
        {
            DWORD       SATB2DARK : BITFIELD_RANGE(0, 9);    // S2.7
            DWORD       SATB3DARK : BITFIELD_RANGE(10, 19);  // S2.7
            DWORD       SATS0DARK : BITFIELD_RANGE(20, 30);  // U3.8
        DWORD: BITFIELD_BIT(31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW23;

    // DWORD 24
    union
    {
        struct
        {
            DWORD       SATS1DARK : BITFIELD_RANGE(0, 10);    // U3.8
            DWORD       SATS2DARK : BITFIELD_RANGE(11, 21);   // U3.8
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW24;

    // DWORD 25
    union
    {
        struct
        {
            DWORD       SATS3DARK : BITFIELD_RANGE(0, 10);   // U3.8
            DWORD       HUEP1DARK : BITFIELD_RANGE(11, 17);  // S6
            DWORD       HUEP2DARK : BITFIELD_RANGE(18, 24);  // S6
            DWORD       HUEP3DARK : BITFIELD_RANGE(25, 31);  // S6
        };
        struct
        {
            DWORD       Value;
        };
    } DW25;

    // DWORD 26
    union
    {
        struct
        {
            DWORD       HUEB1DARK : BITFIELD_RANGE(0, 9);    // S2.7
            DWORD       HUEB2DARK : BITFIELD_RANGE(10, 19);  // S2.7
            DWORD       HUEB3DARK : BITFIELD_RANGE(20, 29);  // S2.7
        DWORD: BITFIELD_RANGE(30, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW26;

    // DWORD 27
    union
    {
        struct
        {
            DWORD       HUES0DARK : BITFIELD_RANGE(0, 10);    // U3.8
            DWORD       HUES1DARK : BITFIELD_RANGE(11, 21);   // U3.8
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW27;

    // DWORD 28
    union
    {
        struct
        {
            DWORD       HUES2DARK : BITFIELD_RANGE(0, 10);    // U3.8
            DWORD       HUES3DARK : BITFIELD_RANGE(11, 21);   // U3.8
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW28;

    // ACE state
    // DWORD 29
    union
    {
        struct
        {
            DWORD       ACEEnable : BITFIELD_BIT(0);
            DWORD       FullImageHistogram : BITFIELD_BIT(1);
            DWORD       SkinThreshold : BITFIELD_RANGE(2, 6);    // U5
        DWORD: BITFIELD_RANGE(7, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW29;

    // DWORD 30
    union
    {
        struct
        {
            DWORD       Ymin : BITFIELD_RANGE(0, 7);
            DWORD       Y1 : BITFIELD_RANGE(8, 15);
            DWORD       Y2 : BITFIELD_RANGE(16, 23);
            DWORD       Y3 : BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW30;

    // DWORD 31
    union
    {
        struct
        {
            DWORD       Y4 : BITFIELD_RANGE(0, 7);
            DWORD       Y5 : BITFIELD_RANGE(8, 15);
            DWORD       Y6 : BITFIELD_RANGE(16, 23);
            DWORD       Y7 : BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW31;

    // DWORD 32
    union
    {
        struct
        {
            DWORD       Y8 : BITFIELD_RANGE(0, 7);
            DWORD       Y9 : BITFIELD_RANGE(8, 15);
            DWORD       Y10 : BITFIELD_RANGE(16, 23);
            DWORD       Ymax : BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW32;

    // DWORD 33
    union
    {
        struct
        {
            DWORD       B1 : BITFIELD_RANGE(0, 7);   // U8
            DWORD       B2 : BITFIELD_RANGE(8, 15);  // U8
            DWORD       B3 : BITFIELD_RANGE(16, 23); // U8
            DWORD       B4 : BITFIELD_RANGE(24, 31); // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW33;

    // DWORD 34
    union
    {
        struct
        {
            DWORD       B5 : BITFIELD_RANGE(0, 7);   // U8
            DWORD       B6 : BITFIELD_RANGE(8, 15);  // U8
            DWORD       B7 : BITFIELD_RANGE(16, 23); // U8
            DWORD       B8 : BITFIELD_RANGE(24, 31); // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW34;

    // DWORD 35
    union
    {
        struct
        {
            DWORD       B9 : BITFIELD_RANGE(0, 7);   // U8
            DWORD       B10 : BITFIELD_RANGE(8, 15);  // U8
        DWORD: BITFIELD_RANGE(16, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW35;

    // DWORD 36
    union
    {
        struct
        {
            DWORD       S0 : BITFIELD_RANGE(0, 10);   // U11
        DWORD: BITFIELD_RANGE(11, 15);
            DWORD       S1 : BITFIELD_RANGE(16, 26);  // U11
        DWORD: BITFIELD_RANGE(27, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW36;

    // DWORD 37
    union
    {
        struct
        {
            DWORD       S2 : BITFIELD_RANGE(0, 10);   // U11
        DWORD: BITFIELD_RANGE(11, 15);
            DWORD       S3 : BITFIELD_RANGE(16, 26);  // U11
        DWORD: BITFIELD_RANGE(27, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW37;

    // DWORD 38
    union
    {
        struct
        {
            DWORD       S4 : BITFIELD_RANGE(0, 10);   // U11
        DWORD: BITFIELD_RANGE(11, 15);
            DWORD       S5 : BITFIELD_RANGE(16, 26);  // U11
        DWORD: BITFIELD_RANGE(27, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW38;

    // DWORD 39
    union
    {
        struct
        {
            DWORD       S6 : BITFIELD_RANGE(0, 10);   // U11
        DWORD: BITFIELD_RANGE(11, 15);
            DWORD       S7 : BITFIELD_RANGE(16, 26);  // U11
        DWORD: BITFIELD_RANGE(27, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW39;

    // DWORD 40
    union
    {
        struct
        {
            DWORD       S8 : BITFIELD_RANGE(0, 10);   // U11
        DWORD: BITFIELD_RANGE(11, 15);
            DWORD       S9 : BITFIELD_RANGE(16, 26);  // U11
        DWORD: BITFIELD_RANGE(27, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW40;

    // DWORD 41
    union
    {
        struct
        {
            DWORD       S10 : BITFIELD_RANGE(0, 10);   // U11
        DWORD: BITFIELD_RANGE(11, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW41;

    // TCC State
    // DWORD 42
    union
    {
        struct
        {
        DWORD: BITFIELD_RANGE(0, 6);
            DWORD       TCCEnable : BITFIELD_BIT(7);
            DWORD       SatFactor1 : BITFIELD_RANGE(8, 15);    // U8 
            DWORD       SatFactor2 : BITFIELD_RANGE(16, 23);   // U8 
            DWORD       SatFactor3 : BITFIELD_RANGE(24, 31);   // U8 
        };
        struct
        {
            DWORD       Value;
        };
    } DW42;

    // DWORD 43
    union
    {
        struct
        {
        DWORD: BITFIELD_RANGE(0, 7);
            DWORD       SatFactor4 : BITFIELD_RANGE(8, 15);    // U8 
            DWORD       SatFactor5 : BITFIELD_RANGE(16, 23);   // U8 
            DWORD       SatFactor6 : BITFIELD_RANGE(24, 31);   // U8 
        };
        struct
        {
            DWORD       Value;
        };
    } DW43;

    // DWORD 44
    union
    {
        struct
        {
            DWORD       BaseColor1 : BITFIELD_RANGE(0, 9);    // U10 
            DWORD       BaseColor2 : BITFIELD_RANGE(10, 19);  // U10 
            DWORD       BaseColor3 : BITFIELD_RANGE(20, 29);  // U10 
        DWORD: BITFIELD_RANGE(30, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW44;

    // DWORD 45
    union
    {
        struct
        {
            DWORD       BaseColor4 : BITFIELD_RANGE(0, 9);    // U10 
            DWORD       BaseColor5 : BITFIELD_RANGE(10, 19);  // U10 
            DWORD       BaseColor6 : BITFIELD_RANGE(20, 29);  // U10 
        DWORD: BITFIELD_RANGE(30, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW45;

    // DWORD 46
    union
    {
        struct
        {
            DWORD       ColorTransitSlope12 : BITFIELD_RANGE(0, 15);    // U16
            DWORD       ColorTransitSlope23 : BITFIELD_RANGE(16, 31);   // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW46;

    // DWORD 47
    union
    {
        struct
        {
            DWORD       ColorTransitSlope34 : BITFIELD_RANGE(0, 15);    // U16
            DWORD       ColorTransitSlope45 : BITFIELD_RANGE(16, 31);   // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW47;

    // DWORD 48
    union
    {
        struct
        {
            DWORD       ColorTransitSlope56 : BITFIELD_RANGE(0, 15);    // U16
            DWORD       ColorTransitSlope61 : BITFIELD_RANGE(16, 31);   // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW48;

    // DWORD 49
    union
    {
        struct
        {
        DWORD: BITFIELD_RANGE(0, 1);
            DWORD       ColorBias1 : BITFIELD_RANGE(2, 11);    // U10
            DWORD       ColorBias2 : BITFIELD_RANGE(12, 21);   // U10
            DWORD       ColorBias3 : BITFIELD_RANGE(22, 31);   // U10
        };
        struct
        {
            DWORD       Value;
        };
    } DW49;

    // DWORD 50
    union
    {
        struct
        {
        DWORD: BITFIELD_RANGE(0, 1);
            DWORD       ColorBias4 : BITFIELD_RANGE(2, 11);    // U10
            DWORD       ColorBias5 : BITFIELD_RANGE(12, 21);   // U10
            DWORD       ColorBias6 : BITFIELD_RANGE(22, 31);   // U10
        };
        struct
        {
            DWORD       Value;
        };
    } DW50;

    // DWORD 51
    union
    {
        struct
        {
            DWORD       STESlopeBits : BITFIELD_RANGE(0, 2);    // U3
        DWORD: BITFIELD_RANGE(3, 7);
            DWORD       STEThreshold : BITFIELD_RANGE(8, 12);   // U5
        DWORD: BITFIELD_RANGE(13, 15);
            DWORD       UVThresholdBits : BITFIELD_RANGE(16, 18);  // U5
        DWORD: BITFIELD_RANGE(19, 23);
            DWORD       UVThreshold : BITFIELD_RANGE(24, 30);  // U7
        DWORD: BITFIELD_BIT(31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW51;

    // DWORD 52
    union
    {
        struct
        {
            DWORD       UVMaxColor : BITFIELD_RANGE(0, 8);    // U9
        DWORD: BITFIELD_RANGE(9, 15);
            DWORD       InvUVMaxColor : BITFIELD_RANGE(16, 31);  // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW52;

    // ProcAmp State
    // DWORD 53
    union
    {
        struct
        {
            DWORD       ProcAmpEnable : BITFIELD_BIT(0);
            DWORD       Brightness : BITFIELD_RANGE(1, 12);    // S7.4
        DWORD: BITFIELD_RANGE(13, 16);
            DWORD       Contrast : BITFIELD_RANGE(17, 27);   // U7.4
        DWORD: BITFIELD_RANGE(28, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW53;

    // DWORD 54
    union
    {
        struct
        {
            DWORD       SINCS : BITFIELD_RANGE(0, 15);    // S7.8    
            DWORD       COSCS : BITFIELD_RANGE(16, 31);   // S7.8
        };
        struct
        {
            DWORD       Value;
        };
    } DW54;

    // CSC State
    // DWORD 55
    union
    {
        struct
        {
            DWORD       TransformEnable : BITFIELD_BIT(0);
            DWORD       YUVChannelSwap : BITFIELD_BIT(1);
        DWORD: BITFIELD_BIT(2);
            DWORD       C0 : BITFIELD_RANGE(3, 15);  // S2.10
            DWORD       C1 : BITFIELD_RANGE(16, 28); // S2.10
        DWORD: BITFIELD_RANGE(29, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW55;

    // DWORD 56
    union
    {
        struct
        {
            DWORD       C2 : BITFIELD_RANGE(0, 12);  // S2.10
            DWORD       C3 : BITFIELD_RANGE(13, 25); // S2.10
        DWORD: BITFIELD_RANGE(26, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW56;

    // DWORD 57
    union
    {
        struct
        {
            DWORD       C4 : BITFIELD_RANGE(0, 12);  // S2.10
            DWORD       C5 : BITFIELD_RANGE(13, 25); // S2.10
        DWORD: BITFIELD_RANGE(26, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW57;

    // DWORD 58
    union
    {
        struct
        {
            DWORD       C6 : BITFIELD_RANGE(0, 12);  // S2.10
            DWORD       C7 : BITFIELD_RANGE(13, 25); // S2.10
        DWORD: BITFIELD_RANGE(26, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW58;

    // DWORD 59
    union
    {
        struct
        {
            DWORD       C8 : BITFIELD_RANGE(0, 12);   // S2.10
        DWORD: BITFIELD_RANGE(13, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW59;

    // DWORD 60
    union
    {
        struct
        {
            DWORD       OffsetIn1 : BITFIELD_RANGE(0, 10);   // S10
            DWORD       OffsetOut1 : BITFIELD_RANGE(11, 21);  // S10
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW60;

    // DWORD 61
    union
    {
        struct
        {
            DWORD       OffsetIn2 : BITFIELD_RANGE(0, 10);   // S10
            DWORD       OffsetOut2 : BITFIELD_RANGE(11, 21);  // S10
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW61;

    // DWORD 62
    union
    {
        struct
        {
            DWORD       OffsetIn3 : BITFIELD_RANGE(0, 10);  // S10
            DWORD       OffsetOut3 : BITFIELD_RANGE(11, 21); // S10
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW62;

    // DWORD 63
    union
    {
        struct
        {
            DWORD       ColorPipeAlpha : BITFIELD_RANGE(0, 11);  // U12
        DWORD: BITFIELD_RANGE(12, 15);
            DWORD       AlphaFromStateSelect : BITFIELD_BIT(16);
        DWORD: BITFIELD_RANGE(17, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW63;

    // Area of Interest
    // DWORD 64
    union
    {
        struct
        {
            DWORD       AOIMinX : BITFIELD_RANGE(0, 15);  // U16
            DWORD       AOIMaxX : BITFIELD_RANGE(16, 31); // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW64;

    // DWORD 65
    union
    {
        struct
        {
            DWORD       AOIMinY : BITFIELD_RANGE(0, 15);  // U16
            DWORD       AOIMaxY : BITFIELD_RANGE(16, 31); // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW65;

    // Color Correction Matrix
    // DWORD 66
    union
    {
        struct
        {
            DWORD       C1Coeff : BITFIELD_RANGE(0, 20);  // S8.12
        DWORD: BITFIELD_RANGE(21, 29);
            DWORD       VignetteCorrectionFormat : BITFIELD_BIT(30);
            DWORD       ColorCorrectionMatrixEnable : BITFIELD_BIT(31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW66;

    // DWORD 67
    union
    {
        struct
        {
            DWORD       C0Coeff : BITFIELD_RANGE(0, 20); // S8.12
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW67;

    // DWORD 68
    union
    {
        struct
        {
            DWORD       C3Coeff : BITFIELD_RANGE(0, 20); // S8.12
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW68;

    // DWORD 69
    union
    {
        struct
        {
            DWORD       C2Coeff : BITFIELD_RANGE(0, 20); // S8.12
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW69;

    // DWORD 70
    union
    {
        struct
        {
            DWORD       C5Coeff : BITFIELD_RANGE(0, 20); // S8.12
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW70;

    // DWORD 71
    union
    {
        struct
        {
            DWORD       C4Coeff : BITFIELD_RANGE(0, 20); // S8.12
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW71;

    // DWORD 72
    union
    {
        struct
        {
            DWORD       C7Coeff : BITFIELD_RANGE(0, 20); // S8.12
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW72;

    // DWORD 73
    union
    {
        struct
        {
            DWORD       C6Coeff : BITFIELD_RANGE(0, 20); // S8.12
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW73;

    // DWORD 74
    union
    {
        struct
        {
            DWORD       C8Coeff : BITFIELD_RANGE(0, 20); // S8.12
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW74;

    // DWORD 75
    union
    {
        struct
        {
            DWORD       BlackPointOffsetR : BITFIELD_RANGE(0, 12); // S12
        DWORD: BITFIELD_RANGE(13, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW75;

    // DWORD 76
    union
    {
        struct
        {
            DWORD       BlackPointOffsetB : BITFIELD_RANGE(0, 12);  // S12
            DWORD       BlackPointOffsetG : BITFIELD_RANGE(13, 25); // S12
        DWORD: BITFIELD_RANGE(26, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW76;

    // Forward Gamma Correction
    // DWORD 77
    union
    {
        struct
        {
            DWORD       ForwardGammaCorrectionEnable : BITFIELD_BIT(0);
        DWORD: BITFIELD_RANGE(1, 7);
            DWORD       PWLFwdGammaPoint1 : BITFIELD_RANGE(8, 15);
            DWORD       PWLFwdGammaPoint2 : BITFIELD_RANGE(16, 23);
            DWORD       PWLFwdGammaPoint3 : BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW77;

    // DWORD 78
    union
    {
        struct
        {
            DWORD       PWLFwdGammaPoint4 : BITFIELD_RANGE(0, 7);
            DWORD       PWLFwdGammaPoint5 : BITFIELD_RANGE(8, 15);
            DWORD       PWLFwdGammaPoint6 : BITFIELD_RANGE(16, 23);
            DWORD       PWLFwdGammaPoint7 : BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW78;

    // DWORD 79
    union
    {
        struct
        {
            DWORD       PWLFwdGammaPoint8 : BITFIELD_RANGE(0, 7);
            DWORD       PWLFwdGammaPoint9 : BITFIELD_RANGE(8, 15);
            DWORD       PWLFwdGammaPoint10 : BITFIELD_RANGE(16, 23);
            DWORD       PWLFwdGammaPoint11 : BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW79;

    // DWORD 80
    union
    {
        struct
        {
            DWORD       PWLFwdGammaBias1 : BITFIELD_RANGE(0, 7);
            DWORD       PWLFwdGammaBias2 : BITFIELD_RANGE(8, 15);
            DWORD       PWLFwdGammaBias3 : BITFIELD_RANGE(16, 23);
            DWORD       PWLFwdGammaBias4 : BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW80;

    // DWORD 81
    union
    {
        struct
        {
            DWORD       PWLFwdGammaBias5 : BITFIELD_RANGE(0, 7);
            DWORD       PWLFwdGammaBias6 : BITFIELD_RANGE(8, 15);
            DWORD       PWLFwdGammaBias7 : BITFIELD_RANGE(16, 23);
            DWORD       PWLFwdGammaBias8 : BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW81;

    // DWORD 82
    union
    {
        struct
        {
            DWORD       PWLFwdGammaBias9 : BITFIELD_RANGE(0, 7);
            DWORD       PWLFwdGammaBias10 : BITFIELD_RANGE(8, 15);
            DWORD       PWLFwdGammaBias11 : BITFIELD_RANGE(16, 23);
        DWORD: BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW82;

    // DWORD 83
    union
    {
        struct
        {
            DWORD       PWLFwdGammaSlope0 : BITFIELD_RANGE(0, 11);  // U4.8
        DWORD: BITFIELD_RANGE(12, 15);
            DWORD       PWLFwdGammaSlope1 : BITFIELD_RANGE(16, 27); // U4.8
        DWORD: BITFIELD_RANGE(28, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW83;

    // DWORD 84
    union
    {
        struct
        {
            DWORD       PWLFwdGammaSlope2 : BITFIELD_RANGE(0, 11);  // U4.8
        DWORD: BITFIELD_RANGE(12, 15);
            DWORD       PWLFwdGammaSlope3 : BITFIELD_RANGE(16, 27); // U4.8
        DWORD: BITFIELD_RANGE(28, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW84;

    // DWORD 85
    union
    {
        struct
        {
            DWORD       PWLFwdGammaSlope4 : BITFIELD_RANGE(0, 11);  // U4.8
        DWORD: BITFIELD_RANGE(12, 15);
            DWORD       PWLFwdGammaSlope5 : BITFIELD_RANGE(16, 27); // U4.8
        DWORD: BITFIELD_RANGE(28, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW85;

    // DWORD 86
    union
    {
        struct
        {
            DWORD       PWLFwdGammaSlope6 : BITFIELD_RANGE(0, 11);  // U4.8
        DWORD: BITFIELD_RANGE(12, 15);
            DWORD       PWLFwdGammaSlope7 : BITFIELD_RANGE(16, 27); // U4.8
        DWORD: BITFIELD_RANGE(28, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW86;

    // DWORD 87
    union
    {
        struct
        {
            DWORD       PWLFwdGammaSlope8 : BITFIELD_RANGE(0, 11);  // U4.8
        DWORD: BITFIELD_RANGE(12, 15);
            DWORD       PWLFwdGammaSlope9 : BITFIELD_RANGE(16, 27); // U4.8
        DWORD: BITFIELD_RANGE(28, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW87;

    // DWORD 88
    union
    {
        struct
        {
            DWORD       PWLFwdGammaSlope10 : BITFIELD_RANGE(0, 11);  // U4.8
        DWORD: BITFIELD_RANGE(12, 15);
            DWORD       PWLFwdGammaSlope11 : BITFIELD_RANGE(16, 27); // U4.8
        DWORD: BITFIELD_RANGE(28, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW88;

    // Front-End CSC
    // DWORD 89
    union
    {
        struct
        {
            DWORD       FrontEndCSCTransformEnable : BITFIELD_BIT(0);
        DWORD: BITFIELD_RANGE(1, 2);
            DWORD       FECSCC0Coeff : BITFIELD_RANGE(3, 15);  // S2.10
            DWORD       FECSCC1Coeff : BITFIELD_RANGE(16, 28); // S2.10
        DWORD: BITFIELD_RANGE(29, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW89;

    // DWORD 90
    union
    {
        struct
        {
            DWORD       FECSCC2Coeff : BITFIELD_RANGE(0, 12);  // S2.10
            DWORD       FECSCC3Coeff : BITFIELD_RANGE(13, 25); // S2.10
        DWORD: BITFIELD_RANGE(26, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW90;

    // DWORD 91
    union
    {
        struct
        {
            DWORD       FECSCC4Coeff : BITFIELD_RANGE(0, 12);  // S2.10
            DWORD       FECSCC5Coeff : BITFIELD_RANGE(13, 25); // S2.10
        DWORD: BITFIELD_RANGE(26, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW91;

    // DWORD 92
    union
    {
        struct
        {
            DWORD       FECSCC6Coeff : BITFIELD_RANGE(0, 12);  // S2.10
            DWORD       FECSCC7Coeff : BITFIELD_RANGE(13, 25); // S2.10
        DWORD: BITFIELD_RANGE(26, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW92;

    // DWORD 93
    union
    {
        struct
        {
            DWORD       FECSCC8Coeff : BITFIELD_RANGE(0, 12);  // S2.10
        DWORD: BITFIELD_RANGE(13, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW93;

    // DWORD 94
    union
    {
        struct
        {
            DWORD       FECSCCOffsetIn1 : BITFIELD_RANGE(0, 10);
            DWORD       FECSCCOffsetOut1 : BITFIELD_RANGE(11, 21);
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW94;

    // DWORD 95
    union
    {
        struct
        {
            DWORD       FECSCCOffsetIn2 : BITFIELD_RANGE(0, 10);
            DWORD       FECSCCOffsetOut2 : BITFIELD_RANGE(11, 21);
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW95;

    // DWORD 96
    union
    {
        struct
        {
            DWORD       FECSCCOffsetIn3 : BITFIELD_RANGE(0, 10);
            DWORD       FECSCCOffsetOut3 : BITFIELD_RANGE(11, 21);
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW96;

    // Padding for 32-byte alignment, VEBOX_IECP_STATE_G8 is 97 DWORDs
    DWORD dwPad[7];
} VEBOX_IECP_STATE_G8, *PVEBOX_IECP_STATE_G8;

typedef struct _VEBOX_CAPTURE_PIPE_STATE_G8
{
    // DWORD 0
    union
    {
        struct
        {
            DWORD       BadAvgMinCostTh : 8; // U8
            DWORD       THColorTh : 8; // U8
            DWORD       ScaleForAvgMinCost : 4; // U4
            DWORD       ShiftMinCost : 3; // U3
        DWORD: 1; // Reserved
            DWORD       GoodPixelTh : 6; // U6
        DWORD: 2; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            DWORD       BadTH3 : 4; // U4
        DWORD: 4; // Reserved
            DWORD       BadTH2 : 8; // U8
            DWORD       BadTH1 : 8; // U8
        DWORD: 4; // Reserved
            DWORD       ScaleForMinCost : 4; // U4
        };
        struct
        {
            DWORD       Value;
        };
    } DW1;

    // DWORD 2
    union
    {
        struct
        {
        DWORD: 8; // Reserved
            DWORD       UVThresholdValue : 8; // U8
            DWORD       YOutlierValue : 8; // U8
            DWORD       YBrightValue : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW2;

    // Padding for 32-byte alignment, VEBOX_CAPTURE_PIPE_STATE_G8 is 3 DWORDs
    DWORD dwPad[5];
} VEBOX_CAPTURE_PIPE_STATE_G8, *PVEBOX_CAPTURE_PIPE_STATE_G8;

typedef struct __CM_VEBOX_PARAM_G8
{
    PVEBOX_DNDI_STATE_G8          pDndiState;
    unsigned char                 padding1[4032];
    PVEBOX_IECP_STATE_G8          pIecpState;
    unsigned char                 padding2[3680];
    PVEBOX_GAMUT_STATE_G75        pGamutState;
    unsigned char                 padding3[3936];
    PVEBOX_VERTEX_TABLE_G75       pVertexTable;
    unsigned char                 padding4[2048];
    PVEBOX_CAPTURE_PIPE_STATE_G8 pCapturePipe;
}CM_VEBOX_PARAM_G8, PCM_VEBOX_PARAM_G8;

#endif // __CM_RT_G8_H__
