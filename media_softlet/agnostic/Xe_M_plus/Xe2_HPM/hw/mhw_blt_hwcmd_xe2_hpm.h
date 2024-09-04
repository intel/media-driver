/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     mhw_blt_hwcmd_XE2_HPM_next.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of XE2_HPM as other components
//!           should use MHW interface to interact with MHW commands and states.
//!

#ifndef __MHW_BLT_HWCMD_XE2_HPM_NEXT_H__
#define __MHW_BLT_HWCMD_XE2_HPM_NEXT_H__

#pragma once
#pragma pack(1)

#include <cstdint>
#include <cstddef>
#include "media_class_trace.h"

namespace mhw
{
namespace blt
{
namespace xe2_hpm
{
class Cmd
{
public:
    // Internal Macros
#define __CODEGEN_MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
#define __CODEGEN_BITFIELD(l, h) (h) - (l) + 1
    //////////////////////////////////////////////////////////////////////////
    /// @brief XY_BLOCK_COPY_BLT
    /// @details
    ///     XY_BLOCK_COPY_BLT instruction performs a color source copy where the
    ///     only operands involved are a color source and destination of the same
    ///     bit width. The source and destination surfaces CAN overlap, the hardware
    ///     handles this internally. Legacy blit commands (2D BLT instructions other
    ///     than XY_BLOCK_COPY_BLT, XY_FAST_COPY_BLT, XY_FAST_COLOR_BLT) and this
    ///     new copy command can be interspersed. No implied flush required between
    ///     the two provided there is no producer consumer relationship between the
    ///     two.The starting pixel of the blit operation for both source and
    ///     destination should be on a pixel boundary. This command now supports
    ///     copy of compressed surface.
    ///       In case of producer consumer relationship between a legacy blitter
    ///     command and anew copy command a flush must be inserted between the two
    ///     by software.
    ///
    struct XY_BLOCK_COPY_BLT_CMD
    {
        union
        {
            struct
            {
                uint32_t         DWordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; ///< U8
                uint32_t         Reserved8                                        : __CODEGEN_BITFIELD( 8,  8)    ; ///< U1
                uint32_t         NumberofMultisamples                             : __CODEGEN_BITFIELD( 9, 11)    ; ///< U3
                uint32_t         SpecialModeofOperation                           : __CODEGEN_BITFIELD(12, 13)    ; ///< U2
                uint32_t         Reserved14                                       : __CODEGEN_BITFIELD(14, 18)    ; ///< U5
                uint32_t         ColorDepth                                       : __CODEGEN_BITFIELD(19, 21)    ; ///< U3
                uint32_t         InstructionTargetOpcode                          : __CODEGEN_BITFIELD(22, 28)    ; ///< U7
                uint32_t         Client                                           : __CODEGEN_BITFIELD(29, 31)    ; ///< U3
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t         DestinationPitch                                 : __CODEGEN_BITFIELD( 0, 17)    ; ///< U18
                uint32_t         Reserved18                                       : __CODEGEN_BITFIELD(18, 20)    ; ///< U3
                uint32_t         DestinationEncryptEn                             : __CODEGEN_BITFIELD(21, 21)    ; ///< U7
                uint32_t         Reserved22                                       : __CODEGEN_BITFIELD(22, 23)    ; ///< U4
                uint32_t         DestinationMocsValue                             : __CODEGEN_BITFIELD(24, 27)    ; ///< U7
                uint32_t         Reserved28                                       : __CODEGEN_BITFIELD(28, 28);                                           ///< U1
                uint32_t         Reserved29                                       : __CODEGEN_BITFIELD(29, 29);                                           ///< U1
                uint32_t         DestinationTiling                                : __CODEGEN_BITFIELD(30, 31)    ; ///< U2
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t         DestinationX1CoordinateLeft                     : __CODEGEN_BITFIELD(0, 15); ///< U16
                uint32_t         DestinationY1CoordinateTop                      : __CODEGEN_BITFIELD(16, 31); ///< U16
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t         DestinationX2CoordinateRight                    : __CODEGEN_BITFIELD( 0, 15)    ; ///< U16
                uint32_t         DestinationY2CoordinateBottom                   : __CODEGEN_BITFIELD(16, 31)    ; ///< U16
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint64_t         DestinationBaseAddress;                                                            //!< U64
            };
             uint32_t                     Value[2];
        } DW4_5;
        union
        {
            struct
            {
                uint32_t         DestinationXOffset                               : __CODEGEN_BITFIELD(0, 13); ///< U14
                uint32_t         Reserved206                                      : __CODEGEN_BITFIELD(14, 15); ///< U2
                uint32_t         DestinationYOffset                               : __CODEGEN_BITFIELD(16, 29); ///< U14
                uint32_t         Reserved222                                      : __CODEGEN_BITFIELD(30, 30); ///< U1
                uint32_t         DestinationTargetMemory                          : __CODEGEN_BITFIELD(31, 31); ///< U1
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            struct
            {
                uint32_t         SourceX1CoordinateLeft                          : __CODEGEN_BITFIELD( 0, 15)    ; ///< U16
                uint32_t         SourceY1CoordinateTop                           : __CODEGEN_BITFIELD(16, 31)    ; ///< U16
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            struct
            {
                uint32_t         SourcePitch                                     : __CODEGEN_BITFIELD(0, 17); ///< U18
                uint32_t         Reserved18                                      : __CODEGEN_BITFIELD(18, 20);                                      ///< U3
                uint32_t         SourceEncrypEn                                  : __CODEGEN_BITFIELD(21, 21); ///< U1
                uint32_t         Reserved2                                       : __CODEGEN_BITFIELD(22, 23); ///< U2
                uint32_t         SourceMocs                                      : __CODEGEN_BITFIELD(24, 27); ///< U4
                uint32_t         Reserved28                                      : __CODEGEN_BITFIELD(28, 28);                                       ///< U1
                uint32_t         Reserved29                                      : __CODEGEN_BITFIELD(29, 29);                                       ///< U1
                uint32_t         SourceTiling                                    : __CODEGEN_BITFIELD(30, 31); ///< U2
            };
            uint32_t                     Value;
        } DW8;
        union
        {
         struct
         {
          uint64_t                 SourceBaseAddress; //!< U64
         };
         uint32_t                     Value[2];
        } DW9_10;
        union
        {
            struct
            {
                uint32_t         SourceXoffset                                    : __CODEGEN_BITFIELD( 0, 13)    ; ///< U14
                uint32_t         Reserved366                                     : __CODEGEN_BITFIELD(14, 15)    ; ///< U2
                uint32_t         SourceYoffset                                    : __CODEGEN_BITFIELD(16, 29)    ; ///< U14
                uint32_t         Reserved382                                     : __CODEGEN_BITFIELD(30, 30)    ; ///< U1
                uint32_t         SourceTargetMemory                               : __CODEGEN_BITFIELD(31, 31)    ; ///< U1
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            struct
            {
                uint32_t         Reserved0                                        : __CODEGEN_BITFIELD(0, 3);   ///< U4
                uint32_t         Reserved4                                        : __CODEGEN_BITFIELD(4, 31); ///< U28
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            struct
            {
                uint32_t         Reserved441                                      : __CODEGEN_BITFIELD(0, 31)    ; ///< U32
            };
            uint32_t                     Value;
        } DW13;
        union
        {
             struct
             {
                uint32_t         Reserved0                                        : __CODEGEN_BITFIELD(0, 3);   ///< U4
                uint32_t         Reserved4                                        : __CODEGEN_BITFIELD( 4,  31)    ; ///< U28
             };
             uint32_t                     Value;
        } DW14;
        union
        {
             struct
             {
                uint32_t         Reserved505                                     : __CODEGEN_BITFIELD(0, 31)    ; ///< U32
             };
             uint32_t                     Value;
        } DW15;
        union
        {
             struct
             {
                uint32_t         DestinationSurfaceHeight                         : __CODEGEN_BITFIELD( 0, 13)    ; ///< U14
                uint32_t         DestinationSurfaceWidth                          : __CODEGEN_BITFIELD(14, 27)    ; ///< U14
                uint32_t         Reserved540                                      : __CODEGEN_BITFIELD(28, 28)    ; ///< U1
                uint32_t         DestinationSurfaceType                           : __CODEGEN_BITFIELD(29, 31)    ; ///< U3
             };
             uint32_t                     Value;
        } DW16;
        union
        {
             struct
             {
                 uint32_t         DestinationLOD                                  : __CODEGEN_BITFIELD(0, 3); ///< U4
                 uint32_t         DestinationSurfaceQpitch                        : __CODEGEN_BITFIELD(4, 18); ///< U15
                 uint32_t         Reserved563                                    : __CODEGEN_BITFIELD(19, 20); ///< U2
                 uint32_t         DestinationSurfaceDepth                         : __CODEGEN_BITFIELD(21, 31); ///< U11
             };
             uint32_t                     Value;
        } DW17;
        union
        {
             struct
             {
                uint32_t         DestinationHorizontalAlign                       : __CODEGEN_BITFIELD( 0,  1)    ; ///< U2
                uint32_t         Reserved578                                      : __CODEGEN_BITFIELD( 2,  2)    ; ///< U1
                uint32_t         DestinationVerticalAlign                         : __CODEGEN_BITFIELD( 3,  4)    ; ///< U2
                uint32_t         Reserved581                                      : __CODEGEN_BITFIELD( 5,  7)    ; ///< U3
                uint32_t         DestinationMipTailStartLOD                       : __CODEGEN_BITFIELD( 8, 11)    ; ///< U4
                uint32_t         Reserved588                                      : __CODEGEN_BITFIELD(12, 17)    ; ///< U6
                uint32_t         Reserved18                                       : __CODEGEN_BITFIELD(18, 18);                                          ///< U1
                uint32_t         Reserved595                                      : __CODEGEN_BITFIELD(19, 20)    ; ///< U2
                uint32_t         DestinationArrayIndex                            : __CODEGEN_BITFIELD(21, 31)    ; ///< U11
             };
             uint32_t                     Value;
        } DW18;
        union
        {
            struct
            {
               uint32_t         SourceSurfaceHeight                              : __CODEGEN_BITFIELD( 0, 13)    ; ///< U14
               uint32_t         SourceSurfaceWidth                               : __CODEGEN_BITFIELD(14, 27)    ; ///< U14
               uint32_t         Reserved636                                      : __CODEGEN_BITFIELD(28, 28)    ; ///< U1
               uint32_t         SourceSurfaceType                                : __CODEGEN_BITFIELD(29, 31)    ; ///< U3
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            struct
            {
               uint32_t         SourceLOD                                        : __CODEGEN_BITFIELD( 0,  3)    ; ///< U4
               uint32_t         SourceSurfaceQpitch                              : __CODEGEN_BITFIELD( 4, 18)    ; ///< U15
               uint32_t         Reserved659                                      : __CODEGEN_BITFIELD(19, 20)    ; ///< U2
               uint32_t         SourceSurfaceDepth                               : __CODEGEN_BITFIELD(21, 31)    ; ///< U11
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            struct
            {
               uint32_t         SourceHorizontalAlign                            : __CODEGEN_BITFIELD( 0,  1)    ; ///< U2
               uint32_t         Reserved674                                      : __CODEGEN_BITFIELD( 2,  2)    ; ///< U1
               uint32_t         SourceVerticalAlign                              : __CODEGEN_BITFIELD( 3,  4)    ; ///< U2
               uint32_t         Reserved677                                      : __CODEGEN_BITFIELD( 5,  7)    ; ///< U3
               uint32_t         SourceMipTailStartLOD                            : __CODEGEN_BITFIELD( 8, 11)    ; ///< U4
               uint32_t         Reserved684                                      : __CODEGEN_BITFIELD(12, 17)    ; ///< U6
               uint32_t         Reserved18                                       : __CODEGEN_BITFIELD(18, 18);                                          ///< U1
               uint32_t         Reserved691                                      : __CODEGEN_BITFIELD(19, 20)    ; ///< U2
               uint32_t         SourceArrayIndex                                 : __CODEGEN_BITFIELD(21, 31)    ; ///< U11
            };
            uint32_t                     Value;
        } DW21;

         //////////////////////////////////////////////////////////////////////////
        /// @name LOCAL ENUMERATIONS
        /// @{

        /// @brief U8
        enum DWORD_LENGTH
        {
            DWORD_LENGTH_EXCLUDES_DWORD_0_1 = 20, ///<
        };

        /// @brief U3
        enum NUMBER_OF_MULTISAMPLES
        {
            NUMBER_OF_MULTISAMPLES_MULTISAMPLECOUNT_1 = 0, ///<
            NUMBER_OF_MULTISAMPLES_MULTISAMPLECOUNT_2 = 1, ///<
            NUMBER_OF_MULTISAMPLES_MULTISAMPLECOUNT_4 = 2, ///<
            NUMBER_OF_MULTISAMPLES_MULTISAMPLECOUNT_8 = 3, ///<
            NUMBER_OF_MULTISAMPLES_MULTISAMPLECOUNT_16 = 4, ///<
        };

        /// @brief U2
        enum SPECIAL_MODE_OF_OPERATION
        {
            SPECIAL_MODE_OF_OPERATION_NONE = 0, ///< No special mode. It will act as regular copy command.
            SPECIAL_MODE_OF_OPERATION_FULL_RESOLVE = 1, ///< In-place resolve to get rid of 128B blocks from clear or compression state.
            SPECIAL_MODE_OF_OPERATION_PARTIAL_RESOLVE = 2, ///< Partial resolve is for resolving the surface for clear values. If the surface is compressed it keeps it compressed, no implied clear values.
        };

        /// @brief U3
        enum COLOR_DEPTH
        {
            COLOR_DEPTH_8BITCOLOR  = 0, //!< No additional details
            COLOR_DEPTH_16BITCOLOR = 1, //!< No additional details
            COLOR_DEPTH_32BITCOLOR = 2, //!< No additional details
            COLOR_DEPTH_64BITCOLOR = 3, //!< No additional details
            COLOR_DEPTH_96BITCOLOR_ONLYLINEARCASEISSUPPORTED = 4, //!< No additional details
            COLOR_DEPTH_128BITCOLOR = 5, //!< No additional details
        };

        /// @brief U3
        enum CLIENT
        {
            CLIENT_2D_PROCESSOR = 2, ///<
        };

        /// @brief U3
        enum DESTINATION_AUXILIARY_SURFACE_MODE
        {
            DESTINATION_AUXILIARY_SURFACE_MODE_AUX_NONE = 0, ///< No Auxiliary surface used
            DESTINATION_AUXILIARY_SURFACE_MODE_AUX_CCS_E = 5, ///Auxiliary surface is a CCS with lossless compression enabled when number of multisamples is 1. When number of multisamples > 1, programming this value means MSAA compression enabled.
        };

        /// @brief U1
        enum DESTINATION_CONTROL_SURFACE_TYPE
        {
            DESTINATION_CONTROL_SURFACE_TYPE_3D_CONTROL_SURFACE = 0, ///< Control Surface type is 3D.
            DESTINATION_CONTROL_SURFACE_TYPE_MEDIA_CONTROL_SURFACE = 1, ///< Control Surface type is media.
        };

        /// @brief U1
        enum DESTINATION_COMPRESSION_ENABLE
        {
            DESTINATION_COMPRESSION_ENABLE_COMPRESSION_DISABLE = 0, ///< Enables uncompressed write operation to destination surface.
            DESTINATION_COMPRESSION_ENABLE_COMPRESSION_ENABLE = 1, ///< Enables compressed write operation to destination surface provided special mode of operation is not FULL_RESOLVE.
        };

        /// @brief U2
        enum DESTINATION_TILING
        {
            DESTINATION_TILING_LINEAR = 0, ///< Linear mode (no tiling)
            DESTINATION_TILING_XMAJOR = 1, ///< X major Tiling
            DESTINATION_TILING_TILE4 = 2, ///< Tile4 4KB tiling
            DESTINATION_TILING_TILE64 = 3, ///< Tile64 64KB tiling
        };

        /// @brief U1
        enum DESTINATION_TARGET_MEMORY
        {
            DESTINATION_TARGET_MEMORY_LOCAL_MEM = 0, ///< Traget memory is local memory.
            DESTINATION_TARGET_MEMORY_SYSTEM_MEM = 1, ///< Target memory is system memory.
        };

        /// @brief U3
        enum SOURCE_AUXILIARY_SURFACE_MODE
        {
            SOURCE_AUXILIARY_SURFACE_MODE_AUX_NONE = 0, ///< No Auxiliary surface used
            SOURCE_AUXILIARY_SURFACE_MODE_AUX_CCS_E = 5, ///< Auxiliary surface is a CCS with lossless compression enabled when number of multisamples is 1. When number of multisamples > 1, programming this value means MSAA compression enabled.
        };

        /// @brief U1
        enum SOURCE_CONTROL_SURFACE_TYPE
        {
            SOURCE_CONTROL_SURFACE_TYPE_3D_CONTROL_SURFACE = 0, ///< Control Surface type is 3D.
            SOURCE_CONTROL_SURFACE_TYPE_MEDIA_CONTROL_SURFACE = 1, ///< Control Surface type is media
        };

        /// @brief U1
        enum SOURCE_COMPRESSION_ENABLE
        {
            SOURCE_COMPRESSION_ENABLE_COMPRESSION_DISABLE = 0, ///<
            SOURCE_COMPRESSION_ENABLE_COMPRESSION_ENABLE = 1, ///<
        };

        /// @brief U2
        enum SOURCE_TILING
        {
            SOURCE_TILING_LINEAR = 0, ///< Linear Tiling (tiking disabled)
            SOURCE_TILING_XMAJOR = 1, ///< X major tiling
            SOURCE_TILING_TILE4 = 2, ///< Tile4 4KB tiling
            SOURCE_TILING_TILE64 = 3, ///< Tile64 64KB tiling
        };

        /// @brief U1
        enum SOURCE_TARGET_MEMORY
        {
            SOURCE_TARGET_MEMORY_LOCAL_MEM = 0, ///< Target memory is local memory.
            SOURCE_TARGET_MEMORY_SYSTEM_MEM = 1, ///< Target memory is system memory.
        };

        /// @brief U1
        enum SOURCE_CLEAR_VALUE_ENABLE
        {
            SOURCE_CLEAR_VALUE_ENABLE_DISABLE = 0, ///<
            SOURCE_CLEAR_VALUE_ENABLE_ENABLE = 1, ///<
        };

        /// @brief U1
        enum DESTINATION_CLEAR_VALUE_ENABLE
        {
            DESTINATION_CLEAR_VALUE_ENABLE_DISABLE = 0, ///<
            DESTINATION_CLEAR_VALUE_ENABLE_ENABLE = 1, ///<
        };

        /// @brief U3
        enum DESTINATION_SURFACE_TYPE
        {
            DESTINATION_SURFACE_TYPE_SURFTYPE_1D = 0, ///< Defines a 1-dimensional map or array of maps
            DESTINATION_SURFACE_TYPE_SURFTYPE_2D = 1, ///< Defines a 2-dimensional map or array of maps
            DESTINATION_SURFACE_TYPE_SURFTYPE_3D = 2, ///< Defines a 3-dimensional (volumetric) map
            DESTINATION_SURFACE_TYPE_SURFTYPE_CUBE = 3, ///< Defines a cube map or array of cube maps.
        };

        /// @brief U3
        enum SOURCE_SURFACE_TYPE
        {
            SOURCE_SURFACE_TYPE_SURFTYPE_1D = 0, ///< Defines a 1-dimensional map or array of maps
            SOURCE_SURFACE_TYPE_SURFTYPE_2D = 1, ///< Defines a 2-dimensional map or array of maps
            SOURCE_SURFACE_TYPE_SURFTYPE_3D = 2, ///< Defines a 1-dimensional (volumetric) map.
            SOURCE_SURFACE_TYPE_SURFTYPE_CUBE = 3, ///< Defines a cube map or array of cube maps.
        };
        //! \name Initializations

        //! \brief Explicit member initialization function
        XY_BLOCK_COPY_BLT_CMD()
        {
            DW0.Value                                            = 0x50400014;
            //DW0.DwordLength                                    = DWORD_LENGTH_EXCLUDES_DWORD_0_1;
            //DW0.NumberOfMultisamples                           = NUMBER_OF_MULTISAMPLES_MULTISAMPLECOUNT_1;
            //DW0.SpecialModeofOperation                         = SPECIAL_MODE_OF_OPERATION_NONE;
            //DW0.ColorDepth                                     = COLOR_DEPTH_8_BIT_COLOR;
            //DW0.InstructionTarget_Opcode                       = 0x0;
            //DW0.Client                                         = CLIENT_2D_PROCESSOR;
            DW1.Value                                            = 0x0;
            //DW1.DestinationPitch                               = 0x0;
            //DW1.DestinationMOCSvalue                           = 0x0;
            //DW1.DestinationTiling                              = DESTINATION_TILING_LINEAR;
            DW2.Value                                            = 0x0;
            //DW2.DestinationX1Coordinate_Left                   = 0x0;
            //DW2.DestinationY1Coordinate_Top                    = 0x0;
            DW3.Value                                            = 0x0;
            //DW3.DestinationX2Coordinate_Right                  = 0x0;
            //DW3.DestinationY2Coordinate_Bottom                 = 0x0;
            DW4_5.Value[0] = DW4_5.Value[1]                      = 0x0;
            //DW4_5.DestinationBaseAddress                         = 0x0;
            DW6.Value                                            = 0x0;
            //DW6.DestinationXoffset                             = 0x0;
            //DW6.DestinationYoffset                             = 0x0;
            //DW6.DestinationTargetMemory                        = DESTINATION_TARGET_MEMORY_LOCAL_MEM;
            DW7.Value                                            = 0x0;
            //DW7.SourceX1Coordinate_Left                        = 0x0;
            //DW7.SourceY1Coordinate_Top                         = 0x0;
            DW8.Value                                            = 0x0;
            //DW8.SourcePitch                                    = 0x0;
            //DW8.SourceMOCS                                     = 0x0;
            //DW8.SourceTiling                                   = SOURCE_TILING_LINEAR;
            DW9_10.Value[0] = DW9_10.Value[1]                    = 0x0;
            //DW9.SourceBaseAddress                              = 0x0;
            DW11.Value                                            = 0x0;
            //DW11.SourceXoffset                                  = 0x0;
            //DW11.SourceYoffset                                  = 0x0;
            //DW11.SourceTargetMemory                             = SOURCE_TARGET_MEMORY_LOCAL_MEM;
            DW12.Value                                            = 0x0;
            DW13.Value                                            = 0x0;
            DW14.Value                                            = 0x0;
            DW15.Value                                            = 0x0;
            //DW15.DestinationClearAddressHigh                    = 0x0;
            DW16.Value = 0x0;
            //DW16.DestinationSurfaceHeight                       = 0x0;
            //DW16.DestinationSurfaceWidth                        = 0x0;
            //DW16.DestinationSurfaceType                         = DESTINATION_SURFACE_TYPE_SURFTYPE_1D;
            DW17.Value                                            = 0x0;
            //DW17.DestinationLOD                                 = 0x0;
            //DW17.DestinationSurfaceQpitch                       = 0x0;
            //DW17.DestinationSurfaceDepth                        = 0x0;
            DW18.Value                                            = 0x0;
            //DW18.DestinationHorizontalAlign                     = 0x0;
            //DW18.DestinationVerticalAlign                       = 0x0;
            //DW18.DestinationMipTailStartLOD                     = 0x0;
            //DW18.DestinationArrayIndex                          = 0x0;
            DW19.Value                                            = 0x0;
            //DW19.SourceSurfaceHeight                            = 0x0;
            //DW19.SourceSurfaceWidth                             = 0x0;
            //DW19.SourceSurfaceType                              = SOURCE_SURFACE_TYPE_SURFTYPE_1D;
            DW20.Value                                            = 0x0;
            //DW20.SourceLOD                                      = 0x0;
            //DW20.SourceSurfaceQpitch                            = 0x0;
            //DW20.SourceSurfaceDepth                             = 0x0;
            DW21.Value                                            = 0x0;
            //DW21.SourceHorizontalAlign                          = 0x0;
            //DW21.SourceVerticalAlign                            = 0x0;
            //DW21.SourceMipTailStartLOD                          = 0x0;
            //DW21.SourceArrayIndex
        }

        static const size_t dwSize = 22;
        static const size_t byteSize = 88;
    };

    //////////////////////////////////////////////////////////////////////////
    /// @brief BCS_SWCTRL
    /// @details
    ///
    ///
    struct BCS_SWCTRL
    {
        union
        {
            struct
            {
               /// DWORD 0
               uint32_t Reserved_0 : __CODEGEN_BITFIELD(0, 1);                  ///< U2
               uint32_t Disable256Bctypegeneration : __CODEGEN_BITFIELD(2, 2);  ///< U1
               uint32_t ForceEnable4bitCCSupdate : __CODEGEN_BITFIELD(3, 3);    ///< U1
               uint32_t Reserved_4 : __CODEGEN_BITFIELD(4, 15);                 ///< U12
               uint32_t Mask : __CODEGEN_BITFIELD(16, 31);                      ///< U16
            };
            uint32_t Value;
        } DW0;

        //////////////////////////////////////////////////////////////////////////
        /// @name LOCAL ENUMERATIONS
        /// @{

        /// @brief U1
        enum DISABLE_256B_CTYPE_GENERATION
        {
            DISABLE_256B_CTYPE_GENERATION_EN  = 0,  ///< Enable 256B ctype generation.
            DISABLE_256B_CTYPE_GENERATION_DIS = 1,  ///< Disable 256B ctype generation logic
        };

        /// @brief U1
        enum FORCE_ENABLE_4_BIT_CCS_UPDATE
        {
            FORCE_ENABLE_4_BIT_CCS_UPDATE_DISABLE = 0,  ///<
            FORCE_ENABLE_4_BIT_CCS_UPDATE_ENABLE  = 1,  ///<
        };

        /// @brief
        enum CONSTANTS_TYPE
        {
            COMMAND_LENGTH  = 1,        ///<
            REGISTER_OFFSET = 0x22200,  ///<
        };

        /// @brief Explicit member initialization function
        BCS_SWCTRL()
        {
            DW0.Value = 0x0;
            //DW0.Common.Disable256Bctypegeneration                     = DISABLE_256B_CTYPE_GENERATION_EN;
            //DW0.Common.ForceEnable4bitCCSupdate                       = FORCE_ENABLE_4_BIT_CCS_UPDATE_DISABLE;
            //DW0.Common.Mask                                           = 0x0;
        }
    };

    //!
    //! \brief XY_FAST_COPY_BLT
    //! \details
    //!     This BLT instruction performs a color source copy where the only
    //!     operands involved are a color source and destination of the same bit
    //!     width. The source and destination surfaces CANNOT overlap. The hardware
    //!     assumes this whenever this Fast_Copy command is given to it. For
    //!     overlapping Blits, use the traditional XY_SRC_COPY_BLT command (for
    //!     overlap determination, read the description given in the XY_SRC_COPY_BLT
    //!     command). Note that this command does not support Clipping operations.
    //!     This new blit command will happen in large numbers, consecutively,
    //!     possibly an entire batch will comprise only new blit commands Legacy
    //!     commands and new blit command will not be interspersed. If they are,
    //!     they will be separated by implied HW flush: Whenever there is a
    //!     transition between this new Fast Blit command and the Legacy Blit
    //!     commands (2D BLT instructions other than XY_BLOCK_COPY_BLT,
    //!     XY_FAST_COPY_BLT and XY_FAST_COLOR_BLT), the HW will impose an automatic
    //!     flush BEFORE the execution (at the beginning) of the next blitter
    //!     command. New blit command can use any combination of memory surface type
    //!     - linear, tiledX, tiledY, and the tiling information is conveyed as part
    //!     of the new Fast Copy command. The Fast Copy Blit supports the new 64KB
    //!     Tiling defined for SKL.The starting pixel of Fast Copy blit for both
    //!     source and destination should be on an OWord boundary.
    //!     Note that when two sequential fast copy blits have different source
    //!     surfaces, but their destinations refer to the same destination surfaces
    //!     and therefore destinations overlap it is imperative that a Flush be
    //!     inserted between the two blits.
    //!

    struct XY_FAST_COPY_BLT_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 12)    ; //!< Reserved
                uint32_t                 DestinationTilingMethod                          : __CODEGEN_BITFIELD(13, 14)    ; //!< DESTINATION_TILING_METHOD
                uint32_t                 Reserved15                                       : __CODEGEN_BITFIELD(15, 19)    ; //!< Reserved
                uint32_t                 SourceTilingMethod                               : __CODEGEN_BITFIELD(20, 21)    ; //!< SOURCE_TILING_METHOD
                uint32_t                 InstructionTargetOpcode                          : __CODEGEN_BITFIELD(22, 28)    ; //!< INSTRUCTION_TARGETOPCODE
                uint32_t                 Client                                           : __CODEGEN_BITFIELD(29, 31)    ; //!< CLIENT
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 DestinationPitch                                 : __CODEGEN_BITFIELD( 0, 15)    ; //!< Destination Pitch
                uint32_t                 Reserved48                                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Reserved
                uint32_t                 ColorDepth                                       : __CODEGEN_BITFIELD(24, 26)    ; //!< COLOR_DEPTH
                uint32_t                 Reserved59                                       : __CODEGEN_BITFIELD(27, 29)    ; //!< Reserved
                uint32_t                 TileYTypeForDestination                          : __CODEGEN_BITFIELD(30, 30)    ; //!< TILE_Y_TYPE_FOR_DESTINATION
                uint32_t                 TileYTypeForSource                               : __CODEGEN_BITFIELD(31, 31)    ; //!< TILE_Y_TYPE_FOR_SOURCE
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 DestinationX1CoordinateLeft                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Destination X1 Coordinate (Left)
                uint32_t                 DestinationY1CoordinateTop                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Destination Y1 Coordinate (Top)
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 DestinationX2CoordinateRight                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< Destination X2 Coordinate (Right)
                uint32_t                 DestinationY2CoordinateBottom                    : __CODEGEN_BITFIELD(16, 31)    ; //!< Destination Y2 Coordinate (Bottom)
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint64_t                 DestinationBaseAddress                                                           ; //!< Destination Base Address
            };
            uint32_t                     Value[2];
        } DW4_5;
        union
        {
            struct
            {
                uint32_t                 SourceX1CoordinateLeft                           : __CODEGEN_BITFIELD( 0, 15)    ; //!< Source X1 Coordinate (Left)
                uint32_t                 SourceY1CoordinateTop                            : __CODEGEN_BITFIELD(16, 31)    ; //!< Source Y1 Coordinate (Top)
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            struct
            {
                uint32_t                 SourcePitch                                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Source Pitch
                uint32_t                 Reserved240                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            struct
            {
                uint64_t                 SourceBaseAddress                                                                ; //!< Source Base Address
            };
            uint32_t                     Value[2];
        } DW8_9;

        //! \name Local enumerations

        //! \brief DESTINATION_TILING_METHOD
        //! \details
        //!     SW is required to flush the HW before changing the polarity of these
        //!     bits for subsequent blits.
        enum DESTINATION_TILING_METHOD
        {
            DESTINATION_TILING_METHOD_LINEAR_TILINGDISABLED                  = 0, //!< No additional details
            DESTINATION_TILING_METHOD_LEGACYTILE_X                           = 1, //!< No additional details
            DESTINATION_TILING_METHOD_TILE_Y                                 = 2, //!< Choosing between 'Legacy Tile-Y' or the 'New 4K Tile-YF' can be done in DWord 1, Bit[30].
            DESTINATION_TILING_METHOD_64KBTILING                             = 3, //!< No additional details
        };

        //! \brief SOURCE_TILING_METHOD
        //! \details
        //!     SW is required to flush the HW before changing the polarity of these
        //!     bits for subsequent blits.
        enum SOURCE_TILING_METHOD
        {
            SOURCE_TILING_METHOD_LINEAR_TILINGDISABLED                       = 0, //!< No additional details
            SOURCE_TILING_METHOD_LEGACYTILE_X                                = 1, //!< No additional details
            SOURCE_TILING_METHOD_TILE_Y                                      = 2, //!< Choosing between 'Legacy Tile-Y' or the 'New 4K Tile-YF' can be done in DWord 1, Bit[31].
            SOURCE_TILING_METHOD_64KBTILING                                  = 3, //!< No additional details
        };

        enum INSTRUCTION_TARGETOPCODE
        {
            INSTRUCTION_TARGETOPCODE_UNNAMED66                               = 66, //!< No additional details
        };

        enum CLIENT
        {
            CLIENT_2DPROCESSOR                                               = 2, //!< No additional details
        };

        enum COLOR_DEPTH
        {
            COLOR_DEPTH_8BITCOLOR                                            = 0, //!< No additional details
            COLOR_DEPTH_16BITCOLOR_565                                       = 1, //!< No additional details
            COLOR_DEPTH_32BITCOLOR                                           = 3, //!< No additional details
            COLOR_DEPTH_64BITCOLOR_FOR64KBTILING                             = 4, //!< No additional details
            COLOR_DEPTH_128BITCOLOR_FOR64KBTILING                            = 5, //!< No additional details
        };

        //! \brief TILE_Y_TYPE_FOR_DESTINATION
        //! \details
        //!     Destination being Tile-Y can be selected in DWord 0, Bit[14:13].
        enum TILE_Y_TYPE_FOR_DESTINATION
        {
            TILE_Y_TYPE_FOR_DESTINATION_NEW4KTILE_YF                         = 1, //!< No additional details
        };

        //! \brief TILE_Y_TYPE_FOR_SOURCE
        //! \details
        //!     Source being Tile-Y can be selected in DWord 0, Bit[21:20].
        enum TILE_Y_TYPE_FOR_SOURCE
        {
            TILE_Y_TYPE_FOR_SOURCE_NEW4KTILE_YF                              = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        XY_FAST_COPY_BLT_CMD()
        {
            DW0.Value                                        = 0x50800008;
            //DW0.DwordLength                                  = GetOpLength(dwSize);
            //DW0.DestinationTilingMethod                      = DESTINATION_TILING_METHOD_LINEAR_TILINGDISABLED;
            //DW0.SourceTilingMethod                           = SOURCE_TILING_METHOD_LINEAR_TILINGDISABLED;
            //DW0.InstructionTargetOpcode                      = INSTRUCTION_TARGETOPCODE_UNNAMED66;
            //DW0.Client                                       = CLIENT_2DPROCESSOR;

            DW1.Value                                        = 0x00000000;
            //DW1.ColorDepth                                   = COLOR_DEPTH_8BITCOLOR;
            //DW1.TileYTypeForDestination                      = 0;
            //DW1.TileYTypeForSource                           = 0;

            DW2.Value                                        = 0x00000000;

            DW3.Value                                        = 0x00000000;

            DW4_5.Value[0] = DW4_5.Value[1]                  = 0x00000000;

            DW6.Value                                        = 0x00000000;

            DW7.Value                                        = 0x00000000;

            DW8_9.Value[0] = DW8_9.Value[1]                  = 0x00000000;

        }


        static const size_t dwSize = 10;
        static const size_t byteSize = 40;
    };

MEDIA_CLASS_DEFINE_END(mhw__blt__xe2_hpm__Cmd)
};
}  // namespace xe2_hpm
}  // namespace blt
}  // namespace mhw
#pragma pack()

#endif  // __MHW_BLT_HWCMD_XE2_HPM__NEXT_H__
