/*
* Copyright (c) 2021, Intel Corporation
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
#ifndef __VP_RENDER_FC_TYPES_H__
#define __VP_RENDER_FC_TYPES_H__

namespace vp
{
//!< Linear sampler bias
#define VP_SAMPLER_BIAS         0.015625f
#define VP_HW_LINEAR_SHIFT      0.5f

// Defined DP used FC kernel for computation.
struct VP_FC_DP_BASED_CURBE_DATA
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
struct VP_FC_CURBE_DATA
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
        };

        uint32_t       Value;
    } DW10;

    // DWORD 11 - GRF R2.3
    union
    {
        struct
        {
            float       ChromasitingUOffset;               // Param for 3D Sampler use case
        };
        uint32_t        Value;
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
            uint32_t       Sampler3DStateSetSelection  : 8;
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

#define VP_MAX_PROCAMP              2

#define VP_COMP_SAMPLER_NEAREST     1
#define VP_COMP_SAMPLER_BILINEAR    2
#define VP_COMP_SAMPLER_LUMAKEY     4

#define VP_COMP_MAX_LAYERS          8
#define VP_COMP_MAX_PALETTES        2
#define VP_COMP_MAX_PROCAMP         1
#define VP_COMP_MAX_LUMA_KEY        1
#define VP_COMP_MAX_AVS             1
#define VP_COMP_MAX_SAMPLER         (VP_COMP_SAMPLER_NEAREST | VP_COMP_SAMPLER_BILINEAR | VP_COMP_SAMPLER_LUMAKEY)

C_ASSERT(VP_MAX_PROCAMP >= VP_COMP_MAX_PROCAMP);

struct VP_LAYER_CALCULATED_PARAMS
{
    uint16_t alpha = 255;
    float fScaleX = 0;
    float fScaleY = 0;
    float fOffsetY = 0;
    float fOffsetX = 0;
    float fShiftX = 0;
    float fShiftY = 0;

    RECT clipedDstRect = {}; // Clipped dest rectangle
    bool isChromaUpSamplingNeeded = false;
    bool isChromaDownSamplingNeeded = false;
    MHW_SAMPLER_FILTER_MODE samplerFilterMode = MHW_SAMPLER_FILTER_MODE::MHW_SAMPLER_FILTER_NEAREST;

    bool chromaSitingEnabled = false;      //!<  Chromasiting flag
};

// Need be gotten with surface entry.
struct VP_LAYER_CALCULATED_PARAMS2
{
    float fStepX = 0;
    float fStepY = 0;
    float fOriginX = 0;
    float fOriginY = 0;
};

struct VP_FC_LAYER
{
    VP_SURFACE              *surf;                      //!< rcDst in surf is the one with rotation, which is different from the rcDst in SwfilterScaling
    int32_t                 layerID;
    int32_t                 layerIDOrigin;              //!< Origin layerID before layerSkipped, which can be used to reference surfaces in SurfaceGroup.
    VPHAL_SCALING_MODE      scalingMode;
    bool                    iefEnabled;
    bool                    iscalingEnabled;
    VPHAL_ROTATION          rotation;
    bool                    useSampleUnorm = false;     //!<  true: sample unorm is used, false: DScaler or AVS is used.
    bool                    useSamplerLumakey;          //!< Disabled on Gen12
    bool                    fieldWeaving;
    int32_t                 paletteID = 0;              //!<Palette Allocation
    bool                    queryVariance;
    bool                    xorComp = false;            //!< is mono-chroma composite mode.
    VP_SURFACE              *surfField = nullptr;       //!< For SurfaceTypeFcInputLayer0Field1Dual during iscaling and fieldWeaving.

    // Filled by hwFilter
    VP_LAYER_CALCULATED_PARAMS calculatedParams = {};   //!< Only valid in source.
    // Filled by packet
    VP_LAYER_CALCULATED_PARAMS2 calculatedParams2 = {}; //!< calcualted parameters which need be normalized by surface entry.

    // Need be initialized during SetupSurfaceState.
    PRENDERHAL_SURFACE_STATE_ENTRY  surfaceEntries[MHW_MAX_SURFACE_PLANES] = {};
    uint32_t                        numOfSurfaceEntries = 0;

    PVPHAL_DI_PARAMS        diParams;
    PVPHAL_LUMAKEY_PARAMS   lumaKeyParams;
    PVPHAL_BLENDING_PARAMS  blendingParams;
    PVPHAL_PROCAMP_PARAMS   procampParams;
};

#define VP_COMP_MAX_LAYERS      8
#define VP_MAX_TARGETS          8        //!< multi output support

//!
//! \brief Structure to VPHAL Composite Parameters
//!
struct VP_COMPOSITE_PARAMS
{
    // Pointer to target and source surfaces
    uint32_t                sourceCount;                       //!< Number of sources
    VP_FC_LAYER             source[VP_COMP_MAX_LAYERS];
    uint32_t                targetCount;                       //!< Number of targets
    VP_FC_LAYER             target[VPHAL_MAX_TARGETS];         //!< Render targets
    // Needed by CP during MHW VP integration, due to pTokenState->pResourceInfo
    //RENDERHAL_SURFACE       renderHalSurfaceSrc[VP_COMP_MAX_LAYERS];
    //RENDERHAL_SURFACE       renderHalSurfaceSrcField[VP_COMP_MAX_LAYERS];
    //RENDERHAL_SURFACE       renderHalSurfaceTarget[VP_MAX_TARGETS];

    PVPHAL_COLORFILL_PARAMS pColorFillParams;     //!< ColorFill - BG only
    PVPHAL_ALPHA_PARAMS     pCompAlpha;           //!< Alpha for composited surface
    bool                    bAlphaCalculateEnable; //!< Alpha Calculation flag

    // Resource counters
    struct
    {
        int32_t                 layers;
        int32_t                 palettes;
        int32_t                 avs;
        int32_t                 procamp;
        int32_t                 lumaKeys;
        int32_t                 sampler;
    } resCounter;
    //VPHAL_ROTATION          rotation;           //!< Layer 0 rotation info
};

class VpCompositeParamsPool
{
public:
    VpCompositeParamsPool()
    {
    }
    virtual ~VpCompositeParamsPool()
    {
    }

    VP_COMPOSITE_PARAMS *Assign()
    {
        VP_COMPOSITE_PARAMS *p = nullptr;
        if (0 == m_idle.size())
        {
            m_pool.push_back(VP_COMPOSITE_PARAMS());
            p = &m_pool[m_pool.size() - 1];
        }
        else
        {
            p = m_idle.begin()->second;
            m_idle.erase(p);
        }
        m_inuse.insert(std::make_pair(p, p));
        return p;
    }

    void Release(VP_COMPOSITE_PARAMS *&p)
    {
        if (nullptr == p || m_inuse.end() == m_inuse.find(p))
        {
            return;
        }
        m_inuse.erase(p);
        m_idle.insert(std::make_pair(p, p));
    }

private:
    std::vector<VP_COMPOSITE_PARAMS> m_pool;
    std::map<VP_COMPOSITE_PARAMS *, VP_COMPOSITE_PARAMS *> m_inuse;
    std::map<VP_COMPOSITE_PARAMS *, VP_COMPOSITE_PARAMS *> m_idle;

MEDIA_CLASS_DEFINE_END(vp__VpCompositeParamsPool)
};

}
#endif // !__VP_RENDER_FC_TYPES_H__
