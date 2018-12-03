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
//! \file     mhw_state_heap_hwcmd_g9_X.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of g9_X as other components
//!           should use MHW interface to interact with MHW commands and states.
//!
#ifndef __MHW_STATE_HEAP_HWCMD_G9_X_H__
#define __MHW_STATE_HEAP_HWCMD_G9_X_H__

#pragma once
#pragma pack(1)

#include <cstdint>
#include <cstddef>

class mhw_state_heap_g9_X
{
public:
    // Internal Macros
    #define __CODEGEN_MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
    #define __CODEGEN_BITFIELD(l, h) (h) - (l) + 1
    #define __CODEGEN_OP_LENGTH_BIAS 2
    #define __CODEGEN_OP_LENGTH(x) (uint32_t)((__CODEGEN_MAX(x, __CODEGEN_OP_LENGTH_BIAS)) - __CODEGEN_OP_LENGTH_BIAS)

    static uint32_t GetOpLength(uint32_t uiLength) { return __CODEGEN_OP_LENGTH(uiLength); }

    //!
    //! \brief INTERFACE_DESCRIPTOR_DATA
    //! \details
    //! 
    //!
    struct INTERFACE_DESCRIPTOR_DATA_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 KernelStartPointer                               : __CODEGEN_BITFIELD( 6, 31)    ; //!< Kernel Start Pointer
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 KernelStartPointerHigh                           : __CODEGEN_BITFIELD( 0, 15)    ; //!< Kernel Start Pointer High
                uint32_t                 Reserved48                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Reserved64                                       : __CODEGEN_BITFIELD( 0,  6)    ; //!< Reserved
                uint32_t                 SoftwareExceptionEnable                          : __CODEGEN_BITFIELD( 7,  7)    ; //!< Software Exception Enable
                uint32_t                 Reserved72                                       : __CODEGEN_BITFIELD( 8, 10)    ; //!< Reserved
                uint32_t                 MaskStackExceptionEnable                         : __CODEGEN_BITFIELD(11, 11)    ; //!< Mask Stack Exception Enable
                uint32_t                 Reserved76                                       : __CODEGEN_BITFIELD(12, 12)    ; //!< Reserved
                uint32_t                 IllegalOpcodeExceptionEnable                     : __CODEGEN_BITFIELD(13, 13)    ; //!< Illegal Opcode Exception Enable
                uint32_t                 Reserved78                                       : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 FloatingPointMode                                : __CODEGEN_BITFIELD(16, 16)    ; //!< FLOATING_POINT_MODE
                uint32_t                 ThreadPriority                                   : __CODEGEN_BITFIELD(17, 17)    ; //!< THREAD_PRIORITY
                uint32_t                 SingleProgramFlow                                : __CODEGEN_BITFIELD(18, 18)    ; //!< SINGLE_PROGRAM_FLOW
                uint32_t                 DenormMode                                       : __CODEGEN_BITFIELD(19, 19)    ; //!< DENORM_MODE
                uint32_t                 Reserved84                                       : __CODEGEN_BITFIELD(20, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 Reserved96                                       : __CODEGEN_BITFIELD( 0,  1)    ; //!< Reserved
                uint32_t                 SamplerCount                                     : __CODEGEN_BITFIELD( 2,  4)    ; //!< SAMPLER_COUNT
                uint32_t                 SamplerStatePointer                              : __CODEGEN_BITFIELD( 5, 31)    ; //!< Sampler State Pointer
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 BindingTableEntryCount                           : __CODEGEN_BITFIELD( 0,  4)    ; //!< Binding Table Entry Count
                uint32_t                 BindingTablePointer                              : __CODEGEN_BITFIELD( 5, 15)    ; //!< Binding Table Pointer
                uint32_t                 Reserved144                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 ConstantUrbEntryReadOffset                       : __CODEGEN_BITFIELD( 0, 15)    ; //!< Constant URB Entry Read Offset
                uint32_t                 ConstantIndirectUrbEntryReadLength               : __CODEGEN_BITFIELD(16, 31)    ; //!< Constant/Indirect URB Entry Read Length
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 NumberOfThreadsInGpgpuThreadGroup                : __CODEGEN_BITFIELD( 0,  9)    ; //!< Number of Threads in GPGPU Thread Group
                uint32_t                 Reserved202                                      : __CODEGEN_BITFIELD(10, 14)    ; //!< Reserved
                uint32_t                 GlobalBarrierEnable                              : __CODEGEN_BITFIELD(15, 15)    ; //!< Global Barrier Enable
                uint32_t                 SharedLocalMemorySize                            : __CODEGEN_BITFIELD(16, 20)    ; //!< SHARED_LOCAL_MEMORY_SIZE
                uint32_t                 BarrierEnable                                    : __CODEGEN_BITFIELD(21, 21)    ; //!< Barrier Enable
                uint32_t                 RoundingMode                                     : __CODEGEN_BITFIELD(22, 23)    ; //!< ROUNDING_MODE
                uint32_t                 Reserved216                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 CrossThreadConstantDataReadLength                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Cross-Thread Constant Data Read Length
                uint32_t                 Reserved232                                      : __CODEGEN_BITFIELD( 8, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;

        //! \name Local enumerations

        //! \brief FLOATING_POINT_MODE
        //! \details
        //!     Specifies the floating point mode used by the dispatched thread.
        enum FLOATING_POINT_MODE
        {
            FLOATING_POINT_MODE_IEEE_754                                     = 0, //!< No additional details
            FLOATING_POINT_MODE_ALTERNATE                                    = 1, //!< No additional details
        };

        //! \brief THREAD_PRIORITY
        //! \details
        //!     Specifies the priority of the thread for dispatch.
        enum THREAD_PRIORITY
        {
            THREAD_PRIORITY_NORMALPRIORITY                                   = 0, //!< No additional details
            THREAD_PRIORITY_HIGHPRIORITY                                     = 1, //!< No additional details
        };

        //! \brief SINGLE_PROGRAM_FLOW
        //! \details
        //!     Specifies whether the kernel program has a single program flow (SIMDnxm
        //!     with m = 1) or multiple program flows (SIMDnxm with m &gt; 1).
        enum SINGLE_PROGRAM_FLOW
        {
            SINGLE_PROGRAM_FLOW_MULTIPLE                                     = 0, //!< No additional details
            SINGLE_PROGRAM_FLOW_SINGLE                                       = 1, //!< No additional details
        };

        //! \brief DENORM_MODE
        //! \details
        //!     This field specifies how Float denormalized numbers are handles in the
        //!     dispatched thread.
        enum DENORM_MODE
        {
            DENORM_MODE_FTZ                                                  = 0, //!< Float denorms will be flushed to zero when appearing as inputs; denorms will never come out of instructions. Double precision float and half precision float numbers are not flushed to zero.
            DENORM_MODE_SETBYKERNEL                                          = 1, //!< Denorms will be handled in by kernel.
        };

        //! \brief SAMPLER_COUNT
        //! \details
        //!     Specifies how many samplers (in multiples of 4) the kernel uses.  Used
        //!     only for prefetching the associated sampler state entries. 
        //!                         <i>This field is ignored for child threads.</i>
        //!                         <i>If this field is not zero, sampler state is prefetched for the
        //!     first instance of a root thread upon the startup of the media
        //!     pipeline.</i>
        enum SAMPLER_COUNT
        {
            SAMPLER_COUNT_NOSAMPLERSUSED                                     = 0, //!< No additional details
            SAMPLER_COUNT_BETWEEN1AND4SAMPLERSUSED                           = 1, //!< No additional details
            SAMPLER_COUNT_BETWEEN5AND8SAMPLERSUSED                           = 2, //!< No additional details
            SAMPLER_COUNT_BETWEEN9AND12SAMPLERSUSED                          = 3, //!< No additional details
            SAMPLER_COUNT_BETWEEN13AND16SAMPLERSUSED                         = 4, //!< No additional details
        };

        //! \brief SHARED_LOCAL_MEMORY_SIZE
        //! \details
        //!     This field indicates how much SharedLocalMemory the thread group
        //!     requires. The amount is specified in 4k blocks, but only powers of 2 are
        //!     allowed: 0, 4k, 8k, 16k, 32k and 64k per half-slice.
        enum SHARED_LOCAL_MEMORY_SIZE
        {
            SHARED_LOCAL_MEMORY_SIZE_ENCODES0K                               = 0, //!< No SLM used
            SHARED_LOCAL_MEMORY_SIZE_ENCODES1K                               = 1, //!< No additional details
            SHARED_LOCAL_MEMORY_SIZE_ENCODES2K                               = 2, //!< No additional details
            SHARED_LOCAL_MEMORY_SIZE_ENCODES4K                               = 3, //!< No additional details
            SHARED_LOCAL_MEMORY_SIZE_ENCODES8K                               = 4, //!< No additional details
            SHARED_LOCAL_MEMORY_SIZE_ENCODES16K                              = 5, //!< No additional details
            SHARED_LOCAL_MEMORY_SIZE_ENCODES32K                              = 6, //!< No additional details
            SHARED_LOCAL_MEMORY_SIZE_ENCODES64K                              = 7, //!< No additional details
        };

        enum ROUNDING_MODE
        {
            ROUNDING_MODE_RTNE                                               = 0, //!< Round to Nearest Even
            ROUNDING_MODE_RU                                                 = 1, //!< Round toward +Infinity
            ROUNDING_MODE_RD                                                 = 2, //!< Round toward -Infinity
            ROUNDING_MODE_RTZ                                                = 3, //!< Round toward Zero
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        INTERFACE_DESCRIPTOR_DATA_CMD();

        static const size_t dwSize = 8;
        static const size_t byteSize = 32;
    };

    //!
    //! \brief BINDING_TABLE_STATE
    //! \details
    //!     The binding table binds surfaces to logical resource indices used by
    //!     shaders and other compute engine kernels. It is stored as an array of up
    //!     to 256 elements, each of which contains one dword as defined here. The
    //!     start of each element is spaced one dword apart. The first element of
    //!     the binding table is aligned to a 64-byte boundary. Binding table
    //!     indexes beyond 256 will automatically be mapped to entry 0 by the HW, w/
    //!     the exception of any messages which support the special indexes 240
    //!     through 255, inclusive.
    //!
    struct BINDING_TABLE_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 SurfaceStatePointer                              : __CODEGEN_BITFIELD( 6, 31)    ; //!< Surface State Pointer
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        BINDING_TABLE_STATE_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief RENDER_SURFACE_STATE
    //! \details
    //!     This is the normal surface state used by all messages that use
    //!     SURFACE_STATE except those that use MEDIA_SURFACE_STATE.
    //!
    struct RENDER_SURFACE_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 CubeFaceEnablePositiveZ                          : __CODEGEN_BITFIELD( 0,  0)    ; //!< Cube Face Enable - Positive Z, [Surface Type] == 'SURFTYPE_CUBE'
                uint32_t                 CubeFaceEnableNegativeZ                          : __CODEGEN_BITFIELD( 1,  1)    ; //!< Cube Face Enable - Negative Z, [Surface Type] == 'SURFTYPE_CUBE'
                uint32_t                 CubeFaceEnablePositiveY                          : __CODEGEN_BITFIELD( 2,  2)    ; //!< Cube Face Enable - Positive Y, [Surface Type] == 'SURFTYPE_CUBE'
                uint32_t                 CubeFaceEnableNegativeY                          : __CODEGEN_BITFIELD( 3,  3)    ; //!< Cube Face Enable - Negative Y, [Surface Type] == 'SURFTYPE_CUBE'
                uint32_t                 CubeFaceEnablePositiveX                          : __CODEGEN_BITFIELD( 4,  4)    ; //!< Cube Face Enable - Positive X, [Surface Type] == 'SURFTYPE_CUBE'
                uint32_t                 CubeFaceEnableNegativeX                          : __CODEGEN_BITFIELD( 5,  5)    ; //!< Cube Face Enable - Negative X, [Surface Type] == 'SURFTYPE_CUBE'
                uint32_t                 MediaBoundaryPixelMode                           : __CODEGEN_BITFIELD( 6,  7)    ; //!< MEDIA_BOUNDARY_PIXEL_MODE
                uint32_t                 RenderCacheReadWriteMode                         : __CODEGEN_BITFIELD( 8,  8)    ; //!< RENDER_CACHE_READ_WRITE_MODE
                uint32_t                 SamplerL2OutOfOrderModeDisable                   : __CODEGEN_BITFIELD( 9,  9)    ; //!< Sampler L2 Out of Order Mode Disable
                uint32_t                 VerticalLineStrideOffset                         : __CODEGEN_BITFIELD(10, 10)    ; //!< Vertical Line Stride Offset
                uint32_t                 VerticalLineStride                               : __CODEGEN_BITFIELD(11, 11)    ; //!< Vertical Line Stride
                uint32_t                 TileMode                                         : __CODEGEN_BITFIELD(12, 13)    ; //!< TILE_MODE
                uint32_t                 SurfaceHorizontalAlignment                       : __CODEGEN_BITFIELD(14, 15)    ; //!< SURFACE_HORIZONTAL_ALIGNMENT
                uint32_t                 SurfaceVerticalAlignment                         : __CODEGEN_BITFIELD(16, 17)    ; //!< SURFACE_VERTICAL_ALIGNMENT
                uint32_t                 SurfaceFormat                                    : __CODEGEN_BITFIELD(18, 26)    ; //!< SURFACE_FORMAT
                uint32_t                 AstcEnable                                       : __CODEGEN_BITFIELD(27, 27)    ; //!< ASTC_Enable
                uint32_t                 SurfaceArray                                     : __CODEGEN_BITFIELD(28, 28)    ; //!< Surface Array
                uint32_t                 SurfaceType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< SURFACE_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 SurfaceQpitch                                    : __CODEGEN_BITFIELD( 0, 14)    ; //!< Surface QPitch
                uint32_t                 Reserved47                                       : __CODEGEN_BITFIELD(15, 18)    ; //!< Reserved
                uint32_t                 BaseMipLevel                                     : __CODEGEN_BITFIELD(19, 23)    ; //!< Base Mip Level
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD(24, 30)    ; //!< Memory Object Control State
                uint32_t                 Reserved63                                       : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Width                                            : __CODEGEN_BITFIELD( 0, 13)    ; //!< Width
                uint32_t                 Reserved78                                       : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 Height                                           : __CODEGEN_BITFIELD(16, 29)    ; //!< Height
                uint32_t                 Reserved94                                       : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 SurfacePitch                                     : __CODEGEN_BITFIELD( 0, 17)    ; //!< Surface Pitch
                uint32_t                 Reserved114                                      : __CODEGEN_BITFIELD(18, 20)    ; //!< Reserved
                uint32_t                 Depth                                            : __CODEGEN_BITFIELD(21, 31)    ; //!< Depth
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 MultisamplePositionPaletteIndex                  : __CODEGEN_BITFIELD( 0,  2)    ; //!< Multisample Position Palette Index, [Surface Type] != 'SURFTYPE_STRBUF'
                uint32_t                 NumberOfMultisamples                             : __CODEGEN_BITFIELD( 3,  5)    ; //!< NUMBER_OF_MULTISAMPLES, [Surface Type] != 'SURFTYPE_STRBUF'
                uint32_t                 MultisampledSurfaceStorageFormat                 : __CODEGEN_BITFIELD( 6,  6)    ; //!< MULTISAMPLED_SURFACE_STORAGE_FORMAT, [Surface Type] != 'SURFTYPE_STRBUF'
                uint32_t                 RenderTargetViewExtent                           : __CODEGEN_BITFIELD( 7, 17)    ; //!< Render Target View Extent, [Surface Type] != 'SURFTYPE_STRBUF'
                uint32_t                 MinimumArrayElement                              : __CODEGEN_BITFIELD(18, 28)    ; //!< Minimum Array Element, [Surface Type] != 'SURFTYPE_STRBUF'
                uint32_t                 RenderTargetAndSampleUnormRotation               : __CODEGEN_BITFIELD(29, 30)    ; //!< RENDER_TARGET_AND_SAMPLE_UNORM_ROTATION, [Surface Type] != 'SURFTYPE_STRBUF'
                uint32_t                 Reserved159                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved, [Surface Type] != 'SURFTYPE_STRBUF'
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 MipCountLod                                      : __CODEGEN_BITFIELD( 0,  3)    ; //!< MIP Count / LOD
                uint32_t                 SurfaceMinLod                                    : __CODEGEN_BITFIELD( 4,  7)    ; //!< Surface Min LOD
                uint32_t                 MipTailStartLod                                  : __CODEGEN_BITFIELD( 8, 11)    ; //!< Mip Tail Start LOD
                uint32_t                 Reserved172                                      : __CODEGEN_BITFIELD(12, 13)    ; //!< Reserved
                uint32_t                 CoherencyType                                    : __CODEGEN_BITFIELD(14, 14)    ; //!< COHERENCY_TYPE
                uint32_t                 Reserved175                                      : __CODEGEN_BITFIELD(15, 17)    ; //!< Reserved
                uint32_t                 TiledResourceMode                                : __CODEGEN_BITFIELD(18, 19)    ; //!< TILED_RESOURCE_MODE
                uint32_t                 EwaDisableForCube                                : __CODEGEN_BITFIELD(20, 20)    ; //!< EWA_DISABLE_FOR_CUBE
                uint32_t                 YOffset                                          : __CODEGEN_BITFIELD(21, 23)    ; //!< Y Offset
                uint32_t                 Reserved184                                      : __CODEGEN_BITFIELD(24, 24)    ; //!< Reserved
                uint32_t                 XOffset                                          : __CODEGEN_BITFIELD(25, 31)    ; //!< X Offset
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 AuxiliarySurfaceMode                             : __CODEGEN_BITFIELD( 0,  2)    ; //!< AUXILIARY_SURFACE_MODE, ([Surface Format] != 'PLANAR')
                uint32_t                 AuxiliarySurfacePitch                            : __CODEGEN_BITFIELD( 3, 11)    ; //!< Auxiliary Surface Pitch, ([Surface Format] != 'PLANAR')
                uint32_t                 Reserved204                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved, ([Surface Format] != 'PLANAR')
                uint32_t                 AuxiliarySurfaceQpitch                           : __CODEGEN_BITFIELD(16, 30)    ; //!< Auxiliary Surface QPitch, ([Surface Format] != 'PLANAR')
                uint32_t                 Reserved223                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved, ([Surface Format] != 'PLANAR')
            } Obj0;
            struct
            {
                uint32_t                 YOffsetForUOrUvPlane                             : __CODEGEN_BITFIELD( 0, 13)    ; //!< Y Offset for U or UV Plane, ([Surface Format] == 'PLANAR')
                uint32_t                 Reserved206                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved, ([Surface Format] == 'PLANAR')
                uint32_t                 XOffsetForUOrUvPlane                             : __CODEGEN_BITFIELD(16, 29)    ; //!< X Offset for U or UV Plane, ([Surface Format] == 'PLANAR')
                uint32_t                 Reserved222                                      : __CODEGEN_BITFIELD(30, 30)    ; //!< Reserved, ([Surface Format] == 'PLANAR')
                uint32_t                 SeparateUvPlaneEnable                            : __CODEGEN_BITFIELD(31, 31)    ; //!< Separate UV Plane Enable, ([Surface Format] == 'PLANAR')
            } Obj1;
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 ResourceMinLod                                   : __CODEGEN_BITFIELD( 0, 11)    ; //!< Resource Min LOD
                uint32_t                 Reserved236                                      : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 ShaderChannelSelectAlpha                         : __CODEGEN_BITFIELD(16, 18)    ; //!< SHADER_CHANNEL_SELECT_ALPHA
                uint32_t                 ShaderChannelSelectBlue                          : __CODEGEN_BITFIELD(19, 21)    ; //!< SHADER_CHANNEL_SELECT_BLUE
                uint32_t                 ShaderChannelSelectGreen                         : __CODEGEN_BITFIELD(22, 24)    ; //!< SHADER_CHANNEL_SELECT_GREEN
                uint32_t                 ShaderChannelSelectRed                           : __CODEGEN_BITFIELD(25, 27)    ; //!< SHADER_CHANNEL_SELECT_RED
                uint32_t                 Reserved252                                      : __CODEGEN_BITFIELD(28, 29)    ; //!< Reserved
                uint32_t                 MemoryCompressionEnable                          : __CODEGEN_BITFIELD(30, 30)    ; //!< Memory Compression Enable
                uint32_t                 MemoryCompressionMode                            : __CODEGEN_BITFIELD(31, 31)    ; //!< MEMORY_COMPRESSION_MODE
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8..9
            struct
            {
                uint64_t                 SurfaceBaseAddress                                                               ; //!< Surface Base Address
            };
            uint32_t                     Value[2];
        } DW8_9;
        union
        {
            //!< DWORD 10..11
            struct
            {
                uint64_t                 QuiltWidth                                       : __CODEGEN_BITFIELD( 0,  4)    ; //!< Quilt Width,
                uint64_t                 QuiltHeight                                      : __CODEGEN_BITFIELD( 5,  9)    ; //!< Quilt Height,
                uint64_t                 Reserved330                                      : __CODEGEN_BITFIELD(10, 63)    ; //!< Reserved,
            } Obj0;
            struct
            {
                uint64_t                 Reserved320                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved,
                uint64_t                 AuxiliarySurfaceBaseAddress                      : __CODEGEN_BITFIELD(12, 63)    ; //!< Auxiliary Surface Base Address, ([Surface Format] != 'PLANAR') AND [Memory Compression Enable] == 0
            } Obj1;
            struct
            {
                uint64_t                 Reserved320                                      : __CODEGEN_BITFIELD( 0, 20)    ; //!< Reserved,
                uint64_t                 AuxiliaryTableIndexForMediaCompressedSurface     : __CODEGEN_BITFIELD(21, 31)    ; //!< Auxiliary Table Index for Media Compressed Surface, [Memory Compression Enable] ==1
                uint64_t                 Reserved352                                      : __CODEGEN_BITFIELD(32, 63)    ; //!< Reserved, [Memory Compression Enable] ==1
            } Obj2;
            struct
            {
                uint64_t                 Reserved320                                      : __CODEGEN_BITFIELD( 0, 31)    ; //!< Reserved, [Memory Compression Enable] ==1
                uint64_t                 YOffsetForVPlane                                 : __CODEGEN_BITFIELD(32, 45)    ; //!< Y Offset for V Plane, ([Surface Format] == 'PLANAR')
                uint64_t                 Reserved366                                      : __CODEGEN_BITFIELD(46, 47)    ; //!< Reserved, ([Surface Format] == 'PLANAR')
                uint64_t                 XOffsetForVPlane                                 : __CODEGEN_BITFIELD(48, 61)    ; //!< X Offset for V Plane, ([Surface Format] == 'PLANAR')
                uint64_t                 Reserved382                                      : __CODEGEN_BITFIELD(62, 63)    ; //!< Reserved, ([Surface Format] == 'PLANAR')
            } Obj3;
            uint32_t                     Value[2];
        } DW10_11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 HierarchicalDepthClearValue                                                      ; //!< Hierarchical Depth Clear Value, [Auxiliary Surface Mode] == 'AUX_HIZ'
            } Obj0;
            struct
            {
                uint32_t                 RedClearColor                                                                    ; //!< Red Clear Color, [Auxiliary Surface Mode] == 'AUX_CCS_D' OR [Auxiliary Surface Mode] == 'AUX_CCS_E'
            } Obj1;
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 GreenClearColor                                                                  ; //!< Green Clear Color, [Auxiliary Surface Mode] == 'AUX_CCS_D' OR [Auxiliary Surface Mode] == 'AUX_CCS_E'
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 BlueClearColor                                                                   ; //!< Blue Clear Color, [Auxiliary Surface Mode] == 'AUX_CCS_D' OR [Auxiliary Surface Mode] == 'AUX_CCS_E'
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 AlphaClearColor                                                                  ; //!< Alpha Clear Color, [Auxiliary Surface Mode] == 'AUX_CCS_D' OR [Auxiliary Surface Mode] == 'AUX_CCS_E'
            };
            uint32_t                     Value;
        } DW15;

        //! \name Local enumerations

        //! \brief MEDIA_BOUNDARY_PIXEL_MODE
        //! \details
        //!     <div id="GroupContent-248" class="GroupContent UseCKEdit">
        //!     <p><b>For 2D Non-Array Surfaces accessed via the Data Port Media Block
        //!     Read Message or Data Port Transpose Read message:</b><br />
        //!         This field enables control of which rows are returned on vertical
        //!     out-of-bounds reads using the Data Port Media Block Read Message or Data
        //!     Port Transpose Read message. In the description below, frame mode refers
        //!     to <b>Vertical Line Stride</b> = 0, field mode is <b>Vertical Line
        //!     Stride</b> = 1 in which only the even or odd rows are addressable. The
        //!     frame refers to the entire surface, while the field refers only to the
        //!     even or odd rows within the surface.</p>
        //!     <p><b>For Other Surfaces:</b><br />
        //!         Reserved : MBZ</p></div>
        enum MEDIA_BOUNDARY_PIXEL_MODE
        {
            MEDIA_BOUNDARY_PIXEL_MODE_NORMALMODE                             = 0, //!< The row returned on an out-of-bound access is the closest row in the frame or field.  Rows from the opposite field are never returned.
            MEDIA_BOUNDARY_PIXEL_MODE_PROGRESSIVEFRAME                       = 2, //!< The row returned on an out-of-bound access is the closest row in the frame, even if in field mode.
            MEDIA_BOUNDARY_PIXEL_MODE_INTERLACEDFRAME                        = 3, //!< In field mode, the row returned on an out-of-bound access is the closest row in the field.  In frame mode, even out-of-bound rows return the nearest even row while odd out-of-bound rows return the nearest odd row.
        };

        //! \brief RENDER_CACHE_READ_WRITE_MODE
        //! \details
        //!     <p><b>For Surfaces accessed via the Data Port to Render Cache:</b><br />
        //!         This field specifies the way Render Cache treats a write request. If
        //!     unset, Render Cache allocates a write-only cache line for a write miss.
        //!     If set, Render Cache allocates a read-write cache line for a write
        //!     miss.</p>
        //!     <p><b>For Surfaces accessed via the Sampling Engine or Data Port to
        //!     Texture Cache or Data Cache:</b><br />
        //!         This field is reserved : MBZ</p>
        enum RENDER_CACHE_READ_WRITE_MODE
        {
            RENDER_CACHE_READ_WRITE_MODE_WRITE_ONLYCACHE                     = 0, //!< Allocating write-only cache for a write miss
            RENDER_CACHE_READ_WRITE_MODE_READ_WRITECACHE                     = 1, //!< Allocating read-write cache for a write miss
        };

        //! \brief TILE_MODE
        //! \details
        //!     This field specifies the type of memory tiling (Linear, WMajor, XMajor,
        //!     or YMajor) employed to tile this surface. See <em>Memory Interface
        //!     Functions</em> for details on memory tiling and restrictions.
        enum TILE_MODE
        {
            TILE_MODE_LINEAR                                                 = 0, //!< Linear mode (no tiling)
            TILE_MODE_WMAJOR                                                 = 1, //!< W major tiling
            TILE_MODE_XMAJOR                                                 = 2, //!< X major tiling
            TILE_MODE_YMAJOR                                                 = 3, //!< Y major tiling
        };

        //! \brief SURFACE_HORIZONTAL_ALIGNMENT
        //! \details
        //!     For Sampling Engine and Render Target Surfaces: This field specifies the
        //!     horizontal alignment requirement for the surface.
        enum SURFACE_HORIZONTAL_ALIGNMENT
        {
            SURFACE_HORIZONTAL_ALIGNMENT_HALIGN4                             = 1, //!< Horizontal alignment factor j = 4
            SURFACE_HORIZONTAL_ALIGNMENT_HALIGN8                             = 2, //!< Horizontal alignment factor j = 8
            SURFACE_HORIZONTAL_ALIGNMENT_HALIGN16                            = 3, //!< Horizontal alignment factor j = 16
        };

        //! \brief SURFACE_VERTICAL_ALIGNMENT
        //! \details
        //!     <b>For Sampling Engine and Render Target Surfaces:</b> This field
        //!     specifies the vertical alignment requirement in elements for the
        //!     surface. Refer to the "Memory Data Formats" chapter for details on how
        //!     this field changes the layout of the surface in memory. An
        //!     <i>element</i> is defined as a pixel in uncompresed surface formats, and
        //!     as a compression block in compressed surface formats. For
        //!     MSFMT_DEPTH_STENCIL type multisampled surfaces, an element is a sample.
        enum SURFACE_VERTICAL_ALIGNMENT
        {
            SURFACE_VERTICAL_ALIGNMENT_VALIGN4                               = 1, //!< Vertical alignment factor j = 4
            SURFACE_VERTICAL_ALIGNMENT_VALIGN8                               = 2, //!< Vertical alignment factor j = 8
            SURFACE_VERTICAL_ALIGNMENT_VALIGN16                              = 3, //!< Vertical alignment factor j = 16
        };

        //! \brief SURFACE_FORMAT
        //! \details
        //!     This field specifies the format of the surface or element within this
        //!     surface. This field is ignored for all data port messages other than the
        //!     render target message and streamed vertex buffer write message. Some
        //!     forms of the media block messages use the surface format.
        enum SURFACE_FORMAT
        {
            SURFACE_FORMAT_R32G32B32A32FLOAT                                 = 0, //!< No additional details
            SURFACE_FORMAT_R32G32B32A32SINT                                  = 1, //!< No additional details
            SURFACE_FORMAT_R32G32B32A32UINT                                  = 2, //!< No additional details
            SURFACE_FORMAT_R32G32B32A32UNORM                                 = 3, //!< No additional details
            SURFACE_FORMAT_R32G32B32A32SNORM                                 = 4, //!< No additional details
            SURFACE_FORMAT_R64G64FLOAT                                       = 5, //!< No additional details
            SURFACE_FORMAT_R32G32B32X32FLOAT                                 = 6, //!< No additional details
            SURFACE_FORMAT_R32G32B32A32SSCALED                               = 7, //!< No additional details
            SURFACE_FORMAT_R32G32B32A32USCALED                               = 8, //!< No additional details
            SURFACE_FORMAT_R32G32B32A32SFIXED                                = 32, //!< No additional details
            SURFACE_FORMAT_R64G64PASSTHRU                                    = 33, //!< No additional details
            SURFACE_FORMAT_R32G32B32FLOAT                                    = 64, //!< No additional details
            SURFACE_FORMAT_R32G32B32SINT                                     = 65, //!< No additional details
            SURFACE_FORMAT_R32G32B32UINT                                     = 66, //!< No additional details
            SURFACE_FORMAT_R32G32B32UNORM                                    = 67, //!< No additional details
            SURFACE_FORMAT_R32G32B32SNORM                                    = 68, //!< No additional details
            SURFACE_FORMAT_R32G32B32SSCALED                                  = 69, //!< No additional details
            SURFACE_FORMAT_R32G32B32USCALED                                  = 70, //!< No additional details
            SURFACE_FORMAT_R32G32B32SFIXED                                   = 80, //!< No additional details
            SURFACE_FORMAT_R16G16B16A16UNORM                                 = 128, //!< No additional details
            SURFACE_FORMAT_R16G16B16A16SNORM                                 = 129, //!< No additional details
            SURFACE_FORMAT_R16G16B16A16SINT                                  = 130, //!< No additional details
            SURFACE_FORMAT_R16G16B16A16UINT                                  = 131, //!< No additional details
            SURFACE_FORMAT_R16G16B16A16FLOAT                                 = 132, //!< No additional details
            SURFACE_FORMAT_R32G32FLOAT                                       = 133, //!< No additional details
            SURFACE_FORMAT_R32G32SINT                                        = 134, //!< No additional details
            SURFACE_FORMAT_R32G32UINT                                        = 135, //!< No additional details
            SURFACE_FORMAT_R32FLOATX8X24TYPELESS                             = 136, //!< No additional details
            SURFACE_FORMAT_X32TYPELESSG8X24UINT                              = 137, //!< No additional details
            SURFACE_FORMAT_L32A32FLOAT                                       = 138, //!< No additional details
            SURFACE_FORMAT_R32G32UNORM                                       = 139, //!< No additional details
            SURFACE_FORMAT_R32G32SNORM                                       = 140, //!< No additional details
            SURFACE_FORMAT_R64FLOAT                                          = 141, //!< No additional details
            SURFACE_FORMAT_R16G16B16X16UNORM                                 = 142, //!< No additional details
            SURFACE_FORMAT_R16G16B16X16FLOAT                                 = 143, //!< No additional details
            SURFACE_FORMAT_A32X32FLOAT                                       = 144, //!< No additional details
            SURFACE_FORMAT_L32X32FLOAT                                       = 145, //!< No additional details
            SURFACE_FORMAT_I32X32FLOAT                                       = 146, //!< No additional details
            SURFACE_FORMAT_R16G16B16A16SSCALED                               = 147, //!< No additional details
            SURFACE_FORMAT_R16G16B16A16USCALED                               = 148, //!< No additional details
            SURFACE_FORMAT_R32G32SSCALED                                     = 149, //!< No additional details
            SURFACE_FORMAT_R32G32USCALED                                     = 150, //!< No additional details
            SURFACE_FORMAT_R32G32SFIXED                                      = 160, //!< No additional details
            SURFACE_FORMAT_R64PASSTHRU                                       = 161, //!< No additional details
            SURFACE_FORMAT_B8G8R8A8UNORM                                     = 192, //!< No additional details
            SURFACE_FORMAT_B8G8R8A8UNORMSRGB                                 = 193, //!< No additional details
            SURFACE_FORMAT_R10G10B10A2UNORM                                  = 194, //!< No additional details
            SURFACE_FORMAT_R10G10B10A2UNORMSRGB                              = 195, //!< No additional details
            SURFACE_FORMAT_R10G10B10A2UINT                                   = 196, //!< No additional details
            SURFACE_FORMAT_R10G10B10SNORMA2UNORM                             = 197, //!< No additional details
            SURFACE_FORMAT_R8G8B8A8UNORM                                     = 199, //!< No additional details
            SURFACE_FORMAT_R8G8B8A8UNORMSRGB                                 = 200, //!< No additional details
            SURFACE_FORMAT_R8G8B8A8SNORM                                     = 201, //!< No additional details
            SURFACE_FORMAT_R8G8B8A8SINT                                      = 202, //!< No additional details
            SURFACE_FORMAT_R8G8B8A8UINT                                      = 203, //!< No additional details
            SURFACE_FORMAT_R16G16UNORM                                       = 204, //!< No additional details
            SURFACE_FORMAT_R16G16SNORM                                       = 205, //!< No additional details
            SURFACE_FORMAT_R16G16SINT                                        = 206, //!< No additional details
            SURFACE_FORMAT_R16G16UINT                                        = 207, //!< No additional details
            SURFACE_FORMAT_R16G16FLOAT                                       = 208, //!< No additional details
            SURFACE_FORMAT_B10G10R10A2UNORM                                  = 209, //!< No additional details
            SURFACE_FORMAT_B10G10R10A2UNORMSRGB                              = 210, //!< No additional details
            SURFACE_FORMAT_R11G11B10FLOAT                                    = 211, //!< No additional details
            SURFACE_FORMAT_R32SINT                                           = 214, //!< No additional details
            SURFACE_FORMAT_R32UINT                                           = 215, //!< No additional details
            SURFACE_FORMAT_R32FLOAT                                          = 216, //!< No additional details
            SURFACE_FORMAT_R24UNORMX8TYPELESS                                = 217, //!< No additional details
            SURFACE_FORMAT_X24TYPELESSG8UINT                                 = 218, //!< No additional details
            SURFACE_FORMAT_L32UNORM                                          = 221, //!< No additional details
            SURFACE_FORMAT_A32UNORM                                          = 222, //!< No additional details
            SURFACE_FORMAT_L16A16UNORM                                       = 223, //!< No additional details
            SURFACE_FORMAT_I24X8UNORM                                        = 224, //!< No additional details
            SURFACE_FORMAT_L24X8UNORM                                        = 225, //!< No additional details
            SURFACE_FORMAT_A24X8UNORM                                        = 226, //!< No additional details
            SURFACE_FORMAT_I32FLOAT                                          = 227, //!< No additional details
            SURFACE_FORMAT_L32FLOAT                                          = 228, //!< No additional details
            SURFACE_FORMAT_A32FLOAT                                          = 229, //!< No additional details
            SURFACE_FORMAT_X8B8UNORMG8R8SNORM                                = 230, //!< No additional details
            SURFACE_FORMAT_A8X8UNORMG8R8SNORM                                = 231, //!< No additional details
            SURFACE_FORMAT_B8X8UNORMG8R8SNORM                                = 232, //!< No additional details
            SURFACE_FORMAT_B8G8R8X8UNORM                                     = 233, //!< No additional details
            SURFACE_FORMAT_B8G8R8X8UNORMSRGB                                 = 234, //!< No additional details
            SURFACE_FORMAT_R8G8B8X8UNORM                                     = 235, //!< No additional details
            SURFACE_FORMAT_R8G8B8X8UNORMSRGB                                 = 236, //!< No additional details
            SURFACE_FORMAT_R9G9B9E5SHAREDEXP                                 = 237, //!< No additional details
            SURFACE_FORMAT_B10G10R10X2UNORM                                  = 238, //!< No additional details
            SURFACE_FORMAT_L16A16FLOAT                                       = 240, //!< No additional details
            SURFACE_FORMAT_R32UNORM                                          = 241, //!< No additional details
            SURFACE_FORMAT_R32SNORM                                          = 242, //!< No additional details
            SURFACE_FORMAT_R10G10B10X2USCALED                                = 243, //!< No additional details
            SURFACE_FORMAT_R8G8B8A8SSCALED                                   = 244, //!< No additional details
            SURFACE_FORMAT_R8G8B8A8USCALED                                   = 245, //!< No additional details
            SURFACE_FORMAT_R16G16SSCALED                                     = 246, //!< No additional details
            SURFACE_FORMAT_R16G16USCALED                                     = 247, //!< No additional details
            SURFACE_FORMAT_R32SSCALED                                        = 248, //!< No additional details
            SURFACE_FORMAT_R32USCALED                                        = 249, //!< No additional details
            SURFACE_FORMAT_B5G6R5UNORM                                       = 256, //!< No additional details
            SURFACE_FORMAT_B5G6R5UNORMSRGB                                   = 257, //!< No additional details
            SURFACE_FORMAT_B5G5R5A1UNORM                                     = 258, //!< No additional details
            SURFACE_FORMAT_B5G5R5A1UNORMSRGB                                 = 259, //!< No additional details
            SURFACE_FORMAT_B4G4R4A4UNORM                                     = 260, //!< No additional details
            SURFACE_FORMAT_B4G4R4A4UNORMSRGB                                 = 261, //!< No additional details
            SURFACE_FORMAT_R8G8UNORM                                         = 262, //!< No additional details
            SURFACE_FORMAT_R8G8SNORM                                         = 263, //!< No additional details
            SURFACE_FORMAT_R8G8SINT                                          = 264, //!< No additional details
            SURFACE_FORMAT_R8G8UINT                                          = 265, //!< No additional details
            SURFACE_FORMAT_R16UNORM                                          = 266, //!< No additional details
            SURFACE_FORMAT_R16SNORM                                          = 267, //!< No additional details
            SURFACE_FORMAT_R16SINT                                           = 268, //!< No additional details
            SURFACE_FORMAT_R16UINT                                           = 269, //!< No additional details
            SURFACE_FORMAT_R16FLOAT                                          = 270, //!< No additional details
            SURFACE_FORMAT_A8P8UNORMPALETTE0                                 = 271, //!< No additional details
            SURFACE_FORMAT_A8P8UNORMPALETTE1                                 = 272, //!< No additional details
            SURFACE_FORMAT_I16UNORM                                          = 273, //!< No additional details
            SURFACE_FORMAT_L16UNORM                                          = 274, //!< No additional details
            SURFACE_FORMAT_A16UNORM                                          = 275, //!< No additional details
            SURFACE_FORMAT_L8A8UNORM                                         = 276, //!< No additional details
            SURFACE_FORMAT_I16FLOAT                                          = 277, //!< No additional details
            SURFACE_FORMAT_L16FLOAT                                          = 278, //!< No additional details
            SURFACE_FORMAT_A16FLOAT                                          = 279, //!< No additional details
            SURFACE_FORMAT_L8A8UNORMSRGB                                     = 280, //!< No additional details
            SURFACE_FORMAT_R5G5SNORMB6UNORM                                  = 281, //!< No additional details
            SURFACE_FORMAT_B5G5R5X1UNORM                                     = 282, //!< No additional details
            SURFACE_FORMAT_B5G5R5X1UNORMSRGB                                 = 283, //!< No additional details
            SURFACE_FORMAT_R8G8SSCALED                                       = 284, //!< No additional details
            SURFACE_FORMAT_R8G8USCALED                                       = 285, //!< No additional details
            SURFACE_FORMAT_R16SSCALED                                        = 286, //!< No additional details
            SURFACE_FORMAT_R16USCALED                                        = 287, //!< No additional details
            SURFACE_FORMAT_P8A8UNORMPALETTE0                                 = 290, //!< No additional details
            SURFACE_FORMAT_P8A8UNORMPALETTE1                                 = 291, //!< No additional details
            SURFACE_FORMAT_A1B5G5R5UNORM                                     = 292, //!< No additional details
            SURFACE_FORMAT_A4B4G4R4UNORM                                     = 293, //!< No additional details
            SURFACE_FORMAT_L8A8UINT                                          = 294, //!< No additional details
            SURFACE_FORMAT_L8A8SINT                                          = 295, //!< No additional details
            SURFACE_FORMAT_R8UNORM                                           = 320, //!< No additional details
            SURFACE_FORMAT_R8SNORM                                           = 321, //!< No additional details
            SURFACE_FORMAT_R8SINT                                            = 322, //!< No additional details
            SURFACE_FORMAT_R8UINT                                            = 323, //!< No additional details
            SURFACE_FORMAT_A8UNORM                                           = 324, //!< No additional details
            SURFACE_FORMAT_I8UNORM                                           = 325, //!< No additional details
            SURFACE_FORMAT_L8UNORM                                           = 326, //!< No additional details
            SURFACE_FORMAT_P4A4UNORMPALETTE0                                 = 327, //!< No additional details
            SURFACE_FORMAT_A4P4UNORMPALETTE0                                 = 328, //!< No additional details
            SURFACE_FORMAT_R8SSCALED                                         = 329, //!< No additional details
            SURFACE_FORMAT_R8USCALED                                         = 330, //!< No additional details
            SURFACE_FORMAT_P8UNORMPALETTE0                                   = 331, //!< No additional details
            SURFACE_FORMAT_L8UNORMSRGB                                       = 332, //!< No additional details
            SURFACE_FORMAT_P8UNORMPALETTE1                                   = 333, //!< No additional details
            SURFACE_FORMAT_P4A4UNORMPALETTE1                                 = 334, //!< No additional details
            SURFACE_FORMAT_A4P4UNORMPALETTE1                                 = 335, //!< No additional details
            SURFACE_FORMAT_Y8UNORM                                           = 336, //!< No additional details
            SURFACE_FORMAT_L8UINT                                            = 338, //!< No additional details
            SURFACE_FORMAT_L8SINT                                            = 339, //!< No additional details
            SURFACE_FORMAT_I8UINT                                            = 340, //!< No additional details
            SURFACE_FORMAT_I8SINT                                            = 341, //!< No additional details
            SURFACE_FORMAT_DXT1RGBSRGB                                       = 384, //!< No additional details
            SURFACE_FORMAT_R1UNORM                                           = 385, //!< No additional details
            SURFACE_FORMAT_YCRCBNORMAL                                       = 386, //!< No additional details
            SURFACE_FORMAT_YCRCBSWAPUVY                                      = 387, //!< No additional details
            SURFACE_FORMAT_P2UNORMPALETTE0                                   = 388, //!< No additional details
            SURFACE_FORMAT_P2UNORMPALETTE1                                   = 389, //!< No additional details
            SURFACE_FORMAT_BC1UNORM                                          = 390, //!< No additional details
            SURFACE_FORMAT_BC2UNORM                                          = 391, //!< No additional details
            SURFACE_FORMAT_BC3UNORM                                          = 392, //!< No additional details
            SURFACE_FORMAT_BC4UNORM                                          = 393, //!< No additional details
            SURFACE_FORMAT_BC5UNORM                                          = 394, //!< No additional details
            SURFACE_FORMAT_BC1UNORMSRGB                                      = 395, //!< No additional details
            SURFACE_FORMAT_BC2UNORMSRGB                                      = 396, //!< No additional details
            SURFACE_FORMAT_BC3UNORMSRGB                                      = 397, //!< No additional details
            SURFACE_FORMAT_MONO8                                             = 398, //!< No additional details
            SURFACE_FORMAT_YCRCBSWAPUV                                       = 399, //!< No additional details
            SURFACE_FORMAT_YCRCBSWAPY                                        = 400, //!< No additional details
            SURFACE_FORMAT_DXT1RGB                                           = 401, //!< No additional details
            SURFACE_FORMAT_FXT1                                              = 402, //!< No additional details
            SURFACE_FORMAT_R8G8B8UNORM                                       = 403, //!< No additional details
            SURFACE_FORMAT_R8G8B8SNORM                                       = 404, //!< No additional details
            SURFACE_FORMAT_R8G8B8SSCALED                                     = 405, //!< No additional details
            SURFACE_FORMAT_R8G8B8USCALED                                     = 406, //!< No additional details
            SURFACE_FORMAT_R64G64B64A64FLOAT                                 = 407, //!< No additional details
            SURFACE_FORMAT_R64G64B64FLOAT                                    = 408, //!< No additional details
            SURFACE_FORMAT_BC4SNORM                                          = 409, //!< No additional details
            SURFACE_FORMAT_BC5SNORM                                          = 410, //!< No additional details
            SURFACE_FORMAT_R16G16B16FLOAT                                    = 411, //!< No additional details
            SURFACE_FORMAT_R16G16B16UNORM                                    = 412, //!< No additional details
            SURFACE_FORMAT_R16G16B16SNORM                                    = 413, //!< No additional details
            SURFACE_FORMAT_R16G16B16SSCALED                                  = 414, //!< No additional details
            SURFACE_FORMAT_R16G16B16USCALED                                  = 415, //!< No additional details
            SURFACE_FORMAT_BC6HSF16                                          = 417, //!< No additional details
            SURFACE_FORMAT_BC7UNORM                                          = 418, //!< No additional details
            SURFACE_FORMAT_BC7UNORMSRGB                                      = 419, //!< No additional details
            SURFACE_FORMAT_BC6HUF16                                          = 420, //!< No additional details
            SURFACE_FORMAT_PLANAR4208                                        = 421, //!< No additional details
            SURFACE_FORMAT_R8G8B8UNORMSRGB                                   = 424, //!< No additional details
            SURFACE_FORMAT_ETC1RGB8                                          = 425, //!< No additional details
            SURFACE_FORMAT_ETC2RGB8                                          = 426, //!< No additional details
            SURFACE_FORMAT_EACR11                                            = 427, //!< No additional details
            SURFACE_FORMAT_EACRG11                                           = 428, //!< No additional details
            SURFACE_FORMAT_EACSIGNEDR11                                      = 429, //!< No additional details
            SURFACE_FORMAT_EACSIGNEDRG11                                     = 430, //!< No additional details
            SURFACE_FORMAT_ETC2SRGB8                                         = 431, //!< No additional details
            SURFACE_FORMAT_R16G16B16UINT                                     = 432, //!< No additional details
            SURFACE_FORMAT_R16G16B16SINT                                     = 433, //!< No additional details
            SURFACE_FORMAT_R32SFIXED                                         = 434, //!< No additional details
            SURFACE_FORMAT_R10G10B10A2SNORM                                  = 435, //!< No additional details
            SURFACE_FORMAT_R10G10B10A2USCALED                                = 436, //!< No additional details
            SURFACE_FORMAT_R10G10B10A2SSCALED                                = 437, //!< No additional details
            SURFACE_FORMAT_R10G10B10A2SINT                                   = 438, //!< No additional details
            SURFACE_FORMAT_B10G10R10A2SNORM                                  = 439, //!< No additional details
            SURFACE_FORMAT_B10G10R10A2USCALED                                = 440, //!< No additional details
            SURFACE_FORMAT_B10G10R10A2SSCALED                                = 441, //!< No additional details
            SURFACE_FORMAT_B10G10R10A2UINT                                   = 442, //!< No additional details
            SURFACE_FORMAT_B10G10R10A2SINT                                   = 443, //!< No additional details
            SURFACE_FORMAT_R64G64B64A64PASSTHRU                              = 444, //!< No additional details
            SURFACE_FORMAT_R64G64B64PASSTHRU                                 = 445, //!< No additional details
            SURFACE_FORMAT_ETC2RGB8PTA                                       = 448, //!< No additional details
            SURFACE_FORMAT_ETC2SRGB8PTA                                      = 449, //!< No additional details
            SURFACE_FORMAT_ETC2EACRGBA8                                      = 450, //!< No additional details
            SURFACE_FORMAT_ETC2EACSRGB8A8                                    = 451, //!< No additional details
            SURFACE_FORMAT_R8G8B8UINT                                        = 456, //!< No additional details
            SURFACE_FORMAT_R8G8B8SINT                                        = 457, //!< No additional details
            SURFACE_FORMAT_RAW                                               = 511, //!< No additional details
        };

        //! \brief SURFACE_TYPE
        //! \details
        //!     This field defines the type of the surface.
        enum SURFACE_TYPE
        {
            SURFACE_TYPE_SURFTYPE1D                                          = 0, //!< Defines a 1-dimensional map or array of maps
            SURFACE_TYPE_SURFTYPE2D                                          = 1, //!< Defines a 2-dimensional map or array of maps
            SURFACE_TYPE_SURFTYPE3D                                          = 2, //!< Defines a 3-dimensional (volumetric) map
            SURFACE_TYPE_SURFTYPECUBE                                        = 3, //!< Defines a cube map or array of cube maps
            SURFACE_TYPE_SURFTYPEBUFFER                                      = 4, //!< Defines an element in a buffer
            SURFACE_TYPE_SURFTYPESTRBUF                                      = 5, //!< Defines a structured buffer surface
            SURFACE_TYPE_SURFTYPENULL                                        = 7, //!< Defines a null surface
        };

        //! \brief NUMBER_OF_MULTISAMPLES
        //! \details
        //!     This field indicates the number of multisamples on the surface.
        enum NUMBER_OF_MULTISAMPLES
        {
            NUMBER_OF_MULTISAMPLES_MULTISAMPLECOUNT1                         = 0, //!< No additional details
            NUMBER_OF_MULTISAMPLES_MULTISAMPLECOUNT2                         = 1, //!< No additional details
            NUMBER_OF_MULTISAMPLES_MULTISAMPLECOUNT4                         = 2, //!< No additional details
            NUMBER_OF_MULTISAMPLES_MULTISAMPLECOUNT8                         = 3, //!< No additional details
            NUMBER_OF_MULTISAMPLES_MULTISAMPLECOUNT16                        = 4, //!< No additional details
        };

        //! \brief MULTISAMPLED_SURFACE_STORAGE_FORMAT
        //! \details
        //!     This field indicates the storage format of the multisampled surface.
        enum MULTISAMPLED_SURFACE_STORAGE_FORMAT
        {
            MULTISAMPLED_SURFACE_STORAGE_FORMAT_MSS                          = 0, //!< Multisampled surface was/is rendered as a render target
            MULTISAMPLED_SURFACE_STORAGE_FORMAT_DEPTHSTENCIL                 = 1, //!< Multisampled surface was rendered as a depth or stencil buffer
        };

        //! \brief RENDER_TARGET_AND_SAMPLE_UNORM_ROTATION
        //! \details
        //!     <b>For Render Target Surfaces:</b>
        //!                         This field specifies the rotation of this render target surface
        //!     when being written to memory.
        enum RENDER_TARGET_AND_SAMPLE_UNORM_ROTATION
        {
            RENDER_TARGET_AND_SAMPLE_UNORM_ROTATION_0DEG                     = 0, //!< No rotation (0 degrees)
            RENDER_TARGET_AND_SAMPLE_UNORM_ROTATION_90DEG                    = 1, //!< Rotate by 90 degrees
            RENDER_TARGET_AND_SAMPLE_UNORM_ROTATION_180DEG                   = 2, //!< Rotate by 180 degrees [for sample_unorm message]
            RENDER_TARGET_AND_SAMPLE_UNORM_ROTATION_270DEG                   = 3, //!< Rotate by 270 degrees
        };

        //! \brief COHERENCY_TYPE
        //! \details
        //!     Specifies the type of coherency maintained for this surface.
        enum COHERENCY_TYPE
        {
            COHERENCY_TYPE_GPUCOHERENT                                       = 0, //!< Surface memory is kept coherent with GPU threads using GPU read/write ordering rules. Surface memory is backed by system memory but is not kept coherent with CPU (LLC).
            COHERENCY_TYPE_IACOHERENT                                        = 1, //!< Surface memory is kept coherent with CPU (LLC).
        };

        //! \brief TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Sampling Engine, Render Target, and Typed/Untyped Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        //! 
        //!                         <b>For other surfaces:</b>
        //!                         This field is ignored.
        enum TILED_RESOURCE_MODE
        {
            TILED_RESOURCE_MODE_NONE                                         = 0, //!< No tiled resource
            TILED_RESOURCE_MODE_4KB                                          = 1, //!< 4KB tiled resources
            TILED_RESOURCE_MODE_TILEYF                                       = 1, //!< 4KB tiled resources
            TILED_RESOURCE_MODE_64KB                                         = 2, //!< 64KB tiled resources
            TILED_RESOURCE_MODE_TILEYS                                       = 2, //!< 64KB tiled resources
        };

        //! \brief EWA_DISABLE_FOR_CUBE
        //! \details
        //!     Specifies if EWA mode for LOD quality improvement needs to be disabled
        //!     for cube maps.
        enum EWA_DISABLE_FOR_CUBE
        {
            EWA_DISABLE_FOR_CUBE_ENABLE                                      = 0, //!< EWA is enabled for cube maps
            EWA_DISABLE_FOR_CUBE_DISABLE                                     = 1, //!< EWA is disabled for cube maps
        };

        //! \brief AUXILIARY_SURFACE_MODE
        //! \details
        //!     Specifies what type of surface the Auxiliary surface is.  The Auxiliary
        //!     surface has its own base address and pitch, but otherwise shares or
        //!     overrides other fields set for the primary surface, detailed in the
        //!     programming notes below.
        enum AUXILIARY_SURFACE_MODE
        {
            AUXILIARY_SURFACE_MODE_AUXNONE                                   = 0, //!< No Auxiliary surface is used
            AUXILIARY_SURFACE_MODE_AUXCCSD                                   = 1, //!< The Auxiliary surface is a CCS (Color Control Surface) with compression disabled or an MCS with compression enabled, depending on Number of Multisamples.  MCS (Multisample Control Surface) is a special type of CCS.
            AUXILIARY_SURFACE_MODE_AUXAPPEND                                 = 2, //!< The Auxiliary surface is an append buffer
            AUXILIARY_SURFACE_MODE_AUXHIZ                                    = 3, //!< The Auxiliary surface is a hierarchical depth buffer
            AUXILIARY_SURFACE_MODE_AUXCCSE                                   = 5, //!< The Auxiliary surface is a CCS with compression enabled or an MCS with compression enabled, depending on Number of Multisamples.
        };

        //! \brief SHADER_CHANNEL_SELECT_ALPHA
        //! \details
        //!     See <b>Shader Channel Select Red</b> for details.
        enum SHADER_CHANNEL_SELECT_ALPHA
        {
            SHADER_CHANNEL_SELECT_ALPHA_ZERO                                 = 0, //!< No additional details
            SHADER_CHANNEL_SELECT_ALPHA_ONE                                  = 1, //!< No additional details
            SHADER_CHANNEL_SELECT_ALPHA_RED                                  = 4, //!< No additional details
            SHADER_CHANNEL_SELECT_ALPHA_GREEN                                = 5, //!< No additional details
            SHADER_CHANNEL_SELECT_ALPHA_BLUE                                 = 6, //!< No additional details
            SHADER_CHANNEL_SELECT_ALPHA_ALPHA                                = 7, //!< No additional details
        };

        //! \brief SHADER_CHANNEL_SELECT_BLUE
        //! \details
        //!     See <b>Shader Channel Select Red</b> for details.
        enum SHADER_CHANNEL_SELECT_BLUE
        {
            SHADER_CHANNEL_SELECT_BLUE_ZERO                                  = 0, //!< No additional details
            SHADER_CHANNEL_SELECT_BLUE_ONE                                   = 1, //!< No additional details
            SHADER_CHANNEL_SELECT_BLUE_RED                                   = 4, //!< No additional details
            SHADER_CHANNEL_SELECT_BLUE_GREEN                                 = 5, //!< No additional details
            SHADER_CHANNEL_SELECT_BLUE_BLUE                                  = 6, //!< No additional details
            SHADER_CHANNEL_SELECT_BLUE_ALPHA                                 = 7, //!< No additional details
        };

        //! \brief SHADER_CHANNEL_SELECT_GREEN
        //! \details
        //!     See <b>Shader Channel Select Red</b> for details.
        enum SHADER_CHANNEL_SELECT_GREEN
        {
            SHADER_CHANNEL_SELECT_GREEN_ZERO                                 = 0, //!< No additional details
            SHADER_CHANNEL_SELECT_GREEN_ONE                                  = 1, //!< No additional details
            SHADER_CHANNEL_SELECT_GREEN_RED                                  = 4, //!< No additional details
            SHADER_CHANNEL_SELECT_GREEN_GREEN                                = 5, //!< No additional details
            SHADER_CHANNEL_SELECT_GREEN_BLUE                                 = 6, //!< No additional details
            SHADER_CHANNEL_SELECT_GREEN_ALPHA                                = 7, //!< No additional details
        };

        //! \brief SHADER_CHANNEL_SELECT_RED
        //! \details
        //!     Specifies which surface channel is read or written in the Red shader
        //!     channel.
        enum SHADER_CHANNEL_SELECT_RED
        {
            SHADER_CHANNEL_SELECT_RED_ZERO                                   = 0, //!< No additional details
            SHADER_CHANNEL_SELECT_RED_ONE                                    = 1, //!< No additional details
            SHADER_CHANNEL_SELECT_RED_RED                                    = 4, //!< No additional details
            SHADER_CHANNEL_SELECT_RED_GREEN                                  = 5, //!< No additional details
            SHADER_CHANNEL_SELECT_RED_BLUE                                   = 6, //!< No additional details
            SHADER_CHANNEL_SELECT_RED_ALPHA                                  = 7, //!< No additional details
        };

        //! \brief MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression.
        enum MEMORY_COMPRESSION_MODE
        {
            MEMORY_COMPRESSION_MODE_HORIZONTAL                               = 0, //!< No additional details
            MEMORY_COMPRESSION_MODE_VERTICAL                                 = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        RENDER_SURFACE_STATE_CMD();

        static const size_t dwSize = 16;
        static const size_t byteSize = 64;
    };

    //!
    //! \brief MEDIA_SURFACE_STATE
    //! \details
    //!     This is the SURFACE_STATE used by only deinterlace, sample_8x8, and VME
    //!     messages.
    //!
    struct MEDIA_SURFACE_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0, 15)    ; //!< Reserved
                uint32_t                 YOffset                                          : __CODEGEN_BITFIELD(16, 19)    ; //!< Y Offset, [Surface Format] is one of Planar Formats
                uint32_t                 XOffset                                          : __CODEGEN_BITFIELD(20, 26)    ; //!< X Offset, [Surface Format] is one of Planar Formats
                uint32_t                 Reserved27                                       : __CODEGEN_BITFIELD(27, 29)    ; //!< Reserved, [Surface Format] is one of Planar Formats
                uint32_t                 Rotation                                         : __CODEGEN_BITFIELD(30, 31)    ; //!< ROTATION
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 CrVCbUPixelOffsetVDirection                      : __CODEGEN_BITFIELD( 0,  1)    ; //!< CRVCBU_PIXEL_OFFSET_V_DIRECTION
                uint32_t                 PictureStructure                                 : __CODEGEN_BITFIELD( 2,  3)    ; //!< PICTURE_STRUCTURE
                uint32_t                 Width                                            : __CODEGEN_BITFIELD( 4, 17)    ; //!< Width
                uint32_t                 Height                                           : __CODEGEN_BITFIELD(18, 31)    ; //!< Height
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 TileMode                                         : __CODEGEN_BITFIELD( 0,  1)    ; //!< TILE_MODE
                uint32_t                 HalfPitchForChroma                               : __CODEGEN_BITFIELD( 2,  2)    ; //!< Half Pitch for Chroma
                uint32_t                 SurfacePitch                                     : __CODEGEN_BITFIELD( 3, 20)    ; //!< Surface Pitch
                uint32_t                 AddressControl                                   : __CODEGEN_BITFIELD(21, 21)    ; //!< ADDRESS_CONTROL
                uint32_t                 MemoryCompressionEnable                          : __CODEGEN_BITFIELD(22, 22)    ; //!< Memory Compression Enable
                uint32_t                 MemoryCompressionMode                            : __CODEGEN_BITFIELD(23, 23)    ; //!< MEMORY_COMPRESSION_MODE
                uint32_t                 CrVCbUPixelOffsetVDirectionMsb                   : __CODEGEN_BITFIELD(24, 24)    ; //!< CRVCBU_PIXEL_OFFSET_V_DIRECTION_MSB
                uint32_t                 CrVCbUPixelOffsetUDirection                      : __CODEGEN_BITFIELD(25, 25)    ; //!< CRVCBU_PIXEL_OFFSET_U_DIRECTION
                uint32_t                 InterleaveChroma                                 : __CODEGEN_BITFIELD(26, 26)    ; //!< Interleave Chroma
                uint32_t                 SurfaceFormat                                    : __CODEGEN_BITFIELD(27, 31)    ; //!< SURFACE_FORMAT
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 YOffsetForUCb                                    : __CODEGEN_BITFIELD( 0, 13)    ; //!< Y Offset for U(Cb)
                uint32_t                 Reserved110                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 XOffsetForUCb                                    : __CODEGEN_BITFIELD(16, 29)    ; //!< X Offset for U(Cb)
                uint32_t                 Reserved126                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 YOffsetForVCr                                    : __CODEGEN_BITFIELD( 0, 14)    ; //!< Y Offset for V(Cr)
                uint32_t                 Reserved143                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 XOffsetForVCr                                    : __CODEGEN_BITFIELD(16, 29)    ; //!< X Offset for V(Cr)
                uint32_t                 Reserved158                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 SurfaceMemoryObjectControlState                  : __CODEGEN_BITFIELD( 0,  6)    ; //!< SURFACE_MEMORY_OBJECT_CONTROL_STATE
                uint32_t                 Reserved167                                      : __CODEGEN_BITFIELD( 7, 17)    ; //!< Reserved
                uint32_t                 TiledResourceMode                                : __CODEGEN_BITFIELD(18, 19)    ; //!< TILED_RESOURCE_MODE
                uint32_t                 Reserved180                                      : __CODEGEN_BITFIELD(20, 29)    ; //!< Reserved
                uint32_t                 VerticalLineStrideOffset                         : __CODEGEN_BITFIELD(30, 30)    ; //!< Vertical Line Stride Offset
                uint32_t                 VerticalLineStride                               : __CODEGEN_BITFIELD(31, 31)    ; //!< Vertical Line Stride
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 SurfaceBaseAddress                                                               ; //!< Surface Base Address
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 SurfaceBaseAddressHigh                           : __CODEGEN_BITFIELD( 0, 15)    ; //!< Surface Base Address High
                uint32_t                 Reserved240                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;

        //! \name Local enumerations

        //! \brief ROTATION
        //! \details
        //!     Rotation is only supported only with AVS function messages and not with
        //!     HDC direct write and 16x8 AVS messages.
        enum ROTATION
        {
            ROTATION_NOROTATIONOR0DEGREE                                     = 0, //!< No additional details
            ROTATION_90DEGREEROTATION                                        = 1, //!< No additional details
            ROTATION_180DEGREEROTATION                                       = 2, //!< No additional details
            ROTATION_270DEGREEROTATION                                       = 3, //!< No additional details
        };

        //! \brief CRVCBU_PIXEL_OFFSET_V_DIRECTION
        //! \details
        //!     Specifies the distance to the U/V values with respect to the even
        //!     numbered Y channels in the V direction
        enum CRVCBU_PIXEL_OFFSET_V_DIRECTION
        {
            CRVCBU_PIXEL_OFFSET_V_DIRECTION_UNNAMED0                         = 0, //!< No additional details
        };

        //! \brief PICTURE_STRUCTURE
        //! \details
        //!     Specifies the encoding of the current picture.
        enum PICTURE_STRUCTURE
        {
            PICTURE_STRUCTURE_FRAMEPICTURE                                   = 0, //!< No additional details
            PICTURE_STRUCTURE_TOPFIELDPICTURE                                = 1, //!< No additional details
            PICTURE_STRUCTURE_BOTTOMFIELDPICTURE                             = 2, //!< No additional details
            PICTURE_STRUCTURE_INVALID_NOTALLOWED                             = 3, //!< No additional details
        };

        //! \brief TILE_MODE
        //! \details
        //!     This field specifies the type of memory tiling (Linear, WMajor, XMajor,
        //!     or YMajor) employed to tile this surface. See Memory Interface Functions
        //!     for details on memory tiling and restrictions.
        enum TILE_MODE
        {
            TILE_MODE_TILEMODELINEAR                                         = 0, //!< Linear mode (no tiling)
            TILE_MODE_TILEMODEXMAJOR                                         = 2, //!< X major tiling
            TILE_MODE_TILEMODEYMAJOR                                         = 3, //!< Y major tiling
        };

        enum ADDRESS_CONTROL
        {
            ADDRESS_CONTROL_CLAMP                                            = 0, //!< Clamp
            ADDRESS_CONTROL_MIRROR                                           = 1, //!< Mirror
        };

        //! \brief MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes Vertical from Horizontal compression.
        enum MEMORY_COMPRESSION_MODE
        {
            MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE                = 0, //!< No additional details
            MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE                  = 1, //!< No additional details
        };

        //! \brief CRVCBU_PIXEL_OFFSET_V_DIRECTION_MSB
        //! \details
        //!     Specifies the distance to the U/V values with respect to the even
        //!     numbered Y channels in the V direction
        enum CRVCBU_PIXEL_OFFSET_V_DIRECTION_MSB
        {
            CRVCBU_PIXEL_OFFSET_V_DIRECTION_MSB_UNNAMED0                     = 0, //!< No additional details
        };

        //! \brief CRVCBU_PIXEL_OFFSET_U_DIRECTION
        //! \details
        //!     Specifies the distance to the U/V values with respect to the even
        //!     numbered Y channels in the U direction
        enum CRVCBU_PIXEL_OFFSET_U_DIRECTION
        {
            CRVCBU_PIXEL_OFFSET_U_DIRECTION_UNNAMED0                         = 0, //!< No additional details
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
            SURFACE_FORMAT_PLANAR4208                                        = 4, //!< No additional details
            SURFACE_FORMAT_Y8UNORMVA                                         = 5, //!< Sample_8x8 only except AVS
            SURFACE_FORMAT_Y16SNORM                                          = 6, //!< Sample_8x8 only except AVS
            SURFACE_FORMAT_Y16UNORMVA                                        = 7, //!< Sample_8x8 only except AVS
            SURFACE_FORMAT_R10G10B10A2UNORM                                  = 8, //!< Sample_8x8 only
            SURFACE_FORMAT_R8G8B8A8UNORM                                     = 9, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_R8B8UNORM_CRCB                                    = 10, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_R8UNORM_CRCB                                      = 11, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_Y8UNORM                                           = 12, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_A8Y8U8V8UNORM                                     = 13, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_B8G8R8A8UNORM                                     = 14, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_R16G16B16A16                                      = 15, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_Y1UNORM                                           = 16, //!< Sample_8x8 only for boolean surfaces (1bit/pixel)
            SURFACE_FORMAT_Y32UNORM                                          = 17, //!< For Integral Image (32bpp)
            SURFACE_FORMAT_PLANAR4228                                        = 18, //!< Sample_8x8 AVS only
        };

        //! \brief SURFACE_MEMORY_OBJECT_CONTROL_STATE
        //! \details
        //!     This 7-bit field is used in various state commands and indirect state
        //!     objects to define cacheability and other attributes related to memory
        //!     objects.
        enum SURFACE_MEMORY_OBJECT_CONTROL_STATE
        {
            SURFACE_MEMORY_OBJECT_CONTROL_STATE_DEFAULTVAUEDESC              = 0, //!< No additional details
        };

        //! \brief TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Sampling Engine, Render Target, and Typed/Untyped Surfaces:</b>
        //!                         This field specifies the tiled resource mode.
        //!                         <b>For other surfaces:</b>
        //!                         This field is ignored.
        enum TILED_RESOURCE_MODE
        {
            TILED_RESOURCE_MODE_TRMODENONE                                   = 0, //!< No tiled resource
            TILED_RESOURCE_MODE_TRMODETILEYF                                 = 1, //!< 4KB tiled resources
            TILED_RESOURCE_MODE_TRMODETILEYS                                 = 2, //!< 64KB tiled resources
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MEDIA_SURFACE_STATE_CMD();

        static const size_t dwSize = 8;
        static const size_t byteSize = 32;
    };

    //!
    //! \brief SAMPLER_STATE
    //! \details
    //!     This is the normal sampler state used by all messages that use
    //!     SAMPLER_STATE except sample_8x8 and deinterlace. The sampler state is
    //!     stored as an array of up to 16 elements, each of which contains the
    //!     dwords described here. The start of each element is spaced 4 dwords
    //!     apart. The first element of the sampler state array is aligned to a
    //!     32-byte boundary.
    //!
    struct SAMPLER_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 LodAlgorithm                                     : __CODEGEN_BITFIELD( 0,  0)    ; //!< LOD_ALGORITHM
                uint32_t                 TextureLodBias                                   : __CODEGEN_BITFIELD( 1, 13)    ; //!< Texture LOD Bias
                uint32_t                 MinModeFilter                                    : __CODEGEN_BITFIELD(14, 16)    ; //!< MIN_MODE_FILTER
                uint32_t                 MagModeFilter                                    : __CODEGEN_BITFIELD(17, 19)    ; //!< MAG_MODE_FILTER
                uint32_t                 MipModeFilter                                    : __CODEGEN_BITFIELD(20, 21)    ; //!< MIP_MODE_FILTER
                uint32_t                 CoarseLodQualityMode                             : __CODEGEN_BITFIELD(22, 26)    ; //!< COARSE_LOD_QUALITY_MODE
                uint32_t                 LodPreclampMode                                  : __CODEGEN_BITFIELD(27, 28)    ; //!< LOD_PRECLAMP_MODE
                uint32_t                 TextureBorderColorMode                           : __CODEGEN_BITFIELD(29, 29)    ; //!< TEXTURE_BORDER_COLOR_MODE
                uint32_t                 Reserved30                                       : __CODEGEN_BITFIELD(30, 30)    ; //!< Reserved
                uint32_t                 SamplerDisable                                   : __CODEGEN_BITFIELD(31, 31)    ; //!< Sampler Disable
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 CubeSurfaceControlMode                           : __CODEGEN_BITFIELD( 0,  0)    ; //!< CUBE_SURFACE_CONTROL_MODE
                uint32_t                 ShadowFunction                                   : __CODEGEN_BITFIELD( 1,  3)    ; //!< SHADOW_FUNCTION
                uint32_t                 ChromakeyMode                                    : __CODEGEN_BITFIELD( 4,  4)    ; //!< CHROMAKEY_MODE
                uint32_t                 ChromakeyIndex                                   : __CODEGEN_BITFIELD( 5,  6)    ; //!< ChromaKey Index
                uint32_t                 ChromakeyEnable                                  : __CODEGEN_BITFIELD( 7,  7)    ; //!< ChromaKey Enable
                uint32_t                 MaxLod                                           : __CODEGEN_BITFIELD( 8, 19)    ; //!< Max LOD
                uint32_t                 MinLod                                           : __CODEGEN_BITFIELD(20, 31)    ; //!< Min LOD
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 LodClampMagnificationMode                        : __CODEGEN_BITFIELD( 0,  0)    ; //!< LOD_CLAMP_MAGNIFICATION_MODE
                uint32_t                 Reserved65                                       : __CODEGEN_BITFIELD( 1,  5)    ; //!< Reserved
                uint32_t                 IndirectStatePointer                             : __CODEGEN_BITFIELD( 6, 23)    ; //!< Indirect State Pointer
                uint32_t                 Reserved88                                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 TczAddressControlMode                            : __CODEGEN_BITFIELD( 0,  2)    ; //!< TCZ_ADDRESS_CONTROL_MODE
                uint32_t                 TcyAddressControlMode                            : __CODEGEN_BITFIELD( 3,  5)    ; //!< TCY_ADDRESS_CONTROL_MODE
                uint32_t                 TcxAddressControlMode                            : __CODEGEN_BITFIELD( 6,  8)    ; //!< TCX_ADDRESS_CONTROL_MODE
                uint32_t                 ReductionTypeEnable                              : __CODEGEN_BITFIELD( 9,  9)    ; //!< Reduction Type Enable
                uint32_t                 NonNormalizedCoordinateEnable                    : __CODEGEN_BITFIELD(10, 10)    ; //!< Non-normalized Coordinate Enable
                uint32_t                 TrilinearFilterQuality                           : __CODEGEN_BITFIELD(11, 12)    ; //!< TRILINEAR_FILTER_QUALITY
                uint32_t                 RAddressMinFilterRoundingEnable                  : __CODEGEN_BITFIELD(13, 13)    ; //!< R Address Min Filter Rounding Enable
                uint32_t                 RAddressMagFilterRoundingEnable                  : __CODEGEN_BITFIELD(14, 14)    ; //!< R Address Mag Filter Rounding Enable
                uint32_t                 VAddressMinFilterRoundingEnable                  : __CODEGEN_BITFIELD(15, 15)    ; //!< V Address Min Filter Rounding Enable
                uint32_t                 VAddressMagFilterRoundingEnable                  : __CODEGEN_BITFIELD(16, 16)    ; //!< V Address Mag Filter Rounding Enable
                uint32_t                 UAddressMinFilterRoundingEnable                  : __CODEGEN_BITFIELD(17, 17)    ; //!< U Address Min Filter Rounding Enable
                uint32_t                 UAddressMagFilterRoundingEnable                  : __CODEGEN_BITFIELD(18, 18)    ; //!< U Address Mag Filter Rounding Enable
                uint32_t                 MaximumAnisotropy                                : __CODEGEN_BITFIELD(19, 21)    ; //!< MAXIMUM_ANISOTROPY
                uint32_t                 ReductionType                                    : __CODEGEN_BITFIELD(22, 23)    ; //!< REDUCTION_TYPE
                uint32_t                 Reserved120                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;

        //! \name Local enumerations

        //! \brief LOD_ALGORITHM
        //! \details
        //!     Controls which algorithm is used for LOD calculation. Generally, the EWA
        //!     approximation algorithm results in higher image quality than the legacy
        //!     algorithm.
        enum LOD_ALGORITHM
        {
            LOD_ALGORITHM_LEGACY                                             = 0, //!< Use the legacy algorithm for anisotropic filtering
            LOD_ALGORITHM_EWAAPPROXIMATION                                   = 1, //!< Use the new EWA approximation algorithm for anisotropic filtering
        };

        //! \brief MIN_MODE_FILTER
        //! \details
        //!     This field determines how texels are sampled/filtered when a texture is
        //!     being "minified" (shrunk). For volume maps, this filter mode selection
        //!     also applies to the 3rd (inter-layer) dimension.See Mag Mode Filter
        enum MIN_MODE_FILTER
        {
            MIN_MODE_FILTER_NEAREST                                          = 0, //!< Sample the nearest texel
            MIN_MODE_FILTER_LINEAR                                           = 1, //!< Bilinearly filter the 4 nearest texels
            MIN_MODE_FILTER_ANISOTROPIC                                      = 2, //!< Perform an "anisotropic" filter on the chosen mip level
            MIN_MODE_FILTER_MONO                                             = 6, //!< Perform a monochrome convolution filter
        };

        //! \brief MAG_MODE_FILTER
        //! \details
        //!     This field determines how texels are sampled/filtered when a texture is
        //!     being "magnified" (enlarged). For volume maps, this filter mode
        //!     selection also applies to the 3rd (inter-layer) dimension.
        enum MAG_MODE_FILTER
        {
            MAG_MODE_FILTER_NEAREST                                          = 0, //!< Sample the nearest texel
            MAG_MODE_FILTER_LINEAR                                           = 1, //!< Bilinearly filter the 4 nearest texels
            MAG_MODE_FILTER_ANISOTROPIC                                      = 2, //!< Perform an "anisotropic" filter on the chosen mip level
            MAG_MODE_FILTER_MONO                                             = 6, //!< Perform a monochrome convolution filter
        };

        //! \brief MIP_MODE_FILTER
        //! \details
        //!     This field determines if and how mip map levels are chosen and/or
        //!     combined when texture filtering.
        enum MIP_MODE_FILTER
        {
            MIP_MODE_FILTER_NONE                                             = 0, //!< Disable mip mapping - force use of the mipmap level corresponding to Min LOD.
            MIP_MODE_FILTER_NEAREST                                          = 1, //!< Nearest, Select the nearest mip map
            MIP_MODE_FILTER_LINEAR                                           = 3, //!< Linearly interpolate between nearest mip maps (combined with linear min/mag filters this is analogous to "Trilinear" filtering).
        };

        //! \brief COARSE_LOD_QUALITY_MODE
        //! \details
        //!     This field configures the coarse LOD image quality mode for the
        //!     sample_d, sample_l, and sample_b messages in the sampling engine. In
        //!     general, performance will increase and power consumption will decrease
        //!     with each step of reduced quality (performance gain for sample_l and
        //!     sample_b will be minimal).
        enum COARSE_LOD_QUALITY_MODE
        {
            COARSE_LOD_QUALITY_MODE_DISABLED                                 = 0, //!< Full quality is enabled, matching prior products
        };

        //! \brief LOD_PRECLAMP_MODE
        //! \details
        //!     <p>This field determines whether the computed LOD is clamped to
        //!     [max,min] mip level
        //!                         before the mag-vs-min determination is performed.</p>
        enum LOD_PRECLAMP_MODE
        {
            LOD_PRECLAMP_MODE_NONE                                           = 0, //!< LOD PreClamp disabled
            LOD_PRECLAMP_MODE_OGL                                            = 2, //!< LOD PreClamp enabled (OGL mode)
        };

        //! \brief TEXTURE_BORDER_COLOR_MODE
        //! \details
        //!     For some surface formats, the 32 bit border color is decoded differently
        //!     based on the border color mode. In addition, the default value of
        //!     channels not included in the surface may be affected by this field.
        //!     Refer to the "Sampler Output Channel Mapping" table for the values of
        //!     these channels, and for surface formats that may only support one of
        //!     these modes. Also refer to the definition of SAMPLER_BORDER_COLOR_STATE
        //!     for more details on the behavior of the two modes defined by this field.
        enum TEXTURE_BORDER_COLOR_MODE
        {
            TEXTURE_BORDER_COLOR_MODE_OGL  = 0, //!< OGL mode for interpreting the border color
            TEXTURE_BORDER_COLOR_MODE_8BIT = 1, //!< Earlier mode for interpreting the border color as UNORM8.
        };

        //! \brief CUBE_SURFACE_CONTROL_MODE
        //! \details
        //!     When sampling from a SURFTYPE_CUBE surface, this field controls whether
        //!     the TC* Address Control Mode fields are interpreted as programmed or
        //!     overridden to TEXCOORDMODE_CUBE.
        enum CUBE_SURFACE_CONTROL_MODE
        {
            CUBE_SURFACE_CONTROL_MODE_PROGRAMMED                             = 0, //!< No additional details
            CUBE_SURFACE_CONTROL_MODE_OVERRIDE                               = 1, //!< No additional details
        };

        //! \brief SHADOW_FUNCTION
        //! \details
        //!     This field is used for shadow mapping support via the sample_c message
        //!     type, and specifies the specific comparison operation to be used. The
        //!     comparison is between the texture sample red channel (except for
        //!     alpha-only formats which use the alpha channel), and the "ref" value
        //!     provided in the input message.
        enum SHADOW_FUNCTION
        {
            SHADOW_FUNCTION_PREFILTEROPALWAYS                                = 0, //!< No additional details
            SHADOW_FUNCTION_PREFILTEROPNEVER                                 = 1, //!< No additional details
            SHADOW_FUNCTION_PREFILTEROPLESS                                  = 2, //!< No additional details
            SHADOW_FUNCTION_PREFILTEROPEQUAL                                 = 3, //!< No additional details
            SHADOW_FUNCTION_PREFILTEROPLEQUAL                                = 4, //!< No additional details
            SHADOW_FUNCTION_PREFILTEROPGREATER                               = 5, //!< No additional details
            SHADOW_FUNCTION_PREFILTEROPNOTEQUAL                              = 6, //!< No additional details
            SHADOW_FUNCTION_PREFILTEROPGEQUAL                                = 7, //!< No additional details
        };

        //! \brief CHROMAKEY_MODE
        //! \details
        //!     This field specifies the behavior of the device in the event of a
        //!     ChromaKey match.  This field is ignored if ChromaKey is disabled.
        enum CHROMAKEY_MODE
        {
            CHROMAKEY_MODE_KEYFILTERKILLONANYMATCH                           = 0, //!< In this mode, if any contributing texel matches the chroma key, the corresponding pixel mask bit for that pixel is cleared.  The result of this operation is observable only if the Killed Pixel Mask Return flag is set on the input message.
            CHROMAKEY_MODE_KEYFILTERREPLACEBLACK                             = 1, //!< In this mode, each texel that matches the chroma key is replaced with (0,0,0,0) (black with alpha=0) prior to filtering.  For YCrCb surface formats, the black value is A=0, R(Cr)=0x80, G(Y)=0x10, B(Cb)=0x80.  This will tend to darken/fade edges of keyed regions.  Note that the pixel pipeline must be programmed to use the resulting filtered texel value to gain the intended effect, e.g., handle the case of a totally keyed-out region (filtered texel alpha==0) through use of alpha test, etc.
        };

        //! \brief LOD_CLAMP_MAGNIFICATION_MODE
        //! \details
        //!     This field allows the flexibility to control how LOD clamping is handled
        //!     when in magnification mode.
        enum LOD_CLAMP_MAGNIFICATION_MODE
        {
            LOD_CLAMP_MAGNIFICATION_MODE_MIPNONE                             = 0, //!< When in magnification mode, Sampler will clamp LOD as if the Mip Mode Filteris MIPFILTER_NONE. This is how OpenGL defines magnification, and therefore it isexpected that those drivers would not set this bit.
            LOD_CLAMP_MAGNIFICATION_MODE_MIPFILTER                           = 1, //!< When in magnification mode, Sampler will clamp LOD based on the value of Mip Mode Filter.
        };

        //! \brief TCZ_ADDRESS_CONTROL_MODE
        //! \details
        //!     Controls how the 3rd (TCZ) component of input texture coordinates are
        //!     mapped to texture map addresses - specifically, how coordinates
        //!     "outside" the texture are handled (wrap/clamp/mirror).See Address TCX
        //!     Control Mode above for details
        enum TCZ_ADDRESS_CONTROL_MODE
        {
            TCZ_ADDRESS_CONTROL_MODE_WRAP                                    = 0, //!< No additional details
            TCZ_ADDRESS_CONTROL_MODE_MIRROR                                  = 1, //!< No additional details
            TCZ_ADDRESS_CONTROL_MODE_CLAMP                                   = 2, //!< No additional details
            TCZ_ADDRESS_CONTROL_MODE_CUBE                                    = 3, //!< No additional details
            TCZ_ADDRESS_CONTROL_MODE_CLAMPBORDER                             = 4, //!< No additional details
            TCZ_ADDRESS_CONTROL_MODE_MIRRORONCE                              = 5, //!< No additional details
            TCZ_ADDRESS_CONTROL_MODE_HALFBORDER                              = 6, //!< No additional details
        };

        //! \brief TCY_ADDRESS_CONTROL_MODE
        //! \details
        //!     Controls how the 2nd (TCY, aka V) component of input texture coordinates
        //!     are mapped to texture map addresses - specifically, how coordinates
        //!     "outside" the texture are handled (wrap/clamp/mirror). See Address TCX
        //!     Control Mode above for details
        enum TCY_ADDRESS_CONTROL_MODE
        {
            TCY_ADDRESS_CONTROL_MODE_WRAP                                    = 0, //!< No additional details
            TCY_ADDRESS_CONTROL_MODE_MIRROR                                  = 1, //!< No additional details
            TCY_ADDRESS_CONTROL_MODE_CLAMP                                   = 2, //!< No additional details
            TCY_ADDRESS_CONTROL_MODE_CUBE                                    = 3, //!< No additional details
            TCY_ADDRESS_CONTROL_MODE_CLAMPBORDER                             = 4, //!< No additional details
            TCY_ADDRESS_CONTROL_MODE_MIRRORONCE                              = 5, //!< No additional details
            TCY_ADDRESS_CONTROL_MODE_HALFBORDER                              = 6, //!< No additional details
        };

        //! \brief TCX_ADDRESS_CONTROL_MODE
        //! \details
        //!     Controls how the 1st (TCX, aka U) component of input texture coordinates
        //!     are mapped to texture map addresses - specifically, how coordinates
        //!     "outside" the texture are handled (wrap/clamp/mirror). The setting of
        //!     this field is subject to being overridden by the Cube Surface Control
        //!     Mode field when sampling from a SURFTYPE_CUBE surface.
        enum TCX_ADDRESS_CONTROL_MODE
        {
            TCX_ADDRESS_CONTROL_MODE_WRAP                                    = 0, //!< No additional details
            TCX_ADDRESS_CONTROL_MODE_MIRROR                                  = 1, //!< No additional details
            TCX_ADDRESS_CONTROL_MODE_CLAMP                                   = 2, //!< No additional details
            TCX_ADDRESS_CONTROL_MODE_CUBE                                    = 3, //!< No additional details
            TCX_ADDRESS_CONTROL_MODE_CLAMPBORDER                             = 4, //!< No additional details
            TCX_ADDRESS_CONTROL_MODE_MIRRORONCE                              = 5, //!< No additional details
            TCX_ADDRESS_CONTROL_MODE_HALFBORDER                              = 6, //!< No additional details
        };

        //! \brief TRILINEAR_FILTER_QUALITY
        //! \details
        //!     Selects the quality level for the trilinear filter.
        enum TRILINEAR_FILTER_QUALITY
        {
            TRILINEAR_FILTER_QUALITY_FULL                                    = 0, //!< Full Quality. Both mip maps are sampled under all circumstances.
            TRILINEAR_FILTER_QUALITY_TRIQUALHIGHMAGCLAMPMIPFILTER            = 1, //!< High Quality.
            TRILINEAR_FILTER_QUALITY_MED                                     = 2, //!< Medium Quality.
            TRILINEAR_FILTER_QUALITY_LOW                                     = 3, //!< Low Quality.
        };

        //! \brief MAXIMUM_ANISOTROPY
        //! \details
        //!     This field clamps the maximum value of the anisotropy ratio used by the
        //!     MAPFILTER_ANISOTROPIC filter (Min or Mag Mode Filter).
        enum MAXIMUM_ANISOTROPY
        {
            MAXIMUM_ANISOTROPY_RATIO21                                       = 0, //!< At most a 2:1 aspect ratio filter is used
            MAXIMUM_ANISOTROPY_RATIO41                                       = 1, //!< At most a 4:1 aspect ratio filter is used
            MAXIMUM_ANISOTROPY_RATIO61                                       = 2, //!< At most a 6:1 aspect ratio filter is used
            MAXIMUM_ANISOTROPY_RATIO81                                       = 3, //!< At most a 8:1 aspect ratio filter is used
            MAXIMUM_ANISOTROPY_RATIO101                                      = 4, //!< At most a 10:1 aspect ratio filter is used
            MAXIMUM_ANISOTROPY_RATIO121                                      = 5, //!< At most a 12:1 aspect ratio filter is used
            MAXIMUM_ANISOTROPY_RATIO141                                      = 6, //!< At most a 14:1 aspect ratio filter is used
            MAXIMUM_ANISOTROPY_RATIO161                                      = 7, //!< At most a 16:1 aspect ratio filter is used
        };

        //! \brief REDUCTION_TYPE
        //! \details
        //!     This field defines the type of reduction that will be performed on the
        //!     texels in the footprint defined by the <b>Min/Mag/Mip Filter Mode</b>
        //!     fields.  This field is ignored if <b>Reduction Type Enable</b> is
        //!     disabled.
        enum REDUCTION_TYPE
        {
            REDUCTION_TYPE_STDFILTER                                         = 0, //!< standard filter
            REDUCTION_TYPE_COMPARISON                                        = 1, //!< comparison followed by standard filter
            REDUCTION_TYPE_MINIMUM                                           = 2, //!< minimum of footprint
            REDUCTION_TYPE_MAXIMUM                                           = 3, //!< maximum of footprint
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        SAMPLER_STATE_CMD();

        static const size_t dwSize = 4;
        static const size_t byteSize = 16;
    };

    //!
    //! \brief SAMPLER_STATE_8x8_AVS_COEFFICIENTS
    //! \details
    //!     ExistsIf = AVS &amp;&amp; (Function_mode = 0)
    //!
    struct SAMPLER_STATE_8x8_AVS_COEFFICIENTS_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 Table0XFilterCoefficientN0                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Table 0X Filter Coefficient[n,0]
                uint32_t                 Table0YFilterCoefficientN0                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Table 0Y Filter Coefficient[n,0]
                uint32_t                 Table0XFilterCoefficientN1                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Table 0X Filter Coefficient[n,1]
                uint32_t                 Table0YFilterCoefficientN1                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Table 0Y Filter Coefficient[n,1]
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 Table0XFilterCoefficientN2                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Table 0X Filter Coefficient[n,2]
                uint32_t                 Table0YFilterCoefficientN2                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Table 0Y Filter Coefficient[n,2]
                uint32_t                 Table0XFilterCoefficientN3                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Table 0X Filter Coefficient[n,3]
                uint32_t                 Table0YFilterCoefficientN3                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Table 0Y Filter Coefficient[n,3]
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 Table0XFilterCoefficientN4                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Table 0X Filter Coefficient[n,4]
                uint32_t                 Table0YFilterCoefficientN4                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Table 0Y Filter Coefficient[n,4]
                uint32_t                 Table0XFilterCoefficientN5                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Table 0X Filter Coefficient[n,5]
                uint32_t                 Table0YFilterCoefficientN5                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Table 0Y Filter Coefficient[n,5]
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 Table0XFilterCoefficientN6                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Table 0X Filter Coefficient[n,6]
                uint32_t                 Table0YFilterCoefficientN6                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Table 0Y Filter Coefficient[n,6]
                uint32_t                 Table0XFilterCoefficientN7                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Table 0X Filter Coefficient[n,7]
                uint32_t                 Table0YFilterCoefficientN7                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Table 0Y Filter Coefficient[n,7]
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 Reserved128                                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Reserved
                uint32_t                 Table1XFilterCoefficientN2                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Table 1X Filter Coefficient[n,2]
                uint32_t                 Table1XFilterCoefficientN3                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Table 1X Filter Coefficient[n,3]
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 Table1XFilterCoefficientN4                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Table 1X Filter Coefficient[n,4]
                uint32_t                 Table1XFilterCoefficientN5                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Table 1X Filter Coefficient[n,5]
                uint32_t                 Reserved176                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 Reserved192                                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Reserved
                uint32_t                 Table1YFilterCoefficientN2                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Table 1Y Filter Coefficient[n,2]
                uint32_t                 Table1YFilterCoefficientN3                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Table 1Y Filter Coefficient[n,3]
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 Table1YFilterCoefficientN4                       : __CODEGEN_BITFIELD( 0,  7)    ; //!< Table 1Y Filter Coefficient[n,4]
                uint32_t                 Table1YFilterCoefficientN5                       : __CODEGEN_BITFIELD( 8, 15)    ; //!< Table 1Y Filter Coefficient[n,5]
                uint32_t                 Reserved240                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        SAMPLER_STATE_8x8_AVS_COEFFICIENTS_CMD();

        static const size_t dwSize = 8;
        static const size_t byteSize = 32;
    };

    //!
    //! \brief SAMPLER_STATE_8x8_AVS
    //! \details
    //!     ExistsIf = AVS &amp;&amp; (Function_mode = 0)
    //!
    struct SAMPLER_STATE_8x8_AVS_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 GainFactor                                       : __CODEGEN_BITFIELD( 0,  5)    ; //!< GAIN_FACTOR
                uint32_t                 WeakEdgeThreshold                                : __CODEGEN_BITFIELD( 6, 11)    ; //!< WEAK_EDGE_THRESHOLD
                uint32_t                 StrongEdgeThreshold                              : __CODEGEN_BITFIELD(12, 17)    ; //!< STRONG_EDGE_THRESHOLD
                uint32_t                 R3XCoefficient                                   : __CODEGEN_BITFIELD(18, 22)    ; //!< R3X_COEFFICIENT
                uint32_t                 R3CCoefficient                                   : __CODEGEN_BITFIELD(23, 27)    ; //!< R3C_COEFFICIENT
                uint32_t                 Reserved28                                       : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
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
                uint32_t                 SinAlpha                                         : __CODEGEN_BITFIELD( 0,  7)    ; //!< Sin(alpha)
                uint32_t                 CosAlpha                                         : __CODEGEN_BITFIELD( 8, 15)    ; //!< Cos(alpha)
                uint32_t                 SatMax                                           : __CODEGEN_BITFIELD(16, 21)    ; //!< SAT_MAX
                uint32_t                 HueMax                                           : __CODEGEN_BITFIELD(22, 27)    ; //!< HUE_MAX
                uint32_t                 Enable8TapFilter                                 : __CODEGEN_BITFIELD(28, 29)    ; //!< ENABLE_8_TAP_FILTER
                uint32_t                 Ief4SmoothEnable                                 : __CODEGEN_BITFIELD(30, 30)    ; //!< IEF4SMOOTH_ENABLE_
                uint32_t                 SkinToneTunedIefEnable                           : __CODEGEN_BITFIELD(31, 31)    ; //!< SKIN_TONE_TUNED_IEF__ENABLE
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 S3U                                              : __CODEGEN_BITFIELD( 0, 10)    ; //!< S3U
                uint32_t                 ShuffleOutputwritebackForSample8X8               : __CODEGEN_BITFIELD(11, 11)    ; //!< SHUFFLE_OUTPUTWRITEBACK_FOR_SAMPLE_8X8
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
                uint32_t                 Skindetailfactor                                 : __CODEGEN_BITFIELD(31, 31)    ; //!< SKINDETAILFACTOR_
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

        mhw_state_heap_g9_X::SAMPLER_STATE_8x8_AVS_COEFFICIENTS_CMD FilterCoefficient016[17];                             //!< Filter Coefficient[0..16]

        union
        {
            //!< DWORD 152
            struct
            {
                uint32_t                 TransitionAreaWith8Pixels                        : __CODEGEN_BITFIELD( 0,  2)    ; //!< Transition Area with 8 Pixels
                uint32_t                 Reserved4867                                     : __CODEGEN_BITFIELD( 3,  3)    ; //!< Reserved
                uint32_t                 TransitionAreaWith4Pixels                        : __CODEGEN_BITFIELD( 4,  6)    ; //!< Transition Area with 4 Pixels
                uint32_t                 Reserved4871                                     : __CODEGEN_BITFIELD( 7,  7)    ; //!< Reserved
                uint32_t                 MaxDerivative8Pixels                             : __CODEGEN_BITFIELD( 8, 15)    ; //!< Max Derivative 8 Pixels
                uint32_t                 MaxDerivative4Pixels                             : __CODEGEN_BITFIELD(16, 23)    ; //!< Max Derivative 4 Pixels
                uint32_t                 DefaultSharpnessLevel                            : __CODEGEN_BITFIELD(24, 31)    ; //!< DEFAULT_SHARPNESS_LEVEL
            };
            uint32_t                     Value;
        } DW152;
        union
        {
            //!< DWORD 153
            struct
            {
                uint32_t                 RgbAdaptive                                      : __CODEGEN_BITFIELD( 0,  0)    ; //!< RGB_ADAPTIVE
                uint32_t                 AdaptiveFilterForAllChannels                     : __CODEGEN_BITFIELD( 1,  1)    ; //!< ADAPTIVE_FILTER_FOR_ALL_CHANNELS
                uint32_t                 Reserved4898                                     : __CODEGEN_BITFIELD( 2, 20)    ; //!< Reserved
                uint32_t                 BypassYAdaptiveFiltering                         : __CODEGEN_BITFIELD(21, 21)    ; //!< BYPASS_Y_ADAPTIVE_FILTERING
                uint32_t                 BypassXAdaptiveFiltering                         : __CODEGEN_BITFIELD(22, 22)    ; //!< BYPASS_X_ADAPTIVE_FILTERING
                uint32_t                 Reserved4919                                     : __CODEGEN_BITFIELD(23, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW153;

        uint32_t                         Reserved4928[6];                                                                 //!< Reserved

        mhw_state_heap_g9_X::SAMPLER_STATE_8x8_AVS_COEFFICIENTS_CMD FilterCoefficient1731[15];                            //!< Filter Coefficient[17..31]

        //! \name Local enumerations

        //! \brief GAIN_FACTOR
        //! \details
        //!     User control sharpening strength
        enum GAIN_FACTOR
        {
            GAIN_FACTOR_UNNAMED44                                            = 44, //!< No additional details
        };

        //! \brief WEAK_EDGE_THRESHOLD
        //! \details
        //!     If <b>Strong Edge Threshold</b> &gt; EM &gt; <b>Weak Edge Threshold</b>,
        //!     the basic VSA detects a weak edge.
        enum WEAK_EDGE_THRESHOLD
        {
            WEAK_EDGE_THRESHOLD_UNNAMED1                                     = 1, //!< No additional details
        };

        //! \brief STRONG_EDGE_THRESHOLD
        //! \details
        //!     If EM &gt; <b>Strong Edge Threshold</b>, the basic VSA detects a strong
        //!     edge.
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
        //!     Sharpening strength when no edge is found in basic VSA.
        enum NON_EDGE_WEIGHT
        {
            NON_EDGE_WEIGHT_UNNAMED1                                         = 1, //!< No additional details
        };

        //! \brief REGULAR_WEIGHT
        //! \details
        //!     Sharpening strength when a weak edge is found in basic VSA.
        enum REGULAR_WEIGHT
        {
            REGULAR_WEIGHT_UNNAMED2                                          = 2, //!< No additional details
        };

        //! \brief STRONG_EDGE_WEIGHT
        //! \details
        //!     Sharpening strength when a strong edge is found in basic VSA.
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
        //!     Rectangle half length
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

        //! \brief ENABLE_8_TAP_FILTER
        //! \details
        //!     <i><b>Adaptive Filtering (Mode = 11) ExistsIf:</b></i>
        //!     <p></p> R10G10B10A2_UNORM R8G8B8A8_UNORM (AYUV also) R8B8G8A8_UNORM
        //!     B8G8R8A8_UNORM R16G16B16A16
        enum ENABLE_8_TAP_FILTER
        {
            ENABLE_8_TAP_FILTER_UNNAMED0                                     = 0, //!< 4-tap filter is only done on all channels.
            ENABLE_8_TAP_FILTER_UNNAMED1                                     = 1, //!< Enable 8-tap Adaptive filter on G-channel. 4-tap filter on other channels.
            ENABLE_8_TAP_FILTER_UNNAMED2                                     = 2, //!< 8-tap filter is done on all channels (UV-ch uses the Y-coefficients)
            ENABLE_8_TAP_FILTER_UNNAMED3                                     = 3, //!< Enable 8-tap Adaptive filter all channels (UV-ch uses the Y-coefficients).
        };

        enum IEF4SMOOTH_ENABLE_
        {
            IEF4SMOOTH_ENABLE_UNNAMED0                                       = 0, //!< IEF is operating as a content adaptive detail filter based on 5x5 region
            IEF4SMOOTH_ENABLE_UNNAMED1                                       = 1, //!< IEF is operating as a content adaptive smooth filter based on 3x3 region
        };

        //! \brief SKIN_TONE_TUNED_IEF__ENABLE
        //! \details
        //!     Control bit to enable the skin tone tuned IEF.
        enum SKIN_TONE_TUNED_IEF__ENABLE
        {
            SKIN_TONE_TUNED_IEF_ENABLE_UNNAMED1                              = 1, //!< No additional details
        };

        enum SHUFFLE_OUTPUTWRITEBACK_FOR_SAMPLE_8X8
        {
            SHUFFLE_OUTPUTWRITEBACK_FOR_SAMPLE_8X8_UNNAMED0                  = 0, //!< Writeback same as Original Sample_8x8
            SHUFFLE_OUTPUTWRITEBACK_FOR_SAMPLE_8X8_UNNAMED1                  = 1, //!< Writeback  of Sample_8x8 Is Modified to Suite Sample_Unorm
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
        //!     Defines rectangle margin
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
            DIAMOND_DU_UNNAMED2                                              = 2, //!< No additional details
        };

        //! \brief SKINDETAILFACTOR_
        //! \details
        //!     This flag bit is in operation only when the control bit <b>Skin Tone
        //!     TunedIEF_Enable</b> is on.
        enum SKINDETAILFACTOR_
        {
            SKINDETAILFACTOR_UNNAMED0                                        = 0, //!< sign(SkinDetailFactor) is equal to -1, and the content of the detected skin tone area is detail revealed.
            SKINDETAILFACTOR_UNNAMED1                                        = 1, //!< sign(SkinDetailFactor) is equal to +1, and the content of the detected skin tone area is not detail revealed.
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

        //! \brief DEFAULT_SHARPNESS_LEVEL
        //! \details
        //!     When adaptive scaling is off, determines the balance between sharp and
        //!     smooth scalers.
        enum DEFAULT_SHARPNESS_LEVEL
        {
            DEFAULT_SHARPNESS_LEVEL_UNNAMED0                                 = 0, //!< Contribute 1 from the smooth scalar
            DEFAULT_SHARPNESS_LEVEL_UNNAMED255                               = 255, //!< Contribute 1 from the sharp scalar
        };

        //! \brief RGB_ADAPTIVE
        //! \details
        //!     This should be always set to 0 for YUV input and can be enabled/disabled
        //!     for RGB input.
        //!                         This should be enabled only if we enable 8-tap adaptive filter for
        //!     RGB input.
        enum RGB_ADAPTIVE
        {
            RGB_ADAPTIVE_DISBLE                                              = 0, //!< Disable the RGB Adaptive equation and use G-Ch directly for adaptive filter
            RGB_ADAPTIVE_ENABLE                                              = 1, //!< Enable the RGB Adaptive filter using the equation (Y=(R+2G+B)>>2)
        };

        //! \brief ADAPTIVE_FILTER_FOR_ALL_CHANNELS
        //! \details
        //!     Only to be enabled if 8-tap Adaptive filter mode is on, eElse it should
        //!     be disabled.
        enum ADAPTIVE_FILTER_FOR_ALL_CHANNELS
        {
            ADAPTIVE_FILTER_FOR_ALL_CHANNELS_DISBLE                          = 0, //!< Disable Adaptive Filter on UV/RB Channels
            ADAPTIVE_FILTER_FOR_ALL_CHANNELS_ENABLE                          = 1, //!< Enable Adaptive Filter on UV/RB Channels
        };

        //! \brief BYPASS_Y_ADAPTIVE_FILTERING
        //! \details
        //!     When disabled, the Y direction will use <b>Default Sharpness Level</b>
        //!     to blend between the smooth and sharp filters rather than the calculated
        //!     value.
        enum BYPASS_Y_ADAPTIVE_FILTERING
        {
            BYPASS_Y_ADAPTIVE_FILTERING_ENABLE                               = 0, //!< Enable Y Adaptive Filtering
            BYPASS_Y_ADAPTIVE_FILTERING_DISBLE                               = 1, //!< Disable Y Adaptive Filtering
        };

        //! \brief BYPASS_X_ADAPTIVE_FILTERING
        //! \details
        //!     When disabled, the X direction will use <b>Default Sharpness Level</b>
        //!     to blend between the smooth and sharp filters rather than the calculated
        //!     value.
        enum BYPASS_X_ADAPTIVE_FILTERING
        {
            BYPASS_X_ADAPTIVE_FILTERING_ENABLE                               = 0, //!< Enable X Adaptive Filtering
            BYPASS_X_ADAPTIVE_FILTERING_DISBLE                               = 1, //!< Disable X Adaptive Filtering
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        SAMPLER_STATE_8x8_AVS_CMD();

        static const size_t dwSize = 280;
        static const size_t byteSize = 1120;
    };

    //!
    //! \brief SAMPLER_STATE_8x8_CONVOLVE_COEFFICIENTS
    //! \details
    //!     Function: 0001b ExistsIf: [Convolve] &amp;&amp; [SKL_mode==0] &amp;&amp;
    //!     [(Kernel Size) =< (15x15)]
    //! 
    //!     Function: 1010b ExistsIf: "[1Pixel Convolution ] &amp;&amp; [(Kernel
    //!     Size) =< (15x15)]
    //! 
    //!     Function: 0001b ExistsIf: [Convolve] &amp;&amp; [SKL_mode==1] &amp;&amp;
    //!     [(Kernel Size) > (15x15)]
    //!
    struct SAMPLER_STATE_8x8_CONVOLVE_COEFFICIENTS_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 FilterCoefficient00                              : __CODEGEN_BITFIELD( 0, 15)    ; //!< Filter Coefficient[0,0]
                uint32_t                 FilterCoefficient01                              : __CODEGEN_BITFIELD(16, 31)    ; //!< Filter Coefficient[0,1]
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 FilterCoefficient02                              : __CODEGEN_BITFIELD( 0, 15)    ; //!< Filter Coefficient[0,2]
                uint32_t                 FilterCoefficient03                              : __CODEGEN_BITFIELD(16, 31)    ; //!< Filter Coefficient[0,3]
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 FilterCoefficient04                              : __CODEGEN_BITFIELD( 0, 15)    ; //!< Filter Coefficient[0,4]
                uint32_t                 FilterCoefficient05                              : __CODEGEN_BITFIELD(16, 31)    ; //!< Filter Coefficient[0,5]
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 FilterCoefficient06                              : __CODEGEN_BITFIELD( 0, 15)    ; //!< Filter Coefficient[0,6]
                uint32_t                 FilterCoefficient07                              : __CODEGEN_BITFIELD(16, 31)    ; //!< Filter Coefficient[0,7]
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 FilterCoefficient08                              : __CODEGEN_BITFIELD( 0, 15)    ; //!< Filter Coefficient[0,8]
                uint32_t                 FilterCoefficient09                              : __CODEGEN_BITFIELD(16, 31)    ; //!< Filter Coefficient[0,9]
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 FilterCoefficient010                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< Filter Coefficient[0,10]
                uint32_t                 FilterCoefficient011                             : __CODEGEN_BITFIELD(16, 31)    ; //!< Filter Coefficient[0,11]
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 FilterCoefficient012                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< Filter Coefficient[0,12]
                uint32_t                 FilterCoefficient013                             : __CODEGEN_BITFIELD(16, 31)    ; //!< Filter Coefficient[0,13]
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 FilterCoefficient014                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< Filter Coefficient[0,14]
                uint32_t                 FilterCoefficient015                             : __CODEGEN_BITFIELD(16, 31)    ; //!< Filter Coefficient[0,15]
            };
            uint32_t                     Value;
        } DW7;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        SAMPLER_STATE_8x8_CONVOLVE_COEFFICIENTS_CMD();

        static const size_t dwSize = 8;
        static const size_t byteSize = 32;
    };

    //!
    //! \brief SAMPLER_STATE_8x8_CONVOLVE
    //! \details
    //!     Function: 0001b ExistsIf: [Convolve] &amp;&amp; [SKL_mode==0] &amp;&amp;
    //!     [(Kernel Size) =< (15x15)]
    //! 
    //!     Function: 1010b ExistsIf: "[1Pixel Convolution ] &amp;&amp; [(Kernel
    //!     Size) =< (15x15)]
    //! 
    //!     Function: 0001b ExistsIf: [Convolve] &amp;&amp; [SKL_mode==1] &amp;&amp;
    //!     [(Kernel Size) > (15x15)]
    //!
    struct SAMPLER_STATE_8x8_CONVOLVE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 Height                                           : __CODEGEN_BITFIELD( 0,  3)    ; //!< HEIGHT
                uint32_t                 Width                                            : __CODEGEN_BITFIELD( 4,  7)    ; //!< WIDTH
                uint32_t                 ScaleDownValue                                   : __CODEGEN_BITFIELD( 8, 11)    ; //!< Scale down value
                uint32_t                 SizeOfTheCoefficient                             : __CODEGEN_BITFIELD(12, 12)    ; //!< SIZE_OF_THE_COEFFICIENT
                uint32_t                 Reserved13                                       : __CODEGEN_BITFIELD(13, 15)    ; //!< Reserved
                uint32_t                 MsbHeight                                        : __CODEGEN_BITFIELD(16, 16)    ; //!< MSB_HEIGHT
                uint32_t                 Reserved17                                       : __CODEGEN_BITFIELD(17, 19)    ; //!< Reserved
                uint32_t                 MsbWidth                                         : __CODEGEN_BITFIELD(20, 20)    ; //!< MSB_WIDTH
                uint32_t                 Reserved21                                       : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;

        uint32_t                         Reserved32[15];                                                                  //!< Reserved

        mhw_state_heap_g9_X::SAMPLER_STATE_8x8_CONVOLVE_COEFFICIENTS_CMD FilterCoefficient300310[62];                     //!< Filter Coefficient[30:0,31:0]

        //! \name Local enumerations

        enum SIZE_OF_THE_COEFFICIENT
        {
            SIZE_OF_THE_COEFFICIENT_8BIT                                     = 0, //!< The lower 8 bits of the accumulator is forced to zero or ignored during the accumulation operation.
            SIZE_OF_THE_COEFFICIENT_16BIT                                    = 1, //!< The lower 8 bits are also included for the operation. The final result of the accumulator is shifted before clamping the result as specified by the Scale down value.:Result[15:0] = Clamp(Accum[40:12] >> scale_down)
        };

        //! \brief MSB_HEIGHT
        //! \details
        //!     It contains the MSB HEIGHT of the kernel and is used to extend the
        //!     kernel width range to 31.
        //!                         Used along with bits[3:0] which represents the LSB for the kernel
        //!     Height.
        enum MSB_HEIGHT
        {
            MSB_HEIGHT_NOCHANGE                                              = 0, //!< No Change to the Filter Size
            MSB_HEIGHT_EXTENDED                                              = 1, //!< Extends the filter size height upto 31.
        };

        //! \brief MSB_WIDTH
        //! \details
        //!     It contains the MSB Width of the kernel and is used to extend the kernel
        //!     width range to 31.
        //!                         Used along with bits[3:0] which represents the LSB for the kernel
        //!     Height.
        enum MSB_WIDTH
        {
            MSB_WIDTH_NOCHANGE                                               = 0, //!< No Change to the Filter Size
            MSB_WIDTH_EXTENDED                                               = 1, //!< Extends the Filter Size Width upto 31.
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        SAMPLER_STATE_8x8_CONVOLVE_CMD();

        static const size_t dwSize = 512;
        static const size_t byteSize = 2048;
    };

    //!
    //! \brief SAMPLER_STATE_8x8_ERODE_DILATE_MINMAXFILTER
    //! \details
    //!     The table is valid for the following funstions: 0100 - Erode &amp;&amp;
    //!     (Function_mode==0) 0101 - Dilate &amp;&amp; (Function_mode==0) 0011 -
    //!     MinMaxFilter &amp;&amp; (Function_mode==0)
    //! 
    //!     Max kernel size is 15x15. For sizes less than 15x15 the coefficients not
    //!     used should be zeroed out.
    //!
    struct SAMPLER_STATE_8x8_ERODE_DILATE_MINMAXFILTER_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 HeightOfTheKernel                                : __CODEGEN_BITFIELD( 0,  3)    ; //!< Height Of The Kernel
                uint32_t                 WidthOfTheKernel                                 : __CODEGEN_BITFIELD( 4,  7)    ; //!< Width Of The Kernel
                uint32_t                 Reserved8                                        : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 BitMask16ForRow0150                              : __CODEGEN_BITFIELD(16, 31)    ; //!< 16bit Mask for Row0 [15:0]
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 BitMask16ForRow1150                              : __CODEGEN_BITFIELD( 0, 15)    ; //!< 16bit Mask for Row1 [15:0]
                uint32_t                 BitMask16ForRow2150                              : __CODEGEN_BITFIELD(16, 31)    ; //!< 16bit Mask for Row2 [15:0]
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 BitMask16ForRow3150                              : __CODEGEN_BITFIELD( 0, 15)    ; //!< 16bit Mask for Row3 [15:0]
                uint32_t                 BitMask16ForRow4150                              : __CODEGEN_BITFIELD(16, 31)    ; //!< 16bit Mask for Row4 [15:0]
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 BitMask16ForRow5150                              : __CODEGEN_BITFIELD( 0, 15)    ; //!< 16bit Mask for Row5 [15:0]
                uint32_t                 BitMask16ForRow6150                              : __CODEGEN_BITFIELD(16, 31)    ; //!< 16bit Mask for Row6 [15:0]
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 BitMask16ForRow7150                              : __CODEGEN_BITFIELD( 0, 15)    ; //!< 16bit Mask for Row7 [15:0]
                uint32_t                 BitMask16ForRow8150                              : __CODEGEN_BITFIELD(16, 31)    ; //!< 16bit Mask for Row8 [15:0]
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 BitMask16ForRow9150                              : __CODEGEN_BITFIELD( 0, 15)    ; //!< 16bit Mask for Row9 [15:0]
                uint32_t                 BitMask16ForRow10150                             : __CODEGEN_BITFIELD(16, 31)    ; //!< 16bit Mask for Row10 [15:0]
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 BitMask16ForRow11150                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< 16bit Mask for Row11 [15:0]
                uint32_t                 BitMask16ForRow12150                             : __CODEGEN_BITFIELD(16, 31)    ; //!< 16bit Mask for Row12 [15:0]
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 BitMask16ForRow13150                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< 16bit Mask for Row13 [15:0]
                uint32_t                 BitMask16ForRow14150                             : __CODEGEN_BITFIELD(16, 31)    ; //!< 16bit Mask for Row14 [15:0]
            };
            uint32_t                     Value;
        } DW7;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        SAMPLER_STATE_8x8_ERODE_DILATE_MINMAXFILTER_CMD();

        static const size_t dwSize = 8;
        static const size_t byteSize = 32;
    };

    //!
    //! \brief SAMPLER_INDIRECT_STATE
    //! \details
    //!     Note: There are three variations of this structure, defined separately
    //!     because their payloads have different lengths. Currently only
    //!     SAMPLER_INDIRECT_STATE_BORDER_COLOR is fully defined. 
    //!     This structure is pointed to by Indirect State Pointer (SAMPLER_STATE).
    //!     The interpretation of the border color depends on the Texture Border
    //!     Color Mode field in SAMPLER_STATE as follows:
    //! 
    //!     In 8BIT mode, the border color is 8-bit UNORM format, regardless of the
    //!     surface format chosen. For surface formats with one or more channels
    //!     missing (i.e. R5G6R5_UNORM is missing the alpha channel), the value from
    //!     the border color, if selected, will be used even for the missing
    //!     channels.
    //! 
    //!     In OGL mode, the format of the border color is R32G32B32A32_FLOAT,
    //!     R32G32B32A32_SINT, or R32G32B32A32_UINT, depending on the surface format
    //!     chosen. For surface formats with one or more channels missing, the value
    //!     from the border color is not used for the missing channels, resulting in
    //!     these channels resulting in the overall default value (0 for colors and
    //!     1 for alpha) regardless of whether border color is chosen. The surface
    //!     formats with "L" and "I" have special behavior with respect to the
    //!     border color. The border color value used for the replicated channels
    //!     (RGB for "L" formats and RGBA for "I" formats) comes from the red
    //!     channel of border color. In these cases, the green and blue channels,
    //!     and also alpha for "I", of the border color are ignored.
    //! 
    //! 
    //! 
    //! 
    //! 
    //!     The format of this state depends on the Texture Border Color Mode field.
    //! 
    //!      8BIT mode is not supported for surfaces with more than 16 bits in any
    //!     channel, other than 32-bit float formats which are supported.
    //!      The conditions under which this color is used depend on the Surface
    //!     Type - 1D/2D/3D surfaces use the border color when the coordinates
    //!     extend beyond the surface extent; cube surfaces use the border color for
    //!     "empty" (disabled) faces.
    //!      The border color itself is accessed through the texture cache hierarchy
    //!     rather than the state cache hierarchy.  Thus, if the border color is
    //!     changed in memory, the texture cache must be invalidated and the state
    //!     cache does not need to be invalidated.
    //!      MAPFILTER_MONO:  The border color is ignored.  Border color is fixed at
    //!     a value of 0 by hardware.
    //! 
    //!
    struct SAMPLER_INDIRECT_STATE_CMD
    {
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 BorderColorRed                                                                   ; //!< Border Color Red, Structure[SAMPLER_STATE][Texture Border Color Mode] == 'OGL'
            } Obj0;
            struct
            {
                uint32_t                 BorderColorRed                                   : __CODEGEN_BITFIELD( 0,  7)    ; //!< Border Color Red, Structure[SAMPLER_STATE][Texture Border Color Mode] == '8BIT'
                uint32_t                 BorderColorGreen                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< Border Color Green, Structure[SAMPLER_STATE][Texture Border Color Mode] == '8BIT'
                uint32_t                 BorderColorBlue                                  : __CODEGEN_BITFIELD(16, 23)    ; //!< Border Color Blue, Structure[SAMPLER_STATE][Texture Border Color Mode] == '8BIT'
                uint32_t                 BorderColorAlpha                                 : __CODEGEN_BITFIELD(24, 31)    ; //!< Border Color Alpha, Structure[SAMPLER_STATE][Texture Border Color Mode] == '8BIT'
            } Obj1;
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 BorderColorGreen                                                                 ; //!< Border Color Green, Structure[SAMPLER_STATE][Texture Border Color Mode] == 'OGL'
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 BorderColorBlue                                                                  ; //!< Border Color Blue, Structure[SAMPLER_STATE][Texture Border Color Mode] == 'OGL'
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 BorderColorAlpha                                                                 ; //!< Border Color Alpha, Structure[SAMPLER_STATE][Texture Border Color Mode] == 'OGL'
            };
            uint32_t                     Value;
        } DW3;

        uint32_t                         Reserved128[12];                                                                 //!< Reserved

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        SAMPLER_INDIRECT_STATE_CMD();

        static const size_t dwSize = 16;
        static const size_t byteSize = 64;
    };

};

#pragma pack()

#endif  // __MHW_STATE_HEAP_HWCMD_G9_X_H__
