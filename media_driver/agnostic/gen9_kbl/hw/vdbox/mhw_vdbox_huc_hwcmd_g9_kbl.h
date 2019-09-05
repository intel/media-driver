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
//! \file     mhw_vdbox_huc_hwcmd_g9_kbl.h
//! \brief    Auto-generated constructors for MHW and states.
//! \details  This file may not be included outside of g9_kbl as other components
//!           should use MHW interface to interact with MHW commands and states.
//!

// DO NOT EDIT

#ifndef __MHW_VDBOX_HUC_HWCMD_G9_KBL_H__
#define __MHW_VDBOX_HUC_HWCMD_G9_KBL_H__

#pragma once
#pragma pack(1)

#include <cstdint>
#include <cstddef>

class mhw_vdbox_huc_g9_kbl
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
        //!     vol1a Memory Data Formats chapter - section media Memory
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
    //! \brief HUC_PIPE_MODE_SELECT
    //! \details
    //!     The HUC is selected with the Media Instruction Opcode "Bh" for all HUC
    //!     Commands. Each HUC command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    //!     The HUC_PIPE_MODE_SELECT command is responsible for general pipeline
    //!     level configuration that would normally be set once for a single stream
    //!     decode and would not be modified on a frame workload basis.
    //!     
    struct HUC_PIPE_MODE_SELECT_CMD
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
                uint32_t                 Reserved32                                       : __CODEGEN_BITFIELD( 0,  3)    ; //!< Reserved
                uint32_t                 IndirectStreamOutEnable                          : __CODEGEN_BITFIELD( 4,  4)    ; //!< INDIRECT_STREAM_OUT_ENABLE
                uint32_t                 Reserved37                                       : __CODEGEN_BITFIELD( 5,  9)    ; //!< Reserved
                uint32_t                 HucStreamObjectEnable                            : __CODEGEN_BITFIELD(10, 10)    ; //!< HUC Stream Object Enable
                uint32_t                 Reserved43                                       : __CODEGEN_BITFIELD(11, 31)    ; //!< Reserved
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

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HUCPIPEMODESELECT                      = 0, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HUP = Bh
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 11, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief INDIRECT_STREAM_OUT_ENABLE
        //! \details
        //!     Enables the bitstream to be written out to memory immediately following
        //!     the output of the bit stream.  The memory buffer is addressed through
        //!     the HuC Indirect Stream Out ObjectBase Address.
        enum INDIRECT_STREAM_OUT_ENABLE
        {
            INDIRECT_STREAM_OUT_ENABLE_DISABLEINDIRECTSTREAMOUT              = 0, //!< No additional details
            INDIRECT_STREAM_OUT_ENABLE_ENABLEINDIRECTSTREAMOUT               = 1, //!< No additional details
        };

        //! \brief MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS
        //! \details
        //!     In decoder modes, this counter value specifies the number of clocks (per
        //!     1000) of GAC inactivity
        //!     before a media soft-reset is applied to the HCP and HuC. If counter
        //!     value is set to 0, the media
        //!     soft-reset feature is disabled and no reset will occur.
        //!     In encoder modes, this counter must be set to 0 to disable media
        //!     soft reset. This feature is not
        //!     supported for the encoder.
        enum MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS
        {
            MEDIA_SOFT_RESET_COUNTER_PER_1000_CLOCKS_DISABLE                 = 0, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        HUC_PIPE_MODE_SELECT_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief HUC_IMEM_STATE
    //! \details
    //!     The HUC is selected with the Media Instruction Opcode "Bh" for all HUC
    //!     Commands. Each HUC command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    //!     The HUC_IMEM_STATE command is used to fetch the HUC firmware from the
    //!     WOPCM region and load it into the HUC 96KB L2 storage RAM. The
    //!     HUC_IMEM_STATE specifies the firmware's offset in WOPCM which is a cache
    //!     line aligned 32-bit offset address. The firmware, code and data segment
    //!     size should always be less than 96KB. If its greater than 96KB then the
    //!     firmware should be broken into smaller functions so that there is no
    //!     firmware straddling case.
    //!     When the HUC_IMEM_STATE command is received, the hardware confirms that
    //!     the code has been successfully authenticated by checking the VCR
    //!     provided authentication successful signal. If this signal is asserted,
    //!     the firmware is loaded by the HUC DMA into the 96KB L2 storage RAM. Once
    //!     the firmware is loaded, the VALID IMEM LOADED bit in the HUC_STATUS2
    //!     register is asserted high in the. If the authentication signal is not
    //!     asserted, the DMA aborts the HUC_IMEM_STATE command, the firmware is not
    //!     loaded, and the VALID IMEM LOADED bit remains low.
    //!     
    struct HUC_IMEM_STATE_CMD
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
                uint32_t                 Reserved32                                                                       ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 Reserved64                                                                       ; //!< Reserved
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 Reserved96                                                                       ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 HucFirmwareDescriptor                            : __CODEGEN_BITFIELD( 0,  7)    ; //!< HUC_FIRMWARE_DESCRIPTOR
                uint32_t                 Reserved136                                      : __CODEGEN_BITFIELD( 8, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HUCIMEMSTATE                           = 1, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HUC = Bh
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 11, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief HUC_FIRMWARE_DESCRIPTOR
        //! \details
        //!     This field specifies 1 of 255 firmware descriptors which describe
        //!     which firmware is be loaded in the L2 storage RAM. If the firmware
        //!     descriptor is set to zero, the HUC will not load the firmware.
        enum HUC_FIRMWARE_DESCRIPTOR
        {
            HUC_FIRMWARE_DESCRIPTOR_UNNAMED0                                 = 0, //!< Illegal
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        HUC_IMEM_STATE_CMD();

        static const size_t dwSize = 5;
        static const size_t byteSize = 20;
    };

    //!
    //! \brief HUC_DMEM_STATE
    //! \details
    //!      The HUC is selected with the Media Instruction Opcode "Bh" for all HUC
    //!     Commands. Each HUC command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    //!     The HUC_DMEM_STATE command is used to fetch the HUC data from the
    //!     graphics memory and load it into the HUC 96KB L2 storage RAM. The
    //!     HUC_DMEM_STATE specifies the data source base address in graphics
    //!     memory.
    //!     
    //!     When the HUC_DMEM_STATE command is received, the data is loaded by the
    //!     HUC DMA into the 96KB L2 storage RAM at the location provided in the
    //!     HUC_DMEM_STATE command. This command also specifies the length of the
    //!     data, which is specified in bytes but must be in increments of 64 byte
    //!     cache lines.
    //!     
    struct HUC_DMEM_STATE_CMD
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
        SPLITBASEADDRESS64BYTEALIGNED_CMD        HucDataSourceBaseAddress;                                                //!< DW1..2, HUC Data Source - Base Address
        MEMORYADDRESSATTRIBUTES_CMD              HucDataSourceAttributes;                                                 //!< DW3, HUC Data Source - Attributes
        union
        {
            struct
            {
                uint32_t                 Reserved128                                      : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 HucDataDestinationBaseAddress                    : __CODEGEN_BITFIELD( 6, 16)    ; //!< HUC Data Destination Base Address
                uint32_t                 Reserved145                                      : __CODEGEN_BITFIELD(17, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            struct
            {
                uint32_t                 Reserved160                                      : __CODEGEN_BITFIELD( 0,  5)    ; //!< Reserved
                uint32_t                 HucDataLength                                    : __CODEGEN_BITFIELD( 6, 16)    ; //!< HUC Data Length
                uint32_t                 Reserved177                                      : __CODEGEN_BITFIELD(17, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HUCDMEMSTATE                           = 2, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HUC = Bh
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 11, //!< No additional details
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
        HUC_DMEM_STATE_CMD();

        static const size_t dwSize = 6;
        static const size_t byteSize = 24;
    };

    //!
    //! \brief HUC_CFG_STATE
    //! \details
    //!     The HUC is selected with the Media Instruction Opcode "Bh" for all HUC
    //!     Commands. Each HUC command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    //!     The HUC_CFG_STATE command is used to force the P24C (MinuteIA) into a
    //!     reset condition as well as forcing it out of a reset condition. This
    //!     command is not normally required since the hardware will handle placing
    //!     the P24C into a reset condition and releasing it from reset, but there
    //!     may be conditions that require a forced reset.
    //!     
    struct HUC_CFG_STATE_CMD
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
                uint32_t                 P24CMinuteia                                     : __CODEGEN_BITFIELD( 0,  0)    ; //!< P24C_MINUTEIA
                uint32_t                 Reserved33                                       : __CODEGEN_BITFIELD( 1, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HUCCFGSTATE                            = 3, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HUC = Bh
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 11, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum P24C_MINUTEIA
        {
            P24C_MINUTEIA_NORMALOPERATION                                    = 0, //!< No reset.
            P24C_MINUTEIA_FORCERESET                                         = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        HUC_CFG_STATE_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

    //!
    //! \brief HUC_VIRTUAL_ADDR_REGION
    //! \details
    //!     
    //!     
    struct HUC_VIRTUAL_ADDR_REGION_CMD
    {
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        HucSurfaceBaseAddressVirtualaddrregion015;                               //!< DW0..2, HUC Surface Base Address (VirtualAddrRegion[0-15])
        MEMORYADDRESSATTRIBUTES_CMD              HucSurfaceVirtualaddrregion015;                                          //!< DW0..2, HUC Surface (VirtualAddrRegion[0-15])

        //! \name Local enumerations

        //! \name Initializations

        //! \brief Explicit member initialization function
        HUC_VIRTUAL_ADDR_REGION_CMD();

        static const size_t dwSize = 3;
        static const size_t byteSize = 12;
    };

    //!
    //! \brief HUC_VIRTUAL_ADDR_STATE
    //! \details
    //!     HUC is selected with the Media Instruction Opcode "Bh" for all HUC
    //!     Commands. Each HUC command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    //!     The HUC_VIRTUAL_ADDR_STATE command is used to define the 48-bit HUC
    //!     Surface Base Address and HUC Surface for each region.
    //!     
    struct HUC_VIRTUAL_ADDR_STATE_CMD
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
        HUC_VIRTUAL_ADDR_REGION_CMD              HucVirtualAddressRegion[16];                                             //!< DW1..48, Huc Virtual Address Region

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HUCVIRTUALADDRSTATE                    = 4, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HUC = Bh
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 11, //!< No additional details
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
        HUC_VIRTUAL_ADDR_STATE_CMD();

        static const size_t dwSize = 49;
        static const size_t byteSize = 196;
    };

    //!
    //! \brief HUC_IND_OBJ_BASE_ADDR_STATE
    //! \details
    //!     The HUC is selected with the Media Instruction Opcode "Bh" for all HUC
    //!     Commands. Each HUC command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    //!     The HUC_IND_OBJ_BASE_ADDR_STATE command is used to define the indirect
    //!     object base address of the stream in graphics memory. This is a frame
    //!     level command.
    //!     
    struct HUC_IND_OBJ_BASE_ADDR_STATE_CMD
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
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        HucIndirectStreamInObjectbaseAddress;                                    //!< DW1..2, HUC Indirect Stream In ObjectBase Address
        MEMORYADDRESSATTRIBUTES_CMD              HucIndirectStreamInObjectbaseAttributes;                                 //!< DW3, HUC Indirect Stream In ObjectBase Attributes
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        HucIndirectStreamInObjectaccessUpperBound;                               //!< DW4..5, HUC Indirect Stream In ObjectAccess Upper Bound
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        HucIndirectStreamOutObjectbaseAddress;                                   //!< DW6..7, HUC Indirect Stream Out ObjectBase Address
        MEMORYADDRESSATTRIBUTES_CMD              HucIndirectStreamOutObjectbaseAttributes;                                //!< DW8, HUC Indirect Stream Out ObjectBase Attributes
        SPLITBASEADDRESS4KBYTEALIGNED_CMD        HucIndirectStreamOutObjectaccessUpperBound;                              //!< DW9..10, HUC Indirect Stream Out ObjectAccess Upper Bound

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HUCINDOBJBASEADDRSTATE                 = 5, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HUC = Bh
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 11, //!< No additional details
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
        HUC_IND_OBJ_BASE_ADDR_STATE_CMD();

        static const size_t dwSize = 11;
        static const size_t byteSize = 44;
    };

    //!
    //! \brief HUC_STREAM_OBJECT
    //! \details
    //!     The HUC is selected with the Media Instruction Opcode "Bh" for all HUC
    //!     Commands. Each HUC command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    //!     The HUC_STREAM_OBJECT command is used to define the bit stream address
    //!     offset to the Stream Indirect Object base Address and the length of the
    //!     bit stream.  The bitstream buffer the HUC operates upon is specified
    //!     through indirect addressing.
    //!     
    struct HUC_STREAM_OBJECT_CMD
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
                uint32_t                 IndirectStreamInDataLength                                                       ; //!< Indirect Stream In Data Length
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            struct
            {
                uint32_t                 IndirectStreamInStartAddress                     : __CODEGEN_BITFIELD( 0, 28)    ; //!< Indirect Stream In Start Address
                uint32_t                 Reserved93                                       : __CODEGEN_BITFIELD(29, 30)    ; //!< Reserved
                uint32_t                 HucProcessing                                    : __CODEGEN_BITFIELD(31, 31)    ; //!< HUC_PROCESSING
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            struct
            {
                uint32_t                 IndirectStreamOutStartAddress                    : __CODEGEN_BITFIELD( 0, 28)    ; //!< Indirect Stream Out Start Address
                uint32_t                 Reserved125                                      : __CODEGEN_BITFIELD(29, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            struct
            {
                uint32_t                 StartCodeByte0                                   : __CODEGEN_BITFIELD( 0,  7)    ; //!< Start Code Byte [0]
                uint32_t                 StartCodeByte1                                   : __CODEGEN_BITFIELD( 8, 15)    ; //!< Start Code Byte [1]
                uint32_t                 StartCodeByte2                                   : __CODEGEN_BITFIELD(16, 23)    ; //!< Start Code Byte [2]
                uint32_t                 StartCodeSearchEngine                            : __CODEGEN_BITFIELD(24, 24)    ; //!< START_CODE_SEARCH_ENGINE
                uint32_t                 EmulationPreventionByteRemoval                   : __CODEGEN_BITFIELD(25, 25)    ; //!< EMULATION_PREVENTION_BYTE_REMOVAL
                uint32_t                 StreamOut                                        : __CODEGEN_BITFIELD(26, 26)    ; //!< STREAM_OUT
                uint32_t                 Drmlengthmode                                    : __CODEGEN_BITFIELD(27, 28)    ; //!< DRMLENGTHMODE
                uint32_t                 HucBitstreamEnable                               : __CODEGEN_BITFIELD(29, 29)    ; //!< HUC_BITSTREAM_ENABLE
                uint32_t                 Reserved158                                      : __CODEGEN_BITFIELD(30, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW4;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HUCSTREAMOBJECT                        = 32, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HUC = Bh
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 11, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        //! \brief HUC_PROCESSING
        //! \details
        //!     Disables the HEVC Decoder CABAC engine to prevent it from starting
        //!     while the HuC is processing. Must be set to "1" for HUC processing so
        //!     that the stream is directed to the HuC and not the CABAC engine.
        enum HUC_PROCESSING
        {
            HUC_PROCESSING_DISABLE                                           = 1, //!< No additional details
        };

        enum START_CODE_SEARCH_ENGINE
        {
            START_CODE_SEARCH_ENGINE_DISABLE                                 = 0, //!< Bypass Start Code Search Engine
            START_CODE_SEARCH_ENGINE_ENABLE                                  = 1, //!< Enables the start code search engine to stop on every third byte start code defined by Start Code Byte [2:0] defined in this DWord.
        };

        enum EMULATION_PREVENTION_BYTE_REMOVAL
        {
            EMULATION_PREVENTION_BYTE_REMOVAL_DISABLE                        = 0, //!< Bypass Emulation Prevention Byte Removal.
            EMULATION_PREVENTION_BYTE_REMOVAL_ENABLE                         = 1, //!< Emulation prevention bytes will be removed after the start code search engine.
        };

        //! \brief STREAM_OUT
        //! \details
        //!     Enables the stream output.
        enum STREAM_OUT
        {
            STREAM_OUT_DISABLE                                               = 0, //!< Disable the stream output.
            STREAM_OUT_ENABLE                                                = 1, //!< Enable the stream output.
        };

        enum DRMLENGTHMODE
        {
            DRMLENGTHMODE_STARTCODEMODE                                      = 0, //!< Stops on a start code
            DRMLENGTHMODE_LENGTHMODE                                         = 1, //!< Stops after a number of bytes are reached in the length counter
        };

        //! \brief HUC_BITSTREAM_ENABLE
        //! \details
        //!     style="color: rgb(0, 0, 0); font-family: Arial, sans-serif;
        //!     line-height: normal;">Enables the bitstream to be sent to the HuC
        enum HUC_BITSTREAM_ENABLE
        {
            HUC_BITSTREAM_ENABLE_DISABLE                                     = 0, //!< No additional details
            HUC_BITSTREAM_ENABLE_ENABLE                                      = 1, //!< No additional details
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        HUC_STREAM_OBJECT_CMD();

        static const size_t dwSize = 5;
        static const size_t byteSize = 20;
    };

    //!
    //! \brief HUC_START
    //! \details
    //!     The HUC is selected with the Media Instruction Opcode "Bh" for all HUC
    //!     Commands. Each HUC command has assigned a media instruction command as
    //!     defined in DWord 0, BitField 22:16.
    //!     
    struct HUC_START_CMD
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
                uint32_t                 Laststreamobject                                 : __CODEGEN_BITFIELD( 0,  0)    ; //!< LASTSTREAMOBJECT
                uint32_t                 Reserved33                                       : __CODEGEN_BITFIELD( 1, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW1;

        //! \name Local enumerations

        enum MEDIA_INSTRUCTION_COMMAND
        {
            MEDIA_INSTRUCTION_COMMAND_HUCSTART                               = 33, //!< No additional details
        };

        //! \brief MEDIA_INSTRUCTION_OPCODE
        //! \details
        //!     Codec/Engine Name = HUC = Bh
        enum MEDIA_INSTRUCTION_OPCODE
        {
            MEDIA_INSTRUCTION_OPCODE_CODECENGINENAME                         = 11, //!< No additional details
        };

        enum PIPELINE_TYPE
        {
            PIPELINE_TYPE_UNNAMED2                                           = 2, //!< No additional details
        };

        enum COMMAND_TYPE
        {
            COMMAND_TYPE_PARALLELVIDEOPIPE                                   = 3, //!< No additional details
        };

        enum LASTSTREAMOBJECT
        {
            LASTSTREAMOBJECT_NOTLASTSTREAMOBJECT                             = 0, //!< Not the last stream object in the workload.
            LASTSTREAMOBJECT_LASTSTREAMOBJECT                                = 1, //!< Last stream object in the workload.
        };

        //! \name Initializations

        //! \brief Explicit member initialization function
        HUC_START_CMD();

        static const size_t dwSize = 2;
        static const size_t byteSize = 8;
    };

};

#pragma pack()

#endif  // __MHW_VDBOX_HUC_HWCMD_G9_KBL_H__
