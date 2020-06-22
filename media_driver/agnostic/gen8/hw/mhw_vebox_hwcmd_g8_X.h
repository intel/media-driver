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
//! \file     mhw_vebox_hwcmd_g8_X.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of g8_X as other components
//!           should use MHW interface to interact with MHW commands and states.
//!
#ifndef __MHW_VEBOX_HWCMD_G8_X_H__
#define __MHW_VEBOX_HWCMD_G8_X_H__

#pragma once
#pragma pack(1)

#include <cstdint>
#include <cstddef>

class mhw_vebox_g8_X
{
public:
    // Internal Macros
    #define __CODEGEN_MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
    #define __CODEGEN_BITFIELD(l, h) (h) - (l) + 1
    #define __CODEGEN_OP_LENGTH_BIAS 2
    #define __CODEGEN_OP_LENGTH(x) (uint32_t)((__CODEGEN_MAX(x, __CODEGEN_OP_LENGTH_BIAS)) - __CODEGEN_OP_LENGTH_BIAS)

    static uint32_t GetOpLength(uint32_t uiLength) { return __CODEGEN_OP_LENGTH(uiLength); }

    //!
    //! \brief VEBOX_ACE_LACE_STATE
    //! \details
    //!     This state structure contains the IECP State Table Contents for ACE
    //!     state.
    //!
    struct VEBOX_ACE_LACE_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 AceEnable                                        : __CODEGEN_BITFIELD( 0,  0)    ; //!< ACE Enable
                uint32_t                 FullImageHistogram                               : __CODEGEN_BITFIELD( 1,  1)    ; //!< FULL_IMAGE_HISTOGRAM
                uint32_t                 SkinThreshold                                    : __CODEGEN_BITFIELD( 2,  6)    ; //!< SKIN_THRESHOLD
                uint32_t                 Reserved7                                        : __CODEGEN_BITFIELD( 7, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Ymin                                             : __CODEGEN_BITFIELD( 0,  7)    ; //!< YMIN
                uint32_t                 Y1                                               : __CODEGEN_BITFIELD( 8, 15)    ; //!< Y1
                uint32_t                 Y2                                               : __CODEGEN_BITFIELD(16, 23)    ; //!< Y2
                uint32_t                 Y3                                               : __CODEGEN_BITFIELD(24, 31)    ; //!< Y3
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Y4                                               : __CODEGEN_BITFIELD( 0,  7)    ; //!< Y4
                uint32_t                 Y5                                               : __CODEGEN_BITFIELD( 8, 15)    ; //!< Y5
                uint32_t                 Y6                                               : __CODEGEN_BITFIELD(16, 23)    ; //!< Y6
                uint32_t                 Y7                                               : __CODEGEN_BITFIELD(24, 31)    ; //!< Y7
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 Y8                                               : __CODEGEN_BITFIELD( 0,  7)    ; //!< Y8
                uint32_t                 Y9                                               : __CODEGEN_BITFIELD( 8, 15)    ; //!< Y9
                uint32_t                 Y10                                              : __CODEGEN_BITFIELD(16, 23)    ; //!< Y10
                uint32_t                 Ymax                                             : __CODEGEN_BITFIELD(24, 31)    ; //!< YMAX
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 B1                                               : __CODEGEN_BITFIELD( 0,  7)    ; //!< B1
                uint32_t                 B2                                               : __CODEGEN_BITFIELD( 8, 15)    ; //!< B2
                uint32_t                 B3                                               : __CODEGEN_BITFIELD(16, 23)    ; //!< B3
                uint32_t                 B4                                               : __CODEGEN_BITFIELD(24, 31)    ; //!< B4
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 B5                                               : __CODEGEN_BITFIELD( 0,  7)    ; //!< B5
                uint32_t                 B6                                               : __CODEGEN_BITFIELD( 8, 15)    ; //!< B6
                uint32_t                 B7                                               : __CODEGEN_BITFIELD(16, 23)    ; //!< B7
                uint32_t                 B8                                               : __CODEGEN_BITFIELD(24, 31)    ; //!< B8
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 B9                                               : __CODEGEN_BITFIELD( 0,  7)    ; //!< B9
                uint32_t                 B10                                              : __CODEGEN_BITFIELD( 8, 15)    ; //!< B10
                uint32_t                 Reserved208                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 S0                                               : __CODEGEN_BITFIELD( 0, 10)    ; //!< S0
                uint32_t                 Reserved235                                      : __CODEGEN_BITFIELD(11, 15)    ; //!< Reserved
                uint32_t                 S1                                               : __CODEGEN_BITFIELD(16, 26)    ; //!< S1
                uint32_t                 Reserved251                                      : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 S2                                               : __CODEGEN_BITFIELD( 0, 10)    ; //!< S2
                uint32_t                 Reserved267                                      : __CODEGEN_BITFIELD(11, 15)    ; //!< Reserved
                uint32_t                 S3                                               : __CODEGEN_BITFIELD(16, 26)    ; //!< S3
                uint32_t                 Reserved283                                      : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 S4                                               : __CODEGEN_BITFIELD( 0, 10)    ; //!< S4
                uint32_t                 Reserved299                                      : __CODEGEN_BITFIELD(11, 15)    ; //!< Reserved
                uint32_t                 S5                                               : __CODEGEN_BITFIELD(16, 26)    ; //!< S5
                uint32_t                 Reserved315                                      : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 S6                                               : __CODEGEN_BITFIELD( 0, 10)    ; //!< S6
                uint32_t                 Reserved331                                      : __CODEGEN_BITFIELD(11, 15)    ; //!< Reserved
                uint32_t                 S7                                               : __CODEGEN_BITFIELD(16, 26)    ; //!< S7
                uint32_t                 Reserved347                                      : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 S8                                               : __CODEGEN_BITFIELD( 0, 10)    ; //!< S8
                uint32_t                 Reserved363                                      : __CODEGEN_BITFIELD(11, 15)    ; //!< Reserved
                uint32_t                 S9                                               : __CODEGEN_BITFIELD(16, 26)    ; //!< S9
                uint32_t                 Reserved379                                      : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 S10                                              : __CODEGEN_BITFIELD( 0, 10)    ; //!< S10
                uint32_t                 Reserved395                                      : __CODEGEN_BITFIELD(11, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW12;

        //! \name Local enumerations

        //! \brief FULL_IMAGE_HISTOGRAM
        //! \details
        //!     Used to ignore the area of interest for full image histogram. This
        //!     applies to all statistics that are affected by AOI (Area of Interest).
        enum FULL_IMAGE_HISTOGRAM
        {
            FULL_IMAGE_HISTOGRAM_UNNAMED0                                    = 0, //!< No additional details
        };

        //! \brief SKIN_THRESHOLD
        //! \details
        //!     Used for Y analysis (min/max) for pixels which are higher than skin
        //!     threshold.
        enum SKIN_THRESHOLD
        {
            SKIN_THRESHOLD_UNNAMED26                                         = 26, //!< No additional details
        };

        //! \brief YMIN
        //! \details
        //!     The value of the y_pixel for point 0 in PWL.
        enum YMIN
        {
            YMIN_UNNAMED16                                                   = 16, //!< No additional details
        };

        //! \brief Y1
        //! \details
        //!     The value of the y_pixel for point 1 in PWL.
        enum Y1
        {
            Y1_UNNAMED36                                                     = 36, //!< No additional details
        };

        //! \brief Y2
        //! \details
        //!     The value of the y_pixel for point 2 in PWL.
        enum Y2
        {
            Y2_UNNAMED56                                                     = 56, //!< No additional details
        };

        //! \brief Y3
        //! \details
        //!     The value of the y_pixel for point 3 in PWL.
        enum Y3
        {
            Y3_UNNAMED76                                                     = 76, //!< No additional details
        };

        //! \brief Y4
        //! \details
        //!     The value of the y_pixel for point 4 in PWL.
        enum Y4
        {
            Y4_UNNAMED96                                                     = 96, //!< No additional details
        };

        //! \brief Y5
        //! \details
        //!     The value of the y_pixel for point 5 in PWL.
        enum Y5
        {
            Y5_UNNAMED116                                                    = 116, //!< No additional details
        };

        //! \brief Y6
        //! \details
        //!     The value of the y_pixel for point 6 in PWL.
        enum Y6
        {
            Y6_UNNAMED136                                                    = 136, //!< No additional details
        };

        //! \brief Y7
        //! \details
        //!     The value of the y_pixel for point 7 in PWL.
        enum Y7
        {
            Y7_UNNAMED156                                                    = 156, //!< No additional details
        };

        //! \brief Y8
        //! \details
        //!     The value of the y_pixel for point 8 in PWL.
        enum Y8
        {
            Y8_UNNAMED176                                                    = 176, //!< No additional details
        };

        //! \brief Y9
        //! \details
        //!     The value of the y_pixel for point 9 in PWL.
        enum Y9
        {
            Y9_UNNAMED196                                                    = 196, //!< No additional details
        };

        //! \brief Y10
        //! \details
        //!     The value of the y_pixel for point 10 in PWL.
        enum Y10
        {
            Y10_UNNAMED216                                                   = 216, //!< No additional details
        };

        //! \brief YMAX
        //! \details
        //!     The value of the y_pixel for point 11 in PWL.
        enum YMAX
        {
            YMAX_UNNAMED235                                                  = 235, //!< No additional details
        };

        //! \brief B1
        //! \details
        //!     The value of the bias for point 1 in PWL.
        enum B1
        {
            B1_UNNAMED36                                                     = 36, //!< No additional details
        };

        //! \brief B2
        //! \details
        //!     The value of the bias for point 2 in PWL.
        enum B2
        {
            B2_UNNAMED56                                                     = 56, //!< No additional details
        };

        //! \brief B3
        //! \details
        //!     The value of the bias for point 3 in PWL.
        enum B3
        {
            B3_UNNAMED76                                                     = 76, //!< No additional details
        };

        //! \brief B4
        //! \details
        //!     The value of the bias for point 4 in PWL.
        enum B4
        {
            B4_UNNAMED96                                                     = 96, //!< No additional details
        };

        //! \brief B5
        //! \details
        //!     The value of the bias for point 5 in PWL.
        enum B5
        {
            B5_UNNAMED116                                                    = 116, //!< No additional details
        };

        //! \brief B6
        //! \details
        //!     The value of the bias for point 6 in PWL.
        enum B6
        {
            B6_UNNAMED136                                                    = 136, //!< No additional details
        };

        //! \brief B7
        //! \details
        //!     The value of the bias for point 7 in PWL.
        enum B7
        {
            B7_UNNAMED156                                                    = 156, //!< No additional details
        };

        //! \brief B8
        //! \details
        //!     The value of the bias for point 8 in PWL.
        enum B8
        {
            B8_UNNAMED176                                                    = 176, //!< No additional details
        };

        //! \brief B9
        //! \details
        //!     The value of the bias for point 9 in PWL.
        enum B9
        {
            B9_UNNAMED196                                                    = 196, //!< No additional details
        };

        //! \brief B10
        //! \details
        //!     The value of the bias for point 10 in PWL.
        enum B10
        {
            B10_UNNAMED216                                                   = 216, //!< No additional details
        };

        //! \brief S0
        //! \details
        //!     The value of the slope for point 0 in PWL
        enum S0
        {
            S0_UNNAMED1024                                                   = 1024, //!< No additional details
        };

        //! \brief S1
        //! \details
        //!     The value of the slope for point 1 in PWL
        enum S1
        {
            S1_UNNAMED1024                                                   = 1024, //!< No additional details
        };

        //! \brief S2
        //! \details
        //!     The value of the slope for point 2 in PWL
        enum S2
        {
            S2_UNNAMED1024                                                   = 1024, //!< No additional details
        };

        //! \brief S3
        //! \details
        //!     The value of the slope for point 3 in PWL
        enum S3
        {
            S3_UNNAMED1024                                                   = 1024, //!< No additional details
        };

        //! \brief S4
        //! \details
        //!     The value of the slope for point 4 in PWL
        enum S4
        {
            S4_UNNAMED1024                                                   = 1024, //!< No additional details
        };

        //! \brief S5
        //! \details
        //!     The value of the slope for point 5 in PWL
        enum S5
        {
            S5_UNNAMED1024                                                   = 1024, //!< No additional details
        };

        //! \brief S6
        //! \details
        //!     The default is 1024/1024
        enum S6
        {
            S6_UNNAMED1024                                                   = 1024, //!< No additional details
        };

        //! \brief S7
        //! \details
        //!     The value of the slope for point 7 in PWL
        enum S7
        {
            S7_UNNAMED1024                                                   = 1024, //!< No additional details
        };

        //! \brief S8
        //! \details
        //!     The value of the slope for point 8 in PWL
        enum S8
        {
            S8_UNNAMED1024                                                   = 1024, //!< No additional details
        };

        //! \brief S9
        //! \details
        //!     The value of the slope for point 9 in PWL
        enum S9
        {
            S9_UNNAMED1024                                                   = 1024, //!< No additional details
        };

        //! \brief S10
        //! \details
        //!     The value of the slope for point 10 in PWL.
        enum S10
        {
            S10_UNNAMED1024                                                  = 1024, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VEBOX_ACE_LACE_STATE_CMD();

        static const size_t dwSize = 13;
        static const size_t byteSize = 52;
    };

    //!
    //! \brief VEBOX_ALPHA_AOI_STATE
    //! \details
    //!     This state structure contains the IECP State Table Contents for Fixed
    //!     Alpha and Area of Interest state.
    //!
    struct VEBOX_ALPHA_AOI_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 ColorPipeAlpha                                   : __CODEGEN_BITFIELD( 0, 11)    ; //!< Color Pipe Alpha
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 AlphaFromStateSelect                             : __CODEGEN_BITFIELD(16, 16)    ; //!< ALPHA_FROM_STATE_SELECT
                uint32_t                 Reserved17                                       : __CODEGEN_BITFIELD(17, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 AoiMinX                                          : __CODEGEN_BITFIELD( 0, 13)    ; //!< AOI_MIN_X
                uint32_t                 Reserved46                                       : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 AoiMaxX                                          : __CODEGEN_BITFIELD(16, 29)    ; //!< AOI_MAX_X
                uint32_t                 Reserved62                                       : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 AoiMinY                                          : __CODEGEN_BITFIELD( 0, 13)    ; //!< AOI_MIN_Y
                uint32_t                 Reserved78                                       : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 AoiMaxY                                          : __CODEGEN_BITFIELD(16, 29)    ; //!< AOI_MAX_Y
                uint32_t                 Reserved94                                       : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;

        //! \name Local enumerations

        //! \brief ALPHA_FROM_STATE_SELECT
        //! \details
        //!     If the input format does not have alpha available and the output format
        //!     provides alpha, this bit should be set to 1. This should be 0 when Alpha
        //!     Plane Enable is 1.
        enum ALPHA_FROM_STATE_SELECT
        {
            ALPHA_FROM_STATE_SELECT_ALPHAISTAKENFROMMESSAGE                  = 0, //!< No additional details
            ALPHA_FROM_STATE_SELECT_ALPHAISTAKENFROMSTATE                    = 1, //!< No additional details
        };

        //! \brief AOI_MIN_X
        //! \details
        //!     This value must be a multiple of 4.
        enum AOI_MIN_X
        {
            AOI_MIN_X_UNNAMED0                                               = 0, //!< No additional details
        };

        //! \brief AOI_MAX_X
        //! \details
        //!     Area of Interest Minimum X - The ACE histogram and Skin Tone Detection
        //!     statistic gathering will occur within the MinX/MinY to MaxX/MaxY area
        //!     (inclusive).
        //!                         This value must be a multiple of 4 minus 1.
        enum AOI_MAX_X
        {
            AOI_MAX_X_UNNAMED3                                               = 3, //!< No additional details
        };

        //! \brief AOI_MIN_Y
        //! \details
        //!     This value must be a multiple of 4.
        enum AOI_MIN_Y
        {
            AOI_MIN_Y_UNNAMED0                                               = 0, //!< No additional details
        };

        //! \brief AOI_MAX_Y
        //! \details
        //!     This value must be a multiple of 4 minus 1.
        enum AOI_MAX_Y
        {
            AOI_MAX_Y_UNNAMED3                                               = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VEBOX_ALPHA_AOI_STATE_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief VEBOX_CAPTURE_PIPE_STATE
    //! \details
    //!     This command  contains variables for controlling Demosaic and the White
    //!     Balance Statistics.
    //!
    struct VEBOX_CAPTURE_PIPE_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 AverageMinCostThreshold                          : __CODEGEN_BITFIELD( 0,  7)    ; //!< AVERAGE_MIN_COST_THRESHOLD
                uint32_t                 AverageColorThreshold                            : __CODEGEN_BITFIELD( 8, 15)    ; //!< AVERAGE_COLOR_THRESHOLD
                uint32_t                 ScaleForAverageMinCost                           : __CODEGEN_BITFIELD(16, 19)    ; //!< SCALE_FOR_AVERAGE_MIN_COST
                uint32_t                 ShiftMinCost                                     : __CODEGEN_BITFIELD(20, 22)    ; //!< SHIFT_MIN_COST
                uint32_t                 Reserved23                                       : __CODEGEN_BITFIELD(23, 23)    ; //!< Reserved
                uint32_t                 GoodPixelThreshold                               : __CODEGEN_BITFIELD(24, 29)    ; //!< GOOD_PIXEL_THRESHOLD
                uint32_t                 Reserved30                                       : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 BadColorThreshold3                               : __CODEGEN_BITFIELD( 0,  3)    ; //!< BAD_COLOR_THRESHOLD_3
                uint32_t                 Reserved36                                       : __CODEGEN_BITFIELD( 4,  7)    ; //!< Reserved
                uint32_t                 BadColorThreshold2                               : __CODEGEN_BITFIELD( 8, 15)    ; //!< BAD_COLOR_THRESHOLD_2
                uint32_t                 BadColorThreshold1                               : __CODEGEN_BITFIELD(16, 23)    ; //!< BAD_COLOR_THRESHOLD_1
                uint32_t                 Reserved56                                       : __CODEGEN_BITFIELD(24, 27)    ; //!< Reserved
                uint32_t                 ScaleForMinCost                                  : __CODEGEN_BITFIELD(28, 31)    ; //!< SCALE_FOR_MIN_COST
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Reserved64                                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Reserved
                uint32_t                 UvThresholdValue                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< UV_THRESHOLD_VALUE
                uint32_t                 YOutlierValue                                    : __CODEGEN_BITFIELD(16, 23)    ; //!< Y_OUTLIER_VALUE
                uint32_t                 YBrightValue                                     : __CODEGEN_BITFIELD(24, 31)    ; //!< Y_BRIGHT_VALUE
            };
            uint32_t                     Value;
        } DW2;

        //! \name Local enumerations

        //! \brief AVERAGE_MIN_COST_THRESHOLD
        //! \details
        //!     The threshold for the H and V Min_cost beyond which the Avg
        //!     interpolation will be used.
        enum AVERAGE_MIN_COST_THRESHOLD
        {
            AVERAGE_MIN_COST_THRESHOLD_UNNAMED75                             = 75, //!< No additional details
        };

        //! \brief AVERAGE_COLOR_THRESHOLD
        //! \details
        //!     The threshold between two colors in a pixel for the Avg interpolation to
        //!     be considered.
        enum AVERAGE_COLOR_THRESHOLD
        {
            AVERAGE_COLOR_THRESHOLD_UNNAMED100                               = 100, //!< No additional details
        };

        //! \brief SCALE_FOR_AVERAGE_MIN_COST
        //! \details
        //!     The amount to scale the min_cost difference during the Avg interpolation
        //!     decision
        enum SCALE_FOR_AVERAGE_MIN_COST
        {
            SCALE_FOR_AVERAGE_MIN_COST_UNNAMED2                              = 2, //!< No additional details
        };

        //! \brief SHIFT_MIN_COST
        //! \details
        //!     The amount to shift the H2/V2 versions of min_cost.
        enum SHIFT_MIN_COST
        {
            SHIFT_MIN_COST_UNNAMED1                                          = 1, //!< No additional details
        };

        //! \brief GOOD_PIXEL_THRESHOLD
        //! \details
        //!     The difference threshold between adjacent pixels for a pixel to be
        //!     considered "good".
        enum GOOD_PIXEL_THRESHOLD
        {
            GOOD_PIXEL_THRESHOLD_UNNAMED15                                   = 15, //!< No additional details
        };

        //! \brief BAD_COLOR_THRESHOLD_3
        //! \details
        //!     Color value threshold used during the bad pixel check.
        enum BAD_COLOR_THRESHOLD_3
        {
            BAD_COLOR_THRESHOLD_3_UNNAMED10                                  = 10, //!< No additional details
        };

        //! \brief BAD_COLOR_THRESHOLD_2
        //! \details
        //!     Color value threshold used during the bad pixel check.
        enum BAD_COLOR_THRESHOLD_2
        {
            BAD_COLOR_THRESHOLD_2_UNNAMED175                                 = 175, //!< No additional details
        };

        //! \brief BAD_COLOR_THRESHOLD_1
        //! \details
        //!     Color value threshold used during the bad pixel check.
        enum BAD_COLOR_THRESHOLD_1
        {
            BAD_COLOR_THRESHOLD_1_UNNAMED100                                 = 100, //!< No additional details
        };

        //! \brief SCALE_FOR_MIN_COST
        //! \details
        //!     The amount to scale the min_cost difference during the confidence check.
        enum SCALE_FOR_MIN_COST
        {
            SCALE_FOR_MIN_COST_UNNAMED10                                     = 10, //!< No additional details
        };

        //! \brief UV_THRESHOLD_VALUE
        //! \details
        //!     The value denotes the maximum threshold of the ratio between U+V to Y
        //!     can have to be considered a gray point.
        enum UV_THRESHOLD_VALUE
        {
            UV_THRESHOLD_VALUE_UNNAMED64                                     = 64, //!< 0.25 * 255 = 64
        };

        //! \brief Y_OUTLIER_VALUE
        //! \details
        //!     The outlier threshold percentile in the Y histogram. Any pixel with Y
        //!     value above this either clipped or an outlier in the image. These points
        //!     will not be included in the white patch calculation.
        enum Y_OUTLIER_VALUE
        {
            Y_OUTLIER_VALUE_UNNAMED253                                       = 253, //!< No additional details
        };

        //! \brief Y_BRIGHT_VALUE
        //! \details
        //!     The whitepoint threshold percentile in the Y histogram. Any pixel with Y
        //!     value above this could be a whitepoint. 
        //!                         This is the larger of the calculated Ybright value
        //!     and the Ythreshold value, which is the minimum Y required to be
        //!     considered a white point.
        enum Y_BRIGHT_VALUE
        {
            Y_BRIGHT_VALUE_UNNAMED230                                        = 230, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VEBOX_CAPTURE_PIPE_STATE_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief VEBOX_CCM_STATE
    //! \details
    //!     This state structure contains the IECP State Table Contents for Color
    //!     Correction Matrix State.
    //!
    struct VEBOX_CCM_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 C1CoefficientOf3X3TransformMatrix                : __CODEGEN_BITFIELD( 0, 20)    ; //!< C1_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
                uint32_t                 Reserved21                                       : __CODEGEN_BITFIELD(21, 29)    ; //!< Reserved
                uint32_t                 VignetteCorrectionFormat                         : __CODEGEN_BITFIELD(30, 30)    ; //!< VIGNETTE_CORRECTION_FORMAT
                uint32_t                 ColorCorrectionMatrixEnable                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Color Correction Matrix Enable
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 C0CoefficientOf3X3TransformMatrix                : __CODEGEN_BITFIELD( 0, 20)    ; //!< C0_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
                uint32_t                 Reserved53                                       : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 C3CoefficientOf3X3TransformMatrix                : __CODEGEN_BITFIELD( 0, 20)    ; //!< C3_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
                uint32_t                 Reserved85                                       : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 C2CoefficientOf3X3TransformMatrix                : __CODEGEN_BITFIELD( 0, 20)    ; //!< C2_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
                uint32_t                 Reserved117                                      : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 C5CoefficientOf3X3TransformMatrix                : __CODEGEN_BITFIELD( 0, 20)    ; //!< C5_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
                uint32_t                 Reserved149                                      : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 C4CoefficientOf3X3TransformMatrix                : __CODEGEN_BITFIELD( 0, 20)    ; //!< C4_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
                uint32_t                 Reserved181                                      : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 C7CoefficientOf3X3TransformMatrix                : __CODEGEN_BITFIELD( 0, 20)    ; //!< C7_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
                uint32_t                 Reserved213                                      : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 C6CoefficientOf3X3TransformMatrix                : __CODEGEN_BITFIELD( 0, 20)    ; //!< C6_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
                uint32_t                 Reserved245                                      : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 C8CoefficientOf3X3TransformMatrix                : __CODEGEN_BITFIELD( 0, 20)    ; //!< C8_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
                uint32_t                 Reserved277                                      : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;

        //! \name Local enumerations

        enum C1_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
        {
            C1_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX_11414096                  = 1141, //!< No additional details
        };

        //! \brief VIGNETTE_CORRECTION_FORMAT
        //! \details
        //!     Defines what shift should be assumed for the Vignette.
        enum VIGNETTE_CORRECTION_FORMAT
        {
            VIGNETTE_CORRECTION_FORMAT_U88                                   = 0, //!< No additional details
            VIGNETTE_CORRECTION_FORMAT_U412                                  = 1, //!< No additional details
        };

        enum C0_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
        {
            C0_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX_27924096                  = 2792, //!< No additional details
        };

        enum C3_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
        {
            C3_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX_714096                    = 71, //!< No additional details
        };

        enum C2_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
        {
            C2_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX_344096                    = 34, //!< No additional details
        };

        enum C5_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
        {
            C5_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX_524096                    = 2097100, //!< No additional details
        };

        enum C4_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
        {
            C4_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX_33634096                  = 3363, //!< No additional details
        };

        enum C7_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
        {
            C7_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX_1684096                   = 168, //!< No additional details
        };

        enum C6_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
        {
            C6_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX_124096                    = 2097140, //!< No additional details
        };

        enum C8_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX
        {
            C8_COEFFICIENT_OF_3X3_TRANSFORM_MATRIX_34344096                  = 3434, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VEBOX_CCM_STATE_CMD();

        static const size_t dwSize = 9;
        static const size_t byteSize = 36;
    };

    //!
    //! \brief VEBOX_CSC_STATE
    //! \details
    //!     This state structure contains the IECP State Table Contents for CSC
    //!     state.
    //!
    struct VEBOX_CSC_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 TransformEnable                                  : __CODEGEN_BITFIELD( 0,  0)    ; //!< Transform Enable
                uint32_t                 YuvChannelSwap                                   : __CODEGEN_BITFIELD( 1,  1)    ; //!< YUV_CHANNEL_SWAP
                uint32_t                 Reserved2                                        : __CODEGEN_BITFIELD( 2,  2)    ; //!< Reserved
                uint32_t                 C0                                               : __CODEGEN_BITFIELD( 3, 15)    ; //!< C0
                uint32_t                 C1                                               : __CODEGEN_BITFIELD(16, 28)    ; //!< C1
                uint32_t                 Reserved29                                       : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 C2                                               : __CODEGEN_BITFIELD( 0, 12)    ; //!< C2
                uint32_t                 C3                                               : __CODEGEN_BITFIELD(13, 25)    ; //!< C3
                uint32_t                 Reserved58                                       : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 C4                                               : __CODEGEN_BITFIELD( 0, 12)    ; //!< C4
                uint32_t                 C5                                               : __CODEGEN_BITFIELD(13, 25)    ; //!< C5
                uint32_t                 Reserved90                                       : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 C6                                               : __CODEGEN_BITFIELD( 0, 12)    ; //!< C6
                uint32_t                 C7                                               : __CODEGEN_BITFIELD(13, 25)    ; //!< C7
                uint32_t                 Reserved122                                      : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 C8                                               : __CODEGEN_BITFIELD( 0, 12)    ; //!< C8
                uint32_t                 Reserved141                                      : __CODEGEN_BITFIELD(13, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 OffsetIn1                                        : __CODEGEN_BITFIELD( 0, 10)    ; //!< OFFSET_IN_1
                uint32_t                 OffsetOut1                                       : __CODEGEN_BITFIELD(11, 21)    ; //!< OFFSET_OUT_1
                uint32_t                 Reserved182                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 OffsetIn2                                        : __CODEGEN_BITFIELD( 0, 10)    ; //!< OFFSET_IN_2
                uint32_t                 OffsetOut2                                       : __CODEGEN_BITFIELD(11, 21)    ; //!< OFFSET_OUT_2
                uint32_t                 Reserved214                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 OffsetIn3                                        : __CODEGEN_BITFIELD( 0, 10)    ; //!< OFFSET_IN_3
                uint32_t                 OffsetOut3                                       : __CODEGEN_BITFIELD(11, 21)    ; //!< OFFSET_OUT_3
                uint32_t                 Reserved246                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;

        //! \name Local enumerations

        //! \brief YUV_CHANNEL_SWAP
        //! \details
        //!     This bit should only be used with RGB output formats.  When this bit is
        //!     set, the YUV channels are swapped into the output RGB channels as shown
        //!     in the following table:
        //! 
        //!                         <table style="border: black solid 1px;">
        //!                             <tr>
        //!                                 <td> </td>
        //!                                 <td colspan="2">YUV_Channel_Swap</td>
        //!                             </tr>
        //!                             <tr>
        //!                                 <td> </td>
        //!                                 <td style="border: black solid 1px;">0</td>
        //!                                 <td style="border: black solid 1px;">1</td>
        //!                             </tr>
        //!                             <tr>
        //!                                 <td style="border: black solid 1px;">Y</td>
        //!                                 <td style="border: black solid 1px;">R</td>
        //!                                 <td style="border: black solid 1px;">G</td>
        //!                             </tr>
        //!                             <tr>
        //!                                 <td style="border: black solid 1px;">U</td>
        //!                                 <td style="border: black solid 1px;">G</td>
        //!                                 <td style="border: black solid 1px;">B</td>
        //!                             </tr>
        //!                             <tr>
        //!                                 <td style="border: black solid 1px;">V</td>
        //!                                 <td style="border: black solid 1px;">B</td>
        //!                                 <td style="border: black solid 1px;">R</td>
        //!                             </tr>
        //!                         </table>
        enum YUV_CHANNEL_SWAP
        {
            YUV_CHANNEL_SWAP_UNNAMED0                                        = 0, //!< No additional details
        };

        //! \brief C0
        //! \details
        //!     Transform coefficient.
        enum C0
        {
            C0_UNNAMED1024                                                   = 1024, //!< No additional details
        };

        //! \brief C1
        //! \details
        //!     Transform coefficient.
        enum C1
        {
            C1_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief C2
        //! \details
        //!     Transform coefficient.
        enum C2
        {
            C2_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief C3
        //! \details
        //!     Transform coefficient.
        enum C3
        {
            C3_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief C4
        //! \details
        //!     Transform coefficient.
        enum C4
        {
            C4_UNNAMED1024                                                   = 1024, //!< No additional details
        };

        //! \brief C5
        //! \details
        //!     Transform coefficient.
        enum C5
        {
            C5_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief C6
        //! \details
        //!     Transform coefficient.
        enum C6
        {
            C6_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief C7
        //! \details
        //!     Transform coefficient.
        enum C7
        {
            C7_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief C8
        //! \details
        //!     Transform coefficient.
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
        VEBOX_CSC_STATE_CMD();

        static const size_t dwSize = 8;
        static const size_t byteSize = 32;
    };

    //!
    //! \brief VEBOX_DNDI_STATE
    //! \details
    //!     This state table is used by the Denoise and Deinterlacer Functions.
    //!     When DN is used in 12-bit mode with the Capture Pipe all the DN pixel
    //!     thresholds (temporal_diff_th, temp_diff_low, good_neighbor_th) are
    //!     compared with the 8 MSBs of the 12-bit pixels.
    //!
    struct VEBOX_DNDI_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DenoiseAsdThreshold                              : __CODEGEN_BITFIELD( 0,  7)    ; //!< Denoise ASD Threshold
                uint32_t                 DenoiseHistoryIncrease                           : __CODEGEN_BITFIELD( 8, 11)    ; //!< DENOISE_HISTORY_INCREASE
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 DenoiseMaximumHistory                            : __CODEGEN_BITFIELD(16, 23)    ; //!< Denoise Maximum History
                uint32_t                 DenoiseStadThreshold                             : __CODEGEN_BITFIELD(24, 31)    ; //!< Denoise STAD Threshold
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 DenoiseThresholdForSumOfComplexityMeasure        : __CODEGEN_BITFIELD( 0,  7)    ; //!< Denoise Threshold for Sum of Complexity Measure
                uint32_t                 DenoiseMovingPixelThreshold                      : __CODEGEN_BITFIELD( 8, 12)    ; //!< Denoise Moving Pixel Threshold
                uint32_t                 StmmC2                                           : __CODEGEN_BITFIELD(13, 15)    ; //!< STMM C2
                uint32_t                 LowTemporalDifferenceThreshold                   : __CODEGEN_BITFIELD(16, 21)    ; //!< Low Temporal Difference Threshold
                uint32_t                 Reserved54                                       : __CODEGEN_BITFIELD(22, 23)    ; //!< Reserved
                uint32_t                 TemporalDifferenceThreshold                      : __CODEGEN_BITFIELD(24, 29)    ; //!< Temporal Difference Threshold
                uint32_t                 Reserved62                                       : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 BlockNoiseEstimateNoiseThreshold                 : __CODEGEN_BITFIELD( 0,  7)    ; //!< Block Noise Estimate Noise Threshold
                uint32_t                 BlockNoiseEstimateEdgeThreshold                  : __CODEGEN_BITFIELD( 8, 11)    ; //!< BLOCK_NOISE_ESTIMATE_EDGE_THRESHOLD
                uint32_t                 Reserved76                                       : __CODEGEN_BITFIELD(12, 13)    ; //!< Reserved
                uint32_t                 SmoothMvThreshold                                : __CODEGEN_BITFIELD(14, 15)    ; //!< Smooth MV Threshold
                uint32_t                 SadTightThreshold                                : __CODEGEN_BITFIELD(16, 19)    ; //!< SAD_TIGHT_THRESHOLD
                uint32_t                 ContentAdaptiveThresholdSlope                    : __CODEGEN_BITFIELD(20, 23)    ; //!< CONTENT_ADAPTIVE_THRESHOLD_SLOPE
                uint32_t                 GoodNeighborThreshold                            : __CODEGEN_BITFIELD(24, 29)    ; //!< GOOD_NEIGHBOR_THRESHOLD
                uint32_t                 Reserved94                                       : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 MaximumStmm                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< Maximum STMM
                uint32_t                 MultiplierForVecm                                : __CODEGEN_BITFIELD( 8, 13)    ; //!< Multiplier for VECM
                uint32_t                 Reserved110                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 BlendingConstantAcrossTimeForSmallValuesOfStmm   : __CODEGEN_BITFIELD(16, 23)    ; //!< BLENDING_CONSTANT_ACROSS_TIME_FOR_SMALL_VALUES_OF_STMM
                uint32_t                 BlendingConstantAcrossTimeForLargeValuesOfStmm   : __CODEGEN_BITFIELD(24, 30)    ; //!< BLENDING_CONSTANT_ACROSS_TIME_FOR_LARGE_VALUES_OF_STMM
                uint32_t                 StmmBlendingConstantSelect                       : __CODEGEN_BITFIELD(31, 31)    ; //!< STMM_BLENDING_CONSTANT_SELECT
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 SdiDelta                                         : __CODEGEN_BITFIELD( 0,  7)    ; //!< SDI Delta
                uint32_t                 SdiThreshold                                     : __CODEGEN_BITFIELD( 8, 15)    ; //!< SDI Threshold
                uint32_t                 StmmOutputShift                                  : __CODEGEN_BITFIELD(16, 19)    ; //!< STMM Output Shift
                uint32_t                 StmmShiftUp                                      : __CODEGEN_BITFIELD(20, 21)    ; //!< STMM_SHIFT_UP
                uint32_t                 StmmShiftDown                                    : __CODEGEN_BITFIELD(22, 23)    ; //!< STMM_SHIFT_DOWN
                uint32_t                 MinimumStmm                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Minimum STMM
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 FmdTemporalDifferenceThreshold                   : __CODEGEN_BITFIELD( 0,  7)    ; //!< FMD Temporal Difference Threshold
                uint32_t                 SdiFallbackMode2ConstantAngle2X1                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< SDI Fallback Mode 2 Constant (Angle2x1)
                uint32_t                 SdiFallbackMode1T2Constant                       : __CODEGEN_BITFIELD(16, 23)    ; //!< SDI Fallback Mode 1 T2 Constant
                uint32_t                 SdiFallbackMode1T1Constant                       : __CODEGEN_BITFIELD(24, 31)    ; //!< SDI Fallback Mode 1 T1 Constant
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 Reserved192                                      : __CODEGEN_BITFIELD( 0,  2)    ; //!< Reserved
                uint32_t                 DnDiTopFirst                                     : __CODEGEN_BITFIELD( 3,  3)    ; //!< DNDI_TOP_FIRST
                uint32_t                 Reserved196                                      : __CODEGEN_BITFIELD( 4,  5)    ; //!< Reserved
                uint32_t                 ProgressiveDn                                    : __CODEGEN_BITFIELD( 6,  6)    ; //!< PROGRESSIVE_DN
                uint32_t                 McdiEnable                                       : __CODEGEN_BITFIELD( 7,  7)    ; //!< MCDI Enable
                uint32_t                 FmdTearThreshold                                 : __CODEGEN_BITFIELD( 8, 13)    ; //!< FMD Tear Threshold
                uint32_t                 CatThreshold                                     : __CODEGEN_BITFIELD(14, 15)    ; //!< CAT_THRESHOLD
                uint32_t                 Fmd2VerticalDifferenceThreshold                  : __CODEGEN_BITFIELD(16, 23)    ; //!< FMD #2 Vertical Difference Threshold
                uint32_t                 Fmd1VerticalDifferenceThreshold                  : __CODEGEN_BITFIELD(24, 31)    ; //!< FMD #1 Vertical Difference Threshold
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 SadTha                                           : __CODEGEN_BITFIELD( 0,  3)    ; //!< SAD_THA
                uint32_t                 SadThb                                           : __CODEGEN_BITFIELD( 4,  7)    ; //!< SAD_THB
                uint32_t                 ProgressiveCadenceReconstructionFor1StFieldOfCurrentFrame : __CODEGEN_BITFIELD( 8,  9)    ; //!< PROGRESSIVE_CADENCE_RECONSTRUCTION_FOR_1ST_FIELD_OF_CURRENT_FRAME
                uint32_t                 McPixelConsistencyThreshold                      : __CODEGEN_BITFIELD(10, 15)    ; //!< MC_PIXEL_CONSISTENCY_THRESHOLD
                uint32_t                 ProgressiveCadenceReconstructionFor2NdFieldOfPreviousFrame : __CODEGEN_BITFIELD(16, 17)    ; //!< PROGRESSIVE_CADENCE_RECONSTRUCTION_FOR_2ND_FIELD_OF_PREVIOUS_FRAME
                uint32_t                 Reserved242                                      : __CODEGEN_BITFIELD(18, 18)    ; //!< Reserved
                uint32_t                 NeighborPixelThreshold                           : __CODEGEN_BITFIELD(19, 22)    ; //!< NEIGHBOR_PIXEL_THRESHOLD
                uint32_t                 InitialDenoiseHistory                            : __CODEGEN_BITFIELD(23, 28)    ; //!< INITIAL_DENOISE_HISTORY
                uint32_t                 Reserved253                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 ChromaLowTemporalDifferenceThreshold             : __CODEGEN_BITFIELD( 0,  5)    ; //!< Chroma Low Temporal Difference Threshold
                uint32_t                 ChromaTemporalDifferenceThreshold                : __CODEGEN_BITFIELD( 6, 11)    ; //!< Chroma Temporal Difference Threshold
                uint32_t                 ChromaDenoiseEnable                              : __CODEGEN_BITFIELD(12, 12)    ; //!< CHROMA_DENOISE_ENABLE
                uint32_t                 Reserved269                                      : __CODEGEN_BITFIELD(13, 15)    ; //!< Reserved
                uint32_t                 ChromaDenoiseStadThreshold                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Chroma Denoise STAD Threshold
                uint32_t                 Reserved280                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 HotPixelThreshold                                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Hot Pixel Threshold
                uint32_t                 HotPixelCount                                    : __CODEGEN_BITFIELD( 8, 11)    ; //!< Hot Pixel Count
                uint32_t                 Reserved300                                      : __CODEGEN_BITFIELD(12, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;

        //! \name Local enumerations

        //! \brief DENOISE_HISTORY_INCREASE
        //! \details
        //!     Amount that denoise_history is increased
        //!                         MAX:15
        enum DENOISE_HISTORY_INCREASE
        {
            DENOISE_HISTORY_INCREASE_UNNAMED8                                = 8, //!< No additional details
        };

        //! \brief BLOCK_NOISE_ESTIMATE_EDGE_THRESHOLD
        //! \details
        //!     Threshold for detecting an edge in block noise estimate.
        //!                         MAX:15
        enum BLOCK_NOISE_ESTIMATE_EDGE_THRESHOLD
        {
            BLOCK_NOISE_ESTIMATE_EDGE_THRESHOLD_UNNAMED1                     = 1, //!< No additional details
        };

        enum SAD_TIGHT_THRESHOLD
        {
            SAD_TIGHT_THRESHOLD_UNNAMED5                                     = 5, //!< No additional details
        };

        //! \brief CONTENT_ADAPTIVE_THRESHOLD_SLOPE
        //! \details
        //!     Determines the slope of the Content Adaptive Threshold.  +1 added
        //!     internally to get CAT_slope.
        enum CONTENT_ADAPTIVE_THRESHOLD_SLOPE
        {
            CONTENT_ADAPTIVE_THRESHOLD_SLOPE_UNNAMED9                        = 9, //!< CAT_slope value = 10
        };

        //! \brief GOOD_NEIGHBOR_THRESHOLD
        //! \details
        //!     Difference from current pixel for neighboring pixels to be considered a
        //!     good neighbor.
        //!                         MAX:63
        enum GOOD_NEIGHBOR_THRESHOLD
        {
            GOOD_NEIGHBOR_THRESHOLD_UNNAMED4                                 = 4, //!< Depending on GNE of previous frame
        };

        enum BLENDING_CONSTANT_ACROSS_TIME_FOR_SMALL_VALUES_OF_STMM
        {
            BLENDING_CONSTANT_ACROSS_TIME_FOR_SMALL_VALUES_OF_STMM_UNNAMED125 = 125, //!< No additional details
        };

        enum BLENDING_CONSTANT_ACROSS_TIME_FOR_LARGE_VALUES_OF_STMM
        {
            BLENDING_CONSTANT_ACROSS_TIME_FOR_LARGE_VALUES_OF_STMM_UNNAMED64 = 64, //!< No additional details
        };

        enum STMM_BLENDING_CONSTANT_SELECT
        {
            STMM_BLENDING_CONSTANT_SELECT_USETHEBLENDINGCONSTANTFORSMALLVALUESOFSTMMFORSTMMMDTH = 0, //!< No additional details
            STMM_BLENDING_CONSTANT_SELECT_USETHEBLENDINGCONSTANTFORLARGEVALUESOFSTMMFORSTMMMDTH = 1, //!< No additional details
        };

        //! \brief STMM_SHIFT_UP
        //! \details
        //!     Amount to shift STMM up (set range).
        enum STMM_SHIFT_UP
        {
            STMM_SHIFT_UP_SHIFTBY6                                           = 0, //!< No additional details
            STMM_SHIFT_UP_SHIFTBY7                                           = 1, //!< No additional details
            STMM_SHIFT_UP_SHIFTBY8                                           = 2, //!< No additional details
        };

        //! \brief STMM_SHIFT_DOWN
        //! \details
        //!     Amount to shift STMM down (quantize to fewer bits)
        enum STMM_SHIFT_DOWN
        {
            STMM_SHIFT_DOWN_SHIFTBY4                                         = 0, //!< No additional details
            STMM_SHIFT_DOWN_SHIFTBY5                                         = 1, //!< No additional details
            STMM_SHIFT_DOWN_SHIFTBY6                                         = 2, //!< No additional details
        };

        //! \brief DNDI_TOP_FIRST
        //! \details
        //!     Indicates the top field is first in sequence, otherwise bottom is first
        enum DNDI_TOP_FIRST
        {
            DNDI_TOP_FIRST_BOTTOMFIELDOCCURSFIRSTINSEQUENCE                  = 0, //!< No additional details
            DNDI_TOP_FIRST_TOPFIELDOCCURSFIRSTINSEQUENCE                     = 1, //!< No additional details
        };

        //! \brief PROGRESSIVE_DN
        //! \details
        //!     Indicates that the denoise algorithm should assume progressive input
        //!     when filtering neighboring pixels.  <b>DI Enable</b> must be disabled
        //!     when this field is enabled
        enum PROGRESSIVE_DN
        {
            PROGRESSIVE_DN_DNASSUMESINTERLACEDVIDEOANDFILTERSALTERNATELINESTOGETHER = 0, //!< No additional details
            PROGRESSIVE_DN_DNASSUMESPROGRESSIVEVIDEOANDFILTERSNEIGHBORINGLINESTOGETHER = 1, //!< No additional details
        };

        enum CAT_THRESHOLD
        {
            CAT_THRESHOLD_UNNAMED0                                           = 0, //!< No additional details
        };

        enum SAD_THA
        {
            SAD_THA_UNNAMED5                                                 = 5, //!< No additional details
        };

        enum SAD_THB
        {
            SAD_THB_UNNAMED10                                                = 10, //!< No additional details
        };

        enum PROGRESSIVE_CADENCE_RECONSTRUCTION_FOR_1ST_FIELD_OF_CURRENT_FRAME
        {
            PROGRESSIVE_CADENCE_RECONSTRUCTION_FOR_1ST_FIELD_OF_CURRENT_FRAME_DEINTERLACE = 0, //!< No additional details
            PROGRESSIVE_CADENCE_RECONSTRUCTION_FOR_1ST_FIELD_OF_CURRENT_FRAME_PUTTOGETHERWITHPREVIOUSFIELDINSEQUENCE = 1, //!< 2nd field of previous frame
            PROGRESSIVE_CADENCE_RECONSTRUCTION_FOR_1ST_FIELD_OF_CURRENT_FRAME_PUTTOGETHERWITHNEXTFIELDINSEQUENCE = 2, //!< 2nd field of current frame
        };

        enum MC_PIXEL_CONSISTENCY_THRESHOLD
        {
            MC_PIXEL_CONSISTENCY_THRESHOLD_UNNAMED25                         = 25, //!< No additional details
        };

        enum PROGRESSIVE_CADENCE_RECONSTRUCTION_FOR_2ND_FIELD_OF_PREVIOUS_FRAME
        {
            PROGRESSIVE_CADENCE_RECONSTRUCTION_FOR_2ND_FIELD_OF_PREVIOUS_FRAME_DEINTERLACE = 0, //!< No additional details
            PROGRESSIVE_CADENCE_RECONSTRUCTION_FOR_2ND_FIELD_OF_PREVIOUS_FRAME_PUTTOGETHERWITHPREVIOUSFIELDINSEQUENCE = 1, //!< 1st field of previous frame
            PROGRESSIVE_CADENCE_RECONSTRUCTION_FOR_2ND_FIELD_OF_PREVIOUS_FRAME_PUTTOGETHERWITHNEXTFIELDINSEQUENCE = 2, //!< 1st field of current frame
        };

        enum NEIGHBOR_PIXEL_THRESHOLD
        {
            NEIGHBOR_PIXEL_THRESHOLD_UNNAMED10                               = 10, //!< No additional details
        };

        //! \brief INITIAL_DENOISE_HISTORY
        //! \details
        //!     Initial value for Denoise history for both Luma and Chroma.
        enum INITIAL_DENOISE_HISTORY
        {
            INITIAL_DENOISE_HISTORY_UNNAMED32                                = 32, //!< No additional details
        };

        enum CHROMA_DENOISE_ENABLE
        {
            CHROMA_DENOISE_ENABLE_THEUANDVCHANNELSWILLBEPASSEDTOTHENEXTSTAGEAFTERDNUNCHANGED = 0, //!< No additional details
            CHROMA_DENOISE_ENABLE_THEUANDVCHROMACHANNELSWILLBEDENOISEFILTERED = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VEBOX_DNDI_STATE_CMD();

        static const size_t dwSize = 10;
        static const size_t byteSize = 40;
    };

    //!
    //! \brief VEBOX_FRONT_END_CSC_STATE
    //! \details
    //!     This state structure contains the IECP State Table Contents for
    //!     Front-end CSC state.
    //!
    struct VEBOX_FRONT_END_CSC_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 FfrontEndCScTransformEnable                      : __CODEGEN_BITFIELD( 0,  0)    ; //!< FFront End C SC Transform Enable
                uint32_t                 Reserved1                                        : __CODEGEN_BITFIELD( 1,  2)    ; //!< Reserved
                uint32_t                 FecscC0TransformCoefficient                      : __CODEGEN_BITFIELD( 3, 15)    ; //!< FECSC_C0_TRANSFORM_COEFFICIENT
                uint32_t                 FecscC1TransformCoefficient                      : __CODEGEN_BITFIELD(16, 28)    ; //!< FECSC_C1_TRANSFORM_COEFFICIENT
                uint32_t                 Reserved29                                       : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 FecScC2TransformCoefficient                      : __CODEGEN_BITFIELD( 0, 12)    ; //!< FEC_SC_C2_TRANSFORM_COEFFICIENT
                uint32_t                 FecScC3TransformCoefficient                      : __CODEGEN_BITFIELD(13, 25)    ; //!< FEC_SC_C3_TRANSFORM_COEFFICIENT
                uint32_t                 Reserved58                                       : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 FecScC4TransformCoefficient                      : __CODEGEN_BITFIELD( 0, 12)    ; //!< FEC_SC_C4_TRANSFORM_COEFFICIENT
                uint32_t                 FecScC5TransformCoefficient                      : __CODEGEN_BITFIELD(13, 25)    ; //!< FEC_SC_C5_TRANSFORM_COEFFICIENT
                uint32_t                 Reserved90                                       : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 FecScC6TransformCoefficient                      : __CODEGEN_BITFIELD( 0, 12)    ; //!< FEC_SC_C6_TRANSFORM_COEFFICIENT
                uint32_t                 FecScC7TransformCoefficient                      : __CODEGEN_BITFIELD(13, 25)    ; //!< FEC_SC_C7_TRANSFORM_COEFFICIENT
                uint32_t                 Reserved122                                      : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 FecScC8TransformCoefficient                      : __CODEGEN_BITFIELD( 0, 12)    ; //!< FEC_SC_C8_TRANSFORM_COEFFICIENT
                uint32_t                 Reserved141                                      : __CODEGEN_BITFIELD(13, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 FecScOffsetIn1OffsetInForYR                      : __CODEGEN_BITFIELD( 0, 10)    ; //!< FEC_SC_OFFSET_IN_1_OFFSET_IN_FOR_YR
                uint32_t                 FecScOffsetOut1OffsetOutForYR                    : __CODEGEN_BITFIELD(11, 21)    ; //!< FEC_SC_OFFSET_OUT_1_OFFSET_OUT_FOR_YR
                uint32_t                 Reserved182                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 FecScOffsetIn2OffsetInForUG                      : __CODEGEN_BITFIELD( 0, 10)    ; //!< FEC_SC_OFFSET_IN_2_OFFSET_IN_FOR_UG
                uint32_t                 FecScOffsetOut2OffsetOutForUG                    : __CODEGEN_BITFIELD(11, 21)    ; //!< FEC_SC_OFFSET_OUT_2_OFFSET_OUT_FOR_UG
                uint32_t                 Reserved214                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 FecScOffsetIn3OffsetInForVB                      : __CODEGEN_BITFIELD( 0, 10)    ; //!< FEC_SC_OFFSET_IN_3_OFFSET_IN_FOR_VB
                uint32_t                 FecScOffsetOut3OffsetOutForVB                    : __CODEGEN_BITFIELD(11, 21)    ; //!< FEC_SC_OFFSET_OUT_3_OFFSET_OUT_FOR_VB
                uint32_t                 Reserved246                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;

        //! \name Local enumerations

        enum FECSC_C0_TRANSFORM_COEFFICIENT
        {
            FECSC_C0_TRANSFORM_COEFFICIENT_1024                              = 1024, //!< No additional details
        };

        enum FECSC_C1_TRANSFORM_COEFFICIENT
        {
            FECSC_C1_TRANSFORM_COEFFICIENT_0                                 = 0, //!< No additional details
        };

        enum FEC_SC_C2_TRANSFORM_COEFFICIENT
        {
            FEC_SC_C2_TRANSFORM_COEFFICIENT_UNNAMED0                         = 0, //!< No additional details
        };

        enum FEC_SC_C3_TRANSFORM_COEFFICIENT
        {
            FEC_SC_C3_TRANSFORM_COEFFICIENT_UNNAMED0                         = 0, //!< No additional details
        };

        enum FEC_SC_C4_TRANSFORM_COEFFICIENT
        {
            FEC_SC_C4_TRANSFORM_COEFFICIENT_1024                             = 1024, //!< No additional details
        };

        enum FEC_SC_C5_TRANSFORM_COEFFICIENT
        {
            FEC_SC_C5_TRANSFORM_COEFFICIENT_UNNAMED0                         = 0, //!< No additional details
        };

        enum FEC_SC_C6_TRANSFORM_COEFFICIENT
        {
            FEC_SC_C6_TRANSFORM_COEFFICIENT_UNNAMED0                         = 0, //!< No additional details
        };

        enum FEC_SC_C7_TRANSFORM_COEFFICIENT
        {
            FEC_SC_C7_TRANSFORM_COEFFICIENT_UNNAMED0                         = 0, //!< No additional details
        };

        enum FEC_SC_C8_TRANSFORM_COEFFICIENT
        {
            FEC_SC_C8_TRANSFORM_COEFFICIENT_1024                             = 1024, //!< No additional details
        };

        enum FEC_SC_OFFSET_IN_1_OFFSET_IN_FOR_YR
        {
            FEC_SC_OFFSET_IN_1_OFFSET_IN_FOR_YR_UNNAMED0                     = 0, //!< No additional details
        };

        enum FEC_SC_OFFSET_OUT_1_OFFSET_OUT_FOR_YR
        {
            FEC_SC_OFFSET_OUT_1_OFFSET_OUT_FOR_YR_UNNAMED0                   = 0, //!< No additional details
        };

        enum FEC_SC_OFFSET_IN_2_OFFSET_IN_FOR_UG
        {
            FEC_SC_OFFSET_IN_2_OFFSET_IN_FOR_UG_UNNAMED0                     = 0, //!< No additional details
        };

        enum FEC_SC_OFFSET_OUT_2_OFFSET_OUT_FOR_UG
        {
            FEC_SC_OFFSET_OUT_2_OFFSET_OUT_FOR_UG_UNNAMED0                   = 0, //!< No additional details
        };

        enum FEC_SC_OFFSET_IN_3_OFFSET_IN_FOR_VB
        {
            FEC_SC_OFFSET_IN_3_OFFSET_IN_FOR_VB_UNNAMED0                     = 0, //!< No additional details
        };

        enum FEC_SC_OFFSET_OUT_3_OFFSET_OUT_FOR_VB
        {
            FEC_SC_OFFSET_OUT_3_OFFSET_OUT_FOR_VB_UNNAMED0                   = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VEBOX_FRONT_END_CSC_STATE_CMD();

        static const size_t dwSize = 8;
        static const size_t byteSize = 32;
    };

    //!
    //! \brief VEBOX_GAMUT_STATE
    //! \details
    //! 
    //!
    struct VEBOX_GAMUT_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 CmW                                              : __CODEGEN_BITFIELD( 0,  9)    ; //!< CM(w)
                uint32_t                 Reserved10                                       : __CODEGEN_BITFIELD(10, 14)    ; //!< Reserved
                uint32_t                 GlobalModeEnable                                 : __CODEGEN_BITFIELD(15, 15)    ; //!< GLOBAL_MODE_ENABLE
                uint32_t                 AR                                               : __CODEGEN_BITFIELD(16, 24)    ; //!< AR
                uint32_t                 Reserved25                                       : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 AB                                               : __CODEGEN_BITFIELD( 0,  6)    ; //!< A(b)
                uint32_t                 Reserved39                                       : __CODEGEN_BITFIELD( 7,  7)    ; //!< Reserved
                uint32_t                 AG                                               : __CODEGEN_BITFIELD( 8, 14)    ; //!< A(g)
                uint32_t                 Reserved47                                       : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 CmS                                              : __CODEGEN_BITFIELD(16, 25)    ; //!< CM(s)
                uint32_t                 Reserved58                                       : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 RI                                               : __CODEGEN_BITFIELD( 0,  7)    ; //!< R(i)
                uint32_t                 CmI                                              : __CODEGEN_BITFIELD( 8, 15)    ; //!< CM(i)
                uint32_t                 RS                                               : __CODEGEN_BITFIELD(16, 25)    ; //!< R(s)
                uint32_t                 Reserved90                                       : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 C0                                               : __CODEGEN_BITFIELD( 0, 14)    ; //!< C0
                uint32_t                 Reserved111                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 C1                                               : __CODEGEN_BITFIELD(16, 30)    ; //!< C1
                uint32_t                 Reserved127                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 C2                                               : __CODEGEN_BITFIELD( 0, 14)    ; //!< C2
                uint32_t                 Reserved143                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 C3                                               : __CODEGEN_BITFIELD(16, 30)    ; //!< C3
                uint32_t                 Reserved159                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 C4                                               : __CODEGEN_BITFIELD( 0, 14)    ; //!< C4
                uint32_t                 Reserved175                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 C5                                               : __CODEGEN_BITFIELD(16, 30)    ; //!< C5
                uint32_t                 Reserved191                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 C6                                               : __CODEGEN_BITFIELD( 0, 14)    ; //!< C6
                uint32_t                 Reserved207                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 C7                                               : __CODEGEN_BITFIELD(16, 30)    ; //!< C7
                uint32_t                 Reserved223                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 C8                                               : __CODEGEN_BITFIELD( 0, 14)    ; //!< C8
                uint32_t                 Reserved239                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 PwlGammaPoint1                                   : __CODEGEN_BITFIELD( 0,  7)    ; //!< PWL_GAMMA_POINT_1
                uint32_t                 PwlGammaPoint2                                   : __CODEGEN_BITFIELD( 8, 15)    ; //!< PWL_GAMMA_POINT_2
                uint32_t                 PwlGammaPoint3                                   : __CODEGEN_BITFIELD(16, 23)    ; //!< PWL_GAMMA_POINT_3
                uint32_t                 PwlGammaPoint4                                   : __CODEGEN_BITFIELD(24, 31)    ; //!< PWL_GAMMA_POINT_4
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 PwlGammaPoint5                                   : __CODEGEN_BITFIELD( 0,  7)    ; //!< PWL_GAMMA_POINT_5
                uint32_t                 PwlGammaPoint6                                   : __CODEGEN_BITFIELD( 8, 15)    ; //!< PWL_GAMMA_POINT_6
                uint32_t                 PwlGammaPoint7                                   : __CODEGEN_BITFIELD(16, 23)    ; //!< PWL_GAMMA_POINT_7
                uint32_t                 PwlGammaPoint8                                   : __CODEGEN_BITFIELD(24, 31)    ; //!< PWL_GAMMA_POINT_8
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 PwlGammaPoint9                                   : __CODEGEN_BITFIELD( 0,  7)    ; //!< PWL_GAMMA_POINT_9
                uint32_t                 PwlGammaPoint10                                  : __CODEGEN_BITFIELD( 8, 15)    ; //!< PWL_GAMMA_POINT_10
                uint32_t                 PwlGammaPoint11                                  : __CODEGEN_BITFIELD(16, 23)    ; //!< PWL_GAMMA_POINT_11
                uint32_t                 Reserved344                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 PwlGammaBias1                                    : __CODEGEN_BITFIELD( 0,  7)    ; //!< PWL_GAMMA_BIAS_1
                uint32_t                 PwlGammaBias2                                    : __CODEGEN_BITFIELD( 8, 15)    ; //!< PWL_GAMMA_BIAS_2
                uint32_t                 PwlGammaBias3                                    : __CODEGEN_BITFIELD(16, 23)    ; //!< PWL_GAMMA_BIAS_3
                uint32_t                 PwlGammaBias4                                    : __CODEGEN_BITFIELD(24, 31)    ; //!< PWL_GAMMA_BIAS_4
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 PwlGammaBias5                                    : __CODEGEN_BITFIELD( 0,  7)    ; //!< PWL_GAMMA_BIAS_5
                uint32_t                 PwlGammaBias6                                    : __CODEGEN_BITFIELD( 8, 15)    ; //!< PWL_GAMMA_BIAS_6
                uint32_t                 PwlGammaBias7                                    : __CODEGEN_BITFIELD(16, 23)    ; //!< PWL_GAMMA_BIAS_7
                uint32_t                 PwlGammaBias8                                    : __CODEGEN_BITFIELD(24, 31)    ; //!< PWL_GAMMA_BIAS_8
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 PwlGammaBias9                                    : __CODEGEN_BITFIELD( 0,  7)    ; //!< PWL_GAMMA_BIAS_9
                uint32_t                 PwlGammaBias10                                   : __CODEGEN_BITFIELD( 8, 15)    ; //!< PWL_GAMMA_BIAS_10
                uint32_t                 PwlGammaBias11                                   : __CODEGEN_BITFIELD(16, 23)    ; //!< PWL_GAMMA_BIAS_11
                uint32_t                 Reserved440                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 PwlGammaSlope0                                   : __CODEGEN_BITFIELD( 0, 11)    ; //!< PWL_Gamma_ Slope_0
                uint32_t                 Reserved460                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 PwlGammaSlope1                                   : __CODEGEN_BITFIELD(16, 27)    ; //!< PWL_Gamma_ Slope_1
                uint32_t                 Reserved476                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 PwlGammaSlope2                                   : __CODEGEN_BITFIELD( 0, 11)    ; //!< PWL_Gamma_ Slope_2
                uint32_t                 Reserved492                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 PwlGammaSlope3                                   : __CODEGEN_BITFIELD(16, 27)    ; //!< PWL_Gamma_ Slope_3
                uint32_t                 Reserved508                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            //!< DWORD 16
            struct
            {
                uint32_t                 PwlGammaSlope4                                   : __CODEGEN_BITFIELD( 0, 11)    ; //!< PWL_Gamma_ Slope_4
                uint32_t                 Reserved524                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 PwlGammaSlope5                                   : __CODEGEN_BITFIELD(16, 27)    ; //!< PWL_Gamma_ Slope_5
                uint32_t                 Reserved540                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            //!< DWORD 17
            struct
            {
                uint32_t                 PwlGammaSlope6                                   : __CODEGEN_BITFIELD( 0, 11)    ; //!< PWL_Gamma_ Slope_6
                uint32_t                 Reserved556                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 PwlGammaSlope7                                   : __CODEGEN_BITFIELD(16, 27)    ; //!< PWL_Gamma_ Slope_7
                uint32_t                 Reserved572                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            //!< DWORD 18
            struct
            {
                uint32_t                 PwlGammaSlope8                                   : __CODEGEN_BITFIELD( 0, 11)    ; //!< PWL_Gamma_ Slope_8
                uint32_t                 Reserved588                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 PwlGammaSlope9                                   : __CODEGEN_BITFIELD(16, 27)    ; //!< PWL_Gamma_ Slope_9
                uint32_t                 Reserved604                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            //!< DWORD 19
            struct
            {
                uint32_t                 PwlGammaSlope10                                  : __CODEGEN_BITFIELD( 0, 11)    ; //!< PWL_Gamma_ Slope_10
                uint32_t                 Reserved620                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 PwlGammaSlope11                                  : __CODEGEN_BITFIELD(16, 27)    ; //!< PWL_Gamma_ Slope_11
                uint32_t                 Reserved636                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            //!< DWORD 20
            struct
            {
                uint32_t                 PwlInvGammaPoint1                                : __CODEGEN_BITFIELD( 0,  7)    ; //!< PWL_INV_GAMMA_POINT_1
                uint32_t                 PwlInvGammaPoint2                                : __CODEGEN_BITFIELD( 8, 15)    ; //!< PWL_INV_GAMMA_POINT_2
                uint32_t                 PwlInvGammaPoint3                                : __CODEGEN_BITFIELD(16, 23)    ; //!< PWL_INV_GAMMA_POINT_3
                uint32_t                 PwlInvGammaPoint4                                : __CODEGEN_BITFIELD(24, 31)    ; //!< PWL_INV_GAMMA_POINT_4
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            //!< DWORD 21
            struct
            {
                uint32_t                 PwlInvGammaPoint5                                : __CODEGEN_BITFIELD( 0,  7)    ; //!< PWL_INV_GAMMA_POINT_5
                uint32_t                 PwlInvGammaPoint6                                : __CODEGEN_BITFIELD( 8, 15)    ; //!< PWL_INV_GAMMA_POINT_6
                uint32_t                 PwlInvGammaPoint7                                : __CODEGEN_BITFIELD(16, 23)    ; //!< PWL_INV_GAMMA_POINT_7
                uint32_t                 PwlInvGammaPoint8                                : __CODEGEN_BITFIELD(24, 31)    ; //!< PWL_INV_GAMMA_POINT_8
            };
            uint32_t                     Value;
        } DW21;
        union
        {
            //!< DWORD 22
            struct
            {
                uint32_t                 PwlInvGammaPoint9                                : __CODEGEN_BITFIELD( 0,  7)    ; //!< PWL_INV_GAMMA_POINT_9
                uint32_t                 PwlInvGammaPoint10                               : __CODEGEN_BITFIELD( 8, 15)    ; //!< PWL_INV_GAMMA_POINT_10
                uint32_t                 PwlInvGammaPoint11                               : __CODEGEN_BITFIELD(16, 23)    ; //!< PWL_INV_GAMMA_POINT_11
                uint32_t                 Reserved728                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW22;
        union
        {
            //!< DWORD 23
            struct
            {
                uint32_t                 PwlInvGammaBias1                                 : __CODEGEN_BITFIELD( 0,  7)    ; //!< PWL_INV_GAMMA_BIAS_1
                uint32_t                 PwlInvGammaBias2                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< PWL_INV_GAMMA_BIAS_2
                uint32_t                 PwlInvGammaBias3                                 : __CODEGEN_BITFIELD(16, 23)    ; //!< PWL_INV_GAMMA_BIAS_3
                uint32_t                 PwlInvGammaBias4                                 : __CODEGEN_BITFIELD(24, 31)    ; //!< PWL_INV_GAMMA_BIAS_4
            };
            uint32_t                     Value;
        } DW23;
        union
        {
            //!< DWORD 24
            struct
            {
                uint32_t                 PwlInvGammaBias5                                 : __CODEGEN_BITFIELD( 0,  7)    ; //!< PWL_INV_GAMMA_BIAS_5
                uint32_t                 PwlInvGammaBias6                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< PWL_INV_GAMMA_BIAS_6
                uint32_t                 PwlInvGammaBias7                                 : __CODEGEN_BITFIELD(16, 23)    ; //!< PWL_INV_GAMMA_BIAS_7
                uint32_t                 PwlInvGammaBias8                                 : __CODEGEN_BITFIELD(24, 31)    ; //!< PWL_INV_GAMMA_BIAS_8
            };
            uint32_t                     Value;
        } DW24;
        union
        {
            //!< DWORD 25
            struct
            {
                uint32_t                 PwlInvGammaBias9                                 : __CODEGEN_BITFIELD( 0,  7)    ; //!< PWL_INV_GAMMA_BIAS_9
                uint32_t                 PwlInvGammaBias10                                : __CODEGEN_BITFIELD( 8, 15)    ; //!< PWL_INV_GAMMA_BIAS_10
                uint32_t                 PwlInvGammaBias11                                : __CODEGEN_BITFIELD(16, 23)    ; //!< PWL_INV_GAMMA_BIAS_11
                uint32_t                 Reserved824                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW25;
        union
        {
            //!< DWORD 26
            struct
            {
                uint32_t                 PwlInvGammaSlope0                                : __CODEGEN_BITFIELD( 0, 11)    ; //!< PWL_INV_GAMMA_ Slope_0
                uint32_t                 Reserved844                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 PwlInvGammaSlope1                                : __CODEGEN_BITFIELD(16, 27)    ; //!< PWL_INV_GAMMA_ Slope_1
                uint32_t                 Reserved860                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW26;
        union
        {
            //!< DWORD 27
            struct
            {
                uint32_t                 PwlInvGammaSlope2                                : __CODEGEN_BITFIELD( 0, 11)    ; //!< PWL_INV_GAMMA_ Slope_2
                uint32_t                 Reserved876                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 PwlInvGammaSlope3                                : __CODEGEN_BITFIELD(16, 27)    ; //!< PWL_INV_GAMMA_ Slope_3
                uint32_t                 Reserved892                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW27;
        union
        {
            //!< DWORD 28
            struct
            {
                uint32_t                 PwlInvGammaSlope4                                : __CODEGEN_BITFIELD( 0, 11)    ; //!< PWL_INV_GAMMA_ Slope_4
                uint32_t                 Reserved908                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 PwlInvGammaSlope5                                : __CODEGEN_BITFIELD(16, 27)    ; //!< PWL_INV_GAMMA_ Slope_5
                uint32_t                 Reserved924                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW28;
        union
        {
            //!< DWORD 29
            struct
            {
                uint32_t                 PwlInvGammaSlope6                                : __CODEGEN_BITFIELD( 0, 11)    ; //!< PWL_INV_GAMMA_ Slope_6
                uint32_t                 Reserved940                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 PwlInvGammaSlope7                                : __CODEGEN_BITFIELD(16, 27)    ; //!< PWL_INV_GAMMA_ Slope_7
                uint32_t                 Reserved956                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW29;
        union
        {
            //!< DWORD 30
            struct
            {
                uint32_t                 PwlInvGammaSlope8                                : __CODEGEN_BITFIELD( 0, 11)    ; //!< PWL_INV_GAMMA_ Slope_8
                uint32_t                 Reserved972                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 PwlInvGammaSlope9                                : __CODEGEN_BITFIELD(16, 27)    ; //!< PWL_INV_GAMMA_ Slope_9
                uint32_t                 Reserved988                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW30;
        union
        {
            //!< DWORD 31
            struct
            {
                uint32_t                 PwlInvGammaSlope10                               : __CODEGEN_BITFIELD( 0, 11)    ; //!< PWL_INV_GAMMA_ Slope_10
                uint32_t                 Reserved1004                                     : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 PwlInvGammaSlope11                               : __CODEGEN_BITFIELD(16, 27)    ; //!< PWL_INV_GAMMA_ Slope_11
                uint32_t                 Reserved1020                                     : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW31;
        union
        {
            //!< DWORD 32
            struct
            {
                uint32_t                 OffsetInR                                        : __CODEGEN_BITFIELD( 0, 14)    ; //!< OFFSET_IN_R
                uint32_t                 Reserved1039                                     : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 OffsetInG                                        : __CODEGEN_BITFIELD(16, 30)    ; //!< OFFSET_IN_G
                uint32_t                 Reserved1055                                     : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW32;
        union
        {
            //!< DWORD 33
            struct
            {
                uint32_t                 OffsetInB                                        : __CODEGEN_BITFIELD( 0, 14)    ; //!< OFFSET_IN_B
                uint32_t                 Reserved1071                                     : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 OffsetOutB                                       : __CODEGEN_BITFIELD(16, 30)    ; //!< Offset_out_B
                uint32_t                 Reserved1087                                     : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW33;
        union
        {
            //!< DWORD 34
            struct
            {
                uint32_t                 OffsetOutR                                       : __CODEGEN_BITFIELD( 0, 14)    ; //!< Offset_out_R
                uint32_t                 Reserved1103                                     : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 OffsetOutG                                       : __CODEGEN_BITFIELD(16, 30)    ; //!< Offset_out_G
                uint32_t                 Reserved1119                                     : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW34;
        union
        {
            //!< DWORD 35
            struct
            {
                uint32_t                 D1Out                                            : __CODEGEN_BITFIELD( 0,  9)    ; //!< D1OUT
                uint32_t                 DOutDefault                                      : __CODEGEN_BITFIELD(10, 19)    ; //!< DOUT_DEFAULT
                uint32_t                 DInDefault                                       : __CODEGEN_BITFIELD(20, 29)    ; //!< DINDEFAULT
                uint32_t                 Fullrangemappingenable                           : __CODEGEN_BITFIELD(30, 30)    ; //!< FULLRANGEMAPPINGENABLE
                uint32_t                 Reserved1151                                     : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW35;
        union
        {
            //!< DWORD 36
            struct
            {
                uint32_t                 D1In                                             : __CODEGEN_BITFIELD( 0,  9)    ; //!< D1IN
                uint32_t                 Reserved1162                                     : __CODEGEN_BITFIELD(10, 27)    ; //!< Reserved
                uint32_t                 Compressionlineshift                             : __CODEGEN_BITFIELD(28, 30)    ; //!< COMPRESSIONLINESHIFT
                uint32_t                 Xvyccdecencenable                                : __CODEGEN_BITFIELD(31, 31)    ; //!< XVYCCDECENCENABLE
            };
            uint32_t                     Value;
        } DW36;
        union
        {
            //!< DWORD 37
            struct
            {
                uint32_t                 CpiOverride                                      : __CODEGEN_BITFIELD( 0,  0)    ; //!< CPI_OVERRIDE
                uint32_t                 Reserved1185                                     : __CODEGEN_BITFIELD( 1, 10)    ; //!< Reserved
                uint32_t                 Basicmodescalingfactor                           : __CODEGEN_BITFIELD(11, 24)    ; //!< BasicModeScalingFactor
                uint32_t                 Reserved1209                                     : __CODEGEN_BITFIELD(25, 28)    ; //!< Reserved
                uint32_t                 Lumachormaonlycorrection                         : __CODEGEN_BITFIELD(29, 29)    ; //!< LUMACHORMAONLYCORRECTION
                uint32_t                 GccBasicmodeselection                            : __CODEGEN_BITFIELD(30, 31)    ; //!< GCC_BASICMODESELECTION
            };
            uint32_t                     Value;
        } DW37;

        //! \name Local enumerations

        //! \brief GLOBAL_MODE_ENABLE
        //! \details
        //!     The gain factor derived from state  CM(w)
        enum GLOBAL_MODE_ENABLE
        {
            GLOBAL_MODE_ENABLE_ADVANCEMODE                                   = 0, //!< No additional details
            GLOBAL_MODE_ENABLE_BASICMODE                                     = 1, //!< No additional details
        };

        //! \brief AR
        //! \details
        //!     Gain_factor_R (default: 436, preferred range: 256-511)
        enum AR
        {
            AR_UNNAMED436                                                    = 436, //!< No additional details
        };

        //! \brief PWL_GAMMA_POINT_1
        //! \details
        //!     Point 1 for PWL for gamma correction
        enum PWL_GAMMA_POINT_1
        {
            PWL_GAMMA_POINT_1_UNNAMED1                                       = 1, //!< No additional details
        };

        //! \brief PWL_GAMMA_POINT_2
        //! \details
        //!     Point 2 for PWL for gamma correction
        enum PWL_GAMMA_POINT_2
        {
            PWL_GAMMA_POINT_2_UNNAMED2                                       = 2, //!< No additional details
        };

        //! \brief PWL_GAMMA_POINT_3
        //! \details
        //!     Point 3 for PWL for gamma correction
        enum PWL_GAMMA_POINT_3
        {
            PWL_GAMMA_POINT_3_UNNAMED5                                       = 5, //!< No additional details
        };

        //! \brief PWL_GAMMA_POINT_4
        //! \details
        //!     Point 4 for PWL for gamma correction
        enum PWL_GAMMA_POINT_4
        {
            PWL_GAMMA_POINT_4_UNNAMED9                                       = 9, //!< No additional details
        };

        //! \brief PWL_GAMMA_POINT_5
        //! \details
        //!     Point 5 for PWL for gamma correction
        enum PWL_GAMMA_POINT_5
        {
            PWL_GAMMA_POINT_5_UNNAMED16                                      = 16, //!< No additional details
        };

        //! \brief PWL_GAMMA_POINT_6
        //! \details
        //!     Point 6 for PWL for gamma correction
        enum PWL_GAMMA_POINT_6
        {
            PWL_GAMMA_POINT_6_UNNAMED26                                      = 26, //!< No additional details
        };

        //! \brief PWL_GAMMA_POINT_7
        //! \details
        //!     Point 7 for PWL for gamma correction
        enum PWL_GAMMA_POINT_7
        {
            PWL_GAMMA_POINT_7_UNNAMED42                                      = 42, //!< No additional details
        };

        //! \brief PWL_GAMMA_POINT_8
        //! \details
        //!     Point 8 for PWL for gamma correction
        enum PWL_GAMMA_POINT_8
        {
            PWL_GAMMA_POINT_8_UNNAMED65                                      = 65, //!< No additional details
        };

        //! \brief PWL_GAMMA_POINT_9
        //! \details
        //!     Point 9 for PWL for gamma correction
        enum PWL_GAMMA_POINT_9
        {
            PWL_GAMMA_POINT_9_UNNAMED96                                      = 96, //!< No additional details
        };

        //! \brief PWL_GAMMA_POINT_10
        //! \details
        //!     Point 10 for PWL for gamma correction
        enum PWL_GAMMA_POINT_10
        {
            PWL_GAMMA_POINT_10_UNNAMED136                                    = 136, //!< No additional details
        };

        //! \brief PWL_GAMMA_POINT_11
        //! \details
        //!     Point 11 for PWL for gamma correction
        enum PWL_GAMMA_POINT_11
        {
            PWL_GAMMA_POINT_11_UNNAMED187                                    = 187, //!< No additional details
        };

        //! \brief PWL_GAMMA_BIAS_1
        //! \details
        //!     Bias 1 for PWL for gamma correction
        enum PWL_GAMMA_BIAS_1
        {
            PWL_GAMMA_BIAS_1_UNNAMED13                                       = 13, //!< No additional details
        };

        //! \brief PWL_GAMMA_BIAS_2
        //! \details
        //!     Bias 2 for PWL for gamma correction
        enum PWL_GAMMA_BIAS_2
        {
            PWL_GAMMA_BIAS_2_UNNAMED23                                       = 23, //!< No additional details
        };

        //! \brief PWL_GAMMA_BIAS_3
        //! \details
        //!     Bias 3 for PWL for gamma correction
        enum PWL_GAMMA_BIAS_3
        {
            PWL_GAMMA_BIAS_3_UNNAMED38                                       = 38, //!< No additional details
        };

        //! \brief PWL_GAMMA_BIAS_4
        //! \details
        //!     Bias 4 for PWL for gamma correction
        enum PWL_GAMMA_BIAS_4
        {
            PWL_GAMMA_BIAS_4_UNNAMED53                                       = 53, //!< No additional details
        };

        //! \brief PWL_GAMMA_BIAS_5
        //! \details
        //!     Bias 5 for PWL for gamma correction
        enum PWL_GAMMA_BIAS_5
        {
            PWL_GAMMA_BIAS_5_UNNAMED71                                       = 71, //!< No additional details
        };

        //! \brief PWL_GAMMA_BIAS_6
        //! \details
        //!     Bias 6 for PWL for gamma correction
        enum PWL_GAMMA_BIAS_6
        {
            PWL_GAMMA_BIAS_6_UNNAMED91                                       = 91, //!< No additional details
        };

        //! \brief PWL_GAMMA_BIAS_7
        //! \details
        //!     Bias 7 for PWL for gamma correction
        enum PWL_GAMMA_BIAS_7
        {
            PWL_GAMMA_BIAS_7_UNNAMED114                                      = 114, //!< No additional details
        };

        //! \brief PWL_GAMMA_BIAS_8
        //! \details
        //!     Bias 8 for PWL for gamma correction
        enum PWL_GAMMA_BIAS_8
        {
            PWL_GAMMA_BIAS_8_UNNAMED139                                      = 139, //!< No additional details
        };

        //! \brief PWL_GAMMA_BIAS_9
        //! \details
        //!     Bias 9 for PWL for gamma correction
        enum PWL_GAMMA_BIAS_9
        {
            PWL_GAMMA_BIAS_9_UNNAMED165                                      = 165, //!< No additional details
        };

        //! \brief PWL_GAMMA_BIAS_10
        //! \details
        //!     Bias 10 for PWL for gamma correction
        enum PWL_GAMMA_BIAS_10
        {
            PWL_GAMMA_BIAS_10_UNNAMED193                                     = 193, //!< No additional details
        };

        //! \brief PWL_GAMMA_BIAS_11
        //! \details
        //!     Bias 11 for PWL for gamma correction
        enum PWL_GAMMA_BIAS_11
        {
            PWL_GAMMA_BIAS_11_UNNAMED223                                     = 223, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_POINT_1
        //! \details
        //!     Point 1 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_POINT_1
        {
            PWL_INV_GAMMA_POINT_1_UNNAMED30                                  = 30, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_POINT_2
        //! \details
        //!     Point 2 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_POINT_2
        {
            PWL_INV_GAMMA_POINT_2_UNNAMED55                                  = 55, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_POINT_3
        //! \details
        //!     Point 3 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_POINT_3
        {
            PWL_INV_GAMMA_POINT_3_UNNAMED79                                  = 79, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_POINT_4
        //! \details
        //!     Point 4 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_POINT_4
        {
            PWL_INV_GAMMA_POINT_4_UNNAMED101                                 = 101, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_POINT_5
        //! \details
        //!     Point 5 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_POINT_5
        {
            PWL_INV_GAMMA_POINT_5_UNNAMED122                                 = 122, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_POINT_6
        //! \details
        //!     Point 6 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_POINT_6
        {
            PWL_INV_GAMMA_POINT_6_UNNAMED141                                 = 141, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_POINT_7
        //! \details
        //!     Point 7 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_POINT_7
        {
            PWL_INV_GAMMA_POINT_7_UNNAMED162                                 = 162, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_POINT_8
        //! \details
        //!     Point 8 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_POINT_8
        {
            PWL_INV_GAMMA_POINT_8_UNNAMED181                                 = 181, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_POINT_9
        //! \details
        //!     Point 9 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_POINT_9
        {
            PWL_INV_GAMMA_POINT_9_UNNAMED200                                 = 200, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_POINT_10
        //! \details
        //!     Point 10 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_POINT_10
        {
            PWL_INV_GAMMA_POINT_10_UNNAMED219                                = 219, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_POINT_11
        //! \details
        //!     Point 11 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_POINT_11
        {
            PWL_INV_GAMMA_POINT_11_UNNAMED237                                = 237, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_BIAS_1
        //! \details
        //!     Bias 1 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_BIAS_1
        {
            PWL_INV_GAMMA_BIAS_1_UNNAMED3                                    = 3, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_BIAS_2
        //! \details
        //!     Bias 2 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_BIAS_2
        {
            PWL_INV_GAMMA_BIAS_2_UNNAMED10                                   = 10, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_BIAS_3
        //! \details
        //!     Bias 3 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_BIAS_3
        {
            PWL_INV_GAMMA_BIAS_3_UNNAMED20                                   = 20, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_BIAS_4
        //! \details
        //!     Bias 4 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_BIAS_4
        {
            PWL_INV_GAMMA_BIAS_4_UNNAMED33                                   = 33, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_BIAS_5
        //! \details
        //!     Bias 5 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_BIAS_5
        {
            PWL_INV_GAMMA_BIAS_5_UNNAMED49                                   = 49, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_BIAS_6
        //! \details
        //!     Bias 6 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_BIAS_6
        {
            PWL_INV_GAMMA_BIAS_6_UNNAMED67                                   = 67, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_BIAS_7
        //! \details
        //!     Bias 7 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_BIAS_7
        {
            PWL_INV_GAMMA_BIAS_7_UNNAMED92                                   = 92, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_BIAS_8
        //! \details
        //!     Bias 8 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_BIAS_8
        {
            PWL_INV_GAMMA_BIAS_8_UNNAMED117                                  = 117, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_BIAS_9
        //! \details
        //!     Bias 9 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_BIAS_9
        {
            PWL_INV_GAMMA_BIAS_9_UNNAMED147                                  = 147, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_BIAS_10
        //! \details
        //!     Bias 10 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_BIAS_10
        {
            PWL_INV_GAMMA_BIAS_10_UNNAMED180                                 = 180, //!< No additional details
        };

        //! \brief PWL_INV_GAMMA_BIAS_11
        //! \details
        //!     Bias 11 for PWL for inverse gamma correction
        enum PWL_INV_GAMMA_BIAS_11
        {
            PWL_INV_GAMMA_BIAS_11_UNNAMED215                                 = 215, //!< No additional details
        };

        //! \brief OFFSET_IN_R
        //! \details
        //!     The input offset for red component
        enum OFFSET_IN_R
        {
            OFFSET_IN_R_UNNAMED0                                             = 0, //!< No additional details
        };

        //! \brief OFFSET_IN_G
        //! \details
        //!     The input offset for green component
        enum OFFSET_IN_G
        {
            OFFSET_IN_G_UNNAMED0                                             = 0, //!< No additional details
        };

        //! \brief OFFSET_IN_B
        //! \details
        //!     The input offset for red component
        enum OFFSET_IN_B
        {
            OFFSET_IN_B_UNNAMED0                                             = 0, //!< No additional details
        };

        //! \brief D1OUT
        //! \details
        //!     OuterTriangleMappingLengthBelow
        enum D1OUT
        {
            D1OUT_UNNAMED287                                                 = 287, //!< No additional details
        };

        //! \brief DOUT_DEFAULT
        //! \details
        //!     OuterTriangleMappingLength
        enum DOUT_DEFAULT
        {
            DOUT_DEFAULT_UNNAMED164                                          = 164, //!< No additional details
        };

        //! \brief DINDEFAULT
        //! \details
        //!     InnerTriangleMappingLength
        enum DINDEFAULT
        {
            DINDEFAULT_UNNAMED205                                            = 205, //!< No additional details
        };

        enum FULLRANGEMAPPINGENABLE
        {
            FULLRANGEMAPPINGENABLE_BASICMODE                                 = 0, //!< No additional details
            FULLRANGEMAPPINGENABLE_ADVANCEMODE                               = 1, //!< No additional details
        };

        //! \brief D1IN
        //! \details
        //!     InnerTriangleMappingLengthBelow
        enum D1IN
        {
            D1IN_UNNAMED820                                                  = 820, //!< No additional details
        };

        enum COMPRESSIONLINESHIFT
        {
            COMPRESSIONLINESHIFT_UNNAMED3                                    = 3, //!< No additional details
        };

        //! \brief XVYCCDECENCENABLE
        //! \details
        //!     This bit is valid only when ColorGamutCompressionnEnable is on.
        enum XVYCCDECENCENABLE
        {
            XVYCCDECENCENABLE_TODISABLEBOTHXVYCCDECODEANDXVYCCENCODE         = 0, //!< No additional details
            XVYCCDECENCENABLE_BOTHXVYCCDECODEANDXVYCCENCODEAREENABLED        = 1, //!< No additional details
        };

        enum CPI_OVERRIDE
        {
            CPI_OVERRIDE_UNNAMED0                                            = 0, //!< No additional details
            CPI_OVERRIDE_OVERRIDECPICALCULATION                              = 1, //!< No additional details
        };

        enum LUMACHORMAONLYCORRECTION
        {
            LUMACHORMAONLYCORRECTION_LUMAONLYCORRECTION                      = 0, //!< No additional details
            LUMACHORMAONLYCORRECTION_CHORMAONLYCORRECTION                    = 1, //!< No additional details
        };

        enum GCC_BASICMODESELECTION
        {
            GCC_BASICMODESELECTION_DEFAULT                                   = 0, //!< No additional details
            GCC_BASICMODESELECTION_SCALINGFACTOR                             = 1, //!< Used along with Dword66 Bits 28:11
            GCC_BASICMODESELECTION_SINGLEAXISGAMMACORRECTION                 = 2, //!< Used along with Dword67 Bit 29
            GCC_BASICMODESELECTION_SCALINGFACTORWITHFIXEDLUMA                = 3, //!< Used along with Dword37 Bits 28:11
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VEBOX_GAMUT_STATE_CMD();

        static const size_t dwSize = 38;
        static const size_t byteSize = 152;
    };

    //!
    //! \brief VEBOX_STD_STE_STATE
    //! \details
    //!     This state structure contains the state used by the STD/STE function.
    //!
    struct VEBOX_STD_STE_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 StdEnable                                        : __CODEGEN_BITFIELD( 0,  0)    ; //!< STD Enable
                uint32_t                 SteEnable                                        : __CODEGEN_BITFIELD( 1,  1)    ; //!< STE Enable
                uint32_t                 OutputControl                                    : __CODEGEN_BITFIELD( 2,  2)    ; //!< OUTPUT_CONTROL
                uint32_t                 Reserved3                                        : __CODEGEN_BITFIELD( 3,  3)    ; //!< Reserved
                uint32_t                 SatMax                                           : __CODEGEN_BITFIELD( 4,  9)    ; //!< SAT_MAX
                uint32_t                 HueMax                                           : __CODEGEN_BITFIELD(10, 15)    ; //!< HUE_MAX
                uint32_t                 UMid                                             : __CODEGEN_BITFIELD(16, 23)    ; //!< U_MID
                uint32_t                 VMid                                             : __CODEGEN_BITFIELD(24, 31)    ; //!< V_MID
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 Sin                                              : __CODEGEN_BITFIELD( 0,  7)    ; //!< SIN
                uint32_t                 Reserved40                                       : __CODEGEN_BITFIELD( 8,  9)    ; //!< Reserved
                uint32_t                 Cos                                              : __CODEGEN_BITFIELD(10, 17)    ; //!< COS
                uint32_t                 HsMargin                                         : __CODEGEN_BITFIELD(18, 20)    ; //!< HS_MARGIN
                uint32_t                 DiamondDu                                        : __CODEGEN_BITFIELD(21, 27)    ; //!< DIAMOND_DU
                uint32_t                 DiamondMargin                                    : __CODEGEN_BITFIELD(28, 30)    ; //!< DIAMOND_MARGIN
                uint32_t                 Reserved63                                       : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 DiamondDv                                        : __CODEGEN_BITFIELD( 0,  6)    ; //!< DIAMOND_DV
                uint32_t                 DiamondTh                                        : __CODEGEN_BITFIELD( 7, 12)    ; //!< DIAMOND_TH
                uint32_t                 DiamondAlpha                                     : __CODEGEN_BITFIELD(13, 20)    ; //!< DIAMOND_ALPHA
                uint32_t                 Reserved85                                       : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 Reserved96                                       : __CODEGEN_BITFIELD( 0,  6)    ; //!< Reserved
                uint32_t                 VyStdEnable                                      : __CODEGEN_BITFIELD( 7,  7)    ; //!< VY_STD_Enable
                uint32_t                 YPoint1                                          : __CODEGEN_BITFIELD( 8, 15)    ; //!< Y_POINT_1
                uint32_t                 YPoint2                                          : __CODEGEN_BITFIELD(16, 23)    ; //!< Y_POINT_2
                uint32_t                 YPoint3                                          : __CODEGEN_BITFIELD(24, 31)    ; //!< Y_POINT_3
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 YPoint4                                          : __CODEGEN_BITFIELD( 0,  7)    ; //!< Y_POINT_4
                uint32_t                 YSlope1                                          : __CODEGEN_BITFIELD( 8, 12)    ; //!< Y_SLOPE_1
                uint32_t                 YSlope2                                          : __CODEGEN_BITFIELD(13, 17)    ; //!< Y_SLOPE_2
                uint32_t                 Reserved146                                      : __CODEGEN_BITFIELD(18, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 InvMarginVyl                                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< INV_Margin_VYL
                uint32_t                 InvSkinTypesMargin                               : __CODEGEN_BITFIELD(16, 31)    ; //!< INV_SKIN_TYPES_MARGIN
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 InvMarginVyu                                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< INV_MARGIN_VYU
                uint32_t                 P0L                                              : __CODEGEN_BITFIELD(16, 23)    ; //!< P0L
                uint32_t                 P1L                                              : __CODEGEN_BITFIELD(24, 31)    ; //!< P1L
            };
            uint32_t                     Value;
        } DW6;
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
        } DW7;
        union
        {
            struct
            {
                uint32_t                 B2L                                              : __CODEGEN_BITFIELD( 0,  7)    ; //!< B2L
                uint32_t                 B3L                                              : __CODEGEN_BITFIELD( 8, 15)    ; //!< B3L
                uint32_t                 S0L                                              : __CODEGEN_BITFIELD(16, 26)    ; //!< S0L
                uint32_t                 Reserved283                                      : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            struct
            {
                uint32_t                 S1L                                              : __CODEGEN_BITFIELD( 0, 10)    ; //!< S1L
                uint32_t                 S2L                                              : __CODEGEN_BITFIELD(11, 21)    ; //!< S2L
                uint32_t                 Reserved310                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            struct
            {
                uint32_t                 S3L                                              : __CODEGEN_BITFIELD( 0, 10)    ; //!< S3L
                uint32_t                 P0U                                              : __CODEGEN_BITFIELD(11, 18)    ; //!< P0U
                uint32_t                 P1U                                              : __CODEGEN_BITFIELD(19, 26)    ; //!< P1U
                uint32_t                 Reserved347                                      : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW10;
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
        } DW11;
        union
        {
            struct
            {
                uint32_t                 B2U                                              : __CODEGEN_BITFIELD( 0,  7)    ; //!< B2U
                uint32_t                 B3U                                              : __CODEGEN_BITFIELD( 8, 15)    ; //!< B3U
                uint32_t                 S0U                                              : __CODEGEN_BITFIELD(16, 26)    ; //!< S0U
                uint32_t                 Reserved411                                      : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            struct
            {
                uint32_t                 S1U                                              : __CODEGEN_BITFIELD( 0, 10)    ; //!< S1U
                uint32_t                 S2U                                              : __CODEGEN_BITFIELD(11, 21)    ; //!< S2U
                uint32_t                 Reserved438                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            struct
            {
                uint32_t                 S3U                                              : __CODEGEN_BITFIELD( 0, 10)    ; //!< S3U
                uint32_t                 SkinTypesEnable                                  : __CODEGEN_BITFIELD(11, 11)    ; //!< SKIN_TYPES_ENABLE
                uint32_t                 SkinTypesThresh                                  : __CODEGEN_BITFIELD(12, 19)    ; //!< SKIN_TYPES_THRESH
                uint32_t                 SkinTypesMargin                                  : __CODEGEN_BITFIELD(20, 27)    ; //!< SKIN_TYPES_MARGIN
                uint32_t                 Reserved476                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            struct
            {
                uint32_t                 Satp1                                            : __CODEGEN_BITFIELD( 0,  6)    ; //!< SATP1
                uint32_t                 Satp2                                            : __CODEGEN_BITFIELD( 7, 13)    ; //!< SATP2
                uint32_t                 Satp3                                            : __CODEGEN_BITFIELD(14, 20)    ; //!< SATP3
                uint32_t                 Satb1                                            : __CODEGEN_BITFIELD(21, 30)    ; //!< SATB1
                uint32_t                 Reserved511                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            struct
            {
                uint32_t                 Satb2                                            : __CODEGEN_BITFIELD( 0,  9)    ; //!< SATB2
                uint32_t                 Satb3                                            : __CODEGEN_BITFIELD(10, 19)    ; //!< SATB3
                uint32_t                 Sats0                                            : __CODEGEN_BITFIELD(20, 30)    ; //!< SATS0
                uint32_t                 Reserved543                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            struct
            {
                uint32_t                 Sats1                                            : __CODEGEN_BITFIELD( 0, 10)    ; //!< SATS1
                uint32_t                 Sats2                                            : __CODEGEN_BITFIELD(11, 21)    ; //!< SATS2
                uint32_t                 Reserved566                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            struct
            {
                uint32_t                 Sats3                                            : __CODEGEN_BITFIELD( 0, 10)    ; //!< SATS3
                uint32_t                 Huep1                                            : __CODEGEN_BITFIELD(11, 17)    ; //!< HUEP1
                uint32_t                 Huep2                                            : __CODEGEN_BITFIELD(18, 24)    ; //!< HUEP2
                uint32_t                 Huep3                                            : __CODEGEN_BITFIELD(25, 31)    ; //!< HUEP3
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            struct
            {
                uint32_t                 Hueb1                                            : __CODEGEN_BITFIELD( 0,  9)    ; //!< HUEB1
                uint32_t                 Hueb2                                            : __CODEGEN_BITFIELD(10, 19)    ; //!< HUEB2
                uint32_t                 Hueb3                                            : __CODEGEN_BITFIELD(20, 29)    ; //!< HUEB3
                uint32_t                 Reserved638                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            struct
            {
                uint32_t                 Hues0                                            : __CODEGEN_BITFIELD( 0, 10)    ; //!< HUES0
                uint32_t                 Hues1                                            : __CODEGEN_BITFIELD(11, 21)    ; //!< HUES1
                uint32_t                 Reserved662                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            struct
            {
                uint32_t                 Hues2                                            : __CODEGEN_BITFIELD( 0, 10)    ; //!< HUES2
                uint32_t                 Hues3                                            : __CODEGEN_BITFIELD(11, 21)    ; //!< HUES3
                uint32_t                 Reserved694                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW21;
        union
        {
            struct
            {
                uint32_t                 Satp1Dark                                        : __CODEGEN_BITFIELD( 0,  6)    ; //!< SATP1_DARK
                uint32_t                 Satp2Dark                                        : __CODEGEN_BITFIELD( 7, 13)    ; //!< SATP2_DARK
                uint32_t                 Satp3Dark                                        : __CODEGEN_BITFIELD(14, 20)    ; //!< SATP3_DARK
                uint32_t                 Satb1Dark                                        : __CODEGEN_BITFIELD(21, 30)    ; //!< SATB1_DARK
                uint32_t                 Reserved735                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW22;
        union
        {
            struct
            {
                uint32_t                 Satb2Dark                                        : __CODEGEN_BITFIELD( 0,  9)    ; //!< SATB2_DARK
                uint32_t                 Satb3Dark                                        : __CODEGEN_BITFIELD(10, 19)    ; //!< SATB3_DARK
                uint32_t                 Sats0Dark                                        : __CODEGEN_BITFIELD(20, 30)    ; //!< SATS0_DARK
                uint32_t                 Reserved767                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW23;
        union
        {
            struct
            {
                uint32_t                 Sats1Dark                                        : __CODEGEN_BITFIELD( 0, 10)    ; //!< SATS1_DARK
                uint32_t                 Sats2Dark                                        : __CODEGEN_BITFIELD(11, 21)    ; //!< SATS2_DARK
                uint32_t                 Reserved790                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW24;
        union
        {
            struct
            {
                uint32_t                 Sats3Dark                                        : __CODEGEN_BITFIELD( 0, 10)    ; //!< SATS3_DARK
                uint32_t                 Huep1Dark                                        : __CODEGEN_BITFIELD(11, 17)    ; //!< HUEP1_DARK
                uint32_t                 Huep2Dark                                        : __CODEGEN_BITFIELD(18, 24)    ; //!< HUEP2_DARK
                uint32_t                 Huep3Dark                                        : __CODEGEN_BITFIELD(25, 31)    ; //!< HUEP3_DARK
            };
            uint32_t                     Value;
        } DW25;
        union
        {
            struct
            {
                uint32_t                 Hueb1Dark                                        : __CODEGEN_BITFIELD( 0,  9)    ; //!< HUEB1_DARK
                uint32_t                 Hueb2Dark                                        : __CODEGEN_BITFIELD(10, 19)    ; //!< HUEB2_DARK
                uint32_t                 Hueb3Dark                                        : __CODEGEN_BITFIELD(20, 29)    ; //!< HUEB3_DARK
                uint32_t                 Reserved862                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW26;
        union
        {
            struct
            {
                uint32_t                 Hues0Dark                                        : __CODEGEN_BITFIELD( 0, 10)    ; //!< HUES0_DARK
                uint32_t                 Hues1Dark                                        : __CODEGEN_BITFIELD(11, 21)    ; //!< HUES1_DARK
                uint32_t                 Reserved886                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW27;
        union
        {
            struct
            {
                uint32_t                 Hues2Dark                                        : __CODEGEN_BITFIELD( 0, 10)    ; //!< HUES2_DARK
                uint32_t                 Hues3Dark                                        : __CODEGEN_BITFIELD(11, 21)    ; //!< HUES3_DARK
                uint32_t                 Reserved918                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW28;

        //! \name Local enumerations

        enum OUTPUT_CONTROL
        {
            OUTPUT_CONTROL_OUTPUTPIXELS                                      = 0, //!< No additional details
            OUTPUT_CONTROL_OUTPUTSTDDECISIONS                                = 1, //!< No additional details
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

        //! \brief SIN
        //! \details
        //!     The default is 101/128
        enum SIN
        {
            SIN_UNNAMED101                                                   = 101, //!< No additional details
        };

        //! \brief COS
        //! \details
        //!     The default is 79/128
        enum COS
        {
            COS_UNNAMED79                                                    = 79, //!< No additional details
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

        enum DIAMOND_MARGIN
        {
            DIAMOND_MARGIN_UNNAMED4                                          = 4, //!< No additional details
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

        //! \brief DIAMOND_ALPHA
        //! \details
        //!     1/tan(ß)
        //!     The default is 100/64
        enum DIAMOND_ALPHA
        {
            DIAMOND_ALPHA_UNNAMED100                                         = 100, //!< No additional details
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

        //! \brief Y_SLOPE_1
        //! \details
        //!     Slope between points Y1 and Y2.
        enum Y_SLOPE_1
        {
            Y_SLOPE_1_UNNAMED31                                              = 31, //!< No additional details
        };

        //! \brief Y_SLOPE_2
        //! \details
        //!     Slope between points Y3 and Y4.
        enum Y_SLOPE_2
        {
            Y_SLOPE_2_UNNAMED31                                              = 31, //!< No additional details
        };

        //! \brief INV_SKIN_TYPES_MARGIN
        //! \details
        //!     1/(2* Skin_types_margin)
        enum INV_SKIN_TYPES_MARGIN
        {
            INV_SKIN_TYPES_MARGIN_SKINTYPEMARGIN                             = 20, //!< No additional details
        };

        //! \brief INV_MARGIN_VYU
        //! \details
        //!     1 / Margin_VYU = 1600/65536
        enum INV_MARGIN_VYU
        {
            INV_MARGIN_VYU_UNNAMED1600                                       = 1600, //!< No additional details
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

        //! \brief S0L
        //! \details
        //!     Slope 0 of the lower part of the detection PWLF.
        enum S0L
        {
            S0L_UNNAMED2043                                                  = 2043, //!< No additional details
        };

        //! \brief S1L
        //! \details
        //!     Slope 1 of the lower part of the detection PWLF.
        enum S1L
        {
            S1L_UNNAMED0                                                     = 0, //!< No additional details
        };

        //! \brief S2L
        //! \details
        //!     The default is 0/256
        enum S2L
        {
            S2L_UNNAMED0                                                     = 0, //!< No additional details
        };

        //! \brief S3L
        //! \details
        //!     Slope 3 of the lower part of the detection PWLF.
        enum S3L
        {
            S3L_UNNAMED0                                                     = 0, //!< No additional details
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
            B3U_UNNAMED200                                                   = 200, //!< No additional details
        };

        //! \brief S0U
        //! \details
        //!     Slope 0 of the upper part of the detection PWLF.
        enum S0U
        {
            S0U_UNNAMED256                                                   = 256, //!< No additional details
        };

        //! \brief S1U
        //! \details
        //!     Slope 1 of the upper part of the detection PWLF.
        enum S1U
        {
            S1U_UNNAMED113                                                   = 113, //!< No additional details
        };

        //! \brief S2U
        //! \details
        //!     Slope 2 of the upper part of the detection PWLF.
        enum S2U
        {
            S2U_UNNAMED1869                                                  = 1869, //!< No additional details
        };

        //! \brief S3U
        //! \details
        //!     Slope 3 of the upper part of the detection PWLF.
        enum S3U
        {
            S3U_UNNAMED0                                                     = 0, //!< No additional details
        };

        //! \brief SKIN_TYPES_ENABLE
        //! \details
        //!     Treat differently bright and dark skin types
        enum SKIN_TYPES_ENABLE
        {
            SKIN_TYPES_ENABLE_DISABLE                                        = 0, //!< No additional details
        };

        //! \brief SKIN_TYPES_THRESH
        //! \details
        //!     Skin types Y margin
        //!     Restrict Skin_types_thresh >= Skin_types_margin > 0
        //!     Restrict (Skin_types_thresh + Skin_types_margin) <= 255
        enum SKIN_TYPES_THRESH
        {
            SKIN_TYPES_THRESH_UNNAMED120                                     = 120, //!< No additional details
        };

        //! \brief SKIN_TYPES_MARGIN
        //! \details
        //!     Skin types Y margin
        //!     Restrict Skin_types_thresh >= Skin_types_margin > 0
        //!     Restrict (Skin_types_thresh + Skin_types_margin) <= 255
        enum SKIN_TYPES_MARGIN
        {
            SKIN_TYPES_MARGIN_UNNAMED20                                      = 20, //!< No additional details
        };

        //! \brief SATP1
        //! \details
        //!     First point for the saturation PWLF (bright skin).
        enum SATP1
        {
            SATP1_UNNAMED6                                                   = 6, //!< No additional details
        };

        //! \brief SATP2
        //! \details
        //!     Second point for the saturation PWLF (bright skin).
        enum SATP2
        {
            SATP2_UNNAMED6                                                   = 6, //!< No additional details
        };

        //! \brief SATP3
        //! \details
        //!     Third point for the saturation PWLF (bright skin).
        enum SATP3
        {
            SATP3_UNNAMED31                                                  = 31, //!< No additional details
        };

        //! \brief SATB1
        //! \details
        //!     First bias for the saturation PWLF (bright skin).
        enum SATB1
        {
            SATB1_UNNAMED8                                                   = 8, //!< No additional details
        };

        //! \brief SATB2
        //! \details
        //!     Second bias for the saturation PWLF (bright skin)
        enum SATB2
        {
            SATB2_UNNAMED8                                                   = 8, //!< No additional details
        };

        //! \brief SATB3
        //! \details
        //!     Third bias for the saturation PWLF (bright skin)
        enum SATB3
        {
            SATB3_UNNAMED124                                                 = 124, //!< No additional details
        };

        //! \brief SATS0
        //! \details
        //!     Zeroth slope for the saturation PWLF (bright skin)
        enum SATS0
        {
            SATS0_UNNAMED297                                                 = 297, //!< No additional details
        };

        //! \brief SATS1
        //! \details
        //!     First slope for the saturation PWLF (bright skin)
        enum SATS1
        {
            SATS1_UNNAMED85                                                  = 85, //!< No additional details
        };

        //! \brief SATS2
        //! \details
        //!     Second slope for the saturation PWLF (bright skin)
        enum SATS2
        {
            SATS2_UNNAMED297                                                 = 297, //!< No additional details
        };

        //! \brief SATS3
        //! \details
        //!     Third slope for the saturation PWLF (bright skin)
        enum SATS3
        {
            SATS3_UNNAMED256                                                 = 256, //!< No additional details
        };

        //! \brief HUEP1
        //! \details
        //!     First point for the hue PWLF (bright skin)
        enum HUEP1
        {
            HUEP1_6                                                          = 122, //!< No additional details
        };

        //! \brief HUEP2
        //! \details
        //!     Second point for the hue PWLF (bright skin)
        enum HUEP2
        {
            HUEP2_UNNAMED6                                                   = 6, //!< No additional details
        };

        //! \brief HUEP3
        //! \details
        //!     Third point for the hue PWLF (bright skin)
        enum HUEP3
        {
            HUEP3_UNNAMED14                                                  = 14, //!< No additional details
        };

        //! \brief HUEB1
        //! \details
        //!     First bias for the hue PWLF (bright skin)
        enum HUEB1
        {
            HUEB1_UNNAMED8                                                   = 8, //!< No additional details
        };

        //! \brief HUEB2
        //! \details
        //!     Second bias for the hue PWLF (bright skin)
        enum HUEB2
        {
            HUEB2_UNNAMED8                                                   = 8, //!< No additional details
        };

        //! \brief HUEB3
        //! \details
        //!     Third bias for the hue PWLF (bright skin)
        enum HUEB3
        {
            HUEB3_UNNAMED56                                                  = 56, //!< No additional details
        };

        //! \brief HUES0
        //! \details
        //!     Zeroth slope for the hue PWLF (bright skin)
        enum HUES0
        {
            HUES0_UNNAMED384                                                 = 384, //!< No additional details
        };

        //! \brief HUES1
        //! \details
        //!     First slope for the hue PWLF (bright skin)
        enum HUES1
        {
            HUES1_UNNAMED85                                                  = 85, //!< No additional details
        };

        //! \brief HUES2
        //! \details
        //!     Second slope for the hue PWLF (bright skin)
        enum HUES2
        {
            HUES2_UNNAMED384                                                 = 384, //!< No additional details
        };

        //! \brief HUES3
        //! \details
        //!     Third slope for the hue PWLF (bright skin)
        enum HUES3
        {
            HUES3_UNNAMED256                                                 = 256, //!< No additional details
        };

        //! \brief SATP1_DARK
        //! \details
        //!     First point for the saturation PWLF (dark skin) Default Value: -5
        enum SATP1_DARK
        {
            SATP1_DARK_UNNAMED123                                            = 123, //!< No additional details
        };

        //! \brief SATP2_DARK
        //! \details
        //!     Second point for the saturation PWLF (dark skin)
        enum SATP2_DARK
        {
            SATP2_DARK_UNNAMED31                                             = 31, //!< No additional details
        };

        //! \brief SATP3_DARK
        //! \details
        //!     Third point for the saturation PWLF (dark skin)
        enum SATP3_DARK
        {
            SATP3_DARK_UNNAMED31                                             = 31, //!< No additional details
        };

        //! \brief SATB1_DARK
        //! \details
        //!     First bias for the saturation PWLF (dark skin)
        enum SATB1_DARK
        {
            SATB1_DARK_UNNAMED0                                              = 0, //!< No additional details
        };

        //! \brief SATB2_DARK
        //! \details
        //!     Second bias for the saturation PWLF (dark skin)
        enum SATB2_DARK
        {
            SATB2_DARK_UNNAMED124                                            = 124, //!< No additional details
        };

        //! \brief SATB3_DARK
        //! \details
        //!     Third bias for the saturation PWLF (dark skin)
        enum SATB3_DARK
        {
            SATB3_DARK_UNNAMED124                                            = 124, //!< No additional details
        };

        //! \brief SATS0_DARK
        //! \details
        //!     Zeroth slope for the saturation PWLF (dark skin)
        enum SATS0_DARK
        {
            SATS0_DARK_UNNAMED397                                            = 397, //!< No additional details
        };

        //! \brief SATS1_DARK
        //! \details
        //!     First slope for the saturation PWLF (dark skin)
        enum SATS1_DARK
        {
            SATS1_DARK_UNNAMED189                                            = 189, //!< No additional details
        };

        //! \brief SATS2_DARK
        //! \details
        //!     Second slope for the saturation PWLF (dark skin)
        enum SATS2_DARK
        {
            SATS2_DARK_UNNAMED256                                            = 256, //!< No additional details
        };

        //! \brief SATS3_DARK
        //! \details
        //!     Third slope for the saturation PWLF (dark skin)
        enum SATS3_DARK
        {
            SATS3_DARK_UNNAMED256                                            = 256, //!< No additional details
        };

        //! \brief HUEP1_DARK
        //! \details
        //!     First point for the hue PWLF (dark skin).
        enum HUEP1_DARK
        {
            HUEP1_DARK_UNNAMED0                                              = 0, //!< No additional details
        };

        //! \brief HUEP2_DARK
        //! \details
        //!     Second point for the hue PWLF (dark skin).
        enum HUEP2_DARK
        {
            HUEP2_DARK_UNNAMED2                                              = 2, //!< No additional details
        };

        //! \brief HUEP3_DARK
        //! \details
        //!     Third point for the hue PWLF (dark skin).
        enum HUEP3_DARK
        {
            HUEP3_DARK_UNNAMED14                                             = 14, //!< No additional details
        };

        //! \brief HUEB1_DARK
        //! \details
        //!     First bias for the hue PWLF (dark skin).
        enum HUEB1_DARK
        {
            HUEB1_DARK_UNNAMED0                                              = 0, //!< No additional details
        };

        //! \brief HUEB2_DARK
        //! \details
        //!     Second bias for the hue PWLF (dark skin).
        enum HUEB2_DARK
        {
            HUEB2_DARK_UNNAMED0                                              = 0, //!< No additional details
        };

        //! \brief HUEB3_DARK
        //! \details
        //!     Third bias for the hue PWLF (dark skin).
        enum HUEB3_DARK
        {
            HUEB3_DARK_UNNAMED56                                             = 56, //!< No additional details
        };

        //! \brief HUES0_DARK
        //! \details
        //!     Zeroth slope for the hue PWLF (dark skin).
        enum HUES0_DARK
        {
            HUES0_DARK_UNNAMED299                                            = 299, //!< No additional details
        };

        //! \brief HUES1_DARK
        //! \details
        //!     First slope for the hue PWLF (dark skin).
        enum HUES1_DARK
        {
            HUES1_DARK_UNNAMED256                                            = 256, //!< No additional details
        };

        //! \brief HUES2_DARK
        //! \details
        //!     Second slope for the hue PWLF (dark skin).
        enum HUES2_DARK
        {
            HUES2_DARK_UNNAMED299                                            = 299, //!< No additional details
        };

        //! \brief HUES3_DARK
        //! \details
        //!     Third slope for the hue PWLF (dark skin).
        enum HUES3_DARK
        {
            HUES3_DARK_UNNAMED256                                            = 256, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VEBOX_STD_STE_STATE_CMD();

        static const size_t dwSize = 29;
        static const size_t byteSize = 116;
    };

    //!
    //! \brief VEBOX_TCC_STATE
    //! \details
    //!     This state structure contains the IECP State Table Contents for TCC
    //!     state.
    //!
    struct VEBOX_TCC_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0,  6)    ; //!< Reserved
                uint32_t                 TccEnable                                        : __CODEGEN_BITFIELD( 7,  7)    ; //!< TCC Enable
                uint32_t                 Satfactor1                                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< SATFACTOR1
                uint32_t                 Satfactor2                                       : __CODEGEN_BITFIELD(16, 23)    ; //!< SATFACTOR2
                uint32_t                 Satfactor3                                       : __CODEGEN_BITFIELD(24, 31)    ; //!< SATFACTOR3
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Reserved
                uint32_t                 Satfactor4                                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< SATFACTOR4
                uint32_t                 Satfactor5                                       : __CODEGEN_BITFIELD(16, 23)    ; //!< SATFACTOR5
                uint32_t                 Satfactor6                                       : __CODEGEN_BITFIELD(24, 31)    ; //!< SATFACTOR6
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Basecolor1                                       : __CODEGEN_BITFIELD( 0,  9)    ; //!< BASECOLOR1
                uint32_t                 Basecolor2                                       : __CODEGEN_BITFIELD(10, 19)    ; //!< BASECOLOR2
                uint32_t                 Basecolor3                                       : __CODEGEN_BITFIELD(20, 29)    ; //!< BASECOLOR3
                uint32_t                 Reserved94                                       : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 Basecolo4                                        : __CODEGEN_BITFIELD( 0,  9)    ; //!< BASECOLO4
                uint32_t                 Basecolor5                                       : __CODEGEN_BITFIELD(10, 19)    ; //!< BASECOLOR5
                uint32_t                 Basecolor6                                       : __CODEGEN_BITFIELD(20, 29)    ; //!< BASECOLOR6
                uint32_t                 Reserved126                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Colortransitslope2                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< COLORTRANSITSLOPE2
                uint32_t                 Colortransitslope23                              : __CODEGEN_BITFIELD(16, 31)    ; //!< COLORTRANSITSLOPE23
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 Colortransitslope34                              : __CODEGEN_BITFIELD( 0, 15)    ; //!< COLORTRANSITSLOPE34
                uint32_t                 Colortransitslope45                              : __CODEGEN_BITFIELD(16, 31)    ; //!< COLORTRANSITSLOPE45
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 Colortransitslope56                              : __CODEGEN_BITFIELD( 0, 15)    ; //!< COLORTRANSITSLOPE56
                uint32_t                 Colortransitslope61                              : __CODEGEN_BITFIELD(16, 31)    ; //!< COLORTRANSITSLOPE61
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 Reserved224                                      : __CODEGEN_BITFIELD( 0,  1)    ; //!< Reserved
                uint32_t                 Colorbias1                                       : __CODEGEN_BITFIELD( 2, 11)    ; //!< COLORBIAS1
                uint32_t                 Colorbias2                                       : __CODEGEN_BITFIELD(12, 21)    ; //!< COLORBIAS2
                uint32_t                 Colorbias3                                       : __CODEGEN_BITFIELD(22, 31)    ; //!< COLORBIAS3
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 Reserved256                                      : __CODEGEN_BITFIELD( 0,  1)    ; //!< Reserved
                uint32_t                 Colorbias4                                       : __CODEGEN_BITFIELD( 2, 11)    ; //!< COLORBIAS4
                uint32_t                 Colorbias5                                       : __CODEGEN_BITFIELD(12, 21)    ; //!< COLORBIAS5
                uint32_t                 Colorbias6                                       : __CODEGEN_BITFIELD(22, 31)    ; //!< COLORBIAS6
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 SteSlopeBits                                     : __CODEGEN_BITFIELD( 0,  2)    ; //!< STE_SLOPE_BITS
                uint32_t                 Reserved291                                      : __CODEGEN_BITFIELD( 3,  7)    ; //!< Reserved
                uint32_t                 SteThreshold                                     : __CODEGEN_BITFIELD( 8, 12)    ; //!< STE_THRESHOLD
                uint32_t                 Reserved301                                      : __CODEGEN_BITFIELD(13, 15)    ; //!< Reserved
                uint32_t                 UvThresholdBits                                  : __CODEGEN_BITFIELD(16, 18)    ; //!< UV_THRESHOLD_BITS
                uint32_t                 Reserved307                                      : __CODEGEN_BITFIELD(19, 23)    ; //!< Reserved
                uint32_t                 UvThreshold                                      : __CODEGEN_BITFIELD(24, 30)    ; //!< UV_THRESHOLD
                uint32_t                 Reserved319                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 Uvmaxcolor                                       : __CODEGEN_BITFIELD( 0,  8)    ; //!< UVMAXCOLOR
                uint32_t                 Reserved329                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved
                uint32_t                 InvUvmaxcolor                                    : __CODEGEN_BITFIELD(16, 31)    ; //!< INV_UVMAXCOLOR
            };
            uint32_t                     Value;
        } DW10;

        //! \name Local enumerations

        //! \brief SATFACTOR1
        //! \details
        //!     The saturation factor for magenta.
        enum SATFACTOR1
        {
            SATFACTOR1_UNNAMED220                                            = 220, //!< No additional details
        };

        //! \brief SATFACTOR2
        //! \details
        //!     The saturation factor for red.
        enum SATFACTOR2
        {
            SATFACTOR2_UNNAMED220                                            = 220, //!< No additional details
        };

        //! \brief SATFACTOR3
        //! \details
        //!     The saturation factor for yellow.
        enum SATFACTOR3
        {
            SATFACTOR3_UNNAMED220                                            = 220, //!< No additional details
        };

        //! \brief SATFACTOR4
        //! \details
        //!     The saturation factor for green.
        enum SATFACTOR4
        {
            SATFACTOR4_UNNAMED220                                            = 220, //!< No additional details
        };

        //! \brief SATFACTOR5
        //! \details
        //!     The saturation factor for cyan.
        enum SATFACTOR5
        {
            SATFACTOR5_UNNAMED220                                            = 220, //!< No additional details
        };

        //! \brief SATFACTOR6
        //! \details
        //!     The saturation factor for blue.
        enum SATFACTOR6
        {
            SATFACTOR6_UNNAMED220                                            = 220, //!< No additional details
        };

        //! \brief BASECOLOR1
        //! \details
        //!     Base Color 1
        enum BASECOLOR1
        {
            BASECOLOR1_UNNAMED145                                            = 145, //!< No additional details
        };

        //! \brief BASECOLOR2
        //! \details
        //!     Base Color 2 - this value must be greater than BaseColor1
        enum BASECOLOR2
        {
            BASECOLOR2_UNNAMED307                                            = 307, //!< No additional details
        };

        //! \brief BASECOLOR3
        //! \details
        //!     Base Color 3 - this value must be greater than BaseColor2
        enum BASECOLOR3
        {
            BASECOLOR3_UNNAMED483                                            = 483, //!< No additional details
        };

        //! \brief BASECOLO4
        //! \details
        //!     Base Color 4 - this value must be greater than BaseColor3
        enum BASECOLO4
        {
            BASECOLO4_UNNAMED657                                             = 657, //!< No additional details
        };

        //! \brief BASECOLOR5
        //! \details
        //!     Base Color 5 - this value must be greater than BaseColor4
        enum BASECOLOR5
        {
            BASECOLOR5_UNNAMED819                                            = 819, //!< No additional details
        };

        //! \brief BASECOLOR6
        //! \details
        //!     Base Color 6 - this value must be greater than BaseColor5
        enum BASECOLOR6
        {
            BASECOLOR6_UNNAMED995                                            = 995, //!< No additional details
        };

        //! \brief COLORTRANSITSLOPE2
        //! \details
        //!     The calculation result of 1 / (BC2 - BC1) [1/57]
        enum COLORTRANSITSLOPE2
        {
            COLORTRANSITSLOPE2_UNNAMED405                                    = 405, //!< No additional details
        };

        //! \brief COLORTRANSITSLOPE23
        //! \details
        //!     The calculation result of 1 / (BC3 - BC2) [1/62]
        enum COLORTRANSITSLOPE23
        {
            COLORTRANSITSLOPE23_UNNAMED744                                   = 744, //!< No additional details
        };

        //! \brief COLORTRANSITSLOPE34
        //! \details
        //!     The calculation result of 1 / (BC4 - BC3) [1/61]
        enum COLORTRANSITSLOPE34
        {
            COLORTRANSITSLOPE34_UNNAMED1131                                  = 1131, //!< No additional details
        };

        //! \brief COLORTRANSITSLOPE45
        //! \details
        //!     The calculation result of 1 / (BC5 - BC4) [1/57]
        enum COLORTRANSITSLOPE45
        {
            COLORTRANSITSLOPE45_UNNAMED407                                   = 407, //!< No additional details
        };

        //! \brief COLORTRANSITSLOPE56
        //! \details
        //!     The calculation result of 1 / (BC6 - BC5) [1/62]
        enum COLORTRANSITSLOPE56
        {
            COLORTRANSITSLOPE56_UNNAMED372                                   = 372, //!< No additional details
        };

        //! \brief COLORTRANSITSLOPE61
        //! \details
        //!     The calculation result of 1 / (BC1 - BC6) [1/62]
        enum COLORTRANSITSLOPE61
        {
            COLORTRANSITSLOPE61_UNNAMED377                                   = 377, //!< No additional details
        };

        //! \brief COLORBIAS1
        //! \details
        //!     Color bias for BaseColor1.
        enum COLORBIAS1
        {
            COLORBIAS1_UNNAMED0                                              = 0, //!< No additional details
        };

        //! \brief COLORBIAS2
        //! \details
        //!     Color bias for BaseColor2.
        enum COLORBIAS2
        {
            COLORBIAS2_UNNAMED150                                            = 150, //!< No additional details
        };

        //! \brief COLORBIAS3
        //! \details
        //!     Color bias for BaseColor3.
        enum COLORBIAS3
        {
            COLORBIAS3_UNNAMED0                                              = 0, //!< No additional details
        };

        //! \brief COLORBIAS4
        //! \details
        //!     Color bias for BaseColor4.
        enum COLORBIAS4
        {
            COLORBIAS4_UNNAMED0                                              = 0, //!< No additional details
        };

        //! \brief COLORBIAS5
        //! \details
        //!     Color bias for BaseColor5.
        enum COLORBIAS5
        {
            COLORBIAS5_UNNAMED0                                              = 0, //!< No additional details
        };

        //! \brief COLORBIAS6
        //! \details
        //!     Color bias for BaseColor6.
        enum COLORBIAS6
        {
            COLORBIAS6_UNNAMED0                                              = 0, //!< No additional details
        };

        //! \brief STE_SLOPE_BITS
        //! \details
        //!     Skin tone pixels enhancement slope bits.
        enum STE_SLOPE_BITS
        {
            STE_SLOPE_BITS_UNNAMED0                                          = 0, //!< No additional details
        };

        //! \brief STE_THRESHOLD
        //! \details
        //!     Skin tone pixels enhancement threshold.
        enum STE_THRESHOLD
        {
            STE_THRESHOLD_UNNAMED0                                           = 0, //!< No additional details
        };

        //! \brief UV_THRESHOLD_BITS
        //! \details
        //!     Low UV transition width bits.
        enum UV_THRESHOLD_BITS
        {
            UV_THRESHOLD_BITS_UNNAMED3                                       = 3, //!< No additional details
        };

        //! \brief UV_THRESHOLD
        //! \details
        //!     Low UV threshold.
        enum UV_THRESHOLD
        {
            UV_THRESHOLD_UNNAMED3                                            = 3, //!< No additional details
        };

        //! \brief UVMAXCOLOR
        //! \details
        //!     The maximum absolute value of the legal UV pixels.  Used for the SFs2
        //!     calculation.
        enum UVMAXCOLOR
        {
            UVMAXCOLOR_UNNAMED448                                            = 448, //!< No additional details
        };

        //! \brief INV_UVMAXCOLOR
        //! \details
        //!     1 / UVMaxColor.  Used for the SFs2 calculation.
        enum INV_UVMAXCOLOR
        {
            INV_UVMAXCOLOR_UNNAMED146                                        = 146, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VEBOX_TCC_STATE_CMD();

        static const size_t dwSize = 11;
        static const size_t byteSize = 44;
    };

    //!
    //! \brief VEBOX_PROCAMP_STATE
    //! \details
    //!     This state structure contains the IECP State Table Contents for ProcAmp
    //!     state.
    //!
    struct VEBOX_PROCAMP_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 ProcampEnable                                    : __CODEGEN_BITFIELD( 0,  0)    ; //!< PROCAMP_ENABLE
                uint32_t                 Brightness                                       : __CODEGEN_BITFIELD( 1, 12)    ; //!< BRIGHTNESS
                uint32_t                 Reserved13                                       : __CODEGEN_BITFIELD(13, 16)    ; //!< Reserved
                uint32_t                 Contrast                                         : __CODEGEN_BITFIELD(17, 27)    ; //!< CONTRAST
                uint32_t                 Reserved28                                       : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 SinCS                                            : __CODEGEN_BITFIELD( 0, 15)    ; //!< SIN_C_S
                uint32_t                 CosCS                                            : __CODEGEN_BITFIELD(16, 31)    ; //!< COS_C_S
            };
            uint32_t                     Value;
        } DW1;

        //! \name Local enumerations

        enum PROCAMP_ENABLE
        {
            PROCAMP_ENABLE_UNNAMED1                                          = 1, //!< No additional details
        };

        //! \brief BRIGHTNESS
        //! \details
        //!     Brightness magnitude.
        enum BRIGHTNESS
        {
            BRIGHTNESS_OR00                                                  = 0, //!< No additional details
        };

        //! \brief CONTRAST
        //! \details
        //!     Contrast magnitude.
        enum CONTRAST
        {
            CONTRAST_10INFIXEDPOINTU47                                       = 128, //!< No additional details
        };

        //! \brief SIN_C_S
        //! \details
        //!     UV multiplication sine factor.
        enum SIN_C_S
        {
            SIN_C_S_UNNAMED0                                                 = 0, //!< No additional details
        };

        //! \brief COS_C_S
        //! \details
        //!     UV multiplication cosine factor.
        enum COS_C_S
        {
            COS_C_S_UNNAMED256                                               = 256, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VEBOX_PROCAMP_STATE_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief BLACKLEVELCORRECTIONSTATE_DW75_76
    //! \details
    //!     This state structure contains the IECP State Table Contents for the
    //!     Black Point State.
    //!
    struct BLACKLEVELCORRECTIONSTATE_DW75_76_CMD
    {
        union
        {
            struct
            {
                /// DWORD 0
                uint32_t                 BlackPointOffsetR                                : __CODEGEN_BITFIELD( 0, 12)    ; ///< S13
                uint32_t                 Reserved13                                       : __CODEGEN_BITFIELD(13, 31)    ; ///< U19
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                /// DWORD 1
                uint32_t                 BlackPointOffsetB                                : __CODEGEN_BITFIELD( 0, 12)    ; ///< S13
                uint32_t                 BlackPointOffsetG                                : __CODEGEN_BITFIELD(13, 25)    ; ///< S13
                uint32_t                 Reserved58                                       : __CODEGEN_BITFIELD(26, 31)    ; ///< U6
            };
            uint32_t                     Value;
        } DW1;

        //! \name Local enumerations

        enum BLACK_POINT_OFFSET_R
        {
            BLACK_POINT_OFFSET_R_UNNAMED0                                    = 0, ///<
        };
        enum BLACK_POINT_OFFSET_B
        {
            BLACK_POINT_OFFSET_B_UNNAMED0                                    = 0, ///<
        };
        enum BLACK_POINT_OFFSET_G
        {
            BLACK_POINT_OFFSET_G_UNNAMED0                                    = 0, ///<
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        BLACKLEVELCORRECTIONSTATE_DW75_76_CMD();

        static const size_t DW_SIZE = 2;
        static const size_t BYTE_SIZE = 8;
    };

    //!
    //! \brief VEBOX_FORWARD_GAMMA_CORRECTION_STATE
    //! \details
    //!     This state structure contains the Forward Gamma Correction state.
    //!
    struct VEBOX_FORWARD_GAMMA_CORRECTION_STATE_CMD
    {
        union
        {
            struct
            {
                /// DWORD 0
                uint32_t                 ForwardGammaCorrectionEnable                     : __CODEGEN_BITFIELD( 0,  0)    ; ///< U1
                uint32_t                 Reserved1                                        : __CODEGEN_BITFIELD( 1,  7)    ; ///< U7
                uint32_t                 PwlFwdGammaPoint1                                : __CODEGEN_BITFIELD( 8, 15)    ; ///< U8
                uint32_t                 PwlFwdGammaPoint2                                : __CODEGEN_BITFIELD(16, 23)    ; ///< U8
                uint32_t                 PwlFwdGammaPoint3                                : __CODEGEN_BITFIELD(24, 31)    ; ///< U8
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                /// DWORD 1
                uint32_t                 PwlFwdGammaPoint4                                : __CODEGEN_BITFIELD( 0,  7)    ; ///< U8
                uint32_t                 PwlFwdGammaPoint5                                : __CODEGEN_BITFIELD( 8, 15)    ; ///< U8
                uint32_t                 PwlFwdGammaPoint6                                : __CODEGEN_BITFIELD(16, 23)    ; ///< U8
                uint32_t                 PwlFwdGammaPoint7                                : __CODEGEN_BITFIELD(24, 31)    ; ///< U8
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                /// DWORD 2
                uint32_t                 PwlFwdGammaPoint8                                : __CODEGEN_BITFIELD( 0,  7)    ; ///< U8
                uint32_t                 PwlFwdGammaPoint9                                : __CODEGEN_BITFIELD( 8, 15)    ; ///< U8
                uint32_t                 PwlFwdGammaPoint10                               : __CODEGEN_BITFIELD(16, 23)    ; ///< U8
                uint32_t                 PwlFwdGammaPoint11                               : __CODEGEN_BITFIELD(24, 31)    ; ///< U8
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                /// DWORD 3
                uint32_t                 PwlFwdGammaBias1                                 : __CODEGEN_BITFIELD( 0,  7)    ; ///< U8
                uint32_t                 PwlFwdGammaBias2                                 : __CODEGEN_BITFIELD( 8, 15)    ; ///< U8
                uint32_t                 PwlFwdGammaBias3                                 : __CODEGEN_BITFIELD(16, 23)    ; ///< U8
                uint32_t                 PwlFwdGammaBias4                                 : __CODEGEN_BITFIELD(24, 31)    ; ///< U8
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                /// DWORD 4
                uint32_t                 PwlFwdGammaBias5                                 : __CODEGEN_BITFIELD( 0,  7)    ; ///< U8
                uint32_t                 PwlFwdGammaBias6                                 : __CODEGEN_BITFIELD( 8, 15)    ; ///< U8
                uint32_t                 PwlFwdGammaBias7                                 : __CODEGEN_BITFIELD(16, 23)    ; ///< U8
                uint32_t                 PwlFwdGammaBias8                                 : __CODEGEN_BITFIELD(24, 31)    ; ///< U8
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                /// DWORD 5
                uint32_t                 PwlFwdGammaBias9                                 : __CODEGEN_BITFIELD( 0,  7)    ; ///< U8
                uint32_t                 PwlFwdGammaBias10                                : __CODEGEN_BITFIELD( 8, 15)    ; ///< U8
                uint32_t                 PwlFwdGammaBias11                                : __CODEGEN_BITFIELD(16, 23)    ; ///< U8
                uint32_t                 Reserved184                                      : __CODEGEN_BITFIELD(24, 31)    ; ///< U8
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                /// DWORD 6
                uint32_t                 PwlFwdGammaSlope0                                : __CODEGEN_BITFIELD( 0, 11)    ; ///< U48
                uint32_t                 Reserved204                                      : __CODEGEN_BITFIELD(12, 15)    ; ///< U4
                uint32_t                 PwlFwdGammaSlope1                                : __CODEGEN_BITFIELD(16, 27)    ; ///< U48
                uint32_t                 Reserved220                                      : __CODEGEN_BITFIELD(28, 31)    ; ///< U4
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            struct
            {
                /// DWORD 7
                uint32_t                 PwlFwdGammaSlope2                                : __CODEGEN_BITFIELD( 0, 11)    ; ///< U48
                uint32_t                 Reserved236                                      : __CODEGEN_BITFIELD(12, 15)    ; ///< U4
                uint32_t                 PwlFwdGammaSlope3                                : __CODEGEN_BITFIELD(16, 27)    ; ///< U48
                uint32_t                 Reserved252                                      : __CODEGEN_BITFIELD(28, 31)    ; ///< U4
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            struct
            {
                /// DWORD 8
                uint32_t                 PwlFwdGammaSlope4                                : __CODEGEN_BITFIELD( 0, 11)    ; ///< U48
                uint32_t                 Reserved268                                      : __CODEGEN_BITFIELD(12, 15)    ; ///< U4
                uint32_t                 PwlFwdGammaSlope5                                : __CODEGEN_BITFIELD(16, 27)    ; ///< U48
                uint32_t                 Reserved284                                      : __CODEGEN_BITFIELD(28, 31)    ; ///< U4
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            struct
            {
                /// DWORD 9
                uint32_t                 PwlFwdGammaSlope6                                : __CODEGEN_BITFIELD( 0, 11)    ; ///< U48
                uint32_t                 Reserved300                                      : __CODEGEN_BITFIELD(12, 15)    ; ///< U4
                uint32_t                 PwlFwdGammaSlope7                                : __CODEGEN_BITFIELD(16, 27)    ; ///< U48
                uint32_t                 Reserved316                                      : __CODEGEN_BITFIELD(28, 31)    ; ///< U4
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            struct
            {
                /// DWORD 10
                uint32_t                 PwlFwdGammaSlope8                                : __CODEGEN_BITFIELD( 0, 11)    ; ///< U48
                uint32_t                 Reserved332                                      : __CODEGEN_BITFIELD(12, 15)    ; ///< U4
                uint32_t                 PwlFwdGammaSlope9                                : __CODEGEN_BITFIELD(16, 27)    ; ///< U48
                uint32_t                 Reserved348                                      : __CODEGEN_BITFIELD(28, 31)    ; ///< U4
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            struct
            {
                /// DWORD 11
                uint32_t                 PwlFwdGammaSlope10                               : __CODEGEN_BITFIELD( 0, 11)    ; ///< U48
                uint32_t                 Reserved364                                      : __CODEGEN_BITFIELD(12, 15)    ; ///< U4
                uint32_t                 PwlFwdGammaSlope11                               : __CODEGEN_BITFIELD(16, 27)    ; ///< U48
                uint32_t                 Reserved380                                      : __CODEGEN_BITFIELD(28, 31)    ; ///< U4
            };
            uint32_t                     Value;
        } DW11;

        //! \name Local enumerations

        enum PWL_FWD_GAMMA_POINT_1
        {
            PWL_FWD_GAMMA_POINT_1_UNNAMED30                                  = 30, ///<
        };
        enum PWL_FWD_GAMMA_POINT_2
        {
            PWL_FWD_GAMMA_POINT_2_UNNAMED55                                  = 55, ///<
        };
        enum PWL_FWD_GAMMA_POINT_3
        {
            PWL_FWD_GAMMA_POINT_3_UNNAMED79                                  = 79, ///<
        };
        enum PWL_FWD_GAMMA_POINT_4
        {
            PWL_FWD_GAMMA_POINT_4_UNNAMED101                                 = 101, ///<
        };
        enum PWL_FWD_GAMMA_POINT_5
        {
            PWL_FWD_GAMMA_POINT_5_UNNAMED122                                 = 122, ///<
        };
        enum PWL_FWD_GAMMA_POINT_6
        {
            PWL_FWD_GAMMA_POINT_6_UNNAMED141                                 = 141, ///<
        };
        enum PWL_FWD_GAMMA_POINT_7
        {
            PWL_FWD_GAMMA_POINT_7_UNNAMED162                                 = 162, ///<
        };
        enum PWL_FWD_GAMMA_POINT_8
        {
            PWL_FWD_GAMMA_POINT_8_UNNAMED181                                 = 181, ///<
        };
        enum PWL_FWD_GAMMA_POINT_9
        {
            PWL_FWD_GAMMA_POINT_9_UNNAMED200                                 = 200, ///<
        };
        enum PWL_FWD_GAMMA_POINT_10
        {
            PWL_FWD_GAMMA_POINT_10_UNNAMED219                                = 219, ///<
        };
        enum PWL_FWD_GAMMA_POINT_11
        {
            PWL_FWD_GAMMA_POINT_11_UNNAMED237                                = 237, ///<
        };
        enum PWL_FWD_GAMMA_BIAS_1
        {
            PWL_FWD_GAMMA_BIAS_1_UNNAMED3                                    = 3, ///<
        };
        enum PWL_FWD_GAMMA_BIAS_2
        {
            PWL_FWD_GAMMA_BIAS_2_UNNAMED10                                   = 10, ///<
        };
        enum PWL_FWD_GAMMA_BIAS_3
        {
            PWL_FWD_GAMMA_BIAS_3_UNNAMED20                                   = 20, ///<
        };
        enum PWL_FWD_GAMMA_BIAS_4
        {
            PWL_FWD_GAMMA_BIAS_4_UNNAMED33                                   = 33, ///<
        };
        enum PWL_FWD_GAMMA_BIAS_5
        {
            PWL_FWD_GAMMA_BIAS_5_UNNAMED49                                   = 49, ///<
        };
        enum PWL_FWD_GAMMA_BIAS_6
        {
            PWL_FWD_GAMMA_BIAS_6_UNNAMED67                                   = 67, ///<
        };
        enum PWL_FWD_GAMMA_BIAS_7
        {
            PWL_FWD_GAMMA_BIAS_7_UNNAMED92                                   = 92, ///<
        };
        enum PWL_FWD_GAMMA_BIAS_8
        {
            PWL_FWD_GAMMA_BIAS_8_UNNAMED117                                  = 117, ///<
        };
        enum PWL_FWD_GAMMA_BIAS_9
        {
            PWL_FWD_GAMMA_BIAS_9_UNNAMED147                                  = 147, ///<
        };
        enum PWL_FWD_GAMMA_BIAS_10
        {
            PWL_FWD_GAMMA_BIAS_10_UNNAMED180                                 = 180, ///<
        };
        enum PWL_FWD_GAMMA_BIAS_11
        {
            PWL_FWD_GAMMA_BIAS_11_UNNAMED215                                 = 215, ///<
        };
        enum PWL_FWD_GAMMA_SLOPE_0
        {
            PWL_FWD_GAMMA_SLOPE_0_26256                                      = 26, ///<
        };
        enum PWL_FWD_GAMMA_SLOPE_1
        {
            PWL_FWD_GAMMA_SLOPE_1_72256                                      = 72, ///<
        };
        enum PWL_FWD_GAMMA_SLOPE_2
        {
            PWL_FWD_GAMMA_SLOPE_2_107256                                     = 107, ///<
        };
        enum PWL_FWD_GAMMA_SLOPE_3
        {
            PWL_FWD_GAMMA_SLOPE_3_151256                                     = 151, ///<
        };
        enum PWL_FWD_GAMMA_SLOPE_4
        {
            PWL_FWD_GAMMA_SLOPE_4_195256                                     = 195, ///<
        };
        enum PWL_FWD_GAMMA_SLOPE_5
        {
            PWL_FWD_GAMMA_SLOPE_5_243256                                     = 243, ///<
        };
        enum PWL_FWD_GAMMA_SLOPE_6
        {
            PWL_FWD_GAMMA_SLOPE_6_305256                                     = 305, ///<
        };
        enum PWL_FWD_GAMMA_SLOPE_7
        {
            PWL_FWD_GAMMA_SLOPE_7_337256                                     = 337, ///<
        };
        enum PWL_FWD_GAMMA_SLOPE_8
        {
            PWL_FWD_GAMMA_SLOPE_8_404256                                     = 404, ///<
        };
        enum PWL_FWD_GAMMA_SLOPE_9
        {
            PWL_FWD_GAMMA_SLOPE_9_445256                                     = 445, ///<
        };
        enum PWL_FWD_GAMMA_SLOPE_10
        {
            PWL_FWD_GAMMA_SLOPE_10_498256                                    = 498, ///<
        };
        enum PWL_FWD_GAMMA_SLOPE_11
        {
            PWL_FWD_GAMMA_SLOPE_11_555256                                    = 555, ///<
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VEBOX_FORWARD_GAMMA_CORRECTION_STATE_CMD();

        static const size_t DW_SIZE = 12;
        static const size_t BYTE_SIZE = 48;
    };

    //!
    //! \brief VEBOX_IECP_STATE
    //! \details
    //! 
    //!
    struct VEBOX_IECP_STATE_CMD
    {
        VEBOX_STD_STE_STATE_CMD      StdSteState                                                                      ; ///< VEBOX_STD_STE_STATE
        VEBOX_ACE_LACE_STATE_CMD     AceState                                                                         ; ///< VEBOX_ACE_LACE_STATE
        VEBOX_TCC_STATE_CMD          TccState                                                                         ; ///< VEBOX_TCC_STATE
        VEBOX_PROCAMP_STATE_CMD      ProcampState                                                                     ; ///< VEBOX_PROCAMP_STATE
        VEBOX_CSC_STATE_CMD          CscState                                                                         ; ///< VEBOX_CSC_STATE
        VEBOX_ALPHA_AOI_STATE_CMD    AlphaAoiState                                                                    ; ///< VEBOX_ALPHA_AOI_STATE
        VEBOX_CCM_STATE_CMD          CcmState                                                                         ; ///< VEBOX_CCM_STATE
        BLACKLEVELCORRECTIONSTATE_DW75_76_CMD BlackLevelCorrectionState                                               ; ///< Black Level Correction State - DW75..76
        VEBOX_FORWARD_GAMMA_CORRECTION_STATE_CMD ForwardGammaCorrection                                               ; ///< VEBOX_FORWARD_GAMMA_CORRECTION_STATE
        VEBOX_FRONT_END_CSC_STATE_CMD FrontEndCsc                                                                     ; ///< VEBOX_FRONT_END_CSC_STATE

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VEBOX_IECP_STATE_CMD();

        static const size_t DW_SIZE = 97;
        static const size_t BYTE_SIZE = 388;
    };

    //!
    //! \brief VEBOX_STATE
    //! \details
    //!     This command controls the internal functions of the VEBOX. This command
    //!     has a set of indirect state buffers:  DN/DI state
    //!      IECP general state
    //!      IECP Gamut Expansion/Compression state
    //!      IECP Gamut Vertex Table state
    //!      Capture Pipe state
    //! 
    //!
    struct VEBOX_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 CommandOpcode                                    : __CODEGEN_BITFIELD(24, 26)    ; //!< COMMAND_OPCODE
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
                uint32_t                 ColorGamutExpansionEnable                        : __CODEGEN_BITFIELD( 0,  0)    ; //!< Color Gamut Expansion Enable
                uint32_t                 ColorGamutCompressionEnable                      : __CODEGEN_BITFIELD( 1,  1)    ; //!< Color Gamut Compression Enable
                uint32_t                 GlobalIecpEnable                                 : __CODEGEN_BITFIELD( 2,  2)    ; //!< Global IECP Enable
                uint32_t                 DnEnable                                         : __CODEGEN_BITFIELD( 3,  3)    ; //!< DN_ENABLE
                uint32_t                 DiEnable                                         : __CODEGEN_BITFIELD( 4,  4)    ; //!< DI_ENABLE
                uint32_t                 DnDiFirstFrame                                   : __CODEGEN_BITFIELD( 5,  5)    ; //!< DNDI_FIRST_FRAME
                uint32_t                 DownsampleMethod422to420                         : __CODEGEN_BITFIELD( 6,  6)    ; //!< _422__420_DOWNSAMPLE_METHOD
                uint32_t                 DownsampleMethod444to422                         : __CODEGEN_BITFIELD( 7,  7)    ; //!< _444__422_DOWNSAMPLE_METHOD
                uint32_t                 DiOutputFrames                                   : __CODEGEN_BITFIELD( 8,  9)    ; //!< DI_OUTPUT_FRAMES
                uint32_t                 DemosaicEnable                                   : __CODEGEN_BITFIELD(10, 10)    ; //!< Demosaic Enable
                uint32_t                 VignetteEnable                                   : __CODEGEN_BITFIELD(11, 11)    ; //!< Vignette Enable
                uint32_t                 AlphaPlaneEnable                                 : __CODEGEN_BITFIELD(12, 12)    ; //!< Alpha Plane Enable
                uint32_t                 HotPixelFilteringEnable                          : __CODEGEN_BITFIELD(13, 13)    ; //!< Hot Pixel Filtering Enable
                uint32_t                 SingleSliceVeboxEnable                           : __CODEGEN_BITFIELD(14, 14)    ; //!< Single Slice VEBOX Enable
                uint32_t                 Reserved47                                       : __CODEGEN_BITFIELD(15, 24)    ; //!< Reserved
                uint32_t                 StateSurfaceControlBits                          : __CODEGEN_BITFIELD(25, 31)    ; //!< State Surface Control Bits
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Reserved64                                       : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 DnDiStatePointerLow                              : __CODEGEN_BITFIELD(12, 31)    ; //!< DN/DI State Pointer Low
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 DnDiStatePointerHigh                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< DN/DI State Pointer High
                uint32_t                 Reserved112                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Reserved128                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 IecpStatePointerLow                              : __CODEGEN_BITFIELD(12, 31)    ; //!< IECP State Pointer Low
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 IecpStatePointerHigh                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< IECP State Pointer High
                uint32_t                 Reserved176                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 Reserved192                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 GamutStatePointerLow                             : __CODEGEN_BITFIELD(12, 31)    ; //!< Gamut State Pointer Low
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 GamutStatePointerHigh                            : __CODEGEN_BITFIELD( 0, 15)    ; //!< Gamut State Pointer High
                uint32_t                 Reserved240                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 Reserved256                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 VertexTableStatePointerLow                       : __CODEGEN_BITFIELD(12, 31)    ; //!< Vertex Table State Pointer Low
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 VertexTableStatePointerHigh                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Vertex Table State Pointer High
                uint32_t                 Reserved304                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 Reserved320                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 CapturePipeStatePointerLow                       : __CODEGEN_BITFIELD(12, 31)    ; //!< Capture Pipe State Pointer Low
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 CapturePipeStatePointerHigh                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Capture Pipe State Pointer High
                uint32_t                 Reserved368                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW11;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_UNNAMED2                                             = 2, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_UNNAMED0                                             = 0, //!< No additional details
        };

        enum COMMAND_OPCODE
        {
            COMMAND_OPCODE_VEBOX                                             = 4, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MEDIA                                                   = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief DN_ENABLE
        //! \details
        //!     Denoise is bypassed if this is low - BNE is still calculated and output,
        //!     but the denoised fields are not. VDI does not read in the denoised
        //!     previous frame but uses the pointer for the original previous frame.
        enum DN_ENABLE
        {
            DN_ENABLE_DONOTDENOISEFRAME                                      = 0, //!< No additional details
            DN_ENABLE_DENOISEFRAME                                           = 1, //!< No additional details
        };

        //! \brief DI_ENABLE
        //! \details
        //!     Deinterlacer is bypassed if this is disabled:  the output is the same as
        //!     the input (same as a 2:2 cadence). 
        //!                         FMD and STMM are not calculated and the values in the response
        //!     message are 0.
        enum DI_ENABLE
        {
            DI_ENABLE_DONOTCALCULATEDI                                       = 0, //!< No additional details
            DI_ENABLE_CALCULATEDI                                            = 1, //!< No additional details
        };

        //! \brief DNDI_FIRST_FRAME
        //! \details
        //!     Indicates that this is the first frame of the stream, so previous clean
        //!     is not available.
        enum DNDI_FIRST_FRAME
        {
            DNDI_FIRST_FRAME_NOTFIRSTFIELD_PREVIOUSCLEANSURFACESTATEISVALID  = 0, //!< No additional details
            DNDI_FIRST_FRAME_FIRSTFIELD_PREVIOUSCLEANSURFACESTATEISINVALID   = 1, //!< No additional details
        };

        //! \brief _422__420_DOWNSAMPLE_METHOD
        //! \details
        //!     To enable averaging in case of 420 (NV12/P016) output formats,
        //!     444-&gt;422 and 422-&gt;420 should be set.
        enum _422__420_DOWNSAMPLE_METHOD
        {
            _422_420_DOWNSAMPLE_METHOD_DROPLOWERCHROMAOFTHEPAIR              = 0, //!< No additional details
            _422_420_DOWNSAMPLE_METHOD_AVERAGEVERTICALLYALIGNEDCHROMAS       = 1, //!< No additional details
        };

        //! \brief _444__422_DOWNSAMPLE_METHOD
        //! \details
        //!     <table border="1">
        //!                             <tr>
        //!                                 <td>444-&gt;422</td>
        //!                                 <td>422-&gt;420</td>
        //!                                 <td>Description</td>
        //!                             </tr>
        //!                             <tr>
        //!                                 <td>0</td>
        //!                                 <td>0</td>
        //!                                 <td>No averaging, only down sampling</td>
        //!                             </tr>
        //!                             <tr>
        //!                                 <td>0</td>
        //!                                 <td>1</td>
        //!                                 <td>Not Supported</td>
        //!                             </tr>
        //!                             <tr>
        //!                                 <td>1</td>
        //!                                 <td>0</td>
        //!                                 <td>Only Horizontal averaging</td>
        //!                             </tr>
        //!                             <tr>
        //!                                 <td>1</td>
        //!                                 <td>1</td>
        //!                                 <td>Horizontal and Vertical averaging</td>
        //!                             </tr>
        //!                         </table>
        enum _444__422_DOWNSAMPLE_METHOD
        {
            _444_422_DOWNSAMPLE_METHOD_DROPRIGHTCHROMAOFTHEPAIR              = 0, //!< No additional details
            _444_422_DOWNSAMPLE_METHOD_AVERAGEHORIZONTALLYALIGNEDCHROMAS     = 1, //!< No additional details
        };

        //! \brief DI_OUTPUT_FRAMES
        //! \details
        //!     Indicates which frames to output in DI mode.
        enum DI_OUTPUT_FRAMES
        {
            DI_OUTPUT_FRAMES_OUTPUTBOTHFRAMES                                = 0, //!< No additional details
            DI_OUTPUT_FRAMES_OUTPUTPREVIOUSFRAMEONLY                         = 1, //!< No additional details
            DI_OUTPUT_FRAMES_OUTPUTCURRENTFRAMEONLY                          = 2, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VEBOX_STATE_CMD();

        static const size_t dwSize = 12;
        static const size_t byteSize = 48;
    };

    //!
    //! \brief VEBOX_SURFACE_STATE
    //! \details
    //!     The input and output data containers accessed are called "surfaces".
    //!     Surface state is sent to VEBOX via an inline state command rather than
    //!     using binding tables. SURFACE_STATE contains the parameters defining
    //!     each surface to be accessed, including its size, format, and offsets to
    //!     its subsurfaces. The surface's base address is in the execution command.
    //!     Despite having multiple input and output surfaces, we limit the number
    //!     of surface states to one for input surfaces and one for output surfaces.
    //!     The other surfaces are derived from the input/output surface states.
    //! 
    //!     The Current Frame Input surface uses the Input SURFACE_STATE
    //! 
    //!     The Previous Denoised Input surface uses the Input SURFACE_STATE. (For
    //!     12-bit Bayer pattern inputs this will be 8-bit.)
    //! 
    //!     The Current Denoised Output surface uses the Input SURFACE_STATE. (For
    //!     12-bit Bayer pattern inputs this will be 8-bit.)
    //! 
    //!     The STMM/Noise History Input surface uses the Input SURFACE_STATE with
    //!     Tile-Y and Width/Height a multiple of 4.
    //! 
    //!     The STMM/Noise History Output surface uses the Input SURFACE_STATE with
    //!     Tile-Y and Width/Height a multiple of 4.
    //! 
    //!     The Current Deinterlaced/IECP Frame Output surface uses the Output
    //!     SURFACE_STATE.
    //! 
    //!     The Previous Deinterlaced/IECP Frame Output surface uses the Output
    //!     SURFACE_STATE.
    //! 
    //!     The FMD per block output / per Frame Output surface uses the Linear
    //!     SURFACE_STATE (see note below).
    //! 
    //!     The Alpha surface uses the Linear A8 SURFACE_STATE with Width/Height
    //!     equal to Input Surface. Pitch is width rounded to next 64.
    //! 
    //!     The Vignette Correction surface uses the Linear 16-bit SURFACE_STATE
    //!     with:
    //!     Width = 4 * ((Input Width 3)/4)
    //!     Height = ((Input Height 3)/4)
    //!     Pitch in bytes is (vignette width*2) rounded to next 64.
    //! 
    //!     The STMM height is the same as the Input Surface height except when the
    //!     input Surface Format is Bayer Pattern and the Bayer Pattern Offset is 10
    //!     or 11, in  which case the height is the input height + 4. For Bayer
    //!     pattern inputs when the Bayer Pattern Offset is 10 or 11, the Current
    //!     Denoised Output/Previous Denoised Input will also have a height which is
    //!     the input height + 4. For Bayer pattern inputs only the Current Denoised
    //!     Output/Previous Denoised Input are in Tile-Y.
    //! 
    //!     The linear surface for FMD statistics is linear (not tiled). The height
    //!     of the per block statistics is (Input Height +3)/4 - the Input Surface
    //!     height in pixels is rounded up to the next even 4 and divided by 4. The
    //!     width of the per block section in bytes is equal to the width of the
    //!     Input Surface in pixels rounded up to the next 16 bytes. The pitch of
    //!     the per block section in bytes is equal to the width of the Input
    //!     Surface in pixels rounded up to the next 64 bytes.
    //! 
    //!     The STMM surfaces must be identical to the Input surface except for the
    //!     tiling mode must be Tile-Y and the pitch must be legal for Tile-Y
    //!     (increased to the next larger legal pitch). If the input surface is
    //!     packed (Surface Format from 0 to 3 for DN/DI) or 12/10-bit Bayer Pattern
    //!     then the pitch for the STMM surface is 1/2 the pitch of the input
    //!     surface (rounded up to the next larger legal Tile-Y pitch). The width
    //!     and height must be a multiple of 4 rounded up from the input height.
    //! 
    //!     VEBOX may write to memory between the surface width and the surface
    //!     pitch for output surfaces.
    //! 
    //!     For 8bit Alpha input, when converting to 16bit output it is padded with
    //!     8bit zeros in the LSB.
    //!
    struct VEBOX_SURFACE_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 SubopcodeB                                       : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPCODE_B
                uint32_t                 SubopcodeA                                       : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPCODE_A
                uint32_t                 MediaCommandOpcode                               : __CODEGEN_BITFIELD(24, 26)    ; //!< MEDIA_COMMAND_OPCODE
                uint32_t                 MediaCommandPipeline                             : __CODEGEN_BITFIELD(27, 28)    ; //!< MEDIA_COMMAND_PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 SurfaceIdentification                            : __CODEGEN_BITFIELD( 0,  0)    ; //!< SURFACE_IDENTIFICATION
                uint32_t                 Reserved33                                       : __CODEGEN_BITFIELD( 1, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Reserved64                                       : __CODEGEN_BITFIELD( 0,  3)    ; //!< Reserved
                uint32_t                 Width                                            : __CODEGEN_BITFIELD( 4, 17)    ; //!< Width
                uint32_t                 Height                                           : __CODEGEN_BITFIELD(18, 31)    ; //!< Height
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 TileWalk                                         : __CODEGEN_BITFIELD( 0,  0)    ; //!< TILE_WALK
                uint32_t                 TiledSurface                                     : __CODEGEN_BITFIELD( 1,  1)    ; //!< TILED_SURFACE
                uint32_t                 HalfPitchForChroma                               : __CODEGEN_BITFIELD( 2,  2)    ; //!< Half Pitch for Chroma
                uint32_t                 SurfacePitch                                     : __CODEGEN_BITFIELD( 3, 19)    ; //!< Surface Pitch
                uint32_t                 Reserved116                                      : __CODEGEN_BITFIELD(20, 23)    ; //!< Reserved
                uint32_t                 BayerPatternFormat                               : __CODEGEN_BITFIELD(24, 24)    ; //!< BAYER_PATTERN_FORMAT
                uint32_t                 BayerPatternOffset                               : __CODEGEN_BITFIELD(25, 26)    ; //!< BAYER_PATTERN_OFFSET
                uint32_t                 InterleaveChroma                                 : __CODEGEN_BITFIELD(27, 27)    ; //!< Interleave Chroma
                uint32_t                 SurfaceFormat                                    : __CODEGEN_BITFIELD(28, 31)    ; //!< SURFACE_FORMAT
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 YOffsetForU                                      : __CODEGEN_BITFIELD( 0, 14)    ; //!< Y Offset for U
                uint32_t                 Reserved143                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 XOffsetForU                                      : __CODEGEN_BITFIELD(16, 28)    ; //!< X Offset for U
                uint32_t                 Reserved157                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 YOffsetForV                                      : __CODEGEN_BITFIELD( 0, 14)    ; //!< Y Offset for V
                uint32_t                 Reserved175                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 XOffsetForV                                      : __CODEGEN_BITFIELD(16, 28)    ; //!< X Offset for V
                uint32_t                 Reserved189                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 Dword67                                                                          ; //!< DWORD6..7
            };
            uint32_t                     Value;
        } DW6;

        //! \name Local enumerations

        enum SUBOPCODE_B
        {
            SUBOPCODE_B_VEBOX                                                = 0, //!< No additional details
        };

        enum SUBOPCODE_A
        {
            SUBOPCODE_A_VEBOX                                                = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_VEBOX                                       = 4, //!< No additional details
        };

        enum MEDIA_COMMAND_PIPELINE
        {
            MEDIA_COMMAND_PIPELINE_MEDIA                                     = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief SURFACE_IDENTIFICATION
        //! \details
        //!     Specifies which set of surfaces this command refers to:
        enum SURFACE_IDENTIFICATION
        {
            SURFACE_IDENTIFICATION_INPUTSURFACEANDDENOISEDCURRENTOUTPUTSURFACE = 0, //!< No additional details
            SURFACE_IDENTIFICATION_OUTPUTSURFACE_ALLEXCEPTTHEDENOISEDCURRENTOUTPUTSURFACE = 1, //!< No additional details
        };

        //! \brief TILE_WALK
        //! \details
        //!     This field specifies the type of memory tiling (XMajor or YMajor)
        //!     employed to tile this surface. See <em>Memory Interface Functions</em>
        //!     for details on memory tiling and restrictions.
        //!                         This field is ignored when the surface is linear.
        enum TILE_WALK
        {
            TILE_WALK_TILEWALKXMAJOR                                         = 0, //!< No additional details
            TILE_WALK_TILEWALKYMAJOR                                         = 1, //!< No additional details
        };

        //! \brief TILED_SURFACE
        //! \details
        //!     This field specifies whether the surface is tiled.
        enum TILED_SURFACE
        {
            TILED_SURFACE_FALSE                                              = 0, //!< Linear
            TILED_SURFACE_TRUE                                               = 1, //!< Tiled
        };

        //! \brief BAYER_PATTERN_FORMAT
        //! \details
        //!     Specifies the format of the Bayer Pattern:
        enum BAYER_PATTERN_FORMAT
        {
            BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE                    = 0, //!< No additional details
            BAYER_PATTERN_FORMAT_12OR10_BITINPUTATA16_BITSTRIDEVALIDDATAISINTHEMSBS = 1, //!< No additional details
        };

        //! \brief BAYER_PATTERN_OFFSET
        //! \details
        //!     Specifies the starting pixel offset for the Bayer pattern used for
        //!     Capture Pipe.
        enum BAYER_PATTERN_OFFSET
        {
            BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISBLUE                          = 0, //!< No additional details
            BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISRED                           = 1, //!< No additional details
            BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISRED       = 2, //!< No additional details
            BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISBLUE      = 3, //!< No additional details
        };

        //! \brief SURFACE_FORMAT
        //! \details
        //!     Specifies the format of the surface.  All of the Y and G channels will
        //!     use table 0 and all of the Cr/Cb/R/B channels will use table 1.
        enum SURFACE_FORMAT
        {
            SURFACE_FORMAT_YCRCBNORMAL                                       = 0, //!< No additional details
            SURFACE_FORMAT_YCRCBSWAPUVY                                      = 1, //!< No additional details
            SURFACE_FORMAT_YCRCBSWAPUV                                       = 2, //!< No additional details
            SURFACE_FORMAT_YCRCBSWAPY                                        = 3, //!< No additional details
            SURFACE_FORMAT_PLANAR4208                                        = 4, //!< NV12 with Interleave Chroma set
            SURFACE_FORMAT_PACKED444A8                                       = 5, //!< IECP input/output only
            SURFACE_FORMAT_PACKED42216                                       = 6, //!< IECP input/output only
            SURFACE_FORMAT_R10G10B10A2UNORMR10G10B10A2UNORMSRGB              = 7, //!< IECP output only
            SURFACE_FORMAT_R8G8B8A8UNORMR8G8B8A8UNORMSRGB                    = 8, //!< IECP input/output only
            SURFACE_FORMAT_PACKED44416                                       = 9, //!< IECP input/output only
            SURFACE_FORMAT_PLANAR42216                                       = 10, //!< IECP input/output only
            SURFACE_FORMAT_Y8UNORM                                           = 11, //!< No additional details
            SURFACE_FORMAT_PLANAR42016                                       = 12, //!< IECP input/output only
            SURFACE_FORMAT_R16G16B16A16                                      = 13, //!< IECP input/output only
            SURFACE_FORMAT_BAYERPATTERN                                      = 14, //!< Demosaic input only
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VEBOX_SURFACE_STATE_CMD();

        static const size_t dwSize = 7;
        static const size_t byteSize = 28;
    };

    //!
    //! \brief VEB_DI_IECP
    //! \details
    //!     The VEB_DI_IECP command causes the VEBOX to start processing the frames
    //!     specified by VEB_SURFACE_STATE using the parameters specified by
    //!     VEB_DI_STATE and VEB_IECP_STATE.  The processing can start and end at
    //!     any 64 pixel column in the frame. If Starting X and Ending X are used to
    //!     split the frame into sections, it should not be split into more than 4
    //!     sections.
    //!     Each VEB_DI_IECP command should be preceded by a VEB_STATE command and
    //!     the input/output VEB_SURFACE_STATE commands.
    //!
    struct VEB_DI_IECP_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopb                                           : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPB
                uint32_t                 Subopa                                           : __CODEGEN_BITFIELD(21, 23)    ; //!< SUBOPA
                uint32_t                 Opcode                                           : __CODEGEN_BITFIELD(24, 26)    ; //!< OPCODE
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
                uint32_t                 EndingX                                          : __CODEGEN_BITFIELD( 0, 13)    ; //!< Ending X
                uint32_t                 Reserved46                                       : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 StartingX                                        : __CODEGEN_BITFIELD(16, 29)    ; //!< Starting X
                uint32_t                 Reserved62                                       : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 CurrentFrameSurfaceControlBitsAgeForQuadlruAge   : __CODEGEN_BITFIELD( 0,  1)    ; ///< Sub-structure
                uint32_t                 CurrentFrameSurfaceControlBitsReserved2          : __CODEGEN_BITFIELD( 2,  2)    ; ///< Sub-structure
                uint32_t                 CurrentFrameSurfaceControlBitsTargetCacheTc      : __CODEGEN_BITFIELD( 3,  4)    ; ///< Sub-structure
                uint32_t                 CurrentFrameSurfaceControlBitsMemoryTypeLlcEllcCacheabilityControlLellccc : __CODEGEN_BITFIELD( 5,  6)    ; ///< Sub-structure
                uint32_t                 CurrentFrameSurfaceControlBitsReserved           : __CODEGEN_BITFIELD( 7,  8)    ; ///< Sub-structure
                uint32_t                 Reserved73                                       : __CODEGEN_BITFIELD( 9, 11)    ; ///< U2
                uint32_t                 CurrentFrameInputAddress                         : __CODEGEN_BITFIELD(12, 31)    ; ///< GraphicsAddress
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 CurrentFrameInputAddressHigh                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< Current Frame Input Address High
                uint32_t                 Reserved112                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 PreviousFrameSurfaceControlBitsAgeForQuadlruAge  : __CODEGEN_BITFIELD( 0,  1)    ; ///< Sub-structure
                uint32_t                 PreviousFrameSurfaceControlBitsReserved2         : __CODEGEN_BITFIELD( 2,  2)    ; ///< Sub-structure
                uint32_t                 PreviousFrameSurfaceControlBitsTargetCacheTc     : __CODEGEN_BITFIELD( 3,  4)    ; ///< Sub-structure
                uint32_t                 PreviousFrameSurfaceControlBitsMemoryTypeLlcEllcCacheabilityControlLellccc : __CODEGEN_BITFIELD( 5,  6)    ; ///< Sub-structure
                uint32_t                 PreviousFrameSurfaceControlBitsReserved          : __CODEGEN_BITFIELD( 7,  8)    ; ///< Sub-structure
                uint32_t                 Reserved137                                      : __CODEGEN_BITFIELD( 9, 11)    ; ///< U2
                uint32_t                 PreviousFrameInputAddress                        : __CODEGEN_BITFIELD(12, 31)    ; ///< GraphicsAddress
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 PreviousFrameInputAddressHigh                    : __CODEGEN_BITFIELD( 0, 15)    ; ///< GraphicsAddress
                uint32_t                 Reserved176                                      : __CODEGEN_BITFIELD(16, 31)    ; ///< U14
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 StmmInputSurfaceControlBitsAgeForQuadlruAge      : __CODEGEN_BITFIELD( 0,  1)    ; ///< Sub-structure
                uint32_t                 StmmInputSurfaceControlBitsReserved2             : __CODEGEN_BITFIELD( 2,  2)    ; ///< Sub-structure
                uint32_t                 StmmInputSurfaceControlBitsTargetCacheTc         : __CODEGEN_BITFIELD( 3,  4)    ; ///< Sub-structure
                uint32_t                 StmmInputSurfaceControlBitsMemoryTypeLlcEllcCacheabilityControlLellccc : __CODEGEN_BITFIELD( 5,  6)    ; ///< Sub-structure
                uint32_t                 StmmInputSurfaceControlBitsReserved              : __CODEGEN_BITFIELD( 7,  8)    ; ///< Sub-structure
                uint32_t                 Reserved201                                      : __CODEGEN_BITFIELD( 9, 11)    ; ///< U2
                uint32_t                 StmmInputAddress                                 : __CODEGEN_BITFIELD(12, 31)    ; ///< GraphicsAddress
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 StmmInputAddressHigh                             : __CODEGEN_BITFIELD( 0, 15)    ; ///< GraphicsAddress
                uint32_t                 Reserved240                                      : __CODEGEN_BITFIELD(16, 31)    ; ///< U14
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 StmmOutputSurfaceControlBitsAgeForQuadlruAge     : __CODEGEN_BITFIELD( 0,  1)    ; ///< Sub-structure
                uint32_t                 StmmOutputSurfaceControlBitsReserved2            : __CODEGEN_BITFIELD( 2,  2)    ; ///< Sub-structure
                uint32_t                 StmmOutputSurfaceControlBitsTargetCacheTc        : __CODEGEN_BITFIELD( 3,  4)    ; ///< Sub-structure
                uint32_t                 StmmOutputSurfaceControlBitsMemoryTypeLlcEllcCacheabilityControlLellccc : __CODEGEN_BITFIELD( 5,  6)    ; ///< Sub-structure
                uint32_t                 StmmOutputSurfaceControlBitsReserved             : __CODEGEN_BITFIELD( 7,  8)    ; ///< Sub-structure
                uint32_t                 Reserved265                                      : __CODEGEN_BITFIELD( 9, 11)    ; ///< U2
                uint32_t                 StmmOutputAddress                                : __CODEGEN_BITFIELD(12, 31)    ; ///< GraphicsAddress
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 StmmOutputAddressHigh                            : __CODEGEN_BITFIELD( 0, 15)    ; //!< STMM Output Address High
                uint32_t                 Reserved304                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 DenoisedCurrentOutputSurfaceControlBitsAgeForQuadlruAge : __CODEGEN_BITFIELD( 0,  1)    ; ///< Sub-structure
                uint32_t                 DenoisedCurrentOutputSurfaceControlBitsReserved2 : __CODEGEN_BITFIELD( 2,  2)    ; ///< Sub-structure
                uint32_t                 DenoisedCurrentOutputSurfaceControlBitsTargetCacheTc : __CODEGEN_BITFIELD( 3,  4)    ; ///< Sub-structure
                uint32_t                 DenoisedCurrentOutputSurfaceControlBitsMemoryTypeLlcEllcCacheabilityControlLellccc : __CODEGEN_BITFIELD( 5,  6)    ; ///< Sub-structure
                uint32_t                 DenoisedCurrentOutputSurfaceControlBitsReserved  : __CODEGEN_BITFIELD( 7,  8)    ; ///< Sub-structure
                uint32_t                 Reserved329                                      : __CODEGEN_BITFIELD( 9, 11)    ; ///< U2
                uint32_t                 DenoisedCurrentFrameOutputAddress                : __CODEGEN_BITFIELD(12, 31)    ; ///< GraphicsAddress
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 DenoisedCurrentFrameOutputAddressHigh            : __CODEGEN_BITFIELD( 0, 15)    ; //!< Denoised Current Frame Output Address High
                uint32_t                 Reserved368                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 CurrentFrameOutputSurfaceControlBitsAgeForQuadlruAge : __CODEGEN_BITFIELD( 0,  1)    ; ///< Sub-structure
                uint32_t                 CurrentFrameOutputSurfaceControlBitsReserved2    : __CODEGEN_BITFIELD( 2,  2)    ; ///< Sub-structure
                uint32_t                 CurrentFrameOutputSurfaceControlBitsTargetCacheTc : __CODEGEN_BITFIELD( 3,  4)    ; ///< Sub-structure
                uint32_t                 CurrentFrameOutputSurfaceControlBitsMemoryTypeLlcEllcCacheabilityControlLellccc : __CODEGEN_BITFIELD( 5,  6)    ; ///< Sub-structure
                uint32_t                 CurrentFrameOutputSurfaceControlBitsReserved     : __CODEGEN_BITFIELD( 7,  8)    ; ///< Sub-structure
                uint32_t                 Reserved393                                      : __CODEGEN_BITFIELD( 9, 11)    ; ///< U2
                uint32_t                 CurrentFrameOutputAddress                        : __CODEGEN_BITFIELD(12, 31)    ; ///< GraphicsAddress
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 CurrentFrameOutputAddressHigh                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< Current Frame Output Address High
                uint32_t                 Reserved432                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 PreviousFrameOutputSurfaceControlBitsAgeForQuadlruAge : __CODEGEN_BITFIELD( 0,  1)    ; ///< Sub-structure
                uint32_t                 PreviousFrameOutputSurfaceControlBitsReserved2   : __CODEGEN_BITFIELD( 2,  2)    ; ///< Sub-structure
                uint32_t                 PreviousFrameOutputSurfaceControlBitsTargetCacheTc : __CODEGEN_BITFIELD( 3,  4)    ; ///< Sub-structure
                uint32_t                 PreviousFrameOutputSurfaceControlBitsMemoryTypeLlcEllcCacheabilityControlLellccc : __CODEGEN_BITFIELD( 5,  6)    ; ///< Sub-structure
                uint32_t                 PreviousFrameOutputSurfaceControlBitsReserved    : __CODEGEN_BITFIELD( 7,  8)    ; ///< Sub-structure
                uint32_t                 Reserved457                                      : __CODEGEN_BITFIELD( 9, 11)    ; ///< U2
                uint32_t                 PreviousFrameOutputAddress                       : __CODEGEN_BITFIELD(12, 31)    ; ///< GraphicsAddress
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 PreviousFrameOutputAddressHigh                   : __CODEGEN_BITFIELD( 0, 15)    ; //!< Previous Frame Output Address High
                uint32_t                 Reserved496                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            //!< DWORD 16
            struct
            {
                uint32_t                 StatisticsOutputSurfaceControlBitsAgeForQuadlruAge : __CODEGEN_BITFIELD( 0,  1)    ; ///< Sub-structure
                uint32_t                 StatisticsOutputSurfaceControlBitsReserved2      : __CODEGEN_BITFIELD( 2,  2)    ; ///< Sub-structure
                uint32_t                 StatisticsOutputSurfaceControlBitsTargetCacheTc  : __CODEGEN_BITFIELD( 3,  4)    ; ///< Sub-structure
                uint32_t                 StatisticsOutputSurfaceControlBitsMemoryTypeLlcEllcCacheabilityControlLellccc : __CODEGEN_BITFIELD( 5,  6)    ; ///< Sub-structure
                uint32_t                 StatisticsOutputSurfaceControlBitsReserved       : __CODEGEN_BITFIELD( 7,  8)    ; ///< Sub-structure
                uint32_t                 Reserved521                                      : __CODEGEN_BITFIELD( 9, 11)    ; ///< U2
                uint32_t                 StatisticsOutputAddress                          : __CODEGEN_BITFIELD(12, 31)    ; ///< GraphicsAddress
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            //!< DWORD 17
            struct
            {
                uint32_t                 StatisticsOutputAddressHigh                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Statistics Output Address High
                uint32_t                 Reserved560                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            //!< DWORD 18
            struct
            {
                uint32_t                 AlphaVignetteControlBitsAgeForQuadlruAge         : __CODEGEN_BITFIELD( 0,  1)    ; ///< Sub-structure
                uint32_t                 AlphaVignetteControlBitsReserved2                : __CODEGEN_BITFIELD( 2,  2)    ; ///< Sub-structure
                uint32_t                 AlphaVignetteControlBitsTargetCacheTc            : __CODEGEN_BITFIELD( 3,  4)    ; ///< Sub-structure
                uint32_t                 AlphaVignetteControlBitsMemoryTypeLlcEllcCacheabilityControlLellccc : __CODEGEN_BITFIELD( 5,  6)    ; ///< Sub-structure
                uint32_t                 AlphaVignetteControlBitsReserved                 : __CODEGEN_BITFIELD( 7,  8)    ; ///< Sub-structure
                uint32_t                 Reserved585                                      : __CODEGEN_BITFIELD( 9, 11)    ; ///< U2
                uint32_t                 AlphaVignetteCorrectionAddress                   : __CODEGEN_BITFIELD(12, 31)    ; ///< GraphicsAddress
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            //!< DWORD 19
            struct
            {
                uint32_t                 AlphaVignetteCorrectionAddressHigh               : __CODEGEN_BITFIELD( 0, 15)    ; //!< Alpha/Vignette Correction Address High
                uint32_t                 Reserved624                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW19;

        //! \name Local enumerations

        enum SUBOPB
        {
            SUBOPB_VEBDIIECP                                                 = 3, //!< No additional details
        };

        enum SUBOPA
        {
            SUBOPA_VEBDIIECP                                                 = 0, //!< No additional details
        };

        enum OPCODE
        {
            OPCODE_VEBOX                                                     = 4, //!< No additional details
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
        VEB_DI_IECP_CMD();

        static const size_t dwSize = 20;
        static const size_t byteSize = 80;
    };

    //!
    //! \brief VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS
    //! \details
    //! 
    //!
    struct VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 AgeForQuadlruAge                                 : __CODEGEN_BITFIELD(0, 1); //!< AGE_FOR_QUADLRU_AGE
                uint32_t                 Reserved2                                        : __CODEGEN_BITFIELD(2, 2); //!< Protected Data
                uint32_t                 TargetCacheTc                                    : __CODEGEN_BITFIELD(3, 4); //!< TARGET_CACHE_TC
                uint32_t                 MemoryTypeLlcEllcCacheabilityControlLellccc      : __CODEGEN_BITFIELD(5, 6); //!< MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC
                uint32_t                 Reserved7                                        : __CODEGEN_BITFIELD(7, 31); //!< Reserved
            };
            uint32_t                     Value;
        } DW0;
        //! \name Local enumerations

        //! \brief AGE_FOR_QUADLRU_AGE
        //! \details
        //!     <p style="margin-left:3.0pt;">This field allows the selection of AGE
        //!     parameter for a given surface in LLC or eLLC. If a particular allocation
        //!     is done at youngest age (("0,1,2") it tends to stay longer in the cache.
        //!     This option is given to GFX software to be able to decide which surfaces
        //!     are more likely to generate HITs, hence need to be replaced least often
        //!     in caches.</p>
        enum AGE_FOR_QUADLRU_AGE
        {
            AGE_FOR_QUADLRU_AGE_BESTCHANCE = 0, //!< Best chance of generating cache hits.
            AGE_FOR_QUADLRU_AGE_BETTERCHANCE = 1, //!< Better chance of generating cache hits.
            AGE_FOR_QUADLRU_AGE_NORMALCHANCE = 2, //!< Normal chance of generating cache hits.
            AGE_FOR_QUADLRU_AGE_POORCHANCE = 3, //!< Poor chance of generating cache hits.
        };

        //! \brief TARGET_CACHE_TC
        //! \details
        //!     <p style="margin-left:3.0pt;">This field controls the L3$, LLC and eLLC
        //!     (eDRAM) cacheability for a given surface. Setting of "00" points to PTE
        //!     settings which defaults to eDRAM (when present). If no eDRAM, the access
        //!     will be allocated to LLC. Setting of "01", allocates into LLC and
        //!     victimizes the line to eDRAM. Setting of "10" allows the line to be
        //!     allocated in either LLC or eDRAM. Setting of "11" is the only option for
        //!     a memory access to be allocated in L3$ as well as LLC/eLLC</p>
        //!     <p style="margin-left:3.0pt;">00b: eLLC Only <i>("00" setting points TC
        //!     selection to PTE which defaults to eLLC)</i></p>
        //!     <p style="margin-left:3.0pt;">01b: LLC Only (W<i>orks at the allocation
        //!     time, later victimization from LLC downgrades the line to eLLC if
        //!     present</i>).</p>
        //!     <p style="margin-left:3.0pt;">10b: LLC/eLLC Allowed.</p>
        //!     <p style="margin-left:3.0pt;">11b: L3, LLC, eLLC Allowed.</p>
        //!     <p style="margin-left:3.0pt;"><b>Errata BDW:A-E (FIXED BY:G0
        //!     Stepping):</b></p>
        //!     <p style="margin-left:3.0pt;"></p>
        //!     <p style="margin-left:3.0pt;">For all system that does NOT use SVM (i.e.
        //!     coherent L3$ surfaces), back snoops from LLC has to be disabled
        //!     (<b>Dis_GtCvUpdtOnRd =  "1"</b>). Than target Cache settings can be
        //!     programmed as POR requirements of L3/LLC/eDRAM caching.</p>
        //!     <p style="margin-left:3.0pt;"></p>
        //!     <p style="margin-left:3.0pt;">For all systems that does use SVM (i.e.
        //!     coherent L3$ surfaces), the recomended setting would be "00" in target
        //!     cache settings. In case of L3 surfaces, the performance has to be tuned
        //!     between "00" and "11" setting based on the benefits of L3 caching
        //!     outweighting the degradation of backsnoops.</p>
        //!     <p style="margin-left:3.0pt;"></p>Post G0-stepping, the above w/a for
        //!     coherent L3$ surfaces is not needed.
        enum TARGET_CACHE_TC
        {
            TARGET_CACHE_TC_ELLCONLY = 0, //!< No additional details
            TARGET_CACHE_TC_LLCONLY = 1, //!< No additional details
            TARGET_CACHE_TC_LLCELLCALLOWED = 2, //!< No additional details
            TARGET_CACHE_TC_L3_LLC_ELLCALLOWED = 3, //!< No additional details
        };

        //! \brief MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC
        //! \details
        //!     This is the field used in GT interface block to determine what type of
        //!     access need to be generated to uncore. For the cases where the LeLLCCC
        //!     is set, cacheable transaction are generated to enable LLC usage for
        //!     particular stream.
        enum MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC
        {
            MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_USECACHEABILITYCONTROLSFROMPAGETABLEUCWITHFENCE_IFCOHERENTCYCLE = 0, //!< No additional details
            MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_UNCACHEABLE_UC_NON_CACHEABLE = 1, //!< No additional details
            MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_WRITETHROUGH_WT = 2, //!< No additional details
            MEMORY_TYPE_LLCELLC_CACHEABILITY_CONTROL_LELLCCC_WRITEBACK_WB = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief VEBOX_VERTEX_TABLE_ENTRY
    //! \details
    //! 
    //!
    struct VEBOX_VERTEX_TABLE_ENTRY
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 Vertextableentry0_Cv                            : __CODEGEN_BITFIELD(0, 11); //!< Vertex table entry 0 - Cv(12 bits)
                uint32_t                                                                 : __CODEGEN_BITFIELD(12, 15); //!< Reserved
                uint32_t                 Vertextableentry0_Lv                            : __CODEGEN_BITFIELD(16, 27); //!< Vertex table entry 0 - Lv(12 bits)
                uint32_t                                                                 : __CODEGEN_BITFIELD(28, 31); //!< Reserved
            };
            uint32_t                     Value;
        } DW0;

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;

     };

    //!
    //! \brief VEBOX_VERTEX_TABLE
    //! \details
    //! 
    //!
    struct VEBOX_VERTEX_TABLE_CMD
    {
        VEBOX_VERTEX_TABLE_ENTRY        VertexTableEntry[512];

        static const size_t dwSize = 512;
        static const size_t byteSize = 2048;
     };

};

#pragma pack()

#endif  // __MHW_VEBOX_HWCMD_G8_X_H__