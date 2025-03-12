/*
* Copyright (c) 2010-2020, Intel Corporation
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
//! \file      vphal_render_common.h 
//! \brief         Unified VP HAL shared rendering definitions 
//!
//!
//! \file     vphal_render_common.h
//! \brief    The header file of common struct/macro definitions shared by low level renderers
//! \details  Common struct/macro for different renderers, e.g. DNDI or Comp
//!
#ifndef __VPHAL_RENDER_COMMON_H__
#define __VPHAL_RENDER_COMMON_H__

#include "vphal.h"
#include "renderhal_legacy.h"
#include "mhw_utilities.h"
#include "mos_os.h"
#include "hal_kerneldll.h"

//!
//! \brief  Max number of Media Threads
//!
#define VPHAL_USE_MEDIA_THREADS_MAX         0

//!
//! \brief  Similar definition from MHAL
//!
#define VPHAL_YTILE_H_ALIGNMENT  32
#define VPHAL_YTILE_W_ALIGNMENT  128
#define VPHAL_XTILE_H_ALIGNMENT  8
#define VPHAL_XTILE_W_ALIGNMENT  512

#define VPHAL_YTILE_H_SHIFTBITS  5
#define VPHAL_YTILE_W_SHIFTBITS  7
#define VPHAL_XTILE_H_SHITFBITS  3
#define VPHAL_XTILE_W_SHIFTBITS  9

#define VPHAL_MACROBLOCK_SIZE   16
#define VPHAL_PAGE_SIZE         0x1000

//!
//! \brief  STE Parameters
//!
#define VPHAL_STE_OPTIMAL           3  // for perfect HD-HQV Score

//!
//! \brief  used to map STE Factor to parameters(SATP1, SATS0, SATS1) for
//!         STE Factor values <= VPHAL_STE_OPTIMAL
//!
#define VPHAL_STE_LTEO(dwSteFactor, fSatp1, fSats0, fSats1) \
    fSatp1 = (-6) + ((3 - (float)dwSteFactor) * 2);         \
    fSats0 = (float)29 / (fSatp1 + 31) * 256;               \
    fSats1 = (float)4 / (6 - fSatp1) * 256;

//!
//! \brief  used to map STE Factor to parameters(SATP1, SATS0, SATS1) for
//!         STE Factor values > VPHAL_STE_OPTIMAL
//!
#define VPHAL_STE_GTO(dwSteFactor, fSatp1, fSats0, fSats1)  \
    fSatp1 = (-20) + ((9 - (float)dwSteFactor) * 2);        \
    fSats0 = (float)31 / (fSatp1 + 31) * 256;               \
    fSats1 = (float)31 / (31 - fSatp1) * 256;

//!
//! \brief  Alignment
//!
#define VPHAL_CURBE_BLOCK_ALIGN_G7      32

// Defined DP used FC kernel for computation.
struct MEDIA_DP_FC_STATIC_DATA
{
     // DWORD 0 - GRF R1.0
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC0               : 16;
            uint32_t       CscConstantC1               : 16;
        };

        uint32_t       Value;
    } DW0;

    // DWORD 1 - GRF R1.1
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC2               : 16;
            uint32_t       CscConstantC3               : 16;
        };

        uint32_t       Value;
    } DW1;

    // DWORD 2 - GRF R1.2
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC4               : 16;
            uint32_t       CscConstantC5               : 16;
        };

        uint32_t       Value;
    } DW2;

    // DWORD 3 - GRF R1.3
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC6               : 16;
            uint32_t       CscConstantC7               : 16;
        };

        uint32_t       Value;
    } DW3;

    // DWORD 4 - GRF R1.4
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC8               : 16;
            uint32_t       CscConstantC9               : 16;
        };

        uint32_t       Value;
    } DW4;

    // DWORD 5 - GRF R1.5
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC10              : 16;
            uint32_t       CscConstantC11              : 16;
        };

        uint32_t       Value;
    } DW5;

    // DWORD 6 - GRF R1.6
    union
    {
        // Dataport-based rotation
        struct
        {
            uint32_t       InputPictureWidth           : 16;
            uint32_t       InputPictureHeight          : 16;
        };

        uint32_t       Value;
    } DW6;

    // DWORD 7 - GRF R1.7 distWidth and distHeight
    union
    {
        struct
        {
            uint32_t       DestinationRectangleWidth   : 16;
            uint32_t       DestinationRectangleHeight  : 16;
        };

        uint32_t       Value;
    } DW7;

    // DWORD 8 - GRF R1.8
    union
    {
        struct
        {
            uint32_t       RotationChromaSitingFlag;
        };

        uint32_t       Value;
    } DW8;

    // DWORD 9 - GRF R1.9  input DeltaX
    union
    {
        struct
        {
            float       HorizontalScalingStepRatioLayer0;
        };

        uint32_t       Value;
    } DW9;

    // DWORD 10 - GRF R2.0  input DeltaY
    union
    {
        struct
        {
            float       VerticalScalingStepRatioLayer0;
        };

        uint32_t       Value;
    } DW10;

    // DWORD 11 - GRF R2.1  input orignin x
    union
    {
        struct
        {
            float       HorizontalFrameOriginLayer0;
        };

        uint32_t       Value;
    } DW11;

    // DWORD 12 - GRF R2.2  input orignin Y
    union
    {
        struct
        {
            float       VerticalFrameOriginLayer0;
        };

        uint32_t       Value;
    } DW12;

    // DWORD13  - GRF R2.3  topleft[2]
    union
    {
        struct
        {
            uint32_t       DestXTopLeftLayer0  : 16;
            uint32_t       DestYTopLeftLayer0  : 16;
        };

        uint32_t       Value;
    } DW13;

    // DWORD14  - GRF R2.4  bottomRight
    union
    {
        struct
        {
            uint32_t       DestXBottomRightLayer0  : 16;
            uint32_t       DestYBottomRightLayer0  : 16;
        };

        uint32_t       Value;
    } DW14;

    // DWORD15  - GRF R2.4  waflag and dstpitch
    union
    {
        struct
        {
            uint32_t       waFlag                 : 16;
            uint32_t       StatisticsSurfacePitch : 16;
        };

        uint32_t       Value;
    } DW15;

};
// Defined in Inline parameter computation
struct MEDIA_WALKER_KA2_STATIC_DATA
{
    // DWORD 0 - GRF R1.0
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC0               : 16;
            uint32_t       CscConstantC1               : 16;
        };

        // Chroma Denoise
        struct
        {
            uint32_t       LocalDifferenceThresholdU   : 8;
            uint32_t       LocalDifferenceThresholdV   : 8;
            uint32_t       SobelEdgeThresholdU         : 8;
            uint32_t       SobelEdgeThresholdV         : 8;
        };

        // Dataport-based rotation
        struct
        {
            uint32_t       InputPictureWidth           : 16;
            uint32_t       InputPictureHeight          : 16;
        };

        uint32_t       Value;
    } DW00;

    // DWORD 1 - GRF R1.1
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC2               : 16;
            uint32_t       CscConstantC3               : 16;
        };

        // Chroma Denoise
        struct
        {
            uint32_t       HistoryInitialValueU        : 8;
            uint32_t       HistoryInitialValueV        : 8;
            uint32_t       HistoryMaxU                 : 8;
            uint32_t       HistoryMaxV                 : 8;
        };

        // Dataport-based rotation
        struct
        {
            uint32_t       RotationMirrorMode          : 8;
            uint32_t       DestinationRGBFormat        : 8;
            uint32_t                                   : 16;
        };

        uint32_t       Value;
    } DW01;

    // DWORD 2 - GRF R1.2
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC4               : 16;
            uint32_t       CscConstantC5               : 16;
        };

        // Chroma Denoise
        struct
        {
            uint32_t       HistoryDeltaU               : 8;
            uint32_t       HistoryDeltaV               : 8;
            uint32_t       DNSADThresholdU             : 8;
            uint32_t       DNSADThresholdV             : 8;
        };

        // Dataport-based rotation
        struct
        {
            uint32_t       CscConstantC0               : 16;
            uint32_t       CscConstantC1               : 16;
        };

        uint32_t       Value;
    } DW02;

    // DWORD 3 - GRF R1.3
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC6               : 16;
            uint32_t       CscConstantC7               : 16;
        };

        // Chroma Denoise
        struct
        {
            uint32_t       DNTDThresholdU              : 8;
            uint32_t       DNTDThresholdV              : 8;
            uint32_t       DNLTDThresholdU             : 8;
            uint32_t       DNLTDThresholdV             : 8;
        };

        // Dataport-based rotation
        struct
        {
            uint32_t       CscConstantC2               : 16;
            uint32_t       CscConstantC3               : 16;
        };

        uint32_t       Value;
    } DW03;

    // DWORD 4 - GRF R1.4
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC8               : 16;
            uint32_t       CscConstantC9               : 16;
        };

        // Dataport-based rotation
        struct
        {
            uint32_t       CscConstantC4               : 16;
            uint32_t       CscConstantC5               : 16;
        };

        uint32_t       Value;
    } DW04;

    // DWORD 5 - GRF R1.5
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC10              : 16;
            uint32_t       CscConstantC11              : 16;
        };

        // Dataport-based rotation
        struct
        {
            uint32_t       CscConstantC6               : 16;
            uint32_t       CscConstantC7               : 16;
        };

        uint32_t       Value;
    } DW05;

    // DWORD 6 - GRF R1.6
    union
    {
        // Const Blending
        struct
        {
            uint32_t       ConstantBlendingAlphaLayer1 : 8;
            uint32_t       ConstantBlendingAlphaLayer2 : 8;
            uint32_t       ConstantBlendingAlphaLayer3 : 8;
            uint32_t       ConstantBlendingAlphaLayer4 : 8;
        };

        // DNDI
        struct
        {
            uint32_t       HalfStatisticsSurfacePitch : 16;  // Statistics surface pitch divided by 2
            uint32_t       StatisticsSurfaceHeight    : 16;  // Statistics surface height divided by 4
        };

        // Dataport-based rotation
        struct
        {
            uint32_t       CscConstantC8               : 16;
            uint32_t       CscConstantC9               : 16;
        };

        uint32_t       Value;
    } DW06;

    // DWORD 7 - GRF R1.7
    union
    {
        // Const Blending
        struct
        {
            uint32_t       ConstantBlendingAlphaLayer5 : 8;
            uint32_t       ConstantBlendingAlphaLayer6 : 8;
            uint32_t       ConstantBlendingAlphaLayer7 : 8;
            uint32_t       PointerToInlineParameters   : 8;
        };

        struct
        {
            uint32_t       ConstantBlendingAlphaLayer51 : 8;
            uint32_t       ConstantBlendingAlphaLayer61 : 8;
            uint32_t       ConstantBlendingAlphaLayer71 : 8;
            uint32_t       OutputDepth                  : 8;
        };

        // DNDI
        struct
        {
            uint32_t       TopFieldFirst               : 8;
            uint32_t                                   : 24;
        };

        // Dataport-based rotation
        struct
        {
            uint32_t       CscConstantC10              : 16;
            uint32_t       CscConstantC11              : 16;
        };

        uint32_t       Value;
    } DW07;

    // DWORD 8 - GRF R2.0
    union
    {
        struct
        {
            uint32_t       DestinationRectangleWidth   : 16;
            uint32_t       DestinationRectangleHeight  : 16;
        };

        // Dataport-based rotation
        struct
        {
            uint32_t       InputIndex                  : 32;
        };

        uint32_t       Value;
    } DW08;

    // DWORD 9 - GRF R2.1
    union
    {
        // Common structure for all gens
        struct
        {
            uint32_t       RotationMirrorMode          : 3;
            uint32_t       RotationMirrorAllLayer      : 1;
            uint32_t       DualOutputMode              : 1;
            uint32_t                                   : 11;
            uint32_t       ChannelSwap                 : 1;
            uint32_t                                   : 1;
            uint32_t                                   : 1;
            uint32_t                                   : 13;
        };

        // Bit field(s) for gen8+ only
        struct
        {
            uint32_t                                   : 3;
            uint32_t                                   : 1;
            uint32_t                                   : 12;
            uint32_t                                   : 1;
            uint32_t                                   : 10;
            uint32_t       IEFByPassEnable             : 1;
            uint32_t                                   : 4;
        };

        // Bit field(s) for Gen9+ only
        struct
        {
            uint32_t                                   : 5;
            uint32_t                                   : 3;
            uint32_t                                   : 8;
            uint32_t                                   : 1;
            uint32_t                                   : 1;
            uint32_t       AlphaChannelCalculation     : 1;
            uint32_t                                   : 8;
            uint32_t                                   : 1;
            uint32_t                                   : 4;
        };

        // Dataport-based rotation
        struct
        {
            uint32_t       OutputIndex                 : 32;
        };

        uint32_t       Value;
    } DW09;

    // DWORD 10 - GRF R2.2
    union
    {
        struct  // Gen7.5 and Gen8 Chroma Siting
        {
            uint32_t       ChromaSitingLocation        : 3;
            uint32_t                                   : 29;
        };

        uint32_t       Value;
    } DW10;

    // DWORD 11 - GRF R2.3
    union
    {
        uint32_t       Value;
    } DW11;

    // DWORD 12 - GRF R2.4
    union
    {
        // CE
        struct
        {
            uint32_t       ColorProcessingEnable       : 1;
            uint32_t                                   : 1;
            uint32_t       MessageFormat               : 2;
            uint32_t                                   : 1;
            uint32_t       ColorProcessingStatePointer : 27;
        };

        uint32_t       Value;
    } DW12;

    // DWORD 13 - GRF R2.5
    union
    {
        // Colorfill ARGB
        struct
        {
            uint32_t       ColorFill_R                 : 8;    // R
            uint32_t       ColorFill_G                 : 8;    // G
            uint32_t       ColorFill_B                 : 8;    // B
            uint32_t       ColorFill_A                 : 8;    // A
        };

        // Colorfill AYUV
        struct
        {
            uint32_t       ColorFill_V                 : 8;    // V
            uint32_t       ColorFill_Y                 : 8;    // Y
            uint32_t       ColorFill_U                 : 8;    // U
            uint32_t                                   : 8;    // A
        };

        uint32_t       Value;
    } DW13;

    // DWORD 14 - GRF R2.6
    union
    {
        // Lumakey, NLAS
        struct
        {
            uint32_t       LumakeyLowThreshold         : 8;
            uint32_t       LumakeyHighThreshold        : 8;
            uint32_t       NLASEnable                  : 8;
            uint32_t       Reserved                    : 8;
        };

        uint32_t       Value;
    } DW14;

    // DWORD 15 - GRF R2.7
    union
    {
        // Save
        struct
        {
            uint8_t     DestinationPackedYOffset;
            uint8_t     DestinationPackedUOffset;
            uint8_t     DestinationPackedVOffset;
            uint8_t     DestinationRGBFormat;
        };

        uint32_t       Value;
    } DW15;

    // DWORD 16 - GRF R3.0
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalScalingStepRatioLayer0;
        };

        uint32_t       Value;
    } DW16;

    // DWORD 17 - GRF R3.1
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalScalingStepRatioLayer1;
        };

        uint32_t       Value;
    } DW17;

    // DWORD 18 - GRF R3.2
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalScalingStepRatioLayer2;
        };

        uint32_t       Value;
    } DW18;

    // DWORD 19 - GRF R3.3
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalScalingStepRatioLayer3;
        };

        uint32_t       Value;
    } DW19;

    // DWORD 20 - GRF R3.4
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalScalingStepRatioLayer4;
        };

        uint32_t       Value;
    } DW20;

    // DWORD 21 - GRF R3.5
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalScalingStepRatioLayer5;
        };

        uint32_t       Value;
    } DW21;

    // DWORD 22 - GRF R3.6
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalScalingStepRatioLayer6;
        };

        uint32_t       Value;
    } DW22;

    // DWORD 23 - GRF R3.7
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalScalingStepRatioLayer7;
        };

        uint32_t       Value;
    } DW23;

    // DWORD 24 - GRF R4.0
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalScalingStepRatioLayer0;
        };

        // Dataport Load
        struct
        {
            uint8_t        SourcePackedYOffset;
            uint8_t        SourcePackedUOffset;
            uint8_t        SourcePackedVOffset;
            uint8_t        Reserved;
        };

        uint32_t       Value;
    } DW24;

    // DWORD 25 - GRF R4.1
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalScalingStepRatioLayer1;
        };

        uint32_t       Value;
    } DW25;

    // DWORD 26 - GRF R4.2
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalScalingStepRatioLayer2;
        };

        // Dataport Load
        struct
        {
            uint32_t       HorizontalFrameOriginOffset : 16;
            uint32_t       VerticalFrameOriginOffset   : 16;
        };

        uint32_t       Value;
    } DW26;

    // DWORD 27 - GRF R4.3
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalScalingStepRatioLayer3;
        };

        uint32_t       Value;
    } DW27;

    // DWORD 28 - GRF R4.4
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalScalingStepRatioLayer4;
        };

        uint32_t       Value;
    } DW28;

    // DWORD 29 - GRF R4.5
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalScalingStepRatioLayer5;
        };

        uint32_t       Value;
    } DW29;

    // DWORD 30 - GRF R4.6
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalScalingStepRatioLayer6;
        };

        uint32_t       Value;
    } DW30;

    // DWORD 31 - GRF R4.7
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalScalingStepRatioLayer7;
        };

        uint32_t       Value;
    } DW31;

    // DWORD 32 - GRF R5.0
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalFrameOriginLayer0;
        };

        uint32_t       Value;
    } DW32;

    // DWORD 33 - GRF R5.1
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalFrameOriginLayer1;
        };

        uint32_t       Value;
    } DW33;

    // DWORD 34 - GRF R5.2
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalFrameOriginLayer2;
        };

        uint32_t       Value;
    } DW34;

    // DWORD 35 - GRF R5.3
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalFrameOriginLayer3;
        };

        uint32_t       Value;
    } DW35;

    // DWORD 36 - GRF R5.4
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalFrameOriginLayer4;
        };

        uint32_t       Value;
    } DW36;

    // DWORD 37 - GRF R5.5
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalFrameOriginLayer5;
        };

        uint32_t       Value;
    } DW37;

    // DWORD 38 - GRF R5.6
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalFrameOriginLayer6;
        };

        uint32_t       Value;
    } DW38;

    // DWORD 39 - GRF R5.7
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalFrameOriginLayer7;
        };

        uint32_t       Value;
    } DW39;

    // DWORD 40 - GRF R6.0
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalFrameOriginLayer0;
        };

        uint32_t       Value;
    } DW40;

    // DWORD 41 - GRF R6.1
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalFrameOriginLayer1;
        };

        uint32_t       Value;
    } DW41;

    // DWORD 42 - GRF R6.2
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalFrameOriginLayer2;
        };

        uint32_t       Value;
    } DW42;

    // DWORD 43 - GRF R6.3
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalFrameOriginLayer3;
        };

        uint32_t       Value;
    } DW43;

    // DWORD 44 - GRF R6.4
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalFrameOriginLayer4;
        };

        uint32_t       Value;
    } DW44;

    // DWORD 45 - GRF R6.5
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalFrameOriginLayer5;
        };

        uint32_t       Value;
    } DW45;

    // DWORD 46 - GRF R6.6
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalFrameOriginLayer6;
        };

        uint32_t       Value;
    } DW46;

    // DWORD 47 - GRF R6.7
    union
    {
        // Sampler Load
        struct
        {
            float   HorizontalFrameOriginLayer7;
        };

        uint32_t       Value;
    } DW47;

    // DWORD48  - GRF R7.0
    union
    {
        struct
        {
            uint32_t       DestXTopLeftLayer0  : BITFIELD_RANGE(0,15);
            uint32_t       DestYTopLeftLayer0  : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW48;

    // DWORD49  - GRF R7.1
    union
    {
        struct
        {
            uint32_t       DestXTopLeftLayer1  : BITFIELD_RANGE(0,15);
            uint32_t       DestYTopLeftLayer1  : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW49;

    // DWORD50  - GRF R7.2
    union
    {
        struct
        {
            uint32_t       DestXTopLeftLayer2  : BITFIELD_RANGE(0,15);
            uint32_t       DestYTopLeftLayer2  : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW50;

    // DWORD51  - GRF R7.3
    union
    {
        struct
        {
            uint32_t       DestXTopLeftLayer3  : BITFIELD_RANGE(0,15);
            uint32_t       DestYTopLeftLayer3  : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW51;

    // DWORD52  - GRF R7.4
    union
    {
        struct
        {
            uint32_t       DestXTopLeftLayer4  : BITFIELD_RANGE(0,15);
            uint32_t       DestYTopLeftLayer4  : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW52;

    // DWORD53  - GRF R7.5
    union
    {
        struct
        {
            uint32_t       DestXTopLeftLayer5  : BITFIELD_RANGE(0,15);
            uint32_t       DestYTopLeftLayer5  : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW53;

    // DWORD54  - GRF R7.6
    union
    {
        struct
        {
            uint32_t       DestXTopLeftLayer6  : BITFIELD_RANGE(0,15);
            uint32_t       DestYTopLeftLayer6  : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW54;

    // DWORD55  - GRF R7.7
    union
    {
        struct
        {
            uint32_t       DestXTopLeftLayer7  : BITFIELD_RANGE(0,15);
            uint32_t       DestYTopLeftLayer7  : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW55;

    // DWORD56  - GRF R8.0
    union
    {
        struct
        {
            uint32_t       DestXBottomRightLayer0  : BITFIELD_RANGE(0,15);
            uint32_t       DestYBottomRightLayer0  : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW56;

    // DWORD57  - GRF R8.1
    union
    {
        struct
        {
            uint32_t       DestXBottomRightLayer1  : BITFIELD_RANGE(0,15);
            uint32_t       DestYBottomRightLayer1  : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW57;

    // DWORD58  - GRF R8.2
    union
    {
        struct
        {
            uint32_t       DestXBottomRightLayer2  : BITFIELD_RANGE(0,15);
            uint32_t       DestYBottomRightLayer2  : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW58;

    // DWORD59  - GRF R8.3
    union
    {
        struct
        {
            uint32_t       DestXBottomRightLayer3  : BITFIELD_RANGE(0,15);
            uint32_t       DestYBottomRightLayer3  : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW59;

    // DWORD60  - GRF R8.4
    union
    {
        struct
        {
            uint32_t       DestXBottomRightLayer4  : BITFIELD_RANGE(0,15);
            uint32_t       DestYBottomRightLayer4  : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW60;

    // DWORD61  - GRF R8.5
    union
    {
        struct
        {
            uint32_t       DestXBottomRightLayer5  : BITFIELD_RANGE(0,15);
            uint32_t       DestYBottomRightLayer5  : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW61;

    // DWORD62  - GRF R8.6
    union
    {
        struct
        {
            uint32_t       DestXBottomRightLayer6  : BITFIELD_RANGE(0,15);
            uint32_t       DestYBottomRightLayer6  : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW62;

    // DWORD63  - GRF R8.7
    union
    {
        struct
        {
            uint32_t       DestXBottomRightLayer7  : BITFIELD_RANGE(0,15);
            uint32_t       DestYBottomRightLayer7  : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW63;

    // DWORD64  - GRF R9.0
    union
    {
        struct
        {
            float       MainVideoXScalingStepLeft;
        };

        uint32_t       Value;
    } DW64;

    // DWORD65  - GRF R9.1
    union
    {
        struct
        {
            float       VideoStepDeltaForNonLinearRegion;
        };

        uint32_t       Value;
    } DW65;

    // DWORD66  - GRF R9.2
    union
    {
        struct
        {
            uint32_t       StartofLinearScalingInPixelPositionC0           : BITFIELD_RANGE(0,15);
            uint32_t       StartofRHSNonLinearScalingInPixelPositionC1     : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW66;

    // DWORD 67 - GRF R9.3
    union
    {
        // Sampler Load
        struct
        {
            float       MainVideoXScalingStepCenter;
        };

        uint32_t       Value;
    } DW67;

    // DWORD 68 - GRF R9.4
    union
    {
        // Sampler Load
        struct
        {
            float       MainVideoXScalingStepRight;
        };

        uint32_t       Value;
    } DW68;

    // DWORD63  - GRF R8.7
    union
    {
        struct
        {
            uint32_t       DestHorizontalBlockOrigin  : BITFIELD_RANGE(0,15);
            uint32_t       DestVerticalBlockOrigin    : BITFIELD_RANGE(16,31);
        };

        uint32_t       Value;
    } DW69;

    // DWORD 70-71 - GRF 9.6-9.7 - Padding is needed as we program ConstantURBEntryReadLength  = iCurbeLength >> 5
    uint32_t           dwPad[2];
};

// Defined in Inline parameter computation
struct MEDIA_OBJECT_KA2_STATIC_DATA
{
    // DWORD 0 - GRF R1.0
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC0               : 16;
            uint32_t       CscConstantC1               : 16;
        };

        // Chroma Denoise
        struct
        {
            uint32_t       LocalDifferenceThresholdU   : 8;
            uint32_t       LocalDifferenceThresholdV   : 8;
            uint32_t       SobelEdgeThresholdU         : 8;
            uint32_t       SobelEdgeThresholdV         : 8;
        };

        uint32_t       Value;
    } DW00;

    // DWORD 1 - GRF R1.1
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC2               : 16;
            uint32_t       CscConstantC3               : 16;
        };

        // Chroma Denoise
        struct
        {
            uint32_t       HistoryInitialValueU        : 8;
            uint32_t       HistoryInitialValueV        : 8;
            uint32_t       HistoryMaxU                 : 8;
            uint32_t       HistoryMaxV                 : 8;
        };

        uint32_t       Value;
    } DW01;

    // DWORD 2 - GRF R1.2
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC4               : 16;
            uint32_t       CscConstantC5               : 16;
        };

        // Chroma Denoise
        struct
        {
            uint32_t       HistoryDeltaU               : 8;
            uint32_t       HistoryDeltaV               : 8;
            uint32_t       DNSADThresholdU             : 8;
            uint32_t       DNSADThresholdV             : 8;
        };

        uint32_t       Value;
    } DW02;

    // DWORD 3 - GRF R1.3
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC6               : 16;
            uint32_t       CscConstantC7               : 16;
        };

        // Chroma Denoise
        struct
        {
            uint32_t       DNTDThresholdU              : 8;
            uint32_t       DNTDThresholdV              : 8;
            uint32_t       DNLTDThresholdU             : 8;
            uint32_t       DNLTDThresholdV             : 8;
        };

        uint32_t       Value;
    } DW03;

    // DWORD 4 - GRF R1.4
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC8               : 16;
            uint32_t       CscConstantC9               : 16;
        };

        uint32_t       Value;
    } DW04;

    // DWORD 5 - GRF R1.5
    union
    {
        // CSC
        struct
        {
            uint32_t       CscConstantC10              : 16;
            uint32_t       CscConstantC11              : 16;
        };

        uint32_t       Value;
    } DW05;

    // DWORD 6 - GRF R1.6
    union
    {
        // Const Blending
        struct
        {
            uint32_t       ConstantBlendingAlphaLayer1 : 8;
            uint32_t       ConstantBlendingAlphaLayer2 : 8;
            uint32_t       ConstantBlendingAlphaLayer3 : 8;
            uint32_t       ConstantBlendingAlphaLayer4 : 8;
        };

        // DNDI
        struct
        {
            uint32_t       HalfStatisticsSurfacePitch : 16;  // Statistics surface pitch divided by 2
            uint32_t       StatisticsSurfaceHeight    : 16;  // Statistics surface height divided by 4
        };

        uint32_t       Value;
    } DW06;

    // DWORD 7 - GRF R1.7
    union
    {
        // Const Blending
        struct
        {
            uint32_t       ConstantBlendingAlphaLayer5 : 8;
            uint32_t       ConstantBlendingAlphaLayer6 : 8;
            uint32_t       ConstantBlendingAlphaLayer7 : 8;
            uint32_t       PointerToInlineParameters   : 8;
        };

        struct
        {
            uint32_t       ConstantBlendingAlphaLayer51 : 8;
            uint32_t       ConstantBlendingAlphaLayer61 : 8;
            uint32_t       ConstantBlendingAlphaLayer71 : 8;
            uint32_t       OutputDepth                  : 8;
        };

        // DNDI
        struct
        {
            uint32_t       TopFieldFirst               : 8;
            uint32_t                                   : 24;
        };

        uint32_t       Value;
    } DW07;

    // DWORD 8 - GRF R2.0
    union
    {
        struct
        {
            uint32_t       DestinationRectangleWidth   : 16;
            uint32_t       DestinationRectangleHeight  : 16;
        };

        uint32_t       Value;
    } DW08;

    // DWORD 9 - GRF R2.1
    union
    {
        struct
        {
            uint32_t       RotationMirrorMode          : 3;
            uint32_t       RotationMirrorAllLayer      : 1;
            uint32_t       DualOutputMode              : 1;
            uint32_t                                   : 11;
            uint32_t       ChannelSwap                 : 1;
            uint32_t                                   : 1;
            uint32_t                                   : 1;
            uint32_t                                   : 13;
        };

        // Bit field(s) for gen8+ only
        struct
        {
            uint32_t                                   : 8;    // Dual output
            uint32_t                                   : 8;    // Reserved
            uint32_t       ChannelSwap                 : 1;
            uint32_t       AlphaChannelCalculation     : 1;
            uint32_t                                   : 9;
            uint32_t       IEFByPassEnable             : 1;
            uint32_t                                   : 4;
        }ObjKa2Gen8;

        // Bit field(s) for Gen9+ only
        struct
        {
            uint32_t                                   : 5;
            uint32_t                                   : 3;
            uint32_t                                   : 8;
            uint32_t       ChannelSwap                 : 1;
            uint32_t       DitheringAvdFlag            : 1;
            uint32_t       AlphaChannelCalculation     : 1;
            uint32_t                                   : 8;
            uint32_t       IEFByPassEnable             : 1;
            uint32_t                                   : 4;
        }ObjKa2Gen9;

        uint32_t       Value;
    } DW09;

    // DWORD 10 - GRF R2.2
    union
    {
        struct  // Gen7.5 and Gen8 Chroma Siting
        {
            uint32_t       ChromaSitingLocation        : 3;
            uint32_t                                   : 29;
        };

        struct  // Gen9+ Rotation
        {
            uint32_t       RotationAngleofLayer0       : 3;
            uint32_t       RotationAngleofLayer1       : 3;
            uint32_t       RotationAngleofLayer2       : 3;
            uint32_t       RotationAngleofLayer3       : 3;
            uint32_t       RotationAngleofLayer4       : 3;
            uint32_t       RotationAngleofLayer5       : 3;
            uint32_t       RotationAngleofLayer6       : 3;
            uint32_t       RotationAngleofLayer7       : 3;
            uint32_t       ChromaSitingLocation        : 3;
            uint32_t       MonoXORCompositeMask        : 3;
            uint32_t                                   : 2;
        } ObjKa2Gen9;

        uint32_t       Value;
    } DW10;

    // DWORD 11 - GRF R2.3
    union
    {
        struct
        {
            float       TopBottomDelta;                     // Used for interlace scaling on Gen8-
        };

        struct
        {
            float       ChromasitingUOffset;               // Param for 3D Sampler use case
        };

        uint32_t       Value;
    } DW11;

    // DWORD 12 - GRF R2.4
    union
    {
        // CE
        struct
        {
            uint32_t       ColorProcessingEnable       : 1;
            uint32_t                                   : 1;
            uint32_t       MessageFormat               : 2;
            uint32_t                                   : 1;
            uint32_t       ColorProcessingStatePointer : 27;
        };

        struct
        {
            float       TopBottomDelta;                     // Used for interlace scaling on Gen9+
        };

        struct
        {
            float       ChromasitingVOffset;                // Param only for 3D Sampler use case
        };

        uint32_t       Value;
    } DW12;

    // DWORD 13 - GRF R2.5
    union
    {
        // Colorfill ARGB
        struct
        {
            uint32_t       ColorFill_R                 : 8;    // R
            uint32_t       ColorFill_G                 : 8;    // G
            uint32_t       ColorFill_B                 : 8;    // B
            uint32_t       ColorFill_A                 : 8;    // A
        };

        // Colorfill AYUV
        struct
        {
            uint32_t       ColorFill_V                 : 8;    // V
            uint32_t       ColorFill_Y                 : 8;    // Y
            uint32_t       ColorFill_U                 : 8;    // U
            uint32_t                                   : 8;    // A
        };

        uint32_t       Value;
    } DW13;

    // DWORD 14 - GRF R2.6
    union
    {
        // Lumakey, NLAS
        struct
        {
            uint32_t       LumakeyLowThreshold         : 8;
            uint32_t       LumakeyHighThreshold        : 8;
            uint32_t       NLASEnable                  : 8;
            uint32_t       Reserved                    : 8;
        };

        uint32_t       Value;
    } DW14;

    // DWORD 15 - GRF R2.7
    union
    {
        // Save
        struct
        {
            uint8_t        DestinationPackedYOffset;
            uint8_t        DestinationPackedUOffset;
            uint8_t        DestinationPackedVOffset;
            uint8_t        DestinationRGBFormat;
        };

        uint32_t       Value;
    } DW15;

    // DWORD 16 - GRF R3.0
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalScalingStepRatioLayer0;
        };

        uint32_t       Value;
    } DW16;

    // DWORD 17 - GRF R3.1
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalScalingStepRatioLayer1;
        };

        uint32_t       Value;
    } DW17;

    // DWORD 18 - GRF R3.2
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalScalingStepRatioLayer2;
        };

        uint32_t       Value;
    } DW18;

    // DWORD 19 - GRF R3.3
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalScalingStepRatioLayer3;
        };

        uint32_t       Value;
    } DW19;

    // DWORD 20 - GRF R3.4
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalScalingStepRatioLayer4;
        };

        uint32_t       Value;
    } DW20;

    // DWORD 21 - GRF R3.5
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalScalingStepRatioLayer5;
        };

        uint32_t       Value;
    } DW21;

    // DWORD 22 - GRF R3.6
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalScalingStepRatioLayer6;
        };

        uint32_t       Value;
    } DW22;

    // DWORD 23 - GRF R3.7
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalScalingStepRatioLayer7;
        };

        uint32_t       Value;
    } DW23;

    // DWORD 24 - GRF R4.0
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalScalingStepRatioLayer0;
        };

        // Dataport Load
        struct
        {
            uint8_t     SourcePackedYOffset;
            uint8_t     SourcePackedUOffset;
            uint8_t     SourcePackedVOffset;
            uint8_t     Reserved;
        };

        uint32_t       Value;
    } DW24;

    // DWORD 25 - GRF R4.1
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalScalingStepRatioLayer1;
        };

        uint32_t       Value;
    } DW25;

    // DWORD 26 - GRF R4.2
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalScalingStepRatioLayer2;
        };

        // Dataport Load
        struct
        {
            uint32_t       HorizontalFrameOriginOffset : 16;
            uint32_t       VerticalFrameOriginOffset   : 16;
        };

        uint32_t       Value;
    } DW26;

    // DWORD 27 - GRF R4.3
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalScalingStepRatioLayer3;
        };

        uint32_t       Value;
    } DW27;

    // DWORD 28 - GRF R4.4
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalScalingStepRatioLayer4;
        };

        uint32_t       Value;
    } DW28;

    // DWORD 29 - GRF R4.5
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalScalingStepRatioLayer5;
        };

        uint32_t       Value;
    } DW29;

    // DWORD 30 - GRF R4.6
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalScalingStepRatioLayer6;
        };

        uint32_t       Value;
    } DW30;

    // DWORD 31 - GRF R4.7
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalScalingStepRatioLayer7;
        };

        uint32_t       Value;
    } DW31;

    // DWORD 32 - GRF R5.0
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalFrameOriginLayer0;
        };

        uint32_t       Value;
    } DW32;

    // DWORD 33 - GRF R5.1
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalFrameOriginLayer1;
        };

        uint32_t       Value;
    } DW33;

    // DWORD 34 - GRF R5.2
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalFrameOriginLayer2;
        };

        uint32_t       Value;
    } DW34;

    // DWORD 35 - GRF R5.3
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalFrameOriginLayer3;
        };

        uint32_t       Value;
    } DW35;

    // DWORD 36 - GRF R5.4
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalFrameOriginLayer4;
        };

        uint32_t       Value;
    } DW36;

    // DWORD 37 - GRF R5.5
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalFrameOriginLayer5;
        };

        uint32_t       Value;
    } DW37;

    // DWORD 38 - GRF R5.6
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalFrameOriginLayer6;
        };

        uint32_t       Value;
    } DW38;

    // DWORD 39 - GRF R5.7
    union
    {
        // Sampler Load
        struct
        {
            float       VerticalFrameOriginLayer7;
        };

        uint32_t       Value;
    } DW39;

    // DWORD 40 - GRF R6.0
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalFrameOriginLayer0;
        };

        uint32_t       Value;
    } DW40;

    // DWORD 41 - GRF R6.1
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalFrameOriginLayer1;
        };

        uint32_t       Value;
    } DW41;

    // DWORD 42 - GRF R6.2
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalFrameOriginLayer2;
        };

        uint32_t       Value;
    } DW42;

    // DWORD 43 - GRF R6.3
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalFrameOriginLayer3;
        };

        uint32_t       Value;
    } DW43;

    // DWORD 44 - GRF R6.4
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalFrameOriginLayer4;
        };

        uint32_t       Value;
    } DW44;

    // DWORD 45 - GRF R6.5
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalFrameOriginLayer5;
        };

        uint32_t       Value;
    } DW45;

    // DWORD 46 - GRF R6.6
    union
    {
        // Sampler Load
        struct
        {
            float       HorizontalFrameOriginLayer6;
        };

        uint32_t       Value;
    } DW46;

    // DWORD 47 - GRF R6.7
    union
    {
        // Sampler Load
        struct
        {
            float   HorizontalFrameOriginLayer7;
        };

        uint32_t       Value;
    } DW47;
};

struct MEDIA_OBJECT_KA2_INLINE_DATA
{
    // DWORD 0 - GRF R7.0
    union
    {
        // All
        struct
        {
            uint32_t       DestinationBlockHorizontalOrigin : 16;
            uint32_t       DestinationBlockVerticalOrigin   : 16;
        };

        // Secure Block Copy
        struct
        {
            uint32_t       BlockHeight                     : 16;
            uint32_t       BufferOffset                    : 16;
        };

        // FMD Summation
        struct
        {
            uint32_t       StartRowOffset;
        };

        uint32_t       Value;
    } DW00;

    // DWORD 1 - GRF R7.1
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer0 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer0   : 16;
        };

        // FMD Summation
        struct
        {
            uint32_t       TotalRows;
        };

        uint32_t       Value;
    } DW01;

    // DWORD 2 - GRF R7.2
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer1 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer1   : 16;
        };

        // FMD Summation
        struct
        {
            uint32_t       StartColumnOffset;
        };

        uint32_t       Value;
    } DW02;

    // DWORD 3 - GRF R7.3
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer2 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer2   : 16;
        };

        // FMD Summation
        struct
        {
            uint32_t       TotalColumns;
        };

        uint32_t       Value;
    } DW03;

    // DWORD 4 - GRF R7.4
    union
    {
        // Sampler Load
        struct
        {
            float       VideoXScalingStep;
        };

        uint32_t       Value;
    } DW04;

    // DWORD 5 - GRF R7.5
    union
    {
        // NLAS
        struct
        {
            float       VideoStepDelta;
        };

        uint32_t       Value;
    } DW05;

    // DWORD 6 - GRF R7.6
    union
    {
        // AVScaling
        struct
        {
            uint32_t       VerticalBlockNumber                :17;
            uint32_t       AreaOfInterest                     :1;
            uint32_t                                          :14;
        };

        uint32_t       Value;
    } DW06;

    // DWORD 7 - GRF R7.7
    union
    {
        // AVScaling
        struct
        {
            uint32_t       GroupIDNumber;
        };

        uint32_t       Value;
    } DW07;

    // DWORD 8 - GRF R8.0
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer3 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer3   : 16;
        };

        uint32_t       Value;
    } DW08;

    // DWORD 9 - GRF R8.1
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer4 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer4   : 16;
        };

        uint32_t       Value;
    } DW09;

    // DWORD 10 - GRF R8.2
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer5 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer5   : 16;
        };

        uint32_t       Value;
    } DW10;

    // DWORD 11 - GRF R8.3
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer6 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer6   : 16;
        };

        uint32_t       Value;
    } DW11;

    // DWORD 12 - GRF R8.4
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer7 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer7   : 16;
        };

        uint32_t       Value;
    } DW12;

    // DWORD 13 - GRF R8.5
    union
    {
        struct
        {
            uint32_t       Reserved;
        };

        uint32_t       Value;
    } DW13;

    // DWORD 14 - GRF R8.6
    union
    {
        struct
        {
            uint32_t       Reserved;
        };

        uint32_t       Value;
    } DW14;

    // DWORD 15 - GRF R8.7
    union
    {
        struct
        {
            uint32_t       Reserved;
        };

        uint32_t       Value;
    } DW15;
};

extern const MEDIA_OBJECT_KA2_INLINE_DATA g_cInit_MEDIA_OBJECT_KA2_INLINE_DATA;

//!
//! \brief Vebox mode of operation
//!
typedef enum _VEBOX_EXECUTION_MODE
{
    VEBOX_EXEC_MODE_0      = 0,                                                 //!< Non-parallel serial legacy mode. Vphal_VeboxRender() processes current frame.
    VEBOX_EXEC_MODE_0_TO_2 = 1,                                                 //!< Future frame detected so transition from serial to parallel mode. Vphal_VeboxRender() processes current and future frame.
    VEBOX_EXEC_MODE_2      = 2,                                                 //!< Parallel execution. Vphal_VeboxRender() processes future frame.
    VEBOX_EXEC_MODE_2_TO_0 = 3                                                  //!< No future frame so switch back to non-parallel serial legacy mode.
} VEBOX_EXECUTION_MODE;

#define MAX_VEBOX_EXECUTION_MODE 4
//!
//! \brief Controls Vebox execution mode
//!
typedef struct _VPHAL_VEBOX_EXEC_STATE
{
    bool                    bEnable                             = false;                                   //!< false, legacy serial execution. true, capable of parallel vebox/render execution
    VEBOX_EXECUTION_MODE    Mode                                = VEBOX_EXEC_MODE_0;                       //!< Current mode of operation.
    bool                    bDIOutputPair01                     = false;                                   //!< Used to alternate between ADI output pairs.
    bool                    bSpeculativeCopy                    = false;                                   //!< true, update VEBOX state for frame N+1 using frame N state
    bool                    bFrcActive                          = false;                                   //!< When FRC is active, stay in VEBOX_EXEC_MODE_0
    bool                    bPostponedFMDCalc                   = false;                                   //!< When in mode2, need to calc fmd variance after composition
    uint32_t                ModeCount[MAX_VEBOX_EXECUTION_MODE] = {0};
} VPHAL_VEBOX_EXEC_STATE, *PVPHAL_VEBOX_EXEC_STATE;

#define RESET_VEBOX_SPECULATIVE_COPY(_a)              (_a->bSpeculativeCopy = false)

//!
//! \brief Should only request in Mode2 or Mode0->Mode2
//!
#define REQUEST_VEBOX_SPECULATIVE_COPY(_a)            (_a->bSpeculativeCopy = true)
#define IS_VEBOX_SPECULATIVE_COPY_REQUESTED(_a)       (_a->bSpeculativeCopy == true)
#define SET_VEBOX_EXECUTION_MODE(_a, _Mode)                           \
    {                                                                 \
        (_a->Mode = _Mode);                                           \
        VPHAL_RENDER_NORMALMESSAGE("VEBOX_EXECUTION_MODE %d", _Mode); \
    }
#define IS_VEBOX_EXECUTION_MODE_PARALLEL_CAPABLE(_a)  (_a->bEnable == true)
#define IS_VEBOX_EXECUTION_MODE_0(_a)                 (_a->Mode == VEBOX_EXEC_MODE_0)
#define IS_VEBOX_EXECUTION_MODE_0_TO_2(_a)            (_a->Mode == VEBOX_EXEC_MODE_0_TO_2)
#define IS_VEBOX_EXECUTION_MODE_2(_a)                 (_a->Mode == VEBOX_EXEC_MODE_2)
#define IS_VEBOX_EXECUTION_MODE_2_TO_0(_a)            (_a->Mode == VEBOX_EXEC_MODE_2_TO_0)
#define VEBOX_EXECUTION_OVERRIDE_ENABLE               1
#define VEBOX_EXECUTION_OVERRIDE_DISABLE              2
#define RESET_VEBOX_POSTPONED_FMD_CALC(_a)            (_a->bPostponedFMDCalc = false)

//!
//! \brief Should only request in Mode2 or Mode0->Mode2
//!
#define REQUEST_VEBOX_POSTPONED_FMD_CALC(_a)          (_a->bPostponedFMDCalc = true)
#define IS_VEBOX_POSTPONED_FMD_CALC_REQUESTED(_a)     (_a->bPostponedFMDCalc == true)

//------------------------------------------------------------------------------
// GLOBAL DEFINITIONS
//------------------------------------------------------------------------------

#define VPHAL_SAMPLER_BIAS_GEN575   0.015625f
#define VPHAL_HW_LINEAR_SHIFT       0.5f
#define FIRST_FRAME                 -1024

//!
//! \brief The size of the General Register File(GRF)
//!
#define GRF_SIZE    (8 * sizeof(uint32_t))

//------------------------------------------------------------------------------
// COMPOSITING DEFINITIONS
//------------------------------------------------------------------------------
#define VPHAL_COMP_MAX_LAYERS       8
#define CM_MAX_KERNELS_PER_TASK     16

#define VPHAL_BB_ALIGN_SIZE         32768

//!
//! \brief      SLM: sharedlocalmemory. DC: data cache. 
//!             I/S: instruction/state cache 
//!             C: constant cache. T: texture cache
//!             Default for GT1/GT2
//!             SLM     URB     DC      RO      I/S     C       T
//!             { 0,    256,     0,     256,    0,      0,      0,      }
//!             Default TLB settings value provided by perf team
//!
#define VPHAL_L3_CACHE_CONFIG_SQCREG1_VALUE_G75     0x00610000
#define VPHAL_L3_CACHE_CONFIG_CNTLREG2_VALUE_G75    0x00880040
#define VPHAL_L3_CACHE_CONFIG_CNTLREG3_VALUE_G75    0x00000000
#define VPHAL_L3_CACHE_CONFIG_L3LRA1REG_VALUE_G75   0x27FD007F

//!
//! \brief Initialize MHW Kernel Param struct for loading Kernel
//!
#define INIT_MHW_KERNEL_PARAM(MhwKernelParam, _pKernelEntry)                        \
    do                                                                              \
    {                                                                               \
        MOS_ZeroMemory(&(MhwKernelParam), sizeof(MhwKernelParam));                  \
        (MhwKernelParam).pBinary  = (_pKernelEntry)->pBinary;                       \
        (MhwKernelParam).iSize    = (_pKernelEntry)->iSize;                         \
        (MhwKernelParam).iKUID    = (_pKernelEntry)->iKUID;                         \
        (MhwKernelParam).iKCID    = (_pKernelEntry)->iKCID;                         \
        (MhwKernelParam).iPaddingSize = (_pKernelEntry)->iPaddingSize;              \
    } while(0)

//!
//! \brief Enum of the phase of Image Stabilization
//!
typedef enum _VPHAL_ISTAB_PHASE
{
    ISTAB_PH_NONE = -1,
    ISTAB_PH1_DS      ,                                                         //!< Down Scaling
    ISTAB_PH2_ME      ,                                                         //!< Motion Estimation
    ISTAB_PH3_GMC     ,                                                         //!< Global Motion Compensation
    ISTAB_PH4_FW      ,                                                         //!< Frame Warping
    ISTAB_PH_MAX
} VPHAL_ISTAB_PHASE;

typedef struct _VPHAL_BATCH_BUFFER          *PVPHAL_BATCH_BUFFER;

//!
//! \brief Compositing BB arguments
//!
typedef struct _VPHAL_BB_COMP_ARGS
{
    int32_t                iMediaID;                                            //!< Media ID used to generate CBs
    float                  fStepX;                                              //!< X step (first layer)
    int32_t                iLayers;                                             //!< Valid layers
    RECT                   rcOutput;                                            //!< Block aligned output area
    RECT                   rcDst[VPHAL_COMP_MAX_LAYERS];                        //!< Dest rectangles for each layer
    bool                   bSkipBlocks;                                         //!< Render all blocks
    bool                   bEnableNLAS;                                         //!< NLAS enable
    VPHAL_ROTATION         Rotation[VPHAL_COMP_MAX_LAYERS];                     //!< Rotation parameter
    VPHAL_NLAS_PARAMS      NLASParams;                                          //!< NLAS parameters
} VPHAL_BB_COMP_ARGS, *PVPHAL_BB_COMP_ARGS;

//!
//! \brief Generic BB Args
//!
typedef struct
{
    int32_t     iMediaID;                                                           //!< Media ID
    uint32_t    uiKuid;                                                             //!< Unique Kernel ID
    uint32_t    uiParamId;                                                          //!< Unique ID for the param (for handling changes)
} VPHAL_BB_GENERIC_ARGS, *PVPHAL_BB_GENERIC_ARGS;

//!
//! \brief Advanced Processing Definitions
//!
typedef struct _VPHAL_ADVPROC_BB_ARGS
{
    int32_t     iMediaID;                                                           //!< Media ID used to generate CBs
    RECT        rcDst;                                                              //!< layer target
    int32_t     BlockWd;                                                            //!< Media Obj Block Width
    int32_t     BlockHt;                                                            //!< Media Obj Block Height
} VPHAL_ADVPROC_BB_ARGS, *PVPHAL_ADVPROC_BB_ARGS;

//!
//! \brief Image Stabilization BB Args
//!
typedef struct _VPHAL_ISTAB_BB_ARGS
{
    VPHAL_ISTAB_PHASE IStabPhase;
    uint32_t          dwWidth;
    uint32_t          dwHeight;
} VPHAL_ISTAB_BB_ARGS, *PVPHAL_ISTAB_BB_ARGS;

//!
//! \brief Batch buffer kind
//!
typedef enum _VPHAL_BB_TYPE
{
    VPHAL_BB_TYPE_COMPOSITING,
    VPHAL_BB_TYPE_ADVANCED,
    VPHAL_BB_TYPE_GENERIC,
    VPHAL_BB_TYPE_PRIVATE
} VPHAL_BB_TYPE;

//!
//! \brief CM BB Args
//!
typedef struct _VPHAL_BB_CM_ARGS
{
    uint64_t  uiKernelIds[CM_MAX_KERNELS_PER_TASK];
    uint64_t  uiRefCount;
    bool      bLatest;
} VPHAL_BB_CM_ARGS, *PVPHAL_BB_CM_ARGS;

//!
//! \brief Global BB parameters
//!
typedef struct _VPHAL_BATCH_BUFFER_PARAMS
{
    bool                        bMatch;                                         //!< Indicates match
    int32_t                     iCallID;                                        //!< CallID last used
    VPHAL_BB_TYPE               iType;                                          //!< Indicates the render type
    int32_t                     iSize;                                          //!< Size of the current render args
    union                                                                       //!< Union of renders' args
    {
        VPHAL_BB_COMP_ARGS      CompositeBB;
        VPHAL_ADVPROC_BB_ARGS   AdvProcBB;
        VPHAL_BB_GENERIC_ARGS   BbGenericArgs;
        VPHAL_BB_CM_ARGS        BbCmArgs;
    } BbArgs;
} VPHAL_BATCH_BUFFER_PARAMS, *PVPHAL_BATCH_BUFFER_PARAMS;

typedef struct _VPHAL_BATCH_BUFFER
{
    MOS_RESOURCE        OsResource;                                             // OS Buffer
    int32_t             iSize;                                                  // Batch buffer size
    int32_t             iCurrent;                                               // Current offset in CB
    bool                bLocked;                                                // True if locked in memory (pData must be valid)
    uint8_t*            pData;                                                  // Pointer to BB data

    // Batch Buffer synchronization logic
    bool                bBusy;                                                  // Busy flag (clear when Sync Tag is reached)
    uint32_t            dwSyncTag;                                              // BB sync tag
    PVPHAL_BATCH_BUFFER pNext;                                                  // Next BB in the sync list
    PVPHAL_BATCH_BUFFER pPrev;                                                  // Prev BB in the sync list

    // Rendering data associated with BB
    PVPHAL_BATCH_BUFFER_PARAMS  pBBRenderData;                                  // Batch Buffer rendering data
} VPHAL_BATCH_BUFFER;

//!
//! \brief Unified Batch Buffer Table
//!
typedef struct _VPHAL_BATCH_BUFFER_TABLE
{
    int32_t*                    piBatchBufferCount;                             //!< Pointer to the count of allocated BBs in render's BB array
    int32_t                     iBbCountMax;                                    //!< Maximum count of BB that can be allocated of the render
    PMHW_BATCH_BUFFER           pBatchBufferHeader;                             //!< Pointer to the BB entry of the render
    PVPHAL_BATCH_BUFFER_PARAMS  pBbParamsHeader;                                //!< Pointer to the BB params entry of the render
} VPHAL_BATCH_BUFFER_TABLE, *PVPHAL_BATCH_BUFFER_TABLE;

//!
//! \brief Performance data value
//!
typedef struct
{
    uint32_t    uiVal;
    bool        bEnabled;
}VPHAL_RNDR_PERF_DATA_VAL;

//!
//! \brief Performance data collection
//!
typedef struct
{
    VPHAL_RNDR_PERF_DATA_VAL    CompMaxThreads;
    VPHAL_RNDR_PERF_DATA_VAL    DndiMaxThreads;
    VPHAL_RNDR_PERF_DATA_VAL    VdiFrameShareEnable;
    VPHAL_RNDR_PERF_DATA_VAL    VdiStride;
    VPHAL_RNDR_PERF_DATA_VAL    VdiColumnWidth;
    VPHAL_RNDR_PERF_DATA_VAL    L3SQCReg1Override;
    VPHAL_RNDR_PERF_DATA_VAL    L3CntlReg2Override;
    VPHAL_RNDR_PERF_DATA_VAL    L3CntlReg3Override;
    VPHAL_RNDR_PERF_DATA_VAL    L3LRA1RegOverride;
    VPHAL_RNDR_PERF_DATA_VAL    L3SQCReg4Override;
    VPHAL_RNDR_PERF_DATA_VAL    L3CntlRegOverride;
} VPHAL_RNDR_PERF_DATA, *PVPHAL_RNDR_PERF_DATA;

//!
//! \brief    Get the aligned the surface height and width unit
//! \details  Accoring to the format of the surface, get the aligned unit for the surface
//!           width and height
//! \param    [in,out] pwWidthAlignUnit
//!           Pointer to the surface width alignment unit
//! \param    [in,out] pwHeightAlignUnit
//!           Pointer to the surface height alignment unit
//! \param    [in] format
//!           The format of the surface
//! \return   void
//!
void VpHal_RndrGetAlignUnit(
    uint16_t*       pwWidthAlignUnit,
    uint16_t*       pwHeightAlignUnit,
    MOS_FORMAT      format);

//!
//! \brief    Set packed YUV component offsets
//! \details  Accoring to the format of the surface, set packed YUV component offsets
//! \param    [in] format
//!           The format of the surface
//! \param    [in,out] pOffsetY
//!           The offset of Y
//! \param    [in,out] pOffsetU
//!           The offset of U
//! \param    [in,out] pOffsetV
//!           The offset of V
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RndrSetYUVComponents(
    MOS_FORMAT      format,
    uint8_t*        pOffsetY,
    uint8_t*        pOffsetU,
    uint8_t*        pOffsetV);

//!
//! \brief    Set the numbers of Slice, Sub-slice, EUs for power mode
//! \details  Set the numbers of Slice, Sub-slice, EUs recommended for
//!           the given kernel type for power mode
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [in] KernelID
//!           VP render Kernel ID
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS VpHal_RndrCommonSetPowerMode(
    PRENDERHAL_INTERFACE                pRenderHal,
    VpKernelID                          KernelID);

//!
//! \brief    Initialized RenderHal Surface according to incoming VPHAL Surface
//! \param    [in] pVpSurface
//!           Pointer to the VPHAL surface
//! \param    [out] pRenderHalSurface
//!           Pointer to the RenderHal surface
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_RndrCommonInitRenderHalSurface(
    PVPHAL_SURFACE          pVpSurface,
    PRENDERHAL_SURFACE      pRenderHalSurface);

//!
//! \brief    Get output RenderHal Surface parameters back to VPHAL Surface
//! \param    [in] pRenderHalSurface
//!           Pointer to the RenderHal surface
//! \param    [in,out] pVpSurface
//!           Pointer to the VPHAL surface
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_RndrCommonGetBackVpSurfaceParams(
    PRENDERHAL_SURFACE      pRenderHalSurface,
    PVPHAL_SURFACE          pVpSurface);

//!
//! \brief    Set Surface for HW Access
//! \details  Common Function for setting up surface state, if render would 
//!           use CP HM, need use VpHal_CommonSetSurfaceForHwAccess instead
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [in] pSurface
//!           Pointer to Surface
//! \param    [in] pSurfaceParams
//!           Pointer to RenderHal Surface Params
//! \param    [in] iBindingTable
//!           Binding Table to bind surface
//! \param    [in] iBTEntry
//!           Binding Table Entry index
//! \param    [in] bWrite
//!           Write mode flag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS VpHal_RndrCommonSetSurfaceForHwAccess(
    PRENDERHAL_INTERFACE                pRenderHal,
    PVPHAL_SURFACE                      pSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS     pSurfaceParams,
    int32_t                             iBindingTable,
    int32_t                             iBTEntry,
    bool                                bWrite);

//!
//! \brief    Set Buffer Surface for HW Access
//! \details  Common Function for setting up buffer surface state, if render would 
//!           use CP HM, need use VpHal_CommonSetBufferSurfaceForHwAccess instead
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [in] pSurface
//!           Pointer to Surface
//! \param    [in,out] pSurfaceParams
//!           Pointer to RenderHal Surface Params
//! \param    [in] iBindingTable
//!           Binding Table to Bind Surface
//! \param    [in] iBTEntry
//!           Binding Table Entry index
//! \param    [in] bWrite
//!           Write mode flag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS VpHal_RndrCommonSetBufferSurfaceForHwAccess(
    PRENDERHAL_INTERFACE                pRenderHal,
    PVPHAL_SURFACE                      pSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS     pSurfaceParams,
    int32_t                             iBindingTable,
    int32_t                             iBTEntry,
    bool                                bWrite);

//!
//! \brief    Set Surface for HW Access for CP HM
//! \details  Common Function for setting up surface state, need to use this function
//!           if render would use CP HM
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [in] pSurface
//!           Pointer to Surface
//! \param    [in] pRenderSurface
//!           Pointer to Render Surface
//! \param    [in] pSurfaceParams
//!           Pointer to RenderHal Surface Params
//! \param    [in] iBindingTable
//!           Binding Table to bind surface
//! \param    [in] iBTEntry
//!           Binding Table Entry index
//! \param    [in] bWrite
//!           Write mode flag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS VpHal_CommonSetSurfaceForHwAccess(
    PRENDERHAL_INTERFACE                pRenderHal,
    PVPHAL_SURFACE                      pSurface,
    PRENDERHAL_SURFACE                  pRenderSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS     pSurfaceParams,
    int32_t                             iBindingTable,
    int32_t                             iBTEntry,
    bool                                bWrite);

//!
//! \brief    Set Buffer Surface for HW Access for CP HM
//! \details  Common Function for setting up buffer surface state, need to use this function
//!           if render would use CP HM
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [in] pSurface
//!           Pointer to Surface
//! \param    [in] pRenderSurface
//!           Pointer to Render Surface
//! \param    [in,out] pSurfaceParams
//!           Pointer to RenderHal Surface Params
//! \param    [in] iBindingTable
//!           Binding Table to Bind Surface
//! \param    [in] iBTEntry
//!           Binding Table Entry index
//! \param    [in] bWrite
//!           Write mode flag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS VpHal_CommonSetBufferSurfaceForHwAccess(
    PRENDERHAL_INTERFACE                pRenderHal,
    PVPHAL_SURFACE                      pSurface,
    PRENDERHAL_SURFACE                  pRenderSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS     pSurfaceParams,
    int32_t                             iBindingTable,
    int32_t                             iBTEntry,
    bool                                bWrite);

//!
//! \brief      Submit commands for rendering
//! \param      [in] pRenderHal
//!             Pointer to RenderHal Interface Structure
//! \param      [in] pBatchBuffer
//!             Pointer to batch buffer
//! \param      [in] bNullRendering
//!             Indicate whether is Null rendering
//! \param      [in] pWalkerParams
//!             Pointer to walker parameters
//! \param      [in] pGpGpuWalkerParams
//!             Pointer to GPGPU walker parameters
//! \param      [in] KernelID
//!             VP Kernel ID
//! \param      [in] bLastSubmission
//!             whether it is the last submission
//! \return     MOS_STATUS
//!
MOS_STATUS VpHal_RndrCommonSubmitCommands(
    PRENDERHAL_INTERFACE                pRenderHal,
    PMHW_BATCH_BUFFER                   pBatchBuffer,
    bool                                bNullRendering,
    PMHW_WALKER_PARAMS                  pWalkerParams,
    PMHW_GPGPU_WALKER_PARAMS            pGpGpuWalkerParams,
    VpKernelID                          KernelID,
    bool                                bLastSubmission);

//!
//! \brief      Submit commands for rendering
//! \details    Submit commands for rendering with status table update enabling
//! \param      [in] pRenderHal
//!             Pointer to RenderHal Interface Structure
//! \param      [in] pBatchBuffer
//!             Pointer to batch buffer
//! \param      [in] bNullRendering
//!             Indicate whether is Null rendering
//! \param      [in] pWalkerParams
//!             Pointer to walker parameters
//! \param      [in] pGpGpuWalkerParams
//!             Pointer to GPGPU walker parameters
//! \param      [in] pStatusTableUpdateParams
//!             Pointer to pStatusTableUpdateParams
//! \param      [in] KernelID
//!             VP Kernel ID
//! \param      [in] FcKernelCount
//!             VP FC Kernel Count
//! \param      [in] FcKernelList
//!             VP FC Kernel List
//! \param      [in] bLastSubmission
//!             whether it is the last submission
//! \return     MOS_STATUS
//!
MOS_STATUS VpHal_RndrSubmitCommands(
    PRENDERHAL_INTERFACE                pRenderHal,
    PMHW_BATCH_BUFFER                   pBatchBuffer,
    bool                                bNullRendering,
    PMHW_WALKER_PARAMS                  pWalkerParams,
    PMHW_GPGPU_WALKER_PARAMS            pGpGpuWalkerParams,
    PSTATUS_TABLE_UPDATE_PARAMS         pStatusTableUpdateParams,
    VpKernelID                          KernelID,
    int                                 FcKernelCount,
    int                                 *FcKernelList,
    bool                                bLastSubmission);

//!
//! \brief      Is Alignment WA needed
//! \details    Decide WA is needed for VEBOX/Render engine
//!             RENDER limitation with composition BOB:
//!             Height should be a multiple of 4, otherwise disable DI in Comp
//!             VEBOX limitation with TiledY NV12 input(Gen75):
//!             Height should be a multiple of 4, otherwise bypass adv render
//!             VEBOX limitation with TiledY NV12 input(Gen8/9):
//!             3D/GMM regresses to allocate linear surface when height is not
//!             a multiple of 4, no need to bypass adv render
//! \param      [in] pSurface
//!             Input surface
//! \param      [in] GpuContext
//!             GpuContext to indicate Render/Vebox
//! \return     bool
//!             true - Solution is needed; false - Solution is not needed
//!
bool VpHal_RndrCommonIsAlignmentWANeeded(
    PVPHAL_SURFACE             pSurface,
    MOS_GPU_CONTEXT            GpuContext);

//!
//! \brief      Set params for AVS table
//! \details    Set 4-tap or 8-tap filtering AVS table
//! \param      [in] pAvsParams
//!             Pointer to avs parameter
//! \param      [in,out] pMhwSamplerAvsTableParam
//!             Pointer to avs table parameter
//! \return     MOS_STATUS
//!
MOS_STATUS VpHal_RenderCommonSetAVSTableParam(
    PMHW_AVS_PARAMS              pAvsParams,
    PMHW_SAMPLER_AVS_TABLE_PARAM pMhwSamplerAvsTableParam);

//!
//! \brief    Initialize AVS parameters shared by Renderers
//! \details  Initialize the members of the AVS parameter and allocate memory for its coefficient tables
//! \param    [in,out] pAVS_Params
//!           Pointer to MHW AVS parameter
//! \param    [in] uiYCoeffTableSize
//!           Size of the Y coefficient table
//! \param    [in] uiUVCoeffTableSize
//!           Size of the UV coefficient table
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RndrCommonInitAVSParams(
    PMHW_AVS_PARAMS     pAVS_Params,
    uint32_t            uiYCoeffTableSize,
    uint32_t            uiUVCoeffTableSize);

//!
//! \brief    Destroy AVS parameters shared by Renderers
//! \details  Free the memory of AVS parameter's coefficient tables
//! \param    [in,out] pAVS_Params
//!           Pointer to VPHAL AVS parameter
//! \return   void
//!
void VpHal_RndrCommonDestroyAVSParams(
    PMHW_AVS_PARAMS   pAVS_Params);

//!
//! \brief    update status report rely on command buffer sync tag
//! \param    [in] pOsInterface
//!           pointer to os interface
//! \param    [in,out] pStatusTableUpdateParams
//!           pointer to STATUS_TABLE_UPDATE_PARAMS for updating status table
//! \param    [in] eMosGpuContext
//!           current mos contexnt enum
//! \param    [in] eLastStatus
//!           indicating last submition is successful or not
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RndrUpdateStatusTableAfterSubmit(
    PMOS_INTERFACE              pOsInterface,
    PSTATUS_TABLE_UPDATE_PARAMS pStatusTableUpdateParams,
    MOS_GPU_CONTEXT             eMosGpuContext,
    MOS_STATUS                  eLastStatus
    );

//!
//! \brief    Determine if the Batch Buffer End is needed to add in the end
//! \details  Detect platform OS and return the flag whether the Batch Buffer End is needed to add in the end
//! \param    [in] pOsInterface
//!           Pointer to MOS_INTERFACE
//! \return   bool
//!           The flag of adding Batch Buffer End
//!
bool VpHal_RndrCommonIsMiBBEndNeeded(
    PMOS_INTERFACE           pOsInterface);

struct AvsCoeffsCacheTag
{
    bool operator==(const AvsCoeffsCacheTag &rhs) const
    {
        return (this->m_format              == rhs.m_format              &&
                this->m_8TapAdaptiveEnable  == rhs.m_8TapAdaptiveEnable  &&
                this->m_balancedFilter      == rhs.m_balancedFilter      &&
                this->m_forcePolyPhaseCoefs == rhs.m_forcePolyPhaseCoefs &&
                this->m_chromaSiting        == rhs.m_chromaSiting        &&
                fabsf(this->m_scaleX - rhs.m_scaleX) < 1e-6              &&
                fabsf(this->m_scaleY - rhs.m_scaleY) < 1e-6);
    }

    MOS_FORMAT  m_format;
    bool        m_8TapAdaptiveEnable;
    bool        m_balancedFilter;
    bool        m_forcePolyPhaseCoefs;
    uint32_t    m_chromaSiting;
    float       m_scaleX;
    float       m_scaleY;
};

struct AvsCoeffsCacheEntry
{
    AvsCoeffsCacheTag m_tag;
    MHW_AVS_PARAMS    m_AvsParams;
    bool              m_valid;
};

template <int N>
class AvsCoeffsCache
{
public:
    AvsCoeffsCache():
        m_evictIndex(0),
        m_YCoeffTableSize(0),
        m_UVCoeffTableSize(0)
    {
        MOS_ZeroMemory(m_entries, sizeof(m_entries));
    }

    ~AvsCoeffsCache()
    {
        for (int i = 0; i < N; ++i)
        {
            VpHal_RndrCommonDestroyAVSParams(&m_entries[i].m_AvsParams);
        }
    }

    void Init(int YCoeffTableSize, int UVCoeffTableSize)
    {
        m_YCoeffTableSize  = YCoeffTableSize;
        m_UVCoeffTableSize = UVCoeffTableSize;

        for (int i = 0; i < N; i++)
        {
            VpHal_RndrCommonInitAVSParams(&m_entries[i].m_AvsParams,
                YCoeffTableSize,
                UVCoeffTableSize);
        }
    }

    const MHW_AVS_PARAMS* Find(const AvsCoeffsCacheTag &tag) const
    {
        for (int i = 0; i < N; i++)
        {
            if (m_entries[i].m_valid && m_entries[i].m_tag == tag)
            {
                return &m_entries[i].m_AvsParams;
            }
        }
        return nullptr;
    }

    void Insert(const AvsCoeffsCacheTag &tag, const MHW_AVS_PARAMS &params)
    {
        m_entries[m_evictIndex].m_tag = tag;
        Clone(params, m_entries[m_evictIndex].m_AvsParams);
        m_entries[m_evictIndex].m_valid = true;
        m_evictIndex = (m_evictIndex + 1) % N;
    }

    void Clone(const MHW_AVS_PARAMS &from, MHW_AVS_PARAMS &to)
    {
        to.Format               = from.Format;
        to.fScaleX              = from.fScaleX;
        to.fScaleY              = from.fScaleY;
        to.bForcePolyPhaseCoefs = from.bForcePolyPhaseCoefs;

        MOS_SecureMemcpy(to.piYCoefsX, m_YCoeffTableSize, from.piYCoefsX, m_YCoeffTableSize);
        MOS_SecureMemcpy(to.piYCoefsY, m_YCoeffTableSize, from.piYCoefsY, m_YCoeffTableSize);
        MOS_SecureMemcpy(to.piUVCoefsX, m_UVCoeffTableSize, from.piUVCoefsX, m_UVCoeffTableSize);
        MOS_SecureMemcpy(to.piUVCoefsY, m_UVCoeffTableSize, from.piUVCoefsY, m_UVCoeffTableSize);
    }

private:
    AvsCoeffsCacheEntry  m_entries[N];
    int                  m_evictIndex;
    int                  m_YCoeffTableSize;
    int                  m_UVCoeffTableSize;
};

#endif  // __VPHAL_RENDER_COMMON_H__
