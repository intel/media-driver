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
//! \file     mhw_vdbox_vdenc_hwcmd_g9_bxt.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of g9_bxt as other components
//!           should use MHW interface to interact with MHW commands and states.
//!
#ifndef __MHW_VDBOX_VDENC_HWCMD_G9_BXT_H__
#define __MHW_VDBOX_VDENC_HWCMD_G9_BXT_H__

#pragma once
#pragma pack(1)

#include <cstdint>
#include <cstddef>

class mhw_vdbox_vdenc_g9_bxt
{
public:
    // Internal Macros
    #define __CODEGEN_MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
    #define __CODEGEN_BITFIELD(l, h) (h) - (l) + 1
    #define __CODEGEN_OP_LENGTH_BIAS 2
    #define __CODEGEN_OP_LENGTH(x) (uint32_t)((__CODEGEN_MAX(x, __CODEGEN_OP_LENGTH_BIAS)) - __CODEGEN_OP_LENGTH_BIAS)

    static uint32_t GetOpLength(uint32_t uiLength) { return __CODEGEN_OP_LENGTH(uiLength); }

    //!
    //! \brief VDENC_64B_Aligned_Lower_Address
    //! \details
    //! 
    //!
    struct VDENC_64B_Aligned_Lower_Address_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 Address                                          : __CODEGEN_BITFIELD( 6, 31)    ; //!< Address
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_64B_Aligned_Lower_Address_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief VDENC_64B_Aligned_Upper_Address
    //! \details
    //! 
    //!
    struct VDENC_64B_Aligned_Upper_Address_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 AddressUpperDword                                : __CODEGEN_BITFIELD( 0, 15)    ; //!< Address Upper DWord
                uint32_t                 Reserved16                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_64B_Aligned_Upper_Address_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief VDENC_Surface_Control_Bits
    //! \details
    //! 
    //!
    struct VDENC_Surface_Control_Bits_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD( 0,  6)    ; //!< Index to Memory Object Control State (MOCS) Tables:
                uint32_t                 ArbitrationPriorityControl                       : __CODEGEN_BITFIELD( 7,  8)    ; //!< ARBITRATION_PRIORITY_CONTROL
                uint32_t                 MemoryCompressionEnable                          : __CODEGEN_BITFIELD( 9,  9)    ; //!< MEMORY_COMPRESSION_ENABLE
                uint32_t                 MemoryCompressionMode                            : __CODEGEN_BITFIELD(10, 10)    ; //!< MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved11                                       : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 CacheSelect                                      : __CODEGEN_BITFIELD(12, 12)    ; //!< CACHE_SELECT
                uint32_t                 TiledResourceMode                                : __CODEGEN_BITFIELD(13, 14)    ; //!< TILED_RESOURCE_MODE
                uint32_t                 Reserved15                                       : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \brief ARBITRATION_PRIORITY_CONTROL
        //! \details
        //!     This field controls the priority of arbitration used in the GAC/GAM
        //!     pipeline for this surface.
        enum ARBITRATION_PRIORITY_CONTROL
        {
            ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY                     = 0, //!< No additional details
            ARBITRATION_PRIORITY_CONTROL_SECONDHIGHESTPRIORITY               = 1, //!< No additional details
            ARBITRATION_PRIORITY_CONTROL_THIRDHIGHESTPRIORITY                = 2, //!< No additional details
            ARBITRATION_PRIORITY_CONTROL_LOWESTPRIORITY                      = 3, //!< No additional details
        };

        //! \brief MEMORY_COMPRESSION_ENABLE
        //! \details
        //!     Memory compression will be attempted for this surface.
        enum MEMORY_COMPRESSION_ENABLE
        {
            MEMORY_COMPRESSION_ENABLE_DISABLE                                = 0, //!< No additional details
            MEMORY_COMPRESSION_ENABLE_ENABLE                                 = 1, //!< No additional details
        };

        //! \brief MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression. Please refer to
        //!     vol1a <b>Memory Data</b>.
        enum MEMORY_COMPRESSION_MODE
        {
            MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE                = 0, //!< No additional details
            MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE                  = 1, //!< No additional details
        };

        //! \brief CACHE_SELECT
        //! \details
        //!     This field controls if the Row Store is going to store inside Media
        //!     Cache (rowstore cache) or to LLC.
        enum CACHE_SELECT
        {
            CACHE_SELECT_UNNAMED0                                            = 0, //!< Buffer going to LLC.
            CACHE_SELECT_UNNAMED1                                            = 1, //!< Buffer going to Internal Media Storage.
        };

        //! \brief TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces</b>: This field specifies the tiled resource mode.
        enum TILED_RESOURCE_MODE
        {
            TILED_RESOURCE_MODE_TRMODENONE                                   = 0, //!< No tiled resource.
            TILED_RESOURCE_MODE_TRMODETILEYF                                 = 1, //!< 4KB tiled resources
            TILED_RESOURCE_MODE_TRMODETILEYS                                 = 2, //!< 64KB tiled resources
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Surface_Control_Bits_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief VDENC_Sub_Mb_Pred_Mode
    //! \details
    //! 
    //!
    struct VDENC_Sub_Mb_Pred_Mode_CMD
    {
        union
        {
            //!< WORD 0
            struct
            {
                uint8_t                  Submbpredmode0                                   : __CODEGEN_BITFIELD( 0,  1)    ; //!< SubMbPredMode[0]
                uint8_t                  Submbpredmode1                                   : __CODEGEN_BITFIELD( 2,  3)    ; //!< SubMbPredMode[1]
                uint8_t                  Submbpredmode2                                   : __CODEGEN_BITFIELD( 4,  5)    ; //!< SubMbPredMode[2]
                uint8_t                  Submbpredmode3                                   : __CODEGEN_BITFIELD( 6,  7)    ; //!< SubMbPredMode[3]
            };
            uint8_t                      Value;
        } DW0;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Sub_Mb_Pred_Mode_CMD();

        static const size_t dwSize = 0;
        static const size_t byteSize = 1;
    };

    //!
    //! \brief VDENC_Block_8x8_4
    //! \details
    //! 
    //!
    struct VDENC_Block_8x8_4_CMD
    {
        union
        {
            //!< WORD 0
            struct
            {
                uint16_t                 Block8X80                                        : __CODEGEN_BITFIELD( 0,  3)    ; //!< Block8x8[0]
                uint16_t                 Block8X81                                        : __CODEGEN_BITFIELD( 4,  7)    ; //!< Block8x8[1]
                uint16_t                 Block8X82                                        : __CODEGEN_BITFIELD( 8, 11)    ; //!< Block8x8[2]
                uint16_t                 Block8X83                                        : __CODEGEN_BITFIELD(12, 15)    ; //!< Block8x8[3]
            };
            uint16_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Block_8x8_4_CMD();

        static const size_t dwSize = 0;
        static const size_t byteSize = 2;
    };

    //!
    //! \brief VDENC_Delta_MV_XY
    //! \details
    //! 
    //! 
    //!     Calculates the difference between the actual MV for the Sub Macroblock
    //!     and the predicted MV based on the availability of the neighbors.
    //! 
    //!     This is calculated and populated for Inter frames only. In case of an
    //!     Intra MB in Inter frames, this value should be 0.
    //!
    struct VDENC_Delta_MV_XY_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 X0                                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< X0
                uint32_t                 Y0                                               : __CODEGEN_BITFIELD(16, 31)    ; //!< Y0
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 X1                                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< X1
                uint32_t                 Y1                                               : __CODEGEN_BITFIELD(16, 31)    ; //!< Y1
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 X2                                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< X2
                uint32_t                 Y2                                               : __CODEGEN_BITFIELD(16, 31)    ; //!< Y2
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 X3                                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< X3
                uint32_t                 Y3                                               : __CODEGEN_BITFIELD(16, 31)    ; //!< Y3
            };
            uint32_t                     Value;
        } DW3;

        //! \name Local enumerations

        //! \brief X0
        //! \details
        enum X0
        {
            X0_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief Y0
        //! \details
        enum Y0
        {
            Y0_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief X1
        //! \details
        enum X1
        {
            X1_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief Y1
        //! \details
        enum Y1
        {
            Y1_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief X2
        //! \details
        enum X2
        {
            X2_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief Y2
        //! \details
        enum Y2
        {
            Y2_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief X3
        //! \details
        enum X3
        {
            X3_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \brief Y3
        //! \details
        enum Y3
        {
            Y3_UNNAMED0                                                      = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Delta_MV_XY_CMD();

        static const size_t dwSize = 4;
        static const size_t byteSize = 16;
    };

    //!
    //! \brief VDENC_Colocated_MV_Picture
    //! \details
    //! 
    //!
    struct VDENC_Colocated_MV_Picture_CMD
    {
                VDENC_64B_Aligned_Lower_Address_CMD LowerAddress                                                                     ; //!< Lower Address
                VDENC_64B_Aligned_Upper_Address_CMD UpperAddress                                                                     ; //!< Upper Address
                VDENC_Surface_Control_Bits_CMD PictureFields                                                                    ; //!< Picture Fields

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Colocated_MV_Picture_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief VDENC_Down_Scaled_Reference_Picture
    //! \details
    //! 
    //!
    struct VDENC_Down_Scaled_Reference_Picture_CMD
    {
                VDENC_64B_Aligned_Lower_Address_CMD LowerAddress                                                                     ; //!< Lower Address
                VDENC_64B_Aligned_Upper_Address_CMD UpperAddress                                                                     ; //!< Upper Address
                VDENC_Surface_Control_Bits_CMD PictureFields                                                                    ; //!< Picture Fields

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Down_Scaled_Reference_Picture_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief VDENC_FRAME_BASED_STATISTICS_STREAMOUT
    //! \details
    //! 
    //!
    struct VDENC_FRAME_BASED_STATISTICS_STREAMOUT_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 SumSadHaarForBestMbChoice                                                        ; //!< Sum sad\haar for best MB choice
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 IntraIso16X16MbCount                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< Intra iso 16x16 MB count
                uint32_t                 IntraMbCount                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Intra MB count
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 IntraIso4X4MbCount                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< Intra iso 4x4 MB count
                uint32_t                 IntraIso8X8MbCount                               : __CODEGEN_BITFIELD(16, 31)    ; //!< Intra iso 8x8 MB count
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 SegmentMapCount0                                 : __CODEGEN_BITFIELD( 0, 15)    ; //!< segment map count 0
                uint32_t                 SegmentMapCount1                                 : __CODEGEN_BITFIELD(16, 31)    ; //!< segment map count 1
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 SegmentMapCount2                                 : __CODEGEN_BITFIELD( 0, 15)    ; //!< segment map count 2
                uint32_t                 SegmentMapCount3                                 : __CODEGEN_BITFIELD(16, 31)    ; //!< segment map count 3
            };
            uint32_t                     Value;
        } DW4;

        uint32_t                         Reserved160[12];                                                                 //!< Reserved

        union
        {
            //!< DWORD 17
            struct
            {
                uint32_t                 SumSadHaarForBestMbChoiceBottomHalfPopulation                                    ; //!< Sum sad\haar for best MB choice bottom half population
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            //!< DWORD 18
            struct
            {
                uint32_t                 SumSadHaarForBestMbChoiceTopHalfPopulation                                       ; //!< Sum sad\haar for best MB choice top half population
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            //!< DWORD 19
            struct
            {
                uint32_t                 SumTopHalfPopulationOccurrences                  : __CODEGEN_BITFIELD( 0, 15)    ; //!< Sum top half population occurrences
                uint32_t                 SumBottomHalfPopulationOccurrences               : __CODEGEN_BITFIELD(16, 31)    ; //!< Sum bottom half population occurrences
            };
            uint32_t                     Value;
        } DW19;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_FRAME_BASED_STATISTICS_STREAMOUT_CMD();

        static const size_t dwSize = 20;
        static const size_t byteSize = 80;
    };

    //!
    //! \brief VDENC_Mode_StreamOut_Data
    //! \details
    //! 
    //!
    struct VDENC_Mode_StreamOut_Data_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 MbX                                              : __CODEGEN_BITFIELD( 0,  7)    ; //!< MB.X
                uint32_t                 MbY                                              : __CODEGEN_BITFIELD( 8, 15)    ; //!< MB.Y
                uint32_t                 MinimalDistortion                                : __CODEGEN_BITFIELD(16, 31)    ; //!< Minimal Distortion
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Skiprawdistortion                                : __CODEGEN_BITFIELD( 0, 15)    ; //!< SkipRawDistortion
                uint32_t                 Interrawdistortion                               : __CODEGEN_BITFIELD(16, 31)    ; //!< InterRawDistortion
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Bestintrarawdistortion                           : __CODEGEN_BITFIELD( 0, 15)    ; //!< BestIntraRawDistortion
                uint32_t                 IntermbmodeChromaPredictionMode                  : __CODEGEN_BITFIELD(16, 17)    ; //!< INTERMBMODECHROMA_PREDICTION_MODE
                uint32_t                 Intrambmode                                      : __CODEGEN_BITFIELD(18, 19)    ; //!< INTRAMBMODE
                uint32_t                 Intrambflag                                      : __CODEGEN_BITFIELD(20, 20)    ; //!< INTRAMBFLAG
                uint32_t                 Lastmbflag                                       : __CODEGEN_BITFIELD(21, 21)    ; //!< LASTMBFLAG
                uint32_t                 CoefficientClampOccurred                         : __CODEGEN_BITFIELD(22, 22)    ; //!< Coefficient Clamp Occurred
                uint32_t                 ConformanceViolation                             : __CODEGEN_BITFIELD(23, 23)    ; //!< Conformance Violation
                uint32_t                 Submbpredmode                                    : __CODEGEN_BITFIELD(24, 31)    ; //!< SubMbPredMode
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 Lumaintramode0                                   : __CODEGEN_BITFIELD( 0, 15)    ; //!< LumaIntraMode[0]
                uint32_t                 Lumaintramode1                                   : __CODEGEN_BITFIELD(16, 31)    ; //!< LumaIntraMode[1]
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Lumaintramode2                                   : __CODEGEN_BITFIELD( 0, 15)    ; //!< LumaIntraMode[2]
                uint32_t                 Lumaintramode3                                   : __CODEGEN_BITFIELD(16, 31)    ; //!< LumaIntraMode[3]
            };
            uint32_t                     Value;
        } DW4;
                VDENC_Delta_MV_XY_CMD    DeltaMv0                                                                         ; //!< Delta MV0
                VDENC_Delta_MV_XY_CMD    DeltaMv1                                                                         ; //!< Delta MV1
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 FwdRefids                                        : __CODEGEN_BITFIELD( 0, 15)    ; //!< FWD REFIDs
                uint32_t                 BwdRefids                                        : __CODEGEN_BITFIELD(16, 31)    ; //!< BWD REFIDs
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 QpY                                              : __CODEGEN_BITFIELD( 0,  5)    ; //!< QP_y
                uint32_t                 MbBitCount                                       : __CODEGEN_BITFIELD( 6, 18)    ; //!< MB_Bit_Count
                uint32_t                 MbHeaderCount                                    : __CODEGEN_BITFIELD(19, 31)    ; //!< MB_Header_Count
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 MbType                                           : __CODEGEN_BITFIELD( 0,  4)    ; //!< MB Type
                uint32_t                 BlockCbp                                         : __CODEGEN_BITFIELD( 5, 30)    ; //!< Block CBP
                uint32_t                 Skipmbflag                                       : __CODEGEN_BITFIELD(31, 31)    ; //!< SkipMbFlag
            };
            uint32_t                     Value;
        } DW15;

        //! \name Local enumerations

        //! \brief INTERMBMODECHROMA_PREDICTION_MODE
        //! \details
        //!     This field indicates the InterMB Parition type for Inter MB.
        //!                         <br>OR</br>
        //!                         This field indicates Chroma Prediction Mode for Intra MB.
        enum INTERMBMODECHROMA_PREDICTION_MODE
        {
            INTERMBMODECHROMA_PREDICTION_MODE_UNNAMED0                       = 0, //!< 16x16
            INTERMBMODECHROMA_PREDICTION_MODE_UNNAMED1                       = 1, //!< 16x8
            INTERMBMODECHROMA_PREDICTION_MODE_UNNAMED2                       = 2, //!< 8x16
            INTERMBMODECHROMA_PREDICTION_MODE_UNNAMED3                       = 3, //!< 8x8
        };

        //! \brief INTRAMBMODE
        //! \details
        //!     This field indicates the Best Intra Partition.
        enum INTRAMBMODE
        {
            INTRAMBMODE_UNNAMED0                                             = 0, //!< 16x16
            INTRAMBMODE_UNNAMED1                                             = 1, //!< 8x8
            INTRAMBMODE_UNNAMED2                                             = 2, //!< 4x4
        };

        //! \brief INTRAMBFLAG
        //! \details
        //!     This field specifies whether the current macroblock is an Intra (I)
        //!     macroblock.
        enum INTRAMBFLAG
        {
            INTRAMBFLAG_INTER                                                = 0, //!< inter macroblock
            INTRAMBFLAG_INTRA                                                = 1, //!< intra macroblock
        };

        enum LASTMBFLAG
        {
            LASTMBFLAG_NOTLAST                                               = 0, //!< The current MB is not the last MB in the current Slice.
            LASTMBFLAG_LAST                                                  = 1, //!< The current MB is the last MB in the current Slice.
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Mode_StreamOut_Data_CMD();

        static const size_t dwSize = 16;
        static const size_t byteSize = 64;
    };

    //!
    //! \brief VDENC_Original_Uncompressed_Picture
    //! \details
    //! 
    //!
    struct VDENC_Original_Uncompressed_Picture_CMD
    {
                VDENC_64B_Aligned_Lower_Address_CMD LowerAddress                                                                     ; //!< Lower Address
                VDENC_64B_Aligned_Upper_Address_CMD UpperAddress                                                                     ; //!< Upper Address
                VDENC_Surface_Control_Bits_CMD PictureFields                                                                    ; //!< Picture Fields

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Original_Uncompressed_Picture_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief VDENC_Reference_Picture
    //! \details
    //! 
    //!
    struct VDENC_Reference_Picture_CMD
    {
                VDENC_64B_Aligned_Lower_Address_CMD LowerAddress                                                                     ; //!< Lower Address
                VDENC_64B_Aligned_Upper_Address_CMD UpperAddress                                                                     ; //!< Upper Address
                VDENC_Surface_Control_Bits_CMD PictureFields                                                                    ; //!< Picture Fields

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Reference_Picture_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief VDENC_Row_Store_Scratch_Buffer_Picture
    //! \details
    //! 
    //!
    struct VDENC_Row_Store_Scratch_Buffer_Picture_CMD
    {
                VDENC_64B_Aligned_Lower_Address_CMD LowerAddress                                                                     ; //!< Lower Address
                VDENC_64B_Aligned_Upper_Address_CMD UpperAddress                                                                     ; //!< Upper Address
                VDENC_Surface_Control_Bits_CMD BufferPictureFields                                                              ; //!< Buffer Picture Fields

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Row_Store_Scratch_Buffer_Picture_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief VDENC_Statistics_Streamout
    //! \details
    //! 
    //!
    struct VDENC_Statistics_Streamout_CMD
    {
                VDENC_64B_Aligned_Lower_Address_CMD LowerAddress                                                                     ; //!< Lower Address
                VDENC_64B_Aligned_Upper_Address_CMD UpperAddress                                                                     ; //!< Upper Address
                VDENC_Surface_Control_Bits_CMD PictureFields                                                                    ; //!< Picture Fields

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Statistics_Streamout_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief VDENC_Streamin_Data_Picture
    //! \details
    //! 
    //!
    struct VDENC_Streamin_Data_Picture_CMD
    {
                VDENC_64B_Aligned_Lower_Address_CMD LowerAddress                                                                     ; //!< Lower Address
                VDENC_64B_Aligned_Upper_Address_CMD UpperAddress                                                                     ; //!< Upper Address
                VDENC_Surface_Control_Bits_CMD PictureFields                                                                    ; //!< Picture Fields

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Streamin_Data_Picture_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief VDENC_STREAMIN_STATE
    //! \details
    //! 
    //!
    struct VDENC_STREAMIN_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 RegionOfInterestRoiSelection                     : __CODEGEN_BITFIELD( 0,  7)    ; //!< Region of Interest (ROI) Selection
                uint32_t                 Forceintra                                       : __CODEGEN_BITFIELD( 8,  8)    ; //!< FORCEINTRA
                uint32_t                 Forceskip                                        : __CODEGEN_BITFIELD( 9,  9)    ; //!< FORCESKIP
                uint32_t                 Reserved10                                       : __CODEGEN_BITFIELD(10, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Qpprimey                                         : __CODEGEN_BITFIELD( 0,  7)    ; //!< QPPRIMEY
                uint32_t                 Targetsizeinword                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< TargetSizeInWord
                uint32_t                 Maxsizeinword                                    : __CODEGEN_BITFIELD(16, 23)    ; //!< MaxSizeInWord
                uint32_t                 Reserved56                                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 FwdPredictorX                                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< Fwd Predictor.X
                uint32_t                 FwdPredictorY                                    : __CODEGEN_BITFIELD(16, 31)    ; //!< Fwd Predictor.Y
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 BwdPredictorX                                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< Bwd Predictor.X
                uint32_t                 BwdPredictorY                                    : __CODEGEN_BITFIELD(16, 31)    ; //!< Bwd Predictor.Y
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 FwdRefid0                                        : __CODEGEN_BITFIELD( 0,  3)    ; //!< Fwd RefID0
                uint32_t                 BwdRefid0                                        : __CODEGEN_BITFIELD( 4,  7)    ; //!< Bwd RefID0
                uint32_t                 Reserved136                                      : __CODEGEN_BITFIELD( 8, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;

        uint32_t                         Reserved160[11];                                                                 //!< Reserved

        //! \name Local enumerations

        //! \brief FORCEINTRA
        //! \details
        //!     This field specifies whether current macroblock should be coded as an
        //!     intra macroblock.
        //!                    It is illegal to enable both ForceSkip and ForceIntra for
        //!     the same macroblock.
        //!                    This should be disabled if Rolling-I is enabled in the
        //!     VDEnc Image State.
        enum FORCEINTRA
        {
            FORCEINTRA_DISABLE                                               = 0, //!< VDEnc determined macroblock type
            FORCEINTRA_ENABLE                                                = 1, //!< Force to be coded as an intra macroblock
        };

        //! \brief FORCESKIP
        //! \details
        //!     This field specifies whether current macroblock should be coded as a
        //!     skipped macroblock.
        //!                    It is illegal to enable both ForceSkip and ForceIntra for
        //!     the same macroblock.
        //!                    This should be disabled if Rolling-I is enabled in the
        //!     VDEnc Image State.
        //!                      It is illegal to enable ForceSkip for I-Frames.
        enum FORCESKIP
        {
            FORCESKIP_DISABLE                                                = 0, //!< VDEnc determined macroblock type
            FORCESKIP_ENABLE                                                 = 1, //!< Force to be coded as a skipped macroblock
        };

        //! \brief QPPRIMEY
        //! \details
        //!     Quantization parameter for Y.
        enum QPPRIMEY
        {
            QPPRIMEY_UNNAMED0                                                = 0, //!< No additional details
            QPPRIMEY_UNNAMED51                                               = 51, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_STREAMIN_STATE_CMD();

        static const size_t dwSize = 16;
        static const size_t byteSize = 64;
    };

    //!
    //! \brief VDENC_Surface_State_Fields
    //! \details
    //! 
    //!
    struct VDENC_Surface_State_Fields_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 CrVCbUPixelOffsetVDirection                      : __CODEGEN_BITFIELD( 0,  1)    ; //!< Cr(V)/Cb(U) Pixel Offset V Direction
                uint32_t                 SurfaceFormatByteSwizzle                         : __CODEGEN_BITFIELD( 2,  2)    ; //!< Surface Format Byte Swizzle
                uint32_t                 ColorSpaceSelection                              : __CODEGEN_BITFIELD( 3,  3)    ; //!< Color space selection
                uint32_t                 Width                                            : __CODEGEN_BITFIELD( 4, 17)    ; //!< Width
                uint32_t                 Height                                           : __CODEGEN_BITFIELD(18, 31)    ; //!< Height
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 TileWalk                                         : __CODEGEN_BITFIELD( 0,  0)    ; //!< TILE_WALK
                uint32_t                 TiledSurface                                     : __CODEGEN_BITFIELD( 1,  1)    ; //!< TILED_SURFACE
                uint32_t                 HalfPitchForChroma                               : __CODEGEN_BITFIELD( 2,  2)    ; //!< HALF_PITCH_FOR_CHROMA
                uint32_t                 SurfacePitch                                     : __CODEGEN_BITFIELD( 3, 19)    ; //!< Surface Pitch
                uint32_t                 Reserved52                                       : __CODEGEN_BITFIELD(20, 26)    ; //!< Reserved
                uint32_t                 InterleaveChroma                                 : __CODEGEN_BITFIELD(27, 27)    ; //!< INTERLEAVE_CHROMA_
                uint32_t                 SurfaceFormat                                    : __CODEGEN_BITFIELD(28, 31)    ; //!< SURFACE_FORMAT
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 YOffsetForUCb                                    : __CODEGEN_BITFIELD( 0, 14)    ; //!< Y Offset for U(Cb)
                uint32_t                 Reserved79                                       : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 XOffsetForUCb                                    : __CODEGEN_BITFIELD(16, 30)    ; //!< X Offset for U(Cb)
                uint32_t                 Reserved95                                       : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 YOffsetForVCr                                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< Y Offset for V(Cr)
                uint32_t                 XOffsetForVCr                                    : __CODEGEN_BITFIELD(16, 28)    ; //!< X Offset for V(Cr)
                uint32_t                 Reserved125                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;

        //! \name Local enumerations

        //! \brief TILE_WALK
        //! \details
        //!     (This field must be set to 1: TILEWALK_YMAJOR.) This field specifies the
        //!     type of memory tiling
        //!                         (XMajor or YMajor) employed to tile this surface. See Memory
        //!     Interface Functions for details
        //!                         on memory tiling and restrictions.This field is ignored when the
        //!     surface is linear. Internally
        //!                         H/W always treats this as set to 1 for all VDEnc usage.
        enum TILE_WALK
        {
            TILE_WALK_XMAJOR                                                 = 0, //!< TILEWALK_XMAJOR
            TILE_WALK_YMAJOR                                                 = 1, //!< TILEWALK_YMAJOR
        };

        //! \brief TILED_SURFACE
        //! \details
        //!     (This field must be set to TRUE: Tiled.) This field specifies whether
        //!     the surface is tiled.
        //!                         This field is ignored by VDEnc usage.
        enum TILED_SURFACE
        {
            TILED_SURFACE_FALSE                                              = 0, //!< Linear
            TILED_SURFACE_TRUE                                               = 1, //!< Tiled
        };

        //! \brief HALF_PITCH_FOR_CHROMA
        //! \details
        //!     (This field must be set to Disable.) This field indicates that the
        //!     chroma plane(s) will use a pitch equal
        //!                         to half the value specified in the Surface Pitch field. This field
        //!     is only used for PLANAR surface formats.
        //!                         This field is igored by VDEnc (unless we support YV12).
        enum HALF_PITCH_FOR_CHROMA
        {
            HALF_PITCH_FOR_CHROMA_DISABLE                                    = 0, //!< No additional details
            HALF_PITCH_FOR_CHROMA_ENABLE                                     = 1, //!< No additional details
        };

        //! \brief INTERLEAVE_CHROMA_
        //! \details
        //!     This field indicates that the chroma fields are interleaved in a single
        //!     plane rather than stored as
        //!                         two separate planes. This field is only used for PLANAR surface
        //!     formats.
        enum INTERLEAVE_CHROMA_
        {
            INTERLEAVE_CHROMA_DISABLE                                        = 0, //!< No additional details
            INTERLEAVE_CHROMA_ENABLE                                         = 1, //!< No additional details
        };

        //! \brief SURFACE_FORMAT
        //! \details
        //!     Specifies the format of the surface.
        enum SURFACE_FORMAT
        {
            SURFACE_FORMAT_YUV422                                            = 0, //!< YUYV/YUY2 (8:8:8:8 MSB V0 Y1 U0 Y0)
            SURFACE_FORMAT_RGBA4444                                          = 1, //!< RGBA 32-bit 4:4:4:4 packed (8:8:8:8 MSB-X:B:G:R)
            SURFACE_FORMAT_YUV444                                            = 2, //!< YUV 32-bit 4:4:4 packed (8:8:8:8 MSB-A:Y:U:V)
            SURFACE_FORMAT_Y8UNORM                                           = 3, //!< No additional details
            SURFACE_FORMAT_PLANAR4208                                        = 4, //!< (NV12, IMC1,2,3,4, YV12)
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_Surface_State_Fields_CMD();

        static const size_t dwSize = 4;
        static const size_t byteSize = 16;
    };

    //!
    //! \brief VD_PIPELINE_FLUSH
    //! \details
    //! 
    //!
    struct VD_PIPELINE_FLUSH_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordCountN                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_COUNT_N
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
                uint32_t                 HevcPipelineDone                                 : __CODEGEN_BITFIELD( 0,  0)    ; //!< HEVC pipeline Done
                uint32_t                 VdencPipelineDone                                : __CODEGEN_BITFIELD( 1,  1)    ; //!< VD-ENC pipeline Done
                uint32_t                 MflPipelineDone                                  : __CODEGEN_BITFIELD( 2,  2)    ; //!< MFL pipeline Done
                uint32_t                 MfxPipelineDone                                  : __CODEGEN_BITFIELD( 3,  3)    ; //!< MFX pipeline Done
                uint32_t                 VdCommandMessageParserDone                       : __CODEGEN_BITFIELD( 4,  4)    ; //!< VD command/message parser Done
                uint32_t                 Reserved37                                       : __CODEGEN_BITFIELD( 5, 15)    ; //!< Reserved
                uint32_t                 HevcPipelineCommandFlush                         : __CODEGEN_BITFIELD(16, 16)    ; //!< HEVC pipeline command flush
                uint32_t                 VdencPipelineCommandFlush                        : __CODEGEN_BITFIELD(17, 17)    ; //!< VD-ENC pipeline command flush
                uint32_t                 MflPipelineCommandFlush                          : __CODEGEN_BITFIELD(18, 18)    ; //!< MFL pipeline command flush
                uint32_t                 MfxPipelineCommandFlush                          : __CODEGEN_BITFIELD(19, 19)    ; //!< MFX pipeline command flush
                uint32_t                 Reserved52                                       : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;

        //! \name Local enumerations

        //! \brief DWORD_COUNT_N
        //! \details
        //!     Total Length - 2
        enum DWORD_COUNT_N
        {
            DWORD_COUNT_N_EXCLUDESDWORD_0                                    = 0, //!< No additional details
        };

        enum SUBOPCODEB
        {
            SUBOPCODEB_UNNAMED0                                              = 0, //!< No additional details
        };

        enum SUBOPCODEA
        {
            SUBOPCODEA_UNNAMED0                                              = 0, //!< No additional details
        };

        enum MEDIA_COMMAND_OPCODE
        {
            MEDIA_COMMAND_OPCODE_EXTENDEDCOMMAND                             = 15, //!< No additional details
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
        VD_PIPELINE_FLUSH_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief VDENC_CONST_QPT_STATE
    //! \details
    //!      This commands provides the tables for frame constants to the VDEnc HW.
    //!     The specific parameter value is picked by the VDEnc HW based on the
    //!     frame level QP. The QP Lambda array for costing (motion-vectors and mode
    //!     costs) has 42 entires. Skip Threshold tables has 27 entries. 7 FTQ
    //!     thresholds [0-6] are programmed using  4 sets of tables with 27 entires
    //!     each.
    //!
    struct VDENC_CONST_QPT_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopb                                           : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPB
                uint32_t                 Subopa                                           : __CODEGEN_BITFIELD(21, 22)    ; //!< SUBOPA
                uint32_t                 Opcode                                           : __CODEGEN_BITFIELD(23, 26)    ; //!< OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union {
            //!< DWORD 1..10
            struct {
                uint8_t        QpLambdaArrayIndex[40];                                                                      //!< QP Lambda Array Index[n]
            };
            uint32_t                     Value[10];
        } DW1_10;
        union {
            //!< DWORD 11
            struct {
                uint32_t       QpLambdaArrayIndex40 : __CODEGEN_BITFIELD(0, 7);
                uint32_t       QpLambdaArrayIndex41 : __CODEGEN_BITFIELD(8, 15);
                uint32_t       Reserved : __CODEGEN_BITFIELD(16, 31);
            };
            uint32_t                     Value;
        } DW11;
        union {
            //!< DWORD 12..24
            struct {
                uint16_t       SkipThresholdArrayIndex[26];                                                                 //!< Skip Threshold Array Index[n]
            };
            uint32_t                     Value[13];
        } DW12_24;
        union {
            //!< DWORD 25
            struct {
                uint32_t       SkipThresholdArrayIndex26 : __CODEGEN_BITFIELD(0, 15);
                uint32_t       Reserved : __CODEGEN_BITFIELD(16, 31);
            };
            uint32_t                     Value;
        } DW25;
        union {
            //!< DWORD 26..38
            struct {
                uint16_t       SicForwardTransformCoeffThresholdMatrix0ArrayIndex[26];                                       //!< SIC Forward Transform Coeff Threshold Matrix0 Array Index[n]
            };
            uint32_t                     Value[13];
        } DW26_38;
        union {
            //!< DWORD 39
            struct {
                uint32_t       SicForwardTransformCoeffThresholdMatrix0ArrayIndex26 : __CODEGEN_BITFIELD(0, 15);
                uint32_t       Reserved : __CODEGEN_BITFIELD(16, 31);
            };
            uint32_t                     Value;
        } DW39;
        union {
            //!< DWORD 40..45
            struct {
                uint8_t        SicForwardTransformCoeffThresholdMatrix135ArrayIndexN[24];                                   //!< SIC Forward Transform Coeff Threshold Matrix1/3/5 Array Index[n]
            };
            uint32_t                     Value[6];
        } DW40_45;
        union {
            //!< DWORD 46
            struct {
                uint32_t       SicForwardTransformCoeffThresholdMatrix135ArrayIndex24 : __CODEGEN_BITFIELD(0, 7);
                uint32_t       SicForwardTransformCoeffThresholdMatrix135ArrayIndex25 : __CODEGEN_BITFIELD(8, 15);
                uint32_t       SicForwardTransformCoeffThresholdMatrix135ArrayIndex26 : __CODEGEN_BITFIELD(16, 23);
                uint32_t       Reserved : __CODEGEN_BITFIELD(24, 31);
            };
            uint32_t                     Value;
        } DW46;
        union {
            //!< DWORD 47..52
            struct {
                uint8_t        SicForwardTransformCoeffThresholdMatrix2ArrayIndex[24];                                      //!< SIC Forward Transform Coeff Threshold Matrix2 Array Index[n]
            };
            uint32_t                     Value[6];
        } DW47_52;
        union {
            //!< DWORD 53
            struct {
                uint32_t       SicForwardTransformCoeffThresholdMatrix2ArrayIndex24 : __CODEGEN_BITFIELD(0, 7);
                uint32_t       SicForwardTransformCoeffThresholdMatrix2ArrayIndex25 : __CODEGEN_BITFIELD(8, 15);
                uint32_t       SicForwardTransformCoeffThresholdMatrix2ArrayIndex26 : __CODEGEN_BITFIELD(16, 23);
                uint32_t       Reserved : __CODEGEN_BITFIELD(24, 31);
            };
            uint32_t                     Value;
        } DW53;
        union {
            //!< DWORD 54..59
            struct {
                uint8_t       SicForwardTransformCoeffThresholdMatrix46ArrayIndexN[24];                                     //!< SIC Forward Transform Coeff Threshold Matrix4/6 Array Index[n]
            };
            uint32_t                     Value[6];
        } DW54_59;
        union {
            //!< DWORD 60
            struct {
                uint32_t       SicForwardTransformCoeffThresholdMatrix46ArrayIndex24 : __CODEGEN_BITFIELD(0, 7);
                uint32_t       SicForwardTransformCoeffThresholdMatrix46ArrayIndex25 : __CODEGEN_BITFIELD(8, 15);
                uint32_t       SicForwardTransformCoeffThresholdMatrix46ArrayIndex26 : __CODEGEN_BITFIELD(16, 23);
                uint32_t       Reserved : __CODEGEN_BITFIELD(24, 31);
            };
            uint32_t                     Value;
        } DW60;

        //! \name Local enumerations

        enum SUBOPB
        {
            SUBOPB_VDENCCONSTQPTSTATE                                        = 6, //!< No additional details
        };

        enum SUBOPA
        {
            SUBOPA_UNNAMED0                                                  = 0, //!< No additional details
        };

        enum OPCODE
        {
            OPCODE_VDENCPIPE                                                 = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXCOMMON                                               = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_CONST_QPT_STATE_CMD();

        static const size_t dwSize = 61;
        static const size_t byteSize = 244;
    };

    //!
    //! \brief VDENC_DS_REF_SURFACE_STATE
    //! \details
    //!     This command specifies the surface state parameters for the downscaled
    //!     reference surfaces.
    //!
    struct VDENC_DS_REF_SURFACE_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopb                                           : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPB
                uint32_t                 Subopa                                           : __CODEGEN_BITFIELD(21, 22)    ; //!< SUBOPA
                uint32_t                 Opcode                                           : __CODEGEN_BITFIELD(23, 26)    ; //!< OPCODE
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
                VDENC_Surface_State_Fields_CMD Dwords25                                                                         ; //!< Dwords 2..5

        //! \name Local enumerations

        enum SUBOPB
        {
            SUBOPB_VDENCDSREFSURFACESTATE                                    = 3, //!< No additional details
        };

        enum SUBOPA
        {
            SUBOPA_UNNAMED0                                                  = 0, //!< No additional details
        };

        enum OPCODE
        {
            OPCODE_VDENCPIPE                                                 = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXCOMMON                                               = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_DS_REF_SURFACE_STATE_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief VDENC_IMG_STATE
    //! \details
    //!     This command programs the frame level parameters required by the VDEnc
    //!     pipeline.
    //!
    struct VDENC_IMG_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopb                                           : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPB
                uint32_t                 Subopa                                           : __CODEGEN_BITFIELD(21, 22)    ; //!< SUBOPA
                uint32_t                 Opcode                                           : __CODEGEN_BITFIELD(23, 26)    ; //!< OPCODE
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
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0,  1)    ; //!< Reserved
                uint32_t                 BidirectionalMixDisable                          : __CODEGEN_BITFIELD( 2,  2)    ; //!< BIDIRECTIONAL_MIX_DISABLE
                uint32_t                 VdencPerfmode                                    : __CODEGEN_BITFIELD( 3,  3)    ; //!< VDENC_PERFMODE
                uint32_t                 TimeBudgetOverflowCheck                          : __CODEGEN_BITFIELD( 4,  4)    ; //!< TIME_BUDGET_OVERFLOW_CHECK
                uint32_t                 Reserved37                                       : __CODEGEN_BITFIELD( 5,  5)    ; //!< Reserved
                uint32_t                 VdencExtendedPakObjCmdEnable                     : __CODEGEN_BITFIELD( 6,  6)    ; //!< VDENC_EXTENDED_PAK_OBJ_CMD_ENABLE
                uint32_t                 Transform8X8Flag                                 : __CODEGEN_BITFIELD( 7,  7)    ; //!< TRANSFORM_8X8_FLAG
                uint32_t                 VdencL1CachePriority                             : __CODEGEN_BITFIELD( 8,  9)    ; //!< VDENC_L1_CACHE_PRIORITY
                uint32_t                 Reserved42                                       : __CODEGEN_BITFIELD(10, 15)    ; //!< Reserved
                uint32_t                 LambdaValueForTrellis                            : __CODEGEN_BITFIELD(16, 31)    ; //!< Lambda value for Trellis
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Reserved64                                       : __CODEGEN_BITFIELD( 0, 15)    ; //!< Reserved
                uint32_t                 BidirectionalWeight                              : __CODEGEN_BITFIELD(16, 21)    ; //!< BIDIRECTIONAL_WEIGHT
                uint32_t                 Reserved86                                       : __CODEGEN_BITFIELD(22, 27)    ; //!< Reserved
                uint32_t                 UnidirectionalMixDisable                         : __CODEGEN_BITFIELD(28, 28)    ; //!< Unidirectional Mix Disable
                uint32_t                 Reserved93                                       : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 Reserved96                                       : __CODEGEN_BITFIELD( 0, 15)    ; //!< Reserved
                uint32_t                 PictureWidth                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Picture Width
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Reserved128                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 SubPelMode                                       : __CODEGEN_BITFIELD(12, 13)    ; //!< SUB_PEL_MODE
                uint32_t                 Reserved142                                      : __CODEGEN_BITFIELD(14, 16)    ; //!< Reserved
                uint32_t                 ForwardTransformSkipCheckEnable                  : __CODEGEN_BITFIELD(17, 17)    ; //!< FORWARD_TRANSFORM_SKIP_CHECK_ENABLE
                uint32_t                 BmeDisableForFbrMessage                          : __CODEGEN_BITFIELD(18, 18)    ; //!< BME_DISABLE_FOR_FBR_MESSAGE
                uint32_t                 BlockBasedSkipEnabled                            : __CODEGEN_BITFIELD(19, 19)    ; //!< BLOCK_BASED_SKIP_ENABLED
                uint32_t                 InterSadMeasureAdjustment                        : __CODEGEN_BITFIELD(20, 21)    ; //!< INTER_SAD_MEASURE_ADJUSTMENT
                uint32_t                 IntraSadMeasureAdjustment                        : __CODEGEN_BITFIELD(22, 23)    ; //!< INTRA_SAD_MEASURE_ADJUSTMENT
                uint32_t                 SubMacroblockSubPartitionMask                    : __CODEGEN_BITFIELD(24, 30)    ; //!< SUB_MACROBLOCK_SUB_PARTITION_MASK
                uint32_t                 BlockBasedSkipType                               : __CODEGEN_BITFIELD(31, 31)    ; //!< BLOCK_BASED_SKIP_TYPE
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 PictureHeightMinusOne                            : __CODEGEN_BITFIELD( 0, 15)    ; //!< Picture Height Minus One
                uint32_t                 CrePrefetchEnable                                : __CODEGEN_BITFIELD(16, 16)    ; //!< CRE_PREFETCH_ENABLE
                uint32_t                 HmeRef1Disable                                   : __CODEGEN_BITFIELD(17, 17)    ; //!< HME_REF1_DISABLE
                uint32_t                 MbSliceThresholdValue                            : __CODEGEN_BITFIELD(18, 21)    ; //!< MB Slice Threshold Value
                uint32_t                 Reserved182                                      : __CODEGEN_BITFIELD(22, 25)    ; //!< Reserved
                uint32_t                 ConstrainedIntraPredictionFlag                   : __CODEGEN_BITFIELD(26, 26)    ; //!< CONSTRAINED_INTRA_PREDICTION_FLAG
                uint32_t                 Reserved187                                      : __CODEGEN_BITFIELD(27, 28)    ; //!< Reserved
                uint32_t                 PictureType                                      : __CODEGEN_BITFIELD(29, 30)    ; //!< PICTURE_TYPE
                uint32_t                 Reserved191                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 SliceMacroblockHeightMinusOne                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< Slice Macroblock Height Minus One
                uint32_t                 Reserved208                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 Hme0XOffset                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< HME0 X Offset
                uint32_t                 Hme0YOffset                                      : __CODEGEN_BITFIELD( 8, 15)    ; //!< HME0 Y Offset
                uint32_t                 Hme1XOffset                                      : __CODEGEN_BITFIELD(16, 23)    ; //!< HME1 X Offset
                uint32_t                 Hme1YOffset                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< HME1 Y Offset
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 LumaIntraPartitionMask                           : __CODEGEN_BITFIELD( 0,  4)    ; //!< LUMA_INTRA_PARTITION_MASK
                uint32_t                 NonSkipZeroMvCostAdded                           : __CODEGEN_BITFIELD( 5,  5)    ; //!< Non Skip Zero MV Cost Added
                uint32_t                 NonSkipMbModeCostAdded                           : __CODEGEN_BITFIELD( 6,  6)    ; //!< Non Skip MB Mode Cost Added
                uint32_t                 Reserved263                                      : __CODEGEN_BITFIELD( 7, 15)    ; //!< Reserved
                uint32_t                 MvCostScalingFactor                              : __CODEGEN_BITFIELD(16, 17)    ; //!< MV_COST_SCALING_FACTOR
                uint32_t                 BilinearFilterEnable                             : __CODEGEN_BITFIELD(18, 18)    ; //!< BiLinear Filter Enable
                uint32_t                 Reserved275                                      : __CODEGEN_BITFIELD(19, 21)    ; //!< Reserved
                uint32_t                 RefidCostModeSelect                              : __CODEGEN_BITFIELD(22, 22)    ; //!< REFID_COST_MODE_SELECT
                uint32_t                 Reserved279                                      : __CODEGEN_BITFIELD(23, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 Mode0Cost                                        : __CODEGEN_BITFIELD( 0,  7)    ; //!< Mode 0 Cost
                uint32_t                 Mode1Cost                                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< Mode 1 Cost
                uint32_t                 Mode2Cost                                        : __CODEGEN_BITFIELD(16, 23)    ; //!< Mode 2 Cost
                uint32_t                 Mode3Cost                                        : __CODEGEN_BITFIELD(24, 31)    ; //!< Mode 3 Cost
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 Mode4Cost                                        : __CODEGEN_BITFIELD( 0,  7)    ; //!< Mode 4 Cost
                uint32_t                 Mode5Cost                                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< Mode 5 Cost
                uint32_t                 Mode6Cost                                        : __CODEGEN_BITFIELD(16, 23)    ; //!< Mode 6 Cost
                uint32_t                 Mode7Cost                                        : __CODEGEN_BITFIELD(24, 31)    ; //!< Mode 7 Cost
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 Mode8Cost                                        : __CODEGEN_BITFIELD( 0,  7)    ; //!< Mode 8 Cost
                uint32_t                 Mode9Cost                                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< Mode 9 Cost
                uint32_t                 RefIdCost                                        : __CODEGEN_BITFIELD(16, 23)    ; //!< RefID Cost
                uint32_t                 ChromaIntraModeCost                              : __CODEGEN_BITFIELD(24, 31)    ; //!< Chroma Intra Mode Cost
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 MvCost0                                          : __CODEGEN_BITFIELD( 0,  7)    ; //!< MvCost 0
                uint32_t                 MvCost1                                          : __CODEGEN_BITFIELD( 8, 15)    ; //!< MvCost 1
                uint32_t                 MvCost2                                          : __CODEGEN_BITFIELD(16, 23)    ; //!< MvCost 2
                uint32_t                 MvCost3                                          : __CODEGEN_BITFIELD(24, 31)    ; //!< MvCost 3
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 MvCost4                                          : __CODEGEN_BITFIELD( 0,  7)    ; //!< MvCost 4
                uint32_t                 MvCost5                                          : __CODEGEN_BITFIELD( 8, 15)    ; //!< MvCost 5
                uint32_t                 MvCost6                                          : __CODEGEN_BITFIELD(16, 23)    ; //!< MvCost 6
                uint32_t                 MvCost7                                          : __CODEGEN_BITFIELD(24, 31)    ; //!< MvCost 7
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 QpPrimeY                                         : __CODEGEN_BITFIELD( 0,  7)    ; //!< QpPrimeY
                uint32_t                 Reserved456                                      : __CODEGEN_BITFIELD( 8, 23)    ; //!< Reserved
                uint32_t                 TargetSizeInWord                                 : __CODEGEN_BITFIELD(24, 31)    ; //!< TargetSizeInWord
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 Reserved480                                                                      ; //!< Reserved
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
                uint32_t                 AvcIntra4X4ModeMask                              : __CODEGEN_BITFIELD( 0,  8)    ; //!< AVC Intra 4x4 Mode Mask
                uint32_t                 Reserved553                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved
                uint32_t                 AvcIntra8X8ModeMask                              : __CODEGEN_BITFIELD(16, 24)    ; //!< AVC Intra 8x8 Mode Mask
                uint32_t                 Reserved569                                      : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            //!< DWORD 18
            struct
            {
                uint32_t                 AvcIntra16X16ModeMask                            : __CODEGEN_BITFIELD( 0,  3)    ; //!< AVC_INTRA_16X16_MODE_MASK
                uint32_t                 AvcIntraChromaModeMask                           : __CODEGEN_BITFIELD( 4,  7)    ; //!< AVC_INTRA_CHROMA_MODE_MASK
                uint32_t                 IntraComputeTypeIntracomputetype                 : __CODEGEN_BITFIELD( 8,  9)    ; //!< INTRA_COMPUTE_TYPE_INTRACOMPUTETYPE
                uint32_t                 Reserved586                                      : __CODEGEN_BITFIELD(10, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            //!< DWORD 19
            struct
            {
                uint32_t                 Reserved608                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            //!< DWORD 20
            struct
            {
                uint32_t                 PenaltyForIntra16X16NondcPrediction              : __CODEGEN_BITFIELD( 0,  7)    ; //!< Penalty for Intra16x16 NonDC Prediction.
                uint32_t                 PenaltyForIntra8X8NondcPrediction                : __CODEGEN_BITFIELD( 8, 15)    ; //!< Penalty for Intra8x8 NonDC Prediction.
                uint32_t                 PenaltyForIntra4X4NondcPrediction                : __CODEGEN_BITFIELD(16, 23)    ; //!< Penalty for Intra4x4 NonDC Prediction.
                uint32_t                 Reserved664                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            //!< DWORD 21
            struct
            {
                uint32_t                 IntraRefreshMBPos                                : __CODEGEN_BITFIELD( 0,  7)    ; //!< IntraRefreshMBPos
                uint32_t                 IntraRefreshMBSizeMinusOne                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< IntraRefreshMBSizeMinusOne
                uint32_t                 IntraRefreshEnableRollingIEnable                 : __CODEGEN_BITFIELD(16, 16)    ; //!< INTRAREFRESHENABLE_ROLLING_I_ENABLE
                uint32_t                 IntraRefreshMode                                 : __CODEGEN_BITFIELD(17, 17)    ; //!< INTRAREFRESHMODE
                uint32_t                 Reserved690                                      : __CODEGEN_BITFIELD(18, 23)    ; //!< Reserved
                uint32_t                 QpAdjustmentForRollingI                          : __CODEGEN_BITFIELD(24, 31)    ; //!< QP adjustment for Rolling-I
            };
            uint32_t                     Value;
        } DW21;
        union
        {
            //!< DWORD 22
            struct
            {
                uint32_t                 Panicmodembthreshold                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< PanicModeMBThreshold
                uint32_t                 Smallmbsizeinword                                : __CODEGEN_BITFIELD(16, 23)    ; //!< SmallMbSizeInWord
                uint32_t                 Largembsizeinword                                : __CODEGEN_BITFIELD(24, 31)    ; //!< LargeMbSizeInWord
            };
            uint32_t                     Value;
        } DW22;
        union
        {
            //!< DWORD 23
            struct
            {
                uint32_t                 L0NumberOfReferencesMinusOne                     : __CODEGEN_BITFIELD( 0,  7)    ; //!< L0 number of references Minus one
                uint32_t                 Reserved744                                      : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 L1NumberOfReferencesMinusOne                     : __CODEGEN_BITFIELD(16, 23)    ; //!< L1 number of references Minus One
                uint32_t                 Reserved760                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW23;
        union
        {
            //!< DWORD 24
            struct
            {
                uint32_t                 MacroblockBudget                                 : __CODEGEN_BITFIELD( 0, 15)    ; //!< Macroblock Budget
                uint32_t                 InitialTime                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Initial Time
            };
            uint32_t                     Value;
        } DW24;
        union
        {
            //!< DWORD 25
            struct
            {
                uint32_t                 Reserved800                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW25;
        union
        {
            //!< DWORD 26
            struct
            {
                uint32_t                 Reserved832                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< Reserved
                uint32_t                 HmeRefWindowsCombiningThreshold                  : __CODEGEN_BITFIELD( 8, 15)    ; //!< HME_REF_WINDOWS_COMBINING_THRESHOLD
                uint32_t                 Reserved848                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW26;
        union
        {
            //!< DWORD 27
            struct
            {
                uint32_t                 MaxHmvR                                          : __CODEGEN_BITFIELD( 0, 15)    ; //!< MAXHMVR
                uint32_t                 MaxVmvR                                          : __CODEGEN_BITFIELD(16, 31)    ; //!< MAXVMVR
            };
            uint32_t                     Value;
        } DW27;
        union
        {
            //!< DWORD 28
            struct
            {
                uint32_t                 HmeMvCost0                                          : __CODEGEN_BITFIELD( 0,  7)    ; //!< HmeMvCost 0
                uint32_t                 HmeMvCost1                                          : __CODEGEN_BITFIELD( 8, 15)    ; //!< HmeMvCost 1
                uint32_t                 HmeMvCost2                                          : __CODEGEN_BITFIELD(16, 23)    ; //!< HmeMvCost 2
                uint32_t                 HmeMvCost3                                          : __CODEGEN_BITFIELD(24, 31)    ; //!< HmeMvCost 3
            };
            uint32_t                     Value;
        } DW28;
        union
        {
            //!< DWORD 29
            struct
            {
                uint32_t                 HmeMvCost4                                          : __CODEGEN_BITFIELD( 0,  7)    ; //!< HmeMvCost 4
                uint32_t                 HmeMvCost5                                          : __CODEGEN_BITFIELD( 8, 15)    ; //!< HmeMvCost 5
                uint32_t                 HmeMvCost6                                          : __CODEGEN_BITFIELD(16, 23)    ; //!< HmeMvCost 6
                uint32_t                 HmeMvCost7                                          : __CODEGEN_BITFIELD(24, 31)    ; //!< HmeMvCost 7
            };
            uint32_t                     Value;
        } DW29;
        union
        {
            //!< DWORD 30
            struct
            {
                uint32_t                 RoiQpAdjustmentForZone0                          : __CODEGEN_BITFIELD( 0,  3)    ; //!< ROI QP adjustment for Zone0
                uint32_t                 RoiQpAdjustmentForZone1                          : __CODEGEN_BITFIELD( 4,  7)    ; //!< ROI QP adjustment for Zone1
                uint32_t                 RoiQpAdjustmentForZone2                          : __CODEGEN_BITFIELD( 8, 11)    ; //!< ROI QP adjustment for Zone2
                uint32_t                 RoiQpAdjustmentForZone3                          : __CODEGEN_BITFIELD(12, 15)    ; //!< ROI QP adjustment for Zone3
                uint32_t                 QpAdjustmentForShapeBestIntra4X4Winner           : __CODEGEN_BITFIELD(16, 19)    ; //!< QP adjustment for shape best intra 4x4 winner
                uint32_t                 QpAdjustmentForShapeBestIntra8X8Winner           : __CODEGEN_BITFIELD(20, 23)    ; //!< QP adjustment for shape best intra 8x8 winner
                uint32_t                 QpAdjustmentForShapeBestIntra16X16Winner         : __CODEGEN_BITFIELD(24, 27)    ; //!< QP adjustment for shape best intra 16x16 winner
                uint32_t                 Reserved988                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW30;
        union
        {
            //!< DWORD 31
            struct
            {
                uint32_t                 BestdistortionQpAdjustmentForZone0               : __CODEGEN_BITFIELD( 0,  3)    ; //!< BestDistortion QP adjustment for Zone0
                uint32_t                 BestdistortionQpAdjustmentForZone1               : __CODEGEN_BITFIELD( 4,  7)    ; //!< BestDistortion QP adjustment for Zone1
                uint32_t                 BestdistortionQpAdjustmentForZone2               : __CODEGEN_BITFIELD( 8, 11)    ; //!< BestDistortion QP adjustment for Zone2
                uint32_t                 BestdistortionQpAdjustmentForZone3               : __CODEGEN_BITFIELD(12, 15)    ; //!< BestDistortion QP adjustment for Zone3
                uint32_t                 SadHaarThreshold0                                : __CODEGEN_BITFIELD(16, 31)    ; //!< Sad/Haar_Threshold_0
            };
            uint32_t                     Value;
        } DW31;
        union
        {
            //!< DWORD 32
            struct
            {
                uint32_t                 SadHaarThreshold1                                : __CODEGEN_BITFIELD( 0, 15)    ; //!< Sad/Haar_Threshold_1
                uint32_t                 SadHaarThreshold2                                : __CODEGEN_BITFIELD(16, 31)    ; //!< Sad/Haar_Threshold_2
            };
            uint32_t                     Value;
        } DW32;
        union
        {
            //!< DWORD 33
            struct
            {
                uint32_t                 MaxQp                                            : __CODEGEN_BITFIELD( 0,  7)    ; //!< MaxQP
                uint32_t                 MinQp                                            : __CODEGEN_BITFIELD( 8, 15)    ; //!< MinQP
                uint32_t                 Reserved1072                                     : __CODEGEN_BITFIELD(16, 23)    ; //!< Reserved
                uint32_t                 Maxdeltaqp                                       : __CODEGEN_BITFIELD(24, 27)    ; //!< MaxDeltaQP
                uint32_t                 Reserved1084                                     : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW33;
        union
        {
            //!< DWORD 34
            struct
            {
                uint32_t                 RoiEnable                                        : __CODEGEN_BITFIELD( 0,  0)    ; //!< ROI_Enable
                uint32_t                 FwdPredictor0MvEnable                            : __CODEGEN_BITFIELD( 1,  1)    ; //!< Fwd/Predictor0 MV Enable
                uint32_t                 BwdPredictor1MvEnable                            : __CODEGEN_BITFIELD( 2,  2)    ; //!< Bwd/Predictor1 MV Enable
                uint32_t                 MbLevelQpEnable                                  : __CODEGEN_BITFIELD( 3,  3)    ; //!< MB Level QP Enable
                uint32_t                 TargetsizeinwordsmbMaxsizeinwordsmbEnable        : __CODEGEN_BITFIELD( 4,  4)    ; //!< TargetSizeinWordsMB/MaxSizeinWordsMB Enable
                uint32_t                 Reserverd                                        : __CODEGEN_BITFIELD( 5,  7)    ; //!< Reserverd
                uint32_t                 PpmvDisable                                      : __CODEGEN_BITFIELD( 8,  8)    ; //!< PPMV_DISABLE
                uint32_t                 CoefficientClampEnable                           : __CODEGEN_BITFIELD( 9,  9)    ; //!< Coefficient Clamp Enable
                uint32_t                 LongtermReferenceFrameBwdRef0Indicator           : __CODEGEN_BITFIELD(10, 10)    ; //!< LONGTERM_REFERENCE_FRAME_BWD_REF0_INDICATOR
                uint32_t                 LongtermReferenceFrameFwdRef2Indicator           : __CODEGEN_BITFIELD(11, 11)    ; //!< LONGTERM_REFERENCE_FRAME_FWD_REF2_INDICATOR
                uint32_t                 LongtermReferenceFrameFwdRef1Indicator           : __CODEGEN_BITFIELD(12, 12)    ; //!< LONGTERM_REFERENCE_FRAME_FWD_REF1_INDICATOR
                uint32_t                 LongtermReferenceFrameFwdRef0Indicator           : __CODEGEN_BITFIELD(13, 13)    ; //!< LONGTERM_REFERENCE_FRAME_FWD_REF0_INDICATOR
                uint32_t                 ImageStateQpOverride                             : __CODEGEN_BITFIELD(14, 14)    ; //!< IMAGE_STATE_QP_OVERRIDE
                uint32_t                 Reserved1102                                     : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 MidpointSadHaar                                  : __CODEGEN_BITFIELD(16, 31)    ; //!< Midpoint sad/haar
            };
            uint32_t                     Value;
        } DW34;

        //! \name Local enumerations

        enum SUBOPB
        {
            SUBOPB_VDENCIMGSTATE                                             = 5, //!< No additional details
        };

        enum SUBOPA
        {
            SUBOPA_UNNAMED0                                                  = 0, //!< No additional details
        };

        enum OPCODE
        {
            OPCODE_VDENCPIPE                                                 = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXCOMMON                                               = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum BIDIRECTIONAL_MIX_DISABLE
        {
            BIDIRECTIONAL_MIX_DISABLE_SUBBLOCKENABLED                        = 0, //!< Bidirectional decision on subblock level that bidirectional mode is enabled.
            BIDIRECTIONAL_MIX_DISABLE_WHOLEMACROBLOCKENABLED                 = 1, //!< Bidirectional decision on whole macroblock.
        };

        //! \brief VDENC_PERFMODE
        //! \details
        //!     This bit indicates if VDEnc is configured for normal or speed mode of
        //!     operation.
        enum VDENC_PERFMODE
        {
            VDENC_PERFMODE_NORMAL                                            = 0, //!< VDEnc is running in normal mode. IME Search: 3x3 SU per each reference. HME Search: 88x92 search window per HME instance (0 &amp; 1).
            VDENC_PERFMODE_SPEED                                             = 1, //!< VDEnc is configured for speed mode. IME Search: 2x2 SU per each reference. HME Search: 48x92 search window per HME instance (0 &amp; 1).
        };

        //! \brief TIME_BUDGET_OVERFLOW_CHECK
        //! \details
        //!     <p>This bit enables the frame time budget detection in VDEnc.</p>
        //!                                  <p>To detect if a Time Budget Overflow happened in a frame, SW
        //!     can read "PAK_Stream-Out Report (Errors)" register in MFX. When Time
        //!     budget overflow condition happens in the frame, this register bits 15:8
        //!     indicate MB y position and bits 7:0 indicate MB x position where Time
        //!     budget overflow occured. When there is no time budget overflow in a
        //!     frame, "<span style="line-height: 20.7999992370605px;">PAK_Stream-Out
        //!     Report (Errors)" register reads zero.</span></p>
        enum TIME_BUDGET_OVERFLOW_CHECK
        {
            TIME_BUDGET_OVERFLOW_CHECK_DISABLED                              = 0, //!< No additional details
            TIME_BUDGET_OVERFLOW_CHECK_ENABLED                               = 1, //!< No additional details
        };

        //! \brief VDENC_EXTENDED_PAK_OBJ_CMD_ENABLE
        //! \details
        //!     This bit enables the distortion data to be populated in the VDenc PAK
        //!     Obj inline data.
        enum VDENC_EXTENDED_PAK_OBJ_CMD_ENABLE
        {
            VDENC_EXTENDED_PAK_OBJ_CMD_ENABLE_DISABLE                        = 0, //!< The extra two DWS from VDEnc (MDC) to PAK will be Zero.
            VDENC_EXTENDED_PAK_OBJ_CMD_ENABLE_ENABLE                         = 1, //!< The last two DWs from VDEnc (MDC) to PAK will be populated with distortion data. (Defined in the PAK Object command DW 22,23.)
        };

        //! \brief TRANSFORM_8X8_FLAG
        //! \details
        //!     8x8 IDCT Transform Mode Flag, trans8x8_mode_flag specifies 8x8 IDCT
        //!     transform may be used in this
        //!                         picture. It is set to the value of the syntax element in the
        //!     current active PPS.
        enum TRANSFORM_8X8_FLAG
        {
            TRANSFORM_8X8_FLAG_DISABLED                                      = 0, //!< No 8x8 IDCT Transform, only 4x4 IDCT transform blocks are present.
            TRANSFORM_8X8_FLAG_ENABLED                                       = 1, //!< 8x8 Transform is allowed.
        };

        //! \brief VDENC_L1_CACHE_PRIORITY
        //! \details
        //!     L1 Cache inside VDEnc has 3 clients - IME, CRE and VMC. These bits
        //!     indicate the priority order for
        //!                         L1 cache to service the client requests.
        enum VDENC_L1_CACHE_PRIORITY
        {
            VDENC_L1_CACHE_PRIORITY_UNNAMED0                                 = 0, //!< CRE High Priority, VMC and IME round robin.
            VDENC_L1_CACHE_PRIORITY_UNNAMED1                                 = 1, //!< CRE and VMC round robin, IME low priority.
            VDENC_L1_CACHE_PRIORITY_UNNAMED2                                 = 2, //!< CRE High Priority, IME Medium, VMC Low.
            VDENC_L1_CACHE_PRIORITY_UNNAMED3                                 = 3, //!< VMC High Priority, CRE Medium, IME low.
        };

        //! \brief BIDIRECTIONAL_WEIGHT
        //! \details
        //!     Default value: Depends on the distance between the B and reference
        //!     pictures.
        enum BIDIRECTIONAL_WEIGHT
        {
            BIDIRECTIONAL_WEIGHT_UNNAMED16                                   = 16, //!< No additional details
            BIDIRECTIONAL_WEIGHT_UNNAMED21                                   = 21, //!< No additional details
            BIDIRECTIONAL_WEIGHT_UNNAMED32                                   = 32, //!< No additional details
            BIDIRECTIONAL_WEIGHT_UNNAMED43                                   = 43, //!< No additional details
            BIDIRECTIONAL_WEIGHT_UNNAMED48                                   = 48, //!< No additional details
        };

        //! \brief SUB_PEL_MODE
        //! \details
        //!     This field defines the half/quarter pel modes. The mode is inclusive,
        //!     i.e., higher precision mode samples lower precision locations.
        enum SUB_PEL_MODE
        {
            SUB_PEL_MODE_UNNAMED0                                            = 0, //!< Integer mode searching.
            SUB_PEL_MODE_UNNAMED1                                            = 1, //!< Half-pel mode searching.
            SUB_PEL_MODE_UNNAMED3                                            = 3, //!< Quarter-pel mode searching.
        };

        //! \brief FORWARD_TRANSFORM_SKIP_CHECK_ENABLE
        //! \details
        //!     This field enables the forward transform calculation for skip check. It
        //!     does not override the other
        //!                         skip calculations but it does decrease the performance marginally
        //!     so don't enable it unless the transform is necessary.
        enum FORWARD_TRANSFORM_SKIP_CHECK_ENABLE
        {
            FORWARD_TRANSFORM_SKIP_CHECK_ENABLE_FTDISABLED                   = 0, //!< No additional details
            FORWARD_TRANSFORM_SKIP_CHECK_ENABLE_FTENABLED                    = 1, //!< No additional details
        };

        //! \brief BME_DISABLE_FOR_FBR_MESSAGE
        //! \details
        //!     FBR messages that do not want bidirectional motion estimation performed
        //!     will set this bit and VME will
        //!                         only perform fractional refinement on the shapes identified by
        //!     subpredmode. Note: only the LSB of the
        //!                         subpredmode for each shape will be considered in FBR (a shape is
        //!     either FWD or BWD as input of FBR,
        //!                         output however could change to BI if BME is enabled).
        enum BME_DISABLE_FOR_FBR_MESSAGE
        {
            BME_DISABLE_FOR_FBR_MESSAGE_BMEENABLED                           = 0, //!< No additional details
            BME_DISABLE_FOR_FBR_MESSAGE_BMEDISABLED                          = 1, //!< No additional details
        };

        //! \brief BLOCK_BASED_SKIP_ENABLED
        //! \details
        //!     When this field is set on the skip thresholding passing criterion will
        //!     be based on the maximal distortion
        //!                         of individual blocks (8x8's or 4x4's) instead of their sum (i.e.
        //!     the distortion of 16x16).
        enum BLOCK_BASED_SKIP_ENABLED
        {
            BLOCK_BASED_SKIP_ENABLED_UNNAMED0                                = 0, //!< 16x16 Block Based Skip threshold check.
            BLOCK_BASED_SKIP_ENABLED_BLOCK_BASEDSKIPTYPE                     = 1, //!< Parameter indicates 8x8 vs. 4x4 based check.
        };

        //! \brief INTER_SAD_MEASURE_ADJUSTMENT
        //! \details
        //!     This field specifies distortion measure adjustments used for the motion
        //!     search SAD comparison.
        //!                         This field applies to both luma and chroma inter measurement.
        enum INTER_SAD_MEASURE_ADJUSTMENT
        {
            INTER_SAD_MEASURE_ADJUSTMENT_NONE                                = 0, //!< No additional details
            INTER_SAD_MEASURE_ADJUSTMENT_HAARTRANSFORMADJUSTED               = 2, //!< No additional details
        };

        //! \brief INTRA_SAD_MEASURE_ADJUSTMENT
        //! \details
        //!     This field specifies distortion measure adjustments used for the motion
        //!     search SAD comparison.
        //!                         This field applies to both luma and chroma intra measurement.
        enum INTRA_SAD_MEASURE_ADJUSTMENT
        {
            INTRA_SAD_MEASURE_ADJUSTMENT_NONE                                = 0, //!< No additional details
            INTRA_SAD_MEASURE_ADJUSTMENT_HAARTRANSFORMADJUSTED               = 2, //!< No additional details
        };

        //! \brief SUB_MACROBLOCK_SUB_PARTITION_MASK
        //! \details
        //!     This field defines the bit-mask for disabling
        //!                         <ul>
        //!                             <li>sub-partition (minor partition [30:28]) modes</li>
        //!                             <li>sub-macroblock (major partition [27:24]) modes</li>
        //!                         </ul>
        enum SUB_MACROBLOCK_SUB_PARTITION_MASK
        {
            SUB_MACROBLOCK_SUB_PARTITION_MASK_UNNAMED113                     = 113, //!< 16x16 sub-macroblock disabled
            SUB_MACROBLOCK_SUB_PARTITION_MASK_UNNAMED114                     = 114, //!< 2x(16x8) sub-macroblock within 16x16 disabled
            SUB_MACROBLOCK_SUB_PARTITION_MASK_UNNAMED116                     = 116, //!< 2x(8x16) sub-macroblock within 16x16 disabled
            SUB_MACROBLOCK_SUB_PARTITION_MASK_UNNAMED120                     = 120, //!< 1x(8x8) sub-partition for 4x(8x8) within 16x16 disabled
        };

        //! \brief BLOCK_BASED_SKIP_TYPE
        //! \details
        //!     The skip thresholding passing criterion will be based on the maximal
        //!     distortion of individual blocks
        //!                         (8x8's or 4x4's) instead of their sum (i.e. the distortion of
        //!     16x16). This field is only valid when
        //!                         <b>Block-Based Skip Enabled</b> = 1.
        enum BLOCK_BASED_SKIP_TYPE
        {
            BLOCK_BASED_SKIP_TYPE_UNNAMED0                                   = 0, //!< 4x4 block-based skip threshold check.
            BLOCK_BASED_SKIP_TYPE_UNNAMED1                                   = 1, //!< 8x8 block-based skip threshold check.
        };

        //! \brief CRE_PREFETCH_ENABLE
        //! \details
        //!     This field determines if IME will prefetch the fractional CLs that are
        //!     required by CRE ahead of time
        //!                         while fetching the reference windows around the IME predictors. The
        //!     recommendation for driver is to 
        //!                         always program this bit to 1 unless some usages restrict SubPelMode
        //!     to be "<i>Integer mode searching</i>".
        enum CRE_PREFETCH_ENABLE
        {
            CRE_PREFETCH_ENABLE_UNNAMED0                                     = 0, //!< Disable
            CRE_PREFETCH_ENABLE_UNNAMED1                                     = 1, //!< Enable
        };

        //! \brief HME_REF1_DISABLE
        //! \details
        //!     This field indicates if HME is disabled for reference 1 (second forward
        //!     reference).
        enum HME_REF1_DISABLE
        {
            HME_REF1_DISABLE_UNNAMED0                                        = 0, //!< HME search is performed on forward reference 1.
            HME_REF1_DISABLE_UNNAMED1                                        = 1, //!< HME search is disabled on forward reference 1.
        };

        enum CONSTRAINED_INTRA_PREDICTION_FLAG
        {
            CONSTRAINED_INTRA_PREDICTION_FLAG_UNNAMED0                       = 0, //!< Allows both intra and inter neighboring MB to be used in the intra-prediction decoding of the current MB.
            CONSTRAINED_INTRA_PREDICTION_FLAG_UNNAMED1                       = 1, //!< Allows only to use neighboring Intra MBs in the intra-prediction decoding of the current MB.If the neighbor is an inter MB, it is considered as not available.
        };

        //! \brief PICTURE_TYPE
        //! \details
        //!     This field specifies how the current picture is predicted. (It might be
        //!     redundant from the kernel type.)
        enum PICTURE_TYPE
        {
            PICTURE_TYPE_I                                                   = 0, //!< No additional details
            PICTURE_TYPE_P                                                   = 1, //!< No additional details
        };

        //! \brief LUMA_INTRA_PARTITION_MASK
        //! \details
        //!     This field specifies which Luma Intra partition is enabled/disabled for
        //!     intra mode decision.
        enum LUMA_INTRA_PARTITION_MASK
        {
            LUMA_INTRA_PARTITION_MASK_UNNAMED1                               = 1, //!< luma_intra_16x16 disabled
            LUMA_INTRA_PARTITION_MASK_UNNAMED2                               = 2, //!< luma_intra_8x8 disabled
            LUMA_INTRA_PARTITION_MASK_UNNAMED4                               = 4, //!< luma_intra_4x4 disabled
        };

        enum MV_COST_SCALING_FACTOR
        {
            MV_COST_SCALING_FACTOR_QPEL                                      = 0, //!< Qpel difference between MV and cost center: eff cost range 0-15pel
            MV_COST_SCALING_FACTOR_HPEL                                      = 1, //!< Hpel difference between MV and cost center: eff cost range 0-31pel
            MV_COST_SCALING_FACTOR_PEL                                       = 2, //!< Pel   difference between MV and cost center: eff cost range 0-63pel
            MV_COST_SCALING_FACTOR_2PEL                                      = 3, //!< 2Pel difference between MV and cost center: eff cost range 0-127pel
        };

        enum REFID_COST_MODE_SELECT
        {
            REFID_COST_MODE_SELECT_MODE0                                     = 0, //!< AVC
            REFID_COST_MODE_SELECT_MODE1                                     = 1, //!< Linear
        };

        enum AVC_INTRA_16X16_MODE_MASK
        {
            AVC_INTRA_16X16_MODE_MASK_VERT                                   = 1, //!< No additional details
            AVC_INTRA_16X16_MODE_MASK_HORZ                                   = 2, //!< No additional details
            AVC_INTRA_16X16_MODE_MASK_DC                                     = 4, //!< No additional details
            AVC_INTRA_16X16_MODE_MASK_PLANAR                                 = 8, //!< No additional details
        };

        enum AVC_INTRA_CHROMA_MODE_MASK
        {
            AVC_INTRA_CHROMA_MODE_MASK_VERT                                  = 1, //!< No additional details
            AVC_INTRA_CHROMA_MODE_MASK_HORZ                                  = 2, //!< No additional details
            AVC_INTRA_CHROMA_MODE_MASK_DC                                    = 4, //!< No additional details
            AVC_INTRA_CHROMA_MODE_MASK_PLANAR                                = 8, //!< No additional details
        };

        //! \brief INTRA_COMPUTE_TYPE_INTRACOMPUTETYPE
        //! \details
        //!     This field specifies the pixel components measured for Intra prediction.
        enum INTRA_COMPUTE_TYPE_INTRACOMPUTETYPE
        {
            INTRA_COMPUTE_TYPE_INTRACOMPUTETYPE_UNNAMED0                     = 0, //!< Luma+Chroma enabled.
            INTRA_COMPUTE_TYPE_INTRACOMPUTETYPE_UNNAMED1                     = 1, //!< Luma Only.
            INTRA_COMPUTE_TYPE_INTRACOMPUTETYPE_UNNAMED2                     = 2, //!< Intra Disabled.
        };

        //! \brief INTRAREFRESHENABLE_ROLLING_I_ENABLE
        //! \details
        //!     <p>This parameter indicates if the IntraRefresh is enabled or
        //!     disabled.</p>
        //! 
        //!     <p>This must be disabled on I-Frames.</p>
        enum INTRAREFRESHENABLE_ROLLING_I_ENABLE
        {
            INTRAREFRESHENABLE_ROLLING_I_ENABLE_DISABLE                      = 0, //!< No additional details
            INTRAREFRESHENABLE_ROLLING_I_ENABLE_ENABLE                       = 1, //!< No additional details
        };

        //! \brief INTRAREFRESHMODE
        //! \details
        //!     This parameter indicates if the IntraRefresh is row based or column
        //!     based.
        enum INTRAREFRESHMODE
        {
            INTRAREFRESHMODE_ROWBASED                                        = 0, //!< No additional details
            INTRAREFRESHMODE_COLUMNBASED                                     = 1, //!< No additional details
        };

        //! \brief HME_REF_WINDOWS_COMBINING_THRESHOLD
        //! \details
        //!     When the reference windows of the HME refinement VME call and the
        //!     regular VME call are overlapped
        //!                         and the difference of the locations is within this threshold in
        //!     quarter pixel unit, the two calls
        //!                         are merged to a single call.
        enum HME_REF_WINDOWS_COMBINING_THRESHOLD
        {
            HME_REF_WINDOWS_COMBINING_THRESHOLD_UNNAMED0                     = 0, //!< No additional details
            HME_REF_WINDOWS_COMBINING_THRESHOLD_UNNAMED255                   = 255, //!< No additional details
        };

        //! \brief MAXHMVR
        //! \details
        //!     Horizontal MV component range. The MV range is restricted to
        //!     [-MaxHmvR+1, MaxHmvR-1] in luma quarter pel unit,
        //!                         which corresponds to [-MaxHmvR/4 + 0.25, MaxHmvR/4-0.25] in luma
        //!     integer pel unit.
        enum MAXHMVR
        {
            MAXHMVR_UNNAMED256                                               = 256, //!< No additional details
            MAXHMVR_UNNAMED512                                               = 512, //!< No additional details
            MAXHMVR_UNNAMED1024                                              = 1024, //!< No additional details
            MAXHMVR_UNNAMED2048                                              = 2048, //!< No additional details
            MAXHMVR_UNNAMED4096                                              = 4096, //!< No additional details
            MAXHMVR_UNNAMED8192                                              = 8192, //!< No additional details
        };

        //! \brief MAXVMVR
        //! \details
        //!     Vertical MV component range defined in the AVC Spec Annex A. The MV
        //!     range is restricted to [-MaxVmvR+1, MaxVmvR-1]
        //!                         in luma quarter pel unit, which corresponds to [-MaxVmvR/4 + 0.25,
        //!     MaxVmvR/4-0.25] in luma integer pel unit.
        enum MAXVMVR
        {
            MAXVMVR_UNNAMED256                                               = 256, //!< No additional details
            MAXVMVR_UNNAMED512                                               = 512, //!< No additional details
            MAXVMVR_UNNAMED1024                                              = 1024, //!< No additional details
            MAXVMVR_UNNAMED2048                                              = 2048, //!< No additional details
        };

        //! \brief PPMV_DISABLE
        //! \details
        //!     This bit forces the IME to use the actual PMV predictor for the IME
        //!     search.
        enum PPMV_DISABLE
        {
            PPMV_DISABLE_UNNAMED0                                            = 0, //!< Use PPMV based IME search.
            PPMV_DISABLE_UNNAMED1                                            = 1, //!< Use PMV based IME search.
        };

        //! \brief LONGTERM_REFERENCE_FRAME_BWD_REF0_INDICATOR
        //! \details
        //!     Indicates whether the reference frame is a long or short term reference.
        enum LONGTERM_REFERENCE_FRAME_BWD_REF0_INDICATOR
        {
            LONGTERM_REFERENCE_FRAME_BWD_REF0_INDICATOR_SHORT_TERMREFERENCE  = 0, //!< No additional details
            LONGTERM_REFERENCE_FRAME_BWD_REF0_INDICATOR_LONG_TERMREFERENCE   = 1, //!< No additional details
        };

        //! \brief LONGTERM_REFERENCE_FRAME_FWD_REF2_INDICATOR
        //! \details
        //!     Indicates whether the reference frame is a long or short term reference.
        enum LONGTERM_REFERENCE_FRAME_FWD_REF2_INDICATOR
        {
            LONGTERM_REFERENCE_FRAME_FWD_REF2_INDICATOR_SHORT_TERMREFERENCE  = 0, //!< No additional details
            LONGTERM_REFERENCE_FRAME_FWD_REF2_INDICATOR_LONG_TERMREFERENCE   = 1, //!< No additional details
        };

        //! \brief LONGTERM_REFERENCE_FRAME_FWD_REF1_INDICATOR
        //! \details
        //!     Indicates whether the reference frame is a long or short term reference.
        enum LONGTERM_REFERENCE_FRAME_FWD_REF1_INDICATOR
        {
            LONGTERM_REFERENCE_FRAME_FWD_REF1_INDICATOR_SHORT_TERMREFERENCE  = 0, //!< No additional details
            LONGTERM_REFERENCE_FRAME_FWD_REF1_INDICATOR_LONG_TERMREFERENCE   = 1, //!< No additional details
        };

        //! \brief LONGTERM_REFERENCE_FRAME_FWD_REF0_INDICATOR
        //! \details
        //!     Indicates whether the reference frame is a long or short term reference.
        enum LONGTERM_REFERENCE_FRAME_FWD_REF0_INDICATOR
        {
            LONGTERM_REFERENCE_FRAME_FWD_REF0_INDICATOR_SHORT_TERMREFERENCE  = 0, //!< No additional details
            LONGTERM_REFERENCE_FRAME_FWD_REF0_INDICATOR_LONG_TERMREFERENCE   = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_IMG_STATE_CMD();

        static const size_t dwSize = 35;
        static const size_t byteSize = 140;
    };

    //!
    //! \brief VDENC_PIPE_BUF_ADDR_STATE
    //! \details
    //!     This state command provides the memory base addresses for all row
    //!     stores, Streamin/StreamOut, DMV buffer along with the uncompressed
    //!     source, reference pictures and downscaled reference pictures required by
    //!     the VDENC pipeline. All reference pixel surfaces in the Encoder are
    //!     programmed with the same surface state (NV12 and TileY format), except
    //!     each has its own frame buffer base address. Same holds true for the
    //!     down-scaled reference pictures too. In the tile format, there is no need
    //!     to provide buffer offset for each slice; since from each MB address, the
    //!     hardware can calculated the corresponding memory location within the
    //!     frame buffer directly.  VDEnc supports 3 Downscaled reference frames ( 2
    //!     fwd, 1 bwd) and 4 normal reference frames ( 3 fwd, 1 bwd). The driver
    //!     will sort out the base address from the DPB table and populate the base
    //!     addresses that map to the corresponding reference index for both DS
    //!     references and normal reference frames. Each of the individual DS ref/
    //!     Normal ref frames have their own MOCS DW that corresponds to the
    //!     respective base address. The only thing that is different in the MOCS DW
    //!     amongst the DS reference frames is the MMCD controls (specified in bits
    //!     [10:9] of the MOCS DW). Driver needs to ensure that the other bits need
    //!     to be the same across the different DS ref frames. The same is
    //!     applicable for the normal reference frames.
    //!
    struct VDENC_PIPE_BUF_ADDR_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopb                                           : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPB
                uint32_t                 Subopa                                           : __CODEGEN_BITFIELD(21, 22)    ; //!< SUBOPA
                uint32_t                 Opcode                                           : __CODEGEN_BITFIELD(23, 26)    ; //!< OPCODE
                uint32_t                 Pipeline                                         : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        VDENC_Down_Scaled_Reference_Picture_CMD DsFwdRef0                                                                 ; //!< DS FWD REF0
        VDENC_Down_Scaled_Reference_Picture_CMD DsFwdRef1                                                                 ; //!< DS FWD REF1
        VDENC_Down_Scaled_Reference_Picture_CMD DsBwdRef0                                                                 ; //!< DS BWD REF0
        VDENC_Original_Uncompressed_Picture_CMD OriginalUncompressedPicture                                               ; //!< Original Uncompressed Picture
        VDENC_Streamin_Data_Picture_CMD StreaminDataPicture                                                               ; //!< Streamin Data Picture
        VDENC_Row_Store_Scratch_Buffer_Picture_CMD RowStoreScratchBuffer                                                  ; //!< Row Store Scratch Buffer
        VDENC_Colocated_MV_Picture_CMD ColocatedMv                                                                        ; //!< Colocated MV
        VDENC_Reference_Picture_CMD FwdRef0                                                                               ; //!< FWD REF0
        VDENC_Reference_Picture_CMD FwdRef1                                                                               ; //!< FWD REF1
        VDENC_Reference_Picture_CMD FwdRef2                                                                               ; //!< FWD REF2
        VDENC_Reference_Picture_CMD BwdRef0                                                                               ; //!< BWD REF0
        VDENC_Statistics_Streamout_CMD VdencStatisticsStreamout                                                           ; //!< VDEnc Statistics Streamout

        //! \name Local enumerations

        enum SUBOPB
        {
            SUBOPB_VDENCPIPEBUFADDRSTATE                                     = 4, //!< No additional details
        };

        enum SUBOPA
        {
            SUBOPA_UNNAMED0                                                  = 0, //!< No additional details
        };

        enum OPCODE
        {
            OPCODE_VDENCPIPE                                                 = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXCOMMON                                               = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_PIPE_BUF_ADDR_STATE_CMD();

        static const size_t dwSize = 37;
        static const size_t byteSize = 148;
    };

    //!
    //! \brief VDENC_PIPE_MODE_SELECT
    //! \details
    //!     Specifies which codec and hardware module is being used to encode/decode
    //!     the video data, on a per-frame basis. The VDENC_PIPE_MODE_SELECT command
    //!     specifies which codec and hardware module is being used to encode/decode
    //!     the video data, on a per-frame basis. It also configures the hardware
    //!     pipeline according to the active encoder/decoder operating mode for
    //!     encoding/decoding the current picture. 
    //!
    struct VDENC_PIPE_MODE_SELECT_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopb                                           : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPB
                uint32_t                 Subopa                                           : __CODEGEN_BITFIELD(21, 22)    ; //!< SUBOPA
                uint32_t                 Opcode                                           : __CODEGEN_BITFIELD(23, 26)    ; //!< OPCODE
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
                uint32_t                 StandardSelect                                   : __CODEGEN_BITFIELD( 0,  3)    ; //!< STANDARD_SELECT
                uint32_t                 Reserved36                                       : __CODEGEN_BITFIELD( 4,  4)    ; //!< Reserved
                uint32_t                 FrameStatisticsStreamOutEnable                   : __CODEGEN_BITFIELD( 5,  5)    ; //!< FRAME_STATISTICS_STREAM_OUT_ENABLE
                uint32_t                 Reserved38                                       : __CODEGEN_BITFIELD( 6,  6)    ; //!< Reserved
                uint32_t                 TlbPrefetchEnable                                : __CODEGEN_BITFIELD( 7,  7)    ; //!< TLB_PREFETCH_ENABLE
                uint32_t                 PakThresholdCheckEnable                          : __CODEGEN_BITFIELD( 8,  8)    ; //!< PAK_THRESHOLD_CHECK_ENABLE
                uint32_t                 VdencStreamInEnable                              : __CODEGEN_BITFIELD( 9,  9)    ; //!< VDENC_STREAM_IN_ENABLE
                uint32_t                 Reserved42                                       : __CODEGEN_BITFIELD(10, 14)    ; //!< Reserved
                uint32_t                 PakChromaSubSamplingType                         : __CODEGEN_BITFIELD(15, 16)    ; //!< PAK_CHROMA_SUB_SAMPLING_TYPE
                uint32_t                 OutputRangeControlAfterColorSpaceConversion      : __CODEGEN_BITFIELD(17, 17)    ; //!< output range control after color space conversion
                uint32_t                 Reserved50                                       : __CODEGEN_BITFIELD(18, 30)    ; //!< Reserved
                uint32_t                 DisableSpeedModeFetchOptimization                : __CODEGEN_BITFIELD(31, 31)    ; //!< Disable Speed Mode fetch optimization
            };
            uint32_t                     Value;
        } DW1;

        //! \name Local enumerations

        enum SUBOPB
        {
            SUBOPB_VDENCPIPEMODESELECT                                       = 0, //!< No additional details
        };

        enum SUBOPA
        {
            SUBOPA_UNNAMED0                                                  = 0, //!< No additional details
        };

        enum OPCODE
        {
            OPCODE_VDENCPIPE                                                 = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXCOMMON                                               = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum STANDARD_SELECT
        {
            STANDARD_SELECT_AVC                                              = 2, //!< No additional details
        };

        //! \brief FRAME_STATISTICS_STREAM_OUT_ENABLE
        //! \details
        //!     This field controls whether the frame statistics stream-out is enabled.
        enum FRAME_STATISTICS_STREAM_OUT_ENABLE
        {
            FRAME_STATISTICS_STREAM_OUT_ENABLE_DISABLE                       = 0, //!< No additional details
            FRAME_STATISTICS_STREAM_OUT_ENABLE_ENABLE                        = 1, //!< No additional details
        };

        //! \brief TLB_PREFETCH_ENABLE
        //! \details
        //!     This field controls whether TLB prefetching is enabled.
        enum TLB_PREFETCH_ENABLE
        {
            TLB_PREFETCH_ENABLE_DISABLE                                      = 0, //!< No additional details
            TLB_PREFETCH_ENABLE_ENABLE                                       = 1, //!< No additional details
        };

        //! \brief PAK_THRESHOLD_CHECK_ENABLE
        //! \details
        //!     <p>For AVC standard: This field controls whether VDEnc will check the
        //!     PAK indicator for bits overflow and terminates the slice. This mode is
        //!     called Dynamic Slice Mode. When this field is disabled, VDEnc is in
        //!     Static Slice Mode. It uses the driver programmed Slice Macroblock Height
        //!     Minus One to terminate the slice. This feature is also referred to as
        //!     slice size conformance.</p>
        //!     <p>For HEVC standard: This bit is used to enable dynamic slice size
        //!     control.</p>
        enum PAK_THRESHOLD_CHECK_ENABLE
        {
            PAK_THRESHOLD_CHECK_ENABLE_DISABLESTATICSLICEMODE                = 0, //!< No additional details
            PAK_THRESHOLD_CHECK_ENABLE_ENABLEDYNAMICSLICEMODE                = 1, //!< No additional details
        };

        //! \brief VDENC_STREAM_IN_ENABLE
        //! \details
        //!     <p>This field controls whether VDEnc will read the stream-in surface
        //!     that is programmed. Currently the stream-in surface has MB level QP,
        //!     ROI, predictors and MaxSize/TargetSizeinWordsMB parameters. The
        //!     individual enables for each of the fields is programmed in the
        //!     VDENC_IMG_STATE.</p>
        //!     <p>(ROI_Enable, Fwd/Predictor0 MV Enable, Bwd/Predictor1 MV Enable, MB
        //!     Level QP Enable, TargetSizeinWordsMB/MaxSizeinWordsMB Enable).</p>
        //!     <p>This bit is valid only in AVC mode. In HEVC / VP9 mode this bit is
        //!     reserved and should be set to zero.</p>
        enum VDENC_STREAM_IN_ENABLE
        {
            VDENC_STREAM_IN_ENABLE_DISABLE                                   = 0, //!< No additional details
            VDENC_STREAM_IN_ENABLE_ENABLE                                    = 1, //!< No additional details
        };

        //! \brief PAK_CHROMA_SUB_SAMPLING_TYPE
        //! \details
        //!     <p>This field is applicable only in HEVC and VP9. In AVC, this field is
        //!     ignored.</p>
        //!     <p></p>
        enum PAK_CHROMA_SUB_SAMPLING_TYPE
        {
            PAK_CHROMA_SUB_SAMPLING_TYPE_420                                 = 1, //!< Used for Main8 and Main10 HEVC, VP9 profile0, AVC.
            PAK_CHROMA_SUB_SAMPLING_TYPE_444                                 = 3, //!< HEVC RExt 444, VP9 444 profiles.
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_PIPE_MODE_SELECT_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief VDENC_REF_SURFACE_STATE
    //! \details
    //!     This command specifies the surface state parameters for the normal
    //!     reference surfaces.
    //!
    struct VDENC_REF_SURFACE_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopb                                           : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPB
                uint32_t                 Subopa                                           : __CODEGEN_BITFIELD(21, 22)    ; //!< SUBOPA
                uint32_t                 Opcode                                           : __CODEGEN_BITFIELD(23, 26)    ; //!< OPCODE
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
                VDENC_Surface_State_Fields_CMD Dwords25                                                                         ; //!< Dwords 2..5

        //! \name Local enumerations

        enum SUBOPB
        {
            SUBOPB_VDENCREFSURFACESTATE                                      = 2, //!< No additional details
        };

        enum SUBOPA
        {
            SUBOPA_UNNAMED0                                                  = 0, //!< No additional details
        };

        enum OPCODE
        {
            OPCODE_VDENCPIPE                                                 = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXCOMMON                                               = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_REF_SURFACE_STATE_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief VDENC_SRC_SURFACE_STATE
    //! \details
    //!     This command specifies the uncompressed original input picture to be
    //!     encoded. The actual base address is defined in the
    //!     VDENC_PIPE_BUF_ADDR_STATE. Pitch can be wider than the Picture Width in
    //!     pixels and garbage will be there at the end of each line. The following
    //!     describes all the different formats that are supported in WLV+ VDEnc: 
    //!     NV12 - 4:2:0 only; UV interleaved; Full Pitch, U and V offset is set to
    //!     0 (the only format supported for video codec); vertical UV offset is MB
    //!     aligned; UV xoffsets = 0.
    //!       This surface state here is identical to the Surface State for
    //!     deinterlace and sample_8x8 messages described in the Shared Function
    //!     Volume and Sampler Chapter. For non pixel data, such as row stores, DMV
    //!     and streamin/out, a linear buffer is employed. For row stores, the H/W
    //!     is designed to guarantee legal memory accesses (read and write). For the
    //!     remaining cases, indirect object base address, indirect object address
    //!     upper bound, object data start address (offset) and object data length
    //!     are used to fully specified their corresponding buffer. This mechanism
    //!     is chosen over the pixel surface type because of their variable record
    //!     sizes. All row store surfaces are linear surface. Their addresses are
    //!     programmed in VDEnc_Pipe_Buf_Base_State.
    //!
    struct VDENC_SRC_SURFACE_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopb                                           : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPB
                uint32_t                 Subopa                                           : __CODEGEN_BITFIELD(21, 22)    ; //!< SUBOPA
                uint32_t                 Opcode                                           : __CODEGEN_BITFIELD(23, 26)    ; //!< OPCODE
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
                VDENC_Surface_State_Fields_CMD Dwords25                                                                         ; //!< Dwords 2..5

        //! \name Local enumerations

        enum SUBOPB
        {
            SUBOPB_VDENCSRCSURFACESTATE                                      = 1, //!< No additional details
        };

        enum SUBOPA
        {
            SUBOPA_UNNAMED0                                                  = 0, //!< No additional details
        };

        enum OPCODE
        {
            OPCODE_VDENCPIPE                                                 = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXCOMMON                                               = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_SRC_SURFACE_STATE_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief VDENC_WALKER_STATE
    //! \details
    //!     This command provides the macroblock start location for the VDEnc
    //!     walker. Current programming to always have this command at the frame
    //!     level, hence the macroblock X,Y location need to be programmed to 0,0 to
    //!     always start at frame origin. Once the hardware receives this command
    //!     packet, it internally starts the VDEnc pipeline. This should be the last
    //!     command that is programmed for the VDEnc pipeline.
    //!
    struct VDENC_WALKER_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Subopb                                           : __CODEGEN_BITFIELD(16, 20)    ; //!< SUBOPB
                uint32_t                 Subopa                                           : __CODEGEN_BITFIELD(21, 22)    ; //!< SUBOPA
                uint32_t                 Opcode                                           : __CODEGEN_BITFIELD(23, 26)    ; //!< OPCODE
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
                uint32_t                 MbLcuStartYPosition                              : __CODEGEN_BITFIELD( 0,  8)    ; //!< MB/LCU Start Y Position
                uint32_t                 Reserved41                                       : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved
                uint32_t                 MbLcuStartXPosition                              : __CODEGEN_BITFIELD(16, 24)    ; //!< MB/LCU Start X Position
                uint32_t                 Reserved57                                       : __CODEGEN_BITFIELD(25, 27)    ; //!< Reserved
                uint32_t                 FirstSuperSlice                                  : __CODEGEN_BITFIELD(28, 28)    ; //!< First Super Slice
                uint32_t                 Reserved61                                       : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;

        //! \name Local enumerations

        enum SUBOPB
        {
            SUBOPB_VDENCWALKERSTATE                                          = 7, //!< No additional details
        };

        enum SUBOPA
        {
            SUBOPA_UNNAMED0                                                  = 0, //!< No additional details
        };

        enum OPCODE
        {
            OPCODE_VDENCPIPE                                                 = 1, //!< No additional details
        };

        enum PIPELINE
        {
            PIPELINE_MFXCOMMON                                               = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_WALKER_STATE_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

};

#pragma pack()

#endif  // __MHW_VDBOX_VDENC_HWCMD_G9_BXT_H__
