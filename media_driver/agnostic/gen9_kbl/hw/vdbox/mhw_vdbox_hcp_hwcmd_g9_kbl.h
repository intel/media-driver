/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     mhw_vdbox_hcp_hwcmd_g9_kbl.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of g9_kbl as other components
//!           should use MHW interface to interact with MHW commands and states.
//!

// DO NOT EDIT

#ifndef __MHW_VDBOX_HCP_HWCMD_G9_KBL_H__
#define __MHW_VDBOX_HCP_HWCMD_G9_KBL_H__

#pragma once
#pragma pack(1)

#include <cstdint>
#include <cstddef>

class mhw_vdbox_hcp_g9_kbl
{
public:
    // Internal Macros
    #define __CODEGEN_MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
    #define __CODEGEN_BITFIELD(l, h) (h) - (l) + 1
    #define __CODEGEN_OP_LENGTH_BIAS 2
    #define __CODEGEN_OP_LENGTH(x) (uint32_t)((__CODEGEN_MAX(x, __CODEGEN_OP_LENGTH_BIAS)) - __CODEGEN_OP_LENGTH_BIAS)

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
                uint32_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 BaseAddressIndexToMemoryObjectControlStateMocsTables : __CODEGEN_BITFIELD( 1,  6)    ; //!< Base Address - Index to Memory Object Control State (MOCS) Tables
                uint32_t                 BaseAddressArbitrationPriorityControl            : __CODEGEN_BITFIELD( 7,  8)    ; //!< Base Address - Arbitration Priority Control
                uint32_t                 BaseAddressMemoryCompressionEnable               : __CODEGEN_BITFIELD( 9,  9)    ; //!< Base Address - Memory Compression Enable
                uint32_t                 BaseAddressMemoryCompressionMode                 : __CODEGEN_BITFIELD(10, 10)    ; //!< BASE_ADDRESS_MEMORY_COMPRESSION_MODE
                uint32_t                 Reserved11                                       : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 BaseAddressRowStoreScratchBufferCacheSelect      : __CODEGEN_BITFIELD(12, 12)    ; //!< BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 BaseAddressTiledResourceMode                     : __CODEGEN_BITFIELD(13, 14)    ; //!< BASE_ADDRESS_TILED_RESOURCE_MODE
                uint32_t                 Reserved15                                       : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \brief BASE_ADDRESS_MEMORY_COMPRESSION_MODE
        //! \details
        //!     Distinguishes vertical from horizontal compression. Please refer to
        //!     vol1a <b>Memory Data Formats chapter - section</b> media Memory
        //!     Compression for more details.
        enum BASE_ADDRESS_MEMORY_COMPRESSION_MODE
        {
            BASE_ADDRESS_MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE   = 0, //!< No additional details
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

        //! \brief BASE_ADDRESS_TILED_RESOURCE_MODE
        //! \details
        //!     <b>For Media Surfaces:</b> This field specifies the tiled resource mode.
        enum BASE_ADDRESS_TILED_RESOURCE_MODE
        {
            BASE_ADDRESS_TILED_RESOURCE_MODE_TRMODENONE                      = 0, //!< TileY resources
            BASE_ADDRESS_TILED_RESOURCE_MODE_TRMODETILEYF                    = 1, //!< 4KB tiled resources
            BASE_ADDRESS_TILED_RESOURCE_MODE_TRMODETILEYS                    = 2, //!< 64KB tiled resources
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        MEMORYADDRESSATTRIBUTES_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief SPLITBASEADDRESS64BYTEALIGNED
    //! \details
    //!     Specifies a 64-bit (48-bit canonical) 64-byte aligned memory base
    //!     address.
    //!     
    struct SPLITBASEADDRESS64BYTEALIGNED_CMD
    {
        union
        {
            struct
            {
                uint64_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint64_t                 Graphicsaddress476                               : __CODEGEN_BITFIELD( 6, 47)    ; //!< GraphicsAddress47-6
                uint64_t                 Reserved48                                       : __CODEGEN_BITFIELD(48, 63)    ; //!< Reserved
            };
            uint32_t                     Value[2];
        } DW0_1;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        SPLITBASEADDRESS64BYTEALIGNED_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief SPLITBASEADDRESS4KBYTEALIGNED
    //! \details
    //!     Specifies a 64-bit (48-bit canonical) 4K-byte aligned memory base
    //!     address. GraphicsAddress is a 64-bit value [63:0], but only a portion of
    //!     it is used by hardware. The upper reserved bits are ignored and MBZ.
    //!     
    struct SPLITBASEADDRESS4KBYTEALIGNED_CMD
    {
        union
        {
            struct
            {
                uint64_t                 Reserved0                                        : __CODEGEN_BITFIELD( 0, 11)    ; //!< Reserved
                uint64_t                 Graphicsaddress4712                              : __CODEGEN_BITFIELD(12, 47)    ; //!< GraphicsAddress47-12
                uint64_t                 Reserved48                                       : __CODEGEN_BITFIELD(48, 63)    ; //!< Reserved
            };
            uint32_t                     Value[2];
        } DW0_1;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        SPLITBASEADDRESS4KBYTEALIGNED_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief HCP_PIPE_MODE_SELECT
    //! \details
    //!     The HCP is selected with the Media Instruction Opcode "7h" for all HCP
    //!     Commands. Each HCP command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    //!     The workload for the HCP is based upon a single frame decode. There are
    //!     no states saved between frame decodes in the HCP. Once the bit stream
    //!     DMA is configured with the HCP_BSD_OBJECT command, and the bit stream is
    //!     presented to the HCP, the frame decode will begin. The
    //!     HCP_PIPE_MODE_SELECT command is responsible for general pipeline level
    //!     configuration that would normally be set once for a single stream encode
    //!     or decode and would not be modified on a frame workload basis. This is a
    //!     picture level state command and is shared by both encoding and decoding
    //!     processes.
    //!     
    struct HCP_PIPE_MODE_SELECT_CMD
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
                uint32_t                 CodecSelect                                      : __CODEGEN_BITFIELD( 0,  0)    ; //!< CODEC_SELECT
                uint32_t                 DeblockerStreamoutEnable                         : __CODEGEN_BITFIELD( 1,  1)    ; //!< DEBLOCKER_STREAMOUT_ENABLE
                uint32_t                 PakPipelineStreamoutEnable                       : __CODEGEN_BITFIELD( 2,  2)    ; //!< PAK_PIPELINE_STREAMOUT_ENABLE
                uint32_t                 PicStatusErrorReportEnable                       : __CODEGEN_BITFIELD( 3,  3)    ; //!< PIC_STATUSERROR_REPORT_ENABLE
                uint32_t                 Reserved36                                       : __CODEGEN_BITFIELD( 4,  4)    ; //!< Reserved
                uint32_t                 CodecStandardSelect                              : __CODEGEN_BITFIELD( 5,  7)    ; //!< CODEC_STANDARD_SELECT
                uint32_t                 Reserved40                                       : __CODEGEN_BITFIELD( 8,  8)    ; //!< Reserved
                uint32_t                 AdvancedRateControlEnable                        : __CODEGEN_BITFIELD( 9,  9)    ; //!< Advanced Rate Control Enable
                uint32_t                 Reserved42                                       : __CODEGEN_BITFIELD(10, 11)    ; //!< Reserved
                uint32_t                 PakFrameLevelStreamoutEnable                     : __CODEGEN_BITFIELD(12, 12)    ; //!< PAK Frame Level StreamOut enable
                uint32_t                 Reserved45                                       : __CODEGEN_BITFIELD(13, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 MediaSoftResetCounterPer1000Clocks                                               ; //!< MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 PicStatusErrorReportId                                                           ; //!< PIC_STATUSERROR_REPORT_ID
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 Reserved128                                    : __CODEGEN_BITFIELD(0, 5)        ; //!< Reserved
                uint32_t                 HprVp9ModeSwitchEcoDisable                     : __CODEGEN_BITFIELD(6, 6)        ; //!< HPR_VP9_MODE_SWITCH_ECO_DISABLE
                uint32_t                 Reserved135                                    : __CODEGEN_BITFIELD(7, 31)       ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 Reserved160                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HCPPIPEMODESELECT                      = 0, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HCP = 7h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 7, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum CODEC_SELECT
        {
            CODEC_SELECT_DECODE                                              = 0, //!< No additional details
            CODEC_SELECT_ENCODE                                              = 1, //!< No additional details
        };

        //! \brief DEBLOCKER_STREAMOUT_ENABLE
        //! \details
        //!     Deblocker Streamout Enable not currently supported for Encode or Decode
        enum DEBLOCKER_STREAMOUT_ENABLE
        {
            DEBLOCKER_STREAMOUT_ENABLE_DISABLE                               = 0, //!< Disable deblocker-only parameter streamout
            DEBLOCKER_STREAMOUT_ENABLE_ENABLE                                = 1, //!< Enable deblocker-only parameter streamout
        };

        //! \brief PAK_PIPELINE_STREAMOUT_ENABLE
        //! \details
        //!     Pipeline Streamout Enable is only defined for encode.  It is ignored for
        //!     decode.
        enum PAK_PIPELINE_STREAMOUT_ENABLE
        {
            PAK_PIPELINE_STREAMOUT_ENABLE_DISABLEPIPELINESTATESANDPARAMETERSSTREAMOUT = 0, //!< No additional details
            PAK_PIPELINE_STREAMOUT_ENABLE_ENABLEPIPELINESTATESANDPARAMETERSSTREAMOUT = 1, //!< No additional details
        };

        enum PIC_STATUSERROR_REPORT_ENABLE
        {
            PIC_STATUSERROR_REPORT_ENABLE_DISABLE                            = 0, //!< Disable status/error reporting
            PIC_STATUSERROR_REPORT_ENABLE_ENABLE                             = 1, //!< Status/Error reporting is written out once per picture. The Pic Status/Error Report ID in DWord3along with the status/error status bits are packed into one cache line and written to theStatus/Error Buffer address in the HCP_PIPE_BUF_ADDR_STATE command. Must be zero for encoder mode.
        };

        enum CODEC_STANDARD_SELECT
        {
            CODEC_STANDARD_SELECT_HEVC                                       = 0, //!< No additional details
            CODEC_STANDARD_SELECT_VP9                                        = 1, //!< No additional details
        };

        //! \brief MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS
        //! \details
        //!     In decoder modes, this counter value specifies the number of clocks (per
        //!     1000) of GAC inactivity
        //!                         before a media soft-reset is applied to the HCP and HuC. If counter
        //!     value is set to 0, the media
        //!                         soft-reset feature is disabled and no reset will occur.
        //!                         <p>In encoder modes, this counter must be set to 0 to disable media
        //!     soft reset. This feature is not
        //!                             supported for the encoder.</p>
        enum MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS
        {
            MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS_DISABLE                 = 0, //!< No additional details
        };

        //! \brief PIC_STATUSERROR_REPORT_ID
        //! \details
        //!     The Pic Status/Error Report ID is a unique 32-bit unsigned integer
        //!     assigned to each picture
        //!                         status/error output. Must be zero for encoder mode.
        enum PIC_STATUSERROR_REPORT_ID
        {
            PIC_STATUSERROR_REPORT_ID_32_BITUNSIGNED                         = 0, //!< Unique ID Number
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        HCP_PIPE_MODE_SELECT_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief HCP_SURFACE_STATE
    //! \details
    //!     The HCP is selected with the Media Instruction Opcode "7h" for all HCP
    //!     Commands. Each HCP command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    //!     The HCP_SURFACE_STATE command is responsible for defining the frame
    //!     buffer pitch and the offset of the chroma component. This is a picture
    //!     level state command and is shared by both encoding and decoding
    //!     processes. Note : Only NV12 and Tile Y are being supported for HEVC.
    //!     Hence full pitch and interleaved UV is always in use. U and V Xoffset
    //!     must be set to 0; U and V Yoffset must be 16-pixel aligned. This Surface
    //!     State is not the same as that of the 3D engine and of the MFX pipeline.
    //!     
    struct HCP_SURFACE_STATE_CMD
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
                uint32_t                 Reserved79                                       : __CODEGEN_BITFIELD(15, 27)    ; //!< Reserved
                uint32_t                 SurfaceFormat                                    : __CODEGEN_BITFIELD(28, 31)    ; //!< SURFACE_FORMAT
            };
            uint32_t                     Value;
        } DW2;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HCPSURFACESTATE                        = 1, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HCP = 7h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 7, //!< No additional details
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
            SURFACE_ID_HEVCFORCURRENTDECODEDPICTURE                          = 0, //!< 8-bit uncompressed data 
            SURFACE_ID_SOURCEINPUTPICTURE_ENCODER                            = 1, //!< 8-bit uncompressed data 
            SURFACE_ID_PREVREFERENCEPICTURE                                  = 2, //!< (VP9 only) Previous Reference
            SURFACE_ID_GOLDENREFERENCEPICTURE                                = 3, //!< (VP9 only) Golden Reference
            SURFACE_ID_ALTREFREFERENCEPICTURE                                = 4, //!< (VP9 only) AltRef Reference
        };

        //! \brief SURFACE_FORMAT
        //! \details
        //!     <p>Specifies the format of the surface.</p>
        enum SURFACE_FORMAT
        {
            SURFACE_FORMAT_P010VARIANT                                       = 3, //!< No additional details
            SURFACE_FORMAT_PLANAR4208                                        = 4, //!< No additional details
            SURFACE_FORMAT_P010                                              = 13, //!< No additional details
            SURFACE_FORMAT_P016                                              = 14, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        HCP_SURFACE_STATE_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief HCP_PIPE_BUF_ADDR_STATE
    //! \details
    //!     The HCP is selected with the Media Instruction Opcode "7h" for all HCP
    //!     Commands. Each HCP command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    //!     This state command provides the memory base addresses for the row store
    //!     buffer and reconstructed picture output buffers required by the HCP.
    //!     This is a picture level state command and is shared by both encoding and
    //!     decoding processes.
    //!     
    //!     All pixel surface addresses must be 4K byte aligned. There is a max of 8
    //!     Reference Picture Buffer Addresses, and all share the same third address
    //!     DW in specifying 48-bit address.
    //!     
    struct HCP_PIPE_BUF_ADDR_STATE_CMD
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
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        DecodedPicture;                                                          //!< DW1..2, Decoded Picture
        MEMORYADDRESSATTRIBUTES_CMD              DecodedPictureMemoryAddressAttributes;                                   //!< DW3, Decoded Picture Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        DeblockingFilterLineBuffer;                                              //!< DW4..5, Deblocking Filter Line Buffer
        MEMORYADDRESSATTRIBUTES_CMD              DeblockingFilterLineBufferMemoryAddressAttributes;                       //!< DW6, Deblocking Filter Line Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        DeblockingFilterTileLineBuffer;                                          //!< DW7..8, Deblocking Filter Tile Line Buffer
        MEMORYADDRESSATTRIBUTES_CMD              DeblockingFilterTileLineBufferMemoryAddressAttributes;                   //!< DW9, Deblocking Filter Tile Line Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        DeblockingFilterTileColumnBuffer;                                        //!< DW10..11, Deblocking Filter Tile Column Buffer
        MEMORYADDRESSATTRIBUTES_CMD              DeblockingFilterTileColumnBufferMemoryAddressAttributes;                 //!< DW12, Deblocking Filter Tile Column Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        MetadataLineBuffer;                                                      //!< DW13..14, Metadata Line Buffer
        MEMORYADDRESSATTRIBUTES_CMD              MetadataLineBufferMemoryAddressAttributes;                               //!< DW15, Metadata Line Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        MetadataTileLineBuffer;                                                  //!< DW16..17, Metadata Tile Line Buffer
        MEMORYADDRESSATTRIBUTES_CMD              MetadataTileLineBufferMemoryAddressAttributes;                           //!< DW18, Metadata Tile Line Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        MetadataTileColumnBuffer;                                                //!< DW19..20, Metadata Tile Column Buffer
        MEMORYADDRESSATTRIBUTES_CMD              MetadataTileColumnBufferMemoryAddressAttributes;                         //!< DW21, Metadata Tile Column Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        SaoLineBuffer;                                                           //!< DW22..23, SAO Line Buffer
        MEMORYADDRESSATTRIBUTES_CMD              SaoLineBufferMemoryAddressAttributes;                                    //!< DW24, SAO Line Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        SaoTileLineBuffer;                                                       //!< DW25..26, SAO Tile Line Buffer
        MEMORYADDRESSATTRIBUTES_CMD              SaoTileLineBufferMemoryAddressAttributes;                                //!< DW27, SAO Tile Line Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        SaoTileColumnBuffer;                                                     //!< DW28..29, SAO Tile Column Buffer
        MEMORYADDRESSATTRIBUTES_CMD              SaoTileColumnBufferMemoryAddressAttributes;                              //!< DW30, SAO Tile Column Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        CurrentMotionVectorTemporalBuffer;                                       //!< DW31..32, Current Motion Vector Temporal Buffer
        MEMORYADDRESSATTRIBUTES_CMD              CurrentMotionVectorTemporalBufferMemoryAddressAttributes;                //!< DW33, Current Motion Vector Temporal Buffer Memory Address Attributes
        union
        {
            struct
            {
                uint64_t                 Reserved1088                                                                     ; //!< Reserved
            };
            uint32_t                     Value[2];
        } DW34_35;
        union
        {
            struct
            {
                uint32_t                 Reserved1152                                                                     ; //!< Reserved
            };
            uint32_t                     Value;
        } DW36;
        SPLITBASEADDRESS64BYTEALIGNED_CMD        ReferencePictureBaseAddressRefaddr07[8];                                 //!< DW37..52, Reference Picture Base Address (RefAddr[0-7])
        MEMORYADDRESSATTRIBUTES_CMD              ReferencePictureBaseAddressMemoryAddressAttributes;                      //!< DW53, Reference Picture Base Address Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        OriginalUncompressedPictureSource;                                       //!< DW54..55, Original Uncompressed Picture Source
        MEMORYADDRESSATTRIBUTES_CMD              OriginalUncompressedPictureSourceMemoryAddressAttributes;                //!< DW56, Original Uncompressed Picture Source Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        StreamoutDataDestination;                                                //!< DW57..58, Streamout Data Destination
        MEMORYADDRESSATTRIBUTES_CMD              StreamoutDataDestinationMemoryAddressAttributes;                         //!< DW59, Streamout Data Destination Memory Address Attributes, Decoder Only
        SPLITBASEADDRESS64BYTEALIGNED_CMD        DecodedPictureStatusErrorBufferBaseAddressOrEncodedSliceSizeStreamoutBaseAddress;//!< DW60..61, Decoded Picture Status/Error Buffer Base Address or Encoded slice size streamout  Base Address
        MEMORYADDRESSATTRIBUTES_CMD              DecodedPictureStatusErrorBufferBaseAddressMemoryAddressAttributes;       //!< DW62, Decoded Picture Status/Error Buffer Base Address Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        LcuIldbStreamoutBuffer;                                                  //!< DW63..64, LCU ILDB Streamout Buffer
        MEMORYADDRESSATTRIBUTES_CMD              LcuIldbStreamoutBufferMemoryAddressAttributes;                           //!< DW65, LCU ILDB Streamout Buffer Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        CollocatedMotionVectorTemporalBuffer07[8];                               //!< DW66..81, Collocated Motion Vector Temporal Buffer[0-7]
        MEMORYADDRESSATTRIBUTES_CMD              CollocatedMotionVectorTemporalBuffer07MemoryAddressAttributes;           //!< DW82, Collocated Motion Vector Temporal Buffer[0-7] Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        Vp9ProbabilityBufferReadWrite;                                           //!< DW83..84, VP9 Probability Buffer Read/Write
        MEMORYADDRESSATTRIBUTES_CMD              Vp9ProbabilityBufferReadWriteMemoryAddressAttributes;                    //!< DW85, VP9 Probability Buffer Read/Write Memory Address Attributes
        union
        {
            struct
            {
                uint64_t                 Vp9SegmentIdBufferReadWrite                                                      ; //!< VP9 Segment ID Buffer Read/Write
            };
            uint32_t                     Value[2];
        } DW86_87;
        MEMORYADDRESSATTRIBUTES_CMD              Vp9SegmentIdBufferReadWriteMemoryAddressAttributes;                      //!< DW88, VP9 Segment ID buffer Read/Write Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        Vp9HvdLineRowstoreBufferReadWrite;                                       //!< DW89..90, VP9 HVD Line Rowstore Buffer Read/Write
        MEMORYADDRESSATTRIBUTES_CMD              Vp9HvdLineRowstoreBufferReadWriteMemoryAddressAttributes;                //!< DW91, VP9 HVD Line Rowstore buffer Read/Write Memory Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        Vp9HvdTileRowstoreBufferReadWrite;                                       //!< DW92..93, VP9 HVD Tile Rowstore Buffer Read/Write
        MEMORYADDRESSATTRIBUTES_CMD              Vp9HvdTileRowstoreBufferReadWriteMemoryAddressAttributes;                //!< DW94, VP9 HVD Tile Rowstore buffer Read/Write Memory Address Attributes
        uint32_t                                 Reserved3040[9];                                                         //!< DWORD95..103

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HCPPIPEBUFADDRSTATE                    = 2, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HCP = 7h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 7, //!< No additional details
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
        HCP_PIPE_BUF_ADDR_STATE_CMD();

        static const size_t dwSize = 104;
        static const size_t byteSize = 416;
    };

    //!
    //! \brief HCP_IND_OBJ_BASE_ADDR_STATE
    //! \details
    //!     The HCP is selected with the Media Instruction Opcode "7h" for all HCP
    //!     Commands. Each HCP command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    //!     The HCP_IND_OBJ_BASE_ADDR_STATE command is used to define the indirect
    //!     object base address of the stream in graphics memory. This is a frame
    //!     level command. (Is it frame or picture level?)
    //!     This is a picture level state command and is issued in both encoding and
    //!     decoding processes.
    //!     
    //!     Compressed Header Format 
    //!     
    //!     
    //!     
    struct HCP_IND_OBJ_BASE_ADDR_STATE_CMD
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
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        HcpIndirectBitstreamObjectBaseAddress;                                   //!< DW1..2, HCP Indirect Bitstream Object Base Address
        MEMORYADDRESSATTRIBUTES_CMD              HcpIndirectBitstreamObjectMemoryAddressAttributes;                       //!< DW3, HCP Indirect Bitstream Object Memory Address Attributes
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        HcpIndirectBitstreamObjectAccessUpperBound;                              //!< DW4..5, HCP Indirect Bitstream Object Access Upper Bound
        union
        {
            struct
            {
                uint64_t                 HcpIndirectCuObjectBaseAddress                                                   ; //!< HCP Indirect CU Object Base Address
            };
            uint32_t                     Value[2];
        } DW6_7;
        MEMORYADDRESSATTRIBUTES_CMD              HcpIndirectCuObjectObjectMemoryAddressAttributes;                        //!< DW8, HCP Indirect CU Object Object Memory Address Attributes
        union
        {
            struct
            {
                uint64_t                 HcpPakBseObjectBaseAddress                                                       ; //!< HCP PAK-BSE Object Base Address
            };
            uint32_t                     Value[2];
        } DW9_10;
        MEMORYADDRESSATTRIBUTES_CMD              HcpPakBseObjectAddressMemoryAddressAttributes;                           //!< DW11, HCP PAK-BSE Object Address Memory Address Attributes
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        HcpPakBseObjectAccessUpperBound;                                         //!< DW12..13, HCP PAK-BSE Object Access Upper Bound

	        union
        {
            struct
            {
                uint64_t                 HcpVp9PakCompressedHeaderSyntaxStreaminBaseAddress                                 ; //!< HCP VP9 PAK Compressed Header Syntax Streamin- Base Address
            };
            uint32_t                     Value[2];
        } DW14_15;
        MEMORYADDRESSATTRIBUTES_CMD              HcpVp9PakCompressedHeaderSyntaxStreaminMemoryAddressAttributes;          //!< DW16, HCP VP9 PAK Compressed Header Syntax StreamIn Memory Address Attributes
        union
        {
            struct
            {
                uint64_t                 HcpVp9PakProbabilityCounterStreamoutBaseAddress                                  ; //!< HCP VP9 PAK Probability Counter StreamOut- Base Address
            };
            uint32_t                     Value[2];
        } DW17_18;
        MEMORYADDRESSATTRIBUTES_CMD              HcpVp9PakProbabilityCounterStreamoutMemoryAddressAttributes;             //!< DW19, HCP VP9 PAK Probability Counter StreamOut Memory Address Attributes
        union
        {
            struct
            {
                uint64_t                 HcpVp9PakProbabilityDeltasStreaminBaseAddress                                    ; //!< HCP VP9 PAK Probability Deltas StreamIn- Base Address
            };
            uint32_t                     Value[2];
        } DW20_21;
        MEMORYADDRESSATTRIBUTES_CMD              HcpVp9PakProbabilityDeltasStreaminMemoryAddressAttributes;               //!< DW22, HCP VP9 PAK Probability Deltas StreamIn Memory Address Attributes
        union
        {
            struct
            {
                uint64_t                 HcpVp9PakTileRecordStreamoutBaseAddress                                          ; //!< HCP VP9 PAK Tile Record StreamOut- Base Address
            };
            uint32_t                     Value[2];
        } DW23_24;
        MEMORYADDRESSATTRIBUTES_CMD              HcpVp9PakTileRecordStreamoutMemoryAddressAttributes;                     //!< DW25, HCP VP9 PAK Tile Record StreamOut Memory Address Attributes
        union
        {
            struct
            {
                uint64_t                 HcpVp9PakCuLevelStatisticStreamoutBaseAddress                                    ; //!< HCP VP9 PAK CU Level Statistic StreamOut- Base Address
            };
            uint32_t                     Value[2];
        } DW26_27;
        MEMORYADDRESSATTRIBUTES_CMD              HcpVp9PakCuLevelStatisticStreamoutMemoryAddressAttributes;               //!< DW28, HCP VP9 PAK CU Level Statistic StreamOut Memory Address Attributes
        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HCPINDOBJBASEADDRSTATE                 = 3, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HCP = 7h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 7, //!< No additional details
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
        HCP_IND_OBJ_BASE_ADDR_STATE_CMD();

        static const size_t dwSize = 29;
        static const size_t byteSize = 116;
    };

    //!
    //! \brief HCP_QM_STATE
    //! \details
    //!     The HCP is selected with the Media Instruction Opcode "7h" for all HCP
    //!     Commands. Each HCP command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    //!     The HCP_QM_STATE command loads the custom HEVC quantization tables into
    //!     local RAM and may be issued up to 20 times: 3x Colour Component plus 2x
    //!     intra/inter plus 4x SizeID minus 4 for the 32x32 chroma components. When
    //!     the scaling_list_enable_flag is set to disable, the scaling matrix is
    //!     still sent to the decoder, and with all entries programmed to the same
    //!     value = 16. This is a picture level state command and is issued in both
    //!     encoding and decoding processes.
    //!     
    //!     Dwords 2-17 form a table for the DCT coefficients, 4 8-bit
    //!     coefficients/DWord.  Size 4x4 for SizeID0, DWords 2-5.
    //!      Size 8x8 for SizeID1/2/3, DWords 2-17.
    //!      
    //!     
    //!     SizeID 0 (Table 4-10) 
    //!     
    struct HCP_QM_STATE_CMD
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
                uint32_t                 PredictionType                                   : __CODEGEN_BITFIELD( 0,  0)    ; //!< PREDICTION_TYPE
                uint32_t                 Sizeid                                           : __CODEGEN_BITFIELD( 1,  2)    ; //!< SIZEID
                uint32_t                 ColorComponent                                   : __CODEGEN_BITFIELD( 3,  4)    ; //!< COLOR_COMPONENT
                uint32_t                 DcCoefficient                                    : __CODEGEN_BITFIELD( 5, 12)    ; //!< DC Coefficient
                uint32_t                 Reserved45                                       : __CODEGEN_BITFIELD(13, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        uint32_t                                 Quantizermatrix[16];                                                     //!< QuantizerMatrix

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HCPQMSTATE                             = 4, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HCP = 7h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 7, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum PREDICTION_TYPE
        {
            PREDICTION_TYPE_INTRA                                            = 0, //!< No additional details
            PREDICTION_TYPE_INTER                                            = 1, //!< No additional details
        };

        enum SIZEID
        {
            SIZEID_4X4                                                       = 0, //!< No additional details
            SIZEID_8X8                                                       = 1, //!< No additional details
            SIZEID_16X16                                                     = 2, //!< No additional details
            SIZEID_32X32                                                     = 3, //!< (Illegal Value for Colour Component Chroma Cr and Cb.)
        };

        //! \brief COLOR_COMPONENT
        //! \details
        //!     Encoder: When RDOQ is enabled, scaling list for all 3 color components
        //!     must be same. So this field is set to always 0.
        enum COLOR_COMPONENT
        {
            COLOR_COMPONENT_LUMA                                             = 0, //!< No additional details
            COLOR_COMPONENT_CHROMACB                                         = 1, //!< No additional details
            COLOR_COMPONENT_CHROMACR                                         = 2, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        HCP_QM_STATE_CMD();

        static const size_t dwSize = 18;
        static const size_t byteSize = 72;
    };

    //!
    //! \brief HCP_PIC_STATE
    //! \details
    //!     The HCP is selected with the Media Instruction Opcode "7h" for all HCP
    //!     Commands. Each HCP command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    //!     This is a picture level command and is issued only once per workload for
    //!     both encoding and decoding processes.
    //!     
    struct HCP_PIC_STATE_CMD
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
                uint32_t                 Framewidthinmincbminus1                          : __CODEGEN_BITFIELD( 0,  9)    ; //!< FrameWidthInMinCbMinus1
                uint32_t                 Reserved42                                       : __CODEGEN_BITFIELD(10, 14)    ; //!< Reserved
                uint32_t                 PakTransformSkipEnable                           : __CODEGEN_BITFIELD(15, 15)    ; //!< PAK Transform Skip Enable
                uint32_t                 Frameheightinmincbminus1                         : __CODEGEN_BITFIELD(16, 25)    ; //!< FrameHeightInMinCbMinus1
                uint32_t                 Reserved58                                       : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 Mincusize                                        : __CODEGEN_BITFIELD( 0,  1)    ; //!< MINCUSIZE
                uint32_t                 CtbsizeLcusize                                   : __CODEGEN_BITFIELD( 2,  3)    ; //!< CTBSIZE_LCUSIZE
                uint32_t                 Mintusize                                        : __CODEGEN_BITFIELD( 4,  5)    ; //!< MINTUSIZE
                uint32_t                 Maxtusize                                        : __CODEGEN_BITFIELD( 6,  7)    ; //!< MAXTUSIZE
                uint32_t                 Minpcmsize                                       : __CODEGEN_BITFIELD( 8,  9)    ; //!< MINPCMSIZE
                uint32_t                 Maxpcmsize                                       : __CODEGEN_BITFIELD(10, 11)    ; //!< MAXPCMSIZE
                uint32_t                 Reserved76                                       : __CODEGEN_BITFIELD(12, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 Colpicisi                                        : __CODEGEN_BITFIELD( 0,  0)    ; //!< COLPICISI
                uint32_t                 Curpicisi                                        : __CODEGEN_BITFIELD( 1,  1)    ; //!< CURPICISI
                uint32_t                 Inserttestflag                                   : __CODEGEN_BITFIELD( 2,  2)    ; //!< INSERTTESTFLAG
                uint32_t                 Reserved99                                       : __CODEGEN_BITFIELD( 3, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 Reserved128                                      : __CODEGEN_BITFIELD( 0,  2)    ; //!< Reserved
                uint32_t                 SampleAdaptiveOffsetEnabledFlag                  : __CODEGEN_BITFIELD( 3,  3)    ; //!< sample_adaptive_offset_enabled_flag
                uint32_t                 PcmEnabledFlag                                   : __CODEGEN_BITFIELD( 4,  4)    ; //!< pcm_enabled_flag
                uint32_t                 CuQpDeltaEnabledFlag                             : __CODEGEN_BITFIELD( 5,  5)    ; //!< CU_QP_DELTA_ENABLED_FLAG
                uint32_t                 DiffCuQpDeltaDepthOrNamedAsMaxDqpDepth           : __CODEGEN_BITFIELD( 6,  7)    ; //!< diff_cu_qp_delta_depth (or named as max_dqp_depth)
                uint32_t                 PcmLoopFilterDisableFlag                         : __CODEGEN_BITFIELD( 8,  8)    ; //!< pcm_loop_filter_disable_flag
                uint32_t                 ConstrainedIntraPredFlag                         : __CODEGEN_BITFIELD( 9,  9)    ; //!< constrained_intra_pred_flag
                uint32_t                 Log2ParallelMergeLevelMinus2                     : __CODEGEN_BITFIELD(10, 12)    ; //!< log2_parallel_merge_level_minus2
                uint32_t                 SignDataHidingFlag                               : __CODEGEN_BITFIELD(13, 13)    ; //!< SIGN_DATA_HIDING_FLAG
                uint32_t                 Reserved142                                      : __CODEGEN_BITFIELD(14, 14)    ; //!< Reserved
                uint32_t                 LoopFilterAcrossTilesEnabledFlag                 : __CODEGEN_BITFIELD(15, 15)    ; //!< loop_filter_across_tiles_enabled_flag
                uint32_t                 EntropyCodingSyncEnabledFlag                     : __CODEGEN_BITFIELD(16, 16)    ; //!< entropy_coding_sync_enabled_flag
                uint32_t                 TilesEnabledFlag                                 : __CODEGEN_BITFIELD(17, 17)    ; //!< tiles_enabled_flag
                uint32_t                 WeightedBipredFlag                               : __CODEGEN_BITFIELD(18, 18)    ; //!< weighted_bipred_flag
                uint32_t                 WeightedPredFlag                                 : __CODEGEN_BITFIELD(19, 19)    ; //!< weighted_pred_flag
                uint32_t                 Fieldpic                                         : __CODEGEN_BITFIELD(20, 20)    ; //!< FIELDPIC
                uint32_t                 Bottomfield                                      : __CODEGEN_BITFIELD(21, 21)    ; //!< BOTTOMFIELD
                uint32_t                 TransformSkipEnabledFlag                         : __CODEGEN_BITFIELD(22, 22)    ; //!< TRANSFORM_SKIP_ENABLED_FLAG
                uint32_t                 AmpEnabledFlag                                   : __CODEGEN_BITFIELD(23, 23)    ; //!< AMP_ENABLED_FLAG
                uint32_t                 Reserved152                                      : __CODEGEN_BITFIELD(24, 24)    ; //!< Reserved
                uint32_t                 TransquantBypassEnableFlag                       : __CODEGEN_BITFIELD(25, 25)    ; //!< TRANSQUANT_BYPASS_ENABLE_FLAG
                uint32_t                 StrongIntraSmoothingEnableFlag                   : __CODEGEN_BITFIELD(26, 26)    ; //!< strong_intra_smoothing_enable_flag
                uint32_t                 CuPacketStructure                                : __CODEGEN_BITFIELD(27, 27)    ; //!< CU packet structure
                uint32_t                 Reserved156                                      : __CODEGEN_BITFIELD(28, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 PicCbQpOffset                                    : __CODEGEN_BITFIELD( 0,  4)    ; //!< pic_cb_qp_offset
                uint32_t                 PicCrQpOffset                                    : __CODEGEN_BITFIELD( 5,  9)    ; //!< pic_cr_qp_offset
                uint32_t                 MaxTransformHierarchyDepthIntraOrNamedAsTuMaxDepthIntra : __CODEGEN_BITFIELD(10, 12)    ; //!< max_transform_hierarchy_depth_intra (or named as tu_max_depth_intra)
                uint32_t                 MaxTransformHierarchyDepthInterOrNamedAsTuMaxDepthInter : __CODEGEN_BITFIELD(13, 15)    ; //!< max_transform_hierarchy_depth_inter(or named as tu_max_depth_inter)
                uint32_t                 PcmSampleBitDepthChromaMinus1                    : __CODEGEN_BITFIELD(16, 19)    ; //!< pcm_sample_bit_depth_chroma_minus1
                uint32_t                 PcmSampleBitDepthLumaMinus1                      : __CODEGEN_BITFIELD(20, 23)    ; //!< pcm_sample_bit_depth_luma_minus1
                uint32_t                 BitDepthChromaMinus8                             : __CODEGEN_BITFIELD(24, 26)    ; //!< BIT_DEPTH_CHROMA_MINUS8
                uint32_t                 BitDepthLumaMinus8                               : __CODEGEN_BITFIELD(27, 29)    ; //!< BIT_DEPTH_LUMA_MINUS8
                uint32_t                 Reserved190                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 LcuMaxBitsizeAllowed                             : __CODEGEN_BITFIELD( 0, 15)    ; //!< LCU Max BitSize Allowed
                uint32_t                 Nonfirstpassflag                                 : __CODEGEN_BITFIELD(16, 16)    ; //!< NONFIRSTPASSFLAG
                uint32_t                 Reserved209                                      : __CODEGEN_BITFIELD(17, 23)    ; //!< Reserved
                uint32_t                 LcumaxbitstatusenLcumaxsizereportmask            : __CODEGEN_BITFIELD(24, 24)    ; //!< LCUMAXBITSTATUSEN_LCUMAXSIZEREPORTMASK
                uint32_t                 FrameszoverstatusenFramebitratemaxreportmask     : __CODEGEN_BITFIELD(25, 25)    ; //!< FRAMESZOVERSTATUSEN_FRAMEBITRATEMAXREPORTMASK
                uint32_t                 FrameszunderstatusenFramebitrateminreportmask    : __CODEGEN_BITFIELD(26, 26)    ; //!< FRAMESZUNDERSTATUSEN_FRAMEBITRATEMINREPORTMASK
                uint32_t                 Reserved219                                      : __CODEGEN_BITFIELD(27, 28)    ; //!< Reserved
                uint32_t                 LoadSlicePointerFlag                             : __CODEGEN_BITFIELD(29, 29)    ; //!< LOAD_SLICE_POINTER_FLAG
                uint32_t                 Reserved222                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            struct
            {
                uint32_t                 Framebitratemax                                  : __CODEGEN_BITFIELD( 0, 13)    ; //!< FrameBitRateMax
                uint32_t                 Reserved238                                      : __CODEGEN_BITFIELD(14, 30)    ; //!< Reserved
                uint32_t                 Framebitratemaxunit                              : __CODEGEN_BITFIELD(31, 31)    ; //!< FRAMEBITRATEMAXUNIT
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            struct
            {
                uint32_t                 Framebitratemin                                  : __CODEGEN_BITFIELD( 0, 13)    ; //!< FrameBitRateMin
                uint32_t                 Reserved270                                      : __CODEGEN_BITFIELD(14, 30)    ; //!< Reserved
                uint32_t                 Framebitrateminunit                              : __CODEGEN_BITFIELD(31, 31)    ; //!< FRAMEBITRATEMINUNIT
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            struct
            {
                uint32_t                 Framebitratemindelta                             : __CODEGEN_BITFIELD( 0, 14)    ; //!< FRAMEBITRATEMINDELTA
                uint32_t                 Reserved303                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 Framebitratemaxdelta                             : __CODEGEN_BITFIELD(16, 30)    ; //!< FRAMEBITRATEMAXDELTA
                uint32_t                 Reserved319                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            struct
            {
                uint64_t                 Framedeltaqpmax                                                                  ; //!< FrameDeltaQpMax
            };
            uint32_t                     Value[2];
        } DW10_11;
        union
        {
            struct
            {
                uint64_t                 Framedeltaqpmin                                                                  ; //!< FrameDeltaQpMin
            };
            uint32_t                     Value[2];
        } DW12_13;
        union
        {
            struct
            {
                uint64_t                 Framedeltaqpmaxrange                                                             ; //!< FrameDeltaQpMaxRange
            };
            uint32_t                     Value[2];
        } DW14_15;
        union
        {
            struct
            {
                uint64_t                 Framedeltaqpminrange                                                             ; //!< FrameDeltaQpMinRange
            };
            uint32_t                     Value[2];
        } DW16_17;
        union
        {
            struct
            {
                uint32_t                 Minframesize                                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< MINFRAMESIZE
                uint32_t                 Reserved592                                      : __CODEGEN_BITFIELD(16, 29)    ; //!< Reserved
                uint32_t                 Minframesizeunits                                : __CODEGEN_BITFIELD(30, 31)    ; //!< MINFRAMESIZEUNITS
            };
            uint32_t                     Value;
        } DW18;
        uint32_t                                 Reserved608[13];                                                         //!< DWORD19..31

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HCPPICSTATE                            = 16, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HCP = 7h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 7, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief MINCUSIZE
        //! \details
        //!     Specifies the smallest coding block size.
        enum MINCUSIZE
        {
            MINCUSIZE_8X8                                                    = 0, //!< No additional details
            MINCUSIZE_16X16                                                  = 1, //!< No additional details
            MINCUSIZE_32X32                                                  = 2, //!< No additional details
            MINCUSIZE_64X64                                                  = 3, //!< No additional details
        };

        //! \brief CTBSIZE_LCUSIZE
        //! \details
        //!     Specifies the coding tree block size.
        enum CTBSIZE_LCUSIZE
        {
            CTBSIZE_LCUSIZE_ILLEGALRESERVED                                  = 0, //!< No additional details
            CTBSIZE_LCUSIZE_16X16                                            = 1, //!< No additional details
            CTBSIZE_LCUSIZE_32X32                                            = 2, //!< No additional details
            CTBSIZE_LCUSIZE_64X64                                            = 3, //!< No additional details
        };

        //! \brief MINTUSIZE
        //! \details
        //!     Specifies the smallest allowed transform block size.
        enum MINTUSIZE
        {
            MINTUSIZE_4X4                                                    = 0, //!< No additional details
            MINTUSIZE_8X8                                                    = 1, //!< No additional details
            MINTUSIZE_16X16                                                  = 2, //!< No additional details
            MINTUSIZE_32X32                                                  = 3, //!< No additional details
        };

        //! \brief MAXTUSIZE
        //! \details
        //!     Specifies the largest allowed transform block size.
        enum MAXTUSIZE
        {
            MAXTUSIZE_4X4                                                    = 0, //!< No additional details
            MAXTUSIZE_8X8                                                    = 1, //!< No additional details
            MAXTUSIZE_16X16                                                  = 2, //!< No additional details
            MAXTUSIZE_32X32                                                  = 3, //!< No additional details
        };

        //! \brief MINPCMSIZE
        //! \details
        //!     Specifies the smallest allowed PCM coding block size.
        enum MINPCMSIZE
        {
            MINPCMSIZE_8X8                                                   = 0, //!< No additional details
            MINPCMSIZE_16X16                                                 = 1, //!< No additional details
            MINPCMSIZE_32X32                                                 = 2, //!< No additional details
        };

        //! \brief MAXPCMSIZE
        //! \details
        //!     Specifies the largest allowed PCM coding block size.
        enum MAXPCMSIZE
        {
            MAXPCMSIZE_8X8                                                   = 0, //!< No additional details
            MAXPCMSIZE_16X16                                                 = 1, //!< No additional details
            MAXPCMSIZE_32X32                                                 = 2, //!< No additional details
        };

        //! \brief COLPICISI
        //! \details
        //!     Specifies that the collocated picture is comprised solely of I slices
        //!     and that there are no P or B slices in the picture.
        enum COLPICISI
        {
            COLPICISI_COLLOCATEDPICTUREHASATLEASTONEPORBSLICE                = 0, //!< No additional details
        };

        //! \brief CURPICISI
        //! \details
        //!     Specifies that the current picture is comprised solely of I slices and
        //!     that there are no P or B slices in the picture.
        enum CURPICISI
        {
            CURPICISI_CURRENTPICTUREHASATLEASTONEPORBSLICE                   = 0, //!< No additional details
        };

        //! \brief INSERTTESTFLAG
        //! \details
        //!     <p>CABAC 0 Word Insertion Test Enable (Encoder Only)This bit will modify
        //!     CABAC K equation so that a positive K value can be generated easily.
        //!     This is done for validation purpose only. In normal usage this bit
        //!     should be set to 0.</p>
        //!     <p>Regular equation for generating 'K' value when CABAC 0 Word Insertion
        //!     Test Enable is set to 0.</p>
        //!     <pre>K = {[((96 * pic_bin_count()) - (RawMinCUBits * PicSizeInMinCUs *3)
        //!     + 1023) / 1024] - bytes_in_picture} / 3</pre>
        //!     
        //!     <p>Modified equation when CABAC 0 Word Insertion Test Enable bit set to
        //!     1.</p>
        //!     
        //!     <pre>K = {[((1536 * pic_bin_count()) - (RawMinCUBits * PicSizeInMinCUs
        //!     *3) + 1023) / 1024] - bytes_in_picture} / 3</pre>
        //!     
        //!     <p>Encoder only feature.</p>
        enum INSERTTESTFLAG
        {
            INSERTTESTFLAG_UNNAMED0                                          = 0, //!< No additional details
            INSERTTESTFLAG_UNNAMED1                                          = 1, //!< No additional details
        };

        //! \brief CU_QP_DELTA_ENABLED_FLAG
        //! \details
        //!     <p>cu_qp_delta_enabled_flag = 1 and Max_DQP_Level = 0 or 3 is supported
        //!     for PAK standalone andVDEnc modes.</p>
        enum CU_QP_DELTA_ENABLED_FLAG
        {
            CU_QP_DELTA_ENABLED_FLAG_DISABLE                                 = 0, //!< Does not allow QP change at CU or LCU level, the same QP is used for the entire slice. Max_DQP_Level = 0 (i.e. diff_cu_qp_delta_depath = 0).
            CU_QP_DELTA_ENABLED_FLAG_ENABLE                                  = 1, //!< Allow QP change at CU level. MAX_DQP_level can be >0.
        };

        //! \brief SIGN_DATA_HIDING_FLAG
        //! \details
        //!     Currently not supported in encoder, so must be set to 0 for encoding
        //!     session.
        enum SIGN_DATA_HIDING_FLAG
        {
            SIGN_DATA_HIDING_FLAG_DISABLE                                    = 0, //!< Specifies that sign bit hiding is disabled.
            SIGN_DATA_HIDING_FLAG_ENABLE                                     = 1, //!< Specifies that sign bit hiding is enabled.
        };

        //! \brief FIELDPIC
        //! \details
        //!     <p>Must be zero for encoder only.</p>
        enum FIELDPIC
        {
            FIELDPIC_VIDEOFRAME                                              = 0, //!< No additional details
            FIELDPIC_VIDEOFIELD                                              = 1, //!< No additional details
        };

        //! \brief BOTTOMFIELD
        //! \details
        //!     <p>Must be zero for encoder only</p>
        enum BOTTOMFIELD
        {
            BOTTOMFIELD_BOTTOMFIELD                                          = 0, //!< No additional details
            BOTTOMFIELD_TOPFIELD                                             = 1, //!< No additional details
        };

        enum TRANSFORM_SKIP_ENABLED_FLAG
        {
            TRANSFORM_SKIP_ENABLED_FLAG_DISABLE                              = 0, //!< transform_skip_flag is not supported in the residual coding
            TRANSFORM_SKIP_ENABLED_FLAG_ENABLE                               = 1, //!< transform_skip_flag is supported
        };

        //! \brief AMP_ENABLED_FLAG
        //! \details
        //!     In VDENC mode, this bit should be set to 1.
        enum AMP_ENABLED_FLAG
        {
            AMP_ENABLED_FLAG_DISABLE                                         = 0, //!< Asymmetric motion partitions cannot be used in coding tree blocks.
            AMP_ENABLED_FLAG_ENABLE                                          = 1, //!< Support asymmetric motion partitions, i.e. PartMode equal to PART_2NxnU, PART_2NxnD, PART_nLx2N, or PART_nRx2N.
        };

        enum TRANSQUANT_BYPASS_ENABLE_FLAG
        {
            TRANSQUANT_BYPASS_ENABLE_FLAG_DISABLE                            = 0, //!< cu_transquant_bypass is not supported
            TRANSQUANT_BYPASS_ENABLE_FLAG_ENABLE                             = 1, //!< cu_transquant_bypass is supported
        };

        //! \brief BIT_DEPTH_CHROMA_MINUS8
        //! \details
        //!     This specifies the number of bit allow for Chroma pixels. In 8 bit mode,
        //!     this must be set to 0. Encoder: Supports bit depths 8, 10 and 12 only.
        //!     And also it must be same as Luma. Encoder: Does not support 10 or 12 bit
        //!     Source Pixels and 8bit PAK. i.e. The source pixel depth should be less
        //!     than or equal to the PAK bit depth.
        enum BIT_DEPTH_CHROMA_MINUS8
        {
            BIT_DEPTH_CHROMA_MINUS8_CHROMA8BIT                               = 0, //!< No additional details
            BIT_DEPTH_CHROMA_MINUS8_CHROMA9BIT                               = 1, //!< Only HEVC decoder supports 9 bits chroma.HEVC encoder does not supports 9 bits chroma.
            BIT_DEPTH_CHROMA_MINUS8_CHROMA10BIT                              = 2, //!< No additional details
            BIT_DEPTH_CHROMA_MINUS8_CHROMA11BIT                              = 3, //!< No additional details
            BIT_DEPTH_CHROMA_MINUS8_CHROMA12BIT                              = 4, //!< No additional details
        };

        //! \brief BIT_DEPTH_LUMA_MINUS8
        //! \details
        //!     This specifies the number of bit allow for Luma pixels. In 8 bit mode,
        //!     this must be set to 0. Encoder: Suports bit depths 8, 10 and 12 only.
        //!     Encoder: Does not support 10 or 12 bit Source Pixels and 8bit PAK i.e.
        //!     the source pixel depth should be less than or equal to PAK bit depth.
        enum BIT_DEPTH_LUMA_MINUS8
        {
            BIT_DEPTH_LUMA_MINUS8_LUMA8BIT                                   = 0, //!< No additional details
            BIT_DEPTH_LUMA_MINUS8_LUMA9BIT                                   = 1, //!< Only HEVC decoder supports 9 bits luma.HEVC encoder does not supports 9 bits luma.
            BIT_DEPTH_LUMA_MINUS8_LUMA10BIT                                  = 2, //!< No additional details
            BIT_DEPTH_LUMA_MINUS8_LUMA11BIT                                  = 3, //!< No additional details
            BIT_DEPTH_LUMA_MINUS8_LUMA12BIT                                  = 4, //!< No additional details
        };

        //! \brief LCUMAXBITSTATUSEN_LCUMAXSIZEREPORTMASK
        //! \details
        //!     This is a mask bit controlling if the condition of any LCU in the frame
        //!     exceeds LCUMaxSize.
        enum LCUMAXBITSTATUSEN_LCUMAXSIZEREPORTMASK
        {
            LCUMAXBITSTATUSEN_LCUMAXSIZEREPORTMASK_DISABLE                   = 0, //!< Do not update bit 0 of HCP_IMAGE_STATUS control register. NOTE: This bit MUST BE set to zero for the last BRC pass if SAO first pass also enabled. This will ensure that HW picks up right accumulated deltaQP for SAO sencond pass.
            LCUMAXBITSTATUSEN_LCUMAXSIZEREPORTMASK_ENABLE                    = 1, //!< HW does not use this bit to set the bit in HCP_IMAGE_STATUS_CONTROL register.
        };

        //! \brief FRAMESZOVERSTATUSEN_FRAMEBITRATEMAXREPORTMASK
        //! \details
        //!     This is a mask bit controlling if the condition of frame level bit count
        //!     exceeds FrameBitRateMax.
        enum FRAMESZOVERSTATUSEN_FRAMEBITRATEMAXREPORTMASK
        {
            FRAMESZOVERSTATUSEN_FRAMEBITRATEMAXREPORTMASK_DISABLE            = 0, //!< Do not update bit 1 of HCP_IMAGE_STATUS control register. NOTE: This bit MUST BE set to zero for the last BRC pass if SAO first pass also enabled. This will ensure that HW picks up right accumulated deltaQP for SAO sencond pass.
            FRAMESZOVERSTATUSEN_FRAMEBITRATEMAXREPORTMASK_ENABLE             = 1, //!< HW does not use this bit to set the bit in HCP_IMAGE_STATUS_CONTROL register. It's used pass the bit inHCP_IMAGE_STATUS_MASK register
        };

        //! \brief FRAMESZUNDERSTATUSEN_FRAMEBITRATEMINREPORTMASK
        //! \details
        //!     This is a mask bit controlling if the condition of frame level bit count
        //!     is less than FrameBitRateMin.
        enum FRAMESZUNDERSTATUSEN_FRAMEBITRATEMINREPORTMASK
        {
            FRAMESZUNDERSTATUSEN_FRAMEBITRATEMINREPORTMASK_DISABLE           = 0, //!< Do not update bit 2 (Frame Bit Count Violate -- under run) of HCP_IMAGE_STATUS control register. NOTE: This bit MUST BE set to zero for the last BRC pass if SAO first pass also enabled. This will ensure that HW picks up right accumulated deltaQP for SAO sencond pass.
            FRAMESZUNDERSTATUSEN_FRAMEBITRATEMINREPORTMASK_ENABLE            = 1, //!< Set bit 2 (Frame Bit Count Violate -- under run) of HCP_IMAGE_STATUS control register if the total frame level bit counter is less than or equal to Frame Bit Rate Minimum limit. HW does not use this bit to set the bit in HCP_IMAGE_STATUS_CONTROL register. It's used pass the bit in HCP_IMAGE_STATUS_MASK register
        };

        //! \brief LOAD_SLICE_POINTER_FLAG
        //! \details
        //!     <p>LoadBitStreamPointerPerSlice (Encoder-only)</p>
        //!     <p>To support multiple slice picture and additional header/data
        //!     insertion before and after an encoded slice. When this field is set to
        //!     0, bitstream pointer is only loaded once for the first slice of a frame.
        //!     For subsequent slices in the frame, bitstream data are stitched together
        //!     to form a single output data stream. When this field is set to 1,
        //!     bitstream pointer is loaded for each slice of a frame. Basically
        //!     bitstream data for different slices of a frame will be written to
        //!     different memory locations.</p>
        enum LOAD_SLICE_POINTER_FLAG
        {
            LOAD_SLICE_POINTER_FLAG_DISABLE                                  = 0, //!< Load BitStream Pointer only once for the first slice of a frame.
            LOAD_SLICE_POINTER_FLAG_ENABLE                                   = 1, //!< Load/reload BitStream Pointer only once for the each slice, reload the start location of thebitstream buffer from the Indirect PAK-BSE Object Data Start Address field.
        };

        //! \brief FRAMEBITRATEMAXUNIT
        //! \details
        //!     This field is the Frame Bitrate Maximum Limit Units.
        enum FRAMEBITRATEMAXUNIT
        {
            FRAMEBITRATEMAXUNIT_BYTE                                         = 0, //!< 32byte unit 
            FRAMEBITRATEMAXUNIT_KILOBYTE                                     = 1, //!< 4kbyte unit
        };

        //! \brief FRAMEBITRATEMINUNIT
        //! \details
        //!     This field is the Frame Bitrate Minimum Limit Units.
        enum FRAMEBITRATEMINUNIT
        {
            FRAMEBITRATEMINUNIT_BYTE                                         = 0, //!< 32byte unit 
            FRAMEBITRATEMINUNIT_KILOBYTE                                     = 1, //!< 4kbyte unit
        };

        //! \brief FRAMEBITRATEMINDELTA
        //! \details
        //!     This field is used to select the slice delta QP when FrameBitRateMin Is
        //!     exceeded. It shares the same
        //!                         FrameBitrateMinUnit.
        enum FRAMEBITRATEMINDELTA
        {
            FRAMEBITRATEMINDELTA_UNNAMED0                                    = 0, //!< No additional details
        };

        //! \brief FRAMEBITRATEMAXDELTA
        //! \details
        //!     This field is used to select the slice delta QP when FrameBitRateMax Is
        //!     exceeded. It shares the same
        //!                         FrameBitrateMaxUnit.
        enum FRAMEBITRATEMAXDELTA
        {
            FRAMEBITRATEMAXDELTA_UNNAMED0                                    = 0, //!< No additional details
        };

        //! \brief MINFRAMESIZE
        //! \details
        //!     <p>Minimum Frame Size [15:0] (in Word, 16-bit)(Encoder Only)</p>
        //!     <p>Mininum Frame Size is specified to compensate for intel Rate Control
        //!     Currently zero fill (no need to perform emulation byte insertion) is
        //!     done only to the end of the CABAC_ZERO_WORD insertion (if any) at the
        //!     last slice of a picture. It is needed for CBR. Intel encoder parameter.
        //!     The caller should always make sure that the value, represented by
        //!     Mininum Frame Size, is always less than maximum frame size
        //!     FrameBitRateMax. This field is reserved in Decode mode.</p>
        enum MINFRAMESIZE
        {
            MINFRAMESIZE_UNNAMED0                                            = 0, //!< No additional details
        };

        //! \brief MINFRAMESIZEUNITS
        //! \details
        //!     This field is the Minimum Frame Size Units
        enum MINFRAMESIZEUNITS
        {
            MINFRAMESIZEUNITS_4KB                                            = 0, //!< Minimum Frame Size is in 4Kbytes.
            MINFRAMESIZEUNITS_16KB                                           = 1, //!< Minimum Frame Size is in 16Kbytes.
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        HCP_PIC_STATE_CMD();

        static const size_t dwSize = 32;
        static const size_t byteSize = 128;
    };

    //!
    //! \brief HCP_TILE_POSITION_IN_CTB
    //! \details
    //!     
    //!     
    struct HCP_TILE_POSITION_IN_CTB_CMD
    {
        union
        {
            struct
            {
                uint32_t                 Ctbpos0I                                         : __CODEGEN_BITFIELD( 0,  7)    ; //!< CtbPos0+i
                uint32_t                 Ctbpos1I                                         : __CODEGEN_BITFIELD( 8, 15)    ; //!< CtbPos1+i
                uint32_t                 Ctbpos2I                                         : __CODEGEN_BITFIELD(16, 23)    ; //!< CtbPos2+i
                uint32_t                 Ctbpos3I                                         : __CODEGEN_BITFIELD(24, 31)    ; //!< CtbPos3+i
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        HCP_TILE_POSITION_IN_CTB_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief HCP_TILE_STATE
    //! \details
    //!     The HCP is selected with the Media Instruction Opcode "7h" for all HCP
    //!     Commands. Each HCP command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    struct HCP_TILE_STATE_CMD
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
                uint32_t                 Numtilerowsminus1                                : __CODEGEN_BITFIELD( 0,  4)    ; //!< NumTileRowsMinus1
                uint32_t                 Numtilecolumnsminus1                             : __CODEGEN_BITFIELD( 5,  9)    ; //!< NumTileColumnsMinus1
                uint32_t                 Reserved42                                       : __CODEGEN_BITFIELD(10, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        HCP_TILE_POSITION_IN_CTB_CMD             CtbColumnPositionOfTileColumn[5];                                        //!< DW2..6, Ctb column position of tile column
        HCP_TILE_POSITION_IN_CTB_CMD             CtbRowPositionOfTileRow[6];                                              //!< DW7..12, Ctb row position of tile row

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HCPTILESTATE                           = 17, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HCP = 7h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 7, //!< No additional details
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
        HCP_TILE_STATE_CMD();

        static const size_t dwSize = 13;
        static const size_t byteSize = 52;
    };

    //!
    //! \brief HCP_REF_LIST_ENTRY
    //! \details
    //!     
    //!     
    struct HCP_REF_LIST_ENTRY_CMD
    {
        union
        {
            struct
            {
                uint32_t                 ReferencePictureTbValue                          : __CODEGEN_BITFIELD( 0,  7)    ; //!< Reference Picture tb Value
                uint32_t                 ListEntryLxReferencePictureFrameIdRefaddr07      : __CODEGEN_BITFIELD( 8, 10)    ; //!< list_entry_lX: Reference Picture Frame ID (RefAddr[0-7])
                uint32_t                 ChromaWeightLxFlag                               : __CODEGEN_BITFIELD(11, 11)    ; //!< CHROMA_WEIGHT_LX_FLAG
                uint32_t                 LumaWeightLxFlag                                 : __CODEGEN_BITFIELD(12, 12)    ; //!< LUMA_WEIGHT_LX_FLAG
                uint32_t                 Longtermreference                                : __CODEGEN_BITFIELD(13, 13)    ; //!< LONGTERMREFERENCE
                uint32_t                 FieldPicFlag                                     : __CODEGEN_BITFIELD(14, 14)    ; //!< FIELD_PIC_FLAG
                uint32_t                 BottomFieldFlag                                  : __CODEGEN_BITFIELD(15, 15)    ; //!< BOTTOM_FIELD_FLAG
                uint32_t                 Reserved16                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \brief CHROMA_WEIGHT_LX_FLAG
        //! \details
        //!     Where X is the RefPicListNum and i is the list entry number 0 through
        //!     15. DW2 corresponds to i=0,
        //!                         DW17 corresponds to i=15.
        enum CHROMA_WEIGHT_LX_FLAG
        {
            CHROMA_WEIGHT_LX_FLAG_DEFAULTWEIGHTEDPREDICTIONFORCHROMA         = 0, //!< No additional details
            CHROMA_WEIGHT_LX_FLAG_EXPLICITWEIGHTEDPREDICTIONFORCHROMA        = 1, //!< No additional details
        };

        //! \brief LUMA_WEIGHT_LX_FLAG
        //! \details
        //!     Where X is the RefPicListNum and i is the list entry number 0 through
        //!     15. DW2 corresponds to i=0,
        //!                         DW17 corresponds to i=15.
        enum LUMA_WEIGHT_LX_FLAG
        {
            LUMA_WEIGHT_LX_FLAG_DEFAULTWEIGHTEDPREDICTIONFORLUMA             = 0, //!< No additional details
            LUMA_WEIGHT_LX_FLAG_EXPLICITWEIGHTEDPREDICTIONFORLUMA            = 1, //!< No additional details
        };

        //! \brief LONGTERMREFERENCE
        //! \details
        //!     Where X is the RefPicListNum and i is the list entry number 0 through
        //!     15. DW2 corresponds to i=0,
        //!                         DW17 corresponds to i=15.
        enum LONGTERMREFERENCE
        {
            LONGTERMREFERENCE_SHORTTERMREFERENCE                             = 0, //!< No additional details
            LONGTERMREFERENCE_LONGTERMREFERENCE                              = 1, //!< No additional details
        };

        //! \brief FIELD_PIC_FLAG
        //! \details
        //!     Where X is the RefPicListNum and i is the list entry number 0 through
        //!     15. DW2 corresponds to i=0,
        //!                         DW17 corresponds to i=15.
        enum FIELD_PIC_FLAG
        {
            FIELD_PIC_FLAG_VIDEOFRAME                                        = 0, //!< No additional details
            FIELD_PIC_FLAG_VIDEOFIELD                                        = 1, //!< No additional details
        };

        //! \brief BOTTOM_FIELD_FLAG
        //! \details
        //!     Where X is the RefPicListNum and i is the list entry number 0 through
        //!     15. DW2 corresponds to i=0,
        //!                         DW17 corresponds to i=15.
        enum BOTTOM_FIELD_FLAG
        {
            BOTTOM_FIELD_FLAG_BOTTOMFIELD                                    = 0, //!< No additional details
            BOTTOM_FIELD_FLAG_TOPFIELD                                       = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        HCP_REF_LIST_ENTRY_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief HCP_REF_IDX_STATE
    //! \details
    //!     The HCP is selected with the Media Instruction Opcode "7h" for all HCP
    //!     Commands. Each HCP command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    //!     This is a slice level command used in both encoding and decoding
    //!     processes. For decoder, it is issued with the HCP_BSD_OBJECT command.
    //!     
    //!     Unlike AVC, HEVC allows 16 reference idx entries in each of the L0 and
    //!     L1 list for a progressive picture. Hence, a max total 32 reference idx
    //!     in both lists together.  The same when the picture is a field picture.
    //!     Regardless the number of reference idx entries, there are only max 8
    //!     reference pictures exist at any one time. Multiple reference idx can
    //!     point to the same reference picture and can optionally pic a top or
    //!     bottom field, or frame.
    //!     
    //!     For P-Slice, this command is issued only once, representing L0 list. For
    //!     B-Slice, this command can be issued up to two times, one for L0 list and
    //!     one for L1 list.
    //!     
    struct HCP_REF_IDX_STATE_CMD
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
                uint32_t                 Refpiclistnum                                    : __CODEGEN_BITFIELD( 0,  0)    ; //!< REFPICLISTNUM
                uint32_t                 NumRefIdxLRefpiclistnumActiveMinus1              : __CODEGEN_BITFIELD( 1,  4)    ; //!< num_ref_idx_l[RefPicListNum]_active_minus1
                uint32_t                 Reserved37                                       : __CODEGEN_BITFIELD( 5, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        HCP_REF_LIST_ENTRY_CMD                   Entries[16];                                                             //!< DW2..17, Entries

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HCPREFIDXSTATE                         = 18, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HCP = 7h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 7, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum REFPICLISTNUM
        {
            REFPICLISTNUM_REFERENCEPICTURELIST0                              = 0, //!< No additional details
            REFPICLISTNUM_REFERENCEPICTURELIST1                              = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        HCP_REF_IDX_STATE_CMD();

        static const size_t dwSize = 18;
        static const size_t byteSize = 72;
    };

    //!
    //! \brief HCP_WEIGHTOFFSET_LUMA_ENTRY
    //! \details
    //!     
    //!     
    struct HCP_WEIGHTOFFSET_LUMA_ENTRY_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DeltaLumaWeightLxI                               : __CODEGEN_BITFIELD( 0,  7)    ; //!< delta_luma_weight_lX[i]
                uint32_t                 LumaOffsetLxI                                    : __CODEGEN_BITFIELD( 8, 15)    ; //!< luma_offset_lX[i]
                uint32_t                 Reserved16                                       : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        HCP_WEIGHTOFFSET_LUMA_ENTRY_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief HCP_WEIGHTOFFSET_CHROMA_ENTRY
    //! \details
    //!     
    //!     
    struct HCP_WEIGHTOFFSET_CHROMA_ENTRY_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DeltaChromaWeightLxI0                            : __CODEGEN_BITFIELD( 0,  7)    ; //!< delta_chroma_weight_lX[i][0]
                uint32_t                 ChromaoffsetlxI0                                 : __CODEGEN_BITFIELD( 8, 15)    ; //!< ChromaOffsetLX[i][0]
                uint32_t                 DeltaChromaWeightLxI1                            : __CODEGEN_BITFIELD(16, 23)    ; //!< delta_chroma_weight_lX[i][1]
                uint32_t                 ChromaoffsetlxI1                                 : __CODEGEN_BITFIELD(24, 31)    ; //!< ChromaOffsetLX [i][1]
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        HCP_WEIGHTOFFSET_CHROMA_ENTRY_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief HCP_WEIGHTOFFSET_STATE
    //! \details
    //!     The HCP is selected with the Media Instruction Opcode "7h" for all HCP
    //!     Commands. Each HCP command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    //!     This slice level command is issued in both the encoding and decoding
    //!     processes, if the weighted_pred_flag or weighted_bipred_flag equals one.
    //!     If zero, then this command is not issued. Weight Prediction Values are
    //!     provided in this command.  Only Explicit Weight Prediction is supported
    //!     in encoder. For P-Slice, this command is issued only once together with
    //!     HCP_REF_IDX_STATE Command for L0 list. For B-Slice, this command can be
    //!     issued up to two times together with HCP_REF_IDX_STATE Command, one for
    //!     L0 list and one for L1 list.
    //!     
    struct HCP_WEIGHTOFFSET_STATE_CMD
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
                uint32_t                 Refpiclistnum                                    : __CODEGEN_BITFIELD( 0,  0)    ; //!< REFPICLISTNUM
                uint32_t                 Reserved33                                       : __CODEGEN_BITFIELD( 1, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        HCP_WEIGHTOFFSET_LUMA_ENTRY_CMD          Lumaoffsets[16];                                                         //!< DW2..17, LumaOffsets
        HCP_WEIGHTOFFSET_CHROMA_ENTRY_CMD        Chromaoffsets[16];                                                       //!< DW18..33, ChromaOffsets

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HCPWEIGHTOFFSETSTATE                   = 19, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HCP = 7h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 7, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum REFPICLISTNUM
        {
            REFPICLISTNUM_REFERENCEPICTURELIST0                              = 0, //!< No additional details
            REFPICLISTNUM_REFERENCEPICTURELIST1                              = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        HCP_WEIGHTOFFSET_STATE_CMD();

        static const size_t dwSize = 34;
        static const size_t byteSize = 136;
    };

    //!
    //! \brief HCP_SLICE_STATE
    //! \details
    //!     The HCP is selected with the Media Instruction Opcode "7h" for all HCP
    //!     Commands. Each HCP command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    //!     This is a slice level command used in both encoding and decoding
    //!     processes. For decoder, it is issued with the HCP_BSD_OBJECT command.
    //!     
    struct HCP_SLICE_STATE_CMD
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
                uint32_t                 SlicestartctbxOrSliceStartLcuXEncoder            : __CODEGEN_BITFIELD( 0,  8)    ; //!< SliceStartCtbX or (slice_start_lcu_x encoder)
                uint32_t                 Reserved41                                       : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved
                uint32_t                 SlicestartctbyOrSliceStartLcuYEncoder            : __CODEGEN_BITFIELD(16, 24)    ; //!< SliceStartCtbY or (slice_start_lcu_y encoder)
                uint32_t                 Reserved57                                       : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 NextslicestartctbxOrNextSliceStartLcuXEncoder    : __CODEGEN_BITFIELD( 0,  8)    ; //!< NextSliceStartCtbX or (next_slice_start_lcu_x encoder)
                uint32_t                 Reserved73                                       : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved
                uint32_t                 NextslicestartctbyOrNextSliceStartLcuYEncoder    : __CODEGEN_BITFIELD(16, 24)    ; //!< NextSliceStartCtbY or (next_slice_start_lcu_y encoder)
                uint32_t                 Reserved89                                       : __CODEGEN_BITFIELD(25, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 SliceType                                        : __CODEGEN_BITFIELD( 0,  1)    ; //!< SLICE_TYPE
                uint32_t                 Lastsliceofpic                                   : __CODEGEN_BITFIELD( 2,  2)    ; //!< LASTSLICEOFPIC
                uint32_t                 SliceqpSignFlag                                  : __CODEGEN_BITFIELD( 3,  3)    ; //!< SliceQp Sign Flag
                uint32_t                 DependentSliceFlag                               : __CODEGEN_BITFIELD( 4,  4)    ; //!< dependent_slice_flag
                uint32_t                 SliceTemporalMvpEnableFlag                       : __CODEGEN_BITFIELD( 5,  5)    ; //!< slice_temporal_mvp_enable_flag
                uint32_t                 Sliceqp                                          : __CODEGEN_BITFIELD( 6, 11)    ; //!< SliceQp
                uint32_t                 SliceCbQpOffset                                  : __CODEGEN_BITFIELD(12, 16)    ; //!< SLICE_CB_QP_OFFSET
                uint32_t                 SliceCrQpOffset                                  : __CODEGEN_BITFIELD(17, 21)    ; //!< SLICE_CR_QP_OFFSET
                uint32_t                 Reserved118                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 SliceHeaderDisableDeblockingFilterFlag           : __CODEGEN_BITFIELD( 0,  0)    ; //!< slice_header_disable_deblocking_filter_flag
                uint32_t                 SliceTcOffsetDiv2OrFinalTcOffsetDiv2Encoder      : __CODEGEN_BITFIELD( 1,  4)    ; //!< slice_tc_offset_div2 or (final tc_offset_div2 Encoder)
                uint32_t                 SliceBetaOffsetDiv2OrFinalBetaOffsetDiv2Encoder  : __CODEGEN_BITFIELD( 5,  8)    ; //!< slice_beta_offset_div2 or (final Beta_Offset_div2 Encoder)
                uint32_t                 Reserved137                                      : __CODEGEN_BITFIELD( 9,  9)    ; //!< Reserved
                uint32_t                 SliceLoopFilterAcrossSlicesEnabledFlag           : __CODEGEN_BITFIELD(10, 10)    ; //!< slice_loop_filter_across_slices_enabled_flag
                uint32_t                 SliceSaoChromaFlag                               : __CODEGEN_BITFIELD(11, 11)    ; //!< slice_sao_chroma_flag
                uint32_t                 SliceSaoLumaFlag                                 : __CODEGEN_BITFIELD(12, 12)    ; //!< slice_sao_luma_flag
                uint32_t                 MvdL1ZeroFlag                                    : __CODEGEN_BITFIELD(13, 13)    ; //!< mvd_l1_zero_flag
                uint32_t                 Islowdelay                                       : __CODEGEN_BITFIELD(14, 14)    ; //!< isLowDelay
                uint32_t                 CollocatedFromL0Flag                             : __CODEGEN_BITFIELD(15, 15)    ; //!< collocated_from_l0_flag
                uint32_t                 Chromalog2Weightdenom                            : __CODEGEN_BITFIELD(16, 18)    ; //!< ChromaLog2WeightDenom
                uint32_t                 LumaLog2WeightDenom                              : __CODEGEN_BITFIELD(19, 21)    ; //!< luma_log2_weight_denom
                uint32_t                 CabacInitFlag                                    : __CODEGEN_BITFIELD(22, 22)    ; //!< cabac_init_flag
                uint32_t                 Maxmergeidx                                      : __CODEGEN_BITFIELD(23, 25)    ; //!< MAXMERGEIDX
                uint32_t                 Collocatedrefidx                                 : __CODEGEN_BITFIELD(26, 28)    ; //!< CollocatedRefIDX
                uint32_t                 Reserved157                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 Sliceheaderlength                                : __CODEGEN_BITFIELD( 0, 15)    ; //!< SliceHeaderLength
                uint32_t                 Reserved176                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 Reserved192                                      : __CODEGEN_BITFIELD( 0, 19)    ; //!< Reserved
                uint32_t                 Roundintra                                       : __CODEGEN_BITFIELD(20, 23)    ; //!< ROUNDINTRA
                uint32_t                 Reserved216                                      : __CODEGEN_BITFIELD(24, 25)    ; //!< Reserved
                uint32_t                 Roundinter                                       : __CODEGEN_BITFIELD(26, 29)    ; //!< ROUNDINTER
                uint32_t                 Reserved222                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            struct
            {
                uint32_t                 Reserved224                                      : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 Cabaczerowordinsertionenable                     : __CODEGEN_BITFIELD( 1,  1)    ; //!< CABACZEROWORDINSERTIONENABLE
                uint32_t                 Emulationbytesliceinsertenable                   : __CODEGEN_BITFIELD( 2,  2)    ; //!< EMULATIONBYTESLICEINSERTENABLE
                uint32_t                 Reserved227                                      : __CODEGEN_BITFIELD( 3,  7)    ; //!< Reserved
                uint32_t                 TailInsertionEnable                              : __CODEGEN_BITFIELD( 8,  8)    ; //!< TAIL_INSERTION_ENABLE
                uint32_t                 SlicedataEnable                                  : __CODEGEN_BITFIELD( 9,  9)    ; //!< SLICEDATA_ENABLE
                uint32_t                 HeaderInsertionEnable                            : __CODEGEN_BITFIELD(10, 10)    ; //!< HEADER_INSERTION_ENABLE
                uint32_t                 Reserved235                                      : __CODEGEN_BITFIELD(11, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            struct
            {
                uint32_t                 Reserved256                                      : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 IndirectPakBseDataStartOffsetWrite               : __CODEGEN_BITFIELD( 6, 28)    ; //!< Indirect PAK-BSE Data Start Offset (Write)
                uint32_t                 Reserved285                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            struct
            {
                uint32_t                 TransformskipLambda                              : __CODEGEN_BITFIELD( 0, 15)    ; //!< Transformskip_lambda
                uint32_t                 Reserved304                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            struct
            {
                uint32_t                 TransformskipNumzerocoeffsFactor0                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Transformskip_numzerocoeffs_factor0
                uint32_t                 TransformskipNumnonzerocoeffsFactor0             : __CODEGEN_BITFIELD( 8, 15)    ; //!< Transformskip_numnonzerocoeffs_factor0
                uint32_t                 TransformskipNumzerocoeffsFactor1                : __CODEGEN_BITFIELD(16, 23)    ; //!< Transformskip_numzerocoeffs_factor1
                uint32_t                 TransformskipNumnonzerocoeffsFactor1             : __CODEGEN_BITFIELD(24, 31)    ; //!< Transformskip_numnonzerocoeffs_factor1
            };
            uint32_t                     Value;
        } DW10;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HCPSLICESTATE                          = 20, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HCP = 7h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 7, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief SLICE_TYPE
        //! \details
        //!     In VDENC mode, for HEVC standard this field can be 0 or 2 only.
        enum SLICE_TYPE
        {
            SLICE_TYPE_B_SLICE                                               = 0, //!< No additional details
            SLICE_TYPE_P_SLICE                                               = 1, //!< No additional details
            SLICE_TYPE_I_SLICE                                               = 2, //!< No additional details
            SLICE_TYPE_ILLEGALRESERVED                                       = 3, //!< No additional details
        };

        //! \brief LASTSLICEOFPIC
        //! \details
        //!     This indicates the current slice is the very last slice of the current
        //!     picture
        enum LASTSLICEOFPIC
        {
            LASTSLICEOFPIC_NOTTHELASTSLICEOFTHEPICTURE                       = 0, //!< No additional details
            LASTSLICEOFPIC_LASTSLICEOFTHEPICTURE                             = 1, //!< No additional details
        };

        //! \brief SLICE_CB_QP_OFFSET
        //! \details
        //!     For deblocking purpose, the pic and slice level cb qp offset must be
        //!     provided separately.
        enum SLICE_CB_QP_OFFSET
        {
            SLICE_CB_QP_OFFSET_0                                             = 0, //!< No additional details
            SLICE_CB_QP_OFFSET_1                                             = 1, //!< No additional details
            SLICE_CB_QP_OFFSET_2                                             = 2, //!< No additional details
            SLICE_CB_QP_OFFSET_3                                             = 3, //!< No additional details
            SLICE_CB_QP_OFFSET_4                                             = 4, //!< No additional details
            SLICE_CB_QP_OFFSET_5                                             = 5, //!< No additional details
            SLICE_CB_QP_OFFSET_6                                             = 6, //!< No additional details
            SLICE_CB_QP_OFFSET_7                                             = 7, //!< No additional details
            SLICE_CB_QP_OFFSET_8                                             = 8, //!< No additional details
            SLICE_CB_QP_OFFSET_9                                             = 9, //!< No additional details
            SLICE_CB_QP_OFFSET_10                                            = 10, //!< No additional details
            SLICE_CB_QP_OFFSET_11                                            = 11, //!< No additional details
            SLICE_CB_QP_OFFSET_12                                            = 12, //!< No additional details
            SLICE_CB_QP_OFFSET_NEG_12                                        = 20, //!< No additional details
            SLICE_CB_QP_OFFSET_NEG_11                                        = 21, //!< No additional details
            SLICE_CB_QP_OFFSET_NEG_10                                        = 22, //!< No additional details
            SLICE_CB_QP_OFFSET_NEG_9                                         = 23, //!< No additional details
            SLICE_CB_QP_OFFSET_NEG_8                                         = 24, //!< No additional details
            SLICE_CB_QP_OFFSET_NEG_7                                         = 25, //!< No additional details
            SLICE_CB_QP_OFFSET_NEG_6                                         = 26, //!< No additional details
            SLICE_CB_QP_OFFSET_NEG_5                                         = 27, //!< No additional details
            SLICE_CB_QP_OFFSET_NEG_4                                         = 28, //!< No additional details
            SLICE_CB_QP_OFFSET_NEG_3                                         = 29, //!< No additional details
            SLICE_CB_QP_OFFSET_NEG_2                                         = 30, //!< No additional details
            SLICE_CB_QP_OFFSET_NEG_1                                         = 31, //!< No additional details
        };

        //! \brief SLICE_CR_QP_OFFSET
        //! \details
        //!     For deblocking purpose, the pic and slice level cr qp offset must be
        //!     provided separately.
        enum SLICE_CR_QP_OFFSET
        {
            SLICE_CR_QP_OFFSET_0                                             = 0, //!< No additional details
            SLICE_CR_QP_OFFSET_1                                             = 1, //!< No additional details
            SLICE_CR_QP_OFFSET_2                                             = 2, //!< No additional details
            SLICE_CR_QP_OFFSET_3                                             = 3, //!< No additional details
            SLICE_CR_QP_OFFSET_4                                             = 4, //!< No additional details
            SLICE_CR_QP_OFFSET_5                                             = 5, //!< No additional details
            SLICE_CR_QP_OFFSET_6                                             = 6, //!< No additional details
            SLICE_CR_QP_OFFSET_7                                             = 7, //!< No additional details
            SLICE_CR_QP_OFFSET_8                                             = 8, //!< No additional details
            SLICE_CR_QP_OFFSET_9                                             = 9, //!< No additional details
            SLICE_CR_QP_OFFSET_10                                            = 10, //!< No additional details
            SLICE_CR_QP_OFFSET_11                                            = 11, //!< No additional details
            SLICE_CR_QP_OFFSET_12                                            = 12, //!< No additional details
            SLICE_CR_QP_OFFSET_NEG_12                                        = 20, //!< No additional details
            SLICE_CR_QP_OFFSET_NEG_11                                        = 21, //!< No additional details
            SLICE_CR_QP_OFFSET_NEG_10                                        = 22, //!< No additional details
            SLICE_CR_QP_OFFSET_NEG_9                                         = 23, //!< No additional details
            SLICE_CR_QP_OFFSET_NEG_8                                         = 24, //!< No additional details
            SLICE_CR_QP_OFFSET_NEG_7                                         = 25, //!< No additional details
            SLICE_CR_QP_OFFSET_NEG_6                                         = 26, //!< No additional details
            SLICE_CR_QP_OFFSET_NEG_5                                         = 27, //!< No additional details
            SLICE_CR_QP_OFFSET_NEG_4                                         = 28, //!< No additional details
            SLICE_CR_QP_OFFSET_NEG_3                                         = 29, //!< No additional details
            SLICE_CR_QP_OFFSET_NEG_2                                         = 30, //!< No additional details
            SLICE_CR_QP_OFFSET_NEG_1                                         = 31, //!< No additional details
        };

        //! \brief MAXMERGEIDX
        //! \details
        //!     MaxNumMergeCand = 5 - five_minus_max_num_merge_cand -1.
        enum MAXMERGEIDX
        {
            MAXMERGEIDX_0                                                    = 0, //!< No additional details
            MAXMERGEIDX_1                                                    = 1, //!< No additional details
            MAXMERGEIDX_2                                                    = 2, //!< No additional details
            MAXMERGEIDX_3                                                    = 3, //!< No additional details
            MAXMERGEIDX_4                                                    = 4, //!< No additional details
        };

        //! \brief ROUNDINTRA
        //! \details
        //!     In VDENC mode, this field is ignored.
        enum ROUNDINTRA
        {
            ROUNDINTRA_132                                                   = 0, //!< No additional details
            ROUNDINTRA_232                                                   = 1, //!< No additional details
            ROUNDINTRA_332                                                   = 2, //!< No additional details
            ROUNDINTRA_432                                                   = 3, //!< No additional details
            ROUNDINTRA_532                                                   = 4, //!< No additional details
            ROUNDINTRA_632                                                   = 5, //!< No additional details
            ROUNDINTRA_732                                                   = 6, //!< No additional details
            ROUNDINTRA_832                                                   = 7, //!< No additional details
            ROUNDINTRA_932                                                   = 8, //!< No additional details
            ROUNDINTRA_1032                                                  = 9, //!< No additional details
            ROUNDINTRA_1132                                                  = 10, //!< No additional details
            ROUNDINTRA_1232                                                  = 11, //!< No additional details
            ROUNDINTRA_1332                                                  = 12, //!< No additional details
            ROUNDINTRA_1432                                                  = 13, //!< No additional details
            ROUNDINTRA_1532                                                  = 14, //!< No additional details
            ROUNDINTRA_1632                                                  = 15, //!< No additional details
        };

        //! \brief ROUNDINTER
        //! \details
        //!     In VDENC mode, this field is ignored.
        enum ROUNDINTER
        {
            ROUNDINTER_132                                                   = 0, //!< No additional details
            ROUNDINTER_232                                                   = 1, //!< No additional details
            ROUNDINTER_332                                                   = 2, //!< No additional details
            ROUNDINTER_432                                                   = 3, //!< No additional details
            ROUNDINTER_532                                                   = 4, //!< No additional details
            ROUNDINTER_632                                                   = 5, //!< No additional details
            ROUNDINTER_732                                                   = 6, //!< No additional details
            ROUNDINTER_832                                                   = 7, //!< No additional details
            ROUNDINTER_932                                                   = 8, //!< No additional details
            ROUNDINTER_1032                                                  = 9, //!< No additional details
            ROUNDINTER_1132                                                  = 10, //!< No additional details
            ROUNDINTER_1232                                                  = 11, //!< No additional details
            ROUNDINTER_1332                                                  = 12, //!< No additional details
            ROUNDINTER_1432                                                  = 13, //!< No additional details
            ROUNDINTER_1532                                                  = 14, //!< No additional details
            ROUNDINTER_1632                                                  = 15, //!< No additional details
        };

        //! \brief CABACZEROWORDINSERTIONENABLE
        //! \details
        //!     To pad the end of a SliceLayer RBSP to meet the encoded size
        //!     requirement.
        enum CABACZEROWORDINSERTIONENABLE
        {
            CABACZEROWORDINSERTIONENABLE_UNNAMED0                            = 0, //!< No Cabac_Zero_Word Insertion.
            CABACZEROWORDINSERTIONENABLE_UNNAMED1                            = 1, //!< Allow internal Cabac_Zero_Word generation and append to the end of RBSP (effectively can be usedas an indicator for last slice of a picture, if the assumption is only the last slice of a pictureneeds to insert CABAC_ZERO_WORDs).
        };

        //! \brief EMULATIONBYTESLICEINSERTENABLE
        //! \details
        //!     To have PAK outputting SODB or EBSP to the output bitstream buffer.
        enum EMULATIONBYTESLICEINSERTENABLE
        {
            EMULATIONBYTESLICEINSERTENABLE_OUTPUTTINGRBSP                    = 0, //!< No additional details
            EMULATIONBYTESLICEINSERTENABLE_OUTPUTTINGEBSP                    = 1, //!< No additional details
        };

        //! \brief TAIL_INSERTION_ENABLE
        //! \details
        //!     Must be followed by the PAK Insertion Object Command to perform the
        //!     actual insertion.
        enum TAIL_INSERTION_ENABLE
        {
            TAIL_INSERTION_ENABLE_UNNAMED0                                   = 0, //!< No tail insertion into the output bitstream buffer, after the current slice encoded bits.
            TAIL_INSERTION_ENABLE_UNNAMED1                                   = 1, //!< Tail insertion into the output bitstream buffer is present, and is after the current slice encoded bits.SKL restriction: Tail insertion is only possible at the end of frame but not in the middle (say slice end)
        };

        //! \brief SLICEDATA_ENABLE
        //! \details
        //!     <p>Must always be enabled.</p>
        //!     <p>Encoder only feature.</p>
        enum SLICEDATA_ENABLE
        {
            SLICEDATA_ENABLE_UNNAMED0                                        = 0, //!< No operation; no insertion.
            SLICEDATA_ENABLE_UNNAMED1                                        = 1, //!< Slice Data insertion by PAK Object Commands into the output bitstream buffer.
        };

        //! \brief HEADER_INSERTION_ENABLE
        //! \details
        //!     Must be followed by the PAK Insertion Object Command to perform the
        //!     actual insertion.
        enum HEADER_INSERTION_ENABLE
        {
            HEADER_INSERTION_ENABLE_UNNAMED0                                 = 0, //!< No header insertion into the output bitstream buffer, before the current slice encoded bits.
            HEADER_INSERTION_ENABLE_UNNAMED1                                 = 1, //!< Header insertion into the output bitstream buffer is present, and is before the current slice encoded bits.
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        HCP_SLICE_STATE_CMD();

        static const size_t dwSize = 11;
        static const size_t byteSize = 44;
    };

    //!
    //! \brief HCP_BSD_OBJECT
    //! \details
    //!      The HCP is selected with the Media Instruction Opcode "7h" for all HCP
    //!     Commands. Each HCP command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    //!      The HCP_BSD_OBJECT command fetches the HEVC bit stream for a slice
    //!     starting with the first byte in the slice. The bit stream ends with the
    //!     last non-zero bit of the frame and does not include any zero-padding at
    //!     the end of the bit stream. There can be multiple slices in a HEVC frame
    //!     and thus this command can be issued multiple times per frame.
    //!     
    //!      The HCP_BSD_OBJECT command must be the last command issued in the
    //!     sequence of batch commands before the HCP starts decoding. Prior to
    //!     issuing this command, it is assumed that all configuration parameters in
    //!     the HCP have been loaded including workload configuration registers and
    //!     configuration tables. When this command is issued, the HCP is waiting
    //!     for bit stream data to be presented to the shift register.
    //!     
    struct HCP_BSD_OBJECT_CMD
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
                uint32_t                 IndirectBsdDataLength                                                            ; //!< Indirect BSD Data Length
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 IndirectDataStartAddress                         : __CODEGEN_BITFIELD( 0, 28)    ; //!< Indirect Data Start Address
                uint32_t                 Reserved93                                       : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HCPBSDOBJECTSTATE                      = 32, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HCP = 7h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 7, //!< No additional details
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
        HCP_BSD_OBJECT_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief HCP_VP9_SEGMENT_STATE
    //! \details
    //!     
    //!     
    struct HCP_VP9_SEGMENT_STATE_CMD
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
                uint32_t                 SegmentId                                        : __CODEGEN_BITFIELD( 0,  2)    ; //!< Segment ID
                uint32_t                 Reserved35                                       : __CODEGEN_BITFIELD( 3, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 SegmentSkipped                                   : __CODEGEN_BITFIELD( 0,  0)    ; //!< Segment Skipped
                uint32_t                 SegmentReference                                 : __CODEGEN_BITFIELD( 1,  2)    ; //!< Segment Reference
                uint32_t                 SegmentReferenceEnabled                          : __CODEGEN_BITFIELD( 3,  3)    ; //!< Segment Reference Enabled
                uint32_t                 Reserved68                                       : __CODEGEN_BITFIELD( 4, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 Filterlevelref0Mode0                             : __CODEGEN_BITFIELD( 0,  5)    ; //!< FilterLevelRef0Mode0
                uint32_t                 Reserved102                                      : __CODEGEN_BITFIELD( 6,  7)    ; //!< Reserved
                uint32_t                 Filterlevelref0Mode1                             : __CODEGEN_BITFIELD( 8, 13)    ; //!< FilterLevelRef0Mode1
                uint32_t                 Reserved110                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 Filterlevelref1Mode0                             : __CODEGEN_BITFIELD(16, 21)    ; //!< FilterLevelRef1Mode0
                uint32_t                 Reserved118                                      : __CODEGEN_BITFIELD(22, 23)    ; //!< Reserved
                uint32_t                 Filterlevelref1Mode1                             : __CODEGEN_BITFIELD(24, 29)    ; //!< FilterLevelRef1Mode1
                uint32_t                 Reserved126                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 Filterlevelref2Mode0                             : __CODEGEN_BITFIELD( 0,  5)    ; //!< FilterLevelRef2Mode0
                uint32_t                 Reserved134                                      : __CODEGEN_BITFIELD( 6,  7)    ; //!< Reserved
                uint32_t                 Filterlevelref2Mode1                             : __CODEGEN_BITFIELD( 8, 13)    ; //!< FilterLevelRef2Mode1
                uint32_t                 Reserved142                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 Filterlevelref3Mode0                             : __CODEGEN_BITFIELD(16, 21)    ; //!< FilterLevelRef3Mode0
                uint32_t                 Reserved150                                      : __CODEGEN_BITFIELD(22, 23)    ; //!< Reserved
                uint32_t                 Filterlevelref3Mode1                             : __CODEGEN_BITFIELD(24, 29)    ; //!< FilterLevelRef3Mode1
                uint32_t                 Reserved158                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 LumaDcQuantScaleDecodeModeOnly                   : __CODEGEN_BITFIELD( 0, 15)    ; //!< Luma DC Quant Scale (Decode mode Only)
                uint32_t                 LumaAcQuantScaleDecodeModeOnly                   : __CODEGEN_BITFIELD(16, 31)    ; //!< Luma AC Quant Scale (Decode mode Only)
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 ChromaDcQuantScaleDecodeModeOnly                 : __CODEGEN_BITFIELD( 0, 15)    ; //!< Chroma DC Quant Scale (Decode mode Only)
                uint32_t                 ChromaAcQuantScaleDecodeModeOnly                 : __CODEGEN_BITFIELD(16, 31)    ; //!< Chroma AC Quant Scale (Decode mode Only)
            };
            uint32_t                     Value;
        } DW6;

	union
        {
            struct
            {
                uint32_t                 SegmentQindexDeltaEncodeModeOnly                 : __CODEGEN_BITFIELD( 0,  8)    ; //!< Segment QIndex Delta (encode mode only)
                uint32_t                 Reserved233                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved
                uint32_t                 SegmentLfLevelDeltaEncodeModeOnly                : __CODEGEN_BITFIELD(16, 22)    ; //!< Segment LF Level Delta (Encode mode Only)
                uint32_t                 Reserved247                                      : __CODEGEN_BITFIELD(23, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HCPVP9SEGMENTSTATE                     = 50, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HUC = Bh
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 7, //!< No additional details
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
        HCP_VP9_SEGMENT_STATE_CMD();

        static const size_t dwSize = 8;
        static const size_t byteSize = 32;
    };

    //!
    //! \brief HCP_FQM_STATE
    //! \details
    //!     The HCP_FQM_STATE command loads the custom HEVC quantization tables into
    //!     local RAM and may be issued up to 8 times: 4 scaling list per intra and
    //!     inter.
    //!     
    //!     Driver is responsible for performing the Scaling List division. So, save
    //!     the division HW cost in HW. The 1/x value is provided in 16-bit
    //!     fixed-point precision as ((1<<17)/QM +1) >> 1.  .
    //!     
    //!     Note: FQM is computed as (2^16)/QM. If QM=1, FQM=all 1's.
    //!     
    //!     To simplify the design, only a limited number of scaling lists are
    //!     provided at the PAK interface: default two SizeID0 and two SizeID123
    //!     (one set for inter and the other set for intra), and the encoder only
    //!     allows custom entries for these four matrices.  The DC value of SizeID2
    //!     and SizeID3 will be provided.
    //!     
    //!     When the scaling_list_enable_flag is set to disable, the scaling matrix
    //!     is still sent to the PAK, and with all entries programmed to the same
    //!     value of 16.
    //!     
    //!     This is a picture level state command and is issued in encoding
    //!     processes only.
    //!     
    //!     Dwords 2-33 form a table for the DCT coefficients, 2 16-bit
    //!     coefficients/DWord.  Size 4x4 for SizeID0, DWords 2-9.
    //!      Size 8x8 for SizeID1/2/3, DWords 2-33.
    //!      
    //!     
    //!     SizeID 0 (Table 4-13) 
    //!     
    struct HCP_FQM_STATE_CMD
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
                uint32_t                 IntraInter                                       : __CODEGEN_BITFIELD( 0,  0)    ; //!< INTRAINTER
                uint32_t                 Sizeid                                           : __CODEGEN_BITFIELD( 1,  2)    ; //!< SIZEID
                uint32_t                 ColorComponent                                   : __CODEGEN_BITFIELD( 3,  4)    ; //!< COLOR_COMPONENT
                uint32_t                 Reserved37                                       : __CODEGEN_BITFIELD( 5, 15)    ; //!< Reserved
                uint32_t                 FqmDcValue1Dc                                    : __CODEGEN_BITFIELD(16, 31)    ; //!< FQM DC Value: (1/DC):
            };
            uint32_t                     Value;
        } DW1;
        uint32_t                                 Quantizermatrix[32];                                                     //!< QuantizerMatrix

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HCPFQMSTATE                            = 5, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HCP = 7h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 7, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief INTRAINTER
        //! \details
        //!     This field specifies the quant matrix intra or inter type.
        enum INTRAINTER
        {
            INTRAINTER_INTRA                                                 = 0, //!< No additional details
            INTRAINTER_INTER                                                 = 1, //!< No additional details
        };

        enum SIZEID
        {
            SIZEID_SIZEID04X4                                                = 0, //!< No additional details
            SIZEID_SIZEID1_2_3_8X8_16X16_32X32                               = 1, //!< No additional details
            SIZEID_SIZEID2_FORDCVALUEIN16X16                                 = 2, //!< No additional details
            SIZEID_SIZEID3_FORDCVALUEIN32X32                                 = 3, //!< No additional details
        };

        //! \brief COLOR_COMPONENT
        //! \details
        //!     <p>Luma and Chroma's share the same scaling list and DC value for the
        //!     same SizeID.</p>
        enum COLOR_COMPONENT
        {
            COLOR_COMPONENT_LUMA                                             = 0, //!< No additional details
            COLOR_COMPONENT_CHROMACB                                         = 1, //!< No additional details
            COLOR_COMPONENT_CHROMACR                                         = 2, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        HCP_FQM_STATE_CMD();

        static const size_t dwSize = 34;
        static const size_t byteSize = 136;
    };

    //!
    //! \brief HCP_PAK_INSERT_OBJECT
    //! \details
    //!     It is an encoder only command, operating at bitstream level, before and
    //!     after SliceData compressed bitstream. It is setup by the header and tail
    //!     present flags in the Slice State command. If these flags are set and no
    //!     subsequent PAK_INSERT_OBJECT commands are issued, the pipeline will
    //!     hang.
    //!     
    //!     The HCP_ PAK_ INSERT _OBJECT command supports both inline and indirect
    //!     data payload, but only one can be active at any time. It is issued to
    //!     insert a chunk of bits (payload) into the current compressed bitstream
    //!     output buffer (specified in the HCP_PAK-BSE Object Base Address field of
    //!     the HCP_IND_OBJ_BASE_ADDR_STATE command) starting at its current write
    //!     pointer bit position. Hardware will keep track of this write pointer's
    //!     byte position and the associated next bit insertion position index.
    //!     
    //!     It is a variable length command when the payload (data to be inserted)
    //!     is presented as inline data within the command itself. The inline
    //!     payload is a multiple of 32-bit (1 DW), as the data bus to the
    //!     compressed bitstream output buffer is 32-bit wide.
    //!     
    //!     The payload data is required to be byte aligned on the left (first
    //!     transmitted bit order) and may or may not be byte aligned on the right
    //!     (last transmitted bits). The command will specify the bit offset of the
    //!     last valid DW. Note that : Stitch Command is used if the beginning
    //!     position of data is in bit position. When PAK Insert Command is used the
    //!     beginning position must be in byte position.
    //!     
    //!     Multiple insertion commands can be issued back to back in a series. It
    //!     is host software's responsibility to make sure their corresponding data
    //!     will properly stitch together to form a valid bitstream.
    //!     
    //!     Internally, HCP hardware will keep track of the very last two bytes'
    //!     (the very last byte can be a partial byte) values of the previous
    //!     insertion. It is required that the next Insertion Object Command or the
    //!     next PAK Object Command to perform the start code emulation sequence
    //!     check and prevention 0x03 byte insertion with this end condition of the
    //!     previous insertion.
    //!     
    //!     The payload data may have already been processed for start code
    //!     emulation byte insertion, except the possibility of the last 2 bytes
    //!     plus the very last partial byte (if any). Hence, when hardware
    //!     performing the concatenation of multiple consecutive insertion commands,
    //!     or concatenation of an insertion command and a PAK object command, it
    //!     must check and perform the necessary start code emulation byte insert at
    //!     the junction.
    //!     
    //!     Data to be inserted can be a valid NAL units or a partial NAL unit. It
    //!     can be any encoded syntax elements bit data before the encoded Slice
    //!     Data (PAK Object Command) of the current Slice - SPS NAL, PPS NAL, SEI
    //!     NAL and Other Non-Slice NAL, Leading_Zero_8_bits (as many bytes as there
    //!     is), Start Code , Slice Header. Any encoded syntax elements bit data
    //!     after the encoded Slice Data (PAK Object Command) of the current Slice
    //!     and prior to  the next encoded Slice Data of the next Slice or prior to
    //!     the end of the bitstream, whichever comes first Cabac_Zero_Word or
    //!     Trailing_Zero_8bits (as many bytes as there is).
    //!     
    //!     Certain NAL unit has a minimum byte size requirement. As such the
    //!     hardware will optionally (enabled by SLICE STATE Command) determines the
    //!     number of CABAC_ZERO_WORD to be inserted to the end of the current NAL,
    //!     based on the minimum byte size of a NAL and the actual bin count of the
    //!     encoded Slice. Since prior to the CABAC_ZERO_WORD insertion, the RBSP or
    //!     EBSP is already byte-aligned, so each CABAC_ZERO_WORD insertion is
    //!     actually a 3-byte sequence 0x00 00 03.
    //!     
    //!     Context switch interrupt is not supported by this command.
    //!     
    struct HCP_PAK_INSERT_OBJECT_CMD
    {
        union
        {
            struct
            {
                uint32_t                 DwordLength                                      : __CODEGEN_BITFIELD( 0, 11)    ; //!< Dword Length
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
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 EndofsliceflagLastdstdatainsertcommandflag       : __CODEGEN_BITFIELD( 1,  1)    ; //!< EndOfSliceFlag - LastDstDataInsertCommandFlag
                uint32_t                 LastheaderflagLastsrcheaderdatainsertcommandflag : __CODEGEN_BITFIELD( 2,  2)    ; //!< LastHeaderFlag - LastSrcHeaderDataInsertCommandFlag
                uint32_t                 EmulationflagEmulationbytebitsinsertenable       : __CODEGEN_BITFIELD( 3,  3)    ; //!< EMULATIONFLAG_EMULATIONBYTEBITSINSERTENABLE
                uint32_t                 SkipemulbytecntSkipEmulationByteCount            : __CODEGEN_BITFIELD( 4,  7)    ; //!< SkipEmulByteCnt - Skip Emulation Byte Count
                uint32_t                 DatabitsinlastdwSrcdataendingbitinclusion50      : __CODEGEN_BITFIELD( 8, 13)    ; //!< DataBitsInLastDW - SrCDataEndingBitInclusion[5:0]
                uint32_t                 Reserved46                                       : __CODEGEN_BITFIELD(14, 14)    ; //!< Reserved
                uint32_t                 Headerlengthexcludefrmsize                       : __CODEGEN_BITFIELD(15, 15)    ; //!< HEADERLENGTHEXCLUDEFRMSIZE_
                uint32_t                 DatabyteoffsetSrcdatastartingbyteoffset10        : __CODEGEN_BITFIELD(16, 17)    ; //!< DataByteOffset - SrcDataStartingByteOffset[1:0]
                uint32_t                 Reserved50                                       : __CODEGEN_BITFIELD(18, 30)    ; //!< Reserved
                uint32_t                 IndirectPayloadEnable                            : __CODEGEN_BITFIELD(31, 31)    ; //!< INDIRECT_PAYLOAD_ENABLE
            };
            uint32_t                     Value;
        } DW1;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HCPPAKINSERTOBJECT                     = 34, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HCP = 7h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 7, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief EMULATIONFLAG_EMULATIONBYTEBITSINSERTENABLE
        //! \details
        //!     Only valid for HEVC and reserved for VP9.
        enum EMULATIONFLAG_EMULATIONBYTEBITSINSERTENABLE
        {
            EMULATIONFLAG_EMULATIONBYTEBITSINSERTENABLE_STARTCODEPREFIX      = 1, //!< Instruct the hardware to perform Start Code Prefix (0x 00 00 01/02/03/00) Search and Prevention Byte (0x 03) insertion on the insertion data of this command. It is required that hardware will handle a start code prefix crossing the boundary between.
            EMULATIONFLAG_EMULATIONBYTEBITSINSERTENABLE_INSERTIONCOMMAND     = 2, //!< Insertion commands, or an insertion command followed by a PAK Object command.
        };

        //! \brief HEADERLENGTHEXCLUDEFRMSIZE_
        //! \details
        //!     <p>In case this flag is on, bits are NOT accumulated during current
        //!     access unit coding neither for Cabac Zero Word insertion bits counting
        //!     or for output in MMIO register
        //!     HCP_BITSTREAM_BYTECOUNT_FRAME_NO_HEADER.</p>
        //!     <p>When using HeaderLenghtExcludeFrmSize for header insertion, the
        //!     software needs to make sure that data comes already with inserted start
        //!     code emulation bytes. SW shouldn't set EmulationFlag bit ( Bit 3 of
        //!     DWORD1 of HCP_PAK_INSERT_OBJECT).</p>
        //!     <table border="1" cellpadding="0" cellspacing="0" style="width: 100%;"
        //!     width="100%">
        //!         <tbody>
        //!             <tr>
        //!                 <td>
        //!                 <p align="center"><b>Value</b></p></td>
        //!                 <td>
        //!                 <p align="center"><b style="text-align:
        //!     -webkit-center;">Description</b></p></td>
        //!             </tr>
        //!             <tr>
        //!                 <td>
        //!                 <p>0</p></td>
        //!                 <td>
        //!                 <p>All bits accumulated</p></td>
        //!             </tr>
        //!             <tr>
        //!                 <td>
        //!                 <p>1</p></td>
        //!                 <td>
        //!                 <p>Bits during current call are not accumulated</p></td>
        //!             </tr>
        //!         </tbody>
        //!     </table>
        //!     
        //!     <p></p>
        enum HEADERLENGTHEXCLUDEFRMSIZE_
        {
            HEADERLENGTHEXCLUDEFRMSIZE_ALLBITSACCUMULATED                    = 0, //!< No additional details
            HEADERLENGTHEXCLUDEFRMSIZE_BITSDURINGCURRENTCALLARENOTACCUMULATED = 1, //!< No additional details
        };

        //! \brief INDIRECT_PAYLOAD_ENABLE
        //! \details
        //!     <p>Only one of these two payload modes can be active at any time.</p>
        //!     <p>When Slice Size Conformance is enable the Payload(header) must be
        //!     inline only so this bit set to MBZ.</p>
        enum INDIRECT_PAYLOAD_ENABLE
        {
            INDIRECT_PAYLOAD_ENABLE_INLINEPAYLOADISUSED                      = 0, //!< No additional details
            INDIRECT_PAYLOAD_ENABLE_INDIRECTPAYLOADISUSED                    = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        HCP_PAK_INSERT_OBJECT_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief HCP_VP9_PIC_STATE
    //! \details
    //!     
    //!     
    struct HCP_VP9_PIC_STATE_CMD
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
                uint32_t                 FrameWidthInPixelsMinus1                         : __CODEGEN_BITFIELD( 0, 13)    ; //!< Frame Width In Pixels Minus 1
                uint32_t                 Reserved46                                       : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 FrameHeightInPixelsMinus1                        : __CODEGEN_BITFIELD(16, 29)    ; //!< Frame Height In Pixels Minus 1
                uint32_t                 Reserved62                                       : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 FrameType                                        : __CODEGEN_BITFIELD( 0,  0)    ; //!< FRAME_TYPE
                uint32_t                 AdaptProbabilitiesFlag                           : __CODEGEN_BITFIELD( 1,  1)    ; //!< ADAPT_PROBABILITIES_FLAG
                uint32_t                 IntraonlyFlag                                    : __CODEGEN_BITFIELD( 2,  2)    ; //!< IntraOnly Flag
                uint32_t                 AllowHiPrecisionMv                               : __CODEGEN_BITFIELD( 3,  3)    ; //!< ALLOW_HI_PRECISION_MV
                uint32_t                 McompFilterType                                  : __CODEGEN_BITFIELD( 4,  6)    ; //!< MCOMP_FILTER_TYPE
                uint32_t                 RefFrameSignBias02                               : __CODEGEN_BITFIELD( 7,  9)    ; //!< Ref Frame Sign Bias[0..2]
                uint32_t                 UsePrevInFindMvReferences                        : __CODEGEN_BITFIELD(10, 10)    ; //!< Use Prev in Find MV References
                uint32_t                 HybridPredictionMode                             : __CODEGEN_BITFIELD(11, 11)    ; //!< HYBRID_PREDICTION_MODE
                uint32_t                 SelectableTxMode                                 : __CODEGEN_BITFIELD(12, 12)    ; //!< SELECTABLE_TX_MODE
                uint32_t                 LastFrameType                                    : __CODEGEN_BITFIELD(13, 13)    ; //!< LAST_FRAME_TYPE
                uint32_t                 RefreshFrameContext                              : __CODEGEN_BITFIELD(14, 14)    ; //!< REFRESH_FRAME_CONTEXT
                uint32_t                 ErrorResilientMode                               : __CODEGEN_BITFIELD(15, 15)    ; //!< ERROR_RESILIENT_MODE
                uint32_t                 FrameParallelDecodingMode                        : __CODEGEN_BITFIELD(16, 16)    ; //!< FRAME_PARALLEL_DECODING_MODE
                uint32_t                 FilterLevel                                      : __CODEGEN_BITFIELD(17, 22)    ; //!< Filter Level
                uint32_t                 SharpnessLevel                                   : __CODEGEN_BITFIELD(23, 25)    ; //!< Sharpness Level
                uint32_t                 SegmentationEnabled                              : __CODEGEN_BITFIELD(26, 26)    ; //!< SEGMENTATION_ENABLED
                uint32_t                 SegmentationUpdateMap                            : __CODEGEN_BITFIELD(27, 27)    ; //!< SEGMENTATION_UPDATE_MAP
                uint32_t                 SegmentationTemporalUpdate                       : __CODEGEN_BITFIELD(28, 28)    ; //!< SEGMENTATION_TEMPORAL_UPDATE
                uint32_t                 LosslessMode                                     : __CODEGEN_BITFIELD(29, 29)    ; //!< LOSSLESS_MODE
                uint32_t                 SegmentIdStreamoutEnable                         : __CODEGEN_BITFIELD(30, 30)    ; //!< SEGMENT_ID_STREAMOUT_ENABLE
                uint32_t                 SegmentIdStreaminEnable                          : __CODEGEN_BITFIELD(31, 31)    ; //!< SEGMENT_ID_STREAMIN_ENABLE
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 Log2TileColumn                                   : __CODEGEN_BITFIELD( 0,  3)    ; //!< LOG2_TILE_COLUMN
                uint32_t                 Reserved100                                      : __CODEGEN_BITFIELD( 4,  7)    ; //!< Reserved
                uint32_t                 Log2TileRow                                      : __CODEGEN_BITFIELD( 8,  9)    ; //!< LOG2_TILE_ROW
                uint32_t                 Reserved106                                      : __CODEGEN_BITFIELD(10, 21)    ; //!< Reserved
                uint32_t                 ChromaSamplingFormat                             : __CODEGEN_BITFIELD(22, 23)    ; //!< CHROMA_SAMPLING_FORMAT
                uint32_t                 Bitdepthminus8                                   : __CODEGEN_BITFIELD(24, 27)    ; //!< BITDEPTHMINUS8
                uint32_t                 ProfileLevel                                     : __CODEGEN_BITFIELD(28, 31)    ; //!< PROFILE_LEVEL
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 VerticalScaleFactorForLast                       : __CODEGEN_BITFIELD( 0, 15)    ; //!< Vertical Scale Factor for LAST
                uint32_t                 HorizontalScaleFactorForLast                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Horizontal Scale Factor for LAST
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 VerticalScaleFactorForGolden                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< Vertical Scale Factor for GOLDEN
                uint32_t                 HorizontalScaleFactorForGolden                   : __CODEGEN_BITFIELD(16, 31)    ; //!< Horizontal Scale Factor for GOLDEN
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 VerticalScaleFactorForAltref                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< Vertical Scale Factor for ALTREF
                uint32_t                 HorizontalScaleFactorForAltref                   : __CODEGEN_BITFIELD(16, 31)    ; //!< Horizontal Scale Factor for ALTREF
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            struct
            {
                uint32_t                 LastFrameWidthInPixelsMinus1                     : __CODEGEN_BITFIELD( 0, 13)    ; //!< Last Frame Width In Pixels Minus 1
                uint32_t                 Reserved238                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 LastFrameHieghtInPixelsMinus1                    : __CODEGEN_BITFIELD(16, 29)    ; //!< Last Frame Hieght In Pixels Minus 1
                uint32_t                 Reserved254                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            struct
            {
                uint32_t                 GoldenFrameWidthInPixelsMinus1                   : __CODEGEN_BITFIELD( 0, 13)    ; //!< Golden Frame Width In Pixels Minus 1
                uint32_t                 Reserved270                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 GoldenFrameHieghtInPixelsMinus1                  : __CODEGEN_BITFIELD(16, 29)    ; //!< Golden Frame Hieght In Pixels Minus 1
                uint32_t                 Reserved286                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            struct
            {
                uint32_t                 AltrefFrameWidthInPixelsMinus1                   : __CODEGEN_BITFIELD( 0, 13)    ; //!< Altref Frame Width In Pixels Minus 1
                uint32_t                 Reserved302                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 AltrefFrameHieghtInPixelsMinus1                  : __CODEGEN_BITFIELD(16, 29)    ; //!< Altref Frame Hieght In Pixels Minus 1
                uint32_t                 Reserved318                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            struct
            {
                uint32_t                 UncompressedHeaderLengthInBytes70                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Uncompressed Header Length in Bytes [7:0]
                uint32_t                 Reserved328                                      : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reserved
                uint32_t                 FirstPartitionSizeInBytes150                     : __CODEGEN_BITFIELD(16, 31)    ; //!< First Partition Size in Bytes [15:0]
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            struct
            {
                uint32_t                 Reserved352                                      : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reserved
                uint32_t                 MotionCompScalingEnableBit                       : __CODEGEN_BITFIELD( 1,  1)    ; //!< MOTION_COMP_SCALING_ENABLE_BIT
                uint32_t                 Reserved354                                      : __CODEGEN_BITFIELD( 2, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            struct
            {
                uint32_t                 Reserved384                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            struct
            {
                uint32_t                 CompressedHeaderBinCount                         : __CODEGEN_BITFIELD( 0, 15)    ; //!< Compressed header BIN count
                uint32_t                 BaseQIndexSameAsLumaAc                           : __CODEGEN_BITFIELD(16, 23)    ; //!< Base Q Index (Same as Luma AC)
                uint32_t                 TailInsertionEnable                              : __CODEGEN_BITFIELD(24, 24)    ; //!< Tail Insertion Enable
                uint32_t                 HeaderInsertionEnable                            : __CODEGEN_BITFIELD(25, 25)    ; //!< Header Insertion Enable
                uint32_t                 Reserved442                                      : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            struct
            {
                uint32_t                 ChromaacQindexdelta                              : __CODEGEN_BITFIELD( 0,  4)    ; //!< ChromaAC_QindexDelta
                uint32_t                 Reserved453                                      : __CODEGEN_BITFIELD( 5,  7)    ; //!< Reserved
                uint32_t                 ChromadcQindexdelta                              : __CODEGEN_BITFIELD( 8, 12)    ; //!< ChromaDC_QindexDelta
                uint32_t                 Reserved461                                      : __CODEGEN_BITFIELD(13, 15)    ; //!< Reserved
                uint32_t                 LumaDcQIndexDelta                                : __CODEGEN_BITFIELD(16, 20)    ; //!< Luma DC Q Index Delta
                uint32_t                 Reserved469                                      : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            struct
            {
                uint32_t                 LfRefDelta0                                      : __CODEGEN_BITFIELD( 0,  6)    ; //!< LF_ref_delta0
                uint32_t                 Reserved487                                      : __CODEGEN_BITFIELD( 7,  7)    ; //!< Reserved
                uint32_t                 LfRefDelta1                                      : __CODEGEN_BITFIELD( 8, 14)    ; //!< LF_ref_delta1
                uint32_t                 Reserved495                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 LfRefDelta2                                      : __CODEGEN_BITFIELD(16, 22)    ; //!< LF_ref_delta2
                uint32_t                 Reserved503                                      : __CODEGEN_BITFIELD(23, 23)    ; //!< Reserved
                uint32_t                 LfRefDelta3                                      : __CODEGEN_BITFIELD(24, 30)    ; //!< LF_ref_delta3
                uint32_t                 Reserved511                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;
        union
        {
            struct
            {
                uint32_t                 LfModeDelta0                                     : __CODEGEN_BITFIELD( 0,  6)    ; //!< LF Mode Delta 0
                uint32_t                 Reserved519                                      : __CODEGEN_BITFIELD( 7,  7)    ; //!< Reserved
                uint32_t                 LfModeDelta1                                     : __CODEGEN_BITFIELD( 8, 14)    ; //!< LF Mode Delta 1
                uint32_t                 Reserved527                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW16;
        union
        {
            struct
            {
                uint32_t                 Bitoffsetforlfrefdelta                           : __CODEGEN_BITFIELD( 0, 15)    ; //!< BitOffsetForLFRefDelta
                uint32_t                 Bitoffsetforlfmodedelta                          : __CODEGEN_BITFIELD(16, 31)    ; //!< BitOffsetForLFModeDelta
            };
            uint32_t                     Value;
        } DW17;
        union
        {
            struct
            {
                uint32_t                 Bitoffsetforqindex                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< BitOffsetForQindex
                uint32_t                 Bitoffsetforlflevel                              : __CODEGEN_BITFIELD(16, 31)    ; //!< BitOffsetForLFLevel
            };
            uint32_t                     Value;
        } DW18;
	union
        {
            struct
            {
                uint32_t                 Reserved608                                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Reserved
                uint32_t                 Nonfirstpassflag                                 : __CODEGEN_BITFIELD(16, 16)    ; //!< NONFIRSTPASSFLAG
                uint32_t                 VdencPakOnlyPass                                 : __CODEGEN_BITFIELD(17, 17)    ; //!< VDENC PAK_ONLY  PASS
                uint32_t                 Reserved626                                      : __CODEGEN_BITFIELD(18, 24)    ; //!< Reserved
                uint32_t                 FrameszoverstatusenFramebitratemaxreportmask     : __CODEGEN_BITFIELD(25, 25)    ; //!< FRAMESZOVERSTATUSEN_FRAMEBITRATEMAXREPORTMASK
                uint32_t                 FrameszunderstatusenFramebitrateminreportmask    : __CODEGEN_BITFIELD(26, 26)    ; //!< FRAMESZUNDERSTATUSEN_FRAMEBITRATEMINREPORTMASK
                uint32_t                 Reserved635                                      : __CODEGEN_BITFIELD(27, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW19;
        union
        {
            struct
            {
                uint32_t                 Framebitratemax                                  : __CODEGEN_BITFIELD( 0, 13)    ; //!< FrameBitRateMax
                uint32_t                 Reserved654                                      : __CODEGEN_BITFIELD(14, 30)    ; //!< Reserved
                uint32_t                 Framebitratemaxunit                              : __CODEGEN_BITFIELD(31, 31)    ; //!< FRAMEBITRATEMAXUNIT
            };
            uint32_t                     Value;
        } DW20;
        union
        {
            struct
            {
                uint32_t                 Framebitratemin                                  : __CODEGEN_BITFIELD( 0, 13)    ; //!< FrameBitRateMin
                uint32_t                 Reserved686                                      : __CODEGEN_BITFIELD(14, 30)    ; //!< Reserved
                uint32_t                 Framebitrateminunit                              : __CODEGEN_BITFIELD(31, 31)    ; //!< FRAMEBITRATEMINUNIT
            };
            uint32_t                     Value;
        } DW21;
        union
        {
            struct
            {
                uint64_t                 Framedeltaqindexmax                                                              ; //!< FrameDeltaQindexMax
            };
            uint32_t                     Value[2];
        } DW22_23;
        union
        {
            struct
            {
                uint32_t                 Framedeltaqindexmin                                                              ; //!< FrameDeltaQindexMin
            };
            uint32_t                     Value;
        } DW24;
        union
        {
            struct
            {
                uint64_t                 Framedeltalfmax                                                                  ; //!< FrameDeltaLFMax
            };
            uint32_t                     Value[2];
        } DW25_26;
        union
        {
            struct
            {
                uint32_t                 Framedeltalfmin                                                                  ; //!< FrameDeltaLFMin
            };
            uint32_t                     Value;
        } DW27;
        union
        {
            struct
            {
                uint64_t                 Framedeltaqindexlfmaxrange                                                       ; //!< FrameDeltaQindexLFMaxRange
            };
            uint32_t                     Value[2];
        } DW28_29;
       	union
        {
            struct
            {
                uint32_t                 Framedeltaqindexlfminrange                                                       ; //!< FrameDeltaQindexLFMinRange
            };
            uint32_t                     Value;
        } DW30;
	union
        {
            struct
            {
                uint32_t                 Minframsize                                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< MinFramSize
                uint32_t                 Reserved1008                                     : __CODEGEN_BITFIELD(16, 29)    ; //!< Reserved
                uint32_t                 Minframesizeunits                                : __CODEGEN_BITFIELD(30, 31)    ; //!< MINFRAMESIZEUNITS
            };
            uint32_t                     Value;
        } DW31;
        union
        {
            struct
            {
                uint32_t                 Bitoffsetforfirstpartitionsize                   : __CODEGEN_BITFIELD( 0, 15)    ; //!< BitOffsetForFirstPartitionSize
                uint32_t                 Reserved1040                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW32;
        union
        {
            struct
            {
                uint32_t                 Class0SseThreshold0                              : __CODEGEN_BITFIELD( 0, 15)    ; //!< Class0_SSE_Threshold0
                uint32_t                 Class0SseThreshold1                              : __CODEGEN_BITFIELD(16, 31)    ; //!< Class0_SSE_Threshold1
            };
            uint32_t                     Value;
        } DW33;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HCPVP9PICSTATE                         = 48, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HUC = Bh
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 7, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief FRAME_TYPE
        //! \details
        //!     Specifies the VP9 frame type
        enum FRAME_TYPE
        {
            FRAME_TYPE_KEYFRAME                                              = 0, //!< No additional details
            FRAME_TYPE_INTERFRAME                                            = 1, //!< No additional details
        };

        //! \brief ADAPT_PROBABILITIES_FLAG
        //! \details
        //!     Indicates that the probabilities used to decode this frame should be
        //!     adapted
        enum ADAPT_PROBABILITIES_FLAG
        {
            ADAPT_PROBABILITIES_FLAG_0DONOTADAPT_ERRORRESILIENTORFRAMEPARALLELMODEARESET = 0, //!< No additional details
            ADAPT_PROBABILITIES_FLAG_1ADAPT_NOTERRORRESILIENTANDNOTFRAMEPARALLELMODE = 1, //!< No additional details
        };

        //! \brief ALLOW_HI_PRECISION_MV
        //! \details
        //!     Indicate high precision mode for Motion Vector prediction
        enum ALLOW_HI_PRECISION_MV
        {
            ALLOW_HI_PRECISION_MV_NORMALMODE                                 = 0, //!< No additional details
            ALLOW_HI_PRECISION_MV_HIGHPRECISIONMODE                          = 1, //!< No additional details
        };

        //! \brief MCOMP_FILTER_TYPE
        //! \details
        //!     Indicate Motion Compensation Filter type.
        enum MCOMP_FILTER_TYPE
        {
            MCOMP_FILTER_TYPE_EIGHT_TAP                                      = 0, //!< No additional details
            MCOMP_FILTER_TYPE_EIGHT_TAP_SMOOTH                               = 1, //!< No additional details
            MCOMP_FILTER_TYPE_EIGHT_TAP_SHARP                                = 2, //!< No additional details
            MCOMP_FILTER_TYPE_BILINEAR                                       = 3, //!< No additional details
            MCOMP_FILTER_TYPE_SWITCHABLE                                     = 4, //!< No additional details
        };

        //! \brief HYBRID_PREDICTION_MODE
        //! \details
        //!     Indicates if comp_pred_mode is hybrid
        enum HYBRID_PREDICTION_MODE
        {
            HYBRID_PREDICTION_MODE_COMPPREDICTIONMODEHYBRID_ENCODERDOESNOTPACKCOMPPREDMODEINTERPREDCOMPINPAKOBJINTOBITSTREAM = 0, //!< No additional details
            HYBRID_PREDICTION_MODE_COMPPREDICTIONMODEHYBRID_ENCODERPACKSCOMPPREDMODEINTOBITSTREAMTHISHELPSREDUCEBITSTREAMSIZEFURTHER = 1, //!< No additional details
        };

        //! \brief SELECTABLE_TX_MODE
        //! \details
        //!     Indicates if tx_mode is selectable
        enum SELECTABLE_TX_MODE
        {
            SELECTABLE_TX_MODE_ENCODERDOESNOTPACKTUSIZEINTOBITSTREAMTHISHELPSREDUCEBITSTREAMSIZEFURTHER = 0, //!< No additional details
            SELECTABLE_TX_MODE_ENCODERPACKSTUSIZEINTOBITSTREAM               = 1, //!< No additional details
        };

        //! \brief LAST_FRAME_TYPE
        //! \details
        //!     <p>It indicates the frame type of previous frame (Key or Non-Key
        //!     Frame)</p>
        enum LAST_FRAME_TYPE
        {
            LAST_FRAME_TYPE_KEYFRAME                                         = 0, //!< No additional details
            LAST_FRAME_TYPE_NONKEYFRAME                                      = 1, //!< No additional details
        };

        //! \brief REFRESH_FRAME_CONTEXT
        //! \details
        //!     <p>Indicates if Frame Context should be refresh.  This bit should come
        //!     from Uncompressed header</p>
        enum REFRESH_FRAME_CONTEXT
        {
            REFRESH_FRAME_CONTEXT_DISABLE                                    = 0, //!< No additional details
            REFRESH_FRAME_CONTEXT_ENABLE                                     = 1, //!< No additional details
        };

        //! \brief ERROR_RESILIENT_MODE
        //! \details
        //!     <p>Indicates if error resilient mode is enabled. This bit should come
        //!     from Uncompressed header.When error resilient is 1, Frame Parallel
        //!     Decoding Mode will be 1, and Refresh Frame Context will be 0.When error
        //!     resilient is 0, Frame Parallel Decoding Mode and Refresh Frame Context
        //!     read from bit stream.Together with Frame Parallel Decoding mode, they
        //!     decide the value of AdaptProbabilityFlag.</p>
        enum ERROR_RESILIENT_MODE
        {
            ERROR_RESILIENT_MODE_DISABLE                                     = 0, //!< No additional details
            ERROR_RESILIENT_MODE_ENABLE                                      = 1, //!< No additional details
        };

        //! \brief FRAME_PARALLEL_DECODING_MODE
        //! \details
        //!     <p>Indicates if parallel decoding mode is enabled.  This bit should come
        //!     from Uncompressed header.  Together with Error Resilient mode, they
        //!     decide the value of AdaptProbabilityFlag.</p>
        enum FRAME_PARALLEL_DECODING_MODE
        {
            FRAME_PARALLEL_DECODING_MODE_DISABLE                             = 0, //!< No additional details
            FRAME_PARALLEL_DECODING_MODE_ENABLE                              = 1, //!< No additional details
        };

        //! \brief SEGMENTATION_ENABLED
        //! \details
        //!     Indicate if segementation is enabled or not
        enum SEGMENTATION_ENABLED
        {
            SEGMENTATION_ENABLED_ALLBLOCKSAREIMPLIEDTOBELONGTOSEGMENT0       = 0, //!< No additional details
            SEGMENTATION_ENABLED_SEGIDDETERMINATIONDEPENDSONSEGMENTATIONUPDATEMAPSETTING = 1, //!< No additional details
        };

        //! \brief SEGMENTATION_UPDATE_MAP
        //! \details
        //!     Indicates how hardware determines segmentation ID
        enum SEGMENTATION_UPDATE_MAP
        {
            SEGMENTATION_UPDATE_MAP_UNNAMED0                                 = 0, //!< Intra block:  segment ID is zero Inter block:  get segment ID from previous frame (streamIN)
            SEGMENTATION_UPDATE_MAP_UNNAMED1                                 = 1, //!< Intra block:  decode segment ID from bitstream.  Inter block: determins from segmentation_temporal_update setting
        };

        //! \brief SEGMENTATION_TEMPORAL_UPDATE
        //! \details
        //!     Indicates whether segID is decoding from bitstream or predicted from
        //!     previous frame.
        enum SEGMENTATION_TEMPORAL_UPDATE
        {
            SEGMENTATION_TEMPORAL_UPDATE_DECODESEGIDFROMBITSTREAM            = 0, //!< No additional details
            SEGMENTATION_TEMPORAL_UPDATE_GETSEGIDEITHERFROMBITSTREAMORFROMPREVIOUSFRAME = 1, //!< No additional details
        };

        //! \brief LOSSLESS_MODE
        //! \details
        //!     This bitSet to indicate lossless coding mode.
        enum LOSSLESS_MODE
        {
            LOSSLESS_MODE_NORMALMODE                                         = 0, //!< No additional details
            LOSSLESS_MODE_LOLESSMODE                                         = 1, //!< No additional details
        };

        //! \brief SEGMENT_ID_STREAMOUT_ENABLE
        //! \details
        //!     Indicates SegmentID of current frame needs to be streamOut for next
        //!     frame
        enum SEGMENT_ID_STREAMOUT_ENABLE
        {
            SEGMENT_ID_STREAMOUT_ENABLE_DISABLE                              = 0, //!< No additional details
            SEGMENT_ID_STREAMOUT_ENABLE_ENABLE                               = 1, //!< No additional details
        };

        //! \brief SEGMENT_ID_STREAMIN_ENABLE
        //! \details
        //!     Indicates SegmentID from previous frame needs to be streamIn for Segment
        //!     ID prediction
        enum SEGMENT_ID_STREAMIN_ENABLE
        {
            SEGMENT_ID_STREAMIN_ENABLE_DISABLE                               = 0, //!< No additional details
            SEGMENT_ID_STREAMIN_ENABLE_ENABLE                                = 1, //!< No additional details
        };

        //! \brief LOG2_TILE_COLUMN
        //! \details
        //!     This indicates the number of tile rows (log2).
        enum LOG2_TILE_COLUMN
        {
            LOG2_TILE_COLUMN_1TILECOLUMN                                     = 0, //!< No additional details
            LOG2_TILE_COLUMN_2TILECOLUMN                                     = 1, //!< No additional details
            LOG2_TILE_COLUMN_4TILECOLUMN                                     = 2, //!< No additional details
            LOG2_TILE_COLUMN_8TILECOLUMN                                     = 3, //!< No additional details
            LOG2_TILE_COLUMN_16TILECOLUMN                                    = 4, //!< No additional details
            LOG2_TILE_COLUMN_32TILECOLUMN                                    = 5, //!< No additional details
            LOG2_TILE_COLUMN_64TILECOLUMN                                    = 6, //!< No additional details
        };

        //! \brief LOG2_TILE_ROW
        //! \details
        //!     This indicates the number of tile rows (log2).
        enum LOG2_TILE_ROW
        {
            LOG2_TILE_ROW_1TILEROW                                           = 0, //!< No additional details
            LOG2_TILE_ROW_2TILEROW                                           = 1, //!< No additional details
            LOG2_TILE_ROW_4TILEROW                                           = 2, //!< No additional details
        };

        //! \brief CHROMA_SAMPLING_FORMAT
        //! \details
        //!     This indicates the chroma sampling format of the bitstream
        enum CHROMA_SAMPLING_FORMAT
        {
            CHROMA_SAMPLING_FORMAT_FORMAT420                                 = 0, //!< No additional details
        };

        //! \brief BITDEPTHMINUS8
        //! \details
        //!     This indicates the bitdepth (minus 8) of the pixels
        enum BITDEPTHMINUS8
        {
            BITDEPTHMINUS8_BITDEPTH8                                         = 0, //!< No additional details
            BITDEPTHMINUS8_BITDEPTH10                                        = 2, //!< No additional details
            BITDEPTHMINUS8_BITDEPTH12                                        = 4, //!< No additional details
        };

        //! \brief PROFILE_LEVEL
        //! \details
        //!     This indicates VP9 Profile level from bitstream
        enum PROFILE_LEVEL
        {
            PROFILE_LEVEL_PROFILE0                                           = 0, //!< Profile 0 only supports 8 bit 420 only
            PROFILE_LEVEL_PROFILE2                                           = 2, //!< Profile 2 only supports 10 bits 420 only
        };

        //! \brief MOTION_COMP_SCALING_ENABLE_BIT
        //! \details
        //!     This bit must be set to "1"
        enum MOTION_COMP_SCALING_ENABLE_BIT
        {
            MOTION_COMP_SCALING_ENABLE_BIT_ENABLE                            = 1, //!< This enables Motion Comp Scaling
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        HCP_VP9_PIC_STATE_CMD();

        static const size_t dwSize = 34;
        static const size_t byteSize = 136;
    };

    //!
    //! \brief HEVC_VP9_RDOQ_LAMBDA_FIELDS
    //! \details
    //!     
    //!     
    struct HEVC_VP9_RDOQ_LAMBDA_FIELDS_CMD
    {
        union
        {
            struct
            {
                uint32_t                 Lambdavalue0                                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< LambdaValue0
                uint32_t                 Lambdavalue1                                     : __CODEGEN_BITFIELD(16, 31)    ; //!< LambdaValue1
            };
            uint32_t                     Value;
        } DW0;

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        HEVC_VP9_RDOQ_LAMBDA_FIELDS_CMD();

        static const size_t dwSize = 1;
        static const size_t byteSize = 4;
    };

    //!
    //! \brief HEVC_VP9_RDOQ_STATE
    //! \details
    //!     
    //!     
    struct HEVC_VP9_RDOQ_STATE_CMD
    {
        union
        {
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
            struct
            {
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0, 29)    ; //!< Reserved
                uint32_t                 DisableHtqPerformanceFix1                        : __CODEGEN_BITFIELD(30, 30)    ; //!< Disable HTQ performance fix1
                uint32_t                 DisableHtqPerformanceFix0                        : __CODEGEN_BITFIELD(31, 31)    ; //!< Disable HTQ performance fix0
            };
            uint32_t                     Value;
        } DW1;
        HEVC_VP9_RDOQ_LAMBDA_FIELDS_CMD          Intralumalambda[32];                                                     //!< DW2..33, IntraLumaLambda
        HEVC_VP9_RDOQ_LAMBDA_FIELDS_CMD          Intrachromalambda[32];                                                   //!< DW34..65, IntraChromaLambda
        HEVC_VP9_RDOQ_LAMBDA_FIELDS_CMD          Interlumalambda[32];                                                     //!< DW66..97, InterLumaLambda
        HEVC_VP9_RDOQ_LAMBDA_FIELDS_CMD          Interchromalambda[32];                                                   //!< DW98..129, InterChromaLambda

        //! \name Local enumerations

        enum SUBOPB
        {
            SUBOPB_UNNAMED8                                                  = 8, //!< No additional details
        };

        enum SUBOPA
        {
            SUBOPA_UNNAMED0                                                  = 0, //!< No additional details
        };

        //! \brief OPCODE
        //! \details
        //!     Codec/Engine Name = HCP = 7h
        enum OPCODE
        {
            OPCODE_UNNAMED7                                                  = 7, //!< No additional details
        };

        //! \brief PIPELINE
        //! \details
        //!     MFX_COMMON
        enum PIPELINE
        {
            PIPELINE_UNNAMED2                                                = 2, //!< No additional details
        };

        //! \brief COMMAND_TYPE
        //! \details
        //!     PARALLEL_VIDEO_PIPE
        enum COMMAND_TYPE
        {
            COMMAND_TYPE_UNNAMED3                                            = 3, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        HEVC_VP9_RDOQ_STATE_CMD();

        static const size_t dwSize = 130;
        static const size_t byteSize = 520;
    };

};

#pragma pack()

#endif  // __MHW_VDBOX_HCP_HWCMD_G9_KBL_H__
