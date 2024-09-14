/*===================== begin_copyright_notice ==================================

# Copyright (c) 2021-2024, Intel Corporation

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
//! \file     mhw_vdbox_aqm_hwcmd_xe2_lpm.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of Xe2_Lpm as other components
//!           should use MHW interface to interact with MHW commands and states.
//!

// DO NOT EDIT

#ifndef __MHW_VDBOX_AQM_HWCMD_XE2_LPM_H__
#define __MHW_VDBOX_AQM_HWCMD_XE2_LPM_H__

#pragma once
#pragma pack(1)

#include "mhw_hwcmd.h"
#include <cstdint>
#include <cstddef>
#include "media_class_trace.h"

#ifdef IGFX_AQM_INTERFACE_EXT_SUPPORT
#include "mhw_vdbox_aqm_hwcmd_ext.h"
#endif


namespace mhw
{
namespace vdbox
{
namespace aqm
{
namespace xe2_lpm
{
class Cmd
{
public:
    virtual ~Cmd() = default;

    static uint32_t GetOpLength(uint32_t uiLength) { return __CODEGEN_OP_LENGTH(uiLength); }

    //!
    //! \brief MEMORYADDRESSATTRIBUTES
    //! \details
    //!     This field controls the priority of arbitration used in the GAC/GAM
    //!     pipeline for this surface. It defines the attributes for VDBOX addresses
    //!     on BDW+.
    //!     
    struct MEMORYADDRESSATTRIBUTES_CMD
    {
        union
        {
            struct
            {
                uint32_t                 MEMORYADDRESSATTRIBUTES_DW0_BIT0                 : __CODEGEN_BITFIELD( 0,  0)    ;
                uint32_t                 BaseAddressIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< Base Address - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 BaseAddressArbitrationPriorityControl            : __CODEGEN_BITFIELD( 7,  8)    ; //!< Base Address - Arbitration Priority Control
                uint32_t                 BaseAddressMemoryCompressionEnable               : __CODEGEN_BITFIELD( 9,  9)    ; //!< Base Address - Memory Compression Enable
                uint32_t                 CompressionType                                  : __CODEGEN_BITFIELD(10, 10)    ; //!< COMPRESSION_TYPE
                uint32_t                 Reserved11                                       : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 BaseAddressRowStoreScratchBufferCacheSelect      : __CODEGEN_BITFIELD(12, 12)    ; //!< BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 Tilemode                                         : __CODEGEN_BITFIELD(13, 14)    ; //!< TILEMODE
                uint32_t                 Reserved15                                       : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \brief COMPRESSION_TYPE
        //! \details
        //!     Indicates if buffer is render/media compressed.
        enum COMPRESSION_TYPE
        {
            COMPRESSION_TYPE_MEDIACOMPRESSIONENABLE                          = 0, //!< No additional details
            COMPRESSION_TYPE_RENDERCOMPRESSIONENABLE                         = 1, //!< Only support rendered compression with unified memory
        };

        //! \brief BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        //! \details
        //!     This field controls if the Row Store is going to store inside Media
        //!     Cache (rowstore cache) or to LLC.
        enum BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
        {
            BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED0      = 0, //!< Buffer going to LLC.
            BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1      = 1, //!< Buffer going to Internal Media Storage.
        };

        enum TILEMODE
        {
            TILEMODE_LINEAR                                                  = 0, //!< No additional details
            TILEMODE_TILES_64K                                               = 1, //!< No additional details
            TILEMODE_TILEX                                                   = 2, //!< No additional details
            TILEMODE_TILEF                                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MEMORYADDRESSATTRIBUTES_CMD()
        {
            DW0.Value = 0x00000000;
            //DW0.CompressionType                              = COMPRESSION_TYPE_MEDIACOMPRESSIONENABLE;
            //DW0.BaseAddressRowStoreScratchBufferCacheSelect  = BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED0;
            //DW0.Tilemode                                     = TILEMODE_LINEAR;
        }

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief SPLITBASEADDRESS4KBYTEALIGNED
    //! \details
    //!     Specifies a 64-bit (48-bit canonical) 4K-byte aligned memory base
    //!     address. GraphicsAddress is a 64-bit value [63:0], but only a portion of
    //!     it is used by hardware. The upper reserved bits are ignored and MBZ.
    //!     
    //!     Bits 63:48 must be zero.
    //!     
    struct SPLITBASEADDRESS4KBYTEALIGNED_CMD
    {
        union
        {
            struct
            {
                uint64_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint64_t                 BaseAddress                                      : __CODEGEN_BITFIELD(12, 56)    ; //!< Base Address
                uint64_t                 Reserved57                                       : __CODEGEN_BITFIELD(57, 63)    ; //!< Reserved
            };
            uint32_t                     Value[2];
        } DW0_1;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        SPLITBASEADDRESS4KBYTEALIGNED_CMD()
        {
            DW0_1.Value[0] = DW0_1.Value[1] = 0x00000000;
        }

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief SPLITBASEADDRESS64BYTEALIGNED
    //! \details
    //!     Specifies a 64-bit (48-bit canonical) 64-byte aligned memory base
    //!     address.
    //!     
    //!     Bits 63:48 must be zero.
    //!     
    struct SPLITBASEADDRESS64BYTEALIGNED_CMD
    {
        union
        {
            struct
            {
                uint64_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint64_t                 BaseAddress                                      : __CODEGEN_BITFIELD( 6, 56)    ; //!< Base Address
                uint64_t                 Reserved57                                       : __CODEGEN_BITFIELD(57, 63)    ; //!< Reserved
            };
            uint32_t                     Value[2];
        } DW0_1;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        SPLITBASEADDRESS64BYTEALIGNED_CMD()
        {
            DW0_1.Value[0] = DW0_1.Value[1] = 0x00000000;
        }

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief VD_CONTROL_STATE_BODY
    //! \details
    //!
    //!
    struct VD_CONTROL_STATE_BODY_CMD
    {
        union
        {
            struct
            {
                uint32_t                 PipelineInitialization                           : __CODEGEN_BITFIELD( 0,  0)    ; //!< Pipeline Initialization
                uint32_t                 VDboxPipelineArchitectureClockgateDisable        : __CODEGEN_BITFIELD( 1,  1)    ; //!< VDBOX_PIPELINE_ARCHITECTURE_CLOCKGATE_DISABLE
                uint32_t                 Reserved2                                        : __CODEGEN_BITFIELD( 2, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 PipeScalableModePipeLock                         : __CODEGEN_BITFIELD( 0,  0)    ; //!< Pipe Scalable Mode Pipe Lock
                uint32_t                 PipeScalableModePipeUnlock                       : __CODEGEN_BITFIELD( 1,  1)    ; //!< Pipe Scalable Mode Pipe Unlock
                uint32_t                 MemoryImplicitFlush                              : __CODEGEN_BITFIELD( 2,  2)    ; //!< Memory Implicit Flush
                uint32_t                 Reserved35                                       : __CODEGEN_BITFIELD( 3, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;

        //! \name Local enumerations

        //! \brief VDBOX_PIPELINE_ARCHITECTURE_CLOCKGATE_DISABLE
        //! \details
        //!     This is used to disable the architecture clockgate for AWM (in AVP
        //!     pipeline) or HWM (in HCP pipeline) if needed.
        enum VDBOX_PIPELINE_ARCHITECTURE_CLOCKGATE_DISABLE
        {
            VDBOX_PIPELINE_ARCHITECTURE_CLOCKGATE_DISABLE_ENABLE             = 0, //!< No additional details
            VDBOX_PIPELINE_ARCHITECTURE_CLOCKGATE_DISABLE_DISABLE            = 1, //!< No additional details
        };


        //! \name Initializations

        //! \brief Explicit member initialization function
        VD_CONTROL_STATE_BODY_CMD()
        {
            DW0.Value = 0x00000000;

            DW1.Value = 0x00000000;
        }

        static const size_t dwSize   = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief AQM_FRAME_START
    //! \details
    //!     This command is used only for VDAQM. It is issued for every tile of a
    //!     frame. If a frame is composed of only 1 tile, it is still being issued.
    //!     Tiling and Tile Group organization in AV1 cannot be disabled, a frame
    //!     minimum must have 1 tile. Currently, each batch buffer can contain only
    //!     1 tile to be processed, it cannot contain more than 1 tile or the entire
    //!     tile group of tiles.  When the tile width exceeds 4096 pixels or the
    //!     tile area exceeds 4096x2304 pixels, tiling must be performed and number
    //!     of tiles in such frame must be >1. There is no mandatory tiling driven
    //!     by tile height. The frame height in pixels will limit the allowed tile
    //!     height in extreme situation. Hence, the AVP_TILE_CODING can be issued
    //!     multiple times for decoding a frame.
    //!     
    struct AQM_FRAME_START_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 22)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(23, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 AqmFrameStart                                                                    ; //!< AQM_FRAME_START
            };
            uint32_t                     Value;
        } DW1;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_AQMFRAMESTART                          = 4, //!< No additional details
        };

        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEAQM                      = 5, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief AQM_FRAME_START
        //! \details
        //!     This command is sent as the last command to be dispatched to VDAQM
        //!     pipe
        enum AQM_FRAME_START
        {
            AQM_FRAME_START_UNNAMED0                                         = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        AQM_FRAME_START_CMD()
        {
            DW0.Value = 0x72840000;
            //DW0.DwordLength                                  = GetOpLength(dwSize);
            //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AQMFRAMESTART;
            //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEAQM;
            //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
            //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

            DW1.Value = 0x00000000;
        }

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

#ifdef IGFX_AQM_INTERFACE_EXT_SUPPORT
#include "mhw_vdbox_aqm_hwcmd_xe2_lpm_ext.h"
#else
    //!
    //! \brief AQM_PIC_STATE
    //! \details
    //!     All AQM_PIC_STATE should stay the same for the whole frame even if
    //!     AQM_PIC_STATE is re-programmed for every tiles.
    //!       
    //!     
    struct AQM_PIC_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 22)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(23, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 FrameWidthInPixelMinus1                          : __CODEGEN_BITFIELD( 0, 15)    ; //!< Frame Width In Pixel Minus 1
                uint32_t                 FrameHeightInPixelMinus1                         : __CODEGEN_BITFIELD(16, 31)    ; //!< Frame Height In Pixel Minus 1
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 Reserved69                                       : __CODEGEN_BITFIELD( 0, 18)    ; //!< Reserved
                uint32_t                 VdaqmEnable                                      : __CODEGEN_BITFIELD(19, 19)    ; //!< VDAQM Enable
                uint32_t                 TileBasedEngine                                  : __CODEGEN_BITFIELD(20, 20)    ; //!< TILE_BASED_ENGINE
                uint32_t                 LcuSize                                          : __CODEGEN_BITFIELD(21, 22)    ; //!< LCU_SIZE
                uint32_t                 Reserved87                                       : __CODEGEN_BITFIELD(23, 23)    ; //!< Reserved
                uint32_t                 Pixelbitdepth                                    : __CODEGEN_BITFIELD(24, 25)    ; //!< PIXELBITDEPTH
                uint32_t                 Chromasubsampling                                : __CODEGEN_BITFIELD(26, 27)    ; //!< CHROMASUBSAMPLING
                uint32_t                 AqmMode                                          : __CODEGEN_BITFIELD(28, 28)    ; //!< AQM_MODE
                uint32_t                 Codectype                                        : __CODEGEN_BITFIELD(29, 31)    ; //!< CODECTYPE
            };
            uint32_t                     Value;
        } DW2;
        uint32_t                         DW3_DW17[15];
        union
        {
            struct
            {
                uint32_t                 Reserved585                                      : __CODEGEN_BITFIELD( 0, 23)    ; //!< Reserved
                uint32_t                 SseEnable                                        : __CODEGEN_BITFIELD(24, 24)    ; //!< SSE Enable
                uint32_t                 Reserved601                                      : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW18;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_AQMPICSTATE                            = 3, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = AQM = 5h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEAQM                      = 5, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        AQM_PIC_STATE_CMD()
        {
            MOS_ZeroMemory(this, sizeof(*this));
            DW0.Value = 0x72830011;
            //DW0.DwordLength                                  = GetOpLength(dwSize);
            //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AQMPICSTATE;
            //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEAQM;
            //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
            //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;
        }

        static const size_t dwSize = 19;
        static const size_t byteSize = 76;
    };
#endif

    //!
    //! \brief AQM_PIPE_BUF_ADDR_STATE
    //! \details
    //!     The AQM Pipeline is selected with the Media Instruction Opcode "8h" for
    //!     all AQM Commands. Each AQM command has assigned a media instruction
    //!     command as defined in DWord 0, BitField 22:16.
    //!     
    //!     This state command provides the physical memory base addresses for all
    //!     row store buffers, reconstructed output and reference frame buffers, and
    //!     auxiliary data buffers (MV, segment map, etc.) that are required by the
    //!     AV1 decoding and encoding process.This is a frame level state command
    //!     and is shared by both encoding and decoding processes.AVP is a tile
    //!     based pipeline and is a stateless pipeline, hence all sequence level,
    //!     frame level, and segment level state commands must be resent to process
    //!     each tile.Memory compression may be applicable to some of these buffers
    //!     for BW saving.Note : there is no buffer to store the 16 QM table sets,
    //!     they are implemented directly inside the HW pipeline.
    //!     
    struct AQM_PIPE_BUF_ADDR_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 22)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(23, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        SourcePixelsFrameBufferAddress;                                          //!< DW1..2, Source Pixels Frame Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              SourcePixelsFrameBufferAddressAttributes;                                //!< DW3, Source Pixels Frame Buffer Address Attributes
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        ReconstructedPixelsFrameBufferAddress;                                   //!< DW4..5, Reconstructed Pixels Frame Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              ReconstructedPixelsFrameBufferAddressAttributes;                         //!< DW6, Reconstructed Pixels Frame Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        AQM_PIPE_BUF_ADDR_STATE_DW7;
        MEMORYADDRESSATTRIBUTES_CMD              AQM_PIPE_BUF_ADDR_STATE_DW9;
        SPLITBASEADDRESS64BYTEALIGNED_CMD        AQM_PIPE_BUF_ADDR_STATE_DW10;
        MEMORYADDRESSATTRIBUTES_CMD              AQM_PIPE_BUF_ADDR_STATE_DW12;
        SPLITBASEADDRESS64BYTEALIGNED_CMD        AQM_PIPE_BUF_ADDR_STATE_DW13;
        MEMORYADDRESSATTRIBUTES_CMD              AQM_PIPE_BUF_ADDR_STATE_DW15;
        SPLITBASEADDRESS64BYTEALIGNED_CMD        AQM_PIPE_BUF_ADDR_STATE_DW16;
        MEMORYADDRESSATTRIBUTES_CMD              AQM_PIPE_BUF_ADDR_STATE_DW18;
        SPLITBASEADDRESS64BYTEALIGNED_CMD        AQM_PIPE_BUF_ADDR_STATE_DW19;
        MEMORYADDRESSATTRIBUTES_CMD              AQM_PIPE_BUF_ADDR_STATE_DW21;
        SPLITBASEADDRESS64BYTEALIGNED_CMD        AQM_PIPE_BUF_ADDR_STATE_DW22;
        MEMORYADDRESSATTRIBUTES_CMD              AQM_PIPE_BUF_ADDR_STATE_DW24;
        SPLITBASEADDRESS64BYTEALIGNED_CMD        AQM_PIPE_BUF_ADDR_STATE_DW25;
        MEMORYADDRESSATTRIBUTES_CMD              AQM_PIPE_BUF_ADDR_STATE_DW27;
        SPLITBASEADDRESS64BYTEALIGNED_CMD        AQM_PIPE_BUF_ADDR_STATE_DW28;
        MEMORYADDRESSATTRIBUTES_CMD              AQM_PIPE_BUF_ADDR_STATE_DW30;
        SPLITBASEADDRESS64BYTEALIGNED_CMD        AQM_PIPE_BUF_ADDR_STATE_DW31;
        MEMORYADDRESSATTRIBUTES_CMD              AQM_PIPE_BUF_ADDR_STATE_DW33;
        SPLITBASEADDRESS64BYTEALIGNED_CMD        AQM_PIPE_BUF_ADDR_STATE_DW34;
        MEMORYADDRESSATTRIBUTES_CMD              AQM_PIPE_BUF_ADDR_STATE_DW36;
        SPLITBASEADDRESS64BYTEALIGNED_CMD        AQM_PIPE_BUF_ADDR_STATE_DW37;
        MEMORYADDRESSATTRIBUTES_CMD              AQM_PIPE_BUF_ADDR_STATE_DW39;
        SPLITBASEADDRESS64BYTEALIGNED_CMD        AQM_PIPE_BUF_ADDR_STATE_DW40;
        MEMORYADDRESSATTRIBUTES_CMD              AQM_PIPE_BUF_ADDR_STATE_DW42;
        SPLITBASEADDRESS64BYTEALIGNED_CMD        AQM_PIPE_BUF_ADDR_STATE_DW43;
        MEMORYADDRESSATTRIBUTES_CMD              AQM_PIPE_BUF_ADDR_STATE_DW45;
        SPLITBASEADDRESS64BYTEALIGNED_CMD        AQM_PIPE_BUF_ADDR_STATE_DW46;
        MEMORYADDRESSATTRIBUTES_CMD              AQM_PIPE_BUF_ADDR_STATE_DW48;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_AQMPIPEBUFADDRSTATE                    = 2, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = AQM = 5h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEAQM                      = 5, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        AQM_PIPE_BUF_ADDR_STATE_CMD()
        {
            DW0.Value = 0x7282002f;
            //DW0.DwordLength                                  = GetOpLength(dwSize);
            //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AQMPIPEBUFADDRSTATE;
            //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEAQM;
            //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
            //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;
        }

        static const size_t dwSize = 49;
        static const size_t byteSize = 196;
    };

    //!
    //! \brief AQM_SURFACE_STATE
    //! \details
    //!     The AQM Pipeline is selected with the Media Instruction Opcode "8h" for
    //!     all AVP Commands. Each AVP command has assigned a media instruction
    //!     command as defined in DWord 0, BitField 22:16.
    //!     
    //!     The AQM_SURFACE_STATE command is responsible for defining the frame
    //!     buffer pitch and the offset of the chroma component.This is a picture
    //!     level state command and is shared by both encoding and decoding
    //!     processes.For Decoder, this command is issued once per surface type.
    //!     There is one reconstructed surface, 8 reference pictures surfaces and
    //!     one optional IntraBC Decoded Surface (only if IBC is ON).For Encoder,
    //!     this command is issued once per surface type. There are4 surface types
    //!     :source down scaled, source original, reference and reconstructed
    //!     picture. All reference frames are defined with the same surface
    //!     command.Tile-Yf and Tile-Ys are not supported in Gen12, but HW interface
    //!     still need to keep these bits as reserved bits.Note : When NV12 and Tile
    //!     Y are being used, full pitch and interleaved UV is always in use. U and
    //!     V Xoffset must be set to 0; U and V Yoffset must be 8-pixel aligned. For
    //!     10-bit pixel, P010 surface definition is being used.
    //!     
    struct AQM_SURFACE_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 22)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(23, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 SurfacePitchMinus1                               : __CODEGEN_BITFIELD( 0, 16)    ; //!< Surface Pitch Minus1
                uint32_t                 Reserved49                                       : __CODEGEN_BITFIELD(17, 27)    ; //!< Reserved
                uint32_t                 SurfaceId                                        : __CODEGEN_BITFIELD(28, 31)    ; //!< SURFACE_ID
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 YOffsetForUCbInPixel                             : __CODEGEN_BITFIELD( 0, 14)    ; //!< Y Offset for U(Cb) in pixel
                uint32_t                 Reserved79                                       : __CODEGEN_BITFIELD(15, 24)    ; //!< Reserved
                uint32_t                 VariantFormatLsbPackedEnable                     : __CODEGEN_BITFIELD(25, 25)    ; //!< VARIANT_FORMAT_LSB_PACKED_ENABLE
                uint32_t                 Reserved90                                       : __CODEGEN_BITFIELD(26, 26)    ; //!< Reserved
                uint32_t                 SurfaceFormat                                    : __CODEGEN_BITFIELD(27, 31)    ; //!< SURFACE_FORMAT
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 DefaultAlphaValue                                : __CODEGEN_BITFIELD( 0, 15)    ; //!< Default Alpha Value
                uint32_t                 YOffsetForVCr                                    : __CODEGEN_BITFIELD(16, 31)    ; //!< Y Offset for V(Cr)
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 Reserved128                                      : __CODEGEN_BITFIELD( 0, 13)    ; //!< Reserved
                uint32_t                 CompressionTypeForSourceFrame                    : __CODEGEN_BITFIELD(14, 14)    ; //!< COMPRESSION_TYPE_FOR_SOURCE_FRAME
                uint32_t                 CompressionTypeForReconstructedFrame             : __CODEGEN_BITFIELD(15, 15)    ; //!< COMPRESSION_TYPE_FOR_RECONSTRUCTED_FRAME
                uint32_t                 CompressionFormat                                : __CODEGEN_BITFIELD(16, 20)    ; //!< COMPRESSION_FORMAT
                uint32_t                 Reserved149                                      : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_SURFACESTATE                           = 1, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = AQM = 5h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEAQM                      = 5, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum SURFACE_ID
        {
            SURFACE_ID_RECONSTRUCTEDPICTURE                                  = 0, //!< This is for the reconstructed picture surface state
            SURFACE_ID_SOURCEINPUTPICTURE                                    = 1, //!< No additional details
        };

        //! \brief VARIANT_FORMAT_LSB_PACKED_ENABLE
        //! \details
        //!     This bit indicates if the LSB portion of the variant format is packed
        //!     together or byte-aligned with 0 to lower portion part of the byte.  
        //!        This is only valid forP010Variant/P016Variant
        //!     andY210Variant/Y216Variant (444 Variant is not supported currently).
        //!     This bit must be programmed to 0 for all other format.
        enum VARIANT_FORMAT_LSB_PACKED_ENABLE
        {
            VARIANT_FORMAT_LSB_PACKED_ENABLE_LSBUNPACKED                     = 0, //!< Indicates LSB portion of the Variant format is byte-aligned per pixel by adding "0" to the lower part of the byte
            VARIANT_FORMAT_LSB_PACKED_ENABLE_LSBPACKED                       = 1, //!< Indicates LSB portion of the Variant format is packed together (multiple LSB pixels are packed together to form a byte). The number of LSB pixels can be packed together depends on the bitdepth of the pixels.
        };

        //! \brief SURFACE_FORMAT
        //! \details
        //!     Specifies the format of the surface.
        enum SURFACE_FORMAT
        {
            SURFACE_FORMAT_P010VARIANT                                       = 3, //!< P010Variant is a modified P010 format,  >8 bit planar 420 with MSB together and LSB at an offset in x direction where the x-offset should be 32-bit aligned.
            SURFACE_FORMAT_PLANAR4208                                        = 4, //!< No additional details
            SURFACE_FORMAT_P010                                              = 13, //!< No additional details
        };

        //! \brief COMPRESSION_TYPE_FOR_SOURCE_FRAME
        //! \details
        //!     This bit is for Source Frame. Valid only when Memory Compression for
        //!     Source Frame is enabled.
        enum COMPRESSION_TYPE_FOR_SOURCE_FRAME
        {
            COMPRESSION_TYPE_FOR_SOURCE_FRAME_MEDIACOMPRESSIONENABLED        = 0, //!< No additional details
            COMPRESSION_TYPE_FOR_SOURCE_FRAME_RENDERCOMPRESSIONENABLED       = 1, //!< No additional details
        };

        //! \brief COMPRESSION_TYPE_FOR_RECONSTRUCTED_FRAME
        //! \details
        //!     This bit is for Reconstructed Frame. Valid only when Memory Compression
        //!     for Reconstructed Frame is enabled.
        enum COMPRESSION_TYPE_FOR_RECONSTRUCTED_FRAME
        {
            COMPRESSION_TYPE_FOR_RECONSTRUCTED_FRAME_MEDIACOMPRESSIONENABLED = 0, //!< No additional details
            COMPRESSION_TYPE_FOR_RECONSTRUCTED_FRAME_RENDERCOMPRESSIONENABLED = 1, //!< No additional details
        };

        //! \brief COMPRESSION_FORMAT
        //! \details
        //!     Specifies the Compression Format.
        enum COMPRESSION_FORMAT
        {
            COMPRESSION_FORMAT_CMFR8                                         = 0, //!< Single 8bit channel format
            COMPRESSION_FORMAT_CMFR8G8                                       = 1, //!< Two 8bit channel format
            COMPRESSION_FORMAT_CMFR8G8B8A8                                   = 2, //!< Four 8bit channel format
            COMPRESSION_FORMAT_CMFR10G10B10A2                                = 3, //!< Three 10bit channels and One 2bit channel
            COMPRESSION_FORMAT_CMFR11G11B10                                  = 4, //!< Two 11bit channels and One 10bit channel
            COMPRESSION_FORMAT_CMFR16                                        = 5, //!< Single 16bit channel format
            COMPRESSION_FORMAT_CMFR16G16                                     = 6, //!< Two 16bit channel format
            COMPRESSION_FORMAT_CMFR16G16B16A16                               = 7, //!< Four 16bit channels
            COMPRESSION_FORMAT_CMFR32                                        = 8, //!< Single 32bit channel
            COMPRESSION_FORMAT_CMFR32G32                                     = 9, //!< Two 32bit channels
            COMPRESSION_FORMAT_CMFR32G32B32A32                               = 10, //!< Four 32bit channels
            COMPRESSION_FORMAT_CMFY16U16Y16V16                               = 11, //!< Packed YUV 16/12/10 bit per channel
            COMPRESSION_FORMAT_CMFML8                                        = 15, //!< Machine Learning format / Generic data
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        AQM_SURFACE_STATE_CMD()
        {
            DW0.Value = 0x72810003;
            //DW0.DwordLength                                  = GetOpLength(dwSize);
            //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_SURFACESTATE;
            //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEAQM;
            //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
            //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

            DW1.Value = 0x00000000;
            //DW1.SurfaceId                                    = SURFACE_ID_RECONSTRUCTEDPICTURE;

            DW2.Value = 0x00000000;
            //DW2.VariantFormatLsbPackedEnable                 = VARIANT_FORMAT_LSB_PACKED_ENABLE_LSBUNPACKED;
            //DW2.SurfaceFormat                                = 0;

            DW3.Value = 0x00000000;

            DW4.Value = 0x00000000;
            //DW4.CompressionTypeForSourceFrame                = COMPRESSION_TYPE_FOR_SOURCE_FRAME_MEDIACOMPRESSIONENABLED;
            //DW4.CompressionTypeForReconstructedFrame         = COMPRESSION_TYPE_FOR_RECONSTRUCTED_FRAME_MEDIACOMPRESSIONENABLED;
            //DW4.CompressionFormat                            = COMPRESSION_FORMAT_CMFR8;
        }

        static const size_t dwSize = 5;
        static const size_t byteSize = 20;
    };

    //!
    //! \brief AQM_TILE_CODING
    //! \details
    //!     This command is used only for VDAQM. It is issued for every tile of a
    //!     frame. If a frame is composed of only 1 tile, it is still being issued.
    //!     Tiling and Tile Group organization in AV1 cannot be disabled, a frame
    //!     minimum must have 1 tile. Currently, each batch buffer can contain only
    //!     1 tile to be processed, it cannot contain more than 1 tile or the entire
    //!     tile group of tiles.  When the tile width exceeds 4096 pixels or the
    //!     tile area exceeds 4096x2304 pixels, tiling must be performed and number
    //!     of tiles in such frame must be >1. There is no mandatory tiling driven
    //!     by tile height. The frame height in pixels will limit the allowed tile
    //!     height in extreme situation. Hence, the AVP_TILE_CODING can be issued
    //!     multiple times for decoding a frame.
    //!     
    struct AQM_TILE_CODING_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 22)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(23, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            struct
            {
                uint32_t                 FrameTileId                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Frame Tile ID
                uint32_t                 Reserved44                                       : __CODEGEN_BITFIELD(12, 23)    ; //!< Reserved
                uint32_t                 TileGroupId                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Tile Group ID
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 TileColumnPositionInSbUnit                       : __CODEGEN_BITFIELD( 0,  9)    ; //!< Tile Column Position in SB Unit
                uint32_t                 Reserved74                                       : __CODEGEN_BITFIELD(10, 15)    ; //!< Reserved
                uint32_t                 TileRowPositionInSbUnit                          : __CODEGEN_BITFIELD(16, 25)    ; //!< Tile Row Position in SB Unit
                uint32_t                 Reserved90                                       : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 TileWidthInSuperblockUnitMinus1                  : __CODEGEN_BITFIELD( 0,  5)    ; //!< Tile Width in SuperBlock Unit Minus1
                uint32_t                 Reserved102                                      : __CODEGEN_BITFIELD( 6, 15)    ; //!< Reserved
                uint32_t                 TileHeightInSuperblockUnitMinus1                 : __CODEGEN_BITFIELD(16, 25)    ; //!< Tile Height in SuperBlock Unit Minus1
                uint32_t                 Reserved122                                      : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 TileNumber                                       : __CODEGEN_BITFIELD( 0,  9)    ; //!< TILE NUMBER
                uint32_t                 Reserved138                                      : __CODEGEN_BITFIELD(10, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_AVPTILECODING                          = 21, //!< No additional details
        };

        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEAQM                      = 5, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        AQM_TILE_CODING_CMD()
        {
            DW0.Value = 0x72950003;
            //DW0.DwordLength                                  = GetOpLength(dwSize);
            //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AVPTILECODING;
            //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEAQM;
            //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
            //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

            DW1.Value = 0x00000000;

            DW2.Value = 0x00000000;

            DW3.Value = 0x00000000;

            DW4.Value = 0x00000000;
        }

        static const size_t dwSize = 5;
        static const size_t byteSize = 20;
    };

    //!
    //! \brief AQM_VD_CONTROL_STATE
    //! \details
    //!     This command is used only for VDAQM. It is issued as the first command
    //!     to resetVDAQM pipe and the last command to do implicit flush in offline
    //!     mode of command sequece.
    //!     
    struct AQM_VD_CONTROL_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 22)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(23, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        VD_CONTROL_STATE_BODY_CMD                VdControlStateBody;                                                      //!< DW1..2, VD Control State Body

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_AQMVDCONTROL                           = 5, //!< No additional details
        };

        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEAQM                      = 5, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        AQM_VD_CONTROL_STATE_CMD()
        {
            DW0.Value = 0x72850001;
            //DW0.DwordLength                                  = GetOpLength(dwSize);
            //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AQMFRAMESTART;
            //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEAQM;
            //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
            //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;
        }

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief AQM_SLICE_STATE
    //! \details
    //!     This command is used only for VDAQM. It is issued as the first command
    //!     to resetVDAQM pipe and the last command to do implicit flush in offline
    //!     mode of command sequence.
    //!     
    struct AQM_SLICE_STATE_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< DWORD_LENGTH
                uint32_t                 Reserved12                                       : __CODEGEN_BITFIELD(12, 15)    ; //!< Reserved
                uint32_t                 MediaInstructionCommand                          : __CODEGEN_BITFIELD(16, 22)    ; //!< MEDIA_INSTRUCTION_COMMAND
                uint32_t                 MediaInstructionOpcode                           : __CODEGEN_BITFIELD(23, 26)    ; //!< MEDIA_INSTRUCTION_OPCODE
                uint32_t                 PipelineType                                     : __CODEGEN_BITFIELD(27, 28)    ; //!< PIPELINE_TYPE
                uint32_t                 CommandType                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< COMMAND_TYPE
            };
            uint32_t                     Value;
        } DW0;
        union
        {
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
            struct
            {
                uint32_t                 NextsliceMbStartYPosition                        : __CODEGEN_BITFIELD( 0,  9)    ; //!< NextSlice MB Start Y Position
                uint32_t                 Reserved74                                       : __CODEGEN_BITFIELD(10, 15)    ; //!< Reserved
                uint32_t                 NextsliceMbLcuStartXPosition                     : __CODEGEN_BITFIELD(16, 25)    ; //!< NextSlice MB/LCU Start X Position
                uint32_t                 Reserved90                                       : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_AQMSLICESTATE                          = 8, //!< No additional details
        };

        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEAQM                      = 5, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        AQM_SLICE_STATE_CMD()
        {
            DW0.Value                                        = 0x72880001;
            //DW0.DwordLength                                  = GetOpLength(dwSize);
            //DW0.MediaInstructionCommand                      = MEDIA_INSTRUCTION_COMMAND_AQMSLICESTATE;
            //DW0.MediaInstructionOpcode                       = MEDIA_INSTRUCTION_OPCODE_CODECENGINENAMEAQM;
            //DW0.PipelineType                                 = PIPELINE_TYPE_UNNAMED2;
            //DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

            DW1.Value                                        = 0x00000000;

            DW2.Value                                        = 0x00000000;

        }

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

MEDIA_CLASS_DEFINE_END(mhw__vdbox__aqm__xe2_lpm__Cmd)
};
}  // namespace xe2_lpm
}  // namespace aqm
}  // namespace vdbox
}  // namespace mhw

#pragma pack()

#endif  // __MHW_VDBOX_AQM_HWCMD_XE2_LPM_H__
