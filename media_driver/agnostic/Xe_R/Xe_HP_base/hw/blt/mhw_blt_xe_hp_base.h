/*
* Copyright (c) 2020 - 2021, Intel Corporation
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
//! \file     mhw_blt_xe_hp_base.h
//! \brief    MHW interface for constructing commands for the BLT
//!
#ifndef __MHW_BLT_XE_HP_BASE__H__
#define __MHW_BLT_XE_HP_BASE__H__

#include "mhw_blt_legacy.h"

#pragma once
#pragma pack(1)
class mhw_blt_state_xe_hp_base: public mhw_blt_state
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
                uint32_t         DestinationAuxiliarysurfacemode                  : __CODEGEN_BITFIELD(18, 20)    ; ///< U3
                uint32_t         DestinationMocSvalue                             : __CODEGEN_BITFIELD(21, 27)    ; ///< U7
                uint32_t         DestinationControlSurfaceType                    : __CODEGEN_BITFIELD(28, 28)    ; ///< U1
                uint32_t         DestinationCompressionEnable                     : __CODEGEN_BITFIELD(29, 29)    ; ///< U1
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
                uint32_t         SourceAuxiliarysurfacemode                      : __CODEGEN_BITFIELD(18, 20); ///< U3
                uint32_t         SourceMocs                                      : __CODEGEN_BITFIELD(21, 27); ///< U7
                uint32_t         SourceControlSurfaceType                        : __CODEGEN_BITFIELD(28, 28); ///< U1
                uint32_t         SourceCompressionEnable                         : __CODEGEN_BITFIELD(29, 29); ///< U1
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
                uint32_t         Reserved366                                      : __CODEGEN_BITFIELD(14, 15)    ; ///< U2
                uint32_t         SourceYoffset                                    : __CODEGEN_BITFIELD(16, 29)    ; ///< U14
                uint32_t         Reserved382                                      : __CODEGEN_BITFIELD(30, 30)    ; ///< U1
                uint32_t         SourceTargetMemory                               : __CODEGEN_BITFIELD(31, 31)    ; ///< U1
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            struct
            {
                uint32_t         SourceCompressionFormat                          : __CODEGEN_BITFIELD(0, 4); ///< U5
                uint32_t         SourceClearValueEnable                           : __CODEGEN_BITFIELD(5, 5); ///< U1
                uint32_t         SourceClearAddressLow                            : __CODEGEN_BITFIELD(6, 31); ///< U26
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            struct
            {
                uint32_t         SourceClearAddressHigh                           : __CODEGEN_BITFIELD( 0, 15)    ; ///< U16
                uint32_t         Reserved441                                      : __CODEGEN_BITFIELD(16, 31)    ; ///< U16
            };
            uint32_t                     Value;
        } DW13;
        union
        {
             struct
             {
                uint32_t         DestinationCompressionFormat                     : __CODEGEN_BITFIELD( 0,  4)    ; ///< U5
                uint32_t         DestinationClearValueEnable                      : __CODEGEN_BITFIELD( 5,  5)    ; ///< U1
                uint32_t         DestinationClearAddressLow                       : __CODEGEN_BITFIELD( 6, 31)    ; ///< U26
             };
             uint32_t                     Value;
        } DW14;
        union
        {
             struct
             {
                uint32_t         DestinationClearAddressHigh                      : __CODEGEN_BITFIELD( 0, 24)    ; ///< U25
                uint32_t         Reserved505                                      : __CODEGEN_BITFIELD(25, 31)    ; ///< U7
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
                 uint32_t         Reserved563                                     : __CODEGEN_BITFIELD(19, 20); ///< U2
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
                uint32_t         DestinationDepthStencilResource                  : __CODEGEN_BITFIELD(18, 18)    ; ///< U1
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
               uint32_t         SourceDepthStencilResource                       : __CODEGEN_BITFIELD(18, 18)    ; ///< U1
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
        enum TILING_MODE
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
        XY_BLOCK_COPY_BLT_CMD();

        static const size_t dwSize = 22;
        static const size_t byteSize = 88;
    };

    //! \brief XY_CTRL_SURF_COPY_BLT
    //! \details
    //!     XY_CTRL_SURF_COPY_BLT instruction copies control surface associated with
    //!     a main surface from source to destination. This operation is always
    //!     associated with main surface copy operation.
    //!
    struct XY_CTRL_SURF_COPY_BLT_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0,  7)    ; //!< DWORD_LENGTH
                uint32_t                 SizeOfControlSurfaceCopy                         : __CODEGEN_BITFIELD( 8, 17)    ; //!< Size of Control Surface Copy
                uint32_t                 Reserved18                                       : __CODEGEN_BITFIELD(18, 19)    ; //!< Reserved
                uint32_t                 DestinationMemoryType                            : __CODEGEN_BITFIELD(20, 20)    ; //!< DESTINATION_MEMORY_TYPE
                uint32_t                 SourceMemoryType                                 : __CODEGEN_BITFIELD(21, 21)    ; //!< SOURCE_MEMORY_TYPE
                uint32_t                 InstructionTargetOpcode                          : __CODEGEN_BITFIELD(22, 28)    ; //!< INSTRUCTION_TARGETOPCODE
                uint32_t                 Client                                           : __CODEGEN_BITFIELD(29, 31)    ; //!< CLIENT
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 SourceStartAddressLow                            : __CODEGEN_BITFIELD(12, 31)    ; //!< Source Start Address Low
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 SourceStartAddressHigh                           : __CODEGEN_BITFIELD( 0, 15)    ; //!< Source Start Address High
                uint32_t                 Reserved80                                       : __CODEGEN_BITFIELD(16, 24)    ; //!< Reserved
                uint32_t                 SourceMocs                                       : __CODEGEN_BITFIELD(25, 31)    ; //!< Source MOCS
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 Reserved96                                       : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint32_t                 DestinationStartAddressLow                       : __CODEGEN_BITFIELD(12, 31)    ; //!< Destination Start Address Low
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 DestinationStartAddressHigh                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Destination Start Address High
                uint32_t                 Reserved144                                      : __CODEGEN_BITFIELD(16, 24)    ; //!< Reserved
                uint32_t                 DestinationMocs                                  : __CODEGEN_BITFIELD(25, 31)    ; //!< Destination MOCS
            };
            uint32_t                     Value;
        } DW4;

        //! \name Local enumerations

        enum DESTINATION_MEMORY_TYPE
        {
            DESTINATION_MEMORY_TYPE_LOCALMEM                                 = 0, //!< Destination is local memory
            DESTINATION_MEMORY_TYPE_SYSMEM                                   = 1, //!< Destination is system memory
        };

        enum SOURCE_MEMORY_TYPE
        {
            SOURCE_MEMORY_TYPE_LOCALMEM                                      = 0, //!< Source is local memory.
            SOURCE_MEMORY_TYPE_SYSMEM                                        = 1, //!< Source is system memory.
        };

        enum INSTRUCTION_TARGETOPCODE
        {
            INSTRUCTION_TARGETOPCODE_UNNAMED72                               = 72, //!< No additional details
        };

        enum CLIENT
        {
            CLIENT_2DPROCESSOR                                               = 2, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        XY_CTRL_SURF_COPY_BLT_CMD();

        static const size_t dwSize = 5;
        static const size_t byteSize = 20;
    };

    //////////////////////////////////////////////////////////////////////////
    /// @brief BCS_SWCTRL
    /// @details
    ///
    ///
    struct BCS_SWCTRL_CMD
    {
        union
        {
            struct
            {
                /// DWORD 0
                uint32_t                 Tile4Source : __CODEGEN_BITFIELD(0, 0); ///< U1
                uint32_t                 Tile4Destination : __CODEGEN_BITFIELD(1, 1); ///< U1
                uint32_t                 SystemMemoryThrottleThreshold : __CODEGEN_BITFIELD(2, 15); ///< U14
                uint32_t                 Mask : __CODEGEN_BITFIELD(16, 31); ///< U16

            };
            uint32_t                     Value;
        } DW0;

        //////////////////////////////////////////////////////////////////////////
        /// @name LOCAL ENUMERATIONS
        /// @{
        /// @brief U1
        enum TILE_4_SOURCE
        {
           TILE_4_SOURCE_XMAJOR = 0, ///<
           TILE_4_SOURCE_TILE4 = 1, ///<
        };

        /// @brief U1
        enum TILE_4_DESTINATION
        {
            TILE_4_DESTINATION_XMAJOR = 0, ///<
            TILE_4_DESTINATION_TILE4 = 1, ///
        };
        /// @brief
        enum CONSTANTS_TYPE
        {
            COMMAND_LENGTH = 1, ///<
            REGISTER_OFFSET = 0x22200, ///<
        };
        //! \name Initializations

        //! \brief Explicit member initialization function
        BCS_SWCTRL_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };
};
#pragma pack()

class MhwBltInterfaceXe_Hp_Base: public MhwBltInterface
{
public:
    MhwBltInterfaceXe_Hp_Base(PMOS_INTERFACE pOsInterface);

    virtual ~MhwBltInterfaceXe_Hp_Base()
    {
        MHW_FUNCTION_ENTER;
    }

    //!
    //! \brief    Add control surface copy blt
    //! \details  MHW function to add control surface copy blt command
    //! \param    [in] pCmdBuffer
    //!           Pointer to Command buffer
    //! \param    [in] pCtrlSurfCopyBltParam
    //!           Pointer to MHW_CTRL_SURF_COPY_BLT_PARAM
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddCtrlSurfCopyBlt(
        PMOS_COMMAND_BUFFER              pCmdBuffer,
        PMHW_CTRL_SURF_COPY_BLT_PARAM    pCtrlSurfCopyBltParam);
    
    //!
    //! \brief    Add Block copy
    //! \details  MHW function to add block copy command
    //! \param    [in] pCmdBuffer
    //!           Pointer to Command buffer
    //! \param    [in] pFastCopyBltParam
    //!           Pointer to MHW_FAST_COPY_BLT_PARAM
    //! \param    [in] srcOffset
    //!           input surface's soruce offset
    //! \param    [in] outOffset
    //!           output surface's soruce offset
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddBlockCopyBlt(
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_FAST_COPY_BLT_PARAM    pFastCopyBltParam,
        uint32_t                    srcOffset,
        uint32_t                    dstOffset);

    protected:
    //!
    //! \brief    Get Block copy tilling mode
    //! \details  Get Block copy tilling mode
    //! \param    [in] TileType
    //!           Pointer to BLT_TILE_TYPE
    //! \return   uint32_t
    //!           0:1:2:3
    //!
    virtual uint32_t GetBlockTilingMode(
        BLT_TILE_TYPE              TileType);
};

typedef class MhwBltInterfaceXe_Hp_Base *PMHW_BLT_INTERFACE_XE_HP;

#endif  // __MHW_BLT_XE_HP_BASE__H__
