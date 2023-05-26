/*===================== begin_copyright_notice ==================================

# Copyright (c) 2022, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     mhw_sfc_hwcmd_xe_lpm_plus_next.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of Xe_LPM_plus as other components
//!           should use MHW interface to interact with MHW commands and states.
//!

#ifndef __MHW_SFC_HWCMD_XE_LPM_PLUS_NEXT_H__
#define __MHW_SFC_HWCMD_XE_LPM_PLUS_NEXT_H__

#pragma once

#include <cstdint>
#include <cstddef>
#include "media_class_trace.h"
#include "mos_utilities.h"

#pragma pack(1)
namespace mhw
{
namespace sfc
{
namespace xe_lpm_plus_next
{
class Cmd
{
public:
    // Internal Macros
    #define __CODEGEN_MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
    #define __CODEGEN_BITFIELD(l, h) (h) - (l) + 1
    #define __CODEGEN_OP_LENGTH_BIAS 2
    #define __CODEGEN_OP_LENGTH(x) (uint32_t)((__CODEGEN_MAX(x, __CODEGEN_OP_LENGTH_BIAS)) - __CODEGEN_OP_LENGTH_BIAS)

    static uint32_t GetOpLength(uint32_t uiLength) { return __CODEGEN_OP_LENGTH(uiLength); }

    //!
    //! \brief SFC_AVS_STATE
    //! \details
    //!     This command is sent from VDBOX/VEBOX to SFC pipeline at the start of
    //!     each frame once the lock request is granted.
    //!
    struct SFC_AVS_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopcodeb                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODEB
                uint32_t                 Subopcodea                                       : __CODEGEN_BITFIELD(21, 22)    ; //!< SUBOPCODEA
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(23, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 TransitionAreaWith8Pixels                        : __CODEGEN_BITFIELD( 0,  2)    ; //!< Transition Area with 8 Pixels
                uint32_t                 Reserved35                                       : __CODEGEN_BITFIELD( 3,  3)    ; //!< Reserved
                uint32_t                 TransitionAreaWith4Pixels                        : __CODEGEN_BITFIELD( 4,  6)    ; //!< Transition Area with 4 Pixels
                uint32_t                 Reserved39                                       : __CODEGEN_BITFIELD( 7, 23)    ; //!< Reserved
                uint32_t                 SharpnessLevel                                   : __CODEGEN_BITFIELD(24, 31)    ; //!< SHARPNESS_LEVEL
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 MaxDerivativePoint8                              : __CODEGEN_BITFIELD( 0,  7)    ; //!< MAX Derivative Point 8
                uint32_t                 Reserved72                                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 MaxDerivative4Pixels                             : __CODEGEN_BITFIELD(16, 23)    ; //!< Max Derivative 4 Pixels
                uint32_t                 Reserved88                                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 InputVerticalSitingSpecifiesTheVerticalSitingOfTheInput : __CODEGEN_BITFIELD( 0,  3)    ; //!< INPUT_VERTICAL_SITING_SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT
                uint32_t                 Reserved100                                      : __CODEGEN_BITFIELD( 4,  7)    ; //!< Reserved
                uint32_t                 InputHorizontalSitingValueSpecifiesTheHorizontalSitingOfTheInput : __CODEGEN_BITFIELD( 8, 12)    ; //!< Input Horizontal Siting Value - Specifies the horizontal siting of the input
                uint32_t                 Reserved109                                      : __CODEGEN_BITFIELD(13, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;

        //! \name Local enumerations

        enum SUBOPCODEB
        {
            SUBOPCODEB_SFCAVSSTATE                                           = 2, //!< No additional details
        };

        enum SUBOPCODEA
        {
            SUBOPCODEA_COMMON                                                = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE                            = 9,   //!< No additional details
            MEDIA_COMMAND_OPCODE_MEDIAMISC                                   = 10, //!< Media MFX/VEBOX+SFC Modegen
            MEDIA_COMMAND_OPCODE_MEDIAAVPSFCMODE                             = 0xD, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MEDIA                                                   = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief SHARPNESS_LEVEL
        //! \details
        //!     When adaptive scaling is off, determines the balance between sharp and
        //!     smooth scalers.
        enum SHARPNESS_LEVEL
        {
            SHARPNESS_LEVEL_UNNAMED0                                         = 0, //!< Contribute 1 from the smooth scalar
            SHARPNESS_LEVEL_UNNAMED255                                       = 255, //!< Contribute 1 from the sharp scalar
        };

        //! \brief INPUT_VERTICAL_SITING_SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT
        //! \details
        //!     For 444 and 422 format, vertical chroma siting should be programmed to
        //!     zero.
        enum INPUT_VERTICAL_SITING_SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT
        {
            INPUT_VERTICAL_SITING_SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT_0 = 0, //!< No additional details
            INPUT_VERTICAL_SITING_SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT_18 = 1, //!< No additional details
            INPUT_VERTICAL_SITING_SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT_28 = 2, //!< No additional details
            INPUT_VERTICAL_SITING_SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT_38 = 3, //!< No additional details
            INPUT_VERTICAL_SITING_SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT_48 = 4, //!< No additional details
            INPUT_VERTICAL_SITING_SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT_58 = 5, //!< No additional details
            INPUT_VERTICAL_SITING_SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT_68 = 6, //!< No additional details
            INPUT_VERTICAL_SITING_SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT_78 = 7, //!< No additional details
            INPUT_VERTICAL_SITING_SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT_88 = 8, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        SFC_AVS_STATE_CMD()
        {
            DW0.Value                                        = 0x75020002;
            //DW0.DwordLength                                  = GetOpLength(dwSize);
            //DW0.Subopcodeb                                   = SUBOPCODEB_SFCAVSSTATE;
            //DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
            //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMISC;
            //DW0.Pipeline                                     = PIPELINE_MEDIA;
            //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

            DW1.Value                                        = 0x00000000;
            //DW1.SharpnessLevel                               = SHARPNESS_LEVEL_UNNAMED0;

            DW2.Value                                        = 0x00000000;

            DW3.Value                                        = 0x00000000;
            //DW3.InputVerticalSitingSpecifiesTheVerticalSitingOfTheInput = INPUT_VERTICAL_SITING_SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT_0;

        }


        static const size_t dwSize = 4;
        static const size_t byteSize = 16;
    };

    //!
    //! \brief SFC_IEF_STATE
    //! \details
    //!     This command is sent from VDBOX/VEBOX to SFC pipeline at the start of
    //!     each frame once the lock request is granted.
    //!
    struct SFC_IEF_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopcodeb                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODEB
                uint32_t                 Subopcodea                                       : __CODEGEN_BITFIELD(21, 22)    ; //!< SUBOPCODEA
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(23, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 GainFactor                                       : __CODEGEN_BITFIELD( 0,  5)    ; //!< GAIN_FACTOR
                uint32_t                 WeakEdgeThreshold                                : __CODEGEN_BITFIELD( 6, 11)    ; //!< WEAK_EDGE_THRESHOLD
                uint32_t                 StrongEdgeThreshold                              : __CODEGEN_BITFIELD(12, 17)    ; //!< STRONG_EDGE_THRESHOLD
                uint32_t                 R3XCoefficient                                   : __CODEGEN_BITFIELD(18, 22)    ; //!< R3X_COEFFICIENT
                uint32_t                 R3CCoefficient                                   : __CODEGEN_BITFIELD(23, 27)    ; //!< R3C_COEFFICIENT
                uint32_t                 Reserved60                                       : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 GlobalNoiseEstimation                            : __CODEGEN_BITFIELD( 0,  7)    ; //!< GLOBAL_NOISE_ESTIMATION
                uint32_t                 NonEdgeWeight                                    : __CODEGEN_BITFIELD( 8, 10)    ; //!< NON_EDGE_WEIGHT
                uint32_t                 RegularWeight                                    : __CODEGEN_BITFIELD(11, 13)    ; //!< REGULAR_WEIGHT
                uint32_t                 StrongEdgeWeight                                 : __CODEGEN_BITFIELD(14, 16)    ; //!< STRONG_EDGE_WEIGHT
                uint32_t                 R5XCoefficient                                   : __CODEGEN_BITFIELD(17, 21)    ; //!< R5X_COEFFICIENT
                uint32_t                 R5CxCoefficient                                  : __CODEGEN_BITFIELD(22, 26)    ; //!< R5CX_COEFFICIENT
                uint32_t                 R5CCoefficient                                   : __CODEGEN_BITFIELD(27, 31)    ; //!< R5C_COEFFICIENT
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 StdSinAlpha                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< STD Sin(alpha) 
                uint32_t                 StdCosAlpha                                      : __CODEGEN_BITFIELD( 8, 15)    ; //!< STD Cos(alpha) 
                uint32_t                 SatMax                                           : __CODEGEN_BITFIELD(16, 21)    ; //!< SAT_MAX
                uint32_t                 HueMax                                           : __CODEGEN_BITFIELD(22, 27)    ; //!< HUE_MAX
                uint32_t                 Reserved124                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 S3U                                              : __CODEGEN_BITFIELD( 0, 10)    ; //!< S3U
                uint32_t                 Reserved139                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 DiamondMargin                                    : __CODEGEN_BITFIELD(12, 14)    ; //!< DIAMOND_MARGIN
                uint32_t                 VyStdEnable                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< VY_STD_Enable
                uint32_t                 UMid                                             : __CODEGEN_BITFIELD(16, 23)    ; //!< U_MID
                uint32_t                 VMid                                             : __CODEGEN_BITFIELD(24, 31)    ; //!< V_MID
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 DiamondDv                                        : __CODEGEN_BITFIELD( 0,  6)    ; //!< DIAMOND_DV
                uint32_t                 DiamondTh                                        : __CODEGEN_BITFIELD( 7, 12)    ; //!< DIAMOND_TH
                uint32_t                 DiamondAlpha                                     : __CODEGEN_BITFIELD(13, 20)    ; //!< Diamond_alpha
                uint32_t                 HsMargin                                         : __CODEGEN_BITFIELD(21, 23)    ; //!< HS_MARGIN
                uint32_t                 DiamondDu                                        : __CODEGEN_BITFIELD(24, 30)    ; //!< DIAMOND_DU
                uint32_t                 SkinDetailFactor                                 : __CODEGEN_BITFIELD(31, 31)    ; //!< SKIN_DETAIL_FACTOR
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 YPoint1                                          : __CODEGEN_BITFIELD( 0,  7)    ; //!< Y_POINT_1
                uint32_t                 YPoint2                                          : __CODEGEN_BITFIELD( 8, 15)    ; //!< Y_POINT_2
                uint32_t                 YPoint3                                          : __CODEGEN_BITFIELD(16, 23)    ; //!< Y_POINT_3
                uint32_t                 YPoint4                                          : __CODEGEN_BITFIELD(24, 31)    ; //!< Y_POINT_4
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            struct
            {
                uint32_t                 InvMarginVyl                                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< INV_Margin_VYL
                uint32_t                 Reserved240                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            struct
            {
                uint32_t                 InvMarginVyu                                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< INV_Margin_VYU
                uint32_t                 P0L                                              : __CODEGEN_BITFIELD(16, 23)    ; //!< P0L
                uint32_t                 P1L                                              : __CODEGEN_BITFIELD(24, 31)    ; //!< P1L
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            struct
            {
                uint32_t                 P2L                                              : __CODEGEN_BITFIELD( 0,  7)    ; //!< P2L
                uint32_t                 P3L                                              : __CODEGEN_BITFIELD( 8, 15)    ; //!< P3L
                uint32_t                 B0L                                              : __CODEGEN_BITFIELD(16, 23)    ; //!< B0L
                uint32_t                 B1L                                              : __CODEGEN_BITFIELD(24, 31)    ; //!< B1L
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            struct
            {
                uint32_t                 B2L                                              : __CODEGEN_BITFIELD( 0,  7)    ; //!< B2L
                uint32_t                 B3L                                              : __CODEGEN_BITFIELD( 8, 15)    ; //!< B3L
                uint32_t                 S0L                                              : __CODEGEN_BITFIELD(16, 26)    ; //!< S0L
                uint32_t                 YSlope2                                          : __CODEGEN_BITFIELD(27, 31)    ; //!< Y_Slope_2
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            struct
            {
                uint32_t                 S1L                                              : __CODEGEN_BITFIELD( 0, 10)    ; //!< S1L
                uint32_t                 S2L                                              : __CODEGEN_BITFIELD(11, 21)    ; //!< S2L
                uint32_t                 Reserved374                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            struct
            {
                uint32_t                 S3L                                              : __CODEGEN_BITFIELD( 0, 10)    ; //!< S3L
                uint32_t                 P0U                                              : __CODEGEN_BITFIELD(11, 18)    ; //!< P0U
                uint32_t                 P1U                                              : __CODEGEN_BITFIELD(19, 26)    ; //!< P1U
                uint32_t                 YSlope1                                          : __CODEGEN_BITFIELD(27, 31)    ; //!< Y_Slope1
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            struct
            {
                uint32_t                 P2U                                              : __CODEGEN_BITFIELD( 0,  7)    ; //!< P2U
                uint32_t                 P3U                                              : __CODEGEN_BITFIELD( 8, 15)    ; //!< P3U
                uint32_t                 B0U                                              : __CODEGEN_BITFIELD(16, 23)    ; //!< B0U
                uint32_t                 B1U                                              : __CODEGEN_BITFIELD(24, 31)    ; //!< B1U
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            struct
            {
                uint32_t                 B2U                                              : __CODEGEN_BITFIELD( 0,  7)    ; //!< B2U
                uint32_t                 B3U                                              : __CODEGEN_BITFIELD( 8, 15)    ; //!< B3U
                uint32_t                 S0U                                              : __CODEGEN_BITFIELD(16, 26)    ; //!< S0U
                uint32_t                 Reserved475                                      : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            struct
            {
                uint32_t                 S1U                                              : __CODEGEN_BITFIELD( 0, 10)    ; //!< S1U
                uint32_t                 S2U                                              : __CODEGEN_BITFIELD(11, 21)    ; //!< S2U
                uint32_t                 Reserved502                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            struct
            {
                uint32_t                 TransformEnable                                  : __CODEGEN_BITFIELD( 0,  0)    ; //!< Transform Enable
                uint32_t                 YuvChannelSwap                                   : __CODEGEN_BITFIELD( 1,  1)    ; //!< YUV Channel Swap
                uint32_t                 Reserved514                                      : __CODEGEN_BITFIELD( 2,  2)    ; //!< Reserved
                uint32_t                 C0                                               : __CODEGEN_BITFIELD( 3, 15)    ; //!< C0
                uint32_t                 C1                                               : __CODEGEN_BITFIELD(16, 28)    ; //!< C1
                uint32_t                 Reserved541                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            struct
            {
                uint32_t                 C2                                               : __CODEGEN_BITFIELD( 0, 12)    ; //!< C2
                uint32_t                 C3                                               : __CODEGEN_BITFIELD(13, 25)    ; //!< C3
                uint32_t                 Reserved570                                      : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            struct
            {
                uint32_t                 C4                                               : __CODEGEN_BITFIELD( 0, 12)    ; //!< C4
                uint32_t                 C5                                               : __CODEGEN_BITFIELD(13, 25)    ; //!< C5
                uint32_t                 Reserved602                                      : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            struct
            {
                uint32_t                 C6                                               : __CODEGEN_BITFIELD( 0, 12)    ; //!< C6
                uint32_t                 C7                                               : __CODEGEN_BITFIELD(13, 25)    ; //!< C7
                uint32_t                 Reserved634                                      : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            struct
            {
                uint32_t                 C8                                               : __CODEGEN_BITFIELD( 0, 12)    ; //!< C8
                uint32_t                 Reserved653                                      : __CODEGEN_BITFIELD(13, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            struct
            {
                uint32_t                 OffsetIn1                                        : __CODEGEN_BITFIELD( 0, 10)    ; //!< OFFSET_IN_1
                uint32_t                 OffsetOut1                                       : __CODEGEN_BITFIELD(11, 21)    ; //!< OFFSET_OUT_1
                uint32_t                 Reserved694                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW21;
        union
        {
            struct
            {
                uint32_t                 OffsetIn2                                        : __CODEGEN_BITFIELD( 0, 10)    ; //!< OFFSET_IN_2
                uint32_t                 OffsetOut2                                       : __CODEGEN_BITFIELD(11, 21)    ; //!< OFFSET_OUT_2
                uint32_t                 Reserved726                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW22;
        union
        {
            struct
            {
                uint32_t                 OffsetIn3                                        : __CODEGEN_BITFIELD( 0, 10)    ; //!< OFFSET_IN_3
                uint32_t                 OffsetOut3                                       : __CODEGEN_BITFIELD(11, 21)    ; //!< OFFSET_OUT_3
                uint32_t                 Reserved758                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW23;

        //! \name Local enumerations

        enum SUBOPCODEB
        {
            SUBOPCODEB_SFCIEFSTATE                                           = 3, //!< No additional details
        };

        enum SUBOPCODEA
        {
            SUBOPCODEA_COMMON                                                = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE                            = 9, //!< No additional details
            MEDIA_COMMAND_OPCODE_MEDIAMISC                                   = 10, //!< Media MFX/VEBOX+SFC Mode
            MEDIA_COMMAND_OPCODE_MEDIAAVPSFCMODE                             = 0xD, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MEDIA                                                   = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief GAIN_FACTOR
        //! \details
        //!     User control sharpening strength.
        enum GAIN_FACTOR
        {
            GAIN_FACTOR_UNNAMED44                                            = 44, //!< No additional details
        };

        //! \brief WEAK_EDGE_THRESHOLD
        //! \details
        //!     If Strong Edge Threshold > EM > Weak Edge Threshold  the basic VSA
        //!     detects a weak edge.
        enum WEAK_EDGE_THRESHOLD
        {
            WEAK_EDGE_THRESHOLD_UNNAMED1                                     = 1, //!< No additional details
        };

        //! \brief STRONG_EDGE_THRESHOLD
        //! \details
        //!     If EM > Strong Edge Threshold  the basic VSA detects a strong edge.
        enum STRONG_EDGE_THRESHOLD
        {
            STRONG_EDGE_THRESHOLD_UNNAMED8                                   = 8, //!< No additional details
        };

        //! \brief R3X_COEFFICIENT
        //! \details
        //!     IEF smoothing coefficient, see IEF map.
        enum R3X_COEFFICIENT
        {
            R3X_COEFFICIENT_UNNAMED5                                         = 5, //!< No additional details
        };

        //! \brief R3C_COEFFICIENT
        //! \details
        //!     IEF smoothing coefficient, see IEF map.
        enum R3C_COEFFICIENT
        {
            R3C_COEFFICIENT_UNNAMED5                                         = 5, //!< No additional details
        };

        //! \brief GLOBAL_NOISE_ESTIMATION
        //! \details
        //!     Global noise estimation of previous frame.
        enum GLOBAL_NOISE_ESTIMATION
        {
            GLOBAL_NOISE_ESTIMATION_UNNAMED255                               = 255, //!< No additional details
        };

        //! \brief NON_EDGE_WEIGHT
        //! \details
        //!     . Sharpening strength when NO EDGE is found in basic VSA.
        enum NON_EDGE_WEIGHT
        {
            NON_EDGE_WEIGHT_UNNAMED1                                         = 1, //!< No additional details
        };

        //! \brief REGULAR_WEIGHT
        //! \details
        //!     Sharpening strength when a WEAK edge is found in basic VSA.
        enum REGULAR_WEIGHT
        {
            REGULAR_WEIGHT_UNNAMED2                                          = 2, //!< No additional details
        };

        //! \brief STRONG_EDGE_WEIGHT
        //! \details
        //!     Sharpening strength when a STRONG edge is found in basic VSA.
        enum STRONG_EDGE_WEIGHT
        {
            STRONG_EDGE_WEIGHT_UNNAMED7                                      = 7, //!< No additional details
        };

        //! \brief R5X_COEFFICIENT
        //! \details
        //!     IEF smoothing coefficient, see IEF map.
        enum R5X_COEFFICIENT
        {
            R5X_COEFFICIENT_UNNAMED7                                         = 7, //!< No additional details
        };

        //! \brief R5CX_COEFFICIENT
        //! \details
        //!     IEF smoothing coefficient, see IEF map.
        enum R5CX_COEFFICIENT
        {
            R5CX_COEFFICIENT_UNNAMED7                                        = 7, //!< No additional details
        };

        //! \brief R5C_COEFFICIENT
        //! \details
        //!     IEF smoothing coefficient, see IEF map.
        enum R5C_COEFFICIENT
        {
            R5C_COEFFICIENT_UNNAMED7                                         = 7, //!< No additional details
        };

        //! \brief SAT_MAX
        //! \details
        //!     Rectangle half length.
        enum SAT_MAX
        {
            SAT_MAX_UNNAMED31                                                = 31, //!< No additional details
        };

        //! \brief HUE_MAX
        //! \details
        //!     Rectangle half width.
        enum HUE_MAX
        {
            HUE_MAX_UNNAMED14                                                = 14, //!< No additional details
        };

        enum DIAMOND_MARGIN
        {
            DIAMOND_MARGIN_UNNAMED4                                          = 4, //!< No additional details
        };

        //! \brief U_MID
        //! \details
        //!     Rectangle middle-point U coordinate.
        enum U_MID
        {
            U_MID_UNNAMED110                                                 = 110, //!< No additional details
        };

        //! \brief V_MID
        //! \details
        //!     Rectangle middle-point V coordinate.
        enum V_MID
        {
            V_MID_UNNAMED154                                                 = 154, //!< No additional details
        };

        //! \brief DIAMOND_DV
        //! \details
        //!     Rhombus center shift in the hue-direction, relative to the rectangle
        //!     center.
        enum DIAMOND_DV
        {
            DIAMOND_DV_UNNAMED0                                              = 0, //!< No additional details
        };

        //! \brief DIAMOND_TH
        //! \details
        //!     Half length of the rhombus axis in the sat-direction.
        enum DIAMOND_TH
        {
            DIAMOND_TH_UNNAMED35                                             = 35, //!< No additional details
        };

        //! \brief HS_MARGIN
        //! \details
        //!     Defines rectangle margin.
        enum HS_MARGIN
        {
            HS_MARGIN_UNNAMED3                                               = 3, //!< No additional details
        };

        //! \brief DIAMOND_DU
        //! \details
        //!     Rhombus center shift in the sat-direction, relative to the rectangle
        //!     center.
        enum DIAMOND_DU
        {
            DIAMOND_DU_UNNAMED0                                              = 0, //!< No additional details
        };

        //! \brief SKIN_DETAIL_FACTOR
        //! \details
        //!     This flag bit is in operation only when one of the following conditions
        //!     exists:when the control bit SkinToneTunedIEF_Enable is
        //!     on.
        //!     When SkinDetailFactor is equal to 0,
        //!     sign(SkinDetailFactor) is equal to +1, and the content of the
        //!     detected skin tone area is detail revealed.</Li>When
        //!     SkinDetailFactor is equal to 1, sign(SkinDetailFactor) is
        //!     equal to -1, and the content of the detected skin tone area is not
        //!     detail revealed.

        enum SKIN_DETAIL_FACTOR
        {
            SKIN_DETAIL_FACTOR_DETAILREVEALED                                = 0, //!< No additional details
            SKIN_DETAIL_FACTOR_NOTDETAILREVEALED                             = 1, //!< No additional details
        };

        //! \brief Y_POINT_1
        //! \details
        //!     First point of the Y piecewise linear membership function.
        enum Y_POINT_1
        {
            Y_POINT_1_UNNAMED46                                              = 46, //!< No additional details
        };

        //! \brief Y_POINT_2
        //! \details
        //!     Second point of the Y piecewise linear membership function.
        enum Y_POINT_2
        {
            Y_POINT_2_UNNAMED47                                              = 47, //!< No additional details
        };

        //! \brief Y_POINT_3
        //! \details
        //!     Third point of the Y piecewise linear membership function.
        enum Y_POINT_3
        {
            Y_POINT_3_UNNAMED254                                             = 254, //!< No additional details
        };

        //! \brief Y_POINT_4
        //! \details
        //!     Fourth point of the Y piecewise linear membership function.
        enum Y_POINT_4
        {
            Y_POINT_4_UNNAMED255                                             = 255, //!< No additional details
        };

        //! \brief P0L
        //! \details
        //!     Y Point 0 of the lower part of the detection PWLF.
        enum P0L
        {
            P0L_UNNAMED46                                                    = 46, //!< No additional details
        };

        //! \brief P1L
        //! \details
        //!     Y Point 1 of the lower part of the detection PWLF.
        enum P1L
        {
            P1L_UNNAMED216                                                   = 216, //!< No additional details
        };

        //! \brief P2L
        //! \details
        //!     Y Point 2 of the lower part of the detection PWLF.
        enum P2L
        {
            P2L_UNNAMED236                                                   = 236, //!< No additional details
        };

        //! \brief P3L
        //! \details
        //!     Y Point 3 of the lower part of the detection PWLF.
        enum P3L
        {
            P3L_UNNAMED236                                                   = 236, //!< No additional details
        };

        //! \brief B0L
        //! \details
        //!     V Bias 0 of the lower part of the detection PWLF.
        enum B0L
        {
            B0L_UNNAMED133                                                   = 133, //!< No additional details
        };

        //! \brief B1L
        //! \details
        //!     V Bias 1 of the lower part of the detection PWLF.
        enum B1L
        {
            B1L_UNNAMED130                                                   = 130, //!< No additional details
        };

        //! \brief B2L
        //! \details
        //!     V Bias 2 of the lower part of the detection PWLF.
        enum B2L
        {
            B2L_UNNAMED130                                                   = 130, //!< No additional details
        };

        //! \brief B3L
        //! \details
        //!     V Bias 3 of the lower part of the detection PWLF.
        enum B3L
        {
            B3L_UNNAMED130                                                   = 130, //!< No additional details
        };

        //! \brief P0U
        //! \details
        //!     Y Point 0 of the upper part of the detection PWLF.
        enum P0U
        {
            P0U_UNNAMED46                                                    = 46, //!< No additional details
        };

        //! \brief P1U
        //! \details
        //!     Y Point 1 of the upper part of the detection PWLF.
        enum P1U
        {
            P1U_UNNAMED66                                                    = 66, //!< No additional details
        };

        //! \brief P2U
        //! \details
        //!     Y Point 2 of the upper part of the detection PWLF.
        enum P2U
        {
            P2U_UNNAMED150                                                   = 150, //!< No additional details
        };

        //! \brief P3U
        //! \details
        //!     Y Point 3 of the upper part of the detection PWLF.
        enum P3U
        {
            P3U_UNNAMED236                                                   = 236, //!< No additional details
        };

        //! \brief B0U
        //! \details
        //!     V Bias 0 of the upper part of the detection PWLF.
        enum B0U
        {
            B0U_UNNAMED143                                                   = 143, //!< No additional details
        };

        //! \brief B1U
        //! \details
        //!     V Bias 1 of the upper part of the detection PWLF.
        enum B1U
        {
            B1U_UNNAMED163                                                   = 163, //!< No additional details
        };

        //! \brief B2U
        //! \details
        //!     V Bias 2 of the upper part of the detection PWLF.
        enum B2U
        {
            B2U_UNNAMED200                                                   = 200, //!< No additional details
        };

        //! \brief B3U
        //! \details
        //!     V Bias 3 of the upper part of the detection PWLF.
        enum B3U
        {
            B3U_UNNAMED140                                                   = 140, //!< No additional details
        };

        //! \brief C0
        //! \details
        //!     Transform coefficient
        enum C0
        {
            C0_UNNAMED1024                                                   = 1024, //!< No additional details
        };

        //! \brief C1
        //! \details
        //!     Transform coefficient
        enum C1
        {
            C1_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief C2
        //! \details
        //!     Transform coefficient
        enum C2
        {
            C2_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief C3
        //! \details
        //!     Transform coefficient
        enum C3
        {
            C3_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief C4
        //! \details
        //!     Transform coefficient
        enum C4
        {
            C4_UNNAMED1024                                                   = 1024, //!< No additional details
        };

        //! \brief C5
        //! \details
        //!     Transform coefficient
        enum C5
        {
            C5_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief C6
        //! \details
        //!     Transform coefficient
        enum C6
        {
            C6_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief C7
        //! \details
        //!     Transform coefficient
        enum C7
        {
            C7_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief C8
        //! \details
        //!     Transform coefficient
        enum C8
        {
            C8_UNNAMED1024                                                   = 1024, //!< No additional details
        };

        //! \brief OFFSET_IN_1
        //! \details
        //!     Offset in for Y/R.
        enum OFFSET_IN_1
        {
            OFFSET_IN_1_UNNAMED0                                             = 0, //!< No additional details
        };

        //! \brief OFFSET_OUT_1
        //! \details
        //!     Offset out for Y/R.
        enum OFFSET_OUT_1
        {
            OFFSET_OUT_1_UNNAMED0                                            = 0, //!< No additional details
        };

        //! \brief OFFSET_IN_2
        //! \details
        //!     Offset in for U/G.
        enum OFFSET_IN_2
        {
            OFFSET_IN_2_UNNAMED0                                             = 0, //!< No additional details
        };

        //! \brief OFFSET_OUT_2
        //! \details
        //!     Offset out for U/G.
        enum OFFSET_OUT_2
        {
            OFFSET_OUT_2_UNNAMED0                                            = 0, //!< No additional details
        };

        //! \brief OFFSET_IN_3
        //! \details
        //!     Offset in for V/B.
        enum OFFSET_IN_3
        {
            OFFSET_IN_3_UNNAMED0                                             = 0, //!< No additional details
        };

        //! \brief OFFSET_OUT_3
        //! \details
        //!     Offset out for V/B.
        enum OFFSET_OUT_3
        {
            OFFSET_OUT_3_UNNAMED0                                            = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        SFC_IEF_STATE_CMD()
        {
            DW0.Value                                        = 0x75030016;
            //DW0.DwordLength                                  = GetOpLength(dwSize);
            //DW0.Subopcodeb                                   = SUBOPCODEB_SFCIEFSTATE;
            //DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
            //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMISC;
            //DW0.Pipeline                                     = PIPELINE_MEDIA;
            //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

            DW1.Value                                        = 0x0294806c;
            //DW1.GainFactor                                   = GAIN_FACTOR_UNNAMED44;
            //DW1.WeakEdgeThreshold                            = WEAK_EDGE_THRESHOLD_UNNAMED1;
            //DW1.StrongEdgeThreshold                          = STRONG_EDGE_THRESHOLD_UNNAMED8;
            //DW1.R3XCoefficient                               = R3X_COEFFICIENT_UNNAMED5;
            //DW1.R3CCoefficient                               = R3C_COEFFICIENT_UNNAMED5;

            DW2.Value                                        = 0x39cfd1ff;
            //DW2.GlobalNoiseEstimation                        = GLOBAL_NOISE_ESTIMATION_UNNAMED255;
            //DW2.NonEdgeWeight                                = NON_EDGE_WEIGHT_UNNAMED1;
            //DW2.RegularWeight                                = REGULAR_WEIGHT_UNNAMED2;
            //DW2.StrongEdgeWeight                             = STRONG_EDGE_WEIGHT_UNNAMED7;
            //DW2.R5XCoefficient                               = R5X_COEFFICIENT_UNNAMED7;
            //DW2.R5CxCoefficient                              = R5CX_COEFFICIENT_UNNAMED7;
            //DW2.R5CCoefficient                               = R5C_COEFFICIENT_UNNAMED7;

            DW3.Value                                        = 0x039f0000;
            //DW3.SatMax                                       = SAT_MAX_UNNAMED31;
            //DW3.HueMax                                       = HUE_MAX_UNNAMED14;

            DW4.Value                                        = 0x9a6e4000;
            //DW4.DiamondMargin                                = DIAMOND_MARGIN_UNNAMED4;
            //DW4.UMid                                         = U_MID_UNNAMED110;
            //DW4.VMid                                         = V_MID_UNNAMED154;

            DW5.Value                                        = 0x00601180;
            //DW5.DiamondDv                                    = DIAMOND_DV_UNNAMED0;
            //DW5.DiamondTh                                    = DIAMOND_TH_UNNAMED35;
            //DW5.HsMargin                                     = HS_MARGIN_UNNAMED3;
            //DW5.DiamondDu                                    = DIAMOND_DU_UNNAMED0;
            //DW5.SkinDetailFactor                             = SKIN_DETAIL_FACTOR_DETAILREVEALED;

            DW6.Value                                        = 0xfffe2f2e;
            //DW6.YPoint1                                      = Y_POINT_1_UNNAMED46;
            //DW6.YPoint2                                      = Y_POINT_2_UNNAMED47;
            //DW6.YPoint3                                      = Y_POINT_3_UNNAMED254;
            //DW6.YPoint4                                      = Y_POINT_4_UNNAMED255;

            DW7.Value                                        = 0x00000000;

            DW8.Value                                        = 0xd82e0000;
            //DW8.P0L                                          = P0L_UNNAMED46;
            //DW8.P1L                                          = P1L_UNNAMED216;

            DW9.Value                                        = 0x8285ecec;
            //DW9.P2L                                          = P2L_UNNAMED236;
            //DW9.P3L                                          = P3L_UNNAMED236;
            //DW9.B0L                                          = B0L_UNNAMED133;
            //DW9.B1L                                          = B1L_UNNAMED130;

            DW10.Value                                       = 0x00008282;
            //DW10.B2L                                         = B2L_UNNAMED130;
            //DW10.B3L                                         = B3L_UNNAMED130;

            DW11.Value                                       = 0x00000000;

            DW12.Value                                       = 0x02117000;
            //DW12.P0U                                         = P0U_UNNAMED46;
            //DW12.P1U                                         = P1U_UNNAMED66;

            DW13.Value                                       = 0xa38fec96;
            //DW13.P2U                                         = P2U_UNNAMED150;
            //DW13.P3U                                         = P3U_UNNAMED236;
            //DW13.B0U                                         = B0U_UNNAMED143;
            //DW13.B1U                                         = B1U_UNNAMED163;

            DW14.Value                                       = 0x00008cc8;
            //DW14.B2U                                         = B2U_UNNAMED200;
            //DW14.B3U                                         = B3U_UNNAMED140;

            DW15.Value                                       = 0x00000000;

            DW16.Value                                       = 0x00002000;
            //DW16.C0                                          = C0_UNNAMED1024;
            //DW16.C1                                          = C1_UNNAMED0;

            DW17.Value                                       = 0x00000000;
            //DW17.C2                                          = C2_UNNAMED0;
            //DW17.C3                                          = C3_UNNAMED0;

            DW18.Value                                       = 0x00000400;
            //DW18.C4                                          = C4_UNNAMED1024;
            //DW18.C5                                          = C5_UNNAMED0;

            DW19.Value                                       = 0x00000000;
            //DW19.C6                                          = C6_UNNAMED0;
            //DW19.C7                                          = C7_UNNAMED0;

            DW20.Value                                       = 0x00000400;
            //DW20.C8                                          = C8_UNNAMED1024;

            DW21.Value                                       = 0x00000000;
            //DW21.OffsetIn1                                   = OFFSET_IN_1_UNNAMED0;
            //DW21.OffsetOut1                                  = OFFSET_OUT_1_UNNAMED0;

            DW22.Value                                       = 0x00000000;
            //DW22.OffsetIn2                                   = OFFSET_IN_2_UNNAMED0;
            //DW22.OffsetOut2                                  = OFFSET_OUT_2_UNNAMED0;

            DW23.Value                                       = 0x00000000;
            //DW23.OffsetIn3                                   = OFFSET_IN_3_UNNAMED0;
            //DW23.OffsetOut3                                  = OFFSET_OUT_3_UNNAMED0;

        }


        static const size_t dwSize = 24;
        static const size_t byteSize = 96;
    };

    //!
    //! \brief SFC_FRAME_START
    //! \details
    //!     This command is sent from VDBOX/VEBOX to SFC pipeline at the start of
    //!     each frame once the lock request is granted.
    //!
    struct SFC_FRAME_START_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopcodeb                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODEB
                uint32_t                 Subopcodea                                       : __CODEGEN_BITFIELD(21, 22)    ; //!< SUBOPCODEA
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(23, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 Reserved32                                                                       ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;

        //! \name Local enumerations

        enum SUBOPCODEB
        {
            SUBOPCODEB_SFCFRAMESTART                                         = 4, //!< No additional details
        };

        enum SUBOPCODEA
        {
            SUBOPCODEA_COMMON                                                = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE                            = 9, //!< No additional details
            MEDIA_COMMAND_OPCODE_MEDIAMISC                                   = 10, //!< Media MFX/VEBOX+SFC Mode
            MEDIA_COMMAND_OPCODE_MEDIAAVPSFCMODE                             = 0xD, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MEDIA                                                   = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        SFC_FRAME_START_CMD()
        {
            DW0.Value                                        = 0x75040000;
            //DW0.DwordLength                                  = GetOpLength(dwSize);
            //DW0.Subopcodeb                                   = SUBOPCODEB_SFCFRAMESTART;
            //DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
            //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMISC;
            //DW0.Pipeline                                     = PIPELINE_MEDIA;
            //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

            DW1.Value                                        = 0x00000000;

        }


        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief SFC_LOCK
    //! \details
    //!     This command is used for VD/VE box to communicate with SFC before the
    //!     start of any SFC workload. VD/VE uses this command to make sure that it
    //!     has the ownership of SFC pipe before running workload with SFC since SFC
    //!     is shared between VD/VE on a frame level.  For VD(MFX)-SFC workload,
    //!     only decoder mode is allowed. Encoder mode cannot use SFC.
    //!
    //!     For VD(HCP)-SFC workload, only decoder mode is allowed. Encoder mode
    //!     cannot use SFC
    //!
    struct SFC_LOCK_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopcodeb                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODEB
                uint32_t                 Subopcodea                                       : __CODEGEN_BITFIELD(21, 22)    ; //!< SUBOPCODEA
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(23, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 VeSfcPipeSelect                                  : __CODEGEN_BITFIELD( 0,  0)    ; //!< VE-SFC Pipe Select
                uint32_t                 PreScaledOutputSurfaceOutputEnable               : __CODEGEN_BITFIELD( 1,  1)    ; //!< Pre-Scaled Output Surface Output Enable
                uint32_t                 SfcDisable                                       : __CODEGEN_BITFIELD( 2,  2)    ; //!< SFC_disable
                uint32_t                 Reserved35                                       : __CODEGEN_BITFIELD( 3, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;

        //! \name Local enumerations

        enum SUBOPCODEB
        {
            SUBOPCODEB_SFCLOCK                                               = 0, //!< No additional details
        };

        enum SUBOPCODEA
        {
            SUBOPCODEA_COMMON                                                = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE                            = 9, //!< No additional details
            MEDIA_COMMAND_OPCODE_MEDIAMISC                                   = 10, //!< Media MFX/VEBOX+SFC Mode. For VD(MFX)+SFC mode, only decoder mode is allowed. Encoder mode cannot use SFC
            MEDIA_COMMAND_OPCODE_MEDIAAVPSFCMODE                             = 0xD, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MEDIA                                                   = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        SFC_LOCK_CMD()
        {
            DW0.Value                                        = 0x75000000;
            //DW0.DwordLength                                  = GetOpLength(dwSize);
            //DW0.Subopcodeb                                   = SUBOPCODEB_SFCLOCK;
            //DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
            //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMISC;
            //DW0.Pipeline                                     = PIPELINE_MEDIA;
            //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

            DW1.Value                                        = 0x00000000;

        }

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief SFC_STATE
    //! \details
    //!     This command is sent from VDBOX/HCP/VEBOX to SFC pipeline at the start
    //!     of each frame once the lock request is granted.
    //!
   struct SFC_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopcodeb                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODEB
                uint32_t                 Subopcodea                                       : __CODEGEN_BITFIELD(21, 22)    ; //!< SUBOPCODEA
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(23, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 SfcPipeMode                                      : __CODEGEN_BITFIELD( 0,  3)    ; //!< SFC_PIPE_MODE
                uint32_t                 SfcInputChromaSubSampling                        : __CODEGEN_BITFIELD( 4,  7)    ; //!< SFC_INPUT_CHROMA_SUB_SAMPLING
                uint32_t                 VdVeInputOrderingMode                            : __CODEGEN_BITFIELD( 8, 10)    ; //!< VDVE_INPUT_ORDERING_MODE
                uint32_t                 Reserved43                                       : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 SfcEngineMode                                    : __CODEGEN_BITFIELD(12, 13)    ; //!< SFC_ENGINE_MODE
                uint32_t                 Reserved46                                       : __CODEGEN_BITFIELD(14, 17)    ; //!< Reserved
                uint32_t                 InputFrameDataFormat                             : __CODEGEN_BITFIELD(18, 19)    ; //!< INPUT_FRAME_DATA_FORMAT
                uint32_t                 OutputFrameDataFormat                            : __CODEGEN_BITFIELD(20, 21)    ; //!< OUTPUT_FRAME_DATA_FORMAT
                uint32_t                 TopBottomField                                   : __CODEGEN_BITFIELD(22, 22)    ; //!< Top/Bottom field
                uint32_t                 TopBottomFieldFirst                              : __CODEGEN_BITFIELD(23, 23)    ; //!< Top/Bottom field first
                uint32_t                 Reserved56                                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 InputFrameResolutionWidth                        : __CODEGEN_BITFIELD( 0, 13)    ; //!< Input Frame Resolution Width
                uint32_t                 Reserved78                                       : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 InputFrameResolutionHeight                       : __CODEGEN_BITFIELD(16, 29)    ; //!< Input Frame Resolution Height
                uint32_t                 Reserved94                                       : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 OutputSurfaceFormatType                          : __CODEGEN_BITFIELD( 0,  3)    ; //!< OUTPUT_SURFACE_FORMAT_TYPE
                uint32_t                 Reserved100                                      : __CODEGEN_BITFIELD( 4,  4)    ; //!< Reserved
                uint32_t                 ChannelSwapEnable                                : __CODEGEN_BITFIELD( 5,  5)    ; //!< Channel_Swap Enable
                uint32_t                 Reserved102                                      : __CODEGEN_BITFIELD( 6,  7)    ; //!< Reserved
                uint32_t                 OutputChromaDownsamplingCoSitingPositionVerticalDirection : __CODEGEN_BITFIELD( 8, 11)    ; //!< OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION
                uint32_t                 OutputChromaDownsamplingCoSitingPositionHorizontalDirection : __CODEGEN_BITFIELD(12, 15)    ; //!< OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION
                uint32_t                 InputColorSpace0Yuv1Rgb                          : __CODEGEN_BITFIELD(16, 16)    ; //!< INPUT_COLOR_SPACE_0_YUV1_RGB
                uint32_t                 DitherEnable                                     : __CODEGEN_BITFIELD(17, 17)    ; //!< Dither Enable
                uint32_t                 OutputCompressionFormat                          : __CODEGEN_BITFIELD(18, 22)    ; //!< OUTPUT_COMPRESSION_FORMAT
                uint32_t                 Reserved119                                      : __CODEGEN_BITFIELD(23, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 IefEnable                                        : __CODEGEN_BITFIELD( 0,  0)    ; //!< IEF_ENABLE
                uint32_t                 SkinToneTunedIefEnable                           : __CODEGEN_BITFIELD( 1,  1)    ; //!< Skin Tone Tuned IEF_Enable
                uint32_t                 Ief4SmoothEnable                                 : __CODEGEN_BITFIELD( 2,  2)    ; //!< IEF4SMOOTH_ENABLE_
                uint32_t                 Enable8TapForChromaChannelsFiltering             : __CODEGEN_BITFIELD( 3,  3)    ; //!< Enable 8 tap for Chroma channels filtering
                uint32_t                 AvsFilterMode                                    : __CODEGEN_BITFIELD( 4,  5)    ; //!< AVS_FILTER_MODE
                uint32_t                 AdaptiveFilterForAllChannels                     : __CODEGEN_BITFIELD( 6,  6)    ; //!< ADAPTIVE_FILTER_FOR_ALL_CHANNELS
                uint32_t                 AvsScalingEnable                                 : __CODEGEN_BITFIELD( 7,  7)    ; //!< AVS_SCALING_ENABLE
                uint32_t                 BypassYAdaptiveFiltering                         : __CODEGEN_BITFIELD( 8,  8)    ; //!< BYPASS_Y_ADAPTIVE_FILTERING
                uint32_t                 BypassXAdaptiveFiltering                         : __CODEGEN_BITFIELD( 9,  9)    ; //!< BYPASS_X_ADAPTIVE_FILTERING
                uint32_t                 RgbAdaptive                                      : __CODEGEN_BITFIELD(10, 10)    ; //!< RGB Adaptive
                uint32_t                 Reserved139                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 ChromaUpsamplingEnable                           : __CODEGEN_BITFIELD(12, 12)    ; //!< Chroma Upsampling Enable
                uint32_t                 MirrorType                                       : __CODEGEN_BITFIELD(13, 13)    ; //!< MIRROR_TYPE
                uint32_t                 MirrorMode                                       : __CODEGEN_BITFIELD(14, 14)    ; //!< MIRROR_MODE
                uint32_t                 Reserved143                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 RotationMode                                     : __CODEGEN_BITFIELD(16, 17)    ; //!< ROTATION_MODE
                uint32_t                 ColorFillEnable                                  : __CODEGEN_BITFIELD(18, 18)    ; //!< Color Fill Enable
                uint32_t                 CscEnable                                        : __CODEGEN_BITFIELD(19, 19)    ; //!< CSC Enable
                uint32_t                 Bitdepth                                         : __CODEGEN_BITFIELD(20, 21)    ; //!< BITDEPTH
                uint32_t                 TileType                                         : __CODEGEN_BITFIELD(22, 22)    ; //!< Tile Type
                uint32_t                 HistogramStreamout                               : __CODEGEN_BITFIELD(23, 23)    ; //!< Histogram Streamout
                uint32_t                 Reserved152                                      : __CODEGEN_BITFIELD(24, 30)    ; //!< Reserved
                uint32_t                 SfdChickenBit                                    : __CODEGEN_BITFIELD(31, 31)    ; //!< SFD chicken bit
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 SourceRegionWidth                                : __CODEGEN_BITFIELD( 0, 13)    ; //!< Source Region Width
                uint32_t                 Reserved174                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 SourceRegionHeight                               : __CODEGEN_BITFIELD(16, 29)    ; //!< Source Region Height
                uint32_t                 Reserved190                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 SourceRegionHorizontalOffset                     : __CODEGEN_BITFIELD( 0, 13)    ; //!< Source Region Horizontal Offset
                uint32_t                 Reserved206                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 SourceRegionVerticalOffset                       : __CODEGEN_BITFIELD(16, 29)    ; //!< Source Region Vertical Offset
                uint32_t                 Reserved222                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            struct
            {
                uint32_t                 OutputFrameWidth                                 : __CODEGEN_BITFIELD( 0, 13)    ; //!< Output Frame Width
                uint32_t                 Reserved238                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 OutputFrameHeight                                : __CODEGEN_BITFIELD(16, 29)    ; //!< Output Frame Height
                uint32_t                 Reserved254                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            struct
            {
                uint32_t                 ScaledRegionSizeWidth                            : __CODEGEN_BITFIELD( 0, 13)    ; //!< Scaled Region Size Width
                uint32_t                 Reserved270                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 ScaledRegionSizeHeight                           : __CODEGEN_BITFIELD(16, 29)    ; //!< Scaled Region Size Height
                uint32_t                 Reserved286                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            struct
            {
                uint32_t                 ScaledRegionHorizontalOffset                     : __CODEGEN_BITFIELD( 0, 14)    ; //!< Scaled Region Horizontal Offset
                uint32_t                 Reserved303                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 ScaledRegionVerticalOffset                       : __CODEGEN_BITFIELD(16, 30)    ; //!< Scaled Region Vertical Offset
                uint32_t                 Reserved319                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            struct
            {
                uint32_t                 GrayBarPixelUG                                   : __CODEGEN_BITFIELD( 0,  9)    ; //!< Gray Bar Pixel - U/G
                uint32_t                 Reserved330                                      : __CODEGEN_BITFIELD(10, 15)    ; //!< Reserved
                uint32_t                 GrayBarPixelYR                                   : __CODEGEN_BITFIELD(16, 25)    ; //!< Gray Bar Pixel - Y/R
                uint32_t                 Reserved346                                      : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            struct
            {
                uint32_t                 GrayBarPixelA                                    : __CODEGEN_BITFIELD( 0,  9)    ; //!< Gray Bar Pixel - A
                uint32_t                 Reserved362                                      : __CODEGEN_BITFIELD(10, 15)    ; //!< Reserved
                uint32_t                 GrayBarPixelVB                                   : __CODEGEN_BITFIELD(16, 25)    ; //!< Gray Bar Pixel - V/B
                uint32_t                 Reserved378                                      : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            struct
            {
                uint32_t                 UvDefaultValueForUChannelForMonoInputSupport     : __CODEGEN_BITFIELD( 0,  9)    ; //!< UV Default value for U channel (For Mono Input Support)
                uint32_t                 Reserved394                                      : __CODEGEN_BITFIELD(10, 15)    ; //!< Reserved
                uint32_t                 UvDefaultValueForVChannelForMonoInputSupport     : __CODEGEN_BITFIELD(16, 25)    ; //!< UV Default value for V channel (For Mono Input Support)
                uint32_t                 Reserved410                                      : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            struct
            {
                uint32_t                 AlphaDefaultValue                                : __CODEGEN_BITFIELD( 0,  9)    ; //!< Alpha Default Value
                uint32_t                 Reserved426                                      : __CODEGEN_BITFIELD(10, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            struct
            {
                uint32_t                 Reserved448                                      : __CODEGEN_BITFIELD( 0,  4)    ; //!< Reserved
                uint32_t                 ScalingFactorHeight                              : __CODEGEN_BITFIELD( 5, 27)    ; //!< Scaling Factor Height
                uint32_t                 Reserved476                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            struct
            {
                uint32_t                 Reserved480                                      : __CODEGEN_BITFIELD( 0,  4)    ; //!< Reserved
                uint32_t                 ScaleFactorWidth                                 : __CODEGEN_BITFIELD( 5, 27)    ; //!< Scale Factor Width
                uint32_t                 Reserved508                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            struct
            {
                uint32_t                 Reserved512                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            struct
            {
                uint32_t                 Reserved544                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 OutputFrameSurfaceBaseAddress                    : __CODEGEN_BITFIELD(12, 31)    ; //!< Output Frame Surface Base Address
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            struct
            {
                uint32_t                 OutputFrameSurfaceBaseAddressHigh                : __CODEGEN_BITFIELD( 0, 15)    ; //!< Output Frame Surface Base Address High
                uint32_t                 Reserved592                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            struct
            {
                uint32_t                 Reserved608                                      : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 OutputFrameSurfaceBaseAddressIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< Output Frame Surface Base Address - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 OutputFrameSurfaceBaseAddressArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< Output Frame Surface Base Address - Arbitration Priority Control
                uint32_t                 OutputFrameSurfaceBaseAddressMemoryCompressionEnable : __CODEGEN_BITFIELD( 9,  9)    ; //!< Output Frame Surface Base Address - Memory Compression Enable
                uint32_t                 CompressionType                                  : __CODEGEN_BITFIELD(10, 10)    ; //!< COMPRESSION_TYPE
                uint32_t                 Reserved619                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 OutputFrameSurfaceBaseAddressRowStoreScratchBufferCacheSelect : __CODEGEN_BITFIELD(12, 12)    ; //!< OUTPUT_FRAME_SURFACE_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 Reserved621                                      : __CODEGEN_BITFIELD(13, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            struct
            {
                uint32_t                 Reserved640                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 AvsLineBufferSurfaceBaseAddress                  : __CODEGEN_BITFIELD(12, 31)    ; //!< AVS Line Buffer Surface Base Address
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            struct
            {
                uint32_t                 AvsLineBufferSurfaceBaseAddressHigh              : __CODEGEN_BITFIELD( 0, 15)    ; //!< AVS Line Buffer Surface Base Address High
                uint32_t                 Reserved688                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW21;
        union
        {
            struct
            {
                uint32_t                 Reserved704                                      : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 AvsLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< AVS Line Buffer Base Address - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 AvsLineBufferBaseAddressArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< AVS Line Buffer Base Address - Arbitration Priority Control
                uint32_t                 AvsLineBufferBaseAddressMemoryCompressionEnable  : __CODEGEN_BITFIELD( 9,  9)    ; //!< AVS_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
                uint32_t                 AvsLineBufferBaseAddressMemoryCompressionMode    : __CODEGEN_BITFIELD(10, 10)    ; //!< AVS_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved715                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 AvsLineBufferBaseAddressRowStoreScratchBufferCacheSelect : __CODEGEN_BITFIELD(12, 12)    ; //!< AVS_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 Reserved717                                      : __CODEGEN_BITFIELD(13, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW22;
        union
        {
            struct
            {
                uint32_t                 Reserved736                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 IefLineBufferSurfaceBaseAddress                  : __CODEGEN_BITFIELD(12, 31)    ; //!< IEF Line Buffer Surface Base Address
            };
            uint32_t                     Value;
        } DW23;
        union
        {
            struct
            {
                uint32_t                 IefLineBufferSurfaceBaseAddressHigh              : __CODEGEN_BITFIELD( 0, 15)    ; //!< IEF Line Buffer Surface Base Address High
                uint32_t                 Reserved784                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW24;
        union
        {
            struct
            {
                uint32_t                 Reserved800                                      : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 IefLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< IEF Line Buffer Base Address - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 IefLineBufferBaseAddressArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< IEF Line Buffer Base Address - Arbitration Priority Control
                uint32_t                 IefLineBufferBaseAddressMemoryCompressionEnable  : __CODEGEN_BITFIELD( 9,  9)    ; //!< IEF_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
                uint32_t                 IefLineBufferBaseAddressMemoryCompressionMode    : __CODEGEN_BITFIELD(10, 10)    ; //!< IEF_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved811                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 IefLineBufferBaseAddressRowStoreScratchBufferCacheSelect : __CODEGEN_BITFIELD(12, 12)    ; //!< IEF_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 Reserved813                                      : __CODEGEN_BITFIELD(13, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW25;
        union
        {
            struct
            {
                uint32_t                 Reserved832                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 SfdLineBufferSurfaceBaseAddress                  : __CODEGEN_BITFIELD(12, 31)    ; //!< SFD Line Buffer Surface Base Address
            };
            uint32_t                     Value;
        } DW26;
        union
        {
            struct
            {
                uint32_t                 SfdLineBufferSurfaceBaseAddressHigh              : __CODEGEN_BITFIELD( 0, 15)    ; //!< SFD Line Buffer Surface Base Address High
                uint32_t                 Reserved880                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW27;
        union
        {
            struct
            {
                uint32_t                 Reserved896                                      : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 SfdLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< SFD Line Buffer Base Address - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 SfdLineBufferBaseAddressArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< SFD Line Buffer Base Address - Arbitration Priority Control
                uint32_t                 SfdLineBufferBaseAddressMemoryCompressionEnable  : __CODEGEN_BITFIELD( 9,  9)    ; //!< SFD_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
                uint32_t                 SfdLineBufferBaseAddressMemoryCompressionMode    : __CODEGEN_BITFIELD(10, 10)    ; //!< SFD_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved907                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 SfdLineBufferBaseAddressRowStoreScratchBufferCacheSelect : __CODEGEN_BITFIELD(12, 12)    ; //!< SFD_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 Reserved909                                      : __CODEGEN_BITFIELD(13, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW28;
        union
        {
            struct
            {
                uint32_t                 TiledMode                                        : __CODEGEN_BITFIELD( 0,  1)    ; //!< TILED_MODE
                uint32_t                 OutputSurfaceHalfPitchForChroma                  : __CODEGEN_BITFIELD( 2,  2)    ; //!< Output Surface Half Pitch For Chroma
                uint32_t                 OutputSurfacePitch                               : __CODEGEN_BITFIELD( 3, 21)    ; //!< Output Surface Pitch
                uint32_t                 Reserved950                                      : __CODEGEN_BITFIELD(22, 26)    ; //!< Reserved
                uint32_t                 OutputSurfaceInterleaveChromaEnable              : __CODEGEN_BITFIELD(27, 27)    ; //!< Output Surface Interleave Chroma Enable
                uint32_t                 OutputSurfaceFormat                              : __CODEGEN_BITFIELD(28, 31)    ; //!< Output Surface Format
            };
            uint32_t                     Value;
        } DW29;
        union
        {
            struct
            {
                uint32_t                 OutputSurfaceYOffsetForU                         : __CODEGEN_BITFIELD( 0, 15)    ; //!< Output Surface Y Offset For U
                uint32_t                 OutputSurfaceXOffsetForU                         : __CODEGEN_BITFIELD(16, 31)    ; //!< Output Surface X Offset For U
            };
            uint32_t                     Value;
        } DW30;
        union
        {
            struct
            {
                uint32_t                 OutputSurfaceYOffsetForV                         : __CODEGEN_BITFIELD( 0, 15)    ; //!< Output Surface Y Offset For V
                uint32_t                 OutputSurfaceXOffsetForV                         : __CODEGEN_BITFIELD(16, 31)    ; //!< Output Surface X Offset For V
            };
            uint32_t                     Value;
        } DW31;
        union
        {
            struct
            {
                uint32_t                 Reserved1024                                                                     ; //!< Reserved
            };
            uint32_t                     Value;
        } DW32;
        union
        {
            struct
            {
                uint32_t                 Av1TileColumnNumber                              : __CODEGEN_BITFIELD( 0,  7)    ; //!< AV1 Tile column number
                uint32_t                 Av1TileRowNumber                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< AV1 Tile row number
                uint32_t                 Reserved1072                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW33;
        union
        {
            struct
            {
                uint32_t                 Sourcestartx                                     : __CODEGEN_BITFIELD( 0, 13)    ; //!< SourceStartX
                uint32_t                 Reserved1102                                     : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 Sourceendx                                       : __CODEGEN_BITFIELD(16, 29)    ; //!< SourceEndX
                uint32_t                 Reserved1118                                     : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW34;
        union
        {
            struct
            {
                uint32_t                 Destinationstartx                                : __CODEGEN_BITFIELD( 0, 13)    ; //!< DestinationStartX
                uint32_t                 Reserved1134                                     : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 Destinationendx                                  : __CODEGEN_BITFIELD(16, 29)    ; //!< DestinationEndX
                uint32_t                 Reserved1150                                     : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW35;
        union
        {
            struct
            {
                uint32_t                 Reserved1152                                     : __CODEGEN_BITFIELD( 0,  4)    ; //!< Reserved
                uint32_t                 Xphaseshift                                      : __CODEGEN_BITFIELD( 5, 28)    ; //!< Xphaseshift
                uint32_t                 Reserved1181                                     : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW36;
        union
        {
            struct
            {
                uint32_t                 Reserved1184                                     : __CODEGEN_BITFIELD( 0,  4)    ; //!< Reserved
                uint32_t                 Yphaseshift                                      : __CODEGEN_BITFIELD( 5, 28)    ; //!< Yphaseshift
                uint32_t                 Reserved1213                                     : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW37;
        union
        {
            struct
            {
                uint32_t                 Reserved1216                                     : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 AvsLineTileBufferSurfaceBaseAddress              : __CODEGEN_BITFIELD(12, 31)    ; //!< AVS Line Tile Buffer Surface Base Address
            };
            uint32_t                     Value;
        } DW38;
        union
        {
            struct
            {
                uint32_t                 AvsLineTileBufferSurfaceBaseAddressHigh          : __CODEGEN_BITFIELD( 0, 15)    ; //!< AVS Line Tile Buffer Surface Base Address High
                uint32_t                 Reserved1264                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW39;
        union
        {
            struct
            {
                uint32_t                 Reserved1280                                     : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 AvsLineTileBufferBaseAddressIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< AVS Line Tile Buffer Base Address - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 AvsLineTileBufferBaseAddressArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< AVS Line Tile Buffer Base Address - Arbitration Priority Control
                uint32_t                 AvsLineTileBufferBaseAddressMemoryCompressionEnable : __CODEGEN_BITFIELD( 9,  9)    ; //!< AVS_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
                uint32_t                 AvsLineTileBufferBaseAddressMemoryCompressionMode : __CODEGEN_BITFIELD(10, 10)    ; //!< AVS_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved1291                                     : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 AvsLineTileBufferBaseAddressRowStoreScratchBufferCacheSelect : __CODEGEN_BITFIELD(12, 12)    ; //!< AVS_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 Reserved1293                                     : __CODEGEN_BITFIELD(13, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW40;
        union
        {
            struct
            {
                uint32_t                 Reserved1312                                     : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 IefLineTileBufferSurfaceBaseAddress              : __CODEGEN_BITFIELD(12, 31)    ; //!< IEF Line Tile Buffer Surface Base Address
            };
            uint32_t                     Value;
        } DW41;
        union
        {
            struct
            {
                uint32_t                 IefLineTileBufferSurfaceBaseAddressHigh          : __CODEGEN_BITFIELD( 0, 15)    ; //!< IEF Line Tile Buffer Surface Base Address High
                uint32_t                 Reserved1360                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW42;
        union
        {
            struct
            {
                uint32_t                 Reserved1376                                     : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 IefLineTileBufferBaseAddressIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< IEF Line Tile Buffer Base Address - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 IefLineTileBufferBaseAddressArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< IEF Line Tile Buffer Base Address - Arbitration Priority Control
                uint32_t                 IefLineTileBufferBaseAddressMemoryCompressionEnable : __CODEGEN_BITFIELD( 9,  9)    ; //!< IEF_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
                uint32_t                 IefLineTileBufferBaseAddressMemoryCompressionMode : __CODEGEN_BITFIELD(10, 10)    ; //!< IEF_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved1387                                     : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 IefLineTileBufferBaseAddressRowStoreScratchBufferCacheSelect : __CODEGEN_BITFIELD(12, 12)    ; //!< IEF_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 Reserved1389                                     : __CODEGEN_BITFIELD(13, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW43;
        union
        {
            struct
            {
                uint32_t                 Reserved1408                                     : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 SfdLineTileBufferSurfaceBaseAddress              : __CODEGEN_BITFIELD(12, 31)    ; //!< SFD Line Tile Buffer Surface Base Address
            };
            uint32_t                     Value;
        } DW44;
        union
        {
            struct
            {
                uint32_t                 SfdLineTileBufferSurfaceBaseAddressHigh          : __CODEGEN_BITFIELD( 0, 15)    ; //!< SFD Line Tile Buffer Surface Base Address High
                uint32_t                 Reserved1456                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW45;
        union
        {
            struct
            {
                uint32_t                 Reserved1472                                     : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 SfdLineTileBufferBaseAddressIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< SFD Line Tile Buffer Base Address - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 SfdLineTileBufferBaseAddressArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< SFD Line Tile Buffer Base Address - Arbitration Priority Control
                uint32_t                 SfdLineTileBufferBaseAddressMemoryCompressionEnable : __CODEGEN_BITFIELD( 9,  9)    ; //!< SFD_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
                uint32_t                 SfdLineTileBufferBaseAddressMemoryCompressionMode : __CODEGEN_BITFIELD(10, 10)    ; //!< SFD_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved1483                                     : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 SfdLineTileBufferBaseAddressRowStoreScratchBufferCacheSelect : __CODEGEN_BITFIELD(12, 12)    ; //!< SFD_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 Reserved1485                                     : __CODEGEN_BITFIELD(13, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW46;
        union
        {
            struct
            {
                uint32_t                 Reserved1504                                     : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 HistogramSurfaceBaseAddress                      : __CODEGEN_BITFIELD(12, 31)    ; //!< Histogram Surface Base Address
            };
            uint32_t                     Value;
        } DW47;
        union
        {
            struct
            {
                uint32_t                 HistogramSurfaceBaseAddressHigh                  : __CODEGEN_BITFIELD( 0, 15)    ; //!< Histogram Surface Base Address High
                uint32_t                 Reserved1552                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW48;
        union
        {
            struct
            {
                uint32_t                 Reserved1568                                     : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 HisgotramBaseAddressIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< Hisgotram Base Address - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 HistogramBaseAddressArbitrationPriorityControl   : __CODEGEN_BITFIELD( 7,  8)    ; //!< Histogram Base Address - Arbitration Priority Control
                uint32_t                 HistogramBaseAddressMemoryCompressionEnable      : __CODEGEN_BITFIELD( 9,  9)    ; //!< HISTOGRAM_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
                uint32_t                 HistogramBaseAddressMemoryCompressionType        : __CODEGEN_BITFIELD(10, 10)    ; //!< HISTOGRAM_BASE_ADDRESS_MEMORY_COMPRESSION_TYPE
                uint32_t                 Reserved1579                                     : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 HistogramBaseAddressCacheSelect                  : __CODEGEN_BITFIELD(12, 12)    ; //!< HISTOGRAM_BASE_ADDRESS_CACHE_SELECT
                uint32_t                 Reserved1581                                     : __CODEGEN_BITFIELD(13, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW49;
        union
        {
            struct
            {
                uint32_t                 DitheringLutDelta12                              : __CODEGEN_BITFIELD( 0,  2)    ; //!< Dithering LUT delta 12
                uint32_t                 Reserved1603                                     : __CODEGEN_BITFIELD( 3,  7)    ; //!< Reserved
                uint32_t                 DitheringLutDelta13                              : __CODEGEN_BITFIELD( 8, 10)    ; //!< Dithering LUT delta 13
                uint32_t                 Reserved1611                                     : __CODEGEN_BITFIELD(11, 15)    ; //!< Reserved
                uint32_t                 DitheringLutDelta14                              : __CODEGEN_BITFIELD(16, 18)    ; //!< Dithering LUT delta 14
                uint32_t                 Reserved1619                                     : __CODEGEN_BITFIELD(19, 23)    ; //!< Reserved
                uint32_t                 DitheringLutDelta15                              : __CODEGEN_BITFIELD(24, 26)    ; //!< Dithering LUT delta 15
                uint32_t                 Reserved1627                                     : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW50;
        union
        {
            struct
            {
                uint32_t                 DitheringLutDelta8                               : __CODEGEN_BITFIELD( 0,  2)    ; //!< Dithering LUT delta 8
                uint32_t                 Reserved1635                                     : __CODEGEN_BITFIELD( 3,  7)    ; //!< Reserved
                uint32_t                 DitheringLutDelta9                               : __CODEGEN_BITFIELD( 8, 10)    ; //!< Dithering LUT delta 9
                uint32_t                 Reserved1643                                     : __CODEGEN_BITFIELD(11, 15)    ; //!< Reserved
                uint32_t                 DitheringLutDelta10                              : __CODEGEN_BITFIELD(16, 18)    ; //!< Dithering LUT delta 10
                uint32_t                 Reserved1651                                     : __CODEGEN_BITFIELD(19, 23)    ; //!< Reserved
                uint32_t                 DitheringLutDelta11                              : __CODEGEN_BITFIELD(24, 26)    ; //!< Dithering LUT delta 11
                uint32_t                 Reserved1659                                     : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW51;
        union
        {
            struct
            {
                uint32_t                 DitheringLutDelta4                               : __CODEGEN_BITFIELD( 0,  2)    ; //!< Dithering LUT delta 4
                uint32_t                 Reserved1667                                     : __CODEGEN_BITFIELD( 3,  7)    ; //!< Reserved
                uint32_t                 DitheringLutDelta5                               : __CODEGEN_BITFIELD( 8, 10)    ; //!< Dithering LUT delta 5
                uint32_t                 Reserved1675                                     : __CODEGEN_BITFIELD(11, 15)    ; //!< Reserved
                uint32_t                 DitheringLutDelta6                               : __CODEGEN_BITFIELD(16, 18)    ; //!< Dithering LUT delta 6
                uint32_t                 Reserved1683                                     : __CODEGEN_BITFIELD(19, 23)    ; //!< Reserved
                uint32_t                 DitheringLutDelta7                               : __CODEGEN_BITFIELD(24, 26)    ; //!< Dithering LUT delta 7
                uint32_t                 Reserved1691                                     : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW52;
        union
        {
            struct
            {
                uint32_t                 DitheringLutDelta0                               : __CODEGEN_BITFIELD( 0,  2)    ; //!< Dithering LUT delta 0
                uint32_t                 Reserved1699                                     : __CODEGEN_BITFIELD( 3,  7)    ; //!< Reserved
                uint32_t                 DitheringLutDelta1                               : __CODEGEN_BITFIELD( 8, 10)    ; //!< Dithering LUT delta 1
                uint32_t                 Reserved1707                                     : __CODEGEN_BITFIELD(11, 15)    ; //!< Reserved
                uint32_t                 DitheringLutDelta2                               : __CODEGEN_BITFIELD(16, 18)    ; //!< Dithering LUT delta 2
                uint32_t                 Reserved1715                                     : __CODEGEN_BITFIELD(19, 23)    ; //!< Reserved
                uint32_t                 DitheringLutDelta3                               : __CODEGEN_BITFIELD(24, 26)    ; //!< Dithering LUT delta 3
                uint32_t                 Reserved1723                                     : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW53;
        union
        {
            struct
            {
                uint32_t                 BottomFieldVerticalScalingOffset                 : __CODEGEN_BITFIELD( 0, 23)    ; //!< Bottom field vertical scaling offset
                uint32_t                 Reserved1752                                     : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW54;
        union
        {
            struct
            {
                uint32_t                 Reserved1760                                     : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 BottomFieldBaseAddress                           : __CODEGEN_BITFIELD(12, 31)    ; //!< Bottom field base address
            };
            uint32_t                     Value;
        } DW55;
        union
        {
            struct
            {
                uint32_t                 BottomFieldBaseAddressHigh                       : __CODEGEN_BITFIELD( 0, 15)    ; //!< Bottom field base address high
                uint32_t                 Reserved1808                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW56;
        union
        {
            struct
            {
                uint32_t                 Reserved1824                                     : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 BottomFieldSurfaceBaseAddressIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< Bottom field Surface Base Address - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 BottomFieldSurfaceBaseAddressArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< Bottom field Surface Base Address - Arbitration Priority Control
                uint32_t                 BottomFiledSurfaceBaseAddressMemoryCompressionEnable : __CODEGEN_BITFIELD( 9,  9)    ; //!< Bottom filed Surface Base Address - Memory Compression Enable
                uint32_t                 BottomFiledSurfaceBaseAddressMemoryCompressionType : __CODEGEN_BITFIELD(10, 10)    ; //!< BOTTOM_FILED_SURFACE_BASE_ADDRESS_MEMORY_COMPRESSION_TYPE
                uint32_t                 Reserved1835                                     : __CODEGEN_BITFIELD(11, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW57;
        union
        {
            struct
            {
                uint32_t                 BottomFieldSurfaceTileWalk                       : __CODEGEN_BITFIELD( 0,  0)    ; //!< BOTTOM_FIELD_SURFACE_TILE_WALK
                uint32_t                 BottomFieldSurfaceTiled                          : __CODEGEN_BITFIELD( 1,  1)    ; //!< BOTTOM_FIELD_SURFACE_TILED
                uint32_t                 BottomFieldSurfaceHalfPitchForChroma             : __CODEGEN_BITFIELD( 2,  2)    ; //!< Bottom field Surface Half Pitch For Chroma
                uint32_t                 BottomFieldSurfacePitch                          : __CODEGEN_BITFIELD( 3, 19)    ; //!< Bottom field Surface Pitch
                uint32_t                 Reserved1878                                     : __CODEGEN_BITFIELD(22, 26)    ; //!< Reserved
                uint32_t                 BottomFieldSurfaceInterleaveChromaEnable         : __CODEGEN_BITFIELD(27, 27)    ; //!< Bottom field Surface Interleave Chroma Enable
                uint32_t                 Reserved1884                                     : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW58;
        union
        {
            struct
            {
                uint32_t                 BottomFieldSurfaceYOffsetForU                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< Bottom field Surface Y Offset For U
                uint32_t                 BottomFieldSurfaceXOffsetForU                    : __CODEGEN_BITFIELD(16, 31)    ; //!< Bottom field Surface X Offset For U
            };
            uint32_t                     Value;
        } DW59;
        union
        {
            struct
            {
                uint32_t                 BottomFieldSurfaceYOffsetForV                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< Bottom field Surface Y Offset For V
                uint32_t                 BottomFieldSurfaceXOffsetForV                    : __CODEGEN_BITFIELD(16, 31)    ; //!< Bottom field Surface X Offset For V
            };
            uint32_t                     Value;
        } DW60;
        //! \name Local enumerations

        enum SUBOPCODEB
        {
            SUBOPCODEB_SFCSTATE                                              = 1, //!< No additional details
        };

        enum SUBOPCODEA
        {
            SUBOPCODEA_COMMON                                                = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MEDIAHCPSFCMODE                             = 9,   //!< No additional details
            MEDIA_COMMAND_OPCODE_MEDIAMFXVEBOXSFCMODE                        = 10,  //!< No additional details
            MEDIA_COMMAND_OPCODE_MEDIAAVPSFCMODE                             = 0xD, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MEDIA                                                   = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Local enumerations

        //! \brief SFC_PIPE_MODE
        //! \details
        //!     Note: for SFC Pipe mode set to VE-to-SFC AVS mode. IECP pipeline mode
        //!     MUST be enabled. However, each sub-IECP feature can be turned on/off
        //!     independently.
        enum SFC_PIPE_MODE
        {
            SFC_PIPE_MODE_UNNAMED0                                           = 0, //!< VD-to-SFC AVS
            SFC_PIPE_MODE_UNNAMED1                                           = 1, //!< VE-to-SFC AVS + IEF + Rotation
            SFC_PIPE_MODE_UNNAMED2                                           = 2, //!< HCP-to-SFC AVS
            SFC_PIPE_MODE_UNNAMED3                                           = 3, //!< Reserved
            SFC_PIPE_MODE_UNNAMED4                                           = 4, //!< VE-to-SFC Integral Image
            SFC_PIPE_MODE_UNNAMED5                                           = 5, //!< VD/AV1-to_SFC AVS
        };

        //! \brief SFC_INPUT_CHROMA_SUB_SAMPLING
        //! \details
        //!     This field shall be programmed according to video modes used in VDBOX.
        //!     NOTE: SFC supports progressive input and output only (Interlaced/MBAFF
        //!     is not supported).<table border="1"><tbody><tr><td>Video
        //!     Mode</td><td>Surface Format</td><td>SFC Input Chroma
        //!     Sub-Sampling</td><td>VD/VE Input Ordering Mode</td></tr><tr><td>VC1 w/o
        //!     LF and w/o OS Note: VC1 LF applies for either ILDB</td><td>420
        //!     (NV12)</td><td>1</td><td>0</td></tr><tr><td>VC1 w/ LF or w/ OS or w/
        //!     both Note: VC1 LF applies for either ILDB</td><td /><td>INVALID with
        //!     SFC</td><td>INVALID with SFC</td></tr><tr><td>AVC w/o
        //!     LF</td><td>Monochrome</td><td>0</td><td>0</td></tr><tr><td>AVC w/o
        //!     LF</td><td>420 (NV12)</td><td>1</td><td>0</td></tr><tr><td>AVC with
        //!     LF</td><td>Monochrome</td><td>0</td><td>1</td></tr><tr><td>AVC/VP8 with
        //!     LF</td><td>420 (NV12)</td><td>1</td><td>1</td></tr><tr><td>VP8 w/o
        //!     LF</td><td>420 (NV12)</td><td>1</td><td>4</td></tr><tr><td>JPEG (YUV
        //!     Interleaved)</td><td>Monochrome</td><td>0</td><td>2</td></tr><tr><td>JPEG
        //!     (YUV Interleaved)</td><td>420</td><td>1</td><td>3</td></tr><tr><td>JPEG
        //!     (YUV
        //!     Interleaved)</td><td>422H_2Y</td><td>2</td><td>2</td></tr><tr><td>JPEG
        //!     (YUV
        //!     Interleaved)</td><td>422H_4Y</td><td>2</td><td>3</td></tr><tr><td>JPEG
        //!     (YUV
        //!     Interleaved)</td><td>444</td><td>4</td><td>2</td></tr></tbody></table>This
        //!     field shall be programmed according to Image enhancement modes used in
        //!     VEBOX.<table border="1"><tbody><tr><td>VEBOX MODE</td><td>Surface
        //!     Format</td><td>SFC Input Chroma Sub Sampling</td><td>VD/VE Input
        //!     Ordering Mode</td></tr><tr><td>Legacy DN/DI/IECP
        //!     features</td><td>Monochrome</td><td>0</td><td>0</td></tr><tr><td>Legacy
        //!     DN/DI/IECP features</td><td>420
        //!     (NV12)</td><td>1</td><td>0</td></tr><tr><td>Legacy DN/DI/IECP
        //!     features</td><td>422H</td><td>2</td><td>0</td></tr><tr><td>Legacy
        //!     DN/DI/IECP
        //!     features</td><td>444</td><td>4</td><td>0</td></tr><tr><td>Capture/Camera
        //!     pipe
        //!     enabled(Demosaic)</td><td>Monochrome</td><td>0</td><td>1</td></tr><tr><td>Capture/Camera
        //!     pipe enabled(Demosaic)</td><td>420
        //!     (NV12)</td><td>1</td><td>1</td></tr><tr><td>Capture/Camera pipe
        //!     enabled(Demosaic)</td><td>422H</td><td>2</td><td>1</td></tr><tr><td>Capture/Camera
        //!     pipe
        //!     enabled(Demosaic)</td><td>444</td><td>4</td><td>1</td></tr></tbody></table>
        enum SFC_INPUT_CHROMA_SUB_SAMPLING
        {
            SFC_INPUT_CHROMA_SUB_SAMPLING_400                                = 0, //!< SFC to insert UV channels
            SFC_INPUT_CHROMA_SUB_SAMPLING_420                                = 1, //!< No additional details
            SFC_INPUT_CHROMA_SUB_SAMPLING_422HORIZONATAL                     = 2, //!< VD: 2:1:1
            SFC_INPUT_CHROMA_SUB_SAMPLING_444PROGRESSIVEINTERLEAVED          = 4, //!< No additional details
            SFC_INPUT_CHROMA_SUB_SAMPLING_411                                = 5, //!< No additional details
        };

        //! \brief VDVE_INPUT_ORDERING_MODE
        //! \details
        //!     VD mode: (SFC pipe mode set as "0")
        //!      VE mode:  (pipe mode set as "1 and 4")
        //!      For
        //!     values for each mode, please refer to the table below:
        enum VDVE_INPUT_ORDERING_MODE
        {
            VDVE_INPUT_ORDERING_MODE_UNNAMED0                                = 0, //!< AV1 64x64 NonIBC (shifted) [Shifted by 8 pixels)
            VDVE_INPUT_ORDERING_MODE_UNNAMED1                                = 1, //!< AV1 128x128 NonIBC (shifted)[Shifted by 8 pixels)
            VDVE_INPUT_ORDERING_MODE_UNNAMED2                                = 2, //!< AV1 64x64 IBC (unshifted)
            VDVE_INPUT_ORDERING_MODE_UNNAMED3                                = 3, //!< AV1 128x128 IBC (unshifted)
            VDVE_INPUT_ORDERING_MODE_UNNAMED4                                = 4, //!< 16x16 block VP8 row-scan order - no shift
        };

        //! \brief SFC_ENGINE_MODE
        //! \details
        //!     If SFC Pipe Mode is HCP-to-SFC and VE-to-SFC modes .Programmer need
        //!     to ensure SFC Engine Mode bits programmed is the same as
        //!     HCP_PIPE_MODE_SELECT command, Multi-Engine Modebits. For VE-SFC mode,
        //!     this field has to be programmed according to the position of the
        //!     split
        //!       This Field is ignored for other SFC Pipe Mode.
        enum SFC_ENGINE_MODE
        {
            SFC_ENGINE_MODE_SINGLESFC                                        = 0, //!< Single SFC mode.
            SFC_ENGINE_MODE_LEFTMOSTSFC                                      = 1, //!< Left Most SFC in scalability/split-frame mode.
            SFC_ENGINE_MODE_RIGHTMOSTSFC                                     = 2, //!< Right Most SFC in scalability/split-frame mode.
            SFC_ENGINE_MODE_MIDDLESFC                                        = 3, //!< Middle SFC in scalability/split-frame mode.
        };

        //! \brief INPUT_FRAME_DATA_FORMAT
        //! \details
        //!     This field specifies the format of Input frame according the
        //!     following table.
        enum INPUT_FRAME_DATA_FORMAT
        {
            INPUT_FRAME_DATA_FORMAT_PROGRESSIVE                              = 0, //!< Progressive - Frame has progressive data.Valid in VD+SFC, VE+SFC and HCP+SFC engine modes.
            INPUT_FRAME_DATA_FORMAT_INTERLEAVED                              = 1, //!< Interleaved - Frame has top and bottom field data interleaved.Top and Bottom field are interleaved. Valid only in VE+SFC mode.
            INPUT_FRAME_DATA_FORMAT_FIELDMODE                                = 2, //!< Field mode - Frame has interlaced data where top field and bottom field are processed a separate frame.Valid in VD+SFC, VE+SFC and HCP+SFC engine modes.
        };

        //! \brief OUTPUT_FRAME_DATA_FORMAT
        //! \details
        //!     This field specifies the format of Output Frame according the
        //!     following table.
        enum OUTPUT_FRAME_DATA_FORMAT
        {
            OUTPUT_FRAME_DATA_FORMAT_PROGRESSIVE                             = 0, //!< Progressive - Frame has progressive data.
            OUTPUT_FRAME_DATA_FORMAT_INTERLEAVED                             = 1, //!< Interleaved - Frame has top and bottom field data interleaved.
            OUTPUT_FRAME_DATA_FORMAT_FIELDMODE                               = 2, //!< Field mode - Frame has interlaced data where top field and bottom field are processed as a separate frame.
        };

        //! \brief OUTPUT_SURFACE_FORMAT_TYPE
        //! \details
        //!     SFC output surface format type.
        enum OUTPUT_SURFACE_FORMAT_TYPE
        {
            OUTPUT_SURFACE_FORMAT_TYPE_AYUV                                  = 0, //!< AYUV 4:4:4 (8:8:8:8 MSB-A:Y:U:V)
            OUTPUT_SURFACE_FORMAT_TYPE_A8B8G8R8                              = 1, //!< RGBA8 4:4:4:4 (8:8:8:8 MSB-A:B:G:R)
            OUTPUT_SURFACE_FORMAT_TYPE_A2R10G10B10                           = 2, //!< RGBA10 10:10:10:2 (2:10:10:10 MSB-A:R:G:B)
            OUTPUT_SURFACE_FORMAT_TYPE_R5G6B5                                = 3, //!< RGB 5:6:5 (5:6:5 MSB-R:G:B)
            OUTPUT_SURFACE_FORMAT_TYPE_NV12                                  = 4, //!< Planar NV12 4:2:0 8-bit
            OUTPUT_SURFACE_FORMAT_TYPE_YUYV                                  = 5, //!< Packed YUYV 4:2:2 8-bit
            OUTPUT_SURFACE_FORMAT_TYPE_UYVY                                  = 6, //!< Packed UYVY 4:2:2 8-bit
            OUTPUT_SURFACE_FORMAT_TYPE_INTEGRAL_32                           = 7, //!< Packed integral Image 32-bit
            OUTPUT_SURFACE_FORMAT_TYPE_INTEGRAL_64                           = 8, //!< Packed integral Image 64-bit
            OUTPUT_SURFACE_FORMAT_TYPE_P016                                  = 9, //!< P016 format
            OUTPUT_SURFACE_FORMAT_TYPE_Y216                                  = 10, //!< Y210 / Y216 FormatBitDepth = 0 => Y210BitDepth = 1 => Y216
            OUTPUT_SURFACE_FORMAT_TYPE_Y416                                  = 11, //!< Y410 / Y416 FormatBitDepth = 0 => Y410BitDepth = 1 => Y416
            OUTPUT_SURFACE_FORMAT_TYPE_A8B8G8R83                             = 13, //!< Y8_NORM
            OUTPUT_SURFACE_FORMAT_TYPE_A8B8G8R84                             = 14, //!< Y16_NORM
            OUTPUT_SURFACE_FORMAT_TYPE_A8B8G8R85                             = 15, //!< R16G16B16A16
        };

        //! \brief OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION
        //! \details
        //!     This field specifies the fractional position of the bilinear filter for
        //!     chroma downsampling. In the Y-axis.
        enum OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION
        {
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_08_LEFTFULLPIXEL = 0, //!< 0 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_18 = 1, //!< 1 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_14_28 = 2, //!< 2 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_38 = 3, //!< 3 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_12_48 = 4, //!< 4 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_58 = 5, //!< 5 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_34_68 = 6, //!< 6 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_78 = 7, //!< 7 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_88 = 8, //!< No additional details
        };

        //! \brief OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION
        //! \details
        //!     This field specifies the fractional position of the bilinear filter for
        //!     chroma downsampling. In the X-axis.
        enum OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION
        {
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_08_LEFTFULLPIXEL = 0, //!< 0 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_18 = 1, //!< 1 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_14_28 = 2, //!< 2 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_38 = 3, //!< 3 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_12_48 = 4, //!< 4 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_58 = 5, //!< 5 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_34_68 = 6, //!< 6 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_78 = 7, //!< 7 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_88 = 8, //!< No additional details
        };

        //! \brief INPUT_COLOR_SPACE_0_YUV1_RGB
        //! \details
        //!     THis specifies the color space of the input format. RGB is valid only
        //!     with the VE-SFC mode.
        enum INPUT_COLOR_SPACE_0_YUV1_RGB
        {
            INPUT_COLOR_SPACE_0_YUV1_RGB_YUVCOLORSPACE                       = 0, //!< No additional details
            INPUT_COLOR_SPACE_0_YUV1_RGB_RGBCOLORSPACE                       = 1, //!< No additional details
        };

        //! \brief OUTPUT_COMPRESSION_FORMAT
        //! \details
        //!     Specifies the 5-bit compression format.
        enum OUTPUT_COMPRESSION_FORMAT
        {
            OUTPUT_COMPRESSION_FORMAT_RGBA16FLOAT                            = 1, //!< No additional details
            OUTPUT_COMPRESSION_FORMAT_Y210                                   = 2, //!< No additional details
            OUTPUT_COMPRESSION_FORMAT_YUY2                                   = 3, //!< No additional details
            OUTPUT_COMPRESSION_FORMAT_Y410_1010102                           = 4, //!< No additional details
            OUTPUT_COMPRESSION_FORMAT_Y216                                   = 5, //!< No additional details
            OUTPUT_COMPRESSION_FORMAT_Y416                                   = 6, //!< No additional details
            OUTPUT_COMPRESSION_FORMAT_P010                                   = 7, //!< Luma P010 has MSB of 0 while chroma P010 has MSB of 1.
            OUTPUT_COMPRESSION_FORMAT_P016                                   = 8, //!< No additional details
            OUTPUT_COMPRESSION_FORMAT_AYUV                                   = 9, //!< No additional details
            OUTPUT_COMPRESSION_FORMAT_ARGB8B                                 = 10, //!< No additional details
            OUTPUT_COMPRESSION_FORMAT_YCRCBSWAPY                             = 11, //!< No additional details
            OUTPUT_COMPRESSION_FORMAT_YCRCBSWAPUV                            = 12, //!< No additional details
            OUTPUT_COMPRESSION_FORMAT_YCRCBSWAPUVY                           = 13, //!< No additional details
            OUTPUT_COMPRESSION_FORMAT_RGB10B                                 = 14, //!< No additional details
            OUTPUT_COMPRESSION_FORMAT_NV21NV12                               = 15, //!< Luma NV12 has MSB of 0 while chroma NV12 has MSB of 1.
        };

        //! \brief IEF_ENABLE
        //! \details
        //!     Restriction : For Integral Image Mode and VD Mode, this field is
        //!     Reserved and MBZ.
        enum IEF_ENABLE
        {
            IEF_ENABLE_DISABLE                                               = 0, //!< IEF Filter is Disabled
            IEF_ENABLE_ENABLE                                                = 1, //!< IEF Filter is Enabled
        };

        //! \brief IEF4SMOOTH_ENABLE_
        //! \details
        //!     Restriction : For Integral Image Mode, this field is Reserved and MBZ.
        enum IEF4SMOOTH_ENABLE_
        {
            IEF4SMOOTH_ENABLE_UNNAMED0                                       = 0, //!< IEF is operating as a content adaptive detail filter based on 5x5 region.
            IEF4SMOOTH_ENABLE_UNNAMED1                                       = 1, //!< IEF is operating as a content adaptive smooth filter based on 3x3 region
        };

        //! \brief AVS_FILTER_MODE
        //! \details
        //!     In VD-to-SFC mode, value of 1 is not allowed.
        enum AVS_FILTER_MODE
        {
            AVS_FILTER_MODE_5X5POLY_PHASEFILTERBILINEAR_ADAPTIVE             = 0, //!< No additional details
            AVS_FILTER_MODE_8X8POLY_PHASEFILTERBILINEAR_ADAPTIVE             = 1, //!< No additional details
            AVS_FILTER_MODE_BILINEARFILTERONLY                               = 2, //!< No additional details
        };

        //! \brief ADAPTIVE_FILTER_FOR_ALL_CHANNELS
        //! \details
        //!     The field can be enabled if 8-tap Adaptive filter mode is on. Else it
        //!     should be disabled.
        enum ADAPTIVE_FILTER_FOR_ALL_CHANNELS
        {
            ADAPTIVE_FILTER_FOR_ALL_CHANNELS_DISABLEADAPTIVEFILTERONUVRBCHANNELS = 0, //!< No additional details
            ADAPTIVE_FILTER_FOR_ALL_CHANNELS_ENABLEADAPTIVEFILTERONUVRBCHANNELS = 1, //!< 8-tap Adaptive Filter Mode is on
        };

        enum AVS_SCALING_ENABLE
        {
            AVS_SCALING_ENABLE_DISABLE                                       = 0, //!< The scaling factor is ignored and a scaling ratio of 1:1 is assumed.
            AVS_SCALING_ENABLE_ENABLE                                        = 1, //!< No additional details
        };

        enum BYPASS_Y_ADAPTIVE_FILTERING
        {
            BYPASS_Y_ADAPTIVE_FILTERING_ENABLEYADAPTIVEFILTERING             = 0, //!< No additional details
            BYPASS_Y_ADAPTIVE_FILTERING_DISABLEYADAPTIVEFILTERING            = 1, //!< The Y direction will use Default Sharpness Level to blend between the smooth and sharp filters rather than the calculated value.
        };

        enum BYPASS_X_ADAPTIVE_FILTERING
        {
            BYPASS_X_ADAPTIVE_FILTERING_ENABLEXADAPTIVEFILTERING             = 0, //!< No additional details
            BYPASS_X_ADAPTIVE_FILTERING_DISABLEXADAPTIVEFILTERING            = 1, //!< The X direction will use Default Sharpness Level to blend between the smooth and sharp filters rather than the calculated value.
        };

        //! \brief MIRROR_TYPE
        //! \details
        //!     0 : Horizontal flip 1 : Vertical flip
        enum MIRROR_TYPE
        {
            MIRROR_TYPE_HORIZONTALFLIP                                       = 0, //!< No additional details
            MIRROR_TYPE_VERTICALFLIP                                         = 1, //!< No additional details
        };

        //! \brief MIRROR_MODE
        //! \details
        //!     0 : Mirror Mode disabled 1 : Mirror Mode enabled
        enum MIRROR_MODE
        {
            MIRROR_MODE_MIRRORMODEDISABLED                                   = 0, //!< No additional details
            MIRROR_MODE_MIRRORMODEENABLED                                    = 1, //!< No additional details
        };

        //! \brief ROTATION_MODE
        //! \details
        //!     SFC rotation (90, 180 and 270) should be set only on VEBox input mode
        //!     and SFC output set to TileY.Restriction: For Integral Image
        //!     Mode, this field is Reserved and MBZ.
        //!    For VDBox Mode, this field
        //!     is Reserved and MBZ.
        //!    For linear or TileX SFC output, this field
        //!     is Reserved and MBZ.

        enum ROTATION_MODE
        {
            ROTATION_MODE_0_DEGREES                                          = 0, //!< No additional details
            ROTATION_MODE_90CLOCKWISE                                        = 1, //!< No additional details
            ROTATION_MODE_180CLOCKWISE                                       = 2, //!< No additional details
            ROTATION_MODE_270CLOCKWISE                                       = 3, //!< No additional details
        };

        //! \brief BITDEPTH
        //! \details
        //!     This field is valid only for output formats P016/Y216/Y416. This field
        //!     is used to specify how many of the LSB bits have valid data.
        enum BITDEPTH
        {
            BITDEPTH_10BITFORMAT                                             = 0, //!< Higher 10 bits are valid and lower 6 bits are 0
        };

        //! \brief COMPRESSION_TYPE
        //! \details
        //!     This field is applicable only when Memory compression is enabled.
        enum COMPRESSION_TYPE
        {
            COMPRESSION_TYPE_MEDIACOMPRESSIONENABLED                         = 0, //!< No additional details
            COMPRESSION_TYPE_RENDERCOMPRESSIONENABLED                        = 1, //!< No additional details
        };

        //! \brief OUTPUT_FRAME_SURFACE_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        //! \details
        //!     This must be set to 0
        enum OUTPUT_FRAME_SURFACE_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        {
            OUTPUT_FRAME_SURFACE_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_DISABLE = 0, //!< This field must be programmed to 0
        };

        //! \brief AVS_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     This bit control memory compression for this surface
        enum AVS_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
        {
            AVS_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE   = 0, //!< No additional details
        };

        //! \brief AVS_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Distinguishes
        //!     vertical from horizontal compression. Please refer to vol1a?
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Memory Data Formats
        //!     chapter - section  style="color: rgb(35, 35, 35);
        //!     font-family: Arial, sans-serif; font-size: 13.3333330154419px;
        //!     line-height: normal;">?media Memory Compression for more details.
        enum AVS_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
        {
            AVS_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< Memory compression is not supported for this surface. ?This bit just defaults to 0
        };

        //! \brief AVS_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">This field controls
        //!     if the Row Store is going to store inside Media Cache (rowstore cache)
        //!     or to LLC.
        enum AVS_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        {
            AVS_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC = 0, //!< Buffer going to LLC
        };

        //! \brief IEF_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial,
        //!     sans-serif; font-size: 13.3333330154419px; line-height: normal;">Memory
        //!     compression is not supported for this surface
        //!        style="color: rgb(35, 35, 35); font-family:
        //!     Arial, sans-serif; font-size: 13.3333330154419px; line-height:
        //!     normal;">Must be 0./>
        enum IEF_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
        {
            IEF_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE   = 0, //!< No additional details
        };

        //! \brief IEF_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Distinguishes
        //!     vertical from horizontal compression.
        enum IEF_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
        {
            IEF_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0    = 0, //!< No additional details
        };

        //! \brief IEF_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">This field controls
        //!     if the Row Store is going to store inside Media Cache (rowstore cache)
        //!     or to LLC.
        enum IEF_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        {
            IEF_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC = 0, //!< Buffer going to LLC
        };

        //! \brief SFD_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial,
        //!     sans-serif; font-size: 13.3333330154419px; line-height: normal;">Memory
        //!     compression is not supported for this surface
        //!        style="color: rgb(35, 35, 35); font-family:
        //!     Arial, sans-serif; font-size: 13.3333330154419px; line-height:
        //!     normal;">Must be 0./>
        enum SFD_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
        {
            SFD_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE   = 0, //!< No additional details
        };

        //! \brief SFD_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Distinguishes
        //!     vertical from horizontal compression. Please refer to vol1a
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Memory Data Formats
        //!     chapter - section  style="color: rgb(35, 35, 35);
        //!     font-family: Arial, sans-serif; font-size: 13.3333330154419px;
        //!     line-height: normal;"> media Memory Compression for more details.
        enum SFD_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
        {
            SFD_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0    = 0, //!< No additional details
        };

        //! \brief SFD_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">This field controls
        //!     if the Row Store is going to store inside Media Cache (rowstore cache)
        //!     or to LLC.
        enum SFD_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        {
            SFD_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC = 0, //!< Buffer going to LLC
            SFD_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_MEDIASTORAGE = 1, //!< style="margin:0in 0in 8pt">        Data will first cache in Media Storage        
        };

        //! \brief TILED_MODE
        //! \details
        //!     Indicates the Tile Mode for the Surface.
        enum TILED_MODE
        {
            TILED_MODE_LINEAR                                                = 0, //!< No additional details
            TILED_MODE_TILES_64K                                             = 1, //!< No additional details
            TILED_MODE_XMAJOR                                                = 2, //!< No additional details
            TILED_MODE_TILEF                                                 = 3, //!< No additional details
        };

        //! \brief AVS_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial,
        //!     sans-serif; font-size: 13.3333330154419px; line-height: normal;">Memory
        //!     compression is not supported for this surface
        //!        style="color: rgb(35, 35, 35); font-family:
        //!     Arial, sans-serif; font-size: 13.3333330154419px; line-height:
        //!     normal;">Must be 0./>
        enum AVS_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
        {
            AVS_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE = 0, //!< No additional details
        };

        //! \brief AVS_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Distinguishes
        //!     vertical from horizontal compression. Please refer to vol1a
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Memory Data Formats
        //!     chapter - section  style="color: rgb(35, 35, 35);
        //!     font-family: Arial, sans-serif; font-size: 13.3333330154419px;
        //!     line-height: normal;"> media Memory Compression for more details.
        enum AVS_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
        {
            AVS_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0 = 0, //!< No additional details
        };

        //! \brief AVS_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">This field controls
        //!     if the Row Store is going to store inside Media Cache (rowstore cache)
        //!     or to LLC.
        enum AVS_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        {
            AVS_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC = 0, //!< Buffer going to LLC
        };

        //! \brief IEF_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial,
        //!     sans-serif; font-size: 13.3333330154419px; line-height: normal;">Memory
        //!     compression is not supported for this surface
        //!        style="color: rgb(35, 35, 35); font-family:
        //!     Arial, sans-serif; font-size: 13.3333330154419px; line-height:
        //!     normal;">Must be 0./>
        enum IEF_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
        {
            IEF_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE = 0, //!< No additional details
        };

        //! \brief IEF_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Distinguishes
        //!     vertical from horizontal compression. Please refer to vol1a
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Memory Data Formats
        //!     chapter - section  style="color: rgb(35, 35, 35);
        //!     font-family: Arial, sans-serif; font-size: 13.3333330154419px;
        //!     line-height: normal;"> media Memory Compression for more details.
        enum IEF_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
        {
            IEF_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0 = 0, //!< No additional details
        };

        //! \brief IEF_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">This field controls
        //!     if the Row Store is going to store inside Media Cache (rowstore cache)
        //!     or to LLC.
        enum IEF_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        {
            IEF_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC = 0, //!< Buffer going to LLC
        };

        //! \brief SFD_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial,
        //!     sans-serif; font-size: 13.3333330154419px; line-height: normal;">Memory
        //!     compression is not supported for this surface
        //!        style="color: rgb(35, 35, 35); font-family:
        //!     Arial, sans-serif; font-size: 13.3333330154419px; line-height:
        //!     normal;">Must be 0./>
        enum SFD_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
        {
            SFD_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE = 0, //!< No additional details
        };

        //! \brief SFD_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Distinguishes
        //!     vertical from horizontal compression. Please refer to vol1a
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Memory Data Formats
        //!     chapter - section  style="color: rgb(35, 35, 35);
        //!     font-family: Arial, sans-serif; font-size: 13.3333330154419px;
        //!     line-height: normal;"> media Memory Compression for more details.
        enum SFD_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE
        {
            SFD_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0 = 0, //!< No additional details
        };

        //! \brief SFD_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">This field controls
        //!     if the Row Store is going to store inside Media Cache (rowstore cache)
        //!     or to LLC.
        enum SFD_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        {
            SFD_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC = 0, //!< Buffer going to LLC
        };

        //! \brief HISTOGRAM_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial,
        //!     sans-serif; font-size: 13.3333330154419px; line-height: normal;">Memory
        //!     compression is not supported for this surface
        //!        style="color: rgb(35, 35, 35); font-family:
        //!     Arial, sans-serif; font-size: 13.3333330154419px; line-height:
        //!     normal;">Must be 0./>
        enum HISTOGRAM_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE
        {
            HISTOGRAM_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE         = 0, //!< No additional details
        };

        //! \brief HISTOGRAM_BASE_ADDRESS_MEMORY_COMPRESSION_TYPE
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Distinguishes Media
        //!     or 3D compression.   style="color: rgb(35, 35, 35);
        //!     font-family: Arial, sans-serif; font-size: 13.3333330154419px;
        //!     line-height: normal;">Memory Data Formats chapter - section
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;"> media Memory
        //!     Compression for more details.
        enum HISTOGRAM_BASE_ADDRESS_MEMORY_COMPRESSION_TYPE
        {
            HISTOGRAM_BASE_ADDRESS_MEMORY_COMPRESSION_TYPE_UNNAMED0          = 0, //!< No additional details
        };

        //! \brief HISTOGRAM_BASE_ADDRESS_CACHE_SELECT
        //! \details
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">This field controls
        //!     if the Histogram need to be cached in LLC or not.
        enum HISTOGRAM_BASE_ADDRESS_CACHE_SELECT
        {
            HISTOGRAM_BASE_ADDRESS_CACHE_SELECT_LLC                          = 0, //!< Buffer going to LLC
        };

        //! \brief BOTTOM_FILED_SURFACE_BASE_ADDRESS_MEMORY_COMPRESSION_TYPE
        //! \details
        //!     This field is applicable only when memory compression is enabled
        enum BOTTOM_FILED_SURFACE_BASE_ADDRESS_MEMORY_COMPRESSION_TYPE
        {
            BOTTOM_FILED_SURFACE_BASE_ADDRESS_MEMORY_COMPRESSION_TYPE_MEDIACOMPRESSIONENABLED = 0, //!< No additional details
            BOTTOM_FILED_SURFACE_BASE_ADDRESS_MEMORY_COMPRESSION_TYPE_RENDERCOMPRESSIONENABLED = 1, //!< No additional details
        };

        //! \brief BOTTOM_FIELD_SURFACE_TILE_WALK
        //! \details
        //!     This field specifies the type of memory tiling (XMajor or YMajor)
        //!     employed to tile this surface. See Memory Interface Functions for
        //!     details on memory tiling and restrictions.
        enum BOTTOM_FIELD_SURFACE_TILE_WALK
        {
            BOTTOM_FIELD_SURFACE_TILE_WALK_TILEWALKXMAJOR                    = 0, //!< No additional details
            BOTTOM_FIELD_SURFACE_TILE_WALK_TILEWALKYMAJOR                    = 1, //!< No additional details
        };

        //! \brief BOTTOM_FIELD_SURFACE_TILED
        //! \details
        //!     This field specifies whether the surface is tiled.
        enum BOTTOM_FIELD_SURFACE_TILED
        {
            BOTTOM_FIELD_SURFACE_TILED_FALSE                                 = 0, //!< Linear
            BOTTOM_FIELD_SURFACE_TILED_TRUE                                  = 1, //!< Tiled
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        SFC_STATE_CMD()
        {
            DW0.Value                                        = 0x7501003b;
            //DW0.DwordLength                                  = GetOpLength(dwSize);
            //DW0.Subopcodeb                                   = SUBOPCODEB_SFCSTATE;
            //DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
            //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMFXVEBOXSFCMODE;
            //DW0.Pipeline                                     = PIPELINE_MEDIA;
            //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

            DW1.Value                                        = 0x00000000;
            //DW1.SfcPipeMode                                  = SFC_PIPE_MODE_UNNAMED0;
            //DW1.SfcInputChromaSubSampling                    = SFC_INPUT_CHROMA_SUB_SAMPLING_400;
            //DW1.VdVeInputOrderingMode                        = VDVE_INPUT_ORDERING_MODE_UNNAMED0;
            //DW1.SfcEngineMode                                = SFC_ENGINE_MODE_SINGLESFC;
            //DW1.InputFrameDataFormat                         = INPUT_FRAME_DATA_FORMAT_PROGRESSIVE;
            //DW1.OutputFrameDataFormat                        = OUTPUT_FRAME_DATA_FORMAT_PROGRESSIVE;

            DW2.Value                                        = 0x00000000;

            DW3.Value                                        = 0x00000000;
            //DW3.OutputSurfaceFormatType                      = OUTPUT_SURFACE_FORMAT_TYPE_AYUV;
            //DW3.OutputChromaDownsamplingCoSitingPositionVerticalDirection = OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_08_LEFTFULLPIXEL;
            //DW3.OutputChromaDownsamplingCoSitingPositionHorizontalDirection = OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_08_LEFTFULLPIXEL;
            //DW3.InputColorSpace0Yuv1Rgb                      = INPUT_COLOR_SPACE_0_YUV1_RGB_YUVCOLORSPACE;
            //DW3.OutputCompressionFormat                      = 0;

            DW4.Value                                        = 0x00000000;
            //DW4.IefEnable                                    = IEF_ENABLE_DISABLE;
            //DW4.Ief4SmoothEnable                             = IEF4SMOOTH_ENABLE_UNNAMED0;
            //DW4.AvsFilterMode                                = AVS_FILTER_MODE_5X5POLY_PHASEFILTERBILINEAR_ADAPTIVE;
            //DW4.AdaptiveFilterForAllChannels                 = ADAPTIVE_FILTER_FOR_ALL_CHANNELS_DISABLEADAPTIVEFILTERONUVRBCHANNELS;
            //DW4.AvsScalingEnable                             = AVS_SCALING_ENABLE_DISABLE;
            //DW4.BypassYAdaptiveFiltering                     = BYPASS_Y_ADAPTIVE_FILTERING_ENABLEYADAPTIVEFILTERING;
            //DW4.BypassXAdaptiveFiltering                     = BYPASS_X_ADAPTIVE_FILTERING_ENABLEXADAPTIVEFILTERING;
            //DW4.MirrorType                                   = MIRROR_TYPE_HORIZONTALFLIP;
            //DW4.MirrorMode                                   = MIRROR_MODE_MIRRORMODEDISABLED;
            //DW4.RotationMode                                 = ROTATION_MODE_0_DEGREES;
            //DW4.Bitdepth                                     = BITDEPTH_10BITFORMAT;

            DW5.Value                                        = 0x00000000;

            DW6.Value                                        = 0x00000000;

            DW7.Value                                        = 0x00000000;

            DW8.Value                                        = 0x00000000;

            DW9.Value                                        = 0x00000000;

            DW10.Value                                        = 0x00000000;

            DW11.Value                                       = 0x00000000;

            DW12.Value                                       = 0x00000000;

            DW13.Value                                       = 0x00000000;

            DW14.Value                                       = 0x00000000;

            DW15.Value                                       = 0x00000000;

            DW16.Value                                       = 0x00000000;

            DW17.Value                                       = 0x00000000;

            DW18.Value                                       = 0x00000000;

            DW19.Value                                       = 0x00000000;
            //DW19.CompressionType                             = COMPRESSION_TYPE_MEDIACOMPRESSIONENABLED;
            //DW19.OutputFrameSurfaceBaseAddressRowStoreScratchBufferCacheSelect = OUTPUT_FRAME_SURFACE_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_DISABLE;

            DW20.Value                                       = 0x00000000;

            DW21.Value                                       = 0x00000000;

            DW22.Value                                       = 0x00000000;
            //DW22.AvsLineBufferBaseAddressMemoryCompressionEnable = AVS_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE;
            //DW22.AvsLineBufferBaseAddressMemoryCompressionMode = AVS_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE;
            //DW22.AvsLineBufferBaseAddressRowStoreScratchBufferCacheSelect = AVS_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;

            DW23.Value                                       = 0x00000000;

            DW24.Value                                       = 0x00000000;

            DW25.Value                                       = 0x00000000;
            //DW25.IefLineBufferBaseAddressMemoryCompressionEnable = IEF_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE;
            //DW25.IefLineBufferBaseAddressMemoryCompressionMode = IEF_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0;
            //DW25.IefLineBufferBaseAddressRowStoreScratchBufferCacheSelect = IEF_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;

            DW26.Value                                       = 0x00000000;

            DW27.Value                                       = 0x00000000;

            DW28.Value                                       = 0x00000000;
            //DW28.SfdLineBufferBaseAddressMemoryCompressionEnable = SFD_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE;
            //DW28.SfdLineBufferBaseAddressMemoryCompressionMode = SFD_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0;
            //DW28.SfdLineBufferBaseAddressRowStoreScratchBufferCacheSelect = SFD_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;

            DW29.Value                                       = 0x00000000;
            //DW29.TiledMode                                   = TILED_MODE_LINEAR;

            DW30.Value                                       = 0x00000000;

            DW31.Value                                       = 0x00000000;

            DW32.Value                                       = 0x00000000;

            DW33.Value                                       = 0x00000000;

            DW34.Value                                       = 0x00000000;

            DW35.Value                                       = 0x00000000;

            DW36.Value                                       = 0x00000000;

            DW37.Value                                       = 0x00000000;

            DW38.Value                                       = 0x00000000;

            DW39.Value                                       = 0x00000000;

            DW40.Value                                       = 0x00000000;
            //DW40.AvsLineTileBufferBaseAddressMemoryCompressionEnable = AVS_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE;
            //DW40.AvsLineTileBufferBaseAddressMemoryCompressionMode = AVS_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0;
            //DW40.AvsLineTileBufferBaseAddressRowStoreScratchBufferCacheSelect = AVS_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;

            DW41.Value                                       = 0x00000000;

            DW42.Value                                       = 0x00000000;

            DW43.Value                                       = 0x00000000;
            //DW43.IefLineTileBufferBaseAddressMemoryCompressionEnable = IEF_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE;
            //DW43.IefLineTileBufferBaseAddressMemoryCompressionMode = IEF_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0;
            //DW43.IefLineTileBufferBaseAddressRowStoreScratchBufferCacheSelect = IEF_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;

            DW44.Value                                       = 0x00000000;

            DW45.Value                                       = 0x00000000;

            DW46.Value                                       = 0x00000000;
            //DW46.SfdLineTileBufferBaseAddressMemoryCompressionEnable = SFD_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE;
            //DW46.SfdLineTileBufferBaseAddressMemoryCompressionMode = SFD_LINE_TILE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0;
            //DW46.SfdLineTileBufferBaseAddressRowStoreScratchBufferCacheSelect = SFD_LINE_TILE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC;

            DW47.Value                                       = 0x00000000;

            DW48.Value                                       = 0x00000000;

            DW49.Value                                       = 0x00000000;
            //DW49.HistogramBaseAddressMemoryCompressionEnable = HISTOGRAM_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE;
            //DW49.HistogramBaseAddressMemoryCompressionType   = HISTOGRAM_BASE_ADDRESS_MEMORY_COMPRESSION_TYPE_UNNAMED0;
            //DW49.HistogramBaseAddressCacheSelect             = HISTOGRAM_BASE_ADDRESS_CACHE_SELECT_LLC;

            DW50.Value                                       = 0x00000000;

            DW51.Value                                       = 0x00000000;

            DW52.Value                                       = 0x00000000;

            DW53.Value                                       = 0x00000000;

            DW54.Value                                       = 0x00000000;

            DW55.Value                                       = 0x00000000;

            DW56.Value                                       = 0x00000000;

            DW57.Value                                       = 0x00000000;
            //DW57.BottomFiledSurfaceBaseAddressMemoryCompressionType = BOTTOM_FILED_SURFACE_BASE_ADDRESS_MEMORY_COMPRESSION_TYPE_MEDIACOMPRESSIONENABLED;

            DW58.Value                                       = 0x00000000;
            //DW58.BottomFieldSurfaceTileWalk                  = BOTTOM_FIELD_SURFACE_TILE_WALK_TILEWALKXMAJOR;
            //DW58.BottomFieldSurfaceTiled                     = BOTTOM_FIELD_SURFACE_TILED_FALSE;

            DW59.Value                                       = 0x00000000;

            DW60.Value                                       = 0x00000000;

        }


        static const size_t dwSize = 61;
        static const size_t byteSize = 244;
    };

    //!
    //! \brief SFC_AVS_LUMA_Coeff_Table
    //! \details
    //!     This command is sent from VDBOX/VEBOX to SFC pipeline at the start of
    //!     each frame once the lock request is granted.
    //!
    struct SFC_AVS_LUMA_Coeff_Table_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopcodeb                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODEB
                uint32_t                 Subopcodea                                       : __CODEGEN_BITFIELD(21, 22)    ; //!< SUBOPCODEA
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(23, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 Table0XFilterCoefficientN0                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Table 0X Filter Coefficient[[n],0]
                uint32_t                 Table0YFilterCoefficientN0                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Table 0Y Filter Coefficient[[n],0]
                uint32_t                 Table0XFilterCoefficientN1                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Table 0X Filter Coefficient[[n],1]
                uint32_t                 Table0YFilterCoefficientN1                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Table 0Y Filter Coefficient[[n],1]
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 Table0XFilterCoefficientN2                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Table 0X Filter Coefficient[[n],2]
                uint32_t                 Table0YFilterCoefficientN2                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Table 0Y Filter Coefficient[[n],2]
                uint32_t                 Table0XFilterCoefficientN3                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Table 0X Filter Coefficient[[n],3]
                uint32_t                 Table0YFilterCoefficientN3                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Table 0Y Filter Coefficient[[n],3]
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 Table0XFilterCoefficientN4                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Table 0X Filter Coefficient[[n],4]
                uint32_t                 Table0YFilterCoefficientN4                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Table 0Y Filter Coefficient[[n],4]
                uint32_t                 Table0XFilterCoefficientN5                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Table 0X Filter Coefficient[[n],5]
                uint32_t                 Table0YFilterCoefficientN5                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Table 0Y Filter Coefficient[[n],5]
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 Table0XFilterCoefficientN6                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Table 0X Filter Coefficient[[n],6]
                uint32_t                 Table0YFilterCoefficientN6                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Table 0Y Filter Coefficient[[n],6]
                uint32_t                 Table0XFilterCoefficientN7                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Table 0X Filter Coefficient[[n],7]
                uint32_t                 Table0YFilterCoefficientN7                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Table 0Y Filter Coefficient[[n],7]
            };
            uint32_t                     Value;
        } DW4;
        uint32_t                                 FilterCoefficients[124];                                                 //!< Filter Coefficients

        //! \name Local enumerations

        enum SUBOPCODEB
        {
            SUBOPCODEB_SFCAVSLUMACOEFFTABLE                                  = 5, //!< No additional details
        };

        enum SUBOPCODEA
        {
            SUBOPCODEA_COMMON                                                = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE                            = 9, //!< No additional details
            MEDIA_COMMAND_OPCODE_MEDIAMISC                                   = 10, //!< Media MFX/VEBOX+SFC Mode
            MEDIA_COMMAND_OPCODE_MEDIAAVPSFCMODE                             = 0xD, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MEDIA                                                   = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        SFC_AVS_LUMA_Coeff_Table_CMD()
        {
            DW0.Value                                        = 0x7505007f;
            //DW0.DwordLength                                  = GetOpLength(dwSize);
            //DW0.Subopcodeb                                   = SUBOPCODEB_SFCAVSLUMACOEFFTABLE;
            //DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
            //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMISC;
            //DW0.Pipeline                                     = PIPELINE_MEDIA;
            //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

            DW1.Value                                        = 0x00000000;

            DW2.Value                                        = 0x00000000;

            DW3.Value                                        = 0x00000000;

            DW4.Value                                        = 0x00000000;

            MOS_ZeroMemory(&FilterCoefficients, sizeof(FilterCoefficients));
        }


        static const size_t dwSize = 129;
        static const size_t byteSize = 516;
    };

    //!
    //! \brief SFC_AVS_CHROMA_Coeff_Table
    //! \details
    //!     This command is sent from VDBOX/VEBOX to SFC pipeline at the start of
    //!     each frame once the lock request is granted.
    //!
    struct SFC_AVS_CHROMA_Coeff_Table_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopcodeb                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODEB
                uint32_t                 Subopcodea                                       : __CODEGEN_BITFIELD(21, 22)    ; //!< SUBOPCODEA
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(23, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 Table1XFilterCoefficientN2                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Table 1X Filter Coefficient[[n],2]
                uint32_t                 Table1YFilterCoefficientN2                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Table 1Y Filter Coefficient[[n],2]
                uint32_t                 Table1XFilterCoefficientN3                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Table 1X Filter Coefficient[[n],3]
                uint32_t                 Table1YFilterCoefficientN3                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Table 1Y Filter Coefficient[[n],3]
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 Table1XFilterCoefficientN4                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Table 1X Filter Coefficient[[n],4]
                uint32_t                 Table1YFilterCoefficientN4                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Table 1Y Filter Coefficient[[n],4]
                uint32_t                 Table1XFilterCoefficientN5                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Table 1X Filter Coefficient[[n],5]
                uint32_t                 Table1YFilterCoefficientN5                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Table 1Y Filter Coefficient[[n],5]
            };
            uint32_t                     Value;
        } DW2;
        uint32_t                                 FilterCoefficients[62];                                                  //!< Filter Coefficients

        //! \name Local enumerations

        enum SUBOPCODEB
        {
            SUBOPCODEB_SFCAVSCHROMACOEFFTABLE                                = 6, //!< No additional details
        };

        enum SUBOPCODEA
        {
            SUBOPCODEA_COMMON                                                = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_MEDIAHEVCSFCMODE                            = 9, //!< No additional details
            MEDIA_COMMAND_OPCODE_MEDIAMISC                                   = 10, //!< Media MFX/VEBOX+SFC Mode
            MEDIA_COMMAND_OPCODE_MEDIAAVPSFCMODE                             = 0xD, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MEDIA                                                   = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        SFC_AVS_CHROMA_Coeff_Table_CMD()
        {
            DW0.Value                                        = 0x7506003f;
            //DW0.DwordLength                                  = GetOpLength(dwSize);
            //DW0.Subopcodeb                                   = SUBOPCODEB_SFCAVSCHROMACOEFFTABLE;
            //DW0.Subopcodea                                   = SUBOPCODEA_COMMON;
            //DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_MEDIAMISC;
            //DW0.Pipeline                                     = PIPELINE_MEDIA;
            //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

            DW1.Value                                        = 0x00000000;

            DW2.Value                                        = 0x00000000;

            MOS_ZeroMemory(&FilterCoefficients, sizeof(FilterCoefficients));
        }


        static const size_t dwSize = 65;
        static const size_t byteSize = 260;
    };
MEDIA_CLASS_DEFINE_END(mhw__sfc__xe_lpm_plus_next__Cmd)
};
}  // namespace xe_lpm_plus_next
}  // namespace sfc
}  // namespace mhw

#pragma pack()

#endif  // __MHW_SFC_HWCMD_XE_LPM_PLUS_H__
