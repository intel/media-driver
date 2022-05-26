/*
* Copyright (c) 2017-2020, Intel Corporation
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
//! \file     mhw_vdbox_avp_hwcmd_g12_X.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of g12_X as other components
//!           should use MHW interface to interact with MHW commands and states.
//!

// DO NOT EDIT

#ifndef __MHW_AVP_HWCMD_G12_H__
#define __MHW_AVP_HWCMD_G12_H__

#pragma once
#pragma pack(1)

#include <cstdint>
#include <cstddef>

class mhw_vdbox_avp_g12_X
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
                uint32_t                 CompressionType                                  : __CODEGEN_BITFIELD(10, 10)    ; //!< COMPRESSION_TYPE
                uint32_t                 Reserved11                                       : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved
                uint32_t                 BaseAddressRowStoreScratchBufferCacheSelect      : __CODEGEN_BITFIELD(12, 12)    ; //!< BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT
                uint32_t                 BaseAddressTiledResourceMode                     : __CODEGEN_BITFIELD(13, 14)    ; //!< BASE_ADDRESS_TILED_RESOURCE_MODE
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
            COMPRESSION_TYPE_RENDERCOMPRESSIONENABLE                         = 1, //!< No additional details
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
        //!     For Media Surfaces: This field specifies the tiled resource mode.
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
                uint64_t                 BaseAddress                                      : __CODEGEN_BITFIELD(12, 63)    ; //!< Base Address
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
                uint64_t                 BaseAddress                                      : __CODEGEN_BITFIELD( 6, 63)    ; //!< Base Address
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
    //! \brief AVP_BSD_OBJECT
    //! \details
    //!      The AVP Pipeline is selected with the Media Instruction Opcode "8h" for
    //!     all AVP Commands. Each AVP command has assigned a media instruction
    //!     command as defined in DWord 0, BitField 22:16. 
    //!     
    //!     style="margin:0in 0in 7.5pt">The AVP_BSD_OBJECT command sends to HW a
    //!     tile at a time from an AV1 bitstream, starting with the first coded byte
    //!     of the tile, not including the prefixed tile byte size. The bit stream
    //!     of a tile, tile group, and of a frame may end with trailing bits and
    //!     extra padding zero bytes. The prefixed tile byte size includes all the
    //!     trailing bits and padding zero bytes at the end of a tile.
    //!     style="margin:0in 0in 7.5pt">Each tile's coded/compressed bitstream is
    //!     started and ended at a byte boundary.
    //!     style="margin:0in 0in 7.5pt">HW is not required to parse the trailing
    //!     bits and padding zero bytes. HW can stop processing right after it has
    //!     completed the decoding of the last block in the tile. Potentially, error
    //!     checkng can be implemented to detect the trailing bits and padding
    //!     zeros, but is not implemented in this generation of AVP Pipeline.
    //!     style="margin:0in 0in 7.5pt">here can be multiple tiles in an AV1 frame
    //!     and thus this command can be issued multiple times per frame. A coded
    //!     frame minumum has at least 1 tile definition, i.e a tile can cover the
    //!     entire frame, unless the frame size exceeds the max allowed tile size
    //!     limits in pixels, then the frame must contain more than 1 tile.There is
    //!     no compressed header in AV1, hence AVP_BSD_OBJECT command is only used
    //!     to process the bitstream of each individual tile of a frame.
    //!     style="margin:0in 0in 7.5pt; text-align:start;
    //!     -webkit-text-stroke-width:0px">The AVP_BSD_OBJECT command must be the
    //!     last command issued in the sequence of batch commands before the AVP
    //!     Pipeline starts decoding. Prior to issuing this command, it is assumed
    //!     that all configuration parameters needed by the AVP Pipeline have been
    //!     loaded in a specific order,including workload configuration registers
    //!     and configuration tables. When this command is issued, the AVP Pipeline
    //!     is waiting for bitstream data to be presented to its bitstream input
    //!     shift register.
    //!     
    struct AVP_BSD_OBJECT_CMD
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
                uint32_t                 TileIndirectBsdDataLength                                                        ; //!< Tile Indirect BSD Data Length
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 TileIndirectDataStartAddress                                                     ; //!< Tile Indirect Data Start Address
            };
            uint32_t                     Value;
        } DW2;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_AVPBSDOBJECTSTATE                      = 32, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = AV1 = 3h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 3, //!< No additional details
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
        AVP_BSD_OBJECT_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;

    };

    //!
    //! \brief AVP_PIC_STATE
    //! \details
    //!     All AVP_PIC_STATE should stay the same for the whole frame even if
    //!     AVP_PIC_STATE is re-programmed for every tiles.
    //!     The bitfields of AVP_PIC_STATE are defined either from
    //!     1) syntax elements of the uncompressed sequence header (received from
    //!     sequence_header_obu) and of the uncompressed frame header (received from
    //!     frame_header_obu),
    //!     2) or, parameters derived from 1).
    //!     Note : Bitstreams may contain several copies of the frame header (there
    //!     can only be one frame_header_obu, but multiple
    //!     redundant_frame_header_obu)interspersed with tile_group_obu to allow for
    //!     greater error resilience. However, the copies must contain identical
    //!     contents to the original frame_header_obu.
    //!     Note : there should be only one sequence_header_obu per video sequence.
    //!     Note : AVP pipeline is invoked to decode a frame from the bitstream,
    //!     only if that frame has show_existing_frame flag (syntax element in the
    //!     frame header) set to 0. For the case that show_existing_frame flag is set
    //!     to 1, application and driver process the frame instead, no block level
    //!     decoding is needed.
    //!     Note : Unlike VP9, AV1 does not have a compressed header. All the syntax
    //!     elements defined in the AV1 sequence and frame level headers are not
    //!     arithmetic coded, hence application and driver can directly read them
    //!     off from the bitstream. If encryption is ON, then only application can
    //!     parse the sequence and frame headers.
    //!     Note : the values of the sequence header/level syntax elements and their
    //!     derived parameters are to last throughout all frames in the video
    //!     sequence, until the next Sequence Header OBU is received that may change
    //!     them. But some sequence header/level syntax elements or their derived
    //!     parameters may further qualified by frame header/level syntax elements
    //!     and their derived parameters, then these type of syntax elements and
    //!     their derived parameters can be changed frame to frame.
    //!     Note : the values of the frame header/level syntax elements and their
    //!     derived parameters can be changed from frame to frame.
    //!     Note : there are some syntax elements and their derived parameters can
    //!     be changed only at KEY FRAME. Hence, the values of these type of syntax
    //!     elements and their derived parameters can last for the entire GOP, i.e.
    //!     until the next KEY FRAME that may change them.
    //!     Note : there is no separate profile for Still Picture. Still Picture is
    //!     coded and decoded as a KEY FRAME, with all coding tools supported
    //!     (tiling, all post in-loop filters, film grain injection, monochrome,
    //!     intraBC, palette prediction mode, etc.). There is no restriction in
    //!     coding Still Picture as a KEY FRAME.
    //!     
    struct AVP_PIC_STATE_CMD
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
                uint32_t                 SequenceChromaSubsamplingFormat                  : __CODEGEN_BITFIELD( 0,  1)    ; //!< SEQUENCE_CHROMA_SUBSAMPLING_FORMAT
                uint32_t                 Reserved66                                       : __CODEGEN_BITFIELD( 2,  2)    ; //!< Reserved (for expansion of Chroma SubSampling Format)
                uint32_t                 SequencePixelBitDepthIdc                         : __CODEGEN_BITFIELD( 3,  4)    ; //!< Sequence Pixel Bit-Depth Idc
                uint32_t                 Reserved69                                       : __CODEGEN_BITFIELD( 5,  5)    ; //!< Reserved (for expansion of Sequence Pixel Bit-Depth Idc)
                uint32_t                 SequenceFullColorRangeFlag                       : __CODEGEN_BITFIELD( 6,  6)    ; //!< Sequence Full Color Range Flag
                uint32_t                 SequenceSuperblockSizeUsed                       : __CODEGEN_BITFIELD( 7,  8)    ; //!< Sequence Superblock Size Used
                uint32_t                 SequenceEnableOrderHintFlag                      : __CODEGEN_BITFIELD( 9,  9)    ; //!< Sequence Enable Order Hint Flag
                uint32_t                 SequenceOrderHintBitsMinus1                      : __CODEGEN_BITFIELD(10, 12)    ; //!< Sequence Order Hint Bits Minus1
                uint32_t                 Reserved77                                       : __CODEGEN_BITFIELD(13, 15)    ; //!< Reserved (for the expansion of Sequence Order Hint Bits Minus1)
                uint32_t                 SequenceEnableFilterIntraFlag                    : __CODEGEN_BITFIELD(16, 16)    ; //!< Sequence Enable Filter_Intra Flag
                uint32_t                 SequenceEnableIntraEdgeFilterFlag                : __CODEGEN_BITFIELD(17, 17)    ; //!< Sequence Enable Intra Edge Filter Flag
                uint32_t                 SequenceEnableDualFilterFlag                     : __CODEGEN_BITFIELD(18, 18)    ; //!< Sequence Enable Dual_Filter Flag
                uint32_t                 SequenceEnableInterIntraCompoundFlag             : __CODEGEN_BITFIELD(19, 19)    ; //!< Sequence Enable Inter-Intra Compound Flag
                uint32_t                 SequenceEnableMaskedCompoundFlag                 : __CODEGEN_BITFIELD(20, 20)    ; //!< Sequence Enable Masked Compound Flag
                uint32_t                 SequenceEnableJointCompoundFlag                  : __CODEGEN_BITFIELD(21, 21)    ; //!< Sequence Enable Joint Compound Flag
                uint32_t                 Reserved86                                       : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 AllowScreenContentToolsFlag                      : __CODEGEN_BITFIELD( 0,  0)    ; //!< Allow Screen Content Tools Flag
                uint32_t                 ForceIntegerMvFlag                               : __CODEGEN_BITFIELD( 1,  1)    ; //!< Force Integer MV Flag
                uint32_t                 AllowWarpedMotionFlag                            : __CODEGEN_BITFIELD( 2,  2)    ; //!< Allow Warped Motion Flag
                uint32_t                 Reserved99                                       : __CODEGEN_BITFIELD( 3,  3)    ; //!< Reserved
                uint32_t                 UseCdefFilterFlag                                : __CODEGEN_BITFIELD( 4,  4)    ; //!< Use CDEF Filter Flag
                uint32_t                 UseSuperResFlag                                  : __CODEGEN_BITFIELD( 5,  5)    ; //!< Use Super-Res Flag
                uint32_t                 FrameLevelLoopRestorationFilterEnableFlag        : __CODEGEN_BITFIELD( 6,  6)    ; //!< Frame Level Loop Restoration Filter Enable Flag 
                uint32_t                 Reserved103                                      : __CODEGEN_BITFIELD( 7, 7)     ; //!< Reserved
                uint32_t                 LargeScaleTileEnableFlag                         : __CODEGEN_BITFIELD( 8,  8)    ; //!< Large Scale Tile Enable Flag
                uint32_t                 Reserved104                                      : __CODEGEN_BITFIELD( 9, 15)    ; //!< Reserved
                uint32_t                 FrameType                                        : __CODEGEN_BITFIELD(16, 17)    ; //!< Frame Type
                uint32_t                 Reserved114                                      : __CODEGEN_BITFIELD(18, 18)    ; //!< Reserved (for the expansion of Frame Type)
                uint32_t                 IntraonlyFlag                                    : __CODEGEN_BITFIELD(19, 19)    ; //!< IntraOnly Flag
                uint32_t                 ShowFrameFlag                                    : __CODEGEN_BITFIELD(20, 20)    ; //!< Show Frame Flag
                uint32_t                 ShowableFrameFlag                                : __CODEGEN_BITFIELD(21, 21)    ; //!< Showable Frame Flag
                uint32_t                 ErrorResilientModeFlag                           : __CODEGEN_BITFIELD(22, 22)    ; //!< ERROR_RESILIENT_MODE_FLAG
                uint32_t                 AllowIntrabcFlag                                 : __CODEGEN_BITFIELD(23, 23)    ; //!< Allow IntraBC Flag
                uint32_t                 Reserved120                                      : __CODEGEN_BITFIELD(24, 27)    ; //!< Reserved
                uint32_t                 PrimaryReferenceFrameIdx                         : __CODEGEN_BITFIELD(28, 30)    ; //!< Primary Reference Frame Idx
                uint32_t                 Reserved127                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved (for future expansion of Primary Reference Frame Idx)
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 SegmentationEnableFlag                           : __CODEGEN_BITFIELD( 0,  0)    ; //!< SEGMENTATION_ENABLE_FLAG
                uint32_t                 SegmentationUpdateMapFlag                        : __CODEGEN_BITFIELD( 1,  1)    ; //!< Segmentation Update Map Flag
                uint32_t                 SegmentationTemporalUpdateFlag                   : __CODEGEN_BITFIELD( 2,  2)    ; //!< SEGMENTATION_TEMPORAL_UPDATE_FLAG
                uint32_t                 PreSkipSegmentIdFlag                             : __CODEGEN_BITFIELD( 3,  3)    ; //!< Pre-Skip Segment ID Flag
                uint32_t                 LastActiveSegmentId                              : __CODEGEN_BITFIELD( 4,  6)    ; //!< Last Active Segment ID
                uint32_t                 DeltaQPresentFlag                                : __CODEGEN_BITFIELD( 7,  7)    ; //!< Delta Q Present Flag
                uint32_t                 DeltaQRes                                        : __CODEGEN_BITFIELD( 8,  9)    ; //!< Delta Q RES
                uint32_t                 FrameCodedLosslessMode                           : __CODEGEN_BITFIELD(10, 10)    ; //!< FRAME_CODED_LOSSLESS_MODE
                uint32_t                 SegmentMapIsZeroFlag                             : __CODEGEN_BITFIELD(11, 11)    ; //!< Segment Map Is Zero Flag
                uint32_t                 SegmentIdBufferStreamInEnableFlag                : __CODEGEN_BITFIELD(12, 12)    ; //!< Reserved
                uint32_t                 SegmentIdBufferStreamOutEnableFlag               : __CODEGEN_BITFIELD(13, 13)    ; //!< Reserved
                uint32_t                 Reserved140                                      : __CODEGEN_BITFIELD(14, 15)    ; //!< Reserved
                uint32_t                 BaseQindex                                       : __CODEGEN_BITFIELD(16, 23)    ; //!< Base Qindex
                uint32_t                 YDcDeltaQ                                        : __CODEGEN_BITFIELD(24, 30)    ; //!< Y_dc_delta_q
                uint32_t                 Reserved159                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 UDcDeltaQ                                        : __CODEGEN_BITFIELD( 0,  6)    ; //!< U_dc_delta_q
                uint32_t                 Reserved167                                      : __CODEGEN_BITFIELD( 7,  7)    ; //!< Reserved
                uint32_t                 UAcDeltaQ                                        : __CODEGEN_BITFIELD( 8, 14)    ; //!< U_ac_delta_q
                uint32_t                 Reserved175                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 VDcDeltaQ                                        : __CODEGEN_BITFIELD(16, 22)    ; //!< V_dc_delta_q
                uint32_t                 Reserved183                                      : __CODEGEN_BITFIELD(23, 23)    ; //!< Reserved
                uint32_t                 VAcDeltaQ                                        : __CODEGEN_BITFIELD(24, 30)    ; //!< V_ac_delta_q
                uint32_t                 Reserved191                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 AllowHighPrecisionMv                             : __CODEGEN_BITFIELD( 0,  0)    ; //!< Allow High Precision MV
                uint32_t                 FrameLevelReferenceModeSelect                    : __CODEGEN_BITFIELD( 1,  1)    ; //!< Frame Level Reference Mode Select
                uint32_t                 McompFilterType                                  : __CODEGEN_BITFIELD( 2,  4)    ; //!< MCOMP_FILTER_TYPE
                uint32_t                 Reserved197                                      : __CODEGEN_BITFIELD( 5,  5)    ; //!< Reserved (for future expansion of Mcomp Filter Type)
                uint32_t                 MotionModeSwitchableFlag                         : __CODEGEN_BITFIELD( 6,  6)    ; //!< Motion Mode Switchable Flag
                uint32_t                 UseReferenceFrameMvSetFlag                       : __CODEGEN_BITFIELD( 7,  7)    ; //!< Use Reference Frame MV Set Flag
                uint32_t                 ReferenceFrameSignBiasI0To7                      : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reference Frame Sign Bias [i=0 to 7]
                uint32_t                 CurrentFrameOrderHint                            : __CODEGEN_BITFIELD(16, 23)    ; //!< Current Frame Order Hint
                uint32_t                 Reserved216                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved (for future expansion of Frame Order Hint)
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            struct
            {
                uint32_t                 ReducedTxSetUsed                                 : __CODEGEN_BITFIELD( 0,  0)    ; //!< Reduced Tx Set Used
                uint32_t                 FrameTransformMode                               : __CODEGEN_BITFIELD( 1,  2)    ; //!< Frame Transform Mode
                uint32_t                 Reserved227                                      : __CODEGEN_BITFIELD( 3,  3)    ; //!< Reserved
                uint32_t                 SkipModePresentFlag                              : __CODEGEN_BITFIELD( 4,  4)    ; //!< Skip Mode Present Flag
                uint32_t                 SkipModeFrame0                                   : __CODEGEN_BITFIELD( 5,  7)    ; //!< Skip Mode Frame [0]
                uint32_t                 Reserved232                                      : __CODEGEN_BITFIELD( 8,  8)    ; //!< Reserved (for future expansion of Skip Mode Frame[0])
                uint32_t                 SkipModeFrame1                                   : __CODEGEN_BITFIELD( 9, 11)    ; //!< Skip Mode Frame [1]
                uint32_t                 Reserved236                                      : __CODEGEN_BITFIELD(12, 23)    ; //!< Reserved (for future expansion of Skip Mode Frame[1])
                uint32_t                 RefFrameSide                                     : __CODEGEN_BITFIELD(24, 31)    ; //!< ref_frame_side for the 8 reference frames INTRA...ALTREF
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            struct
            {
                uint32_t                 Reserved256                                      : __CODEGEN_BITFIELD( 0,  2)    ; //!< Reserved (for future expansion of Global Motion Type[0])
                uint32_t                 GlobalMotionType1                                : __CODEGEN_BITFIELD( 3,  4)    ; //!< Global Motion Type[1]
                uint32_t                 Reserved261                                      : __CODEGEN_BITFIELD( 5,  5)    ; //!< Reserved (for future expansion of Global Motion Type[1]
                uint32_t                 GlobalMotionType2                                : __CODEGEN_BITFIELD( 6,  7)    ; //!< Global Motion Type[2]
                uint32_t                 Reserved264                                      : __CODEGEN_BITFIELD( 8,  8)    ; //!< Reserved (for future expansion of Global Motion Type[2]
                uint32_t                 GlobalMotionType3                                : __CODEGEN_BITFIELD( 9, 10)    ; //!< Global Motion Type[3]
                uint32_t                 Reserved267                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved (for future expansion of Global Motion Type[3]
                uint32_t                 GlobalMotionType4                                : __CODEGEN_BITFIELD(12, 13)    ; //!< Global Motion Type[4]
                uint32_t                 Reserved270                                      : __CODEGEN_BITFIELD(14, 14)    ; //!< Reserved (for future expansion of Global Motion Type[4]
                uint32_t                 GlobalMotionType5                                : __CODEGEN_BITFIELD(15, 16)    ; //!< Global Motion Type[5]
                uint32_t                 Reserved273                                      : __CODEGEN_BITFIELD(17, 17)    ; //!< Reserved (for future expansion of Global Motion Type[5]
                uint32_t                 GlobalMotionType6                                : __CODEGEN_BITFIELD(18, 19)    ; //!< Global Motion Type[6]
                uint32_t                 Reserved276                                      : __CODEGEN_BITFIELD(20, 20)    ; //!< Reserved (for future expansion of Global Motion Type[6]
                uint32_t                 GlobalMotionType7                                : __CODEGEN_BITFIELD(21, 22)    ; //!< Global Motion Type[7]
                uint32_t                 Reserved279                                      : __CODEGEN_BITFIELD(23, 23)    ; //!< Reserved (for future expansion of Global Motion Type[7]
                uint32_t                 FrameLevelGlobalMotionInvalidFlags               : __CODEGEN_BITFIELD(24, 31)    ; //!< Frame Level Global Motion Invalid Flags
            };
            uint32_t                     Value;
        } DW8;
        uint32_t                                 WarpParametersArrayReference1To7Projectioncoeff0To5[21];                 //!< Warp Parameters Array [Reference=1 to 7][ProjectionCoeff=0 to 5]
        union
        {
            struct
            {
                uint32_t                 ReferenceFrameIdx0                               : __CODEGEN_BITFIELD( 0,  2)    ; //!< Reference Frame Idx[0]
                uint32_t                 Reserved963                                      : __CODEGEN_BITFIELD( 3,  3)    ; //!< Reserved (for future expansion of reference frame idx0)
                uint32_t                 ReferenceFrameIdx1                               : __CODEGEN_BITFIELD( 4,  6)    ; //!< Reference Frame Idx[1]
                uint32_t                 Reserved967                                      : __CODEGEN_BITFIELD( 7,  7)    ; //!< Reserved (for future expansion of reference frame idx1)
                uint32_t                 ReferenceFrameIdx2                               : __CODEGEN_BITFIELD( 8, 10)    ; //!< Reference Frame Idx[2]
                uint32_t                 Reserved971                                      : __CODEGEN_BITFIELD(11, 11)    ; //!< Reserved (for future expansion of reference frame idx2)
                uint32_t                 ReferenceFrameIdx3                               : __CODEGEN_BITFIELD(12, 14)    ; //!< Reference Frame Idx[3]
                uint32_t                 Reserved975                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved (for future expansion of reference frame idx3)
                uint32_t                 ReferenceFrameIdx4                               : __CODEGEN_BITFIELD(16, 18)    ; //!< Reference Frame Idx[4]
                uint32_t                 Reserved979                                      : __CODEGEN_BITFIELD(19, 19)    ; //!< Reserved (for future expansion of reference frame idx4)
                uint32_t                 ReferenceFrameIdx5                               : __CODEGEN_BITFIELD(20, 22)    ; //!< Reference Frame Idx[5]
                uint32_t                 Reserved983                                      : __CODEGEN_BITFIELD(23, 23)    ; //!< Reserved (for future expansion of reference frame idx5)
                uint32_t                 ReferenceFrameIdx6                               : __CODEGEN_BITFIELD(24, 26)    ; //!< Reference Frame Idx[6]
                uint32_t                 Reserved987                                      : __CODEGEN_BITFIELD(27, 27)    ; //!< Reserved (for future expansion of reference frame idx6)
                uint32_t                 ReferenceFrameIdx7                               : __CODEGEN_BITFIELD(28, 30)    ; //!< Reference Frame Idx[7]
                uint32_t                 Reserved991                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved (for future expansion of reference frame idx7)
            };
            uint32_t                     Value;
        } DW30;
        union
        {
            struct
            {
                uint32_t                 IntraFrameWidthInPixelMinus1                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< Intra Frame Width In Pixel Minus 1
                uint32_t                 IntraFrameHeightInPixelMinus1                    : __CODEGEN_BITFIELD(16, 31)    ; //!< Intra Frame Height In Pixel Minus 1
            };
            uint32_t                     Value;
        } DW31;
        union
        {
            struct
            {
                uint32_t                 LastFrameWidthInPixelMinus1                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Last Frame Width In Pixel Minus 1
                uint32_t                 LastFrameHeightInPixelMinus1                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Last Frame Height In Pixel Minus 1
            };
            uint32_t                     Value;
        } DW32;
        union
        {
            struct
            {
                uint32_t                 Last2FrameWidthInPixelMinus1                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< Last2 Frame Width In Pixel Minus 1
                uint32_t                 Last2FrameHeightInPixelMinus1                    : __CODEGEN_BITFIELD(16, 31)    ; //!< Last2 Frame Height In Pixel Minus 1
            };
            uint32_t                     Value;
        } DW33;
        union
        {
            struct
            {
                uint32_t                 Last3FrameWidthInPixelMinus1                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< Last3 Frame Width In Pixel Minus 1
                uint32_t                 Last3FrameHeightInPixelMinus1                    : __CODEGEN_BITFIELD(16, 31)    ; //!< Last3 Frame Height In Pixel Minus 1
            };
            uint32_t                     Value;
        } DW34;
        union
        {
            struct
            {
                uint32_t                 GoldenFrameWidthInPixelMinus1                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< Golden Frame Width In Pixel Minus 1
                uint32_t                 GoldenFrameHeightInPixelMinus1                   : __CODEGEN_BITFIELD(16, 31)    ; //!< Golden Frame Height In Pixel Minus 1
            };
            uint32_t                     Value;
        } DW35;
        union
        {
            struct
            {
                uint32_t                 BwdrefFrameWidthInPixelMinus1                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< BWDREF Frame Width In Pixel Minus 1
                uint32_t                 BwdrefFrameHeightInPixelMinus1                   : __CODEGEN_BITFIELD(16, 31)    ; //!< BWDREF Frame Height In Pixel Minus 1
            };
            uint32_t                     Value;
        } DW36;
        union
        {
            struct
            {
                uint32_t                 Altref2FrameWidthInPixelMinus1                   : __CODEGEN_BITFIELD( 0, 15)    ; //!< ALTREF2 Frame Width In Pixel Minus 1
                uint32_t                 Altref2FrameHeightInPixelMinus1                  : __CODEGEN_BITFIELD(16, 31)    ; //!< ALTREF2 Frame Height In Pixel Minus 1
            };
            uint32_t                     Value;
        } DW37;
        union
        {
            struct
            {
                uint32_t                 AltrefFrameWidthInPixelMinus1                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< ALTREF Frame Width In Pixel Minus 1
                uint32_t                 AltrefFrameHeightInPixelMinus1                   : __CODEGEN_BITFIELD(16, 31)    ; //!< ALTREF Frame Height In Pixel Minus 1
            };
            uint32_t                     Value;
        } DW38;
        union
        {
            struct
            {
                uint32_t                 VerticalScaleFactorForIntra                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Vertical Scale Factor for INTRA
                uint32_t                 HorizontalScaleFactorForIntra                    : __CODEGEN_BITFIELD(16, 31)    ; //!< Horizontal Scale Factor for INTRA
            };
            uint32_t                     Value;
        } DW39;
        union
        {
            struct
            {
                uint32_t                 VerticalScaleFactorForLast                       : __CODEGEN_BITFIELD( 0, 15)    ; //!< Vertical Scale Factor for LAST
                uint32_t                 HorizontalScaleFactorForLast                     : __CODEGEN_BITFIELD(16, 31)    ; //!< Horizontal Scale Factor for LAST
            };
            uint32_t                     Value;
        } DW40;
        union
        {
            struct
            {
                uint32_t                 VerticalScaleFactorForLast2                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Vertical Scale Factor for LAST2
                uint32_t                 HorizontalScaleFactorForLast2                    : __CODEGEN_BITFIELD(16, 31)    ; //!< Horizontal Scale Factor for LAST2
            };
            uint32_t                     Value;
        } DW41;
        union
        {
            struct
            {
                uint32_t                 VerticalScaleFactorForLast3                      : __CODEGEN_BITFIELD( 0, 15)    ; //!< Vertical Scale Factor for LAST3
                uint32_t                 HorizontalScaleFactorForLast3                    : __CODEGEN_BITFIELD(16, 31)    ; //!< Horizontal Scale Factor for LAST3
            };
            uint32_t                     Value;
        } DW42;
        union
        {
            struct
            {
                uint32_t                 VerticalScaleFactorForGolden                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< Vertical Scale Factor for GOLDEN
                uint32_t                 HorizontalScaleFactorForGolden                   : __CODEGEN_BITFIELD(16, 31)    ; //!< Horizontal Scale Factor for GOLDEN
            };
            uint32_t                     Value;
        } DW43;
        union
        {
            struct
            {
                uint32_t                 VerticalScaleFactorForBwdrefFrame                : __CODEGEN_BITFIELD( 0, 15)    ; //!< Vertical Scale Factor for BWDREF_FRAME
                uint32_t                 HorizontalScaleFactorForBwdrefFrame              : __CODEGEN_BITFIELD(16, 31)    ; //!< Horizontal Scale Factor for BWDREF_FRAME
            };
            uint32_t                     Value;
        } DW44;
        union
        {
            struct
            {
                uint32_t                 VerticalScaleFactorForAltref2                    : __CODEGEN_BITFIELD( 0, 15)    ; //!< Vertical Scale Factor for ALTREF2
                uint32_t                 HorizontalScaleFactorForAltref2                  : __CODEGEN_BITFIELD(16, 31)    ; //!< Horizontal Scale Factor for ALTREF2
            };
            uint32_t                     Value;
        } DW45;
        union
        {
            struct
            {
                uint32_t                 VerticalScaleFactorForAltref                     : __CODEGEN_BITFIELD( 0, 15)    ; //!< Vertical Scale Factor for ALTREF
                uint32_t                 HorizontalScaleFactorForAltref                   : __CODEGEN_BITFIELD(16, 31)    ; //!< Horizontal Scale Factor for ALTREF
            };
            uint32_t                     Value;
        } DW46;
        union
        {
            struct
            {
                uint32_t                 ReferenceFrameOrderHint0ForIntraFrame            : __CODEGEN_BITFIELD( 0,  7)    ; //!< Reference Frame Order Hint [0] for Intra Frame
                uint32_t                 ReferenceFrameOrderHint1ForLastFrame             : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reference Frame Order Hint [1] for Last Frame
                uint32_t                 ReferenceFrameOrderHint2ForLast2Frame            : __CODEGEN_BITFIELD(16, 23)    ; //!< Reference Frame Order Hint [2] for Last2 Frame
                uint32_t                 ReferenceFrameOrderHint3ForLast3Frame            : __CODEGEN_BITFIELD(24, 31)    ; //!< Reference Frame Order Hint [3] for Last3 Frame
            };
            uint32_t                     Value;
        } DW47;
        union
        {
            struct
            {
                uint32_t                 ReferenceFrameOrderHint4ForGoldenFrame           : __CODEGEN_BITFIELD( 0,  7)    ; //!< Reference Frame Order Hint [4] for Golden Frame
                uint32_t                 ReferenceFrameOrderHint5ForBwdrefFrame           : __CODEGEN_BITFIELD( 8, 15)    ; //!< Reference Frame Order Hint [5] for BWDREF Frame
                uint32_t                 ReferenceFrameOrderHint6ForAltref2Frame          : __CODEGEN_BITFIELD(16, 23)    ; //!< Reference Frame Order Hint [6] for ALTREF2 Frame
                uint32_t                 ReferenceFrameOrderHint7ForAltrefFrame           : __CODEGEN_BITFIELD(24, 31)    ; //!< Reference Frame Order Hint [7] for ALTREF Frame
            };
            uint32_t                     Value;
        } DW48;
        union
        {
            struct
            {
                uint32_t                 Reserved1568                                                                     ; //!< Reserved
            };
            uint32_t                     Value;
        } DW49;
        union
        {
            struct
            {
                uint32_t                 Reserved1600                                                                     ; //!< Reserved
            };
            uint32_t                     Value;
        } DW50;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_AVPPICSTATE                            = 48, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = AVP = 3h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 3, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief SEQUENCE_CHROMA_SUBSAMPLING_FORMAT
        //! \details
        //!     It specifies the chroma subsampling format for all frames in the
        //!     video sequencebeing decoded.
        //!     [1:0] = 00 ; stands for Monochrome 4:0:0, no Chroma planes at all,
        //!     but [subsampling_x and subsampling_y] is defaulted to [1, 1], as only
        //!     Profile 0 can support monochrome video coding.
        //!     [1:0] = 01 ; stands for 4:2:0, with[subsampling_x and subsampling_y]
        //!     defining as[1, 1]. It is supported in all profiles (seq_profile=0, 1, 2
        //!     - syntax element in the sequence header)
        //!     [1:0] = 10 ; stands for 4:2:2, with[subsampling_x and subsampling_y]
        //!     defining as[1, 0]. It is supported only in seq_profile=2.
        //!     [1:0] = 11 ; stands for 4:4:4 with[subsampling_x and subsampling_y]
        //!     defining as[0, 0]. It is supported in both seq_profile=1 and 2.
        //!     It is a sequence-level parameter derived from the sequence header
        //!     syntax elements: seq_profile, subsampling_x, subsampling_y, monochome,
        //!     high_bitdepth and twelve_bit. Default is 1, i.e. 4:2:0..
        //!     Note : AV1 supports 3 profiles:
        //!     seq_profile Bit_depth Chroma Subsampling
        //!     0 (Main Profile) 8 / 10 YUV 4:2:0 and 4:0:0
        //!     1 (High Profile) 8 / 10 YUV 4:4:4 (4:0:0 is not allowed)
        //!     2 (Pro Profile) 8 / 10 /12 YUV 4:2:2 AND
        //!     12 YUV 4:2:0/4:4:4/4:0:0
        //!     Note : for AV1 decoder:
        //!     type="disc" style="margin-top:0in; margin-bottom:0in">
        //!     style="margin:0in 0in 0.0001pt; font-size:11pt;
        //!     font-family:Calibri, sans-serif">A profile 0 compliant decoder must be
        //!     able to decode all bitstreams labeled profile 0

        //!     style="margin:0in 0in 0.0001pt; font-size:11pt;
        //!     font-family:Calibri, sans-serif">A profile 1 compliant decoder must be
        //!     able to decode all bitstreams labeled profile 0 or 1

        //!     style="margin:0in 0in 0.0001pt; font-size:11pt;
        //!     font-family:Calibri, sans-serif">A profile 2 compliant decoder must be
        //!     able to decode all bitstreams labeled profile 0, 1, or 2

        //!     
        enum SEQUENCE_CHROMA_SUBSAMPLING_FORMAT
        {
            SEQUENCE_CHROMA_SUBSAMPLING_FORMAT_420                           = 1, //!< Chroma Sampling 4:2:0
        };

        //! \brief ERROR_RESILIENT_MODE_FLAG
        //! \details
        //!     It specifies whether all syntax decoding of the current frame is
        //!     independent of the previous frames, or not.
        //!     Set to 0 to disable error resilient mode
        //!     Set to 1 to enable error resilient mode (for independent syntax
        //!     decoding)
        //!     It is the frame-level syntax element, error_resilient_mode. Default
        //!     is 0.
        //!     It is read from the bitstream for all frame types (KEY Frame,
        //!     INTRA-ONLY Frame and INTER Frame), except when frame_type is set to
        //!     SWITCH_FRAME, in which it is forced to 1 instead of reading from the
        //!     bitstream.
        //!     When error resilient mode is set to 1 (active), Refresh Frame Context
        //!     is set to0. When error resilient is set to 0,Refresh Frame Context is
        //!     read from the bit stream.
        //!     
        //!     Valid only in Decoder Mode
        //!     
        //!     
        enum ERROR_RESILIENT_MODE_FLAG
        {
            ERROR_RESILIENT_MODE_FLAG_DISABLE                                = 0, //!< No additional details
            ERROR_RESILIENT_MODE_FLAG_ENABLE                                 = 1, //!< No additional details
        };

        //! \brief SEGMENTATION_ENABLE_FLAG
        //! \details
        //!     Indicate if segementation is enabled or not
        enum SEGMENTATION_ENABLE_FLAG
        {
            SEGMENTATION_ENABLE_FLAG_ALLBLOCKSAREIMPLIEDTOBELONGTOSEGMENT0   = 0, //!< No additional details
            SEGMENTATION_ENABLE_FLAG_SEGIDDETERMINATIONDEPENDSONSEGMENTATIONUPDATEMAPSETTING = 1, //!< No additional details
        };

        //! \brief SEGMENTATION_TEMPORAL_UPDATE_FLAG
        //! \details
        //!     Indicates whether segID is decoding from bitstream or predicted from
        //!     previous frame.
        enum SEGMENTATION_TEMPORAL_UPDATE_FLAG
        {
            SEGMENTATION_TEMPORAL_UPDATE_FLAG_DECODESEGIDFROMBITSTREAM       = 0, //!< No additional details
            SEGMENTATION_TEMPORAL_UPDATE_FLAG_GETSEGIDEITHERFROMBITSTREAMORFROMPREVIOUSFRAME = 1, //!< No additional details
        };

        //! \brief FRAME_CODED_LOSSLESS_MODE
        //! \details
        //!     This bitSet to indicate lossless coding mode at frame level.
        //!     Frame Coded Lossless Mode is set to 1, if all active segment's
        //!     segment lossless flag are set to 1.
        //!     The equation for deriving coded lossless mode is presented in the
        //!     AVP_SEGMENT_STATE Command.
        //!     AllLossless = CodedLossless &amp;&amp; ( FrameWidth == UpscaledWidth
        //!     ). The second condition in this equation is equivalent tohaving
        //!     Super-res NOT enabled.
        //!     Only CodedLossless flag is sent to HW. AllLossless flag is not.
        //!     CodedLossless directly control the enabling/disabling of deblocker,
        //!     CDEF in-loop filters.
        //!     But AllLossless is used to control the enabling/disabling of Loop
        //!     Restoration filter. Hence, when super-res is ON, Loop Restoration filter
        //!     can still be ON/OFF, regardless of CodedLossless.
        enum FRAME_CODED_LOSSLESS_MODE
        {
            FRAME_CODED_LOSSLESS_MODE_NORMALMODE                             = 0, //!< No additional details
            FRAME_CODED_LOSSLESS_MODE_CODEDLOSSLESSMODE                      = 1, //!< No additional details
        };

        //! \brief MCOMP_FILTER_TYPE
        //! \details
        //!     It specifies which Motion Compensation Filter type is to be used for
        //!     the current frame.
        //!     It is a frame-level derived parameters. It is derived from the frame
        //!     level syntax elements (is_filter_switchable flag and the 2-bits
        //!     interpolation_filter).
        //!     Default is 0 (i.e. use the eight-tap basic filter).
        enum MCOMP_FILTER_TYPE
        {
            MCOMP_FILTER_TYPE_EIGHT_TAP                                      = 0, //!< No additional details
            MCOMP_FILTER_TYPE_EIGHT_TAP_SMOOTH                               = 1, //!< No additional details
            MCOMP_FILTER_TYPE_EIGHT_TAP_SHARP                                = 2, //!< No additional details
            MCOMP_FILTER_TYPE_BILINEAR                                       = 3, //!< No additional details
            MCOMP_FILTER_TYPE_SWITCHABLE                                     = 4, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        AVP_PIC_STATE_CMD();

        static const size_t dwSize = 51;
        static const size_t byteSize = 204;

    };

    //!
    //! \brief AVP_PIPE_MODE_SELECT
    //! \details
    //!     The AVP Pipeline is selected with the Media Instruction Opcode "8h" for
    //!     all AVP Commands. Each AVP command has assigned a media instruction
    //!     command as defined in DWord 0, BitField 22:16.
    //!     
    //!     The workload for the AVP pipeline is tile based. Once the bit stream DMA
    //!     is configured with the AVP_BSD_OBJECT command for a tile in a frame, and
    //!     the tile's bitstream is presented to the AVP, the tile decoding will
    //!     begin.
    //!     AVP pipeline is stateless, i.e. there is no states saved between the
    //!     decoding of each tile. Hence all sequence, frame and segment state
    //!     commands have to be resent before the tile coding command and the BSD
    //!     object command.
    //!     The AVP_PIPE_MODE_SELECT command is responsible for general pipeline
    //!     level configuration that would normally be set once for a single stream
    //!     encode or decode and would not be modified on a frame workload basis.
    //!     This is a frame level state command and is shared by both encoding and
    //!     decoding processes.
    //!     
    struct AVP_PIPE_MODE_SELECT_CMD
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
                uint32_t                 CdefOutputStreamoutEnableFlag                    : __CODEGEN_BITFIELD( 1,  1)    ; //!< CDEF_OUTPUT_STREAMOUT_ENABLE_FLAG
                uint32_t                 LoopRestorationOutputStreamoutEnableFlag         : __CODEGEN_BITFIELD( 2,  2)    ; //!< LOOP_RESTORATION_OUTPUT_STREAMOUT_ENABLE_FLAG
                uint32_t                 PicStatusErrorReportEnable                       : __CODEGEN_BITFIELD( 3,  3)    ; //!< PIC_STATUSERROR_REPORT_ENABLE
                uint32_t                 Reserved36                                       : __CODEGEN_BITFIELD( 4,  4)    ; //!< Reserved
                uint32_t                 CodecStandardSelect                              : __CODEGEN_BITFIELD( 5,  7)    ; //!< CODEC_STANDARD_SELECT
                uint32_t                 Reserved40                                       : __CODEGEN_BITFIELD( 8, 12)    ; //!< Reserved
                uint32_t                 MultiEngineMode                                  : __CODEGEN_BITFIELD(13, 14)    ; //!< MULTI_ENGINE_MODE
                uint32_t                 PipeWorkingMode                                  : __CODEGEN_BITFIELD(15, 16)    ; //!< PIPE_WORKING_MODE
                uint32_t                 TileBasedEngine                                  : __CODEGEN_BITFIELD(17, 17)    ; //!< Tile Based Engine
                uint32_t                 Reserved50                                       : __CODEGEN_BITFIELD(18, 31)    ; //!< Reserved
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
                uint32_t                 Reserved128                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 PhaseIndicator                                   : __CODEGEN_BITFIELD( 0,  1)    ; //!< PHASE_INDICATOR
                uint32_t                 Reserved162                                      : __CODEGEN_BITFIELD( 2, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_AVPPIPEMODESELECT                      = 0, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = AVP = 3h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 3, //!< No additional details
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

        //! \brief CDEF_OUTPUT_STREAMOUT_ENABLE_FLAG
        //! \details
        //!     It allows the Super-Resolution, Loop Restoration Filtering, and Out
        //!     of Loop Film Graim Synthesisto be done in a separate pass with SW
        //!     instead.
        //!     Since these are in-loop filtering, the results are provided back to
        //!     the HW AV1 pipeline.
        //!     This feature is currently not being implemented in HW.
        enum CDEF_OUTPUT_STREAMOUT_ENABLE_FLAG
        {
            CDEF_OUTPUT_STREAMOUT_ENABLE_FLAG_DISABLE                        = 0, //!< Super-Resolution, Loop Restoration Filtering, and Out of Loop Film Graim Synthesis, if enabled, are performed in HW AVP pipeline.
            CDEF_OUTPUT_STREAMOUT_ENABLE_FLAG_ENABLE                         = 1, //!< Super-Resolution, Loop Restoration Filtering, and Out of Loop Film Graim Synthesis, if enabled, are NOT performed in HW AVP pipeline.
        };

        //! \brief LOOP_RESTORATION_OUTPUT_STREAMOUT_ENABLE_FLAG
        //! \details
        //!     It allows the Film Grain Synthesisto be done in a separate pass with
        //!     SW instead.
        //!     All the Post In-Loop Filters, if any enabled, (Deblocker, CDEF,
        //!     Super-Resolution and Loop Restoration) are still be done in AV1 HW
        //!     pipeline.
        //!     This feature is currently not being implemented in HW.
        enum LOOP_RESTORATION_OUTPUT_STREAMOUT_ENABLE_FLAG
        {
            LOOP_RESTORATION_OUTPUT_STREAMOUT_ENABLE_FLAG_DISABLE            = 0, //!< Film Grain Synthesis, if enabled, is performed in HW AVP Pipeline.
            LOOP_RESTORATION_OUTPUT_STREAMOUT_ENABLE_FLAG_ENABLE             = 1, //!< Film Grain Synthesis, if enabled, is NOT performed in HW AVP Pipeline.
        };

        enum PIC_STATUSERROR_REPORT_ENABLE
        {
            PIC_STATUSERROR_REPORT_ENABLE_DISABLE                            = 0, //!< Disable status/error reporting
            PIC_STATUSERROR_REPORT_ENABLE_ENABLE                             = 1, //!< Status/Error reporting is written out once per picture. The Pic Status/Error Report ID in DWord3 along with the status/error status bits are packed into one cache line and written to the Status/Error Buffer address in the AVP_PIPE_BUF_ADDR_STATE command. Must be zero for encoder mode.
        };

        enum CODEC_STANDARD_SELECT
        {
            CODEC_STANDARD_SELECT_HEVC                                       = 0, //!< No additional details
            CODEC_STANDARD_SELECT_VP9                                        = 1, //!< No additional details
            CODEC_STANDARD_SELECT_AV1                                        = 2, //!< No additional details
        };

        //! \brief MULTI_ENGINE_MODE
        //! \details
        //!     This indicates the current pipe is in single pipe mode or if in
        //!     scalable mode is in left/right/middle pipe in multi-engine mode.
        enum MULTI_ENGINE_MODE
        {
            MULTI_ENGINE_MODE_SINGLEENGINEMODEORMSACFEONLYDECODEMODE         = 0, //!< This is for single engine mode (legacy) OR MSAC FE only decode mode During AV1Decoder Scalability Real Tile Mode, for the last phase, it is possible to have single tile column left. In this case, it should be programmed with pipe as a single engine mode (using this value).For example, for 9 tile column running on 4 pipes. The first two phases will use all 4 pipes and finish 8 tile column. The remaining one column will be processed as last third phase as single tile column.
            MULTI_ENGINE_MODE_PIPEISTHELEFTENGINEINAMULTI_ENGINEMODE         = 1, //!< Current pipe is the most left engine while running in scalable multi-engine mode
            MULTI_ENGINE_MODE_PIPEISTHERIGHTENGINEINAMULTI_ENGINEMODE        = 2, //!< Current pipe is the most right engine while running in scalable multi-engine mode
            MULTI_ENGINE_MODE_PIPEISONEOFTHEMIDDLEENGINEINAMULTI_ENGINEMODE  = 3, //!< Current pipe is in one of the middle engine while running in scalable multi-engine mode
        };

        //! \brief PIPE_WORKING_MODE
        //! \details
        //!     This programs the working mode for AVP pipe.
        enum PIPE_WORKING_MODE
        {
            PIPE_WORKING_MODE_LEGACYDECODERENCODERMODE_SINGLEPIPE            = 0, //!< This is for single pipe mode non-scalable mode. It is used by both decoder and encoder.
            PIPE_WORKING_MODE_MSACFEONLYDECODEMODE_SINGLEMSACPIPE            = 1, //!< This is for the single MSAC FE only in decoder mode. This will be only run MSAC and streamout syntax element.
            PIPE_WORKING_MODE_DECODERBEONLYORENCODERMODE_SCALABLEMULTI_PIPE  = 2, //!< This is for multiple-pipe scalable mode. In decoder, it is only on BE reconstruction. In encoder, it is for PAK.
            PIPE_WORKING_MODE_DECODERSCALABLEMODEWITHMSACINREALTILES_SCALABLEMULTI_PIPE = 3, //!< This is for multiple-pipe scalable mode decoder mode in real tiles. MSAC and reconstruction will run together. Each pipes will run in real tiles vertically.
        };

        //! \brief MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS
        //! \details
        //!     In decoder modes, this counter value specifies the number of clocks (per
        //!     1000) of GAC inactivity before a media soft-reset is applied to the AVP
        //!     and HuC. If counter value is set to 0, the media soft-reset feature is
        //!     disabled and no reset will occur.
        //!     In encoder modes, this counter must be set to 0 to disable media soft
        //!     reset. This feature is not supported for the encoder.
        enum MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS
        {
            MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS_DISABLE                 = 0, //!< No additional details
        };

        //! \brief PIC_STATUSERROR_REPORT_ID
        //! \details
        //!     The Pic Status/Error Report ID is a unique 32-bit unsigned integer
        //!     assigned to each picture
        //!     status/error output. Must be zero for encoder mode.
        enum PIC_STATUSERROR_REPORT_ID
        {
            PIC_STATUSERROR_REPORT_ID_32_BITUNSIGNED                         = 0, //!< Unique ID Number
        };

        //! \brief PHASE_INDICATOR
        //! \details
        //!     This is used to indicate whether this is first, middle or last phase
        //!     of programming during Real-Tile Decoder Mode. This is used by hardware
        //!     to know if the current programming is first or last phases.
        //!     This field is ignored (programmed to 0) for other modes other than
        //!     AV1 Real-Tile Decoder Mode.
        enum PHASE_INDICATOR
        {
            PHASE_INDICATOR_FIRSTPHASE                                       = 0, //!< No additional details
            PHASE_INDICATOR_MIDDLEPHASE                                      = 1, //!< No additional details
            PHASE_INDICATOR_LASTPHASE                                        = 2, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        AVP_PIPE_MODE_SELECT_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief AVP_IND_OBJ_BASE_ADDR_STATE
    //! \details
    //!     The AVP Pipeline is selected with the Media Instruction Opcode "8h" for
    //!     all AVP Commands. Each AVP command has assigned a media instruction
    //!     command as defined in DWord 0, BitField 22:16.
    //!     
    //!     The AVP_IND_OBJ_BASE_ADDR_STATE command is used to define the indirect
    //!     object base address of the AV1 compressed bitstream in graphics memory.
    //!     This is a frame level command issued in both encoding and decoding
    //!     processes.
    //!     Although a frame is coded as separate tiles, all tiles' compressed
    //!     bitstream are still required to line up sequentially as one AV1
    //!     bitstream. Hence, there is only one Indirect Object Base Address for the
    //!     entire AV1 codedframe. If the frame contains more than 1 tiles, the BSD
    //!     Object Command will be issued multiple times, once for each tile and
    //!     with its own tile bitstream starting memory address.
    //!     
    struct AVP_IND_OBJ_BASE_ADDR_STATE_CMD
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
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        AvpIndirectBitstreamObjectBaseAddress;                                   //!< DW1..2, AVP Indirect Bitstream Object Base Address
        MEMORYADDRESSATTRIBUTES_CMD              AvpIndirectBitstreamObjectMemoryAddressAttributes;                       //!< DW3, AVP Indirect Bitstream Object Memory Address Attributes
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        AvpIndirectBitstreamObjectAccessUpperBound;                              //!< DW4..5, AVP Indirect Bitstream Object Access Upper Bound

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_AVPINDOBJBASEADDRSTATE                 = 3, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = AVP = 3h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 3, //!< No additional details
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
        AVP_IND_OBJ_BASE_ADDR_STATE_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief AVP_TILE_CODING
    //! \details
    //!     This command is used only for AV1codec. It is issued for every tile of a
    //!     frame. If a frame is composed of only 1 tile, it is still being issued.
    //!     Tiling and Tile Group organization in AV1 cannot be disabled, a frame
    //!     minimum must have 1 tile. Currently, each batch buffer can contain only
    //!     1 tile to be processed, it cannot contain more than 1 tile or the entire
    //!     tile group of tiles.
    //!     When the tile width exceeds 4096 pixels or the tile area exceeds
    //!     4096x2304 pixels, tiling must be performed and number of tiles in such
    //!     frame must be >1. There is no mandatory tiling driven by tile height.
    //!     The frame height in pixels will limit the allowed tile height in extreme
    //!     situation. Hence, the AVP_TILE_CODING can be issued multiple times for
    //!     decoding a frame.
    //!     Since AVP HW pipeline is stateless, all sequence, frame and segment
    //!     level states (coding parameters in all Frame Level State Commands) must
    //!     be resent before sending each TILE_CODING_STATE command.
    //!     Although tile size is specified in SuperBlock unit, the minimum tile
    //!     size is actually set to be 8x8 pixels (which is the same as the minimum
    //!     frame size in pixels). It can also happen to the rightmost tile column
    //!     and bottommost tile row of a frame which is not divisible by the
    //!     SuperBlock size - this leads to the presence of partial tile and partial
    //!     SuperBlock handling.
    //!     AV1 supports both
    //!     1) a uniform-spacing tiling scheme (as in VP9, which is always in the
    //!     form of 2^N x 2^M number of tiles, for the entire frame), and
    //!     2) a non-uniform-spacing tiling scheme. Bitstream syntax will specify
    //!     the width and height of each tile size in the frame.
    //!     AVP HW pipeline is a tile-based codec engine, it does not need to
    //!     distinguish between these two tiling schemes. Driver will take care of
    //!     the difference and details of these tiling schemes. At the end, Driver
    //!     will send out one tile at a time with all the related tile information
    //!     to the HW through this TILE_CODING State Command.
    //!     In AV1, a frame is partitioned by tile row and tile column. That is, a
    //!     tile boundary must go across the full frame width or the full frame
    //!     height only. There is no tiling within a tile.
    //!     For AV1, the max number of tiles per frame is set to 256 in the LEVEL
    //!     definition for regular video decoding. The ext-tile (Virtual Reality
    //!     mode, currently not supported) has a different tiling configuration,
    //!     constraints and definition.
    //!     
    struct AVP_TILE_CODING_CMD
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
                uint32_t                 TgTileNum                                        : __CODEGEN_BITFIELD(12, 23)    ; //!< TG Tile Num
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
                uint32_t                 FilmGrainSampleTemplateWriteReadControl          : __CODEGEN_BITFIELD( 0,  0)    ; //!< FILM_GRAIN_SAMPLE_TEMPLATE_WRITEREAD_CONTROL
                uint32_t                 Reserved129                                      : __CODEGEN_BITFIELD( 1, 24)    ; //!< Reserved
                uint32_t                 IslasttileofcolumnFlag                           : __CODEGEN_BITFIELD(25, 25)    ; //!< IsLastTileOfColumn Flag
                uint32_t                 IslasttileofrowFlag                              : __CODEGEN_BITFIELD(26, 26)    ; //!< IsLastTileOfRow Flag
                uint32_t                 IsstarttileoftilegroupFlag                       : __CODEGEN_BITFIELD(27, 27)    ; //!< IsStartTileOfTileGroup Flag
                uint32_t                 IsendtileoftilegroupFlag                         : __CODEGEN_BITFIELD(28, 28)    ; //!< IsEndTileOfTileGroup Flag
                uint32_t                 IslasttileofframeFlag                            : __CODEGEN_BITFIELD(29, 29)    ; //!< IsLastTileOfFrame Flag
                uint32_t                 DisableCdfUpdateFlag                             : __CODEGEN_BITFIELD(30, 30)    ; //!< Disable CDF Update Flag
                uint32_t                 DisableFrameContextUpdateFlag                    : __CODEGEN_BITFIELD(31, 31)    ; //!< Disable Frame Context Update Flag
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 NumberOfActiveBePipes                            : __CODEGEN_BITFIELD( 0,  7)    ; //!< Number of Active BE Pipes
                uint32_t                 Reserved168                                      : __CODEGEN_BITFIELD( 8, 11)    ; //!< Reserved MBZ
                uint32_t                 NumOfTileColumnsMinus1InAFrame                   : __CODEGEN_BITFIELD(12, 21)    ; //!< Num of Tile Columns Minus1 in a Frame
                uint32_t                 NumOfTileRowsMinus1InAFrame                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Num of Tile Rows Minus1 in a Frame
            };
            uint32_t                     Value;
        } DW5;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_AVPTILECODING                          = 21, //!< No additional details
        };

        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 3, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief FILM_GRAIN_SAMPLE_TEMPLATE_WRITEREAD_CONTROL
        //! \details
        //!     This should only be set to write on the first tile of the frame in
        //!     single pipe mode (when film grain is enabled) and read for the remaining
        //!     tiles. In scalable mode, this should be set to writeon the first phase
        //!     and top tile of the frame and read for the remaining frames.
        enum FILM_GRAIN_SAMPLE_TEMPLATE_WRITEREAD_CONTROL
        {
            FILM_GRAIN_SAMPLE_TEMPLATE_WRITEREAD_CONTROL_READ                = 0, //!< No additional details
            FILM_GRAIN_SAMPLE_TEMPLATE_WRITEREAD_CONTROL_WRITE               = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        AVP_TILE_CODING_CMD();
        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief AVP_TILE_CODING_CMD for LST
    //! \details
    //!     This command is used when Large Scale Tile decoding is supported
    //!
    struct AVP_TILE_CODING_CMD_LST
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
                uint32_t                 TgTileNum                                        : __CODEGEN_BITFIELD(12, 23)    ; //!< TG Tile Num
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
                uint32_t                 Reserved128                                      : __CODEGEN_BITFIELD( 0, 24)    ; //!< Reserved
                uint32_t                 IslasttileofcolumnFlag                           : __CODEGEN_BITFIELD(25, 25)    ; //!< IsLastTileOfColumn Flag
                uint32_t                 IslasttileofrowFlag                              : __CODEGEN_BITFIELD(26, 26)    ; //!< IsLastTileOfRow Flag
                uint32_t                 IsstarttileoftilegroupFlag                       : __CODEGEN_BITFIELD(27, 27)    ; //!< IsStartTileOfTileGroup Flag
                uint32_t                 IsendtileoftilegroupFlag                         : __CODEGEN_BITFIELD(28, 28)    ; //!< IsEndTileOfTileGroup Flag
                uint32_t                 IslasttileofframeFlag                            : __CODEGEN_BITFIELD(29, 29)    ; //!< IsLastTileOfFrame Flag
                uint32_t                 DisableCdfUpdateFlag                             : __CODEGEN_BITFIELD(30, 30)    ; //!< Disable CDF Update Flag
                uint32_t                 DisableFrameContextUpdateFlag                    : __CODEGEN_BITFIELD(31, 31)    ; //!< Disable Frame Context Update Flag
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 NumberOfActiveBePipes                            : __CODEGEN_BITFIELD( 0,  7)    ; //!< Number of Active BE Pipes
                uint32_t                 Reserved168                                      : __CODEGEN_BITFIELD( 8, 11)    ; //!< Reserved MBZ
                uint32_t                 NumOfTileColumnsMinus1InAFrame                   : __CODEGEN_BITFIELD(12, 21)    ; //!< Num of Tile Columns Minus1 in a Frame
                uint32_t                 NumOfTileRowsMinus1InAFrame                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Num of Tile Rows Minus1 in a Frame
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 OutputDecodedTileColumnPositionInSbUnit          : __CODEGEN_BITFIELD( 0,  9)    ; //!< Output Decoded Tile Column Position in SB Unit
                uint32_t                 Reserved202                                      : __CODEGEN_BITFIELD(10, 15)    ; //!< Reserved
                uint32_t                 OutputDecodedTileRowPositionInSbUnit             : __CODEGEN_BITFIELD(16, 25)    ; //!< Output Decoded Tile Row Position in SB Unit
                uint32_t                 Reserved218                                      : __CODEGEN_BITFIELD(26, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_AVPTILECODING                          = 21, //!< No additional details
        };

        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 3, //!< No additional details
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
        AVP_TILE_CODING_CMD_LST();

        static const size_t dwSize = 7;
        static const size_t byteSize = 28;
    };

    //!
    //! \brief AVP_SURFACE_STATE
    //! \details
    //!     The AVP Pipeline is selected with the Media Instruction Opcode "8h" for
    //!     all AVP Commands. Each AVP command has assigned a media instruction
    //!     command as defined in DWord 0, BitField 22:16.
    //!     
    //!     The AVP_SURFACE_STATE command is responsible for defining the frame
    //!     buffer pitch and the offset of the chroma component.
    //!     This is a picture level state command and is shared by both encoding and
    //!     decoding processes.
    //!     For Decoder, this command is issued once per frame. That is, all pixel
    //!     buffers within the current decoding session using the same surface
    //!     definition.
    //!     For Encoder, this command is issued once per surface type. There are 3
    //!     surface types :source, reference and dst. Hence, all reference frames
    //!     are defined with the same surface command.
    //!     Tile-Yf and Tile-Ys are not supported in Gen12, but HW interface still
    //!     need to keep these bits as reserved bits.
    //!     Note : When NV12 and Tile Y are being used, full pitch and interleaved
    //!     UV is always in use. U and V Xoffset must be set to 0; U and V Yoffset
    //!     must be 16-pixel aligned. This Surface State is not the same as that of
    //!     the 3D engine and of the MFX pipeline. For 10-bit pixel, P010 surface
    //!     definition is being used.
    //!
    struct AVP_SURFACE_STATE_CMD
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
                uint32_t                 Reserved79                                       : __CODEGEN_BITFIELD(15, 26)    ; //!< Reserved
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
                uint32_t                 MemoryCompressionEnableForAv1IntraFrame          : __CODEGEN_BITFIELD( 0,  0)    ; //!< MEMORY_COMPRESSION_ENABLE_FOR_AV1_INTRA_FRAME
                uint32_t                 MemoryCompressionEnableForAv1LastFrame           : __CODEGEN_BITFIELD( 1,  1)    ; //!< MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST_FRAME
                uint32_t                 MemoryCompressionEnableForAv1Last2Frame          : __CODEGEN_BITFIELD( 2,  2)    ; //!< MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST2_FRAME
                uint32_t                 MemoryCompressionEnableForAv1Last3Frame          : __CODEGEN_BITFIELD( 3,  3)    ; //!< MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST3_FRAME
                uint32_t                 MemoryCompressionEnableForAv1GoldenFrame         : __CODEGEN_BITFIELD( 4,  4)    ; //!< MEMORY_COMPRESSION_ENABLE_FOR_AV1_GOLDEN_FRAME
                uint32_t                 MemoryCompressionEnableForAv1BwdrefFrame         : __CODEGEN_BITFIELD( 5,  5)    ; //!< MEMORY_COMPRESSION_ENABLE_FOR_AV1_BWDREF_FRAME
                uint32_t                 MemoryCompressionEnableForAv1Altref2Frame        : __CODEGEN_BITFIELD( 6,  6)    ; //!< MEMORY_COMPRESSION_ENABLE_FOR_AV1_ALTREF2_FRAME
                uint32_t                 MemoryCompressionEnableForAv1AltrefFrame         : __CODEGEN_BITFIELD( 7,  7)    ; //!< MEMORY_COMPRESSION_ENABLE_FOR_AV1_ALTREF_FRAME
                uint32_t                 CompressionTypeForIntraFrame                     : __CODEGEN_BITFIELD( 8,  8)    ; //!< COMPRESSION_TYPE_FOR_INTRA_FRAME
                uint32_t                 CompressionTypeForLastFrame                      : __CODEGEN_BITFIELD( 9,  9)    ; //!< COMPRESSION_TYPE_FOR_LAST_FRAME
                uint32_t                 CompressionTypeForLast2Frame                     : __CODEGEN_BITFIELD(10, 10)    ; //!< COMPRESSION_TYPE_FOR_LAST2_FRAME
                uint32_t                 CompressionTypeForLast3Frame                     : __CODEGEN_BITFIELD(11, 11)    ; //!< COMPRESSION_TYPE_FOR_LAST3_FRAME
                uint32_t                 CompressionTypeForGoldenFrame                    : __CODEGEN_BITFIELD(12, 12)    ; //!< COMPRESSION_TYPE_FOR_GOLDEN_FRAME
                uint32_t                 CompressionTypeForBwdrefFrame                    : __CODEGEN_BITFIELD(13, 13)    ; //!< COMPRESSION_TYPE_FOR_BWDREF_FRAME
                uint32_t                 CompressionTypeForAltref2Frame                   : __CODEGEN_BITFIELD(14, 14)    ; //!< COMPRESSION_TYPE_FOR_ALTREF2_FRAME
                uint32_t                 CompressionTypeForAltrefFrame                    : __CODEGEN_BITFIELD(15, 15)    ; //!< COMPRESSION_TYPE_FOR_ALTREF_FRAME
                uint32_t                 CompressionFormat                                : __CODEGEN_BITFIELD(16, 20)    ; //!< COMPRESSION_FORMAT
                uint32_t                 Reserved144                                      : __CODEGEN_BITFIELD(21, 31)    ; //!< Reserved
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
        //!     Codec/Engine Name = AVP = 3h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 3, //!< No additional details
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
            SURFACE_ID_AV1INTRAFRAME                                         = 6, //!< This is for AV1 Intra Frame (Reference Picture 0). Each AV1 Reference Pictures can have different size so a separate ID is needed.
            SURFACE_ID_AV1LASTFRAME                                          = 7, //!< This is for AV1Last Frame (Reference Picture 1). Each AV1 Reference Pictures can have different size so a separate ID is needed.
            SURFACE_ID_AV1LAST2FRAME                                         = 8, //!< This is for AV1 Last2 Frame (Reference Picture 2). Each AV1 Reference Pictures can have different size so a separate ID is needed.
            SURFACE_ID_AV1LAST3FRAME                                         = 9, //!< This is for AV1 Last3 Frame (Reference Picture 3). Each AV1 Reference Pictures can have different size so a separate ID is needed.
            SURFACE_ID_AV1GOLDENFRAME                                        = 10, //!< This is for AV1 Golden Frame (Reference Picture 4). Each AV1 Reference Pictures can have different size so a separate ID is needed.
            SURFACE_ID_AV1BWDREFFRAME                                        = 11, //!< This is for AV1 Bwdref Frame (Reference Picture 5). Each AV1 Reference Pictures can have different size so a separate ID is needed.
            SURFACE_ID_AV1ALTREF2FRAME                                       = 12, //!< This is for AV1 Altref2 Frame (Reference Picture 6). Each AV1 Reference Pictures can have different size so a separate ID is needed.
            SURFACE_ID_AV1ALTREFFRAME                                        = 13, //!< This is for AV1 Altref Frame (Reference Picture 7). Each AV1 Reference Pictures can have different size so a separate ID is needed.
            SURFACE_ID_INTRABCDECODEDFRAME                                   = 14, //!< This is for AV1 IntraBC Decoded Frame. It will be used both Read/Write at the same time. This surface requires multiple of 8 pixels on both width and height. 
        };

        //! \brief SURFACE_FORMAT
        //! \details
        //!     Specifies the format of the surface. This field should be ignored for
        //!     now. HW should use the definition in AVP_PIC_STATE instead. (HW team
        //!     needs to double check on this ???)
        enum SURFACE_FORMAT
        {
            SURFACE_FORMAT_P010VARIANT                                       = 3,  //!< No additional details
            SURFACE_FORMAT_PLANAR4208                                        = 4,  //!< No additional details
            SURFACE_FORMAT_P010                                              = 13, //!< No additional details
        };

        //! \brief MEMORY_COMPRESSION_ENABLE_FOR_AV1_INTRA_FRAME
        //! \details
        //!     This bit is for AV1 Intra Frame (Reference Picture 0).
        enum MEMORY_COMPRESSION_ENABLE_FOR_AV1_INTRA_FRAME
        {
            MEMORY_COMPRESSION_ENABLE_FOR_AV1_INTRA_FRAME_MEMORYCOMPRESSIONDISABLE = 0, //!< No additional details
            MEMORY_COMPRESSION_ENABLE_FOR_AV1_INTRA_FRAME_MEMORYCOMPRESSIONENABLE = 1, //!< No additional details
        };

        //! \brief MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST_FRAME
        //! \details
        //!     This bit is for AV1 Last Frame (Reference Picture 1).
        enum MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST_FRAME
        {
            MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST_FRAME_MEMORYCOMPRESSIONDISABLE = 0, //!< No additional details
            MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST_FRAME_MEMORYCOMPRESSIONENABLE = 1, //!< No additional details
        };

        //! \brief MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST2_FRAME
        //! \details
        //!     This bit is for AV1 Last2 Frame (Reference Picture 2).
        enum MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST2_FRAME
        {
            MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST2_FRAME_MEMORYCOMPRESSIONDISABLE = 0, //!< No additional details
            MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST2_FRAME_MEMORYCOMPRESSIONENABLE = 1, //!< No additional details
        };

        //! \brief MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST3_FRAME
        //! \details
        //!     This bit is for AV1 Last3 Frame (Reference Picture 3).
        enum MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST3_FRAME
        {
            MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST3_FRAME_MEMORYCOMPRESSIONDISABLE = 0, //!< No additional details
            MEMORY_COMPRESSION_ENABLE_FOR_AV1_LAST3_FRAME_MEMORYCOMPRESSIONENABLE = 1, //!< No additional details
        };

        //! \brief MEMORY_COMPRESSION_ENABLE_FOR_AV1_GOLDEN_FRAME
        //! \details
        //!     This bit is for AV1 Golden Frame (Reference Picture 4).
        enum MEMORY_COMPRESSION_ENABLE_FOR_AV1_GOLDEN_FRAME
        {
            MEMORY_COMPRESSION_ENABLE_FOR_AV1_GOLDEN_FRAME_MEMORYCOMPRESSIONDISABLE = 0, //!< No additional details
            MEMORY_COMPRESSION_ENABLE_FOR_AV1_GOLDEN_FRAME_MEMORYCOMPRESSIONENABLE = 1, //!< No additional details
        };

        //! \brief MEMORY_COMPRESSION_ENABLE_FOR_AV1_BWDREF_FRAME
        //! \details
        //!     This bit is for AV1 Bwdref Frame (Reference Picture 5).
        enum MEMORY_COMPRESSION_ENABLE_FOR_AV1_BWDREF_FRAME
        {
            MEMORY_COMPRESSION_ENABLE_FOR_AV1_BWDREF_FRAME_MEMORYCOMPRESSIONDISABLE = 0, //!< No additional details
            MEMORY_COMPRESSION_ENABLE_FOR_AV1_BWDREF_FRAME_MEMORYCOMPRESSIONENABLE = 1, //!< No additional details
        };

        //! \brief MEMORY_COMPRESSION_ENABLE_FOR_AV1_ALTREF2_FRAME
        //! \details
        //!     This bit is for AV1Altref2 Frame (Reference Picture 6).
        enum MEMORY_COMPRESSION_ENABLE_FOR_AV1_ALTREF2_FRAME
        {
            MEMORY_COMPRESSION_ENABLE_FOR_AV1_ALTREF2_FRAME_MEMORYCOMPRESSIONDISABLE = 0, //!< No additional details
            MEMORY_COMPRESSION_ENABLE_FOR_AV1_ALTREF2_FRAME_MEMORYCOMPRESSIONENABLE = 1, //!< No additional details
        };

        //! \brief MEMORY_COMPRESSION_ENABLE_FOR_AV1_ALTREF_FRAME
        //! \details
        //!     This bit is for AV1Altref Frame (Reference Picture 7).
        enum MEMORY_COMPRESSION_ENABLE_FOR_AV1_ALTREF_FRAME
        {
            MEMORY_COMPRESSION_ENABLE_FOR_AV1_ALTREF_FRAME_MEMORYCOMPRESSIONDISABLE = 0, //!< No additional details
            MEMORY_COMPRESSION_ENABLE_FOR_AV1_ALTREF_FRAME_MEMORYCOMPRESSIONENABLE = 1, //!< No additional details
        };

        //! \brief COMPRESSION_TYPE_FOR_INTRA_FRAME
        //! \details
        //!     This bit is for Intra Frame (Reference Picture 0). Valid only when
        //!     Memory Compression for Intra Frame is enabled.
        enum COMPRESSION_TYPE_FOR_INTRA_FRAME
        {
            COMPRESSION_TYPE_FOR_INTRA_FRAME_MEDIACOMPRESSIONENABLED         = 0, //!< No additional details
            COMPRESSION_TYPE_FOR_INTRA_FRAME_RENDERCOMPRESSIONENABLED        = 1, //!< No additional details
        };

        //! \brief COMPRESSION_TYPE_FOR_LAST_FRAME
        //! \details
        //!     This bit is for AV1 Last Frame (Reference Picture 1). Valid only when
        //!     Memory Compression for Last Frameis enabled.
        enum COMPRESSION_TYPE_FOR_LAST_FRAME
        {
            COMPRESSION_TYPE_FOR_LAST_FRAME_MEDIACOMPRESSIONENABLED          = 0, //!< No additional details
            COMPRESSION_TYPE_FOR_LAST_FRAME_RENDERCOMPRESSIONENABLED         = 1, //!< No additional details
        };

        //! \brief COMPRESSION_TYPE_FOR_LAST2_FRAME
        //! \details
        //!     This bit is for AV1 Last2 Frame (Reference Picture 2). Valid only when
        //!     Memory Compression for Last2 Frame is enabled.
        enum COMPRESSION_TYPE_FOR_LAST2_FRAME
        {
            COMPRESSION_TYPE_FOR_LAST2_FRAME_MEDIACOMPRESSIONENABLED         = 0, //!< No additional details
            COMPRESSION_TYPE_FOR_LAST2_FRAME_RENDERCOMPRESSIONENABLED        = 1, //!< No additional details
        };

        //! \brief COMPRESSION_TYPE_FOR_LAST3_FRAME
        //! \details
        //!     This bit is for AV1 Last3 Frame (Reference Picture 3). Valid only when
        //!     Memory Compression for Last3 Frameis enabled.
        enum COMPRESSION_TYPE_FOR_LAST3_FRAME
        {
            COMPRESSION_TYPE_FOR_LAST3_FRAME_MEDIACOMPRESSIONENABLED         = 0, //!< No additional details
            COMPRESSION_TYPE_FOR_LAST3_FRAME_RENDERCOMPRESSIONENABLED        = 1, //!< No additional details
        };

        //! \brief COMPRESSION_TYPE_FOR_GOLDEN_FRAME
        //! \details
        //!     This bit is for AV1 Golden Frame (Reference Picture 4). Valid only when
        //!     Memory Compression for Golden Frame is enabled.
        enum COMPRESSION_TYPE_FOR_GOLDEN_FRAME
        {
            COMPRESSION_TYPE_FOR_GOLDEN_FRAME_MEDIACOMPRESSIONENABLED        = 0, //!< No additional details
            COMPRESSION_TYPE_FOR_GOLDEN_FRAME_RENDERCOMPRESSIONENABLED       = 1, //!< No additional details
        };

        //! \brief COMPRESSION_TYPE_FOR_BWDREF_FRAME
        //! \details
        //!     This bit is for AV1 Bwdref Frame (Reference Picture 5). Valid only when
        //!     Memory Compression for Bwdref Frame is enabled.
        enum COMPRESSION_TYPE_FOR_BWDREF_FRAME
        {
            COMPRESSION_TYPE_FOR_BWDREF_FRAME_MEDIACOMPRESSIONENABLED        = 0, //!< No additional details
            COMPRESSION_TYPE_FOR_BWDREF_FRAME_RENDERCOMPRESSIONENABLED       = 1, //!< No additional details
        };

        //! \brief COMPRESSION_TYPE_FOR_ALTREF2_FRAME
        //! \details
        //!     This bit is for AV1 Altref2 Frame (Reference Picture 6). Valid only when
        //!     Memory Compression for Altref2 Frame is enabled.
        enum COMPRESSION_TYPE_FOR_ALTREF2_FRAME
        {
            COMPRESSION_TYPE_FOR_ALTREF2_FRAME_MEDIACOMPRESSIONENABLED       = 0, //!< No additional details
            COMPRESSION_TYPE_FOR_ALTREF2_FRAME_RENDERCOMPRESSIONENABLED      = 1, //!< No additional details
        };

        //! \brief COMPRESSION_TYPE_FOR_ALTREF_FRAME
        //! \details
        //!     This bit is for AV1 Altref Frame (Reference Picture 7). Valid only when
        //!     Memory Compression for Altref Frame is enabled.
        enum COMPRESSION_TYPE_FOR_ALTREF_FRAME
        {
            COMPRESSION_TYPE_FOR_ALTREF_FRAME_MEDIACOMPRESSIONENABLED        = 0, //!< No additional details
            COMPRESSION_TYPE_FOR_ALTREF_FRAME_RENDERCOMPRESSIONENABLED       = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        AVP_SURFACE_STATE_CMD();

        static const size_t dwSize = 5;
        static const size_t byteSize = 20;
    };

    //!
    //! \brief AVP_SEGMENT_STATE
    //! \details
    //!     When segmentation is enabled, this Segment State command is issued
    //!     onceper segment. There can be maximum 8 segments specified to decode a
    //!     given frame, so this Segment State Command can be issued maximum 8
    //!     times. It is assumed that there is no gap in segment IDs. So, when the
    //!     AVP PIC States specified that the number of active
    //!     When segmentation is disabled, driver still sends out this command once
    //!     for segment id = 0. HW needs to check the segmentation enable flag from
    //!     AVP_PIC_STATE Command as well to distinguish from the case when
    //!     segmentation is enabled for segment id = 0.
    //!     Each segment can have its own specification of enabling any one of the 8
    //!     features defined in AV1 and their corresponding feature data. When a
    //!     feature is not enabled, its feature data is defaulted to 0. When
    //!     segmentation is not enabled, all the features are disabled and their
    //!     corresponding feature data are set to 0.
    //!     style="margin:0in 0in 8pt">Segment State Command also provides other
    //!     segement related parameters.
    //!     style="margin:0in 0in 8pt">It is assumed that HW is keeping a copy of
    //!     the complete AV1 QM Matrix Tablefor all color components inside its
    //!     internal memory, and Driver only needs to send the qm_level as index
    //!     into this Table.
    //!     
    struct AVP_SEGMENT_STATE_CMD
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
                uint32_t                 SegmentFeatureMask                               : __CODEGEN_BITFIELD( 0,  7)    ; //!< Segment Feature Mask
                uint32_t                 SegmentDeltaQindex                               : __CODEGEN_BITFIELD( 8, 16)    ; //!< Segment Delta Qindex
                uint32_t                 SegmentBlockSkipFlag                             : __CODEGEN_BITFIELD(17, 17)    ; //!< Segment Block Skip Flag
                uint32_t                 SegmentBlockGlobalmvFlag                         : __CODEGEN_BITFIELD(18, 18)    ; //!< Segment Block GlobalMV Flag
                uint32_t                 SegmentLosslessFlag                              : __CODEGEN_BITFIELD(19, 19)    ; //!< Segment Lossless Flag
                uint32_t                 SegmentLumaYQmLevel                              : __CODEGEN_BITFIELD(20, 23)    ; //!< Segment Luma Y QM Level
                uint32_t                 SegmentChromaUQmLevel                            : __CODEGEN_BITFIELD(24, 27)    ; //!< Segment Chroma U QM Level
                uint32_t                 SegmentChromaVQmLevel                            : __CODEGEN_BITFIELD(28, 31)    ; //!< Segment Chroma V QM Level
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 SegmentDeltaLoopFilterLevelLumaVertical          : __CODEGEN_BITFIELD( 0,  6)    ; //!< Segment Delta Loop Filter Level Luma Vertical
                uint32_t                 SegmentDeltaLoopFilterLevelLumaHorizontal        : __CODEGEN_BITFIELD( 7, 13)    ; //!< Segment Delta Loop Filter Level Luma Horizontal
                uint32_t                 SegmentDeltaLoopFilterLevelChromaU               : __CODEGEN_BITFIELD(14, 20)    ; //!< Segment Delta Loop Filter Level Chroma U
                uint32_t                 SegmentDeltaLoopFilterLevelChromaV               : __CODEGEN_BITFIELD(21, 27)    ; //!< Segment Delta Loop Filter Level Chroma V
                uint32_t                 SegmentReferenceFrame                            : __CODEGEN_BITFIELD(28, 30)    ; //!< Segment Reference Frame
                uint32_t                 Reserved127                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_AVPSEGMENTSTATE                        = 50, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = AVP = 3h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 3, //!< No additional details
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
        AVP_SEGMENT_STATE_CMD();

        static const size_t dwSize = 4;
        static const size_t byteSize = 16;
    };

    //!
    //! \brief AVP_PIPE_BUF_ADDR_STATE
    //! \details
    //!     The AVP Pipeline is selected with the Media Instruction Opcode "8h" for
    //!     all AVP Commands. Each AVP command has assigned a media instruction
    //!     command as defined in DWord 0, BitField 22:16.
    //!     
    //!     This state command provides the physical memory base addresses for all
    //!     row store buffers, column store buffers (for tile column and in
    //!     scalability mode), reconstructed output and reference frame buffers, and
    //!     auxiliary data buffers (MV, segment map, etc.) that are required by the
    //!     AV1 decoding and encoding process.
    //!     This is a frame level state command and is shared by both encoding and
    //!     decoding processes.
    //!     AVP is a tile based pipeline and is a stateless pipeline, hence all
    //!     sequence level, frame level, and segment level state commands must be
    //!     resent to process each tile.
    //!     Memory compression may be applicable to some of these buffers for BW
    //!     saving.
    //!     Note : there is no buffer to store the 16 QM table sets, they are
    //!     implemented directly inside the HW pipeline.
    //!     
    struct AVP_PIPE_BUF_ADDR_STATE_CMD
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
        SPLITBASEADDRESS64BYTEALIGNED_CMD        ReferenceFrameBufferBaseAddressRefaddr07[8];                             //!< DW1..16, Reference Frame Buffer Base Address (RefAddr[0-7])
        MEMORYADDRESSATTRIBUTES_CMD              ReferenceFrameBufferBaseAddressAttributes;                               //!< DW17, Reference Frame Buffer Base Address Attributes
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        DecodedOutputFrameBufferAddress;                                         //!< DW18..19, Decoded Output Frame Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              DecodedOutputFrameBufferAddressAttributes;                               //!< DW20, Decoded Output Frame Buffer Address Attributes
        uint32_t                                 Reserved672[3];                                                          //!< Reserved
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        IntrabcDecodedOutputFrameBufferAddress;                                  //!< DW24..25, IntraBC Decoded Output Frame Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              IntrabcDecodedOutputFrameBufferAddressAttributes;                        //!< DW26, IntraBC Decoded Output Frame Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        CdfTablesInitializationBufferAddress;                                    //!< DW27..28, CDF Tables Initialization Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              CdfTablesInitializationBufferAddressAttributes;                          //!< DW29, CDF Tables Initialization Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        CdfTablesBackwardAdaptationBufferAddress;                                //!< DW30..31, CDF Tables Backward Adaptation Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              CdfTablesBackwardAdaptationBufferAddressAttributes;                      //!< DW32, CDF Tables Backward Adaptation Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        Av1SegmentIdReadBufferAddress;                                           //!< DW33..34, AV1 Segment ID Read Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              Av1SegmentIdReadBufferAddressAttributes;                                 //!< DW35, AV1 Segment ID Read Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        Av1SegmentIdWriteBufferAddress;                                          //!< DW36..37, AV1 Segment ID Write Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              Av1SegmentIdWriteBufferAddressAttributes;                                //!< DW38, AV1 Segment ID Write Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        CollocatedMotionVectorTemporalBufferBaseAddressTmvaddr07[8];             //!< DW39..54, Collocated Motion Vector Temporal Buffer Base Address (TmvAddr[0-7])
        MEMORYADDRESSATTRIBUTES_CMD              CollocatedMotionVectorTemporalBufferBaseAddressAttributes;               //!< DW55, Collocated Motion Vector Temporal Buffer Base Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        CurrentFrameMotionVectorWriteBufferAddress;                              //!< DW56..57, Current Frame Motion Vector Write Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              CurrentFrameMotionVectorWriteBufferAddressAttributes;                    //!< DW58, Current Frame Motion Vector Write Buffer Address Attributes
        uint32_t                                 Reserved1888[3];                                                         //!< Reserved
        SPLITBASEADDRESS64BYTEALIGNED_CMD        BitstreamDecoderEncoderLineRowstoreReadWriteBufferAddress;               //!< DW62..63, Bitstream Decoder/Encoder Line Rowstore Read/Write Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              BitstreamDecoderEncoderLineRowstoreReadWriteBufferAddressAttributes;     //!< DW64, Bitstream Decoder/Encoder Line Rowstore Read/Write Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        BitstreamDecoderEncoderTileLineRowstoreReadWriteBufferAddress;           //!< DW65..66, Bitstream Decoder/Encoder Tile Line Rowstore Read/Write Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              BitstreamDecoderEncoderTileLineRowstoreReadWriteBufferAddressAttributes; //!< DW67, Bitstream Decoder/Encoder Tile Line Rowstore Read/Write Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        IntraPredictionLineRowstoreReadWriteBufferAddress;                       //!< DW68..69, Intra Prediction Line Rowstore Read/Write Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              IntraPredictionLineRowstoreReadWriteBufferAddressAttributes;             //!< DW70, Intra Prediction Line Rowstore Read/Write Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        IntraPredictionTileLineRowstoreReadWriteBufferAddress;                   //!< DW71..72, Intra Prediction Tile Line Rowstore Read/Write Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              IntraPredictionTileLineRowstoreReadWriteBufferAddressAttributes;         //!< DW73, Intra Prediction Tile Line Rowstore Read/Write Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        SpatialMotionVectorLineReadWriteBufferAddress;                           //!< DW74..75, Spatial Motion Vector Line Read/Write Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              SpatialMotionVectorLineReadWriteBufferAddressAttributes;                 //!< DW76, Spatial Motion Vector Line Read/Write Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        SpatialMotionVectorCodingTileLineReadWriteBufferAddress;                 //!< DW77..78, Spatial Motion Vector Coding Tile Line Read/Write Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              SpatialMotionVectorTileLineReadWriteBufferAddressAttributes;             //!< DW79, Spatial Motion Vector Tile Line Read/Write Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        LoopRestorationMetaTileColumnReadWriteBufferAddress;                     //!< DW80..81, Loop Restoration Meta Tile Column Read/Write Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              LoopRestorationMetaTileColumnReadWriteBufferAddressAttributes;           //!< DW82, Loop Restoration Meta Tile Column Read/Write Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        LoopRestorationFilterTileReadWriteLineYBufferAddress;                    //!< DW83..84, Loop Restoration Filter Tile Read/Write Line Y Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              LoopRestorationFilterTileReadWriteLineYBufferAddressAttributes;          //!< DW85, Loop Restoration Filter Tile Read/Write Line Y Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        LoopRestorationFilterTileReadWriteLineUBufferAddress;                    //!< DW86..87, Loop Restoration Filter Tile Read/Write Line U Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              LoopRestorationFilterTileReadWriteLineUBufferAddressAttributes;          //!< DW88, Loop Restoration Filter Tile Read/Write Line U Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        LoopRestorationFilterTileReadWriteLineVBufferAddress;                    //!< DW89..90, Loop Restoration Filter Tile Read/Write Line V Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              LoopRestorationFilterTileReadWriteLineVBufferAddressAttributes;          //!< DW91, Loop Restoration Filter Tile Read/Write Line V Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        DeblockerFilterLineReadWriteYBufferAddress;                              //!< DW92..93, Deblocker Filter Line Read/Write Y Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              DeblockerFilterLineReadWriteYBufferAddressAttributes;                    //!< DW94, Deblocker Filter Line Read/Write Y Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        DeblockerFilterLineReadWriteUBufferAddress;                              //!< DW95..96, Deblocker Filter Line Read/Write U Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              DeblockerFilterLineReadWriteUBufferAddressAttributes;                    //!< DW97, Deblocker Filter Line Read/Write U Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        DeblockerFilterLineReadWriteVBufferAddress;                              //!< DW98..99, Deblocker Filter Line Read/Write V Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              DeblockerFilterLineReadWriteVBufferAddressAttributes;                    //!< DW100, Deblocker Filter Line Read/Write V Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        DeblockerFilterTileLineReadWriteYBufferAddress;                          //!< DW101..102, Deblocker Filter Tile Line Read/Write Y Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              DeblockerFilterTileLineReadWriteYBufferAddressAttributes;                //!< DW103, Deblocker Filter Tile Line Read/Write Y Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        DeblockerFilterTileLineReadWriteVBufferAddress;                          //!< DW104..105, Deblocker Filter Tile Line Read/Write V Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              DeblockerFilterTileLineReadWriteVBufferAddressAttributes;                //!< DW106, Deblocker Filter Tile Line Read/Write V Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        DeblockerFilterTileLineReadWriteUBufferAddress;                          //!< DW107..108, Deblocker Filter Tile Line Read/Write U Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              DeblockerFilterTileLineReadWriteUBufferAddressAttributes;                //!< DW109, Deblocker Filter Tile Line Read/Write U Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        DeblockerFilterTileColumnReadWriteYBufferAddress;                        //!< DW110..111, Deblocker Filter Tile Column Read/Write Y Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              DeblockerFilterTileColumnReadWriteYBufferAddressAttributes;              //!< DW112, Deblocker Filter Tile Column Read/Write Y Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        DeblockerFilterTileColumnReadWriteUBufferAddress;                        //!< DW113..114, Deblocker Filter Tile Column Read/Write U Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              DeblockerFilterTileColumnReadWriteUBufferAddressAttributes;              //!< DW115, Deblocker Filter Tile Column Read/Write U Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        DeblockerFilterTileColumnReadWriteVBufferAddress;                        //!< DW116..117, Deblocker Filter Tile Column Read/Write V Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              DeblockerFilterTileColumnReadWriteVBufferAddressAttributes;              //!< DW118, Deblocker Filter Tile Column Read/Write V Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        CdefFilterLineReadWriteBufferAddress;                                    //!< DW119..120, CDEF Filter Line Read/Write Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              CdefFilterLineReadWriteBufferAddressAttributes;                          //!< DW121, CDEF Filter Line Read/Write Buffer Address Attributes
        uint32_t                                 Reserved3904[6];                                                         //!< Reserved
        SPLITBASEADDRESS64BYTEALIGNED_CMD        CdefFilterTileLineReadWriteBufferAddress;                                //!< DW128..129, CDEF Filter Tile Line Read/Write Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              CdefFilterTileLineReadWriteBufferAddressAttributes;                      //!< DW130, CDEF Filter Tile Line Read/Write Buffer Address Attributes
        uint32_t                                 Reserved4192[6];                                                         //!< Reserved
        SPLITBASEADDRESS64BYTEALIGNED_CMD        CdefFilterTileColumnReadWriteBufferAddress;                              //!< DW137..138, CDEF Filter Tile Column Read/Write Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              CdefFilterTileColumnReadWriteBufferAddressAttributes;                    //!< DW139, CDEF Filter Tile Column Read/Write Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        CdefFilterMetaTileLineReadWriteBufferAddress;                            //!< DW140..141, CDEF Filter Meta Tile Line Read/Write Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              CdefFilterMetaTileLineReadWriteBufferAddressAttributes;                  //!< DW142, CDEF Filter Meta Tile Line Read/Write Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        CdefFilterMetaTileColumnReadWriteBufferAddress;                          //!< DW143..144, CDEF Filter Meta Tile Column Read/Write Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              CdefFilterMetaTileColumnReadWriteBufferAddressAttributes;                //!< DW145, CDEF Filter Meta Tile Column Read/Write Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        CdefFilterTopLeftCornerReadWriteBufferAddress;                           //!< DW146..147, CDEF Filter Top-Left Corner Read/Write Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              CdefFilterTopLeftCornerReadWriteBufferAddressAttributes;                 //!< DW148, CDEF Filter Top-Left Corner Read/Write Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        SuperResTileColumnReadWriteYBufferAddress;                               //!< DW149..150, Super-Res Tile Column Read/Write Y Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              SuperResTileColumnReadWriteYBufferAddressAttributes;                     //!< DW151, Super-Res Tile Column Read/Write Y Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        SuperResTileColumnReadWriteUBufferAddress;                               //!< DW152..153, Super-Res Tile Column Read/Write U Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              SuperResTileColumnReadWriteUBufferAddressAttributes;                     //!< DW154, Super-Res Tile Column Read/Write U Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        SuperResTileColumnReadWriteVBufferAddress;                               //!< DW155..156, Super-Res Tile Column Read/Write V Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              SuperResTileColumnReadWriteVBufferAddressAttributes;                     //!< DW157, Super-Res Tile Column Read/Write V Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        LoopRestorationFilterTileColumnReadWriteYBufferAddress;                  //!< DW158..159, Loop Restoration Filter Tile Column Read/Write Y Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              LoopRestorationFilterTileColumnReadWriteYBufferAddressAttributes;        //!< DW160, Loop Restoration Filter Tile Column Read/Write Y Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        LoopRestorationFilterTileColumnReadWriteUBufferAddress;                  //!< DW161..162, Loop Restoration Filter Tile Column Read/Write U Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              LoopRestorationFilterTileColumnReadWriteUBufferAddressAttributes;        //!< DW163, Loop Restoration Filter Tile Column Read/Write U Buffer Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        LoopRestorationFilterTileColumnReadWriteVBufferAddress;                  //!< DW164..165, Loop Restoration Filter Tile Column Read/Write V Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              LoopRestorationFilterTileColumnReadWriteVBufferAddressAttributes;        //!< DW166, Loop Restoration Filter Tile Column Read/Write V Buffer Address Attributes
        uint32_t                                 Reserved5344[9];                                                         //!< Reserved
        SPLITBASEADDRESS64BYTEALIGNED_CMD        DecodedFrameStatusErrorBufferBaseAddress;                                //!< DW176..177, Decoded Frame Status/Error Buffer Base Address
        MEMORYADDRESSATTRIBUTES_CMD              DecodedFrameStatusErrorBufferBaseAddressAttributes;                      //!< DW178, Decoded Frame Status/Error Buffer Base Address Attributes
        SPLITBASEADDRESS64BYTEALIGNED_CMD        DecodedBlockDataStreamoutBufferAddress;                                  //!< DW179..180, Decoded Block Data Streamout Buffer Address
        MEMORYADDRESSATTRIBUTES_CMD              DecodedBlockDataStreamoutBufferAddressAttributes;                        //!< DW181, Decoded Block Data Streamout Buffer Address Attributes
        uint32_t                                 Reserved5824[3];                                                         //!< Reserved
        uint32_t                                 Reserved5920[3];                                                         //!< Reserved

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_AVPPIPEBUFADDRSTATE                    = 2, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = AVP = 3h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 3, //!< No additional details
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
        AVP_PIPE_BUF_ADDR_STATE_CMD();

        static const size_t dwSize = 188;
        static const size_t byteSize = 752;
    };

    //!
    //! \brief AVP_INLOOP_FILTER_STATE
    //! \details
    //!     The AVP_INLOOP_FILTER_STATE command provides all the frame level syntax
    //!     elements and derived parameters that are needed for the processing of
    //!     all the post in-loop filters present in the AV1 codec. This includes the
    //!     Deblocker, the CDEF (Constrained Directional Enhancement Filter), the
    //!     HSRF (Horizontal-only Super-Resolution Filter), and the LRF (Loop
    //!     Restoration Filter). These syntax elements can be changed in the
    //!     bitstream from frame to frame.
    //!     All Post In-Loop Filters are inherently frame-based filtering, but when
    //!     implemented in a HW pipeline, the filtering process is performed in a
    //!     block by block fashion. In the addition to these frame level states,
    //!     there are additional syntax elements and derived parameters that are
    //!     generated at SuperBlock level, and are not described here.
    //!     Each of these 4 Post In-Loop Filters can be controlled independently and
    //!     each can be enabled or disabled independently. Except the HSRF, all the
    //!     other 3 filters have separate controls for each color plane as well. To
    //!     disable a Post In-Loop Filter, its control parameter(s) are set to 0 -
    //!     the default state.
    //!     This command should be issued once per tile, even if no post in-loop
    //!     filter is enabled for decoding the current frame. When in frame lossless
    //!     mode or when IntraBC is enabled, all the Post In-Loop Filters are
    //!     disabled for all color planes, this command will provide the default
    //!     values for all parameters. All syntax elements are then assumed a value
    //!     of 0, except otherwise specified in each field of this State Command.
    //!     When it is in monochrome video, no filter parameter for the two chroma
    //!     planes is present in the bitstream.
    //!     
    struct AVP_INLOOP_FILTER_STATE_CMD
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
                uint32_t                 LumaYDeblockerFilterLevelVertical                : __CODEGEN_BITFIELD( 0,  5)    ; //!< Luma Y Deblocker Filter Level Vertical
                uint32_t                 LumaYDeblockerFilterLevelHorizontal              : __CODEGEN_BITFIELD( 6, 11)    ; //!< Luma Y Deblocker Filter Level Horizontal
                uint32_t                 ChromaUDeblockerFilterLevel                      : __CODEGEN_BITFIELD(12, 17)    ; //!< Chroma U Deblocker Filter Level
                uint32_t                 ChromaVDeblockerFilterLevel                      : __CODEGEN_BITFIELD(18, 23)    ; //!< Chroma V Deblocker Filter Level
                uint32_t                 DeblockerFilterSharpnessLevel                    : __CODEGEN_BITFIELD(24, 26)    ; //!< Deblocker Filter Sharpness Level
                uint32_t                 DeblockerFilterModeRefDeltaEnableFlag            : __CODEGEN_BITFIELD(27, 27)    ; //!< Deblocker Filter Mode Ref Delta Enable Flag
                uint32_t                 DeblockerDeltaLfResolution                       : __CODEGEN_BITFIELD(28, 29)    ; //!< Deblocker Delta LF Resolution
                uint32_t                 DeblockerFilterDeltaLfMultiFlag                  : __CODEGEN_BITFIELD(30, 30)    ; //!< Deblocker Filter Delta LF Multi Flag
                uint32_t                 DeblockerFilterDeltaLfPresentFlag                : __CODEGEN_BITFIELD(31, 31)    ; //!< Deblocker Filter Delta LF Present Flag
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 DeblockerFilterRefDeltas0                        : __CODEGEN_BITFIELD( 0,  6)    ; //!< Deblocker Filter Ref Deltas[0]
                uint32_t                 Reserved71                                       : __CODEGEN_BITFIELD( 7,  7)    ; //!< Reserved
                uint32_t                 DeblockerFilterRefDeltas1                        : __CODEGEN_BITFIELD( 8, 14)    ; //!< Deblocker Filter Ref Deltas[1]
                uint32_t                 Reserved79                                       : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 DeblockerFilterRefDeltas2                        : __CODEGEN_BITFIELD(16, 22)    ; //!< Deblocker Filter Ref Deltas[2]
                uint32_t                 Reserved87                                       : __CODEGEN_BITFIELD(23, 23)    ; //!< Reserved
                uint32_t                 DeblockerFilterRefDeltas3                        : __CODEGEN_BITFIELD(24, 30)    ; //!< Deblocker Filter Ref Deltas[3]
                uint32_t                 Reserved95                                       : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 DeblockerFilterRefDeltas4                        : __CODEGEN_BITFIELD( 0,  6)    ; //!< Deblocker Filter Ref Deltas[4]
                uint32_t                 Reserved103                                      : __CODEGEN_BITFIELD( 7,  7)    ; //!< Reserved
                uint32_t                 DeblockerFilterRefDeltas5                        : __CODEGEN_BITFIELD( 8, 14)    ; //!< Deblocker Filter Ref Deltas[5]
                uint32_t                 Reserved111                                      : __CODEGEN_BITFIELD(15, 15)    ; //!< Reserved
                uint32_t                 DeblockerFilterRefDeltas6                        : __CODEGEN_BITFIELD(16, 22)    ; //!< Deblocker Filter Ref Deltas[6]
                uint32_t                 Reserved119                                      : __CODEGEN_BITFIELD(23, 23)    ; //!< Reserved
                uint32_t                 DeblockerFilterRefDeltas7                        : __CODEGEN_BITFIELD(24, 30)    ; //!< Deblocker Filter Ref Deltas[7]
                uint32_t                 Reserved127                                      : __CODEGEN_BITFIELD(31, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 DeblockerFilterModeDeltas0                       : __CODEGEN_BITFIELD( 0,  6)    ; //!< Deblocker Filter Mode Deltas[0]
                uint32_t                 Reserved135                                      : __CODEGEN_BITFIELD( 7,  7)    ; //!< Reserved
                uint32_t                 DeblockerFilterModeDeltas1                       : __CODEGEN_BITFIELD( 8, 14)    ; //!< Deblocker Filter Mode Deltas[1]
                uint32_t                 Reserved143                                      : __CODEGEN_BITFIELD(15, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 CdefYStrength0                                   : __CODEGEN_BITFIELD( 0,  5)    ; //!< CDEF Y Strength[0]
                uint32_t                 CdefYStrength1                                   : __CODEGEN_BITFIELD( 6, 11)    ; //!< CDEF Y Strength[1]
                uint32_t                 CdefYStrength2                                   : __CODEGEN_BITFIELD(12, 17)    ; //!< CDEF Y Strength[2]
                uint32_t                 CdefYStrength3                                   : __CODEGEN_BITFIELD(18, 23)    ; //!< CDEF Y Strength[3]
                uint32_t                 Reserved184                                      : __CODEGEN_BITFIELD(24, 27)    ; //!< Reserved
                uint32_t                 CdefBits                                         : __CODEGEN_BITFIELD(28, 29)    ; //!< CDEF Bits
                uint32_t                 CdefFilterDampingFactorMinus3                    : __CODEGEN_BITFIELD(30, 31)    ; //!< CDEF Filter Damping Factor Minus3
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 CdefYStrength4                                   : __CODEGEN_BITFIELD( 0,  5)    ; //!< CDEF Y Strength[4]
                uint32_t                 CdefYStrength5                                   : __CODEGEN_BITFIELD( 6, 11)    ; //!< CDEF Y Strength[5]
                uint32_t                 CdefYStrength6                                   : __CODEGEN_BITFIELD(12, 17)    ; //!< CDEF Y Strength[6]
                uint32_t                 CdefYStrength7                                   : __CODEGEN_BITFIELD(18, 23)    ; //!< CDEF Y Strength[7]
                uint32_t                 Reserved216                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            struct
            {
                uint32_t                 CdefUvStrength0                                  : __CODEGEN_BITFIELD( 0,  5)    ; //!< CDEF UV Strength[0]
                uint32_t                 CdefUvStrength1                                  : __CODEGEN_BITFIELD( 6, 11)    ; //!< CDEF UV Strength[1]
                uint32_t                 CdefUvStrength2                                  : __CODEGEN_BITFIELD(12, 17)    ; //!< CDEF UV Strength[2]
                uint32_t                 CdefUvStrength3                                  : __CODEGEN_BITFIELD(18, 23)    ; //!< CDEF UV Strength[3]
                uint32_t                 Reserved248                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            struct
            {
                uint32_t                 CdefUvStrength4                                  : __CODEGEN_BITFIELD( 0,  5)    ; //!< CDEF UV Strength[4]
                uint32_t                 CdefUvStrength5                                  : __CODEGEN_BITFIELD( 6, 11)    ; //!< CDEF UV Strength[5]
                uint32_t                 CdefUvStrength6                                  : __CODEGEN_BITFIELD(12, 17)    ; //!< CDEF UV Strength[6]
                uint32_t                 CdefUvStrength7                                  : __CODEGEN_BITFIELD(18, 23)    ; //!< CDEF UV Strength[7]
                uint32_t                 Reserved280                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            struct
            {
                uint32_t                 SuperResUpscaledFrameWidthMinus1                 : __CODEGEN_BITFIELD( 0, 15)    ; //!< Super-Res Upscaled Frame Width Minus1
                uint32_t                 SuperResDenom                                    : __CODEGEN_BITFIELD(16, 20)    ; //!< Super-Res Denom
                uint32_t                 HorzSuperResEnableFlag                           : __CODEGEN_BITFIELD(21, 21)    ; //!< Horz Super-Res Enable Flag
                uint32_t                 Reserved310                                      : __CODEGEN_BITFIELD(22, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            struct
            {
                uint32_t                 FrameLoopRestorationFilterTypeForLumaY           : __CODEGEN_BITFIELD( 0,  1)    ; //!< Frame Loop Restoration Filter Type for Luma Y
                uint32_t                 FrameLoopRestorationFilterTypeForChromaU         : __CODEGEN_BITFIELD( 2,  3)    ; //!< Frame Loop Restoration Filter Type for Chroma U
                uint32_t                 FrameLoopRestorationFilterTypeForChromaV         : __CODEGEN_BITFIELD( 4,  5)    ; //!< Frame Loop Restoration Filter Type for Chroma V
                uint32_t                 Reserved326                                      : __CODEGEN_BITFIELD( 6,  7)    ; //!< Reserved
                uint32_t                 LoopRestorationUnitSizeForLumaY                  : __CODEGEN_BITFIELD( 8,  9)    ; //!< Loop Restoration Unit Size for Luma Y
                uint32_t                 UseSameLoopRestorationUnitSizeForChromasUvFlag   : __CODEGEN_BITFIELD(10, 10)    ; //!< Use Same Loop Restoration Unit Size for Chromas UV Flag
                uint32_t                 Reserved331                                      : __CODEGEN_BITFIELD(11, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW10;

        union
        {
            struct
            {
                uint32_t                 LumaPlaneXStepQn                                 : __CODEGEN_BITFIELD( 0, 15)    ; //!< Luma Plane x_step_qn
                uint32_t                 Reserved368                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved (for higher precision of x_step_qn)
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            struct
            {
                uint32_t                 LumaPlaneX0Qn                                    : __CODEGEN_BITFIELD( 0, 31)    ; //!< Luma Plane x0_qn
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            struct
            {
                uint32_t                 ChromaPlaneXStepQn                               : __CODEGEN_BITFIELD( 0, 15)    ; //!< Chroma Plane x_step_qn
                uint32_t                 Reserved368                                      : __CODEGEN_BITFIELD(16, 31)    ; //!< Reserved (for higher precision of x_step_qn)
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            struct
            {
                uint32_t                 ChromaPlaneX0Qn                                  : __CODEGEN_BITFIELD( 0, 31)    ; //!< Chroma Plane x0_qn
            };
            uint32_t                     Value;
        } DW14;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_AVPINLOOPFILTERSTATE                   = 51, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = AV1 = 3h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 3, //!< No additional details
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
        AVP_INLOOP_FILTER_STATE_CMD();

        static const size_t dwSize = 15;
        static const size_t byteSize = 60;
    };

    //!
    //! \brief AVP_INTER_PRED_STATE
    //! \details
    //!     The AVP Pipeline is selected with the Media Instruction Opcode "8h" for
    //!     all AVP Commands. Each AVP command has assigned a media instruction
    //!     command as defined in DWord 0, BitField 22:16.
    //!     AVP supports a 8-reference frames display buffer. But at any given frame
    //!     being decoded, only up to 7reference frames out of the 8 can be active.
    //!     There are also further constraints on which of these 7frames can be used
    //!     for forward and backward reference in the compound mode.
    //!     For each inter-codedtile, this command is issued only once.
    //!     
    struct AVP_INTER_PRED_STATE_CMD
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
                uint32_t                 SavedOrderHintsForAllReferences00                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Saved Order Hints for All References[0][0]
                uint32_t                 SavedOrderHintsForAllReferences01                : __CODEGEN_BITFIELD( 8, 15)    ; //!< Saved Order Hints for All References[0][1]
                uint32_t                 SavedOrderHintsForAllReferences02                : __CODEGEN_BITFIELD(16, 23)    ; //!< Saved Order Hints for All References[0][2]
                uint32_t                 SavedOrderHintsForAllReferences03                : __CODEGEN_BITFIELD(24, 31)    ; //!< Saved Order Hints for All References[0][3]
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 SavedOrderHintsForAllReferences04                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Saved Order Hints for All References[0][4]
                uint32_t                 SavedOrderHintsForAllReferences05                : __CODEGEN_BITFIELD( 8, 15)    ; //!< Saved Order Hints for All References[0][5]
                uint32_t                 SavedOrderHintsForAllReferences06                : __CODEGEN_BITFIELD(16, 23)    ; //!< Saved Order Hints for All References[0][6]
                uint32_t                 ActiveReferenceBitmaskForMotionFieldProjection   : __CODEGEN_BITFIELD(24, 31)    ; //!< Active Reference Bitmask for Motion Field Projection
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 SavedOrderHintsForAllReferences10                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Saved Order Hints for All References[1][0]
                uint32_t                 SavedOrderHintsForAllReferences11                : __CODEGEN_BITFIELD( 8, 15)    ; //!< Saved Order Hints for All References[1][1]
                uint32_t                 SavedOrderHintsForAllReferences12                : __CODEGEN_BITFIELD(16, 23)    ; //!< Saved Order Hints for All References[1][2]
                uint32_t                 SavedOrderHintsForAllReferences13                : __CODEGEN_BITFIELD(24, 31)    ; //!< Saved Order Hints for All References[1][3]
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 SavedOrderHintsForAllReferences14                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Saved Order Hints for All References[1][4]
                uint32_t                 SavedOrderHintsForAllReferences15                : __CODEGEN_BITFIELD( 8, 15)    ; //!< Saved Order Hints for All References[1][5]
                uint32_t                 SavedOrderHintsForAllReferences16                : __CODEGEN_BITFIELD(16, 23)    ; //!< Saved Order Hints for All References[1][6]
                uint32_t                 Reserved152                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 SavedOrderHintsForAllReferences20                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Saved Order Hints for All References[2][0]
                uint32_t                 SavedOrderHintsForAllReferences21                : __CODEGEN_BITFIELD( 8, 15)    ; //!< Saved Order Hints for All References[2][1]
                uint32_t                 SavedOrderHintsForAllReferences22                : __CODEGEN_BITFIELD(16, 23)    ; //!< Saved Order Hints for All References[2][2]
                uint32_t                 SavedOrderHintsForAllReferences23                : __CODEGEN_BITFIELD(24, 31)    ; //!< Saved Order Hints for All References[2][3]
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            struct
            {
                uint32_t                 SavedOrderHintsForAllReferences24                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Saved Order Hints for All References[2][4]
                uint32_t                 SavedOrderHintsForAllReferences25                : __CODEGEN_BITFIELD( 8, 15)    ; //!< Saved Order Hints for All References[2][5]
                uint32_t                 SavedOrderHintsForAllReferences26                : __CODEGEN_BITFIELD(16, 23)    ; //!< Saved Order Hints for All References[2][6]
                uint32_t                 Reserved216                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            struct
            {
                uint32_t                 SavedOrderHintsForAllReferences30                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Saved Order Hints for All References[3][0]
                uint32_t                 SavedOrderHintsForAllReferences31                : __CODEGEN_BITFIELD( 8, 15)    ; //!< Saved Order Hints for All References[3][1]
                uint32_t                 SavedOrderHintsForAllReferences32                : __CODEGEN_BITFIELD(16, 23)    ; //!< Saved Order Hints for All References[3][2]
                uint32_t                 SavedOrderHintsForAllReferences33                : __CODEGEN_BITFIELD(24, 31)    ; //!< Saved Order Hints for All References[3][3]
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            struct
            {
                uint32_t                 SavedOrderHintsForAllReferences34                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Saved Order Hints for All References[3][4]
                uint32_t                 SavedOrderHintsForAllReferences35                : __CODEGEN_BITFIELD( 8, 15)    ; //!< Saved Order Hints for All References[3][5]
                uint32_t                 SavedOrderHintsForAllReferences36                : __CODEGEN_BITFIELD(16, 23)    ; //!< Saved Order Hints for All References[3][6]
                uint32_t                 Reserved280                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            struct
            {
                uint32_t                 SavedOrderHintsForAllReferences40                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Saved Order Hints for All References[4][0]
                uint32_t                 SavedOrderHintsForAllReferences41                : __CODEGEN_BITFIELD( 8, 15)    ; //!< Saved Order Hints for All References[4][1]
                uint32_t                 SavedOrderHintsForAllReferences42                : __CODEGEN_BITFIELD(16, 23)    ; //!< Saved Order Hints for All References[4][2]
                uint32_t                 SavedOrderHintsForAllReferences43                : __CODEGEN_BITFIELD(24, 31)    ; //!< Saved Order Hints for All References[4][3]
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            struct
            {
                uint32_t                 SavedOrderHintsForAllReferences44                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Saved Order Hints for All References[4][4]
                uint32_t                 SavedOrderHintsForAllReferences45                : __CODEGEN_BITFIELD( 8, 15)    ; //!< Saved Order Hints for All References[4][5]
                uint32_t                 SavedOrderHintsForAllReferences46                : __CODEGEN_BITFIELD(16, 23)    ; //!< Saved Order Hints for All References[4][6]
                uint32_t                 Reserved344                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            struct
            {
                uint32_t                 SavedOrderHintsForAllReferences50                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Saved Order Hints for All References[5][0]
                uint32_t                 SavedOrderHintsForAllReferences51                : __CODEGEN_BITFIELD( 8, 15)    ; //!< Saved Order Hints for All References[5][1]
                uint32_t                 SavedOrderHintsForAllReferences52                : __CODEGEN_BITFIELD(16, 23)    ; //!< Saved Order Hints for All References[5][2]
                uint32_t                 SavedOrderHintsForAllReferences53                : __CODEGEN_BITFIELD(24, 31)    ; //!< Saved Order Hints for All References[5][3]
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            struct
            {
                uint32_t                 SavedOrderHintsForAllReferences54                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Saved Order Hints for All References[5][4]
                uint32_t                 SavedOrderHintsForAllReferences55                : __CODEGEN_BITFIELD( 8, 15)    ; //!< Saved Order Hints for All References[5][5]
                uint32_t                 SavedOrderHintsForAllReferences56                : __CODEGEN_BITFIELD(16, 23)    ; //!< Saved Order Hints for All References[5][6]
                uint32_t                 Reserved408                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            struct
            {
                uint32_t                 SavedOrderHintsForAllReferences60                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Saved Order Hints for All References[6][0]
                uint32_t                 SavedOrderHintsForAllReferences61                : __CODEGEN_BITFIELD( 8, 15)    ; //!< Saved Order Hints for All References[6][1]
                uint32_t                 SavedOrderHintsForAllReferences62                : __CODEGEN_BITFIELD(16, 23)    ; //!< Saved Order Hints for All References[6][2]
                uint32_t                 SavedOrderHintsForAllReferences63                : __CODEGEN_BITFIELD(24, 31)    ; //!< Saved Order Hints for All References[6][3]
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            struct
            {
                uint32_t                 SavedOrderHintsForAllReferences64                : __CODEGEN_BITFIELD( 0,  7)    ; //!< Saved Order Hints for All References[6][4]
                uint32_t                 SavedOrderHintsForAllReferences65                : __CODEGEN_BITFIELD( 8, 15)    ; //!< Saved Order Hints for All References[6][5]
                uint32_t                 SavedOrderHintsForAllReferences66                : __CODEGEN_BITFIELD(16, 23)    ; //!< Saved Order Hints for All References[6][6]
                uint32_t                 Reserved472                                      : __CODEGEN_BITFIELD(24, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW14;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_AVPINTERPREDSTATE                      = 18, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = AVP = 3h
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 3, //!< No additional details
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
        AVP_INTER_PRED_STATE_CMD();

        static const size_t dwSize = 15;
        static const size_t byteSize = 60;
    };
};

#pragma pack()

#endif  // __MHW_AVP_HWCMD_G12_H__