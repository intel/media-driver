/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     mhw_sfc_hwcmd_g11_X.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of g11_X as other components
//!           should use MHW interface to interact with MHW commands and states.
//!
#ifndef __MHW_SFC_HWCMD_G11_X_H__
#define __MHW_SFC_HWCMD_G11_X_H__

#pragma once
#pragma pack(1)

#include <cstdint>
#include <cstddef>

class mhw_sfc_g11_X
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
            //!< DWORD 0
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
            //!< DWORD 1
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
            //!< DWORD 2
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
            //!< DWORD 3
            struct
            {
                uint32_t                 InputVerticalSitingSpecifiesTheVerticalSitingOfTheInput : __CODEGEN_BITFIELD( 0,  3)    ; //!< INPUT_VERTICAL_SITING__SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT
                uint32_t                 Reserved100                                      : __CODEGEN_BITFIELD( 4,  7)    ; //!< Reserved
                uint32_t                 InputHorizontalSitingValueSpecifiesTheHorizontalSitingOfTheInput : __CODEGEN_BITFIELD( 8, 11)    ; //!< INPUT_HORIZONTAL_SITING_VALUE__SPECIFIES_THE_HORIZONTAL_SITING_OF_THE_INPUT
                uint32_t                 Reserved108                                      : __CODEGEN_BITFIELD(12, 31)    ; //!< Reserved
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
            MEDIA_COMMAND_OPCODE_MEDIAMISC                                   = 10, //!< No additional details
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

        //! \brief INPUT_VERTICAL_SITING__SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT
        //! \details
        //!     For 444 and 422 format, vertical chroma siting should be programmed to
        //!     zero.
        enum INPUT_VERTICAL_SITING__SPECIFIES_THE_VERTICAL_SITING_OF_THE_INPUT
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

        //! \brief INPUT_HORIZONTAL_SITING_VALUE__SPECIFIES_THE_HORIZONTAL_SITING_OF_THE_INPUT
        //! \details
        //!     For 444 format, horizontal chroma siting should be programmed to zero.
        enum INPUT_HORIZONTAL_SITING_VALUE__SPECIFIES_THE_HORIZONTAL_SITING_OF_THE_INPUT
        {
            INPUT_HORIZONTAL_SITING_VALUE_SPECIFIES_THE_HORIZONTAL_SITING_OF_THE_INPUT_0_FRACTIONININTEGER = 0, //!< No additional details
            INPUT_HORIZONTAL_SITING_VALUE_SPECIFIES_THE_HORIZONTAL_SITING_OF_THE_INPUT_18 = 1, //!< No additional details
            INPUT_HORIZONTAL_SITING_VALUE_SPECIFIES_THE_HORIZONTAL_SITING_OF_THE_INPUT_28 = 2, //!< No additional details
            INPUT_HORIZONTAL_SITING_VALUE_SPECIFIES_THE_HORIZONTAL_SITING_OF_THE_INPUT_38 = 3, //!< No additional details
            INPUT_HORIZONTAL_SITING_VALUE_SPECIFIES_THE_HORIZONTAL_SITING_OF_THE_INPUT_48 = 4, //!< No additional details
            INPUT_HORIZONTAL_SITING_VALUE_SPECIFIES_THE_HORIZONTAL_SITING_OF_THE_INPUT_58 = 5, //!< No additional details
            INPUT_HORIZONTAL_SITING_VALUE_SPECIFIES_THE_HORIZONTAL_SITING_OF_THE_INPUT_68 = 6, //!< No additional details
            INPUT_HORIZONTAL_SITING_VALUE_SPECIFIES_THE_HORIZONTAL_SITING_OF_THE_INPUT_78 = 7, //!< No additional details
            INPUT_HORIZONTAL_SITING_VALUE_SPECIFIES_THE_HORIZONTAL_SITING_OF_THE_INPUT_88 = 8, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        SFC_AVS_STATE_CMD();

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
            //!< DWORD 0
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
            //!< DWORD 1
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
            //!< DWORD 2
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
            //!< DWORD 3
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
            //!< DWORD 4
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
            //!< DWORD 5
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
            //!< DWORD 6
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
            //!< DWORD 7
            struct
            {
                uint32_t                 InvMarginVyl                                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< INV_Margin_VYL
                uint32_t                 Reserved240                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
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
            //!< DWORD 9
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
            //!< DWORD 10
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
            //!< DWORD 11
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
            //!< DWORD 12
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
            //!< DWORD 13
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
            //!< DWORD 14
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
            //!< DWORD 15
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
            //!< DWORD 16
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
            //!< DWORD 17
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
            //!< DWORD 18
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
            //!< DWORD 19
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
            //!< DWORD 20
            struct
            {
                uint32_t                 C8                                               : __CODEGEN_BITFIELD( 0, 12)    ; //!< C8
                uint32_t                 Reserved653                                      : __CODEGEN_BITFIELD(13, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            //!< DWORD 21
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
            //!< DWORD 22
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
            //!< DWORD 23
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
            MEDIA_COMMAND_OPCODE_MEDIAMISC                                   = 10, //!< No additional details
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
            GAIN_FACTOR_UNNAMED_4_4                                          = 44, //!< No additional details
        };

        //! \brief WEAK_EDGE_THRESHOLD
        //! \details
        //!     If Strong Edge Threshold &gt; EM &gt; Weak Edge Threshold ? the basic
        //!     VSA detects a weak edge.
        enum WEAK_EDGE_THRESHOLD
        {
            WEAK_EDGE_THRESHOLD_UNNAMED1                                     = 1, //!< No additional details
        };

        //! \brief STRONG_EDGE_THRESHOLD
        //! \details
        //!     If EM &gt; Strong Edge Threshold ? the basic VSA detects a strong edge.
        enum STRONG_EDGE_THRESHOLD
        {
            STRONG_EDGE_THRESHOLD_UNNAMED8                                   = 8, //!< No additional details
        };

        //! \brief R3X_COEFFICIENT
        //! \details
        //!     IEF smoothing coefficient, <i>see IEF map.</i>
        enum R3X_COEFFICIENT
        {
            R3X_COEFFICIENT_UNNAMED5                                         = 5, //!< No additional details
        };

        //! \brief R3C_COEFFICIENT
        //! \details
        //!     IEF smoothing coefficient, <i>see IEF map.</i>
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
        //!     . Sharpening strength when <u>NO EDGE</u> is found in basic VSA.
        enum NON_EDGE_WEIGHT
        {
            NON_EDGE_WEIGHT_UNNAMED1                                         = 1, //!< No additional details
        };

        //! \brief REGULAR_WEIGHT
        //! \details
        //!     Sharpening strength when a <u>WEAK</u> edge is found in basic VSA.
        enum REGULAR_WEIGHT
        {
            REGULAR_WEIGHT_UNNAMED2                                          = 2, //!< No additional details
        };

        //! \brief STRONG_EDGE_WEIGHT
        //! \details
        //!     Sharpening strength when a <u>STRONG</u> edge is found in basic VSA.
        enum STRONG_EDGE_WEIGHT
        {
            STRONG_EDGE_WEIGHT_UNNAMED7                                      = 7, //!< No additional details
        };

        //! \brief R5X_COEFFICIENT
        //! \details
        //!     IEF smoothing coefficient, <i>see IEF map.</i>
        enum R5X_COEFFICIENT
        {
            R5X_COEFFICIENT_UNNAMED7                                         = 7, //!< No additional details
        };

        //! \brief R5CX_COEFFICIENT
        //! \details
        //!     IEF smoothing coefficient, <i>see IEF map.</i>
        enum R5CX_COEFFICIENT
        {
            R5CX_COEFFICIENT_UNNAMED7                                        = 7, //!< No additional details
        };

        //! \brief R5C_COEFFICIENT
        //! \details
        //!     IEF smoothing coefficient, <i>see IEF map.</i>
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
            HUE_MAX_UNNAMED1_4                                               = 14, //!< No additional details
        };

        enum DIAMOND_MARGIN
        {
            DIAMOND_MARGIN_UNNAMED_4                                         = 4, //!< No additional details
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
            V_MID_UNNAMED15_4                                                = 154, //!< No additional details
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
        //!     exists:
        //!                     <ul>
        //!                         <li>when the control bit <b>SkinToneTunedIEF_Enable</b> is on.
        //!     </li>
        //!                         <Li>When <b>SkinDetailFactor</b> is equal to 0,
        //!     sign(<b>SkinDetailFactor</b>) is equal to +1, and the content of the
        //!     detected skin tone area is detail revealed.</Li>
        //!                         <li>When <b>SkinDetailFactor</b> is equal to 1,
        //!     sign(<b>SkinDetailFactor</b>) is equal to -1, and the content of the
        //!     detected skin tone area is not detail revealed.</li>
        //!                     </ul>
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
            Y_POINT_1_UNNAMED_46                                             = 46, //!< No additional details
        };

        //! \brief Y_POINT_2
        //! \details
        //!     Second point of the Y piecewise linear membership function.
        enum Y_POINT_2
        {
            Y_POINT_2_UNNAMED_47                                             = 47, //!< No additional details
        };

        //! \brief Y_POINT_3
        //! \details
        //!     Third point of the Y piecewise linear membership function.
        enum Y_POINT_3
        {
            Y_POINT_3_UNNAMED25_4                                            = 254, //!< No additional details
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
            P0L_UNNAMED_46                                                   = 46, //!< No additional details
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
            P0U_UNNAMED_46                                                   = 46, //!< No additional details
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
            B0U_UNNAMED1_43                                                  = 143, //!< No additional details
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
            B3U_UNNAMED1_40                                                  = 140, //!< No additional details
        };

        //! \brief C0
        //! \details
        //!     Transform coefficient
        enum C0
        {
            C0_UNNAMED102_4                                                  = 1024, //!< No additional details
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
            C4_UNNAMED102_4                                                  = 1024, //!< No additional details
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
            C8_UNNAMED102_4                                                  = 1024, //!< No additional details
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
        SFC_IEF_STATE_CMD();

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
            //!< DWORD 0
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
            //!< DWORD 1
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
            MEDIA_COMMAND_OPCODE_MEDIAMISC                                   = 10, //!< No additional details
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
        SFC_FRAME_START_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief SFC_LOCK
    //! \details
    //! 
    //!
    struct SFC_LOCK_CMD
    {
        union
        {
            //!< DWORD 0
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
            //!< DWORD 1
            struct
            {
                uint32_t                 VeSfcPipeSelect                                  : __CODEGEN_BITFIELD( 0,  0)    ; //!< VE-SFC Pipe Select
                uint32_t                 PreScaledOutputSurfaceOutputEnable               : __CODEGEN_BITFIELD( 1,  1)    ; //!< Pre-Scaled Output Surface Output Enable
                uint32_t                 Reserved34                                       : __CODEGEN_BITFIELD( 2, 31)    ; //!< Reserved
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
            MEDIA_COMMAND_OPCODE_MEDIAMISC                                   = 10, //!< No additional details
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
        SFC_LOCK_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief SFC_STATE
    //! \details
    //!     This command is sent from VDBOX/VEBOX to SFC pipeline at the start of
    //!     each frame once the lock request is granted.
    //!
    struct SFC_STATE_CMD
    {
        union
        {
            //!< DWORD 0
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
            //!< DWORD 1
            struct
            {
                uint32_t                 SfcPipeMode                                      : __CODEGEN_BITFIELD( 0,  3)    ; //!< SFC_PIPE_MODE
                uint32_t                 SfcInputChromaSubSampling                        : __CODEGEN_BITFIELD( 4,  7)    ; //!< SFC_INPUT_CHROMA_SUB_SAMPLING
                uint32_t                 VdVeInputOrderingMode                            : __CODEGEN_BITFIELD( 8, 10)    ; //!< VDVE_INPUT_ORDERING_MODE
                uint32_t                 Reserved43                                       : __CODEGEN_BITFIELD(11, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 InputFrameResolutionWidth                        : __CODEGEN_BITFIELD( 0, 11)    ; //!< Input Frame Resolution Width
                uint32_t                 Reserved76                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 InputFrameResolutionHeight                       : __CODEGEN_BITFIELD(16, 27)    ; //!< Input Frame Resolution Height
                uint32_t                 Reserved92                                       : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 OutputSurfaceFormatType                          : __CODEGEN_BITFIELD( 0,  3)    ; //!< OUTPUT_SURFACE_FORMAT_TYPE
                uint32_t                 Reserved100                                      : __CODEGEN_BITFIELD( 4,  4)    ; //!< Reserved
                uint32_t                 RgbaChannelSwapEnable                            : __CODEGEN_BITFIELD( 5,  5)    ; //!< RGBA_CHANNEL_SWAP_ENABLE
                uint32_t                 Reserved102                                      : __CODEGEN_BITFIELD( 6,  7)    ; //!< Reserved
                uint32_t                 OutputChromaDownsamplingCoSitingPositionVerticalDirection : __CODEGEN_BITFIELD( 8, 11)    ; //!< OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION
                uint32_t                 OutputChromaDownsamplingCoSitingPositionHorizontalDirection : __CODEGEN_BITFIELD(12, 15)    ; //!< OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION
                uint32_t                 Reserved112                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 IefEnable                                        : __CODEGEN_BITFIELD( 0,  0)    ; //!< IEF_ENABLE
                uint32_t                 SkinToneTunedIefEnable                           : __CODEGEN_BITFIELD( 1,  1)    ; //!< Skin Tone Tuned IEF_Enable
                uint32_t                 Ief4SmoothEnable                                 : __CODEGEN_BITFIELD( 2,  2)    ; //!< IEF4SMOOTH_ENABLE_
                uint32_t                 Reserved131                                      : __CODEGEN_BITFIELD( 3,  3)    ; //!< Reserved
                uint32_t                 AvsFilterMode                                    : __CODEGEN_BITFIELD( 4,  5)    ; //!< AVS_FILTER_MODE
                uint32_t                 AdaptiveFilterForAllChannels                     : __CODEGEN_BITFIELD( 6,  6)    ; //!< ADAPTIVE_FILTER_FOR_ALL_CHANNELS
                uint32_t                 AvsScalingEnable                                 : __CODEGEN_BITFIELD( 7,  7)    ; //!< AVS_SCALING_ENABLE
                uint32_t                 BypassYAdaptiveFiltering                         : __CODEGEN_BITFIELD( 8,  8)    ; //!< BYPASS_Y_ADAPTIVE_FILTERING
                uint32_t                 BypassXAdaptiveFiltering                         : __CODEGEN_BITFIELD( 9,  9)    ; //!< BYPASS_X_ADAPTIVE_FILTERING
                uint32_t                 Reserved138                                      : __CODEGEN_BITFIELD(10, 11)    ; //!< Reserved
                uint32_t                 ChromaUpsamplingEnable                           : __CODEGEN_BITFIELD(12, 12)    ; //!< Chroma Upsampling Enable
                uint32_t                 Reserved141                                      : __CODEGEN_BITFIELD(13, 15)    ; //!< Reserved
                uint32_t                 RotationMode                                     : __CODEGEN_BITFIELD(16, 17)    ; //!< ROTATION_MODE
                uint32_t                 ColorFillEnable                                  : __CODEGEN_BITFIELD(18, 18)    ; //!< Color Fill Enable
                uint32_t                 CscEnable                                        : __CODEGEN_BITFIELD(19, 19)    ; //!< CSC Enable
                uint32_t                 Bitdepth                                         : __CODEGEN_BITFIELD(20, 21)    ; //!< BITDEPTH
                uint32_t                 Reserved150                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 SourceRegionWidth                                : __CODEGEN_BITFIELD( 0, 11)    ; //!< Source Region Width
                uint32_t                 Reserved172                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SourceRegionHeight                               : __CODEGEN_BITFIELD(16, 27)    ; //!< Source Region Height
                uint32_t                 Reserved188                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 SourceRegionHorizontalOffset                     : __CODEGEN_BITFIELD( 0, 11)    ; //!< Source Region Horizontal Offset
                uint32_t                 Reserved204                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SourceRegionVerticalOffset                       : __CODEGEN_BITFIELD(16, 27)    ; //!< Source Region Vertical Offset
                uint32_t                 Reserved220                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 OutputFrameWidth                                 : __CODEGEN_BITFIELD( 0, 11)    ; //!< Output Frame Width
                uint32_t                 Reserved236                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 OutputFrameHeight                                : __CODEGEN_BITFIELD(16, 27)    ; //!< Output Frame Height
                uint32_t                 Reserved252                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 ScaledRegionSizeWidth                            : __CODEGEN_BITFIELD( 0, 11)    ; //!< Scaled Region Size Width
                uint32_t                 Reserved268                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 ScaledRegionSizeHeight                           : __CODEGEN_BITFIELD(16, 27)    ; //!< Scaled Region Size Height
                uint32_t                 Reserved284                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 ScaledRegionHorizontalOffset                     : __CODEGEN_BITFIELD( 0, 12)    ; //!< Scaled Region Horizontal Offset
                uint32_t                 Reserved301                                      : __CODEGEN_BITFIELD(13, 15)    ; //!< Reserved
                uint32_t                 ScaledRegionVerticalOffset                       : __CODEGEN_BITFIELD(16, 28)    ; //!< Scaled Region Vertical Offset
                uint32_t                 Reserved317                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
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
            //!< DWORD 11
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
            //!< DWORD 12
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
            //!< DWORD 13
            struct
            {
                uint32_t                 AlphaDefaultValue                                : __CODEGEN_BITFIELD( 0,  9)    ; //!< Alpha Default Value
                uint32_t                 Reserved426                                      : __CODEGEN_BITFIELD(10, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 ScalingFactorHeight                              : __CODEGEN_BITFIELD( 0, 20)    ; //!< SCALING_FACTOR_HEIGHT
                uint32_t                 Reserved469                                      : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 ScalingFactorWidth                               : __CODEGEN_BITFIELD( 0, 20)    ; //!< SCALING_FACTOR_WIDTH
                uint32_t                 Reserved501                                      : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            //!< DWORD 16
            struct
            {
                uint32_t                 Reserved512                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            //!< DWORD 17
            struct
            {
                uint32_t                 Reserved544                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 OutputFrameSurfaceBaseAddress                    : __CODEGEN_BITFIELD(12, 31)    ; //!< Output Frame Surface Base Address
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            //!< DWORD 18
            struct
            {
                uint32_t                 OutputFrameSurfaceBaseAddressHigh                : __CODEGEN_BITFIELD( 0, 15)    ; //!< Output Frame Surface Base Address High
                uint32_t                 Reserved592                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            //!< DWORD 19
            struct
            {
                uint32_t                 Reserved608                                      : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 OutputFrameSurfaceBaseAddressIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< Output Frame Surface Base Address - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 OutputFrameSurfaceBaseAddressArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< Output Frame Surface Base Address - Arbitration Priority Control
                uint32_t                 OutputFrameSurfaceBaseAddressMemoryCompressionEnable : __CODEGEN_BITFIELD( 9,  9)    ; //!< Output Frame Surface Base Address - Memory Compression Enable
                uint32_t                 OutputFrameSurfaceBaseAddressMemoryCompressionMode : __CODEGEN_BITFIELD(10, 10)    ; //!< OUTPUT_FRAME_SURFACE_BASE_ADDRESS__MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved619                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 OutputFrameSurfaceBaseAddressRowStoreScratchBufferCacheSelect : __CODEGEN_BITFIELD(12, 12)    ; //!< OUTPUT_FRAME_SURFACE_BASE_ADDRESS__ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 OutputSurfaceTiledMode                           : __CODEGEN_BITFIELD(13, 14)    ; //!< OUTPUT_SURFACE_TILED_MODE
                uint32_t                 Reserved623                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            //!< DWORD 20
            struct
            {
                uint32_t                 Reserved640                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 AvsLineBufferSurfaceBaseAddress                  : __CODEGEN_BITFIELD(12, 31)    ; //!< AVS Line Buffer Surface Base Address
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            //!< DWORD 21
            struct
            {
                uint32_t                 AvsLineBufferSurfaceBaseAddressHigh              : __CODEGEN_BITFIELD( 0, 15)    ; //!< AVS Line Buffer Surface Base Address High
                uint32_t                 Reserved688                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW21;
        union
        {
            //!< DWORD 22
            struct
            {
                uint32_t                 Reserved704                                      : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 AvsLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< AVS Line Buffer Base Address - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 AvsLineBufferBaseAddressArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< AVS Line Buffer Base Address - Arbitration Priority Control
                uint32_t                 AvsLineBufferBaseAddressMemoryCompressionEnable  : __CODEGEN_BITFIELD( 9,  9)    ; //!< AVS_LINE_BUFFER_BASE_ADDRESS__MEMORY_COMPRESSION_ENABLE
                uint32_t                 AvsLineBufferBaseAddressMemoryCompressionMode    : __CODEGEN_BITFIELD(10, 10)    ; //!< AVS_LINE_BUFFER_BASE_ADDRESS__MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved715                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 AvsLineBufferBaseAddressRowStoreScratchBufferCacheSelect : __CODEGEN_BITFIELD(12, 12)    ; //!< AVS_LINE_BUFFER_BASE_ADDRESS__ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 AvsLineBufferTiledMode                           : __CODEGEN_BITFIELD(13, 14)    ; //!< AVS_LINE_BUFFER_TILED_MODE
                uint32_t                 Reserved719                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW22;
        union
        {
            //!< DWORD 23
            struct
            {
                uint32_t                 Reserved736                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 IefLineBufferSurfaceBaseAddress                  : __CODEGEN_BITFIELD(12, 31)    ; //!< IEF Line Buffer Surface Base Address
            };
            uint32_t                     Value;
        } DW23;
        union
        {
            //!< DWORD 24
            struct
            {
                uint32_t                 IefLineBufferSurfaceBaseAddressHigh              : __CODEGEN_BITFIELD( 0, 15)    ; //!< IEF Line Buffer Surface Base Address High
                uint32_t                 Reserved784                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW24;
        union
        {
            //!< DWORD 25
            struct
            {
                uint32_t                 Reserved800                                      : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 IefLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< IEF Line Buffer Base Address - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 IefLineBufferBaseAddressArbitrationPriorityControl : __CODEGEN_BITFIELD( 7,  8)    ; //!< IEF Line Buffer Base Address - Arbitration Priority Control
                uint32_t                 IefLineBufferBaseAddressMemoryCompressionEnable  : __CODEGEN_BITFIELD( 9,  9)    ; //!< IEF_LINE_BUFFER_BASE_ADDRESS__MEMORY_COMPRESSION_ENABLE
                uint32_t                 IefLineBufferBaseAddressMemoryCompressionMode    : __CODEGEN_BITFIELD(10, 10)    ; //!< IEF_LINE_BUFFER_BASE_ADDRESS__MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved811                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 IefLineBufferBaseAddressRowStoreScratchBufferCacheSelect : __CODEGEN_BITFIELD(12, 12)    ; //!< IEF_LINE_BUFFER_BASE_ADDRESS__ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 IefLineBufferTiledMode                           : __CODEGEN_BITFIELD(13, 14)    ; //!< IEF_LINE_BUFFER_TILED_MODE
                uint32_t                 Reserved815                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW25;
        union
        {
            //!< DWORD 26
            struct
            {
                uint32_t                 Reserved832                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW26;
        union
        {
            //!< DWORD 27
            struct
            {
                uint32_t                 Reserved864                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW27;
        union
        {
            //!< DWORD 28
            struct
            {
                uint32_t                 Reserved896                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW28;
        union
        {
            //!< DWORD 29
            struct
            {
                uint32_t                 OutputSurfaceTileWalk                            : __CODEGEN_BITFIELD( 0,  0)    ; //!< OUTPUT_SURFACE_TILE_WALK
                uint32_t                 OutputSurfaceTiled                               : __CODEGEN_BITFIELD( 1,  1)    ; //!< OUTPUT_SURFACE_TILED
                uint32_t                 OutputSurfaceHalfPitchForChroma                  : __CODEGEN_BITFIELD( 2,  2)    ; //!< Output Surface Half Pitch For Chroma
                uint32_t                 OutputSurfacePitch                               : __CODEGEN_BITFIELD( 3, 19)    ; //!< Output Surface Pitch
                uint32_t                 Reserved948                                      : __CODEGEN_BITFIELD(20, 26)    ; //!< Reserved
                uint32_t                 OutputSurfaceInterleaveChromaEnable              : __CODEGEN_BITFIELD(27, 27)    ; //!< Output Surface Interleave Chroma Enable
                uint32_t                 OutputSurfaceFormat                              : __CODEGEN_BITFIELD(28, 31)    ; //!< Output Surface Format
            };
            uint32_t                     Value;
        } DW29;
        union
        {
            //!< DWORD 30
            struct
            {
                uint32_t                 OutputSurfaceYOffsetForU                         : __CODEGEN_BITFIELD( 0, 13)    ; //!< Output Surface Y Offset For U
                uint32_t                 Reserved974                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 OutputSurfaceXOffsetForU                         : __CODEGEN_BITFIELD(16, 29)    ; //!< Output Surface X Offset For U
                uint32_t                 Reserved990                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW30;
        union
        {
            //!< DWORD 31
            struct
            {
                uint32_t                 OutputSurfaceYOffsetForV                         : __CODEGEN_BITFIELD( 0, 13)    ; //!< Output Surface Y Offset For V
                uint32_t                 Reserved1006                                     : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 OutputSurfaceXOffsetForV                         : __CODEGEN_BITFIELD(16, 29)    ; //!< Output Surface X Offset For V
                uint32_t                 Reserved1022                                     : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW31;
        union
        {
            //!< DWORD 32
            struct
            {
                uint32_t                 Reserved1024                                                                     ; //!< Reserved
            };
            uint32_t                     Value;
        } DW32;
        union
        {
            //!< DWORD 33
            struct
            {
                uint32_t                 Reserved1056                                                                     ; //!< Reserved
            };
            uint32_t                     Value;
        } DW33;

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
            MEDIA_COMMAND_OPCODE_MEDIAMISC                                   = 10, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MEDIA                                                   = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief SFC_PIPE_MODE
        //! \details
        //!     Note: for SFC Pipe mode set to VE-to-SFC AVS mode. 
        //!                         IECP pipeline mode MUST be enabled. 
        //!                         However, each sub-IECP feature can be turned on/off independently.
        enum SFC_PIPE_MODE
        {
            SFC_PIPE_MODE_UNNAMED0                                           = 0, //!< VD-to-SFC AVS
            SFC_PIPE_MODE_UNNAMED1                                           = 1, //!< VE-to-SFC AVS + IEF + Rotation
            SFC_PIPE_MODE_UNNAMED_4                                          = 4, //!< VE-to-SFC Integral Image
        };

        //! \brief SFC_INPUT_CHROMA_SUB_SAMPLING
        //! \details
        //!     This field shall be programmed according to video modes used in VDBOX.
        //!     NOTE: SFC supports progressive input and output only (Interlaced/MBAFF
        //!     is not supported).
        //!     <table border="1">
        //!         <tbody>
        //!             <tr>
        //!                 <td>Video Mode</td>
        //!                 <td>Surface Format</td>
        //!                 <td>SFC Input Chroma Sub-Sampling</td>
        //!                 <td>VD/VE Input Ordering Mode</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>VC1 w/o LF and w/o OS Note: VC1 LF applies for ILDB</td>
        //!                 <td>420 (NV12)</td>
        //!                 <td>1</td>
        //!                 <td>0</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>VC1 w/ LF or w/ OS or w/ both Note: VC1 LF applies for ILDB</td>
        //!                 <td></td>
        //!                 <td>INVALID with SFC</td>
        //!                 <td>INVALID with SFC</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>AVC w/o LF</td>
        //!                 <td>Monochrome</td>
        //!                 <td>0</td>
        //!                 <td>0</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>AVC w/o LF</td>
        //!                 <td>420 (NV12)</td>
        //!                 <td>1</td>
        //!                 <td>0</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>AVC with LF</td>
        //!                 <td>Monochrome</td>
        //!                 <td>0</td>
        //!                 <td>1</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>AVC/VP8 with LF</td>
        //!                 <td>420 (NV12)</td>
        //!                 <td>1</td>
        //!                 <td>1</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>VP8 w/o LF</td>
        //!                 <td>420 (NV12)</td>
        //!                 <td>1</td>
        //!                 <td>4</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>JPEG (YUV Interleaved)</td>
        //!                 <td>Monochrome</td>
        //!                 <td>0</td>
        //!                 <td>2</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>JPEG (YUV Interleaved)</td>
        //!                 <td>420</td>
        //!                 <td>1</td>
        //!                 <td>3</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>JPEG (YUV Interleaved)</td>
        //!                 <td>422H_2Y</td>
        //!                 <td>2</td>
        //!                 <td>2</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>JPEG (YUV Interleaved)</td>
        //!                 <td>422H_4Y</td>
        //!                 <td>2</td>
        //!                 <td>3</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>JPEG (YUV Interleaved)</td>
        //!                 <td>444</td>
        //!                 <td>4</td>
        //!                 <td>2</td>
        //!             </tr>
        //!         </tbody>
        //!     </table>
        //!     This field shall be programmed according to Image enhancement modes used
        //!     in VEBOX.
        //! 
        //!     <table border="1">
        //!         <tbody>
        //!             <tr>
        //!                 <td>VEBOX MODE</td>
        //!                 <td>Surface Format</td>
        //!                 <td>SFC Input Chroma Sub Sampling</td>
        //!                 <td>VD/VE Input Ordering Mode</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>Legacy DN/DI/IECP features</td>
        //!                 <td>Monochrome</td>
        //!                 <td>0</td>
        //!                 <td>0</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>Legacy DN/DI/IECP features</td>
        //!                 <td>420 (NV12)</td>
        //!                 <td>1</td>
        //!                 <td>0</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>Legacy DN/DI/IECP features</td>
        //!                 <td>422H</td>
        //!                 <td>2</td>
        //!                 <td>0</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>Legacy DN/DI/IECP features</td>
        //!                 <td>444</td>
        //!                 <td>4</td>
        //!                 <td>0</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>Capture/Camera pipe enabled(Demosaic)</td>
        //!                 <td>Monochrome</td>
        //!                 <td>0</td>
        //!                 <td>1</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>Capture/Camera pipe enabled(Demosaic)</td>
        //!                 <td>420 (NV12)</td>
        //!                 <td>1</td>
        //!                 <td>1</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>Capture/Camera pipe enabled(Demosaic)</td>
        //!                 <td>422H</td>
        //!                 <td>2</td>
        //!                 <td>1</td>
        //!             </tr>
        //!             <tr>
        //!                 <td>Capture/Camera pipe enabled(Demosaic)</td>
        //!                 <td>444</td>
        //!                 <td>4</td>
        //!                 <td>1</td>
        //!             </tr>
        //!         </tbody>
        //!     </table>
        enum SFC_INPUT_CHROMA_SUB_SAMPLING
        {
            SFC_INPUT_CHROMA_SUB_SAMPLING_400                                = 0, //!< SFC to insert UV channels
            SFC_INPUT_CHROMA_SUB_SAMPLING_420                                = 1, //!< No additional details
            SFC_INPUT_CHROMA_SUB_SAMPLING_422HORIZONATAL                     = 2, //!< VD: 2:1:1
            SFC_INPUT_CHROMA_SUB_SAMPLING_4_4_4PROGRESSIVEINTERLEAVED        = 4, //!< No additional details
        };

        //! \brief VDVE_INPUT_ORDERING_MODE
        //! \details
        //!     <ul>
        //!                             <li>VD mode: (SFC pipe mode set as "0")</li>
        //!                             <li> VE mode:  (pipe mode set as "1 and 4")</li>
        //!                         </ul>
        //!                         For values for each mode, please refer to the table below:
        enum VDVE_INPUT_ORDERING_MODE
        {
            VDVE_INPUT_ORDERING_MODE_UNNAMED0                                = 0, //!< 16x16 block z-scan order - no shift
                                                                                  //!< 8x4 block column order, 64 pixel column
            VDVE_INPUT_ORDERING_MODE_UNNAMED1                                = 1, //!< 16x16 block z-scan order - 4 pixels shift upward
                                                                                  //!< 4x4 block column order, 64 pixel column
            VDVE_INPUT_ORDERING_MODE_UNNAMED2                                = 2, //!< 8x8 block jpeg z-scan order
                                                                                  //!< 8x4 block column order, 128 pixel column
            VDVE_INPUT_ORDERING_MODE_UNNAMED3                                = 3, //!< 16x16 block jpeg z-scan order
                                                                                  //!< 4x4 block column order, 128 pixel column
            VDVE_INPUT_ORDERING_MODE_UNNAMED_4                               = 4, //!< 16x16 block VP8 row-scan order - no shift
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
        };

        //! \brief RGBA_CHANNEL_SWAP_ENABLE
        //! \details
        //!     This bit should only be used with RGB output formats and CSC conversion
        //!     is turned on. When this bit is set,
        //!                         the R and B channels are swapped into the output RGB channels as
        //!     shown in the following table:
        //!                         <table>
        //!                             <tr>
        //!                                 <th>Name</th>
        //!                                 <th>Bits</th>
        //!                                 <th>MSB Color Order</th>
        //!                                 <th>Swapped</th>
        //!                             </tr>
        //!                             <tr>
        //!                                 <td>RGBA8</td>
        //!                                 <td>8:8:8:8</td>
        //!                                 <td>A:B:G:R</td>
        //!                                 <td>A:R:G:B</td>
        //!                             </tr>
        //!                             <tr>
        //!                                 <td>RGBA10</td>
        //!                                 <td>2:10:10:10</td>
        //!                                 <td>A:R:G:B</td>
        //!                                 <td>A:B:G:R</td>
        //!                             </tr>
        //!                             <tr>
        //!                                 <td>RGB 5:6:5</td>
        //!                                 <td>5:6:5</td>
        //!                                 <td>R:G:B</td>
        //!                                 <td>B:G:R</td>
        //!                             </tr>
        //!                         </table>
        enum RGBA_CHANNEL_SWAP_ENABLE
        {
            RGBA_CHANNEL_SWAP_ENABLE_UNNAMED0                                = 0, //!< No additional details
        };

        //! \brief OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION
        //! \details
        //!     This field specifies the fractional position of the bilinear filter for
        //!     chroma downsampling. In the Y-axis.
        enum OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION
        {
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_08_LEFTFULLPIXEL = 0, //!< 0 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_18 = 1, //!< 1 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_1_4_28 = 2, //!< 2 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_38 = 3, //!< 3 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_12_48 = 4, //!< 4 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_58 = 5, //!< 5 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_VERTICAL_DIRECTION_3_4_68 = 6, //!< 6 (fraction_in_integer)
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
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_1_4_28 = 2, //!< 2 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_38 = 3, //!< 3 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_12_48 = 4, //!< 4 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_58 = 5, //!< 5 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_3_4_68 = 6, //!< 6 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_78 = 7, //!< 7 (fraction_in_integer)
            OUTPUT_CHROMA_DOWNSAMPLING_CO_SITING_POSITION_HORIZONTAL_DIRECTION_88 = 8, //!< No additional details
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

        //! \brief ROTATION_MODE
        //! \details
        //!     <p>SFC rotation (90, 180 and 270) should be set only on VEBox input mode
        //!     and SFC output set to TileY.</p>
        //!                     Restriction: 
        //!                         <ul>
        //!                             <li>For Integral Image Mode, this field is Reserved and MBZ.</li>
        //!                             <li>For VDBox Mode, this field is Reserved and MBZ.</li>
        //!                             <li>For linear or TileX SFC output, this field is Reserved and
        //!     MBZ.</li>
        //!                         </ul>
        enum ROTATION_MODE
        {
            ROTATION_MODE_0_DEGREES                                          = 0, //!< No additional details
            ROTATION_MODE_90CLOCKWISE                                        = 1, //!< No additional details
            ROTATION_MODE_180CLOCKWISE                                       = 2, //!< No additional details
            ROTATION_MODE_270CLOCKWISE                                       = 3, //!< No additional details
        };

        //! \brief BITDEPTH
        //! \details
        //!     This field is valid only for output formats P016. This field is used to
        //!     specify how many of the LSB bits have valid data.
        enum BITDEPTH
        {
            BITDEPTH_10BITFORMAT                                             = 0, //!< Higher 10 bits are valid and lower 6 bits are 0
        };

        //! \brief SCALING_FACTOR_HEIGHT
        //! \details
        //!     <p>This field specifies the scaling ratio of the vertical sizes between
        //!     the crop/source region and the scaled region.
        //!                         The destination pixel coordinate, y-axis, is multiplied with this
        //!     scaling factor to mapping back to the source input pixel coordinate.</p>
        //!                         <p>The field specifies the ratio of crop height resolution/ scaled
        //!     height resolution. This implies 1/<i>sf<sub>u</sub></i> in the
        //!     equation.</p>
        enum SCALING_FACTOR_HEIGHT
        {
            SCALING_FACTOR_HEIGHT_UNNAMED0                                   = 0, //!< Reserved
        };

        //! \brief SCALING_FACTOR_WIDTH
        //! \details
        //!     <p>This field specifies the scaling ratio of the horizontal sizes
        //!     between the crop/source region and the scaled region.
        //!                         The destination pixel coordinate, x-axis, is multiplied with this
        //!     scaling factor to mapping back to the source input pixel coordinate.
        //!     </p>
        //!                         <p>The field specifies the ratio of crop width resolution/ scaled
        //!     width resolution. This implies 1/<i>sf<sub>u</sub></i> in the equations
        //!     above.</p>
        enum SCALING_FACTOR_WIDTH
        {
            SCALING_FACTOR_WIDTH_UNNAMED0                                    = 0, //!< Reserved
        };

        //! \brief OUTPUT_FRAME_SURFACE_BASE_ADDRESS__MEMORY_COMPRESSION_MODE
        //! \details
        //!     <span style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Distinguishes
        //!     vertical from horizontal compression. Please refer to vol1a</span><b
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Memory Data Formats
        //!     chapter - section</b><span style="color: rgb(35, 35, 35); font-family:
        //!     Arial, sans-serif; font-size: 13.3333330154419px; line-height:
        //!     normal;">media Memory Compression for more details.</span>
        enum OUTPUT_FRAME_SURFACE_BASE_ADDRESS__MEMORY_COMPRESSION_MODE
        {
            OUTPUT_FRAME_SURFACE_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSION = 0, //!< No additional details
            OUTPUT_FRAME_SURFACE_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSION = 1, //!< No additional details
        };

        //! \brief OUTPUT_FRAME_SURFACE_BASE_ADDRESS__ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        //! \details
        //!     This must be set to 0
        enum OUTPUT_FRAME_SURFACE_BASE_ADDRESS__ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        {
            OUTPUT_FRAME_SURFACE_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_DISABLE = 0, //!< This field must be programmed to 0
        };

        //! \brief OUTPUT_SURFACE_TILED_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum OUTPUT_SURFACE_TILED_MODE
        {
            OUTPUT_SURFACE_TILED_MODE_TRMODENONE                             = 0, //!< No tiled resource
            OUTPUT_SURFACE_TILED_MODE_TRMODETILEYF                           = 1, //!< 4KB tiled resources
            OUTPUT_SURFACE_TILED_MODE_TRMODETILEYS                           = 2, //!< 64KB tiled resources
        };

        //! \brief AVS_LINE_BUFFER_BASE_ADDRESS__MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     This bit control memory compression for this surface
        enum AVS_LINE_BUFFER_BASE_ADDRESS__MEMORY_COMPRESSION_ENABLE
        {
            AVS_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE   = 0, //!< No additional details
        };

        //! \brief AVS_LINE_BUFFER_BASE_ADDRESS__MEMORY_COMPRESSION_MODE
        //! \details
        //!     <span style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Distinguishes
        //!     vertical from horizontal compression. Please refer to vol1a </span><b
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Memory Data Formats
        //!     chapter - section</b><span style="color: rgb(35, 35, 35); font-family:
        //!     Arial, sans-serif; font-size: 13.3333330154419px; line-height:
        //!     normal;"> media Memory Compression for more details.</span>
        enum AVS_LINE_BUFFER_BASE_ADDRESS__MEMORY_COMPRESSION_MODE
        {
            AVS_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE = 0, //!< No additional details
        };

        //! \brief AVS_LINE_BUFFER_BASE_ADDRESS__ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        //! \details
        //!     <span style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">This field controls
        //!     if the Row Store is going to store inside Media Cache (rowstore cache)
        //!     or to LLC.</span>
        enum AVS_LINE_BUFFER_BASE_ADDRESS__ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        {
            AVS_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC = 0, //!< Buffer going to LLC
        };

        //! \brief AVS_LINE_BUFFER_TILED_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum AVS_LINE_BUFFER_TILED_MODE
        {
            AVS_LINE_BUFFER_TILED_MODE_TRMODENONE                            = 0, //!< No tiled resource
            AVS_LINE_BUFFER_TILED_MODE_TRMODETILEYF                          = 1, //!< 4KB tiled resources
            AVS_LINE_BUFFER_TILED_MODE_TRMODETILEYS                          = 2, //!< 64KB tiled resources
        };

        //! \brief IEF_LINE_BUFFER_BASE_ADDRESS__MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     <p><span style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Memory compression
        //!     is not supported for this surface</span></p>
        //!     <p><span style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Must be
        //!     0.</span></p>
        //!     <p></p>
        enum IEF_LINE_BUFFER_BASE_ADDRESS__MEMORY_COMPRESSION_ENABLE
        {
            IEF_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_ENABLE_DISABLE   = 0, //!< No additional details
        };

        //! \brief IEF_LINE_BUFFER_BASE_ADDRESS__MEMORY_COMPRESSION_MODE
        //! \details
        //!     <span style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Distinguishes
        //!     vertical from horizontal compression. Please refer to vol1a </span><b
        //!     style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">Memory Data Formats
        //!     chapter - section</b><span style="color: rgb(35, 35, 35); font-family:
        //!     Arial, sans-serif; font-size: 13.3333330154419px; line-height:
        //!     normal;"> media Memory Compression for more details.</span>
        enum IEF_LINE_BUFFER_BASE_ADDRESS__MEMORY_COMPRESSION_MODE
        {
            IEF_LINE_BUFFER_BASE_ADDRESS_MEMORY_COMPRESSION_MODE_UNNAMED0    = 0, //!< No additional details
        };

        //! \brief IEF_LINE_BUFFER_BASE_ADDRESS__ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        //! \details
        //!     <span style="color: rgb(35, 35, 35); font-family: Arial, sans-serif;
        //!     font-size: 13.3333330154419px; line-height: normal;">This field controls
        //!     if the Row Store is going to store inside Media Cache (rowstore cache)
        //!     or to LLC.</span>
        enum IEF_LINE_BUFFER_BASE_ADDRESS__ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        {
            IEF_LINE_BUFFER_BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_LLC = 0, //!< Buffer going to LLC
        };

        //! \brief IEF_LINE_BUFFER_TILED_MODE
        //! \details
        //!     <b>For Media Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        enum IEF_LINE_BUFFER_TILED_MODE
        {
            IEF_LINE_BUFFER_TILED_MODE_TRMODENONE                            = 0, //!< No tiled resource
            IEF_LINE_BUFFER_TILED_MODE_TRMODETILEYF                          = 1, //!< 4KB tiled resources
            IEF_LINE_BUFFER_TILED_MODE_TRMODETILEYS                          = 2, //!< 64KB tiled resources
        };

        //! \brief OUTPUT_SURFACE_TILE_WALK
        //! \details
        //!     This field specifies the type of memory tiling (XMajor or YMajor)
        //!     employed to tile this surface. See <i>Memory Interface Functions</i> for
        //!     details on memory tiling and restrictions.
        enum OUTPUT_SURFACE_TILE_WALK
        {
            OUTPUT_SURFACE_TILE_WALK_TILEWALKXMAJOR                          = 0, //!< No additional details
            OUTPUT_SURFACE_TILE_WALK_TILEWALKYMAJOR                          = 1, //!< No additional details
        };

        //! \brief OUTPUT_SURFACE_TILED
        //! \details
        //!     This field specifies whether the surface is tiled.
        enum OUTPUT_SURFACE_TILED
        {
            OUTPUT_SURFACE_TILED_FALSE                                       = 0, //!< Linear
            OUTPUT_SURFACE_TILED_TRUE                                        = 1, //!< Tiled
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        SFC_STATE_CMD();

        static const size_t dwSize = 34;
        static const size_t byteSize = 136;
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
            //!< DWORD 0
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
            //!< DWORD 1
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
            //!< DWORD 2
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
            //!< DWORD 3
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
            //!< DWORD 4
            struct
            {
                uint32_t                 Table0XFilterCoefficientN6                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Table 0X Filter Coefficient[[n],6]
                uint32_t                 Table0YFilterCoefficientN6                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Table 0Y Filter Coefficient[[n],6]
                uint32_t                 Table0XFilterCoefficientN7                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Table 0X Filter Coefficient[[n],7]
                uint32_t                 Table0YFilterCoefficientN7                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Table 0Y Filter Coefficient[[n],7]
            };
            uint32_t                     Value;
        } DW4;

        uint32_t                         FilterCoefficients[124];                                                         //!< Filter Coefficients

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
            MEDIA_COMMAND_OPCODE_MEDIAMISC                                   = 10, //!< No additional details
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
        SFC_AVS_LUMA_Coeff_Table_CMD();

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
            //!< DWORD 0
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
            //!< DWORD 1
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
            //!< DWORD 2
            struct
            {
                uint32_t                 Table1XFilterCoefficientN4                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Table 1X Filter Coefficient[[n],4]
                uint32_t                 Table1YFilterCoefficientN4                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Table 1Y Filter Coefficient[[n],4]
                uint32_t                 Table1XFilterCoefficientN5                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Table 1X Filter Coefficient[[n],5]
                uint32_t                 Table1YFilterCoefficientN5                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Table 1Y Filter Coefficient[[n],5]
            };
            uint32_t                     Value;
        } DW2;

        uint32_t                         FilterCoefficients[62];                                                          //!< Filter Coefficients

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
            MEDIA_COMMAND_OPCODE_MEDIAMISC                                   = 10, //!< No additional details
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
        SFC_AVS_CHROMA_Coeff_Table_CMD();

        static const size_t dwSize = 65;
        static const size_t byteSize = 260;
    };

};

#pragma pack()

#endif  // __MHW_SFC_HWCMD_G11_X_H__