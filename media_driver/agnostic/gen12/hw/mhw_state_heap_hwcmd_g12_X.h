/*
* Copyright (c) 2015-2019, Intel Corporation
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
//! \file     mhw_state_heap_hwcmd_g12_X.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of g12_X as other components
//!           should use MHW interface to interact with MHW commands and states.
//!

// DO NOT EDIT

#ifndef __MHW_STATE_HEAP_HWCMD_G12_X_H__
#define __MHW_STATE_HEAP_HWCMD_G12_X_H__

#pragma once
#pragma pack(1)

#include <cstdint>
#include <cstddef>

class mhw_state_heap_g12_X
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
            struct
            {
                uint32_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 KernelStartPointer                               : __CODEGEN_BITFIELD( 6, 31)    ; //!< Kernel Start Pointer
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 KernelStartPointerHigh                           : __CODEGEN_BITFIELD( 0, 15)    ; //!< Kernel Start Pointer High
                uint32_t                 Reserved48                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
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
                uint32_t                 ThreadPreemptionDisable                          : __CODEGEN_BITFIELD(20, 20)    ; //!< THREAD_PREEMPTION_DISABLE
                uint32_t                 Reserved85                                       : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
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
            struct
            {
                uint32_t                 BindingTableEntryCount                           : __CODEGEN_BITFIELD( 0,  4)    ; //!< Binding Table Entry Count
                uint32_t                 BindingTablePointer                              : __CODEGEN_BITFIELD( 5, 20)    ; //!< Binding Table Pointer
                uint32_t                 Reserved149                                      : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 ConstantUrbEntryReadOffset                       : __CODEGEN_BITFIELD( 0, 15)    ; //!< Constant URB Entry Read Offset
                uint32_t                 ConstantIndirectUrbEntryReadLength               : __CODEGEN_BITFIELD(16, 31)    ; //!< Constant/Indirect URB Entry Read Length
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 NumberOfThreadsInGpgpuThreadGroup                : __CODEGEN_BITFIELD( 0,  9)    ; //!< Number of Threads in GPGPU Thread Group
                uint32_t                 Reserved202                                      : __CODEGEN_BITFIELD(10, 12)    ; //!< Reserved
                uint32_t                 OverDispatchControl                              : __CODEGEN_BITFIELD(13, 14)    ; //!< OVER_DISPATCH_CONTROL
                uint32_t                 Reserved207                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 SharedLocalMemorySize                            : __CODEGEN_BITFIELD(16, 20)    ; //!< SHARED_LOCAL_MEMORY_SIZE
                uint32_t                 BarrierEnable                                    : __CODEGEN_BITFIELD(21, 21)    ; //!< Barrier Enable
                uint32_t                 RoundingMode                                     : __CODEGEN_BITFIELD(22, 23)    ; //!< ROUNDING_MODE
                uint32_t                 Reserved216                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
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
        //!     with m = 1) or multiple program flows (SIMDnxm with m > 1).
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

        //! \brief THREAD_PREEMPTION_DISABLE
        //! \details
        //!     This field specifies whether, when dispatched, the thread is allowed to
        //!     stop in middle on receiving mid-thread pre-emption request.
        enum THREAD_PREEMPTION_DISABLE
        {
            THREAD_PREEMPTION_DISABLE_DISABLE                                = 0, //!< Thread is pre-empted on receiving pre-emption indication.
            THREAD_PREEMPTION_DISABLE_ENABLE                                 = 1, //!< Thread is preempted only in case of page-fault.
        };

        //! \brief SAMPLER_COUNT
        //! \details
        //!     Specifies how many samplers (in multiples of 4) the kernel uses.  Used
        //!     only for prefetching the associated sampler state entries. 
        //!     This field is ignored for child threads.
        //!     If this field is not zero, sampler state is prefetched for the
        //!     first instance of a root thread upon the startup of the media
        //!     pipeline.
        enum SAMPLER_COUNT
        {
            SAMPLER_COUNT_NOSAMPLERSUSED                                     = 0, //!< No additional details
            SAMPLER_COUNT_BETWEEN1AND4SAMPLERSUSED                           = 1, //!< No additional details
            SAMPLER_COUNT_BETWEEN5AND8SAMPLERSUSED                           = 2, //!< No additional details
            SAMPLER_COUNT_BETWEEN9AND12SAMPLERSUSED                          = 3, //!< No additional details
            SAMPLER_COUNT_BETWEEN13AND16SAMPLERSUSED                         = 4, //!< No additional details
        };

        //! \brief OVER_DISPATCH_CONTROL
        //! \details
        //!     Over Dispatch Control that enables and controls the amount of GPGPU
        //!     thread over dispatch.
        enum OVER_DISPATCH_CONTROL
        {
            OVER_DISPATCH_CONTROL_NONE                                       = 0, //!< No additional details
            OVER_DISPATCH_CONTROL_LOW                                        = 1, //!< No additional details
            OVER_DISPATCH_CONTROL_HIGH                                       = 2, //!< No additional details
            OVER_DISPATCH_CONTROL_NORMAL                                     = 3, //!< No additional details
        };

        //! \brief SHARED_LOCAL_MEMORY_SIZE
        //! \details
        //!     This field indicates how much Shared Local Memory the thread group
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
            struct
            {
                uint32_t                 CubeFaceEnablePositiveZ                          : __CODEGEN_BITFIELD( 0,  0)    ; //!< Cube Face Enable - Positive Z
                uint32_t                 CubeFaceEnableNegativeZ                          : __CODEGEN_BITFIELD( 1,  1)    ; //!< Cube Face Enable - Negative Z
                uint32_t                 CubeFaceEnablePositiveY                          : __CODEGEN_BITFIELD( 2,  2)    ; //!< Cube Face Enable - Positive Y
                uint32_t                 CubeFaceEnableNegativeY                          : __CODEGEN_BITFIELD( 3,  3)    ; //!< Cube Face Enable - Negative Y
                uint32_t                 CubeFaceEnablePositiveX                          : __CODEGEN_BITFIELD( 4,  4)    ; //!< Cube Face Enable - Positive X
                uint32_t                 CubeFaceEnableNegativeX                          : __CODEGEN_BITFIELD( 5,  5)    ; //!< Cube Face Enable - Negative X
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
            struct
            {
                uint32_t                 SurfaceQpitch                                    : __CODEGEN_BITFIELD( 0, 14)    ; //!< Surface QPitch
                uint32_t                 SampleTapDiscardDisable                          : __CODEGEN_BITFIELD(15, 15)    ; //!< SAMPLE_TAP_DISCARD_DISABLE
                uint32_t                 Reserved48                                       : __CODEGEN_BITFIELD(16, 16)    ; //!< Reserved
                uint32_t                 DoubleFetchDisable                               : __CODEGEN_BITFIELD(17, 17)    ; //!< DOUBLE_FETCH_DISABLE
                uint32_t                 CornerTexelMode                                  : __CODEGEN_BITFIELD(18, 18)    ; //!< CORNER_TEXEL_MODE
                uint32_t                 BaseMipLevel                                     : __CODEGEN_BITFIELD(19, 23)    ; //!< Base Mip Level
                uint32_t                 MemoryObjectControlState                         : __CODEGEN_BITFIELD(24, 30)    ; //!< Memory Object Control State
                uint32_t                 EnableUnormPathInColorPipe                       : __CODEGEN_BITFIELD(31, 31)    ; //!< ENABLE_UNORM_PATH_IN_COLOR_PIPE
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 Width                                            : __CODEGEN_BITFIELD( 0, 13)    ; //!< Width
                uint32_t                 Reserved78                                       : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 Height                                           : __CODEGEN_BITFIELD(16, 29)    ; //!< Height
                uint32_t                 Reserved94                                       : __CODEGEN_BITFIELD(30, 30)    ; //!< Reserved
                uint32_t                 DepthStencilResouce                              : __CODEGEN_BITFIELD(31, 31)    ; //!< Depth/Stencil Resouce
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 SurfacePitch                                     : __CODEGEN_BITFIELD( 0, 17)    ; //!< Surface Pitch
                uint32_t                 NullProbingEnable                                : __CODEGEN_BITFIELD(18, 18)    ; //!< NULL_PROBING_ENABLE
                uint32_t                 StandardTilingModeExtensions                     : __CODEGEN_BITFIELD(19, 19)    ; //!< STANDARD_TILING_MODE_EXTENSIONS
                uint32_t                 TileAddressMappingMode                           : __CODEGEN_BITFIELD(20, 20)    ; //!< TILE_ADDRESS_MAPPING_MODE
                uint32_t                 Depth                                            : __CODEGEN_BITFIELD(21, 31)    ; //!< Depth
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 MultisamplePositionPaletteIndex                  : __CODEGEN_BITFIELD( 0,  2)    ; //!< Multisample Position Palette Index
                uint32_t                 NumberOfMultisamples                             : __CODEGEN_BITFIELD( 3,  5)    ; //!< NUMBER_OF_MULTISAMPLES
                uint32_t                 MultisampledSurfaceStorageFormat                 : __CODEGEN_BITFIELD( 6,  6)    ; //!< MULTISAMPLED_SURFACE_STORAGE_FORMAT
                uint32_t                 RenderTargetViewExtent                           : __CODEGEN_BITFIELD( 7, 17)    ; //!< Render Target View Extent
                uint32_t                 MinimumArrayElement                              : __CODEGEN_BITFIELD(18, 28)    ; //!< Minimum Array Element
                uint32_t                 RenderTargetAndSampleUnormRotation               : __CODEGEN_BITFIELD(29, 30)    ; //!< RENDER_TARGET_AND_SAMPLE_UNORM_ROTATION
                uint32_t                 DecompressInL3                                   : __CODEGEN_BITFIELD(31, 31)    ; //!< DECOMPRESS_IN_L3
            };
            uint32_t                     Value;
        } DW4;
        union
        {
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
            struct
            {
                uint32_t                 AuxiliarySurfaceMode                             : __CODEGEN_BITFIELD( 0,  2)    ; //!< AUXILIARY_SURFACE_MODE, ([Surface Format] != 'PLANAR')
                uint32_t                 Reserved195                                      : __CODEGEN_BITFIELD( 3, 31)    ; //!< Reserved, ([Surface Format] != 'PLANAR')
            } Obj0;
            struct
            {
                uint32_t                 Reserved192                                      : __CODEGEN_BITFIELD( 0,  2)    ; //!< Reserved, ([Surface Format] != 'PLANAR')
                uint32_t                 AuxiliarySurfacePitch                            : __CODEGEN_BITFIELD( 3, 12)    ; //!< Auxiliary Surface Pitch, ([Surface Format] != 'PLANAR')
                uint32_t                 Reserved205                                      : __CODEGEN_BITFIELD(13, 15)    ; //!< Reserved, ([Surface Format] != 'PLANAR')
                uint32_t                 AuxiliarySurfaceQpitch                           : __CODEGEN_BITFIELD(16, 30)    ; //!< Auxiliary Surface QPitch, ([Surface Format] != 'PLANAR')
                uint32_t                 Reserved223                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved, ([Surface Format] != 'PLANAR')
            } Obj1;
            struct
            {
                uint32_t                 YOffsetForUOrUvPlane                             : __CODEGEN_BITFIELD( 0, 13)    ; //!< Y Offset for U or UV Plane, ([Surface Format] == 'PLANAR')
                uint32_t                 Reserved206                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved, ([Surface Format] == 'PLANAR')
                uint32_t                 XOffsetForUOrUvPlane                             : __CODEGEN_BITFIELD(16, 29)    ; //!< X Offset for U or UV Plane, ([Surface Format] == 'PLANAR')
                uint32_t                 HalfPitchForChroma                               : __CODEGEN_BITFIELD(30, 30)    ; //!< HALF_PITCH_FOR_CHROMA, ([Surface Format] == 'PLANAR')
                uint32_t                 SeparateUvPlaneEnable                            : __CODEGEN_BITFIELD(31, 31)    ; //!< Separate UV Plane Enable, ([Surface Format] == 'PLANAR')
            } Obj2;
            struct
            {
                uint32_t                 Reserved192                                      : __CODEGEN_BITFIELD( 0, 14)    ; //!< Reserved, ([Surface Format] == 'PLANAR')
                uint32_t                 YuvInterpolationEnable                           : __CODEGEN_BITFIELD(15, 15)    ; //!< YUV_INTERPOLATION_ENABLE, 
                uint32_t                 Reserved208                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved, 
            } Obj3;
            uint32_t                     Value;
        } DW6;
        union
        {
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
            struct
            {
                uint64_t                 SurfaceBaseAddress                                                               ; //!< Surface Base Address
            };
            uint32_t                     Value[2];
        } DW8_9;
        union
        {
            struct
            {
                uint64_t                 QuiltWidth                                       : __CODEGEN_BITFIELD( 0,  4)    ; //!< Quilt Width, 
                uint64_t                 QuiltHeight                                      : __CODEGEN_BITFIELD( 5,  9)    ; //!< Quilt Height, 
                uint64_t                 ClearValueAddressEnable                          : __CODEGEN_BITFIELD(10, 10)    ; //!< CLEAR_VALUE_ADDRESS_ENABLE, 
                uint64_t                 ProceduralTexture                                : __CODEGEN_BITFIELD(11, 11)    ; //!< Procedural Texture, 
                uint64_t                 Reserved332                                      : __CODEGEN_BITFIELD(12, 31)    ; //!< Reserved, 
                uint64_t                 YOffsetForVPlane                                 : __CODEGEN_BITFIELD(32, 45)    ; //!< Y Offset for V Plane, ([Surface Format] == 'PLANAR')
                uint64_t                 Reserved366                                      : __CODEGEN_BITFIELD(46, 63)    ; //!< Reserved, ([Surface Format] == 'PLANAR')
            } Obj0;
            struct
            {
                uint64_t                 Reserved320                                      : __CODEGEN_BITFIELD( 0, 47)    ; //!< Reserved, ([Surface Format] == 'PLANAR')
                uint64_t                 XOffsetForVPlane                                 : __CODEGEN_BITFIELD(48, 61)    ; //!< X Offset for V Plane, ([Surface Format] == 'PLANAR')
                uint64_t                 Reserved382                                      : __CODEGEN_BITFIELD(62, 63)    ; //!< Reserved, ([Surface Format] == 'PLANAR')
            } Obj1;
            struct
            {
                uint64_t                 Reserved320                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved, ([Surface Format] == 'PLANAR')
                uint64_t                 AuxiliarySurfaceBaseAddress                      : __CODEGEN_BITFIELD(12, 63)    ; //!< Auxiliary Surface Base Address, ([Surface Format] != 'PLANAR') AND [Memory Compression Enable] == 0
            } Obj2;
            uint32_t                     Value[2];
        } DW10_11;
        union
        {
            struct
            {
                uint32_t                 Reserved384                                      : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 ClearAddressLow                                  : __CODEGEN_BITFIELD( 6, 31)    ; //!< Clear Address Low
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            struct
            {
                uint32_t                 ClearAddressHigh                                 : __CODEGEN_BITFIELD( 0, 15)    ; //!< Clear Address High
                uint32_t                 Reserved432                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            struct
            {
                uint32_t                 Reserved448                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            struct
            {
                uint32_t                 Reserved480                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;

        //! \name Local enumerations

        //! \brief MEDIA_BOUNDARY_PIXEL_MODE
        //! \details
        //!     
        //!     For 2D Non-Array Surfaces accessed via the Data Port Media Block
        //!     Read Message or Data Port Transpose Read message:
        //!     This field enables control of which rows are returned on vertical
        //!     out-of-bounds reads using the Data Port Media Block Read Message or Data
        //!     Port Transpose Read message. In the description below, frame mode refers
        //!     to Vertical Line Stride = 0, field mode is Vertical Line
        //!     Stride = 1 in which only the even or odd rows are addressable. The
        //!     frame refers to the entire surface, while the field refers only to the
        //!     even or odd rows within the surface.
        //!     For Other Surfaces:
        //!     Reserved : MBZ
        enum MEDIA_BOUNDARY_PIXEL_MODE
        {
            MEDIA_BOUNDARY_PIXEL_MODE_NORMALMODE                             = 0, //!< The row returned on an out-of-bound access is the closest row in the frame or field.  Rows from the opposite field are never returned.
            MEDIA_BOUNDARY_PIXEL_MODE_PROGRESSIVEFRAME                       = 2, //!< The row returned on an out-of-bound access is the closest row in the frame, even if in field mode.
            MEDIA_BOUNDARY_PIXEL_MODE_INTERLACEDFRAME                        = 3, //!< In field mode, the row returned on an out-of-bound access is the closest row in the field.  In frame mode, even out-of-bound rows return the nearest even row while odd out-of-bound rows return the nearest odd row.
        };

        //! \brief RENDER_CACHE_READ_WRITE_MODE
        //! \details
        //!     For Surfaces accessed via the Data Port to Render Cache:
        //!     This field specifies the way Render Cache treats a write request. If
        //!     unset, Render Cache allocates a write-only cache line for a write miss.
        //!     If set, Render Cache allocates a read-write cache line for a write
        //!     miss.
        //!     For Surfaces accessed via the Sampling Engine or Data Port to
        //!     Texture Cache or Data Cache:
        //!     This field is reserved : MBZ
        enum RENDER_CACHE_READ_WRITE_MODE
        {
            RENDER_CACHE_READ_WRITE_MODE_WRITE_ONLYCACHE                     = 0, //!< Allocating write-only cache for a write miss
            RENDER_CACHE_READ_WRITE_MODE_READ_WRITECACHE                     = 1, //!< Allocating read-write cache for a write miss
        };

        //! \brief TILE_MODE
        //! \details
        //!     This field specifies the type of memory tiling (Linear, XMajor, or
        //!     YMajor) employed to tile this surface. See Memory Interface
        //!     Functions for details on memory tiling and restrictions.
        enum TILE_MODE
        {
            TILE_MODE_LINEAR                                                 = 0, //!< Linear mode (no tiling)
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
        //!     For Sampling Engine and Render Target Surfaces: This field
        //!     specifies the vertical alignment requirement in elements for the
        //!     surface. Refer to the "Memory Data Formats" chapter for details on how
        //!     this field changes the layout of the surface in memory. An
        //!     element is defined as a pixel in uncompresed surface formats, and
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
            SURFACE_FORMAT_R10G10B10FLOATA2UNORM                             = 213, //!< No additional details
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
            SURFACE_FORMAT_R1UNORM                                           = 385, //!< SET0_LEGACY: Undefined behavior if used in any feature added for GEN11+. See Legacy sampler feature page for detalls
            SURFACE_FORMAT_YCRCBNORMAL                                       = 386, //!< No additional details
            SURFACE_FORMAT_YCRCBSWAPUVY                                      = 387, //!< No additional details
            SURFACE_FORMAT_P2UNORMPALETTE0                                   = 388, //!< No additional details
            SURFACE_FORMAT_P2UNORMPALETTE1                                   = 389, //!< No additional details
            SURFACE_FORMAT_BC1UNORM                                          = 390, //!< (DXT1)
            SURFACE_FORMAT_BC2UNORM                                          = 391, //!< (DXT2/3)
            SURFACE_FORMAT_BC3UNORM                                          = 392, //!< (DXT4/5)
            SURFACE_FORMAT_BC4UNORM                                          = 393, //!< No additional details
            SURFACE_FORMAT_BC5UNORM                                          = 394, //!< No additional details
            SURFACE_FORMAT_BC1UNORMSRGB                                      = 395, //!< (DXT1_SRGB)
            SURFACE_FORMAT_BC2UNORMSRGB                                      = 396, //!< (DXT2/3_SRGB)
            SURFACE_FORMAT_BC3UNORMSRGB                                      = 397, //!< (DXT4/5_SRGB)
            SURFACE_FORMAT_MONO8                                             = 398, //!< SET0_LEGACY: Undefined behavior if used in any feature added for GEN11+. See Legacy sampler feature page for detalls
            SURFACE_FORMAT_YCRCBSWAPUV                                       = 399, //!< No additional details
            SURFACE_FORMAT_YCRCBSWAPY                                        = 400, //!< No additional details
            SURFACE_FORMAT_DXT1RGB                                           = 401, //!< No additional details
            SURFACE_FORMAT_FXT1                                              = 402, //!< SET0_LEGACY: Undefined behavior if used in any feature added for GEN11+. See Legacy sampler feature page for detalls
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
            SURFACE_FORMAT_PLANAR42016                                       = 422, //!< No additional details
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

        //! \brief SAMPLE_TAP_DISCARD_DISABLE
        //! \details
        //!     This bit forces sample tap discard filter mode to be disabled for this
        //!     surface state. This bit must be set for surfaces which are no Alpha
        //!     Channel such as R8G8B8_UNORM.
        enum SAMPLE_TAP_DISCARD_DISABLE
        {
            SAMPLE_TAP_DISCARD_DISABLE_DISABLE                               = 0, //!< When programmed to 0h, Sample Tap Discard filter mode is allowed and is not disabled by this bit. This bit is ignored if Sample Tap Discard is not enabled in the Sampler State.
            SAMPLE_TAP_DISCARD_DISABLE_ENABLE                                = 1, //!< When programmed to 1h, Sample Tap Discard filter mode will be disabled even if enabled through Sampler State
        };

        //! \brief DOUBLE_FETCH_DISABLE
        //! \details
        //!     This bit is intended to disable the "double fetch" of adjacent
        //!     cache-lines in most all cases. Double fetch is a performance mode, but
        //!     for some surface types for formats it may be lower performance due to
        //!     fetching unused cache-lines.
        enum DOUBLE_FETCH_DISABLE
        {
            DOUBLE_FETCH_DISABLE_ENABLE                                      = 0, //!< When programmed to 0h, double fetches are allowed.
            DOUBLE_FETCH_DISABLE_DISABLE                                     = 1, //!< When programmed to 1h, double fetches are disabled.
        };

        //! \brief CORNER_TEXEL_MODE
        //! \details
        //!     This field, when ENABLED, indicates when a surface is using corner
        //!     texel-mode for sampling.
        //!     Corner Texel Mode is ignored for Planar YUV/YCrCb surface
        //!     formats.
        //!     Corner Texel Mode is ignored for sample_8X8 and sample_unorm messasge
        //!     types.
        //!     <!--StartFragment-->Does not support legacy sampler features set0 See
        //!     legacy sampler page for more details<!--EndFragment-->
        enum CORNER_TEXEL_MODE
        {
            CORNER_TEXEL_MODE_DISABLE                                        = 0, //!< When programmed to 0h, Corner Texel Mode is disabled.  This means texel references are shifted a half-texel from the upper-right corner of the texture map which is the standard texel referencemode.
            CORNER_TEXEL_MODE_ENABLE                                         = 1, //!< When programmed to 1h, Corner Texel Mode is enabled.  The location of a sampled texel on a texture map is shifted a half-texel to the upper-left, meaning texel (0,0) isin the exact upper-left corner of the surface.
        };

        //! \brief ENABLE_UNORM_PATH_IN_COLOR_PIPE
        //! \details
        //!     Enables Unorm Path (fixed Point Conversion of floating point for fill
        //!     and blend in DAPRSS) in color Pipe.
        enum ENABLE_UNORM_PATH_IN_COLOR_PIPE
        {
            ENABLE_UNORM_PATH_IN_COLOR_PIPE_DISABLE                          = 0, //!< Disables Unorm path in Color Pipe.
            ENABLE_UNORM_PATH_IN_COLOR_PIPE_ENABLE                           = 1, //!< Enables Unorm Path in Color Pipe.
        };

        //! \brief NULL_PROBING_ENABLE
        //! \details
        //!     This field is applicable for Render Targets only. It is used for
        //!     enabling NULL tile probing as defined under Tiled Resources. This
        //!     probing guarantees consistent read data return after previous write
        //!     including fast clears. SW must enable this bit for render target only if
        //!     surface contains NULL tiles and read after write consistency is
        //!     required.
        enum NULL_PROBING_ENABLE
        {
            NULL_PROBING_ENABLE_DISABLE                                      = 0, //!< No additional details
            NULL_PROBING_ENABLE_ENABLE                                       = 1, //!< No additional details
        };

        //! \brief STANDARD_TILING_MODE_EXTENSIONS
        //! \details
        //!     It changes in the MIP Tail Packing. When enabled (programmed to 1h),
        //!     MIP Tail packing for Volumetric and 1D are changed as defined in the
        //!     Surface Layout and Tiling section.
        //!     
        enum STANDARD_TILING_MODE_EXTENSIONS
        {
            STANDARD_TILING_MODE_EXTENSIONS_DISABLE                          = 0, //!< When programmed to 0h, the Gen11 extensions to support Standard Tiling are disabled. Behvaior reverts to Gen10 and Gen9 Miptail packing.
            STANDARD_TILING_MODE_EXTENSIONS_ENABLE                           = 1, //!< When programmed to 1h, the Gen11 changes to support Standard Tiling Extensions are enabled.  See the Surface Layout and Tiling section for details.
        };

        //! \brief TILE_ADDRESS_MAPPING_MODE
        //! \details
        //!     This field is used to select between Gen9 Tile Address Mapping mode and
        //!     Gen10 for TileYs and TileYf.
        enum TILE_ADDRESS_MAPPING_MODE
        {
            TILE_ADDRESS_MAPPING_MODE_GEN9                                   = 0, //!< Gen9 Tile Address Mapping ModeThis field should always be programmed to 0h. Gen9 Tiling mode is the standard tile layout for 3D.
            TILE_ADDRESS_MAPPING_MODE_GEN10                                  = 1, //!< Gen10+ Tile Address Mapping Mode (for Standard Tiling).This field should NOT be programmed to 1h as the Gen10 Tiling Address Mapping mode is not supported by SW.
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
            MULTISAMPLED_SURFACE_STORAGE_FORMAT_MSS                          = 0, //!< Multsampled surface was/is rendered as a render target
            MULTISAMPLED_SURFACE_STORAGE_FORMAT_DEPTHSTENCIL                 = 1, //!< Multisampled surface was rendered as a depth or stencil buffer
        };

        //! \brief RENDER_TARGET_AND_SAMPLE_UNORM_ROTATION
        //! \details
        //!     For Render Target Surfaces:
        //!     This field specifies the rotation of this render target surface
        //!     when being written to memory.
        enum RENDER_TARGET_AND_SAMPLE_UNORM_ROTATION
        {
            RENDER_TARGET_AND_SAMPLE_UNORM_ROTATION_0DEG                     = 0, //!< No rotation (0 degrees)
            RENDER_TARGET_AND_SAMPLE_UNORM_ROTATION_90DEG                    = 1, //!< Rotate by 90 degrees
            RENDER_TARGET_AND_SAMPLE_UNORM_ROTATION_180DEG                   = 2, //!< Rotate by 180 degrees [for sample_unorm message]
            RENDER_TARGET_AND_SAMPLE_UNORM_ROTATION_270DEG                   = 3, //!< Rotate by 270 degrees
        };

        enum DECOMPRESS_IN_L3
        {
            DECOMPRESS_IN_L3_DISABLE                                         = 0, //!< When this field is set to 0h, the associated compressible surface, when accessed by sampler, can be compressed in L3. If the surface is not compressible, this bit field is ignored.
            DECOMPRESS_IN_L3_ENABLE                                          = 1, //!< When this field is set to 1h, the associated compressible surface, when accessed by sampler, will be uncompressed in L3. If the surface is not compressible, this bit field is ignored.
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
        //!     For Sampling Engine, Render Target, and Typed/Untyped Surfaces:
        //!     This field specifies the tiled resource mode.
        //!     
        //!     For other surfaces:
        //!     This field is ignored.
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
            AUXILIARY_SURFACE_MODE_AUXCCSD                                   = 1, //!< The Auxiliary surface is a CCS (Color Control Surface) with color compression disabled. The Auxiliary surface is a MCS (Multisample Control Surface) with MSAA compression disabled. MCS is tiled y
            AUXILIARY_SURFACE_MODE_AUXAPPEND                                 = 2, //!< The Auxiliary surface is an append buffer
            AUXILIARY_SURFACE_MODE_AUXMCSLCE                                 = 4, //!< Enables lossless compresion on the top of MSAA compression for RTs with Number of Multisamples not equal to MULTISAMPLECOUNT_1.CCS is tiling format is linear.MCS tiling format is always tiledY.
            AUXILIARY_SURFACE_MODE_AUXCCSE                                   = 5, //!< If Number of multisamples = 1, programming this value means lossless compression is enabled for that surface.Auxiliary surface is a CCS with linear tiling. If Number of multisamples > 1, programming this value means MSAA compression is enabled for that surface. Auxillary surafce is MSC with tile y.
        };

        //! \brief YUV_INTERPOLATION_ENABLE
        //! \details
        //!     This bit controls whether a Non-Planar YUV4:2:2 and Planar YUV4:2:0
        //!     surface use interpolated or replicated U and V channels for input to the
        //!     Sampler filter. Programming to 1h causes interpolation of U and V
        //!     channels. In this case the chrominance for odd pixels is computed by an
        //!     interpolation between adjacent even pixels. Programming to 0h causes the
        //!     chrominance to be copied from the pixel to the left.
        enum YUV_INTERPOLATION_ENABLE
        {
            YUV_INTERPOLATION_ENABLE_DISABLE                                 = 0, //!< Programming to 0h causes the sampler to replicate U and V channels.  This will lead to lower quality in certain cases where the YUV surface is being filtered (e.g. linear).
            YUV_INTERPOLATION_ENABLE_ENABLE                                  = 1, //!< Programming to 1h causes the sampler to interpolate the U and V channels between the horizonally neighboring pixels.  This will improvie image quality if the surface is being filtered.
        };

        //! \brief HALF_PITCH_FOR_CHROMA
        //! \details
        //!     style="font-family: &quot;Arial&quot;,&quot;sans-serif&quot;;
        //!     font-size: 10pt; mso-fareast-font-family: &quot;Times New Roman&quot;;
        //!     mso-ansi-language: EN-US; mso-fareast-language: EN-US;
        //!     mso-bidi-language: AR-SA;"><font color="#000000">This bit enables
        //!     support for half-pitch chroma planes for Planar YUV surfaces. It is
        //!     ignored for Non-Planar surfaces. For planar surfaces it allows the
        //!     chroma planes to be one-half the width of a the Y (Luma)
        //!     plane.</font>
        //!     style="font-family: &quot;Arial&quot;,&quot;sans-serif&quot;;
        //!     font-size: 10pt; mso-fareast-font-family: &quot;Times New Roman&quot;;
        //!     mso-ansi-language: EN-US; mso-fareast-language: EN-US;
        //!     mso-bidi-language: AR-SA;"><font color="#000000">For example, should be
        //!     set to 0h for NV12 surfaces.</font>
        //!     style="font-family: &quot;Arial&quot;,&quot;sans-serif&quot;;
        //!     font-size: 10pt; mso-fareast-font-family: &quot;Times New Roman&quot;;
        //!     mso-ansi-language: EN-US; mso-fareast-language: EN-US;
        //!     mso-bidi-language: AR-SA;"><font color="#000000">Must be set to 1h for
        //!     YV12 surfaces.</font>
        enum HALF_PITCH_FOR_CHROMA
        {
            HALF_PITCH_FOR_CHROMA_DISABLE                                    = 0, //!< <font color="#000000">Setting this bit to 0h (default) causes Chroma planes to be treated as full width (same as Y plane).</font>
            HALF_PITCH_FOR_CHROMA_ENABLE                                     = 1, //!< <font color="#000000">Setting this bit to 1h causes Chroma planes (U and V) to be treated as half the width of the Luma (Y) plane.</font>
        };

        //! \brief SHADER_CHANNEL_SELECT_ALPHA
        //! \details
        //!     See Shader Channel Select Red for details.
        enum SHADER_CHANNEL_SELECT_ALPHA
        {
            SHADER_CHANNEL_SELECT_ALPHA_ZERO                                 = 0, //!< No additional details
            SHADER_CHANNEL_SELECT_ALPHA_ONE                                  = 1, //!< No additional details
            SHADER_CHANNEL_SELECT_ALPHA_RED                                  = 4, //!< Shader channel is set to surface red channel
            SHADER_CHANNEL_SELECT_ALPHA_GREEN                                = 5, //!< Shader channel is set to surface green channel
            SHADER_CHANNEL_SELECT_ALPHA_BLUE                                 = 6, //!< Shader channel is set to surface blue channel
            SHADER_CHANNEL_SELECT_ALPHA_ALPHA                                = 7, //!< Shader channel is set to surface alpha channel
        };

        //! \brief SHADER_CHANNEL_SELECT_BLUE
        //! \details
        //!     See Shader Channel Select Red for details.
        enum SHADER_CHANNEL_SELECT_BLUE
        {
            SHADER_CHANNEL_SELECT_BLUE_ZERO                                  = 0, //!< No additional details
            SHADER_CHANNEL_SELECT_BLUE_ONE                                   = 1, //!< No additional details
            SHADER_CHANNEL_SELECT_BLUE_RED                                   = 4, //!< Shader channel is set to surface red channel
            SHADER_CHANNEL_SELECT_BLUE_GREEN                                 = 5, //!< Shader channel is set to surface green channel
            SHADER_CHANNEL_SELECT_BLUE_BLUE                                  = 6, //!< Shader channel is set to surface blue channel
            SHADER_CHANNEL_SELECT_BLUE_ALPHA                                 = 7, //!< Shader channel is set to surface alpha channel
        };

        //! \brief SHADER_CHANNEL_SELECT_GREEN
        //! \details
        //!     See Shader Channel Select Red for details.
        enum SHADER_CHANNEL_SELECT_GREEN
        {
            SHADER_CHANNEL_SELECT_GREEN_ZERO                                 = 0, //!< No additional details
            SHADER_CHANNEL_SELECT_GREEN_ONE                                  = 1, //!< No additional details
            SHADER_CHANNEL_SELECT_GREEN_RED                                  = 4, //!< Shader channel is set to surface red channel
            SHADER_CHANNEL_SELECT_GREEN_GREEN                                = 5, //!< Shader channel is set to surface green channel
            SHADER_CHANNEL_SELECT_GREEN_BLUE                                 = 6, //!< Shader channel is set to surface blue channel
            SHADER_CHANNEL_SELECT_GREEN_ALPHA                                = 7, //!< Shader channel is set to surface alpha channel
        };

        //! \brief SHADER_CHANNEL_SELECT_RED
        //! \details
        //!     Specifies which surface channel is read or written in the Red shader
        //!     channel.
        enum SHADER_CHANNEL_SELECT_RED
        {
            SHADER_CHANNEL_SELECT_RED_ZERO                                   = 0, //!< No additional details
            SHADER_CHANNEL_SELECT_RED_ONE                                    = 1, //!< No additional details
            SHADER_CHANNEL_SELECT_RED_RED                                    = 4, //!< Shader channel is set to surface red channel
            SHADER_CHANNEL_SELECT_RED_GREEN                                  = 5, //!< Shader channel is set to surface green channel
            SHADER_CHANNEL_SELECT_RED_BLUE                                   = 6, //!< Shader channel is set to surface blue channel
            SHADER_CHANNEL_SELECT_RED_ALPHA                                  = 7, //!< Shader channel is set to surface alpha channel
        };

        //! \brief MEMORY_COMPRESSION_MODE
        //! \details
        //!     MBZ: Only Horizontal mode is supported.
        enum MEMORY_COMPRESSION_MODE
        {
            MEMORY_COMPRESSION_MODE_HORIZONTAL                               = 0, //!< No additional details
        };

        //! \brief CLEAR_VALUE_ADDRESS_ENABLE
        //! \details
        //!     This field enables HW Managed Clear Value Layout for the Surface State.
        //!     If this bit is enabled, Clear Value Address is present instead of
        //!     explicit clear values.
        enum CLEAR_VALUE_ADDRESS_ENABLE
        {
            CLEAR_VALUE_ADDRESS_ENABLE_DISABLE                               = 0, //!< Clear values are present in the surface state explicitly.
            CLEAR_VALUE_ADDRESS_ENABLE_ENABLE                                = 1, //!< Clear value Address is present instead of explicit clear values.
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
            struct
            {
                uint32_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0, 15)    ; //!< Reserved
                uint32_t                 YOffset                                          : __CODEGEN_BITFIELD(16, 19)    ; //!< Y Offset
                uint32_t                 XOffset                                          : __CODEGEN_BITFIELD(20, 26)    ; //!< X Offset
                uint32_t                 Reserved27                                       : __CODEGEN_BITFIELD(27, 29)    ; //!< Reserved
                uint32_t                 Rotation                                         : __CODEGEN_BITFIELD(30, 31)    ; //!< ROTATION
            };
            uint32_t                     Value;
        } DW0;
        union
        {
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
            struct
            {
                uint32_t                 TileMode                                         : __CODEGEN_BITFIELD( 0,  1)    ; //!< TILE_MODE
                uint32_t                 HalfPitchForChroma                               : __CODEGEN_BITFIELD( 2,  2)    ; //!< Half Pitch for Chroma
                uint32_t                 SurfacePitch                                     : __CODEGEN_BITFIELD( 3, 20)    ; //!< Surface Pitch
                uint32_t                 AddressControl                                   : __CODEGEN_BITFIELD(21, 21)    ; //!< ADDRESS_CONTROL
                uint32_t                 MemoryCompressionEnable                          : __CODEGEN_BITFIELD(22, 22)    ; //!< Memory Compression Enable
                uint32_t                 MemoryCompressionType                            : __CODEGEN_BITFIELD(23, 23)    ; //!< MEMORY_COMPRESSION_TYPE
                uint32_t                 CrVCbUPixelOffsetVDirectionMsb                   : __CODEGEN_BITFIELD(24, 24)    ; //!< CRVCBU_PIXEL_OFFSET_V_DIRECTION_MSB
                uint32_t                 CrVCbUPixelOffsetUDirection                      : __CODEGEN_BITFIELD(25, 25)    ; //!< CRVCBU_PIXEL_OFFSET_U_DIRECTION
                uint32_t                 InterleaveChroma                                 : __CODEGEN_BITFIELD(26, 26)    ; //!< Interleave Chroma
                uint32_t                 SurfaceFormat                                    : __CODEGEN_BITFIELD(27, 31)    ; //!< SURFACE_FORMAT
            };
            uint32_t                     Value;
        } DW2;
        union
        {
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
            struct
            {
                uint32_t                 SurfaceBaseAddress                                                               ; //!< Surface Base Address
            };
            uint32_t                     Value;
        } DW6;
        union
        {
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

        //! \brief MEMORY_COMPRESSION_TYPE
        //! \details
        //!     Specifies the type of memory compression used.
        enum MEMORY_COMPRESSION_TYPE
        {
            MEMORY_COMPRESSION_TYPE_MEDIACOMPRESSION                         = 0, //!< No additional details
            MEMORY_COMPRESSION_TYPE_RENDERCOMPRESSION                        = 1, //!< No additional details
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
            SURFACE_FORMAT_R10G10B10A2UNORM                                  = 8, //!< Sample_8x8 only
            SURFACE_FORMAT_R8G8B8A8UNORM                                     = 9, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_R8B8UNORM_CRCB                                    = 10, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_R8UNORM_CRCB                                      = 11, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_Y8UNORM                                           = 12, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_A8Y8U8V8UNORM                                     = 13, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_B8G8R8A8UNORM                                     = 14, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_R16G16B16A16                                      = 15, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_PLANAR4228                                        = 18, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_PLANAR42016                                       = 23, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_R16B16UNORM_CRCB                                  = 24, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_R16UNORM_CRCB                                     = 25, //!< Sample_8x8 AVS only
            SURFACE_FORMAT_Y16UNORM                                          = 26, //!< Sample_8x8 AVS only
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
        //!     For Sampling Engine, Render Target, and Typed/Untyped Surfaces:
        //!     This field specifies the tiled resource mode.
        //!     For other surfaces:
        //!     This field is ignored.
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
                uint32_t                 CpsLodCompensationEnable                         : __CODEGEN_BITFIELD(30, 30)    ; //!< CPS LOD Compensation Enable
                uint32_t                 SamplerDisable                                   : __CODEGEN_BITFIELD(31, 31)    ; //!< Sampler Disable
            };
            uint32_t                     Value;
        } DW0;
        union
        {
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
            struct
            {
                uint32_t                 LodClampMagnificationMode                        : __CODEGEN_BITFIELD( 0,  0)    ; //!< LOD_CLAMP_MAGNIFICATION_MODE
                uint32_t                 SrgbDecode                                       : __CODEGEN_BITFIELD( 1,  1)    ; //!< SRGB_DECODE
                uint32_t                 ReturnFilterWeightForNullTexels                  : __CODEGEN_BITFIELD( 2,  2)    ; //!< RETURN_FILTER_WEIGHT_FOR_NULL_TEXELS
                uint32_t                 ReturnFilterWeightForBorderTexels                : __CODEGEN_BITFIELD( 3,  3)    ; //!< RETURN_FILTER_WEIGHT_FOR_BORDER_TEXELS
                uint32_t                 Reserved68                                       : __CODEGEN_BITFIELD( 4,  5)    ; //!< Reserved
                uint32_t                 IndirectStatePointer                             : __CODEGEN_BITFIELD( 6, 23)    ; //!< Indirect State Pointer
                uint32_t                 Reserved88                                       : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
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
                uint32_t                 AllowLowQualityLodCalculation                    : __CODEGEN_BITFIELD(24, 24)    ; //!< Allow low quality LOD calculation
                uint32_t                 Reserved121                                      : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
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
            LOD_ALGORITHM_LEGACY                                             = 0, //!< Use the legacy algorithm for non-anisotropic filtering
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
        //!     This field determines whether the computed LOD is clamped to
        //!     [max,min] mip level
        //!     before the mag-vs-min determination is performed.
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
            TEXTURE_BORDER_COLOR_MODE_OGL                                    = 0, //!< New mode for interpreting the border color
            TEXTURE_BORDER_COLOR_MODE_8BIT                                   = 1, //!< Earlier mode for interpreting the border color
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

        //! \brief SRGB_DECODE
        //! \details
        //!     This bit controls whether the 3D sampler will decode an sRGB
        //!     formatted surface into RGB prior to any filtering operation.
        //!     When set, it does not convert to linear RGB (via a reverse gamma
        //!     conversion). This bit is ignored for ASTC formats, which are always
        //!     converted to linear RGB prior to filtering.
        enum SRGB_DECODE
        {
            SRGB_DECODE_DECODEEXT                                            = 0, //!< When set to 0h, the 3D sampler will convert texels from an sRGB surface to linear RGB prior to filtering and/or returning the value.
            SRGB_DECODE_SKIPDECODEEXT                                        = 1, //!< When set to 1h, the 3D sampler will not convert texels to linear RGB before filtering and returning results.
        };

        //! \brief RETURN_FILTER_WEIGHT_FOR_NULL_TEXELS
        //! \details
        //!     This bit, when set, causes samples to return filter_weight of all
        //!     non-NULL texels in the Alpha channel; Red, Green, and Blue channels are
        //!     contain the filter result with NULL texels excluded; A non-NULL texel is
        //!     a texel which does not reference a Null Tile.
        //!     For cases where Tiled_Resource_Mode is TR_NONE, the result will
        //!     always be 1.0 since no texels would be NULL.
        //!     For cases where the surface format contains an Alpha channel, the
        //!     result returned will be overridden to return the filter weight.
        //!     For cases where the surface format does not contain Alpha, the result
        //!     will still be returned in the Alpha Channel.
        enum RETURN_FILTER_WEIGHT_FOR_NULL_TEXELS
        {
            RETURN_FILTER_WEIGHT_FOR_NULL_TEXELS_DISABLE                     = 0, //!< When programmed to 0h, filter weight will not be returned, and normal data will be returned on the Alpha channel.
            RETURN_FILTER_WEIGHT_FOR_NULL_TEXELS_ENABLE                      = 1, //!< When programmed to 1h, filter weight will be returned on the Alpha channel rather than the normal data expected on the Alpha channel.
        };

        //! \brief RETURN_FILTER_WEIGHT_FOR_BORDER_TEXELS
        //! \details
        //!     This bit, when set, returns the filter_weight in the Alpha channel of
        //!     all non-border texels. Red, Green, and Blue channels will contain the
        //!     sample result with border texels excluded.
        //!     For cases where the surface format contains an Alpha channel, the
        //!     result returned will be ovewritten to return the filter weight.
        //!     For cases where the surface format does not contain Alpha, the result
        //!     will still be returned in the Alpha Channel.
        enum RETURN_FILTER_WEIGHT_FOR_BORDER_TEXELS
        {
            RETURN_FILTER_WEIGHT_FOR_BORDER_TEXELS_DISABLE                   = 0, //!< When programmed to 0h, normal data will be returned on RGBA channels, including contribution from border color texels.
            RETURN_FILTER_WEIGHT_FOR_BORDER_TEXELS_ENABLE                    = 1, //!< When programmed to 1h, RGB channels return filter data contributed from non-border color texels, and A channel returns filter weight of contributing texels.
        };

        //! \brief TCZ_ADDRESS_CONTROL_MODE
        //! \details
        //!     Controls how the 3rd (TCZ) component of input texture coordinates are
        //!     mapped to texture map addresses - specifically, how coordinates
        //!     "outside" the texture are handled (wrap/clamp/mirror).See Address TCX
        //!     Control Mode above for details
        enum TCZ_ADDRESS_CONTROL_MODE
        {
            TCZ_ADDRESS_CONTROL_MODE_WRAP                                    = 0, //!< Map is repeated in the U direction
            TCZ_ADDRESS_CONTROL_MODE_MIRROR                                  = 1, //!< Map is mirrored in the U direction
            TCZ_ADDRESS_CONTROL_MODE_CLAMP                                   = 2, //!< Map is clamped to the edges of the accessed map
            TCZ_ADDRESS_CONTROL_MODE_CUBE                                    = 3, //!< For cube-mapping, filtering in edges access adjacent map faces
            TCZ_ADDRESS_CONTROL_MODE_CLAMPBORDER                             = 4, //!< Map is infinitely extended with the border color
            TCZ_ADDRESS_CONTROL_MODE_MIRRORONCE                              = 5, //!< Map is mirrored once about origin, then clamped
            TCZ_ADDRESS_CONTROL_MODE_HALFBORDER                              = 6, //!< Map is infinitely extended with the average of the nearest edge texel and the border color
            TCZ_ADDRESS_CONTROL_MODE_MIRROR101                               = 7, //!< Map is mirrored one time in each direction, but the first pixel of the reflected image is skipped, and the reflected image is effectively 1 pixel less in that direction.May only be used on 2D surfaces.
        };

        //! \brief TCY_ADDRESS_CONTROL_MODE
        //! \details
        //!     Controls how the 2nd (TCY, aka V) component of input texture coordinates
        //!     are mapped to texture map addresses - specifically, how coordinates
        //!     "outside" the texture are handled (wrap/clamp/mirror). See Address TCX
        //!     Control Mode above for details
        enum TCY_ADDRESS_CONTROL_MODE
        {
            TCY_ADDRESS_CONTROL_MODE_WRAP                                    = 0, //!< Map is repeated in the U direction
            TCY_ADDRESS_CONTROL_MODE_MIRROR                                  = 1, //!< Map is mirrored in the U direction
            TCY_ADDRESS_CONTROL_MODE_CLAMP                                   = 2, //!< Map is clamped to the edges of the accessed map
            TCY_ADDRESS_CONTROL_MODE_CUBE                                    = 3, //!< For cube-mapping, filtering in edges access adjacent map faces
            TCY_ADDRESS_CONTROL_MODE_CLAMPBORDER                             = 4, //!< Map is infinitely extended with the border color
            TCY_ADDRESS_CONTROL_MODE_MIRRORONCE                              = 5, //!< Map is mirrored once about origin, then clamped
            TCY_ADDRESS_CONTROL_MODE_HALFBORDER                              = 6, //!< Map is infinitely extended with the average of the nearest edge texel and the border color
            TCY_ADDRESS_CONTROL_MODE_MIRROR101                               = 7, //!< Map is mirrored one time in each direction, but the first pixel of the reflected image is skipped, and the reflected image is effectively 1 pixel less in that direction.May only be used on 2D surfaces.
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
            TCX_ADDRESS_CONTROL_MODE_WRAP                                    = 0, //!< Map is repeated in the U direction
            TCX_ADDRESS_CONTROL_MODE_MIRROR                                  = 1, //!< Map is mirrored in the U direction
            TCX_ADDRESS_CONTROL_MODE_CLAMP                                   = 2, //!< Map is clamped to the edges of the accessed map
            TCX_ADDRESS_CONTROL_MODE_CUBE                                    = 3, //!< For cube-mapping, filtering in edges access adjacent map faces
            TCX_ADDRESS_CONTROL_MODE_CLAMPBORDER                             = 4, //!< Map is infinitely extended with the border color
            TCX_ADDRESS_CONTROL_MODE_MIRRORONCE                              = 5, //!< Map is mirrored once about origin, then clamped
            TCX_ADDRESS_CONTROL_MODE_HALFBORDER                              = 6, //!< Map is infinitely extended with the average of the nearest edge texel and the border color
            TCX_ADDRESS_CONTROL_MODE_MIRROR101                               = 7, //!< Map is mirrored one time in each direction, but the first pixel of the reflected image is skipped, and the reflected image is effectively 1 pixel less in that direction.May only be used on 2D surfaces.
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
        //!     texels in the footprint defined by the Min/Mag/Mip Filter Mode
        //!     fields.  This field is ignored if Reduction Type Enable is
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
    //!     ExistsIf = AVS
    //!     
    struct SAMPLER_STATE_8x8_AVS_CMD
    {
        uint32_t                                 Reserved0[3];                                                            //!< Reserved
        union
        {
            struct
            {
                uint32_t                 Reserved96                                       : __CODEGEN_BITFIELD( 0, 27)    ; //!< Reserved
                uint32_t                 Enable8TapFilter                                 : __CODEGEN_BITFIELD(28, 29)    ; //!< ENABLE_8_TAP_FILTER
                uint32_t                 Reserved126                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 Reserved128                                      : __CODEGEN_BITFIELD( 0, 10)    ; //!< Reserved
                uint32_t                 ShuffleOutputwritebackForSample8X8               : __CODEGEN_BITFIELD(11, 11)    ; //!< SHUFFLE_OUTPUTWRITEBACK_FOR_SAMPLE_8X8
                uint32_t                 Reserved140                                      : __CODEGEN_BITFIELD(12, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        uint32_t                                 Reserved160[11];                                                         //!< Reserved
        SAMPLER_STATE_8x8_AVS_COEFFICIENTS_CMD   FilterCoefficient016[17];                                                //!< DW16..151, Filter Coefficient[0..16]
        union
        {
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
        uint32_t                                 Reserved4928[6];                                                         //!< Reserved
        SAMPLER_STATE_8x8_AVS_COEFFICIENTS_CMD   FilterCoefficient1731[15];                                               //!< DW160..279, Filter Coefficient[17..31]

        //! \name Local enumerations

        //! \brief ENABLE_8_TAP_FILTER
        //! \details
        //!     Adaptive Filtering (Mode = 11) ExistsIf:
        //!      R10G10B10A2_UNORM R8G8B8A8_UNORM (AYUV also) R8B8G8A8_UNORM
        //!     B8G8R8A8_UNORM R16G16B16A16
        enum ENABLE_8_TAP_FILTER
        {
            ENABLE_8_TAP_FILTER_UNNAMED0                                     = 0, //!< 4-tap filter is only done on all channels.
            ENABLE_8_TAP_FILTER_UNNAMED1                                     = 1, //!< Enable 8-tap Adaptive filter on G-channel. 4-tap filter on other channels.
            ENABLE_8_TAP_FILTER_UNNAMED2                                     = 2, //!< 8-tap filter is done on all channels (UV-ch uses the Y-coefficients)
            ENABLE_8_TAP_FILTER_UNNAMED3                                     = 3, //!< Enable 8-tap Adaptive filter all channels (UV-ch uses the Y-coefficients).
        };

        enum SHUFFLE_OUTPUTWRITEBACK_FOR_SAMPLE_8X8
        {
            SHUFFLE_OUTPUTWRITEBACK_FOR_SAMPLE_8X8_UNNAMED0                  = 0, //!< Writeback same as Original Sample_8x8
            SHUFFLE_OUTPUTWRITEBACK_FOR_SAMPLE_8X8_UNNAMED1                  = 1, //!< Writeback  of Sample_8x8 Is Modified to Suite Sample_Unorm
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
        //!     This should be enabled only if we enable 8-tap adaptive filter for
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
        //!     When disabled, the Y direction will use Default Sharpness Level
        //!     to blend between the smooth and sharp filters rather than the calculated
        //!     value.
        enum BYPASS_Y_ADAPTIVE_FILTERING
        {
            BYPASS_Y_ADAPTIVE_FILTERING_ENABLE                               = 0, //!< Enable Y Adaptive Filtering
            BYPASS_Y_ADAPTIVE_FILTERING_DISBLE                               = 1, //!< Disable Y Adaptive Filtering
        };

        //! \brief BYPASS_X_ADAPTIVE_FILTERING
        //! \details
        //!     When disabled, the X direction will use Default Sharpness Level
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
            struct
            {
                uint32_t                 BorderColorRed                                   : __CODEGEN_BITFIELD( 0,  7)    ; //!< Border Color Red, Structure[SAMPLER_STATE][Texture Border Color Mode] == '8BIT'
                uint32_t                 BorderColorGreen                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< Border Color Green, Structure[SAMPLER_STATE][Texture Border Color Mode] == '8BIT'
                uint32_t                 Reserved16                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved, Structure[SAMPLER_STATE][Texture Border Color Mode] == '8BIT'
            } Obj0;
            struct
            {
                uint32_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0, 15)    ; //!< Reserved, Structure[SAMPLER_STATE][Texture Border Color Mode] == '8BIT'
                uint32_t                 BorderColorBlue                                  : __CODEGEN_BITFIELD(16, 23)    ; //!< Border Color Blue, Structure[SAMPLER_STATE][Texture Border Color Mode] == '8BIT'
                uint32_t                 BorderColorAlpha                                 : __CODEGEN_BITFIELD(24, 31)    ; //!< Border Color Alpha, Structure[SAMPLER_STATE][Texture Border Color Mode] == '8BIT'
            } Obj1;
            struct
            {
                uint32_t                 BorderColorRed                                                                   ; //!< Border Color Red, Structure[SAMPLER_STATE][Texture Border Color Mode] == 'OGL'
            } Obj2;
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 BorderColorGreen                                                                 ; //!< Border Color Green, Structure[SAMPLER_STATE][Texture Border Color Mode] == 'OGL'
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 BorderColorBlue                                                                  ; //!< Border Color Blue, Structure[SAMPLER_STATE][Texture Border Color Mode] == 'OGL'
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 BorderColorAlpha                                                                 ; //!< Border Color Alpha, Structure[SAMPLER_STATE][Texture Border Color Mode] == 'OGL'
            };
            uint32_t                     Value;
        } DW3;
        uint32_t                                 Reserved128[12];                                                         //!< Reserved

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        SAMPLER_INDIRECT_STATE_CMD();

        static const size_t dwSize = 16;
        static const size_t byteSize = 64;
    };

};

#pragma pack()

#endif  // __MHW_STATE_HEAP_HWCMD_G12_X_H__