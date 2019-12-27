/*===================== begin_copyright_notice ==================================

Copyright (c) 2017-2019, Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     mhw_vdbox_vdenc_hwcmd_g12_X.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of g12_X as other components
//!           should use MHW interface to interact with MHW commands and states.
//!
#ifndef __MHW_VDBOX_VDENC_HWCMD_G12_X_H__
#define __MHW_VDBOX_VDENC_HWCMD_G12_X_H__

#pragma once
#pragma pack(1)

#include <cstdint>
#include <cstddef>

class mhw_vdbox_vdenc_g12_X
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
                uint32_t                 CompressionType                                  : __CODEGEN_BITFIELD(10, 10)    ; //!< Compression Type
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
        //!     <br>OR</br>
        //!     This field indicates Chroma Prediction Mode for Intra MB.
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
    //! \brief VDENC_HEVC_VP9_FRAME_BASED_STATISTICS_STREAMOUT
    //! \details
    //!     
    //!     
    struct VDENC_HEVC_VP9_FRAME_BASED_STATISTICS_STREAMOUT_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 SumSadHaarForBestModeDecision                                                    ; //!< Sum sad\haar for best mode decision
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 IntraCuCountNormalized                           : __CODEGEN_BITFIELD( 0, 19)    ; //!< Intra CU count normalized
                uint32_t                 Reserved52                                       : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 NonSkipInterCuCountNormalized                    : __CODEGEN_BITFIELD( 0, 19)    ; //!< Non-skip Inter CU count normalized
                uint32_t                 Reserved84                                       : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 SegmentMapCount0                                 : __CODEGEN_BITFIELD( 0, 19)    ; //!< segment map count 0
                uint32_t                 Reserved116                                      : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 SegmentMapCount1                                 : __CODEGEN_BITFIELD( 0, 19)    ; //!< segment map count 1
                uint32_t                 Reserved148                                      : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 SegmentMapCount2                                 : __CODEGEN_BITFIELD( 0, 19)    ; //!< segment map count 2
                uint32_t                 Reserved180                                      : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 SegmentMapCount3                                 : __CODEGEN_BITFIELD( 0, 19)    ; //!< segment map count 3
                uint32_t                 Reserved212                                      : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 MvXGlobalMeSample025X25X                         : __CODEGEN_BITFIELD( 0, 15)    ; //!< MV.x Global ME sample 0 (.25x,.25x)
                uint32_t                 MvYGlobalMeSample025X25X                         : __CODEGEN_BITFIELD(16, 31)    ; //!< MV.y Global ME sample 0 (.25x,.25x)
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 MvXGlobalMeSample125X25X                         : __CODEGEN_BITFIELD( 0, 15)    ; //!< MV.x Global ME sample 1 (.25x,.25x)
                uint32_t                 MvYGlobalMeSample125X25X                         : __CODEGEN_BITFIELD(16, 31)    ; //!< MV.y Global ME sample 1 (.25x,.25x)
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 MvXGlobalMeSample225X25X                         : __CODEGEN_BITFIELD( 0, 15)    ; //!< MV.x Global ME sample 2 (.25x,.25x)
                uint32_t                 MvYGlobalMeSample225X25X                         : __CODEGEN_BITFIELD(16, 31)    ; //!< MV.y Global ME sample 2 (.25x,.25x)
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 MvXGlobalMeSample325X25X                         : __CODEGEN_BITFIELD( 0, 15)    ; //!< MV.x Global ME sample 3 (.25x,.25x)
                uint32_t                 MvYGlobalMeSample325X25X                         : __CODEGEN_BITFIELD(16, 31)    ; //!< MV.y Global ME sample 3 (.25x,.25x)
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 MvXGlobalMeSample425X25X                         : __CODEGEN_BITFIELD( 0, 15)    ; //!< MV.x Global ME sample 4 (.25x,.25x)
                uint32_t                 MvYGlobalMeSample425X25X                         : __CODEGEN_BITFIELD(16, 31)    ; //!< MV.y Global ME sample 4 (.25x,.25x)
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 MvXGlobalMeSample525X25X                         : __CODEGEN_BITFIELD( 0, 15)    ; //!< MV.x Global ME sample 5 (.25x,.25x)
                uint32_t                 MvYGlobalMeSample525X25X                         : __CODEGEN_BITFIELD(16, 31)    ; //!< MV.y Global ME sample 5 (.25x,.25x)
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 MvXGlobalMeSample625X25X                         : __CODEGEN_BITFIELD( 0, 15)    ; //!< MV.x Global ME sample 6 (.25x,.25x)
                uint32_t                 MvYGlobalMeSample625X25X                         : __CODEGEN_BITFIELD(16, 31)    ; //!< MV.y Global ME sample 6 (.25x,.25x)
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 MvXGlobalMeSample725X25X                         : __CODEGEN_BITFIELD( 0, 15)    ; //!< MV.x Global ME sample 7 (.25x,.25x)
                uint32_t                 MvYGlobalMeSample725X25X                         : __CODEGEN_BITFIELD(16, 31)    ; //!< MV.y Global ME sample 7 (.25x,.25x)
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 MvXGlobalMeSample825X25X                         : __CODEGEN_BITFIELD( 0, 15)    ; //!< MV.x Global ME sample 8 (.25x,.25x)
                uint32_t                 MvYGlobalMeSample825X25X                         : __CODEGEN_BITFIELD(16, 31)    ; //!< MV.y Global ME sample 8 (.25x,.25x)
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            //!< DWORD 16
            struct
            {
                uint32_t                 RefidForGlobalmeSample0                          : __CODEGEN_BITFIELD( 0,  1)    ; //!< RefID for GlobalME sample 0
                uint32_t                 RefidForGlobalmeSample18                         : __CODEGEN_BITFIELD( 2, 17)    ; //!< RefID for GlobalME sample 1-8
                uint32_t                 Reserved530                                      : __CODEGEN_BITFIELD(18, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            //!< DWORD 17
            struct
            {
                uint32_t                 PaletteCuCountNormalized                         : __CODEGEN_BITFIELD( 0, 19)    ; //!< Palette CU Count Normalized
                uint32_t                 Reserved564                                      : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            //!< DWORD 18
            struct
            {
                uint32_t                 IbcCuCountNormalized                             : __CODEGEN_BITFIELD( 0, 19)    ; //!< IBC CU Count Normalized
                uint32_t                 Reserved596                                      : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            //!< DWORD 19
            struct
            {
                uint32_t                 NumberOfSecondaryColorsChannel1                  : __CODEGEN_BITFIELD( 0, 15)    ; //!< Number of secondary colors (Channel1)
                uint32_t                 NumberOfPrimaryColorsChannel0                    : __CODEGEN_BITFIELD(16, 31)    ; //!< Number of primary colors (Channel0)
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            //!< DWORD 20
            struct
            {
                uint32_t                 NumberOfSecondaryColorsChannel2                  : __CODEGEN_BITFIELD( 0, 15)    ; //!< Number of secondary colors (Channel2)
                uint32_t                 Reserved656                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            //!< DWORD 21
            struct
            {
                uint32_t                 Reserved672                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW21;
        union
        {
            //!< DWORD 22
            struct
            {
                uint32_t                 PositionOfTimerExpiration                        : __CODEGEN_BITFIELD( 0, 15)    ; //!< Position of Timer expiration
                uint32_t                 TimerExpireStatus                                : __CODEGEN_BITFIELD(16, 16)    ; //!< Timer Expire status
                uint32_t                 Reserved721                                      : __CODEGEN_BITFIELD(17, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW22;
        union
        {
            //!< DWORD 23
            struct
            {
                uint32_t                 LocationOfPanic                                  : __CODEGEN_BITFIELD( 0, 15)    ; //!< Location of panic
                uint32_t                 PanicDetected                                    : __CODEGEN_BITFIELD(16, 16)    ; //!< Panic detected
                uint32_t                 Reserved753                                      : __CODEGEN_BITFIELD(17, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW23;

        uint32_t                         Reserved768[5];                                                                  //!< Reserved

        union
        {
            //!< DWORD 29
            struct
            {
                uint32_t                 SumSadHaarForBestModeDecisionBottomHalfPopulation                                 ; //!< Sum sad\haar for best mode decision bottom half population
            };
            uint32_t                     Value;
        } DW29;
        union
        {
            //!< DWORD 30
            struct
            {
                uint32_t                 SumSadHaarForBestModeDecisionTopHalfPopulation                                   ; //!< Sum sad\haar for best mode decision top half population
            };
            uint32_t                     Value;
        } DW30;
        union
        {
            //!< DWORD 31
            struct
            {
                uint32_t                 SumTopHalfPopulationOccurrences                  : __CODEGEN_BITFIELD( 0, 15)    ; //!< Sum top half population occurrences
                uint32_t                 SumBottomHalfPopulationOccurrences               : __CODEGEN_BITFIELD(16, 31)    ; //!< Sum bottom half population occurrences
            };
            uint32_t                     Value;
        } DW31;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_HEVC_VP9_FRAME_BASED_STATISTICS_STREAMOUT_CMD();

        static const size_t dwSize = 32;
        static const size_t byteSize = 128;
    };

    //!
    //! \brief VDENC_HEVC_VP9_STREAMIN_STATE
    //! \details
    //!     For the NumMergeCandidate paramaters [64x64/32x32/16x16/8x8], only the
    //!     following configurations are valid.
    //!     Normal Mode without force mv or force intra: 4321 [64x64 --> 16x16].
    //!     Speed Mode without force mv or force intra: 2220, 2110, 1210, 2200, 1110
    //!     [64x64 --> 16x16].
    //!     
    struct VDENC_HEVC_VP9_STREAMIN_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 Roi32X32016X1603                                 : __CODEGEN_BITFIELD( 0,  7)    ; //!< ROI 32x32_0 16x16_03
                uint32_t                 Maxtusize                                        : __CODEGEN_BITFIELD( 8,  9)    ; //!< MaxTUSize
                uint32_t                 Maxcusize                                        : __CODEGEN_BITFIELD(10, 11)    ; //!< MaxCUSize
                uint32_t                 Numimepredictors                                 : __CODEGEN_BITFIELD(12, 15)    ; //!< NUMIMEPREDICTORS
                uint32_t                 PuType32X32016X1603                              : __CODEGEN_BITFIELD(24, 31)    ; //!< PU Type 32x32_0 16x16_03
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 ForceMvX32X32016X160                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< force_mv.x 32x32_0 16x16_0
                uint32_t                 ForceMvY32X32016X160                             : __CODEGEN_BITFIELD(16, 31)    ; //!< force_mv.y 32x32_0 16x16_0
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 ForceMvX32X32016X161                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< force_mv.x 32x32_0 16x16_1
                uint32_t                 ForceMvY32X32016X161                             : __CODEGEN_BITFIELD(16, 31)    ; //!< force_mv.y 32x32_0 16x16_1
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 ForceMvX32X32016X162                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< force_mv.x 32x32_0 16x16_2
                uint32_t                 ForceMvY32X32016X162                             : __CODEGEN_BITFIELD(16, 31)    ; //!< force_mv.y 32x32_0 16x16_2
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 ForceMvX32X32016X163                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< force_mv.x 32x32_0 16x16_3
                uint32_t                 ForceMvY32X32016X163                             : __CODEGEN_BITFIELD(16, 31)    ; //!< force_mv.y 32x32_0 16x16_3
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 Reserved160                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 ForceMvRefidx32X32016X160                        : __CODEGEN_BITFIELD( 0,  3)    ; //!< force_mv refidx 32x32_0 16x16_0
                uint32_t                 ForceMvRefidx32X32016X1613                       : __CODEGEN_BITFIELD( 4, 15)    ; //!< force_mv refidx 32x32_0 16x16_1-3
                uint32_t                 Nummergecandidatecu8X8                           : __CODEGEN_BITFIELD(16, 19)    ; //!< NumMergeCandidateCU8x8
                uint32_t                 Nummergecandidatecu16X16                         : __CODEGEN_BITFIELD(20, 23)    ; //!< NumMergeCandidateCU16x16
                uint32_t                 Nummergecandidatecu32X32                         : __CODEGEN_BITFIELD(24, 27)    ; //!< NumMergeCandidateCU32x32
                uint32_t                 Nummergecandidatecu64X64                         : __CODEGEN_BITFIELD(28, 31)    ; //!< NumMergeCandidateCU64x64
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 Segid32X32016X1603Vp9Only                        : __CODEGEN_BITFIELD( 0, 15)    ; //!< SegID 32x32_0 16x16_03 (VP9 only)
                uint32_t                 QpEn32X32016X1603                                : __CODEGEN_BITFIELD(16, 19)    ; //!< QP_En 32x32_0 16x16_03
                uint32_t                 SegidEnable                                      : __CODEGEN_BITFIELD(20, 20)    ; //!< SegID Enable
                uint32_t                 Reserved245                                      : __CODEGEN_BITFIELD(21, 22)    ; //!< Reserved
                uint32_t                 ForceRefidEnable32X320                           : __CODEGEN_BITFIELD(23, 23)    ; //!< Force Refid Enable (32x32_0)
                uint32_t                 ImePredictorRefidSelect0332X320                  : __CODEGEN_BITFIELD(24, 31)    ; //!< IME predictor/refid Select0-3  32x32_0
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 ImePredictor0X32X320                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< ime_predictor0.x 32x32_0
                uint32_t                 ImePredictor0Y32X320                             : __CODEGEN_BITFIELD(16, 31)    ; //!< ime_predictor0.y 32x32_0
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 ImePredictor0X32X321                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< ime_predictor0.x 32x32_1
                uint32_t                 ImePredictor0Y32X321                             : __CODEGEN_BITFIELD(16, 31)    ; //!< ime_predictor0.y 32x32_1
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 ImePredictor0X32X322                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< ime_predictor0.x 32x32_2
                uint32_t                 ImePredictor0Y32X322                             : __CODEGEN_BITFIELD(16, 31)    ; //!< ime_predictor0.y 32x32_2
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 ImePredictor0X32X323                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< ime_predictor0.x 32x32_3
                uint32_t                 ImePredictor0Y32X323                             : __CODEGEN_BITFIELD(16, 31)    ; //!< ime_predictor0.y 32x32_3
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 ImePredictor0Refidx32X320                        : __CODEGEN_BITFIELD( 0,  3)    ; //!< ime_predictor0 refidx 32x32_0
                uint32_t                 ImePredictor13Refidx32X3213                      : __CODEGEN_BITFIELD( 4, 15)    ; //!< ime_predictor1-3 refidx 32x32_1-3
                uint32_t                 Reserved400                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 Panicmodelcuthreshold                            : __CODEGEN_BITFIELD( 0, 15)    ; //!< PanicModeLCUThreshold
                uint32_t                 Reserved432                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 ForceQpValue16X160                               : __CODEGEN_BITFIELD( 0,  7)    ; //!< Force QP Value 16x16_0
                uint32_t                 ForceQpValue16X161                               : __CODEGEN_BITFIELD( 8, 15)    ; //!< Force QP Value 16x16_1
                uint32_t                 ForceQpValue16X162                               : __CODEGEN_BITFIELD(16, 23)    ; //!< Force QP Value 16x16_2
                uint32_t                 ForceQpValue16X163                               : __CODEGEN_BITFIELD(24, 31)    ; //!< Force QP Value 16x16_3
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

        //! \name Local enumerations

        //! \brief NUMIMEPREDICTORS
        //! \details
        //!     <p>This parameter specifes the number of IME predictors to be processed
        //!     in stage3 IME.</p>
        //!     <p></p>
        enum NUMIMEPREDICTORS
        {
            NUMIMEPREDICTORS_UNNAMED0                                        = 0, //!< No additional details
            NUMIMEPREDICTORS_UNNAMED4                                        = 4, //!< No additional details
            NUMIMEPREDICTORS_UNNAMED8                                        = 8, //!< No additional details
            NUMIMEPREDICTORS_UNNAMED12                                       = 12, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_HEVC_VP9_STREAMIN_STATE_CMD();

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
                uint32_t                 ChromaDownsampleFilterControl                    : __CODEGEN_BITFIELD(20, 22)    ; //!< Chroma Downsample Filter Control
                uint32_t                 Reserved55                                       : __CODEGEN_BITFIELD(23, 26)    ; //!< Reserved
                uint32_t                 SurfaceFormat                                    : __CODEGEN_BITFIELD(27, 31)    ; //!< SURFACE_FORMAT
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
        //!     type of memory tiling (XMajor or YMajor) employed to tile this surface.
        //!     See Memory Interface Functions for details on memory tiling and restrictions.
        //!     This field is ignored when the surface is linear. Internally H/W always
        //!     treats this as set to 1 for all VDEnc usage.
        enum TILE_WALK
        {
            TILE_WALK_XMAJOR                                                 = 0, //!< TILEWALK_XMAJOR
            TILE_WALK_YMAJOR                                                 = 1, //!< TILEWALK_YMAJOR
        };

        //! \brief TILED_SURFACE
        //! \details
        //!     (This field must be set to TRUE: Tiled.) This field specifies whether
        //!     the surface is tiled. This field is ignored by VDEnc usage.
        enum TILED_SURFACE
        {
            TILED_SURFACE_FALSE                                              = 0, //!< Linear
            TILED_SURFACE_TRUE                                               = 1, //!< Tiled
        };

        //! \brief HALF_PITCH_FOR_CHROMA
        //! \details
        //!     (This field must be set to Disable.) This field indicates that the
        //!     chroma plane(s) will use a pitch equal to half the value specified
        //!     in the Surface Pitch field. This field is only used for PLANAR
        //!     surface formats. This field is igored by VDEnc (unless we support YV12).
        enum HALF_PITCH_FOR_CHROMA
        {
            HALF_PITCH_FOR_CHROMA_DISABLE                                    = 0, //!< No additional details
            HALF_PITCH_FOR_CHROMA_ENABLE                                     = 1, //!< No additional details
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
            SURFACE_FORMAT_PLANAR_420_8                                      = 4, //!< (NV12, IMC1,2,3,4, YV12) 
            SURFACE_FORMAT_YCRCB_SWAPY_422                                   = 5, //!< UYVY (8:8:8:8 MSB Y1 V0 Y0 U0)
            SURFACE_FORMAT_YCRCB_SWAPUV_422                                  = 6, //!< YVYU (8:8:8:8 MSB U0 Y1 V0 Y0)
            SURFACE_FORMAT_YCRCB_SWAPUVY_422                                 = 7, //!< VYUY (8:8:8:8 MSB Y1 U0 Y0 V0)
            SURFACE_FORMAT_P010                                              = 8, //!< 10 - bit planar 420 (Tile - Y / Linear / Tile - X)
            SURFACE_FORMAT_RGBA_10_10_10_2                                   = 9, //!<  Need to convert to YUV. 2 bits Alpha, 10 bits R 10 bits G 10 bits B
            SURFACE_FORMAT_Y410                                              = 10, //!< 10 bit 4:4:4 packed
            SURFACE_FORMAT_NV21                                              = 11, //!< 8-bit, same as NV12 but UV interleave is reversed
            SURFACE_FORMAT_P010_VARIANT                                      = 12, //!< >8 bit planar 420 with MSB together and LSB at an offset in x direction
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
    //! \brief VDENC_WEIGHTSOFFSETS_STATE
    //! \details
    //!     
    //!     
    struct VDENC_WEIGHTSOFFSETS_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwLength                                         : __CODEGEN_BITFIELD( 0, 11)    ; //!< DW_LENGTH
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
                uint32_t                 WeightsForwardReference0                         : __CODEGEN_BITFIELD( 0,  7)    ; //!< Weights Forward Reference0
                uint32_t                 OffsetForwardReference0                          : __CODEGEN_BITFIELD( 8, 15)    ; //!< Offset Forward Reference0
                uint32_t                 WeightsForwardReference1                         : __CODEGEN_BITFIELD(16, 23)    ; //!< Weights Forward Reference1
                uint32_t                 OffsetForwardReference1                          : __CODEGEN_BITFIELD(24, 31)    ; //!< Offset Forward Reference1
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 WeightsForwardReference2                         : __CODEGEN_BITFIELD( 0,  7)    ; //!< Weights Forward Reference2
                uint32_t                 OffsetForwardReference2                          : __CODEGEN_BITFIELD( 8, 15)    ; //!< Offset Forward Reference2
                uint32_t                 HevcVp9WeightsBackwardReference0                 : __CODEGEN_BITFIELD(16, 23)    ; //!< HEVC/VP9 Weights Backward Reference0
                uint32_t                 HevcVp9OffsetBackwardReference0                  : __CODEGEN_BITFIELD(24, 31)    ; //!< HEVC/VP9 Offset Backward Reference0
            };
            uint32_t                     Value;
        } DW2;

        //! \name Local enumerations

        //! \brief DW_LENGTH
        //! \details
        //!     Total Length - 2
        enum DW_LENGTH
        {
            DW_LENGTH_DWORDCOUNTN                                            = 1, //!< Excludes DWord (0,1)
        };

        enum SUBOPB
        {
            SUBOPB_VDENCAVCWEIGHTSOFFSETSTATE                                = 8, //!< No additional details
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
        VDENC_WEIGHTSOFFSETS_STATE_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
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
                VDENC_Surface_State_Fields_CMD Dwords69                                                                         ; //!< Dwords 6..9

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

        static const size_t dwSize = 10;
        static const size_t byteSize = 40;
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
                uint32_t                 VdencUltraMode                                   : __CODEGEN_BITFIELD( 5,  5)    ; //!< VDEnc_UltraMode
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
                uint32_t                 Reserved1102                                     : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
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
        //!     <p>To detect if a Time Budget Overflow happened in a frame, SW
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
        //!     transform may be used in this picture. It is set to the value of the
        //!     syntax element in the current active PPS.
        enum TRANSFORM_8X8_FLAG
        {
            TRANSFORM_8X8_FLAG_DISABLED                                      = 0, //!< No 8x8 IDCT Transform, only 4x4 IDCT transform blocks are present.
            TRANSFORM_8X8_FLAG_ENABLED                                       = 1, //!< 8x8 Transform is allowed.
        };

        //! \brief VDENC_L1_CACHE_PRIORITY
        //! \details
        //!     L1 Cache inside VDEnc has 3 clients - IME, CRE and VMC.
        //!     These bits indicate the priority order for L1 cache to
        //!     service the client requests.
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
        //!     This field enables the forward transform calculation for skip check.
        //!     It does not override the other skip calculations but it does decrease
        //!     the performance marginally so don't enable it unless the transform
        //!     is necessary.
        enum FORWARD_TRANSFORM_SKIP_CHECK_ENABLE
        {
            FORWARD_TRANSFORM_SKIP_CHECK_ENABLE_FTDISABLED                   = 0, //!< No additional details
            FORWARD_TRANSFORM_SKIP_CHECK_ENABLE_FTENABLED                    = 1, //!< No additional details
        };

        //! \brief BME_DISABLE_FOR_FBR_MESSAGE
        //! \details
        //!     FBR messages that do not want bidirectional motion estimation
        //!     performed will set this bit and VME will only perform
        //!     fractional refinement on the shapes identified by subpredmode.
        //!     Note: only the LSB of the subpredmode for each shape will be
        //!     considered in FBR (a shape iseither FWD or BWD as input of FBR,
        //!     output however could change toBI if BME is enabled).
        enum BME_DISABLE_FOR_FBR_MESSAGE
        {
            BME_DISABLE_FOR_FBR_MESSAGE_BMEENABLED                           = 0, //!< No additional details
            BME_DISABLE_FOR_FBR_MESSAGE_BMEDISABLED                          = 1, //!< No additional details
        };

        //! \brief BLOCK_BASED_SKIP_ENABLED
        //! \details
        //!     When this field is set on the skip thresholding passing criterion will
        //!     be based on the maximal distortion of individual blocks (8x8's or 4x4's)
        //!     instead of their sum (i.e. the distortion of 16x16).
        enum BLOCK_BASED_SKIP_ENABLED
        {
            BLOCK_BASED_SKIP_ENABLED_UNNAMED0                                = 0, //!< 16x16 Block Based Skip threshold check.
            BLOCK_BASED_SKIP_ENABLED_BLOCK_BASEDSKIPTYPE                     = 1, //!< Parameter indicates 8x8 vs. 4x4 based check.
        };

        //! \brief INTER_SAD_MEASURE_ADJUSTMENT
        //! \details
        //!     This field specifies distortion measure adjustments used
        //!     for the motion search SAD comparison. This field applies
        //!     to both luma and chroma inter measurement.
        enum INTER_SAD_MEASURE_ADJUSTMENT
        {
            INTER_SAD_MEASURE_ADJUSTMENT_NONE                                = 0, //!< No additional details
            INTER_SAD_MEASURE_ADJUSTMENT_HAARTRANSFORMADJUSTED               = 2, //!< No additional details
        };

        //! \brief INTRA_SAD_MEASURE_ADJUSTMENT
        //! \details
        //!     This field specifies distortion measure adjustments used for the motion
        //!     search SAD comparison. This field applies to both luma and chroma
        //!     intra measurement.
        enum INTRA_SAD_MEASURE_ADJUSTMENT
        {
            INTRA_SAD_MEASURE_ADJUSTMENT_NONE                                = 0, //!< No additional details
            INTRA_SAD_MEASURE_ADJUSTMENT_HAARTRANSFORMADJUSTED               = 2, //!< No additional details
        };

        //! \brief SUB_MACROBLOCK_SUB_PARTITION_MASK
        //! \details
        //!     This field defines the bit-mask for disabling
        //!     <ul>
        //!     <li>sub-partition (minor partition [30:28]) modes</li>
        //!     <li>sub-macroblock (major partition [27:24]) modes</li>
        //!     </ul>
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
        //!     distortion of individual blocks (8x8's or 4x4's) instead of
        //!     their sum (i.e. the distortion of 16x16). This field is only valid
        //!     when <b>Block-Based Skip Enabled</b> = 1.
        enum BLOCK_BASED_SKIP_TYPE
        {
            BLOCK_BASED_SKIP_TYPE_UNNAMED0                                   = 0, //!< 4x4 block-based skip threshold check.
            BLOCK_BASED_SKIP_TYPE_UNNAMED1                                   = 1, //!< 8x8 block-based skip threshold check.
        };

        //! \brief CRE_PREFETCH_ENABLE
        //! \details
        //!     This field determines if IME will prefetch the fractional CLs that are
        //!     required by CRE ahead of time while fetching the reference windows around
        //!     the IME predictors. The recommendation for driver is to always program
        //!     this bit to 1 unless some usages restrict SubPelMode to be "<i>Integer
        //!     mode searching</i>".
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
        //!     regular VME call are overlapped and the difference of the locations
        //!     is within this threshold in quarter pixel unit, the two calls
        //!     are merged to a single call.
        enum HME_REF_WINDOWS_COMBINING_THRESHOLD
        {
            HME_REF_WINDOWS_COMBINING_THRESHOLD_UNNAMED0                     = 0, //!< No additional details
            HME_REF_WINDOWS_COMBINING_THRESHOLD_UNNAMED255                   = 255, //!< No additional details
        };

        //! \brief MAXHMVR
        //! \details
        //!     Horizontal MV component range. The MV range is restricted to
        //!     [-MaxHmvR+1, MaxHmvR-1] in luma quarter pel unit,
        //!     which corresponds to [-MaxHmvR/4 + 0.25, MaxHmvR/4-0.25] in luma
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
        //!     Vertical MV component range defined in the AVC Spec Annex A.
        //!     The MV range is restricted to [-MaxVmvR+1, MaxVmvR-1]
        //!     in luma quarter pel unit, which corresponds to
        //!     [-MaxVmvR/4 + 0.25, MaxVmvR/4-0.25] in luma integer pel unit.
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
        union
        {
            VDENC_Down_Scaled_Reference_Picture_CMD DsFwdRef1; //!< DS FWD REF1
            VDENC_Down_Scaled_Reference_Picture_CMD DsBwdRef0; //!< DS BWD REF0
        };
        VDENC_Down_Scaled_Reference_Picture_CMD Reserved; //!< Reserved
        VDENC_Original_Uncompressed_Picture_CMD OriginalUncompressedPicture                                               ; //!< Original Uncompressed Picture
        VDENC_Streamin_Data_Picture_CMD StreaminDataPicture                                                               ; //!< Streamin Data Picture
        VDENC_Row_Store_Scratch_Buffer_Picture_CMD RowStoreScratchBuffer                                                  ; //!< Row Store Scratch Buffer
        VDENC_Colocated_MV_Picture_CMD ColocatedMv                                                                        ; //!< Colocated MV
        VDENC_Reference_Picture_CMD FwdRef0                                                                               ; //!< FWD REF0
        VDENC_Reference_Picture_CMD FwdRef1                                                                               ; //!< FWD REF1
        VDENC_Reference_Picture_CMD FwdRef2                                                                               ; //!< FWD REF2
        VDENC_Reference_Picture_CMD BwdRef0                                                                               ; //!< BWD REF0
        VDENC_Statistics_Streamout_CMD VdencStatisticsStreamout                                                           ; //!< VDEnc Statistics Streamout
        VDENC_Down_Scaled_Reference_Picture_CMD DsFwdRef04X                                                               ; //!< DS FWD REF0 4X
        union
        {
            VDENC_Down_Scaled_Reference_Picture_CMD DsFwdRef14X                                                           ; //!< DS FWD REF1 4X
            VDENC_Down_Scaled_Reference_Picture_CMD DsBwdRef04X                                                           ; //!< DS BWD REF1 4X
        };
        VDENC_Colocated_MV_Picture_CMD VdencCuRecordStreamOutBuffer                                                       ; //!< VDEnc CuRecord stream-out buffer
        VDENC_Colocated_MV_Picture_CMD VdencLcuPakObjCmdBuffer                                                            ; //!< VDEnc LCU PAK OBJ CMD Buffer
        VDENC_Down_Scaled_Reference_Picture_CMD ScaledReferenceSurface8X                                                  ; //!< Scaled Reference Surface 8X
        VDENC_Down_Scaled_Reference_Picture_CMD ScaledReferenceSurface4X                                                  ; //!< Scaled Reference Surface 4X
        VDENC_Colocated_MV_Picture_CMD Vp9SegmentationMapStreaminBuffer                                                   ; //!< VP9 Segmentation Map Streamin Buffer
        VDENC_Colocated_MV_Picture_CMD Vp9SegmentationMapStreamoutBuffer                                                  ; //!< VP9 Segmentation Map Streamout Buffer
        union
        {
            //!< DWORD 61
            struct
            {
                uint32_t                 WeightsHistogramStreamoutOffset                                                  ; //!< Weights Histogram Streamout offset
            };
            uint32_t                     Value;
        } DW61;
        VDENC_Row_Store_Scratch_Buffer_Picture_CMD VdencTileRowStoreBuffer                                                ; //!< VDENC Tile Row store Buffer
        VDENC_Statistics_Streamout_CMD VdencCumulativeCuCountStreamoutSurface                                             ; //!< VDENC Cumulative CU count streamout surface
        VDENC_Statistics_Streamout_CMD VdencPaletteModeStreamoutSurface                                                   ; //!< VDENC Palette Mode streamout surface

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

        static const size_t dwSize = 71;
        static const size_t byteSize = 284;
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
                uint32_t                 ScalabilityMode                                  : __CODEGEN_BITFIELD( 4,  4)    ; //!< Scalability Mode
                uint32_t                 FrameStatisticsStreamOutEnable                   : __CODEGEN_BITFIELD( 5,  5)    ; //!< FRAME_STATISTICS_STREAM_OUT_ENABLE
                uint32_t                 VdencPakObjCmdStreamOutEnable                    : __CODEGEN_BITFIELD( 6,  6)    ; //!< VDEnc PAK_OBJ_CMD Stream-Out Enable
                uint32_t                 TlbPrefetchEnable                                : __CODEGEN_BITFIELD( 7,  7)    ; //!< TLB_PREFETCH_ENABLE
                uint32_t                 PakThresholdCheckEnable                          : __CODEGEN_BITFIELD( 8,  8)    ; //!< PAK_THRESHOLD_CHECK_ENABLE
                uint32_t                 VdencStreamInEnable                              : __CODEGEN_BITFIELD( 9,  9)    ; //!< VDENC_STREAM_IN_ENABLE
                uint32_t                 Downscaled8XWriteDisable                         : __CODEGEN_BITFIELD(10, 10)    ; //!< DownScaled 8x write Disable
                uint32_t                 Downscaled4XWriteDisable                         : __CODEGEN_BITFIELD(11, 11)    ; //!< DownScaled 4x write Disable
                uint32_t                 BitDepth                                         : __CODEGEN_BITFIELD(12, 14)    ; //!< BIT_DEPTH
                uint32_t                 PakChromaSubSamplingType                         : __CODEGEN_BITFIELD(15, 16)    ; //!< PAK_CHROMA_SUB_SAMPLING_TYPE
                uint32_t                 OutputRangeControlAfterColorSpaceConversion      : __CODEGEN_BITFIELD(17, 17)    ; //!< output range control after color space conversion
                uint32_t                 IsRandomAccess                                   : __CODEGEN_BITFIELD(18, 18)    ; //!< Is random access B frame or not
                uint32_t                 Reserved50                                       : __CODEGEN_BITFIELD(19, 19)    ; //!< Reserved
                uint32_t                 RgbEncodingEnable                                : __CODEGEN_BITFIELD(20, 20)    ; //!< RGB encoding enable
                uint32_t                 PrimaryChannelSelectionForRgbEncoding            : __CODEGEN_BITFIELD(21, 22)    ; //!< PRIMARY_CHANNEL_SELECTION_FOR_RGB_ENCODING
                uint32_t                 FirstSecondaryChannelSelectionForRgbEncoding     : __CODEGEN_BITFIELD(23, 24)    ; //!< FIRST_SECONDARY_CHANNEL_SELECTION_FOR_RGB_ENCODING
                uint32_t                 TileReplayEnable                                 : __CODEGEN_BITFIELD(25, 25)    ; //!< Tile replay enable
                uint32_t                 StreamingBufferConfig                            : __CODEGEN_BITFIELD(26, 27)    ; //!< Streaming buffer config
                uint32_t                 Reserved58                                       : __CODEGEN_BITFIELD(28, 30)    ; //!< Reserved
                uint32_t                 DisableSpeedModeFetchOptimization                : __CODEGEN_BITFIELD(31, 31)    ; //!< Disable Speed Mode fetch optimization
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 HmeRegionPreFetchenable                          : __CODEGEN_BITFIELD( 0,  0)    ; //!< HME_REGION_PRE_FETCHENABLE
                uint32_t                 Topprefetchenablemode                            : __CODEGEN_BITFIELD( 1,  2)    ; //!< TOPPREFETCHENABLEMODE
                uint32_t                 LeftpreFetchatwraparound                         : __CODEGEN_BITFIELD( 3,  3)    ; //!< LEFTPRE_FETCHATWRAPAROUND
                uint32_t                 Verticalshift32Minus1                            : __CODEGEN_BITFIELD( 4,  7)    ; //!< VERTICALSHIFT32MINUS1
                uint32_t                 Hzshift32Minus1                                  : __CODEGEN_BITFIELD( 8, 11)    ; //!< HZSHIFT32MINUS1
                uint32_t                 Reserved76                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 NumVerticalReqMinus1                             : __CODEGEN_BITFIELD(16, 19)    ; //!< NUMVERTICALREQMINUS1
                uint32_t                 Numhzreqminus1                                   : __CODEGEN_BITFIELD(20, 23)    ; //!< NUMHZREQMINUS1
                uint32_t                 PreFetchOffsetForReferenceIn16PixelIncrement     : __CODEGEN_BITFIELD(24, 27)    ; //!< PRE_FETCH_OFFSET_FOR_REFERENCE_IN_16_PIXEL_INCREMENT
                uint32_t                 Reserved92                                       : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 SourceLumaPackedDataTlbPreFetchenable            : __CODEGEN_BITFIELD( 0,  0)    ; //!< SOURCE_LUMAPACKED_DATA_TLB_PRE_FETCHENABLE
                uint32_t                 SourceChromaTlbPreFetchenable                    : __CODEGEN_BITFIELD( 1,  1)    ; //!< SOURCE_CHROMA_TLB_PRE_FETCHENABLE
                uint32_t                 Reserved98                                       : __CODEGEN_BITFIELD( 2,  3)    ; //!< Reserved
                uint32_t                 Verticalshift32Minus1Src                         : __CODEGEN_BITFIELD( 4,  7)    ; //!< VERTICALSHIFT32MINUS1SRC
                uint32_t                 Hzshift32Minus1Src                               : __CODEGEN_BITFIELD( 8, 11)    ; //!< HZSHIFT32MINUS1SRC
                uint32_t                 Reserved108                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 Numverticalreqminus1Src                          : __CODEGEN_BITFIELD(16, 19)    ; //!< NUMVERTICALREQMINUS1SRC
                uint32_t                 Numhzreqminus1Src                                : __CODEGEN_BITFIELD(20, 23)    ; //!< NUMHZREQMINUS1SRC
                uint32_t                 PreFetchoffsetforsource                          : __CODEGEN_BITFIELD(24, 27)    ; //!< PRE_FETCHOFFSETFORSOURCE
                uint32_t                 Reserved124                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 Debugtilepassnum                                 : __CODEGEN_BITFIELD( 0,  3)    ; //!< DebugTilePassNum
                uint32_t                 Debugtilenum                                     : __CODEGEN_BITFIELD( 4, 11)    ; //!< DebugTileNum
                uint32_t                 Reserved140                                      : __CODEGEN_BITFIELD(12, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 FrameNumber                                      : __CODEGEN_BITFIELD( 0,  3)    ; //!< Frame Number
                uint32_t                 Reserved164                                      : __CODEGEN_BITFIELD( 4,  9)    ; //!< Reserved
                uint32_t                 HeadPointerUpdateAuto                            : __CODEGEN_BITFIELD(10, 10)    ; //!< Head Pointer Update Auto
                uint32_t                 CaptureMode                                      : __CODEGEN_BITFIELD(11, 12)    ; //!< CAPTURE_MODE
                uint32_t                 ParallelCaptureAndEncodeSessionId                : __CODEGEN_BITFIELD(13, 15)    ; //!< PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID
                uint32_t                 Reserved176                                      : __CODEGEN_BITFIELD(16, 23)    ; //!< Reserved
                uint32_t                 TailPointerReadFrequency                         : __CODEGEN_BITFIELD(24, 31)    ; //!< Tail pointer read frequency
            };
            uint32_t                     Value;
        } DW5;

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
        //!     For AVC standard: This field controls whether VDEnc will check the
        //!     PAK indicator for bits overflow and terminates the slice. This mode is
        //!     called Dynamic Slice Mode. When this field is disabled, VDEnc is in
        //!     Static Slice Mode. It uses the driver programmed Slice Macroblock Height
        //!     Minus One to terminate the slice. This feature is also referred to as
        //!     slice size conformance.
        //!     For HEVC standard: This bit is used to enable dynamic slice size
        //!     control.
        enum PAK_THRESHOLD_CHECK_ENABLE
        {
            PAK_THRESHOLD_CHECK_ENABLE_DISABLESTATICSLICEMODE                = 0, //!< No additional details
            PAK_THRESHOLD_CHECK_ENABLE_ENABLEDYNAMICSLICEMODE                = 1, //!< No additional details
        };

        //! \brief VDENC_STREAM_IN_ENABLE
        //! \details
        //!     This field controls whether VDEnc will read the stream-in surface
        //!     that is programmed. Currently the stream-in surface has MB level QP,
        //!     ROI, predictors and MaxSize/TargetSizeinWordsMB parameters. The
        //!     individual enables for each of the fields is programmed in the
        //!     VDENC_IMG_STATE.
        //!     (ROI_Enable, Fwd/Predictor0 MV Enable, Bwd/Predictor1 MV Enable, MB
        //!     Level QP Enable, TargetSizeinWordsMB/MaxSizeinWordsMB Enable).
        //!     This bit is valid only in AVC mode. In HEVC / VP9 mode this bit is
        //!     reserved and should be set to zero.
        enum VDENC_STREAM_IN_ENABLE
        {
            VDENC_STREAM_IN_ENABLE_DISABLE                                   = 0, //!< No additional details
            VDENC_STREAM_IN_ENABLE_ENABLE                                    = 1, //!< No additional details
        };

        //! \brief BIT_DEPTH
        //! \details
        //!     This parameter indicates the PAK bit depth. The valid values for this
        //!     are 0 / 2 in HEVC / VP9 standard. In AVC standard this field should be
        //!     set to 0.
        enum BIT_DEPTH
        {
            BIT_DEPTH_8BIT                                                   = 0, //!< No additional details
            BIT_DEPTH_10BIT                                                  = 2, //!< No additional details
            BIT_DEPTH_12BIT                                                  = 3, //!< No additional details
        };

        //! \brief PAK_CHROMA_SUB_SAMPLING_TYPE
        //! \details
        //!     This field is applicable only in HEVC and VP9. In AVC, this field is ignored.
        enum PAK_CHROMA_SUB_SAMPLING_TYPE
        {
            PAK_CHROMA_SUB_SAMPLING_TYPE_420                                 = 1, //!< Used for Main8 and Main10 HEVC, VP9 profile0, AVC.
            PAK_CHROMA_SUB_SAMPLING_TYPE_4_4_4                               = 3, //!< HEVC RExt 444, VP9 444 profiles.
        };

        //! \brief PRIMARY_CHANNEL_SELECTION_FOR_RGB_ENCODING
        //! \details
        //!     In RGB encoding, any one of the channel could be primary. This field is
        //!     used for selcting primary channel
        enum PRIMARY_CHANNEL_SELECTION_FOR_RGB_ENCODING
        {
            PRIMARY_CHANNEL_SELECTION_FOR_RGB_ENCODING_UNNAMED0              = 0, //!< Channel R is primary channel
            PRIMARY_CHANNEL_SELECTION_FOR_RGB_ENCODING_UNNAMED1              = 1, //!< Channel G is primary channel.
            PRIMARY_CHANNEL_SELECTION_FOR_RGB_ENCODING_UNNAMED2              = 2, //!< Channel B is primary channel
        };

        //! \brief FIRST_SECONDARY_CHANNEL_SELECTION_FOR_RGB_ENCODING
        //! \details
        //!     In RGB encoding, any one of the channel could be primary. This field is
        //!     used for selcting primary channel
        enum FIRST_SECONDARY_CHANNEL_SELECTION_FOR_RGB_ENCODING
        {
            FIRST_SECONDARY_CHANNEL_SELECTION_FOR_RGB_ENCODING_UNNAMED0      = 0, //!< Channel R is first secondary channel
            FIRST_SECONDARY_CHANNEL_SELECTION_FOR_RGB_ENCODING_UNNAMED1      = 1, //!< Channel G is first secondary channel
            FIRST_SECONDARY_CHANNEL_SELECTION_FOR_RGB_ENCODING_UNNAMED2      = 2, //!< Channel B is first secondary channel.
        };

        //! \brief HME_REGION_PRE_FETCHENABLE
        //! \details
        //!     When this bit is set, for all reference frames HME region pages are pre-fetched.
        enum HME_REGION_PRE_FETCHENABLE
        {
            HME_REGION_PRE_FETCHENABLE_UNNAMED0                              = 0, //!< No additional details
            HME_REGION_PRE_FETCHENABLE_UNNAMED1                              = 1, //!< No additional details
        };

        //! \brief TOPPREFETCHENABLEMODE
        //! \details
        //!     Top Pre-fetch enable Mode
        enum TOPPREFETCHENABLEMODE
        {
            TOPPREFETCHENABLEMODE_UNNAMED1                                   = 1, //!< No additional details
        };

        //! \brief LEFTPRE_FETCHATWRAPAROUND
        //! \details
        //!     Left pre-fetch enabled on wraparound
        enum LEFTPRE_FETCHATWRAPAROUND
        {
            LEFTPRE_FETCHATWRAPAROUND_UNNAMED1                               = 1, //!< No additional details
        };

        enum VERTICALSHIFT32MINUS1
        {
            VERTICALSHIFT32MINUS1_UNNAMED0                                   = 0, //!< No additional details
        };

        //! \brief HZSHIFT32MINUS1
        //! \details
        //!     Horizontal_shift &gt;= LCU_size and Horizontal_shift prefetch_offset
        enum HZSHIFT32MINUS1
        {
            HZSHIFT32MINUS1_UNNAMED3                                         = 3, //!< No additional details
        };

        //! \brief NUMHZREQMINUS1
        //! \details
        //!     Number of Vertical requests in each region for a constant horizontal position.
        enum NUMVERTICALREQMINUS1
        {
            NUMVERTICALREQMINUS1_UNNAMED11                                   = 11, //!< No additional details
        };

        //! \brief NUMHZREQMINUS1
        //! \details
        //!     Number of Horizontal Requests minus 1 at row begining.
        enum NUMHZREQMINUS1
        {
            NUMHZREQMINUS1_UNNAMED2                                          = 2, //!< No additional details
        };

        enum PRE_FETCH_OFFSET_FOR_REFERENCE_IN_16_PIXEL_INCREMENT
        {
            PRE_FETCH_OFFSET_FOR_REFERENCE_IN_16_PIXEL_INCREMENT_UNNAMED0    = 0, //!< No additional details
        };

        //! \brief SOURCE_LUMAPACKED_DATA_TLB_PRE_FETCHENABLE
        //! \details
        //!     When this bit is set, Souce Luma / Packed data TLB pre-fetches are
        //!     performed.
        enum SOURCE_LUMAPACKED_DATA_TLB_PRE_FETCHENABLE
        {
            SOURCE_LUMAPACKED_DATA_TLB_PRE_FETCHENABLE_UNNAMED0              = 0, //!< No additional details
            SOURCE_LUMAPACKED_DATA_TLB_PRE_FETCHENABLE_UNNAMED1              = 1, //!< No additional details
        };

        //! \brief SOURCE_CHROMA_TLB_PRE_FETCHENABLE
        //! \details
        //!     When this bit is set, Souce Chroma TLB pre-fetches are performed.
        enum SOURCE_CHROMA_TLB_PRE_FETCHENABLE
        {
            SOURCE_CHROMA_TLB_PRE_FETCHENABLE_UNNAMED0                       = 0, //!< No additional details
            SOURCE_CHROMA_TLB_PRE_FETCHENABLE_UNNAMED1                       = 1, //!< No additional details
        };

        enum VERTICALSHIFT32MINUS1SRC
        {
            VERTICALSHIFT32MINUS1SRC_UNNAMED0                                = 0, //!< No additional details
        };

        //! \brief HZSHIFT32MINUS1SRC
        //! \details
        //!     Horizontal_shift &gt;= LCU_size and Horizontal_shift prefetch_offset
        enum HZSHIFT32MINUS1SRC
        {
            HZSHIFT32MINUS1SRC_UNNAMED3                                      = 3, //!< No additional details
        };

        //! \brief NUMVERTICALREQMINUS1SRC
        //! \details
        //!     Number of Horizontal requests Minus 1 for source
        enum NUMVERTICALREQMINUS1SRC
        {
            NUMVERTICALREQMINUS1SRC_UNNAMED0                                 = 0, //!< This is the valid for AVC
            NUMVERTICALREQMINUS1SRC_UNNAMED1                                 = 1, //!< This is the valid value for HEVC
        };

        //! \brief NUMHZREQMINUS1SRC
        //! \details
        //!     Number of Horizontal requests Minus 1 for source
        enum NUMHZREQMINUS1SRC
        {
            NUMHZREQMINUS1SRC_UNNAMED0                                       = 0, //!< No additional details
        };

        //! \brief PRE_FETCHOFFSETFORSOURCE
        //! \details
        //!     Pre-fetch offset for Reference in 16 pixel increment.
        enum PRE_FETCHOFFSETFORSOURCE
        {
            PRE_FETCHOFFSETFORSOURCE_UNNAMED_4                               = 4, //!< This value is applicable in HEVC mode
            PRE_FETCHOFFSETFORSOURCE_UNNAMED7                                = 7, //!< This Value is applicable in AVC mode
        };

        enum CAPTURE_MODE
        {
            CAPTURE_MODE_UNNAMED0                                            = 0, //!< No Parallel capture
            CAPTURE_MODE_UNNAMED1                                            = 1, //!< Parallel encode from Display overlay
            CAPTURE_MODE_CAMERA                                              = 2, //!< Parallel encode from Camera Pipe 
            CAPTURE_MODE_UNNAMED3                                            = 3, //!< Reserved 
        };

        enum STREAMING_BUFFER_CONFIG
        {
            STREAMING_BUFFER_UNSUPPORTED                                     = 0,
            STREAMING_BUFFER_64                                              = 1,
            STREAMING_BUFFER_128                                             = 2,
            STREAMING_BUFFER_256                                             = 3,
        };

        enum PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID
        {
            PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID_UNNAMED0                  = 0, //!< Display tailpointer address location 00ED0h-00ED3h 
            PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID_UNNAMED1                  = 1, //!< Display tailpointer address location 00ED4h-00ED7h 
            PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID_UNNAMED2                  = 2, //!< Display tailpointer address location 00ED8h-00EDBh 
            PARALLEL_CAPTURE_AND_ENCODE_SESSION_ID_UNNAMED3                  = 3, //!< Display tailpointer address location 00EDCh-00EDFh 
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_PIPE_MODE_SELECT_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
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
                uint32_t                 SurfaceId                                        : __CODEGEN_BITFIELD(0, 2)    ; //!< Surface ID
                uint32_t                 Reserved                                         : __CODEGEN_BITFIELD(3, 31)   ; //!< Reserved
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
    //!     This command is programmed per super-slice. The X location always needs
    //!     to be programmed to 0. The Y location needs to be programmed to the
    //!     starting point of the current super-slice. The programming needs to
    //!     ensure that all super-slices are contiguous. It is illegal to have gaps
    //!     between the super-slices.
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
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 NextsliceMbStartYPosition                        : __CODEGEN_BITFIELD( 0,  9)    ; //!< NextSlice MB Start Y Position
                uint32_t                 Reserved74                                       : __CODEGEN_BITFIELD(10, 15)    ; //!< Reserved
                uint32_t                 NextsliceMbLcuStartXPosition                     : __CODEGEN_BITFIELD(16, 25)    ; //!< NextSlice MB/LCU Start X Position
                uint32_t                 Reserved90                                       : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 Log2WeightDenomLuma                              : __CODEGEN_BITFIELD( 0,  2)    ; //!< Log 2 Weight Denom Luma
                uint32_t                 Reserved99                                       : __CODEGEN_BITFIELD( 3,  3)    ; //!< Reserved
                uint32_t                 HevcLog2WeightDemonLuma                          : __CODEGEN_BITFIELD( 4,  6)    ; //!< HevcLog2WeightDemonLuma
                uint32_t                 Reserved78                                       : __CODEGEN_BITFIELD( 7,  8)    ; //!< Reserved
                uint32_t                 NumParEngine                                     : __CODEGEN_BITFIELD( 9, 10)    ; //!< NUM_PAR_ENGINE
                uint32_t                 Reserved107                                      : __CODEGEN_BITFIELD(11, 15)    ; //!< Reserved
                uint32_t                 TileRowStoreSelect                               : __CODEGEN_BITFIELD(16, 16)    ; //!< TileRowStoreSelect
                uint32_t                 Reserved108                                      : __CODEGEN_BITFIELD(17, 23)    ; //!< Reserved
                uint32_t                 TileNumber                                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Tile number
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 TileStartCtbY                                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< Tile Start CTB-Y
                uint32_t                 TileStartCtbX                                    : __CODEGEN_BITFIELD(16, 31)    ; //!< Tile Start CTB-X
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 TileWidth                                        : __CODEGEN_BITFIELD( 0, 15)    ; //!< Tile Width
                uint32_t                 TileHeight                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Tile Height
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 StreaminOffsetEnable                             : __CODEGEN_BITFIELD( 0,  0)    ; //!< Streamin Offset enable
                uint32_t                 Reserved193                                      : __CODEGEN_BITFIELD( 1,  5)    ; //!< Reserved
                uint32_t                 TileStreaminOffset                               : __CODEGEN_BITFIELD( 6, 31)    ; //!< Tile Streamin Offset
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 RowStoreOffsetEnable                             : __CODEGEN_BITFIELD( 0,  0)    ; //!< Row store Offset enable
                uint32_t                 Reserved225                                      : __CODEGEN_BITFIELD( 1,  5)    ; //!< Reserved
                uint32_t                 TileRowstoreOffset                               : __CODEGEN_BITFIELD( 6, 31)    ; //!< Tile Rowstore Offset
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 TileStreamoutOffsetEnable                        : __CODEGEN_BITFIELD( 0,  0)    ; //!< Tile streamout offset enable
                uint32_t                 Reserved257                                      : __CODEGEN_BITFIELD( 1,  5)    ; //!< Reserved
                uint32_t                 TileStreamoutOffset                              : __CODEGEN_BITFIELD( 6, 31)    ; //!< Tile streamout offset
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 LcuStreamOutOffsetEnable                         : __CODEGEN_BITFIELD( 0,  0)    ; //!< LCU stream out offset enable
                uint32_t                 Reserved289                                      : __CODEGEN_BITFIELD( 1,  5)    ; //!< Reserved
                uint32_t                 TileLcuStreamOutOffset                           : __CODEGEN_BITFIELD( 6, 31)    ; //!< Tile LCU stream out offset
            };
            uint32_t                     Value;
        } DW9;
        uint32_t DW_Reserved_10_15[6] = {0}; //!< Reserved
        union
        {
            //!< DWORD 16
            struct
            {
                uint32_t                 Reserved518                                      : __CODEGEN_BITFIELD( 0, 23)    ; //!< Reserved
                uint32_t                 AdaptiveChannelThreshold                         : __CODEGEN_BITFIELD(24, 28)    ; //!< Adaptive Channel Threshold
                uint32_t                 Reserved520                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW16;
        uint32_t DW_Reserved_17_26[10] = {0}; //!< Reserved

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

        static const size_t dwSize = 27;
        static const size_t byteSize = 108;
    };

    //!
    //! \brief VDENC_CONTROL_STATE
    //! \details
    //!     
    //!     
    struct VDENC_CONTROL_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Dword Length
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 22)    ; //!< Media Instruction Command
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(23, 26)    ; //!< Media Instruction Opcode
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< Pipeline Type
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Command Type
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 VdencInitialization                              : __CODEGEN_BITFIELD( 1,  1)    ; //!< VDenc Initialization
                uint32_t                 Reserved34                                       : __CODEGEN_BITFIELD( 2, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        VDENC_CONTROL_STATE_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief VDENC_CMD1_CMD
    //! \details
    //!
    //!
    struct VDENC_CMD1_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWord Length
                uint32_t                 Reserved13                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
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
                uint32_t                 Reserved30                                       : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Reserved42                                       : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 Reserved54                                       : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Reserved66                                       : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 Reserved78                                       : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 Reserved90                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 Reserved102                                     : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 Reserved114                                     : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 Reserved126                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 Reserved138                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 Reserved150                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 Reserved162                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 Reserved174                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 Reserved184                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 Reserved198                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            //!< DWORD 16
            struct
            {
                uint32_t                 Reserved210                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            //!< DWORD 17
            struct
            {
                uint32_t                 Reserved222                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            //!< DWORD 18
            struct
            {
                uint32_t                 Reserved231                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            //!< DWORD 19
            struct
            {
                uint32_t                 Reserved246                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            //!< DWORD 20
            struct
            {
                uint32_t                 Reserved258                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            //!< DWORD 21
            struct
            {
                uint32_t                 Reserved270                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW21;
        union
        {
            //!< DWORD 22
            struct
            {
                uint32_t                 Reserved282                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW22;
        union
        {
            //!< DWORD 23
            struct
            {
                uint32_t                 Reserved294                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW23;
        union
        {
            //!< DWORD 24
            struct
            {
                uint32_t                 Reserved306                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW24;
        union
        {
            //!< DWORD 25
            struct
            {
                uint32_t                 Reserved318                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW25;
        union
        {
            //!< DWORD 26
            struct
            {
                uint32_t                 Reserved330                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW26;
        union
        {
            //!< DWORD 27
            struct
            {
                uint32_t                 Reserved342                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW27;
        union
        {
            //!< DWORD 28
            struct
            {
                uint32_t                 Reserved354                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW28;
        union
        {
            //!< DWORD 29
            struct
            {
                uint32_t                 Reserved366                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW29;
        union
        {
            //!< DWORD 30
            struct
            {
                uint32_t                 Reserved378                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW30;

        //! \name Local enumerations

        enum SUBOPB
        {
            SUBOPB_VDENCCMD1CMD                                              = 10, //!< No additional details
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
        VDENC_CMD1_CMD();

        static const size_t dwSize = 31;
        static const size_t byteSize = 124;
    };

    //!
    //! \brief VDENC_CMD2_STATE
    //! \details
    //!
    //!
    struct VDENC_CMD2_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWord Length
                uint32_t                 Reserved13                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
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
                uint32_t                 FrameWidthInPixelsMinusOne                       : __CODEGEN_BITFIELD( 0, 15)    ; //!< FrameWidthInPixelsMinusOne
                uint32_t                 FrameHeightInPixelsMinusOne                      : __CODEGEN_BITFIELD(16, 31)    ; //!< FrameHeightInPixelsMinusOne
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Reserved46                                       : __CODEGEN_BITFIELD( 0, 19)    ; //!< Reserved
                uint32_t                 PictureType                                      : __CODEGEN_BITFIELD(20, 21)    ; //!< Picture Type
                uint32_t                 TemporalMvpEnableFlag                            : __CODEGEN_BITFIELD(22, 22)    ; //!< TemporalMvpEnableFlag
                uint32_t                 Reserved49                                       : __CODEGEN_BITFIELD(23, 23)    ; //!< Reserved
                uint32_t                 LongTermReferenceFlagsL0                         : __CODEGEN_BITFIELD(24, 26)    ; //!< LongTermReferenceFlags_L0
                uint32_t                 LongTermReferenceFlagsL1                         : __CODEGEN_BITFIELD(27, 27)    ; //!< LongTermReferenceFlags_L1
                uint32_t                 Reserved53                                       : __CODEGEN_BITFIELD(28, 29)    ; //!< Reserved
                uint32_t                 TransformSkip                                    : __CODEGEN_BITFIELD(30, 30)    ; //!< TransformSkip
                uint32_t                 Reserved55                                       : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 PocNumberForRefid0InL0                           : __CODEGEN_BITFIELD( 0,  7)    ; //!< FWD_POC_NUMBER_FOR_REFID_0_IN_L0
                uint32_t                 PocNumberForRefid0InL1                           : __CODEGEN_BITFIELD( 8, 15)    ; //!< BWD_POC_NUMBER_FOR_REFID_0_IN_L1
                uint32_t                 PocNumberForRefid1InL0                           : __CODEGEN_BITFIELD(16, 23)    ; //!< POC_NUMBER_FOR_REFID_1_IN_L0
                uint32_t                 PocNumberForRefid1InL1                           : __CODEGEN_BITFIELD(24, 31)    ; //!< POC_NUMBER_FOR_REFID_1_IN_L1
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 PocNumberForRefid2InL0                           : __CODEGEN_BITFIELD( 0,  7)    ; //!< FWD_POC_NUMBER_FOR_REFID_2_IN_L0
                uint32_t                 PocNumberForRefid2InL1                           : __CODEGEN_BITFIELD( 8, 15)    ; //!< BWD_POC_NUMBER_FOR_REFID_2_IN_L1
                uint32_t                 Reserved79                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 Reserved89                                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Reserved
                uint32_t                 StreaminRoiEnable                                : __CODEGEN_BITFIELD( 8,  8)    ; //!< StreamIn ROI Enable
                uint32_t                 Reserved98                                       : __CODEGEN_BITFIELD( 9, 23)    ; //!< Reserved
                uint32_t                 NumRefIdxL0Minus1                                : __CODEGEN_BITFIELD(24, 27)    ; //!< NumRefIdxL0_minus1
                uint32_t                 NumRefIdxL1Minus1                                : __CODEGEN_BITFIELD(28, 31)    ; //!< NumRefIdxL1_minus1
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 Reserved112                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 Reserved121                                      : __CODEGEN_BITFIELD( 0,  3)    ; //!< Reserved
                uint32_t                 SegmentationEnable                               : __CODEGEN_BITFIELD( 4,  4)    ; //!< Segmentation Enable
                uint32_t                 SegmentationMapTemporalPredictionEnable          : __CODEGEN_BITFIELD( 5,  5)    ; //!< Segmentation map temporal prediction enable
                uint32_t                 Reserved124                                      : __CODEGEN_BITFIELD( 6,  6)    ; //!< Reserved
                uint32_t                 TilingEnable                                     : __CODEGEN_BITFIELD( 7,  7)    ; //!< Tiling enable
                uint32_t                 Reserved126                                      : __CODEGEN_BITFIELD( 8,  8)    ; //!< Reserved
                uint32_t                 VdencStreamInEnable                              : __CODEGEN_BITFIELD( 9,  9)    ; //!< VDENC Stream IN
                uint32_t                 Reserved130                                      : __CODEGEN_BITFIELD(10, 15)    ; //!< Reserved
                uint32_t                 PakOnlyMultiPassEnable                           : __CODEGEN_BITFIELD(16, 16)    ; //!< PAK-Only Multi-Pass Enable
                uint32_t                 Reserved139                                      : __CODEGEN_BITFIELD(17, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 Reserved155                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 Reserved170                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 Reserved182                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 Reserved199                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 Reserved209                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 Reserved218                                      : __CODEGEN_BITFIELD( 0,  3)    ; //!< Reserved
                uint32_t                 RoiQpAdjustmentForZone1Stage3                    : __CODEGEN_BITFIELD( 4,  7)    ; //!< ROI QP adjustment for Zone1 (stage3)
                uint32_t                 RoiQpAdjustmentForZone2Stage3                    : __CODEGEN_BITFIELD( 8, 11)    ; //!< ROI QP adjustment for Zone2 (stage3)
                uint32_t                 RoiQpAdjustmentForZone3Stage3                    : __CODEGEN_BITFIELD(12, 15)    ; //!< ROI QP adjustment for Zone3 (stage3)
                uint32_t                 Reserved225                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 Reserved238                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 Reserved248                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            //!< DWORD 16
            struct
            {
                uint32_t                 MinQp                                            : __CODEGEN_BITFIELD( 0,  7)    ; //!< MINQP
                uint32_t                 MaxQp                                            : __CODEGEN_BITFIELD( 8, 15)    ; //!< MAXQP
                uint32_t                 Reserved262                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            //!< DWORD 17
            struct
            {
                uint32_t                 Reserved271                                      : __CODEGEN_BITFIELD( 0, 19)    ; //!< Reserved
                uint32_t                 TemporalMVEnableForIntegerSearch                 : __CODEGEN_BITFIELD(20, 20)    ; //!< Temporal MV disable for Integer Search
                uint32_t                 Reserved273                                      : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            //!< DWORD 18
            struct
            {
                uint32_t                 Reserved286                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW18;
        union
        {
            //!< DWORD 19
            struct
            {
                uint32_t                 Reserved298                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            //!< DWORD 20
            struct
            {
                uint32_t                 Reserved312                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            //!< DWORD 21
            struct
            {
                uint32_t                 IntraRefreshPos                                  : __CODEGEN_BITFIELD( 0,  8)    ; //!< IntraRefreshPos
                uint32_t                 Reserved322                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved
                uint32_t                 IntraRefreshMBSizeMinusOne                       : __CODEGEN_BITFIELD(16, 23)    ; //!< IntraRefreshMBSizeMinusOne
                uint32_t                 IntraRefreshMode                                 : __CODEGEN_BITFIELD(24, 24)    ; //!< IntraRefreshMode
                uint32_t                 Reserved326                                      : __CODEGEN_BITFIELD(25, 27)    ; //!< Reserved
                uint32_t                 QpAdjustmentForRollingI                          : __CODEGEN_BITFIELD(28, 31)    ; //!< QP_ADJUSTMENT_FOR_ROLLING_I
            };
            uint32_t                     Value;
        } DW21;
        union
        {
            //!< DWORD 22
            struct
            {
                uint32_t                 Reserved343                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW22;
        union
        {
            //!< DWORD 23
            struct
            {
                uint32_t                 Reserved357                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW23;
        union
        {
            //!< DWORD 24
            struct
            {
                uint32_t                 QpForSeg0                                        : __CODEGEN_BITFIELD( 0,  7)    ; //!< QP for Seg0
                uint32_t                 QpForSeg1                                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< QP for Seg1
                uint32_t                 QpForSeg2                                        : __CODEGEN_BITFIELD(16, 23)    ; //!< QP for Seg2
                uint32_t                 QpForSeg3                                        : __CODEGEN_BITFIELD(24, 31)    ; //!< QP for Seg3
            };
            uint32_t                     Value;
        } DW24;
        union
        {
            //!< DWORD 25
            struct
            {
                uint32_t                 QpForSeg4                                        : __CODEGEN_BITFIELD( 0,  7)    ; //!< QP for Seg4
                uint32_t                 QpForSeg5                                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< QP for Seg5
                uint32_t                 QpForSeg6                                        : __CODEGEN_BITFIELD(16, 23)    ; //!< QP for Seg6
                uint32_t                 QpForSeg7                                        : __CODEGEN_BITFIELD(24, 31)    ; //!< QP for Seg7
            };
            uint32_t                     Value;
        } DW25;
        union
        {
            //!< DWORD 26
            struct
            {
                uint32_t                 RdQpLambda                                       : __CODEGEN_BITFIELD( 0, 15)    ; //!< RD QP Lambda
                uint32_t                 SadQpLambda                                      : __CODEGEN_BITFIELD(16, 24)    ; //!< SAD QP Lambda
                uint32_t                 Vp9DynamicSliceEnable                            : __CODEGEN_BITFIELD(25, 25)    ; //!< VP9 Dynamic slice enable
                uint32_t                 Reserved394                                      : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW26;
        union
        {
            //!< DWORD 27
            struct
            {
                uint32_t                 QpPrimeYDc                                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< QPPRIMEY_DC
                uint32_t                 QpPrimeYAc                                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< QPPRIMEY_AC
                uint32_t                 Reserved405                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW27;
        union
        {
            //!< DWORD 28
            struct
            {
                uint32_t                 Reserved415                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW28;
        union
        {
            //!< DWORD 29
            struct
            {
                uint32_t                 Reserved425                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW29;
        union
        {
            //!< DWORD 30
            struct
            {
                uint32_t                 Reserved435                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW30;
        union
        {
            //!< DWORD 31
            struct
            {
                uint32_t                 Reserved445                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW31;
        union
        {
            //!< DWORD 32
            struct
            {
                uint32_t                 Reserved454                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW32;
        union
        {
            //!< DWORD 33
            struct
            {
                uint32_t                 Reserved474                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW33;
        union
        {
            //!< DWORD 34
            struct
            {
                uint32_t                 Reserved490                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW34;
        union
        {
            //!< DWORD 35
            struct
            {
                uint32_t                 Reserved503                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW35;
        union
        {
            //!< DWORD 36
            struct
            {
                uint32_t                 IntraRefreshBoundaryRef0                         : __CODEGEN_BITFIELD( 0,  8)    ; //!< IntraRefreshBoundary Ref0
                uint32_t                 Reserved513                                      : __CODEGEN_BITFIELD( 9,  9)    ; //!< Reserved
                uint32_t                 IntraRefreshBoundaryRef1                         : __CODEGEN_BITFIELD(10, 18)    ; //!< IntraRefreshBoundary Ref1
                uint32_t                 Reserved515                                      : __CODEGEN_BITFIELD(19, 19)    ; //!< Reserved
                uint32_t                 IntraRefreshBoundaryRef2                         : __CODEGEN_BITFIELD(20, 28)    ; //!< IntraRefreshBoundary Ref2
                uint32_t                 Reserved517                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW36;
        union
        {
            //!< DWORD 37
            struct
            {
                uint32_t                 Reserved530                                      : __CODEGEN_BITFIELD( 0, 22)    ; //!< Reserved
                uint32_t                 Reserved533                                      : __CODEGEN_BITFIELD(23, 26)    ; //!< Reserved
                uint32_t                 TileReplayEnable                                 : __CODEGEN_BITFIELD(27, 27)    ; //!< Tile Replay enable
                uint32_t                 Reserved536                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW37;
        union
        {
            //!< DWORD 38
            struct
            {
                uint32_t                 Reserved551                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW38;
        union
        {
            //!< DWORD 39
            struct
            {
                uint32_t                 Reserved561                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW39;
        union
        {
            //!< DWORD 40
            struct
            {
                uint32_t                 Reserved571                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW40;
        union
        {
            //!< DWORD 41
            struct
            {
                uint32_t                 Reserved581                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW41;
        union
        {
            //!< DWORD 42
            struct
            {
                uint32_t                 Reserved591                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW42;
        union
        {
            //!< DWORD 43
            struct
            {
                uint32_t                 Reserved601                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW43;
        union
        {
            //!< DWORD 44
            struct
            {
                uint32_t                 Reserved611                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW44;
        union
        {
            //!< DWORD 45
            struct
            {
                uint32_t                 Reserved621                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW45;
        union
        {
            //!< DWORD 46
            struct
            {
                uint32_t                 Reserved631                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW46;
        union
        {
            //!< DWORD 47
            struct
            {
                uint32_t                 Reserved641                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW47;
        union
        {
            //!< DWORD 48
            struct
            {
                uint32_t                 Reserved651                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW48;
        union
        {
            //!< DWORD 49
            struct
            {
                uint32_t                 Reserved661                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW49;
        union
        {
            //!< DWORD 50
            struct
            {
                uint32_t                 Reserved671                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW50;


        //! \name Local enumerations

        enum SUBOPB
        {
            SUBOPB_VDENCCMD2CMD                                              = 9, //!< No additional details
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

        //! \brief Explicit member initialization function
        VDENC_CMD2_CMD();

        static const size_t dwSize = 51;
        static const size_t byteSize = 204;
    };
};

#pragma pack()

#endif  // __MHW_VDBOX_VDENC_HWCMD_G12_X_H__
