/*
* Copyright (c) 2022, Intel Corporation
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
#ifndef __VP_RENDER_HDR_CMD_PACKET_EXT_H__
#define __VP_RENDER_HDR_CMD_PACKET_EXT_H__

#include "vp_platform_interface.h"
#include "vp_render_kernel_obj.h"
#include "vp_render_cmd_packet.h"

namespace vp {
// Static Data for HDR kernel
typedef struct _MEDIA_WALKER_HDR_STATIC_DATA
{
    // uint32_t 0 - GRF R1.0
    union
    {
        struct {
            float HorizontalFrameOriginLayer0;
        };
        float       Value;
    } DW0;

    // uint32_t 1 - GRF R1.1
    union
    {
        struct {
            float HorizontalFrameOriginLayer1;
        };
        float       Value;
    } DW1;

    // uint32_t 2 - GRF R1.2
    union
    {
        struct {
            float HorizontalFrameOriginLayer2;
        };
        float       Value;
    } DW2;

    // uint32_t 3 - GRF R1.3
    union
    {
        struct {
            float HorizontalFrameOriginLayer3;
        };
        float       Value;
    } DW3;

    // uint32_t 4 - GRF R1.4
    union
    {
        struct {
            float HorizontalFrameOriginLayer4;
        };
        float       Value;
    } DW4;

    // uint32_t 5 - GRF R1.5
    union
    {
        struct {
            float HorizontalFrameOriginLayer5;
        };
        float       Value;
    } DW5;

    // uint32_t 6 - GRF R1.6
    union
    {
        struct {
            float HorizontalFrameOriginLayer6;
        };
        float       Value;
    } DW6;

    // uint32_t 7 - GRF R1.7
    union
    {
        struct {
            float HorizontalFrameOriginLayer7;
        };
        float       Value;
    } DW7;

    // uint32_t 8 - GRF R2.0
    union
    {
        struct {
            float VerticalFrameOriginLayer0;
        };
        float       Value;
    } DW8;

    // uint32_t 9 - GRF R2.1
    union
    {
        struct {
            float VerticalFrameOriginLayer1;
        };
        float       Value;
    } DW9;

    // uint32_t 10 - GRF R2.2
    union
    {
        struct {
            float VerticalFrameOriginLayer2;
        };
        float       Value;
    } DW10;

    // uint32_t 11 - GRF R2.3
    union
    {
        struct {
            float VerticalFrameOriginLayer3;
        };
        float       Value;
    } DW11;

    // uint32_t 12 - GRF R2.4
    union
    {
        struct {
            float VerticalFrameOriginLayer4;
        };
        float       Value;
    } DW12;

    // uint32_t 13 - GRF R2.5
    union
    {
        struct {
            float VerticalFrameOriginLayer5;
        };
        float       Value;
    } DW13;

    // uint32_t 14 - GRF R2.6
    union
    {
        struct {
            float VerticalFrameOriginLayer6;
        };
        float       Value;
    } DW14;

    // uint32_t 15 - GRF R2.7
    union
    {
        struct {
            float VerticalFrameOriginLayer7;
        };
        float       Value;
    } DW15;

    // uint32_t 16 - GRF R3.0
    union
    {
        struct {
            float HorizontalScalingStepRatioLayer0;
        };
        float       Value;
    } DW16;

    // uint32_t 17 - GRF R3.1
    union
    {
        struct {
            float HorizontalScalingStepRatioLayer1;
        };
        float       Value;
    } DW17;

    // uint32_t 18 - GRF R3.2
    union
    {
        struct {
            float HorizontalScalingStepRatioLayer2;
        };
        float       Value;
    } DW18;

    // uint32_t 19 - GRF R3.3
    union
    {
        struct {
            float HorizontalScalingStepRatioLayer3;
        };
        float       Value;
    } DW19;

    // uint32_t 20 - GRF R3.4
    union
    {
        struct {
            float HorizontalScalingStepRatioLayer4;
        };
        float       Value;
    } DW20;

    // uint32_t 21 - GRF R3.5
    union
    {
        struct {
            float HorizontalScalingStepRatioLayer5;
        };
        float       Value;
    } DW21;

    // uint32_t 22 - GRF R3.6
    union
    {
        struct {
            float HorizontalScalingStepRatioLayer6;
        };
        float       Value;
    } DW22;

    // uint32_t 23 - GRF R3.7
    union
    {
        struct {
            float HorizontalScalingStepRatioLayer7;
        };
        float       Value;
    } DW23;

    // uint32_t 24 - GRF R4.0
    union
    {
        struct {
            float VerticalScalingStepRatioLayer0;
        };
        float       Value;
    } DW24;

    // uint32_t 25 - GRF R4.1
    union
    {
        struct {
            float VerticalScalingStepRatioLayer1;
        };
        float       Value;
    } DW25;

    // uint32_t 26 - GRF R4.2
    union
    {
        struct {
            float VerticalScalingStepRatioLayer2;
        };
        float       Value;
    } DW26;

    // uint32_t 27 - GRF R4.3
    union
    {
        struct {
            float VerticalScalingStepRatioLayer3;
        };
        float       Value;
    } DW27;

    // uint32_t 28 - GRF R4.4
    union
    {
        struct {
            float VerticalScalingStepRatioLayer4;
        };
        float       Value;
    } DW28;

    // uint32_t 29 - GRF R4.5
    union
    {
        struct {
            float VerticalScalingStepRatioLayer5;
        };
        float       Value;
    } DW29;

    // uint32_t 30 - GRF R4.6
    union
    {
        struct {
            float VerticalScalingStepRatioLayer6;
        };
        float       Value;
    } DW30;

    // uint32_t 31 - GRF R4.7
    union
    {
        struct {
            float VerticalScalingStepRatioLayer7;
        };
        float       Value;
    } DW31;

    // uint32_t 32 - GRF R5.0
    union
    {
        struct {
            uint32_t LeftCoordinateRectangleLayer0 : BITFIELD_RANGE(  0,15 );
            uint32_t TopCoordinateRectangleLayer0  : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW32;

    // uint32_t 33 - GRF R5.1
    union
    {
        struct {
            uint32_t LeftCoordinateRectangleLayer1 : BITFIELD_RANGE(  0,15 );
            uint32_t TopCoordinateRectangleLayer1  : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW33;

    // uint32_t 34 - GRF R5.2
    union
    {
        struct {
            uint32_t LeftCoordinateRectangleLayer2 : BITFIELD_RANGE(  0,15 );
            uint32_t TopCoordinateRectangleLayer2  : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW34;

    // uint32_t 35 - GRF R5.3
    union
    {
        struct {
            uint32_t LeftCoordinateRectangleLayer3 : BITFIELD_RANGE(  0,15 );
            uint32_t TopCoordinateRectangleLayer3  : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW35;

    // uint32_t 36 - GRF R5.4
    union
    {
        struct {
            uint32_t LeftCoordinateRectangleLayer4 : BITFIELD_RANGE(  0,15 );
            uint32_t TopCoordinateRectangleLayer4  : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW36;

    // uint32_t 37 - GRF R5.5
    union
    {
        struct {
            uint32_t LeftCoordinateRectangleLayer5 : BITFIELD_RANGE(  0,15 );
            uint32_t TopCoordinateRectangleLayer5  : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW37;

    // uint32_t 38 - GRF R5.6
    union
    {
        struct {
            uint32_t LeftCoordinateRectangleLayer6 : BITFIELD_RANGE(  0,15 );
            uint32_t TopCoordinateRectangleLayer6  : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW38;

    // uint32_t 39 - GRF R5.7
    union
    {
        struct {
            uint32_t LeftCoordinateRectangleLayer7 : BITFIELD_RANGE(  0,15 );
            uint32_t TopCoordinateRectangleLayer7  : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW39;

    // uint32_t 40 - GRF R6.0
    union
    {
        struct {
            uint32_t RightCoordinateRectangleLayer0  : BITFIELD_RANGE(  0,15 );
            uint32_t BottomCoordinateRectangleLayer0 : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW40;

    // uint32_t 41 - GRF R6.1
    union
    {
        struct {
            uint32_t RightCoordinateRectangleLayer1  : BITFIELD_RANGE(  0,15 );
            uint32_t BottomCoordinateRectangleLayer1 : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW41;

    // uint32_t 42 - GRF R6.2
    union
    {
        struct {
            uint32_t RightCoordinateRectangleLayer2  : BITFIELD_RANGE(  0,15 );
            uint32_t BottomCoordinateRectangleLayer2 : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW42;

    // uint32_t 43 - GRF R6.3
    union
    {
        struct {
            uint32_t RightCoordinateRectangleLayer3  : BITFIELD_RANGE(  0,15 );
            uint32_t BottomCoordinateRectangleLayer3 : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW43;

    // uint32_t 44 - GRF R6.4
    union
    {
        struct {
            uint32_t RightCoordinateRectangleLayer4  : BITFIELD_RANGE(  0,15 );
            uint32_t BottomCoordinateRectangleLayer4 : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW44;

    // uint32_t 45 - GRF R6.5
    union
    {
        struct {
            uint32_t RightCoordinateRectangleLayer5  : BITFIELD_RANGE(  0,15 );
            uint32_t BottomCoordinateRectangleLayer5 : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW45;

    // uint32_t 46 - GRF R6.6
    union
    {
        struct {
            uint32_t RightCoordinateRectangleLayer6  : BITFIELD_RANGE(  0,15 );
            uint32_t BottomCoordinateRectangleLayer6 : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW46;

    // uint32_t 47 - GRF R6.7
    union
    {
        struct {
            uint32_t RightCoordinateRectangleLayer7  : BITFIELD_RANGE(  0,15 );
            uint32_t BottomCoordinateRectangleLayer7 : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW47;

    // uint32_t 48 - GRF R7.0
    union
    {
        struct {
            uint32_t FormatDescriptorLayer0                : BITFIELD_RANGE(  0,7  );
            uint32_t ChromaSittingLocationLayer0           : BITFIELD_RANGE(  8,10 );
            uint32_t ChannelSwapEnablingFlagLayer0         : BITFIELD_RANGE( 11,11 );
            uint32_t IEFBypassEnablingFlagLayer0           : BITFIELD_RANGE( 12,12 );
            uint32_t RotationAngleMirrorDirectionLayer0    : BITFIELD_RANGE( 13,15 );
            uint32_t SamplerIndexFirstPlaneLayer0          : BITFIELD_RANGE( 16,19 );
            uint32_t SamplerIndexSecondThirdPlaneLayer0    : BITFIELD_RANGE( 20,23 );
            uint32_t CCMExtensionEnablingFlagLayer0        : BITFIELD_RANGE( 24,24 );
            uint32_t ToneMappingEnablingFlagLayer0         : BITFIELD_RANGE( 25,25 );
            uint32_t PriorCSCEnablingFlagLayer0            : BITFIELD_RANGE( 26,26 );
            uint32_t EOTF1DLUTEnablingFlagLayer0           : BITFIELD_RANGE( 27,27 );
            uint32_t CCMEnablingFlagLayer0                 : BITFIELD_RANGE( 28,28 );
            uint32_t OETF1DLUTEnablingFlagLayer0           : BITFIELD_RANGE( 29,29 );
            uint32_t PostCSCEnablingFlagLayer0             : BITFIELD_RANGE( 30,30 );
            uint32_t Enabling3DLUTFlagLayer0               : BITFIELD_RANGE( 31,31 );
        };
        uint32_t       Value;
    } DW48;

    // uint32_t 49 - GRF R7.1
    union
    {
        struct {
            uint32_t FormatDescriptorLayer1                : BITFIELD_RANGE(  0,7  );
            uint32_t ChromaSittingLocationLayer1           : BITFIELD_RANGE(  8,10 );
            uint32_t ChannelSwapEnablingFlagLayer1         : BITFIELD_RANGE( 11,11 );
            uint32_t IEFBypassEnablingFlagLayer1           : BITFIELD_RANGE( 12,12 );
            uint32_t RotationAngleMirrorDirectionLayer1    : BITFIELD_RANGE( 13,15 );
            uint32_t SamplerIndexFirstPlaneLayer1          : BITFIELD_RANGE( 16,19 );
            uint32_t SamplerIndexSecondThirdPlaneLayer1    : BITFIELD_RANGE( 20,23 );
            uint32_t CCMExtensionEnablingFlagLayer1        : BITFIELD_RANGE( 24,24 );
            uint32_t ToneMappingEnablingFlagLayer1         : BITFIELD_RANGE( 25,25 );
            uint32_t PriorCSCEnablingFlagLayer1            : BITFIELD_RANGE( 26,26 );
            uint32_t EOTF1DLUTEnablingFlagLayer1           : BITFIELD_RANGE( 27,27 );
            uint32_t CCMEnablingFlagLayer1                 : BITFIELD_RANGE( 28,28 );
            uint32_t OETF1DLUTEnablingFlagLayer1           : BITFIELD_RANGE( 29,29 );
            uint32_t PostCSCEnablingFlagLayer1             : BITFIELD_RANGE( 30,30 );
            uint32_t Enabling3DLUTFlagLayer1               : BITFIELD_RANGE( 31,31 );
        };
        uint32_t       Value;
    } DW49;

    // uint32_t 50 - GRF R7.2
    union
    {
        struct {
            uint32_t FormatDescriptorLayer2                : BITFIELD_RANGE(  0,7  );
            uint32_t ChromaSittingLocationLayer2           : BITFIELD_RANGE(  8,10 );
            uint32_t ChannelSwapEnablingFlagLayer2         : BITFIELD_RANGE( 11,11 );
            uint32_t IEFBypassEnablingFlagLayer2           : BITFIELD_RANGE( 12,12 );
            uint32_t RotationAngleMirrorDirectionLayer2    : BITFIELD_RANGE( 13,15 );
            uint32_t SamplerIndexFirstPlaneLayer2          : BITFIELD_RANGE( 16,19 );
            uint32_t SamplerIndexSecondThirdPlaneLayer2    : BITFIELD_RANGE( 20,23 );
            uint32_t CCMExtensionEnablingFlagLayer2        : BITFIELD_RANGE( 24,24 );
            uint32_t ToneMappingEnablingFlagLayer2         : BITFIELD_RANGE( 25,25 );
            uint32_t PriorCSCEnablingFlagLayer2            : BITFIELD_RANGE( 26,26 );
            uint32_t EOTF1DLUTEnablingFlagLayer2           : BITFIELD_RANGE( 27,27 );
            uint32_t CCMEnablingFlagLayer2                 : BITFIELD_RANGE( 28,28 );
            uint32_t OETF1DLUTEnablingFlagLayer2           : BITFIELD_RANGE( 29,29 );
            uint32_t PostCSCEnablingFlagLayer2             : BITFIELD_RANGE( 30,30 );
            uint32_t Enabling3DLUTFlagLayer2               : BITFIELD_RANGE( 31,31 );
        };
        uint32_t       Value;
    } DW50;

    // uint32_t 51 - GRF R7.3
    union
    {
        struct {
            uint32_t FormatDescriptorLayer3                : BITFIELD_RANGE(  0,7  );
            uint32_t ChromaSittingLocationLayer3           : BITFIELD_RANGE(  8,10 );
            uint32_t ChannelSwapEnablingFlagLayer3         : BITFIELD_RANGE( 11,11 );
            uint32_t IEFBypassEnablingFlagLayer3           : BITFIELD_RANGE( 12,12 );
            uint32_t RotationAngleMirrorDirectionLayer3    : BITFIELD_RANGE( 13,15 );
            uint32_t SamplerIndexFirstPlaneLayer3          : BITFIELD_RANGE( 16,19 );
            uint32_t SamplerIndexSecondThirdPlaneLayer3    : BITFIELD_RANGE( 20,23 );
            uint32_t CCMExtensionEnablingFlagLayer3        : BITFIELD_RANGE( 24,24 );
            uint32_t ToneMappingEnablingFlagLayer3         : BITFIELD_RANGE( 25,25 );
            uint32_t PriorCSCEnablingFlagLayer3            : BITFIELD_RANGE( 26,26 );
            uint32_t EOTF1DLUTEnablingFlagLayer3           : BITFIELD_RANGE( 27,27 );
            uint32_t CCMEnablingFlagLayer3                 : BITFIELD_RANGE( 28,28 );
            uint32_t OETF1DLUTEnablingFlagLayer3           : BITFIELD_RANGE( 29,29 );
            uint32_t PostCSCEnablingFlagLayer3             : BITFIELD_RANGE( 30,30 );
            uint32_t Enabling3DLUTFlagLayer3               : BITFIELD_RANGE( 31,31 );
        };
        uint32_t       Value;
    } DW51;

    // uint32_t 52 - GRF R7.4
    union
    {
        struct {
            uint32_t FormatDescriptorLayer4                : BITFIELD_RANGE(  0,7  );
            uint32_t ChromaSittingLocationLayer4           : BITFIELD_RANGE(  8,10 );
            uint32_t ChannelSwapEnablingFlagLayer4         : BITFIELD_RANGE( 11,11 );
            uint32_t IEFBypassEnablingFlagLayer4           : BITFIELD_RANGE( 12,12 );
            uint32_t RotationAngleMirrorDirectionLayer4    : BITFIELD_RANGE( 13,15 );
            uint32_t SamplerIndexFirstPlaneLayer4          : BITFIELD_RANGE( 16,19 );
            uint32_t SamplerIndexSecondThirdPlaneLayer4    : BITFIELD_RANGE( 20,23 );
            uint32_t CCMExtensionEnablingFlagLayer4        : BITFIELD_RANGE( 24,24 );
            uint32_t ToneMappingEnablingFlagLayer4         : BITFIELD_RANGE( 25,25 );
            uint32_t PriorCSCEnablingFlagLayer4            : BITFIELD_RANGE( 26,26 );
            uint32_t EOTF1DLUTEnablingFlagLayer4           : BITFIELD_RANGE( 27,27 );
            uint32_t CCMEnablingFlagLayer4                 : BITFIELD_RANGE( 28,28 );
            uint32_t OETF1DLUTEnablingFlagLayer4           : BITFIELD_RANGE( 29,29 );
            uint32_t PostCSCEnablingFlagLayer4             : BITFIELD_RANGE( 30,30 );
            uint32_t Enabling3DLUTFlagLayer4               : BITFIELD_RANGE( 31,31 );
        };
        uint32_t       Value;
    } DW52;

    // uint32_t 53 - GRF R7.5
    union
    {
        struct {
            uint32_t FormatDescriptorLayer5                : BITFIELD_RANGE(  0,7  );
            uint32_t ChromaSittingLocationLayer5           : BITFIELD_RANGE(  8,10 );
            uint32_t ChannelSwapEnablingFlagLayer5         : BITFIELD_RANGE( 11,11 );
            uint32_t IEFBypassEnablingFlagLayer5           : BITFIELD_RANGE( 12,12 );
            uint32_t RotationAngleMirrorDirectionLayer5    : BITFIELD_RANGE( 13,15 );
            uint32_t SamplerIndexFirstPlaneLayer5          : BITFIELD_RANGE( 16,19 );
            uint32_t SamplerIndexSecondThirdPlaneLayer5    : BITFIELD_RANGE( 20,23 );
            uint32_t CCMExtensionEnablingFlagLayer5        : BITFIELD_RANGE( 24,24 );
            uint32_t ToneMappingEnablingFlagLayer5         : BITFIELD_RANGE( 25,25 );
            uint32_t PriorCSCEnablingFlagLayer5            : BITFIELD_RANGE( 26,26 );
            uint32_t EOTF1DLUTEnablingFlagLayer5           : BITFIELD_RANGE( 27,27 );
            uint32_t CCMEnablingFlagLayer5                 : BITFIELD_RANGE( 28,28 );
            uint32_t OETF1DLUTEnablingFlagLayer5           : BITFIELD_RANGE( 29,29 );
            uint32_t PostCSCEnablingFlagLayer5             : BITFIELD_RANGE( 30,30 );
            uint32_t Enabling3DLUTFlagLayer5               : BITFIELD_RANGE( 31,31 );
        };
        uint32_t       Value;
    } DW53;

    // uint32_t 54 - GRF R7.6
    union
    {
        struct {
            uint32_t FormatDescriptorLayer6                : BITFIELD_RANGE(  0,7  );
            uint32_t ChromaSittingLocationLayer6           : BITFIELD_RANGE(  8,10 );
            uint32_t ChannelSwapEnablingFlagLayer6         : BITFIELD_RANGE( 11,11 );
            uint32_t IEFBypassEnablingFlagLayer6           : BITFIELD_RANGE( 12,12 );
            uint32_t RotationAngleMirrorDirectionLayer6    : BITFIELD_RANGE( 13,15 );
            uint32_t SamplerIndexFirstPlaneLayer6          : BITFIELD_RANGE( 16,19 );
            uint32_t SamplerIndexSecondThirdPlaneLayer6    : BITFIELD_RANGE( 20,23 );
            uint32_t CCMExtensionEnablingFlagLayer6        : BITFIELD_RANGE( 24,24 );
            uint32_t ToneMappingEnablingFlagLayer6         : BITFIELD_RANGE( 25,25 );
            uint32_t PriorCSCEnablingFlagLayer6            : BITFIELD_RANGE( 26,26 );
            uint32_t EOTF1DLUTEnablingFlagLayer6           : BITFIELD_RANGE( 27,27 );
            uint32_t CCMEnablingFlagLayer6                 : BITFIELD_RANGE( 28,28 );
            uint32_t OETF1DLUTEnablingFlagLayer6           : BITFIELD_RANGE( 29,29 );
            uint32_t PostCSCEnablingFlagLayer6             : BITFIELD_RANGE( 30,30 );
            uint32_t Enabling3DLUTFlagLayer6               : BITFIELD_RANGE( 31,31 );
        };
        uint32_t       Value;
    } DW54;

    // uint32_t 55 - GRF R7.7
    union
    {
        struct {
            uint32_t FormatDescriptorLayer7                : BITFIELD_RANGE(  0,7  );
            uint32_t ChromaSittingLocationLayer7           : BITFIELD_RANGE(  8,10 );
            uint32_t ChannelSwapEnablingFlagLayer7         : BITFIELD_RANGE( 11,11 );
            uint32_t IEFBypassEnablingFlagLayer7           : BITFIELD_RANGE( 12,12 );
            uint32_t RotationAngleMirrorDirectionLayer7    : BITFIELD_RANGE( 13,15 );
            uint32_t SamplerIndexFirstPlaneLayer7          : BITFIELD_RANGE( 16,19 );
            uint32_t SamplerIndexSecondThirdPlaneLayer7    : BITFIELD_RANGE( 20,23 );
            uint32_t CCMExtensionEnablingFlagLayer7        : BITFIELD_RANGE( 24,24 );
            uint32_t ToneMappingEnablingFlagLayer7         : BITFIELD_RANGE( 25,25 );
            uint32_t PriorCSCEnablingFlagLayer7            : BITFIELD_RANGE( 26,26 );
            uint32_t EOTF1DLUTEnablingFlagLayer7           : BITFIELD_RANGE( 27,27 );
            uint32_t CCMEnablingFlagLayer7                 : BITFIELD_RANGE( 28,28 );
            uint32_t OETF1DLUTEnablingFlagLayer7           : BITFIELD_RANGE( 29,29 );
            uint32_t PostCSCEnablingFlagLayer7             : BITFIELD_RANGE( 30,30 );
            uint32_t Enabling3DLUTFlagLayer7               : BITFIELD_RANGE( 31,31 );
        };
        uint32_t       Value;
    } DW55;

    // uint32_t 56 - GRF R8.0
    union
    {
        struct {
            uint32_t ConstantBlendingAlphaFillColorLayer0  : BITFIELD_RANGE(  0,7  );
            uint32_t ConstantBlendingAlphaFillColorLayer1  : BITFIELD_RANGE(  8,15 );
            uint32_t ConstantBlendingAlphaFillColorLayer2  : BITFIELD_RANGE( 16,23 );
            uint32_t ConstantBlendingAlphaFillColorLayer3  : BITFIELD_RANGE( 24,31 );
        };
        uint32_t       Value;
    } DW56;

    // uint32_t 57 - GRF R8.1
    union
    {
        struct {
            uint32_t ConstantBlendingAlphaFillColorLayer4  : BITFIELD_RANGE(  0,7  );
            uint32_t ConstantBlendingAlphaFillColorLayer5  : BITFIELD_RANGE(  8,15 );
            uint32_t ConstantBlendingAlphaFillColorLayer6  : BITFIELD_RANGE( 16,23 );
            uint32_t ConstantBlendingAlphaFillColorLayer7  : BITFIELD_RANGE( 24,31 );
        };
        uint32_t       Value;
    } DW57;

    // uint32_t 58 - GRF R8.2
    union
    {
        struct {
            uint32_t TwoLayerOperationLayer0 : BITFIELD_RANGE(  0,7  );
            uint32_t TwoLayerOperationLayer1 : BITFIELD_RANGE(  8,15 );
            uint32_t TwoLayerOperationLayer2 : BITFIELD_RANGE( 16,23 );
            uint32_t TwoLayerOperationLayer3 : BITFIELD_RANGE( 24,31 );
        };
        uint32_t       Value;
    } DW58;

    // uint32_t 59 - GRF R8.3
    union
    {
        struct {
            uint32_t TwoLayerOperationLayer4 : BITFIELD_RANGE(  0,7  );
            uint32_t TwoLayerOperationLayer5 : BITFIELD_RANGE(  8,15 );
            uint32_t TwoLayerOperationLayer6 : BITFIELD_RANGE( 16,23 );
            uint32_t TwoLayerOperationLayer7 : BITFIELD_RANGE( 24,31 );
        };
        uint32_t       Value;
    } DW59;

    // uint32_t 60 - GRF R8.4
    union
    {
        struct {
            uint32_t FixedPointFillColorRVChannel : BITFIELD_RANGE(  0,15 );
            uint32_t FixedPointFillColorGYChannel : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW60;

    // uint32_t 61 - GRF R8.5
    union
    {
        struct {
            uint32_t FixedPointFillColorBUChannel    : BITFIELD_RANGE(  0,15 );
            uint32_t FixedPointFillColorAlphaChannel : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW61;

    // uint32_t 62 - GRF R8.6
    union
    {
        struct {
            uint32_t DestinationWidth  : BITFIELD_RANGE(  0,15 );
            uint32_t DestinationHeight : BITFIELD_RANGE( 16,31 );
        };
        uint32_t       Value;
    } DW62;

    // uint32_t 63 - GRF R8.7
    union
    {
        struct {
            uint32_t TotalNumberInputLayers                    : BITFIELD_RANGE(  0,15 );
            uint32_t FormatDescriptorDestination               : BITFIELD_RANGE( 16,23 );
            uint32_t ChromaSittingLocationDestination          : BITFIELD_RANGE( 24,26 );
            uint32_t ChannelSwapEnablingFlagDestination        : BITFIELD_RANGE( 27,27 );
            uint32_t DstCSCEnablingFlagDestination             : BITFIELD_RANGE( 28,28 );
            uint32_t Reserved                                  : BITFIELD_RANGE( 29,29 );
            uint32_t DitherRoundEnablingFlagDestinationSurface : BITFIELD_RANGE( 30,31 );
        };
        uint32_t       Value;
    } DW63;
} MEDIA_WALKER_HDR_STATIC_DATA, * PMEDIA_WALKER_HDR_STATIC_DATA;
C_ASSERT(SIZE32(MEDIA_WALKER_HDR_STATIC_DATA) == 64);

//!
//! \brief HDR Format Descriptor enum
//!
typedef enum _VPHAL_HDR_FORMAT_DESCRIPTOR
{
    VPHAL_HDR_FORMAT_DESCRIPTOR_UNKNOW = -1,
    VPHAL_HDR_FORMAT_R16G16B16A16_FLOAT = 44,
    VPHAL_HDR_FORMAT_DESCRIPTOR_R16G16_UNORM = 60,
    VPHAL_HDR_FORMAT_DESCRIPTOR_R16_UNORM = 70,
    VPHAL_HDR_FORMAT_DESCRIPTOR_R10G10B10A2_UNORM = 89,
    VPHAL_HDR_FORMAT_DESCRIPTOR_R8G8B8A8_UNORM = 101,
    VPHAL_HDR_FORMAT_DESCRIPTOR_YUY2 = 201,
    VPHAL_HDR_FORMAT_DESCRIPTOR_NV12 = 220,
    VPHAL_HDR_FORMAT_DESCRIPTOR_P010 = 222,
    VPHAL_HDR_FORMAT_DESCRIPTOR_P016 = 223
} VPHAL_HDR_FORMAT_DESCRIPTOR;

//!
//! \brief HDR Chroma Siting enum
//!
typedef enum _VPHAL_HDR_CHROMA_SITING
{
    VPHAL_HDR_CHROMA_SITTING_A = 0, // Sample even index at even line
    VPHAL_HDR_CHROMA_SITTING_B,     // Sample even index at odd line
    VPHAL_HDR_CHROMA_SITTING_AC,    // Average consistent even index and odd index at even line
    VPHAL_HDR_CHROMA_SITTING_BD,    // Average consistent even index and odd index at odd line
    VPHAL_HDR_CHROMA_SITTING_AB,    // Average even index of even line and even index of odd line
    VPHAL_HDR_CHROMA_SITTING_ABCD   // Average even and odd index at even line and odd line
} VPHAL_HDR_CHROMA_SITING;

//!
//! \brief HDR Rotation enum
//!
typedef enum _VPHAL_HDR_ROTATION
{
    VPHAL_HDR_LAYER_ROTATION_0 = 0, // 0 degree rotation
    VPHAL_HDR_LAYER_ROTATION_90,     // 90 degree CW rotation
    VPHAL_HDR_LAYER_ROTATION_180,    // 180 degree rotation
    VPHAL_HDR_LAYER_ROTATION_270,    // 270 degree CW rotation
    VPHAL_HDR_LAYER_MIRROR_H,        // 0 degree rotation then mirror horizontally
    VPHAL_HDR_LAYER_ROT_90_MIR_H,    // 90 degree CW rotation then mirror horizontally
    VPHAL_HDR_LAYER_MIRROR_V,        // 180 degree rotation then mirror horizontally (vertical mirror)
    VPHAL_HDR_LAYER_ROT_90_MIR_V     // 270 degree CW rotation then mirror horizontally (90 degree CW rotation then vertical mirror)
} VPHAL_HDR_ROTATION;

//!
//! \brief Two Layer Option enum
//!
typedef enum _VPHAL_HDR_TWO_LAYER_OPTION
{
    VPHAL_HDR_TWO_LAYER_OPTION_SBLEND = 0, // Source Blending
    VPHAL_HDR_TWO_LAYER_OPTION_CBLEND,     // Constant Blending
    VPHAL_HDR_TWO_LAYER_OPTION_PBLEND,     // Partial Blending
    VPHAL_HDR_TWO_LAYER_OPTION_CSBLEND,    // Constant Source Blending
    VPHAL_HDR_TWO_LAYER_OPTION_CPBLEND,    // Constant Partial Blending
    VPHAL_HDR_TWO_LAYER_OPTION_COMP        // Composition
} VPHAL_HDR_TWO_LAYER_OPTION;

//!
//! \brief sampler state index enum
//!
typedef enum _VPHAL_HDR_SAMPLER_STATE_INDEX
{
    VPHAL_HDR_SAMPLER_STATE_AVS_NEAREST_INDEX = 1,
    VPHAL_HDR_SAMPLER_STATE_AVS_POLYPHASE_INDEX = 3,
    VPHAL_HDR_SAMPLER_STATE_3D_NEAREST_INDEX = 13,
    VPHAL_HDR_SAMPLER_STATE_3D_BILINEAR_INDEX = 14
} VPHAL_HDR_SAMPLER_STATE_INDEX;

struct VP_HDR_LAYER
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

class VpRenderHdrKernel : public VpRenderKernelObj
{
public:
    VpRenderHdrKernel(PVP_MHWINTERFACE hwInterface, PVpAllocator allocator);

    virtual ~VpRenderHdrKernel()
    {
        MOS_Delete(m_hdrParams);
    }

    virtual MOS_STATUS GetCurbeState(void*& curbe, uint32_t& curbeLength) override;

    virtual uint32_t GetInlineDataSize() override;

    //virtual MOS_STATUS GetAlignedLength(uint32_t &curbeLength, uint32_t &curbeLengthAligned, RENDERHAL_KERNEL_PARAM kernelParam, uint32_t dwBlockAlign) override;

    virtual MOS_STATUS GetWalkerSetting(KERNEL_WALKER_PARAMS& walkerParam, KERNEL_PACKET_RENDER_DATA &renderData) override;

    virtual MOS_STATUS FreeCurbe(void*& curbe) override
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual bool IsKernelCached() override
    {
        return true;
    }

    virtual Kdll_CacheEntry *GetCachedEntryForKernelLoad() override
    {
        return m_kernelEntry;
    }

    virtual MOS_STATUS InitRenderHalSurface(
        SurfaceType             type,
        VP_SURFACE              *surf,
        PRENDERHAL_SURFACE      renderHalSurface) override;

    //virtual void OcaDumpKernelInfo(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext) override;

    virtual MOS_STATUS GetScoreboardParams(PMHW_VFE_SCOREBOARD &scoreboardParams) override;
    virtual MOS_STATUS SetSamplerStates(KERNEL_SAMPLER_STATE_GROUP &samplerStateGroup) override;

    void DumpSurfaces() override;
    void DumpCurbe(void *pCurbe, int32_t iSize);
    void PrintCurbeData(PMEDIA_WALKER_HDR_STATIC_DATA curbeData);

protected:
    virtual MOS_STATUS SetupSurfaceState() override;
    virtual MOS_STATUS SetKernelConfigs(KERNEL_CONFIGS &kernelConfigs) override;
    //virtual MOS_STATUS SetSamplerStates(KERNEL_SAMPLER_STATE_GROUP& samplerStateGroup) override;
    MOS_STATUS GetSamplerIndex(VPHAL_SCALING_MODE scalingMode, uint32_t yuvPlane, int32_t &samplerIndex, MHW_SAMPLER_TYPE &samplerType);
    MOS_STATUS SetSurfaceParams(KERNEL_SURFACE_STATE_PARAM &surfParam, VP_SURFACE *layer, bool is32MWColorFillKern);
    MOS_STATUS InitRenderHalSurface(VP_SURFACE *surf, PRENDERHAL_SURFACE renderHalSurface);
    MOS_STATUS SetCacheCntl(PVP_RENDER_CACHE_CNTL surfMemCacheCtl) override;
    VPHAL_HDR_FORMAT_DESCRIPTOR GetFormatDescriptor(MOS_FORMAT Format);
    VPHAL_HDR_CHROMA_SITING GetHdrChromaSiting(uint32_t ChromaSiting);
    VPHAL_HDR_ROTATION GetHdrRotation(VPHAL_ROTATION Rotation);
    MOS_STATUS              SamplerAvsCalcScalingTable(
                     MOS_FORMAT      SrcFormat,
                     float           fScale,
                     bool            bVertical,
                     uint32_t        dwChromaSiting,
                     bool            bBalancedFilter,
                     bool            b8TapAdaptiveEnable,
                     PMHW_AVS_PARAMS pAvsParams);
    MOS_STATUS SetSamplerAvsTableParam(
        PRENDERHAL_INTERFACE     pRenderHal,
        PMHW_SAMPLER_STATE_PARAM pSamplerStateParams,
        PMHW_AVS_PARAMS          pAvsParams,
        MOS_FORMAT               SrcFormat,
        float                    fScaleX,
        float                    fScaleY,
        uint32_t                 dwChromaSiting);

    MOS_STATUS VpHal_HdrCalcYuvToRgbMatrix(
        VPHAL_CSPACE src,
        VPHAL_CSPACE dst,
        float       *pTransferMatrix,
        float       *pOutMatrix);

    MOS_STATUS VpHal_HdrGetYuvRangeAndOffset(
            VPHAL_CSPACE cspace,
            float       *pLumaOffset,
            float       *pLumaExcursion,
            float       *pChromaZero,
            float       *pChromaExcursion);

    MOS_STATUS VpHal_HdrGetRgbRangeAndOffset(
        VPHAL_CSPACE cspace,
        float       *pRgbOffset,
        float       *pRgbExcursion);

    MOS_STATUS VpHal_HdrCalcRgbToYuvMatrix(
        VPHAL_CSPACE src,
        VPHAL_CSPACE dst,
        float       *pTransferMatrix,
        float       *pOutMatrix);

    MOS_STATUS VpHal_HdrCalcCCMMatrix(
            float *pTransferMatrix,
            float *pOutMatrix);

    MOS_STATUS VpHal_HdrColorTransfer3dLut(
        PRENDER_HDR_PARAMS params,
        int32_t            iIndex,
        float              fInputX,
        float              fInputY,
        float              fInputZ,
        uint16_t          *puOutputX,
        uint16_t          *puOutputY,
        uint16_t          *puOutputZ);

    MOS_STATUS VpHal_HdrToneMapping3dLut(
        VPHAL_HDR_MODE HdrMode,
        double         fInputX,
        double         fInputY,
        double         fInputZ,
        double        *pfOutputX,
        double        *pfOutputY,
        double        *pfOutputZ);

    //MOS_STATUS HdrUpdatePerLayerPipelineStates(FeatureParamHdr    &params, uint32_t* pdwUpdateMask);
    MOS_STATUS InitOETF1DLUT(PRENDER_HDR_PARAMS params, int32_t iIndex, VP_SURFACE *pOETF1DLUTSurface);
    virtual MOS_STATUS HdrInitCoeff(PRENDER_HDR_PARAMS params, VP_SURFACE *pCoeffSurface);
    //MOS_STATUS HdrInitInput3DLUTExt(PRENDER_HDR_PARAMS params, PVPHAL_SURFACE pInput3DLUTSurface);

    MOS_STATUS InitCri3DLUT(
        PRENDER_HDR_PARAMS params,
        int32_t            iIndex,
        VP_SURFACE         *pCRI3DLUTSurface);

    virtual void CalculateH2HPWLFCoefficients(
        HDR_PARAMS       *pSource,
        HDR_PARAMS       *pTarget,
        float            *pPivotPoint,
        uint16_t         *pSlopeIntercept,
        PMOS_INTERFACE    pOsInterface);

    bool ToneMappingStagesAssemble(
        HDR_PARAMS          *srcHDRParams,
        HDR_PARAMS          *targetHDRParams,
        HDRStageConfigEntry *pConfigEntry,
        uint32_t index);

    MOS_STATUS UpdatePerLayerPipelineStates(
        uint32_t           *pdwUpdateMask);

    VP_EXECUTE_CAPS     m_executeCaps       = {};
    Kdll_FilterDesc     m_searchFilter      = {};
    Kdll_SearchState    m_kernelSearch      = {};
    Kdll_State          *m_kernelDllState   = nullptr; //!< Compositing Kernel DLL/Search state

    void HdrLimitFP32ArrayPrecisionToF3_9(float fps[], size_t size);
    void HdrCalculateCCMWithMonitorGamut(
        VPHAL_HDR_CCM_TYPE CCMType,
        HDR_PARAMS         Target,
        float              TempMatrix[12]);

    // Procamp
    int32_t                 m_maxProcampEntries = VP_MAX_PROCAMP;
    Kdll_Procamp            m_Procamp[VP_MAX_PROCAMP] = {};

    PRENDERHAL_INTERFACE    m_renderHal = nullptr;

    VP_FC_DP_BASED_CURBE_DATA m_curbeDataDp = {};
    VP_FC_CURBE_DATA        m_curbeData = {};
    Kdll_CacheEntry         *m_kernelEntry = nullptr;

    // CSC parameters
    VPHAL_COLOR_SAMPLE_8 m_srcColor = {};
    VPHAL_COLOR_SAMPLE_8 m_dstColor = {};
    MEDIA_CSPACE        m_srcCspace  = CSpace_None;
    MEDIA_CSPACE        m_dstCspace  = CSpace_None;

    PRENDER_HDR_PARAMS   m_hdrParams = nullptr;
    MEDIA_WALKER_HDR_STATIC_DATA m_hdrCurbe = {};
    MHW_VFE_SCOREBOARD      m_scoreboardParams;

    bool                m_cscCoeffPatchModeEnabled = false;      //!< Set CSC Coeff using patch mode
    bool                m_computeWalkerEnabled = false;

    // Cache attributes
    VPHAL_HDR_CACHE_CNTL m_surfMemCacheCtl = {};
    KERNEL_SAMPLER_STATE_GROUP *m_samplerStateGroup = nullptr;  // Sampler states parameters for both current kernel object and others.
                                                                // The sampler parameter for specific kernel should be added by kernel
                                                                // object during VpRenderKernelObj::SetSamplerStates being called.
    KERNEL_SAMPLER_INDEX        m_samplerIndexes    = {};       // sampler index for current kernel object.
    PRENDERHAL_INTERFACE        renderHal           = nullptr;

    static const int32_t s_bindingTableIndex[];
    static const int32_t s_bindingTableIndexField[];

MEDIA_CLASS_DEFINE_END(vp__VpRenderHdrKernel)
};
}


#endif // __VP_RENDER_HDR_CMD_PACKET_EXT_H__
